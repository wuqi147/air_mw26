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

/* FILE NAME:   logon.c
 * PURPOSE:
 *      CGI and SSI function of logon web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "lwip/opt.h"
#include <lwip/apps/httpd.h>
#include "httpd_queue.h"
#include "mw_account.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
LOGON_ERR_TYPE_T logon_errtype = LOGON_CFM_OK;
const char *index_uri = "/index.html";
const char *index_redirect_uri = "/index_re.html";

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
static int hex_char_to_int(C8_T c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

static MW_ERROR_NO_T hexstr_to_bytes(const C8_T *hexstr, UI8_T *out)
{
    UI32_T len = strlen(hexstr), i, high, low;

    /* The length of SHA256 hex string is 64 characters */
    if (len != 64)
    {
        return MW_E_BAD_PARAMETER;
    }
    for (i = 0; i < len / 2; i++)
    {
        high = hex_char_to_int(hexstr[2 * i]);
        low  = hex_char_to_int(hexstr[2 * i + 1]);
        if (high < 0 || low < 0)
        {
            return MW_E_BAD_PARAMETER;
        }
        out[i] = (high << 4) | low;
    }
    return MW_E_OK;
}

const char* cgi_set_logon_info_handle(char* pcValue[])
{
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;
    UI8_T logon_fail_count = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    account_info_t *ptr_account = NULL;

    logon_errtype = LOGON_CFM_FAIL;

    if ((NULL == pcValue[1]) || ('\0' == pcValue[1][0]))
    {
        return index_uri;
    }

    if (MW_E_NO_MEMORY == osapi_calloc(sizeof(account_info_t), HTTPD_QUEUE_CLI, (void **)&ptr_account))
    {
        return index_uri;
    }
    osapi_memset(ptr_account, 0, sizeof(account_info_t));

    if (MW_E_OK != hexstr_to_bytes(pcValue[1], ptr_account->hashPass))
    {
        MW_FREE(ptr_account);
        return index_uri;
    }

    /* get db username*/
    rc = httpd_queue_getData(ACCOUNT_INFO, ACC_USERNAME, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "get username success, ptr_msg =%p", db_msg);
    }
    else
    {
        MW_FREE(ptr_account);
        CGI_LOG_WARN(SYSTEM, "get username failed");
        return index_uri;
    }

    osapi_memcpy(ptr_account->username, db_data, db_size);
    MW_FREE(db_msg);

    /* get db password*/
    rc = httpd_queue_getData(ACCOUNT_INFO, ACC_PASSWD, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "get password success, ptr_msg =%p", db_msg);
    }
    else
    {
        MW_FREE(ptr_account);
        CGI_LOG_WARN(SYSTEM, "get password failed");
        return index_uri;
    }

    osapi_memcpy(ptr_account->passwd, db_data, db_size);
    MW_FREE(db_msg);

    if ((MW_E_OK == mw_account_verifyUser(pcValue[0], ptr_account->username)) &&
        (MW_E_OK == mw_account_verifyPass((C8_T *)ptr_account->hashPass, ptr_account->passwd, TRUE)))
    {
        logon_errtype = LOGON_CFM_OK;
        logon_fail_count = 0;
        rc = httpd_queue_setData(M_UPDATE, LOGON_INFO, LOGON_FAIL_COUNT, DB_ALL_ENTRIES, &logon_fail_count, sizeof(logon_fail_count));
        if(MW_E_OK != rc)
        {
            CGI_LOG_DEBUG(SYSTEM, "set logon info failed");
        }
        MW_FREE(ptr_account);
        return index_redirect_uri;
    }
    else
    {
        /* get db logon fail count*/
        rc = httpd_queue_getData(LOGON_INFO, LOGON_FAIL_COUNT, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(SYSTEM, "get logon fail count success, ptr_msg =%p", db_msg);
        }
        else
        {
            MW_FREE(ptr_account);
            CGI_LOG_WARN(SYSTEM, "get logon fail count failed");
            return index_uri;
        }
        osapi_memcpy(&logon_fail_count, db_data, db_size);
        MW_FREE(db_msg);
        if(logon_fail_count < 255)
        {
            logon_fail_count ++;
        }
        else
        {
            logon_fail_count = 1;
        }
        rc = httpd_queue_setData(M_UPDATE, LOGON_INFO, LOGON_FAIL_COUNT, DB_ALL_ENTRIES, &logon_fail_count, sizeof(logon_fail_count));
        if(MW_E_OK != rc)
        {
            CGI_LOG_WARN(SYSTEM, "set logon info failed");
        }
        MW_FREE(ptr_account);
        return index_uri;
    }
}

char ssi_get_errtype_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    UI32_T total_len = 0;
    UI16_T len = 0;
    C8_T err = 0;

    CGI_LOG_DEBUG(SYSTEM, "errtype entered");

    err = send_format_response(&len, pcb, apiflags, "%d", logon_errtype);
    if(ERR_OK != err)
    {
        return err;
    }

    logon_errtype = LOGON_CFM_OK;

    total_len = len;
    *length = total_len;

    CGI_LOG_DEBUG(SYSTEM, "errtype leave");

    return ERR_OK;
}

MW_ERROR_NO_T
ssi_get_connection_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
#if LWIP_HTTPD
    char err = 0;

    if(HTTPD_CONNECTION_FULL == http_cookie_index || HTTPD_CONNECTION_TIMEOUT == http_cookie_index)
    {
        if(HTTPD_CONNECTION_FULL == http_cookie_index)
        {
            logon_errtype = LOGON_CFM_FULL;
        }
        err = send_format_response(&len, ptr_pcb, apiflags, "<script>var allow = 0;</script>");
    }
    else
    {
        err = send_format_response(&len, ptr_pcb, apiflags, "<script>var allow = 1;</script>");
    }

    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }
#endif

    *ptr_length = len;

    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_cookie_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
#if LWIP_HTTPD
    char err = 0;

    if(HTTPD_CONNECTION_FULL == http_cookie_index || HTTPD_CONNECTION_TIMEOUT == http_cookie_index)
    {
        err = send_format_response_no_chunk(&len, ptr_pcb, apiflags, "");
    }
    else
    {
        err = send_format_response_no_chunk(&len, ptr_pcb, apiflags, "Set-cookie: Cookies=%s; HttpOnly; SameSite=Lax;\r\n", http_cookies[http_cookie_index].cookie);
    }
    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }
#endif

    *ptr_length = len;

    return MW_E_OK;
}

MW_ERROR_NO_T
cgi_set_handle_logout(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
#if LWIP_HTTPD
    if (HTTPD_CONNECTION_FULL == http_cookie_index || HTTPD_CONNECTION_TIMEOUT == http_cookie_index)
    {
        return MW_E_OK;
    }
    if (http_cookie_index < HTTPD_MAX_LOGIN_NUM)
    {
        memset(http_cookies[http_cookie_index].cookie, 0, HTTPD_MAX_COOKIE_LEN);
        http_cookies[http_cookie_index].idleTime = 0;
        http_cookies[http_cookie_index].executable = HTTPD_BROWSER_START;
    }
#endif

    return MW_E_OK;
}

MW_ERROR_NO_T cgi_set_handle_logon(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    CGI_LOG_DEBUG(SYSTEM, "web logon");
    return 0;
}
