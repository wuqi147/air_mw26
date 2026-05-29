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

/* FILE NAME:  mw_log.c
 * PURPOSE:
 * It provides log API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdarg.h>
#include "mw_cmd_util.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_LOG_MODULE_NAME_SYSTEM          "SYSTEM"
#define MW_LOG_MODULE_NAME_HTTPD           "HTTPD"
#define MW_LOG_MODULE_NAME_SYNCD           "SYNCD"
#define MW_LOG_MODULE_NAME_CMD             "CMD"
#define MW_LOG_MODULE_NAME_LED             "LED"
#define MW_LOG_MODULE_NAME_POE             "POE"
#define MW_LOG_MODULE_NAME_SNMP            "SNMP"
#define MW_LOG_MODULE_NAME_LAG             "LAG"
#define MW_LOG_MODULE_NAME_LACP            "LACP"
#define MW_LOG_MODULE_NAME_DHCPSNP         "DHCPSN"
#define MW_LOG_MODULE_NAME_LLDP            "LLDP"
#define MW_LOG_MODULE_NAME_QOS             "QOS"
#define MW_LOG_MODULE_NAME_SNTP            "SNTP"
#define MW_LOG_MODULE_NAME_STORM_CTRL      "STMCTL"
#define MW_LOG_MODULE_NAME_TRAFFIC_MON     "TRAMON"
#define MW_LOG_MODULE_NAME_CABLE_DIAG      "CBLDAG"
#define MW_LOG_MODULE_NAME_REMOTE_DEBUG    "RMTDBG"
#define MW_LOG_MODULE_NAME_ERPS            "ERPS"
#define MW_LOG_MODULE_NAME_DB              "DB"
#define MW_LOG_MODULE_NAME_PING            "PING"
#define MW_LOG_MODULE_NAME_MAC             "MAC"
#define MW_LOG_MODULE_NAME_LP              "LP"
#define MW_LOG_MODULE_NAME_IGMP_SNP        "IGSN"
#define MW_LOG_MODULE_NAME_MQTTD           "MQTTD"
#define MW_LOG_MODULE_NAME_VLAN            "VLAN"
#define MW_LOG_MODULE_NAME_PORT_SETTING    "PSET"
#define MW_LOG_MODULE_NAME_PORT_MIRROR     "PMIR"
#define MW_LOG_MODULE_NAME_BANDWIDTH       "BDWTH"
#define MW_LOG_MODULE_NAME_JUMBO_FRAME     "JFRAME"
#define MW_LOG_MODULE_NAME_STP             "STP"
#define MW_LOG_MODULE_NAME_SFP             "SFP"
#define MW_LOG_MODULE_NAME_IP6             "IP6"
#define MW_LOG_MODULE_NAME_CLI             "CLI"
#define MW_LOG_MODULE_NAME_TELNET          "TELNET"
#define MW_LOG_MODULE_NAME_SSH             "SSH"

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
static const C8_T *ptr_mw_log_module_name[MW_LOG_MODULE_LAST] = {
    MW_LOG_MODULE_NAME_SYSTEM,
    MW_LOG_MODULE_NAME_HTTPD,
    MW_LOG_MODULE_NAME_SYNCD,
    MW_LOG_MODULE_NAME_CMD,
    MW_LOG_MODULE_NAME_LED,
#ifdef AIR_SUPPORT_SNMP
    MW_LOG_MODULE_NAME_SNMP,
#endif
#ifdef AIR_SUPPORT_POE
    MW_LOG_MODULE_NAME_POE,
#endif
    MW_LOG_MODULE_NAME_LAG,
#ifdef AIR_SUPPORT_LACP
    MW_LOG_MODULE_NAME_LACP,
#endif
#ifdef AIR_SUPPORT_DHCP_SNOOP
    MW_LOG_MODULE_NAME_DHCPSNP,
#endif
#ifdef AIR_SUPPORT_LLDPD
    MW_LOG_MODULE_NAME_LLDP,
#endif
    MW_LOG_MODULE_NAME_QOS,
#ifdef AIR_SUPPORT_SNTP
    MW_LOG_MODULE_NAME_SNTP,
#endif
    MW_LOG_MODULE_NAME_STORM_CTRL,
    MW_LOG_MODULE_NAME_TRAFFIC_MON,
#ifdef AIR_SUPPORT_CABLE_DIAG
    MW_LOG_MODULE_NAME_CABLE_DIAG,
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    MW_LOG_MODULE_NAME_REMOTE_DEBUG,
#endif
#ifdef AIR_SUPPORT_ERPS
    MW_LOG_MODULE_NAME_ERPS,
#endif
    MW_LOG_MODULE_NAME_DB,
#ifdef AIR_SUPPORT_ICMP_CLIENT
    MW_LOG_MODULE_NAME_PING,
#endif
    MW_LOG_MODULE_NAME_MAC,
#ifdef AIR_SUPPORT_LP
    MW_LOG_MODULE_NAME_LP,
#endif
#ifdef AIR_SUPPORT_IGMP_SNP
    MW_LOG_MODULE_NAME_IGMP_SNP,
#endif
#ifdef AIR_SUPPORT_MQTTD
    MW_LOG_MODULE_NAME_MQTTD,
#endif
    MW_LOG_MODULE_NAME_VLAN,
    MW_LOG_MODULE_NAME_PORT_SETTING,
    MW_LOG_MODULE_NAME_PORT_MIRROR,
    MW_LOG_MODULE_NAME_BANDWIDTH,
    MW_LOG_MODULE_NAME_JUMBO_FRAME,
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    MW_LOG_MODULE_NAME_STP,
#endif
#ifdef AIR_SUPPORT_SFP
    MW_LOG_MODULE_NAME_SFP,
#endif
#ifdef AIR_SUPPORT_IPV6
    MW_LOG_MODULE_NAME_IP6,
#endif
#ifdef AIR_SUPPORT_CLI
    MW_LOG_MODULE_NAME_CLI,
#endif
#ifdef AIR_SUPPORT_TELNET
    MW_LOG_MODULE_NAME_TELNET,
#endif
#ifdef AIR_SUPPORT_SSH
    MW_LOG_MODULE_NAME_SSH,
#endif
};

static UI8_T _mw_log_level[MW_LOG_MODULE_LAST] = {
    MW_LOG_LEVEL_ERROR,     /* SYSTEM */
    MW_LOG_LEVEL_ERROR,     /* HTTPD */
    MW_LOG_LEVEL_ERROR,     /* SYNCD */
    MW_LOG_LEVEL_ERROR,     /* CMD */
    MW_LOG_LEVEL_ERROR,     /* LED */
#ifdef AIR_SUPPORT_SNMP
    MW_LOG_LEVEL_ERROR,     /* SNMP */
#endif
#ifdef AIR_SUPPORT_POE
    MW_LOG_LEVEL_ERROR,     /* POE */
#endif
    MW_LOG_LEVEL_ERROR,     /* LAG */
#ifdef AIR_SUPPORT_LACP
    MW_LOG_LEVEL_ERROR,     /* LACP */
#endif
#ifdef AIR_SUPPORT_DHCP_SNOOP
    MW_LOG_LEVEL_ERROR,     /* DHCP SNP */
#endif
#ifdef AIR_SUPPORT_LLDPD
    MW_LOG_LEVEL_ERROR,     /* LLDP */
#endif
    MW_LOG_LEVEL_ERROR,     /* QOS */
#ifdef AIR_SUPPORT_SNTP
    MW_LOG_LEVEL_ERROR,     /* SNTP */
#endif
    MW_LOG_LEVEL_ERROR,     /* STROM CTRL */
    MW_LOG_LEVEL_ERROR,     /* TRAFFIC MONITOR */
#ifdef AIR_SUPPORT_CABLE_DIAG
    MW_LOG_LEVEL_ERROR,     /* CABLE DIAG */
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    MW_LOG_LEVEL_ERROR,     /* REMOTE DEBUG */
#endif
#ifdef AIR_SUPPORT_ERPS
    MW_LOG_LEVEL_ERROR,     /* ERPS */
#endif
    MW_LOG_LEVEL_ERROR,     /* DB */
#ifdef AIR_SUPPORT_ICMP_CLIENT
    MW_LOG_LEVEL_ERROR,     /* ICMP CLIENT */
#endif
    MW_LOG_LEVEL_ERROR,     /* MAC */
#ifdef AIR_SUPPORT_LP
    MW_LOG_LEVEL_ERROR,     /* LP */
#endif
#ifdef AIR_SUPPORT_IGMP_SNP
    MW_LOG_LEVEL_ERROR,     /* IGMP SNP */
#endif
#ifdef AIR_SUPPORT_MQTTD
    MW_LOG_LEVEL_ERROR,     /* MQTTD */
#endif
    MW_LOG_LEVEL_ERROR,     /* VLAN */
    MW_LOG_LEVEL_ERROR,     /* PORT SETTING */
    MW_LOG_LEVEL_ERROR,     /* PORT MIRROR */
    MW_LOG_LEVEL_ERROR,     /* BANDWIDTH CONTROL */
    MW_LOG_LEVEL_ERROR,     /* JUMBO FRAME */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    MW_LOG_LEVEL_ERROR,     /* STP */
#endif
#ifdef AIR_SUPPORT_SFP
    MW_LOG_LEVEL_ERROR,     /* SFP */
#endif
#ifdef AIR_SUPPORT_IPV6
    MW_LOG_LEVEL_ERROR,     /* IP6 */
#endif
#ifdef AIR_SUPPORT_CLI
    MW_LOG_LEVEL_ERROR,     /* CLI */
#endif
#ifdef AIR_SUPPORT_TELNET
    MW_LOG_LEVEL_ERROR,     /* TELNET */
#endif
#ifdef AIR_SUPPORT_SSH
    MW_LOG_LEVEL_ERROR,     /* SSH */
#endif
};

static const C8_T *_ptr_mw_log_level_str[] = { "OFF", "ERR", "WRN", "INF", "DBG" };
static MW_LOG_SETTING_T _mw_log_settings = {
    .ptr_module = ptr_mw_log_module_name,
    .ptr_log_level = _mw_log_level,
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
    .log_output_type = 0
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
static BOOL_T
_mw_log_check_send_to_remote(
    const MW_LOG_MODULE_T module)
{
    if ((0 != _mw_log_settings.log_output_type) &&
    ((MW_LOG_MODULE_REMOTE_DEBUG != module)
#ifdef AIR_SUPPORT_TELNET
     && (MW_LOG_MODULE_TELNET != module)
#endif
#ifdef AIR_SUPPORT_SSH
     && (MW_LOG_MODULE_SSH != module)
#endif
     ))
    {
        return TRUE;
    }

    return FALSE;
}
#endif
/* EXPORTED SUBPROGRAM BODIES
 */

void
mw_log_print(
    const MW_LOG_MODULE_T module,
    MW_LOG_LEVEL_T level,
    BOOL_T format,
    const C8_T *ptr_fmt,
    ...)
{
    va_list args;

    if (module >= MW_LOG_MODULE_LAST)
    {
        return;
    }

    if ((level > _mw_log_level[module]) || (MW_LOG_LEVEL_OFF == _mw_log_level[module]))
    {
        return;
    }

    if (TRUE == format)
    {
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
        if (TRUE == _mw_log_check_send_to_remote(module))
        {
            MW_LOG_PRINTF("[%s][%s]", _ptr_mw_log_level_str[level], ptr_mw_log_module_name[module]);
        }
        else
#endif
        {
            MW_LOG_CONSOLE_PRINTF("[%s][%s]", _ptr_mw_log_level_str[level], ptr_mw_log_module_name[module]);
        }
    }

    va_start(args, ptr_fmt);

#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
    if (TRUE == _mw_log_check_send_to_remote(module))
    {
        osapi_xvprintf(ptr_fmt, args);
    }
    else
#endif
    {
        osapi_console_vprintf(ptr_fmt, args);
    }

    va_end(args);

    if (TRUE == format)
    {
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
        if (TRUE == _mw_log_check_send_to_remote(module))
        {
            MW_LOG_PRINTF("\r\n");
        }
        else
#endif
        {
            MW_LOG_CONSOLE_PRINTF("\r\n");
        }
    }
}

MW_LOG_SETTING_T *
mw_log_get_setttings(
    void)
{
    return &_mw_log_settings;
}

MW_ERROR_NO_T
mw_log_show_settings(
    void)
{
    UI8_T i;

#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
    MW_CMD_OUTPUT("\n");
    MW_CMD_OUTPUT("MW Log Output:\n");
#ifdef AIR_SUPPORT_TELNET
    MW_CMD_OUTPUT("Telnet: %s\n", (_mw_log_settings.log_output_type & MW_LOG_OUTPUT_TYPE_BITMAP_TELNET) ? "enable" : "disable");
#endif
#ifdef AIR_SUPPORT_SSH
    MW_CMD_OUTPUT("SSH: %s\n", (_mw_log_settings.log_output_type & MW_LOG_OUTPUT_TYPE_BITMAP_SSH) ? "enable" : "disable");
#endif
#endif
    MW_CMD_OUTPUT("\n");
    MW_CMD_OUTPUT("MW Log Level: \n");
    for (i = 0;i < MW_LOG_MODULE_LAST;i++)
    {
        MW_CMD_OUTPUT("[%2u] Module: %6s, Level: %d(%s)\n", i + 1, ptr_mw_log_module_name[i], _mw_log_level[i], _ptr_mw_log_level_str[_mw_log_level[i]]);
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
mw_log_get_level(
    const MW_LOG_MODULE_T module,
    MW_LOG_LEVEL_T *ptr_level)
{
    if (module >= MW_LOG_MODULE_LAST)
    {
        MW_CMD_OUTPUT("%s: Invalid module(%d)\n", __func__, module);
        return MW_E_BAD_PARAMETER;
    }

    *ptr_level = _mw_log_level[module];

    return MW_E_OK;
}

MW_ERROR_NO_T
mw_log_set_level(
    const MW_LOG_MODULE_T module,
    MW_LOG_LEVEL_T level)
{
    if (module >= MW_LOG_MODULE_LAST)
    {
        MW_CMD_OUTPUT("%s: Invalid module(%d)\n", __func__, module);
        return MW_E_BAD_PARAMETER;
    }

    if (level >= MW_LOG_LEVEL_LAST)
    {
        MW_CMD_OUTPUT("%s: Invalid log level(%d)\n", __func__, level);
        return MW_E_BAD_PARAMETER;
    }

    _mw_log_level[module] = level;

    return MW_E_OK;
}

MW_ERROR_NO_T
mw_log_set_global_level(
    MW_LOG_LEVEL_T level)
{
    UI8_T i;

    if (level >= MW_LOG_LEVEL_LAST)
    {
        MW_CMD_OUTPUT("%s: Invalid log level(%d)\n", __func__, level);
        return MW_E_BAD_PARAMETER;
    }
    else
    {
        for (i = 0; i < MW_LOG_MODULE_LAST; i++)
        {
            _mw_log_level[i] = level;
        }
    }

    return MW_E_OK;
}

#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
MW_ERROR_NO_T
mw_log_set_global_output_type(
    UI32_T type_bitmap)
{
    if (type_bitmap >= MW_LOG_OUTPUT_TYPE_BITMAP_LAST)
    {
        MW_CMD_OUTPUT("Invalid output type mask(0x%x)\n", type_bitmap);
        return MW_E_BAD_PARAMETER;
    }

    _mw_log_settings.log_output_type = type_bitmap;

    MW_CMD_OUTPUT("\nSet MW log output:\n");
#ifdef AIR_SUPPORT_TELNET
    MW_CMD_OUTPUT("Telnet: %s\n", (_mw_log_settings.log_output_type & MW_LOG_OUTPUT_TYPE_BITMAP_TELNET) ? "enable" : "disable");
#endif
#ifdef AIR_SUPPORT_SSH
    MW_CMD_OUTPUT("SSH: %s\n", (_mw_log_settings.log_output_type & MW_LOG_OUTPUT_TYPE_BITMAP_SSH) ? "enable" : "disable");
#endif

    return MW_E_OK;
}

UI8_T
mw_log_get_global_output_type(
    void)
{
    return _mw_log_settings.log_output_type;
}
#endif