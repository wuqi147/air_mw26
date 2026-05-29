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

/* FILE NAME:  hal_coral_serdes.c
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_serdes.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/coral/hal_coral_ifmon.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_SERDES_SFP_MAX_LED_COUNT (2)

/* MACRO FUNCTION DECLARATIONS
 */

#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__)                           \
    do                                                                                           \
    {                                                                                            \
        __base_addr__ = (HAL_CORAL_SERDES_ID_S0 == __serdes_id__) ? (0x10240000) : (0x10220000); \
    } while (0)

#define HAL_CORAL_SERDES_AML_READ_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)     \
    do                                                                                       \
    {                                                                                        \
        __RETURN__ = aml_readReg(__UNIT__, __REG__, __DATA__, __SIZE__);                     \
        if (AIR_E_OK != __RETURN__)                                                          \
        {                                                                                    \
            DIAG_PRINT(HAL_DBG_ERR, "read reg 0x%08X fail, rc = %d\n", __REG__, __RETURN__); \
            return __RETURN__;                                                               \
        }                                                                                    \
    } while (0)

#define HAL_CORAL_SERDES_AML_WRITE_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)                    \
    do                                                                                                       \
    {                                                                                                        \
        __RETURN__ = aml_writeReg(__UNIT__, __REG__, __DATA__, __SIZE__);                                    \
        if (AIR_E_OK != __RETURN__)                                                                          \
        {                                                                                                    \
            DIAG_PRINT(HAL_DBG_ERR, "write reg 0x%08X value to 0x%08X fail rc = %d\n", __REG__, *(__DATA__), \
                       __RETURN__);                                                                          \
            return __RETURN__;                                                                               \
        }                                                                                                    \
    } while (0)

#define GET_PCS_INTR_AN_DONE(__data__, __value__) __value__ = (__data__ & HAL_CORAL_SERDES_PCS_INTR_AN_DONE)

#define CLEAR_SERDES_AN_DONE_INTR(__unit__, __reg_base__)                                                        \
    do                                                                                                           \
    {                                                                                                            \
        rc = aml_readReg(__unit__, (__reg_base__ + HAL_CORAL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        if (AIR_E_OK == rc)                                                                                      \
        {                                                                                                        \
            data |= HAL_CORAL_SERDES_AN_DONE_CLEAR;                                                              \
            aml_writeReg(__unit__, (__reg_base__ + HAL_CORAL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
            data &= ~(HAL_CORAL_SERDES_AN_DONE_CLEAR);                                                           \
            aml_writeReg(__unit__, (__reg_base__ + HAL_CORAL_SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        }                                                                                                        \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
extern const UI32_T                          _hal_coral_serdes_qp_dbg_cfg_info_size;
extern const HAL_CORAL_SERDES_DBG_CFG_INFO_T _hal_coral_serdes_qp_dbg_cfg_info[];

extern const UI32_T                          _hal_coral_serdes_pxp_dbg_cfg_info_size;
extern const HAL_CORAL_SERDES_DBG_CFG_INFO_T _hal_coral_serdes_pxp_dbg_cfg_info[];

extern const UI32_T                          _hal_coral_serdes_qp_cfg_info_size;
extern const HAL_CORAL_SERDES_CFG_INFO_T     _hal_coral_serdes_qp_cfg_info[];

extern const UI32_T                          _hal_coral_serdes_pxp_cfg_info_size;
extern const HAL_CORAL_SERDES_CFG_INFO_T     _hal_coral_serdes_pxp_cfg_info[];

extern const UI32_T                          _hal_coral_serdes_pxp_setLck2Data_patch_size;
extern const HAL_CORAL_SERDES_CFG_T          _hal_coral_serdes_pxp_setLck2Data_patch[];

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PORT, "hal_coral_serdes.c");

/* LOCAL SUBPROGRAM BODIES
 */

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_setRegister(
    const UI32_T                  unit,
    const HAL_CORAL_SERDES_ID_T   serdes_id,
    const HAL_CORAL_SERDES_CFG_T *ptr_serdes_cfg)
{
    UI32_T         serdes_reg, serdes_regValue;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (ptr_serdes_cfg->delay_time)
    {
        osal_delayUs(ptr_serdes_cfg->delay_time * 1000);
    }

    serdes_reg = ptr_serdes_cfg->reg_addr;

    HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);

    serdes_regValue &=
        ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));

    serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                  ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));

    HAL_CORAL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_configMode(
    const UI32_T                  unit,
    const HAL_CORAL_SERDES_ID_T   serdes_id,
    const HAL_CORAL_SERDES_MODE_T mode)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  i = 0, j = 0;
    HAL_CORAL_SERDES_CFG_T *ptr_mac_port_map;

    DIAG_PRINT(HAL_DBG_INFO, "config mode is %d\n", mode);
    for (i = 0; i < _hal_coral_serdes_qp_cfg_info_size; i++)
    {
        if (mode == _hal_coral_serdes_qp_cfg_info[i].serdes_mode)
        {
            for (j = 0; j < _hal_coral_serdes_qp_cfg_info[i].serdes_register_cnt; j++)
            {
                ptr_mac_port_map = (HAL_CORAL_SERDES_CFG_T *)(_hal_coral_serdes_qp_cfg_info[i].ptr_mac_port_map + j);
                rc = _hal_coral_serdes_qp_setRegister(unit, serdes_id, ptr_mac_port_map);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_qp_setRegister fail rc = %d\n", rc);
                    return rc;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_configPolarity(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_POLARITY_T polarity)
{
    UI32_T         reg_addr = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_txInv = 0, reg_rxInv = 0;
    UI32_T         swap_tx = 0, swap_rx = 0;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);
    reg_txInv = reg_addr + HAL_CORAL_SERDES_TX_DA_CTRL_1;
    reg_rxInv = reg_addr + HAL_CORAL_SERDES_RX_AFIFO;

    HAL_CORAL_SERDES_AML_READ_REG(unit, reg_rxInv, &swap_rx, sizeof(UI32_T), rc);
    HAL_CORAL_SERDES_AML_READ_REG(unit, reg_txInv, &swap_tx, sizeof(UI32_T), rc);

    /* Tx polarity swap default value is 1, set tx to 0 if it requreies swapping */
    switch (polarity)
    {
        case HAL_CORAL_SERDES_POLARITY_NOSWAP:
            swap_tx |= HAL_CORAL_SERDES_TX_BIT_INV;
            swap_rx &= ~(HAL_CORAL_SERDES_RX_DATA_INV);
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_TX:
            swap_tx &= ~(HAL_CORAL_SERDES_TX_BIT_INV);
            swap_rx &= ~(HAL_CORAL_SERDES_RX_DATA_INV);
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_RX:
            swap_tx |= HAL_CORAL_SERDES_TX_BIT_INV;
            swap_rx |= HAL_CORAL_SERDES_RX_DATA_INV;
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_TXRX:
            swap_tx &= ~(HAL_CORAL_SERDES_TX_BIT_INV);
            swap_rx |= HAL_CORAL_SERDES_RX_DATA_INV;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            DIAG_PRINT(HAL_DBG_ERR, "unexpected priority value = %d, rc = %d\n", polarity, rc);
            break;
    }
    if (AIR_E_OK == rc)
    {
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_rxInv, &swap_rx, sizeof(UI32_T), rc);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_txInv, &swap_tx, sizeof(UI32_T), rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_setting(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_MODE_T     mode,
    const HAL_CORAL_SERDES_POLARITY_T polarity)
{
    UI32_T         data = 0x12340000 | mode;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CORAL_SERDES_AML_WRITE_REG(unit, 0x10005010, &data, sizeof(UI32_T), rc);

    if ((serdes_id >= HAL_CORAL_SERDES_ID_LAST) || (mode >= HAL_CORAL_SERDES_MODE_LAST) ||
        (polarity >= HAL_CORAL_SERDES_POLARITY_LAST))
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = _hal_coral_serdes_qp_configMode(unit, serdes_id, mode);
    if (rc != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_qp_configMode fail rc = %d\n", rc);
        return rc;
    }
    rc = _hal_coral_serdes_qp_configPolarity(unit, serdes_id, polarity);
    if (rc != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_qp_configPolarity fail rc = %d\n", rc);
        return rc;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_getPortMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_CORAL_SERDES_MODE_T *ptr_mode)
{
    HAL_SDK_PORT_MAP_T     *ptr_sdk_port_map_entry;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    HAL_CORAL_SERDES_ID_T   serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                  air_port = 0, count = 0;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

    if (AIR_INIT_PORT_TYPE_XSGMII == ptr_sdk_port_map_entry->port_type)
    {
        if (AIR_INIT_PORT_SPEED_2500M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_CORAL_SERDES_MODE_HSGMII;
        }
        else if (AIR_INIT_PORT_SPEED_5000M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_CORAL_SERDES_MODE_5GBASETR;
        }

        serdes_id = ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (HAL_CORAL_SERDES_MODE_DEFAULT_VALUE == serdes_mode)
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
        serdes_mode = HAL_CORAL_SERDES_MODE_SGMII_AN;
    }
    *ptr_mode = serdes_mode;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_getEEELineSideAnStatus(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_status)
{
    UI16_T         eee_adv = 0, eee_lp = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &eee_adv);
    if (rc == AIR_E_OK)
    {
        rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &eee_lp);
        if (rc == AIR_E_OK)
        {
            *ptr_status = (eee_adv & eee_lp) & (EEE_1000BASE_T | EEE_100BASE_TX);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_checkChangeMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_CORAL_SERDES_MODE_T *ptr_mode,
    BOOL_T                  *ptr_change)
{
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_PHY_LINK_STATUS_T link_status;
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                base_addr = 0, serdes_speed = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    rc = hal_phy_getLinkStatus(unit, port, &link_status);
    if (rc == AIR_E_OK)
    {
        if (link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
        {
            rc = aml_readReg(unit, (base_addr + HAL_CORAL_SERDES_DIG_MODE_CTRL_1), &serdes_speed, sizeof(serdes_speed));
            if (rc == AIR_E_OK)
            {
                serdes_speed &= HAL_CORAL_SERDES_TPHY_SPEED_MASK;
                if ((HAL_CORAL_SERDES_TPHY_SPEED_SGMII == serdes_speed) && (HAL_PHY_SPEED_2500M == link_status.speed))
                {
                    *ptr_mode = HAL_CORAL_SERDES_MODE_HSGMII;
                    *ptr_change = TRUE;
                }
                else if ((HAL_CORAL_SERDES_TPHY_SPEED_HSGMII == serdes_speed) &&
                         (HAL_PHY_SPEED_2500M != link_status.speed))
                {
                    *ptr_mode = HAL_CORAL_SERDES_MODE_SGMII_AN;
                    *ptr_change = TRUE;
                }
                else
                {
                    *ptr_change = FALSE;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_changeMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const HAL_CORAL_SERDES_MODE_T mode)
{
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                base_addr, reg_data;
    UI32_T                intf_ctrl_6_rg = 0, cl_ctrl_1_rg = 0;
    UI32_T                intf_ctrl_6_val = 0, cl_ctrl_1_val = 0;
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    intf_ctrl_6_rg = base_addr + HAL_CORAL_SERDES_INTF_CTRL_6;
    cl_ctrl_1_rg = base_addr + HAL_CORAL_SERDES_PLL_CL_CTRL_1;

    /* Tx_Disable = 1*/
    HAL_CORAL_SERDES_AML_READ_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rc);
    intf_ctrl_6_val |= HAL_CORAL_SERDES_TX_DATA_EN_SEL;
    intf_ctrl_6_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_FORCE);
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rc);

    /* PLL force Unstable for PMA */
    HAL_CORAL_SERDES_AML_READ_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rc);
    cl_ctrl_1_val |= HAL_CORAL_SERDES_TX_LFPS_EN_SEL;
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rc);

    /* phya auto init reg = 0*/
    HAL_CORAL_SERDES_AML_READ_REG(unit, (base_addr + HAL_CORAL_SERDES_PLL_CTRL_0), &reg_data, sizeof(reg_data), rc);
    reg_data &= ~(HAL_CORAL_SERDES_PHYA_AUTO_INIT);
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, (base_addr + HAL_CORAL_SERDES_PLL_CTRL_0), &reg_data, sizeof(reg_data), rc);

    rc = _hal_coral_serdes_qp_configMode(unit, serdes_id, HAL_CORAL_SERDES_MODE_DEFAULT_VALUE);
    if (AIR_E_OK == rc)
    {
        rc = _hal_coral_serdes_qp_configMode(unit, serdes_id, mode);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "config port %u serdes mode %x failed\n", port, mode);
        }

        /* PLL force stable for PMA */
        HAL_CORAL_SERDES_AML_READ_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rc);
        cl_ctrl_1_val |= (HAL_CORAL_SERDES_TX_LFPS_EN_SEL | HAL_CORAL_SERDES_TX_LFPS_EN_FORCE);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rc);

        cl_ctrl_1_val &= ~(HAL_CORAL_SERDES_TX_LFPS_EN_SEL | HAL_CORAL_SERDES_TX_LFPS_EN_FORCE);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, cl_ctrl_1_rg, &cl_ctrl_1_val, sizeof(UI32_T), rc);
        /* Tx disable = 0 */
        HAL_CORAL_SERDES_AML_READ_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rc);
        intf_ctrl_6_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_SEL);
        intf_ctrl_6_val |= HAL_CORAL_SERDES_TX_DATA_EN_FORCE;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rc);
        intf_ctrl_6_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_FORCE);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, intf_ctrl_6_rg, &intf_ctrl_6_val, sizeof(UI32_T), rc);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "reset port %u serdes register default value failed\n", port);
    }

    return rc;
}

static void
_hal_coral_serdes_qp_linkChangeCallback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T   serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI16_T                  eee_data = 0;
    UI32_T                  base_addr = 0, eee_serdes = 0;
    UI32_T                  phy_id = 0;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    BOOL_T                  need_change = FALSE;
    AIR_CFG_VALUE_T         serdes_op_mode;

    if (HAL_IS_XSGMII_PORT_VALID(unit, port))
    {
        osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
        serdes_op_mode.param0 = port;
        hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

        if (HAL_PHY_SERDES_OP_MODE_SPEED_CAHNGE == serdes_op_mode.value)
        {
            if (link)
            {
                rc = _hal_coral_serdes_qp_checkChangeMode(unit, port, &serdes_mode, &need_change);
                if ((rc == AIR_E_OK) && (need_change == TRUE))
                {
                    rc = _hal_coral_serdes_qp_changeMode(unit, port, serdes_mode);
                    if (rc != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Set port %u, serdes mode to %u failed\n", port, serdes_mode);
                        return;
                    }
                }
            }
        }
        if (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port))
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

            /* eee status */
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

            rc = aml_readReg(unit, (base_addr + (HAL_CORAL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                             &eee_serdes, sizeof(eee_serdes));
            if (AIR_E_OK == rc)
            {
                if (link)
                {
                    rc = _hal_coral_serdes_qp_getEEELineSideAnStatus(unit, port, &eee_data);
                    if ((AIR_E_OK == rc) && (eee_data))
                    {
                        eee_serdes |= (HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
                    }
                    else
                    {
                        eee_serdes &= ~(HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
                    }
                }
                else
                {
                    eee_data = 0;
                    eee_serdes &= ~(HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
                }
                /* In order to bypass LPI to MAC for 100M, it should set mii tx/rx eee enable bits
                   in RATE_ADP_PN_CTRL_REG_N */
                rc = aml_writeReg(unit, (base_addr + (HAL_CORAL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                                  &eee_serdes, sizeof(eee_serdes));
                if (AIR_E_OK != rc)
                {
                    return;
                }
            }
        }
    }
}

static AIR_ERROR_NO_T
_hal_coral_serdes_qp_dumpDbgRegister(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_DBG_CFG_T *ptr_serdes_cfg)
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
        HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        serdes_regValue &=
            ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                      ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    }
    else /* read */
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        osal_printf("%30s       0x%8x       0x%08x \n", ptr_serdes_cfg->reg_name, serdes_reg, serdes_regValue);
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_serdes_qp_init
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
hal_coral_serdes_qp_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    HAL_CORAL_SERDES_ID_T   serdes_id = HAL_CORAL_SERDES_ID_S0;
    AIR_CFG_VALUE_T         polarity;
    UI32_T                  reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);

    HAL_CORAL_SERDES_AML_READ_REG(unit, HAL_CORAL_SERDES_RG_ELDO_NDIS_EN, &reg_data, sizeof(UI32_T), rc);
    reg_data &= ~(BIT(HAL_CORAL_SERDES_RG_CFG_EXT_OFFSET));
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, HAL_CORAL_SERDES_RG_ELDO_NDIS_EN, &reg_data, sizeof(UI32_T), rc);

    rc = _hal_coral_serdes_qp_getPortMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        osal_memset(&polarity, 0, sizeof(AIR_CFG_VALUE_T));

        polarity.value = 0;
        polarity.param0 = port;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE, &polarity);
        if (AIR_E_OK == rc)
        {
            rc = _hal_coral_serdes_qp_setting(unit, serdes_id, serdes_mode, polarity.value);
            if (AIR_E_OK == rc)
            {
                /* regsiter ifmon to know link change event */
                rc = hal_coral_ifmon_lookup(unit, _hal_coral_serdes_qp_linkChangeCallback, NULL);
                if (AIR_E_ENTRY_NOT_FOUND == rc)
                {
                    rc = hal_coral_ifmon_register(unit, _hal_coral_serdes_qp_linkChangeCallback, NULL);
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_setAdminState
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
hal_coral_serdes_qp_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_base = 0;
    UI32_T                rx_ctrl_2_val = 0, intf_ctrl_6_val = 0, cl_ctrl_1_val = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_2), &rx_ctrl_2_val, sizeof(UI32_T),
                                      rc);
        rx_ctrl_2_val &= ~(HAL_CORAL_SERDES_RX_PWD);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_2), &rx_ctrl_2_val, sizeof(UI32_T),
                                       rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PLL_CL_CTRL_1), &cl_ctrl_1_val, sizeof(UI32_T),
                                      rc);
        cl_ctrl_1_val |= (HAL_CORAL_SERDES_TX_DATA_EN_FORCE | HAL_CORAL_SERDES_TX_DATA_EN_SEL);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_PLL_CL_CTRL_1), &cl_ctrl_1_val,
                                       sizeof(UI32_T), rc);
        cl_ctrl_1_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_FORCE | HAL_CORAL_SERDES_TX_DATA_EN_SEL);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_PLL_CL_CTRL_1), &cl_ctrl_1_val,
                                       sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_INTF_CTRL_6), &intf_ctrl_6_val, sizeof(UI32_T),
                                      rc);
        intf_ctrl_6_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_FORCE | HAL_CORAL_SERDES_TX_DATA_EN_SEL);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_INTF_CTRL_6), &intf_ctrl_6_val,
                                       sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &data, sizeof(UI32_T), rc);
        data |= HAL_CORAL_SERDES_AN_REG_0_RESET;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &data, sizeof(UI32_T), rc);
    }
    else
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_2), &rx_ctrl_2_val, sizeof(UI32_T),
                                      rc);
        rx_ctrl_2_val |= HAL_CORAL_SERDES_RX_PWD;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_2), &rx_ctrl_2_val, sizeof(UI32_T),
                                       rc);
        // osal_delayUs(500 * 1000);
        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_INTF_CTRL_6), &intf_ctrl_6_val, sizeof(UI32_T),
                                      rc);
        intf_ctrl_6_val &= ~(HAL_CORAL_SERDES_TX_DATA_EN_FORCE);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_INTF_CTRL_6), &intf_ctrl_6_val,
                                       sizeof(UI32_T), rc);
        intf_ctrl_6_val |= HAL_CORAL_SERDES_TX_DATA_EN_SEL;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_INTF_CTRL_6), &intf_ctrl_6_val,
                                       sizeof(UI32_T), rc);
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getAdminState
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
hal_coral_serdes_qp_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

    reg_addr = reg_addr + HAL_CORAL_SERDES_RX_CTRL_2;
    HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

    if (!!(data & HAL_CORAL_SERDES_RX_PWD))
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
    }
    else
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_setAutoNego
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
hal_coral_serdes_qp_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);
    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if ((HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode) || (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
        {
            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_0;

            HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

            if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
            {
                data |= HAL_CORAL_SERDES_AN_REG_0_ANENABLE;
            }
            else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
            {
                data |= HAL_CORAL_SERDES_AN_REG_0_ANRESTART;
            }
            else
            {
                data &= ~(HAL_CORAL_SERDES_AN_REG_0_ANENABLE);
            }

            HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
        }
        else
        {
            rc = AIR_E_NOT_SUPPORT;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getAutoNego
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
hal_coral_serdes_qp_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_0;
    HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

    if (data & HAL_CORAL_SERDES_AN_REG_0_ANENABLE)
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
    }
    else
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_setLocalAdvAbility
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
hal_coral_serdes_qp_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, data = 0;

    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if ((ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100FUDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100HFDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10HFDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10FUDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_2500M))
            {
                rc = AIR_E_NOT_SUPPORT;
            }
            else if (!(ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_1000FUDX))
            {
                rc = AIR_E_BAD_PARAMETER;
            }
            else
            {
                HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
                SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

                reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_4;
                rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
                if (AIR_E_OK == rc)
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
                    rc = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
                }
            }
        }
        else
        {
            rc = AIR_E_NOT_SUPPORT;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getLocalAdvAbility
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
hal_coral_serdes_qp_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_4;
            rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rc)
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
                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
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
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getRemoteAdvAbility
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
hal_coral_serdes_qp_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_5;
            rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rc)
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
                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            }
        }
        else
        {
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100HFDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10HFDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_setSpeed
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
hal_coral_serdes_qp_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            if (HAL_PHY_SPEED_2500M != speed)
            {
                rc = AIR_E_OP_INVALID;
            }
        }

        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if (HAL_PHY_SPEED_1000M != speed)
            {
                rc = AIR_E_OP_INVALID;
            }
        }

        if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            if (HAL_PHY_SPEED_100M != speed)
            {
                rc = AIR_E_BAD_PARAMETER;
            }
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getSpeed
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
hal_coral_serdes_qp_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;

    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_2500M;
        }
        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_1000M;
        }
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_1000M;
        }
        else if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_100M;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "serdes mode %d get speed fail\n", serdes_mode);
            rc = AIR_E_OTHERS;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getLinkStatus
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
hal_coral_serdes_qp_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                reg_base = 0, reg_data = 0, data = 0, err_cnt = 0;
    UI8_T                 rx_sync = 0, an_done = 0, is_link = 0, an_intr = 0;
    UI8_T                 speed = 0, duplex = 0, an_enable = 0, sig_det = 0;
    UI8_T                 ro_100fx_rdy = 0, err_detect = 0, rx_pwd = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    /* Get speed by serdes config */
    rc = hal_coral_serdes_qp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), &reg_data, sizeof(UI32_T),
                                      rc);
        rx_sync = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_SYNC_OFFSET, HAL_CORAL_SERDES_PCS_RX_SYNC_MASK);
        an_done = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_AN_DONE_OFFSET, HAL_CORAL_SERDES_PCS_RX_AN_DONE_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_5), &reg_data, sizeof(UI32_T), rc);
        is_link = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_LINK_OFFSET, HAL_CORAL_SERDES_AN5_LINK_MASK);
        duplex = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_DUPLEX_OFFSET, HAL_CORAL_SERDES_AN5_DUPLEX_MASK);
        speed = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_SPEED_OFFSET, HAL_CORAL_SERDES_AN5_SPEED_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &reg_data, sizeof(UI32_T), rc);
        an_enable = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN0_ENABLE_OFFSET, HAL_CORAL_SERDES_AN0_ENABLE_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_14), &reg_data, sizeof(UI32_T), rc);
        sig_det = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_SIGDET_CAL_RDY_OFFSET, HAL_CORAL_SERDES_SIGDET_CAL_RDY_MASK);

        DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes an status check: an_enable=%d, an_done=%d, rx_sync=%d,\n", an_enable,
                       an_done, rx_sync);
        DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes link status check: link=%d, speed=%d, duplex=%d,\n", is_link, speed,
                       duplex);

        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            if (rx_sync & sig_det)
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
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            /* update link status */
            if (rx_sync & an_done & is_link)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE);
            }

            /* update sgmii an result */
            if (HAL_CORAL_SERDES_LINK_SPEED_1000M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_1000M;
            }
            else if (HAL_CORAL_SERDES_LINK_SPEED_100M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_100M;
            }
            else if (HAL_CORAL_SERDES_LINK_SPEED_10M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_10M;
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, "get sgmii speed fail, value is 0x%u\n", speed);
                rc = AIR_E_OTHERS;
            }
            if (AIR_E_OK == rc)
            {
                if (HAL_CORAL_SERDES_LINK_DUPLEX_FULL == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                }
                else
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                }
            }
        }

        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if (rx_sync)
            {
                if (an_enable) /* 1000baseX an mode */
                {
                    if (an_done)
                    {
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
                    }
                    else
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                    }
                }
                else /* 1000base-x force mode */
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }

                /* get an done interrupt state, handle 1000base-x flow control sync */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_INT_STATE_REG), &data,
                                              sizeof(UI32_T), rc);
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
        else if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_2), &reg_data, sizeof(UI32_T), rc);
            rx_pwd = !!(reg_data & HAL_CORAL_SERDES_RX_PWD);

            if (rx_pwd)
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }
            else
            {
                /* read ro_100fx_rdy */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX_CTRL_44), &reg_data, sizeof(UI32_T),
                                              rc);
                ro_100fx_rdy = !!(reg_data & HAL_CORAL_SERDES_RO_100FX_RDY);

                /* read error cnt */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_ERR_STATUS), &data,
                                              sizeof(UI32_T), rc);
                err_cnt =
                    BITS_OFF_R(data, HAL_CORAL_SERDES_RO_RCV_ERR_CNT_OFFSET, HAL_CORAL_SERDES_RO_RCV_ERR_CNT_MASK);

                /* read error detect mask */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_STATUS), &data,
                                              sizeof(UI32_T), rc);
                err_detect =
                    BITS_OFF_R(data, HAL_CORAL_SERDES_RO_ERR_DETECT_OFFSET, HAL_CORAL_SERDES_RO_ERR_DETECT_MASK);

                if (ro_100fx_rdy && (err_cnt == 0) && (err_detect == 0))
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
                else
                {
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                }

                /* clear rcv err cnt */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                              sizeof(UI32_T), rc);
                reg_data |= (HAL_CORAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_CORAL_SERDES_RG_ERR_DETECT_CLR);
                HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                               sizeof(UI32_T), rc);
                reg_data &= ~(HAL_CORAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_CORAL_SERDES_RG_ERR_DETECT_CLR);
                HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                               sizeof(UI32_T), rc);
            }

            /* update speed and duplex*/
            ptr_status->speed = HAL_PHY_SPEED_100M;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
            rc = AIR_E_OTHERS;
            DIAG_PRINT(HAL_DBG_ERR, "get serdes speed fail, mode is %d \n", serdes_mode);
            return rc;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_setSerdesMode
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
hal_coral_serdes_qp_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0, reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    /* Run serdes init script */
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        rc = _hal_coral_serdes_qp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_100BASEFX);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "set port %u, serdes mode to %u failed\n", port, serdes_mode);
        }

        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
        reg_data |= RG_QP_100FX_MODE_EN;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
    }
    else
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_QP_100FX_MODE_EN);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);

        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode) /* HSGMII mode */
        {
            rc = _hal_coral_serdes_qp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_HSGMII);
        }
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            rc = _hal_coral_serdes_qp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_SGMII_AN);
        }
        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            rc = _hal_coral_serdes_qp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_1000BASEX);
        }
        else if (HAL_PHY_SERDES_MODE_5GBASE_R == serdes_mode)
        {
            rc = _hal_coral_serdes_qp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_5GBASETR);
        }
        else
        {
            rc = AIR_E_OP_INVALID;
        }
    }
    if (rc == AIR_E_OK)
    {
        /* PCS SGMII_RESET_PHY = 1 */
        reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_0;
        HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
        data |= HAL_CORAL_SERDES_AN_REG_0_RESET;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_getSerdesMode
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
hal_coral_serdes_qp_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0, reg_data;
    UI32_T                data = 0, serdes_speed = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
    if (reg_data & RG_QP_100FX_MODE_EN)
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_100BASE_FX;
    }
    else
    {
        reg_addr = serdes_base + HAL_CORAL_SERDES_DIG_MODE_CTRL_1;
        HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &serdes_speed, sizeof(UI32_T), rc);

        serdes_speed &= HAL_CORAL_SERDES_TPHY_SPEED_MASK;
        if (HAL_CORAL_SERDES_TPHY_SPEED_HSGMII == serdes_speed)
        {
            *ptr_serdes_mode = HAL_PHY_SERDES_MODE_HSGMII;
        }
        else if (HAL_CORAL_SERDES_TPHY_SPEED_SGMII == serdes_speed)
        {
            reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_13;
            HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

            if (data & HAL_CORAL_SERDES_SGMII_ENABLE)
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
            rc = AIR_E_OP_INVALID;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_qp_dumpPortCnt
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
hal_coral_serdes_qp_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI16_T                      idx, rg_idx;
    HAL_CORAL_SERDES_ID_T       serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_CORAL_SERDES_DBG_CFG_T *ptr_serdes_dbg_list;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    osal_printf("%20s        %16s        %9s \n", "Register", "Addr", "Value");
    for (idx = 0; idx < _hal_coral_serdes_qp_dbg_cfg_info_size; idx++)
    {
        if (_hal_coral_serdes_qp_dbg_cfg_info[idx].serdes_interface & param)
        {
            for (rg_idx = 0; rg_idx < _hal_coral_serdes_qp_dbg_cfg_info[idx].serdes_register_cnt; rg_idx++)
            {
                ptr_serdes_dbg_list =
                    (HAL_CORAL_SERDES_DBG_CFG_T *)(_hal_coral_serdes_qp_dbg_cfg_info[idx].ptr_serdes_dbg_cfg_map +
                                                   rg_idx);

                rc = _hal_coral_serdes_qp_dumpDbgRegister(unit, serdes_id, ptr_serdes_dbg_list);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set _hal_coral_serdes_qp_dumpDbgRegister fail, rc = %d\n", rc);
                    break;
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_setPhyLedCtrlMode
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
hal_coral_serdes_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &=
            ~(BITS_OFF_L(HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_MASK, offset, HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_WIDTH));
        led_config |= BITS_OFF_L(ctrl_mode, offset, HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_WIDTH);
        rv = aml_writeReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_coral_serdes_getPhyLedCtrlMode
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
hal_coral_serdes_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(led_config, offset, HAL_CORAL_SERDES_SFP_LED_CTRL_MODE_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_coral_serdes_setPhyLedForceState
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
hal_coral_serdes_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &= ~(
            BITS_OFF_L(HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_MASK, offset, HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_WIDTH));
        led_config |= BITS_OFF_L(state, offset, HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_WIDTH);
        rv = aml_writeReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_coral_serdes_getPhyLedForceState
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
hal_coral_serdes_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(led_config, offset, HAL_CORAL_SERDES_SFP_LED_FORCE_STATE_WIDTH);
    }

    return rv;
}

/* FUNCTION NAME:   hal_coral_serdes_setPhyLedForcePattCfg
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
hal_coral_serdes_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_PATT_CFG_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &=
            ~(BITS_OFF_L(HAL_CORAL_SERDES_SFP_LED_PATT_CFG_MASK, offset, HAL_CORAL_SERDES_SFP_LED_PATT_CFG_WIDTH));
        led_config |= BITS_OFF_L(pattern, offset, HAL_CORAL_SERDES_SFP_LED_PATT_CFG_WIDTH);
        rv = aml_writeReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_coral_serdes_getPhyLedForcePattCfg
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
hal_coral_serdes_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_CORAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, HAL_CORAL_SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (HAL_CORAL_SERDES_SFP_LED_PATT_CFG_BIT + (HAL_CORAL_SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (HAL_CORAL_SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_pattern = BITS_OFF_R(led_config, offset, HAL_CORAL_SERDES_SFP_LED_PATT_CFG_WIDTH);
    }

    return rv;
}
/* FUNCTION NAME:   hal_coral_serdes_qp_dumpDebugInfo
 * PURPOSE:
 *      Dump port debug information.
 *
 * INPUT:
 *      unit                    --  Device unit number
 *      port                    --  Port number
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
hal_coral_serdes_qp_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T        rc = 0;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_base = 0, reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    osal_printf("\n[Internal status] ========================================================\n");
    /* PCS_STATUS */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), &reg_data, sizeof(UI32_T), rc);

    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), reg_data);
    osal_printf("  rx_sync = %u             an_down = %u\n",
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_SYNC_OFFSET, HAL_CORAL_SERDES_PCS_RX_SYNC_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_AN_DONE_OFFSET, HAL_CORAL_SERDES_PCS_RX_AN_DONE_MASK));

    /* AN_5 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_5), &reg_data, sizeof(UI32_T), rc);
    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_AN_REG_5), reg_data);

    osal_printf("  link = %u                speed = %u                duplex = %u\n",
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_LINK_OFFSET, HAL_CORAL_SERDES_AN5_LINK_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_SPEED_OFFSET, HAL_CORAL_SERDES_AN5_SPEED_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_DUPLEX_OFFSET, HAL_CORAL_SERDES_AN5_DUPLEX_MASK));

    /* AN_4 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_4), &reg_data, sizeof(UI32_T), rc);
    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_AN_REG_4), reg_data);
    osal_printf("  pause = %u               asm_dir = %u\n",
                BITS_OFF_R(reg_data, SGMII_1000BASEX_PAUSE_OFFSET, SGMII_1000BASEX_PAUSE_MASK),
                BITS_OFF_R(reg_data, SGMII_1000BASEX_ASM_DIR_OFFSET, SGMII_1000BASEX_PAUSE_MASK));
    return rc;
}
HAL_PHY_DRIVER_T
_int_coral_serdes_qp_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_coral_serdes_qp_init,
    hal_coral_serdes_qp_setAdminState,
    hal_coral_serdes_qp_getAdminState,
    hal_coral_serdes_qp_setAutoNego,
    hal_coral_serdes_qp_getAutoNego,
    hal_coral_serdes_qp_setLocalAdvAbility,
    hal_coral_serdes_qp_getLocalAdvAbility,
    hal_coral_serdes_qp_getRemoteAdvAbility,
    hal_coral_serdes_qp_setSpeed,
    hal_coral_serdes_qp_getSpeed,
    NULL, /* setDuplex */
    NULL, /* getDuplex */
    hal_coral_serdes_qp_getLinkStatus,
    NULL, /* getLoopBack */
    NULL, /* setLoopBack */
    NULL, /* setSmartSpeedDown */
    NULL, /* getSmartSpeedDown */
    NULL, /* setLedOnCtrl */
    NULL, /* getLedOnCtrl */
    NULL, /* testTxCompliance*/
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    hal_coral_serdes_qp_setSerdesMode,
    hal_coral_serdes_qp_getSerdesMode,
    hal_coral_serdes_setPhyLedCtrlMode,
    hal_coral_serdes_getPhyLedCtrlMode,
    hal_coral_serdes_setPhyLedForceState,
    hal_coral_serdes_getPhyLedForceState,
    hal_coral_serdes_setPhyLedForcePattCfg,
    hal_coral_serdes_getPhyLedForcePattCfg,
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
    hal_coral_serdes_qp_dumpPortCnt,
    hal_coral_serdes_qp_dumpDebugInfo,
};

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_setRegister(
    const UI32_T                  unit,
    const HAL_CORAL_SERDES_ID_T   serdes_id,
    const HAL_CORAL_SERDES_CFG_T *ptr_serdes_cfg)
{
    UI32_T         serdes_reg, serdes_regValue;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (ptr_serdes_cfg->delay_time)
    {
        osal_delayUs(ptr_serdes_cfg->delay_time * 1000);
    }

    serdes_reg = ptr_serdes_cfg->reg_addr;

    HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);

    serdes_regValue &=
        ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));

    serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                  ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));

    HAL_CORAL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_configMode(
    const UI32_T                  unit,
    const HAL_CORAL_SERDES_ID_T   serdes_id,
    const HAL_CORAL_SERDES_MODE_T mode)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  i = 0, j = 0;
    HAL_CORAL_SERDES_CFG_T *ptr_mac_port_map;

    DIAG_PRINT(HAL_DBG_INFO, "config mode is %d\n", mode);
    for (i = 0; i < _hal_coral_serdes_pxp_cfg_info_size; i++)
    {
        if (mode == _hal_coral_serdes_pxp_cfg_info[i].serdes_mode)
        {
            for (j = 0; j < _hal_coral_serdes_pxp_cfg_info[i].serdes_register_cnt; j++)
            {
                ptr_mac_port_map = (HAL_CORAL_SERDES_CFG_T *)(_hal_coral_serdes_pxp_cfg_info[i].ptr_mac_port_map + j);
                rc = _hal_coral_serdes_pxp_setRegister(unit, serdes_id, ptr_mac_port_map);
                if (rc != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_pxp_setRegister fail rc = %d\n", rc);
                    return rc;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_configPolarity(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_POLARITY_T polarity)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    UI32_T         swap_tx = 0, swap_rx = 0;

    HAL_CORAL_SERDES_AML_READ_REG(unit, ADD_DIG_RESERVE_0, &swap_rx, sizeof(UI32_T), rc);
    HAL_CORAL_SERDES_AML_READ_REG(unit, XPON_SETTING_0, &swap_tx, sizeof(UI32_T), rc);

    switch (polarity)
    {
        case HAL_CORAL_SERDES_POLARITY_NOSWAP:
            swap_tx &= ~(RG_TRANS_TX_DATA_INV);
            swap_rx &= ~(RG_RX_DATA_INV);
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_TX:
            swap_tx |= RG_TRANS_TX_DATA_INV;
            swap_rx &= ~(RG_RX_DATA_INV);
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_RX:
            swap_tx &= ~(RG_TRANS_TX_DATA_INV);
            swap_rx |= RG_RX_DATA_INV;
            break;
        case HAL_CORAL_SERDES_POLARITY_SWAP_TXRX:
            swap_tx |= RG_TRANS_TX_DATA_INV;
            swap_rx |= RG_RX_DATA_INV;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            DIAG_PRINT(HAL_DBG_ERR, "unexpected priority value = %d, rc = %d\n", polarity, rc);
            break;
    }
    if (AIR_E_OK == rc)
    {
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, ADD_DIG_RESERVE_0, &swap_rx, sizeof(UI32_T), rc);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, XPON_SETTING_0, &swap_tx, sizeof(UI32_T), rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_setting(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_MODE_T     mode,
    const HAL_CORAL_SERDES_POLARITY_T polarity)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if ((serdes_id >= HAL_CORAL_SERDES_ID_LAST) || (mode >= HAL_CORAL_SERDES_MODE_LAST) ||
        (polarity >= HAL_CORAL_SERDES_POLARITY_LAST))
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = _hal_coral_serdes_pxp_configMode(unit, serdes_id, mode);
    if (rc != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_pxp_configMode fail rc = %d\n", rc);
        return rc;
    }
    rc = _hal_coral_serdes_pxp_configPolarity(unit, serdes_id, polarity);
    if (rc != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invoke _hal_coral_serdes_pxp_configPolarity fail rc = %d\n", rc);
        return rc;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_getPortMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_CORAL_SERDES_MODE_T *ptr_mode)
{
    HAL_SDK_PORT_MAP_T     *ptr_sdk_port_map_entry;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    HAL_CORAL_SERDES_ID_T   serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                  air_port = 0, count = 0;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

    if (AIR_INIT_PORT_TYPE_XSGMII == ptr_sdk_port_map_entry->port_type)
    {
        if (AIR_INIT_PORT_SPEED_2500M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_CORAL_SERDES_MODE_HSGMII;
        }
        else if (AIR_INIT_PORT_SPEED_5000M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = HAL_CORAL_SERDES_MODE_5GBASETR;
        }

        serdes_id = ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (HAL_CORAL_SERDES_MODE_DEFAULT_VALUE == serdes_mode)
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
        serdes_mode = HAL_CORAL_SERDES_MODE_SGMII_AN;
    }
    *ptr_mode = serdes_mode;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_getEEELineSideAnStatus(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_status)
{
    UI16_T         eee_adv = 0, eee_lp = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &eee_adv);
    if (rc == AIR_E_OK)
    {
        rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &eee_lp);
        if (rc == AIR_E_OK)
        {
            *ptr_status = (eee_adv & eee_lp) & (EEE_1000BASE_T | EEE_100BASE_TX);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_checkChangeMode(
    const UI32_T             unit,
    const UI32_T             port,
    HAL_CORAL_SERDES_MODE_T *ptr_mode,
    BOOL_T                  *ptr_change)
{
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_PHY_LINK_STATUS_T link_status;
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                base_addr = 0, serdes_speed = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    rc = hal_phy_getLinkStatus(unit, port, &link_status);
    if (rc == AIR_E_OK)
    {
        if (link_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
        {
            rc = aml_readReg(unit, (base_addr + HAL_CORAL_SERDES_DIG_MODE_CTRL_1), &serdes_speed, sizeof(serdes_speed));
            if (rc == AIR_E_OK)
            {
                serdes_speed &= HAL_CORAL_SERDES_TPHY_SPEED_MASK;
                if ((HAL_CORAL_SERDES_TPHY_SPEED_SGMII == serdes_speed) && (HAL_PHY_SPEED_2500M == link_status.speed))
                {
                    *ptr_mode = HAL_CORAL_SERDES_MODE_HSGMII;
                    *ptr_change = TRUE;
                }
                else if ((HAL_CORAL_SERDES_TPHY_SPEED_HSGMII == serdes_speed) &&
                         (HAL_PHY_SPEED_2500M != link_status.speed))
                {
                    *ptr_mode = HAL_CORAL_SERDES_MODE_SGMII_AN;
                    *ptr_change = TRUE;
                }
                else
                {
                    *ptr_change = FALSE;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_changeMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const HAL_CORAL_SERDES_MODE_T mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_data;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);

    /* Tx_Disable = 1*/
    HAL_CORAL_SERDES_AML_READ_REG(unit, RG_XPON_TX_CKLDO_EN, &reg_data, sizeof(UI32_T), rc);
    reg_data &= ~(RG_XPON_TX_CKLDO_ENABLE);
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_XPON_TX_CKLDO_EN, &reg_data, sizeof(UI32_T), rc);

    /* RX_RST_N = 0 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);
    reg_data &= ~(RG_SW_RX_RST_N);
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);

    /* PLL_EN = 0 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, RG_FORCE_DA_PXP_TXPLL_CKOUT_EN, &reg_data, sizeof(reg_data), rc);
    reg_data &= ~(RG_FORCE_SEL_DA_PXP_TXPLL_EN | RG_FORCE_DA_PXP_TXPLL_EN);
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_FORCE_DA_PXP_TXPLL_CKOUT_EN, &reg_data, sizeof(reg_data), rc);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    rc = _hal_coral_serdes_pxp_configMode(unit, serdes_id, HAL_CORAL_SERDES_MODE_DEFAULT_VALUE);
    if (AIR_E_OK == rc)
    {
        rc = _hal_coral_serdes_pxp_configMode(unit, serdes_id, mode);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "config port %u serdes mode %x failed\n", port, mode);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "reset port %u serdes register default value failed\n", port);
    }

    return rc;
}

static void
_hal_coral_serdes_pxp_linkChangeCallback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T   serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI16_T                  eee_data = 0;
    UI32_T                  base_addr = 0, eee_serdes = 0;
    UI32_T                  phy_id = 0;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    BOOL_T                  need_change = FALSE;
    AIR_CFG_VALUE_T         serdes_op_mode;

    if (HAL_IS_XSGMII_PORT_VALID(unit, port))
    {
        osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
        serdes_op_mode.param0 = port;
        hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

        if (HAL_PHY_SERDES_OP_MODE_SPEED_CAHNGE == serdes_op_mode.value)
        {
            if (link)
            {
                rc = _hal_coral_serdes_pxp_checkChangeMode(unit, port, &serdes_mode, &need_change);
                if ((rc == AIR_E_OK) && (need_change == TRUE))
                {
                    rc = _hal_coral_serdes_pxp_changeMode(unit, port, serdes_mode);
                    if (rc != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Set port %u, serdes mode to %u failed\n", port, serdes_mode);
                        return;
                    }
                }
            }
        }

        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

        /* eee status */
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

        rc = aml_readReg(unit, (base_addr + (HAL_CORAL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &eee_serdes,
                         sizeof(eee_serdes));
        if (AIR_E_OK == rc)
        {
            if (link)
            {
                rc = _hal_coral_serdes_pxp_getEEELineSideAnStatus(unit, port, &eee_data);
                if ((AIR_E_OK == rc) && (eee_data))
                {
                    eee_serdes |= (HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
                }
                else
                {
                    eee_serdes &= ~(HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
                }
            }
            else
            {
                eee_data = 0;
                eee_serdes &= ~(HAL_CORAL_SERDES_RA_P0_MII_RX_EEE_EN + HAL_CORAL_SERDES_RA_P0_MII_TX_EEE_EN);
            }
            /* In order to bypass LPI to MAC for 100M, it should set mii tx/rx eee enable bits
               in RATE_ADP_PN_CTRL_REG_N */
            rc = aml_writeReg(unit, (base_addr + (HAL_CORAL_SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)),
                              &eee_serdes, sizeof(eee_serdes));
            if (rc != AIR_E_OK)
            {
                return;
            }
        }
    }
}

static AIR_ERROR_NO_T
_hal_coral_serdes_pxp_dumpDbgRegister(
    const UI32_T                      unit,
    const HAL_CORAL_SERDES_ID_T       serdes_id,
    const HAL_CORAL_SERDES_DBG_CFG_T *ptr_serdes_cfg)
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
        HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        serdes_regValue &=
            ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                      ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    }
    else /* read */
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        osal_printf("%30s       0x%8x       0x%08x \n", ptr_serdes_cfg->reg_name, serdes_reg, serdes_regValue);
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_serdes_pxp_init
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
hal_coral_serdes_pxp_init(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    HAL_CORAL_SERDES_MODE_T serdes_mode = HAL_CORAL_SERDES_MODE_DEFAULT_VALUE;
    AIR_CFG_VALUE_T         polarity;
    UI32_T                  reg_data = 0;

    HAL_CORAL_SERDES_AML_READ_REG(unit, HAL_CORAL_SERDES_RG_ELDO_NDIS_EN, &reg_data, sizeof(UI32_T), rc);
    reg_data &= ~(BIT(HAL_CORAL_SERDES_RG_CFG_EXT_OFFSET));
    HAL_CORAL_SERDES_AML_WRITE_REG(unit, HAL_CORAL_SERDES_RG_ELDO_NDIS_EN, &reg_data, sizeof(UI32_T), rc);

    rc = _hal_coral_serdes_pxp_getPortMode(unit, port, &serdes_mode);
    if (rc == AIR_E_OK)
    {
        osal_memset(&polarity, 0, sizeof(AIR_CFG_VALUE_T));

        polarity.value = 0;
        polarity.param0 = port;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE, &polarity);

        if (rc == AIR_E_OK)
        {
            rc = _hal_coral_serdes_pxp_setting(unit, HAL_CORAL_SERDES_ID_S0, serdes_mode, polarity.value);
            if (rc == AIR_E_OK)
            {
                /* regsiter ifmon to know link change event */
                rc = hal_coral_ifmon_lookup(unit, _hal_coral_serdes_pxp_linkChangeCallback, NULL);
                if (rc == AIR_E_ENTRY_NOT_FOUND)
                {
                    rc = hal_coral_ifmon_register(unit, _hal_coral_serdes_pxp_linkChangeCallback, NULL);
                }
            }
        }
        else
        {
            rc = AIR_E_OP_INCOMPLETE;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_setAdminState
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
hal_coral_serdes_pxp_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_base = 0;
    UI32_T                reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, RX_HW_CTRL_0, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_XPON_FORCE_CDR_LCK2DATA | RG_XPON_FORCE_CDR_LCK2DATA_SEL);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RX_HW_CTRL_0, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_FORCE_SEL_DA_PXP_TX_DATA_EN | RG_FORCE_DA_PXP_TX_DATA_EN);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &reg_data, sizeof(UI32_T), rc);
        reg_data |= HAL_CORAL_SERDES_AN_REG_0_RESET;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &reg_data, sizeof(UI32_T), rc);
    }
    else
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, RX_HW_CTRL_0, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_XPON_FORCE_CDR_LCK2DATA);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RX_HW_CTRL_0, &reg_data, sizeof(UI32_T), rc);
        reg_data |= RG_XPON_FORCE_CDR_LCK2DATA_SEL;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RX_HW_CTRL_0, &reg_data, sizeof(UI32_T), rc);
        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_FORCE_DA_PXP_TX_DATA_EN);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);
        reg_data |= RG_FORCE_SEL_DA_PXP_TX_DATA_EN;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, ADD_DIG_RESERVE_47, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(USGMII_TXPMA_SW_RST_N | USGMII_RXPMA_SW_RST_N | FX_EFIFO_SW_RST_N);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, ADD_DIG_RESERVE_47, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_SW_RX_FIFO_RST_N | RG_SW_TX_FIFO_RST_N);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, ADD_DIG_RESERVE_47, &reg_data, sizeof(UI32_T), rc);
        reg_data |= (USGMII_TXPMA_SW_RST_N | USGMII_RXPMA_SW_RST_N | FX_EFIFO_SW_RST_N);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, ADD_DIG_RESERVE_47, &reg_data, sizeof(UI32_T), rc);

        HAL_CORAL_SERDES_AML_READ_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);
        reg_data |= (RG_SW_RX_FIFO_RST_N | RG_SW_TX_FIFO_RST_N);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, SW_RST_SET, &reg_data, sizeof(UI32_T), rc);
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getAdminState
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
hal_coral_serdes_pxp_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;

    HAL_CORAL_SERDES_AML_READ_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);

    if (!!(reg_data & RG_FORCE_SEL_DA_PXP_TX_DATA_EN))
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
    }
    else
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_setAutoNego
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
hal_coral_serdes_pxp_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);
    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if ((HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode) || (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
        {
            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_0;

            HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

            if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
            {
                data |= HAL_CORAL_SERDES_AN_REG_0_ANENABLE;
            }
            else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
            {
                data |= HAL_CORAL_SERDES_AN_REG_0_ANRESTART;
            }
            else
            {
                data &= ~(HAL_CORAL_SERDES_AN_REG_0_ANENABLE);
            }

            HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
        }
        else
        {
            rc = AIR_E_NOT_SUPPORT;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getAutoNego
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
hal_coral_serdes_pxp_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);
    reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_0;
    HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);

    if (data & HAL_CORAL_SERDES_AN_REG_0_ANENABLE)
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
    }
    else
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_setLocalAdvAbility
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
hal_coral_serdes_pxp_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, data = 0;

    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if ((ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100FUDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_100HFDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10HFDX) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_10FUDX) ||
                (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_EEE) || (ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_2500M))
            {
                rc = AIR_E_NOT_SUPPORT;
            }
            else if (!(ptr_adv->flags & HAL_PHY_AN_ADV_FLAGS_1000FUDX))
            {
                rc = AIR_E_BAD_PARAMETER;
            }
            else
            {
                HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
                SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

                reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_4;
                rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
                if (AIR_E_OK == rc)
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
                    rc = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
                }
            }
        }
        else
        {
            rc = AIR_E_NOT_SUPPORT;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getLocalAdvAbility
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
hal_coral_serdes_pxp_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_4;
            rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rc)
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
                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
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
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getRemoteAdvAbility
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
hal_coral_serdes_pxp_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + HAL_CORAL_SERDES_AN_REG_5;
            rc = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (AIR_E_OK == rc)
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
                ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            }
        }
        else
        {
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_1000FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100HFDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10FUDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_10HFDX;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_EEE_INVALID;
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_setSpeed
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
hal_coral_serdes_pxp_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            if (HAL_PHY_SPEED_2500M != speed)
            {
                rc = AIR_E_OP_INVALID;
            }
        }

        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if (HAL_PHY_SPEED_1000M != speed)
            {
                rc = AIR_E_OP_INVALID;
            }
        }

        if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            if (HAL_PHY_SPEED_100M != speed)
            {
                rc = AIR_E_BAD_PARAMETER;
            }
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getSpeed
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
hal_coral_serdes_pxp_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    AIR_ERROR_NO_T        rc = AIR_E_NOT_SUPPORT;
    HAL_PHY_SERDES_MODE_T serdes_mode;

    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_2500M;
        }
        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_1000M;
        }
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_1000M;
        }
        else if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            *ptr_speed = HAL_PHY_SPEED_100M;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "serdes mode %d get speed fail\n", serdes_mode);
            rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getLinkStatus
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
hal_coral_serdes_pxp_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    UI32_T                reg_base = 0, reg_data = 0, data = 0, err_cnt = 0;
    UI8_T                 rx_sync = 0, an_done = 0, is_link = 0, an_intr = 0;
    UI8_T                 speed = 0, duplex = 0, an_enable = 0, lck2data = 0;
    UI8_T                 ro_100fx_rdy = 0, err_detect = 0, tx_data_en = 0;
    UI8_T                 patch_rg = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    /* Get speed by serdes config */
    rc = hal_coral_serdes_pxp_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rc)
    {
        /* LCK2DATA patch start */
        if ((HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode) || (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode))
        {
            HAL_CORAL_SERDES_AML_READ_REG(unit, XPON_INT_STA_5, &reg_data, sizeof(UI32_T), rc);
            if (reg_data & RX_SIGDET_DOWN_INT) /* RX_SIGDET_DOWN_INT */
            {
                if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
                {
                    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                                  sizeof(UI32_T), rc);
                    reg_data |= BITS_OFF_L(0x0, HAL_CORAL_SERDES_RG_LINK_CTRL_100_OFFSET,
                                           HAL_CORAL_SERDES_RG_LINK_CTRL_100_MASK);
                    HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                                   sizeof(UI32_T), rc);
                }

                for (patch_rg = 0; patch_rg < _hal_coral_serdes_pxp_setLck2Data_patch_size; patch_rg++)
                {
                    _hal_coral_serdes_pxp_setRegister(unit, serdes_id,
                                                      &_hal_coral_serdes_pxp_setLck2Data_patch[patch_rg]);
                }

                if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
                {
                    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                                  sizeof(UI32_T), rc);
                    reg_data |= BITS_OFF_L(0x3, HAL_CORAL_SERDES_RG_LINK_CTRL_100_OFFSET,
                                           HAL_CORAL_SERDES_RG_LINK_CTRL_100_MASK);
                    HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                                   sizeof(UI32_T), rc);
                }

                /* RX_SIGDET_DOWN_INT clear interrupt */
                HAL_CORAL_SERDES_AML_READ_REG(unit, XPON_INT_STA_5, &reg_data, sizeof(UI32_T), rc);
                reg_data |= RX_SIGDET_DOWN_INT;
                HAL_CORAL_SERDES_AML_WRITE_REG(unit, XPON_INT_STA_5, &reg_data, sizeof(UI32_T), rc);
            }
            osal_delayUs(10 * 1000);
        }
        /* LCK2DATA patch end */

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), &reg_data, sizeof(UI32_T),
                                      rc);
        rx_sync = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_SYNC_OFFSET, HAL_CORAL_SERDES_PCS_RX_SYNC_MASK);
        an_done = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_AN_DONE_OFFSET, HAL_CORAL_SERDES_PCS_RX_AN_DONE_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_5), &reg_data, sizeof(UI32_T), rc);
        is_link = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_LINK_OFFSET, HAL_CORAL_SERDES_AN5_LINK_MASK);
        duplex = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_DUPLEX_OFFSET, HAL_CORAL_SERDES_AN5_DUPLEX_MASK);
        speed = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_SPEED_OFFSET, HAL_CORAL_SERDES_AN5_SPEED_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_0), &reg_data, sizeof(UI32_T), rc);
        an_enable = BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN0_ENABLE_OFFSET, HAL_CORAL_SERDES_AN0_ENABLE_MASK);

        HAL_CORAL_SERDES_AML_READ_REG(unit, DEBUG_XPON_0, &reg_data, sizeof(UI32_T), rc);
        lck2data = BITS_OFF_R(reg_data, DEBUG_RX_0_LCK2DATA_OFFSET, DEBUG_RX_0_LCK2DATA_MASK);

        DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes an status check: an_enable=%d, an_done=%d, rx_sync=%d,\n", an_enable,
                       an_done, rx_sync);
        DIAG_PRINT_RAW(HAL_DBG_INFO, "serdes link status check: link=%d, speed=%d, duplex=%d,\n", is_link, speed,
                       duplex);

        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)
        {
            if (rx_sync & lck2data)
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
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            /* update link status */
            if (rx_sync & an_done & is_link)
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE);
            }

            /* update sgmii an result */
            if (HAL_CORAL_SERDES_LINK_SPEED_1000M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_1000M;
            }
            else if (HAL_CORAL_SERDES_LINK_SPEED_100M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_100M;
            }
            else if (HAL_CORAL_SERDES_LINK_SPEED_10M == speed)
            {
                ptr_status->speed = HAL_PHY_SPEED_10M;
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, "get sgmii speed fail, value is 0x%x\n", speed);
                rc = AIR_E_OTHERS;
            }
            if (AIR_E_OK == rc)
            {
                if (HAL_CORAL_SERDES_LINK_DUPLEX_FULL == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                }
                else
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                }
            }
        }

        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            if (rx_sync)
            {
                if (an_enable) /* 1000baseX an mode */
                {
                    if (an_done)
                    {
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                        ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE;
                    }
                    else
                    {
                        ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                    }
                }
                else /* 1000base-x force mode */
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }

                /* get an done interrupt state, handle 1000base-x flow control sync */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_INT_STATE_REG), &data,
                                              sizeof(UI32_T), rc);
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
        else if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            HAL_CORAL_SERDES_AML_READ_REG(unit, RG_FORCE_DA_PXP_TX_HSDATA_EN, &reg_data, sizeof(UI32_T), rc);
            tx_data_en = !!(reg_data & RG_FORCE_SEL_DA_PXP_TX_DATA_EN);

            if (tx_data_en)
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }
            else
            {
                /* read ro_fx_rdy */
                HAL_CORAL_SERDES_AML_READ_REG(unit, DEBUG_DEC_ERRO_100FX, &reg_data, sizeof(UI32_T), rc);
                ro_100fx_rdy = !!(reg_data & RO_FX_RDY);

                /* read error cnt */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_ERR_STATUS), &data,
                                              sizeof(UI32_T), rc);
                err_cnt =
                    BITS_OFF_R(data, HAL_CORAL_SERDES_RO_RCV_ERR_CNT_OFFSET, HAL_CORAL_SERDES_RO_RCV_ERR_CNT_MASK);

                /* read error detect mask */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_STATUS), &data,
                                              sizeof(UI32_T), rc);
                err_detect =
                    BITS_OFF_R(data, HAL_CORAL_SERDES_RO_ERR_DETECT_OFFSET, HAL_CORAL_SERDES_RO_ERR_DETECT_MASK);

                if (ro_100fx_rdy && (err_cnt == 0) && (err_detect == 0))
                {
                    ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
                }
                else
                {
                    ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
                }

                /* clear rcv err cnt */
                HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                              sizeof(UI32_T), rc);
                reg_data |= (HAL_CORAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_CORAL_SERDES_RG_ERR_DETECT_CLR);
                HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                               sizeof(UI32_T), rc);
                reg_data &= ~(HAL_CORAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_CORAL_SERDES_RG_ERR_DETECT_CLR);
                HAL_CORAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_CORAL_SERDES_RX100FX_PHY_CTRL), &reg_data,
                                               sizeof(UI32_T), rc);
            }
            /* update speed and duplex*/
            ptr_status->speed = HAL_PHY_SPEED_100M;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        else
        {
            rc = AIR_E_OTHERS;
            DIAG_PRINT(HAL_DBG_ERR, "get serdes speed fail, mode is %d \n", serdes_mode);
            return rc;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_setSerdesMode
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
hal_coral_serdes_pxp_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0, reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    /* Run serdes init script */
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_100BASEFX);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "set port %u, serdes mode to %u failed\n", port, serdes_mode);
        }

        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
        reg_data |= RG_PXP_100FX_MODE_EN;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
    }
    else
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
        reg_data &= ~(RG_PXP_100FX_MODE_EN);
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);

        if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode) /* HSGMII mode */
        {
            rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_HSGMII);
        }
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_SGMII_AN);
        }
        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_1000BASEX);
        }
        else if (HAL_PHY_SERDES_MODE_5GBASE_R == serdes_mode)
        {
            rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_5GBASETR);
        }
        else if (HAL_PHY_SERDES_MODE_USXGMII == serdes_mode)
        {
            rc = _hal_coral_serdes_pxp_changeMode(unit, port, HAL_CORAL_SERDES_MODE_USXGMII);
        }
        else
        {
            rc = AIR_E_OP_INVALID;
        }
    }
    if (rc == AIR_E_OK)
    {
        /* PCS SGMII_RESET_PHY = 1 */
        reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_0;
        HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
        data |= HAL_CORAL_SERDES_AN_REG_0_RESET;
        HAL_CORAL_SERDES_AML_WRITE_REG(unit, reg_addr, &data, sizeof(UI32_T), rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_getSerdesMode
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
hal_coral_serdes_pxp_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0, reg_data;
    UI32_T                xfi_rx_mode = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    HAL_CORAL_SERDES_AML_READ_REG(unit, RG_BASE100_FX_MODE, &reg_data, sizeof(UI32_T), rc);
    if (reg_data & RG_PXP_100FX_MODE_EN) /* 100base-FX */
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_100BASE_FX;
    }
    else
    {
        HAL_CORAL_SERDES_AML_READ_REG(unit, ADD_XPON_MODE_1, &reg_data, sizeof(UI32_T), rc);
        xfi_rx_mode = BITS_OFF_R(reg_data, RG_XFI_RX_MODE_OFFT, RG_XFI_RX_MODE_MASK);

        if (RG_XFI_RX_MODE_SGMII == xfi_rx_mode)
        {
            reg_addr = serdes_base + HAL_CORAL_SERDES_AN_REG_13;
            HAL_CORAL_SERDES_AML_READ_REG(unit, reg_addr, &reg_data, sizeof(UI32_T), rc);

            if (reg_data & HAL_CORAL_SERDES_SGMII_ENABLE)
            {
                *ptr_serdes_mode = HAL_PHY_SERDES_MODE_SGMII;
            }
            else
            {
                *ptr_serdes_mode = HAL_PHY_SERDES_MODE_1000BASE_X;
            }
        }
        else if (RG_XFI_RX_MODE_HSGMII == xfi_rx_mode)
        {
            *ptr_serdes_mode = HAL_PHY_SERDES_MODE_HSGMII;
        }
        else
        {
            rc = AIR_E_OTHERS;
            DIAG_PRINT(HAL_DBG_ERR, "serdes mode get fail xfi_rx_mode = %u, rc = %d\n", xfi_rx_mode, rc);
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_dumpPortCnt
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
hal_coral_serdes_pxp_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;

    UI16_T                      idx, rg_idx;
    HAL_CORAL_SERDES_ID_T       serdes_id = HAL_CORAL_SERDES_ID_S0;
    HAL_CORAL_SERDES_DBG_CFG_T *ptr_serdes_dbg_list;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    osal_printf("%20s        %16s        %9s \n", "Register", "Addr", "Value");
    for (idx = 0; idx < _hal_coral_serdes_pxp_dbg_cfg_info_size; idx++)
    {
        if (_hal_coral_serdes_pxp_dbg_cfg_info[idx].serdes_interface & param)
        {
            for (rg_idx = 0; rg_idx < _hal_coral_serdes_pxp_dbg_cfg_info[idx].serdes_register_cnt; rg_idx++)
            {
                ptr_serdes_dbg_list =
                    (HAL_CORAL_SERDES_DBG_CFG_T *)(_hal_coral_serdes_pxp_dbg_cfg_info[idx].ptr_serdes_dbg_cfg_map +
                                                   rg_idx);

                rc = _hal_coral_serdes_pxp_dumpDbgRegister(unit, serdes_id, ptr_serdes_dbg_list);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set _hal_coral_serdes_pxp_dumpDbgRegister fail, rc = %d\n", rc);
                    break;
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_serdes_pxp_dumpDebugInfo
 * PURPOSE:
 *      Dump port debug information.
 *
 * INPUT:
 *      unit                    --  Device unit number
 *      port                    --  Port number
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
hal_coral_serdes_pxp_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T        rc = 0;
    HAL_CORAL_SERDES_ID_T serdes_id = HAL_CORAL_SERDES_ID_S0;
    UI32_T                reg_base = 0, reg_data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    osal_printf("\n[Internal status] ========================================================\n");
    /* PCS_STATUS */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), &reg_data, sizeof(UI32_T), rc);

    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_PCS_STATE_REG_2), reg_data);
    osal_printf("  rx_sync = %u             an_down = %u\n",
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_SYNC_OFFSET, HAL_CORAL_SERDES_PCS_RX_SYNC_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_PCS_RX_AN_DONE_OFFSET, HAL_CORAL_SERDES_PCS_RX_AN_DONE_MASK));

    /* AN_5 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_5), &reg_data, sizeof(UI32_T), rc);
    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_AN_REG_5), reg_data);

    osal_printf("  link = %u                speed = %u                duplex = %u\n",
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_LINK_OFFSET, HAL_CORAL_SERDES_AN5_LINK_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_SPEED_OFFSET, HAL_CORAL_SERDES_AN5_SPEED_MASK),
                BITS_OFF_R(reg_data, HAL_CORAL_SERDES_AN5_DUPLEX_OFFSET, HAL_CORAL_SERDES_AN5_DUPLEX_MASK));

    /* AN_4 */
    HAL_CORAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_CORAL_SERDES_AN_REG_4), &reg_data, sizeof(UI32_T), rc);
    osal_printf(" reg 0x%x = 0x%x\n", (reg_base + HAL_CORAL_SERDES_AN_REG_4), reg_data);
    osal_printf("  pause = %u               asm_dir = %u\n",
                BITS_OFF_R(reg_data, SGMII_1000BASEX_PAUSE_OFFSET, SGMII_1000BASEX_PAUSE_MASK),
                BITS_OFF_R(reg_data, SGMII_1000BASEX_ASM_DIR_OFFSET, SGMII_1000BASEX_PAUSE_MASK));

    return rc;
}

const static HAL_PHY_DRIVER_T _int_coral_serdes_pxp_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_coral_serdes_pxp_init,
    hal_coral_serdes_pxp_setAdminState,
    hal_coral_serdes_pxp_getAdminState,
    hal_coral_serdes_pxp_setAutoNego,
    hal_coral_serdes_pxp_getAutoNego,
    hal_coral_serdes_pxp_setLocalAdvAbility,
    hal_coral_serdes_pxp_getLocalAdvAbility,
    hal_coral_serdes_pxp_getRemoteAdvAbility,
    hal_coral_serdes_pxp_setSpeed,
    hal_coral_serdes_pxp_getSpeed,
    NULL, /* setDuplex */
    NULL, /* getDuplex */
    hal_coral_serdes_pxp_getLinkStatus,
    NULL, /* getLoopBack */
    NULL, /* setLoopBack */
    NULL, /* setSmartSpeedDown */
    NULL, /* getSmartSpeedDown */
    NULL, /* setLedOnCtrl */
    NULL, /* getLedOnCtrl */
    NULL, /* testTxCompliance*/
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    hal_coral_serdes_pxp_setSerdesMode,
    hal_coral_serdes_pxp_getSerdesMode,
    hal_coral_serdes_setPhyLedCtrlMode,
    hal_coral_serdes_getPhyLedCtrlMode,
    hal_coral_serdes_setPhyLedForceState,
    hal_coral_serdes_getPhyLedForceState,
    hal_coral_serdes_setPhyLedForcePattCfg,
    hal_coral_serdes_getPhyLedForcePattCfg,
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
    hal_coral_serdes_pxp_dumpPortCnt,
    hal_coral_serdes_pxp_dumpDebugInfo,
};

AIR_ERROR_NO_T
hal_coral_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    if (param == HAL_CORAL_SERDES_ID_S1)
    {
        (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_int_coral_serdes_qp_func_vec;
    }
    else if (param == HAL_CORAL_SERDES_ID_S0)
    {
        (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_int_coral_serdes_pxp_func_vec;
    }
    else
    {
        return (AIR_E_BAD_PARAMETER);
    }

    return (AIR_E_OK);
}
