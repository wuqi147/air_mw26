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

/* FILE NAME:  igmp_snoop_acl.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop ACL.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_acl.h"
#include "igmp_snoop_log.h"
#include "mw_acl.h"
#include "mw_dos.h"
#include "osapi.h"

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: igmp_snp_acl_initVariable
 * PURPOSE:
 *      Initialize the ACL variables.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_acl_initVariable(
    void)
{
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    ptr_igmp_snp->cfg_info.igmp_acl_id = MW_ACL_ID_INVALID;
#ifdef AIR_SUPPORT_PIM_AWARE
    ptr_igmp_snp->cfg_info.pim_acl_id = MW_ACL_ID_INVALID;
#endif

    return MW_E_OK;
}
/* FUNCTION NAME:   igmp_snp_acl_add
 * PURPOSE:
 *      This API is used to add ACL entry for IGMP snooping.
 *
 * INPUT:
 *      vlan_id      --  VLAN ID
 *      ptr_acl_id   --  Pointer to ACL ID
 *      type         --  Type of packet to be filtered
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_acl_add(
    UI16_T      vlan_id,
    UI32_T      *ptr_acl_id,
    IGMP_SNP_ACL_TYPE_T type)
{
    UI32_T              acl_rule_id = MW_ACL_ID_INVALID, acl_ap_id = MW_ACL_ID_INVALID;
    const UI32_T        unit = 0;
    AIR_ACL_RULE_T      acl_rule;
    I32_T               rc;
    AIR_ACL_ACTION_T    action;
    AIR_DOS_RATE_LIMIT_CFG_T dos_rate_limit = {0};
    UI8_T               next_header = IGMP_SNP_IPPROTO_IGMP, port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;

    /* Per vlan acl rule will be supported in the future */
    UNUSED(vlan_id);
    MW_PARAM_CHK((MW_ACL_ID_INVALID != (*ptr_acl_id)), MW_E_OK);

#ifdef AIR_SUPPORT_PIM_AWARE
    if(IGMP_SNP_ACL_TYPE_PIM == type)
    {
        next_header = IGMP_SNP_IPPROTO_PIM;
        port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
    }
#endif /* AIR_SUPPORT_PIM_AWARE */
    if(MW_E_OK == mw_acl_mutex_take())
    {
        mw_dos_setGlobalCfg(unit, TRUE);
        rc = MW_ATTACK_ID_GET_AVAILABLERULE(&acl_ap_id, &dos_rate_limit, unit);
        if(MW_E_OK == rc)
        {
            dos_rate_limit.pkt_thld = AIR_IGMP_RATE_LIMIT_THLD;
            dos_rate_limit.time_span = 1000;
            dos_rate_limit.block_time = 1;
            dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
            rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
            if (AIR_E_OK != rc)
            {
                mw_acl_mutex_release();
                IGMP_SNP_LOG_ERROR("Add IGMP SNP ACL ap-id %d failed, rc=%d!", acl_ap_id, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
        mw_acl_mutex_release();
        if(MW_E_OK != rc)
        {
            MW_CMD_OUTPUT("Get free ACL ap-id failed !\n");
            return MW_E_ENTRY_REACH_END;
        }
    }

    if(MW_E_OK == mw_acl_mutex_take())
    {
        /* Add igmp_snp acl */
        rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
        if (MW_E_OK == rc)
        {
            osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
            acl_rule.rule_en = TRUE;
            AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
            AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
            acl_rule.end = TRUE;
            acl_rule.key.etype = ETHTYPE_IP;
            acl_rule.mask.etype = 0x3;
            acl_rule.key.next_header = next_header;
            acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_NEXT_HEADER_KEY));
            rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
            if (AIR_E_OK == rc)
            {
                osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#ifdef IGMPSNP_DEBUG
                action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id); /* Set mib id as acl rule id. */
                action.field_valid |= (1U << AIR_ACL_MIB);
#endif /* IGMPSNP_DEBUG */
                action.attack_rate_id = (acl_ap_id & 0xFF);
                action.field_valid |= (1U << AIR_ACL_ATTACK);
                action.port_fw = port_fw;
                action.field_valid |= (1U << AIR_ACL_FW_PORT);
                action.pri_user = MW_ACL_RX_PRIORITY_IGMP_SNP;
                action.field_valid |= (1U << AIR_ACL_PRI);
                rc = air_acl_setAction(unit, acl_rule_id, &action);
                if (AIR_E_OK == rc)
                {
                    *ptr_acl_id = acl_rule_id;
                }
                else
                {
                    MW_CMD_OUTPUT("Add IGMP SNP ACL rule entry-id %d action fail, rc=%d.\n", acl_rule_id, rc);
                    air_dos_clearRateLimitCfg(unit, acl_ap_id);
                    air_acl_delRule(unit, acl_rule_id);
                }
            }
            else
            {
                MW_CMD_OUTPUT("Add IGMP SNP ACL rule entry-id %d failed, rc=%d.\n", acl_rule_id, rc);
            }
        }
        else
        {
            MW_CMD_OUTPUT("Get free ACL rule-id for IGMP SNP failed rc:%d\n", rc);
        }

        mw_acl_mutex_release();
        if (AIR_E_OK != rc)
        {
            return rc;
        }
    }

    IGMP_SNP_LOG_DEBUG("Enable IGMP ACL Done, acl_rule_id:%d", acl_rule_id);

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_acl_delete
 * PURPOSE:
 *      This API is used to delete ACL entry for IGMP snooping.
 *
 * INPUT:
 *      vlan_id      --  VLAN ID
 *      ptr_acl_id   --  Pointer to ACL ID
 *      type         --  Type of packet to be filtered
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
igmp_snp_acl_delete(
    UI16_T      vlan_id,
    UI32_T      *ptr_acl_id,
    IGMP_SNP_ACL_TYPE_T type)
{
    UI32_T              acl_rule_id, acl_ap_id = MW_ACL_ID_INVALID;
    const UI32_T        unit = 0;
    AIR_ERROR_NO_T      rc;
    AIR_ACL_ACTION_T    action;

    /* Per vlan acl rule will be supported in the future */
    UNUSED(vlan_id);
    acl_rule_id = *ptr_acl_id;
    MW_PARAM_CHK((FALSE == MW_ACL_ID_IS_VALID(acl_rule_id)), MW_E_BAD_PARAMETER);

    if(MW_E_OK == mw_acl_mutex_take())
    {
        mw_dos_setGlobalCfg(unit, FALSE);
        rc = air_acl_getAction(unit, acl_rule_id, &action);
        if (AIR_E_OK == rc)
        {
            acl_ap_id = action.attack_rate_id;
            if((0 <= acl_ap_id) && (AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID > acl_ap_id))
            {
                air_dos_clearRateLimitCfg(unit, acl_ap_id);
            }
        }

        rc = air_acl_delAction(unit, acl_rule_id);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("Delete ACL action entry-id %d failed, rc=%d.", acl_rule_id, rc);
        }
        rc = air_acl_delRule(unit, acl_rule_id);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("Delete ACL rule entry-id %d failed, rc=%d", acl_rule_id, rc);
        }

        *ptr_acl_id = MW_ACL_ID_INVALID;
        mw_acl_mutex_release();
    }
    return MW_E_OK;
}