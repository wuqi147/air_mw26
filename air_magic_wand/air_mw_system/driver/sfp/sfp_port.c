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

/* FILE NAME:   sfp_port.c
 * PURPOSE:
 *      Provide API to Access the inside PHY of the SFP module.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "air_port.h"
#include "air_error.h"
#include "mw_utils.h"
#include "sfp_util.h"
#include "sfp_port.h"
#include "sfp_module_inside_phy.h"
#include "sfp_pin.h"
#include "sfp_task.h"
#include "sfp_module_handle.h"
#include "sfp_auto_adaptation.h"

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_port_setAdminState
 * PURPOSE:
 *      Set the port status for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- TRUE: port enable
 *                                  FALSE: port disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    I32_T ret = AIR_E_OP_INCOMPLETE;
    BOOL_T pin_init_state = FALSE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        ret = air_port_setAdminState(unit, port, enable);
    }
    else
    {
        ret = sfp_phy_setAdminState(unit, port, enable);
    }

    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboPort(unit, port)))
    {
        sfp_port_checkPinInitState(unit, port, (SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED), &pin_init_state);
        if (FALSE == pin_init_state)
        {
            /* SFP auto adaption */
            if ((AIR_E_OK == ret) && (TRUE == enable) &&
                (TRUE == sfp_port_is_comboPort(unit, port)))
            {
                AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;

                ret = air_port_getComboMode(unit, port, &combo_mode);
                if (AIR_E_OK != ret)
                {
                    /* Admin state is set successfully. */
                    SFP_LOG_WARN("COMBO serdes may be disabled.");
                    return AIR_E_OK;
                }
                /* Enable serdes MAC. */
                sfp_port_setComboMode(unit, port, TRUE, TRUE, combo_mode);
            }
        }
    }

    SFP_LOG_DEBUG("%s port:%d enable:%d ret:%d", __func__, port, enable, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getAdminState
 * PURPOSE:
 *      Get the port status for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- TRUE: port enable
 *                                  FALSE: port disable
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_enable)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getAdminState(unit, port, ptr_enable);
    }
    else
    {
        return sfp_phy_getAdminState(unit, port, ptr_enable);
    }
}

/* FUNCTION NAME:   sfp_port_setPhyAutoNego
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port
 *      (Auto or Forced).
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      auto_nego                -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setPhyAutoNego(
    const UI32_T unit,
    const UI32_T port,
    const AIR_PORT_PHY_AN_T auto_nego)
{
    I32_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        ret = air_port_setPhyAutoNego(unit, port, auto_nego);
    }
    else
    {
        ret = sfp_phy_setAutoNego(unit, port, auto_nego);
    }

    SFP_LOG_DEBUG("%s, port:%d AN:%d ret:%d", __func__, port, auto_nego, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getPhyAutoNego
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_auto_nego            -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getPhyAutoNego(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_PHY_AN_T   *ptr_auto_nego)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getPhyAutoNego(unit, port, ptr_auto_nego);
    }
    else
    {
        return sfp_phy_getAutoNego(unit, port, ptr_auto_nego);
    }
}

/* FUNCTION NAME:   sfp_port_setPhyLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ptr_adv                  -- Struct of the AN advertisement setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setPhyLocalAdvAbility(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    I32_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        ret = air_port_setPhyLocalAdvAbility(unit, port, ptr_adv);
    }
    else
    {
        ret = sfp_phy_setLocalAdvAbility(unit, port, ptr_adv);
    }

    SFP_LOG_DEBUG("%s, port:%d flags:0x%x ret:%d", __func__, port, ptr_adv->flags, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getPhyLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getPhyLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_PHY_AN_ADV_T   *ptr_adv)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getPhyLocalAdvAbility(unit, port, ptr_adv);
    }
    else
    {
        return sfp_phy_getLocalAdvAbility(unit, port, ptr_adv);
    }
}

/* FUNCTION NAME:   sfp_port_getPhyRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a partner.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getPhyRemoteAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_PHY_AN_ADV_T   *ptr_adv)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getPhyRemoteAdvAbility(unit, port, ptr_adv);
    }
    else
    {
        return sfp_phy_getRemoteAdvAbility(unit, port, ptr_adv);
    }
}

/* FUNCTION NAME:   sfp_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port. This setting is used on force mode only.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      speed                    -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setSpeed(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_SPEED_T  speed)
{
    I32_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        ret = air_port_setSpeed(unit, port, speed);
    }
    else
    {
        ret = sfp_phy_setSpeed(unit, port, speed);
    }

    SFP_LOG_DEBUG("%s, port:%d speed:0x%x ret:%d", __func__, port, speed, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_speed                -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getSpeed(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_SPEED_T    *ptr_speed)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getSpeed(unit, port, ptr_speed);
    }
    else
    {
        return sfp_phy_getSpeed(unit, port, ptr_speed);
    }
}

/* FUNCTION NAME:   sfp_port_setDuplex
 * PURPOSE:
 *      Set the duplex for a specific port. This setting is used on force mode only.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      duplex                   -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex)
{
    I32_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        ret = air_port_setDuplex(unit, port, duplex);
        if (AIR_E_NOT_SUPPORT == ret)
        {
            ret = AIR_E_OK;
        }
    }
    else
    {
        ret = sfp_phy_setDuplex(unit, port, duplex);
    }

    SFP_LOG_DEBUG("%s, port:%d duplex:0x%x ret:%d", __func__, port, duplex, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_duplex               -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getDuplex(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_DUPLEX_T   *ptr_duplex)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return air_port_getDuplex(unit, port, ptr_duplex);
    }
    else
    {
        return sfp_phy_getDuplex(unit, port, ptr_duplex);
    }
}

/* FUNCTION NAME:   sfp_port_setBackPressure
 * PURPOSE:
 *      Set the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setBackPressure(
    const UI32_T    unit,
    const UI32_T    port,
    const BOOL_T    enable)
{
    AIR_ERROR_NO_T ret = AIR_E_OP_INCOMPLETE;

    ret = air_port_setBackPressure(unit, port, enable);

    SFP_LOG_DEBUG("%s, port:%d enable:0x%x ret:%d", __func__, port, enable, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getBackPressure
 * PURPOSE:
 *      Get the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getBackPressure(
    const UI32_T    unit,
    const UI32_T    port,
    BOOL_T          *ptr_enable)
{
    return air_port_getBackPressure(unit, port, ptr_enable);
}

/* FUNCTION NAME:   sfp_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port
 *                                  direction
 *                                  AIR_PORT_DIR_T
 *      fc_en                    -- TRUE: Enable select port flow
 *                                        control
 *                                  FALSE:Disable select port flow
 *                                        control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_setFlowCtrl(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DIR_T    dir,
    const BOOL_T            fc_en)
{
    AIR_ERROR_NO_T ret = AIR_E_OP_INCOMPLETE;

    ret = air_port_setFlowCtrl(unit, port, dir, fc_en);

    SFP_LOG_DEBUG("%s, port:%d fc_en:0x%x ret:%d", __func__, port, fc_en, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port direction
 *                                  AIR_PORT_DIR_T
 * OUTPUT:
 *      ptr_fc_en                -- FALSE: Port flow control disable
 *                                  TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getFlowCtrl(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DIR_T    dir,
    BOOL_T                  *ptr_fc_en)
{
    return air_port_getFlowCtrl(unit, port, dir, ptr_fc_en);
}

/* FUNCTION NAME:   sfp_port_getPortStatus
 * PURPOSE:
 *      Get the physical link status for a specific port.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_status               -- Strcut of the port status
 *                                  AIR_PORT_STATUS_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO port.
 */
AIR_ERROR_NO_T
sfp_port_getPortStatus(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_STATUS_T   *ptr_status)
{
    I32_T ret = AIR_E_OK;
    BOOL_T pin_init_state = FALSE;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    osapi_memset(ptr_status, 0, sizeof(AIR_PORT_STATUS_T));

    if ((TRUE != sfp_port_is_serdesPort(unit, port)) &&
        (TRUE != sfp_port_is_comboSerdesPort(unit, port)))
    {
        /* PHY port or COMBO PHY port */
        return air_port_getPortStatus(unit, port, ptr_status);
    }

    ret = air_port_getSerdesMode(unit, port, &serdes_mode);
    if ((AIR_E_OK == ret) && (AIR_PORT_SERDES_MODE_100BASE_FX == serdes_mode))
    {
        /* It is always link up at the MAC side if serdes mode is set to 100base-fx.
         * RX_LOS is required to support 100base-fx. Therefore, if RX_LOS is HIGH,
         * return link down for 100base-fx.
         */
        UI8_T rx_los_pin = SFP_PIN_PIONUM_INVALID;
        I32_T data = 0xFF;

        ret = sfp_pin_io_getPIONum(unit, port, SFP_PIN_TYPE_RX_LOSS, &rx_los_pin);
        ret |= sfp_pin_io_getValue(unit, port, rx_los_pin, &data);
        if ((MW_E_OK == ret) && (SFP_MODULE_PIN_HIGH == data))
        {
            /* Link down for 100base-fx when RX_LOS pin is HIGH. */
            return AIR_E_OK;
        }

        if (MW_E_OK != ret)
        {
            SFP_LOG_WARN("RX_LOS pin does not exist for 100BASE_FX. port:%d", port);
        }
    }

    sfp_port_checkPinInitState(unit, port, (SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED), &pin_init_state);
    if (FALSE == pin_init_state)
    {
        /* SFP auto adaption */
        SFP_TASK_PORT_ADAPTION_STATE_T auto_state = SFP_TASK_PORT_ADAPTION_STATE_LAST;
        UI32_T pcs_state = 0;

        sfp_auto_adaptation_getState(port, &auto_state);
        if (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != auto_state)
        {
            /* Link down if state is not ACTIVE. */
            return AIR_E_OK;
        }

        if (AIR_PORT_SERDES_MODE_100BASE_FX == serdes_mode)
        {
            sfp_adaption_readReg(unit, port, SFP_SERDES_PCS_STATE_REG_2, &pcs_state);
            if (0 != (SFP_SERDES_PCS_STATE_REG_2_BIT5_RXSYNC & pcs_state))
            {
                /* Link down for 100base-fx when RX_SYNC is set.  */
                return AIR_E_OK;
            }
        }

        return air_port_getPortStatus(unit, port, ptr_status);
    }
    else
    {
        /* SFP port with I2C */
        SFP_MODULE_PORT_STATE_T state = SFP_MODULE_PORT_STATE_LAST;

        sfp_module_state_getState(port, &state);
        if (SFP_MODULE_PORT_STATE_ACTIVE != state)
        {
            /* Link down if state is not ACTIVE. */
            return AIR_E_OK;
        }

        /* For SGMII Force mode, admin state of MAC will only be enabled when it is link up at the PHY side. */
        return air_port_getPortStatus(unit, port, ptr_status);
    }

    return ret;
}
