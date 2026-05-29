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

/* FILE NAME:  hal_an8804_phy.c
 * PURPOSE:
 *  Implement an8804 phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/an8804/hal_an8804_phy.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#include <hal/switch/pearl/hal_pearl_perif.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define PHY_LED_BLK_CFG_REG        (HAL_AN8804_PHY_LED_3_ON_MASK)
#define HAL_AN8804_PHY_LED_CFG_REG (HAL_AN8804_PHY_LED_3_BLK_MASK)

#define PHY_LED_TYPE_ON  (0)
#define PHY_LED_TYPE_BLK (1)

#define TKRG_DATA_ADDR_OFFSET             (1)
#define TKRG_NODE_ADDR_OFFSET             (7)
#define TKRG_CHANNEL_ADDR_OFFSET          (11)
#define TKRG_WR_RD_CTRL_OFFSET            (13)
#define TKRG_READ_CTRL                    (1 << TKRG_WR_RD_CTRL_OFFSET)
#define TKRG_WRITE_CTRL                   (0 << TKRG_WR_RD_CTRL_OFFSET)
#define TKRG_PKT_XMT_STA                  BIT(15)
#define HAL_PHY_AN8804_WORD_SIZE          (4)
#define HAL_PHY_AN8804_EFUSE_PHY_BASE     (3)
#define HAL_PHY_AN8804_EFUSE_SIZE_PER_PHY (4)

#define HAL_PHY_AN8804_LONG_REACH         (0xc00)
#define HAL_PHY_AN8804_NORMAL             (0x800)
#define HAL_PHY_AN8804_UNPLUG_LOW_POWER   (0x50aa)
#define HAL_PHY_AN8804_100M_10M_LOW_POWER (0x53aa)
#define HAL_PHY_AN8804_1G_LOW_POWER       (0x5faa)

#define HAL_PHY_AN8804_EFUSE_DATA5 (0x1000A514)
#define HAL_PHY_AN8804_EFUSE_DATA6 (0x1000A518)

#define HAL_PHY_AN8804_EFUSE_DATA0 (0x1000A500)

#define HAL_AN8804_PHY_FLASH_SET_HALF_HZ   (0x00)
#define HAL_AN8804_PHY_FLASH_SET_ONE_HZ    (0x01)
#define HAL_AN8804_PHY_FLASH_SET_TWO_HZ    (0x02)
#define HAL_AN8804_PHY_FLASH_SET_FORCE_OFF (0x04)
#define HAL_AN8804_PHY_FLASH_SET_FORCE_ON  (0x05)
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

#define WRITE_CL22_BY_PORT(__unit__, __port__, __page__, __reg__, __data__)                          \
    do                                                                                               \
    {                                                                                                \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                                              \
        {                                                                                            \
            rv = hal_mdio_writeC22FromIntBusByPort(__unit__, __port__, __page__, __reg__, __data__); \
        }                                                                                            \
        else                                                                                         \
        {                                                                                            \
            rv = hal_mdio_writeC22ByPort(__unit__, __port__, __reg__, __data__);                     \
        }                                                                                            \
    } while (0)
#define READ_CL22_BY_PORT(__unit__, __port__, __page__, __reg__, __data__)                          \
    do                                                                                              \
    {                                                                                               \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                                             \
        {                                                                                           \
            rv = hal_mdio_readC22FromIntBusByPort(__unit__, __port__, __page__, __reg__, __data__); \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            rv = hal_mdio_readC22ByPort(__unit__, __port__, __reg__, __data__);                     \
        }                                                                                           \
    } while (0)
#define WRITE_CL45_BY_PORT(__unit__, __port__, __dev__, __reg__, __data__)                          \
    do                                                                                              \
    {                                                                                               \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                                             \
        {                                                                                           \
            rv = hal_mdio_writeC45FromIntBusByPort(__unit__, __port__, __dev__, __reg__, __data__); \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            rv = hal_mdio_writeC45ByPort(__unit__, __port__, __dev__, __reg__, __data__);           \
        }                                                                                           \
    } while (0)
#define READ_CL45_BY_PORT(__unit__, __port__, __dev__, __reg__, __data__)                          \
    do                                                                                             \
    {                                                                                              \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                                            \
        {                                                                                          \
            rv = hal_mdio_readC45FromIntBusByPort(__unit__, __port__, __dev__, __reg__, __data__); \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            rv = hal_mdio_readC45ByPort(__unit__, __port__, __dev__, __reg__, __data__);           \
        }                                                                                          \
    } while (0)
#define READ_REG_BY_PORT(__unit__, __port__, __reg__, __data__, __length__) \
    do                                                                      \
    {                                                                       \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                     \
        {                                                                   \
            rv = aml_readReg(__unit__, __reg__, __data__, __length__);      \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* TODO AN8804 read REG */                                      \
            return AIR_E_NOT_SUPPORT;                                       \
        }                                                                   \
    } while (0)
#define WRITE_REG_BY_PORT(__unit__, __port__, __reg__, __data__, __length__) \
    do                                                                       \
    {                                                                        \
        if (HAL_IS_GPHY_PORT_VALID(__unit__, __port__))                      \
        {                                                                    \
            rv = aml_writeReg(__unit__, __reg__, __data__, __length__);      \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            /* TODO AN8804 read REG */                                       \
            return AIR_E_NOT_SUPPORT;                                        \
        }                                                                    \
    } while (0)

#define HAL_AN8804_PHY_GET_PSR_LINK_STS(data, phy_port)                                           \
    ((data & (HAL_AN8804_PHY_PSR_LINK_STS_MASK                                                    \
              << ((phy_port % HAL_AN8804_PHY_PER_PSR_PORTS) * HAL_AN8804_PHY_PER_PSR_OFFSET))) >> \
     ((phy_port % HAL_AN8804_PHY_PER_PSR_PORTS) * HAL_AN8804_PHY_PER_PSR_OFFSET))

#define HAL_AN8804_PHY_LED_SRC_SEL_POS(__port__, __ledId__)   (1 << ((__port__ * 3) + (__ledId__)))
#define HAL_AN8804_PHY_RG_LED_FLASH_SET_CTRL(__port__)        (HAL_AN8804_PHY_LED_FLASH_SET_CTRL0_REG + ((__port__ / 2) * 4))
#define HAL_AN8804_PHY_LED_FLASH_SET_POS(__port__, __ledId__) (((__port__ & 0x01) * 16) + (__ledId__ * 4))
#define HAL_AN8804_PHY_LED_FLASH_SET_MASK                     (0x7)
#define HAL_AN8804_PHY_GPIO_FLASH_MAP_CFG_POS(__gpio__)       ((__gpio__) * 4)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_an8804_phy.c");

extern const HAL_PHY_CFG_T _hal_an8804_longreach[];
extern const UI32_T        _hal_an8804_longreach_size;
extern const HAL_PHY_CFG_T _hal_an8804_normal[];
extern const UI32_T        _hal_an8804_normal_size;

/* STATIC VARIABLE DECLARATIONS */
static const UI8_T         _hal_an8804_phy_r50ohm_table[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, /* 10 */
                                                             127, 127, 127, 127, 127, 127, 127, 126, 122, 117, /* 20 */
                                                             112, 109, 104, 101, 97,  94,  90,  88,  84,  80,  /* 30 */
                                                             78,  74,  72,  68,  66,  64,  61,  58,  56,  53,  /* 40 */
                                                             51,  48,  47,  44,  42,  40,  38,  36,  34,  32,  /* 50 */
                                                             31,  28,  27,  24,  24,  22,  20,  18,  16,  16,  /* 60 */
                                                             14,  12,  11,  9};

static const UI8_T         _hal_an8804_phy_pled_gpio[5] = {7, 5, 4, 3, 2};

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static UI8_T
_hal_an8804_phy_shift_check(
    UI8_T base)
{
    UI8_T  index = 0;
    UI32_T sz = sizeof(_hal_an8804_phy_r50ohm_table) / sizeof(UI8_T);

    for (index = 0; index < sz; ++index)
    {
        if (_hal_an8804_phy_r50ohm_table[index] == base)
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
_hal_an8804_phy_get_shift_val(
    UI8_T idx)
{
    return _hal_an8804_phy_r50ohm_table[idx];
}

static void
_hal_an8804_phy_tkrg_wait_ready(
    UI8_T phyAddr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI8_T          retry = 0x7F;
    UI16_T         reg_data = 0;

    do
    {
        rv = hal_mdio_readC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x10, &reg_data);
        if (AIR_E_OK == rv)
        {
            retry--;
        }
    } while (((reg_data & TKRG_PKT_XMT_STA) == 0) && (retry > 0));
}

static void
_hal_an8804_phy_tkrg_write(
    UI8_T  phyAddr,
    UI8_T  chan_addr,
    UI8_T  node_addr,
    UI8_T  data_addr,
    UI32_T wdata)
{
    UI16_T low_data = wdata & 0xffff;
    UI16_T high_data = (wdata >> 16) & 0xffff;
    UI16_T reg_data = 0;

    reg_data = (data_addr << TKRG_DATA_ADDR_OFFSET) | (node_addr << TKRG_NODE_ADDR_OFFSET) |
               (chan_addr << TKRG_CHANNEL_ADDR_OFFSET) | TKRG_WRITE_CTRL | TKRG_PKT_XMT_STA;

    hal_mdio_writeC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x11, low_data);
    hal_mdio_writeC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x12, high_data);
    hal_mdio_writeC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x10, reg_data);
}

static UI32_T
_hal_an8804_phy_tkrg_read(
    UI8_T phyAddr,
    UI8_T channel_addr,
    UI8_T node_addr,
    UI8_T data_addr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;
    UI32_T         rdata = 0;

    reg_data = (data_addr << TKRG_DATA_ADDR_OFFSET) | (node_addr << TKRG_NODE_ADDR_OFFSET) |
               (channel_addr << TKRG_CHANNEL_ADDR_OFFSET) | TKRG_READ_CTRL | TKRG_PKT_XMT_STA;

    hal_mdio_writeC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x10, reg_data);
    _hal_an8804_phy_tkrg_wait_ready(phyAddr);

    rv = hal_mdio_readC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x11, &reg_data);
    if (AIR_E_OK == rv)
    {
        rdata = reg_data;
        rv = hal_mdio_readC22FromIntBus(0, 0, phyAddr, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x12, &reg_data);
        if (AIR_E_OK == rv)
        {
            rdata |= (reg_data << 16);
        }
    }
    return rdata;
}

AIR_ERROR_NO_T
_hal_an8804_phy_getRandomSeed(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *random_seed)
{
    UI8_T          index = 0;
    UI16_T         backup_dev1e_reg03ch = 0, backup_dev1f_reg015h = 0, backup_reg0 = 0, reg_data = 0;
    UI32_T         backup_vga = 0, value, seed = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &backup_reg0);
    backup_reg0 &= ~(BMCR_PDOWN);
    WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, backup_reg0);

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x3C, &backup_dev1e_reg03ch);
    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x15, &backup_dev1f_reg015h);
    backup_vga = _hal_an8804_phy_tkrg_read(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), 0x1, 0xf, 0x10);

    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x3C, 0x8800);
    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x15, 0x108b);
    value = backup_vga | 0xF8000;
    _hal_an8804_phy_tkrg_write(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), 0x1, 0xF, 0x10, value);

    for (index = 0; index < 16; index++)
    {
        READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        seed |= ((reg_data & 0x1) << index);
    }

    _hal_an8804_phy_tkrg_write(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), 0x1, 0xF, 0x10, backup_vga);
    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x15, backup_dev1f_reg015h);
    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x3C, backup_dev1e_reg03ch);

    backup_reg0 |= BMCR_PDOWN;
    WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, backup_reg0);

    if (AIR_E_OK == rv)
    {
        *random_seed = seed;
    }
    return rv;
}

AIR_ERROR_NO_T
_hal_an8804_phy_calRandomSeed(
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

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rv && (reg_data & MASTER_SLAVE_CFAULT))
    {
        seed = (osal_rand() % 5);

        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, (reg_data | BMCR_PDOWN));
            osal_sleepTask(seed * 10);
            if (AIR_E_OK == rv)
            {
                WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, (reg_data & (~(BMCR_PDOWN))));
                osal_sleepTask(seed * 10);
                /* per port random delay */
                if (AIR_E_OK == rv)
                {
                    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x327, &reg_value);
                    if (AIR_E_OK == rv)
                    {
                        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x327, (reg_value | 0x1F));
                        if (AIR_E_OK == rv)
                        {
                            seed = (osal_rand() % 5);
                            osal_sleepTask(seed * 10);
                            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x327, (reg_value & 0xFFEF));
                            osal_sleepTask(seed * 10);
                            if (AIR_E_OK == rv)
                            {
                                reg_data |= BMCR_ANRESTART;
                                WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_data);
                            }
                        }
                    }
                }
            }
        }
    }
    return rv;
}

AIR_ERROR_NO_T
_hal_an8804_phy_checkInitDone(
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
    return rv;
}

static AIR_ERROR_NO_T
_genphy_update_link(
    UI32_T                 unit,
    UI16_T                 port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         status = 0;
    UI16_T         phy_port = 0;
    UI32_T         reg_addr = 0;
    UI8_T          link_status = 0;
    UI32_T         data = 0;

    /* Do a fake read */
    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMSR, &status);
    if (AIR_E_OK == rv)
    {
        /* Read link and autonegotiation status */
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMSR, &status);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(status, BMSR_ANEGCOMPLETE, HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
            SET_PHY_ABILITY(status, BMSR_RFAULT, HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);
            if (HAL_IS_GPHY_PORT_VALID(unit, port))
            {
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                if (HAL_AN8804_PHY_PER_PSR_PORTS <= phy_port)
                {
                    reg_addr = HAL_AN8804_PHY_PSR_P4_REG;
                }
                else
                {
                    reg_addr = HAL_AN8804_PHY_PSR_P3_P0_REG;
                }
                rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));

                if (AIR_E_OK == rv)
                {
                    link_status = HAL_AN8804_PHY_GET_PSR_LINK_STS(data, phy_port);
                    if (link_status)
                    {
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                    }
                    else
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                    }
                }
            }
            else
            {
                SET_PHY_ABILITY(status, BMSR_LSTATUS, HAL_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_status->flags);
            }
        }
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
    HAL_PHY_AUTO_NEGO_T auto_nego;
    UI16_T              adv = 0, lpa = 0, lpagb = 0, bmcr = 0, common_adv_gb = 0, common_adv = 0;
    UI16_T              speed_status = 0;

    rv = _genphy_update_link(unit, port, ptr_status);
    if (AIR_E_OK == rv)
    {
        rv = hal_an8804_phy_getAutoNego(unit, port, &auto_nego);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
            {
                READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_STAT1000, &lpagb);
                if (AIR_E_OK == rv)
                {
                    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, &adv);
                    if (AIR_E_OK == rv)
                    {
                        common_adv_gb = lpagb & adv << 2;
                        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_LPA, &lpa);
                        if (AIR_E_OK == rv)
                        {
                            READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, &adv);
                            if (AIR_E_OK == rv)
                            {
                                common_adv = lpa & adv;

                                ptr_status->speed = HAL_PHY_SPEED_10M;
                                ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
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
                            }
                        }
                    }
                }
            }
            else
            {
                READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &bmcr);
                if (AIR_E_OK == rv)
                {
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
                }
            }
            /* Workaround for the issue of obtaining incorrect speeds after enabling EEE and downshift to 100M
               on RTL ic */
            rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_AUX_CTRL_STA,
                                                  &speed_status);
            if (AIR_E_OK == rv)
            {
                if (ptr_status->speed != BITS_OFF_R(speed_status, 3, 2))
                {
                    DIAG_PRINT(HAL_DBG_WARN,
                               "port %d, adjust link speed from %d to %d, auto-nego speed not the same phy speed \n",
                               port, ptr_status->speed, BITS_OFF_R(speed_status, 3, 2));
                    ptr_status->speed = BITS_OFF_R(speed_status, 3, 2);
                }
            }
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8804_phy_ledConfig(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_behavior;
    UI16_T          led_id = 0, led_config = 0, led_count = 0, data = 0, led_type = 0, phy_port = 0;
    UI16_T          link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    UI16_T          on_rate_addr = 0, blk_rate_addr = 0;
    UI32_T          reg_data = 0;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = HAL_AN8804_PHY_LED_COUNT;
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_behavior);
    led_type = led_behavior.value;

    /* LED configuration */
    for (led_id = 0; led_id < led_count; led_id++)
    {
        data |= (1 << led_id);
    }
    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CTRL_SELECT, data);

    for (led_id = 0; led_id < led_count; led_id++)
    {
        if (led_id == 0)
        {
            link_reg_addr = HAL_AN8804_PHY_LED_0_ON_MASK;
            blk_reg_addr = HAL_AN8804_PHY_LED_0_BLK_MASK;
            on_rate_addr = HAL_AN8804_PHY_LED_0_ON_DURATION;
            blk_rate_addr = HAL_AN8804_PHY_LED_0_BLK_DURATION;
        }
        else if (led_id == 1)
        {
            link_reg_addr = HAL_AN8804_PHY_LED_1_ON_MASK;
            blk_reg_addr = HAL_AN8804_PHY_LED_1_BLK_MASK;
            on_rate_addr = HAL_AN8804_PHY_LED_1_ON_DURATION;
            blk_rate_addr = HAL_AN8804_PHY_LED_1_BLK_DURATION;
        }
        else if (led_id == 2)
        {
            link_reg_addr = HAL_AN8804_PHY_LED_2_ON_MASK;
            blk_reg_addr = HAL_AN8804_PHY_LED_2_BLK_MASK;
            on_rate_addr = HAL_AN8804_PHY_LED_2_ON_DURATION;
            blk_rate_addr = HAL_AN8804_PHY_LED_2_BLK_DURATION;
        }
        else
        {
            link_reg_addr = HAL_AN8804_PHY_LED_3_ON_MASK;
            blk_reg_addr = HAL_AN8804_PHY_LED_3_BLK_MASK;
            on_rate_addr = HAL_AN8804_PHY_LED_3_ON_DURATION;
            blk_rate_addr = HAL_AN8804_PHY_LED_3_BLK_DURATION;
        }

        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

        led_behavior.value = 0xFFFF;
        led_behavior.param0 = port;
        led_behavior.param1 = led_id;

        hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
        led_config = led_behavior.value;

        link_reg_data = 0;
        blk_reg_data = 0;

        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_LINK_1000, HAL_AN8804_PHY_LED_LINK_1000, link_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_LINK_100, HAL_AN8804_PHY_LED_LINK_100, link_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_LINK_10, HAL_AN8804_PHY_LED_LINK_10, link_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_LINK_FULLDPLX, HAL_AN8804_PHY_LED_LINK_FULLDPLX,
                        link_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_LINK_HALFDPLX, HAL_AN8804_PHY_LED_LINK_HALFDPLX,
                        link_reg_data);

        if (HAL_AN8804_PHY_LED_TYPE_SERIAL == led_type)
        {
            GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_HIGH_ACTIVE, HAL_AN8804_PHY_LED_POL_HIGH_ACT,
                            link_reg_data);
        }
        else
        {
            link_reg_data |= HAL_AN8804_PHY_LED_POL_HIGH_ACT;
        }

        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_TX_1000, HAL_AN8804_PHY_LED_BLINK_TX_1000,
                        blk_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_RX_1000, HAL_AN8804_PHY_LED_BLINK_RX_1000,
                        blk_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_TX_100, HAL_AN8804_PHY_LED_BLINK_TX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_RX_100, HAL_AN8804_PHY_LED_BLINK_RX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_TX_10, HAL_AN8804_PHY_LED_BLINK_TX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, HAL_AN8804_PHY_LED_BHV_BLINK_RX_10, HAL_AN8804_PHY_LED_BLINK_RX_10, blk_reg_data);

        if (link_reg_data != 0 || blk_reg_data != 0)
        {
            link_reg_data |= HAL_AN8804_PHY_LED_FUNC_ENABLE;
        }

        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, on_rate_addr, HAL_AN8804_PHY_LED_ON_RATE_DEFAULT);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, blk_rate_addr, HAL_AN8804_PHY_LED_BLINK_RATE_DEFAULT);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, link_reg_addr, link_reg_data);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, blk_reg_addr, blk_reg_data);

        if (HAL_AN8804_PHY_LED_TYPE_PARALLEL == led_type)
        {
            if (AIR_E_OK == rv)
            {
                READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_L_INV_REG, &reg_data, sizeof(reg_data));
                if (AIR_E_OK == rv)
                {
                    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                    if (led_config & HAL_AN8804_PHY_LED_BHV_HIGH_ACTIVE)
                    {
                        /* high active */
                        reg_data &= ~(1 << (_hal_an8804_phy_pled_gpio[phy_port]));
                    }
                    else
                    {
                        /* low active */
                        reg_data |= (1 << (_hal_an8804_phy_pled_gpio[phy_port]));
                    }
                    WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_L_INV_REG, &reg_data, sizeof(reg_data));
                }
            }
        }

        /* reset blink duration */
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x720, 0x1f);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x720, 0x0);
    }

    return rv;
}

/* FUNCTION NAME:   _hal_an8804_phy_restoreConfigFromEfuse
 * PURPOSE:
 *      Restore AN8804 PHY config from efuse
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
_hal_an8804_phy_restoreConfigFromEfuse(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         data = 0, phy_port = 0;
    UI32_T         efuse_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);

    /* i2mpb_a_tbt */
    reg_addr = HAL_PHY_AN8804_EFUSE_DATA5 + (phy_port * 0x10);
    READ_REG_BY_PORT(unit, port, reg_addr, &efuse_data, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x12, &data);
        data &= ~(BITS(0, 5));
        data |= BITS_OFF_R(efuse_data, 12, 6);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x12, data);
    }

    /* i2mpb_b_tbt */
    reg_addr = HAL_PHY_AN8804_EFUSE_DATA6 + (phy_port * 0x10);
    READ_REG_BY_PORT(unit, port, reg_addr, &efuse_data, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x17, &data);
        data &= ~(BITS(0, 5));
        data |= BITS_OFF_R(efuse_data, 12, 6);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x17, data);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8804_phy_applyParam
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
_hal_an8804_phy_applyParam(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        set_page,
    UI32_T             *ptr_page,
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
            if (MMD_DEV_VSPEC2 == phy_cfg.reg_addr)
            {
                switch (phy_cfg.reg_data)
                {
                    case 0x0:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_0000;
                        break;
                    case 0x2A30:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_2A30;
                        break;
                    case 0x52B5:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_52B5;
                        break;
                    case 0x1:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_0001;
                        break;
                    case 0x2:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_0002;
                        break;
                    case 0x3:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_0003;
                        break;
                    case 0x4:
                        *ptr_page = HAL_AN8804_PHY_PHY_PAGE_0004;
                        break;
                    default:
                        rv = AIR_E_BAD_PARAMETER;
                        break;
                }
                DIAG_PRINT(HAL_DBG_INFO, "[CL22]change page to %u\n", *ptr_page);
            }
            else
            {
                WRITE_CL22_BY_PORT(unit, port, set_page, phy_cfg.reg_addr, phy_cfg.reg_data);
                DIAG_PRINT(HAL_DBG_INFO, "[CL22]port %u, page %u, reg_addr 0x%x, reg_data 0x%x\n", port, set_page,
                           phy_cfg.reg_addr, phy_cfg.reg_data);
            }
        }
        else
        {
            WRITE_CL45_BY_PORT(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, phy_cfg.reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "[CL45]port %u, dev 0x%x, reg_addr 0x%x, reg_data 0x%x\n", port, phy_cfg.device_id,
                       phy_cfg.reg_addr, phy_cfg.reg_data);
        }
    }
    else
    {
        if (HAL_PHY_ACCESS_METHOD_CL22 == phy_cfg.access_method)
        {
            READ_CL22_BY_PORT(unit, port, set_page, phy_cfg.reg_addr, &data);
        }
        else
        {
            READ_CL45_BY_PORT(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, &data);
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
                WRITE_CL22_BY_PORT(unit, port, set_page, phy_cfg.reg_addr, data);
                DIAG_PRINT(HAL_DBG_INFO, "[CL22]port %u, page %u, reg_addr 0x%x, reg_data 0x%x\n", port, set_page,
                           phy_cfg.reg_addr, phy_cfg.reg_data);
            }
            else
            {
                WRITE_CL45_BY_PORT(unit, port, phy_cfg.device_id, phy_cfg.reg_addr, data);
                DIAG_PRINT(HAL_DBG_INFO, "[CL45]port %u, dev 0x%x, reg_addr 0x%x, reg_data 0x%x\n", port,
                           phy_cfg.device_id, phy_cfg.reg_addr, phy_cfg.reg_data);
            }
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8804_phy_enhanceIotVga(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T surge_mode;

    osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);

    if ((HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value) && (HAL_PHY_OP_MODE_NORMAL == mode))
    {
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x12, 0x66);
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x11, 0x4d2a);
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x10, 0x8fa4);
    }
    else
    {
        /* default value */
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x12, 0x3e);
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x11, 0x4d2a);
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_52B5, 0x10, 0x8fa4);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8804_phy_setPhyOpMode
 * PURPOSE:
 *      Set AN8804 PHY operation mode
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
_hal_an8804_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI32_T                count = 0, page = 0;
    HAL_PHY_CFG_T         phy_cfg;
    HAL_PHY_AN_ADV_T      hal_adv;
    HAL_PHY_ADMIN_STATE_T admin_state = HAL_PHY_ADMIN_STATE_LAST;

    osal_memset(&phy_cfg, 0, sizeof(HAL_PHY_CFG_T));

    rv = hal_an8804_phy_getAdminState(unit, port, &admin_state);
    if ((AIR_E_OK == rv) && (HAL_PHY_ADMIN_STATE_DISABLE == admin_state))
    {
        rv = hal_an8804_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_ENABLE);
    }
    if (HAL_PHY_OP_MODE_NORMAL == mode)
    {
        for (count = 0; count < _hal_an8804_normal_size; count++)
        {
            if (AIR_E_OK == rv)
            {
                phy_cfg = _hal_an8804_normal[count];
                rv = _hal_an8804_phy_applyParam(unit, port, page, &page, phy_cfg);
            }
        }

        if (AIR_E_OK == rv)
        {
            rv = _hal_an8804_phy_restoreConfigFromEfuse(unit, port);
        }
    }
    else if (HAL_PHY_OP_MODE_LONG_REACH == mode)
    {
        rv = hal_an8804_phy_getLocalAdvAbility(unit, port, &hal_adv);
        if (AIR_E_OK == rv)
        {
            if (hal_adv.flags & HAL_PHY_AN_ADV_FLAGS_EEE)
            {
                rv = AIR_E_OP_INVALID;
            }
            else
            {
                for (count = 0; count < _hal_an8804_longreach_size; count++)
                {
                    if (AIR_E_OK == rv)
                    {
                        phy_cfg = _hal_an8804_longreach[count];
                        rv = _hal_an8804_phy_applyParam(unit, port, page, &page, phy_cfg);
                    }
                }
            }
        }
    }
    else
    {
        rv = AIR_E_BAD_PARAMETER;
    }
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8804_phy_enhanceIotVga(unit, port, mode);
    }
    if (HAL_PHY_ADMIN_STATE_DISABLE == admin_state)
    {
        rv |= hal_an8804_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_DISABLE);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8804_phy_setSurgeProtect
 * PURPOSE:
 *      Set AN8804 PHY surge protection
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
_hal_an8804_phy_setSurgeProtect(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T surge_mode;
    UI32_T          reg_addr = 0, efuse_data = 0, shift_sel = 0;
    UI32_T          efuse_data_raw[4] = {}, raw = 0;
    UI16_T          reg_data = 0, phy_port = 0;

    osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    for (raw = 0; raw < HAL_PHY_AN8804_EFUSE_SIZE_PER_PHY; raw++)
    {
        reg_addr = (HAL_PHY_AN8804_EFUSE_DATA0 +
                    (HAL_PHY_AN8804_EFUSE_SIZE_PER_PHY *
                     (HAL_PHY_AN8804_EFUSE_PHY_BASE + raw + (HAL_PHY_AN8804_WORD_SIZE * phy_port))));
        READ_REG_BY_PORT(unit, port, reg_addr, &efuse_data, sizeof(UI32_T));
        if (AIR_E_OK == rv)
        {
            efuse_data_raw[raw] = efuse_data;
        }
    }

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8804 surge protection mode - 0R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[0], 24, 7);
            reg_data |= (efuse_data << 8);
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[1], 24, 7);
            reg_data |= efuse_data;
        }
        else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8804 surge protection mode - 5R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[0], 24, 7);
            shift_sel = _hal_an8804_phy_shift_check(efuse_data);
            efuse_data = _hal_an8804_phy_get_shift_val(shift_sel);
            reg_data |= (efuse_data << 8);
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[1], 24, 7);
            shift_sel = _hal_an8804_phy_shift_check(efuse_data);
            efuse_data = _hal_an8804_phy_get_shift_val(shift_sel);
            reg_data |= efuse_data;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "port %u, an8804 surge protection mode %u is invalid!\n", port, surge_mode.value);
            return AIR_E_BAD_PARAMETER;
        }
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x174, reg_data);
    }

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
        {
            /* cr_r50ohm_rsel_tx_c */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[2], 24, 7);
            reg_data |= (efuse_data << 8);
            /* cr_r50ohm_rsel_tx_d */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[3], 24, 7);
            reg_data |= efuse_data;
        }
        else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
        {
            /* cr_r50ohm_rsel_tx_c */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[2], 24, 7);
            shift_sel = _hal_an8804_phy_shift_check(efuse_data);
            efuse_data = _hal_an8804_phy_get_shift_val(shift_sel);
            reg_data |= (efuse_data << 8);
            /* cr_r50ohm_rsel_tx_d */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[3], 24, 7);
            shift_sel = _hal_an8804_phy_shift_check(efuse_data);
            efuse_data = _hal_an8804_phy_get_shift_val(shift_sel);
            reg_data |= efuse_data;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "port %u, an8804 surge protection mode %u is invalid!\n", port, surge_mode.value);
            return AIR_E_BAD_PARAMETER;
        }
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x175, reg_data);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8804_phy_setWaveGen
 * PURPOSE:
 *      Set AN8804 PHY wave-gen
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
_hal_an8804_phy_setWaveGen(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0;
    AIR_CFG_VALUE_T led_behavior;

    READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_CYCLE_CFG_VALUE1_REG, &reg_data, sizeof(UI32_T));
    reg_data = (reg_data & 0x000000ff) | HAL_AN8804_PHY_WAVE_GEN_CYCLE_567;
    WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_CYCLE_CFG_VALUE1_REG, &reg_data, sizeof(UI32_T));

    /* set wave-gen pattern */
    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = 0xFFFF;
    led_behavior.param0 = port;
    led_behavior.param1 = 0;
    rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
    if (led_behavior.value & HAL_AN8804_PHY_LED_BHV_HIGH_ACTIVE)
    {
        /* high active */
        reg_data = (HAL_AN8804_PHY_PATTERN_OFF << 16) | HAL_AN8804_PHY_PATTERN_ON;
    }
    else
    {
        /* low active */
        reg_data = (HAL_AN8804_PHY_PATTERN_ON << 16) | HAL_AN8804_PHY_PATTERN_OFF;
    }
    WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_FLASH_PRD_SET2_REG, &reg_data, sizeof(UI32_T));
    return rv;
}

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME: hal_an8804_phy_init
 * PURPOSE:
 *      AN8804 PHY initialization
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
hal_an8804_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         data = 0, random_seed = 0, seed = 0, bus_id = 0;
    UI32_T         phy_base = 0, phy_addr = 0;
    UI16_T         reg_data = 0;
    BOOL_T         init_status = FALSE;

    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, 0);
    rv = _hal_an8804_phy_ledConfig(unit, port);
    if (AIR_E_OK == rv)
    {
        /* EMI patch */
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x13, 0x4040);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0xD8, 0x1010);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0xD9, 0x0100);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0xDA, 0x0100);

        rv = _hal_an8804_phy_setSurgeProtect(unit, port);
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8804_phy_setPhyOpMode(unit, port, HAL_PHY_OP_MODE_NORMAL);
            if (AIR_E_OK == rv)
            {
                rv = _hal_an8804_phy_checkInitDone(unit, port, &init_status);
                if ((AIR_E_OK == rv) && (TRUE == init_status))
                {
                    return rv;
                }
            }
        }

        READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_HW_TRAP_READ_REG, &data, sizeof(UI32_T));
        if (AIR_E_OK == rv)
        {
            phy_base = (data >> HAL_AN8804_PHY_PHY_BASE_ADDR_SHIFT) & HAL_AN8804_PHY_PHY_BASE_ADDR_MASK;
            rv = _hal_an8804_phy_getRandomSeed(unit, port, &random_seed);
            if (AIR_E_OK == rv)
            {
                /* set random seed */
                for (phy_addr = phy_base; phy_addr < (phy_base + 5); phy_addr++)
                {
                    rv = hal_mdio_readC45FromIntBus(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x12, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        random_seed += reg_data;
                    }
                }
                osal_srand(random_seed);

                for (phy_addr = phy_base; phy_addr < (phy_base + 5); phy_addr++)
                {
                    rv |= hal_mdio_writeC22FromIntBus(unit, bus_id, phy_addr, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR,
                                                      0x200); /* gphy power up */
                }

                for (phy_addr = phy_base; phy_addr < (phy_base + 5); phy_addr++)
                {
                    seed = osal_rand() % 100000;
                    rv |= hal_mdio_readC45FromIntBus(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, &reg_data);
                    reg_data &= ~(0xF);
                    reg_data |= (0x10 | (seed & 0xF)); /* use seed 0~3bit as random seed */

                    rv |= hal_mdio_writeC45FromIntBus(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, reg_data);
                    osal_delayUs(seed);
                    rv |= hal_mdio_writeC45FromIntBus(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x327, reg_data & 0xFFEF);

                    rv |= hal_mdio_writeC45FromIntBus(unit, bus_id, phy_addr, MMD_DEV_VSPEC1, 0x145, 0x1000);
                    rv |= hal_mdio_writeC22FromIntBus(unit, bus_id, phy_addr, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR,
                                                      0x1200); /* re-AN */
                }
            }
        }

        if (AIR_E_OK == rv)
        {
            _hal_an8804_phy_setWaveGen(unit, port);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setAdminState
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
hal_an8804_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
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
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_value);
        if ((AIR_E_OK == rv) && (HAL_PHY_ADMIN_STATE_ENABLE == state))
        {
            /* Re-train RX gain */
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x148, 0x200);
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x148, 0x0);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getAdminState
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
hal_an8804_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
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

/* FUNCTION NAME:   hal_an8804_phy_setAutoNego
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
hal_an8804_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
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
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getAutoNego
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
hal_an8804_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
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

/* FUNCTION NAME:   hal_an8804_phy_setLocalAdvAbility
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
hal_an8804_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    UI16_T            reg_value = 0;
    HAL_PHY_OP_MODE_T phy_mode;

    if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE)
    {
        rv = hal_an8804_phy_getPhyOpMode(unit, port, &phy_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_LONG_REACH == phy_mode))
        {
            rv = AIR_E_OP_INVALID;
        }
    }

    if (AIR_E_OK == rv)
    {
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, &reg_value);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ADVERTISE_1000FULL, reg_value);
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, reg_value);
        }

        if (AIR_E_OK == rv)
        {
            READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, &reg_value);
            if (AIR_E_OK == rv)
            {
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10HFDX, ADVERTISE_10HALF, reg_value);
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10FUDX, ADVERTISE_10FULL, reg_value);
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100HFDX, ADVERTISE_100HALF, reg_value);
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100FUDX, ADVERTISE_100FULL, reg_value);
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ADVERTISE_PAUSE_CAP, reg_value);
                SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ADVERTISE_PAUSE_ASYM, reg_value);
                WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, reg_value);
            }
        }

        if (AIR_E_OK == rv)
        {
            reg_value = (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) ? (EEE_1000BASE_T | EEE_100BASE_TX) : 0;
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_value);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getLocalAdvAbility
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
hal_an8804_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    ptr_adv->flags = 0;
    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, ADVERTISE_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, ADVERTISE_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, ADVERTISE_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
    }

    READ_CL45_BY_PORT(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getRemoteAdvAbility
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
hal_an8804_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    ptr_adv->flags = 0;
    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_STAT1000, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, LPA_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_LPA, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, LPA_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, LPA_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, LPA_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, LPA_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, LPA_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
        GET_PHY_ABILITY(reg_value, LPA_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
    }

    READ_CL45_BY_PORT(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &reg_value);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_value, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setSpeed
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
hal_an8804_phy_setSpeed(
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

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
    if (AIR_E_OK == rv)
    {
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
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_value);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getSpeed
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
hal_an8804_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
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
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setDuplex
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
hal_an8804_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_value);
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
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_value);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getDuplex
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
hal_an8804_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & BMCR_FULLDPLX)
        {
            *ptr_duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
            *ptr_duplex = HAL_PHY_DUPLEX_HALF;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getLinkStatus
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
hal_an8804_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;

    rv = _genphy_read_status(unit, port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] flags=(%x)\n", unit, port, ptr_status->flags);
    rv = _hal_an8804_phy_calRandomSeed(unit, port,
                                       (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP ? TRUE : FALSE));
    if (AIR_E_OK == rv)
    {
        READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x0c6, &reg_data);
        if (AIR_E_OK == rv)
        {
            if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (HAL_PHY_SPEED_1000M == ptr_status->speed))
            {
                if (HAL_PHY_AN8804_1G_LOW_POWER != reg_data)
                {
                    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_PHY_AN8804_1G_LOW_POWER);
                }
            }
            else if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) &&
                     ((HAL_PHY_SPEED_100M == ptr_status->speed) || (HAL_PHY_SPEED_10M == ptr_status->speed)))
            {
                if (HAL_PHY_AN8804_100M_10M_LOW_POWER != reg_data)
                {
                    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_PHY_AN8804_100M_10M_LOW_POWER);
                }
            }
            else
            {
                if (HAL_PHY_AN8804_UNPLUG_LOW_POWER != reg_data)
                {
                    WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_PHY_AN8804_UNPLUG_LOW_POWER);
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setLoopBack
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
hal_an8804_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_LPBK_FAR_END == lpbk_type)
    {
        /* Read data from ExtReg1A */
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, MII_RESV2, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (TRUE == enable)
            {
                reg_data |= HAL_AN8804_PHY_LPBK_FAR_END;
            }
            else
            {
                reg_data &= ~(HAL_AN8804_PHY_LPBK_FAR_END);
            }
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, MII_RESV2, reg_data);
        }
    }
    else
    {
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (TRUE == enable)
            {
                reg_data |= BMCR_LOOPBACK;
            }
            else
            {
                reg_data &= ~(BMCR_LOOPBACK);
            }
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_data);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getLoopBack
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
hal_an8804_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_LPBK_FAR_END == lpbk_type)
    {
        /* Read data from ExtReg1A */
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, MII_RESV2, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (reg_data & HAL_AN8804_PHY_LPBK_FAR_END)
            {
                *ptr_enable = TRUE;
            }
            else
            {
                *ptr_enable = FALSE;
            }
        }
    }
    else
    {
        READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            if (reg_data & BMCR_LOOPBACK)
            {
                *ptr_enable = TRUE;
            }
            else
            {
                *ptr_enable = FALSE;
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setSmartSpeedDown
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
hal_an8804_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode)
{
    UI16_T         data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if ((HAL_PHY_SSD_MODE_1T == ssd_mode) || (HAL_PHY_SSD_MODE_5T < ssd_mode))
    {
        return AIR_E_BAD_PARAMETER;
    }

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x14, &data);
    if (AIR_E_OK == rv)
    {
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
        WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x14, data);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getSmartSpeedDown
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
hal_an8804_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    READ_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x14, &data);
    if (AIR_E_OK == rv)
    {
        reg_data = BITS_OFF_R(data, 4, 1);
        if (reg_data)
        {
            *ptr_ssd_mode = (BITS_OFF_R(data, 2, 2) + HAL_PHY_SSD_MODE_2T);
        }
        else
        {
            *ptr_ssd_mode = HAL_PHY_SSD_MODE_DISABLE;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setLedOnCtrl
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
hal_an8804_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getLedOnCtrl
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
hal_an8804_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (led_id == 0)
    {
        reg_addr = HAL_AN8804_PHY_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = HAL_AN8804_PHY_LED_1_ON_MASK;
    }
    else if (led_id == 2)
    {
        reg_addr = HAL_AN8804_PHY_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = HAL_AN8804_PHY_LED_3_ON_MASK;
    }

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & HAL_AN8804_PHY_LED_LINK_FORCE_ON)
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

/* FUNCTION NAME:   hal_an8804_phy_testTxCompliance
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
hal_an8804_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         u16dat = 0;

    rv |= hal_mdio_writeC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, 0x0);
    switch (mode)
    {
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B:
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, 0x8000);
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, 0x100);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A == mode))
            {
                u16dat = 0x5010;
            }
            else
            {
                u16dat = 0x5018;
            }
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x33, 0x177);
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC2, 0x27b, 0x1177);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0xd2;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0x2;
            }
            else
            {
                u16dat = 0x0;
            }
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x1a3, u16dat);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0x110e;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0x1100;
            }
            else
            {
                u16dat = 0x0;
            }
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x1a4, u16dat);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0x0;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0xf842;
            }
            else
            {
                /* 10M sine pair a/b*/
                u16dat = 0xf840;
            }
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, MII_AUX_CTRL_STA, u16dat);
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, 0x0);
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE:
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, 0x2100);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                u16dat = 0x5010;
            }
            else
            {
                u16dat = 0x5018;
            }
            WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, 0x0);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x23, 0x882);
            }
            else
            {
                WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x24, 0x882);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3:
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
            {
                u16dat = 0x2700;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2 == mode)
            {
                u16dat = 0x4700;
            }
            else
            {
                u16dat = 0x6700;
            }
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, u16dat);
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
            {
                WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x14, 0x4444);
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
            {
                WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x143, 0x200);
            }
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, 0x1040);
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D:
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, 0x8700);
            WRITE_CL22_BY_PORT(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, 0x1040);
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A == mode)
            {
                u16dat = 0x7878;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B == mode)
            {
                u16dat = 0xb8b8;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C == mode)
            {
                u16dat = 0xd8d8;
            }
            else
            {
                /* HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D */
                u16dat = 0xe8e8;
            }
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4 != mode)
            {
                WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x3e, u16dat);
            }
            break;
        /* not support 2.5G at 8858 */
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6:
            rv = AIR_E_NOT_SUPPORT;
            break;
        default:
            rv = AIR_E_BAD_PARAMETER;
            break;
    }
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Port %u test tx compliance mode %u failed! rv=%u\n", port, mode, rv);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setPhyLedCtrlMode
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
hal_an8804_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    AIR_CFG_VALUE_T     led_count;
    UI16_T              phy_port = 0;
    UI32_T              reg_data = 0, bit_mask = 0;
    HAL_PHY_LED_STATE_T state = HAL_PHY_LED_STATE_LAST;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);

    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PARAM((ctrl_mode >= HAL_PHY_LED_CTRL_MODE_LAST), AIR_E_BAD_PARAMETER);
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_count);

    if (HAL_AN8804_PHY_LED_TYPE_SERIAL == led_count.value)
    {
        READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_SLED_SRC_SEL_REG, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rv)
        {
            bit_mask = HAL_AN8804_PHY_LED_SRC_SEL_POS(phy_port, led_id);
            if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                reg_data |= bit_mask;
            }
            else
            {
                reg_data &= ~(bit_mask);
            }
            WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_SLED_SRC_SEL_REG, &reg_data, sizeof(reg_data));
            if (AIR_E_OK == rv)
            {
                if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
                {
                    rv = hal_an8804_phy_getPhyLedForceState(unit, port, led_id, &state);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_an8804_phy_setPhyLedForceState(unit, port, led_id, state);
                    }
                }
                else
                {
                    rv = _hal_an8804_phy_ledConfig(unit, port);
                }
            }
        }
    }
    else if (HAL_AN8804_PHY_LED_TYPE_PARALLEL == led_count.value)
    {
        READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_FLASH_MODE_REG, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rv)
        {
            bit_mask = (1 << (_hal_an8804_phy_pled_gpio[phy_port]));
            if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                reg_data |= bit_mask;
            }
            else
            {
                reg_data &= ~(bit_mask);
            }
            WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_FLASH_MODE_REG, &reg_data, sizeof(reg_data));
            if (AIR_E_OK == rv)
            {
                READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_LAN_LED_IOMUX_REG, &reg_data, sizeof(reg_data));
                if (AIR_E_OK == rv)
                {
                    if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
                    {
                        reg_data &= ~(bit_mask);
                    }
                    else
                    {
                        reg_data |= bit_mask;
                    }
                    WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_LAN_LED_IOMUX_REG, &reg_data, sizeof(reg_data));
                }
            }
            if (AIR_E_OK == rv)
            {
                READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_OE_REG, &reg_data, sizeof(reg_data));
                if (AIR_E_OK == rv)
                {
                    if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
                    {
                        reg_data |= bit_mask;
                    }
                    else
                    {
                        reg_data &= ~(bit_mask);
                    }
                    WRITE_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_OE_REG, &reg_data, sizeof(reg_data));
                }
            }
            if (AIR_E_OK == rv)
            {
                if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
                {
                    rv = hal_an8804_phy_getPhyLedForceState(unit, port, led_id, &state);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_an8804_phy_setPhyLedForceState(unit, port, led_id, state);
                    }
                }
                else
                {
                    rv = _hal_an8804_phy_ledConfig(unit, port);
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

/* FUNCTION NAME:   hal_an8804_phy_getPhyLedCtrlMode
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
hal_an8804_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_count;
    AIR_CFG_VALUE_T led_type;
    UI16_T          phy_port = 0;
    UI32_T          reg_data = 0, bit_mask = 0;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);

    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);

    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);

    if (HAL_AN8804_PHY_LED_TYPE_SERIAL == led_type.value)
    {
        READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_SLED_SRC_SEL_REG, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rv)
        {
            bit_mask = HAL_AN8804_PHY_LED_SRC_SEL_POS(phy_port, led_id);
            if (reg_data & bit_mask)
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_FORCE;
            }
            else
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_PHY;
            }
        }
    }
    else if (HAL_AN8804_PHY_LED_TYPE_PARALLEL == led_type.value)
    {
        READ_REG_BY_PORT(unit, port, HAL_AN8804_PHY_GPIO_FLASH_MODE_REG, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rv)
        {
            bit_mask = (1 << (_hal_an8804_phy_pled_gpio[phy_port]));
            if (reg_data & bit_mask)
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_FORCE;
            }
            else
            {
                *ptr_ctrl_mode = HAL_PHY_LED_CTRL_MODE_PHY;
            }
        }
    }
    else
    {
        rv = AIR_E_BAD_PARAMETER;
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setPhyLedForceState
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
hal_an8804_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    AIR_CFG_VALUE_T         led_count;
    AIR_CFG_VALUE_T         led_type;
    UI16_T                  phy_port = 0, data = 0, offset = 0;
    UI32_T                  reg_addr = 0, reg_data = 0, bit_mask = 0, flash_set_pos = 0;
    HAL_PHY_LED_PATT_T      led_patt = HAL_PHY_LED_PATT_LAST;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode = HAL_PHY_LED_CTRL_MODE_LAST;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);

    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PARAM((state >= HAL_PHY_LED_STATE_LAST), AIR_E_BAD_PARAMETER);

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (HAL_AN8804_PHY_LED_CFG_RG_OFFSET * led_id);
        data &= ~(BITS_OFF_L(HAL_AN8804_PHY_LED_CFG_STATE_MASK, offset, HAL_AN8804_PHY_LED_CFG_STATE_WIDTH));
        data |= BITS_OFF_L(state, offset, HAL_AN8804_PHY_LED_CFG_STATE_WIDTH);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
        hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);
        if (HAL_AN8804_PHY_LED_TYPE_SERIAL == led_type.value)
        {
            rv = hal_an8804_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
            {
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                flash_set_pos = HAL_AN8804_PHY_LED_FLASH_SET_POS(phy_port, led_id);
                reg_addr = HAL_AN8804_PHY_RG_LED_FLASH_SET_CTRL(phy_port);
                READ_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                if (AIR_E_OK == rv)
                {
                    bit_mask = (HAL_AN8804_PHY_LED_FLASH_SET_MASK << flash_set_pos);
                    reg_data &= ~(bit_mask);
                    switch (state)
                    {
                        case HAL_PHY_LED_STATE_FORCE_PATT:
                            rv = hal_an8804_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_an8804_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                            }
                            break;
                        case HAL_PHY_LED_STATE_OFF:
                            reg_data |= (HAL_AN8804_PHY_FLASH_SET_FORCE_OFF << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                            break;
                        case HAL_PHY_LED_STATE_ON:
                            reg_data |= (HAL_AN8804_PHY_FLASH_SET_FORCE_ON << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                            break;
                        default:
                            rv = AIR_E_BAD_PARAMETER;
                            break;
                    }
                }
            }
        }
        else if (HAL_AN8804_PHY_LED_TYPE_PARALLEL == led_type.value)
        {
            rv = hal_an8804_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
            {
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                flash_set_pos = HAL_AN8804_PHY_GPIO_FLASH_MAP_CFG_POS(_hal_an8804_phy_pled_gpio[phy_port]);
                reg_addr = HAL_AN8804_PHY_GPIO_FLASH_CFG0_REG;
                READ_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                {
                    bit_mask = (HAL_AN8804_PHY_LED_FLASH_SET_MASK << flash_set_pos);
                    reg_data &= ~(bit_mask);
                    switch (state)
                    {
                        case HAL_PHY_LED_STATE_FORCE_PATT:
                            rv = hal_an8804_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_an8804_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                            }
                            break;
                        case HAL_PHY_LED_STATE_OFF:
                            reg_data |= ((BIT(3) | HAL_AN8804_PHY_FLASH_SET_FORCE_OFF) << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                            break;
                        case HAL_PHY_LED_STATE_ON:
                            reg_data |= ((BIT(3) | HAL_AN8804_PHY_FLASH_SET_FORCE_ON) << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                            break;
                        default:
                            rv = AIR_E_BAD_PARAMETER;
                            break;
                    }
                }
            }
        }
        else
        {
            rv = AIR_E_BAD_PARAMETER;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getPhyLedForceState
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
hal_an8804_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_count;
    UI16_T          data = 0, offset = 0;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);

    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (HAL_AN8804_PHY_LED_CFG_RG_OFFSET * led_id);
        *ptr_state = BITS_OFF_R(data, offset, HAL_AN8804_PHY_LED_CFG_STATE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_setPhyLedForcePattCfg
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
hal_an8804_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    AIR_CFG_VALUE_T         led_behavior;
    AIR_CFG_VALUE_T         led_type;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode = HAL_PHY_LED_CTRL_MODE_LAST;
    HAL_PHY_LED_STATE_T     led_state = HAL_PHY_LED_STATE_LAST;
    UI16_T                  phy_port = 0, data = 0, offset = 0;
    UI32_T                  reg_addr = 0, reg_data = 0, bit_mask = 0, wave_id = 0, flash_set_pos = 0;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);

    HAL_CHECK_PARAM((led_id >= led_behavior.value), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PARAM((pattern >= HAL_PHY_LED_PATT_HZ_ZERO), AIR_E_BAD_PARAMETER);

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = HAL_AN8804_PHY_LED_CFG_PATT_WIDTH + (HAL_AN8804_PHY_LED_CFG_RG_OFFSET * led_id);
        data &= ~(BITS_OFF_L(HAL_AN8804_PHY_LED_CFG_PATT_MASK, offset, HAL_AN8804_PHY_LED_CFG_PATT_WIDTH));
        data |= BITS_OFF_L(pattern, offset, HAL_AN8804_PHY_LED_CFG_PATT_WIDTH);
        WRITE_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
        hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);
        if (HAL_AN8804_PHY_LED_TYPE_SERIAL == led_type.value)
        {
            rv = hal_an8804_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
            {
                rv = hal_an8804_phy_getPhyLedForceState(unit, port, led_id, &led_state);
                if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
                {
                    switch (pattern)
                    {
                        case HAL_PHY_LED_PATT_HZ_HALF:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_HALF_HZ;
                            break;
                        case HAL_PHY_LED_PATT_HZ_ONE:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_ONE_HZ;
                            break;
                        case HAL_PHY_LED_PATT_HZ_TWO:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_TWO_HZ;
                            break;
                        default:
                            rv = AIR_E_BAD_PARAMETER;
                            break;
                    }
                    if (AIR_E_OK == rv)
                    {
                        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                        flash_set_pos = HAL_AN8804_PHY_LED_FLASH_SET_POS(phy_port, led_id);
                        reg_addr = HAL_AN8804_PHY_RG_LED_FLASH_SET_CTRL(phy_port);
                        READ_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                        if (AIR_E_OK == rv)
                        {
                            bit_mask = (HAL_AN8804_PHY_LED_FLASH_SET_MASK << flash_set_pos);
                            reg_data &= ~(bit_mask);
                            reg_data |= (wave_id << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                        }
                    }
                }
            }
        }
        else if (HAL_AN8804_PHY_LED_TYPE_PARALLEL == led_type.value)
        {
            rv = hal_an8804_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
            {
                rv = hal_an8804_phy_getPhyLedForceState(unit, port, led_id, &led_state);
                if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
                {
                    switch (pattern)
                    {
                        case HAL_PHY_LED_PATT_HZ_HALF:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_HALF_HZ;
                            break;
                        case HAL_PHY_LED_PATT_HZ_ONE:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_ONE_HZ;
                            break;
                        case HAL_PHY_LED_PATT_HZ_TWO:
                            wave_id = HAL_AN8804_PHY_FLASH_SET_TWO_HZ;
                            break;
                        default:
                            rv = AIR_E_BAD_PARAMETER;
                            break;
                    }
                    if (AIR_E_OK == rv)
                    {
                        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                        flash_set_pos = HAL_AN8804_PHY_GPIO_FLASH_MAP_CFG_POS(_hal_an8804_phy_pled_gpio[phy_port]);
                        reg_addr = HAL_AN8804_PHY_GPIO_FLASH_CFG0_REG;
                        READ_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                        if (AIR_E_OK == rv)
                        {
                            bit_mask = (HAL_AN8804_PHY_LED_FLASH_SET_MASK << flash_set_pos);
                            reg_data &= ~(bit_mask);
                            reg_data |= ((BIT(3) | wave_id) << flash_set_pos);
                            WRITE_REG_BY_PORT(unit, port, reg_addr, &reg_data, sizeof(reg_data));
                        }
                    }
                }
            }
        }
        else
        {
            rv = AIR_E_BAD_PARAMETER;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8804_phy_getPhyLedForcePattCfg
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
hal_an8804_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    UI16_T          data = 0, offset = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);

    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, HAL_AN8804_PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = HAL_AN8804_PHY_LED_CFG_PATT_WIDTH + (HAL_AN8804_PHY_LED_CFG_RG_OFFSET * led_id);
        *ptr_pattern = BITS_OFF_R(data, offset, HAL_AN8804_PHY_LED_CFG_PATT_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME: hal_an8804_phy_triggerCableTest
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
hal_an8804_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_NOT_SUPPORT;
    ret = hal_cmn_phy_triggerCableTest(unit, port, test_pair, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_getCableTestRawData
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
hal_an8804_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_NOT_SUPPORT;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_setPhyLedGlbCfg
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
hal_an8804_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_getPhyLedGlbCfg
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
hal_an8804_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_setPhyLedBlkEvent
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
hal_an8804_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_getPhyLedBlkEvent
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
hal_an8804_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_setPhyLedDuration
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
hal_an8804_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_getPhyLedDuration
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
hal_an8804_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_an8804_phy_setPhyOpMode
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
hal_an8804_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    rv = _hal_an8804_phy_setPhyOpMode(unit, port, mode);
    return rv;
}

/* FUNCTION NAME: hal_an8804_phy_getPhyOpMode
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
hal_an8804_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;

    READ_CL45_BY_PORT(unit, port, MMD_DEV_VSPEC1, 0x03d, &reg_data);
    if (AIR_E_OK == rv)
    {
        switch (reg_data)
        {
            case HAL_PHY_AN8804_LONG_REACH:
                *ptr_mode = HAL_PHY_OP_MODE_LONG_REACH;
                break;
            case HAL_PHY_AN8804_NORMAL:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
            default:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
        }
    }
    return rv;
}

/* FUNCTION NAME: hal_an8804_phy_dumpPhyPara
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
hal_an8804_phy_dumpPhyPara(
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
    rv = hal_an8804_phy_getPhyOpMode(unit, port, &phy_mode);
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
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMCR, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_BMSR, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_ADVERTISE, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_LPA, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_LPA, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_CTRL1000, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0000, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0000, MII_STAT1000, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x12, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0001, 0x12, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x14, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0001, 0x14, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x17, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0001, 0x17, reg_data);
    }
    rv = hal_mdio_readC22FromIntBusByPort(unit, port, HAL_AN8804_PHY_PHY_PAGE_0001, 0x1a, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    page=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                    HAL_AN8804_PHY_PHY_PAGE_0001, 0x1a, reg_data);
    }

    osal_printf("\n=== cl45 ===\n");
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_ANEG, 0x3c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_ANEG, 0x3c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_ANEG, 0x3d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_ANEG, 0x3d, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, 0x3, 0x1, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port, 0x3, 0x1,
                    reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x39, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x39, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x3d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3d, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3e, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xc6, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xc6, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x12, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x12, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x13, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x13, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x14, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x14, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x15, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x15, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x16, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x16, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x17, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x17, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x18, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x18, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x19, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x19, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x20, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x20, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x21, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x21, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x22, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x22, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x171, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x171, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x174, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x175, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x9b, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9b, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x9c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xa2, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xa2, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xe6, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xe6, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xe7, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xe7, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xfe, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xfe, reg_data);
    }

    osal_printf("\n=== TX shapper ===\n");
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x0, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x0, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x1, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x1, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x2, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x2, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x3, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x3, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x4, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x4, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x5, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x5, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x6, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x6, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x7, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x7, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x8, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x8, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x9, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x9, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xa, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xa, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0xb, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0xb, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC1, 0x11, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC1, 0x11, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x200, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x200, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x201, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x201, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x202, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x202, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x203, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x203, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x204, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x204, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x205, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x205, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x206, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x206, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x207, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x207, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x208, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x208, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x209, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x209, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20a, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20a, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20b, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20b, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20d, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20e, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20e, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x20f, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x20f, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x210, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x210, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x211, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x211, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x212, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x212, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x213, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x213, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x214, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x214, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x215, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x215, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x216, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x216, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x217, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x217, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x218, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x218, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x219, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x219, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21a, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21a, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21b, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21b, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21d, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21e, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21e, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x21f, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x21f, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x220, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x220, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x221, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x221, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x222, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x222, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x223, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x223, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x224, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x224, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x225, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x225, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x226, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x226, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x227, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x227, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x228, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x228, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x229, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x229, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22a, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22a, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22b, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22b, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22c, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22c, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22d, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22e, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22e, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x22f, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x22f, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x230, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x230, reg_data);
    }
    rv = hal_mdio_readC45FromIntBusByPort(unit, port, MMD_DEV_VSPEC2, 0x231, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%4u    reg-data=0x%04x\n", unit, port,
                    MMD_DEV_VSPEC2, 0x231, reg_data);
    }
    return rv;
}

const HAL_PHY_DRIVER_T _ext_an8804_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_an8804_phy_init,
    hal_an8804_phy_setAdminState,
    hal_an8804_phy_getAdminState,
    hal_an8804_phy_setAutoNego,
    hal_an8804_phy_getAutoNego,
    hal_an8804_phy_setLocalAdvAbility,
    hal_an8804_phy_getLocalAdvAbility,
    hal_an8804_phy_getRemoteAdvAbility,
    hal_an8804_phy_setSpeed,
    hal_an8804_phy_getSpeed,
    hal_an8804_phy_setDuplex,
    hal_an8804_phy_getDuplex,
    hal_an8804_phy_getLinkStatus,
    hal_an8804_phy_setLoopBack,
    hal_an8804_phy_getLoopBack,
    hal_an8804_phy_setSmartSpeedDown,
    hal_an8804_phy_getSmartSpeedDown,
    hal_an8804_phy_setLedOnCtrl,
    hal_an8804_phy_getLedOnCtrl,
    hal_an8804_phy_testTxCompliance,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_an8804_phy_setPhyLedCtrlMode,
    hal_an8804_phy_getPhyLedCtrlMode,
    hal_an8804_phy_setPhyLedForceState,
    hal_an8804_phy_getPhyLedForceState,
    hal_an8804_phy_setPhyLedForcePattCfg,
    hal_an8804_phy_getPhyLedForcePattCfg,
    hal_an8804_phy_triggerCableTest,
    hal_an8804_phy_getCableTestRawData,
    hal_an8804_phy_setPhyLedGlbCfg,
    hal_an8804_phy_getPhyLedGlbCfg,
    hal_an8804_phy_setPhyLedBlkEvent,
    hal_an8804_phy_getPhyLedBlkEvent,
    hal_an8804_phy_setPhyLedDuration,
    hal_an8804_phy_getPhyLedDuration,
    hal_an8804_phy_setPhyOpMode,
    hal_an8804_phy_getPhyOpMode,
    hal_an8804_phy_dumpPhyPara,
    NULL,
    NULL, /* dumpPortCnt */
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_an8804_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_an8804_phy_func_vec;

    return (AIR_E_OK);
}
