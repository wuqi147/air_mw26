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

/* FILE NAME:   sfp_pin.c
 * PURPOSE:
 *      Provide the interfaces to access an SFP module via the SFP PINs.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_pin_sda.h"
#include "sfp_pin_io.h"
#include "sfp_pin.h"
#include "sfp_util.h"
#include "sfp_config_customer.h"
#include "sfp_module_handle.h"
#include "sfp_sff_data.h"
#include "sfp_pin_customer.h"

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

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _sfp_pin_io_setSWPinValue
 * PURPOSE:
 *      Set a value to a specific software SFP IO PIN. A software SFP IO PIN is
 *      implemented by the software and it is not connected to any HW GPIO PIN.
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
_sfp_pin_io_setSWPinValue(
    const UI32_T unit,
    const UI8_T pin,
    const I32_T value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data)
{
    return MW_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   _sfp_pin_io_getSWPinValue
 * PURPOSE:
 *      Get the value of a specific software SFP IO PIN. A software SFP IO PIN is
 *      implemented by the software and it is not connected to any HW GPIO PIN.
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
_sfp_pin_io_getSWPinValue(
    const UI32_T unit,
    const UI8_T pin,
    I32_T *const ptr_value,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data)
{
    UI8_T port = SFP_PIN_GET_SWPIN_PORTNUM(pin);
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        if (SFP_PIN_IS_SWABSPIN(pin))
        {
            *ptr_value = (0 != (SFP_TASK_MODULE_STATUS_INSERTED & ptr_port_info->module_status)) ? SFP_MODULE_PIN_LOW : SFP_MODULE_PIN_HIGH;
            return MW_E_OK;
        }
        else if (SFP_PIN_IS_SWRXLOSPIN(pin))
        {
            if (0 != (SFP_TASK_MODULE_STATUS_SWRXLOS_IMPLEMENTED & ptr_port_info->module_status))
            {
                *ptr_value = (0 == (SFP_TASK_MODULE_STATUS_SWRXLOS_HIGH & ptr_port_info->module_status)) ? SFP_MODULE_PIN_LOW : SFP_MODULE_PIN_HIGH;
            }
            else
            {
                *ptr_value = (0 != (SFP_TASK_MODULE_STATUS_INSERTED & ptr_port_info->module_status)) ? SFP_MODULE_PIN_LOW : SFP_MODULE_PIN_HIGH;
            }
            return MW_E_OK;
        }
    }

    return MW_E_NOT_SUPPORT;
}

/* EXPORTED SUBPROGRAM BODIES
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
    SFP_PIN_INIT_STATE_T flags)
{
    BOOL_T ret = FALSE;

    if (SFP_PIN_INITED == flags)
    {
        ret = (0 != (state & SFP_PIN_INITED));
    }
    else if ((flags & SFP_PIN_SDA_INIT_SUCCEED) ||
             (flags & SFP_PIN_IO_INIT_SUCCEED))
    {
        if (0 != (state & SFP_PIN_INITED))
        {
            flags &= (~SFP_PIN_INITED);
            state &= (~SFP_PIN_INITED);
            ret = (flags == (flags & state));
        }
    }

    return ret;
}

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
    const UI32_T unit)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettingsArray = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize(), i = 0;
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;

    SFP_LOG_DEBUG("SFP port config:0x%p count:%d", ptr_portSettingsArray, port_count);
    if ((NULL != ptr_portSettingsArray) && (0 != port_count))
    {
        for (; i < port_count; i++)
        {
            ptr_taskPortInfo = sfp_task_getPortInfo(ptr_portSettingsArray[i].port);
            if (NULL == ptr_taskPortInfo)
            {
                ret = MW_E_OTHERS;
                break;
            }
            ptr_taskPortInfo->pin_state = SFP_PIN_INITED;

            ret = sfp_config_checkPortDriverSettings(unit, ptr_portSettingsArray[i].port);
            if (MW_E_OK != ret)
            {
                SFP_LOG_ERROR("Customer config error for port:%d", ptr_portSettingsArray[i].port);
                break;
            }

            if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettingsArray[i].port_driver)
            {
                SFP_PIN_INIT_STATE_T init_state = 0;
                SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

                if ((NULL == ptr_custFunc) || (NULL == ptr_custFunc->init))
                {
                    ret = MW_E_BAD_PARAMETER;
                    SFP_LOG_ERROR("No customer driver:0x%p or init function for port:%d", ptr_custFunc, ptr_portSettingsArray[i].port);
                    break;
                }
                ret = ptr_custFunc->init(unit, &init_state);
                if (MW_E_OK == ret)
                {
                    if (init_state & SFP_PIN_SDA_INIT_SUCCEED)
                    {
                        ptr_taskPortInfo->pin_state |= SFP_PIN_SDA_INIT_SUCCEED;
                    }

                    if (init_state & SFP_PIN_IO_INIT_SUCCEED)
                    {
                        ptr_taskPortInfo->pin_state |= SFP_PIN_IO_INIT_SUCCEED;
                    }
                }
                else
                {
                    SFP_LOG_ERROR("Customer init fail for port:%d", ptr_portSettingsArray[i].port);
                    break;
                }
            }
            else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettingsArray[i].port_driver)
            {
                /* Init SFP SDA PIN driver */
                if ((NULL != ptr_portSettingsArray[i].ptr_sdaPinDriver) &&
                    (NULL != ptr_portSettingsArray[i].ptr_sdaPinDriver->init))
                {
                    ret = ptr_portSettingsArray[i].ptr_sdaPinDriver->init(unit, ptr_portSettingsArray[i].port, ptr_portSettingsArray[i].ptr_hwParam, NULL);
                    if (MW_E_OK == ret)
                    {
                        ptr_taskPortInfo->pin_state |= SFP_PIN_SDA_INIT_SUCCEED;
                    }
                    else
                    {
                        SFP_LOG_ERROR("SDA PIN Init fail for port:%d", ptr_portSettingsArray[i].port);
                        break;
                    }
                }
                else
                {
                    SFP_LOG_DEBUG("SDA PIN Init driver is NULL for port%d", ptr_portSettingsArray[i].port);
                }

                /* Init SFP IO PIN driver */
                if ((NULL != ptr_portSettingsArray[i].ptr_ioPinDriver) &&
                    (NULL != ptr_portSettingsArray[i].ptr_ioPinDriver->init))
                {
                    ret = ptr_portSettingsArray[i].ptr_ioPinDriver->init(unit, ptr_portSettingsArray[i].port, ptr_portSettingsArray[i].ptr_hwParam, NULL);
                    if (MW_E_OK == ret)
                    {
                        ptr_taskPortInfo->pin_state |= SFP_PIN_IO_INIT_SUCCEED;
                    }
                    else
                    {
                        SFP_LOG_ERROR("IO PIN Init fail for port:%d", ptr_portSettingsArray[i].port);
                        break;
                    }
                }
                else
                {
                    SFP_LOG_DEBUG("IO PIN Init driver is NULL for port%d", ptr_portSettingsArray[i].port);
                }
            }

            SFP_LOG_DEBUG("port:%d INIT state:0x%x", ptr_portSettingsArray[i].port, ptr_taskPortInfo->pin_state);
        }
    }

    return ret;
}

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
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_switchCtrlToPort))
        {
            ret = ptr_custFunc->pin_sda_switchCtrlToPort(unit, port);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->switchCtrlToPort))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->switchCtrlToPort(unit, port, ptr_portSettings->ptr_hwParam, NULL);
        }
    }

    return ret;
}

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
    UI32_T *const ptr_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    ret = sfp_pin_sda_switchCtrlToPort(unit, port);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_write))
        {
            ret = ptr_custFunc->pin_sda_write(unit, port, sff_2wire_addr, offset, length, ptr_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->write))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->write(unit, ptr_portSettings->ptr_hwParam->i2c_channel, sff_2wire_addr, offset, length, ptr_data);
        }
    }

    return ret;
}

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
    UI32_T *const ptr_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    ret = sfp_pin_sda_switchCtrlToPort(unit, port);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_read))
        {
            ret = ptr_custFunc->pin_sda_read(unit, port, sff_2wire_addr, offset, length, ptr_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->read))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->read(unit, ptr_portSettings->ptr_hwParam->i2c_channel, sff_2wire_addr, offset, length, ptr_data);
        }
    }

    return ret;
}

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
    const UI16_T reg_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_writeC22ByPort))
        {
            ret = ptr_custFunc->pin_sda_writeC22ByPort(unit, port, reg_addr, reg_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->writeC22ByPort))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->writeC22ByPort(unit, port, reg_addr, reg_data);
        }
    }

    return ret;
}

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
    UI16_T *const ptr_reg_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_readC22ByPort))
        {
            ret = ptr_custFunc->pin_sda_readC22ByPort(unit, port, reg_addr, ptr_reg_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->readC22ByPort))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->readC22ByPort(unit, port, reg_addr, ptr_reg_data);
        }
    }

    return ret;
}

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
    const UI16_T reg_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_writeC45ByPort))
        {
            ret = ptr_custFunc->pin_sda_writeC45ByPort(unit, port, dev_type, reg_addr, reg_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->writeC45ByPort))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->writeC45ByPort(unit, port, dev_type, reg_addr, reg_data);
        }
    }

    return ret;
}

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
    UI16_T *const ptr_reg_data)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_sda_readC45ByPort))
        {
            ret = ptr_custFunc->pin_sda_readC45ByPort(unit, port, dev_type, reg_addr, ptr_reg_data);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_sdaPinDriver) &&
            (NULL != ptr_portSettings->ptr_sdaPinDriver->readC45ByPort))
        {
            ret = ptr_portSettings->ptr_sdaPinDriver->readC45ByPort(unit, port, dev_type, reg_addr, ptr_reg_data);
        }
    }

    return ret;
}

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
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_IO_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_io_switchCtrlToPort))
        {
         ret = ptr_custFunc->pin_io_switchCtrlToPort(unit, port);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_ioPinDriver) &&
            (NULL != ptr_portSettings->ptr_ioPinDriver->switchCtrlToPort))
        {
            ret = ptr_portSettings->ptr_ioPinDriver->switchCtrlToPort(unit, port, ptr_portSettings->ptr_hwParam, NULL);
        }
    }

    return ret;
}

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
    UI8_T *const ptr_pin)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_IO_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_io_getPIONum))
        {
            ret = ptr_custFunc->pin_io_getPIONum(unit, port, type, ptr_pin);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if ((NULL != ptr_portSettings->ptr_ioPinDriver) &&
            (NULL != ptr_portSettings->ptr_ioPinDriver->getPIONum))
        {
            ret = ptr_portSettings->ptr_ioPinDriver->getPIONum(unit, port, type, ptr_pin);
        }
    }

    return ret;
}

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
    const I32_T value)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    if (SFP_PIN_PIONUM_INVALID == pin)
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_IO_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_io_setValue))
        {
            ret = ptr_custFunc->pin_io_setValue(unit, port, pin, value);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if (SFP_PIN_IS_SWPIN(pin))
        {
            ret = _sfp_pin_io_setSWPinValue(unit, pin, value, ptr_portSettings->ptr_hwParam, NULL);
        }
        else
        {
        if ((NULL != ptr_portSettings->ptr_ioPinDriver) &&
            (NULL != ptr_portSettings->ptr_ioPinDriver->setValue))
        {
            ret = ptr_portSettings->ptr_ioPinDriver->setValue(unit, pin, value, ptr_portSettings->ptr_hwParam, NULL);
        }
    }
    }

    return ret;
}

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
    I32_T *const ptr_value)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    if (SFP_PIN_PIONUM_INVALID == pin)
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return ret;
    }

    if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_IO_INIT_SUCCEED))
    {
        return ret;
    }

    if (SFP_PORT_DRIVER_CUSTOMER == ptr_portSettings->port_driver)
    {
        SFP_PIN_FUNC_T *ptr_custFunc = sfp_customer_pin_getFunc();

        if ((NULL != ptr_custFunc) && (NULL != ptr_custFunc->pin_io_getValue))
        {
            ret = ptr_custFunc->pin_io_getValue(unit, port, pin, ptr_value);
        }
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if (SFP_PIN_IS_SWPIN(pin))
        {
            ret = _sfp_pin_io_getSWPinValue(unit, pin, ptr_value, ptr_portSettings->ptr_hwParam, NULL);
        }
        else
        {
        if ((NULL != ptr_portSettings->ptr_ioPinDriver) &&
            (NULL != ptr_portSettings->ptr_ioPinDriver->getValue))
        {
            ret = ptr_portSettings->ptr_ioPinDriver->getValue(unit, pin, ptr_value, ptr_portSettings->ptr_hwParam, NULL);
        }
    }
    }

    return ret;
}

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
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T abs_pin = SFP_PIN_PIONUM_INVALID, rx_los_pin = SFP_PIN_PIONUM_INVALID;
    UI8_T sw_rxlos_implemented = FALSE, sw_rxlos;
    I32_T abs_pin_state = SFP_MODULE_PIN_HIGH;

    /* The SFP SDA PIN driver and/or the SFP SDA IO driver are not initialized
     * successfully. The software PIN cannot be implemented.
     */
    if (FALSE == sfp_pin_checkInitState(ptr_port_info->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        return;
    }

    sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_ABS, &abs_pin);
    sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_RX_LOSS, &rx_los_pin);
    if (SFP_PIN_IS_SWABSPIN(abs_pin) || SFP_PIN_IS_SWRXLOSPIN(rx_los_pin))
    {
        ptr_port_info->module_status = 0;
        if ((SFP_PIN_PIONUM_INVALID != abs_pin) && (FALSE == SFP_PIN_IS_SWABSPIN(abs_pin)))
        {
            /* The ABS pin is connected to an HW IO pin. */
            ret = sfp_pin_io_getValue(unit, ptr_port_info->port, abs_pin, &abs_pin_state);
            if ((AIR_E_OK == ret) && (SFP_MODULE_PIN_HIGH == abs_pin_state))
            {
                /* There is no SFP module inserted for the port. */
                return;
            }
        }

        ret = sfp_sff_isSWRXLOSImplemented(unit, ptr_port_info->port, &sw_rxlos_implemented);
        if (AIR_E_OK == ret)
        {
            ptr_port_info->module_status |= SFP_TASK_MODULE_STATUS_INSERTED;
            if ((SFP_PIN_IS_SWRXLOSPIN(rx_los_pin)) && (TRUE == sw_rxlos_implemented))
            {
                ptr_port_info->module_status |= SFP_TASK_MODULE_STATUS_SWRXLOS_IMPLEMENTED;
                ret = sfp_sff_getSWRXLOS(unit, ptr_port_info->port, &sw_rxlos);
                if ((AIR_E_OK == ret) && (SFP_MODULE_PIN_HIGH == sw_rxlos))
                {
                    ptr_port_info->module_status |= SFP_TASK_MODULE_STATUS_SWRXLOS_HIGH;
                }
            }
        }
    }
}

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
    void)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettingsArray = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize(), i = 0;
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;

    if ((NULL != ptr_portSettingsArray) && (0 != port_count))
    {
        for (; i < port_count; i++)
        {
            ptr_taskPortInfo = sfp_task_getPortInfo(ptr_portSettingsArray[i].port);
            if (NULL == ptr_taskPortInfo)
            {
                break;
            }

            if (TRUE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

