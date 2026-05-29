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

/* FILE NAME:  syncd.c
 * PURPOSE:
 *  Implement synchronization daemon which synchronizes data between SDK and middleware.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include "osapi_thread.h"
#include "osapi_message.h"
#include "sys_mgmt.h"
#include "syncd.h"
#include "syncd_in.h"
#include "syncd_opts.h"
#include "syncd_timer.h"
#include "syncd_msg.h"
#include "air_ifmon.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define MW_SYNCD_PORT_SETTINGS_ADMIN_STATE    (1UL)
#define MW_SYNCD_PORT_SETTINGS_LP_BLOCK       (1UL << 1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static void
_syncd_port_link_change_handle(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie);

/* STATIC VARIABLE DECLARATIONS
 */
GDMPSRAM_BSS_EX static SYNCD_CFG_T _syncd_cfg;
static volatile UI32_T _link_flag = FALSE;


/* LOCAL SUBPROGRAM BODIES
 */
static void
_syncd_cfg_free(
    SYNCD_CFG_T *ptr_cfg)
{
    threadhandle_t self_handle = NULL;

    if (NULL != ptr_cfg)
    {
        /* process */
        if (ptr_cfg->proc_handle)
        {
            self_handle = ptr_cfg->proc_handle;
        }

        /* timer */
        syncd_timer_free(ptr_cfg);

        /* API */
        syncd_api_free(ptr_cfg);

        /* Message queue */
        syncd_queue_free();

        /* ptr_cfg */
        if (NULL != ptr_cfg->ptr_port)
        {
            MW_FREE(ptr_cfg->ptr_port);
        }

        /* Delete process */
        if (NULL != self_handle)
        {
            osapi_processDelete(self_handle);
        }

        air_ifmon_deregister(0, _syncd_port_link_change_handle, NULL);
    }
}

static SYNCD_CFG_T *
_syncd_cfg_init(
    void)
{
    SYNCD_CFG_T *ptr_cfg = NULL;
    MW_ERROR_NO_T rc;

    ptr_cfg = &_syncd_cfg;

    rc = osapi_calloc(
            sizeof(SYNCD_PORT_T) * PLAT_MAX_PORT_NUM,
            SYNCD_NAME,
            (void **)&ptr_cfg->ptr_port);
    if (MW_E_OK != rc)
    {
        _syncd_cfg_free(ptr_cfg);
        return NULL;
    }

    return ptr_cfg;
}

static void
_syncd_port_link_change_handle(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    SYNCD_LOG_DEBUG("ifmon notify port %d link %d", port, link);
    _link_flag = TRUE;
}

static inline MW_ERROR_NO_T
_syncd_handlePortStatus(
    SYNCD_CFG_T *ptr_cfg)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if (TRUE == _link_flag)
    {
        _link_flag = FALSE;
        ret = syncd_port_status_handle(ptr_cfg);
    }

    return ret;
}

static void
_syncd_handleTaskMsg(
    MW_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if (NULL != ptr_msg)
    {
        switch (ptr_msg->msg_id)
        {
            /* Invoke the message handlers here. */
            case MW_MSG_ID_SYNCD_TIMER_BASE_TIMER_EXPIRED_NOTI:
            {
                rc = _syncd_handlePortStatus(&_syncd_cfg);
                if (MW_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Handle port status msg failed, rc:%d", rc);
                }
                syncd_stp_flush_fdb_handle();
                syncd_port_matrix_resume_handle();
                break;
            }

            case MW_MSG_ID_SYNCD_TIMER_MIB_COUNTER_TIMER_EXPIRED_NOTI:
            {
                rc = syncd_mib_counter_msg_handle();
                if (MW_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("handle MIB counter msg failed, rc:%d", rc);
                }
                break;
            }

#ifdef SYNCD_TEST_ENABLE
            case MW_MSG_ID_SYNCD_SYNCD_TEST_REQ:
            {
                syncd_queue_test_handleTestMsg((SYNCD_MSG_TEST_REQ_T *)ptr_msg);
                break;
            }
#endif

            default:
            {
                break;
            }
        }

        MW_FREE(ptr_msg);
    }
    else
    {
        SYNCD_LOG_ERROR("Receive message queue failed!(%d)", rc);
    }
}

static void
_syncd_handleDBMsg(
    SYNCD_CFG_T *ptr_cfg,
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T i = 0;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;

    if (NULL != ptr_msg)
    {
        SYNCD_LOG_DEBUG("ptr_msg=%p method=0x%X", ptr_msg, ptr_msg ->method);
        SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Receive");
        if (0 != (ptr_msg->method & M_B_RESPONSE))
        {
            /* Process Response message. Do nothing currently. */
            SYNCD_LOG_DEBUG("Response message");
        }
        else
        {
            /* Process the notification message */
            do {
                rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
                if (MW_E_OK == rc)
                {
                    SYNCD_LOG_DEBUG("index=%u\nptr_payload=%p\nt_idx=%u\nf_idx=%u\ne_idx=%u\ndata_size=%u",
                                i++, ptr_data, request.t_idx, request.f_idx, request.e_idx, data_size);
                    rc = syncd_api_process(ptr_cfg,
                                           ptr_msg->method,
                                           &request,
                                           data_size,
                                           ptr_data);
                    if (MW_E_OK != rc)
                    {
                        SYNCD_LOG_ERROR("Process API function failed!(%d)", rc);
                    }
                }
                /* Continue to parse the next request within the payload. */
            } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
        }
        SYNCD_LOG_DEBUG("ptr_msg=%p", ptr_msg);
        /* Free message buffer */
        MW_FREE(ptr_msg);
    }
    else
    {
        SYNCD_LOG_ERROR("Receive message queue failed!(%d)", rc);
    }
}

static void
_syncd_thread(
    void *arg)
{
    MW_ERROR_NO_T rc;
    SYNCD_CFG_T *ptr_cfg = (SYNCD_CFG_T *)arg;
    MW_MSG_T *ptr_msg = NULL;

    /* Check DB is ready */
    do{
        rc = dbapi_dbisReady();
        if (MW_E_OK != rc)
        {
            SYNCD_LOG_DEBUG("DB is not ready");
            osapi_delay(SYNCD_DELAY_TIME);
        }
        else
        {
            SYNCD_LOG_DEBUG("DB is ready");
            break;
        }
    }while(1);

    /* Subscribe API items to DB */
    rc = syncd_api_subscribe(ptr_cfg, M_SUBSCRIBE);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Subscribe API items failed!");
        _syncd_cfg_free(ptr_cfg);
        return;
    }

    /* Rx thread */
    while(1)
    {
        rc = syncd_queue_recv(SYNCD_MSG_QUEUE_NAME, (UI8_T **)&ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
        if (MW_E_OK == rc)
        {
            switch (ptr_msg->msg_id)
            {
                case MW_MSG_ID_DB:
                {
                    _syncd_handleDBMsg(ptr_cfg, (DB_MSG_T *)ptr_msg);
                    break;
                }
                default:
                {
                    _syncd_handleTaskMsg(ptr_msg);
                    break;
                }
            }
        }
    }

    SYNCD_LOG_DEBUG("Out of thread");

    /* process suicide in _syncd_cfg_free() */
    _syncd_cfg_free(ptr_cfg);
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_init
 * PURPOSE:
 *      Initialization of syncd daemon.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void syncd_init(void)
{
    MW_ERROR_NO_T rc;
    SYNCD_CFG_T  *ptr_cfg = NULL;
    UI32_T        unit = 0;

    /* Initialize global configuration */
    ptr_cfg = _syncd_cfg_init();
    if (NULL == ptr_cfg)
    {
        SYNCD_LOG_ERROR("Initialize global configuration failed");
        return;
    }

    /* Initialize queue */
    rc = syncd_queue_init();
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Initialize queue failed! rc:%d", rc);
        _syncd_cfg_free(ptr_cfg);
        return;
    }

    /* Initialize API table */
    rc = syncd_api_init(ptr_cfg);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Initialize API table failed!");
        _syncd_cfg_free(ptr_cfg);
        return;
    }

    /* Initialize timer */
    rc = syncd_timer_init(ptr_cfg);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Initialize timer failed!");
        _syncd_cfg_free(ptr_cfg);
        return;
    }

    /* Ifmon register */
    if (AIR_E_OK != air_ifmon_register(unit, _syncd_port_link_change_handle, NULL))
    {
        SYNCD_LOG_ERROR("Ifmon register failed!");
        _syncd_cfg_free(ptr_cfg);
        return;
    }

    /* Create new thread to receive packet */
    rc = osapi_threadCreateStatic(SYNCD_NAME,
                                  SYNCD_STACK_SIZE,
                                  MW_TASK_PRIORITY_SYNCD,
                                  _syncd_thread,
                                  (void *)ptr_cfg,
                                  _syncd_cfg.stack_buffer,
                                  &_syncd_cfg.task_buffer,
                                  &_syncd_cfg.proc_handle);
    if (MW_E_OK == rc)
    {
        SYNCD_LOG_DEBUG("syncd thread created!");
        SYNCD_LOG_DEBUG("proc_handle = %p", ptr_cfg ->proc_handle);
    }
    else
    {
        _syncd_cfg_free(ptr_cfg);
        SYNCD_LOG_ERROR("Create syncd thread failed!");
    }
}

/* FUNCTION NAME: syncd_api_setPortAdmin
 * PURPOSE:
 *      Set admin status of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *      admin           --  port admin status
 *
 * OUTPUT:
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_setPortAdmin(
    const UI8_T p_idx,
    const BOOL_T admin)
{
    MW_PARAM_CHK((p_idx >= PLAT_MAX_PORT_NUM), MW_E_BAD_PARAMETER);
    MW_CHECK_BOOL(admin, MW_E_BAD_PARAMETER);

    if (TRUE == admin)
    {
        _syncd_cfg.ptr_port[p_idx].port_settings |= MW_SYNCD_PORT_SETTINGS_ADMIN_STATE;
    }
    else
    {
        _syncd_cfg.ptr_port[p_idx].port_settings &= ~MW_SYNCD_PORT_SETTINGS_ADMIN_STATE;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_getPortAdmin
 * PURPOSE:
 *      Get admin status of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *
 * OUTPUT:
 *      ptr_admin       --  port admin status
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_getPortAdmin(
    const UI8_T p_idx,
    BOOL_T * const ptr_admin)
{
    MW_PARAM_CHK((p_idx >= PLAT_MAX_PORT_NUM), MW_E_BAD_PARAMETER);
    MW_CHECK_PTR(ptr_admin);

    *ptr_admin = (_syncd_cfg.ptr_port[p_idx].port_settings & MW_SYNCD_PORT_SETTINGS_ADMIN_STATE)?TRUE:FALSE;
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_setPortLpBlock
 * PURPOSE:
 *      Set loop prevention blocking state of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *      lp_block        --  Loop prevention blocking state
 *
 * OUTPUT:
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_setPortLpBlock(
    const UI8_T p_idx,
    const BOOL_T lp_block)
{
    MW_PARAM_CHK((p_idx >= PLAT_MAX_PORT_NUM), MW_E_BAD_PARAMETER);
    MW_CHECK_BOOL(lp_block, MW_E_BAD_PARAMETER);

    if (TRUE == lp_block)
    {
        _syncd_cfg.ptr_port[p_idx].port_settings |= MW_SYNCD_PORT_SETTINGS_LP_BLOCK;
    }
    else
    {
        _syncd_cfg.ptr_port[p_idx].port_settings &= ~MW_SYNCD_PORT_SETTINGS_LP_BLOCK;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_getPortLpBlock
 * PURPOSE:
 *      Get loop prevention blocking state of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *
 * OUTPUT:
 *      ptr_lp_block    --  Loop prevention blocking state
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_getPortLpBlock(
    const UI8_T p_idx,
    BOOL_T * const ptr_lp_block)
{
    MW_PARAM_CHK((p_idx >= PLAT_MAX_PORT_NUM), MW_E_BAD_PARAMETER);
    MW_CHECK_PTR(ptr_lp_block);

    *ptr_lp_block = (_syncd_cfg.ptr_port[p_idx].port_settings & MW_SYNCD_PORT_SETTINGS_LP_BLOCK)?TRUE:FALSE;
    return MW_E_OK;
}

