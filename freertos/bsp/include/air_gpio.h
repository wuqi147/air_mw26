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

/* FILE NAME:  air_gpio.h
 * PURPOSE:
 *      It provides GPIO module API.
 *
 * NOTES:
 */

#ifndef AIR_GPIO_H
#define AIR_GPIO_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_GPIO_MAX_NUM                   (64)
#define AIR_GPIO_BITMAP_SIZE               AIR_BITMAP_SIZE(AIR_GPIO_MAX_NUM)

typedef enum
{
    AIR_GPIO_DIRECTION_INPUT = 0,
    AIR_GPIO_DIRECTION_OUTPUT,
    AIR_GPIO_DIRECTION_MAX
} AIR_GPIO_DIRECTION_T;

typedef enum
{
    AIR_GPIO_EDGE_TRIGGER_DISABLE = 0,
    AIR_GPIO_EDGE_TRIGGER_RISING,
    AIR_GPIO_EDGE_TRIGGER_FALLING,
    AIR_GPIO_EDGE_TRIGGER_BOTH,
    AIR_GPIO_EDGE_TRIGGER_MAX
} AIR_GPIO_EDGE_TRIGGER_T;

typedef enum
{
    AIR_GPIO_LEVEL_TRIGGER_DISABLE = 0,
    AIR_GPIO_LEVEL_TRIGGER_HIGH,
    AIR_GPIO_LEVEL_TRIGGER_LOW,
    AIR_GPIO_LEVEL_TRIGGER_MAX
} AIR_GPIO_LEVEL_TRIGGER_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: air_gpio_getDirection
 * PURPOSE:
 *      Get direction of specific GPIO pin.
 *
 * INPUT:
 *      pin             --  Select GPIO pin number
 *
 * OUTPUT:
 *      ptr_dir         --  GPIO direction
 *                          0: input mode
 *                          1: output mode
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_getDirection(
    const unsigned char pin,
    unsigned char *ptr_dir);

/* FUNCTION NAME: air_gpio_setDirection
 * PURPOSE:
 *      Set direction of specific GPIO pin.
 *
 * INPUT:
 *      pin             --  Select GPIO pin number
 *      dir             --  GPIO direction
 *                          0: input mode
 *                          1: output mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_setDirection(
    const unsigned char pin,
    const unsigned char dir);

/* FUNCTION NAME: air_gpio_getOutputEnable
 * PURPOSE:
 *      Get output enable state of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *
 * OUTPUT:
 *      ptr_mode        --  GPIO output enable mode.
 *                          TRUE : enable
 *                          FALSE: disable in Hi-Z state;
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_getOutputEnable(
    const unsigned char pin,
    int *ptr_mode);

/* FUNCTION NAME: air_gpio_setOutputEnable
 * PURPOSE:
 *      Set output enable mode of specific GPIO pin when it is set to output mode.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *      mode            --  GPIO output enable mode.
 *                          TRUE : enable
 *                          FALSE: disable in Hi-Z state;
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_setOutputEnable(
    const unsigned char pin,
    const int mode);

/* FUNCTION NAME: air_gpio_getValue
 * PURPOSE:
 *      Get input/output current value of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin
 *
 * OUTPUT:
 *      ptr_value       --  current input/output value
 *                          input mode:
 *                               FALSE: The current state is low
 *                                TRUE: The current state is high
 *                          output mode:
 *                               FALSE: Drive low
 *                                TRUE: Drive high
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_getValue(
    const unsigned char pin,
    int *ptr_value);

/* FUNCTION NAME: air_gpio_setValue
 * PURPOSE:
 *      Set output value of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin
 *      value           --  output value
 *                               FALSE: Drive low
 *                                TRUE: Drive high
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_setValue(
    const unsigned char pin,
    const int value);

/* FUNCTION NAME: air_gpio_getInterruptEdgeDetectMode
 * PURPOSE:
 *      Get interrupt edge detect mode settings of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *
 * OUTPUT:
 *      ptr_mode   --  Edge triggger mode.
 *                          0: Disable edge trigger;
 *                          1: Rising edge, interrupt triggered when GPIO pin toggles from low to high
 *                          2: Falling edge, interrupt triggered when GPIO pin toggles from high to low
 *                          3: Both, interrupt triggered when GPIO pin toggles
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_getInterruptEdgeDetectMode(
    const unsigned char pin,
    unsigned char *ptr_mode);

/* FUNCTION NAME: air_gpio_setInterruptEdgeDetectMode
 * PURPOSE:
 *      Set interrupt edge detect mode settings of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *      trig_mode       --  Edge triggger mode.
 *                          0: Disable edge trigger;
 *                          1: Rising edge, interrupt triggered when GPIO pin toggles from low to high
 *                          2: Falling edge, interrupt triggered when GPIO pin toggles from high to low
 *                          3: Both, interrupt triggered when GPIO pin toggles
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_setInterruptEdgeDetectMode(
    const unsigned char pin,
    const unsigned char mode);

/* FUNCTION NAME: air_gpio_getInterruptLevelDetectMode
 * PURPOSE:
 *      Get interrupt level detect mode settings of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *
 * OUTPUT:
 *      ptr_mode        --  Level triggger mode.
 *                          0: Disable level trigger;
 *                          1: High level, interrupt triggered when GPIO pin is high
 *                          2: Low level, interrupt triggered when GPIO pin is low
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_getInterruptLevelDetectMode(
    const unsigned char pin,
    unsigned char *ptr_mode);

/* FUNCTION NAME: air_gpio_setInterruptLevelDetectMode
 * PURPOSE:
 *      Set interrupt level detect mode settings of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin index
 *      trig_mode       --  Level triggger mode.
 *                          0: Disable edge trigger;
 *                          1: High level, interrupt triggered when GPIO pin is high
 *                          2: Low level, interrupt triggered when GPIO pin is low
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
int
air_gpio_setInterruptLevelDetectMode(
    const unsigned char pin,
    const unsigned char mode);

/* FUNCTION NAME: air_gpio_getInterruptStatus
 * PURPOSE:
 *      Get interrupt status of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin
 *
 * OUTPUT:
 *      ptr_irs         --  current interrupt status
 *                          FALSE: Interrupt is not asserted
 *                          TRUE : Interrupt is asserted
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_getInterruptStatus(
    const unsigned char pin,
    int *ptr_status);

/* FUNCTION NAME: air_gpio_clearInterrupt
 * PURPOSE:
 *      Clear interrupt status of specific GPIO pin.
 *
 * INPUT:
 *      gpio_pin        --  GPIO pin
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_gpio_clearInterrupt(
    const unsigned char pin);

#endif /* End of AIR_GPIO_H */

