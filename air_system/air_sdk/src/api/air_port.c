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

/* FILE NAME:  air_port.c
 * PURPOSE:
 *    It provide Port module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_port.h>

#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal.h>

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PORT, "air_port.c");

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_port_setSerdesMode
 * PURPOSE:
 *      Set the SGMII mode for SGMII or 1000BASE_X
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the serdes mode
 *                                  AIR_PORT_SERDES_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Speed will change to 1000M, when serdes mode set to 1000BASE_X.
 */
AIR_ERROR_NO_T
air_port_setSerdesMode(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_XSGMII_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(mode, AIR_PORT_SERDES_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setSerdesMode, (unit, port, mode));
}

/* FUNCTION NAME:   air_port_getSerdesMode
 * PURPOSE:
 *      Get the SGMII mode
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the serdes mode
 *                                  AIR_PORT_SERDES_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getSerdesMode(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_XSGMII_PORT(unit, port);
    HAL_CHECK_PTR(ptr_mode);

    return HAL_FUNC_CALL(unit, port, getSerdesMode, (unit, port, ptr_mode));
}

/* FUNCTION NAME:   air_port_setPhyAutoNego
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port
 *      (Auto or Forced)
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      auto_nego                -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyAutoNego(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(auto_nego, AIR_PORT_PHY_AN_LAST);

    return HAL_FUNC_CALL(unit, port, setPhyAutoNego, (unit, port, auto_nego));
}

/* FUNCTION NAME:   air_port_getPhyAutoNego
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_auto_nego            -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyAutoNego(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_auto_nego);

    return HAL_FUNC_CALL(unit, port, getPhyAutoNego, (unit, port, ptr_auto_nego));
}

/* FUNCTION NAME:   air_port_setPhyLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyLocalAdvAbility(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_adv);

    return HAL_FUNC_CALL(unit, port, setPhyLocalAdvAbility, (unit, port, ptr_adv));
}

/* FUNCTION NAME:   air_port_getPhyLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyLocalAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_adv);

    return HAL_FUNC_CALL(unit, port, getPhyLocalAdvAbility, (unit, port, ptr_adv));
}

/* FUNCTION NAME:   air_port_getPhyRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a partner
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyRemoteAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_adv);

    return HAL_FUNC_CALL(unit, port, getPhyRemoteAdvAbility, (unit, port, ptr_adv));
}

/* FUNCTION NAME:   air_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port. This setting is used on
 *      force mode only
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      speed                    -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSpeed(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(speed, AIR_PORT_SPEED_LAST);

    return HAL_FUNC_CALL(unit, port, setSpeed, (unit, port, speed));
}

/* FUNCTION NAME:   air_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_speed                -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpeed(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_speed);

    return HAL_FUNC_CALL(unit, port, getSpeed, (unit, port, ptr_speed));
}

/* FUNCTION NAME:   air_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port. This setting is used on
 *      force mode only
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      duplex                   -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(duplex, AIR_PORT_DUPLEX_LAST);

    return HAL_FUNC_CALL(unit, port, setDuplex, (unit, port, duplex));
}

/* FUNCTION NAME:   air_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_duplex               -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getDuplex(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_duplex);

    return HAL_FUNC_CALL(unit, port, getDuplex, (unit, port, ptr_duplex));
}

/* FUNCTION NAME:   air_port_setBackPressure
 * PURPOSE:
 *      Set the back pressure configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setBackPressure(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, port, setBackPressure, (unit, port, enable));
}

/* FUNCTION NAME:   air_port_getBackPressure
 * PURPOSE:
 *      Get the back pressure configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getBackPressure(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, port, getBackPressure, (unit, port, ptr_enable));
}

/* FUNCTION NAME:   air_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port direction
 *                                  AIR_PORT_DIR_T
 *      fc_en                    -- TRUE: Enable select port flow
 *                                        control
 *                                  FALSE:Disable select port flow
 *                                        control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(dir, AIR_PORT_DIR_LAST);
    HAL_CHECK_BOOL(fc_en);

    return HAL_FUNC_CALL(unit, port, setFlowCtrl, (unit, port, dir, fc_en));
}

/* FUNCTION NAME:   air_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port direction
 *                                  AIR_PORT_DIR_T
 * OUTPUT:
 *      ptr_fc_en                -- FALSE: Port flow control disable
 *                                  TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(dir, AIR_PORT_DIR_LAST);
    HAL_CHECK_PTR(ptr_fc_en);

    return HAL_FUNC_CALL(unit, port, getFlowCtrl, (unit, port, dir, ptr_fc_en));
}

/* FUNCTION NAME:   air_port_setPhyLoopBack
 * PURPOSE:
 *      Set the loop back configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port loopback
 *                                  direction
 *                                  AIR_PORT_LPBK_DIR_T
 *      enable                   -- FALSE:Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(dir, AIR_PORT_LPBK_DIR_LAST);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, port, setPhyLoopBack, (unit, port, dir, enable));
}

/* FUNCTION NAME:   air_port_getPhyLoopBack
 * PURPOSE:
 *      Get the loop back configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port loopback
 *                                  direction
 *                                  AIR_PORT_LPBK_DIR_T
 * OUTPUT:
 *      ptr_enable               -- FALSE:Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(dir, AIR_PORT_LPBK_DIR_LAST);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, port, getPhyLoopBack, (unit, port, dir, ptr_enable));
}

/* FUNCTION NAME:   air_port_getPortStatus
 * PURPOSE:
 *      Get the physical link status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_status               -- Strcut of the port status
 *                                  AIR_PORT_STATUS_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_status)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_status);

    if (!HAL_IS_XSGMII_PORT_VALID(unit, port) && (HAL_CPU_PORT(unit) == port))
    {
        return HAL_FUNC_CALL(unit, port, getPortStatus, (unit, port, ptr_status));
    }
    return HAL_FUNC_CALL(unit, ifmon, getPortStatus, (unit, port, ptr_status));
}

/* FUNCTION NAME:   air_port_setAdminState
 * PURPOSE:
 *      Set the port status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- TRUE: port enable
 *                                  FALSE: port disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, port, setAdminState, (unit, port, enable));
}

/* FUNCTION NAME:   air_port_getAdminState
 * PURPOSE:
 *      Get the port status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- TRUE: port enable
 *                                  FALSE: port disable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, port, getAdminState, (unit, port, ptr_enable));
}

/* FUNCTION NAME:   air_port_setPhySmartSpeedDown
 * PURPOSE:
 *      Set Smart speed down feature for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ssd_mode                 -- Struct of the port SSD mode
 *                                  AIR_PORT_SSD_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhySmartSpeedDown(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T ssd_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(ssd_mode, AIR_PORT_SSD_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setPhySmartSpeedDown, (unit, port, ssd_mode));
}

/* FUNCTION NAME:   air_port_getPhySmartSpeedDown
 * PURPOSE:
 *      Get Smart speed down feature for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_ssd_mode             -- Struct of the port SSD mode
 *                                  AIR_PORT_SSD_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhySmartSpeedDown(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_ssd_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_ssd_mode);

    return HAL_FUNC_CALL(unit, port, getPhySmartSpeedDown, (unit, port, ptr_ssd_mode));
}

/* FUNCTION NAME:   air_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      port_bitmap              -- Matrix port bitmap
                                    AIR_PORT_BITMAP_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPortMatrix(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PORT_BITMAP(unit, port_bitmap);

    return HAL_FUNC_CALL(unit, port, setPortMatrix, (unit, port, port_bitmap));
}

/* FUNCTION NAME:   air_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      port_bitmap              -- Matrix port bitmap
 *                                  AIR_PORT_BITMAP_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPortMatrix(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(port_bitmap);

    return HAL_FUNC_CALL(unit, port, getPortMatrix, (unit, port, port_bitmap));
}

/* FUNCTION NAME:   air_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the port vlan mode
 *                                  AIR_PORT_VLAN_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setVlanMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(mode, AIR_PORT_VLAN_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setVlanMode, (unit, port, mode));
}

/* FUNCTION NAME:   air_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the port vlan mode
                                    AIR_PORT_VLAN_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getVlanMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *ptr_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_mode);

    return HAL_FUNC_CALL(unit, port, getVlanMode, (unit, port, ptr_mode));
}

/* FUNCTION NAME:   air_port_setPhyLedOnCtrl
 * PURPOSE:
 *      Set LED state for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      enable                   -- FALSE:Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, port, setPhyLedOnCtrl, (unit, port, led_id, enable));
}

/* FUNCTION NAME:   air_port_getPhyLedOnCtrl
 * PURPOSE:
 *      Get LED state for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_enable               -- FALSE:Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, port, getPhyLedOnCtrl, (unit, port, led_id, ptr_enable));
}

/* FUNCTION NAME:   air_port_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      combo_mode               -- Struct of the port combo mode
 *                                  AIR_PORT_COMBO_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setComboMode(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T combo_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(combo_mode, AIR_PORT_COMBO_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setComboMode, (unit, port, combo_mode));
}

/* FUNCTION NAME:   air_port_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_combo_mode           -- Struct of the port combo
 *                                  mode
 *                                  AIR_PORT_COMBO_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getComboMode(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_combo_mode);

    return HAL_FUNC_CALL(unit, port, getComboMode, (unit, port, ptr_combo_mode));
}
/* FUNCTION NAME:   air_port_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      ctrl_mode                -- LED control mode enumeration type
 *                                  AIR_PORT_PHY_LED_CTRL_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedCtrlMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(ctrl_mode, AIR_PORT_PHY_LED_CTRL_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setPhyLedCtrlMode, (unit, port, led_id, ctrl_mode));
}

/* FUNCTION NAME:   air_port_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode            -- LED control enumeration type
 *                                  AIR_PORT_PHY_LED_CTRL_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_ctrl_mode);

    return HAL_FUNC_CALL(unit, port, getPhyLedCtrlMode, (unit, port, led_id, ptr_ctrl_mode));
}

/* FUNCTION NAME:   air_port_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      state                    -- LED force state
 *                                  AIR_PORT_PHY_LED_STATE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedForceState(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   led_id,
    const AIR_PORT_PHY_LED_STATE_T state)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(state, AIR_PORT_PHY_LED_STATE_LAST);

    return HAL_FUNC_CALL(unit, port, setPhyLedForceState, (unit, port, led_id, state));
}

/* FUNCTION NAME:   air_port_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_state                -- LED force state enumeration type
 *                                  AIR_PORT_PHY_LED_STATE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    AIR_PORT_PHY_LED_STATE_T *ptr_state)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_state);

    return HAL_FUNC_CALL(unit, port, getPhyLedForceState, (unit, port, led_id, ptr_state));
}

/* FUNCTION NAME:   air_port_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      pattern                  -- LED force pattern
 *                                  AIR_PORT_PHY_LED_PATT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedForcePattCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const AIR_PORT_PHY_LED_PATT_T pattern)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(pattern, AIR_PORT_PHY_LED_PATT_LAST);

    return HAL_FUNC_CALL(unit, port, setPhyLedForcePattCfg, (unit, port, led_id, pattern));
}

/* FUNCTION NAME:   air_port_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_pattern              -- LED force pattern enumeration type
 *                                  AIR_PORT_PHY_LED_PATT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    AIR_PORT_PHY_LED_PATT_T *ptr_pattern)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_pattern);

    return HAL_FUNC_CALL(unit, port, getPhyLedForcePattCfg, (unit, port, led_id, ptr_pattern));
}

/* FUNCTION NAME:   air_port_triggerCableTest
 * PURPOSE:
 *      Trigger port cable status.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      test_pair                -- Tested cable pair
 *                                  AIR_PORT_CABLE_TEST_PAIR_T
 * OUTPUT:
 *      ptr_test_rslt            -- Cable diagnostic information
 *                                  AIR_PORT_CABLE_TEST_RSLT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
air_port_triggerCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_test_rslt);

    return HAL_FUNC_CALL(unit, port, triggerCableTest, (unit, port, test_pair, ptr_test_rslt));
}

/* FUNCTION NAME:   air_port_setPhyOpMode
 * PURPOSE:
 *      Set Phy operation mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Phy operation mode
 *                                  AIR_PORT_OP_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyOpMode(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_OP_MODE_T mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(mode, AIR_PORT_OP_MODE_LAST);

    return HAL_FUNC_CALL(unit, port, setPhyOpMode, (unit, port, mode));
}

/* FUNCTION NAME:   air_port_getPhyOpMode
 * PURPOSE:
 *      Get Phy operation mode0
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_mode                 -- Phy operation mode enumeration type
 *                                  AIR_PORT_OP_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
air_port_getPhyOpMode(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_OP_MODE_T *ptr_mode)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_mode);

    return HAL_FUNC_CALL(unit, port, getPhyOpMode, (unit, port, ptr_mode));
}

/* FUNCTION NAME:   air_port_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_test_rslt            -- Cable diagnostic information
 *                                  AIR_PORT_CABLE_TEST_RSLT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Support cable diagnostic in link down mode only.
 */
AIR_ERROR_NO_T
air_port_triggerLinkDownCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_test_rslt);
    return HAL_FUNC_CALL(unit, port, triggerLinkDownCableTest, (unit, port, ptr_test_rslt));
}
