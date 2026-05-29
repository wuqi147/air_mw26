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

/* FILE NAME:  lp_led.c
 * PURPOSE:
 *    This file contains the implementation of LED control for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_log.h"
#include    "lp_led.h"
#include    <air_cfg.h>
#include    <hal_cfg.h>
#ifdef AIR_SUPPORT_SFP
#include    "sfp_util.h"
#include    "sfp_port.h"
#endif
#include    "mw_led.h"
#include    <air_perif.h>
#include    <pp_def.h>
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
#include    "lp_config_customer.h"
#endif
#ifdef LP_GLOBAL_LED_SUPPORT
#include    "air_chipscu.h"
#include    "air_gpio.h"
#include    "hwcfg_util.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T                   _led_1g_pbmp = 0;
static UI32_T                   _led_speed_pbmp = 0;
static MW_PORT_BITMAP_T         _led_single_pbmp = { 0 };
#ifdef LP_GLOBAL_LED_SUPPORT
static UI8_T                    _lp_global_led_pin = MW_LED_GPIO_PIN_INVALID;
static MW_LED_ACTIVE_TYPE_T     _lp_global_led_active_type = MW_LED_ACTIVE_TYPE_LOW_ACTIVE;
static AIR_PORT_PHY_LED_PATT_T  _lp_global_led_patt = AIR_PORT_PHY_LED_PATT_HZ_HALF;
#endif /* LP_GLOBAL_LED_SUPPORT */

#ifdef LP_CUSTOMER_CONFIG_SUPPORT
static AIR_PORT_PHY_LED_PATT_T  _lp_port_led_patt = AIR_PORT_PHY_LED_PATT_HZ_HALF;
#endif /* LP_CUSTOMER_CONFIG_SUPPORT */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: lp_led_init
 * PURPOSE:
 *      Initialize the LED for loop prevention module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_led_init(
    void)
{
    UI32_T u_portId = 0;
    AIR_CFG_VALUE_T ledBehavior;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        osapi_memset(&ledBehavior, 0, sizeof(AIR_CFG_VALUE_T));
        ledBehavior.value = 0xFFF;
        ledBehavior.param0 = u_portId;
        ledBehavior.param1 = 1;

        if (0 == hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &ledBehavior))
        {
            if (0xFFF == ledBehavior.value)
            {
                MW_PORT_ADD(_led_single_pbmp, u_portId);
            }
            else if (ledBehavior.value & 0x01)
            {
                _led_1g_pbmp |= (0x01 << u_portId);
            }
        }
        else
        {
            LP_LOG_ERROR("get Port%d LED behavior fail", u_portId);
        }
    }


#ifdef LP_CUSTOMER_CONFIG_SUPPORT
    MW_LP_LED_CFG_T lp_led_cfg = {0};
    MW_ERROR_NO_T   ret = MW_E_OK;

    ret = lp_getLpLedCfg(&lp_led_cfg);
    if (ret == MW_E_OK)
    {
        if ((lp_led_cfg.port_led_pattern >= AIR_PORT_PHY_LED_PATT_HZ_HALF) &&
             (lp_led_cfg.port_led_pattern < AIR_PORT_PHY_LED_PATT_LAST))
        {
            _lp_port_led_patt = lp_led_cfg.port_led_pattern;
        }
    }
#endif

#ifdef LP_GLOBAL_LED_SUPPORT
    AIR_PORT_BITMAP_T lp_pbmp = {0};

    AIR_PORT_BITMAP_CLEAR(lp_pbmp);
    lp_led_updateGlobalLed(lp_pbmp);
#endif /* LP_GLOBAL_LED_SUPPORT */

    return MW_E_OK;
}

/* FUNCTION NAME: lp_led_setBehavior
 * PURPOSE:
 *      Set LED behavior for specific port.
 *
 * INPUT:
 *      block            --  TRUE if block, FALSE if unblock
 *      u_portId         --  Port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_led_setBehavior(
    BOOL_T block,
    UI16_T u_portId)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = u_portId;
    AIR_PORT_STATUS_T ps;
    UI32_T            led_id_en = 0, led_id_dis = 0;
    MW_ERROR_NO_T     ret = MW_E_OK;
    AIR_PORT_PHY_LED_CTRL_MODE_T    led_mode = 0;

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboPort(unit, port)))
    {
        rc = sfp_port_getPortStatus(unit, port, &ps);
    }
    else
#endif
    {
        rc = air_port_getPortStatus(unit, port, &ps);
    }
    if (AIR_E_OK == rc)
    {
        LP_LOG_DEBUG("block=%u, u_portId=%u, ps.speed=%u, ps.flags=0x%x", block, u_portId, ps.speed, ps.flags);
        if (block)
        {
            if (ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)
            {
                if ((AIR_PORT_SPEED_10M == ps.speed) || (AIR_PORT_SPEED_100M == ps.speed))
                {
                    _led_speed_pbmp &= ~(0x01 << port);
                    if (TRUE == MW_PORT_CHK(_led_single_pbmp, port))
                    {
                        led_id_en = USE_LED0;
                    }
                    else if (_led_1g_pbmp & (0x01 << port))
                    {
                        led_id_en = USE_LED0;
                        led_id_dis = USE_LED1;
                    }
                    else
                    {
                        led_id_en = USE_LED1;
                        led_id_dis = USE_LED0;
                    }
                    rc = air_port_setPhyLedCtrlMode(unit, port, led_id_en, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
                    if (rc != AIR_E_OK)
                    {
                        LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_en);
                        ret = MW_E_OP_INCOMPLETE;
                    }
                    else
                    {
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
                        rc = air_port_setPhyLedForcePattCfg(unit, port, led_id_en, _lp_port_led_patt);
#endif
                        rc |= air_port_setPhyLedForceState(unit, port, led_id_en, AIR_PORT_PHY_LED_STATE_FORCE_PATT);
                        if (rc != AIR_E_OK)
                        {
                            LP_LOG_ERROR("set port=%u led %u phyLedForceState failed", port, led_id_en);
                            ret = MW_E_OP_INCOMPLETE;
                        }
                    }
                    if (FALSE == MW_PORT_CHK(_led_single_pbmp, port))
                    {
                        if ((AIR_E_OK == air_port_getPhyLedCtrlMode(unit, port, led_id_dis, &led_mode)) &&
                           (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == led_mode))
                        {
                            rc = air_port_setPhyLedCtrlMode(unit, port, led_id_dis, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
                            if (rc != AIR_E_OK)
                            {
                                LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_dis);
                            }
                        }
                    }
                }
                else
                {
                    _led_speed_pbmp |= (0x01 << port);
                    if (TRUE == MW_PORT_CHK(_led_single_pbmp, port))
                    {
                        led_id_en = USE_LED0;
                    }
                    else if (_led_1g_pbmp & (0x01 << port))
                    {
                        led_id_en = USE_LED1;
                        led_id_dis = USE_LED0;
                    }
                    else
                    {
                        led_id_en = USE_LED0;
                        led_id_dis = USE_LED1;
                    }
                    rc = air_port_setPhyLedCtrlMode(unit, port, led_id_en, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
                    if (rc != AIR_E_OK)
                    {
                        LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_en);
                        ret = MW_E_OP_INCOMPLETE;
                    }
                    else
                    {
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
                        rc = air_port_setPhyLedForcePattCfg(unit, port, led_id_en, _lp_port_led_patt);
#endif
                        rc |= air_port_setPhyLedForceState(unit, port, led_id_en, AIR_PORT_PHY_LED_STATE_FORCE_PATT);
                        if (rc != AIR_E_OK)
                        {
                            LP_LOG_ERROR("set port=%u led %u phyLedForceState failed", port, led_id_dis);
                            ret = MW_E_OP_INCOMPLETE;
                        }
                    }
                    if (FALSE == MW_PORT_CHK(_led_single_pbmp, port))
                    {
                        if ((AIR_E_OK == air_port_getPhyLedCtrlMode(unit, port, led_id_dis, &led_mode)) &&
                           (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == led_mode))
                        {
                            rc = air_port_setPhyLedCtrlMode(unit, port, led_id_dis, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
                            if (rc != AIR_E_OK)
                            {
                                LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_dis);
                            }
                        }
                    }
                }
            }
            else
            {
                ret = MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            led_id_dis = USE_LED0;
            rc = air_port_setPhyLedCtrlMode(unit, port, led_id_dis, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
            if (rc != AIR_E_OK)
            {
                LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_dis);
            }

            if (FALSE == MW_PORT_CHK(_led_single_pbmp, port))
            {
                led_id_dis = USE_LED1;
                rc = air_port_setPhyLedCtrlMode(unit, port, led_id_dis, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
                if (rc != AIR_E_OK)
                {
                    LP_LOG_ERROR("set port=%u led %u phyLedCtrlMode failed", port, led_id_dis);
                }
            }
        }
    }
    else
    {
        LP_LOG_ERROR("get port=%u link status failed", port);
        ret = MW_E_OP_INCOMPLETE;
    }

    return ret;
}

/* FUNCTION NAME: lp_led_checkRateChange
 * PURPOSE:
 *      Check if the speed of the port has changed.
 *
 * INPUT:
 *      chkPort          --  Port ID to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      MW_E_OK means change is detected, MW_E_OTHERS means no change.
 */
MW_ERROR_NO_T
lp_led_checkRateChange(
    UI16_T chkPort)
{
    AIR_PORT_STATUS_T ps;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T  ret = MW_E_OTHERS;

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(0, chkPort)) ||
        (TRUE == sfp_port_is_comboPort(0, chkPort)))
    {
        rc = sfp_port_getPortStatus(0, chkPort, &ps);
    }
    else
#endif
    {
        rc = air_port_getPortStatus(0, chkPort, &ps);
    }
    if (AIR_E_OK == rc)
    {
        if (AIR_PORT_SPEED_1000M == ps.speed)
        {
            if (0 == (_led_speed_pbmp & (0x01 << chkPort)))
            {
                ret = MW_E_OK;
            }
        }
        else
        {
            if (0 != (_led_speed_pbmp & (0x01 << chkPort)))
            {
                ret = MW_E_OK;
            }
        }
    }
    else
    {
        LP_LOG_ERROR("Get port %d port status failed !", chkPort);
    }

    return ret;
}

/* FUNCTION NAME: lp_led_state_check
 * PURPOSE:
 *      Check port LED state.
 *
 * INPUT:
 *      u_portId         --  Port ID to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_led_checkPortState(
    UI32_T u_portId)
{
    MW_ERROR_NO_T       rc = MW_E_OP_INCOMPLETE;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return rc;
    }

    if (AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, u_portId))
    {
        if ((TRUE == lp_port_getLinkStatus(u_portId)) && (TRUE == lp_led_checkRateChange(u_portId)))
        {
            LP_LOG_DEBUG("[LED] u_portId=%d speed change!", u_portId);
            lp_led_setBehavior(TRUE, u_portId);
        }
        else if (FALSE == lp_port_getLinkStatus(u_portId))
        {
            LP_LOG_DEBUG("[LED] u_portId=%d link down!", u_portId);
            AIR_PORT_DEL(ptr_lp_config->loop_led_pbmp, u_portId);
            lp_led_setBehavior(FALSE, u_portId);
        }
    }
    else
    {
        if (TRUE == lp_port_getLinkStatus(u_portId))
        {
            LP_LOG_DEBUG("[LED] u_portId=%d link up!", u_portId);
            if (MW_E_OK == lp_led_setBehavior(TRUE, u_portId))
            {
                AIR_PORT_ADD(ptr_lp_config->loop_led_pbmp, u_portId);
            }
        }
    }

    return rc;
}

#ifdef LP_GLOBAL_LED_SUPPORT
/* FUNCTION NAME: lp_led_updateGlobalLed
 * PURPOSE:
 *      Update global LED status based on the loop bitmap.
 *
 * INPUT:
 *      loop_pbmp        --  Loop bitmap indicating the ports in loop.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_led_updateGlobalLed(
    AIR_PORT_BITMAP_T loop_pbmp)
{
    UI32_T unit = 0;
    MW_ERROR_NO_T ret = MW_E_OK;

    if (MW_LED_GPIO_PIN_INVALID == _lp_global_led_pin)
    {
        return MW_E_BAD_PARAMETER;
    }
    if (AIR_PORT_BITMAP_EMPTY(loop_pbmp) != TRUE)
    {
        ret = air_perif_setGpioOutputAutoMode(unit, _lp_global_led_pin, TRUE);
        ret |= air_perif_setGpioOutputAutoPatt(unit, _lp_global_led_pin, _lp_global_led_patt);
    }
    else
    {
        ret = air_perif_setGpioOutputAutoMode(unit, _lp_global_led_pin, FALSE);
        if (MW_LED_ACTIVE_TYPE_LOW_ACTIVE == _lp_global_led_active_type)
        {
            ret |= air_perif_setGpioOutputData(unit, _lp_global_led_pin, AIR_PERIF_GPIO_DATA_HIGH);
        }
        else
        {
            ret |= air_perif_setGpioOutputData(unit, _lp_global_led_pin, AIR_PERIF_GPIO_DATA_LOW);
        }
    }

    return ret;
}

/* FUNCTION NAME: lp_led_initGlobalLed
 * PURPOSE:
 *      Init global LED status based on the loop bitmap.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_led_initGlobalLed(
    void)
{
    I32_T rc = MW_E_OTHERS;
#ifdef LP_CUSTOMER_CONFIG_SUPPORT
    MW_LP_LED_CFG_T lp_led_cfg = {0};

    rc = lp_getLpLedCfg(&lp_led_cfg);
    if (MW_E_OK != rc)
    {
        return MW_E_OTHERS;
    }

    if ((lp_led_cfg.global_led_pattern >= AIR_PORT_PHY_LED_PATT_HZ_HALF) &&
        (lp_led_cfg.global_led_pattern < AIR_PORT_PHY_LED_PATT_LAST))
    {
        _lp_global_led_patt = lp_led_cfg.global_led_pattern;
    }
    _lp_global_led_pin = lp_led_cfg.global_led_pin;
    _lp_global_led_active_type = lp_led_cfg.global_led_active_type;

    if (_lp_global_led_pin != MW_LED_GPIO_PIN_INVALID)
    {
        rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE + _lp_global_led_pin, AIR_CHIPSCU_IOMUX_ENABLE);
        rc |= air_gpio_setDirection(_lp_global_led_pin, AIR_GPIO_DIRECTION_OUTPUT);
        if (MW_LED_ACTIVE_TYPE_LOW_ACTIVE == _lp_global_led_active_type)
        {
            rc |= air_gpio_setValue(_lp_global_led_pin, GPIO_PIN_HIGH);
        }
        else
        {
            rc |= air_gpio_setValue(_lp_global_led_pin, GPIO_PIN_LOW);
        }
        rc |= air_gpio_setOutputEnable(_lp_global_led_pin, ENABLE_PIN_OE);
    }
#endif /* LP_CUSTOMER_CONFIG_SUPPORT */

    return (E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

#endif /* LP_GLOBAL_LED_SUPPORT */

/* FUNCTION NAME: lp_led_dumpInfo
 * PURPOSE:
 *      Dump LED information.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
lp_led_dumpInfo(
    void)
{
    MW_CMD_OUTPUT("LED 1G Map: %x, led_single_pbmp: 0x%08x, led_speed_pbmp: 0x%08x\n", _led_1g_pbmp, _led_single_pbmp[0], _led_speed_pbmp);
    return;
}