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

/* FILE NAME:  hal_coral_chip.c
 * PURPOSE:
 *  Implement Chip module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/coral/hal_coral_mdio.h>
#include <hal/switch/coral/hal_coral_reg.h>
// #define MCU_DOWNLOAD_ENABLED

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_CHIP_DMEM_ADDRESS          (0x10023000)
#define HAL_CORAL_CHIP_MCU_RG_FW_VERSION     (0x10023FFC)
#define HAL_CORAL_CHIP_MAX_CHECK_RETRY       (5)
#define HAL_CORAL_CHIP_MCU_ENABLE            (0x30)
#define HAL_CORAL_CHIP_BOOT_MODE_REG         (0x1000003C)
#define HAL_CORAL_CHIP_RGS_PKG_SEL_OFFSET    (24)
#define HAL_CORAL_CHIP_RGS_PKG_SEL_LENGTH    (3)
#define HAL_CORAL_CHIP_EFUSE_DATA0_ADDR      (0x1000A500)
#define HAL_CORAL_CHIP_MDIO_DISABLE          (0)
#define HAL_CORAL_CHIP_MDIO_ENABLE           (1)
#define HAL_CORAL_CHIP_MDIO_CLK_CFG_0        (0)  /* 700KHz */
#define HAL_CORAL_CHIP_MDIO_CLK_CFG_1        (1)  /* 2.8MHz */
#define HAL_CORAL_CHIP_MDIO_CLK_CFG_2        (2)  /* 5.6MHz */
#define HAL_CORAL_CHIP_MDIO_CLK_CFG_3        (3)  /* 11.2MHz */
#define HAL_CORAL_CHIP_MDIO_CLK_700KHZ       (70) /* 50MHz / (70+1) ~= 700KHz */
#define HAL_CORAL_CHIP_MDIO_CLK_2800KHZ      (17) /* 50MHz / (17+1) ~= 2.8MHz */
#define HAL_CORAL_CHIP_MDIO_CLK_5600KHZ      (8)  /* 50MHz / (8+1)  ~= 5.6MHz */
#define HAL_CORAL_CHIP_MDIO_CLK_11200KHZ     (3)  /* 50MHz / (3+1)  ~= 11.2MHz */
#define HAL_CORAL_CHIP_CO_CLOCK_CONTROL_REG  (0x1000014C)
#define HAL_CORAL_CHIP_CO_CLOCK_MODE_ENABLE  (1)
#define HAL_CORAL_CHIP_CO_CLOCK_MODE_MASK    (0x7)
#define HAL_CORAL_CHIP_HWRST_DEGLITCH_REG    (0x10000074)
#define HAL_CORAL_CHIP_HWRST_DEGLITCH_ENABLE (0x7)

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_CHIP_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                     \
    {                                                                      \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));              \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));    \
    } while (0)

#define HAL_CORAL_CHIP_SET_BMU_CFG(__unit__, __cfg__)                                                                 \
    do                                                                                                                \
    {                                                                                                                 \
        UI32_T __u32dat;                                                                                              \
        aml_readReg(__unit__, CORAL_GFCCR0, &__u32dat, sizeof(__u32dat));                                             \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[0], CORAL_GFCCR0_FREE_BLK_HI_OFFT, CORAL_GFCCR0_FREE_BLK_HI_LENG); \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[1], CORAL_GFCCR0_FREE_BLK_LO_OFFT, CORAL_GFCCR0_FREE_BLK_LO_LENG); \
        aml_writeReg(__unit__, CORAL_GFCCR0, &__u32dat, sizeof(__u32dat));                                            \
        aml_readReg(__unit__, CORAL_GFCCR1, &__u32dat, sizeof(__u32dat));                                             \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[2], CORAL_GFCCR1_PORT_BLK_HI_OFFT, CORAL_GFCCR1_PORT_BLK_HI_LENG); \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[3], CORAL_GFCCR1_PORT_BLK_LO_OFFT, CORAL_GFCCR1_PORT_BLK_LO_LENG); \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[4], CORAL_GFCCR1_QUE_BLK_HI_OFFT, CORAL_GFCCR1_QUE_BLK_HI_LENG);   \
        aml_writeReg(__unit__, CORAL_GFCCR1, &__u32dat, sizeof(__u32dat));                                            \
        aml_readReg(__unit__, CORAL_GFCCR2, &__u32dat, sizeof(__u32dat));                                             \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR2_QUE_BLK_Q7_OFFT, CORAL_GFCCR2_QUE_BLK_Q7_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR2_QUE_BLK_Q6_OFFT, CORAL_GFCCR2_QUE_BLK_Q6_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR2_QUE_BLK_Q5_OFFT, CORAL_GFCCR2_QUE_BLK_Q5_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR2_QUE_BLK_Q4_OFFT, CORAL_GFCCR2_QUE_BLK_Q4_LENG);   \
        aml_writeReg(__unit__, CORAL_GFCCR2, &__u32dat, sizeof(__u32dat));                                            \
        aml_readReg(__unit__, CORAL_GFCCR3, &__u32dat, sizeof(__u32dat));                                             \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR3_QUE_BLK_Q3_OFFT, CORAL_GFCCR3_QUE_BLK_Q3_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR3_QUE_BLK_Q2_OFFT, CORAL_GFCCR3_QUE_BLK_Q2_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR3_QUE_BLK_Q1_OFFT, CORAL_GFCCR3_QUE_BLK_Q1_LENG);   \
        HAL_CORAL_CHIP_SET_VALUE(__u32dat, __cfg__[5], CORAL_GFCCR3_QUE_BLK_Q0_OFFT, CORAL_GFCCR3_QUE_BLK_Q0_LENG);   \
        aml_writeReg(__unit__, CORAL_GFCCR3, &__u32dat, sizeof(__u32dat));                                            \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
extern const UI32_T hal_coral_mcu_fw_len;
extern const UI8_T  hal_coral_mcu_fw[];

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_CHIP, "hal_coral_chip.c");
static const UI32_T hal_coral_chip_bmu_cfg_1[6] = {546, 479, 65, 33, 33, 18}; /* 6x1G + 2x2G5 + CPU */
static const UI32_T hal_coral_chip_bmu_cfg_2[6] = {540, 492, 65, 30, 30, 18}; /* 8x1G + 2x2G5 + CPU */
static const UI32_T hal_coral_chip_bmu_cfg_3[6] = {484, 432, 65, 33, 33, 20}; /* 8x1G + CPU */
static const UI32_T hal_coral_chip_bmu_cfg_4[6] = {553, 488, 72, 46, 46, 33}; /* 8x1G + 1x10G + CPU */

/* LOCAL SUBPROGRAM BODIES
 */
#ifdef MCU_DOWNLOAD_ENABLED
static AIR_ERROR_NO_T
_hal_coral_mcu_firmware_download(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         idx = 0, u32dat = 0, wLoop = 0;
    UI32_T        *ptrAddr;
    UI32_T         TargetAddr = HAL_CORAL_CHIP_DMEM_ADDRESS;

    u32dat = 0;
    rc = aml_writeReg(unit, HAL_CORAL_CHIP_MCU_RG_FW_VERSION, &u32dat, sizeof(UI32_T));
    for (idx = 0; idx <= hal_coral_mcu_fw_len; idx += 4)
    {
        ptrAddr = (UI32_T *)&hal_coral_mcu_fw[idx];
        rc |= aml_writeReg(unit, TargetAddr, ptrAddr, sizeof(UI32_T));
        TargetAddr += 4;
    }

    /* enable MCU */
    rc |= aml_readReg(unit, CORAL_REG_CLK_CPU_ICG_ENABLE, &u32dat, sizeof(UI32_T));
    u32dat |= HAL_CORAL_CHIP_MCU_ENABLE;
    rc |= aml_writeReg(unit, CORAL_REG_CLK_CPU_ICG_ENABLE, &u32dat, sizeof(UI32_T));

    /* get version code for check MCU is ready or not */
    for (wLoop = 0; wLoop < HAL_CORAL_CHIP_MAX_CHECK_RETRY; wLoop++)
    {
        osal_delayUs(1000);
        rc |= aml_readReg(unit, HAL_CORAL_CHIP_MCU_RG_FW_VERSION, &u32dat, sizeof(UI32_T));
        if (u32dat != 0)
        {
            break;
        }
    }

    if (wLoop >= HAL_CORAL_CHIP_MAX_CHECK_RETRY)
    {
        return AIR_E_OP_INCOMPLETE;
    }
    return rc;
}
#endif

static void
_hal_coral_chip_initBmuCfg(
    const UI32_T unit)
{
    const UI32_T *bmu_cfg = NULL;
    /* Initialize BMU threshold */
    switch (HAL_DEVICE_CHIP_ID(unit))
    {
        case HAL_CORAL_DEVICE_ID_AN8858B:
            bmu_cfg = hal_coral_chip_bmu_cfg_1;
            break;
        case HAL_CORAL_DEVICE_ID_AN8858C:
        case HAL_CORAL_DEVICE_ID_AN8858D:
            bmu_cfg = hal_coral_chip_bmu_cfg_2;
            break;
        case HAL_CORAL_DEVICE_ID_AN8858E:
        case HAL_CORAL_DEVICE_ID_AN8858F:
            bmu_cfg = hal_coral_chip_bmu_cfg_3;
            break;
        case HAL_CORAL_DEVICE_ID_AN8858H:
            bmu_cfg = hal_coral_chip_bmu_cfg_4;
            break;
        default:
            break;
    }
    if (NULL != bmu_cfg)
    {
        HAL_CORAL_CHIP_SET_BMU_CFG(unit, bmu_cfg);
    }
}

/* FUNCTION NAME: _hal_coral_chip_setMdioClock
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
_hal_coral_chip_setMdioClock(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T mdio_clk_cfg;
    UI32_T          data, mdio_clk_div, mdio_clk_sel;

    osal_memset(&mdio_clk_cfg, 0, sizeof(AIR_CFG_VALUE_T));

    /* default mdio clock is 2.8MHz */
    mdio_clk_cfg.value = HAL_CORAL_CHIP_MDIO_CLK_CFG_1;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_CLOCK, &mdio_clk_cfg);
    rc = aml_readReg(unit, CLK_MDIO_CFG, &data, sizeof(data));

    if (AIR_E_OK == rc)
    {
        mdio_clk_sel = data & CSR_CLK_MDIO_MASK;

        switch (mdio_clk_cfg.value)
        {
            case HAL_CORAL_CHIP_MDIO_CLK_CFG_0:
                mdio_clk_div = HAL_CORAL_CHIP_MDIO_CLK_700KHZ;
                break;
            case HAL_CORAL_CHIP_MDIO_CLK_CFG_1:
                mdio_clk_div = HAL_CORAL_CHIP_MDIO_CLK_2800KHZ;
                break;
            case HAL_CORAL_CHIP_MDIO_CLK_CFG_2:
                mdio_clk_div = HAL_CORAL_CHIP_MDIO_CLK_5600KHZ;
                break;
            case HAL_CORAL_CHIP_MDIO_CLK_CFG_3:
                mdio_clk_div = HAL_CORAL_CHIP_MDIO_CLK_11200KHZ;
                break;
            default:
            {
                rc = AIR_E_OTHERS;
                mdio_clk_div = mdio_clk_sel;
                DIAG_PRINT(HAL_DBG_ERR, "configure mdio interface clock fail, type=%d, rc=%d\n", mdio_clk_cfg.value,
                           rc);
                break;
            }
        }

        if (AIR_E_OK == rc)
        {
            if (mdio_clk_sel != mdio_clk_div)
            {
                data = (data & ~(CSR_CLK_MDIO_MASK)) | ((mdio_clk_div & CSR_CLK_MDIO_MASK) << CSR_CLK_MDIO_OFFT);
                rc = aml_writeReg(unit, CLK_MDIO_CFG, &data, sizeof(data));
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_chip_setIoCurrDriving(
    const UI32_T unit)
{
    AIR_CFG_VALUE_T mdio_enable;
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;

    osal_memset(&mdio_enable, 0, sizeof(AIR_CFG_VALUE_T));

    mdio_enable.value = HAL_CORAL_CHIP_MDIO_DISABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &mdio_enable);

    /* SMI uses 4 MA current driving capability */
    if (HAL_CORAL_CHIP_MDIO_ENABLE == mdio_enable.value)
    {
        aml_readReg(unit, CORAL_REG_IO_CFG0, &u32dat, sizeof(u32dat));
        u32dat &= ~CORAL_REG_IO_SMI_BITS;
        aml_writeReg(unit, CORAL_REG_IO_CFG0, &u32dat, sizeof(u32dat));
    }

    /* I2C use 4MA current driving capability */
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, CORAL_REG_IO_CFG1, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~CORAL_REG_IO_I2C_BIT19;
            rc = aml_writeReg(unit, CORAL_REG_IO_CFG1, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                rc = aml_readReg(unit, CORAL_REG_IO_CFG2, &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    u32dat &= ~CORAL_REG_IO_I2C_BIT20;
                    rc = aml_writeReg(unit, CORAL_REG_IO_CFG2, &u32dat, sizeof(u32dat));
                }
            }
        }
    }
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_chip_readDeviceInfo
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
hal_coral_chip_readDeviceInfo(
    const UI32_T unit,
    UI32_T      *ptr_device_id,
    UI32_T      *ptr_revision_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32_data = 0, efuse_date = 0;

    HAL_CHECK_PTR(ptr_device_id);
    HAL_CHECK_PTR(ptr_revision_id);

    rc = aml_readReg(unit, HAL_CORAL_CHIP_BOOT_MODE_REG, &u32_data, sizeof(u32_data));
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, HAL_CORAL_CHIP_EFUSE_DATA0_ADDR, &efuse_date, sizeof(efuse_date));
        if ((AIR_E_OK == rc) && (efuse_date != 0))
        {
            (*ptr_device_id) =
                (BITS_OFF_R(u32_data, HAL_CORAL_CHIP_RGS_PKG_SEL_OFFSET, HAL_CORAL_CHIP_RGS_PKG_SEL_LENGTH) << 2) |
                BITS_OFF_R(efuse_date, 0, 2);
        }
    }
    DIAG_PRINT(HAL_DBG_INFO, "unit=%u, device_id=%d\n", unit, (*ptr_device_id));
    return rc;
}

/* FUNCTION NAME: hal_coral_chip_init
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
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_chip_init(
    const UI32_T           unit,
    HAL_CHIP_INIT_PARAM_T *ptr_chip_init_param)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T chip_cfg;
    UI32_T          u32dat = 0, index = 0, aux_val = 1;

#ifdef MCU_DOWNLOAD_ENABLED
    rc = _hal_coral_mcu_firmware_download(unit);
#endif

    /*
     * Disable MAC power saving
     * Note: Acl udf access clock will stop when all port link down.
     */
    aml_readReg(unit, CORAL_CKGCR, &u32dat, sizeof(u32dat));
    u32dat &= ~CORAL_CKG_LNKDN_GLB_STOP;
    aml_writeReg(unit, CORAL_CKGCR, &u32dat, sizeof(u32dat));

    rc = aml_readReg(unit, CORAL_REG_HWTRAP, &u32dat, sizeof(u32dat));

    if (AIR_E_OK == rc)
    {
        /* mdio external bus is configured by hw trap. */
        if (BITS_OFF_R(u32dat, 13, 2) == 2)
        {
            rc = _hal_coral_chip_setMdioClock(unit);
        }
        else
        {
            osal_memset(&chip_cfg, 0, sizeof(AIR_CFG_VALUE_T));
            chip_cfg.value = HAL_CORAL_CHIP_MDIO_DISABLE;
            hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &chip_cfg);

            /*if mdio external bus is enable, then set gpio8 as mdc, gpio9 as mdio and config mdio clock*/
            if (HAL_CORAL_CHIP_MDIO_ENABLE == chip_cfg.value)
            {
                rc = aml_readReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    u32dat |= BITS(8, 9);
                    rc = aml_writeReg(unit, CORAL_REG_HWTRAP2IOMUX_BYPASS, &u32dat, sizeof(u32dat));
                }

                if (AIR_E_OK == rc)
                {
                    for (index = 8; index <= 9; index++)
                    {
                        rc = aml_readReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));
                        if (AIR_E_OK == rc)
                        {
                            u32dat &= ~BITS_RANGE((index % 8) * 4, 4);
                            u32dat |= CORAL_REG_GPIO_AUX_CFG(index, aux_val);
                            rc = aml_writeReg(unit, CORAL_REG_GPIO_MODE_SEL(index), &u32dat, sizeof(u32dat));

                            if (AIR_E_OK != rc)
                            {
                                break;
                            }
                        }
                    }
                }

                /* config mdio clock */
                if (AIR_E_OK == rc)
                {
                    rc = _hal_coral_chip_setMdioClock(unit);
                }
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(&chip_cfg, 0, sizeof(AIR_CFG_VALUE_T));
        chip_cfg.value = HAL_CORAL_CHIP_CO_CLOCK_MODE_ENABLE;
        hal_cfg_getValue(unit, AIR_CFG_TYPE_CHIP_CO_CLOCK_ENABLE, &chip_cfg);
        if (HAL_CORAL_CHIP_CO_CLOCK_MODE_ENABLE != chip_cfg.value)
        {
            /* Disabled co-clock pin output */
            rc = aml_readReg(unit, HAL_CORAL_CHIP_CO_CLOCK_CONTROL_REG, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                u32dat |= HAL_CORAL_CHIP_CO_CLOCK_MODE_MASK;
                aml_writeReg(unit, HAL_CORAL_CHIP_CO_CLOCK_CONTROL_REG, &u32dat, sizeof(u32dat));
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        _hal_coral_chip_initBmuCfg(unit);
    }

    /* Enable HW reset de-glitch */
    u32dat = HAL_CORAL_CHIP_HWRST_DEGLITCH_ENABLE;
    aml_writeReg(unit, HAL_CORAL_CHIP_HWRST_DEGLITCH_REG, &u32dat, sizeof(u32dat));

    rc = _hal_coral_chip_setIoCurrDriving(unit);
    return rc;
}

/* FUNCTION NAME: hal_coral_chip_deinit
 * PURPOSE:
 *      Chip deinitialization
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_chip_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}
