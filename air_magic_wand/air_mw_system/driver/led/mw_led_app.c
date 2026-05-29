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

/* FILE NAME:  mw_led_app.c
 * PURPOSE:
 *      This file implements LED Apps.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "hwcfg_util.h"
#include "air_init.h"
#include "mw_led_app.h"
#include "switch.h"
#include "mw_utils.h"
#include "osapi_memory.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#endif
#include "timer.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T sysled_pin;
    UI8_T active_type;  /* MW_LED_ACTIVE_TYPE_T */
    UI8_T flash_type;   /* MW_LEDAPP_SYSLED_FLASH_TYPE_T */
}__attribute__((packed)) MW_LEDAPP_SYSLED_INFO_T;

typedef struct
{
#ifdef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    MW_LEDAPP_SYSLED_INFO_T sysled_info;
#endif
}__attribute__((packed)) MW_LEDAPP_CONTEXT_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
MW_LEDAPP_CONTEXT_T _mw_ledapp_context = {
#ifdef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    {MW_LED_GPIO_PIN_INVALID, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, MW_LEDAPP_SYSLED_FLASH_TYPE_NONE}
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
_mw_ledapp_sysled_set_info(
    UI8_T sysled_pin,
    MW_LED_ACTIVE_TYPE_T active_type,
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_AUTO_FLASH == flash_type) && (GPIO_PIN16 <= sysled_pin))
    {
        return MW_E_NOT_SUPPORT;
    }

    _mw_ledapp_context.sysled_info.sysled_pin = sysled_pin;
    _mw_ledapp_context.sysled_info.active_type = active_type;
    _mw_ledapp_context.sysled_info.flash_type = flash_type;

    return MW_E_OK;
#endif
}

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
mw_ledapp_sysled_enable_auto_flash(
    UI32_T unit)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_AUTO_FLASH == _mw_ledapp_context.sysled_info.flash_type) &&
        (GPIO_PIN16 > _mw_ledapp_context.sysled_info.sysled_pin))
    {
        return mw_led_set_gpio_led_auto_flash(unit, _mw_ledapp_context.sysled_info.sysled_pin, TRUE);
    }

    return MW_E_NOT_SUPPORT;
#endif
}

MW_ERROR_NO_T
mw_ledapp_sysled_trigger_manual_flash(
    UI32_T unit)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH == _mw_ledapp_context.sysled_info.flash_type) &&
        (MW_LED_GPIO_PIN_INVALID != _mw_ledapp_context.sysled_info.sysled_pin))
    {
        return mw_led_set_gpio_led_manual_flash(unit, _mw_ledapp_context.sysled_info.sysled_pin, _mw_ledapp_context.sysled_info.active_type);
    }

    return MW_E_NOT_SUPPORT;
#endif
}

MW_ERROR_NO_T
mw_ledapp_sysled_set_state(
    UI32_T unit,
    AIR_PORT_PHY_LED_STATE_T state)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;

    if (MW_LED_GPIO_PIN_INVALID != _mw_ledapp_context.sysled_info.sysled_pin)
    {
        if (MW_LEDAPP_SYSLED_FLASH_TYPE_FLASHING == _mw_ledapp_context.sysled_info.flash_type)
        {
            /* This function is supposed to be called only once. Therefore do not check if gpio has been inited or not. */
            rc = mw_led_int_gpio_led(unit, _mw_ledapp_context.sysled_info.sysled_pin);
        }

        rc |= mw_led_set_gpio_led(unit, _mw_ledapp_context.sysled_info.sysled_pin, (MW_LED_ACTIVE_TYPE_T)_mw_ledapp_context.sysled_info.active_type, state);
    }

    return rc;
#endif
}

void
mw_ledapp_sysled_get_info(
    UI8_T *ptr_sysled_pin,
    MW_LED_ACTIVE_TYPE_T *ptr_active_type,
    MW_LEDAPP_SYSLED_FLASH_TYPE_T *ptr_flash_type)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    if (NULL != ptr_sysled_pin)
    {
        (*ptr_sysled_pin) = _mw_ledapp_context.sysled_info.sysled_pin;
    }

    if (NULL != ptr_active_type)
    {
        (*ptr_active_type) = _mw_ledapp_context.sysled_info.active_type;
    }

    if (NULL != ptr_flash_type)
    {
        (*ptr_flash_type) = _mw_ledapp_context.sysled_info.flash_type;
    }
#endif
}

/* SYS LED flashing:
* pin >= GPIO_PIN16: only manaul flashing could be adopted.
* pin < GPIO_PIN16: auto flashing could be used.
*/
MW_ERROR_NO_T
mw_ledapp_sysled_init(
    const UI32_T unit,
    UI8_T sysled_pin,
    MW_LED_ACTIVE_TYPE_T active_type,
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type)
{
#ifndef MW_LEDAPP_SYSLED_FLASH_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    MW_ERROR_NO_T rc = MW_E_BAD_PARAMETER;

    rc = _mw_ledapp_sysled_set_info(sysled_pin, active_type, flash_type);
    if ((MW_E_OK == rc) && ((MW_LEDAPP_SYSLED_FLASH_TYPE_NONE <= flash_type) && (MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH >= flash_type)))
    {
        /* Do not init gpio if it is in FLASHING state. Otherwise, it will stop flashing. */
        rc = mw_led_int_gpio_led(unit, sysled_pin);
    }

    return rc;
#endif
}

MW_ERROR_NO_T
mw_ledapp_portled_selftest(
    const UI32_T unit,
    MW_LEDAPP_PORTLED_SELFTEST_HOOK ptr_func)
{
#ifndef MW_LEDAPP_PORTLED_SELFTEST_SUPPORT
    return MW_E_NOT_SUPPORT;
#else
    I32_T rc = MW_E_OP_INCOMPLETE;
    UI32_T total_port_num = 0, port = 0, copper_led_count = 0, sfp_led_count = 0, combo_sfp_led_count = 0, led_id = 0;
    AIR_INIT_PORT_MAP_T *ptr_portMapList = NULL, *ptr_cur = NULL;

    osapi_calloc((sizeof(AIR_INIT_PORT_MAP_T) * (AIR_MAX_PORT_NUM + 1)), "MWLEDAPP", (void**)&ptr_portMapList);
    if (NULL != ptr_portMapList)
    {
        rc = air_init_getSdkPortMap(unit, &total_port_num, ptr_portMapList);
        if (AIR_E_OK != rc)
        {
            MW_FREE(ptr_portMapList);
        }
    }

    if (NULL != ptr_portMapList)
    {
        /* Port LED self test:
         * 1. Copper port / Combo copper port LED on/off: LED 0 on/off, LED 1 on/off, ...
         * 2. SFP port / Combo SFP port LED on/off: LED 0 on/off, LED 1 on/off, ...
         */

        /* Get led count */
        for (port = 0; port < total_port_num; port++)
        {
            if (AIR_INIT_PORT_TYPE_CPU == ptr_cur->port_type)
            {
                continue;
            }

            if ((0 == sfp_led_count) && (TRUE == sfp_port_is_serdesPort(unit, port)))
            {
                rc |= mw_led_getPortLedCount(unit, port, NULL, (UI8_T *)(&sfp_led_count));
            }
            else if ((0 == combo_sfp_led_count) && (TRUE == sfp_port_is_comboPort(unit, port)))
            {
                rc |= mw_led_getPortLedCount(unit, port, NULL, (UI8_T *)(&combo_sfp_led_count));
            }
            else if (0 == copper_led_count)
            {
                rc |= mw_led_getPortLedCount(unit, port, (UI8_T *)(&copper_led_count), NULL);
            }
        }

        /* 1. Copper port / Combo copper port LED on/off */
        for (led_id = 0; led_id < copper_led_count; led_id++)
        {
            if (NULL != ptr_func)
            {
                ptr_func(unit);
            }
            /* Set Copper port / Combo copper port LED on */
            for (port = 0; port < total_port_num; port++)
            {
                ptr_cur = &ptr_portMapList[port];
                if ((AIR_INIT_PORT_TYPE_CPU == ptr_cur->port_type)
#ifdef AIR_SUPPORT_SFP
                    || (TRUE == sfp_port_is_serdesPort(unit, port))
                    || (TRUE == sfp_port_is_pureComboSerdesPort(unit, port))
#endif
                    )
                {
                    continue;
                }

                rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_COPPER, copper_led_count, led_id, AIR_PORT_PHY_LED_STATE_ON);
            }

            delay1ms(MW_LEDAPP_LED_ON_PERIOD_MS);

            if (NULL != ptr_func)
            {
                ptr_func(unit);
            }

            /* Set Copper port / Combo copper port LED off */
            for (port = 0; port < total_port_num; port++)
            {
                ptr_cur = &ptr_portMapList[port];
                if ((AIR_INIT_PORT_TYPE_CPU == ptr_cur->port_type)
#ifdef AIR_SUPPORT_SFP
                    || (TRUE == sfp_port_is_serdesPort(unit, port))
                    || (TRUE == sfp_port_is_pureComboSerdesPort(unit, port))
#endif
                    )
                {
                    continue;
                }

                rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_COPPER, copper_led_count, led_id, AIR_PORT_PHY_LED_STATE_OFF);
            }

            delay1ms(MW_LEDAPP_LED_OFF_PERIOD_MS);
        }

        /* 2. SFP port / Combo SFP port LED on/off */
#ifdef AIR_SUPPORT_SFP
        for (led_id = 0; ((led_id < sfp_led_count) || (led_id < combo_sfp_led_count)); led_id++)
        {
            if (NULL != ptr_func)
            {
                ptr_func(unit);
            }

            /* Set SFP port / Combo SFP port LED on */
            for (port = 0; port < total_port_num; port++)
            {
                ptr_cur = &ptr_portMapList[port];
                if ((AIR_INIT_PORT_TYPE_CPU == ptr_cur->port_type) ||
                    ((TRUE != sfp_port_is_serdesPort(unit, port)) &&
                      (TRUE != sfp_port_is_comboPort(unit, port))))
                {
                    continue;
                }

                if ((TRUE == sfp_port_is_comboPort(unit, port)) && (led_id < combo_sfp_led_count))
                {
                    rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_SFP, combo_sfp_led_count, led_id, AIR_PORT_PHY_LED_STATE_ON);
                }
                else if (led_id < sfp_led_count)
                {
                    rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_SFP, sfp_led_count, led_id, AIR_PORT_PHY_LED_STATE_ON);
                }
            }

            delay1ms(MW_LEDAPP_LED_ON_PERIOD_MS);

            if (NULL != ptr_func)
            {
                ptr_func(unit);
            }

            /* Set SFP port / Combo SFP port LED off */
            for (port = 0; port < total_port_num; port++)
            {
                ptr_cur = &ptr_portMapList[port];
                if ((AIR_INIT_PORT_TYPE_CPU == ptr_cur->port_type) ||
                    ((TRUE != sfp_port_is_serdesPort(unit, port)) &&
                      (TRUE != sfp_port_is_comboPort(unit, port))))
                {
                    continue;
                }

                if ((TRUE == sfp_port_is_comboPort(unit, port)) && (led_id < combo_sfp_led_count))
                {
                    rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_SFP, combo_sfp_led_count, led_id, AIR_PORT_PHY_LED_STATE_OFF);
                }
                else if (led_id < sfp_led_count)
                {
                    rc |= mw_led_set_port_led(unit, port, MW_LED_TYPE_SFP, sfp_led_count, led_id, AIR_PORT_PHY_LED_STATE_OFF);
                }
            }

            delay1ms(MW_LEDAPP_LED_OFF_PERIOD_MS);
        }
#endif

        MW_FREE(ptr_portMapList);
    }

    return rc;
#endif
}

