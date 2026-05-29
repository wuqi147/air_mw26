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
/* FILE NAME:  air_poe.c
 * PURPOSE:
 *  It provide POE module API.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <air_poe.h>

#include <api/diag.h>
#include <hal/common/hal_poe.h>
#include <osal/osal.h>

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
DIAG_SET_MODULE_INFO(AIR_MODULE_POE, "air_poe.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_poe_getDeviceInfo
 * PURPOSE:
 *      Get revision ID of the PoE device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_value                -- device revision ID
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      APP PoE requires it for a workaround.
 */
AIR_ERROR_NO_T
air_poe_getDeviceInfo(
    const UI32_T unit,
    const UI32_T device,
    UI16_T      *ptr_value)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_value);

    return hal_poe_getDeviceInfo(unit, device, ptr_value);
}

/* FUNCTION NAME:   air_poe_getDeviceScratch
 * PURPOSE:
 *      Get scratch pad value of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_value                -- scratch pad value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_value)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_value);

    return HAL_POE_FUNC_CALL(unit, device, getDeviceScratch, (unit, device, ptr_value));
}

/* FUNCTION NAME:   air_poe_setDeviceScratch
 * PURPOSE:
 *      Set scratch pad value of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 *      value                    -- scratch pad value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setDeviceScratch(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T value)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);

    return HAL_POE_FUNC_CALL(unit, device, setDeviceScratch, (unit, device, value));
}

/* FUNCTION NAME:   air_poe_getDevicePowerUpMode
 * PURPOSE:
 *      Get power up mode of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_mode                 -- power up mode
 *                                  AIR_POE_POWER_UP_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getDevicePowerUpMode(
    const UI32_T             unit,
    const UI32_T             device,
    AIR_POE_POWER_UP_MODE_T *ptr_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_mode);

    return HAL_POE_FUNC_CALL(unit, device, getDevicePowerUpMode, (unit, device, ptr_mode));
}

/* FUNCTION NAME:   air_poe_setDevicePowerUpMode
 * PURPOSE:
 *      Set power up mode of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 *      mode                     -- power up mode
 *                                  AIR_POE_POWER_UP_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setDevicePowerUpMode(
    const UI32_T                  unit,
    const UI32_T                  device,
    const AIR_POE_POWER_UP_MODE_T mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);

    return HAL_POE_FUNC_CALL(unit, device, setDevicePowerUpMode, (unit, device, mode));
}

/* FUNCTION NAME:   air_poe_getDevicePowerStrategy
 * PURPOSE:
 *      Get power strategy of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_strategy             -- power strategy
 *                                  AIR_POE_PWR_STRATEGY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getDevicePowerStrategy(
    const UI32_T            unit,
    const UI32_T            device,
    AIR_POE_PWR_STRATEGY_T *ptr_strategy)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_strategy);

    return HAL_POE_FUNC_CALL(unit, device, getDevicePowerStrategy, (unit, device, ptr_strategy));
}

/* FUNCTION NAME:   air_poe_setDevicePowerStrategy
 * PURPOSE:
 *      Set power strategy of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 *      strategy                 -- power strategy
 *                                  AIR_POE_PWR_STRATEGY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setDevicePowerStrategy(
    const UI32_T                 unit,
    const UI32_T                 device,
    const AIR_POE_PWR_STRATEGY_T strategy)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);

    return HAL_POE_FUNC_CALL(unit, device, setDevicePowerStrategy, (unit, device, strategy));
}

/* FUNCTION NAME:   air_poe_getDevicePowerLimit
 * PURPOSE:
 *      Get power limit of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_enable               -- limit enable
 *      ptr_limit                -- limit value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_limit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_enable);
    HAL_CHECK_PTR(ptr_limit);

    return HAL_POE_FUNC_CALL(unit, device, getDevicePowerLimit, (unit, device, ptr_enable, ptr_limit));
}

/* FUNCTION NAME:   air_poe_setDevicePowerLimit
 * PURPOSE:
 *      Set power limit of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 *      enable                   -- limit enable
 *      limit                    -- limit value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setDevicePowerLimit(
    const UI32_T unit,
    const UI32_T device,
    const BOOL_T enable,
    const UI32_T limit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);

    return HAL_POE_FUNC_CALL(unit, device, setDevicePowerLimit, (unit, device, enable, limit));
}

/* FUNCTION NAME:   air_poe_getDeviceMeasurement
 * PURPOSE:
 *      Get measurement of the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      device                   -- device id
 * OUTPUT:
 *      ptr_meas                 -- measurement data
 *                                  AIR_POE_DEVICE_MEASUREMENT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getDeviceMeasurement(
    const UI32_T                  unit,
    const UI32_T                  device,
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_UNIT(unit, device);
    HAL_CHECK_PTR(ptr_meas);

    return HAL_POE_FUNC_CALL(unit, device, getDeviceMeasurement, (unit, device, ptr_meas));
}

/* FUNCTION NAME:   air_poe_getPortPowerUp
 * PURPOSE:
 *      Get state machine of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_ready                -- state suspended before entering
 *                                  power up
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortPowerUp(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    BOOL_T                     *ptr_ready)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);

    return hal_poe_getPortPowerUp(unit, port, alt, ptr_ready);
}

/* FUNCTION NAME:   air_poe_triggerPortPowerUp
 * PURPOSE:
 *      Trigger power up of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_triggerPortPowerUp(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, triggerPortPowerUp, (unit, port, alt));
}

/* FUNCTION NAME:   air_poe_getPortType
 * PURPOSE:
 *      Get power supply standard of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_type                 -- power supply standard
 *                                  AIR_POE_TYPE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_TYPE_T             *ptr_type)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_type);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, getPortType, (unit, port, alt, ptr_type));
}

/* FUNCTION NAME:   air_poe_setPortType
 * PURPOSE:
 *      Set power supply standard of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 *      type                     -- power supply standard
 *                                  AIR_POE_TYPE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setPortType(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_TYPE_T        type)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, setPortType, (unit, port, alt, type));
}

/* FUNCTION NAME:   air_poe_getPortPse
 * PURPOSE:
 *      Get PSE functionality of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_mode                 -- PSE functionality mode
 *                                  AIR_POE_PSE_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PSE_MODE_T         *ptr_mode)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_mode);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, getPortPse, (unit, port, alt, ptr_mode));
}

/* FUNCTION NAME:   air_poe_setPortPse
 * PURPOSE:
 *      Set PSE functionality of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 *      mode                     -- PSE functionality mode
 *                                  AIR_POE_PSE_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_PSE_MODE_T    mode)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, setPortPse, (unit, port, alt, mode));
}

/* FUNCTION NAME:   air_poe_getPortPriority
 * PURPOSE:
 *      Get priority of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_priority             -- priority
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    UI32_T                     *ptr_priority)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_priority);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, getPortPriority, (unit, port, alt, ptr_priority));
}

/* FUNCTION NAME:   air_poe_setPortPriority
 * PURPOSE:
 *      Set priority of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 *      priority                 -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setPortPriority(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const UI32_T                priority)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, alt, device);

    return HAL_POE_FUNC_CALL(unit, device, setPortPriority, (unit, port, alt, priority));
}

/* FUNCTION NAME:   air_poe_getPortCurrentLimit
 * PURPOSE:
 *      Get current limit of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_current              -- current limit
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_current)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_PTR(ptr_current);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);

    return HAL_POE_FUNC_CALL(unit, device, getPortCurrentLimit, (unit, port, ptr_current));
}

/* FUNCTION NAME:   air_poe_setPortCurrentLimit
 * PURPOSE:
 *      Set current limit of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      current                  -- current limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_setPortCurrentLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T current)
{
    UI8_T device = 0;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_POE_AIR_PORT_TO_DEV_IDX(unit, port, AIR_POE_ALTERNATIVE_NONE, device);

    return HAL_POE_FUNC_CALL(unit, device, setPortCurrentLimit, (unit, port, current));
}

/* FUNCTION NAME:   air_poe_getPortMeasurement
 * PURPOSE:
 *      Get measurement of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_meas                 -- measurement data
 *                                  AIR_POE_PORT_MEASUREMENT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortMeasurement(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_meas);

    return hal_poe_getPortMeasurement(unit, port, alt, ptr_meas);
}

/* FUNCTION NAME:   air_poe_getPortStatus
 * PURPOSE:
 *      Get status of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_ps                   -- status
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortStatus(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PORT_STATUS_T      *ptr_ps)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_ps);

    return hal_poe_getPortStatus(unit, port, alt, ptr_ps);
}

/* FUNCTION NAME:   air_poe_getPortEvent
 * PURPOSE:
 *      Get event of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      ptr_event                -- event flags
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_getPortEvent(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_EVENT_T            *ptr_event)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);
    HAL_CHECK_PTR(ptr_event);

    return hal_poe_getPortEvent(unit, port, alt, ptr_event);
}

/* FUNCTION NAME:   air_poe_clearPortEvent
 * PURPOSE:
 *      Clear event records of the specified port.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      alt                      -- role of the alternative
 *                                  AIR_POE_ALTERNATIVE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_clearPortEvent(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_POE_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(alt, AIR_POE_ALTERNATIVE_LAST);

    return hal_poe_clearPortEvent(unit, port, alt);
}

/* FUNCTION NAME:   air_poe_init
 * PURPOSE:
 *      This API is used to initialize POE module and thread.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_NOT_INITED         -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);

    rc = hal_poe_initModule(unit);

    if (AIR_E_OK == rc)
    {
        rc = hal_poe_initThread(unit);
    }

    return rc;
}

/* FUNCTION NAME:   air_poe_deinit
 * PURPOSE:
 *      This API is used to deinitialize POE module and thread.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_poe_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);

    rc = hal_poe_deinitThread(unit);

    if (AIR_E_OK == rc)
    {
        rc = hal_poe_deinitModule(unit);
    }

    return rc;
}
