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

/* FILE NAME:   sfp_module_inside_phy.h
 * PURPOSE:
 *      Access the inside PHY of the SFP module.
 * NOTES:
 *
 */

#ifndef SFP_MODULE_INSIDE_PHY_H
#define SFP_MODULE_INSIDE_PHY_H
/* INCLUDE FILE DECLARATIONS
 */
#include "air_cfg.h"
#include "air_port.h"
#include "air_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_MODULE_INSIDE_PHY_UPDATE_MAC_DELAY_COUNT (0)
#define SFP_MODULE_INSIDE_PHY_UPDATE_MAC_DELAY_MS (SFP_MODULE_INSIDE_PHY_UPDATE_MAC_DELAY_COUNT * SFP_TASK_TIMER_BASE_INTERVAL_MS)

#define SFP_PHY_MII_MMD_CTRL            (0x0d)    /* MMD Access Control Register      */
#define SFP_PHY_MII_MMD_DATA            (0x0e)    /* MMD Access Data Register         */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct SFP_PHY_LINK_STATUS_S
{
#define SFP_PHY_LINK_STATUS_FLAGS_LINK_UP           (1U << 0)
#define SFP_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE    (1U << 1)
#define SFP_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT      (1U << 2)
    UI32_T              flags;
    AIR_PORT_SPEED_T    speed;
    AIR_PORT_DUPLEX_T   duplex;
}SFP_PHY_LINK_STATUS_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   sfp_phy_setAdminState
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   sfp_phy_getAdminState
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_enable);

/* FUNCTION NAME:   sfp_phy_setAutoNego
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_setAutoNego(
    const UI32_T unit,
    const UI32_T port,
    const AIR_PORT_PHY_AN_T auto_nego);

/* FUNCTION NAME:   sfp_phy_getAutoNego
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getAutoNego(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego);

/* FUNCTION NAME:   sfp_phy_setLocalAdvAbility
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_setLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   sfp_phy_getLocalAdvAbility
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   sfp_phy_getRemoteAdvAbility
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getRemoteAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   sfp_phy_setSpeed
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_setSpeed(
    const UI32_T unit,
    const UI32_T port,
    const AIR_PORT_SPEED_T speed);

/* FUNCTION NAME:   sfp_phy_getSpeed
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getSpeed(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SPEED_T *ptr_speed);

/* FUNCTION NAME:   sfp_phy_setDuplex
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_setDuplex(
    const UI32_T unit,
    const UI32_T port,
    const AIR_PORT_DUPLEX_T duplex);

/* FUNCTION NAME:   sfp_phy_getDuplex
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
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getDuplex(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   sfp_phy_getLinkStatus
 * PURPOSE:
 *      Get the port link status.
 *
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *
 * OUTPUT:
 *      ptr_status      --  Link Status
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
AIR_ERROR_NO_T
sfp_phy_getLinkStatus(
    const UI32_T unit,
    const UI32_T port,
    SFP_PHY_LINK_STATUS_T *ptr_status);

/* FUNCTION NAME:   sfp_phy_init
 * PURPOSE:
 *      Initialize the context of SFP PHY handling.
 *
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK        -- Operation success
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_phy_init(
    const UI32_T unit);

/* FUNCTION NAME:   sfp_phy_deinit
 * PURPOSE:
 *      Deinitialize the context of SFP PHY handling.
 *
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK        -- Operation success
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_phy_deinit(
    const UI32_T unit);

/* FUNCTION NAME:   sfp_phy_linkStatusChangeCallback
 * PURPOSE:
 *      It is the callback for the link status change. It will be invoked when
 *      the link status of a port changes.
 *
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Port number
 *      link            -- link status: TRUE - link up
 *                                      FALSE - link down
 *      ptr_cookie      --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_phy_linkStatusChangeCallback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void *ptr_cookie);

/* FUNCTION NAME:   sfp_phy_pollingLinkStatus
 * PURPOSE:
 *      Poll the link status for each SERDES port or COMBO SERDES port. The
 *      polling interval is SFP_TASK_TIMER_3BASE_INTERVAL_MS.
 *
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_phy_pollingLinkStatus(
    const UI32_T unit);

/* FUNCTION NAME:   sfp_phy_compensateLinkDown
 * PURPOSE:
 *      Compensate a link down event for a port before its COMBO/SERDES mode changes.
 *      This is needed because link down may not be detected when changing
 *      COMBO/SERDES mode.
 *
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_phy_compensateLinkDown(
    UI32_T unit,
    UI32_T port);
#endif /* End of SFP_MODULE_INSIDE_PHY_H */
