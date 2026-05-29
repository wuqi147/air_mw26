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

/* FILE NAME:   poe_led.c
 * PURPOSE:
 *      Define poe led function.
 *
 * NOTES:
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <poe_led.h>
#include <poe_main.h>
#include <poe_info.h>
#include <poe_config_customer.h>
#include <air_perif.h>
#include <air_gpio.h>
#include <air_chipscu.h>
#include <mw_led.h>
#include <poe_api.h>
#include <hal_poe.h>
#include <switch.h>
#include <hwcfg_util.h>
/* NAMING CONSTANT DECLARATIONS
*/
/* MACRO FUNCTION DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
*/
/* STATIC VARIABLE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
static void
_poe_maxLed_set(
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg,
    const BOOL_T                 enable_blink,
    const BOOL_T                 isAlwaysOn);
/* LOCAL SUBPROGRAM BODIES
*/
/* FUNCTION NAME:   _poe_maxLed_set
 * PURPOSE:
 *      PoE Max LED helper function.
 * INPUT:
 *      enable_blink            -- TRUE: enable PoE Max LED blinking
 *                                 FALSE:disable PoE Max LED blinking
 *      isAlwaysOn              -- TRUE: PoE Max LED is always on
 *                                 FALSE: PoE Max LED is off
 *      ptr_poeCfg              -- Pointer to the PoE customer config
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_maxLed_set(
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg,
    const BOOL_T                 enable_blink,
    const BOOL_T                 isAlwaysOn)
{
    UI8_T                 rc = AIR_E_OK;
    AIR_PERIF_GPIO_DATA_T gpio_data = AIR_PERIF_GPIO_DATA_LOW;

    if (TRUE == enable_blink)
    {
        rc = air_perif_setGpioOutputAutoMode(ptr_poeCfg->max_led_switch_unit,
                                             ptr_poeCfg->global_led_pin,
                                             TRUE);
        rc |= air_perif_setGpioOutputAutoPatt(ptr_poeCfg->max_led_switch_unit,
                                              ptr_poeCfg->global_led_pin,
                                              AIR_PORT_PHY_LED_PATT_HZ_TWO);
        if (AIR_E_OK != rc)
        {
            POE_LOG_ERROR("Enable PoE Max LED blnking failed(%d)", rc);
        }
    }
    else
    {
        rc = air_perif_setGpioOutputAutoMode(ptr_poeCfg->max_led_switch_unit,
                                             ptr_poeCfg->global_led_pin,
                                             FALSE);
        if (AIR_E_OK != rc)
        {
            POE_LOG_ERROR("Disable PoE Max LED blnking failed(%d)", rc);
        }
        if (TRUE == isAlwaysOn)
        {
            if (MW_LED_ACTIVE_TYPE_HIGH_ACTIVE == ptr_poeCfg->global_led_active_type)
            {
                gpio_data = AIR_PERIF_GPIO_DATA_HIGH;
            }
            else
            {
                gpio_data = AIR_PERIF_GPIO_DATA_LOW;
            }
        }
        else
        {
            if (MW_LED_ACTIVE_TYPE_HIGH_ACTIVE == ptr_poeCfg->global_led_active_type)
            {
                gpio_data = AIR_PERIF_GPIO_DATA_LOW;
            }
            else
            {
                gpio_data = AIR_PERIF_GPIO_DATA_HIGH;
            }
        }
        rc = air_perif_setGpioOutputData(ptr_poeCfg->max_led_switch_unit,
                                         ptr_poeCfg->global_led_pin,
                                         gpio_data);
        if (AIR_E_OK != rc)
        {
            POE_LOG_ERROR("set GPIO output data error(%d), data(%d)", rc, gpio_data);
        }
    }
}
/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME:   poe_maxLed_handler
 * PURPOSE:
 *      PoE Max LED handler function.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_poe_control_block         -- Pointer to the related PoE data
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
poe_maxLed_handler(
    POE_CB_T    *ptr_poe_control_block)
{
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
    static BOOL_T                max_led_off = FALSE, max_led_blink = FALSE, max_led_on = FALSE;
    UI8_T                        rc = AIR_E_OK;
    UI32_T                       power_limit = 0, data = 0;
    POE_SYS_SUPPLY_T             supply = {0};

    ptr_poeCfg = poe_config_getPoeCfg();
    if ((NULL != ptr_poeCfg) && (MW_LED_GPIO_PIN_INVALID != ptr_poeCfg->global_led_pin))
    {
        if (FALSE == ptr_poe_control_block->max_led_solid_flag)
        {
            rc = hal_poe_readReg(0, 0,
                                 HAL_POE_PAGE_1,
                                 POE_SYS_INIT_AND_CURR_OVERLOAD_EVENT,
                                 HAL_POE_REG_LEN_1,
                                 &data);
            if (POE_PSE_CURR_OVERLOAD_EVENT == BITS_OFF_R(data,
                                                          POE_PSE_CURR_OVERLOAD_EVENT_OFFSET,
                                                          POE_PSE_CURR_OVERLOAD_EVENT_LEN))
            {
                data = POE_PSE_CURR_OVERLOAD_EVENT_MASK;
                rc = hal_poe_writeReg(0, 0,
                                      HAL_POE_PAGE_1,
                                      POE_SYS_INIT_AND_CURR_OVERLOAD_EVENT,
                                      HAL_POE_REG_LEN_1,
                                      &data);
                ptr_poe_control_block->max_led_solid_flag = TRUE;
                ptr_poe_control_block->max_led_counter = 0;
                if (FALSE == max_led_on)
                {
                    _poe_maxLed_set(ptr_poeCfg, FALSE, TRUE);
                    max_led_on = TRUE;
                }
                max_led_off = FALSE;
                max_led_blink = FALSE;
            }
        }
        if (AIR_E_OK == rc)
        {
            rc = poe_getSystemPowerBudget(&power_limit);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("get system power budget error(%d)", rc);
            }
            else
            {
                rc = poe_getSystemPowerSupply(&supply);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("get system power supply error(%d)", rc);
                }
            }
        }
        if (AIR_E_OK == rc)
        {
            if (TRUE == ptr_poe_control_block->max_led_solid_flag)
            {
                if (0 == (++ptr_poe_control_block->max_led_counter % POE_MAX_LED_SOLID_COUNT))
                {
                    ptr_poe_control_block->max_led_solid_flag = FALSE;
                }
            }
            else if ((supply.allocated_power >= ((power_limit * (UI32_T)ptr_poeCfg->max_led_blink_threshold) / 100)) &&
                     (supply.allocated_power <= power_limit))
            {
                if (FALSE == max_led_blink)
                {
                    _poe_maxLed_set(ptr_poeCfg, TRUE, FALSE);
                    max_led_blink = TRUE;
                }
                max_led_off = FALSE;
                max_led_on = FALSE;
            }
            else
            {
                if (FALSE == max_led_off)
                {
                    _poe_maxLed_set(ptr_poeCfg, FALSE, FALSE);
                    max_led_off = TRUE;
                }
                max_led_blink = FALSE;
                max_led_on = FALSE;
            }
        }
    }
}

/* FUNCTION NAME:   poe_maxLed_init
 * PURPOSE:
 *      PoE Max LED initialization function.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_maxLed_init(
    void)
{
    UI8_T rc = AIR_E_OK;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;

    ptr_poeCfg = poe_config_getPoeCfg();
    if ((NULL != ptr_poeCfg) && (MW_LED_GPIO_PIN_INVALID != ptr_poeCfg->global_led_pin))
    {
        rc = air_chipscu_setIomuxFuncState(ptr_poeCfg->max_led_switch_unit,
                                           AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE + ptr_poeCfg->global_led_pin,
                                           AIR_CHIPSCU_IOMUX_ENABLE);

        rc |= air_gpio_setDirection(ptr_poeCfg->global_led_pin, AIR_GPIO_DIRECTION_OUTPUT);
        if (MW_LED_ACTIVE_TYPE_LOW_ACTIVE == ptr_poeCfg->global_led_active_type)
        {
            rc |= air_gpio_setValue(ptr_poeCfg->global_led_pin, GPIO_PIN_HIGH);
        }
        else
        {
            rc |= air_gpio_setValue(ptr_poeCfg->global_led_pin, GPIO_PIN_LOW);
        }
        rc |= air_gpio_setOutputEnable(ptr_poeCfg->global_led_pin, ENABLE_PIN_OE);
        if (AIR_E_OK != rc)
        {
            POE_LOG_ERROR("Max LED set HW blink failed!(%d)", rc);
        }
    }

    return MW_E_OK;
}