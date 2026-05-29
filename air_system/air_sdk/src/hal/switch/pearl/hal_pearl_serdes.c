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

/* FILE NAME:  hal_pearl_serdes.c
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_serdes.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/pearl/hal_pearl_ifmon.h>
#include <hal/switch/pearl/hal_pearl_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_SERDES_SFP_MAX_LED_COUNT   (2)
#define HAL_PEARL_SERDES_5GBASER_RETRY_COUNT (50)

/* MACRO FUNCTION DECLARATIONS
 */

#define HAL_PEARL_SERDES_AML_READ_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)      \
    do                                                                                        \
    {                                                                                         \
        __RETURN__ = aml_readReg(__UNIT__, __REG__, __DATA__, __SIZE__);                      \
        if (__RETURN__ != AIR_E_OK)                                                           \
        {                                                                                     \
            DIAG_PRINT(HAL_DBG_WARN, "read reg 0x%08X fail, rv = %d\n", __REG__, __RETURN__); \
            return __RETURN__;                                                                \
        }                                                                                     \
    } while (0)

#define HAL_PEARL_SERDES_AML_WRITE_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)                     \
    do                                                                                                        \
    {                                                                                                         \
        __RETURN__ = aml_writeReg(__UNIT__, __REG__, __DATA__, __SIZE__);                                     \
        if (__RETURN__ != AIR_E_OK)                                                                           \
        {                                                                                                     \
            DIAG_PRINT(HAL_DBG_WARN, "write reg 0x%08X value to 0x%08X fail rv = %d\n", __REG__, *(__DATA__), \
                       __RETURN__);                                                                           \
            return __RETURN__;                                                                                \
        }                                                                                                     \
    } while (0)

#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__)                                     \
    do                                                                                                     \
    {                                                                                                      \
        __base_addr__ = HAL_PEARL_SERDES_ID_S0_BASE_ADDR + (HAL_PEARL_SERDES_ADDR_OFFSER * __serdes_id__); \
    } while (0)

#define GET_PCS_RX_SYNC(__data__, __value__)                                                              \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_PCS_RX_SYNC_OFFSET)) & HAL_PEARL_SERDES_PCS_RX_SYNC_MASK)
#define GET_SGMII_AN_5_LINK_STATUS(__data__, __value__)                                             \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_AN5_LINK_OFFSET)) & HAL_PEARL_SERDES_AN5_LINK_MASK)
#define GET_SGMII_AN_5_LINK_SPEED(__data__, __value__)                                                \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_AN5_SPEED_OFFSET)) & HAL_PEARL_SERDES_AN5_SPEED_MASK)
#define GET_SGMII_AN_5_LINK_DUPLEX(__data__, __value__)                                                 \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_AN5_DUPLEX_OFFSET)) & HAL_PEARL_SERDES_AN5_DUPLEX_MASK)
#define GET_SGMII_AN_0_AN_ENABLE(__data__, __value__)                                                   \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_AN0_ENABLE_OFFSET)) & HAL_PEARL_SERDES_AN0_ENABLE_MASK)
#define GET_HSGMII_RX_CTRL_SIGDET(__data__, __value__)                                                          \
    __value__ = ((__data__ >> (HAL_PEARL_SERDES_SIGDET_CAL_RDY_OFFSET)) & HAL_PEARL_SERDES_SIGDET_CAL_RDY_MASK)

#define GET_PCS_RX_AN_DONE(__data__, __value__)   __value__ = (__data__ & HAL_PEARL_SERDES_PCS_RX_AN_DONE)
#define GET_PCS_INTR_AN_DONE(__data__, __value__) __value__ = (__data__ & HAL_PEARL_SERDES_PCS_INTR_AN_DONE)

#define CLEAR_SERDES_AN_DONE_INTR(__unit__, __reg_base__)                                                        \
    do                                                                                                           \
    {                                                                                                            \
        rv = aml_readReg(__unit__, (__reg_base__ + HAL_PEARL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        if (AIR_E_OK == rv)                                                                                      \
        {                                                                                                        \
            data |= HAL_PEARL_SERDES_AN_DONE_CLEAR;                                                              \
            aml_writeReg(__unit__, (__reg_base__ + HAL_PEARL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
            data &= ~(HAL_PEARL_SERDES_AN_DONE_CLEAR);                                                           \
            aml_writeReg(__unit__, (__reg_base__ + HAL_PEARL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        }                                                                                                        \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_pearl_serdes.c");

extern const HAL_PEARL_SERDES_CFG_INFO_T     _hal_pearl_serdes_cfg_info[];
extern const UI32_T                          _hal_pearl_serdes_cfg_info_size;
extern const HAL_PEARL_SERDES_CFG_T          _hal_pearl_serdes_set5GbaserCfg_pll_config[];
extern const UI32_T                          _hal_pearl_serdes_pll_config_cnt;
extern const HAL_PEARL_SERDES_CFG_T          _hal_pearl_serdes_set5GbaserCfg_pll_kbandCode[];
extern const UI32_T                          _hal_pearl_serdes_pll_kbandCode;
extern const HAL_PEARL_SERDES_CFG_T          _hal_pearl_serdes_set5gbaserCfg_pll_forceStable[];
extern const UI32_T                          _hal_pearl_serdes_pll_forceStable;

extern const UI32_T                          _hal_pearl_serdes_dbg_cfg_info_size;
extern const HAL_PEARL_SERDES_DBG_CFG_INFO_T _hal_pearl_serdes_dbg_cfg_info[];

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS */

static AIR_ERROR_NO_T
_hal_pearl_serdes_setRegister(
    const UI32_T                  unit,
    const HAL_PEARL_SERDES_ID_T   serdes_id,
    const HAL_PEARL_SERDES_CFG_T *ptr_serdes_cfg)
{
    UI32_T         base_addr, serdes_reg, serdes_regValue;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    if (ptr_serdes_cfg->delay_time)
    {
        osal_delayUs(ptr_serdes_cfg->delay_time * 1000);
    }

    serdes_reg = base_addr + ptr_serdes_cfg->reg_addr;

    HAL_PEARL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rv);

    serdes_regValue &=
        ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
    serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                  ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));

    HAL_PEARL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rv);
    DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_check_kband(
    const UI32_T                unit,
    const HAL_PEARL_SERDES_ID_T serdes_id)
{
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    UI32_T                  reg_addr = 0, reg_data = 0, base_addr;
    UI32_T                  j = 0;
    UI16_T                  tmpvalue1 = 0, tmpvalue2 = 0, tmpdata = 0;
    HAL_PEARL_SERDES_CFG_T *ptr_mac_port_map;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    osal_sleepTask(50);
    for (j = 0; j < (_hal_pearl_serdes_pll_kbandCode - 1); j++)
    {
        ptr_mac_port_map = (HAL_PEARL_SERDES_CFG_T *)&_hal_pearl_serdes_set5GbaserCfg_pll_kbandCode[j];
        rv = _hal_pearl_serdes_setRegister(unit, serdes_id, ptr_mac_port_map);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_setRegister fail rv = %d\n", rv);
            return rv;
        }
    }
    /* wait 5 ms */
    /* read PLL_VCOCAL_STATE_INTF */
    osal_sleepTask(5);
    reg_addr = base_addr + 0xE364;
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(reg_data), rv);
    tmpvalue1 = BITS_OFF_R(reg_data, 16, 8);

    /* set RG_QP_PLL_DEBUG_SEL = 1 */
    ptr_mac_port_map =
        (HAL_PEARL_SERDES_CFG_T *)&_hal_pearl_serdes_set5GbaserCfg_pll_kbandCode[(_hal_pearl_serdes_pll_kbandCode - 1)];
    rv = _hal_pearl_serdes_setRegister(unit, serdes_id, ptr_mac_port_map);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_setRegister fail rv = %d\n", rv);
        return rv;
    }

    /* wait 5 ms */
    /* read PLL_VCOCAL_STATE_INTF */
    osal_sleepTask(5);
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(reg_data), rv);
    tmpvalue2 = BITS_OFF_R(reg_data, 16, 8);

    tmpdata = (tmpvalue1 << 4) | tmpvalue2;
    if (0 != (tmpdata & 0x800) && (0xFFF != tmpdata))
    {
        rv = AIR_E_OK;
    }
    else /* KBAND_DONE = 0 or KBAND_Code = 0xFFF */
    {
        rv = AIR_E_OP_INCOMPLETE;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_configMode(
    const UI32_T                  unit,
    const HAL_PEARL_SERDES_ID_T   serdes_id,
    const HAL_PEARL_SERDES_MODE_T mode)
{
    UI32_T                  reg_addr = 0, reg_data = 0, base_addr;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    UI32_T                  i = 0, j = 0, retry = 0;
    HAL_PEARL_SERDES_CFG_T *ptr_mac_port_map;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    DIAG_PRINT(HAL_DBG_INFO, "config mode is %d\n", mode);
    for (i = 0; i < _hal_pearl_serdes_cfg_info_size; i++)
    {
        if (_hal_pearl_serdes_cfg_info[i].serdes_mode == mode)
        {
            for (j = 0; j < _hal_pearl_serdes_cfg_info[i].serdes_register_cnt; j++)
            {
                ptr_mac_port_map = (HAL_PEARL_SERDES_CFG_T *)(_hal_pearl_serdes_cfg_info[i].ptr_mac_port_map + j);
                rv = _hal_pearl_serdes_setRegister(unit, serdes_id, ptr_mac_port_map);
                if (rv != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_setRegister fail rv = %d\n", rv);
                    return rv;
                }
            }
            /* For 5gbaser corner ICO workaround */
            if (HAL_PEARL_SERDES_MODE_5GBASETR == mode)
            {
                for (retry = 0; retry < HAL_PEARL_SERDES_5GBASER_RETRY_COUNT; retry++)
                {
                    /* Check Kband Code Flow */
                    for (j = 0; j < _hal_pearl_serdes_pll_config_cnt; j++)
                    {
                        ptr_mac_port_map = (HAL_PEARL_SERDES_CFG_T *)&_hal_pearl_serdes_set5GbaserCfg_pll_config[j];
                        rv = _hal_pearl_serdes_setRegister(unit, serdes_id, ptr_mac_port_map);
                        if (AIR_E_OK != rv)
                        {
                            DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_setRegister fail rv = %d\n", rv);
                            return rv;
                        }
                    }
                    /* check pll KBand code */
                    if (AIR_E_OK == _hal_pearl_serdes_check_kband(unit, serdes_id))
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "check pll KBand code retry times = %d\n", retry);
                        break;
                    }
                }
                /* PLL Reset to default, PLL Force stable, TX Force Enable */
                for (j = 0; j < _hal_pearl_serdes_pll_forceStable; j++)
                {
                    ptr_mac_port_map = (HAL_PEARL_SERDES_CFG_T *)&_hal_pearl_serdes_set5gbaserCfg_pll_forceStable[j];
                    rv = _hal_pearl_serdes_setRegister(unit, serdes_id, ptr_mac_port_map);
                    if (AIR_E_OK != rv)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_setRegister fail rv = %d\n", rv);
                        return rv;
                    }
                }
            }
        }
    }

    if (HAL_PEARL_SERDES_MODE_5GBASETR != mode)
    {
        /* Reset port */
        reg_addr = HAL_PEARL_SERDES_NPSCU_BASE_ADDR + HAL_PEARL_SERDES_NPSCU_RST_CTRL2;

        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);
        reg_data = (reg_data | (0x2000 << serdes_id));
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

        osal_delayUs(5000);

        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);
        reg_data = (reg_data & ~(0x2000 << serdes_id));
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);
    }

    reg_addr = base_addr + HAL_PEARL_SERDES_MSG_RX_LIK_STS_2;

    /* Bypass link status to MAC */
    if (HAL_PEARL_SERDES_MODE_HSGMII == mode)
    {
        reg_data = 0x0fff;
    }
    else
    {
        reg_data = 0xe;
    }
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_configPolarity(
    const UI32_T                      unit,
    const HAL_PEARL_SERDES_ID_T       serdes_id,
    const HAL_PEARL_SERDES_POLARITY_T polarity)
{
    UI32_T         reg_addr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         reg_txInv = 0, reg_rxInv = 0;
    UI32_T         swap_tx = 0, swap_rx = 0;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);
    reg_txInv = reg_addr + HAL_PEARL_SERDES_TX_DA_CTRL_1;
    reg_rxInv = reg_addr + HAL_PEARL_SERDES_RX_AFIFO;

    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_rxInv, &swap_rx, sizeof(UI32_T), rv);
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_txInv, &swap_tx, sizeof(UI32_T), rv);

    /* Tx polarity swap default value is 1, set tx to 0 if it requreies swapping */
    switch (polarity)
    {
        case HAL_PEARL_SERDES_POLARITY_NOSWAP:
            swap_tx |= HAL_PEARL_SERDES_TX_BIT_INV;
            swap_rx &= ~(HAL_PEARL_SERDES_RX_DATA_INV);
            break;
        case HAL_PEARL_SERDES_POLARITY_SWAP_TX:
            swap_tx &= ~(HAL_PEARL_SERDES_TX_BIT_INV);
            swap_rx &= ~(HAL_PEARL_SERDES_RX_DATA_INV);
            break;
        case HAL_PEARL_SERDES_POLARITY_SWAP_RX:
            swap_tx |= HAL_PEARL_SERDES_TX_BIT_INV;
            swap_rx |= HAL_PEARL_SERDES_RX_DATA_INV;
            break;
        case HAL_PEARL_SERDES_POLARITY_SWAP_TXRX:
            swap_tx &= ~(HAL_PEARL_SERDES_TX_BIT_INV);
            swap_rx |= HAL_PEARL_SERDES_RX_DATA_INV;
            break;
        default:
            /* do nothing */
            break;
    }
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_rxInv, &swap_rx, sizeof(UI32_T), rv);
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_txInv, &swap_tx, sizeof(UI32_T), rv);

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_setting(
    const UI32_T                      unit,
    const HAL_PEARL_SERDES_ID_T       serdes_id,
    const HAL_PEARL_SERDES_MODE_T     mode,
    const HAL_PEARL_SERDES_POLARITY_T polarity)
{
    UI32_T         data = 0x12340000 | mode, base_addr, reg_data;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    HAL_PEARL_SERDES_AML_WRITE_REG(unit, 0x10005010, &data, sizeof(UI32_T), rv);

    /* Enable serdes HAL_PEARL_SERDES_SYS_CTRL_1, RG_FORCE_RG_VUSB10_ON_SEL[16] = 1*/
    HAL_PEARL_SERDES_AML_READ_REG(unit, (base_addr + HAL_PEARL_SERDES_SYS_CTRL_1), &reg_data, sizeof(UI32_T), rv);
    reg_data |= HAL_PEARL_SERDES_FORCE_RG_VUSB10_ON_SEL;
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, (base_addr + HAL_PEARL_SERDES_SYS_CTRL_1), &reg_data, sizeof(UI32_T), rv);

    if ((serdes_id >= HAL_PEARL_SERDES_ID_LAST) || (mode >= HAL_PEARL_SERDES_MODE_LAST) ||
        (polarity >= HAL_PEARL_SERDES_POLARITY_LAST))
    {
        return AIR_E_BAD_PARAMETER;
    }

    rv = _hal_pearl_serdes_configMode(unit, serdes_id, mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_configMode fail rv = %d\n", rv);
        return rv;
    }
    rv = _hal_pearl_serdes_configPolarity(unit, serdes_id, polarity);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_pearl_serdes_configPolarity fail rv = %d\n", rv);
        return rv;
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_getPortMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_PEARL_SERDES_MODE_T *ptr_mode)
{
    HAL_SDK_PORT_MAP_T     *ptr_sdk_port_map_entry;
    HAL_PEARL_SERDES_MODE_T serdes_mode = HAL_PEARL_SERDES_MODE_DEFAULT_VALUE;
    HAL_PEARL_SERDES_ID_T   serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                  air_port = 0, count = 0;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

    if (AIR_INIT_PORT_TYPE_XSGMII == ptr_sdk_port_map_entry->port_type)
    {
        if (AIR_INIT_PORT_SPEED_2500M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_PEARL_SERDES_MODE_HSGMII;
        }
        else if (AIR_INIT_PORT_SPEED_5000M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_PEARL_SERDES_MODE_5GBASETR;
        }

        serdes_id = ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (HAL_PEARL_SERDES_MODE_DEFAULT_VALUE == serdes_mode)
    {
        AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), air_port)
        {
            ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, air_port));

            if (AIR_INIT_PORT_TYPE_CPU == ptr_sdk_port_map_entry->port_type ||
                AIR_INIT_PORT_TYPE_BASET == ptr_sdk_port_map_entry->port_type ||
                AIR_INIT_PORT_TYPE_ENHANCED_BASET == ptr_sdk_port_map_entry->port_type)
            {
                continue;
            }
            if (serdes_id == ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id)
            {
                count++;
            }
        }
        serdes_mode = HAL_PEARL_SERDES_MODE_SGMII_AN;
    }
    *ptr_mode = serdes_mode;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_checkChangeMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_PEARL_SERDES_MODE_T *ptr_mode,
    BOOL_T                  *ptr_change)
{
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    HAL_PHY_LINK_STATUS_T link_status;
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI32_T                base_addr = 0, serdes_speed = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    rv = hal_phy_getLinkStatus(unit, port, &link_status);
    if (rv == AIR_E_OK)
    {
        if (link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
        {
            rv = aml_readReg(unit, (base_addr + HAL_PEARL_SERDES_DIG_MODE_CTRL_1), &serdes_speed, sizeof(serdes_speed));
            if (rv == AIR_E_OK)
            {
                serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
                if ((HAL_PEARL_SERDES_TPHY_SPEED_SGMII == serdes_speed) && (HAL_PHY_SPEED_2500M == link_status.speed))
                {
                    *ptr_mode = HAL_PEARL_SERDES_MODE_HSGMII;
                    *ptr_change = TRUE;
                }
                else if ((HAL_PEARL_SERDES_TPHY_SPEED_HSGMII == serdes_speed) &&
                         (HAL_PHY_SPEED_2500M != link_status.speed))
                {
                    *ptr_mode = HAL_PEARL_SERDES_MODE_SGMII_AN;
                    *ptr_change = TRUE;
                }
                else
                {
                    *ptr_change = FALSE;
                }
            }
        }
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_changeMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const HAL_PEARL_SERDES_MODE_T mode)
{
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                base_addr, reg_data;
    AIR_ERROR_NO_T        rv = AIR_E_OK;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
    /* phya auto init reg = 0*/
    HAL_PEARL_SERDES_AML_READ_REG(unit, (base_addr + HAL_PEARL_SERDES_PLL_CTRL_0), &reg_data, sizeof(reg_data), rv);

    reg_data &= ~(HAL_PEARL_SERDES_PHYA_AUTO_INIT);

    HAL_PEARL_SERDES_AML_WRITE_REG(unit, (base_addr + HAL_PEARL_SERDES_PLL_CTRL_0), &reg_data, sizeof(reg_data), rv);

    rv = _hal_pearl_serdes_configMode(unit, serdes_id, HAL_PEARL_SERDES_MODE_DEFAULT_VALUE);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "reset port %u serdes register default value failed\n", port);
        return rv;
    }

    rv = _hal_pearl_serdes_configMode(unit, serdes_id, mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "config port %u serdes mode %x failed\n", port, mode);
        return rv;
    }

    return rv;
}
static void
serdes_set_rate_adaption(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI32_T                phy_id, base_addr, reg_data, serdes_speed;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    HAL_PHY_LINK_STATUS_T link_status;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

    /* get link speed */
    rv = hal_phy_getLinkStatus(unit, port, &link_status);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "get port %u link status failed\n", port);
        return;
    }

    /* check current serdes type is not HSGMII, this work around only work on SGMII and QSGMII */
    rv = aml_readReg(unit, (base_addr + HAL_PEARL_SERDES_DIG_MODE_CTRL_1), &serdes_speed, sizeof(serdes_speed));
    if (AIR_E_OK != rv)
    {
        return;
    }

    serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
    if (HAL_PEARL_SERDES_TPHY_SPEED_HSGMII != serdes_speed)
    {
        /* MULTIPHY_RATE_ADP_P0_CTRL_REG_0 */
        if (link_status.speed == HAL_PHY_SPEED_1000M)
        {
            rv = aml_readReg(unit, (base_addr + (HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                             &reg_data, sizeof(reg_data));

            reg_data &= ~(0xf);
            rv = aml_writeReg(unit, (base_addr + (HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                              &reg_data, sizeof(reg_data));
            if (AIR_E_OK != rv)
            {
                return;
            }
        }

        /* MULTIPHY_MII_RA_AN_ENABLE */
        rv = aml_readReg(unit, (base_addr + HAL_PEARL_SERDES_MII_RA_AN_ENABLE), &reg_data, sizeof(reg_data));
        if (AIR_E_OK != rv)
        {
            return;
        }

        if ((HAL_PHY_SPEED_1000M == link_status.speed) && (link)) /* Link up & speed is 1G, set force rate adaption */
        {
            reg_data &= ~(0x1 << ((phy_id) % 4));
        }
        else /* set rate adaption controlled by AN */
        {
            reg_data |= (0x1 << ((phy_id) % 4));
        }
        rv = aml_writeReg(unit, (base_addr + HAL_PEARL_SERDES_MII_RA_AN_ENABLE), &reg_data, sizeof(reg_data));
        if (AIR_E_OK != rv)
        {
            return;
        }
    }
}

static void
_hal_pearl_serdes_linkChangeCallback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    HAL_PHY_DUPLEX_T        duplex;
    HAL_PHY_LINK_STATUS_T   link_status;
    HAL_PEARL_SERDES_ID_T   serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI16_T                  mac_port = 0, eee_data = 0;
    UI32_T                  base_addr = 0, reg_addr = 0, data = 0, pre_data = 0, eee_serdes = 0;
    UI32_T                  phy_id = 0, shift = 0, serdes_speed = 0;
    HAL_PEARL_SERDES_MODE_T serdes_mode = HAL_PEARL_SERDES_MODE_DEFAULT_VALUE;
    BOOL_T                  need_change = FALSE;
    UI32_T                  sgmii_sts_ctrl = 0, force_linkdown = 0;
    AIR_CFG_VALUE_T         serdes_op_mode;
    AIR_PORT_STATUS_T       port_status;

    osal_memset(&port_status, 0, sizeof(AIR_PORT_STATUS_T));
    if (HAL_IS_XSGMII_PORT_VALID(unit, port))
    {
        osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
        serdes_op_mode.param0 = port;
        hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

        if (HAL_PHY_SERDES_OP_MODE_SPEED_CAHNGE == serdes_op_mode.value)
        {
            if (link)
            {
                rv = _hal_pearl_serdes_checkChangeMode(unit, port, &serdes_mode, &need_change);
                if ((rv == AIR_E_OK) && (need_change == TRUE))
                {
                    rv = _hal_pearl_serdes_changeMode(unit, port, serdes_mode);
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Set port %u, serdes mode to %u failed\n", port, serdes_mode);
                        return;
                    }
                }
            }
        }

        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        reg_addr = HAL_PEARL_SERDES_PORT_MCR_BASE_ADDR + (mac_port * HAL_PEARL_SERDES_PORT_ADDR_OFFSET);

        rv = aml_readReg(unit, reg_addr, &data, sizeof(data));
        if (rv != AIR_E_OK)
        {
            return;
        }

        rv = hal_pearl_ifmon_getPortStatus(unit, port, &port_status);
        if (AIR_E_OK != rv)
        {
            return;
        }

        if (data & BIT(PEARL_MCR_FORCE_MODE_OFFT))
        {
            /* handle serdes rate adaption mode in force mode */
            serdes_set_rate_adaption(unit, port, link);
            /* force mode don't need to update flow control status */
            return;
        }
        else
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
            reg_addr = base_addr + HAL_PEARL_SERDES_MSG_RX_LIK_STS_0;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(data));
            if (rv != AIR_E_OK)
            {
                return;
            }
            pre_data = data;

            serdes_set_rate_adaption(unit, port, link);
            /* eee status */
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            sgmii_sts_ctrl = (base_addr + HAL_PEARL_SERDES_SGMII_STS_CTRL_0);
            rv = aml_readReg(unit, (base_addr + (HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                             &eee_serdes, sizeof(eee_serdes));
            if (rv != AIR_E_OK)
            {
                return;
            }
            if (link)
            {
                if (port_status.flags & AIR_PORT_STATUS_FLAGS_EEE)
                {
                    eee_serdes |= (HAL_PEARL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_PEARL_SERDES_RA_P0_MII_TX_EEE_EN);
                }
                else
                {
                    eee_serdes &= ~(HAL_PEARL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_PEARL_SERDES_RA_P0_MII_TX_EEE_EN);
                }
            }
            else
            {
                eee_data = 0;
                eee_serdes &= ~(HAL_PEARL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_PEARL_SERDES_RA_P0_MII_TX_EEE_EN);
            }
            /* In order to bypass LPI to MAC for 100M, it should set mii tx/rx eee enable bits
               in RATE_ADP_PN_CTRL_REG_N */
            rv = aml_writeReg(unit, (base_addr + (HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                              &eee_serdes, sizeof(eee_serdes));
            if (rv != AIR_E_OK)
            {
                return;
            }

            if (link) /* Update side band information when port is link up */
            {
                /* check current serdes type is QSGMII or not */
                rv = aml_readReg(unit, (base_addr + HAL_PEARL_SERDES_DIG_MODE_CTRL_1), &serdes_speed,
                                 sizeof(serdes_speed));
                if (rv != AIR_E_OK)
                {
                    return;
                }
                serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
                /* 5gbaser no need to update sideband for EEE and FC */
                if (HAL_PEARL_SERDES_TPHY_SPEED_BASER != serdes_speed)
                {
                    shift = 0;
                    data &= ~(HAL_PEARL_SERDES_HSGMII_LINK_STS_MASK << shift);
                    /* PHY flow control enabled */
                    if (port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX)
                    {
                        data |= (HAL_PEARL_SERDES_HSGMII3_FC_TX_ON << shift);
                    }

                    if (port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)
                    {
                        data |= (HAL_PEARL_SERDES_HSGMII3_FC_RX_ON << shift);
                    }

                    rv = hal_phy_getLinkStatus(unit, port, &link_status);
                    if (rv != AIR_E_OK)
                    {
                        return;
                    }
                    duplex = link_status.duplex;
                    if (HAL_PHY_DUPLEX_FULL == duplex)
                    {
                        data |= (HAL_PEARL_SERDES_FULL_DUPLEX << shift);
                    }
                    data |= (eee_data << (2 + shift));
                    if (pre_data == data)
                    {
                        return;
                    }

                    force_linkdown = (0x2 << shift);
                    rv = aml_writeReg(unit, sgmii_sts_ctrl, &force_linkdown, sizeof(force_linkdown));
                    if (rv != AIR_E_OK)
                    {
                        return;
                    }
                    rv = aml_writeReg(unit, reg_addr, &data, sizeof(data));
                    if (rv != AIR_E_OK)
                    {
                        return;
                    }

                    force_linkdown = 0x0;
                    rv = aml_writeReg(unit, sgmii_sts_ctrl, &force_linkdown, sizeof(force_linkdown));
                    if (rv != AIR_E_OK)
                    {
                        return;
                    }
                }
            }
        }
    }
}

static AIR_ERROR_NO_T
_hal_pearl_serdes_dumpDbgRegister(
    const UI32_T                      unit,
    const HAL_PEARL_SERDES_ID_T       serdes_id,
    const HAL_PEARL_SERDES_DBG_CFG_T *ptr_serdes_cfg)
{
    UI32_T         base_addr, serdes_reg, serdes_regValue;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    if (ptr_serdes_cfg->delay_time)
    {
        osal_delayUs(ptr_serdes_cfg->delay_time * 1000);
    }

    serdes_reg = base_addr + ptr_serdes_cfg->reg_addr;

    if (1 == ptr_serdes_cfg->operation) /* write */
    {
        HAL_PEARL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        serdes_regValue &=
            ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                      ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    }
    else /* read */
    {
        HAL_PEARL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        osal_printf("%35s       0x%8x       0x%08x \n", ptr_serdes_cfg->reg_name, serdes_reg, serdes_regValue);
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES */

/* FUNCTION NAME:   hal_pearl_serdes_init
 * PURPOSE:
 *      Serdes initialization
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
hal_pearl_serdes_init(
    const UI32_T unit,
    const UI32_T port)
{
    HAL_PEARL_SERDES_MODE_T serdes_mode = HAL_PEARL_SERDES_MODE_DEFAULT_VALUE;
    AIR_ERROR_NO_T          rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T   serdes_id = HAL_PEARL_SERDES_ID_S0;
    AIR_CFG_VALUE_T         polarity;
    UI32_T                  reg_addr = 0, reg_data = 0, base_addr = 0;
    const UI16_T            shift = HSGMII_HSI0_RESET_BIT_OFFSET;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    rv = _hal_pearl_serdes_getPortMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    osal_memset(&polarity, 0, sizeof(AIR_CFG_VALUE_T));

    polarity.value = 0;
    polarity.param0 = port;
    rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE, &polarity);
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    /* Power up Serdes */
    reg_addr = HAL_PEARL_SERDES_NPSCU_BASE_ADDR + HAL_PEARL_SERDES_NPSCU_RST_CTRL2;
    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }
    reg_data &= ~(1 << (shift + serdes_id));
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

    rv = _hal_pearl_serdes_setting(unit, serdes_id, serdes_mode, polarity.value);
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    /* rx sync force on */
    reg_addr = base_addr + HAL_PEARL_SERDES_PCS_CTRL_REG_1;
    reg_data = SERDES_PCS_CTRL_REG_1_DEFAUT_VALUE;
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

    /* regsiter ifmon to know link change event */
    rv = hal_pearl_ifmon_lookup(unit, _hal_pearl_serdes_linkChangeCallback, NULL);
    if (rv == AIR_E_ENTRY_NOT_FOUND)
    {
        rv = hal_pearl_ifmon_register(unit, _hal_pearl_serdes_linkChangeCallback, NULL);
        if (rv != AIR_E_OK)
        {
            return AIR_E_NOT_INITED;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setAdminState
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
hal_pearl_serdes_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_base = 0;
    UI32_T                rx_ctrl_2_rg = 0, intf_ctrl_6_rg = 0, cl_ctrl_1_rg = 0;
    UI32_T                rx_ctrl_2_val = 0, intf_ctrl_6_val = 0, cl_ctrl_1_val = 0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    rx_ctrl_2_rg = reg_base + HAL_PEARL_SERDES_RX_CTRL_2;
    HAL_PEARL_SERDES_AML_READ_REG(unit, rx_ctrl_2_rg, &rx_ctrl_2_val, sizeof(UI32_T), rv);

    intf_ctrl_6_rg = reg_base + HAL_PEARL_SERDES_INTF_CTRL_6;
    HAL_PEARL_SERDES_AML_READ_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rv);

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        rx_ctrl_2_val &= ~(HAL_PEARL_SERDES_RX_PWD);
        intf_ctrl_6_val &= ~(HAL_PEARL_SERDES_TX_DATA_EN_SEL);
        intf_ctrl_6_val &= ~(HAL_PEARL_SERDES_TX_DATA_EN_FORCE);
    }
    else
    {
        rx_ctrl_2_val |= HAL_PEARL_SERDES_RX_PWD;
        intf_ctrl_6_val |= HAL_PEARL_SERDES_TX_DATA_EN_SEL;
        intf_ctrl_6_val &= ~(HAL_PEARL_SERDES_TX_DATA_EN_FORCE);
    }
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, rx_ctrl_2_rg, &rx_ctrl_2_val, sizeof(UI32_T), rv);
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rv);

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        cl_ctrl_1_rg = reg_base + HAL_PEARL_SERDES_PLL_CL_CTRL_1;
        HAL_PEARL_SERDES_AML_READ_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rv);

        cl_ctrl_1_val |= (HAL_PEARL_SERDES_TX_DATA_EN_FORCE | HAL_PEARL_SERDES_TX_DATA_EN_SEL);
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rv);

        cl_ctrl_1_val &= ~(HAL_PEARL_SERDES_TX_DATA_EN_FORCE | HAL_PEARL_SERDES_TX_DATA_EN_SEL);
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rv);

        reg_addr = reg_base + HAL_PEARL_SERDES_AN_REG_0;
        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);
        data |= HAL_PEARL_SERDES_AN_REG_0_RESET;
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getAdminState
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
hal_pearl_serdes_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

    reg_addr = reg_addr + HAL_PEARL_SERDES_RX_CTRL_2;
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    if (!!(data & HAL_PEARL_SERDES_RX_PWD))
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
    }
    else
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setAutoNego
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
 *      AIR_E_OP_INVALID
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, reg_base = 0;
    UI32_T                data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;

    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if (HAL_IS_CASCADE_PORT_VALID(unit, port))
    {
        return AIR_E_OP_INVALID;
    }

    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %d, serdes mode read fail !\n", port);
        return rv;
    }
    if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
    {
        return AIR_E_NOT_SUPPORT;
    }

    reg_addr = reg_base + HAL_PEARL_SERDES_AN_REG_0;

    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        data |= HAL_PEARL_SERDES_AN_REG_0_ANENABLE;
    }
    else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
    {
        data |= HAL_PEARL_SERDES_AN_REG_0_ANRESTART;
    }
    else
    {
        data &= ~(HAL_PEARL_SERDES_AN_REG_0_ANENABLE);
    }

    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    if (port_cfg.value == 1)
    {
        /* For SGMII Force mode to AN mode (exclude HSGMII)*/
        if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
        {
            rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
            if (rv == AIR_E_OK)
            {
                if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
                {
                    rv = _hal_pearl_serdes_changeMode(unit, port, HAL_PEARL_SERDES_MODE_SGMII_AN);
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_WARN, "write reg 0x%08X value to 0x%08X fail, rv = %d\n", reg_addr, data,
                                   rv);
                        return rv;
                    }
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "write reg 0x%08X value to 0x%08X fail rv = %d\n", reg_addr, data, rv);
                return rv;
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getAutoNego
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
 *      AIR_E_OP_INVALID
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;

    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    if (HAL_IS_CASCADE_PORT_VALID(unit, port))
    {
        return AIR_E_OP_INVALID;
    }

    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %d, serdes mode read fail !\n", port);
        return rv;
    }
    if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
    {
        return AIR_E_NOT_SUPPORT;
    }

    reg_addr = serdes_base + HAL_PEARL_SERDES_AN_REG_13;

    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    if ((port_cfg.value == 1) && (HAL_PEARL_SERDES_SGMII_FORCE_MODE == (data & 0x3f))) /* Serdes force RA */
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
    }
    else
    {
        /* Read HAL_PEARL_SERDES_AN_REG_0  AN bits */
        reg_addr = serdes_base + HAL_PEARL_SERDES_AN_REG_0;
        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

        if (data & HAL_PEARL_SERDES_AN_REG_0_ANENABLE)
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

/* FUNCTION NAME:   hal_pearl_serdes_setLocalAdvAbility
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
hal_pearl_serdes_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if ((ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100FUDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100HFDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10HFDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10FUDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_2500M))
            {
                rv = AIR_E_NOT_SUPPORT;
            }
            else if (!(ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_1000FUDX))
            {
                rv = AIR_E_BAD_PARAMETER;
            }
            else
            {
                HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
                SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

                reg_addr = reg_addr + HAL_PEARL_SERDES_AN_REG_4;
                rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
                if (AIR_E_OK == rv)
                {
                    data &= ~(BIT(SGMII_1000BASEX_PAUSE_OFFSET));
                    data &= ~(BIT(SGMII_1000BASEX_ASM_DIR_OFFSET));
                    if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE)
                    {
                        data |= BIT(SGMII_1000BASEX_PAUSE_OFFSET);
                    }
                    if (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE)
                    {
                        data |= BIT(SGMII_1000BASEX_ASM_DIR_OFFSET);
                    }
                    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
                }
            }
        }
        else
        {
            rv = AIR_E_NOT_SUPPORT;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getLocalAdvAbility
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
hal_pearl_serdes_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_PEARL_SERDES_AN_REG_4;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rv)
            {
                ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
                ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
                if (data & BIT(SGMII_1000BASEX_PAUSE_OFFSET))
                {
                    ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE;
                }
                if (data & BIT(SGMII_1000BASEX_ASM_DIR_OFFSET))
                {
                    ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE;
                }
                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
            }
        }
        else if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_2500M;
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
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getRemoteAdvAbility
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
hal_pearl_serdes_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_PEARL_SERDES_AN_REG_5;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rv)
            {
                ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
                ptr_adv->flags &= ~(HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
                if (data & BIT(SGMII_1000BASEX_PAUSE_OFFSET))
                {
                    ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE;
                }
                if (data & BIT(SGMII_1000BASEX_ASM_DIR_OFFSET))
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
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setSpeed
 * PURPOSE:
 *      This API is used to set serdes speed.
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
 *      AIR_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    UI32_T                reg_data = 0, reg_addr = 0, reg_base = 0, auto_nego = 0;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI8_T                 force_speed = 0;
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    if (port_cfg.value == 2)
    {
        return AIR_E_NOT_SUPPORT;
    }

    if (HAL_PHY_SPEED_5000M == speed)
    {
        /* Not allowed speed config to 5000M */
        return AIR_E_OP_INVALID;
    }

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if (HAL_IS_CASCADE_PORT_VALID(unit, port))
    {
        return AIR_E_OP_INVALID;
    }

    if ((rv = hal_pearl_serdes_getAutoNego(unit, port, &auto_nego)) != AIR_E_OK)
    {
        return rv;
    }
    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %u an mode is enable, not allow force speed setting\n", port);
        return AIR_E_NOT_SUPPORT;
    }

    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %d, serdes mode read fail !\n", port);
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
    if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
    {
        if (HAL_PHY_SPEED_2500M != speed)
        {
            return AIR_E_OP_INVALID;
        }
        return rv;
    }

    if (port_cfg.value == 1)
    {
        switch (speed)
        {
            case HAL_PHY_SPEED_10M:
                force_speed = HAL_PEARL_SERDES_MODE_SGMII_FORCE_10M;
                break;
            case HAL_PHY_SPEED_100M:
                force_speed = HAL_PEARL_SERDES_MODE_SGMII_FORCE_100M;
                break;
            case HAL_PHY_SPEED_1000M:
                force_speed = HAL_PEARL_SERDES_MODE_SGMII_FORCE_1000M;
                break;
            case HAL_PHY_SPEED_2500M:
                force_speed = HAL_PEARL_SERDES_MODE_HSGMII;
                break;
            default:
                return AIR_E_BAD_PARAMETER;
        }
        /* Set serdes RA by speed  */
        rv = _hal_pearl_serdes_changeMode(unit, port, force_speed);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "set port %u, serdes mode to %u failed\n", port, force_speed);
            return rv;
        }

        /* sgmii reset phy */
        reg_addr = reg_base + HAL_PEARL_SERDES_AN_REG_0;
        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

        reg_data |= HAL_PEARL_SERDES_AN_REG_0_RESET;
        HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getSpeed
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
hal_pearl_serdes_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI32_T                reg_data = 0, reg_addr = 0, reg_base = 0;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI8_T                 force_speed = 0;
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if (HAL_IS_CASCADE_PORT_VALID(unit, port))
    {
        return AIR_E_OP_INVALID;
    }

    rv = hal_pearl_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %d, serdes mode read fail !\n", port);
        return rv;
    }
    if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
    {
        *ptr_speed = HAL_PHY_SPEED_1000M;
        return rv;
    }
    if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
    {
        *ptr_speed = HAL_PHY_SPEED_2500M;
        return rv;
    }
    if (port_cfg.value == 1)
    {
        reg_addr = reg_base + HAL_PEARL_SERDES_RATE_ADP_P0_CTRL_REG_0;
        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rv);

        force_speed = (reg_data & 0xf);

        if (force_speed == (HAL_PEARL_SERDES_RA_P0_MII_RA_RX_EN | HAL_PEARL_SERDES_RA_P0_MII_RA_TX_EN |
                            HAL_PEARL_SERDES_RA_P0_MII_RA_RX_MODE | HAL_PEARL_SERDES_RA_P0_MII_RA_TX_MODE))
        {
            *ptr_speed = HAL_PHY_SPEED_10M;
        }
        else if (force_speed == (HAL_PEARL_SERDES_RA_P0_MII_RA_RX_EN | HAL_PEARL_SERDES_RA_P0_MII_RA_TX_EN))
        {
            *ptr_speed = HAL_PHY_SPEED_100M;
        }
        else if (force_speed == (HAL_PEARL_SERDES_RA_P0_MII_RA_RX_EN))
        {
            *ptr_speed = HAL_PHY_SPEED_2500M;
        }
        else
        {
            *ptr_speed = HAL_PHY_SPEED_1000M;
        }
    }
    else if (port_cfg.value == 2)
    {
        return AIR_E_NOT_SUPPORT;
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getLinkStatus
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
hal_pearl_serdes_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI8_T                 link_status = 0, an_done = 0, ls_link = 0, an_intr = 0;
    UI8_T                 speed = 0, duplex = 0, an_enable = 0;
    UI32_T                serdes_speed = 0, sgmii_mode = 0;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_base = 0;
    UI32_T                data = 0, data2 = 0, pmcr = 0, force_mode = 0, mac_port = 0, force_speed = 0, data3 = 0;
    UI8_T                 sig_det = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    /* Get PMCR setting */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    HAL_PEARL_SERDES_AML_READ_REG(unit, PEARL_PMCR(mac_port), &pmcr, sizeof(UI32_T), rv);

    force_mode = !!(pmcr & BIT(PEARL_MCR_FORCE_MODE_OFFT));
    /* Get speed by serdes config */
    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_DIG_MODE_CTRL_1), &serdes_speed, sizeof(UI32_T),
                                  rv);

    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_AN_REG_13), &sgmii_mode, sizeof(UI32_T), rv);

    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_PCS_STATE_REG_2), &data, sizeof(UI32_T), rv);

    GET_PCS_RX_SYNC(data, link_status);
    GET_PCS_RX_AN_DONE(data, an_done);
    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_AN_REG_5), &data2, sizeof(UI32_T), rv);

    GET_SGMII_AN_5_LINK_STATUS(data2, ls_link);
    GET_SGMII_AN_5_LINK_SPEED(data2, speed);
    GET_SGMII_AN_5_LINK_DUPLEX(data2, duplex);

    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_AN_REG_0), &data3, sizeof(UI32_T), rv);
    GET_SGMII_AN_0_AN_ENABLE(data3, an_enable);

    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_RX_CTRL_14), &data3, sizeof(UI32_T), rv);
    GET_HSGMII_RX_CTRL_SIGDET(data3, sig_det);

    DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes an status check: an_enable=%d, an_done=%d, rx_sync=%d,\n", an_enable, an_done,
                   link_status);
    DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes link status check: link=%d, speed=%d, duplex=%d,\n", ls_link, speed, duplex);

    if (force_mode)
    {
        /* if sgmii_mode == 1000baseX & reg 0x0 bit 12 = 1, than check an-bits */
        if (link_status)
        {
            if ((!(sgmii_mode & HAL_PEARL_SERDES_SGMII_ENABLE)) & an_enable) /* for 1000baseX */
            {
                if (link_status & an_done)
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
                else
                {
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                }
            }
            else
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
            }
        }
        else
        {
            ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
        }

        if (port_cfg.value == 1) /* force rate adaotion */
        {
            serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
            rv = hal_pearl_serdes_getSpeed(unit, port, &force_speed);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "get serdes speed fail rv = %d\n", rv);
                return rv;
            }
            ptr_status->speed = force_speed;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
            /*port_cfg.value = 0, 2 return AIR_NOT_SUPPORT, the speed and dupelx
             * information decide by port api */
            return AIR_E_NOT_SUPPORT;
        }
    }
    else
    {
        serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
        if (HAL_PEARL_SERDES_TPHY_SPEED_HSGMII == serdes_speed)
        {
            if (link_status & sig_det)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }

            ptr_status->speed = HAL_PHY_SPEED_2500M;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else if (HAL_PEARL_SERDES_TPHY_SPEED_BASER == serdes_speed)
        {
            HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_10GB_T_PCS_STUS_1), &data2, sizeof(UI32_T),
                                          rv);

            link_status = !!(data2 & (HAL_PEARL_SERDES_10GB_T_RX_LINK_STUS | HAL_PEARL_SERDES_10GB_T_RX_BLK_LK));
            if (link_status)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }

            ptr_status->speed = HAL_PHY_SPEED_5000M;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else if (HAL_PEARL_SERDES_TPHY_SPEED_SGMII == serdes_speed)
        {
            if (link_status & an_done & ls_link)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE);
            }
            /* SGMII AN mode (get speed and duplex information from serdes AN result) */
            if (sgmii_mode & HAL_PEARL_SERDES_SGMII_ENABLE)
            {
                if (HAL_PEARL_SERDES_LINK_SPEED_1000M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_1000M;
                }
                else if (HAL_PEARL_SERDES_LINK_SPEED_100M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_100M;
                }
                else if (HAL_PEARL_SERDES_LINK_SPEED_10M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_10M;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "get speed fail rv = %d, value is 0x%08X\n", rv, speed);
                    return rv;
                }

                if (HAL_PEARL_SERDES_LINK_DUPLEX_FULL == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                }
                else if (HAL_PEARL_SERDES_LINK_DUPLEX_HALF == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "get duplex fail rv = %d, value is 0x%08X\n", rv, duplex);
                    return rv;
                }
            }
            else /* 1000baseX mode */
            {
                if (link_status & an_done)
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
                    /* get an done interrupt state */
                    HAL_PEARL_SERDES_AML_READ_REG(unit, (reg_base + HAL_PEARL_SERDES_PCS_INT_STATE_REG), &data,
                                                  sizeof(UI32_T), rv);

                    GET_PCS_INTR_AN_DONE(data, an_intr);
                    if (an_intr)
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                        CLEAR_SERDES_AN_DONE_INTR(unit, reg_base);
                    }
                }
                else
                {
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE);
                }

                /* update speed and duplex*/
                ptr_status->speed = HAL_PHY_SPEED_1000M;
                ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
            }
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "get serdes speed fail rv = %d, value is 0x%08X\n", rv, serdes_speed);
            return rv;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setSerdesMode
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
hal_pearl_serdes_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    /* Run serdes init script */

    if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode) /* HSGMII mode */
    {
        rv = _hal_pearl_serdes_changeMode(unit, port, HAL_PEARL_SERDES_MODE_HSGMII);
    }
    else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
    {
        rv = _hal_pearl_serdes_changeMode(unit, port, HAL_PEARL_SERDES_MODE_SGMII_AN);
    }
    else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
    {
        rv = _hal_pearl_serdes_changeMode(unit, port, HAL_PEARL_SERDES_MODE_1000BASEX);
    }
    else
    {
        /* Not support HAL_PHY_SERDES_MODE_100BASE_FX in AN8855 */
        rv = AIR_E_OP_INVALID;
    }

    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "set port %u, serdes mode to %u failed\n", port, serdes_mode);
        return rv;
    }
    /* serdes re-AN */
    reg_addr = serdes_base + HAL_PEARL_SERDES_AN_REG_0;
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    data |= BIT(PEARL_REG_SGMII_AN_RESTART_OFFT);
    HAL_PEARL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getSerdesMode
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
hal_pearl_serdes_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0, serdes_speed = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    reg_addr = serdes_base + HAL_PEARL_SERDES_DIG_MODE_CTRL_1;
    HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &serdes_speed, sizeof(UI32_T), rv);

    serdes_speed &= HAL_PEARL_SERDES_TPHY_SPEED_MASK;
    if (HAL_PEARL_SERDES_TPHY_SPEED_HSGMII == serdes_speed)
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_HSGMII;
    }
    else if (HAL_PEARL_SERDES_TPHY_SPEED_SGMII == serdes_speed)
    {
        reg_addr = serdes_base + HAL_PEARL_SERDES_AN_REG_13;
        HAL_PEARL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rv);

        if (data & HAL_PEARL_SERDES_SGMII_ENABLE)
        {
            *ptr_serdes_mode = HAL_PHY_SERDES_MODE_SGMII;
        }
        else
        {
            *ptr_serdes_mode = HAL_PHY_SERDES_MODE_1000BASE_X;
        }
    }
    else
    {
        rv = AIR_E_OP_INVALID;
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to set port led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      ctrl_mode       --  LED control mode
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
hal_pearl_serdes_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &=
            ~(BITS_OFF_L(HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_MASK, offset, HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_WIDTH));
        led_config |= BITS_OFF_L(ctrl_mode, offset, HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_WIDTH);
        rv = aml_writeReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedCtrlMode
 * PURPOSE:
 *      This API is used to get port led control mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *
 * OUTPUT:
 *      ptr_ctrl_mode   --  LED control mode
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(led_config, offset, HAL_PEARL_SERDES_SFP_LED_CTRL_MODE_WIDTH);
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedForceState
 * PURPOSE:
 *      This API is used to set port led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      state           --  LED force state
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
hal_pearl_serdes_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &= ~(
            BITS_OFF_L(HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_MASK, offset, HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_WIDTH));
        led_config |= BITS_OFF_L(state, offset, HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_WIDTH);
        rv = aml_writeReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedForceState
 * PURPOSE:
 *      This API is used to get port led force state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_state       --  LED force state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(led_config, offset, HAL_PEARL_SERDES_SFP_LED_FORCE_STATE_WIDTH);
    }

    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_setPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to set phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 *      pattern         --  LED force pattern
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
hal_pearl_serdes_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);

        offset = (HAL_PEARL_SERDES_SFP_LED_PATT_CFG_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &=
            ~(BITS_OFF_L(HAL_PEARL_SERDES_SFP_LED_PATT_CFG_MASK, offset, HAL_PEARL_SERDES_SFP_LED_PATT_CFG_WIDTH));
        led_config |= BITS_OFF_L(pattern, offset, HAL_PEARL_SERDES_SFP_LED_PATT_CFG_WIDTH);
        rv = aml_writeReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_getPhyLedForcePattCfg
 * PURPOSE:
 *      This API is used to get phy led force pattern.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED Id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_serdes_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PEARL_SERDES_ID_T serdes_id = HAL_PEARL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_PEARL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_PEARL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_PEARL_SERDES_SFP_LED_PATT_CFG_BIT + (HAL_PEARL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_PEARL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_pattern = BITS_OFF_R(led_config, offset, HAL_PEARL_SERDES_SFP_LED_PATT_CFG_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_pearl_serdes_dumpPortCnt
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
hal_pearl_serdes_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI16_T                      idx, rg_idx;
    HAL_PEARL_SERDES_ID_T       serdes_id = HAL_PEARL_SERDES_ID_S0;
    HAL_PEARL_SERDES_DBG_CFG_T *ptr_serdes_dbg_list;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    osal_printf("%25s        %16s        %9s \n", "Register", "Addr", "Value");
    for (idx = 0; idx < _hal_pearl_serdes_dbg_cfg_info_size; idx++)
    {
        if (_hal_pearl_serdes_dbg_cfg_info[idx].serdes_interface & param)
        {
            for (rg_idx = 0; rg_idx < _hal_pearl_serdes_dbg_cfg_info[idx].serdes_register_cnt; rg_idx++)
            {
                ptr_serdes_dbg_list =
                    (HAL_PEARL_SERDES_DBG_CFG_T *)(_hal_pearl_serdes_dbg_cfg_info[idx].ptr_serdes_dbg_cfg_map + rg_idx);

                rc = _hal_pearl_serdes_dumpDbgRegister(unit, serdes_id, ptr_serdes_dbg_list);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set _hal_pearl_serdes_dumpDbgRegister fail, rc = %d\n", rc);
                    break;
                }
            }
        }
    }

    return rc;
}

const static HAL_PHY_DRIVER_T _int_pearl_serdes_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_pearl_serdes_init,
    hal_pearl_serdes_setAdminState,
    hal_pearl_serdes_getAdminState,
    hal_pearl_serdes_setAutoNego,
    hal_pearl_serdes_getAutoNego,
    hal_pearl_serdes_setLocalAdvAbility,
    hal_pearl_serdes_getLocalAdvAbility,
    hal_pearl_serdes_getRemoteAdvAbility,
    hal_pearl_serdes_setSpeed,
    hal_pearl_serdes_getSpeed,
    NULL, /* setDuplex */
    NULL, /* getDuplex */
    hal_pearl_serdes_getLinkStatus,
    NULL, /* getLoopBack */
    NULL, /* setLoopBack */
    NULL, /* setSmartSpeedDown */
    NULL, /* getSmartSpeedDown */
    NULL, /* setLedOnCtrl */
    NULL, /* getLedOnCtrl */
    NULL, /* testTxCompliance*/
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    hal_pearl_serdes_setSerdesMode,
    hal_pearl_serdes_getSerdesMode,
    hal_pearl_serdes_setPhyLedCtrlMode,
    hal_pearl_serdes_getPhyLedCtrlMode,
    hal_pearl_serdes_setPhyLedForceState,
    hal_pearl_serdes_getPhyLedForceState,
    hal_pearl_serdes_setPhyLedForcePattCfg,
    hal_pearl_serdes_getPhyLedForcePattCfg,
    NULL, /* triggerCableTest */
    NULL, /* getCableTestRawData */
    NULL, /* setPhyLedGlbCfg */
    NULL, /* getPhyLedGlbCfg */
    NULL, /* setPhyLedBlkEvent */
    NULL, /* getPhyLedBlkEvent */
    NULL, /* setPhyLedDuration */
    NULL, /* getPhyLedDuration */
    NULL, /* setPhyOpMode */
    NULL, /* getPhyOpMode */
    NULL, /* dumpPhyPara */
    NULL, /* triggerLinkDownCableTest */
    hal_pearl_serdes_dumpPortCnt,
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_pearl_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_int_pearl_serdes_func_vec;

    return (AIR_E_OK);
}
