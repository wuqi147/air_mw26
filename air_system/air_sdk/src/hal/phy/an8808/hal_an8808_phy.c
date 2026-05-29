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

/* FILE NAME:  hal_an8808_phy.c
 * PURPOSE:
 *  Implement an8808 phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/an8808/hal_an8808_phy.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_AN8808_PHY_LED_CFG_REG (HAL_AN8808_PHY_LED_3_BLK_MASK)

#define HAL_AN8808_PHY_LONG_REACH         (0xc00)
#define HAL_AN8808_PHY_NORMAL             (0x800)
#define HAL_AN8808_PHY_UNPLUG_LOW_POWER   (0x50aa)
#define HAL_AN8808_PHY_100M_10M_LOW_POWER (0x53aa)
#define HAL_AN8808_PHY_1G_LOW_POWER       (0x5faa)

#define TKRG_PKT_XMT_STA         BIT(15)
#define TKRG_DATA_ADDR_OFFSET    (1)
#define TKRG_NODE_ADDR_OFFSET    (7)
#define TKRG_CHANNEL_ADDR_OFFSET (11)
#define TKRG_WR_RD_CTRL_OFFSET   (13)
#define TKRG_READ_CTRL           (1 << TKRG_WR_RD_CTRL_OFFSET)
#define TKRG_WRITE_CTRL          (0 << TKRG_WR_RD_CTRL_OFFSET)

#define HAL_AN8808_PHY_WORD_SIZE          (4)
#define HAL_AN8808_PHY_EFUSE_PHY_BASE     (5)
#define HAL_AN8808_PHY_EFUSE_SIZE_PER_PHY (4)

#define HAL_AN8808_PHY_EFUSE_DATA5 (0x1000A514)
#define HAL_AN8808_PHY_EFUSE_DATA7 (0x1000A51C)
#define HAL_AN8808_PHY_EFUSE_DATA8 (0x1000A520)

#define HAL_AN8808_PHY_GEAR_NUM (8)

#define HAL_AN8808_PHY_PORT0_3_SEED       (0x1000A5B0)
#define HAL_AN8808_PHY_PORT4_7_SEED       (0x1000A5B4)
#define HAL_AN8808_PHY_RANDOM_SEED_LSB    (0x760)
#define HAL_AN8808_PHY_RANDOM_SEED_MSB    (0x761)
#define HAL_AN8808_PHY_MAX_READ_SUM_RETRY (10)

#define MAX_DOWN_SHIFT_TIMEOUT      (10)
#define HAL_AN8808_PHY_MAX_PORT_NUM (8)

#define HAL_AN8808_PHY_HWRST_DEGLITCH_REG    (0x10000074)
#define HAL_AN8808_PHY_HWRST_DEGLITCH_ENABLE (0x7)

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

#define GET_PHY_BUCK_ADDRESS(port, phy_addr)           \
    do                                                 \
    {                                                  \
        phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(0, port); \
        phy_addr = ((phy_addr - 1) & ~0x07) + 1;       \
    } while (0)

#define HAL_AN8808_PHY_GET_PSR_LINK_STS(data, phy_port)                                           \
    ((data & (HAL_AN8808_PHY_PSR_LINK_STS_MASK                                                    \
              << ((phy_port % HAL_AN8808_PHY_PER_PSR_PORTS) * HAL_AN8808_PHY_PER_PSR_OFFSET))) >> \
     ((phy_port % HAL_AN8808_PHY_PER_PSR_PORTS) * HAL_AN8808_PHY_PER_PSR_OFFSET))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_an8808_phy.c");

extern const UI32_T           _hal_an8808_fw_len;
extern const UI8_T            _hal_an8808_fw[];
extern const HAL_PHY_CFG_T    _hal_an8808_longreach[];
extern const UI32_T           _hal_an8808_longreach_size;
extern const HAL_PHY_CFG_T    _hal_an8808_normal[];
extern const UI32_T           _hal_an8808_normal_size;
extern const HAL_PHY_CFG_T    _hal_an8808_pre_init_global[];
extern const UI32_T           _hal_an8808_pre_init_global_size;
extern const HAL_PHY_CFG_T    _hal_an8808_pre_init_port[];
extern const UI32_T           _hal_an8808_pre_init_port_size;

extern const HAL_QSGMII_CFG_T _hal_an8808q_qp_parm[];
extern const UI32_T           _hal_an8808q_qp_size;
extern const HAL_QSGMII_CFG_T _hal_an8808q_pxp_parm[];
extern const UI32_T           _hal_an8808q_pxp_size;

/* STATIC VARIABLE DECLARATIONS */
static const UI8_T _hal_an8808_phy_r50ohm_table[] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, /* 10 */
                                                     127, 127, 127, 127, 127, 127, 127, 126, 122, 117, /* 20 */
                                                     112, 109, 104, 101, 97,  94,  90,  88,  84,  80,  /* 30 */
                                                     78,  74,  72,  68,  66,  64,  61,  58,  56,  53,  /* 40 */
                                                     51,  48,  47,  44,  42,  40,  38,  36,  34,  32,  /* 50 */
                                                     31,  28,  27,  24,  24,  22,  20,  18,  16,  16,  /* 60 */
                                                     14,  12,  11,  9};
/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void
_hal_an8808_phy_getSurgeVal(
    UI8_T  base,
    UI8_T *ptr_val)
{
    UI8_T  index = 0, table_idx = 0;
    UI32_T sz = sizeof(_hal_an8808_phy_r50ohm_table) / sizeof(UI8_T);

    for (index = 0; index < sz; ++index)
    {
        if (_hal_an8808_phy_r50ohm_table[index] == base)
        {
            break;
        }
    }

    if (index < HAL_AN8808_PHY_GEAR_NUM || index >= sz)
    {
        table_idx = 25; /* index of 94 */
    }
    else
    {
        table_idx = index - HAL_AN8808_PHY_GEAR_NUM;
    }

    if (table_idx < sz)
    {
        *ptr_val = _hal_an8808_phy_r50ohm_table[table_idx];
        DIAG_PRINT(HAL_DBG_INFO, "base:%u, index:%u, table_idx:%u, val:%u\n", base, index, table_idx, *ptr_val);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "get surge value failed! base:%u, index:%u, table_idx:%u, val:%u\n", base, index,
                   table_idx, *ptr_val);
    }
}

static UI32_T
_hal_an8808_phy_readBuckPbus(
    UI16_T phy_addr,
    UI32_T pbus_addr)
{
    UI32_T         pbus_data = 0;
    UI16_T         pbus_data_low = 0, pbus_data_high = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x1F, 0x4);
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x15, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x16, (UI32_T)(pbus_addr & 0xffff));
    rv |= hal_mdio_readC22(0, 0, phy_addr, 0x17, &pbus_data_high);
    rv |= hal_mdio_readC22(0, 0, phy_addr, 0x18, &pbus_data_low);
    rv |= hal_mdio_writeC22(0, 0, phy_addr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "phy %u, read pbus %8x is fail !\n", phy_addr, pbus_addr);
    }
    return pbus_data;
}

static void
_hal_an8808_phy_writeBuckPbus(
    UI16_T phy_addr,
    UI32_T pbus_addr,
    UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x1F, 0x4);
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x11, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x12, (UI32_T)(pbus_addr & 0xffff));
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x13, (UI32_T)((pbus_data >> 16) & 0xffff));
    rv |= hal_mdio_writeC22(0, 0, phy_addr, 0x14, (UI32_T)(pbus_data & 0xffff));
    rv |= hal_mdio_writeC22(0, 0, phy_addr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "phy %u, write pbus %8x is fail !\n", phy_addr, pbus_addr);
    }
}

AIR_ERROR_NO_T
_hal_an8808_phy_download(
    UI16_T phy_addr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T        *ptrAddr;
    UI32_T         idx = 0, data = 0, retry = 0;
    UI32_T         sum_result = 0, dmem_addr = 0;

    dmem_addr = HAL_AN8808_DMEM_ADDRESS;
    for (idx = 0; idx <= _hal_an8808_fw_len; idx += 4)
    {
        ptrAddr = (UI32_T *)&_hal_an8808_fw[idx];
        _hal_an8808_phy_writeBuckPbus(phy_addr, dmem_addr, *ptrAddr);
        dmem_addr += 4;
    }
    /* calculate check sum */
    for (idx = 0; idx < _hal_an8808_fw_len; idx++)
    {
        sum_result += (UI32_T)_hal_an8808_fw[idx];
    }

    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_FIRMWARE_LEN_REG, _hal_an8808_fw_len);
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_FIRMWARE_SUM_REG, sum_result);
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_FIRMWARE_SUM_RET_REG, 0);
    /* overwrite a value to enable DMEM firmware, the original value is 0x42 */
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_ACTIVE_REG, 0x000A8808);

    for (retry = 0; retry < HAL_AN8808_PHY_MAX_READ_SUM_RETRY; retry++)
    {
        osal_delayUs(100000);
        data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_FIRMWARE_SUM_RET_REG);
        if ((data != 0) && (data == sum_result))
        {
            break;
        }
    }
    if (HAL_AN8808_PHY_MAX_READ_SUM_RETRY <= retry)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg] an8808 download firmware fail !\n");
        return AIR_E_OP_INCOMPLETE;
    }
    return (rv);
}

static void
_hal_an8808_phy_tkrg_wait_ready(
    UI8_T phyAddr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI8_T          retry = 0x7F;
    UI16_T         reg_data = 0;

    do
    {
        rv = hal_mdio_readC22(0, 0, phyAddr, 0x10, &reg_data);
        if (AIR_E_OK == rv)
        {
            retry--;
        }
    } while (((reg_data & TKRG_PKT_XMT_STA) == 0) && (retry > 0));
}

static void
_hal_an8808_phy_tkrg_write(
    UI8_T  phyAddr,
    UI8_T  chan_addr,
    UI8_T  node_addr,
    UI8_T  data_addr,
    UI32_T wdata)
{
    UI16_T low_data = wdata & 0xffff;
    UI16_T high_data = (wdata >> 16) & 0xffff;
    UI16_T reg_data = 0;

    hal_mdio_writeC22(0, 0, phyAddr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_TrReg);

    reg_data = (data_addr << TKRG_DATA_ADDR_OFFSET) | (node_addr << TKRG_NODE_ADDR_OFFSET) |
               (chan_addr << TKRG_CHANNEL_ADDR_OFFSET) | TKRG_WRITE_CTRL | TKRG_PKT_XMT_STA;

    hal_mdio_writeC22(0, 0, phyAddr, 0x11, low_data);
    hal_mdio_writeC22(0, 0, phyAddr, 0x12, high_data);
    hal_mdio_writeC22(0, 0, phyAddr, 0x10, reg_data);

    hal_mdio_writeC22(0, 0, phyAddr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
}

static UI32_T
_hal_an8808_phy_tkrg_read(
    UI8_T phyAddr,
    UI8_T channel_addr,
    UI8_T node_addr,
    UI8_T data_addr)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;
    UI32_T         rdata = 0;

    hal_mdio_writeC22(0, 0, phyAddr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_TrReg);

    reg_data = (data_addr << TKRG_DATA_ADDR_OFFSET) | (node_addr << TKRG_NODE_ADDR_OFFSET) |
               (channel_addr << TKRG_CHANNEL_ADDR_OFFSET) | TKRG_READ_CTRL | TKRG_PKT_XMT_STA;

    hal_mdio_writeC22(0, 0, phyAddr, 0x10, reg_data);
    _hal_an8808_phy_tkrg_wait_ready(phyAddr);

    rv = hal_mdio_readC22(0, 0, phyAddr, 0x11, &reg_data);
    if (AIR_E_OK == rv)
    {
        rdata = reg_data;
        rv = hal_mdio_readC22(0, 0, phyAddr, 0x12, &reg_data);
        if (AIR_E_OK == rv)
        {
            rdata |= (reg_data << 16);
        }
    }
    hal_mdio_writeC22(0, 0, phyAddr, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    return rdata;
}

static void
_hal_an8808_parm_apply(
    const UI16_T           phy_addr,
    const HAL_QSGMII_CFG_T qsgmii_cfg)
{
    UI32_T data = 0;

    if (0 == qsgmii_cfg.reg_addr)
    {
        osal_delayUs(qsgmii_cfg.reg_data * 1000);
        return;
    }
    data = _hal_an8808_phy_readBuckPbus(phy_addr, qsgmii_cfg.reg_addr);
    DIAG_PRINT(HAL_DBG_INFO, "Read addr=%x , data=%x\n", qsgmii_cfg.reg_addr, (unsigned int)data);
    if ((31 == qsgmii_cfg.data_msb) && (0 == qsgmii_cfg.data_lsb))
    {
        DIAG_PRINT(HAL_DBG_INFO, "write data=%x\n", qsgmii_cfg.reg_data);
        _hal_an8808_phy_writeBuckPbus(phy_addr, qsgmii_cfg.reg_addr, qsgmii_cfg.reg_data);
    }
    else
    {
        data = _hal_an8808_phy_readBuckPbus(phy_addr, qsgmii_cfg.reg_addr);
        data &= ~(BITS(qsgmii_cfg.data_lsb, qsgmii_cfg.data_msb));
        data |= (qsgmii_cfg.reg_data << qsgmii_cfg.data_lsb);
        DIAG_PRINT(HAL_DBG_INFO, "write data=%x\n", (unsigned int)data);
        _hal_an8808_phy_writeBuckPbus(phy_addr, qsgmii_cfg.reg_addr, data);
    }
}

static void
_hal_an8808q_qsgmii_init(
    const UI16_T phy_addr)
{
    HAL_QSGMII_CFG_T qsgmii_cfg;
    UI32_T           count = 0, port = 0;

    for (count = 0; count < _hal_an8808q_qp_size; count++)
    {
        qsgmii_cfg = _hal_an8808q_qp_parm[count];
        _hal_an8808_parm_apply(phy_addr, qsgmii_cfg);
    }
    for (count = 0; count < _hal_an8808q_pxp_size; count++)
    {
        qsgmii_cfg = _hal_an8808q_pxp_parm[count];
        _hal_an8808_parm_apply(phy_addr, qsgmii_cfg);
    }
    /* EEE transparent */
    for (port = 0; port < HAL_AN8808_PHY_MAX_PORT_NUM; port++)
    {
        if (port > 3)
        {
            _hal_an8808_phy_writeBuckPbus(
                phy_addr, HAL_AN8808_PHY_RATE_ADP_P4_CTRL0_REG + ((port - 4) * HAL_AN8808_PHY_RATE_ADP_CTRL0_OFFSET),
                0x70000000);
        }
        else
        {
            _hal_an8808_phy_writeBuckPbus(
                phy_addr, HAL_AN8808_PHY_RATE_ADP_P0_CTRL0_REG + (port * HAL_AN8808_PHY_RATE_ADP_CTRL0_OFFSET),
                0x70000000);
        }
    }
}

static AIR_ERROR_NO_T
_hal_an8808_phy_getRandomSeed(
    const UI32_T unit,
    const UI32_T phy_addr,
    UI32_T      *random_seed)
{
    UI8_T          index = 0;
    UI16_T         backup_dev1e_reg03ch = 0, backup_dev1f_reg015h = 0, backup_reg0 = 0, reg_data = 0, phy_port = 0;
    UI32_T         backup_vga = 0, value = 0, seed = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22(unit, 0, phy_addr, MII_BMCR, &backup_reg0);
    rv = hal_mdio_writeC22(unit, 0, phy_addr, MII_BMCR, 0x140);

    rv = hal_mdio_readC45(unit, 0, phy_addr, MMD_DEV_VSPEC1, 0x3C, &backup_dev1e_reg03ch);
    rv = hal_mdio_readC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x15, &backup_dev1f_reg015h);
    backup_vga = _hal_an8808_phy_tkrg_read(phy_addr, 0x1, 0xf, 0x10);

    rv = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC1, 0x3C, 0x8800);
    phy_port = (phy_addr - 1) & 0x03;
    /* debug port assigned by b13..b15 , every 4 ports for one group */
    rv = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x15, (phy_port << 13) | 0x108b);
    value = backup_vga | 0xF8000;
    _hal_an8808_phy_tkrg_write(phy_addr, 0x1, 0xF, 0x10, value);

    for (index = 0; index < 16; index++)
    {
        rv = hal_mdio_readC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        seed |= ((reg_data & 0x1) << index);
    }

    _hal_an8808_phy_tkrg_write(phy_addr, 0x1, 0xF, 0x10, backup_vga);
    rv = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x15, backup_dev1f_reg015h);
    rv = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC1, 0x3C, backup_dev1e_reg03ch);

    backup_reg0 |= BMCR_PDOWN;
    rv = hal_mdio_writeC22(unit, 0, phy_addr, MII_BMCR, backup_reg0);

    if (AIR_E_OK == rv)
    {
        *random_seed = seed;
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_restartLink(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_an8808_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_DISABLE);
    if (AIR_E_OK == rv)
    {
        rv = hal_an8808_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_ENABLE);
        if (AIR_E_OK == rv)
        {
            rv = hal_an8808_phy_setAutoNego(unit, port, HAL_PHY_AUTO_NEGO_RESTART);
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_checkLinkStatus(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T link_status)
{
    UI16_T         reg_data = 0;
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
        /* phy power down then up if AN is in fault state */
        rv = _hal_an8808_phy_restartLink(unit, port);
        return rv;
    }
#ifdef AIR_EN_SPEED_DOWNSHIFT_ENHANCE
    rv = hal_an8808_phy_getSmartSpeedDown(unit, port, &spd_mode);
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

            reg_data = _hal_an8808_phy_tkrg_read(HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), 0x0, 0xf, 0x1);
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

static AIR_ERROR_NO_T
_hal_an8808_phy_setRandomSeedByGenModule(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         efuse_data1 = 0, efuse_data2 = 0, bus_id = 0, index = 0, seed = 0;
    UI16_T         reg_data = 0, tmp_data = 0, phy_addr = 0;
    BOOL_T         efuse_valid = FALSE;

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    efuse_data1 = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_PORT0_3_SEED);
    efuse_data2 = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_PORT4_7_SEED);

    DIAG_PRINT(HAL_DBG_INFO, "read efuse addr 0x%08x:0x%08x\n", HAL_AN8808_PHY_PORT0_3_SEED, efuse_data1);
    DIAG_PRINT(HAL_DBG_INFO, "read efuse addr 0x%08x:0x%08x\n", HAL_AN8808_PHY_PORT4_7_SEED, efuse_data2);
    if ((0 == efuse_data1) || (0 == efuse_data2) || (efuse_data1 == efuse_data2))
    {
        DIAG_PRINT(HAL_DBG_INFO, "set random_seed_gen module value from reg 0x1e 0x12.\n");
        efuse_valid = FALSE;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "set random_seed_gen module value from efuse.\n");
        efuse_valid = TRUE;
    }

    if (TRUE == efuse_valid)
    {
        reg_data = efuse_data1 & BITS(0, 15);
        rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_LSB, reg_data);
        DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x760 to 0x%04x\n", phy_addr, reg_data);
        if (AIR_E_OK == rv)
        {
            reg_data = ((BITS_OFF_R(efuse_data1, 16, 15)) | BIT(15));
            rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_MSB, reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x761 to 0x%04x\n", phy_addr, reg_data);
        }

        if (AIR_E_OK == rv)
        {
            reg_data = efuse_data2 & BITS(0, 15);
            rv = hal_mdio_writeC45(unit, bus_id, (phy_addr + 4), MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_LSB,
                                   reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x760 to 0x%04x\n", (phy_addr + 4), reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data = ((BITS_OFF_R(efuse_data2, 16, 15)) | BIT(15));
                rv = hal_mdio_writeC45(unit, bus_id, (phy_addr + 4), MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_MSB,
                                       reg_data);
                DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x761 to 0x%04x\n", (phy_addr + 4), reg_data);
            }
        }
    }
    else
    {
        rv = _hal_an8808_phy_getRandomSeed(unit, phy_addr, &seed);
        if (AIR_E_OK == rv)
        {
            for (index = 0; index < 0x4; index++)
            {
                rv |= hal_mdio_readC45(unit, bus_id, (phy_addr + index), MMD_DEV_VSPEC1, 0x12, &tmp_data);
                if (AIR_E_OK == rv)
                {
                    seed += tmp_data;
                }
            }
            DIAG_PRINT(HAL_DBG_INFO, "seed:0x%08x\n", seed);
        }
        if (AIR_E_OK == rv)
        {
            reg_data = (seed & BITS(0, 15));
            rv = hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_LSB, reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x760 to 0x%04x\n", phy_addr, reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data = ((~(seed & BITS(0, 15))) | BIT(15));
                rv =
                    hal_mdio_writeC45(unit, bus_id, phy_addr, MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_MSB, reg_data);
                DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x761 to 0x%04x\n", phy_addr, reg_data);
            }
        }
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8808_phy_getRandomSeed(unit, (phy_addr + 4), &seed);
            if (AIR_E_OK == rv)
            {
                for (index = 0x4; index < 0x8; index++)
                {
                    rv |= hal_mdio_readC45(unit, bus_id, (phy_addr + index), MMD_DEV_VSPEC1, 0x12, &tmp_data);
                    if (AIR_E_OK == rv)
                    {
                        seed += tmp_data;
                    }
                }
                DIAG_PRINT(HAL_DBG_INFO, "seed:0x%08x\n", seed);
            }

            if (AIR_E_OK == rv)
            {
                reg_data = (seed & BITS(0, 15));
                rv = hal_mdio_writeC45(unit, bus_id, (phy_addr + 4), MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_LSB,
                                       reg_data);
                DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x760 to 0x%04x\n", (phy_addr + 4), reg_data);
                if (AIR_E_OK == rv)
                {
                    reg_data = ((~(seed & BITS(0, 15))) | BIT(15));
                    rv = hal_mdio_writeC45(unit, bus_id, (phy_addr + 4), MMD_DEV_VSPEC2, HAL_AN8808_PHY_RANDOM_SEED_MSB,
                                           reg_data);
                    DIAG_PRINT(HAL_DBG_INFO, "set phy addr 0x%02x cl45 0x1f 0x761 to 0x%04x\n", (phy_addr + 4),
                               reg_data);
                }
            }
        }
    }

    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "do random seed by gen module failed!\n");
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
    UI16_T         status = 0, reg_data = 0;

    /* Do a fake read */
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &status);
    if (AIR_E_OK == rv)
    {
        /* Read link and autonegotiation status */
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &status);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(status, BMSR_ANEGCOMPLETE, HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
            SET_PHY_ABILITY(status, BMSR_RFAULT, HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);
            ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            /* 10M link status */
            rv = hal_mdio_readC22ByPort(unit, port, 0x16, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (reg_data & BIT(6))
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
            }
            /* 100M link status */
            rv = hal_mdio_readC22ByPort(unit, port, 0x10, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (reg_data & BIT(12))
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
            }
            /* 1000M link status */
            rv = hal_mdio_readC22ByPort(unit, port, 0x11, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (reg_data & BIT(12))
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
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
        rv = hal_an8808_phy_getAutoNego(unit, port, &auto_nego);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
            {
                rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &lpagb);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &adv);
                    if (AIR_E_OK == rv)
                    {
                        common_adv_gb = lpagb & adv << 2;
                        rv = hal_mdio_readC22ByPort(unit, port, MII_LPA, &lpa);
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &adv);
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
                rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &bmcr);
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
            rv = hal_mdio_readC22ByPort(unit, port, MII_AUX_CTRL_STA, &speed_status);
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

/* FUNCTION NAME:   _hal_an8808_phy_restoreConfigFromEfuse
 * PURPOSE:
 *      Restore AN8808Q PHY config from efuse
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
_hal_an8808_phy_restoreConfigFromEfuse(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         data = 0, phy_port = 0, phy_addr = 0;
    UI32_T         efuse_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    /* i2mpb_a_tbt */
    reg_addr = HAL_AN8808_PHY_EFUSE_DATA7 + (phy_port * 0x10);
    efuse_data = _hal_an8808_phy_readBuckPbus(phy_addr, reg_addr);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x12, &data);
    if (AIR_E_OK == rv)
    {
        data &= ~(BITS(0, 5));
        data |= BITS_OFF_R(efuse_data, 12, 6);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x12, data);
    }

    /* i2mpb_b_tbt */
    if (AIR_E_OK == rv)
    {
        reg_addr = HAL_AN8808_PHY_EFUSE_DATA8 + (phy_port * 0x10);
        efuse_data = _hal_an8808_phy_readBuckPbus(phy_addr, reg_addr);

        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x17, &data);
        if (AIR_E_OK == rv)
        {
            data &= ~(BITS(0, 5));
            data |= BITS_OFF_R(efuse_data, 12, 6);
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x17, data);
        }
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8808_phy_restoreCfg
 * PURPOSE:
 *      Restore AN8808Q PHY config when operation mode from long-reach to normal.
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
_hal_an8808_phy_restoreCfg(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    HAL_PHY_OP_MODE_T mode = HAL_PHY_OP_MODE_LAST;

    rv = hal_an8808_phy_getPhyOpMode(unit, port, &mode);
    if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_LONG_REACH == mode))
    {
        rv = _hal_an8808_phy_restoreConfigFromEfuse(unit, port);
        if (AIR_E_OK == rv)
        {
            rv |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_TrReg);
            rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, 0x1e);
            rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, 0x0);
            rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x8f80);
            rv |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
            DIAG_PRINT(HAL_DBG_INFO, "Apply long-reach restore setting.\n");
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "Not apply long-reach restore setting.\n");
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8808_phy_applyParam
 * PURPOSE:
 *      Apply AN8808Q PHY parameters
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
_hal_an8808_phy_applyParam(
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

static AIR_ERROR_NO_T
_hal_an8808_phy_enhanceIotVga(
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
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x1f, 0x52b5);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, 0x66);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, 0x4d2a);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x8fa4);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x1f, 0x0);
    }
    else
    {
        /* default value */
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x1f, 0x52b5);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, 0x3e);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, 0x4d2a);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x8fa4);
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x1f, 0x0);
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_pre_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         count = 0;
    UI16_T         data = 0;
    HAL_PHY_CFG_T  phy_cfg;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x268, &data);
    if (AIR_E_OK == rv)
    {
        if (0x7f1 != data) /* check if it has been initialized before */
        {
            for (count = 0; count < _hal_an8808_pre_init_global_size; count++)
            {
                phy_cfg = _hal_an8808_pre_init_global[count];
                rv = _hal_an8808_phy_applyParam(unit, port, phy_cfg);
                if (AIR_E_OK != rv)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "port %u, an8808 PHY per-port initialized fail !\n", port);
                    break;
                }
            }
        }
    }
    if (AIR_E_OK == rv)
    {
        for (count = 0; count < _hal_an8808_pre_init_port_size; count++)
        {
            phy_cfg = _hal_an8808_pre_init_port[count];
            rv = _hal_an8808_phy_applyParam(unit, port, phy_cfg);
            if (AIR_E_OK != rv)
            {
                DIAG_PRINT(HAL_DBG_ERR, "port %u, an8808 PHY per-port initialized fail !\n", port);
                break;
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8808_phy_setPhyOpMode
 * PURPOSE:
 *      Set AN8808Q PHY operation mode
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
_hal_an8808_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI32_T                count = 0;
    HAL_PHY_CFG_T         phy_cfg;
    HAL_PHY_AN_ADV_T      hal_adv;
    HAL_PHY_ADMIN_STATE_T admin_state = HAL_PHY_ADMIN_STATE_LAST;

    osal_memset(&phy_cfg, 0, sizeof(HAL_PHY_CFG_T));

    rv = hal_an8808_phy_getAdminState(unit, port, &admin_state);
    if ((AIR_E_OK == rv) && (HAL_PHY_ADMIN_STATE_DISABLE == admin_state))
    {
        rv = hal_an8808_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_ENABLE);
    }
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_OP_MODE_NORMAL == mode)
        {
            rv = _hal_an8808_phy_restoreCfg(unit, port);
            if (AIR_E_OK == rv)
            {
                for (count = 0; count < _hal_an8808_normal_size; count++)
                {
                    if (AIR_E_OK == rv)
                    {
                        phy_cfg = _hal_an8808_normal[count];
                        rv = _hal_an8808_phy_applyParam(unit, port, phy_cfg);
                    }
                }
            }
        }
        else if (HAL_PHY_OP_MODE_LONG_REACH == mode)
        {
            rv = hal_an8808_phy_getLocalAdvAbility(unit, port, &hal_adv);
            if (AIR_E_OK == rv)
            {
                if (hal_adv.flags & HAL_PHY_AN_ADV_FLAGS_EEE)
                {
                    rv = AIR_E_OP_INVALID;
                }
                else
                {
                    for (count = 0; count < _hal_an8808_longreach_size; count++)
                    {
                        if (AIR_E_OK == rv)
                        {
                            phy_cfg = _hal_an8808_longreach[count];
                            rv = _hal_an8808_phy_applyParam(unit, port, phy_cfg);
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
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8808_phy_enhanceIotVga(unit, port, mode);
    }
    if (HAL_PHY_ADMIN_STATE_DISABLE == admin_state)
    {
        rv |= hal_an8808_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_DISABLE);
    }
    return rv;
}

/* FUNCTION NAME:   _hal_an8808_phy_setSurgeProtect
 * PURPOSE:
 *      Set AN8808Q PHY surge protection
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
_hal_an8808_phy_setSurgeProtect(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T surge_mode;
    UI32_T          reg_addr = 0, efuse_data = 0;
    UI32_T          efuse_data_raw[4] = {}, raw = 0;
    UI16_T          reg_data = 0, phy_port = 0, phy_addr = 0;
    UI8_T           table_val = 0;

    osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    for (raw = 0; raw < HAL_AN8808_PHY_EFUSE_SIZE_PER_PHY; raw++)
    {
        reg_addr = (HAL_AN8808_PHY_EFUSE_DATA5 +
                    (HAL_AN8808_PHY_EFUSE_SIZE_PER_PHY * (raw + (HAL_AN8808_PHY_WORD_SIZE * phy_port))));
        efuse_data = _hal_an8808_phy_readBuckPbus(phy_addr, reg_addr);
        efuse_data_raw[raw] = efuse_data;
        DIAG_PRINT(HAL_DBG_INFO, "Read efuse addr:0x%08X, data:0x%08X, efuse_data_raw[%u]:0x%08X\n", reg_addr,
                   efuse_data, raw, efuse_data_raw[raw]);
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, &reg_data);
    DIAG_PRINT(HAL_DBG_INFO, "[Port %u]Read CL45 dev 0x%02X addr:0x%04X, data:0x%04X\n", port, MMD_DEV_VSPEC1, 0x174,
               reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8808 surge protection mode - 0R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[0], 24, 7);
            reg_data |= ((efuse_data << 8) | BIT(15));
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[1], 24, 7);
            reg_data |= (efuse_data | BIT(7));
        }
        else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: an8808 surge protection mode - 5R\n");
            /* cr_r50ohm_rsel_tx_a */
            reg_data &= ~(BITS_RANGE(8, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[0], 24, 7);
            _hal_an8808_phy_getSurgeVal(efuse_data, &table_val);
            reg_data |= ((table_val << 8) | BIT(15));
            /* cr_r50ohm_rsel_tx_b */
            reg_data &= ~(BITS_RANGE(0, 7));
            efuse_data = BITS_OFF_R(efuse_data_raw[1], 24, 7);
            _hal_an8808_phy_getSurgeVal(efuse_data, &table_val);
            reg_data |= (table_val | BIT(7));
        }
        else
        {
            DIAG_PRINT(HAL_DBG_INFO, "port %u, an8808 surge protection mode %u is invalid!\n", port, surge_mode.value);
            return AIR_E_BAD_PARAMETER;
        }
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x174, reg_data);
        DIAG_PRINT(HAL_DBG_INFO, "[Port %u]Write CL45 dev 0x%02X addr:0x%04X, data:0x%04X\n", port, MMD_DEV_VSPEC1,
                   0x174, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, &reg_data);
        DIAG_PRINT(HAL_DBG_INFO, "[Port %u]Read CL45 dev 0x%02X addr:0x%04X, data:0x%04X\n", port, MMD_DEV_VSPEC1,
                   0x175, reg_data);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
            {
                /* cr_r50ohm_rsel_tx_c */
                reg_data &= ~(BITS_RANGE(8, 7));
                efuse_data = BITS_OFF_R(efuse_data_raw[2], 24, 7);
                reg_data |= ((efuse_data << 8) | BIT(15));
                /* cr_r50ohm_rsel_tx_d */
                reg_data &= ~(BITS_RANGE(0, 7));
                efuse_data = BITS_OFF_R(efuse_data_raw[3], 24, 7);
                reg_data |= (efuse_data | BIT(7));
            }
            else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
            {
                /* cr_r50ohm_rsel_tx_c */
                reg_data &= ~(BITS_RANGE(8, 7));
                efuse_data = BITS_OFF_R(efuse_data_raw[2], 24, 7);
                _hal_an8808_phy_getSurgeVal(efuse_data, &table_val);
                reg_data |= ((table_val << 8) | BIT(15));
                /* cr_r50ohm_rsel_tx_d */
                reg_data &= ~(BITS_RANGE(0, 7));
                efuse_data = BITS_OFF_R(efuse_data_raw[3], 24, 7);
                _hal_an8808_phy_getSurgeVal(efuse_data, &table_val);
                reg_data |= (table_val | BIT(7));
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, "port %u, an8808 surge protection mode %u is invalid!\n", port,
                           surge_mode.value);
                return AIR_E_BAD_PARAMETER;
            }
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x175, reg_data);
            DIAG_PRINT(HAL_DBG_INFO, "[Port %u]Write CL45 dev 0x%02X addr:0x%04X, data:0x%04X\n", port, MMD_DEV_VSPEC1,
                       0x175, reg_data);
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_ledConfig(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_behavior;
    UI16_T          led_id = 0, led_config = 0, led_count = 0, data = 0, phy_addr = 0;
    UI16_T          link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    UI16_T          on_rate_addr = 0, blk_rate_addr = 0;
    UI32_T          value32 = 0;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = HAL_AN8808_PHY_LED_COUNT;
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    /* LED configuration */
    for (led_id = 0; led_id < led_count; led_id++)
    {
        data |= (1 << led_id);
    }
    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, HAL_AN8808_PHY_LED_CTRL_SELECT, data);
    if (AIR_E_OK == rv)
    {
        for (led_id = 0; led_id < led_count; led_id++)
        {
            if (led_id == 0)
            {
                link_reg_addr = HAL_AN8808_PHY_LED_0_ON_MASK;
                blk_reg_addr = HAL_AN8808_PHY_LED_0_BLK_MASK;
                on_rate_addr = HAL_AN8808_PHY_LED_0_ON_DURATION;
                blk_rate_addr = HAL_AN8808_PHY_LED_0_BLK_DURATION;
            }
            else if (led_id == 1)
            {
                link_reg_addr = HAL_AN8808_PHY_LED_1_ON_MASK;
                blk_reg_addr = HAL_AN8808_PHY_LED_1_BLK_MASK;
                on_rate_addr = HAL_AN8808_PHY_LED_1_ON_DURATION;
                blk_rate_addr = HAL_AN8808_PHY_LED_1_BLK_DURATION;
            }
            else if (led_id == 2)
            {
                link_reg_addr = HAL_AN8808_PHY_LED_2_ON_MASK;
                blk_reg_addr = HAL_AN8808_PHY_LED_2_BLK_MASK;
                on_rate_addr = HAL_AN8808_PHY_LED_2_ON_DURATION;
                blk_rate_addr = HAL_AN8808_PHY_LED_2_BLK_DURATION;
            }
            else
            {
                link_reg_addr = HAL_AN8808_PHY_LED_3_ON_MASK;
                blk_reg_addr = HAL_AN8808_PHY_LED_3_BLK_MASK;
                on_rate_addr = HAL_AN8808_PHY_LED_3_ON_DURATION;
                blk_rate_addr = HAL_AN8808_PHY_LED_3_BLK_DURATION;
            }

            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

            led_behavior.value = 0xFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = led_id;

            hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
            led_config = led_behavior.value;

            link_reg_data = 0;
            blk_reg_data = 0;

            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_LINK_1000, HAL_AN8808_PHY_LED_LINK_1000, link_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_LINK_100, HAL_AN8808_PHY_LED_LINK_100, link_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_LINK_10, HAL_AN8808_PHY_LED_LINK_10, link_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_LINK_FULLDPLX, HAL_AN8808_PHY_LED_LINK_FULLDPLX,
                            link_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_LINK_HALFDPLX, HAL_AN8808_PHY_LED_LINK_HALFDPLX,
                            link_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_HIGH_ACTIVE, HAL_AN8808_PHY_LED_POL_HIGH_ACT,
                            link_reg_data);

            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_TX_1000, HAL_AN8808_PHY_LED_BLINK_TX_1000,
                            blk_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_RX_1000, HAL_AN8808_PHY_LED_BLINK_RX_1000,
                            blk_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_TX_100, HAL_AN8808_PHY_LED_BLINK_TX_100,
                            blk_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_RX_100, HAL_AN8808_PHY_LED_BLINK_RX_100,
                            blk_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_TX_10, HAL_AN8808_PHY_LED_BLINK_TX_10,
                            blk_reg_data);
            GET_PHY_ABILITY(led_config, HAL_AN8808_PHY_LED_BHV_BLINK_RX_10, HAL_AN8808_PHY_LED_BLINK_RX_10,
                            blk_reg_data);

            if (link_reg_data != 0 || blk_reg_data != 0)
            {
                link_reg_data |= HAL_AN8808_PHY_LED_FUNC_ENABLE;
            }

            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, link_reg_addr, link_reg_data);
            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, blk_reg_addr, blk_reg_data);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, on_rate_addr,
                                                 HAL_AN8808_PHY_LED_ON_RATE_DEFAULT);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, blk_rate_addr,
                                                     HAL_AN8808_PHY_LED_BLINK_RATE_DEFAULT);
                    }
                }
            }
        }
    }
    if (AIR_E_OK == rv)
    {
        GET_PHY_BUCK_ADDRESS(port, phy_addr);
        value32 = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_MODE);
        if ((value32 & 0xff) != 0x11)
        {
            /* set wave-gen pattern */
            osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
            led_behavior.value = 0xFFFF;
            led_behavior.param0 = port;
            led_behavior.param1 = 0;
            rv = hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
            if (led_behavior.value & (1 << 11))
            {
                /* high active */
                value32 = (HAL_AN8808_PHY_PATTERN_OFF << 16) | HAL_AN8808_PHY_PATTERN_ON;
            }
            else
            {
                /* low active */
                value32 = (HAL_AN8808_PHY_PATTERN_ON << 16) | HAL_AN8808_PHY_PATTERN_OFF;
            }
            _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_FLASH_PRD_SET2_REG, value32);
            /* SLED clock rate fine tune; refresh 60Hz, clock 2.08Mhz */
            value32 = (16000 << 16) + 12;
            _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_SLED_CTRL1, value32);
            value32 = (led_count << 1) + HAL_AN8808_PHY_SLED_RISING_EDGE + HAL_AN8808_PHY_SLED_OUTPUT_TRANSITION_MODE;
            value32 |= 0xff00; /* enable all ports */
            _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_SLED_CTRL0, value32);

            value32 = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_MODE);
            value32 = (value32 & ~(0xff)) | 0x11;
            _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_MODE, value32);
            value32 = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_HWTRAP_BYPASS);
            value32 |= 0x11;
            _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_HWTRAP_BYPASS, value32);
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_ledInit(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, HAL_AN8808_PHY_LED_CFG_REG, 0);
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8808_phy_ledConfig(unit, port);
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8808_phy_setIoCurrDriving(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T phy_addr = 0, data = 0;

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_REG_IO_CFG0);
    data &= ~HAL_AN8808_PHY_REG_IO_SMI_BITS;
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_REG_IO_CFG0, data);

    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME: hal_an8808_phy_init
 * PURPOSE:
 *      AN8808Q PHY initialization
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
hal_an8808_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         data = 0;
    UI32_T         phy_addr = 0;
    UI16_T         reg_id_msb = 0, reg_id_lsb = 0;

    /* Update force mcr port bitmap */
    AIR_PORT_ADD(HAL_PORT_BMP_FORCE_MCR(unit), port);

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_ACTIVE_REG);
    /* check for download firmware */
    if (data == HAL_AN8808_IDLE_CODE)
    {
        _hal_an8808_phy_download(phy_addr);
        rv = hal_mdio_readC22ByPort(unit, port, MII_PHYSID1, &reg_id_msb);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_PHYSID2, &reg_id_lsb);
            if (AIR_E_OK == rv)
            {
                data = (reg_id_msb << 16) | (reg_id_lsb & BITS_RANGE(MII_PHY_LSB_ID_OFFT, MII_PHY_LSB_ID_LENG));
                if (HAL_PHY_DEVICE_ID_AN8808Q == data)
                {
                    _hal_an8808q_qsgmii_init(phy_addr);
                }
                else if (HAL_PHY_DEVICE_ID_AN8808X == data)
                {
                    /* TODO */
                    rv = AIR_E_NOT_SUPPORT;
                }
                else
                {
                    rv = AIR_E_NOT_SUPPORT;
                }
            }
        }
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8808_phy_setRandomSeedByGenModule(unit, port);
            if (AIR_E_OK == rv)
            {
                /* Enable HW reset de-glitch */
                data = HAL_AN8808_PHY_HWRST_DEGLITCH_ENABLE;
                _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_HWRST_DEGLITCH_REG, data);
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        /* These setting will apply into E2 version */
        rv = _hal_an8808_phy_pre_init(unit, port);
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8808_phy_setSurgeProtect(unit, port);
            if (AIR_E_OK == rv)
            {
                rv = _hal_an8808_phy_setPhyOpMode(unit, port, HAL_PHY_OP_MODE_NORMAL);
                if (AIR_E_OK == rv)
                {
                    rv = _hal_an8808_phy_ledInit(unit, port);
                    if (AIR_E_OK == rv)
                    {
                        rv = _hal_an8808_phy_setIoCurrDriving(unit, port);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_setAdminState
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
hal_an8808_phy_setAdminState(
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
        if ((AIR_E_OK == rv) && (HAL_PHY_ADMIN_STATE_ENABLE == state))
        {
            /* Re-train RX gain */
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x148, 0x200);
            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x148, 0x0);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getAdminState
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
hal_an8808_phy_getAdminState(
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

/* FUNCTION NAME:   hal_an8808_phy_setAutoNego
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
hal_an8808_phy_setAutoNego(
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
    /* workaround for E1 version, enable the 100F/100H/10F/10H AN capability when the port is set to force mode */
    if (HAL_PHY_PORT_DEV_REVISION_ID(unit, port) == HAL_AN8808_PHY_DEVICE_REV_ID_E1)
    {
        if (HAL_PHY_AUTO_NEGO_DISABLE == auto_nego)
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_value);
            if (AIR_E_OK == rv)
            {
                reg_value |= (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);
                rv = hal_mdio_writeC22ByPort(unit, port, MII_ADVERTISE, reg_value);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getAutoNego
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
hal_an8808_phy_getAutoNego(
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

/* FUNCTION NAME:   hal_an8808_phy_setLocalAdvAbility
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
hal_an8808_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T    rv = AIR_E_OK;
    UI16_T            reg_value = 0;
    HAL_PHY_OP_MODE_T phy_mode;

    if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE)
    {
        rv = hal_an8808_phy_getPhyOpMode(unit, port, &phy_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_LONG_REACH == phy_mode))
        {
            rv = AIR_E_OP_INVALID;
        }
    }

    if (AIR_E_OK == rv)
    {
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
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getLocalAdvAbility
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
hal_an8808_phy_getLocalAdvAbility(
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

/* FUNCTION NAME:   hal_an8808_phy_getRemoteAdvAbility
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
hal_an8808_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_value = 0;

    ptr_adv->flags = 0;
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

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_setSpeed
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
hal_an8808_phy_setSpeed(
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

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_value);
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
            rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_value);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getSpeed
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
hal_an8808_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
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

/* FUNCTION NAME:   hal_an8808_phy_setDuplex
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
hal_an8808_phy_setDuplex(
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

/* FUNCTION NAME:   hal_an8808_phy_getDuplex
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
hal_an8808_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
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

/* FUNCTION NAME:   hal_an8808_phy_getLinkStatus
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
hal_an8808_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;
    UI32_T         phy_addr = 0, efifo_data = 0, new_efifo_data = 0;
    UI8_T          phy_id = 0;

    rv = _genphy_read_status(unit, port, ptr_status);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    DIAG_PRINT(HAL_DBG_INFO, " (%u) port[%d] flags=(%x)\n", unit, port, ptr_status->flags);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, &reg_data);
        if (AIR_E_OK == rv)
        {
            if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (HAL_PHY_SPEED_1000M == ptr_status->speed))
            {
                if (HAL_AN8808_PHY_1G_LOW_POWER != reg_data)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_AN8808_PHY_1G_LOW_POWER);
                }
            }
            else if ((ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) &&
                     ((HAL_PHY_SPEED_100M == ptr_status->speed) || (HAL_PHY_SPEED_10M == ptr_status->speed)))
            {
                if (HAL_AN8808_PHY_100M_10M_LOW_POWER != reg_data)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_AN8808_PHY_100M_10M_LOW_POWER);
                }
            }
            else
            {
                if (HAL_AN8808_PHY_UNPLUG_LOW_POWER != reg_data)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x0c6, HAL_AN8808_PHY_UNPLUG_LOW_POWER);
                }
            }
        }
    }
    if (AIR_E_OK == rv)
    {
        /* Update efifo preamble even compensation if the port links up with 10M speed */
        if (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
        {
            GET_PHY_BUCK_ADDRESS(port, phy_addr);
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            efifo_data = _hal_an8808_phy_readBuckPbus(
                phy_addr, HAL_AN8808_PHY_SYS_CONFIG0 + (phy_id * HAL_AN8808_PHY_EFIFO_REG_OFFSET));

            new_efifo_data = efifo_data;

            if (HAL_PHY_SPEED_10M == ptr_status->speed)
            {
                new_efifo_data |= HAL_AN8808_PHY_LS_CFG_PBL_CMPSAT_EN;
            }
            else
            {
                new_efifo_data &= ~(HAL_AN8808_PHY_LS_CFG_PBL_CMPSAT_EN);
            }

            if (new_efifo_data != efifo_data)
            {
                _hal_an8808_phy_writeBuckPbus(
                    phy_addr, HAL_AN8808_PHY_SYS_CONFIG0 + (phy_id * HAL_AN8808_PHY_EFIFO_REG_OFFSET), new_efifo_data);
            }
        }
    }
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8808_phy_checkLinkStatus(unit, port,
                                             (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP ? TRUE : FALSE));
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_setLoopBack
 * PURPOSE:
 *      This API is used to set the loop back configuration for a specific port.
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
hal_an8808_phy_setLoopBack(
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
        rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (TRUE == enable)
                {
                    reg_data |= HAL_AN8808_PHY_LPBK_FAR_END;
                }
                else
                {
                    reg_data &= ~(HAL_AN8808_PHY_LPBK_FAR_END);
                }
                rv = hal_mdio_writeC22ByPort(unit, port, MII_RESV2, reg_data);
            }
        }
        rv |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    }
    else
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
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
            rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get the loop back configuration for a specific port.
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
hal_an8808_phy_getLoopBack(
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
        rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
            if (AIR_E_OK == rv)
            {
                if (reg_data & HAL_AN8808_PHY_LPBK_FAR_END)
                {
                    *ptr_enable = TRUE;
                }
                else
                {
                    *ptr_enable = FALSE;
                }
            }
        }
        rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    }
    else
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
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

/* FUNCTION NAME:   hal_an8808_phy_setSmartSpeedDown
 * PURPOSE:
 *      This API is used to set port smart speed down.
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
hal_an8808_phy_setSmartSpeedDown(
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
    rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data);
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
            rv = hal_mdio_writeC22ByPort(unit, port, 0x14, data);
        }
    }
    rv |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getSmartSpeedDown
 * PURPOSE:
 *      This API is used to get smart speed down.
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
hal_an8808_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data);
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
    }
    rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_testTxCompliance
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
hal_an8808_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI16_T         u16dat = 0, page = 0;

    /* Backup page of CL22 */
    ret |= hal_mdio_readC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, &page);
    ret |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, HAL_CMN_PHY_CL22_PAGE_REG);
    ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, 0x0);
    switch (mode)
    {
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B:
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x8000);
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x100);
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
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x33, 0x177);
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x27b, 0x1177);
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
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x1a3, u16dat);
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
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x1a4, u16dat);
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
            ret |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_AUX_CTRL_STA, u16dat);
            ret |= hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x0);
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
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, u16dat);
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
            {
                ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x14, 0x4444);
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
            {
                ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x143, 0x200);
            }
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1040);
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D:
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x8700);
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1040);
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
                ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, u16dat);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE:
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x2100);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                u16dat = 0x5010;
            }
            else
            {
                u16dat = 0x5018;
            }
            ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x0);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x23, 0x882);
            }
            else
            {
                ret |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x24, 0x882);
            }
            break;
        /* not support 2.5G at 8808 */
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
            ret = AIR_E_NOT_SUPPORT;
            break;
        default:
            ret = AIR_E_BAD_PARAMETER;
            break;
    }
    /* Restore page of CL22 */
    ret |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, page);
    if (AIR_E_OK != ret)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Port %u test tx compliance mode %u failed! ret=%u\n", port, mode, ret);
    }
    return ret;
}

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedCtrlMode
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
hal_an8808_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0, src_ctrl = 0, phy_port = 0, phy_addr = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    if (0 == led_id)
    {
        src_ctrl = HAL_AN8808_SLED_PER_PORT_LED_BIT0 << (phy_port * HAL_AN8808_SLED_PER_PORT_LEDS);
    }
    else if (1 == led_id)
    {
        src_ctrl = HAL_AN8808_SLED_PER_PORT_LED_BIT1 << (phy_port * HAL_AN8808_SLED_PER_PORT_LEDS);
    }
    else if (2 == led_id)
    {
        src_ctrl = HAL_AN8808_SLED_PER_PORT_LED_BIT2 << (phy_port * HAL_AN8808_SLED_PER_PORT_LEDS);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_SLED_SRC_SEL_REG);
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
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_SLED_SRC_SEL_REG, reg_data);

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedCtrlMode
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
hal_an8808_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          src_ctrl = 0, phy_port = 0, phy_addr = 0;
    UI32_T          reg_data = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    if (0 == led_id)
    {
        src_ctrl = HAL_AN8808_SLED_PER_PORT_LED_BIT0 << (phy_port * HAL_AN8808_SLED_PER_PORT_LEDS);
    }
    else if (1 == led_id)
    {
        src_ctrl = HAL_AN8808_SLED_PER_PORT_LED_BIT1 << (phy_port * HAL_AN8808_SLED_PER_PORT_LEDS);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_SLED_SRC_SEL_REG);

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

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedForceState
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
hal_an8808_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0, flash_id = 0, phy_port = 0, reg_addr = 0, shift = 0, phy_addr = 0;
    UI32_T          mask = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    if (HAL_PHY_LED_STATE_FORCE_PATT == state)
    {
        if (0 == led_id)
        {
            flash_id = HAL_AN8808_SLED_FLASH_MAP_ID_PATT_6;
        }
        else if (1 == led_id)
        {
            flash_id = HAL_AN8808_SLED_FLASH_MAP_ID_PATT_7;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (HAL_PHY_LED_STATE_OFF == state)
    {
        flash_id = HAL_AN8808_SLED_FLASH_MAP_ID_OFF;
    }
    else if (HAL_PHY_LED_STATE_ON == state)
    {
        flash_id = HAL_AN8808_SLED_FLASH_MAP_ID_ON;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    reg_addr = HAL_AN8808_SLED_FLASH_SET_CTR0_REG + (phy_port >> 1) * 4;
    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, reg_addr);

    shift = led_id * HAL_AN8808_SLED_FLASH_SET_ID_OFFSET;
    if (phy_port & 0x01)
    {
        shift += HAL_AN8808_SLED_FLASH_SET_ID_PORT_OFFSET; /* every 16 bits for one port */
    }
    mask = HAL_AN8808_SLED_FLASH_SET_ID_MASK << shift;
    reg_data &= ~mask;
    reg_data |= (flash_id << shift);
    _hal_an8808_phy_writeBuckPbus(phy_addr, reg_addr, reg_data);

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedForceState
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
hal_an8808_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0, phy_port = 0, reg_addr = 0, shift = 0, phy_addr = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
    reg_addr = HAL_AN8808_SLED_FLASH_SET_CTR0_REG + (phy_port >> 1) * 4;
    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, reg_addr);

    shift = led_id * HAL_AN8808_SLED_FLASH_SET_ID_OFFSET;
    if (phy_port & 0x01)
    {
        shift += HAL_AN8808_SLED_FLASH_SET_ID_PORT_OFFSET; /* every 16 bits for one port */
    }
    reg_data = reg_data >> shift;
    reg_data &= HAL_AN8808_SLED_FLASH_SET_ID_MASK;

    if ((HAL_AN8808_SLED_FLASH_MAP_ID_PATT_6 == reg_data) || (HAL_AN8808_SLED_FLASH_MAP_ID_PATT_7 == reg_data))
    {
        *ptr_state = HAL_PHY_LED_STATE_FORCE_PATT;
    }
    else if (HAL_AN8808_SLED_FLASH_MAP_ID_OFF == reg_data)
    {
        *ptr_state = HAL_PHY_LED_STATE_OFF;
    }
    else if (HAL_AN8808_SLED_FLASH_MAP_ID_ON == reg_data)
    {
        *ptr_state = HAL_PHY_LED_STATE_ON;
    }
    else
    {
        return AIR_E_NOT_SUPPORT;
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_setPhyLedForcePattCfg
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
hal_an8808_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0, wg_period = 0, phy_addr = 0;
    UI32_T          wg_cycle = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    if (HAL_PHY_LED_PATT_HZ_HALF == pattern)
    {
        wg_period = HAL_AN8808_PHY_PATTERN_HZ_HALF;
        wg_cycle = HAL_AN8808_PHY_WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_ONE == pattern)
    {
        wg_period = HAL_AN8808_PHY_PATTERN_HZ_ONE;
        wg_cycle = HAL_AN8808_PHY_WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_TWO == pattern)
    {
        wg_period = HAL_AN8808_PHY_PATTERN_HZ_TWO;
        wg_cycle = HAL_AN8808_PHY_WAVE_GEN_CYCLE_MAX;
    }
    else if (HAL_PHY_LED_PATT_HZ_ZERO == pattern)
    {
        wg_period = HAL_AN8808_PHY_PATTERN_HZ_ZERO;
        wg_cycle = HAL_AN8808_PHY_WAVE_GEN_CYCLE_MIN;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_FLASH_PRD_SET3_REG);

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
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_FLASH_PRD_SET3_REG, reg_data);

    /* set wave-gen cycle */
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_CYCLE_CFG_VALUE1_REG);
    if (0 == led_id)
    {
        reg_data = (reg_data & 0xff00ffff) | (wg_cycle << 16);
    }
    else if (1 == led_id)
    {
        reg_data = (reg_data & 0x00ffffff) | (wg_cycle << 24);
    }
    _hal_an8808_phy_writeBuckPbus(phy_addr, HAL_AN8808_PHY_CYCLE_CFG_VALUE1_REG, reg_data);
    return rv;
}

/* FUNCTION NAME:   hal_an8808_phy_getPhyLedForcePattCfg
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
hal_an8808_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI32_T          reg_data = 0, phy_addr = 0;
    AIR_CFG_VALUE_T led_count;

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    HAL_CHECK_PARAM((led_id >= led_count.value), AIR_E_BAD_PARAMETER);

    GET_PHY_BUCK_ADDRESS(port, phy_addr);
    reg_data = _hal_an8808_phy_readBuckPbus(phy_addr, HAL_AN8808_PHY_GPIO_FLASH_PRD_SET3_REG);
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

    if (HAL_AN8808_PHY_PATTERN_HZ_HALF == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_HALF;
    }
    else if (HAL_AN8808_PHY_PATTERN_HZ_ONE == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_ONE;
    }
    else if (HAL_AN8808_PHY_PATTERN_HZ_TWO == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_TWO;
    }
    else if (HAL_AN8808_PHY_PATTERN_HZ_ZERO == reg_data)
    {
        *ptr_pattern = HAL_PHY_LED_PATT_HZ_ZERO;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    return rv;
}

/* FUNCTION NAME: hal_an8808_phy_triggerCableTest
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
hal_an8808_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_NOT_SUPPORT;
    ret = hal_cmn_phy_triggerCableTest(unit, port, test_pair, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_getCableTestRawData
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
hal_an8808_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_NOT_SUPPORT;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_setPhyLedGlbCfg
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
hal_an8808_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI16_T         phy_addr = 0;

    phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    ret = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC1, 0x3e8, 0x03);
    if (AIR_E_OK == ret)
    {
        if (0 == ptr_glb_cfg->flags)
        {
            ret = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x720, 0xff);
        }
        else
        {
            ret = hal_mdio_writeC45(unit, 0, phy_addr, MMD_DEV_VSPEC2, 0x720, 0x0);
        }
    }
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_getPhyLedGlbCfg
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
hal_an8808_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_setPhyLedBlkEvent
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
hal_an8808_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_getPhyLedBlkEvent
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
hal_an8808_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_setPhyLedDuration
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
hal_an8808_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_getPhyLedDuration
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
hal_an8808_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_an8808_phy_setPhyOpMode
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
hal_an8808_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    rv = _hal_an8808_phy_setPhyOpMode(unit, port, mode);
    return rv;
}

/* FUNCTION NAME: hal_an8808_phy_getPhyOpMode
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
hal_an8808_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         reg_data = 0;

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3d, &reg_data);
    if (AIR_E_OK == rv)
    {
        switch (reg_data)
        {
            case HAL_AN8808_PHY_LONG_REACH:
                *ptr_mode = HAL_PHY_OP_MODE_LONG_REACH;
                break;
            case HAL_AN8808_PHY_NORMAL:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
            default:
                *ptr_mode = HAL_PHY_OP_MODE_NORMAL;
                break;
        }
    }
    return rv;
}

/* FUNCTION NAME: hal_an8808_phy_dumpPhyPara
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
hal_an8808_phy_dumpPhyPara(
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
    rv = hal_an8808_phy_getPhyOpMode(unit, port, &phy_mode);
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
    rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
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
    rv = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_Reg);
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

/* FUNCTION NAME: hal_an8808_phy_dumpDebugInfo
 * PURPOSE:
 *      Dump port debug information.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_an8808_phy_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         reg_data = 0, speed_msb = 0, speed_lsb = 0;

    osal_printf("[External status] ========================================================\n");
    rc = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_BMCR, reg_data);
        osal_printf("  loopback = %u            auto_nego = %u           admin_state = %u\n",
                    BITS_OFF_R(reg_data, 14, 1), BITS_OFF_R(reg_data, 12, 1), BITS_OFF_R(reg_data, 11, 1));
        speed_msb = BITS_OFF_R(reg_data, 6, 1);
        speed_lsb = BITS_OFF_R(reg_data, 13, 1);
        if ((0 == speed_msb) && (0 == speed_lsb))
        {
            osal_printf("  force_speed = 10m       force_duplex = %u\n", BITS_OFF_R(reg_data, 8, 1));
        }
        else if ((0 == speed_msb) && (1 == speed_lsb))
        {
            osal_printf("  force_speed = 100m      force_duplex = %u\n", BITS_OFF_R(reg_data, 8, 1));
        }
        else if ((1 == speed_msb) && (0 == speed_lsb))
        {
            osal_printf("  force_speed = 1000m     force_duplex = %u\n", BITS_OFF_R(reg_data, 8, 1));
        }
        else
        {
            osal_printf("  force_speed = unknown   force_duplex = %u\n", BITS_OFF_R(reg_data, 8, 1));
        }
    }
    rc = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_BMSR, reg_data);
        osal_printf("  an_done = %u             remote_fault = %u\n", BITS_OFF_R(reg_data, 5, 1),
                    BITS_OFF_R(reg_data, 4, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_ADVERTISE, reg_data);
        osal_printf("  asym_pause = %u          pause = %u               100m_full = %u\n", BITS_OFF_R(reg_data, 11, 1),
                    BITS_OFF_R(reg_data, 10, 1), BITS_OFF_R(reg_data, 8, 1));
        osal_printf("  100m_half = %u           10m_full = %u            10m_half = %u\n", BITS_OFF_R(reg_data, 7, 1),
                    BITS_OFF_R(reg_data, 6, 1), BITS_OFF_R(reg_data, 5, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, MII_LPA, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_LPA, reg_data);
        osal_printf("  asym_pause = %u          pause = %u               100m_full = %u\n", BITS_OFF_R(reg_data, 11, 1),
                    BITS_OFF_R(reg_data, 10, 1), BITS_OFF_R(reg_data, 8, 1));
        osal_printf("  100m_half = %u           10m_full = %u            10m_half = %u\n", BITS_OFF_R(reg_data, 7, 1),
                    BITS_OFF_R(reg_data, 6, 1), BITS_OFF_R(reg_data, 5, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_CTRL1000, reg_data);
        osal_printf("  master/slave_config = %u master/slave_value = %u  1000m_full = %u\n",
                    BITS_OFF_R(reg_data, 12, 1), BITS_OFF_R(reg_data, 11, 1), BITS_OFF_R(reg_data, 9, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    MII_STAT1000, reg_data);
        osal_printf("  master/slave_fault = %u  1000m_full = %u\n", BITS_OFF_R(reg_data, 15, 1),
                    BITS_OFF_R(reg_data, 11, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, 0x10, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    0x10, reg_data);
        osal_printf("  100m_link_status = %u\n", BITS_OFF_R(reg_data, 12, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, 0x11, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    0x11, reg_data);
        osal_printf("  1000m_link_status = %u\n", BITS_OFF_R(reg_data, 12, 1));
    }
    rc = hal_mdio_readC22ByPort(unit, port, 0x16, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                    0x16, reg_data);
        osal_printf("  10m_link_status = %u\n", BITS_OFF_R(reg_data, 6, 1));
    }

    rc = hal_mdio_writeC22ByPort(unit, port, HAL_AN8808_PHY_MII_PAGE_SELECT, HAL_AN8808_PHY_CL22_Page_ExtReg);
    if (AIR_E_OK == rc)
    {
        rc = hal_mdio_readC22ByPort(unit, port, 0x19, &reg_data);
        if (AIR_E_OK == rc)
        {
            osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                        0x19, reg_data);
            osal_printf("  lpi_rx_state = %u        lpi_tx_state = %u\n", BITS_OFF_R(reg_data, 12, 3),
                        BITS_OFF_R(reg_data, 8, 3));
        }
        rc = hal_mdio_readC22ByPort(unit, port, 0x1a, &reg_data);
        if (AIR_E_OK == rc)
        {
            osal_printf(" cl22 phy_addr/reg_addr/data = 0x%02x/0x%02x/0x%04x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                        0x1a, reg_data);
            osal_printf("  far_end_loopback = %u\n", BITS_OFF_R(reg_data, 15, 1));
        }
    }

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl45 phy_addr/dev/reg_addr/data = 0x%02x/0x%02x/0x%02x/0x%04x\n",
                    HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), MMD_DEV_ANEG, MMD_EEEAR, reg_data);
        osal_printf("  10g_kr_eee = %u          10g_kx4_eee = %u         1g_kx_eee = %u\n", BITS_OFF_R(reg_data, 6, 1),
                    BITS_OFF_R(reg_data, 5, 1), BITS_OFF_R(reg_data, 4, 1));
        osal_printf("  10g_t_eee = %u           1g_t_eee = %u            100m_tx_eee = %u\n",
                    BITS_OFF_R(reg_data, 3, 1), BITS_OFF_R(reg_data, 2, 1), BITS_OFF_R(reg_data, 1, 1));
    }
    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl45 phy_addr/dev/reg_addr/data = 0x%02x/0x%02x/0x%02x/0x%04x\n",
                    HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), MMD_DEV_ANEG, MMD_EEELPAR, reg_data);
        osal_printf("  10g_kr_eee = %u          10g_kx4_eee = %u         1g_kx_eee = %u\n", BITS_OFF_R(reg_data, 6, 1),
                    BITS_OFF_R(reg_data, 5, 1), BITS_OFF_R(reg_data, 4, 1));
        osal_printf("  10g_t_eee = %u           1g_t_eee = %u            100m_tx_eee = %u\n",
                    BITS_OFF_R(reg_data, 3, 1), BITS_OFF_R(reg_data, 2, 1), BITS_OFF_R(reg_data, 1, 1));
    }
    return rc;
}

const HAL_PHY_DRIVER_T _ext_an8808_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_an8808_phy_init,
    hal_an8808_phy_setAdminState,
    hal_an8808_phy_getAdminState,
    hal_an8808_phy_setAutoNego,
    hal_an8808_phy_getAutoNego,
    hal_an8808_phy_setLocalAdvAbility,
    hal_an8808_phy_getLocalAdvAbility,
    hal_an8808_phy_getRemoteAdvAbility,
    hal_an8808_phy_setSpeed,
    hal_an8808_phy_getSpeed,
    hal_an8808_phy_setDuplex,
    hal_an8808_phy_getDuplex,
    hal_an8808_phy_getLinkStatus,
    hal_an8808_phy_setLoopBack,
    hal_an8808_phy_getLoopBack,
    hal_an8808_phy_setSmartSpeedDown,
    hal_an8808_phy_getSmartSpeedDown,
    NULL,
    NULL,
    hal_an8808_phy_testTxCompliance,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_an8808_phy_setPhyLedCtrlMode,
    hal_an8808_phy_getPhyLedCtrlMode,
    hal_an8808_phy_setPhyLedForceState,
    hal_an8808_phy_getPhyLedForceState,
    hal_an8808_phy_setPhyLedForcePattCfg,
    hal_an8808_phy_getPhyLedForcePattCfg,
    hal_an8808_phy_triggerCableTest,
    hal_an8808_phy_getCableTestRawData,
    hal_an8808_phy_setPhyLedGlbCfg,
    hal_an8808_phy_getPhyLedGlbCfg,
    hal_an8808_phy_setPhyLedBlkEvent,
    hal_an8808_phy_getPhyLedBlkEvent,
    hal_an8808_phy_setPhyLedDuration,
    hal_an8808_phy_getPhyLedDuration,
    hal_an8808_phy_setPhyOpMode,
    hal_an8808_phy_getPhyOpMode,
    hal_an8808_phy_dumpPhyPara,
    NULL,
    NULL,
    hal_an8808_phy_dumpDebugInfo,
};

AIR_ERROR_NO_T
hal_an8808_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_an8808_phy_func_vec;

    return (AIR_E_OK);
}
