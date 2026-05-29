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

/* FILE NAME:   cli_cmd_ipsetting.c
 * PURPOSE:
 *      Implement the ip address setting command and default gateway setting command for CLI.
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdarg.h>
#include <ctype.h>
#include "cli_internal.h"
#include "cli_db.h"
#include "db_api.h"
#include "mw_cmd_util.h"
#include "mw_cmd_parser.h"
#include "osapi_message.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "osapi_mutex.h"
#include "web.h"
#include "air_chipscu.h"
#include "inet_utils.h"
#include "lwip/ip_addr.h"
#include "ip4_addr_util.h"
#include "cli_cmd_ipsetting.h"
#include "lwip/inet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CLI_CMD_DEFAULT_VLAN_ID    (1)

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
/* FUNCTION NAME: cli_cmd_ipSetting_setIp
 * PURPOSE:
 *      Management ip command handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      no_flag     -- Flag indicating whether the command is no.
 *      argc        -- Argument count.
 *      pptr_argv   -- Argument vector.
 *      token_idx   -- Token index.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_setIp(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI16_T data_size = 0;
    C8_T *ptr_cgi_str = NULL;
    UI8_T *ptr_data = NULL;
    DB_MSG_T *ptr_msg = NULL;
    DB_SYS_INFO_T *ptr_curr_sys_info = NULL;
    DB_SYS_INFO_T *ptr_factory_sys_info = NULL;

    C8_T ip_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T mask_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T gw_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T dns_str[IP4ADDR_STRLEN_MAX] = {0};
    const C8_T *ptr_ip_for_cgi = NULL;
    const C8_T *ptr_mask_for_cgi = NULL;

    UI32_T ip_address = 0;
    UI32_T ip_mask = 0;
    UI32_T ip_gw = 0;
    UI32_T ip_dns = 0;
    UI32_T len = 0;
    ip4_addr_t addr;
    ip4_addr_t parsed_ip;
    ip4_addr_t parsed_mask;

    if (TRUE == no_flag)
    {
        if (MW_E_OK != osapi_calloc(sizeof(DB_SYS_INFO_T), CLI_TASK_NAME, (void **)&ptr_factory_sys_info))
        {
            CLI_LOG_ERROR("Failed to allocate memory for ptr_factory_sys_info");
            return len;
        }

        /* Restore to factory default IP & mask */
        if (MW_E_OK != dbapi_getFactoryDefault(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &data_size, (void **)&ptr_data))
        {
            CLI_LOG_WARN("Failed to get default DB data");
            MW_FREE(ptr_factory_sys_info);
            return len;
        }

        osapi_memcpy(ptr_factory_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));
        MW_FREE(ptr_data);

        ip_address = ptr_factory_sys_info->static_ip;
        ip_mask = ptr_factory_sys_info->static_mask;
        MW_FREE(ptr_factory_sys_info);

        osapi_memset(&addr, 0, sizeof(addr));
        addr.addr = ip_address;
        inet_ntoa_r(addr, ip_str, IP4ADDR_STRLEN_MAX);
        ip_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

        osapi_memset(&addr, 0, sizeof(addr));
        addr.addr = ip_mask;
        inet_ntoa_r(addr, mask_str, IP4ADDR_STRLEN_MAX);
        mask_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

        ptr_ip_for_cgi = ip_str;
        ptr_mask_for_cgi = mask_str;

        CLI_LOG_INFO("Restore management IP to factory default: ip=0x%x mask=0x%x (%s/%s)",
                     ip_address, ip_mask, ptr_ip_for_cgi, ptr_mask_for_cgi);
    }
    else
    {
        /*
         * Command format (relative to token_idx):
         *   [token_idx + 0] management
         *   [token_idx + 1] vlan
         *   [token_idx + 2] ip-address
         *   [token_idx + 3] A.B.C.D (IP)
         *   [token_idx + 4] mask
         *   [token_idx + 5] A.B.C.D (Mask)
         */
        if ((token_idx + 5) >= argc)
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }

        /* Verify the format of the subnet mask and the validity of the subnet mask */
        if ((0 == ip4addr_aton(pptr_argv[token_idx + 5], &parsed_mask)) ||
            (FALSE == ip4_addr_netmask_isValid(&parsed_mask)))
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INVALID_IPV4_SUBNET_MASK);
            return len;
        }

        /* Verify the format of the IP address and check special IP address */
        if ((0 == ip4addr_aton(pptr_argv[token_idx + 3], &parsed_ip)) ||
            (FALSE == ip4_addr_ipAddr_isValid(&parsed_ip, &parsed_mask)))
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INVALID_IPV4_ADDR);
            return len;
        }

        /* Reuse parsed_ip / parsed_mask instead of calling getIpaddr() again */
        ip_address = ip4_addr_get_u32(&parsed_ip);
        ip_mask = ip4_addr_get_u32(&parsed_mask);

        ptr_ip_for_cgi = pptr_argv[token_idx + 3];
        ptr_mask_for_cgi = pptr_argv[token_idx + 5];

        CLI_LOG_INFO("Set management IP: ip=0x%x mask=0x%x (%s/%s)",
                     ip_address, ip_mask, ptr_ip_for_cgi, ptr_mask_for_cgi);
    }

    /* Get current ip info from DB */
    if (MW_E_OK != osapi_calloc(sizeof(DB_SYS_INFO_T), CLI_TASK_NAME, (void **)&ptr_curr_sys_info))
    {
        CLI_LOG_ERROR("Failed to allocate memory for ptr_curr_sys_info");
        return len;
    }
    if (MW_E_OK != cli_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
    {
        MW_FREE(ptr_curr_sys_info);
        CLI_LOG_WARN("Failed to get data from DB");
        return len;
    }
    osapi_memcpy(ptr_curr_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));
    MW_FREE(ptr_msg);

    ip_gw = ptr_curr_sys_info->static_gw;
    ip_dns = ptr_curr_sys_info->static_dns;

    osapi_memset(&addr, 0, sizeof(addr));
    addr.addr = ip_gw;
    inet_ntoa_r(addr, gw_str, IP4ADDR_STRLEN_MAX);
    gw_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    osapi_memset(&addr, 0, sizeof(addr));
    addr.addr = ip_dns;
    inet_ntoa_r(addr, dns_str, IP4ADDR_STRLEN_MAX);
    dns_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        MW_FREE(ptr_curr_sys_info);
        CLI_LOG_ERROR("Failed to allocate memory for management IP CGI string");
        return len;
    }

    osapi_snprintf(ptr_cgi_str,  CLI_CGI_STR_MAX_LEN,
                   "/ip_setting.cgi?dhcpSetting=%d&autodnsSetting=%d&ip_address=%s&ip_netmask=%s&ip_gateway=%s&ip_dns=%s&mgmt_vlan=%d",
                   ptr_curr_sys_info->dhcp_enable,
                   ptr_curr_sys_info->autodns_enable,
                   ptr_ip_for_cgi,
                   ptr_mask_for_cgi,
                   gw_str,
                   dns_str,
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
                   ptr_curr_sys_info->mgmt_vlan
#else
                   CLI_CMD_DEFAULT_VLAN_ID
#endif
                   );

    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_curr_sys_info);
    MW_FREE(ptr_cgi_str);

    return len;
}

/* FUNCTION NAME: cli_cmd_ipSetting_SetGw
 * PURPOSE:
 *      IP default gateway command handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      no_flag     -- Flag indicating whether the command is no.
 *      argc        -- Argument count.
 *      pptr_argv   -- Argument vector.
 *      token_idx   -- Token index.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_SetGw(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI16_T data_size = 0;
    UI32_T *ptr_gw_default = NULL;
    C8_T *ptr_cgi_str = NULL;
    UI8_T *ptr_data = NULL;
    DB_MSG_T *ptr_msg = NULL;
    DB_SYS_INFO_T *ptr_curr_sys_info = NULL;

    C8_T ip_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T mask_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T dns_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T gw_str[IP4ADDR_STRLEN_MAX] = {0};
    const C8_T *ptr_gw_for_cgi = NULL;

    UI32_T ip_address = 0;
    UI32_T ip_mask = 0;
    UI32_T ip_dns = 0;
    UI32_T ip_gateway = 0;
    UI32_T len = 0;
    ip4_addr_t addr;
    ip4_addr_t parsed_gw;
    ip4_addr_t gw_addr;
    ip4_addr_t mask_addr;

    if (TRUE == no_flag)
    {
        /* Restore to factory default gateway */
        if (MW_E_OK != dbapi_getFactoryDefault(SYS_INFO, SYS_STATIC_IP_GW, DB_ALL_ENTRIES, &data_size, (void **)&ptr_gw_default))
        {
            CLI_LOG_WARN("Failed to get default DB gateway");
            return len;
        }

        if (data_size != sizeof(UI32_T))
        {
            CLI_LOG_WARN("Unexpected size for default gw: %u", data_size);
            MW_FREE(ptr_gw_default);
            return len;
        }

        ip_gateway = *ptr_gw_default;
        MW_FREE(ptr_gw_default);
        ptr_gw_default = NULL;

        ip4_addr_set_u32(&addr, ip_gateway);
        inet_ntoa_r(addr, gw_str, IP4ADDR_STRLEN_MAX);
        gw_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

        ptr_gw_for_cgi = gw_str;
        CLI_LOG_INFO("Restore ip gw to factory default: ip=0x%x (%s)", ip_gateway, ptr_gw_for_cgi);
    }
    else
    {
        /*
         * Command format (relative to token_idx):
         *   [token_idx + 0] ip
         *   [token_idx + 1] default-gateway
         *   [token_idx + 2] A.B.C.D (gateway)
         */
        if ((token_idx + 2) >= argc)
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }

        /* Verify the format of the default gateway */
        if (0 == ip4addr_aton(pptr_argv[token_idx + 2], &parsed_gw))
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INVALID_IPV4_GATEWAY);
            return len;
        }

        /* Reuse parsed_gw instead of calling getIpaddr() again */
        ip_gateway = ip4_addr_get_u32(&parsed_gw);

        ptr_gw_for_cgi = pptr_argv[token_idx + 2];
        CLI_LOG_INFO("Set ip gw: ip=0x%x (%s)", ip_gateway, ptr_gw_for_cgi);
    }

    /* Get current ip info from DB */
    if (MW_E_OK != osapi_calloc(sizeof(DB_SYS_INFO_T), CLI_TASK_NAME, (void **)&ptr_curr_sys_info))
    {
        CLI_LOG_ERROR("Failed to allocate memory for ptr_curr_sys_info");
        return len;
    }
    if (MW_E_OK != cli_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
    {
        MW_FREE(ptr_curr_sys_info);
        CLI_LOG_WARN("Failed to get data from DB");
        return len;
    }
    osapi_memcpy(ptr_curr_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));
    MW_FREE(ptr_msg);

    ip_address = ptr_curr_sys_info->static_ip;
    ip_mask = ptr_curr_sys_info->static_mask;
    ip_dns = ptr_curr_sys_info->static_dns;

    ip4_addr_set_u32(&gw_addr, ip_gateway);
    ip4_addr_set_u32(&mask_addr, ip_mask);

    if ((FALSE == no_flag) && (FALSE == ip4_addr_gateway_isValid(&gw_addr, &mask_addr)))
    {
        MW_FREE(ptr_curr_sys_info);
        CLI_LOG_DEBUG("Invalid gateway IP address");
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, CLI_ERR_INVALID_IPV4_GATEWAY);
        return len;
    }

    ip4_addr_set_u32(&addr, ip_address);
    inet_ntoa_r(addr, ip_str, IP4ADDR_STRLEN_MAX);
    ip_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    inet_ntoa_r(mask_addr, mask_str, IP4ADDR_STRLEN_MAX);
    mask_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    ip4_addr_set_u32(&addr, ip_dns);
    inet_ntoa_r(addr, dns_str, IP4ADDR_STRLEN_MAX);
    dns_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        MW_FREE(ptr_curr_sys_info);
        CLI_LOG_ERROR("Failed to allocate memory for ip gw CGI string");
        return len;
    }

    osapi_snprintf(ptr_cgi_str,  CLI_CGI_STR_MAX_LEN,
                   "/ip_setting.cgi?dhcpSetting=%d&autodnsSetting=%d&ip_address=%s&ip_netmask=%s&ip_gateway=%s&ip_dns=%s&mgmt_vlan=%d",
                   ptr_curr_sys_info->dhcp_enable,
                   ptr_curr_sys_info->autodns_enable,
                   ip_str,
                   mask_str,
                   ptr_gw_for_cgi,
                   dns_str,
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
                   ptr_curr_sys_info->mgmt_vlan
#else
                   CLI_CMD_DEFAULT_VLAN_ID
#endif
                   );

    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_curr_sys_info);
    MW_FREE(ptr_cgi_str);

    return len;
}

/* FUNCTION NAME: cli_cmd_ipSetting_showIp
 * PURPOSE:
 *      Management ip show handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      show_mode   -- Indicate show mode, 0: running-config, 1: startup-config.
 *      instance_id -- Instance ID
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_showIp(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode)
{
    UI16_T data_size = 0;
    DB_SYS_INFO_T *ptr_sys_info = NULL;
    UI8_T *ptr_data = NULL;
    UI32_T ip_address = 0;
    UI32_T ip_mask = 0;
    C8_T ip_str[IP4ADDR_STRLEN_MAX] = {0};
    C8_T mask_str[IP4ADDR_STRLEN_MAX] = {0};
    ip4_addr_t addr;
    UI32_T len = 0;

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "!\n");

    if (CLI_SHOW_RUNNING_CONFIG == show_mode)
    {
        DB_MSG_T *ptr_msg = NULL;

        if (MW_E_OK != osapi_calloc(sizeof(DB_SYS_INFO_T), CLI_TASK_NAME, (void **)&ptr_sys_info))
        {
            CLI_LOG_ERROR("Failed to allocate memory for ptr_sys_info");
            return len;
        }

        /* Get running IP address and IP mask */
        if (MW_E_OK != cli_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
        {
            CLI_LOG_WARN("Failed to get running SYS_INFO");
            MW_FREE(ptr_sys_info);
            return len;
        }

        osapi_memcpy(ptr_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));
        MW_FREE(ptr_msg);

        ip_address = ptr_sys_info->static_ip;
        ip_mask = ptr_sys_info->static_mask;
        MW_FREE(ptr_sys_info);
    }
    else
    {
        if (MW_E_OK != osapi_calloc(sizeof(DB_SYS_INFO_T), CLI_TASK_NAME, (void **)&ptr_sys_info))
        {
            CLI_LOG_ERROR("Failed to allocate memory for ptr_sys_info");
            return len;
        }

        /* Get startup IP address and IP mask */
        if (MW_E_OK != dbapi_getStartUp(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &data_size, (void **)&ptr_data))
        {
            CLI_LOG_WARN("Failed to get startup SYS_INFO");
            MW_FREE(ptr_sys_info);
            return len;
        }

        osapi_memcpy(ptr_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));
        MW_FREE(ptr_data);

        ip_address = ptr_sys_info->static_ip;
        ip_mask = ptr_sys_info->static_mask;
        MW_FREE(ptr_sys_info);
    }

    osapi_memset(&addr, 0, sizeof(addr));
    addr.addr = ip_address;
    inet_ntoa_r(addr, ip_str, IP4ADDR_STRLEN_MAX);
    ip_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    osapi_memset(&addr, 0, sizeof(addr));
    addr.addr = ip_mask;
    inet_ntoa_r(addr, mask_str, IP4ADDR_STRLEN_MAX);
    mask_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    if (len < out_buf_len)
    {
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "management vlan ip-address %s mask %s\n", ip_str, mask_str);
    }

    if (len < out_buf_len)
    {
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "no management vlan ip\n");
    }


    return len;
}

/* FUNCTION NAME: cli_cmd_ipSetting_showGw
 * PURPOSE:
 *      IP default gateway show handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      show_mode   -- Indicate show mode, 0: running-config, 1: startup-config.
 *      instance_id -- Instance ID
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_showGw(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode)
{
    UI16_T data_size = 0;
    UI32_T *ptr_default_gw = NULL;
    UI32_T ip_gw_default = 0;
    C8_T gw_str[IP4ADDR_STRLEN_MAX] = {0};
    ip4_addr_t addr;
    UI32_T len = 0;

    if (CLI_SHOW_RUNNING_CONFIG == show_mode)
    {
        DB_MSG_T *ptr_msg = NULL;

        /* Get running default gateway */
        if (MW_E_OK != cli_queue_getData(SYS_INFO, SYS_STATIC_IP_GW, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_default_gw))
        {
            CLI_LOG_WARN("Failed to get running default gw");
            return len;
        }

        if (data_size != sizeof(UI32_T))
        {
            CLI_LOG_WARN("Unexpected size for running default gw: %u", data_size);
            MW_FREE(ptr_msg);
            return len;
        }

        ip_gw_default = *ptr_default_gw;
        MW_FREE(ptr_msg);
        ptr_default_gw = NULL;
    }
    else
    {
        /* Get startup default gateway */
        if (MW_E_OK != dbapi_getStartUp(SYS_INFO, SYS_STATIC_IP_GW, DB_ALL_ENTRIES, &data_size, (void **)&ptr_default_gw))
        {
            CLI_LOG_WARN("Failed to get startup default gw");
            return len;
        }

        if (data_size != sizeof(UI32_T))
        {
            CLI_LOG_WARN("Unexpected size for startup default gw: %u", data_size);
            MW_FREE(ptr_default_gw);
            return len;
        }

        ip_gw_default = *ptr_default_gw;
        MW_FREE(ptr_default_gw);
        ptr_default_gw = NULL;
    }

    osapi_memset(&addr, 0, sizeof(addr));
    addr.addr = ip_gw_default;
    inet_ntoa_r(addr, gw_str, IP4ADDR_STRLEN_MAX);
    gw_str[IP4ADDR_STRLEN_MAX - 1] = '\0';

    if (len < out_buf_len)
    {
        if (0 != ip_gw_default)
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "ip default-gateway %s\n", gw_str);
        }
        else
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "no ip default-gateway\n");
        }
    }

    return len;
}

