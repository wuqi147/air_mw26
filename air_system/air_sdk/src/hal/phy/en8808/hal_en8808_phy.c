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

/* FILE NAME:  hal_en8808_phy.c
 * PURPOSE:
 *  Implement en8808 phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/en8808/hal_en8808_phy.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_crc.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define PHY_MAX_NUMBER     (32)
#define MAX_READ_CRC_RETRY (50)
#define EN8808_PHY_NUMBER  (8)
#define EN8804_PHY_NUMBER  (4)

#define EN_8808_TR_REG_WR (0x8000)
#define SCREG_WF1         (0x10005014)
#define SKU_ID_MASK       (0xFFFF)
#define SKU_ID_EN8808     (0x8808)

#define EN8808_10M_100M_LOW_POWER (0x53aa)
#define EN8808_1G_LOW_POWER       (0x5faa)
#define EN8808_LONG_REACH         (0xf000)
#define EN8808_NORMAL             (0xc000)

#define EN8808_REG_BGPOR_CTRL1 (0x10000124UL)

#define SERDES_PCS_INT_STATE_REG  (0x0B5C)
#define SERDES_MODE_INTERRUPT_REG (0x0A20)
#define SERDES_AN_DONE            (0x1)
#define SERDES_AN_DONE_INTR       (0x1)
#define SERDES_AN_DONE_CLEAR      (0x4)

#define MAX_READ_EFUSE_RETRY (1000)
#define EN8808_EFUSE_CTRL    (0x10004008)
#define EN8808_EFUSE_RDATA1  (0x10004034)
#define EN8808_EFUSE_RDATA3  (0x1000403C)

#define EFUSE_GPHY_BASE (0x40200052)
#define EFSROM_KICK     (0x40000000)

#define TKRG_PKT_XMT_STA         BIT(15)
#define TKRG_DATA_ADDR_OFFSET    (1)
#define TKRG_NODE_ADDR_OFFSET    (7)
#define TKRG_CHANNEL_ADDR_OFFSET (11)
#define TKRG_WR_RD_CTRL_OFFSET   (13)
#define TKRG_READ_CTRL           (1 << TKRG_WR_RD_CTRL_OFFSET)
#define TKRG_WRITE_CTRL          (0 << TKRG_WR_RD_CTRL_OFFSET)
#define MAX_DOWN_SHIFT_TIMEOUT   (10)

#define HWRST_DEGLITCH_REG    (0x100000d4)
#define HWRST_DEGLITCH_ENABLE (0x7)
/* MACRO FUNCTION DECLARATIONS
 */
#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__)                 \
    do                                                                                 \
    {                                                                                  \
        __base_addr__ = SERDES_ID_S0_BASE_ADDR + (SERDES_ADDR_OFFSER * __serdes_id__); \
    } while (0)

#define PHY_ID_TO_SERDES_BASE_ADDRESS(__phy_id__, __serdes_addr__)       \
    do                                                                   \
    {                                                                    \
        UI32_T i;                                                        \
        for (i = 0; i < QSGMII_LANE_PORT_NUM; i++)                       \
        {                                                                \
            if (__phy_id__ == _PHY_REMOTE_SERDES_ADDR_MAPPING[i][0])     \
            {                                                            \
                __serdes_addr__ = _PHY_REMOTE_SERDES_ADDR_MAPPING[i][1]; \
            }                                                            \
        }                                                                \
    } while (0)

#define WRITE_EXTERNAL_EN880X_RG(__rg__, __val__)                               \
    do                                                                          \
    {                                                                           \
        rv = hal_mdio_writeRegByI2c(unit, port, __rg__, __val__);               \
        if (rv != AIR_E_OK)                                                     \
        {                                                                       \
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d bring up fail \n", port); \
            return rv;                                                          \
        }                                                                       \
    } while (0)

#define SFP_LED_WAVE_CFG_POS(__pid__, __led__)   (((__pid__ * 2) + __led__) * SFP_WAVEGEN_BIT_LEN)
#define SFP_LED_WAVE_EN_MASK(__pid__, __led__)   (SFP_WAVEGEN_EN_BIT << SFP_LED_WAVE_CFG_POS(__pid__, __led__))
#define SFP_LED_WAVE_TYPE_MASK(__pid__, __led__) (SFP_WAVEGEN_TYPE_MASK << SFP_LED_WAVE_CFG_POS(__pid__, __led__))

#define GET_SERDES_AN_DONE(__data__, __value__)      __value__ = (__data__ & SERDES_AN_DONE)
#define GET_SERDES_AN_DONE_INTR(__data__, __value__) __value__ = (__data__ & SERDES_AN_DONE_INTR)
#define CLEAR_SERDES_AN_DONE_INTR(__unit__, __port__, __serdes_base__)                                        \
    do                                                                                                        \
    {                                                                                                         \
        rv = hal_mdio_readRegByI2c(__unit__, __port__, (__serdes_base__ + SERDES_MODE_INTERRUPT_REG), &data); \
        if (AIR_E_OK == rv)                                                                                   \
        {                                                                                                     \
            data |= SERDES_AN_DONE_CLEAR;                                                                     \
            hal_mdio_writeRegByI2c(__unit__, __port__, (__serdes_base__ + SERDES_MODE_INTERRUPT_REG), data);  \
            data &= ~(SERDES_AN_DONE_CLEAR);                                                                  \
            hal_mdio_writeRegByI2c(__unit__, __port__, (__serdes_base__ + SERDES_MODE_INTERRUPT_REG), data);  \
        }                                                                                                     \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_en8808_phy.c");

extern const UI32_T                        en8808_fw_size;
extern const UI8_T                         en8808_fw[];
extern const HAL_PHY_CFG_T                 _hal_en8808_longreach[];
extern const UI32_T                        _hal_en8808_longreach_size;
extern const HAL_PHY_CFG_T                 _hal_en8808_normal[];
extern const UI32_T                        _hal_en8808_normal_size;
extern const UI32_T                        _hal_en8808_phy_dbg_cfg_info_size;
extern const HAL_EN8808_PHY_DBG_CFG_INFO_T _hal_en8808_phy_dbg_cfg_info[];
/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS */
/* Zcal to R50 mapping table */
static const UI8_T                         _en8808_zcal_to_r50ohm_tbl[64] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 124, 121, 118,
    114, 111, 108, 105, 102, 98,  96,  92,  88,  85,  82,  80,  76,  72,  70,  67,  64,  62,  60,  56,  54,  52,
    49,  48,  45,  43,  40,  39,  36,  34,  32,  32,  30,  28,  25,  24,  22,  20,  18,  16,  16,  14};

/* Tx offset table, value is from small to big */
static const UI8_T _en8808_tx_ofs_tbl[64] = {
    0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
    0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};

/* Initialization Tables */
#define TOTAL_NUMBER_OF_PATCH (14)
static UI16_T eee_patch_table[TOTAL_NUMBER_OF_PATCH][2] = {
    {RgAddr_dev1Eh_reg120h, 0x8014},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg144h, 0x0200},
    {RgAddr_dev1Eh_reg14Ah, 0xEE20},
    {RgAddr_dev1Eh_reg19Bh, 0x0111},
    {RgAddr_dev1Eh_reg234h, 0x1181},
    {RgAddr_dev1Eh_reg238h, 0x0120},
    {RgAddr_dev1Eh_reg239h, 0x0117},
    {RgAddr_dev1Fh_reg268h, 0x07F4},
    {RgAddr_dev1Eh_reg2D1h, 0x0733},
    {RgAddr_dev1Eh_reg323h, 0x0011},
    {RgAddr_dev1Eh_reg324h, 0x013F},
    {RgAddr_dev1Eh_reg326h, 0x0037}
};

#define TOTAL_NUMBER_OF_TR (19)
static UI16_T tr_reg_table[TOTAL_NUMBER_OF_TR][3] = {
    {0x55A0, 0x0000, 0x83AA},
    {0xFF3F, 0x0007, 0x83AE},
    {0x001E, 0x0000, 0x8F80},
    {0xB90A, 0x006F, 0x8F82},
    {0x0671, 0x0006, 0x8FAE},
    {0x2F00, 0x000E, 0x8FB0},
    {0x4444, 0x0044, 0x8ECC},
    {0x0004, 0x0000, 0x9686},
    {0xBAEF, 0x002E, 0x968C},
    {0x000B, 0x0000, 0x9690},
    {0x504D, 0x0000, 0x9698},
    {0x314F, 0x0002, 0x969A},
    {0x3028, 0x0000, 0x969E},
    {0x5010, 0x0000, 0x96A0},
    {0x0001, 0x0004, 0x96A2},
    {0x8670, 0x0001, 0x96A6},
    {0x024A, 0x0000, 0x96A8},
    {0x0072, 0x0000, 0x96B6},
    {0x3210, 0x0000, 0x96B8}
};

#ifdef AIR_EN_I2C_PHY

static UI32_T _hal_sco_serdes_force_speed[NUMBER_OF_SERDES_FORCE_SPEED_RG][SERDES_SGMII_FORCE_LAST] = {
    /*  {         Register_Address, AN        , 1000M     , 100M      , 10M       }, */
    {           SERDES_XGMII_DBG_0, 0x00000002, 0x00000002, 0x00000002, 0x00000002},
    {           SERDES_XGMII_DBG_1, 0x0000ff11, 0x0000ff11, 0x0000FF11, 0x0000FF11},
    {     SERDES_MSG_TX_CTRL_REG_1, 0x00000700, 0x00000700, 0x00000755, 0x000007AA},
    {   SERDES_SGMII_STS_CTL_REG_0, 0x00000000, 0x00000000, 0x00000014, 0x00000004},
    {     SERDES_MSG_RX_CTRL_REG_4, 0x00000700, 0x00000700, 0x00000755, 0x000007AA},
    {           SERDES_PHYA_REG_11, 0x00014813, 0x00014813, 0x00014813, 0x00014813},
    {           SERDES_PHYA_REG_19, 0x60004007, 0x60004007, 0x60004007, 0x60004007},
    {           SERDES_PHYA_REG_80, 0x00000005, 0x00000005, 0x00000010, 0x00000020},
    {SERDES_RATE_ADP_P0_CTRL_REG_0, 0x70000000, 0x70000000, 0x7000000c, 0x7000000f},
    {SERDES_RATE_ADP_P0_CTRL_REG_1, 0x010F010F, 0x010F010F, 0x010F010F, 0x010F010F},
    {              SERDES_AN_REG_0, 0x00009140, 0x00009140, 0x00009140, 0x00009140},
    {      SERDES_MII_RA_AN_ENABLE, 0x0000000F, 0x00000000, 0x00000000, 0x00000000},
    {           SERDES_PHYA_REG_30, 0x00010450, 0x00010050, 0x00010050, 0x00010050},
    {             SERDES_AN_REG_13, 0x31120023, 0x31120009, 0x31120009, 0x31120009}
};

#define REMOTE_SERDES_ADDR (0x10230000)
static UI32_T _PHY_REMOTE_SERDES_ADDR_MAPPING[4][2] = {
    /* {PHY ID, serdes offset } */
    {0, 0x10220000},
    {1, 0x10240000},
    {2, 0x10250000},
    {3, 0x10260000},
};
#endif

/* table/register control blocks */
static const UI8_T _en8808_pled_gpio[8] = {0, 1, 2, 3, 8, 10, 11, 12};

/* EXPORTED SUBPROGRAM BODIES*/
#ifdef AIR_EN_SPEED_DOWNSHIFT_ENHANCE
static void
_hal_en8808_phy_tkrg_wait_ready(
    UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI8_T          retry = 0x7F;
    UI16_T         reg_data = 0;

    do
    {
        rv = hal_mdio_readC22ByPort(0, port, 0x10, &reg_data);
        if (AIR_E_OK == rv)
        {
            retry--;
        }
    } while (((reg_data & TKRG_PKT_XMT_STA) == 0) && (retry > 0));
}

static UI32_T
_hal_en8808_phy_tkrg_read(
    UI32_T port,
    UI8_T  channel_addr,
    UI8_T  node_addr,
    UI8_T  data_addr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;
    UI32_T         rdata = 0;

    rv = hal_mdio_writeC22ByPort(0, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    if (AIR_E_OK == rv)
    {
        reg_data = (data_addr << TKRG_DATA_ADDR_OFFSET) | (node_addr << TKRG_NODE_ADDR_OFFSET) |
                   (channel_addr << TKRG_CHANNEL_ADDR_OFFSET) | TKRG_READ_CTRL | TKRG_PKT_XMT_STA;

        rv = hal_mdio_writeC22ByPort(0, port, 0x10, reg_data);
        if (AIR_E_OK == rv)
        {
            _hal_en8808_phy_tkrg_wait_ready(port);

            rv = hal_mdio_readC22ByPort(0, port, 0x11, &reg_data);
            if (AIR_E_OK == rv)
            {
                rdata = reg_data;
                rv = hal_mdio_readC22ByPort(0, port, 0x12, &reg_data);
                if (AIR_E_OK == rv)
                {
                    rdata |= (reg_data << 16);
                }
            }
        }
        rv = hal_mdio_writeC22ByPort(0, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg);
    }
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "port=%d, access error !\n", port);
    }
    return rdata;
}
#endif

static void
_en8808_TR_RegWr(
    UI16_T phy_addr,
    UI32_T tr_reg_addr,
    UI32_T tr_data)
{
    hal_mdio_writeC22(0, 0, phy_addr, 0x1F, 0x52B5); /* page select */
    hal_mdio_writeC22(0, 0, phy_addr, 0x11, (UI16_T)(tr_data & 0xFFFF));
    hal_mdio_writeC22(0, 0, phy_addr, 0x12, (UI16_T)(tr_data >> 16));
    hal_mdio_writeC22(0, 0, phy_addr, 0x10, (UI16_T)(tr_reg_addr | EN_8808_TR_REG_WR));
    hal_mdio_writeC22(0, 0, phy_addr, 0x1F, 0x0); /* page resetore */
    return;
}

static void
_en8808_anacal_exe(
    UI8_T phyadd_common)
{
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ch, TRUE);  /* da_calin_flag pull high */
    osal_delayUs(1000);
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ch, FALSE); /* da_calin_flag pull low */
}

static UI8_T
_en8808_R50_Calibration(
    UI8_T phyadd,
    UI8_T phyadd_common)
{
    I8_T   rg_zcal_ctrl = 0, rg_r50ohm_rsel_tx = 0, calibration_polarity = 0;
    UI8_T  all_ana_cal_status = 1;
    I16_T  backup_dev1e_e0 = 0, ad_cal_comp_out_init = 0, calibration_pair = 0;
    UI16_T rdata = 0;

    /* setting */
    hal_mdio_writeC22(0, 0, phyadd, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg); /* g0 */
    hal_mdio_writeC22(0, 0, phyadd, RgAddr_Reg00h, AN_disable_force_1000M);     /* AN disable, force 1000M */

    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg100h,
                      BG_voltage_output);                                                /* BG voltage output */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg145h, Fix_mdi);     /* fix mdi */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg185h,
                      Disable_tx_slew_control);                                          /* disable tx slew control */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0FBh, LDO_control); /* ldo */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_R50); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Disable_all);     /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E1h,
                      Disable_all);     /* 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD */

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    {
        all_ana_cal_status = 1;

        if (calibration_pair == ANACAL_PAIR_A)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                              Cal_control_R50_pairA_ENABLE); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen,
                                                                [4]:rg_rext_calen, [0]:rg_zcalen_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh, Zcalen_A_ENABLE);
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            hal_mdio_writeC45(
                0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                Cal_control_R50); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                              Zcalen_B_ENABLE); /* 1e_dc[12]:rg_zcalen_b */
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            hal_mdio_writeC45(
                0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                Cal_control_R50); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                              Zcalen_C_ENABLE); /* 1e_dc[8]:rg_zcalen_c */
        }
        else                                    /* if(calibration_pair == ANACAL_PAIR_D) */
        {
            hal_mdio_writeC45(
                0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                Cal_control_R50); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                              Zcalen_D_ENABLE); /* 1e_dc[4]:rg_zcalen_d */
        }

        /* calibrate */
        rg_zcal_ctrl = ZCAL_MIDDLE; /* start with 0 dB */

        hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h, &rdata);
        backup_dev1e_e0 = (rdata & (~0x003f));
        hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h, (backup_dev1e_e0 | rg_zcal_ctrl));

        _en8808_anacal_exe(phyadd_common);

        hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);
        ad_cal_comp_out_init = (rdata >> 8) & 0x1; /* 1e_17a[8]:ad_cal_comp_out */

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }

        while (all_ana_cal_status < ANACAL_ERROR)
        {
            rg_zcal_ctrl += calibration_polarity;

            hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h,
                              (backup_dev1e_e0 | rg_zcal_ctrl));

            _en8808_anacal_exe(phyadd_common);

            hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((rdata >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
            }
            else
            {
                if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00))
                {
                    all_ana_cal_status = ANACAL_SATURATION; /* need to FT */
                    rg_zcal_ctrl = ZCAL_MIDDLE;             /* 0 dB */
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            rg_r50ohm_rsel_tx = ZCAL_MIDDLE; /* 0 dB */
        }
        else
        {
            if (rg_zcal_ctrl > (0x3F - R50_OFFSET_VALUE))
            {
                all_ana_cal_status = ANACAL_SATURATION; /* need to FT */
                rg_zcal_ctrl = ZCAL_MIDDLE;             /* 0 dB */
            }
            else
            {
                rg_zcal_ctrl += R50_OFFSET_VALUE;
            }

            rg_r50ohm_rsel_tx = _en8808_zcal_to_r50ohm_tbl[rg_zcal_ctrl];
        }

        if (calibration_pair == ANACAL_PAIR_A)
        {
            /* cr_r50ohm_rsel_tx_a */
            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg174h, &rdata);
            ad_cal_comp_out_init = rdata & (~MASK_r50ohm_rsel_tx_a);
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg174h,
                              (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 8) & MASK_MSB_8bit) |
                                                       Rg_r50ohm_rsel_tx_a_en))); /* 1e_174[15:8] */
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            /* cr_r50ohm_rsel_tx_b */
            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg174h, &rdata);
            ad_cal_comp_out_init = rdata & (~MASK_r50ohm_rsel_tx_b);
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg174h,
                              (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 0) & MASK_LSB_8bit) |
                                                       Rg_r50ohm_rsel_tx_b_en))); /* 1e_174[7:0] */
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            /* cr_r50ohm_rsel_tx_c */
            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg175h, &rdata);
            ad_cal_comp_out_init = rdata & (~MASK_r50ohm_rsel_tx_c);
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg175h,
                              (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 8) & MASK_MSB_8bit) |
                                                       Rg_r50ohm_rsel_tx_c_en))); /* 1e_175[15:8] */
        }
        else /* if(calibration_pair == ANACAL_PAIR_D) */
        {
            /* cr_r50ohm_rsel_tx_d */
            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg175h, &rdata);
            ad_cal_comp_out_init = rdata & (~MASK_r50ohm_rsel_tx_d);
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg175h,
                              (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 0) & MASK_LSB_8bit) |
                                                       Rg_r50ohm_rsel_tx_d_en))); /* 1e_175[7:0] */
        }
    }

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh, Disable_all);

    return all_ana_cal_status;
}

static UI8_T
_en8808_TX_OFS_Calibration(
    UI8_T phyadd,
    UI8_T phyadd_common)
{
    I8_T   tx_offset_index = 0, calibration_polarity = 0;
    UI8_T  all_ana_cal_status = 1, tx_offset_reg_shift = 0, tbl_idx = 0;
    I16_T  ad_cal_comp_out_init = 0, calibration_pair = 0, tx_offset_reg = 0, reg_temp = 0;
    UI16_T rdata = 0;

    /* setting */
    hal_mdio_writeC22(0, 0, phyadd, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg); /* g0 */
    hal_mdio_writeC22(0, 0, phyadd, RgAddr_Reg00h, AN_disable_force_1000M);     /* AN disable, force 1000M */

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg100h, BG_voltage_output); /* BG voltage output */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg145h, Fix_mdi);           /* fix mdi */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg185h,
                      Disable_tx_slew_control);                                          /* disable tx slew control */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0FBh, LDO_control); /* ldo */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_TX_OFST);   /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Rg_txvos_calen_ENABLE); /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_TX_OFST);   /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Rg_txvos_calen_ENABLE); /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E1h,
                      Disable_all);           /* 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg096h,
                      Bypass_tx_offset_cal);  /* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg03Eh, Enable_Tx_VLD); /* 1e_3e:enable Tx VLD */

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    {
        all_ana_cal_status = 1;

        tbl_idx = TX_OFFSET_0mV_idx;
        tx_offset_index = _en8808_tx_ofs_tbl[tbl_idx];

        if (calibration_pair == ANACAL_PAIR_A)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_a_ENABLE);                   /* 1e_dd[12]:rg_txg_calen_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Dh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_0V)); /* 1e_17d:dac_in0_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg181h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_0V)); /* 1e_181:dac_in1_a */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg172h, &rdata);
            reg_temp = (rdata & (~MASK_cr_tx_amp_offset_MSB));
            tx_offset_reg_shift = 8; /* 1e_172[13:8] */
            tx_offset_reg = RgAddr_dev1Eh_reg172h;
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_b_ENABLE);                   /* 1e_dd[8]:rg_txg_calen_b */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Eh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_0V)); /* 1e_17e:dac_in0_b */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg182h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_0V)); /* 1e_182:dac_in1_b */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg172h, &rdata);
            reg_temp = (rdata & (~MASK_cr_tx_amp_offset_LSB));
            tx_offset_reg_shift = 0; /* 1e_172[5:0] */
            tx_offset_reg = RgAddr_dev1Eh_reg172h;
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_c_ENABLE);                   /* 1e_dd[4]:rg_txg_calen_c */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Fh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_0V)); /* 1e_17f:dac_in0_c */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg183h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_0V)); /* 1e_183:dac_in1_c */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg173h, &rdata);
            reg_temp = (rdata & (~MASK_cr_tx_amp_offset_MSB));
            tx_offset_reg_shift = 8; /* 1e_173[13:8] */
            tx_offset_reg = RgAddr_dev1Eh_reg173h;
        }
        else /* if(calibration_pair == ANACAL_PAIR_D) */
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_d_ENABLE);                   /* 1e_dd[0]:rg_txg_calen_d */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg180h,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_0V)); /* 1e_180:dac_in0_d */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg184h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_0V)); /* 1e_184:dac_in1_d */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg173h, &rdata);
            reg_temp = (rdata & (~MASK_cr_tx_amp_offset_LSB));
            tx_offset_reg_shift = 0; /* 1e_173[5:0] */
            tx_offset_reg = RgAddr_dev1Eh_reg173h;
        }

        /* calibrate */
        /* tx_offset_index = TX_AMP_OFFSET_0mV; */
        tbl_idx = TX_OFFSET_0mV_idx;
        tx_offset_index = _en8808_tx_ofs_tbl[tbl_idx];
        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, tx_offset_reg,
                          (reg_temp | (tx_offset_index << tx_offset_reg_shift))); /* 1e_172, 1e_173 */

        _en8808_anacal_exe(phyadd_common);

        hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);
        ad_cal_comp_out_init = (rdata >> 8) & 0x1; /* 1e_17a[8]:ad_cal_comp_out */

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }

        while (all_ana_cal_status < ANACAL_ERROR)
        {
            tbl_idx += calibration_polarity;
            tx_offset_index = _en8808_tx_ofs_tbl[tbl_idx];

            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, tx_offset_reg,
                              (reg_temp | (tx_offset_index << tx_offset_reg_shift))); /* 1e_172, 1e_173 */

            _en8808_anacal_exe(phyadd_common);

            hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((rdata >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
            }
            else
            {
                if ((tx_offset_index == 0x3f) || (tx_offset_index == 0x1f))
                {
                    all_ana_cal_status = ANACAL_SATURATION; /* need to FT */
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            tbl_idx = TX_OFFSET_0mV_idx;
            tx_offset_index = _en8808_tx_ofs_tbl[tbl_idx];

            hal_mdio_writeC45(
                0, 0, phyadd, MMD_DEV_VSPEC1, tx_offset_reg,
                (reg_temp | (tx_offset_index << tx_offset_reg_shift))); /* cr_tx_amp_offset_a/b/c/d, 1e_172, 1e_173 */
        }
    }

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Dh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Eh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Fh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg180h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg181h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg182h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg183h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg184h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Disable_all); /* disable analog calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Disable_all); /* disable Tx offset calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Disable_all); /* disable analog calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Disable_all); /* disable Tx offset calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg03Eh, Disable_all); /* disable Tx VLD force mode */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                      Disable_all); /* disable Tx offset/amplitude calibration circuit */

    return all_ana_cal_status;
}

UI8_T
_en8808_TX_AMP_Calibration(
    UI8_T phyadd,
    UI8_T phyadd_common)
{
    I8_T   tx_amp_index = 0, calibration_polarity = 0;
    UI8_T  all_ana_cal_status = 1, tx_amp_reg_shift = 0;
    UI8_T  tx_amp_reg = 0, tx_amp_reg_100 = 0;
    UI16_T ad_cal_comp_out_init = 0, calibration_pair = 0, reg_temp = 0, rdata = 0;

    /* phyadd_common = phyadd; */

    /* setting */
    hal_mdio_writeC22(0, 0, phyadd, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg); /* g0 */
    hal_mdio_writeC22(0, 0, phyadd, RgAddr_Reg00h, AN_disable_force_1000M);     /* AN disable, force 1000M */

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg100h, BG_voltage_output); /* BG voltage output */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg145h, Fix_mdi);           /* fix mdi */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg185h,
                      Disable_tx_slew_control);                                          /* disable tx slew control */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0FBh, LDO_control); /* ldo */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_TX_AMP);    /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Rg_txvos_calen_ENABLE); /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E1h,
                      Rg_cal_refsel_ENABLE);  /* 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_TX_AMP);    /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Rg_txvos_calen_ENABLE); /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E1h,
                      Rg_cal_refsel_ENABLE);  /* 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg096h,
                      Bypass_tx_offset_cal);  /* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg03Eh, Enable_Tx_VLD); /* 1e_3e:enable Tx VLD */

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    /* for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_B; calibration_pair++) debugging */
    {
        all_ana_cal_status = 1;

        /* calibrate */
        tx_amp_index = TX_AMP_MIDDLE; /* start with 0 dB */
        if (calibration_pair == ANACAL_PAIR_A)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_a_ENABLE); /* 1e_dd[12]:rg_txg_calen_a amp calibration enable */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Dh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_2V)); /* 1e_17d:dac_in0_a */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg181h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_2V)); /* 1e_181:dac_in1_a */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg012h, &rdata);
            reg_temp = (rdata & (~MASK_da_tx_i2mpb_a_gbe));
            tx_amp_reg_shift = 10; /* 1e_12[15:10] */
            tx_amp_reg = RgAddr_dev1Eh_reg012h;
            tx_amp_reg_100 = 0x16;
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_b_ENABLE); /* 1e_dd[8]:rg_txg_calen_b amp calibration enable */
            /* Serial.println(Rg_txg_calen_b_ENABLE, HEX); */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Eh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_2V)); /* 1e_17e:dac_in0_b */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg182h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_2V)); /* 1e_182:dac_in1_b */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg017h, &rdata);
            reg_temp = (rdata & (~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; /* 1e_17[13:8] */
            tx_amp_reg = RgAddr_dev1Eh_reg017h;
            tx_amp_reg_100 = 0x18;
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_c_ENABLE); /* 1e_dd[4]:rg_txg_calen_c amp calibration enable */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Fh,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_2V)); /* 1e_17f:dac_in0_c */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg183h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_2V)); /* 1e_183:dac_in1_c */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg019h, &rdata);
            reg_temp = (rdata & (~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; /* 1e_19[13:8] */
            tx_amp_reg = RgAddr_dev1Eh_reg019h;
            tx_amp_reg_100 = 0x20;
        }
        else /* if(calibration_pair == ANACAL_PAIR_D) */
        {
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                              Rg_txg_calen_d_ENABLE); /* 1e_dd[0]:rg_txg_calen_d amp calibration enable */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg180h,
                              (Force_dasn_dac_in0_ENABLE | DAC_IN_2V)); /* 1e_180:dac_in0_d */
            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg184h,
                              (Force_dasn_dac_in1_ENABLE | DAC_IN_2V)); /* 1e_184:dac_in1_d */

            hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg021h, &rdata);
            reg_temp = (rdata & (~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; /* 1e_21[13:8] */
            tx_amp_reg = RgAddr_dev1Eh_reg021h;
            tx_amp_reg_100 = 0x22;
        }

        /* calibrate */
        tx_amp_index = TX_AMP_MIDDLE; /* start with 0 dB */

        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, tx_amp_reg,
                          (reg_temp | (tx_amp_index << tx_amp_reg_shift))); /* 1e_12/17/19/21 */

        _en8808_anacal_exe(phyadd_common);

        hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);
        ad_cal_comp_out_init = (rdata >> 8) & 0x1; /* 1e_17a[8]:ad_cal_comp_out */
        /* Serial.println(ad_cal_comp_out_init, HEX); */

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }
        while (all_ana_cal_status < ANACAL_ERROR)
        {
            tx_amp_index += calibration_polarity;
            /* Serial.println(tx_amp_index, HEX); */

            hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, tx_amp_reg,
                              (reg_temp | (tx_amp_index << tx_amp_reg_shift)));

            _en8808_anacal_exe(phyadd_common);

            hal_mdio_readC45(0, 0, phyadd_common, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((rdata >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
                /* Serial.print("    tx_amp_index: "); */
                /* Serial.println(tx_amp_index, HEX); */
                /* reg_temp = get_gphy_reg_cl45(phyadd, 0x1e, tx_amp_reg)&(~0xff00); */
                /* set_gphy_reg_cl45(phyadd, 0x1e, tx_amp_reg, (reg_temp|((tx_amp_index + */
                /* tst_offset)<<tx_amp_reg_shift)));  for gbe(DAC) */
            }
            else
            {
                if ((tx_amp_index == 0x3f) || (tx_amp_index == 0x00))
                {
                    all_ana_cal_status = ANACAL_SATURATION; /* need to FT */
                    tx_amp_index = TX_AMP_MIDDLE;
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            tx_amp_index = TX_AMP_MIDDLE;
        }

        /* da_tx_i2mpb_a_gbe / b/c/d, only GBE for now */
        hal_mdio_writeC45(
            0, 0, phyadd, MMD_DEV_VSPEC1, tx_amp_reg,
            ((tx_amp_index - TXAMP_offset) | ((tx_amp_index - TXAMP_offset) << tx_amp_reg_shift))); /* temp modify */
        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, tx_amp_reg_100,
                          ((tx_amp_index - TXAMP_offset) | ((tx_amp_index + TX_i2mpb_hbt_ofs) << tx_amp_reg_shift)));
    }

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Dh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Eh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Fh, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg180h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg181h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg182h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg183h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg184h, Disable_all);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Disable_all); /* disable analog calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh,
                      Disable_all); /* disable Tx offset calibration circuit */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg03Eh, Disable_all); /* disable Tx VLD force mode */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DDh,
                      Disable_all); /* disable Tx offset/amplitude calibration circuit */

    return all_ana_cal_status;
}

static UI8_T
_en8808_BG_Calibration(
    UI8_T phyadd,
    I8_T  calipolarity)
{
    I8_T   rg_zcal_ctrl = 0, calibration_polarity = 0;
    UI8_T  all_ana_cal_status = 1;
    UI16_T ad_cal_comp_out_init = 0, rdata = 0;

    /* setting */
    hal_mdio_writeC22(0, 0, phyadd, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg); /* g0 */
    hal_mdio_writeC22(0, 0, phyadd, RgAddr_Reg00h, AN_disable_force_1000M);     /* AN disable, force 1000M */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg100h, BG_voltage_output); /* BG voltage output */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg145h, Fix_mdi);           /* fix mdi */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Fh_reg0FFh, 0x2);
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh,
                      Cal_control_BG); /* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DCh, Disable_all); /* 1e_dc[0]:rg_txvos_calen */
    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E1h,
                      Disable_all); /* 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD */

    /* calibrate */
    rg_zcal_ctrl = ZCAL_MIDDLE;

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h, (UI16_T)rg_zcal_ctrl);

    _en8808_anacal_exe(phyadd);

    hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);
    ad_cal_comp_out_init = (rdata >> 8) & 0x1;

    if (ad_cal_comp_out_init == 1)
    {
        calibration_polarity = -calipolarity;
    }
    else /* ad_cal_comp_out_init == 0 */
    {
        calibration_polarity = calipolarity;
    }

    while (all_ana_cal_status < ANACAL_ERROR)
    {
        rg_zcal_ctrl += calibration_polarity;

        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h, (UI16_T)rg_zcal_ctrl);

        _en8808_anacal_exe(phyadd);

        hal_mdio_readC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg17Ah, &rdata);

        if (all_ana_cal_status == 0)
        {
            all_ana_cal_status = ANACAL_ERROR;
        }

        else if (((rdata >> 8) & 0x1) != ad_cal_comp_out_init)
        {
            all_ana_cal_status = ANACAL_FINISH;
        }
        else
        {
            if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00))
            {
                all_ana_cal_status = ANACAL_SATURATION; /* need to FT */
                rg_zcal_ctrl = ZCAL_MIDDLE;             /* 0 dB */
            }
        }
    }

    if (all_ana_cal_status == ANACAL_ERROR)
    {
        rg_zcal_ctrl = ZCAL_MIDDLE; /* 0 dB */

        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h, (UI16_T)rg_zcal_ctrl);
    }
    else
    {
        /* rg_zcal_ctrl[5:0] rg_rext_trim[13:8] */
        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0E0h,
                          (UI16_T)((rg_zcal_ctrl << 8) | rg_zcal_ctrl));

        /* 1f_115[2:0](rg_bg_rasel) = rg_zcal_ctrl[5:3] */
        hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg115h, (UI16_T)((rg_zcal_ctrl & 0x3f) >> 3));
    }

    hal_mdio_writeC45(0, 0, phyadd, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg0DBh, Disable_all);
    return all_ana_cal_status;
}

static void
_en8808_phy_config(
    UI8_T phy_base)
{
    UI8_T port = 1;

    for (port = 1; port <= TOTAL_PHYS_IN_CHIP; port++)
    {
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x3e, 0xf000); /* force on TXVLD */

        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x120, 0x8041);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x122, 0xFFFF);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x123, 0xFFFF);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x144, 0x0200);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x14A, 0xEE20);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x189, 0x0110);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x19B, 0x0111);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x234, 0x0181);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x238, 0x0120);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x239, 0x0117);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x268, 0x07F4);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x2D1, 0x0733);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x323, 0x0011);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x324, 0x013F);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x326, 0x0037);

        /* Token Ring */
        _en8808_TR_RegWr(phy_base + port, 0x03AA, 0x0055A0);
        _en8808_TR_RegWr(phy_base + port, 0x03AE, 0x07FF3F);
        _en8808_TR_RegWr(phy_base + port, 0x0F80, 0x00001E);
        _en8808_TR_RegWr(phy_base + port, 0x0F82, 0x6FB90A);
        _en8808_TR_RegWr(phy_base + port, 0x0FAE, 0x060671);
        _en8808_TR_RegWr(phy_base + port, 0x0FB0, 0x0E2F00);
        _en8808_TR_RegWr(phy_base + port, 0x0ECC, 0x444444);
        _en8808_TR_RegWr(phy_base + port, 0x1686, 0x000000);
        _en8808_TR_RegWr(phy_base + port, 0x168C, 0x2EBAEF);
        _en8808_TR_RegWr(phy_base + port, 0x1690, 0x00000B);
        _en8808_TR_RegWr(phy_base + port, 0x1698, 0x00504D);
        _en8808_TR_RegWr(phy_base + port, 0x169A, 0x02314F);
        _en8808_TR_RegWr(phy_base + port, 0x169E, 0x003028);
        _en8808_TR_RegWr(phy_base + port, 0x16A0, 0x005010);
        _en8808_TR_RegWr(phy_base + port, 0x16A2, 0x040001);
        _en8808_TR_RegWr(phy_base + port, 0x16A6, 0x018670);
        _en8808_TR_RegWr(phy_base + port, 0x16A8, 0x00024A);
        _en8808_TR_RegWr(phy_base + port, 0x16B6, 0x000072);
        _en8808_TR_RegWr(phy_base + port, 0x16B8, 0x003210);

        hal_mdio_writeC22(0, 0, phy_base + port, EN8808_PHY_MII_PAGE_SELECT, 0x0003);
        hal_mdio_writeC22(0, 0, phy_base + port, 0x1C, 0x0c92);
        hal_mdio_writeC22(0, 0, phy_base + port, 0x1D, 0x0001);
        hal_mdio_writeC22(0, 0, phy_base + port, EN8808_PHY_MII_PAGE_SELECT, 0x0000);
    }
}

static void
_en8808_set_gphy_TrReg(
    UI8_T  prtid,
    UI16_T parm_1,
    UI16_T parm_2,
    UI16_T parm_3)
{
    hal_mdio_writeC22(0, 0, prtid, RgAddr_TrReg11h, parm_1);
    hal_mdio_writeC22(0, 0, prtid, RgAddr_TrReg12h, parm_2);
    hal_mdio_writeC22(0, 0, prtid, RgAddr_TrReg10h, parm_3);
}

static UI32_T
_en8808_get_gphy_TrReg(
    const UI32_T unit,
    const UI32_T port,
    UI16_T       reg_addr)
{
    UI32_T         data = 0;
    UI16_T         high = 0, low = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_writeC22ByPort(unit, port, RgAddr_TrReg10h, reg_addr);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, RgAddr_TrReg11h, &low);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, RgAddr_TrReg12h, &high);
            if (AIR_E_OK == rv)
            {
                data = ((high << 16) | low);
            }
        }
    }

    return data;
}

AIR_ERROR_NO_T
_hal_en8808_phy_getRandomSeed(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *random_seed)
{
    UI8_T          index = 0;
    UI16_T         backup_dev1e_reg03ch = 0, backup_dev1f_reg015h = 0, reg_data = 0;
    UI16_T         backup_cl22_reg9h = 0, backup_dev1e_reg0c9h = 0;
    UI32_T         backup_vga = 0, value, seed = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &backup_cl22_reg9h);
    rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xc9, &backup_dev1e_reg0c9h);
    rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, &backup_dev1e_reg03ch);
    rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, &backup_dev1f_reg015h);

    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    backup_vga = _en8808_get_gphy_TrReg(unit, port, 0xafa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x140);
    value = backup_dev1e_reg03ch | 0x8800;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, value);

    value = backup_dev1f_reg015h & (~0xF1FF);
    value |= 0x108b;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, value);

    value = backup_vga | 0x001f0000;
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8808_set_gphy_TrReg(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), (value & 0xffff), (value >> 16) & 0xffff, 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    for (index = 0; index < 32; index++)
    {
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        reg_data &= 0xff;
        seed |= ((reg_data & 0x1) << index);
    }

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1000);

    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8808_set_gphy_TrReg(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), (backup_vga & 0xffff), ((backup_vga >> 16) & 0xffff),
                           0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, backup_dev1f_reg015h);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, backup_dev1e_reg03ch);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xc9, backup_dev1e_reg0c9h);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, backup_cl22_reg9h);

    if (AIR_E_OK == rv)
    {
        *random_seed = seed;
    }

    return rv;
}

AIR_ERROR_NO_T
_hal_en8808_phy_calRandomSeed(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T link_status)
{
    UI32_T         seed = 0;
    UI16_T         reg_value = 0, reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_SPEED_DOWNSHIFT_ENHANCE
    HAL_PHY_SSD_MODE_T spd_mode = HAL_PHY_SSD_MODE_LAST;
#endif

    if (TRUE == link_status)
    {
#ifdef AIR_EN_SPEED_DOWNSHIFT_ENHANCE
        if (HAL_PHY_PORT_DEV_SPD_DOWN_ADV_CHANGED(unit, port))
        {
            /* restore 1G's advertise if it was changed */
            HAL_PHY_PORT_DEV_SPD_DOWN_ADV_CHANGED(unit, port) = 0;
            HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port) = 0;
            reg_data = HAL_PHY_PORT_DEV_SPD_DOWN_ADV_BACKUP(unit, port);
            rv = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "port=%d, 1G adv restore =%x\n", port, reg_data);
        }
#endif
        return rv;
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rv && (reg_data & MASTER_SLAVE_CFAULT))
    {
        seed = (osal_rand() % 5);

        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (reg_data | BMCR_PDOWN));
            osal_sleepTask(seed * 10);
            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (reg_data & (~(BMCR_PDOWN))));
                osal_sleepTask(seed * 10);
                /* per port random delay */
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, &reg_value);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, (reg_value | 0x1F));
                        if (AIR_E_OK == rv)
                        {
                            seed = (osal_rand() % 5);
                            osal_sleepTask(seed * 10);
                            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, (reg_value & 0xFFEF));
                            osal_sleepTask(seed * 10);
                            if (AIR_E_OK == rv)
                            {
                                reg_data |= BMCR_ANRESTART;
                                rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
                            }
                        }
                    }
                }
            }
        }
    }
#ifdef AIR_EN_SPEED_DOWNSHIFT_ENHANCE
    rv = hal_en8808_phy_getSmartSpeedDown(unit, port, &spd_mode);
    if ((AIR_E_OK == rv) && (HAL_PHY_SSD_MODE_DISABLE != spd_mode))
    {
        /* speed down shift check for work around*/
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (reg_data & BMCR_PDOWN)
            {
                /* if phy is power down then skip work around */
                return rv;
            }

            reg_data = _hal_en8808_phy_tkrg_read(port, 0x0, 0xf, 0x1);
            reg_data &= 0xf000;
            DIAG_PRINT(HAL_DBG_INFO, "port=%d, status= %x, speed down counter=%d \n", port, reg_data,
                       HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port));
            /* check 1G's AN state */
            if (0xa000 == reg_data)
            {
                if (HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port) > MAX_DOWN_SHIFT_TIMEOUT)
                {
                    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        HAL_PHY_PORT_DEV_SPD_DOWN_ADV_BACKUP(unit, port) = reg_data;
                        /* disable 1G abaility */
                        reg_data &= ~ADVERTISE_1000FULL;
                        rv = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, reg_data);
                        if (AIR_E_OK == rv)
                        {
                            HAL_PHY_PORT_DEV_SPD_DOWN_ADV_CHANGED(unit, port) = 1;
                            rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
                            if (AIR_E_OK == rv)
                            {
                                reg_data |= BMCR_ANRESTART;
                                rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
                            }
                        }
                    }
                    HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port) = 0;
                }
                else
                {
                    HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port)++;
                }
            }
            else
            {
                HAL_PHY_PORT_DEV_SPD_DOWN_TIMEOUT_COUNT(unit, port) = 0;
            }
        }
    }
#endif
    return rv;
}

AIR_ERROR_NO_T
_hal_en8808_phy_checkInitDone(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *init_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         user_port = 0;

    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), user_port)
    {
        if (HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port) == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, user_port))
        {
            if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
            {
                if (HAL_PHY_PORT_DEV_I2C_ADDR(unit, port) == HAL_PHY_PORT_DEV_I2C_ADDR(unit, user_port))
                {
                    if (port > user_port)
                    {
                        *init_status = TRUE;
                    }
                    else
                    {
                        *init_status = FALSE;
                    }
                    DIAG_PRINT(HAL_DBG_INFO, "port %u type i2c init status is %s\n", port,
                               (TRUE == (*init_status) ? "true" : "false"));
                    break;
                }
            }
            else
            {
                if (HAL_PHY_PORT_DEV_PHY_ID(unit, port) == HAL_PHY_PORT_DEV_PHY_ID(unit, user_port))
                {
                    if (port > user_port)
                    {
                        *init_status = TRUE;
                    }
                    else
                    {
                        *init_status = FALSE;
                    }
                    DIAG_PRINT(HAL_DBG_INFO, "port %u type mdio init status is %s\n", port,
                               (TRUE == (*init_status) ? "true" : "false"));
                    break;
                }
            }
        }
    }
    return rv;
}

static void
_en8808_gphy_eee_patch(
    UI8_T  phy_base,
    UI32_T hwtrap)
{
    UI8_T  port = 1, index = 0;
    UI16_T data = 0;

    for (port = 1; port <= 8; port++)
    {
        hal_mdio_readC22(0, 0, phy_base + port, MII_BMCR, &data);
        hal_mdio_writeC22(0, 0, phy_base + port, MII_BMCR, data & ~(BMCR_PDOWN)); /* PHY power on */

        /* Change EEE RG default value */
        for (index = 0; index < TOTAL_NUMBER_OF_PATCH; index++)
        {
            hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, eee_patch_table[index][0],
                              eee_patch_table[index][1]);
        }

        hal_mdio_writeC22(0, 0, phy_base + port, RgAddr_dev1Fh, CL22_Page_TrReg); /* change CL22page to LpiReg(0x3) */
        for (index = 0; index < TOTAL_NUMBER_OF_TR; index++)
        {
            _en8808_set_gphy_TrReg(phy_base + port, tr_reg_table[index][0], tr_reg_table[index][1],
                                   tr_reg_table[index][2]);
        }

        hal_mdio_writeC22(0, 0, phy_base + port, 0x1f, CL22_Page_LpiReg); /* change CL22page to LpiReg(0x3) */
        hal_mdio_writeC22(0, 0, phy_base + port, RgAddr_LpiReg1Ch,
                          0x0c92); /* Fine turn SigDet for B2B LPI link down issue */
        hal_mdio_writeC22(0, 0, phy_base + port, RgAddr_LpiReg1Dh,
                          0x0001); /* Enable "lpi_quit_waitafesigdet_en" for LPI link down issue */
        hal_mdio_writeC22(0, 0, phy_base + port, 0x1f, CL22_Page_Reg); /* change CL22page to Reg(0x0) */

        /* GPHY Rx low pass filter */
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0xc7, 0xd000);

        /* patch for RX ADC offset */
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x171, 0x6f);

        hal_mdio_readC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x39, &data);
        data = data & ~(1 << 14);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x39, data);

        hal_mdio_readC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x107, &data);
        data = data & ~(1 << 12);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x107, data);

        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0x3d, 0xc00);
        hal_mdio_writeC22(0, 0, phy_base + port, 0x00, 0x800);
        hal_mdio_writeC22(0, 0, phy_base + port, 0x09, 0x200);
        hal_mdio_writeC22(0, 0, phy_base + port, 0x00, 0x1200);

        /* low power mode */
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x269, 0x1114);

        /* low power for 100M & 10M */
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x268, 0x0341);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x271, 0x2c65);
        hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC2, 0x302, 0x0034);

        /* patch for echo cancellation */
        if (hwtrap & HW_TRAP_SURGE_BIT)
        {
            hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0xe7, 0x6666);
        }
        else
        {
            hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0xe7, 0x4444);
        }
    }
}

static AIR_ERROR_NO_T
_en8808_phy_disabledUnusedPhy(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T phy_base,
    const UI16_T phy_num)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    UI16_T          phy_addr = 0;
    UI16_T          i2c_addr = HAL_PHY_PORT_DEV_I2C_ADDR(unit, port);
    UI16_T          phy_index = 0;
    UI16_T          port_index = 0;
    UI16_T          port_phy_addr = 0;
    UI16_T          port_i2c_addr = 0;
    UI16_T          access_type = HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port);
    UI16_T          port_access_type = 0;
    UI16_T          bus_id = HAL_PHY_PORT_DEV_I2C_BUS_ID(unit, port);
    UI16_T          reg_data = 0;
    UI8_T           used_phy_addr[PHY_MAX_NUMBER];
    UI8_T           used_phy_num = 0;
    UI8_T           temp_index = 0;
    BOOL_T          is_used = FALSE;
    AIR_CFG_VALUE_T cfg_value;

    osal_memset(used_phy_addr, 0, sizeof(used_phy_addr));
    osal_memset(&cfg_value, 0, sizeof(AIR_CFG_VALUE_T));

    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port_index)
    {
        cfg_value.value = 0;
        cfg_value.param0 = port_index;

        rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_ADDRESS, &cfg_value);
        if (AIR_E_OK == rv)
        {
            port_phy_addr = cfg_value.value;

            osal_memset(&cfg_value, 0, sizeof(AIR_CFG_VALUE_T));
            cfg_value.value = 0;
            cfg_value.param0 = port_index;

            rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_ACCESS_TYPE, &cfg_value);
            if (AIR_E_OK == rv)
            {
                port_access_type = cfg_value.value;

                osal_memset(&cfg_value, 0, sizeof(AIR_CFG_VALUE_T));
                cfg_value.value = 0;
                cfg_value.param0 = port_index;

                rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_I2C_ADDRESS, &cfg_value);
                if (AIR_E_OK == rv)
                {
                    port_i2c_addr = cfg_value.value;
                }
            }
        }

        if (AIR_E_OK == rv)
        {
            if (((HAL_PHY_ACCESS_TYPE_I2C_MDIO == access_type) && (access_type == port_access_type) &&
                 (i2c_addr == port_i2c_addr)) ||
                ((HAL_PHY_ACCESS_TYPE_MDIO == access_type) && (access_type == port_access_type) &&
                 HAL_IS_GPHY_PORT_VALID(unit, port_index)))
            {
                if (used_phy_num >= phy_num)
                {
                    DIAG_PRINT(
                        HAL_DBG_ERR,
                        "[Dbg] phy access type %d , phy number is out of bound. correct number is %d, now use %d !\n",
                        port_access_type, phy_num, used_phy_num);
                    rv = AIR_E_OTHERS;
                    break;
                }
                used_phy_addr[used_phy_num] = port_phy_addr;
                used_phy_num++;
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        port_index = port;

        for (phy_index = 1; phy_index <= phy_num; phy_index++)
        {
            phy_addr = phy_base + phy_index;

            for (temp_index = 0; temp_index < used_phy_num; temp_index++)
            {
                if (used_phy_addr[temp_index] == phy_addr)
                {
                    is_used = TRUE;
                    break;
                }
            }

            if (TRUE == is_used)
            {
                is_used = FALSE;
            }
            else
            {
                /*set phy power down*/
                /* Disable PHY digital power */
                if (HAL_PHY_ACCESS_TYPE_MDIO == access_type)
                {
                    rv = hal_mdio_readC22(unit, bus_id, phy_addr, MII_BMCR, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC22(unit, bus_id, phy_addr, MII_BMCR, (reg_data | BMCR_PDOWN));
                        /* Disable PHY analog power */
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x3c, 0xFFFF);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x3d, 0xFCFC);
                                if (AIR_E_OK == rv)
                                {
                                    rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x3e, 0xF8F8);
                                }
                            }
                        }
                    }
                }
#ifdef AIR_EN_I2C_PHY
                else
                {
                    rv = hal_mdio_readC22ByI2c(unit, bus_id, i2c_addr, phy_addr, MII_BMCR, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        rv =
                            hal_mdio_writeC22ByI2c(unit, bus_id, i2c_addr, phy_addr, MII_BMCR, (reg_data | BMCR_PDOWN));
                        /* Disable PHY analog power */
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_writeC45ByI2c(unit, bus_id, i2c_addr, phy_addr, MMD_DEV_VSPEC1, 0x3c, 0xFFFF);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_mdio_writeC45ByI2c(unit, bus_id, i2c_addr, phy_addr, MMD_DEV_VSPEC1, 0x3d,
                                                            0xFCFC);
                                if (AIR_E_OK == rv)
                                {
                                    rv = hal_mdio_writeC45ByI2c(unit, bus_id, i2c_addr, phy_addr, MMD_DEV_VSPEC1, 0x3e,
                                                                0xF8F8);
                                }
                            }
                        }
                    }
                }
#endif
            }
        }
    }

    return rv;
}

static AIR_ERROR_NO_T
_en8808_getChipLEDType(
    const UI32_T unit,
    UI32_T      *ptr_data)
{
    AIR_CFG_VALUE_T led_type;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
    rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);
    if (0 == led_type.value)
    {
        *ptr_data = LED_TYPE_SERIAL;
    }
    else
    {
        *ptr_data = LED_TYPE_PARALLEL;
    }
    return rv;
}

#ifdef AIR_EN_I2C_PHY
static AIR_ERROR_NO_T
_en8808_getComboPortLEDType(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_data)
{
    AIR_CFG_VALUE_T led_type;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
    led_type.param0 = port;
    rv = hal_cfg_getValue(0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE, &led_type);
    if (LED_TYPE_PARALLEL == led_type.value)
    {
        *ptr_data = LED_TYPE_PARALLEL;
    }
    else
    {
        *ptr_data = LED_TYPE_SERIAL;
    }
    return rv;
}

static AIR_ERROR_NO_T
_en8808_SFP_led_config(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI16_T      *ptr_link_data,
    UI16_T      *ptr_blk_data)
{
    AIR_CFG_VALUE_T led_behavior;
    UI32_T          data = 0;
    UI16_T          led_config = 0, link_reg_data = 0, blk_reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    _en8808_getComboPortLEDType(unit, port, &data);
    if (LED_TYPE_SERIAL == data)
    {
        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
        led_behavior.value = 0x0;
        led_behavior.param0 = port;
        led_behavior.param1 = led_id;

        rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR, &led_behavior);
        if (0 == led_behavior.value)
        {
            return rv;
        }
        led_config = led_behavior.value;

        link_reg_data = 0;
        blk_reg_data = 0;

        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_1000, EN8808_LED_LINK_1000, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_100, EN8808_LED_LINK_100, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_10, EN8808_LED_LINK_10, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_FULLDPLX, EN8808_LED_LINK_FULLDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_HALFDPLX, EN8808_LED_LINK_HALFDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_HIGH_ACTIVE, EN8808_LED_POL_HIGH_ACT, link_reg_data);

        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_1000, EN8808_LED_BLINK_TX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_1000, EN8808_LED_BLINK_RX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_100, EN8808_LED_BLINK_TX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_100, EN8808_LED_BLINK_RX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_10, EN8808_LED_BLINK_TX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_10, EN8808_LED_BLINK_RX_10, blk_reg_data);

        if (link_reg_data != 0 || blk_reg_data != 0)
        {
            link_reg_data |= EN8808_LED_FUNC_ENABLE;
        }
        *ptr_link_data = link_reg_data;
        *ptr_blk_data = blk_reg_data;
    }
    return rv;
}
#endif

static AIR_ERROR_NO_T
_en8808_led_config(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_behavior;
    UI32_T          data = 0;
    UI16_T          led_id = 0, led_config = 0;
    UI16_T          link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
#ifdef AIR_EN_I2C_PHY
    UI32_T offset = 0, phy_id = 0, led_pol = 0;
#endif

    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21, 0x8008);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22, 0x600);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x23, 0xc00);

    for (led_id = 0; led_id < EN8808_LED_COUNT; led_id++)
    {
        if (led_id == 0)
        {
            link_reg_addr = EN8808_LED_0_ON_MASK;
            blk_reg_addr = EN8808_LED_0_BLK_MASK;
        }
        else if (led_id == 1)
        {
            link_reg_addr = EN8808_LED_1_ON_MASK;
            blk_reg_addr = EN8808_LED_1_BLK_MASK;
        }
        else if (led_id == 2)
        {
            link_reg_addr = EN8808_LED_2_ON_MASK;
            blk_reg_addr = EN8808_LED_2_BLK_MASK;
        }
        else
        {
            link_reg_addr = EN8808_LED_3_ON_MASK;
            blk_reg_addr = EN8808_LED_3_BLK_MASK;
        }

        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

        led_behavior.value = 0xFFF;
        led_behavior.param0 = port;
        led_behavior.param1 = led_id;

        rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

        led_config = led_behavior.value;

        link_reg_data = 0;
        blk_reg_data = 0;

        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_1000, EN8808_LED_LINK_1000, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_100, EN8808_LED_LINK_100, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_10, EN8808_LED_LINK_10, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_FULLDPLX, EN8808_LED_LINK_FULLDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_LINK_HALFDPLX, EN8808_LED_LINK_HALFDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_HIGH_ACTIVE, EN8808_LED_POL_HIGH_ACT, link_reg_data);

        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_1000, EN8808_LED_BLINK_TX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_1000, EN8808_LED_BLINK_RX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_100, EN8808_LED_BLINK_TX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_100, EN8808_LED_BLINK_RX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_TX_10, EN8808_LED_BLINK_TX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8808_LED_BHV_BLINK_RX_10, EN8808_LED_BLINK_RX_10, blk_reg_data);

        if (link_reg_data != 0 || blk_reg_data != 0)
        {
            link_reg_data |= EN8808_LED_FUNC_ENABLE;
        }

        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);

        _en8808_getChipLEDType(unit, &data);
        if ((LED_TYPE_PARALLEL == data) && (0 == led_id))
        {
            /* force PHY LED active high and the polarity is controled by gpio inverter */
            link_reg_addr |= EN8808_LED_POL_HIGH_ACT;
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
            /* copy LED-0 config to LED-2 */
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8808_LED_2_ON_MASK, link_reg_data);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8808_LED_2_BLK_MASK, blk_reg_data);
        }
#ifdef AIR_EN_I2C_PHY
        if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            /* check combo mode */
            if (HAL_IS_COMBO_PORT_VALID(unit, port))
            {
                _en8808_SFP_led_config(unit, port, led_id, &link_reg_data, &blk_reg_data);

                led_pol = (link_reg_data & EN8808_LED_POL_HIGH_ACT) ? 1 : 0;
                data = ((blk_reg_data << 12) | ((link_reg_data & 0x00df) << 1) | (led_pol << SFP_LED_POL_OFF));
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                offset = phy_id * SFP_LED_CFG_OFFSET;
                if (led_id == 0)
                {
                    WRITE_EXTERNAL_EN880X_RG(SFP0_LED0_CFG_REG + offset, data);
                }
                else if (led_id == 1)
                {
                    WRITE_EXTERNAL_EN880X_RG(SFP0_LED1_CFG_REG + offset, data);
                }
                osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
                led_behavior.param0 = port;
                rv = hal_cfg_getValue(0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE, &led_behavior);
                WRITE_EXTERNAL_EN880X_RG(SFP0_LED_PIN_CFG_REG, led_behavior.value);
            }
        }
#endif
    }

    return rv;
}

static void
en8808_phy_calibration(
    UI8_T  phy_base,
    UI32_T hwtrap)
{
    UI8_T  port = 1, phy_addr = 1, phy_group = 1;
    UI16_T data = 0;

    _en8808_BG_Calibration(phy_base + 0x1, 0x1);
#if !defined(EN8804)
    _en8808_BG_Calibration(phy_base + 0x5, 0x1);
#endif

    for (port = 1; port <= TOTAL_PHYS_IN_CHIP; port++)
    {
        if (port <= 4)
        {
            phy_group = phy_base + 0x01; /* PHY group 1 */
        }
        else
        {
            phy_group = phy_base + 0x05; /* PHY group 2 */
        }
        phy_addr = phy_base + port;
        _en8808_R50_Calibration(phy_addr, phy_group);
        _en8808_TX_OFS_Calibration(phy_addr, phy_group);
        _en8808_TX_AMP_Calibration(phy_addr, phy_group);
    }

    for (port = 1; port <= TOTAL_PHYS_IN_CHIP; port++)
    {
        phy_addr = phy_base + port;
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x017d, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x017e, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x017f, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0180, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0181, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0182, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0183, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0184, 0x0000);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x00db, 0x0000); /* disable analog calibration circuit */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x00dc, 0x0000); /* disable Tx offset calibration circuit */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x003e, 0x0000); /* disable Tx VLD force mode */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x00dd, 0x0000); /* disable Tx offset/amplitude calibration circuit */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x0145, 0x1000); /* enable auto MDI/MDIX */

        hal_mdio_writeC22(0, 0, phy_addr, 0, 0x1200);
        /* GPHY Rx low pass filter */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0xc7, 0xd000);

        /* patch for RX ADC offset */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x171, 0x6f);
        hal_mdio_readC45(0, 0, phy_addr, 0x1e, 0x39, &data);
        data = data & ~(1 << 14);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1e, 0x39, data);

        hal_mdio_readC45(0, 0, phy_addr, 0x1f, 0x107, &data);
        data = data & ~(1 << 12);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1f, 0x107, data);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1f, 0x107, 0xc00);

        hal_mdio_writeC22(0, 0, phy_addr, 0x00, 0x800);
        hal_mdio_writeC22(0, 0, phy_addr, 0x09, 0x200);
        hal_mdio_writeC22(0, 0, phy_addr, 0x00, 0x1200);

        /* patch for low power - tx bias */
        hal_mdio_writeC45(0, 0, phy_addr, 0x1f, 0x268, 0x0341);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1f, 0x269, 0x1114);
        hal_mdio_writeC45(0, 0, phy_addr, 0x1f, 0x271, 0x2c65);

        /* patch for echo cancellation */
        if (hwtrap & HW_TRAP_SURGE_BIT)
        {
            hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0xe7, 0x6666);
        }
        else
        {
            hal_mdio_writeC45(0, 0, phy_base + port, MMD_DEV_VSPEC1, 0xe7, 0x4444);
        }
    }
}

#ifdef AIR_EN_I2C_PHY
static AIR_ERROR_NO_T
en8808_firmware_download(
    const UI32_T unit,
    const UI32_T port)
{
    UI8_T          sid = 1, wLoop = 0;
    UI32_T        *ptrAddr;
    UI32_T         idx = 0;
    UI32_T         crc_result = 0, data = 0, TargetAddr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    sid = HAL_PHY_PORT_DEV_I2C_ADDR(unit, port);
    if ((sid < 0x50) || (sid > 0x52))
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
        return AIR_E_BAD_PARAMETER;
    }
    /* check product ID here */
    rv = hal_mdio_readRegByI2c(unit, port, PRODUCT_ID_REG_ADDR, &data);
    if ((rv != AIR_E_OK) || (PRODUCT_ID != data))
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d check device fail !\n", port);
        return rv;
    }

    rv = hal_mdio_readRegByI2c(unit, port, CPU_CLOCK_GATING_REG, &data);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
        return rv;
    }

    if (CPU_CLOCK_ENABLE == (data & CPU_CLOCK_GATING_MASK))
    {
        /* EN8808/EN8804 CPU is enabled */
        return rv;
    }
    cmlib_crc32(en8808_fw, en8808_fw_size, &crc_result);
    DIAG_PRINT(HAL_DBG_INFO, " (%u) firmware crc32=(%x)\n", unit, crc_result);

    /* external EN8808/EN8804 bring up */
    WRITE_EXTERNAL_EN880X_RG(TIMER_CONTROL_REG, WDT_DISABLE);
    WRITE_EXTERNAL_EN880X_RG(CPU_CLOCK_GATING_REG, CPU_CLOCK_DISABLE);
    WRITE_EXTERNAL_EN880X_RG(FIRMWARE_LEN_REG, en8808_fw_size);
    WRITE_EXTERNAL_EN880X_RG(FIRMWARE_CRC_REG, crc_result);
    WRITE_EXTERNAL_EN880X_RG(XPHY_CRC_RET_REG, 0);

    TargetAddr = DMEM_ADDRESS;
    for (idx = 0; idx <= en8808_fw_size; idx += 4)
    {
        ptrAddr = (UI32_T *)&en8808_fw[idx];
        WRITE_EXTERNAL_EN880X_RG(TargetAddr, *ptrAddr);
        TargetAddr += 4;
    }
    WRITE_EXTERNAL_EN880X_RG(CPU_CLOCK_GATING_REG, CPU_CLOCK_ENABLE);
    for (wLoop = 0; wLoop < MAX_READ_CRC_RETRY; wLoop++)
    {
        osal_delayUs(100000);
        rv = hal_mdio_readRegByI2c(unit, port, XPHY_CRC_RET_REG, &data);
        if ((data != 0) && (data == crc_result))
        {
            break;
        }
    }
    if (MAX_READ_CRC_RETRY <= wLoop)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg] en8808 %d download firmware fail ! SID=%x\n", port, sid);
        return AIR_E_OP_INCOMPLETE;
    }
    if (rv == AIR_E_OK)
    {
        rv = hal_mdio_readRegByI2c(unit, port, HW_FORCE_GPIO_EN_REG, &data);
        WRITE_EXTERNAL_EN880X_RG(HW_FORCE_GPIO_EN_REG, data | EN8808_SLED_PIN_IOMUX);
        /* need config again */
        if (rv == AIR_E_OK)
        {
            rv = _en8808_led_config(unit, port);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 port %d led config fail \n", port);
                return rv;
            }
            rv = hal_mdio_readRegByI2c(unit, port, HW_FORCE_GPIO_EN_REG, &data);
            WRITE_EXTERNAL_EN880X_RG(HW_FORCE_GPIO_EN_REG, data & ~(EN8808_SLED_PIN_IOMUX));

            /* Enable HW reset de-glitch */
            data = HWRST_DEGLITCH_ENABLE;
            WRITE_EXTERNAL_EN880X_RG(HWRST_DEGLITCH_REG, data);
        }
    }
    return rv;
}
#endif

static AIR_ERROR_NO_T
_en8808_readReg(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T reg_addr,
    UI32_T      *ptr_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
    {
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, ptr_data);
    }
    else
    {
        rv = aml_readReg(unit, reg_addr, ptr_data, sizeof(UI32_T));
    }
#else
    rv = aml_readReg(unit, reg_addr, ptr_data, sizeof(UI32_T));
#endif
    return rv;
}

static AIR_ERROR_NO_T
_en8808_writeReg(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T reg_addr,
    const UI32_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

#ifdef AIR_EN_I2C_PHY
    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
    {
        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, reg_data);
    }
    else
    {
        rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    }
#else
    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
#endif
    return rv;
}

#ifdef AIR_EN_I2C_PHY
static AIR_ERROR_NO_T
_hal_en8808_phy_dumpDbgRegister(
    const UI32_T                    unit,
    const UI32_T                    port,
    const HAL_EN8808_PHY_DBG_CFG_T *ptr_serdes_cfg)
{
    UI32_T         base_addr, serdes_reg, serdes_regValue;
    UI32_T         phy_port = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* get combo port serdes base id */
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, base_addr);

    if (ptr_serdes_cfg->delay_time)
    {
        osal_delayUs(ptr_serdes_cfg->delay_time * 1000);
    }

    serdes_reg = base_addr + ptr_serdes_cfg->reg_addr;

    if (1 == ptr_serdes_cfg->operation) /* write */
    {
        rc = _en8808_readReg(unit, port, serdes_reg, &serdes_regValue);
        if (AIR_E_OK == rc)
        {
            serdes_regValue &=
                ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
            serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                          ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
            DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);
            rc = _en8808_writeReg(unit, port, serdes_reg, serdes_regValue);
        }
    }
    else /* read */
    {
        rc = _en8808_readReg(unit, port, serdes_reg, &serdes_regValue);
        if (AIR_E_OK == rc)
        {
            osal_printf("%35s       0x%8x       0x%08x \n", ptr_serdes_cfg->reg_name, serdes_reg, serdes_regValue);
        }
    }

    return rc;
}
#endif

/* FUNCTION NAME:   _hal_en8808_phy_enableEEE
 * PURPOSE:
 *      Enable EN8808 EEE
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
static AIR_ERROR_NO_T
_hal_en8808_phy_enableEEE(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Enable EEE */
    reg_data = (RG_SYSPLL_DDSFBK_EN | RG_SYSPLL_DMY1 | RG_SYSPLL_DMY2);
    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, RgAddr_dev1Fh_reg403h, reg_data);
    if (AIR_E_OK == rv)
    {
        reg_data = (EEE_1000BASE_T | EEE_100BASE_TX);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* DSP state machine FRE PM download write data */
        reg_data = 1;
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg20Bh, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        reg_data = (DSP_FRE_RP_FSM_EN | DSP_FRE_DW_AUTO_INC | DSP_FRE_WR_EN | DSP_FRE_SW_RST);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg20Eh, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        reg_data = (RG_LPI_VCO_EEE_STGO_EN | RG_LPI_TR_READY | RG_LPI_SKIP_SD_SLV_TR | VCO_SLICER_THRES_H);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg2D1h, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* Set TX_PAIR_DELAY_SEL gbe mode */
        reg_data = 0;
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg013h, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        reg_data = PICMD_MISER_MODE_INT(0x771);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, RgAddr_dev1Eh_reg14Ah, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* Set page to external reg */
        reg_data = 0x52b5;
        rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* Set LSB value */
        reg_data = 0x002b;
        rv = hal_mdio_writeC22ByPort(unit, port, RgAddr_52B5_reg11h, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* Set MSB value */
        reg_data = 0x0;
        rv = hal_mdio_writeC22ByPort(unit, port, RgAddr_52B5_reg12h, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* command: trigger & write */
        reg_data = 0x8f80;
        rv = hal_mdio_writeC22ByPort(unit, port, RgAddr_52B5_reg10h, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        /* Restore page */
        reg_data = 0x0;
        rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data);
    }

    return rv;
}

/* FUNCTION NAME:   _hal_en8808_phy_processAdv
 * PURPOSE:
 *      Process GET/SET EN8808 PHY advertisement setting
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      isGet           --  is Get operation or Set operation
 *      isLocal         --  is local or remote
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_en8808_phy_processAdv(
    const UI32_T      unit,
    const UI32_T      port,
    const BOOL_T      isGet,
    const BOOL_T      isLocal,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T            reg_data = 0, eee_reg_addr = 0, speed_reg_addr = 0, adv_reg_addr = 0, speed_ability = 0;
    UI16_T            an_complete = 0;
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    HAL_PHY_OP_MODE_T phy_mode;

    if (TRUE == isLocal)
    {
        eee_reg_addr = MMD_EEEAR;
        speed_reg_addr = MII_CTRL1000;
        adv_reg_addr = MII_ADVERTISE;
        speed_ability = ADVERTISE_1000FULL;
    }
    else
    {
        eee_reg_addr = MMD_EEELPAR;
        speed_reg_addr = MII_STAT1000;
        adv_reg_addr = MII_LPA;
        speed_ability = LPA_1000FULL;
    }
    an_complete = MII_BMSR;

    if (TRUE == isGet)
    {
        /* Read the remote advertisement ability information after AN is complete */
        rv = hal_mdio_readC22ByPort(unit, port, an_complete, &reg_data);
        if (AIR_E_OK == rv)
        {
            /* In remote mode, if AN not complete, it would not get remote-adv information */
            if ((TRUE != isLocal) && (!(reg_data & BMSR_ANEGCOMPLETE)))
            {
                ptr_adv->flags = 0;
                return AIR_E_OK;
            }
            else
            {
                rv = hal_mdio_readC22ByPort(unit, port, speed_reg_addr, &reg_data);
                if (AIR_E_OK == rv)
                {
                    GET_PHY_ABILITY(reg_data, speed_ability, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);

                    rv = hal_mdio_readC22ByPort(unit, port, adv_reg_addr, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        if (TRUE == isLocal)
                        {
                            GET_PHY_ABILITY(reg_data, ADVERTISE_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, ADVERTISE_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, ADVERTISE_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, ADVERTISE_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE,
                                            ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE,
                                            ptr_adv->flags);
                        }
                        else
                        {
                            GET_PHY_ABILITY(reg_data, LPA_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, LPA_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, LPA_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, LPA_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, LPA_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
                            GET_PHY_ABILITY(reg_data, LPA_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
                        }

                        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, eee_reg_addr, &reg_data);
                        if (AIR_E_OK == rv)
                        {
                            GET_PHY_ABILITY(reg_data, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE,
                                            ptr_adv->flags);
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE)
        {
            rv = hal_en8808_phy_getPhyOpMode(unit, port, &phy_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_LONG_REACH == phy_mode))
            {
                rv = AIR_E_OP_INVALID;
            }
        }

        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);
            if (AIR_E_OK == rv)
            {
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ADVERTISE_1000FULL, reg_data);

                rv = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, reg_data);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ADVERTISE_PAUSE_ASYM,
                                        reg_data);
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ADVERTISE_PAUSE_CAP, reg_data);
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100FUDX, ADVERTISE_100FULL, reg_data);
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100HFDX, ADVERTISE_100HALF, reg_data);
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10FUDX, ADVERTISE_10FULL, reg_data);
                        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10HFDX, ADVERTISE_10HALF, reg_data);

                        rv = hal_mdio_writeC22ByPort(unit, port, MII_ADVERTISE, reg_data);
                    }

                    if (AIR_E_OK == rv)
                    {
                        if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE)
                        {
                            rv = _hal_en8808_phy_enableEEE(unit, port);
                        }
                        else
                        {
                            /* Disable EEE */
                            reg_data = 0;
                            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_data);
                        }
                    }
                }
            }
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_en8808_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (AIR_E_OK != (rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)))
    {
        return rv;
    }

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        reg_data &= ~(BMCR_PDOWN);
    }
    else
    {
        reg_data |= BMCR_PDOWN;
    }

    rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    return rv;
}

static AIR_ERROR_NO_T
_hal_en8808_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (AIR_E_OK != (rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)))
    {
        return rv;
    }

    if (reg_data & BMCR_PDOWN)
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
    }
    else
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
    }
    return rv;
}

/* FUNCTION NAME:   _hal_en8808_phy_restoreConfigFromEfuse
 * PURPOSE:
 *      Restore EN8808 PHY config from efuse
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
static AIR_ERROR_NO_T
_hal_en8808_phy_restoreConfigFromEfuse(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         data = 0, timeout = MAX_READ_EFUSE_RETRY;
    UI32_T         efuse_data = 0, phy_addr = 0, phy_port = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    phy_port = ((phy_addr - 1) & 0x07);

    efuse_data = ((phy_port * 2) << 20) + EFUSE_GPHY_BASE;

    rv = _en8808_writeReg(unit, port, EN8808_EFUSE_CTRL, efuse_data);
    if (AIR_E_OK == rv)
    {
        while (timeout)
        {
            osal_delayUs(1000);
            rv = _en8808_readReg(unit, port, EN8808_EFUSE_CTRL, &efuse_data);
            if ((efuse_data & EFSROM_KICK) == 0)
            {
                break;
            }
            timeout--;
        }
    }

    if (0 == timeout)
    {
        rv = AIR_E_TIMEOUT;
    }

    if (AIR_E_OK == rv)
    {
        rv |= _en8808_readReg(unit, port, EN8808_EFUSE_RDATA3, &efuse_data);

        /* tx_i2mpb_a_tbt */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x12, &data);
        data &= ~(BITS(0, 5));
        data |= BITS_OFF_R(efuse_data, 16, 6);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x12, data);

        /* tx_i2mpb_b_tbt */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x17, &data);
        data &= ~(BITS(0, 5));
        data |= BITS_OFF_R(efuse_data, 24, 6);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x17, data);

        /* tx_i2mpb_a_hbt */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x16, &data);
        data &= ~(BITS(10, 15));
        data |= BITS_OFF_L(efuse_data, 10, 6);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x16, data);

        /* tx_i2mpb_b_hbt */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x18, &data);
        data &= ~(BITS(8, 13));
        data |= (efuse_data & BITS(8, 13));
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x18, data);

        rv |= _en8808_readReg(unit, port, EN8808_EFUSE_RDATA1, &efuse_data);

        /* tx_i2mp_a_gbe */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x12, &data);
        data &= ~(BITS(10, 15));
        data |= BITS_OFF_L(efuse_data, 10, 6);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x12, data);

        /* tx_i2mp_b_gbe */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x17, &data);
        data &= ~(BITS(8, 13));
        data |= (efuse_data & BITS(8, 13));
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x17, data);

        /* tx_i2mp_c_gbe */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x19, &data);
        data &= ~(BITS(8, 13));
        data |= ((efuse_data >> 8) & BITS_RANGE(8, 13));
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x19, data);

        /* tx_i2mp_d_gbe */
        rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x21, &data);
        data &= ~(BITS(8, 13));
        data |= ((efuse_data >> 16) & BITS_RANGE(8, 13));
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x21, data);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_en8808_phy_applyParam
 * PURPOSE:
 *      Apply EN8808 PHY parameters
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
static AIR_ERROR_NO_T
_hal_en8808_phy_applyParam(
    const UI32_T        unit,
    const UI32_T        port,
    const HAL_PHY_CFG_T phy_cfg)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         data = 0, check_data = 0;

    if (phy_cfg.delay_time)
    {
        osal_delayUs(phy_cfg.delay_time * 1000);
    }

    if ((HAL_PHY_MAX_DATA_MSB == phy_cfg.data_msb) && (HAL_PHY_MIN_DATA_LSB == phy_cfg.data_lsb))
    {
        if (HAL_PHY_ACCESS_METHOD_CL22 == phy_cfg.access_method)
        {
            rv = hal_mdio_writeC22ByPort(unit, port, phy_cfg.reg_addr, phy_cfg.reg_data);
        }
        else
        {
            rv = hal_mdio_writeC45ByPort(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, phy_cfg.reg_data);
        }
    }
    else
    {
        if (HAL_PHY_ACCESS_METHOD_CL22 == phy_cfg.access_method)
        {
            rv = hal_mdio_readC22ByPort(unit, port, phy_cfg.reg_addr, &data);
        }
        else
        {
            rv = hal_mdio_readC45ByPort(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, &data);
        }

        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_ACCESS_METHOD_CL45_INC == phy_cfg.access_method)
            {
                check_data = BITS_OFF_R(data, phy_cfg.data_lsb, (phy_cfg.data_msb - phy_cfg.data_lsb));
                if (check_data > 0x3f)
                {
                    data &= ~(BITS(phy_cfg.data_lsb, phy_cfg.data_msb));
                    data |= BITS_OFF_L(0x3f, phy_cfg.data_lsb, (phy_cfg.data_msb - phy_cfg.data_lsb));
                }
                else
                {
                    data += (phy_cfg.reg_data << phy_cfg.data_lsb);
                }
            }
            else
            {
                data &= ~(BITS(phy_cfg.data_lsb, phy_cfg.data_msb));
                data |= (phy_cfg.reg_data << phy_cfg.data_lsb);
            }

            if (HAL_PHY_ACCESS_METHOD_CL22 == phy_cfg.access_method)
            {
                rv = hal_mdio_writeC22ByPort(unit, port, phy_cfg.reg_addr, data);
            }
            else
            {
                rv = hal_mdio_writeC45ByPort(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, data);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   _hal_en8808_phy_setPhyOpMode
 * PURPOSE:
 *      Set EN8808 PHY operation mode
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
static AIR_ERROR_NO_T
_hal_en8808_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T   rv = AIR_E_OK;
    UI32_T           count = 0;
    HAL_PHY_CFG_T    phy_cfg;
    HAL_PHY_AN_ADV_T hal_adv;

    osal_memset(&phy_cfg, 0, sizeof(HAL_PHY_CFG_T));

    if (HAL_PHY_OP_MODE_NORMAL == mode)
    {
        for (count = 0; count < _hal_en8808_normal_size; count++)
        {
            if (AIR_E_OK == rv)
            {
                phy_cfg = _hal_en8808_normal[count];
                rv = _hal_en8808_phy_applyParam(unit, port, phy_cfg);
            }
        }

        if (AIR_E_OK == rv)
        {
            rv = _hal_en8808_phy_restoreConfigFromEfuse(unit, port);
        }
    }
    else if (HAL_PHY_OP_MODE_LONG_REACH == mode)
    {
        rv = hal_en8808_phy_getLocalAdvAbility(unit, port, &hal_adv);
        if (AIR_E_OK == rv)
        {
            if (hal_adv.flags & HAL_PHY_AN_ADV_FLAGS_EEE)
            {
                rv = AIR_E_OP_INVALID;
            }
            else
            {
                for (count = 0; count < _hal_en8808_longreach_size; count++)
                {
                    if (AIR_E_OK == rv)
                    {
                        phy_cfg = _hal_en8808_longreach[count];
                        rv = _hal_en8808_phy_applyParam(unit, port, phy_cfg);
                    }
                }
            }
        }
    }
    else
    {
        rv = AIR_E_BAD_PARAMETER;
    }

    return rv;
}

static AIR_ERROR_NO_T
_en8808_init_config(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          data = 0;
    AIR_CFG_VALUE_T led_behavior;
    UI32_T          phy_base = 0;
    UI32_T          random_seed = 0, seed = 0;
    UI32_T          phy_addr = 0;
    UI16_T          reg_data = 0, bus_id = HAL_PHY_PORT_DEV_I2C_BUS_ID(unit, port);
    BOOL_T          init_status = FALSE;
#ifdef AIR_EN_I2C_PHY
    UI32_T          phy_num = 0;
    AIR_CFG_VALUE_T mdio_clock;
    AIR_CFG_VALUE_T sled;
#endif

    rv = _en8808_readReg(unit, port, ECO_ID_REG_ADDR, &data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_PORT_DEV_REVISION_ID(unit, port) != data)
        {
            HAL_PHY_PORT_DEV_REVISION_ID(unit, port) = data;
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = _hal_en8808_phy_checkInitDone(unit, port, &init_status);
        if ((AIR_E_OK == rv) && (TRUE == init_status))
        {
            return rv;
        }
    }
    if (AIR_E_OK == rv)
    {
#ifdef AIR_EN_I2C_PHY
        if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            /* setup mdio clock */
            osal_memset(&mdio_clock, 0, sizeof(AIR_CFG_VALUE_T));
            mdio_clock.value = 0;
            hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_CLOCK, &mdio_clock);
            rv = hal_mdio_readRegByI2c(unit, port, PHY_SMI, &data);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d MDC initialize fail !\n", port);
                return rv;
            }
            data = (data & ~(CSR_SMI_PMDC_MASK)) | ((mdio_clock.value & 0x03) << CSR_SMI_PMDC_OFFT);
            WRITE_EXTERNAL_EN880X_RG(PHY_SMI, data);

            /* setup serial LED */
            osal_memset(&sled, 0, sizeof(AIR_CFG_VALUE_T));
            sled.value = 2;
            hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &sled);
            data = (sled.value << 1) + EN8808_SLED_RISING_EDGE + EN8808_SLED_OUTPUT_TRANSITION_MODE;
            WRITE_EXTERNAL_EN880X_RG(EN8808_SLED_CTRL0, data);
            data = (16000 << 16) + 12;
            WRITE_EXTERNAL_EN880X_RG(EN8808_SLED_CTRL1, data);

            /* init wave-gen and flash map */
            rv = hal_mdio_readRegByI2c(unit, port, CYCLE_CFG_VALUE1_REG, &data);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
                return rv;
            }
            data = (data & 0x000000ff) | WAVE_GEN_CYCLE_567;
            WRITE_EXTERNAL_EN880X_RG(CYCLE_CFG_VALUE1_REG, data);

            rv = hal_mdio_readRegByI2c(unit, port, GPIO_FLASH_MAP_CFG1_REG, &data);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
                return rv;
            }
            data = (data & 0x0000ffff) | (FLASH_MAP_ID_PATT_1 << 16) | (FLASH_MAP_ID_PATT_0 << 24);
            WRITE_EXTERNAL_EN880X_RG(GPIO_FLASH_MAP_CFG1_REG, data);

            WRITE_EXTERNAL_EN880X_RG(GPIO_FLASH_PRD_SET3_REG, PATTERN_HZ_HALF | (PATTERN_HZ_HALF << 16));

            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
            led_behavior.value = 0xFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = 0;
            rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
            if (led_behavior.value & (1 << 11))
            {
                /* high active */
                data = (PATTERN_OFF << 16) | PATTERN_ON;
            }
            else
            {
                /* low active */
                data = (PATTERN_ON << 16) | PATTERN_OFF;
            }
            WRITE_EXTERNAL_EN880X_RG(GPIO_FLASH_PRD_SET2_REG, data);

            rv = hal_mdio_readRegByI2c(unit, port, SCREG_WF1, &data);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
                return rv;
            }
            phy_num = ((data & SKU_ID_MASK) == SKU_ID_EN8808) ? EN8808_PHY_NUMBER : EN8804_PHY_NUMBER;

            rv = hal_mdio_readRegByI2c(unit, port, HW_TRAP_READ_REG, &data);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail !\n", port);
                return rv;
            }
            phy_base = (data >> PHY_BASE_ADDR_SHIFT) & PHY_BASE_ADDR_MASK; /* get PHY base address */

            rv = _en8808_phy_disabledUnusedPhy(unit, port, phy_base, phy_num);
        }
        else
#endif
        {
            /* init wave-gen and flash map */
            _en8808_readReg(unit, port, CYCLE_CFG_VALUE1_REG, &data);
            data = (data & 0x000000ff) | WAVE_GEN_CYCLE_567;
            _en8808_writeReg(unit, port, CYCLE_CFG_VALUE1_REG, data);

            _en8808_readReg(unit, port, GPIO_FLASH_MAP_CFG1_REG, &data);
            data = (data & 0x0000ffff) | (FLASH_MAP_ID_PATT_1 << 16) | (FLASH_MAP_ID_PATT_0 << 24);
            _en8808_writeReg(unit, port, GPIO_FLASH_MAP_CFG1_REG, data);
            _en8808_writeReg(unit, port, GPIO_FLASH_PRD_SET3_REG, PATTERN_HZ_HALF | (PATTERN_HZ_HALF << 16));
            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
            led_behavior.value = 0xFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = 0;
            rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
            if (led_behavior.value & (1 << 11))
            {
                /* high active */
                data = (PATTERN_OFF << 16) | PATTERN_ON;
            }
            else
            {
                /* low active */
                data = (PATTERN_ON << 16) | PATTERN_OFF;
            }
            _en8808_writeReg(unit, port, GPIO_FLASH_PRD_SET2_REG, data);

            rv = aml_readReg(unit, HW_TRAP_READ_REG, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail \n", port);
                return rv;
            }

            phy_base = (data >> PHY_BASE_ADDR_SHIFT) & PHY_BASE_ADDR_MASK; /* get PHY base address */

            rv = _hal_en8808_phy_getRandomSeed(unit, port, &random_seed);
            if (AIR_E_OK == rv)
            {
                /* set random seed */
                for (phy_addr = (phy_base + 1); phy_addr <= (phy_base + 8); phy_addr++)
                {
                    rv = hal_mdio_readC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x12, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        random_seed += reg_data;
                    }
                }
                osal_srand(random_seed);

                for (phy_addr = port; phy_addr < (port + 8); phy_addr++)
                {
                    rv |= hal_mdio_writeC22(unit, bus_id, phy_addr, MII_BMCR, 0x200); /* gphy power up */
                }

                for (phy_addr = (phy_base + 1); phy_addr <= (phy_base + 8); phy_addr++)
                {
                    seed = osal_rand() % 100000;
                    rv |= hal_mdio_readC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, &reg_data);
                    reg_data &= ~(0xF);
                    reg_data |= (0x10 | (seed & 0xF)); /* use seed 0~3bit as random seed */

                    rv |= hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, reg_data);
                    osal_delayUs(seed);
                    rv |= hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, reg_data & 0xFFEF);

                    rv |= hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x145, 0x1000);
                    rv |= hal_mdio_writeC22(unit, bus_id, phy_addr, MII_BMCR, 0x1200); /* re-AN */
                }
            }

            if (AIR_E_OK == rv)
            {
                rv = _en8808_phy_disabledUnusedPhy(unit, port, phy_base, EN8808_PHY_NUMBER);
            }
        }
        if (AIR_E_OK == rv)
        {
            /* I2C use 4MA current driving capability */
            rv = _en8808_readReg(unit, port, EN8808_I2C_E4_TX_DRIVING_CTRL, &data);
            if (AIR_E_OK == rv)
            {
                data &= ~EN8808_I2C_IO_E4_BITS;
                rv = _en8808_writeReg(unit, port, EN8808_I2C_E4_TX_DRIVING_CTRL, data);
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_init
 * PURPOSE:
 *      EN8808 PHY initialization
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
hal_en8808_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
    static BOOL_T cal_done = TRUE;
#else
    static BOOL_T cal_done = FALSE;
#endif
    UI32_T         phy_base = 0;
    UI32_T         data = 0;
    UI16_T         reg_data = 0;
    UI8_T          phyAddr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI8_T accessType = 0;
#endif

    phyAddr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    if (phyAddr >= PHY_MAX_NUMBER)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail \n", port);
        return AIR_E_BAD_PARAMETER;
    }

    rv = _en8808_led_config(unit, port);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 port %d led config fail \n", port);
        return rv;
    }

#ifdef AIR_EN_I2C_PHY
    accessType = HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port);
    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == accessType)
    {
        rv = en8808_firmware_download(unit, port);
        if (AIR_E_OK == rv)
        {
            rv = _en8808_init_config(unit, port);
            rv |= _hal_en8808_phy_setPhyOpMode(unit, port, HAL_PHY_OP_MODE_NORMAL);
            /* Rx ADC re-calibration */
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x000f);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x200f);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x000f);
            /* PHY power off */
            rv |= hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data | BMCR_PDOWN);
        }
        return rv;
    }
    else if (HAL_PHY_ACCESS_TYPE_MDIO != accessType)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail \n", port);
        return AIR_E_BAD_PARAMETER;
    }
#endif

    rv = aml_readReg(unit, HW_TRAP_READ_REG, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8808 %d initialize fail \n", port);
        return rv;
    }

    phy_base = (data >> PHY_BASE_ADDR_SHIFT) & PHY_BASE_ADDR_MASK; /* get PHY base address */
    if ((phy_base == ((phyAddr - 1) & PHY_BASE_ADDR_MASK)) && (FALSE == cal_done))
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) phy_base=(%x)\n", unit, phy_base);
        if (HAL_DEVICE_REV_ID(unit) < 3)
        {
            /* software calibration for E2 */
            en8808_phy_calibration(phy_base, data);
            _en8808_phy_config(phy_base);
        }
        else
        {
            /* internal rom calibration for E3*/
            _en8808_gphy_eee_patch(phy_base, data);
        }
        cal_done = TRUE;
    }

    rv = _en8808_init_config(unit, port);
    if (AIR_E_OK == rv)
    {
        rv = _hal_en8808_phy_setPhyOpMode(unit, port, HAL_PHY_OP_MODE_NORMAL);
        /* Rx ADC re-calibration */
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x000f);
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x200f);
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x039, 0x000f);
        /* PHY power off */
        rv |= hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
        rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data | BMCR_PDOWN);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setAdminState
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
hal_en8808_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI16_T                phy_port = 0;
    UI32_T                serdes_base = 0;
    UI32_T                data = 0, speed = 0, force_mode = 0;
    UI32_T                reg_addr = 0, sideband_speed = 0;
    UI32_T                efifo_mode_addr = 0, efifo_mode_val = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK == rv)
                {
                    /* get combo port serdes base id */
                    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                    PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, serdes_base);

                    reg_addr = serdes_base + SERDES_PHYA_REG_61;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (AIR_E_OK == rv)
                    {
                        if (HAL_PHY_ADMIN_STATE_ENABLE == state)
                        {
                            data &= ~(SERDES_POWER_DOWN | SERDES_TX_DISABLE);
                        }
                        else
                        {
                            data |= SERDES_POWER_DOWN;
                            if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                            {
                                data |= SERDES_TX_DISABLE;
                            }
                        }
                        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);
                    }

                    if (AIR_E_OK == rv)
                    {
                        reg_addr = serdes_base + SERDES_PCS_CTRL_REG_1;
                        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                        if (AIR_E_OK == rv)
                        {
                            if (HAL_PHY_ADMIN_STATE_ENABLE == state)
                            {
                                data &= ~(SERDES_FORCE_SYNC_DISABLE);
                            }
                            else
                            {
                                data |= SERDES_FORCE_SYNC_DISABLE;
                            }
                            rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);
                        }
                    }

                    if (port_cfg.value == 1)
                    {
                        reg_addr = serdes_base + SERDES_AN_REG_13;
                        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                        if (rv == AIR_E_OK)
                        {
                            force_mode = BITS_OFF_R(data, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);

                            /* Only for force mode QSGMII AN speed sync */
                            if ((HAL_PHY_ADMIN_STATE_ENABLE == state) && (force_mode & REG_IF_MODE_FORCE))
                            {
                                rv = hal_en8808_phy_getSpeed(unit, port, &speed);
                                if (AIR_E_OK == rv)
                                {
                                    switch (speed)
                                    {
                                        case HAL_PHY_SPEED_10M:
                                            sideband_speed = 0x5001;
                                            efifo_mode_val = EFIFO_MODE_100_10;
                                            break;
                                        case HAL_PHY_SPEED_100M:
                                            sideband_speed = 0x5401;
                                            efifo_mode_val = EFIFO_MODE_100_10;
                                            break;
                                        case HAL_PHY_SPEED_1000M:
                                            sideband_speed = 0x5801;
                                            efifo_mode_val = EFIFO_MODE_1000;
                                            break;
                                        default:
                                            return AIR_E_BAD_PARAMETER;
                                    }
                                    /* SYNC speed to system side (QSGMII AN) */
                                    reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_4) + (phy_port * QSGMII_LANE_OFFSET);
                                    WRITE_EXTERNAL_EN880X_RG(reg_addr, sideband_speed);

                                    reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_0) + (phy_port * QSGMII_LANE_OFFSET);
                                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                                    if (rv == AIR_E_OK)
                                    {
                                        /* Remote Efifo setting */
                                        efifo_mode_addr = EFIFO_MODE_REG + (phy_port * EFIFO_REG_OFFSET);
                                        DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write 8808 Efifo 0x%08X value to 0x%08X \n",
                                                   efifo_mode_addr, efifo_mode_val);
                                        WRITE_EXTERNAL_EN880X_RG(efifo_mode_addr, efifo_mode_val);

                                        /* reAN system side QSGMII AN */
                                        data |= SERDES_AN_REG_0_ANRESTART;
                                        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);
                                    }
                                }
                                else
                                {
                                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read 8808 speed fali rv = %d\n", rv);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                rv = _hal_en8808_phy_setAdminState(unit, port, state);
            }
        }
    }
    else
    {
        rv = _hal_en8808_phy_setAdminState(unit, port, state);
    }
#else
    rv = _hal_en8808_phy_setAdminState(unit, port, state);
#endif
    return rv;
}
/* FUNCTION NAME:   hal_en8808_phy_getAdminState
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
hal_en8808_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI16_T                phy_port = 0;
    UI32_T                serdes_base = 0;
    UI32_T                data = 0;
    UI32_T                reg_addr = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK == rv)
                {
                    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                    PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, serdes_base);
                    reg_addr = serdes_base + SERDES_PHYA_REG_61;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (AIR_E_OK == rv)
                    {
                        if (data & SERDES_POWER_DOWN)
                        {
                            *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
                        }
                        else
                        {
                            *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
                        }
                    }
                }
            }
            else
            {
                rv = _hal_en8808_phy_getAdminState(unit, port, ptr_state);
            }
        }
    }
    else
    {
        rv = _hal_en8808_phy_getAdminState(unit, port, ptr_state);
    }
#else
    rv = _hal_en8808_phy_getAdminState(unit, port, ptr_state);
#endif
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setAutoNego
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
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8808_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    UI32_T                reg_addr = 0, i = 0, phy_id = 0;
    UI32_T                serdes_base = 0, serdes_data = 0, data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode) /* combo mode = serdes*/
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (rv != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
                    return rv;
                }

                if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
                {
                    return AIR_E_NOT_SUPPORT;
                }

                /* get combo port serdes base id */
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);

                if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
                {
                    /* AN bit enable */
                    reg_addr = serdes_base + SERDES_AN_REG_0;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (rv == AIR_E_OK)
                    {
                        data |= SERDES_AN_REG_0_ANENABLE;
                        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);

                        if (rv == AIR_E_OK)
                        {
                            if (port_cfg.value == 1) /* force rate adaption */
                            {
                                if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
                                {
                                    /* Set SGMII AN serdes RG configuration */
                                    for (i = 0; i < NUMBER_OF_SERDES_FORCE_SPEED_RG; i++)
                                    {
                                        reg_addr =
                                            serdes_base + _hal_sco_serdes_force_speed[i][SERDES_SGMII_REGISTER_ADDRESS];
                                        serdes_data = _hal_sco_serdes_force_speed[i][SERDES_SGMII_AN];
                                        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, serdes_data);
                                        if (rv != AIR_E_OK)
                                        {
                                            DIAG_PRINT(HAL_DBG_WARN,
                                                       "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr,
                                                       serdes_data, rv);
                                            return rv;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if (HAL_PHY_AUTO_NEGO_DISABLE == auto_nego)
                {
                    /* AN bit disable*/
                    reg_addr = serdes_base + SERDES_AN_REG_0;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (rv == AIR_E_OK)
                    {
                        data &= ~(SERDES_AN_REG_0_ANENABLE);
                        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);
                        if (rv != AIR_E_OK)
                        {
                            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr,
                                       data, rv);
                            return rv;
                        }
                    }
                }
                else
                {
                    /* AN bit restart */
                    reg_addr = serdes_base + SERDES_AN_REG_0;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (rv == AIR_E_OK)
                    {
                        data |= SERDES_AN_REG_0_ANRESTART;
                        rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);
                        if (rv != AIR_E_OK)
                        {
                            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr,
                                       data, rv);
                            return rv;
                        }
                    }
                }
                return rv;
            }
        }
        else
        {
            return rv;
        }
    }
#endif

    if (AIR_E_OK != (rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)))
    {
        return rv;
    }

    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        reg_data |= BMCR_ANENABLE;
    }
    else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
    {
        reg_data |= BMCR_ANRESTART;
    }
    else
    {
        reg_data &= ~(BMCR_ANENABLE);
    }

    rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getAutoNego
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
 *      AIE_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8808_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

#ifdef AIR_EN_I2C_PHY
    UI32_T                phy_id = 0, serdes_base = 0, reg_addr = 0, data = 0;
    UI32_T                combo_mode = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (rv != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
                    return rv;
                }

                if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
                {
                    return AIR_E_NOT_SUPPORT;
                }

                /* get combo port  serdes base id */
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);

                reg_addr = serdes_base + SERDES_AN_REG_13;
                rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                if (rv != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
                    return rv;
                }
                if (((data & 0x3f) == REG_IF_MODE_FORCE) && (port_cfg.value == 1)) /* force rate adaption */
                {
                    *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
                }
                else
                {
                    reg_addr = serdes_base + SERDES_AN_REG_0;
                    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv,
                                   data);
                        return rv;
                    }
                    if (data & SERDES_AN_REG_0_ANENABLE)
                    {
                        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
                    }
                    else
                    {
                        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
                    }
                }
                return rv;
            }
        }
    }
#endif

    if (AIR_E_OK != (rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)))
    {
        return rv;
    }

    if (reg_data & BMCR_ANENABLE)
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
    }
    else
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setLocalAdvAbility
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
hal_en8808_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T   rv = AIR_E_OK;
    BOOL_T           isGet = FALSE;
    BOOL_T           isLocal = TRUE;
    HAL_PHY_AN_ADV_T set_adv = *ptr_adv;

#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                data = 0;
    UI16_T                phy_port = 0;
    UI32_T                reg_addr = 0, serdes_base = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK == rv)
                {
                    if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                    {
                        if ((ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100FUDX) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100HFDX) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10HFDX) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10FUDX) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10FUDX) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) ||
                            (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_2500M))
                        {
                            rv = AIR_E_NOT_SUPPORT;
                        }
                        else if (!(ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_1000FUDX))
                        {
                            rv = AIR_E_BAD_PARAMETER;
                        }
                        else
                        {
                            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                            PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, serdes_base);
                            /* SERDES_AN_REG_4 */
                            reg_addr = serdes_base + SERDES_AN_REG_4;
                            rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                            if (AIR_E_OK == rv)
                            {
                                data &= ~(BIT(REG_1000BASEX_PAUSE_OFFT));
                                data &= ~(BIT(REG_1000BASEX_ASM_DIR_OFFT));
                                if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE)
                                {
                                    data |= BIT(REG_1000BASEX_PAUSE_OFFT);
                                }
                                if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE)
                                {
                                    data |= BIT(REG_1000BASEX_ASM_DIR_OFFT);
                                }
                                rv = hal_mdio_writeRegByI2c(unit, port, reg_addr, data);
                            }
                        }
                    }
                    else
                    {
                        rv = AIR_E_NOT_SUPPORT;
                    }
                }
            }
            else
            {
                rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, &set_adv);
            }
        }
    }
    else
    {
        rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, &set_adv);
    }
#else
    rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, &set_adv);
#endif
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getLocalAdvAbility
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
hal_en8808_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    BOOL_T         isGet = TRUE;
    BOOL_T         isLocal = TRUE;

    ptr_adv->flags = 0;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                data = 0;
    UI16_T                phy_port = 0;
    UI32_T                reg_addr = 0, serdes_base = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK == rv)
                {
                    if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                    {
                        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, serdes_base);
                        /* SERDES_AN_REG_4 */
                        reg_addr = serdes_base + SERDES_AN_REG_4;
                        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                        if (AIR_E_OK == rv)
                        {
                            ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
                            ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
                            if (data & BIT(REG_1000BASEX_PAUSE_OFFT))
                            {
                                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE;
                            }
                            if (data & BIT(REG_1000BASEX_ASM_DIR_OFFT))
                            {
                                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE;
                            }
                            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
                        }
                    }
                    else
                    {
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100HFDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10HFDX;
                    }
                }
            }
            else
            {
                rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
            }
        }
    }
    else
    {
        rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
    }
#else
    rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
#endif
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getRemoteAdvAbility
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
hal_en8808_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    BOOL_T         isGet = TRUE;
    BOOL_T         isLocal = FALSE;

    ptr_adv->flags = 0;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                data = 0;
    UI16_T                phy_port = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK == rv)
                {
                    if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                    {
                        rv = hal_mdio_readRegByI2c(unit, port, SFP_LINK_STATUS_REG, &data);
                        if (AIR_E_OK == rv)
                        {
                            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                            ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
                            ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
                            if (SFP_PAUSE_ADV == GET_SFP_LP_PAUSE_ADV(data, phy_port))
                            {
                                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE;
                            }
                            if (SFP_PAUSE_ADV == GET_SFP_LP_ASM_PAUSE_ADV(data, phy_port))
                            {
                                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE;
                            }
                            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
                        }
                    }
                    else
                    {
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100HFDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10FUDX;
                        ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10HFDX;
                    }
                }
            }
            else
            {
                rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
            }
        }
    }
    else
    {
        rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
    }
#else
    rv = _hal_en8808_phy_processAdv(unit, port, isGet, isLocal, ptr_adv);
#endif
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setSpeed
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
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8808_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                reg_addr = 0, i = 0, phy_id = 0, data = 0;
    UI32_T                serdes_base = 0, force_speed = 0, serdes_data = 0;
    UI32_T                sideband_speed = 0, auto_nego = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK != rv)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
                    return rv;
                }
                if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                {
                    if (HAL_PHY_SPEED_1000M != speed)
                    {
                        return AIR_E_OP_INVALID;
                    }
                    return rv;
                }
                if (port_cfg.value == 1) /* force rare adaption */
                {
                    if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
                    {
                        if ((rv = hal_en8808_phy_getAutoNego(unit, port, &auto_nego)) != AIR_E_OK)
                        {
                            return rv;
                        }
                        if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
                        {
                            DIAG_PRINT(HAL_DBG_WARN, "Port %u AN mode is enable, not allow force speed setting\n",
                                       port);
                            return AIR_E_NOT_SUPPORT;
                        }

                        /* get combo port  serdes base id */
                        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);

                        switch (speed)
                        {
                            case HAL_PHY_SPEED_10M:
                                force_speed = SERDES_SGMII_FORCE_10M;
                                sideband_speed = 0x5001;
                                break;
                            case HAL_PHY_SPEED_100M:
                                force_speed = SERDES_SGMII_FORCE_100M;
                                sideband_speed = 0x5401;
                                break;
                            case HAL_PHY_SPEED_1000M:
                                force_speed = SERDES_SGMII_FORCE_1000M;
                                sideband_speed = 0x5801;
                                break;
                            default:
                                return AIR_E_BAD_PARAMETER;
                        }
                        for (i = 0; i < NUMBER_OF_SERDES_FORCE_SPEED_RG; i++)
                        {
                            reg_addr = serdes_base + _hal_sco_serdes_force_speed[i][SERDES_SGMII_REGISTER_ADDRESS];
                            serdes_data = _hal_sco_serdes_force_speed[i][force_speed];
                            WRITE_EXTERNAL_EN880X_RG(reg_addr, serdes_data);
                        }

                        /* SYNC en8804 speed to system side (QSGMII AN ability 0x10) */
                        reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_4) + (phy_id * QSGMII_LANE_OFFSET);
                        WRITE_EXTERNAL_EN880X_RG(reg_addr, sideband_speed);

                        /* reAN */
                        reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_0) + (phy_id * QSGMII_LANE_OFFSET);
                        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                        if (rv != AIR_E_OK)
                        {
                            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv,
                                       data);
                            return rv;
                        }
                        /* reAN system side QSGMII AN */
                        data |= SERDES_AN_REG_0_ANRESTART;
                        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);
                        return rv;
                    }
                    else
                    {
                        return AIR_E_NOT_SUPPORT;
                    }
                }
                else if (port_cfg.value == 2)
                {
                    return AIR_E_NOT_SUPPORT;
                }
                return rv;
            }
        }
        else
        {
            return rv;
        }
    }

#endif

    if (speed >= HAL_PHY_SPEED_1000M)
    {
        return AIR_E_OP_INVALID;
    }

    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    reg_data &= ~(BMCR_SPEED1000 | BMCR_SPEED100);
    switch (speed)
    {
        case HAL_PHY_SPEED_100M:
            reg_data |= BMCR_SPEED100;
            break;
        case HAL_PHY_SPEED_10M:
            reg_data |= 0;
            break;
        default:
            rv = AIR_E_NOT_SUPPORT;
            break;
    }
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getSpeed
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
hal_en8808_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_COMBO_MODE_T  combo_mode;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                reg_addr = 0, phy_id = 0, data = 0;
    UI32_T                serdes_base = 0, force_speed = 0;
    UI32_T                force_mode = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK != rv)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
                    return rv;
                }

                if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                {
                    *ptr_speed = HAL_PHY_SPEED_1000M;
                    return rv;
                }

                if (port_cfg.value == 1) /* force rate apaption */
                {
                    if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
                    {
                        /* get combo port  serdes base id */
                        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);

                        reg_addr = serdes_base + SERDES_AN_REG_13;
                        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                        if (rv != AIR_E_OK)
                        {
                            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv,
                                       data);
                            return rv;
                        }
                        force_mode = BITS_OFF_R(data, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
                        if (force_mode & REG_IF_MODE_FORCE)
                        {
                            reg_addr = serdes_base + SERDES_RATE_ADP_P0_CTRL_REG_0;
                            rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
                            if (rv != AIR_E_OK)
                            {
                                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr,
                                           rv, data);
                                return rv;
                            }
                            force_speed = (data & 0xf);

                            if (force_speed == (SERDES_RA_P0_MII_RA_RX_EN | SERDES_RA_P0_MII_RA_TX_EN |
                                                SERDES_RA_P0_MII_RA_RX_MODE | SERDES_RA_P0_MII_RA_TX_MODE))
                            {
                                *ptr_speed = HAL_PHY_SPEED_10M;
                            }
                            else if (force_speed == (SERDES_RA_P0_MII_RA_RX_EN | SERDES_RA_P0_MII_RA_TX_EN))
                            {
                                *ptr_speed = HAL_PHY_SPEED_100M;
                            }
                            else
                            {
                                *ptr_speed = HAL_PHY_SPEED_1000M;
                            }
                        }
                        return rv;
                    }
                    else
                    {
                        return AIR_E_NOT_SUPPORT;
                    }
                }
                else if (port_cfg.value == 2)
                {
                    return AIR_E_NOT_SUPPORT;
                }
            }
        }
        else
        {
            return rv;
        }
    }
#endif
    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if (reg_data & BMCR_SPEED1000)
    {
        *ptr_speed = HAL_PHY_SPEED_1000M;
    }
    else if (reg_data & BMCR_SPEED100)
    {
        *ptr_speed = HAL_PHY_SPEED_100M;
    }
    else
    {
        *ptr_speed = HAL_PHY_SPEED_10M;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setDuplex
 * PURPOSE:
 *      This API is used to set port duplex.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      duplex         --  Port duplex
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
hal_en8808_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if (HAL_PHY_DUPLEX_FULL == duplex)
    {
        reg_data |= BMCR_FULLDPLX;
    }
    else if (HAL_PHY_DUPLEX_HALF == duplex)
    {
        reg_data &= ~(BMCR_FULLDPLX);
    }
    else
    {
        return AIR_E_NOT_SUPPORT;
    }

    rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getDuplex
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
hal_en8808_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if (reg_data & BMCR_FULLDPLX)
    {
        *ptr_duplex = HAL_PHY_DUPLEX_FULL;
    }
    else
    {
        *ptr_duplex = HAL_PHY_DUPLEX_HALF;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getLinkStatus
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
hal_en8808_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    UI32_T              reg_addr = 0;
    UI16_T              reg_data = 0;
    UI16_T              phy_port = 0;
    UI8_T               speed = 0, duplex = 0, link_status = 0;
    UI32_T              data = 0;
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    HAL_PHY_AUTO_NEGO_T auto_nego;
    HAL_PHY_OP_MODE_T   phy_mode = HAL_PHY_OP_MODE_LAST;
#ifdef AIR_EN_I2C_PHY
    HAL_PHY_SERDES_MODE_T serdes_mode;
    HAL_PHY_COMBO_MODE_T  combo_mode;
    UI32_T                serdes_base = 0, an_done = 0, an_done_intr = 0;
    AIR_CFG_VALUE_T       port_cfg;
    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);
#endif

    if ((rv = hal_en8808_phy_getAutoNego(unit, port, &auto_nego)) != AIR_E_OK)
    {
#ifdef AIR_EN_I2C_PHY
        if ((port_cfg.value == 2) && (rv == AIR_E_NOT_SUPPORT))
        {
            /* handle SERDES_PORT_OPTION mode 2 in 1000baseX */
            auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
        }
        else
#endif
        {
            return rv;
        }
    }

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        if (PER_PSR_PORTS <= phy_port)
        {
            reg_addr = PSR_P7_P4_REG;
        }
        else
        {
            reg_addr = PSR_P3_P0_REG;
        }
#ifdef AIR_EN_I2C_PHY
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            reg_addr = SFP_LINK_STATUS_REG;
        }
        if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        }
        else
        {
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        }
#else
        aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
#endif
        speed = GET_PSR_LINK_SPEED(data, phy_port);

        if (PSR_LINK_SPEED_1000M == speed)
        {
            ptr_status->speed = HAL_PHY_SPEED_1000M;
        }
        else if (PSR_LINK_SPEED_100M == speed)
        {
            ptr_status->speed = HAL_PHY_SPEED_100M;
        }
        else if (PSR_LINK_SPEED_10M == speed)
        {
            ptr_status->speed = HAL_PHY_SPEED_10M;
        }

        duplex = GET_PSR_LINK_DUPLEX(data, phy_port);

        if (PSR_LINK_DUPLEX_FULL == duplex)
        {
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else if (PSR_LINK_DUPLEX_HALF == duplex)
        {
            ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
        }

        link_status = GET_PSR_LINK_STS(data, phy_port);

        if (link_status)
        {
            ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
#ifdef AIR_EN_I2C_PHY
            rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
            {
                rv = hal_en8808_phy_getSerdesMode(unit, port, &serdes_mode);
                if (AIR_E_OK != rv)
                {
                    return rv;
                }

                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                PHY_ID_TO_SERDES_BASE_ADDRESS(phy_port, serdes_base);
                if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
                {
                    rv = hal_mdio_readRegByI2c(unit, port, (serdes_base + SERDES_PCS_STATE_REG_2), &data);
                    if (rv != AIR_E_OK)
                    {
                        return rv;
                    }
                    GET_SERDES_AN_DONE(data, an_done);
                    if (an_done)
                    {
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                    }
                    else
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                    }

                    /* get an done interrupt state */
                    rv = hal_mdio_readRegByI2c(unit, port, (serdes_base + SERDES_PCS_INT_STATE_REG), &data);
                    if (rv != AIR_E_OK)
                    {
                        return rv;
                    }
                    GET_SERDES_AN_DONE_INTR(data, an_done_intr);
                    if (an_done_intr)
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                        CLEAR_SERDES_AN_DONE_INTR(unit, port, serdes_base);
                    }
                }
            }
#endif
        }
        else
        {
            ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
        }

        if ((rv = hal_mdio_readC22ByPort(0, port, MII_BMSR, &reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        SET_PHY_ABILITY(reg_data, BMSR_ANEGCOMPLETE, HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
        SET_PHY_ABILITY(reg_data, BMSR_RFAULT, HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);
    }
    else
    {
#ifdef AIR_EN_I2C_PHY

        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            reg_addr = SFP_LINK_STATUS_REG;
            rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
            if (AIR_E_OK == rv)
            {
                link_status = GET_PSR_LINK_STS(data, phy_port);
                if (link_status)
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
                else
                {
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                }

                if (port_cfg.value == 2)
                {
                    /*port_cfg.value = 2 return AIR_NOT_SUPPORT, the speed and dupelx
                     * information decide by port api */
                    return AIR_E_NOT_SUPPORT;
                }
                else
                {
                    speed = GET_PSR_LINK_SPEED(data, phy_port);

                    if (PSR_LINK_SPEED_1000M == speed)
                    {
                        ptr_status->speed = HAL_PHY_SPEED_1000M;
                    }
                    else if (PSR_LINK_SPEED_100M == speed)
                    {
                        ptr_status->speed = HAL_PHY_SPEED_100M;
                    }
                    else if (PSR_LINK_SPEED_10M == speed)
                    {
                        ptr_status->speed = HAL_PHY_SPEED_10M;
                    }

                    duplex = GET_PSR_LINK_DUPLEX(data, phy_port);

                    if (PSR_LINK_DUPLEX_FULL == duplex)
                    {
                        ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                    }
                    else if (PSR_LINK_DUPLEX_HALF == duplex)
                    {
                        ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                    }
                    return AIR_E_OK;
                }
            }
        }
        /* combo phy mode & build-in gphy */
        else if (((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_PHY == combo_mode)) || AIR_E_NOT_SUPPORT == rv)
#endif
        {
            rv = hal_mdio_readC22ByPort(0, port, MII_BMSR, &reg_data);
            if (AIR_E_OK != rv)
            {
                return rv;
            }

            if (reg_data & BMSR_LSTATUS)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }
            SET_PHY_ABILITY(reg_data, BMSR_ANEGCOMPLETE, HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
            SET_PHY_ABILITY(reg_data, BMSR_RFAULT, HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);

            if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
            {
                return rv;
            }

            if (reg_data & BMCR_FULLDPLX)
            {
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
            else
            {
                ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
            }
            if (reg_data & BMCR_SPEED1000)
            {
                ptr_status->speed = HAL_PHY_SPEED_1000M;
            }
            else if (reg_data & BMCR_SPEED100)
            {
                ptr_status->speed = HAL_PHY_SPEED_100M;
            }
            else
            {
                ptr_status->speed = HAL_PHY_SPEED_10M;
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, &reg_data);
        if (AIR_E_OK == rv)
        {
            if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (HAL_PHY_SPEED_1000M == ptr_status->speed))
            {
                if (EN8808_1G_LOW_POWER != reg_data)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, EN8808_1G_LOW_POWER);
                }
            }
            else
            {
                if (EN8808_10M_100M_LOW_POWER != reg_data)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, EN8808_10M_100M_LOW_POWER);
                }
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = _hal_en8808_phy_calRandomSeed(unit, port,
                                           (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP ? TRUE : FALSE));
    }

    if (AIR_E_OK == rv)
    {
        /* Fix 0x1e 0x148 value to 0x200 when link speed is 10M at long-reach mode */
        rv = hal_en8808_phy_getPhyOpMode(unit, port, &phy_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_LONG_REACH == phy_mode))
        {
            rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x148, &reg_data);
            if (AIR_E_OK == rv)
            {
                if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (HAL_PHY_SPEED_10M == ptr_status->speed))
                {
                    if (0x0 == reg_data)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "port %u link at 10M in long-reach mode, set 0x1e 0x148 to 0x200!\n",
                                   port);
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x148, 0x200);
                    }
                }
                else
                {
                    if (0x0 != reg_data)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "port %u other status, set 0x1e 0x148 to 0x0!\n", port);
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x148, 0x0);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setLoopBack
 * PURPOSE:
 *      This API is used to get port link status.
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
hal_en8808_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable)
{
    UI16_T         reg_data = 0, page = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_LPBK_FAR_END == lpbk_type)
    {
        /* Backup page */
        if ((rv = hal_mdio_readC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        /* Read data from ExtReg1A */
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        if (TRUE == enable)
        {
            reg_data |= LPBK_FAR_END;
        }
        else
        {
            reg_data &= ~(LPBK_FAR_END);
        }

        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_RESV2, reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        /* Restore page*/
        if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
        {
            return rv;
        }
    }
    else
    {
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        if (TRUE == enable)
        {
            reg_data |= BMCR_LOOPBACK;
        }
        else
        {
            reg_data &= ~(BMCR_LOOPBACK);
        }

        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data)) != AIR_E_OK)
        {
            return rv;
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get port link status.
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
hal_en8808_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable)
{
    UI16_T         reg_data = 0, page = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_LPBK_FAR_END == lpbk_type)
    {
        /* Backup page */
        if ((rv = hal_mdio_readC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        /* Read data from ExtReg1A */
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        if (reg_data & LPBK_FAR_END)
        {
            *ptr_enable = TRUE;
        }
        else
        {
            *ptr_enable = FALSE;
        }

        /* Restore page*/
        if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
        {
            return rv;
        }
    }
    else
    {
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data)) != AIR_E_OK)
        {
            return rv;
        }

        if (reg_data & BMCR_LOOPBACK)
        {
            *ptr_enable = TRUE;
        }
        else
        {
            *ptr_enable = FALSE;
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setSmartSpeedDown
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
hal_en8808_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if ((HAL_PHY_SSD_MODE_1T == ssd_mode) || (HAL_PHY_SSD_MODE_5T < ssd_mode))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Backup page */
    if ((rv = hal_mdio_readC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
    {
        return rv;
    }
    if ((rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data)) != AIR_E_OK)
    {
        return rv;
    }

    data &= ~BITS(2, 3);
    if (HAL_PHY_SSD_MODE_DISABLE != ssd_mode)
    {
        data |= BIT(4);
        data |= (ssd_mode - HAL_PHY_SSD_MODE_2T) << 2;
    }
    else
    {
        data &= ~BIT(4);
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if ((rv = hal_mdio_writeC22ByPort(unit, port, 0x14, data)) != AIR_E_OK)
    {
        return rv;
    }

    /* Restore page */
    if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getSmartSpeedDown
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
hal_en8808_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Backup page */
    if ((rv = hal_mdio_readC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
    {
        return rv;
    }
    if ((rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data)) != AIR_E_OK)
    {
        return rv;
    }

    reg_data = BITS_OFF_R(data, 4, 1);
    if (reg_data)
    {
        *ptr_ssd_mode = (BITS_OFF_R(data, 2, 2) + HAL_PHY_SSD_MODE_2T);
    }
    else
    {
        *ptr_ssd_mode = HAL_PHY_SSD_MODE_DISABLE;
    }

    /* Restore page */
    if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8808_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setLedOnCtrl
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
hal_en8808_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = EN8808_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8808_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = EN8808_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = EN8808_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (TRUE == enable)
        {
            reg_data |= EN8808_LED_LINK_FORCE_ON;
        }
        else
        {
            reg_data &= ~(EN8808_LED_LINK_FORCE_ON);
        }

        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getLedOnCtrl
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
hal_en8808_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = EN8808_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8808_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = EN8808_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = EN8808_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & EN8808_LED_LINK_FORCE_ON)
        {
            *ptr_enable = TRUE;
        }
        else
        {
            *ptr_enable = FALSE;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_testTxCompliance
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
hal_en8808_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    if (mode > HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D)
    {
        ret = AIR_E_BAD_PARAMETER;
    }
    else
    {
        ret = hal_cmn_phy_testTxCompliance(unit, port, mode);
    }
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_setComboMode
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
hal_en8808_phy_setComboMode(
    const UI32_T               unit,
    const UI32_T               port,
    const HAL_PHY_COMBO_MODE_T combo_mode)
{
#ifdef AIR_EN_I2C_PHY
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         reg_data = 0;
    UI32_T         phy_id = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        if (HAL_DEVICE_REV_ID(unit) == 0x3)
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, EFIFO_CTRL1_E3_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
                {
                    reg_data &= ~(1 << phy_id);
                }
                else if (HAL_PHY_COMBO_MODE_PHY == combo_mode)
                {
                    reg_data |= (1 << phy_id);
                }
                else
                {
                    rv = AIR_E_BAD_PARAMETER;
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", EFIFO_CTRL1_E3_REG, rv,
                           reg_data);
            }

            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeRegByI2c(unit, port, EFIFO_CTRL1_E3_REG, reg_data);
            }
        }
        else
        {
            if (HAL_PHY_COMBO_MODE_PHY == combo_mode)
            {
                reg_data = RG_PHY_MODE_SEL;
            }
            else if (HAL_PHY_COMBO_MODE_SERDES == combo_mode)
            {
                reg_data = RG_COMBO_MODE_SEL;
            }
            else
            {
                rv = AIR_E_BAD_PARAMETER;
            }

            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeRegByI2c(unit, port, EFIFO_CTRL1_REG, reg_data);
            }
        }
    }
    else
    {
        rv = AIR_E_NOT_SUPPORT;
    }
    return rv;
#else
    return AIR_E_NOT_SUPPORT;
#endif
}

/* FUNCTION NAME: hal_en8808_phy_getComboMode
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
hal_en8808_phy_getComboMode(
    const UI32_T          unit,
    const UI32_T          port,
    HAL_PHY_COMBO_MODE_T *ptr_combo_mode)
{
#ifdef AIR_EN_I2C_PHY
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         reg_data = 0;
    UI32_T         phy_id = 0;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        if (HAL_DEVICE_REV_ID(unit) >= 0x3)
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, EFIFO_CTRL1_E3_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                *ptr_combo_mode = BITS_OFF_R(~reg_data, phy_id, 1);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", EFIFO_CTRL1_E3_REG, rv,
                           reg_data);
            }
        }
        else
        {
            rv = hal_mdio_readRegByI2c(unit, port, EFIFO_CTRL1_REG, &reg_data);
            if (reg_data == RG_COMBO_MODE_SEL)
            {
                *ptr_combo_mode = HAL_PHY_COMBO_MODE_SERDES;
            }
            else if (reg_data == RG_PHY_MODE_SEL)
            {
                *ptr_combo_mode = HAL_PHY_COMBO_MODE_PHY;
            }
            else
            {
                rv = AIR_E_BAD_PARAMETER;
            }
        }
    }
    else
    {
        rv = AIR_E_NOT_SUPPORT;
    }
    return rv;
#else
    return AIR_E_NOT_SUPPORT;
#endif
}

/* FUNCTION NAME:   hal_en8808_phy_setSerdesMode
 * PURPOSE:
 *      This API is used to set port serdes mode.
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
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8808_phy_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
#ifdef AIR_EN_I2C_PHY
    AIR_ERROR_NO_T rv = AIR_E_NOT_SUPPORT;
    UI32_T         reg_addr = 0, serdes_base = 0;
    UI32_T         data = 0, phy_id;
    UI32_T         efifo_mode_addr = 0;
    UI32_T         serdes_id = 0, tx_disable = 0;

    if ((HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode) || (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode))
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Combo port dose not support this mode - %d\n", serdes_mode);
        return AIR_E_BAD_PARAMETER;
    }
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

    if (FALSE == HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        /* get remote serdes id */
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);
    }
    else
    {
        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);
    }
    if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
    {
        rv = hal_mdio_readRegByI2c(unit, port, (serdes_base + SERDES_PHYA_REG_61), &tx_disable);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, tx_disable);
            return rv;
        }
        tx_disable |= SERDES_TX_DISABLE;
        WRITE_EXTERNAL_EN880X_RG((serdes_base + SERDES_PHYA_REG_61), tx_disable);

        osal_delayUs(250000);

        /* SERDES_AN_REG_13 */
        reg_addr = serdes_base + SERDES_AN_REG_13;
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(BITS_RANGE(REG_IF_MODE_OFFT, REG_IF_MODE_LENG));
        data |= BITS_OFF_L(REG_IF_MODE_1000BASEX, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write Reg 0x%08X value to 0x%08X \n", reg_addr, data);

        /* SERDES_AN_REG_4 */
        reg_addr = serdes_base + SERDES_AN_REG_4;
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(BIT(REG_TX_ABILITY_SGMII_OFFT));
        data |= (BIT(REG_1000BASEX_FULL_DUPLEX_OFFT) | BIT(REG_1000BASEX_PAUSE_OFFT) | BIT(REG_1000BASEX_ASM_DIR_OFFT));
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write Reg 0x%08X value to 0x%08X \n", reg_addr, data);
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);

        /* Config remote Efifo */
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            efifo_mode_addr = EFIFO_MODE_REG + (phy_id * EFIFO_REG_OFFSET);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write Reg 0x%08X value to 0x%08X \n", efifo_mode_addr, EFIFO_MODE_1000);
            WRITE_EXTERNAL_EN880X_RG(efifo_mode_addr, EFIFO_MODE_1000);
        }

        reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_4) + (phy_id * QSGMII_LANE_OFFSET);
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write Reg 0x%08X value to 0x%08X \n", reg_addr, data);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data = (0x5801); /* AN ability 1G speed & full duplex */
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg] Write Reg 0x%08X value to 0x%08X \n", reg_addr, data);
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);

        tx_disable &= ~(SERDES_TX_DISABLE);
        WRITE_EXTERNAL_EN880X_RG((serdes_base + SERDES_PHYA_REG_61), tx_disable);
    }
    else
    {
        /* SERDES_AN_REG_13 */
        reg_addr = serdes_base + SERDES_AN_REG_13;
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(BITS_RANGE(REG_IF_MODE_OFFT, REG_IF_MODE_LENG));
        data |= BITS_OFF_L(REG_IF_MODE_SGMII, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);

        /* SERDES_AN_REG_4 */
        reg_addr = serdes_base + SERDES_AN_REG_4;
        rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &=
            ~(BIT(REG_1000BASEX_FULL_DUPLEX_OFFT) | BIT(REG_1000BASEX_PAUSE_OFFT) | BIT(REG_1000BASEX_ASM_DIR_OFFT));
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);
        data |= BIT(REG_TX_ABILITY_SGMII_OFFT);
        WRITE_EXTERNAL_EN880X_RG(reg_addr, data);
    }

    /* reAN */
    reg_addr = (REMOTE_SERDES_ADDR + SERDES_AN_REG_0) + (phy_id * QSGMII_LANE_OFFSET);
    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data |= BIT(REG_SGMII_AN_RESTART_OFFT);
    WRITE_EXTERNAL_EN880X_RG(reg_addr, data);

    return rv;
#else
    return AIR_E_NOT_SUPPORT;
#endif
}

/* FUNCTION NAME:   hal_en8808_phy_getSerdesMode
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
hal_en8808_phy_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
#ifdef AIR_EN_I2C_PHY
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         reg_addr = 0, phy_id;
    UI32_T         data = 0;
    UI32_T         serdes_id, serdes_base = 0;
    ;

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

    if (FALSE == HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        /* get remote serdes id */
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);
    }
    else
    {
        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, serdes_base);
    }
    reg_addr = serdes_base + SERDES_AN_REG_13;

    /* read remote serdes value */
    rv = hal_mdio_readRegByI2c(unit, port, reg_addr, &data);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fali (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    if (data & SERDES_SGMII_ENABLE)
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_SGMII;
    }
    else
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_1000BASE_X;
    }

    return rv;
#else
    return AIR_E_NOT_SUPPORT;
#endif
}

/* FUNCTION NAME: hal_en8808_phy_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 *      ctrl_mode           -- LED control mode enumeration type
 *                             HAL_PHY_LED_CTRL_MODE_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_en8808_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    UI32_T         src_ctrl = 0, phy_port = 0;
    UI32_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;
#endif

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    _en8808_getChipLEDType(unit, &reg_data);

    if (LED_TYPE_PARALLEL == reg_data)
    {
        if (0 == led_id)
        {
            src_ctrl = 1 << _en8808_pled_gpio[phy_port];
            rv |= aml_readReg(unit, HW_FORCE_GPIO_EN_REG, &reg_data, sizeof(UI32_T));
            if (HAL_PHY_LED_CTRL_MODE_PHY == ctrl_mode)
            {
                reg_data &= ~src_ctrl;
            }
            else if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                reg_data |= src_ctrl;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
            rv |= aml_writeReg(unit, HW_FORCE_GPIO_EN_REG, &reg_data, sizeof(UI32_T));
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        return rv;
    }

    if (HAL_DEVICE_REV_ID(unit) < 3)
    {
        return AIR_E_NOT_SUPPORT;
    }
#ifdef AIR_EN_I2C_PHY
    /* check combo mode */
    _en8808_getComboPortLEDType(unit, port, &data);
    if (HAL_IS_COMBO_PORT_VALID(unit, port) && (LED_TYPE_SERIAL == data))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_CFG_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (HAL_PHY_LED_CTRL_MODE_PHY == ctrl_mode)
                {
                    reg_data &= ~SFP_LED_WAVE_EN_MASK(phy_id, led_id);
                }
                else if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
                {
                    reg_data |= SFP_LED_WAVE_EN_MASK(phy_id, led_id);
                }
                else
                {
                    return AIR_E_BAD_PARAMETER;
                }
                rv = hal_mdio_writeRegByI2c(unit, port, SFP_WG_LED_CFG_REG, reg_data);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG, rv,
                           reg_data);
            }
            return rv;
        }
    }

#endif

    if (0 == led_id)
    {
        src_ctrl = 0x100 << (phy_port * 2);
    }
    else if (1 == led_id)
    {
        src_ctrl = 0x200 << (phy_port * 2);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    _en8808_readReg(unit, port, SLED_SRC_SEL_REG, &reg_data);

    if (HAL_PHY_LED_CTRL_MODE_PHY == ctrl_mode)
    {
        reg_data &= ~src_ctrl;
    }
    else if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
    {
        reg_data |= src_ctrl;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    _en8808_writeReg(unit, port, SLED_SRC_SEL_REG, reg_data);
    return rv;
}

/* FUNCTION NAME: hal_en8808_phy_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode       -- LED control enumeration type
 *                             HAL_PHY_LED_CTRL_MODE_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      [Serial LED]
 *      GPIO_12_flash_set_id = FlashingPeriod-6 = LED0
 *      GPIO_13_flash_set_id = FlashingPeriod-6 = LED0
 *      GPIO_14_flash_set_id = FlashingPeriod-7 = LED1
 *      GPIO_15_flash_set_id = FlashingPeriod-7 = LED1
 */
AIR_ERROR_NO_T
hal_en8808_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    UI32_T         src_ctrl = 0, phy_port = 0;
    UI32_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;
#endif

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    _en8808_getChipLEDType(unit, &reg_data);
    if (LED_TYPE_PARALLEL == reg_data)
    {
        if (0 == led_id)
        {
            src_ctrl = 1 << _en8808_pled_gpio[phy_port];
            rv = aml_readReg(unit, HW_FORCE_GPIO_EN_REG, &reg_data, sizeof(UI32_T));
            if (reg_data & src_ctrl)
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_FORCE;
            }
            else
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_PHY;
            }
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        return rv;
    }

    if (HAL_DEVICE_REV_ID(unit) < 3)
    {
        return AIR_E_NOT_SUPPORT;
    }

#ifdef AIR_EN_I2C_PHY
    /* check combo mode */
    _en8808_getComboPortLEDType(unit, port, &data);
    if (HAL_IS_COMBO_PORT_VALID(unit, port) && (LED_TYPE_SERIAL == data))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_CFG_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (reg_data & SFP_LED_WAVE_EN_MASK(phy_id, led_id))
                {
                    *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_FORCE;
                }
                else
                {
                    *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_PHY;
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG, rv,
                           reg_data);
            }
            return rv;
        }
    }

#endif

    if (0 == led_id)
    {
        src_ctrl = 0x100 << (phy_port * 2);
    }
    else if (1 == led_id)
    {
        src_ctrl = 0x200 << (phy_port * 2);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    _en8808_readReg(unit, port, SLED_SRC_SEL_REG, &reg_data);

    if (reg_data & src_ctrl)
    {
        *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_FORCE;
    }
    else
    {
        *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_PHY;
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 *
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 *      state               -- LED force state
 *                             HAL_PHY_LED_STATE_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      FlashingPeriod-4 = OFF (active low)
 *      FlashingPeriod-5 = ON  (active low)
 *      FlashingPeriod-6 = LED0
 *      FlashingPeriod-7 = LED1
 */
AIR_ERROR_NO_T
hal_en8808_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    UI32_T         reg_data = 0, flash_id = 0, phy_port = 0, reg_addr = 0, shift = 0, gpio_pin = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, patt_data = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;
#endif

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    _en8808_getChipLEDType(unit, &reg_data);
    if (LED_TYPE_PARALLEL == reg_data)
    {
        if (HAL_PHY_LED_STATE_FORCE_PATT == state)
        {
            if (0 == led_id)
            {
                flash_id = FLASH_MAP_ID_PATT_0 & 0x0f;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }
        else if (HAL_PHY_LED_STATE_OFF == state)
        {
            flash_id = FLASH_MAP_ID_OFF & 0x0f;
        }
        else if (HAL_PHY_LED_STATE_ON == state)
        {
            flash_id = FLASH_MAP_ID_ON & 0x0f;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
        gpio_pin = _en8808_pled_gpio[phy_port];
        if (gpio_pin >= 8)
        {
            reg_addr = GPIO_FLASH_MAP_CFG1_REG;
        }
        else
        {
            reg_addr = GPIO_FLASH_MAP_CFG0_REG;
        }
        shift = (gpio_pin & 0x07) * 4;
        rv |= aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        reg_data = (reg_data & ~(0x0f << shift)) | (flash_id << shift);
        rv |= aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        return rv;
    }

    if (HAL_DEVICE_REV_ID(unit) < 3)
    {
        return AIR_E_NOT_SUPPORT;
    }
#ifdef AIR_EN_I2C_PHY
    /* check combo mode */
    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        _en8808_getComboPortLEDType(unit, port, &data);
        if (LED_TYPE_SERIAL == data)
        {
            rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
            {
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_CFG_REG, &reg_data);
                rv |= hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_PATT_REG, &patt_data);
                if (AIR_E_OK == rv)
                {
                    if (state >= HAL_PHY_LED_STATE_LAST)
                    {
                        return AIR_E_BAD_PARAMETER;
                    }
                    reg_data &= ~SFP_LED_WAVE_TYPE_MASK(phy_id, led_id);
                    flash_id = state << SFP_LED_WAVE_CFG_POS(phy_id, led_id);
                    reg_data |= flash_id;
                    rv |= hal_mdio_writeRegByI2c(unit, port, SFP_WG_LED_CFG_REG, reg_data);
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG,
                               rv, reg_data);
                }
                return rv;
            }
        }
        else
        {
            if (HAL_PHY_LED_STATE_FORCE_PATT == state)
            {
                flash_id = FLASH_MAP_ID_PATT_0 & 0x0f;
            }
            else if (HAL_PHY_LED_STATE_OFF == state)
            {
                flash_id = FLASH_MAP_ID_OFF & 0x0f;
            }
            else if (HAL_PHY_LED_STATE_ON == state)
            {
                flash_id = FLASH_MAP_ID_ON & 0x0f;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
            gpio_pin = phy_port + (led_id * 4);
            if (gpio_pin >= 8)
            {
                reg_addr = GPIO_FLASH_MAP_CFG1_REG;
            }
            else
            {
                reg_addr = GPIO_FLASH_MAP_CFG0_REG;
            }
            shift = (gpio_pin & 0x07) * 4;
            _en8808_readReg(unit, port, reg_addr, &reg_data);
            reg_data = (reg_data & ~(0x0f << shift)) | (flash_id << shift);
            _en8808_writeReg(unit, port, reg_addr, reg_data);
            return rv;
        }
    }

#endif

    if (HAL_PHY_LED_STATE_FORCE_PATT == state)
    {
        if (0 == led_id)
        {
            flash_id = FLASH_MAP_ID_PATT_0;
        }
        else
        {
            flash_id = FLASH_MAP_ID_PATT_1;
        }
    }
    else if (HAL_PHY_LED_STATE_OFF == state)
    {
        flash_id = FLASH_MAP_ID_OFF;
    }
    else if (HAL_PHY_LED_STATE_ON == state)
    {
        flash_id = FLASH_MAP_ID_ON;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    /* set flash map */
    _en8808_readReg(unit, port, GPIO_FLASH_MAP_CFG1_REG, &reg_data);
    if (0 == led_id)
    {
        reg_data = (reg_data & 0xff00ffff) | (flash_id << 16);
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data & 0x00ffffff) | (flash_id << 24);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    _en8808_writeReg(unit, port, GPIO_FLASH_MAP_CFG1_REG, reg_data);

    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 *
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 * OUTPUT:
 *      ptr_state           -- LED force state enumeration type
 *                             HAL_PHY_LED_STATE_T
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
hal_en8808_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    UI32_T         reg_data = 0, reg_addr = 0, shift = 0, phy_port = 0, gpio_pin = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;
#endif

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    _en8808_getChipLEDType(unit, &reg_data);
    if (LED_TYPE_PARALLEL == reg_data)
    {
        gpio_pin = _en8808_pled_gpio[phy_port];
        if (gpio_pin >= 8)
        {
            reg_addr = GPIO_FLASH_MAP_CFG1_REG;
        }
        else
        {
            reg_addr = GPIO_FLASH_MAP_CFG0_REG;
        }
        shift = (gpio_pin & 0x07) * 4;
        rv |= aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        reg_data = (reg_data >> shift) & 0x0000000f;

        if ((FLASH_MAP_ID_PATT_0 & 0x0f) == reg_data)
        {
            *ptr_state = HAL_PHY_LED_STATE_FORCE_PATT;
        }
        else if ((FLASH_MAP_ID_OFF & 0x0f) == reg_data)
        {
            *ptr_state = HAL_PHY_LED_STATE_OFF;
        }
        else if ((FLASH_MAP_ID_ON & 0x0f) == reg_data)
        {
            *ptr_state = HAL_PHY_LED_STATE_ON;
        }
        else
        {
            return AIR_E_NOT_SUPPORT;
        }
        return rv;
    }

    if (HAL_DEVICE_REV_ID(unit) < 3)
    {
        return AIR_E_NOT_SUPPORT;
    }
#ifdef AIR_EN_I2C_PHY
    /* check combo mode */
    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        _en8808_getComboPortLEDType(unit, port, &data);
        if (LED_TYPE_SERIAL == data)
        {
            rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
            {
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_CFG_REG, &reg_data);
                if (AIR_E_OK == rv)
                {
                    reg_data = (reg_data >> SFP_LED_WAVE_CFG_POS(phy_id, led_id)) & SFP_WAVEGEN_TYPE_MASK;
                    if (reg_data >= HAL_PHY_LED_STATE_LAST)
                    {
                        return AIR_E_BAD_PARAMETER;
                    }
                    *ptr_state = reg_data;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG,
                               rv, reg_data);
                }
                return rv;
            }
        }
        else
        {
            gpio_pin = phy_port + (led_id * 4);
            if (gpio_pin >= 8)
            {
                reg_addr = GPIO_FLASH_MAP_CFG1_REG;
            }
            else
            {
                reg_addr = GPIO_FLASH_MAP_CFG0_REG;
            }
            shift = (gpio_pin & 0x07) * 4;
            _en8808_readReg(unit, port, reg_addr, &reg_data);
            reg_data = (reg_data >> shift) & 0x0000000f;

            if ((FLASH_MAP_ID_PATT_0 & 0x0f) == reg_data)
            {
                *ptr_state = HAL_PHY_LED_STATE_FORCE_PATT;
            }
            else if ((FLASH_MAP_ID_OFF & 0x0f) == reg_data)
            {
                *ptr_state = HAL_PHY_LED_STATE_OFF;
            }
            else if ((FLASH_MAP_ID_ON & 0x0f) == reg_data)
            {
                *ptr_state = HAL_PHY_LED_STATE_ON;
            }
            else
            {
                return AIR_E_NOT_SUPPORT;
            }
            return rv;
        }
    }
#endif
    _en8808_readReg(unit, port, GPIO_FLASH_MAP_CFG1_REG, &reg_data);
    if (0 == led_id)
    {
        reg_data = (reg_data >> 16) & 0x000000ff;
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data >> 24);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (FLASH_MAP_ID_PATT_0 == reg_data || FLASH_MAP_ID_PATT_1 == reg_data)
    {
        *ptr_state = HAL_PHY_LED_STATE_FORCE_PATT;
    }
    else if (FLASH_MAP_ID_OFF == reg_data)
    {
        *ptr_state = HAL_PHY_LED_STATE_OFF;
    }
    else if (FLASH_MAP_ID_ON == reg_data)
    {
        *ptr_state = HAL_PHY_LED_STATE_ON;
    }
    else
    {
        return AIR_E_NOT_SUPPORT;
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 *      pattern             -- LED force pattern
 *                             HAL_PHY_LED_PATT_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      FlashingPeriod-6 for user define LED_0
 *      FlashingPeriod-7 for user define LED_1
 */
AIR_ERROR_NO_T
hal_en8808_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    UI32_T         reg_data = 0, wg_period = 0;
    UI32_T         wg_cycle = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, wave_type = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;

    /* check combo mode */
    _en8808_getComboPortLEDType(unit, port, &data);
    if (HAL_IS_COMBO_PORT_VALID(unit, port) && (LED_TYPE_SERIAL == data))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_PATT_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (pattern >= HAL_PHY_LED_PATT_LAST)
                {
                    return AIR_E_BAD_PARAMETER;
                }
                reg_data &= ~SFP_LED_WAVE_TYPE_MASK(phy_id, led_id);
                wave_type = pattern << SFP_LED_WAVE_CFG_POS(phy_id, led_id);
                reg_data |= wave_type;
                rv = hal_mdio_writeRegByI2c(unit, port, SFP_WG_LED_PATT_REG, reg_data);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG, rv,
                           reg_data);
            }
            return rv;
        }
    }
#endif

    if (HAL_PHY_LED_PATT_HZ_HALF == pattern)
    {
        wg_period = PATTERN_HZ_HALF;
        wg_cycle = WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_ONE == pattern)
    {
        wg_period = PATTERN_HZ_ONE;
        wg_cycle = WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_TWO == pattern)
    {
        wg_period = PATTERN_HZ_TWO;
        wg_cycle = WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_ZERO == pattern)
    {
        wg_period = PATTERN_HZ_ZERO;
        wg_cycle = WAVE_GEN_CYCLE_MIN;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* set wave-gen pattern */
    _en8808_readReg(unit, port, GPIO_FLASH_PRD_SET3_REG, &reg_data);
    if (0 == led_id)
    {
        reg_data = (reg_data & 0xffff0000) | wg_period;
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data & 0x0000ffff) | (wg_period << 16);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    _en8808_writeReg(unit, port, GPIO_FLASH_PRD_SET3_REG, reg_data);

    /* set wave-gen cycle */
    _en8808_readReg(unit, port, CYCLE_CFG_VALUE1_REG, &reg_data);
    if (0 == led_id)
    {
        reg_data = (reg_data & 0xff00ffff) | (wg_cycle << 16);
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data & 0x00ffffff) | (wg_cycle << 24);
    }
    _en8808_writeReg(unit, port, CYCLE_CFG_VALUE1_REG, reg_data);
    return rv;
}

/* FUNCTION NAME:   hal_en8808_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit                -- Device ID
 *      port                -- Index of port number
 *      led_id              -- LED ID
 * OUTPUT:
 *      ptr_pattern         -- LED force pattern enumeration type
 *                             HAL_PHY_LED_PATT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      FlashingPeriod-6 for user define LED_0
 *      FlashingPeriod-7 for user define LED_1
 */
AIR_ERROR_NO_T
hal_en8808_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    UI32_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI32_T               phy_id = 0, data = 0;
    HAL_PHY_COMBO_MODE_T combo_mode;

    /* check combo mode */
    _en8808_getComboPortLEDType(unit, port, &data);
    if (HAL_IS_COMBO_PORT_VALID(unit, port) && (LED_TYPE_SERIAL == data))
    {
        rv = hal_en8808_phy_getComboMode(unit, port, &combo_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_COMBO_MODE_SERDES == combo_mode))
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            rv = hal_mdio_readRegByI2c(unit, port, SFP_WG_LED_PATT_REG, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data = (reg_data >> SFP_LED_WAVE_CFG_POS(phy_id, led_id)) & SFP_WAVEGEN_TYPE_MASK;
                if (reg_data >= HAL_PHY_LED_PATT_LAST)
                {
                    return AIR_E_BAD_PARAMETER;
                }
                *ptr_pattern = reg_data;
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", SFP_WG_LED_CFG_REG, rv,
                           reg_data);
            }
            return rv;
        }
    }
#endif
    /* get wave-gen pattern */
    _en8808_readReg(unit, port, GPIO_FLASH_PRD_SET3_REG, &reg_data);
    if (0 == led_id)
    {
        reg_data = (reg_data & 0x0000ffff);
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data >> 16);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (PATTERN_HZ_HALF == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_HALF;
    }
    else if (PATTERN_HZ_ONE == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_ONE;
    }
    else if (PATTERN_HZ_TWO == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_TWO;
    }
    else if (PATTERN_HZ_ZERO == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_ZERO;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    return rv;
}

/* FUNCTION NAME: hal_en8808_phy_triggerCableTest
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
hal_en8808_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerCableTest(unit, port, test_pair, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_getCableTestRawData
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
hal_en8808_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_setPhyLedGlbCfg
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
hal_en8808_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_getPhyLedGlbCfg
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
hal_en8808_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_setPhyLedBlkEvent
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
hal_en8808_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_getPhyLedBlkEvent
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
hal_en8808_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_setPhyLedDuration
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
hal_en8808_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_getPhyLedDuration
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
hal_en8808_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_en8808_phy_setPhyOpMode
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
hal_en8808_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    rv = _hal_en8808_phy_setPhyOpMode(unit, port, mode);
    return rv;
}

/* FUNCTION NAME: hal_en8808_phy_getPhyOpMode
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
hal_en8808_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, &reg_data);
    if (AIR_E_OK == rv)
    {
        switch (reg_data)
        {
            case EN8808_LONG_REACH:
                *ptr_mode = HAL_PHY_OP_MODE_LONG_REACH;
                break;
            case EN8808_NORMAL:
            default:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
        }
    }
    return rv;
}

/* FUNCTION NAME: hal_en8808_phy_dumpPhyPara
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
hal_en8808_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    UI16_T            reg_data = 0;
    HAL_PHY_OP_MODE_T phy_mode;

    osal_printf("\n=== cl22 ===\n");
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_BMCR, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_BMSR, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_ADVERTISE, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, MII_LPA, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_LPA, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_CTRL1000, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_STAT1000, reg_data);
    }

    osal_printf("\n=== cl45 ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xa2, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xa2, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_ANEG, MMD_EEEAR, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3e, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3d, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3c, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x9b, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9b, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x9c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9c, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x23, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x23, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x24, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x24, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x25, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x25, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x26, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x26, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x176, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x176, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x177, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x177, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x200, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x200, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x201, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x201, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x202, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x202, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x203, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x203, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x204, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x204, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x205, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x205, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x206, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x206, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x207, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x207, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x208, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x208, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x209, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x209, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x210, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x210, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x211, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x211, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x212, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x212, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x213, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x213, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x214, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x214, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x215, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x215, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x216, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x216, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x217, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x217, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x218, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x218, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x219, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x219, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x220, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x220, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x221, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x221, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x222, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x222, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x223, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x223, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x224, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x224, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x225, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x225, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x226, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x226, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x227, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x227, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x228, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x228, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x229, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x229, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x230, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x230, reg_data);
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x231, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x231, reg_data);
    }
    osal_printf("\n=== 1g amplitude ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x16, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-a : 0x%04x\n", port, (BITS(0, 5) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x18, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-b : 0x%04x\n", port, (BITS(0, 5) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x20, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-c : 0x%04x\n", port, (BITS(0, 5) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x22, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-d : 0x%04x\n", port, (BITS(0, 5) & reg_data));
    }
    osal_printf("\n=== 100m amplitude ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x16, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-a : 0x%04x\n", port, (BITS(10, 15) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x18, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-b : 0x%04x\n", port, (BITS(8, 13) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x20, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-c : 0x%04x\n", port, (BITS(8, 13) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x22, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-d : 0x%04x\n", port, (BITS(8, 13) & reg_data));
    }
    osal_printf("\n=== 100m tx filter ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x23, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-a : 0x%04x\n", port, (BITS(0, 3) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x24, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-b : 0x%04x\n", port, (BITS(0, 3) & reg_data));
    }
    osal_printf("\n=== r50 ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-a : 0x%04x\n", port, (BITS(8, 14) & reg_data));
        osal_printf("port=%2u   pair-b : 0x%04x\n", port, (BITS(0, 6) & reg_data));
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u   pair-c : 0x%04x\n", port, (BITS(8, 14) & reg_data));
        osal_printf("port=%2u   pair-d : 0x%04x\n", port, (BITS(0, 6) & reg_data));
    }
    osal_printf("\n=== TX ML3 shaper ===\n");
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x9, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9, reg_data);
    }
    osal_printf("\n=== phy operation mode ===\n");
    rv = hal_en8808_phy_getPhyOpMode(unit, port, &phy_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_OP_MODE_NORMAL == phy_mode)
        {
            osal_printf("port=%2u    phy_op_mode               = normal\n", port);
        }
        else
        {
            osal_printf("port=%2u    phy_op_mode               = long-reach\n", port);
        }
    }
    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xc6, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xc6, reg_data);
    }
    return rv;
}

/* FUNCTION NAME: hal_en8808_phy_triggerLinkDownCableTest
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
hal_en8808_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerLinkDownCableTest(unit, port, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME:   hal_en8808_phy_dumpPortCnt
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
hal_en8808_phy_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_I2C_PHY
    UI16_T                    idx, rg_idx;
    HAL_EN8808_PHY_DBG_CFG_T *ptr_serdes_dbg_list;

    if (HAL_IS_COMBO_PORT_VALID(unit, port))
    {
        osal_printf("%25s        %16s        %9s \n", "Register", "Addr", "Value");
        for (idx = 0; idx < _hal_en8808_phy_dbg_cfg_info_size; idx++)
        {
            if (_hal_en8808_phy_dbg_cfg_info[idx].serdes_interface & param)
            {
                for (rg_idx = 0; rg_idx < _hal_en8808_phy_dbg_cfg_info[idx].serdes_register_cnt; rg_idx++)
                {
                    ptr_serdes_dbg_list =
                        (HAL_EN8808_PHY_DBG_CFG_T *)(_hal_en8808_phy_dbg_cfg_info[idx].ptr_serdes_dbg_cfg_map + rg_idx);

                    rc = _hal_en8808_phy_dumpDbgRegister(unit, port, ptr_serdes_dbg_list);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "set _hal_en8808_phy_dumpDbgRegister fail, rc = %d\n", rc);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
#endif
    return rc;
}

const HAL_PHY_DRIVER_T _ext_EN8808_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_en8808_phy_init,
    hal_en8808_phy_setAdminState,
    hal_en8808_phy_getAdminState,
    hal_en8808_phy_setAutoNego,
    hal_en8808_phy_getAutoNego,
    hal_en8808_phy_setLocalAdvAbility,
    hal_en8808_phy_getLocalAdvAbility,
    hal_en8808_phy_getRemoteAdvAbility,
    hal_en8808_phy_setSpeed,
    hal_en8808_phy_getSpeed,
    hal_en8808_phy_setDuplex,
    hal_en8808_phy_getDuplex,
    hal_en8808_phy_getLinkStatus,
    hal_en8808_phy_setLoopBack,
    hal_en8808_phy_getLoopBack,
    hal_en8808_phy_setSmartSpeedDown,
    hal_en8808_phy_getSmartSpeedDown,
    hal_en8808_phy_setLedOnCtrl,
    hal_en8808_phy_getLedOnCtrl,
    hal_en8808_phy_testTxCompliance,
    hal_en8808_phy_setComboMode,
    hal_en8808_phy_getComboMode,
    hal_en8808_phy_setSerdesMode,
    hal_en8808_phy_getSerdesMode,
    hal_en8808_phy_setPhyLedCtrlMode,
    hal_en8808_phy_getPhyLedCtrlMode,
    hal_en8808_phy_setPhyLedForceState,
    hal_en8808_phy_getPhyLedForceState,
    hal_en8808_phy_setPhyLedForcePattCfg,
    hal_en8808_phy_getPhyLedForcePattCfg,
    hal_en8808_phy_triggerCableTest,
    hal_en8808_phy_getCableTestRawData,
    hal_en8808_phy_setPhyLedGlbCfg,
    hal_en8808_phy_getPhyLedGlbCfg,
    hal_en8808_phy_setPhyLedBlkEvent,
    hal_en8808_phy_getPhyLedBlkEvent,
    hal_en8808_phy_setPhyLedDuration,
    hal_en8808_phy_getPhyLedDuration,
    hal_en8808_phy_setPhyOpMode,
    hal_en8808_phy_getPhyOpMode,
    hal_en8808_phy_dumpPhyPara,
    hal_en8808_phy_triggerLinkDownCableTest,
    hal_en8808_phy_dumpPortCnt,
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_en8808_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_EN8808_phy_func_vec;

    return (AIR_E_OK);
}
