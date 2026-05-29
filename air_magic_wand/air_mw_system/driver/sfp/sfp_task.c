/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:  sfp_task.c
 * PURPOSE:
 *      SFP task implementation.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "FreeRTOS.h"
#include "task.h"
#include "air_error.h"
#include "db_api.h"
#include "sfp_db.h"
#include "sfp_task.h"
#include "sfp_trunk.h"
#include "sfp_module_handle.h"
#include "sfp_util.h"
#include "sfp_sff_data.h"
#include "sfp_pin.h"
#include "sfp_config_customer.h"
#include "sfp_auto_adaptation.h"
#include "sfp_module_inside_phy.h"
#ifdef AIR_SUPPORT_SFP_DDM
#include "sfp_ddm.h"
#endif
#include "sys_mgmt.h"
#if defined(AIR_EN_SFP_LED) && !defined(AIR_EN_SFP_LED_WITH_THREAD)
#include "sfp_led.h"
#endif
#include "sfp_msg.h"
#include <api/diag.h>
#include "osapi_timer.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_TASK_DB_READY_CHECK_DELAY_MS (10)
#define SFP_TASK_POLLING_INTERVAL_MS (100)

#define SFP_TASK_QUEUE_RECEIVE_DELAY_MS (1000)

#define SFP_TASK_TIMER_NAME "SFP_TIMR"
#define SFP_TASK_TIMER_3BASE_INTERVAL_MS (600) /* It should be 3 times of SFP_TASK_TIMER_BASE_INTERVAL_MS */
#define SFP_TASK_TIMER_5BASE_INTERVAL_MS (1000) /* It should be 5 times of SFP_TASK_TIMER_BASE_INTERVAL_MS */


/* MACRO FUNCTION DECLARATIONS
 */
/* Get an item of an array by its index.  */
#define SFP_TASK_ARRAY_GET_ITH_ITEM(array, index, type)   ((UI8_T *)(array) + ((index) * sizeof(type)))
/* DATA TYPE DECLARATIONS
 */
typedef struct
{
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
    TaskHandle_t task_handle;
#endif
    TimerHandle_t timer_handle;
    SFP_TASK_PORT_INFO_T *port_info;
    UI8_T sfp_port_total_num;
    UI8_T timer_message_handled;
    BOOL_T sfp_ready;
#ifndef AIR_SUPPORT_SFP_WITH_THREAD
    UI8_T start_sfp_timer;
#endif
} SFP_TASK_CONTEXT_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SFP_TASK_CONTEXT_T _sfp_task_ctx;

/* LOCAL SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
static void
_sfp_task_timer_handleExpired(
    TimerHandle_t ptr_timer)
{
    SFP_MSG_T *ptr_message = NULL;
    MW_ERROR_NO_T ret = AIR_E_OTHERS;

    if (TRUE == _sfp_task_ctx.timer_message_handled)
    {
        ptr_message = sfp_msg_create(0, MW_MSG_ID_SFP_TIMER_BASE_TIMER_EXPIRED_NOTI, 0);
        if (NULL != ptr_message)
        {
            ret = sfp_msg_send(ptr_message);
            if (MW_E_OK == ret)
            {
                _sfp_task_ctx.timer_message_handled = FALSE;
            }
            else
            {
                MW_FREE(ptr_message);
            }
        }
    }
}
#endif
static void
_sfp_task_deinit(
    const UI32_T unit)
{
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
    const C8_T *ptr_queue_name = sfp_msg_get_queue_name();

    osapi_msgDelete(ptr_queue_name);
#endif
    sfp_msg_set_queue_name(NULL);

    if (NULL != _sfp_task_ctx.timer_handle)
    {
        osapi_timerStop(_sfp_task_ctx.timer_handle);
        osapi_timerDelete(_sfp_task_ctx.timer_handle);
        _sfp_task_ctx.timer_handle = NULL;
    }

    if (NULL != _sfp_task_ctx.port_info)
    {
        osapi_free(_sfp_task_ctx.port_info);
        _sfp_task_ctx.port_info = NULL;
    }
    _sfp_task_ctx.sfp_port_total_num = 0;
    _sfp_task_ctx.timer_message_handled = TRUE;

    sfp_db_queue_free();
    sfp_phy_deinit(unit);
}

static void
_sfp_task_message_handleBaseIntervalExpired(
    SFP_MSG_T *ptr_message)
{
    UI32_T i = 0, unit = 0;
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;
    BOOL_T isTrunkMember = FALSE;

    for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
    {
        ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);

        /* Check Trunk */
        isTrunkMember = FALSE;
        ret = sfp_trunk_check(ptr_port_info->port, NULL, NULL);
        if (AIR_E_OK == ret)
        {
            isTrunkMember = TRUE;
        }

        if (TRUE == isTrunkMember)
        {
            ptr_port_info->trunk_member_status = SFP_TRUNK_MEMBER_STATUS_TRUE;
        }
        else
        {
            ptr_port_info->trunk_member_status |= SFP_TRUNK_MEMBER_STATUS_FALSE;
        }

        if (SFP_TRUNK_MEMBER_STATUS_REMOVED == ptr_port_info->trunk_member_status)
        {
            ptr_port_info->trunk_member_status = SFP_TRUNK_MEMBER_STATUS_NONE;
            sfp_trunk_handleTrunkDeleted(unit, ptr_port_info->port);
        }

        /* Polling */
        sfp_pin_io_SWPinPolling(ptr_message->unit, ptr_port_info);
        sfp_module_pollingSFPModuleStatus(ptr_message->unit, ptr_port_info);
        if (SFP_MODULE_PORT_STATE_ACTIVE == ptr_port_info->state)
        {
            if ((0 != ptr_port_info->err_count) && (0 != ptr_port_info->delay_count))
            {
                /* Currently, ptr_port_info->err_count is reused to indicate 1st link up after SFP module is inserted. */
                ptr_port_info->delay_count--;
                if (0 == ptr_port_info->delay_count)
                {
                    ptr_port_info->err_count = 0;
                    sfp_phy_linkStatusChangeCallback(ptr_message->unit, ptr_port_info->port, TRUE, NULL);
                }
            }
        }
    }
}

static void
_sfp_task_message_handle3BaseIntervalExpired(
    SFP_MSG_T *ptr_message)
{
    UI32_T i = 0;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;

    sfp_phy_pollingLinkStatus(ptr_message->unit);
    for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
    {
        ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
        sfp_adaptation_polling(ptr_message->unit, i, ptr_port_info);
    }

#if defined(AIR_EN_SFP_LED) && !defined(AIR_EN_SFP_LED_WITH_THREAD)
    sfp_led_updateLedStatus(ptr_message->unit);
#endif
}

static void
_sfp_task_message_handle5BaseIntervalExpired(
    SFP_MSG_T *ptr_message)
{
#ifdef AIR_SUPPORT_SFP_DDM
    UI32_T i = 0;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;

    for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
    {
        ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
        sfp_ddm_polling(ptr_message->unit, ptr_port_info);
    }
#endif  /* AIR_SUPPORT_SFP_DDM */
}

static void
_sfp_task_message_handleTimerExpired(
    SFP_MSG_T *ptr_message)
{
    static UI8_T _3base_interval_count = SFP_TASK_TIMER_3BASE_INTERVAL_MS / SFP_TASK_TIMER_BASE_INTERVAL_MS;
    static UI8_T _5base_interval_count = SFP_TASK_TIMER_5BASE_INTERVAL_MS / SFP_TASK_TIMER_BASE_INTERVAL_MS;

    _sfp_task_ctx.timer_message_handled = TRUE;

    _sfp_task_message_handleBaseIntervalExpired(ptr_message);

    _3base_interval_count--;
    if (0 == _3base_interval_count)
    {
        /* Long timer expires. Reset long timer. */
        _3base_interval_count = SFP_TASK_TIMER_3BASE_INTERVAL_MS / SFP_TASK_TIMER_BASE_INTERVAL_MS;
        _sfp_task_message_handle3BaseIntervalExpired(ptr_message);
    }

    _5base_interval_count--;
    if (0 == _5base_interval_count)
    {
        /* Long timer expires. Reset long timer. */
        _5base_interval_count = SFP_TASK_TIMER_5BASE_INTERVAL_MS / SFP_TASK_TIMER_BASE_INTERVAL_MS;
        _sfp_task_message_handle5BaseIntervalExpired(ptr_message);
    }
}

static void
_sfp_task_message_handleCMD(
    SFP_MSG_T *ptr_message)
{
    if (MW_MSG_ID_SFP_CMD_START_SFP_REQ == ptr_message->msg_id)
    {
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
        if (MW_E_OK != osapi_timerActive(_sfp_task_ctx.timer_handle))
        {
            osapi_timerStart(_sfp_task_ctx.timer_handle);
        }
#else
        _sfp_task_ctx.start_sfp_timer = TRUE;
#endif
    }
    else if (MW_MSG_ID_SFP_CMD_STOP_SFP_REQ == ptr_message->msg_id)
    {
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
        if (MW_E_OK == osapi_timerActive(_sfp_task_ctx.timer_handle))
        {
            osapi_timerStop(_sfp_task_ctx.timer_handle);
        }
#else
        _sfp_task_ctx.start_sfp_timer = FALSE;
#endif
    }
}

#ifdef AIR_SUPPORT_SFP_WITH_THREAD
/* FUNCTION NAME:   _sfp_task_main
 * PURPOSE:
 *      SFP application task.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      This task will cycle through the status of the SFP port.
 *
 */
static void
_sfp_task_main(
    void)
{
    I32_T ret = AIR_E_OTHERS;
    UI32_T unit = 0;
    SFP_MSG_T *ptr_message = NULL;

    ret = sfp_init(unit, SFP_MSG_QUEUE_NAME);
    if (AIR_E_OK != ret)
    {
        threadhandle_t task_handle = _sfp_task_ctx.task_handle;

        SFP_LOG_ERROR("Fail to init SFP task! Remove it. ret:%d", ret);
        _sfp_task_ctx.task_handle = NULL;
        osapi_threadDelete(task_handle);
        return;
    }

    while (1)
    {
        ret = osapi_msgRecv(SFP_MSG_QUEUE_NAME, (UI8_T **)&ptr_message, 0, SFP_TASK_QUEUE_RECEIVE_DELAY_MS / portTICK_PERIOD_MS);
        if (MW_E_OK == ret)
        {
            sfp_handle_msg((MW_MSG_T *)ptr_message);
            MW_FREE(ptr_message);
        }
    }
}
#endif

MW_ERROR_NO_T
_sfp_task_checkPortTypeSettings(
    const UI32_T unit)
{
    I32_T ret = MW_E_NO_MEMORY;
    AIR_INIT_PORT_MAP_T *ptr_portMapList = NULL;
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettingsArray = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize(), i = 0, total_port_num = 0, port = 0;

    if ((0 == port_count) || (NULL == ptr_portSettingsArray))
    {
        SFP_LOG_WARN("No SFP port configured!");
        return MW_E_OP_INVALID;
    }

    osapi_calloc((sizeof(AIR_INIT_PORT_MAP_T) * PLAT_TOTAL_NUM), SFP_TASK_NAME, (void**)&ptr_portMapList);
    if (NULL != ptr_portMapList)
    {
        ret = air_init_getSdkPortMap(unit, &total_port_num, ptr_portMapList);
        if (AIR_E_OK == ret)
        {
            for (i = 0; i < port_count; i++)
            {
                port = ptr_portSettingsArray[i].port;
                if ((PLAT_CPU_PORT == ptr_portSettingsArray[i].port) ||
                    (PLAT_TOTAL_NUM < ptr_portSettingsArray[i].port) ||
                    ((SFP_CONFIG_PORT_TYPE_SFP != ptr_portSettingsArray[i].port_type) &&
                     (SFP_CONFIG_PORT_TYPE_COMBO != ptr_portSettingsArray[i].port_type) &&
                     (SFP_CONFIG_PORT_TYPE_COMBO_SFP != ptr_portSettingsArray[i].port_type)) ||
                    (AIR_INIT_PORT_TYPE_XSGMII != ptr_portMapList[port].port_type) ||
                    ((0 != (ptr_portMapList[port].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_COMBO)) &&
                     ((SFP_CONFIG_PORT_TYPE_COMBO != ptr_portSettingsArray[i].port_type) &&
                      (SFP_CONFIG_PORT_TYPE_COMBO_SFP != ptr_portSettingsArray[i].port_type))) ||
                    ((0 == (ptr_portMapList[port].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_COMBO)) &&
                     ((SFP_CONFIG_PORT_TYPE_COMBO == ptr_portSettingsArray[i].port_type) ||
                      (SFP_CONFIG_PORT_TYPE_COMBO_SFP == ptr_portSettingsArray[i].port_type))))
                {
                    SFP_LOG_ERROR("Wrong settings for SFP port:%d", port);
                    ret = MW_E_BAD_PARAMETER;
                    break;
                }
            }
        }

        osapi_free(ptr_portMapList);
    }

    return (MW_ERROR_NO_T)ret;
}

/* EXPORTED SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
/* FUNCTION NAME:   sfp_task_create
 * PURPOSE:
 *      Create the SFP task.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_task_create(
    void)
{
    UI32_T unit = 0;
    MW_ERROR_NO_T ret = _sfp_task_checkPortTypeSettings(unit);

    if (MW_E_OK != ret)
    {
        SFP_LOG_WARN("No SFP port or wrong SFP port type settings. ret:%d", ret);
        return;
    }

    if (NULL == _sfp_task_ctx.task_handle)
    {
        osapi_threadCreate(SFP_TASK_NAME,
                           SFP_TASK_STACK_SIZE,
                           MW_TASK_PRIORITY_SFP,
                           (threadfunc_t)_sfp_task_main,
                           NULL,
                           &(_sfp_task_ctx.task_handle));
    }
}
#endif

/* FUNCTION NAME:   sfp_task_getPortIndex
 * PURPOSE:
 *      Get the index of a port in the array _sfp_task_ctx.port_info.
 *
 * INPUT:
 *      port                         -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      SFP_TASK_PORT_INDEX_INVALID  -- Failed
 *      Other value                  -- The index of the port
 *
 * NOTES:
 *      None.
 */
UI8_T
sfp_task_getPortIndex(
    UI8_T port)
{
    UI32_T i = 0;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;

    for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
    {
        ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
        if (ptr_port_info->port == port)
        {
            return i + 1;
        }
    }

    return SFP_TASK_PORT_INDEX_INVALID;
}

/* FUNCTION NAME:   sfp_task_getPortInfo
 * PURPOSE:
 *      Get the port information of a port.
 *
 * INPUT:
 *      port                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      Not NULL            -- A pointer points to the port information
 *      NULL                -- Failed
 *
 * NOTES:
 *      None.
 */
SFP_TASK_PORT_INFO_T *
sfp_task_getPortInfo(
    UI8_T port)
{
    UI32_T i = 0;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;

    for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
    {
        ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
        if (ptr_port_info->port == port)
        {
            return ptr_port_info;
        }
    }

    SFP_LOG_WARN("No task context for port:%d", port);
    return NULL;
}

/* FUNCTION NAME:   sfp_init
 * PURPOSE:
 *      Initialize SFP.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_queue_name           -- The name of the queue receives SFP messages
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      The message will be freed before the API returns.
 */
MW_ERROR_NO_T
sfp_init(
    const UI32_T unit,
    const C8_T * const ptr_queue_name)
{
    I32_T ret = AIR_E_NOT_INITED;
    UI32_T i = 0;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettingsArray = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize();

#ifndef AIR_SUPPORT_SFP_WITH_THREAD
    ret = _sfp_task_checkPortTypeSettings(unit);
    if (MW_E_OK != ret)
    {
        return MW_E_OP_INVALID;
    }
#endif

    if ((0 == port_count) || (NULL == ptr_portSettingsArray))
    {
        SFP_LOG_ERROR("No SFP port configured!");
        return MW_E_OP_INVALID;
    }

    diag_setDebugFlag(AIR_MODULE_SIF, HAL_DBG_ERR, FALSE);

    do {
        sfp_msg_set_queue_name(ptr_queue_name);

#ifdef AIR_SUPPORT_SFP_WITH_THREAD
        ret = osapi_msgCreate((const C8_T *)ptr_queue_name, SFP_MSG_QUEUE_LENGTH, sizeof(SFP_MSG_T *));
        if (MW_E_OK != ret)
        {
            break;
        }
        ret = osapi_timerCreate(SFP_TASK_TIMER_NAME,
                                _sfp_task_timer_handleExpired,
                                TRUE,
                                SFP_TASK_TIMER_BASE_INTERVAL_MS / portTICK_PERIOD_MS,
                                NULL,
                                &(_sfp_task_ctx.timer_handle));
        if (MW_E_OK != ret)
        {
            break;
        }
#endif

        _sfp_task_ctx.timer_message_handled = TRUE;

        /* Wait until DB is ready */
        do {
            ret = dbapi_dbisReady();
            if (MW_E_OK != ret)
            {
                osapi_delay(SFP_TASK_DB_READY_CHECK_DELAY_MS);
            }
            else
            {
                break;
            }
        } while (1);

        ret = sfp_db_queue_create();
        if (AIR_E_OK != ret)
        {
            break;
        }

        _sfp_task_ctx.sfp_port_total_num = port_count;
        SFP_LOG_DEBUG("Total SFP port:%d PLAT_MAX_PORT_NUM:%d", _sfp_task_ctx.sfp_port_total_num, PLAT_MAX_PORT_NUM);

        _sfp_task_ctx.port_info = NULL;
        osapi_calloc((sizeof(SFP_TASK_PORT_INFO_T)) * _sfp_task_ctx.sfp_port_total_num, SFP_TASK_NAME, (void **)&_sfp_task_ctx.port_info);
        if (NULL != _sfp_task_ctx.port_info)
        {
            for (i = 0; i < port_count; i++)
            {
                /* Init _sfp_task_ctx.port_info */
                ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
                ptr_port_info->port = ptr_portSettingsArray[i].port;
                ptr_port_info->state = SFP_MODULE_PORT_STATE_ABSENT;
                ptr_port_info->abs_pin_state = SFP_MODULE_PIN_HIGH;
                ptr_port_info->rx_loss_pin_state = SFP_MODULE_PIN_HIGH;
                ptr_port_info->auto_adaption_retry_count = 0;
                ptr_port_info->phy_2wire_addr = SFP_SFF_INVALID_ADDRESS;
                ptr_port_info->trunk_member_status = SFP_TRUNK_MEMBER_STATUS_NONE;
                ptr_port_info->rx_loss_delay_count = 0;
                SFP_LOG_DEBUG("ptr_port_info:0x%p index:%d port:%d", ptr_port_info, i, ptr_port_info->port);
            }
        }
        else
        {
            ret = AIR_E_NO_MEMORY;
            break;
        }

        ret = sfp_pin_init(unit);
        if (AIR_E_OK != ret)
        {
            break;
        }

        ret = sfp_trunk_init(unit);
        if (AIR_E_OK != ret)
        {
            break;
        }

        ret = sfp_auto_adaption_init(unit);
        if (AIR_E_OK != ret)
        {
            break;
        }

        ret = sfp_phy_init(unit);
        if (AIR_E_OK != ret)
        {
            break;
        }

        _sfp_task_ctx.sfp_ready = TRUE;
#ifndef AIR_SUPPORT_SFP_WITH_THREAD
        _sfp_task_ctx.start_sfp_timer = TRUE;
#endif
        for (i = 0; i < _sfp_task_ctx.sfp_port_total_num; i++)
        {
            ptr_port_info = (SFP_TASK_PORT_INFO_T *)SFP_TASK_ARRAY_GET_ITH_ITEM(_sfp_task_ctx.port_info, i, SFP_TASK_PORT_INFO_T);
            if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
            {
                /* Init COMBO PHY ports. */
                BOOL_T admin_state = FALSE;
                AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_PHY;
                AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_SGMII;

                air_port_getAdminState(unit, ptr_port_info->port, &admin_state);
                if (TRUE == sfp_port_is_pureComboSerdesPort(unit, ptr_port_info->port))
                {
                    combo_mode = AIR_PORT_COMBO_MODE_SERDES;
                    serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;

                    sfp_port_setComboMode(unit, ptr_port_info->port, admin_state, TRUE, combo_mode);
                    air_port_setSerdesMode(unit, ptr_port_info->port, serdes_mode);
                }
                else
                {
                    sfp_port_setComboMode(unit, ptr_port_info->port, admin_state, TRUE, combo_mode);
                }
                sfp_db_updatePortMode(unit, ptr_port_info, combo_mode, serdes_mode);
                sfp_module_state_initPortBasedOnDB(ptr_port_info);
                /* Even if error occurs, still set port inited. */
                sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
            }
        }

#ifdef AIR_SUPPORT_SFP_WITH_THREAD
        ret = osapi_timerStart(_sfp_task_ctx.timer_handle);
        if (MW_E_OK != ret)
        {
            break;
        }
#endif
    } while (0);

    if (AIR_E_OK != ret)
    {
        _sfp_task_deinit(unit);
    }

    return ret;
}

/* FUNCTION NAME:   sfp_handle_msg
 * PURPOSE:
 *      Handle the SFP messages.
 *
 * INPUT:
 *      ptr_message         -- The pointer points to the message to be handled.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      The message need be freed after the API returns.
 */
void
sfp_handle_msg(
    MW_MSG_T *ptr_message)
{
    switch(ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_TIMER_BASE_TIMER_EXPIRED_NOTI:
        {
            _sfp_task_message_handleTimerExpired((SFP_MSG_T *)ptr_message);
            break;
        }

        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        case MW_MSG_ID_SFP_SFP_RXLOS_PIN_STATE_CHANGED_NOTI:
        case MW_MSG_ID_SFP_SFP_INFO_FOR_SERDES_MODE_OBTAINED_NOTI:
        {
            sfp_module_handleMessage((SFP_MSG_T *)ptr_message);
            break;
        }

        case MW_MSG_ID_SFP_CMD_START_SFP_REQ:
        case MW_MSG_ID_SFP_CMD_STOP_SFP_REQ:
        {
            _sfp_task_message_handleCMD((SFP_MSG_T *)ptr_message);
            break;
        }

        default:
        {
            SFP_LOG_WARN("Unknown msg_id:%d", ptr_message->msg_id);
            break;
        }
    }
}

/* FUNCTION NAME:   sfp_check_ready
 * PURPOSE:
 *      Check if SFP task is initialized successfully.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE               -- SFP task is initialized successfully.
 *      FALSE              -- SFP task has not been initialized successfully
 *
 * NOTES:
 *      None.
 */
BOOL_T
sfp_check_ready(
    void)
{
    return _sfp_task_ctx.sfp_ready;
}

/* FUNCTION NAME:   sfp_set_port_inited
 * PURPOSE:
 *      Set flag(s) to indicate the port setting(s) which is(are) complete for
 *      a port.
 *
 * INPUT:
 *      port                 -- Port number
 *      add_bits             -- Flag(s) to be added
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NOT_SUPPORT    -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_set_port_inited(
    UI8_T port,
    SFP_PORT_INITED_BITMASK_T add_bits)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        ptr_port_info->port_inited |= add_bits;
        SFP_LOG_DEBUG("%s port:%d add_bits:0x%x port_inited:0x%x", __func__, port, add_bits, ptr_port_info->port_inited);
        return MW_E_OK;
    }

    return MW_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   sfp_clear_port_inited
 * PURPOSE:
 *      Clear flag(s) that is(are) used to indicate the port setting(s) which
 *      is(are) complete for a port.
 *
 * INPUT:
 *      port                 -- Port number
 *      delete_bits          -- Flag(s) to be cleared
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NOT_SUPPORT    -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_clear_port_inited(
    UI8_T port,
    SFP_PORT_INITED_BITMASK_T delete_bits)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        ptr_port_info->port_inited &= (~delete_bits);
        SFP_LOG_DEBUG("%s port:%d delete_bits:0x%x port_inited:0x%x", __func__, port, delete_bits, ptr_port_info->port_inited);
        return MW_E_OK;
    }

    return MW_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   sfp_get_port_inited
 * PURPOSE:
 *      Check if all port settings are complete for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                 -- All port settings are set.
 *      FALSE                -- Not all port settings are set.
 *
 * NOTES:
 *      None.
 */
BOOL_T
sfp_get_port_inited(
    UI8_T port)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        return (ptr_port_info->port_inited & SFP_PORT_INITED_BITMASK_ALL) == SFP_PORT_INITED_BITMASK_ALL;
    }

    /* Not a serdes port. */
    return TRUE;
}

#ifndef AIR_SUPPORT_SFP_WITH_THREAD
/* FUNCTION NAME:   sfp_handle_timerExpired
 * PURPOSE:
 *      Handle sfp base timer expired.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_handle_timerExpired(
    void)
{
    SFP_MSG_T *ptr_message = NULL;

    if (TRUE != _sfp_task_ctx.start_sfp_timer)
    {
        return;
    }

    ptr_message = sfp_msg_create(0, MW_MSG_ID_SFP_TIMER_BASE_TIMER_EXPIRED_NOTI, 0);
    if (NULL != ptr_message)
    {
        _sfp_task_message_handleTimerExpired(ptr_message);
        MW_FREE(ptr_message);
    }
}
#endif

