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

/* FILE NAME:  hal_cmn_phy.h
 * PURPOSE:
 *      It provides common PHY module API.
 * NOTES:
 */

#ifndef HAL_CMN_PHY_H
#define HAL_CMN_PHY_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_EC_ALL_TAPS    (80)
#define HAL_CABLE_PAIR_A   (1)
#define HAL_CABLE_PAIR_B   (2)
#define HAL_CABLE_PAIR_C   (4)
#define HAL_CABLE_PAIR_D   (8)
#define HAL_CABLE_PAIR_ALL (15)

/* Common PHY LED register */
#define HAL_CMN_PHY_LED_BASIC_CTRL         (0x0021)
#define HAL_CMN_PHY_LED_ENHANCE_MOD_OFFSET (15)
#define HAL_CMN_PHY_LED_ENHANCE_MOD_LENGTH (1)
#define HAL_CMN_PHY_LED_CLK_EN_OFFSET      (3)
#define HAL_CMN_PHY_LED_CLK_EN_LENGTH      (1)
#define HAL_CMN_PHY_LED_FAST_CLK_OFFSET    (2)
#define HAL_CMN_PHY_LED_FAST_CLK_LENGTH    (1)
#define HAL_CMN_PHY_LED_ON_DRA             (0x0022)
#define HAL_CMN_PHY_LED_BLK_DRA            (0x0023)
#define HAL_CMN_PHY_LED_0_ON_MASK          (0x0024)
#define HAL_CMN_PHY_LED_0_BLK_MASK         (0x0025)
#define HAL_CMN_PHY_LED_1_ON_MASK          (0x0026)
#define HAL_CMN_PHY_LED_1_BLK_MASK         (0x0027)
#define HAL_CMN_PHY_LED_2_ON_MASK          (0x0028)
#define HAL_CMN_PHY_LED_2_BLK_MASK         (0x0029)

/* CL22 Reg Support Page Select */
#define HAL_CMN_PHY_MII_PAGE_SELECT   (0x1f)
#define HAL_CMN_PHY_CL22_PAGE_REG     (0x0000)
#define HAL_CMN_PHY_CL22_PAGE_EXTREG  (0x0001)
#define HAL_CMN_PHY_CL22_PAGE_MISCREG (0x0002)
#define HAL_CMN_PHY_CL22_PAGE_LPIREG  (0x0003)

/* 1000BASE-T Control Register */
#define HAL_CMN_PHY_CR1G          (0x09)
#define HAL_CMN_CR1G_MS_EN_OFFT   (12)
#define HAL_CMN_CR1G_MS_EN_LENG   (1)
#define HAL_CMN_CR1G_MS_CONF_OFFT (11)
#define HAL_CMN_CR1G_MS_CONF_LENG (1)

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    /* 04:00 */
    UI32_T TrRG_LSB    : 5;
    /* 07:05 */
    UI32_T Reserved_21 : 3;
    /* 12:08 */
    UI32_T TrRG_MSB    : 5;
    /* 15:13 */
    UI32_T Reserved_29 : 3;
    /* 16:16 */
    UI32_T Reserved_0  : 1;
    /* 22:17 */
    UI32_T DATA_ADDR   : 6;
    /* 26:23 */
    UI32_T NODE_ADDR   : 4;
    /* 28:27 */
    UI32_T CH_ADDR     : 2;
    /* 29:29 */
    UI32_T WR_RD_CTRL  : 1;
    /* 30:30 */
    UI32_T Reserved_14 : 1;
    /* 31:31 */
    UI32_T PKT_XMT_STA : 1;
} TrRG_PARAM_T;

typedef union
{
    TrRG_PARAM_T param;
    UI16_T       Raw[2];
    UI32_T       DescVal;
} TrRGDesc_T;

typedef struct
{
    TrRGDesc_T TrRGDesc;
    UI32_T     RgMask;
} TrRG_T;

typedef struct
{
    UI16_T data_lo;
    UI8_T  data_hi;
} HAL_TR_DATA_T;

/* FUNCTION NAME: hal_cmn_phy_triggerCableTest
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
hal_cmn_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_cmn_phy_getCableTestRawData
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
hal_cmn_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME: hal_cmn_phy_setPhyLedGlbCfg
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
hal_cmn_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_cmn_phy_getPhyLedGlbCfg
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
hal_cmn_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

/* FUNCTION NAME: hal_cmn_phy_setPhyLedBlkEvent
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
hal_cmn_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags);

/* FUNCTION NAME: hal_cmn_phy_getPhyLedBlkEvent
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
hal_cmn_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags);

/* FUNCTION NAME: hal_cmn_phy_setPhyLedDuration
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
hal_cmn_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time);

/* FUNCTION NAME: hal_cmn_phy_getPhyLedDuration
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
hal_cmn_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time);

/* FUNCTION NAME:   hal_cmn_phy_testTxCompliance
 * PURPOSE:
 *      This API is used to set the Tx compliance mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- BIST mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

/* FUNCTION NAME: hal_cmn_phy_triggerLinkDownCableTest
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
hal_cmn_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

#endif /* #ifndef HAL_CMN_PHY_H */
