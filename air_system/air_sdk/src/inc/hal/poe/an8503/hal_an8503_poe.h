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
/* FILE NAME:  hal_an8503_poe.h
 * PURPOSE:
 *      Define AN8503 driver function
 *
 * NOTES:
 *
 */
#ifndef HAL_AN8503_POE_H
#define HAL_AN8503_POE_H
/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <hal/common/hal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AN8503_MAX_PORT_CURR_INT        (0x3ff)
#define AN8503_MAX_PORT_CURR_FRAC       (0x3)
#define AN8503_DEFAULT_SCRATCH_VALUE    (0xA5)
#define AN8503_MAX_PSE_AVAI_CURRENT_INT (0x1fff)
#define AN8503_MIN_ERROR_DELAY          (0x9)

/* Page 0x0 */
#define AN8503_AF_AT_MODE                   (0x25)
#define AN8503_DETECTED_SIGNATURE           (0x68)
#define AN8503_DETECTED_PD_CLASS            (0x88)
#define AN8503_PD_REQUESTED_POWER_BASE      (0x90)
#define AN8503_PORT_CURRENT                 (0xA0)
#define AN8503_PORT_VOLTAGE                 (0xB0)
#define AN8503_PORT_TEMPERATURE             (0xC0)
#define AN8503_SUPPLY_VOLTAGE               (0xE0)
#define AN8503_IVT_POLL                     (0xE3)
#define AN8503_ERROR_DELAY                  (0xE4)
#define AN8503_PD_REQUESTED_POWER(__port__) ((AN8503_PD_REQUESTED_POWER_BASE) + (__port__))

/* Page 0x1 */
#define AN8503_SYSTEM_CONFIGURATION          (0x01)
#define AN8503_SCRATCH_REGISTER              (0x06)
#define AN8503_ALTERNATIVE_A_B               (0x07)
#define AN8503_LED_CONFIGURATION             (0x08)
#define AN8503_PORT_CURR_LIMIT_BASE          (0x30)
#define AN8503_PSE_AVAI_CURR_LIMIT_MSB       (0x54)
#define AN8503_PSE_AVAI_CURR_LIMIT_LSB       (0x55)
#define AN8503_PSE_CONSUMED_CURRENT          (0x56)
#define AN8503_PORT_PRIORITY_BASE            (0x58)
#define AN8503_PORT_POWER_EVENT_BASE         (0x70)
#define AN8503_POWER_EVENT_HANDLER           (0x81)
#define AN8503_PORT_POWER_STATUS             (0x82)
#define AN8503_PORT_STATE_MACHINE_BASE       (0x90)
#define AN8503_PORT_PWR_CTRL_BASE            (0x98)
#define AN8503_CURRENT_LIMIT_CTRL            (0xC0)
#define AN8503_PORT_CURR_LIMIT_MSB(__port__) ((AN8503_PORT_CURR_LIMIT_BASE) + (__port__) * (2))
#define AN8503_PORT_CURR_LIMIT_LSB(__port__) ((AN8503_PORT_CURR_LIMIT_BASE) + (__port__) * (2) + 1)
#define AN8503_PORT_PWR_CTRL(__port__)       ((AN8503_PORT_PWR_CTRL_BASE) + (__port__))
#define AN8503_PORT_STATE_MACHINE(__port__)  ((AN8503_PORT_STATE_MACHINE_BASE) + (__port__))
#define AN8503_PORT_POWER_EVENT(__port__)    ((AN8503_PORT_POWER_EVENT_BASE) + (__port__))
#define AN8503_PORT_PRIORITY(__port__)       ((AN8503_PORT_PRIORITY_BASE) + (__port__))

#define AN8503_TOTAL_PORTS               (8)
#define AN8503_OPERATION_MODE_OFFSET     (6)
#define AN8503_OPERATION_MODE_RELMASK    (0x00000003)
#define AN8503_OPERATION_MODE_MASK       (AN8503_OPERATION_MODE_RELMASK << AN8503_OPERATION_MODE_OFFSET)
#define AN8503_SUSPEND_PWRUP_OFFSET      (1)
#define AN8503_SUSPEND_PWRUP_RELMASK     (0x00000001)
#define AN8503_SUSPEND_PWRUP_MASK        (AN8503_SUSPEND_PWRUP_RELMASK << AN8503_SUSPEND_PWRUP_OFFSET)
#define AN8503_PWRUP_SUSPENDED_OFFSET    (7)
#define AN8503_PWRUP_SUSPENDED_RELMASK   (0x00000001)
#define AN8503_PWRUP_SUSPENDED_MASK      (AN8503_PWRUP_SUSPENDED_RELMASK << AN8503_PWRUP_SUSPENDED_OFFSET)
#define AN8503_START_PWRUP_OFFSET        (5)
#define AN8503_START_PWRUP_RELMASK       (0x00000001)
#define AN8503_START_PWRUP_MASK          (AN8503_START_PWRUP_RELMASK << AN8503_START_PWRUP_OFFSET)
#define AN8503_PSE_PORT_STATUS_OFFSET    (0)
#define AN8503_PSE_PORT_STATUS_RELMASK   (0x00000003)
#define AN8503_PSE_PORT_STATUS_MASK      (AN8503_PSE_PORT_STATUS_RELMASK << AN8503_PSE_PORT_STATUS_OFFSET)
#define AN8503_PORT_PWR_CTRL_PSE_OFFT    (0)
#define AN8503_PORT_PWR_CTRL_PSE_RELMASK (0x00000003)
#define AN8503_PORT_PWR_CTRL_PSE_MASK    (AN8503_PORT_PWR_CTRL_PSE_RELMASK << AN8503_PORT_PWR_CTRL_PSE_OFFT)
#define AN8503_VOLT_BAD_EVENT_OFFSET     (3)
#define AN8503_VOLT_BAD_EVENT_RELMASK    (0x00000001)
#define AN8503_VOLT_BAD_EVENT_MASK       (AN8503_VOLT_BAD_EVENT_RELMASK << AN8503_VOLT_BAD_EVENT_OFFSET)
#define AN8503_CURR_LIMIT_EVENT_OFFSET   (5)
#define AN8503_CURR_LIMIT_EVENT_RELMASK  (0x00000001)
#define AN8503_CURR_LIMIT_EVENT_MASK     (AN8503_CURR_LIMIT_EVENT_RELMASK << AN8503_CURR_LIMIT_EVENT_OFFSET)
#define AN8503_AUTO_POLL_OFFSET          (5)
#define AN8503_AUTO_POLL_RELMASK         (0x00000001)
#define AN8503_AUTO_POLL_MASK            (AN8503_AUTO_POLL_RELMASK << AN8503_AUTO_POLL_OFFSET)
#define AN8503_VICTIM_STRATEGY_OFFSET    (0)
#define AN8503_VICTIM_STRATEGY_RELMASK   (0x00000007)
#define AN8503_VICTIM_STRATEGY_MASK      (AN8503_VICTIM_STRATEGY_RELMASK << AN8503_VICTIM_STRATEGY_OFFSET)
#define AN8503_PORT_PWR_CTRL_PSE_DISABLE (0x0)
#define AN8503_PORT_PWR_CTRL_PSE_ENABLE  (0x1)
#define AN8503_PORT_PWR_CTRL_PSE_FORCE   (0x2)
#define AN8503_SIGNATURE_OFFSET          (0)
#define AN8503_SIGNATURE_RELMASK         (0x00000003)
#define AN8503_SIGNATURE_MASK            (AN8503_SIGNATURE_RELMASK << AN8503_SIGNATURE_OFFSET)
#define AN8503_PSE_CURR_LIMITER_OFFSET   (7)
#define AN8503_PSE_CURR_LIMITER_RELMASK  (0x00000001)
#define AN8503_PSE_CURR_LIMITER_MASK     (AN8503_PSE_CURR_LIMITER_RELMASK << AN8503_PSE_CURR_LIMITER_OFFSET)
#define AN8503_SIGNATURE_BAD             (0x00)
#define AN8503_SIGNATURE_GOOD            (0x01)
#define AN8503_SIGNATURE_OPEN            (0x02)
#define AN8503_PD_CLASS_MASK             (0x00000007)
#define AN8503_PD_CLASS_PORT_SHIFT       (4)
#define AN8503_CLASS_0                   (0x00)
#define AN8503_CLASS_1                   (0x01)
#define AN8503_CLASS_2                   (0x02)
#define AN8503_CLASS_3                   (0x03)
#define AN8503_CLASS_4                   (0x04)
#define AN8503_PORT_POWER_LEN            (1)
#define AN8503_PORT_POWER_OFF            (0)
#define AN8503_PORT_POWER_ON             (1)
#define AN8503_OVERLOAD_EVENT            (0x01)
#define AN8503_SHORT_CIRCUIT_LIMIT_EVENT (0x02)
#define AN8503_MPS_ERROR_EVENT           (0x04)
#define AN8503_VOLT_BAD_EVENT            (0x08)
#define AN8503_THERMAL_SHUTDOWN_EVENT    (0x10)
#define AN8503_CURR_LIMIT_EVENT          (0x20)
#define AN8503_VOLT_LIMIT_EVENT          (0x40)
#define AN8503_TEMPERATURE_LIMIT_EVENT   (0x80)
#define AN8503_ALT_A_B_LEN               (1)
#define AN8503_PAIR_A                    (0)
#define AN8503_PAIR_B                    (1)
#define AN8503_SUSPEND_PWRUP_DIS         (0x00)
#define AN8503_SUSPEND_PWRUP_EN          (0x01)
#define AN8503_AUTO_MODE                 (0x00)
#define AN8503_MANUL_MODE                (0x01)
#define AN8503_AF_AT_MODE_LEN            (1)
#define AN8503_AF_MODE                   (0)
#define AN8503_AT_MODE                   (1)
#define AN8503_LAST_PORT_POWERED_UP      (0x0)
#define AN8503_PORT_WITH_LOWEST_PRIORITY (0x4)
#define AN8503_PSE_CURR_LIMITER_EN       (1)
#define AN8503_PSE_CURR_LIMITER_LEN      (1)
#define AN8503_VICTIM_STRATEGY_LEN       (3)

#define AN8503_PSE_CONSUMED_CURR_FRAC_BITS (2)
#define AN8503_PSE_CONSUMED_CURR_FRAC_MASK (0x00000003)
#define AN8503_SUPPLY_VOLT_FRAC_BITS       (4)
#define AN8503_SUPPLY_VOLT_FRAC_MASK       (0x0000000F)
#define AN8503_PORT_CURR_LIMIT_FRAC_BITS   (2)
#define AN8503_PORT_CURR_LIMIT_FRAC_MASK   (0x00000003)
#define AN8503_PORT_CURR_FRAC_BITS         (2)
#define AN8503_PORT_CURR_FRAC_MASK         (0x00000003)
#define AN8503_PORT_VOLT_FRAC_BITS         (4)
#define AN8503_PORT_VOLT_FRAC_MASK         (0x0000000F)
#define AN8503_PORT_TEMPERATURE_FRAC_BITS  (4)
#define AN8503_PORT_TEMPERATURE_FRAC_MASK  (0x0000000F)
#define AN8503_PORT_POWER_EVENT_MASK       (0x000000FF)
#define AN8503_PSE_AVAI_CURRENT_FRAC_BITS  (2)
#define AN8503_PSE_AVAI_CURRENT_FRAC_MASK  (0x00000003)

#define AN8503_LED_ACTIVE_LEVEL_OFFSET   (3)
#define AN8503_LED_ACTIVE_LEVEL_RELMASK  (0x00000001)
#define AN8503_LED_ACTIVE_LEVEL_MASK     (AN8503_LED_ACTIVE_LEVEL_RELMASK << AN8503_LED_ACTIVE_LEVEL_OFFSET)
#define AN8503_LED_BHV_ACTIVE_LEVEL_MASK (0x0001)

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    HAL_POE_OPERATING_MODE_T *ptr_mode);

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
    UI32_T      *ptr_value);

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
    const UI32_T value);

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
    AIR_POE_POWER_UP_MODE_T *ptr_mode);

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
    const AIR_POE_POWER_UP_MODE_T mode);

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
    AIR_POE_PWR_STRATEGY_T *ptr_strategy);

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
    const AIR_POE_PWR_STRATEGY_T strategy);

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
    UI32_T      *ptr_limit);

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
    const UI32_T limit);

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
    UI32_T      *ptr_delay);

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
    const UI32_T delay);

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
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas);

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
    BOOL_T                   *ptr_ready);

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
    const AIR_POE_ALTERNATIVE_T alt);

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
    HAL_POE_PAIR_T *ptr_pair);

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
    const HAL_POE_PAIR_T pair);

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
    AIR_POE_TYPE_T             *ptr_type);

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
    const AIR_POE_TYPE_T        type);

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
    AIR_POE_PSE_MODE_T         *ptr_mode);

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
 *      None
 */
AIR_ERROR_NO_T
hal_an8503_poe_setPortPse(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_PSE_MODE_T    mode);

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
    UI32_T                     *ptr_priority);

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
    const UI32_T                priority);

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
    UI32_T      *ptr_current);

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
    const UI32_T current);

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
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas);

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
    AIR_POE_PORT_STATUS_T    *ptr_ps);

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
    HAL_POE_POWER_EVENT_T    *ptr_event);

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
    const UI32_T device);

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
    HAL_POE_DRIVER_T **pptr_poe_driver);

#endif /* End of HAL_AN8503_POE_H */
