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

/* FILE NAME:  hal_an8811b_phy.c
 * PURPOSE:
 *  Implement an8811b phy module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/phy/an8811b/hal_an8811b_phy.h>

#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_crc.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_AN8811B_PHY_READY        (0x02)
#define HAL_AN8811B_MAX_RETRY        (10)
#define HAL_AN8811B_LINK_SPEED_2500M (0x10)

#define PHY_MAX_NUMBER (32)

#define PHY_LED_TYPE_ON  (0)
#define PHY_LED_TYPE_BLK (1)

#define HAL_AN8811B_MAX_LED_COUNT (2)
#define PHY_LED_CFG_REG           (AN8811B_LED_3_BLK_MASK)
/* MACRO FUNCTION DECLARATIONS
 */
#define LED_ID_TO_LED_REG_ADDRESS(__led_id__, __led_type__, __led_reg_addr__)                \
    do                                                                                       \
    {                                                                                        \
        ((__led_type__) == PHY_LED_TYPE_ON ? ((__led_reg_addr__) = AN8811B_LED_0_ON_MASK) :  \
                                             ((__led_reg_addr__) = AN8811B_LED_0_BLK_MASK)); \
        __led_reg_addr__ += (AN8811B_LED_RG_OFFSET * __led_id__);                            \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_an8811b_phy.c");

extern const UI32_T an8811b_fw_dm_size;
extern const UI8_T  an8811b_fw_dm[];
extern const UI32_T an8811b_fw_pm_size;
extern const UI8_T  an8811b_fw_pm[];

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* AN8811B PBUS write function */
static AIR_ERROR_NO_T
_hal_an8811b_pbus_regwr(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pbus_addr,
    const UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, (pbus_addr >> 6));
    rv |= hal_mdio_writeC22ByPort(unit, port, ((pbus_addr >> 2) & 0xf), (pbus_data & 0xFFFF));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, (pbus_data >> 16));
    return rv;
}

/* AN8811B BUCK write function */
static AIR_ERROR_NO_T
_hal_an8811b_buck_pbus_regwr(
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
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
    return rv;
}

/* AN8811B BUCK read function */
static AIR_ERROR_NO_T
_hal_an8811b_buck_pbus_regrd(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pbus_addr,
    UI32_T      *ptr_pbus_data)
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
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    *ptr_pbus_data = pbus_data;
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_set_host_cmd(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T cmd_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         retry = 0, reg_value = 0, ack_value = 0;

    for (retry = 0; retry < AN8811B_HOST_CMD_MAX_RETRY; retry++)
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_STATUS, &reg_value);
        if ((AIR_E_OK == rv) && (AN8811B_HOST_STATUS_READY == reg_value)) /* host command done */
        {
            rv = AIR_E_OK;
            break;
        }
        osal_delayUs(100000);
        rv = AIR_E_TIMEOUT;
    }
    if (AIR_E_OK == rv)
    {
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_STATUS, 0x0);
        if (AIR_E_OK == rv)
        {
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_ACK, 0x0);
            if (AIR_E_OK == rv)
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_DES_H, AN8811B_HOST_CMD_ISSUE);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_DES_L, cmd_data);
                }
            }
        }
    }
    else
    {
        rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_ACK, &ack_value);
        if (AIR_E_OK == rv)
        {
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d set host command fail. ack = %x, status = %x\n", port,
                       ack_value, reg_value);
        }
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_forceXBZ(
    UI32_T unit,
    UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbus_data = 0x0;

    rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, (BMCR_SPEED1000 | BMCR_SPEED100 | BMCR_FULLDPLX));
    if (AIR_E_OK == rv)
    {
        rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x10204, 0x01);
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
            pbus_data |= 0x01;
            rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_txc_2500m_tmode(
    UI32_T unit,
    UI32_T port,
    UI32_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         tx_bias = 0, tx_driving = 0, tx_filter = 0;

    rv = _hal_an8811b_forceXBZ(unit, port);
    if (AIR_E_OK == rv)
    {
        if (2 == mode)
        {
            /* 2500m_tmode_4 */
            tx_bias = 0x07070707;
            tx_driving = 0x00004444;
            tx_filter = 0x1f1f1f1f;
        }
        else
        {
            /* 2500m_tmode others */
            tx_bias = 0x05050505;
            tx_driving = 0x00002222;
            tx_filter = 0x10101010;
        }
        rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x10c5c, mode);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b0020, 0x30000f4);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b0080, tx_bias);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b031c, 0x1111111);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b0088, tx_driving);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b007c, tx_filter);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b001c, 0x02707777);
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_txc_2500m_tm4_tone(
    UI32_T unit,
    UI32_T port,
    UI32_T tone)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbus_data = 0x0;

    rv = _hal_an8811b_txc_2500m_tmode(unit, port, 0x02); /* mode 4 */
    if (AIR_E_OK == rv)
    {
        tone &= 0x07;
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
        pbus_data &= ~BITS(0, 11);
        pbus_data |= 0x0f0f;
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, (0x01 << 16));
        rv |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
        pbus_data &= ~BITS(20, 22);
        pbus_data |= (tone << 20);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);
        pbus_data &= ~BITS(16, 19);
        pbus_data |= (4 << 16);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);

        rv |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
        pbus_data &= ~BIT(16);
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
        rv |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x3089c, &pbus_data);
        pbus_data &= ~BITS(0, 10);
        pbus_data |= 0x3ff;
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x3089c, pbus_data);
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_mdio_write_buffer(
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
_hal_an8811b_crc_check(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T rg_set,
    const UI32_T rg_mon2,
    const UI32_T rg_mon3)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbus_data = 0, retry = 0;

    rv = _hal_an8811b_buck_pbus_regrd(unit, port, rg_set, &pbus_data);
    if (AIR_E_OK == rv)
    {
        pbus_data |= AN8811B_CRC_RD_EN;
        rv = _hal_an8811b_buck_pbus_regwr(unit, port, rg_set, pbus_data);
        if (AIR_E_OK == rv)
        {
            for (retry = 0; retry < HAL_AN8811B_MAX_RETRY; retry++)
            {
                osal_delayUs(300000);
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, rg_mon2, &pbus_data);
                if ((AIR_E_OK == rv) && (2 == (pbus_data & AN8811B_CRC_ST_MASK)))
                {
                    rv = _hal_an8811b_buck_pbus_regrd(unit, port, rg_mon3, &pbus_data);
                    if (AIR_E_OK == rv)
                    {
                        if (pbus_data & AN8811B_CRC_CHECK_PASS)
                        {
                            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: AN8811B port %d CRC check pass\n", port);
                        }
                        else
                        {
                            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d CRC check fail\n", port);
                            rv = AIR_E_NOT_INITED;
                        }
                        break;
                    }
                }
            }
            if (HAL_AN8811B_MAX_RETRY == retry)
            {
                rv = AIR_E_NOT_INITED;
            }
        }
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_firmware_download(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T         reg_value = 0, retry = 0;
    UI32_T         pbus_value = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* PMU buck power reset */
    rv = _hal_an8811b_pbus_regwr(unit, port, 0x5cf9f8, 0x10000);
    rv |= _hal_an8811b_pbus_regwr(unit, port, 0x5cf9fc, 0x0);
    osal_delayUs(50000);
    rv |= _hal_an8811b_pbus_regwr(unit, port, 0x5cf9fc, 0x10000);
    rv |= _hal_an8811b_pbus_regwr(unit, port, 0x5cf9f8, 0x10001);

    rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x0f0018, 0x0);
    rv |= _hal_an8811b_mdio_write_buffer(unit, port, 0x00000000, an8811b_fw_dm_size, an8811b_fw_dm);
    /* DM CRC check */
    rv |= _hal_an8811b_crc_check(unit, port, AN8811B_RG_CRC_DM_SET1, AN8811B_RG_CRC_DM_MON2, AN8811B_RG_CRC_DM_MON3);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d DM CRC check fail\n", port);
        return AIR_E_NOT_INITED;
    }
    rv = _hal_an8811b_mdio_write_buffer(unit, port, 0x00100000, an8811b_fw_pm_size, an8811b_fw_pm);
    /* PM CRC check */
    rv |= _hal_an8811b_crc_check(unit, port, AN8811B_RG_CRC_PM_SET1, AN8811B_RG_CRC_PM_MON2, AN8811B_RG_CRC_PM_MON3);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d PM CRC check fail\n", port);
        return AIR_E_NOT_INITED;
    }
    /* MD32 firmware control */
    for (retry = 0; retry < HAL_AN8811B_MAX_RETRY; retry++)
    {
        osal_delayUs(300000);
        rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x0f0018, 0x01);
        rv |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x0f0018, &pbus_value);
        if (0x01 == pbus_value)
        {
            break;
        }
    }
    if (HAL_AN8811B_MAX_RETRY != retry)
    {
        for (retry = 0; retry < HAL_AN8811B_MAX_RETRY; retry++)
        {
            osal_delayUs(300000);
            rv |= hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_STATUS, &reg_value);
            if (HAL_AN8811B_PHY_READY == reg_value)
            {
                break;
            }
        }
    }

    if (HAL_AN8811B_MAX_RETRY == retry)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d firmware download fail\n", port);
        return AIR_E_NOT_INITED;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_an8811b_led_setting(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T led_behavior;
    UI16_T          led_id = 0, led_config = 0, led_count = 0;
    UI16_T          link_reg_data = 0, blk_reg_data = 0, link_reg_addr = 0, blk_reg_addr = 0;

    /* LED blink frequence */
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x21, 0x8008);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x22, 0x600);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x23, 0xc00);

    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
    led_behavior.value = AN8811B_LED_COUNT;
    rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_COUNT, &led_behavior);
    led_count = led_behavior.value;

    for (led_id = 0; led_id < led_count; led_id++)
    {
        if (0 == led_id)
        {
            link_reg_addr = AN8811B_LED_0_ON_MASK;
            blk_reg_addr = AN8811B_LED_0_BLK_MASK;
        }
        else if (1 == led_id)
        {
            link_reg_addr = AN8811B_LED_1_ON_MASK;
            blk_reg_addr = AN8811B_LED_1_BLK_MASK;
        }
        else if (2 == led_id)
        {
            link_reg_addr = AN8811B_LED_2_ON_MASK;
            blk_reg_addr = AN8811B_LED_2_BLK_MASK;
        }
        else
        {
            link_reg_addr = AN8811B_LED_3_ON_MASK;
            blk_reg_addr = AN8811B_LED_3_BLK_MASK;
        }

        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

        led_behavior.value = 0xFFFF;
        led_behavior.param0 = port;
        led_behavior.param1 = led_id;

        rv |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

        led_config = led_behavior.value;

        link_reg_data = 0;
        blk_reg_data = 0;

        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_1000, AN8811B_LED_LINK_1000, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_100, AN8811B_LED_LINK_100, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_10, AN8811B_LED_LINK_10, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_FULLDPLX, AN8811B_LED_LINK_FULLDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_HALFDPLX, AN8811B_LED_LINK_HALFDPLX, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_HIGH_ACTIVE, AN8811B_LED_POL_HIGH_ACT, link_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_2500, AN8811B_LED_LINK_2500, link_reg_data);

        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_1000, AN8811B_LED_BLINK_TX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_1000, AN8811B_LED_BLINK_RX_1000, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_100, AN8811B_LED_BLINK_TX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_100, AN8811B_LED_BLINK_RX_100, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_10, AN8811B_LED_BLINK_TX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_10, AN8811B_LED_BLINK_RX_10, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_2500, AN8811B_LED_BLINK_TX_2500, blk_reg_data);
        GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_2500, AN8811B_LED_BLINK_RX_2500, blk_reg_data);

        if (link_reg_data != 0 || blk_reg_data != 0)
        {
            link_reg_data |= AN8811B_LED_FUNC_ENABLE;
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

/* EXPORTED SUBPROGRAM BODIES*/

/* FUNCTION NAME:   hal_an8811b_phy_init
 * PURPOSE:
 *      AN8811B PHY initialization
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
hal_an8811b_phy_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    UI16_T          phy_addr = 0, reg_value = 0;
    UI32_T          reg_data = 0;
    AIR_CFG_VALUE_T mtcc_cfg;

    phy_addr = HAL_PHY_PORT_DEV_PHY_ADDR(unit, port);
    if (phy_addr >= PHY_MAX_NUMBER)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: an8811b %d initialize fail\n", port);
        return AIR_E_BAD_PARAMETER;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC1, AN8811B_RG_HOST_STATUS, &reg_value);
    if (AIR_E_OK == rv)
    {
        if (AN8811B_HOST_STATUS_READY != reg_value)
        {
            rv = _hal_an8811b_firmware_download(unit, port);
            if (AIR_E_OK != rv)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: an8811b %d download firmware fail\n", port);
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        /* led iomux setup GPIO-4 & 5 */
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5cf8b8, &reg_data);
        if (AIR_E_OK == rv)
        {
            reg_data &= ~(0x88);
            reg_data |= 0x30;
            rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x5cf8b8, reg_data);

            if (AIR_E_OK == rv)
            {
                rv = _hal_an8811b_led_setting(unit, port);
            }
        }
        /* set serdes polarity */
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5ce004, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data |= AN8811B_TX_POLARITY_NORMAL;
                rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x5ce004, reg_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5ce61c, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data |= AN8811B_RX_POLARITY_NORMAL;
                rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x5ce61c, reg_data);
            }
        }
        /* disable co-clock out */
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5cf9e4, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data |= 0x7000;
                rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x5cf9e4, reg_data);
                if (AIR_E_OK == rv)
                {
                    DIAG_PRINT(HAL_DBG_INFO, "[Port %d]CKO Output mode - Disabled.\n", port);
                }
            }
        }
        /* set mtcc */
        if (AIR_E_OK == rv)
        {
            osal_memset(&mtcc_cfg, 0, sizeof(AIR_CFG_VALUE_T));
            hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT, &mtcc_cfg);
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x213e00, &reg_data);
            if (AIR_E_OK == rv)
            {
                reg_data &= ~BITS_RANGE(8, 4);
                reg_data |= ((mtcc_cfg.value << 8) & BITS_RANGE(8, 4));
                rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x213e00, reg_data);
                if (AIR_E_OK == rv)
                {
                    DIAG_PRINT(HAL_DBG_INFO, "[Port %d]MTCC - %u.\n", port, mtcc_cfg.value);
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_setAdminState
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
hal_an8811b_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    if (HAL_PHY_ADMIN_STATE_DISABLE == state)
    {
        /* workaround */
        rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
        if (AIR_E_OK == rv)
        {
            reg_data |= BMCR_ANRESTART;
            rv = hal_mdio_writeC22ByPort(unit, port, MII_BMCR, reg_data);
        }
    }
    if (AIR_E_OK == rv)
    {
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
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getAdminState
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
hal_an8811b_phy_getAdminState(
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

/* FUNCTION NAME:   hal_an8811b_phy_setAutoNego
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
hal_an8811b_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

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

/* FUNCTION NAME:   hal_an8811b_phy_getAutoNego
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
hal_an8811b_phy_getAutoNego(
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

/* FUNCTION NAME:   hal_an8811b_phy_setLocalAdvAbility
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
hal_an8811b_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

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
        if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE)
        {
            rv = _hal_an8811b_set_host_cmd(unit, port, AN8811B_CMD_EEE_2G5_ENABLE);
            if (AIR_E_OK == rv)
            {
                rv = _hal_an8811b_set_host_cmd(unit, port, AN8811B_CMD_EEE_1G_100M_ENABLE);
            }
        }
        else
        {
            rv = _hal_an8811b_set_host_cmd(unit, port, AN8811B_CMD_EEE_DISABLE);
        }
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

/* FUNCTION NAME:   hal_an8811b_phy_getLocalAdvAbility
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
hal_an8811b_phy_getLocalAdvAbility(
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

/* FUNCTION NAME:   hal_an8811b_phy_getRemoteAdvAbility
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
hal_an8811b_phy_getRemoteAdvAbility(
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

/* FUNCTION NAME:   hal_an8811b_phy_setSpeed
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
hal_an8811b_phy_setSpeed(
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

/* FUNCTION NAME:   hal_an8811b_phy_getSpeed
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
hal_an8811b_phy_getSpeed(
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

/* FUNCTION NAME:   hal_an8811b_phy_setDuplex
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
hal_an8811b_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    UI16_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (AIR_E_OK == rv)
    {
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
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getDuplex
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
hal_an8811b_phy_getDuplex(
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

/* FUNCTION NAME:   hal_an8811b_phy_getLinkStatus
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
hal_an8811b_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    HAL_PHY_AUTO_NEGO_T auto_nego;
    UI16_T              reg_data = 0;
    UI16_T              adv = 0, lpa = 0, lpagb = 0, common_adv_gb = 0, common_adv = 0;
    AIR_ERROR_NO_T      rv = AIR_E_OK;

    rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_REG);
    if (AIR_E_OK != rv)
    {
        return rv;
    }
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
        rv = hal_an8811b_phy_getAutoNego(unit, port, &auto_nego);
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

                                ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                                if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF))
                                {
                                    if (common_adv_gb & LPA_1000FULL)
                                    {
                                        ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                                    }
                                }
                                else if (common_adv & (LPA_100FULL | LPA_100HALF))
                                {
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
                                /* speed status */
                                rv = hal_mdio_readC22ByPort(unit, port, 0x1d, &reg_data);
                                if (AIR_E_OK == rv)
                                {
                                    switch (reg_data & AN8811B_AUX_CTRL_SPEED_MASK)
                                    {
                                        case AN8811B_AUX_CTRL_SPEED_2500:
                                            ptr_status->speed = HAL_PHY_SPEED_2500M;
                                            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                                            break;
                                        case AN8811B_AUX_CTRL_SPEED_1000:
                                            ptr_status->speed = HAL_PHY_SPEED_1000M;
                                            break;
                                        case AN8811B_AUX_CTRL_SPEED_100:
                                            ptr_status->speed = HAL_PHY_SPEED_100M;
                                            break;
                                        case AN8811B_AUX_CTRL_SPEED_10:
                                        default:
                                            ptr_status->speed = HAL_PHY_SPEED_10M;
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                rv = hal_mdio_readC22ByPort(unit, port, MII_BMCR, &reg_data);
                if (AIR_E_OK == rv)
                {
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
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_setLoopBack
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
hal_an8811b_phy_setLoopBack(
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
        rv = hal_mdio_readC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, &page);
        if (AIR_E_OK == rv)
        {
            /* Swtich to page 1 */
            reg_data = 1;
            rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, reg_data);
            if (AIR_E_OK == rv)
            {
                /* Read data from ExtReg1A */
                rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
                if (AIR_E_OK == rv)
                {
                    if (TRUE == enable)
                    {
                        reg_data |= AN8811B_LPBK_FAR_END;
                    }
                    else
                    {
                        reg_data &= ~(AN8811B_LPBK_FAR_END);
                    }

                    rv = hal_mdio_writeC22ByPort(unit, port, MII_RESV2, reg_data);
                    if (AIR_E_OK != rv)
                    {
                        hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page);
                        return rv;
                    }
                }

                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page);
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] AN8811B %d switch to page %u fail \n", port, reg_data);
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

/* FUNCTION NAME:   hal_an8811b_phy_getLoopBack
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
hal_an8811b_phy_getLoopBack(
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
        rv = hal_mdio_readC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, &page);
        if (AIR_E_OK == rv)
        {
            /* Swtich to page 1 */
            reg_data = 1;
            rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, reg_data);
            if (AIR_E_OK == rv)
            {
                /* Read data from ExtReg1A */
                rv = hal_mdio_readC22ByPort(unit, port, MII_RESV2, &reg_data);
                if (AIR_E_OK == rv)
                {
                    if (reg_data & AN8811B_LPBK_FAR_END)
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
                    rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page);
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] AN8811B %d switch to page %u fail \n", port, reg_data);
            }
        }

        /* Restore page*/
        if ((rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page)) != AIR_E_OK)
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

/* FUNCTION NAME:   hal_an8811b_phy_setSmartSpeedDown
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
hal_an8811b_phy_setSmartSpeedDown(
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

    rv = hal_mdio_readC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, &page);
    if (AIR_E_OK == rv)
    {
        reg_data = 1;
        rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, reg_data);
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
                    rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page);
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getSmartSpeedDown
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
hal_an8811b_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    UI16_T         reg_data = 0, page = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    /* Backup page */
    rv = hal_mdio_readC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, &page);
    if (AIR_E_OK == rv)
    {
        reg_data = 1;
        rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, reg_data);
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

                rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, page);
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_setLedOnCtrl
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
hal_an8811b_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    if (0 == led_id)
    {
        reg_addr = AN8811B_LED_0_ON_MASK;
    }
    else if (1 == led_id)
    {
        reg_addr = AN8811B_LED_1_ON_MASK;
    }
    else if (2 == led_id)
    {
        reg_addr = AN8811B_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = AN8811B_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (TRUE == enable)
        {
            reg_data |= AN8811B_LED_LINK_FORCE_ON;
        }
        else
        {
            reg_data &= ~(AN8811B_LED_LINK_FORCE_ON);
        }

        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
    }

    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getLedOnCtrl
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
hal_an8811b_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    UI16_T         reg_data = 0, reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    if (0 == led_id)
    {
        reg_addr = AN8811B_LED_0_ON_MASK;
    }
    else if (1 == led_id)
    {
        reg_addr = AN8811B_LED_1_ON_MASK;
    }
    else if (2 == led_id)
    {
        reg_addr = AN8811B_LED_2_ON_MASK;
    }
    else
    {
        reg_addr = AN8811B_LED_3_ON_MASK;
    }

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & AN8811B_LED_LINK_FORCE_ON)
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

/* FUNCTION NAME:   hal_an8811b_phy_testTxCompliance
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
hal_an8811b_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI16_T         u16dat = 0;
    UI32_T         pbus_data = 0, reg_data2 = 0;

    switch (mode)
    {
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B:
            ret |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1840);
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
            ret |= hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_EXTREG);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0xf842;
                pbus_data = 0x3707777;
                reg_data2 = 0x444444;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0xf842;
                pbus_data = 0x7707777;
                reg_data2 = 0x774444;
            }
            else
            {
                u16dat = 0xf840;
                pbus_data = 0x3707777;
                reg_data2 = 0x444444;
            }
            ret |= hal_mdio_writeC22ByPort(unit, port, 0x1d, u16dat);
            ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b001c, pbus_data);
            ret |= hal_mdio_writeC22ByPort(unit, port, 0x09, 0x0);
            ret |= hal_mdio_writeC22ByPort(unit, port, 0x00, 0x100);
            ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x1b0084, reg_data2);
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
                    else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D == mode)
                    {
                        u16dat = 0x8;
                    }
                    else
                    {
                        ret = AIR_E_BAD_PARAMETER;
                    }

                    if (AIR_E_OK == ret)
                    {
                        ret = _hal_an8811b_buck_pbus_regwr(unit, port, 0x3a20, u16dat);
                        if (AIR_E_OK == ret)
                        {
                            ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                        }
                    }
                }
                if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
                {
                    if (AIR_E_OK == ret)
                    {
                        ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x143, 0x200);
                    }
                }
                if (AIR_E_OK == ret)
                {
                    ret = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, 0x1010);
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
                    ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
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
                    ret = hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x0);
                }
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1:
            ret = _hal_an8811b_txc_2500m_tmode(unit, port, 0x0);
            if (AIR_E_OK == ret)
            {
                ret = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
                pbus_data &= ~BITS(0, 11);
                pbus_data |= 0x0f0f;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30008, &pbus_data);
                pbus_data &= ~BITS(0, 2);
                pbus_data |= 0x07;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30008, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
                pbus_data |= BIT(24);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data |= BIT(25);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data |= BIT(26);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
                pbus_data &= ~BITS(16, 19);
                pbus_data |= (0x01 << 16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2:
            ret = _hal_an8811b_txc_2500m_tmode(unit, port, 0x0);
            if (AIR_E_OK == ret)
            {
                ret = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
                pbus_data &= ~BITS(0, 11);
                pbus_data |= 0x0f0f;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
                pbus_data |= BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
                pbus_data &= ~BITS(16, 19);
                pbus_data |= (0x02 << 16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data &= ~BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3:
            ret = _hal_an8811b_txc_2500m_tmode(unit, port, 0x0);
            if (AIR_E_OK == ret)
            {
                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
                pbus_data |= BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data |= BIT(24);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data |= BIT(25);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data |= BIT(26);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
                pbus_data &= ~BITS(16, 19);
                pbus_data |= (0x03 << 16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x85024, pbus_data);
                pbus_data &= ~BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x85024, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data &= ~BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                /* delay 1s */
                osal_delayUs(1000000);
                ret = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
                pbus_data &= ~BITS(0, 11);
                pbus_data |= 0x0808;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1:
            _hal_an8811b_txc_2500m_tm4_tone(unit, port, 1);
            if (AIR_E_OK != ret)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d Tx compliance configure fail\n", port);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2:
            _hal_an8811b_txc_2500m_tm4_tone(unit, port, 2);
            if (AIR_E_OK != ret)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d Tx compliance configure fail\n", port);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3:
            _hal_an8811b_txc_2500m_tm4_tone(unit, port, 4);
            if (AIR_E_OK != ret)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d Tx compliance configure fail\n", port);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4:
            _hal_an8811b_txc_2500m_tm4_tone(unit, port, 5);
            if (AIR_E_OK != ret)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d Tx compliance configure fail\n", port);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5:
            _hal_an8811b_txc_2500m_tm4_tone(unit, port, 6);
            if (AIR_E_OK != ret)
            {
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: AN8811B port %d Tx compliance configure fail\n", port);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5:
            ret = _hal_an8811b_txc_2500m_tmode(unit, port, 0x0);
            if (AIR_E_OK == ret)
            {
                ret = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
                pbus_data &= ~BITS(0, 11);
                pbus_data |= 0x0f0f;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
                pbus_data |= BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
                pbus_data &= ~BITS(16, 19);
                pbus_data |= (0x05 << 16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data &= ~BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30080, pbus_data);
                pbus_data &= ~BITS(0, 2);
                pbus_data |= 0x06;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30080, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30898, pbus_data);
                pbus_data &= ~BITS(0, 8);
                pbus_data |= 0x01d8;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30898, pbus_data);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6:
            ret = _hal_an8811b_txc_2500m_tmode(unit, port, 0x0);
            if (AIR_E_OK == ret)
            {
                ret = _hal_an8811b_buck_pbus_regrd(unit, port, 0x10608, &pbus_data);
                pbus_data &= ~BITS(0, 11);
                pbus_data |= 0x0f0f;
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x10608, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30200, &pbus_data);
                pbus_data |= BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regrd(unit, port, 0x30004, &pbus_data);
                pbus_data &= ~BITS(16, 19);
                pbus_data |= (0x06 << 16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30004, pbus_data);

                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
                pbus_data &= ~BIT(16);
                ret |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x30200, pbus_data);
            }
            break;
        default:
            ret = AIR_E_BAD_PARAMETER;
            break;
    }
    return ret;
}

/* FUNCTION NAME:   hal_an8811b_phy_setPhyLedCtrlMode
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
hal_an8811b_phy_setPhyLedCtrlMode(
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

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (AN8811B_LED_0_CFG_CTRL_MODE_BIT + (AN8811B_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(AN8811B_LED_CFG_CTRL_MODE_MASK, offset, AN8811B_LED_CFG_CTRL_MODE_WIDTH));
        data |= BITS_OFF_L(ctrl_mode, offset, AN8811B_LED_CFG_CTRL_MODE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        is_force = (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode) ? TRUE : FALSE;
        if (TRUE == is_force)
        {
            rv = hal_an8811b_phy_getPhyLedForceState(unit, port, led_id, &state);
            if (AIR_E_OK == rv)
            {
                rv = hal_an8811b_phy_setPhyLedForceState(unit, port, led_id, state);
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

                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_1000, AN8811B_LED_LINK_1000, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_100, AN8811B_LED_LINK_100, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_10, AN8811B_LED_LINK_10, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_FULLDPLX, AN8811B_LED_LINK_FULLDPLX, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_HALFDPLX, AN8811B_LED_LINK_HALFDPLX, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_HIGH_ACTIVE, AN8811B_LED_POL_HIGH_ACT, link_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_LINK_2500, AN8811B_LED_LINK_2500, link_reg_data);

                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_1000, AN8811B_LED_BLINK_TX_1000, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_1000, AN8811B_LED_BLINK_RX_1000, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_100, AN8811B_LED_BLINK_TX_100, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_100, AN8811B_LED_BLINK_RX_100, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_10, AN8811B_LED_BLINK_TX_10, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_10, AN8811B_LED_BLINK_RX_10, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_TX_2500, AN8811B_LED_BLINK_TX_2500, blk_reg_data);
                GET_PHY_ABILITY(led_config, AN8811B_LED_BHV_BLINK_RX_2500, AN8811B_LED_BLINK_RX_2500, blk_reg_data);

                if (link_reg_data != 0 || blk_reg_data != 0)
                {
                    link_reg_data |= AN8811B_LED_FUNC_ENABLE;
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
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8811B_LED_ON_DURATION,
                                                     (AN8811B_LED_BLINK_RATE_DEFAULT / 2));
                        rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8811B_LED_BLINK_DURATION,
                                                      AN8811B_LED_BLINK_RATE_DEFAULT);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getPhyLedCtrlMode
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
hal_an8811b_phy_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    UI16_T         data = 0, offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (AN8811B_LED_0_CFG_CTRL_MODE_BIT + (AN8811B_LED_CFG_RG_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(data, offset, AN8811B_LED_CFG_CTRL_MODE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_setPhyLedForceState
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
hal_an8811b_phy_setPhyLedForceState(
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

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (AN8811B_LED_0_CFG_STATE_BIT + (AN8811B_LED_CFG_RG_OFFSET * led_id));
        data &= ~(BITS_OFF_L(AN8811B_LED_CFG_STATE_MASK, offset, AN8811B_LED_CFG_STATE_WIDTH));
        data |= BITS_OFF_L(state, offset, AN8811B_LED_CFG_STATE_WIDTH);
        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_an8811b_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if (AIR_E_OK == rv)
        {
            if (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode)
            {
                is_force = (HAL_PHY_LED_STATE_FORCE_PATT == state) ? TRUE : FALSE;
                if (TRUE == is_force)
                {
                    rv = hal_an8811b_phy_getPhyLedForcePattCfg(unit, port, led_id, &led_patt);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_an8811b_phy_setPhyLedForcePattCfg(unit, port, led_id, led_patt);
                    }
                }
                else
                {
                    (HAL_PHY_LED_STATE_ON == state ? (reg_data = AN8811B_LED_LINK_FORCE_ON) : (reg_data = 0));
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr,
                                                 (reg_data | AN8811B_LED_FUNC_ENABLE));
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

/* FUNCTION NAME:   hal_an8811b_phy_getPhyLedForceState
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
hal_an8811b_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    UI16_T         data = 0, offset = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        offset = (AN8811B_LED_0_CFG_STATE_BIT + (AN8811B_LED_CFG_RG_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(data, offset, AN8811B_LED_CFG_STATE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_setPhyLedForcePattCfg
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
hal_an8811b_phy_setPhyLedForcePattCfg(
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
    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    switch (pattern)
    {
        case HAL_PHY_LED_PATT_HZ_HALF:
            reg_data = AN8811B_LED_BLINK_RATE_HZ_HALF;
            break;
        case HAL_PHY_LED_PATT_HZ_ONE:
            reg_data = AN8811B_LED_BLINK_RATE_HZ_ONE;
            break;
        case HAL_PHY_LED_PATT_HZ_TWO:
            reg_data = AN8811B_LED_BLINK_RATE_HZ_TWO;
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
            data &= ~(BITS_OFF_L(AN8811B_LED_CFG_PATT_MASK, AN8811B_LED_CFG_PATT_BIT, AN8811B_LED_CFG_PATT_WIDTH));
            data |= BITS_OFF_L(pattern, AN8811B_LED_CFG_PATT_BIT, AN8811B_LED_CFG_PATT_WIDTH);
            rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, data);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = hal_an8811b_phy_getPhyLedCtrlMode(unit, port, led_id, &ctrl_mode);
        if ((AIR_E_OK == rv) && (HAL_PHY_LED_CTRL_MODE_FORCE == ctrl_mode))
        {
            rv = hal_an8811b_phy_getPhyLedForceState(unit, port, led_id, &led_state);
            if ((AIR_E_OK == rv) && (HAL_PHY_LED_STATE_FORCE_PATT == led_state))
            {
                rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8811B_LED_ON_DURATION, (reg_data / 2));
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, AN8811B_LED_BLINK_DURATION, reg_data);
                if (AIR_E_OK == rv)
                {
                    LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_BLK, reg_addr);
                    rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, AN8811B_LED_BLINK_FORCE);
                    if (AIR_E_OK == rv)
                    {
                        LED_ID_TO_LED_REG_ADDRESS(led_id, PHY_LED_TYPE_ON, reg_addr);
                        reg_data = AN8811B_LED_FUNC_ENABLE;
                        led_behavior.value = 0xFFFF;
                        led_behavior.param0 = port;
                        led_behavior.param1 = led_id;

                        hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
                        if (led_behavior.value & AN8811B_LED_BHV_HIGH_ACTIVE)
                        {
                            reg_data |= AN8811B_LED_POL_HIGH_ACT;
                        }
                        rv = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, reg_data);
                    }
                }
            }
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_an8811b_phy_getPhyLedForcePattCfg
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
hal_an8811b_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    UI16_T         data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, PHY_LED_CFG_REG, &data);
    if (AIR_E_OK == rv)
    {
        *ptr_pattern = BITS_OFF_R(data, AN8811B_LED_CFG_PATT_BIT, AN8811B_LED_CFG_PATT_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME: hal_an8811b_phy_setPhyLedGlbCfg
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
hal_an8811b_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_getPhyLedGlbCfg
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
hal_an8811b_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedGlbCfg(unit, port, ptr_glb_cfg);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_setPhyLedBlkEvent
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
hal_an8811b_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedBlkEvent(unit, port, led_id, evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_getPhyLedBlkEvent
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
hal_an8811b_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    HAL_CHECK_PARAM((led_id >= HAL_AN8811B_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedBlkEvent(unit, port, led_id, ptr_evt_flags);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_setPhyLedDuration
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
hal_an8811b_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_setPhyLedDuration(unit, port, mode, time);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_getPhyLedDuration
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
hal_an8811b_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    ret = hal_cmn_phy_getPhyLedDuration(unit, port, mode, ptr_time);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_dumpPhyPara
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
hal_an8811b_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI16_T                reg_data = 0;
    UI32_T                pbus_data = 0;
    HAL_PHY_LINK_STATUS_T link_status;

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
    rv = hal_mdio_readC22ByPort(unit, port, 0x1d, &reg_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, 0x1d, reg_data);
    }
    osal_printf("\n=== buck pbus reg ===\n");
    rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x3b3c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u     an8811b version           = %x\n", unit, port, pbus_data);
    }
    rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5ce004, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("unit=%2u    port=%2u     an8811b Tx, Rx Polarity   = %s, ", unit, port,
                    (pbus_data & AN8811B_TX_POLARITY_NORMAL) ? "Tx Normal" : "Tx Reverse");
    }
    rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5ce61c, &pbus_data);
    if (AIR_E_OK == rv)
    {
        osal_printf("%s\n", (pbus_data & AN8811B_RX_POLARITY_NORMAL) ? "Rx Normal" : "Rx Reverse");
    }

    rv = hal_an8811b_phy_getLinkStatus(unit, port, &link_status);
    if (AIR_E_OK == rv)
    {
        osal_printf("\n=== system side counter ===\n");
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c902c, &pbus_data);
        pbus_data |= 0x03; /* toggle before read */
        rv |= _hal_an8811b_buck_pbus_regwr(unit, port, 0x5c902c, pbus_data);
        if (AIR_E_OK == rv)
        {
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90bc, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     rx star packets           = %u\n", unit, port, pbus_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90c0, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     rx terminal packets       = %u\n", unit, port, pbus_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90c4, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     rx error packets          = %u\n", unit, port, pbus_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90b0, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     tx star packets           = %u\n", unit, port, pbus_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90b4, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     tx terminal packets       = %u\n", unit, port, pbus_data);
            }
            rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x5c90b8, &pbus_data);
            if (AIR_E_OK == rv)
            {
                osal_printf("unit=%2u    port=%2u     tx error packets          = %u\n", unit, port, pbus_data);
            }
        }
        pbus_data = (1 << 2); /* toggle before read */
        rv = _hal_an8811b_buck_pbus_regwr(unit, port, 0x5c902c, pbus_data);
        osal_printf("[MIB counter]\n");
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214208, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx unicast packets        = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21420c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx multicast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214210, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx broadcast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214220, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx deferred event         = %u\n", unit, port, pbus_data);
        }

        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214280, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx drop packets           = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214284, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx filtering packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214288, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx unicast packets        = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21428c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx multicast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214290, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx broadcast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214294, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx alignment error packets= %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214298, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx FCS error packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21429c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx undersize packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2142a0, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx fragment error packets = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2142a4, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx oversize packets       = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2142a8, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx jabber error packets   = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2142ac, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx pause packets          = %u\n", unit, port, pbus_data);
        }
    }
    if (AIR_E_OK == rv)
    {
        if (link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
        {
            osal_printf("\n=== line side counter ===\n");
            if (HAL_PHY_SPEED_2500M == link_status.speed)
            {
                osal_printf("[2.5G]\n");
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x3072c, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, rx side_s      = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30730, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, rx side_t      = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30728, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, rx dec         = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30710, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, tx side_s      = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30720, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, tx side_t      = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30724, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     before ef, tx enc         = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30764, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     after ef, rx side_s       = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30768, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     after ef, rx side_t       = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30734, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     after ef, tx side_s       = %u\n", unit, port, pbus_data);
                }
                rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x30738, &pbus_data);
                if (AIR_E_OK == rv)
                {
                    osal_printf("unit=%2u    port=%2u     after ef, tx side_t       = %u\n", unit, port, pbus_data);
                }
            }
            else /* none 2.5G speed */
            {
                rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_EXTREG);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_readC22ByPort(unit, port, 0x12, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("unit=%2u    port=%2u     rx                        = %u\n", unit, port,
                                    reg_data & 0x7fff);
                    }
                    rv = hal_mdio_readC22ByPort(unit, port, 0x17, &reg_data);
                    if (AIR_E_OK == rv)
                    {
                        osal_printf("unit=%2u    port=%2u     rx error                  = %u\n", unit, port,
                                    reg_data & 0xff);
                    }
                }
                rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_REG);
                if (AIR_E_OK == rv)
                {
                    rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_CL22_Page_TrReg);
                    if (AIR_E_OK == rv)
                    {
                        rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_RgAddr_TrReg10h, 0xbf92);
                        if (AIR_E_OK == rv)
                        {
                            rv = hal_mdio_readC22ByPort(unit, port, AN8811B_RgAddr_TrReg11h, &reg_data);
                            if (AIR_E_OK == rv)
                            {
                                osal_printf("unit=%2u    port=%2u     tx                        = %u\n", unit, port,
                                            (reg_data & 0x7ffe) >> 1);
                                rv = hal_mdio_readC22ByPort(unit, port, AN8811B_RgAddr_TrReg12h, &reg_data);
                                if (AIR_E_OK == rv)
                                {
                                    osal_printf("unit=%2u    port=%2u     tx error                  = %u\n", unit, port,
                                                (reg_data & 0x7f));
                                }
                            }
                        }
                    }
                }
            }
        }
        rv = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_REG);
        osal_printf("[MIB counter]\n");
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214000, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx collision drop packets = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214008, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx unicast packets        = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21400c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx multicast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214010, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx broadcast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214014, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx collision event packets= %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214018, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx single collision packet= %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21401c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx multiple collision     = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214020, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx deferred event         = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214024, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx late collision         = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214028, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx excessive collision    = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21402c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Tx pause packets          = %u\n", unit, port, pbus_data);
        }

        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214080, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx drop packets           = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214084, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx filtering packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214088, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx unicast packets        = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21408c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx multicast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214090, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx broadcast packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214094, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx alignment error packets= %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x214098, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx FCS error packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21409c, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx undersize packets      = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2140a0, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx fragment error packets = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2140a4, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx oversize packets       = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2140a8, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx jabber error packets   = %u\n", unit, port, pbus_data);
        }
        rv = _hal_an8811b_buck_pbus_regrd(unit, port, 0x2140ac, &pbus_data);
        if (AIR_E_OK == rv)
        {
            osal_printf("unit=%2u    port=%2u     Rx pause packets          = %u\n", unit, port, pbus_data);
        }
    }

    return rv;
}

/* FUNCTION NAME: hal_an8811b_phy_triggerLinkDownCableTest
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
hal_an8811b_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    ret = hal_cmn_phy_triggerLinkDownCableTest(unit, port, ptr_test_rslt);
    return ret;
}

/* FUNCTION NAME: hal_an8811b_phy_dumpDebugInfo
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
hal_an8811b_phy_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         pbus_data = 0;
    UI16_T         reg_data = 0, speed_msb = 0, speed_lsb = 0;

    osal_printf("[External status] ========================================================\n");
    rc = _hal_an8811b_buck_pbus_regrd(unit, port, 0x21c100, &pbus_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" reg 0x0021c100=%x\n", port, pbus_data);
        osal_printf("  ls-link = %u              ls-speed = %u                duplex = %u\n",
                    BITS_OFF_R(pbus_data, 30, 1), BITS_OFF_R(pbus_data, 24, 3), BITS_OFF_R(pbus_data, 28, 1));
        osal_printf("  ls-txfc = %u               ls-rxfc = %u\n", BITS_OFF_R(pbus_data, 20, 1),
                    BITS_OFF_R(pbus_data, 18, 3));
    }
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

    rc = hal_mdio_writeC22ByPort(unit, port, AN8811B_PHY_MII_PAGE_SELECT, AN8811B_PHY_CL22_PAGE_EXTREG);
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
    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR_2G5, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl45 phy_addr/dev/reg_addr/data = 0x%02x/0x%02x/0x%02x/0x%04x\n",
                    HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), MMD_DEV_ANEG, MMD_EEEAR_2G5, reg_data);
        osal_printf("  2g5_t_eee = %u\n", BITS_OFF_R(reg_data, 0, 1));
    }
    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR_2G5, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl45 phy_addr/dev/reg_addr/data = 0x%02x/0x%02x/0x%02x/0x%04x\n",
                    HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), MMD_DEV_ANEG, MMD_EEELPAR_2G5, reg_data);
        osal_printf("  2g5_t_eee = %u\n", BITS_OFF_R(reg_data, 0, 1));
    }
    /* 2.5G */
    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, &reg_data);
    if (AIR_E_OK == rc)
    {
        osal_printf(" cl45 phy_addr/dev/reg_addr/data = 0x%02x/0x%02x/0x%02x/0x%04x\n",
                    HAL_PHY_PORT_DEV_PHY_ADDR(unit, port), MMD_DEV_ANEG, MULTIG_ANAR, reg_data);
        osal_printf("  adv_2g5 = %u          \n", BITS_OFF_R(reg_data, 7, 1));
    }
    return rc;
}

const HAL_PHY_DRIVER_T _ext_AN8811B_phy_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_an8811b_phy_init,
    hal_an8811b_phy_setAdminState,
    hal_an8811b_phy_getAdminState,
    hal_an8811b_phy_setAutoNego,
    hal_an8811b_phy_getAutoNego,
    hal_an8811b_phy_setLocalAdvAbility,
    hal_an8811b_phy_getLocalAdvAbility,
    hal_an8811b_phy_getRemoteAdvAbility,
    hal_an8811b_phy_setSpeed,
    hal_an8811b_phy_getSpeed,
    hal_an8811b_phy_setDuplex,
    hal_an8811b_phy_getDuplex,
    hal_an8811b_phy_getLinkStatus,
    hal_an8811b_phy_setLoopBack,
    hal_an8811b_phy_getLoopBack,
    hal_an8811b_phy_setSmartSpeedDown,
    hal_an8811b_phy_getSmartSpeedDown,
    hal_an8811b_phy_setLedOnCtrl,
    hal_an8811b_phy_getLedOnCtrl,
    hal_an8811b_phy_testTxCompliance,
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    NULL, /* setSerdesMode */
    NULL, /* getSerdesMode */
    hal_an8811b_phy_setPhyLedCtrlMode,
    hal_an8811b_phy_getPhyLedCtrlMode,
    hal_an8811b_phy_setPhyLedForceState,
    hal_an8811b_phy_getPhyLedForceState,
    hal_an8811b_phy_setPhyLedForcePattCfg,
    hal_an8811b_phy_getPhyLedForcePattCfg,
    NULL, /* triggerCableTest */
    NULL, /* getCableTestRawData */
    hal_an8811b_phy_setPhyLedGlbCfg,
    hal_an8811b_phy_getPhyLedGlbCfg,
    hal_an8811b_phy_setPhyLedBlkEvent,
    hal_an8811b_phy_getPhyLedBlkEvent,
    hal_an8811b_phy_setPhyLedDuration,
    hal_an8811b_phy_getPhyLedDuration,
    NULL, /* setPhyOpMode */
    NULL, /* getPhyOpMode */
    hal_an8811b_phy_dumpPhyPara,
    NULL, /* triggerLinkDownCableTest */
    NULL, /* dumpPortCnt */
    hal_an8811b_phy_dumpDebugInfo};

AIR_ERROR_NO_T
hal_an8811b_phy_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_ext_AN8811B_phy_func_vec;

    return (AIR_E_OK);
}
