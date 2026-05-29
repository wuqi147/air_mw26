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

/* FILE NAME:  hal_sco_serdes.c
 * PURPOSE:
 *  Implement SERDES module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_serdes.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/sco/hal_sco_ifmon.h>
#include <hal/switch/sco/hal_sco_reg.h>
/* NAMING CONSTANT DECLARATIONS
 */
#define TOTAL_NUMBER_OF_REGISTER_SERDES   (15)
#define TOTAL_NUMBER_OF_REGISTER_QSGMII_1 (15)
#define TOTAL_NUMBER_OF_REGISTER_QSGMII_2 (5)
#define QSGMII_LAN_INDEX_MASK             (0x03)

#define HSGMII_HSI0_RESET_BIT_OFFSET       (13)
#define SERDES_PCS_CTRL_REG_1_DEFAUT_VALUE (0x0C9CC000)

#define SERDES_LINK_SPEED_1000M (0x00000002)
#define SERDES_LINK_SPEED_100M  (0x00000001)
#define SERDES_LINK_SPEED_10M   (0x00000000)
#define SERDES_LINK_DUPLEX_FULL (1)
#define SERDES_LINK_DUPLEX_HALF (0)
#define SERDES_AN_DONE_CLEAR    (0x4)

#define PCS_RX_SYNC_MASK               (0x1)
#define PCS_RX_SYNC_OFFSET             (5)
#define PCS_RX_AN_DONE                 (0x1)
#define PCS_INTR_AN_DONE               (0x1)
#define SGMII_AN_5_SPEED_MASK          (0x3)
#define SGMII_AN_5_SPEED_OFFSET        (10)
#define SGMII_AN_5_DUPLEX_MASK         (0x1)
#define SGMII_AN_5_DUPLEX_OFFSET       (12)
#define SGMII_AN_5_LINK_MASK           (0x1)
#define SGMII_AN_5_LINK_OFFSET         (15)
#define SGMII_1000BASEX_PAUSE_OFFSET   (7)
#define SGMII_1000BASEX_ASM_DIR_OFFSET (8)
#define SGMII_STS_CTRL_0               (0x4018)
#define SGMII_AN_0_AN_ENABLE_OFFSET    (12)
#define SGMII_AN_0_AN_ENABLE_MASK      (0x1)

#define HAL_SERDES_SFP_MAX_LED_COUNT (2)

/* MACRO FUNCTION DECLARATIONS
 */
#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__)                 \
    do                                                                                 \
    {                                                                                  \
        __base_addr__ = SERDES_ID_S0_BASE_ADDR + (SERDES_ADDR_OFFSER * __serdes_id__); \
    } while (0)

#define GET_PCS_RX_SYNC(__data__, __value__)      __value__ = ((__data__ >> (PCS_RX_SYNC_OFFSET)) & PCS_RX_SYNC_MASK)
#define GET_PCS_RX_AN_DONE(__data__, __value__)   __value__ = (__data__ & PCS_RX_AN_DONE)
#define GET_PCS_INTR_AN_DONE(__data__, __value__) __value__ = (__data__ & PCS_INTR_AN_DONE)
#define GET_SGMII_AN_5_LINK_STATUS(__data__, __value__)                         \
    __value__ = ((__data__ >> (SGMII_AN_5_LINK_OFFSET)) & SGMII_AN_5_LINK_MASK)
#define GET_SGMII_AN_5_LINK_SPEED(__data__, __value__)                            \
    __value__ = ((__data__ >> (SGMII_AN_5_SPEED_OFFSET)) & SGMII_AN_5_SPEED_MASK)
#define GET_SGMII_AN_5_LINK_DUPLEX(__data__, __value__)                             \
    __value__ = ((__data__ >> (SGMII_AN_5_DUPLEX_OFFSET)) & SGMII_AN_5_DUPLEX_MASK)
#define GET_SGMII_AN_0_AN_ENABLE(__data__, __value__)                                     \
    __value__ = ((__data__ >> (SGMII_AN_0_AN_ENABLE_OFFSET)) & SGMII_AN_0_AN_ENABLE_MASK)
#define GET_SGMII_SIGDET_CHECK(__data__, __value__)                                     \
    __value__ = ((__data__ >> (SERDES_RO_SIGDET_OUT_OFFT)) & SERDES_RO_SIGDET_OUT_MASK)

#define CLEAR_SERDES_AN_DONE_INTR(__unit__, __reg_base__)                                              \
    do                                                                                                 \
    {                                                                                                  \
        rv = aml_readReg(__unit__, (__reg_base__ + SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        if (AIR_E_OK == rv)                                                                            \
        {                                                                                              \
            data |= SERDES_AN_DONE_CLEAR;                                                              \
            aml_writeReg(__unit__, (__reg_base__ + SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
            data &= ~(SERDES_AN_DONE_CLEAR);                                                           \
            aml_writeReg(__unit__, (__reg_base__ + SERDES_MODE_INTERRUPT_REG), &data, sizeof(UI32_T)); \
        }                                                                                              \
    } while (0)

#define HAL_SERDES_AML_READ_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)           \
    do                                                                                       \
    {                                                                                        \
        __RETURN__ = aml_readReg(__UNIT__, __REG__, __DATA__, __SIZE__);                     \
        if (AIR_E_OK != __RETURN__)                                                          \
        {                                                                                    \
            DIAG_PRINT(HAL_DBG_ERR, "read reg 0x%08X fail, rc = %d\n", __REG__, __RETURN__); \
            return __RETURN__;                                                               \
        }                                                                                    \
    } while (0)

#define HAL_SERDES_AML_WRITE_REG(__UNIT__, __REG__, __DATA__, __SIZE__, __RETURN__)                          \
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

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_sco_serdes.c");

extern const UI32_T                        _hal_sco_serdes_dbg_cfg_info_size;
extern const HAL_SCO_SERDES_DBG_CFG_INFO_T _hal_sco_serdes_dbg_cfg_info[];

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS */
static UI32_T _hal_sco_serdes_init_table[TOTAL_NUMBER_OF_REGISTER_SERDES][SERDES_MODE_LAST] = {
    /* {          Register_Address, QSGMII    , HSGMII    , AN        , 1000M     , 100M      , 10M       }*/
    {           SERDES_XGMII_DBG_0, 0x00000002, 0x00000000, 0x00000002, 0x00000002, 0x00000002, 0x00000002},
    {           SERDES_XGMII_DBG_1, 0x00001111, 0x00001111, 0x0000FF11, 0x0000ff11, 0x0000ff11, 0x0000ffaa},
    {     SERDES_MSG_TX_CTRL_REG_1, 0x00000700, 0x00000700, 0x00000700, 0x00000700, 0x00000755, 0x000007aa},
    {   SERDES_SGMII_STS_CTL_REG_0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000014, 0x00000004},
    {     SERDES_MSG_RX_CTRL_REG_4, 0x00000700, 0x00000700, 0x00000700, 0x00000700, 0x00000755, 0x000007aa},
    {           SERDES_PHYA_REG_11, 0x0001281B, 0x00014817, 0x00014813, 0x00014813, 0x00014813, 0x00014813},
    {           SERDES_PHYA_REG_19, 0x60008007, 0x60004007, 0x60004007, 0x60004007, 0x60004007, 0x60004007},
    {           SERDES_PHYA_REG_80, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000010, 0x00000020},
    {SERDES_RATE_ADP_P0_CTRL_REG_0, 0x10000000, 0x00000008, 0x10000000, 0x10000000, 0x1000000c, 0x1000000f},
    {SERDES_RATE_ADP_P0_CTRL_REG_1, 0x010F010F, 0x0100010F, 0x010F010F, 0x010F010F, 0x010F010F, 0x010F010F},
    {              SERDES_AN_REG_0, 0x00009140, 0x00008140, 0x00009140, 0x00009140, 0x00009140, 0x00009140},
    {              SERDES_AN_REG_0, 0x00001140, 0x00000140, 0x00001140, 0x00001140, 0x00001140, 0x00001140},
    {      SERDES_MII_RA_AN_ENABLE, 0x00000000, 0x00000000, 0x0000000F, 0x00000000, 0x00000000, 0x00000000},
    {           SERDES_PHYA_REG_30, 0x00010050, 0x00010050, 0x00010450, 0x00010050, 0x00010050, 0x00010050},
    {             SERDES_AN_REG_13, 0x31120009, 0x31120009, 0x31120023, 0x31120009, 0x31120009, 0x31120009}
};

static UI32_T _hal_sco_serdes_qsgmii_init_table_1[TOTAL_NUMBER_OF_REGISTER_QSGMII_1][2] = {
    /* {     Register_Address, AN mode   }*/
    { SERDES_MII_RA_AN_ENABLE, 0x0000000F},
    {      SERDES_PHYA_REG_30, 0x00010450},
    {    SERDES_MSG_RX_CTRL_0, 0x00010011},
    {       SERDES_PHYA_REG_6, 0x0010001C},
    {       SERDES_PHYA_REG_9, 0x0E302508},
    {      SERDES_PHYA_REG_24, 0x11210000},
    {      SERDES_PHYA_REG_28, 0x0003C010},
    {       SERDES_PHYA_REG_7, 0x00000007},
    {      SERDES_PHYA_REG_14, 0x1A020807},
    {      SERDES_PHYA_REG_15, 0x00D05A1A},
    {       SERDES_PHYA_REG_8, 0x00000C3C},
    {      SERDES_PHYA_REG_13, 0x00000398},
    {      SERDES_PHYA_REG_62, 0x01A01501},
    {SERDES_RATE_ADAPT_CTRL_0, 0x0C000C00},
    {SERDES_MSG_TX_CTRL_REG_0, 0x00010020}
};

static UI32_T _hal_sco_serdes_qsgmii_init_table_2[TOTAL_NUMBER_OF_REGISTER_QSGMII_2][2] = {
    /* {  Register_Address, AN mode   }*/
    {     SERDES_AN_REG_13, 0x3112010B},
    {      SERDES_AN_REG_4, 0x00001801},
    {SERDES_PCS_CTRL_REG_6, 0x00000013},
    {      SERDES_AN_REG_0, 0x00009140},
    {      SERDES_AN_REG_0, 0x00001140},
};
/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES*/
static AIR_ERROR_NO_T
_hal_sco_serdes_configMode(
    const UI32_T        unit,
    const SERDES_ID_T   serdes_id,
    const SERDES_MODE_T mode)
{
    UI32_T         reg_addr = 0, reg_data = 0, lane_port = 0, base_addr = 0, count = 0, data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    for (count = 0; count < TOTAL_NUMBER_OF_REGISTER_SERDES; count++)
    {
        reg_addr = base_addr + _hal_sco_serdes_init_table[count][SERDES_MODE_REGISTER_ADDRESS];
        reg_data = _hal_sco_serdes_init_table[count][mode];

        rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
            return rv;
        }
    }

    if (SERDES_MODE_QSGMII == mode)
    {
        for (count = 0; count < TOTAL_NUMBER_OF_REGISTER_QSGMII_1; count++)
        {
            reg_addr = base_addr + _hal_sco_serdes_qsgmii_init_table_1[count][SERDES_MODE_REGISTER_ADDRESS];
            reg_data = _hal_sco_serdes_qsgmii_init_table_1[count][1];

            rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
                return rv;
            }
        }
        osal_delayUs(1000);
        for (lane_port = 0; lane_port < QSGMII_LANE_PORT_NUM; lane_port++)
        {
            for (count = 0; count < TOTAL_NUMBER_OF_REGISTER_QSGMII_2; count++)
            {
                reg_addr = base_addr + (_hal_sco_serdes_qsgmii_init_table_2[count][SERDES_MODE_REGISTER_ADDRESS] +
                                        (lane_port * QSGMII_LANE_OFFSET));
                reg_data = _hal_sco_serdes_qsgmii_init_table_2[count][1];

                rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
                if (rv != AIR_E_OK)
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data,
                               rv);
                    return rv;
                }
            }
        }
    }

    /* Reset port */
    reg_addr = NPSCU_BASE_ADDR + NPSCU_RST_CTRL2;

    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
        return rv;
    }

    reg_data = (reg_data | (0x2000 << serdes_id));

    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
        return rv;
    }

    osal_delayUs(5000);

    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
        return rv;
    }

    reg_data = (reg_data & ~(0x2000 << serdes_id));

    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
        return rv;
    }

    reg_addr = base_addr + SERDES_MSG_RX_LIK_STS_2;

    /* Bypass link status to MAC */
    if (SERDES_MODE_HSGMII != mode)
    {
        data = MSG_RX_LIK_STS_FC_BYPASS;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        }
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_configPolarity(
    const UI32_T            unit,
    const SERDES_ID_T       serdes_id,
    const SERDES_POLARITY_T polarity)
{
    UI32_T         reg_addr = 0;
    UI32_T         reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);
    reg_addr = reg_addr + SERDES_PHYA_REG_62;

    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
        return rv;
    }

    reg_data = ((reg_data & ~0x3) | polarity);

    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
        return rv;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_setting(
    const UI32_T            unit,
    const SERDES_ID_T       serdes_id,
    const SERDES_MODE_T     mode,
    const SERDES_POLARITY_T polarity)
{
    UI32_T         data = 0x12340000 | mode;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv = aml_writeReg(unit, 0x10005010, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x10005010 value to 0x%08X fail(%d)\n", data, rv);
        return rv;
    }

    if ((serdes_id >= SERDES_ID_LAST) || (mode >= SERDES_MODE_LAST) || (polarity >= SERDES_POLARITY_LAST))
    {
        return AIR_E_BAD_PARAMETER;
    }

    rv = _hal_sco_serdes_configMode(unit, serdes_id, mode);
    if (rv != AIR_E_OK)
    {
        return rv;
    }
    rv = _hal_sco_serdes_configPolarity(unit, serdes_id, polarity);
    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_getPortMode(
    const UI32_T   unit,
    const UI32_T   port,
    SERDES_MODE_T *ptr_mode)
{
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    SERDES_MODE_T       serdes_mode = SERDES_MODE_REGISTER_ADDRESS;
    SERDES_ID_T         serdes_id = SERDES_ID_S0;
    UI32_T              air_port = 0, count = 0;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

    if (AIR_INIT_PORT_TYPE_XSGMII == ptr_sdk_port_map_entry->port_type)
    {
        if (AIR_INIT_PORT_SPEED_2500M == ptr_sdk_port_map_entry->max_speed)
        {
            serdes_mode = SERDES_MODE_HSGMII;
        }

        serdes_id = ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (SERDES_MODE_REGISTER_ADDRESS == serdes_mode)
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
        if (1 == count)
        {
            serdes_mode = SERDES_MODE_SGMII_AN;
        }
        else
        {
            serdes_mode = SERDES_MODE_QSGMII;
        }
    }
    *ptr_mode = serdes_mode;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_checkChangeMode(
    const UI32_T   unit,
    const UI32_T   port,
    SERDES_MODE_T *ptr_mode,
    BOOL_T        *ptr_change)
{
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
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
            rv = aml_readReg(unit, (base_addr + SERDES_PHYA_REG_11), &serdes_speed, sizeof(serdes_speed));
            if (rv == AIR_E_OK)
            {
                serdes_speed &= SERDES_SPEED_MASK;
                if ((SERDES_SPEED_SETUP_SGMII == serdes_speed) && (HAL_PHY_SPEED_2500M == link_status.speed))
                {
                    *ptr_mode = SERDES_MODE_HSGMII;
                    *ptr_change = TRUE;
                }
                else if ((SERDES_SPEED_SETUP_HSGMII == serdes_speed) && (HAL_PHY_SPEED_2500M != link_status.speed))
                {
                    *ptr_mode = SERDES_MODE_SGMII_AN;
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
_hal_sco_serdes_changeMode(
    const UI32_T        unit,
    const UI32_T        port,
    const SERDES_MODE_T mode)
{
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    rv = _hal_sco_serdes_configMode(unit, serdes_id, mode);

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
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    HAL_PHY_LINK_STATUS_T link_status;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);

    /* get link speed */
    rv = hal_phy_getLinkStatus(unit, port, &link_status);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Get Port %u link status failed\n", port);
        return;
    }

    /* check current serdes type is not HSGMII, this work around only work on SGMII and QSGMII */
    rv = aml_readReg(unit, (base_addr + SERDES_PHYA_REG_11), &serdes_speed, sizeof(serdes_speed));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Get Port %u serdes speed failed\n", port);
        return;
    }
    serdes_speed &= SERDES_SPEED_MASK;
    if (SERDES_SPEED_SETUP_HSGMII != serdes_speed)
    {
        /* MULTIPHY_RATE_ADP_P0_CTRL_REG_0 */
        if (link_status.speed == HAL_PHY_SPEED_1000M)
        {
            rv = aml_readReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &reg_data,
                             sizeof(reg_data));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get Port %u SERDES_RATE_ADP_P0_CTRL_REG_0 failed\n", port);
                return;
            }

            reg_data &= ~(0xf);
            rv = aml_writeReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &reg_data,
                              sizeof(reg_data));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set Port %u SERDES_RATE_ADP_P0_CTRL_REG_0 to %x failed\n", port, reg_data);
                return;
            }
        }

        /* MULTIPHY_MII_RA_AN_ENABLE */
        rv = aml_readReg(unit, (base_addr + SERDES_MII_RA_AN_ENABLE), &reg_data, sizeof(reg_data));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "Get Port %u SERDES_MII_RA_AN_ENABLE failed\n", port);
            return;
        }

        if ((link_status.speed == HAL_PHY_SPEED_1000M) && (link)) /* Link up & speed is 1G, set force rate adaption */
        {
            reg_data &= ~(0x1 << ((phy_id) % 4));
        }
        else /* set rate adaption controlled by AN */
        {
            reg_data |= (0x1 << ((phy_id) % 4));
        }
        rv = aml_writeReg(unit, (base_addr + SERDES_MII_RA_AN_ENABLE), &reg_data, sizeof(reg_data));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "Set Port %u SERDES_MII_RA_AN_ENABLE to %x failed\n", port, reg_data);
            return;
        }
    }
}

static void
_hal_sco_serdes_linkChangeCallback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_DUPLEX_T      duplex;
    HAL_PHY_LINK_STATUS_T link_status;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI16_T                mac_port = 0, eee_data = 0;
    UI32_T                base_addr = 0, reg_addr = 0, data = 0, pre_data = 0, eee_serdes = 0;
    UI32_T                phy_id = 0, shift = 0, serdes_speed = 0;
    SERDES_MODE_T         serdes_mode = SERDES_MODE_REGISTER_ADDRESS;
    BOOL_T                need_change = FALSE;
    UI32_T                sgmii_sts_ctrl = 0, force_linkdown = 0;
    AIR_CFG_VALUE_T       serdes_op_mode;
    UI32_T                force_sync = 0;
    AIR_PORT_STATUS_T     port_status;

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
                rv = _hal_sco_serdes_checkChangeMode(unit, port, &serdes_mode, &need_change);
                if ((rv == AIR_E_OK) && (need_change == TRUE))
                {
                    rv = _hal_sco_serdes_changeMode(unit, port, serdes_mode);
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_WARN, "Set Port %u, serdes mode to %u failed\n", port, serdes_mode);
                        return;
                    }
                }
            }
        }

        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        reg_addr = PORT_MCR_BASE_ADDR + (mac_port * PORT_ADDR_OFFSET);

        rv = aml_readReg(unit, reg_addr, &data, sizeof(data));
        if (rv != AIR_E_OK)
        {
            return;
        }

        rv = hal_sco_ifmon_getPortStatus(unit, port, &port_status);
        if (AIR_E_OK != rv)
        {
            return;
        }

        if (data & PORT_FORCE_MODE_MASK)
        {
            /* handle serdes rate adaption mode in force mode */
            serdes_set_rate_adaption(unit, port, link);
            /* force mode don't need to update flow control status */

            /* eee status */
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
            HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
            sgmii_sts_ctrl = (base_addr + SGMII_STS_CTRL_0);
            rv = aml_readReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &eee_serdes,
                             sizeof(eee_serdes));
            if (rv == AIR_E_OK)
            {
                if (link)
                {
                    if (port_status.flags & AIR_PORT_STATUS_FLAGS_EEE)
                    {
                        if (eee_data)
                        {
                            eee_serdes |= (SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                        }
                        else
                        {
                            eee_serdes &= ~(SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                        }
                    }
                }
                else
                {
                    eee_data = 0;
                    eee_serdes &= ~(SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                }
            }
            /* In order to bypass LPI to MAC for 100M, it should set mii tx/rx eee enable bits
               in RATE_ADP_PN_CTRL_REG_N */
            rv |= aml_writeReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &eee_serdes,
                               sizeof(eee_serdes));
            return;
        }
        else
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);
            reg_addr = base_addr + SERDES_MSG_RX_LIK_STS_0;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(data));
            if (rv != AIR_E_OK)
            {
                return;
            }
            pre_data = data;

            serdes_set_rate_adaption(unit, port, link);

            if (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port))
            {
                rv = aml_readReg(unit, (base_addr + SERDES_PHYA_REG_11), &serdes_speed, sizeof(serdes_speed));
                if (rv == AIR_E_OK)
                {
                    serdes_speed &= SERDES_SPEED_MASK;
                    if (serdes_speed == SERDES_SPEED_SETUP_HSGMII)
                    {
                        rv = aml_readReg(unit, (base_addr + SERDES_PCS_CTRL_REG_1), &force_sync, sizeof(force_sync));
                        if (rv == AIR_E_OK)
                        {
                            if (link)
                            {
                                force_sync &= ~(SERDES_FORCE_SYNC_DISABLE);
                            }
                            else
                            {
                                force_sync |= SERDES_FORCE_SYNC_DISABLE;
                            }
                            rv = aml_writeReg(unit, (base_addr + SERDES_PCS_CTRL_REG_1), &force_sync,
                                              sizeof(force_sync));
                        }
                    }
                }
            }
            if (rv == AIR_E_OK)
            {
                /* eee status */
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                sgmii_sts_ctrl = (base_addr + SGMII_STS_CTRL_0);
                rv = aml_readReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &eee_serdes,
                                 sizeof(eee_serdes));
                if (rv == AIR_E_OK)
                {
                    if (link)
                    {
                        if (port_status.flags & AIR_PORT_STATUS_FLAGS_EEE)
                        {
                            if (eee_data)
                            {
                                eee_serdes |= (SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                            }
                            else
                            {
                                eee_serdes &= ~(SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                            }
                        }
                    }
                    else
                    {
                        eee_data = 0;
                        eee_serdes &= ~(SERDES_RA_P0_MII_RX_EEE_EN + SERDES_RA_P0_MII_TX_EEE_EN);
                    }
                }
            }
            /* In order to bypass LPI to MAC for 100M, it should set mii tx/rx eee enable bits
               in RATE_ADP_PN_CTRL_REG_N */
            rv |= aml_writeReg(unit, (base_addr + (SERDES_RATE_ADP_P0_CTRL_REG_0 + (phy_id % 4) * 8)), &eee_serdes,
                               sizeof(eee_serdes));
            if (rv != AIR_E_OK)
            {
                return;
            }

            if (link) /* Update side band information when port is link up */
            {
                /* check current serdes type is QSGMII or not */
                rv = aml_readReg(unit, (base_addr + SERDES_PHYA_REG_11), &serdes_speed, sizeof(serdes_speed));
                if (rv != AIR_E_OK)
                {
                    return;
                }
                serdes_speed &= SERDES_SPEED_MASK;
                if (SERDES_SPEED_SETUP_QSGMII == serdes_speed)
                {
                    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                    shift = (phy_id & QSGMII_LAN_INDEX_MASK) * 8;
                }
                else
                {
                    shift = 0;
                }
                data &= ~(HSGMII_LINK_STS_MASK << shift);
                if (port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX)
                {
                    data |= (HSGMII3_FC_TX_ON << shift);
                }

                if (port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)
                {
                    data |= (HSGMII3_FC_RX_ON << shift);
                }

                rv = hal_phy_getLinkStatus(unit, port, &link_status);
                if (rv != AIR_E_OK)
                {
                    return;
                }
                duplex = link_status.duplex;
                if (HAL_PHY_DUPLEX_FULL == duplex)
                {
                    data |= (SERDES_FULL_DUPLEX << shift);
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

static AIR_ERROR_NO_T
_hal_sco_serdes_disable_100base(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         reg_addr = 0, serdes_base = 0;
    UI32_T         data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);
    reg_addr = serdes_base + SERDES_SGMII_REG_PHYA_65;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~(BITS_RANGE(REG_100FX_TBI_REVERSE_OFF, REG_100FX_TBI_REVERSE_LENG));
    data |= BITS_OFF_L(REG_100FX_TBI_REVERSE_OFF, REG_100FX_TBI_REVERSE_OFF, REG_100FX_TBI_REVERSE_LENG);
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    reg_addr = serdes_base + SERDES_SGMII_MSG_TX_CTRL_0;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~REG_FORCE_100FX_SIGDET;
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    reg_addr = serdes_base + SERDES_SGMII_FPGA_MODE_CONT;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~(REG_HWTRAP_100FX_MUX_EN | REG_HWTRAP_100FX_MUX);
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    /* analog refinement */
    reg_addr = serdes_base + SERDES_PHYA_REG_24;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~REG_SSUSB_LN0_RXAFE_RESERVE;
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }
    reg_addr = serdes_base + SERDES_PHYA_REG_28;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~REG_PCIE_LN0_SIGDET_LPF;
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }
    reg_addr = serdes_base + SERDES_PHYA_REG_7;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data &= ~REG_FORCE_CDR_LCK2REF;
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_Check100fxMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *serdes_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         reg_addr = 0, reg_data = 0, serdes_base = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    reg_addr = serdes_base + SERDES_SGMII_MSG_TX_CTRL_0;
    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
        return rv;
    }
    if (reg_data & REG_FORCE_100FX_SIGDET)
    {
        *serdes_mode = HAL_PHY_SERDES_MODE_100BASE_FX;
    }
    else
    {
        *serdes_mode = HAL_PHY_SERDES_MODE_LAST;
    }
    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_serdes_dumpDbgRegister(
    const UI32_T                    unit,
    const SERDES_ID_T               serdes_id,
    const HAL_SCO_SERDES_DBG_CFG_T *ptr_serdes_cfg)
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
        HAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        serdes_regValue &=
            ~BITS_RANGE(ptr_serdes_cfg->data_lsb, ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        serdes_regValue |= BITS_OFF_L(ptr_serdes_cfg->reg_data, ptr_serdes_cfg->data_lsb,
                                      ((ptr_serdes_cfg->data_msb - ptr_serdes_cfg->data_lsb) + 1));
        DIAG_PRINT(HAL_DBG_INFO, "set serdes reg=%x, value=%x\n", serdes_reg, serdes_regValue);
        HAL_SERDES_AML_WRITE_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
    }
    else /* read */
    {
        HAL_SERDES_AML_READ_REG(unit, serdes_reg, &serdes_regValue, sizeof(serdes_regValue), rc);
        osal_printf("%35s       0x%8x       0x%08x \n", ptr_serdes_cfg->reg_name, serdes_reg, serdes_regValue);
    }

    return rc;
}

/* FUNCTION NAME:   hal_sco_serdes_init
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
hal_sco_serdes_init(
    const UI32_T unit,
    const UI32_T port)
{
    SERDES_MODE_T   serdes_mode = SERDES_MODE_REGISTER_ADDRESS;
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    SERDES_ID_T     serdes_id = SERDES_ID_S0;
    AIR_CFG_VALUE_T polarity;
    UI32_T          reg_addr = 0, reg_data = 0, base_addr = 0;
    const UI16_T    shift = HSGMII_HSI0_RESET_BIT_OFFSET;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    rv = _hal_sco_serdes_getPortMode(unit, port, &serdes_mode);
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
    reg_addr = NPSCU_BASE_ADDR + NPSCU_RST_CTRL2;
    rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }
    reg_data &= ~(1 << (shift + serdes_id));
    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    rv = _hal_sco_serdes_setting(unit, serdes_id, serdes_mode, polarity.value);
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    /* rx sync force on */
    reg_addr = base_addr + SERDES_PCS_CTRL_REG_1;
    reg_data = SERDES_PCS_CTRL_REG_1_DEFAUT_VALUE;
    rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        return AIR_E_NOT_INITED;
    }

    /* regsiter ifmon to know link change event */
    rv = hal_sco_ifmon_lookup(unit, _hal_sco_serdes_linkChangeCallback, NULL);
    if (rv == AIR_E_ENTRY_NOT_FOUND)
    {
        rv = hal_sco_ifmon_register(unit, _hal_sco_serdes_linkChangeCallback, NULL);
        if (rv != AIR_E_OK)
        {
            return AIR_E_NOT_INITED;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setAdminState
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
hal_sco_serdes_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0, reg_base = 0;
    UI32_T                data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    reg_addr = reg_base + SERDES_PHYA_REG_61;

    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg] Get port %d SERDES mode fail (%d)\n", port, rv);
        return rv;
    }

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        data &= ~(SERDES_POWER_DOWN | SERDES_TX_DISABLE);
    }
    else
    {
        data |= SERDES_POWER_DOWN;
        if ((HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode) || (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode))
        {
            data |= SERDES_TX_DISABLE;
        }
    }

    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    reg_addr = reg_base + SERDES_PCS_CTRL_REG_1;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    if (HAL_PHY_ADMIN_STATE_ENABLE == state)
    {
        data &= ~(SERDES_FORCE_SYNC_DISABLE);
    }
    else
    {
        data |= SERDES_FORCE_SYNC_DISABLE;
    }

    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getAdminState
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
hal_sco_serdes_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         reg_addr = 0;
    UI32_T         data = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

    reg_addr = reg_addr + SERDES_PHYA_REG_61;

    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    if (data & SERDES_POWER_DOWN)
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_DISABLE;
    }
    else
    {
        *ptr_state = HAL_PHY_ADMIN_STATE_ENABLE;
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setAutoNego
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
hal_sco_serdes_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0, reg_base = 0;
    UI32_T                data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;

    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);
    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
        return rv;
    }
    if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
    {
        return AIR_E_NOT_SUPPORT;
    }

    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode) /* 100 base-fx mode */
    {
        return AIR_E_OP_INVALID;
    }

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    reg_addr = reg_base + SERDES_AN_REG_0;

    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        data |= SERDES_AN_REG_0_ANENABLE;
    }
    else if (HAL_PHY_AUTO_NEGO_RESTART == auto_nego)
    {
        data |= SERDES_AN_REG_0_ANRESTART;
    }
    else
    {
        data &= ~(SERDES_AN_REG_0_ANENABLE);
    }

    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }

    if (port_cfg.value == 1)
    {
        /* For SGMII Force mode to AN mode (exclude HSGMII)*/
        if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
        {
            rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
            if (rv == AIR_E_OK)
            {
                if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
                {
                    rv = _hal_sco_serdes_changeMode(unit, port, SERDES_MODE_SGMII_AN);
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data,
                                   rv);
                        return rv;
                    }
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
                return rv;
            }
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getAutoNego
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
hal_sco_serdes_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;

    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);
    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
        return rv;
    }
    if ((port_cfg.value == 2) && (HAL_PHY_SERDES_MODE_SGMII == serdes_mode))
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    reg_addr = serdes_base + SERDES_AN_REG_13;

    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    if ((port_cfg.value == 1) && ((data & 0x3f) == SERDES_SGMII_FORCE_MODE)) /* Serdes force RA */
    {
        *ptr_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
    }
    else
    {
        /* Read SERDES_AN_REG_0  AN bits */
        reg_addr = serdes_base + SERDES_AN_REG_0;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
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

/* FUNCTION NAME:   hal_sco_serdes_setLocalAdvAbility
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
hal_sco_serdes_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
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

                reg_addr = reg_addr + SERDES_AN_REG_4;
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

/* FUNCTION NAME:   hal_sco_serdes_getLocalAdvAbility
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
hal_sco_serdes_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + SERDES_AN_REG_4;
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
        else if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
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

/* FUNCTION NAME:   hal_sco_serdes_getRemoteAdvAbility
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
hal_sco_serdes_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0;
    UI32_T                data = 0;

    ptr_adv->flags = 0;
    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK == rv)
    {
        if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            ptr_adv->flags |= HAL_PHY_AN_ADV_FLAGS_100FUDX;
        }
        else if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
            SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_addr);

            reg_addr = reg_addr + SERDES_AN_REG_5;
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

/* FUNCTION NAME:   hal_sco_serdes_setSpeed
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
hal_sco_serdes_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    UI32_T                reg_data = 0, reg_addr = 0, reg_base = 0, auto_nego = 0;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
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

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    if ((rv = hal_sco_serdes_getAutoNego(unit, port, &auto_nego)) != AIR_E_OK)
    {
        return rv;
    }
    if (HAL_PHY_AUTO_NEGO_ENABLE == auto_nego)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Port %u AN mode is enable, not allow force speed setting\n", port);
        return AIR_E_NOT_SUPPORT;
    }

    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
        return rv;
    }

    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        if (HAL_PHY_SPEED_100M != speed)
        {
            return AIR_E_OP_INVALID;
        }
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
                force_speed = SERDES_MODE_SGMII_FORCE_10M;
                break;
            case HAL_PHY_SPEED_100M:
                force_speed = SERDES_MODE_SGMII_FORCE_100M;
                break;
            case HAL_PHY_SPEED_1000M:
                force_speed = SERDES_MODE_SGMII_FORCE_1000M;
                break;
            case HAL_PHY_SPEED_2500M:
                force_speed = SERDES_MODE_HSGMII;
                break;
            default:
                return AIR_E_BAD_PARAMETER;
        }
        /* Set serdes RA by speed  */
        rv = _hal_sco_serdes_changeMode(unit, port, force_speed);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "Set Port %u, serdes mode to %u failed\n", port, force_speed);
            return rv;
        }

        /* sgmii reset phy */
        reg_addr = reg_base + SERDES_AN_REG_0;
        rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
            return rv;
        }
        reg_data |= SERDES_AN_REG_0_RESET;
        rv = aml_writeReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, reg_data, rv);
            return rv;
        }
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getSpeed
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
hal_sco_serdes_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    UI32_T                reg_data = 0, reg_addr = 0, reg_base = 0;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI8_T                 force_speed = 0;
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T serdes_mode = 0;
    AIR_CFG_VALUE_T       port_cfg;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    rv = hal_sco_serdes_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Port (%d), serdes mode read fail !\n", port);
        return rv;
    }
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        *ptr_speed = HAL_PHY_SPEED_100M;
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
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

        reg_addr = reg_base + SERDES_RATE_ADP_P0_CTRL_REG_0;
        rv = aml_readReg(unit, reg_addr, &reg_data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, reg_data);
            return rv;
        }
        force_speed = (reg_data & 0xf);

        if (force_speed == (SERDES_RA_P0_MII_RA_RX_EN | SERDES_RA_P0_MII_RA_TX_EN | SERDES_RA_P0_MII_RA_RX_MODE |
                            SERDES_RA_P0_MII_RA_TX_MODE))
        {
            *ptr_speed = HAL_PHY_SPEED_10M;
        }
        else if (force_speed == (SERDES_RA_P0_MII_RA_RX_EN | SERDES_RA_P0_MII_RA_TX_EN))
        {
            *ptr_speed = HAL_PHY_SPEED_100M;
        }
        else if (force_speed == (SERDES_RA_P0_MII_RA_RX_EN))
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

/* FUNCTION NAME:   hal_sco_serdes_getLinkStatus
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
hal_sco_serdes_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    UI8_T                 link_status = 0, an_done = 0, ls_link = 0, an_intr = 0, sig_det = 0;
    UI8_T                 speed = 0, duplex = 0, an_enable = 0;
    UI32_T                serdes_speed = 0, sgmii_mode = 0;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_base = 0, reg_data = 0;
    UI32_T                data = 0, data2 = 0, pmcr = 0, force_mode = 0, mac_port = 0, force_speed = 0, data3 = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode = 0;
    AIR_CFG_VALUE_T       port_cfg;
    UI8_T                 err_detect = 0, err_cnt = 0;

    osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    port_cfg.value = 0;
    port_cfg.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_PORT_OPTION, &port_cfg);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, reg_base);

    rv = aml_readReg(unit, (reg_base + SERDES_RO_AD_INTERFACE), &data3, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_RO_AD_INTERFACE 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_RO_AD_INTERFACE), rv, data3);
        return rv;
    }
    GET_SGMII_SIGDET_CHECK(data3, sig_det);

    _hal_sco_serdes_Check100fxMode(unit, port, &serdes_mode);
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        rv = aml_readReg(unit, (reg_base + SERDES_PHYA_REG_61), &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n",
                       (reg_base + SERDES_PHYA_REG_61), rv, data);
            return rv;
        }
        /* 100 base-fx there is no link status so setting by manual */
        if (data & SERDES_POWER_DOWN)
        {
            ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
        }
        else
        {
            /* read error cnt */
            HAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_SERDES_RX100FX_PHY_ERR_STATUS), &data, sizeof(UI32_T), rv);
            err_cnt = BITS_OFF_R(data, HAL_SERDES_RO_RCV_ERR_CNT_OFFSET, HAL_SERDES_RO_RCV_ERR_CNT_MASK);

            /* read error detect mask */
            HAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_SERDES_RX100FX_PHY_STATUS), &data, sizeof(UI32_T), rv);
            err_detect = BITS_OFF_R(data, HAL_SERDES_RO_ERR_DETECT_OFFSET, HAL_SERDES_RO_ERR_DETECT_MASK);

            if (sig_det && (err_cnt == 0) && (err_detect == 0))
            {
                ptr_status->flags |= HAL_PHY_LINK_STATUS_FLAGS_LINK_UP;
            }
            else
            {
                ptr_status->flags &= ~(HAL_PHY_LINK_STATUS_FLAGS_LINK_UP);
            }

            /* clear rvv err cnt */
            HAL_SERDES_AML_READ_REG(unit, (reg_base + HAL_SERDES_RX100FX_PHY_CTRL), &reg_data, sizeof(UI32_T), rv);
            reg_data |= (HAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_SERDES_RG_ERR_DETECT_CLR);
            HAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_SERDES_RX100FX_PHY_CTRL), &reg_data, sizeof(UI32_T), rv);
            reg_data &= ~(HAL_SERDES_RG_RCV_ERR_CNT_CLR | HAL_SERDES_RG_ERR_DETECT_CLR);
            HAL_SERDES_AML_WRITE_REG(unit, (reg_base + HAL_SERDES_RX100FX_PHY_CTRL), &reg_data, sizeof(UI32_T), rv);

            /* update speed and duplex*/
            ptr_status->speed = HAL_PHY_SPEED_100M;
            ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
        }
        return rv;
    }

    /* Get PMCR setting */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rv = aml_readReg(unit, PMCR(mac_port), &pmcr, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read PMCR 0x%08X fail (%d), value is 0x%08X\n", PMCR(mac_port), rv, pmcr);
        return rv;
    }
    force_mode = !!(pmcr & BIT(MCR_FORCE_MODE_OFFT));
    /* Get speed by serdes config */
    rv = aml_readReg(unit, (reg_base + SERDES_PHYA_REG_11), &serdes_speed, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_PHYA_REG_11 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_PHYA_REG_11), rv, serdes_speed);
        return rv;
    }
    rv = aml_readReg(unit, (reg_base + SERDES_AN_REG_13), &sgmii_mode, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_AN_REG_13 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_AN_REG_13), rv, sgmii_mode);
        return rv;
    }
    rv = aml_readReg(unit, (reg_base + SERDES_PCS_STATE_REG_2), &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_PCS_STATE_REG_2 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_PCS_STATE_REG_2), rv, data);
        return rv;
    }
    GET_PCS_RX_SYNC(data, link_status);
    GET_PCS_RX_AN_DONE(data, an_done);
    rv = aml_readReg(unit, (reg_base + SERDES_AN_REG_5), &data2, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_AN_REG_5 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_AN_REG_5), rv, data2);
        return rv;
    }
    GET_SGMII_AN_5_LINK_STATUS(data2, ls_link);
    GET_SGMII_AN_5_LINK_SPEED(data2, speed);
    GET_SGMII_AN_5_LINK_DUPLEX(data2, duplex);

    rv = aml_readReg(unit, (reg_base + SERDES_AN_REG_0), &data3, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read SERDES_AN_REG_0 0x%08X fail (%d), value is 0x%08X\n",
                   (reg_base + SERDES_AN_REG_0), rv, data3);
        return rv;
    }
    GET_SGMII_AN_0_AN_ENABLE(data3, an_enable);

    link_status = (link_status & sig_det);

    if (force_mode)
    {
        /* if sgmii_mode == 1000baseX & reg 0x0 bit 12 = 1, than check an-bits */
        if (link_status)
        {
            if ((!(sgmii_mode & SERDES_SGMII_ENABLE)) & an_enable) /* for 1000baseX */
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
            serdes_speed &= SERDES_SPEED_MASK;
            rv = hal_sco_serdes_getSpeed(unit, port, &force_speed);
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Get serdes speed fail (%d)\n", rv);
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
        serdes_speed &= SERDES_SPEED_MASK;
        if (SERDES_SPEED_SETUP_HSGMII == serdes_speed)
        {
            if (link_status)
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
        else if (SERDES_SPEED_SETUP_SGMII == serdes_speed)
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

            if (sgmii_mode &
                SERDES_SGMII_ENABLE) /* SGMII AN mode (get speed and duplex information from serdes AN result) */
            {
                if (SERDES_LINK_SPEED_1000M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_1000M;
                }
                else if (SERDES_LINK_SPEED_100M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_100M;
                }
                else if (SERDES_LINK_SPEED_10M == speed)
                {
                    ptr_status->speed = HAL_PHY_SPEED_10M;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Get speed fail (%d), value is 0x%08X\n", rv, speed);
                    return rv;
                }

                if (SERDES_LINK_DUPLEX_FULL == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_FULL;
                }
                else if (SERDES_LINK_DUPLEX_HALF == duplex)
                {
                    ptr_status->duplex = HAL_PHY_DUPLEX_HALF;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Get duplex fail (%d), value is 0x%08X\n", rv, duplex);
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
                    rv = aml_readReg(unit, (reg_base + SERDES_PCS_INT_STATE_REG), &data, sizeof(UI32_T));
                    if (rv != AIR_E_OK)
                    {
                        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n",
                                   (reg_base + SERDES_PCS_INT_STATE_REG), rv, data);
                        return rv;
                    }
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
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Get serdes speed fail (%d), value is 0x%08X\n", rv, serdes_speed);
            return rv;
        }
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setSerdesMode
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
hal_sco_serdes_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         reg_addr = 0, serdes_base = 0, efifo_mode_addr = 0;
    UI32_T         data = 0, phy_id, tx_disable = 0;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    /* Run serdes init script */
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode) /* 100 base-fx mode */
    {
        reg_addr = serdes_base + SERDES_SGMII_REG_PHYA_65;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(BITS_RANGE(REG_100FX_TBI_REVERSE_OFF, REG_100FX_TBI_REVERSE_LENG));
        data |= BITS_OFF_L(REG_100FX_TBI_REVERSE, REG_100FX_TBI_REVERSE_OFF, REG_100FX_TBI_REVERSE_LENG);
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }

        reg_addr = serdes_base + SERDES_SGMII_MSG_TX_CTRL_0;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data |= REG_FORCE_100FX_SIGDET;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }

        reg_addr = serdes_base + SERDES_SGMII_FPGA_MODE_CONT;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data |= (REG_HWTRAP_100FX_MUX_EN | REG_HWTRAP_100FX_MUX);
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
        /* 100 base-fx EFIFO control */
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
        efifo_mode_addr = EFIFO_MODE_100_FX_BASE + (phy_id * EFIFO_100_FX_REG_OFFSET);
        reg_addr = efifo_mode_addr + EFIFO_MODE_100_FX_REG;
        data = EFIFO_100_FX_MODE;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
        reg_addr = efifo_mode_addr + EFIFO_CTRL_100_FX_REG;
        data = EFIFO_100_FX_CTRL;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }

        /* serdes disable AN-bit and set force rg */
        reg_addr = serdes_base + SERDES_AN_REG_0;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(SERDES_AN_REG_0_ANENABLE);
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
        reg_addr = serdes_base + SERDES_AN_REG_13;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data &= ~(BITS_RANGE(REG_IF_MODE_OFFT, REG_IF_MODE_LENG));
        data |= BITS_OFF_L(REG_IF_MODE_FORCE, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }

        /* analog refinement */
        reg_addr = serdes_base + SERDES_PHYA_REG_24;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data |= REG_SSUSB_LN0_RXAFE_RESERVE;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
        reg_addr = serdes_base + SERDES_PHYA_REG_28;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data |= REG_PCIE_LN0_SIGDET_LPF;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
        reg_addr = serdes_base + SERDES_PHYA_REG_7;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
            return rv;
        }
        data |= REG_FORCE_CDR_LCK2REF;
        rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
            return rv;
        }
    }
    else if (HAL_PHY_SERDES_MODE_HSGMII == serdes_mode)   /* HSGMII mode */
    {
        rv = _hal_sco_serdes_disable_100base(unit, port); /* disbale 100 base-fx */
        if (rv != AIR_E_OK)
        {
            return rv;
        }
        rv = _hal_sco_serdes_changeMode(unit, port, SERDES_MODE_HSGMII);
    }
    else                                                  /* 100base-fx, 1000baseX and SGMII mode */
    {
        rv = _hal_sco_serdes_disable_100base(unit, port); /* disbale 100 base-fx */
        if (rv != AIR_E_OK)
        {
            return rv;
        }
        rv = _hal_sco_serdes_changeMode(unit, port, SERDES_MODE_SGMII_AN);
        /* Config 1000baseX and SGMII */
        if (HAL_PHY_SERDES_MODE_1000BASE_X == serdes_mode)
        {
            rv = aml_readReg(unit, (serdes_base + SERDES_PHYA_REG_61), &tx_disable, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv,
                           tx_disable);
                return rv;
            }
            tx_disable |= SERDES_TX_DISABLE;
            rv = aml_writeReg(unit, (serdes_base + SERDES_PHYA_REG_61), &tx_disable, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, tx_disable, rv);
                return rv;
            }
            osal_delayUs(250000);

            reg_addr = serdes_base + SERDES_AN_REG_13;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
                return rv;
            }

            data &= ~(BITS_RANGE(REG_IF_MODE_OFFT, REG_IF_MODE_LENG));
            data |= BITS_OFF_L(REG_IF_MODE_1000BASEX, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
            rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
                return rv;
            }

            reg_addr = serdes_base + SERDES_AN_REG_4;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
                return rv;
            }
            data &= ~(BIT(REG_TX_ABILITY_SGMII_OFFT));
            data |= (BIT(REG_1000BASEX_FULL_DUPLEX_OFFT) | BIT(SGMII_1000BASEX_PAUSE_OFFSET) |
                     BIT(SGMII_1000BASEX_ASM_DIR_OFFSET));
            rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
                return rv;
            }

            tx_disable &= ~(SERDES_TX_DISABLE);
            rv = aml_writeReg(unit, (serdes_base + SERDES_PHYA_REG_61), &tx_disable, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, tx_disable, rv);
                return rv;
            }
        }
        else if (HAL_PHY_SERDES_MODE_SGMII == serdes_mode)
        {
            reg_addr = serdes_base + SERDES_AN_REG_13;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
                return rv;
            }

            data &= ~(BITS_RANGE(REG_IF_MODE_OFFT, REG_IF_MODE_LENG));
            data |= BITS_OFF_L(REG_IF_MODE_SGMII, REG_IF_MODE_OFFT, REG_IF_MODE_LENG);
            rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
                return rv;
            }

            reg_addr = serdes_base + SERDES_AN_REG_4;
            rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
                return rv;
            }
            data &= ~(BIT(REG_1000BASEX_FULL_DUPLEX_OFFT));
            data |= BIT(REG_TX_ABILITY_SGMII_OFFT);
            rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
            if (rv != AIR_E_OK)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
                return rv;
            }
        }
    }
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Set Port %u, serdes mode to %u failed\n", port, serdes_mode);
        return rv;
    }
    /* serdes re-AN */
    reg_addr = serdes_base + SERDES_AN_REG_0;
    rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }
    data |= BIT(REG_SGMII_AN_RESTART_OFFT);
    rv = aml_writeReg(unit, reg_addr, &data, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Write Reg 0x%08X value to 0x%08X fail(%d)\n", reg_addr, data, rv);
        return rv;
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getSerdesMode
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
hal_sco_serdes_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T        rv = AIR_E_OK;
    SERDES_ID_T           serdes_id = SERDES_ID_S0;
    UI32_T                reg_addr = 0, serdes_base = 0;
    UI32_T                data = 0, serdes_speed = 0;
    HAL_PHY_SERDES_MODE_T serdes_mode = 0;

    _hal_sco_serdes_Check100fxMode(unit, port, &serdes_mode);
    if (HAL_PHY_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_100BASE_FX;
        return rv;
    }

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, serdes_base);

    reg_addr = serdes_base + SERDES_PHYA_REG_11;
    rv = aml_readReg(unit, reg_addr, &serdes_speed, sizeof(UI32_T));
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
        return rv;
    }

    serdes_speed &= SERDES_SPEED_MASK;
    if (SERDES_SPEED_SETUP_HSGMII == serdes_speed)
    {
        *ptr_serdes_mode = HAL_PHY_SERDES_MODE_HSGMII;
    }
    else
    {
        reg_addr = serdes_base + SERDES_AN_REG_13;
        rv = aml_readReg(unit, reg_addr, &data, sizeof(UI32_T));
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] Read Reg 0x%08X fail (%d), value is 0x%08X\n", reg_addr, rv, data);
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
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedCtrlMode
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
hal_sco_serdes_setPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (SERDES_SFP_LED_CTRL_MODE_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &= ~(BITS_OFF_L(SERDES_SFP_LED_CTRL_MODE_MASK, offset, SERDES_SFP_LED_CTRL_MODE_WIDTH));
        led_config |= BITS_OFF_L(ctrl_mode, offset, SERDES_SFP_LED_CTRL_MODE_WIDTH);
        rv = aml_writeReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedCtrlMode
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
hal_sco_serdes_getPhyLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (SERDES_SFP_LED_CTRL_MODE_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_ctrl_mode = BITS_OFF_R(led_config, offset, SERDES_SFP_LED_CTRL_MODE_WIDTH);
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedForceState
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
hal_sco_serdes_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (SERDES_SFP_LED_FORCE_STATE_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &= ~(BITS_OFF_L(SERDES_SFP_LED_FORCE_STATE_MASK, offset, SERDES_SFP_LED_FORCE_STATE_WIDTH));
        led_config |= BITS_OFF_L(state, offset, SERDES_SFP_LED_FORCE_STATE_WIDTH);
        rv = aml_writeReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedForceState
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
hal_sco_serdes_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (SERDES_SFP_LED_FORCE_STATE_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_state = BITS_OFF_R(led_config, offset, SERDES_SFP_LED_FORCE_STATE_WIDTH);
    }

    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_setPhyLedForcePattCfg
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
hal_sco_serdes_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);

        offset = (SERDES_SFP_LED_PATT_CFG_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        led_config &= ~(BITS_OFF_L(SERDES_SFP_LED_PATT_CFG_MASK, offset, SERDES_SFP_LED_PATT_CFG_WIDTH));
        led_config |= BITS_OFF_L(pattern, offset, SERDES_SFP_LED_PATT_CFG_WIDTH);
        rv = aml_writeReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_getPhyLedForcePattCfg
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
hal_sco_serdes_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    SERDES_ID_T    serdes_id = SERDES_ID_S0;
    UI32_T         led_config = 0, offset = 0;

    HAL_CHECK_PARAM((led_id >= HAL_SERDES_SFP_MAX_LED_COUNT), AIR_E_BAD_PARAMETER);

    rv = aml_readReg(unit, SERDES_SFP_LED_CFG_REG, &led_config, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        offset = (SERDES_SFP_LED_PATT_CFG_BIT + (SERDES_SFP_LED_CFG_PORT_OFFSET * serdes_id) +
                  (SERDES_SFP_LED_CFG_LED_ID_OFFSET * led_id));
        *ptr_pattern = BITS_OFF_R(led_config, offset, SERDES_SFP_LED_PATT_CFG_WIDTH);
    }
    return rv;
}

/* FUNCTION NAME:   hal_sco_serdes_dumpPortCnt
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
hal_sco_serdes_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI16_T                    idx, rg_idx;
    SERDES_ID_T               serdes_id = SERDES_ID_S0;
    HAL_SCO_SERDES_DBG_CFG_T *ptr_serdes_dbg_list;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    osal_printf("%25s        %16s        %9s \n", "Register", "Addr", "Value");
    for (idx = 0; idx < _hal_sco_serdes_dbg_cfg_info_size; idx++)
    {
        if (_hal_sco_serdes_dbg_cfg_info[idx].serdes_interface & param)
        {
            for (rg_idx = 0; rg_idx < _hal_sco_serdes_dbg_cfg_info[idx].serdes_register_cnt; rg_idx++)
            {
                ptr_serdes_dbg_list =
                    (HAL_SCO_SERDES_DBG_CFG_T *)(_hal_sco_serdes_dbg_cfg_info[idx].ptr_serdes_dbg_cfg_map + rg_idx);

                rc = _hal_sco_serdes_dumpDbgRegister(unit, serdes_id, ptr_serdes_dbg_list);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set _hal_sco_serdes_dumpDbgRegister fail, rc = %d\n", rc);
                    break;
                }
            }
        }
    }

    return rc;
}

const static HAL_PHY_DRIVER_T _int_sco_serdes_func_vec = {
    /* note: if function not support, fill "NULL". */
    hal_sco_serdes_init,
    hal_sco_serdes_setAdminState,
    hal_sco_serdes_getAdminState,
    hal_sco_serdes_setAutoNego,
    hal_sco_serdes_getAutoNego,
    hal_sco_serdes_setLocalAdvAbility,
    hal_sco_serdes_getLocalAdvAbility,
    hal_sco_serdes_getRemoteAdvAbility,
    hal_sco_serdes_setSpeed,
    hal_sco_serdes_getSpeed,
    NULL, /* setDuplex */
    NULL, /* getDuplex */
    hal_sco_serdes_getLinkStatus,
    NULL, /* getLoopBack */
    NULL, /* setLoopBack */
    NULL, /* setSmartSpeedDown */
    NULL, /* getSmartSpeedDown */
    NULL, /* setLedOnCtrl */
    NULL, /* getLedOnCtrl */
    NULL, /* testTxCompliance*/
    NULL, /* setComboMode */
    NULL, /* getComboMode */
    hal_sco_serdes_setSerdesMode,
    hal_sco_serdes_getSerdesMode,
    hal_sco_serdes_setPhyLedCtrlMode,
    hal_sco_serdes_getPhyLedCtrlMode,
    hal_sco_serdes_setPhyLedForceState,
    hal_sco_serdes_getPhyLedForceState,
    hal_sco_serdes_setPhyLedForcePattCfg,
    hal_sco_serdes_getPhyLedForcePattCfg,
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
    hal_sco_serdes_dumpPortCnt,
    NULL, /* dumpDebugInfo*/
};

AIR_ERROR_NO_T
hal_sco_serdes_getDriver(
    HAL_PHY_DRIVER_T **pptr_hal_driver,
    UI32_T             param)
{
    (*pptr_hal_driver) = (HAL_PHY_DRIVER_T *)&_int_sco_serdes_func_vec;

    return (AIR_E_OK);
}
