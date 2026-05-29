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

/* FILE NAME:  air_acl.c
 * PURPOSE:
 *    It provide acl module API.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_acl.h>

#include <air_error.h>
#include <air_init.h>
#include <air_types.h>
#include <hal/common/hal.h>
#include <osal/osal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_ACL, "air_acl.c");

/* EXPORTED SUBPROGRAM BODIES
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
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, acl, getGlobalState, (unit, ptr_enable));
}

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
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setGlobalState, (unit, enable));
}

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
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, acl, getPortState, (unit, port, ptr_enable));
}

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
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setPortState, (unit, port, enable));
}

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
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, clearAll, (unit));
}

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
    AIR_ACL_RULE_T *ptr_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rule);

    return HAL_FUNC_CALL(unit, acl, getRule, (unit, entry_idx, ptr_rule));
}

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
    AIR_ACL_RULE_T *ptr_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rule);

    return HAL_FUNC_CALL(unit, acl, setRule, (unit, entry_idx, ptr_rule));
}

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
    const UI32_T entry_idx)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, delRule, (unit, entry_idx));
}

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
    AIR_ACL_ACTION_T *ptr_action)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_action);

    return HAL_FUNC_CALL(unit, acl, getAction, (unit, entry_idx, ptr_action));
}

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
    AIR_ACL_ACTION_T *ptr_action)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_action);

    return HAL_FUNC_CALL(unit, acl, setAction, (unit, entry_idx, ptr_action));
}

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
    const UI32_T entry_idx)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, delAction, (unit, entry_idx));
}

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
    BOOL_T               *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, acl, getBlockState, (unit, block, ptr_enable));
}

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
    const BOOL_T          enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setBlockState, (unit, block, enable));
}

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
    UI32_T      *ptr_cnt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_cnt);

    return HAL_FUNC_CALL(unit, acl, getMibCnt, (unit, cnt_index, ptr_cnt));
}

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
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, clearMibCnt, (unit));
}

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
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_udf_rule);

    return HAL_FUNC_CALL(unit, acl, getUdfRule, (unit, entry_idx, ptr_udf_rule));
}

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
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_udf_rule);

    return HAL_FUNC_CALL(unit, acl, setUdfRule, (unit, entry_idx, ptr_udf_rule));
}

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
    const UI8_T  entry_idx)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, delUdfRule, (unit, entry_idx));
}

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
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, clearUdfRule, (unit));
}

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
    UI32_T      *ptr_rate)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);
    HAL_CHECK_PTR(ptr_rate);

    return HAL_FUNC_CALL(unit, acl, getMeterTable, (unit, meter_id, ptr_enable, ptr_rate));
}

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
    const UI32_T rate)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setMeterTable, (unit, meter_id, enable, rate));
}

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
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, acl, getDropEn, (unit, port, ptr_enable));
}

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
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setDropEn, (unit, port, enable));
}

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
    UI32_T                  *ptr_low)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(color, AIR_ACL_DP_COLOR_LAST);
    HAL_CHECK_MIN_MAX_RANGE(queue, 0, AIR_QOS_QUEUE_MAX_NUM - 1);
    HAL_CHECK_PTR(ptr_high);
    HAL_CHECK_PTR(ptr_low);

    return HAL_FUNC_CALL(unit, acl, getDropThrsh, (unit, port, color, queue, ptr_high, ptr_low));
}

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
    const UI32_T             low)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(color, AIR_ACL_DP_COLOR_LAST);
    HAL_CHECK_MIN_MAX_RANGE(queue, 0, AIR_QOS_QUEUE_MAX_NUM - 1);

    return HAL_FUNC_CALL(unit, acl, setDropThrsh, (unit, port, color, queue, high, low));
}

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
    UI32_T                  *ptr_pbb)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(color, AIR_ACL_DP_COLOR_LAST);
    HAL_CHECK_MIN_MAX_RANGE(queue, 0, AIR_QOS_QUEUE_MAX_NUM - 1);
    HAL_CHECK_PTR(ptr_pbb);

    return HAL_FUNC_CALL(unit, acl, getDropPbb, (unit, port, color, queue, ptr_pbb));
}

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
    const UI32_T             pbb)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(color, AIR_ACL_DP_COLOR_LAST);
    HAL_CHECK_MIN_MAX_RANGE(queue, 0, AIR_QOS_QUEUE_MAX_NUM - 1);

    return HAL_FUNC_CALL(unit, acl, setDropPbb, (unit, port, color, queue, pbb));
}

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
    BOOL_T      *ptr_type)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_type);

    return HAL_FUNC_CALL(unit, acl, getDropExMfrm, (unit, ptr_type));
}

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
    const BOOL_T type)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(type);

    return HAL_FUNC_CALL(unit, acl, setDropExMfrm, (unit, type));
}

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
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, acl, getTrtcmEn, (unit, ptr_enable));
}

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
    BOOL_T       enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, acl, setTrtcmEn, (unit, enable));
}

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
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_tcm);

    return HAL_FUNC_CALL(unit, acl, getTrtcmTable, (unit, tcm_idx, ptr_tcm));
}

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
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_tcm);

    return HAL_FUNC_CALL(unit, acl, setTrtcmTable, (unit, tcm_idx, ptr_tcm));
}

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
    AIR_MAC_T    ptr_mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(mac_sel, AIR_RAT_MAC_SEL_LAST);
    HAL_CHECK_PTR(ptr_mac);

    return HAL_FUNC_CALL(unit, acl, getL3RoutingTable, (unit, mac_sel, id, ptr_mac));
}

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
    const AIR_MAC_T ptr_mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(mac_sel, AIR_RAT_MAC_SEL_LAST);
    HAL_CHECK_PTR(ptr_mac);

    return HAL_FUNC_CALL(unit, acl, setL3RoutingTable, (unit, mac_sel, id, ptr_mac));
}

/* FUNCTION NAME:   air_acl_getArpRule
 * PURPOSE:
 *      Get ACL ARP rule of specified entry index.
 * INPUT:
 *      unit                     -- Device unit number
 *      entry_idx                -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule                 -- Pointer buffer of rule
 *                                 AIR_ACL_ARP_RULE_T
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
    AIR_ACL_ARP_RULE_T *ptr_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rule);

    return HAL_FUNC_CALL(unit, acl, getArpRule, (unit, entry_idx, ptr_rule));
}

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
    AIR_ACL_ARP_RULE_T *ptr_rule)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rule);

    return HAL_FUNC_CALL(unit, acl, setArpRule, (unit, entry_idx, ptr_rule));
}

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
    const UI32_T entry_idx)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, acl, delArpRule, (unit, entry_idx));
}
