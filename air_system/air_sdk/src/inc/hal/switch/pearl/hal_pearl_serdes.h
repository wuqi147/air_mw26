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

/* FILE NAME:  hal_pearl_serdes.h
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_PEARL_SERDES_H
#define HAL_PEARL_SERDES_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define HAL_PEARL_SERDES_PORT_MCR_BASE_ADDR (0x10210000)
#define HAL_PEARL_SERDES_PORT_ADDR_OFFSET   (0x200)

#define HAL_PEARL_SERDES_NPSCU_BASE_ADDR (0x10005000)
#define HAL_PEARL_SERDES_NPSCU_RST_CTRL2 (0x00C4)
#define HSGMII_HSI0_RESET_BIT_OFFSET     (13)

#define HAL_PEARL_SERDES_ID_S0_BASE_ADDR (0x10220000)
#define HAL_PEARL_SERDES_ADDR_OFFSER     (0x10000)

#define HAL_PEARL_SERDES_AN_REG_0           (0x0000)
#define HAL_PEARL_SERDES_AN0_ENABLE_OFFSET  (12)
#define HAL_PEARL_SERDES_AN0_ENABLE_MASK    (0x1)
#define HAL_PEARL_SERDES_AN_REG_0_RESET     (0x8000) /* Serdes reset phy */
#define HAL_PEARL_SERDES_AN_REG_0_ANRESTART (0x0200) /* Serdes auto negotation restart */
#define HAL_PEARL_SERDES_AN_REG_0_ANENABLE  (0x1000) /* Serdes enable auto negotation */

#define HAL_PEARL_SERDES_AN_REG_4      (0x0010)
#define SGMII_1000BASEX_PAUSE_OFFSET   (7)
#define SGMII_1000BASEX_ASM_DIR_OFFSET (8)

#define HAL_PEARL_SERDES_AN_REG_5          (0x0014)
#define HAL_PEARL_SERDES_AN5_SPEED_MASK    (0x3)
#define HAL_PEARL_SERDES_AN5_SPEED_OFFSET  (10)
#define HAL_PEARL_SERDES_AN5_DUPLEX_MASK   (0x1)
#define HAL_PEARL_SERDES_AN5_DUPLEX_OFFSET (12)
#define HAL_PEARL_SERDES_AN5_LINK_MASK     (0x1)
#define HAL_PEARL_SERDES_AN5_LINK_OFFSET   (15)

#define HAL_PEARL_SERDES_PCS_CTRL_REG_1     (0x0A00)
#define SERDES_PCS_CTRL_REG_1_DEFAUT_VALUE  (0x0C9CC000)
#define HAL_PEARL_SERDES_FORCE_SYNC_DISABLE (0x2)

#define HAL_PEARL_SERDES_PCS_STATE_REG_2    (0x0B04)
#define HAL_PEARL_SERDES_PCS_RX_SYNC_MASK   (0x1)
#define HAL_PEARL_SERDES_PCS_RX_SYNC_OFFSET (5)
#define HAL_PEARL_SERDES_PCS_RX_AN_DONE     (0x1)

#define HAL_PEARL_SERDES_PCS_INT_STATE_REG  (0x0B5C)
#define HAL_PEARL_SERDES_PCS_INTR_AN_DONE   (0x1)
#define HAL_PEARL_SERDES_MODE_INTERRUPT_REG (0x0A20)
#define HAL_PEARL_SERDES_AN_DONE_CLEAR      (0x4)

#define HAL_PEARL_SERDES_MII_RA_AN_ENABLE (0x6300)

// TODO for polarity
#define HAL_PEARL_SERDES_PHYA_REG_62  (0xA0F8)
#define HAL_PEARL_SERDES_TX_DA_CTRL_1 (0xE004)
#define HAL_PEARL_SERDES_TX_BIT_INV   (0x80)
#define HAL_PEARL_SERDES_RX_AFIFO     (0xE61C)
#define HAL_PEARL_SERDES_RX_DATA_INV  (0x80)

#define HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0 (0x6100)
#define HAL_PEARL_SERDES_RA_P0_MII_RX_EEE_EN    (1UL << 30)
#define HAL_PEARL_SERDES_RA_P0_MII_TX_EEE_EN    (1UL << 29)
#define HAL_PEARL_SERDES_RA_P0_MII_RA_RX_EN     (1UL << 3)
#define HAL_PEARL_SERDES_RA_P0_MII_RA_TX_EN     (1UL << 2)
#define HAL_PEARL_SERDES_RA_P0_MII_RA_RX_MODE   (1UL << 1)
#define HAL_PEARL_SERDES_RA_P0_MII_RA_TX_MODE   (1UL << 0)

#define HAL_PEARL_SERDES_DIG_MODE_CTRL_1   (0xA330)
#define HAL_PEARL_SERDES_TPHY_SPEED_MASK   (0xc)
#define HAL_PEARL_SERDES_TPHY_SPEED_SGMII  (0x0)
#define HAL_PEARL_SERDES_TPHY_SPEED_HSGMII (0x4)
#define HAL_PEARL_SERDES_TPHY_SPEED_QSGMII (0x8)
#define HAL_PEARL_SERDES_TPHY_SPEED_BASER  (0xC)

#define HAL_PEARL_SERDES_PLL_CTRL_0     (0xE400)
#define HAL_PEARL_SERDES_PHYA_AUTO_INIT (0x1)
#define HAL_PEARL_SERDES_POWER_DOWN     (0x4)
#define HAL_PEARL_SERDES_TX_DISABLE     (0x20)

#define HAL_PEARL_SERDES_RX_CTRL_14            (0xE660)
#define HAL_PEARL_SERDES_SIGDET_CAL_RDY_OFFSET (23)
#define HAL_PEARL_SERDES_SIGDET_CAL_RDY_MASK   (0x1)

#define HAL_PEARL_SERDES_SYS_CTRL_1             (0xE504)
#define HAL_PEARL_SERDES_FORCE_RG_VUSB10_ON_SEL (0x10000)

#define HAL_PEARL_SERDES_AN_REG_13        (0x0034)
#define HAL_PEARL_SERDES_SGMII_ENABLE     (0x01)
#define HAL_PEARL_SERDES_SGMII_FORCE_MODE (0x09)
#define HAL_PEARL_SERDES_LINK_SPEED_1000M (0x00000002)
#define HAL_PEARL_SERDES_LINK_SPEED_100M  (0x00000001)
#define HAL_PEARL_SERDES_LINK_SPEED_10M   (0x00000000)
#define HAL_PEARL_SERDES_LINK_DUPLEX_FULL (1)
#define HAL_PEARL_SERDES_LINK_DUPLEX_HALF (0)

#define HAL_PEARL_SERDES_MSG_RX_LIK_STS_0     (0x4514)
#define HAL_PEARL_SERDES_HSGMII_LINK_STS_MASK (0x1f)
#define HAL_PEARL_SERDES_HSGMII3_FC_RX_ON     (0x02)
#define HAL_PEARL_SERDES_HSGMII3_FC_TX_ON     (0x04)
#define HAL_PEARL_SERDES_FULL_DUPLEX          (0x01)

#define HAL_PEARL_SERDES_MSG_RX_LIK_STS_2 (0x451C)

#define HAL_PEARL_SERDES_SGMII_STS_CTRL_0 (0x4018)

#define HAL_PEARL_SERDES_10GB_T_PCS_STUS_1   (0x5930)
#define HAL_PEARL_SERDES_10GB_T_RX_LINK_STUS (0x1000)
#define HAL_PEARL_SERDES_10GB_T_RX_BLK_LK    (0x1)

#define HAL_PEARL_SERDES_SFP_LED_CFG_REG           (0x1000A388) /* this is an unused register */
#define HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET   (0x0003)
#define HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET (0x0010)
#define HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_BIT     (0x0000)
#define HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_WIDTH   (0x0001)
#define HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_MASK    (0x0001)
#define HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_BIT   (0x0001)
#define HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_WIDTH (0x0002)
#define HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_MASK  (0x0003)

#define HAL_PEARL_SERDES_SFP_LED_PATT_CFG_BIT   (0x0003)
#define HAL_PEARL_SERDES_SFP_LED_PATT_CFG_WIDTH (0x0002)
#define HAL_PEARL_SERDES_SFP_LED_PATT_CFG_MASK  (0x0003)

/* Wave-Gen pattern */
#define HAL_PEARL_SERDES_WAVE_GEN_PATTERN_HZ_HALF (0xfafa)
#define HAL_PEARL_SERDES_WAVE_GEN_PATTERN_HZ_ONE  (0x7d7d)
#define HAL_PEARL_SERDES_WAVE_GEN_PATTERN_HZ_TWO  (0x3e3e)

#define HAL_PEARL_SERDES_RX_CTRL_2 (0xE630)
#define HAL_PEARL_SERDES_RX_PWD    (1U << 31)

#define HAL_PEARL_SERDES_INTF_CTRL_6      (0xE318)
#define HAL_PEARL_SERDES_TX_DATA_EN_FORCE (1U << 12)
#define HAL_PEARL_SERDES_TX_DATA_EN_SEL   (1U << 13)

#define HAL_PEARL_SERDES_PLL_CL_CTRL_1      (0xE418)
#define HAL_PEARL_SERDES_PLL_FORCE_UNSTABLE (1U << 19)
#define HAL_PEARL_SERDES_PLL_FORCE_STABLE   (1U << 18)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    HAL_PEARL_SERDES_ID_S0 = 0,
    HAL_PEARL_SERDES_ID_S1,
    HAL_PEARL_SERDES_ID_S2,
    HAL_PEARL_SERDES_ID_S3,
    HAL_PEARL_SERDES_ID_S4,
    HAL_PEARL_SERDES_ID_LAST
} HAL_PEARL_SERDES_ID_T;

typedef enum
{
    HAL_PEARL_SERDES_POLARITY_NOSWAP = 0,
    HAL_PEARL_SERDES_POLARITY_SWAP_TX,
    HAL_PEARL_SERDES_POLARITY_SWAP_RX,
    HAL_PEARL_SERDES_POLARITY_SWAP_TXRX,
    HAL_PEARL_SERDES_POLARITY_LAST
} HAL_PEARL_SERDES_POLARITY_T;

typedef enum
{
    HAL_PEARL_SERDES_MODE_DEFAULT_VALUE = 0,
    HAL_PEARL_SERDES_MODE_QSGMII,
    HAL_PEARL_SERDES_MODE_HSGMII,
    HAL_PEARL_SERDES_MODE_SGMII_AN,
    HAL_PEARL_SERDES_MODE_SGMII_FORCE_1000M,
    HAL_PEARL_SERDES_MODE_SGMII_FORCE_100M,
    HAL_PEARL_SERDES_MODE_SGMII_FORCE_10M,
    HAL_PEARL_SERDES_MODE_1000BASEX,
    HAL_PEARL_SERDES_MODE_5GBASETR,
    HAL_PEARL_SERDES_MODE_LAST,
} HAL_PEARL_SERDES_MODE_T;

typedef struct HAL_PEARL_SERDES_CFG_S
{
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
} HAL_PEARL_SERDES_CFG_T;

typedef struct HAL_PEARL_SERDES_CFG_INFO_S
{
    HAL_PEARL_SERDES_MODE_T       serdes_mode;
    UI32_T                        serdes_register_cnt;
    const HAL_PEARL_SERDES_CFG_T *ptr_mac_port_map;
} HAL_PEARL_SERDES_CFG_INFO_T;

typedef struct HAL_PEARL_SERDES_DBG_CFG_S
{
    UI8_T  operation;
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
    C8_T  *reg_name;
} HAL_PEARL_SERDES_DBG_CFG_T;

typedef struct HAL_PEARL_SERDES_DBG_CFG_INFO_S
{
    UI32_T                            serdes_interface;
    UI32_T                            serdes_register_cnt;
    const HAL_PEARL_SERDES_DBG_CFG_T *ptr_serdes_dbg_cfg_map;
} HAL_PEARL_SERDES_DBG_CFG_INFO_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS */

/* EXPORTED SUBPROGRAM BODIES*/
/* FUNCTION NAME:   hal_pearl_serdes_init
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
hal_pearl_serdes_init(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_pearl_serdes_getDriver
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
hal_pearl_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

/* FUNCTION NAME:   hal_pearl_serdes_setAdminState
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
hal_pearl_serdes_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_pearl_serdes_getAdminState
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
hal_pearl_serdes_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_pearl_serdes_setAutoNego
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
hal_pearl_serdes_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_pearl_serdes_getAutoNego
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
hal_pearl_serdes_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_pearl_serdes_setLocalAdvAbility
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
hal_pearl_serdes_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_pearl_serdes_getLocalAdvAbility
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
hal_pearl_serdes_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_pearl_serdes_setSpeed
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
hal_pearl_serdes_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_pearl_serdes_getSpeed
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
hal_pearl_serdes_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *prt_speed);

/* FUNCTION NAME:   hal_pearl_serdes_getRemoteAdvAbility
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
hal_pearl_serdes_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_pearl_serdes_getLinkStatus
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
hal_pearl_serdes_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_pearl_serdes_setSerdesMode
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
hal_pearl_serdes_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   hal_pearl_serdes_getSerdesMode
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
hal_pearl_serdes_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedCtrlMode
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
hal_pearl_serdes_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedCtrlMode
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
hal_pearl_serdes_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedForceState
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
hal_pearl_serdes_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedForceState
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
hal_pearl_serdes_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedForcePattCfg
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
hal_pearl_serdes_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedForcePattCfg
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
hal_pearl_serdes_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME:   hal_pearl_serdes_dumpPortCnt
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
hal_pearl_serdes_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);
#endif /* end of HAL_PEARL_SERDES_H */
