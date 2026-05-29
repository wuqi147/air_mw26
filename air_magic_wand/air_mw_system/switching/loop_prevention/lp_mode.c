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

/* FILE NAME:  lp_mode.c
 * PURPOSE:
 *    This file contains the implementation of mode for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_mode.h"
#include    "lp_log.h"
#include    "lp_led.h"
#include    "lp_acl.h"
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
#include    "lp_hw.h"
#endif
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
#include    "lp_config_customer.h"
#endif
#ifdef LP_MW_SUPPORT
#include    "lp_db.h"
#endif

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
static
MW_ERROR_NO_T
_lp_updateState(
    BOOL_T state);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _lp_updateState
 * PURPOSE:
 *        Update the state of the loop prevention.
 *
 * INPUT:
 *      state            --  state to be updated
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
static
MW_ERROR_NO_T
_lp_updateState(
    BOOL_T state)
{
    UI16_T              u_portId = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return MW_E_OP_INCOMPLETE;
    }
    if (TRUE == state)
    {
        ptr_lp_config->timestamp = 0;
        lp_acl_set();
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
        lp_hw_sendLdf(TRUE);
#endif
    }
    else if (FALSE == state)
    {
        lp_acl_remove();
        ptr_lp_config->timestamp = 0;
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
        lp_hw_sendLdf(FALSE);
#endif
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
        {
            if (AIR_PORT_CHK(ptr_lp_config->looped_pbmp, u_portId))
            {
                AIR_PORT_DEL(ptr_lp_config->looped_pbmp, u_portId);
                lp_port_delFromLoopPbmpGroup(u_portId);
                ptr_lp_config->loop_timer[u_portId - 1] = 0;
                if (FALSE == AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))
                {
#ifdef LP_LOOP_DECTECTION_SUPPORT
                    if (MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin)
                    {
                        lp_port_setFlowControl(FALSE, u_portId);
                    }
#endif /* LP_LOOP_DECTECTION_SUPPORT */
#ifdef LP_MW_SUPPORT
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, u_portId=%d, LP_NORMAL)", u_portId);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, u_portId, sizeof(UI8_T), LP_NORMAL);
#endif /* LP_MW_SUPPORT */
                }
            }
            if (AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, u_portId))
            {
                AIR_PORT_DEL(ptr_lp_config->loop_led_pbmp, u_portId);
                lp_led_setBehavior(FALSE, u_portId);
            }
            if (AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId))
            {
                AIR_PORT_DEL(ptr_lp_config->blocked_pbmp, u_portId);
                ptr_lp_config->block_timer[u_portId - 1] = 0;
                lp_port_delFromBlockPbmpGroup(u_portId);
#ifdef LP_MW_SUPPORT
                LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, u_portId=%d, LP_NORMAL)", u_portId);
                lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, u_portId, sizeof(UI8_T), LP_NORMAL);
#endif /* LP_MW_SUPPORT */
                LP_LOG_DEBUG("->lp_port_block(FALSE, u_portId=%d)", u_portId);
                /* Set port-matrix to enable packet.PortID RX/TX */
                lp_port_block(FALSE, u_portId);
            }
        }
#ifdef LP_GLOBAL_LED_SUPPORT
        lp_led_updateGlobalLed(ptr_lp_config->loop_led_pbmp);
#endif
    }
    else
    {
        LP_LOG_DEBUG("Invalid state - %d", state);
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   lp_updateAdminMode
 * PURPOSE:
 *        Update the mode of the loop prevention.
 *
 * INPUT:
 *      mode             --  mode to be updated
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_updateAdminMode(
    MW_LP_MODE_T mode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if ((NULL == ptr_lp_config) || (mode == ptr_lp_config->lp_admin))
    {
        return MW_E_OP_INCOMPLETE;
    }
    rc = _lp_updateState(FALSE);
    switch (mode)
    {
        case MW_LP_MODE_DISABLE:
            ptr_lp_config->lp_admin = MW_LP_MODE_DISABLE;
            LP_LOG_DEBUG("Loop Mode update to DISABLE");
            break;

        case MW_LP_MODE_PREVENTION:
            ptr_lp_config->lp_admin = MW_LP_MODE_PREVENTION;
            ptr_lp_config->looped_port_blink = FALSE;
            rc |= _lp_updateState(TRUE);
            LP_LOG_DEBUG("Loop Mode update to PREVENTION");
            break;

#ifdef LP_LOOP_DECTECTION_SUPPORT
        case MW_LP_MODE_DETECTION:
            ptr_lp_config->lp_admin = MW_LP_MODE_DETECTION;
            ptr_lp_config->looped_port_blink = TRUE;
            rc |= _lp_updateState(TRUE);
            LP_LOG_DEBUG("Loop Mode update to DETECTION");
            break;

#endif /* LP_LOOP_DECTECTION_SUPPORT */

        default:
            rc = MW_E_BAD_PARAMETER;
            LP_LOG_ERROR("Loop Mode update failed: Invalid mode %d", mode);
            break;
    }

    return rc;
}

#ifdef LP_CUSTOMER_CONFIG_SUPPORT
/* FUNCTION NAME:   lp_setDefaultCfgMode
 * PURPOSE:
 *        Set the mode for the loop prevention with default configuration.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_setDefaultCfgMode(
    void)
{
    MW_LP_MODE_T lp_mode = MW_LP_MODE_DISABLE;
    MW_ERROR_NO_T ret = MW_E_OK;

    ret = lp_getDefaultCfg(&lp_mode);
    if (MW_E_OK != ret)
    {
        return MW_E_OP_INCOMPLETE;
    }

    ret = lp_updateAdminMode(lp_mode);

    return ret;
}

/* FUNCTION NAME:   lp_set_dipSwitchCfgMode
 * PURPOSE:
 *        Set the mode for the loop prevention with the configuration from the dip switch.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_set_dipSwitchCfgMode(
    void)
{
    MW_LP_MODE_T lp_mode = MW_LP_MODE_DISABLE;
    MW_ERROR_NO_T ret = MW_E_OK;

    ret = lp_getDipSwitchCfg(&lp_mode);
    if (MW_E_OK != ret)
    {
        return MW_E_OP_INCOMPLETE;
    }

    ret = lp_updateAdminMode(lp_mode);

    return ret;
}
#endif /* LP_CUSTOMER_CONFIG_SUPPORT */
