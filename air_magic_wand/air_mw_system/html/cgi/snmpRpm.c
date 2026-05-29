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

/* FILE NAME:   snmpRpm.c
 * PURPOSE:
 *      CGI and SSI function of SNMP web page.
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
#include "inet_utils.h"
#include "lwip/apps/snmp.h"

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

#ifdef AIR_SUPPORT_SNMP
MW_ERROR_NO_T
cgi_set_handle_snmpConfig(
    int iIndex,
    int iNumParams,
    char *ptr_Param[],
    char *ptr_Value[])
{
    UI32_T i;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T snmp_ver = SNMP_NO_SUPPORT, trap_ver = SNMP_NO_TRAP_SUPPORT, trap_type = SNMP_NO_TRAP_TYPE_SUPPORT;
    C8_T *ptr_string = NULL;
    UI16_T string_len = MAX(MAX_SNMP_CM_LEN, MAX_HOST_NAME_SIZE);
    UI32_T trapIp = 0;
    UI32_T temp = 0;

    rc = osapi_calloc(string_len, HTTPD_QUEUE_CLI, (void **)&ptr_string);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(SNMP, "Failed to allocate memory for SNMP config. Error code: %d", rc);
        return rc;
    }
    /* expected CGI format:
     * eeeSet.cgi?state=1=&portBit=5 */

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_Param[i], "snmpv1")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                snmp_ver |= SNMP_V1_SUPPORT;
            }
        }
        if ((0 == osapi_strcmp(ptr_Param[i], "snmpv2")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                snmp_ver |= SNMP_V2_SUPPORT;
            }
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_VERSION, DB_ALL_ENTRIES, &snmp_ver, sizeof(snmp_ver));
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set snmp version to 0x%x", snmp_ver);
        }

        if ((0 == osapi_strcmp(ptr_Param[i], "trapV1En")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                trap_ver |= SNMP_TRAP_SUPPORT_V1;
            }
        }
        if ((0 == osapi_strcmp(ptr_Param[i], "trapV2En")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                trap_ver |= SNMP_TRAP_SUPPORT_V2;
            }
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_EN, DB_ALL_ENTRIES, &trap_ver, sizeof(trap_ver));
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set snmp trap version to 0x%x", trap_ver);
        }

        if (0 == osapi_strcmp(ptr_Param[i], "trapName"))
        {
            osapi_memset(ptr_string, 0, string_len);
            osapi_strncpy(ptr_string, ptr_Value[i], MAX_HOST_NAME_SIZE);
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_HOSTNAME, DB_ALL_ENTRIES, ptr_string, MAX_HOST_NAME_SIZE);
            rc |= httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_DST_IP, DB_ALL_ENTRIES, &trapIp, sizeof(trapIp));
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set snmp trap name to %s", ptr_string);
        }

        if (0 == osapi_strcmp(ptr_Param[i], "trapIp"))
        {
            osapi_memset(ptr_string, 0, string_len);
            trapIp = getIpaddr(ptr_Value[i]);
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_HOSTNAME, DB_ALL_ENTRIES, ptr_string, MAX_HOST_NAME_SIZE);
            rc |= httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_DST_IP, DB_ALL_ENTRIES, &trapIp, sizeof(trapIp));
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "Set SNMP trap ip to 0x%x", trapIp);
        }

        if ((0 == osapi_strcmp(ptr_Param[i], "coldwarmstartEn")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                trap_type |= SNMP_COLD_WARM_START_SUPPORT;
            }
        }
        if ((0 == osapi_strcmp(ptr_Param[i], "linkupdownEn")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                trap_type |= SNMP_LINKUP_DOWN_SUPPORT;
            }
        }
        if ((0 == osapi_strcmp(ptr_Param[i], "AuthfailEn")) && (MW_E_OK == osapi_strtou32(ptr_Value[i], &temp)))
        {
            if (SNMP_ENABLE == temp)
            {
                trap_type |= SNMP_AUTHFAIL_SUPPORT;
            }
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_TYPE, DB_ALL_ENTRIES, &trap_type, sizeof(trap_type));
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "Set SNMP trap type to 0x%x", trap_type);
        }

        if (0 == osapi_strcmp(ptr_Param[i], "readCm"))
        {
            osapi_memset(ptr_string, 0, string_len);
            osapi_strncpy(ptr_string, ptr_Value[i], MAX_SNMP_CM_LEN);
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_READ_COMMUNITY, DB_ALL_ENTRIES, ptr_string, MAX_SNMP_CM_LEN);
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set readCm to %s", ptr_string);
        }
        if (0 == osapi_strcmp(ptr_Param[i], "writeCm"))
        {
            osapi_memset(ptr_string, 0, string_len);
            osapi_strncpy(ptr_string, ptr_Value[i], MAX_SNMP_CM_LEN);
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_WRITE_COMMUNITY, DB_ALL_ENTRIES, ptr_string, MAX_SNMP_CM_LEN);
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set writeCm to %s", ptr_string);
        }
        if (0 == osapi_strcmp(ptr_Param[i], "trapCm"))
        {
            osapi_memset(ptr_string, 0, string_len);
            osapi_strncpy(ptr_string, ptr_Value[i], MAX_SNMP_CM_LEN);
            rc = httpd_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_COMMUNITY, DB_ALL_ENTRIES, ptr_string, MAX_SNMP_CM_LEN);
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_string);
                CGI_LOG_ERROR(SNMP, "[%s][%d]httpd_queue_setData failed, rc = %d", __func__, __LINE__, rc);
                return rc;
            }
            CGI_LOG_INFO(SNMP, "set trapCm to %s", ptr_string);
        }
    }

    MW_FREE(ptr_string);
    return rc;
}

MW_ERROR_NO_T
ssi_get_snmp_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    UI32_T total_len = 0;
    ip_addr_t s;
    ip_addr_t *addr = NULL;
    char err = 0;
    DB_MSG_T *db_msg = NULL;
    u16_t db_size = 0;
    void *db_data = NULL;
    UI8_T snmp_ver = SNMP_NO_SUPPORT, trap_ver = SNMP_NO_TRAP_SUPPORT, trap_type = SNMP_NO_TRAP_TYPE_SUPPORT;
    UI8_T snmpv1, snmpv2, trapv1, trapv2, coldwarm, linkupdown, authfail;
    C8_T *ptr_string = NULL;
    UI16_T string_len = MAX(MAX_SNMP_CM_LEN, MAX_HOST_NAME_SIZE);
    u32_t trapIp = 0;

    if (MW_E_OK != osapi_calloc(string_len, HTTPD_QUEUE_CLI, (void **)&ptr_string))
    {
        CGI_LOG_ERROR(SNMP, "Failed to allocate memory for SNMP string.");
        return MW_E_NO_MEMORY;
    }

    if (httpd_queue_getData(SNMP_INFO, SNMP_VERSION, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_VERSION) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memcpy(&snmp_ver, db_data, db_size);
    snmpv1 = (snmp_ver & SNMP_V1_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE;
    snmpv2 = (snmp_ver & SNMP_V2_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE;
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>var snmp_info={snmpv1:%d,snmpv2:%d,", snmpv1, snmpv2);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;


    if (httpd_queue_getData(SNMP_INFO, SNMP_TRAP_EN, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_TRAP_EN) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memcpy(&trap_ver, db_data, db_size);
    trapv1 = (trap_ver & SNMP_TRAP_SUPPORT_V1) ? SNMP_ENABLE : SNMP_DISABLE;
    trapv2 = (trap_ver & SNMP_TRAP_SUPPORT_V2) ? SNMP_ENABLE : SNMP_DISABLE;
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "trapv1:%d,trapv2:%d,", trapv1, trapv2);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_TRAP_TYPE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_TRAP_TYPE) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memcpy(&trap_type, db_data, db_size);
    coldwarm = (trap_type & SNMP_COLD_WARM_START_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE;
    linkupdown = (trap_type & SNMP_LINKUP_DOWN_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE;
    authfail = (trap_type & SNMP_AUTHFAIL_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE;
    MW_FREE(db_msg);
    err = send_format_response(&len, ptr_pcb, apiflags, "coldwarm:%d,linkupdown:%d,authfail:%d,", coldwarm, linkupdown, authfail);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_TRAP_DST_IP, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_TRAP_DST_IP) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memcpy(&trapIp, db_data, db_size);
    MW_FREE(db_msg);
    addr = &s;
    ip_addr_set_ip4_u32(addr, trapIp);
    err = send_format_response(&len, ptr_pcb, apiflags, "trapIp:['%s'],", ipaddr_ntoa(&s));
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_READ_COMMUNITY, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_READ_COMMUNITY) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memset(ptr_string, 0, string_len);
    osapi_memcpy(ptr_string, db_data, db_size);
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "readCm:[\"%s\"],", ptr_string);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_WRITE_COMMUNITY, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_WRITE_COMMUNITY) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memset(ptr_string, 0, string_len);
    osapi_memcpy(ptr_string, db_data, db_size);
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "writeCm:[\"%s\"],", ptr_string);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_TRAP_COMMUNITY, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_TRAP_COMMUNITY) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memset(ptr_string, 0, string_len);
    osapi_memcpy(ptr_string, db_data, db_size);
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "trapCm:[\"%s\"],", ptr_string);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    if (httpd_queue_getData(SNMP_INFO, SNMP_TRAP_HOSTNAME, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(SNMP, "httpd_queue_getData(SNMP_INFO:SNMP_TRAP_HOSTNAME) failed");
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    osapi_memset(ptr_string, 0, string_len);
    osapi_memcpy(ptr_string, db_data, db_size);
    MW_FREE(db_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, "trapName:[\"%s\"],", ptr_string);
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    total_len += len;

    err = send_format_response(&len, ptr_pcb, apiflags, "};</script>");
    total_len += len;
    if (err != ERR_OK)
    {
        MW_FREE(ptr_string);
        return MW_E_OP_STOPPED;
    }

    *length = total_len;

    MW_FREE(ptr_string);

    return MW_E_OK;
}

#endif
