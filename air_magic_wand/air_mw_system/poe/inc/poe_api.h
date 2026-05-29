/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  poe_main.h
 * PURPOSE:
 *      Wrapper APIs for PoE application.
 * NOTES:
 */

#ifndef POE_API_H
#define POE_API_H

/* INCLUDE FILE DECLARATIONS
 */
#include <mw_error.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    POE_PWR_CONTROL_HW = 0,
    POE_PWR_CONTROL_SW,
    POE_PWR_CONTROL_LAST
} POE_PWR_CONTROL_T;

typedef enum
{
    POE_PORT_PSE_MODE_DISABLE = 0,
    POE_PORT_PSE_MODE_ENABLE,
    POE_PORT_PSE_MODE_LAST
} POE_PORT_PSE_MODE_T;

typedef enum
{
    POE_PORT_TYPE_AF = 0,
    POE_PORT_TYPE_AT,
    POE_PORT_TYPE_BT_TYPE3,
    POE_PORT_TYPE_BT_TYPE4,
    POE_PORT_TYPE_LAST
} POE_PORT_TYPE_T;

typedef enum
{
    POE_PORT_PRIORITY_CRITICAL = 0,
    POE_PORT_PRIORITY_HIGH,
    POE_PORT_PRIORITY_LOW,
    POE_PORT_PRIORITY_LAST
} POE_PORT_PRIORITY_T;

typedef enum
{
    POE_SYS_POWER_MODE_PRIORITY = 0, /* power-off with the lowest priority */
    POE_SYS_POWER_MODE_ORDER,        /* power-off with last come */
    POE_SYS_POWER_MODE_LAST
} POE_SYS_POWER_MODE_T;

typedef enum
{
    POE_PORT_ALT_NONE = 0,
    POE_PORT_ALT_PRIMARY,
    POE_PORT_ALT_SECONDARY,
    POE_PORT_ALT_LAST
} POE_PORT_ALT_T;

typedef enum
{
    POE_PORT_SIGNATURE_BAD = 0,
    POE_PORT_SIGNATURE_GOOD,
    POE_PORT_SIGNATURE_OPEN,
    POE_PORT_SIGNATURE_LARGE,
    POE_PORT_SIGNATURE_LOW,
    POE_PORT_SIGNATURE_HIGH,
    POE_PORT_SIGNATURE_LAST
} POE_PORT_SIGNATURE_T;

typedef enum
{
    POE_PORT_CLASS_0 = 0,
    POE_PORT_CLASS_1,
    POE_PORT_CLASS_2,
    POE_PORT_CLASS_3,
    POE_PORT_CLASS_4,
    POE_PORT_CLASS_LAST
} POE_PORT_CLASS_T;

typedef enum
{
    POE_PORT_POWER_STATUS_OFF = 0,
    POE_PORT_POWER_STATUS_ON,
    POE_PORT_POWER_STATUS_LAST
} POE_PORT_POWER_STATUS_T;

typedef struct POE_PORT_CONFIG_S
{
    POE_PORT_PSE_MODE_T mode;
    POE_PORT_TYPE_T     type;
    POE_PORT_PRIORITY_T priority;
    UI32_T              power_limit;
} POE_PORT_CONFIG_T;

typedef struct POE_PORT_OP_STATUS_S
{
    POE_PORT_SIGNATURE_T    pd_signature;
    POE_PORT_CLASS_T        pd_class;
    POE_PORT_POWER_STATUS_T power_status;
    POE_PORT_PSE_MODE_T     pse_mode;

#define POE_EVENT_FLAGS_OVERLOAD      (1U << 0)          /* Power overload detected (beyond Icut threshold) */
#define POE_EVENT_FLAGS_SHORT_CIRCUIT (1U << 1)          /* Short-circuit condition detected */
#define POE_EVENT_FLAGS_MPS_ERROR     (1U << 2)          /* MPS(Maintain Power Signature) lost */
#define POE_EVENT_FLAGS_VOLTAGE_BAD   (1U << 3)          /* Triggered when PSE output voltage drops abnormally.   \
                                                          * For instance, a drop below 30V. Commonly occurs       \
                                                          * along with short circuit or overcurrent events,       \
                                                          * as the output voltage falls due to excessive current. \
                                                          */
#define POE_EVENT_FLAGS_THERMAL_SHUTDOWN       (1U << 4) /* Device has shut down due to high temperature */
#define POE_EVENT_FLAGS_OVER_CURRENT_LIMIT     (1U << 5) /* Current has exceeded the allowed limit */
#define POE_EVENT_FLAGS_OVER_VOLTAGE_LIMIT     (1U << 6) /* Voltage has exceeded the allowed limit */
#define POE_EVENT_FLAGS_OVER_TEMPERATURE_LIMIT (1U << 7) /* Over-temperature condition detected */
#define POE_EVENT_FLAGS_DEVICE_RESET           (1U << 8) /* Device has been reset */
#define POE_EVENT_FLAGS_INSUFFICIENT_POWER     (1U << 9) /* Insufficient power  */
    UI32_T event_flags;
} POE_PORT_OP_STATUS_T;

typedef struct POE_PORT_MEAS_S
{
    UI32_T voltage;
    UI32_T current;
    UI32_T temperature;
    UI32_T power;
} POE_PORT_MEAS_T;

typedef struct POE_SYS_CONFIG_S
{
    POE_SYS_POWER_MODE_T power_mode;
    UI32_T               power_limit;
    UI32_T               max_shutdown_volt;
    UI32_T               min_shutdown_volt;
} POE_SYS_CONFIG_T;

typedef struct POE_SYS_SUPPLY_S
{
    UI32_T allocated_power;
    UI32_T available_power;
    UI32_T supply_voltage;
} POE_SYS_SUPPLY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   poe_setPortPseControl
 * PURPOSE:
 *      Set PSE functionality of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      enable                  -- TRUE: enable pse control
 *                                 FALSE:disable pse control
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   poe_getPortPseControl
 * PURPOSE:
 *      Get PSE functionality of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_enable              -- TRUE: enable pse control
 *                                 FALSE:disable pse control
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   poe_setPortType
 * PURPOSE:
 *      Set operation mode of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      type                    -- poe type
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setPortType(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_TYPE_T type);

/* FUNCTION NAME:   poe_getPortType
 * PURPOSE:
 *      Get operation mode of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_type                -- poe type
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortType(
    const UI32_T     unit,
    const UI32_T     port,
    POE_PORT_TYPE_T *ptr_type);

/* FUNCTION NAME:   poe_setPortPowerLimit
 * PURPOSE:
 *      Set power limit of the specified port. If the port's power consumption
 *      exceeds this level, the port will be disconnected.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      power_limit             -- power limit
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T power_limit);

/* FUNCTION NAME:   poe_getPortPowerLimit
 * PURPOSE:
 *      Get power limit of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_power_limit         -- power limit
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_power_limit);

/* FUNCTION NAME:   poe_setPortPriority
 * PURPOSE:
 *      Set priority of the specified port. It is used when the system
 *      power_mode is set to POWER_MODE_PRIORITY.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      priority                -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setPortPriority(
    const UI32_T              unit,
    const UI32_T              port,
    const POE_PORT_PRIORITY_T priority);

/* FUNCTION NAME:   poe_getPortPriority
 * PURPOSE:
 *      Get priority of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_priority            -- priority
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortPriority(
    const UI32_T         unit,
    const UI32_T         port,
    POE_PORT_PRIORITY_T *ptr_priority);

/* FUNCTION NAME:   poe_setSystemPowerMode
 * PURPOSE:
 *      Set the power-on strategy of the system.
 * INPUT:
 *      power_mode              -- power mode
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_NOT_SUPPORT        -- Not supported.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setSystemPowerMode(
    const POE_SYS_POWER_MODE_T power_mode);

/* FUNCTION NAME:   poe_getSystemPowerMode
 * PURPOSE:
 *      Get the power-on strategy of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_power_mode          -- power mode
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getSystemPowerMode(
    POE_SYS_POWER_MODE_T *ptr_power_mode);

/* FUNCTION NAME:   poe_setSystemPowerBudget
 * PURPOSE:
 *      Set the power budget of the system.
 * INPUT:
 *      power_limit             -- power limit
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_setSystemPowerBudget(
    const UI32_T power_limit);

/* FUNCTION NAME:   poe_getSystemPowerBudget
 * PURPOSE:
 *      Set the power budget of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_power_limit         -- power limit
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getSystemPowerBudget(
    UI32_T *ptr_power_limit);

/* FUNCTION NAME:   poe_getPortStatus
 * PURPOSE:
 *      Get the status of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- port alternative
 * OUTPUT:
 *      ptr_status              -- port status
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortStatus(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_ALT_T  alt,
    POE_PORT_OP_STATUS_T *ptr_status);

/* FUNCTION NAME:   poe_getPortMeasurement
 * PURPOSE:
 *      Get the measurement of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- port alternative
 * OUTPUT:
 *      ptr_meas                -- port measurement
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getPortMeasurement(
    const UI32_T         unit,
    const UI32_T         port,
    const POE_PORT_ALT_T alt,
    POE_PORT_MEAS_T     *ptr_meas);

/* FUNCTION NAME:   poe_getSystemPowerSupply
 * PURPOSE:
 *      Get the power supply of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_supply              -- port supply
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 *      MW_E_OP_INVALID         -- Operation is invalid.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getSystemPowerSupply(
    POE_SYS_SUPPLY_T *ptr_supply);

/* FUNCTION NAME:   poe_getSystemPowerStrategy
 * PURPOSE:
 *      Get the power strategy control of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_strategy            -- port strategy
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_getSystemPowerStrategy(
    POE_PWR_CONTROL_T *ptr_strategy);

/* FUNCTION NAME:   poe_clearPortEvent
 * PURPOSE:
 *      Clear the PoE events of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_clearPortEvent(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   poe_getPoeMaxPower
 * PURPOSE:
 *      get poe max power budget value.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T                  -- max power budget
 * NOTES:
 *      None
 */
UI32_T
poe_getPoeMaxPower(
    void);

/* FUNCTION NAME:   poe_checkPseStatus
 * PURPOSE:
 *      check that PoE is working normally.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BOOL_T                  -- TRUE: PSE working normally.
 *                                 FALSE:PSE working abnormally.
 * NOTES:
 *      None
 */
BOOL_T
poe_checkPseStatus(
    void);

#endif /* POE_API_H */
