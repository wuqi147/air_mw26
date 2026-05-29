/*******************************************************************************
 * Copyright Statement:
 *
 * (C) 2024 Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement
 * ("Permitted User").  If you are not a Permitted User, please cease any access
 * or use of Airoha Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (“AIROHA SOFTWARE”)
 * RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
 * ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
 * SUCH AIROHA SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("Airoha
 * Software") have been modified by Airoha Corp. All revisions are subject to
 * any receiver's applicable license agreements with Airoha Corp.
 ********************************************************************************/

/* FILE NAME:   systime.c
 * PURPOSE:
 *      CGI and SSI function of port trunk web page.
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
#include "httpd_queue.h"
#include "osapi_memory.h"
#include "web.h"
#include "lwip/apps/sntp.h"
#include "sys_mgmt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CLOCK_MODE_INIT     (0xF)

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
MW_ERROR_NO_T
cgi_set_systime_info_handle(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    int i;
    UI8_T clkmode = CLOCK_MODE_INIT;
    UI32_T ip = 0;
    I8_T timezone;
    MW_DATE_T date = {0};

    date.year = 0;
    for (i = 0; i < iNumParams; i++)
    {
        /*mode check*/
        if(!strcmp(pcParam[i], "clkmode"))
        {
            clkmode = atoi(pcValue[i]);
        }
        /*Date*/
        else if(!strcmp(pcParam[i], "year"))
        {
            date.year = atoi(pcValue[i]);
        }
        else if(!strcmp(pcParam[i], "month"))
        {
            date.month = atoi(pcValue[i]);
        }
        else if(!strcmp(pcParam[i], "day"))
        {
            date.day = atoi(pcValue[i]);
        }
        /*Time*/
        else if(!strcmp(pcParam[i], "hour"))
        {
            date.hours = atoi(pcValue[i]);
        }
        else if(!strcmp(pcParam[i], "minute"))
        {
            date.minutes = atoi(pcValue[i]);
        }
        else if(!strcmp(pcParam[i], "second"))
        {
            date.seconds = atoi(pcValue[i]);
        }
        else if(!strcmp(pcParam[i], "ip0"))
        {
            ip = getIpaddr(pcValue[i]);
            rc = httpd_queue_setData(M_UPDATE, SNTP_CFG, SNTP_SERVER_1, DB_ALL_ENTRIES, &ip, sizeof(ip));
        }
        else if(!strcmp(pcParam[i], "ip1"))
        {
            ip = getIpaddr(pcValue[i]);
            rc = httpd_queue_setData(M_UPDATE, SNTP_CFG, SNTP_SERVER_2, DB_ALL_ENTRIES, &ip, sizeof(ip));
        }
        else if(!strcmp(pcParam[i], "ip2"))
        {
            ip = getIpaddr(pcValue[i]);
            rc = httpd_queue_setData(M_UPDATE, SNTP_CFG, SNTP_SERVER_3, DB_ALL_ENTRIES, &ip, sizeof(ip));
        }
        else if(!strcmp(pcParam[i], "timezone"))
        {
            timezone = atoi(pcValue[i]);
            rc = httpd_queue_setData(M_UPDATE, SNTP_CFG, SNTP_TIMEZONE, DB_ALL_ENTRIES, &timezone, sizeof(timezone));
        }
        else
        {
            CGI_LOG_DEBUG(SYSTEM, "%s: %d Wrong parameter\n", __func__, __LINE__);
        }
    }
    if(0 != date.year)
    {
        sys_mgmt_setTimes(date);
    }
    if(CLOCK_MODE_INIT != clkmode)
    {
        rc = httpd_queue_setData(M_UPDATE, SNTP_CFG, SNTP_CLOCK_MODE, DB_ALL_ENTRIES, &clkmode, sizeof(clkmode));
    }

    return rc;
}

char
ssi_get_systime_info_handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    unsigned int total_len = 0;
    UI16_T len = 0;
    char err;
    UI64_T times = 0;
    MW_DATE_T date;
    UI32_T serverIp = 0;
    ip_addr_t s;
    DB_MSG_T *db_msg = NULL;
    u16_t db_size = 0;
    void *db_data = NULL;

    sys_mgmt_getTimes(&times, &date);

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>");
    if(ERR_OK != err)
    {
        return err;
    }

    total_len += len;

    if(httpd_queue_getData(SNTP_CFG, SNTP_CLOCK_MODE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        return ERR_INPROGRESS;
    }
    err = send_format_response(&len, ptr_pcb, apiflags, "var timermode = %d;", ((UI8_T*)db_data)[0]);
    if(ERR_OK != err)
    {
        return err;
    }

    err = send_format_response(&len, ptr_pcb, apiflags, "var curtime = new Array(%u, %u, %u, %u, %u, %u);", date.year, date.month, date.day,
            date.hours, date.minutes, date.seconds);
    if(ERR_OK != err)
    {
        return err;
    }

    if(httpd_queue_getData(SNTP_CFG, SNTP_TIMEZONE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        return ERR_INPROGRESS;
    }
    err = send_format_response(&len, ptr_pcb, apiflags, "var timezone = %d;", ((I8_T*)db_data)[0]);
    osapi_free(db_msg);
    if(ERR_OK != err)
    {
        return err;
    }

    err = send_format_response(&len, ptr_pcb, apiflags, "var sntpmaxidx = %d;", SNTP_MAX_SERVERS);
    if(ERR_OK != err)
    {
        return err;
    }

    if(httpd_queue_getData(SNTP_CFG, SNTP_SERVER_1, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        return ERR_INPROGRESS;
    }
    memcpy(&serverIp, db_data, db_size);
    osapi_free(db_msg);
    ip_addr_set_ip4_u32_val(s, serverIp);
    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, "var server1 = '%s';", ipaddr_ntoa(&s));
    if(err != ERR_OK)
    {
        return err;
    }

    if(httpd_queue_getData(SNTP_CFG, SNTP_SERVER_2, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        return ERR_INPROGRESS;
    }
    memcpy(&serverIp, db_data, db_size);
    osapi_free(db_msg);
    ip_addr_set_ip4_u32_val(s, serverIp);
    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, "var server2 = '%s';", ipaddr_ntoa(&s));
    if(err != ERR_OK)
    {
        return err;
    }

    if(httpd_queue_getData(SNTP_CFG, SNTP_SERVER_3, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data) != MW_E_OK)
    {
        return ERR_INPROGRESS;
    }
    memcpy(&serverIp, db_data, db_size);
    osapi_free(db_msg);
    ip_addr_set_ip4_u32_val(s, serverIp);
    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, "var server3 = '%s';", ipaddr_ntoa(&s));
    if(err != ERR_OK)
    {
        return err;
    }

    err = send_format_response(&len, ptr_pcb, apiflags, "</script>");
    if(ERR_OK != err)
    {
        return err;
    }

    total_len += len;
    *length = total_len;

    return MW_E_OK;
}

char
ssi_get_timeCur_xmlHandle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    unsigned int total_len = 0;
    UI16_T len = 0;
    char err;
    UI64_T times = 0;
    MW_DATE_T date;

    sys_mgmt_getTimes(&times, &date);

    err = send_format_response(&len, ptr_pcb, apiflags, "%u,%u,%u,%u,%u,%u", date.year, date.month, date.day,
            date.hours, date.minutes, date.seconds);
    if(ERR_OK != err)
    {
        return err;
    }

    total_len += len;
    *length = total_len;

    return MW_E_OK;

}

