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

/* FILE NAME:  hal_sco_port.c
 * PURPOSE:
 *  Implement Port module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_port.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/sco/hal_sco_ifmon.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <hal/switch/sco/hal_sco_stag.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_PORT_MCR_DEFAULT_VALUE (0x001591F0)

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SCO_PORT_SET_REG_BIT(cond, reg, bit) \
    do                                            \
    {                                             \
        if (TRUE == (cond))                       \
        {                                         \
            (reg) |= (bit);                       \
        }                                         \
        else                                      \
        {                                         \
            (reg) &= ~(bit);                      \
        }                                         \
    } while (0)

#define _HAL_SCO_PORT_SET_FLAGS(_src_struct_, _src_flag_, _dst_struct_, _dst_flag_) \
    do                                                                              \
    {                                                                               \
        if ((_src_struct_)->flags & (_src_flag_))                                   \
        {                                                                           \
            (_dst_struct_)->flags |= (_dst_flag_);                                  \
        }                                                                           \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PORT, "hal_sco_port.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: _hal_sco_port_updateMac
 * PURPOSE:
 *      Update MCR force link status in force mode
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      ptr_cookie      --  Cookie for additional information
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */

static void
_hal_sco_port_updateMac(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    UI32_T            reg_mcr = 0;
    UI32_T            reg_msr = 0;
    UI32_T            mac_port = 0;
    UI32_T            port_speed = 0;
    HAL_PHY_OP_MODE_T phy_mode;
    AIR_PORT_STATUS_T port_status;
    AIR_ERROR_NO_T    rv;
    AIR_CFG_VALUE_T   ignore_an;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /* Get port link status from ifmon DB */
    osal_memset(&port_status, 0, sizeof(AIR_PORT_STATUS_T));
    hal_sco_ifmon_getPortStatus(unit, port, &port_status);

    if (HAL_IS_FORCE_MCR_PORT_VALID(unit, port))
    {
        aml_readReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
        reg_mcr |= BIT(MCR_FORCE_MODE_OFFT);

        if (TRUE == link)
        {
            aml_readReg(unit, PMSR(mac_port), &reg_msr, sizeof(reg_msr));
            if (AIR_PORT_DUPLEX_HALF == port_status.duplex)
            {
                reg_mcr &= ~BIT(MCR_FORCE_DUPLEX_OFFT);
            }
            else
            {
                reg_mcr |= BIT(MCR_FORCE_DUPLEX_OFFT);
            }

            reg_mcr &= ~BIT(MCR_EEE_1G_EN_OFFT);
            reg_mcr &= ~BIT(MCR_EEE_100_EN_OFFT);
            reg_mcr &= ~BIT(MCR_EEE_2P5G_EN_OFFT);
            rv = hal_phy_getPhyOpMode(unit, port, &phy_mode);
            if (AIR_E_OK == rv)
            {
                if (HAL_PHY_OP_MODE_NORMAL == phy_mode)
                {
                    if (AIR_PORT_STATUS_FLAGS_EEE & port_status.flags)
                    {
                        if (AIR_PORT_SPEED_1000M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_1G_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_100M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_100_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_2500M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_2P5G_EN_OFFT);
                        }
                    }
                }
            }

            /* Update Flow Ctrl */
            if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX) & port_status.flags)
            {
                reg_mcr |= BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
                reg_mcr |= BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX & port_status.flags) != 0)
            {
                reg_mcr |= BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX & port_status.flags) != 0)
            {
                reg_mcr |= BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
            }
            else
            {
                reg_mcr &= ~BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
                reg_mcr &= ~BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
            }

            osal_memset(&ignore_an, 0, sizeof(AIR_CFG_VALUE_T));
            ignore_an.value = 0;
            rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_BACKPRES_CTRL_MODE, &ignore_an);
            if (AIR_E_OK == rv)
            {
                if (0 == ignore_an.value)
                {
                    /* handle half mode back pressure */
                    if (AIR_PORT_DUPLEX_HALF == port_status.duplex)
                    {
                        if (reg_mcr & BIT(MCR_FORCE_FLOWCTRL_TX_OFFT))
                        {
                            reg_mcr |= BIT(MCR_BACK_PRESSURE_OFFT);
                        }
                        else
                        {
                            reg_mcr &= ~BIT(MCR_BACK_PRESSURE_OFFT);
                        }
                    }
                }
            }
            /* Update speed */
            switch (port_status.speed)
            {
                case AIR_PORT_SPEED_10M:
                    port_speed = HAL_PHY_SPEED_10M;
                    break;
                case AIR_PORT_SPEED_100M:
                    port_speed = HAL_PHY_SPEED_100M;
                    break;
                case AIR_PORT_SPEED_1000M:
                    port_speed = HAL_PHY_SPEED_1000M;
                    break;
                case AIR_PORT_SPEED_2500M:
                    port_speed = HAL_PHY_SPEED_2500M;
                    break;
                default:
                    port_speed = HAL_PHY_SPEED_100M;
                    break;
            }
            reg_mcr &= ~BITS_RANGE(MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
            reg_mcr |= BITS_OFF_L(port_speed, MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
            /* Update link status */
            reg_mcr |= BIT(MCR_FORCE_LINK_OFFT);
        }
        else
        {
            /* Disable EEE when the port is link down */
            rv = hal_phy_getPhyOpMode(unit, port, &phy_mode);
            if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_NORMAL == phy_mode))
            {
                reg_mcr &= ~BIT(MCR_EEE_1G_EN_OFFT);
            }
            /* Update link status */
            reg_mcr &= ~BIT(MCR_FORCE_LINK_OFFT);
        }

        aml_writeReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
        DIAG_PRINT(HAL_DBG_INFO, "Update pmcr = 0x%x\n", reg_mcr);
    }
    else
    {
        /* Check MAC force mode */
        aml_readReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
        if (reg_mcr & BIT(MCR_FORCE_MODE_OFFT))
        {
            /* Force mode */
            /* Modify force link status in force mode only */
            if (TRUE == link)
            {
                reg_mcr |= BIT(MCR_FORCE_LINK_OFFT);
            }
            else
            {
                reg_mcr &= ~BIT(MCR_FORCE_LINK_OFFT);
            }
        }
        else
        {
            /* AN mode */
            /* Modify back pressure in half duplex mode */
            if (TRUE == link)
            {
                osal_memset(&ignore_an, 0, sizeof(AIR_CFG_VALUE_T));
                ignore_an.value = 0;
                rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_BACKPRES_CTRL_MODE, &ignore_an);
                if (AIR_E_OK == rv)
                {
                    if (0 == ignore_an.value)
                    {
                        aml_readReg(unit, PMSR(mac_port), &reg_msr, sizeof(reg_msr));
                        if (!(reg_msr & BIT(REG_PORT_DUPLEX_OFFT)))
                        {
                            if (reg_msr & BIT(REG_PORT_TX_FLOWCTRL_OFFT))
                            {
                                reg_mcr |= BIT(MCR_BACK_PRESSURE_OFFT);
                            }
                            else
                            {
                                reg_mcr &= ~BIT(MCR_BACK_PRESSURE_OFFT);
                            }
                        }
                    }
                }

                reg_mcr &= ~BIT(MCR_EEE_1G_EN_OFFT);
                reg_mcr &= ~BIT(MCR_EEE_100_EN_OFFT);
                reg_mcr &= ~BIT(MCR_EEE_2P5G_EN_OFFT);
                rv = hal_phy_getPhyOpMode(unit, port, &phy_mode);
                if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_NORMAL == phy_mode))
                {
                    if (AIR_PORT_STATUS_FLAGS_EEE & port_status.flags)
                    {
                        if (AIR_PORT_SPEED_1000M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_1G_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_100M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_100_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_2500M == port_status.speed)
                        {
                            reg_mcr |= BIT(MCR_EEE_2P5G_EN_OFFT);
                        }
                    }
                }
            }
            else
            {
                rv = hal_phy_getPhyOpMode(unit, port, &phy_mode);
                if ((AIR_E_OK == rv) && (HAL_PHY_OP_MODE_NORMAL == phy_mode))
                {
                    reg_mcr &= ~BIT(MCR_EEE_1G_EN_OFFT);
                }
            }
        }
        aml_writeReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
    }
}

static AIR_ERROR_NO_T
_hal_sco_port_initCpu(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rv;
    UI32_T              cpu_air_port = 0;
    UI32_T              cpu_mac_port = 0;
    UI32_T              reg_data = 0;
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    AIR_PORT_SPEED_T    speed;

    /* Get CPU port */
    cpu_air_port = HAL_CPU_PORT(unit);
    if (!HAL_IS_PORT_VALID(unit, cpu_air_port))
    {
        return AIR_E_OK;
    }
    HAL_AIR_PORT_TO_MAC_PORT(unit, cpu_air_port, cpu_mac_port);

    /* Set MFC */
    rv = aml_readReg(unit, MFC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    reg_data |= BITS_OFF_L(cpu_mac_port, CSR_CPU_PORT, CSR_CPU_PORT_LENGTH);
    reg_data |= BIT(CSR_CPU_PORT_EN);
    rv = aml_writeReg(unit, MFC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    /* Set Speical Tag, insert mode */
    rv = hal_sco_stag_setPort(unit, cpu_air_port, TRUE);
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    rv = hal_sco_stag_setMode(unit, cpu_air_port, AIR_STAG_MODE_INSERT);
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    /* For EN8853, if external cpu is combo port, set it serdes mode */
    if (HAL_IS_COMBO_PORT_VALID(unit, cpu_air_port))
    {
        rv = hal_sco_port_setComboMode(unit, cpu_air_port, AIR_PORT_COMBO_MODE_SERDES);
        if (AIR_E_OK != rv)
        {
            return rv;
        }
    }

    /* Set port to force mode (disable AutoNego) */
    rv = hal_sco_port_setPhyAutoNego(unit, cpu_air_port, AIR_PORT_PHY_AN_DISABLE);
    if ((AIR_E_OK != rv) && (AIR_E_NOT_SUPPORT != rv))
    {
        return rv;
    }

    /* Set MCR */
    /* Set force full duplex */
    rv = hal_sco_port_setDuplex(unit, cpu_air_port, AIR_PORT_DUPLEX_FULL);
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    /* Set force tx,rx FC on */
    rv = hal_sco_port_setFlowCtrl(unit, cpu_air_port, AIR_PORT_DIR_BOTH, TRUE);
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    /* Set force speed */
    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, cpu_air_port));
    switch (ptr_sdk_port_map_entry->max_speed)
    {
        case AIR_INIT_PORT_SPEED_2500M:
            speed = AIR_PORT_SPEED_2500M;
            break;
        case AIR_INIT_PORT_SPEED_1000M:
            speed = AIR_PORT_SPEED_1000M;
            break;
        default:
            speed = AIR_PORT_SPEED_1000M;
            break;
    }
    rv = hal_sco_port_setSpeed(unit, cpu_air_port, speed);
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    /* Set force mode enable */
    /* Set back pressure enable */
    /* Set EEE 100m,1G,2.5G disable */
    rv = aml_readReg(unit, PMCR(cpu_mac_port), &reg_data, sizeof(reg_data));
    if (AIR_E_OK != rv)
    {
        return rv;
    }
    reg_data |= BIT(MCR_FORCE_MODE_OFFT);
    reg_data |= BIT(MCR_BACK_PRESSURE_OFFT);
    reg_data |= BIT(MCR_FORCE_LINK_OFFT);
    reg_data &= ~BIT(MCR_EEE_100_EN_OFFT);
    reg_data &= ~BIT(MCR_EEE_1G_EN_OFFT);
    reg_data &= ~BIT(MCR_EEE_2P5G_EN_OFFT);
    rv = aml_writeReg(unit, PMCR(cpu_mac_port), &reg_data, sizeof(reg_data));
    if (AIR_E_OK != rv)
    {
        return rv;
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_sco_port_cfgParallelLedPol(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          port;
    AIR_CFG_VALUE_T led_type, led_behavior;
    UI32_T          u32dat, phy_id;
    UI32_T          inverse_bit[] = {0x01, 0x02, 0x04, 0x08, 0x100, 0x400, 0x800, 0x1000};

    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));
    osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));

    led_type.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);
    if (1 == led_type.value) /* The LED is parallel mode */
    {
        AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
        {
            if (HAL_IS_GPHY_PORT_VALID(unit, port))
            {
                aml_readReg(unit, RG_GPIO_INVERSE, &u32dat, sizeof(u32dat));
                led_behavior.value = 0xFFF;
                led_behavior.param0 = port;
                led_behavior.param1 = 0;
                rc |= hal_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);
                HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
                if (!(led_behavior.value & 0x0800)) /* LED polarity check */
                {
                    u32dat &= ~inverse_bit[phy_id & 0x07];
                }
                aml_writeReg(unit, RG_GPIO_INVERSE, &u32dat, sizeof(u32dat));
            }
        }
    }
    return rc;
}
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_sco_port_init
 * PURPOSE:
 *      Initialization functions of port.
 *
 * INPUT:
 *      unit            --  Device ID
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
hal_sco_port_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                port;
    AIR_PORT_PHY_AN_ADV_T adv;
    HAL_SDK_PORT_MAP_T   *ptr_sdk_port_map_entry;
    AIR_CFG_VALUE_T       port_cfg;
    BOOL_T                admin_state;
    UI32_T                reg_mcr = 0;
    UI32_T                mac_port = 0;

    /* Initialize PHY */
    rc = hal_phy_init(unit);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Set unit %d port module initialize fail(%d)\n", unit, rc);
        hal_phy_deinit(unit);
        return rc;
    }

    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
    {
        /* Set all ether port as PHY disabled */
        rc = hal_sco_port_setAdminState(unit, port, FALSE);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        adv.flags = AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX | AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX |
                    AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX | AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX |
                    AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX | AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE |
                    AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE;
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));
        if (AIR_INIT_PORT_SPEED_2500M == ptr_sdk_port_map_entry->max_speed)
        {
            adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_2500M;
        }

        /* Set all ether port as speed 100M on force mode */
        rc = hal_sco_port_setSpeed(unit, port, AIR_PORT_SPEED_100M);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        /* Set all ether port as full duplex on force mode */
        rc = hal_sco_port_setDuplex(unit, port, AIR_PORT_DUPLEX_FULL);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        /* Set PHY advertisment of all ether ports */
        rc = hal_sco_port_setPhyLocalAdvAbility(unit, port, &adv);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        /* Set all ether port as AN mode */
        rc = hal_sco_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_ENABLE);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        /* Set all ether port as PHY enabled */
        osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
        port_cfg.value = 1; /* defaule port enable */
        port_cfg.param0 = port;
        hal_cfg_getValue(unit, AIR_CFG_TYPE_PORT_ADMIN_STATE, &port_cfg);

        admin_state = (port_cfg.value) ? TRUE : FALSE;
        rc = hal_sco_port_setAdminState(unit, port, admin_state);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            break;
        }

        /* Set all port default port matrix pbm */
        rc = hal_sco_port_setPortMatrix(unit, port, HAL_PORT_BMP_TOTAL(unit));
        if (AIR_E_OK != rc)
        {
            break;
        }
        if (HAL_IS_FORCE_MCR_PORT_VALID(unit, port))
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
            aml_readReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
            reg_mcr |= BIT(MCR_FORCE_MODE_OFFT);
            aml_writeReg(unit, PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
        }
    }
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Set unit %d: port %d default value fail(%d)\n", unit, port, rc);
        return rc;
    }
    /* Initialize CPU port */
    rc = _hal_sco_port_initCpu(unit);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: CPU init failed(%d)\n", rc);
    }

    /* If parallel LED, polarity must be adjusted. */
    rc = _hal_sco_port_cfgParallelLedPol(unit);
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        return rc;
    }

    if (HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE_TASK_RSRC)
    {
        /* add callback function for check link status */
        rc = hal_sco_ifmon_register(unit, _hal_sco_port_updateMac, NULL);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "Register MAC process callback function fail: unit %u, rc %d\n", unit, rc);
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_deinit
 * PURPOSE:
 *      Deinitialization of port.
 *
 * INPUT:
 *      unit            --  Device ID
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
hal_sco_port_deinit(
    const UI32_T unit)
{
    UI32_T port, u32dat;

    hal_sco_ifmon_deregister(unit, _hal_sco_port_updateMac, NULL);
    hal_phy_deinit(unit);
    u32dat = HAL_SCO_PORT_MCR_DEFAULT_VALUE;
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        aml_writeReg(unit, PMCR(port), &u32dat, sizeof(u32dat));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_setSerdesMode
 * PURPOSE:
 *      Set tx ability of SGMII ports.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  AIR_PORT_SERDES_MODE_SGMII
 *                          AIR_PORT_SERDES_MODE_1000BASE_X
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Speed will change to 1000M full duplex, when serdes mode set to 1000BASE_X.
 */
AIR_ERROR_NO_T
hal_sco_port_setSerdesMode(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    HAL_PHY_SERDES_MODE_T hal_serdes_mode = HAL_PHY_SERDES_MODE_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }
    switch (mode)
    {
        case AIR_PORT_SERDES_MODE_1000BASE_X:
            hal_serdes_mode = HAL_PHY_SERDES_MODE_1000BASE_X;
            break;
        case AIR_PORT_SERDES_MODE_SGMII:
            hal_serdes_mode = HAL_PHY_SERDES_MODE_SGMII;
            break;
        case AIR_PORT_SERDES_MODE_HSGMII:
            hal_serdes_mode = HAL_PHY_SERDES_MODE_HSGMII;
            break;
        case AIR_PORT_SERDES_MODE_100BASE_FX:
            hal_serdes_mode = HAL_PHY_SERDES_MODE_100BASE_FX;
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_phy_setSerdesMode(unit, port, hal_serdes_mode);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: call hal_phy_setAutoNego() fail\n");
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_getSerdesMode
 * PURPOSE:
 *      Get mode of sgmii ports.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_PORT_SERDES_MODE_SGMII
 *                          AIR_PORT_SERDES_MODE_1000BASE_X
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
hal_sco_port_getSerdesMode(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_SERDES_MODE_T hal_serdes_mode = HAL_PHY_SERDES_MODE_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getSerdesMode(unit, port, &hal_serdes_mode);
    if (AIR_E_OK == rc)
    {
        switch (hal_serdes_mode)
        {
            case HAL_PHY_SERDES_MODE_1000BASE_X:
                *ptr_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
                break;
            case HAL_PHY_SERDES_MODE_SGMII:
                *ptr_mode = AIR_PORT_SERDES_MODE_SGMII;
                break;
            case HAL_PHY_SERDES_MODE_HSGMII:
                *ptr_mode = AIR_PORT_SERDES_MODE_HSGMII;
                break;
            case HAL_PHY_SERDES_MODE_100BASE_FX:
                *ptr_mode = AIR_PORT_SERDES_MODE_100BASE_FX;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_setPhyAutoNego
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port.(Auto or Forced)
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      auto_nego       --  AIR_PORT_PHY_AN_DISABLE
 *                          AIR_PORT_PHY_AN_ENABLE
 *                          AIR_PORT_PHY_AN_RESTART
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
hal_sco_port_setPhyAutoNego(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              u32dat = 0;
    UI32_T              mac_port = 0;
    HAL_PHY_AUTO_NEGO_T hal_auto_nego = HAL_PHY_AUTO_NEGO_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* Set PHY configuration */
    switch (auto_nego)
    {
        case AIR_PORT_PHY_AN_DISABLE:
            hal_auto_nego = HAL_PHY_AUTO_NEGO_DISABLE;
            break;
        case AIR_PORT_PHY_AN_ENABLE:
            hal_auto_nego = HAL_PHY_AUTO_NEGO_ENABLE;
            break;
        case AIR_PORT_PHY_AN_RESTART:
            hal_auto_nego = HAL_PHY_AUTO_NEGO_RESTART;
            break;
        default:
            break;
    }
    rc = hal_phy_setAutoNego(unit, port, hal_auto_nego);
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: call hal_phy_setAutoNego() fail\n");
        return rc;
    }

    if ((AIR_PORT_PHY_AN_RESTART != auto_nego) && (!HAL_IS_FORCE_MCR_PORT_VALID(unit, port)))
    {
        /* Set MAC configuration */
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));

        if (AIR_PORT_PHY_AN_ENABLE == auto_nego)
        {
            /* Disable Force mode of port */
            u32dat &= ~BIT(MCR_FORCE_MODE_OFFT);
            /* Enable EEE ability bits */
            u32dat |= BIT(MCR_EEE_100_EN_OFFT);
            u32dat |= BIT(MCR_EEE_1G_EN_OFFT);
            u32dat |= BIT(MCR_EEE_2P5G_EN_OFFT);
        }
        else if (AIR_PORT_PHY_AN_DISABLE == auto_nego)
        {
            /* Enable Force mode of port */
            u32dat |= BIT(MCR_FORCE_MODE_OFFT);
            /* Disable EEE ability bits */
            u32dat &= ~BIT(MCR_EEE_100_EN_OFFT);
            u32dat &= ~BIT(MCR_EEE_1G_EN_OFFT);
            u32dat &= ~BIT(MCR_EEE_2P5G_EN_OFFT);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unknown parameter.\n");
            return AIR_E_BAD_PARAMETER;
        }

        /* Write data to register */
        aml_writeReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getPhyAutoNego
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_auto_nego   --  AIR_PORT_PHY_AN_DISABLE
 *                          AIR_PORT_PHY_AN_ENABLE
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyAutoNego(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              u32dat = 0;
    UI32_T              mac_port = 0;
    HAL_PHY_AUTO_NEGO_T hal_auto_nego = HAL_PHY_AUTO_NEGO_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getAutoNego(unit, port, &hal_auto_nego);
    if (AIR_E_OK == rc)
    {
        switch (hal_auto_nego)
        {
            case HAL_PHY_AUTO_NEGO_DISABLE:
                *ptr_auto_nego = AIR_PORT_PHY_AN_DISABLE;
                break;
            case HAL_PHY_AUTO_NEGO_ENABLE:
                *ptr_auto_nego = AIR_PORT_PHY_AN_ENABLE;
                break;
            case HAL_PHY_AUTO_NEGO_RESTART:
                *ptr_auto_nego = AIR_PORT_PHY_AN_RESTART;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    else if (AIR_E_NOT_SUPPORT == rc)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
        (*ptr_auto_nego) = (BITS_OFF_R(u32dat, MCR_FORCE_MODE_OFFT, MCR_FORCE_MODE_LENG)) ? AIR_PORT_PHY_AN_DISABLE :
                                                                                            AIR_PORT_PHY_AN_ENABLE;
        rc = AIR_E_OK;
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_setPhyLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      ptr_adv         --  Pointer of the AN advertisement setting
 *
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
hal_sco_port_setPhyLocalAdvAbility(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    HAL_PHY_AN_ADV_T hal_adv;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    osal_memset(&hal_adv, 0, sizeof(HAL_PHY_AN_ADV_T));

    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE, &hal_adv, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE, &hal_adv, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE, &hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE);
    _HAL_SCO_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_2500M, &hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M);

    return hal_phy_setLocalAdvAbility(unit, port, &hal_adv);
}

/* FUNCTION NAME: hal_sco_port_getPhyLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
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
hal_sco_port_getPhyLocalAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_PHY_AN_ADV_T hal_adv;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getLocalAdvAbility(unit, port, &hal_adv);
    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d local adv. flags = %x\n", port, hal_adv.flags);
        ptr_adv->flags = 0;
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv,
                                AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
        _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d AIR flags = %x\n", port, ptr_adv->flags);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_getPhyRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation remote advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
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
hal_sco_port_getPhyRemoteAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_LINK_STATUS_T hal_status;
    HAL_PHY_AN_ADV_T      hal_adv;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* Check AN completed */
    rc = hal_phy_getLinkStatus(unit, port, &hal_status);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d get link status fail!(%d)\n", port, rc);
        return rc;
    }
    if (!(hal_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP))
    {
        ptr_adv->flags = 0;
        return AIR_E_OK;
    }

    /* Get remote AN advertisement ability */
    rc = hal_phy_getRemoteAdvAbility(unit, port, &hal_adv);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d get remote advertisement fail!(%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d remote adv. flags = %x\n", port, hal_adv.flags);
    ptr_adv->flags = 0;
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
    _HAL_SCO_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d AIR flags = %x\n", port, ptr_adv->flags);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps
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
hal_sco_port_setSpeed(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32mcr = 0;
    UI32_T          mac_port = 0;
    HAL_PHY_SPEED_T hal_speed = HAL_PHY_SPEED_LAST;

    HAL_CHECK_ENUM_RANGE(speed, AIR_PORT_SPEED_LAST);

    /* If port is not CPU port, set PHY register */
    if ((HAL_CPU_PORT(unit) != port) || (HAL_IS_XSGMII_PORT_VALID(unit, port) && (HAL_CPU_PORT(unit) == port)))
    {
        /* Set PHY speed */
        switch (speed)
        {
            case AIR_PORT_SPEED_10M:
                hal_speed = HAL_PHY_SPEED_10M;
                break;
            case AIR_PORT_SPEED_100M:
                hal_speed = HAL_PHY_SPEED_100M;
                break;
            case AIR_PORT_SPEED_1000M:
                hal_speed = HAL_PHY_SPEED_1000M;
                break;
            case AIR_PORT_SPEED_2500M:
                hal_speed = HAL_PHY_SPEED_2500M;
                break;
            default:
                break;
        }
        rc = hal_phy_setSpeed(unit, port, hal_speed);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            return rc;
        }
    }

    /* Set MAC speed */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));

    u32mcr &= ~BITS_RANGE(MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
    u32mcr |= BITS_OFF_L(speed, MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);

    /* Write data to register */
    aml_writeReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_speed       --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps (Port5, Port6)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getSpeed(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    HAL_PHY_SPEED_T hal_speed = HAL_PHY_SPEED_LAST;
    UI32_T          mac_port = 0, u32mcr = 0;

    /* If port is CPU port, get MAC register directly */
    if (HAL_CPU_PORT(unit) == port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        (*ptr_speed) = BITS_OFF_R(u32mcr, MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
    }
    else
    {
        /* Get PHY speed */
        rc = hal_phy_getSpeed(unit, port, &hal_speed);
        if (AIR_E_OK == rc)
        {
            switch (hal_speed)
            {
                case HAL_PHY_SPEED_10M:
                    *ptr_speed = AIR_PORT_SPEED_10M;
                    break;
                case HAL_PHY_SPEED_100M:
                    *ptr_speed = AIR_PORT_SPEED_100M;
                    break;
                case HAL_PHY_SPEED_1000M:
                    *ptr_speed = AIR_PORT_SPEED_1000M;
                    break;
                case HAL_PHY_SPEED_2500M:
                    *ptr_speed = AIR_PORT_SPEED_2500M;
                    break;
                default:
                    rc = AIR_E_OTHERS;
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

            /* Read data from register */
            aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
            (*ptr_speed) = BITS_OFF_R(u32mcr, MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
            rc = AIR_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      duplex          --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
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
hal_sco_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           u32mcr = 0;
    UI32_T           mac_port = 0;
    HAL_PHY_DUPLEX_T hal_duplex = HAL_PHY_DUPLEX_LAST;

    HAL_CHECK_ENUM_RANGE(duplex, AIR_PORT_DUPLEX_LAST);

    /* If port is not CPU port, set PHY register */
    if (HAL_CPU_PORT(unit) != port)
    {
        /* Set PHY duplex */
        switch (duplex)
        {
            case AIR_PORT_DUPLEX_HALF:
                hal_duplex = HAL_PHY_DUPLEX_HALF;
                break;
            case AIR_PORT_DUPLEX_FULL:
                hal_duplex = HAL_PHY_DUPLEX_FULL;
                break;
            default:
                break;
        }
        rc = hal_phy_setDuplex(unit, port, hal_duplex);
        if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
        {
            return rc;
        }
    }

    /* Set MAC duplex */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
    if (AIR_PORT_DUPLEX_FULL == duplex)
    {
        u32mcr |= BIT(MCR_FORCE_DUPLEX_OFFT);
    }
    else
    {
        u32mcr &= ~BIT(MCR_FORCE_DUPLEX_OFFT);
    }
    /* Write data to register */
    aml_writeReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_duplex      --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getDuplex(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_PHY_DUPLEX_T hal_duplex = HAL_PHY_DUPLEX_LAST;
    UI32_T           mac_port = 0, u32mcr = 0;

    /* If port is CPU port, get MAC register directly*/
    if (HAL_CPU_PORT(unit) == port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        (*ptr_duplex) = BITS_OFF_R(u32mcr, MCR_FORCE_DUPLEX_OFFT, MCR_FORCE_DUPLEX_LENG);
    }
    else
    {
        /* Get PHY duplex */
        rc = hal_phy_getDuplex(unit, port, &hal_duplex);
        if (AIR_E_OK == rc)
        {
            switch (hal_duplex)
            {
                case HAL_PHY_DUPLEX_HALF:
                    *ptr_duplex = AIR_PORT_DUPLEX_HALF;
                    break;
                case HAL_PHY_DUPLEX_FULL:
                    *ptr_duplex = AIR_PORT_DUPLEX_FULL;
                    break;
                default:
                    rc = AIR_E_OTHERS;
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

            /* Read data from register */
            aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
            (*ptr_duplex) = BITS_OFF_R(u32mcr, MCR_FORCE_DUPLEX_OFFT, MCR_FORCE_DUPLEX_LENG);
            rc = AIR_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_setBackPressure
 * PURPOSE:
 *      Set the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      bckPres         --  FALSE:Disable
 *                          TRUE: Enable
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
hal_sco_port_setBackPressure(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    if (TRUE == bckPres)
    {
        u32dat |= BIT(MCR_BACK_PRESSURE_OFFT);
    }
    else
    {
        u32dat &= ~BIT(MCR_BACK_PRESSURE_OFFT);
    }

    /* Write data to register */
    aml_writeReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getBackPressure
 * PURPOSE:
 *      Get the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_bckPres     --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getBackPressure(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_bckPres)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    (*ptr_bckPres) = BITS_OFF_R(u32dat, MCR_BACK_PRESSURE_OFFT, MCR_BACK_PRESSURE_LENG);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_PORT_DIR_TX
 *                          AIR_PORT_DIR_RX
 *                          AIR_PORT_DIR_BOTH
 *      fc_en           --  TRUE: Enable select port flow control
 *                          FALSE:Disable select port flow control
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
hal_sco_port_setFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en)
{
    UI32_T u32mcr = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));
    if (TRUE == fc_en)
    {
        /* Enable port flow control */
        if (AIR_PORT_DIR_TX == dir)
        {
            u32mcr |= BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
        }
        else if (AIR_PORT_DIR_RX == dir)
        {
            u32mcr |= BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
        }
        else if (AIR_PORT_DIR_BOTH == dir)
        {
            u32mcr |= BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
            u32mcr |= BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else
    {
        /* Disable port flow control */
        if (AIR_PORT_DIR_TX == dir)
        {
            u32mcr &= ~BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
        }
        else if (AIR_PORT_DIR_RX == dir)
        {
            u32mcr &= ~BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
        }
        else if (AIR_PORT_DIR_BOTH == dir)
        {
            u32mcr &= ~BIT(MCR_FORCE_FLOWCTRL_TX_OFFT);
            u32mcr &= ~BIT(MCR_FORCE_FLOWCTRL_RX_OFFT);
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    aml_writeReg(unit, PMCR(mac_port), &u32mcr, sizeof(u32mcr));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_PORT_DIR_TX
 *                          AIR_PORT_DIR_RX
 * OUTPUT:
 *      ptr_fc_en       --  FALSE: Port flow control disable
 *                          TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read port flow control status*/
    aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    if (AIR_PORT_DIR_TX == dir)
    {
        (*ptr_fc_en) = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_TX_OFFT, MCR_FORCE_FLOWCTRL_TX_LENG);
    }
    else if (AIR_PORT_DIR_RX == dir)
    {
        (*ptr_fc_en) = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_RX_OFFT, MCR_FORCE_FLOWCTRL_RX_LENG);
    }
    else
    {
        return AIR_E_NOT_SUPPORT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_setPhyLoopBack
 * PURPOSE:
 *      Set the loop back configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      dir             --  AIR_PORT_LPBK_DIR_NEAR_END
 *                          AIR_PORT_LPBK_DIR_FAR_END
 *      lpBack          --  FALSE:Disable
 *                          TRUE: Enable
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
hal_sco_port_setPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              lpBack_en)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_PHY_LPBK_T hal_phy_lpbk_dir = HAL_PHY_LPBK_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    switch (dir)
    {
        case AIR_PORT_LPBK_DIR_NEAR_END:
            hal_phy_lpbk_dir = HAL_PHY_LPBK_NEAR_END;
            break;
        case AIR_PORT_LPBK_DIR_FAR_END:
            hal_phy_lpbk_dir = HAL_PHY_LPBK_FAR_END;
            break;
        default:
            return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_setLoopBack(unit, port, hal_phy_lpbk_dir, lpBack_en);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d set loop back [%s]:%s failed(%d)\n", port,
                   (AIR_PORT_LPBK_DIR_NEAR_END == dir) ? "Near End" : "Far End",
                   (TRUE == lpBack_en) ? "enable" : "disable", rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_getPhyLoopBack
 * PURPOSE:
 *      Get the loop back configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      dir             --  AIR_PORT_LPBK_DIR_NEAR_END
 *                          AIR_PORT_LPBK_DIR_FAR_END
 *
 * OUTPUT:
 *      ptr_lpBack      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_lpBack_en)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_PHY_LPBK_T hal_phy_lpbk_dir = HAL_PHY_LPBK_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    switch (dir)
    {
        case AIR_PORT_LPBK_DIR_NEAR_END:
            hal_phy_lpbk_dir = HAL_PHY_LPBK_NEAR_END;
            break;
        case AIR_PORT_LPBK_DIR_FAR_END:
            hal_phy_lpbk_dir = HAL_PHY_LPBK_FAR_END;
            break;
        default:
            return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getLoopBack(unit, port, hal_phy_lpbk_dir, ptr_lpBack_en);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d get loop back [%s] failed(%d)\n", port,
                   (AIR_PORT_LPBK_DIR_NEAR_END == dir) ? "Near End" : "Far End", rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_getPortStatus
 * PURPOSE:
 *      Get the physical link status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_ps          --  AIR_PORT_STATUS_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_LINK_STATUS_T hal_status;
    UI32_T                u32dat, u32datField = 0;
    UI32_T                mac_port = 0, pmsr_speed, pmsr_duplex;
    HAL_PHY_AN_ADV_T      an_adv, an_lp;
    HAL_PHY_AUTO_NEGO_T   an = HAL_PHY_AUTO_NEGO_LAST;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    osal_memset(&hal_status, 0, sizeof(HAL_PHY_LINK_STATUS_T));

    /* If port is CPU port, get MAC register directly */
    if (HAL_CPU_PORT(unit) == port)
    {
        /* Read data from MAC register */
        aml_readReg(unit, PMSR(mac_port), &u32dat, sizeof(u32dat));

        _HAL_SCO_PORT_SET_REG_BIT(BITS_OFF_R(u32dat, REG_PORT_LINK_OFFT, REG_PORT_LINK_LENG), ptr_ps->flags,
                                  AIR_PORT_STATUS_FLAGS_LINK_UP);
        ptr_ps->speed = BITS_OFF_R(u32dat, REG_PORT_SPEED_OFFT, REG_PORT_SPEED_LENG);
        ptr_ps->duplex = BITS_OFF_R(u32dat, REG_PORT_DUPLEX_OFFT, REG_PORT_DUPLEX_LENG);
    }
    else
    {
        rc = hal_phy_getLinkStatus(unit, port, &hal_status);
        if (AIR_E_OK == rc)
        {
            if (hal_status.flags & HAL_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT)
            {
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: call hal_phy_getLinkStatus(): remote fault\n");
                return AIR_E_OP_INCOMPLETE;
            }
            osal_memset(ptr_ps, 0, sizeof(AIR_PORT_STATUS_T));
            _HAL_SCO_PORT_SET_FLAGS(&hal_status, HAL_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_ps,
                                    AIR_PORT_STATUS_FLAGS_LINK_UP);
            switch (hal_status.speed)
            {
                case HAL_PHY_SPEED_10M:
                    ptr_ps->speed = AIR_PORT_SPEED_10M;
                    break;
                case HAL_PHY_SPEED_100M:
                    ptr_ps->speed = AIR_PORT_SPEED_100M;
                    break;
                case HAL_PHY_SPEED_1000M:
                    ptr_ps->speed = AIR_PORT_SPEED_1000M;
                    break;
                case HAL_PHY_SPEED_2500M:
                    ptr_ps->speed = AIR_PORT_SPEED_2500M;
                    break;
                default:
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unknown type: hal_phy_getLinkStatus->speed(0x%x)\n",
                               hal_status.speed);
                    break;
            }
            switch (hal_status.duplex)
            {
                case HAL_PHY_DUPLEX_HALF:
                    ptr_ps->duplex = AIR_PORT_DUPLEX_HALF;
                    break;
                case HAL_PHY_DUPLEX_FULL:
                    ptr_ps->duplex = AIR_PORT_DUPLEX_FULL;
                    break;
                default:
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unknown type: hal_phy_getLinkStatus->duplex(0x%x)\n",
                               hal_status.duplex);
                    break;
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            /*for phy driver port_cfg.value = 2 return AIR_NOT_SUPPORT,
            the speed and dupelx information decide by port api */
            osal_memset(ptr_ps, 0, sizeof(AIR_PORT_STATUS_T));
            _HAL_SCO_PORT_SET_FLAGS(&hal_status, HAL_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_ps,
                                    AIR_PORT_STATUS_FLAGS_LINK_UP);
            aml_readReg(unit, PMSR(mac_port), &u32dat, sizeof(u32dat));
            pmsr_speed = BITS_OFF_R(u32dat, REG_PORT_SPEED_OFFT, REG_PORT_SPEED_LENG);
            pmsr_duplex = BITS_OFF_R(u32dat, REG_PORT_DUPLEX_OFFT, REG_PORT_DUPLEX_LENG);

            switch (pmsr_speed)
            {
                case HAL_PHY_SPEED_10M:
                    ptr_ps->speed = AIR_PORT_SPEED_10M;
                    break;
                case HAL_PHY_SPEED_100M:
                    ptr_ps->speed = AIR_PORT_SPEED_100M;
                    break;
                case HAL_PHY_SPEED_1000M:
                    ptr_ps->speed = AIR_PORT_SPEED_1000M;
                    break;
                case HAL_PHY_SPEED_2500M:
                    ptr_ps->speed = AIR_PORT_SPEED_2500M;
                    break;
                default:
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unknown type: pmcr speed (0x%x)\n", pmsr_speed);
                    break;
            }
            switch (pmsr_duplex)
            {
                case HAL_PHY_DUPLEX_HALF:
                    ptr_ps->duplex = AIR_PORT_DUPLEX_HALF;
                    break;
                case HAL_PHY_DUPLEX_FULL:
                    ptr_ps->duplex = AIR_PORT_DUPLEX_FULL;
                    break;
                default:
                    DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unknown type: pmcr duplex (0x%x)\n", pmsr_duplex);
                    break;
            }
            rc = AIR_E_OK;
        }
    }

    /* only update fc and eee information in link up mode */
    if (hal_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)
    {
        rc = hal_phy_getAutoNego(unit, port, &an);
        if ((AIR_E_OK == rc) && (HAL_PHY_AUTO_NEGO_ENABLE == an))
        {
            rc = hal_phy_getLocalAdvAbility(unit, port, &an_adv);
            if (AIR_E_OK == rc)
            {
                rc = hal_phy_getRemoteAdvAbility(unit, port, &an_lp);
                if (AIR_E_OK == rc)
                {
                    if (!((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_EEE_INVALID))
                    {
                        if ((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_EEE)
                        {
                            ptr_ps->flags |= (AIR_PORT_STATUS_FLAGS_EEE);
                        }
                    }
                    if (!((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_PAUSE_INVALID))
                    {
                        /* Update Flow Ctrl */
                        if ((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE)
                        {
                            ptr_ps->flags |= (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX);
                        }
                        else if ((an_adv.flags & HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE) &&
                                 ((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE))
                        {
                            ptr_ps->flags |= (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX);
                        }
                        else if ((an_lp.flags & HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE) &&
                                 ((an_adv.flags & an_lp.flags) & HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE))
                        {
                            ptr_ps->flags |= (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX);
                        }
                        else
                        {
                            /* do nothing */
                        }
                    }
                    else
                    {
                        /* read mcr */
                        aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
                        u32datField = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_TX_OFFT, MCR_FORCE_FLOWCTRL_TX_LENG);
                        if (u32datField)
                        {
                            ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                        }
                        u32datField = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_RX_OFFT, MCR_FORCE_FLOWCTRL_RX_LENG);
                        if (u32datField)
                        {
                            ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX;
                        }
                    }
                }
            }
        }
        else
        {
            /* read mcr */
            rc = aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                if (AIR_PORT_DUPLEX_FULL == ptr_ps->duplex)
                {
                    u32datField = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_TX_OFFT, MCR_FORCE_FLOWCTRL_TX_LENG);
                    if (u32datField)
                    {
                        ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                    }
                    u32datField = BITS_OFF_R(u32dat, MCR_FORCE_FLOWCTRL_RX_OFFT, MCR_FORCE_FLOWCTRL_RX_LENG);
                    if (u32datField)
                    {
                        ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX;
                    }
                }
                else
                {
                    u32datField = BITS_OFF_R(u32dat, MCR_BACK_PRESSURE_OFFT, MCR_BACK_PRESSURE_LENG);
                    if (u32datField)
                    {
                        ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                    }
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_setAdminState
 * PURPOSE:
 *      Set the port status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  TRUE: port enable
 *                          FALSE: port disable
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
hal_sco_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    UI32_T                u32dat = 0;
    UI32_T                mac_port = 0;
    HAL_PHY_ADMIN_STATE_T phyState = HAL_PHY_ADMIN_STATE_LAST;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d, state = %d\n", port, state);

#ifdef IS_FPGA_STAGE
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    if (TRUE == state)
    {
        u32dat |= BIT(MCR_FORCE_LINK_OFFT);
    }
    else
    {
        u32dat &= ~BIT(MCR_FORCE_LINK_OFFT);
    }
    aml_writeReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
#else
    /* If port is not CPU port, set PHY register */
    if (HAL_CPU_PORT(unit) != port)
    {
        phyState = (state) ? HAL_PHY_ADMIN_STATE_ENABLE : HAL_PHY_ADMIN_STATE_DISABLE;

        if (AIR_E_OK != hal_phy_setAdminState(unit, port, phyState))
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: hal_phy_setAdminState set fail \n");
            return AIR_E_OTHERS;
        }
    }
    else
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
        aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
        if (TRUE == state)
        {
            u32dat |= BIT(MCR_FORCE_LINK_OFFT);
        }
        else
        {
            u32dat &= ~BIT(MCR_FORCE_LINK_OFFT);
        }
        aml_writeReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));
    }
#endif

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_getAdminState
 * PURPOSE:
 *      Get the port status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  TRUE: port enable
 *                          FALSE: port disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_state)
{
    HAL_PHY_ADMIN_STATE_T phyState = HAL_PHY_ADMIN_STATE_LAST;
    UI32_T                mac_port = 0, u32dat = 0;
#ifdef IS_FPGA_STAGE
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));

    if (u32dat & BIT(MCR_FORCE_LINK_OFFT))
    {
        *ptr_state = TRUE;
    }
    else
    {
        *ptr_state = FALSE;
    }
#else

    /* If port is CPU port, get MAC register directly */
    if (HAL_CPU_PORT(unit) == port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        aml_readReg(unit, PMCR(mac_port), &u32dat, sizeof(u32dat));

        if (u32dat & BIT(MCR_FORCE_LINK_OFFT))
        {
            (*ptr_state) = TRUE;
        }
        else
        {
            (*ptr_state) = FALSE;
        }
    }
    else
    {
        if (AIR_E_OK != hal_phy_getAdminState(unit, port, &phyState))
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: hal_phy_getAdminState set fail \n");
            return AIR_E_OTHERS;
        }

        if (HAL_PHY_ADMIN_STATE_ENABLE == phyState)
        {
            *ptr_state = TRUE;
        }
        else
        {
            *ptr_state = FALSE;
        }
    }

#endif

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_port_setPhySmartSpeedDown
 * PURPOSE:
 *      Set Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  AIR_PORT_SSD_MODE_DISABLE
 *                          AIR_PORT_SSD_MODE_2T
 *                          AIR_PORT_SSD_MODE_3T
 *                          AIR_PORT_SSD_MODE_4T
 *                          AIR_PORT_SSD_MODE_5T
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
hal_sco_port_setPhySmartSpeedDown(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T mode)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    HAL_PHY_SSD_MODE_T hal_mode = 0;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* Set PHY speed */
    switch (mode)
    {
        case AIR_PORT_SSD_MODE_2T:
            hal_mode = HAL_PHY_SSD_MODE_2T;
            break;
        case AIR_PORT_SSD_MODE_3T:
            hal_mode = HAL_PHY_SSD_MODE_3T;
            break;
        case AIR_PORT_SSD_MODE_4T:
            hal_mode = HAL_PHY_SSD_MODE_4T;
            break;
        case AIR_PORT_SSD_MODE_5T:
            hal_mode = HAL_PHY_SSD_MODE_5T;
            break;
        case AIR_PORT_SSD_MODE_DISABLE:
        default:
            hal_mode = HAL_PHY_SSD_MODE_DISABLE;
            break;
    }

    rc = hal_phy_setSmartSpeedDown(unit, port, hal_mode);

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: hal_phy_setSmartSpeedDown set fail \n");
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_getPhySmartSpeedDown
 * PURPOSE:
 *      Get Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_PORT_SSD_MODE_DISABLE
 *                          AIR_PORT_SSD_MODE_2T
 *                          AIR_PORT_SSD_MODE_3T
 *                          AIR_PORT_SSD_MODE_4T
 *                          AIR_PORT_SSD_MODE_5T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhySmartSpeedDown(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    HAL_PHY_SSD_MODE_T hal_mode = HAL_PHY_SSD_MODE_DISABLE;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getSmartSpeedDown(unit, port, &hal_mode);

    if (AIR_E_OK == rc)
    {
        switch (hal_mode)
        {
            case HAL_PHY_SSD_MODE_2T:
                *ptr_mode = AIR_PORT_SSD_MODE_2T;
                break;
            case HAL_PHY_SSD_MODE_3T:
                *ptr_mode = AIR_PORT_SSD_MODE_3T;
                break;
            case HAL_PHY_SSD_MODE_4T:
                *ptr_mode = AIR_PORT_SSD_MODE_4T;
                break;
            case HAL_PHY_SSD_MODE_5T:
                *ptr_mode = AIR_PORT_SSD_MODE_5T;
                break;
            case HAL_PHY_SSD_MODE_DISABLE:
            default:
                *ptr_mode = AIR_PORT_SSD_MODE_DISABLE;
                break;
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: hal_phy_getSmartSpeedDown set fail \n");
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      port_bitmap     --  Matrix port bitmap
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_setPortMatrix(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            mac_port = 0;
    AIR_PORT_BITMAP_T mac_port_bitmap;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, port_bitmap, mac_port_bitmap);
    aml_writeReg(unit, PORTMATRIX(mac_port), &mac_port_bitmap[0], sizeof(UI32_T));
    return rc;
}

/* FUNCTION NAME: hal_sco_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      port_bitmap     --  Matrix port bitmap
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_getPortMatrix(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            mac_port = 0;
    AIR_PORT_BITMAP_T mac_port_bitmap;

    AIR_PORT_BITMAP_CLEAR(mac_port_bitmap);
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PORTMATRIX(mac_port), &mac_port_bitmap[0], sizeof(UI32_T));
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, mac_port_bitmap, port_bitmap);
    return rc;
}

/* FUNCTION NAME: hal_sco_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      mode            --  Port vlan mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_setVlanMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PCR(mac_port), &val, sizeof(UI32_T));
    val &= ~PCR_PORT_VLAN_MASK;
    val |= (mode & PCR_PORT_VLAN_RELMASK) << PCR_PORT_VLAN_OFFT;
    aml_writeReg(unit, PCR(mac_port), &val, sizeof(UI32_T));
    return rc;
}

/* FUNCTION NAME: hal_sco_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      ptr_mode        --  Port vlan mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_getVlanMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PCR(mac_port), &val, sizeof(UI32_T));
    *ptr_mode = (val >> PCR_PORT_VLAN_OFFT) & PCR_PORT_VLAN_RELMASK;
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_setPhyLedOnCtrl
 * PURPOSE:
 *      Set LED state for a specific port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_setLedOnCtrl(unit, port, led_id, enable);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d set LED-ID[%d]:%s failed(%d)\n", port, led_id,
                   (TRUE == enable) ? "enable" : "disable", rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_getPhyLedOnCtrl
 * PURPOSE:
 *      Get LED state for a specific port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED ID
 *
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getLedOnCtrl(unit, port, led_id, ptr_enable);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d get LED-ID[%d] failed(%d)\n", port, led_id, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      combo_mode      --  AIR_PORT_COMBO_MODE_PHY
 *                          AIR_PORT_COMBO_MODE_SERDES
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
hal_sco_port_setComboMode(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T combo_mode)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    HAL_PHY_COMBO_MODE_T hal_combo_mode = HAL_PHY_COMBO_MODE_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO != HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* Set PHY configuration */
    switch (combo_mode)
    {
        case AIR_PORT_COMBO_MODE_PHY:
            hal_combo_mode = HAL_PHY_COMBO_MODE_PHY;
            break;
        case AIR_PORT_COMBO_MODE_SERDES:
            hal_combo_mode = HAL_PHY_COMBO_MODE_SERDES;
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_phy_setComboMode(unit, port, hal_combo_mode);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: call hal_phy_setAutoNego() fail\n");
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port.
 *
 * INPUT:
 *      unit             --  Device ID
 *      port             --  Index of port number
 *
 * OUTPUT:
 *      ptr_combo_mode   --  AIR_PORT_COMBO_MODE_PHY
 *                           AIR_PORT_COMBO_MODE_SERDES
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getComboMode(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    HAL_PHY_COMBO_MODE_T hal_combo_mode = HAL_PHY_COMBO_MODE_LAST;

    if ((HAL_CPU_PORT(unit) == port) && !(HAL_IS_XSGMII_PORT_VALID(unit, port)))
    {
        return AIR_E_NOT_SUPPORT;
    }

    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO != HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getComboMode(unit, port, &hal_combo_mode);
    if (AIR_E_OK == rc)
    {
        switch (hal_combo_mode)
        {
            case HAL_PHY_COMBO_MODE_PHY:
                *ptr_combo_mode = AIR_PORT_COMBO_MODE_PHY;
                break;
            case HAL_PHY_COMBO_MODE_SERDES:
                *ptr_combo_mode = AIR_PORT_COMBO_MODE_SERDES;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      led_id          --  LED ID
 *      ctrl_mode       --  LED control mode enumeration type
 *                          AIR_PORT_PHY_LED_CTRL_MODE_T
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
hal_sco_port_setPhyLedCtrlMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_setLedCtrlMode(unit, port, led_id, ctrl_mode);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d set LED-ID[%d]:%s failed(%d)\n", port, led_id,
                   (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == ctrl_mode) ? "force" : "phy", rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode           -- LED control enumeration type
 *                                 AIR_PORT_PHY_LED_CTRL_MODE_T
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
hal_sco_port_getPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    HAL_PHY_LED_CTRL_MODE_T hal_ctrl_mode = HAL_PHY_LED_CTRL_MODE_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getLedCtrlMode(unit, port, led_id, &hal_ctrl_mode);
    if (AIR_E_OK == rc)
    {
        switch (hal_ctrl_mode)
        {
            case HAL_PHY_LED_CTRL_MODE_PHY:
                *ptr_ctrl_mode = AIR_PORT_PHY_LED_CTRL_MODE_PHY;
                break;
            case HAL_PHY_LED_CTRL_MODE_FORCE:
                *ptr_ctrl_mode = AIR_PORT_PHY_LED_CTRL_MODE_FORCE;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      state                   -- LED force state
 *                                 AIR_PORT_PHY_LED_STATE_T
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
hal_sco_port_setPhyLedForceState(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   led_id,
    const AIR_PORT_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }
    rc = hal_phy_setPhyLedForceState(unit, port, led_id, state);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d set LED-ID[%d] state:%d failed(%d)\n", port, led_id, state, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_getPhyLedForceState
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_state               -- LED control enumeration type
 *                                 AIR_PORT_PHY_LED_STATE_T
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
hal_sco_port_getPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    AIR_PORT_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    HAL_PHY_LED_STATE_T hal_state = HAL_PHY_LED_STATE_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getPhyLedForceState(unit, port, led_id, &hal_state);
    if (AIR_E_OK == rc)
    {
        switch (hal_state)
        {
            case HAL_PHY_LED_CTRL_MODE_PHY:
                *ptr_state = AIR_PORT_PHY_LED_STATE_FORCE_PATT;
                break;
            case HAL_PHY_LED_STATE_OFF:
                *ptr_state = AIR_PORT_PHY_LED_STATE_OFF;
                break;
            case HAL_PHY_LED_STATE_ON:
                *ptr_state = AIR_PORT_PHY_LED_STATE_ON;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      pattern                 -- LED force pattern
 *                                 AIR_PORT_PHY_LED_PATT_T
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
hal_sco_port_setPhyLedForcePattCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const AIR_PORT_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_setPhyLedForcePattCfg(unit, port, led_id, pattern);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d set LED-ID[%d] patt:%d failed(%d)\n", port, led_id, pattern, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_pattern             -- LED force pattern enumeration type
 *                                 AIR_PORT_PHY_LED_PATT_T
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
hal_sco_port_getPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    AIR_PORT_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    HAL_PHY_LED_PATT_T hal_pattern = HAL_PHY_LED_PATT_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getPhyLedForcePattCfg(unit, port, led_id, &hal_pattern);
    if (AIR_E_OK == rc)
    {
        switch (hal_pattern)
        {
            case HAL_PHY_LED_PATT_HZ_HALF:
                *ptr_pattern = AIR_PORT_PHY_LED_PATT_HZ_HALF;
                break;
            case HAL_PHY_LED_PATT_HZ_ONE:
                *ptr_pattern = AIR_PORT_PHY_LED_PATT_HZ_ONE;
                break;
            case HAL_PHY_LED_PATT_HZ_TWO:
                *ptr_pattern = AIR_PORT_PHY_LED_PATT_HZ_TWO;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_triggerCableTest
 * PURPOSE:
 *      Get cable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *      test_pair       --  Select test pair
 *                          AIR_PORT_CABLE_TEST_PAIR_A
 *                          AIR_PORT_CABLE_TEST_PAIR_B
 *                          AIR_PORT_CABLE_TEST_PAIR_C
 *                          AIR_PORT_CABLE_TEST_PAIR_D
 *                          AIR_PORT_CABLE_TEST_PAIR_ALL
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          AIR_PORT_CABLE_TEST_RSLT_T
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
hal_sco_port_triggerCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    i = 0;
    HAL_PHY_CABLE_TEST_RSLT_T phy_test_rslt;
    HAL_PHY_CABLE_TEST_PAIR_T phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_LAST;
    BOOL_T                    admin_state = FALSE;

    osal_memset(&phy_test_rslt, 0, sizeof(phy_test_rslt));
    rc = hal_sco_port_getAdminState(unit, port, &admin_state);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Port %d call hal_sco_port_getAdminState() fail\n", port);
    }

    if ((HAL_CPU_PORT(unit) == port) || (FALSE == admin_state))
    {
        return AIR_E_NOT_SUPPORT;
    }
    switch (test_pair)
    {
        case AIR_PORT_CABLE_TEST_PAIR_A:
            phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_A;
            break;
        case AIR_PORT_CABLE_TEST_PAIR_B:
            phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_B;
            break;
        case AIR_PORT_CABLE_TEST_PAIR_C:
            phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_C;
            break;
        case AIR_PORT_CABLE_TEST_PAIR_D:
            phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_D;
            break;
        case AIR_PORT_CABLE_TEST_PAIR_ALL:
            phy_test_pair = HAL_PHY_CABLE_TEST_PAIR_ALL;
            break;
        case AIR_PORT_CABLE_TEST_PAIR_LAST:
            /* do nothing */
            break;
    }

    rc = hal_phy_triggerCableTest(unit, port, phy_test_pair, &phy_test_rslt);
    for (i = 0; i < AIR_PORT_CABLE_MAX_PAIR; i++)
    {
        ptr_test_rslt->status[i] = phy_test_rslt.status[i];
        ptr_test_rslt->length[i] = phy_test_rslt.length[i];
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_port_getCableTestRawData
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
 p

 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_sco_port_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }
    rc = hal_phy_getCableTestRawData(unit, port, pptr_raw_data_all);
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_setPhyOpMode
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
hal_sco_port_setPhyOpMode(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_setPhyOpMode(unit, port, mode);
    return rc;
}

/* FUNCTION NAME:   hal_sco_port_getPhyOpMode
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
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyOpMode(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    HAL_PHY_OP_MODE_T hal_mode = HAL_PHY_OP_MODE_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_getPhyOpMode(unit, port, &hal_mode);
    if (AIR_E_OK == rc)
    {
        switch (hal_mode)
        {
            case HAL_PHY_OP_MODE_NORMAL:
                *ptr_mode = AIR_PORT_OP_MODE_NORMAL;
                break;
            case HAL_PHY_OP_MODE_LONG_REACH:
                *ptr_mode = AIR_PORT_OP_MODE_LONG_REACH;
                break;
            default:
                rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_port_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          AIR_PORT_CABLE_TEST_RSLT_T
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
hal_sco_port_triggerLinkDownCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    HAL_PHY_CABLE_TEST_RSLT_T phy_test_rslt;

    if (HAL_CPU_PORT(unit) == port)
    {
        return AIR_E_NOT_SUPPORT;
    }

    rc = hal_phy_triggerLinkDownCableTest(unit, port, &phy_test_rslt);
    if (AIR_E_OK == rc)
    {
        ptr_test_rslt->status[0] = phy_test_rslt.status[0];
        ptr_test_rslt->length[0] = phy_test_rslt.length[0];
        ptr_test_rslt->status[1] = phy_test_rslt.status[1];
        ptr_test_rslt->length[1] = phy_test_rslt.length[1];
    }

    return rc;
}
