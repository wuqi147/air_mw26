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
_customer_system_init_an8858e_8p(
    void);

static int
_customer_system_post_init_an8858e_8p(
    void);

static int
_customer_system_init_an8858c_8p_2sfp(
    void);

static int
_customer_system_post_init_an8858c_8p_2sfp(
    void);

static int
_customer_system_init_an8858c_8p_2sfp_a(
    void);

static int
_customer_system_post_init_an8858c_8p_2sfp_a(
    void);

static int
_customer_system_post_init_an8858b_6p_2sfp(
    void);

static int
_customer_system_init_an8858b_6p_2sfp(
    void);

static int
_customer_system_post_init_an8858b_6p_2sfp_a(
    void);

static int
_customer_system_init_an8858b_6p_2sfp_a(
    void);

static int
_customer_system_post_init_an8858h_8p_an8811b_1p(
    void);

static int
_customer_system_init_an8858h_8p_an8811b_1p(
    void);

/*STATIC VARIABLE DECLARATIONS
*/
static const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T _customer_system_init_funcs_map[]=
{
    {MW_PRODUCT_ID_AN8858E_8P, _customer_system_init_an8858e_8p, _customer_system_post_init_an8858e_8p},
    {MW_PRODUCT_ID_AN8858E_8P_AN8503_8P, _customer_system_init_an8858e_8p, _customer_system_post_init_an8858e_8p},
    {MW_PRODUCT_ID_AN8858C_8P_2SFP, _customer_system_init_an8858c_8p_2sfp, _customer_system_post_init_an8858c_8p_2sfp},
    {MW_PRODUCT_ID_AN8858C_8P_2SFP_A, _customer_system_init_an8858c_8p_2sfp_a, _customer_system_post_init_an8858c_8p_2sfp_a},
    {MW_PRODUCT_ID_AN8858B_6P_2SFP, _customer_system_init_an8858b_6p_2sfp, _customer_system_post_init_an8858b_6p_2sfp},
    {MW_PRODUCT_ID_AN8858B_6P_2SFP_A, _customer_system_init_an8858b_6p_2sfp_a, _customer_system_post_init_an8858b_6p_2sfp_a},
    {MW_PRODUCT_ID_AN8858H_8P_AN8811B_1P, _customer_system_init_an8858h_8p_an8811b_1p, _customer_system_post_init_an8858h_8p_an8811b_1p},
};

/*LOCAL SUBPROGRAM BODIES
*/
static int
_customer_system_init_an8858e_8p(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858e_8p(
    void)
{
    int rc = E_OK;

    return rc;
}

static int
_customer_system_init_an8858c_8p_2sfp(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    /* GPIO2: SLED_CLR */
    rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN2, rc);
    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858c_8p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_init_an8858c_8p_2sfp_a(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    /* GPIO2: SLED_CLR */
    rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN2, rc);
    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858c_8p_2sfp_a(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_init_an8858b_6p_2sfp(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    /* GPIO2: SLED_CLR */
    rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN2, rc);
    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858b_6p_2sfp_a(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_init_an8858b_6p_2sfp_a(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    /* GPIO2: SLED_CLR */
    rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN2, rc);
    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858b_6p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_init_an8858h_8p_an8811b_1p(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    /* GPIO2: SLED_CLR */
    rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
    /* GPIO9: AN8811B reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN2, rc);
    SET_GPIO_OUTPUT(GPIO_PIN9, rc);
    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN9, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN9, GPIO_PIN_HIGH);

    mw_ledapp_sysled_init(0, GPIO_PIN17, MW_LED_ACTIVE_TYPE_HIGH_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_an8858h_8p_an8811b_1p(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
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
    I32_T rc = E_ENTRY_NOT_FOUND;
    UI32_T unit = 0;
    const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T *ptr_funcs = _customer_system_get_init_funcs();
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_LAST;
    MW_LEDAPP_PORTLED_SELFTEST_HOOK ptr_sysled_manual_flash = NULL;

    if (NULL != ptr_funcs)
    {
        rc = ptr_funcs->post_init_func();
    }

    mw_ledapp_sysled_get_info(NULL, NULL, &flash_type);

    if (MW_LEDAPP_SYSLED_FLASH_TYPE_AUTO_FLASH == flash_type)
    {
        mw_ledapp_sysled_enable_auto_flash(unit);
    }
    else if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        ptr_sysled_manual_flash = mw_ledapp_sysled_trigger_manual_flash;
    }

    mw_ledapp_portled_selftest(unit, ptr_sysled_manual_flash);

    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_NONE <= flash_type) && (MW_LEDAPP_SYSLED_FLASH_TYPE_LAST > flash_type))
    {
        mw_ledapp_sysled_set_state(unit, AIR_PORT_PHY_LED_STATE_ON);
    }

    return rc;
}

