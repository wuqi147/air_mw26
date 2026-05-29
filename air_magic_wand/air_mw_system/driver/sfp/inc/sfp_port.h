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

/* FILE NAME:   sfp_port.h
 * PURPOSE:
 *      Provide API to Access the inside PHY of the SFP module.
 * NOTES:
 *
 */

#ifndef SFP_PORT_H
#define SFP_PORT_H
/* INCLUDE FILE DECLARATIONS
 */
#include "air_error.h"
#include "air_port.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const BOOL_T enable);

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
    BOOL_T *ptr_enable);

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
    const AIR_PORT_PHY_AN_T auto_nego);

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
    AIR_PORT_PHY_AN_T   *ptr_auto_nego);

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
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv);

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
    AIR_PORT_PHY_AN_ADV_T   *ptr_adv);

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
    AIR_PORT_PHY_AN_ADV_T   *ptr_adv);

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
    const AIR_PORT_SPEED_T  speed);

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
    AIR_PORT_SPEED_T    *ptr_speed);

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
    const AIR_PORT_DUPLEX_T duplex);

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
    AIR_PORT_DUPLEX_T   *ptr_duplex);

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
    const BOOL_T    enable);

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
    BOOL_T          *ptr_enable);

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
    const BOOL_T            fc_en);

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
    BOOL_T                  *ptr_fc_en);

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
    AIR_PORT_STATUS_T   *ptr_status);
#endif /* End of SFP_PORT_H */

