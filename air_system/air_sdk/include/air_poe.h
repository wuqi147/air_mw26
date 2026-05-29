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
/* FILE NAME:  air_poe.h
 * PURPOSE:
 *      Define PoE driver function
 *
 * NOTES:
 *
 */
#ifndef AIR_POE_H
#define AIR_POE_H
/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_POE_MAXIMUM_PORTS_PER_DEVICE (8)

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_POE_POWER_UP_MODE_NORMAL = 0,
    AIR_POE_POWER_UP_MODE_MANUAL,
    AIR_POE_POWER_UP_MODE_LAST,
} AIR_POE_POWER_UP_MODE_T;

typedef enum
{
    AIR_POE_TYPE_AF = 0,
    AIR_POE_TYPE_AT,
    AIR_POE_TYPE_LAST,
} AIR_POE_TYPE_T;

typedef enum
{
    AIR_POE_PSE_MODE_DISABLE = 0,
    AIR_POE_PSE_MODE_ENABLE,
    AIR_POE_PSE_MODE_FORCE_POWER,
    AIR_POE_PSE_MODE_LAST,
} AIR_POE_PSE_MODE_T;

typedef enum
{
    AIR_POE_SIGNATURE_BAD = 0,
    AIR_POE_SIGNATURE_GOOD,
    AIR_POE_SIGNATURE_OPEN,
    AIR_POE_SIGNATURE_LARGE,
    AIR_POE_SIGNATURE_LOW,
    AIR_POE_SIGNATURE_HIGH,
    AIR_POE_SIGNATURE_LAST,
} AIR_POE_SIGNATURE_T;

typedef enum
{
    AIR_POE_CLASS_0 = 0,
    AIR_POE_CLASS_1,
    AIR_POE_CLASS_2,
    AIR_POE_CLASS_3,
    AIR_POE_CLASS_4,
    AIR_POE_CLASS_LAST,
} AIR_POE_CLASS_T;

typedef enum
{
    AIR_POE_POWER_STATUS_OFF = 0,
    AIR_POE_POWER_STATUS_ON,
    AIR_POE_POWER_STATUS_LAST,
} AIR_POE_POWER_STATUS_T;

typedef enum
{
    AIR_POE_ALTERNATIVE_NONE = 0,
    AIR_POE_ALTERNATIVE_PRIMARY,
    AIR_POE_ALTERNATIVE_SECONDARY,
    AIR_POE_ALTERNATIVE_LAST,
} AIR_POE_ALTERNATIVE_T;

typedef enum
{
    AIR_POE_PWR_STRATEGY_PLUG,     /* pick victim: last port powered up */
    AIR_POE_PWR_STRATEGY_PRIORITY, /* pick victim: port with lowest priority */
    AIR_POE_PWR_STRATEGY_LAST
} AIR_POE_PWR_STRATEGY_T;

typedef struct AIR_POE_PORT_STATUS_S
{
    AIR_POE_SIGNATURE_T    pd_signature;
    AIR_POE_CLASS_T        pd_class;
    AIR_POE_POWER_STATUS_T power_status;
} AIR_POE_PORT_STATUS_T;

typedef struct AIR_POE_PORT_MEASUREMENT_S
{
    UI32_T current;     /* gain factor is 0.01 mA */
    UI32_T voltage;     /* gain factor is 0.01 volts */
    UI32_T temperature; /* gain factor is 0.01 celsius */
} AIR_POE_PORT_MEASUREMENT_T;

typedef struct AIR_POE_DEVICE_MEASUREMENT_S
{
    UI32_T total_current;  /* gain factor is 0.01 mA */
    UI32_T supply_voltage; /* gain factor is 0.01 volts */
    UI32_T consume_power;  /* gain factor is 0.01 watt */
} AIR_POE_DEVICE_MEASUREMENT_T;

typedef struct AIR_POE_EVENT_S
{
/* Power overload detected (beyond Icut threshold) */
#define AIR_POE_EVENT_FLAGS_OVERLOAD (1U << 0)

/* Short-circuit condition detected */
#define AIR_POE_EVENT_FLAGS_SHORT_CIRCUIT (1U << 1)

/* MPS(Maintain Power Signature) lost */
#define AIR_POE_EVENT_FLAGS_MPS_ERROR (1U << 2)

/* Triggered when PSE output voltage drops abnormally.
 * For instance, a drop below 30V. Commonly occurs
 * along with short circuit or overcurrent events,
 * as the output voltage falls due to excessive current.
 */
#define AIR_POE_EVENT_FLAGS_VOLTAGE_BAD (1U << 3)

/* Device has shut down due to high temperature */
#define AIR_POE_EVENT_FLAGS_THERMAL_SHUTDOWN (1U << 4)

/* Current has exceeded the allowed limit */
#define AIR_POE_EVENT_FLAGS_OVER_CURRENT_LIMIT (1U << 5)

/* Voltage has exceeded the allowed limit */
#define AIR_POE_EVENT_FLAGS_OVER_VOLTAGE_LIMIT (1U << 6)

/* Over-temperature condition detected */
#define AIR_POE_EVENT_FLAGS_OVER_TEMPERATURE_LIMIT (1U << 7)
    UI32_T event_flags;
} AIR_POE_EVENT_T;

typedef enum
{
    AIR_POE_NOTIFY_CATEGORY_POWER = 0,
    AIR_POE_NOTIFY_CATEGORY_EVENT,
    AIR_POE_NOTIFY_CATEGORY_RESTART,
    AIR_POE_NOTIFY_CATEGORY_LAST,
} AIR_POE_NOTIFY_CATEGORY_T;

typedef enum
{
    AIR_POE_PORT_POWER_OFF = 0,
    AIR_POE_PORT_POWER_ON,
    AIR_POE_PORT_POWER_SUSPEND,
    AIR_POE_PORT_POWER_LAST,
} AIR_POE_PORT_POWER_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    UI16_T      *ptr_value);

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
    UI32_T      *ptr_value);

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
    const UI32_T value);

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
    AIR_POE_POWER_UP_MODE_T *ptr_mode);

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
    const AIR_POE_POWER_UP_MODE_T mode);

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
    AIR_POE_PWR_STRATEGY_T *ptr_strategy);

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
    const AIR_POE_PWR_STRATEGY_T strategy);

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
    UI32_T      *ptr_limit);

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
    const UI32_T limit);

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
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas);

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
    BOOL_T                     *ptr_ready);

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
    const AIR_POE_ALTERNATIVE_T alt);

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
    AIR_POE_TYPE_T             *ptr_type);

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
    const AIR_POE_TYPE_T        type);

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
    AIR_POE_PSE_MODE_T         *ptr_mode);

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
    const AIR_POE_PSE_MODE_T    mode);

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
    UI32_T                     *ptr_priority);

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
    const UI32_T                priority);

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
    UI32_T      *ptr_current);

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
    const UI32_T current);

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
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas);

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
    AIR_POE_PORT_STATUS_T      *ptr_ps);

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
    AIR_POE_EVENT_T            *ptr_event);

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
    const AIR_POE_ALTERNATIVE_T alt);

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
    const UI32_T unit);

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
    const UI32_T unit);

#endif /* End of AIR_POE_H */
