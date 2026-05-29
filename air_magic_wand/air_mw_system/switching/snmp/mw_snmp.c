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

/* FILE NAME:  mw_snmp.c
 * PURPOSE:
 *  Implement internal queue function of snmp daemon.
 *
 * NOTES:
 *
 */

#include <string.h>
#include "mw_snmp.h"

#include "mw_error.h"
#include "osapi.h"
#include "osapi_memory.h"
#include "osapi_message.h"
#include "osapi_string.h"
#include "db_data.h"
#include "lwip/api.h"
#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "air_chipscu.h"
#include "mw_log.h"

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
static UI16_T snmp_linkup = 0;
static UI16_T snmp_send_coldwarm_start = 1;
static BOOL_T snmp_subscribe_done = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
_snmp_queue_send(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    MW_ERROR_NO_T rc;

    MW_CHECK_PTR(ptr_msg);

    rc = dbapi_dbisReady();
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_FREE(ptr_msg);
        return rc;
    }

    rc = dbapi_sendRequesttoDb(size, ptr_msg);
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_LOG_ERROR(SNMP, "%s: dbapi_sendRequesttoDb() failed", __func__);
        MW_FREE(ptr_msg);
    }
    return rc;
}

static MW_ERROR_NO_T
_snmp_db_msg_process(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void *ptr_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    err_t err;

    if ((NULL == ptr_request) || (NULL == ptr_data) || (0 == data_size))
    {
        MW_LOG_ERROR(SNMP, "%s[%d] bad parameter! ptr_request=%p, ptr_data=%p, data_size=%d", __func__, __LINE__, ptr_request, ptr_data, data_size);
        return MW_E_BAD_PARAMETER;
    }

    switch (method)
    {
        case M_GET:
        case M_UPDATE:
            switch (ptr_request->t_idx)
            {
                case PORT_OPER_INFO:
                    switch (ptr_request->f_idx)
                    {
                        case PORT_OPER_STATUS:
                        {
                            UI8_T trap_type = ((*((UI8_T *)ptr_data) == 0) ? SNMP_GENTRAP_LINKDOWN : SNMP_GENTRAP_LINKUP);
                            osapi_memcpy((UI8_T *)&trap_type, (UI8_T *)ptr_data, sizeof(UI8_T));
                            trap_type = ((trap_type == 0) ? SNMP_GENTRAP_LINKDOWN : SNMP_GENTRAP_LINKUP);
                            struct snmpcallback_msg_trap trap_info;
                            int br = 0;

                            MW_LOG_INFO(SNMP, "%s[%d] port[%d] PORT_OPER_STATUS: %s, linkup flag:%d, coldwarm start flag:%d",
                            __func__, __LINE__, ptr_request->e_idx, (trap_type == SNMP_GENTRAP_LINKDOWN) ? "LINKDOWN" : "LINKUP", snmp_linkup, snmp_send_coldwarm_start);
                            if ((SNMP_GENTRAP_LINKUP == trap_type) && (0 == snmp_linkup))
                            {
                                snmp_linkup = 1;
                            }
                            if ((SNMP_GENTRAP_LINKUP == trap_type) && (1 == snmp_send_coldwarm_start))
                            {
                                snmp_send_coldwarm_start = 0;
                                air_chipscu_getBootReason(0, &br);
                                trap_info.trap_type = ((br == 1) ? SNMP_GENTRAP_WARMSTART : SNMP_GENTRAP_COLDSTART);
                                trap_info.ifIndex = ptr_request->e_idx;
                                err = snmp_trap_callback(&trap_info);
                                MW_LOG_INFO(SNMP, "%s[%d] send trap: %s, ret:%d", __func__, __LINE__, (br == 1) ? "WARMSTART" : "COLDSTART", err);
                            }
                            trap_info.trap_type = trap_type;
                            trap_info.ifIndex = ptr_request->e_idx;
                            err = snmp_trap_callback(&trap_info);
                            MW_LOG_INFO(SNMP, "%s[%d] port[%d] send trap: %s, ret:%d", __func__, __LINE__, ptr_request->e_idx, trap_type == SNMP_GENTRAP_LINKDOWN ? "LINKDOWN" : "LINKUP", err);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case LOGON_INFO:
                    switch (ptr_request->f_idx)
                    {
                        case LOGON_FAIL_COUNT:
                        {
                            UI8_T fail_count;
                            struct snmpcallback_msg_trap trap_info;
                            osapi_memcpy((UI8_T *)&fail_count, (UI8_T *)ptr_data, sizeof(UI8_T));
                            if (0 != fail_count)
                            {
                                trap_info.trap_type = SNMP_GENTRAP_AUTH_FAILURE;
                                trap_info.ifIndex = ptr_request->e_idx;
                                err = snmp_trap_callback(&trap_info);
                                MW_LOG_INFO(SNMP, "%s[%d] LOGON_INFO(fail count:%d), send trap: %s, ret:%d", __func__, __LINE__, fail_count, "Authentication failure", err);
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case SNMP_INFO:
                    switch (ptr_request->f_idx)
                    {
                        case SNMP_VERSION:
                        {
                            UI8_T snmp_ver;
                            osapi_memcpy((UI8_T *)&snmp_ver, (UI8_T *)ptr_data, sizeof(UI8_T));
                            snmp_v1_enable((snmp_ver & SNMP_V1_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE);
                            snmp_v2c_enable((snmp_ver & SNMP_V2_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE);
                            MW_LOG_INFO(SNMP, "%s[%d] SNMP_VERSION=0x%x", __func__, __LINE__, snmp_ver);
                            break;
                        }
                        case SNMP_TRAP_EN:
                        {
                            UI8_T snmp_trap_enable;
                            osapi_memcpy((UI8_T *)&snmp_trap_enable, (UI8_T *)ptr_data, sizeof(UI8_T));
                            snmp_trap_dst_enable(0, (0 == snmp_trap_enable) ? SNMP_DISABLE : SNMP_ENABLE);
                            MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_EN=0x%x", __func__, __LINE__, snmp_trap_enable);
                            break;
                        }
                        case SNMP_TRAP_TYPE:
                        {
                            UI8_T snmp_trap_type;
                            osapi_memcpy((UI8_T *)&snmp_trap_type, (UI8_T *)ptr_data, sizeof(UI8_T));
                            snmp_set_auth_traps_enabled((0 == (snmp_trap_type & SNMP_AUTHFAIL_SUPPORT)) ? SNMP_DISABLE : SNMP_ENABLE);
                            MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_TYPE=0x%x", __func__, __LINE__, snmp_trap_type);
                            break;
                        }
                        case SNMP_TRAP_DST_IP:
                        {
                            UI32_T dip = 0;
                            ip_addr_t dst;
                            osapi_memset(&dst, 0, sizeof(ip_addr_t));
                            osapi_memcpy((UI8_T *)&dip, (UI8_T *)ptr_data, sizeof(UI32_T));
                            if (0 != dip)
                            {
                                ip_addr_set_ip4_u32_val(dst, dip);
                                snmp_trap_dst_ip_set(0, &dst);
                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_DST_IP=%s", __func__, __LINE__, ipaddr_ntoa(&dst));
                            }

                            break;
                        }
#if LWIP_DNS
                        case SNMP_TRAP_HOSTNAME:
                        {
                            C8_T *trapName = NULL;
                            ip_addr_t dst;
                            osapi_memset(&dst, 0, sizeof(ip_addr_t));

                            rc = osapi_calloc(MAX_HOST_NAME_SIZE, SNMP_MODULE_NAME, (void **)&trapName);
                            if (NULL != trapName)
                            {
                                memcpy(trapName, ptr_data, data_size > MAX_HOST_NAME_SIZE - 1 ? MAX_HOST_NAME_SIZE - 1 : data_size);
                                trapName[MAX_HOST_NAME_SIZE - 1] = '\0';
                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_HOSTNAME=%s", __func__, __LINE__, trapName);
                                if (0 != trapName[0])
                                {
                                    netconn_gethostbyname(trapName, &dst);
                                    snmp_trap_dst_ip_set(0, &dst);
                                    MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_HOSTNAME IP=%s", __func__, __LINE__, ipaddr_ntoa(&dst));
                                }
                                MW_FREE(trapName);
                            }
                            else
                            {
                                MW_LOG_ERROR(SNMP, "%s[%d] allocate trap name memory fail(%d)!", __func__, __LINE__, rc);
                            }

                            break;
                        }
#endif
                        case SNMP_READ_COMMUNITY:
                        {
                            C8_T *read_community = NULL;

                            rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&read_community);
                            if (NULL != read_community)
                            {
                                memcpy(read_community, ptr_data, data_size > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : data_size);
                                read_community[MAX_SNMP_CM_LEN - 1] = 0;
                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_READ_COMMUNITY=%s", __func__, __LINE__, read_community);
                                snmp_set_community(read_community, osapi_strlen(read_community));
                                MW_FREE(read_community);
                            }
                            break;
                        }
                        case SNMP_WRITE_COMMUNITY:
                        {
                            C8_T *write_community = NULL;

                            rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&write_community);
                            if (NULL != write_community)
                            {
                                memcpy(write_community, ptr_data, data_size > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : data_size);
                                write_community[MAX_SNMP_CM_LEN - 1] = 0;
                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_WRITE_COMMUNITY=%s", __func__, __LINE__, write_community);
                                snmp_set_community_write(write_community, osapi_strlen(write_community));
                                MW_FREE(write_community);
                            }
                            break;
                        }
                        case SNMP_TRAP_COMMUNITY:
                        {
                            C8_T *trap_community = NULL;

                            rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&trap_community);
                            if (NULL != trap_community)
                            {
                                memcpy(trap_community, ptr_data, data_size > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : data_size);
                                trap_community[MAX_SNMP_CM_LEN - 1] = 0;
                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_COMMUNITY=%s", __func__, __LINE__, trap_community);
                                snmp_set_community_trap(trap_community, osapi_strlen(trap_community));
                                MW_FREE(trap_community);
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case SYS_OPER_INFO:
                    switch (ptr_request->f_idx)
                    {
                        case SYS_OPER_IP_ADDR:
                        {
                            UI32_T sys_ip = 0;
                            osapi_memcpy(&sys_ip, ptr_data, sizeof(sys_ip));
                            if (0 != sys_ip)
                            {
                                DB_MSG_T *ptrMsg = NULL;
                                UI16_T dataSize = 0;
                                UI8_T *ptrData = NULL;
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_VERSION, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    UI8_T snmp_ver;
                                    osapi_memcpy((UI8_T *)&snmp_ver, (UI8_T *)ptrData, sizeof(UI8_T));
                                    snmp_v1_enable((snmp_ver & SNMP_V1_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE);
                                    snmp_v2c_enable((snmp_ver & SNMP_V2_SUPPORT) ? SNMP_ENABLE : SNMP_DISABLE);
                                    MW_LOG_INFO(SNMP, "%s[%d] SNMP_VERSION=0x%x", __func__, __LINE__, snmp_ver);
                                    MW_FREE(ptrMsg);
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_TRAP_EN, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    UI8_T snmp_trap_enable;
                                    osapi_memcpy((UI8_T *)&snmp_trap_enable, (UI8_T *)ptrData, sizeof(UI8_T));
                                    snmp_trap_dst_enable(0, (0 == snmp_trap_enable) ? SNMP_DISABLE : SNMP_ENABLE);
                                    MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_EN=0x%x", __func__, __LINE__, snmp_trap_enable);
                                    MW_FREE(ptrMsg);
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_TRAP_TYPE, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    UI8_T snmp_trap_type;
                                    osapi_memcpy((UI8_T *)&snmp_trap_type, (UI8_T *)ptrData, sizeof(UI8_T));
                                    snmp_set_auth_traps_enabled((0 == (snmp_trap_type & SNMP_AUTHFAIL_SUPPORT)) ? SNMP_DISABLE : SNMP_ENABLE);
                                    MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_TYPE=0x%x", __func__, __LINE__, snmp_trap_type);
                                    MW_FREE(ptrMsg);
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_TRAP_DST_IP, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    ip_addr_t dst;
                                    osapi_memset(&dst, 0, sizeof(ip_addr_t));
                                    ip_addr_set_ip4_u32_val(dst, *(u32_t *)ptrData);
                                    MW_FREE(ptrMsg);
                                    if (FALSE == ip_addr_isany_val(dst))
                                    {
                                        snmp_trap_dst_ip_set(0, &dst);
                                        MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_DST_IP=%s", __func__, __LINE__, ipaddr_ntoa(&dst));
                                    }
                                    else
                                    {
                                        if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_TRAP_HOSTNAME, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                        {
                                            C8_T *trapName = NULL;
                                            rc = osapi_calloc(MAX_HOST_NAME_SIZE, SNMP_MODULE_NAME, (void **)&trapName);
                                            if (NULL != trapName)
                                            {
                                                memcpy(trapName, ptrData, dataSize > MAX_HOST_NAME_SIZE - 1 ? MAX_HOST_NAME_SIZE - 1 : dataSize);
                                                trapName[MAX_HOST_NAME_SIZE - 1] = 0;
                                                MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_HOSTNAME=%s", __func__, __LINE__, trapName);
#if LWIP_DNS
                                        /* get ip addr by hostname */
                                                if (0 != trapName[0])
                                                {
                                                    netconn_gethostbyname(trapName, &dst);
                                                    snmp_trap_dst_ip_set(0, &dst);
                                                    MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_HOSTNAME IP=%s", __func__, __LINE__, ipaddr_ntoa(&dst));
                                                }
                                                MW_FREE(trapName);
#endif
                                            }
                                            MW_FREE(ptrMsg);
                                        }
                                    }
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_READ_COMMUNITY, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    C8_T *read_community = NULL;
                                    rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&read_community);
                                    if (NULL != read_community)
                                    {
                                        memcpy(read_community, ptrData, dataSize > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : dataSize);
                                        read_community[MAX_SNMP_CM_LEN - 1] = 0;
                                        MW_LOG_INFO(SNMP, "%s[%d] SNMP_READ_COMMUNITY=%s", __func__, __LINE__, read_community);
                                        snmp_set_community(read_community, osapi_strlen(read_community));
                                        MW_FREE(read_community);
                                    }
                                    MW_FREE(ptrMsg);
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_WRITE_COMMUNITY, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    C8_T *write_community = NULL;
                                    rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&write_community);
                                    if (NULL != write_community)
                                    {
                                        memcpy(write_community, ptrData, dataSize > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : dataSize);
                                        write_community[MAX_SNMP_CM_LEN - 1] = 0;
                                        MW_LOG_INFO(SNMP, "%s[%d] SNMP_WRITE_COMMUNITY=%s", __func__, __LINE__, write_community);
                                        snmp_set_community_write(write_community, osapi_strlen(write_community));
                                        MW_FREE(write_community);
                                    }
                                    MW_FREE(ptrMsg);
                                }
                                if (MW_E_OK == snmp_queue_getData(SNMP_INFO, SNMP_TRAP_COMMUNITY, DB_ALL_ENTRIES, &ptrMsg, &dataSize, (void **)&ptrData))
                                {
                                    C8_T *trap_community = NULL;
                                    rc = osapi_calloc(MAX_SNMP_CM_LEN, SNMP_MODULE_NAME, (void **)&trap_community);
                                    if (NULL != trap_community)
                                    {
                                        memcpy(trap_community, ptrData, dataSize > MAX_SNMP_CM_LEN - 1 ? MAX_SNMP_CM_LEN - 1 : dataSize);
                                        trap_community[MAX_SNMP_CM_LEN - 1] = 0;
                                        MW_LOG_INFO(SNMP, "%s[%d] SNMP_TRAP_COMMUNITY=%s", __func__, __LINE__, trap_community);
                                        snmp_set_community_trap(trap_community, osapi_strlen(trap_community));
                                        MW_FREE(trap_community);
                                    }
                                    MW_FREE(ptrMsg);
                                }
                                if ((1 == snmp_linkup) && (1 == snmp_send_coldwarm_start))
                                {
                                    snmp_send_coldwarm_start = 0;
                                    struct snmpcallback_msg_trap trap_info;
                                    int br = 0;
                                    air_chipscu_getBootReason(0, &br);
                                    trap_info.trap_type = ((br == 1) ? SNMP_GENTRAP_WARMSTART : SNMP_GENTRAP_COLDSTART);
                                    trap_info.ifIndex = ptr_request->e_idx;
                                    err = snmp_trap_callback(&trap_info);
                                    MW_LOG_INFO(SNMP, "%s[%d] send trap: %s, ret:%d", __func__, __LINE__, (br == 1) ? "WARMSTART" : "COLDSTART", err);
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case M_RESPONSE:
                    /*
                     *
                     */
                    break;
                case M_ACK:
                    /*
                     *
                     */
                    break;
                default:
                    break;
            }
    }
    return rc;
}
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   snmp_queue_init
 * PURPOSE:
 *      Initialize DB communication message receiver.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
snmp_queue_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    /* message CLI name */
    rc = osapi_msgCreate(
        SNMP_QUEUE_NAME,
        SNMP_QUEUE_LEN,
        sizeof(void *));
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SNMP, "%s: init queue:%s failed", __func__, SNMP_QUEUE_NAME);
        return MW_E_NOT_INITED;
    }

    rc = osapi_msgCreate(
        SNMP_QUEUE_GET,
        SNMP_QUEUE_GET_LEN,
        sizeof(void *));
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SNMP, "%s: init queue:%s failed", __func__, SNMP_QUEUE_GET);
        return MW_E_NOT_INITED;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   snmp_queue_free
 * PURPOSE:
 *      Release all allocated memory in snmp queue.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
snmp_queue_free(
    void)
{
    msghandle_t q_handle = osapi_msgFindHandle(SNMP_QUEUE_NAME);
    msghandle_t get_q_handle = osapi_msgFindHandle(SNMP_QUEUE_GET);

    if (NULL != q_handle && osapi_msgDelete(SNMP_QUEUE_NAME) != MW_E_OK)
    {
        MW_LOG_ERROR(SNMP, "%s:%d: Error: osapi_msgDelete for %s failed !", __FUNCTION__, __LINE__, SNMP_QUEUE_NAME);
    }
    if (NULL != get_q_handle && osapi_msgDelete(SNMP_QUEUE_GET) != MW_E_OK)
    {
        MW_LOG_ERROR(SNMP, "%s:%d: Error: osapi_msgDelete for %s failed !", __FUNCTION__, __LINE__, SNMP_QUEUE_GET);
    }

    MW_LOG_ERROR(SNMP, "snmp queue free!");
}

/* FUNCTION NAME:   snmp_queue_recv
 * PURPOSE:
 *      Receive DB communication message from DB.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_buf     --  pointer to pointer of receiving buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
snmp_queue_recv(
    void **pptr_buf)
{
    MW_ERROR_NO_T rc;
    UI8_T *ptr_msg = NULL;

    rc = osapi_msgRecv(
        SNMP_QUEUE_GET,
        &ptr_msg,
        0,
        MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    (*pptr_buf) = ptr_msg;

    return MW_E_OK;
}

/* FUNCTION NAME:   snmp_queue_send
 * PURPOSE:
 *      package message and call sending function to DB.
 *
 * INPUT:
 *      method          --  the method bitmap
 *      t_idx           --  the enum of the table
 *      f_idx           --  the enum of the field
 *      e_idx           --  the entry index in the table
 *      ptr_data        --  pointer to message data
 *      size            --  size of ptr_data
 *
 * OUTPUT:
 *      pptr_out_msg    -- double pointer to db message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
snmp_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    DB_PAYLOAD_T *ptr_payload = NULL;
    UI32_T          msg_size;


    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(
            msg_size,
            SNMP_MODULE_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_DEBUG(SNMP, "%s: allocate memory failed(%d)", __func__, rc);
        return MW_E_NO_MEMORY;
    }
    MW_LOG_DEBUG(SNMP, "ptr_msg=%p", ptr_msg);

    /* message */
    if (M_GET == method)
    {
        osapi_strncpy(ptr_msg->cq_name, SNMP_QUEUE_GET, DB_Q_NAME_SIZE);
    }
    else if (M_SUBSCRIBE == method)
    {
        osapi_strncpy(ptr_msg->cq_name, SNMP_QUEUE_NAME, DB_Q_NAME_SIZE);
    }

    ptr_msg->method = method;
    ptr_msg->type.count = 1;
    MW_LOG_DEBUG(SNMP, "cq_name=%s", ptr_msg->cq_name);
    MW_LOG_DEBUG(SNMP, "method=0x%X", ptr_msg->method);
    MW_LOG_DEBUG(SNMP, "count=%u", ptr_msg->type.count);
    MW_LOG_DEBUG(SNMP, "size=%u", size);

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    if ((size > 0) && (method != M_GET))
    {
        memcpy(&(ptr_payload->ptr_data), ptr_data, size);
    }

    MW_LOG_DEBUG(SNMP, "ptr_payload=%p", ptr_payload);
    MW_LOG_DEBUG(SNMP, "t_idx=%u", ptr_payload->request.t_idx);
    MW_LOG_DEBUG(SNMP, "f_idx=%u", ptr_payload->request.f_idx);
    MW_LOG_DEBUG(SNMP, "e_idx=%u", ptr_payload->request.e_idx);
    MW_LOG_DEBUG(SNMP, "data_size=%u", ptr_payload->data_size);

    /* Send message to DB */
    rc = _snmp_queue_send(ptr_msg, msg_size);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SNMP, "%s: Send message to DB failed(%d)", __func__, rc);
        return MW_E_OP_INCOMPLETE;
    }
    else
    {
        if (NULL != pptr_out_msg)
        {
            *pptr_out_msg = ptr_msg;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   snmp_queue_setData
 * PURPOSE:
 *      package message and call sending function to DB directly.
 *
 * INPUT:
 *      method      --  the method bitmap
 *      t_idx       --  the enum of the table
 *      f_idx       --  the enum of the field
 *      e_idx       --  the entry index in the table
 *      ptr_data    --  pointer to message data
 *      size        --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
snmp_queue_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    MW_LOG_DEBUG(SNMP, "%s: set DB data", __func__);
    rc = snmp_queue_send(method, t_idx, f_idx, e_idx, ptr_data, size, NULL);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SNMP, "%s: snmp_queue_send failed(%d)", __func__, rc);
        return rc;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   snmp_queue_getData
 * PURPOSE:
 *      1. Calculate db data size based on tid,fid,eid and then alloc memory
 *      2. Send db queue and wait db response
 *
 * INPUT:
 *      t_idx           --  the enum of the table
 *      f_idx           --  the enum of the field
 *      e_idx           --  the entry index in the table
 *
 * OUTPUT:
 *      pptr_out_msg    --  double pointer to db message
 *      ptr_out_size    --  pointer to size of ptr_data
 *      pptr_out_data   --  double pointer to db data in db payload
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      When return MW_E_OK, caller need to free the memory which pointed by ptr_out_msg!
 */
MW_ERROR_NO_T
snmp_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T           total_size = 0;
    DB_PAYLOAD_T *ptr_pload = NULL;

    DB_REQUEST_TYPE_T request = {
        .t_idx = in_t_idx,
        .f_idx = in_f_idx,
        .e_idx = in_e_idx
    };

    rc = dbapi_getDataSize(request, &total_size);
    if (MW_E_OK != rc)
    {
        MW_LOG_DEBUG(SNMP, "%s[%d] dbapi_getDataSize failed(%d)", __func__, __LINE__, rc);
        return rc;
    }
    MW_LOG_DEBUG(SNMP, "%s[%d] dbapi_getDataSize sucess, total_size = %d ", __func__, __LINE__, total_size);

    rc = snmp_queue_send(M_GET, in_t_idx, in_f_idx, in_e_idx, NULL, total_size, &ptr_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_DEBUG(SNMP, "%s[%d] snmp_queue_send failed(%d)", __func__, __LINE__, rc);
        return rc;
    }

    /* wait for DB response messgae */
    rc = snmp_queue_recv((void **)&ptr_msg);
    if (MW_E_OK == rc)
    {
        MW_LOG_DEBUG(SNMP, "%s[%d] snmp_queue_recv success", __func__, __LINE__);
    }
    else
    {
        MW_LOG_DEBUG(SNMP, "%s[%d] snmp_queue_recv failed(%d) ", __func__, __LINE__, rc);
        MW_FREE(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    (*ptr_out_size) = total_size;

    ptr_pload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    (*pptr_out_data) = &(ptr_pload->ptr_data);

    MW_LOG_DEBUG(SNMP, "%s[%d] *pptr_out_msg = %p, *pptr_out_data = %p ", __func__, __LINE__, *pptr_out_msg, *pptr_out_data);

    return MW_E_OK;
}

/* FUNCTION NAME: snmp_handle_db_msg
 * PURPOSE:
 *      waiting DB message and handle it.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
snmp_handle_db_msg(
    void)
{
    DB_MSG_T *ptr_msg = NULL;
    DB_REQUEST_TYPE_T request = { 0 };
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    do
    {
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

    if (FALSE == snmp_subscribe_done)
    {
        snmp_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, 0, 0, NULL);
        snmp_queue_send(M_SUBSCRIBE, SNMP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0, NULL);
        snmp_queue_send(M_SUBSCRIBE, SYS_OPER_INFO, SYS_OPER_IP_ADDR, DB_ALL_ENTRIES, 0, 0, NULL);
        snmp_queue_send(M_SUBSCRIBE, LOGON_INFO, LOGON_FAIL_COUNT, DB_ALL_ENTRIES, 0, 0, NULL);
        snmp_subscribe_done = TRUE;
        MW_LOG_INFO(SNMP, "%s[%d] Subscribe done.", __func__, __LINE__);
    }
    else
    {
        osapi_msgRecv(SNMP_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, 0);
        if (NULL != ptr_msg)
        {
            MW_LOG_DEBUG(SNMP, "%s[%d] Received DB message(msg id:%d).", __func__, __LINE__, ((MW_MSG_T *)ptr_msg)->msg_id);
            if (MW_MSG_ID_DB == ((MW_MSG_T *)ptr_msg)->msg_id)
            {
                if (0 != (ptr_msg->method & M_B_RESPONSE))
                {
                    /* Process Response message. Do nothing currently. */
                    MW_LOG_DEBUG(SNMP, "%s[%d] Received response message.", __func__, __LINE__);
                }
                else
                {
                    /* Process the notification and other messages. */
                    do
                    {
                        rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
                        if (MW_E_OK == rc)
                        {
                            MW_LOG_DEBUG(SNMP, "%s[%d]: recv method:0x%x, [t/f/e]:[%d/%d/%d]",
                                        __func__, __LINE__, ptr_msg->method, request.t_idx, request.f_idx, request.e_idx);
                            _snmp_db_msg_process(ptr_msg->method, &request, data_size, ptr_data);
                        }
                        /* Continue to parse the next request within the payload. */
                    } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
                }
            }
            MW_FREE(ptr_msg);
        }
    }
}
