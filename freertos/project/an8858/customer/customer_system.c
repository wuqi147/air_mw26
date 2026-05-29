/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
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
#include "util.h"
#include "air_chipscu.h"
#include "air_gpio.h"
#include "hwcfg_util.h"

/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/
#define SET_GPIO_OUTPUT(x, rc)    do                            \
    {                                                           \
        rc |= air_gpio_setValue(x, GPIO_PIN_HIGH);              \
        rc |= air_gpio_setOutputEnable(x, ENABLE_PIN_OE);       \
    }while(0)

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* EXPORTED SUBPROGRAM BODIES*/
int customer_system_init(void)
{
    int rc = E_OK;
	
#if 0
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    c |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE, AIR_CHIPSCU_IOMUX_ENABLE);
    rc |= air_chipscu_setIomuxFuncState(0, AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE, AIR_CHIPSCU_IOMUX_ENABLE);

    /* Output pin */
    /* GPIO0 :  SGMII-0 LED */
    /* GPIO5 :  SGMII-1 LED */
    /* GPIO4 :  RESET LED */
    /* GPIO11:  TX-DIS-1 */
    rc |= air_gpio_setDirection(GPIO_PIN0, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN5, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN4, AIR_GPIO_DIRECTION_OUTPUT);
    rc |= air_gpio_setDirection(GPIO_PIN11, AIR_GPIO_DIRECTION_OUTPUT);
    /* Input pin */
    /* GPIO9 :  SW-Reset */
    rc |= air_gpio_setDirection(GPIO_PIN9, AIR_GPIO_DIRECTION_INPUT);
    rc |= air_gpio_setDirection(GPIO_PIN16, AIR_GPIO_DIRECTION_INPUT);

    SET_GPIO_OUTPUT(GPIO_PIN0, rc);
    SET_GPIO_OUTPUT(GPIO_PIN5, rc);
    SET_GPIO_OUTPUT(GPIO_PIN4, rc);
    SET_GPIO_OUTPUT(GPIO_PIN11, rc);

    rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_LOW);
#endif
    return rc;
}

int customer_system_post_init(void)
{
    int rc = E_OK;

    //rc |= air_gpio_setValue(GPIO_PIN4, GPIO_PIN_HIGH);
    return rc;
}
