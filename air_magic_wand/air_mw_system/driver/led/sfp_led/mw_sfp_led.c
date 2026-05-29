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

/* FILE NAME:  mw_sfp_led.c
 * PURPOSE:
 *      It provide SFP LED application API in MW.
 * NOTES:
 */

#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
/* INCLUDE FILE DECLARATIONS
 */
#include "air_port.h"
#include "sfp_util.h"
#include "sfp_port.h"
#include "mw_utils.h"
#include "mw_led.h"
#include "mw_sfp_led.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static MW_SFP_LED_PORT_MAP_T *ptr_sfpLed_portMap = NULL;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: mw_sfpLed_port_getPortStatus
 *
 * PURPOSE:
 *      Get sfp port status.
 * INPUT:
 *      unit                --  Device unit number
 *      port                --  Index of port number
 * OUTPUT:
 *      ptr_status          --  Strcut of the port status
 *                              SFP_LED_PORT_STATUS_T
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
mw_sfpLed_getPortStatus(
    const UI32_T            unit,
    const UI32_T            port,
    SFP_LED_PORT_STATUS_T   *ptr_status)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    AIR_PORT_STATUS_T ps;

    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboPort(unit, port)))
    {
        rc = sfp_port_getPortStatus(unit, port, &ps);
    }
    else
    {
        rc = air_port_getPortStatus(unit, port, &ps);
    }

    if (AIR_E_OK == rc)
    {
        ptr_status->speed = ps.speed;
        ptr_status->flags = ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP;
        if (AIR_PORT_DUPLEX_FULL == ps.duplex)
        {
            ptr_status->flags |= SFP_LED_PORT_STATUS_FLAGS_DUPLEX_FULL;
        }
    }

    return rc;
}

void
mw_sfpLed_setPortMap(
    const MW_SFP_LED_PORT_MAP_T *ptr_portMap)
{
    ptr_sfpLed_portMap = (MW_SFP_LED_PORT_MAP_T *)ptr_portMap;
}

const UI32_T
mw_sfpLed_getLedCount(
    void)
{
    MW_SFP_LED_PORT_MAP_T *ptr_portMap = ptr_sfpLed_portMap;
    UI32_T i = 0, led_count = 1;

    if ((NULL == ptr_portMap) || (0 == ptr_portMap->count))
    {
        return 0;
    }

    for (i = 1; i < ptr_portMap->count; i++)
    {
        if (ptr_portMap->ptr_portMap[0].port == ptr_portMap->ptr_portMap[i].port)
        {
            led_count++;
        }
    }

    return led_count;
}

MW_ERROR_NO_T
mw_sfpLed_getPortPin(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T led_id,
    UI8_T *ptr_pin)
{
    MW_SFP_LED_PORT_MAP_T *ptr_portMap = ptr_sfpLed_portMap;
    UI32_T i = 0, led_count = 0;

    if ((NULL == ptr_portMap) || (0 == ptr_portMap->count))
    {
        return MW_E_BAD_PARAMETER;
    }

    *ptr_pin = MW_LED_GPIO_PIN_INVALID;
    for (i = 0; i < ptr_portMap->count; i++)
    {
        if (port == ptr_portMap->ptr_portMap[i].port)
        {
            if (led_id == led_count)
            {
                *ptr_pin = ptr_portMap->ptr_portMap[i].gpio_pin_id;
                return MW_E_OK;
            }
            led_count++;
        }
    }

    return MW_E_ENTRY_NOT_FOUND;
}

MW_ERROR_NO_T
mw_sfpLed_getPortActiveType(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T led_id,
    MW_LED_ACTIVE_TYPE_T *ptr_active_type)
{
    const MW_LED_CFG_T *ptr_ledCfg = NULL;
    UI8_T cfg_count = 0, i = 0;

    /* SERDES port */
    if (TRUE == sfp_port_is_serdesPort(unit, port))
    {
        mw_led_getLedCfg(&ptr_ledCfg, &cfg_count);
        if ((NULL == ptr_ledCfg) || (0 == cfg_count))
        {
            return MW_E_NOT_INITED;
        }

        for (i = 0; i < cfg_count; i++)
        {
            if ((AIR_CFG_TYPE_PHY_LED_BEHAVIOR == ptr_ledCfg[i].cfg_type) &&
                (port == ptr_ledCfg[i].param0) &&
                (led_id == ptr_ledCfg[i].param1))
            {
                *ptr_active_type = (TRUE == BIT_CHK(ptr_ledCfg[i].value, 11)) ? MW_LED_ACTIVE_TYPE_HIGH_ACTIVE : MW_LED_ACTIVE_TYPE_LOW_ACTIVE;
                return MW_E_OK;
            }
        }

        return MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        return MW_E_NOT_SUPPORT;
    }
}

#endif /* defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED) */
