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

/* FILE NAME:  hal_pearl_chip.c
 * PURPOSE:
 *  Implement Chip module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_chip.h>

#include <air_cfg.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/pearl/hal_pearl_mdio.h>
#include <hal/switch/pearl/hal_pearl_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_CHIP_MDIO_DISABLE          (0)
#define HAL_PEARL_CHIP_MDIO_ENABLE           (1)
#define HAL_PEARL_CHIP_MDIO_CLK_CFG_0        (0)  /* 700KHz */
#define HAL_PEARL_CHIP_MDIO_CLK_CFG_1        (1)  /* 2.8MHz */
#define HAL_PEARL_CHIP_MDIO_CLK_CFG_2        (2)  /* 5.6MHz */
#define HAL_PEARL_CHIP_MDIO_CLK_CFG_3        (3)  /* 11.2MHz */
#define HAL_PEARL_CHIP_MDIO_CLK_700KHZ       (70) /* 50MHz / (70+1) ~= 700KHz */
#define HAL_PEARL_CHIP_MDIO_CLK_2800KHZ      (17) /* 50MHz / (17+1) ~= 2.8MHz */
#define HAL_PEARL_CHIP_MDIO_CLK_5600KHZ      (8)  /* 50MHz / (8+1)  ~= 5.6MHz */
#define HAL_PEARL_CHIP_MDIO_CLK_11200KHZ     (3)  /* 50MHz / (3+1)  ~= 11.2MHz */
#define HAL_PEARL_CHIP_HWRST_DEGLITCH_REG    (0x100000cc)
#define HAL_PEARL_CHIP_HWRST_DEGLITCH_ENABLE (0x7)
#define HAL_PEARL_CHIP_GPIO_PIN7             (7)
#define HAL_PEARL_CHIP_GPIO_PIN8             (8)
#define HAL_PEARL_CHIP_PKG_SEL_EXT_CPU       (0x182)
#define HAL_PEARL_CHIP_PKG_SEL_MASK          (0x187)
#define HAL_PEARL_CHIP_ASM_NOP_0             (0x846)
#define HAL_PEARL_CHIP_ASM_NOP_1             (0x4a)
#define HAL_PEARL_CHIP_CLK_CPU_ICG_ENABLE_EN (1 << 3)
#define HAL_PEARL_CHIP_TIMERCTLR_WDOG_EN     (1 << 25)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_CHIP, "hal_pearl_chip.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_pearl_chip_setMdioClock
 * PURPOSE:
 *      set mdio clock
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_OTHERS    --  Operation fail.
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_pearl_chip_setMdioClock(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T mdio_clk_cfg;
    UI32_T          data, mdio_clk_div, mdio_clk_sel;
    osal_memset(&mdio_clk_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    rv = hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_CLOCK, &mdio_clk_cfg);

    if (AIR_E_OK == rv)
    {
        aml_readReg(unit, CLK_MDIO_CFG, &data, sizeof(data));
        mdio_clk_sel = data & CSR_CLK_MDIO_MASK;

        switch (mdio_clk_cfg.value)
        {
            case HAL_PEARL_CHIP_MDIO_CLK_CFG_0:
                mdio_clk_div = HAL_PEARL_CHIP_MDIO_CLK_700KHZ;
                break;
            case HAL_PEARL_CHIP_MDIO_CLK_CFG_1:
                mdio_clk_div = HAL_PEARL_CHIP_MDIO_CLK_2800KHZ;
                break;
            case HAL_PEARL_CHIP_MDIO_CLK_CFG_2:
                mdio_clk_div = HAL_PEARL_CHIP_MDIO_CLK_5600KHZ;
                break;
            case HAL_PEARL_CHIP_MDIO_CLK_CFG_3:
                mdio_clk_div = HAL_PEARL_CHIP_MDIO_CLK_11200KHZ;
                break;
            default:
            {
                rv = AIR_E_OTHERS;
                mdio_clk_div = mdio_clk_sel;
                DIAG_PRINT(HAL_DBG_ERR, "configure mdio interface clock fail, type=%d, rv=%d\n", mdio_clk_cfg.value,
                           rv);
                break;
            }
        }

        if (mdio_clk_sel != mdio_clk_div)
        {
            data = (data & ~(CSR_CLK_MDIO_MASK)) | ((mdio_clk_div & CSR_CLK_MDIO_MASK) << CSR_CLK_MDIO_OFFT);
            aml_writeReg(unit, CLK_MDIO_CFG, &data, sizeof(data));
        }
    }

    return rv;
}

static AIR_ERROR_NO_T
_hal_pearl_chip_setIoCurrDriving(
    const UI32_T unit)
{
    AIR_CFG_VALUE_T mdio_enable;
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;

    osal_memset(&mdio_enable, 0, sizeof(AIR_CFG_VALUE_T));

    mdio_enable.value = HAL_PEARL_CHIP_MDIO_DISABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &mdio_enable);

    /* SMI uses 4 MA current driving capability */
    if (HAL_PEARL_CHIP_MDIO_ENABLE == mdio_enable.value)
    {
        aml_readReg(unit, PEARL_GPIO_L_E2_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        u32dat &= ~PEARL_GPIO_SMI_E2_BITS;
        aml_writeReg(unit, PEARL_GPIO_L_E2_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
    }

    /* I2C use 4MA current driving capability */
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, PEARL_GPIO_L_E2_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~PEARL_GPIO_I2C_E2_BITS;
            rc = aml_writeReg(unit, PEARL_GPIO_L_E2_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        }
    }
    return rc;
}

static BOOL_T
_hal_pearl_chip_checkI2cMasterEnable(
    const UI32_T unit)
{
    AIR_CFG_VALUE_T pled_enable, gpio7_enable, gpio8_enable;

    pled_enable.param0 = 0;
    pled_enable.param1 = 0;
    pled_enable.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_TYPE, &pled_enable);

    gpio7_enable.param0 = HAL_PEARL_CHIP_GPIO_PIN7;
    gpio7_enable.param1 = 0;
    gpio7_enable.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN, &gpio7_enable);

    gpio8_enable.param0 = HAL_PEARL_CHIP_GPIO_PIN8;
    gpio8_enable.param1 = 0;
    gpio8_enable.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN, &gpio8_enable);

    if (0 == (pled_enable.value | gpio7_enable.value | gpio8_enable.value))
    {
        return TRUE;
    }
    return FALSE;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_pearl_chip_readDeviceInfo
 * PURPOSE:
 *      To read the device/revision ID of the EFUSE.
 * INPUT:
 *      unit            -- the device unit
 * OUTPUT:
 *      ptr_device_id   -- pointer for the device ID
 *      ptr_revision_id -- pointer for the revision ID
 * RETURN:
 *      AIR_E_OK            -- Successfully get the IDs.
 *      AIR_E_BAD_PARAMETER -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_chip_readDeviceInfo(
    const UI32_T unit,
    UI32_T      *ptr_device_id,
    UI32_T      *ptr_revision_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    HAL_CHECK_PTR(ptr_device_id);
    HAL_CHECK_PTR(ptr_revision_id);

    aml_readReg(unit, PEARL_REG_RGS_PKG_SEL, &u32dat, sizeof(u32dat));
    (*ptr_device_id) = u32dat & BITS(0, 2);
    aml_readReg(unit, PEARL_REG_REVISION_ID, &u32dat, sizeof(u32dat));
    (*ptr_revision_id) = u32dat;
    return rc;
}

/* FUNCTION NAME: hal_pearl_chip_init
 * PURPOSE:
 *      Chip initialization
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_chip_init_param     --  chip init parameter
 *
 * RETURN:
 *        AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_chip_init(
    const UI32_T           unit,
    HAL_CHIP_INIT_PARAM_T *ptr_chip_init_param)
{
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_CFG_VALUE_T mdio_enable;
    UI32_T          u32dat = 0;
    HAL_CHECK_PTR(ptr_chip_init_param);

    /* Check if an external CPU is being used */
    aml_readReg(unit, PEARL_REG_RGS_PKG_SEL, &u32dat, sizeof(u32dat));
    if (HAL_PEARL_CHIP_PKG_SEL_EXT_CPU == (u32dat & HAL_PEARL_CHIP_PKG_SEL_MASK))
    {
        /* MCU NOP CMD */
        u32dat = HAL_PEARL_CHIP_ASM_NOP_0;
        aml_writeReg(unit, PEARL_REG_GDMP_RAM, &u32dat, sizeof(u32dat));
        u32dat = HAL_PEARL_CHIP_ASM_NOP_1;
        aml_writeReg(unit, PEARL_REG_GDMP_RAM + 4, &u32dat, sizeof(u32dat));
        aml_readReg(unit, PEARL_REG_CLK_CPU_ICG_ENABLE, &u32dat, sizeof(u32dat));
        if (!(u32dat & HAL_PEARL_CHIP_CLK_CPU_ICG_ENABLE_EN))
        {
            /* Enable MCU */
            u32dat |= HAL_PEARL_CHIP_CLK_CPU_ICG_ENABLE_EN;
            aml_writeReg(unit, PEARL_REG_CLK_CPU_ICG_ENABLE, &u32dat, sizeof(u32dat));
            osal_sleepTask(5);
            /* Disable MCU watchdog */
            aml_readReg(unit, PEARL_REG_TIMERCTLR, &u32dat, sizeof(u32dat));
            u32dat &= ~HAL_PEARL_CHIP_TIMERCTLR_WDOG_EN;
            aml_writeReg(unit, PEARL_REG_TIMERCTLR, &u32dat, sizeof(u32dat));
        }
    }

    /*
     * Disable MAC power saving
     * Note: Acl udf access clock will stop when all port link down.
     */
    aml_readReg(unit, PEARL_CKGCR, &u32dat, sizeof(u32dat));
    u32dat &= ~PEARL_CKG_LNKDN_GLB_STOP;
    aml_writeReg(unit, PEARL_CKGCR, &u32dat, sizeof(u32dat));

    osal_memset(&mdio_enable, 0, sizeof(AIR_CFG_VALUE_T));
    mdio_enable.value = HAL_PEARL_CHIP_MDIO_DISABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &mdio_enable);

    /*if mdio external bus is enable, then set gpio7 as mdc, gpio8 as mdio and config mdio clock*/
    if (HAL_PEARL_CHIP_MDIO_ENABLE == mdio_enable.value)
    {
        aml_readReg(unit, PEARL_RG_SMI_IOMUX, &u32dat, sizeof(u32dat));

        if (HAL_PEARL_CHIP_MDIO_DISABLE == (u32dat & PEARL_RG_SMI_MST_IOMUX_MASK))
        {
            u32dat |= ((HAL_PEARL_CHIP_MDIO_ENABLE & PEARL_RG_SMI_MST_IOMUX_RELMASK) << PEARL_RG_SMI_MST_IOMUX_OFFT);
            aml_writeReg(unit, PEARL_RG_SMI_IOMUX, &u32dat, sizeof(u32dat));
        }
        /* config mdio clock */
        rv = _hal_pearl_chip_setMdioClock(unit);
    }
    else
    {
        if (TRUE == _hal_pearl_chip_checkI2cMasterEnable(unit))
        {
            aml_readReg(unit, PEARL_RG_I2C_IOMUX, &u32dat, sizeof(u32dat));
            u32dat |= PEARL_RG_I2C_MST_IOMUX_MASK;
            aml_writeReg(unit, PEARL_RG_I2C_IOMUX, &u32dat, sizeof(u32dat));
        }
    }

    /* Enable HW reset de-glitch */
    u32dat = HAL_PEARL_CHIP_HWRST_DEGLITCH_ENABLE;
    aml_writeReg(unit, HAL_PEARL_CHIP_HWRST_DEGLITCH_REG, &u32dat, sizeof(u32dat));

    rv |= _hal_pearl_chip_setIoCurrDriving(unit);
    return rv;
}

/* FUNCTION NAME: hal_pearl_chip_deinit
 * PURPOSE:
 *      Chip deinitialization
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_chip_deinit(
    const UI32_T unit)
{
    return AIR_E_OK;
}
