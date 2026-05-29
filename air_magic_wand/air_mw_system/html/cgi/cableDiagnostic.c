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

/* FILE NAME:   cableDiagnostic.c
 * PURPOSE:
 *      CGI and SSI function of cable test web page.
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
#include "timer.h"
#include "sfp_task.h"
/* INCLUDE FILE DECLARATIONS
 */
#define MW_CABLE_STATE_NUM       (5)
#define MW_CABLE_STATE_SIZE      (7)
#define MW_CABLE_SET_PAIRA       (1)

#define SSI_CABLE_DIAGNOSTIC_NO_SUPPORT  (98)
#define SSI_CABLE_DIAGNOSTIC_TIMEOUT     (99)

#define SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN (64)
#define SERDES_DEBUG


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
static UI8_T _cableDiagnostic_portTypeList[MW_DEFAULT_MAX_PORT_NUM] = {0};
/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_cableCur_xmlHandle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T total_len = 0;
    UI16_T len = 0;
    C8_T *ptr_tmpbuf = NULL;
    const UI32_T tmpbuf_size = (MAX_PAIR_NUM* (MW_CABLE_STATE_SIZE * 2)) + MW_CABLE_STATE_SIZE;

    err = osapi_calloc(tmpbuf_size, HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf);
    if (MW_E_OK != err)
    {
        return err;
    }

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T i = 0, num = 0, size = 0;
    UI8_T f_idx = 0;

    for(i = 0; i < MW_CABLE_STATE_NUM; i++)
    {
        f_idx = (PORT_CABLE_SET + i);
        err = httpd_queue_getData(PORT_DIAG, f_idx, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
        if(MW_E_OK != err)
        {
            MW_FREE(ptr_tmpbuf);
            return err;
        }
        for(num = 0; num < MAX_PAIR_NUM; num++)
        {
            if((PORT_CABLE_LENGTH == f_idx) || (PORT_CABLE_IMPEDANCE == f_idx))
            {
                len += snprintf(ptr_tmpbuf + len, tmpbuf_size - len, "%d,", (ptr_data[num*2] | (ptr_data[num*2+1] << 8)));
            }
            else
            {
                len += snprintf(ptr_tmpbuf + len, tmpbuf_size - len, "%d,", ptr_data[num]);
            }
        }
        len += snprintf(ptr_tmpbuf + len, tmpbuf_size - len, "&");
        MW_FREE(ptr_msg);
    }
    air_wdog_kick();

    err = send_format_response(&len, ptr_pcb, apiflags, ptr_tmpbuf);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }
    total_len += len;
    *ptr_length = total_len;

    MW_FREE(ptr_tmpbuf);
    return err;
}

MW_ERROR_NO_T
ssi_get_cable_diag_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    I32_T err = MW_E_OK;
    UI16_T len = 0;
    UI32_T total_len = 0, i = 0;
    UI32_T port_idx = 0;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;

    /* Cable variable */
    DB_PORT_DIAG_T *ptr_cable = NULL;
    UI16_T size = 0;
    C8_T *ptr_str_buf_1 = NULL;
    C8_T *ptr_str_buf_2 = NULL;
    C8_T *ptr_str_buf_3 = NULL;
    C8_T *ptr_str_buf_4 = NULL;
    UI32_T str_len_1 = 0, str_len_2 = 0, str_len_3 = 0, str_len_4 = 0;
    C8_T *ptr_str_port_type_list = NULL;
    UI32_T str_port_type_list_len = 0;

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>\n");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;
#if(defined(AIR_SUPPORT_SFP))
    if (TRUE == sfp_check_ready())
    {
        err = ssi_utility_getAllPortType( _cableDiagnostic_portTypeList , sizeof(_cableDiagnostic_portTypeList));
        if (MW_E_OK != err)
        {
            return err;
        }
    }
#endif

    err = osapi_calloc(SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_str_buf_1);
    if (MW_E_OK != err)
    {
        return err;
    }

    err = osapi_calloc(SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_str_buf_2);
    if (MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        return err;
    }

    err = osapi_calloc(SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_str_buf_3);
    if (MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        return err;
    }
    err = osapi_calloc(SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_str_buf_4);
    if (MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        return err;
    }
    err = osapi_calloc(SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_str_port_type_list);
    if (MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        return err;
    }

    for (port_idx = 0; port_idx < PLAT_MAX_PORT_NUM; port_idx++)
    {
        if (port_idx < (PLAT_MAX_PORT_NUM -1) )
        {
            str_port_type_list_len += snprintf( (ptr_str_port_type_list+str_port_type_list_len),(64 - str_port_type_list_len),"%d,",_cableDiagnostic_portTypeList[port_idx]);
        }
        else
        {
            str_port_type_list_len += snprintf( (ptr_str_port_type_list+str_port_type_list_len),(64 - str_port_type_list_len),"%d",_cableDiagnostic_portTypeList[port_idx]);
        }
    }
    err = send_format_response(&len,ptr_pcb,apiflags,"var portTypeList = [%s];\n",ptr_str_port_type_list);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        MW_FREE(ptr_str_port_type_list);
        return err;
    }
    total_len += len;

    /* Request DB for check cable diagnostic is ready or not */
    err = httpd_queue_getData(PORT_DIAG, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_cable);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        MW_FREE(ptr_str_port_type_list);
        return err;
    }

    /* Update cable test current information */
    err = send_format_response(&len, ptr_pcb, apiflags, "var portNumber = [%d];\n", PLAT_MAX_PORT_NUM);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        MW_FREE(ptr_str_port_type_list);
        return err;
    }
    total_len += len;

    err = send_format_response(&len, ptr_pcb, apiflags, "var pairNumber = [%d];\n", MAX_PAIR_NUM);
    if(MW_E_OK != err)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        MW_FREE(ptr_str_port_type_list);
        return err;
    }
    total_len += len;

    for(i = 0; i < MAX_PAIR_NUM; i++)
    {
        str_len_1 += snprintf(ptr_str_buf_1 + str_len_1, SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN - str_len_1, "%d,", ptr_cable->set[i]);
        str_len_2 += snprintf(ptr_str_buf_2 + str_len_2, SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN - str_len_2, "%d,", ptr_cable->length[i]);
        str_len_3 += snprintf(ptr_str_buf_3 + str_len_3, SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN - str_len_3, "%d,", ptr_cable->state[i]);
        str_len_4 += snprintf(ptr_str_buf_4 + str_len_4, SSI_CABLE_DIAGNOSTIC_MAX_BUF_LEN - str_len_4, "%d,", ptr_cable->port[i]);
    }

    err = send_format_response(&len, ptr_pcb, apiflags, "var testresult_ds = {set:[%s],cablelength:[%s],result:[%s],port_idx:[%s]",
        ptr_str_buf_1, ptr_str_buf_2, ptr_str_buf_3, ptr_str_buf_4);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_str_buf_1);
        MW_FREE(ptr_str_buf_2);
        MW_FREE(ptr_str_buf_3);
        MW_FREE(ptr_str_buf_4);
        MW_FREE(ptr_str_port_type_list);
        return err;
    }
    total_len += len;
    MW_FREE(ptr_str_buf_1);
    MW_FREE(ptr_str_buf_2);
    MW_FREE(ptr_str_buf_3);
    MW_FREE(ptr_str_buf_4);
    MW_FREE(ptr_str_port_type_list);
    err = send_format_response(&len, ptr_pcb, apiflags, "};\n</script>");
    if(ERR_OK != err)
    {
        MW_FREE(ptr_msg);
        return err;
    }
    total_len += len;
    MW_FREE(ptr_msg);

    *ptr_length = total_len;

    return err;
}

MW_ERROR_NO_T
cgi_set_handle_cableDiag(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T port = 0, i = 0;

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(ptr_pcParam[i], "port_sel"))
        {
            port = atoi(ptr_pcValue[i])+1;
        }
    }
#if(defined(AIR_SUPPORT_SFP))
    /*query port type*/
    if ((TRUE == sfp_check_ready()) && (cgi_utility_checkPortAvailForCableDiag(port) == FALSE))
    {
        UI8_T setting[MAX_PAIR_NUM] = {0};

        memset( setting , SSI_CABLE_DIAGNOSTIC_NO_SUPPORT , (sizeof(UI8_T) * MAX_PAIR_NUM) );
        rc = httpd_queue_setData(M_UPDATE, PORT_DIAG, PORT_CABLE_SET, DB_ALL_ENTRIES, &setting, sizeof(setting));
    }
    else
#endif
    {
        /* Tell DB do cable diagnostic of specific port */
        rc = httpd_queue_setData(M_UPDATE, PORT_DIAG, PORT_CABLE_SET, MW_CABLE_SET_PAIRA, &port, sizeof(UI8_T));
    }
    return rc;
}

