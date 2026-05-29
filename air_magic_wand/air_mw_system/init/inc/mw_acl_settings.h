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


/* FILE NAME:  mw_acl_settings.h
 * PURPOSE:
 * It provides the API and definitions for MW ACL settings of some modules.
 *
 * NOTES:
 */

#ifndef MW_ACL_SETTINGS_H
#define MW_ACL_SETTINGS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Definition for the queue priority. */
#define MW_ACL_PRIORIY_QUEUE_0    (0)
#define MW_ACL_PRIORIY_QUEUE_1    (1)
#define MW_ACL_PRIORIY_QUEUE_2    (2)
#define MW_ACL_PRIORIY_QUEUE_3    (3)
#define MW_ACL_PRIORIY_QUEUE_4    (4)
#define MW_ACL_PRIORIY_QUEUE_5    (5)
#define MW_ACL_PRIORIY_QUEUE_6    (6)
#define MW_ACL_PRIORIY_QUEUE_7    (7)

/* Definition for ACL ID */
#define MW_ACL_ID_INVALID                   (0xFFFF)
#define MW_ATTACK_ID_INVALID                (0xFFFF)

/* Definition for ACL RX priority for different types of packets. */
#define MW_ACL_RX_PRIORITY_RAPS             (MW_ACL_PRIORIY_QUEUE_7)
#define MW_ACL_RX_PRIORITY_LACP             (MW_ACL_PRIORIY_QUEUE_7)
#define MW_ACL_RX_PRIORITY_BPDU             (MW_ACL_PRIORIY_QUEUE_7)
#define MW_ACL_RX_PRIORITY_LP               (MW_ACL_PRIORIY_QUEUE_7)
#define MW_ACL_RX_PRIORITY_LLDP             (MW_ACL_PRIORIY_QUEUE_7)
#define MW_ACL_RX_PRIORITY_ICMPV6_ND        (MW_ACL_PRIORIY_QUEUE_6)
#define MW_ACL_RX_PRIORITY_ICMPV6_RS        (MW_ACL_PRIORIY_QUEUE_5)
#define MW_ACL_RX_PRIORITY_ARP              (MW_ACL_PRIORIY_QUEUE_5)
#define MW_ACL_RX_PRIORITY_IGMP_SNP         (MW_ACL_PRIORIY_QUEUE_5)
#define MW_ACL_RX_PRIORITY_MLD_SNP          (MW_ACL_PRIORIY_QUEUE_5)
#define MW_ACL_RX_PRIORITY_NOMRAL_PACKET    (MW_ACL_PRIORIY_QUEUE_4)

#define MW_ACL_ACT_EGTAG_DIS                (0)
#define MW_ACL_ACT_EGTAG_CONSISTENT         (1)
#define MW_ACL_ACT_EGTAG_UNTAG              (4)
#define MW_ACL_ACT_EGTAG_SWAP               (5)
#define MW_ACL_ACT_EGTAG_TAG                (6)
#define MW_ACL_ACT_EGTAG_STACK              (7)

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_ACL_ID_IS_VALID(id)      (((id) >= 0) && ((id) < AIR_MAX_NUM_OF_ACL_ENTRY))
#define MW_ACL_ID_IS_STATIC(id)     (((id) >= MW_ACL_ID_STATIC_MIN) && ((id) <= MW_ACL_ID_STATIC_MAX))
#define MW_ACL_GEN_MIBID(rule_id)   ((rule_id) % AIR_MAX_NUM_OF_MIB_ID)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
BOOL_T
mw_acl_untag_rule_id_check(
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id);

BOOL_T
mw_acl_da_get_sysmac_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id);

#ifdef AIR_SUPPORT_IPV6
MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id);

BOOL_T
mw_acl_da_get_ipv6_solinode_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id);

BOOL_T
mw_acl_da_get_ipv6_allnodes_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_ipv6_nd_init_tag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_ipv6_nd_init_priTag(
    UI32_T unit,
    UI32_T acl_rule_id);

MW_ERROR_NO_T
mw_acl_ipv6_nd_init_untag(
    UI32_T unit,
    UI32_T acl_rule_id);

BOOL_T
mw_acl_ipv6_nd_get_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id);
#endif /* AIR_SUPPORT_IPV6 */

MW_ERROR_NO_T
mw_acl_settings_init(
    UI32_T unit);

void
mw_acl_settings_deinit(
    UI32_T unit);

#endif /* End of MW_ACL_SETTINGS_H */
