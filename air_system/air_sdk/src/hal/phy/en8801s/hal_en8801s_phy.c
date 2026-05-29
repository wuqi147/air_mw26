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

/* FILE NAME:  hal_en8801s_phy.c
 * PURPOSE:
 *  Implement en8801s phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/en8801s/hal_en8801s_phy.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define PHY_LED_BLK_CFG_REG (EN8801S_LED_3_ON_MASK)
#define PHY_LED_CFG_REG     (EN8801S_LED_3_BLK_MASK)

#define PHY_LED_TYPE_ON  (0)
#define PHY_LED_TYPE_BLK (1)
/* MACRO FUNCTION DECLARATIONS
 */
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

#define SERDES_NUMBER(phy, number)                    \
    do                                                \
    {                                                 \
        number = (phy == EN8801S_PHY_ADDR_2) ? 1 : 0; \
    } while (0)

#define LED_ID_TO_LED_REG_ADDRESS(led_id, led_type, reg_addr)                    \
    do                                                                           \
    {                                                                            \
        ((led_type) == PHY_LED_TYPE_ON ? ((reg_addr) = EN8801S_LED_0_ON_MASK) :  \
                                         ((reg_addr) = EN8801S_LED_0_BLK_MASK)); \
        (reg_addr) += (EN8801S_LED_RG_OFFSET * led_id);                          \
    } while (0)
/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_en8801s_phy.c");

/* STATIC VARIABLE DECLARATIONS */

/* table/register control blocks */
static HAL_PHY_SPEED_T preSpeed[MAX_SERDES];
static EN8801S_STATE_T procee_state[MAX_SERDES] = {EN8801S_STATE_INIT, EN8801S_STATE_INIT, EN8801S_STATE_INIT,
                                                   EN8801S_STATE_INIT, EN8801S_STATE_INIT};

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void
_en8801s_phy_set_gphy_TrReg(
    const UI32_T unit,
    const UI32_T port,
    UI16_T       parm_1,
    UI16_T       parm_2,
    UI16_T       parm_3)
{
    hal_mdio_writeC22ByPort(unit, port, RgAddr_TrReg11h, parm_1);
    hal_mdio_writeC22ByPort(unit, port, RgAddr_TrReg12h, parm_2);
    hal_mdio_writeC22ByPort(unit, port, RgAddr_TrReg10h, parm_3);
}

static UI32_T
_en8801s_phy_get_gphy_TrReg(
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
_hal_en8801s_phy_getRandomSeed(
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

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_TrReg);
    backup_vga = _en8801s_phy_get_gphy_TrReg(unit, port, 0xafa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_Reg);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x140);
    value = backup_dev1e_reg03ch | 0x8800;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, value);

    value = backup_dev1f_reg015h & (~0xF1FF);
    value |= 0x108b;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, value);

    value = backup_vga | 0x001f0000;
    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8801s_phy_set_gphy_TrReg(unit, port, (value & 0xffff), (value >> 16) & 0xffff, 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_Reg);

    for (index = 0; index < 32; index++)
    {
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        reg_data &= 0xff;
        seed |= ((reg_data & 0x1) << index);
    }

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1000);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8801s_phy_set_gphy_TrReg(unit, port, (backup_vga & 0xffff), ((backup_vga >> 16) & 0xffff), 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, CL22_Page_Reg);

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
_hal_en8801s_phy_calRandomSeed(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T link_status)
{
    UI32_T         seed = 0;
    UI16_T         reg_value = 0, reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (TRUE == link_status)
    {
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
    return rv;
}

static UI32_T
_en8801s_pbus_rd(
    UI32_T unit,
    UI16_T pbus_addr,
    UI32_T reg_addr)
{
    UI16_T data_high = 0, data_low = 0;
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x1f, (UI16_T)(reg_addr >> 6));
    hal_mdio_readC22(unit, 0, pbus_addr, (UI16_T)((reg_addr >> 2) & 0x0f), &data_low);
    hal_mdio_readC22(unit, 0, pbus_addr, 0x10, &data_high);
    return ((data_high << 16) | data_low);
}

static void
_en8801s_pbus_wr(
    UI32_T unit,
    UI16_T pbus_addr,
    UI32_T reg_addr,
    UI32_T pbus_data)
{
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x1f, (UI16_T)(reg_addr >> 6));
    hal_mdio_writeC22(unit, 0, pbus_addr, (UI16_T)((reg_addr >> 2) & 0x0f), (UI16_T)(pbus_data & 0xffff));
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x10, (UI16_T)(pbus_data >> 16));
    return;
}

/* EN8801 Token Ring Write function */
static void
_en8801s_TR_RegWr(
    UI32_T unit,
    UI16_T pbus_addr,
    UI32_T tr_reg_addr,
    UI32_T tr_data)
{
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x1F, 0x52b5); /* page select */
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x11, (UI16_T)(tr_data & 0xffff));
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x12, (UI16_T)(tr_data >> 16));
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x10, (UI16_T)(tr_reg_addr | TrReg_WR));
    hal_mdio_writeC22(unit, 0, pbus_addr, 0x1F, 0x0); /* page resetore */
    return;
}

static AIR_ERROR_NO_T
_en8801s_phy_process(
    const UI32_T unit,
    const UI16_T pbus_addr)
{
    UI32_T         reg_value = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* 10M odd nibble patch */
    reg_value = _en8801s_pbus_rd(unit, pbus_addr, 0x19e0);
    reg_value |= (1 << 0);
    _en8801s_pbus_wr(unit, pbus_addr, 0x19e0, reg_value);
    reg_value = _en8801s_pbus_rd(unit, pbus_addr, 0x19e0);
    reg_value &= ~(1 << 0);
    _en8801s_pbus_wr(unit, pbus_addr, 0x19e0, reg_value);
    return rv;
}

static AIR_ERROR_NO_T
_en8801s_phase2_init(
    const UI32_T unit,
    const UI16_T port,
    const UI16_T phy_addr)
{
    gephy_all_REG_LpiReg1Ch      GPHY_RG_LPI_1C;
    gephy_all_REG_dev1Eh_reg324h GPHY_RG_1E_324;
    gephy_all_REG_dev1Eh_reg012h GPHY_RG_1E_012;
    gephy_all_REG_dev1Eh_reg017h GPHY_RG_1E_017;
    AIR_ERROR_NO_T               rv = AIR_E_OK;
    UI32_T                       pbus_data = 0;
    UI16_T                       pbus_addr = 0, reg_value = 0, retry = 0;
    AIR_CFG_VALUE_T              led_behavior;
    UI16_T                       led_id = 0, led_config = 0, led_count = 0;
    UI16_T                       link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    UI32_T                       random_seed = 0, seed = 0;
    HAL_PHY_LED_CTRL_MODE_T      led_ctrl_mode = HAL_PHY_LED_CTRL_MODE_LAST;

    DIAG_PRINT(HAL_DBG_INFO, " phyAddr=(%x)\n", phy_addr);
    pbus_addr = phy_addr + 1;

    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1690);
    _en8801s_pbus_wr(unit, pbus_addr, 0x1690, pbus_data | (1 << 31));

    _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c000c00);
    _en8801s_pbus_wr(unit, pbus_addr, 0x10, 0xD801);
    _en8801s_pbus_wr(unit, pbus_addr, 0x0, 0x9140);

    _en8801s_pbus_wr(unit, pbus_addr, 0x0A14, 0x0003);
    _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c000c00);
    /* Set FCM control */
    _en8801s_pbus_wr(unit, pbus_addr, 0x1404, 0x004b);
    _en8801s_pbus_wr(unit, pbus_addr, 0x140c, 0x0007);

    _en8801s_pbus_wr(unit, pbus_addr, 0x142c, 0x05050505);
    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1440);
    _en8801s_pbus_wr(unit, pbus_addr, 0x1440, pbus_data & ~(1 << 11)); /* disable PAUSE frame bybass */

    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1408);
    _en8801s_pbus_wr(unit, pbus_addr, 0x1408, pbus_data | (1 << 5)); /* system side eee enable */
    /* Set GPHY Perfomance*/
    /* Token Ring */
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_R1000DEC_15h, 0x0055A0);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_R1000DEC_17h, 0x07ff3f);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_PMA_00h, 0x00001e);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_PMA_01h, 0x6FB90A);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_PMA_17h, 0x060671);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_PMA_18h, 0x0E2F00);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_TR_26h, 0x444444);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_03h, 0x000000);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_06h, 0x2EBAEF);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_08h, 0x00000B);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_0Ch, 0x00504D);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_0Dh, 0x02314F);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_0Fh, 0x003028);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_10h, 0x005010);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_11h, 0x040001);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_13h, 0x018670);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_14h, 0x00024A);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_1Bh, 0x000072);
    _en8801s_TR_RegWr(unit, pbus_addr, RgAddr_DSPF_1Ch, 0x003210);

    /* CL22 & CL45 */
    hal_mdio_writeC22(unit, 0, phy_addr, 0x1F, 0x03);
    hal_mdio_readC22(unit, 0, phy_addr, RgAddr_LPI_1Ch, &GPHY_RG_LPI_1C.DATA);
    GPHY_RG_LPI_1C.DataBitField.smi_deton_th = 0x0C;
    hal_mdio_writeC22(unit, 0, phy_addr, RgAddr_LPI_1Ch, GPHY_RG_LPI_1C.DATA);
    hal_mdio_writeC22(unit, 0, phy_addr, RgAddr_LPI_1Ch, 0xc92);
    hal_mdio_writeC22(unit, 0, phy_addr, RgAddr_AUXILIARY_1Dh, 0x1);
    hal_mdio_writeC22(unit, 0, phy_addr, 0x1f, 0x0);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x120, 0x8014);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x122, 0xffff);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x123, 0xffff);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x144, 0x0200);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x14A, 0xEE20);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x189, 0x0110);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x19B, 0x0111);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x234, 0x0181);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x238, 0x0120);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x239, 0x0117);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1F, 0x268, 0x07F4);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x2d1, 0x0733);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x323, 0x0011);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x324, 0x013f);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x326, 0x0037);

    hal_mdio_readC45(unit, 0, phy_addr, 0x1E, 0x324, &reg_value);
    GPHY_RG_1E_324.DATA = reg_value;
    GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off = 0;
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x324, (UI16_T)GPHY_RG_1E_324.DATA);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x19E, 0xC2);
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x013, 0x0);

    /* EFUSE */
    _en8801s_pbus_wr(unit, pbus_addr, 0x1C08, 0x40000040);
    retry = MAX_RETRY;
    while (0 != retry)
    {
        osal_delayUs(1000);
        pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1C08);
        if ((pbus_data & (1 << 30)) == 0)
        {
            break;
        }
        retry--;
    }
    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1C38); /* RAW#2 */
    GPHY_RG_1E_012.DataBitField.da_tx_i2mpb_a_tbt = pbus_data & 0x03f;
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x12, (UI16_T)GPHY_RG_1E_012.DATA);
    GPHY_RG_1E_017.DataBitField.da_tx_i2mpb_b_tbt = (reg_value >> 8) & 0x03f;
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x12, (UI16_T)GPHY_RG_1E_017.DATA);

    _en8801s_pbus_wr(unit, pbus_addr, 0x1C08, 0x40400040);
    retry = MAX_RETRY;
    while (0 != retry)
    {
        osal_delayUs(1000);
        pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1C08);
        if ((pbus_data & (1 << 30)) == 0)
        {
            break;
        }
        retry--;
    }
    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1C30); /* RAW#16 */
    GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off = (pbus_data >> 12) & 0x01;
    hal_mdio_writeC45(unit, 0, phy_addr, 0x1E, 0x324, (UI16_T)GPHY_RG_1E_324.DATA);

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = EN8801S_LED_COUNT;
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    /* LED configuration */
    for (led_id = 0; led_id < led_count; led_id++)
    {
        if (led_id == 0)
        {
            link_reg_addr = EN8801S_LED_0_ON_MASK;
            blk_reg_addr = EN8801S_LED_0_BLK_MASK;
        }
        else if (led_id == 1)
        {
            link_reg_addr = EN8801S_LED_1_ON_MASK;
            blk_reg_addr = EN8801S_LED_1_BLK_MASK;
        }
        else if (led_id == 2)
        {
            link_reg_addr = EN8801S_LED_2_ON_MASK;
            blk_reg_addr = EN8801S_LED_2_BLK_MASK;
        }
        else
        {
            link_reg_addr = EN8801S_LED_3_ON_MASK;
            blk_reg_addr = EN8801S_LED_3_BLK_MASK;
        }

        hal_en8801s_phy_getPhyLedCtrlMode(0, port, led_id, &led_ctrl_mode);

        if (HAL_PHY_LED_CTRL_MODE_PHY == led_ctrl_mode)
        {
            _en8801s_pbus_wr(unit, pbus_addr, 0x186c, 0x3);
            _en8801s_pbus_wr(unit, pbus_addr, 0X1870, 0x100);
            pbus_data = (_en8801s_pbus_rd(unit, pbus_addr, 0x1880) & ~(0x3));
            _en8801s_pbus_wr(unit, pbus_addr, 0x1880, pbus_data);
            hal_mdio_writeC45(unit, 0, phy_addr, 0x1f, 0x21, 0x8008);
            hal_mdio_writeC45(unit, 0, phy_addr, 0x1f, 0x22, 0x600);
            hal_mdio_writeC45(unit, 0, phy_addr, 0x1f, 0x23, 0xc00);

            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

            led_behavior.value = 0xFFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = led_id;

            hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

            led_config = led_behavior.value;

            link_reg_data = 0;
            blk_reg_data = 0;

            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_1000, EN8801S_LED_LINK_1000, link_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_100, EN8801S_LED_LINK_100, link_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_10, EN8801S_LED_LINK_10, link_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_FULLDPLX, EN8801S_LED_LINK_FULLDPLX, link_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_HALFDPLX, EN8801S_LED_LINK_HALFDPLX, link_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_HIGH_ACTIVE, EN8801S_LED_POL_HIGH_ACT, link_reg_data);

            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_1000, EN8801S_LED_BLINK_TX_1000, blk_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_1000, EN8801S_LED_BLINK_RX_1000, blk_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_100, EN8801S_LED_BLINK_TX_100, blk_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_100, EN8801S_LED_BLINK_RX_100, blk_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_10, EN8801S_LED_BLINK_TX_10, blk_reg_data);
            GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_10, EN8801S_LED_BLINK_RX_10, blk_reg_data);

            if (link_reg_data != 0 || blk_reg_data != 0)
            {
                link_reg_data |= EN8801S_LED_FUNC_ENABLE;
            }

            hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
            hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
        }
    }

    /* link down and up */
    hal_mdio_readC22(unit, 0, phy_addr, MII_BMCR, &reg_value);
    reg_value |= BMCR_PDOWN;
    hal_mdio_writeC22(unit, 0, phy_addr, MII_BMCR, reg_value);
    osal_delayUs(100000);
    hal_mdio_readC22(unit, 0, phy_addr, MII_BMCR, &reg_value);
    reg_value &= ~(BMCR_PDOWN);
    hal_mdio_writeC22(unit, 0, phy_addr, MII_BMCR, reg_value);

    /* Internal buck power adjustment for eee application */
    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x1960);
    pbus_data -= (2 << 22);
    _en8801s_pbus_wr(unit, pbus_addr, 0x1960, pbus_data);
    osal_delayUs(10000);
    pbus_data -= (2 << 22);
    _en8801s_pbus_wr(unit, pbus_addr, 0x1960, pbus_data);

    rv = _hal_en8801s_phy_getRandomSeed(unit, port, &random_seed);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x12, &reg_value);
        if (AIR_E_OK == rv)
        {
            random_seed += reg_value;
        }

        osal_srand(random_seed);

        rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x200); /* gphy power up */

        seed = osal_rand() % 100000;
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, &reg_value);

        reg_value &= ~(0xF);
        reg_value |= (0x10 | (seed & 0xF)); /* use seed 0~3bit as random seed */

        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, reg_value);
        osal_delayUs(seed);
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x327, reg_value & 0xFFEF);

        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, 0x1000);
        rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1200); /* re-AN */
    }

    return rv;
}

static AIR_ERROR_NO_T
_genphy_update_link(
    const UI32_T           unit,
    const UI16_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         status = 0;

    /* Do a fake read */
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    /* Read link and autonegotiation status */
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    if (status & BMSR_LSTATUS)
    {
        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
    }
    else
    {
        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
    }
    if (status & BMSR_ANEGCOMPLETE)
    {
        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
    }
    else
    {
        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE);
    }
    if (status & BMSR_RFAULT)
    {
        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT;
    }
    else
    {
        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT);
    }
    return rv;
}

static AIR_ERROR_NO_T
_genphy_read_status(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    HAL_PHY_AUTO_NEGO_T auto_nego = HAL_PHY_AUTO_NEGO_LAST;
    UI16_T              adv = 0, lpa = 0, lpagb = 0, bmcr = 0, common_adv_gb = 0, common_adv = 0;

    rv = _genphy_update_link(unit, port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    hal_en8801s_phy_getAutoNego(unit, port, &auto_nego);

    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &lpagb);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
        rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &adv);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
        common_adv_gb = lpagb & adv << 2;

        rv = hal_mdio_readC22ByPort(unit, port, MII_LPA, &lpa);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
        rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &adv);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
        common_adv = lpa & adv;

        ptr_status->speed = HAL_PHY_SPEED_10M;
        ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
        /*phydev->pause = phydev->asym_pause = 0; */

        if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF))
        {
            ptr_status->speed = HAL_PHY_SPEED_1000M;
            if (common_adv_gb & LPA_1000FULL)
            {
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
        }
        else if (common_adv & (LPA_100FULL | LPA_100HALF))
        {
            ptr_status->speed = HAL_PHY_SPEED_100M;
            if (common_adv & LPA_100FULL)
            {
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
        }
        else
        {
            if (common_adv & LPA_10FULL)
            {
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
        }
        /*
        if (ptr_status->duplex == DUPLEX_FULL)
        {
            phydev->pause = lpa & LPA_PAUSE_CAP ? 1 : 0;
            phydev->asym_pause = lpa & LPA_PAUSE_ASYM ? 1 : 0;
        }
        */
    }
    else
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &bmcr);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
        if (bmcr & BMCR_FULLDPLX)
        {
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
            ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
        }
        if (bmcr & BMCR_SPEED1000)
        {
            ptr_status->speed = HAL_PHY_SPEED_1000M;
        }
        else if (bmcr & BMCR_SPEED100)
        {
            ptr_status->speed = HAL_PHY_SPEED_100M;
        }
        else
        {
            ptr_status->speed = HAL_PHY_SPEED_10M;
        }
        /* phydev->pause = phydev->asym_pause = 0; */
    }
    return rv;
}

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME: hal_en8801s_phy_init
 * PURPOSE:
 *      EN8801S PHY initialization
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  PHY address
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T pbus_data = 0;
    UI16_T pbus_addr = 0;
    UI8_T  phyAddr;

    osal_memset(&preSpeed[0], NO_SPEED, sizeof(HAL_PHY_SPEED_T) * MAX_SERDES);

    phyAddr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    if (phyAddr >= MAX_PHYS)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: en8801s %d initialize fail\n", port);
        return AIR_E_BAD_PARAMETER;
    }
    pbus_addr = phyAddr + 1;

    /* updata revision ID here */
    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, EN8801S_RG_ECO) & 0x0f;
    if (HAL_PHY_PORT_DEV_REVISION_ID(unit, port) != pbus_data)
    {
        HAL_PHY_PORT_DEV_REVISION_ID(unit, port) = pbus_data;
    }
    if (pbus_data < EN8801S_ECO_E4)
    {
        osal_delayUs(1500000L); /* wait for EN8801S CPU boot up */
        DIAG_PRINT(HAL_DBG_INFO, " (%u) phyAddr=(%x)\n", unit, phyAddr);
    }
    DIAG_PRINT(HAL_DBG_ERR, " (%u) phyAddr=(%x)\n", unit, phyAddr);
    /* SGMII set polarity and loopback for 10 ms */
    pbus_data = (_en8801s_pbus_rd(unit, pbus_addr, EN8801S_RG_LTR_CTL) & 0xfffffffc) | 0x12;
    _en8801s_pbus_wr(unit, pbus_addr, EN8801S_RG_LTR_CTL, pbus_data);
    osal_delayUs(10000);
    pbus_data &= 0xffffffef;
    _en8801s_pbus_wr(unit, pbus_addr, EN8801S_RG_LTR_CTL, pbus_data);

    pbus_data = _en8801s_pbus_rd(unit, pbus_addr, 0x34);
    pbus_data |= (1 << 8); /* workaround to align duplex */
    _en8801s_pbus_wr(unit, pbus_addr, 0x34, pbus_data);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_en8801s_phy_setAdminState
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
hal_en8801s_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_ADMIN_STATE_DISABLE == state)
        {
            reg_value |= BMCR_PDOWN;
        }
        else
        {
            reg_value &= ~(BMCR_PDOWN);
        }
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getAdminState
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
hal_en8801s_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (reg_value & BMCR_PDOWN)
        {
            *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
        }
        else
        {
            *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setAutoNego
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
hal_en8801s_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
        {
            reg_value |= BMCR_ANENABLE;
        }
        else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
        {
            reg_value |= BMCR_ANRESTART;
        }
        else
        {
            reg_value &= ~(BMCR_ANENABLE);
        }
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getAutoNego
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
hal_en8801s_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (reg_value & BMCR_ANENABLE)
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

/* FUNCTION NAME:   hal_en8801s_phy_setLocalAdvAbility
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
hal_en8801s_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_value);
    if (AIR_E_OK == rv)
    {
        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ADVERTISE_1000FULL, reg_value);
        rv = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, reg_value);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_value);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10HFDX, ADVERTISE_10HALF, reg_value);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10FUDX, ADVERTISE_10FULL, reg_value);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100HFDX, ADVERTISE_100HALF, reg_value);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100FUDX, ADVERTISE_100FULL, reg_value);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ADVERTISE_PAUSE_CAP, reg_value);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ADVERTISE_PAUSE_ASYM, reg_value);
            rv = hal_mdio_writeC22ByPort(unit, port, MII_ADVERTISE, reg_value);
        }
    }

    if (AIR_E_OK == rv)
    {
        reg_value = (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) ? (EEE_1000BASE_T | EEE_100BASE_TX) : 0;
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_value);
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getLocalAdvAbility
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
hal_en8801s_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    ptr_adv->flags = 0;
    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, ADVERTISE_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, ADVERTISE_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getRemoteAdvAbility
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
hal_en8801s_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    ptr_adv->flags = 0;

    /* Read the remote advertisement ability information after AN is complete */
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_value);
    if (AIR_E_OK == rv)
    {
        /* AN not complete, it would not get remote-adv information */
        if ((reg_value & BMSR_ANEGCOMPLETE))
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &reg_value);
            if (AIR_E_OK == rv)
            {
                GET_PHY_ABILITY(reg_value, LPA_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
            }

            rv = hal_mdio_readC22ByPort(unit, port, MII_LPA, &reg_value);
            if (AIR_E_OK == rv)
            {
                GET_PHY_ABILITY(reg_value, LPA_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
                GET_PHY_ABILITY(reg_value, LPA_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
                GET_PHY_ABILITY(reg_value, LPA_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
                GET_PHY_ABILITY(reg_value, LPA_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
                GET_PHY_ABILITY(reg_value, LPA_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
                GET_PHY_ABILITY(reg_value, LPA_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
            }

            rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &reg_value);
            if (AIR_E_OK == rv)
            {
                GET_PHY_ABILITY(reg_value, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setSpeed
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
hal_en8801s_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    if (speed >= HAL_PHY_SPEED_1000M)
    {
        return AIR_E_OP_INVALID;
    }

    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value)) != AIR_E_OK)
    {
        return rv;
    }

    reg_value &= ~(BMCR_SPEED1000 | BMCR_SPEED100);
    switch (speed)
    {
        case HAL_PHY_SPEED_100M:
            reg_value |= BMCR_SPEED100;
            break;
        case HAL_PHY_SPEED_10M:
            reg_value |= 0;
            break;
        default:
            rv = AIR_E_NOT_SUPPORT;
            break;
    }
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getSpeed
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
hal_en8801s_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

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

/* FUNCTION NAME:   hal_en8801s_phy_setDuplex
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
hal_en8801s_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_DUPLEX_FULL == duplex)
        {
            reg_value |= BMCR_FULLDPLX;
        }
        else if (HAL_PHY_DUPLEX_HALF == duplex)
        {
            reg_value &= ~(BMCR_FULLDPLX);
        }
        else
        {
            return AIR_E_NOT_SUPPORT;
        }
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getDuplex
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
hal_en8801s_phy_getDuplex(
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

/* FUNCTION NAME:   hal_en8801s_phy_getLinkStatus
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
hal_en8801s_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         phy_addr = 0, pbus_addr = 0, serdes = 0;
    UI32_T         reg_value;
    UI8_T          phyAddr = 0;

    rv = _genphy_read_status(unit, port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] flags=(%x)\n", unit, port, ptr_status->flags);

    phyAddr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    if (phyAddr >= MAX_PHYS)
    {
        return AIR_E_BAD_PARAMETER;
    }

    phy_addr = phyAddr;
    pbus_addr = phyAddr + 1;
    SERDES_NUMBER(phy_addr, serdes);

    if (EN8801S_STATE_PROCESS == procee_state[serdes])
    {
        _en8801s_phy_process(unit, pbus_addr);
        procee_state[serdes] = EN8801S_STATE_DONE;
    }

    if (0 == (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP))
    {
        if (NO_SPEED != preSpeed[serdes])
        {
            reg_value = _en8801s_pbus_rd(unit, pbus_addr, EN8801S_PCS_CTRL1);
            _en8801s_pbus_wr(unit, pbus_addr, EN8801S_PCS_CTRL1,
                             ((reg_value & ~SGMII_PCS_FORCE_SYNC_MASK) | SGMII_PCS_FORCE_SYNC_OFF));
        }
        preSpeed[serdes] = NO_SPEED;
        rv = _hal_en8801s_phy_calRandomSeed(unit, port, FALSE);
        return rv;
    }

    /* set rate adaption and re-an */
    if (preSpeed[serdes] != ptr_status->speed)
    {
        reg_value = _en8801s_pbus_rd(unit, pbus_addr, EN8801S_PCS_CTRL1);
        _en8801s_pbus_wr(unit, pbus_addr, EN8801S_PCS_CTRL1, (reg_value & ~SGMII_PCS_FORCE_SYNC_MASK));
        osal_delayUs(2000); /* delay 2 ms */
        preSpeed[serdes] = ptr_status->speed;
        if (EN8801S_STATE_INIT == procee_state[serdes])
        {
            rv = _en8801s_phase2_init(unit, port, phy_addr);
            if (AIR_E_OK != rv)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8801s %d initialize fail(%d)\n", port, rv);
                return rv;
            }
            procee_state[serdes] = EN8801S_STATE_PROCESS;
        }

        if (HAL_PHY_SPEED_10M == ptr_status->speed)
        {
            reg_value = _en8801s_pbus_rd(unit, pbus_addr, 0x1694);
            reg_value |= (1 << 31);
            _en8801s_pbus_wr(unit, pbus_addr, 0x1694, reg_value);
            procee_state[serdes] = EN8801S_STATE_PROCESS;
        }
        else
        {
            reg_value = _en8801s_pbus_rd(unit, pbus_addr, 0x1694);
            reg_value &= ~(1 << 31);
            _en8801s_pbus_wr(unit, pbus_addr, 0x1694, reg_value);
            procee_state[serdes] = EN8801S_STATE_PROCESS;
        }

        _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c000c00);
        if (HAL_PHY_SPEED_1000M == ptr_status->speed)
        {
            DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] speed=1000M\n", unit, port);
            reg_value = 0xC801;
            if (HAL_PHY_DUPLEX_FULL == ptr_status->duplex)
            {
                reg_value |= EN8801S_PHY_DUPLEX_BIT;
            }
            _en8801s_pbus_wr(unit, pbus_addr, 0x10, reg_value);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0, 0x9140);

            _en8801s_pbus_wr(unit, pbus_addr, 0x0A14, 0x0003);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c000c00);
            osal_delayUs(2000); /* delay 2 ms */
            _en8801s_pbus_wr(unit, pbus_addr, 0x1404, 0x004b);
            _en8801s_pbus_wr(unit, pbus_addr, 0x140c, 0x0007);
        }
        else if (HAL_PHY_SPEED_100M == ptr_status->speed)
        {
            DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] speed=100M\n", unit, port);
            reg_value = 0xC401;
            if (HAL_PHY_DUPLEX_FULL == ptr_status->duplex)
            {
                reg_value |= EN8801S_PHY_DUPLEX_BIT;
            }
            _en8801s_pbus_wr(unit, pbus_addr, 0x10, reg_value);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0, 0x9140);

            _en8801s_pbus_wr(unit, pbus_addr, 0x0A14, 0x0007);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c11);
            osal_delayUs(2000); /* delay 2 ms */
            _en8801s_pbus_wr(unit, pbus_addr, 0x1404, 0x0027);
            _en8801s_pbus_wr(unit, pbus_addr, 0x140c, 0x0007);
        }
        else if (HAL_PHY_SPEED_10M == ptr_status->speed)
        {
            DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] speed=10M\n", unit, port);
            reg_value = 0xC001;
            if (HAL_PHY_DUPLEX_FULL == ptr_status->duplex)
            {
                reg_value |= EN8801S_PHY_DUPLEX_BIT;
            }
            _en8801s_pbus_wr(unit, pbus_addr, 0x10, reg_value);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0, 0x9140);

            _en8801s_pbus_wr(unit, pbus_addr, 0x0A14, 0x000b);
            _en8801s_pbus_wr(unit, pbus_addr, 0x0600, 0x0c11);
            osal_delayUs(2000); /* delay 2 ms */
            _en8801s_pbus_wr(unit, pbus_addr, 0x1404, 0x0027);
            _en8801s_pbus_wr(unit, pbus_addr, 0x140c, 0x0007);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, " (%u) port[%d] invalid speed !\n", unit, port);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setLoopBack
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
hal_en8801s_phy_setLoopBack(
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
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, page)) != AIR_E_OK)
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

/* FUNCTION NAME:   hal_en8801s_phy_getLoopBack
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
hal_en8801s_phy_getLoopBack(
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
        if ((rv = hal_mdio_readC22ByPort(unit, port, MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
        if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, page)) != AIR_E_OK)
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

/* FUNCTION NAME:   hal_en8801s_phy_setSmartSpeedDown
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
hal_en8801s_phy_setSmartSpeedDown(
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
    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
    if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if ((rv = hal_mdio_writeC22ByPort(unit, port, 0x14, data)) != AIR_E_OK)
    {
        return rv;
    }

    /* Restore page */
    if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getSmartSpeedDown
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
hal_en8801s_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Backup page */
    if ((rv = hal_mdio_readC22ByPort(unit, port, MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
    if ((rv = hal_mdio_writeC22ByPort(unit, port, MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setLedOnCtrl
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
hal_en8801s_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    UI16_T         reg_data = 0, reg_addr = 0, phyAddr = 0, pbus_addr = 0, pbus_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = EN8801S_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8801S_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = EN8801S_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = EN8801S_LED_3_ON_MASK;
    }

    phyAddr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    pbus_addr = phyAddr + 1;

    /* LED configuration */
    _en8801s_pbus_wr(unit, pbus_addr, 0x186c, 0x3);
    _en8801s_pbus_wr(unit, pbus_addr, 0X1870, 0x100);
    pbus_data = (_en8801s_pbus_rd(unit, pbus_addr, 0x1880) & ~(0x3));
    _en8801s_pbus_wr(unit, pbus_addr, 0x1880, pbus_data);

    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21, 0x8008);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22, 0x600);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x23, 0xc00);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (TRUE == enable)
            {
                reg_data |= EN8801S_LED_LINK_FORCE_ON;
            }
            else
            {
                reg_data &= ~(EN8801S_LED_LINK_FORCE_ON);
            }

            if (reg_data != 0)
            {
                reg_data |= EN8801S_LED_FUNC_ENABLE;
            }

            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getLedOnCtrl
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
hal_en8801s_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = EN8801S_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8801S_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = EN8801S_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = EN8801S_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & EN8801S_LED_LINK_FORCE_ON)
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

/* FUNCTION NAME:   hal_en8801s_phy_testTxCompliance
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
hal_en8801s_phy_testTxCompliance(
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

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to set phy led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      ctrl_mode       --  LED control mode enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    UI16_T              data = 0, led_config = 0, offset = 0, phy_addr = 0, pbus_addr = 0, pbus_data = 0;
    UI16_T              link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    BOOL_T              is_force = FALSE;
    HAL_PHY_LED_STATE_T state;
    AIR_CFG_VALUE_T     led_behavior;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8801S_LED_0_CFG_CTRL_MODE_BIT + (EN8801S_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(EN8801S_LED_CFG_CTRL_MODE_MASK, offset, EN8801S_LED_CFG_CTRL_MODE_WIDTH));
        data |= BITS_OFF_L(ctrl_mode, offset, EN8801S_LED_CFG_CTRL_MODE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
        pbus_addr = phy_addr + 1;

        /* LED configuration */
        _en8801s_pbus_wr(unit, pbus_addr, 0x186c, 0x3);
        _en8801s_pbus_wr(unit, pbus_addr, 0X1870, 0x100);
        pbus_data = (_en8801s_pbus_rd(unit, pbus_addr, 0x1880) & ~(0x3));
        _en8801s_pbus_wr(unit, pbus_addr, 0x1880, pbus_data);

        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21, 0x8008);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22, 0x600);
        }

        if (AIR_E_OK == rv)
        {
            is_force = (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode) ? TRUE : FALSE;
            if (TRUE == is_force)
            {
                rv = hal_en8801s_phy_getPhyLedForceState(unit, port, led_id, &state);
                if (AIR_E_OK == rv)
                {
                    rv = hal_en8801s_phy_setPhyLedForceState(unit, port, led_id, state);
                }
            }
            else
            {
                osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
                led_behavior.value = 0xFFFF;
                led_behavior.param0 = port;
                led_behavior.param1 = led_id;

                rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x23, 0xc00);
                    if (AIR_E_OK == rv)
                    {
                        led_config = led_behavior.value;

                        link_reg_data = 0;
                        blk_reg_data = 0;

                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, link_reg_addr);
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, blk_reg_addr);

                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_1000, EN8801S_LED_LINK_1000, link_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_100, EN8801S_LED_LINK_100, link_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_10, EN8801S_LED_LINK_10, link_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_FULLDPLX, EN8801S_LED_LINK_FULLDPLX,
                                        link_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_LINK_HALFDPLX, EN8801S_LED_LINK_HALFDPLX,
                                        link_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_HIGH_ACTIVE, EN8801S_LED_POL_HIGH_ACT,
                                        link_reg_data);

                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_1000, EN8801S_LED_BLINK_TX_1000,
                                        blk_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_1000, EN8801S_LED_BLINK_RX_1000,
                                        blk_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_100, EN8801S_LED_BLINK_TX_100,
                                        blk_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_100, EN8801S_LED_BLINK_RX_100,
                                        blk_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_TX_10, EN8801S_LED_BLINK_TX_10, blk_reg_data);
                        GET_PHY_ABILITY(led_config, EN8801S_LED_BHV_BLINK_RX_10, EN8801S_LED_BLINK_RX_10, blk_reg_data);

                        if (link_reg_data != 0 || blk_reg_data != 0)
                        {
                            link_reg_data |= EN8801S_LED_FUNC_ENABLE;
                        }

                        DIAG_PRINT(HAL_DBG_INFO, "port %u, link_reg_addr 0x%02X, link_reg_data 0x%04X\n", port,
                                   link_reg_addr, link_reg_data);
                        DIAG_PRINT(HAL_DBG_INFO, "port %u, blk_reg_addr 0x%02X, blk_reg_data 0x%04X\n", port,
                                   blk_reg_addr, blk_reg_data);

                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8801S_LED_ON_DURATION,
                                                             (EN8801S_LED_BLINK_RATE_DEFAULT / 2));
                                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8801S_LED_BLINK_DURATION,
                                                              EN8801S_LED_BLINK_RATE_DEFAULT);
                            }
                        }
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to get phy led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_ctrl_mode   --  LED control mode
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    UI16_T data = 0, offset = 0;
    ;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8801S_LED_0_CFG_CTRL_MODE_BIT + (EN8801S_LED_CFG_RG_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(data, offset, EN8801S_LED_CFG_CTRL_MODE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedForceState
 * PURPOSE:
 *      This API is used to set phy led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      state           --  LED force state enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    UI16_T                  reg_data = 0, reg_addr = 0, data = 0, offset = 0;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    BOOL_T                  is_force = FALSE;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode;
    HAL_PHY_LED_PATT_T      led_patt;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8801S_LED_0_CFG_STATE_BIT + (EN8801S_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(EN8801S_LED_CFG_STATE_MASK, offset, EN8801S_LED_CFG_STATE_WIDTH));
        data |= BITS_OFF_L(state, offset, EN8801S_LED_CFG_STATE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_en8801s_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                is_force = (HAL_PHY_LED_STATE_FORCE_PATT == state) ? TRUE : FALSE;
                if (TRUE == is_force)
                {
                    rv = hal_en8801s_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_en8801s_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                    }
                }
                else
                {
                    (HAL_PHY_LED_STATE_ON == state ? (reg_data = EN8801S_LED_LINK_FORCE_ON) : (reg_data = 0));
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr,
                                                 (reg_data | EN8801S_LED_FUNC_ENABLE));
                    if (AIR_E_OK == rv)
                    {
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, 0);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedForceState
 * PURPOSE:
 *      This API is used to get phy led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_state       --  LED force state enumeration type
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    UI16_T         data = 0, offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8801S_LED_0_CFG_STATE_BIT + (EN8801S_LED_CFG_RG_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(data, offset, EN8801S_LED_CFG_STATE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to set phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      pattern         --  LED force pattern enumeration type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    UI16_T                  reg_data = 0, reg_addr = 0, data = 0;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode;
    AIR_CFG_VALUE_T         led_behavior;
    HAL_PHY_LED_STATE_T     led_state;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

    switch (pattern)
    {
        case HAL_PHY_LED_PATT_HZ_HALF:
            reg_data = EN8801S_LED_BLINK_RATE_HZ_HALF;
            break;
        case HAL_PHY_LED_PATT_HZ_ONE:
            reg_data = EN8801S_LED_BLINK_RATE_HZ_ONE;
            break;
        case HAL_PHY_LED_PATT_HZ_TWO:
            reg_data = EN8801S_LED_BLINK_RATE_HZ_TWO;
            break;
        default:
            rv = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK == rv)
    {
        data = BITS_OFF_L(pattern, EN8801S_LED_CFG_PATT_OFFSET, EN8801S_LED_CFG_PATT_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_BLK_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_en8801s_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
        {
            rv = hal_en8801s_phy_getPhyLedForceState(unit, port, led_id, &led_state);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8801S_LED_ON_DURATION, (reg_data / 2));
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8801S_LED_BLINK_DURATION, reg_data);
                if (AIR_E_OK == rv)
                {
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, EN8801S_LED_BLINK_FORCE);
                    if (AIR_E_OK == rv)
                    {
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                        reg_data = EN8801S_LED_FUNC_ENABLE;
                        led_behavior.value = 0xFFFF;
                        led_behavior.param0 = port;
                        led_behavior.param1 = led_id;

                        hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
                        if (led_behavior.value & EN8801S_LED_BHV_HIGH_ACTIVE)
                        {
                            reg_data |= EN8801S_LED_POL_HIGH_ACT;
                        }
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8801s_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to get phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern enumeration type
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_en8801s_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    UI16_T         data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_BLK_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        *ptr_pattern = BITS_OFF_R(data, EN8801S_LED_CFG_PATT_OFFSET, EN8801S_LED_CFG_PATT_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME: hal_en8801s_phy_triggerCableTest
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
hal_en8801s_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerCableTest(unit, port, test_pair, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_getCableTestRawData
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
hal_en8801s_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedGlbCfg
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
hal_en8801s_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedGlbCfg
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
hal_en8801s_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedBlkEvent
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
hal_en8801s_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedBlkEvent
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
hal_en8801s_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_setPhyLedDuration
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
hal_en8801s_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_getPhyLedDuration
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
hal_en8801s_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_en8801s_phy_triggerLinkDownCableTest
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
hal_en8801s_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerLinkDownCableTest(unit, port, ptr_test_rslt);
    return ret;
}

HAL_PHY_DRIVER_T
_ext_EN8801S_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_en8801s_phy_init,
    hal_en8801s_phy_setAdminState,
    hal_en8801s_phy_getAdminState,
    hal_en8801s_phy_setAutoNego,
    hal_en8801s_phy_getAutoNego,
    hal_en8801s_phy_setLocalAdvAbility,
    hal_en8801s_phy_getLocalAdvAbility,
    hal_en8801s_phy_getRemoteAdvAbility,
    hal_en8801s_phy_setSpeed,
    hal_en8801s_phy_getSpeed,
    hal_en8801s_phy_setDuplex,
    hal_en8801s_phy_getDuplex,
    hal_en8801s_phy_getLinkStatus,
    hal_en8801s_phy_setLoopBack,
    hal_en8801s_phy_getLoopBack,
    hal_en8801s_phy_setSmartSpeedDown,
    hal_en8801s_phy_getSmartSpeedDown,
    hal_en8801s_phy_setLedOnCtrl,
    hal_en8801s_phy_getLedOnCtrl,
    hal_en8801s_phy_testTxCompliance,
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    NULL, /* setSerdesMode */
    NULL, /* getSerdesMode */
    hal_en8801s_phy_setPhyLedCtrlMode,
    hal_en8801s_phy_getPhyLedCtrlMode,
    hal_en8801s_phy_setPhyLedForceState,
    hal_en8801s_phy_getPhyLedForceState,
    hal_en8801s_phy_setPhyLedForcePattCfg,
    hal_en8801s_phy_getPhyLedForcePattCfg,
    hal_en8801s_phy_triggerCableTest,
    hal_en8801s_phy_getCableTestRawData,
    hal_en8801s_phy_setPhyLedGlbCfg,
    hal_en8801s_phy_getPhyLedGlbCfg,
    hal_en8801s_phy_setPhyLedBlkEvent,
    hal_en8801s_phy_getPhyLedBlkEvent,
    hal_en8801s_phy_setPhyLedDuration,
    hal_en8801s_phy_getPhyLedDuration,
    NULL, /* setPhyOpMode */
    NULL, /* getPhyOpMode */
    NULL, /* dumpPhyPara */
    hal_en8801s_phy_triggerLinkDownCableTest,
    NULL, /* dumpPortCnt */
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_en8801s_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_EN8801S_phy_func_vec;

    return (AIR_E_OK);
}
