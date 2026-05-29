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

/* FILE NAME:   sfp_pin_customer.c
 * PURPOSE:
 *      This file is for customers to implement the interfaces to access SFP
 *      module based on the customers' boards.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "sfp_pin.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* FUNCTION NAME:   _sfp_customer_pin_init
 * PURPOSE:
 *      Initialize the SFP PIN drivers for each SFP port or each COMBO port.
 *      If the SFP SDA PIN driver or the IO PIN driver initialization fails for
 *      a port, auto-adaption algorithm will be adopted for the port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the PIN initialization state
 * RETURN:
 *      MW_E_OK             -- Success always
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_init(
    const UI32_T unit,
    SFP_PIN_INIT_STATE_T *ptr_state);

/* FUNCTION NAME:   _sfp_customer_pin_sda_switchCtrlToPort
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
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
_sfp_customer_pin_sda_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   _sfp_customer_pin_sda_write
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_write(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   _sfp_customer_pin_sda_read
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_read(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   _sfp_customer_pin_sda_writeC22ByPort
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
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_writeC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   _sfp_customer_pin_sda_readC22ByPort
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
_sfp_customer_pin_sda_readC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* FUNCTION NAME:   _sfp_customer_pin_sda_writeC45ByPort
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
_sfp_customer_pin_sda_writeC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   _sfp_customer_pin_sda_readC45ByPort
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
_sfp_customer_pin_sda_readC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* FUNCTION NAME:   _sfp_customer_pin_io_switchCtrlToPort
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP IO PINs.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   _sfp_customer_pin_io_getPIONum
 * PURPOSE:
 *      Return the PIO number (the GPIO number or the PIO number of an IO extender)
 *      of a specific pin, such as MOD_ABS pin, TX_DISABLE pin, RX_LOSS pin and
 *      TX_FAULT pin, for a specific port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      type                 -- PIN type
 * OUTPUT:
 *      ptr_pin              -- A pointer returns the PIO number of the pin
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_getPIONum()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_getPIONum(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin);

/* FUNCTION NAME:   _sfp_customer_pin_io_setValue
 * PURPOSE:
 *      Set a value to a specific SFP IO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      pin                  -- The PIO number of the pin
 *      value                -- The value to set
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_setValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_setValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    const I32_T value);

/* FUNCTION NAME:   _sfp_customer_pin_io_getValue
 * PURPOSE:
 *      Get the value of a specific SFP IO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      pin                  -- The GPIO number of the pin
 * OUTPUT:
 *      ptr_value            -- A pointer to return the value of the GPIO pin
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_getValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_getValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    I32_T *const ptr_value);

/* STATIC VARIABLE DECLARATIONS
 */
static const SFP_PIN_FUNC_T _sfp_customer_pin_func = {
    _sfp_customer_pin_init,
    _sfp_customer_pin_sda_switchCtrlToPort,
    _sfp_customer_pin_sda_write,
    _sfp_customer_pin_sda_read,
    _sfp_customer_pin_sda_writeC22ByPort,
    _sfp_customer_pin_sda_readC22ByPort,
    _sfp_customer_pin_sda_writeC45ByPort,
    _sfp_customer_pin_sda_readC45ByPort,
    _sfp_customer_pin_io_switchCtrlToPort,
    _sfp_customer_pin_io_getPIONum,
    _sfp_customer_pin_io_setValue,
    _sfp_customer_pin_io_getValue,
};

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _sfp_customer_pin_init
 * PURPOSE:
 *      Initialize the SFP PIN drivers for each SFP port or each COMBO port.
 *      If the SFP SDA PIN driver or the IO PIN driver initialization fails for
 *      a port, auto-adaption algorithm will be adopted for the port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the PIN initialization state
 * RETURN:
 *      MW_E_OK             -- Success always
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_init(
    const UI32_T unit,
    SFP_PIN_INIT_STATE_T *ptr_state)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_switchCtrlToPort
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
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
_sfp_customer_pin_sda_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_write
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_write(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_read
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_read(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_writeC22ByPort
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
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_sda_writeC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_readC22ByPort
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
_sfp_customer_pin_sda_readC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_writeC45ByPort
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
_sfp_customer_pin_sda_writeC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_sda_readC45ByPort
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
_sfp_customer_pin_sda_readC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_io_switchCtrlToPort
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP IO PINs.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_io_getPIONum
 * PURPOSE:
 *      Return the PIO number (the GPIO number or the PIO number of an IO extender)
 *      of a specific pin, such as MOD_ABS pin, TX_DISABLE pin, RX_LOSS pin and
 *      TX_FAULT pin, for a specific port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      type                 -- PIN type
 * OUTPUT:
 *      ptr_pin              -- A pointer returns the PIO number of the pin
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_getPIONum()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_getPIONum(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_io_setValue
 * PURPOSE:
 *      Set a value to a specific SFP IO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      pin                  -- The PIO number of the pin
 *      value                -- The value to set
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_setValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_setValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    const I32_T value)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_customer_pin_io_getValue
 * PURPOSE:
 *      Get the value of a specific SFP IO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      pin                  -- The GPIO number of the pin
 * OUTPUT:
 *      ptr_value            -- A pointer to return the value of the GPIO pin
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_getValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
_sfp_customer_pin_io_getValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    I32_T *const ptr_value)
{
    /* Customer need implement this function based on their HW schematics and the PHY modules they want to support. */
    return AIR_E_NOT_SUPPORT;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_customer_pin_getFunc
 * PURPOSE:
 *      Get the customized function pointers to access SFP PINs. If customers
 *      want to use their own implementation for SFP PIN access, for example based
 *      on their own HW schematics, customers need to implement the functions of
 *      sfp_pin_customer.c and return a pointer points to a structure with function
 *      pointers for SFP PIN access for this function.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      A pinter points to a structure with function pointers for SFP module access
 *
 * NOTES:
 *      If NULL is returned, the default module access implementation for RFB will be used. *
 */
SFP_PIN_FUNC_T *
sfp_customer_pin_getFunc(
    void)
{
    /* return &_sfp_customer_pin_func; */
    return NULL;
}
