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

/* FILE NAME:  hal_sco_serdes.h
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_SERDES_H
#define HAL_SCO_SERDES_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define PORT_MCR_BASE_ADDR (0x10210000)
#define PORT_ADDR_OFFSET   (0x200)

#define PORT_FORCE_MODE_MASK   (0x20000)
#define PORT_FORCE_SPEED_2500M (0xC)
#define PORT_FORCE_FULL_DUPLEX (0x2)
#define PORT_FLOWCTRL_TX_EN    (0x10)
#define PORT_FLOWCTRL_RX_EN    (0x20)
#define PORT_EEE_100M_EN       (0x40)
#define PORT_EEE_1G_EN         (0x80)
#define PORT_EEE_2P5G_EN       (0x100)

#define CHIPSCU_BASE_ADDR (0x10000000)
#define NPSCU_BASE_ADDR   (0x10005000)

#define SERDES_ID_S0_BASE_ADDR (0x10220000)
#define SERDES_ADDR_OFFSER     (0x10000)

#define QSGMII_LANE_PORT_NUM (0x4)
#define QSGMII_LANE_OFFSET   (0x1000)

#define CHIPSCU_PHY_MODE_SEL          (0x0128)
#define NPSCU_RST_CTRL2               (0x00C4)
#define SERDES_AN_REG_0               (0x0000)
#define SERDES_AN_REG_0_RESET         (0x8000) /* Serdes reset phy */
#define SERDES_AN_REG_0_ANRESTART     (0x0200) /* Serdes auto negotation restart */
#define SERDES_AN_REG_0_ANENABLE      (0x1000) /* Serdes enable auto negotation */
#define SERDES_AN_REG_4               (0x0010)
#define SERDES_AN_REG_5               (0x0014)
#define SERDES_AN_REG_13              (0x0034)
#define SERDES_PCS_CTRL_REG_1         (0x0A00)
#define SERDES_PCS_CTRL_REG_6         (0x0A14)
#define SERDES_PCS_STATE_REG_2        (0x0B04)
#define SERDES_PCS_STATE_REG_9        (0x0B58)
#define SERDES_PCS_INT_STATE_REG      (0x0B5C)
#define SERDES_MODE_INTERRUPT_REG     (0x0A20)
#define SERDES_MSG_TX_CTRL_REG_0      (0x4000)
#define SERDES_MSG_TX_CTRL_REG_1      (0x4004)
#define SERDES_SGMII_STS_CTL_REG_0    (0x4018)
#define SERDES_MSG_RX_CTRL_0          (0x4100)
#define SERDES_MSG_RX_CTRL_REG_4      (0x450C)
#define SERDES_MSG_RX_LIK_STS_0       (0x4514)
#define SERDES_MSG_RX_LIK_STS_2       (0x451C)
#define SERDES_RATE_ADAPT_CTRL_0      (0x6000)
#define SERDES_XGMII_DBG_0            (0x60E0)
#define SERDES_XGMII_DBG_1            (0x60E4)
#define SERDES_RATE_ADP_P0_CTRL_REG_0 (0x6100)
#define SERDES_RA_P0_MII_RX_EEE_EN    (1UL << 30)
#define SERDES_RA_P0_MII_TX_EEE_EN    (1UL << 29)
#define SERDES_RA_P0_MII_MODE         (1UL << 28)
#define SERDES_RA_P0_MII_RA_RX_EN     (1UL << 3)
#define SERDES_RA_P0_MII_RA_TX_EN     (1UL << 2)
#define SERDES_RA_P0_MII_RA_RX_MODE   (1UL << 1)
#define SERDES_RA_P0_MII_RA_TX_MODE   (1UL << 0)

#define SERDES_RATE_ADP_P0_CTRL_REG_1 (0x6104)
#define SERDES_RA_P0_RX_RD_THR_LENGTH (5)
#define SERDES_RA_P0_RX_RD_THR_OFFSET (24)
#define SERDES_RA_P0_RX_WR_THR_LENGTH (5)
#define SERDES_RA_P0_RX_WR_THR_OFFSET (16)
#define SERDES_RA_P0_TX_RD_THR_LENGTH (5)
#define SERDES_RA_P0_TX_RD_THR_OFFSET (8)
#define SERDES_RA_P0_TX_WR_THR_LENGTH (5)
#define SERDES_RA_P0_TX_WR_THR_OFFSET (0)

#define SERDES_RATE_ADP_P1_CTRL_REG_0 (0x6108)
#define SERDES_RATE_ADP_P2_CTRL_REG_0 (0x6110)
#define SERDES_RATE_ADP_P3_CTRL_REG_0 (0x6118)
#define SERDES_MII_RA_AN_ENABLE       (0x6300)
#define SERDES_PHYA_REG_6             (0xA018)
#define SERDES_PHYA_REG_7             (0xA01C)
#define SERDES_PHYA_REG_8             (0xA020)
#define SERDES_PHYA_REG_9             (0xA024)
#define SERDES_PHYA_REG_11            (0xA02C)
#define SERDES_PHYA_REG_13            (0xA034)
#define SERDES_PHYA_REG_14            (0xA038)
#define SERDES_PHYA_REG_15            (0xA03C)
#define SERDES_PHYA_REG_19            (0xA04C)
#define SERDES_PHYA_REG_24            (0xA060)
#define SERDES_PHYA_REG_28            (0xA070)
#define SERDES_PHYA_REG_30            (0xA078)
#define SERDES_PHYA_REG_61            (0xA0F4)
#define SERDES_PHYA_REG_62            (0xA0F8)
#define SERDES_PHYA_REG_80            (0xA140)

#define SERDES_RO_AD_INTERFACE    (0xA504)
#define SERDES_RO_SIGDET_OUT_OFFT (24)
#define SERDES_RO_SIGDET_OUT_MASK (1)

#define SERDES_RX100FX_PHY_CTRL     (0x7200)
#define SERDES_RX100FX_PHY_STATUS   (0x7208)
#define SERDES_SGMII_MSG_TX_CTRL_0  (0x4000)
#define SERDES_SGMII_REG_PHYA_65    (0x4104)
#define SERDES_SGMII_FPGA_MODE_CONT (0x4154)
#define REG_100FX_TBI_REVERSE       (0x0F)
#define REG_100FX_TBI_REVERSE_OFF   (0x00)
#define REG_100FX_TBI_REVERSE_LENG  (0x04)
#define REG_HWTRAP_100FX_MUX_EN     (1UL << 3)
#define REG_HWTRAP_100FX_MUX        (1UL << 2)
#define REG_FORCE_100FX_SIGDET      (1UL << 28)
#define REG_FORCE_100FX_SYM_LOCK    (1UL << 8)
#define REG_100FX_LINK_CTRL_DUPLEX  (1UL << 2)
#define REG_100FX_LINK_CTRL_DISABLE (0x00)
#define REG_100FX_LINK_CTRL_ENABLE  (0x03)
#define REG_100FX_LINK_CTRL_OFF     (0x00)
#define REG_100FX_LINK_CTRL_LENG    (0x02)
#define REG_SSUSB_LN0_RXAFE_RESERVE (0x3 << 20)
#define REG_PCIE_LN0_SIGDET_LPF     (0x1 << 17)
#define REG_FORCE_CDR_LCK2REF       (0x1 << 5)
#define REG_SSUSB_CDR_LCK2REF       (0x1 << 23)

#define EFIFO_MODE_100_FX_BASE  (0x10278000)
#define EFIFO_MODE_100_FX_REG   (0x0000)
#define EFIFO_CTRL_100_FX_REG   (0x0004)
#define EFIFO_100_FX_REG_OFFSET (0x1000)
#define EFIFO_100_FX_MODE       (0x00)
#define EFIFO_100_FX_CTRL       (0x0f)

#define LPA_AN_100M  (0x1)
#define LPA_AN_1000M (0x2)

#define SERDES_SPEED_MASK         (0xc)
#define SERDES_SPEED_SETUP_SGMII  (0x0)
#define SERDES_SPEED_SETUP_HSGMII (0x4)
#define SERDES_SPEED_SETUP_QSGMII (0x8)

#define SERDES_RX_SYNC (0x20)

#define SERDES_POWER_DOWN         (0x400)
#define SERDES_TX_DISABLE         (0x2000)
#define SERDES_SGMII_ENABLE       (0x01)
#define SERDES_SGMII_FORCE_MODE   (0x09)
#define SERDES_FORCE_SYNC_DISABLE (0x2)

#define MSG_RX_LIK_STS_FC_BYPASS (0x0f)
#define HSGMII_LINK_STS_MASK     (0x1f)
#define HSGMII3_FC_ON            (0x06)
#define HSGMII3_FC_RX_ON         (0x02)
#define HSGMII3_FC_TX_ON         (0x04)
#define HSGMII3_FC_OFF           (0x00)
#define SERDES_FULL_DUPLEX       (0x01)

#define SERDES_MII_EEE_ENABLE  (0x70000000)
#define SERDES_MII_EEE_DISABLE (0x10000000)

#define SERDES_SFP_LED_CFG_REG           (0x1000A388) /* this is an unused register */
#define SERDES_SFP_LED_CFG_PORT_OFFSET   (0x0003)
#define SERDES_SFP_LED_CFG_LED_ID_OFFSET (0x0010)

#define SERDES_SFP_LED_CTRL_MODE_BIT     (0x0000)
#define SERDES_SFP_LED_CTRL_MODE_WIDTH   (0x0001)
#define SERDES_SFP_LED_CTRL_MODE_MASK    (0x0001)
#define SERDES_SFP_LED_FORCE_STATE_BIT   (0x0001)
#define SERDES_SFP_LED_FORCE_STATE_WIDTH (0x0002)
#define SERDES_SFP_LED_FORCE_STATE_MASK  (0x0003)

#define SERDES_SFP_LED_PATT_CFG_BIT   (0x0003)
#define SERDES_SFP_LED_PATT_CFG_WIDTH (0x0002)
#define SERDES_SFP_LED_PATT_CFG_MASK  (0x0003)

#define HAL_SERDES_RX100FX_PHY_ERR_STATUS (0x7214)
#define HAL_SERDES_RO_RCV_ERR_CNT_MASK    (8)
#define HAL_SERDES_RO_RCV_ERR_CNT_OFFSET  (0)

#define HAL_SERDES_RX100FX_PHY_STATUS   (0x7208)
#define HAL_SERDES_RO_ERR_DETECT_MASK   (4)
#define HAL_SERDES_RO_ERR_DETECT_OFFSET (12)

#define HAL_SERDES_RX100FX_PHY_CTRL   (0x7200)
#define HAL_SERDES_RG_ERR_DETECT_CLR  (1U << 8)
#define HAL_SERDES_RG_RCV_ERR_CNT_CLR (1U << 16)

/* Wave-Gen pattern */
#define WAVE_GEN_PATTERN_HZ_HALF (0xfafa)
#define WAVE_GEN_PATTERN_HZ_ONE  (0x7d7d)
#define WAVE_GEN_PATTERN_HZ_TWO  (0x3e3e)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SERDES_ID_S0 = 0,
    SERDES_ID_S1,
    SERDES_ID_S2,
    SERDES_ID_S3,
    SERDES_ID_S4,
    SERDES_ID_LAST
} SERDES_ID_T;

typedef enum
{
    SERDES_POLARITY_NOSWAP = 0,
    SERDES_POLARITY_SWAP_TX,
    SERDES_POLARITY_SWAP_RX,
    SERDES_POLARITY_SWAP_TXRX,
    SERDES_POLARITY_LAST
} SERDES_POLARITY_T;

typedef enum
{
    SERDES_MODE_REGISTER_ADDRESS = 0,
    SERDES_MODE_QSGMII,
    SERDES_MODE_HSGMII,
    SERDES_MODE_SGMII_AN,
    SERDES_MODE_SGMII_FORCE_1000M,
    SERDES_MODE_SGMII_FORCE_100M,
    SERDES_MODE_SGMII_FORCE_10M,
    SERDES_MODE_LAST
} SERDES_MODE_T;

typedef enum
{
    SERDES_QSGMII_MODE_SWITCH = 0,
    SERDES_QSGMII_MODE_PHY,
    SERDES_QSGMII_MODE_COMBO,
    SERDES_QSGMII_MODE_LAST
} SERDES_QSGMII_MODE_T;

typedef struct HAL_SCO_SERDES_DBG_CFG_S
{
    UI8_T  operation;
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
    C8_T  *reg_name;
} HAL_SCO_SERDES_DBG_CFG_T;

typedef struct HAL_SCO_SERDES_DBG_CFG_INFO_S
{
    UI32_T                          serdes_interface;
    UI32_T                          serdes_register_cnt;
    const HAL_SCO_SERDES_DBG_CFG_T *ptr_serdes_dbg_cfg_map;
} HAL_SCO_SERDES_DBG_CFG_INFO_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS */

/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES*/
/* FUNCTION NAME:   hal_sco_serdes_init
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
hal_sco_serdes_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_sco_serdes_getDriver
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
hal_sco_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_sco_serdes_setAdminState
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
hal_sco_serdes_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_sco_serdes_getAdminState
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
hal_sco_serdes_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_sco_serdes_setAutoNego
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
 *      AIR_E_OP_INVALID
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_sco_serdes_getAutoNego
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
 *      AIR_E_OP_INVALID
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_sco_serdes_setLocalAdvAbility
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
hal_sco_serdes_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_sco_serdes_getLocalAdvAbility
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
hal_sco_serdes_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_sco_serdes_setSpeed
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
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_sco_serdes_getSpeed
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
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *prt_speed);

/* FUNCTION NAME:   hal_sco_serdes_getRemoteAdvAbility
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
hal_sco_serdes_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_sco_serdes_getLinkStatus
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
hal_sco_serdes_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_sco_serdes_setSerdesMode
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
hal_sco_serdes_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   hal_sco_serdes_getSerdesMode
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
hal_sco_serdes_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to set port led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      ctrl_mode       --  LED control mode
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
hal_sco_serdes_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to get port led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *
 * OUTPUT:
 *      ptr_ctrl_mode   --  LED control mode
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedForceState
 * PURPOSE:
 *      This API is used to set port led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      state           --  LED force state
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
hal_sco_serdes_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedForceState
 * PURPOSE:
 *      This API is used to get port led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_state       --  LED force state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to set phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      pattern         --  LED force pattern
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
hal_sco_serdes_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to get phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_serdes_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME:   hal_sco_serdes_dumpPortCnt
 * PURPOSE:
 *      Dump port debug counter.
 *
 * INPUT:
 *      unit                    --  Device unit number
 *      port                    --  Port number
 *      type                    --  Phy type
 *      param                   --  Parameter
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
hal_sco_serdes_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);

#endif /* end of HAL_SCO_SERDES_H */
