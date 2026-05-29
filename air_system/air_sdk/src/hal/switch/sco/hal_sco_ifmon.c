/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Airoha Technology Corp. (C) 2021
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

/* FILE NAME:  hal_sco_ifmon.c
 * PURPOSE:
 *      It provide IfMon module HAL layer API.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <osal/osal.h>
#ifdef AIR_EN_LED_CLK_SYNC
#include <hal/common/hal_phy.h>
#endif /* End of AIR_EN_LED_CLK_SYNC */
#include <air_cfg.h>
#include <air_ifmon.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/sco/hal_sco_drv.h>
#include <hal/switch/sco/hal_sco_ifmon.h>
#include <hal/switch/sco/hal_sco_port.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_EN_LED_CLK_SYNC
/* 0x00A4CB80 = 3 hrs * 60 mins * 60 secs * 1000 mini-secs */
#define HAL_SCO_IFMON_RAND_SYNC_TIME (0x00A4CB80)

/* 0x01EE6280 = 9 hrs * 60 mins * 60 secs * 1000 mini-secs */
#define HAL_SCO_IFMON_BASE_SYNC_TIME (0x01EE6280)
#endif /* End of AIR_EN_LED_CLK_SYNC */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_SCO_IFMON_SEM_CONF(unit) _ptr_hal_sco_ifmon_cb[unit]->sem_conf

#define HAL_SCO_IFMON_SEM_PORT(unit) _ptr_hal_sco_ifmon_cb[unit]->sem_portstatus

#define HAL_SCO_IFMON_MONITOR_STATE(unit) _ptr_hal_sco_ifmon_cb[unit]->monitor_state

#define HAL_SCO_IFMON_THREAD_ID(unit) _ptr_hal_sco_ifmon_cb[unit]->thread_id

#define HAL_SCO_IFMON_MODE(unit) _ptr_hal_sco_ifmon_cb[unit]->mode

#define HAL_SCO_IFMON_INTERVAL(unit) _ptr_hal_sco_ifmon_cb[unit]->interval

#define HAL_SCO_IFMON_SCAN_PORT_BITMAP(unit) _ptr_hal_sco_ifmon_cb[unit]->scan_port_bitmap

#define HAL_SCO_IFMON_CUR_LINK_BITMAP(unit) _ptr_hal_sco_ifmon_cb[unit]->cur_state.link_bitmap

#define HAL_SCO_IFMON_NEW_LINK_BITMAP(unit) _ptr_hal_sco_ifmon_cb[unit]->new_state.link_bitmap

#define HAL_SCO_IFMON_DEV_LINK_BITMAP(unit) _ptr_hal_sco_ifmon_cb[unit]->dev_state.link_bitmap

#define PTR_SCO_IFMON_PORT_LINK_STATUS(__unit__, __port__) (&(_ptr_hal_sco_ifmon_cb[__unit__]->dev_status[__port__]))

#define PTR_HAL_SCO_IFMON_NOTIFY_HANDLER(__unit__, __idx__)       \
    (&(_ptr_hal_sco_ifmon_cb[__unit__]->notify_handler[__idx__]))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_IFMON, "hal_sco_ifmon.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

static HAL_SCO_IFMON_CB_T *_ptr_hal_sco_ifmon_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_processCallbackFunc(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T link)
{
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                          idx = 0;

    for (idx = 0; idx < HAL_SCO_IFMON_NOTIFY_HANDLER_CNT; idx++)
    {
        ptr_handler = PTR_HAL_SCO_IFMON_NOTIFY_HANDLER(unit, idx);
        if (NULL != ptr_handler->notify_func)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Process handler %p: notify_func %p, cookie %p\n", ptr_handler,
                       ptr_handler->notify_func, ptr_handler->ptr_cookie);
            ptr_handler->notify_func(unit, port, link, ptr_handler->ptr_cookie);
        }
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_ifmon_updatePortLink(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T cur_link,
    const BOOL_T new_link)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         notify = FALSE;
    UI32_T         tx_en = 0;

    if ((FALSE == cur_link) && (TRUE == new_link))
    {
        tx_en = TRUE;
        notify = TRUE;
    }
    else if ((TRUE == cur_link) && (FALSE == new_link))
    {
        tx_en = FALSE;
        notify = TRUE;
    }

    if (TRUE == notify)
    {
        rc = _hal_sco_ifmon_processCallbackFunc(unit, port, tx_en);
    }

    DIAG_PRINT(HAL_DBG_INFO, " Update unit %u port %u cur_link %d new_link %d rc %d\n", unit, port, cur_link, new_link,
               rc);

    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_updateHwLinkStatus
 * PURPOSE:
 *      To get hardware link status and update control block.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      link_bitmap         --  Link port bitmap
 *      link_state          --  Port status
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_updateHwLinkStatus(
    const UI32_T      unit,
    AIR_PORT_BITMAP_T link_bitmap)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             port = 0;
    AIR_PORT_STATUS_T  port_status = {0};
    AIR_PORT_STATUS_T *ptr_dev_status = NULL;

    AIR_PORT_BITMAP_CLEAR(link_bitmap);
    AIR_PORT_FOREACH(HAL_SCO_IFMON_SCAN_PORT_BITMAP(unit), port)
    {
        rc = hal_sco_port_getPortStatus(unit, port, &port_status);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            DIAG_PRINT(HAL_DBG_WARN, " _hal_sco_ifmon_updateHwLinkStatus: unit %u port %u rc %d\n", unit, port, rc);
            continue;
        }

        if (AIR_PORT_STATUS_FLAGS_LINK_UP & port_status.flags)
        {
            AIR_PORT_ADD(link_bitmap, port);
        }

        ptr_dev_status = PTR_SCO_IFMON_PORT_LINK_STATUS(unit, port);

        HAL_SCO_IFMON_PORT_LOCK(unit); /* lock */
        ptr_dev_status->speed = port_status.speed;
        ptr_dev_status->duplex = port_status.duplex;
        ptr_dev_status->flags = port_status.flags;
        HAL_SCO_IFMON_PORT_UNLOCK(unit); /* unlock */
    }

    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_processDevState
 * PURPOSE:
 *      To process device state.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_processDevState(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _hal_sco_ifmon_updateHwLinkStatus(unit, HAL_SCO_IFMON_DEV_LINK_BITMAP(unit));
    CMLIB_PORT_BITMAP_SET(HAL_SCO_IFMON_NEW_LINK_BITMAP(unit), HAL_SCO_IFMON_DEV_LINK_BITMAP(unit));

    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_processLink
 * PURPOSE:
 *      To process link.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_processLink(
    const UI32_T unit)
{
    AIR_PORT_BITMAP_T changed_link_bitmap;
    AIR_PORT_BITMAP_T changed_port_bitmap;
    BOOL_T            cur_link = FALSE;
    BOOL_T            new_link = FALSE;
    UI32_T            link_equal = 0;
    UI32_T            port = 0;
    AIR_ERROR_NO_T    rc = AIR_E_OK;

    link_equal = AIR_PORT_BITMAP_EQUAL(HAL_SCO_IFMON_CUR_LINK_BITMAP(unit), HAL_SCO_IFMON_NEW_LINK_BITMAP(unit));

    DIAG_PRINT(HAL_DBG_INFO, " link equal %d\n", link_equal);

    if (TRUE == link_equal)
    {
        return AIR_E_OK;
    }

    CMLIB_PORT_BITMAP_SET(changed_link_bitmap, HAL_SCO_IFMON_CUR_LINK_BITMAP(unit));
    CMLIB_PORT_BITMAP_XOR(changed_link_bitmap, HAL_SCO_IFMON_NEW_LINK_BITMAP(unit));
    CMLIB_PORT_BITMAP_SET(changed_port_bitmap, changed_link_bitmap);

    /* Scan all port to handle link change and speed change */
    AIR_PORT_FOREACH(changed_port_bitmap, port)
    {
        cur_link = AIR_PORT_CHK(HAL_SCO_IFMON_CUR_LINK_BITMAP(unit), port) ? TRUE : FALSE;

        DIAG_PRINT(HAL_DBG_INFO, " Update unit %u port %u cur_link %d\n", unit, port, cur_link);

        /* Get new status */
        new_link = AIR_PORT_CHK(HAL_SCO_IFMON_NEW_LINK_BITMAP(unit), port) ? TRUE : FALSE;

        DIAG_PRINT(HAL_DBG_INFO, " Update unit %u port %u new_link %d\n", unit, port, new_link);

        /* Handle link change */
        if (new_link != cur_link)
        {
            if (TRUE == new_link)
            {
                AIR_PORT_ADD(HAL_SCO_IFMON_CUR_LINK_BITMAP(unit), port);
            }
            else
            {
                AIR_PORT_DEL(HAL_SCO_IFMON_CUR_LINK_BITMAP(unit), port);
            }

            rc = _hal_sco_ifmon_updatePortLink(unit, port, cur_link, new_link);
        }
    }

    return rc;
}

#ifdef AIR_EN_LED_CLK_SYNC
static AIR_ERROR_NO_T
_hal_sco_ifmon_setSrand(
    UI32_T unit)
{
    UI32_T tmp_val, kval = 0;

    /* Read EFUSE P0 calibration value*/
    tmp_val = 0x40200052;
    aml_writeReg(unit, EFUSE_CTRL, &tmp_val, sizeof(UI32_T));
    aml_readReg(unit, EFUSE_CTRL, &tmp_val, sizeof(UI32_T));
    tmp_val = 0;
    aml_readReg(unit, EFUSE_RDATA0, &tmp_val, sizeof(UI32_T));
    kval += tmp_val;
    aml_readReg(unit, EFUSE_RDATA1, &tmp_val, sizeof(UI32_T));
    kval += tmp_val;
    aml_readReg(unit, EFUSE_RDATA2, &tmp_val, sizeof(UI32_T));
    kval += tmp_val;
    aml_readReg(unit, EFUSE_RDATA3, &tmp_val, sizeof(UI32_T));
    kval += tmp_val;
    osal_srand(kval);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_ifmon_getRandSyncTime(
    I32_T *ptr_time)
{
    (*ptr_time) = HAL_SCO_IFMON_BASE_SYNC_TIME + (osal_rand() % HAL_SCO_IFMON_RAND_SYNC_TIME);
    return AIR_E_OK;
}
#endif /* End of AIR_EN_LED_CLK_SYNC */

/* FUNCTION NAME:   _hal_sco_ifmon_thread
 * PURPOSE:
 *      IfMon thread.
 * INPUT:
 *      ptr_argv            --  Pointer for device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_ifmon_thread(
    void *ptr_argv)
{
    UI32_T         unit = (UI32_T)(AIR_HUGE_T)ptr_argv;
    UI32_T         interval = HAL_SCO_IFMON_INTERVAL_DFLT;
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_LED_CLK_SYNC
    I32_T  re_sync_ms_cnt = 0;
    BOOL_T do_re_sync = FALSE;
    UI32_T port_cnt = 0;
#endif /* End of AIR_EN_LED_CLK_SYNC */

#ifdef AIR_EN_LED_CLK_SYNC
    _hal_sco_ifmon_setSrand(unit);
    _hal_sco_ifmon_getRandSyncTime(&re_sync_ms_cnt);
    CMLIB_PORT_BITMAP_COUNT(HAL_PORT_BMP_ETH(unit), port_cnt);
    if (port_cnt >= 16)
    {
        do_re_sync = TRUE;
    }
#endif /* End of AIR_EN_LED_CLK_SYNC */

    while (AIR_E_OK == osal_isRunThread())
    {
        osal_sleepTask(interval);

        HAL_SCO_IFMON_CFG_LOCK(unit);

        interval = HAL_SCO_IFMON_INTERVAL(unit);

        if (TRUE != HAL_SCO_IFMON_MONITOR_STATE(unit))
        {
            HAL_SCO_IFMON_CFG_UNLOCK(unit);
            continue;
        }

        rc = _hal_sco_ifmon_processDevState(unit);
        if (AIR_E_OK == rc)
        {
            rc = _hal_sco_ifmon_processLink(unit);
        }

        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, " Ifmon process unit %u rc %d\n", unit, rc);
        }

        HAL_SCO_IFMON_CFG_UNLOCK(unit);

#ifdef AIR_EN_LED_CLK_SYNC
        if (TRUE == do_re_sync)
        {
            re_sync_ms_cnt -= (I32_T)interval;
            if (re_sync_ms_cnt < 0)
            {
                rc = hal_phy_syncLedClock(unit, HAL_PORT_BMP_ETH(unit), 0);

                if (AIR_E_OK == rc)
                {
                    rc = hal_phy_syncWaveGenClock(unit, HAL_PORT_BMP_ETH(unit), 0);
                }

                if (AIR_E_OK == rc)
                {
                    _hal_sco_ifmon_getRandSyncTime(&re_sync_ms_cnt);
                }
            }
        }
#endif /* End of AIR_EN_LED_CLK_SYNC */
    }
}

/* FUNCTION NAME:   _hal_sco_ifmon_initThread
 * PURPOSE:
 *      To initialize thread.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_initThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T ifmon_en;
    AIR_CFG_VALUE_T cfg_pri;
    AIR_CFG_VALUE_T cfg_stack_size;

    osal_memset(&ifmon_en, 0, sizeof(AIR_CFG_VALUE_T));
    ifmon_en.value = FALSE;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_IFMON_ENABLE, &ifmon_en);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    if (FALSE == ifmon_en.value)
    {
        return AIR_E_OK;
    }

    osal_memset(&cfg_pri, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_pri.value = HAL_SCO_IFMON_THREAD_PRI;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_IFMON_THREAD_PRI, &cfg_pri);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    osal_memset(&cfg_stack_size, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_stack_size.value = HAL_SCO_IFMON_STACK_SIZE;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_IFMON_THREAD_STACK, &cfg_stack_size);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = osal_createThread("IFMON", (UI32_T)cfg_stack_size.value, (UI32_T)cfg_pri.value, _hal_sco_ifmon_thread,
                           (void *)((AIR_HUGE_T)unit), &(HAL_SCO_IFMON_THREAD_ID(unit)));

    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_deinitThread
 * PURPOSE:
 *      To deinitialize thread.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_deinitThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T ifmon_en;

    osal_memset(&ifmon_en, 0, sizeof(AIR_CFG_VALUE_T));
    ifmon_en.value = FALSE;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_IFMON_ENABLE, &ifmon_en);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    if (FALSE == ifmon_en.value)
    {
        return AIR_E_OK;
    }

    rc = osal_stopThread(&(HAL_SCO_IFMON_THREAD_ID(unit)));
    rc = osal_destroyThread(&(HAL_SCO_IFMON_THREAD_ID(unit)));
    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_initRsrc
 * PURPOSE:
 *      To initialize IfMon module resource.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_initRsrc(
    const UI32_T unit)
{
    HAL_SCO_IFMON_CB_T *ptr_cb = NULL;
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              size = 0;

    size = sizeof(HAL_SCO_IFMON_CB_T);
    ptr_cb = (HAL_SCO_IFMON_CB_T *)osal_alloc(size, air_module_getModuleName(AIR_MODULE_IFMON));
    HAL_CHECK_PTR(ptr_cb);
    osal_memset(ptr_cb, 0, sizeof(HAL_SCO_IFMON_CB_T));
    _ptr_hal_sco_ifmon_cb[unit] = ptr_cb;
    rc = osal_createSemaphore("IFMON_CONF", AIR_SEMAPHORE_BINARY, &(ptr_cb->sem_conf),
                              air_module_getModuleName(AIR_MODULE_IFMON));
    rc = osal_createSemaphore("IFMON_PORT", AIR_SEMAPHORE_BINARY, &(ptr_cb->sem_portstatus),
                              air_module_getModuleName(AIR_MODULE_IFMON));
    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_deinitRsrc
 * PURPOSE:
 *      To deinitialize IfMon module resource.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_deinitRsrc(
    const UI32_T unit)
{
    HAL_SCO_IFMON_CB_T *ptr_cb = NULL;
    AIR_ERROR_NO_T      rc = AIR_E_OK;

    ptr_cb = _ptr_hal_sco_ifmon_cb[unit];

    rc = osal_destroySemaphore(&(ptr_cb->sem_conf));
    if (AIR_E_OK == rc)
    {
        rc = osal_destroySemaphore(&(ptr_cb->sem_portstatus));
        if (AIR_E_OK == rc)
        {
            osal_free(ptr_cb);
            _ptr_hal_sco_ifmon_cb[unit] = NULL;
        }
    }

    return rc;
}

/* FUNCTION NAME:   _hal_sco_ifmon_initCfg
 * PURPOSE:
 *      To initialize IfMon module default configuration.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_initCfg(
    const UI32_T unit)
{
    HAL_SCO_IFMON_MODE(unit) = HAL_SCO_IFMON_MODE_DFLT;
    HAL_SCO_IFMON_INTERVAL(unit) = HAL_SCO_IFMON_INTERVAL_DFLT;
    HAL_SCO_IFMON_MONITOR_STATE(unit) = TRUE;
    CMLIB_BITMAP_SET(HAL_SCO_IFMON_SCAN_PORT_BITMAP(unit), HAL_PORT_BMP_ETH(unit), AIR_PORT_BITMAP_SIZE);

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_sco_ifmon_allocNotifyHandler
 * PURPOSE:
 *      To allocate notify handler.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      pointer of the handler structure.
 * NOTES:
 *      None
 */
static HAL_SCO_IFMON_NOTIFY_HANDLER_T *
_hal_sco_ifmon_allocNotifyHandler(
    const UI32_T unit)
{
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                          idx = 0;

    for (idx = 0; idx < HAL_SCO_IFMON_NOTIFY_HANDLER_CNT; idx++)
    {
        ptr_handler = PTR_HAL_SCO_IFMON_NOTIFY_HANDLER(unit, idx);
        if (NULL == ptr_handler->notify_func)
        {
            break;
        }
    }

    if (HAL_SCO_IFMON_NOTIFY_HANDLER_CNT != idx)
    {
        DIAG_PRINT(HAL_DBG_INFO, "Alloc notify handler %p\n", ptr_handler);
        return ptr_handler;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "[ERROR %d] Can't alloc notify handler\n", AIR_E_OTHERS);
        return NULL;
    }
}

/* FUNCTION NAME:   _hal_sco_ifmon_freeNotifyHandler
 * PURPOSE:
 *      To free notify handler.
 * INPUT:
 *      unit                --  Device unit number
 *      ptr_handler         --  Notify handler
 * OUTPUT:
 *      None
 * RETURN
 *      None
 * NOTES:
 *      None
 */
static void
_hal_sco_ifmon_freeNotifyHandler(
    const UI32_T                    unit,
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler)
{
    ptr_handler->notify_func = NULL;
    ptr_handler->ptr_cookie = NULL;

    DIAG_PRINT(HAL_DBG_INFO, "Free notify handler %p\n", ptr_handler);
}

/* FUNCTION NAME:   _hal_sco_ifmon_getNotifyHandler
 * PURPOSE:
 *      To get notify handler.
 * INPUT:
 *      unit                --  Device unit number
 *      notify_func         --  Callback function
 *      ptr_cookie          --  Cookie data of callback function
 * OUTPUT:
 *      pptr_handler        --  Notify handler
 * RETURN
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_ifmon_getNotifyHandler(
    const UI32_T                     unit,
    const AIR_IFMON_NOTIFY_FUNC_T    notify_func,
    void                            *ptr_cookie,
    HAL_SCO_IFMON_NOTIFY_HANDLER_T **pptr_handler)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                          idx = 0;

    for (idx = 0; idx < HAL_SCO_IFMON_NOTIFY_HANDLER_CNT; idx++)
    {
        ptr_handler = PTR_HAL_SCO_IFMON_NOTIFY_HANDLER(unit, idx);

        if ((ptr_handler->notify_func == notify_func) && (ptr_handler->ptr_cookie == ptr_cookie))
        {
            *pptr_handler = ptr_handler;
            break;
        }
    }

    if (HAL_SCO_IFMON_NOTIFY_HANDLER_CNT == idx)
    {
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: hal_sco_ifmon_init
 *
 * PURPOSE:
 *      Initialize IfMon function.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE(unit))
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_sco_ifmon_initRsrc(unit);

            if (AIR_E_OK == rc)
            {
                rc = _hal_sco_ifmon_initCfg(unit);
            }
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = _hal_sco_ifmon_initThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) |= HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_ifmon_deinit
 *
 * PURPOSE:
 *      Deinitialize IfMon function.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (!(HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE(unit)))
    {
        rc = AIR_E_NOT_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_sco_ifmon_deinitRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            HAL_SCO_IFMON_MONITOR_STATE(unit) = FALSE;
            rc = _hal_sco_ifmon_deinitThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) &= ~HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_ifmon_lockIfmonResource
 *
 * PURPOSE:
 *      Lock the resource of IfMon.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_lockIfmonResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    return HAL_COMMON_LOCK_RESOURCE(&(HAL_SCO_IFMON_SEM_CONF(unit)), AIR_SEMAPHORE_WAIT_FOREVER);
}

/* FUNCTION NAME: hal_sco_ifmon_unlockIfmonResource
 *
 * PURPOSE:
 *      Unlock the resource of IfMon.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_unlockIfmonResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    return HAL_COMMON_FREE_RESOURCE(&(HAL_SCO_IFMON_SEM_CONF(unit)));
}

/* FUNCTION NAME: hal_sco_ifmon_lockIfmonPortResource
 *
 * PURPOSE:
 *      Lock the resource of IfMon port status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_lockIfmonPortResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    return HAL_COMMON_LOCK_RESOURCE(&(HAL_SCO_IFMON_SEM_PORT(unit)), AIR_SEMAPHORE_WAIT_FOREVER);
}

/* FUNCTION NAME: hal_sco_ifmon_unlockIfmonPortResource
 *
 * PURPOSE:
 *      Unlock the resource of IfMon port status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_unlockIfmonPortResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    return HAL_COMMON_FREE_RESOURCE(&(HAL_SCO_IFMON_SEM_PORT(unit)));
}
/* FUNCTION NAME:   hal_sco_ifmon_register
 * PURPOSE:
 *      To register a callback function to handle a port link change.
 * INPUT:
 *      unit                --  Device unit number
 *      notify_func         --  Callback function
 *      ptr_cookie          --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ifmon_register(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE_TASK_RSRC)
    {
        HAL_SCO_IFMON_CFG_LOCK(unit);

        ptr_handler = _hal_sco_ifmon_allocNotifyHandler(unit);
        if (NULL != ptr_handler)
        {
            ptr_handler->notify_func = notify_func;
            ptr_handler->ptr_cookie = ptr_cookie;
        }
        else
        {
            rc = AIR_E_TABLE_FULL;
        }

        HAL_SCO_IFMON_CFG_UNLOCK(unit);
    }

    DIAG_PRINT(HAL_DBG_INFO, " hal_sco_ifmon_register: unit %u, rc %d\n", unit, rc);

    return rc;
}

/* FUNCTION NAME:   hal_sco_ifmon_deregister
 * PURPOSE:
 *      To deregister a callback function from callback functions.
 * INPUT:
 *      unit                  --  Device unit number
 *      notify_func           --  Callback function
 *      ptr_cookie            --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK              --  Operation is successful.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ifmon_deregister(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE_TASK_RSRC)
    {
        HAL_SCO_IFMON_CFG_LOCK(unit);

        rc = _hal_sco_ifmon_getNotifyHandler(unit, notify_func, ptr_cookie, &ptr_handler);
        if (AIR_E_OK == rc)
        {
            _hal_sco_ifmon_freeNotifyHandler(unit, ptr_handler);
        }

        HAL_SCO_IFMON_CFG_UNLOCK(unit);
    }

    DIAG_PRINT(HAL_DBG_INFO, " hal_sco_ifmon_deregister: unit %u, rc %d\n", unit, rc);

    return rc;
}

/* FUNCTION NAME:   hal_sco_ifmon_setMode
 * PURPOSE:
 *      This API is used to set interface monitor mode, interface monitor
 *      port bitmap and interface monitor interval.
 * INPUT:
 *      unit                --  Device unit number
 *      mode                --  Interface monitor mode
 *      port_bitmap         --  Interface monitor port bitmap
 *      interval            --  Interface monitor polling interval in
 *                              miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      The polling interval is valid if and only if the interface monitor
 *      polling mode is used.
 */
AIR_ERROR_NO_T
hal_sco_ifmon_setMode(
    const UI32_T            unit,
    const AIR_IFMON_MODE_T  mode,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            interval)
{
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    HAL_SCO_IFMON_CFG_LOCK(unit);

    HAL_SCO_IFMON_MODE(unit) = mode;
    HAL_SCO_IFMON_INTERVAL(unit) = interval;
    CMLIB_PORT_BITMAP_SET(HAL_SCO_IFMON_SCAN_PORT_BITMAP(unit), port_bitmap);

    HAL_SCO_IFMON_CFG_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ifmon_getMode
 * PURPOSE:
 *      This API is used to get interface monitor mode, interface monitor
 *      port bitmap and interface monitor interval.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_mode            --  Pointer for interface monitor mode
 *      ptr_port_bitmap     --  Pointer for interface monitor port bitmap
 *      ptr_interval        --  Pointer for interface monitor polling interval
 *                              in miliseconds
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      The polling interval is valid if and only if the interface monitor
 *      polling mode is used.
 */
AIR_ERROR_NO_T
hal_sco_ifmon_getMode(
    const UI32_T       unit,
    AIR_IFMON_MODE_T  *ptr_mode,
    AIR_PORT_BITMAP_T *ptr_port_bitmap,
    UI32_T            *ptr_interval)
{
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    HAL_SCO_IFMON_CFG_LOCK(unit);

    *ptr_mode = HAL_SCO_IFMON_MODE(unit);
    *ptr_interval = HAL_SCO_IFMON_INTERVAL(unit);
    CMLIB_PORT_BITMAP_SET(*ptr_port_bitmap, HAL_SCO_IFMON_SCAN_PORT_BITMAP(unit));

    HAL_SCO_IFMON_CFG_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ifmon_setMonitorState
 * PURPOSE:
 *      To set monitor state.
 * INPUT:
 *      unit                --  Device unit number
 *      enable              --  Monitor state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_setMonitorState(
    const UI32_T unit,
    const BOOL_T enable)
{
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    HAL_SCO_IFMON_CFG_LOCK(unit);

    HAL_SCO_IFMON_MONITOR_STATE(unit) = enable;

    if (TRUE == enable)
    {
        /* after enable, all link up port will be notified once */
        CMLIB_BITMAP_CLEAR(HAL_SCO_IFMON_CUR_LINK_BITMAP(unit), AIR_PORT_BITMAP_SIZE);
    }

    HAL_SCO_IFMON_CFG_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ifmon_getMonitorState
 * PURPOSE:
 *      To get monitor state.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_enable          --  Pointer for monitor state
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_ifmon_getMonitorState(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    HAL_SCO_IFMON_CFG_LOCK(unit);

    *ptr_enable = HAL_SCO_IFMON_MONITOR_STATE(unit);

    HAL_SCO_IFMON_CFG_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ifmon_lookup
 * PURPOSE:
 *      To look up a callback function is registered.
 * INPUT:
 *      unit                  --  Device unit number
 *      notify_func           --  Callback function
 *      ptr_cookie            --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK              --  Operation is successful.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ifmon_lookup(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    HAL_SCO_IFMON_NOTIFY_HANDLER_T *ptr_handler = NULL;

    HAL_CHECK_INIT(unit, AIR_MODULE_IFMON);

    HAL_SCO_IFMON_CFG_LOCK(unit);

    rc = _hal_sco_ifmon_getNotifyHandler(unit, notify_func, ptr_cookie, &ptr_handler);

    HAL_SCO_IFMON_CFG_UNLOCK(unit);

    return rc;
}

/* FUNCTION NAME: hal_sco_ifmon_getPortStatus
 * PURPOSE:
 *      Get the control block link status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_ps          --  AIR_PORT_STATUS_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ifmon_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    AIR_PORT_STATUS_T *ptr_dev_status = NULL;

    ptr_dev_status = PTR_SCO_IFMON_PORT_LINK_STATUS(unit, port);

    HAL_SCO_IFMON_PORT_LOCK(unit); /* lock */
    ptr_ps->speed = ptr_dev_status->speed;
    ptr_ps->duplex = ptr_dev_status->duplex;
    ptr_ps->flags = ptr_dev_status->flags;
    HAL_SCO_IFMON_PORT_UNLOCK(unit); /* unlock */

    return rc;
}
