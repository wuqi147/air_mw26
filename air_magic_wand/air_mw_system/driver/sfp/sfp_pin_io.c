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

/* FILE NAME:   sfp_pin_io.c
 * PURPOSE:
 *      Provide the interfaces to access SFP IO PINs such as MOD_ABS and RX_LOS.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "sfp_pin_io.h"
#include "sfp_config_customer.h"
#include "sfp_util.h"
#include "air_chipscu.h"
#include "air_perif.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define SFP_PIN_IO_GETIOMUXMODE(pin) ((AIR_CHIPSCU_IOMUX_FORCE_GPIO22_MODE < (AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE + (pin))) ? \
                                         AIR_CHIPSCU_IOMUX_FUNC_MAX : (AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE + (pin)))

#define SFP_PIN_IO_PIONUM_I2C_CHECK(pin) ((SFP_PIN_PIONUM_INVALID == (pin)) || (SFP_PIN_IS_SWPIN(pin)) || ((pin) <= SFP_PIN_PIONUM_I2C_MAX))

/* DATA TYPE DECLARATIONS
 */


/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
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
    void *ptr_data)
{
    UNUSED(ptr_hwParam)
    UNUSED(ptr_data)

    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    UI32_T ret = MW_E_OK;
    BOOL_T pin_exist = FALSE;

    if (NULL == ptr_portSettings)
    {
        return MW_E_NOT_SUPPORT;
    }

    if (SFP_PIN_IS_SWABSPIN(ptr_portSettings->mod_abs))
    {
        pin_exist = TRUE;
    }
    else if (ptr_portSettings->mod_abs != SFP_PIN_PIONUM_INVALID)
    {
        pin_exist = TRUE;
        ret |= air_chipscu_setIomuxFuncState(unit, SFP_PIN_IO_GETIOMUXMODE(ptr_portSettings->mod_abs), AIR_CHIPSCU_IOMUX_ENABLE);
        ret |= air_perif_setGpioDirection(unit, ptr_portSettings->mod_abs, AIR_PERIF_GPIO_DIRECTION_INPUT);
    }

    if (SFP_PIN_IS_SWRXLOSPIN(ptr_portSettings->rx_loss))
    {
        pin_exist = TRUE;
    }
    else if (ptr_portSettings->rx_loss != SFP_PIN_PIONUM_INVALID)
    {
        pin_exist = TRUE;
        ret |= air_chipscu_setIomuxFuncState(unit, SFP_PIN_IO_GETIOMUXMODE(ptr_portSettings->rx_loss), AIR_CHIPSCU_IOMUX_ENABLE);
        ret |= air_perif_setGpioDirection(unit, ptr_portSettings->rx_loss, AIR_PERIF_GPIO_DIRECTION_INPUT);
    }

    if (ptr_portSettings->tx_fault != SFP_PIN_PIONUM_INVALID)
    {
        pin_exist = TRUE;
        ret |= air_chipscu_setIomuxFuncState(unit, SFP_PIN_IO_GETIOMUXMODE(ptr_portSettings->tx_fault), AIR_CHIPSCU_IOMUX_ENABLE);
        ret |= air_perif_setGpioDirection(unit, ptr_portSettings->tx_fault, AIR_PERIF_GPIO_DIRECTION_INPUT);
    }

    if (ptr_portSettings->tx_disable != SFP_PIN_PIONUM_INVALID)
    {
        pin_exist = TRUE;
        ret |= air_chipscu_setIomuxFuncState(unit, SFP_PIN_IO_GETIOMUXMODE(ptr_portSettings->tx_disable), AIR_CHIPSCU_IOMUX_ENABLE);
        ret |= air_perif_setGpioDirection(unit, ptr_portSettings->tx_disable, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
    }

    if (FALSE == pin_exist)
    {
        ret = MW_E_OP_INVALID;
    }

    if (MW_E_OK != ret)
    {
        SFP_LOG_ERROR("%s fail:%d port:%d", __func__, ret, ptr_portSettings->port);
    }

    return (MW_ERROR_NO_T)ret;
}

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
    void *ptr_data)
{
    UNUSED(ptr_data)

    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    MW_ERROR_NO_T ret = MW_E_OK;
    BOOL_T flag_write = FALSE, sw_pin_exist = FALSE;

    if ((NULL == ptr_portSettings) || (NULL == ptr_hwParam) ||
        (FALSE == SFP_PIN_IO_PIONUM_I2C_CHECK(ptr_portSettings->mod_abs)) ||
        (FALSE == SFP_PIN_IO_PIONUM_I2C_CHECK(ptr_portSettings->rx_loss)) ||
        (FALSE == SFP_PIN_IO_PIONUM_I2C_CHECK(ptr_portSettings->tx_fault)) ||
        (FALSE == SFP_PIN_IO_PIONUM_I2C_CHECK(ptr_portSettings->tx_disable)))
    {
        return MW_E_BAD_PARAMETER;
    }

    sif_info.channel = ptr_hwParam->i2c_channel;
    sif_info.slave_id = ptr_hwParam->ext_addr[0];
    sif_param.addr_len = 1;
    sif_param.addr = ptr_hwParam->ext_addr[1];
    sif_param.data_len = 2;
    sif_param.info.data = 0;

    ret = sfp_util_sif_read(unit, &sif_info, &sif_param);
    if (MW_E_OK == ret)
    {
        /* Input */
        if (SFP_PIN_IS_SWABSPIN(ptr_portSettings->mod_abs))
        {
            sw_pin_exist = TRUE;
        }
        else if (SFP_PIN_PIONUM_INVALID != ptr_portSettings->mod_abs)
        {
            sif_param.info.data |= BIT(ptr_portSettings->mod_abs);
            flag_write = TRUE;
        }

        if (SFP_PIN_IS_SWRXLOSPIN(ptr_portSettings->rx_loss))
        {
            sw_pin_exist = TRUE;
        }
        else if (SFP_PIN_PIONUM_INVALID != ptr_portSettings->rx_loss)
        {
            sif_param.info.data |= BIT(ptr_portSettings->rx_loss);
            flag_write = TRUE;
        }

        if (SFP_PIN_PIONUM_INVALID != ptr_portSettings->tx_fault)
        {
            sif_param.info.data |= BIT(ptr_portSettings->tx_fault);
            flag_write = TRUE;
        }

        /* Output */
        if (SFP_PIN_PIONUM_INVALID != ptr_portSettings->tx_disable)
        {
            sif_param.info.data &= ~BIT(ptr_portSettings->tx_disable);
            flag_write = TRUE;
        }

        if (TRUE == flag_write)
        {
            ret = sfp_util_sif_write(unit, &sif_info, &sif_param);
            if (MW_E_OK != ret)
            {
                SFP_LOG_ERROR("%s fail:%d", __func__, ret);
            }
        }
        else if (FALSE == sw_pin_exist)
        {
            ret = MW_E_BAD_PARAMETER;
            SFP_LOG_ERROR("%s fail:%d. No IO pin!", __func__, ret);
        }
    }

    return ret;
}

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
    void *ptr_data)
{
    UNUSED(unit)
    UNUSED(port)
    UNUSED(ptr_hwParam)
    UNUSED(ptr_data)

    return MW_E_OK;
}

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
    UI8_T *const ptr_pin)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    MW_ERROR_NO_T ret = MW_E_OK;

    if ((NULL == ptr_pin) || (NULL == ptr_portSettings))
    {
        return MW_E_BAD_PARAMETER;
    }

    (*ptr_pin) = SFP_PIN_PIONUM_INVALID;

    switch (type)
    {
        case SFP_PIN_TYPE_ABS:
        {
            (*ptr_pin) = ptr_portSettings->mod_abs;
            break;
        }
        case SFP_PIN_TYPE_TX_DISABLE:
        {
            (*ptr_pin) = ptr_portSettings->tx_disable;
            break;
        }
        case SFP_PIN_TYPE_RX_LOSS:
        {
            (*ptr_pin) = ptr_portSettings->rx_loss;
            break;
        }
        case SFP_PIN_TYPE_TX_FAULT:
        {
            (*ptr_pin) = ptr_portSettings->tx_fault;
            break;
        }

        default:
        {
            ret = MW_E_NOT_SUPPORT;
            break;
        }
    }

    return ret;
}

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
 *      MW_E_OK              -- Operation success
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
    void *ptr_data)
{
    UNUSED(ptr_hwParam)
    UNUSED(ptr_data)

    return air_perif_setGpioOutputData(unit, pin, (AIR_PERIF_GPIO_DATA_T)value);
}

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
    void *ptr_data)
{
    UNUSED(ptr_hwParam)
    UNUSED(ptr_data)

    return air_perif_getGpioInputData(unit, pin, (AIR_PERIF_GPIO_DATA_T *)ptr_value);
}

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
    void *ptr_data)
{
    UNUSED(ptr_data)

    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    MW_ERROR_NO_T ret = MW_E_OK;

    if (NULL == ptr_hwParam)
    {
        return MW_E_BAD_PARAMETER;
    }

    sif_info.channel = ptr_hwParam->i2c_channel;
    sif_info.slave_id = ptr_hwParam->ext_addr[0];
    sif_param.addr_len = 1;
    sif_param.addr = ptr_hwParam->ext_addr[3];
    sif_param.data_len = 2;
    sif_param.info.data = 0;

    ret = sfp_util_sif_read(unit, &sif_info, &sif_param);
    if (MW_E_OK == ret)
    {
        if (0 != value)
        {
            sif_param.info.data |= (1 << pin);
        }
        else
        {
            sif_param.info.data &= ~(1 << pin);
        }
        ret = sfp_util_sif_write(unit, &sif_info, &sif_param);
    }

    if (MW_E_OK != ret)
    {
        SFP_LOG_ERROR("%s fail ret:%d", __func__, ret);
    }
    return ret;
}

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
    void *ptr_data)
{
    UNUSED(ptr_data)

    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    MW_ERROR_NO_T ret = MW_E_OK;

    if (NULL == ptr_hwParam)
    {
        return MW_E_BAD_PARAMETER;
    }

    sif_info.channel = ptr_hwParam->i2c_channel;
    sif_info.slave_id = ptr_hwParam->ext_addr[0];
    sif_param.addr_len = 1;
    sif_param.addr = ptr_hwParam->ext_addr[2];
    sif_param.data_len = 2;
    sif_param.info.data = 0;

    ret = sfp_util_sif_read(unit, &sif_info, &sif_param);
    if (MW_E_OK == ret)
    {
        *ptr_value = (sif_param.info.data >> pin) & 1;
    }
    else
    {
        SFP_LOG_WARN("%s fail ret:%d", __func__, ret);
    }
    return ret;
}

