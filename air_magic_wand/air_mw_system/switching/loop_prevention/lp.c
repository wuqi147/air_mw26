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

/* FILE NAME:   lp.c
 * PURPOSE:
 *      Define loop prevention function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "lp.h"
#include "lp_log.h"
#include "lp_queue.h"
#include "lp_timer.h"
#include "lp_led.h"
#include "lp_acl.h"
#ifdef LP_MW_SUPPORT
#include "lp_db.h"
#endif
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
#include "lp_config_customer.h"
#endif
#include "lp_hw.h"
#include "lp_utils.h"
#include "mw_utils.h"
#include "osapi_thread.h"
#include "osapi_message.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void
_lp_task(
    void *ptr_pvParameters);

/* STATIC VARIABLE DECLARATIONS
 */
static threadhandle_t         _ptr_lpd = NULL;
static StackType_t            _lp_task_stack[LP_STACK_SIZE] = { 0 };
static StaticTask_t           _lp_task_tcb;
static LP_CONFIG_INFO_T       _lp_config;

/* LOCAL SUBPROMGRAM BODIES
*/
/* FUNCTION NAME:   _lp_task
 * PURPOSE:
 *      lp task function.
 *
 * INPUT:
 *      ptr_pvParameters
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
static void
_lp_task(
    void *ptr_pvParameters)
{
    MW_ERROR_NO_T       xRet = MW_E_OK;
    MW_MSG_T *ptr_msg = NULL;

    /* Just to kill the compiler warning. */
    (void)ptr_pvParameters;
    lp_led_init();
#ifdef LP_MW_SUPPORT
    lp_db_subscribeModule();
#endif
    do
    {
        xRet = mac_rcv_readyGet();
    } while (0 != xRet);

    while (1)
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        lp_timer_handleExpirationEvent();
        xRet = osapi_msgRecv(LP_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, 100);
        if (MW_E_OK == xRet)
        {

            if (MW_MSG_ID_ETHERNET_PBUF == ptr_msg->msg_id)
            {
                NET_MSG_T *ptr_pkt_msg = (NET_MSG_T *)ptr_msg;

                if (NULL != ptr_pkt_msg->ptr_pbuf)
                {
                    lp_process_rxPacket(ptr_pkt_msg->ptr_pbuf);
                }
            }
#ifdef LP_MW_SUPPORT
            else if (MW_MSG_ID_DB == ptr_msg->msg_id)
            {
                lp_db_handleMsg((DB_MSG_T *)ptr_msg);
            }
#endif
            else
            {
                /* unknown notification */
                LP_LOG_DEBUG("Receive unknown notification msg_id:%d", ptr_msg->msg_id);
            }
            MW_FREE(ptr_msg);
        }
    }

    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: lp_init
 * PURPOSE:
 *      Start loop prevention function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_init(
    void)
{
    MW_ERROR_NO_T xRet = MW_E_OK;

    if (NULL != _ptr_lpd)
    {
        return MW_E_ALREADY_INITED;
    }

    MW_LOG_INIT_PRINTF("Initializing loop prevention...\n");
    osapi_memset(&_lp_config, 0, sizeof(_lp_config));
    do
    {
        xRet = osapi_calloc(sizeof(UI8_T) * PLAT_MAX_PORT_NUM, LP_MODULE_NAME, (void **)&(_lp_config.ptr_fc_resume_timer));
        if (MW_E_OK != xRet)
        {
            break;
        }
        xRet = osapi_calloc(sizeof(AIR_PORT_BITMAP_T) * PLAT_MAX_PORT_NUM, LP_MODULE_NAME, (void **)&(_lp_config.ptr_to_blk_pbmp));
        if (MW_E_OK != xRet)
        {
            break;
        }
        xRet = osapi_calloc(sizeof(AIR_PORT_BITMAP_T) * PLAT_MAX_PORT_NUM, LP_MODULE_NAME, (void **)&(_lp_config.ptr_to_loop_pbmp));
        if (MW_E_OK != xRet)
        {
            break;
        }
        xRet = lp_queue_init();
        if (MW_E_OK != xRet)
        {
            break;
        }

        /* Create task for loop prevention */
        xRet = osapi_threadCreateStatic(LP_TASK_NAME,
                                        LP_STACK_SIZE,
                                        MW_TASK_PRIORITY_LP,
                                        _lp_task,
                                        NULL,
                                        _lp_task_stack,
                                        &_lp_task_tcb,
                                        &_ptr_lpd);
        if (MW_E_OK != xRet)
        {
            break;
        }

        /* Create timer */
        xRet = lp_timer_init();
        if (MW_E_OK != xRet)
        {
            break;
        }
        xRet = lp_timer_start();
        if (MW_E_OK != xRet)
        {
            LP_LOG_ERROR("start loop prevent timer fail");
            break;
        }
    }while (0);

    if (MW_E_OK != xRet)
    {
        LP_LOG_ERROR("LP init failed, ret:%d", xRet);
        lp_deinit();
        return MW_E_NOT_INITED;
    }

    lp_acl_initVariable();

#ifdef LP_CUSTOMER_CONFIG_SUPPORT
    MW_LP_MODE_T lp_mode = MW_LP_MODE_DISABLE;

    if (MW_E_OK != lp_getDefaultCfg(&lp_mode))
    {
        /* Default work in prevent mode */
        lp_updateAdminMode(MW_LP_MODE_PREVENTION);
    }
    else
    {
        lp_updateAdminMode(lp_mode);
    }
#endif

    return xRet;
}

/* FUNCTION NAME:   lp_deinit
 * PURPOSE:
 *      Free the resources in lp init function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_deinit(void)
{
    MW_FREE(_lp_config.ptr_fc_resume_timer);
    MW_FREE(_lp_config.ptr_to_blk_pbmp);
    MW_FREE(_lp_config.ptr_to_loop_pbmp);
    lp_queue_deinit();
    lp_timer_deinit();
    if (NULL != _ptr_lpd)
    {
        osapi_threadDelete(_ptr_lpd);
        _ptr_lpd = NULL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: lp_getConfig
 * PURPOSE:
 *      Get the loop prevention configuration
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to the loop prevention configuration
 * NOTES:
 *      None
 */
LP_CONFIG_INFO_T *
lp_getConfig(
    void)
{
    return &_lp_config;
}
