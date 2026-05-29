/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  mw_led.h
 * PURPOSE:
 *      This file define LED types and declare LED APIs in MW.
 * NOTES:
 */

#ifndef MW_LED_H
#define MW_LED_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_cfg.h"
#include "air_port.h"
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_LED_GPIO_PIN_INVALID    (0xFF)

#define MW_LED_ID_ALL    (0xFF)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    MW_LED_TYPE_COPPER,
    MW_LED_TYPE_SFP,

    MW_LED_TYPE_LAST
} MW_LED_TYPE_T;

typedef enum
{
    MW_LED_ACTIVE_TYPE_LOW_ACTIVE,
    MW_LED_ACTIVE_TYPE_HIGH_ACTIVE,

    MW_LED_ACTIVE_TYPE_LAST
} MW_LED_ACTIVE_TYPE_T;

typedef struct
{
    UI32_T                      unit;
    AIR_CFG_TYPE_T              cfg_type;
    UI32_T                      param0;
    UI32_T                      param1;
    UI32_T                      value;
} MW_LED_CFG_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   mw_led_set_port_led
 * PURPOSE:
 *      Turn on or off a specific type of led for a specific port. If state is ON, set the led control
 *      mode to force and set the force state to ON. If state is OFF, set the
 *      force state to OFF and set the led control mode to phy.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Port number
 *      led_type                -- LED type
 *      led_count               -- The number of the leds for the LED type
 *      led_id                  -- LED ID
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      It should be called after SDK init.
 */
MW_ERROR_NO_T
mw_led_set_port_led(
    const UI32_T unit,
    const UI32_T port,
    const MW_LED_TYPE_T led_type,
    const UI32_T led_count,
    const UI32_T led_id,
    const AIR_PORT_PHY_LED_STATE_T state);

MW_ERROR_NO_T
mw_led_int_gpio_led(
    const UI32_T unit,
    const UI32_T pin);

MW_ERROR_NO_T
mw_led_set_gpio_led(
    const UI32_T unit,
    const UI32_T pin,
    const MW_LED_ACTIVE_TYPE_T active_type,
    const AIR_PORT_PHY_LED_STATE_T state);

MW_ERROR_NO_T
mw_led_set_gpio_led_manual_flash(
    const UI32_T unit,
    const UI32_T pin,
    const MW_LED_ACTIVE_TYPE_T active_type);

/* It should be called after SDK init */
MW_ERROR_NO_T
mw_led_set_gpio_led_auto_flash(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable);

void
mw_led_setLedCfg(
    const MW_LED_CFG_T *ptr_ledCfg,
    const UI8_T cfg_count);

void
mw_led_getLedCfg(
    const MW_LED_CFG_T **pptr_ledCfg,
    UI8_T *ptr_cfg_count);

MW_ERROR_NO_T
mw_led_getPortLedCount(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_copperLedCount,
    UI8_T *ptr_sfpLedCount);

#endif /* MW_LED_H */
