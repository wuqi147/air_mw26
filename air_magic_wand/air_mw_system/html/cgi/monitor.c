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

/* FILE NAME:   monitor.c
 * PURPOSE:
 *      CGI and SSI function of port monitor web page.
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
#include "web.h"
#include <mw_platform.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_MAX_STRING_SIZE  (20 * 28)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    CGI_TRAFFIC_MONITER_CLEAR_NONE,         /* Do not clear MIB counter */
    CGI_TRAFFIC_MONITER_CLEAR_SINGLE_PORT,  /* Clear MIB counter of a specific port */
    CGI_TRAFFIC_MONITER_CLEAR_ALL_PORTS,    /* Clear MIB counters of all ports */
    CGI_TRAFFIC_MONITER_CLEAR_STATE_LAST
} CGI_TRAFFIC_MONITER_CLEAR_T;

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


char ssi_get_port_data_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{

    C8_T err = 0;
    UI16_T len = 0;
    UI32_T total_len = 0;

    CGI_LOG_DEBUG(TRAFFIC_MON, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    /* Only display port number, mib information will pop from XML */
    err = send_format_response(&len, pcb, apiflags, "<script> var portNumber = %d; </script>", PLAT_MAX_PORT_NUM);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(TRAFFIC_MON, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    return ERR_OK;
}

MW_ERROR_NO_T ssi_get_mib_state_xmlHandle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{

    C8_T err = 0;
    UI16_T len = 0, total_len = 0;
    C8_T uint64_string[20] = {0};
    C8_T *ptr_tmpbuf = NULL;
    UI64_T *ptr_port = NULL;
    UI16_T size = 0, i = 0;
    DB_MSG_T *ptr_msg = NULL;

    err = osapi_calloc(MW_MAX_STRING_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf);
    if (MW_E_OK != err)
    {
        return err;
    }

    /* txbytes */
    err = httpd_queue_getData(MIB_CNT, MIB_CNT_TX_OCTETS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_port);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }

    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    memset(&uint64_string ,0 , sizeof(uint64_string));
    for(i = 0;i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%s,", _printUI64_T((UI64_T)ptr_port[i], uint64_string, sizeof(uint64_string)));
    }
    ptr_tmpbuf[len] = ';';
    len += 1;
    MW_FREE(ptr_msg);

    err = send_format_response((UI16_T *)&len, pcb, apiflags, ptr_tmpbuf);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;


    /* rxbytes */
    err = httpd_queue_getData(MIB_CNT, MIB_CNT_RX_OCTETS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_port);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }

    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    memset(&uint64_string ,0 , sizeof(uint64_string));
    for(i = 0;i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%s,", _printUI64_T((UI64_T)ptr_port[i], uint64_string, sizeof(uint64_string)));
    }
    ptr_tmpbuf[len] = ';';
    len += 1;
    MW_FREE(ptr_msg);

    err = send_format_response((UI16_T *)&len, pcb, apiflags, ptr_tmpbuf);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    /* txpkts */
    err = httpd_queue_getData(MIB_CNT, MIB_CNT_TX_PACKETS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_port);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    memset(&uint64_string ,0 , sizeof(uint64_string));
    for(i = 0;i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%s,", _printUI64_T((UI64_T)ptr_port[i], uint64_string, sizeof(uint64_string)));
    }
    ptr_tmpbuf[len] = ';';
    len += 1;
    MW_FREE(ptr_msg);

    err = send_format_response((UI16_T *)&len, pcb, apiflags, ptr_tmpbuf);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;


    /*  rxpkts */
    err = httpd_queue_getData(MIB_CNT, MIB_CNT_RX_PACKETS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_port);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    memset(&uint64_string ,0 , sizeof(uint64_string));
    for(i = 0;i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%s,", _printUI64_T((UI64_T)ptr_port[i], uint64_string, sizeof(uint64_string)));

    }
    MW_FREE(ptr_msg);

    err = send_format_response((UI16_T *)&len, pcb, apiflags, ptr_tmpbuf);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    CGI_LOG_DEBUG(TRAFFIC_MON, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    MW_FREE(ptr_tmpbuf);
    return ERR_OK;
}


MW_ERROR_NO_T ssi_get_clear_mib_counter_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags)
{
    C8_T err = 0;
    UI16_T len = 0;
    UI32_T total_len = 0;
    C8_T *ptr_tmpbuf = NULL;
    UI16_T i = 0;

    err = osapi_calloc(MW_MAX_STRING_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf);
    if (MW_E_OK != err)
    {
        return err;
    }

    /* txbytes */
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%d,", 0);
    }
    ptr_tmpbuf[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_tmpbuf);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    /* rxbytes */
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%d,", 0);
    }
    ptr_tmpbuf[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_tmpbuf);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    /* txpkts */
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%d,", 0);
    }
    ptr_tmpbuf[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_tmpbuf);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    /* rxpkts */
    len = 0;
    memset(ptr_tmpbuf, 0, MW_MAX_STRING_SIZE);
    for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
    {
        len += snprintf(ptr_tmpbuf+len, MW_MAX_STRING_SIZE - len, "%d,", 0);
    }

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_tmpbuf);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;

    CGI_LOG_DEBUG(TRAFFIC_MON, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    MW_FREE(ptr_tmpbuf);
    return ERR_OK;
}


/* clear_mib_counter.cgi */
MW_ERROR_NO_T
cgi_set_handle_clearMibCounter(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_bitmap = 0, i = 0;
    CGI_TRAFFIC_MONITER_CLEAR_T clear_mib_counter = CGI_TRAFFIC_MONITER_CLEAR_SINGLE_PORT;

    /* Parser cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(ptr_pcParam[i], "port_bitmap"))
        {
            rc = osapi_strtou32(ptr_pcValue[i], &port_bitmap);
            if (rc != MW_E_OK)
            {
                /* Conversion failed: not a valid integer string */
                return rc;
            }
        }
    }

    for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
    {
        if(0 == (port_bitmap & (0x01 << i)))
        {
            break;
        }
    }

    if(PLAT_MAX_PORT_NUM <= i)
    {
        /* Clear all ports */
        clear_mib_counter = CGI_TRAFFIC_MONITER_CLEAR_ALL_PORTS;
    }

    if(CGI_TRAFFIC_MONITER_CLEAR_ALL_PORTS == clear_mib_counter)
    {
        /* When clear all ports, notify port 1 only and SYNCD will clear MIB counters of all ports by checking clear_mib_counter. */
        rc = httpd_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_MIB_COUNTER_CLEAR, 1, &clear_mib_counter, sizeof(UI8_T));
    }
    else
    {
        for(i = 0; PLAT_MAX_PORT_NUM > i; i++)
        {
            if(port_bitmap & (0x01 << i))
            {
                rc = httpd_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_MIB_COUNTER_CLEAR, (i + 1), &clear_mib_counter, sizeof(UI8_T));
                if(MW_E_OK != rc)
                {
                    break;
                }
            }
        }
    }

    return rc;
}

