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

/* FILE NAME:   port_utils.c
 * PURPOSE:
 *      Implement the common interfaces for port.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "port_utils.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_task.h"
#include "sfp_pin.h"
#endif
#include "mw_platform.h"
#include "air_init.h"
#include "air_port.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

void
port_db_updateSerdesModeFlags(
    UI8_T *ptr_port_mode,
    UI8_T serdes_mode)
{
    if (AIR_PORT_SERDES_MODE_LAST == serdes_mode)
    {
        serdes_mode = PORT_DB_SERDES_MODE_UNKNOWN;
    }
    (*ptr_port_mode) &= ~((PORT_MODE_SERDES_MODE_BITMASK) << PORT_MODE_SERDES_MODE_OFFSET);
    (*ptr_port_mode) |= (serdes_mode & PORT_MODE_SERDES_MODE_BITMASK) << PORT_MODE_SERDES_MODE_OFFSET;
}

MW_ERROR_NO_T
port_checkPortSettingsValid(
    const UI16_T portSettings)
{
    if ((0 == (portSettings & PORT_SETTINGS_SPEED_ABILITY_BITMASK)) ||
       ((0 != (portSettings & PORT_SETTINGS_PORT_MODE_BITMASK)) &&
        (0 != (portSettings & (PORT_SETTINGS_RATE_DUPLEX_1000FUDX | PORT_SETTINGS_RATE_DUPLEX_2500M)))) ||
        (0 != (portSettings & (~(PORT_SETTINGS_PORT_MODE_BITMASK
                                | PORT_SETTINGS_FLOW_CONTROL_BITMASK
                                | PORT_SETTINGS_SPEED_ABILITY_BITMASK
                                | PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT)))))
    {
        return MW_E_BAD_PARAMETER;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
port_genPortSettings(
    UI8_T portMode,
    UI16_T speedAbility,
    UI8_T flowCtrl,
    UI16_T *ptr_portSettings)
{
    if ((ptr_portSettings == NULL) || (0 == (speedAbility & PORT_SETTINGS_SPEED_ABILITY_BITMASK)))
    {
        return MW_E_BAD_PARAMETER;
    }

    *ptr_portSettings = 0;

    /* Port mode(bit15): 0 AN; 1 Force */
    if (PORT_SETTINGS_PORT_MODE_AN != portMode)
    {
        *ptr_portSettings = PORT_SETTINGS_PORT_MODE_BITMASK;
    }

    /* Speed Ability PORT_SETTINGS_RATE_DUPLEX_X in port_uilts.h */
    *ptr_portSettings |= (speedAbility & PORT_SETTINGS_SPEED_ABILITY_BITMASK);

    /* Flow control(bit14): 0 OFF; 1 ON */
    if (PORT_SETTINGS_FLOW_CTRL_OFF != flowCtrl)
    {
        *ptr_portSettings |= PORT_SETTINGS_FLOW_CONTROL_BITMASK;
    }
    return MW_E_OK;
}

MW_ERROR_NO_T
port_parsePortSettings(
    const UI16_T portSettings,
    UI8_T *ptr_portMode,
    UI16_T *ptr_speedAbility,
    UI8_T *ptr_flowCtrl)
{
    if ((ptr_portMode == NULL) && (ptr_speedAbility == NULL) && (ptr_flowCtrl == NULL))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Port mode(bit15): 0 AN; 1 Force */
    if (ptr_portMode != NULL)
    {
        *ptr_portMode = (0 != (portSettings & PORT_SETTINGS_PORT_MODE_BITMASK)) ? PORT_SETTINGS_PORT_MODE_FORCE : PORT_SETTINGS_PORT_MODE_AN;
    }

    /* Speed Ability PORT_SETTINGS_RATE_DUPLEX_X in port_uilts.h */
    if (ptr_speedAbility != NULL)
    {
        *ptr_speedAbility = portSettings & PORT_SETTINGS_SPEED_ABILITY_BITMASK;
    }

    /* Flow control(bit14): 0 OFF; 1 ON */
    if (ptr_flowCtrl != NULL)
    {
        *ptr_flowCtrl = (0 != (portSettings & PORT_SETTINGS_FLOW_CONTROL_BITMASK)) ? PORT_SETTINGS_FLOW_CTRL_ON : PORT_SETTINGS_FLOW_CTRL_OFF;
    }
    return MW_E_OK;
}


/* FUNCTION NAME:   port_correctPortMode
 * PURPOSE:
 *      Correct the speed for a specific port with a certain serdes_mode. It is
 *      needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the speed to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the speed corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctPortMode(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI8_T *ptr_portMode)
{
    if (AIR_PORT_SERDES_MODE_1000BASE_X == serdes_mode)
    {
        /* AN mode only */
        *ptr_portMode = PORT_SETTINGS_PORT_MODE_AN;
    }
    else if (AIR_PORT_SERDES_MODE_HSGMII == serdes_mode)
    {
        if (FALSE == port_check25GCopperPort(unit, port))
        {
            *ptr_portMode = PORT_SETTINGS_PORT_MODE_FORCE;
        }
    }
    else if (AIR_PORT_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        /* Force mode only */
        *ptr_portMode = PORT_SETTINGS_PORT_MODE_FORCE;
    }
#ifdef AIR_SUPPORT_SFP
    else if (AIR_PORT_SERDES_MODE_SGMII == serdes_mode)
    {
        SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = sfp_task_getPortInfo(port);

        if (NULL == ptr_taskPortInfo)
        {
            return;
        }
        if (FALSE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
        {
            /* SFP auto adaption SFP port or COMBO SFP port: AN mode only */
            if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
            {
                *ptr_portMode = PORT_SETTINGS_PORT_MODE_AN;
            }
        }
    }
#endif
}

/* FUNCTION NAME:   port_correctSpeedAbility
 * PURPOSE:
 *      Correct the speed ability for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the speed ability to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the speed ability corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctSpeedAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI16_T *ptr_speedAbility)
{
    *ptr_speedAbility &= PORT_SETTINGS_SPEED_ABILITY_BITMASK;
    if (AIR_PORT_SERDES_MODE_1000BASE_X == serdes_mode)
    {
        *ptr_speedAbility = PORT_SETTINGS_RATE_DUPLEX_1000FUDX;
    }
    else if (AIR_PORT_SERDES_MODE_HSGMII == serdes_mode)
    {
        if (TRUE == port_check25GCopperPort(unit, port))
        {
            if (0 == (*ptr_speedAbility))
            {
                *ptr_speedAbility = (PORT_SETTINGS_RATE_DUPLEX_10HFDX |
                                       PORT_SETTINGS_RATE_DUPLEX_10FUDX |
                                       PORT_SETTINGS_RATE_DUPLEX_100HFDX |
                                       PORT_SETTINGS_RATE_DUPLEX_100FUDX |
                                       PORT_SETTINGS_RATE_DUPLEX_1000FUDX|
                                       PORT_SETTINGS_RATE_DUPLEX_2500M);
            }
        }
        else
        {
            *ptr_speedAbility = PORT_SETTINGS_RATE_DUPLEX_2500M;
        }
    }
    else if (AIR_PORT_SERDES_MODE_100BASE_FX == serdes_mode)
    {
        *ptr_speedAbility = PORT_SETTINGS_RATE_DUPLEX_100FUDX;
    }
    else
    {
        *ptr_speedAbility &= ~PORT_SETTINGS_RATE_DUPLEX_2500M;
        /* SGMII */
        if (0 == (*ptr_speedAbility))
        {
            (*ptr_speedAbility) = (PORT_SETTINGS_RATE_DUPLEX_10HFDX |
                                   PORT_SETTINGS_RATE_DUPLEX_10FUDX |
                                   PORT_SETTINGS_RATE_DUPLEX_100HFDX |
                                   PORT_SETTINGS_RATE_DUPLEX_100FUDX |
                                   PORT_SETTINGS_RATE_DUPLEX_1000FUDX);
        }
    }
}

/* FUNCTION NAME:   port_correctFlowctrl
 * PURPOSE:
 *      Correct the flow control setting for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the flow control setting to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the flow control setting corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctFlowctrl(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI8_T *ptr_flowCtrl)
{
    if ((PORT_SETTINGS_FLOW_CTRL_OFF != (*ptr_flowCtrl)) &&
        (PORT_SETTINGS_FLOW_CTRL_ON != (*ptr_flowCtrl)))
    {
        (*ptr_flowCtrl) = PORT_SETTINGS_FLOW_CTRL_OFF;
    }
}

/* FUNCTION NAME:   port_correctFlowctrl
 * PURPOSE:
 *      Correct the port setting for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 * OUTPUT:
 *      ptr_portSettings -- A pointer returns the port setting corrected
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
port_correctPortSettings(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI16_T *ptr_portSettings)
{
    UI8_T portMode = 0xFF, flowCtrl = 0xFF;
    UI16_T speedAbility = 0;

    port_parsePortSettings(*ptr_portSettings, &portMode, &speedAbility, &flowCtrl);
    port_correctPortMode(unit, port, serdes_mode, &portMode);
    port_correctSpeedAbility(unit, port, serdes_mode, &speedAbility);
    port_correctFlowctrl(unit, port, serdes_mode, &flowCtrl);

    return port_genPortSettings(portMode, speedAbility, flowCtrl, ptr_portSettings);
}

/* Convert MW used speed ability to AIR used speed ability defined in AIR_PORT_PHY_AN_ADV_T. */
void
port_covertSpeedAbility(
    UI16_T *ptr_speedAbility)
{
    UI16_T speedAbility = *ptr_speedAbility;

    /* They are the same as AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX ... AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX */
    *ptr_speedAbility &= (PORT_SETTINGS_RATE_DUPLEX_10HFDX |
                          PORT_SETTINGS_RATE_DUPLEX_10FUDX |
                          PORT_SETTINGS_RATE_DUPLEX_100HFDX |
                          PORT_SETTINGS_RATE_DUPLEX_100FUDX |
                          PORT_SETTINGS_RATE_DUPLEX_1000FUDX);

    if (0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_2500M))
    {
        *ptr_speedAbility |= AIR_PORT_PHY_AN_ADV_FLAGS_2500M;
    }
}

/* FUNCTION NAME:   port_check25GCopperPort
 * PURPOSE:
 *      Check port is weather 2.5G speed or not.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             -  2.5G port
 *      FALSE            -  not 2.5G port
 *
 * NOTES:
 *      Exclude sfp port
 */
UI8_T
port_check25GCopperPort(
    const UI32_T unit,
    const UI32_T port)
{
#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboPort(unit, port)))
    {
        return FALSE;
    }
#endif

    if (TRUE == AIR_PORT_CHK(PLAT_PORT_CAPACITY, port))
    {
        return TRUE;
    }

    return FALSE;
}

/* FUNCTION NAME:   port_correctPortSpeedDbCfg
 * PURPOSE:
 *      Correct DB default config for high speed port.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             -  The config changed for high speed port
 *      FALSE            -  The config not change
 *
 * NOTES:
 *      Exclude sfp port
 */
UI8_T
port_correctPortSpeedDbCfg(
    const UI32_T unit,
    const UI32_T port,
    UI16_T *ptr_cfg)
{
    if (NULL != ptr_cfg)
    {
        if (TRUE == port_check25GCopperPort(unit, port))
        {
            *ptr_cfg |= PORT_SETTINGS_RATE_DUPLEX_2500M;
            return TRUE;
        }
    }

    return FALSE;
}

/* FUNCTION NAME:   port_getInterfaceShortName
 * PURPOSE:
 *      Get port short name.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      name_size        -- ptr_name size
 * OUTPUT:
 *      ptr_name         -- Short name
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      gi1/2.5gi1
 */
MW_ERROR_NO_T
port_getInterfaceShortName(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T name_size,
    C8_T  *ptr_name)
{
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_MIN_MAX_RANGE(port, 0, PLAT_MAX_PORT_NUM);

    if (TRUE == port_check25GCopperPort(unit, port))
    {
        snprintf(ptr_name, name_size, "2.5gi%d", port);
        return MW_E_OK;
    }

    snprintf(ptr_name, name_size, "gi%d", port);
    return MW_E_OK;
}

/* FUNCTION NAME:   port_getInterfaceFullName
 * PURPOSE:
 *      Get port short name.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      name_size        -- ptr_name size
 * OUTPUT:
 *      ptr_name         -- Short name
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      GigabitEthernet1/2.5GigabitEthernet1
 */
MW_ERROR_NO_T
port_getInterfaceFullName(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T name_size,
    C8_T  *ptr_name)
{
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_MIN_MAX_RANGE(port, 0, PLAT_MAX_PORT_NUM);

    if (TRUE == port_check25GCopperPort(unit, port))
    {
        snprintf(ptr_name, name_size, "2.5GigabitEthernet%d", port);
        return MW_E_OK;
    }

    snprintf(ptr_name, name_size, "GigabitEthernet%d", port);
    return MW_E_OK;
}

/* FUNCTION NAME:   port_db_initPortMode
 * PURPOSE:
 *      Initialize port_oper_mode for all ports to DB.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port_num             -- Port num for ptr_mode
 * OUTPUT:
 *      ptr_mode             -- Port mode
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NO_MEMORY      -- Insufficient memory
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OTHERS         -- Other errors
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_initPortMode(
    const UI32_T unit,
    const UI32_T port_num,
    UI8_T       *ptr_mode)
{
    UI8_T port = 0, port_mode = 0;
    UI32_T count = 0;
    AIR_INIT_PORT_MAP_T *ptr_portMapList = NULL;
    AIR_PORT_COMBO_MODE_T combo_mode;
    AIR_PORT_SERDES_MODE_T serdes_mode;
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    MW_ERROR_NO_T  ret = MW_E_OK;

    MW_CHECK_PTR(ptr_mode);

    osapi_calloc((sizeof(AIR_INIT_PORT_MAP_T) * (PLAT_TOTAL_NUM)), "PORT", (void **)&ptr_portMapList);
    if (NULL != ptr_portMapList)
    {
        rc = air_init_getSdkPortMap(unit, &count, ptr_portMapList);
        if (AIR_E_OK == rc)
        {
            for (port = 1; port < count; port++)
            {
                port_mode = 0;
                /* Check port type bit7-6 for AIR_INIT_PORT_TYPE_T info. */
                if (AIR_INIT_PORT_TYPE_LAST <= ptr_portMapList[port].port_type)
                {
                    continue;
                }
                port_mode = (ptr_portMapList[port].port_type & PORT_MODE_BASIC_PORT_TYPE_BITMASK) << PORT_MODE_BASIC_PORT_TYPE_OFFSET;

                /* Check XSGMII type bit5-4 for AIR_INIT_XSGMII_PORT_T.flags info. */
                if (AIR_INIT_PORT_TYPE_XSGMII == ptr_portMapList[port].port_type)
                {
                    port_mode |= (ptr_portMapList[port].xsgmii_port.flags & PORT_MODE_XSGMII_TYPE_BITMASK) << PORT_MODE_XSGMII_TYPE_OFFSET;
                } /* else it is 0 for PHY */

#ifdef AIR_SUPPORT_SFP
                /* Check COMBO mode bit3 for AIR_PORT_COMBO_MODE_T info. */
                if ((ptr_portMapList[port].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_COMBO) &&
                    (FALSE == sfp_port_is_pureComboSerdesPort(unit, port)))
                {
                    rc = air_port_getComboMode(unit, port, &combo_mode);
                    if (AIR_E_OK == rc)
                    {
                        port_mode |= (combo_mode & PORT_MODE_COMBO_MODE_BITMASK) << PORT_MODE_COMBO_MODE_OFFSET;
                    }
                } /* else it is 0 with no meaning for non-COMBO port. */
#endif
                /* Check SERDES mode bit2-0 for AIR_PORT_SERDES_MODE_T info. */
                if (AIR_INIT_PORT_TYPE_XSGMII == ptr_portMapList[port].port_type)
                {
                    rc = air_port_getSerdesMode(unit, port, &serdes_mode);
                    if (AIR_E_OK == rc)
                    {
                        port_db_updateSerdesModeFlags(&port_mode, serdes_mode);
                    }
                } /* else it is 0 with no meaning for non-SERDES port. */

                if (port <= port_num)
                {
                    ptr_mode[port-1] = port_mode;
                }
            }
        }
        else
        {
            ret = MW_E_OTHERS;
        }
        osapi_free(ptr_portMapList);
    }
    else
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME:   port_db_ParsePortMode
 * PURPOSE:
 *      Parse port_oper_mode DB data.
 *
 * INPUT:
 *      port_mode              -- The port_oper_mode DB data
 * OUTPUT:
 *      ptr_basic_port_type    -- A pointer to the basic port type parsed from the DB data
 *      ptr_xsgmii_type        -- A pointer to XSGMII type parsed from the DB data
 *      ptr_combo_mode         -- A pointer to COMBO mode parsed from the DB data
 *      ptr_serdes_mode        -- A pointer to SERDES mode parsed from the DB data
 * RETURN:
 *      MW_E_OK               -- Operation success
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_parsePortMode(
    const UI8_T port_mode,
    PORT_DB_BASIC_TYPE_T *ptr_basic_port_type,
    PORT_DB_XSGMII_TYPE_T *ptr_xsgmii_type,
    PORT_DB_COMBO_MODE_T *ptr_combo_mode,
    PORT_DB_SERDES_MODE_T *ptr_serdes_mode)
{
    if (NULL != ptr_basic_port_type)
    {
        *ptr_basic_port_type = (port_mode >> PORT_MODE_BASIC_PORT_TYPE_OFFSET) & PORT_MODE_BASIC_PORT_TYPE_BITMASK;
    }

    if (NULL != ptr_xsgmii_type)
    {
        *ptr_xsgmii_type = (port_mode >> PORT_MODE_XSGMII_TYPE_OFFSET) & PORT_MODE_XSGMII_TYPE_BITMASK;
    }

    if (NULL != ptr_combo_mode)
    {
        *ptr_combo_mode = (port_mode >> PORT_MODE_COMBO_MODE_OFFSET) & PORT_MODE_COMBO_MODE_BITMASK;
    }

    if (NULL != ptr_serdes_mode)
    {
        *ptr_serdes_mode = (port_mode >> PORT_MODE_SERDES_MODE_OFFSET) & PORT_MODE_SERDES_MODE_BITMASK;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   port_db_getPortType
 * PURPOSE:
 *      Extract port type from the port_oper_mode DB data.
 *
 * INPUT:
 *      unit                   -- Device unit number
 *      port                   -- Port number
 *      port_mode              -- The port_oper_mode DB data
 * OUTPUT:
 *      ptr_port_type          -- A pointer to port type parsed from the DB data
 * RETURN:
 *      MW_E_OK                -- Operation success
 *      MW_E_NOT_SUPPORT       -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_getPortType(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T port_mode,
    PORT_DB_TYPE_T *ptr_port_type)
{
    PORT_DB_BASIC_TYPE_T basic_port_type = PORT_DB_BASIC_TYPE_LAST;
    PORT_DB_XSGMII_TYPE_T xsgmii_type = PORT_DB_XSGMII_TYPE_LAST;
    PORT_DB_COMBO_MODE_T combo_mode = PORT_DB_COMBO_MODE_LAST;
    MW_ERROR_NO_T ret = AIR_E_OK;

    *ptr_port_type = PORT_DB_TYPE_LAST;

    ret = port_db_parsePortMode(port_mode, &basic_port_type, &xsgmii_type, &combo_mode, NULL);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    if (PORT_DB_BASIC_TYPE_BASET == basic_port_type)
    {
        *ptr_port_type = PORT_DB_TYPE_COPPER;
    }
    else if (PORT_DB_BASIC_TYPE_XSGMII == basic_port_type)
    {
#ifdef AIR_SUPPORT_SFP
        if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
            (TRUE == sfp_port_is_pureComboSerdesPort(unit, port)))
        {
            *ptr_port_type = PORT_DB_TYPE_SERDES;
        }
        else
#endif
        {
            if (PORT_DB_XSGMII_TYPE_COMBO == xsgmii_type)
            {
                if (PORT_DB_COMBO_MODE_PHY == combo_mode)
                {
                    *ptr_port_type = PORT_DB_TYPE_COMBO_COPPER;
                }
                else if (PORT_DB_COMBO_MODE_SERDES == combo_mode)
                {
                    *ptr_port_type = PORT_DB_TYPE_COMBO_SERDES;
                }
            }
            else if (PORT_DB_XSGMII_TYPE_PHY == xsgmii_type)
            {
                *ptr_port_type = PORT_DB_TYPE_COPPER;
            }
        }
    }

    return (*ptr_port_type == PORT_DB_TYPE_LAST) ? MW_E_NOT_SUPPORT : MW_E_OK;
}

