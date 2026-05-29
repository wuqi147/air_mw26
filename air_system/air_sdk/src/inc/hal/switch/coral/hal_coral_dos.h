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

/* FILE NAME:  hal_coral_dos.h
 * PURPOSE:
 *  Define DoS attack prevention module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_DOS_H
#define HAL_CORAL_DOS_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_dos.h>
#include <air_error.h>
#include <air_port.h>
#include <air_swc.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_DOS_MAX_BUSY_TIME     (20)
#define HAL_CORAL_DOS_DELAY_US          (1000)
#define HAL_CORAL_DOS_MIN_ATTACK_LENGTH (0)
#define HAL_CORAL_DOS_MAX_ATTACK_LENGTH (BITS(0, (CORAL_ATTACK_LENGTH_THLD_LENG - 1))) /* 0xffff */
#define HAL_CORAL_DOS_MIN_RATE_ID       (0)
#define HAL_CORAL_DOS_MAX_RATE_ID       (95)
#define HAL_CORAL_DOS_MIN_RATE_WR_CFG   (0)
#define HAL_CORAL_DOS_MAX_RATE_WR_CFG   (BITS(0, (CORAL_ATTACK_RATE_WR_CFG_LENG - 1))) /* 0xffff */

/* DATA TYPE DECLARATIONS
 */
/* the attack type enum sequence follows the hardware definition
 */
typedef enum
{
    HAL_CORAL_DOS_ATTACK_LAND = 0,
    HAL_CORAL_DOS_ATTACK_BLAT,
    HAL_CORAL_DOS_ATTACK_LENGTH,
    HAL_CORAL_DOS_ATTACK_TCP0,
    HAL_CORAL_DOS_ATTACK_TCP1,
    HAL_CORAL_DOS_ATTACK_TCP2,
    HAL_CORAL_DOS_ATTACK_PING,
    HAL_CORAL_DOS_ATTACK_TCP_HDR_MIN,
    HAL_CORAL_DOS_ATTACK_IP_LAND,
    HAL_CORAL_DOS_ATTACK_LAST
} HAL_CORAL_DOS_ATTACK_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_coral_dos_init
 * PURPOSE:
 *      Initialization of DoS.
 * INPUT:
 *      unit    -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_dos_deinit
 * PURPOSE:
 *      Deinitialization of DoS.
 * INPUT:
 *      unit    -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_dos_setGlobalCfg
 * PURPOSE:
 *      Set DoS global state.
 * INPUT:
 *      unit     -- Device unit number
 *      enable   -- DoS global state
 *                  FALSE: Disable
 *                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_setGlobalCfg(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME: hal_coral_dos_getGlobalCfg
 * PURPOSE:
 *      Get DoS global state.
 * INPUT:
 *      unit         -- Device unit number
 * OUTPUT:
 *      ptr_enable   -- DoS global state
 *                      FALSE: Disable
 *                      TRUE: Enable
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getGlobalCfg(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_coral_dos_setIcmpLengthLimit
 * PURPOSE:
 *      Set DoS length attack threshold.
 * INPUT:
 *      unit    -- Device unit number
 *      length  -- DoS length attack threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_setIcmpLengthLimit(
    const UI32_T unit,
    const UI32_T length);

/* FUNCTION NAME: hal_coral_dos_getIcmpLengthLimit
 * PURPOSE:
 *      Get DoS length attack threshold.
 * INPUT:
 *      unit        -- Device unit number
 * OUTPUT:
 *      ptr_length  -- DoS length attack threshold
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getIcmpLengthLimit(
    const UI32_T unit,
    UI32_T      *ptr_length);

/* FUNCTION NAME: hal_coral_dos_setTcpFlagCfg
 * PURPOSE:
 *      Set DoS TCP flag which regard as attack.
 * INPUT:
 *      unit            -- Device unit number
 *      idx             -- TCP setting id
 *      ptr_tcp_cfg     -- TCP flags setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_setTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg);

/* FUNCTION NAME: hal_coral_dos_getTcpFlagCfg
 * PURPOSE:
 *      Get DoS TCP setting.
 * INPUT:
 *      unit            -- Device unit number
 *      idx             -- TCP setting id
 * OUTPUT:
 *      ptr_tcp_cfg     -- TCP flag setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg);

/* FUNCTION NAME: hal_coral_dos_setPortActionCfg
 * PURPOSE:
 *      Set DoS attack action on port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 *      ptr_action_cfg  -- Port action setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_setPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg);

/* FUNCTION NAME: hal_coral_dos_getPortActionCfg
 * PURPOSE:
 *      Get DoS attack action on port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 * OUTPUT:
 *      ptr_action_cfg  -- Port action setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg);

/* FUNCTION NAME: hal_coral_dos_clearPortActionCfg
 * PURPOSE:
 *      Clear DoS attack action on specific port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_clearPortActionCfg(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_coral_dos_clearActionCfg
 * PURPOSE:
 *      Clear DoS attack action on all port.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_clearActionCfg(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_dos_setRateLimitCfg
 * PURPOSE:
 *      Set DoS rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 *      ptr_rate_cfg    -- Rate limit setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg);

/* FUNCTION NAME: hal_coral_dos_getRateLimitCfg
 * PURPOSE:
 *      Get DoS rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 * OUTPUT:
 *      ptr_rate_cfg    -- Rate limit setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg);

/* FUNCTION NAME: hal_coral_dos_clearRateLimitCfg
 * PURPOSE:
 *      Clear DoS attack rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_clearRateLimitCfg(
    const UI32_T unit,
    const UI32_T rate_cfg_idx);

/* FUNCTION NAME: hal_coral_dos_clearAllRateLimitCfg
 * PURPOSE:
 *      Clear all DoS attack rate limit rules.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_clearAllRateLimitCfg(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_dos_getDropCnt
 * PURPOSE:
 *      Get packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      ptr_drop_cnt    -- packet drop counter
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_getDropCnt(
    const UI32_T unit,
    UI32_T      *ptr_drop_cnt);

/* FUNCTION NAME: hal_coral_dos_clearDropCnt
 * PURPOSE:
 *      Clear packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_dos_clearDropCnt(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_dos_getCapacity
 * PURPOSE:
 *      Get the dos resource capacity
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
hal_coral_dos_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_coral_dos_getUsage
 * PURPOSE:
 *      Get the dos resource usage
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
hal_coral_dos_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* end of HAL_CORAL_DOS_H */
