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

/* FILE NAME:  air_chipscu.h
 * PURPOSE:
 *      It provides CHIP SCU module API.
 *
 * NOTES:
 */

#ifndef AIR_CHIPSCU_H
#define AIR_CHIPSCU_H

#include "platform.h"

/* INCLUDE FILE DECLARATIONS
 */


/* NAMING CONSTANT DECLARATIONS
*/
/* Open IRQ for GPIO/EXT_IRQ0~EXT_IRQ2 only */
#define AIR_IRQ_MAX_NUM                    (4)
#define AIR_CHIPSCU_IRQ_BITMAP_SIZE        AIR_BITMAP_SIZE(AIR_IRQ_MAX_NUM)

/* EXT_IRQ0 equal 28 */
#define AIR_CHIPSCU_IRQ_EXT_BASE           (IRQ_EXT_00)

/* Offset compare to AIR_CHIPSCU_INTR_SOURCE_GPIO from AIR_CHIPSCU_INTR_SOURCE_T*/
#define AIR_CHIPSCU_IRQ_EXT_OFFSET         (1)

#define AIR_CHIPSCU_IOMUX_ENABLE           (1)
typedef enum
{
    AIR_CHIPSCU_INTR_SOURCE_GPIO = 0,
    AIR_CHIPSCU_INTR_SOURCE_EXT_IRQ0,
    AIR_CHIPSCU_INTR_SOURCE_EXT_IRQ1,
    AIR_CHIPSCU_INTR_SOURCE_EXT_IRQ2,
    AIR_CHIPSCU_INTR_SOURCE_MAX
} AIR_CHIPSCU_INTR_SOURCE_T;

typedef enum
{
    AIR_CHIPSCU_LEVEL_TRIGGER_HIGH = 0,
    AIR_CHIPSCU_EDGE_TRIGGER_RISING,
    AIR_CHIPSCU_TRIGGER_TYPE_MAX
} AIR_CHIPSCU_INTR_TRIGGER_TYPE_T;

typedef enum
{
    /* GPIO_LAN_LED */
    AIR_CHIPSCU_IOMUX_LAN0_LED0_MODE = 0,
    AIR_CHIPSCU_IOMUX_LAN0_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN0_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN1_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN1_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN1_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN2_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN2_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN2_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN3_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN3_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN3_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN4_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN4_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN4_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN5_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN5_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN5_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN6_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN6_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN6_LED2_MODE,
    AIR_CHIPSCU_IOMUX_LAN7_LED0_MODE,
    AIR_CHIPSCU_IOMUX_LAN7_LED1_MODE,
    AIR_CHIPSCU_IOMUX_LAN7_LED2_MODE,
    /* MISC : 24*/
    AIR_CHIPSCU_IOMUX_C_MDIO_MODE,
    AIR_CHIPSCU_IOMUX_P_MDIO_MODE,
    AIR_CHIPSCU_IOMUX_SGPIO_MODE,
    AIR_CHIPSCU_IOMUX_SGPIO_RCLK_MODE,
    AIR_CHIPSCU_IOMUX_IRQ_MODE,
    AIR_CHIPSCU_IOMUX_SPI_CS1_MODE,
    AIR_CHIPSCU_IOMUX_SPI_QUAD_MODE,
    AIR_CHIPSCU_IOMUX_CHIP_PROBE0_MODE,
    /* Force GPIO : 32 */
    AIR_CHIPSCU_IOMUX_FORCE_GPIO0_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO1_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO2_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO3_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO4_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO5_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO6_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO7_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO8_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO9_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO10_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO11_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO12_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO13_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO14_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO15_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO16_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO17_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO18_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO19_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO20_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO21_MODE,
    AIR_CHIPSCU_IOMUX_FORCE_GPIO22_MODE,
    /* LED */
    AIR_CHIPSCU_IOMUX_LED_A_MODE,
    AIR_CHIPSCU_IOMUX_LED_B_MODE,
    AIR_CHIPSCU_IOMUX_LED_C_MODE,
    AIR_CHIPSCU_IOMUX_LED_D_MODE,
    AIR_CHIPSCU_IOMUX_LED_E_MODE,
    AIR_CHIPSCU_IOMUX_LED_F_MODE,
    AIR_CHIPSCU_IOMUX_LED_G_MODE,
    AIR_CHIPSCU_IOMUX_LED_H_MODE,
    AIR_CHIPSCU_IOMUX_LED_I_MODE,
    AIR_CHIPSCU_IOMUX_LED_J_MODE,
    AIR_CHIPSCU_IOMUX_LED_K_MODE,
    AIR_CHIPSCU_IOMUX_LED_L_MODE,
    AIR_CHIPSCU_IOMUX_LED_M_MODE,
    AIR_CHIPSCU_IOMUX_LED_N_MODE,
    AIR_CHIPSCU_IOMUX_LED_O_MODE,
    AIR_CHIPSCU_IOMUX_LED_P_MODE,
    AIR_CHIPSCU_IOMUX_LED_Q_MODE,
    
    AIR_CHIPSCU_IOMUX_FUNC_MAX
} AIR_CHIPSCU_IOMUX_FUNC_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
 *
 * NOTES:
 *      None
 */
int
air_chipscu_getInterruptTriggerMode(
    const unsigned int unit,
    const AIR_CHIPSCU_INTR_SOURCE_T hw_intr_src,
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T *ptr_trig_types);

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
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T chip_trig_types);

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
    int *ptr_state);

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
    int state);

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
    const unsigned int unit);

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
int
air_chipscu_getBootReason(
    const unsigned int unit,
    int *ptr_boot_reason);

#endif /* End of AIR_CHIPSCU_H */
