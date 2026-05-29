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

/* FILE NAME:  hal_pearl_perif.c
 * PURPOSE:
 *    It provides periferal module API.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/pearl/hal_pearl_perif.h>

#include <aml/aml.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/pearl/hal_pearl_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_PEARL_PERIF_SET_REG(__VAL__, __DATA__, __MASK__, __PIN__)                                     \
    (((__VAL__) & (~((__MASK__) << (__PIN__)))) | (((__DATA__) << (__PIN__)) & ((__MASK__) << (__PIN__))))

#define HAL_PEARL_PERIF_GPIO_FLASH_PATT_HALF_HZ (0x00)
#define HAL_PEARL_PERIF_GPIO_FLASH_PATT_ONE_HZ  (0x01)
#define HAL_PEARL_PERIF_GPIO_FLASH_PATT_TWO_HZ  (0x02)
#define HAL_PEARL_PERIF_GPIO_FLASH_PATT_TRAFFIC (0x03)

#define HAL_PEARL_PERIF_GPIO_DEFAULT_WAVE_CYCLE (0x1fffffff) /* for 250Hz cycle */
#define HAL_PEARL_PERIF_GPIO_DEFAULT_PERIOD_01  (0x7d7dfafa)
#define HAL_PEARL_PERIF_GPIO_DEFAULT_PERIOD_23  (0x7d7d3e3e)
#define HAL_PEARL_PERIF_GPIO_DEFAULT_MAP        (0x99999999)
#define HAL_PEARL_PERIF_GPIO_IOMUX_ENABLED      (1)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_IFMON, "hal_peral_perif.c");
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static AIR_ERROR_NO_T
_hal_pearl_perif_initGpio(const UI32_T unit);

static AIR_ERROR_NO_T
_hal_pearl_perif_getReg(const UI32_T unit, const UI32_T reg, const UI32_T mask, const UI32_T pin, UI32_T *ptr_out);

static AIR_ERROR_NO_T
_hal_pearl_perif_writeMask(const UI32_T unit, const UI32_T reg, const UI32_T data, const UI32_T mask, const UI32_T pin);

static AIR_ERROR_NO_T
_hal_pearl_perif_checkGpioPin(const UI32_T unit, const UI32_T pin);

static AIR_ERROR_NO_T
_hal_pearl_perif_checkGpioDirection(const UI32_T unit, const UI32_T pin, const UI32_T expect_direction);

static AIR_ERROR_NO_T
_hal_pearl_perif_initGpioAutoMode(const UI32_T unit);
/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_hal_pearl_perif_initGpio(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T perif_gpio_cfg;
    UI32_T          i = 0, reg_value = 0, inverse_value = 0;

    osal_memset(&perif_gpio_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    rc = aml_readReg(unit, PEARL_RG_GPIO_EN_REG, &reg_value, sizeof(UI32_T));
    rc |= aml_readReg(unit, PEARL_RG_GPIO_INVERSE, &inverse_value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        for (i = 2; i < HAL_PEARL_PERIF_GPIO_PIN_COUNT; i++) /* skip UART pins (GPIO 0,1) */
        {
            if (9 == i)
            {
                i = 13; /* skip SPI pins (GPIO 9~12) */
            }
            perif_gpio_cfg.param0 = i;
            perif_gpio_cfg.param1 = 0;
            perif_gpio_cfg.value = (reg_value & (1 << i)) ? 1 : 0;

            rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN, &perif_gpio_cfg);
            if (AIR_E_OK == rc)
            {
                if (0 == perif_gpio_cfg.value)
                {
                    /* clear the bit */
                    reg_value &= ~(1 << i);
                }
                else
                {
                    /* set the bit */
                    reg_value |= (1 << i);
                }
            }
            perif_gpio_cfg.value = 0;
            rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_FORCE_GPIO_LEVEL_INVERSE, &perif_gpio_cfg);
            if (AIR_E_OK == rc)
            {
                if (1 == perif_gpio_cfg.value)
                {
                    /* set the bit */
                    inverse_value |= (1 << i);
                }
                else
                {
                    /* clear the bit */
                    inverse_value &= ~(1 << i);
                }
            }
        }
        /* Force GPIO */
        rc = aml_writeReg(unit, PEARL_RG_GPIO_EN_REG, &reg_value, sizeof(UI32_T));
        /* Set GPIO to inverse mode */
        rc |= aml_writeReg(unit, PEARL_RG_GPIO_INVERSE, &inverse_value, sizeof(UI32_T));
        if (AIR_E_OK == rc)
        {
            /*
             * Config wave generator
             *
             * wave 0: 0.5 Hz
             * wave 1: 1 Hz
             * wave 2: 2 Hz
             * wave 3: 8 Hz
             */
            reg_value = HAL_PEARL_PERIF_GPIO_DEFAULT_WAVE_CYCLE;
            rc = aml_writeReg(unit, PEARL_RG_CYCLE_CFG_VALUE0, &reg_value, sizeof(UI32_T));
            reg_value = HAL_PEARL_PERIF_GPIO_DEFAULT_PERIOD_01;
            rc |= aml_writeReg(unit, PEARL_RG_GPIO_FLASH_PRD_SET0, &reg_value, sizeof(UI32_T));
            reg_value = HAL_PEARL_PERIF_GPIO_DEFAULT_PERIOD_23;
            rc |= aml_writeReg(unit, PEARL_RG_GPIO_FLASH_PRD_SET1, &reg_value, sizeof(UI32_T));
            reg_value = HAL_PEARL_PERIF_GPIO_DEFAULT_MAP;
            rc |= aml_writeReg(unit, PEARL_RG_GPIO_FLASH_MAP_CFG0, &reg_value, sizeof(UI32_T));
            rc |= aml_writeReg(unit, PEARL_RG_GPIO_FLASH_MAP_CFG1, &reg_value, sizeof(UI32_T));
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_pearl_perif_getReg(
    const UI32_T unit,
    const UI32_T reg,
    const UI32_T mask,
    const UI32_T pin,
    UI32_T      *ptr_out)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;

    rc = aml_readReg(unit, reg, &val, sizeof(UI32_T));

    if (AIR_E_OK == rc)
    {
        *ptr_out = ((val & (mask << pin)) >> pin);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_pearl_perif_writeMask(
    const UI32_T unit,
    const UI32_T reg,
    const UI32_T data,
    const UI32_T mask,
    const UI32_T pin)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    rc = aml_readReg(unit, reg, &val, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        val = _HAL_PEARL_PERIF_SET_REG(val, data, mask, pin);
        rc = aml_writeReg(unit, reg, &val, sizeof(UI32_T));
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_pearl_perif_checkGpioPin(
    const UI32_T unit,
    const UI32_T pin)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         out = 0;
    rc = _hal_pearl_perif_getReg(unit, PEARL_RG_GPIO_EN_REG, HAL_PEARL_PERIF_BIT_MASK, pin, &out);
    if (out != HAL_PEARL_PERIF_GPIO_IOMUX_ENABLED)
    {
        rc = AIR_E_NOT_INITED;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_pearl_perif_checkGpioDirection(
    const UI32_T unit,
    const UI32_T pin,
    const UI32_T expect_direction)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         direction = AIR_PERIF_GPIO_DIRECTION_INPUT;

    rc = hal_pearl_perif_getGpioDirection(unit, pin, &direction);
    if (AIR_E_OK == rc)
    {
        if (expect_direction != direction)
        {
            rc = AIR_E_OP_INVALID;
        }
        else
        {
            rc = AIR_E_OK;
        }
    }
    return rc;
}

/* GPIO data relevant */
static AIR_ERROR_NO_T
_hal_pearl_perif_setGpioData(
    const UI32_T unit,
    const UI32_T pin,
    const UI32_T data)
{
    return _hal_pearl_perif_writeMask(unit, PEARL_REG_GPIODATA, data, HAL_PEARL_PERIF_BIT_MASK, pin);
}

static AIR_ERROR_NO_T
_hal_pearl_perif_getGpioData(
    const UI32_T unit,
    const UI32_T pin,
    UI32_T      *ptr_data)
{
    return _hal_pearl_perif_getReg(unit, PEARL_REG_GPIODATA, HAL_PEARL_PERIF_BIT_MASK, pin, ptr_data);
}

/* Output enable relevant */
static AIR_ERROR_NO_T
_hal_pearl_perif_setGpioOutputEnable(
    const UI32_T unit,
    const UI32_T pin,
    const UI32_T status)
{
    return _hal_pearl_perif_writeMask(unit, PEARL_REG_GPIOOE, status, HAL_PEARL_PERIF_BIT_MASK, pin);
}

static AIR_ERROR_NO_T
_hal_pearl_perif_getOutputEnable(
    const UI32_T unit,
    const UI32_T pin,
    UI32_T      *ptr_output_enable)
{
    return _hal_pearl_perif_getReg(unit, PEARL_REG_GPIOOE, HAL_PEARL_PERIF_BIT_MASK, pin, ptr_output_enable);
}

/* Flash Mode relevant */
static AIR_ERROR_NO_T
_hal_pearl_perif_setGpioFlashMode(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable)
{
    return _hal_pearl_perif_writeMask(unit, PEARL_RG_GPIO_FLASH_MODE_CFG, enable, HAL_PEARL_PERIF_BIT_MASK, pin);
}

static AIR_ERROR_NO_T
_hal_pearl_perif_getGpioFlashMode(
    const UI32_T unit,
    const UI32_T pin,
    BOOL_T      *ptr_enable)
{
    return _hal_pearl_perif_getReg(unit, PEARL_RG_GPIO_FLASH_MODE_CFG, HAL_PEARL_PERIF_BIT_MASK, pin,
                                   (UI32_T *)ptr_enable);
}

static AIR_ERROR_NO_T
_hal_pearl_perif_initGpioAutoMode(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T cfg_value;
    UI32_T          p = 0;

    for (p = 0; p < HAL_PEARL_PERIF_GPIO_PIN_FLASH_COUNT; p++)
    {
        cfg_value.param0 = p;
        cfg_value.param1 = 0;
        cfg_value.value = 0xffff;

        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_GPIO_AUTO_MODE, &cfg_value);
        if (AIR_E_OK == rc)
        {
            if (0xffff != cfg_value.value)
            {
                rc = hal_pearl_perif_setGpioOutputAutoMode(unit, p, TRUE);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "[Dbg] gpio-%d, set auto mode fail, rc=%d\n", p, rc);
                    return rc;
                }
            }
        }
        else
        {
            return rc;
        }

        cfg_value.param0 = p;
        cfg_value.param1 = 0;
        cfg_value.value = 0xffff;

        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PERIF_GPIO_AUTO_PATT, &cfg_value);
        if (AIR_E_OK == rc)
        {
            if (0xffff != cfg_value.value)
            {
                rc = hal_pearl_perif_setGpioOutputAutoPatt(unit, p, cfg_value.value);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "[Dbg] gpio-%d, set auto pattern fail, rc=%d\n", p, rc);
                    return rc;
                }
            }
        }
        else
        {
            return rc;
        }
    }
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_pearl_perif_init
 * PURPOSE:
 *      Initialize peripheral module.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _hal_pearl_perif_initGpio(unit);
    if (AIR_E_OK == rc)
    {
        rc = _hal_pearl_perif_initGpioAutoMode(unit);
    }
    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_deinit
 * PURPOSE:
 *      Deinitialize peripheral module.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_deinit(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_perif_setGpioDirection
 * PURPOSE:
 *      This API is used to set gpio pin direction
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      direction            -- Gpio direction
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_setGpioDirection(
    const UI32_T                     unit,
    const UI32_T                     pin,
    const AIR_PERIF_GPIO_DIRECTION_T direction)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg = 0, index = 0;

    /* Check valid pin */
    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_COUNT), AIR_E_OP_INVALID);

    /* Check the gpio iomux */
    rc = _hal_pearl_perif_checkGpioPin(unit, pin);
    if (AIR_E_OK == rc)
    {
        /* Before we change the direction, we should close output enable */
        rc = _hal_pearl_perif_setGpioOutputEnable(unit, pin, 0);
        if (AIR_E_OK == rc)
        {
            if (HAL_PEARL_PERIF_GPIO_PIN_BOUNDARY > pin)
            {
                /* gpio pin is 0 ~ 15 */
                reg = PEARL_REG_GPIOCTRL;
                index = pin;
            }
            else
            {
                /* gpio pin is 16 ~ 31 */
                reg = PEARL_REG_GPIOCTRL1;
                index = pin - 16;
            }
            rc = _hal_pearl_perif_writeMask(unit, reg, direction, HAL_PEARL_PERIF_TWO_BIT_MASK, (index * 2));
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_getGpioDirection
 * PURPOSE:
 *      This API is used to get gpio pin direction
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_direction        -- Pointer of gpio direction
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_getGpioDirection(
    const UI32_T                unit,
    const UI32_T                pin,
    AIR_PERIF_GPIO_DIRECTION_T *ptr_direction)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg = 0, index = 0;

    /* Check valid pin */
    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_COUNT), AIR_E_OP_INVALID);

    rc = _hal_pearl_perif_checkGpioPin(unit, pin);
    if (AIR_E_OK == rc)
    {
        if (HAL_PEARL_PERIF_GPIO_PIN_BOUNDARY > pin)
        {
            /* gpio pin is 0 ~ 15 */
            reg = PEARL_REG_GPIOCTRL;
            index = pin;
        }
        else
        {
            /* gpio pin is 16 ~ 31 */
            reg = PEARL_REG_GPIOCTRL1;
            index = pin - 16;
        }

        rc = _hal_pearl_perif_getReg(unit, reg, HAL_PEARL_PERIF_TWO_BIT_MASK, (index * 2), ptr_direction);
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_setGpioOutputData
 * PURPOSE:
 *      This API is used to set gpio pin output value
 *      to data register
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      data                 -- High or low
 *                              AIR_PERIF_GPIO_DATA_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_setGpioOutputData(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_DATA_T data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         output_enable = 0;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_COUNT), AIR_E_OP_INVALID);

    /* Step 1, check the pin is gpio mode, and the direction is output mode. */
    rc = _hal_pearl_perif_checkGpioPin(unit, pin);
    if (AIR_E_OK == rc)
    {
        rc = _hal_pearl_perif_checkGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
        if (AIR_E_OK == rc)
        {
            /* Step 2, write the data to the data register. */
            rc = _hal_pearl_perif_setGpioData(unit, pin, data);
            if (AIR_E_OK == rc)
            {
                /* Step 3, check the output status is output enable,
                           if not, open output enable to push the data out. */
                rc = _hal_pearl_perif_getOutputEnable(unit, pin, &output_enable);
                if (AIR_E_OK == rc)
                {
                    if (0 == output_enable)
                    {
                        rc = _hal_pearl_perif_setGpioOutputEnable(unit, pin, 1);
                    }
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_getGpioInputData
 * PURPOSE:
 *      This API is used to get gpio pin input value
 *      from data register
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_data             -- Pointer of gpio data
 *                              AIR_PERIF_GPIO_DATA_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_getGpioInputData(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_DATA_T *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_COUNT), AIR_E_OP_INVALID);

    /* Step 1, check the pin is gpio mode, and the direction is input mode. */
    rc = _hal_pearl_perif_checkGpioPin(unit, pin);
    if (AIR_E_OK == rc)
    {
        rc = _hal_pearl_perif_checkGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_INPUT);
        if (AIR_E_OK == rc)
        {
            /* Step 2, get the data register value. */
            rc = _hal_pearl_perif_getGpioData(unit, pin, ptr_data);
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_setGpioOutputAutoMode
 * PURPOSE:
 *      This API is used to set auto mode
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      enable               -- TRUE: enable auto mode
 *                              FALSE: disable auto mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_setGpioOutputAutoMode(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         output_enable = 0;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_FLASH_COUNT), AIR_E_OP_INVALID);

    /* Step 1, set enable/disable flash mode. */
    rc = _hal_pearl_perif_setGpioFlashMode(unit, pin, enable);
    if (AIR_E_OK == rc)
    {
        /* Step 2, check the output status is output enable,
                    if not, open output enable to push the data out. */
        rc = _hal_pearl_perif_getOutputEnable(unit, pin, &output_enable);
        if (AIR_E_OK == rc)
        {
            if (0 == output_enable)
            {
                rc = _hal_pearl_perif_setGpioOutputEnable(unit, pin, 1);
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_getGpioOutputAutoMode
 * PURPOSE:
 *      This API is used to get auto mode status
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_enable           -- TRUE: enable auto mode
 *                              FALSE: disable auto mode
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_getGpioOutputAutoMode(
    const UI32_T unit,
    const UI32_T pin,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_FLASH_COUNT), AIR_E_OP_INVALID);

    rc = _hal_pearl_perif_getGpioFlashMode(unit, pin, ptr_enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_setGpioOutputAutoPatt
 * PURPOSE:
 *      This API is used to set gpio auto pattern
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      pattern              -- Pattern configuration
 *                              AIR_PERIF_GPIO_PATT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_setGpioOutputAutoPatt(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_PATT_T pattern)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         wave = 0;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_FLASH_COUNT), AIR_E_OP_INVALID);

    if (AIR_PERIF_GPIO_PATT_HZ_EIGHT == pattern)
    {
        wave = HAL_PEARL_PERIF_GPIO_FLASH_PATT_TRAFFIC;
    }
    else if (AIR_PERIF_GPIO_PATT_HZ_HALF == pattern)
    {
        wave = HAL_PEARL_PERIF_GPIO_FLASH_PATT_HALF_HZ;
    }
    else if (AIR_PERIF_GPIO_PATT_HZ_ONE == pattern)
    {
        wave = HAL_PEARL_PERIF_GPIO_FLASH_PATT_ONE_HZ;
    }
    else if (AIR_PERIF_GPIO_PATT_HZ_TWO == pattern)
    {
        wave = HAL_PEARL_PERIF_GPIO_FLASH_PATT_TWO_HZ;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (pin < 8)
    {
        rc = _hal_pearl_perif_writeMask(unit, PEARL_RG_GPIO_FLASH_MAP_CFG0, (wave + 8), 0xf, (pin * 4));
    }
    else
    {
        rc = _hal_pearl_perif_writeMask(unit, PEARL_RG_GPIO_FLASH_MAP_CFG1, (wave + 8), 0xf, ((pin - 8) * 4));
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_perif_getGpioOutputAutoPatt
 * PURPOSE:
 *      This API is used to get gpio auto pattern
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_pattern          -- Pointer of pattern configuration
 *                              AIR_PERIF_GPIO_PATT_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_perif_getGpioOutputAutoPatt(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         wave = 0;

    HAL_CHECK_PARAM(!(pin < HAL_PEARL_PERIF_GPIO_PIN_FLASH_COUNT), AIR_E_OP_INVALID);

    /* read the register according to the pin number */
    if (pin < 8)
    {
        rc = _hal_pearl_perif_getReg(unit, PEARL_RG_GPIO_FLASH_MAP_CFG0, 0x07, (pin * 4), &wave);
    }
    else
    {
        rc = _hal_pearl_perif_getReg(unit, PEARL_RG_GPIO_FLASH_MAP_CFG1, 0x07, ((pin - 8) * 4), &wave);
    }

    if (HAL_PEARL_PERIF_GPIO_FLASH_PATT_TRAFFIC == wave)
    {
        *ptr_pattern = AIR_PERIF_GPIO_PATT_HZ_EIGHT;
    }
    else if (HAL_PEARL_PERIF_GPIO_FLASH_PATT_HALF_HZ == wave)
    {
        *ptr_pattern = AIR_PERIF_GPIO_PATT_HZ_HALF;
    }
    else if (HAL_PEARL_PERIF_GPIO_FLASH_PATT_ONE_HZ == wave)
    {
        *ptr_pattern = AIR_PERIF_GPIO_PATT_HZ_ONE;
    }
    else if (HAL_PEARL_PERIF_GPIO_FLASH_PATT_TWO_HZ == wave)
    {
        *ptr_pattern = AIR_PERIF_GPIO_PATT_HZ_TWO;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    return rc;
}
