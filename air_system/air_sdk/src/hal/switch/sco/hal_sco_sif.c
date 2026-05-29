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

/* FILE NAME:  hal_sco_sif.c
 * PURPOSE:
 *  Implement SIF module HAL function.
 *
 * NOTES:
 *
 */
#include <hal/switch/sco/hal_sco_sif.h>

#include <air_cfg.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_SIF_SIFM_REG_DEFAULT_VALUE (0)

/* MACRO FUNCTION DECLARATIONS
 */
/* This semaphore cannot be removed, like mdio module */
#define HAL_SCO_SIF_SEMA(unit) (*_ptr_hal_sco_sif_sema[unit])
#define HAL_SCO_SIF_TAKE_SEMA(unit)                                                             \
    ({                                                                                          \
        AIR_ERROR_NO_T __rc = AIR_E_OK;                                                         \
        __rc = HAL_COMMON_LOCK_RESOURCE(&(HAL_SCO_SIF_SEMA(unit)), AIR_SEMAPHORE_WAIT_FOREVER); \
        if (AIR_E_OK != __rc)                                                                   \
        {                                                                                       \
            DIAG_PRINT(HAL_DBG_ERR, "u = %u, take semaphore failed, rc = %d\n", unit, __rc);    \
            return __rc;                                                                        \
        }                                                                                       \
    })

#define HAL_SCO_SIF_GIVE_SEMA(unit)                                                          \
    ({                                                                                       \
        AIR_ERROR_NO_T __rc = AIR_E_OK;                                                      \
        __rc = HAL_COMMON_FREE_RESOURCE(&(HAL_SCO_SIF_SEMA(unit)));                          \
        if (AIR_E_OK != __rc)                                                                \
        {                                                                                    \
            DIAG_PRINT(HAL_DBG_ERR, "u = %u, take semaphore failed, rc = %d\n", unit, __rc); \
            return __rc;                                                                     \
        }                                                                                    \
    })

#define HAL_SCO_SIF_GET_BASE_ADDRESS(channel)                                            \
    (((channel) == 0) ? (REG_SIF_MASTER0_BASE_ADDRESS) : (REG_SIF_MASTER1_BASE_ADDRESS))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SIF, "hal_sco_sif.c");

/* Used for semaphore */
static AIR_SEMAPHORE_ID_T *_ptr_hal_sco_sif_sema[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static AIR_ERROR_NO_T
_hal_sco_sif_opSpecMod(const UI32_T               unit,
                       const UI16_T               channel,
                       HAL_SCO_SIF_PARAM_T       *ptr_sif_param,
                       const HAL_SCO_SIF_DEVICE_T sif_device,
                       const HAL_SCO_SIFM_OP_T    read_or_write);

/**********************************************************************
 * LOCAL SUBPROGRAM BODIES
 **********************************************************************/

static AIR_ERROR_NO_T
_hal_sco_sif_readReg(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI32_T         offset,
    UI32_T              *ptr_data,
    HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T  *ptr_sif_info)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              addr = 0;
    HAL_SCO_SIF_PARAM_T sif_param;

    addr = HAL_SCO_SIF_GET_BASE_ADDRESS(channel) + offset;

    if (HAL_SCO_SIF_LOCAL_DEVICE == sif_device)
    {
        rc = aml_readReg(unit, addr, ptr_data, sizeof(UI32_T));
    }
    else
    {
        sif_param.device_info.slave_id = ptr_sif_info->slave_id;
        sif_param.device_info.remote_slave_id = 0;
        sif_param.device_info.channel = 0;
        sif_param.device_info.remote_channel = 0;
        sif_param.addr = addr;
        sif_param.addr_len = 4;
        sif_param.ptr_data = ptr_data;
        sif_param.data_len = 4;

        rc = _hal_sco_sif_opSpecMod(0, ptr_sif_info->channel, &sif_param, 0, HAL_SCO_SIFM_READ);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_writeReg(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI32_T         offset,
    UI32_T              *ptr_data,
    HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T  *ptr_sif_info)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              addr = 0;
    HAL_SCO_SIF_PARAM_T sif_param;

    addr = HAL_SCO_SIF_GET_BASE_ADDRESS(channel) + offset;

    if (HAL_SCO_SIF_LOCAL_DEVICE == sif_device)
    {
        rc = aml_writeReg(unit, addr, ptr_data, sizeof(UI32_T));
    }
    else
    {
        sif_param.device_info.slave_id = ptr_sif_info->slave_id;
        sif_param.device_info.remote_slave_id = 0;
        sif_param.device_info.channel = 0;
        sif_param.device_info.remote_channel = 0;
        sif_param.addr = addr;
        sif_param.addr_len = 4;
        sif_param.ptr_data = ptr_data;
        sif_param.data_len = 4;

        rc = _hal_sco_sif_opSpecMod(0, ptr_sif_info->channel, &sif_param, 0, HAL_SCO_SIFM_WRITE);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_writeRegMask(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI8_T          offset,
    const UI32_T         value,
    const UI32_T         mask,
    HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T  *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* read then write */
    UI32_T         reg_value, data;

    rc = _hal_sco_sif_readReg(unit, channel, offset, &reg_value, sif_device, ptr_sif_info);
    if (AIR_E_OK == rc)
    {
        data = (reg_value & ~(mask)) | (value & mask);
        rc = _hal_sco_sif_writeReg(unit, channel, offset, &data, sif_device, ptr_sif_info);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_configCr0(
    const UI32_T               unit,
    const UI16_T               channel,
    const UI32_T               clk_spd,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* get address */
    UI32_T         data = (clk_spd << SIF_CLK_DIV_BIT) | (SIF_EN_MSK << SIF_EN_BIT);
    UI32_T         mask = SIF_CLK_DIV_SM | SIF_EN_SM;

    rc = _hal_sco_sif_writeRegMask(unit, channel, SIF_SIFMCTL0, data, mask, sif_device, ptr_sif_info);
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_reverseAddr(
    const UI32_T unit,
    UI32_T      *ptr_addr,
    UI32_T       addr_len)
{
    /* Use bitwise operation to avoid branch prediction */
    *ptr_addr = (*ptr_addr & 0xFFFF0000) >> 16 | (*ptr_addr & 0x0000FFFF) << 16;
    *ptr_addr = (*ptr_addr & 0xFF00FF00) >> 8 | (*ptr_addr & 0x00FF00FF) << 8;
    *ptr_addr >>= (4 - addr_len) * 8;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_sif_checkBus(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_value;

    rc = _hal_sco_sif_readReg(unit, channel, SIF_SIFMCTL0, &reg_value, sif_device, ptr_sif_info);
    if (AIR_E_OK == rc)
    {
        if ((reg_value & (SIF_SDA_STATE_SM)) && (reg_value & (SIF_SCL_STATE_SM)))
        {
            rc = AIR_E_OK;
        }
        else
        {
            rc = AIR_E_OTHERS;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_waitHwDone(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = SIF_TRI_MSK, loopcount = 0;

    while (data & SIF_TRI_MSK)
    {
        rc = _hal_sco_sif_readReg(unit, channel, SIF_SIFMCTL1, &data, sif_device, ptr_sif_info);
        if (AIR_E_OK != rc)
        {
            return rc;
        }
        if (++loopcount >= 0xffff)
        {
            DIAG_PRINT(HAL_DBG_ERR, "sif time out\n");
            return AIR_E_TIMEOUT;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putModeBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_CR1_OP_MODE_T  op_mode,
    UI32_T                     page_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    UI32_T         data = (page_len << SIF_PGLEN_BIT) | (op_mode << SIF_MODE_BIT) | SIF_TRI_MSK;
    UI32_T         mask = SIF_PGLEN_SM | SIF_MODE_SM | SIF_TRI_SM;

    rc = _hal_sco_sif_writeRegMask(unit, channel, SIF_SIFMCTL1, data, mask, sif_device, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    return _hal_sco_sif_waitHwDone(unit, channel, sif_device, ptr_sif_info);
}

static AIR_ERROR_NO_T
_hal_sco_sif_putStartBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _hal_sco_sif_putModeBit(unit, channel, sif_device, HAL_SCO_SIF_START_BIT, 0, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "put start bit failed\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putWriteBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const UI16_T               page_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _hal_sco_sif_putModeBit(unit, channel, sif_device, HAL_SCO_SIF_WRITE_DATA, page_len, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "put write bit failed\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putStopBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _hal_sco_sif_putModeBit(unit, channel, sif_device, HAL_SCO_SIF_STOP_BIT, 0, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "put stop bit failed\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putReadFinalBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const UI16_T               page_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _hal_sco_sif_putModeBit(unit, channel, sif_device, HAL_SCO_SIF_READ_FINAL_BIT, page_len, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "put read final bit failed\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putReadBit(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const UI16_T               pageLen,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _hal_sco_sif_putModeBit(unit, channel, sif_device, HAL_SCO_SIF_READ_BIT, pageLen, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "put read bit failed\n");
    }
    return rc;
}

static BOOL_T
_hal_sco_sif_checkAck(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const UI16_T               expect_ack,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    /* read configuration register 1 */
    AIR_ERROR_NO_T rc = AIR_E_OK;

    UI32_T         reg_value = 0, ack = 0;

    rc = _hal_sco_sif_readReg(unit, channel, SIF_SIFMCTL1, &reg_value, sif_device, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        return FALSE;
    }

    reg_value &= SIF_ACK_SM;

    /* Count number of 1 */
    while (reg_value)
    {
        reg_value &= (reg_value - 1);
        ack++;
    }

    if (ack == expect_ack)
    {
        return TRUE;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "expect_ack = %d, ack = %d\n", expect_ack, ack);
        return FALSE;
    }
}

static AIR_ERROR_NO_T
_hal_sco_sif_putSlaveId(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const UI16_T               slave_id,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* write data in data register */
    UI32_T         data = slave_id;
    UI32_T         mask = 0xFF;

    rc = _hal_sco_sif_writeRegMask(unit, channel, SIF_SIFMD0, data, mask, sif_device, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* put write bit */
    rc = _hal_sco_sif_putWriteBit(unit, channel, sif_device, 0, ptr_sif_info);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* check ACK */
    if (FALSE == _hal_sco_sif_checkAck(unit, channel, sif_device, 1, ptr_sif_info))
    {
        DIAG_PRINT(HAL_DBG_ERR, "slave_id = 0x%x\n", slave_id);
        return AIR_E_OTHERS;
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putMultiBytes(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    UI32_T                    *ptr_data,
    UI32_T                     data_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* This function supports infinite byte transmission */
    UI32_T         dr0 = 0, dr1 = 0, i, split_bytes, total_loops;

    total_loops = (data_len + HAL_SCO_SIF_MAX_PAGE_SIZE - 1) / HAL_SCO_SIF_MAX_PAGE_SIZE;
    for (i = 0; i < total_loops; i++)
    {
        if (i == (total_loops - 1))
        {
            split_bytes = data_len % HAL_SCO_SIF_MAX_PAGE_SIZE;
        }
        else
        {
            split_bytes = HAL_SCO_SIF_MAX_PAGE_SIZE;
        }

        split_bytes = split_bytes == 0 ? HAL_SCO_SIF_MAX_PAGE_SIZE : split_bytes;

        /* data do not need to reverse, the page length bit will prevent the
         * redundant bytes are sent
         */
        /* get the data for data register */
        dr0 = ptr_data[2 * i];

        /* write to data register */
        rc = _hal_sco_sif_writeReg(unit, channel, SIF_SIFMD0, &dr0, sif_device, ptr_sif_info);
        if (AIR_E_OK != rc)
        {
            return rc;
        }

        if (4 < split_bytes)
        {
            dr1 = ptr_data[2 * i + 1];
            rc = _hal_sco_sif_writeReg(unit, channel, SIF_SIFMD1, &dr1, sif_device, ptr_sif_info);
            if (rc)
            {
                return rc;
            }
        }

        /* send write data mode */
        if (AIR_E_OK == _hal_sco_sif_putWriteBit(unit, channel, sif_device, split_bytes - 1, ptr_sif_info))
        {
            /* check ACK */
            if (FALSE == _hal_sco_sif_checkAck(unit, channel, sif_device, split_bytes, ptr_sif_info))
            {
                rc = AIR_E_OTHERS;
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_putSubaddress(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    UI32_T                    *ptr_data,
    UI32_T                     data_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    return _hal_sco_sif_putMultiBytes(unit, channel, sif_device, ptr_data, data_len, ptr_sif_info);
}

static AIR_ERROR_NO_T
_hal_sco_sif_getMultiBytes(
    const UI32_T               unit,
    const UI16_T               channel,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    UI32_T                    *ptr_data,
    UI32_T                     data_len,
    HAL_SCO_SIF_INFO_T        *ptr_sif_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* This function supports infinite byte transmission */
    UI32_T         split_bytes, total_loops, i, dr0 = 0, dr1 = 0;

    total_loops = (data_len + HAL_SCO_SIF_MAX_PAGE_SIZE - 1) / HAL_SCO_SIF_MAX_PAGE_SIZE;
    for (i = 0; i < total_loops; i++)
    {
        if (i == (total_loops - 1))
        {
            split_bytes = data_len % HAL_SCO_SIF_MAX_PAGE_SIZE;
        }
        else
        {
            split_bytes = HAL_SCO_SIF_MAX_PAGE_SIZE;
        }

        split_bytes = split_bytes == 0 ? HAL_SCO_SIF_MAX_PAGE_SIZE : split_bytes;

        if (i == (total_loops - 1))
        {
            rc = _hal_sco_sif_putReadFinalBit(unit, channel, sif_device, split_bytes - 1, ptr_sif_info);
        }
        else
        {
            rc = _hal_sco_sif_putReadBit(unit, channel, sif_device, split_bytes - 1, ptr_sif_info);
        }

        if (AIR_E_OK == rc)
        {
            rc = _hal_sco_sif_readReg(unit, channel, SIF_SIFMD0, &dr0, sif_device, ptr_sif_info);
            rc = _hal_sco_sif_readReg(unit, channel, SIF_SIFMD1, &dr1, sif_device, ptr_sif_info);

            /* if split_bytes == 3, then the DR1 may return 0xa8010fa5
             * However, the valid value is 0x10fa5
             * Thus, we need to clear the 0xa80 terms
             */
            if (AIR_E_OK == rc)
            {
                if (split_bytes < 4)
                {
                    dr0 = dr0 & (0xffffffff >> (4 - split_bytes) * 8);
                }
                else
                {
                    dr1 = dr1 & (0xffffffff >> (4 - (split_bytes - 4)) * 8);
                }

                /* return the value */
                ptr_data[2 * i] = dr0;

                if (split_bytes > 4)
                {
                    ptr_data[2 * i + 1] = dr1;
                }
            }
        }
    }
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_sif_opSpecMod(
    const UI32_T               unit,
    const UI16_T               channel,
    HAL_SCO_SIF_PARAM_T       *ptr_sif_param,
    const HAL_SCO_SIF_DEVICE_T sif_device,
    const HAL_SCO_SIFM_OP_T    rw_operation)
{
    /* Here, I divide this function into three parts,
     * First part, check the i2c bus is idle
     * Second part, i2c addressing
     * Third part, do the read or write operation
     */
    AIR_ERROR_NO_T rc = AIR_E_OK, stop_rc = AIR_E_OK;

    /* check bus status */
    rc = _hal_sco_sif_checkBus(unit, channel, sif_device, &ptr_sif_param->device_info);
    if (AIR_E_OK != rc)
    {
        /* if the bus is not idle, put the stop bit */
        if (AIR_E_OK == _hal_sco_sif_putStopBit(unit, channel, sif_device, &ptr_sif_param->device_info))
        {
            rc = _hal_sco_sif_checkBus(unit, channel, sif_device, &ptr_sif_param->device_info);
        }
    }

    /* put start bit */
    if (AIR_E_OK == rc)
    {
        if (AIR_E_OK == _hal_sco_sif_putStartBit(unit, channel, sif_device, &ptr_sif_param->device_info))
        {
            /* put device ID */
            if (ptr_sif_param->device_info.remote_slave_id)
            {
                rc = _hal_sco_sif_putSlaveId(unit, channel, sif_device,
                                             (ptr_sif_param->device_info.remote_slave_id << 1 | 0),
                                             &ptr_sif_param->device_info);
            }
            else
            {
                rc = _hal_sco_sif_putSlaveId(unit, channel, sif_device, (ptr_sif_param->device_info.slave_id << 1 | 0),
                                             &ptr_sif_param->device_info);
            }

            if (AIR_E_OK == rc)
            {
                if (!(0 == ptr_sif_param->addr_len))
                {
                    /*
                     * Due to the hardware capability, the address needs to reverse
                     * ADDRESS: Send from LSB, recieve from MSB
                     * However, the data no need to reverse
                     * DATA: Send from LSB, receive from LSB
                     *
                     * Where MSB stands for Most Significant Byte, and LSB stands for Least Significant Byte
                     */
                    if (AIR_E_OK == _hal_sco_sif_reverseAddr(unit, &ptr_sif_param->addr, ptr_sif_param->addr_len))
                    {
                        rc = _hal_sco_sif_putSubaddress(unit, channel, sif_device, &ptr_sif_param->addr,
                                                        ptr_sif_param->addr_len, &ptr_sif_param->device_info);
                    }
                }
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        /* read or write */
        if (HAL_SCO_SIFM_READ == rw_operation)
        {
            /* read, restart */
            if (AIR_E_OK == _hal_sco_sif_putStartBit(unit, channel, sif_device, &ptr_sif_param->device_info))
            {
                if (ptr_sif_param->device_info.remote_slave_id)
                {
                    rc = _hal_sco_sif_putSlaveId(unit, channel, sif_device,
                                                 (ptr_sif_param->device_info.remote_slave_id << 1 | 1),
                                                 &ptr_sif_param->device_info);
                }
                else
                {
                    rc = _hal_sco_sif_putSlaveId(unit, channel, sif_device,
                                                 (ptr_sif_param->device_info.slave_id << 1 | 1),
                                                 &ptr_sif_param->device_info);
                }
                if (AIR_E_OK == rc)
                {
                    rc = _hal_sco_sif_getMultiBytes(unit, channel, sif_device, ptr_sif_param->ptr_data,
                                                    ptr_sif_param->data_len, &ptr_sif_param->device_info);
                }
            }
        }
        else
        {
            /* write, continue write data */
            rc = _hal_sco_sif_putMultiBytes(unit, channel, sif_device, ptr_sif_param->ptr_data, ptr_sif_param->data_len,
                                            &ptr_sif_param->device_info);
        }

        /* avoid tricking upper layer OK if stop is put flawlessly. */
        stop_rc = _hal_sco_sif_putStopBit(unit, channel, sif_device, &ptr_sif_param->device_info);
    }

    /* every operation aside from putting stop bit is OK */
    if (AIR_E_OK == rc)
    {
        /* the result should reflect the last operation - putting stop bit */
        return stop_rc;
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_sco_sif_rwOperation(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param,
    const UI32_T          read_or_write)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    HAL_SCO_SIF_PARAM_T  sif_param;
    HAL_SCO_SIF_DEVICE_T sif_device;
    UI32_T               channel;

    /* Load the parameters */
    sif_param.device_info.slave_id = ptr_sif_info->slave_id;
    sif_param.device_info.channel = ptr_sif_info->channel;
    if (ptr_sif_remote_info == NULL)
    {
        sif_param.device_info.remote_slave_id = 0;
        sif_param.device_info.remote_channel = 0;
    }
    else
    {
        sif_param.device_info.remote_slave_id = ptr_sif_remote_info->slave_id;
        sif_param.device_info.remote_channel = ptr_sif_remote_info->channel;
    }
    sif_param.addr_len = ptr_sif_param->addr_len;
    sif_param.addr = ptr_sif_param->addr;
    sif_param.data_len = ptr_sif_param->data_len;

    if (4 < sif_param.data_len)
    {
        sif_param.ptr_data = ptr_sif_param->info.ptr_data;
    }
    else
    {
        sif_param.ptr_data = &ptr_sif_param->info.data;
    }
    /* Load the parameters complete */

    if (sif_param.device_info.remote_slave_id)
    {
        sif_device = HAL_SCO_SIF_REMOTE_DEVICE;
    }
    else
    {
        sif_device = HAL_SCO_SIF_LOCAL_DEVICE;
    }

    channel =
        HAL_SCO_SIF_REMOTE_DEVICE == sif_device ? sif_param.device_info.remote_channel : sif_param.device_info.channel;
    if ((sif_param.device_info.remote_channel < HAL_SCO_SIF_MASTER_COUNT) &&
        (sif_param.device_info.channel < HAL_SCO_SIF_MASTER_COUNT))
    {
        HAL_SCO_SIF_TAKE_SEMA(unit);
        rc = _hal_sco_sif_opSpecMod(0, channel, &sif_param, sif_device, read_or_write);
        HAL_SCO_SIF_GIVE_SEMA(unit);
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

#ifdef AIR_EN_I2C_TO_I2C_ACCESS
static AIR_ERROR_NO_T
_hal_sco_sif_initRemote(
    const UI32_T unit,
    const UI16_T channel,
    const UI32_T slave_id,
    const UI32_T clk_spd)
{
    AIR_ERROR_NO_T      rc;
    I32_T               i;
    UI32_T              reg_value;
    HAL_SCO_SIF_PARAM_T sif_param;

    /* load the sif parameter structure */
    sif_param.addr_len = 4;
    sif_param.addr = REG_RGS_CPU_EN;
    sif_param.ptr_data = &reg_value;
    sif_param.data_len = 4;
    sif_param.device_info.slave_id = slave_id;
    sif_param.device_info.channel = channel;
    sif_param.device_info.remote_slave_id = 0;
    sif_param.device_info.remote_channel = 0;

    rc = _hal_sco_sif_opSpecMod(0, channel, &sif_param, HAL_SCO_SIF_LOCAL_DEVICE, HAL_SCO_SIFM_READ);
    if (AIR_E_OK == rc)
    {
        /* initialize the master 0 and master 1 simultaneously */
        for (i = 0; i < HAL_SCO_SIF_MASTER_COUNT; i++)
        {
            if (reg_value & (1 << (i + CPU_I2C0_MODE_BIT)))
            {
                rc = _hal_sco_sif_configCr0(unit, i, clk_spd, HAL_SCO_SIF_REMOTE_DEVICE, &sif_param.device_info);
                if (AIR_E_OK != rc)
                {
                    break;
                }
            }
        }
    }
    return rc;
}
#endif

static AIR_ERROR_NO_T
_hal_sco_sif_initRscr(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    AIR_SEMAPHORE_ID_T *ptr_sif_sema_id = NULL;
    UI32_T              size = 0;

    size = sizeof(AIR_SEMAPHORE_ID_T);
    ptr_sif_sema_id = (AIR_SEMAPHORE_ID_T *)osal_alloc(size, air_module_getModuleName(AIR_MODULE_SIF));
    HAL_CHECK_PTR(ptr_sif_sema_id);
    osal_memset(ptr_sif_sema_id, 0, size);
    _ptr_hal_sco_sif_sema[unit] = ptr_sif_sema_id;
    rc = osal_createSemaphore("SIF_SEMA", AIR_SEMAPHORE_BINARY, ptr_sif_sema_id,
                              air_module_getModuleName(AIR_MODULE_SIF));

    return rc;
}
static AIR_ERROR_NO_T
_hal_sco_sif_deinitRscr(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    AIR_SEMAPHORE_ID_T *ptr_sif_sema_id = NULL;

    ptr_sif_sema_id = _ptr_hal_sco_sif_sema[unit];

    rc = osal_destroySemaphore(ptr_sif_sema_id);
    if (AIR_E_OK == rc)
    {
        osal_free(ptr_sif_sema_id);
        _ptr_hal_sco_sif_sema[unit] = NULL;
    }
    return rc;
}
/**********************************************************************
 * EXPORTED SUBPROGRAM BODIES
 **********************************************************************/

/* FUNCTION NAME:   hal_sco_sif_init
 * PURPOSE:
 *      Initialize sif module.
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
hal_sco_sif_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T  rc;
    AIR_CFG_VALUE_T sif_cfg;
    UI32_T          reg_value = 0, i;

#ifdef AIR_EN_I2C_TO_I2C_ACCESS
    UI32_T slave_id = 0;
    BOOL_T sif_local_channel[HAL_SCO_SIF_MASTER_COUNT] = {HAL_SCO_SIFM_SLAVE, HAL_SCO_SIFM_SLAVE};
#endif

    /* 0xc7 denotes for standard mode, 100kHz
     * 0X31 denotes for fast mode, 400kHz
     * 0x13 denotes for fast mode plus, 1MHz
     */
    const UI32_T sif_clk_spd[] = {0xc7, 0x31, 0x13};

    /* initialize the lock */
    rc = _hal_sco_sif_initRscr(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize the customer channel */
    osal_memset(&sif_cfg, 0, sizeof(AIR_CFG_VALUE_T));

    /* get the local config value from customer_ref.c */
    for (i = 0; i < HAL_SCO_SIF_MASTER_COUNT; i++)
    {
        sif_cfg.param0 = i;
        sif_cfg.param1 = 0;
        sif_cfg.value = 0;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_SIF_LOCAL_CLOCK, &sif_cfg);

        if (AIR_E_OK == rc)
        {
            aml_readReg(unit, REG_RGS_CPU_EN, &reg_value, sizeof(UI32_T));
            /* if the i2c module is master */
            if (reg_value & (1 << (CPU_I2C0_MODE_BIT + sif_cfg.param0)))
            {
                rc = _hal_sco_sif_configCr0(unit, sif_cfg.param0, sif_clk_spd[sif_cfg.value], 0, NULL);
#ifdef AIR_EN_I2C_TO_I2C_ACCESS
                sif_local_channel[i] = HAL_SCO_SIFM_MASTER;
#endif
            }
        }
    }

#ifdef AIR_EN_I2C_TO_I2C_ACCESS
    /* Remote device init */
    for (i = 0; i < HAL_SCO_SIF_MASTER_COUNT; i++)
    {
        if (HAL_SCO_SIFM_MASTER == sif_local_channel[i])
        {
            /* First we need to get slave id */
            sif_cfg.param0 = i;
            sif_cfg.param1 = 0;
            sif_cfg.value = 0x52; /* default value */
            rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_SIF_REMOTE_SLAVE_ID, &sif_cfg);

            /* debug usage */
            slave_id = sif_cfg.value;

            /* get clock from customer_ref.c */
            sif_cfg.param0 = i;
            sif_cfg.param1 = slave_id;
            sif_cfg.value = 0; /* default value */
            rc = _hal_sco_sif_initRemote(unit, sif_cfg.param0, slave_id, sif_clk_spd[sif_cfg.value]);
        }
    }
#endif
    return rc;
}

/* FUNCTION NAME:   hal_sco_sif_deinit
 * PURPOSE:
 *      Deinitialize sif module
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
hal_sco_sif_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    UI32_T         reg_value;

    reg_value = HAL_SCO_SIF_SIFM_REG_DEFAULT_VALUE;
    aml_writeReg(unit, REG_SIF_MASTER0_BASE_ADDRESS + SIF_SIFMCTL0, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER1_BASE_ADDRESS + SIF_SIFMCTL0, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER0_BASE_ADDRESS + SIF_SIFMCTL1, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER0_BASE_ADDRESS + SIF_SIFMD0, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER0_BASE_ADDRESS + SIF_SIFMD1, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER1_BASE_ADDRESS + SIF_SIFMCTL1, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER1_BASE_ADDRESS + SIF_SIFMD0, &reg_value, sizeof(reg_value));
    aml_writeReg(unit, REG_SIF_MASTER1_BASE_ADDRESS + SIF_SIFMD1, &reg_value, sizeof(reg_value));

    /* initialize the lock */
    rc = _hal_sco_sif_deinitRscr(unit);

    return rc;
}

/* FUNCTION NAME:   hal_sco_sif_write
 * PURPOSE:
 *      This API is used to do the I2C write operation
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sif_write(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param)
{
    return _hal_sco_sif_rwOperation(unit, ptr_sif_info, NULL, ptr_sif_param, 0);
}

/* FUNCTION NAME:   hal_sco_sif_read
 * PURPOSE:
 *      This API is used to do the I2C read operation
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sif_read(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param)
{
    return _hal_sco_sif_rwOperation(unit, ptr_sif_info, NULL, ptr_sif_param, 1);
}

/* FUNCTION NAME:   hal_sco_sif_writeByRemote
 * PURPOSE:
 *      This API is used to do the I2C write operation
 *      by remote device
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_remote_info  -- Pointer of remote sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sif_writeByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param)
{
#ifdef AIR_EN_I2C_TO_I2C_ACCESS
    return _hal_sco_sif_rwOperation(unit, ptr_sif_info, ptr_sif_remote_info, ptr_sif_param, 0);
#else
    return AIR_E_NOT_SUPPORT;
#endif
}

/* FUNCTION NAME:   hal_sco_sif_readByRemote
 * PURPOSE:
 *      This API is used to do the I2C read operation
 *      by remote device
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_remote_info  -- Pointer of remote sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation failed.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sif_readByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param)
{
#ifdef AIR_EN_I2C_TO_I2C_ACCESS
    return _hal_sco_sif_rwOperation(unit, ptr_sif_info, ptr_sif_remote_info, ptr_sif_param, 1);
#else
    return AIR_E_NOT_SUPPORT;
#endif
}
