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

/* FILE NAME:   systemInfoRpm.c
 * PURPOSE:
 *      CGI and SSI function of system information web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/priv/nd6_priv.h"
#include "lwip/nd6.h"
#include "lwip/ip6_addr.h"
#include "inet_utils.h"
#endif

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

/* LOCAL SUBPROMGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T cgi_set_handle_systemInfoRpm(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

    UI8_T i;
    C8_T *ptr_sys_name = NULL;
    UI8_T space[4] = "%20";
    C8_T *ptr_name;
    UI8_T rc = MW_E_OK;

    if (MW_E_NO_MEMORY == osapi_calloc((MAX_SYS_NAME_SIZE*3), HTTPD_QUEUE_CLI, (void **)&ptr_sys_name))
    {
        return MW_E_NO_MEMORY;
    }

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if (!strcmp(pcParam[i], "sysName"))
        {
            osapi_strncpy(ptr_sys_name, pcValue[i], MAX_SYS_NAME_SIZE);
            ptr_sys_name[MAX_SYS_NAME_SIZE] = '\0';
            /* check space */
            for (ptr_name = strstr((const char *)ptr_sys_name, (const char *)space); ptr_name != NULL; ptr_name = strstr((const char *)ptr_sys_name, (const char *)space))
            {
                ptr_name[0] = ' ';
                ptr_name[1] = '\0';
                ptr_name += 3;
                if ((osapi_strlen(ptr_sys_name) + osapi_strlen(ptr_name)) <= ((MAX_SYS_NAME_SIZE*3) - 1))
                {
                    osapi_strncat((char *)ptr_sys_name, ptr_name, MAX_SYS_NAME_SIZE - osapi_strlen(ptr_sys_name));
                }
                else
                {
                    break;
                }
            }
        }
    }
    /* parser params to db format */
    if (0 != ptr_sys_name[0])
    {
        CGI_LOG_DEBUG(SYSTEM, "ptr_sys_name = %s", ptr_sys_name);
        rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_NAME, DB_ALL_ENTRIES, ptr_sys_name, MAX_SYS_NAME_SIZE);
    }
    MW_FREE(ptr_sys_name);
    return rc;
}

char ssi_get_system_name_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    UI16_T total_len = 0;
    C8_T err = 0;
    DB_SYS_INFO_T *ptr_curr_sys_info = NULL;
    MW_IPV4_T ip4_address;
    DB_SYS_OPER_INFO_T *ptr_curr_sys_oper_info = NULL;
#ifdef AIR_SUPPORT_IPV6
    ip6_addr_t ip6_address;
    BOOL_T glb_ipv6_1_exist = FALSE;
    BOOL_T glb_ipv6_2_exist = FALSE;
#endif

    system_info_rpm_t *ptr_system_info_rpm = NULL;

    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;
    UI16_T tmplen = 0;
    UI16_T strlen = HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN;
    C8_T *ptr_ssi_str = NULL;
    rc = osapi_calloc(strlen, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str);
    if (MW_E_NO_MEMORY == rc)
    {
        return ERR_MEM;
    }

    rc = osapi_calloc(sizeof(DB_SYS_INFO_T), "IP6", (void **)&ptr_curr_sys_info);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_curr_sys_info osapi_calloc() failed", __func__, __LINE__);
        MW_FREE(ptr_ssi_str);
        return rc;
    }

    rc = osapi_calloc(sizeof(DB_SYS_OPER_INFO_T), "IP6", (void **)&ptr_curr_sys_oper_info);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: ptr_curr_sys_oper_info osapi_calloc() failed", __func__, __LINE__);
        MW_FREE(ptr_ssi_str);
        MW_FREE(ptr_curr_sys_info);
        return rc;
    }

    /* get SYS_INFO*/
    rc = httpd_queue_getData(SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if (MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_INFO DB_ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__, ptr_db_msg);
    }
    else
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: get SYS_INFO DB_ALL_FIELDS failed\n", __func__, __LINE__);
        MW_FREE(ptr_ssi_str);
        MW_FREE(ptr_curr_sys_info);
        MW_FREE(ptr_curr_sys_oper_info);
        return ERR_VAL;
    }
    osapi_memcpy(ptr_curr_sys_info, ptr_db_data, sizeof(DB_SYS_INFO_T));
    MW_FREE(ptr_db_msg);

    if (MW_E_NO_MEMORY == osapi_calloc(sizeof(system_info_rpm_t), HTTPD_QUEUE_CLI, (void **)&ptr_system_info_rpm))
    {
        MW_FREE(ptr_ssi_str);
        MW_FREE(ptr_curr_sys_info);
        MW_FREE(ptr_curr_sys_oper_info);
        return MW_E_NO_MEMORY;
    }

    /* get SYS_OPER_INFO */
    rc = httpd_queue_getData(SYS_OPER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s[%d]: get SYS_OPER_INFO DB_ALL_FIELDS success, ptr_msg =%p\n", __func__, __LINE__, ptr_db_msg);
    }
    else
    {
        CGI_LOG_ERROR(SYSTEM, "%s[%d]: get SYS_OPER_INFO DB_ALL_FIELDS failed\n", __func__, __LINE__);
        MW_FREE(ptr_ssi_str);
        MW_FREE(ptr_curr_sys_info);
        MW_FREE(ptr_curr_sys_oper_info);
        MW_FREE(ptr_system_info_rpm);
        return ERR_VAL;
    }
    osapi_memcpy(ptr_curr_sys_oper_info, ptr_db_data, sizeof(DB_SYS_OPER_INFO_T));
    MW_FREE(ptr_db_msg);

    /* Send sys_name */
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "var data_info = {sysnameStr:[\"%s\"],", ptr_curr_sys_info->sys_name);

    /* Send sys_mac */
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "macStr:[\"%02x:%02x:%02x:%02x:%02x:%02x\"],",
                               ptr_curr_sys_oper_info->sys_mac[0], ptr_curr_sys_oper_info->sys_mac[1], ptr_curr_sys_oper_info->sys_mac[2], ptr_curr_sys_oper_info->sys_mac[3], ptr_curr_sys_oper_info->sys_mac[4], ptr_curr_sys_oper_info->sys_mac[5]);

    /* Send ip_addr */
    osapi_memcpy(&ip4_address, &ptr_curr_sys_oper_info->ip_addr, sizeof(ip4_address));
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "ipStr:[\"%s\"],", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)&ip4_address));

#ifdef AIR_SUPPORT_IPV6
    /* send ip6_link_local_addr */
    osapi_memcpy(&ip6_address, &ptr_curr_sys_oper_info->ip6_link_local_addr, sizeof(ip6_address));
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "linkLocalIpv6Str:[\"%s/64<br>\"],", ip6addr_ntoa(&ip6_address));

    if (IPV6_CONFIG_MODE_AUTO == ptr_curr_sys_info->config_mode)
    {
        /* Send ip6_addr_1 */
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "globalIpv6Str:[\"");
        if (!ip6_addr_isany(&ptr_curr_sys_oper_info->ip6_addr_1))
        {
            osapi_memcpy(&ip6_address, &ptr_curr_sys_oper_info->ip6_addr_1, sizeof(ip6_address));
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "%s/64 ", ip6addr_ntoa(&ip6_address));
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "(%s)<br>", IP6_STATE_AUTO);
            glb_ipv6_1_exist = TRUE;
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: globalIpv6Str: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }
        else
        {
            glb_ipv6_1_exist = FALSE;
        }

        /* Send ip6_addr_2 */
        if (!ip6_addr_isany(&ptr_curr_sys_oper_info->ip6_addr_2))
        {
            osapi_memcpy(&ip6_address, &ptr_curr_sys_oper_info->ip6_addr_2, sizeof(ip6_address));
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "%s/64 ", ip6addr_ntoa(&ip6_address));
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "(%s)<br>", IP6_STATE_AUTO);
            glb_ipv6_2_exist = TRUE;
            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: globalIpv6Str: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }
        else
        {
            glb_ipv6_2_exist = FALSE;
        }

        if ((FALSE == glb_ipv6_1_exist) && (FALSE == glb_ipv6_2_exist))
        {
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "None<br>");
        }
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "\"],");
    }
    else
    {
        /* Send manual_ip6_addr */
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "globalIpv6Str:[\"");
        if (!ip6_addr_isany(&ptr_curr_sys_info->manual_ip6_addr))
        {
            osapi_memcpy(&ip6_address, &ptr_curr_sys_info->manual_ip6_addr, sizeof(ip6_address));
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "%s/%d ", ip6addr_ntoa(&ip6_address), ptr_curr_sys_info->manual_ip6_prefix_len);

            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: dad_result: %d\n", __func__, __LINE__, ptr_curr_sys_oper_info->dad_result);
            if (IP6_ADDR_PREFERRED == ptr_curr_sys_oper_info->dad_result || IP6_ADDR_VALID == ptr_curr_sys_oper_info->dad_result)
            {
                tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "(%s)<br>", IP6_STATE_MANUAL);
            }
            else
            {
                tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "(%s)<br>", IP6_STATE_INVAILD);
            }

            CGI_LOG_DEBUG(SYSTEM, "%s[%d]: globalIpv6Str: %s\n", __func__, __LINE__, ip6addr_ntoa(&ip6_address));
        }
        else
        {
            tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "None<br>");
        }
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "\"],");
    }

    /* Send manual_ip6_default_gw */
    osapi_memcpy(&ip6_address, &ptr_curr_sys_info->manual_ip6_default_gw, sizeof(ip6_address));
#endif
    /* Send ip_gw */
    osapi_memcpy(&ip4_address, &ptr_curr_sys_oper_info->ip_mask, sizeof(ip4_address));
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "netmaskStr:[\"%s\"],gatewayStr:[\"", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)&ip4_address));
#ifdef AIR_SUPPORT_IPV6
    if (!ip6_addr_isany(&ptr_curr_sys_info->manual_ip6_default_gw))
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen,  "%s<br>", ip6addr_ntoa(&ip6_address));
    }
#endif
    if (0 != ptr_curr_sys_oper_info->ip_gw)
    {
        osapi_memcpy(&ip4_address, &ptr_curr_sys_oper_info->ip_gw, sizeof(ip4_address));
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "%s\"],", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)&ip4_address));
    }
    else
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "----\"],");
    }

    /* Send ip_dns */
    if (0 != ptr_curr_sys_oper_info->ip_dns)
    {
        osapi_memcpy(&ip4_address, &ptr_curr_sys_oper_info->ip_dns, sizeof(ip4_address));
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "dnsStr:[\"%s\"],", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)&ip4_address));
    }
    else
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "dnsStr:[\"----\"],");
    }

    /* get sw_version*/
    mw_get_version(ptr_system_info_rpm->sw_version);
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "firmwareStr:[\"%s\"], ", ptr_system_info_rpm->sw_version);

    /* get hw_version*/
    tmplen += snprintf(ptr_ssi_str + tmplen, strlen - tmplen, "hardwareStr:[\"%s\"]};\n", ptr_curr_sys_oper_info->hw_version);

    err = send_format_response(&total_len, pcb, apiflags, "<script>%s</script>", ptr_ssi_str);
    MW_FREE(ptr_ssi_str);
    MW_FREE(ptr_curr_sys_info);
    MW_FREE(ptr_curr_sys_oper_info);
    MW_FREE(ptr_system_info_rpm);
    if (ERR_OK != err)
    {
        return err;
    }

    *length = (int)total_len;

    return ERR_OK;
}
