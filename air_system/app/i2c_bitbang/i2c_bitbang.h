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
#ifndef __I2C_BITBANG_H__
#define __I2C_BITBANG_H__
#include <air_error.h>
#include <air_types.h>

#define I2C_BITBANG_MAX_ADDR_LEN (4)
#define I2C_BITBANG_MAX_DATA_LEN (256)

typedef struct I2C_BITBANG_PARAM_S
{
    UI32_T addr_len;
    UI32_T addr;

    UI32_T data_len;
    union
    {
        UI32_T  data;
        UI32_T *ptr_data;
    } info;
} I2C_BITBANG_PARAM_T;

typedef struct I2C_BITBANG_CFG_S
{
    UI32_T unit;
    UI8_T  sda_pin;
    UI8_T  scl_pin;
} I2C_BITBANG_CFG_T;

/* FUNCTION NAME:   i2c_bitbang_init
 * PURPOSE:
 *      Initialize the I2C bit-bang feature
 * INPUT:
 *      unit                 -- Device unit number
 *      count                -- Count of config array
 *      ptr_cfg              -- Pointer of i2c bitbang pin config
 *                              I2C_BITBANG_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_ALREADY_INITED -- Init func was called
 *      AIR_E_NO_MEMORY      -- Malloc failed
 *      AIR_E_BAD_PARAMETER  -- Customer_ref not config
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_init(
    const UI32_T       unit,
    const UI32_T       count,
    I2C_BITBANG_CFG_T *ptr_cfg);

/* FUNCTION NAME:   i2c_bitbang_deinit
 * PURPOSE:
 *      De-initialize the I2C bit-bang feature
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_deinit(
    const UI32_T unit);

/* FUNCTION NAME:   i2c_bitbang_write
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang write operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- No ack.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_NOT_INITED     -- I2C bus is not init.
 *      AIR_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_write(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI16_T         slave_id,
    I2C_BITBANG_PARAM_T *ptr_param);

/* FUNCTION NAME:   i2c_bitbang_read
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang read operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- No ack.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_NOT_INITED     -- I2C bus is not inited.
 *      AIR_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_read(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI16_T         slave_id,
    I2C_BITBANG_PARAM_T *ptr_param);
#endif
