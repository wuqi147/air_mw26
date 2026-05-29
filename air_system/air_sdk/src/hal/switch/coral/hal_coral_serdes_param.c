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

/* FILE NAME:  hal_coral_serdes_param.c
 * PURPOSE:
 *  parameters for coral serdes
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */

#include <hal/common/hal.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/coral/hal_coral_serdes.h>
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_defaultValue[] = {
    {0x1022EF30, 0, 31, 0,  0x2AA0000}, /* rg_qp_bgr_en */
    {0x1022A330, 0, 31, 0,        0x3}, /* qp_dig_mode_ctrl_1 */
    {0x1022E408, 0, 31, 0, 0x21170305}, /* pll_ctrl_2 */
    {0x1022E410, 0, 31, 0,        0x0}, /* pll_ctrl_4 */
    {0x1022E208, 0, 31, 0,        0xA}, /* ss_lcpll_pwctl_setting_2 */
    {0x1022E230, 0, 31, 0, 0x7FFFFFFF}, /* ss_lcpll_tdc_flt_2 */
    {0x1022E248, 0, 31, 0, 0x63886595}, /* ss_lcpll_tdc_pcw_1 */
    {0x1022E23C, 0, 31, 0,  0x1010100}, /* ss_lcpll_tdc_flt_5 */
    {0x1022E414, 0, 31, 0,       0x40}, /* pll_ck_ctrl_0 */
    {0x1022E40C, 0, 31, 0,        0x0}, /* pll_ctrl_3 */
    {0x1022EF40, 0, 31, 0,       0x1B}, /* rg_qp_pll_sdm_ord */
    {0x1022EF3C, 0, 31, 0, 0x102802A2}, /* rg_qp_pll_ipll_dig_pwr_sel */
    {0x1022E100, 0, 31, 0,  0x2001409}, /* pon_rxfedig_ctrl_0 */
    {0x1022EF28, 0, 31, 0,  0x2000000}, /* rg_qp_tx_mode_16b_en */
    {0x1022E000, 0, 31, 0, 0x873C50A0}, /* qp_tx_da_ctrl_0 */
    {0x1022EF04, 0, 31, 0,      0x600}, /* rg_qp_rxafe_reserve */
    {0x1022EF0C, 0, 31, 0,        0x4}, /* rg_qp_cdr_lpf_mjv_lim */
    {0x1022EF08, 0, 31, 0, 0xA4460000}, /* rg_qp_cdr_lpf_bot_lim */
    {0x1022EF14, 0, 31, 0, 0xC2000000}, /* rg_qp_cdr_lpf_setvalue */
    {0x1022EF18, 0, 31, 0,  0x4001800}, /* rg_qp_cdr_pr_ckref_div1 */
    {0x1022EF1C, 0, 31, 0, 0x18000019}, /* rg_qp_cdr_pr_kband_div_pcie */
    {0x1022EF20, 0, 31, 0,    0x20840}, /* rg_qp_cdr_force_ibandlpf_r_off */
    {0x1022EF00, 0, 31, 0,   0x825000}, /* rg_qp_rx_dac_en */
    {0x1022E690, 0, 31, 0,  0x2000000}, /* rx_ctrl_26 */
    {0x1022E614, 0, 31, 0,      0xF10}, /*  rx_dly_0  */
    {0x1022E6DC, 0, 31, 0, 0x254A094C}, /* rx_ctrl_45 */
    {0x1022E630, 0, 31, 0,  0x94C0000}, /* rx_ctrl_2  */
    {0x1022E124, 0, 31, 0,        0x2}, /* pon_rxfedig_ctrl_9 */
    {0x1022E648, 0, 31, 0,  0x1FF02FF}, /* rx_ctrl_8  */
    {0x1022E63C, 0, 31, 0, 0x3C9C4010}, /* rx_ctrl_5  */
    {0x1022E640, 0, 31, 0,    0x186A0}, /* rx_ctrl_6  */
    {0x1022E644, 0, 31, 0,      0x500}, /* rx_ctrl_7  */
    {0x10220A08, 0, 31, 0,        0x0}, /* rg_hsgmii_pcs_ctrol_3 */
    {0x10229100, 0, 31, 0, 0x80000300}, /* rate_adp_p0_ctrl_0 */
    {0x10229000, 0, 31, 0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10229300, 0, 31, 0,        0x0}, /* mii_ra_an_enable */
    {0x10220B88, 0, 31, 0,   0x200004}, /* pcs_dec_erro */
    {0x10220B84, 0, 31, 0,      0x2A5}, /* rg_det_pad_ctrl */
    {0x1022E004, 0, 31, 0,      0x200}, /* qp_tx_da_ctrl_1 */
    {0x1022E400, 0, 31, 0,    0x60000}, /* pll_ctrl_0 */
    {0x10228018, 0, 31, 0,        0x0}, /* xsgmii_sgmii_sts_ctrl_0 */
    {0x10228000, 0, 31, 0,       0x20}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10228100, 0, 31, 0,    0x10001}, /* xsgmii_msg_rx_ctrl_0 */
    {0x10228520, 0, 31, 0,        0x0}, /* xsgmii_phy_rx_force_ctrl_0 */
    {0x1022A324, 0, 31, 0,        0x3}, /* qp_dig_mode_ctrl_0 */
    {0x10220000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10221000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10222000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10223000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10220034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10221034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10222034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10223034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10220060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10221060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10222060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10223060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x1022002C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x1022102C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x1022202C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x1022302C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x10220010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10221010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10222010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10223010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x1022C158, 0, 31, 0,    0x10100}, /* qsgmii_tx_fifo_ctrl */
    {0x1022C15C, 0, 31, 0,    0x10100}, /* qsgmii_rx_fifo_ctrl */
    {0x10270F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10271F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10272F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10273F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10220EF8, 0, 31, 0,  0x6330001}, /* rg_usxgmii_an_control_0 */
    {0x10220F34, 0, 31, 0,  0x3E11111}, /* rg_rtl_modified */
    {0x10220A00, 0, 31, 0,  0xC9CC000}, /* rg_hsgmii_pcs_ctrol_1 */
    {0x10220A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10220A20, 0, 31, 0,      0x113}, /* rg_hsgmii_mode_interrupt */
    {0x10228154, 0, 31, 0,        0x0}, /* xsgmii_multi_sgmii_fpga_mode_control */
    {0x1022B404, 0, 31, 0,        0xC}, /* efifo_ctrl */
    {0x10228104, 0, 31, 0,        0x5}, /* xsgmii_multi_sgmii_reg_phya_65 */
    {0x1022E690, 0, 31, 0,  0x2000000}, /* rx_ctrl_26 */
    {0x10220B88, 0, 31, 0,   0x200004}, /* pcs_dec_erro */
};

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_defaultValue[] = {
    {0x10250114, 0, 31, 0,    0x40000}, /* rg_xpon_rx_sigdet_novth */
    {0x1025177C, 0, 31, 0,        0x0}, /* rg_force_da_pxp_tx_term_sel */
    {0x10250000, 0, 31, 0, 0x10040000}, /* rg_xpon_cmn_en */
    {0x10251364, 0, 31, 0,        0x0}, /* add_dig_reserve_1 */
    {0x10251990, 0, 31, 0,  0x9C40000}, /* rx_hw_ctrl_0 */
    {0x10251460, 0, 31, 0,        0x0}, /* sw_rst_set */
    {0x10251950, 0, 31, 0,        0x0}, /* add_dig_reserve_45 */
    {0x10251414, 0, 31, 0,        0x0}, /* add_xpon_mode_1 */
    {0x1025011C, 0, 31, 0,      0x400}, /* rg_xpon_rx_fe_vcm_gen_pwdb */
    {0x102500A8, 0, 31, 0,        0x0}, /* rg_xpon_tdc_sync_ck_sel */
    {0x10250004, 0, 31, 0,      0x101}, /* rg_xpon_cmn_mode_sel */
    {0x10250084, 0, 31, 0,  0x101031B}, /* rg_xpon_txpll_ssc_period */
    {0x10250088, 0, 31, 0,        0x0}, /* rg_xpon_txpll_vtp_en */
    {0x10250064, 0, 31, 0,    0x40001}, /* rg_xpon_txpll_refin_internal */
    {0x10251854, 0, 31, 0,        0x0}, /* rg_force_da_pxp_txpll_ckout_en */
    {0x10250068, 0, 31, 0,      0x300}, /* rg_xpon_txpll_sdm_di_en */
    {0x1025006C, 0, 31, 0,  0x1000003}, /* rg_xpon_txpll_sdm_ord */
    {0x10250080, 0, 31, 0,   0x820082}, /* rg_xpon_txpll_ssc_delta1 */
    {0x1025007C, 0, 31, 0,    0x10000}, /* rg_xpon_txpll_ssc_en */
    {0x10250098, 0, 31, 0,        0x0}, /* rg_xpon_txpll_chp_double_en */
    {0x10250050, 0, 31, 0, 0x1F05000C}, /* rg_xpon_txpll_chp_ibias */
    {0x10250054, 0, 31, 0,        0x5}, /* rg_xpon_txpll_lpf_bp */
    {0x10250074, 0, 31, 0,  0x3000001}, /* rg_xpon_txpll_tcl_lpf_en */
    {0x10250078, 0, 31, 0,  0x4040401}, /* rg_xpon_txpll_vco_halflsb_en */
    {0x10251794, 0, 31, 0,        0x0}, /* rg_force_da_pxp_cdr_pr_idac */
    {0x10251798, 0, 31, 0,        0x0}, /* rg_force_da_pxp_txpll_sdm_pcw */
    {0x10251048, 0, 31, 0,  0x8400000}, /* ss_lcpll_tdc_pcw_1 */
    {0x1025104C, 0, 31, 0,  0x8400000}, /* ss_lcpll_tdc_pcw_2 */
    {0x10250058, 0, 31, 0,  0x30003FF}, /* rg_xpon_txpll_kband_code */
    {0x1025005C, 0, 31, 0,      0x100}, /* rg_xpon_txpll_kband_ks */
    {0x10250094, 0, 31, 0,    0x10010}, /* rg_xpon_txpll_tcl_kband_vref */
    {0x10251858, 0, 31, 0,        0x0}, /* rg_force_da_pxp_txpll_kband_load_en */
    {0x10250070, 0, 31, 0,  0x4000903}, /* rg_xpon_txpll_tcl_amp_gain */
    {0x102500B4, 0, 31, 0,        0x2}, /* rg_xpon_tx_ser_loadsel */
    {0x102500C4, 0, 31, 0,    0x10400}, /* rg_xpon_tx_ckldo_en */
    {0x102500C8, 0, 31, 0, 0xF0000000}, /* rg_xpon_tx_termcal_selpn */
    {0x10251874, 0, 31, 0,        0x0}, /* rg_force_da_pxp_tx_acjtag_en */
    {0x10251784, 0, 31, 0,        0x0}, /* rg_force_da_pxp_tx_rate_ctrl */
    {0x102518EC, 0, 31, 0,        0x0}, /* add_dig_reserve_20 */
    {0x1025199C, 0, 31, 0, 0x1001405F}, /* rx_hw_ctrl_3 */
    {0x10251320, 0, 31, 0,  0x1010101}, /* rx_disb_mode_4 */
    {0x1025148C, 0, 31, 0,  0x1000203}, /* rx_extral_ctrl */
    {0x102500DC, 0, 31, 0,      0x100}, /* rg_xpon_cdr_pd_pical_ckd8_inv */
    {0x10250118, 0, 31, 0,        0x0}, /* rg_xpon_rx_fe_eq_hzen */
    {0x1025188C, 0, 31, 0,        0x0}, /* rg_force_da_pxp_rx_fe_gain_ctrl */
    {0x102500D4, 0, 31, 0, 0x4CCB1030}, /* rg_xpon_rx_rev_0 */
    {0x10250120, 0, 31, 0,        0x0}, /* rg_xpon_rx_oscal_force */
    {0x10250110, 0, 31, 0,      0x200}, /* rg_xpon_rx_dac_range */
    {0x10251840, 0, 31, 0,        0x0}, /* rg_force_da_pxp_rx_oscal_en */
    {0x1025176C, 0, 31, 0,        0x0}, /* rg_force_da_pxp_aeq_speed */
    {0x10251814, 0, 31, 0,        0x0}, /* rg_force_da_pxp_aeq_rstb */
    {0x102500E8, 0, 31, 0,  0x2000000}, /* rg_xpon_cdr_lpf_ratio */
    {0x102500F8, 0, 31, 0,  0x4010808}, /* rg_xpon_cdr_pr_beta_dac */
    {0x102500FC, 0, 31, 0,    0x80505}, /* rg_xpon_cdr_pr_vreg_iband_val */
    {0x1025010C, 0, 31, 0,    0x70600}, /* rg_xpon_cdr_pr_monpr_en */
    {0x102500D8, 0, 31, 0,  0x100000A}, /* rg_xpon_rx_phyck_div */
    {0x102500CC, 0, 31, 0,        0x0}, /* rg_xpon_rx_busbit_sel */
    {0x102518B8, 0, 31, 0,        0x0}, /* add_dig_reserve_12 */
    {0x10251090, 0, 31, 0,  0x3E80002}, /* rx_ctrl_sequence_ctrl_1 */
    {0x1025109C, 0, 31, 0,  0x3E80002}, /* rx_ctrl_sequence_ctrl_4 */
    {0x10251094, 0, 31, 0,  0x3E80002}, /* rx_ctrl_sequence_ctrl_2 */
    {0x10251098, 0, 31, 0,  0x3E80002}, /* rx_ctrl_sequence_ctrl_3 */
    {0x10251100, 0, 31, 0,   0xC80064}, /* rx_ctrl_sequence_ctrl_5 */
    {0x10251148, 0, 31, 0, 0x109F0900}, /* ss_rx_blwc */
    {0x10251170, 0, 31, 0,     0xA503}, /* ss_rx_fll_0 */
    {0x10251178, 0, 31, 0,    0x20403}, /* ss_rx_fll_2 */
    {0x10251374, 0, 31, 0,        0x2}, /* rg_xpon_rx_reserved_1 */
    {0x10251580, 0, 31, 0,        0x2}, /* xpon_tx_rate_ctrl */
    {0x1025133C, 0, 31, 0,  0x1010101}, /* rx_disb_mode_8 */
    {0x10251330, 0, 31, 0,        0x0}, /* rx_force_mode_9 */
    {0x10240A00, 0, 31, 0,  0xC9CC000}, /* rg_hsgmii_pcs_ctrol_1 */
    {0x10248000, 0, 31, 0,       0x20}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10240000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10240034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10240060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x1024002C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x10240010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10249100, 0, 31, 0, 0x80000300}, /* rate_adp_p0_ctrl_0 */
    {0x10249000, 0, 31, 0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10249300, 0, 31, 0,        0x0}, /* mii_ra_an_enable */
    {0x10240B88, 0, 31, 0,   0x200004}, /* pcs_dec_erro */
    {0x10240B84, 0, 31, 0,      0x2A5}, /* rg_det_pad_ctrl */
    {0x10250060, 0, 31, 0,      0x101}, /* rg_xpon_txpll_phy_ck1_en */
    {0x10248018, 0, 31, 0,        0x0}, /* xsgmii_sgmii_sts_ctrl_0 */
    {0x1025120C, 0, 31, 0,     0xFFFF}, /* rx_debug_0 */
    {0x10251958, 0, 31, 0,    0x7FFFF}, /* add_dig_reserve_47 */
    {0x10248100, 0, 31, 0,    0x10001}, /* xsgmii_msg_rx_ctrl_0 */
    {0x10248520, 0, 31, 0,        0x0}, /* xsgmii_phy_rx_force_ctrl_0 */
    {0x1024A324, 0, 31, 0,        0x3}, /* qp_dig_mode_ctrl_0 */
    {0x10241000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10242000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10243000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10241034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10242034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10243034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10241060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10242060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10243060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x1024102C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x1024202C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x1024302C, 0, 31, 0,    0x98968}, /* sgmii_reg_an_11 */
    {0x10241010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10242010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10243010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x1024C158, 0, 31, 0,    0x10100}, /* qsgmii_tx_fifo_ctrl */
    {0x1024C15C, 0, 31, 0,    0x10100}, /* qsgmii_rx_fifo_ctrl */
    {0x10270F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10271F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10272F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10273F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10240EF8, 0, 31, 0,  0x6330001}, /* rg_usxgmii_an_control_0 */
    {0x10240F00, 0, 31, 0,       0x7B}, /* rg_usxgmii_an_control_2 */
    {0x10240F04, 0, 31, 0,       0x7B}, /* rg_usxgmii_an_control_3 */
    {0x10240F08, 0, 31, 0,       0x7B}, /* rg_usxgmii_an_control_4 */
    {0x10240F20, 0, 31, 0,        0x0}, /* rg_usxgmii_an_control_7 */
    {0x102480B8, 0, 31, 0,  0x3333450}, /* usxgmii_spd_sel_0 */
    {0x10240EFC, 0, 31, 0,     0x1601}, /* rg_usxgmii_an_control_1 */
    {0x10240F1C, 0, 31, 0,        0x0}, /* rg_usxgmii_an_control_6 */
    {0x10244000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10245000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10246000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10247000, 0, 31, 0,     0x1140}, /* sgmii_reg_an0 */
    {0x10244034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10245034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10246034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10247034, 0, 31, 0, 0x31120009}, /* sgmii_reg_an_13 */
    {0x10244060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10245060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10246060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x10247060, 0, 31, 0,        0x0}, /* sgmii_reg_an_force_cl37 */
    {0x1024402C, 0, 31, 0,        0x0}, /* sgmii_reg_an_11 */
    {0x1024502C, 0, 31, 0,        0x0}, /* sgmii_reg_an_11 */
    {0x1024602C, 0, 31, 0,        0x0}, /* sgmii_reg_an_11 */
    {0x1024702C, 0, 31, 0,        0x0}, /* sgmii_reg_an_11 */
    {0x10244010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10245010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10246010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10247010, 0, 31, 0,        0x1}, /* sgmii_reg_an_4 */
    {0x10241A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10242A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10243A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10244A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10245A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10246A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10247A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x10240BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10241BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10242BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10243BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10244BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10245BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10246BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10247BA8, 0, 31, 0,        0x6}, /* usgmii_reg_an */
    {0x10249400, 0, 31, 0, 0x80000300}, /* rate_adp_p4_ctrl_0 */
    {0x10249600, 0, 31, 0,        0x0}, /* mii_ra_an_enable_p4_p7 */
    {0x10274F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10275F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10276F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10277F00, 0, 31, 0, 0x50080808}, /* sys_config0 */
    {0x10240A08, 0, 31, 0,        0x0}, /* rg_hsgmii_pcs_ctrol_3 */
    {0x10240A20, 0, 31, 0,      0x113}, /* rg_hsgmii_mode_interrupt */
    {0x102500A0, 0, 31, 0,  0x1000009}, /* rg_xpon_pll_monclk_sel */
    {0x102518F8, 0, 31, 0,        0x0}, /* add_dig_reserve_23 */
    {0x10248154, 0, 31, 0,        0x0}, /* xsgmii_multi_sgmii_fpga_mode_control */
    {0x1024B404, 0, 31, 0,        0xC}, /* efifo_ctrl */
    {0x10248104, 0, 31, 0,        0x5}, /* xsgmii_multi_sgmii_reg_phya_65 */
    {0x10251260, 0, 31, 0,        0x0}, /* ss_tx_rst_b */
    {0x10251994, 0, 31, 0, 0x138803CC}, /* rx_hw_ctrl_1 */
    {0x10240A14, 0, 31, 0,       0x13}, /* rg_hsgmii_pcs_ctrol_6 */
    {0x1028C840, 0, 31, 0,        0x0}, /* rg_force_mac9_sb */
    {0x102519A0, 0,  1, 1,        0x1}, /* xpon_int_sta_5 */
};

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_setLck2Data_patch[] = {
    {0x10251990, 0,  6,  6, 0x1}, /* RG_XPON_FORCE_CDR_LCK2DATA_SEL */
    {0x10251958, 0, 16, 16, 0x0}, /* USGMII_TXPMA_SW_RST_N */
    {0x10251958, 0, 17, 17, 0x0}, /* USGMII_RXPMA_SW_RST_N */
    {0x10251958, 0, 18, 18, 0x0}, /* FX_EFIFO_SW_RST_N */
    {0x10251460, 0,  6,  6, 0x0}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0, 0x0}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251958, 0, 16, 16, 0x1}, /* USGMII_TXPMA_SW_RST_N */
    {0x10251958, 0, 17, 17, 0x1}, /* USGMII_RXPMA_SW_RST_N */
    {0x10251958, 0, 18, 18, 0x1}, /* FX_EFIFO_SW_RST_N */
    {0x10251460, 0,  6,  6, 0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0, 0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251990, 1,  6,  6, 0x0}, /* RG_XPON_FORCE_CDR_LCK2DATA_SEL */
    {0x10240000, 0, 15, 15, 0x1}, /* SGMII_RESET_PHY */
};

const UI32_T _hal_coral_serdes_pxp_setLck2Data_patch_size =
    (sizeof(_hal_coral_serdes_pxp_setLck2Data_patch) / sizeof(_hal_coral_serdes_pxp_setLck2Data_patch[0]));

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_setUsxgmiiCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251364, 0, 12,  8,        0x2}, /* HW_DA_PXP_TX_FIR_CN1 */
    {0x10251364, 0, 20, 16,        0x6}, /* HW_DA_PXP_TX_FIR_C1 */
    {0x10251364, 0, 26, 24,        0x0}, /* HW_DA_PXP_TX_FIR_C2 */
    {0x10251364, 0,  5,  0,        0x9}, /* HW_DA_PXP_TX_FIR_C0B */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251950, 0, 14, 12,        0x1}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x10251414, 0, 11,  9,        0x0}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x0}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x1}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,       0x00}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,       0x01}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,       0x00}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,       0x03}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,       0x01}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x1F}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,       0x02}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,       0x07}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,       0x04}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,       0x00}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,       0x04}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x33900000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x67200000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x67200000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0B}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x1}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x102500C8, 0, 31, 24,        0x0}, /* RG_XPON_TX_RESERVED */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x5}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x2}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x1}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x1}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x1030}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x0}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x0}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x8}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x8}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x0}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x7}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,       0x42}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x2}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x0}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x0}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x2}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x2}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x1025199C, 0, 25, 25,        0x1}, /* RG_RETRAIN_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240F00, 0, 31,  0,       0x7B}, /* rg_usxgmii_an_control_2 */
    {0x10240F04, 0, 31,  0,       0x7B}, /* rg_usxgmii_an_control_3 */
    {0x10240F08, 0, 31,  0,       0x7B}, /* rg_usxgmii_an_control_4 */
    {0x10240F20, 0, 31,  0,   0x110000}, /* rg_usxgmii_an_control_7 */
    {0x10240A00, 0, 30, 30,        0x0}, /* RG_TBI_10B_MODE */
    {0x10248000, 0, 31,  0, 0x80080020}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10248100, 0, 31,  0, 0x10010001}, /* xsgmii_msg_rx_ctrl_0 */
    {0x102480B8, 0, 31,  0,  0x3333451}, /* usxgmii_spd_sel_0 */
    {0x10240EF8, 0, 31,  0,  0x6330001}, /* rg_usxgmii_an_control_0 */
    {0x10240EFC, 0, 15,  0,     0x1601}, /* RG_AN_ABILITY */
    {0x10249000, 0, 31,  0,      0xC11}, /* rg_rate_adapt_ctrl_0 */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
    {0x1025120C, 0, 24, 24,        0x1}, /* RG_RO_TOGGLE */
    {0x1025120C, 0, 24, 24,        0x0}, /* RG_RO_TOGGLE */
    {0x10240F1C, 0,  0,  0,        0x1}, /* RG_TOG_PCS_AUTONEG_STS */
    {0x10240F1C, 0,  0,  0,        0x0}, /* RG_TOG_PCS_AUTONEG_STS */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_PXP_USXGMII-R06.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_set5gbaserCfg[] = {
    {0x1022E324, 0,  6,  4,        0x1}, /* RG_DA_QP_TX_FIR_CN1_FORCE */
    {0x1022E324, 0,  7,  7,        0x1}, /* RG_DA_QP_TX_FIR_CN1_SEL */
    {0x1022E328, 0, 20, 16,        0x5}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0x1022E328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0x1022E328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0x1022E328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0x1022E32C, 0,  5,  0,        0x0}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0x1022E32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0x1022EF30, 0,  1,  1,        0x1}, /* RG_QP_CHPEN */
    {0x1022A330, 0,  3,  2,        0x3}, /* RG_TPHY_SPEED */
    {0x1022E408, 0,  1,  0,        0x3}, /* RG_DA_QP_PLL_BC_INTF */
    {0x1022E408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0x1022E408, 0,  7,  6,        0x1}, /* RG_DA_QP_PLL_BPB_INTF */
    {0x1022E408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0x1022E408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0x1022E408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0x1022E410, 0,  2,  2,        0x0}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0x1022E408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0x1022E408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0x1022E408, 0, 21, 20,        0x0}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0x1022E408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0x1022E408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0x1022E408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0x1022E408, 0, 27, 27,        0x1}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0x1022E408, 0, 28, 28,        0x1}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0x1022E410, 0,  4,  3,        0x1}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0x1022E408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0x1022E208, 0, 17, 16,        0x3}, /* RG_NCPO_ANA_MSB */
    {0x1022E230, 0, 30,  0, 0x1C800000}, /* RG_LCPLL_NCPO_VALUE */
    {0x1022E248, 0, 30,  0, 0x1C800000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1022E23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0x1022E414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0x1022E40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0x1022E410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0x1022E40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0x1022E414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0x1022EF40, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0x1022EF40, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0x1022EF3C, 0, 26, 25,        0x1}, /* RG_QP_PLL_PREDIV */
    {0x1022E100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0x1022EF28, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0x1022EF28, 0, 31, 16,      0x300}, /* RG_QP_TX_RESERVE */
    {0x1022E000, 0,  0,  0,        0x0}, /* RG_FVAL_TX_EIDLE_LP_OFF */
    {0x1022EF04, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0x1022EF04, 0, 10, 10,        0x0}, /* RG_QP_CDR_PD_EDGE_DIS */
    {0x1022EF0C, 0,  5,  4,        0x0}, /* RG_QP_CDR_LPF_RATIO */
    {0x1022EF08, 0, 26, 24,        0x4}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0x1022EF08, 0, 22, 20,        0x4}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0x1022EF14, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0x1022EF14, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0x1022EF18, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0x1022EF1C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0x1022EF1C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0x1022EF20, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0x1022EF20, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0x1022EF20, 0, 13, 13,        0x1}, /* RG_QP_CDR_PHYCK_RSTB */
    {0x1022EF1C, 0, 30, 30,        0x1}, /* RG_QP_CDR_PR_XFICK_EN */
    {0x1022EF18, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0x1022EF00, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0x1022E690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0x1022E690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0x1022E690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0x1022E614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0x1022E614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0x1022E6DC, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0x1022E630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0x1022E124, 0,  2,  0,        0x7}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0x1022E648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0x1022E690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0x1022E694, 0, 31,  0,  0x17D7840}, /* RG_QP_LINK_ERRO_CNT */
    {0x1022E63C, 0, 29, 10,        0x9}, /* RG_FREDET_CHK_CYCLE */
    {0x1022E640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0x1022E644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x1022E654, 0,  0,  0,        0x1}, /* RG_FORCE_FREQ_LOCK_SEL */
    {0x1022E654, 0,  1,  1,        0x1}, /* RG_FORCE_FREQ_LOCK */
    {0x10220EF8, 0,  0,  0,        0x0}, /* RG_AN_ENABLE */
    {0x10228000, 0, 31, 31,        0x1}, /* RG_RX_PCS_RDY_CTRL */
    {0x10228000, 0, 22, 20,        0x4}, /* RG_BASER_SPD */
    {0x10228000, 0, 18, 18,        0x1}, /* RG_BASER_MODE */
    {0x10228100, 0, 28, 28,        0x1}, /* RG_HSGMII_XFI_SEL */
    {0x10220B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10220B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1022E004, 0,  7,  7,        0x1}, /* RG_FORCE_TX_BIT_INVERSE */
    {0x10220F34, 0, 12, 12,        0x0}, /* RG_MODIFIED_PCS_RX_64B66BDEC_FSM_1 */
    {0x1022E400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_QP_5GBASE_R-R06.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_set5gbaserCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251364, 0, 12,  8,        0x1}, /* HW_DA_PXP_TX_FIR_CN1 */
    {0x10251364, 0, 20, 16,        0x6}, /* HW_DA_PXP_TX_FIR_C1 */
    {0x10251364, 0, 26, 24,        0x0}, /* HW_DA_PXP_TX_FIR_C2 */
    {0x10251364, 0,  5,  0,        0x0}, /* HW_DA_PXP_TX_FIR_C0B */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251950, 0, 14, 12,        0x1}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x10251414, 0, 11,  9,        0x1}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x1}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x1}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,       0x00}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,       0x01}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,       0x00}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,       0x03}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,       0x01}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x1F}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,       0x02}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,       0x07}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,       0x04}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,       0x00}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,       0x04}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x33900000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x67200000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x67200000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0B}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x2}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x102500C8, 0, 31, 24,        0x0}, /* RG_XPON_TX_RESERVED */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x4}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x2}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x1}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x3}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x18B0}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x1}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x1}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x8}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x8}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x0}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x7}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,       0x42}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x2}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x1}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x1}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x2}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x2}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x1025199C, 0, 25, 25,        0x1}, /* RG_RETRAIN_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240A00, 0, 30, 30,        0x0}, /* RG_TBI_10B_MODE */
    {0x10240EF8, 0,  0,  0,        0x0}, /* RG_AN_ENABLE */
    {0x10248000, 0, 31, 31,        0x1}, /* RG_RX_PCS_RDY_CTRL */
    {0x10248000, 0, 22, 20,        0x4}, /* RG_BASER_SPD */
    {0x10248000, 0, 18, 18,        0x1}, /* RG_BASER_MODE */
    {0x10248100, 0, 28, 28,        0x1}, /* RG_HSGMII_XFI_SEL */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
    {0x1025120C, 0, 24, 24,        0x1}, /* RG_RO_TOGGLE */
    {0x1025120C, 0, 24, 24,        0x0}, /* RG_RO_TOGGLE */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_PXP_5GBASE_R-R06.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_setHsgmiiCfg[] = {
    {0x1022E324, 0,  6,  4,        0x0}, /* RG_DA_QP_TX_FIR_CN1_FORCE */
    {0x1022E324, 0,  7,  7,        0x1}, /* RG_DA_QP_TX_FIR_CN1_SEL */
    {0x1022E328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0x1022E328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0x1022E328, 0, 28, 24,        0x3}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0x1022E328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0x1022E32C, 0,  5,  0,        0x2}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0x1022E32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0x1022EF30, 0,  1,  1,        0x1}, /* RG_QP_CHPEN */
    {0x1022A330, 0,  3,  2,        0x1}, /* RG_TPHY_SPEED */
    {0x1022E408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0x1022E408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0x1022E408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0x1022E408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0x1022E408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0x1022E408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0x1022E410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0x1022E408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0x1022E408, 0, 19, 16,        0x6}, /* RG_DA_QP_PLL_IR_INTF */
    {0x1022E408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0x1022E408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0x1022E408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0x1022E408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0x1022E408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0x1022E408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0x1022E410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0x1022E408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0x1022E208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0x1022E230, 0, 30,  0, 0x7A000000}, /* RG_LCPLL_NCPO_VALUE */
    {0x1022E248, 0, 30,  0, 0x7A000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1022E23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0x1022E414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0x1022E40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0x1022E410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0x1022E40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0x1022E414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0x1022EF40, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0x1022EF40, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0x1022EF3C, 0, 26, 25,        0x0}, /* RG_QP_PLL_PREDIV */
    {0x1022E100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0x1022EF28, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0x1022EF28, 0, 31, 16,      0x304}, /* RG_QP_TX_RESERVE */
    {0x1022E000, 0,  0,  0,        0x0}, /* RG_FVAL_TX_EIDLE_LP_OFF */
    {0x1022EF04, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0x1022EF04, 0, 10, 10,        0x0}, /* RG_QP_CDR_PD_EDGE_DIS */
    {0x1022EF0C, 0,  5,  4,        0x1}, /* RG_QP_CDR_LPF_RATIO */
    {0x1022EF08, 0, 26, 24,        0x5}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0x1022EF08, 0, 22, 20,        0x5}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0x1022EF14, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0x1022EF14, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0x1022EF18, 0, 12,  8,        0xF}, /* RG_QP_CDR_PR_DAC_BAND */
    {0x1022EF1C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0x1022EF1C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0x1022EF20, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0x1022EF20, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0x1022EF20, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0x1022EF1C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0x1022EF18, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0x1022EF00, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0x1022E690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0x1022E690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0x1022E690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0x1022E614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0x1022E614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0x1022E6DC, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0x1022E630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0x1022E124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0x1022E648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0x1022E648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0x1022E690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0x1022E694, 0, 31,  0,  0x17D7840}, /* RG_QP_LINK_ERRO_CNT */
    {0x1022E63C, 0, 29, 10,       0x10}, /* RG_FREDET_CHK_CYCLE */
    {0x1022E640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0x1022E644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x1022E654, 0,  0,  0,        0x1}, /* RG_FORCE_FREQ_LOCK_SEL */
    {0x1022E654, 0,  1,  1,        0x1}, /* RG_FORCE_FREQ_LOCK */
    {0x10220000, 0, 12, 12,        0x0}, /* SGMII_AN_ENABLE */
    {0x10229100, 0, 31,  0,      0x300}, /* rate_adp_p0_ctrl_0 */
    {0x10229000, 0,  0,  0,        0x1}, /* RG_RATE_ADAPT_TX_EN */
    {0x10229000, 0,  4,  4,        0x1}, /* RG_RATE_ADAPT_RX_EN */
    {0x10229000, 0, 26, 26,        0x1}, /* RG_RATE_ADAPT_TX_BYPASS */
    {0x10229000, 0, 27, 27,        0x1}, /* RG_RATE_ADAPT_RX_BYPASS */
    {0x10228018, 0,  2,  2,        0x1}, /* RG_FORCE_SPD_MODE_P0 */
    {0x10228018, 0,  5,  4,        0x3}, /* RG_LINK_MODE_P0 */
    {0x10220B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10220B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1022E004, 0,  7,  7,        0x1}, /* RG_FORCE_TX_BIT_INVERSE */
    {0x1022E400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_QP_HSGMII-R07.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_setHsgmiiCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251364, 0, 12,  8,        0x1}, /* HW_DA_PXP_TX_FIR_CN1 */
    {0x10251364, 0, 20, 16,        0x4}, /* HW_DA_PXP_TX_FIR_C1 */
    {0x10251364, 0, 26, 24,        0x0}, /* HW_DA_PXP_TX_FIR_C2 */
    {0x10251364, 0,  5,  0,        0x2}, /* HW_DA_PXP_TX_FIR_C0B */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251950, 0, 14, 12,        0x2}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x10251414, 0, 11,  9,        0x4}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x4}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x1}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,        0x3}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,        0x0}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,        0x0}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,        0x0}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,        0x0}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,        0x0}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x1F}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,        0x7}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,        0x4}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,        0x0}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x3E800000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x7D000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x7D000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x01}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0E}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x2}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x4}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x0}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x3}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x18B0}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x1}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x1}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x6}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x6}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x1}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x6}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,        0xB}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x1}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x0}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x0}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x102518B8, 0,  0,  0,        0x1}, /* RG_FEOS_8B_MODE */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x0}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x1}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x1025199C, 0, 25, 25,        0x1}, /* RG_RETRAIN_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240A00, 0, 30, 30,        0x1}, /* RG_TBI_10B_MODE */
    {0x10248000, 0, 31,  0,       0x20}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10240000, 0, 12, 12,        0x0}, /* SGMII_AN_ENABLE */
    {0x10249100, 0, 31,  0,      0x300}, /* rate_adp_p0_ctrl_0 */
    {0x10249000, 0,  0,  0,        0x1}, /* RG_RATE_ADAPT_TX_EN */
    {0x10249000, 0,  4,  4,        0x1}, /* RG_RATE_ADAPT_RX_EN */
    {0x10249000, 0, 26, 26,        0x1}, /* RG_RATE_ADAPT_TX_BYPASS */
    {0x10249000, 0, 27, 27,        0x1}, /* RG_RATE_ADAPT_RX_BYPASS */
    {0x10248018, 0,  2,  2,        0x1}, /* RG_FORCE_SPD_MODE_P0 */
    {0x10248018, 0,  5,  4,        0x3}, /* RG_LINK_MODE_P0 */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
    {0x1025120C, 0, 24, 24,        0x1}, /* RG_RO_TOGGLE */
    {0x1025120C, 0, 24, 24,        0x0}, /* RG_RO_TOGGLE */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_PXP_HSGMII-R10.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_setSgmiiCfg[] = {
    {0x1022E324, 0,  6,  4,        0x0}, /* RG_DA_QP_TX_FIR_CN1_FORCE */
    {0x1022E324, 0,  7,  7,        0x1}, /* RG_DA_QP_TX_FIR_CN1_SEL */
    {0x1022E328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0x1022E328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0x1022E328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0x1022E328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0x1022E32C, 0,  5,  0,        0xD}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0x1022E32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0x1022EF30, 0,  1,  1,        0x1}, /* RG_QP_CHPEN */
    {0x1022A330, 0,  3,  2,        0x0}, /* RG_TPHY_SPEED */
    {0x1022E408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0x1022E408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0x1022E408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0x1022E408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0x1022E408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0x1022E408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0x1022E410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0x1022E408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0x1022E408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0x1022E408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0x1022E408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0x1022E408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0x1022E408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0x1022E408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0x1022E408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0x1022E410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0x1022E408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0x1022E208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0x1022E230, 0, 30,  0, 0x48000000}, /* RG_LCPLL_NCPO_VALUE */
    {0x1022E248, 0, 30,  0, 0x48000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1022E23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0x1022E414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0x1022E40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0x1022E410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0x1022E40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0x1022E414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0x1022EF40, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0x1022EF40, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0x1022EF3C, 0, 26, 25,        0x0}, /* RG_QP_PLL_PREDIV */
    {0x1022E100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0x1022EF28, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0x1022EF28, 0, 31, 16,      0x300}, /* RG_QP_TX_RESERVE */
    {0x1022E000, 0,  0,  0,        0x0}, /* RG_FVAL_TX_EIDLE_LP_OFF */
    {0x1022EF04, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0x1022EF04, 0, 10, 10,        0x0}, /* RG_QP_CDR_PD_EDGE_DIS */
    {0x1022EF0C, 0,  5,  4,        0x2}, /* RG_QP_CDR_LPF_RATIO */
    {0x1022EF08, 0, 26, 24,        0x6}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0x1022EF08, 0, 22, 20,        0x6}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0x1022EF14, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0x1022EF14, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0x1022EF18, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0x1022EF1C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0x1022EF1C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0x1022EF20, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0x1022EF20, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0x1022EF20, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0x1022EF1C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0x1022EF18, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0x1022EF00, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0x1022E690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0x1022E690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0x1022E690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0x1022E614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0x1022E614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0x1022E6DC, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0x1022E630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0x1022E124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0x1022E648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0x1022E648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0x1022E690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0x1022E694, 0, 31,  0,  0x17D7840}, /* RG_QP_LINK_ERRO_CNT */
    {0x1022E63C, 0, 29, 10,       0x28}, /* RG_FREDET_CHK_CYCLE */
    {0x1022E640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0x1022E644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x1022E654, 0,  0,  0,        0x1}, /* RG_FORCE_FREQ_LOCK_SEL */
    {0x1022E654, 0,  1,  1,        0x1}, /* RG_FORCE_FREQ_LOCK */
    {0x10220000, 0, 31,  0,     0x1140}, /* sgmii_reg_an0 */
    {0x10220034, 0, 31,  0, 0x3112010B}, /* sgmii_reg_an_13 */
    {0x10220060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1022002C, 0, 19,  0,    0x186A0}, /* SGMII_LINK_TIMER */
    {0x10220010, 0, 31,  0,     0x1801}, /* sgmii_reg_an_4 */
    {0x10229100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10229000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10229300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x10220B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10220B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1022E004, 0,  7,  7,        0x1}, /* RG_FORCE_TX_BIT_INVERSE */
    {0x1022E400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_QP_SGMII_Slave-R07.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_setSgmiiCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251364, 0, 12,  8,        0x0}, /* HW_DA_PXP_TX_FIR_CN1 */
    {0x10251364, 0, 20, 16,        0x0}, /* HW_DA_PXP_TX_FIR_C1 */
    {0x10251364, 0, 26, 24,        0x0}, /* HW_DA_PXP_TX_FIR_C2 */
    {0x10251364, 0,  5,  0,        0xE}, /* HW_DA_PXP_TX_FIR_C0B */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251950, 0, 14, 12,        0x3}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x10251414, 0, 11,  9,        0x5}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x5}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x1}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,       0x00}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,       0x00}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,       0x03}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x1F}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,       0x07}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,       0x04}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,       0x00}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,       0x04}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x32000000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0B}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x2}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x102500C8, 0, 31, 24,        0x0}, /* RG_XPON_TX_RESERVED */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x2}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x0}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x3}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x18B0}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x3}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x3}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x8}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x8}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x0}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x7}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,       0x29}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x1}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x0}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x0}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x102518B8, 0,  0,  0,        0x1}, /* RG_FEOS_8B_MODE */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x0}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x1}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x1025199C, 0, 25, 25,        0x1}, /* RG_RETRAIN_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240A00, 0, 30, 30,        0x1}, /* RG_TBI_10B_MODE */
    {0x10248000, 0, 31,  0,       0x20}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10240000, 0, 31,  0,     0x1140}, /* sgmii_reg_an0 */
    {0x10240034, 0, 31,  0, 0x3112010B}, /* sgmii_reg_an_13 */
    {0x10240060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1024002C, 0, 19,  0,    0x186A0}, /* SGMII_LINK_TIMER */
    {0x10240010, 0, 31,  0,     0x1801}, /* sgmii_reg_an_4 */
    {0x10249100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10249000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10249300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_PXP_SGMII-R11.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_set1000basexCfg[] = {
    {0x1022E324, 0,  6,  4,        0x0}, /* RG_DA_QP_TX_FIR_CN1_FORCE */
    {0x1022E324, 0,  7,  7,        0x1}, /* RG_DA_QP_TX_FIR_CN1_SEL */
    {0x1022E328, 0, 20, 16,        0x0}, /* RG_DA_QP_TX_FIR_C1_FORCE */
    {0x1022E328, 0, 21, 21,        0x1}, /* RG_DA_QP_TX_FIR_C1_SEL */
    {0x1022E328, 0, 28, 24,        0x0}, /* RG_DA_QP_TX_FIR_C2_FORCE */
    {0x1022E328, 0, 29, 29,        0x1}, /* RG_DA_QP_TX_FIR_C2_SEL */
    {0x1022E32C, 0,  5,  0,        0xD}, /* RG_DA_QP_TX_FIR_C0B_FORCE */
    {0x1022E32C, 0,  6,  6,        0x1}, /* RG_DA_QP_TX_FIR_C0B_SEL */
    {0x1022EF30, 0,  1,  1,        0x1}, /* RG_QP_CHPEN */
    {0x1022A330, 0,  3,  2,        0x0}, /* RG_TPHY_SPEED */
    {0x1022E408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0x1022E408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0x1022E408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0x1022E408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0x1022E408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0x1022E408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0x1022E410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0x1022E408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0x1022E408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0x1022E408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0x1022E408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0x1022E408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0x1022E408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0x1022E408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0x1022E408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0x1022E410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0x1022E408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0x1022E208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0x1022E230, 0, 30,  0, 0x48000000}, /* RG_LCPLL_NCPO_VALUE */
    {0x1022E248, 0, 30,  0, 0x48000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1022E23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0x1022E414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0x1022E40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0x1022E410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0x1022E40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0x1022E414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0x1022EF40, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0x1022EF40, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0x1022EF3C, 0, 26, 25,        0x0}, /* RG_QP_PLL_PREDIV */
    {0x1022E100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0x1022EF28, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0x1022EF28, 0, 31, 16,      0x300}, /* RG_QP_TX_RESERVE */
    {0x1022E000, 0,  0,  0,        0x0}, /* RG_FVAL_TX_EIDLE_LP_OFF */
    {0x1022EF04, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0x1022EF04, 0, 10, 10,        0x0}, /* RG_QP_CDR_PD_EDGE_DIS */
    {0x1022EF0C, 0,  5,  4,        0x2}, /* RG_QP_CDR_LPF_RATIO */
    {0x1022EF08, 0, 26, 24,        0x6}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0x1022EF08, 0, 22, 20,        0x6}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0x1022EF14, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0x1022EF14, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0x1022EF18, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0x1022EF1C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0x1022EF1C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0x1022EF20, 0, 12,  6,       0x21}, /* RG_QP_CDR_PHYCK_DIV */
    {0x1022EF20, 0, 17, 16,        0x2}, /* RG_QP_CDR_PHYCK_SEL */
    {0x1022EF20, 0, 13, 13,        0x0}, /* RG_QP_CDR_PHYCK_RSTB */
    {0x1022EF1C, 0, 30, 30,        0x0}, /* RG_QP_CDR_PR_XFICK_EN */
    {0x1022EF18, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0x1022EF00, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0x1022E690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0x1022E690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0x1022E690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0x1022E614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0x1022E614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0x1022E6DC, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0x1022E630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0x1022E124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0x1022E648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0x1022E648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0x1022E690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0x1022E694, 0, 31,  0,  0x17D7840}, /* RG_QP_LINK_ERRO_CNT */
    {0x1022E63C, 0, 29, 10,       0x28}, /* RG_FREDET_CHK_CYCLE */
    {0x1022E640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0x1022E644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x1022E654, 0,  0,  0,        0x1}, /* RG_FORCE_FREQ_LOCK_SEL */
    {0x1022E654, 0,  1,  1,        0x1}, /* RG_FORCE_FREQ_LOCK */
    {0x10220000, 0, 31,  0,     0x1140}, /* sgmii_reg_an0 */
    {0x10220034, 0, 31,  0, 0x31120000}, /* sgmii_reg_an_13 */
    {0x10220060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1022002C, 0, 19,  0,    0x98968}, /* SGMII_LINK_TIMER */
    {0x10220A00, 0, 24, 24,        0x1}, /* RG_SGMII_SEND_AN_ERROR_EN */
    {0x10220A14, 0, 12, 12,        0x1}, /* RG_SGMII_SPD_FORCE_1000 */
    {0x10220A20, 0, 31,  0,      0x33F}, /* rg_hsgmii_mode_interrupt */
    {0x10220A20, 0, 31,  0,      0x113}, /* rg_hsgmii_mode_interrupt */
    {0x10220010, 0, 31,  0,      0x1a0}, /* sgmii_reg_an_4 */
    {0x10229100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10229000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10229300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x10220B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10220B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1022E004, 0,  7,  7,        0x1}, /* RG_FORCE_TX_BIT_INVERSE */
    {0x1022E400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_QP_1000BASE_X-R06.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_set1000basexCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251364, 0, 12,  8,        0x0}, /* HW_DA_PXP_TX_FIR_CN1 */
    {0x10251364, 0, 20, 16,        0x0}, /* HW_DA_PXP_TX_FIR_C1 */
    {0x10251364, 0, 26, 24,        0x0}, /* HW_DA_PXP_TX_FIR_C2 */
    {0x10251364, 0,  5,  0,        0xE}, /* HW_DA_PXP_TX_FIR_C0B */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x10251950, 0, 14, 12,        0x3}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x10251414, 0, 11,  9,        0x5}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x5}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x1}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,       0x00}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,       0x00}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,       0x03}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x1F}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,       0x07}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,       0x04}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,       0x00}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,       0x04}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x32000000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0B}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x2}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x102500C8, 0, 31, 24,        0x0}, /* RG_XPON_TX_RESERVED */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x2}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x0}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x3}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x18B0}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x3}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x3}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x8}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x8}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x0}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x7}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,       0x29}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x1}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x0}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x0}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x102518B8, 0,  0,  0,        0x1}, /* RG_FEOS_8B_MODE */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x0}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x1}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x1025199C, 0, 25, 25,        0x1}, /* RG_RETRAIN_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240A00, 0, 30, 30,        0x1}, /* RG_TBI_10B_MODE */
    {0x10240000, 0, 31,  0,     0x1140}, /* sgmii_reg_an0 */
    {0x10240034, 0, 31,  0, 0x31120000}, /* sgmii_reg_an_13 */
    {0x10240060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1024002C, 0, 19,  0,    0x98968}, /* SGMII_LINK_TIMER */
    {0x10240A00, 0, 24, 24,        0x1}, /* RG_SGMII_SEND_AN_ERROR_EN */
    {0x10240A14, 0, 12, 12,        0x1}, /* RG_SGMII_SPD_FORCE_1000 */
    {0x10240A20, 0, 31,  0,      0x33F}, /* rg_hsgmii_mode_interrupt */
    {0x10240A20, 0, 31,  0,      0x113}, /* rg_hsgmii_mode_interrupt */
    {0x10240010, 0, 31,  0,      0x1a0}, /* sgmii_reg_an_4 */
    {0x10249100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10249000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10249300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
    {0x1025120C, 0, 24, 24,        0x1}, /* RG_RO_TOGGLE */
    {0x1025120C, 0, 24, 24,        0x0}, /* RG_RO_TOGGLE */
}; /* ENB-SE1-250205-April_AN8858_Init_Script_PXP_1000BASE-X-R09.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_qp_set100basefxCfg[] = {
    {0x1022EF30, 0,  1,  1,        0x1}, /* RG_QP_CHPEN */
    {0x1022A330, 0,  3,  2,        0x0}, /* RG_TPHY_SPEED */
    {0x1022E408, 0,  1,  0,        0x1}, /* RG_DA_QP_PLL_BC_INTF */
    {0x1022E408, 0,  4,  2,        0x5}, /* RG_DA_QP_PLL_BPA_INTF */
    {0x1022E408, 0,  7,  6,        0x0}, /* RG_DA_QP_PLL_BPB_INTF */
    {0x1022E408, 0, 10,  8,        0x3}, /* RG_DA_QP_PLL_BR_INTF */
    {0x1022E408, 0, 29, 29,        0x1}, /* RG_DA_QP_PLL_RICO_SEL_INTF */
    {0x1022E408, 0, 13, 12,        0x0}, /* RG_DA_QP_PLL_FBKSEL_INTF */
    {0x1022E410, 0,  2,  2,        0x1}, /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    {0x1022E408, 0, 14, 14,        0x0}, /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    {0x1022E408, 0, 19, 16,        0x4}, /* RG_DA_QP_PLL_IR_INTF */
    {0x1022E408, 0, 21, 20,        0x1}, /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    {0x1022E408, 0, 25, 24,        0x1}, /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    {0x1022E408, 0, 26, 26,        0x0}, /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    {0x1022E408, 0, 22, 22,        0x1}, /* RG_DA_QP_PLL_PCK_SEL_INTF */
    {0x1022E408, 0, 27, 27,        0x0}, /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    {0x1022E408, 0, 28, 28,        0x0}, /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    {0x1022E410, 0,  4,  3,        0x0}, /* RG_DA_QP_PLL_SDM_HREN_INTF */
    {0x1022E408, 0, 30, 30,        0x0}, /* RG_DA_QP_PLL_SDM_IFM_INTF */
    {0x1022E208, 0, 17, 16,        0x1}, /* RG_NCPO_ANA_MSB */
    {0x1022E230, 0, 30,  0, 0x48000000}, /* RG_LCPLL_NCPO_VALUE */
    {0x1022E248, 0, 30,  0, 0x48000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1022E23C, 0, 24, 24,        0x0}, /* RG_LCPLL_NCPO_CHG */
    {0x1022E414, 0,  8,  8,        0x0}, /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    {0x1022E40C, 0, 15,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    {0x1022E410, 0,  1,  0,        0x0}, /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    {0x1022E40C, 0, 31, 16,        0x0}, /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    {0x1022E414, 0,  9,  9,        0x0}, /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    {0x1022EF40, 0,  3,  3,        0x1}, /* RG_QP_PLL_SSC_PHASE_INI */
    {0x1022EF40, 0,  4,  4,        0x1}, /* RG_QP_PLL_SSC_TRI_EN */
    {0x1022EF3C, 0, 26, 25,        0x0}, /* RG_QP_PLL_PREDIV */
    {0x1022E100, 0, 12, 12,        0x0}, /* RG_QP_EQ_RX500M_CK_SEL */
    {0x1022EF28, 0,  0,  0,        0x0}, /* RG_QP_TX_MODE_16B_EN */
    {0x1022EF28, 0, 31, 16,      0x300}, /* RG_QP_TX_RESERVE */
    {0x1022E000, 0,  0,  0,        0x0}, /* RG_FVAL_TX_EIDLE_LP_OFF */
    {0x1022EF04, 0, 11, 11,        0x1}, /* RG_QP_CDR_PD_10B_EN */
    {0x1022EF04, 0, 10, 10,        0x0}, /* RG_QP_CDR_PD_EDGE_DIS */
    {0x1022EF0C, 0,  5,  4,        0x2}, /* RG_QP_CDR_LPF_RATIO */
    {0x1022EF08, 0, 26, 24,        0x6}, /* RG_QP_CDR_LPF_KP_GAIN */
    {0x1022EF08, 0, 22, 20,        0x6}, /* RG_QP_CDR_LPF_KI_GAIN */
    {0x1022EF14, 0, 28, 25,        0x1}, /* RG_QP_CDR_PR_BETA_SEL */
    {0x1022EF14, 0, 31, 29,        0x6}, /* RG_QP_CDR_PR_BUF_IN_SR */
    {0x1022EF18, 0, 12,  8,        0xC}, /* RG_QP_CDR_PR_DAC_BAND */
    {0x1022EF1C, 0,  5,  0,       0x19}, /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    {0x1022EF1C, 0,  6,  6,        0x0}, /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    {0x1022EF20, 0, 12,  6,        0xA}, /* RG_QP_CDR_PHYCK_DIV */
    {0x1022EF20, 0, 17, 16,        0x0}, /* RG_QP_CDR_PHYCK_SEL */
    {0x1022EF20, 0, 13, 13,        0x1}, /* RG_QP_CDR_PHYCK_RSTB */
    {0x1022EF1C, 0, 30, 30,        0x1}, /* RG_QP_CDR_PR_XFICK_EN */
    {0x1022EF18, 0, 26, 24,        0x4}, /* RG_QP_CDR_PR_KBAND_DIV */
    {0x1022EF00, 0, 18, 18,        0x1}, /* RG_QP_LFPS_LPF_AUX */
    {0x1022EF00, 0, 17, 16,        0x2}, /* RG_QP_SIGDET_HF */
    {0x1022EF00, 0, 15, 14,        0x3}, /* RG_QP_SIGDET_LPF */
    {0x1022E690, 0, 23, 23,        0x1}, /* RG_LINK_ERRO_EN */
    {0x1022E690, 0, 24, 24,        0x0}, /* RG_LINK_NE_EN */
    {0x1022E690, 0, 26, 26,        0x1}, /* RG_QP_EQ_RETRAIN_ONLY_EN */
    {0x1022E614, 0,  7,  0,       0x6F}, /* RG_QP_RX_PI_CAL_EN_H_DLY */
    {0x1022E614, 0, 13,  8,       0x3F}, /* RG_QP_RX_SAOSC_EN_H_DLY */
    {0x1022E6DC, 0, 12,  0,      0x150}, /* RG_QP_EQ_EN_DLY */
    {0x1022E630, 0, 28, 16,      0x150}, /* RG_QP_RX_EQ_EN_H_DLY */
    {0x1022E124, 0,  2,  0,        0x1}, /* RG_QP_EQ_LEQOSC_DLYCNT */
    {0x1022E648, 0, 27, 16,      0x200}, /* RG_DA_QP_SAOSC_DONE_TIME */
    {0x1022E648, 0, 14,  0,      0xFFF}, /* RG_DA_QP_LEQOS_EN_TIME */
    {0x1022E690, 0,  0,  0,        0x1}, /* RG_QP_SIG_LINKDOWN_SEL */
    {0x1022E694, 0, 31,  0,  0x17D7840}, /* RG_QP_LINK_ERRO_CNT */
    {0x1022E6F0, 0, 31, 31,        0x0}, /* RG_QP_100FX_SIG_SEL */
    {0x1022E63C, 0, 29, 10,       0x28}, /* RG_FREDET_CHK_CYCLE */
    {0x1022E640, 0, 19,  0,       0x64}, /* RG_FREDET_GOLDEN_CYCLE */
    {0x1022E644, 0, 19,  0,     0x2710}, /* RG_FREDET_TOLERATE_CYCLE */
    {0x1022E654, 0,  0,  0,        0x1}, /* RG_FORCE_FREQ_LOCK_SEL */
    {0x1022E654, 0,  1,  1,        0x1}, /* RG_FORCE_FREQ_LOCK */
    {0x10228154, 0, 31,  0,        0x0}, /* xsgmii_multi_sgmii_fpga_mode_control */
    {0x10228000, 0, 31,  0, 0x13000020}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10220000, 0, 31,  0,      0x140}, /* sgmii_reg_an0 */
    {0x10220034, 0, 31,  0, 0x3112010B}, /* sgmii_reg_an_13 */
    {0x10220060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1022002C, 0, 19,  0,       0x50}, /* SGMII_LINK_TIMER */
    {0x10220010, 0, 31,  0,     0x1801}, /* sgmii_reg_an_4 */
    {0x10229100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10229000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10229300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x1022B404, 0,  7,  7,        0x0}, /* TX_ODD_NIBBLE_PREAM_DEAL_EN */
    {0x1022B404, 0,  6,  6,        0x0}, /* RX_ODD_NIBBLE_PREAM_DEAL_EN */
    {0x1022B404, 0,  5,  5,        0x0}, /* TEFIFO_RXER_MASK */
    {0x1022B404, 0,  4,  4,        0x0}, /* REFIFO_RXER_MASK */
    {0x1022B404, 0,  3,  3,        0x1}, /* TEFIFO_CLK_EN */
    {0x1022B404, 0,  2,  2,        0x1}, /* REFIFO_CLK_EN */
    {0x1022B404, 0,  1,  1,        0x1}, /* TEFIFO_ENABLE */
    {0x1022B404, 0,  0,  0,        0x1}, /* REFIFO_ENABLE */
    {0x10228104, 0,  0,  0,        0x1}, /* RG_TBI_20_TX_REVERSE_PMA */
    {0x10228104, 0,  1,  1,        0x1}, /* RG_TBI_TX_REVERSE_10B_PMA */
    {0x10228104, 0,  2,  2,        0x1}, /* RG_TBI_20_RX_REVERSE_PMA */
    {0x10228104, 0,  3,  3,        0x1}, /* RG_TBI_10_RX_REVERSE_PMA */
    {0x10228104, 0,  4,  4,        0x0}, /* RG_TBI_20_RX_REVERSE_ALL */
    {0x10228104, 0,  5,  5,        0x0}, /* RG_TBI_20_TX_REVERSE_ALL */
    {0x10220B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10220B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1022E004, 0,  7,  7,        0x1}, /* RG_FORCE_TX_BIT_INVERSE */
    {0x1022E400, 0,  0,  0,        0x1}, /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250321-Dora_AN8856_Init_Script_QP_100BASE_FX-R07.xlsx */

const HAL_CORAL_SERDES_CFG_T _hal_coral_serdes_pxp_set100basefxCfg[] = {
    {0x10250114, 0, 25, 24,        0x1}, /* RG_XPON_RX_FE_50OHMS_SEL */
    {0x1025177C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    {0x1025177C, 0,  2,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    {0x10250000, 0, 28, 24,       0x10}, /* RG_XPON_CMN_TRIM */
    {0x10251958, 0, 18, 18,        0x0}, /* FX_EFIFO_SW_RST_N */
    {0x10251990, 0,  0,  0,        0x1}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  5,  5,        0x1}, /* RG_SW_REF_RST_N */
    {0x10251460, 0,  1,  1,        0x1}, /* RG_SW_RX_RST_N */
    {0x10251460, 0,  3,  3,        0x1}, /* RG_SW_PMA_RST_N */
    {0x10251460, 0,  6,  6,        0x1}, /* RG_SW_TX_FIFO_RST_N */
    {0x10251460, 0,  0,  0,        0x1}, /* RG_SW_RX_FIFO_RST_N */
    {0x102500A0, 0, 15,  8,        0x1}, /* RG_XPON_PLL_CMN_RESERVE0 */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x10251950, 0, 14, 12,        0x6}, /* RG_FORCE_XSI_MODE */
    {0x10251950, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_XSI_MODE */
    {0x102518F8, 0,  3,  3,        0x1}, /* RG_FORCE_P0_SGMII_MODE_SEL */
    {0x102518F8, 0,  1,  0,        0x1}, /* RG_FORCE_P0_SGMII_MODE */
    {0x10251414, 0, 11,  9,        0x5}, /* RG_XFI_RX_MODE */
    {0x10251414, 0,  5,  3,        0x5}, /* RG_XFI_TX_MODE */
    {0x1025011C, 0,  0,  0,        0x1}, /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    {0x102500A8, 0, 17, 16,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    {0x102500A8, 0,  8,  8,        0x1}, /* RG_XPON_PLL_LDO_CKDRV_EN */
    {0x10250000, 0,  0,  0,        0x1}, /* RG_XPON_CMN_EN */
    {0x10250004, 0,  8,  8,        0x0}, /* RG_XPON_CMN_BYPASS_LPF */
    {0x10250004, 0, 16, 16,        0x0}, /* RG_XPON_CMN_BGBYPASS_LPF */
    {0x10250084, 0, 25, 24,        0x1}, /* RG_XPON_TXPLL_LDO_VCO_OUT */
    {0x10250084, 0, 17, 16,        0x1}, /* RG_XPON_TXPLL_LDO_OUT */
    {0x10250088, 0, 25, 24,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP */
    {0x10250088, 0, 16, 16,        0x0}, /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    {0x10250088, 0, 10,  8,        0x0}, /* RG_XPON_TXPLL_VTP */
    {0x10250088, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_VTP_EN */
    {0x10250064, 0, 24, 24,        0x1}, /* RG_XPON_TXPLL_PLL_RSTB */
    {0x10250064, 0, 18, 16,        0x4}, /* RG_XPON_TXPLL_RST_DLY */
    {0x10250064, 0,  0,  0,        0x0}, /* RG_XPON_TXPLL_REFIN_INTERNAL */
    {0x10250064, 0,  9,  8,        0x0}, /* RG_XPON_TXPLL_REFIN_DIV */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x0}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250068, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SDM_DI_EN */
    {0x10250068, 0,  9,  8,       0x00}, /* RG_XPON_TXPLL_SDM_DI_LS */
    {0x10250068, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_IFM */
    {0x10250068, 0, 25, 24,       0x00}, /* RG_XPON_TXPLL_SDM_MODE */
    {0x1025006C, 0,  1,  0,       0x03}, /* RG_XPON_TXPLL_SDM_ORD */
    {0x1025006C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SDM_HREN */
    {0x1025006C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SDM_OUT */
    {0x10250080, 0, 31, 16,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA */
    {0x10250080, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_DELTA1 */
    {0x1025007C, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_SSC_TRI_EN */
    {0x1025007C, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_SSC_PHASE_INI */
    {0x1025007C, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_SSC_EN */
    {0x10250084, 0, 15,  0,       0x00}, /* RG_XPON_TXPLL_SSC_PERIOD */
    {0x10250098, 0,  0,  0,       0x00}, /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    {0x10250098, 0,  8,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST_EN */
    {0x10250050, 0,  5,  0,       0x24}, /* RG_XPON_TXPLL_CHP_IBIAS */
    {0x10250050, 0, 13,  8,       0x00}, /* RG_XPON_TXPLL_CHP_IOFST */
    {0x10250050, 0, 20, 16,       0x0A}, /* RG_XPON_TXPLL_LPF_BR */
    {0x10250050, 0, 28, 24,       0x1F}, /* RG_XPON_TXPLL_LPF_BC */
    {0x10250054, 0,  4,  0,       0x18}, /* RG_XPON_TXPLL_LPF_BP */
    {0x10250054, 0, 12,  8,       0x16}, /* RG_XPON_TXPLL_LPF_BWR */
    {0x10250054, 0, 20, 16,       0x18}, /* RG_XPON_TXPLL_LPF_BWC */
    {0x10250074, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_VCO_CFIX */
    {0x10250078, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    {0x10250078, 0, 10,  8,       0x07}, /* RG_XPON_TXPLL_VCO_SCAPWR */
    {0x10250078, 0, 26, 24,       0x04}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    {0x10250078, 0, 29, 27,       0x00}, /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    {0x10250078, 0, 18, 16,       0x04}, /* RG_XPON_TXPLL_VCO_TCLVAR */
    {0x10251794, 0, 24, 24,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    {0x10251798, 0, 30,  0, 0x32000000}, /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    {0x10251048, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    {0x1025104C, 0, 30,  0, 0x64000000}, /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    {0x10250058, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_KBAND_KFC */
    {0x10250058, 0, 25, 24,       0x03}, /* RG_XPON_TXPLL_KBAND_KF */
    {0x1025005C, 0,  1,  0,       0x01}, /* RG_XPON_TXPLL_KBAND_KS */
    {0x10250058, 0, 10,  8,       0x02}, /* RG_XPON_TXPLL_KBAND_DIV */
    {0x10250058, 0,  7,  0,       0xE4}, /* RG_XPON_TXPLL_KBAND_CODE */
    {0x10250054, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_KBAND_OPTION */
    {0x10250094, 0, 24, 24,       0x00}, /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    {0x10251858, 0,  8,  8,       0x01}, /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x10251858, 0,  0,  0,       0x00}, /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    {0x1025005C, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    {0x1025005C, 0,  8,  8,       0x01}, /* RG_XPON_TXPLL_POSTDIV_EN */
    {0x10250074, 0, 17, 16,       0x00}, /* RG_XPON_TXPLL_VCODIV */
    {0x10250098, 0, 16, 16,       0x00}, /* RG_XPON_TXPLL_TCL_BYP_EN */
    {0x10250094, 0,  4,  0,       0x0F}, /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    {0x10250070, 0,  2,  0,       0x03}, /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    {0x10250070, 0, 12,  8,       0x0B}, /* RG_XPON_TXPLL_TCL_AMP_VREF */
    {0x10250074, 0, 10,  8,       0x00}, /* RG_XPON_TXPLL_TCL_LPF_BW */
    {0x10250074, 0,  0,  0,       0x01}, /* RG_XPON_TXPLL_TCL_LPF_EN */
    {0x1025006C, 0, 24, 24,       0x01}, /* RG_XPON_TXPLL_TCL_AMP_EN */
    {0x102500B4, 0,  1,  0,        0x1}, /* RG_XPON_TX_SER_LOADSEL */
    {0x102500C4, 0,  0,  0,        0x1}, /* RG_XPON_TX_CKLDO_EN */
    {0x102500C4, 0, 10,  8,        0x3}, /* RG_XPON_TX_CKLDO_LVR */
    {0x102500C4, 0, 16, 16,        0x0}, /* RG_XPON_TX_RXDET_METHOD */
    {0x102500C4, 0, 24, 24,        0x1}, /* RG_XPON_TX_DMEDGEGEN_EN */
    {0x102500C8, 0,  8,  8,        0x1}, /* RG_XPON_TX_TERMCAL_SELPN */
    {0x102500C8, 0, 31, 24,        0x0}, /* RG_XPON_TX_RESERVED */
    {0x10251874, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    {0x10251874, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    {0x1025177C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    {0x1025177C, 0, 19, 16,        0x2}, /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    {0x10251784, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    {0x10251784, 0,  1,  0,        0x1}, /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    {0x10251260, 0,  0,  0,        0x1}, /* TX_TOP_RST_B */
    {0x102518EC, 0,  0,  0,        0x1}, /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    {0x102518EC, 0,  4,  4,        0x1}, /* RG_FORCE_PMA_TX_FIFO_WR */
    {0x1025199C, 0, 26, 26,        0x0}, /* SPEED_SEL_1 */
    {0x10251320, 0, 24, 24,        0x0}, /* RG_DISB_BLWC_OFFSET */
    {0x1025148C, 0,  0,  0,        0x0}, /* RG_DISB_LEQ */
    {0x102500DC, 0,  8,  8,        0x0}, /* RG_XPON_CDR_PD_EDGE_DIS */
    {0x10250118, 0,  8,  8,        0x1}, /* RG_XPON_RX_FE_VB_EQ1_EN */
    {0x10250118, 0, 16, 16,        0x1}, /* RG_XPON_RX_FE_VB_EQ2_EN */
    {0x10250118, 0, 24, 24,        0x1}, /* RG_XPON_RX_FE_VB_EQ3_EN */
    {0x1025188C, 0,  1,  0,        0x3}, /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    {0x1025188C, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    {0x102500D4, 0, 15,  0,     0x18B0}, /* RG_XPON_RX_REV_0 */
    {0x102500D4, 0, 19, 18,        0x0}, /* RG_XPON_RX_REV_1 */
    {0x10250120, 0, 17,  8,      0x3FF}, /* RG_XPON_RX_OSCAL_FORCE */
    {0x10250114, 0,  9,  8,        0x2}, /* RG_XPON_RX_SIGDET_PEAK */
    {0x10250114, 0, 20, 16,        0x2}, /* RG_XPON_RX_SIGDET_VTH_SEL */
    {0x10250110, 0, 25, 24,        0x3}, /* RG_XPON_RX_SIGDET_LPF_CTRL */
    {0x10251840, 0,  0,  0,        0x0}, /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    {0x10251840, 0,  8,  8,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    {0x1025176C, 0, 17, 16,        0x3}, /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    {0x1025176C, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    {0x10251814, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    {0x10251814, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    {0x102500E8, 0,  1,  0,        0x3}, /* RG_XPON_CDR_LPF_RATIO */
    {0x102500F8, 0,  6,  0,        0x8}, /* RG_XPON_CDR_PR_BETA_DAC */
    {0x102500FC, 0, 20, 16,        0x8}, /* RG_XPON_CDR_PR_DAC_BAND */
    {0x1025010C, 0, 19, 19,        0x0}, /* RG_XPON_CDR_PR_CAP_EN */
    {0x1025010C, 0, 18, 16,        0x7}, /* RG_XPON_CDR_PR_BUF_IN_SR */
    {0x102500D8, 0,  7,  0,       0x15}, /* RG_XPON_RX_PHYCK_DIV */
    {0x102500D8, 0,  9,  8,        0x1}, /* RG_XPON_RX_PHYCK_SEL */
    {0x102500D8, 0, 16, 16,        0x1}, /* RG_XPON_RX_PHYCK_RSTB */
    {0x1025010C, 0,  2,  2,        0x1}, /* RG_XPON_CDR_PR_XFICK_EN */
    {0x102500CC, 0,  8,  8,        0x0}, /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    {0x102500CC, 0,  0,  0,        0x0}, /* RG_XPON_RX_BUSBIT_SEL */
    {0x102500CC, 0, 24, 24,        0x1}, /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    {0x102500CC, 0, 16, 16,        0x0}, /* RG_XPON_RX_PHY_CK_SEL */
    {0x102500D4, 0, 30, 28,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 26, 24,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 22, 20,        0x5}, /* RG_XPON_RX_REV_1 */
    {0x102500D4, 0, 17, 17,        0x1}, /* RG_XPON_RX_REV_1 */
    {0x102518B8, 0,  0,  0,        0x1}, /* RG_FEOS_8B_MODE */
    {0x1025148C, 0, 15,  8,        0xF}, /* RG_L2D_TRIG_EQ_EN_TIME */
    {0x10251090, 0, 31, 16,      0xA00}, /* RG_RX_PICAL_END */
    {0x10251090, 0, 15,  0,        0x2}, /* RG_RX_PICAL_START */
    {0x1025109C, 0, 31, 16,      0x2E0}, /* RG_RX_SDCAL_END */
    {0x1025109C, 0, 15,  0,        0x2}, /* RG_RX_SDCAL_START */
    {0x10251094, 0, 31, 16,      0x2E0}, /* RG_RX_PDOS_END */
    {0x10251094, 0, 15,  0,        0x2}, /* RG_RX_PDOS_START */
    {0x10251098, 0, 31, 16,      0x2E0}, /* RG_RX_FEOS_END */
    {0x10251098, 0, 15,  0,        0x2}, /* RG_RX_FEOS_START */
    {0x10251100, 0, 31, 16,       0x5A}, /* RG_RX_RDY  */
    {0x10251100, 0, 15,  0,        0x5}, /* RG_RX_BLWC_RDY_EN */
    {0x10251148, 0, 29, 23,       0x40}, /* RG_EQ_BLWC_CNT_BOT_LIM */
    {0x10251148, 0, 22, 16,       0x3F}, /* RG_EQ_BLWC_CNT_TOP_LIM */
    {0x10251148, 0, 11,  8,        0xA}, /* RG_EQ_BLWC_GAIN */
    {0x10251170, 0, 25, 24,        0x1}, /* RG_KBAND_KFC */
    {0x10251170, 0, 18,  8,       0xA5}, /* RG_FPKDIV  */
    {0x10251170, 0,  2,  0,        0x4}, /* RG_KBAND_PREDIV */
    {0x10251178, 0, 18, 16,        0x0}, /* RG_CK_RATE */
    {0x10251374, 0,  1,  0,        0x0}, /* RG_XPON_RX_RATE_CTRL */
    {0x10251580, 0,  1,  0,        0x1}, /* RG_PON_TX_RATE_CTRL */
    {0x1025199C, 0, 24, 24,        0x1}, /* RG_LCK2REF_COND */
    {0x10251994, 0, 15, 15,        0x0}, /* RG_XPON_SIG_OUT_SEL */
    {0x102519A0, 0,  4,  4,        0x0}, /* RG_PXP_100FX_SIG_SEL */
    {0x102519AC, 0,  0,  0,        0x1}, /* RG_PXP_100FX_DEC_ERR_EN */
    {0x1025133C, 0,  0,  0,        0x0}, /* RG_DISB_FBCK_LOCK */
    {0x10251330, 0,  0,  0,        0x1}, /* RG_FORCE_FBCK_LOCK */
    {0x10240A00, 0, 30, 30,        0x1}, /* RG_TBI_10B_MODE */
    {0x10248154, 0, 31,  0,        0x0}, /* xsgmii_multi_sgmii_fpga_mode_control */
    {0x10248000, 0, 31,  0, 0x10000020}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10248000, 0, 31,  0,       0x20}, /* xsgmii_msg_tx_ctrl_0 */
    {0x10240000, 0, 31,  0,      0x140}, /* sgmii_reg_an0 */
    {0x10240034, 0, 31,  0, 0x3112010B}, /* sgmii_reg_an_13 */
    {0x10240060, 0,  0,  0,        0x1}, /* RG_FORCE_AN_DONE */
    {0x1024002C, 0, 19,  0,       0x50}, /* SGMII_LINK_TIMER */
    {0x10240010, 0, 31,  0,     0x1801}, /* sgmii_reg_an_4 */
    {0x10249100, 0, 31,  0, 0x90000000}, /* rate_adp_p0_ctrl_0 */
    {0x10249000, 0, 31,  0,  0xC000C00}, /* rg_rate_adapt_ctrl_0 */
    {0x10249300, 0,  0,  0,        0x1}, /* RG_P0_RA_AN_EN */
    {0x10240B88, 0, 31,  0,   0x200008}, /* pcs_dec_erro */
    {0x10240B84, 0,  0,  0,        0x0}, /* RG_DET_PAD_EN */
    {0x1024B404, 0,  7,  7,        0x0}, /* TX_ODD_NIBBLE_PREAM_DEAL_EN */
    {0x1024B404, 0,  6,  6,        0x0}, /* RX_ODD_NIBBLE_PREAM_DEAL_EN */
    {0x1024B404, 0,  5,  5,        0x0}, /* TEFIFO_RXER_MASK */
    {0x1024B404, 0,  4,  4,        0x0}, /* REFIFO_RXER_MASK */
    {0x1024B404, 0,  3,  3,        0x1}, /* TEFIFO_CLK_EN */
    {0x1024B404, 0,  2,  2,        0x1}, /* REFIFO_CLK_EN */
    {0x1024B404, 0,  1,  1,        0x1}, /* TEFIFO_ENABLE */
    {0x1024B404, 0,  0,  0,        0x1}, /* REFIFO_ENABLE */
    {0x10248104, 0,  0,  0,        0x1}, /* RG_TBI_20_TX_REVERSE_PMA */
    {0x10248104, 0,  1,  1,        0x1}, /* RG_TBI_TX_REVERSE_10B_PMA */
    {0x10248104, 0,  2,  2,        0x1}, /* RG_TBI_20_RX_REVERSE_PMA */
    {0x10248104, 0,  3,  3,        0x1}, /* RG_TBI_10_RX_REVERSE_PMA */
    {0x10248104, 0,  4,  4,        0x0}, /* RG_TBI_20_RX_REVERSE_ALL */
    {0x10248104, 0,  5,  5,        0x0}, /* RG_TBI_20_TX_REVERSE_ALL */
    {0x10251854, 0, 24, 24,        0x1}, /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    {0x10251854, 0, 16, 16,        0x1}, /* RG_FORCE_DA_PXP_TXPLL_EN */
    {0x10250060, 0,  8,  8,        0x1}, /* RG_XPON_TXPLL_PHY_CK2_EN */
    {0x10250060, 0,  0,  0,        0x1}, /* RG_XPON_TXPLL_PHY_CK1_EN */
    {0x10251460, 0, 17, 17,        0x1}, /* RG_SW_XFI_RXMAC_RST_N */
    {0x10251460, 0, 16, 16,        0x1}, /* RG_SW_XFI_TXMAC_RST_N */
    {0x10251460, 0, 15, 15,        0x1}, /* RG_SW_PONOLT_RXMAC_RST_N */
    {0x10251460, 0, 13, 13,        0x1}, /* RG_SW_PONOLT_TXMAC_RST_N */
    {0x10251460, 0, 11, 11,        0x1}, /* RG_SW_HSG_RXPCS_RST_N */
    {0x10251460, 0, 10, 10,        0x1}, /* RG_SW_HSG_TXPCS_RST_N */
    {0x10251460, 0,  8,  8,        0x1}, /* RG_SW_XFI_RXPCS_RST_N */
    {0x10251460, 0,  7,  7,        0x1}, /* RG_SW_XFI_TXPCS_RST_N */
    {0x10251460, 0,  4,  4,        0x1}, /* RG_SW_ALLPCS_RST_N */
    {0x10251990, 0,  0,  0,        0x0}, /* RG_SIGDET_EN_SEL */
    {0x10251460, 0,  2,  2,        0x1}, /* RG_SW_TX_RST_N */
    {0x10251958, 0, 18, 18,        0x1}, /* FX_EFIFO_SW_RST_N */
    {0x1025120C, 0, 24, 24,        0x1}, /* RG_RO_TOGGLE */
    {0x1025120C, 0, 24, 24,        0x0}, /* RG_RO_TOGGLE */
}; /* ENB-SE1-250321-Dora_AN8856_Init_Script_PXP_100BASE_FX-R06.xlsx */

/* clang-format off */
const HAL_CORAL_SERDES_CFG_INFO_T
_hal_coral_serdes_qp_cfg_info[] =
{
    {
        HAL_CORAL_SERDES_MODE_DEFAULT_VALUE,
        (sizeof(_hal_coral_serdes_qp_defaultValue) / sizeof(_hal_coral_serdes_qp_defaultValue[0])),
        &_hal_coral_serdes_qp_defaultValue[0]
    },
    {
        HAL_CORAL_SERDES_MODE_5GBASETR,
        (sizeof(_hal_coral_serdes_qp_set5gbaserCfg) / sizeof(_hal_coral_serdes_qp_set5gbaserCfg[0])),
        &_hal_coral_serdes_qp_set5gbaserCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_HSGMII,
        (sizeof(_hal_coral_serdes_qp_setHsgmiiCfg) / sizeof(_hal_coral_serdes_qp_setHsgmiiCfg[0])),
        &_hal_coral_serdes_qp_setHsgmiiCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_SGMII_AN,
        (sizeof(_hal_coral_serdes_qp_setSgmiiCfg) / sizeof(_hal_coral_serdes_qp_setSgmiiCfg[0])),
        &_hal_coral_serdes_qp_setSgmiiCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_1000BASEX,
        (sizeof(_hal_coral_serdes_qp_set1000basexCfg) / sizeof(_hal_coral_serdes_qp_set1000basexCfg[0])),
        &_hal_coral_serdes_qp_set1000basexCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_100BASEFX,
        (sizeof(_hal_coral_serdes_qp_set100basefxCfg) / sizeof(_hal_coral_serdes_qp_set100basefxCfg[0])),
        &_hal_coral_serdes_qp_set100basefxCfg[0]
    },
};

const HAL_CORAL_SERDES_CFG_INFO_T
_hal_coral_serdes_pxp_cfg_info[] =
{
    {
        HAL_CORAL_SERDES_MODE_DEFAULT_VALUE,
        (sizeof(_hal_coral_serdes_pxp_defaultValue) / sizeof(_hal_coral_serdes_pxp_defaultValue[0])),
        &_hal_coral_serdes_pxp_defaultValue[0]
    },
    {
        HAL_CORAL_SERDES_MODE_USXGMII,
        (sizeof(_hal_coral_serdes_pxp_setUsxgmiiCfg) / sizeof(_hal_coral_serdes_pxp_setUsxgmiiCfg[0])),
        &_hal_coral_serdes_pxp_setUsxgmiiCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_5GBASETR,
        (sizeof(_hal_coral_serdes_pxp_set5gbaserCfg) / sizeof(_hal_coral_serdes_pxp_set5gbaserCfg[0])),
        &_hal_coral_serdes_pxp_set5gbaserCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_HSGMII,
        (sizeof(_hal_coral_serdes_pxp_setHsgmiiCfg) / sizeof(_hal_coral_serdes_pxp_setHsgmiiCfg[0])),
        &_hal_coral_serdes_pxp_setHsgmiiCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_SGMII_AN,
        (sizeof(_hal_coral_serdes_pxp_setSgmiiCfg) / sizeof(_hal_coral_serdes_pxp_setSgmiiCfg[0])),
        &_hal_coral_serdes_pxp_setSgmiiCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_1000BASEX,
        (sizeof(_hal_coral_serdes_pxp_set1000basexCfg) / sizeof(_hal_coral_serdes_pxp_set1000basexCfg[0])),
        &_hal_coral_serdes_pxp_set1000basexCfg[0]
    },
    {
        HAL_CORAL_SERDES_MODE_100BASEFX,
        (sizeof(_hal_coral_serdes_pxp_set100basefxCfg) / sizeof(_hal_coral_serdes_pxp_set100basefxCfg[0])),
        &_hal_coral_serdes_pxp_set100basefxCfg[0]
    },
};
/* clang-format on */

const UI32_T _hal_coral_serdes_qp_cfg_info_size =
    (sizeof(_hal_coral_serdes_qp_cfg_info) / sizeof(_hal_coral_serdes_qp_cfg_info[0]));
const UI32_T _hal_coral_serdes_pxp_cfg_info_size =
    (sizeof(_hal_coral_serdes_pxp_cfg_info) / sizeof(_hal_coral_serdes_pxp_cfg_info[0]));

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_qp_dbg_xgmii_cfg[] = {
    {1, 0x902C, 0,  1, 1, 0x1,           "RG_XGMII_PKT_RD_TOGGLE"},
    {1, 0x902C, 5,  1, 1, 0x0,           "RG_XGMII_PKT_RD_TOGGLE"},
    {0, 0x90B0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_0_STS(TX_FB)"},
    {0, 0x90B4, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_1_STS(TX_FD)"},
    {0, 0x90B8, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_2_STS(TX_ERR)"},
    {0, 0x90BC, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_3_STS(RX_FB)"},
    {0, 0x90C0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_4_STS(RX_FD)"},
    {0, 0x90C4, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_5_STS(RX_ERR)"},
};

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_qp_dbg_mii_cfg[] = {
    {1, 0x9124, 0, 31, 0, 0xAA,           "rate_adp_dbg_p0_0"},
    {1, 0x9124, 5, 31, 0, 0x00,           "rate_adp_dbg_p0_0"},
    {0, 0x9128, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x912C, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9148, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9130, 0, 15, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9134, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9138, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x914C, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9150, 0, 15, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9154, 0, 31, 0, 0xAA,           "rate_adp_dbg_p1_0"},
    {1, 0x9154, 5, 31, 0, 0x00,           "rate_adp_dbg_p1_0"},
    {0, 0x9158, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x915C, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9178, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9160, 0, 15, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9164, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9168, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x917C, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9180, 0, 15, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9184, 0, 31, 0, 0xAA,           "rate_adp_dbg_p2_0"},
    {1, 0x9184, 5, 31, 0, 0x00,           "rate_adp_dbg_p2_0"},
    {0, 0x9188, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x918C, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9208, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9190, 0, 15, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9194, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9198, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x920C, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9210, 0, 15, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9214, 0, 31, 0, 0xAA,           "rate_adp_dbg_p3_0"},
    {1, 0x9214, 5, 31, 0, 0x00,           "rate_adp_dbg_p3_0"},
    {0, 0x9218, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x921C, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9238, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9220, 0, 15, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9224, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9228, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x923C, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9240, 0, 15, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_ER_CNT"},
};

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_qp_dbg_clear_cfg[] = {
    {1, 0x9124, 0, 31, 0, 0x55,     "rate_adp_dbg_p0_0"},
    {1, 0x9124, 5, 31, 0, 0x00,     "rate_adp_dbg_p0_0"},
    {1, 0x9154, 0, 31, 0, 0x55,     "rate_adp_dbg_p1_0"},
    {1, 0x9154, 5, 31, 0, 0x00,     "rate_adp_dbg_p1_0"},
    {1, 0x9184, 0, 31, 0, 0x55,     "rate_adp_dbg_p2_0"},
    {1, 0x9184, 5, 31, 0, 0x00,     "rate_adp_dbg_p2_0"},
    {1, 0x9214, 0, 31, 0, 0x55,     "rate_adp_dbg_p3_0"},
    {1, 0x9214, 5, 31, 0, 0x00,     "rate_adp_dbg_p3_0"},
    {1, 0x902c, 0,  2, 2,  0x0, "rg_rate_adapt_ctrl_11"},
    {1, 0x902c, 5,  2, 2,  0x1, "rg_rate_adapt_ctrl_11"},
};

/* clang-format off */
const HAL_CORAL_SERDES_DBG_CFG_INFO_T
_hal_coral_serdes_qp_dbg_cfg_info[] =
{
    {
        HAL_PHY_SERDES_INTERFACE_XGMII,
        (sizeof(_hal_coral_serdes_qp_dbg_xgmii_cfg) / sizeof(_hal_coral_serdes_qp_dbg_xgmii_cfg[0])),
        &_hal_coral_serdes_qp_dbg_xgmii_cfg[0]
    },
    {
        HAL_PHY_SERDES_INTERFACE_MII,
        (sizeof(_hal_coral_serdes_qp_dbg_mii_cfg) / sizeof(_hal_coral_serdes_qp_dbg_mii_cfg[0])),
        &_hal_coral_serdes_qp_dbg_mii_cfg[0]
    },
    {
        HAL_PHY_SERDES_READ_CLEAR,
        (sizeof(_hal_coral_serdes_qp_dbg_clear_cfg) / sizeof(_hal_coral_serdes_qp_dbg_clear_cfg[0])),
        &_hal_coral_serdes_qp_dbg_clear_cfg[0]
    }
};
/* clang-format on */

const UI32_T _hal_coral_serdes_qp_dbg_cfg_info_size =
    (sizeof(_hal_coral_serdes_qp_dbg_cfg_info) / sizeof(_hal_coral_serdes_qp_dbg_cfg_info[0]));

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_pxp_dbg_usxgmii_cfg[] = {
    {1, 0x902C, 0,  0, 0, 0x1,      "RG_RATE_ADAPT_RD_TOGGLE"},
    {1, 0x902C, 5,  0, 0, 0x0,      "RG_RATE_ADAPT_RD_TOGGLE"},
    {0, 0x9044, 0, 31, 0, 0x0, "RO_RATE_ADAPT_RX_XFI_SOP_CNT"},
    {0, 0x9048, 0, 31, 0, 0x0, "RO_RATE_ADAPT_RX_XFI_EOP_CNT"},
    {0, 0x904C, 0, 31, 0, 0x0, "RO_RATE_ADAPT_RX_MAC_SOP_CNT"},
    {0, 0x9050, 0, 31, 0, 0x0, "RO_RATE_ADAPT_RX_MAC_EOP_CNT"},
    {0, 0x9058, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_RX_XFI_FE_CNT"},
    {0, 0x9034, 0, 31, 0, 0x0, "RO_RATE_ADAPT_TX_XFI_SOP_CNT"},
    {0, 0x9038, 0, 31, 0, 0x0, "RO_RATE_ADAPT_TX_XFI_EOP_CNT"},
    {0, 0x903C, 0, 31, 0, 0x0, "RO_RATE_ADAPT_TX_MAC_SOP_CNT"},
    {0, 0x9040, 0, 31, 0, 0x0, "RO_RATE_ADAPT_TX_MAC_EOP_CNT"},
    {0, 0x9054, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_TX_MAC_FE_CNT"},
};

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_pxp_dbg_xgmii_cfg[] = {
    {1, 0x902C, 0,  1, 1, 0x1,           "RG_XGMII_PKT_RD_TOGGLE"},
    {1, 0x902C, 5,  1, 1, 0x0,           "RG_XGMII_PKT_RD_TOGGLE"},
    {0, 0x90B0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_0_STS(TX_FB)"},
    {0, 0x90B4, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_1_STS(TX_FD)"},
    {0, 0x90B8, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_2_STS(TX_ERR)"},
    {0, 0x90BC, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_3_STS(RX_FB)"},
    {0, 0x90C0, 0, 31, 0, 0x0,  "RO_RATE_ADAPT_CTRL_4_STS(RX_FD)"},
    {0, 0x90C4, 0, 31, 0, 0x0, "RO_RATE_ADAPT_CTRL_5_STS(RX_ERR)"},
};

const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_pxp_dbg_mii_cfg[] = {
    {1, 0x9124, 0, 31, 0, 0xAA,           "rate_adp_dbg_p0_0"},
    {1, 0x9124, 5, 31, 0, 0x00,           "rate_adp_dbg_p0_0"},
    {0, 0x9128, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x912C, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9148, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9130, 0, 15, 0,  0x0, "RO_P0_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9134, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9138, 0, 31, 0,  0x0, "RO_P0_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x914C, 0, 31, 0,  0x0, "RO_P0_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9150, 0, 15, 0,  0x0, "RO_P0_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9154, 0, 31, 0, 0xAA,           "rate_adp_dbg_p1_0"},
    {1, 0x9154, 5, 31, 0, 0x00,           "rate_adp_dbg_p1_0"},
    {0, 0x9158, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x915C, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9178, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9160, 0, 15, 0,  0x0, "RO_P1_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9164, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9168, 0, 31, 0,  0x0, "RO_P1_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x917C, 0, 31, 0,  0x0, "RO_P1_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9180, 0, 15, 0,  0x0, "RO_P1_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9184, 0, 31, 0, 0xAA,           "rate_adp_dbg_p2_0"},
    {1, 0x9184, 5, 31, 0, 0x00,           "rate_adp_dbg_p2_0"},
    {0, 0x9188, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x918C, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9208, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9190, 0, 15, 0,  0x0, "RO_P2_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9194, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9198, 0, 31, 0,  0x0, "RO_P2_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x920C, 0, 31, 0,  0x0, "RO_P2_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9210, 0, 15, 0,  0x0, "RO_P2_DBG_PHY_MII_TX_ER_CNT"},
    {1, 0x9214, 0, 31, 0, 0xAA,           "rate_adp_dbg_p3_0"},
    {1, 0x9214, 5, 31, 0, 0x00,           "rate_adp_dbg_p3_0"},
    {0, 0x9218, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_D5_CNT"},
    {0, 0x921C, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_EN_CNT"},
    {0, 0x9238, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_EN_CNT"},
    {0, 0x9220, 0, 15, 0,  0x0, "RO_P3_DBG_MAC_MII_TX_ER_CNT"},
    {0, 0x9224, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_D5_CNT"},
    {0, 0x9228, 0, 31, 0,  0x0, "RO_P3_DBG_MAC_MII_RX_DV_CNT"},
    {0, 0x923C, 0, 31, 0,  0x0, "RO_P3_DBG_PHY_MII_RX_DV_CNT"},
    {0, 0x9240, 0, 15, 0,  0x0, "RO_P3_DBG_PHY_MII_TX_ER_CNT"},
};
const HAL_CORAL_SERDES_DBG_CFG_T _hal_coral_serdes_pxp_dbg_clear_cfg[] = {
    {1, 0x9124, 0, 31, 0, 0x55,     "rate_adp_dbg_p0_0"},
    {1, 0x9124, 5, 31, 0, 0x00,     "rate_adp_dbg_p0_0"},
    {1, 0x9154, 0, 31, 0, 0x55,     "rate_adp_dbg_p1_0"},
    {1, 0x9154, 5, 31, 0, 0x00,     "rate_adp_dbg_p1_0"},
    {1, 0x9184, 0, 31, 0, 0x55,     "rate_adp_dbg_p2_0"},
    {1, 0x9184, 5, 31, 0, 0x00,     "rate_adp_dbg_p2_0"},
    {1, 0x9214, 0, 31, 0, 0x55,     "rate_adp_dbg_p3_0"},
    {1, 0x9214, 5, 31, 0, 0x00,     "rate_adp_dbg_p3_0"},
    {1, 0x902c, 0,  2, 2,  0x0, "rg_rate_adapt_ctrl_11"},
    {1, 0x902c, 5,  2, 2,  0x1, "rg_rate_adapt_ctrl_11"},
};

/* clang-format off */
const HAL_CORAL_SERDES_DBG_CFG_INFO_T
_hal_coral_serdes_pxp_dbg_cfg_info[] =
{
    {
        HAL_PHY_SERDES_INTERFACE_USXGMII,
        (sizeof(_hal_coral_serdes_pxp_dbg_usxgmii_cfg) / sizeof(_hal_coral_serdes_pxp_dbg_usxgmii_cfg[0])),
        &_hal_coral_serdes_pxp_dbg_usxgmii_cfg[0]
    },
    {
        HAL_PHY_SERDES_INTERFACE_XGMII,
        (sizeof(_hal_coral_serdes_pxp_dbg_xgmii_cfg) / sizeof(_hal_coral_serdes_pxp_dbg_xgmii_cfg[0])),
        &_hal_coral_serdes_pxp_dbg_xgmii_cfg[0]
    },
    {
        HAL_PHY_SERDES_INTERFACE_MII,
        (sizeof(_hal_coral_serdes_pxp_dbg_mii_cfg) / sizeof(_hal_coral_serdes_pxp_dbg_mii_cfg[0])),
        &_hal_coral_serdes_pxp_dbg_mii_cfg[0]
    },
    {
        HAL_PHY_SERDES_READ_CLEAR,
        (sizeof(_hal_coral_serdes_pxp_dbg_clear_cfg) / sizeof(_hal_coral_serdes_pxp_dbg_clear_cfg[0])),
        &_hal_coral_serdes_pxp_dbg_clear_cfg[0]
    }
};
/* clang-format on */

const UI32_T _hal_coral_serdes_pxp_dbg_cfg_info_size =
    (sizeof(_hal_coral_serdes_pxp_dbg_cfg_info) / sizeof(_hal_coral_serdes_pxp_dbg_cfg_info[0]));
