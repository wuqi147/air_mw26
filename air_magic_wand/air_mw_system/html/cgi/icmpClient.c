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

/* FILE NAME:  icmpClient.c
 * PURPOSE:
 *  CGI and SSI function of ping web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "mw_error.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#include "web.h"

/* INCLUDE FILE DECLARATIONS
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
char ssi_get_pinginfo_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags)
{
    C8_T err = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T len = 0;
    char tmpbuf[256] = {0};
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;
    DB_ICMP_CLIENT_INFO_T *ptr_ping_info = NULL;
    /* get ping info */
    rc = httpd_queue_getData(ICMP_CLIENT_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    ptr_ping_info = (DB_ICMP_CLIENT_INFO_T *)ptr_db_data;

    len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%d.%d.%d.%d;"
                , (0xFF & ((UI8_T*)(&ptr_ping_info->ping_ip))[0])
                , (0xFF & ((UI8_T*)(&ptr_ping_info->ping_ip))[1])
                , (0xFF & ((UI8_T*)(&ptr_ping_info->ping_ip))[2])
                , (0xFF & ((UI8_T*)(&ptr_ping_info->ping_ip))[3]));

    len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%d;%d;%d;%d;%d;%d;%d"
                , ptr_ping_info->send_packets
                , ptr_ping_info->recv_packets
                , ptr_ping_info->lost_packets
                , ptr_ping_info->min_rtt
                , ptr_ping_info->max_rtt
                , ptr_ping_info->ave_rtt
                , ptr_ping_info->status);
    osapi_free(ptr_db_msg);
    err = send_format_response(&len, ptr_pcb, apiflags, tmpbuf);
    if(ERR_OK != err)
    {
        return err;
    }
    *ptr_length = len;

    CGI_LOG_DEBUG(PING, "s:%d r:%d l:%d min:%d max:%d ave:%d sta:%d",ptr_ping_info->send_packets,
                ptr_ping_info->recv_packets,
                ptr_ping_info->lost_packets,
                ptr_ping_info->min_rtt,
                ptr_ping_info->max_rtt,
                ptr_ping_info->ave_rtt,
                ptr_ping_info->status);

    return ERR_OK;
}

char ssi_get_ping_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags)
{
    UI16_T len = 0, total_len = 0;
    UI8_T *ptr_hostname = NULL;
    char err = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;
    UI16_T num = 0;
    UI16_T status = 0;
    /* get host name  */
    rc = httpd_queue_getData(ICMP_CLIENT_INFO, ICMP_CLIENT_HOST_NAME, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PING, "get hostname failed");
        return ERR_VAL;
    }

    ptr_hostname = (UI8_T *)ptr_db_data;
    err = send_format_response(&len, ptr_pcb, apiflags, "<script> var input_info = { host:\"%s\",",ptr_hostname);
    MW_FREE(ptr_db_msg);
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    /* get current ping ip  */
    rc = httpd_queue_getData(ICMP_CLIENT_INFO, PING_IP, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PING, "get current ip failed");
        return ERR_VAL;
    }
    err = send_format_response(&len, ptr_pcb, apiflags, "cur_ping_ip:\"%d.%d.%d.%d\","
                , (0xFF & ((UI8_T*)ptr_db_data)[0])
                , (0xFF & ((UI8_T*)ptr_db_data)[1])
                , (0xFF & ((UI8_T*)ptr_db_data)[2])
                , (0xFF & ((UI8_T*)ptr_db_data)[3]));
    MW_FREE(ptr_db_msg);
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    /* get num of pings  */
    rc = httpd_queue_getData(ICMP_CLIENT_INFO, NUM, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PING, "get num failed");
        return ERR_VAL;
    }
    memcpy(&num, ptr_db_data, db_size);
    MW_FREE(ptr_db_msg);
    err = send_format_response(&len, ptr_pcb, apiflags, "num:%d,", num);
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    /* get ping status  */
    rc = httpd_queue_getData(ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PING, "get status failed");
        return ERR_VAL;
    }
    memcpy(&status, ptr_db_data, db_size);
    MW_FREE(ptr_db_msg);
    err = send_format_response(&len, ptr_pcb, apiflags, "status:%d};", status);
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, ptr_pcb, apiflags, "</script>");
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;
    *ptr_length = total_len;

    CGI_LOG_DEBUG(PING, "Get hostname: %s num: %d status: %d", ptr_hostname, num, status);

    return ERR_OK;
}

MW_ERROR_NO_T cgi_set_handle_pinginfo(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i = 0;
    int rc = MW_E_OK;
    UI8_T *ptr_hostname = NULL;
    UI16_T num = 0;
    UI16_T status = 0;

    rc = osapi_calloc((MAX_HOST_NAME_SIZE), "icping", (void**)&ptr_hostname);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    /* get value from url params */
    for(i = 0 ; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "host"))
        {
            osapi_snprintf((C8_T *)ptr_hostname, (MAX_HOST_NAME_SIZE - 1), "%s", pcValue[i]);
        }
        if(!strcmp(pcParam[i], "num"))
        {
            num = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "status"))
        {
            status = atoi(pcValue[i]);
        }
    }
    CGI_LOG_DEBUG(PING, "set hostname: %s num: %d status: %d",ptr_hostname, num, status);

    if(status && (AIR_ICMP_CLIENT_ERR_START == status))
    {
        if(0 != strlen((const C8_T *) ptr_hostname))
        {
            /* set hostname */
            rc = httpd_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, ICMP_CLIENT_HOST_NAME, DB_ALL_ENTRIES, ptr_hostname, MAX_HOST_NAME_SIZE);
            if(MW_E_OK != rc)
            {
                MW_FREE(ptr_hostname);
                return ERR_VAL;
            }
        }
        if(num)
        {
            /* set ping numbers */
            rc = httpd_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, NUM, DB_ALL_ENTRIES, &num, sizeof(num));
        }
        /* set status */
        rc = httpd_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &status, sizeof(status));
    }
    else if(AIR_ICMP_CLIENT_CANCELED == status)
    {
        /* browser clicked cancel*/
        rc = httpd_queue_setData(M_UPDATE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &status, sizeof(status));
    }
    MW_FREE(ptr_hostname);
    return rc;
}
