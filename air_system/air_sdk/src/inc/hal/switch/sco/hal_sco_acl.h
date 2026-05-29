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

/* FILE NAME:  hal_sco_acl.h
 * PURPOSE:
 *  Define ACL module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_ACL_H
#define HAL_SCO_ACL_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_acl.h>
#include <air_error.h>
#include <air_port.h>
#include <air_qos.h>
#include <air_swc.h>
#include <air_types.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_MAX_NUM_OF_ACL_ENTRY       (512)
#define HAL_SCO_MAX_NUM_OF_MIB_ID          (64)
#define HAL_SCO_MAX_NUM_OF_UDF_ENTRY       (16)
#define HAL_SCO_MAX_NUM_OF_METER_ID        (32)
#define HAL_SCO_MAX_NUM_OF_TOKEN           (0xffff)
#define HAL_SCO_MAX_NUM_OF_VLAN_ID         (4096)
#define HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY     (32)
#define HAL_SCO_MAX_NUM_OF_DROP_PCD        (8)
#define HAL_SCO_MAX_NUM_OF_CLASS_SLR       (8)
#define HAL_SCO_MAX_NUM_OF_USER_PRI        (8)
#define HAL_SCO_MAX_NUM_OF_MIRROR_PORT     (0xf)
#define HAL_SCO_MAX_NUM_OF_ATTACK_RATE_ID  (96)
#define HAL_SCO_MAX_NUM_OF_BLOCK           (4)
#define HAL_SCO_MAX_NUM_OF_LAST_LINE       (32)
#define HAL_SCO_MAX_NUM_OF_OFST_TP         (8)
#define HAL_SCO_MAX_NUM_OF_WORD_OFST       (128)
#define HAL_SCO_MAX_NUM_OF_CMP_SEL         (2)
#define HAL_SCO_MAX_NUM_OF_CMP_PAT         (0xffff)
#define HAL_SCO_MAX_NUM_OF_CMP_BIT         (0xffff)
#define HAL_SCO_MAX_NUM_OF_DMAC_MASK       (0x3f)
#define HAL_SCO_MAX_NUM_OF_SMAC_MASK       (0x3f)
#define HAL_SCO_MAX_NUM_OF_ETYPE_MASK      (0x3)
#define HAL_SCO_MAX_NUM_OF_STAG_MASK       (0x3)
#define HAL_SCO_MAX_NUM_OF_CTAG_MASK       (0x3)
#define HAL_SCO_MAX_NUM_OF_DIP_MASK_IPV4   (0xf)
#define HAL_SCO_MAX_NUM_OF_SIP_MASK_IPV4   (0xf)
#define HAL_SCO_MAX_NUM_OF_DIP_MASK_IPV6   (0xffff)
#define HAL_SCO_MAX_NUM_OF_SIP_MASK_IPV6   (0xffff)
#define HAL_SCO_MAX_NUM_OF_FLOW_LABEL_MASK (0x7)
#define HAL_SCO_MAX_NUM_OF_DPORT_MASK      (0x3)
#define HAL_SCO_MAX_NUM_OF_SPORT_MASK      (0x3)
#define HAL_SCO_ACL_MAX_BUSY_TIME          (10)
#define HAL_SCO_MAX_NUM_OF_CBS             (0xffff)
#define HAL_SCO_MAX_NUM_OF_CIR             (0xffff)
#define HAL_SCO_MAX_NUM_OF_PBS             (0xffff)
#define HAL_SCO_MAX_NUM_OF_PIR             (0xffff)
#define HAL_SCO_ENTRY_NUM_PER_IPV6_RULE    (4)
/* Drop threshold value should not exceed switch free page capacity size */
#define HAL_SCO_MAX_NUM_OF_DROP_THRSH (2047)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    HAL_SCO_ACL_MEM_SEL_RULE = 0,
    HAL_SCO_ACL_MEM_SEL_ACTION,
    HAL_SCO_ACL_MEM_SEL_LAST
} HAL_SCO_ACL_MEM_SEL_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:
 *      hal_sco_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      ptr_enable       -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getGlobalState(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_sco_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit        -- unit id
 *      enable      -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setGlobalState(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_sco_acl_getPortState
 * PURPOSE:
 *      Get enable status of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getPortState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_sco_acl_setPortState
 * PURPOSE:
 *      Set enable state of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 *      enable          -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setPortState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_sco_acl_clearAll
 * PURPOSE:
 *      Clear ACL all rule and action HW memory.
 * INPUT:
 *      unit        -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_clearAll(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_sco_acl_getRule
 * PURPOSE:
 *      Get ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule         -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_sco_acl_setRule
 * PURPOSE:
 *      Add/Update ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 *      ptr_rule         -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1.If isipv6=1, dip&sip&dscp&next header will hit only when packet is IPv6;
 *      2.Every 4 rule can do aggregation. For rule aggregation, final hit id rule-end must be 1;
 *        When rule-end=0, means that it can be aggregated with next rule;
 *        Rule3 cannot aggregate with rule4, if rule3 end=0, rule3 will be an invalid rule;
 *      3.For reverse function use, do reverse before rule aggregation.
 *      4.When byte mask of each field is set 0, key of each field must be set 0;
 *      5.Udf-list is bit mask of udf-rule entry, range 1-16;
 */
AIR_ERROR_NO_T
hal_sco_acl_setRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_sco_acl_delRule
 * PURPOSE:
 *      Delete ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delRule(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:
 *      hal_sco_acl_getAction
 * PURPOSE:
 *      Get ACL action of specified entry index.
 * INPUT:
 *      unit              -- unit id
 *      entry_idx         -- ACL action entry index
 * OUTPUT:
 *      ptr_action        -- pointer buffer of action
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:
 *      hal_sco_acl_setAction
 * PURPOSE:
 *      Add/Update ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 *      ptr_action       -- pointer buffer of action
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If acl-tcm=defined, the corresponding action is defined-color;
 *      If acl-tcm=meter, the corresponding action is meter-id;
 */
AIR_ERROR_NO_T
hal_sco_acl_setAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:
 *      hal_sco_acl_delAction
 * PURPOSE:
 *      Delete ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delAction(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:
 *      hal_sco_acl_getBlockState
 * PURPOSE:
 *      Get ACL block combine state.
 * INPUT:
 *      unit             -- unit id
 *      block            -- block id
 * OUTPUT:
 *      ptr_enable       -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by setting block combination state;
 *      When combination=enable, means this block can combine with next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
hal_sco_acl_getBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    BOOL_T               *ptr_enable);

/* FUNCTION NAME:
 *      hal_sco_acl_setBlockState
 * PURPOSE:
 *      Set block combination state.
 * INPUT:
 *      unit             -- unit id
 *      block            -- block id
 *      enable           -- enable state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by setting block combination state;
 *      When combination=enable, means this block can combine with next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
hal_sco_acl_setBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const BOOL_T          enable);

/* FUNCTION NAME:
 *      hal_sco_acl_getLastLine
 * PURPOSE:
 *      Get specified block last ACL rule row number.
 * INPUT:
 *      unit                -- unit id
 *      block               -- block id
 * OUTPUT:
 *      ptr_last_line       -- last rule row number of specified block
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Each block has total 128 ACL rules, and they will be split as 4 banks, each bank has 32 rows.
 */
AIR_ERROR_NO_T
hal_sco_acl_getLastLine(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    UI32_T               *ptr_last_line);

/* FUNCTION NAME:
 *      hal_sco_acl_setLastLine
 * PURPOSE:
 *      Set specified block last ACL rule row number.
 * INPUT:
 *      unit                -- unit id
 *      block               -- block id
 *      last_line           -- last rule row number of specified block
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Each block has total 128 ACL rules, and they will be split as 4 banks, each bank has 32 rows.
 */
AIR_ERROR_NO_T
hal_sco_acl_setLastLine(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const UI32_T          last_line);

/* FUNCTION NAME:
 *      hal_sco_acl_getMibCnt
 * PURPOSE:
 *      Get ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 *      cnt_index        -- mib counter index
 * OUTPUT:
 *      ptr_cnt          -- pointer to receive count
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has total 64 mib counters, the counter index can be set by hal_sco_acl_setAction.
 */
AIR_ERROR_NO_T
hal_sco_acl_getMibCnt(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt);

/* FUNCTION NAME:
 *      hal_sco_acl_clearMibCnt
 * PURPOSE:
 *      Clear ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      For clear all mib counters, ACL has total 64 mib counters,
 *      the counter index can be set by hal_sco_acl_setAction.
 */
AIR_ERROR_NO_T
hal_sco_acl_clearMibCnt(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_sco_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule     -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:
 *      hal_sco_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 *      ptr_udf_rule     -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If cmp=pattern, the corresponding setting is cmp-key&cmp-mask;
 *      If cmp=threshold, the corresponding setting is cmp-low&cmp-high;
 */
AIR_ERROR_NO_T
hal_sco_acl_setUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:
 *      hal_sco_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delUdfRule(
    const UI32_T unit,
    const UI8_T  entry_idx);

/* FUNCTION NAME:
 *      hal_sco_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_clearUdfRule(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_sco_acl_getMeterTable
 * PURPOSE:
 *      get meter table configuration.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 * OUTPUT:
 *      ptr_enable          -- meter enable state
 *      ptr_rate            -- ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate);

/* FUNCTION NAME:
 *      hal_sco_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 *      enable              -- meter enable state
 *      rate                -- ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate);

/* FUNCTION NAME:
 *      hal_sco_acl_getDropEn
 * PURPOSE:
 *      Get enable state of drop precedence on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropEn(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_sco_acl_setDropEn
 * PURPOSE:
 *      Set enable state of drop precedence on specified port.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      enable      -- enable state of drop precedence
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropEn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_sco_acl_getDropThrsh
 * PURPOSE:
 *      Get ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_high        --  High threshold
 *      ptr_low         --  Low threshold
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low);

/* FUNCTION NAME:
 *      hal_sco_acl_setDropThrsh
 * PURPOSE:
 *      Set ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      high            --  High threshold
 *      low             --  Low threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low);

/* FUNCTION NAME:
 *      hal_sco_acl_getDropPbb
 * PURPOSE:
 *      Get ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_pbb         --  Drop probability(unit:1/1023)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb);

/* FUNCTION NAME:
 *      hal_sco_acl_setDropPbb
 * PURPOSE:
 *      Set ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      pbb             --  Drop probability(unit:1/1023)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb);

/* FUNCTION NAME:
 *      hal_sco_acl_getDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            -- unit id
 * OUTPUT:
 *      ptr_type        -- TRUE: Exclude management frame
 *                         FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropExMfrm(
    const UI32_T unit,
    BOOL_T      *ptr_type);

/* FUNCTION NAME:
 *      hal_sco_acl_setDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            --  unit id
 *      type            --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropExMfrm(
    const UI32_T unit,
    const BOOL_T type);

/* FUNCTION NAME:
 *      hal_sco_acl_getTrtcmEn
 * PURPOSE:
 *      Get TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_enable      --  Enable/Disable trTCM
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getTrtcmEn(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_sco_acl_setTrtcmEn
 * PURPOSE:
 *      Set TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  Enable/Disable TRTCM
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setTrtcmEn(
    const UI32_T unit,
    BOOL_T       enable);

/* FUNCTION NAME:
 *      hal_sco_acl_getTrtcmTable
 * PURPOSE:
 *      Get a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *
 * OUTPUT:
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:
 *      hal_sco_acl_setTrtcmTable
 * PURPOSE:
 *      Set a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:
 *      hal_sco_acl_getL3RoutingTable
 * PURPOSE:
 *      Get DA/SA of the specific index in routing address table.
 *
 * INPUT:
 *      unit            --  device ID
 *      mac_sel         --  It indicates the routing address table select is DA or SA.
 *      id              --  the specific table index
 *
 * OUTPUT:
 *      ptr_mac         --  routing address
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getL3RoutingTable(
    const UI32_T unit,
    const UI32_T mac_sel,
    const UI32_T id,
    AIR_MAC_T    ptr_mac);

/* FUNCTION NAME:
 *      hal_sco_acl_setL3RoutingTable
 * PURPOSE:
 *      Set DA/SA routing address table.
 * INPUT:
 *      unit            --  device ID
 *      mac_sel         --  It indicates the routing address table select is DA or SA.
 *      id              --  the specific table index
 *      ptr_mac         --  routing address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setL3RoutingTable(
    const UI32_T    unit,
    const UI32_T    mac_sel,
    const UI32_T    id,
    const AIR_MAC_T ptr_mac);

/* FUNCTION NAME:
 *      hal_sco_acl_init
 * PURPOSE:
 *      This API is used to init acl.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_init(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_sco_acl_deinit
 * PURPOSE:
 *      This API is used to deinit acl.
 * INPUT:
 *      unit             -- unit id
 *      acl_info         -- acl information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_acl_getCapacity
 * PURPOSE:
 *      Get the acl resource capacity
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 */
AIR_ERROR_NO_T
hal_sco_acl_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_sco_acl_getUsage
 * PURPOSE:
 *      Get the acl resource usage
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_cnt         --  Count of usage
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 */
AIR_ERROR_NO_T
hal_sco_acl_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* end of HAL_SCO_ACL_H */
