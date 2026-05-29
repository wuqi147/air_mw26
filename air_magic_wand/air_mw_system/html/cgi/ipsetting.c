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

/* FILE NAME:   ipsetting.c
 * PURPOSE:
 *      CGI and SSI function of ip setting web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "web.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "lwip/ip_addr.h"
#include "inet_utils.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/priv/nd6_priv.h"
#include "lwip/nd6.h"
#include "lwip/ip6_addr.h"
#endif

sys_info_t sys_info;

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

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_dhcpState_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK,dhcp_err = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    DB_MSG_T *ptr_db_dhcp = NULL;
    u16_t db_size = 0;
    void *db_data = NULL;
    C8_T tmpbuf[36] = {0};
    UI16_T len = 0;
    void *ptr_dhcp_data = NULL;

    err = httpd_queue_getData(SYS_OPER_INFO, SYS_OPER_IP_ADDR, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &db_data);
    dhcp_err = httpd_queue_getData(SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &ptr_db_dhcp, &db_size, &ptr_dhcp_data);

    if(MW_E_OK == err && MW_E_OK == dhcp_err)
    {
        len += snprintf(tmpbuf, sizeof(tmpbuf), "%d.%d.%d.%d,dhcp_enable:%d",
                            (0xFF & ((UI8_T*)db_data)[0]),
                            (0xFF & ((UI8_T*)db_data)[1]),
                            (0xFF & ((UI8_T*)db_data)[2]),
                            (0xFF & ((UI8_T*)db_data)[3]),
                            (0xFF & *((UI8_T *)ptr_dhcp_data ))
                       );
        err = send_format_response(&len, ptr_pcb, apiflags, tmpbuf);
        *ptr_length = (int)len;
    }
    MW_FREE(ptr_db_msg);
    MW_FREE(ptr_db_dhcp);
    return err;
}

#ifdef AIR_SUPPORT_IPV6
MW_ERROR_NO_T
ssi_get_dadResult_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    u16_t db_size = 0;
    void *ptr_db_data = NULL;
    u16_t len = 0;

    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: enter ssi_get_dadResult_xmlHandle\n", __func__, __LINE__);

    err = httpd_queue_getData(SYS_OPER_INFO, SYS_IP6_DAD_RESULT, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);

    if(err == MW_E_OK)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: dad result: %d\n", __func__, __LINE__, *(UI8_T *)ptr_db_data);
        err = send_format_response(&len, ptr_pcb, apiflags, "%d", *(UI8_T *)ptr_db_data);
        *ptr_length = (int)len;
        MW_FREE(ptr_db_msg);
    }
    return err;
}
#endif

MW_ERROR_NO_T cgi_set_handle_ipsetting(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    int     i = 0;
    UI32_T  temp = 0;
    int     rc = MW_E_OK;
    u8_t    dhcpSetting = 0, autodnsSetting = 0;
    u32_t   ip_address = 0, ip_netmask = 0, ip_gateway = 0, ip_dns = 0;
#ifdef AIR_SUPPORT_IPV6
    ip6_addr_t manual_ip6_address;
    ip6_addr_t manual_ip6_default_gw;
    u8_t manual_ip6_prefix_len = 0;
    u8_t isManualIp6AddrUpdate = 0, isManualIp6DefaultGwUpdate = 0;
    u8_t isManualIp6ModeUpdate = 0;
    u8_t isAutoIp6ModeUpdate = 0;
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    u16_t   mgmt_vlan;
#endif
    u8_t    isDhcpSettingUpdate = 0, isAutodnsSettingUpdate = 0,isIpAddressUpdate = 0, isIpNetmaskUpdate = 0, isIpGatewayUpdate = 0, isIpDnsUpdate = 0;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    u8_t    isMgmtVlanUpdate = 0;
#endif
#ifdef AIR_SUPPORT_IPV6
    ip6_addr_set_zero(&manual_ip6_address);
    ip6_addr_set_zero(&manual_ip6_default_gw);
#endif

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if((!strcmp(pcParam[i], "dhcpSetting")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp))){
            dhcpSetting = (UI8_T)temp;
            isDhcpSettingUpdate = TRUE;
        }
        if((!strcmp(pcParam[i], "autodnsSetting")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp))){
            autodnsSetting = (UI8_T)temp;
            isAutodnsSettingUpdate = TRUE;
        }
        if(!strcmp(pcParam[i], "ip_address")){
            ip_address = getIpaddr(pcValue[i]);
            isIpAddressUpdate = TRUE;
        }
        if(!strcmp(pcParam[i], "ip_netmask")){
            ip_netmask = getIpaddr(pcValue[i]);
            isIpNetmaskUpdate = TRUE;
        }
        if(!strcmp(pcParam[i], "ip_gateway")){
            ip_gateway = getIpaddr(pcValue[i]);
            isIpGatewayUpdate = TRUE;
        }
        if(!strcmp(pcParam[i], "ip_dns")){
            ip_dns = getIpaddr(pcValue[i]);
            isIpDnsUpdate = TRUE;
        }
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
        if((!strcmp(pcParam[i], "mgmt_vlan")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp))){
            mgmt_vlan = (UI16_T)temp;
            isMgmtVlanUpdate = TRUE;
        }
#endif
#ifdef AIR_SUPPORT_IPV6
        if(0 == osapi_strcmp(pcParam[i], "manual_ip6_prefix_len"))
        {
            manual_ip6_prefix_len = atoi(pcValue[i]);
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update IP6 prefix len: %s", __func__, __LINE__, pcValue[i]);
        }

        if(0 == osapi_strcmp(pcParam[i], "ipv6_mode"))
        {
            if (atoi(pcValue[i]) == 1)
            {
                isManualIp6ModeUpdate = TRUE;
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update IP6 mode: %s", __func__, __LINE__, pcValue[i]);
            }
            else
            {
                isAutoIp6ModeUpdate = TRUE;
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update IP6 mode: %s", __func__, __LINE__, pcValue[i]);
            }
        }

        if(0 == osapi_strcmp(pcParam[i], "manual_ip6_address"))
        {
            char *ptr_decoded = NULL;

            rc = osapi_calloc(IPV6_ADDR_LEN, "IP6", (void **)&ptr_decoded);
            if (MW_E_OK != rc)
            {
                CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_decoded osapi_calloc() failed", __func__, __LINE__);
                return rc;
            }
            url_decode(pcValue[i], ptr_decoded);
            if (ip6addr_aton(ptr_decoded, &manual_ip6_address))
            {
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update manual IP6 address: %s", __func__, __LINE__, *ptr_decoded);
                isManualIp6AddrUpdate = TRUE;
            }
            else
            {
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Delete manual IP address: %s", __func__, __LINE__, *ptr_decoded);
                isManualIp6AddrUpdate = FALSE;
            }
            MW_FREE(ptr_decoded);
        }

        if(0 == osapi_strcmp(pcParam[i], "manual_ip6_gw"))
        {
            char *ptr_decoded = NULL;

            rc = osapi_calloc(IPV6_ADDR_LEN, "IP6", (void **)&ptr_decoded);
            if (MW_E_OK != rc)
            {
                CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_decoded osapi_calloc() failed", __func__, __LINE__);
                return rc;
            }
            url_decode(pcValue[i], ptr_decoded);
            if (ip6addr_aton(ptr_decoded, &manual_ip6_default_gw))
            {
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update IP6 static default gateway: %s", __func__, __LINE__, *ptr_decoded);
                isManualIp6DefaultGwUpdate = TRUE;
            }
            else
            {
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Delete IP6 static default gateway: %s", __func__, __LINE__, *ptr_decoded);
                isManualIp6DefaultGwUpdate = FALSE;
            }
            MW_FREE(ptr_decoded);
        }
#endif
    }

    if(TRUE == isAutodnsSettingUpdate)
    {
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_AUTODNS_ENABLE, DB_ALL_ENTRIES, &autodnsSetting, sizeof(autodnsSetting));
    }
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    if(TRUE == isMgmtVlanUpdate)
    {
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_MGMT_VLAN, DB_ALL_ENTRIES, &mgmt_vlan, sizeof(mgmt_vlan));
    }
#endif

    if(isDhcpSettingUpdate){

        CGI_LOG_DEBUG(SYSTEM, "[%s] dhcpSetting = %d\n", __FUNCTION__, dhcpSetting);
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &dhcpSetting, sizeof(dhcpSetting));
        if(FALSE == dhcpSetting)
        {
            rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_AUTODNS_ENABLE, DB_ALL_ENTRIES, &autodnsSetting, sizeof(autodnsSetting));
        }
    }

    if((TRUE == isIpDnsUpdate) && ((0 != dhcpSetting) || (FALSE == isDhcpSettingUpdate)))
    {
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_STATIC_IP_DNS, DB_ALL_ENTRIES, &ip_dns, sizeof(ip_dns));
    }

    if((isIpAddressUpdate | isIpNetmaskUpdate | isIpGatewayUpdate | isIpDnsUpdate) && (0 == dhcpSetting) && (TRUE == isDhcpSettingUpdate))
    {
        DB_MSG_T *ptr_db_msg = NULL;
        u16_t db_size = 0;
        void *db_data = NULL;
        DB_SYS_INFO_T curr_sys_info;
        memset(&curr_sys_info, 0, sizeof(DB_SYS_INFO_T));

        CGI_LOG_DEBUG(SYSTEM, "[%s] ip_address = 0x%x, ip_netmask = 0x%x, ip_gateway = 0x%x\n", __FUNCTION__,
                          (UI32_T)ip_address, (UI32_T)ip_netmask, (UI32_T)ip_gateway);

        /* get sys_info */
        rc = httpd_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(SYSTEM, "get SYS_INFO ALL_FIELDS success, ptr_msg =%p\n", ptr_db_msg);
        }
        else
        {
            CGI_LOG_DEBUG(SYSTEM, "get SYS_INFO ALL_FIELDS failed \n");
            return ERR_VAL;
        }
        memcpy(&curr_sys_info, db_data, sizeof(DB_SYS_INFO_T));
        MW_FREE(ptr_db_msg);
        curr_sys_info.static_ip = ip_address;
        curr_sys_info.static_mask = ip_netmask;
        curr_sys_info.static_gw = ip_gateway;
        curr_sys_info.static_dns = ip_dns;
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &curr_sys_info, sizeof(DB_SYS_INFO_T));
    }

#ifdef AIR_SUPPORT_IPV6
    DB_MSG_T *ptr_db_msg = NULL;
    u16_t db_size = 0;
    void *ptr_db_data = NULL;
    DB_SYS_INFO_T *ptr_curr_sys_info = NULL;
    DB_SYS_OPER_INFO_T *ptr_curr_sys_oper_info = NULL;

    rc = osapi_calloc(sizeof(DB_SYS_INFO_T), "IP6", (void **)&ptr_curr_sys_info);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_curr_sys_info osapi_calloc() failed", __func__, __LINE__);
        return rc;
    }

    rc = osapi_calloc(sizeof(DB_SYS_OPER_INFO_T), "IP6", (void **)&ptr_curr_sys_oper_info);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_curr_sys_info);
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_curr_sys_oper_info osapi_calloc() failed", __func__, __LINE__);
        return rc;
    }

    /* For ipv6 address, prefix len, ipv6 gateway, ipv6 config mode */
    rc = httpd_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_INFO ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__,
                        ptr_db_msg);
    }
    else
    {
        MW_FREE(ptr_curr_sys_info);
        MW_FREE(ptr_curr_sys_oper_info);
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_INFO ALL_FIELDS failed\n", __func__, __LINE__);
        return ERR_VAL;
    }
    osapi_memcpy(ptr_curr_sys_info, ptr_db_data, sizeof(DB_SYS_INFO_T));
    MW_FREE(ptr_db_msg);

    /* For ipv6 dad result */
    rc = httpd_queue_getData(SYS_OPER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_OPER_INFO ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__,
                        ptr_db_msg);
    }
    else
    {
        MW_FREE(ptr_curr_sys_info);
        MW_FREE(ptr_curr_sys_oper_info);
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_OPER_INFO ALL_FIELDS failed\n", __func__, __LINE__);
        return ERR_VAL;
    }
    osapi_memcpy(ptr_curr_sys_oper_info, ptr_db_data, sizeof(DB_SYS_OPER_INFO_T));
    MW_FREE(ptr_db_msg);

    /* Update/Delete manual ipv6 address/prefix length/default gateway */
    if (TRUE == isManualIp6ModeUpdate)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: static ip6 address = %s\n", __func__, __LINE__,
                        ip6addr_ntoa(&manual_ip6_address));

        /* Update ipv6 default gateway */
        if (TRUE == isManualIp6DefaultGwUpdate)
        {
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update ipv6 default gateway\n", __func__, __LINE__);
            ip6_addr_copy(ptr_curr_sys_info->manual_ip6_default_gw, manual_ip6_default_gw);
            ptr_curr_sys_info->config_mode = IPV6_CONFIG_MODE_MANUAL;
        }
        /* Delete ipv6 default gateway */
        else
        {
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Delete ipv6 default gateway\n", __func__, __LINE__);
            ip6_addr_set_zero(&ptr_curr_sys_info->manual_ip6_default_gw);
            ptr_curr_sys_info->config_mode = IPV6_CONFIG_MODE_MANUAL;
        }

        /* Update ipv6 manual address/prefix length */
        if (TRUE == isManualIp6AddrUpdate)
        {
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Update ipv6 static address/prefix length\n", __func__, __LINE__);
            if (!ip6_addr_cmp_zoneless(&ptr_curr_sys_info->manual_ip6_addr, &manual_ip6_address))
            {
                ip6_addr_copy(ptr_curr_sys_info->manual_ip6_addr, manual_ip6_address);
                /* For the case user update only prefix_len, the dad_result can not be inited.
                 * dadCheck == IPV6_DAD_CHECK_ENABLE indicates that log in ip != modified ip, web page will be blocked.
                 * dadCehck == IPV6_DAD_CHECK_DISABLE indicates that log in ip == modified ip, web page will not be blocked.
                 */
                ptr_curr_sys_oper_info->dad_result = IP6_ADDR_TENTATIVE;
            }
            if (ptr_curr_sys_info->manual_ip6_prefix_len != manual_ip6_prefix_len)
            {
                ptr_curr_sys_info->manual_ip6_prefix_len = manual_ip6_prefix_len;
            }
            ptr_curr_sys_info->config_mode = IPV6_CONFIG_MODE_MANUAL;

            ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_1);
            ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_2);
        }
        /* Delete ipv6 manual address/prefix length/default gateway */
        else
        {
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Delete ipv6 static address/prefix length\n", __func__, __LINE__);
            /* Delete ipv6 static address will switch to dynamic mode, therefore delete all static info.
             * Though we update or delete default gateway or prefix len before.
             */
            ip6_addr_set_zero(&ptr_curr_sys_info->manual_ip6_addr);
            ptr_curr_sys_info->manual_ip6_prefix_len = 0;
            ptr_curr_sys_info->config_mode = IPV6_CONFIG_MODE_MANUAL;

            ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_1);
            ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_2);
        }

        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_curr_sys_info,
                sizeof(DB_SYS_INFO_T));
        rc = httpd_queue_setData(M_UPDATE, SYS_OPER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_curr_sys_oper_info,
                sizeof(DB_SYS_OPER_INFO_T));
    }
    /* Update auto IP6 settings */
    else if (TRUE == isAutoIp6ModeUpdate)
    {
        /* Clear manual ipv6 info */
        ip6_addr_set_zero(&ptr_curr_sys_info->manual_ip6_addr);
        ptr_curr_sys_info->manual_ip6_prefix_len = MW_IPV6_SLAAC_PREFIX_LEN;
        ip6_addr_set_zero(&ptr_curr_sys_info->manual_ip6_default_gw);

        ptr_curr_sys_info->config_mode = IPV6_CONFIG_MODE_AUTO;
        ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_1);
        ip6_addr_set_zero(&ptr_curr_sys_oper_info->ip6_addr_2);
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_curr_sys_info,
                sizeof(DB_SYS_INFO_T));
        rc = httpd_queue_setData(M_UPDATE, SYS_OPER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_curr_sys_oper_info,
                sizeof(DB_SYS_OPER_INFO_T));
    }
    MW_FREE(ptr_curr_sys_info);
    MW_FREE(ptr_curr_sys_oper_info);
#endif
    return rc;
}

MW_ERROR_NO_T
ssi_get_ipsetting_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    ip_addr_t s;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    u16_t db_size = 0;
    void *ptr_db_data = NULL;
    C8_T *ptr_tmpstr = NULL, *ptr_tag_insert = NULL;
    UI32_T tmplen = 0, tag_insert_max_len = 0;
    DB_SYS_INFO_T curr_sys_info;
#ifdef AIR_SUPPORT_IPV6
    DB_SYS_OPER_INFO_T curr_sys_oper_info;
    ip6_addr_t ip6_address;
    ip6_addr_t ip6_gw;
    UI8_T i = 0;
#endif

    CGI_LOG_DEBUG(SYSTEM, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

#ifdef AIR_SUPPORT_IPV6
    ip6_addr_set_zero(&ip6_address);
    ip6_addr_set_zero(&ip6_gw);
#endif
    if ((NULL == ptr_tag_param) || (NULL == ptr_tag_param->ptr_tag_insert) || (0 == ptr_tag_param->tag_insert_max_len))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_tag_insert = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;

    /* Get all SYS_INFO */
    rc = httpd_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_INFO DB_ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__, ptr_db_msg);
    } else
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: get SYS_INFO DB_ALL_FIELDS failed\n", __func__, __LINE__);
        return ERR_VAL;
    }
    osapi_memcpy(&curr_sys_info, ptr_db_data, sizeof(DB_SYS_INFO_T));
    MW_FREE(ptr_db_msg);

    /* get sys_info dhcp_enable  */
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "<script>var ip_ds={state:%d,vlan:1,maxVlan:4094,", curr_sys_info.dhcp_enable);

    /* get sys_info autodns_enable  */
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "autodnsstate:%d,", curr_sys_info.autodns_enable);

    /* get sys_info ip_addr  */
    ip_addr_set_ip4_u32_val(s, curr_sys_info.static_ip);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ipStr:['%s'],", ipaddr_ntoa(&s));

    /* get sys_info ip_mask  */
    ip_addr_set_ip4_u32_val(s, curr_sys_info.static_mask);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "netmaskStr:['%s'],", ipaddr_ntoa(&s));

    /* get sys_info ip_gw  */
    ip_addr_set_ip4_u32_val(s, curr_sys_info.static_gw);
    if (FALSE == ip_addr_isany_val(s))
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "gatewayStr:['%s'],", ipaddr_ntoa(&s));
    }
    else
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "gatewayStr:[''],");
    }

    /* get sys_info ip_dns  */
    ip_addr_set_ip4_u32_val(s, curr_sys_info.static_dns);
    if (FALSE == ip_addr_isany_val(s))
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "dnsStr:['%s'],", ipaddr_ntoa(&s));
    }
    else
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "dnsStr:[''],");
    }

#ifdef AIR_SUPPORT_IPV6
    /* Get ipv6 config mode */
    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: SYS_IP6_CONFIG_MODE: %d\n", __func__, __LINE__, curr_sys_info.config_mode);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6ConfigMode:%d,", curr_sys_info.config_mode);

    /* Get ipv6 prefix length */
    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: SYS_IP6_PREFIX_LEN: %d\n", __func__, __LINE__, curr_sys_info.manual_ip6_prefix_len);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6PrefixLen:%d,", curr_sys_info.manual_ip6_prefix_len);

    /* Get ipv6 static address */
    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: SYS_IP6_STATIC_ADDRESS: %p\n", __func__, __LINE__,
                    &curr_sys_info.manual_ip6_addr);
    /* For the case of no static ip6 address, return '' */
    if (ip6_addr_isany(&curr_sys_info.manual_ip6_addr))
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6ManualAddr:[''],");
    }
    else
    {
        ip6_addr_copy(ip6_address, curr_sys_info.manual_ip6_addr);
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6ManualAddr:['%s'],", ip6addr_ntoa(&ip6_address));
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6ManualAddr: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
    }

    /* Get ipv6 default gateway */
    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: SYS_IP6_DEFAULT_GW: %p\n", __func__, __LINE__, &curr_sys_info.manual_ip6_default_gw);
    if (ip6_addr_isany(&curr_sys_info.manual_ip6_default_gw))
    {
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6GwStr:[''],");
    }
    else
    {
        ip6_addr_copy(ip6_gw, curr_sys_info.manual_ip6_default_gw);
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6GwStr:['%s'],", ip6addr_ntoa(&ip6_gw));
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6GwStr: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_gw));
    }

    rc = httpd_queue_getData(SYS_OPER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_OPER_INFO DB_ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__,
            ptr_db_msg);
    } else
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: get SYS_OPER_INFO DB_ALL_FIELDS failed\n", __func__, __LINE__);
        return ERR_VAL;
    }

    /* Current config mode is dynamic mode */
    if (curr_sys_info.config_mode == IPV6_CONFIG_MODE_AUTO)
    {
        osapi_memcpy(&curr_sys_oper_info, ptr_db_data, sizeof(DB_SYS_OPER_INFO_T));
        MW_FREE(ptr_db_msg);

        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: SYS_OPER_IP6_ADDR_1: %p\n", __func__, __LINE__,
                    &curr_sys_oper_info.ip6_addr_1);

        if (ip6_addr_isany(&curr_sys_oper_info.ip6_addr_1))
        {
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_1:[''],");
        }
        else
        {
            osapi_memcpy(&ip6_address, &curr_sys_oper_info.ip6_addr_1, sizeof(ip6_address));
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_1:['%s/64'],", ip6addr_ntoa(&ip6_address));
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6AutoAddr_1: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }

        if (ip6_addr_isany(&curr_sys_oper_info.ip6_addr_2))
        {
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_2:[''],");
        }
        else
        {
            osapi_memcpy(&ip6_address, &curr_sys_oper_info.ip6_addr_2, sizeof(ip6_address));
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_2:['%s/64'],", ip6addr_ntoa(&ip6_address));
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6AutoAddr_2: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }

        if (ip6_addr_isany(&curr_sys_oper_info.ip6_link_local_addr))
        {
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6LinkLocalStr:[''],");
        }
        else
        {
            osapi_memcpy(&ip6_address, &curr_sys_oper_info.ip6_link_local_addr, sizeof(ip6_address));
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6LinkLocalStr:['%s/64'],", ip6addr_ntoa(&ip6_address));
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6LinkLocalStr: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }
    }
    else
    {
        for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; ++i)
        {
            if (i == 1)
            {
                ip6_addr_copy(ip6_address, curr_sys_info.manual_ip6_addr);
                if (0 == curr_sys_info.manual_ip6_prefix_len)
                {
                    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Noip6AutoAddr\n", __func__, __LINE__);
                    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_%d:[''],", i);
                }
                else
                {
                    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_%d:['%s/%d'],", i, ip6addr_ntoa(&ip6_address), curr_sys_info.manual_ip6_prefix_len);
                    CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6AutoAddr_1: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
                }
            }
            else
            {
                CGI_LOG_DEBUG(SYSTEM, "%s[%d]: Noip6AutoAddr\n", __func__, __LINE__);
                tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6AutoAddr_%d:[''],", i);
            }
        }

        osapi_memcpy(&curr_sys_oper_info, ptr_db_data, sizeof(DB_SYS_OPER_INFO_T));
        MW_FREE(ptr_db_msg);

        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6DadResult:['%d'],", curr_sys_oper_info.dad_result);

        if (ip6_addr_isany(&curr_sys_oper_info.ip6_link_local_addr))
        {
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6LinkLocalStr:[''],");
        }
        else
        {
            osapi_memcpy(&ip6_address, &curr_sys_oper_info.ip6_link_local_addr, sizeof(ip6_address));
            tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "ip6LinkLocalStr:['%s/64'],", ip6addr_ntoa(&ip6_address));
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: ip6LinkLocalStr: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }
    }
#endif

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    /* get mgmt vlan  */
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "mgmt_vlan:['%d'],", curr_sys_info.mgmt_vlan);
#else
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "mgmt_vlan:['%d'],", 0);
#endif

    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "};</script>");

    MW_FREE(ptr_tmpstr);
#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = tmplen;
    ptr_tag_param->chunk_enable = TRUE;

    CGI_LOG_DEBUG(SYSTEM, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    return ERR_OK;
}
