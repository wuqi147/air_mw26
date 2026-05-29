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

/* FILE NAME:  mw_led.c
 * PURPOSE:
 *      This file implements LED interfaces in MW.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "hwcfg_util.h"
#include "timer.h"
#include "air_chipscu.h"
#include "air_gpio.h"
#include "air_perif.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "mw_sfp_led.h"
#endif
#include "mw_led.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    const MW_LED_CFG_T *ptr_ledCfg;
    UI8_T cfg_count;
}__attribute__((packed)) MW_LED_CONTEX_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static MW_LED_CONTEX_T _mw_led_contex;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
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
    const AIR_PORT_PHY_LED_STATE_T state)
{
    I32_T rc = MW_E_OK, i = 0;
#ifdef AIR_SUPPORT_SFP
    AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;
#ifdef AIR_EN_SFP_LED
    MW_LED_ACTIVE_TYPE_T active_type = 0xFF;
    UI8_T pin = MW_LED_GPIO_PIN_INVALID;
#endif /* AIR_EN_SFP_LED */
#endif /* AIR_SUPPORT_SFP */

    if ((0 == led_count) || (led_id >= led_count) || (led_type >= MW_LED_TYPE_LAST))
    {
        return MW_E_BAD_PARAMETER;
    }

    MW_LOG_DEBUG(LED, "port:%d led_type:%d led_count:%d led_id:%d state:%d", port, led_type, led_count, led_id, state);

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_comboPort(unit, port)) && (AIR_PORT_PHY_LED_STATE_ON == state))
    {
        rc = air_port_getComboMode(unit, port, &combo_mode);
        if ((MW_LED_TYPE_COPPER == led_type) && (AIR_PORT_COMBO_MODE_PHY != combo_mode))
        {
            combo_mode = AIR_PORT_COMBO_MODE_PHY;
        }
        else if ((MW_LED_TYPE_SFP == led_type) && (AIR_PORT_COMBO_MODE_SERDES != combo_mode))
        {
            combo_mode = AIR_PORT_COMBO_MODE_SERDES;
        }
        rc |= air_port_setComboMode(unit, port, combo_mode);
        MW_LOG_DEBUG(LED, "air_port_setComboMode port:%d combo_mode:%d rc:%d", port, combo_mode, rc);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
    }
#endif

    for (i = 0; i < led_count; i++)
    {
        if ((MW_LED_ID_ALL != led_count) && (i != led_id))
        {
            continue;
        }

#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
        if (TRUE == sfp_port_is_serdesPort(unit, port))
        {
            mw_sfpLed_getPortActiveType(unit, port, i, &active_type);
            mw_sfpLed_getPortPin(unit, port, i, &pin);
            mw_led_set_gpio_led(unit, pin, active_type, state);
        }
        else
#endif
        {
            if (AIR_PORT_PHY_LED_STATE_ON == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
            }

            rc |= air_port_setPhyLedForceState(unit, port, i, state);
            MW_LOG_DEBUG(LED, "air_port_setPhyLedForceState port:%d led_id:%d state:%d rc:%d", port, i, state, rc);
            if (AIR_PORT_PHY_LED_STATE_OFF == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
            }
        }
    }

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_comboPort(unit, port)) && (AIR_PORT_PHY_LED_STATE_OFF == state))
    {
        /* Switch to PHY mode when turning LED off. */
        rc = air_port_getComboMode(unit, port, &combo_mode);
        if (AIR_PORT_COMBO_MODE_PHY != combo_mode)
        {
            rc |= air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_PHY);
            MW_LOG_DEBUG(LED, "air_port_setComboMode port:%d combo_mode:%d rc:%d", port, AIR_PORT_COMBO_MODE_PHY, rc);
        }
    }
#endif

    return rc;
}

MW_ERROR_NO_T
mw_led_int_gpio_led(
    const UI32_T unit,
    const UI32_T pin)
{
    I32_T rc = MW_E_OP_INVALID;

    rc = air_perif_setGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
    if (rc == AIR_E_OK)
    {
        /* Called after SDK init */
        if (pin < GPIO_PIN16)
        {
            rc = air_perif_setGpioOutputAutoMode(unit, pin, FALSE);
        }
    }
    else
    {
        /* Called before SDK init */
        rc = air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE + pin, AIR_CHIPSCU_IOMUX_ENABLE);
        rc |= air_gpio_setDirection(pin, AIR_GPIO_DIRECTION_OUTPUT);
        rc |= air_gpio_setOutputEnable(pin, FALSE);
    }

    return rc;
}

MW_ERROR_NO_T
mw_led_set_gpio_led(
    const UI32_T unit,
    const UI32_T pin,
    const MW_LED_ACTIVE_TYPE_T active_type,
    const AIR_PORT_PHY_LED_STATE_T state)
{
    I32_T rc = MW_E_OP_INVALID;

    if ((AIR_PORT_PHY_LED_STATE_ON != state) && (AIR_PORT_PHY_LED_STATE_OFF != state))
    {
        return MW_E_NOT_SUPPORT;
    }

    if (((MW_LED_ACTIVE_TYPE_HIGH_ACTIVE == active_type) && (AIR_PORT_PHY_LED_STATE_ON == state)) ||
        ((MW_LED_ACTIVE_TYPE_LOW_ACTIVE == active_type) && (AIR_PORT_PHY_LED_STATE_OFF == state)))
    {
        rc = air_perif_setGpioOutputData(unit, pin, AIR_PERIF_GPIO_DATA_HIGH);
    }
    else
    {
        rc = air_perif_setGpioOutputData(unit, pin, AIR_PERIF_GPIO_DATA_LOW);
    }

    if (rc != AIR_E_OK)
    {
        /* Called before SDK init */
        if (((MW_LED_ACTIVE_TYPE_HIGH_ACTIVE == active_type) && (AIR_PORT_PHY_LED_STATE_ON == state)) ||
            ((MW_LED_ACTIVE_TYPE_LOW_ACTIVE == active_type) && (AIR_PORT_PHY_LED_STATE_OFF == state)))
        {
            rc = air_gpio_setValue(pin, GPIO_PIN_HIGH);
        }
        else
        {
            rc = air_gpio_setValue(pin, GPIO_PIN_LOW);
        }

        rc |= air_gpio_setOutputEnable(pin, TRUE);
    } /* else Called after SDK init */

    return rc;
}

MW_ERROR_NO_T
mw_led_set_gpio_led_manual_flash(
    const UI32_T unit,
    const UI32_T pin,
    const MW_LED_ACTIVE_TYPE_T active_type)
{
    MW_ERROR_NO_T rc = MW_E_BAD_PARAMETER;

    if (MW_LED_GPIO_PIN_INVALID != pin)
    {
        rc = mw_led_set_gpio_led(unit, pin, active_type, AIR_PORT_PHY_LED_STATE_ON);
        delay1ms(20);
        rc |= mw_led_set_gpio_led(unit, pin, active_type, AIR_PORT_PHY_LED_STATE_OFF);
    }

    return rc;
}

/* It should be called after SDK init */
MW_ERROR_NO_T
mw_led_set_gpio_led_auto_flash(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if (pin >= GPIO_PIN16)
    {
        return MW_E_BAD_PARAMETER;
    }

    if (TRUE == enable)
    {
        rc  = air_perif_setGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
        rc |= air_perif_setGpioOutputAutoPatt(unit, pin, AIR_PERIF_GPIO_PATT_HZ_ONE);
    }

    rc |= air_perif_setGpioOutputAutoMode(unit, pin, enable);

    return rc;
}

void
mw_led_setLedCfg(
    const MW_LED_CFG_T *ptr_ledCfg,
    const UI8_T cfg_count)
{
    _mw_led_contex.ptr_ledCfg = ptr_ledCfg;
    _mw_led_contex.cfg_count = cfg_count;
}

void
mw_led_getLedCfg(
    const MW_LED_CFG_T **pptr_ledCfg,
    UI8_T *ptr_cfg_count)
{
    *pptr_ledCfg = _mw_led_contex.ptr_ledCfg;
    *ptr_cfg_count = _mw_led_contex.cfg_count;
}

MW_ERROR_NO_T
mw_led_getPortLedCount(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_copperLedCount,
    UI8_T *ptr_sfpLedCount)
{
    UI8_T i = 0, copper_led_count = 0, sfp_led_count = 0;

    if ((NULL == ptr_copperLedCount) && (NULL == ptr_sfpLedCount))
    {
        return MW_E_BAD_PARAMETER;
    }

#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
    /* SERDES port */
    if (sfp_port_is_serdesPort(unit, port))
    {
        sfp_led_count = mw_sfpLed_getLedCount();
    }
    else
#endif
    {
        if ((NULL == _mw_led_contex.ptr_ledCfg) || (0 == _mw_led_contex.cfg_count))
        {
            return MW_E_NOT_INITED;
        }

        /* PHY port and Combo port */
        for (i = 0; i < _mw_led_contex.cfg_count; i++)
        {
            if (AIR_CFG_TYPE_PHY_LED_COUNT == _mw_led_contex.ptr_ledCfg[i].cfg_type)
            {
                copper_led_count = _mw_led_contex.ptr_ledCfg[i].value;
            }

#ifdef AIR_SUPPORT_SFP
            if (sfp_port_is_comboPort(unit, port))
            {
                if ((AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR == _mw_led_contex.ptr_ledCfg[i].cfg_type) &&
                    (port == _mw_led_contex.ptr_ledCfg[i].param0))
                {
                    sfp_led_count++;
                }
            }
#endif
        }

#ifdef AIR_SUPPORT_SFP
        if ((sfp_port_is_comboPort(unit, port)) && (0 == sfp_led_count))
        {
            sfp_led_count = copper_led_count;
        }
#endif
    }

    if (NULL != ptr_copperLedCount)
    {
        *ptr_copperLedCount = copper_led_count;
    }

    if (NULL != ptr_sfpLedCount)
    {
        *ptr_sfpLedCount = sfp_led_count;
    }

    return MW_E_OK;
}

