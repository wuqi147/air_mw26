/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:   sfp_util.h
 * PURPOSE:
 *      Provide some useful interfaces.
 * NOTES:
 *
 */

#ifndef SFP_UTIL_H
#define SFP_UTIL_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "osapi_string.h"
#include "osapi_message.h"
#include "air_sif.h"
#include "air_port.h"
#ifdef AIR_EN_I2C_BITBANG
#include <i2c_bitbang.h>
#endif
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define SFP_CONVERT_REGISTER_DATA(data)    ((((data) & 0xFF) << 8) | (((data) >> 8) & 0xFF))

#define SFP_LOG_ERROR(fmt,...)  MW_LOG_ERROR(SFP, fmt "\n", ##__VA_ARGS__)
#define SFP_LOG_WARN(fmt,...)   MW_LOG_WARN(SFP, fmt "\n", ##__VA_ARGS__)
#define SFP_LOG_INFO(fmt,...)   MW_LOG_INFO(SFP, fmt "\n", ##__VA_ARGS__)
#define SFP_LOG_DEBUG(fmt,...)  MW_LOG_DEBUG(SFP, fmt "\n", ##__VA_ARGS__)

#define SFP_UTIL_GETARRAYSIZE(a, type) (sizeof(a) / sizeof(type))

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFP_PIN_TYPE_ABS,
    SFP_PIN_TYPE_TX_DISABLE,
    SFP_PIN_TYPE_RX_LOSS,
    SFP_PIN_TYPE_TX_FAULT,

    SFP_PIN_TYPE_LAST
} SFP_PIN_TYPE_T;

typedef struct
{
    UI8_T port;
    UI8_T abs;
    UI8_T tx_disable;
    UI8_T rx_loss;
    UI8_T tx_fault;
    UI8_T channel;
} SFP_HW_INFO_T;

typedef enum
{
    SFP_PORT_SERDES_MODE_SGMII_MASK = 0x01,
    SFP_PORT_SERDES_MODE_1000BASE_X_MASK = 0x02,
    SFP_PORT_SERDES_MODE_HSGMII_MASK = 0x04,
} SFP_PORT_SERDES_MODE_MASK_T;

/* GLOBAL VARIABLE DECLARATIONS */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   sfp_util_sif_write
 * PURPOSE:
 *      This API is used to do the I2C write operation.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_util_sif_write(
    const UI32_T            unit,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param);

/* FUNCTION NAME:   sfp_util_sif_read
 * PURPOSE:
 *      This API is used to do the I2C read operation.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_sif_read(
    const UI32_T            unit,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param);

#ifdef AIR_EN_I2C_BITBANG
/* FUNCTION NAME:   sfp_util_i2c_bitbang_write
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
 *      MW_E_OK             -- Operation success.
 *      MW_E_OTHERS         -- No ack.
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong.
 *      MW_E_NOT_INITED     -- I2C bus is not inited.
 *      MW_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_i2c_bitbang_write(
    const UI32_T unit,
    const UI16_T channel,
    const UI16_T slave_id,
    I2C_BITBANG_PARAM_T *ptr_param);

/* FUNCTION NAME:   sfp_util_i2c_bitbang_read
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
 *      MW_E_OK             -- Operation success.
 *      MW_E_OTHERS         -- No ack.
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong.
 *      MW_E_NOT_INITED     -- I2C bus is not inited.
 *      MW_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_i2c_bitbang_read(
    const UI32_T unit,
    const UI16_T channel,
    const UI16_T slave_id,
    I2C_BITBANG_PARAM_T *ptr_param);
#endif

/* FUNCTION NAME:   sfp_port_is_serdesPort
 * PURPOSE:
 *      Check if the port is a pure serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a pure serdes port
 *      FALSE            --  The port is not a pure serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_serdesPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_port_is_comboPort
 * PURPOSE:
 *      Check if the port is a COMBO port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a COMBO port
 *      FALSE            --  The port is not a COMBO port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_comboPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_port_is_pureComboSerdesPort
 * PURPOSE:
 *      Check if the port is a pure COMBO serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a pure COMBO serdes port
 *      FALSE            --  The port is not a pure COMBO serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_pureComboSerdesPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_port_is_comboSerdesPort
 * PURPOSE:
 *      Check if the port is a COMBO serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a COMBO serdes port
 *      FALSE            --  The port is not a COMBO serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_comboSerdesPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_port_checkInsidePHYReady
 * PURPOSE:
 *      Check if there is an accessible inside PHY within an SFP module.
 *
 * INPUT:
 *      unit                -- Device unit number
 *      port                -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- There is an accessible inside PHY
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_NOT_SUPPORT    -- Operation is not supported
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_OTHERS         -- Other errors
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_port_checkInsidePHYReady(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_port_checkPinInitState
 * PURPOSE:
 *      Check the PIN initialization state of a port to obtain the initialization
 *      status.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port index
 *      flags                -- The states to be checked
 *                              SFP_PIN_INIT_STATE_T
 * OUTPUT:
 *      ptr_state            -- A pointer returns the PIN initialization state
 * RETURN:
 *      MW_E_OK              -- The states are set
 *      Others               -- The states are not set
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_port_checkPinInitState(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T flags,
    BOOL_T *ptr_state);

/* FUNCTION NAME:   sfp_port_setComboMode
 * PURPOSE:
 *      Set the COMBO mode to PHY or SERDES for a specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      admin_state              -- Current admin state of the port
 *      keep_serdesMACEnable     -- Keep COMBO SERDES enable or not when changing
 *                                  to COMBO PHY mode
 *      combo_mode               -- The COMBO mode to set
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                  -- Operation success.
 *      MW_E_BAD_PARAMETER       -- Parameter is wrong.
 *      MW_E_OP_INVALID          -- Operation is invalid.
 * NOTES:
 *      Only support the COMBO ports.
 *      To avoid the COMBO port from being link up between its PHY port and SERDES port, COMBO
 *      PHY need be disabled before changing to COMBO SERDES. However, for SFP auto
 *      adaption, COMBO SERDES need keep enabled even after changing to COMBO PHY to read
 *      MAC registers.
 */
MW_ERROR_NO_T
sfp_port_setComboMode(
    UI32_T unit,
    UI32_T port,
    BOOL_T admin_state,
    BOOL_T keep_serdesMACEnable,
    AIR_PORT_COMBO_MODE_T combo_mode);

MW_ERROR_NO_T
sfp_cmd_startSFP(
    void);

MW_ERROR_NO_T
sfp_cmd_stopSFP(
    void);

MW_ERROR_NO_T
sfp_writeReg(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI32_T reg_val);

#endif /* SFP_UTIL_H */

