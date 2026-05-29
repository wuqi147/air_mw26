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

/* FILE NAME:  hal_en8808_phy.h
 * PURPOSE:
 *      It provides PHY module API.
 * NOTES:
 */

#ifndef HAL_EN8808_PHY_H
#define HAL_EN8808_PHY_H

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PHY_DEVICE_ID_EN8808 (0x03a294b0)

#define PRODUCT_ID_REG_ADDR (0x10005000)
#define PRODUCT_ID          (0x00008851)

#define ECO_ID_REG_ADDR (0x10005004)

#define EN8808_I2C_E4_TX_DRIVING_CTRL (0x10000014)
#define EN8808_I2C_IO_E4_BITS         (0x0000000F)

#define PSR_P3_P0_REG          (0x10218030)
#define PSR_P7_P4_REG          (0x10218034)
#define PER_PSR_PORTS          (4)
#define PER_PSR_OFFSET         (8)
#define PSR_LINK_STS_MASK      (0x00000001)
#define PSR_LINK_SPEED_MASK    (0x00000003)
#define PSR_LINK_SPEED_OFFSET  (0x00000001)
#define PSR_LINK_SPEED_1000M   (0x00000002)
#define PSR_LINK_SPEED_100M    (0x00000001)
#define PSR_LINK_SPEED_10M     (0x00000000)
#define PSR_LINK_DUPLEX_MASK   (0x00000001)
#define PSR_LINK_DUPLEX_OFFSET (0x00000003)
#define PSR_LINK_DUPLEX_FULL   (1)
#define PSR_LINK_DUPLEX_HALF   (0)

#define SFP_LP_PAUSE_ADV_MASK       (0x00000001)
#define SFP_LP_PAUSE_ADV_OFFSET     (0x00000004)
#define SFP_LP_ASM_PAUSE_ADV_MASK   (0x00000001)
#define SFP_LP_ASM_PAUSE_ADV_OFFSET (0x00000005)
#define SFP_PAUSE_ADV               (0x1)

/* Register of serial LED control */
#define EN8808_SLED_CTRL0 (0x1021803c)
#define EN8808_SLED_CTRL1 (0x10218040)
/* Register of wave-gen and PHY LED source control  */
#define SLED_SRC_SEL_REG        (0x1021c018)
#define GPIO_CONTROL_REG        (0x1000A300)
#define GPIO_OE_REG             (0x1000A314)
#define GPIO_FLASH_MODE_CFG_REG (0x1000A334)
#define GPIO_RSP_MODECFG_REG    (0x1000A338)
#define GPIO_FLASH_PRD_SET0_REG (0x1000A33C)
#define GPIO_FLASH_PRD_SET1_REG (0x1000A340)
#define GPIO_FLASH_PRD_SET2_REG (0x1000A344)
#define GPIO_FLASH_PRD_SET3_REG (0x1000A348)
#define GPIO_FLASH_MAP_CFG0_REG (0x1000A34C)
#define GPIO_FLASH_MAP_CFG1_REG (0x1000A350)
#define CYCLE_CFG_VALUE0_REG    (0x1000A398)
#define CYCLE_CFG_VALUE1_REG    (0x1000A39C)

/* fields of serial LED control register */
#define EN8808_SLED_SOURCE_NORMAL_MODE       (0)
#define EN8808_SLED_SOURCE_INTERLEAVING_MODE (1 << 0)
#define EN8808_SLED_NUM_LED0                 (1 << 1)
#define EN8808_SLED_NUM_LED0_LED1            (2 << 1)
#define EN8808_SLED_NUM_LED0_LED1_LED2       (3 << 1)
#define EN8808_SLED_OUTPUT_TRANSITION_MODE   (0)
#define EN8808_SLED_OUTPUT_CONTINUOUS_MODE   (1 << 3)
#define EN8808_SLED_FALLING_EDGE             (0)
#define EN8808_SLED_RISING_EDGE              (1 << 4)

#define HW_TRAP_READ_REG     (0x100000a8)
#define HW_FORCE_GPIO_EN_REG (0x10000090)
#define HW_LAN_LED_IOMUX     (0x10000088)

#define PHY_BASE_ADDR_MASK  (0x0018)
#define PHY_BASE_ADDR_SHIFT (9)
#define HW_TRAP_SURGE_BIT   (1 << 14)

#define CPU_CLOCK_GATING_REG  (0x100000c8)
#define CPU_CLOCK_GATING_MASK (0x00000001)
#define CPU_CLOCK_DISABLE     (0x0)
#define CPU_CLOCK_ENABLE      (0x1)
#define TIMER_CONTROL_REG     (0x1000a100)
#define WDT_DISABLE           (0x0)

#define DMEM_ADDRESS           (0x80000000)
#define FIRMWARE_LEN_REG       (0x80038000)
#define FIRMWARE_CRC_REG       (0x80038004)
#define XPHY_CRC_RET_REG       (0x10005018)
#define HOST_CONTROL_REG       (0x1000501c)
#define SFP_LINK_STATUS_REG    (0x10004010)
#define SFP0_LED0_CFG_REG      (0x10211004) /* Using PHY's MCR for LED configuration */
#define SFP0_LED1_CFG_REG      (0x10212004) /* Using PHY's MCR for LED configuration */
#define SFP0_LED_PIN_CFG_REG   (0x10213004) /* Using PHY's MCR for LED pin configuration */
#define SFP_LED_CFG_OFFSET     (0x200)
#define SFP_LED_POL_OFF        (6)
#define COMBO_PARALLEL_LED_CFG (0x36db)

#define SFP_WG_LED_CFG_REG    (0x10004018)
#define SFP_WG_LED_PATT_REG   (0x1000401c)
#define SFP_WAVEGEN_BIT_LEN   (4)
#define SFP_WAVEGEN_MASK      (0x0f)
#define SFP_WAVEGEN_EN_BIT    (1 << 3)
#define SFP_WAVEGEN_TYPE_MASK (0x07)

#define SFP_WAVEGEN_HALF_HZ (0)
#define SFP_WAVEGEN_ONE_HZ  (1)
#define SFP_WAVEGEN_TWO_HZ  (2)
#define SFP_WAVEGEN_TRAFFIC (3)
#define SFP_WAVEGEN_OFF     (4)
#define SFP_WAVEGEN_ON      (5)

#define TOTAL_PHYS_IN_CHIP (8)

#define TXAMP_offset     (0)      // for 8851
#define TX_i2mpb_hbt_ofs (0x4)    // 8851 fine tune 100M v1 (20220414)

#define AN_disable_force_1000M       (0x0140)
#define BG_voltage_output            (0xc000)
#define Fix_mdi                      (0x1010)
#define Disable_tx_slew_control      (0x0000)
#define LDO_control                  (0x0100)
#define Cal_control_R50              (0x1100)
#define Disable_all                  (0x0)
#define Zcalen_A_ENABLE              (0x0000)
#define Zcalen_B_ENABLE              (0x1000)
#define Zcalen_C_ENABLE              (0x0100)
#define Zcalen_D_ENABLE              (0x0010)
#define MASK_r50ohm_rsel_tx_a        (0x7f00)
#define MASK_r50ohm_rsel_tx_b        (0x007f)
#define MASK_r50ohm_rsel_tx_c        (0x7f00)
#define MASK_r50ohm_rsel_tx_d        (0x007f)
#define Rg_r50ohm_rsel_tx_a_en       (0x8000)
#define Rg_r50ohm_rsel_tx_b_en       (0x0080)
#define Rg_r50ohm_rsel_tx_c_en       (0x8000)
#define Rg_r50ohm_rsel_tx_d_en       (0x0080)
#define Cal_control_R50_pairA_ENABLE (0x1101)
#define MASK_MSB_8bit                (0xff00)
#define MASK_LSB_8bit                (0x00ff)
#define Cal_control_TX_OFST          (0x0100)
#define Rg_txvos_calen_ENABLE        (0x0001)
#define Bypass_tx_offset_cal         (0x8000)
#define Enable_Tx_VLD                (0xf808)
#define Rg_txg_calen_a_ENABLE        (0x1000)
#define Rg_txg_calen_b_ENABLE        (0x0100)
#define Rg_txg_calen_c_ENABLE        (0x0010)
#define Rg_txg_calen_d_ENABLE        (0x0001)
#define Force_dasn_dac_in0_ENABLE    (0x8000)
#define Force_dasn_dac_in1_ENABLE    (0x8000)
#define MASK_cr_tx_amp_offset_MSB    (0x3f00)
#define MASK_cr_tx_amp_offset_LSB    (0x003f)
#define Cal_control_TX_AMP           (0x1100)
#define Rg_cal_refsel_ENABLE         (0x0010)
#define MASK_da_tx_i2mpb_a_gbe       (0xfc00)
#define MASK_da_tx_i2mpb_b_c_d_gbe   (0x3f00)
#define Cal_control_BG               (0x1110)

/* CL22 Reg Support Page Select */
#define EN8808_PHY_MII_PAGE_SELECT (0x1f)
#define CL22_Page_Reg              (0x0000)
#define CL22_Page_ExtReg           (0x0001)
#define CL22_Page_MiscReg          (0x0002)
#define CL22_Page_LpiReg           (0x0003)
#define CL22_Page_tReg             (0x02A3)
#define CL22_Page_TrReg            (0x52B5)
#define LPBK_FAR_END               (0x8000)

/* Ethernet Packet Generator (EPG) Control 1 Register */
#define EN8808_REG_EXT_1D        (0x1D)
#define EN8808_EPG_EN            (0x8000)
#define EN8808_EPG_RUN           (0x4000)
#define EN8808_EPG_TX_DUR        (0x2000)
#define EN8808_EPG_PKT_LEN_10KB  (0x1800)
#define EN8808_EPG_PKT_LEN_1518B (0x1000)
#define EN8808_EPG_PKT_LEN_64B   (0x0800)
#define EN8808_EPG_PKT_LEN_125B  (0x0000)
#define EN8808_EPG_PKT_GAP       (0x0400)
#define EN8808_EPG_DES_ADDR(a)   (((a) & 0xF) << 6)
#define EN8808_EPG_SUR_ADDR(a)   (((a) & 0xF) << 2)
#define EN8808_EPG_PL_TYP_RANDOM (0x0002)
#define EN8808_EPG_BAD_FCS       (0x0001)

#define EN8808_SLED_PIN_IOMUX (0x0c)
/* LED Behavior */
#define EN8808_LED_BHV_LINK_1000     (0x0001)
#define EN8808_LED_BHV_LINK_100      (0x0002)
#define EN8808_LED_BHV_LINK_10       (0x0004)
#define EN8808_LED_BHV_LINK_FULLDPLX (0x0008)
#define EN8808_LED_BHV_LINK_HALFDPLX (0x0010)
#define EN8808_LED_BHV_BLINK_TX_1000 (0x0020)
#define EN8808_LED_BHV_BLINK_RX_1000 (0x0040)
#define EN8808_LED_BHV_BLINK_TX_100  (0x0080)
#define EN8808_LED_BHV_BLINK_RX_100  (0x0100)
#define EN8808_LED_BHV_BLINK_TX_10   (0x0200)
#define EN8808_LED_BHV_BLINK_RX_10   (0x0400)
#define EN8808_LED_BHV_HIGH_ACTIVE   (0x0800)

/* LED Link register */
#define EN8808_LED_LINK_1000     (0x0001)
#define EN8808_LED_LINK_100      (0x0002)
#define EN8808_LED_LINK_10       (0x0004)
#define EN8808_LED_LINK_DOWN     (0x0008)
#define EN8808_LED_LINK_FULLDPLX (0x0010)
#define EN8808_LED_LINK_HALFDPLX (0x0020)
#define EN8808_LED_LINK_FORCE_ON (0x0040)
#define EN8808_LED_POL_HIGH_ACT  (0x4000)
#define EN8808_LED_FUNC_ENABLE   (0x8000)

/* LED Blink register */
#define EN8808_LED_BLINK_TX_1000 (0x0001)
#define EN8808_LED_BLINK_RX_1000 (0x0002)
#define EN8808_LED_BLINK_TX_100  (0x0004)
#define EN8808_LED_BLINK_RX_100  (0x0008)
#define EN8808_LED_BLINK_TX_10   (0x0010)
#define EN8808_LED_BLINK_RX_10   (0x0020)

/* LED Register */
#define EN8808_LED_COUNT      (4)
#define EN8808_LED_0_ON_MASK  (0x0024)
#define EN8808_LED_0_BLK_MASK (0x0025)
#define EN8808_LED_1_ON_MASK  (0x0026)
#define EN8808_LED_1_BLK_MASK (0x0027)
#define EN8808_LED_2_ON_MASK  (0x0028)
#define EN8808_LED_2_BLK_MASK (0x0029)
#define EN8808_LED_3_ON_MASK  (0x002A)
#define EN8808_LED_3_BLK_MASK (0x002B)

/* Wave-Gen pattern */
#define WAVE_GEN_CYCLE_567 (0xffffff00) /* for 250Hz cycle */
#define WAVE_GEN_CYCLE_MAX (0xff)
#define WAVE_GEN_CYCLE_MIN (0x00)
#define PATTERN_ON         (0xff00)
#define PATTERN_OFF        (0x00ff)
#define PATTERN_HZ_HALF    (0xfafa)
#define PATTERN_HZ_ONE     (0x7d7d)
#define PATTERN_HZ_TWO     (0x3e3e)
#define PATTERN_HZ_ZERO    (0x0000)

#define FLASH_MAP_ID_PATT_0        (0x00ee)
#define FLASH_MAP_ID_PATT_1        (0x00ff)
#define FLASH_MAP_ID_PATT_PARALLEL (0xeeeeeeee)
#define FLASH_MAP_ID_OFF           (0x00cc)
#define FLASH_MAP_ID_ON            (0x00dd)
#define LED_TYPE_SERIAL            (0)
#define LED_TYPE_PARALLEL          (1)
#define LED_TYPE_SFP_ONLY          (2)

#define ANACAL_INIT       (0x01)
#define ANACAL_ERROR      (0xFD)
#define ANACAL_SATURATION (0xFE)
#define ANACAL_FINISH     (0xFF)
#define ANACAL_PAIR_A     (0)
#define ANACAL_PAIR_B     (1)
#define ANACAL_PAIR_C     (2)
#define ANACAL_PAIR_D     (3)
#define DAC_IN_0V         (0x000)
#define DAC_IN_2V         (0x0f0)    // +/-1V

#define ZCAL_MIDDLE       (0x20)
#define TX_OFFSET_0mV_idx (31)
#define TX_AMP_MIDDLE     (0x20)

#define R50_OFFSET_VALUE (0x5)

/* 0x0000 */
#define RgAddr_Reg00h (0x00)

#define RgAddr_dev1Eh (0x1e)

/* 0x51e00120 */
#define RgAddr_dev1Eh_reg012h (0x0012)

/* 0x51e00130 */
#define RgAddr_dev1Eh_reg013h (0x0013)

/* 0x51e00170 */
#define RgAddr_dev1Eh_reg017h (0x0017)

/* 0x51e00190 */
#define RgAddr_dev1Eh_reg019h (0x0019)

/* 0x51e00210 */
#define RgAddr_dev1Eh_reg021h (0x0021)

/* 0x51e003e0 */
#define RgAddr_dev1Eh_reg03Eh (0x003e)

/* 0x51e00960 */
#define RgAddr_dev1Eh_reg096h (0x0096)

/* 0x51e00db0 */
#define RgAddr_dev1Eh_reg0DBh (0x00db)

/* 0x51e00dc0 */
#define RgAddr_dev1Eh_reg0DCh (0x00dc)

/* 0x51e00dd0 */
#define RgAddr_dev1Eh_reg0DDh (0x00dd)

/* 0x51e00e00 */
#define RgAddr_dev1Eh_reg0E0h (0x00e0)

/* 0x51e00e10 */
#define RgAddr_dev1Eh_reg0E1h (0x00e1)

/* 0x51e00fb0 */
#define RgAddr_dev1Eh_reg0FBh (0x00fb)

/* 0x51e01430 */
#define RgAddr_dev1Eh_reg143h (0x0143)

/* 0x51e01450 */
#define RgAddr_dev1Eh_reg145h (0x0145)
#define PD_DIS                (1 << 15)
#define FC_TDI_EN             (1 << 14)
#define FC_DI_ACT             (1 << 13)
#define FC_LITN_NO_COMP       (1 << 12)
#define FC_MDI_CO_MDIX        (3 << 3)
#define FC_MDI_CO_MDI         (2 << 3)
#define FC_MDI_CO_NOT         (0 << 3)
#define FC_10T_POLAR_SWAP     (3 << 1)
#define FC_10T_POLAR_NORMAL   (2 << 1)
#define FC_10T_POLAR_NOT      (0 << 1)
#define FC_MDI_CO_OFFT        (3)
#define FC_MDI_CO_LENG        (2)

/* 0x51e014A0 */
#define RgAddr_dev1Eh_reg14Ah   (0x014a)
#define PICMD_MISER_MODE_INT(v) (((v) & 0x7ff) << 5)

/* 0x51e01200*/
#define RgAddr_dev1Eh_reg120h (0x0120)

/* 0x51e01220*/
#define RgAddr_dev1Eh_reg122h (0x0122)

/* 0x51e01440 */
#define RgAddr_dev1Eh_reg144h (0x0144)

/* 0x51e019b0 */
#define RgAddr_dev1Eh_reg19Bh (0x019b)

/* 0x51e02340 */
#define RgAddr_dev1Eh_reg234h (0x0234)

/* 0x51e02380 */
#define RgAddr_dev1Eh_reg238h (0x0238)

/* 0x51e02390 */
#define RgAddr_dev1Eh_reg239h (0x0239)

/* 0x51f02680 */
#define RgAddr_dev1Fh_reg268h (0x0268)

/* 0x51e03230 */
#define RgAddr_dev1Eh_reg323h (0x0323)

/* 0x51e03240 */
#define RgAddr_dev1Eh_reg324h (0x0324)

/* 0x51e03260 */
#define RgAddr_dev1Eh_reg326h (0x0326)

/* 0x51e01720 */
#define RgAddr_dev1Eh_reg172h (0x0172)

/* 0x51e01730 */
#define RgAddr_dev1Eh_reg173h (0x0173)

/* 0x51e01740 */
#define RgAddr_dev1Eh_reg174h (0x0174)

/* 0x51e01750 */
#define RgAddr_dev1Eh_reg175h (0x0175)

/* 0x51e017a0 */
#define RgAddr_dev1Eh_reg17Ah (0x017a)

/* 0x51e017c0 */
#define RgAddr_dev1Eh_reg17Ch (0x017c)

/* 0x51e017d0 */
#define RgAddr_dev1Eh_reg17Dh (0x017d)

/* 0x51e017e0 */
#define RgAddr_dev1Eh_reg17Eh (0x017e)

/* 0x51e017f0 */
#define RgAddr_dev1Eh_reg17Fh (0x017f)

/* 0x51e01800 */
#define RgAddr_dev1Eh_reg180h (0x0180)

/* 0x51e01810 */
#define RgAddr_dev1Eh_reg181h (0x0181)

/* 0x51e01820 */
#define RgAddr_dev1Eh_reg182h (0x0182)

/* 0x51e01830 */
#define RgAddr_dev1Eh_reg183h (0x0183)

/* 0x51e01840 */
#define RgAddr_dev1Eh_reg184h (0x0184)

/* 0x51e01850 */
#define RgAddr_dev1Eh_reg185h (0x0185)

/* 0x51e01a30 */
#define RgAddr_dev1Eh_reg1A3h (0x01a3)

/* 0x51e01a40 */
#define RgAddr_dev1Eh_reg1A4h (0x01a4)

/* 0x51e020b0 */
#define RgAddr_dev1Eh_reg20Bh (0x020b)

/* 0x51e020e0 */
#define RgAddr_dev1Eh_reg20Eh (0x020e)
#define DSP_FRE_RP_FSM_EN     (1 << 4)
#define DSP_FRE_DW_AUTO_INC   (1 << 2)
#define DSP_FRE_WR_EN         (1 << 1)
#define DSP_FRE_SW_RST        (1 << 0)

/* 0x51e02d10 */
#define RgAddr_dev1Eh_reg2D1h  (0x02d1)
#define RG_LPI_VCO_EEE_STGO_EN (1 << 10)
#define RG_LPI_TR_READY        (1 << 9)
#define RG_LPI_SKIP_SD_SLV_TR  (1 << 8)
#define VCO_SLICER_THRES_H     (0x33)

#define RgAddr_dev1Fh (0x1f)

/* 0x31c0 */
#define RgAddr_LpiReg1Ch (0x1c)
/* 0x31d0 */
#define RgAddr_LpiReg1Dh (0x1d)

/* 0x52b5100 */
#define RgAddr_TrReg10h (0x10)
/* 0x52b5110 */
#define RgAddr_TrReg11h (0x11)
/* 0x52b5120 */
#define RgAddr_TrReg12h (0x12)

/* 0x51f00ff0 */
#define RgAddr_dev1Fh_reg0FFh (0x00ff)

/* 0x51f01000 */
#define RgAddr_dev1Fh_reg100h (0x0100)

/* 0x51f01150 */
#define RgAddr_dev1Fh_reg115h (0x0115)

/* 0x51f04030 */
#define RgAddr_dev1Fh_reg403h (0x0403)
#define RG_SYSPLL_DDSFBK_EN   (1 << 12)
#define RG_SYSPLL_DMY1        (3 << 8)
#define RG_SYSPLL_DMY2        (0xD9)

/* Register 52B5 */
/* Token Ring */
#define RgAddr_52B5_reg10h (0x10)
#define RgAddr_52B5_reg11h (0x11)
#define RgAddr_52B5_reg12h (0x12)

#define EFIFO_CTRL1_REG    (0x10000128)
#define EFIFO_CTRL1_E3_REG (0x1021c014)
#define RG_PHY_MODE_SEL    (1 << 0)
#define RG_COMBO_MODE_SEL  (1 << 1)

#define EFIFO_MODE_REG    (0x10270000)
#define EFIFO_REG_OFFSET  (0x1000)
#define EFIFO_MODE_1000   (0x0f)
#define EFIFO_MODE_100_10 (0x0c)
/* MACRO FUNCTION DECLARATIONS
 */

#define GET_PSR_LINK_STS(data, mac_port)         ((data & (PSR_LINK_STS_MASK << ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))) >> ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))
#define GET_PSR_LINK_SPEED(data, mac_port)       ((data & (PSR_LINK_SPEED_MASK << (PSR_LINK_SPEED_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))) >> (PSR_LINK_SPEED_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))
#define GET_PSR_LINK_DUPLEX(data, mac_port)      ((data & (PSR_LINK_DUPLEX_MASK << (PSR_LINK_DUPLEX_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))) >> (PSR_LINK_DUPLEX_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))
#define GET_SFP_LP_PAUSE_ADV(data, mac_port)     ((data & (SFP_LP_PAUSE_ADV_MASK << (SFP_LP_PAUSE_ADV_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))) >> (SFP_LP_PAUSE_ADV_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))
#define GET_SFP_LP_ASM_PAUSE_ADV(data, mac_port) ((data & (SFP_LP_ASM_PAUSE_ADV_MASK << (SFP_LP_ASM_PAUSE_ADV_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))) >> (SFP_LP_ASM_PAUSE_ADV_OFFSET + ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET)))

#define SET_PHY_ABILITY(flag, cod, sb, result) \
    do                                         \
    {                                          \
        if (flag & cod)                        \
        {                                      \
            result |= sb;                      \
        }                                      \
        else                                   \
        {                                      \
            result &= ~(sb);                   \
        }                                      \
    } while (0)

#define GET_PHY_ABILITY(reg, cod, sb, result) \
    do                                        \
    {                                         \
        if (reg & cod)                        \
        {                                     \
            result |= sb;                     \
        }                                     \
    } while (0)

#define SERDES_ID_S0_BASE_ADDR    (0x10220000)
#define SERDES_ADDR_OFFSER        (0x10000)
#define QSGMII_LANE_PORT_NUM      (0x4)
#define QSGMII_LANE_OFFSET        (0x1000)
#define SERDES_AN_REG_13          (0x0034)
#define SERDES_AN_REG_4           (0x0010)
#define SERDES_AN_REG_0           (0x0000)
#define SERDES_AN_REG_0_RESET     (0x8000) /* Serdes reset phy */
#define SERDES_AN_REG_0_ANRESTART (0x0200) /* Serdes auto negotation restart */
#define SERDES_AN_REG_0_ANENABLE  (0x1000) /* Serdes enable auto negotation */
#define SERDES_PHYA_REG_61        (0xA0F4)
#define SERDES_SGMII_ENABLE       (0x01)
#define SERDES_POWER_DOWN         (0x400)
#define SERDES_TX_DISABLE         (0x2000)
#define SERDES_FORCE_SYNC_DISABLE (0x2)

#define REG_IF_MODE_OFFT               (0)
#define REG_IF_MODE_LENG               (6)
#define REG_IF_MODE_1000BASEX          (0x22)
#define REG_IF_MODE_SGMII              (0x23)
#define REG_IF_MODE_FORCE              (0x9)
#define REG_TX_ABILITY_SGMII_OFFT      (0)
#define REG_TX_ABILITY_SGMII_LENG      (1)
#define REG_1000BASEX_FULL_DUPLEX_OFFT (5)
#define REG_1000BASEX_PAUSE_OFFT       (7)
#define REG_1000BASEX_ASM_DIR_OFFT     (8)
#define REG_SGMII_AN_RESTART_OFFT      (9)

#define PHY_SMI           (0x1021801C)
#define CSR_SMI_PMDC_MASK (0xc0)
#define CSR_SMI_PMDC_OFFT (6)

#define SERDES_XGMII_DBG_0            (0x60E0)
#define SERDES_XGMII_DBG_1            (0x60E4)
#define SERDES_MSG_TX_CTRL_REG_1      (0x4004)
#define SERDES_SGMII_STS_CTL_REG_0    (0x4018)
#define SERDES_MSG_RX_CTRL_REG_4      (0x450C)
#define SERDES_PHYA_REG_11            (0xA02C)
#define SERDES_PHYA_REG_19            (0xA04C)
#define SERDES_PHYA_REG_80            (0xA140)
#define SERDES_RATE_ADP_P0_CTRL_REG_0 (0x6100)
#define SERDES_PCS_CTRL_REG_1         (0x0A00)
#define SERDES_PCS_STATE_REG_2        (0x0B04)
#define SERDES_RA_P0_MII_RX_EEE_EN    (1UL << 30)
#define SERDES_RA_P0_MII_TX_EEE_EN    (1UL << 29)
#define SERDES_RA_P0_MII_MODE         (1UL << 28)
#define SERDES_RA_P0_MII_RA_RX_EN     (1UL << 3)
#define SERDES_RA_P0_MII_RA_TX_EN     (1UL << 2)
#define SERDES_RA_P0_MII_RA_RX_MODE   (1UL << 1)
#define SERDES_RA_P0_MII_RA_TX_MODE   (1UL << 0)

#define SERDES_RATE_ADP_P0_CTRL_REG_1 (0x6104)
#define SERDES_MII_RA_AN_ENABLE       (0x6300)
#define SERDES_PHYA_REG_30            (0xA078)

/* DATA TYPE DECLARATIONS
 */

#define NUMBER_OF_SERDES_FORCE_SPEED_RG (14)
typedef enum
{
    SERDES_SGMII_REGISTER_ADDRESS = 0,
    SERDES_SGMII_AN,
    SERDES_SGMII_FORCE_1000M,
    SERDES_SGMII_FORCE_100M,
    SERDES_SGMII_FORCE_10M,
    SERDES_SGMII_FORCE_LAST
} SERDES_SGMII_FORCE_SPEED_T;

typedef struct HAL_EN8808_PHY_DBG_CFG_S
{
    UI8_T  operation;
    UI32_T reg_addr;
    UI16_T delay_time;
    UI16_T data_msb;
    UI16_T data_lsb;
    UI32_T reg_data;
    C8_T  *reg_name;
} HAL_EN8808_PHY_DBG_CFG_T;

typedef struct HAL_EN8808_PHY_DBG_CFG_INFO_S
{
    UI32_T                          serdes_interface;
    UI32_T                          serdes_register_cnt;
    const HAL_EN8808_PHY_DBG_CFG_T *ptr_serdes_dbg_cfg_map;
} HAL_EN8808_PHY_DBG_CFG_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

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
    const UI32_T port);

/* FUNCTION NAME: hal_en8808_phy_getDriver
 * PURPOSE:
 *      EN8808 PHY probe get driver
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
hal_en8808_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param);

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
    const HAL_PHY_ADMIN_STATE_T state);

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
    HAL_PHY_ADMIN_STATE_T *ptr_state);

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
    const HAL_PHY_AUTO_NEGO_T auto_nego);

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
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego);

/* FUNCTION NAME:   hal_en8808_phy_setLocalAdvAbility
 * PURPOSE:
 *      This API is used to set port local advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      an_adv          --  Advertisement ability
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
    const HAL_PHY_AN_ADV_T *ptr_adv);

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
    HAL_PHY_AN_ADV_T *ptr_adv);

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
    HAL_PHY_AN_ADV_T *ptr_adv);

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
    const HAL_PHY_SPEED_T speed);

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
    HAL_PHY_SPEED_T *ptr_speed);

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
    const HAL_PHY_DUPLEX_T duplex);

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
    HAL_PHY_DUPLEX_T *ptr_duplex);

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
    HAL_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   hal_en8808_phy_setLoopBack
 * PURPOSE:
 *      This API is used to set port loop back.
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
    const BOOL_T         enable);

/* FUNCTION NAME:   hal_en8808_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get port loop back.
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
    BOOL_T              *ptr_enable);

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
    const HAL_PHY_SSD_MODE_T ssd_mode);

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
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode);

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
    const BOOL_T enable);

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
    BOOL_T      *ptr_enable);

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
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode);

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
    const HAL_PHY_COMBO_MODE_T combo_mode);

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
    HAL_PHY_COMBO_MODE_T *ptr_combo_mode);

/* FUNCTION NAME:   hal_en8808_phy_setSerdesMode
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
hal_en8808_phy_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode);
;

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
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME: hal_en8808_phy_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      ctrl_mode               -- LED control mode enumeration type
 *                                 HAL_PHY_LED_CTRL_MODE_T
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
hal_en8808_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME: hal_en8808_phy_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode           -- LED control enumeration type
 *                                 HAL_PHY_LED_CTRL_MODE_T
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
hal_en8808_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_en8808_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      pattern                 -- LED force pattern
 *                                 HAL_PHY_LED_PATT_T
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
hal_en8808_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_en8808_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_pattern             -- LED force pattern enumeration type
 *                                 HAL_PHY_LED_PATT_T
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
hal_en8808_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_en8808_phy_triggerCableTest
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
hal_en8808_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

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
    UI32_T     **pptr_raw_data_all);

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
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

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
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg);

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
    const UI32_T evt_flags);

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
    UI32_T      *ptr_evt_flags);

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
    const UI32_T                      time);

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
    UI32_T                           *ptr_time);

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
    const HAL_PHY_OP_MODE_T mode);

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
    HAL_PHY_OP_MODE_T *ptr_mode);

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
    const UI32_T port);

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
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt);

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
    const UI32_T             param);

#endif /* #ifndef HAL_EN8808_PHY_H */
