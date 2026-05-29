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

/* FILE NAME:  lp_acl.c
 * PURPOSE:
 *    This file contains the implementation of ACL for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_log.h"
#include    "lp_acl.h"
#include    "lp_hw.h"
#include    "mw_acl.h"
#ifdef LP_MW_SUPPORT
#include    "mw_rate_limit.h"
#endif
#include    "air_dos.h"
#include    "air_sec.h"
#include    "air_swc.h"
#include    "osapi.h"
#include    "mw_dos.h"

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
static MW_ERROR_NO_T
_lp_acl_setPortAttackPrevention(
    UI32_T unit,
    UI16_T port_id,
    UI32_T *ptr_attack_id);

static MW_ERROR_NO_T
lp_acl_setMacComparationUdf(
    void);

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef AIR_LP_USE_STP_BLOCK
UI16_T               _lp_acl_ldf_asbpdu_id = MW_ACL_ID_INVALID;
#endif
UI16_T               _lp_acl_id[MAX_PORT_NUM];
UI16_T               _lp_acl_udf_id = MW_ACL_ID_INVALID;
UI16_T               _lp_mac_acl_id[LP_MAC_ACL_NUM];
UI16_T               _lp_mac_acl_udf_id[LP_MAC_UDF_NUM];

#ifndef LP_MW_SUPPORT
UI16_T               _lp_acl_attack_prevention_id[MAX_PORT_NUM];
#endif
UI16_T               _lp_generic_acl_attack_prevention_id[MAX_PORT_NUM];
UI16_T               _lp_generic_acl_id[MAX_PORT_NUM];

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_lp_acl_setPortAttackPrevention(
    UI32_T unit,
    UI16_T port_id,
    UI32_T *ptr_attack_id)
{
    UI32_T acl_ap_id = 0;
    I32_T  rc = MW_E_OK;
    AIR_DOS_RATE_LIMIT_CFG_T dos_rate_limit;

    MW_CHECK_PTR(ptr_attack_id);
    *ptr_attack_id = acl_ap_id;
    rc = MW_ATTACK_ID_GET_AVAILABLERULE(&acl_ap_id, &dos_rate_limit, unit);
    if (MW_E_OK == rc)
    {
        dos_rate_limit.pkt_thld = 40;
        dos_rate_limit.time_span = 1000;
        dos_rate_limit.block_time = 1;
        dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
        rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
        if (AIR_E_OK == rc)
        {
            *ptr_attack_id = acl_ap_id;
            rc = MW_E_OK;
        }
        else
        {
            rc = MW_E_OP_INCOMPLETE;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
lp_acl_setMacComparationUdf(
    void)
{
    UI32_T             unit = 0, val = 0;
    UI8_T              index;
    AIR_MAC_T          mac;
    UI16_T             udf_cmp;
    I32_T              rc;
    UI8_T              acl_udf_id = 0;
    AIR_ACL_UDF_RULE_T acl_udf_rule;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        LP_LOG_ERROR("Get sys-mac error");
        return MW_E_BAD_PARAMETER;
    }
    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (index = 0; index < LP_MAC_UDF_NUM; index++)
        {
            if (MW_ACL_ID_INVALID != _lp_mac_acl_udf_id[index])
            {
                continue;
            }

            for (acl_udf_id = 0; acl_udf_id < AIR_MAX_NUM_OF_UDF_ENTRY; acl_udf_id++)
            {
                if ((air_acl_getUdfRule(unit, acl_udf_id, &acl_udf_rule) == AIR_E_OK) && (FALSE == acl_udf_rule.udf_rule_en))
                {
                    /* udf rule 0 */
                    switch (index)
                    {
                        case 0:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 3;/* offset 6 */
                        acl_udf_rule.cmp_sel = 1;/* threshold */
                        udf_cmp = ((mac[0] << 8) | mac[1]);
                        val = 0;
                        if (udf_cmp != 0)
                        {
                            val = udf_cmp - 1;
                        }
                        acl_udf_rule.cmp_pat = 0;/* CMP low value */
                        acl_udf_rule.cmp_mask = val;/* CMP high value */
                        break;

                        case 1:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 4;/* offset 8 */
                        acl_udf_rule.cmp_sel = 1;/* threshold */
                        udf_cmp = ((mac[2] << 8) | mac[3]);
                        val = 0;
                        if (udf_cmp != 0)
                        {
                            val = udf_cmp - 1;
                        }
                        acl_udf_rule.cmp_pat = 0;/* CMP low value */
                        acl_udf_rule.cmp_mask = val;/* CMP high value */
                        break;

                        case 2:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 5;/* offset 10 */
                        acl_udf_rule.cmp_sel = 1;/* threshold */
                        udf_cmp = ((mac[4] << 8) | mac[5]);
                        val = 0;
                        if (udf_cmp != 0)
                        {
                            val = udf_cmp - 1;
                        }
                        acl_udf_rule.cmp_pat = 0;/* CMP low value */
                        acl_udf_rule.cmp_mask = val;/* CMP high value */
                        break;

                        case 3:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_L2_PAYLOAD;
                        acl_udf_rule.word_ofst = 2;/* offset 4, l2 payload */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
                        acl_udf_rule.cmp_pat = ((mac[0] << 8) | (mac[1]));/* HW LDF module ID */
#else
                        acl_udf_rule.cmp_pat = LP_MODULE_ID;/* CMP pattern */
#endif
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */
                        break;

                        case 4:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 3;/* offset 6 */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
                        acl_udf_rule.cmp_pat = ((mac[0] << 8) | (mac[1]));/* CMP pattern */
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */
                        break;

                        case 5:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 4;/* offset 8 */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
                        acl_udf_rule.cmp_pat = ((mac[2] << 8) | (mac[3]));/* CMP pattern */
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */
                        break;

                        case 6:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 3;/* offset 6 */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
                        acl_udf_rule.cmp_pat = 0;/* CMP pattern */
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */
                        break;

                        case 7:
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
                        acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
                        acl_udf_rule.word_ofst = 4;/* offset 8 */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
                        acl_udf_rule.cmp_pat = 0;/* CMP pattern */
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */
                        break;

                        default:
                        break;
                    }

                    if (TRUE == acl_udf_rule.udf_rule_en)
                    {
                        rc = air_acl_setUdfRule(unit, acl_udf_id, &acl_udf_rule);
                        if (rc != AIR_E_OK)
                        {
                            LP_LOG_ERROR("Add LD/LP MAC ACL udf-id %d failed, rc=%d !", acl_udf_id, rc);
                            mw_acl_mutex_release();
                            return MW_E_OP_INCOMPLETE;
                        }
                        _lp_mac_acl_udf_id[index] = acl_udf_id;
                    }
                    break;
                }
            }

            if (acl_udf_id == AIR_MAX_NUM_OF_UDF_ENTRY)
            {
                LP_LOG_ERROR("Get free ACL udf-id failed !");
                mw_acl_mutex_release();
                return MW_E_ENTRY_REACH_END;
            }
        }
        mw_acl_mutex_release();
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: lp_acl_setMacComparationAcl
 * PURPOSE:
 *      Set ACL for loop prevention mac comparison ACL
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_REACH_END
 *      MW_E_NOT_INITED
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_setMacComparationAcl(
    void)
{
    I32_T               rc = 0;
    UI8_T               index;
    UI32_T              unit = 0, acl_rule_id = 0;
    AIR_MAC_T           mac;
    AIR_ACL_RULE_T      acl_rule;
    AIR_ACL_ACTION_T    action;
    BOOL_T              set_acl_rule = TRUE;
#ifdef LP_LOOP_DECTECTION_SUPPORT
    LP_CONFIG_INFO_T    *ptr_lp_config = lp_getConfig();

    if ((NULL == ptr_lp_config) || (MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin))
    {
        LP_LOG_WARN("This acl not support in LD mode");
        return MW_E_NOT_INITED;
    }
#endif /* LP_LOOP_DECTECTION_SUPPORT */

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        LP_LOG_ERROR("Get sys-mac error");
        return MW_E_BAD_PARAMETER;
    }
    rc = lp_acl_setMacComparationUdf();
    if(MW_E_OK != rc)
    {
        LP_LOG_ERROR("Set compare mac udf error");
        return MW_E_NOT_INITED;
    }
    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (index = 0; index < LP_MAC_ACL_NUM; index++)
        {
            if (MW_ACL_ID_INVALID != _lp_mac_acl_id[index])
            {
                continue;
            }

            rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
            if (MW_E_OK == rc)
            {
                set_acl_rule = TRUE;
                /* lp-mac acl rule 0 */
                switch (index)
                {
                    case 0:
                    if (FALSE == (0 == ((mac[0] << 8) | mac[1]) && 0 == ((mac[2] << 8) | mac[3]) && 0 != ((mac[4] << 8) | mac[5])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[7]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[7]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));
                    /* Set ACL action */
                    /* entry-id=1 mib-id=1 fw_port=drop */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 1:
                    if (FALSE == (0 == ((mac[0] << 8) | mac[1]) && 0 != ((mac[2] << 8) | mac[3]) && 0 != ((mac[4] << 8) | mac[5])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[1]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[1]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.field_valid |= (1U << AIR_ACL_MIB);
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 2:
                    if (FALSE == (0 == ((mac[0] << 8) | mac[1]) && 0 != ((mac[2] << 8) | mac[3]) && 0 != ((mac[4] << 8) | mac[5])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[5]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[6]) | (1 << _lp_mac_acl_udf_id[5]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 3:
                    if (0 == ((mac[0] << 8) | mac[1]))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[0]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[0]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.field_valid |= (1U << AIR_ACL_MIB);
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 4:
                    if (FALSE == (0 != ((mac[0] << 8) | mac[1]) && 0 == ((mac[2] << 8) | mac[3]) && 0 != ((mac[4] << 8) | mac[5])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[7]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[7]) | (1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.field_valid |= (1U << AIR_ACL_MIB);
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 5:
                    if (FALSE == (0 != ((mac[0] << 8) | mac[1]) && 0 != ((mac[2] << 8) | mac[3])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[1]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[1]) | (1 << _lp_mac_acl_udf_id[3]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.field_valid |= (1U << AIR_ACL_MIB);
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;

                    case 6:
                    if (FALSE == (0 != ((mac[0] << 8) | mac[1]) && 0 != ((mac[2] << 8) | mac[3]) && 0 != ((mac[4] << 8) | mac[5])))
                    {
                        set_acl_rule = FALSE;
                        break;
                    }
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
                    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = ((1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]) | (1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[5]));
                    acl_rule.mask.udf = ((1 << _lp_mac_acl_udf_id[2]) | (1 << _lp_mac_acl_udf_id[3]) | (1 << _lp_mac_acl_udf_id[4]) | (1 << _lp_mac_acl_udf_id[5]));
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));
                    /* Set ACL action */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
                    action.field_valid |= (1U << AIR_ACL_FW_PORT);
                    break;
                    default:
                    set_acl_rule = FALSE;
                    break;
                }

                if (TRUE == set_acl_rule)
                {
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc == AIR_E_OK)
                    {
#if defined(AIR_EN_CORAL)
                        action.dont_learn= 1;
                        action.field_valid |= BIT(AIR_ACL_DONT_LEARN);
#endif

                        /* acl add action entry-id=0 mib-id=0 port-fw=cpu-port-only pri-user 7 */
                        rc = air_acl_setAction(unit, acl_rule_id, &action);
                        if (AIR_E_OK == rc)
                        {
                            _lp_mac_acl_id[index] = acl_rule_id;
                        }
                        else
                        {
                            LP_LOG_ERROR("Add LD/LP MAC ACL rule entry-id %d action fail, rc=%d.", acl_rule_id, rc);
                            air_acl_delRule(unit, acl_rule_id);
                        }
                    }
                    else
                    {
                        LP_LOG_ERROR("Add LD/LP MAC  ACL rule entry-id %d failed, rc=%d.", acl_rule_id, rc);
                    }
                }
            }
            else
            {
                LP_LOG_ERROR("Get free LD/LP MAC ACL rule-id failed rc:%d", rc);
            }

            if (MW_E_OK != rc)
            {
                mw_acl_mutex_release();
                return rc;
            }
        }
        mw_acl_mutex_release();
    }

    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_removeMacComparationAcl
 * PURPOSE:
 *      Remove ACL for loop prevention mac comparison ACL
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
lp_acl_removeMacComparationAcl(
    void)
{
    UI32_T unit = 0, index = 0;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (index = 0; index < LP_MAC_ACL_NUM; index++)
        {
            if (_lp_mac_acl_id[index] != MW_ACL_ID_INVALID)
            {
                air_acl_delRule(unit, _lp_mac_acl_id[index]);
                air_acl_delAction(unit, _lp_mac_acl_id[index]);
                _lp_mac_acl_id[index] = MW_ACL_ID_INVALID;
            }
        }

        for (index = 0; index < LP_MAC_UDF_NUM; index++)
        {
            if (_lp_mac_acl_udf_id[index] != MW_ACL_ID_INVALID)
            {
                air_acl_delUdfRule(unit, _lp_mac_acl_udf_id[index]);
                _lp_mac_acl_udf_id[index] = MW_ACL_ID_INVALID;
            }
        }
        mw_acl_mutex_release();
    }

    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_setGenericAcl
 * PURPOSE:
 *      This function is used to set the generic ACL for the loop prevention.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_REACH_END
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_setGenericAcl(
    void)
{
    I32_T               rc = MW_E_OK;
    UI32_T              unit = 0, port = 0;
    UI32_T              acl_rule_id = 0, acl_attack_prevention_id = 0;
    AIR_ACL_RULE_T      acl_rule;
    AIR_ACL_ACTION_T    action;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }

        if (MW_ACL_ID_INVALID == _lp_generic_acl_id[(port - 1)])
        {
            if (MW_E_OK == mw_acl_mutex_take())
            {
                rc = _lp_acl_setPortAttackPrevention(unit, port, &acl_attack_prevention_id);
                mw_acl_mutex_release();
            }
            if (MW_E_OK != rc)
            {
                LP_LOG_ERROR("Get free ACL acl_attack_prevention_id failed, rc:%d !", rc);
                return MW_E_OP_INCOMPLETE;
            }
            _lp_generic_acl_attack_prevention_id[(port - 1)] = acl_attack_prevention_id;

            osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
            if (MW_E_OK == mw_acl_mutex_take())
            {
                rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
                if (MW_E_OK == rc)
                {
                    /* acl add rule entry-id=0 state=enable portlist=0-27 rule-end etype=0x8070 */
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_ADD(acl_rule.portmap, port);
                    acl_rule.end = TRUE;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = (0x1 << _lp_acl_udf_id);
                    acl_rule.mask.udf = (0x1 << _lp_acl_udf_id);
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_UDF_KEY));

                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc == AIR_E_OK)
                    {
                        LP_LOG_DEBUG("ACL rule entry-id %d, acl_rule.rule_en = %d", acl_rule_id, acl_rule.rule_en);
                        /* acl add action entry-id=0 mib-id=0 port-fw=cpu-port-only pri-user 7, (1)rate-limit, (2)mark as MG, (3)set acl_vlan_swap, (4)set port to all, (5)set prio to 7 */
                        osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                        action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                        action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                        action.attack_rate_id = (acl_attack_prevention_id & 0xFF);
                        action.field_valid |= (1U << AIR_ACL_ATTACK);
                        action.pri_user = MW_ACL_RX_PRIORITY_LP;
                        action.field_valid |= (1U << AIR_ACL_PRI);
#ifdef AIR_LP_USE_STP_BLOCK
                        action.as_bpdu = 1;
                        action.field_valid |= BIT(AIR_ACL_BPDU);
#endif /* AIR_LP_USE_STP_BLOCK */
#if defined(AIR_EN_CORAL)
                        action.dont_learn= 1;
                        action.field_valid |= BIT(AIR_ACL_DONT_LEARN);
#endif
                        rc = air_acl_setAction(unit, acl_rule_id, &action);
                        if (AIR_E_OK == rc)
                        {
                            _lp_generic_acl_id[(port - 1)] = acl_rule_id;
                        }
                        else
                        {
                            LP_LOG_ERROR("Add LD/LP generic ACL rule entry-id %d action fail, rc=%d.", acl_rule_id, rc);
                            air_acl_delRule(unit, acl_rule_id);
                        }
                    }
                    else
                    {
                        LP_LOG_ERROR("Add LD/LP ACL rule entry-id %d failed, rc=%d.", acl_rule_id, rc);
                    }
                }
                else
                {
                    LP_LOG_ERROR("Fail to get an ACL entry, rc=%d.", rc);
                }
                mw_acl_mutex_release();
            }
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_removeGenericAcl
 * PURPOSE:
 *      This function is used to remove the generic ACL for the loop prevention.
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
lp_acl_removeGenericAcl(
    void)
{
    UI32_T unit = 0, port = 0;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                /* Skip cpu port */
                continue;
            }
            if (_lp_generic_acl_id[(port - 1)] != MW_ACL_ID_INVALID)
            {
                air_acl_delRule(unit, _lp_generic_acl_id[(port - 1)]);
                air_acl_delAction(unit, _lp_generic_acl_id[(port - 1)]);
                _lp_generic_acl_id[(port - 1)] = MW_ACL_ID_INVALID;
            }

            if (_lp_generic_acl_attack_prevention_id[(port - 1)] != MW_ACL_ID_INVALID)
            {
                air_dos_clearRateLimitCfg(unit, _lp_generic_acl_attack_prevention_id[(port - 1)]);
                _lp_generic_acl_attack_prevention_id[(port - 1)] = MW_ACL_ID_INVALID;
            }
        }
        mw_acl_mutex_release();
    }

    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_initVariable
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
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_initVariable(
    void)
{
    UI32_T    port = 0;
    UI8_T     index;

#ifdef AIR_LP_USE_STP_BLOCK
    _lp_acl_ldf_asbpdu_id = MW_ACL_ID_INVALID;
#endif
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        _lp_acl_id[(port - 1)] = MW_ACL_ID_INVALID;
#ifndef LP_MW_SUPPORT
        _lp_acl_attack_prevention_id[(port - 1)] = MW_ACL_ID_INVALID;
#endif
        _lp_generic_acl_attack_prevention_id[(port - 1)] = MW_ACL_ID_INVALID;
        _lp_generic_acl_id[(port - 1)] = MW_ACL_ID_INVALID;
    }
    _lp_acl_udf_id = MW_ACL_ID_INVALID;

    for (index = 0; index < LP_MAC_ACL_NUM; index++)
    {
        _lp_mac_acl_id[index] = MW_ACL_ID_INVALID;
    }

    for (index = 0; index < LP_MAC_UDF_NUM; index++)
    {
        _lp_mac_acl_udf_id[index] = MW_ACL_ID_INVALID;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_set
 * PURPOSE:
 *      Set ACL for loop prevention
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_set(
    void)
{
    UI32_T    unit = 0;
    UI32_T    port = 0;
    AIR_MAC_T mac;
    I32_T     rc = MW_E_OK;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        LP_LOG_ERROR("Get sys-mac error");
        return MW_E_BAD_PARAMETER;
    }
    mw_dos_setGlobalCfg(unit, TRUE);
    if (MW_ACL_ID_INVALID == _lp_acl_udf_id)
    {
        UI8_T              acl_udf_id = 0;
        AIR_ACL_UDF_RULE_T acl_udf_rule;

        if (MW_E_OK == mw_acl_mutex_take())
        {
            for (;acl_udf_id < AIR_MAX_NUM_OF_UDF_ENTRY; acl_udf_id++)
            {
                if (air_acl_getUdfRule(unit, acl_udf_id, &acl_udf_rule) == AIR_E_OK)
                {
                    if (FALSE == acl_udf_rule.udf_rule_en)
                    {
                        /* udf rule 0 */
                        acl_udf_rule.udf_rule_en = TRUE;
                        AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
#ifndef AIR_LP_USE_STP_BLOCK
                        AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
#endif /* AIR_LP_USE_STP_BLOCK */
                        acl_udf_rule.udf_pkt_type = AIR_ACL_L2_PAYLOAD;
                        acl_udf_rule.word_ofst = 2;/* offset 4, l2 payload */
                        acl_udf_rule.cmp_sel = 0;/* pattern */
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
                        acl_udf_rule.cmp_pat = ((mac[0] << 8) | (mac[1]));/* HW LDF module ID */
#else
                        acl_udf_rule.cmp_pat = LP_MODULE_ID;/* CMP pattern */
#endif
                        acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */

                        rc = air_acl_setUdfRule(unit, acl_udf_id, &acl_udf_rule);
                        if (rc != AIR_E_OK)
                        {
                            LP_LOG_ERROR("Add LD/LP ACL udf-id %d failed, rc=%d", acl_udf_id, rc);
                            mw_acl_mutex_release();
                            return MW_E_OP_INCOMPLETE;
                        }

                        _lp_acl_udf_id = acl_udf_id;
                        break;
                    }
                }
            }
            mw_acl_mutex_release();
        }
        if (acl_udf_id == AIR_MAX_NUM_OF_UDF_ENTRY)
        {
            LP_LOG_ERROR("Get free ACL udf-id failed !");
            return MW_E_ENTRY_REACH_END;
        }
    }
#ifdef AIR_LP_USE_STP_BLOCK
    lp_acl_setLdfAsBpduAcl();
#endif
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        if (MW_ACL_ID_INVALID == _lp_acl_id[(port - 1)])
        {
            UI32_T           acl_rule_id = 0, acl_attack_prevention_id = 0;
            AIR_ACL_RULE_T   acl_rule;
            AIR_ACL_ACTION_T action;

            if (MW_E_OK == mw_acl_mutex_take())
            {
#ifdef LP_MW_SUPPORT
                rc = mw_rate_limit_lp_stp_port_attack_id_get(unit, port, MW_PORT_RATE_LIMIT_USER_LP, &acl_attack_prevention_id);
#else
                rc = _lp_acl_setPortAttackPrevention(unit, port, &acl_attack_prevention_id);
#endif
                mw_acl_mutex_release();
            }
            if (MW_E_OK != rc)
            {
                LP_LOG_ERROR("Get free ACL acl_attack_prevention_id failed, rc:%d", rc);
                return MW_E_OP_INCOMPLETE;
            }
#ifndef LP_MW_SUPPORT
            _lp_acl_attack_prevention_id[(port - 1)] = acl_attack_prevention_id;
#endif
            if (MW_E_OK == mw_acl_mutex_take())
            {
                rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
                if (MW_E_OK == rc)
                {
                    /* acl add rule entry-id=0 state=enable portlist=0-27 rule-end smac=00:aa:bb:11:22:33 etype=0x8070 */
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    acl_rule.rule_en = TRUE;
                    AIR_PORT_ADD(acl_rule.portmap, port);
                    acl_rule.end = TRUE;
                    osapi_memcpy(acl_rule.key.smac, mac, 6);
                    acl_rule.mask.smac = 0x3f;
                    acl_rule.key.etype = ETHTYPE_LP;
                    acl_rule.mask.etype = 0x3;
                    acl_rule.key.udf = (0x1 << _lp_acl_udf_id);
                    acl_rule.mask.udf = (0x1 << _lp_acl_udf_id);
                    acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_SMAC_KEY) | (1U << AIR_ACL_UDF_KEY));
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    LP_LOG_DEBUG("Add LD/LP ACL rule entry-id %d , rc=%d", acl_rule_id, rc);
                    if (rc == AIR_E_OK)
                    {
                        /* acl add action entry-id=0 mib-id=0 port-fw=cpu-port-only pri-user 7 */
                        osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                        action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                        action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                        action.attack_rate_id = (acl_attack_prevention_id & 0xFF);
                        action.field_valid |= (1U << AIR_ACL_ATTACK);
                        action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
                        action.field_valid |= (1U << AIR_ACL_FW_PORT);
                        action.pri_user = MW_ACL_RX_PRIORITY_LP;
                        action.field_valid |= (1U << AIR_ACL_PRI);
#ifdef AIR_LP_USE_STP_BLOCK
                        action.as_bpdu = 1;
                        action.field_valid |= BIT(AIR_ACL_BPDU);
#endif /* AIR_LP_USE_STP_BLOCK */
#if defined(AIR_EN_CORAL)
                        action.dont_learn= 1;
                        action.field_valid |= BIT(AIR_ACL_DONT_LEARN);
#endif

                        rc = air_acl_setAction(unit, acl_rule_id, &action);
                        if (AIR_E_OK == rc)
                        {
                            _lp_acl_id[(port - 1)] = acl_rule_id;
                        }
                        else
                        {
                            LP_LOG_ERROR("Add LD/LP ACL rule entry-id %d action fail, rc=%d.", acl_rule_id, rc);
                            air_acl_delRule(unit, acl_rule_id);
                        }
                    }
                }
                else
                {
                    LP_LOG_ERROR("Fail to get an ACL entry, rc=%d.", rc);
                }
                mw_acl_mutex_release();
            }
        }
    }

    lp_acl_setMacComparationAcl();
    lp_acl_setGenericAcl();

    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_remove
 * PURPOSE:
 *      Remove ACL for loop prevention
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
lp_acl_remove(
    void)
{
    UI32_T    unit = 0;
    UI32_T    port = 0;

#ifdef AIR_LP_USE_STP_BLOCK
    lp_acl_removeLdfAsBpduAcl();
#endif
    if (MW_E_OK == mw_acl_mutex_take())
    {
        mw_dos_setGlobalCfg(unit, FALSE);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                /* Skip cpu port */
                continue;
            }
            if (_lp_acl_id[(port - 1)] != MW_ACL_ID_INVALID)
            {
                air_acl_delRule(unit, _lp_acl_id[(port - 1)]);
                air_acl_delAction(unit, _lp_acl_id[(port - 1)]);
                _lp_acl_id[(port - 1)] = MW_ACL_ID_INVALID;
#ifdef LP_MW_SUPPORT
                mw_rate_limit_lp_stp_port_attack_clear(unit, port);
#else
                if (_lp_acl_attack_prevention_id[(port - 1)] != MW_ACL_ID_INVALID)
                {
                    air_dos_clearRateLimitCfg(unit, _lp_acl_attack_prevention_id[(port - 1)]);
                    _lp_acl_attack_prevention_id[(port - 1)] = MW_ACL_ID_INVALID;
                }
#endif /* LP_MW_SUPPORT */
            }
        }
        if (_lp_acl_udf_id != MW_ACL_ID_INVALID)
        {
            air_acl_delUdfRule(unit, _lp_acl_udf_id);
            _lp_acl_udf_id = MW_ACL_ID_INVALID;
        }

        mw_acl_mutex_release();
    }
    lp_acl_removeMacComparationAcl();
    lp_acl_removeGenericAcl();

    return MW_E_OK;
}

#ifdef AIR_LP_USE_STP_BLOCK
/* FUNCTION NAME: lp_acl_setLdfAsBpduAcl
 * PURPOSE:
 *      Create the LDF AS BPDU ACL.
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
lp_acl_setLdfAsBpduAcl(
    void)
{
    UI32_T              unit = 0;
    I32_T               rc = MW_E_OK;
    UI32_T              acl_rule_id = 0;
    AIR_ACL_RULE_T      acl_rule;
    AIR_ACL_ACTION_T    action;
    AIR_MAC_T           mac;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        LP_LOG_ERROR("Get sys-mac error");
        return MW_E_OP_INCOMPLETE;
    }
    if (MW_E_OK == mw_acl_mutex_take())
    {
        rc = air_acl_setPortState(unit, PLAT_CPU_PORT, TRUE);
        if (AIR_E_OK != rc)
        {
            LP_LOG_ERROR("Enable ACL port %d config failed, rc=%d.", PLAT_CPU_PORT, rc);
            mw_acl_mutex_release();
            return MW_E_OP_INCOMPLETE;
        }
        if (MW_ACL_ID_INVALID == _lp_acl_ldf_asbpdu_id)
        {
            rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
            if (MW_E_OK == rc)
            {
                /* acl add rule entry-id=0 state=enable portlist=0-27 rule-end smac=00:aa:bb:11:22:33 etype=0x8070 */
                osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                acl_rule.rule_en = TRUE;
                AIR_PORT_ADD(acl_rule.portmap, PLAT_CPU_PORT);
                acl_rule.end = TRUE;
                osapi_memcpy(acl_rule.key.smac, mac, 6);
                acl_rule.mask.smac = 0x3f;
                acl_rule.key.etype = ETHTYPE_LP;
                acl_rule.mask.etype = 0x3;
                acl_rule.key.udf = BIT(_lp_acl_udf_id);
                acl_rule.mask.udf = BIT(_lp_acl_udf_id);
                acl_rule.field_valid |= (BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_SMAC_KEY) | BIT(AIR_ACL_UDF_KEY));
                rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                LP_LOG_DEBUG("Add LD/LP ACL rule entry-id %d , rc=%d", acl_rule_id, rc);
                if (rc == AIR_E_OK)
                {
                    /* acl add action entry-id=0 mib-id=0 port-fw=cpu-port-only pri-user 7 */
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#if LP_DEBUG
                    action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
                    action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* LP_DEBUG */
                    action.pri_user = MW_ACL_RX_PRIORITY_LP;
                    action.field_valid |= BIT(AIR_ACL_PRI);
                    action.as_bpdu = 1;
                    action.field_valid |= BIT(AIR_ACL_BPDU);
                    rc = air_acl_setAction(unit, acl_rule_id, &action);
                    if (AIR_E_OK == rc)
                    {
                        _lp_acl_ldf_asbpdu_id = acl_rule_id;
                    }
                    else
                    {
                        LP_LOG_ERROR("Add LD/LP ACL rule entry-id %d action fail, rc=%d", acl_rule_id, rc);
                        air_acl_delRule(unit, acl_rule_id);
                    }
                }
            }
            else
            {
                LP_LOG_ERROR("Fail to get an ACL entry, rc=%d", rc);
            }
        }
        mw_acl_mutex_release();
    }
    return MW_E_OK;
}

/* FUNCTION NAME: lp_acl_removeLdfAsBpduAcl
 * PURPOSE:
 *      Remove the LDF AS BPDU ACL.
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
lp_acl_removeLdfAsBpduAcl(
    void)
{
    UI32_T unit = 0;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        air_acl_setPortState(unit, PLAT_CPU_PORT, FALSE);
        if (MW_ACL_ID_INVALID != _lp_acl_ldf_asbpdu_id)
        {
            air_acl_delRule(unit, _lp_acl_ldf_asbpdu_id);
            air_acl_delAction(unit, _lp_acl_ldf_asbpdu_id);
            _lp_acl_ldf_asbpdu_id = MW_ACL_ID_INVALID;
        }
        mw_acl_mutex_release();
    }

    return MW_E_OK;
}
#endif /* AIR_LP_USE_STP_BLOCK */

/* FUNCTION NAME: lp_acl_dumpInfo
 * PURPOSE:
 *      Dump lp ACL information.
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
lp_acl_dumpInfo(
    void)
{
    UI32_T           u_portId = 0, index = 0;

    MW_CMD_OUTPUT("  LP ACL:\n");
#ifdef AIR_LP_USE_STP_BLOCK
    MW_CMD_OUTPUT("  lp_acl_ldf_asbpdu_id: %d\n", _lp_acl_ldf_asbpdu_id);
#endif
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        MW_CMD_OUTPUT("  lp_acl_id[%d]: %d\n", u_portId, _lp_acl_id[(u_portId - 1)]);
    }
    MW_CMD_OUTPUT("  lp_acl_udf_id: %d\n", _lp_acl_udf_id);
    MW_CMD_OUTPUT("  LP MAC ACL:\n");
    for (index = 0; index < LP_MAC_ACL_NUM; index++)
    {
        MW_CMD_OUTPUT("  lp_mac_acl_id[%d]: %d\n", index, _lp_mac_acl_id[index]);
    }
    for (index = 0; index < LP_MAC_UDF_NUM; index++)
    {
        MW_CMD_OUTPUT("  lp_mac_acl_udf_id[%d]: %d\n", index, _lp_mac_acl_udf_id[index]);
    }

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        MW_CMD_OUTPUT("  lp_generic_acl_id[%d]: %d\n", u_portId, _lp_generic_acl_id[(u_portId - 1)]);
    }

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        MW_CMD_OUTPUT("  lp_generic_acl_dos_id[%d]: %d\n", u_portId, _lp_generic_acl_attack_prevention_id[(u_portId - 1)]);
    }
    return;
}
