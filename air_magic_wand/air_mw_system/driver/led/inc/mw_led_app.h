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

/* FILE NAME:  mw_led_app.h
 * PURPOSE:
 *      This file define types and declare APIs for LED Apps.
 * NOTES:
 */

#ifndef MW_LED_APP_H
#define MW_LED_APP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_led.h"

/* NAMING CONSTANT DECLARATIONS
 */
//#define MW_LEDAPP_PORTLED_SELFTEST_SUPPORT
#define MW_LEDAPP_SYSLED_FLASH_SUPPORT

#define MW_LEDAPP_LED_OFF_PERIOD_MS   (500)
#define MW_LEDAPP_LED_ON_PERIOD_MS    (500)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    MW_LEDAPP_SYSLED_FLASH_TYPE_NONE,
    MW_LEDAPP_SYSLED_FLASH_TYPE_AUTO_FLASH,
    MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH,
    MW_LEDAPP_SYSLED_FLASH_TYPE_FLASHING,   /* SYS LED is flashing */

    MW_LEDAPP_SYSLED_FLASH_TYPE_LAST
} MW_LEDAPP_SYSLED_FLASH_TYPE_T;

typedef MW_ERROR_NO_T (*MW_LEDAPP_PORTLED_SELFTEST_HOOK)(const UI32_T unit);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T
mw_ledapp_sysled_enable_auto_flash(
    UI32_T unit);

MW_ERROR_NO_T
mw_ledapp_sysled_trigger_manual_flash(
    UI32_T unit);

MW_ERROR_NO_T
mw_ledapp_sysled_set_state(
    UI32_T unit,
    AIR_PORT_PHY_LED_STATE_T state);

void
mw_ledapp_sysled_get_info(
    UI8_T *ptr_sysled_pin,
    MW_LED_ACTIVE_TYPE_T *ptr_active_type,
    MW_LEDAPP_SYSLED_FLASH_TYPE_T *ptr_flash_type);

/* SYS LED flashing:
* pin >= GPIO_PIN16: only manaul flashing could be adopted.
* pin < GPIO_PIN16: auto flashing could be used.
*/
MW_ERROR_NO_T
mw_ledapp_sysled_init(
    const UI32_T unit,
    UI8_T sysled_pin,
    MW_LED_ACTIVE_TYPE_T active_type,
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type);

MW_ERROR_NO_T
mw_ledapp_portled_selftest(
    const UI32_T unit,
    MW_LEDAPP_PORTLED_SELFTEST_HOOK ptr_func);

#endif /* MW_LED_APP_H */
