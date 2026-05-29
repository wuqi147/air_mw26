/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2024
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

/* FILE NAME:  customer_system.c
 * PURPOSE:
 *  Specify customer system configuration.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include "customer_system.h"
#include "hwcfg_util.h"
#include "timer.h"
#include "util.h"
#include "air_chipscu.h"
#include "air_gpio.h"
#include "air_error.h"
#include "air_init.h"
#include "air_port.h"
#include "air_cfg.h"
#include "air_perif.h"
#include <platform.h>
#include "mw_led.h"
#include "mw_led_app.h"
#include "btn_reset.h"
#include "osapi_string.h"
#include "product_ref.h"
#include "customer_ref.h"
#include "default_config.h"
#include "sfp_util.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
*/

/*DATA TYPE DECLARATIONS
*/
typedef int (* CUSTOMER_SYSTEM_INIT_FUNC_T)(void);
typedef int (* CUSTOMER_SYSTEM_POST_INIT_FUNC_T)(void);

typedef struct
{
    UI8_T product_id;
    CUSTOMER_SYSTEM_INIT_FUNC_T init_func;
    CUSTOMER_SYSTEM_POST_INIT_FUNC_T post_init_func;
}CUSTOMER_SYSTEM_INIT_FUNC_MAP_T;

/* MACRO FUNCTION DECLARATIONS
*/
#define SET_GPIO_OUTPUT(x, rc)    do                              \
    {                                                             \
        rc |= air_gpio_setValue(x, GPIO_PIN_HIGH);                \
        rc |= air_gpio_setOutputEnable(x, ENABLE_PIN_OE);         \
    }while(0)

/*GLOBAL VARIABLE DECLARATIONS
*/
/*LOCAL SUBPROGRAM DECLARATIONS
*/
static int
_customer_system_init_an8855m_5p(
    void);

static int
_customer_system_post_init_an8855m_5p(
    void);

static int
_customer_system_init_an8855m_5p_1sfp(
    void);

static int
_customer_system_post_init_an8855m_5p_1sfp(
    void);

static int
_customer_system_init_an8855m_5p_an8801sb_1p(
    void);

static int
_customer_system_post_init_an8855m_5p_an8801sb_1p(
    void);

/*STATIC VARIABLE DECLARATIONS
*/
static const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T _customer_system_init_funcs_map[]=
{
    {MW_PRODUCT_ID_AN8855M_5P, _customer_system_init_an8855m_5p, _customer_system_post_init_an8855m_5p},
    {MW_PRODUCT_ID_AN8855M_5P_1SFP, _customer_system_init_an8855m_5p_1sfp, _customer_system_post_init_an8855m_5p_1sfp},
    {MW_PRODUCT_ID_AN8855M_5P_1SFP_A, _customer_system_init_an8855m_5p_1sfp, _customer_system_post_init_an8855m_5p_1sfp},
    {MW_PRODUCT_ID_AN8855M_5P_AN8801SB_1P, _customer_system_init_an8855m_5p_an8801sb_1p, _customer_system_post_init_an8855m_5p_an8801sb_1p},
    {MW_PRODUCT_ID_AN8855M_5P_AN8502_4P, _customer_system_init_an8855m_5p_an8801sb_1p, _customer_system_post_init_an8855m_5p_an8801sb_1p},
};

/*LOCAL SUBPROGRAM BODIES
*/
static int
_customer_system_init_an8855m_5p(
    void)
{
    int rc = E_OK;

    return rc;
}

static int
_customer_system_post_init_an8855m_5p(
    void)
{
    int rc = E_OK;

    return rc;
}

static int
_customer_system_init_an8855m_5p_1sfp(
    void)
{
    int rc = E_OK;

#ifdef AIR_SUPPORT_SFP
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO6_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_gpio_setDirection(GPIO_PIN6, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN6, rc);
    rc |= air_gpio_setValue(GPIO_PIN6, GPIO_PIN_LOW);
#endif

    return rc;
}

static int
_customer_system_post_init_an8855m_5p_1sfp(
    void)
{
    int rc = E_OK;

#ifdef AIR_SUPPORT_SFP
    rc |= air_gpio_setValue(GPIO_PIN6, GPIO_PIN_HIGH);
#endif

    return rc;
}

static int
_customer_system_init_an8855m_5p_an8801sb_1p(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
    return rc;
}

static int
_customer_system_post_init_an8855m_5p_an8801sb_1p(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);
    return rc;
}

/*FUNCTION NAME: _customer_system_get_init_funcs
* PURPOSE:
*       Get the configuring item based on the product ID.
* INPUT:
*       none
* OUTPUT:
*       none
* RETURN:
*       NULL -- having not found the port configuring item
*       NOT A NULL -- return the pointer which point to the configuring item which of module ID matched the module ID inputed
* NOTES:
*      None
*/
const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T *
_customer_system_get_init_funcs(
    void)
{
    UI8_T product_id = mw_get_productID();
    UI32_T i = 0, count = sizeof(_customer_system_init_funcs_map) / sizeof(CUSTOMER_SYSTEM_INIT_FUNC_MAP_T);

    for(; i < count; i++)
    {
        if (_customer_system_init_funcs_map[i].product_id == product_id)
        {
            return &(_customer_system_init_funcs_map[i]);
        }
    }

    return NULL;
}

/* FUNCTION NAME:   _customer_system_set_port_led
 * PURPOSE:
 *      For the led(s) of a specific port, if state is ON, set the led control
 *      mode to force and set the force state to ON. If state is OFF, set the
 *      force state to OFF and set the led control mode to phy.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      phy_led_count           -- The number of the leds for a PHY port or a
 *                                 combo port in PHY mode
 *      sfp_led_count           -- The number of the leds for a SERDES port or a
 *                                 combo port in SERDES mode
 *      state                   -- The LED force state to set
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
UI32_T
_customer_system_set_port_led(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T phy_led_count,
    const UI32_T sfp_led_count,
    const AIR_PORT_PHY_LED_STATE_T state)
{
    I32_T rc = AIR_E_OK, i = 0;
#ifdef AIR_SUPPORT_SFP
    UI32_T led_count = 0;
    AIR_PORT_COMBO_MODE_T old_combo_mode = AIR_PORT_COMBO_MODE_LAST, new_combo_mode = AIR_PORT_COMBO_MODE_LAST;

    if ((0 == phy_led_count) && (0 == sfp_led_count))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        rc = air_port_getComboMode(unit, port, &old_combo_mode);
        if ((TRUE == sfp_port_is_pureComboSerdesPort(unit, port)) &&
            (AIR_PORT_COMBO_MODE_SERDES != old_combo_mode))
        {
            rc |= air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_SERDES);
            old_combo_mode = AIR_PORT_COMBO_MODE_SERDES;
        }

        if (AIR_E_OK != rc)
        {
            return rc;
        }
    }

    if (((FALSE == sfp_port_is_serdesPort(unit, port)) &&
          (FALSE == sfp_port_is_comboPort(unit, port))) ||
        (AIR_PORT_COMBO_MODE_PHY == old_combo_mode))
    {
        /* Set PHY LED: PHY port + combo PHY port */
        for (i = 0; i < phy_led_count; i++)
        {
            if (AIR_PORT_PHY_LED_STATE_ON == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
            }
            rc |= air_port_setPhyLedForceState(unit, port, i, state);
            if (AIR_PORT_PHY_LED_STATE_OFF == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
            }
        }
    }
    else
    {
        /* Set SFP LED: Serdes port + combo serdes port + pure combo serdes port */
        for (i = 0; i < sfp_led_count; i++)
        {
            if (AIR_PORT_PHY_LED_STATE_ON == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
            }
            rc |= air_port_setPhyLedForceState(unit, port, i, state);
            if (AIR_PORT_PHY_LED_STATE_OFF == state)
            {
                rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
            }
        }
    }

    if ((TRUE == sfp_port_is_comboPort(unit, port)) &&
        (FALSE == sfp_port_is_pureComboSerdesPort(unit, port)))
    {
        /* combo PHY port or combo serdes port */
        new_combo_mode = (AIR_PORT_COMBO_MODE_SERDES == old_combo_mode) ? AIR_PORT_COMBO_MODE_PHY : AIR_PORT_COMBO_MODE_SERDES;
        led_count = (AIR_PORT_COMBO_MODE_SERDES == old_combo_mode) ? phy_led_count : sfp_led_count;

        if (0 != led_count)
        {
            rc |= air_port_setComboMode(unit, port, new_combo_mode);

            for (i = 0; i < led_count; i++)
            {
                if (AIR_PORT_PHY_LED_STATE_ON == state)
                {
                    rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
                }
                rc |= air_port_setPhyLedForceState(unit, port, i, state);
                if (AIR_PORT_PHY_LED_STATE_OFF == state)
                {
                    rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
                }
            }

            rc |= air_port_setComboMode(unit, port, old_combo_mode);
        }
    }
#else
    for (i = 0; i < phy_led_count; i++)
    {
        if (AIR_PORT_PHY_LED_STATE_ON == state)
        {
            rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_FORCE);
        }
        rc |= air_port_setPhyLedForceState(unit, port, i, state);
        if (AIR_PORT_PHY_LED_STATE_OFF == state)
        {
            /* PHY port */
            rc |= air_port_setPhyLedCtrlMode(unit, port, i, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
        }
    }
#endif

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
*/
/*FUNCTION NAME: customer_system_init
* PURPOSE:
*        the API offered to main.cpp call for initializing system by customer's configuration
* INPUT:
*       none
* OUTPUT:
*       none
* RETURN:
*       E_OK --Initializing successfully
*       OTHERS -- Initializing fail
* NOTES:
*      None
*/
int
customer_system_init(
    void)
{
    int rc = E_ENTRY_NOT_FOUND;
    const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T *ptr_funcs = _customer_system_get_init_funcs();

    if (NULL != ptr_funcs)
    {
        rc = ptr_funcs->init_func();
    }

    return rc;
}

/*FUNCTION NAME: customer_system_post_init
* PURPOSE:
*        the API offered to main.cpp call for doing some thing of after initializing system
* INPUT:
*       none
* OUTPUT:
*       none
* RETURN:
*       E_OK -- post-initializing successfully
*       OTHERS -- post-initializing fail
* NOTES:
*      None
*/
int
customer_system_post_init(
    void)
{
    I32_T rc = E_ENTRY_NOT_FOUND, ret = E_ENTRY_NOT_FOUND;
    UI32_T total_port_num = 0, unit = 0;
#ifdef AIR_SUPPORT_SFP
    UI32_T port = 0;
#endif
    const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T *ptr_funcs = _customer_system_get_init_funcs();
    AIR_INIT_PORT_MAP_T *ptr_portMapList = NULL;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_LAST;
    MW_LEDAPP_PORTLED_SELFTEST_HOOK ptr_sysled_manual_flash = NULL;

    if (NULL != ptr_funcs)
    {
        rc = ptr_funcs->post_init_func();
    }

    osapi_calloc((sizeof(AIR_INIT_PORT_MAP_T) * (MAX_PORT_NUM + 1)), "CUSTSYS", (void**)&ptr_portMapList);
    if (NULL != ptr_portMapList)
    {
        ret = air_init_getSdkPortMap(unit, &total_port_num, ptr_portMapList);
        if (AIR_E_OK != ret)
        {
            total_port_num = 0;
            MW_LOG_CONSOLE_PRINTF("[WARN]air_init_getSdkPortMap fail. rc:%d\n", ret);

        }
        MW_FREE(ptr_portMapList);
    }

#ifdef AIR_SUPPORT_SFP
    for (port = 0; port < total_port_num; port++)
    {
        if (TRUE == sfp_port_is_serdesPort(unit, port))
        {
            sfp_writeReg(unit, port, 0xE400, 0x60000);
        }
    }
#endif

    mw_ledapp_sysled_get_info(NULL, NULL, &flash_type);

    if (MW_LEDAPP_SYSLED_FLASH_TYPE_AUTO_FLASH == flash_type)
    {
        mw_ledapp_sysled_enable_auto_flash(unit);
    }
    else if (MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH == flash_type)
    {
        ptr_sysled_manual_flash = mw_ledapp_sysled_trigger_manual_flash;
    }

    ret = mw_ledapp_portled_selftest(unit, ptr_sysled_manual_flash);
    if (MW_E_OK != ret)
    {
        if (MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH == flash_type)
        {
            mw_ledapp_sysled_trigger_manual_flash(unit);
        }
        delay1ms(500); /* Delay for SFP REG settings. */
        if (MW_LEDAPP_SYSLED_FLASH_TYPE_MANUAL_FLASH == flash_type)
        {
            mw_ledapp_sysled_trigger_manual_flash(unit);
        }
    }

    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_NONE <= flash_type) && (MW_LEDAPP_SYSLED_FLASH_TYPE_LAST > flash_type))
    {
        mw_ledapp_sysled_set_state(unit, AIR_PORT_PHY_LED_STATE_ON);
    }

    return rc;
}

