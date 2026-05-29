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

/* FILE NAME:  air_dos.c
 * PURPOSE:
 *    It provides DoS attack prevention module API.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_dos.h>

#include <api/diag.h>
#include <hal/common/hal.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_DOS, "air_dos.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   air_dos_setGlobalCfg
 * PURPOSE:
 *      Set DoS global state.
 * INPUT:
 *      unit                 -- Device unit number
 *      enable               -- DoS global state
 *                              FALSE: Disable
 *                              TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_setGlobalCfg(
    const UI32_T unit,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, dos, setGlobalCfg, (unit, enable));
}

/* FUNCTION NAME:   air_dos_getGlobalCfg
 * PURPOSE:
 *      Get DoS global state.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_enable           -- DoS global state
 *                              FALSE: Disable
 *                              TRUE: Enable
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getGlobalCfg(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, dos, getGlobalCfg, (unit, ptr_enable));
}

/* FUNCTION NAME:   air_dos_setIcmpLengthLimit
 * PURPOSE:
 *      Set DoS length attack threshold.
 * INPUT:
 *      unit                 -- Device unit number
 *      length               -- DoS length attack threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_setIcmpLengthLimit(
    const UI32_T unit,
    const UI32_T length)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, dos, setIcmpLengthLimit, (unit, length));
}

/* FUNCTION NAME:   air_dos_getIcmpLengthLimit
 * PURPOSE:
 *      Get DoS length attack threshold.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_length           -- DoS length attack threshold
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getIcmpLengthLimit(
    const UI32_T unit,
    UI32_T      *ptr_length)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_length);

    return HAL_FUNC_CALL(unit, dos, getIcmpLengthLimit, (unit, ptr_length));
}

/* FUNCTION NAME:   air_dos_setTcpFlagCfg
 * PURPOSE:
 *      Set DoS TCP flag which regard as attack.
 * INPUT:
 *      unit                 -- Device unit number
 *      idx                  -- TCP setting id
 *      ptr_tcp_cfg          -- TCP flags setting
 *                              AIR_DOS_TCP_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_setTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_tcp_cfg);

    return HAL_FUNC_CALL(unit, dos, setTcpFlagCfg, (unit, idx, ptr_tcp_cfg));
}

/* FUNCTION NAME:   air_dos_getTcpFlagCfg
 * PURPOSE:
 *      Get DoS TCP setting.
 * INPUT:
 *      unit                 -- Device unit number
 *      idx                  -- TCP setting id
 * OUTPUT:
 *      ptr_tcp_cfg          -- TCP flag setting
 *                              AIR_DOS_TCP_CFG_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_tcp_cfg);

    return HAL_FUNC_CALL(unit, dos, getTcpFlagCfg, (unit, idx, ptr_tcp_cfg));
}

/* FUNCTION NAME:   air_dos_setPortActionCfg
 * PURPOSE:
 *      Set DoS attack action on port.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Index of port number
 *      ptr_action_cfg       -- Port action setting
 *                              AIR_DOS_PORT_ACTION_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_setPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_action_cfg);

    return HAL_FUNC_CALL(unit, dos, setPortActionCfg, (unit, port, ptr_action_cfg));
}

/* FUNCTION NAME:   air_dos_getPortActionCfg
 * PURPOSE:
 *      Get DoS attack action on port.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Index of port number
 * OUTPUT:
 *      ptr_action_cfg       -- Port action setting
 *                              AIR_DOS_PORT_ACTION_CFG_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_action_cfg);

    return HAL_FUNC_CALL(unit, dos, getPortActionCfg, (unit, port, ptr_action_cfg));
}

/* FUNCTION NAME:   air_dos_clearPortActionCfg
 * PURPOSE:
 *      Clear DoS attack action on specific port.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_clearPortActionCfg(
    const UI32_T unit,
    const UI32_T port)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);

    return HAL_FUNC_CALL(unit, dos, clearPortActionCfg, (unit, port));
}

/* FUNCTION NAME:   air_dos_clearActionCfg
 * PURPOSE:
 *      Clear DoS attack action on all port.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_clearActionCfg(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, dos, clearActionCfg, (unit));
}

/* FUNCTION NAME:   air_dos_setRateLimitCfg
 * PURPOSE:
 *      Set DoS rate limit rule on specific id.
 * INPUT:
 *      unit                 -- Device unit number
 *      rate_cfg_idx         -- Index of rate limit
 *      ptr_rate_cfg         -- Rate limit setting
 *                              AIR_DOS_RATE_LIMIT_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rate_cfg);

    return HAL_FUNC_CALL(unit, dos, setRateLimitCfg, (unit, rate_cfg_idx, ptr_rate_cfg));
}

/* FUNCTION NAME:   air_dos_getRateLimitCfg
 * PURPOSE:
 *      Get DoS rate limit rule on specific id.
 * INPUT:
 *      unit                 -- Device unit number
 *      rate_cfg_idx         -- Index of rate limit
 * OUTPUT:
 *      ptr_rate_cfg         -- Rate limit setting
 *                              AIR_DOS_RATE_LIMIT_CFG_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_rate_cfg);

    return HAL_FUNC_CALL(unit, dos, getRateLimitCfg, (unit, rate_cfg_idx, ptr_rate_cfg));
}

/* FUNCTION NAME:   air_dos_clearRateLimitCfg
 * PURPOSE:
 *      Clear DoS attack rate limit rule on specific id.
 * INPUT:
 *      unit                 -- Device unit number
 *      rate_cfg_idx         -- Index of rate limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_clearRateLimitCfg(
    const UI32_T unit,
    const UI32_T rate_cfg_idx)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, dos, clearRateLimitCfg, (unit, rate_cfg_idx));
}

/* FUNCTION NAME:   air_dos_clearAllRateLimitCfg
 * PURPOSE:
 *      Clear all DoS attack rate limit rules.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_clearAllRateLimitCfg(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, dos, clearAllRateLimitCfg, (unit));
}

/* FUNCTION NAME:   air_dos_getDropCnt
 * PURPOSE:
 *      Get packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_drop_cnt         -- packet drop counter
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_getDropCnt(
    const UI32_T unit,
    UI32_T      *ptr_drop_cnt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_drop_cnt);

    return HAL_FUNC_CALL(unit, dos, getDropCnt, (unit, ptr_drop_cnt));
}

/* FUNCTION NAME:   air_dos_clearDropCnt
 * PURPOSE:
 *      Clear packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_dos_clearDropCnt(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, dos, clearDropCnt, (unit));
}
