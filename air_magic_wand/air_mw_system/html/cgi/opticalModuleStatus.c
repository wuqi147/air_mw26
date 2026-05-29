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

/* FILE NAME:   opticalModuleStatus.c
 * PURPOSE:
 *      CGI and SSI function of optical module status web page.
 * NOTES:
 */
#ifdef AIR_SUPPORT_SFP_DDM
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "mw_error.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "mw_log.h"
#include "web.h"
#include <mw_platform.h>


/* NAMING CONSTANT DECLARATIONS
 */
#define MAX_STRING_SIZE  (20 * 5)

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
static UI32_T port_count = 0;

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */

char
ssi_get_sfp_port_data_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI32_T len = 0, i = 0;
    UI8_T port_num[SFP_MODULE_INFO_MAX_NUM];
    C8_T uint64_string[20] = {0};
    C8_T *ptr_ssi_str = NULL;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0, total_len = 0;
    void *ptr_db_data = NULL;

    CGI_LOG_DEBUG(SFP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    /* get port_num*/
    rc = httpd_queue_getData(SFP_MODULE_INFO, SFP_MODULE_PORTNUM, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SFP, "get port_num success, ptr_msg =%p\n", ptr_db_msg);
    } else
    {
        CGI_LOG_ERROR(SFP, "get port_num failed \n");
        return ERR_VAL;
    }
    osapi_memset(port_num, 0, sizeof(port_num));
    osapi_memcpy(port_num, ptr_db_data, db_size);
    MW_FREE(ptr_db_msg);

    /* send total number of sfp port*/
    port_count = 0;
    for (i = 0; i < sizeof(port_num); i++)
    {
        if (0 != port_num[i])
        {
            port_count++;
        }
    }

    /* Only display port number and total number of sfp port, ddm information will pop from XML */
    /* send port_num*/
    len = 0;
    if (MW_E_NO_MEMORY == osapi_calloc(MAX_STRING_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        return MW_E_NO_MEMORY;
    }
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)port_num[i], uint64_string, sizeof(uint64_string)));
    }

    err = send_format_response(&total_len, ptr_pcb, apiflags, "<script> var memberNum=%d;var sfpPortNumber=[%s];</script>", port_count, ptr_ssi_str);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_ssi_str);
        return ERR_VAL;
    }
    MW_FREE(ptr_ssi_str);

    *ptr_length = total_len;

    CGI_LOG_DEBUG(SFP, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    return ERR_OK;
}

MW_ERROR_NO_T
ssi_get_sfp_module_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T total_len = 0, i = 0;
    C8_T err = 0;
    C8_T uint64_string[20] = {0};
    C8_T *ptr_ssi_str = NULL;
    UI8_T loop;
    DB_SFP_MODULE_INFO_T module_info[SFP_MODULE_INFO_MAX_NUM];
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0, len = 0;
    void *ptr_db_data = NULL;

    CGI_LOG_DEBUG(SFP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    /*get module info*/
    for (loop = 0; loop < port_count; loop++)
    {
        osapi_memset(&module_info[loop], 0, sizeof(DB_SFP_MODULE_INFO_T));
        rc = httpd_queue_getData(SFP_MODULE_INFO, DB_ALL_FIELDS, (loop + 1) , &ptr_db_msg, &db_size, &ptr_db_data);
        if (MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(SFP, "get port_mode success, ptr_msg =%p\n", ptr_db_msg);
        }
        else
        {
            CGI_LOG_ERROR(SFP, "get port_mode failed \n");
            return rc;
        }

        osapi_memcpy(&module_info[loop], ptr_db_data, db_size);
        osapi_free(ptr_db_msg);
    }

    /*send all ddm type*/
    len = 0;
    if (MW_E_NO_MEMORY == osapi_calloc(MAX_STRING_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        return MW_E_NO_MEMORY;
    }

    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].diagnostic_monitoring_type, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all loss of signal*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].loss_of_signal, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all temp*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].temperature, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all vol*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].voltage, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all current*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].current, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all tx power*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].output_power, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    total_len += len;

    /*send all rx power*/
    len = 0;
    osapi_memset(ptr_ssi_str, 0, MAX_STRING_SIZE);
    osapi_memset(&uint64_string ,0 , sizeof(uint64_string));
    for (i = 0; i < port_count; i++)
    {
        len += snprintf(ptr_ssi_str+len, (MAX_STRING_SIZE-len), "%s,", _printUI64_T((UI64_T)module_info[i].input_power, uint64_string, sizeof(uint64_string)));
    }
    ptr_ssi_str[len] = ';';
    len += 1;

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_ssi_str);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_OTHERS;
    }
    MW_FREE(ptr_ssi_str);

    total_len += len;

    *ptr_length = total_len;

    CGI_LOG_DEBUG(SFP, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    return MW_E_OK;
}
#endif  /* AIR_SUPPORT_SFP_DDM */
