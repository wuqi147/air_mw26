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

/* FILE NAME:  hal_an8808_phy.h
 * PURPOSE:
 *      Define AN8808 driver function
 *
 * NOTES:
 *
 */

#ifndef HAL_AN8808_PHY_H
#define HAL_AN8808_PHY_H

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal_phy.h>

/* NAMING DECLARATIONS
 */
/* Register of wave-gen and PHY LED source control  */
#define HAL_AN8808_PHY_SLED_SRC_SEL_REG (0x1021C02C)
#define HAL_PHY_DEVICE_ID_AN8808Q       (0xc0ff0480)
#define HAL_PHY_DEVICE_ID_AN8808X       (0xc0ff0490)
#define HAL_AN8808_PHY_DEVICE_REV_ID_E1 (1)

#define HAL_AN8808_PHY_SGMII_AN0             (0x10220000)
#define HAL_AN8808_PHY_SGMII_AN4             (0x10220010)
#define HAL_AN8808_PHY_PCS_CTRL1             (0x10220a00)
#define HAL_AN8808_PHY_PCS_STS               (0x10220b04)
#define HAL_AN8808_PHY_SYS_CONFIG0           (0x10270f00)
#define HAL_AN8808_PHY_LS_CFG_PBL_CMPSAT_EN  (1 << 31)
#define HAL_AN8808_PHY_EFIFO_REG_OFFSET      (0x1000)
#define HAL_AN8808_PHY_RATE_ADP_P0_CTRL0_REG (0x10229100)
#define HAL_AN8808_PHY_RATE_ADP_P4_CTRL0_REG (0x10249100)
#define HAL_AN8808_PHY_RATE_ADP_CTRL0_OFFSET (8)
#define HAL_AN8808_PHY_SGMII_AN0_ANRESTART   (0x0200) /* Serdes auto negotation restart */
#define HAL_AN8808_PHY_SGMII_AN0_AN_DONE     (0x0001)
#define HAL_AN8808_PHY_SGMII_AN0_RESET       (0x8000)

/* chip control register */
#define HAL_AN8808_PHY_RG_GPIO_LED_INV        (0x10000010)
#define HAL_AN8808_PHY_RG_GPIO_DRIVING_E2     (0x10000018)
#define HAL_AN8808_PHY_RG_GPIO_LAN0_LED0_MODE (0x10000054)
#define HAL_AN8808_PHY_RG_GPIO_LAN0_LED0_SEL  (0x10000058)
#define HAL_AN8808_PHY_RG_FORCE_GPIO0_EN      (0x1000007C)

/* Register of serial LED control */
#define HAL_AN8808_PHY_GPIO_MODE     (0x10006000)
#define HAL_AN8808_PHY_HWTRAP_BYPASS (0x10006010)
#define HAL_AN8808_PHY_SLED_CTRL0    (0x10283024)
#define HAL_AN8808_PHY_SLED_CTRL1    (0x10283028)

/* fields of serial LED control register */
#define HAL_AN8808_PHY_SLED_SOURCE_NORMAL_MODE       (0)
#define HAL_AN8808_PHY_SLED_SOURCE_INTERLEAVING_MODE (1 << 0)
#define HAL_AN8808_PHY_SLED_NUM_LED0                 (1 << 1)
#define HAL_AN8808_PHY_SLED_NUM_LED0_LED1            (2 << 1)
#define HAL_AN8808_PHY_SLED_NUM_LED0_LED1_LED2       (3 << 1)
#define HAL_AN8808_PHY_SLED_OUTPUT_TRANSITION_MODE   (0)
#define HAL_AN8808_PHY_SLED_OUTPUT_CONTINUOUS_MODE   (1 << 3)
#define HAL_AN8808_PHY_SLED_FALLING_EDGE             (0)
#define HAL_AN8808_PHY_SLED_RISING_EDGE              (1 << 4)

/* LED Behavior */
#define HAL_AN8808_PHY_LED_BHV_LINK_1000     (0x0001)
#define HAL_AN8808_PHY_LED_BHV_LINK_100      (0x0002)
#define HAL_AN8808_PHY_LED_BHV_LINK_10       (0x0004)
#define HAL_AN8808_PHY_LED_BHV_LINK_FULLDPLX (0x0008)
#define HAL_AN8808_PHY_LED_BHV_LINK_HALFDPLX (0x0010)
#define HAL_AN8808_PHY_LED_BHV_BLINK_TX_1000 (0x0020)
#define HAL_AN8808_PHY_LED_BHV_BLINK_RX_1000 (0x0040)
#define HAL_AN8808_PHY_LED_BHV_BLINK_TX_100  (0x0080)
#define HAL_AN8808_PHY_LED_BHV_BLINK_RX_100  (0x0100)
#define HAL_AN8808_PHY_LED_BHV_BLINK_TX_10   (0x0200)
#define HAL_AN8808_PHY_LED_BHV_BLINK_RX_10   (0x0400)
#define HAL_AN8808_PHY_LED_BHV_HIGH_ACTIVE   (0x0800)

/* LED Link register */
#define HAL_AN8808_PHY_LED_LINK_1000     (0x0001)
#define HAL_AN8808_PHY_LED_LINK_100      (0x0002)
#define HAL_AN8808_PHY_LED_LINK_10       (0x0004)
#define HAL_AN8808_PHY_LED_LINK_DOWN     (0x0008)
#define HAL_AN8808_PHY_LED_LINK_FULLDPLX (0x0010)
#define HAL_AN8808_PHY_LED_LINK_HALFDPLX (0x0020)
#define HAL_AN8808_PHY_LED_LINK_FORCE_ON (0x0040)
#define HAL_AN8808_PHY_LED_POL_HIGH_ACT  (0x4000)
#define HAL_AN8808_PHY_LED_FUNC_ENABLE   (0x8000)

/* LED Blink register */
#define HAL_AN8808_PHY_LED_BLINK_TX_1000       (0x0001)
#define HAL_AN8808_PHY_LED_BLINK_RX_1000       (0x0002)
#define HAL_AN8808_PHY_LED_BLINK_TX_100        (0x0004)
#define HAL_AN8808_PHY_LED_BLINK_RX_100        (0x0008)
#define HAL_AN8808_PHY_LED_BLINK_TX_10         (0x0010)
#define HAL_AN8808_PHY_LED_BLINK_RX_10         (0x0020)
#define HAL_AN8808_PHY_LED_BLINK_COLLISION     (0x0040)
#define HAL_AN8808_PHY_LED_BLINK_RX_CRC_ERROR  (0x0080)
#define HAL_AN8808_PHY_LED_BLINK_RX_IDLE_ERROR (0x0100)
#define HAL_AN8808_PHY_LED_BLINK_FORCE         (0x0200)

/* LED Register */
#define HAL_AN8808_PHY_LED_COUNT          (2)
#define HAL_AN8808_PHY_LED_0_ON_MASK      (0x03e0)
#define HAL_AN8808_PHY_LED_0_BLK_MASK     (0x03e1)
#define HAL_AN8808_PHY_LED_1_ON_MASK      (0x03e2)
#define HAL_AN8808_PHY_LED_1_BLK_MASK     (0x03e3)
#define HAL_AN8808_PHY_LED_2_ON_MASK      (0x03e4)
#define HAL_AN8808_PHY_LED_2_BLK_MASK     (0x03e5)
#define HAL_AN8808_PHY_LED_3_ON_MASK      (0x03e6)
#define HAL_AN8808_PHY_LED_3_BLK_MASK     (0x03e7)
#define HAL_AN8808_PHY_LED_CTRL_SELECT    (0x03e8)
#define HAL_AN8808_PHY_LED_0_ON_DURATION  (0x03e9)
#define HAL_AN8808_PHY_LED_0_BLK_DURATION (0x03ea)
#define HAL_AN8808_PHY_LED_1_ON_DURATION  (0x03eb)
#define HAL_AN8808_PHY_LED_1_BLK_DURATION (0x03ec)
#define HAL_AN8808_PHY_LED_2_ON_DURATION  (0x03ed)
#define HAL_AN8808_PHY_LED_2_BLK_DURATION (0x03ee)
#define HAL_AN8808_PHY_LED_3_ON_DURATION  (0x03ef)
#define HAL_AN8808_PHY_LED_3_BLK_DURATION (0x03f0)

#define HAL_AN8808_PHY_LED_CFG_RG_OFFSET       (0x0005)
#define HAL_AN8808_PHY_LED_0_CFG_CTRL_MODE_BIT (0x0000)
#define HAL_AN8808_PHY_LED_CFG_CTRL_MODE_WIDTH (0x0001)
#define HAL_AN8808_PHY_LED_CFG_CTRL_MODE_MASK  (0x0001)
#define HAL_AN8808_PHY_LED_0_CFG_STATE_BIT     (0x0001)
#define HAL_AN8808_PHY_LED_CFG_STATE_WIDTH     (0x0002)
#define HAL_AN8808_PHY_LED_CFG_STATE_MASK      (0x0003)
#define HAL_AN8808_PHY_LED_0_CFG_PATT_BIT      (0x0003)
#define HAL_AN8808_PHY_LED_CFG_PATT_WIDTH      (0x0002)
#define HAL_AN8808_PHY_LED_CFG_PATT_MASK       (0x0003)

#define HAL_AN8808_PHY_LED_ON_RATE_DEFAULT    (0x0600)
#define HAL_AN8808_PHY_LED_BLINK_RATE_DEFAULT (0x0C00)
#define HAL_AN8808_PHY_LED_BLINK_RATE_HZ_HALF (0xBEBE)
#define HAL_AN8808_PHY_LED_BLINK_RATE_HZ_ONE  (0x5F5F)
#define HAL_AN8808_PHY_LED_BLINK_RATE_HZ_TWO  (0x2FB0)

#define HAL_AN8808_PHY_LPBK_FAR_END (0x8000)

/* Wave-Gen pattern */
#define HAL_AN8808_PHY_GPIO_FLASH_PRD_SET2_REG (0x1000A344)
#define HAL_AN8808_PHY_GPIO_FLASH_PRD_SET3_REG (0x1000A348)
#define HAL_AN8808_PHY_CYCLE_CFG_VALUE1_REG    (0x1000A39C)
#define HAL_AN8808_PHY_WAVE_GEN_CYCLE_567      (0xffffff00) /* for 250Hz cycle */
#define HAL_AN8808_PHY_WAVE_GEN_CYCLE_MAX      (0xff)
#define HAL_AN8808_PHY_WAVE_GEN_CYCLE_MIN      (0x00)
#define HAL_AN8808_PHY_PATTERN_ON              (0xff00)
#define HAL_AN8808_PHY_PATTERN_OFF             (0x00ff)
#define HAL_AN8808_PHY_PATTERN_HZ_HALF         (0xfafa)
#define HAL_AN8808_PHY_PATTERN_HZ_ONE          (0x7d7d)
#define HAL_AN8808_PHY_PATTERN_HZ_TWO          (0x3e3e)
#define HAL_AN8808_PHY_PATTERN_HZ_ZERO         (0x0000)

#define HAL_AN8808_PHY_FLASH_MAP_ID_PATT_0        (0x00ee)
#define HAL_AN8808_PHY_FLASH_MAP_ID_PATT_1        (0x00ff)
#define HAL_AN8808_PHY_FLASH_MAP_ID_PATT_PARALLEL (0xeeeeeeee)
#define HAL_AN8808_PHY_FLASH_MAP_ID_OFF           (0x00cc)
#define HAL_AN8808_PHY_FLASH_MAP_ID_ON            (0x00dd)

#define HAL_AN8808_PHY_LED_TYPE_PARALLEL (1)
#define HAL_AN8808_PHY_LED_TYPE_SERIAL   (0)

#define HAL_AN8808_PHY_PHY_PAGE_0000 (0)
#define HAL_AN8808_PHY_PHY_PAGE_2A30 (1)
#define HAL_AN8808_PHY_PHY_PAGE_52B5 (2)
#define HAL_AN8808_PHY_PHY_PAGE_0001 (3)
#define HAL_AN8808_PHY_PHY_PAGE_0002 (4)
#define HAL_AN8808_PHY_PHY_PAGE_0003 (5)
#define HAL_AN8808_PHY_PHY_PAGE_0004 (6)

/* CL22 Reg Support Page Select */
#define HAL_AN8808_PHY_MII_PAGE_SELECT   (0x1f)
#define HAL_AN8808_PHY_CL22_Page_Reg     (0x0000)
#define HAL_AN8808_PHY_CL22_Page_ExtReg  (0x0001)
#define HAL_AN8808_PHY_CL22_Page_MiscReg (0x0002)
#define HAL_AN8808_PHY_CL22_Page_LpiReg  (0x0003)
#define HAL_AN8808_PHY_CL22_Page_tReg    (0x02A3)
#define HAL_AN8808_PHY_CL22_Page_TrReg   (0x52B5)

#define HAL_AN8808_PHY_HW_TRAP_READ_REG      (0x1000003c)
#define HAL_AN8808_PHY_PHY_BASE_ADDR_MASK    (0x3)
#define HAL_AN8808_PHY_PHY_BASE_ADDR_SHIFT   (12)
#define HAL_AN8808_PHY_CPU_CLOCK_GATING_REG  (0x10000068)
#define HAL_AN8808_PHY_CPU_CLOCK_GATING_MASK (0x1)

#define HAL_AN8808_DMEM_ADDRESS         (0x80000000)
#define HAL_AN8808_FIRMWARE_LEN_REG     (0x80039100)
#define HAL_AN8808_FIRMWARE_SUM_REG     (0x80039104)
#define HAL_AN8808_FIRMWARE_SUM_RET_REG (0x80039108)

#define HAL_AN8808_ACTIVE_REG (0x1000a364)
#define HAL_AN8808_IDLE_CODE  (0x42)

#define HAL_AN8808_SLED_SRC_SEL_REG              (0x1028302c)
#define HAL_AN8808_SLED_PER_PORT_LEDS            (3)
#define HAL_AN8808_SLED_PER_PORT_LED_BIT0        (1)
#define HAL_AN8808_SLED_PER_PORT_LED_BIT1        (2)
#define HAL_AN8808_SLED_PER_PORT_LED_BIT2        (4)
#define HAL_AN8808_SLED_FLASH_SET_CTR0_REG       (0x10283030)
#define HAL_AN8808_SLED_FLASH_SET_ID_OFFSET      (4)
#define HAL_AN8808_SLED_FLASH_SET_ID_PORT_OFFSET (16)
#define HAL_AN8808_SLED_FLASH_SET_ID_MASK        (0x0f)
#define HAL_AN8808_SLED_FLASH_MAP_ID_OFF         (0x04)
#define HAL_AN8808_SLED_FLASH_MAP_ID_ON          (0x05)
#define HAL_AN8808_SLED_FLASH_MAP_ID_PATT_6      (0x06)
#define HAL_AN8808_SLED_FLASH_MAP_ID_PATT_7      (0x07)
/* PSR */
#define HAL_AN8808_PHY_PSR_P3_P0_REG     (0x1021c018)
#define HAL_AN8808_PHY_PSR_P4_REG        (0x1021c01c)
#define HAL_AN8808_PHY_PER_PSR_PORTS     (4)
#define HAL_AN8808_PHY_PER_PSR_OFFSET    (8)
#define HAL_AN8808_PHY_PSR_LINK_STS_MASK (0x00000001)

#define HAL_AN8808_PHY_REG_IO_CFG0     (0x1000A700UL)
#define HAL_AN8808_PHY_REG_IO_SMI_BITS (0x3F000000UL)

typedef struct HAL_QSGMII_CFG_S
{
    UI32_T reg_addr;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
} HAL_QSGMII_CFG_T;

/* FUNCTION NAME: hal_an8808_phy_init
 * PURPOSE:
 *      AN8804 PHY initialization
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  PHY address
 *
 * OUTPUT:
 *        None
 *
 * RETURN:an8808
 *        AIR_E_OK
 *        AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_an8808_phy_getDriver
 * PURPOSE:
 *      AN8804 PHY probe get driver
 *
 * INPUT:
 *      pptr_hal_driver  --  pointer for store function vector
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_an8808_phy_setAdminState
 * PURPOSE:
 *      This API is used to set port state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      state           --  Port state
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_an8808_phy_getAdminState
 * PURPOSE:
 *      This API is used to get port state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_state       --  Port state
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_an8808_phy_setAutoNego
 * PURPOSE:
 *      This API is used to set port auto-negotiation.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      auto_nego       --  Auto-negotiation
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_an8808_phy_getAutoNego
 * PURPOSE:
 *      This API is used to get port auto-negotiation.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_auto_nego   --  Auto-negotiation
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_an8808_phy_setLocalAdvAbility
 * PURPOSE:
 *      This API is used to set port local advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8808_phy_getLocalAdvAbility
 * PURPOSE:
 *      This API is used to get port local advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8808_phy_getRemoteAdvAbility
 * PURPOSE:
 *      This API is used to get port remote advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8808_phy_setSpeed
 * PURPOSE:
 *      This API is used to set port speed.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      speed           --  Port speed
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_an8808_phy_getSpeed
 * PURPOSE:
 *      This API is used to get port speed.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_speed       --  Port speed
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed);

/* FUNCTION NAME:   hal_an8808_phy_setDuplex
 * PURPOSE:
 *      This API is used to set port duplex.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      duplex          --  Port duplex
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex);

/* FUNCTION NAME:   hal_an8808_phy_getDuplex
 * PURPOSE:
 *      This API is used to get port duplex.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_duplex      --  Port duplex
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   hal_an8808_phy_getLinkStatus
 * PURPOSE:
 *      This API is used to get port link status.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_status      --  Link Status
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_an8808_phy_setLedOnCtrl
 * PURPOSE:
 *      This API is used to set control of port LED.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_an8808_phy_getLedOnCtrl
 * PURPOSE:
 *      This API is used to get port LED control setting.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED ID
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_an8808_phy_setLoopBack
 * PURPOSE:
 *      This API is used to set port loop back.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  LookBack type
 *      enable          --  mode enable/disable
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable);

/* FUNCTION NAME:   hal_an8808_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get port loop back.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  LookBack type
 *
 * OUTPUT:
 *      ptr_enable      --  mode enable/disable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable);

/* FUNCTION NAME:   hal_an8808_phy_setSmartSpeedDown
 * PURPOSE:
 *      This API is used to gst port smart speed down.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ssd_mode        --  smart speed down mode
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode);

/* FUNCTION NAME:   hal_an8808_phy_getSmartSpeedDown
 * PURPOSE:
 *      This API is used to get port power save.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *
 * OUTPUT:
 *      ptr_ssd_mode    --  smart speed down mode
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

/* FUNCTION NAME:   hal_an8808_phy_testTxCompliance
 * PURPOSE:
 *      This API is used to set the Tx compliance mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      mode            --  BIST mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to set phy led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      ctrl_mode       --  LED control mode enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to get phy led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_ctrl_mode   --  LED control mode
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedForceState
 * PURPOSE:
 *      This API is used to set phy led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      state           --  LED force state enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedForceState
 * PURPOSE:
 *      This API is used to get phy led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_state       --  LED force state enumeration type
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to set phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      pattern         --  LED force pattern enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to get phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern enumeration type
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_an8808_phy_triggerCableTest
 * PURPOSE:
 *      Trigger cable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *      test_pair       --  Select test pair
 *                          HAL_PHY_CABLE_TEST_PAIR_A
 *                          HAL_PHY_CABLE_TEST_PAIR_B
 *                          HAL_PHY_CABLE_TEST_PAIR_C
 *                          HAL_PHY_CABLE_TEST_PAIR_D
 *                          HAL_PHY_CABLE_TEST_PAIR_ALL
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          HAL_PHY_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
hal_an8808_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_an8808_phy_getCableTestRawData
 * PURPOSE:
 *      Get cable ec training 4 pair raw date.
 *
 * INPUT:
 *      unit                --  Device ID
 *      port                --  Select port number
 *      test_pair           --  Select test pair
 *
 * OUTPUT:
 *      pptr_raw_data_all   --  Cable diagnostic raw information
 *
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_an8808_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME: hal_an8808_phy_setPhyLedGlbCfg
 * PURPOSE:
 *      Set LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_an8808_phy_getPhyLedGlbCfg
 * PURPOSE:
 *      Get LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_an8808_phy_setPhyLedBlkEvent
 * PURPOSE:
 *      Set LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 *      evt_flags                -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

/* FUNCTION NAME: hal_an8808_phy_getPhyLedBlkEvent
 * PURPOSE:
 *      Get LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_evt_flags            -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

/* FUNCTION NAME: hal_an8808_phy_setPhyLedDuration
 * PURPOSE:
 *      Set LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 *      time                     -- Duration time, unit: ms
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

/* FUNCTION NAME: hal_an8808_phy_getPhyLedDuration
 * PURPOSE:
 *      Get LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 * OUTPUT:
 *      ptr_time                 -- Duration time, unit: ms
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

/* FUNCTION NAME: hal_an8808_phy_setPhyOpMode
 * PURPOSE:
 *      Set Phy operation mode.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Phy operation mode
 *                                  AIR_PORT_OP_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode);

/* FUNCTION NAME: hal_an8808_phy_getPhyOpMode
 * PURPOSE:
 *      Get Phy operation mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *      ptr_mode                 -- Phy operation mode enumeration type
 *                                  AIR_PORT_OP_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode);

#endif /* End of hal_an8808_PHY_H */
