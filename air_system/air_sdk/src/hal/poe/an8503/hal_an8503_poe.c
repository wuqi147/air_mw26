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
/* FILE NAME:  hal_an8503_poe.c
 * PURPOSE:
 *  Implement an8503 POE module HAL function.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <hal/poe/an8503/hal_an8503_poe.h>

#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_i2c.h>
#include <hal/common/hal_poe.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_POE, "hal_an8503_poe.c");
static UI8_T  deviceIOffset = 0, deviceVOffset = 0;
static BOOL_T trim_en = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */
AIR_ERROR_NO_T
_hal_an8503_poe_setDeviceAutoPoll(
    const UI32_T unit,
    const UI32_T device)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_IVT_POLL, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        data |= AN8503_AUTO_POLL_MASK;
        data &= 0xFF;

        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_0, AN8503_IVT_POLL, HAL_POE_REG_LEN_1, &data);
    }
    return rc;
}

AIR_ERROR_NO_T
_hal_an8503_poe_setDevicePowerEventHandle(
    const UI32_T unit,
    const UI32_T device)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_POWER_EVENT_HANDLER, HAL_POE_REG_LEN_1, &data);

    if (AIR_E_OK == rc)
    {
        data |= AN8503_VOLT_BAD_EVENT_MASK;
        data |= AN8503_CURR_LIMIT_EVENT_MASK;
        data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_POWER_EVENT_HANDLER, HAL_POE_REG_LEN_1, &data);
    }
    return rc;
}

AIR_ERROR_NO_T
_hal_an8503_poe_setDeviceLedActive(
    const UI32_T unit,
    const UI32_T device,
    const UI8_T  active_level)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_LED_CONFIGURATION, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        data &= ~(AN8503_LED_ACTIVE_LEVEL_MASK);
        data |= (active_level << AN8503_LED_ACTIVE_LEVEL_OFFSET);

        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_LED_CONFIGURATION, HAL_POE_REG_LEN_1, &data);
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_an8503_poe_getDeviceOperatingMode
 * PURPOSE:
 *      Get operation mode of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_mode                -- operation mode
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDeviceOperatingMode(
    const UI32_T              unit,
    const UI32_T              device,
    HAL_POE_OPERATING_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0;
    UI8_T          mode = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_SYSTEM_CONFIGURATION, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        mode = r_data & AN8503_OPERATION_MODE_MASK;
        if (AN8503_AUTO_MODE == mode)
        {
            *ptr_mode = HAL_POE_OPERATING_MODE_AUTO;
        }
        else
        {
            *ptr_mode = HAL_POE_OPERATING_MODE_MANUAL;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDeviceScratch
 * PURPOSE:
 *      Get scratch pad value of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_value               -- scratch pad value
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_value)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_SCRATCH_REGISTER, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        *ptr_value = r_data;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setDeviceScratch
 * PURPOSE:
 *      Set scratch pad value of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      value                   -- scratch pad value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T value)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         w_data = 0;

    HAL_CHECK_MIN_MAX_RANGE(value, 0, BITS(0, 7));

    w_data = value;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg] cfg %d set w_data = %X\n", value, w_data);

    w_data &= 0xFF;
    rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_SCRATCH_REGISTER, HAL_POE_REG_LEN_1, &w_data);

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDevicePowerUpMode
 * PURPOSE:
 *      Get power up mode of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_mode                -- power up mode
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDevicePowerUpMode(
    const UI32_T             unit,
    const UI32_T             device,
    AIR_POE_POWER_UP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_SYSTEM_CONFIGURATION, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AN8503_SUSPEND_PWRUP_DIS == (r_data & AN8503_SUSPEND_PWRUP_MASK))
        {
            *ptr_mode = AIR_POE_POWER_UP_MODE_NORMAL;
        }
        else
        {
            *ptr_mode = AIR_POE_POWER_UP_MODE_MANUAL;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setDevicePowerUpMode
 * PURPOSE:
 *      Set power up mode of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      mode                    -- power up mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setDevicePowerUpMode(
    const UI32_T                  unit,
    const UI32_T                  device,
    const AIR_POE_POWER_UP_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0, w_data = 0;

    HAL_CHECK_ENUM_RANGE(device, AIR_POE_POWER_UP_MODE_LAST);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_SYSTEM_CONFIGURATION, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AIR_POE_POWER_UP_MODE_NORMAL == mode)
        {
            w_data = r_data & ~AN8503_SUSPEND_PWRUP_MASK;
        }
        else
        {
            w_data = r_data | AN8503_SUSPEND_PWRUP_MASK;
        }

        w_data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_SYSTEM_CONFIGURATION, HAL_POE_REG_LEN_1, &w_data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDevicePowerStrategy
 * PURPOSE:
 *      Get power strategy of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_strategy            -- power strategy
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDevicePowerStrategy(
    const UI32_T            unit,
    const UI32_T            device,
    AIR_POE_PWR_STRATEGY_T *ptr_strategy)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AN8503_LAST_PORT_POWERED_UP ==
            BITS_OFF_R(r_data, AN8503_VICTIM_STRATEGY_OFFSET, AN8503_VICTIM_STRATEGY_LEN))
        {
            *ptr_strategy = AIR_POE_PWR_STRATEGY_PLUG;
        }
        else
        {
            *ptr_strategy = AIR_POE_PWR_STRATEGY_PRIORITY;
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d get power strategy:%d\n", device, *ptr_strategy);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setDevicePowerStrategy
 * PURPOSE:
 *      Set power strategy of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      strategy                -- power strategy
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setDevicePowerStrategy(
    const UI32_T                 unit,
    const UI32_T                 device,
    const AIR_POE_PWR_STRATEGY_T strategy)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        data &= ~(AN8503_VICTIM_STRATEGY_MASK);
        if (AIR_POE_PWR_STRATEGY_PLUG == strategy)
        {
            data |= (AN8503_LAST_PORT_POWERED_UP << AN8503_VICTIM_STRATEGY_OFFSET);
        }
        else
        {
            data |= (AN8503_PORT_WITH_LOWEST_PRIORITY << AN8503_VICTIM_STRATEGY_OFFSET);
        }
        data &= 0xFF;
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d set power strategy to mode:%d\n", device, strategy);
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDevicePowerLimit
 * PURPOSE:
 *      Get power limit of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_enable              -- limit enable
 *      ptr_limit               -- limit value
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_limit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0, vmain = 0, tmp = 0, v_offset = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_SUPPLY_VOLTAGE, HAL_POE_REG_LEN_2, &data);
    if (AIR_E_OK == rc)
    {
        data = HAL_POE_SWAP_U16_BYTE_ORDER(data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/ get power supply voltage value %x\n", device, data);
        tmp = (data >> AN8503_SUPPLY_VOLT_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(data & AN8503_SUPPLY_VOLT_FRAC_MASK, AN8503_SUPPLY_VOLT_FRAC_BITS) / 100;

        if (trim_en == TRUE)
        {
            v_offset = hal_poe_calculateVoltageOffset(deviceVOffset);
            if (v_offset & 0x8000)
            {
                tmp = tmp - ((v_offset & 0x7FFF));
            }
            else
            {
                tmp = tmp + v_offset;
            }
        }
        vmain = tmp / 10;
        if (!(vmain >= 4800 && vmain <= 6000))
        {
            vmain = 4800;
        }

        rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &data);
    }

    if (AIR_E_OK == rc)
    {
        if (AN8503_PSE_CURR_LIMITER_EN == BITS_OFF_R(data, AN8503_PSE_CURR_LIMITER_OFFSET, AN8503_PSE_CURR_LIMITER_LEN))
        {
            *ptr_enable = TRUE;
        }
        else
        {
            *ptr_enable = FALSE;
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d get PSE current limiter mode:%s\n", device,
                   *ptr_enable ? "enable" : "disable");

        rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PSE_AVAI_CURR_LIMIT_MSB, HAL_POE_REG_LEN_2, &data);
        if (AIR_E_OK == rc)
        {
            data = HAL_POE_SWAP_U16_BYTE_ORDER(data);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d get PSE available current:%x\n", device, data);
            tmp = (data >> AN8503_PSE_AVAI_CURRENT_FRAC_BITS) * 1000;
            tmp +=
                hal_poe_handleFloat(data & AN8503_PSE_AVAI_CURRENT_FRAC_MASK, AN8503_PSE_AVAI_CURRENT_FRAC_BITS) / 100;
            tmp /= 10;
            *ptr_limit = tmp / 100 * vmain / 1000;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setDevicePowerLimit
 * PURPOSE:
 *      Set power limit of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      enable                  -- limit enable
 *      limit                   -- limit value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    const BOOL_T enable,
    const UI32_T limit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0, w_data = 0, tmp = 0, i = 0, current_limit = 0, v_offset = 0;
    const UI32_T   bit_table[] = {512, 256, 128, 64};

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_SUPPLY_VOLTAGE, HAL_POE_REG_LEN_2, &data);
    if (AIR_E_OK == rc)
    {
        data = HAL_POE_SWAP_U16_BYTE_ORDER(data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/ get power supply voltage value %x\n", device, data);
        tmp = (data >> AN8503_SUPPLY_VOLT_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(data & AN8503_SUPPLY_VOLT_FRAC_MASK, AN8503_SUPPLY_VOLT_FRAC_BITS) / 100;

        if (trim_en == TRUE)
        {
            v_offset = hal_poe_calculateVoltageOffset(deviceVOffset);
            if (v_offset & 0x8000)
            {
                tmp = tmp - ((v_offset & 0x7FFF));
            }
            else
            {
                tmp = tmp + v_offset;
            }
        }
        tmp /= 10;
        if (!(tmp >= 4800 && tmp <= 6000))
        {
            tmp = 4800;
        }
        current_limit = limit * 100000 / tmp;

        rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &data);
    }

    if (AIR_E_OK == rc)
    {
        if (TRUE == enable)
        {
            data |= AN8503_PSE_CURR_LIMITER_MASK;
        }
        else
        {
            data &= ~AN8503_PSE_CURR_LIMITER_MASK;
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d set PSE current limiter to mode:%s\n", device,
                   enable ? "enable" : "disable");

        data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_CURRENT_LIMIT_CTRL, HAL_POE_REG_LEN_1, &data);
    }

    if (AIR_E_OK == rc)
    {
        HAL_CHECK_MIN_MAX_RANGE(current_limit / 100, 0, AN8503_MAX_PSE_AVAI_CURRENT_INT);
        data = (current_limit / 100) << AN8503_PSE_AVAI_CURRENT_FRAC_BITS;
        tmp = (current_limit % 100) * 10;
        for (i = 0; i < AN8503_PSE_AVAI_CURRENT_FRAC_BITS; i++)
        {
            if (tmp >= bit_table[i])
            {
                tmp -= bit_table[i];
                data |= (1 << (AN8503_PSE_AVAI_CURRENT_FRAC_BITS - i - 1));
            }
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d set PSE available current to %x\n", device, data);

        w_data = (data & 0xFF00) >> 8;
        rc |=
            hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PSE_AVAI_CURR_LIMIT_MSB, HAL_POE_REG_LEN_1, &w_data);
        w_data = data & 0xFF;
        rc |=
            hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PSE_AVAI_CURR_LIMIT_LSB, HAL_POE_REG_LEN_1, &w_data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDeviceErrorDelay
 * PURPOSE:
 *      Get error delay of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_delay               -- error delay
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDeviceErrorDelay(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_delay)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_ERROR_DELAY, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d get error delay:%x\n", device, r_data);
        *ptr_delay = r_data / 10;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setDeviceErrorDelay
 * PURPOSE:
 *      Set error delay of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      delay                   -- error delay
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setDeviceErrorDelay(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T delay)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         w_data = 0;

    w_data = delay * 10;
    HAL_CHECK_MIN_MAX_RANGE(w_data, AN8503_MIN_ERROR_DELAY, BITS(0, 7));

    w_data &= 0xFF;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d set error delay to %x\n", device, w_data);
    rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_0, AN8503_ERROR_DELAY, HAL_POE_REG_LEN_1, &w_data);

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getDeviceMeasurement
 * PURPOSE:
 *      Get measurement of the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_meas                -- measurement data
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDeviceMeasurement(
    const UI32_T                  unit,
    const UI32_T                  device,
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         r_data = 0, tmp = 0;
    UI32_T         v_offset = 0;

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PSE_CONSUMED_CURRENT, HAL_POE_REG_LEN_2, &r_data);
    if (AIR_E_OK == rc)
    {
        r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/ get PSE consumed current value %x\n", device, r_data);
        tmp = (r_data >> AN8503_PSE_CONSUMED_CURR_FRAC_BITS) * 1000;
        tmp +=
            hal_poe_handleFloat(r_data & AN8503_PSE_CONSUMED_CURR_FRAC_MASK, AN8503_PSE_CONSUMED_CURR_FRAC_BITS) / 100;
        tmp /= 10;
        ptr_meas->total_current = tmp;

        rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_SUPPLY_VOLTAGE, HAL_POE_REG_LEN_2, &r_data);
    }

    if (AIR_E_OK == rc)
    {
        r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/ get power supply voltage value %x\n", device, r_data);
        tmp = (r_data >> AN8503_SUPPLY_VOLT_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(r_data & AN8503_SUPPLY_VOLT_FRAC_MASK, AN8503_SUPPLY_VOLT_FRAC_BITS) / 100;

        if (trim_en == TRUE)
        {
            v_offset = hal_poe_calculateVoltageOffset(deviceVOffset);
            if (v_offset & 0x8000)
            {
                tmp = tmp - ((v_offset & 0x7FFF));
            }
            else
            {
                tmp = tmp + v_offset;
            }
        }
        tmp /= 10;
        ptr_meas->supply_voltage = tmp;
        ptr_meas->consume_power = (ptr_meas->total_current / 100) * ptr_meas->supply_voltage / 1000;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortPowerUp
 * PURPOSE:
 *      Get state machine of the multiple ports.
 * INPUT:
 *      unit                    -- unit id
 *      ptr_poe_info            -- pointer of poe port info
 *      poe_info_cnt            -- count of poe port info
 * OUTPUT:
 *      ptr_ready               -- pointer of state of the state machine
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortPowerUp(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    BOOL_T                   *ptr_ready)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         dev_bitmap = 0;
    UI32_T         dev_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_CALC_U32_COUNT(AN8503_TOTAL_PORTS, 4)];
    UI32_T         i = 0;
    UI32_T         device = 0;
    UI32_T         poe_port = 0;
    UI32_T         val = 0, offset = 0, byte_shift = 0;
    const UI8_T    offset_map[4] = {0, 1, 2, 3};

    osal_memset(dev_data, 0, sizeof(dev_data));

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        dev_bitmap |= (1 << device);
    }

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        if (dev_bitmap & (1 << i))
        {
            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_1, AN8503_PORT_STATE_MACHINE_BASE, AN8503_TOTAL_PORTS,
                                 dev_data[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }
        }
    }

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);
        val = dev_data[device][poe_port / 4];
        offset = poe_port % 4;
        byte_shift = offset_map[offset] * 8;
        val = (val >> byte_shift) & 0xFF;

        if (val & AN8503_PWRUP_SUSPENDED_MASK)
        {
            ptr_ready[i] = TRUE;
        }
        else
        {
            ptr_ready[i] = FALSE;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_triggerPortPowerUp
 * PURPOSE:
 *      Trigger power up of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_triggerPortPowerUp(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0, w_data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_STATE_MACHINE(poe_port), HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        w_data = (r_data | AN8503_START_PWRUP_MASK);
        DIAG_PRINT(HAL_DBG_INFO,
                   "[Dbg]:d:%d/p:%d(%d) set port to re-start the state machine for entering power up state\n", device,
                   port, poe_port);

        w_data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_STATE_MACHINE(poe_port), HAL_POE_REG_LEN_1,
                              &w_data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortPairMode
 * PURPOSE:
 *      Get wiring alternative of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_pair                -- alternative value
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortPairMode(
    const UI32_T    unit,
    const UI32_T    port,
    HAL_POE_PAIR_T *ptr_pair)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_ALTERNATIVE_A_B, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AN8503_PAIR_A == BITS_OFF_R(r_data, poe_port, AN8503_ALT_A_B_LEN))
        {
            *ptr_pair = HAL_POE_PAIR_A;
        }
        else
        {
            *ptr_pair = HAL_POE_PAIR_B;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setPortPairMode
 * PURPOSE:
 *      Set wiring alternative of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      pair                    -- alternative value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortPairMode(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_POE_PAIR_T pair)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0, w_data = 0;

    HAL_CHECK_ENUM_RANGE(pair, HAL_POE_PAIR_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_ALTERNATIVE_A_B, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (HAL_POE_PAIR_A == pair)
        {
            w_data = (r_data & ~(BIT(poe_port)));
        }
        else
        {
            w_data = (r_data | (BIT(poe_port)));
        }

        w_data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_ALTERNATIVE_A_B, HAL_POE_REG_LEN_1, &w_data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortType
 * PURPOSE:
 *      Get power supply standard of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_type                -- power supply standard
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_TYPE_T             *ptr_type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_AF_AT_MODE, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AN8503_AT_MODE == BITS_OFF_R(r_data, poe_port, AN8503_AF_AT_MODE_LEN))
        {
            *ptr_type = AIR_POE_TYPE_AT;
        }
        else
        {
            *ptr_type = AIR_POE_TYPE_AF;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setPortType
 * PURPOSE:
 *      Set power supply standard of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 *      type                    -- power supply standard
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_TYPE_T        type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0, w_data = 0;

    HAL_CHECK_ENUM_RANGE(type, AIR_POE_TYPE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_AF_AT_MODE, HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        if (AIR_POE_TYPE_AF == type)
        {
            w_data = (r_data & ~(BIT(poe_port)));
        }
        else
        {
            w_data = (r_data | (BIT(poe_port)));
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/p:%d(%d) set port to mode:%d\n", device, port, poe_port, type);

        w_data &= 0xFF;
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_0, AN8503_AF_AT_MODE, HAL_POE_REG_LEN_1, &w_data);
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortPse
 * PURPOSE:
 *      Get PSE functionality of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_mode                -- PSE functionality mode
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PSE_MODE_T         *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PWR_CTRL(poe_port), HAL_POE_REG_LEN_1, &data);

    if (AIR_E_OK == rc)
    {
        if (AN8503_PORT_PWR_CTRL_PSE_DISABLE == (data & AN8503_PORT_PWR_CTRL_PSE_MASK))
        {
            *ptr_mode = AIR_POE_PSE_MODE_DISABLE;
        }
        else if (AN8503_PORT_PWR_CTRL_PSE_ENABLE == (data & AN8503_PORT_PWR_CTRL_PSE_MASK))
        {
            *ptr_mode = AIR_POE_PSE_MODE_ENABLE;
        }
        else if (AN8503_PORT_PWR_CTRL_PSE_FORCE == (data & AN8503_PORT_PWR_CTRL_PSE_MASK))
        {
            *ptr_mode = AIR_POE_PSE_MODE_FORCE_POWER;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setPortPse
 * PURPOSE:
 *      Set PSE functionality of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 *      mode                    -- PSE functionality mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      Force mode must be set when in Enable mode;
 *      it cannot be set when in Disable mode.
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_PSE_MODE_T    mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         data = 0;

    HAL_CHECK_ENUM_RANGE(mode, AIR_POE_PSE_MODE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PWR_CTRL(poe_port), HAL_POE_REG_LEN_1, &data);

    if (AIR_E_OK == rc)
    {
        data &= ~(AN8503_PORT_PWR_CTRL_PSE_MASK);
        if (AIR_POE_PSE_MODE_DISABLE == mode)
        {
            data |= (AN8503_PORT_PWR_CTRL_PSE_DISABLE << AN8503_PORT_PWR_CTRL_PSE_OFFT);
        }
        else
        {
            /* to set force mode, first set it to enable mode */
            data |= (AN8503_PORT_PWR_CTRL_PSE_ENABLE << AN8503_PORT_PWR_CTRL_PSE_OFFT);
        }
        data &= 0xFF;

        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PWR_CTRL(poe_port), HAL_POE_REG_LEN_1, &data);
    }

    if (AIR_E_OK == rc)
    {
        /* to set force mode, then switch to force mode */
        if (AIR_POE_PSE_MODE_FORCE_POWER == mode)
        {
            data &= ~(AN8503_PORT_PWR_CTRL_PSE_MASK);
            data |= (AN8503_PORT_PWR_CTRL_PSE_FORCE << AN8503_PORT_PWR_CTRL_PSE_OFFT);
            data &= 0xFF;

            rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PWR_CTRL(poe_port), HAL_POE_REG_LEN_1,
                                  &data);
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortPriority
 * PURPOSE:
 *      Get priority of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_priority            -- priority
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    UI32_T                     *ptr_priority)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PRIORITY(poe_port), HAL_POE_REG_LEN_1, &r_data);
    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/p:%d(%d) get port priority:%x\n", device, port, poe_port, r_data);
        *ptr_priority = r_data;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setPortPriority
 * PURPOSE:
 *      Set priority of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 *      priority                -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const UI32_T                priority)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         w_data = 0;

    HAL_CHECK_MIN_MAX_RANGE(priority, 0, BITS(0, 5));
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    w_data = priority;
    w_data &= 0xFF;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/p:%d(%d) set port priority to %x\n", device, port, poe_port, w_data);
    rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_PRIORITY(poe_port), HAL_POE_REG_LEN_1, &w_data);

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortCurrentLimit
 * PURPOSE:
 *      Get current limit of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_current             -- current limit
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_current)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         r_data = 0, tmp = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    /* get PSE available current */
    rc =
        hal_poe_readReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_CURR_LIMIT_MSB(poe_port), HAL_POE_REG_LEN_2, &r_data);
    if (AIR_E_OK == rc)
    {
        r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:d:%d/p:%d(%d) get port current limit %x\n", device, port, poe_port, r_data);
        tmp = (r_data >> AN8503_PORT_CURR_LIMIT_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(r_data & AN8503_PORT_CURR_LIMIT_FRAC_MASK, AN8503_PORT_CURR_LIMIT_FRAC_BITS) / 100;
        tmp /= 10;
        *ptr_current = tmp;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_setPortCurrentLimit
 * PURPOSE:
 *      Set current limit of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      current                 -- current limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T current)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device = 0, poe_port = 0;
    UI32_T         w_data = 0, tmp = 0, res = 0, i = 0, msb_data = 0, lsb_data = 0;
    const UI32_T   bit_table[] = {512, 256, 128, 64};

    HAL_CHECK_MIN_MAX_RANGE(current / 100, 0, AN8503_MAX_PORT_CURR_INT);
    w_data = (current / 100) << AN8503_PORT_CURR_LIMIT_FRAC_BITS;
    tmp = (current % 100) * 10;
    for (i = 0; i < AN8503_PORT_CURR_LIMIT_FRAC_BITS; i++)
    {
        if (tmp >= bit_table[i])
        {
            tmp -= bit_table[i];
            res |= (1 << (AN8503_PORT_CURR_LIMIT_FRAC_BITS - i - 1));
        }
    }
    w_data = w_data | res;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg] cfg %d set w_data = %X\n", current, w_data);

    msb_data = (w_data & 0xFF00) >> 8;
    rc |= hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_CURR_LIMIT_MSB(poe_port), HAL_POE_REG_LEN_1,
                           &msb_data);
    lsb_data = w_data & 0xFF;
    rc |= hal_poe_writeReg(unit, device, HAL_POE_PAGE_1, AN8503_PORT_CURR_LIMIT_LSB(poe_port), HAL_POE_REG_LEN_1,
                           &lsb_data);

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortMeasurement
 * PURPOSE:
 *      Get measurement of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      ptr_poe_info            -- pointer of poe port info
 *      poe_info_cnt            -- count of poe port info
 * OUTPUT:
 *      ptr_meas                -- pointer of measurement data
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortMeasurement(
    const UI32_T                unit,
    const HAL_POE_PORT_IND_T   *ptr_poe_ind,
    const UI32_T                poe_info_cnt,
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas)
{
    /*----------------------------------------------------------------------------------
     * Currnt, voltage and temperature require accessing both the MSB and LSB. However,
     * there may be cases where the hardware updates only the MSB while the LSB remains
     * unchanged, resulting in incorrect values.
     * To address this problem:
     * - The current, voltage, and temperature of each port are read twice. If the difference
     * - between the two readings is less than 0x50, the second reading is returned. Otherwise,
     * - a third reading is performed, and its result is used directly regardless of the
     * - value and difference.
     *--------------------------------------------------------------------------------*/

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         dev_bitmap = 0;
    UI32_T         dev_data_first[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_U32_NUM_PER_BLOCK * 3] = {{0}};
    UI32_T         dev_data_second[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_U32_NUM_PER_BLOCK * 3] = {{0}};
    UI32_T         dev_supply_voltage_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][1] = {{0}};
    UI32_T         i = 0;
    UI32_T         device = 0;
    UI32_T         poe_port = 0;
    UI32_T         current_val_first = 0, voltage_val_first = 0, temperature_val_first = 0, supply_voltage_val = 0;
    UI32_T         current_val_second = 0, voltage_val_second = 0, temperature_val_second = 0;
    UI32_T         val = 0, tmp = 0, v_offset = 0, current_frac = 0, r_data = 0;

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        dev_bitmap |= (1 << device);
    }

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        if (dev_bitmap & (1 << i))
        {
            /*------------------------------------------------------------------
             * Read the current of ports twice to ensure that the value is stable,
             * and read the current, voltage, and temperature of all ports in a
             * single I2C access.
             *------------------------------------------------------------------*/
            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_0, AN8503_PORT_CURRENT, HAL_POE_REG_ADDR_BLOCK_SIZE * 3,
                                 dev_data_first[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }

            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_0, AN8503_PORT_CURRENT, HAL_POE_REG_ADDR_BLOCK_SIZE * 3,
                                 dev_data_second[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }

            /* Read the supply voltage of the device */
            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_0, AN8503_SUPPLY_VOLTAGE, HAL_POE_REG_LEN_2,
                                 dev_supply_voltage_data[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }
        }
    }

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);
        current_val_first = dev_data_first[device][poe_port / 2];
        current_val_second = dev_data_second[device][poe_port / 2];
        voltage_val_first = dev_data_first[device][4 + poe_port / 2];
        voltage_val_second = dev_data_second[device][4 + poe_port / 2];
        temperature_val_first = dev_data_first[device][8 + poe_port / 2];
        temperature_val_second = dev_data_second[device][8 + poe_port / 2];
        supply_voltage_val = dev_supply_voltage_data[device][0];
        if (poe_port % 2 == 0)
        {
            current_val_first = current_val_first & BITS(0, 15);
            current_val_second = current_val_second & BITS(0, 15);
            voltage_val_first = voltage_val_first & BITS(0, 15);
            voltage_val_second = voltage_val_second & BITS(0, 15);
            temperature_val_first = temperature_val_first & BITS(0, 15);
            temperature_val_second = temperature_val_second & BITS(0, 15);
        }
        else
        {
            current_val_first = BITS_OFF_R(current_val_first, 16, 16);
            current_val_second = BITS_OFF_R(current_val_second, 16, 16);
            voltage_val_first = BITS_OFF_R(voltage_val_first, 16, 16);
            voltage_val_second = BITS_OFF_R(voltage_val_second, 16, 16);
            temperature_val_first = BITS_OFF_R(temperature_val_first, 16, 16);
            temperature_val_second = BITS_OFF_R(temperature_val_second, 16, 16);
        }
        supply_voltage_val = supply_voltage_val & 0xFFFF;
        current_val_first = HAL_POE_SWAP_U16_BYTE_ORDER(current_val_first);
        current_val_second = HAL_POE_SWAP_U16_BYTE_ORDER(current_val_second);
        voltage_val_first = HAL_POE_SWAP_U16_BYTE_ORDER(voltage_val_first);
        voltage_val_second = HAL_POE_SWAP_U16_BYTE_ORDER(voltage_val_second);
        temperature_val_first = HAL_POE_SWAP_U16_BYTE_ORDER(temperature_val_first);
        temperature_val_second = HAL_POE_SWAP_U16_BYTE_ORDER(temperature_val_second);
        supply_voltage_val = HAL_POE_SWAP_U16_BYTE_ORDER(supply_voltage_val);

        /* Check the validity of the current value */
        if (HAL_POE_U32_DIFF(current_val_first, current_val_second) > 0x50)
        {
            rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_PORT_CURRENT + (poe_port * 2), HAL_POE_REG_LEN_2,
                                 &r_data);
            if (AIR_E_OK == rc)
            {
                r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
                val = r_data;
            }
        }
        else
        {
            val = current_val_second;
        }
        current_frac = val & AN8503_PORT_CURR_FRAC_MASK;
        val = val >> AN8503_PORT_CURR_FRAC_BITS;
        if ((trim_en == TRUE) && (val != 0))
        {
            hal_poe_getFixPortCurrent(poe_port, deviceIOffset, &val);
        }
        val = (val << AN8503_PORT_CURR_FRAC_BITS) | (current_frac & AN8503_PORT_CURR_FRAC_MASK);
        tmp = (val >> AN8503_PORT_CURR_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(val & AN8503_PORT_CURR_FRAC_MASK, AN8503_PORT_CURR_FRAC_BITS) / 100;
        tmp /= 10;
        ptr_meas[i].current = tmp;
        /* Check the validity of the voltage value */
        if (HAL_POE_U32_DIFF(voltage_val_first, voltage_val_second) > 0x50)
        {
            rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_PORT_VOLTAGE + (poe_port * 2), HAL_POE_REG_LEN_2,
                                 &r_data);
            if (AIR_E_OK == rc)
            {
                r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
                val = supply_voltage_val - r_data;
            }
        }
        else
        {
            val = supply_voltage_val - voltage_val_second;
        }
        tmp = (val >> AN8503_PORT_VOLT_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(val & AN8503_PORT_VOLT_FRAC_MASK, AN8503_PORT_VOLT_FRAC_BITS) / 100;
        if (trim_en == TRUE)
        {
            v_offset = hal_poe_calculateVoltageOffset(deviceVOffset);
            if (v_offset & 0x8000)
            {
                tmp = tmp - ((v_offset & 0x7FFF));
            }
            else
            {
                tmp = tmp + v_offset;
            }
        }
        tmp /= 10;
        ptr_meas[i].voltage = tmp;
        /* Check the validity of the temperature value */
        if (HAL_POE_U32_DIFF(temperature_val_first, temperature_val_second) > 0x50)
        {
            rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, AN8503_PORT_TEMPERATURE + (poe_port * 2),
                                 HAL_POE_REG_LEN_2, &r_data);
            if (AIR_E_OK == rc)
            {
                r_data = HAL_POE_SWAP_U16_BYTE_ORDER(r_data);
                val = r_data;
            }
        }
        else
        {
            val = temperature_val_second;
        }
        tmp = (val >> AN8503_PORT_TEMPERATURE_FRAC_BITS) * 1000;
        tmp += hal_poe_handleFloat(val & AN8503_PORT_TEMPERATURE_FRAC_MASK, AN8503_PORT_TEMPERATURE_FRAC_BITS) / 100;
        tmp /= 10;
        ptr_meas[i].temperature = tmp;
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortStatus
 * PURPOSE:
 *      Get status of the multiple ports.
 * INPUT:
 *      unit                    -- unit id
 *      ptr_poe_info            -- pointer of poe port info
 *      poe_info_cnt            -- count of poe port info
 * OUTPUT:
 *      ptr_ps                  -- pointer of status
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortStatus(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    AIR_POE_PORT_STATUS_T    *ptr_ps)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         dev_bitmap = 0;
    UI32_T         dev_sig_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_CALC_U32_COUNT(AN8503_TOTAL_PORTS, 4)];
    UI32_T         dev_class_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_CALC_U32_COUNT(AN8503_TOTAL_PORTS, 8)];
    UI32_T         dev_power_status_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][1] = {{0}};
    UI32_T         i = 0;
    UI32_T         device = 0;
    UI32_T         poe_port = 0;
    UI32_T         sig_val = 0, class_val = 0, power_status_val = 0;
    const UI8_T    offset_map[4] = {0, 1, 2, 3};

    osal_memset(dev_sig_data, 0, sizeof(dev_sig_data));
    osal_memset(dev_class_data, 0, sizeof(dev_class_data));

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        dev_bitmap |= (1 << device);
    }

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        if (dev_bitmap & (1 << i))
        {
            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_0, AN8503_DETECTED_SIGNATURE, AN8503_TOTAL_PORTS,
                                 dev_sig_data[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }

            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_0, AN8503_DETECTED_PD_CLASS, AN8503_TOTAL_PORTS / 2,
                                 dev_class_data[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }

            rc = hal_poe_readReg(unit, i, HAL_POE_PAGE_1, AN8503_PORT_POWER_STATUS, HAL_POE_REG_LEN_1,
                                 dev_power_status_data[i]);
            if (AIR_E_OK != rc)
            {
                return rc;
            }
        }
    }

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);
        sig_val = dev_sig_data[device][poe_port / 4];
        class_val = dev_class_data[device][poe_port / 8];
        power_status_val = dev_power_status_data[device][0];

        sig_val = ((sig_val >> (offset_map[poe_port % 4] * 8)) & 0xFF) & AN8503_SIGNATURE_MASK;
        class_val = (class_val >> (offset_map[(poe_port / 2) % 4] * 8)) & 0xFF;
        if (poe_port % 2 == 0)
        {
            class_val = class_val & AN8503_PD_CLASS_MASK;
        }
        else
        {
            class_val = (class_val >> AN8503_PD_CLASS_PORT_SHIFT) & AN8503_PD_CLASS_MASK;
        }
        power_status_val = BITS_OFF_R(power_status_val, poe_port, AN8503_PORT_POWER_LEN);

        if (AN8503_SIGNATURE_BAD == sig_val)
        {
            ptr_ps[i].pd_signature = AIR_POE_SIGNATURE_BAD;
        }
        else if (AN8503_SIGNATURE_GOOD == sig_val)
        {
            ptr_ps[i].pd_signature = AIR_POE_SIGNATURE_GOOD;
        }
        else if (AN8503_SIGNATURE_OPEN == sig_val)
        {
            ptr_ps[i].pd_signature = AIR_POE_SIGNATURE_OPEN;
        }

        if (AN8503_CLASS_0 == class_val)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_0;
        }
        else if (AN8503_CLASS_1 == class_val)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_1;
        }
        else if (AN8503_CLASS_2 == class_val)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_2;
        }
        else if (AN8503_CLASS_3 == class_val)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_3;
        }
        else if (AN8503_CLASS_4 == class_val)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_4;
        }
        else
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_LAST;
        }

        if (AN8503_PORT_POWER_ON == power_status_val)
        {
            ptr_ps[i].power_status = AIR_POE_POWER_STATUS_ON;
        }
        else if (AN8503_PORT_POWER_OFF == power_status_val)
        {
            ptr_ps[i].power_status = AIR_POE_POWER_STATUS_OFF;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_getPortEvent
 * PURPOSE:
 *      Get event of the multiple ports.
 * INPUT:
 *      unit                    -- unit id
 *      ptr_poe_info            -- pointer of poe port info
 *      poe_info_cnt            -- count of poe port info
 * OUTPUT:
 *      ptr_event               -- pointer of power event
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_getPortEvent(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    HAL_POE_POWER_EVENT_T    *ptr_event)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         dev_bitmap = 0;
    UI32_T         dev_data[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP][HAL_POE_CALC_U32_COUNT(AN8503_TOTAL_PORTS, 4)];
    UI32_T         i = 0;
    UI32_T         device = 0;
    UI32_T         poe_port = 0;
    UI32_T         word = 0, offset = 0, event_byte = 0, w_data = 0;
    const UI8_T    offset_map[4] = {0, 1, 2, 3};

    osal_memset(dev_data, 0, sizeof(dev_data));

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        dev_bitmap |= (1 << device);
    }

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        if (dev_bitmap & (1 << i))
        {
            rc =
                hal_poe_readReg(unit, i, HAL_POE_PAGE_1, AN8503_PORT_POWER_EVENT_BASE, AN8503_TOTAL_PORTS, dev_data[i]);

            w_data = AN8503_PORT_POWER_EVENT_MASK;
            for (poe_port = 0; poe_port < AN8503_TOTAL_PORTS; poe_port++)
            {
                rc |= hal_poe_writeReg(unit, i, HAL_POE_PAGE_1, AN8503_PORT_POWER_EVENT(poe_port), HAL_POE_REG_LEN_1,
                                       &w_data);
            }

            if (AIR_E_OK != rc)
            {
                return rc;
            }
        }
    }

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);
        word = dev_data[device][poe_port / 4];
        offset = poe_port % 4;
        UI32_T byte_shift = offset_map[offset] * 8;
        event_byte = (word >> byte_shift) & 0xFF;

        ptr_event[i].flags = 0;
        if (event_byte & AN8503_OVERLOAD_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_OVERLOAD;
        }
        if (event_byte & AN8503_SHORT_CIRCUIT_LIMIT_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_SHORT_CIRCUIT;
        }
        if (event_byte & AN8503_MPS_ERROR_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_DC_DISCONNECT;
        }
        if (event_byte & AN8503_VOLT_BAD_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_VOLTAGE_BAD_EVENT;
        }
        if (event_byte & AN8503_THERMAL_SHUTDOWN_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_THERMAL_SHUTDOWN;
        }
        if (event_byte & AN8503_CURR_LIMIT_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_CURRENT_LIMIT;
        }
        if (event_byte & AN8503_VOLT_LIMIT_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_TRUNK_VOLTAGE_LIMIT;
        }
        if (event_byte & AN8503_TEMPERATURE_LIMIT_EVENT)
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_TEMPERATURE;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_an8503_poe_init
 * PURPOSE:
 *      AN8503 POE initialization.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_NOT_SUPPORT       -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_init(
    const UI32_T unit,
    const UI32_T device)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T poe_cfg = {0};
    UI8_T           offset = 0;

    /* set auto poll */
    rc = _hal_an8503_poe_setDeviceAutoPoll(unit, device);

    if (AIR_E_OK == rc)
    {
        /* set power event handle  */
        rc = _hal_an8503_poe_setDevicePowerEventHandle(unit, device);
    }
    if (AIR_E_OK == rc)
    {
        /* set scratch value */
        poe_cfg.param0 = device;
        poe_cfg.param1 = 0;
        poe_cfg.value = AN8503_DEFAULT_SCRATCH_VALUE;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_POE_RESET_SCRATCH, &poe_cfg);
        if (0 == poe_cfg.value)
        {
            /* 0 is the default value for the hardware, and it is not expected that users will set it to 0 */
            rc = AIR_E_BAD_PARAMETER;
        }
        else
        {
            rc = hal_an8503_poe_setDeviceScratch(unit, device, poe_cfg.value);
        }
    }
    if (AIR_E_OK == rc)
    {
        /* set poe LED active level */
        poe_cfg.param0 = device;
        poe_cfg.param1 = 0;
        poe_cfg.value = 0; /* default value */
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_POE_LED_BEHAVIOR, &poe_cfg);
        if (AIR_E_OK == rc)
        {
            rc = _hal_an8503_poe_setDeviceLedActive(unit, device, (poe_cfg.value & AN8503_LED_BHV_ACTIVE_LEVEL_MASK));
        }
    }
    /* Calculate Current and Voltage offset */
    if (AIR_E_OK == rc && ((AIR_E_OK == hal_poe_getOffsetState(unit, device, &trim_en)) && (trim_en == TRUE)))
    {
        if (AIR_E_OK == hal_poe_getCurrentOffset(unit, device, &offset))
        {
            deviceIOffset = offset;
        }
        if (AIR_E_OK == hal_poe_getVoltageOffset(unit, device, &offset))
        {
            deviceVOffset = offset;
        }
    }

    return rc;
}

const HAL_POE_DRIVER_T _ext_an8503_poe_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_an8503_poe_init,
    hal_an8503_poe_getDeviceOperatingMode,
    hal_an8503_poe_getDeviceScratch,
    hal_an8503_poe_setDeviceScratch,
    hal_an8503_poe_getDevicePowerUpMode,
    hal_an8503_poe_setDevicePowerUpMode,
    hal_an8503_poe_getDevicePowerStrategy,
    hal_an8503_poe_setDevicePowerStrategy,
    hal_an8503_poe_getDevicePowerLimit,
    hal_an8503_poe_setDevicePowerLimit,
    hal_an8503_poe_getDeviceErrorDelay,
    hal_an8503_poe_setDeviceErrorDelay,
    hal_an8503_poe_getDeviceMeasurement,
    hal_an8503_poe_getPortPowerUp, /* get multiple ports */
    hal_an8503_poe_triggerPortPowerUp,
    hal_an8503_poe_getPortPairMode,
    hal_an8503_poe_setPortPairMode,
    hal_an8503_poe_getPortType,
    hal_an8503_poe_setPortType,
    hal_an8503_poe_getPortPse,
    hal_an8503_poe_setPortPse,
    hal_an8503_poe_getPortPriority,
    hal_an8503_poe_setPortPriority,
    hal_an8503_poe_getPortCurrentLimit,
    hal_an8503_poe_setPortCurrentLimit,
    hal_an8503_poe_getPortMeasurement, /* get multiple ports */
    hal_an8503_poe_getPortStatus,      /* get multiple ports */
    hal_an8503_poe_getPortEvent,       /* get multiple ports */
};

/* FUNCTION NAME:   hal_an8503_poe_getDriver
 * PURPOSE:
 *      AN8503 POE probe get driver.
 * INPUT:
 *      None
 * OUTPUT:
 *      pptr_poe_driver         -- The pointer for store function vector.
 * RETURN:
 *      AIR_E_OK                -- Initialize the device driver successfully .
 *      AIR_E_OTHERS            -- Fail to initialize the device driver.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_an8503_poe_getDriver(
    HAL_POE_DRIVER_T **pptr_poe_driver)
{
    (*pptr_poe_driver) = (HAL_POE_DRIVER_T *)&_ext_an8503_poe_func_vec;
    return (AIR_E_OK);
}
