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

/* FILE NAME:   sfp_pin.h
 * PURPOSE:
 *      Provide interfaces to access SFP module.
 * NOTES:
 *
 */

#ifndef SFP_PIN_H
#define SFP_PIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_util.h"
#include "sfp_task.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define SFP_PIN_EXT_ADDR_NUM    (4)

#define SFP_PIN_PIONUM_INVALID    (0xFF)
#define SFP_PIN_PIONUM_I2C_MAX    (17)

#define SFP_PIN_PIONUM_SWPIN_MASK              (0xC0)
#define SFP_PIN_PIONUM_SWPIN_ABS               (0x40)
#define SFP_PIN_PIONUM_SWPIN_RXLOS             (0x80)
#define SFP_PIN_PIONUM_SWABSPIN(port_num)      (SFP_PIN_PIONUM_SWPIN_ABS | ((~SFP_PIN_PIONUM_SWPIN_MASK) & (port_num)))
#define SFP_PIN_PIONUM_SWRXLOSPIN(port_num)    (SFP_PIN_PIONUM_SWPIN_RXLOS | ((~SFP_PIN_PIONUM_SWPIN_MASK) & (port_num)))
#define SFP_PIN_IS_SWABSPIN(sw_pin)            (SFP_PIN_PIONUM_SWPIN_ABS == (SFP_PIN_PIONUM_SWPIN_MASK & (sw_pin)))
#define SFP_PIN_IS_SWRXLOSPIN(sw_pin)          (SFP_PIN_PIONUM_SWPIN_RXLOS == (SFP_PIN_PIONUM_SWPIN_MASK & (sw_pin)))
#define SFP_PIN_IS_SWPIN(sw_pin)               (SFP_PIN_IS_SWABSPIN(sw_pin) || SFP_PIN_IS_SWRXLOSPIN(sw_pin))
#define SFP_PIN_GET_SWPIN_PORTNUM(sw_pin)      ((~SFP_PIN_PIONUM_SWPIN_MASK) & (sw_pin))

#define SFP_PIN_MUXCHANNEL_INVALID    (0xFF)

/* The PIN initialization state for a port. */
typedef enum
{
    /* PIN is initialized. */
    SFP_PIN_INITED = 0x01,
    /* SFP SDA PIN is initialized successfully. */
    SFP_PIN_SDA_INIT_SUCCEED = 0x02,
    /* SFP IO PINs are initialized successfully. */
    SFP_PIN_IO_INIT_SUCCEED = 0x04,
} SFP_PIN_INIT_STATE_T;

/* A function pointer type. The function it points to initializes the serial
 * interface to access an SFP module via 2-wire addresses.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_INIT_FUNC_T)(
    const UI32_T unit,
    SFP_PIN_INIT_STATE_T *ptr_initState);

/* A function pointer type. The function it points to switch the control to
 * an specific port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_SWITCHCONTROLTOPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

/* A function pointer type. The function it points to write data through a 2-wire
 * address of an SFP module via the serial interface.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_WRITE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* A function pointer type. The function it points to read data through a 2-wire
 * address of an SFP module via the serial interface.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_READ_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* A function pointer type. The function it points to write data to a CL22 PHY
 * register of the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_WRITEC22BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* A function pointer type. The function it points to read a CL22 PHY register of
 * the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_READC22BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* A function pointer type. The function it points to write data to a CL45 PHY
 * register of the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_WRITEC45BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* A function pointer type. The function it points to read a CL45 PHY register of
 * the inside PHY of an SFP module inserted to a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_SDA_READC45BYPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* A function pointer type. The function it points to switch the control to
 * an specific port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_SWITCHCTRLTOPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

/* A function pointer type. The function it points to return the GPIO number of
 * a pin, such as MOD_ABS pin, TX_DISABLE pin, RX_LOSS pin and TX_FAULT pin, for
 * a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_GETPIONUM_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin);

/* A function pointer type. The function it points to set a value to a specific
 * GPIO pin.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_SETVALUE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    const I32_T value);

/* A function pointer type. The function it points to get the value of a specific
 * GPIO pin.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_GETVALUE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    I32_T *const ptr_value);

/* A structure used to register functions to access SFP SDA PIN and SFP IO PINs. */
typedef struct
{
    SFP_PIN_INIT_FUNC_T init;
    SFP_PIN_SDA_SWITCHCONTROLTOPORT_FUNC_T pin_sda_switchCtrlToPort;
    SFP_PIN_SDA_WRITE_FUNC_T pin_sda_write;
    SFP_PIN_SDA_READ_FUNC_T pin_sda_read;
    SFP_PIN_SDA_WRITEC22BYPORT_FUNC_T pin_sda_writeC22ByPort;
    SFP_PIN_SDA_READC22BYPORT_FUNC_T pin_sda_readC22ByPort;
    SFP_PIN_SDA_WRITEC45BYPORT_FUNC_T pin_sda_writeC45ByPort;
    SFP_PIN_SDA_READC45BYPORT_FUNC_T pin_sda_readC45ByPort;
    SFP_PIN_IO_SWITCHCTRLTOPORT_FUNC_T pin_io_switchCtrlToPort;
    SFP_PIN_IO_GETPIONUM_FUNC_T pin_io_getPIONum;
    SFP_PIN_IO_SETVALUE_FUNC_T pin_io_setValue;
    SFP_PIN_IO_GETVALUE_FUNC_T pin_io_getValue;
} SFP_PIN_FUNC_T;

typedef struct
{
    UI8_T i2c_channel;
    UI8_T i2c_addr;
    UI8_T ext_addr[SFP_PIN_EXT_ADDR_NUM];
} SFP_HW_PARAM_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 /* FUNCTION NAME:   sfp_pin_checkInitState
 * PURPOSE:
 *      Check the PIN initialization state of a port to obtain the initialization
 *      status.
 *
 * INPUT:
 *      state                -- The PIN initialization state
 *      flags                -- The states to be checked
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                -- The states are set
 *      FALSE               -- The states are not set
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_pin_checkInitState(
    UI8_T state,
    SFP_PIN_INIT_STATE_T flags);

/* FUNCTION NAME:   sfp_pin_init
 * PURPOSE:
 *      Initialize the SFP PIN drivers for each SFP port or each COMBO port.
 *      If the SFP SDA PIN driver or the IO PIN driver initialization fails for
 *      a port, auto-adaption algorithm will be adopted for the port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OTHER           -- Other errors
 *
 * NOTES:
 *      It must be called after _sfp_task_ctx is initialized.
 */
MW_ERROR_NO_T
sfp_pin_init(
    const UI32_T unit);

/* FUNCTION NAME:   sfp_pin_sda_switchCtrlToPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_pin_sda_write
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_write(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_read
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_read(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data);

/* FUNCTION NAME:   sfp_pin_sda_writeC22ByPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_writeC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   sfp_pin_sda_readC22ByPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_readC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* FUNCTION NAME:   sfp_pin_sda_writeC45ByPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_writeC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME:   sfp_pin_sda_readC45ByPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_sda_readC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data);

/* FUNCTION NAME:   sfp_pin_io_switchCtrlToPort
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_io_switchCtrlToPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   sfp_pin_io_getPIONum
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_io_getPIONum(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin);

/* FUNCTION NAME:   sfp_pin_io_setValue
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
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_pin_io_setValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    const I32_T value);

/* FUNCTION NAME:   sfp_pin_io_getValue
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
 *      None
 */
MW_ERROR_NO_T
sfp_pin_io_getValue(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T pin,
    I32_T *const ptr_value);

/* FUNCTION NAME:   sfp_pin_io_SWPinPolling
 * PURPOSE:
 *      Try to read the soft RX_LOS SFF data of a specific SFP port. If succeeds,
 *      it means there is an SFP module inserted to the port. Record the inserted
 *      status and the soft RX_LOS SFF data. These information will be used to
 *      determine the pin value of the software ABS pin and the software RX_LOS pin.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer points to the port information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_pin_io_SWPinPolling(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info);

/* FUNCTION NAME:   sfp_pin_sda_isAnyPortReady
 * PURPOSE:
 *      Check if there is any (COMBO) SFP port the SDA pin of which is inited and ready to use.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE              -- There is at least a port with SDA pin ready
 *      FALSE             -- There is no port with SDA pin ready
 *
 * NOTES:
 *      None.
 */
BOOL_T
sfp_pin_sda_isAnyPortReady(
    void);
#endif /* End of SFP_PIN_H */

