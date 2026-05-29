/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  lp_timer.c
 * PURPOSE:
 *    This file contains the implementation of timer for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_log.h"
#include    "lp_timer.h"
#ifdef LP_MW_SUPPORT
#include    "lp_db.h"
#endif
#include    "lp_led.h"
#include    "osapi_timer.h"

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
_lp_tmr(
    timehandle_t ptr_xTimer);

/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T            _lp_timer_expired_flag = FALSE;
static timehandle_t     _ptr_lp_time = NULL;

/* LOCAL SUBPROGRAM BODIES
 */
static void
_lp_tmr(
    timehandle_t ptr_xTimer)
{
    if (FALSE == _lp_timer_expired_flag)
    {
        _lp_timer_expired_flag = TRUE;
    }
    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   lp_timer_init
 * PURPOSE:
 *      This API is used to initial lp software timer.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_timer_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    rc = osapi_timerCreate(LP_TIMER_NAME, _lp_tmr, TRUE, LP_TIMER_PERIOD, NULL, &_ptr_lp_time);
    if (MW_E_OK != rc)
    {
        /* Error create timer */
        LP_LOG_ERROR("Error: create lp timer fail");
        return MW_E_NOT_INITED;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   lp_timer_deinit
 * PURPOSE:
 *      This API is used to de-initial lp software timer.
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
lp_timer_deinit(
    void)
{
    if (NULL != _ptr_lp_time)
    {
        if(MW_E_OK == osapi_timerActive(_ptr_lp_time))
        {
            osapi_timerStop(_ptr_lp_time);
        }
        osapi_timerDelete(_ptr_lp_time);
        _ptr_lp_time = NULL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   lp_timer_start
 * PURPOSE:
 *      This API is used to start lp software timer.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_timer_start(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL == _ptr_lp_time)
    {
        return MW_E_OTHERS;
    }
    if(MW_E_OK != osapi_timerActive(_ptr_lp_time))
    {
        rc = osapi_timerStart(_ptr_lp_time);
    }
    return rc;
}

/* FUNCTION NAME:   lp_timer_handleExpirationEvent
 * PURPOSE:
 *      This API is used to check the timeout status of the loop prevention.
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
void
lp_timer_handleExpirationEvent(
    void)
{
    UI32_T              u_portId = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if ((FALSE == _lp_timer_expired_flag) || (NULL == ptr_lp_config) || (MW_LP_MODE_DISABLE == ptr_lp_config->lp_admin))
    {
        return;
    }
    _lp_timer_expired_flag = FALSE;
    ptr_lp_config->timestamp++;
#ifndef AIR_HARDWARE_SEND_LDF_SUPPORT
    lp_sendLdf();
#endif

    if (0 == (ptr_lp_config->timestamp % LP_LDF_TIMESTAPE_VALID_TIME))
    {
        LP_LOG_DEBUG("timestamp: %d, block_port_bitmap: 0x%x, loop_detect_bitmap: 0x%x, loop_led_pbmp: 0x%x."
            , ptr_lp_config->timestamp, ptr_lp_config->blocked_pbmp[0], ptr_lp_config->looped_pbmp[0], ptr_lp_config->loop_led_pbmp[0]);
    }

    /* RECOVER: 1. The blocked port doesn't receive loop detection packet after (3*TX_interval) */
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        if(0 < ptr_lp_config->ptr_fc_resume_timer[(u_portId - 1)])
        {
            ptr_lp_config->ptr_fc_resume_timer[(u_portId - 1)]--;
            if(0 == ptr_lp_config->ptr_fc_resume_timer[(u_portId - 1)])
            {
#ifdef LP_LOOP_DECTECTION_SUPPORT
                if((((MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin)) && (FALSE == AIR_PORT_CHK(ptr_lp_config->looped_pbmp, u_portId))) ||
                   (((MW_LP_MODE_PREVENTION == ptr_lp_config->lp_admin)) && (FALSE == AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))))
#else
                if(FALSE == AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))
#endif
                {
                    /* recover flow control */
                    lp_port_setFlowControl(FALSE, u_portId);
                }
            }
        }
        if (ptr_lp_config->loop_timer[u_portId - 1] > 0)
        {
            ptr_lp_config->loop_timer[u_portId - 1]--;
            /* RECOVER: 2. The blocked port link-down loop status. */
            if (FALSE == lp_lag_getLinkStatus(u_portId))
            {
                ptr_lp_config->loop_timer[u_portId - 1] = 0;
            }

            if ((0 == ptr_lp_config->loop_timer[u_portId - 1]))
            {
                AIR_PORT_DEL(ptr_lp_config->looped_pbmp, u_portId);
                lp_port_delFromLoopPbmpGroup(u_portId);
                if (TRUE == ptr_lp_config->looped_port_blink)
                {
                    if (TRUE == AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, u_portId))
                    {
                        lp_led_setBehavior(FALSE, u_portId);
                        AIR_PORT_DEL(ptr_lp_config->loop_led_pbmp, u_portId);
                    }
                }
#ifdef LP_LOOP_DECTECTION_SUPPORT
                if (MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin)
                {
                    lp_port_setFlowControl(FALSE, u_portId);
                }
#endif /* LP_LOOP_DECTECTION_SUPPORT */
#ifdef LP_MW_SUPPORT
                if (FALSE == AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))
                {
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, u_portId=%d, LP_NORMAL)", u_portId);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, u_portId, sizeof(UI8_T), LP_NORMAL);
                }
#endif /* LP_MW_SUPPORT */
            }
            else
            {
                if (TRUE == ptr_lp_config->looped_port_blink)
                {
                    lp_led_checkPortState(u_portId);
                }
            }
        }

        if (ptr_lp_config->block_timer[u_portId - 1] > 0)
        {
            ptr_lp_config->block_timer[u_portId - 1]--;

            /* RECOVER: 2. The blocked port link-down loop status. */
            if (FALSE == lp_lag_getLinkStatus(u_portId))
            {
                ptr_lp_config->block_timer[u_portId - 1] = 0;
            }

            if (0 == ptr_lp_config->block_timer[u_portId - 1])
            {
                AIR_PORT_DEL(ptr_lp_config->loop_led_pbmp, u_portId);
                lp_led_setBehavior(FALSE, u_portId);
                if (AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))
                {
                    AIR_PORT_DEL(ptr_lp_config->blocked_pbmp, u_portId);
                    lp_port_delFromBlockPbmpGroup(u_portId);
                    lp_port_block(FALSE, u_portId);
#ifdef LP_MW_SUPPORT
                    UI8_T port_state = LP_NORMAL;

                    if (TRUE == MW_PORT_CHK(ptr_lp_config->looped_pbmp, u_portId))
                    {
                        /* A port may be in loop and block state at the same time */
                        port_state = LP_LOOP;
                    }
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, u_portId=%d, %s)", u_portId, port_state == LP_LOOP ? "LP_LOOP" : "LP_NORMAL");
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, u_portId, sizeof(UI8_T), port_state);
#endif
                    LP_LOG_DEBUG("->lp_port_block(FALSE, u_portId=%d)", u_portId);
                }
            }
            else
            {
                lp_led_checkPortState(u_portId);
            }
        }
    }

#ifdef LP_GLOBAL_LED_SUPPORT
    lp_led_updateGlobalLed(ptr_lp_config->loop_led_pbmp);
#endif

    return;
}
