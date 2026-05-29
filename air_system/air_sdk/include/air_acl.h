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

/* FILE NAME:  air_acl.h
 * PURPOSE:
 *      It provides ACL module API.
 * NOTES:
 */

#ifndef AIR_ACL_H
#define AIR_ACL_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_MAX_NUM_OF_ACL_ENTRY            (512)
#define AIR_MAX_NUM_OF_MIB_ID               (64)
#define AIR_MAX_NUM_OF_UDF_ENTRY            (16)
#define AIR_MAX_NUM_OF_METER_ID             (32)
#define AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID (96)
#define AIR_MAX_NUM_OF_LAST_LINE            (32)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_ACL_BLOCK_0 = 0,
    AIR_ACL_BLOCK_1,
    AIR_ACL_BLOCK_2,
    AIR_ACL_BLOCK_3,
    AIR_ACL_BLOCK_LAST
} AIR_ACL_BLOCK_T;

typedef enum
{
    AIR_ACL_DMAC_KEY = 0,
    AIR_ACL_SMAC_KEY,
    AIR_ACL_ETYPE_KEY,
    AIR_ACL_STAG_KEY,
    AIR_ACL_CTAG_KEY,
    AIR_ACL_DIP_KEY,
    AIR_ACL_SIP_KEY,
    AIR_ACL_DSCP_KEY,
    AIR_ACL_NEXT_HEADER_KEY,
    AIR_ACL_FLOW_LABEL_KEY,
    AIR_ACL_DPORT_KEY,
    AIR_ACL_SPORT_KEY,
    AIR_ACL_UDF_KEY,
    AIR_ACL_DIPV6_KEY,
    AIR_ACL_SIPV6_KEY,
    AIR_ACL_RULE_FIELD_LAST
} AIR_ACL_RULE_FIELD_T;

typedef enum
{
    AIR_ACL_PORT = 0,
    AIR_ACL_MIB,
    AIR_ACL_ATTACK,
    AIR_ACL_METER,
    AIR_ACL_FW_PORT,
    AIR_ACL_MIRROR,
    AIR_ACL_PRI,
    AIR_ACL_EGTAG,
    AIR_ACL_LKY_VLAN,
    AIR_ACL_MANG,
    AIR_ACL_DROP_PCD,
    AIR_ACL_CLASS_SLR,
    AIR_ACL_TRTCM,
    AIR_ACL_VLAN,
    AIR_ACL_MAC_CHG,
    AIR_ACL_DONT_LEARN,
    AIR_ACL_BPDU,
    AIR_ACL_ACTION_FIELD_LAST
} AIR_ACL_ACTION_FIELD_T;

typedef enum
{
    AIR_ACL_MAC_HEADER = 0,
    AIR_ACL_L2_PAYLOAD,
    AIR_ACL_IPV4_HEADER,
    AIR_ACL_IPV6_HEADER,
    AIR_ACL_L3_PAYLOAD,
    AIR_ACL_TCP_HEADER,
    AIR_ACL_UDP_HEADER,
    AIR_ACL_L4_PAYLOAD,
    AIR_ACL_PKT_TYPE_LAST
} AIR_ACL_PKT_TP_T;

typedef enum
{
    AIR_ACL_DP_COLOR_GREEN,
    AIR_ACL_DP_COLOR_YELLOW,
    AIR_ACL_DP_COLOR_RED,
    AIR_ACL_DP_COLOR_LAST
} AIR_ACL_DP_COLOR_T;

typedef enum
{
    AIR_RAT_MAC_SEL_SA = 0,
    AIR_RAT_MAC_SEL_DA,
    AIR_RAT_MAC_SEL_LAST
} AIR_RAT_MAC_SEL_T;

typedef enum
{
    AIR_ACL_ARP_DMAC_KEY = 0,
    AIR_ACL_ARP_SMAC_KEY,
    AIR_ACL_ARP_STAG_KEY,
    AIR_ACL_ARP_CTAG_KEY,
    AIR_ACL_ARP_THA_KEY,
    AIR_ACL_ARP_SHA_KEY,
    AIR_ACL_ARP_DIP_KEY,
    AIR_ACL_ARP_SIP_KEY,
    AIR_ACL_ARP_UDF_KEY,
    AIR_ACL_ARP_RULE_FIELD_LAST
} AIR_ACL_ARP_RULE_FIELD_T;

typedef struct AIR_ACL_RULE_S
{
    struct
    {
        UI8_T      dmac[6];
        UI8_T      smac[6];
        UI16_T     etype;
        UI16_T     stag;
        UI16_T     ctag;
        UI32_T     dip;
        UI32_T     sip;
        UI8_T      dscp;
        UI8_T      next_header;
        UI32_T     flow_label;
        UI16_T     dport;
        UI16_T     sport;
        UI16_T     udf;
        AIR_IPV6_T dipv6;
        AIR_IPV6_T sipv6;
    } key;
    struct
    {
        UI8_T  dmac;
        UI8_T  smac;
        UI8_T  etype;
        UI8_T  stag;
        UI8_T  ctag;
        UI8_T  dip;
        UI8_T  sip;
        UI8_T  flow_label;
        UI8_T  dport;
        UI8_T  sport;
        UI16_T udf;
        UI16_T dipv6;
        UI16_T sipv6;
    } mask;
    UI16_T            field_type;
    UI8_T             isipv6;
    UI8_T             end;
    UI8_T             rule_en;
    UI8_T             reverse;
    AIR_PORT_BITMAP_T portmap;
    UI16_T            field_valid;
    UI8_T             end_reverse;
} AIR_ACL_RULE_T;

typedef struct AIR_ACL_ARP_RULE_S
{
    struct
    {
        UI8_T  dmac[6];
        UI8_T  smac[6];
        UI16_T stag;
        UI16_T ctag;
        UI8_T  tha[6];
        UI8_T  sha[6];
        UI32_T dip;
        UI32_T sip;
        UI16_T udf;
    } key;
    struct
    {
        UI8_T  dmac;
        UI8_T  smac;
        UI8_T  stag;
        UI8_T  ctag;
        UI8_T  tha;
        UI8_T  sha;
        UI8_T  dip;
        UI8_T  sip;
        UI16_T udf;
    } mask;
    UI8_T             end;
    UI8_T             rule_en;
    UI8_T             reverse;
    AIR_PORT_BITMAP_T portmap;
    UI16_T            field_valid;
    UI8_T             end_reverse;
} AIR_ACL_ARP_RULE_T;

typedef struct AIR_ACL_ACTION_S
{
    AIR_PORT_BITMAP_T portmap;
    UI8_T             dst_port_swap;
    UI8_T             vlan_port_swap;
    UI8_T             acl_mib_id;
    UI8_T             attack_rate_id;
    UI8_T             meter_id;
    UI8_T             port_fw;
    UI8_T             mirror_port;
    UI8_T             pri_user;
    UI8_T             eg_tag;
    UI8_T             lky_vlan;
    UI8_T             acl_mang;
    UI8_T             drop_pcd_sel;
    UI8_T             drop_pcd_r;
    UI8_T             drop_pcd_y;
    UI8_T             drop_pcd_g;
    UI8_T             class_slr_sel;
    UI8_T             class_slr;
    UI8_T             acl_tcm_sel;
    UI8_T             def_color;
    UI8_T             trtcm_id;
    UI16_T            acl_vlan_vid;
    UI8_T             da_index;
    UI8_T             sa_index;
    UI32_T            field_valid;
    UI8_T             dont_learn;
    UI8_T             as_bpdu;
} AIR_ACL_ACTION_T;

typedef struct AIR_ACL_FIELD_S
{
    UI8_T port_en;
    UI8_T mib_en;
    UI8_T attack_rate_en;
    UI8_T rate_en;
    UI8_T port_fw_en;
    UI8_T mirror_en;
    UI8_T pri_user_en;
    UI8_T eg_tag_en;
    UI8_T lky_vlan_en;
    UI8_T mang_en;
    UI8_T trtcm_en;
    UI8_T drop_pcd_en;
    UI8_T class_slr_en;
    UI8_T trtcm_idx_en;
    UI8_T vlan_en;
    UI8_T mac_chg;
} AIR_ACL_FIELD_T;

typedef struct AIR_ACL_UDF_RULE_S
{
    UI8_T             udf_rule_en;
    AIR_PORT_BITMAP_T portmap;
    AIR_ACL_PKT_TP_T  udf_pkt_type;
    UI8_T             word_ofst;
    UI8_T             cmp_sel;
    UI16_T            cmp_pat;
    UI16_T            cmp_mask;
} AIR_ACL_UDF_RULE_T;

typedef struct AIR_ACL_TRTCM_S
{
    /* Committed information rate (unit: 64Kbps) */
    UI32_T cir;

    /* Peak information rate (unit: 64Kbps) */
    UI32_T pir;

    /* Committed burst size (unit: byte) */
    UI32_T cbs;

    /* Peak burst size (unit: byte) */
    UI32_T pbs;
} AIR_ACL_TRTCM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      ptr_enable               -- Enable state
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getGlobalState(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit                     -- Device unit number
 *      enable                   -- Enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setGlobalState(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:   air_acl_getPortState
 * PURPOSE:
 *      Get enable status of ACL on specified port.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 * OUTPUT:
 *      ptr_enable               -- Enable state
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getPortState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_acl_setPortState
 * PURPOSE:
 *      Set enable state of ACL on specified port.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      enable                   -- Enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setPortState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_acl_clearAll
 * PURPOSE:
 *      Clear ACL all rule and action HW memory.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearAll(
    const UI32_T unit);

/* FUNCTION NAME:   air_acl_getRule
 * PURPOSE:
 *      Get ACL rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit numbe
 *      entry_idx                -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule                 -- Pointer buffer of rule
 *                                  AIR_ACL_RULE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:   air_acl_setRule
 * PURPOSE:
 *      Add/Update ACL rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL rule entry index
 *      ptr_rule                 -- Pointer buffer of rule
 *                                  AIR_ACL_RULE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      1.If isipv6=1, dip&sip&dscp&next header will hit only
 *        when packet is IPv6;
 *      2.Every 4 rule can do aggregation.
 *        For rule aggregation, final hit id rule-end must be 1;
 *        When rule-end=0, this rule can be aggregated with next rule;
 *        Rule3 cannot aggregate with rule4, if rule3 end=0,
 *        rule3 will be an invalid rule;
 *      3.Do reverse function before rule aggregation.
 *      4.When byte mask of each field is set 0,
 *        key of each field must be set 0;
 *      5.Udf-list is bit mask of udf-rule entry, range 1-16;
 */
AIR_ERROR_NO_T
air_acl_setRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:   air_acl_delRule
 * PURPOSE:
 *      Delete ACL rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delRule(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:   air_acl_getAction
 * PURPOSE:
 *      Get ACL action of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL action entry index
 * OUTPUT:
 *      ptr_action               -- Pointer buffer of action
 *                                  AIR_ACL_ACTION_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:   air_acl_setAction
 * PURPOSE:
 *      Add/Update ACL action of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL action entry index
 *      ptr_action               -- Pointer buffer of action
 *                                  AIR_ACL_ACTION_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      If acl-tcm=defined, the corresponding action is defined-color;
 *      If acl-tcm=meter, the corresponding action is meter-id;
 */
AIR_ERROR_NO_T
air_acl_setAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:   air_acl_delAction
 * PURPOSE:
 *      Delete ACL action of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL action entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delAction(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:   air_acl_getBlockState
 * PURPOSE:
 *      Get ACL block combine state.
 * INPUT:
 *      unit                     -- Device unit number
 *      block                    -- Block id
 *                                  AIR_ACL_BLOCK_T
 * OUTPUT:
 *      ptr_enable               -- Enable state
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by
 *      setting block combination state;
 *      When combination=enable, this block can combine with
 *      next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
air_acl_getBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    BOOL_T               *ptr_enable);

/* FUNCTION NAME:   air_acl_setBlockState
 * PURPOSE:
 *      Set block combination state.
 * INPUT:
 *      unit                     -- Device unit number
 *      block                    -- Block id
 *                                  AIR_ACL_BLOCK_T
 *      enable                   -- Enable state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by
 *      setting block combination state;
 *      When combination=enable, this block can combine with
 *      next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
air_acl_setBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const BOOL_T          enable);

/* FUNCTION NAME:   air_acl_getMibCnt
 * PURPOSE:
 *      Get ACL mib counter.
 * INPUT:
 *      unit                     -- Device unit number
 *      cnt_index                -- Mib counter index
 * OUTPUT:
 *      ptr_cnt                  -- Pointer to receive count
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      ACL has total 64 mib counters, the counter index
 *      can be set by hal_sco_acl_setAction.
 */
AIR_ERROR_NO_T
air_acl_getMibCnt(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt);

/* FUNCTION NAME:   air_acl_clearMibCnt
 * PURPOSE:
 *      Clear ACL mib counter.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearMibCnt(
    const UI32_T unit);

/* FUNCTION NAME:   air_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule             -- Pointer buffer of rule
 *                                  AIR_ACL_UDF_RULE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:   air_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACLUDF table entry index
 *      ptr_udf_rule             -- Pointer buffer of rule
 *                                  AIR_ACL_UDF_RULE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      If cmp=pattern, the corresponding setting is cmp-key&cmp-mask;
 *      If cmp=threshold, the corresponding setting is
 *      cmp-low&cmp-high;
 */
AIR_ERROR_NO_T
air_acl_setUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:   air_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delUdfRule(
    const UI32_T unit,
    const UI8_T  entry_idx);

/* FUNCTION NAME:   air_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearUdfRule(
    const UI32_T unit);

/* FUNCTION NAME:   air_acl_getMeterTable
 * PURPOSE:
 *      get meter table configuration.
 * INPUT:
 *      unit                     -- Device unit number
 *      meter_id                 -- Meter id
 * OUTPUT:
 *      ptr_enable               -- Meter enable state
 *      ptr_rate                 -- Ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate);

/* FUNCTION NAME:   air_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                     -- Device unit number
 *      meter_id                 -- Meter id
 *      enable                   -- Meter enable state
 *      rate                     -- Ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate);

/* FUNCTION NAME:   air_acl_getDropEn
 * PURPOSE:
 *      Get enable state of drop precedence on specified port.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 * OUTPUT:
 *      ptr_enable               -- Enable state
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getDropEn(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_acl_setDropEn
 * PURPOSE:
 *      Set enable state of drop precedence on specified port.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      enable                   -- Enable state of drop precedence
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setDropEn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_acl_getDropThrsh
 * PURPOSE:
 *      Get ACL drop threshold.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      color                    -- AIR_ACL_DP_COLOR_T
 *      queue                    -- Output queue number
 * OUTPUT:
 *      ptr_high                 -- High threshold
 *      ptr_low                  -- Low threshold
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low);

/* FUNCTION NAME:   air_acl_setDropThrsh
 * PURPOSE:
 *      Set ACL drop threshold.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      color                    -- AIR_ACL_DP_COLOR_T
 *      queue                    -- Output queue number
 *      high                     -- High threshold
 *      low                      -- Low threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low);

/* FUNCTION NAME:   air_acl_getDropPbb
 * PURPOSE:
 *      Get ACL drop probability.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      color                    -- AIR_ACL_DP_COLOR_T
 *      queue                    -- Output queue number
 * OUTPUT:
 *      ptr_pbb                  -- Drop probability(unit:1/1023)
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb);

/* FUNCTION NAME:   air_acl_setDropPbb
 * PURPOSE:
 *      Set ACL drop probability.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port id
 *      color                    -- AIR_ACL_DP_COLOR_T
 *      queue                    -- Output queue number
 *      pbb                      -- Drop probability(unit:1/1023)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb);

/* FUNCTION NAME:   air_acl_getDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      ptr_type                 -- TRUE: Exclude management frame
 *                                  FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getDropExMfrm(
    const UI32_T unit,
    BOOL_T      *ptr_type);

/* FUNCTION NAME:   air_acl_setDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit                     -- Device unit number
 *      type                     -- TRUE: Exclude management frame
 *                                  FALSE:Include management frame
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setDropExMfrm(
    const UI32_T unit,
    const BOOL_T type);

/* FUNCTION NAME:   air_acl_getTrtcmEn
 * PURPOSE:
 *      Get TRTCM enable status.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      ptr_enable               -- Enable/Disable TRTCM
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getTrtcmEn(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_acl_setTrtcmEn
 * PURPOSE:
 *      Set TRTCM enable status.
 * INPUT:
 *      unit                     -- Device unit number
 *      enable                   -- Enable/Disable TRTCM
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setTrtcmEn(
    const UI32_T unit,
    BOOL_T       enable);

/* FUNCTION NAME:   air_acl_getTrtcmTable
 * PURPOSE:
 *      Get a TRTCM entry with the specific index.
 * INPUT:
 *      unit                     -- Device unit number
 *      tcm_idx                  -- Index of TRTCM entry
 * OUTPUT:
 *      ptr_tcm                  -- TRTCM entry
 *                                  AIR_ACL_TRTCM_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:   air_acl_setTrtcmTable
 * PURPOSE:
 *      Set a TRTCM entry with the specific index.
 * INPUT:
 *      unit                     -- Device unit number
 *      tcm_idx                  -- Index of TRTCM entry
 *      ptr_tcm                  -- TRTCM entry
 *                                  AIR_ACL_TRTCM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:   air_acl_getL3RoutingTable
 * PURPOSE:
 *      Get DA/SA of the specific index in routing address table.
 * INPUT:
 *      unit                     -- Device unit number
 *      mac_sel                  -- Select DA or SA
 *      id                       -- The specific table index
 * OUTPUT:
 *      ptr_mac                  -- Routing address
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getL3RoutingTable(
    const UI32_T unit,
    const UI32_T mac_sel,
    const UI32_T id,
    AIR_MAC_T    ptr_mac);

/* FUNCTION NAME:   air_acl_setL3RoutingTable
 * PURPOSE:
 *      Set DA/SA routing address table.
 * INPUT:
 *      unit                     -- Device unit number
 *      mac_sel                  -- Select DA or SA
 *      id                       -- The specific table index
 *      ptr_mac                  -- Routing address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setL3RoutingTable(
    const UI32_T    unit,
    const UI32_T    mac_sel,
    const UI32_T    id,
    const AIR_MAC_T ptr_mac);

/* FUNCTION NAME:   air_acl_getArpRule
 * PURPOSE:
 *      Get ACL ARP rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule                 -- Pointer buffer of rule
 *                                  AIR_ACL_ARP_RULE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

/* FUNCTION NAME:   air_acl_setArpRule
 * PURPOSE:
 *      Add/Update ACL ARP rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL ARP rule entry index
 *      ptr_rule                 -- Pointer buffer of rule
 *                                  AIR_ACL_ARP_RULE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      1.When byte mask of each field is set 0,
 *        key of each field must be set 0;
 *      2.Udf-list is bit mask of udf-rule entry, range 1-16;
 */
AIR_ERROR_NO_T
air_acl_setArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

/* FUNCTION NAME:   air_acl_delArpRule
 * PURPOSE:
 *      Delete ACL ARP rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL ARP rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delArpRule(
    const UI32_T unit,
    const UI32_T entry_idx);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
 */
#endif /* End of AIR_ACL_H */
