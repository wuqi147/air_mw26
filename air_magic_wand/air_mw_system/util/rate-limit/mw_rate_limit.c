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

/* FILE NAME:  mw_rate_limit.c
 * PURPOSE:
 *    This file contains the implementation of rate limit functionality for the Magic Wand module.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "mw_rate_limit.h"
#include    "sys_mgmt.h"
#include    "air_dos.h"
#include    "mw_acl.h"

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

/* STATIC VARIABLE DECLARATIONS
 */
#if defined(AIR_SUPPORT_LP) || defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
static MW_PORT_RATE_LIMIT_T *_ptr_lp_stp_port_rate_limit = NULL;
#endif /* AIR_SUPPORT_LP || AIR_SUPPORT_RSTP || AIR_SUPPORT_MSTP */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   mw_rate_limit_init
 * PURPOSE:
 *      This API is used to initialize the mw rate limit module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *      MW_E_NO_MEMORY
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_rate_limit_init(
    void)
{
#if defined(AIR_SUPPORT_LP) || defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    UI8_T           index = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    rc = osapi_calloc(sizeof(MW_PORT_RATE_LIMIT_T) * PLAT_MAX_PORT_NUM, MW_RATE_LIMIT_MODULE_NAME, (void**)&_ptr_lp_stp_port_rate_limit);
    if(MW_E_OK != rc)
    {
        return MW_E_NO_MEMORY;
    }
    for(index = 0; index < PLAT_MAX_PORT_NUM; index++)
    {
        _ptr_lp_stp_port_rate_limit[index].ref_cnt = 0;
        _ptr_lp_stp_port_rate_limit[index].attack_id = MW_ATTACK_ID_INVALID;
    }
#endif /* AIR_SUPPORT_LP || AIR_SUPPORT_RSTP || AIR_SUPPORT_MSTP */

    return MW_E_OK;
}

#if defined(AIR_SUPPORT_LP) || defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* FUNCTION NAME:   mw_rate_limit_lp_stp_port_attack_id_get
 * PURPOSE:
 *      This API is used to get port's attack ID for the loop prevention or stp.
 *
 * INPUT:
 *      unit                 -- Unit id
 *      port                 -- Port id
 *      user                 -  User id
 *
 * OUTPUT:
 *      ptr_attack_id        -- Pointer to the attack ID
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_rate_limit_lp_stp_port_attack_id_get(
    const UI32_T                unit,
    const UI8_T                 port,
    MW_PORT_RATE_LIMIT_USER_T   user,
    UI32_T                      *ptr_attack_id)
{
    I32_T                       rc = MW_E_OP_INCOMPLETE;
    UI32_T                      acl_ap_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T    dos_rate_limit;
    UI32_T                      pkt_thrd;

    if((NULL == ptr_attack_id) || (NULL == _ptr_lp_stp_port_rate_limit) || (PLAT_MAX_PORT_NUM < port))
    {
        return MW_E_BAD_PARAMETER;
    }

    switch (user)
    {
        case MW_PORT_RATE_LIMIT_USER_LP:
            pkt_thrd = LP_PER_PORT_DOS_RATE_LIMIT_PKT_THLD;
            break;
        case MW_PORT_RATE_LIMIT_USER_RSTP:
            pkt_thrd = RSTP_PER_PORT_DOS_RATE_LIMIT_PKT_THLD;
            break;
        default:
            return MW_E_BAD_PARAMETER;
    }

    *ptr_attack_id = MW_ATTACK_ID_INVALID;
    if(0 == _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt)
    {
        rc = MW_ATTACK_ID_GET_AVAILABLERULE(&acl_ap_id, &dos_rate_limit, unit);
        if(MW_E_OK == rc)
        {
            dos_rate_limit.pkt_thld = pkt_thrd;
            dos_rate_limit.time_span = 1000;
            dos_rate_limit.block_time = 1;
            dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
            rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
            if (AIR_E_OK == rc)
            {
                _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt++;
                _ptr_lp_stp_port_rate_limit[port - 1].attack_id = acl_ap_id;
                *ptr_attack_id = acl_ap_id;
                rc = MW_E_OK;
            }
            else
            {
                rc = MW_E_OP_INCOMPLETE;
            }

        }
        else
        {
            rc = MW_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        acl_ap_id = _ptr_lp_stp_port_rate_limit[port - 1].attack_id;
        rc = air_dos_getRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
        if (AIR_E_OK != rc)
        {
            return MW_E_OP_INCOMPLETE;
        }

        if (dos_rate_limit.pkt_thld != pkt_thrd)
        {
            dos_rate_limit.pkt_thld = pkt_thrd;
            rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
            if (AIR_E_OK == rc)
            {
                _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt++;
                *ptr_attack_id = acl_ap_id;
                rc = MW_E_OK;
            }
            else
            {
                rc = MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt++;
            *ptr_attack_id = acl_ap_id;
            rc = MW_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME:   mw_rate_limit_lp_stp_port_attack_clear
 * PURPOSE:
 *      This API is used to clear DoS attack rate limit rule on specific port for loop prevention or stp.
 *
 * INPUT:
 *      unit                 -- Unit id
 *      port                 -- Port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_rate_limit_lp_stp_port_attack_clear(
    const UI32_T    unit,
    const UI8_T     port)
{
    I32_T                       rc = MW_E_OK;
    UI32_T                      acl_ap_id = MW_ACL_ID_INVALID;

    MW_CHECK_PTR(_ptr_lp_stp_port_rate_limit);
    MW_CHECK_MIN_MAX_RANGE(port, 0, PLAT_MAX_PORT_NUM);

    if(1 == _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt)
    {
        acl_ap_id = _ptr_lp_stp_port_rate_limit[port - 1].attack_id;
        if(MW_ACL_ID_INVALID != acl_ap_id)
        {
            air_dos_clearRateLimitCfg(unit, acl_ap_id);
        }
        _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt = 0;
        _ptr_lp_stp_port_rate_limit[port - 1].attack_id = MW_ATTACK_ID_INVALID;
    }
    else if(1 < _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt)
    {
        _ptr_lp_stp_port_rate_limit[port - 1].ref_cnt--;
    }
    else
    {
        rc = MW_E_OTHERS;
    }

    return rc;
}
#endif /* AIR_SUPPORT_LP || AIR_SUPPORT_RSTP || defined(AIR_SUPPORT_MSTP) */
