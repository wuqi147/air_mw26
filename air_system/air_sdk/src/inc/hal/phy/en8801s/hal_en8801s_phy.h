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

/* FILE NAME:  hal_en8801s_phy.h
 * PURPOSE:
 *      Define EN8801S driver function
 *
 * NOTES:
 *
 */

#ifndef __EN8801S_H
#define __EN8801S_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal_phy.h>

/* NAMING DECLARATIONS
 */
#define HAL_PHY_DEVICE_ID_EN8801S (0x03a29460)

#define MAX_PHYS                 (32)
#define EN8801S_RG_ETHER_PHY_OUI (0x19a4)
#define EN8801S_RG_SMI_ADDR      (0x19a8)
#define EN8801S_RG_BUCK_CTL      (0x1a20)
#define EN8801S_RG_LTR_CTL       (0x0cf8)

#define EN8801S_PBUS_OUI (0x17a5)
#define EN8801S_PHY_ID1  (0x03a2)
#define EN8801S_PHY_ID2  (0x9461)
#define EN8801S_PHY_ID   (unsigned long)((EN8801S_PHY_ID1 << 16) | EN8801S_PHY_ID2)

#define DEV1E_REG013_VALUE (0)
#define DEV1E_REG19E_VALUE (0xC2)
#define DEV1E_REG324_VALUE (0x200)

#define EN8801S_RG_ECO (0x18e0)
#define EN8801S_ECO_E4 (0x04)

#define EN8801S_TX_POLARITY (0)
#define EN8801S_RX_POLARITY (1)

#define MAX_RETRY     (5)
#define MAX_OUI_CHECK (2)
/* CL45 MDIO control */
#define MII_MMD_ACC_CTL_REG   (0x0d)
#define MII_MMD_ADDR_DATA_REG (0x0e)
#define MMD_OP_MODE_DATA      BIT(14)

#define MAX_TRG_COUNTER (5)

/* TokenRing Reg Access */
#define TrReg_PKT_XMT_STA (0x8000)
#define TrReg_WR          (0x8000)
#define TrReg_RD          (0xA000)

#define RgAddr_LPI_1Ch       (0x1c)
#define RgAddr_AUXILIARY_1Dh (0x1d)
#define RgAddr_R1000DEC_15h  (0x3aa)
#define RgAddr_R1000DEC_17h  (0x3ae)
#define RgAddr_PMA_00h       (0x0f80)
#define RgAddr_PMA_01h       (0x0f82)
#define RgAddr_PMA_17h       (0x0fae)
#define RgAddr_PMA_18h       (0x0fb0)
#define RgAddr_TR_26h        (0x0ecc)
#define RgAddr_DSPF_03h      (0x1686)
#define RgAddr_DSPF_06h      (0x168c)
#define RgAddr_DSPF_08h      (0x1690)
#define RgAddr_DSPF_0Ch      (0x1698)
#define RgAddr_DSPF_0Dh      (0x169a)
#define RgAddr_DSPF_0Fh      (0x169e)
#define RgAddr_DSPF_10h      (0x16a0)
#define RgAddr_DSPF_11h      (0x16a2)
#define RgAddr_DSPF_13h      (0x16a6)
#define RgAddr_DSPF_14h      (0x16a8)
#define RgAddr_DSPF_1Bh      (0x16b6)
#define RgAddr_DSPF_1Ch      (0x16b8)
#define RgAddr_TR_26h        (0x0ecc)

#define MAX_SERDES           (5)
#define SERDES_0             (0)
#define SERDES_1             (1)
#define RG_HSGMII_0_LK_STS_0 (0x10224514)
#define RG_HSGMII_1_LK_STS_0 (0x10234514)
#define RG_HSGMII_0_LK_STS_2 (0x1022451c)
#define RG_HSGMII_1_LK_STS_2 (0x1023451c)

#define EN8801S_PHY_ADDR_1        (0x1b)
#define EN8801S_PHY_ADDR_2        (0x1d)
#define EN8801S_PCS_CTRL1         (0x0a00)
#define SGMII_PCS_FORCE_SYNC_OFF  (0x2)
#define SGMII_PCS_FORCE_SYNC_MASK (0x6)

#define NO_SPEED               (0xff)
#define SGMII_STATUS_LINK      (0x8000)
#define SGMII_STATUS_DUPLEX    (0x1000)
#define EN8801S_PHY_DUPLEX_BIT BIT(12)

/* CL22 Reg Support Page Select */
#define MII_PAGE_SELECT  (0x1f)
#define CL22_Page_Reg    (0x0000)
#define CL22_Page_ExtReg (0x0001)
#define CL22_Page_TrReg  (0x52B5)
#define LPBK_FAR_END     (0x8000)

/* Ethernet Packet Generator (EPG) Control 1 Register */
#define EN8801S_REG_EXT_1D        (0x1D)
#define EN8801S_EPG_EN            (0x8000)
#define EN8801S_EPG_RUN           (0x4000)
#define EN8801S_EPG_TX_DUR        (0x2000)
#define EN8801S_EPG_PKT_LEN_10KB  (0x1800)
#define EN8801S_EPG_PKT_LEN_1518B (0x1000)
#define EN8801S_EPG_PKT_LEN_64B   (0x0800)
#define EN8801S_EPG_PKT_LEN_125B  (0x0000)
#define EN8801S_EPG_PKT_GAP       (0x0400)
#define EN8801S_EPG_DES_ADDR(a)   (((a) & 0xF) << 6)
#define EN8801S_EPG_SUR_ADDR(a)   (((a) & 0xF) << 2)
#define EN8801S_EPG_PL_TYP_RANDOM (0x0002)
#define EN8801S_EPG_BAD_FCS       (0x0001)

/* LED Behavior */
#define EN8801S_LED_BHV_LINK_1000     (0x0001)
#define EN8801S_LED_BHV_LINK_100      (0x0002)
#define EN8801S_LED_BHV_LINK_10       (0x0004)
#define EN8801S_LED_BHV_LINK_FULLDPLX (0x0008)
#define EN8801S_LED_BHV_LINK_HALFDPLX (0x0010)
#define EN8801S_LED_BHV_BLINK_TX_1000 (0x0020)
#define EN8801S_LED_BHV_BLINK_RX_1000 (0x0040)
#define EN8801S_LED_BHV_BLINK_TX_100  (0x0080)
#define EN8801S_LED_BHV_BLINK_RX_100  (0x0100)
#define EN8801S_LED_BHV_BLINK_TX_10   (0x0200)
#define EN8801S_LED_BHV_BLINK_RX_10   (0x0400)
#define EN8801S_LED_BHV_HIGH_ACTIVE   (0x0800)

/* LED Link register */
#define EN8801S_LED_LINK_1000     (0x0001)
#define EN8801S_LED_LINK_100      (0x0002)
#define EN8801S_LED_LINK_10       (0x0004)
#define EN8801S_LED_LINK_DOWN     (0x0008)
#define EN8801S_LED_LINK_FULLDPLX (0x0010)
#define EN8801S_LED_LINK_HALFDPLX (0x0020)
#define EN8801S_LED_LINK_FORCE_ON (0x0040)
#define EN8801S_LED_POL_HIGH_ACT  (0x4000)
#define EN8801S_LED_FUNC_ENABLE   (0x8000)

/* LED Blink register */
#define EN8801S_LED_BLINK_TX_1000 (0x0001)
#define EN8801S_LED_BLINK_RX_1000 (0x0002)
#define EN8801S_LED_BLINK_TX_100  (0x0004)
#define EN8801S_LED_BLINK_RX_100  (0x0008)
#define EN8801S_LED_BLINK_TX_10   (0x0010)
#define EN8801S_LED_BLINK_RX_10   (0x0020)
#define EN8801S_LED_BLINK_FORCE   (0x0200)

/* LED Register */
#define EN8801S_LED_COUNT          (2)
#define EN8801S_LED_ON_DURATION    (0x0022)
#define EN8801S_LED_BLINK_DURATION (0x0023)
#define EN8801S_LED_0_ON_MASK      (0x0024)
#define EN8801S_LED_0_BLK_MASK     (0x0025)
#define EN8801S_LED_1_ON_MASK      (0x0026)
#define EN8801S_LED_1_BLK_MASK     (0x0027)
#define EN8801S_LED_2_ON_MASK      (0x0028)
#define EN8801S_LED_2_BLK_MASK     (0x0029)
#define EN8801S_LED_3_ON_MASK      (0x002A)
#define EN8801S_LED_3_BLK_MASK     (0x002B)

#define EN8801S_LED_RG_OFFSET           (0x0002)
#define EN8801S_LED_CFG_RG_OFFSET       (0x0003)
#define EN8801S_LED_0_CFG_CTRL_MODE_BIT (0x0000)
#define EN8801S_LED_CFG_CTRL_MODE_WIDTH (0x0001)
#define EN8801S_LED_CFG_CTRL_MODE_MASK  (0x0001)
#define EN8801S_LED_CFG_STATE_MASK      (0x0003)
#define EN8801S_LED_0_CFG_STATE_BIT     (0x0001)
#define EN8801S_LED_CFG_STATE_WIDTH     (0x0002)
#define EN8801S_LED_CFG_PATT_OFFSET     (0x0004)
#define EN8801S_LED_CFG_PATT_WIDTH      (0x0002)
#define EN8801S_LED_CFG_PATT_MASK       (0x0003)

#define EN8801S_LED_BLINK_RATE_DEFAULT (0x0c00)
#define EN8801S_LED_BLINK_RATE_HZ_HALF (0xEF00)
#define EN8801S_LED_BLINK_RATE_HZ_ONE  (0x7780)
#define EN8801S_LED_BLINK_RATE_HZ_TWO  (0x38C0)

/* Tx Compliance */
#define RgAddr_dev1Eh (0x1e)

/* 0x51e01450 */
#define RgAddr_dev1Eh_reg145h (0x0145)
#define PD_DIS                (1 << 15)
#define FC_TDI_EN             (1 << 14)
#define FC_DI_ACT             (1 << 13)
#define FC_LITN_NO_COMP       (1 << 12)
#define FC_MDI_CO_MDIX        (3 << 3)
#define FC_MDI_CO_MDI         (2 << 3)
#define FC_MDI_CO_NOT         (0 << 3)
#define FC_10T_POLAR_SWAP     (3 << 1)
#define FC_10T_POLAR_NORMAL   (2 << 1)
#define FC_10T_POLAR_NOT      (0 << 1)
#define FC_MDI_CO_OFFT        (3)
#define FC_MDI_CO_LENG        (2)

/* 0x51e01a30 */
#define RgAddr_dev1Eh_reg1A3h (0x01a3)

/* 0x51e01a40 */
#define RgAddr_dev1Eh_reg1A4h (0x01a4)

#define RgAddr_dev1Fh (0x1f)

/* 0x52b5100 */
#define RgAddr_TrReg10h (0x10)
/* 0x52b5110 */
#define RgAddr_TrReg11h (0x11)
/* 0x52b5120 */
#define RgAddr_TrReg12h (0x12)

typedef enum
{
    EN8801S_STATE_INIT = 0x0,
    EN8801S_STATE_PROCESS,
    EN8801S_STATE_DONE,
    EN8801S_STATE_LAST
} EN8801S_STATE_T;

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI16_T DATA_Lo;
    UI16_T DATA_Hi;
} TR_DATA_T;

typedef union
{
    struct
    {
        /* b[15:00] */
        UI16_T smi_deton_wt             : 3;
        UI16_T smi_det_mdi_inv          : 1;
        UI16_T smi_detoff_wt            : 3;
        UI16_T smi_sigdet_debouncing_en : 1;
        UI16_T smi_deton_th             : 6;
        UI16_T rsv_14                   : 2;
    } DataBitField;
    UI16_T DATA;
} gephy_all_REG_LpiReg1Ch, *Pgephy_all_REG_LpiReg1Ch;

typedef union
{
    struct
    {
        /* b[15:00] */
        UI16_T rg_smi_detcnt_max    : 6;
        UI16_T rsv_6                : 2;
        UI16_T rg_smi_det_max_en    : 1;
        UI16_T smi_det_deglitch_off : 1;
        UI16_T rsv_10               : 6;
    } DataBitField;
    UI16_T DATA;
} gephy_all_REG_dev1Eh_reg324h, *Pgephy_all_REG_dev1Eh_reg324h;

typedef union
{
    struct
    {
        /* b[15:00] */
        UI16_T da_tx_i2mpb_a_tbt : 6;
        UI16_T rsv_6             : 4;
        UI16_T da_tx_i2mpb_a_gbe : 6;
    } DataBitField;
    UI16_T DATA;
} gephy_all_REG_dev1Eh_reg012h, *Pgephy_all_REG_dev1Eh_reg012h;

typedef union
{
    struct
    {
        /* b[15:00] */
        UI16_T da_tx_i2mpb_b_tbt : 6;
        UI16_T rsv_6             : 2;
        UI16_T da_tx_i2mpb_b_gbe : 6;
        UI16_T rsv_14            : 2;
    } DataBitField;
    UI16_T DATA;
} gephy_all_REG_dev1Eh_reg017h, *Pgephy_all_REG_dev1Eh_reg017h;

/* FUNCTION NAME: hal_en8801s_phy_init
 * PURPOSE:
 *      EN8801S PHY initialization
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
hal_en8801s_phy_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_en8801s_phy_getDriver
 * PURPOSE:
 *      EN8801S PHY probe get driver
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
hal_en8801s_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_en8801s_phy_setAdminState
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
hal_en8801s_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_en8801s_phy_getAdminState
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
hal_en8801s_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_en8801s_phy_setAutoNego
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
hal_en8801s_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_en8801s_phy_getAutoNego
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
hal_en8801s_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_en8801s_phy_setLocalAdvAbility
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
hal_en8801s_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_en8801s_phy_getLocalAdvAbility
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
hal_en8801s_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_en8801s_phy_getRemoteAdvAbility
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
hal_en8801s_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_en8801s_phy_setSpeed
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
hal_en8801s_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_en8801s_phy_getSpeed
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
hal_en8801s_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed);

/* FUNCTION NAME:   hal_en8801s_phy_setDuplex
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
hal_en8801s_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex);

/* FUNCTION NAME:   hal_en8801s_phy_getDuplex
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
hal_en8801s_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   hal_en8801s_phy_getLinkStatus
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
hal_en8801s_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_en8801s_phy_setLedOnCtrl
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
hal_en8801s_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_en8801s_phy_getLedOnCtrl
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
hal_en8801s_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_en8801s_phy_setLoopBack
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
hal_en8801s_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable);

/* FUNCTION NAME:   hal_en8801s_phy_getLoopBack
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
hal_en8801s_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable);

/* FUNCTION NAME:   hal_en8801s_phy_setSmartSpeedDown
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
hal_en8801s_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode);

/* FUNCTION NAME:   hal_en8801s_phy_getSmartSpeedDown
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
hal_en8801s_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

/* FUNCTION NAME:   hal_en8801s_phy_testTxCompliance
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
hal_en8801s_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedCtrlMode
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
hal_en8801s_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedCtrlMode
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
hal_en8801s_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedForceState
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
hal_en8801s_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedForceState
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
hal_en8801s_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedForcePattCfg
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
hal_en8801s_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedForcePattCfg
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
hal_en8801s_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_en8801s_phy_triggerCableTest
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
hal_en8801s_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_en8801s_phy_getCableTestRawData
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
hal_en8801s_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedGlbCfg
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
hal_en8801s_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedGlbCfg
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
hal_en8801s_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedBlkEvent
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
hal_en8801s_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedBlkEvent
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
hal_en8801s_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedDuration
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
hal_en8801s_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedDuration
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
hal_en8801s_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

/* FUNCTION NAME: hal_en8801s_phy_triggerLinkDownCableTest
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
hal_en8801s_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

#endif /* End of __EN8801S_H */
