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
 *      E_BAD_PARAMETER -- if hw_intr_scr exceeds the number of supported irq.
 *
 * NOTES:
 *      Deprecated
 */
int
air_chipscu_getInterruptTriggerMode(
    const unsigned int unit,
    const AIR_CHIPSCU_INTR_SOURCE_T hw_intr_src,
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T *ptr_trig_types)
{
    unsigned int irqIdx = 0, u32dat = 0;

    if (hw_intr_src >= AIR_CHIPSCU_INTR_SOURCE_MAX)
    {
        return E_BAD_PARAMETER;
    }

    if (hw_intr_src != AIR_CHIPSCU_INTR_SOURCE_GPIO)
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
 *      E_BAD_PARAMETER -- if hw_intr_scr exceeds the number of supported irq.
 *
 * NOTES:
 *      Deprecated
 */
int
air_chipscu_setInterruptTriggerMode(
    const unsigned int unit,
    const AIR_CHIPSCU_INTR_SOURCE_T hw_intr_src,
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T chip_trig_types)
{
    unsigned int irqIdx = 0, u32dat = 0;

    if (hw_intr_src >= AIR_CHIPSCU_INTR_SOURCE_MAX)
    {
        return E_BAD_PARAMETER;
    }

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
 *      Deprecated
 */
int
air_chipscu_getIomuxFuncState(
    const unsigned int unit,
    const AIR_CHIPSCU_IOMUX_FUNC_T func_idx,
    int *ptr_state)
{
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
 *      Deprecated
 */
int
air_chipscu_setIomuxFuncState(
    const unsigned int unit,
    const AIR_CHIPSCU_IOMUX_FUNC_T func_idx,
    int state)
{
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
