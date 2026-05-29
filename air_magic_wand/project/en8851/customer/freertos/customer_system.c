/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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
_customer_system_init_en8851c_rfb(
    void);

static int
_customer_system_post_init_en8851c_rfb(
    void);

static int
_customer_system_init_en8853c_rfb(
    void);

static int
_customer_system_post_init_en8853c_rfb(
    void);

static int
_customer_system_init_en8853c_an8808q_rfb(
    void);

static int
_customer_system_post_init_en8853c_an8808q_rfb(
    void);

static int
_customer_system_init_en8851c_8p_2sfp(
    void);

static int
_customer_system_post_init_en8851c_8p_2sfp(
    void);

static int
_customer_system_init_en8853c_24p_en8804_4p_4sfp(
    void);

static int
_customer_system_post_init_en8853c_24p_en8804_4p_4sfp(
    void);

static int
_customer_system_init_en8853c_24p_1sfp(
    void);

static int
_customer_system_post_init_en8853c_24p_1sfp(
    void);

static int
_customer_system_init_en8853c_8p_en8804_4sfp(
    void);

static int
_customer_system_post_init_en8853c_8p_en8804_4sfp(
    void);

static int
_customer_system_init_en8853c_8p_en8804_8sfp(
    void);

static int
_customer_system_post_init_en8853c_8p_en8804_8sfp(
    void);

static int
_customer_system_init_en8853c_16p_2sfp(
    void);

static int
_customer_system_post_init_en8853c_16p_2sfp(
    void);

/*STATIC VARIABLE DECLARATIONS
*/
static const CUSTOMER_SYSTEM_INIT_FUNC_MAP_T _customer_system_init_funcs_map[] =
{
    {MW_PRODUCT_ID_EN8851C_8P, _customer_system_init_en8851c_rfb, _customer_system_post_init_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_RFB, _customer_system_init_en8851c_rfb, _customer_system_post_init_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8502_4P, _customer_system_init_en8851c_rfb, _customer_system_post_init_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8503_8P, _customer_system_init_en8851c_rfb, _customer_system_post_init_en8851c_rfb},
    {MW_PRODUCT_ID_EN8853C_RFB, _customer_system_init_en8853c_rfb, _customer_system_post_init_en8853c_rfb},
    {MW_PRODUCT_ID_EN8853C_AN8808Q_RFB, _customer_system_init_en8853c_an8808q_rfb, _customer_system_post_init_en8853c_an8808q_rfb},

    {MW_PRODUCT_ID_EN8851C_8P_2SFP, _customer_system_init_en8851c_8p_2sfp, _customer_system_post_init_en8851c_8p_2sfp},
    {MW_PRODUCT_ID_EN8851C_8P_2SFP_L, _customer_system_init_en8851c_8p_2sfp, _customer_system_post_init_en8851c_8p_2sfp},
    {MW_PRODUCT_ID_EN8853C_24P_EN8804_4P_4SFP, _customer_system_init_en8853c_24p_en8804_4p_4sfp, _customer_system_post_init_en8853c_24p_en8804_4p_4sfp},
    {MW_PRODUCT_ID_EN8853C_24P_1SFP, _customer_system_init_en8853c_24p_1sfp, _customer_system_post_init_en8853c_24p_1sfp},
    {MW_PRODUCT_ID_EN8853C_8P_EN8804_4SFP, _customer_system_init_en8853c_8p_en8804_4sfp, _customer_system_post_init_en8853c_8p_en8804_4sfp},
    {MW_PRODUCT_ID_EN8853C_8P_EN8804_8SFP, _customer_system_init_en8853c_8p_en8804_8sfp, _customer_system_post_init_en8853c_8p_en8804_8sfp},
    {MW_PRODUCT_ID_EN8853C_16P_2SFP, _customer_system_init_en8853c_16p_2sfp, _customer_system_post_init_en8853c_16p_2sfp},
};

/*LOCAL SUBPROGRAM BODIES
*/
static int
_customer_system_init_en8851c_rfb(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO6_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    /* Output pin */
    /* GPIO0 :  SYSTEM LED */
    /* GPIO6 :  SGMII-0 LED */
    /* GPIO8 :  SGMII-1 LED */
    /* GPIO12:  RESET LED */
    /* GPIO13:  TX-DIS-0 */
    /* GPIO16:  TX-DIS-1 */
    rc |= air_gpio_setDirection(GPIO_PIN6, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN13, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN16, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN6, rc);
    SET_GPIO_OUTPUT(GPIO_PIN8, rc);
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN13, rc);
    SET_GPIO_OUTPUT(GPIO_PIN16, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    /* Input pin */
    /* GPIO9 :  SW-Reset */
    /* GPIO11:  MOD-ABS-0 */
    /* GPIO15:  MOD-ABS-1 */
    /* GPIO10:  RX-LOS-0 */
    /* GPIO17:  RX-LOS-1 */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN15, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN10, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN17, AIR_GPIO_DIRECTION_INPUT);
    /**/
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);

    mw_ledapp_sysled_init(0, GPIO_PIN0, MW_LED_ACTIVE_TYPE_LOW_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_en8851c_rfb(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_init_en8853c_rfb(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO7_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    /* EN8808/8804 reset: GPIO12, 1, 7 */
    /* LED clear control: GPIO4 high to release clear */
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN7, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    /**/
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN7, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    /* Input pin */
    /* GPIO9 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);
    /**/
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);    /* LED off */
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_HIGH);

    mw_ledapp_sysled_init(0, GPIO_PIN0, MW_LED_ACTIVE_TYPE_LOW_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_en8853c_rfb(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);    /* LED enable */
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_an8808q_rfb(
    void)
{
    int rc = E_OK;
    MW_LEDAPP_SYSLED_FLASH_TYPE_T flash_type = MW_LEDAPP_SYSLED_FLASH_TYPE_NONE;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO7_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_P_MDIO_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    /* AN8808Q/EN8804 reset: GPIO12, 1, 7 */
    /* LED clear control: GPIO16 high to release clear */
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN7, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN16, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN7, rc);
    SET_GPIO_OUTPUT(GPIO_PIN16, rc);

    /* Input pin */
    /* GPIO9 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);

    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN16, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_HIGH);

    mw_ledapp_sysled_init(0, GPIO_PIN0, MW_LED_ACTIVE_TYPE_LOW_ACTIVE, flash_type);
    if (MW_LEDAPP_SYSLED_FLASH_TYPE_NONE == flash_type)
    {
        mw_ledapp_sysled_set_state(0, AIR_PORT_PHY_LED_STATE_ON);
    }
    return rc;
}

static int
_customer_system_post_init_en8853c_an8808q_rfb(
    void)
{
    int                 rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN16, GPIO_PIN_HIGH);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8851c_8p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO6_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    /* Output pin */
    /* GPIO0 :  SYSTEM LED */
    /* GPIO6 :  SGMII-0 LED */
    /* GPIO8 :  SGMII-1 LED */
    /* GPIO12:  RESET LED */
    /* GPIO13:  TX-DIS-0 */
    /* GPIO16:  TX-DIS-1 */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN6, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN13, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN16, AIR_GPIO_DIRECTION_OUTPUT);
    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN6, rc);
    SET_GPIO_OUTPUT(GPIO_PIN8, rc);
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN13, rc);
    SET_GPIO_OUTPUT(GPIO_PIN16, rc);
    /* Input pin */
    /* GPIO9 :  SW-Reset */
    /* GPIO11:  MOD-ABS-0 */
    /* GPIO15:  MOD-ABS-1 */
    /* GPIO10:  RX-LOS-0 */
    /* GPIO17:  RX-LOS-1 */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN15, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN10, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN17, AIR_GPIO_DIRECTION_INPUT);
    /**/
    rc |= air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    return rc;
}

static int
_customer_system_post_init_en8851c_8p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_24p_en8804_4p_4sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO7_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    /* EN8808/8804 reset: GPIO12, 1, 7 */
    /* LED clear control: GPIO4 high to release clear */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN7, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    /**/
    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN7, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    /* Input pin */
    /* GPIO8 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);
    /**/
    rc |= air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_post_init_en8853c_24p_en8804_4p_4sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);
    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_24p_1sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    /* EN8808 reset: GPIO12, 1 */
    /* LED clear control: GPIO4 high to release clear */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN8, rc);

    /* Input pin */
    /* GPIO9 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN15, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN13, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN17, AIR_GPIO_DIRECTION_INPUT);


    rc |= air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);

    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_post_init_en8853c_24p_1sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);

    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_HIGH);
    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_8p_en8804_4sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    //rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN10, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN8, rc);
    SET_GPIO_OUTPUT(GPIO_PIN10, rc);

    /* Input pin */
    /* GPIO9 :  SW-Reset */
    //rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);

    rc |= air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN10, GPIO_PIN_LOW);

    delay1ms(5);

    return rc;
}

static int
_customer_system_post_init_en8853c_8p_en8804_4sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);

    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    //btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_8p_en8804_8sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO7_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    //rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    /* EN8804 reset: GPIO7 */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN10, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN7, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN8, rc);
    SET_GPIO_OUTPUT(GPIO_PIN10, rc);
    SET_GPIO_OUTPUT(GPIO_PIN7, rc);

    /* Input pin */
    /* GPIO9 :  SW-Reset */
    //rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN10, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN7, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);

    return rc;
}

static int
_customer_system_post_init_en8853c_8p_en8804_8sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);

    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    //btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
    return rc;
}

static int
_customer_system_init_en8853c_16p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO6_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    /* EN8808 reset: GPIO12, 1 */
    /* LED clear control: GPIO4 high to release clear */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN12, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN1, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN6, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN8, AIR_GPIO_DIRECTION_OUTPUT);

    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    air_gpio_setValue(GPIO_PIN0, GPIO_PIN_LOW);
    SET_GPIO_OUTPUT(GPIO_PIN12, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN1, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);

    /* Input pin */
    /* GPIO9 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN10, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN13, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN17, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN16, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN15, AIR_GPIO_DIRECTION_INPUT);

    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);

    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN6, GPIO_PIN_LOW);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_LOW);
    delay1ms(5);
    rc |= air_gpio_setValue(GPIO_PIN12, GPIO_PIN_HIGH);
    return rc;
}

static int
_customer_system_post_init_en8853c_16p_2sfp(
    void)
{
    int rc = E_OK;

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);

    rc |= air_gpio_setValue(GPIO_PIN1, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN5, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN6, GPIO_PIN_HIGH);
    rc |= air_gpio_setValue(GPIO_PIN8, GPIO_PIN_HIGH);

    air_perif_setGpioOutputAutoMode(0, GPIO_PIN0, FALSE);
    btn_reset_isr_init(GPIO_PIN9, GPIO_BOTH_EDG_DETECT);
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
    UI8_T product_id = mw_get_productID();

    if (NULL != ptr_funcs)
    {
        rc = ptr_funcs->init_func();

        if ((E_OK == rc) && (((MW_PRODUCT_ID_EN8853C_8P_EN8804_4SFP == product_id) ||
            (MW_PRODUCT_ID_EN8853C_8P_EN8804_8SFP == product_id))))
        {
            unsigned int i = 0;

            rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO2_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
            rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO3_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

            rc |= air_gpio_setDirection(GPIO_PIN2, AIR_GPIO_DIRECTION_OUTPUT);
            rc |= air_gpio_setDirection(GPIO_PIN3, AIR_GPIO_DIRECTION_OUTPUT);

            SET_GPIO_OUTPUT(GPIO_PIN2, rc);
            SET_GPIO_OUTPUT(GPIO_PIN3, rc);
            rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);
            rc |= air_gpio_setValue(GPIO_PIN3, GPIO_PIN_LOW);
            delay1ms(10);

            rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);
            for (i = 0; i < 16; i++)
            {
                if (0 == (i % 2))
                {
                    rc |= air_gpio_setValue(GPIO_PIN3, GPIO_PIN_LOW);
                }
                else
                {
                    rc |= air_gpio_setValue(GPIO_PIN3, GPIO_PIN_HIGH);
                }
                delay1ms(1);
                rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_HIGH);
                rc |= air_gpio_setValue(GPIO_PIN2, GPIO_PIN_LOW);
            }

            delay1ms(500);

            rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO2_MODE, 0);
            rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO3_MODE, 0);

            rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
            /* Do not change the return value of ptr_funcs->init_func(). */
            rc = E_OK;
        }
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
        if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
            (TRUE == sfp_port_is_comboPort(unit, port)))
        {
            sfp_writeReg(unit, port, 0xAF04, 0x203C);
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

#ifdef AIR_SUPPORT_SFP
    for (port = 0; port < total_port_num; port++)
    {
        if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
            (TRUE == sfp_port_is_comboPort(unit, port)))
        {
            sfp_writeReg(unit, port, 0xAF04, 0x3C);
        }
    }
#endif

    if ((MW_LEDAPP_SYSLED_FLASH_TYPE_NONE <= flash_type) && (MW_LEDAPP_SYSLED_FLASH_TYPE_LAST > flash_type))
    {
        mw_ledapp_sysled_set_state(unit, AIR_PORT_PHY_LED_STATE_ON);
    }

    return rc;
}

