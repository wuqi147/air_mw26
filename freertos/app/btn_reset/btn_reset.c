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

/* FILE NAME:  btn_reset.c
 * PURPOSE:
 *    It provide customer reset button functionality.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <pp_def.h>
#include "air_types.h"
#include "btn_reset.h"

#include "util.h"
#include "task.h"
#include "timer.h"
#include "air_gpio.h"
#include "air_chipscu.h"
#include "platform.h"
#include "spinorwrite.h"
#ifdef AIR_MW_SUPPORT
#include "web.h"
#include "mw_gpio_isr.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define BTN_PRESS_LEVEL (0)
#define BTN_RELEASE_LEVEL (1)
#define BTN_SHORT_PRESS_TIMEOUT (1000 / portTICK_PERIOD_MS)
#define BTN_LONG_PRESS_TIMEOUT (6000 / portTICK_PERIOD_MS)
#define DB_FLASH_CONFIGFILE_SIZE     (32 * 1024) /* half of SystemConfigSize, store*/
/* DATA TYPE DECLARATIONS
 */
typedef void (*isr_t)(void);
/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T  gpio = 0;
static TickType_t btn_press_start =0;
static TickType_t btn_press_end = 0;
/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*FUNCTION NAME: btn_reset_isr_init
 * PURPOSE:
 *        initialize processing for reset button interrupt.
 * INPUT:
 *       usr_input_gpio -- The pointer which points to the TCP protocol control block
 *       triggered_condition -- 0 - disable;
 *                              1 - rising edge;
 *                              2 - falling edge;
 *                              3 - both rising and falling edges
 * OUTPUT:
 *       N/A
 * RETURN:
 *       N/A
 * NOTES:
 *       N/A
*/
void
btn_reset_isr_init(
    UI16_T usr_input_gpio,
    UI16_T triggered_condition)
{
    if(AIR_GPIO_MAX_NUM <= usr_input_gpio)
    {
        return;
    }
    gpio = (UI8_T) usr_input_gpio;
    /* Set GPIO triggered by triggered condition*/
    air_gpio_setInterruptEdgeDetectMode((UI8_T) usr_input_gpio, triggered_condition);
    /* GPIO int to IRQ_GPIO*/
#ifdef AIR_MW_SUPPORT
    mw_gpio_isr_register(MW_GPIO_ISR_USER_BTN_REST, (MW_GPIO_ISR_CALLBACK_FUNC_T) btn_reset_isr, NULL);
#else
    register_isr(IRQ_GPIO, btn_reset_isr);
#endif
}
/*FUNCTION NAME: btn_reset_isr
 * PURPOSE:
 *        processing for reset button interrupt.
 * INPUT:
 *       N/A
 * OUTPUT:
 *       N/A
 * RETURN:
 *       N/A
 * NOTES:
 *       N/A
*/
void
btn_reset_isr(
    void)
{
    I32_T  status = 0;
    I32_T  gpio_val=0;
    static UI32_T gpio_val_old = BTN_RELEASE_LEVEL;
    TickType_t btn_press_duration =0;

#ifndef AIR_MW_SUPPORT
    record_interrupts(IRQ_GPIO);
    unregister_isr(IRQ_GPIO);
#endif
    /*clear interrupt flag*/
    air_gpio_getInterruptStatus(gpio, &status);
    air_gpio_clearInterrupt(gpio);
    if(0 == status)
    {
        /* isr not triggered */
        return;
    }
    air_gpio_getValue(gpio,&gpio_val);

    if ( (BTN_PRESS_LEVEL == gpio_val) && (BTN_RELEASE_LEVEL == gpio_val_old) )
    {
        btn_press_start = xTaskGetTickCountFromISR();
    }
    else if ( (BTN_RELEASE_LEVEL == gpio_val) && (BTN_PRESS_LEVEL == gpio_val_old))
    {
        btn_press_end = xTaskGetTickCountFromISR();
        btn_press_duration = btn_press_end - btn_press_start;
        if ( (btn_press_duration > BTN_SHORT_PRESS_TIMEOUT) && (btn_press_duration < BTN_LONG_PRESS_TIMEOUT))
            {
                portDISABLE_INTERRUPTS();
                air_chipscu_resetSystem(0);
            }
            else if (btn_press_duration >= BTN_LONG_PRESS_TIMEOUT)
            {
                portDISABLE_INTERRUPTS();
                flash_partition_copy(SystemConfig0, SystemConfig1, DB_FLASH_CONFIGFILE_SIZE);
                air_chipscu_resetSystem(0);
            }
    }
    gpio_val_old = gpio_val;
#ifndef AIR_MW_SUPPORT
    register_isr(IRQ_GPIO, btn_reset_isr);
#endif
}

