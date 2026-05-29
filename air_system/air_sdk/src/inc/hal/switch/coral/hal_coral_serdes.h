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

/* FILE NAME:  hal_coral_serdes.h
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_SERDES_H
#define HAL_CORAL_SERDES_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_SERDES_ID_S0_BASE_ADDR (0x10220000)
#define HAL_CORAL_SERDES_ADDR_OFFSER     (0x10000)

#define HAL_CORAL_SERDES_NPSCU_BASE_ADDR (0x10005000)
#define HAL_CORAL_SERDES_NPSCU_RST_CTRL2 (0x00C4)
#define HSGMII_HSI0_RESET_BIT_OFFSET     (13)
#define PXP_RESET_BIT_OFFSET             (27)

#define HAL_CORAL_SERDES_RG_ELDO_NDIS_EN   (0x10000158)
#define HAL_CORAL_SERDES_RG_CFG_EXT_OFFSET (0x1)

#define HAL_CORAL_SERDES_ID_S0_BASE_ADDR (0x10220000)
#define HAL_CORAL_SERDES_ADDR_OFFSER     (0x10000)

#define HAL_CORAL_SERDES_AN_REG_0           (0x0000)
#define HAL_CORAL_SERDES_AN0_ENABLE_OFFSET  (12)
#define HAL_CORAL_SERDES_AN0_ENABLE_MASK    (0x1)
#define HAL_CORAL_SERDES_AN_REG_0_RESET     (0x8000) /* Serdes reset phy */
#define HAL_CORAL_SERDES_AN_REG_0_ANRESTART (0x0200) /* Serdes auto negotation restart */
#define HAL_CORAL_SERDES_AN_REG_0_ANENABLE  (0x1000) /* Serdes enable auto negotation */

#define HAL_CORAL_SERDES_AN_REG_4      (0x0010)
#define SGMII_1000BASEX_PAUSE_OFFSET   (7)
#define SGMII_1000BASEX_ASM_DIR_OFFSET (8)
#define SGMII_1000BASEX_PAUSE_MASK     (1)

#define HAL_CORAL_SERDES_AN_REG_5          (0x0014)
#define HAL_CORAL_SERDES_AN5_SPEED_MASK    (0x2)
#define HAL_CORAL_SERDES_AN5_SPEED_OFFSET  (10)
#define HAL_CORAL_SERDES_AN5_DUPLEX_MASK   (0x1)
#define HAL_CORAL_SERDES_AN5_DUPLEX_OFFSET (12)
#define HAL_CORAL_SERDES_AN5_LINK_MASK     (0x1)
#define HAL_CORAL_SERDES_AN5_LINK_OFFSET   (15)

#define HAL_CORAL_SERDES_PCS_CTRL_REG_1     (0x0A00)
#define SERDES_PCS_CTRL_REG_1_DEFAUT_VALUE  (0x0C9CC000)
#define HAL_CORAL_SERDES_FORCE_SYNC_DISABLE (0x2)

#define HAL_CORAL_SERDES_PCS_STATE_REG_2       (0x0B04)
#define HAL_CORAL_SERDES_PCS_RX_SYNC_MASK      (0x1)
#define HAL_CORAL_SERDES_PCS_RX_SYNC_OFFSET    (5)
#define HAL_CORAL_SERDES_PCS_RX_AN_DONE_MASK   (0x1)
#define HAL_CORAL_SERDES_PCS_RX_AN_DONE_OFFSET (0)

#define HAL_CORAL_SERDES_PCS_INT_STATE_REG  (0x0B5C)
#define HAL_CORAL_SERDES_PCS_INTR_AN_DONE   (0x1)
#define HAL_CORAL_SERDES_MODE_INTERRUPT_REG (0x0A20)
#define HAL_CORAL_SERDES_AN_DONE_CLEAR      (0x4)

#define HAL_CORAL_SERDES_RX100FX_PHY_STATUS        (0xB208)
#define HAL_CORAL_SERDES_RO_SYM_LOCK_DETECT_MASK   (0x1)
#define HAL_CORAL_SERDES_RO_SYM_LOCK_DETECT_OFFSET (8)
#define HAL_CORAL_SERDES_RO_ERR_DETECT_MASK        (4)
#define HAL_CORAL_SERDES_RO_ERR_DETECT_OFFSET      (12)

#define HAL_CORAL_SERDES_RX100FX_PHY_CTRL        (0xB200)
#define HAL_CORAL_SERDES_RG_LINK_CTRL_100_MASK   (2)
#define HAL_CORAL_SERDES_RG_LINK_CTRL_100_OFFSET (0)
#define HAL_CORAL_SERDES_RG_ERR_DETECT_CLR       (1U << 8)
#define HAL_CORAL_SERDES_RG_RCV_ERR_CNT_CLR      (1U << 16)

#define HAL_CORAL_SERDES_XSGMII_MSG_RX_STT_15   (0x843C)
#define HAL_CORAL_SERDES_RO_SPEED_STS_P0_MASK   (3)
#define HAL_CORAL_SERDES_RO_SPEED_STS_P0_OFFSET (0)

#define HAL_CORAL_SERDES_RX100FX_PHY_ERR_STATUS (0xB214)
#define HAL_CORAL_SERDES_RO_RCV_ERR_CNT_MASK    (8)
#define HAL_CORAL_SERDES_RO_RCV_ERR_CNT_OFFSET  (0)

#define HAL_CORAL_SERDES_RX_CTRL_44   (0xE6D8)
#define HAL_CORAL_SERDES_RO_100FX_RDY (1U << 19)

#define HAL_CORAL_SERDES_TX_DA_CTRL_1 (0xE004)
#define HAL_CORAL_SERDES_TX_BIT_INV   (0x80)
#define HAL_CORAL_SERDES_RX_AFIFO     (0xE61C)
#define HAL_CORAL_SERDES_RX_DATA_INV  (0x80)

#define HAL_CORAL_SERDES_RATE_ADP_P0_CTRL_REG_0 (0x9100)
#define HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN    (1UL << 30)
#define HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN    (1UL << 29)
#define HAL_CORAL_SERDES_RA_P0_MII_RA_RX_EN     (1UL << 3)
#define HAL_CORAL_SERDES_RA_P0_MII_RA_TX_EN     (1UL << 2)
#define HAL_CORAL_SERDES_RA_P0_MII_RA_RX_MODE   (1UL << 1)
#define HAL_CORAL_SERDES_RA_P0_MII_RA_TX_MODE   (1UL << 0)

#define HAL_CORAL_SERDES_DIG_MODE_CTRL_1   (0xA330)
#define HAL_CORAL_SERDES_TPHY_SPEED_MASK   (0xc)
#define HAL_CORAL_SERDES_TPHY_SPEED_SGMII  (0x0)
#define HAL_CORAL_SERDES_TPHY_SPEED_HSGMII (0x4)
#define HAL_CORAL_SERDES_TPHY_SPEED_QSGMII (0x8)
#define HAL_CORAL_SERDES_TPHY_SPEED_BASER  (0xC)

/* 100baseFX */
#define HAL_CORAL_SERDES_SGMII_MSG_TX_CTRL_0    (0x8000)
#define HAL_CORAL_SERDES_REG_FORCE_100FX_SIGDET (1UL << 28)

#define HAL_CORAL_SERDES_PLL_CTRL_0     (0xE400)
#define HAL_CORAL_SERDES_PHYA_AUTO_INIT (0x1)
#define HAL_CORAL_SERDES_POWER_DOWN     (0x4)
#define HAL_CORAL_SERDES_TX_DISABLE     (0x20)

#define HAL_CORAL_SERDES_RX_CTRL_14            (0xE660)
#define HAL_CORAL_SERDES_SIGDET_CAL_RDY_OFFSET (23)
#define HAL_CORAL_SERDES_SIGDET_CAL_RDY_MASK   (0x1)

#define HAL_CORAL_SERDES_SYS_CTRL_1             (0xE504)
#define HAL_CORAL_SERDES_FORCE_RG_VUSB10_ON_SEL (0x10000)

#define HAL_CORAL_SERDES_AN_REG_13        (0x0034)
#define HAL_CORAL_SERDES_SGMII_ENABLE     (0x01)
#define HAL_CORAL_SERDES_SGMII_FORCE_MODE (0x09)
#define HAL_CORAL_SERDES_LINK_SPEED_1000M (0x00000002)
#define HAL_CORAL_SERDES_LINK_SPEED_100M  (0x00000001)
#define HAL_CORAL_SERDES_LINK_SPEED_10M   (0x00000000)
#define HAL_CORAL_SERDES_LINK_DUPLEX_FULL (1)
#define HAL_CORAL_SERDES_LINK_DUPLEX_HALF (0)

#define HAL_CORAL_SERDES_10GB_T_PCS_STUS_1   (0x5930)
#define HAL_CORAL_SERDES_10GB_T_RX_LINK_STUS (0x1000)
#define HAL_CORAL_SERDES_10GB_T_RX_BLK_LK    (0x1)

/* SFP LED status config (pseudo register) */
/* LED1: b31 ~ b16     LED0: b15 ~ b0      */
/* Port9: b9 ~ b5      Port8: b4 ~ b0      */
/* b0   : PHY/Force mode                   */
/* b1~2 : Force state                      */
/* b3~4 : pattern config                   */
#define HAL_CORAL_SERDES_SFP_LED_CFG_REG           (0x1000A388) /* this is an unused register */
#define HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET   (0x0003)
#define HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET (0x0010)
#define HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_BIT     (0x0000)
#define HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_WIDTH   (0x0001)
#define HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_MASK    (0x0001)
#define HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_BIT   (0x0001)
#define HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_WIDTH (0x0002)
#define HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_MASK  (0x0003)
#define HAL_CORAL_SERDES_SFP_LED_PATT_CFG_BIT      (0x0003)
#define HAL_CORAL_SERDES_SFP_LED_PATT_CFG_WIDTH    (0x0002)
#define HAL_CORAL_SERDES_SFP_LED_PATT_CFG_MASK     (0x0003)

/* Wave-Gen pattern */
#define HAL_CORAL_SERDES_WAVE_GEN_PATTERN_HZ_HALF (0xfafa)
#define HAL_CORAL_SERDES_WAVE_GEN_PATTERN_HZ_ONE  (0x7d7d)
#define HAL_CORAL_SERDES_WAVE_GEN_PATTERN_HZ_TWO  (0x3e3e)

#define HAL_CORAL_SERDES_RX_CTRL_2 (0xE630)
#define HAL_CORAL_SERDES_RX_PWD    (1U << 31)

#define HAL_CORAL_SERDES_INTF_CTRL_6      (0xE318)
#define HAL_CORAL_SERDES_TX_DATA_EN_FORCE (1U << 12)
#define HAL_CORAL_SERDES_TX_DATA_EN_SEL   (1U << 13)
#define HAL_CORAL_SERDES_TX_LFPS_EN_FORCE (1U << 18)
#define HAL_CORAL_SERDES_TX_LFPS_EN_SEL   (1U << 19)

#define HAL_CORAL_SERDES_PLL_CL_CTRL_1      (0xE418)
#define HAL_CORAL_SERDES_PLL_FORCE_UNSTABLE (1U << 19)
#define HAL_CORAL_SERDES_PLL_FORCE_STABLE   (1U << 18)

/* PXP serdes register */
#define RX_HW_CTRL_0                   (0x10251990)
#define RG_XPON_FORCE_CDR_LCK2DATA_SEL (1U << 6)
#define RG_XPON_FORCE_CDR_LCK2DATA     (1U << 7)

#define RG_FORCE_DA_PXP_TX_HSDATA_EN   (0x1025187C)
#define RG_FORCE_DA_PXP_TX_DATA_EN     (1U << 16)
#define RG_FORCE_SEL_DA_PXP_TX_DATA_EN (1U << 24)
#define ADD_DIG_RESERVE_47             (0x10251958)
#define USGMII_TXPMA_SW_RST_N          (1U << 16)
#define USGMII_RXPMA_SW_RST_N          (1U << 17)
#define FX_EFIFO_SW_RST_N              (1U << 18)

#define SW_RST_SET          (0x10251460)
#define RG_SW_RX_FIFO_RST_N (1U << 0)
#define RG_SW_RX_RST_N      (1U << 1)
#define RG_SW_TX_FIFO_RST_N (1U << 6)

#define ADD_DIG_RESERVE_0    (0x10251360)
#define RG_RX_DATA_INV       (1U << 0)
#define XPON_SETTING_0       (0x10251270)
#define RG_TRANS_TX_DATA_INV (1U << 9)

#define RG_XPON_TX_CKLDO_EN     (0x102500C4)
#define RG_XPON_TX_CKLDO_ENABLE (1U << 0)

#define RG_FORCE_DA_PXP_TXPLL_CKOUT_EN (0x10251854)
#define RG_FORCE_DA_PXP_TXPLL_EN       (1U << 16)
#define RG_FORCE_SEL_DA_PXP_TXPLL_EN   (1U << 24)

#define ADD_XPON_MODE_1        (0x10251414)
#define RG_XFI_RX_MODE_OFFT    (9)
#define RG_XFI_RX_MODE_MASK    (3)
#define RG_XFI_RX_MODE_SGMII   (5)
#define RG_XFI_RX_MODE_HSGMII  (4)
#define RG_XFI_RX_MODE_USGMII  (7)
#define RG_XFI_RX_MODE_5GBASER (1)
#define RG_XFI_RX_MODE_USXGMII (0)

#define RX_HW_CTRL_3     (0x1025199C)
#define SPEED_SEL_1_OFFT (26)
#define SPEED_SEL_1_MASK (1)

#define RG_USXGMII_AN_CONTROL_0 (0x0EF8)
#define RG_AN_ENABLE_OFFT       (0)
#define RG_AN_ENABLE_MASK       (1)

#define RG_BASE100_FX_MODE   (0x1028C880)
#define RG_QP_100FX_MODE_EN  (1U << 0)
#define RG_PXP_100FX_MODE_EN (1U << 8)

#define DEBUG_DEC_ERRO_100FX (0x102519AC)
#define RO_FX_RDY            (1U << 5)

#define DEBUG_XPON_0               (0x102519A8)
#define DEBUG_RX_0_LCK2DATA_OFFSET (30)
#define DEBUG_RX_0_LCK2DATA_MASK   (0x1)

#define XPON_INT_STA_5     (0x102519A0)
#define RX_SIGDET_DOWN_INT (1U << 0)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef enum
{
    HAL_CORAL_SERDES_ID_S0 = 0,
    HAL_CORAL_SERDES_ID_S1,
    HAL_CORAL_SERDES_ID_LAST
} HAL_CORAL_SERDES_ID_T;

typedef enum
{
    HAL_CORAL_SERDES_POLARITY_NOSWAP = 0,
    HAL_CORAL_SERDES_POLARITY_SWAP_TX,
    HAL_CORAL_SERDES_POLARITY_SWAP_RX,
    HAL_CORAL_SERDES_POLARITY_SWAP_TXRX,
    HAL_CORAL_SERDES_POLARITY_LAST
} HAL_CORAL_SERDES_POLARITY_T;

typedef enum
{
    HAL_CORAL_SERDES_MODE_DEFAULT_VALUE = 0,
    HAL_CORAL_SERDES_MODE_QSGMII,
    HAL_CORAL_SERDES_MODE_HSGMII,
    HAL_CORAL_SERDES_MODE_SGMII_AN,
    HAL_CORAL_SERDES_MODE_SGMII_FORCE_1000M,
    HAL_CORAL_SERDES_MODE_SGMII_FORCE_100M,
    HAL_CORAL_SERDES_MODE_SGMII_FORCE_10M,
    HAL_CORAL_SERDES_MODE_1000BASEX,
    HAL_CORAL_SERDES_MODE_5GBASETR,
    HAL_CORAL_SERDES_MODE_100BASEFX,
    HAL_CORAL_SERDES_MODE_USXGMII,
    HAL_CORAL_SERDES_MODE_LAST,
} HAL_CORAL_SERDES_MODE_T;

typedef struct HAL_CORAL_SERDES_CFG_S
{
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
} HAL_CORAL_SERDES_CFG_T;

typedef struct HAL_CORAL_SERDES_CFG_INFO_S
{
    HAL_CORAL_SERDES_MODE_T       serdes_mode;
    UI32_T                        serdes_register_cnt;
    const HAL_CORAL_SERDES_CFG_T *ptr_mac_port_map;
} HAL_CORAL_SERDES_CFG_INFO_T;

typedef struct HAL_CORAL_SERDES_DBG_CFG_S
{
    UI8_T  operation;
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
    C8_T  *reg_name;
} HAL_CORAL_SERDES_DBG_CFG_T;

typedef struct HAL_CORAL_SERDES_DBG_CFG_INFO_S
{
    UI32_T                            serdes_interface;
    UI32_T                            serdes_register_cnt;
    const HAL_CORAL_SERDES_DBG_CFG_T *ptr_serdes_dbg_cfg_map;
} HAL_CORAL_SERDES_DBG_CFG_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   hal_coral_serdes_qp_init
 * PURPOSE:
 *      Serdes initialization
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
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
hal_coral_serdes_qp_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_coral_serdes_getDriver
 * PURPOSE:
 *      SERDES probe get driver
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
hal_coral_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_coral_serdes_qp_setAdminState
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
hal_coral_serdes_qp_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_coral_serdes_qp_getAdminState
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
hal_coral_serdes_qp_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_coral_serdes_qp_setAutoNego
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
hal_coral_serdes_qp_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_coral_serdes_qp_getAutoNego
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
hal_coral_serdes_qp_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_coral_serdes_qp_setLocalAdvAbility
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
hal_coral_serdes_qp_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_qp_getLocalAdvAbility
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
hal_coral_serdes_qp_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_qp_setSpeed
 * PURPOSE:
 *      This API is used to set serdes speed.
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
hal_coral_serdes_qp_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_coral_serdes_qp_getSpeed
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
hal_coral_serdes_qp_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *prt_speed);

/* FUNCTION NAME:   hal_coral_serdes_qp_getRemoteAdvAbility
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
hal_coral_serdes_qp_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_qp_getLinkStatus
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
hal_coral_serdes_qp_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_coral_serdes_qp_setSerdesMode
 * PURPOSE:
 *      This API is used to set port serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      serdes_mode     --  Serdes mode
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
hal_coral_serdes_qp_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   hal_coral_serdes_qp_getSerdesMode
 * PURPOSE:
 *      This API is used to get port serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_serdes_mode --  Serdes mode
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
hal_coral_serdes_qp_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME:   hal_coral_serdes_qp_dumpPortCnt
 * PURPOSE:
 *      Dump port debug counter.
 *
 * INPUT:
 *      unit                --  Device unit number
 *      port                --  Port number
 *      type                --  Phy type
 *      param               --  Parameter
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_serdes_qp_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);

/* FUNCTION NAME:   hal_coral_serdes_pxp_init
 * PURPOSE:
 *      Serdes initialization
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
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
hal_coral_serdes_pxp_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   hal_coral_serdes_pxp_setAdminState
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
hal_coral_serdes_pxp_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getAdminState
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
hal_coral_serdes_pxp_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_coral_serdes_pxp_setAutoNego
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
hal_coral_serdes_pxp_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getAutoNego
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
hal_coral_serdes_pxp_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_coral_serdes_pxp_setLocalAdvAbility
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
hal_coral_serdes_pxp_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getLocalAdvAbility
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
hal_coral_serdes_pxp_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_pxp_setSpeed
 * PURPOSE:
 *      This API is used to set serdes speed.
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
hal_coral_serdes_pxp_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getSpeed
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
hal_coral_serdes_pxp_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *prt_speed);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getRemoteAdvAbility
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
hal_coral_serdes_pxp_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getLinkStatus
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
hal_coral_serdes_pxp_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_coral_serdes_pxp_setSerdesMode
 * PURPOSE:
 *      This API is used to set port serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      serdes_mode     --  Serdes mode
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
hal_coral_serdes_pxp_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   hal_coral_serdes_pxp_getSerdesMode
 * PURPOSE:
 *      This API is used to get port serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_serdes_mode --  Serdes mode
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
hal_coral_serdes_pxp_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME:   hal_coral_serdes_pxp_dumpPortCnt
 * PURPOSE:
 *      Dump port debug counter.
 *
 * INPUT:
 *      unit                --  Device unit number
 *      port                --  Port number
 *      type                --  Phy type
 *      param               --  Parameter
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_serdes_pxp_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);

#endif /* end of HAL_CORAL_SERDES_H */
