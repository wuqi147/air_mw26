/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  mw_sif.c
 * PURPOSE:
 *    This file contains the implementation of setting sif clock functionality for the Magic Wand module.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#if (!defined(AIR_LITE_MW))
#include    "mw_sif.h"
#include    "air_cfg.h"
#include    "osapi.h"
#include    "osapi_string.h"
#include    "aml/aml.h"
#include    "osapi_mutex.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static MW_SIF_CONTEXT_T _mw_sif_contex = {0};

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_air_sif_clock_set(
    const UI32_T unit,
    const UI32_T channel,
    const UI32_T clock)
{
    AIR_ERROR_NO_T  rc;

#if defined(AIR_EN_CORAL)

#define HAL_CORAL_SIF_GET_BASE_ADDRESS(channel) (0x10008000UL)
#define CORAL_REG_GPIO16_MODE                   (0x10006008UL)
#define CORAL_PIN_SCL                           (19UL)
#define CORAL_PINCTRL_FIELD_MSK                 (0xFUL)
#define CORAL_GPIO19_MODE_SCL                   (3UL)
#define CORAL_PIN_SDA                           (20UL)
#define CORAL_GPIO20_MODE_SDA                   (3UL)
#define CORAL_PINCTRL_PINx_OFS(x)               (((x % 8UL) * 4UL))
#define CORAL_AUTOMODE_EN_BIT                   (31UL)
#define CORAL_REG_SIF_AUTO_CTRL                 (0xA0UL)
#define CORAL_AUTOMODE_EN_MSK                   (0x1UL)
#define CORAL_SIF_CLK_DIV_BIT                   (16UL)
#define CORAL_SIF_EN_MSK                        (1UL)
#define CORAL_SIF_EN_BIT                        (1UL)
#define CORAL_SIF_CLK_DIV_SM                    ((0xfffUL) << (CORAL_SIF_CLK_DIV_BIT))
#define CORAL_SIF_EN_SM                         ((CORAL_SIF_EN_MSK) << (CORAL_SIF_EN_BIT))
#define CORAL_SIF_SIFMCTL0                      (0x40UL)


    const UI32_T    sif_clk_spd[] = {0xc7, 0x31, 0x13};
    UI32_T          pin_info = 0;
    UI32_T          data;
    UI32_T          mask;
    UI32_T          reg = 0;
    UI32_T          addr = 0, reg_value;

    /* check sif mst enable */
    aml_readReg(unit, CORAL_REG_GPIO16_MODE, &pin_info, sizeof(UI32_T));
    if (((pin_info >> CORAL_PINCTRL_PINx_OFS(CORAL_PIN_SCL)) & CORAL_PINCTRL_FIELD_MSK)
         != CORAL_GPIO19_MODE_SCL)
    {
        return AIR_E_NOT_SUPPORT;
    }
    if (((pin_info >> CORAL_PINCTRL_PINx_OFS(CORAL_PIN_SDA)) & CORAL_PINCTRL_FIELD_MSK)
         != CORAL_GPIO20_MODE_SDA)
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* disable auto mode */
    addr = HAL_CORAL_SIF_GET_BASE_ADDRESS(channel) + CORAL_REG_SIF_AUTO_CTRL;
    rc = aml_readReg(unit, addr, &reg, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return AIR_E_NOT_SUPPORT;
    }
    /* make sure automode is enabled. o.w. do nothing here. */
    if (AIR_E_OK == rc && (1 == ((reg >> CORAL_AUTOMODE_EN_BIT) & CORAL_AUTOMODE_EN_MSK)))
    {
        data = (0 << CORAL_AUTOMODE_EN_BIT);
        mask = (1 << CORAL_AUTOMODE_EN_BIT);
        reg_value = (reg &  ~(mask)) | (data & mask);
        rc = aml_writeReg(unit, addr, &reg_value, sizeof(UI32_T));
        if (AIR_E_OK != rc)
        {
            return AIR_E_NOT_SUPPORT;
        }
    }

    /* sif config cr0 */
    addr = HAL_CORAL_SIF_GET_BASE_ADDRESS(channel) + CORAL_SIF_SIFMCTL0;
    rc = aml_readReg(unit, addr, &reg, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return AIR_E_NOT_SUPPORT;
    }
    data = (sif_clk_spd[clock] << CORAL_SIF_CLK_DIV_BIT) | (CORAL_SIF_EN_MSK << CORAL_SIF_EN_BIT);
    mask = CORAL_SIF_CLK_DIV_SM | CORAL_SIF_EN_SM;
    reg_value = (reg & ~(mask)) | (data & mask);
    rc = aml_writeReg(unit, addr, &reg_value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return AIR_E_NOT_SUPPORT;
    }
#elif AIR_8851_SUPPORT

#define REG_RGS_CPU_EN                  (0x100000A8UL)
#define REG_SIF_MASTER0_BASE_ADDRESS    (0x10008000UL)
#define REG_SIF_MASTER1_BASE_ADDRESS    (0x10022000UL)
#define CPU_I2C0_MODE_BIT               (4UL)
#define SIF_CLK_DIV_BIT                 (16UL)
#define SIF_EN_MSK                      (1UL)
#define SIF_EN_BIT                      (1UL)
#define SIF_CLK_DIV_MSK                 (0xfffUL)
#define SIF_CLK_DIV_SM                  ((SIF_CLK_DIV_MSK) << (SIF_CLK_DIV_BIT))
#define SIF_EN_SM                       ((SIF_EN_MSK) << (SIF_EN_BIT))
#define SIF_SIFMCTL0                    (0x40UL)
#define HAL_SCO_SIF_GET_BASE_ADDRESS(channel)                                               \
        (((channel) == 0) ? (REG_SIF_MASTER0_BASE_ADDRESS) : (REG_SIF_MASTER1_BASE_ADDRESS))

    UI32_T reg_value, reg, addr;
    const UI32_T sif_clk_spd[] = {0xc7, 0x31, 0x13};
    UI32_T data, mask;

    rc = AIR_E_OTHERS;
    aml_readReg(unit, REG_RGS_CPU_EN, &reg, sizeof(UI32_T));
    /* if the i2c module is master */
    if (reg & (1 << (CPU_I2C0_MODE_BIT + channel)))
    {
        /* get address */
        addr = HAL_SCO_SIF_GET_BASE_ADDRESS(channel) + SIF_SIFMCTL0;
        data = (sif_clk_spd[clock] << SIF_CLK_DIV_BIT) | (SIF_EN_MSK << SIF_EN_BIT);
        mask = SIF_CLK_DIV_SM | SIF_EN_SM;

        rc = aml_readReg(unit, addr, &reg, sizeof(UI32_T));
        if (AIR_E_OK == rc)
        {
            reg_value = (reg &  ~(mask)) | (data & mask);
            rc = aml_writeReg(unit, addr, &reg_value, sizeof(UI32_T));
        }
    }

#endif

    return rc;
}

static MW_ERROR_NO_T
_mw_sif_clock_set(
    const UI32_T                unit,
    const UI32_T                channel,
    const MW_SIF_CLOCK_TYPE_T   type)
{
    UI32_T          clock = 0;
    AIR_ERROR_NO_T  rc;

    switch(type)
    {
        case MW_SIF_CLOCK_TYPE_100K:
            clock = 0;
            break;
        case MW_SIF_CLOCK_TYPE_400K:
            clock = 1;
            break;
        case MW_SIF_CLOCK_TYPE_1M:
            clock = 2;
            break;
        default:
            return MW_E_OP_INVALID;
    }

    rc = _air_sif_clock_set(unit, channel, clock);
    if (AIR_E_OK != rc)
    {
        return MW_E_OTHERS;
    }
    _mw_sif_contex.sif_clock[channel] = clock;

    return MW_E_OK;
}

static MW_SIF_CLOCK_TYPE_T
_mw_sif_clock_get(
    const UI32_T                unit,
    const UI32_T                channel)
{
    UNUSED(unit)

    return _mw_sif_contex.sif_clock[channel];
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   mw_sif_init
 * PURPOSE:
 *      This API is used to initialize the mw sif module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_init(
    void)
{
    UI32_T          i, unit = 0;
    AIR_CFG_VALUE_T sif_cfg;
    AIR_ERROR_NO_T  rc;

    osapi_memset(&_mw_sif_contex, 0, sizeof(_mw_sif_contex));
    osapi_mutexCreate(MW_SIF_CLOCK_MODULE_NAME, &(_mw_sif_contex.sif_mutex));
    if(NULL == _mw_sif_contex.sif_mutex)
    {
        return MW_E_OTHERS;
    }

    osapi_memset(&sif_cfg, 0, sizeof(sif_cfg));

    for (i = 0; i < MW_SIF_CLOCK_MASTER_COUNT; i++)
    {
        sif_cfg.param0 = i;
        sif_cfg.param1 = 0;
        sif_cfg.value = 0;
        rc = air_cfg_getValue(unit, AIR_CFG_TYPE_SIF_LOCAL_CLOCK, &sif_cfg);
        if (AIR_E_OK != rc)
        {
            _mw_sif_contex.sif_clock[i] = MW_SIF_CLOCK_TYPE_100K;
            continue;
        }

        switch (sif_cfg.value)
        {
            case 0:
                _mw_sif_contex.sif_clock[i] = MW_SIF_CLOCK_TYPE_100K;
                break;
            case 1:
                _mw_sif_contex.sif_clock[i] = MW_SIF_CLOCK_TYPE_400K;
                break;
            case 2:
                _mw_sif_contex.sif_clock[i] = MW_SIF_CLOCK_TYPE_1M;
                break;
            default:
                osapi_mutexDelete(_mw_sif_contex.sif_mutex);
                _mw_sif_contex.sif_mutex = NULL;
                return MW_E_OTHERS;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME:    mw_sif_set_clock
 * PURPOSE:
 *      This API is used to set sif with specific clock.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      clock           --  Clock speed
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_set_clock(
    const UI32_T            unit,
    const UI32_T            channel,
    MW_SIF_CLOCK_TYPE_T     clock)
{
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    MW_CHECK_PTR(_mw_sif_contex.sif_mutex);
    MW_CHECK_MIN_MAX_RANGE(channel, 0, (MW_SIF_CLOCK_MASTER_COUNT-1));
    MW_CHECK_MIN_MAX_RANGE(clock, MW_SIF_CLOCK_TYPE_100K, MW_SIF_CLOCK_TYPE_1M);

    ret = osapi_mutexTake(_mw_sif_contex.sif_mutex, MW_SIF_CLOCK_WAITTIME);
    if (MW_E_OK == ret)
    {
        ret = _mw_sif_clock_set(unit, channel, clock);
        osapi_mutexGive(_mw_sif_contex.sif_mutex);
    }

    return ret;
}

/* FUNCTION NAME:    mw_sif_get_clock
 * PURPOSE:
 *      This API is used to get sif clock.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *
 * OUTPUT:
 *      ptr_clock       --  Clock speed
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_get_clock(
    const UI32_T            unit,
    const UI32_T            channel,
    MW_SIF_CLOCK_TYPE_T     *ptr_clock)
{
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    MW_CHECK_PTR(_mw_sif_contex.sif_mutex);
    MW_CHECK_PTR(ptr_clock);
    MW_CHECK_MIN_MAX_RANGE(channel, 0, (MW_SIF_CLOCK_MASTER_COUNT-1));

    *ptr_clock = MW_SIF_CLOCK_TYPE_LAST;
    ret = osapi_mutexTake(_mw_sif_contex.sif_mutex, MW_SIF_CLOCK_WAITTIME);
    if (MW_E_OK == ret)
    {
        *ptr_clock = _mw_sif_contex.sif_clock[channel];
        osapi_mutexGive(_mw_sif_contex.sif_mutex);
    }

    return ret;
}

/* FUNCTION NAME:   mw_sif_read
 * PURPOSE:
 *      This API is used to read data by sif with specific clock.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_read(
    const UI32_T            unit,
    MW_SIF_CLOCK_TYPE_T     clock,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param)
{
    UI8_T               retry_count = MW_SIF_OPERATION_RETRY_COUNT;
    MW_ERROR_NO_T       ret = MW_E_NOT_SUPPORT;
    AIR_ERROR_NO_T      rc;
    MW_SIF_CLOCK_TYPE_T old_clock;

    MW_CHECK_PTR(_mw_sif_contex.sif_mutex);
    MW_CHECK_PTR(ptr_sif_info);
    MW_CHECK_PTR(ptr_sif_param);

    MW_CHECK_MIN_MAX_RANGE(ptr_sif_info->channel, 0, (MW_SIF_CLOCK_MASTER_COUNT-1));
    MW_CHECK_MIN_MAX_RANGE(clock, MW_SIF_CLOCK_TYPE_100K, MW_SIF_CLOCK_TYPE_1M);

    ret = mw_sif_get_clock(unit, ptr_sif_info->channel, &old_clock);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    ret = osapi_mutexTake(_mw_sif_contex.sif_mutex, MW_SIF_CLOCK_WAITTIME);
    if (MW_E_OK == ret)
    {
        if (clock != _mw_sif_clock_get(unit, ptr_sif_info->channel))
        {
            ret = _mw_sif_clock_set(unit, ptr_sif_info->channel, clock);
            if (MW_E_OK != ret)
            {
                osapi_mutexGive(_mw_sif_contex.sif_mutex);
                return ret;
            }
        }

        while (retry_count--)
        {
            rc = air_sif_read(unit, ptr_sif_info, ptr_sif_param);
            if (AIR_E_OK == rc)
            {
                break;
            }
        }
        if (AIR_E_OK != rc)
        {
            ret = MW_E_OP_INCOMPLETE;
        }
        osapi_mutexGive(_mw_sif_contex.sif_mutex);
    }

    return ret;
}

/* FUNCTION NAME:   mw_sif_write
 * PURPOSE:
 *      This API is used to write data by sif with specific clock.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_write(
    const UI32_T            unit,
    MW_SIF_CLOCK_TYPE_T     clock,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param)
{
    UI8_T           retry_count = MW_SIF_OPERATION_RETRY_COUNT;
    MW_ERROR_NO_T   ret = MW_E_NOT_SUPPORT;
    AIR_ERROR_NO_T  rc;

    MW_CHECK_PTR(_mw_sif_contex.sif_mutex);
    MW_CHECK_PTR(ptr_sif_info);
    MW_CHECK_PTR(ptr_sif_param);

    MW_CHECK_MIN_MAX_RANGE(ptr_sif_info->channel, 0, (MW_SIF_CLOCK_MASTER_COUNT-1));
    MW_CHECK_MIN_MAX_RANGE(clock, MW_SIF_CLOCK_TYPE_100K, MW_SIF_CLOCK_TYPE_1M);

    ret = osapi_mutexTake(_mw_sif_contex.sif_mutex, MW_SIF_CLOCK_WAITTIME);
    if (MW_E_OK == ret)
    {
        if (clock != _mw_sif_clock_get(unit, ptr_sif_info->channel))
        {
            ret = _mw_sif_clock_set(unit, ptr_sif_info->channel, clock);
            if (MW_E_OK != ret)
            {
                osapi_mutexGive(_mw_sif_contex.sif_mutex);
                return ret;
            }
        }

        while (retry_count--)
        {
            rc = air_sif_write(unit, ptr_sif_info, ptr_sif_param);
            if (AIR_E_OK == rc)
            {
                break;
            }
        }

        if (AIR_E_OK != rc)
        {
            ret = MW_E_OP_INCOMPLETE;
        }
        osapi_mutexGive(_mw_sif_contex.sif_mutex);
    }

    return ret;
}
#endif
