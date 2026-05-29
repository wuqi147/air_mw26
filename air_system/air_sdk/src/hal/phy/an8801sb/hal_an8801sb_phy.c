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

/* FILE NAME:  hal_an8801sb_phy.c
 * PURPOSE:
 *  Implement an8801sb phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/an8801sb/hal_an8801sb_phy.h>

#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define PHY_LED_BLK_CFG_REG (AN8801SB_LED_3_ON_MASK)
#define PHY_LED_CFG_REG     (AN8801SB_LED_3_BLK_MASK)

#define PHY_LED_TYPE_ON  (0)
#define PHY_LED_TYPE_BLK (1)

#define DEFAULT_LED_CFG                                                                            \
    (AN8801SB_LED_BHV_LINK_1000 | AN8801SB_LED_BHV_LINK_100 | AN8801SB_LED_BHV_LINK_10 |           \
     AN8801SB_LED_BHV_BLINK_TX_1000 | AN8801SB_LED_BLINK_RX_1000 | AN8801SB_LED_BHV_BLINK_TX_100 | \
     AN8801SB_LED_BLINK_RX_100 | AN8801SB_LED_BHV_BLINK_TX_10 | AN8801SB_LED_BHV_BLINK_RX_10)
#define MAX_SGMII_AN_RETRY (100)

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

#define LED_ID_TO_LED_REG_ADDRESS(led_id, led_type, reg_addr)                     \
    do                                                                            \
    {                                                                             \
        ((led_type) == PHY_LED_TYPE_ON ? ((reg_addr) = AN8801SB_LED_0_ON_MASK) :  \
                                         ((reg_addr) = AN8801SB_LED_0_BLK_MASK)); \
        (reg_addr) += (AN8801SB_LED_RG_OFFSET * led_id);                          \
    } while (0)
/* DATA TYPE DECLARATIONS
 */
#define AN8801SB_10M_100M_LOW_POWER (0x53aa)
#define AN8801SB_1G_LOW_POWER       (0x5faa)
#define AN8801SB_LONG_REACH         (0x5302)

#define MAX_READ_EFUSE_RETRY   (1000)
#define AN8801SB_EFUSE_CTRL    (0x10004000)
#define AN8801SB_EFUSE_ADDR    (0x10004004)
#define AN8801SB_EFUSE_DONE    (0x10004008)
#define AN8801SB_EFUSE_STATUS  (0x1000400c)
#define AN8801SB_EFUSE_RDATA   (0x10004014)
#define AN8801SB_EFUSE_EN_KEY1 (0x10004034)
#define AN8801SB_EFUSE_EN_KEY2 (0x1000408C)
#define AN8801SB_EFUSE_KEY1    (0XDE7502BC)
#define AN8801SB_EFUSE_KEY2    (0x78D39BF1)

#define EFUSE_GPHY_BASE (0x40200052)
#define EFSROM_KICK     (0x40000000)

#define AN8801SB_HWRST_DEGLITCH_REG    (0x100000c8)
#define AN8801SB_HWRST_DEGLITCH_ENABLE (0x7)

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_an8801sb_phy.c");

extern const HAL_PHY_CFG_T _hal_an8801sb_longreach[];
extern const UI32_T        _hal_an8801sb_longreach_size;
extern const HAL_PHY_CFG_T _hal_an8801sb_normal[];
extern const UI32_T        _hal_an8801sb_normal_size;

/* STATIC VARIABLE DECLARATIONS */
static const UI8_T _hal_an8801sb_phy_r50ohm_table[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, /* 10 */
                                                       127, 127, 127, 127, 127, 127, 127, 126, 122, 117, /* 20 */
                                                       112, 109, 104, 101, 97,  94,  90,  88,  84,  80,  /* 30 */
                                                       78,  74,  72,  68,  66,  64,  61,  58,  56,  53,  /* 40 */
                                                       51,  48,  47,  44,  42,  40,  38,  36,  34,  32,  /* 50 */
                                                       31,  28,  27,  24,  24,  22,  20,  18,  16,  16,  /* 60 */
                                                       14,  12,  11,  9};

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void
_an8801sb_phy_set_gphy_TrReg(
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
_an8801sb_phy_get_gphy_TrReg(
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
_hal_an8801sb_phy_getRandomSeed(
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

    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    backup_vga = _an8801sb_phy_get_gphy_TrReg(unit, port, 0xafa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x140);
    value = backup_dev1e_reg03ch | 0x8800;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, value);

    value = backup_dev1f_reg015h & (~0xF1FF);
    value |= 0x108b;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, value);

    value = backup_vga | 0x001f0000;
    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _an8801sb_phy_set_gphy_TrReg(unit, port, (value & 0xffff), (value >> 16) & 0xffff, 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    for (index = 0; index < 32; index++)
    {
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        reg_data &= 0xff;
        seed |= ((reg_data & 0x1) << index);
    }

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1000);

    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _an8801sb_phy_set_gphy_TrReg(unit, port, (backup_vga & 0xffff), ((backup_vga >> 16) & 0xffff), 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

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
_hal_an8801sb_phy_calRandomSeed(
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
_hal_an8801sb_phy_readBuckPbus(
    UI16_T port,
    UI32_T pbus_addr)
{
    UI32_T         pbus_data = 0;
    UI16_T         pbus_data_low = 0, pbus_data_high = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(0, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(0, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(0, port, 0x15, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(0, port, 0x16, (UI32_T)(pbus_addr & 0xffff));

    rv |= hal_mdio_readC22ByPort(0, port, 0x17, &pbus_data_high);
    rv |= hal_mdio_readC22ByPort(0, port, 0x18, &pbus_data_low);
    rv |= hal_mdio_writeC22ByPort(0, port, 0x1F, 0x0);
    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "port %u, read pbus %8x is fail !\n", port, pbus_addr);
    }
    return pbus_data;
}

static void
_hal_an8801sb_phy_writeBuckPbus(
    UI16_T port,
    UI32_T pbus_addr,
    UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(0, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(0, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(0, port, 0x11, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(0, port, 0x12, (UI32_T)(pbus_addr & 0xffff));
    rv |= hal_mdio_writeC22ByPort(0, port, 0x13, (UI32_T)((pbus_data >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(0, port, 0x14, (UI32_T)(pbus_data & 0xffff));
    rv |= hal_mdio_writeC22ByPort(0, port, 0x1F, 0x0);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "port %u, write pbus %8x is fail !\n", port, pbus_addr);
    }
}

static AIR_ERROR_NO_T
_hal_an8801sb_updatePhyLink(
    UI16_T                 port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         status = 0;

    /* Do a fake read */
    rv = hal_mdio_readC22ByPort(0, port, MII_BMSR, &status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    /* Read link and autonegotiation status */
    rv = hal_mdio_readC22ByPort(0, port, MII_BMSR, &status);
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
_hal_an8801sb_readGenphyStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    HAL_PHY_AUTO_NEGO_T auto_nego = HAL_PHY_AUTO_NEGO_LAST;
    UI16_T              adv = 0, lpa = 0, lpagb = 0, bmcr = 0, common_adv_gb = 0, common_adv = 0;
    UI16_T              speed_status = 0;

    rv = _hal_an8801sb_updatePhyLink(port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    hal_an8801sb_phy_getAutoNego(unit, port, &auto_nego);

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
    /* Workaround for the issue of obtaining incorrect speeds after enabling EEE and downshift to 100M
       on RTL ic */
    rv = hal_mdio_readC22ByPort(unit, port, MII_AUX_CTRL_STA, &speed_status);
    if (AIR_E_OK == rv)
    {
        if (ptr_status->speed != BITS_OFF_R(speed_status, 3, 2))
        {
            DIAG_PRINT(HAL_DBG_WARN,
                       "port %d, adjust link speed from %d to %d, auto-nego speed not the same phy speed \n", port,
                       ptr_status->speed, BITS_OFF_R(speed_status, 3, 2));
            ptr_status->speed = BITS_OFF_R(speed_status, 3, 2);
        }
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8801sb_phy_restoreConfigFromEfuse
 * PURPOSE:
 *      Restore AN8801SB PHY config from efuse
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
_hal_an8801sb_phy_restoreConfigFromEfuse(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         data = 0, timeout = MAX_READ_EFUSE_RETRY;
    UI32_T         efuse_data = 0;
    UI32_T         efuse_data_raw[4], raw = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* enable efuse for access */
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, AN8801SB_EFUSE_KEY1);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, AN8801SB_EFUSE_KEY2);

    for (raw = 0; raw < 3; raw++)
    {
        if (raw == 1)
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_ADDR, 2); /* i2mpb hbt */
        }
        else if (raw == 2)
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_ADDR, 19); /* i2mpb tbt */
        }
        else
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_ADDR, raw);
        }
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_CTRL, EFUSE_CMD_READ);

        while (timeout)
        {
            osal_delayUs(1000);
            efuse_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_EFUSE_DONE);
            if (efuse_data != 0)
            {
                efuse_data_raw[raw] = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_EFUSE_RDATA);
                break;
            }
            timeout--;
        }

        if (0 == timeout)
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);
            return AIR_E_TIMEOUT;
        }
    }
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x12, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x17, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x19, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x21, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x16, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x18, &data);

    efuse_data = efuse_data_raw[0];
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
    data |= ((efuse_data >> 8) & BITS(8, 13));
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x19, data);

    /* tx_i2mp_d_gbe */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x21, &data);
    data &= ~(BITS(8, 13));
    data |= ((efuse_data >> 16) & BITS(8, 13));
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x21, data);

    efuse_data = efuse_data_raw[1];
    /* tx_i2mpb_a_hbt */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x16, &data);
    data &= ~(BITS(10, 15));
    data |= BITS_OFF_L(efuse_data, 10, 6);
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x16, data);

    /* tx_i2mpb_b_hbt */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x18, &data);
    data &= ~(BITS(8, 13));
    data |= ((efuse_data >> 2) & BITS(8, 13));
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x18, data);

    efuse_data = efuse_data_raw[2];
    /* tx_i2mpb_a_tbt */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x12, &data);
    data &= ~(BITS(0, 5));
    data |= (efuse_data & BITS(0, 5));
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x12, data);

    /* tx_i2mpb_b_tbt */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x17, &data);
    data &= ~(BITS(0, 5));
    data |= BITS_OFF_R(efuse_data, 6, 6);
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x17, data);

    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);

    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x12, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x17, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x19, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x21, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x16, &data);
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1e, 0x18, &data);
    return rv;
}

/* FUNCTION NAME:   _hal_an8801sb_phy_applyParam
 * PURPOSE:
 *      Apply AN8801SB PHY parameters
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
_hal_an8801sb_phy_applyParam(
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

/* FUNCTION NAME:   _hal_an8801sb_phy_setPhyOpMode
 * PURPOSE:
 *      Set AN8801SB PHY operation mode
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
_hal_an8801sb_phy_setPhyOpMode(
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
        for (count = 0; count < _hal_an8801sb_normal_size; count++)
        {
            if (AIR_E_OK == rv)
            {
                phy_cfg = _hal_an8801sb_normal[count];
                rv = _hal_an8801sb_phy_applyParam(unit, port, phy_cfg);
            }
        }

        if (AIR_E_OK == rv)
        {
            rv = _hal_an8801sb_phy_restoreConfigFromEfuse(unit, port);
        }
    }
    else if (HAL_PHY_OP_MODE_LONG_REACH == mode)
    {
        rv = hal_an8801sb_phy_getLocalAdvAbility(unit, port, &hal_adv);
        if (AIR_E_OK == rv)
        {
            if (hal_adv.flags & HAL_PHY_AN_ADV_FLAGS_EEE)
            {
                rv = AIR_E_OP_INVALID;
            }
            else
            {
                for (count = 0; count < _hal_an8801sb_longreach_size; count++)
                {
                    if (AIR_E_OK == rv)
                    {
                        phy_cfg = _hal_an8801sb_longreach[count];
                        rv = _hal_an8801sb_phy_applyParam(unit, port, phy_cfg);
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

static UI8_T
_hal_an8801sb_phy_shift_check(
    UI8_T base)
{
    UI8_T  index = 0;
    UI32_T sz = sizeof(_hal_an8801sb_phy_r50ohm_table) / sizeof(UI8_T);

    for (index = 0; index < sz; ++index)
    {
        if (_hal_an8801sb_phy_r50ohm_table[index] == base)
        {
            break;
        }
    }

    if (index < 8 || index >= sz)
    {
        return 25; /* index of 94 */
    }

    return index - 8;
}

static UI8_T
_hal_an8801sb_phy_get_shift_val(
    UI8_T idx)
{
    return _hal_an8801sb_phy_r50ohm_table[idx];
}

/* FUNCTION NAME:   _hal_an8801sb_phy_setSurgeProtect
 * PURPOSE:
 *      Set AN8801SB PHY surge protection
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_an8801sb_phy_setSurgeProtect(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T surge_mode;
    UI32_T          efuse_data = 0, shift_sel = 0;
    UI16_T          reg_data = 0, data = 0, timeout = MAX_READ_EFUSE_RETRY;

    osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);

    /* enable efuse for access */
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, AN8801SB_EFUSE_KEY1);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, AN8801SB_EFUSE_KEY2);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_ADDR, 4);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_CTRL, EFUSE_CMD_READ);

    while (timeout)
    {
        osal_delayUs(1000);
        efuse_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_EFUSE_DONE);
        if (efuse_data != 0)
        {
            efuse_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_EFUSE_RDATA);
            break;
        }
        timeout--;
    }

    if (0 == timeout)
    {
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);
        return AIR_E_TIMEOUT;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8801sb surge protection mode - 0R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            data = BITS_OFF_R(efuse_data, 0, 7);
            reg_data |= (data << 8);
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            data = BITS_OFF_R(efuse_data, 8, 7);
            reg_data |= data;
        }
        else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8801sb surge protection mode - 5R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            data = BITS_OFF_R(efuse_data, 0, 7);
            shift_sel = _hal_an8801sb_phy_shift_check(data);
            data = _hal_an8801sb_phy_get_shift_val(shift_sel);
            reg_data |= (data << 8);
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            data = BITS_OFF_R(efuse_data, 8, 7);
            shift_sel = _hal_an8801sb_phy_shift_check(data);
            data = _hal_an8801sb_phy_get_shift_val(shift_sel);
            reg_data |= data;
        }
        else
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);
            DIAG_PRINT(HAL_DBG_ERR, "port %u, an8801sb surge protection mode %u is invalid!\n", port, surge_mode.value);
            return AIR_E_BAD_PARAMETER;
        }
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, reg_data);
    }

    rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
        {
            /* cr_r50ohm_rsel_tx_c */
            reg_data &= ~(BITS_RANGE(8, 7));
            data = BITS_OFF_R(efuse_data, 16, 7);
            reg_data |= (data << 8);
            /* cr_r50ohm_rsel_tx_d */
            reg_data &= ~(BITS_RANGE(0, 7));
            data = BITS_OFF_R(efuse_data, 24, 7);
            reg_data |= data;
        }
        else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
        {
            /* cr_r50ohm_rsel_tx_c */
            reg_data &= ~(BITS_RANGE(8, 7));
            data = BITS_OFF_R(efuse_data, 16, 7);
            shift_sel = _hal_an8801sb_phy_shift_check(data);
            data = _hal_an8801sb_phy_get_shift_val(shift_sel);
            reg_data |= (data << 8);
            /* cr_r50ohm_rsel_tx_d */
            reg_data &= ~(BITS_RANGE(0, 7));
            data = BITS_OFF_R(efuse_data, 24, 7);
            shift_sel = _hal_an8801sb_phy_shift_check(data);
            data = _hal_an8801sb_phy_get_shift_val(shift_sel);
            reg_data |= data;
        }
        else
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);
            DIAG_PRINT(HAL_DBG_ERR, "port %u, an8801sb surge protection mode %u is invalid!\n", port, surge_mode.value);
            return AIR_E_BAD_PARAMETER;
        }
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, reg_data);
    }
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY1, 0x0);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFUSE_EN_KEY2, 0x0);
    return rv;
}

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME: hal_an8801sb_phy_init
 * PURPOSE:
 *      AN8801SB PHY initialization
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
hal_an8801sb_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    UI32_T                  pbus_data;
    AIR_CFG_VALUE_T         led_behavior;
    UI16_T                  led_id = 0, led_config = 0, led_count = 0;
    UI16_T                  link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    UI16_T                  reg_value = 0;
    UI32_T                  random_seed = 0, seed = 0;
    HAL_PHY_LED_CTRL_MODE_T led_ctrl_mode = 0;

    /* disable LPM */
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x600, 0x1e);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x601, 0x02);

    /* IOMUX for LAN LED */
    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_RG_FORCE_GPIO0_EN);
    pbus_data &= ~(0x3);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_RG_FORCE_GPIO0_EN, pbus_data);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_RG_GPIO_LAN0_LED0_MODE, 0x3);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_RG_GPIO_LAN0_LED0_SEL, 0x08);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_RG_GPIO_LED_INV, 0x01);
    /* LED 0 & 1 driving for 4 MA */
    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_RG_GPIO_DRIVING_E2);
    pbus_data &= ~(0x3);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_RG_GPIO_DRIVING_E2, pbus_data);

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = AN8801SB_LED_COUNT;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    /* LED blink frequence */
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21, 0x8008);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22, 0x600);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x23, 0xc00);

    /* LED configuration */
    for (led_id = 0; led_id < led_count; led_id++)
    {
        if (led_id == 0)
        {
            link_reg_addr = AN8801SB_LED_0_ON_MASK;
            blk_reg_addr = AN8801SB_LED_0_BLK_MASK;
        }
        else if (led_id == 1)
        {
            link_reg_addr = AN8801SB_LED_1_ON_MASK;
            blk_reg_addr = AN8801SB_LED_1_BLK_MASK;
        }
        else if (led_id == 2)
        {
            link_reg_addr = AN8801SB_LED_2_ON_MASK;
            blk_reg_addr = AN8801SB_LED_2_BLK_MASK;
        }
        else
        {
            link_reg_addr = AN8801SB_LED_3_ON_MASK;
            blk_reg_addr = AN8801SB_LED_3_BLK_MASK;
        }

        hal_an8801sb_phy_getPhyLedCtrlMode(unit, port, led_id, &led_ctrl_mode);
        if (HAL_PHY_LED_CTRL_MODE_PHY == led_ctrl_mode)
        {
            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

            led_behavior.value = 0xFFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = led_id;

            hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
            led_config = led_behavior.value;

            link_reg_data = 0;
            blk_reg_data = 0;

            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_1000, AN8801SB_LED_LINK_1000, link_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_100, AN8801SB_LED_LINK_100, link_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_10, AN8801SB_LED_LINK_10, link_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_FULLDPLX, AN8801SB_LED_LINK_FULLDPLX, link_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_HALFDPLX, AN8801SB_LED_LINK_HALFDPLX, link_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_HIGH_ACTIVE, AN8801SB_LED_POL_HIGH_ACT, link_reg_data);

            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_1000, AN8801SB_LED_BLINK_TX_1000, blk_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_1000, AN8801SB_LED_BLINK_RX_1000, blk_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_100, AN8801SB_LED_BLINK_TX_100, blk_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_100, AN8801SB_LED_BLINK_RX_100, blk_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_10, AN8801SB_LED_BLINK_TX_10, blk_reg_data);
            GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_10, AN8801SB_LED_BLINK_RX_10, blk_reg_data);

            if (link_reg_data != 0 || blk_reg_data != 0)
            {
                link_reg_data |= AN8801SB_LED_FUNC_ENABLE;
            }

            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
        }
    }
    rv = _hal_an8801sb_phy_setPhyOpMode(unit, port, HAL_PHY_OP_MODE_NORMAL);
    rv |= _hal_an8801sb_phy_getRandomSeed(unit, port, &random_seed);
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
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8801sb_phy_setSurgeProtect(unit, port);
        if (AIR_E_OK == rv)
        {
            /* Enable HW reset de-glitch */
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_HWRST_DEGLITCH_REG, AN8801SB_HWRST_DEGLITCH_ENABLE);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_setAdminState
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
hal_an8801sb_phy_setAdminState(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getAdminState
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
hal_an8801sb_phy_getAdminState(
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

/* FUNCTION NAME:   hal_an8801sb_phy_setAutoNego
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
hal_an8801sb_phy_setAutoNego(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getAutoNego
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
hal_an8801sb_phy_getAutoNego(
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

/* FUNCTION NAME:   hal_an8801sb_phy_setLocalAdvAbility
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
hal_an8801sb_phy_setLocalAdvAbility(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getLocalAdvAbility
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
hal_an8801sb_phy_getLocalAdvAbility(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getRemoteAdvAbility
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
hal_an8801sb_phy_getRemoteAdvAbility(
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

/* FUNCTION NAME:   hal_an8801sb_phy_setSpeed
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
hal_an8801sb_phy_setSpeed(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getSpeed
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
hal_an8801sb_phy_getSpeed(
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

/* FUNCTION NAME:   hal_an8801sb_phy_setDuplex
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
hal_an8801sb_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    HAL_CHECK_ENUM_RANGE(duplex, HAL_PHY_DUPLEX_LAST);

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_DUPLEX_FULL == duplex)
        {
            reg_value |= BMCR_FULLDPLX;
        }
        else
        {
            reg_value &= ~(BMCR_FULLDPLX);
        }
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getDuplex
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
hal_an8801sb_phy_getDuplex(
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

/* FUNCTION NAME:   hal_an8801sb_phy_getLinkStatus
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
hal_an8801sb_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    UI16_T            preSpeed = 0, reg_bmcr = 0, reg_data = 0;
    UI32_T            reg_value = 0, force_speed = 0;
    I32_T             an_retry = MAX_SGMII_AN_RETRY;
    HAL_PHY_OP_MODE_T phy_mode = HAL_PHY_OP_MODE_LAST;

    rv = _hal_an8801sb_readGenphyStatus(unit, port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] flags=(%x)\n", unit, port, ptr_status->flags);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_PRE_SPEED_REG, &preSpeed);
    rv |= hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_bmcr);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    if (0 == (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP))
    {
        if (NO_SPEED != preSpeed)
        {
            reg_value = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PCS_CTRL1);
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_PCS_CTRL1,
                                            ((reg_value & ~SGMII_PCS_FORCE_SYNC_MASK) | SGMII_PCS_FORCE_SYNC_OFF));
            preSpeed = NO_SPEED;
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_PRE_SPEED_REG, preSpeed);
        }
        rv |= _hal_an8801sb_phy_calRandomSeed(unit, port, FALSE);
        return rv;
    }

    force_speed = 0;
    if (0 == (reg_bmcr & BMCR_ANENABLE))
    {
        /* PHY is force mode so force serdes speed */
        if (reg_bmcr & BMCR_SPEED1000)
        {
            force_speed = 0xd801;
            ptr_status->speed = HAL_PHY_SPEED_1000M;
        }
        else if (reg_bmcr & BMCR_SPEED100)
        {
            force_speed = 0xd401;
            ptr_status->speed = HAL_PHY_SPEED_100M;
        }
        else
        {
            force_speed = 0xd001;
            ptr_status->speed = HAL_PHY_SPEED_10M;
        }
    }

    if (preSpeed != ptr_status->speed)
    {
        preSpeed = ptr_status->speed;
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_PRE_SPEED_REG, preSpeed);

        reg_value = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PCS_CTRL1);
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_PCS_CTRL1, (reg_value & ~SGMII_PCS_FORCE_SYNC_MASK));
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_EFIFO_WM, EFIFO_WM_VALUE);
        while (an_retry > 0)
        {
            osal_delayUs(1000); /* delay 1 ms */
            reg_value = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PCS_STS);
            if (reg_value & AN8801SB_SGMII_AN0_AN_DONE)
            {
                break;
            }
            an_retry--;
        }
        osal_delayUs(10000); /* delay 10 ms */
        if (force_speed != 0)
        {
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_SGMII_AN4, force_speed);

            reg_value = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_SGMII_AN0);
            reg_value |= AN8801SB_SGMII_AN0_ANRESTART;
            _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_SGMII_AN0, reg_value);
        }
        reg_value = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_SGMII_AN0);
        reg_value |= AN8801SB_SGMII_AN0_RESET;
        _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_SGMII_AN0, reg_value);
    }

    if (AIR_E_OK == rv)
    {
        /* Fix 0x1e 0x148 value to 0x200 when link speed is 10M at long-reach mode*/
        rv = hal_an8801sb_phy_getPhyOpMode(unit, port, &phy_mode);
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

/* FUNCTION NAME:   hal_an8801sb_phy_setLoopBack
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
hal_an8801sb_phy_setLoopBack(
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
        if ((rv = hal_mdio_readC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
        if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
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

/* FUNCTION NAME:   hal_an8801sb_phy_getLoopBack
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
hal_an8801sb_phy_getLoopBack(
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
        if ((rv = hal_mdio_readC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
        {
            return rv;
        }

        /* Swtich to page 1 */
        reg_data = 1;
        if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
        if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
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

/* FUNCTION NAME:   hal_an8801sb_phy_setSmartSpeedDown
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
hal_an8801sb_phy_setSmartSpeedDown(
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
    if ((rv = hal_mdio_readC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
    if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
    {
        return rv;
    }

    if ((rv = hal_mdio_writeC22ByPort(unit, port, 0x14, data)) != AIR_E_OK)
    {
        return rv;
    }

    /* Restore page */
    if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getSmartSpeedDown
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
hal_an8801sb_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Backup page */
    if ((rv = hal_mdio_readC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, &page)) != AIR_E_OK)
    {
        return rv;
    }

    /* Switch to page 1*/
    reg_data = 1;
    if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, reg_data)) != AIR_E_OK)
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
    if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
    {
        return rv;
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_setLedOnCtrl
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
hal_an8801sb_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = AN8801SB_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = AN8801SB_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = AN8801SB_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = AN8801SB_LED_3_ON_MASK;
    }

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
                reg_data |= AN8801SB_LED_LINK_FORCE_ON;
            }
            else
            {
                reg_data &= ~(AN8801SB_LED_LINK_FORCE_ON);
            }

            if (reg_data != 0)
            {
                reg_data |= AN8801SB_LED_FUNC_ENABLE;
            }

            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getLedOnCtrl
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
hal_an8801sb_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = AN8801SB_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = AN8801SB_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = AN8801SB_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = AN8801SB_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & AN8801SB_LED_LINK_FORCE_ON)
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

/* FUNCTION NAME:   hal_an8801sb_phy_testTxCompliance
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
hal_an8801sb_phy_testTxCompliance(
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

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedCtrlMode
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
hal_an8801sb_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    UI16_T              led_config = 0, offset = 0;
    UI16_T              link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    UI32_T              pbus_data = 0;
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    BOOL_T              is_force = FALSE;
    HAL_PHY_LED_STATE_T state;
    AIR_CFG_VALUE_T     led_behavior;

    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PHY_LED_CFG_REG);

    offset = (AN8801SB_LED_0_CFG_CTRL_MODE_BIT + (AN8801SB_LED_CFG_RG_OFFSET * led_id));
    pbus_data &= ~(BITS_OFF_L(AN8801SB_LED_CFG_CTRL_MODE_MASK, offset, AN8801SB_LED_CFG_CTRL_MODE_WIDTH));
    pbus_data |= BITS_OFF_L(ctrl_mode, offset, AN8801SB_LED_CFG_CTRL_MODE_WIDTH);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_PHY_LED_CFG_REG, pbus_data);

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
            rv = hal_an8801sb_phy_getPhyLedForceState(unit, port, led_id, &state);
            if (AIR_E_OK == rv)
            {
                rv = hal_an8801sb_phy_setPhyLedForceState(unit, port, led_id, state);
            }
        }
        else
        {
            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
            led_behavior.value = DEFAULT_LED_CFG;
            led_behavior.param0 = port;
            led_behavior.param1 = led_id;

            rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

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

                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_1000, AN8801SB_LED_LINK_1000, link_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_100, AN8801SB_LED_LINK_100, link_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_10, AN8801SB_LED_LINK_10, link_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_FULLDPLX, AN8801SB_LED_LINK_FULLDPLX,
                                    link_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_LINK_HALFDPLX, AN8801SB_LED_LINK_HALFDPLX,
                                    link_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_HIGH_ACTIVE, AN8801SB_LED_POL_HIGH_ACT, link_reg_data);

                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_1000, AN8801SB_LED_BLINK_TX_1000,
                                    blk_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_1000, AN8801SB_LED_BLINK_RX_1000,
                                    blk_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_100, AN8801SB_LED_BLINK_TX_100, blk_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_100, AN8801SB_LED_BLINK_RX_100, blk_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_TX_10, AN8801SB_LED_BLINK_TX_10, blk_reg_data);
                    GET_PHY_ABILITY(led_config, AN8801SB_LED_BHV_BLINK_RX_10, AN8801SB_LED_BLINK_RX_10, blk_reg_data);

                    if (link_reg_data != 0 || blk_reg_data != 0)
                    {
                        link_reg_data |= AN8801SB_LED_FUNC_ENABLE;
                    }

                    DIAG_PRINT(HAL_DBG_INFO, "port %u, link_reg_addr 0x%02X, link_reg_data 0x%04X\n", port,
                               link_reg_addr, link_reg_data);
                    DIAG_PRINT(HAL_DBG_INFO, "port %u, blk_reg_addr 0x%02X, blk_reg_data 0x%04X\n", port, blk_reg_addr,
                               blk_reg_data);

                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8801SB_LED_ON_DURATION,
                                                         (AN8801SB_LED_BLINK_RATE_DEFAULT / 2));
                            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8801SB_LED_BLINK_DURATION,
                                                          AN8801SB_LED_BLINK_RATE_DEFAULT);
                        }
                    }
                }
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedCtrlMode
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
hal_an8801sb_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    UI16_T         offset = 0;
    UI32_T         pbus_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PHY_LED_CFG_REG);

    offset = (AN8801SB_LED_0_CFG_CTRL_MODE_BIT + (AN8801SB_LED_CFG_RG_OFFSET * led_id));
    *ptr_ctrl_mode = BITS_OFF_R(pbus_data, offset, AN8801SB_LED_CFG_CTRL_MODE_WIDTH);

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedForceState
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
hal_an8801sb_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    UI16_T                  reg_data = 0, reg_addr = 0, offset = 0;
    UI32_T                  pbus_data = 0;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    BOOL_T                  is_force = FALSE;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode;
    HAL_PHY_LED_PATT_T      led_patt;

    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PHY_LED_CFG_REG);

    offset = (AN8801SB_LED_0_CFG_STATE_BIT + (AN8801SB_LED_CFG_RG_OFFSET * led_id));
    pbus_data &= ~(BITS_OFF_L(AN8801SB_LED_CFG_STATE_MASK, offset, AN8801SB_LED_CFG_STATE_WIDTH));
    pbus_data |= BITS_OFF_L(state, offset, AN8801SB_LED_CFG_STATE_WIDTH);
    _hal_an8801sb_phy_writeBuckPbus(port, AN8801SB_PHY_LED_CFG_REG, pbus_data);

    rv = hal_an8801sb_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
        {
            is_force = (HAL_PHY_LED_STATE_FORCE_PATT == state) ? TRUE : FALSE;
            if (TRUE == is_force)
            {
                rv = hal_an8801sb_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                if (AIR_E_OK == rv)
                {
                    rv = hal_an8801sb_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                }
            }
            else
            {
                (HAL_PHY_LED_STATE_ON == state ? (reg_data = AN8801SB_LED_LINK_FORCE_ON) : (reg_data = 0));
                LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr,
                                             (reg_data | AN8801SB_LED_FUNC_ENABLE));
                if (AIR_E_OK == rv)
                {
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, 0);
                }
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedForceState
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
hal_an8801sb_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    UI16_T         offset = 0;
    UI32_T         pbus_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    pbus_data = _hal_an8801sb_phy_readBuckPbus(port, AN8801SB_PHY_LED_CFG_REG);

    offset = (AN8801SB_LED_0_CFG_STATE_BIT + (AN8801SB_LED_CFG_RG_OFFSET * led_id));
    *ptr_state = BITS_OFF_R(pbus_data, offset, AN8801SB_LED_CFG_STATE_WIDTH);

    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_setPhyLedForcePattCfg
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
hal_an8801sb_phy_setPhyLedForcePattCfg(
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
            reg_data = AN8801SB_LED_BLINK_RATE_HZ_HALF;
            break;
        case HAL_PHY_LED_PATT_HZ_ONE:
            reg_data = AN8801SB_LED_BLINK_RATE_HZ_ONE;
            break;
        case HAL_PHY_LED_PATT_HZ_TWO:
            reg_data = AN8801SB_LED_BLINK_RATE_HZ_TWO;
            break;
        default:
            rv = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK == rv)
    {
        data = BITS_OFF_L(pattern, AN8801SB_LED_CFG_PATT_OFFSET, AN8801SB_LED_CFG_PATT_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_BLK_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_an8801sb_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
        {
            rv = hal_an8801sb_phy_getPhyLedForceState(unit, port, led_id, &led_state);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8801SB_LED_ON_DURATION, (reg_data / 2));
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8801SB_LED_BLINK_DURATION, reg_data);
                if (AIR_E_OK == rv)
                {
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, AN8801SB_LED_BLINK_FORCE);
                    if (AIR_E_OK == rv)
                    {
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                        reg_data = AN8801SB_LED_FUNC_ENABLE;
                        led_behavior.value = 0xFFFF;
                        led_behavior.param0 = port;
                        led_behavior.param1 = led_id;

                        hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
                        if (led_behavior.value & AN8801SB_LED_BHV_HIGH_ACTIVE)
                        {
                            reg_data |= AN8801SB_LED_POL_HIGH_ACT;
                        }
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8801sb_phy_getPhyLedForcePattCfg
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
hal_an8801sb_phy_getPhyLedForcePattCfg(
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
        *ptr_pattern = BITS_OFF_R(data, AN8801SB_LED_CFG_PATT_OFFSET, AN8801SB_LED_CFG_PATT_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME: hal_an8801sb_phy_triggerCableTest
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
hal_an8801sb_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerCableTest(unit, port, test_pair, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_getCableTestRawData
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
hal_an8801sb_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedGlbCfg
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
hal_an8801sb_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedGlbCfg
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
hal_an8801sb_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedBlkEvent
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
hal_an8801sb_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedBlkEvent
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
hal_an8801sb_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_setPhyLedDuration
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
hal_an8801sb_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_getPhyLedDuration
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
hal_an8801sb_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_setPhyOpMode
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
hal_an8801sb_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    rv = _hal_an8801sb_phy_setPhyOpMode(unit, port, mode);
    return rv;
}

/* FUNCTION NAME: hal_an8801sb_phy_getPhyOpMode
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
hal_an8801sb_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, &reg_data);
    if (AIR_E_OK == rv)
    {
        switch (reg_data)
        {
            case AN8801SB_LONG_REACH:
                *ptr_mode = HAL_PHY_OP_MODE_LONG_REACH;
                break;
            case AN8801SB_1G_LOW_POWER:
            case AN8801SB_10M_100M_LOW_POWER:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
            default:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
        }
    }
    return rv;
}

/* FUNCTION NAME: hal_an8801sb_phy_triggerLinkDownCableTest
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
hal_an8801sb_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerLinkDownCableTest(unit, port, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_an8801sb_phy_dumpPhyPara
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
hal_an8801sb_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    UI16_T            reg_data = 0;
    HAL_PHY_OP_MODE_T phy_mode;
    AIR_CFG_VALUE_T   surge_mode;

    osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);

    osal_printf("\n=== phy operation mode ===\n");
    rv = hal_an8801sb_phy_getPhyOpMode(unit, port, &phy_mode);
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

    osal_printf("\n=== phy surge mode ===\n");
    if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
    {
        osal_printf("port=%2u    surge mode                = 0R\n", port);
    }
    else
    {
        osal_printf("port=%2u    surge mode                = 5R\n", port);
    }

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
    rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_ExtReg);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, 0x12, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x12, reg_data);
        }
        rv = hal_mdio_readC22ByPort(unit, port, 0x14, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x14, reg_data);
        }
        rv = hal_mdio_readC22ByPort(unit, port, 0x17, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x17, reg_data);
        }
        rv = hal_mdio_readC22ByPort(unit, port, 0x1a, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x1a, reg_data);
        }
    }
    rv = hal_mdio_writeC22ByPort(unit, port, AN8801SB_PHY_MII_PAGE_SELECT, CL22_Page_Reg);
    if (AIR_E_OK == rv)
    {
        osal_printf("\n=== cl45 ===\n");
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, 0x3c, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_ANEG, 0x3c, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, 0x3d, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_ANEG, 0x3d, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, 0x3, 0x1, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port, 0x3,
                        0x1, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x39, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x39, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x3c, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3d, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x3d, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x3e, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xc6, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xc6, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x12, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x12, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x13, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x13, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x14, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x14, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x15, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x15, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x16, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x16, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x17, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x17, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x18, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x18, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x19, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x19, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x20, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x20, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x21, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x21, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x22, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x22, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x171, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x171, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x174, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x175, reg_data);
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
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xa2, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xa2, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xe6, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xe6, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xe7, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xe7, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xfe, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xfe, reg_data);
        }
        osal_printf("\n=== TX shapper ===\n");
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x0, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x1, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x1, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x2, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x2, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x3, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x4, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x4, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x5, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x5, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x6, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x6, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x7, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x7, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x8, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x8, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x9, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x9, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xa, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xa, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0xb, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0xb, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x11, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC1, 0x11, reg_data);
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
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20a, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20a, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20b, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20b, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20c, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20c, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20d, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20d, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20e, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20e, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x20f, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x20f, reg_data);
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
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21a, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21a, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21b, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21b, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21c, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21c, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21d, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21d, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21e, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21e, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21f, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x21f, reg_data);
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
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22a, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22a, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22b, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22b, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22c, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22c, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22d, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22d, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22e, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22e, reg_data);
        }
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22f, &reg_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                        MMD_DEV_VSPEC2, 0x22f, reg_data);
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
    }
    return rv;
}

const HAL_PHY_DRIVER_T _ext_an8801sb_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_an8801sb_phy_init,
    hal_an8801sb_phy_setAdminState,
    hal_an8801sb_phy_getAdminState,
    hal_an8801sb_phy_setAutoNego,
    hal_an8801sb_phy_getAutoNego,
    hal_an8801sb_phy_setLocalAdvAbility,
    hal_an8801sb_phy_getLocalAdvAbility,
    hal_an8801sb_phy_getRemoteAdvAbility,
    hal_an8801sb_phy_setSpeed,
    hal_an8801sb_phy_getSpeed,
    hal_an8801sb_phy_setDuplex,
    hal_an8801sb_phy_getDuplex,
    hal_an8801sb_phy_getLinkStatus,
    hal_an8801sb_phy_setLoopBack,
    hal_an8801sb_phy_getLoopBack,
    hal_an8801sb_phy_setSmartSpeedDown,
    hal_an8801sb_phy_getSmartSpeedDown,
    hal_an8801sb_phy_setLedOnCtrl,
    hal_an8801sb_phy_getLedOnCtrl,
    hal_an8801sb_phy_testTxCompliance,
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    NULL, /* setSerdeMode */
    NULL, /* getSerdeMode */
    hal_an8801sb_phy_setPhyLedCtrlMode,
    hal_an8801sb_phy_getPhyLedCtrlMode,
    hal_an8801sb_phy_setPhyLedForceState,
    hal_an8801sb_phy_getPhyLedForceState,
    hal_an8801sb_phy_setPhyLedForcePattCfg,
    hal_an8801sb_phy_getPhyLedForcePattCfg,
    hal_an8801sb_phy_triggerCableTest,
    hal_an8801sb_phy_getCableTestRawData,
    hal_an8801sb_phy_setPhyLedGlbCfg,
    hal_an8801sb_phy_getPhyLedGlbCfg,
    hal_an8801sb_phy_setPhyLedBlkEvent,
    hal_an8801sb_phy_getPhyLedBlkEvent,
    hal_an8801sb_phy_setPhyLedDuration,
    hal_an8801sb_phy_getPhyLedDuration,
    hal_an8801sb_phy_setPhyOpMode,
    hal_an8801sb_phy_getPhyOpMode,
    hal_an8801sb_phy_dumpPhyPara,
    hal_an8801sb_phy_triggerLinkDownCableTest,
    NULL, /* dumpPortCnt */
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_an8801sb_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_an8801sb_phy_func_vec;

    return (AIR_E_OK);
}
