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

/* FILE NAME:  hal_sco_chip.c
 * PURPOSE:
 *  Implement Chip module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_chip.h>

#include <air_cfg.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/sco/hal_sco_mdio.h>
#include <hal/switch/sco/hal_sco_perif.h>
#include <hal/switch/sco/hal_sco_reg.h>
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
#include <cmlib/cmlib_crc.h>
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_CHIP_RESET_CONTROL_REG_2_DEFAULT_VALUE (0)
#define HAL_SCO_CHIP_RG_LAN_LED_IOMUX_DEFAULT_VALUE    (0)
#define HAL_SCO_CHIP_RG_GPIO_INVERSE_DEFAULT_VALUE     (0)
#define HAL_SCO_CHIP_RG_GPIO_EN_REG_DEFAULT_VALUE      (0)
#define HAL_SCO_CHIP_SLED_CTRL0_DEFAULT_VALUE          (0x00000016)
#define HAL_SCO_CHIP_PHY_SMI_DEFAULT_VALUE             (0x00FF8700)
#define HAL_SCO_CHIP_CKGCR_DEFAULT_VALUE               (0x1D4C0033)
#define HAL_SCO_CHIP_EFUSE_CTRL_DEFAULT_VALUE          (0)

#ifdef AIR_EN_CHIP_INIT_RESET
#define HAL_SCO_CHIP_CPU_CLOCK_GATING_REG  (0x100000C8)
#define HAL_SCO_CHIP_CPU_CLOCK_DISABLE     (0x0)
#define HAL_SCO_CHIP_CPU_CLOCK_ENABLE      (0x1)
#define HAL_SCO_CHIP_TIMER_CONTROL_REG     (0x1000A100)
#define HAL_SCO_CHIP_TIMER3_LOAD_VALUE_REG (0x1000A12C)
#define HAL_SCO_CHIP_WDOG_THRESHOLD_REG    (0x1000A134)
#define HAL_SCO_CHIP_RESET_TIME            (0x00AAE600)
#define HAL_SCO_CHIP_TIMER_INTERRUPT_EN    (0x00010001)
#define HAL_SCO_CHIP_WDOG_TIMER_EN         (0x02000021)
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
#define HAL_SCO_CHIP_WDT_DISABLE        (0x0)
#define HAL_SCO_CHIP_DMEM_ADDRESS       (0x80000000)
#define HAL_SCO_CHIP_FIRMWARE_LEN_REG   (0x80038000)
#define HAL_SCO_CHIP_FIRMWARE_CRC_REG   (0x80038004)
#define HAL_SCO_CHIP_XPHY_CRC_RET_REG   (0x10005018)
#define HAL_SCO_CHIP_XPHY_ERR_RET_REG   (0x1000501c)
#define HAL_SCO_CHIP_MAX_READ_CRC_RETRY (100)

#define HAL_SCO_CHIP_EXTERNAL_PHY_BOOT_SUCCESS (0x0)
#define HAL_SCO_CHIP_EXTERNAL_PHY_NOT_EXIST    (0x1)
#define HAL_SCO_CHIP_EXTERNAL_PHY_BOOT_FAIL    (0x2)
#endif
#endif

#define HAL_SCO_CHIP_MDIO_DISABLE          (0)
#define HAL_SCO_CHIP_MDIO_ENABLE           (1)
#define HAL_SCO_CHIP_CO_CLOCK_MODE_ENABLE  (1)
#define HAL_SCO_CHIP_HWRST_DEGLITCH_REG    (0x100000d4)
#define HAL_SCO_CHIP_HWRST_DEGLITCH_ENABLE (0x7)
/* MACRO FUNCTION DECLARATIONS
 */
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
#define WRITE_EN8853_RG(__rg__, __val__)                                  \
    do                                                                    \
    {                                                                     \
        UI32_T data = __val__;                                            \
        rv = aml_writeReg(unit, __rg__, &data, sizeof(data));             \
        if (rv != AIR_E_OK)                                               \
        {                                                                 \
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] en8853 %d bring up fail \n"); \
            return rv;                                                    \
        }                                                                 \
    } while (0)
#endif

#define HAL_SCO_CHIP_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                   \
    {                                                                    \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));            \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));  \
    } while (0)

#define HAL_SCO_CHIP_SET_BMU_CFG(__unit__, __cfg__)                                                             \
    do                                                                                                          \
    {                                                                                                           \
        UI32_T __u32dat;                                                                                        \
        aml_readReg(__unit__, SCO_GFCCR0, &__u32dat, sizeof(__u32dat));                                         \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[0], SCO_GFCCR0_FREE_BLK_HI_OFFT, SCO_GFCCR0_FREE_BLK_HI_LENG); \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[1], SCO_GFCCR0_FREE_BLK_LO_OFFT, SCO_GFCCR0_FREE_BLK_LO_LENG); \
        aml_writeReg(__unit__, SCO_GFCCR0, &__u32dat, sizeof(__u32dat));                                        \
        aml_readReg(__unit__, SCO_GFCCR1, &__u32dat, sizeof(__u32dat));                                         \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[2], SCO_GFCCR1_PORT_BLK_HI_OFFT, SCO_GFCCR1_PORT_BLK_HI_LENG); \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[3], SCO_GFCCR1_PORT_BLK_LO_OFFT, SCO_GFCCR1_PORT_BLK_LO_LENG); \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[4], SCO_GFCCR1_QUE_BLK_HI_OFFT, SCO_GFCCR1_QUE_BLK_HI_LENG);   \
        aml_writeReg(__unit__, SCO_GFCCR1, &__u32dat, sizeof(__u32dat));                                        \
        aml_readReg(__unit__, SCO_GFCCR2, &__u32dat, sizeof(__u32dat));                                         \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR2_QUE_BLK_Q7_OFFT, SCO_GFCCR2_QUE_BLK_Q7_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR2_QUE_BLK_Q6_OFFT, SCO_GFCCR2_QUE_BLK_Q6_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR2_QUE_BLK_Q5_OFFT, SCO_GFCCR2_QUE_BLK_Q5_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR2_QUE_BLK_Q4_OFFT, SCO_GFCCR2_QUE_BLK_Q4_LENG);   \
        aml_writeReg(__unit__, SCO_GFCCR2, &__u32dat, sizeof(__u32dat));                                        \
        aml_readReg(__unit__, SCO_GFCCR3, &__u32dat, sizeof(__u32dat));                                         \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR3_QUE_BLK_Q3_OFFT, SCO_GFCCR3_QUE_BLK_Q3_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR3_QUE_BLK_Q2_OFFT, SCO_GFCCR3_QUE_BLK_Q2_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR3_QUE_BLK_Q1_OFFT, SCO_GFCCR3_QUE_BLK_Q1_LENG);   \
        HAL_SCO_CHIP_SET_VALUE(__u32dat, __cfg__[5], SCO_GFCCR3_QUE_BLK_Q0_OFFT, SCO_GFCCR3_QUE_BLK_Q0_LENG);   \
        aml_writeReg(__unit__, SCO_GFCCR3, &__u32dat, sizeof(__u32dat));                                        \
    } while (0)

/* DATA TYPE DECLARATIONS
 */
typedef struct EFUSE_DATA_S
{
    UI32_T data[4];
} EFUSE_DATA_T;

#define COL_TO_BYTE(col)  (col << 4)
#define WORD_TO_COL(word) (word >> 2)
#define EFUSE_MAX_DELAY   (0x10000000) /* (1073741823 * 6) / 900M ~= 1.78s */
#define EFUSE_KICK_SHIFT  (30)

#define EFUSE_ADDR_MASK  (0x1FF)
#define EFUSE_ADDR_SHIFT (16)
#define EFUSE_MODE_MASK  (0x3)
#define EFUSE_MODE_SHIFT (6)
#define EFUSE_MODE_READ  (0x1)

#define EFUSE_REMARK_DEVICE_ID_OFFT (16)
#define EFUSE_REMARK_DEVICE_ID_LENG (8)
#define EFUSE_DEVICE_ID_OFFT        (8)
#define EFUSE_DEVICE_ID_LENG        (8)
#define EFUSE_REVISION_ID_OFFT      (0)
#define EFUSE_REVISION_ID_LENG      (3)

#define RESET_CONTROL_REG_2 (0x100050C4)
#define HSGMII_HSI0_RESET   (0x2000)
#define HSGMII_HSI1_RESET   (0x4000)
#define HSGMII_HSI2_RESET   (0x8000)
#define HSGMII_HSI3_RESET   (0x10000)
#define HSGMII_HSI4_RESET   (0x20000)

#define GLOBAL_FC_PORT_BLK_HI (0x84)

/* GLOBAL VARIABLE DECLARATIONS
 */
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
extern const UI32_T hal_sco_fw_len;
extern const UI8_T  hal_sco_fw[];
#endif

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_CHIP, "hal_sco_chip.c");
static const UI32_T hal_sco_chip_bmu_cfg_0[6] = {632, 392, 132, 30, 30, 15}; /* default waterline */
static const UI32_T hal_sco_chip_bmu_cfg_1[6] = {632, 392, 132, 8, 8, 5};    /* 24x1G + 4xCombo + CPU */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static AIR_ERROR_NO_T
_hal_chip_efuse_check_busy(
    const UI32_T unit)
{
    UI32_T delay = EFUSE_MAX_DELAY;
    UI32_T efuse_ctrl;

    do
    {
        delay--;
        aml_readReg(unit, EFUSE_CTRL, &efuse_ctrl, sizeof(efuse_ctrl));
    } while ((efuse_ctrl & (1 << EFUSE_KICK_SHIFT)) && delay);

    if (!delay)
    {
        DIAG_PRINT(HAL_DBG_ERR, "efuse_read_data timeout \n");
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_chip_efuse_read_data(
    const UI32_T  unit,
    const UI32_T  byte_offset,
    EFUSE_DATA_T *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         efuse_ctrl = 0;

    rc = aml_readReg(unit, EFUSE_CTRL, &efuse_ctrl, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    if (efuse_ctrl & (1 << EFUSE_KICK_SHIFT))
    {
        return AIR_E_OP_INVALID;
    }

    efuse_ctrl &= ~((EFUSE_ADDR_MASK << EFUSE_ADDR_SHIFT) | (EFUSE_MODE_MASK << EFUSE_MODE_SHIFT));
    efuse_ctrl |= ((EFUSE_MODE_READ << EFUSE_MODE_SHIFT) | ((byte_offset & EFUSE_ADDR_MASK) << EFUSE_ADDR_SHIFT) |
                   (1 << EFUSE_KICK_SHIFT));

    rc = aml_writeReg(unit, EFUSE_CTRL, &efuse_ctrl, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = _hal_chip_efuse_check_busy(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    aml_readReg(unit, EFUSE_RDATA0, &ptr_data->data[0], sizeof(UI32_T));
    aml_readReg(unit, EFUSE_RDATA1, &ptr_data->data[1], sizeof(UI32_T));
    aml_readReg(unit, EFUSE_RDATA2, &ptr_data->data[2], sizeof(UI32_T));
    aml_readReg(unit, EFUSE_RDATA3, &ptr_data->data[3], sizeof(UI32_T));

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_chip_efuse_read_word(
    const UI32_T unit,
    const UI32_T word_offset,
    UI32_T      *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    EFUSE_DATA_T   efuse_data;
    osal_memset(&efuse_data, 0, sizeof(EFUSE_DATA_T));

    rc = _hal_chip_efuse_read_data(unit, COL_TO_BYTE(WORD_TO_COL(word_offset)), &efuse_data);
    DIAG_PRINT(HAL_DBG_INFO, "word %d: %x \n", word_offset, efuse_data.data[word_offset % 4]);

    (*ptr_data) = efuse_data.data[word_offset % 4];

    return rc;
}

#ifdef AIR_EN_CHIP_INIT_RESET
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
static AIR_ERROR_NO_T
_hal_sco_chip_speed_up_firmware_download(
    const UI32_T unit)
{
    UI8_T          wLoop = 0;
    UI32_T        *ptrAddr;
    UI32_T         idx = 0;
    UI32_T         crc_result = 0, data = 0, TargetAddr = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    for (idx = 0; idx < hal_sco_fw_len; idx++)
    {
        crc_result += hal_sco_fw[idx];
    }

    /* EN8853 bring up */
    WRITE_EN8853_RG(HAL_SCO_CHIP_TIMER_CONTROL_REG, HAL_SCO_CHIP_WDT_DISABLE);
    WRITE_EN8853_RG(HAL_SCO_CHIP_CPU_CLOCK_GATING_REG, HAL_SCO_CHIP_CPU_CLOCK_DISABLE);
    WRITE_EN8853_RG(HAL_SCO_CHIP_FIRMWARE_LEN_REG, hal_sco_fw_len);
    WRITE_EN8853_RG(HAL_SCO_CHIP_FIRMWARE_CRC_REG, crc_result);
    WRITE_EN8853_RG(HAL_SCO_CHIP_XPHY_CRC_RET_REG, 0);
    WRITE_EN8853_RG(HAL_SCO_CHIP_XPHY_ERR_RET_REG, 0);

    TargetAddr = HAL_SCO_CHIP_DMEM_ADDRESS;
    for (idx = 0; idx <= hal_sco_fw_len; idx += 4)
    {
        ptrAddr = (UI32_T *)&hal_sco_fw[idx];
        rv = aml_writeReg(unit, TargetAddr, ptrAddr, sizeof(UI32_T));
        TargetAddr += 4;
    }

    WRITE_EN8853_RG(HAL_SCO_CHIP_CPU_CLOCK_GATING_REG, HAL_SCO_CHIP_CPU_CLOCK_ENABLE);

    for (wLoop = 0; wLoop < HAL_SCO_CHIP_MAX_READ_CRC_RETRY; wLoop++)
    {
        osal_delayUs(100000);
        rv = aml_readReg(unit, HAL_SCO_CHIP_XPHY_CRC_RET_REG, &data, sizeof(UI32_T));
        if ((data != 0) && (data == crc_result))
        {
            break;
        }
    }

    WRITE_EN8853_RG(HAL_SCO_CHIP_CPU_CLOCK_GATING_REG, HAL_SCO_CHIP_CPU_CLOCK_DISABLE);

    if (HAL_SCO_CHIP_MAX_READ_CRC_RETRY <= wLoop)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg] chip %d fw download fail!\n");
        return AIR_E_OP_INCOMPLETE;
    }

    rv = aml_readReg(unit, HAL_SCO_CHIP_XPHY_ERR_RET_REG, &data, sizeof(UI32_T));
    if (AIR_E_OK == rv)
    {
        switch (data)
        {
            case HAL_SCO_CHIP_EXTERNAL_PHY_BOOT_SUCCESS:
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg] external phy boot success!\n");
                break;
            case HAL_SCO_CHIP_EXTERNAL_PHY_NOT_EXIST:
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg] external phy do not exist!\n");
                break;
            case HAL_SCO_CHIP_EXTERNAL_PHY_BOOT_FAIL:
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg] external phy boot failed!\n");
                rv = AIR_E_OP_INCOMPLETE;
                break;
            default:
                DIAG_PRINT(HAL_DBG_ERR, "[Dbg] external phy fw download failed, data 0x%x!\n", data);
                rv = AIR_E_OTHERS;
                break;
        }
    }
    return rv;
}
#endif
static void
_hal_sco_chip_resetSystem(
    const UI32_T unit)
{
    UI32_T u32dat;
    u32dat = HAL_SCO_CHIP_TIMER_INTERRUPT_EN;
    aml_writeReg(unit, HAL_SCO_CHIP_TIMER_CONTROL_REG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_RESET_TIME;
    aml_writeReg(unit, HAL_SCO_CHIP_TIMER3_LOAD_VALUE_REG, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, HAL_SCO_CHIP_WDOG_THRESHOLD_REG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_WDOG_TIMER_EN;
    aml_writeReg(unit, HAL_SCO_CHIP_TIMER_CONTROL_REG, &u32dat, sizeof(u32dat));
    osal_delayUs(1000000);
}
#endif

static AIR_ERROR_NO_T
_hal_sco_chip_setIoCurrDriving(
    const UI32_T unit)
{
    AIR_CFG_VALUE_T mdio_enable;
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;

    osal_memset(&mdio_enable, 0, sizeof(AIR_CFG_VALUE_T));

    mdio_enable.value = HAL_SCO_CHIP_MDIO_DISABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &mdio_enable);

    /* SMI use 4 MA current driving capability except EN8860C */
    if ((HAL_SCO_DEVICE_ID_EN8860C != HAL_DEVICE_CHIP_ID(unit)) && (HAL_SCO_CHIP_MDIO_ENABLE == mdio_enable.value))
    {
        rc = aml_readReg(unit, RG_GPIO_L_E4_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~GPIO_SMI_E4_BITS;
            rc = aml_writeReg(unit, RG_GPIO_L_E4_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        }
    }

    /* I2C use 4MA current driving capability */
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, RG_I2C_E4_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~I2C_IO_E4_BITS;
            rc = aml_writeReg(unit, RG_I2C_E4_TX_DRIVING_CTRL, &u32dat, sizeof(u32dat));
        }
    }
    return rc;
}

static void
_hal_sco_chip_initBmuCfg(
    const UI32_T unit)
{
    const UI32_T *bmu_cfg = NULL;

    /* Initialize BMU threshold */
    switch (HAL_DEVICE_CHIP_ID(unit))
    {
        case HAL_SCO_DEVICE_ID_EN8853C:
            bmu_cfg = hal_sco_chip_bmu_cfg_1;
            break;
        case HAL_SCO_DEVICE_ID_EN8851C:
        case HAL_SCO_DEVICE_ID_EN8851E:
        case HAL_SCO_DEVICE_ID_EN8860C:
            bmu_cfg = hal_sco_chip_bmu_cfg_0;
            break;
        default:
            break;
    }
    if (NULL != bmu_cfg)
    {
        HAL_SCO_CHIP_SET_BMU_CFG(unit, bmu_cfg);
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_sco_chip_readDeviceInfo
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
hal_sco_chip_readDeviceInfo(
    const UI32_T unit,
    UI32_T      *ptr_device_id,
    UI32_T      *ptr_revision_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         efuse_data = 0;
    UI32_T         remark_device_id = 0;

    HAL_CHECK_PTR(ptr_device_id);
    HAL_CHECK_PTR(ptr_revision_id);

    rc = _hal_chip_efuse_read_word(unit, 0, &efuse_data);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "unit=%u, get efuse failed, rc=%d\n", unit, rc);
        return rc;
    }

    remark_device_id = BITS_OFF_R(efuse_data, EFUSE_REMARK_DEVICE_ID_OFFT, EFUSE_REMARK_DEVICE_ID_LENG);
    if (0 == remark_device_id)
    {
        (*ptr_device_id) = BITS_OFF_R(efuse_data, EFUSE_DEVICE_ID_OFFT, EFUSE_DEVICE_ID_LENG);
        (*ptr_revision_id) = BITS_OFF_R(efuse_data, EFUSE_REVISION_ID_OFFT, EFUSE_REVISION_ID_LENG);
    }
    else
    {
        (*ptr_device_id) = remark_device_id;
        (*ptr_revision_id) = BITS_OFF_R(efuse_data, EFUSE_REVISION_ID_OFFT, EFUSE_REVISION_ID_LENG);
    }
    DIAG_PRINT(HAL_DBG_INFO, "unit=%u, device_id=%d revision_id=%d\n", unit, (*ptr_device_id), (*ptr_revision_id));

    return rc;
}

/* FUNCTION NAME: hal_sco_chip_init
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
hal_sco_chip_init(
    const UI32_T           unit,
    HAL_CHIP_INIT_PARAM_T *ptr_chip_init_param)
{
    AIR_CFG_VALUE_T mdio_clock, mdio_enable;
    AIR_CFG_VALUE_T sled;
    AIR_CFG_VALUE_T led_type;
    AIR_CFG_VALUE_T cko_cfg;
    UI32_T          u32dat = 0;
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    HAL_CHECK_PTR(ptr_chip_init_param);

    osal_memset(&mdio_clock, 0, sizeof(AIR_CFG_VALUE_T));
    osal_memset(&mdio_enable, 0, sizeof(AIR_CFG_VALUE_T));
    osal_memset(&sled, 0, sizeof(AIR_CFG_VALUE_T));
    osal_memset(&led_type, 0, sizeof(AIR_CFG_VALUE_T));

#ifdef AIR_EN_CHIP_INIT_RESET
    _hal_sco_chip_resetSystem(unit);
#ifdef AIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
    if ((HAL_DEVICE_CHIP_ID(unit) == HAL_SCO_DEVICE_ID_EN8853C) ||
        (HAL_DEVICE_CHIP_ID(unit) == HAL_SCO_DEVICE_ID_EN8851C))
    {
        rc = _hal_sco_chip_speed_up_firmware_download(unit);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg] chip %d speed-up phy fw download fail, rc=%d\n", HAL_DEVICE_CHIP_ID(unit),
                       rc);
        }
    }
    else
#endif
    {
        u32dat = HAL_SCO_CHIP_CPU_CLOCK_ENABLE;
        aml_writeReg(unit, HAL_SCO_CHIP_CPU_CLOCK_GATING_REG, &u32dat, sizeof(u32dat));
        osal_delayUs(1000000);
        u32dat = HAL_SCO_CHIP_CPU_CLOCK_DISABLE;
        aml_writeReg(unit, HAL_SCO_CHIP_CPU_CLOCK_GATING_REG, &u32dat, sizeof(u32dat));
    }
#endif

    /*
     * Disable MAC power saving
     * Note: Acl udf access clock will stop when all port link down.
     */
    aml_readReg(unit, CKGCR, &u32dat, sizeof(u32dat));
    u32dat &= ~CKG_LNKDN_GLB_STOP;
    aml_writeReg(unit, CKGCR, &u32dat, sizeof(u32dat));

    mdio_enable.value = HAL_SCO_CHIP_MDIO_DISABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_ENABLE, &mdio_enable);

    /*if mdio external bus is enable, then set gpio4 as mdc, gpio5 as mdio*/
    if (HAL_SCO_CHIP_MDIO_ENABLE == mdio_enable.value)
    {
        aml_readReg(unit, REG_IOMUX_CONTROL2, &u32dat, sizeof(u32dat));

        if (HAL_SCO_CHIP_MDIO_DISABLE == (u32dat & IOMUX_CONTROL2_P_MDIO_MODE_MASK))
        {
            u32dat = (u32dat & ~(IOMUX_CONTROL2_P_MDIO_MODE_MASK)) |
                     ((mdio_enable.value & IOMUX_CONTROL2_P_MDIO_MODE_RELMASK) << IOMUX_CONTROL2_P_MDIO_MODE_OFFT);
            aml_writeReg(unit, REG_IOMUX_CONTROL2, &u32dat, sizeof(u32dat));
        }
    }
    /* setup mdio clock */
    mdio_clock.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MDIO_CLOCK, &mdio_clock);
    aml_readReg(unit, PHY_SMI, &u32dat, sizeof(u32dat));
    u32dat = (u32dat & ~(CSR_SMI_PMDC_MASK)) | ((mdio_clock.value & 0x03) << CSR_SMI_PMDC_OFFT);
    aml_writeReg(unit, PHY_SMI, &u32dat, sizeof(u32dat));

    /* setup serial LED */
    sled.value = 2;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &sled);
    u32dat = (sled.value << 1) + SLED_RISING_EDGE + SLED_OUTPUT_TRANSITION_MODE;
    aml_writeReg(unit, SLED_CTRL0, &u32dat, sizeof(u32dat));

    /* setup LED for serial or parallel type */
    led_type.value = 0;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_TYPE, &led_type);
    if (0 == led_type.value)
    {
        /* enable serial LED */
        u32dat = 0x0;
        aml_writeReg(unit, RG_GPIO_INVERSE, &u32dat, sizeof(u32dat));
        u32dat = 0x0;
        aml_writeReg(unit, RG_LAN_LED_IOMUX, &u32dat, sizeof(u32dat));
    }
    else
    {
        /* enable parallel LED */
        aml_readReg(unit, RG_GPIO_EN_REG, &u32dat, sizeof(u32dat));
        u32dat &= ~(PARALLEL_LED_PORT);
        aml_writeReg(unit, RG_GPIO_EN_REG, &u32dat, sizeof(u32dat));
        u32dat = PARALLEL_LED_PORT;
        aml_writeReg(unit, RG_GPIO_INVERSE, &u32dat, sizeof(u32dat));
        u32dat = PARALLEL_LED_MODE;
        aml_writeReg(unit, RG_LAN_LED_IOMUX, &u32dat, sizeof(u32dat));
        /* for parallel Led flash mode config */
        u32dat = PARALLEL_LED_PORT;
        aml_writeReg(unit, RG_GPIO_FLASH_MODE_CFG, &u32dat, sizeof(u32dat));
        /* direction output */
        aml_readReg(unit, REG_GPIOCTRL, &u32dat, sizeof(u32dat));
        u32dat = (u32dat & 0xfc0cff00) | 0x1510155;
        aml_writeReg(unit, REG_GPIOCTRL, &u32dat, sizeof(u32dat));
        /* output enable */
        aml_readReg(unit, REG_GPIOOE, &u32dat, sizeof(u32dat));
        u32dat |= PARALLEL_LED_PORT;
        aml_writeReg(unit, REG_GPIOOE, &u32dat, sizeof(u32dat));
    }

    osal_memset(&cko_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    cko_cfg.value = HAL_SCO_CHIP_CO_CLOCK_MODE_ENABLE;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_CHIP_CO_CLOCK_ENABLE, &cko_cfg);
    if (HAL_SCO_CHIP_CO_CLOCK_MODE_ENABLE != cko_cfg.value)
    {
        /* Disabled co-clock pin output */
        aml_readReg(unit, REG_BGPOR_CTRL1, &u32dat, sizeof(u32dat));
        u32dat |= BIT(16);
        aml_writeReg(unit, REG_BGPOR_CTRL1, &u32dat, sizeof(u32dat));
    }

    /* Power down all serdes */
    aml_readReg(unit, RESET_CONTROL_REG_2, &u32dat, sizeof(u32dat));
    u32dat |= (HSGMII_HSI0_RESET | HSGMII_HSI1_RESET | HSGMII_HSI2_RESET | HSGMII_HSI3_RESET | HSGMII_HSI4_RESET);
    aml_writeReg(unit, RESET_CONTROL_REG_2, &u32dat, sizeof(u32dat));

    /* Config Global Flow Control Threshold */
    _hal_sco_chip_initBmuCfg(unit);

    /* Enable HW reset de-glitch */
    u32dat = HAL_SCO_CHIP_HWRST_DEGLITCH_ENABLE;
    aml_writeReg(unit, HAL_SCO_CHIP_HWRST_DEGLITCH_REG, &u32dat, sizeof(u32dat));

    rc = _hal_sco_chip_setIoCurrDriving(unit);
    return rc;
}

/* FUNCTION NAME: hal_sco_chip_deinit
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
hal_sco_chip_deinit(
    const UI32_T unit)
{
    UI32_T u32dat;

    u32dat = HAL_SCO_CHIP_RESET_CONTROL_REG_2_DEFAULT_VALUE;
    aml_writeReg(unit, RESET_CONTROL_REG_2, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_RG_LAN_LED_IOMUX_DEFAULT_VALUE;
    aml_writeReg(unit, RG_LAN_LED_IOMUX, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_RG_GPIO_INVERSE_DEFAULT_VALUE;
    aml_writeReg(unit, RG_GPIO_INVERSE, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_RG_GPIO_EN_REG_DEFAULT_VALUE;
    aml_writeReg(unit, RG_GPIO_EN_REG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_SLED_CTRL0_DEFAULT_VALUE;
    aml_writeReg(unit, SLED_CTRL0, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_PHY_SMI_DEFAULT_VALUE;
    aml_writeReg(unit, PHY_SMI, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_CKGCR_DEFAULT_VALUE;
    aml_writeReg(unit, CKGCR, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_CHIP_EFUSE_CTRL_DEFAULT_VALUE;
    aml_writeReg(unit, EFUSE_CTRL, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_chip_readEfuse
 * PURPOSE:
 *      This API is used to read chip efuse data
 * INPUT:
 *      unit                 -- Device unit number
 *      word_offset          -- Efuse data word offset
 * OUTPUT:
 *      ptr_data             -- Efuse data
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 *      AIR_E_TIMEOUT        -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_chip_readEfuse(
    const UI32_T unit,
    const UI32_T word_offset,
    UI32_T      *ptr_data)
{
    return _hal_chip_efuse_read_word(unit, word_offset, ptr_data);
}
