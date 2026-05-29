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

/* FILE NAME:   sfp_pin_io.h
 * PURPOSE:
 *      Provide the interfaces to access SFP IO PINs such as MOD_ABS and RX_LOS.
 * NOTES:
 *
 */

#ifndef SFP_PIN_IO_H
#define SFP_PIN_IO_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_pin.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_DRIVER_INIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A function pointer type. The function it points to switch the control to
 * an specific port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_DRIVER_SWITCHCTRLPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A function pointer type. The function it points to return the GPIO number of
 * a pin, such as MOD_ABS pin, TX_DISABLE pin, RX_LOSS pin and TX_FAULT pin, for
 * a port.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_DRIVER_GETPIONUM_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin);

/* A function pointer type. The function it points to set a value to a specific
 * GPIO pin.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_DRIVER_SETVALUE_FUNC_T)(
    const UI32_T unit,
    const UI8_T pin,
    const I32_T value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A function pointer type. The function it points to get the value of a specific
 * GPIO pin.
 */
typedef MW_ERROR_NO_T
(*SFP_PIN_IO_DRIVER_GETVALUE_FUNC_T)(
    const UI32_T unit,
    const UI8_T pin,
    I32_T *const ptr_value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* A structure used to register functions to access the SFP IO PINs. */
typedef struct
{
    SFP_PIN_IO_DRIVER_INIT_FUNC_T init;
    SFP_PIN_IO_DRIVER_SWITCHCTRLPORT_FUNC_T switchCtrlToPort;
    SFP_PIN_IO_DRIVER_GETPIONUM_FUNC_T getPIONum;
    SFP_PIN_IO_DRIVER_SETVALUE_FUNC_T setValue;
    SFP_PIN_IO_DRIVER_GETVALUE_FUNC_T getValue;
} SFP_PIN_IO_DRIVER_FUNC_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   sfp_pin_io_driver_init0
 * PURPOSE:
 *      Initialize the SFP IO PIN driver for each SFP port or each COMBO port
 *      to access MOD_ABS pin, RX_LOS pin and so on when the SFP PINs are
 *      connected to GPIO PINs.
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
 *      MW_E_NOT_SUPPORT    -- Operation is not supported
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_init0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_io_driver_init1
 * PURPOSE:
 *      Initialize the SFP IO PIN driver for each SFP port or each COMBO port
 *      to access MOD_ABS pin, RX_LOS pin and so on.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_init1(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_io_driver_switchCtrlToPort0
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP IO PINs.
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
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_switchCtrlToPort0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_io_driver_getPIONum0
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
sfp_pin_io_driver_getPIONum0(
    const UI32_T unit,
    const UI32_T port,
    SFP_PIN_TYPE_T type,
    UI8_T *const ptr_pin);


/* FUNCTION NAME:   sfp_pin_io_driver_setValue0
 * PURPOSE:
 *      Set a value to a specific SFP IO PIN when it is connected to a GPIO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- The PIO number of the pin
 *      value                -- The value to set
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_setValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_setValue0(
    const UI32_T unit,
    const UI8_T pin,
    const I32_T value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_getValue0
 * PURPOSE:
 *      Get the value of a specific SFP IO PIN when it is connected to a GPIO PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- The GPIO number of the pin
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      ptr_value            -- A pointer to return the value of the GPIO pin
 * RETURN:
 *      MW_E_OK             -- Operation success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_getValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_getValue0(
    const UI32_T unit,
    const UI8_T pin,
    I32_T *const ptr_value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_io_driver_setValue1
 * PURPOSE:
 *      Set a value to a specific SFP IO PIN when it is connected to an IO extender.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- The PIO number of the pin
 *      value                -- The value to set
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_io_setValue()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_io_driver_setValue1(
    const UI32_T unit,
    const UI8_T pin,
    const I32_T value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

/* FUNCTION NAME:   sfp_pin_getValue0
 * PURPOSE:
 *      Get the value of a specific SFP IO PIN when it is connected to an IO extender.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- The GPIO number of the pin
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
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
sfp_pin_io_driver_getValue1(
    const UI32_T unit,
    const UI8_T pin,
    I32_T *const ptr_value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data);

#endif /* End of SFP_PIN_IO_H */

