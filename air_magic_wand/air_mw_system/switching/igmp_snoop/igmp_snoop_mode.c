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

/* FILE NAME:  igmp_snoop_mode.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop mode.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_mode.h"
#include "igmp_snoop.h"
#include "igmp_snoop_utils.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_acl.h"
#include "igmp_snoop_port.h"
#ifndef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_syncd.h"
#endif
#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
#include "igmp_snp_config_customer.h"
#include "dumb_config_customer.h"
#endif
#include "air_l2.h"

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
/* FUNCTION NAME:   igmp_snp_setAdminMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping admin mode.
 *
 * INPUT:
 *      enable       --  admin mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setAdminMode(
    UI8_T enable)
{
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("enable=%d", enable);
    if (enable == ptr_igmp_snp->cfg_info.enable)
    {
        return MW_E_OP_INVALID;
    }

    if(TRUE == enable)
    {
        ptr_igmp_snp->cfg_info.enable = ENABLE;
        igmp_snp_acl_add(IGMP_SNP_DEFAULT_VID, &(ptr_igmp_snp->cfg_info.igmp_acl_id), IGMP_SNP_ACL_TYPE_IGMP);
#ifdef AIR_SUPPORT_PIM_AWARE
        igmp_snp_acl_add(IGMP_SNP_DEFAULT_VID, &(ptr_igmp_snp->cfg_info.pim_acl_id), IGMP_SNP_ACL_TYPE_PIM);
#endif
        if(0 != ptr_igmp_snp->cfg_info.static_router[0])
        {
            /* Add static router */
            igmp_snp_port_addStaticMrouterPort(ptr_igmp_snp->cfg_info.static_router[0], 0);
        }
    }
    else
    {
        ptr_igmp_snp->cfg_info.enable = DISABLE;
        igmp_snp_acl_delete(IGMP_SNP_DEFAULT_VID, &(ptr_igmp_snp->cfg_info.igmp_acl_id), IGMP_SNP_ACL_TYPE_IGMP);
#ifdef AIR_SUPPORT_PIM_AWARE
        igmp_snp_acl_delete(IGMP_SNP_DEFAULT_VID, &(ptr_igmp_snp->cfg_info.pim_acl_id), IGMP_SNP_ACL_TYPE_PIM);
#endif
        igmp_snp_clearAllEntry();
    }

#ifndef IGMP_SNP_MW_SUPPORT
    igmp_snp_syncd_setIpmcMode(enable);
#endif

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_setRptSuppressMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping report suppress.
 *
 * INPUT:
 *      enable       --  report suppress mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setRptSuppressMode(
    UI8_T enable)
{
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("enable=%d", enable);
    if (enable == ptr_igmp_snp->cfg_info.rpt_suppress)
    {
        return MW_E_OP_INVALID;
    }

    if (enable)
    {
        ptr_igmp_snp->cfg_info.rpt_suppress = ENABLE;
    }
    else
    {
        struct IGMP_SNP_GROUP_LIST_S *ptr_cur = NULL;

        ptr_igmp_snp->cfg_info.rpt_suppress = DISABLE;

        CSLIST_FOREACH(ptr_cur, &ptr_igmp_snp->group_head, next)
        {
            ptr_cur->rpt_suppress_flag = FALSE;
            ptr_cur->rpt_suppress_timer = 0;
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_setFastLeaveMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping fast-leave.
 *
 * INPUT:
 *      enable       --  fast-leave mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setFastLeaveMode(
    UI8_T enable)
{
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("enable=%d", enable);

    if (enable == ptr_igmp_snp->cfg_info.fast_leave)
    {
        return MW_E_OP_INVALID;
    }

    if (enable)
    {
        ptr_igmp_snp->cfg_info.fast_leave = ENABLE;
    }
    else
    {
        ptr_igmp_snp->cfg_info.fast_leave = DISABLE;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_setUnIpmcForwardMode
 * PURPOSE:
 *      This API is used to enable/disable unknown ip multicast packet drop function.
 *
 * INPUT:
 *      enable       --  enable or disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setUnIpmcForwardMode(
    UI8_T enable)
{
    I32_T                   rc = MW_E_OK;
    AIR_PORT_BITMAP_T       pbm = {0};
    AIR_FORWARD_ACTION_T    action = AIR_FORWARD_ACTION_FLOODING;
    IGMP_SNP_LIST_T         *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);

    if (DISABLE == ptr_igmp_snp->cfg_info.enable)
    {
        action = AIR_FORWARD_ACTION_FLOODING;
    }
    else
    {
        if(TRUE == enable)
        {
            action = AIR_FORWARD_ACTION_DROP;
        }
        else
        {
            action = AIR_FORWARD_ACTION_FLOODING;
        }
    }
    rc = air_l2_setForwardMode(0, AIR_FORWARD_TYPE_UIPMCST, action, pbm);
    IGMP_SNP_LOG_DEBUG("uipmc_forwarding: %d", action);

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
/* FUNCTION NAME:   igmp_snp_setModeWithCfg
 * PURPOSE:
 *      Set the igmp snp with the data of config structure input.
 *
 * INPUT:
 *      ptr_data     --  Pointer to the igmp_snp config structure filled with the config data
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
igmp_snp_setModeWithCfg(
    const void *ptr_data)
{
    I32_T   rc = MW_E_OK;
    const IGMP_SNP_CUSTOMER_CONFIG_FLAGS_T *ptr_cfg = NULL;

    MW_CHECK_PTR(ptr_data);
    ptr_cfg = (const IGMP_SNP_CUSTOMER_CONFIG_FLAGS_T *)ptr_data;
    rc = igmp_snp_setAdminMode(ptr_cfg->igmp_snp_enable);
    rc |= igmp_snp_setUnIpmcForwardMode(ptr_cfg->igmp_uipmc_drop);
#ifdef AIR_SUPPORT_IGMPV3_AWARE
    rc |= igmp_snp_setV3AwareMode(ptr_cfg->igmpv3_aware_enable);
#endif


    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

/* FUNCTION NAME:   igmp_snp_setDefaultCfgMode
 * PURPOSE:
 *      Set the igmp snp to default configuration mode.
 *
 * INPUT:
 *      None
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
igmp_snp_setDefaultCfgMode(
    void)
{
    I32_T   rc = MW_E_OK;
    const   IGMP_SNP_CUSTOMER_CONFIG_FLAGS_T *ptr_cfg = mw_dumb_getIgmpSnpCfg();

    MW_CHECK_PTR(ptr_cfg);
    rc = igmp_snp_setModeWithCfg(ptr_cfg);
    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

/* FUNCTION NAME:   igmp_snp_setDipSwitchCfgMode
 * PURPOSE:
 *      Set the igmp snp dip switch config mode.
 *
 * INPUT:
 *      None
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
igmp_snp_setDipSwitchCfgMode(
    void)
{
    I32_T   rc = MW_E_OK;
    const   IGMP_SNP_CUSTOMER_CONFIG_FLAGS_T *ptr_cfg = mw_dumb_getIgmpSnpDipSwitchCfg();

    MW_CHECK_PTR(ptr_cfg);
    rc = igmp_snp_setModeWithCfg(ptr_cfg);
    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

#endif /* IGMP_SNP_CUSTOMER_CONFIG_SUPPORT */

#ifdef AIR_SUPPORT_IGMPV3_AWARE
/* FUNCTION NAME:   igmp_snp_v3ware_enable_set
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping v3 aware function.
 *
 * INPUT:
 *      enable       --  enable or disable IGMP snooping v3 aware function
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
igmp_snp_setV3AwareMode(
    UI8_T enable)
{
    IGMP_SNP_LIST_T         *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);

    ptr_igmp_snp->cfg_info.igmpv3_aware = enable;

    return MW_E_OK;
}
#endif /* IGMP_SNP_V3_AWARE_SUPPORT */
