/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:  sfp_ddm.c
 * PURPOSE:
 *      Process SFP DDM data.
 * NOTES:
 *
 */
#ifdef AIR_SUPPORT_SFP_DDM
/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_ddm.h"
#include "sfp_db.h"
#include "sfp_sff_data.h"
#include "sfp_util.h"
#include "sfp_module_handle.h"
#include "sfp_pin.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_DDM_DDM_TYPE_ADDRESS (0x5c)
#define SFP_DDM_DDM_CALIBRATION_DATA_ADDRESS (0x38)
#define SFP_DDM_DDM_DATA_ADDRESS (0x60)

/* It must be a multiple of 4. */
#define SFP_DDM_DDM_CALIBRATION_DATA_LENGTH (40)
#define SFP_DDM_DDM_DATA_LENGTH (10)

#define SFP_DDM_TEMPERATURE_OFFSET (0)
#define SFP_DDM_VOLTAGE_OFFSET (2)
#define SFP_DDM_CURRENT_OFFSET (4)
#define SFP_DDM_OUTPUT_POWER_OFFSET (6)
#define SFP_DDM_INPUT_POWER_OFFSET (8)

#define SFP_DDM_LOSS_OF_SIGNAL (1)
#define SFP_DDM_EXIST_OF_SIGNAL (0)

/* MACRO FUNCTION DECLARATIONS
 */
#define SFP_DDM_CALCULATE_PARAMETER(ddm_data, offset) (((*(ddm_data + offset)) << 8) + *(ddm_data + offset + 1))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_sfp_ddm_getDiagMonitorType(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_type)
{
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    UI32_T temp_type = 0;

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_DDM_DDM_TYPE_ADDRESS, 1, &temp_type);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    *ptr_type = temp_type;
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_sfp_ddm_getDDMdata(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_buffer,
    const UI32_T buffer_length)
{
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    UI32_T temp_buffer[(SFP_DDM_DDM_DATA_LENGTH / sizeof(UI32_T)) + 1] = {0};

    if ((NULL == ptr_buffer) || (buffer_length < SFP_DDM_DDM_DATA_LENGTH))
    {
        return AIR_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A2H_ADDRESS, SFP_DDM_DDM_DATA_ADDRESS, SFP_DDM_DDM_DATA_LENGTH, temp_buffer);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    osapi_memcpy(ptr_buffer, temp_buffer, SFP_DDM_DDM_DATA_LENGTH);
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_sfp_ddm_getLossSignalInfo(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    UI8_T *ptr_loss_of_signal)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    int loss_of_signal = 0;
    UI8_T rx_loss_pin = 0;

    if ((NULL == ptr_port_info) || (NULL == ptr_loss_of_signal))
    {
        return AIR_E_BAD_PARAMETER;
    }

    ret = sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_RX_LOSS, &rx_loss_pin);
    ret |= sfp_pin_io_getValue(unit, ptr_port_info->port, rx_loss_pin, &loss_of_signal);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    if(SFP_MODULE_PIN_HIGH == loss_of_signal)
    {
        *ptr_loss_of_signal = SFP_DDM_LOSS_OF_SIGNAL;
    }
    else
    {
        *ptr_loss_of_signal = SFP_DDM_EXIST_OF_SIGNAL;
    }

    return ret;
}

static MW_ERROR_NO_T
_sfp_ddm_extCalibrateData(
    const UI32_T unit,
    const UI32_T port,
    DB_SFP_MODULE_INFO_T *ptr_module_info)
{
    UI8_T *ptr_data = NULL, *ptr_cali_data = NULL, read_len = 0, i = 0;
    I32_T ret = MW_E_OTHERS;
    FLOAT_T rx_pwr4 = 0, rx_pwr3 = 0, rx_pwr2 = 0, rx_pwr1 = 0, rx_pwr0 = 0;
    FLOAT_T i_slope = 0, tx_pwrSlope = 0, t_slope = 0, v_slope = 0;
    I16_T i_offset = 0, tx_pwrOffset = 0, t_offset = 0, v_offset = 0;

    osapi_calloc(SFP_DDM_DDM_CALIBRATION_DATA_LENGTH + sizeof(UI32_T), SFP_TASK_NAME, (void**)&ptr_cali_data);
    if (NULL == ptr_cali_data)
    {
        return MW_E_NO_MEMORY;
    }

    /* 4-bytes aligned */
    ptr_data = (UI8_T *)(((UI32_T)ptr_cali_data & 0xFFFFFFFC) + sizeof(UI32_T));
    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A2H_ADDRESS, SFP_DDM_DDM_CALIBRATION_DATA_ADDRESS, SFP_DDM_DDM_CALIBRATION_DATA_LENGTH, (UI32_T *)ptr_data);
    if (MW_E_OK != ret)
    {
        SFP_LOG_ERROR("Read cali_data fail ret:%d", ret);
        MW_FREE(ptr_cali_data);
        return ret;
    }

    for (i = 0; i < SFP_DDM_DDM_CALIBRATION_DATA_LENGTH;)
    {
        if (i + 16 < SFP_DDM_DDM_CALIBRATION_DATA_LENGTH)
        {
            SFP_LOG_DEBUG("0x%x%x%x%x 0x%x%x%x%x 0x%x%x%x%x 0x%x%x%x%x",
                         ptr_data[i], ptr_data[i + 1], ptr_data[i + 2], ptr_data[i + 3],
                         ptr_data[i + 4], ptr_data[i + 5], ptr_data[i + 6], ptr_data[i + 7],
                         ptr_data[i + 8], ptr_data[i + 9], ptr_data[i + 10], ptr_data[i + 11],
                         ptr_data[i + 12], ptr_data[i + 13], ptr_data[i + 14], ptr_data[i + 15]);
            i += 16;
        }
        else
        {
           SFP_LOG_DEBUG("0x%x%x%x%x 0x%x%x%x%x",
                         ptr_data[i], ptr_data[i + 1], ptr_data[i + 2], ptr_data[i + 3],
                         ptr_data[i + 4], ptr_data[i + 5], ptr_data[i + 6], ptr_data[i + 7]);
            i += 8;
        }
    }

    rx_pwr4 = (FLOAT_T)get32(ptr_data);
    read_len += 4;
    rx_pwr3 = (FLOAT_T)get32(ptr_data + read_len);
    read_len += 4;
    rx_pwr2 = (FLOAT_T)get32(ptr_data + read_len);
    read_len += 4;
    rx_pwr1 = (FLOAT_T)get32(ptr_data + read_len);
    read_len += 4;
    rx_pwr0 = (FLOAT_T)get32(ptr_data + read_len);
    read_len += 4;
    i_slope = (FLOAT_T)ptr_data[read_len] + (FLOAT_T)ptr_data[read_len + 1] / 256.0;
    read_len += 2;
    i_offset = (I16_T)get16(ptr_data + read_len);
    read_len += 2;
    tx_pwrSlope = (FLOAT_T)ptr_data[read_len] + (FLOAT_T)ptr_data[read_len + 1] / 256.0;
    read_len += 2;
    tx_pwrOffset = (I16_T)get16(ptr_data + read_len);
    read_len += 2;
    t_slope = (FLOAT_T)ptr_data[read_len] + (FLOAT_T)ptr_data[read_len + 1] / 256.0;
    read_len += 2;
    t_offset = (I16_T)get16(ptr_data + read_len);
    read_len += 2;
    v_slope = (FLOAT_T)ptr_data[read_len] + (FLOAT_T)ptr_data[read_len + 1] / 256.0;
    read_len += 2;
    v_offset = (I16_T)get16(ptr_data + read_len);
    read_len += 2;

    ptr_data = NULL;
    MW_FREE(ptr_cali_data);

    SFP_LOG_DEBUG("ptr_data:%p ptr_cali_data:%p rx_pwr4:0x%x rx_pwr3:0x%x rx_pwr2:0x%x rx_pwr1:0x%x rx_pwr0:0x%x\r\n" \
              "i_slope:0x%x tx_pwrSlope:0x%x t_slope:0x%x v_slope:0x%x i_offset:%d tx_pwrOffset:%d t_offset:%d v_offset:%d",
              ptr_data, ptr_cali_data, (I32_T)rx_pwr4, (I32_T)rx_pwr3, (I32_T)rx_pwr2, (I32_T)rx_pwr1, (I32_T)rx_pwr0,
              (I32_T)i_slope, (I32_T)tx_pwrSlope, (I32_T)t_slope, (I32_T)v_slope, i_offset, tx_pwrOffset, t_offset, v_offset);

    ptr_module_info->temperature = (I16_T)((FLOAT_T)t_slope * (FLOAT_T)ptr_module_info->temperature + (FLOAT_T)t_offset);
    ptr_module_info->voltage = (UI16_T)((FLOAT_T)v_slope * (FLOAT_T)ptr_module_info->voltage + (FLOAT_T)v_offset);
    ptr_module_info->current = (UI16_T)((FLOAT_T)i_slope * (FLOAT_T)ptr_module_info->current+ (FLOAT_T)i_offset);
    ptr_module_info->output_power = (UI16_T)((FLOAT_T)tx_pwrSlope * (FLOAT_T)ptr_module_info->output_power+ (FLOAT_T)tx_pwrOffset);
    ptr_module_info->input_power = (UI16_T)(rx_pwr4 * (FLOAT_T)ptr_module_info->input_power * 10000.0 +
                                            rx_pwr3 * (FLOAT_T)ptr_module_info->input_power * 1000.0 +
                                            rx_pwr2 * (FLOAT_T)ptr_module_info->input_power * 100.0 +
                                            rx_pwr1 * (FLOAT_T)ptr_module_info->input_power +
                                            rx_pwr0);

    SFP_LOG_DEBUG("After calibration. port:%d ddm_type:0x%x temperature:%d voltage:%d current:%d output_power:%d input_power:%d",
              port, ptr_module_info->diagnostic_monitoring_type, ptr_module_info->temperature, ptr_module_info->voltage, ptr_module_info->current,
              ptr_module_info->output_power, ptr_module_info->input_power);

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_ddm_polling
 * PURPOSE:
 *      Polling DDM information for a port and update to DB.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *      AIR_E_NOT_INITED     -- Not initialized
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_ddm_polling(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    DB_SFP_MODULE_INFO_T module_info;
    UI8_T ddm_type = 0;
    UI8_T *ptr_ddm_data = NULL;
    UI8_T port_index = 0;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* The SFP SDA PIN driver and/or the SFP SDA IO driver are not initialized successfully. Adopt the SFP auto-adaption algorithm. */
    if (FALSE == sfp_pin_checkInitState(ptr_port_info->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        return AIR_E_OP_INVALID;
    }

    port_index = sfp_task_getPortIndex(ptr_port_info->port);
    if (SFP_TASK_PORT_INDEX_INVALID == port_index)
    {
        return AIR_E_BAD_PARAMETER;
    }

    osapi_memset(&module_info, 0, sizeof(DB_SFP_MODULE_INFO_T));
    module_info.port_num = ptr_port_info->port;
    if (SFP_MODULE_PORT_STATE_ACTIVE == ptr_port_info->state)
    {
        rc = _sfp_ddm_getLossSignalInfo(unit, ptr_port_info, &module_info.loss_of_signal);
        if (AIR_E_OK != rc)
        {
            SFP_LOG_ERROR("Fail to get loss_of_signalret:%d", rc);
            return rc;
        }

        rc = _sfp_ddm_getDiagMonitorType(unit, ptr_port_info->port, &ddm_type);
        SFP_LOG_DEBUG("port:%d ddm_type:0x%x ret:%d", ptr_port_info->port, ddm_type, rc);
        if (AIR_E_OK != rc)
        {
            return rc;
        }

        module_info.diagnostic_monitoring_type = ddm_type;

        if ((0 != (ddm_type & BIT(6))) && ((0 != (ddm_type & BIT(4))) || (0 != (ddm_type & BIT(5)))))
        {
            /* DDM is implemented(bit6) with internal calibration(bit5) or External calibration(bit4) set */
            osapi_calloc(SFP_DDM_DDM_DATA_LENGTH, SFP_TASK_NAME, (void**)&ptr_ddm_data);
            if (NULL == ptr_ddm_data)
            {
                return MW_E_NO_MEMORY;
            }

            osapi_memset(ptr_ddm_data, 0, SFP_DDM_DDM_DATA_LENGTH);

            rc = _sfp_ddm_getDDMdata(unit, ptr_port_info->port, ptr_ddm_data, SFP_DDM_DDM_DATA_LENGTH);
            if (AIR_E_OK != rc)
            {
                MW_FREE(ptr_ddm_data);
                SFP_LOG_ERROR("Fail to get DDM data ret:%d", rc);
                return rc;
            }

            module_info.temperature = SFP_DDM_CALCULATE_PARAMETER(ptr_ddm_data, SFP_DDM_TEMPERATURE_OFFSET);
            module_info.voltage = SFP_DDM_CALCULATE_PARAMETER(ptr_ddm_data, SFP_DDM_VOLTAGE_OFFSET);
            module_info.current = SFP_DDM_CALCULATE_PARAMETER(ptr_ddm_data, SFP_DDM_CURRENT_OFFSET);
            module_info.output_power = SFP_DDM_CALCULATE_PARAMETER(ptr_ddm_data, SFP_DDM_OUTPUT_POWER_OFFSET);
            module_info.input_power = SFP_DDM_CALCULATE_PARAMETER(ptr_ddm_data, SFP_DDM_INPUT_POWER_OFFSET);

            MW_FREE(ptr_ddm_data);

            SFP_LOG_DEBUG("port:%d port_index:%d type:0x%x temperature:0x%x voltage:0x%x current:0x%x output_power:0x%x input_power:0x%x",
                      ptr_port_info->port, port_index, module_info.diagnostic_monitoring_type, module_info.temperature, module_info.voltage, module_info.current, module_info.output_power, module_info.input_power);
            if (0 != (ddm_type & BIT(4)))
            {
                /* External calibration */
                _sfp_ddm_extCalibrateData(unit, ptr_port_info->port, &module_info);
            }
        }
    }

    return sfp_db_updateSFPInfo(port_index , &module_info);
}
#endif  /* AIR_SUPPORT_SFP_DDM */
