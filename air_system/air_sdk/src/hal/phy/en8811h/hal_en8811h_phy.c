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

/* FILE NAME:  hal_en8811h_phy.c
 * PURPOSE:
 *  Implement en8811 phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/en8811h/hal_en8811h_phy.h>

#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_EN8811H_PBUS_PHY_ADDRESS (0x17)
#define HAL_EN8811H_PHY_READY        (0x02)
#define HAL_EN8811H_MAX_RETRY        (40)
#define HAL_EN8811H_LINK_SPEED_2500M (0x10)

#define PHY_MAX_NUMBER (32)

#define PHY_LED_TYPE_ON  (0)
#define PHY_LED_TYPE_BLK (1)

#define HAL_EN8811H_MAX_LED_COUNT (2)
#define PHY_LED_CFG_REG           (EN8811H_LED_3_BLK_MASK)
/* MACRO FUNCTION DECLARATIONS
 */
#define LED_ID_TO_LED_REG_ADDRESS(__led_id__, __led_type__, __led_reg_addr__)                \
    do                                                                                       \
    {                                                                                        \
        ((__led_type__) == PHY_LED_TYPE_ON ? ((__led_reg_addr__) = EN8811H_LED_0_ON_MASK) :  \
                                             ((__led_reg_addr__) = EN8811H_LED_0_BLK_MASK)); \
        __led_reg_addr__ += (EN8811H_LED_RG_OFFSET * __led_id__);                            \
    } while (0)

#define HAL_EN8811H_READ_CL22_RG(__unit__, __port__, __reg__, __val__)                             \
    do                                                                                             \
    {                                                                                              \
        rc = hal_mdio_readC22ByPort(__unit__, __port__, __reg__, __val__);                         \
        if (rc != AIR_E_OK)                                                                        \
        {                                                                                          \
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg] port %u mdio read cl22 %x fail \n", __port__, __reg__); \
            return rc;                                                                             \
        }                                                                                          \
    } while (0)

#define HAL_EN8811H_WRITE_CL22_RG(__unit__, __port__, __reg__, __val__)                             \
    do                                                                                              \
    {                                                                                               \
        rc = hal_mdio_writeC22ByPort(__unit__, __port__, __reg__, __val__);                         \
        if (rc != AIR_E_OK)                                                                         \
        {                                                                                           \
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg] port %u mdio write cl22 %x fail \n", __port__, __reg__); \
            return rc;                                                                              \
        }                                                                                           \
    } while (0)

#define HAL_EN8811H_READ_CL45_RG(__unit__, __port__, __dev__, __reg__, __val__)        \
    do                                                                                 \
    {                                                                                  \
        rc = hal_mdio_readC45ByPort(__unit__, __port__, __dev__, __reg__, __val__);    \
        if (rc != AIR_E_OK)                                                            \
        {                                                                              \
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg] port %u mdio read cl45 fail \n", __port__); \
            return rc;                                                                 \
        }                                                                              \
    } while (0)

#define HAL_EN8811H_WRITE_CL45_RG(__unit__, __port__, __dev__, __reg__, __val__)        \
    do                                                                                  \
    {                                                                                   \
        rc = hal_mdio_writeC45ByPort(__unit__, __port__, __dev__, __reg__, __val__);    \
        if (rc != AIR_E_OK)                                                             \
        {                                                                               \
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg] port %u mdio write cl45 fail \n", __port__); \
            return rc;                                                                  \
        }                                                                               \
    } while (0)

#define HAL_EN8811H_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                  \
    {                                                                   \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));           \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__)); \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_en8811_phy.c");

extern const UI32_T en8811h_fw_dm_size;
extern const UI8_T  en8811h_fw_dm[];
extern const UI32_T en8811h_fw_pm_size;
extern const UI8_T  en8811h_fw_pm[];

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* EN8811H PBUS write function */
static AIR_ERROR_NO_T
_hal_en8811h_pbus_regwr(
    const UI32_T unit,
    const UI32_T pbus_addr,
    const UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22(unit, 0, HAL_EN8811H_PBUS_PHY_ADDRESS, 0x1F, (UI16_T)(pbus_addr >> 6));
    rv |= hal_mdio_writeC22(unit, 0, HAL_EN8811H_PBUS_PHY_ADDRESS, (UI16_T)((pbus_addr >> 2) & 0xf),
                            (UI16_T)(pbus_data & 0xFFFF));
    rv |= hal_mdio_writeC22(unit, 0, HAL_EN8811H_PBUS_PHY_ADDRESS, 0x10, (UI16_T)(pbus_data >> 16));
    return rv;
}

/* EN8811H BUCK write function */
static AIR_ERROR_NO_T
_hal_en8811h_buck_pbus_regwr(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pbus_addr,
    const UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, (UI32_T)(pbus_addr & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x13, (UI32_T)((pbus_data >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x14, (UI32_T)(pbus_data & 0xffff));
    return rv;
}

/* EN8811H BUCK read function */
static AIR_ERROR_NO_T
_hal_en8811h_buck_pbus_regrd(
    UI32_T  unit,
    UI32_T  port,
    UI32_T  pbus_addr,
    UI32_T *ptr_pbus_data)
{
    UI32_T         pbus_data = 0;
    UI16_T         pbus_data_low = 0, pbus_data_high = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x15, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x16, (UI32_T)(pbus_addr & 0xffff));

    rv |= hal_mdio_readC22ByPort(unit, port, 0x17, &pbus_data_high);
    rv |= hal_mdio_readC22ByPort(unit, port, 0x18, &pbus_data_low);

    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    *ptr_pbus_data = pbus_data;
    return rv;
}

static AIR_ERROR_NO_T
_hal_en8811h_forceXBZ(
    UI32_T unit,
    UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1140);

    rv |= _hal_en8811h_buck_pbus_regwr(unit, port, 0x10204, 0x0);
    if (AIR_E_OK == rv)
    {
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800c, 0x8);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800d, 0x0);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800e, 0x1100);
        rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800f, 0x1);
    }
    return rv;
}

static void
_hal_en8811h_token_ring_write(
    const UI32_T unit,
    const UI32_T port,
    TrRG_T       TrRG,
    UI32_T       WrtVal)
{
    UI32_T         TmpVal;
    UI16_T         regVal = 0, u16dat = 0;
    HAL_TR_DATA_T  TR_DATA;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* enable gating of RClk125 */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1E, 0x148, &u16dat);
    HAL_EN8811H_SET_VALUE(u16dat, 1, 9, 1);
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1E, 0x148, u16dat);
    /* change page to 0x52b5 */
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x52B5);

    /* write addr */
    TrRG.TrRGDesc.param.WR_RD_CTRL = 1;
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, TrRG.TrRGDesc.Raw[1]);
    /* read data */
    rv |= hal_mdio_readC22ByPort(unit, port, 0x12, &regVal);
    TR_DATA.data_hi = regVal;
    rv |= hal_mdio_readC22ByPort(unit, port, 0x11, &regVal);
    TR_DATA.data_lo = regVal;
    TmpVal = ((((UI32_T)TR_DATA.data_hi) << 16) + (UI32_T)TR_DATA.data_lo) & ~TrRG.RgMask;

    TrRG.TrRGDesc.param.WR_RD_CTRL = 0;
    WrtVal = (WrtVal << TrRG.TrRGDesc.param.TrRG_LSB) | (TmpVal & ~TrRG.RgMask);
    /* write data */
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, (WrtVal >> 16) & 0x00FF);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, WrtVal & 0xFFFF);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, TrRG.TrRGDesc.Raw[1]);

    /* change page to 0 */
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);

    /* disable gating of RClk125 */
    rv |= hal_mdio_readC45ByPort(unit, port, 0x1E, 0x148, &u16dat);
    HAL_EN8811H_SET_VALUE(u16dat, 0, 0x9, 0x1);
    rv |= hal_mdio_writeC45ByPort(unit, port, 0x1E, 0x148, u16dat);
}

static AIR_ERROR_NO_T
_hal_en8811h_mdio_write_buffer(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T addr,
    const UI32_T arr_size,
    const UI8_T *ptr_buf)
{
    UI16_T         write_data = 0;
    UI32_T         offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x8000);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, (UI16_T)((addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, (UI16_T)(addr & 0xffff));

    for (offset = 0; offset < arr_size; offset += 4)
    {
        write_data = (ptr_buf[offset + 3] << 8) | ptr_buf[offset + 2];
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x13, write_data);

        write_data = (ptr_buf[offset + 1] << 8) | ptr_buf[offset];
        rv |= hal_mdio_writeC22ByPort(unit, port, 0x14, write_data);
    }

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
    return rv;
}

static AIR_ERROR_NO_T
_hal_en8811h_firmware_download(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         reg_value = 0, retry = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= _hal_en8811h_pbus_regwr(unit, 0xcf928, 0x0);
    rv |= _hal_en8811h_buck_pbus_regwr(unit, port, 0x0f0018, 0x0);
    rv |= hal_en8811h_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_DISABLE);
    rv |= _hal_en8811h_mdio_write_buffer(unit, port, 0x00000000, en8811h_fw_dm_size, en8811h_fw_dm);
    rv |= _hal_en8811h_mdio_write_buffer(unit, port, 0x00100000, en8811h_fw_pm_size, en8811h_fw_pm);
    rv |= hal_en8811h_phy_setAdminState(unit, port, HAL_PHY_ADMIN_STATE_ENABLE);
    rv |= _hal_en8811h_buck_pbus_regwr(unit, port, 0x0f0018, 0x01);

    for (retry = 0; retry < HAL_EN8811H_MAX_RETRY; retry++)
    {
        osal_delayUs(100000);
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x8009, &reg_value);
        if (HAL_EN8811H_PHY_READY == reg_value)
        {
            break;
        }
    }

    if (HAL_EN8811H_MAX_RETRY == retry)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: en8811 port %d FIRMWARE download fail\n", port);
        return AIR_E_NOT_INITED;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_en8811h_led_setting(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_behavior;
    UI16_T          led_id = 0, led_config = 0, led_count = 0;
    UI16_T          link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = EN8811H_LED_COUNT;
    rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    for (led_id = 0; led_id < led_count; led_id++)
    {
        if (led_id == 0)
        {
            link_reg_addr = EN8811H_LED_0_ON_MASK;
            blk_reg_addr = EN8811H_LED_0_BLK_MASK;
        }
        else if (led_id == 1)
        {
            link_reg_addr = EN8811H_LED_1_ON_MASK;
            blk_reg_addr = EN8811H_LED_1_BLK_MASK;
        }
        else if (led_id == 2)
        {
            link_reg_addr = EN8811H_LED_2_ON_MASK;
            blk_reg_addr = EN8811H_LED_2_BLK_MASK;
        }
        else
        {
            link_reg_addr = EN8811H_LED_3_ON_MASK;
            blk_reg_addr = EN8811H_LED_3_BLK_MASK;
        }

        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

        led_behavior.value = 0xFFFF;
        led_behavior.param0 = port;
        led_behavior.param1 = led_id;

        rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

        led_config = led_behavior.value;

        link_reg_data = 0;
        blk_reg_data = 0;

        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_1000, EN8811H_LED_LINK_1000, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_100, EN8811H_LED_LINK_100, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_10, EN8811H_LED_LINK_10, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_FULLDPLX, EN8811H_LED_LINK_FULLDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_HALFDPLX, EN8811H_LED_LINK_HALFDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_HIGH_ACTIVE, EN8811H_LED_POL_HIGH_ACT, link_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_2500, EN8811H_LED_LINK_2500, link_reg_data);

        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_1000, EN8811H_LED_BLINK_TX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_1000, EN8811H_LED_BLINK_RX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_100, EN8811H_LED_BLINK_TX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_100, EN8811H_LED_BLINK_RX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_10, EN8811H_LED_BLINK_TX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_10, EN8811H_LED_BLINK_RX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_2500, EN8811H_LED_BLINK_TX_2500, blk_reg_data);
        GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_2500, EN8811H_LED_BLINK_RX_2500, blk_reg_data);

        if (link_reg_data != 0 || blk_reg_data != 0)
        {
            link_reg_data |= EN8811H_LED_FUNC_ENABLE;
        }

        DIAG_PRINT(HAL_DBG_INFO, "port %u, link_reg_addr 0x%02X, link_reg_data 0x%04X\n", port, link_reg_addr,
                   link_reg_data);
        DIAG_PRINT(HAL_DBG_INFO, "port %u, blk_reg_addr 0x%02X, blk_reg_data 0x%04X\n", port, blk_reg_addr,
                   blk_reg_data);

        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
    }

    return rv;
}

static void
_hal_en8811h_cable_pair_swap(
    HAL_PHY_CABLE_TEST_PAIR_T  pair0,
    HAL_PHY_CABLE_TEST_PAIR_T  pair1,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_cable)
{
    HAL_PHY_CABLE_STATUS_T tmp_status;
    UI32_T                 tmp_length;

    tmp_status = ptr_cable->status[pair0];
    tmp_length = ptr_cable->length[pair0];
    ptr_cable->status[pair0] = ptr_cable->status[pair1];
    ptr_cable->length[pair0] = ptr_cable->length[pair1];
    ptr_cable->status[pair1] = tmp_status;
    ptr_cable->length[pair1] = tmp_length;
}

static AIR_ERROR_NO_T
_hal_en8811h_waitHostCommandReady(
    UI32_T unit,
    UI32_T port,
    UI32_T wait_ms)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16dat = 0;
    UI8_T          retry = 0;

    do
    {
        osal_sleepTask(wait_ms);
        rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_9, &u16dat);
        retry++;
    } while ((HAL_EN8811H_PHY_READY != u16dat) && (retry < HAL_EN8811H_MAX_RETRY));

    if (retry >= HAL_EN8811H_MAX_RETRY)
    {
        return AIR_E_TIMEOUT;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_reset_md32_fw(
    UI32_T unit,
    UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16dat = 0;

    HAL_EN8811H_WRITE_CL22_RG(unit, port, EN8811H_PHY_MII_PAGE_SELECT,
                              EN8811H_PHY_CL22_PAGE_REG); /* CL22 0x1f = 0x0 page 0 */
    HAL_EN8811H_READ_CL22_RG(unit, port, MII_BMCR, &u16dat);
    u16dat |= BMCR_PDOWN;
    HAL_EN8811H_WRITE_CL22_RG(unit, port, MII_BMCR, u16dat);
    u16dat &= ~(BMCR_PDOWN);
    HAL_EN8811H_WRITE_CL22_RG(unit, port, MII_BMCR, u16dat);
    osal_sleepTask(1000); /* 1000ms */

    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_Cable_diag_normal_1(
    UI32_T unit,
    UI32_T port,
    UI16_T cable_step_control_12,
    UI16_T cable_step_control_13)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16dat = 0, link_status = 0;

    HAL_EN8811H_WRITE_CL22_RG(unit, port, EN8811H_PHY_MII_PAGE_SELECT,
                              EN8811H_PHY_CL22_PAGE_REG); /* CL22 0x1f = 0x0 page 0 */
    /* read cl22 0x11 bit 12 check link status */
    HAL_EN8811H_READ_CL22_RG(unit, port, MII_LINKSTATUS, &u16dat); /* CL22 0x11, bit 12 LinkStatus1000_OK */
    link_status = BITS_OFF_R(u16dat, 12, 0x1);

    if (!link_status)
    {
        /* if link down, write Cl22 0x0 bit 11 1->0, and write chagne page 1 write ExtReg 0x18 = 0x8000 */
        /* wait host comman 0x8009 == 2 */

        HAL_EN8811H_READ_CL22_RG(unit, port, MII_BMCR, &u16dat);
        u16dat |= BMCR_PDOWN;
        HAL_EN8811H_WRITE_CL22_RG(unit, port, MII_BMCR, u16dat);
        u16dat &= ~(BMCR_PDOWN);
        HAL_EN8811H_WRITE_CL22_RG(unit, port, MII_BMCR, u16dat);
        osal_sleepTask(500); /* 500ms */

        HAL_EN8811H_WRITE_CL22_RG(unit, port, EN8811H_PHY_MII_PAGE_SELECT,
                                  EN8811H_PHY_CL22_PAGE_EXTREG);                             /* CL22 0x1f = 0x1 page 1*/
        HAL_EN8811H_WRITE_CL22_RG(unit, port, EN8811H_PHY_EXT_18, EN8811H_REG_VERIPHY_TRIG); /* CL22 ext18 = 0x8000 */

        rc = _hal_en8811h_waitHostCommandReady(unit, port, 1000);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Over retry times, md32 host cmd not ready\n");
            return rc;
        }
    }
    /* host command 0x800e = 0x1100 */
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_14, 0x1100);
    /* host command 0x800f = 0xd4 */
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_15, 0xd4);

    /* wait host comman 0x8009 == 2 */
    rc = _hal_en8811h_waitHostCommandReady(unit, port, 1000);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Over retry times, md32 host cmd not ready\n");
        return rc;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_SOC_step2_Click(
    UI32_T unit,
    UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         cable_step_ctrl_13, cable_step_ctrl_12;
    UI16_T         dbg_ctrl_12, dbg_ctrl_13;

    cable_step_ctrl_13 = 0;
    cable_step_ctrl_12 = 0;
    rc = _hal_en8811h_Cable_diag_normal_1(unit, port, cable_step_ctrl_12, cable_step_ctrl_13);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "_hal_en8811h_SOC_step2_Click fail %d \n", rc);
        return rc;
    }
    /* debug check */
    HAL_EN8811H_READ_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_12, &dbg_ctrl_12);
    HAL_EN8811H_READ_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_13, &dbg_ctrl_13);
    DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "cable_step_ctrl_12= %x \n", dbg_ctrl_12);
    DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "cable_step_ctrl_13= %x \n", dbg_ctrl_13);
    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_SOC_step4_Click(
    UI32_T                     unit,
    UI32_T                     port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16dat, link_status;
    UI8_T          test_pair;
    UI32_T         cable_info = 0, cable_len, term;

    osal_sleepTask(1000);
    /* host command 0x800e = 0x1100 */
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_14, 0x1100);
    /* host command 0x800f = 0xFA */
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_15, 0xFA);

    rc = _hal_en8811h_waitHostCommandReady(unit, port, 1000);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Over retry times, md32 host cmd not ready\n");
        return rc;
    }

    HAL_EN8811H_WRITE_CL22_RG(unit, port, EN8811H_PHY_MII_PAGE_SELECT,
                              EN8811H_PHY_CL22_PAGE_REG);          /*CL22 0x1f = 0 page 0*/
    HAL_EN8811H_READ_CL22_RG(unit, port, MII_LINKSTATUS, &u16dat); /*CL22 0x11 bit 12 */
    link_status = BITS_OFF_R(u16dat, 12, 0x1);
    /* result */
    for (test_pair = 0; test_pair < 4; test_pair++)
    {
        rc |= _hal_en8811h_buck_pbus_regrd(unit, port, (0x11E0 + test_pair * 4), &cable_info);
        cable_len = cable_info & 0xffff;
        term = cable_info >> 16;
        if (!term)
        {
            DIAG_PRINT(HAL_DBG_ERR, "Pair %d, term=%d, len=%d \n", test_pair, term, cable_len);
        }
        else
        {
            ptr_test_rslt->status[test_pair] = term - 1;
            ptr_test_rslt->length[test_pair] = cable_len;
        }
    }

    if (!link_status)
    {
        _hal_en8811h_cable_pair_swap(HAL_PHY_CABLE_TEST_PAIR_A, HAL_PHY_CABLE_TEST_PAIR_B, ptr_test_rslt);
        _hal_en8811h_cable_pair_swap(HAL_PHY_CABLE_TEST_PAIR_C, HAL_PHY_CABLE_TEST_PAIR_D, ptr_test_rslt);
    }
    rc |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_13, 0);
    DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "_hal_en8811h_SOC_step4_Click \n");
    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_SOC_step1_Click(
    UI32_T                     unit,
    UI32_T                     port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         LinkStatus;
    /* set host command 14, 15*/
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_14, 0x1100); /* 0x1e 0x800e = 0x1100*/
    HAL_EN8811H_WRITE_CL45_RG(unit, port, MMD_DEV_VSPEC1, EN8811H_RG_HOST_CTRL_15,
                              0xD6); /* 0x1e 0x800f = 0xd6 (check link status)*/

    /* wait host command done */
    rc = _hal_en8811h_waitHostCommandReady(unit, port, 1000);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Over retry times, md32 host cmd not ready\n");
        return rc;
    }

    rc = _hal_en8811h_buck_pbus_regrd(unit, port, 0x11FC, &LinkStatus);

    if ((LinkStatus == 1) || (LinkStatus == 3))
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Not Support \n");
        return AIR_E_NOT_SUPPORT;
    }
    else if (LinkStatus == 2)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Link up mode \n");
        /* Link up mode */
    }
    else if (LinkStatus == 4)
    {
        DIAG_PRINT(HAL_DBG_PHY_CABLE_DIAG, "Link down mode \n");
        /* Link down mode */
    }

    if ((LinkStatus == 4) || (LinkStatus == 2))
    {
        rc = _hal_en8811h_SOC_step2_Click(unit, port);
        if (AIR_E_OK != rc)
        {
            _hal_en8811h_reset_md32_fw(unit, port);
            return rc;
        }
        rc = _hal_en8811h_SOC_step4_Click(unit, port, ptr_test_rslt);
        if (AIR_E_OK != rc)
        {
            _hal_en8811h_reset_md32_fw(unit, port);
            return rc;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_en8811h_dump_EC_COEF(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    UI32_T         PMEM_addr = 0;
    UI32_T         PMEM_value = 0;
    I16_T          EC_COEF;
    UI8_T          pair, Test_Cnt;

    DIAG_PRINT_RAW(HAL_DBG_PHY_CABLE_DIAG, "\r\n Cable Diag EC COEF Dump \n");
    for (pair = 0; pair < 4; pair++)
    {
        for (Test_Cnt = 0; Test_Cnt < 240; Test_Cnt++) /* EC_FULL_TAPS = 240*/
        {
            PMEM_addr = ((0x118000 + ((Test_Cnt / 2) * 4)) + (0x200 * pair));

            rv = _hal_en8811h_buck_pbus_regrd(unit, port, PMEM_addr, &PMEM_value);
            if (AIR_E_OK == rv)
            {
                EC_COEF = 0;
                if ((Test_Cnt % 2) == 0)
                {
                    EC_COEF = (I16_T)(PMEM_value & 0xffff);
                }
                else
                {
                    EC_COEF = (I16_T)((PMEM_value >> 16) & 0xffff);
                }
                DIAG_PRINT_RAW(HAL_DBG_PHY_CABLE_DIAG, " EC_COEF=  %d \n", EC_COEF);
            }
            else
            {
                DIAG_PRINT_RAW(HAL_DBG_PHY_CABLE_DIAG, "Fail pair %d, tab=%d, Read pbus %x\n", pair, Test_Cnt,
                               PMEM_addr);
            }
        }
    }
    DIAG_PRINT_RAW(HAL_DBG_PHY_CABLE_DIAG, "\r\n Cable Diag EC COEF Dump Finish \n");
    return rv;
}

static void
_en8811_set_gphy_TrReg(
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
_en8811_get_gphy_TrReg(
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
_hal_en8811_phy_getRandomSeed(
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

    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    backup_vga = _en8811_get_gphy_TrReg(unit, port, 0xafa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x140);
    value = backup_dev1e_reg03ch | 0x8800;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3c, value);

    value = backup_dev1f_reg015h & (~0xF1FF);
    value |= 0x108b;
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x15, value);

    value = backup_vga | 0x001f0000;
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8811_set_gphy_TrReg(unit, port, (value & 0xffff), (value >> 16) & 0xffff, 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

    for (index = 0; index < 32; index++)
    {
        rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x1A, &reg_data);
        reg_data &= 0xff;
        seed |= ((reg_data & 0x1) << index);
    }

    rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1000);

    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_TrReg);
    _en8811_set_gphy_TrReg(unit, port, (backup_vga & 0xffff), ((backup_vga >> 16) & 0xffff), 0x8fa0);
    rv |= hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_Reg);

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
_hal_en8811_phy_calRandomSeed(
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

/* STATIC VARIABLE DECLARATIONS */
/*-------------------------------------
  0xa0300800: ECNC_C8h
/------------------------------------ */
/* [ 1: 0],  2 bit(s) */
/* volatile static TrRG_T _fldRW_tr_reg__EcVarShowtimeGain_ECNC_C8h =
                        { .TrRGDesc.DescVal =0x81900100, .RgMask = 0x00000003}; */
/* [ 3: 2],  2 bit(s) */
volatile static TrRG_T _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h = {.TrRGDesc.DescVal = 0x81900302,
                                                                    .RgMask = 0x0000000C};
/* [15: 4], 12 bit(s) */
volatile static TrRG_T _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h = {.TrRGDesc.DescVal = 0x81900F04,
                                                                    .RgMask = 0x0000FFF0};

/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME:   hal_en8811h_phy_init
 * PURPOSE:
 *      EN8811 PHY initialization
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
hal_en8811h_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI16_T          phy_addr = 0, reg_value = 0;
    UI32_T          reg_data = 0;
    UI32_T          random_seed = 0, seed = 0;
    AIR_CFG_VALUE_T serdes_op_mode;
    AIR_CFG_VALUE_T surge_mode;

    phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    if (phy_addr >= PHY_MAX_NUMBER)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: en8811 %d initialize fail\n", port);
        return AIR_E_BAD_PARAMETER;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x8009, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (HAL_EN8811H_PHY_READY != reg_value)
        {
            /* Mute Tx before loading MD32 FW. */
            rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x1e00d0, 0xf);
            if (AIR_E_OK == rv)
            {
                rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x1e0228, 0xf0);
                if (AIR_E_OK == rv)
                {
                    rv = _hal_en8811h_firmware_download(unit, port);
                }
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        /* led setup */
        rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xcf8b8, &reg_data);
        if (AIR_E_OK == rv)
        {
            reg_data |= 0x30;
            rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xcf8b8, reg_data);

            if (AIR_E_OK == rv)
            {
                rv = _hal_en8811h_led_setting(unit, port);
            }
        }

        if (AIR_E_OK == rv)
        {
            rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xca0f8, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data = (reg_data & 0xfffffffc) | EN8811H_TX_POLARITY_NORMAL | EN8811H_RX_POLARITY_NORMAL;
                rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xca0f8, reg_data);
            }
        }

        if (AIR_E_OK == rv)
        {
            osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
            serdes_op_mode.param0 = port;
            hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

            if (HAL_PHY_SERDES_OP_MODE_SPEED_CAHNGE == serdes_op_mode.value)
            {
                /* set 8811 to mode-2 */
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: en8811 Mode 2\n");
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800c, 0x1);
            }
            else
            {
                /* set 8811 to mode-1 */
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: en8811 Mode 1\n");
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800c, 0x0);
            }
            rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800d, 0x0);
            rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800e, 0x1101);
            rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800f, 0x0002);
        }

        if (AIR_E_OK == rv)
        {
            osal_memset(&surge_mode, 0, sizeof(AIR_CFG_VALUE_T));
            hal_cfg_getValue(unit, AIR_CFG_TYPE_SURGE_PROTECTION_MODE, &surge_mode);
            if (HAL_PHY_SURGE_PROTECTION_MODE_0R == surge_mode.value)
            {
                /* MD32 FW default setting is 0R */
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: en8811 surge protection mode - 0R\n");
            }
            else if (HAL_PHY_SURGE_PROTECTION_MODE_5R == surge_mode.value)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: en8811 surge protection mode - 5R\n");
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800c, 0x0);
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800d, 0x0);
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800e, 0x1100);
                rv |= hal_mdio_writeC45ByPort(unit, port, 0x1e, 0x800f, 0xb0);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, "port %u, en8811 surge protection mode %u is invalid!\n", port,
                           surge_mode.value);
                return AIR_E_BAD_PARAMETER;
            }

            if (AIR_E_OK == rv)
            {
                rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xcf958, &reg_data);
                if (AIR_E_OK == rv)
                {
                    reg_data &= ~(BIT(26));
                    rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xcf958, reg_data);
                    if (AIR_E_OK == rv)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "[Port %d]CKO Output mode - Disabled.\n", port);
                    }
                }
            }

            if (AIR_E_OK == rv)
            {
                rv = _hal_en8811_phy_getRandomSeed(unit, port, &random_seed);
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
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setAdminState
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
hal_en8811h_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_ADMIN_STATE_ENABLE == state)
        {
            reg_data &= ~(BMCR_PDOWN);
        }
        else
        {
            reg_data |= BMCR_PDOWN;
        }

        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getAdminState
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
hal_en8811h_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (reg_data & BMCR_PDOWN)
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

/* FUNCTION NAME:   hal_en8811h_phy_setAutoNego
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
hal_en8811h_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* EN8811h do not support force mode. */
    if (HAL_PHY_AUTO_NEGO_DISABLE == auto_nego)
    {
        return AIR_E_OP_INVALID;
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
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
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getAutoNego
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
hal_en8811h_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (reg_data & BMCR_ANENABLE)
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

/* FUNCTION NAME:   hal_en8811h_phy_setLocalAdvAbility
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
hal_en8811h_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T         reg_data = 0;
    UI16_T         flags = HAL_PHY_AN_ADV_FLAGS_10FUDX | HAL_PHY_AN_ADV_FLAGS_10HFDX | HAL_PHY_AN_ADV_FLAGS_100HFDX;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (ptr_adv->flags & flags)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);

    if (AIR_E_OK == rv)
    {
        SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ADVERTISE_1000FULL, reg_data);
        rv = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ADVERTISE_PAUSE_ASYM, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ADVERTISE_PAUSE_CAP, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100FUDX, ADVERTISE_100FULL, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_100HFDX, ADVERTISE_100HALF, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10FUDX, ADVERTISE_10FULL, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_10HFDX, ADVERTISE_10HALF, reg_data);
            rv = hal_mdio_writeC22ByPort(unit, port, MII_ADVERTISE, reg_data);
        }
    }

    if (AIR_E_OK == rv)
    {
        reg_data = (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) ? (EEE_1000BASE_T | EEE_100BASE_TX) : 0;
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, HAL_PHY_AN_ADV_FLAGS_2500M, MULTIG_ANAR_2500M, reg_data);
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, reg_data);
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getLocalAdvAbility
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
hal_en8811h_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    ptr_adv->flags = 0;

    rv = hal_mdio_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);

    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_data, ADVERTISE_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, ADVERTISE_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, MULTIG_ANAR_2500M, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv->flags);
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getRemoteAdvAbility
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
hal_en8811h_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    ptr_adv->flags = 0;

    /* Read the remote advertisement ability information after AN is complete */
    rv = hal_mdio_readC22ByPort(0, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        /* AN not complete, it would not get remote-adv information */
        if ((reg_data & BMSR_ANEGCOMPLETE))
        {
            rv = hal_mdio_readC22ByPort(unit, port, MII_STAT1000, &reg_data);
            if (AIR_E_OK == rv)
            {
                GET_PHY_ABILITY(reg_data, LPA_1000FULL, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
            }

            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_readC22ByPort(unit, port, MII_LPA, &reg_data);
                if (AIR_E_OK == rv)
                {
                    GET_PHY_ABILITY(reg_data, LPA_10HALF, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
                    GET_PHY_ABILITY(reg_data, LPA_10FULL, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
                    GET_PHY_ABILITY(reg_data, LPA_100HALF, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
                    GET_PHY_ABILITY(reg_data, LPA_100FULL, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
                    GET_PHY_ABILITY(reg_data, LPA_PAUSE_CAP, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
                    GET_PHY_ABILITY(reg_data, LPA_PAUSE_ASYM, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
                }
            }

            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &reg_data);
                if (AIR_E_OK == rv)
                {
                    GET_PHY_ABILITY(reg_data, (EEE_1000BASE_T | EEE_100BASE_TX), HAL_PHY_AN_ADV_FLAGS_EEE,
                                    ptr_adv->flags);
                }
            }

            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_LPAR, &reg_data);
                if (AIR_E_OK == rv)
                {
                    GET_PHY_ABILITY(reg_data, MULTIG_LPAR_2500M, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv->flags);
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setSpeed
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
hal_en8811h_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (speed >= HAL_PHY_SPEED_1000M)
    {
        return AIR_E_OP_INVALID;
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
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
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getSpeed
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
hal_en8811h_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* 2.5G, TODO */

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

/* FUNCTION NAME:   hal_en8811h_phy_setDuplex
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
hal_en8811h_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_DUPLEX_HALF == duplex)
    {
        return AIR_E_BAD_PARAMETER;
    }

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_DUPLEX_FULL == duplex)
        {
            reg_data |= BMCR_FULLDPLX;
        }
        else
        {
            return AIR_E_NOT_SUPPORT;
        }
        rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getDuplex
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
hal_en8811h_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_AUX_CTRL_STA, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & AUX_FDX_STATUS)
        {
            *ptr_duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
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
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getLinkStatus
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
hal_en8811h_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_data);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(reg_data, BMSR_LSTATUS, HAL_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_status->flags);
            SET_PHY_ABILITY(reg_data, BMSR_ANEGCOMPLETE, HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
            SET_PHY_ABILITY(reg_data, BMSR_RFAULT, HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);
        }
    }

    if ((AIR_E_OK == rv) && (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP))
    {
        rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, CL22_Page_Reg);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, 0x1d, &reg_data);
            if (AIR_E_OK == rv)
            {
                switch (reg_data & EN8811H_AUX_CTRL_SPEED_MASK)
                {
                    case EN8811H_AUX_CTRL_SPEED_2500:
                        ptr_status->speed = HAL_PHY_SPEED_2500M;
                        break;
                    case EN8811H_AUX_CTRL_SPEED_1000:
                        ptr_status->speed = HAL_PHY_SPEED_1000M;
                        break;
                    case EN8811H_AUX_CTRL_SPEED_100:
                    default:
                        ptr_status->speed = HAL_PHY_SPEED_100M;
                        break;
                }
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = _hal_en8811_phy_calRandomSeed(unit, port,
                                           (ptr_status->flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP ? TRUE : FALSE));
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setLoopBack
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
hal_en8811h_phy_setLoopBack(
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
        rv = hal_mdio_readC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, &page);
        if (AIR_E_OK == rv)
        {
            /* Swtich to page 1 */
            reg_data = 1;
            rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, reg_data);
            if (AIR_E_OK == rv)
            {
                /* Read data from ExtReg1A */
                rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
                if (AIR_E_OK == rv)
                {
                    if (TRUE == enable)
                    {
                        reg_data |= LPBK_FAR_END;
                    }
                    else
                    {
                        reg_data &= ~(LPBK_FAR_END);
                    }

                    rv = hal_mdio_writeC22ByPort(unit, port, MII_RESV2, reg_data);
                    if (AIR_E_OK != rv)
                    {
                        hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page);
                        return rv;
                    }
                }

                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page);
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8811 %d switch to page %u fail \n", port, reg_data);
            }
        }
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

/* FUNCTION NAME:   hal_en8811h_phy_getLoopBack
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
hal_en8811h_phy_getLoopBack(
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
        rv = hal_mdio_readC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, &page);
        if (AIR_E_OK == rv)
        {
            /* Swtich to page 1 */
            reg_data = 1;
            rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, reg_data);
            if (AIR_E_OK == rv)
            {
                /* Read data from ExtReg1A */
                rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
                if (AIR_E_OK == rv)
                {
                    if (reg_data & LPBK_FAR_END)
                    {
                        *ptr_enable = TRUE;
                    }
                    else
                    {
                        *ptr_enable = FALSE;
                    }
                }

                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page);
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8811 %d switch to page %u fail \n", port, reg_data);
            }
        }

        /* Restore page*/
        if ((rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
        {
            return rv;
        }
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

/* FUNCTION NAME:   hal_en8811h_phy_setSmartSpeedDown
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
hal_en8811h_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, &page);
    if (AIR_E_OK == rv)
    {
        reg_data = 1;
        rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, reg_data);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data);
            if (AIR_E_OK == rv)
            {
                data &= ~BITS(0, 3);
                if (HAL_PHY_SSD_MODE_DISABLE != ssd_mode)
                {
                    data |= BIT(4);
                    data |= ssd_mode;
                }
                else
                {
                    data &= ~BIT(4);
                }

                rv = hal_mdio_writeC22ByPort(unit, port, 0x14, data);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page);
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getSmartSpeedDown
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
hal_en8811h_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Backup page */
    rv = hal_mdio_readC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, &page);
    if (AIR_E_OK == rv)
    {
        reg_data = 1;
        rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, reg_data);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_readC22ByPort(unit, port, 0x14, &data);
            if (AIR_E_OK == rv)
            {
                reg_data = BITS_OFF_R(data, 4, 1);
                if (reg_data)
                {
                    *ptr_ssd_mode = (BITS_OFF_R(data, 0, 4));
                }
                else
                {
                    *ptr_ssd_mode = HAL_PHY_SSD_MODE_DISABLE;
                }

                rv = hal_mdio_writeC22ByPort(unit, port, EN8811H_PHY_MII_PAGE_SELECT, page);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setLedOnCtrl
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
hal_en8811h_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    if (led_id == 0)
    {
        reg_addr = EN8811H_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8811H_LED_1_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (TRUE == enable)
        {
            reg_data |= EN8811H_LED_LINK_FORCE_ON;
        }
        else
        {
            reg_data &= ~(EN8811H_LED_LINK_FORCE_ON);
        }

        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
    }

    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getLedOnCtrl
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
hal_en8811h_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    if (led_id == 0)
    {
        reg_addr = EN8811H_LED_0_ON_MASK;
    }
    else if (led_id == 1)
    {
        reg_addr = EN8811H_LED_1_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & EN8811H_LED_LINK_FORCE_ON)
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

/* FUNCTION NAME:   hal_en8811h_phy_testTxCompliance
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
hal_en8811h_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI16_T         u16dat = 0;

    switch (mode)
    {
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B:
            ret = AIR_E_NOT_SUPPORT;
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D:
            ret = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (BMCR_ANENABLE | BMCR_SPEED1000 | BMCR_FULLDPLX));
            if (AIR_E_OK == ret)
            {
                if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
                {
                    u16dat = (CTL1000_TEST_TM1 | CTL1000_PORT_TYPE | ADVERTISE_1000FULL | ADVERTISE_1000HALF);
                }
                else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2 == mode)
                {
                    u16dat = (CTL1000_TEST_TM2 | CTL1000_PORT_TYPE | ADVERTISE_1000FULL | ADVERTISE_1000HALF);
                }
                else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
                {
                    u16dat = (CTL1000_TEST_TM3 | CTL1000_PORT_TYPE | ADVERTISE_1000FULL | ADVERTISE_1000HALF);
                }
                else
                {
                    u16dat = (CTL1000_TEST_TM4 | CTL1000_PORT_TYPE | ADVERTISE_1000FULL | ADVERTISE_1000HALF);
                }

                ret = hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, u16dat);
                /* delay 1s */
                osal_delayUs(1000000);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x1e0228, 0x0);
                    if (AIR_E_OK == ret)
                    {
                        ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                    }
                }
                if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4 == mode ||
                    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A == mode ||
                    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B == mode ||
                    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C == mode ||
                    HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D == mode)
                {
                    if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4 == mode)
                    {
                        u16dat = 0xf;
                    }
                    else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A == mode)
                    {
                        u16dat = 0x1;
                    }
                    else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B == mode)
                    {
                        u16dat = 0x2;
                    }
                    else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C == mode)
                    {
                        u16dat = 0x4;
                    }
                    else
                    {
                        /* HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D */
                        u16dat = 0x8;
                    }

                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3a20, u16dat);
                        if (AIR_E_OK == ret)
                        {
                            ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                        }
                    }
                }
                if (AIR_E_OK == ret)
                {
                    ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, 0x1010);
                }
                if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
                {
                    if (AIR_E_OK == ret)
                    {
                        ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x143, 0x200);
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE:
            ret = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (BMCR_SPEED100 | BMCR_FULLDPLX));
            if (AIR_E_OK == ret)
            {
                ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, 0x5010);
                /* delay 1s */
                osal_delayUs(1000000);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x1e0228, 0x0);
                    if (AIR_E_OK == ret)
                    {
                        ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE:
            ret = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (BMCR_SPEED100 | BMCR_FULLDPLX));
            if (AIR_E_OK == ret)
            {
                ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, 0x5018);
                /* delay 1s */
                osal_delayUs(1000000);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x1e0228, 0x0);
                    if (AIR_E_OK == ret)
                    {
                        ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30008, 0x1000007);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8f601101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x112101);
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x122101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x89611101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x132101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x85024, 0x0);
                            if (AIR_E_OK == ret)
                            {
                                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x89601101);
                                /* delay 1s */
                                osal_delayUs(1000000);
                                if (AIR_E_OK == ret)
                                {
                                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x10608, 0x808);
                                }
                            }
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x142101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3089c, 0x1ff);
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x242101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3089c, 0x1ff);
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x442101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3089c, 0x1ff);
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x542101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3089c, 0x1ff);
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x642101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x3089c, 0x1ff);
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x152101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                        if (AIR_E_OK == ret)
                        {
                            ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30080, 0xc000006);
                            if (AIR_E_OK == ret)
                            {
                                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30898, 0x1ff01d8);
                            }
                        }
                    }
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6:
            ret = _hal_en8811h_forceXBZ(unit, port);
            if (AIR_E_OK == ret)
            {
                ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c611101);
                if (AIR_E_OK == ret)
                {
                    ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30004, 0x162101);
                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30200, 0x8c601101);
                    }
                }
            }
            break;
        default:
            ret = AIR_E_BAD_PARAMETER;
            break;
    }
    return ret;
}

/* FUNCTION NAME:   hal_en8811h_phy_setPhyLedCtrlMode
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
hal_en8811h_phy_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    UI16_T              data = 0, led_config = 0, offset = 0;
    UI16_T              link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;
    AIR_ERROR_NO_T      rv = AIR_E_OK;
    BOOL_T              is_force = FALSE;
    HAL_PHY_LED_STATE_T state;
    AIR_CFG_VALUE_T     led_behavior;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8811H_LED_0_CFG_CTRL_MODE_BIT + (EN8811H_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(EN8811H_LED_CFG_CTRL_MODE_MASK, offset, EN8811H_LED_CFG_CTRL_MODE_WIDTH));
        data |= BITS_OFF_L(ctrl_mode, offset, EN8811H_LED_CFG_CTRL_MODE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        is_force = (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode) ? TRUE : FALSE;
        if (TRUE == is_force)
        {
            rv = hal_en8811h_phy_getPhyLedForceState(unit, port, led_id, &state);
            if (AIR_E_OK == rv)
            {
                rv = hal_en8811h_phy_setPhyLedForceState(unit, port, led_id, state);
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
                led_config = led_behavior.value;

                link_reg_data = 0;
                blk_reg_data = 0;

                LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, link_reg_addr);
                LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, blk_reg_addr);

                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_1000, EN8811H_LED_LINK_1000, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_100, EN8811H_LED_LINK_100, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_10, EN8811H_LED_LINK_10, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_FULLDPLX, EN8811H_LED_LINK_FULLDPLX, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_HALFDPLX, EN8811H_LED_LINK_HALFDPLX, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_HIGH_ACTIVE, EN8811H_LED_POL_HIGH_ACT, link_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_LINK_2500, EN8811H_LED_LINK_2500, link_reg_data);

                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_1000, EN8811H_LED_BLINK_TX_1000, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_1000, EN8811H_LED_BLINK_RX_1000, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_100, EN8811H_LED_BLINK_TX_100, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_100, EN8811H_LED_BLINK_RX_100, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_10, EN8811H_LED_BLINK_TX_10, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_10, EN8811H_LED_BLINK_RX_10, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_TX_2500, EN8811H_LED_BLINK_TX_2500, blk_reg_data);
                GET_PHY_ABILITY(led_config, EN8811H_LED_BHV_BLINK_RX_2500, EN8811H_LED_BLINK_RX_2500, blk_reg_data);

                if (link_reg_data != 0 || blk_reg_data != 0)
                {
                    link_reg_data |= EN8811H_LED_FUNC_ENABLE;
                }

                DIAG_PRINT(HAL_DBG_INFO, "port %u, link_reg_addr 0x%02X, link_reg_data 0x%04X\n", port, link_reg_addr,
                           link_reg_data);
                DIAG_PRINT(HAL_DBG_INFO, "port %u, blk_reg_addr 0x%02X, blk_reg_data 0x%04X\n", port, blk_reg_addr,
                           blk_reg_data);

                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, link_reg_addr, link_reg_data);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, blk_reg_addr, blk_reg_data);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8811H_LED_ON_DURATION,
                                                     (EN8811H_LED_BLINK_RATE_DEFAULT / 2));
                        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8811H_LED_BLINK_DURATION,
                                                      EN8811H_LED_BLINK_RATE_DEFAULT);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getPhyLedCtrlMode
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
hal_en8811h_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    UI16_T         data = 0, offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8811H_LED_0_CFG_CTRL_MODE_BIT + (EN8811H_LED_CFG_RG_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(data, offset, EN8811H_LED_CFG_CTRL_MODE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setPhyLedForceState
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
hal_en8811h_phy_setPhyLedForceState(
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

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8811H_LED_0_CFG_STATE_BIT + (EN8811H_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(EN8811H_LED_CFG_STATE_MASK, offset, EN8811H_LED_CFG_STATE_WIDTH));
        data |= BITS_OFF_L(state, offset, EN8811H_LED_CFG_STATE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_en8811h_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                is_force = (HAL_PHY_LED_STATE_FORCE_PATT == state) ? TRUE : FALSE;
                if (TRUE == is_force)
                {
                    rv = hal_en8811h_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_en8811h_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                    }
                }
                else
                {
                    (HAL_PHY_LED_STATE_ON == state ? (reg_data = EN8811H_LED_LINK_FORCE_ON) : (reg_data = 0));
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr,
                                                 (reg_data | EN8811H_LED_FUNC_ENABLE));
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

/* FUNCTION NAME:   hal_en8811h_phy_getPhyLedForceState
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
hal_en8811h_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    UI16_T         data = 0, offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (EN8811H_LED_0_CFG_STATE_BIT + (EN8811H_LED_CFG_RG_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(data, offset, EN8811H_LED_CFG_STATE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_setPhyLedForcePattCfg
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
hal_en8811h_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    UI16_T                  reg_data = 0, data = 0, reg_addr = 0;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    HAL_PHY_LED_CTRL_MODE_T ctrl_mode;
    AIR_CFG_VALUE_T         led_behavior;
    HAL_PHY_LED_STATE_T     led_state;

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    switch (pattern)
    {
        case HAL_PHY_LED_PATT_HZ_HALF:
            reg_data = EN8811H_LED_BLINK_RATE_HZ_HALF;
            break;
        case HAL_PHY_LED_PATT_HZ_ONE:
            reg_data = EN8811H_LED_BLINK_RATE_HZ_ONE;
            break;
        case HAL_PHY_LED_PATT_HZ_TWO:
            reg_data = EN8811H_LED_BLINK_RATE_HZ_TWO;
            break;
        default:
            rv = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
        if (AIR_E_OK == rv)
        {
            data &= ~(BITS_OFF_L(EN8811H_LED_CFG_PATT_MASK, EN8811H_LED_CFG_PATT_BIT, EN8811H_LED_CFG_PATT_WIDTH));
            data |= BITS_OFF_L(pattern, EN8811H_LED_CFG_PATT_BIT, EN8811H_LED_CFG_PATT_WIDTH);
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_en8811h_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
        {
            rv = hal_en8811h_phy_getPhyLedForceState(unit, port, led_id, &led_state);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8811H_LED_ON_DURATION, (reg_data / 2));
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, EN8811H_LED_BLINK_DURATION, reg_data);
                if (AIR_E_OK == rv)
                {
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, EN8811H_LED_BLINK_FORCE);
                    if (AIR_E_OK == rv)
                    {
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                        reg_data = EN8811H_LED_FUNC_ENABLE;
                        led_behavior.value = 0xFFFF;
                        led_behavior.param0 = port;
                        led_behavior.param1 = led_id;

                        hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
                        if (led_behavior.value & EN8811H_LED_BHV_HIGH_ACTIVE)
                        {
                            reg_data |= EN8811H_LED_POL_HIGH_ACT;
                        }
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_en8811h_phy_getPhyLedForcePattCfg
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
hal_en8811h_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    UI16_T         data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        *ptr_pattern = BITS_OFF_R(data, EN8811H_LED_CFG_PATT_BIT, EN8811H_LED_CFG_PATT_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME: hal_en8811h_phy_triggerCableTest
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
hal_en8811h_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    _hal_en8811h_token_ring_write(unit, port, _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h, 0x2);
    _hal_en8811h_token_ring_write(unit, port, _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h, 0x0);

    ret = _hal_en8811h_SOC_step1_Click(unit, port, ptr_test_rslt);

    _hal_en8811h_token_ring_write(unit, port, _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h, 0xf4);
    _hal_en8811h_token_ring_write(unit, port, _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h, 0x1);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_getCableTestRawData
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
hal_en8811h_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_setPhyLedGlbCfg
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
hal_en8811h_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_getPhyLedGlbCfg
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
hal_en8811h_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_setPhyLedBlkEvent
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
hal_en8811h_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_getPhyLedBlkEvent
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
hal_en8811h_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    HAL_CHECK_PARAM((led_id >= HAL_EN8811H_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_setPhyLedDuration
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
hal_en8811h_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_getPhyLedDuration
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
hal_en8811h_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_en8811h_phy_dumpPhyPara
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
hal_en8811h_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI16_T                reg_data = 0;
    UI32_T                pbus_data = 0;
    HAL_PHY_LINK_STATUS_T link_status;

    osal_printf("\n=== cl22 ===\n");
    rv = hal_mdio_readC22ByPort(unit, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, MII_BMSR, reg_data);
    }
    rv = hal_mdio_readC22ByPort(unit, port, 0x1d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x1d, reg_data);
    }
    osal_printf("\n=== buck pbus reg ===\n");
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x3b3c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     en8811h version           = %x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xca0f8, &pbus_data);
    if (AIR_E_OK == rv)
    {
        switch (reg_data & 0x3)
        {
            case (EN8811H_TX_POLARITY_REVERSE | EN8811H_RX_POLARITY_NORMAL):
                osal_printf("port=%2u     en8811h Tx, Rx Polarity   = Tx Reverse, Rx Normal\n", port);
                break;
            case (EN8811H_TX_POLARITY_REVERSE | EN8811H_RX_POLARITY_REVERSE):
                osal_printf("port=%2u     en8811h Tx, Rx Polarity   = Tx Reverse, Rx Reverse\n", port);
                break;
            case (EN8811H_TX_POLARITY_NORMAL | EN8811H_RX_POLARITY_REVERSE):
                osal_printf("port=%2u     en8811h Tx, Rx Polarity   = Tx Normal, Rx Reverse\n", port);
                break;
            case (EN8811H_TX_POLARITY_NORMAL | EN8811H_RX_POLARITY_NORMAL):
            default:
                osal_printf("port=%2u     en8811h Tx, Rx Polarity   = Tx Normal, Rx Normal\n", port);
                break;
        }
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x3a9c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        if (pbus_data & BIT(3))
        {
            osal_printf("port=%2u     en8811h surge mode        = 5R\n", port);
        }
        else
        {
            osal_printf("port=%2u     en8811h surge mode        = 0R\n", port);
        }
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xcf958, &pbus_data);
    if (AIR_E_OK == rv)
    {
        if (pbus_data & BIT(26))
        {
            osal_printf("port=%2u     en8811h Co-Clock Output   = Enable\n", port);
        }
        else
        {
            osal_printf("port=%2u     en8811h Co-Clock Output   = Disable\n", port);
        }
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0004, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_SYS_LINK_MODE          = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe000c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_FCM_CTRL               = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0020, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_SS_PAUSE_TIME          = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe002c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_MIN_IPG_NUM            = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc0000, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_CTROL_0                = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc0b04, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_LINK_STATUS            = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc0014, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_LINK_PARTNER_AN        = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x1020c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_FN_PWR_CTRL_STATUS     = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x3a48, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     RG_WHILE_LOOP_COUNT       = 0x%08x\n", port, pbus_data);
    }
    osal_printf("\n=== fcm counter ===\n");
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0090, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     rx from line side_s       = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0094, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     rx from line side_t       = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe009c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     tx to system side_s       = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe00a0, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     tx to system side_t       = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0078, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     rx from system side_s     = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe007c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     rx from system side_t     = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0084, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     tx to line side_s         = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0088, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     tx to line side_t         = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe00a4, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     pause to system side      = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0098, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     pause from line side      = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe0080, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     pause from system side    = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xe008c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("port=%2u     pause to line side        = 0x%08x\n", port, pbus_data);
    }
    rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xe0074, 0x3);
    if (AIR_E_OK == rv)
    {
        rv = hal_en8811h_phy_getLinkStatus(unit, port, &link_status);
        if (AIR_E_OK == rv)
        {
            if ((link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (link_status.speed == HAL_PHY_SPEED_2500M))
            {
                osal_printf("\n=== line side counter ===\n");
                rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0x10);
                if (AIR_E_OK == rv)
                {
                    rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0x0);
                }
                if (AIR_E_OK == rv)
                {
                    osal_printf("\n=== before efifo ===\n");
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x3071c, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx to line side_s         = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30720, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx to line side_t         = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x3072c, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx from line side_s       = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30730, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx from line side_t       = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30724, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx_enc                    = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30728, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx_dec                    = 0x%08x\n", port, pbus_data);
                    }
                    osal_printf("\n=== after efifo ===\n");
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30734, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx to line side_s         = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30738, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx to line side_t         = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30764, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx from line side_s      = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x30768, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx from line side_t       = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0x13);
                    if (AIR_E_OK == rv)
                    {
                        rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0x3);
                        if (AIR_E_OK == rv)
                        {
                            rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0x10);
                            if (AIR_E_OK == rv)
                            {
                                rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0x30718, 0);
                            }
                        }
                    }

                    if (AIR_E_OK == rv)
                    {
                        osal_printf("\n=== mac counter ===\n");
                        rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x131000, &pbus_data);
                        if (AIR_E_OK == rv)
                        {
                            osal_printf("port=%2u     tx error from system side = 0x%08x\n", port, pbus_data);
                        }
                        rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x132000, &pbus_data);
                        if (AIR_E_OK == rv)
                        {
                            osal_printf("port=%2u     rx error to system side   = 0x%08x\n", port, pbus_data);
                        }
                        rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x131004, &pbus_data);
                        if (AIR_E_OK == rv)
                        {
                            osal_printf("port=%2u     tx from system side       = 0x%08x\n", port, pbus_data);
                        }
                        rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0x132004, &pbus_data);
                        if (AIR_E_OK == rv)
                        {
                            osal_printf("port=%2u     rx to system side         = 0x%08x\n", port, pbus_data);
                        }
                    }
                }
            }
            else if ((link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) &&
                     (link_status.speed != HAL_PHY_SPEED_2500M))
            {
                osal_printf("\n=== line side counter ===\n");
                rv = hal_mdio_writeC22ByPort(unit, port, 0x1f, 1);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_readC22ByPort(unit, port, 0x12, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx from line side         = 0x%08x\n", port, (reg_data & 0x7fff));
                    }
                    rv = hal_mdio_readC22ByPort(unit, port, 0x17, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx error from line side   = 0x%08x\n", port, (reg_data & 0xff));
                    }
                    rv = hal_mdio_writeC22ByPort(unit, port, 0x1f, 0);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC22ByPort(unit, port, 0x1f, 0x52b5);
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_writeC22ByPort(unit, port, 0x10, 0xbf92);
                            if (AIR_E_OK == rv)
                            {
                                rv = hal_mdio_readC22ByPort(unit, port, 0x11, &reg_data);
                                if (AIR_E_OK == rv)
                                {
                                    osal_printf("port=%2u     tx to line side           = 0x%08x\n", port,
                                                ((reg_data & 0x7ffe) >> 1));
                                }
                                rv = hal_mdio_readC22ByPort(unit, port, 0x12, &reg_data);
                                if (AIR_E_OK == rv)
                                {
                                    osal_printf("port=%2u     tx error to line side     = 0x%08x\n", port,
                                                ((reg_data & 0x7f)));
                                }
                                rv = hal_mdio_writeC22ByPort(unit, port, 0x1f, 0);
                            }
                        }
                    }
                }
            }

            if ((link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP) && (AIR_E_OK == rv))
            {
                osal_printf("\n=== system side counter ===\n");
                rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xc602c, 0x3);
                if (AIR_E_OK == rv)
                {
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc60b0, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx start                  = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc60b4, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     tx terminal               = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc60bc, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx start                  = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regrd(unit, port, 0xc60c0, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("port=%2u     rx terminal               = 0x%08x\n", port, pbus_data);
                    }
                    rv = _hal_en8811h_buck_pbus_regwr(unit, port, 0xc602c, 0x4);
                }
            }
        }
    }

    /* Dump cable diag EC_COEF information*/
    rv |= _hal_en8811h_dump_EC_COEF(unit, port);
    return rv;
}

/* FUNCTION NAME: hal_en8811h_phy_triggerLinkDownCableTest
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
hal_en8811h_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerLinkDownCableTest(unit, port, ptr_test_rslt);
    return ret;
}

const HAL_PHY_DRIVER_T _ext_EN8811H_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_en8811h_phy_init,
    hal_en8811h_phy_setAdminState,
    hal_en8811h_phy_getAdminState,
    hal_en8811h_phy_setAutoNego,
    hal_en8811h_phy_getAutoNego,
    hal_en8811h_phy_setLocalAdvAbility,
    hal_en8811h_phy_getLocalAdvAbility,
    hal_en8811h_phy_getRemoteAdvAbility,
    hal_en8811h_phy_setSpeed,
    hal_en8811h_phy_getSpeed,
    hal_en8811h_phy_setDuplex,
    hal_en8811h_phy_getDuplex,
    hal_en8811h_phy_getLinkStatus,
    hal_en8811h_phy_setLoopBack,
    hal_en8811h_phy_getLoopBack,
    hal_en8811h_phy_setSmartSpeedDown,
    hal_en8811h_phy_getSmartSpeedDown,
    hal_en8811h_phy_setLedOnCtrl,
    hal_en8811h_phy_getLedOnCtrl,
    hal_en8811h_phy_testTxCompliance,
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    NULL, /* setSerdesMode */
    NULL, /* getSerdesMode */
    hal_en8811h_phy_setPhyLedCtrlMode,
    hal_en8811h_phy_getPhyLedCtrlMode,
    hal_en8811h_phy_setPhyLedForceState,
    hal_en8811h_phy_getPhyLedForceState,
    hal_en8811h_phy_setPhyLedForcePattCfg,
    hal_en8811h_phy_getPhyLedForcePattCfg,
    hal_en8811h_phy_triggerCableTest,
    NULL, /* getCableTestRawData */
    hal_en8811h_phy_setPhyLedGlbCfg,
    hal_en8811h_phy_getPhyLedGlbCfg,
    hal_en8811h_phy_setPhyLedBlkEvent,
    hal_en8811h_phy_getPhyLedBlkEvent,
    hal_en8811h_phy_setPhyLedDuration,
    hal_en8811h_phy_getPhyLedDuration,
    NULL, /* setPhyOpMode */
    NULL, /* getPhyOpMode */
    hal_en8811h_phy_dumpPhyPara,
    NULL, /* triggerLinkDownCableTest */
    NULL, /* dumpPortCnt */
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_en8811h_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_EN8811H_phy_func_vec;

    return (AIR_E_OK);
}
