#ifndef EFUSE_H
#define EFUSE_H
#include "typedefs.h"

#define CR_EFUSEC_CON1          (CR_EFUSE_BASE)
#define CR_EFUSEC_CON2          (CR_EFUSE_BASE + 0x4)
#define CR_EFUSE_DATA0          (CR_EFUSE_BASE + 0x100)

#define EFUSE_VLD_OFS           (0)

/* address for accessing the base of each section in EFUSE */
#define PROD_CFG_HEADER_BASE        (0)
#define EF_SERDES_CALI_BASE         (2)                 /* word 2 */
#define EF_PHY_CALI_BASE            (3)                 /* word 3 */
#define EF_PHY_CALI_SIZE_PER_PHY    (4)                 /* 4 word per phy */
#define EF_PHY_CALI_GLB_BASE        (23)
#define BUCK_CFG_BASE               (25)
#define RSV_REG_BASE                (30)
#define RSV_REG_END                 (61)

typedef struct
{
    uint32_t sku_id                     :2;             /* 0:1 */
    uint32_t ldps                       :1;             /* 2 */
    uint32_t led_sync                   :1;             /* 3 */
    uint32_t phy_num                    :3;             /* 4:6 */
    uint32_t cpu_freq                   :1;             /* 7 */
    uint32_t lpm                        :1;             /* 8 */
    uint32_t wol                        :1;             /* 9 */
    uint32_t rsv0                       :6;             /* 10:15 */
    uint32_t sku_id_rmk                 :2;             /* 16:17 */
    uint32_t ldps_rmk                   :1;             /* 18 */
    uint32_t led_sync_rmk               :1;             /* 19 */
    uint32_t phy_num_rmk                :3;             /* 20:22 */
    uint32_t cpu_freq_rmk               :1;             /* 23 */
    uint32_t lpm_rmk                    :1;             /* 24 */
    uint32_t wol_rmk                    :1;             /* 25 */
    uint32_t rsv1                       :6;             /* 26:31 */
} MCU_PARA_T;

typedef union
{
    MCU_PARA_T param;
    uint32_t word;
} MCU_CFG_T;

typedef struct
{
    uint32_t efuse_ver                  :2;             /* 0:1 */
    uint32_t eco_ver                    :2;             /* 2:3 */
    uint32_t iddq                       :8;             /* 4:11 */
    uint32_t early_col_detect           :1;             /* 12 */
    uint32_t crs_col_rxdv               :1;             /* 13 */
    uint32_t col_sync                   :1;             /* 14 */
    uint32_t cfg_pbl_cmpsat_en          :1;             /* 15 */
    uint32_t full_lv                    :4;             /* 16:19 */
    uint32_t empty_lv                   :4;             /* 20:23 */
    uint32_t rsv0                       :8;             /* 24:31 */
} EFIFO_PARA_T;

typedef union
{
    EFIFO_PARA_T param;
    uint32_t word;
} EFIFO_CFG_T;

typedef struct
{
    uint32_t qp_bias_v2v_cal            :6;             /* 0:5 */
    uint32_t qp_tx_term_sel             :2;             /* 6:7 */
    uint32_t qp_rx_imp_sel              :5;             /* 8:12 */
    uint32_t comp_ctl_p0                :3;             /* 13:15 */
    uint32_t comp_ctl_p1                :3;             /* 16:18 */
    uint32_t comp_ctl_p2                :3;             /* 19:21 */
    uint32_t comp_ctl_p3                :3;             /* 22:24 */
    uint32_t comp_ctl_p4                :3;             /* 25:27 */
    uint32_t tx_bit_inv                 :1;             /* 28 */
    uint32_t rsv0                       :3;             /* 29:31 */
} SERDES_CALI_T;

typedef union
{
    SERDES_CALI_T calibration;
    uint32_t word;
} EFUSE_SERDES_CALI_T;

typedef struct
{
    /* word 0 */
    uint32_t da_tx_i2mpb_a_gbe          :6;             /* 0:5 */
    uint32_t da_tx_i2mpb_a_tst          :6;             /* 6:11 */
    uint32_t da_tx_i2mpb_a_hbt          :6;             /* 12:17 */
    uint32_t cr_tx_amp_offset_a         :6;             /* 18:23 */
    uint32_t cr_r50ohm_rsel_tx_a        :7;             /* 24:30 */
    uint32_t rsv1                       :1;             /* 31 */

    /* word 1 */
    uint32_t da_tx_i2mpb_b_gbe          :6;             /* 0:5 */
    uint32_t da_tx_i2mpb_b_tst          :6;             /* 6:11 */
    uint32_t da_tx_i2mpb_b_hbt          :6;             /* 12:17 */
    uint32_t cr_tx_amp_offset_b         :6;             /* 18:23 */
    uint32_t cr_r50ohm_rsel_tx_b        :7;             /* 24:30 */
    uint32_t rsv2                       :1;             /* 31 */

    /* word 2 */
    uint32_t da_tx_i2mpb_c_gbe          :6;             /* 0:5 */
    uint32_t da_tx_i2mpb_c_tst          :6;             /* 6:11 */
    uint32_t da_tx_i2mpb_a_tbt          :6;             /* 12:17 */
    uint32_t cr_tx_amp_offset_c         :6;             /* 18:23 */
    uint32_t cr_r50ohm_rsel_tx_c        :7;             /* 24:30 */
    uint32_t rsv3                       :1;             /* 31 */

    /* word 3 */
    uint32_t da_tx_i2mpb_d_gbe          :6;             /* 0:5 */
    uint32_t da_tx_i2mpb_d_tst          :6;             /* 6:11 */
    uint32_t da_tx_i2mpb_b_tbt          :6;             /* 12:17 */
    uint32_t cr_tx_amp_offset_d         :6;             /* 18:23 */
    uint32_t cr_r50ohm_rsel_tx_d        :7;             /* 24:30 */
    uint32_t rsv4                       :1;             /* 31 */

} PHY_CALI_T;

typedef union
{
    PHY_CALI_T calibration;
    uint32_t word[4];
} EFUSE_PHY_CALI_T;

typedef struct
{
    uint32_t rg_bg_rasel                :3;             /* 0:2 */
    uint32_t rg_rext_trim               :6;             /* 3:8 */
    uint32_t cr_da_tx_ps_drir0_hbt_tbt  :4;             /* 9:12 */
    uint32_t cr_da_tx_ps_drir0_b_hbt_tbt:4;             /* 13:16 */
    uint32_t cr_da_tx_ps_op_hbt         :5;             /* 17:21 */
    uint32_t cr_da_tx_ps_op_b_hbt       :5;             /* 22:26 */
    uint32_t cr_da_tx_ps_op_tbt         :5;             /* 27:31 */

    uint32_t cr_da_tx_ps_op_b_tbt       :5;             /* 0:4 */
    uint32_t cr_da_tx_rm2p_op_gbe       :3;             /* 5:7 */
    uint32_t cr_da_tx_rm2p_op_b_gbe     :3;             /* 8:10 */
    uint32_t cr_da_tx_cm1_op_gbe_llp    :4;             /* 11:14 */
    uint32_t rg_hvga_echo_res           :3;             /* 15:17 */
    uint32_t rg_hvga_echo_gain          :3;             /* 18:20 */
    uint32_t cr_adbuf_bias_gbe          :2;             /* 21:22 */
    uint32_t cr_adbuf_bias_lp           :2;             /* 23:24 */
    uint32_t rsv6                       :7;             /* 25:31 */
} PHY_CALI_GLB_T;

typedef union
{
    PHY_CALI_GLB_T calibration;
    uint32_t word[2];
} EFUSE_PHY_CALI_GLB_T;

typedef struct
{
    uint32_t rg_vbg_sel                 :3;             /* 0:2 */
    uint32_t rg_v2i_vref                :3;             /* 3:5 */
    uint32_t rg_v2i_ibias_sel           :4;             /* 6:9 */
    uint32_t rg_buck_vosel              :5;             /* 10:14 */
    uint32_t rg_buck_ugsr               :3;             /* 15:17 */
    uint32_t rg_buck_ib_trim            :5;             /* 18:22 */
    uint32_t rg_buck_pfmos_trim         :6;             /* 23:28 */
    uint32_t rg_buck_pfmos_sel_trim     :1;             /* 29 */
    uint32_t rsv0                       :2;             /* 30:31 */

    uint32_t rg_buck_pwmos_trim         :6;             /* 0:5 */
    uint32_t rg_buck_pwmos_sel_trim     :1;             /* 6 */
    uint32_t rg_buck_csos_trim          :6;             /* 7:12 */
    uint32_t rg_buck_csos_sel_trim      :1;             /* 13 */
    uint32_t rg_buck_csauto_trim        :3;             /* 14:16 */
    uint32_t rg_buck_csauto_sel_trim    :1;             /* 17 */
    uint32_t rg_buck_csm_trim           :3;             /* 18:20 */
    uint32_t rg_buck_zxos_trim          :4;             /* 21:24 */
    uint32_t rsv1                       :7;             /* 25:31 */
} BUCK_PARA_T;

typedef union
{
    BUCK_PARA_T param;
    uint32_t word[2];
} BUCK_CFG_T;

void efuse_bpkey_write(unsigned int enable);
void efuse_reload(void);
unsigned int efuse_read_word(unsigned int word_offset);
void efuse_write_word(unsigned int word_offset, unsigned int value);
void efuse_read_pkgid(void);
int efuse_write_pkgid(unsigned char id, unsigned char remark);
int efuse_test(unsigned int mode);
uint8_t is_efuse_empty(void);
#endif
