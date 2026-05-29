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

/* FILE NAME:  icmp_client.c
 * PURPOSE:
 *  It provides ICMP Client module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "inc/icmp_client.h"
#include "inc/ping_queue.h"
#include "osapi_memory.h"
#include "lwip/opt.h"
#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */
#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/inet_chksum.h"
#include "lwip/prot/ip4.h"
#include "lwip/api.h"
#if PING_USE_SOCKETS
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <string.h>
#endif /* PING_USE_SOCKETS */

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

/* ping variables */
static UI32_T _db_ping_status;
static threadhandle_t _ptr_ping = NULL;

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */

#if PING_USE_SOCKETS
/* FUNCTION NAME:   ping_thread_init
 * PURPOSE:
 *      Initialize ping thread
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Ping using the socket ip
 */
MW_ERROR_NO_T
ping_thread_init(void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    err_t err;
    DB_ICMP_CLIENT_INFO_T *ptr_db_ping_info = NULL;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;
    UI8_T ip_addr1 = 0;

    rc = osapi_calloc(sizeof(DB_ICMP_CLIENT_INFO_T), ICMP_CLIENT_MODULE_NAME, (void**)&ptr_db_ping_info);
    if (MW_E_OK != rc)
    {
        PING_LOG_ERROR("malloc memory failed");
        return rc;
    }
    /* Initialize global variable */
    rc = ping_global_init();
    if (MW_E_OK != rc)
    {
        PING_LOG_ERROR("ping global init failed");
        osapi_free(ptr_db_ping_info);
        return rc;
    }
    /* Initialize ping queue */
    rc = ping_queue_init();
    if (MW_E_OK != rc)
    {
        PING_LOG_ERROR("ping queue init failed");
        osapi_free(ptr_db_ping_info);
        return rc;
    }
    /* get hostname from db */
    rc = ping_queue_getData(ICMP_CLIENT_INFO, ICMP_CLIENT_HOST_NAME, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        memcpy(ptr_db_ping_info->host_name, ptr_db_data, db_size);
        osapi_free(ptr_db_msg);
        PING_LOG_DEBUG("get hostname from db: \"%s\"",ptr_db_ping_info->host_name);
    }
    else
    {
        PING_LOG_ERROR("get hostname from db failed");
        osapi_free(ptr_db_ping_info);
        return rc;
    }
    /* get ping numbers from db */
    rc = ping_queue_getData(ICMP_CLIENT_INFO, NUM, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        memcpy(&(ping_info.ping_num), ptr_db_data, db_size);
        ptr_db_ping_info->num = ping_info.ping_num;
        osapi_free(ptr_db_msg);
    }
    else
    {
        PING_LOG_ERROR("get ping num failed");
        osapi_free(ptr_db_ping_info);
        return rc;
    }

    /* check status */
    ping_info.status = *(ping_get_db_ping_status());
    /* user click stop when ping thread init */
    if(AIR_ICMP_CLIENT_CANCELED == ping_info.status)
    {
        PING_LOG_DEBUG("canceled");
        osapi_free(ptr_db_ping_info);
        ping_thread_free_resource();
    }
    /* update status */
    ptr_db_ping_info->status = AIR_ICMP_CLIENT_DNS_PROCESSING;
    rc = ping_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_db_ping_info,sizeof(DB_ICMP_CLIENT_INFO_T));
    if (MW_E_OK != rc)
    {
        PING_LOG_ERROR("update status failed");
        osapi_free(ptr_db_ping_info);
        return rc;
    }
#if LWIP_DNS
    /* get ip addr by hostname */
    err = netconn_gethostbyname((const char *) ptr_db_ping_info->host_name,ping_target);
    if(ERR_OK == err)
    {
        PING_LOG_DEBUG("get ip addr:%s from dns",ipaddr_ntoa(ping_target));
    }
    else
    {
        PING_LOG_ERROR("gethostbyname(%s) failed, err=%d", ptr_db_ping_info->host_name, err);
        ping_info.status = AIR_ICMP_CLIENT_DNS_TIMEOUT;
        osapi_free(ptr_db_ping_info);
        return MW_E_NOT_INITED;
    }
#else
    if(inet_aton((const char *)ptr_db_ping_info->host_name,ping_target) == 0)
    {
        PING_LOG_ERROR("inet_aton() convert ip addr failed");
        ping_info.status = AIR_ICMP_CLIENT_DNS_TIMEOUT;
        osapi_free(ptr_db_ping_info);
        return MW_E_NOT_INITED;
    }
#endif /* LWIP_DNS */
    ptr_db_ping_info->ping_ip = ip_addr_get_ip4_u32(ping_target);
    ip_addr1 = ip4_addr1(ip_2_ip4(ping_target));
    /* valid IP  except zero network */
    if((0 == ip_addr1))
    {
        ping_info.status = AIR_ICMP_CLIENT_DNS_TIMEOUT;
        osapi_free(ptr_db_ping_info);
        return MW_E_NOT_INITED;
    }
    ping_queue_setData(M_UPDATE, ICMP_CLIENT_INFO,PING_IP, DB_ALL_ENTRIES, &(ptr_db_ping_info->ping_ip),sizeof(MW_IPV4_T));
    /* check status */
    ping_info.status = *(ping_get_db_ping_status());
    /* user click stop when ping thread init */
    if(AIR_ICMP_CLIENT_CANCELED == ping_info.status)
    {
        PING_LOG_DEBUG("canceled");
        osapi_free(ptr_db_ping_info);
        ping_thread_free_resource();
    }
    ping_info.status = AIR_ICMP_CLIENT_PROCESSING;
    ping_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &(ping_info.status),sizeof(u16_t));
    osapi_free(ptr_db_ping_info);
    PING_LOG_INFO("ping thread init done");
    return MW_E_OK;
}

#endif /* PING_USE_SOCKETS */

/* FUNCTION NAME:   ping_get_db_ping_status
 * PURPOSE:
 *      Get the status of the global varible _db_ping_status
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      &_db_ping_status
 *
 * NOTES:
 *      None
 */
UI32_T *
ping_get_db_ping_status(void)
{
    return &_db_ping_status;
}

/* FUNCTION NAME:   ping_set_db_ping_status
 * PURPOSE:
 *      Set the status of the global varible _db_ping_status
 *
 * INPUT:
 *      status             -- status about ping
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_set_db_ping_status(
    UI32_T status)
{
    UI32_T *ptr_status = NULL;
    ptr_status = ping_get_db_ping_status();
    *ptr_status = status;
    return MW_E_OK;
}

/* FUNCTION NAME:   ping_create_ping_thread
 * PURPOSE:
 *      Create ping thread
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_create_ping_thread(void)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;
    if(NULL == _ptr_ping)
    {
        ret = osapi_processCreate("ping_thread", MW_PING_TASK_SIZE, MW_PING_TASK_PRI, ping_thread, NULL, &_ptr_ping);
    }
    return ret;
}
/* FUNCTION NAME:   ping_thread_free_resource
 * PURPOSE:
 *      Free ping thread resource
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_thread_free_resource(void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    threadhandle_t ptr_ping = NULL;
    PING_LOG_DEBUG("ping free resource");
    ret = osapi_free(ping_target);
    if(MW_E_OK == ret)
    {
        ping_target = NULL;
    }
    ping_queue_free();
    ptr_ping = _ptr_ping;
    _ptr_ping = NULL;
    ret = osapi_processDelete(ptr_ping);
    return ret;
}

/* FUNCTION NAME:   ping_global_init
 * PURPOSE:
 *      Send icmp packet by socket
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_global_init(void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    memset(&ping_info, 0, sizeof(ping_info));
    ret = osapi_malloc(sizeof(ip_addr_t), ICMP_CLIENT_MODULE_NAME, (void**)&ping_target);
    return ret;
}
#endif /* LWIP_RAW */
