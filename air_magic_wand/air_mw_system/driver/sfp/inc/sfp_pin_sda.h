/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2024
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

/* FILE NAME:   sfp_pin_sda.h
 * PURPOSE:
 *      Provide the interfaces to access SFP module via the SFP SDA PIN.
 * NOTES:
 *
 */

#ifndef SFP_PIN_SDA_H
#define SFP_PIN_SDA_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_pin.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* A function pointer type. The function it points to initialize the serial
 * interface to access an SFP module via 2-wire addresses.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_INIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A function pointer type. The function it points to switch the control to
 * an specific port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_SWITCHCTRLPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A function pointer type. The function it points to write data through a 2-wire
 * address of an SFP module via the serial interface.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_WRITE_FUNC_T)(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* A function pointer type. The function it points to read data through a 2-wire
 * address of an SFP module via the serial interface.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_READ_FUNC_T)(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* A function pointer type. The function it points to write data to a CL22 PHY
 * register of the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_WRITEC22BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* A function pointer type. The function it points to read a CL22 PHY register of
 * the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_READC22BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* A function pointer type. The function it points to write data to a CL45 PHY
 * register of the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_WRITEC45BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* A function pointer type. The function it points to read a CL45 PHY register of
 * the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_DRIVER_READC45BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* A structure used to register functions to access the SFP SDA PIN. */
typedef struct
{
    SFP_PIN_SDA_DRIVER_INIT_FUNC_T init;
    SFP_PIN_SDA_DRIVER_SWITCHCTRLPORT_FUNC_T switchCtrlToPort;
    SFP_PIN_SDA_DRIVER_WRITE_FUNC_T write;
    SFP_PIN_SDA_DRIVER_READ_FUNC_T read;
    SFP_PIN_SDA_DRIVER_WRITEC22BYPORT_FUNC_T writeC22ByPort;
    SFP_PIN_SDA_DRIVER_READC22BYPORT_FUNC_T readC22ByPort;
    SFP_PIN_SDA_DRIVER_WRITEC45BYPORT_FUNC_T writeC45ByPort;
    SFP_PIN_SDA_DRIVER_READC45BYPORT_FUNC_T readC45ByPort;
} SFP_PIN_SDA_DRIVER_FUNC_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   sfp_pin_sda_driver_init0
 * PURPOSE:
 *      Initialize the SFP PIN driver for each SFP port or each COMBO port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_init0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_switchCtrlToPort0
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_switchCtrlToPort0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_write0
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_write0(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_read0
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_read0(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_switchCtrlToPort1
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_switchCtrlToPort1(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_write1
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_write1(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_read1
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_read1(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_writeC22ByPort0
 * PURPOSE:
 *      Write data to a CL22 PHY register of the inside PHY of an SFP module
 *      inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      reg_addr             -- PHY register address to be written to
 *      reg_data             -- Data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_writeC22ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_readC22ByPort0
 * PURPOSE:
 *      Read a CL22 PHY register of the inside PHY of an SFP module inserted to
 *      a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      reg_addr             -- PHY register address to be read from
 * OUTPUT:
 *      ptr_reg_data         -- A pointer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_readC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_readC22ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_writeC45ByPort0
 * PURPOSE:
 *      Write data to a CL45 PHY register of the inside PHY of an SFP module
 *      inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      dev_type             -- MMD address
 *      reg_addr             -- PHY register address to be written to
 *      reg_data             -- Data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC45ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_writeC45ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   sfp_pin_sda_driver_readC45ByPort0
 * PURPOSE:
 *      Read a CL45 PHY register of the inside PHY of an SFP module inserted to
 *      a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      dev_type             -- MMD address
 *      reg_addr             -- PHY register address to be read from
 * OUTPUT:
 *      ptr_reg_data         -- A pointer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_readC45ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_readC45ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

#endif /* End of SFP_PIN_SDA_H */

