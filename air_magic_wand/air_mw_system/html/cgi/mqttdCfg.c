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
/* FILE NAME:   mqttdCfg.c
 * PURPOSE:
 *      CGI and SSI function of Cloud Connection web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "osapi_string.h"
#include "osapi_memory.h"

#include "mw_utils.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "web.h"
#include "lwip/opt.h"
#include <lwip/apps/httpd.h>

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_mqttd(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i;
    I32_T mqttd_en = -1;
    UI8_T enable;
    BOOL_T get_val = FALSE;
    UI8_T idx = 0, cookieLen = 0;

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "mqttd_en"))
        {
            mqttd_en = atoi(pcValue[i]);
            if ((mqttd_en != FALSE) && (mqttd_en != TRUE))
            {
                break;
            }
            enable = (UI8_T)(mqttd_en & 0xFF);
            get_val = TRUE;
        }
        if(TRUE == mqttd_en)
        {
            if(!strcmp(pcParam[i], "Cookies"))
            {
#if LWIP_HTTPD
                for(idx = 0; idx < HTTPD_MAX_LOGIN_NUM; idx ++)
                {
                    cookieLen = strlen(http_cookies[idx].cookie);
                    if(0 != cookieLen)
                    {
                        if(0 == strncmp(pcValue[i], http_cookies[idx].cookie, cookieLen))
                        {
                            memset(http_cookies[idx].cookie, 0, HTTPD_MAX_COOKIE_LEN);
                            http_cookies[idx].idleTime = 0;
                            http_cookies[idx].executable = HTTPD_BROWSER_START;
                            break;
                        }
                    }
                }
#endif
            }
        }
    }
    if (FALSE == get_val)
    {
        return MW_E_OK;
    }

    CGI_LOG_DEBUG(MQTTD, ("mqttd state = %d\n", mqttd_en));

    /* Update db */
    rc = httpd_queue_setData(M_UPDATE, MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(enable));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(MQTTD, ("Update DB mqttd_cfg_info failed(%d)\n", rc));
    }

    return rc;
}

//==========================================================================================
//  SSI handler
//==========================================================================================
char
ssi_get_mqttd_info_xmlHandle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags)
{
    return ssi_get_mqttd_info_Handle(length, pcb, apiflags);
}

char
ssi_get_mqttd_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI16_T total_len = 0;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI8_T *ptr_data = NULL;

    /* Request DB for jumbo frame info */
    rc = httpd_queue_getData(MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(MQTTD, ("get mqttd_cfg_info data success, ptr_msg =%p, size =%d\n", ptr_msg, size));
    }
    else
    {
        CGI_LOG_ERROR(MQTTD, ("get mqttd_cfg_info data failed \n"));
        return ERR_VAL;
    }

    err = send_format_response(&total_len, pcb, apiflags,
                                "<script>var mqttd_ds={enable:%d};</script>",
                                (*ptr_data));
    MW_FREE(ptr_msg);
    if (ERR_OK != err)
    {
        return err;
    }

    *length = total_len;

    return err;
}

