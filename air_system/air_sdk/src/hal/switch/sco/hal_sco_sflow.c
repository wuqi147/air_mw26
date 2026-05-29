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

/* FILE NAME:  hal_sco_sflow.c
 * PURPOSE:
 *    It provides HAL sFlow driver API functions.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_sflow.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_SFLOW_SFCR_DEFAULT_VALUE (0x01000000)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SFLOW, "hal_sco_sflow.c");

/* FUNCTION NAME: hal_sco_sflow_init
 * PURPOSE:
 *      Initialization of sFlow.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_init(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sflow_deinit
 * PURPOSE:
 *      Deinitialization of sFlow.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_deinit(
    const UI32_T unit)
{
    UI32_T mac_port, u32dat;

    u32dat = HAL_SCO_SFLOW_SFCR_DEFAULT_VALUE;
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), mac_port)
    {
        aml_writeReg(unit, SFCR(mac_port), &u32dat, sizeof(u32dat));
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sflow_setSampling
 * PURPOSE:
 *      Set sFlow sampling rate, number
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      rate            --  sFlow sampling rate
 *      number          --  sFlow sampling number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1.  Don't set sflow sampling on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_sflow_setSampling(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T rate,
    const UI32_T number)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    HAL_CHECK_MIN_MAX_RANGE(rate, HAL_SCO_SFLOW_MIN_SAMPLING_RATE, HAL_SCO_SFLOW_MAX_SAMPLING_RATE);
    HAL_CHECK_MIN_MAX_RANGE(number, HAL_SCO_SFLOW_MIN_SAMPLING_NUM, HAL_SCO_SFLOW_MAX_SAMPLING_NUM);
    /* Set to register */
    aml_readReg(unit, SFCR(mac_port), &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from SFCR(%u)=0x%x\n", unit, u32dat, mac_port, SFCR(mac_port));
    /* Set sampling rate */
    u32dat &= ~(BITS_RANGE(SFLOW_SAMPLING_RATE_OFFT, SFLOW_SAMPLING_RATE_LENG));
    u32dat |= (rate << SFLOW_SAMPLING_RATE_OFFT);
    /* Set sampling number */
    u32dat &= ~(BITS_RANGE(SFLOW_SAMPLING_NUM_OFFT, SFLOW_SAMPLING_NUM_LENG));
    u32dat |= (number << SFLOW_SAMPLING_NUM_OFFT);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to SFCR(%u)\n", unit, u32dat, mac_port);
    aml_writeReg(unit, SFCR(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sflow_getSampling
 * PURPOSE:
 *      Get sFlow sampling rate, number.
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 * OUTPUT:
 *      ptr_rate        --  sFlow sampling rate
 *      ptr_numbe       --  sFlow sampling number
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_getSampling(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_rate,
    UI32_T      *ptr_number)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /* Read data from register */
    aml_readReg(unit, SFCR(mac_port), &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from SFCR(%u)=0x%x\n", unit, u32dat, mac_port, SFCR(mac_port));
    (*ptr_rate) = BITS_OFF_R(u32dat, SFLOW_SAMPLING_RATE_OFFT, SFLOW_SAMPLING_RATE_LENG);
    (*ptr_number) = BITS_OFF_R(u32dat, SFLOW_SAMPLING_NUM_OFFT, SFLOW_SAMPLING_NUM_LENG);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from SFCR(%u), sampling rate=%u, number=%u\n", unit, u32dat, mac_port,
               *ptr_rate, *ptr_number);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sflow_setMngFrm
 * PURPOSE:
 *      Set sFlow sampling to include/exclude management frame
 * INPUT:
 *      unit            --  Select device ID
 *      state           --  FALSE: Include management frames
 *                          TRUE: Exclude management frames
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1. This API is internal now.
 *          Because MRD didn't require this function, HEC didn't verify this part.
 *          Currently, it occur expected counter when setting state state to exclude.
 */
AIR_ERROR_NO_T
hal_sco_sflow_setMngFrm(
    const UI32_T unit,
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Set to register */
    aml_readReg(unit, GIRLCR, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from GIRLCR=0x%x\n", unit, u32dat, GIRLCR);
    if (TRUE == state)
    {
        u32dat |= BIT(SFLOW_MFRM_EX_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(SFLOW_MFRM_EX_OFFT));
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to GIRLCR\n", unit, u32dat);
    aml_writeReg(unit, GIRLCR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sflow_getMngFrm
 * PURPOSE:
 *      Get sFlow sampling state of include/exclude management frame.
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_state       --  FALSE: Include management frames
 *                          TRUE: Exclude management frames
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 * NOTES:
 *      1. This API is internal now.
 *          Because MRD didn't require this function, HEC didn't verify this part.
 *          Currently, it occur expected counter when setting state state to exclude.
 */
AIR_ERROR_NO_T
hal_sco_sflow_getMngFrm(
    const UI32_T unit,
    BOOL_T      *ptr_state)
{
    UI32_T u32dat = 0;

    /* Read data from register */
    aml_readReg(unit, GIRLCR, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from GIRLCR=0x%x\n", unit, u32dat, GIRLCR);
    if (BITS_OFF_R(u32dat, SFLOW_MFRM_EX_OFFT, SFLOW_MFRM_EX_LENG))
    {
        *ptr_state = TRUE;
    }
    else
    {
        *ptr_state = FALSE;
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from GIRLCR, mgnFrm state=%d\n", unit, u32dat, *ptr_state);
    return AIR_E_OK;
}
