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

/* FILE NAME:  hal_pearl_serdes_param.c
 * PURPOSE:
 *  parameters for pearl serdes
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */

#include <hal/common/hal.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/pearl/hal_pearl_serdes.h>
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_defaultValue[] = {
    {0x0A20,   0, 31, 0,      0x113}, /* rg_hsgmii_mode_interrupt */
    {0x5BF8,   0, 31, 0,  0x6330001}, /* rg_usxgmii_an_control_0 */
    {0xC000,   0, 31, 0,       0x40}, /* usgmii_ctrl_0 */
    {0xE648,   0, 31, 0,  0x1FF02FF}, /* rx_ctrl_8  */
    {0xE408,   0, 31, 0, 0x21170305}, /* pll_ctrl_2 */
    {0xE410,   0, 31, 0,        0x0}, /* pll_ctrl_4 */
    {0xE414,   0, 31, 0,       0x41}, /* pll_ck_ctrl_0 */
    {0xE40C,   0, 31, 0,        0x0}, /* pll_ctrl_3 */
    {0xE32C,   0, 31, 0,        0x0}, /* intf_ctrl_11 */
    {0xE328,   0, 31, 0,        0x0}, /* intf_ctrl_10 */
    {0x0060,   0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x1060,   0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x2060,   0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x3060,   0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x0A24,   0, 31, 0,        0x0}, /* rg_an_sgmii_mode_force */
    {0x4520,   0, 31, 0,        0x0}, /* phy_rx_force_ctrl_0 */
    {0x4018,   0, 31, 0,        0x0}, /* sgmii_sts_ctrl_0 */
    {0xE63C,   0, 31, 0, 0x3C9C4000}, /* rx_ctrl_5  */
    {0xE640,   0, 31, 0,    0x186A0}, /* rx_ctrl_6  */
    {0xE644,   0, 31, 0,      0x500}, /* rx_ctrl_7  */
    {0x4100,   0, 31, 0,    0x10001}, /* msg_rx_ctrl_0 */
    {0xE23C,   0, 31, 0,  0x1010100}, /* ss_lcpll_tdc_flt_5 */
    {0xE230,   0, 31, 0, 0x7FFFFFFF}, /* ss_lcpll_tdc_flt_2 */
    {0xE248,   0, 31, 0, 0x63886595}, /* ss_lcpll_tdc_pcw_1 */
    {0xE690,   0, 31, 0,  0x7800000}, /* rx_ctrl_26 */
    {0x0A08,   0, 31, 0,        0x0}, /* rg_hsgmii_pcs_ctrol_3 */
    {0xE208,   0, 31, 0,        0xA}, /* ss_lcpll_pwctl_setting_2 */
    {0x6100,   0, 31, 0, 0x80000000}, /* rate_adp_p0_ctrl_0 */
    {0x6300,   0, 31, 0,        0x0}, /* mii_ra_an_enable */
    {0xE400,   0, 31, 0,    0x60000}, /* pll_ctrl_0 */
    {0xF008, 500, 31, 0, 0xA4460000}, /* rg_qp_cdr_lpf_bot_lim */
    {0xF00C,   0, 31, 0,        0x4}, /* rg_qp_cdr_lpf_mjv_lim */
    {0xF004,   0, 31, 0,      0x200}, /* rg_qp_rxafe_reserve */
    {0xF020,   0, 31, 0,    0x20840}, /* rg_qp_cdr_force_ibandlpf_r_off */
    {0xF014,   0, 31, 0, 0xC2000000}, /* rg_qp_cdr_lpf_setvalue */
    {0xF018,   0, 31, 0,  0x4001800}, /* rg_qp_cdr_pr_ckref_div1 */
    {0xF01C,   0, 31, 0, 0x18000019}, /* rg_qp_cdr_pr_kband_div_pcie */
    {0xE6D0,   0, 31, 0,       0x64}, /* rx_ctrl_42 */
    {0xE124,   0, 31, 0,        0x2}, /* pon_rxfedig_ctrl_9 */
    {0xE100,   0, 31, 0,  0x2001409}, /* pon_rxfedig_ctrl_0 */
    {0xF03C,   0, 31, 0, 0x102802A2}, /* rg_qp_pll_ipll_dig_pwr_sel */
    {0xF040,   0, 31, 0,        0x3}, /* rg_qp_pll_sdm_ord */
    {0xE630,   0, 31, 0,  0x1000000}, /* rx_ctrl_2  */
    {0xE614,   0, 31, 0,      0xF10}, /*  rx_dly_0  */
    {0xF000,   0, 31, 0,   0x815000}, /* rg_qp_rx_dac_en */
    {0xF028,   0, 31, 0,        0x0}, /* rg_qp_tx_mode_16b_en */
    {0xA324,   0, 31, 0,        0x3}, /* qp_dig_mode_ctrl_0 */
    {0xC15C,   0, 31, 0,    0x10100}, /* qsgmii_rx_fifo_ctrl */
    {0xC158,   0, 31, 0,    0x10100}, /* qsgmii_tx_fifo_ctrl */
    {0x6000,   0, 31, 0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x0A14,   0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x1A14,   0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x2A14,   0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x3A14,   0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x0A00,   0, 31, 0,  0xC9CC000}, /* rg_hsgmii_pcs_ctrol_1 */
    {0x0A14,   0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0xA330,   0, 31, 0,        0x3}, /* qp_dig_mode_ctrl_1 */
    {0xA310,   0, 31, 0,  0x3FFFFFF}, /* qp_ck_rst_ctrl_4 */
    {0x0000,   0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x1000,   0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x2000,   0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x3000,   0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x0010,   0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x1010,   0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x2010,   0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x3010,   0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x0034,   0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x1034,   0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x2034,   0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x3034,   0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x002C,   0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x102C,   0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x202C,   0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x302C,   0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0xE318,   0, 31, 0,        0x0}, /* intf_ctrl_6 */
    {0xE418,   0, 31, 0,      0x107}, /* pll_ck_ctrl_1 */
    {0xE31C,   0, 31, 0,        0x0}, /* intf_ctrl_7 */
    {0xF030,   0, 31, 0,  0x2AA0002}, /* rg_qp_bgr_en */
    {0xE320,   0, 31, 0,        0x0}, /* intf_ctrl_8 */
    {0xF038,   0, 31, 0, 0x80040100}, /* rg_qp_tdc_ft_ck_en */
    {0x4000,   0, 31, 0,       0x20}, /* msg_tx_ctrl_0 */
    {0xC00C,   0, 31, 0, 0x10FF00FF}, /* usgmii_ctrl_3 */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_set5gbaserCfg[] = {
    {0xE328, 0, 20, 16,        0x5}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0xE328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0xE328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0xE328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0xE32C, 0,  5,  0,        0x0}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0xE32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0xF008, 0, 26, 24,        0x4}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0xF008, 0, 22, 20,        0x4}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0xE318, 0, 12, 12,        0x0}, /* RG_DA_QP_TX_DATA_EN_FORCE */
    {0xE318, 0, 13, 13,        0x1}, /* RG_DA_QP_TX_DATA_EN_SEL */
    {0xE418, 0, 19, 19,        0x1}, /* RG_PLL_FORCE_UNSTABLE */
    {0xE400, 0,  0,  0,        0x0}, /* RG_PHYA_AUTO_INIT */
    {0xE400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
    {0xE31C, 0,  4,  4,        0x0}, /* RG_DA_QP_PLL_EN_FORCE */
    {0xE31C, 0,  5,  5,        0x1}, /* RG_DA_QP_PLL_EN_SEL */
    {0xA330, 0,  3,  2,        0x3}, /* RG_TPHY_SPEED */
    {0xE408, 0,  1,  0,        0x3}, /* RG_DA_QP_PLL_BC_INTF */
    {0xE408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0xE408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0xE408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0xE408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0xE410, 0,  2,  2,        0x0}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xE408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0xE408, 0, 19, 16,        0xF}, /* RG_DA_QP_PLL_IR_INTF */
    {0xE408, 0, 21, 20,        0x0}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0xE408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0xE408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0xE408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0xE408, 0, 27, 27,        0x1}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0xE408, 0, 28, 28,        0x1}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0xE410, 0,  4,  3,        0x1}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0xE408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0xE208, 0, 17, 16,        0x3}, /* RG_NCPO_ANA_MSB */
    {0xE230, 0, 30,  0, 0x1C800000}, /* RG_LCPLL_NCPO_VALUE */
    {0xE248, 0, 30,  0, 0x1C800000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0xE23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0xE414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0xE40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0xE410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0xE40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0xE414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0xF03C, 0, 26, 25,        0x1}, /* RG_QP_PLL_PREDIV */
    {0xF040, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0xF040, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0xF000, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0xE100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0xF004, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0xF00C, 0,  5,  4,        0x0}, /* RG_QP_CDR_LPF_RATIO */
    {0xF018, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0xF020, 0, 13, 13,        0x1}, /* RG_QP_CDR_PHYCK_RSTB */
    {0xF01C, 0, 30, 30,        0x1}, /* RG_QP_CDR_PR_XFICK_EN */
    {0xE690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0xE690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0xE690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0xE614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0xE614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0xE6D0, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0xE630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0xE124, 0,  2,  0,        0x7}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0xE648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0xE63C, 0, 29, 10,        0x9}, /* RG_FREDET_CHK_CYCLE */
    {0xE640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0xE644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x5BF8, 0,  0,  0,        0x0}, /* RG_AN_ENABLE */
    {0xC000, 0,  2,  2,        0x1}, /* RG_BASER_MODE */
    {0x4100, 0, 28, 28,        0x1}, /* RG_HSGMII_XFI_SEL */
    {0xA310, 0, 14, 14,        0x1}, /* RG_XFI_PCS_TX_GB_FIFO_WR_RESET_N */
    {0xA310, 0, 15, 15,        0x1}, /* RG_XFI_PCS_TX_GB_FIFO_RD_RESET_N */
    {0xA310, 0, 16, 16,        0x1}, /* RG_XFI_PCS_RX_GB_FIFO_WR_RESET_N */
    {0xA310, 0, 17, 17,        0x1}, /* RG_XFI_PCS_RX_GB_FIFO_RD_RESET_N */
    {0xA310, 0, 18, 18,        0x1}, /* RG_XFI_PCS_GB_LB_FIFO_WR_RESET_N */
    {0xA310, 0, 19, 19,        0x1}, /* RG_XFI_PCS_GB_LB_FIFO_RD_RESET_N */
    {0xA310, 0, 20, 20,        0x1}, /* RG_XFI_RX_BIST_ERR_CNT_RESET_N */
    {0xE31C, 0,  4,  4,        0x1}, /* RG_DA_QP_PLL_EN_FORCE */
    {0xE31C, 0,  5,  5,        0x1}, /* RG_DA_QP_PLL_EN_SEL */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_set5GbaserCfg_pll_config[] = {
    {0xE31C, 0,  4,  4, 0x0}, /* RG_DA_QP_PLL_EN_FORCE */
    {0xE31C, 0,  5,  5, 0x1}, /* RG_DA_QP_PLL_EN_SEL */
    {0xE410, 0,  2,  2, 0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xF030, 0, 26, 21, 0x0}, /* RG_QP_BIAS_V2V_CAL */
    {0xF03C, 0,  6,  5, 0x3}, /* RG_QP_PLL_LDOLPF_VSEL */
    {0xF03C, 0, 19, 19, 0x0}, /* RG_QP_PLL_OSCAL_ENB */
    {0xE408, 0, 29, 29, 0x0}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE320, 0, 12, 11, 0x3}, /* RG_DA_QP_XTAL_EXT_EN_FORCE */
    {0xE320, 0, 13, 13, 0x1}, /* RG_DA_QP_XTAL_EXT_EN_SEL */
    {0xE408, 0, 14, 14, 0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0xF03C, 0, 14, 14, 0x1}, /* RG_QP_PLL_MONVC_EN */
    {0xF03C, 0, 18, 16, 0x3}, /* RG_QP_PLL_MON_LDO_SEL */
    {0xE31C, 0,  4,  4, 0x1}, /* RG_DA_QP_PLL_EN_FORCE */
    {0xE31C, 0,  5,  5, 0x1}, /* RG_DA_QP_PLL_EN_SEL */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_set5GbaserCfg_pll_kbandCode[] = {
    {0xE410, 0,  2,  2,  0x0}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xF030, 0, 26, 21, 0x15}, /* RG_QP_BIAS_V2V_CAL */
    {0xF03C, 0,  6,  5,  0x1}, /* RG_QP_PLL_LDOLPF_VSEL */
    {0xF03C, 0, 19, 19,  0x1}, /* RG_QP_PLL_OSCAL_ENB */
    {0xE408, 0, 29, 29,  0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE320, 0, 12, 11,  0x0}, /* RG_DA_QP_XTAL_EXT_EN_FORCE */
    {0xE320, 0, 13, 13,  0x0}, /* RG_DA_QP_XTAL_EXT_EN_SEL */
    {0xE408, 0, 14, 14,  0x0}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xF038, 0, 19, 19,  0x0}, /* RG_QP_PLL_DEBUG_SEL */
    {0xF038, 0, 19, 19,  0x1}, /* RG_QP_PLL_DEBUG_SEL */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_set5gbaserCfg_pll_forceStable[] = {
    {0xF03C, 0, 14, 14, 0x0}, /* RG_QP_PLL_MONVC_EN */
    {0xF03C, 0, 18, 16, 0x0}, /* RG_QP_PLL_MON_LDO_SEL */
    {0xE31C, 0,  5,  5, 0x0}, /* RG_DA_QP_PLL_EN_SEL */
    {0xE31C, 0,  4,  4, 0x0}, /* RG_DA_QP_PLL_EN_FORCE */
    {0xE418, 0, 19, 19, 0x1}, /* RG_PLL_FORCE_UNSTABLE */
    {0xE418, 0, 18, 18, 0x1}, /* RG_PLL_FORCE_STABLE */
    {0xE418, 0, 19, 19, 0x0}, /* RG_PLL_FORCE_UNSTABLE */
    {0xE418, 0, 18, 18, 0x0}, /* RG_PLL_FORCE_STABLE */
    {0xE318, 0, 12, 12, 0x1}, /* RG_DA_QP_TX_DATA_EN_FORCE */
    {0xE318, 0, 13, 13, 0x0}, /* RG_DA_QP_TX_DATA_EN_SEL */
    {0xE318, 0, 12, 12, 0x0}, /* RG_DA_QP_TX_DATA_EN_FORCE */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_setHsgmiiCfg[] = {
    {0xE328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0xE328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0xE328, 0, 28, 24,        0x4}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0xE328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0xE32C, 0,  5,  0,        0x0}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0xE32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0xF008, 0, 26, 24,        0x5}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0xF008, 0, 22, 20,        0x5}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0xA330, 0,  3,  2,        0x1}, /* RG_TPHY_SPEED */
    {0xE408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0xE408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0xE408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0xE408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0xE408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0xE410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xE408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0xE408, 0, 19, 16,        0x6}, /* RG_DA_QP_PLL_IR_INTF */
    {0xE408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0xE408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0xE408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0xE408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0xE408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0xE408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0xE410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0xE408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0xE208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0xE230, 0, 30,  0, 0x7A000000}, /* RG_LCPLL_NCPO_VALUE */
    {0xE248, 0, 30,  0, 0x7A000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0xE23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0xE414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0xE40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0xE410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0xE40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0xE414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0xF040, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0xF040, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0xF000, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0xE100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0xF028, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0xF028, 0, 31, 16,        0x4}, /* RG_QP_TX_RESERVE */
    {0xF004, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0xF00C, 0,  5,  4,        0x1}, /* RG_QP_CDR_LPF_RATIO */
    {0xF014, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0xF014, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0xF018, 0, 12,  8,        0xF}, /* RG_QP_CDR_PR_DAC_BAND */
    {0xF01C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0xF01C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0xF020, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0xF020, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0xF020, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0xF01C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0xF018, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0xE690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0xE690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0xE690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0xE614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0xE614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0xE6D0, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0xE630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0xE124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0xE648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0xE648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0xE63C, 0, 29, 10,       0x10}, /* RG_FREDET_CHK_CYCLE */
    {0xE640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0xE644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0xE400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
    {0x0A00, 0, 30, 30,        0x0}, /* RG_TBI_10B_MODE */
    {0x6100, 0, 31, 31,        0x0}, /* RG_P0_DIS_MII_MODE */
    {0x6000, 0,  0,  0,        0x1}, /* RG_RATE_ADAPT_TX_EN */
    {0x6000, 0,  4,  4,        0x1}, /* RG_RATE_ADAPT_RX_EN */
    {0x6000, 0, 26, 26,        0x1}, /* RG_RATE_ADAPT_TX_BYPASS */
    {0x6000, 0, 27, 27,        0x1}, /* RG_RATE_ADAPT_RX_BYPASS */
    {0x0000, 0, 12, 12,        0x0}, /* SGMII_AN_ENABLE */
    {0x4018, 0,  2,  2,        0x1}, /* RG_FORCE_SPD_MODE_P0 */
    {0x4018, 0,  5,  4,        0x3}, /* RG_LINK_MODE_P0 */
    {0xE690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0xE690, 0, 19, 19,        0x1}, /* RG_QP_BYPASS_SIG_DET_FREQ_LOCK_RELINK */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_setSgmiiCfg[] = {
    {0xE328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0xE328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0xE328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0xE328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0xE32C, 0,  5,  0,        0xD}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0xE32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0xF008, 0, 26, 24,        0x6}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0xF008, 0, 22, 20,        0x6}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0xA330, 0,  3,  2,        0x0}, /* RG_TPHY_SPEED */
    {0xE408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0xE408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0xE408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0xE408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0xE408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0xE410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xE408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0xE408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0xE408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0xE408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0xE408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0xE408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0xE408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0xE408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0xE410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0xE408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0xE208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0xE230, 0, 30,  0, 0x48000000}, /* RG_LCPLL_NCPO_VALUE */
    {0xE248, 0, 30,  0, 0x48000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0xE23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0xE414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0xE40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0xE410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0xE40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0xE414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0xF040, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0xF040, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0xF000, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0xE100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0xF028, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0xF028, 0, 31, 16,        0x0}, /* RG_QP_TX_RESERVE */
    {0xF004, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0xF00C, 0,  5,  4,        0x2}, /* RG_QP_CDR_LPF_RATIO */
    {0xF014, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0xF014, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0xF018, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0xF01C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0xF01C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0xF020, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0xF020, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0xF020, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0xF01C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0xF018, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0xE690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0xE690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0xE690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0xE614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0xE614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0xE6D0, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0xE630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0xE124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0xE648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0xE648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0xE63C, 0, 29, 10,       0x28}, /* RG_FREDET_CHK_CYCLE */
    {0xE640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0xE644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0xE400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
    {0x0A00, 0, 30, 30,        0x0}, /* RG_TBI_10B_MODE */
    {0x0060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x0034, 0,  5,  0,        0xB}, /* SGMII_IF_MODE_5_0 */
    {0x0034, 0,  8,  8,        0x1}, /* SGMII_REMOTE_FAULT_DIS */
    {0x0000, 0, 12, 12,        0x1}, /* SGMII_AN_ENABLE */
    {0x6300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x6100, 0, 28, 28,        0x1}, /* RG_P0_MII_MODE */
    {0x6000, 0,  0,  0,        0x1}, /* RG_RATE_ADAPT_TX_EN */
    {0x6000, 0,  4,  4,        0x1}, /* RG_RATE_ADAPT_RX_EN */
    {0x6000, 0, 26, 26,        0x1}, /* RG_RATE_ADAPT_TX_BYPASS */
    {0x6000, 0, 27, 27,        0x1}, /* RG_RATE_ADAPT_RX_BYPASS */
    {0x0000, 0,  9,  9,        0x1}, /* SGMII_AN_RESTART */
};

const HAL_PEARL_SERDES_CFG_T _hal_pearl_serdes_set1000basexCfg[] = {
    {0xE328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0xE328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0xE328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0xE328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0xE32C, 0,  5,  0,        0xD}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0xE32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0xF008, 0, 26, 24,        0x6}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0xF008, 0, 22, 20,        0x6}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0xA330, 0,  3,  2,        0x0}, /* RG_TPHY_SPEED */
    {0xE408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0xE408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0xE408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0xE408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0xE408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0xE408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0xE410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0xE408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0xE408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0xE408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0xE408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0xE408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0xE408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0xE408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0xE408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0xE410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0xE408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0xE208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0xE230, 0, 30,  0, 0x48000000}, /* RG_LCPLL_NCPO_VALUE */
    {0xE248, 0, 30,  0, 0x48000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0xE23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0xE414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0xE40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0xE410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0xE40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0xE414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0xF040, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0xF040, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0xF000, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0xE100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0xF028, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0xF028, 0, 31, 16,        0x0}, /* RG_QP_TX_RESERVE */
    {0xF004, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0xF00C, 0,  5,  4,        0x2}, /* RG_QP_CDR_LPF_RATIO */
    {0xF014, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0xF014, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0xF018, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0xF01C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0xF01C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0xF020, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0xF020, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0xF020, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0xF01C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0xF018, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0xE690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0xE690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0xE690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0xE614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0xE614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0xE6D0, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0xE630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0xE124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0xE648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0xE648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0xE63C, 0, 29, 10,       0x28}, /* RG_FREDET_CHK_CYCLE */
    {0xE640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0xE644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0xE400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
    {0x0A00, 0, 30, 30,        0x0}, /* RG_TBI_10B_MODE */
    {0x0060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x0034, 0,  5,  0,        0x0}, /* SGMII_IF_MODE_5_0 */
    {0x0034, 0,  8,  8,        0x0}, /* SGMII_REMOTE_FAULT_DIS */
    {0x0A08, 0, 19,  0,        0x0}, /* RG_LINK_STSTIME */
    {0x002C, 0, 19,  0,       0x50}, /* RG_HSGMII_PCS_CTROL_3 */
    {0x0A00, 0, 24, 24,        0x1}, /* RG_SGMII_SEND_AN_ERROR_EN */
    {0x0A14, 0, 12, 12,        0x1}, /* RG_SGMII_SPD_FORCE_1000 */
    {0x0010, 0, 15,  0,      0x1a0}, /* SGMII_DEV_ABILITY */
    {0x0A20, 0, 31,  0,      0x33F}, /*   mode2    */
    {0x0A20, 0, 31,  0,      0x113}, /*   mode2    */
    {0x0000, 0, 12, 12,        0x1}, /* SGMII_AN_ENABLE */
    {0x6300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x6100, 0, 28, 28,        0x1}, /* RG_P0_MII_MODE */
    {0x6000, 0,  0,  0,        0x1}, /* RG_RATE_ADAPT_TX_EN */
    {0x6000, 0,  4,  4,        0x1}, /* RG_RATE_ADAPT_RX_EN */
    {0x6000, 0, 26, 26,        0x1}, /* RG_RATE_ADAPT_TX_BYPASS */
    {0x6000, 0, 27, 27,        0x1}, /* RG_RATE_ADAPT_RX_BYPASS */
    {0x0000, 0,  9,  9,        0x1}, /* SGMII_AN_RESTART */
};

/* clang-format off */
const HAL_PEARL_SERDES_CFG_INFO_T
_hal_pearl_serdes_cfg_info[] =
{
    {
        HAL_PEARL_SERDES_MODE_DEFAULT_VALUE,
        (sizeof(_hal_pearl_serdes_defaultValue) / sizeof(_hal_pearl_serdes_defaultValue[0])),
        &_hal_pearl_serdes_defaultValue[0]
    },
    {
        HAL_PEARL_SERDES_MODE_5GBASETR,
        (sizeof(_hal_pearl_serdes_set5gbaserCfg) / sizeof(_hal_pearl_serdes_set5gbaserCfg[0])),
        &_hal_pearl_serdes_set5gbaserCfg[0]
    },
    {
        HAL_PEARL_SERDES_MODE_HSGMII,
        (sizeof(_hal_pearl_serdes_setHsgmiiCfg) / sizeof(_hal_pearl_serdes_setHsgmiiCfg[0])),
        &_hal_pearl_serdes_setHsgmiiCfg[0]
    },
    {
        HAL_PEARL_SERDES_MODE_SGMII_AN,
        (sizeof(_hal_pearl_serdes_setSgmiiCfg) / sizeof(_hal_pearl_serdes_setSgmiiCfg[0])),
        &_hal_pearl_serdes_setSgmiiCfg[0]
    },
    {
        HAL_PEARL_SERDES_MODE_1000BASEX,
        (sizeof(_hal_pearl_serdes_set1000basexCfg) / sizeof(_hal_pearl_serdes_set1000basexCfg[0])),
        &_hal_pearl_serdes_set1000basexCfg[0]
    },
};
/* clang-format on */

const UI32_T _hal_pearl_serdes_cfg_info_size =
    (sizeof(_hal_pearl_serdes_cfg_info) / sizeof(_hal_pearl_serdes_cfg_info[0]));
const UI32_T _hal_pearl_serdes_pll_config_cnt =
    (sizeof(_hal_pearl_serdes_set5GbaserCfg_pll_config) / sizeof(_hal_pearl_serdes_set5GbaserCfg_pll_config[0]));
const UI32_T _hal_pearl_serdes_pll_kbandCode =
    (sizeof(_hal_pearl_serdes_set5GbaserCfg_pll_kbandCode) / sizeof(_hal_pearl_serdes_set5GbaserCfg_pll_kbandCode[0]));
const UI32_T _hal_pearl_serdes_pll_forceStable = (sizeof(_hal_pearl_serdes_set5gbaserCfg_pll_forceStable) /
                                                  sizeof(_hal_pearl_serdes_set5gbaserCfg_pll_forceStable[0]));

const HAL_PEARL_SERDES_DBG_CFG_T _hal_pearl_serdes_dbg_xgmii_cfg[] = {
    {1, 0x602C, 0,  1, 1, 0x1,           "RG_XGMII_PKT_RD_TOGGLE"},
    {1, 0x602C, 5,  1, 1, 0x0,           "RG_XGMII_PKT_RD_TOGGLE"},
    {0, 0x60B0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_0_STS(TX_FB)"},
    {0, 0x60B4, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_1_STS(TX_FD)"},
    {0, 0x60B8, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_2_STS(TX_ERR)"},
    {0, 0x60BC, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_3_STS(RX_FB)"},
    {0, 0x60C0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_4_STS(RX_FD)"},
    {0, 0x60C4, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_5_STS(RX_ERR)"},
};

const HAL_PEARL_SERDES_DBG_CFG_T _hal_pearl_serdes_dbg_mii_cfg[] = {
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

const HAL_PEARL_SERDES_DBG_CFG_T _hal_pearl_serdes_dbg_clear_cfg[] = {
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
const HAL_PEARL_SERDES_DBG_CFG_INFO_T
_hal_pearl_serdes_dbg_cfg_info[] =
{
    {
        HAL_PHY_SERDES_INTERFACE_XGMII,
        (sizeof(_hal_pearl_serdes_dbg_xgmii_cfg) / sizeof(_hal_pearl_serdes_dbg_xgmii_cfg[0])),
        &_hal_pearl_serdes_dbg_xgmii_cfg[0]
    },
    {
        HAL_PHY_SERDES_INTERFACE_MII,
        (sizeof(_hal_pearl_serdes_dbg_mii_cfg) / sizeof(_hal_pearl_serdes_dbg_mii_cfg[0])),
        &_hal_pearl_serdes_dbg_mii_cfg[0]
    },
    {
        HAL_PHY_SERDES_READ_CLEAR,
        (sizeof(_hal_pearl_serdes_dbg_clear_cfg) / sizeof(_hal_pearl_serdes_dbg_clear_cfg[0])),
        &_hal_pearl_serdes_dbg_clear_cfg[0]
    }
};
/* clang-format on */

const UI32_T _hal_pearl_serdes_dbg_cfg_info_size =
    (sizeof(_hal_pearl_serdes_dbg_cfg_info) / sizeof(_hal_pearl_serdes_dbg_cfg_info[0]));
