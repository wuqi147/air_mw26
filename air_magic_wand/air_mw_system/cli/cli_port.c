/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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
#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"
#include "cli.h"
#include "cli_db.h"
#include "cli_internal.h"
#include "cli_port.h"
#include "db_api.h"
#include "db_data.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T _cli_port_max_num = 0;
static C8_T   _cli_port_list_str[CLI_PORT_LIST_STR_SIZE] = {0};

static C8_T *
_cli_port_get_speed_string(
    AIR_PORT_SPEED_T speed)
{
    switch(speed)
    {
        case AIR_PORT_SPEED_10M:
            return "10M";
        case AIR_PORT_SPEED_100M:
            return "100M";
        case AIR_PORT_SPEED_1000M:
            return "1000M";
        case AIR_PORT_SPEED_2500M:
            return "2500M";
        default:
            break;
    }

    return "None";
}

static CLI_PORT_DUPLEX_TYPE_T
_cli_port_get_duplex_from_portsetting(
    UI16_T port_setting)
{
    /* clear AN/FC */
    port_setting &= PORT_SETTINGS_SPEED_ABILITY_BITMASK;

    if (0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_HALF_BITMASK)))
    {
        return CLI_PORT_DUPLEX_TYPE_HALF;
    }

    if (0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_FULL_BITMASK)))
    {
        return CLI_PORT_DUPLEX_TYPE_FULL;
    }

    if ((0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_AUTO_10_BITMASK))) ||
        (0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_AUTO_100_BITMASK))) ||
        (0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_AUTO_1000_BITMASK))) ||
        (0  == (port_setting & (~CLI_PORT_SETTINGS_DUPLEX_AUTO_2500_BITMASK))))
    {
        return CLI_PORT_DUPLEX_TYPE_AUTO;
    }

    return CLI_PORT_DUPLEX_TYPE_AUTO;
}

static CLI_PORT_SPEED_TYPE_T
_cli_port_get_speed_from_portsetting(
    UI16_T port_setting)
{
    /* clear AN/FC */
    port_setting &= PORT_SETTINGS_SPEED_ABILITY_BITMASK;

    if (port_setting & CLI_PORT_SETTINGS_SPEED_AUTO_BITMASK)
    {
        return CLI_PORT_SPEED_TYPE_AUTO;
    }

    if (0 == (port_setting & (~CLI_PORT_SETTINGS_SPEED_10_BITMASK)))
    {
        return CLI_PORT_SPEED_TYPE_10;
    }

    if (0 == (port_setting & (~CLI_PORT_SETTINGS_SPEED_100_BITMASK)))
    {
        return CLI_PORT_SPEED_TYPE_100;
    }

    return CLI_PORT_SPEED_TYPE_AUTO;
}

static void
_cli_port_setting_send_cgistring(
    UI32_T port,
    UI8_T  admin_state,
    UI16_T port_setting)
{
    UI8_T               speed = 0;
    UI8_T               flowcontrol = 0;
    UI8_T               ability = 0;
    C8_T                *ptr_cgi_str = NULL;
    UI32_T              trunkBitMap = 0;
    UI8_T               forceFc = FALSE;

    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        CLI_LOG_ERROR("Failed to allocate memory for CGI string");
        return;
    }

    if (port_setting & PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT)
    {
        flowcontrol = (PORT_SETTINGS_FLOW_CONTROL_BITMASK & port_setting) ? 1 : 0;
        forceFc = TRUE;
    }
    else
    {
        /* Auto flow-control */
        if (PORT_SETTINGS_PORT_MODE_BITMASK & port_setting)
        {
            flowcontrol= 0;
        }
        else
        {
            flowcontrol= 1;
        }
    }

    if (PORT_SETTINGS_PORT_MODE_BITMASK & port_setting)
    {
        speed = 1;
        if ((PORT_SETTINGS_RATE_DUPLEX_100HFDX & port_setting) ||
            (PORT_SETTINGS_RATE_DUPLEX_100FUDX & port_setting))
        {
            speed = 2;
        }
    }

    ability = port_setting & PORT_SETTINGS_SPEED_ABILITY_BITMASK;
    port = BIT(port-1);

    snprintf(ptr_cgi_str, CLI_CGI_STR_MAX_LEN, "/port_setting.cgi?port_bit=%u&state=%u"
                   "&speed=%u&flowcontrol=%u&ability=%u&trunkBitMap=%u&cmdFlag=%u&forceFc=%u", port, admin_state,
                   speed, flowcontrol, ability, trunkBitMap, TRUE, forceFc);
    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_cgi_str);

    return;
}

static void
_cli_cmd_port_setDuplex(
    UI32_T port_map,
    CLI_PORT_DUPLEX_TYPE_T duplex)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              port_setting[MAX_PORT_NUM] = {0};
    CLI_PORT_SPEED_TYPE_T   port_speed = 0;
    CLI_PORT_DUPLEX_TYPE_T  port_duplex = 0;

    rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return;
    }
    osapi_memcpy(port_setting, ptr_data, sizeof(port_setting));
    MW_FREE(ptr_msg);

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT(port) & port_map)
        {
            port_duplex = _cli_port_get_duplex_from_portsetting(port_setting[port]);
            if (port_duplex == duplex)
            {
                BIT_DEL(port_map, port);
            }
        }
    }

    if (0 == port_map)
    {
        /* not change */
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }

            port_speed = _cli_port_get_speed_from_portsetting(port_setting[port]);
            if (CLI_PORT_DUPLEX_TYPE_AUTO == duplex)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_speed == CLI_PORT_SPEED_TYPE_AUTO)
                    {
                        port_setting[port] |= PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                        if (FALSE == port_check25GCopperPort(0, port+1))
                        {
                            port_setting[port] &= ~PORT_SETTINGS_RATE_DUPLEX_2500M;
                        }
                    }
                    else if (port_speed == CLI_PORT_SPEED_TYPE_10)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_10FUDX);
                    }
                    else if (port_speed == CLI_PORT_SPEED_TYPE_100)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_100HFDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX);
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_speed == CLI_PORT_SPEED_TYPE_10) ?
                                          (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_10FUDX) :
                                          (PORT_SETTINGS_RATE_DUPLEX_100HFDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX);
                    port_setting[port] &= ~PORT_SETTINGS_PORT_MODE_BITMASK;
                }
            }
            else if (CLI_PORT_DUPLEX_TYPE_FULL == duplex)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_speed == CLI_PORT_SPEED_TYPE_AUTO)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_10FUDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX | PORT_SETTINGS_RATE_DUPLEX_1000FUDX;
#ifdef AIR_EN_AN8811B_PHY
                        if (TRUE == port_check25GCopperPort(0, port+1))
                        {
                            port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_2500M;
                        }
#endif
                    }
                    if (port_speed == CLI_PORT_SPEED_TYPE_10)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_10FUDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                    if (port_speed == CLI_PORT_SPEED_TYPE_100)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_100FUDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_speed == CLI_PORT_SPEED_TYPE_10) ?
                                          PORT_SETTINGS_RATE_DUPLEX_10FUDX : PORT_SETTINGS_RATE_DUPLEX_100FUDX;
                }
            }
            else if (CLI_PORT_DUPLEX_TYPE_HALF == duplex)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_speed == CLI_PORT_SPEED_TYPE_AUTO)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_100HFDX);
                    }
                    if (port_speed == CLI_PORT_SPEED_TYPE_10)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_10HFDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                    if (port_speed == CLI_PORT_SPEED_TYPE_100)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_100HFDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_speed == CLI_PORT_SPEED_TYPE_10) ?
                                          PORT_SETTINGS_RATE_DUPLEX_10HFDX : PORT_SETTINGS_RATE_DUPLEX_100HFDX;
                }
            }

            _cli_port_setting_send_cgistring(port+1, *ptr_data, port_setting[port]);
            MW_FREE(ptr_msg);
        }
    }
}

static void
_cli_cmd_port_setEee(
    UI32_T port_map,
    UI8_T  status)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    C8_T                *ptr_cgi_str = {0};

    rc = cli_queue_getData(PORT_CFG_INFO, PORT_EEE_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT(port) & port_map)
        {
            if ((TRUE == status) && (TRUE == ptr_data[port]))
            {
                BIT_DEL(port_map, port);
            }

            if ((FALSE == status) && (FALSE == ptr_data[port]))
            {
                BIT_DEL(port_map, port);
            }
        }
    }
    MW_FREE(ptr_msg);

    if (0 == port_map)
    {
        /* not change */
        return;
    }

    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        CLI_LOG_ERROR("Failed to allocate memory for CGI string");
        return;
    }

    snprintf(ptr_cgi_str, CLI_CGI_STR_MAX_LEN, "/eeeSet.cgi?state=%d&portBit=%u", status, port_map);
    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_cgi_str);

    return;
}

static void
_cli_cmd_port_setFlowcontrol(
    UI32_T          port_map,
    CLI_PORT_FC_TYPE_T  fc_state)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              port_setting[MAX_PORT_NUM] = {0};
    UI8_T               status = 0;
    UI8_T               send_flag = FALSE;

    rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return;
    }
    osapi_memcpy(port_setting, ptr_data, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);
    MW_FREE(ptr_msg);

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT(port) & port_map)
        {
            if (CLI_PORT_FC_TYPE_AUTO == fc_state)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    status = TRUE;
                }
                else
                {
                    status = FALSE;
                }
                send_flag = (port_setting[port] & PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT) ? TRUE : FALSE;
                port_setting[port] &= ~PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT;
            }
            else if (CLI_PORT_FC_TYPE_OFF == fc_state)
            {
                status = FALSE;
                send_flag = (port_setting[port] & PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT) ? FALSE : TRUE;
                port_setting[port] |= PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT;
            }
            else
            {
                status = TRUE;
                send_flag = (port_setting[port] & PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT) ? FALSE : TRUE;
                port_setting[port] |= PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT;
            }

            if ((TRUE == status) && (PORT_SETTINGS_FLOW_CONTROL_BITMASK & port_setting[port]) &&
                (FALSE == send_flag))
            {
                BIT_DEL(port_map, port);
            }

            if ((FALSE == status) && (0 == (PORT_SETTINGS_FLOW_CONTROL_BITMASK & port_setting[port])) &&
                (FALSE == send_flag))
            {
                BIT_DEL(port_map, port);
            }
        }
    }

    if ((0 == port_map) && (FALSE == send_flag))
    {
        /* not change */
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }

            if (TRUE == status)
            {
                port_setting[port] |= PORT_SETTINGS_FLOW_CONTROL_BITMASK;
            }
            else
            {
                port_setting[port] &= ~PORT_SETTINGS_FLOW_CONTROL_BITMASK;
            }

            _cli_port_setting_send_cgistring(port+1, *ptr_data, port_setting[port]);
            MW_FREE(ptr_msg);
        }
    }
}

static void
_cli_cmd_port_setAdminState(
    UI32_T port_map,
    UI8_T  status)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              port_setting = 0;

    rc = cli_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT(port) & port_map)
        {
            if ((TRUE == status) && (TRUE == ptr_data[port]))
            {
                BIT_DEL(port_map, port);
            }

            if ((FALSE == status) && (FALSE == ptr_data[port]))
            {
                BIT_DEL(port_map, port);
            }
        }
    }
    MW_FREE(ptr_msg);

    if (0 == port_map)
    {
        /* not change */
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
            MW_FREE(ptr_msg);

            _cli_port_setting_send_cgistring(port+1, status, port_setting);
        }
    }

    return;
}

static void
_cli_cmd_port_setSpeed(
    UI32_T port_map,
    CLI_PORT_SPEED_TYPE_T speed)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              port_setting[MAX_PORT_NUM] = {0};
    CLI_PORT_SPEED_TYPE_T   port_speed = 0;
    CLI_PORT_DUPLEX_TYPE_T  port_duplex = 0;

    rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return;
    }
    osapi_memcpy(port_setting, ptr_data, sizeof(port_setting));
    MW_FREE(ptr_msg);

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT(port) & port_map)
        {
            port_speed = _cli_port_get_speed_from_portsetting(port_setting[port]);
            if (speed == port_speed)
            {
                BIT_DEL(port_map, port);
            }
        }
    }

    if (0 == port_map)
    {
        /* not change */
        return;
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            port_duplex = _cli_port_get_duplex_from_portsetting(port_setting[port]);

            if (CLI_PORT_SPEED_TYPE_AUTO == speed)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_duplex == CLI_PORT_DUPLEX_TYPE_AUTO)
                    {
                        port_setting[port] |= PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                        if (FALSE == port_check25GCopperPort(0, port+1))
                        {
                            port_setting[port] &= ~PORT_SETTINGS_RATE_DUPLEX_2500M;
                        }
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_100HFDX);
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_FULL)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_10FUDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX | PORT_SETTINGS_RATE_DUPLEX_1000FUDX);
#ifdef AIR_EN_AN8811B_PHY
                        if (TRUE == port_check25GCopperPort(0, port+1))
                        {
                            port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_2500M;
                        }
#endif
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF) ?
                                          (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_100HFDX) :
                                          (PORT_SETTINGS_RATE_DUPLEX_10FUDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX | PORT_SETTINGS_RATE_DUPLEX_1000FUDX);
                    port_setting[port] &= ~PORT_SETTINGS_PORT_MODE_BITMASK;
                }
            }
            else if (CLI_PORT_SPEED_TYPE_100 == speed)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_duplex == CLI_PORT_DUPLEX_TYPE_AUTO)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_100HFDX | PORT_SETTINGS_RATE_DUPLEX_100FUDX;
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_100HFDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_FULL)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_100FUDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF) ?
                                          PORT_SETTINGS_RATE_DUPLEX_100HFDX : PORT_SETTINGS_RATE_DUPLEX_100FUDX;
                }
            }
            else if (CLI_PORT_SPEED_TYPE_10 == speed)
            {
                if (0 == (port_setting[port] & PORT_SETTINGS_PORT_MODE_BITMASK))
                {
                    /* AN mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    if (port_duplex == CLI_PORT_DUPLEX_TYPE_AUTO)
                    {
                        port_setting[port] |= (PORT_SETTINGS_RATE_DUPLEX_10HFDX | PORT_SETTINGS_RATE_DUPLEX_10FUDX);
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_10HFDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                    else if (port_duplex == CLI_PORT_DUPLEX_TYPE_FULL)
                    {
                        port_setting[port] |= PORT_SETTINGS_RATE_DUPLEX_10FUDX;
                        port_setting[port] |= PORT_SETTINGS_PORT_MODE_BITMASK;
                    }
                }
                else
                {
                    /* FORCE mode */
                    port_setting[port] &= ~PORT_SETTINGS_SPEED_ABILITY_BITMASK;
                    port_setting[port] |= (port_duplex == CLI_PORT_DUPLEX_TYPE_HALF) ?
                                          PORT_SETTINGS_RATE_DUPLEX_10HFDX : PORT_SETTINGS_RATE_DUPLEX_10FUDX;
                }
            }

            _cli_port_setting_send_cgistring(port+1, *ptr_data, port_setting[port]);
            MW_FREE(ptr_msg);
        }
    }
}

static UI32_T
_cli_cmd_port_showStatus(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    UI32_T port_map)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    DB_PORT_OPER_INFO_T port_info = {0};
    C8_T                buf[CLI_PORT_STRING_LEN] = {0};
    UI32_T              port_duplex = 0;
    UI32_T              port_speed = 0;
    PORT_DB_TYPE_T      port_type = 0;
    UI32_T              len = 0;
    UI16_T              port_setting = 0;

    if (0 == port_map)
    {
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "invalid parameter\n");
        return len;
    }

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-6s  %-12s  %-8s  %-7s  %-14s  %-12s\n", "Port", "Link-State", "Duplex", "Speed", "Flow-Control", "Type");

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(PORT_OPER_INFO, DB_ALL_FIELDS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            osapi_memcpy(&port_info, ptr_data, sizeof(DB_PORT_OPER_INFO_T));
            MW_FREE(ptr_msg);

            port_getInterfaceShortName(0, port+1, CLI_PORT_STRING_LEN, buf);
            port_db_getPortType(0, port+1, port_info.oper_mode, &port_type);

            if (TRUE == port_info.oper_status)
            {
                len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-6s  %-12s  %-8s  %-7s  %-14s  %-12s\n",
                                      buf, (port_info.oper_status ? "up":"down"),
                                      (port_info.oper_duplex == AIR_PORT_DUPLEX_HALF ? "half" : "full"),
                                      _cli_port_get_speed_string(port_info.oper_speed),
                                      (port_info.oper_flow_ctrl ? "on" : "off"),
                                      (port_type == PORT_DB_TYPE_COPPER) ? "Copper" : "Fiber");
            }
            else
            {
                rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
                if (MW_E_OK != rc)
                {
                    return len;
                }
                osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
                MW_FREE(ptr_msg);
                port_duplex = _cli_port_get_duplex_from_portsetting(port_setting);
                port_speed = _cli_port_get_speed_from_portsetting(port_setting);
                len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-6s  %-12s  %-8s  %-7s  %-14s  %-12s\n",
                                      buf, (port_info.oper_status ? "up":"down"),
                                      (port_duplex == CLI_PORT_DUPLEX_TYPE_AUTO ? "auto" : (port_duplex == CLI_PORT_DUPLEX_TYPE_FULL ? "full" : "half")),
                                      (port_speed == CLI_PORT_SPEED_TYPE_AUTO ? "auto" : (port_speed == CLI_PORT_SPEED_TYPE_10 ? "10M" : "100M")),
                                      ((port_setting & PORT_SETTINGS_FLOW_CONTROL_BITMASK) ? "on" : "off"),
                                      (port_type == PORT_DB_TYPE_COPPER) ? "Copper" : "Fiber");
            }
        }
    }
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "\n");

    return len;
}

static UI32_T
_cli_cmd_port_show(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    UI32_T port_map)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    C8_T                buf[CLI_PORT_STRING_LEN] = {0};
    DB_MIB_CNT_T        cnt = {0};
    DB_PORT_OPER_INFO_T port_info = {0};
    C8_T                *ptr_str = NULL;
    UI32_T              str_len = 0;
    PORT_DB_TYPE_T      port_type = 0;
    C8_T                uint64_string[20] = {0};
    UI16_T              port_setting = 0;
    UI32_T              port_duplex = 0;
    UI32_T              port_speed = 0;
    UI32_T              len = 0;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if (BIT_CHK(port_map, port))
        {
            rc = cli_queue_getData(MIB_CNT, DB_ALL_FIELDS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            osapi_memcpy(&cnt, ptr_data, sizeof(DB_MIB_CNT_T));
            MW_FREE(ptr_msg);
            osapi_memset(&uint64_string ,0 , sizeof(uint64_string));

            rc = cli_queue_getData(PORT_OPER_INFO, DB_ALL_FIELDS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            osapi_memcpy(&port_info, ptr_data, sizeof(DB_PORT_OPER_INFO_T));
            MW_FREE(ptr_msg);

            rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port+1, &ptr_msg, &data_size, (void **)&ptr_data);
            if (MW_E_OK != rc)
            {
                continue;
            }
            osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
            MW_FREE(ptr_msg);

            port_db_getPortType(0, port+1, port_info.oper_mode, &port_type);

            port_getInterfaceFullName(0, port+1, CLI_PORT_STRING_LEN, buf);
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%s is %s\n", buf, port_info.oper_status ? "up" : "down");
            str_len = strlen(buf);
            ptr_str = strstr(buf, "Ethernet");
            snprintf(buf+(ptr_str-buf), CLI_PORT_STRING_LEN-str_len, " Ethernet");
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " Hardware is %s\n", buf);

            port_duplex = _cli_port_get_duplex_from_portsetting(port_setting);
            port_speed = _cli_port_get_speed_from_portsetting(port_setting);

            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s, %s, media type is %s\n",
                                 (port_duplex == CLI_PORT_DUPLEX_TYPE_AUTO ? "Auto-duplex" : (port_duplex == CLI_PORT_DUPLEX_TYPE_FULL ? "Full-duplex" : "Half-duplex")),
                                 (port_speed == CLI_PORT_SPEED_TYPE_AUTO ? "Auto-speed":
                                 (port_speed == CLI_PORT_SPEED_TYPE_10 ? "10M": "100M")),
                                 (port_type == PORT_DB_TYPE_COPPER) ? "Copper" : "Fiber");
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " flow-control is %s\n",
                                 (port_setting & PORT_SETTINGS_FLOW_CONTROL_BITMASK) ? "on" : "off");

            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    %s packets input,", _printUI64_T(cnt.rx_packets, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s bytes,\n", _printUI64_T(cnt.rx_octets, uint64_string, sizeof(uint64_string)));

#ifdef AIR_SUPPORT_SNMP
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    Received %s broadcasts,", _printUI64_T(cnt.rx_broadcast_pkts, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s multicasts,", _printUI64_T(cnt.rx_multicast_pkts, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s unicasts,\n", _printUI64_T(cnt.rx_unicast_pkts, uint64_string, sizeof(uint64_string)));
#endif

            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    %s input errors\n", _printUI64_T(cnt.rx_errors, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    %s packets output,", _printUI64_T(cnt.tx_packets, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s bytes,\n", _printUI64_T(cnt.tx_octets, uint64_string, sizeof(uint64_string)));
#ifdef AIR_SUPPORT_SNMP
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    Transmitted %s broadcasts,", _printUI64_T(cnt.tx_broadcast_pkts, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s multicasts,", _printUI64_T(cnt.tx_multicast_pkts, uint64_string, sizeof(uint64_string)));
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, " %s unicasts,\n", _printUI64_T(cnt.tx_unicast_pkts, uint64_string, sizeof(uint64_string)));
#endif
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "    %s output errors\n", _printUI64_T(cnt.tx_errors, uint64_string, sizeof(uint64_string)));
        }
    }
    return len;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   cli_cmd_port_setDuplex
 * PURPOSE:
 *      Execute port cmd to set duplex.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setDuplex(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    MW_ERROR_NO_T       rc = 0;
    CLI_PORT_DUPLEX_TYPE_T  port_duplex = 0;
    UI16_T              default_setting = 0;
    UI32_T              port_map = 0;
    UI32_T              len = 0;
    UI8_T               valid_state = FALSE;

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    len = cli_cmd_port_checkValid(port_id, ptr_out_buf, out_buf_len, &valid_state);
    if (FALSE == valid_state)
    {
        return len;
    }

    if (osapi_strcmp(pptr_argv[0], "no") == 0)
    {
        rc = dbapi_getFactoryDefault(PORT_CFG_INFO, PORT_SETTINGS, port_id, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return 0;
        }
        osapi_memcpy(&default_setting, ptr_data, sizeof(default_setting));
        MW_FREE(ptr_data);
        port_duplex = _cli_port_get_duplex_from_portsetting(default_setting);
    }
    else if (osapi_strcmp(pptr_argv[1], "auto") == 0)
    {
        port_duplex = CLI_PORT_DUPLEX_TYPE_AUTO;
    }
    else if (osapi_strcmp(pptr_argv[1], "full") == 0)
    {
        port_duplex = CLI_PORT_DUPLEX_TYPE_FULL;
    }
    else if (osapi_strcmp(pptr_argv[1], "half") == 0)
    {
        port_duplex = CLI_PORT_DUPLEX_TYPE_HALF;
    }

    port_map = BIT(port_id-1);
    _cli_cmd_port_setDuplex(port_map, port_duplex);

    return 0;
}

/* FUNCTION NAME:   cli_cmd_port_setEEE
 * PURPOSE:
 *      Execute port cmd to set EEE.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setEEE(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id)
{
    UI8_T               state = 0;
    UI32_T              port_map = 0;

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    if (osapi_strcmp(pptr_argv[0], "no") == 0)
    {
        state = FALSE;
    }
    else
    {
        state = TRUE;
    }

    port_map = BIT(port_id-1);
    _cli_cmd_port_setEee(port_map, state);

    return 0;
}

/* FUNCTION NAME:   cli_cmd_port_setFlowcontrol
 * PURPOSE:
 *      Execute port cmd to set flow control.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setFlowcontrol(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              default_setting = 0;
    UI32_T              port_map = 0;
    CLI_PORT_FC_TYPE_T  flowcontrol = 0;
    UI32_T              len = 0;
    UI8_T               valid_state = FALSE;

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    len = cli_cmd_port_checkValid(port_id, ptr_out_buf, out_buf_len, &valid_state);
    if (FALSE == valid_state)
    {
        return len;
    }

    if (osapi_strcmp(pptr_argv[0], "no") == 0)
    {
        rc = dbapi_getFactoryDefault(PORT_CFG_INFO, PORT_SETTINGS, port_id, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return 0;
        }
        osapi_memcpy(&default_setting, ptr_data, sizeof(default_setting));
        MW_FREE(ptr_data);
        flowcontrol = (default_setting & PORT_SETTINGS_FLOW_CONTROL_BITMASK) ? CLI_PORT_FC_TYPE_AUTO : CLI_PORT_FC_TYPE_OFF;
    }
    else if (osapi_strcmp(pptr_argv[1], "auto") == 0)
    {
        flowcontrol = CLI_PORT_FC_TYPE_AUTO;
    }
    else if (osapi_strcmp(pptr_argv[1], "off") == 0)
    {
        flowcontrol = CLI_PORT_FC_TYPE_OFF;
    }
    else if (osapi_strcmp(pptr_argv[1], "on") == 0)
    {
        flowcontrol = CLI_PORT_FC_TYPE_ON;
    }

    port_map = BIT(port_id-1);
    _cli_cmd_port_setFlowcontrol(port_map, flowcontrol);

    return 0;
}

/* FUNCTION NAME:   cli_port_setAdminState
 * PURPOSE:
 *      Execute port cmd to set admin state.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_port_setAdminState(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id)
{
    UI8_T  state = 0;
    UI32_T port_map = 0;
    UI32_T len = 0;
    UI8_T  valid_state = FALSE;

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    len = cli_cmd_port_checkValid(port_id, ptr_out_buf, out_buf_len, &valid_state);
    if (FALSE == valid_state)
    {
        return len;
    }

    if (osapi_strcmp(pptr_argv[0], "no") == 0)
    {
        state = TRUE;
    }
    else
    {
        state = FALSE;
    }

    port_map = BIT(port_id-1);
    _cli_cmd_port_setAdminState(port_map, state);

    return 0;
}

/* FUNCTION NAME:   cli_cmd_port_setSpeed
 * PURPOSE:
 *      Execute port cmd to set speed.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setSpeed(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    MW_ERROR_NO_T       rc = 0;
    CLI_PORT_SPEED_TYPE_T   port_speed = 0;
    UI16_T              default_setting = 0;
    UI32_T              port_map = 0;
    UI32_T              len = 0;
    UI8_T               valid_state = FALSE;

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    len = cli_cmd_port_checkValid(port_id, ptr_out_buf, out_buf_len, &valid_state);
    if (FALSE == valid_state)
    {
        return len;
    }

    if (osapi_strcmp(pptr_argv[0], "no") == 0)
    {
        rc = dbapi_getFactoryDefault(PORT_CFG_INFO, PORT_SETTINGS, port_id, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return 0;
        }
        osapi_memcpy(&default_setting, ptr_data, sizeof(default_setting));
        MW_FREE(ptr_data);
        port_speed = _cli_port_get_speed_from_portsetting(default_setting);
    }
    else if (osapi_strcmp(pptr_argv[1], "10") == 0)
    {
        port_speed = CLI_PORT_SPEED_TYPE_10;
    }
    else if (osapi_strcmp(pptr_argv[1], "100") == 0)
    {
        port_speed = CLI_PORT_SPEED_TYPE_100;
    }
    else if (osapi_strcmp(pptr_argv[1], "auto") == 0)
    {
        port_speed = CLI_PORT_SPEED_TYPE_AUTO;
    }

    port_map = BIT(port_id-1);
    _cli_cmd_port_setSpeed(port_map, port_speed);

    return 0;
}

/* FUNCTION NAME:   cli_cmd_if_showStatus
 * PURPOSE:
 *      Display port current operating configuration.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_if_showStatus(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx)
{
    UI32_T    port_map = 0;
    UI32_T    len = 0, port = 0;
    UI32_T    max_port = cli_cmd_port_getMaxNum();
    C8_T     *ptr_p = pptr_argv[3];

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    if (argc > 4)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    if (0 == osapi_strcmp(ptr_p, "all"))
    {
        /* All ports' info */
        for (port = 1; port <= max_port; port++)
        {
            port_map |= BIT(port-1);
        }
        len = _cli_cmd_port_showStatus(ptr_out_buf, out_buf_len, port_map);
        return len;
    }

    /* Port id string check */
    while (0 != (*ptr_p))
    {
        if (0 == isdigit((I32_T)*ptr_p))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }
        ptr_p++;
    }

    ptr_p = pptr_argv[3];
    /* Fallback: parse the first number found (e.g. "2") */
    while ((0 != (*ptr_p)) && (0 == isdigit((I32_T)*ptr_p)))
    {
        ptr_p++;
    }
    if (0 != (*ptr_p))
    {
        port = atoi(ptr_p);
    }

    if ((port > 0) && (port <= max_port))
    {
        port_map = BIT(port-1);
        len = _cli_cmd_port_showStatus(ptr_out_buf, out_buf_len, port_map);
        return len;
    }

    len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
    return len;
}

/* FUNCTION NAME:   cli_cmd_if_show
 * PURPOSE:
 *      Display port status and packets statistics.
 *
 * INPUT:
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_if_show(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx)
{
    UI32_T    port_map = 0;
    UI32_T    len = 0, port= 0;
    UI32_T    max_port = cli_cmd_port_getMaxNum();
    C8_T     *ptr_p = pptr_argv[2];

    if ((NULL == ptr_out_buf) || (NULL == pptr_argv) || (0 == argc))
    {
        return 0;
    }

    if (argc > 3)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    /* Port id string check */
    while (0 != (*ptr_p))
    {
        if (0 == isdigit((I32_T)*ptr_p))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }
        ptr_p++;
    }

    ptr_p = pptr_argv[2];
    /* Fallback: parse the first number found (e.g. "2") */
    while ((0 != (*ptr_p)) && (0 == isdigit((I32_T)*ptr_p)))
    {
        ptr_p++;
    }
    if (0 != (*ptr_p))
    {
        port = atoi(ptr_p);
    }

    if ((port > 0) && (port <= max_port))
    {
        port_map = BIT(port-1);
        len = _cli_cmd_port_show(ptr_out_buf, out_buf_len, port_map);
        return len;
    }

    len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
    return len;
}

/* FUNCTION NAME:   cli_cmd_if_showCfg
 * PURPOSE:
 *      Show port config.
 *
 * INPUT:
 *      ptr_out_buf       -- print buffer
 *      out_buf_len       -- print buffer size
 *      conf_mode_flag    -- running-config/start-up config
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      output interface Gigabitethernet 1.
 */
UI32_T
cli_cmd_if_showCfg(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag)
{
    UI32_T          len = 0;
    UI8_T           port = 0;
    UI8_T          *ptr_db_table_head = NULL;
    UI16_T          data_size = 0;
    MW_ERROR_NO_T   rc = 0;

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (CLI_SHOW_STARTUP_CONFIG == conf_mode_flag)
    {
        rc = dbapi_getStartUp(PORT_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &data_size, (void **)&ptr_db_table_head);
        if (MW_E_OK != rc)
        {
            for (port = 0; port < _cli_port_max_num; port++)
            {
                len += osapi_snprintf(ptr_out_buf+len, out_buf_len-len, "!\n");
                len += osapi_snprintf(ptr_out_buf+len, out_buf_len-len, "interface %u\n", port+1);
            }
            return len;
        }
    }

    for (port = 0; port < _cli_port_max_num; port++)
    {
        len += osapi_snprintf(ptr_out_buf+len, out_buf_len-len, "!\n");

        len += osapi_snprintf(ptr_out_buf+len, out_buf_len-len, "interface %u\n", port+1);

        len += cli_cmd_port_showDuplexCfg(ptr_db_table_head, ptr_out_buf+len, out_buf_len-len, conf_mode_flag, port);
        len += cli_cmd_port_showEEECfg(ptr_db_table_head, ptr_out_buf+len, out_buf_len-len, conf_mode_flag, port);
        len += cli_cmd_port_showFcCfg(ptr_db_table_head, ptr_out_buf+len, out_buf_len-len, conf_mode_flag, port);
        len += cli_cmd_port_showAdminStateCfg(ptr_db_table_head, ptr_out_buf+len, out_buf_len-len, conf_mode_flag, port);
        len += cli_cmd_port_showSpeedCfg(ptr_db_table_head, ptr_out_buf+len, out_buf_len-len, conf_mode_flag, port);
    }

    if (CLI_SHOW_STARTUP_CONFIG == conf_mode_flag)
    {
        MW_FREE(ptr_db_table_head);
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_replacePortLable
 * PURPOSE:
 *      Repalce port-list string label.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
cli_cmd_port_replacePortLable(
    void)
{
    UI32_T port = 0, unit = 0;

    for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
    {
#ifdef AIR_SUPPORT_SFP
        if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
            (TRUE == sfp_port_is_comboPort(unit, port)))
        {
            break;
        }
#endif
    }

    _cli_port_max_num = port-1;
    snprintf(_cli_port_list_str, CLI_PORT_LIST_STR_SIZE, "<1-%d>", _cli_port_max_num);

    /* replace port-list label */
    cli_modify_cmd_node_name(CLI_PORT_LIST_LABEL, _cli_port_list_str);
}

/* FUNCTION NAME:   cli_cmd_port_getMaxNum
 * PURPOSE:
 *      Get max port num which can config by cmd.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      _cli_port_max_num
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_getMaxNum(
    void)
{
    return _cli_port_max_num;
}

/* FUNCTION NAME:   cli_cmd_port_showDuplexCfg
 * PURPOSE:
 *      Show port duplex config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showDuplexCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    CLI_PORT_DUPLEX_TYPE_T  port_duplex = 0;
    UI16_T              port_setting = 0;
    UI32_T              len = 0;
    UI16_T              port = (UI16_T)port_id + 1;
    DB_REQUEST_TYPE_T   req = {.t_idx = PORT_CFG_INFO,
                               .f_idx = PORT_SETTINGS,
                               .e_idx = port};

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (port > _cli_port_max_num)
    {
        return len;
    }

    if (CLI_SHOW_RUNNING_CONFIG == conf_mode_flag)
    {
        rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = db_getDataFromRawTable((UI8_T *)&port_setting, ptr_db_startup_head, req);
        if (MW_E_OK != rc)
        {
            return len;
        }
    }

    port_duplex = _cli_port_get_duplex_from_portsetting(port_setting);
    if (CLI_PORT_DUPLEX_TYPE_AUTO == port_duplex)
    {
        /* Do not show duplex auto */
    }
    else if (CLI_PORT_DUPLEX_TYPE_FULL == port_duplex)
    {
        /* show duplex full */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "duplex full\n");
    }
    else
    {
        /* show duplex half */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "duplex half\n");
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_showEEECfg
 * PURPOSE:
 *      Show port eee config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showEEECfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI8_T               state = 0;
    UI32_T              len = 0;
    UI16_T              port = (UI16_T)port_id + 1;
    DB_REQUEST_TYPE_T   req = {.t_idx = PORT_CFG_INFO,
                               .f_idx = PORT_EEE_ENABLE,
                               .e_idx = port};

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (port > _cli_port_max_num)
    {
        return len;
    }

    if (CLI_SHOW_RUNNING_CONFIG == conf_mode_flag)
    {
        rc = cli_queue_getData(PORT_CFG_INFO, PORT_EEE_ENABLE, port, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&state, ptr_data, sizeof(state));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = db_getDataFromRawTable((UI8_T *)&state, ptr_db_startup_head, req);
        if (MW_E_OK != rc)
        {
            return len;
        }
    }

    if (FALSE == state)
    {
        /* Do not show no eee enable */
    }
    else
    {
        /* show eee enable */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "eee enable\n");
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_showFcCfg
 * PURPOSE:
 *      Show port fc config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showFcCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI16_T              port_setting = 0;
    UI32_T              len = 0;
    UI16_T              port = (UI16_T)port_id + 1;
    DB_REQUEST_TYPE_T   req = {.t_idx = PORT_CFG_INFO,
                               .f_idx = PORT_SETTINGS,
                               .e_idx = port};

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (port > _cli_port_max_num)
    {
        return len;
    }

    if (CLI_SHOW_RUNNING_CONFIG == conf_mode_flag)
    {
        rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = db_getDataFromRawTable((UI8_T *)&port_setting, ptr_db_startup_head, req);
        if (MW_E_OK != rc)
        {
            return len;
        }
    }

    if (port_setting & PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT)
    {
        if (port_setting & PORT_SETTINGS_FLOW_CONTROL_BITMASK)
        {
          len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "flow-control on\n");
        }
        else
        {
          len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "flow-control off\n");
        }
    }
    else
    {
        /* Do not show flow-control auto */
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_showAdminStateCfg
 * PURPOSE:
 *      Show port admin state config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showAdminStateCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI8_T               state = 0;
    UI32_T              len = 0;
    UI16_T              port = (UI16_T)port_id + 1;
    DB_REQUEST_TYPE_T   req = {.t_idx = PORT_CFG_INFO,
                               .f_idx = PORT_ADMIN_STATUS,
                               .e_idx = port};

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (port > _cli_port_max_num)
    {
        return len;
    }

    if (CLI_SHOW_RUNNING_CONFIG == conf_mode_flag)
    {
        rc = cli_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, port, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&state, ptr_data, sizeof(state));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = db_getDataFromRawTable(&state, ptr_db_startup_head, req);
        if (MW_E_OK != rc)
        {
            return len;
        }
    }

    if (TRUE == state)
    {
        /* Do not show no shutdown*/
    }
    else
    {
        /* show shutdown */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "shutdown\n");
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_showSpeedCfg
 * PURPOSE:
 *      Show port speed config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showSpeedCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    CLI_PORT_SPEED_TYPE_T   port_speed = 0;
    UI16_T              port_setting = 0;
    UI32_T              len = 0;
    UI16_T              port = (UI16_T)port_id + 1;
    DB_REQUEST_TYPE_T   req = {.t_idx = PORT_CFG_INFO,
                               .f_idx = PORT_SETTINGS,
                               .e_idx = port};

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    if (port > _cli_port_max_num)
    {
        return len;
    }

    if (CLI_SHOW_RUNNING_CONFIG == conf_mode_flag)
    {
        rc = cli_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, port, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&port_setting, ptr_data, sizeof(port_setting));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = db_getDataFromRawTable((UI8_T *)&port_setting, ptr_db_startup_head, req);
        if (MW_E_OK != rc)
        {
            return len;
        }
    }

    port_speed = _cli_port_get_speed_from_portsetting(port_setting);
    if (CLI_PORT_SPEED_TYPE_AUTO == port_speed)
    {
        /* Do not show speed auto */
    }
    else if (CLI_PORT_SPEED_TYPE_10 == port_speed)
    {
        /* show speed 10 */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "speed 10\n");
    }
    else if (CLI_PORT_SPEED_TYPE_100 == port_speed)
    {
        /* show speed 100 */
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "speed 100\n");
    }

    return len;
}

/* FUNCTION NAME:   cli_cmd_port_checkValid
 * PURPOSE:
 *      Check if port is not configurable.
 *
 * INPUT:
 *      port_id              -- port id
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 * OUTPUT:
 *      ptr_out_res          -- TRUE(valid), FALSE(invalid)
 * RETURN:
 *      None
 *
 * NOTES:
 *      1. Loop state port it's not allowed to be configured.
 */
UI32_T cli_cmd_port_checkValid(
    UI32_T port_id,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    UI8_T *ptr_out_res)
{
    UI8_T               *ptr_data = NULL;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = 0;
    UI8_T               loop_state = 0;
    UI32_T              len = 0;

    if ((NULL == ptr_out_buf) || (NULL == ptr_out_res))
    {
        return len;
    }

    *ptr_out_res = FALSE;
    if ((PLAT_CPU_PORT == port_id) || (port_id > _cli_port_max_num))
    {
        return len;
    }

    /* Reqeust DB for port loop status information */
    rc = cli_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, port_id, &ptr_msg, &data_size, (void**)&ptr_data);
    if(MW_E_OK != rc)
    {
        CLI_LOG_ERROR("Failed to get port loop state.");
        return FALSE;
    }
    osapi_memcpy(&loop_state, ptr_data, sizeof(loop_state));
    MW_FREE(ptr_msg);

    if (0 != loop_state)
    {
        len = osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_LOOP_STATE_PORT);
        return len;
    }

    *ptr_out_res = TRUE;

    return len;
}
