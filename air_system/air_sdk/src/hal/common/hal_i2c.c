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

/* FILE NAME:  hal_i2c.c
 * PURPOSE:
 *  Implement I2C module API function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/common/hal_i2c.h>

#include <hal/common/hal.h>

/* NAMING CONSTANT DECLARATIONS
 */

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

/* FUNCTION NAME: hal_i2c_readReg
 * PURPOSE:
 *      Read the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      slave_id        --  Slave ID
 *      reg_addr        --  Register address
 *      reg_addr_len    --  Register address length
 *      reg_data_len    --  Register data length
 *      ptr_reg_data    --  Register data
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_i2c_readReg(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T slave_id,
    const UI32_T reg_addr,
    const UI16_T reg_addr_len,
    const UI16_T reg_data_len,
    UI32_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    UI8_T           data[AIR_SIF_MAX_DATA_LEN] = {0};

    sif_info.channel = bus_id;
    sif_info.slave_id = slave_id;

    sifm_param.addr_len = reg_addr_len;
    sifm_param.addr = reg_addr;
    sifm_param.data_len = reg_data_len;
    if (4 < reg_data_len)
    {
        sifm_param.info.ptr_data = (UI32_T *)data;
    }

    rc = HAL_FUNC_CALL(unit, sif, read, (unit, &sif_info, &sifm_param));
    if (AIR_E_OK == rc)
    {
        if (4 < reg_data_len)
        {
            osal_memcpy(ptr_reg_data, data, reg_data_len);
        }
        else
        {
            osal_memcpy(ptr_reg_data, &sifm_param.info.data, reg_data_len);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_i2c_writeReg
 * PURPOSE:
 *      Write the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  Slave ID
 *      reg_addr        --  Register address
 *      reg_addr_len    --  Register address length
 *      reg_data_len    --  Register data length
 *      reg_data        --  Register data
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_i2c_writeReg(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T slave_id,
    const UI32_T reg_addr,
    const UI16_T reg_addr_len,
    const UI16_T reg_data_len,
    UI32_T      *ptr_reg_data)
{
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    UI32_T          word_data = 0;
    UI8_T           idx;

    sif_info.channel = bus_id;
    sif_info.slave_id = slave_id;

    sifm_param.addr_len = reg_addr_len;
    sifm_param.addr = reg_addr;
    sifm_param.data_len = reg_data_len;
    if (4 < reg_data_len)
    {
        sifm_param.info.ptr_data = ptr_reg_data;
    }
    else
    {
        for (idx = 0; idx < reg_data_len; idx++)
        {
            word_data |= (UI8_T)ptr_reg_data[idx] << (idx * 8);
        }
        sifm_param.info.data = word_data;
    }
    return HAL_FUNC_CALL(unit, sif, write, (unit, &sif_info, &sifm_param));
}
