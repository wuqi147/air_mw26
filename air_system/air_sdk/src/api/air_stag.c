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

/* FILE NAME:  air_stag.c
 * PURPOSE:
 *  Implement STAG module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */

#include <air_stag.h>

#include <air_init.h>
#include <hal/common/hal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_SVLAN, "air_stag.c");

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_STAG_BUF_LEN (4)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   air_stag_setSpecialTagCfg
 * PURPOSE:
 *      Set the STAG port state for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- Enable stag
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_setSpecialTagCfg(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    return HAL_FUNC_CALL(unit, stag, setPort, (unit, port, enable));
}

/* FUNCTION NAME:   air_stag_getSpecialTagCfg
 * PURPOSE:
 *      Set the STAG port state for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- Enable stag
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_getSpecialTagCfg(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);
    return HAL_FUNC_CALL(unit, stag, getPort, (unit, port, ptr_enable));
}

/* FUNCTION NAME:   air_stag_setSpecialTagMode
 * PURPOSE:
 *      Set the STAG port mode for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the stag mode
 *                                  AIR_STAG_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_setSpecialTagMode(
    const UI32_T          unit,
    const UI32_T          port,
    const AIR_STAG_MODE_T mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    return HAL_FUNC_CALL(unit, stag, setMode, (unit, port, mode));
}

/* FUNCTION NAME:   air_stag_getSpecialTagMode
 * PURPOSE:
 *      Get the STAG port mode for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the stag mode
 *                                  AIR_STAG_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_getSpecialTagMode(
    const UI32_T     unit,
    const UI32_T     port,
    AIR_STAG_MODE_T *ptr_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_mode);
    return HAL_FUNC_CALL(unit, stag, getMode, (unit, port, ptr_mode));
}

/* FUNCTION NAME:   air_stag_encodeTxStag
 * PURPOSE:
 *      Encode tx special tag into buffer.
 * INPUT:
 *      unit                     -- Device ID
 *      mode                     -- Stag mode
 *      ptr_stag_tx              -- Stag parameters
 *                                  AIR_STAG_TX_PARA_T
 *      ptr_buf                  -- Buffer address
 *      ptr_len                  -- Buffer length
 * OUTPUT:
 *      ptr_len                  -- Written buffer length
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_encodeTxStag(
    const UI32_T          unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T   *ptr_stag_tx,
    UI8_T                *ptr_buf,
    UI32_T               *ptr_len)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PARAM((mode >= AIR_STAG_MODE_LAST), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PTR(ptr_stag_tx);
    HAL_CHECK_PTR(ptr_buf);
    HAL_CHECK_PTR(ptr_len);

    return HAL_FUNC_CALL(unit, stag, encodeTxStag, (unit, mode, ptr_stag_tx, ptr_buf, ptr_len));
}

/* FUNCTION NAME:   air_stag_decodeRxStag
 * PURPOSE:
 *      Decode rx special tag from buffer.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_buf                  -- Buffer address
 *      len                      -- Buffer length
 * OUTPUT:
 *      ptr_stag_rx              -- Stag parameters
 *                                  AIR_STAG_RX_PARA_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_decodeRxStag(
    const UI32_T        unit,
    const UI8_T        *ptr_buf,
    const UI32_T        len,
    AIR_STAG_RX_PARA_T *ptr_stag_rx)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_buf);
    HAL_CHECK_PTR(ptr_stag_rx);
    HAL_CHECK_PARAM((len != AIR_STAG_BUF_LEN), AIR_E_BAD_PARAMETER);

    return HAL_FUNC_CALL(unit, stag, decodeRxStag, (unit, ptr_buf, len, ptr_stag_rx));
}
