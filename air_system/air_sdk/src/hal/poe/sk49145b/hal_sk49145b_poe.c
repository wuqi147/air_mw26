/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Airoha Technology Corp. (C) 2024
 *
 *******************************************************************************/
/* FILE NAME:  hal_sk49145b_poe.c
 * PURPOSE:
 *  Implement SK49145B POE module HAL function.
 * NOTES:
 *  Derived from DH2184 driver. Register offsets and conversion factors must
 *  be verified against SK49145B datasheet (SK49145_寄存器说明_0.2) during
 *  hardware bring-up. Known differences from DH2184:
 *    - Different ID register value at 0x1B
 *    - CLS5_EN is active low
 *    - Default UV threshold 44V, OV threshold 57V
 *    - Current/voltage LSB/MSB byte order and scaling may differ
 */
#include <hal/poe/sk49145b/hal_sk49145b_poe.h>

#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_i2c.h>
#include <hal/common/hal_poe.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_POE, "hal_sk49145b_poe.c");

static AIR_ERROR_NO_T
_hal_sk49145b_poe_readReg(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T addr,
    const UI32_T len,
    UI32_T      *ptr_val)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_HW_TBL_LOCK(unit, device);
    rc = hal_i2c_readReg(unit, HAL_POE_DEVICE_I2C_BUS_ID(unit, device), HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, device),
                         addr, HAL_POE_REG_LEN_1, len, ptr_val);
    HAL_POE_HW_TBL_UNLOCK(unit, device);

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "SK49145B device %d read addr[0x%x] fail!\n", device, addr);
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_sk49145b_poe_writeReg(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T addr,
    const UI32_T len,
    UI32_T      *ptr_val)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_HW_TBL_LOCK(unit, device);
    rc = hal_i2c_writeReg(unit, HAL_POE_DEVICE_I2C_BUS_ID(unit, device), HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, device),
                          addr, HAL_POE_REG_LEN_1, len, ptr_val);
    HAL_POE_HW_TBL_UNLOCK(unit, device);

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "SK49145B device %d write addr[0x%x] fail!\n", device, addr);
    }

    return rc;
}

static UI32_T
_hal_sk49145b_poe_portModeOffset(
    const UI32_T poe_port)
{
    return poe_port * 2;
}

static UI32_T
_hal_sk49145b_poe_bit(
    const UI32_T poe_port)
{
    return BIT(poe_port);
}

static UI32_T
_hal_sk49145b_poe_portCfgReg(
    const UI32_T base,
    const UI32_T poe_port)
{
    return base + (poe_port * SK49145B_PORT_CFG_STRIDE);
}

static AIR_ERROR_NO_T
_hal_sk49145b_poe_setPortMode(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T poe_port,
    const UI32_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;
    UI32_T         offset = _hal_sk49145b_poe_portModeOffset(poe_port);

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        data &= ~(0x3 << offset);
        data |= ((mode & 0x3) << offset);
        data &= 0xFF;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_init(
    const UI32_T unit,
    const UI32_T device)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T poe_cfg = {0};
    UI32_T          data = 0;

    data = SK49145B_WATCHDOG_DISABLE;
    rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_WATCHDOG, HAL_POE_REG_LEN_1, &data);

    if (AIR_E_OK == rc)
    {
        data = SK49145B_ALL_CLASS_DET_MASK;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_DETECT_CLASS_ENABLE, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK == rc)
    {
        data = SK49145B_ALL_PORTS_MASK << 4;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_DISCONNECT_ENABLE, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK == rc)
    {
        data = SK49145B_ALL_PORTS_MASK;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_HIGH_POWER_ENABLE, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK == rc)
    {
        data = SK49145B_INT_CLR;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_GLOBAL_BUTTON, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK == rc)
    {
        poe_cfg.param0 = device;
        poe_cfg.param1 = 0;
        poe_cfg.value = SK49145B_DEFAULT_SCRATCH_VALUE;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_POE_RESET_SCRATCH, &poe_cfg);
        if (AIR_E_OK == rc)
        {
            data = poe_cfg.value & 0xFF;
            rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_SCRATCH, HAL_POE_REG_LEN_1, &data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDeviceOperatingMode(
    const UI32_T              unit,
    const UI32_T              device,
    HAL_POE_OPERATING_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        *ptr_mode = (data == 0xFF) ? HAL_POE_OPERATING_MODE_AUTO : HAL_POE_OPERATING_MODE_MANUAL;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_value)
{
    return _hal_sk49145b_poe_readReg(unit, device, SK49145B_SCRATCH, HAL_POE_REG_LEN_1, ptr_value);
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T value)
{
    UI32_T data = value & 0xFF;

    return _hal_sk49145b_poe_writeReg(unit, device, SK49145B_SCRATCH, HAL_POE_REG_LEN_1, &data);
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDevicePowerUpMode(
    const UI32_T             unit,
    const UI32_T             device,
    AIR_POE_POWER_UP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        *ptr_mode = (0xFF == data) ? AIR_POE_POWER_UP_MODE_NORMAL : AIR_POE_POWER_UP_MODE_MANUAL;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setDevicePowerUpMode(
    const UI32_T                  unit,
    const UI32_T                  device,
    const AIR_POE_POWER_UP_MODE_T mode)
{
    UI32_T data = 0;

    HAL_CHECK_ENUM_RANGE(mode, AIR_POE_POWER_UP_MODE_LAST);

    data = (AIR_POE_POWER_UP_MODE_NORMAL == mode) ? 0xFF : 0xAA;
    return _hal_sk49145b_poe_writeReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDevicePowerStrategy(
    const UI32_T            unit,
    const UI32_T            device,
    AIR_POE_PWR_STRATEGY_T *ptr_strategy)
{
    (void)unit;
    (void)device;

    *ptr_strategy = AIR_POE_PWR_STRATEGY_PRIORITY;
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setDevicePowerStrategy(
    const UI32_T                 unit,
    const UI32_T                 device,
    const AIR_POE_PWR_STRATEGY_T strategy)
{
    (void)unit;
    (void)device;

    HAL_CHECK_ENUM_RANGE(strategy, AIR_POE_PWR_STRATEGY_LAST);
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_limit)
{
    (void)unit;
    (void)device;

    *ptr_enable = TRUE;
    *ptr_limit = 0;
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    const BOOL_T enable,
    const UI32_T limit)
{
    (void)unit;
    (void)device;
    (void)enable;
    (void)limit;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getDeviceMeasurement(
    const UI32_T                  unit,
    const UI32_T                  device,
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         port = 0;
    UI32_T         data[1] = {0};
    UI32_T         current = 0, voltage = 0;

    osal_memset(ptr_meas, 0, sizeof(AIR_POE_DEVICE_MEASUREMENT_T));

    for (port = 0; port < SK49145B_TOTAL_PORTS; port++)
    {
        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_CURRENT_BASE + (port * 4), HAL_POE_REG_LEN_2, data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        /* Byte-swap: register uses big-endian 16-bit within the i2c word */
        current = ((data[0] & 0xFF) << 8) | ((data[0] >> 8) & 0xFF);
        /* TODO: verify conversion factor (12207 uA/LSB) against SK49145B datasheet */
        ptr_meas->total_current += (current * 12207) / 1000;

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_CURRENT_BASE + (port * 4) + 2, HAL_POE_REG_LEN_2,
                                       data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        voltage = ((data[0] & 0xFF) << 8) | ((data[0] >> 8) & 0xFF);
        if (voltage > ptr_meas->supply_voltage)
        {
            /* TODO: verify conversion factor (5835/10000 V/LSB) against SK49145B datasheet */
            ptr_meas->supply_voltage = (voltage * 5835) / 10000;
        }
    }

    ptr_meas->consume_power = (ptr_meas->total_current / 100) * ptr_meas->supply_voltage / 1000;

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortPowerUp(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    BOOL_T                   *ptr_ready)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0, i = 0, device = 0, poe_port = 0, det = 0, cls = 0;

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);
        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_STATUS_BASE + poe_port, HAL_POE_REG_LEN_1, &data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        det = data & 0x7;
        cls = BITS_OFF_R(data, 4, 3);
        ptr_ready[i] = ((0x4 == det) && (0x0 != cls) && (0x7 != cls)) ? TRUE : FALSE;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_triggerPortPowerUp(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt)
{
    UI32_T device = 0, poe_port = 0, data = 0;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    data = _hal_sk49145b_poe_bit(poe_port);
    return _hal_sk49145b_poe_writeReg(unit, device, SK49145B_POWER_ENABLE_BUTTON, HAL_POE_REG_LEN_1, &data);
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortPairMode(
    const UI32_T    unit,
    const UI32_T    port,
    HAL_POE_PAIR_T *ptr_pair)
{
    (void)unit;
    (void)port;

    *ptr_pair = HAL_POE_PAIR_A;
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setPortPairMode(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_POE_PAIR_T pair)
{
    (void)unit;
    (void)port;

    HAL_CHECK_ENUM_RANGE(pair, HAL_POE_PAIR_LAST);
    return (HAL_POE_PAIR_A == pair) ? AIR_E_OK : AIR_E_NOT_SUPPORT;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_TYPE_T             *ptr_type)
{
    UI32_T device = 0, poe_port = 0, data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_HIGH_POWER_ENABLE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        *ptr_type = (data & _hal_sk49145b_poe_bit(poe_port)) ? AIR_POE_TYPE_AT : AIR_POE_TYPE_AF;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_TYPE_T        type)
{
    UI32_T device = 0, poe_port = 0, data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_ENUM_RANGE(type, AIR_POE_TYPE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_HIGH_POWER_ENABLE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        if (AIR_POE_TYPE_AT == type)
        {
            data |= _hal_sk49145b_poe_bit(poe_port);
        }
        else
        {
            data &= ~_hal_sk49145b_poe_bit(poe_port);
        }
        data &= 0xFF;
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_HIGH_POWER_ENABLE, HAL_POE_REG_LEN_1, &data);
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PSE_MODE_T         *ptr_mode)
{
    UI32_T device = 0, poe_port = 0, data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_OPERATING_MODE, HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        data = BITS_OFF_R(data, _hal_sk49145b_poe_portModeOffset(poe_port), 2);
        *ptr_mode = (SK49145B_MODE_SHUTDOWN == data) ? AIR_POE_PSE_MODE_DISABLE : AIR_POE_PSE_MODE_ENABLE;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_PSE_MODE_T    mode)
{
    UI32_T device = 0, poe_port = 0, data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_ENUM_RANGE(mode, AIR_POE_PSE_MODE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, alt, poe_port);

    if (AIR_POE_PSE_MODE_DISABLE == mode)
    {
        data = BIT(poe_port + 4);
        rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_POWER_ENABLE_BUTTON, HAL_POE_REG_LEN_1, &data);
        if (AIR_E_OK == rc)
        {
            rc = _hal_sk49145b_poe_setPortMode(unit, device, poe_port, SK49145B_MODE_SHUTDOWN);
        }
    }
    else
    {
        rc = _hal_sk49145b_poe_setPortMode(unit, device, poe_port,
                                           (AIR_POE_PSE_MODE_FORCE_POWER == mode) ? SK49145B_MODE_MANUAL :
                                                                                   SK49145B_MODE_AUTO);
        if ((AIR_E_OK == rc) && (AIR_POE_PSE_MODE_FORCE_POWER == mode))
        {
            data = BIT(poe_port);
            rc = _hal_sk49145b_poe_writeReg(unit, device, SK49145B_POWER_ENABLE_BUTTON, HAL_POE_REG_LEN_1, &data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    UI32_T                     *ptr_priority)
{
    (void)unit;
    (void)alt;

    *ptr_priority = port;
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const UI32_T                priority)
{
    (void)unit;
    (void)port;
    (void)alt;
    (void)priority;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_current)
{
    UI32_T device = 0, poe_port = 0, data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    rc = _hal_sk49145b_poe_readReg(unit, device, _hal_sk49145b_poe_portCfgReg(SK49145B_PORT_ICUT_BASE, poe_port),
                                   HAL_POE_REG_LEN_1, &data);
    if (AIR_E_OK == rc)
    {
        /* TODO: verify step size (1875/3750 uA) against SK49145B datasheet */
        *ptr_current = ((data & BIT(6)) ? 1875 : 3750) * (data & 0x3F);
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_setPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T current)
{
    UI32_T device = 0, poe_port = 0, data = 0, step = 1875;

    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);
    HAL_POE_AIR_PORT_TO_POE_PORT(unit, port, AIR_POE_ALTERNATIVE_NONE, poe_port);

    data = current / step;
    if (data > 0x3F)
    {
        data = 0x3F;
    }
    data |= BIT(7) | BIT(6);

    return _hal_sk49145b_poe_writeReg(unit, device, _hal_sk49145b_poe_portCfgReg(SK49145B_PORT_ICUT_BASE, poe_port),
                                      HAL_POE_REG_LEN_1, &data);
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortMeasurement(
    const UI32_T                unit,
    const HAL_POE_PORT_IND_T   *ptr_poe_ind,
    const UI32_T                poe_info_cnt,
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i = 0, device = 0, poe_port = 0, data = 0, raw = 0;

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_CURRENT_BASE + (poe_port * 4), HAL_POE_REG_LEN_2,
                                       &data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        raw = ((data & 0xFF) << 8) | ((data >> 8) & 0xFF);
        /* TODO: verify conversion factor (12207 uA/LSB) against SK49145B datasheet */
        ptr_meas[i].current = (raw * 12207) / 1000;

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_CURRENT_BASE + (poe_port * 4) + 2, HAL_POE_REG_LEN_2,
                                       &data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        raw = ((data & 0xFF) << 8) | ((data >> 8) & 0xFF);
        /* TODO: verify conversion factor (5835/10000 V/LSB) against SK49145B datasheet */
        ptr_meas[i].voltage = (raw * 5835) / 10000;
        ptr_meas[i].temperature = 0;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortStatus(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    AIR_POE_PORT_STATUS_T    *ptr_ps)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i = 0, device = 0, poe_port = 0, data = 0, det = 0, cls = 0, power = 0;

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_PORT_STATUS_BASE + poe_port, HAL_POE_REG_LEN_1, &data);
        if (AIR_E_OK != rc)
        {
            return rc;
        }

        det = data & 0x7;
        cls = BITS_OFF_R(data, 4, 3);
        ptr_ps[i].pd_signature = (0x4 == det) ? AIR_POE_SIGNATURE_GOOD :
                                  (0x6 == det) ? AIR_POE_SIGNATURE_OPEN : AIR_POE_SIGNATURE_BAD;
        if (0x6 == cls)
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_0;
        }
        else if ((cls >= 0x1) && (cls <= 0x4))
        {
            ptr_ps[i].pd_class = (AIR_POE_CLASS_T)cls;
        }
        else
        {
            ptr_ps[i].pd_class = AIR_POE_CLASS_LAST;
        }

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_POWER_STATUS, HAL_POE_REG_LEN_1, &power);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        ptr_ps[i].power_status = (power & _hal_sk49145b_poe_bit(poe_port)) ? AIR_POE_POWER_STATUS_ON :
                                                                              AIR_POE_POWER_STATUS_OFF;
    }

    return rc;
}

static AIR_ERROR_NO_T
hal_sk49145b_poe_getPortEvent(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    HAL_POE_POWER_EVENT_T    *ptr_event)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i = 0, device = 0, poe_port = 0;
    UI32_T         error = 0, start = 0, supply = 0;

    for (i = 0; i < poe_info_cnt; i++)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, ptr_poe_ind[i].port, ptr_poe_ind[i].alt, poe_port);

        rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_ERROR_EVENT, HAL_POE_REG_LEN_1, &error);
        rc |= _hal_sk49145b_poe_readReg(unit, device, SK49145B_START_EVENT, HAL_POE_REG_LEN_1, &start);
        rc |= _hal_sk49145b_poe_readReg(unit, device, SK49145B_SUPPLY_EVENT, HAL_POE_REG_LEN_1, &supply);
        if (AIR_E_OK != rc)
        {
            return rc;
        }

        ptr_event[i].flags = 0;
        if (error & BIT(poe_port))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_OVERLOAD;
        }
        if (error & BIT(poe_port + 4))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_DC_DISCONNECT;
        }
        if (start & BIT(poe_port))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_CURRENT_LIMIT;
        }
        if (start & BIT(poe_port + 4))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_SHORT_CIRCUIT;
        }
        if (supply & BIT(7))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_THERMAL_SHUTDOWN;
        }
        if (supply & BIT(4))
        {
            ptr_event[i].flags |= HAL_POE_POWER_EVENT_FLAGS_TRUNK_VOLTAGE_LIMIT;
        }
    }

    /* Clear-on-read for event registers */
    error = 0;
    rc = _hal_sk49145b_poe_readReg(unit, device, SK49145B_ERROR_EVENT_COR, HAL_POE_REG_LEN_1, &error);
    rc |= _hal_sk49145b_poe_readReg(unit, device, SK49145B_START_EVENT_COR, HAL_POE_REG_LEN_1, &error);
    rc |= _hal_sk49145b_poe_readReg(unit, device, SK49145B_SUPPLY_EVENT_COR, HAL_POE_REG_LEN_1, &error);

    return rc;
}

const HAL_POE_DRIVER_T _ext_sk49145b_poe_func_vec = {
    hal_sk49145b_poe_init,
    hal_sk49145b_poe_getDeviceOperatingMode,
    hal_sk49145b_poe_getDeviceScratch,
    hal_sk49145b_poe_setDeviceScratch,
    hal_sk49145b_poe_getDevicePowerUpMode,
    hal_sk49145b_poe_setDevicePowerUpMode,
    hal_sk49145b_poe_getDevicePowerStrategy,
    hal_sk49145b_poe_setDevicePowerStrategy,
    hal_sk49145b_poe_getDevicePowerLimit,
    hal_sk49145b_poe_setDevicePowerLimit,
    NULL,
    NULL,
    hal_sk49145b_poe_getDeviceMeasurement,
    hal_sk49145b_poe_getPortPowerUp,
    hal_sk49145b_poe_triggerPortPowerUp,
    hal_sk49145b_poe_getPortPairMode,
    hal_sk49145b_poe_setPortPairMode,
    hal_sk49145b_poe_getPortType,
    hal_sk49145b_poe_setPortType,
    hal_sk49145b_poe_getPortPse,
    hal_sk49145b_poe_setPortPse,
    hal_sk49145b_poe_getPortPriority,
    hal_sk49145b_poe_setPortPriority,
    hal_sk49145b_poe_getPortCurrentLimit,
    hal_sk49145b_poe_setPortCurrentLimit,
    hal_sk49145b_poe_getPortMeasurement,
    hal_sk49145b_poe_getPortStatus,
    hal_sk49145b_poe_getPortEvent,
};

AIR_ERROR_NO_T
hal_sk49145b_poe_getDriver(
    HAL_POE_DRIVER_T **pptr_poe_driver)
{
    (*pptr_poe_driver) = (HAL_POE_DRIVER_T *)&_ext_sk49145b_poe_func_vec;
    return AIR_E_OK;
}
