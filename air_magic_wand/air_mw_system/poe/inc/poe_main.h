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
 *      PoE application main loop.
 * NOTES:
 */

#ifndef POE_MAIN_H
#define POE_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include <mw_error.h>
#include <mw_log.h>
#include <poe_api.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define POE_APP_TASK_NAME               "POEAPP"
#define POE_APP_STACK_SIZE              (414)
#define POE_APP_THREAD_PRI              (5)
#define POE_RESET_PIN                   (4)
#define POE_APP_RESET_FLAG              (0xA5)
#define POE_APP_DEVICE_RESET_EVENT      (1U << 8)
#define POE_APP_INSUFFICIENT_PWR_EVENT  (1U << 9)
#define POE_APP_REVISION_ID_IP808AR     (0x808A)
#define MAX_POE_SYS_PWR_LIM             (400)  /* gain factor is 0.1W */
#define MAX_POE_SYS_PWR_BUF             (200)  /* gain factor is 0.1W */
#define POE_APP_KICK_OUT_RESTART_TIME   (1)    /* unit: 1(second) */
#define POE_APP_DEFAULT_RESTART_TIME    (10)
#define POE_APP_DEFAULT_SYS_AVAI_PWR    (1200) /* unit: 0.1W */
#define POE_APP_DEFAULT_PORT_AVAI_PWR   (360)  /* unit: 0.1W */
#define POE_PWR_EVT_BIT_LEN             (16)
#define POE_PD_CLASS_0_REQ_PWR          (155)  /* unit: 0.1W */
#define POE_PD_CLASS_1_REQ_PWR          (40)   /* unit: 0.1W */
#define POE_PD_CLASS_2_REQ_PWR          (70)   /* unit: 0.1W */
#define POE_PD_CLASS_3_REQ_PWR          (155)  /* unit: 0.1W */
#define POE_PD_CLASS_4_REQ_PWR          (300)  /* unit: 0.1W */
#define POE_APP_HW_RESET_COUNT          (3)
#define POE_APP_INIT_CTRL_MODULE_COUNT  (5)

/* MACRO FUNCTION DECLARATIONS
 */
#define UNIT_INFO(__var__, __unit__) ((__var__)->ptr_chip_info[__unit__])

#define PORT_INFO(__var__, __unit__, __port_idx__)                                              \
    ((__var__)->ptr_chip_info[__unit__]->ptr_port_map_info[__port_idx__])

#define POE_LOG_ERROR(...)     MW_LOG_ERROR(POE, ##__VA_ARGS__)
#define POE_LOG_WARN(...)      MW_LOG_WARN(POE, ##__VA_ARGS__)
#define POE_LOG_INFO(...)      MW_LOG_INFO(POE, ##__VA_ARGS__)
#define POE_LOG_DEBUG(...)     MW_LOG_DEBUG(POE, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    POE_APP_CTRL_MODULE_DEINIT = 0,
    POE_APP_CTRL_MODULE_INIT,
    POE_APP_CTRL_MODULE_LAST
} POE_APP_CTRL_MODULE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: poe_app_init
 *
 * PURPOSE:
 *      Initialize PoE application.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             --  Operation is successful.
 *      MW_E_BAD_PARAMETER  --  Bad parameter.
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_app_init(
    void);

/* FUNCTION NAME: poe_app_deinit
 *
 * PURPOSE:
 *      De-initialize PoE application.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             --  Operation is successful.
 *      MW_E_BAD_PARAMETER  --  Bad parameter.
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_app_deinit(
    void);

/* FUNCTION NAME:   poe_app_setPortPseControl
 * PURPOSE:
 *      Set pse control of the specified port.
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
poe_app_setPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   poe_app_getPortPseControl
 * PURPOSE:
 *      Get pse control settings of the specified port.
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
poe_app_getPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   poe_app_setPortType
 * PURPOSE:
 *      Set operation mode of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      type                    -- poe type
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                -- Successfully write the data.
 *      MW_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setPortType(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_TYPE_T type);

/* FUNCTION NAME:   poe_app_getPortType
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
poe_app_getPortType(
    const UI32_T     unit,
    const UI32_T     port,
    POE_PORT_TYPE_T *ptr_type);

/* FUNCTION NAME:   poe_app_setPortPowerLimit
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
poe_app_setPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T power_limit);

/* FUNCTION NAME:   poe_app_getPortPowerLimit
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
poe_app_getPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_power_limit);

/* FUNCTION NAME:   poe_app_setPortPriority
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
poe_app_setPortPriority(
    const UI32_T              unit,
    const UI32_T              port,
    const POE_PORT_PRIORITY_T priority);

/* FUNCTION NAME:   poe_app_getPortPriority
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
poe_app_getPortPriority(
    const UI32_T         unit,
    const UI32_T         port,
    POE_PORT_PRIORITY_T *ptr_priority);

/* FUNCTION NAME:   poe_app_setSystemPowerMode
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
poe_app_setSystemPowerMode(
    const POE_SYS_POWER_MODE_T power_mode);

/* FUNCTION NAME:   poe_app_getSystemPowerMode
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
poe_app_getSystemPowerMode(
    POE_SYS_POWER_MODE_T *ptr_power_mode);

/* FUNCTION NAME:   poe_app_setSystemPowerBudget
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
poe_app_setSystemPowerBudget(
    const UI32_T power_limit);

/* FUNCTION NAME:   poe_app_getSystemPowerBudget
 * PURPOSE:
 *      Set the power budget of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_power_limit         -- power limit
 * RETURN:
 *      MW_E_OK                -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerBudget(
    UI32_T *ptr_power_limit);

/* FUNCTION NAME:   poe_app_getPortStatus
 * PURPOSE:
 *      Get the status of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- port alternative
 * OUTPUT:
 *      ptr_status              -- port status
 * RETURN:
 *      MW_E_OK                -- Successfully write the data.
 *      MW_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortStatus(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_ALT_T  alt,
    POE_PORT_OP_STATUS_T *ptr_status);

/* FUNCTION NAME:   poe_app_getPortMeasurement
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
poe_app_getPortMeasurement(
    const UI32_T         unit,
    const UI32_T         port,
    const POE_PORT_ALT_T alt,
    POE_PORT_MEAS_T     *ptr_meas);

/* FUNCTION NAME:   poe_app_getSystemPowerSupply
 * PURPOSE:
 *      Get the power supply of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_supply              -- port supply
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_OP_INVALID         -- Operation is invalid.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerSupply(
    POE_SYS_SUPPLY_T *ptr_supply);

/* FUNCTION NAME:   poe_app_getSystemPowerStrategy
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
poe_app_getSystemPowerStrategy(
    POE_PWR_CONTROL_T *ptr_strategy);

/* FUNCTION NAME:   poe_app_clearPortEvent
 * PURPOSE:
 *      Clear the PoE events of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully clear the event.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_clearPortEvent(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: poe_app_getPoeMaxPower
 * PURPOSE:
 *      get poe max power budget value
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      UI32_T                   -- max power budget
 * NOTES:
 *      None
 */
UI32_T
poe_app_getPoeMaxPower(
    void);

/* FUNCTION NAME: poe_app_checkPseStatus
 * PURPOSE:
 *      check that PoE is working normally
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      BOOL_T                  -- TRUE: PSE working normally.
 *                                 FALSE:PSE working abnormally.
 * NOTES:
 *
 */
BOOL_T
poe_app_checkPseStatus(
    void);

#endif /* POE_MAIN_H */
