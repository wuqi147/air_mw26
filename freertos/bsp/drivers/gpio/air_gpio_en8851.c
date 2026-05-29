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

/* FILE NAME:  air_gpio.c
 * PURPOSE:
 *    It provide GPIO module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_gpio.h>
#include <pp_def.h>
#include <util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_GPIO_MAX                   (64)
#define HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1  (16)    /* For GPIOCTRL/ GPIOINTEDG/ GPIOINTLV */
#define HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2  (32)    /* For GPIOOE/GPIODATA/GPIOINT */

/* GPIO Group index */
typedef enum
{
    _HAL_SCO_GPIO_GROUP0,       /* GPIO00~GPIO15 */
    _HAL_SCO_GPIO_GROUP1,       /* GPIO16~GPIO31 */
    _HAL_SCO_GPIO_GROUP2,       /* GPIO32~GPIO47 */
    _HAL_SCO_GPIO_GROUP3,       /* GPIO48~GPIO63 */
    _HAL_SCO_GPIO_GROUP_MAX
}_HAL_SCO_GPIO_GROUP_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SCO_GPIO_SET_VALUE(__out__, __val__, __offset__, __length__) do                \
{                                                                                           \
    (__out__) &= ~BITS_RANGE((__offset__), (__length__));                                   \
    (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));                         \
}while(0)

/* EXPORTED SUBPROGRAM BODIES
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
    const unsigned char gpio_pin,
    unsigned char *ptr_dir)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOCTRL;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOCTRL1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOCTRL2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOCTRL3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    (*ptr_dir) = BITS_OFF_R(u32dat, ((gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1) * GPIO_CTRL_OFFSET), GPIO_CTRL_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    const unsigned char dir)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOCTRL;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOCTRL1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOCTRL2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOCTRL3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    _HAL_SCO_GPIO_SET_VALUE(u32dat, dir, GPIO_CTRL_OFFSET * (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1), GPIO_CTRL_LENGTH);
    io_write32(u32regOff, u32dat);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    int *ptr_mode)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIOOE;
    }
    else
    {
        u32regOff = GPIOOE1;
    }

    /* Get GPIO output enable mode */
    u32dat = io_read32(u32regOff);
    (*ptr_mode) = !!BITS_OFF_R(u32dat, (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2), GPIO_OUTPUT_ENABLE_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    const int mode)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIOOE;
    }
    else
    {
        u32regOff = GPIOOE1;
    }

    /* Configure output enable mode */
    u32dat = io_read32(u32regOff);
    if(mode == FALSE)
    {
        u32dat &= ~BIT(gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2);
    }
    else
    {
        u32dat |= BIT(gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2);
    }
    io_write32(u32regOff, u32dat);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    int *ptr_value)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIODATA;
    }
    else
    {
        u32regOff = GPIODATA1;
    }

    /* Get GPIO pin status */
    u32dat = io_read32(u32regOff);
    (*ptr_value) = !!BITS_OFF_R(u32dat, (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2), GPIO_DATA_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    const int value)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIODATA;
    }
    else
    {
        u32regOff = GPIODATA1;
    }

    /* Configure output value */
    u32dat = io_read32(u32regOff);
    if(value == FALSE)
    {
        u32dat &= ~BIT(gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2);
    }
    else
    {
        u32dat |= BIT(gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2);
    }
    io_write32(u32regOff, u32dat);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    unsigned char *ptr_trig_mode)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOINTEDG;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOINTEDG1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOINTEDG2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOINTEDG3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    (*ptr_trig_mode) = BITS_OFF_R(u32dat, ((gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1) * GPIO_EDGE_INT_CTRL_OFFSET), GPIO_EDGE_INT_CTRL_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    const unsigned char trig_mode)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOINTEDG;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOINTEDG1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOINTEDG2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOINTEDG3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    _HAL_SCO_GPIO_SET_VALUE(u32dat, trig_mode, GPIO_EDGE_INT_CTRL_OFFSET * (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1), GPIO_EDGE_INT_CTRL_LENGTH);
    io_write32(u32regOff, u32dat);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    unsigned char *ptr_trig_mode)
{
    unsigned int u32dat=0x0, u32regOff=0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOINTLVL;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOINTLVL1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOINTLVL2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOINTLVL3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    (*ptr_trig_mode) = BITS_OFF_R(u32dat, ((gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1) * GPIO_LEVEL_INT_CTRL_OFFSET), GPIO_LEVEL_INT_CTRL_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    const unsigned char trig_mode)
{
    unsigned int u32dat=0x0, u32regOff=0x0;

    /* Calculate GPIO GROUP index */
    switch(gpio_pin >> _HAL_SCO_GPIO_GROUP_MAX)
    {
        case _HAL_SCO_GPIO_GROUP0:
            u32regOff = GPIOINTLVL;
            break;
        case _HAL_SCO_GPIO_GROUP1:
            u32regOff = GPIOINTLVL1;
            break;
        case _HAL_SCO_GPIO_GROUP2:
            u32regOff = GPIOINTLVL2;
            break;
        case _HAL_SCO_GPIO_GROUP3:
            u32regOff = GPIOINTLVL3;
            break;
        default:
            return E_ENTRY_NOT_FOUND;
    }
    u32dat = io_read32(u32regOff);
    _HAL_SCO_GPIO_SET_VALUE(u32dat, trig_mode, GPIO_LEVEL_INT_CTRL_OFFSET * (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_1), GPIO_LEVEL_INT_CTRL_LENGTH);
    io_write32(u32regOff, u32dat);
    return E_OK;
}

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
    const unsigned char gpio_pin,
    int *ptr_irs)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIOINT;
    }
    else
    {
        u32regOff = GPIOINT1;
    }

    /* Get GPIO pin interrupt status */
    u32dat = io_read32(u32regOff);
    (*ptr_irs) = !!BITS_OFF_R(u32dat, (gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2), GPIO_INT_LENGTH);
    return E_OK;
}

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
    const unsigned char gpio_pin)
{
    unsigned int u32dat = 0x0, u32regOff = 0x0;

    /* Calculate GPIO GROUP index */
    if(gpio_pin < HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2)
    {
        u32regOff = GPIOINT;
    }
    else
    {
        u32regOff = GPIOINT1;
    }

    /* Clear interrupt */
    u32dat = BIT(gpio_pin % HAL_SCO_GPIO_MAX_PER_GROUP_TYPE_2);
    io_write32(u32regOff, u32dat);
    return E_OK;
}
