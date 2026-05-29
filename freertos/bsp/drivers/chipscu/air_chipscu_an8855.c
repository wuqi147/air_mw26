/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:  air_chipscu.c
 * PURPOSE:
 *    It provide CHIP SCU module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_chipscu.h>
#include <pp_def.h>

/* EXPORTED SUBPROGRAM BODIES
*/
#define AIR_CHIPSCU_IOMUX_CONTROL2_ENTRY_BIT_LENGTH         (12)

/* FUNCTION NAME: air_chipscu_getInterruptTriggerMode
 * PURPOSE:
 *      Get HW interrupt trigger type for CPU
 *
 * INPUT:
 *      unit            --  Device ID
 *      hw_intr_src     --  CPU HW interrupt source number
 *
 * OUTPUT:
 *      ptr_trig_type   --  CPU HW interrupt trigger type
 *                          0: high-level-triggered
 *                          1: positive-edge-triggered
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */
int
air_chipscu_getInterruptTriggerMode(
    const unsigned int unit,
    const AIR_CHIPSCU_INTR_SOURCE_T hw_intr_src,
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T *ptr_trig_types)
{
    unsigned int irqIdx = 0, u32dat = 0;

    if(hw_intr_src != AIR_CHIPSCU_INTR_SOURCE_GPIO)
    {
        irqIdx = hw_intr_src + AIR_CHIPSCU_IRQ_EXT_BASE - AIR_CHIPSCU_IRQ_EXT_OFFSET;
    }

    /* Configure ivic_trig_type */
    u32dat = io_read32(CPU_INTERRUPT_TRIGGER_TYPE);
    (*ptr_trig_types) = !!BITS_OFF_R(u32dat, irqIdx, CPU_INTR_TRIG_TYPE_LENG);
    return E_OK;
}

/* FUNCTION NAME: air_chipscu_setInterruptTriggerMode
 * PURPOSE:
 *      Configure HW interrupt trigger type for CPU
 *
 * INPUT:
 *      unit            --  Device ID
 *      hw_intr_src     --  CPU HW interrupt source number
 *      trigger_type    --  CPU HW interrupt trigger type
 *                          0: high-level-triggered
 *                          1: positive-edge-triggered
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      Specific for N10 CPU only
 */
int
air_chipscu_setInterruptTriggerMode(
    const unsigned int unit,
    const AIR_CHIPSCU_INTR_SOURCE_T hw_intr_src,
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T chip_trig_types)
{
    unsigned int irqIdx = 0, u32dat = 0;

    if(hw_intr_src != AIR_CHIPSCU_INTR_SOURCE_GPIO)
    {
        irqIdx = hw_intr_src + AIR_CHIPSCU_IRQ_EXT_BASE - AIR_CHIPSCU_IRQ_EXT_OFFSET;
    }

    /* Configure ivic_trig_type */
    u32dat = io_read32(CPU_INTERRUPT_TRIGGER_TYPE);
    if(chip_trig_types == AIR_CHIPSCU_LEVEL_TRIGGER_HIGH)
    {
        u32dat &= ~BIT(irqIdx);
    }
    else
    {
        u32dat |= BIT(irqIdx);
    }
    io_write32(CPU_INTERRUPT_TRIGGER_TYPE, u32dat);
    return E_OK;
}

/* FUNCTION NAME: air_chipscu_getIomuxFuncState
 * PURPOSE:
 *      Get specific function state of IOMUX.
 *
 * INPUT:
 *      unit            --  Device ID
 *      func_idx        --  Funciton index of IOMUX
 *
 * OUTPUT:
 *      ptr_state       --  specific function state of IOMUX
 *                          0: disable
 *                          1: enable
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      If needs to use IOMUX get/set API to change pin function for specific purpose,
 *      be aware of precedence relation between function of IOMUX and check the precedence in related document.
 *
 *      For EN8851/53/60, few IOMUX pins are effected as group control by below functions:
 *      C_MDIO/ P_MDIO/ SGPIO/ IRQ/ SPI_QUAD/ ... etc.
 *      Ex. GPIO9~GPIO12 pin will translate function from GPIO to IRQ if enable IRQ mode via IOMUX.
 *      Ex. GPIO10/GPIO11 pin will tranlate function from GPIO to SPI_HOLD/SPI_WP if enable SPI_QUAD mode via IOMUX.
 *      The IOMUX pin will translate function by precedence in case such conflict happened.
 *      For example, the precedence of GPIO pin 10 & GPIO 11 is Force GPIO > GPIO LAN LED > IRQ > SPI Quad
 *      Both enabling SPI_QUAD and IRQ mode via IOMUX, GPIO pin 9 ~ GPIO pin 12 will tranlate to IRQ mode,
 *      GPIO pin 10 & GPIO pin 11 will keep stay IRQ mode because IRQ mode has higher precedence than SPI Quad mode.
 *
 */
int
air_chipscu_getIomuxFuncState(
    const unsigned int unit,
    const AIR_CHIPSCU_IOMUX_FUNC_T func_idx,
    int *ptr_state)
{
    unsigned int u32regOff = 0x0, u32dat = 0x0, bit_idx = 0x0;

    if( (func_idx >= AIR_CHIPSCU_IOMUX_LED_A_MODE) && (func_idx <= AIR_CHIPSCU_IOMUX_LED_Q_MODE) )
    {
        u32regOff = IOMUX_CONTROL1_REGISTER;
        if (func_idx <= AIR_CHIPSCU_IOMUX_LED_F_MODE)
        {
            bit_idx += func_idx - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
        else if (func_idx <= AIR_CHIPSCU_IOMUX_LED_L_MODE)
        {
            bit_idx += func_idx + 1 - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
        else
        {
            bit_idx += func_idx + 2 - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
    }
    else if( (func_idx >= AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE) && (func_idx <= AIR_CHIPSCU_IOMUX_FORCE_GPIO20_MODE) )
    {
        u32regOff = IOMUX_CONTROL3_REGISTER;
        bit_idx = func_idx - AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE;
    }
    else
    {
        return E_BAD_PARAMETER;
    }
    u32dat = io_read32(u32regOff);
    (*ptr_state) = !!BITS_OFF_R(u32dat, bit_idx, IOMUX_BIT_LENG);
    return E_OK;
}

/* FUNCTION NAME: air_chipscu_setIomuxFuncState
 * PURPOSE:
 *      Set specific function state of IOMUX.
 *
 * INPUT:
 *      unit            --  Device ID
 *      func_idx        --  Funciton index of IOMUX
 *      state           --  specific function state of IOMUX
 *                          0: disable
 *                          1: enable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      If needs to use IOMUX get/set API to change pin function for specific purpose,
 *      be aware of precedence relation between function of IOMUX and check the precedence in related document.
 *
 *      For EN8851/53/60, few IOMUX pins are effected as group control by below functions:
 *      C_MDIO/ P_MDIO/ SGPIO/ IRQ/ SPI_QUAD/ ... etc.
 *      Ex. GPIO9~GPIO12 pin will translate function from GPIO to IRQ if enable IRQ mode via IOMUX.
 *      Ex. GPIO10/GPIO11 pin will tranlate function from GPIO to SPI_HOLD/SPI_WP if enable SPI_QUAD mode via IOMUX.
 *      The IOMUX pin will translate function by precedence in case such conflict happened.
 *      For example, the precedence of GPIO pin 10 & GPIO 11 is Force GPIO > GPIO LAN LED > IRQ > SPI Quad
 *      Both enabling SPI_QUAD and IRQ mode via IOMUX, GPIO pin 9 ~ GPIO pin 12 will tranlate to IRQ mode,
 *      GPIO pin 10 & GPIO pin 11 will keep stay IRQ mode because IRQ mode has higher precedence than SPI Quad mode.
 *
 */
int
air_chipscu_setIomuxFuncState(
    const unsigned int unit,
    const AIR_CHIPSCU_IOMUX_FUNC_T func_idx,
    int state)
{
    unsigned int u32regOff = 0x0, u32dat = 0x0, bit_idx = 0x0;
    int i, j;

    if( (func_idx >= AIR_CHIPSCU_IOMUX_LED_A_MODE) && (func_idx <= AIR_CHIPSCU_IOMUX_LED_Q_MODE) )
    {
        u32regOff = IOMUX_CONTROL1_REGISTER;
        if (func_idx <= AIR_CHIPSCU_IOMUX_LED_F_MODE)
        {
            bit_idx += func_idx - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
        else if (func_idx <= AIR_CHIPSCU_IOMUX_LED_L_MODE)
        {
            bit_idx += func_idx + 1 - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
        else
        {
            bit_idx += func_idx + 2 - AIR_CHIPSCU_IOMUX_LED_A_MODE;
        }
    }
    else if( (func_idx >= AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE) && (func_idx <= AIR_CHIPSCU_IOMUX_FORCE_GPIO20_MODE) )
    {
        u32regOff = IOMUX_CONTROL3_REGISTER;
        bit_idx = func_idx - AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE;
    }
    else
    {
        return E_BAD_PARAMETER;
    }

    /* Configure IOMUX function state */
    u32dat = io_read32(u32regOff);
    if(state == FALSE)
    {
        u32dat &= ~BIT(bit_idx);
    }
    else
    {
        u32dat |= BIT(bit_idx);
    }
    io_write32(u32regOff, u32dat);
    return E_OK;
}

/* FUNCTION NAME: air_chipscu_resetSystem
 * PURPOSE:
 *      Reset system.
 *
 * INPUT:
 *      unit            --  Device ID
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
air_chipscu_resetSystem(
    const unsigned int unit)
{
    unsigned int u32dat = 0x0;

    io_write32(CHIPSCU_SCREG_WF0, u32dat);

    u32dat |= BIT(31);
    io_write32(CHIPSCU_RST_CTRL1, u32dat);
    return E_OK;
}

/* FUNCTION NAME: air_chipscu_getBootReason
 * PURPOSE:
 *      Check system boot/reset reason.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_boot_reason --  system boot/reset reason
 *                          0: cold boot
 *                          1: warm boot
 *
 * RETURN:
 *      E_OK
 *
 * NOTES:
 *      None
 */

#ifdef SYS_MONITOR_DIS_INTERRUPT
unsigned int g_u32dat_reset_reason;
#endif

int
air_chipscu_getBootReason(
    const unsigned int unit,
    int *ptr_boot_reason)
{
    unsigned int u32dat = 0x0;

#ifdef SYS_MONITOR_DIS_INTERRUPT
    u32dat = g_u32dat_reset_reason;
#else
    u32dat = io_read32(CHIPSCU_SYS_CTRL1);
#endif
    (*ptr_boot_reason) = BITS_OFF_R(u32dat, SYS_SOFT_RESET_DECT_BIT, SYSTEM_SOFTWARE_RESET);

    return E_OK;
}
