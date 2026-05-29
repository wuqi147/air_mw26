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

/* FILE NAME:   loopPrevention.c
 * PURPOSE:
 *      CGI and SSI function of loop prevention web page.
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

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_HOURTOSEC   (3600)
#define MW_MINTOSEC    (60)

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
ssi_get_loopprevention_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T total_len = 0, len = 0;
    C8_T    *ptr_tmpbuf = NULL;

    /* DB variable*/
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;

    /* Loop prevention variable */
    DB_LOOP_PREVEN_INFO_T *ptr_lp = NULL;
    UI16_T size = 0, i = 0;

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>\n");
    if(err != MW_E_OK)
    {
        return err;
    }
    total_len += len;

    /* Request DB for loop prevention current information */
    err = httpd_queue_getData(LOOP_PREVEN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_lp);
    if(MW_E_OK == err)
    {
        err = send_format_response(&len, ptr_pcb, apiflags, "var lpEn = [%d];\n", ptr_lp->enable);
        MW_FREE(ptr_msg);
        if(err != MW_E_OK)
        {
            return err;
        }
        total_len += len;
    }
    else
    {
        return err;
    }

    err = osapi_calloc((MAX_PORT_NUM * 2), HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf);
    if(MW_E_OK != err)
    {
        return err;
    }
    /* Reqeust DB for port loop status information */
    err = httpd_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK == err)
    {
        len = 0;
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            len += snprintf(ptr_tmpbuf + len, ((MAX_PORT_NUM * 2) - len), "%d,", ptr_data[i]);
        }
        MW_FREE(ptr_msg);
        err = send_format_response(&len, ptr_pcb, apiflags, "var portState = [%s];\n", ptr_tmpbuf);
        MW_FREE(ptr_tmpbuf);
        if(err != MW_E_OK)
        {
            return err;
        }
        total_len += len;
    }
    else
    {
        MW_FREE(ptr_tmpbuf);
        return err;
    }

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    err = httpd_queue_getData(STP_INFO, STP_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        err = send_format_response(&len, ptr_pcb, apiflags, "var stpEn = %d;\n", *ptr_data);
        if(err != MW_E_OK)
        {
            MW_FREE(ptr_msg);
            return err;
        }
        total_len += len;
        MW_FREE(ptr_msg);
    }
#else
    err = send_format_response(&len, ptr_pcb, apiflags, "var stpEn = %d;\n", 0);
    if(err != MW_E_OK)
    {
        return err;
    }
    total_len += len;
#endif /* AIR_SUPPORT_RSTP */

    err = send_format_response(&len, ptr_pcb, apiflags, "</script>");
    if(err != MW_E_OK)
    {
        return err;
    }
    total_len += len;

    *ptr_length = total_len;

    return err;
}

MW_ERROR_NO_T
cgi_set_handle_loopPrevention(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T i = 0;
    UI8_T en = 0;

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(ptr_pcParam[i], "lpEn"))
        {
            en = atoi(ptr_pcValue[i]);
            /* Request loop prevention state to DB */
            rc = httpd_queue_setData(M_UPDATE, LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &en, sizeof(UI8_T));
            if(MW_E_OK != rc)
            {
                return rc;
            }
        }
    }

    return rc;
}
