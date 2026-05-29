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

/* FILE NAME:  hal_phy.h
 * PURPOSE:
 *      It provides PHY module API.
 * NOTES:
 */

#ifndef HAL_PHY_H
#define HAL_PHY_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
#include <aml/aml.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* PHY access type */
#define HAL_PHY_ACCESS_TYPE_MDIO     (0x00)
#define HAL_PHY_ACCESS_TYPE_I2C_MDIO (0x01)

/* Generic MII registers. */
#define MII_BMCR            (0x00) /* Basic mode control register      */
#define MII_BMSR            (0x01) /* Basic mode status register       */
#define MII_PHYSID1         (0x02) /* PHYS ID 1                        */
#define MII_PHYSID2         (0x03) /* PHYS ID 2                        */
#define MII_ADVERTISE       (0x04) /* Advertisement control reg        */
#define MII_LPA             (0x05) /* Link partner ability reg         */
#define MII_EXPANSION       (0x06) /* Expansion register               */
#define MII_CTRL1000        (0x09) /* 1000BASE-T control               */
#define MII_STAT1000        (0x0a) /* 1000BASE-T status                */
#define MII_MMD_CTRL        (0x0d) /* MMD Access Control Register      */
#define MII_MMD_DATA        (0x0e) /* MMD Access Data Register         */
#define MII_ESTATUS         (0x0f) /* Extended Status                  */
#define MII_LINKSTATUS      (0x11) /* Link Status                      */
#define MII_DCOUNTER        (0x12) /* Disconnect counter               */
#define MII_FCSCOUNTER      (0x13) /* False carrier counter            */
#define MII_NWAYTEST        (0x14) /* N-way auto-neg test reg          */
#define MII_RERRCOUNTER     (0x15) /* Receive error counter            */
#define MII_SREVISION       (0x16) /* Silicon revision                 */
#define MII_RESV1           (0x17) /* Reserved...                      */
#define MII_LBRERROR        (0x18) /* Lpback, rx, bypass error         */
#define MII_PHYADDR         (0x19) /* PHY address                      */
#define MII_RESV2           (0x1a) /* Reserved...                      */
#define MII_TPISTATUS       (0x1b) /* TPI status for 10mbps            */
#define MII_NCONFIG         (0x1c) /* Network interface config         */
#define MII_AUX_CTRL_STA    (0x1d) /* Auxiliary control and status     */
#define MII_PHY_REV_ID_OFFT (0)    /* PHY revision ID offset           */
#define MII_PHY_REV_ID_LENG (4)    /* PHY revision ID length           */
#define MII_PHY_LSB_ID_OFFT (4)    /* PHY ID LSB offset                */
#define MII_PHY_LSB_ID_LENG (12)   /* PHY ID LSB length                */

/* Reg00h  Basic mode control register. */
#define BMCR_RESV      (0x003f) /* Unused...                        */
#define BMCR_SPEED1000 (0x0040) /* MSB of Speed (1000)              */
#define BMCR_CTST      (0x0080) /* Collision test                   */
#define BMCR_FULLDPLX  (0x0100) /* Full duplex                      */
#define BMCR_ANRESTART (0x0200) /* Auto negotiation restart         */
#define BMCR_ISOLATE   (0x0400) /* Isolate data paths from MII      */
#define BMCR_PDOWN     (0x0800) /* Enable low power state           */
#define BMCR_ANENABLE  (0x1000) /* Enable auto negotiation          */
#define BMCR_SPEED100  (0x2000) /* Select 100Mbps                   */
#define BMCR_LOOPBACK  (0x4000) /* TXD loopback bits                */
#define BMCR_RESET     (0x8000) /* Reset to default state           */

/* Reg01h Basic mode status register. */
#define BMSR_ERCAP        (0x0001) /* Ext-reg capability               */
#define BMSR_JCD          (0x0002) /* Jabber detected                  */
#define BMSR_LSTATUS      (0x0004) /* Link status                      */
#define BMSR_ANEGCAPABLE  (0x0008) /* Able to do auto-negotiation      */
#define BMSR_RFAULT       (0x0010) /* Remote fault detected            */
#define BMSR_ANEGCOMPLETE (0x0020) /* Auto-negotiation complete        */
#define BMSR_RESV         (0x00c0) /* Unused...                        */
#define BMSR_ESTATEN      (0x0100) /* Extended Status in R15           */
#define BMSR_100HALF2     (0x0200) /* Can do 100BASE-T2 HDX            */
#define BMSR_100FULL2     (0x0400) /* Can do 100BASE-T2 FDX            */
#define BMSR_10HALF       (0x0800) /* Can do 10mbps, half-duplex       */
#define BMSR_10FULL       (0x1000) /* Can do 10mbps, full-duplex       */
#define BMSR_100HALF      (0x2000) /* Can do 100mbps, half-duplex      */
#define BMSR_100FULL      (0x4000) /* Can do 100mbps, full-duplex      */
#define BMSR_100BASE4     (0x8000) /* Can do 100mbps, 4k packets       */

/* Reg04h  Advertisement control register. */
#define ADVERTISE_SLCT          (0x001f) /* Selector bits                    */
#define ADVERTISE_CSMA          (0x0001) /* Only selector supported          */
#define ADVERTISE_10HALF        (0x0020) /* Try for 10mbps half-duplex       */
#define ADVERTISE_1000XFULL     (0x0020) /* Try for 1000BASE-X full-duplex   */
#define ADVERTISE_10FULL        (0x0040) /* Try for 10mbps full-duplex       */
#define ADVERTISE_1000XHALF     (0x0040) /* Try for 1000BASE-X half-duplex   */
#define ADVERTISE_100HALF       (0x0080) /* Try for 100mbps half-duplex      */
#define ADVERTISE_1000XPAUSE    (0x0080) /* Try for 1000BASE-X pause         */
#define ADVERTISE_100FULL       (0x0100) /* Try for 100mbps full-duplex      */
#define ADVERTISE_1000XPSE_ASYM (0x0100) /* Try for 1000BASE-X asym pause    */
#define ADVERTISE_100BASE4      (0x0200) /* Try for 100mbps 4k packets       */
#define ADVERTISE_PAUSE_CAP     (0x0400) /* Try for pause                    */
#define ADVERTISE_PAUSE_ASYM    (0x0800) /* Try for asymetric pause          */
#define ADVERTISE_RESV          (0x1000) /* Unused...                        */
#define ADVERTISE_RFAULT        (0x2000) /* Say we can detect faults         */
#define ADVERTISE_LPACK         (0x4000) /* Ack link partners response       */
#define ADVERTISE_NPAGE         (0x8000) /* Next page bit                    */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                        ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL |  \
                       ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Reg05h Link partner ability register. */
#define LPA_SLCT            (0x001f) /* Same as advertise selector       */
#define LPA_10HALF          (0x0020) /* Can do 10mbps half-duplex        */
#define LPA_1000XFULL       (0x0020) /* Can do 1000BASE-X full-duplex    */
#define LPA_10FULL          (0x0040) /* Can do 10mbps full-duplex        */
#define LPA_1000XHALF       (0x0040) /* Can do 1000BASE-X half-duplex    */
#define LPA_100HALF         (0x0080) /* Can do 100mbps half-duplex       */
#define LPA_1000XPAUSE      (0x0080) /* Can do 1000BASE-X pause          */
#define LPA_100FULL         (0x0100) /* Can do 100mbps full-duplex       */
#define LPA_1000XPAUSE_ASYM (0x0100) /* Can do 1000BASE-X pause asym     */
#define LPA_100BASE4        (0x0200) /* Can do 100mbps 4k packets        */
#define LPA_PAUSE_CAP       (0x0400) /* Can pause                        */
#define LPA_PAUSE_ASYM      (0x0800) /* Can pause asymetrically          */
#define LPA_RESV            (0x1000) /* Unused...                        */
#define LPA_RFAULT          (0x2000) /* Link partner faulted             */
#define LPA_LPACK           (0x4000) /* Link partner acked us            */
#define LPA_NPAGE           (0x8000) /* Next page bit                    */

#define LPA_DUPLEX (LPA_10FULL | LPA_100FULL)
#define LPA_100    (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* Reg09h 1000BASE-T Control register Register */
#define ADVERTISE_1000HALF    (0x0100) /* Advertise 1000BASE-T half duplex */
#define ADVERTISE_1000FULL    (0x0200) /* Advertise 1000BASE-T full duplex */
#define CTL1000_PORT_TYPE     (0x0400)
#define CTL1000_AS_MASTER     (0x0800)
#define CTL1000_ENABLE_MASTER (0x1000)
#define CTL1000_TEST_NORMAL   (0x0000)
#define CTL1000_TEST_TM1      (0x2000)
#define CTL1000_TEST_TM2      (0x4000)
#define CTL1000_TEST_TM3      (0x6000)
#define CTL1000_TEST_TM4      (0x8000)

/* Reg0Ah 1000BASE-T Status register */
#define MASTER_SLAVE_CFAULT (0x8000) /* Master/Slave Manual Configuration Fault */
#define LPA_1000LOCALRXOK   (0x2000) /* Link partner local receiver status  */
#define LPA_1000REMRXOK     (0x1000) /* Link partner remote receiver status */
#define LPA_1000FULL        (0x0800) /* Link partner 1000BASE-T full duplex */
#define LPA_1000HALF        (0x0400) /* Link partner 1000BASE-T half duplex */

/* Flow control flags */
#define FLOW_CTRL_TX (0x01)
#define FLOW_CTRL_RX (0x02)

/* CL45 Reg Support DEVID */
#define MMD_DEV_ANEG   (0x07)
#define MMD_DEV_VSPEC1 (0x1E)
#define MMD_DEV_VSPEC2 (0x1F)

/* EEE advertisement register */
#define MMD_EEEAR       (0x3C)
#define MMD_EEELPAR     (0x3D)
#define MMD_EEEAR_2G5   (0x3E)
#define MMD_EEELPAR_2G5 (0x3F)
#define EEE_2G5BASE_T   (0x1)
#define EEE_1000BASE_T  (0x4)
#define EEE_100BASE_TX  (0x2)

/* MultiGBASE-T AN register */
#define MULTIG_ANAR       (0x20)
#define MULTIG_LPAR       (0x21)
#define MULTIG_ANAR_2500M (0x0080)
#define MULTIG_LPAR_2500M (0x0020)

/* Auxiliary control and status register */
#define AUX_FDX_STATUS (0x20)

/* Cable Length */
#define HAL_PHY_EC_ALL_TAPS    (81) /* EC Training raw data which used to calclulate cable length */
#define HAL_PHY_CABLE_MAX_PAIR (4)

/* PHY LED blink events */
#define HAL_PHY_LED_BLK_EVT_FLAGS_FORCE    (1UL << 0)
#define HAL_PHY_LED_BLK_EVT_FLAGS_1000M_TX (1UL << 1)
#define HAL_PHY_LED_BLK_EVT_FLAGS_1000M_RX (1UL << 2)
#define HAL_PHY_LED_BLK_EVT_FLAGS_100M_TX  (1UL << 3)
#define HAL_PHY_LED_BLK_EVT_FLAGS_100M_RX  (1UL << 4)
#define HAL_PHY_LED_BLK_EVT_FLAGS_10M_TX   (1UL << 5)
#define HAL_PHY_LED_BLK_EVT_FLAGS_10M_RX   (1UL << 6)

/* PHY LED blink control mode */
typedef enum
{
    HAL_PHY_LED_BLK_CTRL_MODE_ON,
    HAL_PHY_LED_BLK_CTRL_MODE_BLK,
    HAL_PHY_LED_BLK_CTRL_MODE_LAST
} HAL_PHY_LED_BLK_CTRL_MODE_T;

/* LED Hardware parameter */
#define HAL_PHY_LED_CLOCK_TICK_US (40)
#define HAL_PHY_LED_MAX_DURATION  (0x10000)
#define HAL_PHY_LED_FAST_DURATION (0x2000)
#define HAL_PHY_LED_MAX_CNT       (2)

#define HAL_PHY_MAX_DATA_MSB (15)
#define HAL_PHY_MIN_DATA_LSB (0)

/* SERDES operation mode */
#define HAL_PHY_SERDES_OP_MODE_SPEED_CAHNGE (0x0)
#define HAL_PHY_SERDES_OP_MODE_SPEED_FIX    (0x1)

/* surge protection mode */
#define HAL_PHY_SURGE_PROTECTION_MODE_0R (0x0)
#define HAL_PHY_SURGE_PROTECTION_MODE_5R (0x1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    HAL_PHY_ADMIN_STATE_DISABLE = 0x0,
    HAL_PHY_ADMIN_STATE_ENABLE,
    HAL_PHY_ADMIN_STATE_LAST
} HAL_PHY_ADMIN_STATE_T;

typedef enum
{
    HAL_PHY_AUTO_NEGO_DISABLE = 0x0,
    HAL_PHY_AUTO_NEGO_ENABLE,
    HAL_PHY_AUTO_NEGO_RESTART,
    HAL_PHY_AUTO_NEGO_LAST
} HAL_PHY_AUTO_NEGO_T;

typedef enum
{
    HAL_PHY_SPEED_10M = 0x0,
    HAL_PHY_SPEED_100M,
    HAL_PHY_SPEED_1000M,
    HAL_PHY_SPEED_2500M,
    HAL_PHY_SPEED_5000M,
    HAL_PHY_SPEED_10000M,
    HAL_PHY_SPEED_LAST
} HAL_PHY_SPEED_T;

typedef enum
{
    HAL_PHY_DUPLEX_HALF = 0x0,
    HAL_PHY_DUPLEX_FULL,
    HAL_PHY_DUPLEX_LAST
} HAL_PHY_DUPLEX_T;

typedef struct HAL_PHY_AN_ADV_S
{
#define HAL_PHY_AN_ADV_FLAGS_10HFDX     (1U << 0)
#define HAL_PHY_AN_ADV_FLAGS_10FUDX     (1U << 1)
#define HAL_PHY_AN_ADV_FLAGS_100HFDX    (1U << 2)
#define HAL_PHY_AN_ADV_FLAGS_100FUDX    (1U << 3)
#define HAL_PHY_AN_ADV_FLAGS_1000FUDX   (1U << 4)
#define HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE  (1U << 5)
#define HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE (1U << 6)
#define HAL_PHY_AN_ADV_FLAGS_EEE        (1U << 7)
#define HAL_PHY_AN_ADV_FLAGS_2500M      (1U << 8)
/* Used for internal phy(serdes) driver to indicate if current operation mode support these
 auto-negotation abilities or not */
#define HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID (1U << 30)
#define HAL_PHY_AN_ADV_FLAGS_EEE_INVALID   (1U << 31)
    UI32_T flags;
} HAL_PHY_AN_ADV_T;

typedef struct HAL_PHY_LINK_STATUS_S
{
#define HAL_PHY_LINK_STATUS_FLAGS_LINK_UP        (1U << 0)
#define HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE (1U << 1)
#define HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT   (1U << 2)
    UI32_T           flags;
    HAL_PHY_SPEED_T  speed;
    HAL_PHY_DUPLEX_T duplex;
} HAL_PHY_LINK_STATUS_T;

typedef enum
{
    HAL_PHY_LPBK_NEAR_END = 0,
    HAL_PHY_LPBK_FAR_END,
    HAL_PHY_LPBK_LAST
} HAL_PHY_LPBK_T;

typedef enum
{
    HAL_PHY_SSD_MODE_DISABLE = 0,
    HAL_PHY_SSD_MODE_1T,
    HAL_PHY_SSD_MODE_2T,
    HAL_PHY_SSD_MODE_3T,
    HAL_PHY_SSD_MODE_4T,
    HAL_PHY_SSD_MODE_5T,
    HAL_PHY_SSD_MODE_6T,
    HAL_PHY_SSD_MODE_7T,
    HAL_PHY_SSD_MODE_8T,
    HAL_PHY_SSD_MODE_9T,
    HAL_PHY_SSD_MODE_10T,
    HAL_PHY_SSD_MODE_11T,
    HAL_PHY_SSD_MODE_12T,
    HAL_PHY_SSD_MODE_13T,
    HAL_PHY_SSD_MODE_14T,
    HAL_PHY_SSD_MODE_15T,
    HAL_PHY_SSD_MODE_LAST
} HAL_PHY_SSD_MODE_T;

typedef enum
{
    HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A,
    HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B,
    HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A,
    HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B,
    HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A,
    HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B,
    HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A,
    HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE,
    HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B,
    HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C,
    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5,
    HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6,
    HAL_PHY_TX_COMPLIANCE_MODE_LAST
} HAL_PHY_TX_COMPLIANCE_MODE_T;

typedef enum
{
    HAL_PHY_COMBO_MODE_PHY,
    HAL_PHY_COMBO_MODE_SERDES,
    HAL_PHY_COMBO_MODE_LAST
} HAL_PHY_COMBO_MODE_T;

typedef enum
{
    HAL_PHY_SERDES_MODE_SGMII,
    HAL_PHY_SERDES_MODE_1000BASE_X,
    HAL_PHY_SERDES_MODE_HSGMII,
    HAL_PHY_SERDES_MODE_100BASE_FX,
    HAL_PHY_SERDES_MODE_5GBASE_R,
    HAL_PHY_SERDES_MODE_USXGMII,
    HAL_PHY_SERDES_MODE_LAST
} HAL_PHY_SERDES_MODE_T;

/* Definition of port LED control */
typedef enum
{
    HAL_PHY_LED_CTRL_MODE_PHY = 0,
    HAL_PHY_LED_CTRL_MODE_FORCE,
    HAL_PHY_LED_CTRL_MODE_LAST
} HAL_PHY_LED_CTRL_MODE_T;

typedef enum
{
    HAL_PHY_LED_STATE_FORCE_PATT = 0,
    HAL_PHY_LED_STATE_OFF,
    HAL_PHY_LED_STATE_ON,
    HAL_PHY_LED_STATE_LAST
} HAL_PHY_LED_STATE_T;

typedef enum
{
    HAL_PHY_LED_PATT_HZ_HALF = 0,
    HAL_PHY_LED_PATT_HZ_ONE,
    HAL_PHY_LED_PATT_HZ_TWO,
    HAL_PHY_LED_PATT_HZ_ZERO,
    HAL_PHY_LED_PATT_LAST
} HAL_PHY_LED_PATT_T;

typedef enum
{
    HAL_PHY_CABLE_STATUS_OPEN,
    HAL_PHY_CABLE_STATUS_SHORT,
    HAL_PHY_CABLE_STATUS_NORMAL,
    HAL_PHY_CABLE_STATUS_LAST
} HAL_PHY_CABLE_STATUS_T;

typedef enum
{
    HAL_PHY_CABLE_TEST_PAIR_A,
    HAL_PHY_CABLE_TEST_PAIR_B,
    HAL_PHY_CABLE_TEST_PAIR_C,
    HAL_PHY_CABLE_TEST_PAIR_D,
    HAL_PHY_CABLE_TEST_PAIR_ALL,
    HAL_PHY_CABLE_TEST_PAIR_LAST
} HAL_PHY_CABLE_TEST_PAIR_T;

typedef struct HAL_PHY_CABLE_TEST_RSLT_S
{
    HAL_PHY_CABLE_STATUS_T status[HAL_PHY_CABLE_MAX_PAIR];

    /* Cable length = length * 0.1 m */
    UI32_T                 length[HAL_PHY_CABLE_MAX_PAIR];
} HAL_PHY_CABLE_TEST_RSLT_T;

typedef struct
{
#define HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD (1UL << 0)
#define HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN    (1UL << 1)
#define HAL_PHY_LED_GLB_CFG_FLAGS_FAST_CLOCK  (1UL << 2)
    UI32_T flags;
} HAL_PHY_LED_GLB_CFG_T;

typedef enum
{
    HAL_PHY_OP_MODE_NORMAL,
    HAL_PHY_OP_MODE_LONG_REACH,
    HAL_PHY_OP_MODE_LAST
} HAL_PHY_OP_MODE_T;

typedef enum
{
    HAL_PHY_ACCESS_METHOD_CL22,
    HAL_PHY_ACCESS_METHOD_CL45,
    HAL_PHY_ACCESS_METHOD_CL45_INC,
    HAL_PHY_ACCESS_METHOD_LAST
} HAL_PHY_ACCESS_METHOD_T;

typedef struct HAL_PHY_CFG_S
{
    HAL_PHY_ACCESS_METHOD_T access_method;
    UI16_T                  device_id;
    UI16_T                  reg_addr;
    UI16_T                  delay_time;
    UI16_T                  data_msb;
    UI16_T                  data_lsb;
    UI16_T                  reg_data;
} HAL_PHY_CFG_T;

typedef enum
{
    HAL_PHY_PHY_TYPE_INTERNAL = 0x0,
    HAL_PHY_PHY_TYPE_EXTERNAL,
    HAL_PHY_PHY_TYPE_LAST
} HAL_PHY_PHY_TYPE_T;

#define HAL_PHY_SERDES_INTERFACE_MII     (1UL << 0)
#define HAL_PHY_SERDES_INTERFACE_XGMII   (1UL << 1)
#define HAL_PHY_SERDES_INTERFACE_USXGMII (1UL << 2)
#define HAL_PHY_SERDES_READ_CLEAR        (1UL << 3)

/* Macros for phy related information */
#define PTR_HAL_PHY_CB_CTX(__unit__)                _ext_ptr_phy_cb[__unit__]
#define PTR_HAL_PHY_PORT_CB_CTX(__unit__, __port__) _ext_ptr_phy_cb[__unit__][__port__]
#define PTR_HAL_PHY_PORT_DEV(__unit__, __port__)    _ext_ptr_phy_cb[__unit__][__port__].phy_dev
#define PTR_HAL_PHY_PORT_INT_DRIVER(__unit__, __port__)                   \
    _ext_ptr_phy_cb[__unit__][__port__].driver[HAL_PHY_PHY_TYPE_INTERNAL]
#define PTR_HAL_PHY_PORT_EXT_DRIVER(__unit__, __port__)                   \
    _ext_ptr_phy_cb[__unit__][__port__].driver[HAL_PHY_PHY_TYPE_EXTERNAL]

#define HAL_PHY_PORT_DEV_ACCESS_TYPE(__unit__, __port__) PTR_HAL_PHY_PORT_DEV(__unit__, __port__).access_type
#define HAL_PHY_PORT_DEV_PHY_ADDR(__unit__, __port__)    PTR_HAL_PHY_PORT_DEV(__unit__, __port__).phy_addr
#define HAL_PHY_PORT_DEV_I2C_ADDR(__unit__, __port__)    PTR_HAL_PHY_PORT_DEV(__unit__, __port__).i2c_addr
#define HAL_PHY_PORT_DEV_REVISION_ID(__unit__, __port__) PTR_HAL_PHY_PORT_DEV(__unit__, __port__).revision_id
#define HAL_PHY_PORT_DEV_PHY_ID(__unit__, __port__)      PTR_HAL_PHY_PORT_DEV(__unit__, __port__).phy_id
#define HAL_PHY_PORT_DEV_I2C_BUS_ID(__unit__, __port__)  PTR_HAL_PHY_PORT_DEV(__unit__, __port__).i2c_bus_id
#define HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(__unit__, __port__) \
    PTR_HAL_PHY_PORT_DEV(__unit__, __port__).timeout_counter
#define HAL_PHY_PORT_DEV_SPD_DOWN_ADV_CHANGED(__unit__, __port__) \
    PTR_HAL_PHY_PORT_DEV(__unit__, __port__).adv_1g_changed
#define HAL_PHY_PORT_DEV_SPD_DOWN_ADV_BACKUP(__unit__, __port__) \
    PTR_HAL_PHY_PORT_DEV(__unit__, __port__).adv_1g_backup

#define HAL_PHY_INT_FUNC_CALL(__unit__, __port__, __func__, __param__) ({             \
    AIR_ERROR_NO_T __rc = AIR_E_OK;                                                   \
    if ((NULL == PTR_HAL_PHY_PORT_INT_DRIVER(__unit__, __port__)) ||                  \
        (NULL == PTR_HAL_PHY_PORT_INT_DRIVER(__unit__, __port__)->__func__))          \
    {                                                                                 \
        __rc = AIR_E_NOT_SUPPORT;                                                     \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        __rc = (PTR_HAL_PHY_PORT_INT_DRIVER(__unit__, __port__)->__func__ __param__); \
    }                                                                                 \
    __rc;                                                                             \
})

#define HAL_PHY_EXT_FUNC_CALL(__unit__, __port__, __func__, __param__) ({             \
    AIR_ERROR_NO_T __rc = AIR_E_OK;                                                   \
    if ((NULL == PTR_HAL_PHY_PORT_EXT_DRIVER(__unit__, __port__)) ||                  \
        (NULL == PTR_HAL_PHY_PORT_EXT_DRIVER(__unit__, __port__)->__func__))          \
    {                                                                                 \
        __rc = AIR_E_NOT_SUPPORT;                                                     \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        __rc = (PTR_HAL_PHY_PORT_EXT_DRIVER(__unit__, __port__)->__func__ __param__); \
    }                                                                                 \
    __rc;                                                                             \
})

typedef AIR_ERROR_NO_T (*HAL_PHY_INIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETADMINSTATE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETADMINSTATE_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETAUTONEGO_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETAUTONEGO_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETLOCALADVABILITY_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETLOCALADVABILITY_FUNC_T)(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETREMOTEADVABILITY_FUNC_T)(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETSPEED_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETSPEED_FUNC_T)(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETDUPLEX_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETDUPLEX_FUNC_T)(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETLINKSTATUS_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETLOOPBACK_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETLOOPBACK_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETSMARTSPEEDDOWN_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETSMARTSPEEDDOWN_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETLEDONCTRL_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETLEDONCTRL_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_PHY_TESTTXCOMPLIANCE_FUNC_T)(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETCOMBOMODE_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    const HAL_PHY_COMBO_MODE_T combo_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETCOMBOMODE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    HAL_PHY_COMBO_MODE_T *ptr_combo_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETSERDESMODE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETSERDESMODE_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETLEDCTRLMODE_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETLEDCTRLMODE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYLEDFORCESTATE_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYLEDFORCESTATE_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYLEDFORCEPATT_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYLEDFORCEPATT_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

typedef AIR_ERROR_NO_T (*HAL_PHY_TRIGGERCABLETEST_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETCABLETESTRAWDATA_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYLEDGLBCFG_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYLEDGLBCFG_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYLEDBLKEVENT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYLEDBLKEVENT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYLEDDURATION_FUNC_T)(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYLEDDURATION_FUNC_T)(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

typedef AIR_ERROR_NO_T (*HAL_PHY_SETPHYOPMODE_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_GETPHYOPMODE_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PHY_DUMPPHYPARA_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_PHY_TRIGGERLINKDOWNCABLETEST_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

typedef AIR_ERROR_NO_T (*HAL_PHY_DUMPPORTCNT_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);

typedef AIR_ERROR_NO_T (*HAL_PHY_DUMPDEBUGINFO_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

typedef struct
{
    HAL_PHY_INIT_FUNC_T                     hal_phy_init;
    HAL_PHY_SETADMINSTATE_FUNC_T            hal_phy_setAdminState;
    HAL_PHY_GETADMINSTATE_FUNC_T            hal_phy_getAdminState;
    HAL_PHY_SETAUTONEGO_FUNC_T              hal_phy_setAutoNego;
    HAL_PHY_GETAUTONEGO_FUNC_T              hal_phy_getAutoNego;
    HAL_PHY_SETLOCALADVABILITY_FUNC_T       hal_phy_setLocalAdvAbility;
    HAL_PHY_GETLOCALADVABILITY_FUNC_T       hal_phy_getLocalAdvAbility;
    HAL_PHY_GETREMOTEADVABILITY_FUNC_T      hal_phy_getRemoteAdvAbility;
    HAL_PHY_SETSPEED_FUNC_T                 hal_phy_setSpeed;
    HAL_PHY_GETSPEED_FUNC_T                 hal_phy_getSpeed;
    HAL_PHY_SETDUPLEX_FUNC_T                hal_phy_setDuplex;
    HAL_PHY_GETDUPLEX_FUNC_T                hal_phy_getDuplex;
    HAL_PHY_GETLINKSTATUS_FUNC_T            hal_phy_getLinkStatus;
    HAL_PHY_SETLOOPBACK_FUNC_T              hal_phy_setLoopBack;
    HAL_PHY_GETLOOPBACK_FUNC_T              hal_phy_getLoopBack;
    HAL_PHY_SETSMARTSPEEDDOWN_FUNC_T        hal_phy_setSmartSpeedDown;
    HAL_PHY_GETSMARTSPEEDDOWN_FUNC_T        hal_phy_getSmartSpeedDown;
    HAL_PHY_SETLEDONCTRL_FUNC_T             hal_phy_setLedOnCtrl;
    HAL_PHY_GETLEDONCTRL_FUNC_T             hal_phy_getLedOnCtrl;
    HAL_PHY_TESTTXCOMPLIANCE_FUNC_T         hal_phy_testTxCompliance;
    HAL_PHY_SETCOMBOMODE_FUNC_T             hal_phy_setComboMode;
    HAL_PHY_GETCOMBOMODE_FUNC_T             hal_phy_getComboMode;
    HAL_PHY_SETSERDESMODE_FUNC_T            hal_phy_setSerdesMode;
    HAL_PHY_GETSERDESMODE_FUNC_T            hal_phy_getSerdesMode;
    HAL_PHY_SETLEDCTRLMODE_FUNC_T           hal_phy_setLedCtrlMode;
    HAL_PHY_GETLEDCTRLMODE_FUNC_T           hal_phy_getLedCtrlMode;
    HAL_PHY_SETPHYLEDFORCESTATE_FUNC_T      hal_phy_setPhyLedForceState;
    HAL_PHY_GETPHYLEDFORCESTATE_FUNC_T      hal_phy_getPhyLedForceState;
    HAL_PHY_SETPHYLEDFORCEPATT_FUNC_T       hal_phy_setPhyLedForcePattCfg;
    HAL_PHY_GETPHYLEDFORCEPATT_FUNC_T       hal_phy_getPhyLedForcePattCfg;
    HAL_PHY_TRIGGERCABLETEST_FUNC_T         hal_phy_triggerCableTest;
    HAL_PHY_GETCABLETESTRAWDATA_FUNC_T      hal_phy_getCableTestRawData;
    HAL_PHY_SETPHYLEDGLBCFG_FUNC_T          hal_phy_setPhyLedGlbCfg;
    HAL_PHY_GETPHYLEDGLBCFG_FUNC_T          hal_phy_getPhyLedGlbCfg;
    HAL_PHY_SETPHYLEDBLKEVENT_FUNC_T        hal_phy_setPhyLedBlkEvent;
    HAL_PHY_GETPHYLEDBLKEVENT_FUNC_T        hal_phy_getPhyLedBlkEvent;
    HAL_PHY_SETPHYLEDDURATION_FUNC_T        hal_phy_setPhyLedDuration;
    HAL_PHY_GETPHYLEDDURATION_FUNC_T        hal_phy_getPhyLedDuration;
    HAL_PHY_SETPHYOPMODE_FUNC_T             hal_phy_setPhyOpMode;
    HAL_PHY_GETPHYOPMODE_FUNC_T             hal_phy_getPhyOpMode;
    HAL_PHY_DUMPPHYPARA_FUNC_T              hal_phy_dumpPhyPara;
    HAL_PHY_TRIGGERLINKDOWNCABLETEST_FUNC_T hal_phy_triggerLinkDownCableTest;
    HAL_PHY_DUMPPORTCNT_FUNC_T              hal_phy_dumpPortCnt;
    HAL_PHY_DUMPDEBUGINFO_FUNC_T            hal_phy_dumpDebugInfo;
} HAL_PHY_DRIVER_T;

typedef AIR_ERROR_NO_T (*HAL_PHY_DRIVER_FUNC_T)(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

typedef struct
{
    UI32_T                phy_id;          /* PHY driver ID */
    HAL_PHY_DRIVER_FUNC_T phy_driver_func; /* PHY driver handler function pointer */
} HAL_PHY_DRIVER_MAP_T;

typedef struct HAL_PHY_DEV_S
{
    /* config */
    UI8_T  access_type;
    UI8_T  i2c_addr;
    UI8_T  phy_addr;
    UI8_T  i2c_bus_id;
    /* status */
    UI8_T  revision_id;
    UI32_T phy_id;
    /* speed down shift work around */
    UI8_T  timeout_counter;
    UI8_T  adv_1g_changed;
    UI16_T adv_1g_backup;
} HAL_PHY_DEV_T;

typedef struct HAL_PHY_CB_S
{
    HAL_PHY_DEV_T     phy_dev; /* come from customer init cfg mechanism */
    HAL_PHY_DRIVER_T *driver[HAL_PHY_PHY_TYPE_LAST];
} HAL_PHY_CB_T;

/* GLOBAL VARIABLE EXTERN DECLARATIONS
 */
extern HAL_PHY_CB_T *_ext_ptr_phy_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   hal_phy_init
 * PURPOSE:
 *      This API is used to init phy control block and init/probe each port.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_init(
    const UI32_T unit);

/* FUNCTION NAME:   hal_phy_deinit
 * PURPOSE:
 *      This API is used to deinit phy control block and deinit each port.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_deinit(
    const UI32_T unit);

/* FUNCTION NAME:   hal_phy_setAdminState
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
hal_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state);

/* FUNCTION NAME:   hal_phy_getAdminState
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
hal_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_phy_setAutoNego
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
hal_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego);

/* FUNCTION NAME:   hal_phy_getAutoNego
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
hal_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_phy_setLocalAdvAbility
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
hal_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_phy_getLocalAdvAbility
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
hal_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_phy_getRemoteAdvAbility
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
hal_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   hal_phy_setSpeed
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
hal_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed);

/* FUNCTION NAME:   hal_phy_getSpeed
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
hal_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed);

/* FUNCTION NAME:   hal_phy_setDuplex
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
hal_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex);

/* FUNCTION NAME:   hal_phy_getDuplex
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
hal_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   hal_phy_getLinkStatus
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
hal_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_phy_setLoopBack
 * PURPOSE:
 *      This API is used to set the loop back configuration for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  HAL_PHY_LPBK_NEAR_END
 *                          HAL_PHY_LPBK_FAR_END
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable);

/* FUNCTION NAME:   hal_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get the loop back configuration for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  HAL_PHY_LPBK_NEAR_END
 *                          HAL_PHY_LPBK_FAR_END
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable);

/* FUNCTION NAME:   hal_phy_setSmartSpeedDown
 * PURPOSE:
 *      This API is used to set port smart speed down feature for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ssd_mode        --  HAL_PHY_SSD_MODE_DISABLE
 *                          HAL_PHY_SSD_MODE_2T
 *                          HAL_PHY_SSD_MODE_3T
 *                          HAL_PHY_SSD_MODE_4T
 *                          HAL_PHY_SSD_MODE_5T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode);

/* FUNCTION NAME:   hal_phy_getSmartSpeedDown
 * PURPOSE:
 *      This API is used to get port smart speed down setting for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 * OUTPUT:
 *      ptr_ssd_mode    --  HAL_PHY_SSD_MODE_DISABLE
 *                          HAL_PHY_SSD_MODE_2T
 *                          HAL_PHY_SSD_MODE_3T
 *                          HAL_PHY_SSD_MODE_4T
 *                          HAL_PHY_SSD_MODE_5T
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

/* FUNCTION NAME:   hal_phy_setLedOnCtrl
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
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_phy_getLedOnCtrl
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
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_phy_testTxCompliance
 * PURPOSE:
 *      This API is used to set the Tx compliance mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      mode            --  bist mode
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

/* FUNCTION NAME: hal_phy_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      combo_mode      --  HAL_PHY_COMBO_MODE_PHY
 *                          HAL_PHY_COMBO_MODE_SERDES
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_phy_setComboMode(
    const UI32_T               unit,
    const UI32_T               port,
    const HAL_PHY_COMBO_MODE_T combo_mode);

/* FUNCTION NAME: hal_phy_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port.
 *
 * INPUT:
 *      unit             --  Device ID
 *      port             --  Index of port number
 *
 * OUTPUT:
 *      ptr_combo_mode   --  HAL_PHY_COMBO_MODE_PHY
 *                           HAL_PHY_COMBO_MODE_SERDES
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_phy_getComboMode(
    const UI32_T          unit,
    const UI32_T          port,
    HAL_PHY_COMBO_MODE_T *ptr_combo_mode);

/* FUNCTION NAME:   hal_phy_setSerdesMode
 * PURPOSE:
 *      This API is used to set serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      serdes_mode     --  Serdes mode
 *
 * OUTPUT:
 *      None
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
hal_phy_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   hal_phy_getSerdesMode
 * PURPOSE:
 *      This API is used to get serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *
 * OUTPUT:
 *      ptr_serdes_mode --  Serdes mode
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
hal_phy_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME: hal_phy_setLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      led_ctrl        --  LED control mode enumeration type
 *                          HAL_PHY_LED_CTRL_MODE_T
 * OUTPUT:
 *      None
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
hal_phy_setLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T led_ctrl);

/* FUNCTION NAME: hal_phy_getLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      led_ctrl        --  LED control enumeration type
 *                          HAL_PHY_LED_CTRL_MODE_T
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
hal_phy_getLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_led_ctrl);

/* FUNCTION NAME:   hal_phy_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      state           --  LED force state
 *                          HAL_PHY_LED_STATE_T
 * OUTPUT:
 *      None
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
hal_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_phy_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      ptr_state       --  LED force state enumeration type
 *                          AIR_PORT_PHY_LED_STATE_T
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
hal_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      pattern         --  LED force pattern
 *                          HAL_PHY_LED_PATT_T
 * OUTPUT:
 *      None
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
hal_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern enumeration type
 *                          HAL_PHY_LED_PATT_T
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
hal_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_phy_triggerCableTest
 * PURPOSE:
 *      Get cable status.
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
hal_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_phy_getCableTestRawData
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
 *
 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME: hal_phy_setPhyLedGlbCfg
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
hal_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_phy_getPhyLedGlbCfg
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
hal_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_phy_setPhyLedBlkEvent
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
hal_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

/* FUNCTION NAME: hal_phy_getPhyLedBlkEvent
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
hal_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

/* FUNCTION NAME: hal_phy_setPhyLedDuration
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
hal_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

/* FUNCTION NAME: hal_phy_getPhyLedDuration
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
hal_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

/* FUNCTION NAME: hal_phy_syncLedClock
 * PURPOSE:
 *      Synchronize LED clock
 * INPUT:
 *      unit                     -- Device ID
 *      port_bitmap              -- Configured port bitmap
 *                                  AIR_PORT_BITMAP_T
 *      delay                    -- Delay time(Unit:us)
 *                                  0: Default delay time
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_syncLedClock(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            delay);

/* FUNCTION NAME: hal_phy_syncWaveGenClock
 * PURPOSE:
 *      Synchronize wave generator clock
 * INPUT:
 *      unit                     -- Device ID
 *      port_bitmap              -- Configured port bitmap
 *                                  AIR_PORT_BITMAP_T
 *      delay                    -- Delay time(Unit:us)
 *                                  0: Default delay time
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_syncWaveGenClock(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            delay);

/* FUNCTION NAME: hal_phy_setPhyOpMode
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
hal_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode);

/* FUNCTION NAME: hal_phy_getPhyOpMode
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
hal_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_phy_dumpPhyPara
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
hal_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: hal_phy_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          AIR_PORT_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic link down mode only.
 */
AIR_ERROR_NO_T
hal_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_phy_dumpPortCnt
 * PURPOSE:
 *      Dump port debug counter.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Phy type
 *      param                    -- Parameter
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
hal_phy_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param);

/* FUNCTION NAME: hal_phy_dumpDebugInfo
 * PURPOSE:
 *      Dump port deubg information.
 *
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
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port);

#endif /* #ifndef HAL_PHY_H */
