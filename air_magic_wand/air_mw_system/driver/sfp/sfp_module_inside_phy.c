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

/* FILE NAME:   sfp_module_inside_phy.c
 * PURPOSE:
 *      Access the inside PHY of the SFP module.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "cmlib_port.h"
#include "mw_utils.h"
#include "mw_platform.h"
#include "db_api.h"
#include "sfp_module_inside_phy.h"
#include "sfp_util.h"
#include "sfp_trunk.h"
#include "sfp_module_handle.h"
#include "sfp_auto_adaptation.h"
#include "port_utils.h"
#include "sfp_port.h"
#include "sfp_pin.h"
#include "sfp_db.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define GET_PHY_ABILITY(reg, cod, sb, result)    do         \
    {                                                       \
        if (0 != (reg & cod))                               \
        {                                                   \
            result |= sb;                                   \
        }                                                   \
    } while(0)

#define SET_PHY_ABILITY(flag, cod, sb, result)    do        \
    {                                                       \
        if (0 != (flag & cod))                              \
        {                                                   \
            result |= sb;                                   \
        }                                                   \
        else                                                \
        {                                                   \
            result &= ~(sb);                                \
        }                                                   \
    }while(0)

#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__) do                   \
        {                                                                                   \
            __base_addr__ = SERDES_ID_S0_BASE_ADDR + (SERDES_ADDR_OFFSER * __serdes_id__);  \
        }while(0)

#define SGMII_STS_CTRL_0                    (0x4018)
#define QSGMII_LAN_INDEX_MASK               (0x03)

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
static AIR_PORT_BITMAP_T _current_link_bitmap;

/* LOCAL SUBPROGRAM BODIES
 */
void
_sfp_phy_linkStatusChange_setMAC(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void *ptr_cookie)
{
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    AIR_PORT_PHY_AN_T auto_nego = AIR_PORT_PHY_AN_LAST, an_mode = AIR_PORT_PHY_AN_LAST;
    I32_T ret = AIR_E_OTHERS;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        return;
    }

    ret = air_port_getSerdesMode(unit, port, &serdes_mode);
    if (AIR_PORT_SERDES_MODE_SGMII != serdes_mode)
    {
        return;
    }

    if (FALSE == link)
    {
        /* Link down at the PHY side.
         * MAC need be disabled when it is link down at the PHY side. If not, air_port_getPortStatus()
         * will get wrong link up information before "link up at the PHY side" event is processed. */
        air_port_setAdminState(unit, port, FALSE);
    }
    else
    {
        /* Link up at the PHY side */
#ifndef AIR_EN_CORAL
        AIR_PORT_SPEED_T speed;
#endif
        AIR_PORT_DUPLEX_T duplex;
        DB_MSG_T *pptr_out_msg = NULL;
        UI16_T data_size = 0;
        UI8_T *ptr_data = NULL, flow_ctrl = 0xFF;
        SFP_PHY_LINK_STATUS_T link_status;
        UI16_T port_settings = 0;

        /* Get the PHY side information. */
        ret = sfp_phy_getLinkStatus(unit, port, &link_status);
#ifndef AIR_EN_CORAL
        speed = link_status.speed;
#endif
        duplex = link_status.duplex;
        ret |= sfp_port_getPhyAutoNego(unit, port, &auto_nego);
        ret |= sfp_db_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port, &pptr_out_msg, &data_size, (void **)&ptr_data);
        if (AIR_E_OK != ret)
        {
            MW_FREE(pptr_out_msg);
            return;
        }
        osapi_memcpy(&port_settings, ptr_data, sizeof(UI16_T));
        MW_FREE(pptr_out_msg);
        port_parsePortSettings(port_settings, NULL, NULL, &flow_ctrl);

        SFP_LOG_DEBUG("PHY side: port:%d speed:%d duplex:%d AN:%d FC set:%d", port, link_status.speed, duplex, auto_nego, ptr_data[0]);

        /* Set MAC */
        ret |= air_port_setAdminState(unit, port, FALSE);
#ifndef AIR_EN_CORAL
        ret |= air_port_setSpeed(unit, port, speed);
        ret |= air_port_setDuplex(unit, port, duplex);
#endif
        SFP_LOG_DEBUG("Set MAC: port:%d ret:%d line:%d", port, ret, __LINE__);
        if (AIR_PORT_DUPLEX_HALF == duplex)
        {
            /* Set the flow control as it is set by the upper layer when it is Half duplex. */
            ret |= air_port_setBackPressure(unit, port, (PORT_SETTINGS_FLOW_CTRL_ON == flow_ctrl));
        }
        else
        {
            /* Set the flow control as it is set by the upper layer when it is Full duplex and Force mode. */
            if (AIR_PORT_PHY_AN_DISABLE == auto_nego)
            {
                ret |= air_port_setFlowCtrl(unit, port, AIR_PORT_DIR_BOTH, (PORT_SETTINGS_FLOW_CTRL_ON == flow_ctrl));
            }
            else
            {
                /* AN mode && Full duplex */
                AIR_PORT_PHY_AN_ADV_T local_adv, remote_adv;
                BOOL_T fc_en = FALSE;

                ret = sfp_phy_getLocalAdvAbility(unit, port, &local_adv);
                ret |= sfp_phy_getRemoteAdvAbility(unit, port,&remote_adv);
                if (AIR_E_OK == ret)
                {
                    if ((0 != (local_adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE)) &&
                        (0 != (local_adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE)) &&
                        (0 != (remote_adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE)) &&
                        (0 != (remote_adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE)))
                    {
                        fc_en = TRUE;
                    }

                    ret = air_port_setFlowCtrl(unit, port, AIR_PORT_DIR_BOTH, fc_en);
                    SFP_LOG_DEBUG("Set MAC: port:%d ret:%d line:%d", port, ret, __LINE__);
                }
            }
        }

        SFP_LOG_DEBUG("Set MAC: port:%d ret:%d line:%d", port, ret, __LINE__);
#ifndef AIR_EN_CORAL
        /* SGMII uses serdes Force mode */
        ret |= air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_DISABLE);
#endif
        air_port_getPhyAutoNego(unit, port, &an_mode);
        SFP_LOG_DEBUG("Set MAC: port:%d ret:%d AN set:%d AN get:%d line:%d", port, ret, AIR_PORT_PHY_AN_DISABLE, an_mode, __LINE__);
        /* There's no need to check DB for admin state. If it is disabled, it should not link up. */
        ret |= air_port_setAdminState(unit, port, TRUE);
        SFP_LOG_DEBUG("Set MAC: port:%d ret:%d line:%d", port, ret, __LINE__);
    }
}

void
_sfp_phy_handlelinkStatusChange(
    const UI32_T unit,
    AIR_PORT_BITMAP_T new_link_bitmap)
{
    AIR_PORT_BITMAP_T changed_link_bitmap;
    AIR_PORT_BITMAP_T changed_port_bitmap;
    BOOL_T cur_link = FALSE;
    BOOL_T new_link = FALSE;
    SFP_TASK_PORT_INFO_T *ptr_port_info = NULL;
    UI32_T port = 0;

    if (AIR_PORT_BITMAP_EQUAL(new_link_bitmap, _current_link_bitmap))
    {
        /* No change. */
        return;
    }

    AIR_PORT_BITMAP_CLEAR(changed_link_bitmap);
    AIR_PORT_BITMAP_CLEAR(changed_port_bitmap);

    CMLIB_PORT_BITMAP_SET(changed_link_bitmap, _current_link_bitmap);
    CMLIB_PORT_BITMAP_XOR(changed_link_bitmap, new_link_bitmap);
    CMLIB_PORT_BITMAP_SET(changed_port_bitmap, changed_link_bitmap);

    /* Scan all port to handle link change and speed change */
    AIR_PORT_FOREACH(changed_port_bitmap, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }

        cur_link  = AIR_PORT_CHK(_current_link_bitmap, port) ? TRUE : FALSE;

        /* Get new status */
        new_link  = AIR_PORT_CHK(new_link_bitmap, port) ? TRUE : FALSE;

        /* Handle link change */
        if (new_link != cur_link)
        {
            if (TRUE == new_link)
            {
                /* Link up */
                SFP_LOG_DEBUG("Link up. Port:%d", port);
                AIR_PORT_ADD(_current_link_bitmap, port);
            }
            else
            {
                /* Link down */
                SFP_LOG_DEBUG("Link down. Port:%d", port);
                AIR_PORT_DEL(_current_link_bitmap, port);
            }

            ptr_port_info = sfp_task_getPortInfo(port);

            /* Currently, ptr_port_info->err_count is reused to indicate 1st link up after SFP module is inserted. */
            if ((FALSE == new_link) || (NULL == ptr_port_info) || (0 == ptr_port_info->err_count))
            {
                sfp_phy_linkStatusChangeCallback(unit, port, new_link, NULL);
            }
            else
            {
                /* Delay to update MAC for 1st link up after SFP module is inserted. */
                ptr_port_info->delay_count = SFP_MODULE_INSIDE_PHY_UPDATE_MAC_DELAY_COUNT;
                if (0 == ptr_port_info->delay_count)
                {
                    ptr_port_info->err_count = 0;
                    sfp_phy_linkStatusChangeCallback(unit, port, new_link, NULL);
                }
            }
        }
    }
}

AIR_ERROR_NO_T
_sfp_phy_checkLinkup(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_linkUp)
{
    I32_T ret = AIR_E_NOT_SUPPORT;

    if (NULL == ptr_linkUp)
    {
        return AIR_E_BAD_PARAMETER;
    }

    *ptr_linkUp = FALSE;

    ret = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != ret)
    {
        AIR_PORT_STATUS_T port_status = {0};

        ret = sfp_port_getPortStatus(unit, port, &port_status);
        if (AIR_E_OK != ret)
        {
            return ret;
        }

        if (0 != (AIR_PORT_STATUS_FLAGS_LINK_UP & port_status.flags))
        {
            *ptr_linkUp = TRUE;
        }
    }
    else
    {
        SFP_PHY_LINK_STATUS_T link_status = {0};

        ret = sfp_phy_getLinkStatus(unit, port, &link_status);
        if (AIR_E_OK != ret)
        {
            return ret;
        }

        if (0 != (SFP_PHY_LINK_STATUS_FLAGS_LINK_UP & link_status.flags))
        {
            *ptr_linkUp = TRUE;
        }
    }

    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
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
    const BOOL_T enable)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (TRUE == enable)
        {
            reg_data &= ~(BMCR_PDOWN);
        }
        else
        {
            reg_data |= BMCR_PDOWN;
        }

        rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

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
    BOOL_T *ptr_enable)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if ( 0!= (reg_data & BMCR_PDOWN))
        {
            *ptr_enable = FALSE;
        }
        else
        {
            *ptr_enable = TRUE;
        }
    }
    return rv;
}

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
    const AIR_PORT_PHY_AN_T auto_nego)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (AIR_PORT_PHY_AN_ENABLE == auto_nego)
        {
            reg_data |= BMCR_ANENABLE;
        }
        else if (AIR_PORT_PHY_AN_RESTART == auto_nego)
        {
            reg_data |= BMCR_ANRESTART;
        }
        else
        {
            reg_data &= ~(BMCR_ANENABLE);
        }

        rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

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
    AIR_PORT_PHY_AN_T *ptr_auto_nego)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (reg_data & BMCR_ANENABLE)
        {
            *ptr_auto_nego = AIR_PORT_PHY_AN_ENABLE;
        }
        else
        {
            *ptr_auto_nego = AIR_PORT_PHY_AN_DISABLE;
        }
    }
    return rv;
}

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
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);

    if (AIR_E_OK == rv)
    {
        SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX, ADVERTISE_1000FULL, reg_data);
        rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_CTRL1000, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ADVERTISE_PAUSE_ASYM, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE, ADVERTISE_PAUSE_CAP, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX, ADVERTISE_100FULL, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX, ADVERTISE_100HALF, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX, ADVERTISE_10FULL, reg_data);
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX, ADVERTISE_10HALF, reg_data);
            rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_ADVERTISE, reg_data);
        }
    }

    if (AIR_E_OK == rv)
    {
        reg_data = (ptr_adv->flags & AIR_PORT_PHY_AN_ADV_FLAGS_EEE) ? (EEE_1000BASE_T | EEE_100BASE_TX ) : 0;
        rv = sfp_pin_sda_writeC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, reg_data);
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            SET_PHY_ABILITY(ptr_adv->flags, AIR_PORT_PHY_AN_ADV_FLAGS_2500M, MULTIG_ANAR_2500M, reg_data);
            rv = sfp_pin_sda_writeC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, reg_data);
        }
    }

    return rv;
}

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
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    ptr_adv->flags = 0;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_CTRL1000, &reg_data);

    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_data, ADVERTISE_1000FULL, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC22ByPort(unit, port, MII_ADVERTISE, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, ADVERTISE_10HALF, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_10FULL, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_100HALF, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_100FULL, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_CAP, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, ADVERTISE_PAUSE_ASYM, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEEAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, (EEE_1000BASE_T | EEE_100BASE_TX), AIR_PORT_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_ANAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, MULTIG_ANAR_2500M, AIR_PORT_PHY_AN_ADV_FLAGS_2500M, ptr_adv->flags);
        }
    }
    return rv;
}

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
    AIR_PORT_PHY_AN_ADV_T *ptr_adv)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    SFP_PHY_LINK_STATUS_T link_status;

    ptr_adv->flags = 0;
    sfp_phy_getLinkStatus(unit, port, &link_status);
    if (0 == (SFP_PHY_LINK_STATUS_FLAGS_LINK_UP & link_status.flags))
    {
        return AIR_E_OK;
    }

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_STAT1000, &reg_data);
    if (AIR_E_OK == rv)
    {
        GET_PHY_ABILITY(reg_data, LPA_1000FULL, AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX, ptr_adv->flags);
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC22ByPort(unit, port, MII_LPA, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, LPA_10HALF, AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, LPA_10FULL, AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, LPA_100HALF, AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, LPA_100FULL, AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, LPA_PAUSE_CAP, AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE, ptr_adv->flags);
            GET_PHY_ABILITY(reg_data, LPA_PAUSE_ASYM, AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC45ByPort(unit, port, MMD_DEV_ANEG, MMD_EEELPAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, (EEE_1000BASE_T | EEE_100BASE_TX), AIR_PORT_PHY_AN_ADV_FLAGS_EEE, ptr_adv->flags);
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = sfp_pin_sda_readC45ByPort(unit, port, MMD_DEV_ANEG, MULTIG_LPAR, &reg_data);
        if (AIR_E_OK == rv)
        {
            GET_PHY_ABILITY(reg_data, MULTIG_LPAR_2500M, AIR_PORT_PHY_AN_ADV_FLAGS_2500M, ptr_adv->flags);
        }
    }
    return rv;
}

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
    const AIR_PORT_SPEED_T speed)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    if(speed >= AIR_PORT_SPEED_1000M)
    {
        return AIR_E_OP_INVALID;
    }

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        reg_data &= ~(BMCR_SPEED1000 | BMCR_SPEED100);

        if (AIR_PORT_SPEED_100M == speed)
        {
            reg_data |= BMCR_SPEED100;
        }
        else if (AIR_PORT_SPEED_10M == speed)
        {
            reg_data |= 0;
        }
        else
        {
            return AIR_E_NOT_SUPPORT;
        }

        rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }

    return rv;
}

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
    AIR_PORT_SPEED_T *ptr_speed)
{
    UI16_T  reg_data = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;
    AIR_PORT_SERDES_MODE_T serdes_mode;

    /* 2.5G TODO */
    rv = air_port_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        return rv;
    }

    if (AIR_PORT_SERDES_MODE_HSGMII == serdes_mode)
    {
        *ptr_speed = AIR_PORT_SPEED_2500M;
        return AIR_E_OK;
    }

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);
    if (rv != AIR_E_OK)
    {
        return rv;
    }

    if (0 != (reg_data & BMCR_SPEED1000))
    {
        *ptr_speed = AIR_PORT_SPEED_1000M;
    }
    else if (0 != (reg_data & BMCR_SPEED100))
    {
        *ptr_speed = AIR_PORT_SPEED_100M;
    }
    else
    {
        *ptr_speed = AIR_PORT_SPEED_10M;
    }
    return rv;
}

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
    const AIR_PORT_DUPLEX_T duplex)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_PORT_SERDES_MODE_T serdes_mode;

    rv = air_port_getSerdesMode(unit, port, &serdes_mode);
    if (rv != AIR_E_OK)
    {
        return rv;
    }

    if ((AIR_PORT_SERDES_MODE_HSGMII == serdes_mode) &&
        (AIR_PORT_DUPLEX_HALF == duplex))
    {
       return AIR_E_BAD_PARAMETER;
    }

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

    if (AIR_E_OK == rv)
    {
        if (AIR_PORT_DUPLEX_FULL == duplex)
        {
            reg_data |= BMCR_FULLDPLX;
        }
        else if (AIR_PORT_DUPLEX_HALF == duplex)
        {
            reg_data &= ~(BMCR_FULLDPLX);
        }
        else
        {
            return AIR_E_NOT_SUPPORT;
        }

        rv = sfp_pin_sda_writeC22ByPort(unit, port, MII_BMCR, reg_data);
    }
    return rv;
}

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
    AIR_PORT_DUPLEX_T *ptr_duplex)
{
    UI16_T          reg_data = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_AUX_CTRL_STA, &reg_data);
    if (AIR_E_OK == rv)
    {
        if (reg_data & AUX_FDX_STATUS)
        {
            *ptr_duplex = AIR_PORT_DUPLEX_FULL;
        }
        else
        {
            rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);

            if (AIR_E_OK == rv)
            {
                if (0 != (reg_data & BMCR_FULLDPLX))
                {
                    *ptr_duplex = AIR_PORT_DUPLEX_FULL;
                }
                else
                {
                    *ptr_duplex = AIR_PORT_DUPLEX_HALF;
                }
            }
        }
    }
    return rv;
}

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
    SFP_PHY_LINK_STATUS_T *ptr_status)
{
    UI16_T reg_data = 0;
    I32_T rv = AIR_E_OK;
    AIR_PORT_PHY_AN_T auto_nego;
    UI16_T adv = 0, lpa = 0, lpagb = 0, common_adv_gb = 0, common_adv = 0;

    osapi_memset(ptr_status, 0, sizeof(SFP_PHY_LINK_STATUS_T));

    rv = sfp_port_checkInsidePHYReady(unit, port);
    if (MW_E_OK != rv)
    {
        return rv;
    }

    rv = sfp_pin_sda_readC22ByPort(0, port, MII_BMSR, &reg_data);
    if (AIR_E_OK == rv)
    {
        SET_PHY_ABILITY(reg_data, BMSR_LSTATUS, SFP_PHY_LINK_STATUS_FLAGS_LINK_UP, ptr_status->flags);
        SET_PHY_ABILITY(reg_data, BMSR_ANEGCOMPLETE, SFP_PHY_LINK_STATUS_FLAGS_AUTO_NEGO_DONE, ptr_status->flags);
        SET_PHY_ABILITY(reg_data, BMSR_RFAULT, SFP_PHY_LINK_STATUS_FLAGS_REMOTE_FAULT, ptr_status->flags);
    }

    if ((AIR_E_OK == rv) && (ptr_status->flags & SFP_PHY_LINK_STATUS_FLAGS_LINK_UP))
    {
        AIR_PORT_SERDES_MODE_T serdes_mode;

        rv = air_port_getSerdesMode(unit, port, &serdes_mode);
        if (AIR_E_OK == rv)
        {
            if (AIR_PORT_SERDES_MODE_HSGMII == serdes_mode)
            {
                ptr_status->speed = AIR_PORT_SPEED_2500M;
                ptr_status->duplex = AIR_PORT_DUPLEX_FULL;
            }
            else
            {
                rv = sfp_phy_getAutoNego(unit, port, &auto_nego);
                if (AIR_E_OK != rv)
                {
                    return rv;
                }

                if (AIR_PORT_PHY_AN_ENABLE == auto_nego)
                {
                    rv = sfp_pin_sda_readC22ByPort(unit, port, MII_STAT1000, &lpagb);
                    if (AIR_E_OK == rv)
                    {
                        rv = sfp_pin_sda_readC22ByPort(unit, port, MII_CTRL1000, &adv);
                        if (AIR_E_OK == rv)
                        {
                            common_adv_gb = (lpagb & (adv << 2));
                            rv = sfp_pin_sda_readC22ByPort(unit, port, MII_LPA, &lpa);
                            if (AIR_E_OK == rv)
                            {
                                rv = sfp_pin_sda_readC22ByPort(unit, port, MII_ADVERTISE, &adv);
                                if (AIR_E_OK == rv)
                                {
                                    common_adv = lpa & adv;

                                    ptr_status->speed = AIR_PORT_SPEED_10M;
                                    ptr_status->duplex = AIR_PORT_DUPLEX_HALF;

                                    if (0 != (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)))
                                    {
                                        ptr_status->speed = AIR_PORT_SPEED_1000M;
                                        if (0 != (common_adv_gb & LPA_1000FULL))
                                        {
                                            ptr_status->duplex = AIR_PORT_DUPLEX_FULL;
                                        }
                                    }
                                    else if (0 != (common_adv & (LPA_100FULL | LPA_100HALF)))
                                    {
                                        ptr_status->speed = AIR_PORT_SPEED_100M;
                                        if (common_adv & LPA_100FULL)
                                        {
                                            ptr_status->duplex = AIR_PORT_DUPLEX_FULL;
                                        }
                                    }
                                    else
                                    {
                                        if (0 != (common_adv & LPA_10FULL))
                                        {
                                            ptr_status->duplex = AIR_PORT_DUPLEX_FULL;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                   rv = sfp_pin_sda_readC22ByPort(unit, port, MII_BMCR, &reg_data);
                   if (AIR_E_OK == rv)
                   {
                       if (0 != (reg_data & BMCR_FULLDPLX))
                       {
                           ptr_status->duplex = AIR_PORT_DUPLEX_FULL;
                       }
                       else
                       {
                           ptr_status->duplex = AIR_PORT_DUPLEX_HALF;
                       }
                       if (0 != (reg_data & BMCR_SPEED1000))
                       {
                           ptr_status->speed = AIR_PORT_SPEED_1000M;
                       }
                       else if (0 != (reg_data & BMCR_SPEED100))
                       {
                           ptr_status->speed = AIR_PORT_SPEED_100M;
                       }
                       else
                       {
                           ptr_status->speed = AIR_PORT_SPEED_10M;
                       }
                    }
                }
            }
        }
    }
    return rv;
}

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
    const UI32_T unit)
{
    AIR_PORT_BITMAP_CLEAR(_current_link_bitmap);
    return AIR_E_OK;
}

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
    const UI32_T unit)
{
    AIR_PORT_BITMAP_CLEAR(_current_link_bitmap);
    return AIR_E_OK;
}

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
    void *ptr_cookie)
{
    /* Only need to handle (combo) serdes port with serdes mode of SGMII or HSGMII. */
    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboPort(unit, port)))
    {
        return;
    }

    _sfp_phy_linkStatusChange_setMAC(unit, port, link, ptr_cookie);
    sfp_trunk_handleLinkStatusChange(unit, port, link, ptr_cookie);
}

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
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T port = 0;
    AIR_PORT_BITMAP_T new_link_bitmap;
    BOOL_T linkUp = FALSE;

    AIR_PORT_BITMAP_CLEAR(new_link_bitmap);
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        /* Only polling combo serdes port and serdes port. */
        if ((PLAT_CPU_PORT == port) ||
            ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
             (FALSE == sfp_port_is_comboPort(unit, port))))
        {
            continue;
        }

        rc = _sfp_phy_checkLinkup(unit, port, &linkUp);
        if (AIR_E_OK != rc)
        {
            SFP_LOG_DEBUG("getLinkStatus fail ret:%d. Port:%d", rc, port);
            continue;
        }

        if (TRUE == linkUp)
        {
            AIR_PORT_ADD(new_link_bitmap, port);
        }
    }

    _sfp_phy_handlelinkStatusChange(unit, new_link_bitmap);
}

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
    UI32_T port)
{
    AIR_PORT_BITMAP_T new_link_bitmap;

    if ((PLAT_CPU_PORT == port) ||
        ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
         (FALSE == sfp_port_is_comboPort(unit, port))))
    {
        SFP_LOG_DEBUG("No need to process Port:%d", port);
        return;
    }

    AIR_PORT_BITMAP_CLEAR(new_link_bitmap);
    AIR_PORT_BITMAP_COPY(new_link_bitmap, _current_link_bitmap);
    if (FALSE == AIR_PORT_CHK(_current_link_bitmap, port))
    {
        SFP_LOG_DEBUG("No need to compensateLinkDown Port:%d", port);
        return;
    }
    AIR_PORT_DEL(new_link_bitmap, port);

    SFP_LOG_DEBUG("compensateLinkDown Port:%d", port);
    _sfp_phy_handlelinkStatusChange(unit, new_link_bitmap);
}

