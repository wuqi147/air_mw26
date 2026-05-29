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

/* FILE NAME:  hal_sco_serdes_param.c
 * PURPOSE:
 *  parameters for sco serdes
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */

#include <hal/common/hal.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/sco/hal_sco_serdes.h>
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

const HAL_SCO_SERDES_DBG_CFG_T _hal_sco_serdes_dbg_xgmii_cfg[] = {
    {1, 0x602C, 0,  1, 1, 0x1,           "RG_XGMII_PKT_RD_TOGGLE"},
    {1, 0x602C, 5,  1, 1, 0x0,           "RG_XGMII_PKT_RD_TOGGLE"},
    {0, 0x60B0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_0_STS(TX_FB)"},
    {0, 0x60B4, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_1_STS(TX_FD)"},
    {0, 0x60B8, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_2_STS(TX_ERR)"},
    {0, 0x60BC, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_3_STS(RX_FB)"},
    {0, 0x60C0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_4_STS(RX_FD)"},
    {0, 0x60C4, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_5_STS(RX_ERR)"},
};

const HAL_SCO_SERDES_DBG_CFG_T _hal_sco_serdes_dbg_mii_cfg[] = {
    {1, 0x6124, 0, 31, 0, 0xAA,           "rate_adp_dbg_p0_0"},
    {1, 0x6124, 5, 31, 0, 0x00,           "rate_adp_dbg_p0_0"},
    {0, 0x6128, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x612C, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x6148, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x6130, 0, 15, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x6134, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x6138, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x614C, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x6150, 0, 15, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x6154, 0, 31, 0, 0xAA,           "rate_adp_dbg_p1_0"},
    {1, 0x6154, 5, 31, 0, 0x00,           "rate_adp_dbg_p1_0"},
    {0, 0x6158, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x615C, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x6178, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x6160, 0, 15, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x6164, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x6168, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x617C, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x6180, 0, 15, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x6184, 0, 31, 0, 0xAA,           "rate_adp_dbg_p2_0"},
    {1, 0x6184, 5, 31, 0, 0x00,           "rate_adp_dbg_p2_0"},
    {0, 0x6188, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x618C, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x6208, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x6190, 0, 15, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x6194, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x6198, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x620C, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x6210, 0, 15, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x6214, 0, 31, 0, 0xAA,           "rate_adp_dbg_p3_0"},
    {1, 0x6214, 5, 31, 0, 0x00,           "rate_adp_dbg_p3_0"},
    {0, 0x6218, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x621C, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x6238, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x6220, 0, 15, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x6224, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x6228, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x623C, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x6240, 0, 15, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_ER_CNT"},
};

const HAL_SCO_SERDES_DBG_CFG_T _hal_sco_serdes_dbg_clear_cfg[] = {
    {1, 0x6124, 0, 31, 0, 0x55,     "rate_adp_dbg_p0_0"},
    {1, 0x6124, 5, 31, 0, 0x00,     "rate_adp_dbg_p0_0"},
    {1, 0x6154, 0, 31, 0, 0x55,     "rate_adp_dbg_p1_0"},
    {1, 0x6154, 5, 31, 0, 0x00,     "rate_adp_dbg_p1_0"},
    {1, 0x6184, 0, 31, 0, 0x55,     "rate_adp_dbg_p2_0"},
    {1, 0x6184, 5, 31, 0, 0x00,     "rate_adp_dbg_p2_0"},
    {1, 0x6214, 0, 31, 0, 0x55,     "rate_adp_dbg_p3_0"},
    {1, 0x6214, 5, 31, 0, 0x00,     "rate_adp_dbg_p3_0"},
    {1, 0x602c, 0,  2, 2,  0x0, "rg_rate_adapt_ctrl_11"},
    {1, 0x602c, 5,  2, 2,  0x1, "rg_rate_adapt_ctrl_11"},
};

/* clang-format off */
const HAL_SCO_SERDES_DBG_CFG_INFO_T
_hal_sco_serdes_dbg_cfg_info[] =
{
    {
        HAL_PHY_SERDES_INTERFACE_XGMII,
        (sizeof(_hal_sco_serdes_dbg_xgmii_cfg) / sizeof(_hal_sco_serdes_dbg_xgmii_cfg[0])),
        &_hal_sco_serdes_dbg_xgmii_cfg[0]
    },
    {
        HAL_PHY_SERDES_INTERFACE_MII,
        (sizeof(_hal_sco_serdes_dbg_mii_cfg) / sizeof(_hal_sco_serdes_dbg_mii_cfg[0])),
        &_hal_sco_serdes_dbg_mii_cfg[0]
    },
    {
        HAL_PHY_SERDES_READ_CLEAR,
        (sizeof(_hal_sco_serdes_dbg_clear_cfg) / sizeof(_hal_sco_serdes_dbg_clear_cfg[0])),
        &_hal_sco_serdes_dbg_clear_cfg[0]
    }
};
/* clang-format on */

const UI32_T _hal_sco_serdes_dbg_cfg_info_size =
    (sizeof(_hal_sco_serdes_dbg_cfg_info) / sizeof(_hal_sco_serdes_dbg_cfg_info[0]));
