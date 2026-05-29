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

/* FILE NAME:  hal_an8801sb_phy.h
 * PURPOSE:
 *      Define AN8801SB driver function
 *
 * NOTES:
 *
 */

#ifndef __AN8801S_H
#define __AN8801S_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal_phy.h>

/* NAMING DECLARATIONS
 */
#define HAL_PHY_DEVICE_ID_AN8801SB (0xc0ff0420)

#define AN8801SB_SGMII_AN0           (0x10220000)
#define AN8801SB_SGMII_AN4           (0x10220010)
#define AN8801SB_PCS_CTRL1           (0x10220a00)
#define AN8801SB_PCS_STS             (0x10220b04)
#define AN8801SB_EFIFO_MODE          (0x10270100)
#define AN8801SB_EFIFO_CTRL          (0x10270104)
#define AN8801SB_EFIFO_WM            (0x10270108)
#define AN8801SB_PHY_LED_CFG_REG     (0x1000A394) /* pseudo register for led force state */
#define SGMII_PCS_FORCE_SYNC_OFF     (0x2)
#define SGMII_PCS_FORCE_SYNC_MASK    (0x6)
#define EFIFO_MODE_1000              (0x0f)
#define EFIFO_MODE_10_100            (0x0c)
#define EFIFO_CTRL_1000              (0x3f)
#define EFIFO_CTRL_10_100            (0xff)
#define EFIFO_WM_VALUE               (0x05050303)
#define EFIFO_WM_VALUE_1G            (0x10100303)
#define EFIFO_WM_VALUE_10_100        (0x05050303)
#define AN8801SB_SGMII_AN0_ANRESTART (0x0200) /* Serdes auto negotation restart */
#define AN8801SB_SGMII_AN0_AN_DONE   (0x0001)
#define AN8801SB_SGMII_AN0_RESET     (0x8000)

#define NO_SPEED            (0xff)
#define SGMII_STATUS_LINK   (0x8000)
#define SGMII_STATUS_DUPLEX (0x1000)

#define PHY_PRE_SPEED_REG (0x2b)

/* CL22 Reg Support Page Select */
#define LPBK_FAR_END          (0x8000)
#define MAX_WAIT_PHY_POWER_UP (50)
#define NEED_POWER_UP_DELAY   (0x01)
#define NO_POWER_UP_DELAY     (0x00)

/* chip control register */
#define AN8801SB_RG_GPIO_LED_INV        (0x10000010)
#define AN8801SB_RG_GPIO_DRIVING_E2     (0x10000018)
#define AN8801SB_RG_GPIO_LAN0_LED0_MODE (0x10000054)
#define AN8801SB_RG_GPIO_LAN0_LED0_SEL  (0x10000058)
#define AN8801SB_RG_FORCE_GPIO0_EN      (0x10000070)

/* LED Behavior */
#define AN8801SB_LED_BHV_LINK_1000     (0x0001)
#define AN8801SB_LED_BHV_LINK_100      (0x0002)
#define AN8801SB_LED_BHV_LINK_10       (0x0004)
#define AN8801SB_LED_BHV_LINK_FULLDPLX (0x0008)
#define AN8801SB_LED_BHV_LINK_HALFDPLX (0x0010)
#define AN8801SB_LED_BHV_BLINK_TX_1000 (0x0020)
#define AN8801SB_LED_BHV_BLINK_RX_1000 (0x0040)
#define AN8801SB_LED_BHV_BLINK_TX_100  (0x0080)
#define AN8801SB_LED_BHV_BLINK_RX_100  (0x0100)
#define AN8801SB_LED_BHV_BLINK_TX_10   (0x0200)
#define AN8801SB_LED_BHV_BLINK_RX_10   (0x0400)
#define AN8801SB_LED_BHV_HIGH_ACTIVE   (0x0800)

/* LED Link register */
#define AN8801SB_LED_LINK_1000     (0x0001)
#define AN8801SB_LED_LINK_100      (0x0002)
#define AN8801SB_LED_LINK_10       (0x0004)
#define AN8801SB_LED_LINK_DOWN     (0x0008)
#define AN8801SB_LED_LINK_FULLDPLX (0x0010)
#define AN8801SB_LED_LINK_HALFDPLX (0x0020)
#define AN8801SB_LED_LINK_FORCE_ON (0x0040)
#define AN8801SB_LED_POL_HIGH_ACT  (0x4000)
#define AN8801SB_LED_FUNC_ENABLE   (0x8000)

/* LED Blink register */
#define AN8801SB_LED_BLINK_TX_1000 (0x0001)
#define AN8801SB_LED_BLINK_RX_1000 (0x0002)
#define AN8801SB_LED_BLINK_TX_100  (0x0004)
#define AN8801SB_LED_BLINK_RX_100  (0x0008)
#define AN8801SB_LED_BLINK_TX_10   (0x0010)
#define AN8801SB_LED_BLINK_RX_10   (0x0020)
#define AN8801SB_LED_BLINK_FORCE   (0x0200)

/* LED Register */
#define AN8801SB_LED_COUNT          (2)
#define AN8801SB_LED_ON_DURATION    (0x0022)
#define AN8801SB_LED_BLINK_DURATION (0x0023)
#define AN8801SB_LED_0_ON_MASK      (0x0024)
#define AN8801SB_LED_0_BLK_MASK     (0x0025)
#define AN8801SB_LED_1_ON_MASK      (0x0026)
#define AN8801SB_LED_1_BLK_MASK     (0x0027)
#define AN8801SB_LED_2_ON_MASK      (0x0028)
#define AN8801SB_LED_2_BLK_MASK     (0x0029)
#define AN8801SB_LED_3_ON_MASK      (0x002A)
#define AN8801SB_LED_3_BLK_MASK     (0x002B)

#define AN8801SB_LED_RG_OFFSET           (0x0002)
#define AN8801SB_LED_CFG_RG_OFFSET       (0x0003)
#define AN8801SB_LED_0_CFG_CTRL_MODE_BIT (0x0000)
#define AN8801SB_LED_CFG_CTRL_MODE_WIDTH (0x0001)
#define AN8801SB_LED_CFG_CTRL_MODE_MASK  (0x0001)
#define AN8801SB_LED_CFG_STATE_MASK      (0x0003)
#define AN8801SB_LED_0_CFG_STATE_BIT     (0x0001)
#define AN8801SB_LED_CFG_STATE_WIDTH     (0x0002)
#define AN8801SB_LED_CFG_PATT_OFFSET     (0x0004)
#define AN8801SB_LED_CFG_PATT_WIDTH      (0x0002)
#define AN8801SB_LED_CFG_PATT_MASK       (0x0003)

#define AN8801SB_LED_BLINK_RATE_DEFAULT (0x0c00)
#define AN8801SB_LED_BLINK_RATE_HZ_HALF (0xBEBE)
#define AN8801SB_LED_BLINK_RATE_HZ_ONE  (0x5F5F)
#define AN8801SB_LED_BLINK_RATE_HZ_TWO  (0x2FB0)

/* CL22 Reg Support Page Select */
#define AN8801SB_PHY_MII_PAGE_SELECT (0x1f)
#define CL22_Page_Reg                (0x0000)
#define CL22_Page_ExtReg             (0x0001)
#define CL22_Page_MiscReg            (0x0002)
#define CL22_Page_LpiReg             (0x0003)
#define CL22_Page_tReg               (0x02A3)
#define CL22_Page_TrReg              (0x52B5)
#define LPBK_FAR_END                 (0x8000)

/* 0x52b5100 */
#define RgAddr_TrReg10h (0x10)
/* 0x52b5110 */
#define RgAddr_TrReg11h (0x11)
/* 0x52b5120 */
#define RgAddr_TrReg12h (0x12)

/* EFUSE define */
#define EFUSE_CMD_READ (0x01)

/* FUNCTION NAME: hal_an8801sb_phy_init
 * PURPOSE:
 *      AN8801SB PHY initialization
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  PHY address
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8801sb_phy_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_an8801sb_phy_getDriver
 * PURPOSE:
 *      AN8801SB PHY probe get driver
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
hal_an8801sb_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_an8801sb_phy_setAdminState
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
hal_an8801sb_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_an8801sb_phy_getAdminState
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
hal_an8801sb_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_an8801sb_phy_setAutoNego
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
hal_an8801sb_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_an8801sb_phy_getAutoNego
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
hal_an8801sb_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_an8801sb_phy_setLocalAdvAbility
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
hal_an8801sb_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8801sb_phy_getLocalAdvAbility
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
hal_an8801sb_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8801sb_phy_getRemoteAdvAbility
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
hal_an8801sb_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_an8801sb_phy_setSpeed
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
hal_an8801sb_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_an8801sb_phy_getSpeed
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
hal_an8801sb_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed);

/* FUNCTION NAME:   hal_an8801sb_phy_setDuplex
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
hal_an8801sb_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex);

/* FUNCTION NAME:   hal_an8801sb_phy_getDuplex
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
hal_an8801sb_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   hal_an8801sb_phy_getLinkStatus
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
hal_an8801sb_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_an8801sb_phy_setLedOnCtrl
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
hal_an8801sb_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_an8801sb_phy_getLedOnCtrl
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
hal_an8801sb_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_an8801sb_phy_setLoopBack
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
hal_an8801sb_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable);

/* FUNCTION NAME:   hal_an8801sb_phy_getLoopBack
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
hal_an8801sb_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable);

/* FUNCTION NAME:   hal_an8801sb_phy_setSmartSpeedDown
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
hal_an8801sb_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode);

/* FUNCTION NAME:   hal_an8801sb_phy_getSmartSpeedDown
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
hal_an8801sb_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

/* FUNCTION NAME:   hal_an8801sb_phy_testTxCompliance
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
hal_an8801sb_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedCtrlMode
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
hal_an8801sb_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedCtrlMode
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
hal_an8801sb_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedForceState
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
hal_an8801sb_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedForceState
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
hal_an8801sb_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedForcePattCfg
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
hal_an8801sb_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedForcePattCfg
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
hal_an8801sb_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_an8801sb_phy_triggerCableTest
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
hal_an8801sb_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_an8801sb_phy_getCableTestRawData
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
hal_an8801sb_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedGlbCfg
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
hal_an8801sb_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedGlbCfg
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
hal_an8801sb_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedBlkEvent
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
hal_an8801sb_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedBlkEvent
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
hal_an8801sb_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedDuration
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
hal_an8801sb_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedDuration
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
hal_an8801sb_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

/* FUNCTION NAME: hal_an8801sb_phy_setPhyOpMode
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
hal_an8801sb_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode);

/* FUNCTION NAME: hal_an8801sb_phy_getPhyOpMode
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
hal_an8801sb_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_an8801sb_phy_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
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
 *      Support cable diagnostic in link down mode only.
 */
AIR_ERROR_NO_T
hal_an8801sb_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_an8801sb_phy_dumpPhyPara
 * PURPOSE:
 *      Dump Phy parameters.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8801sb_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port);
#endif /* End of __AN8801S_H */
