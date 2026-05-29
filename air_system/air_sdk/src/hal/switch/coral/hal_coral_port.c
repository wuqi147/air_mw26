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

/* FILE NAME:  hal_coral_port.c
 * PURPOSE:
 *  Implement Port module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_port.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
#include <hal/switch/coral/hal_coral_ifmon.h>
#include <hal/switch/coral/hal_coral_perif.h>
#include <hal/switch/coral/hal_coral_reg.h>
#include <hal/switch/coral/hal_coral_stag.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_PORT_LED_TYPE_PARALLEL (1)
#define HAL_CORAL_PORT_LED_TYPE_SERIAL   (0)
#define HAL_CORAL_PORT_SLED_CFG_GPIO_NUM (2)

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_CORAL_PORT_SET_REG_BIT(cond, reg, bit) \
    do                                              \
    {                                               \
        if (TRUE == (cond))                         \
        {                                           \
            (reg) |= (bit);                         \
        }                                           \
        else                                        \
        {                                           \
            (reg) &= ~(bit);                        \
        }                                           \
    } while (0)

#define _HAL_CORAL_PORT_SET_FLAGS(_src_struct_, _src_flag_, _dst_struct_, _dst_flag_) \
    do                                                                                \
    {                                                                                 \
        if ((_src_struct_)->flags & (_src_flag_))                                     \
        {                                                                             \
            (_dst_struct_)->flags |= (_dst_flag_);                                    \
        }                                                                             \
    } while (0)
/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PORT, "hal_coral_port.c");

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: _hal_coral_port_updateMac
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
_hal_coral_port_updateMac(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    UI32_T              reg_mcr = 0;
    UI32_T              reg_msr = 0;
    UI32_T              mac_port = 0;
    UI32_T              port_speed = 0;
    HAL_PHY_OP_MODE_T   phy_mode;
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    AIR_PORT_STATUS_T   port_status;
    AIR_CFG_VALUE_T     ignore_an;
    AIR_CFG_VALUE_T     serdes_op_mode;
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

    osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
    serdes_op_mode.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

    /* Get port link status from ifmon DB */
    osal_memset(&port_status, 0, sizeof(AIR_PORT_STATUS_T));
    hal_coral_ifmon_getPortStatus(unit, port, &port_status);
    aml_readReg(unit, CORAL_PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
    reg_mcr |= BIT(CORAL_MCR_FORCE_MODE_OFFT);

    if (TRUE == link)
    {
        aml_readReg(unit, CORAL_PMSR(mac_port), &reg_msr, sizeof(reg_msr));
        if (AIR_PORT_DUPLEX_HALF == port_status.duplex)
        {
            reg_mcr &= ~BIT(CORAL_MCR_FORCE_DUPLEX_OFFT);
        }
        else
        {
            reg_mcr |= BIT(CORAL_MCR_FORCE_DUPLEX_OFFT);
        }

        reg_mcr &= ~BIT(CORAL_MCR_EEE_1G_EN_OFFT);
        reg_mcr &= ~BIT(CORAL_MCR_EEE_100_EN_OFFT);
        reg_mcr &= ~BIT(CORAL_MCR_EEE_2P5G_EN_OFFT);
        reg_mcr &= ~BIT(CORAL_MCR_EEE_5G_EN_OFFT);
        reg_mcr &= ~BIT(CORAL_MCR_EEE_10G_EN_OFFT);

        if (HAL_PHY_SERDES_OP_MODE_SPEED_FIX != serdes_op_mode.value)
        {
            rc = hal_phy_getPhyOpMode(unit, port, &phy_mode);
            if (AIR_E_OK == rc)
            {
                if (HAL_PHY_OP_MODE_NORMAL == phy_mode)
                {
                    if (AIR_PORT_STATUS_FLAGS_EEE & port_status.flags)
                    {
                        if (AIR_PORT_SPEED_1000M == port_status.speed)
                        {
                            reg_mcr |= BIT(CORAL_MCR_EEE_1G_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_100M == port_status.speed)
                        {
                            reg_mcr |= BIT(CORAL_MCR_EEE_100_EN_OFFT);
                        }
                        if (AIR_PORT_SPEED_2500M == port_status.speed)
                        {
                            reg_mcr |= BIT(CORAL_MCR_EEE_2P5G_EN_OFFT);
                        }
                    }
                }
            }

            /* Update Flow Ctrl */
            if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX) & port_status.flags)
            {
                reg_mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
                reg_mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX & port_status.flags) != 0)
            {
                reg_mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if ((AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX & port_status.flags) != 0)
            {
                reg_mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
            }
            else
            {
                reg_mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
                reg_mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
        }
        osal_memset(&ignore_an, 0, sizeof(AIR_CFG_VALUE_T));
        ignore_an.value = 0;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_BACKPRES_CTRL_MODE, &ignore_an);
        if (AIR_E_OK == rc)
        {
            if (0 == ignore_an.value)
            {
                /* handle half mode back pressure */
                if (AIR_PORT_DUPLEX_HALF == port_status.duplex)
                {
                    if (reg_mcr & BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT))
                    {
                        reg_mcr |= BIT(CORAL_MCR_BACK_PRESSURE_OFFT);
                    }
                    else
                    {
                        reg_mcr &= ~BIT(CORAL_MCR_BACK_PRESSURE_OFFT);
                    }
                }
            }
        }

        if ((ptr_sdk_port_map_entry->max_speed == AIR_INIT_PORT_SPEED_2500M) &&
            (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port)))
        {
            /* Config speed to 2500M Full */
            port_speed = HAL_PHY_SPEED_2500M;
            reg_mcr |= BIT(CORAL_MCR_FORCE_DUPLEX_OFFT);
        }
        else
        {
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
                case AIR_PORT_SPEED_5000M:
                    port_speed = HAL_PHY_SPEED_5000M;
                    break;
                case AIR_PORT_SPEED_10000M:
                    port_speed = HAL_PHY_SPEED_10000M;
                    break;
                default:
                    port_speed = HAL_PHY_SPEED_100M;
                    break;
            }
        }
        reg_mcr &= ~BITS_RANGE(CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);
        reg_mcr |= BITS_OFF_L(port_speed, CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);
        /* Update link status */
        reg_mcr |= BIT(CORAL_MCR_FORCE_LINK_OFFT);
    }
    else
    {
        /* Disable EEE when the port is link down */
        rc = hal_phy_getPhyOpMode(unit, port, &phy_mode);
        if ((AIR_E_OK == rc) && (HAL_PHY_OP_MODE_NORMAL == phy_mode))
        {
            reg_mcr &= ~BIT(CORAL_MCR_EEE_1G_EN_OFFT);
        }
        /* Update link status */
        reg_mcr &= ~BIT(CORAL_MCR_FORCE_LINK_OFFT);
    }

    aml_writeReg(unit, CORAL_PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
    DIAG_PRINT(HAL_DBG_INFO, "Update pmcr = 0x%x\n", reg_mcr);
}

static AIR_ERROR_NO_T
_hal_coral_port_initCpu(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              cpu_air_port = 0;
    UI32_T              cpu_mac_port = 0;
    UI32_T              reg_data = 0;
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    AIR_PORT_SPEED_T    speed;

    /* Check if the system has assigned CPU port */
    if (AIR_PORT_INVALID == HAL_CPU_PORT(unit))
    {
        return AIR_E_OK;
    }

    /* Get CPU port */
    cpu_air_port = HAL_CPU_PORT(unit);
    if (!HAL_IS_PORT_VALID(unit, cpu_air_port))
    {
        rc = AIR_E_OP_INVALID;
    }
    else
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, cpu_air_port, cpu_mac_port);

        /* Set CORAL_MFC */
        rc = aml_readReg(unit, CORAL_MFC, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            reg_data |= BITS_OFF_L(cpu_mac_port, CORAL_CSR_CPU_PORT, CORAL_CSR_CPU_PORT_LENGTH);
            reg_data |= BIT(CORAL_CSR_CPU_PORT_EN);
            rc = aml_writeReg(unit, CORAL_MFC, &reg_data, sizeof(reg_data));
        }

        if (AIR_E_OK == rc)
        {
            /* Set Speical Tag, insert mode */
            rc = hal_coral_stag_setPort(unit, cpu_air_port, TRUE);
        }

        if (AIR_E_OK == rc)
        {
            rc = hal_coral_stag_setMode(unit, cpu_air_port, AIR_STAG_MODE_INSERT);
        }

        if (AIR_E_OK == rc)
        {
            /* Set MCR */
            /* Set force full duplex */
            rc = hal_coral_port_setDuplex(unit, cpu_air_port, AIR_PORT_DUPLEX_FULL);
        }

        if (AIR_E_OK == rc)
        {
            /* Set force tx,rx FC on */
            rc = hal_coral_port_setFlowCtrl(unit, cpu_air_port, AIR_PORT_DIR_BOTH, FALSE);
        }

        if (AIR_E_OK == rc)
        {
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
            rc = hal_coral_port_setSpeed(unit, cpu_air_port, speed);
        }

        if (AIR_E_OK == rc)
        {
            /* Set force mode enable */
            /* Set back pressure enable */
            /* Set EEE 100m,1G,2.5G disable */
            rc = aml_readReg(unit, CORAL_PMCR(cpu_mac_port), &reg_data, sizeof(reg_data));
            if (AIR_E_OK == rc)
            {
                reg_data |= BIT(CORAL_MCR_FORCE_MODE_OFFT);
                reg_data |= BIT(CORAL_MCR_BACK_PRESSURE_OFFT);

                reg_data &= ~BIT(CORAL_MCR_EEE_100_EN_OFFT);
                reg_data &= ~BIT(CORAL_MCR_EEE_1G_EN_OFFT);
                reg_data &= ~BIT(CORAL_MCR_EEE_2P5G_EN_OFFT);
                rc = aml_writeReg(unit, CORAL_PMCR(cpu_mac_port), &reg_data, sizeof(reg_data));
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_port_setLedGpioReg(
    const UI32_T unit)
{
    UI32_T          max_gpio = 0, gpio_num = 0, aux_val = 0, max_clear_rg = 0, count = 0;
    UI32_T          u32dat = 0, index = 0, port = 0, phy_port = 0, data = 0;
    AIR_CFG_VALUE_T led_num, led_type;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    osal_memset(&led_num, 0, sizeof(AIR_CFG_VALUE_T));
    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));

    led_num.value = 2;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_num);

    led_type.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);

    if (HAL_CORAL_PORT_LED_TYPE_SERIAL == led_type.value)
    {
        gpio_num = HAL_CORAL_PORT_SLED_CFG_GPIO_NUM;
        max_gpio = (gpio_num - 1);
        aux_val = 0x1;

        /* bypass hwtrap */
        rv = aml_readReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rv)
        {
            u32dat |= BITS(0, max_gpio);
            rv = aml_writeReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
        }

        /* setup serial LED */
        if (AIR_E_OK == rv)
        {
            u32dat = ((led_num.value << 1) + CORAL_SLED_RISING_EDGE + CORAL_SLED_OUTPUT_TRANSITION_MODE +
                      CORAL_SLED_SOURCE_NORMAL_MODE) |
                     (0xFF << 8);
            rv = aml_writeReg(unit, CORAL_SLED_CTRL0, &u32dat, sizeof(u32dat));
        }

        /* set gpio pin to aux function*/
        if (AIR_E_OK == rv)
        {
            for (index = 0; index < gpio_num; index++)
            {
                rv = aml_readReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rv)
                {
                    u32dat &= ~BITS_RANGE((index % 8) * 4, 4);
                    u32dat |= CORAL_REG_GPIO_AUX_CFG(index, aux_val);
                    rv = aml_writeReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));
                }
            }
        }
    }
    else if (HAL_CORAL_PORT_LED_TYPE_PARALLEL == led_type.value)
    {
        AIR_PORT_FOREACH(HAL_PORT_BMP_GPHY(unit), port)
        {
            phy_port++;
        }

        gpio_num = (phy_port * led_num.value);
        max_gpio = (gpio_num - 1);
        aux_val = 0xa;
        max_clear_rg = ((gpio_num / 4) + 1);

        /* bypass hwtrap */
        rv = aml_readReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rv)
        {
            u32dat |= BITS(0, max_gpio);
            rv = aml_writeReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
        }

        if (AIR_E_OK == rv)
        {
            u32dat = 0;
            /* clear LED mapping */
            for (index = 0; index < max_clear_rg; index++)
            {
                rv = aml_writeReg(unit, CORAL_RG_GPIO_LED_SEL(index * 4), &u32dat, sizeof(u32dat));
            }

            if (AIR_E_OK == rv)
            {
                AIR_PORT_FOREACH(HAL_PORT_BMP_GPHY(unit), port)
                {
                    HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_port);
                    for (index = 0; index < led_num.value; index++)
                    {
                        rv = aml_readReg(unit, CORAL_RG_GPIO_LED_SEL(count), &u32dat, sizeof(u32dat));
                        if (AIR_E_OK == rv)
                        {
                            u32dat &= ~(CORAL_RG_GPIO_LED_MASK(count));
                            data = ((phy_port * 4) + index);
                            u32dat |= CORAL_RG_GPIO_LED_VAL(count, data);
                            rv = aml_writeReg(unit, CORAL_RG_GPIO_LED_SEL(count), &u32dat, sizeof(u32dat));
                            count++;
                        }
                    }
                }
            }
        }

        if (AIR_E_OK == rv)
        {
            /* set gpio pin to aux function*/
            for (index = 0; index < gpio_num; index++)
            {
                rv = aml_readReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rv)
                {
                    u32dat |= CORAL_REG_GPIO_AUX_CFG(index, aux_val);
                    rv = aml_writeReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));
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

static AIR_ERROR_NO_T
_hal_coral_port_resetGbeLedBlinkDuration(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         port_index = 0;

    AIR_PORT_FOREACH(HAL_PORT_BMP_GPHY(unit), port_index)
    {
        rc |= hal_mdio_writeC45FromIntBusByPort(unit, port_index, MMD_DEV_VSPEC2, 0x720, 0xff);
    }
    AIR_PORT_FOREACH(HAL_PORT_BMP_GPHY(unit), port_index)
    {
        rc |= hal_mdio_writeC45FromIntBusByPort(unit, port_index, MMD_DEV_VSPEC2, 0x720, 0x0);
    }
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_port_init
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
hal_coral_port_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                port = 0;
    AIR_PORT_PHY_AN_ADV_T adv;
    HAL_SDK_PORT_MAP_T   *ptr_sdk_port_map_entry;
    AIR_CFG_VALUE_T       port_cfg;
    BOOL_T                admin_state;
    UI32_T                mac_port = 0;
    UI32_T                reg_mcr = 0;

    /* Initialize PHY */
    rc = hal_phy_init(unit);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "set unit %u port module initialize fail, rc = %d\n", unit, rc);
        hal_phy_deinit(unit);
    }
    else
    {
        rc = _hal_coral_port_resetGbeLedBlinkDuration(unit);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "reset unit %u GBE port LED blink duration fail, rc = %d\n", unit, rc);
            hal_phy_deinit(unit);
        }
        else
        {
            AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
            {
                /*  Since the cascade port is an internal interface, it will skip the default
                    value initialization  */
                if (HAL_IS_CASCADE_PORT_VALID(unit, port))
                {
                    continue;
                }
                /* Set all ether port as PHY disabled */
                rc = hal_coral_port_setAdminState(unit, port, FALSE);
                if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u disable fail, rc = %d\n", unit, port, rc);
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

                if (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port))
                {
                    /* Set all ether port as speed 100M on force mode */
                    rc = hal_coral_port_setSpeed(unit, port, AIR_PORT_SPEED_100M);
                    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u speed 100M fail, rc = %d\n", unit, port, rc);
                        break;
                    }

                    /* Set all ether port as full duplex on force mode */
                    rc = hal_coral_port_setDuplex(unit, port, AIR_PORT_DUPLEX_FULL);
                    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u full duplex fail, rc = %d\n", unit, port, rc);
                        break;
                    }
                }
                /* Set all ether port as flow control enable */
                rc = hal_coral_port_setFlowCtrl(unit, port, AIR_PORT_DIR_BOTH, TRUE);
                if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u flow control fail, rc = %d\n", unit, port, rc);
                    break;
                }

                if (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port))
                {
                    /* Set PHY advertisment of all ether ports */
                    rc = hal_coral_port_setPhyLocalAdvAbility(unit, port, &adv);
                    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u local adv ability fail, rc = %d\n", unit, port,
                                   rc);
                        break;
                    }

                    /* Set all ether port as AN mode */
                    rc = hal_coral_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_ENABLE);
                    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u auto-nego enable fail, rc = %d\n", unit, port, rc);
                        break;
                    }
                }

                /* Set all ether port as PHY enabled */
                osal_memset(&port_cfg, 0, sizeof(AIR_CFG_VALUE_T));
                port_cfg.value = 1; /* defaule port enable */
                port_cfg.param0 = port;
                hal_cfg_getValue(unit, AIR_CFG_TYPE_PORT_ADMIN_STATE, &port_cfg);

                admin_state = (port_cfg.value) ? TRUE : FALSE;
                rc = hal_coral_port_setAdminState(unit, port, admin_state);
                if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u = %d, rc = %d\n", unit, port, admin_state, rc);
                    break;
                }

                /* Set all port default port matrix pbm */
                rc = hal_coral_port_setPortMatrix(unit, port, HAL_PORT_BMP_TOTAL(unit));
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u port matrix failed, rc = %d\n", unit, port, rc);
                    break;
                }

                /* Update force mcr port */
                HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                aml_readReg(unit, CORAL_PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
                reg_mcr |= BIT(CORAL_MCR_FORCE_MODE_OFFT);
                aml_writeReg(unit, CORAL_PMCR(mac_port), &reg_mcr, sizeof(reg_mcr));
            }
        }

        if ((AIR_E_OK == rc) || (AIR_E_NOT_SUPPORT == rc))
        {
            /* Initialize CPU port */
            rc = _hal_coral_port_initCpu(unit);
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_ERR, "cpu init failed, rc = %d\n", rc);
            }
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "set unit %u port %u default value fail, rc = %d\n", unit, port, rc);
        }

        if (AIR_E_OK == rc)
        {
            rc = _hal_coral_port_setLedGpioReg(unit);
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_ERR, "set unit %u GBE port led gpio register fail, rc = %d\n", unit, rc);
            }
        }

        if (AIR_E_OK == rc)
        {
            if (HAL_MODULE_INITED(unit, AIR_MODULE_IFMON) & HAL_INIT_STAGE_TASK_RSRC)
            {
                /* add callback function for check link status */
                rc = hal_coral_ifmon_register(unit, _hal_coral_port_updateMac, NULL);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "register mac process callback function fail, unit %u rc = %d\n", unit, rc);
                }
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_deinit
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
hal_coral_port_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = hal_phy_deinit(unit);

    if (AIR_E_OK == rc)
    {
        rc = hal_coral_ifmon_deregister(unit, _hal_coral_port_updateMac, NULL);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setSerdesMode
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
hal_coral_port_setSerdesMode(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    HAL_PHY_SERDES_MODE_T hal_serdes_mode = HAL_PHY_SERDES_MODE_LAST;

    if (HAL_CPU_PORT(unit) == port)
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
            rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = hal_phy_setSerdesMode(unit, port, hal_serdes_mode);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "call hal_phy_setSerdesMode() fail, rc = %d\n", rc);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getSerdesMode
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
hal_coral_port_getSerdesMode(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    HAL_PHY_SERDES_MODE_T hal_serdes_mode = HAL_PHY_SERDES_MODE_LAST;

    if (HAL_CPU_PORT(unit) == port)
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

/* FUNCTION NAME: hal_coral_port_setPhyAutoNego
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
hal_coral_port_setPhyAutoNego(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    HAL_PHY_AUTO_NEGO_T hal_auto_nego = HAL_PHY_AUTO_NEGO_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPhyAutoNego
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
hal_coral_port_getPhyAutoNego(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    HAL_PHY_AUTO_NEGO_T hal_auto_nego = HAL_PHY_AUTO_NEGO_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setPhyLocalAdvAbility
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
hal_coral_port_setPhyLocalAdvAbility(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_PHY_AN_ADV_T hal_adv;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        osal_memset(&hal_adv, 0, sizeof(HAL_PHY_AN_ADV_T));

        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX, &hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE, &hal_adv,
                                  HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE, &hal_adv,
                                  HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE, &hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE);
        _HAL_CORAL_PORT_SET_FLAGS(ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_2500M, &hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M);
        rc = hal_phy_setLocalAdvAbility(unit, port, &hal_adv);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPhyLocalAdvAbility
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
hal_coral_port_getPhyLocalAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_PHY_AN_ADV_T hal_adv;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        rc = hal_phy_getLocalAdvAbility(unit, port, &hal_adv);
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "port %u local adv. flags = %x\n", port, hal_adv.flags);
            ptr_adv->flags = 0;
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv,
                                      AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv,
                                      AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv,
                                      AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv,
                                      AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv,
                                      AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
            _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
            DIAG_PRINT(HAL_DBG_INFO, "port %u AIR flags = %x\n", port, ptr_adv->flags);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPhyRemoteAdvAbility
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
hal_coral_port_getPhyRemoteAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_LINK_STATUS_T hal_status;
    HAL_PHY_AN_ADV_T      hal_adv;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        /* Check AN completed */
        rc = hal_phy_getLinkStatus(unit, port, &hal_status);
        if ((AIR_E_OK == rc) && (!(hal_status.flags & HAL_PHY_LINK_STATUS_FLAGS_LINK_UP)))
        {
            /* Port is link down */
            ptr_adv->flags = 0;
        }
        else if (AIR_E_OK == rc)
        {
            /* Get remote AN advertisement ability */
            rc = hal_phy_getRemoteAdvAbility(unit, port, &hal_adv);
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "port %u remote adv. flags = %x\n", port, hal_adv.flags);
                ptr_adv->flags = 0;
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_EEE, ptr_adv, AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
                _HAL_CORAL_PORT_SET_FLAGS(&hal_adv, HAL_PHY_AN_ADV_FLAGS_2500M, ptr_adv,
                                          AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
                DIAG_PRINT(HAL_DBG_INFO, "port %u AIR flags = %x\n", port, ptr_adv->flags);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_WARN, "port %u get remote advertisement fail, rc = %d\n", port, rc);
            }
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "port %u get link status fail, rc = %d\n", port, rc);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setSpeed
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
hal_coral_port_setSpeed(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32mcr = 0;
    UI32_T          mac_port = 0;
    HAL_PHY_SPEED_T hal_speed = HAL_PHY_SPEED_LAST;
    AIR_CFG_VALUE_T serdes_op_mode;

    HAL_CHECK_ENUM_RANGE(speed, AIR_PORT_SPEED_LAST);
    osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
    serdes_op_mode.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

    /* If port is not CPU port, set PHY register */
    if (HAL_CPU_PORT(unit) != port)
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
            case AIR_PORT_SPEED_5000M:
                hal_speed = HAL_PHY_SPEED_5000M;
                break;
            case AIR_PORT_SPEED_10000M:
                hal_speed = HAL_PHY_SPEED_5000M;
                break;
            default:
                break;
        }
        rc = hal_phy_setSpeed(unit, port, hal_speed);
    }
    if ((AIR_E_OK == rc) || (AIR_E_NOT_SUPPORT == rc))
    {
        if (HAL_PHY_SERDES_OP_MODE_SPEED_FIX != serdes_op_mode.value)
        {
            /* Set MAC speed */
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

            /* Read data from register */
            aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));

            u32mcr &= ~BITS_RANGE(CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);
            u32mcr |= BITS_OFF_L(speed, CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);

            /* Write data to register */
            aml_writeReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getSpeed
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
hal_coral_port_getSpeed(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          mac_port = 0, u32mcr = 0;
    HAL_PHY_SPEED_T hal_speed = HAL_PHY_SPEED_LAST;

    /* If port is CPU port, get MAC register directly */
    if (HAL_CPU_PORT(unit) == port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        (*ptr_speed) = BITS_OFF_R(u32mcr, CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);
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
                case HAL_PHY_SPEED_5000M:
                    *ptr_speed = AIR_PORT_SPEED_5000M;
                    break;
                case HAL_PHY_SPEED_10000M:
                    *ptr_speed = AIR_PORT_SPEED_10000M;
                    break;
                default:
                    rc = AIR_E_OTHERS;
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

            /* Read data from register */
            aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
            (*ptr_speed) = BITS_OFF_R(u32mcr, CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG);
            rc = AIR_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_setDuplex
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
hal_coral_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           u32mcr = 0;
    UI32_T           mac_port = 0;
    AIR_CFG_VALUE_T  serdes_op_mode;

    HAL_PHY_DUPLEX_T hal_duplex = HAL_PHY_DUPLEX_LAST;
    HAL_CHECK_ENUM_RANGE(duplex, AIR_PORT_DUPLEX_LAST);
    osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
    serdes_op_mode.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

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
    }
    if ((AIR_E_OK == rc) || (AIR_E_NOT_SUPPORT == rc))
    {
        if (HAL_PHY_SERDES_OP_MODE_SPEED_FIX != serdes_op_mode.value)
        {
            /* Set MAC duplex */
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

            /* Read data from register */
            aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
            if (AIR_PORT_DUPLEX_FULL == duplex)
            {
                u32mcr |= BIT(CORAL_MCR_FORCE_DUPLEX_OFFT);
            }
            else
            {
                u32mcr &= ~BIT(CORAL_MCR_FORCE_DUPLEX_OFFT);
            }
            /* Write data to register */
            aml_writeReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getDuplex
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
hal_coral_port_getDuplex(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           mac_port = 0, u32mcr = 0;
    HAL_PHY_DUPLEX_T hal_duplex = HAL_PHY_DUPLEX_LAST;

    /* If port is CPU port, get MAC register directly*/
    if (HAL_CPU_PORT(unit) == port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
        (*ptr_duplex) = BITS_OFF_R(u32mcr, CORAL_MCR_FORCE_DUPLEX_OFFT, CORAL_MCR_FORCE_DUPLEX_LENG);
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
            aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
            (*ptr_duplex) = BITS_OFF_R(u32mcr, CORAL_MCR_FORCE_DUPLEX_OFFT, CORAL_MCR_FORCE_DUPLEX_LENG);
            rc = AIR_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_setBackPressure
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
hal_coral_port_setBackPressure(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
        if (TRUE == bckPres)
        {
            u32dat |= BIT(CORAL_MCR_BACK_PRESSURE_OFFT);
        }
        else
        {
            u32dat &= ~BIT(CORAL_MCR_BACK_PRESSURE_OFFT);
        }

        /* Write data to register */
        aml_writeReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getBackPressure
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
hal_coral_port_getBackPressure(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_bckPres)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        /* Read data from register */
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
        (*ptr_bckPres) = BITS_OFF_R(u32dat, CORAL_MCR_BACK_PRESSURE_OFFT, CORAL_MCR_BACK_PRESSURE_LENG);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setFlowCtrl
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
hal_coral_port_setFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en)
{
    UI32_T          u32mcr = 0, mac_port = 0;
    AIR_CFG_VALUE_T serdes_op_mode;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    osal_memset(&serdes_op_mode, 0, sizeof(AIR_CFG_VALUE_T));
    serdes_op_mode.param0 = port;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE, &serdes_op_mode);

    if (HAL_PHY_SERDES_OP_MODE_SPEED_FIX == serdes_op_mode.value)
    {
        DIAG_PRINT(HAL_DBG_WARN, "port %d is configured for fixed speed, this configuration must no be changed !\n",
                   port);
        return AIR_E_NOT_SUPPORT;
    }
    else
    {
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));

        if (TRUE == fc_en)
        {
            /* Enable port flow control */
            if (AIR_PORT_DIR_TX == dir)
            {
                u32mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
            }
            else if (AIR_PORT_DIR_RX == dir)
            {
                u32mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if (AIR_PORT_DIR_BOTH == dir)
            {
                u32mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
                u32mcr |= BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
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
                u32mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
            }
            else if (AIR_PORT_DIR_RX == dir)
            {
                u32mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else if (AIR_PORT_DIR_BOTH == dir)
            {
                u32mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT);
                u32mcr &= ~BIT(CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT);
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }
        aml_writeReg(unit, CORAL_PMCR(mac_port), &u32mcr, sizeof(u32mcr));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_port_getFlowCtrl
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
hal_coral_port_getFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read port flow control status*/
    aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
    if (AIR_PORT_DIR_TX == dir)
    {
        (*ptr_fc_en) = BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT, CORAL_MCR_FORCE_FLOWCTRL_TX_LENG);
    }
    else if (AIR_PORT_DIR_RX == dir)
    {
        (*ptr_fc_en) = BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT, CORAL_MCR_FORCE_FLOWCTRL_RX_LENG);
    }
    else
    {
        return AIR_E_NOT_SUPPORT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_port_setPhyLoopBack
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
hal_coral_port_setPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              lpBack_en)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_PHY_LPBK_T hal_phy_lpbk_dir = HAL_PHY_LPBK_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
            DIAG_PRINT(HAL_DBG_WARN, "port %d set loop back [%s]:%s failed, rc = %d\n", port,
                       (AIR_PORT_LPBK_DIR_NEAR_END == dir) ? "Near End" : "Far End",
                       (TRUE == lpBack_en) ? "enable" : "disable", rc);
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPhyLoopBack
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
hal_coral_port_getPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_lpBack_en)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_PHY_LPBK_T hal_phy_lpbk_dir = HAL_PHY_LPBK_LAST;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
            DIAG_PRINT(HAL_DBG_WARN, "port %d get loop back [%s] failed, rc = %d\n", port,
                       (AIR_PORT_LPBK_DIR_NEAR_END == dir) ? "Near End" : "Far End", rc);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPortStatus
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
hal_coral_port_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_PHY_LINK_STATUS_T hal_status;
    UI32_T                u32dat, u32datField = 0;
    UI32_T                mac_port = 0;
    HAL_PHY_AN_ADV_T      an_adv, an_lp;
    HAL_PHY_AUTO_NEGO_T   an = HAL_PHY_AUTO_NEGO_LAST;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    osal_memset(&an_adv, 0, sizeof(an_adv));
    osal_memset(&an_lp, 0, sizeof(an_lp));

    /* If port is CPU port, get MAC register directly */
    if (HAL_CPU_PORT(unit) == port)
    {
        /* Read data from MAC register */
        aml_readReg(unit, CORAL_PMSR(mac_port), &u32dat, sizeof(u32dat));

        _HAL_CORAL_PORT_SET_REG_BIT(BITS_OFF_R(u32dat, CORAL_REG_PORT_LINK_OFFT, CORAL_REG_PORT_LINK_LENG),
                                    ptr_ps->flags, AIR_PORT_STATUS_FLAGS_LINK_UP);
        ptr_ps->speed = BITS_OFF_R(u32dat, CORAL_REG_PORT_SPEED_OFFT, CORAL_REG_PORT_SPEED_LENG);
        ptr_ps->duplex = BITS_OFF_R(u32dat, CORAL_REG_PORT_DUPLEX_OFFT, CORAL_REG_PORT_DUPLEX_LENG);

        /* Get flow control status */
        if (AIR_PORT_DUPLEX_FULL == ptr_ps->duplex)
        {
            /* Full-duplex : read MSR flow control status */
            /* Read data from register */
            aml_readReg(unit, CORAL_PMSR(mac_port), &u32dat, sizeof(u32dat));
            u32datField = BITS_OFF_R(u32dat, CORAL_REG_PORT_TX_FLOWCTRL_OFFT, CORAL_REG_PORT_TX_FLOWCTRL_LENG);
            if (u32datField)
            {
                ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
            }
            u32datField = BITS_OFF_R(u32dat, CORAL_REG_PORT_RX_FLOWCTRL_OFFT, CORAL_REG_PORT_RX_FLOWCTRL_LENG);
            if (u32datField)
            {
                ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX;
            }
        }
        else
        {
            /* Half-duplex : read MCR backpressure setting */
            aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
            u32datField = BITS_OFF_R(u32dat, CORAL_MCR_BACK_PRESSURE_OFFT, CORAL_MCR_BACK_PRESSURE_LENG);
            if (u32datField)
            {
                ptr_ps->flags |= (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX);
            }
        }
    }
    else
    {
        osal_memset(&hal_status, 0, sizeof(HAL_PHY_LINK_STATUS_T));
        rc = hal_phy_getLinkStatus(unit, port, &hal_status);
        if (AIR_E_OK == rc)
        {
            osal_memset(ptr_ps, 0, sizeof(AIR_PORT_STATUS_T));
            _HAL_CORAL_PORT_SET_FLAGS(&hal_status, HAL_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_ps,
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
                case HAL_PHY_SPEED_5000M:
                    ptr_ps->speed = AIR_PORT_SPEED_5000M;
                    break;
                default:
                    rc = AIR_E_OTHERS;
                    DIAG_PRINT(HAL_DBG_WARN, "unknown type: hal_phy_getLinkStatus->speed(0x%x)\n", hal_status.speed);
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
                    rc = AIR_E_OTHERS;
                    DIAG_PRINT(HAL_DBG_WARN, "unknown type: hal_phy_getLinkStatus->duplex(0x%x)\n", hal_status.duplex);
                    break;
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
                                ptr_ps->flags |=
                                    (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX);
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
                            aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
                            if (HAL_PHY_DUPLEX_FULL == hal_status.duplex)
                            {
                                u32datField = BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT,
                                                         CORAL_MCR_FORCE_FLOWCTRL_TX_LENG);
                                if (u32datField)
                                {
                                    ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                                }
                                u32datField = BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT,
                                                         CORAL_MCR_FORCE_FLOWCTRL_RX_LENG);
                                if (u32datField)
                                {
                                    ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX;
                                }
                            }
                            else
                            {
                                u32datField =
                                    BITS_OFF_R(u32dat, CORAL_MCR_BACK_PRESSURE_OFFT, CORAL_MCR_BACK_PRESSURE_LENG);
                                if (u32datField)
                                {
                                    ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /* read mcr */
                aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
                if (HAL_PHY_DUPLEX_FULL == hal_status.duplex)
                {
                    u32datField =
                        BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_TX_OFFT, CORAL_MCR_FORCE_FLOWCTRL_TX_LENG);
                    if (u32datField)
                    {
                        ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX;
                    }
                    u32datField =
                        BITS_OFF_R(u32dat, CORAL_MCR_FORCE_FLOWCTRL_RX_OFFT, CORAL_MCR_FORCE_FLOWCTRL_RX_LENG);
                    if (u32datField)
                    {
                        ptr_ps->flags |= AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX;
                    }
                }
                else
                {
                    u32datField = BITS_OFF_R(u32dat, CORAL_MCR_BACK_PRESSURE_OFFT, CORAL_MCR_BACK_PRESSURE_LENG);
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

/* FUNCTION NAME: hal_coral_port_setAdminState
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
hal_coral_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;
    DIAG_PRINT(HAL_DBG_INFO, "port %u, state = %d\n", port, state);

    HAL_PHY_ADMIN_STATE_T phyState = HAL_PHY_ADMIN_STATE_LAST;
    /* If port is not CPU port, set PHY register */
    if (HAL_CPU_PORT(unit) != port)
    {
        phyState = (state) ? HAL_PHY_ADMIN_STATE_ENABLE : HAL_PHY_ADMIN_STATE_DISABLE;

        if (AIR_E_OK != hal_phy_setAdminState(unit, port, phyState))
        {
            DIAG_PRINT(HAL_DBG_INFO, "set hal_phy_setAdminState fail\n");
            rc = AIR_E_OTHERS;
        }
    }
    else
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
        aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
        if (TRUE == state)
        {
            u32dat |= BIT(CORAL_MCR_FORCE_LINK_OFFT);
        }
        else
        {
            u32dat &= ~BIT(CORAL_MCR_FORCE_LINK_OFFT);
        }
        aml_writeReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_getAdminState
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
hal_coral_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                mac_port = 0, u32dat = 0;
    HAL_PHY_ADMIN_STATE_T phyState = HAL_PHY_ADMIN_STATE_LAST;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    if (HAL_CPU_PORT(unit) != port)
    {
        if (AIR_E_OK != hal_phy_getAdminState(unit, port, &phyState))
        {
            DIAG_PRINT(HAL_DBG_INFO, "set hal_phy_getAdminState fail\n");
            rc = AIR_E_OTHERS;
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
    else /* If port is CPU port, get MAC register directly */
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

        aml_readReg(unit, CORAL_PMCR(mac_port), &u32dat, sizeof(u32dat));

        if (u32dat & BIT(CORAL_MCR_FORCE_LINK_OFFT))
        {
            (*ptr_state) = TRUE;
        }
        else
        {
            (*ptr_state) = FALSE;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_setPhySmartSpeedDown
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
hal_coral_port_setPhySmartSpeedDown(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T mode)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    HAL_PHY_SSD_MODE_T hal_mode = 0;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
            DIAG_PRINT(HAL_DBG_INFO, "set hal_phy_setSmartSpeedDown fail, rc = %d\n", rc);
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPhySmartSpeedDown
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
hal_coral_port_getPhySmartSpeedDown(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    HAL_PHY_SSD_MODE_T hal_mode = HAL_PHY_SSD_MODE_DISABLE;

    if (HAL_CPU_PORT(unit) == port)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
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
            DIAG_PRINT(HAL_DBG_INFO, "set hal_phy_getSmartSpeedDown fail, rc = %d\n", rc);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setPortMatrix
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
hal_coral_port_setPortMatrix(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            mac_port = 0;
    AIR_PORT_BITMAP_T mac_port_bitmap;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, port_bitmap, mac_port_bitmap);
    aml_writeReg(unit, CORAL_PORTMATRIX(mac_port), &mac_port_bitmap[0], sizeof(UI32_T));
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getPortMatrix
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
hal_coral_port_getPortMatrix(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            mac_port = 0;
    AIR_PORT_BITMAP_T mac_port_bitmap;

    AIR_PORT_BITMAP_CLEAR(mac_port_bitmap);
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, CORAL_PORTMATRIX(mac_port), &mac_port_bitmap[0], sizeof(UI32_T));
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, mac_port_bitmap, port_bitmap);
    return rc;
}

/* FUNCTION NAME: hal_coral_port_setVlanMode
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
hal_coral_port_setVlanMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, CORAL_PCR(mac_port), &val, sizeof(UI32_T));
    val &= ~CORAL_PCR_PORT_VLAN_MASK;
    val |= (mode & CORAL_PCR_PORT_VLAN_RELMASK) << CORAL_PCR_PORT_VLAN_OFFT;
    aml_writeReg(unit, CORAL_PCR(mac_port), &val, sizeof(UI32_T));
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getVlanMode
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
hal_coral_port_getVlanMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, CORAL_PCR(mac_port), &val, sizeof(UI32_T));
    *ptr_mode = (val >> CORAL_PCR_PORT_VLAN_OFFT) & CORAL_PCR_PORT_VLAN_RELMASK;
    return rc;
}

/* FUNCTION NAME:   hal_coral_port_setPhyLedOnCtrl
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
hal_coral_port_setPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   hal_coral_port_getPhyLedOnCtrl
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
hal_coral_port_getPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME: hal_coral_port_setComboMode
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
hal_coral_port_setComboMode(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T combo_mode)
{
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME: hal_coral_port_getComboMode
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
hal_coral_port_getComboMode(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode)
{
    return AIR_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   hal_coral_port_setPhyLedCtrlMode
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
hal_coral_port_setPhyLedCtrlMode(
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
        DIAG_PRINT(HAL_DBG_WARN, "port %u set led-id[%u]:%s failed, rc = %d\n", port, led_id,
                   (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == ctrl_mode) ? "force" : "phy", rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_port_getPhyLedCtrlMode
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
hal_coral_port_getPhyLedCtrlMode(
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

/* FUNCTION NAME:   hal_coral_port_setPhyLedForceState
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
hal_coral_port_setPhyLedForceState(
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
        DIAG_PRINT(HAL_DBG_WARN, "port %u set led-id[%u] state:%d failed, rc = %d\n", port, led_id, state, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_port_getPhyLedForceState
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
hal_coral_port_getPhyLedForceState(
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

/* FUNCTION NAME:   hal_coral_port_setPhyLedForcePattCfg
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
hal_coral_port_setPhyLedForcePattCfg(
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
        DIAG_PRINT(HAL_DBG_WARN, "port %u set led-id[%u] patt:%d failed, rc = %d\n", port, led_id, pattern, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_port_getPhyLedForcePattCfg
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
hal_coral_port_getPhyLedForcePattCfg(
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

/* FUNCTION NAME: hal_coral_port_triggerCableTest
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
hal_coral_port_triggerCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    DIAG_PRINT(HAL_DBG_ERR, "Function %s has not been implemented!\n", __func__);
    return rc;
}

/* FUNCTION NAME: hal_coral_port_getCableTestRawData
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
hal_coral_port_getCableTestRawData(
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

/* FUNCTION NAME:   hal_coral_port_setPhyOpMode
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
hal_coral_port_setPhyOpMode(
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

/* FUNCTION NAME:   hal_coral_port_getPhyOpMode
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
hal_coral_port_getPhyOpMode(
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

/* FUNCTION NAME: hal_coral_port_triggerLinkDownCableTest
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
hal_coral_port_triggerLinkDownCableTest(
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

/* FUNCTION NAME: hal_coral_port_dumpDebugInfo
 * PURPOSE:
 *      Dump port debug information.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
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
hal_coral_port_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    osal_printf("\n[MAC status] =============================================================\n");
    /* MCR */
    aml_readReg(unit, CORAL_PMCR(mac_port), &reg_data, sizeof(UI32_T));
    osal_printf(" MCR 0x%x = 0x%x\n", CORAL_PMCR(mac_port), reg_data);
    osal_printf("  force mode = %u          backpr_enable = %u        rx_fc = %u\n",
                BITS_OFF_R(reg_data, CORAL_MCR_FORCE_MODE_OFFT, CORAL_MCR_FORCE_MODE_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_BACK_PRESSURE_OFFT, CORAL_MCR_BACK_PRESSURE_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_FORCE_RX_ENABLE_OFFT, CORAL_MCR_FORCE_RX_ENABLE_LENG));
    osal_printf("  tx_fc = %u\n", BITS_OFF_R(reg_data, CORAL_MCR_FORCE_TX_ENABLE_OFFT, CORAL_MCR_FORCE_TX_ENABLE_LENG));

    osal_printf("  eee_2.5g_enable = %u     eee_1g_enable = %u        eee_100m_enable = %u\n",
                BITS_OFF_R(reg_data, CORAL_MCR_EEE_2P5G_EN_OFFT, CORAL_MCR_EEE_EN_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_EEE_1G_EN_OFFT, CORAL_MCR_EEE_EN_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_EEE_100_EN_OFFT, CORAL_MCR_EEE_EN_LENG));

    osal_printf("  link = %u                speed = %u                duplex = %u\n",
                BITS_OFF_R(reg_data, CORAL_MCR_FORCE_LINK_OFFT, CORAL_MCR_FORCE_LINK_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_FORCE_SPEED_OFFT, CORAL_MCR_FORCE_SPEED_LENG),
                BITS_OFF_R(reg_data, CORAL_MCR_FORCE_DUPLEX_OFFT, CORAL_MCR_FORCE_DUPLEX_LENG));

    /* MSR */
    aml_readReg(unit, CORAL_PMSR(mac_port), &reg_data, sizeof(UI32_T));
    osal_printf(" MSR 0x%x = 0x%x\n", CORAL_PMSR(mac_port), reg_data);

    osal_printf("  lpi_status = %u          rx_fc = %u                tx_fc = %u\n",
                BITS_OFF_R(reg_data, CORAL_REG_PORT_LPI_STS_OFFT, CORAL_REG_PORT_LPI_STS_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_RX_FLOWCTRL_OFFT, CORAL_REG_PORT_RX_FLOWCTRL_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_TX_FLOWCTRL_OFFT, CORAL_REG_PORT_TX_FLOWCTRL_LENG));
    osal_printf("  eee_2.5g = %u            eee_1g = %u               eee_100m = %u\n",
                BITS_OFF_R(reg_data, CORAL_REG_PORT_EEE_2P5G_EN_OFFT, CORAL_REG_PORT_EEE_EN_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_EEE_1G_EN_OFFT, CORAL_REG_PORT_EEE_EN_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_EEE_100_EN_OFFT, CORAL_REG_PORT_EEE_EN_LENG));
    osal_printf("  link = %u                speed = %u                duplex = %u\n",
                BITS_OFF_R(reg_data, CORAL_REG_PORT_LINK_OFFT, CORAL_REG_PORT_LINK_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_SPEED_OFFT, CORAL_REG_PORT_SPEED_LENG),
                BITS_OFF_R(reg_data, CORAL_REG_PORT_DUPLEX_OFFT, CORAL_REG_PORT_DUPLEX_LENG));

    /* GMACCR */
    aml_readReg(unit, CORAL_GMACCR, &reg_data, sizeof(UI32_T));
    osal_printf(" GMACCR 0x%x = 0x%x\n", CORAL_GMACCR, reg_data);
    osal_printf("  mtcc = %u\n", BITS_OFF_R(reg_data, CORAL_GMACCR_MTCC_OFFT, CORAL_GMACCR_MTCC_LENG));
    /* call next stage for phy debug information */
    rc = hal_phy_dumpDebugInfo(unit, port);

    return rc;
}
