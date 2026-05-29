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

/* FILE NAME:   poe_config_customer.h
 * PURPOSE:
 *      Configure PoE.
 * NOTES:
 *
 */

#ifndef POE_CONFIG_CUSTOMER_H
#define POE_CONFIG_CUSTOMER_H

/* INCLUDE FILE DECLARATIONS
 */
#include <mw_utils.h>
#include <mw_led.h>
#include <poe_api.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define POE_MAX_LED_BLINK_THRESHOLD_IN_PERCENTAGE_DEFAULT         (90)
/* DATA TYPE DECLARATIONS
 */
/* Config PoE port settings. */
typedef struct
{
    UI8_T                 poe_port_number;
    UI8_T                 max_led_blink_threshold;      /* When total consumed power is above the threshold,
                                                           the MAX LED will blink. */
    UI8_T                 max_led_switch_unit;          /* switch unit containing the GPIO pin assigned to the MAX LED */
    UI8_T                 global_led_pin;               /* GPIO pin assigned to the MAX LED */
    MW_LED_ACTIVE_TYPE_T  global_led_active_type;       /* Determine the signal level required to turn on MAX LED */
    POE_PWR_CONTROL_T     poe_power_strategy_control;   /* PoE power strategy control */
    UI8_T                 poe_reset_pin;                /* PoE reset pin */
} POE_CONFIG_SETTINGS_T;

typedef struct
{
    I8_T                         product_id;
    const POE_CONFIG_SETTINGS_T *ptr_poe_config_settings;
} POE_CONFIG_PRODUCT_SETTINGS_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   poe_config_getPoeCfg
 * PURPOSE:
 *      Get the poe config settings.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      A pointer which points to the poe config settings
 *
 * NOTES:
 *      None
 */
const POE_CONFIG_SETTINGS_T*
poe_config_getPoeCfg(
    void);

#endif
