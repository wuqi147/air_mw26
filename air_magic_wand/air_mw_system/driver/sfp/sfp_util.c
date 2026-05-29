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

/* FILE NAME:   sfp_util.c
 * PURPOSE:
 *      Provide some useful interfaces.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_memory.h"
#include "sfp_util.h"
#include "sfp_sff_data.h"
#include "sfp_task.h"
#include "sfp_config_customer.h"
#include "sfp_auto_adaptation.h"
#include "sfp_module_handle.h"
#include "sfp_msg.h"
#if (!defined(AIR_LITE_MW))
#include "mw_sif.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_UTIL_SERDES_OPERATION_RETRY_COUNT    (3)

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_util_sif_write
 * PURPOSE:
 *      This API is used to do the I2C write operation
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_util_sif_write(
    const UI32_T            unit,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param)
{
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

#if (!defined(AIR_LITE_MW))
    ret = mw_sif_write(unit, MW_SIF_CLOCK_TYPE_100K, ptr_sif_info, ptr_sif_param);
#endif

    return ret;
}

/* FUNCTION NAME:   sfp_util_sif_read
 * PURPOSE:
 *      This API is used to do the I2C read operation.
 *
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
 *      MW_E_OK              -- Operation success
 *      MW_E_OTHERS          -- Other errors
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_OP_INVALID      -- Operation is invalid
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_sif_read(
    const UI32_T            unit,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param)
{
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

#if (!defined(AIR_LITE_MW))
    ret = mw_sif_read(unit, MW_SIF_CLOCK_TYPE_100K, ptr_sif_info, ptr_sif_param);
#endif

    return ret;
}

#ifdef AIR_EN_I2C_BITBANG
/* FUNCTION NAME:   sfp_util_i2c_bitbang_write
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang write operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success.
 *      MW_E_OTHERS         -- No ack.
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong.
 *      MW_E_NOT_INITED     -- I2C bus is not inited.
 *      MW_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_i2c_bitbang_write(
    const UI32_T unit,
    const UI16_T channel,
    const UI16_T slave_id,
    I2C_BITBANG_PARAM_T *ptr_param)
{
    UI8_T retry_count = SFP_UTIL_SERDES_OPERATION_RETRY_COUNT;
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    while(retry_count--)
    {
        ret = i2c_bitbang_write(unit, channel, slave_id, ptr_param);
        if (MW_E_OK == ret)
        {
            break;
        }
    }

    return (MW_ERROR_NO_T)ret;
}

/* FUNCTION NAME:   sfp_util_i2c_bitbang_read
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang read operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * RETURN:
 *      MW_E_OK             -- Operation success.
 *      MW_E_OTHERS         -- No ack.
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong.
 *      MW_E_NOT_INITED     -- I2C bus is not inited.
 *      MW_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_util_i2c_bitbang_read(
    const UI32_T unit,
    const UI16_T channel,
    const UI16_T slave_id,
    I2C_BITBANG_PARAM_T *ptr_param)
{
    UI8_T retry_count = SFP_UTIL_SERDES_OPERATION_RETRY_COUNT;
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    while(retry_count--)
    {
        ret = i2c_bitbang_read(unit, channel, slave_id, ptr_param);
        if (MW_E_OK == ret)
        {
            break;
        }
    }

    return (MW_ERROR_NO_T)ret;
}
#endif

const SFP_CONFIG_PORT_SETTINGS_T *
sfp_port_getPortSettings(
    const UI32_T unit,
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize(), i = 0;

    if ((NULL != ptr_portSettings) && (0 != port_count))
    {
        for (; i < port_count; i++)
        {
            if (ptr_portSettings[i].port == port)
            {
                return &ptr_portSettings[i];
            }
        }
    }

    return NULL;
}

/* FUNCTION NAME:   sfp_port_is_serdesPort
 * PURPOSE:
 *      Check if the port is a pure serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a pure serdes port
 *      FALSE            --  The port is not a pure serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_serdesPort(
    const UI32_T unit,
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);

    return (NULL == ptr_portSettings) ? FALSE : (SFP_CONFIG_PORT_TYPE_SFP == ptr_portSettings->port_type);
}

/* FUNCTION NAME:   sfp_port_is_comboPort
 * PURPOSE:
 *      Check if the port is a COMBO port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a COMBO port
 *      FALSE            --  The port is not a COMBO port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_comboPort(
    const UI32_T unit,
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);

    return (NULL == ptr_portSettings) ? FALSE : ((SFP_CONFIG_PORT_TYPE_COMBO == ptr_portSettings->port_type) || (SFP_CONFIG_PORT_TYPE_COMBO_SFP == ptr_portSettings->port_type));
}

/* FUNCTION NAME:   sfp_port_is_pureComboSerdesPort
 * PURPOSE:
 *      Check if the port is a pure COMBO serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a pure COMBO serdes port
 *      FALSE            --  The port is not a pure COMBO serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_pureComboSerdesPort(
    const UI32_T unit,
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);

    return (NULL == ptr_portSettings) ? FALSE : (SFP_CONFIG_PORT_TYPE_COMBO_SFP == ptr_portSettings->port_type);
}

/* FUNCTION NAME:   sfp_port_is_comboSerdesPort
 * PURPOSE:
 *      Check if the port is a COMBO serdes port.
 *
 * INPUT:
 *      unit             --  Device unit number
 *      port             --  Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             --  The port is a COMBO serdes port
 *      FALSE            --  The port is not a COMBO serdes port
 *
 * NOTES:
 *      None
 */
BOOL_T
sfp_port_is_comboSerdesPort(
    const UI32_T unit,
    const UI32_T port)
{
    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        MW_ERROR_NO_T ret = MW_E_OTHERS;
        AIR_PORT_COMBO_MODE_T combo_mode;

        ret = air_port_getComboMode(unit, port, &combo_mode);
        if ((MW_E_OK == ret) && (AIR_PORT_COMBO_MODE_SERDES == combo_mode))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* FUNCTION NAME:   sfp_port_checkInsidePHYReady
 * PURPOSE:
 *      Check if there is an accessible inside PHY within an SFP module.
 *
 * INPUT:
 *      unit                -- Device unit number
 *      port                -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- There is an accessible inside PHY
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_NOT_SUPPORT    -- Operation is not supported
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_OTHERS         -- Other errors
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_port_checkInsidePHYReady(
    const UI32_T unit,
    const UI32_T port)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;
    SFP_MODULE_PORT_STATE_T state = SFP_MODULE_PORT_STATE_LAST;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    I32_T ret = MW_E_OTHERS;

    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboSerdesPort(unit, port)))
    {
        /* PHY ports or COMBO PHY ports */
        return MW_E_NOT_SUPPORT;
    }

    ptr_port_info = sfp_task_getPortInfo(port);
    if (NULL == ptr_port_info)
    {
        return MW_E_BAD_PARAMETER;
    }

    if (TRUE != sfp_pin_checkInitState(ptr_port_info->pin_state, (SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED)))
    {
        /* SFP auto adaption */
        return MW_E_NOT_SUPPORT;
    }

    sfp_module_state_getState(port, &state);
    if (SFP_MODULE_PORT_STATE_ACTIVE != state)
    {
        /* SERDES mode is not decided for (COMBO) SFP ports */
        return MW_E_OP_INVALID;
    }

    ret = air_port_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_E_OK != ret)
    {
        return MW_E_OTHERS;
    }

    if ((AIR_PORT_SERDES_MODE_SGMII == serdes_mode) &&
        (SFP_SFF_INVALID_ADDRESS != ptr_port_info->phy_2wire_addr))
    {
        return MW_E_OK;
    }

    return MW_E_NOT_SUPPORT;
}

/* FUNCTION NAME:   sfp_port_checkPinInitState
 * PURPOSE:
 *      Check the PIN initialization state of a port to obtain the initialization
 *      status.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port index
 *      flags                -- The states to be checked
 *                              SFP_PIN_INIT_STATE_T
 * OUTPUT:
 *      ptr_state            -- A pointer returns the PIN initialization state
 * RETURN:
 *      MW_E_OK              -- The states are set
 *      Others               -- The states are not set
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_port_checkPinInitState(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T flags,
    BOOL_T *ptr_state)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;

    if (NULL == ptr_state)
    {
        return MW_E_BAD_PARAMETER;
    }

    *ptr_state = FALSE;

    ptr_port_info = sfp_task_getPortInfo(port);
    if (NULL == ptr_port_info)
    {
        return MW_E_BAD_PARAMETER;
    }

    *ptr_state = sfp_pin_checkInitState(ptr_port_info->pin_state, flags);
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_port_setComboMode
 * PURPOSE:
 *      Set the COMBO mode to PHY or SERDES for a specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      admin_state              -- Current admin state of the port
 *      keep_serdesMACEnable     -- Keep COMBO SERDES enable or not when changing
 *                                  to COMBO PHY mode
 *      combo_mode               -- The COMBO mode to set
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                  -- Operation success.
 *      MW_E_BAD_PARAMETER       -- Parameter is wrong.
 *      MW_E_OP_INVALID          -- Operation is invalid.
 * NOTES:
 *      Only support the COMBO ports.
 *      To avoid the COMBO port from being link up between its PHY port and SERDES port, COMBO
 *      PHY need be disabled before changing to COMBO SERDES. However, for SFP auto
 *      adaption, COMBO SERDES need keep enabled even after changing to COMBO PHY to read
 *      MAC registers.
 */
MW_ERROR_NO_T
sfp_port_setComboMode(
    UI32_T unit,
    UI32_T port,
    BOOL_T admin_state,
    BOOL_T keep_serdesMACEnable,
    AIR_PORT_COMBO_MODE_T combo_mode)
{
    AIR_PORT_COMBO_MODE_T old_combo_mode = AIR_PORT_COMBO_MODE_LAST;
    I32_T ret = AIR_E_OK;

    if (FALSE == sfp_port_is_comboPort(unit, port))
    {
        return MW_E_OP_INVALID;
    }

    air_port_getComboMode(unit, port, &old_combo_mode);
    SFP_LOG_DEBUG("port:%d Change combo_mode from %d to %d. admin_state:%d SerdesMACEnable:%d",
              port, old_combo_mode, combo_mode, admin_state, keep_serdesMACEnable);

    if (AIR_PORT_COMBO_MODE_SERDES == combo_mode)
    {
        if (AIR_PORT_COMBO_MODE_SERDES != old_combo_mode)
        {
            if (TRUE == admin_state)
            {
                /* Disable PHY MAC */
                air_port_setAdminState(unit, port, FALSE);
            }
            ret = air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_SERDES);
            if (TRUE == admin_state)
            {
                /* Enable SERDES MAC */
                air_port_setAdminState(unit, port, TRUE);
            }
            /* Only SERDES MAC eanbled if admin_state is TRUE. */
        }
    }
    else if (AIR_PORT_COMBO_MODE_PHY == combo_mode)
    {
        /* Keep SERDES MAC enabled for SFP auto adaption. For SFP port with I2C, it will disable
         * MAC first when SFP modules is detected. Therefore, it will not be affected.
         */
        if ((TRUE == keep_serdesMACEnable) && (TRUE == admin_state))
        {
            air_port_setAdminState(unit, port, FALSE);

            /* Enable both PHY MAC and SERDES MAC. */
            air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_SERDES);
            air_port_setAdminState(unit, port, TRUE);
            ret = air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_PHY);
            air_port_setAdminState(unit, port, TRUE);

            SFP_LOG_DEBUG("port:%d Open both SERDES MAC and PHY MAC!", port);

            return ret;
        }

        if (AIR_PORT_COMBO_MODE_PHY != old_combo_mode)
        {
            if (TRUE == admin_state)
            {
                /* Disable SERDES MAC */
                air_port_setAdminState(unit, port, FALSE);
            }
            ret = air_port_setComboMode(unit, port, AIR_PORT_COMBO_MODE_PHY);
            if (TRUE == admin_state)
            {
                /* Enable PHY MAC */
                air_port_setAdminState(unit, port, TRUE);
            }

            SFP_LOG_DEBUG("port:%d Open PHY MAC only! admin_state:%d", port, admin_state);
        }
    }

    return ret;
}

MW_ERROR_NO_T
sfp_cmd_startSFP(
    void)
{
    SFP_MSG_T *ptr_message = NULL;
    MW_ERROR_NO_T ret = MW_E_NO_MEMORY;

    ptr_message = sfp_msg_create(0, MW_MSG_ID_SFP_CMD_START_SFP_REQ, 0);
    if (NULL != ptr_message)
    {
        ret = sfp_msg_send(ptr_message);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_message);
        }
    }

    return ret;
}

MW_ERROR_NO_T
sfp_cmd_stopSFP(
    void)
{
    SFP_MSG_T *ptr_message = NULL;
    MW_ERROR_NO_T ret = MW_E_NO_MEMORY;

    ptr_message = sfp_msg_create(0, MW_MSG_ID_SFP_CMD_STOP_SFP_REQ, 0);
    if (NULL != ptr_message)
    {
        ret = sfp_msg_send(ptr_message);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_message);
        }
    }

    return ret;
}

