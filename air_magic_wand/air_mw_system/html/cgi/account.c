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

/* FILE NAME:   account.c
 * PURPOSE:
 *      CGI and SSI function of user account web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#include "mbedtls/build_info.h"
#include "mbedtls/sha256.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static ACCOUNT_ERR_TYPE_T errtype = ACCOUNT_DEFAULT;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

int _cgi_set_account_hash(const unsigned char *input,
                   size_t ilen,
                   unsigned char *output,
                   int is224)
{
#ifndef MBEDTLS_SHA256_C
    return ERR_VAL;
#else
    return mbedtls_sha256(input, ilen, output, is224);
#endif
}

/* EXPORTED SUBPROGRAM BODIES
 */

MW_ERROR_NO_T cgi_set_handle_account(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    UI8_T i;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;
    account_info_t *ptr_account = NULL;

    if (MW_E_NO_MEMORY == osapi_calloc(sizeof(account_info_t), HTTPD_QUEUE_CLI, (void **)&ptr_account))
    {
        return MW_E_NO_MEMORY;
    }
    osapi_memset(ptr_account, 0, sizeof(account_info_t));

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "username"))
        {
            osapi_strncpy(ptr_account->username, pcValue[i], sizeof(ptr_account->username));
        }
        if(!strcmp(pcParam[i], "old_password"))
        {
            osapi_strncpy(ptr_account->old_pwd, pcValue[i], sizeof(ptr_account->old_pwd));
        }
        if(!strcmp(pcParam[i], "password"))
        {
            osapi_strncpy(ptr_account->new_pwd, pcValue[i], sizeof(ptr_account->new_pwd));
        }
    }

    if(0 != strlen(ptr_account->old_pwd))
    {
        if (MW_E_OK != _cgi_set_account_hash((const UI8_T *)ptr_account->old_pwd,
                            strlen(ptr_account->old_pwd),
                            ptr_account->hashPass,
                            FALSE))
        {
            MW_FREE(ptr_account);
            CGI_LOG_WARN(SYSTEM, "convert password failed");
            return ERR_VAL;
        }

        /* get db password*/
        rc = httpd_queue_getData(ACCOUNT_INFO, ACC_PASSWD, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(SYSTEM, "get password success, ptr_msg = %p", db_msg);
        }
        else
        {
            MW_FREE(ptr_account);
            CGI_LOG_WARN(SYSTEM, "get password failed");
            return ERR_VAL;
        }

        osapi_memcpy(ptr_account->passwd, db_data, db_size);
        osapi_free(db_msg);

        if (0 == memcmp(ptr_account->hashPass, ptr_account->passwd, MAX_PASSWORD_SIZE))
        {
            if(0 != strlen(ptr_account->username))
            {
                CGI_LOG_DEBUG(SYSTEM, "ptr_account->username = %s", ptr_account->username);
                rc = httpd_queue_setData(M_UPDATE, ACCOUNT_INFO, ACC_USERNAME, DB_ALL_ENTRIES, ptr_account->username, sizeof(ptr_account->username));
            }
            if(0 != strlen(ptr_account->new_pwd))
            {
                CGI_LOG_DEBUG(SYSTEM, "ptr_account->new_pwd = %s", ptr_account->new_pwd);
                osapi_memset(ptr_account->hashPass, 0, sizeof(ptr_account->hashPass));
                if (MW_E_OK != _cgi_set_account_hash((const UI8_T *)ptr_account->new_pwd,
                                    strlen(ptr_account->new_pwd),
                                    ptr_account->hashPass,
                                    FALSE))
                {
                    MW_FREE(ptr_account);
                    CGI_LOG_WARN(SYSTEM, "convert password failed");
                    return ERR_VAL;
                }
                rc = httpd_queue_setData(M_UPDATE, ACCOUNT_INFO, ACC_PASSWD, DB_ALL_ENTRIES, ptr_account->hashPass, sizeof(ptr_account->hashPass));
            }
            errtype = ACCOUNT_UPDATED;
        }
        else
        {
            /* password confirm failed */
            errtype = ACCOUNT_CFM_FAIL;
        }
    }
    else
    {
        errtype = ACCOUNT_DEFAULT;
    }

    MW_FREE(ptr_account);
    return rc;
}

char  ssi_get_username_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    UI16_T total_len = 0;
    C8_T err = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;
    C8_T username[MAX_USER_NAME_SIZE];

    /* get username*/
    rc = httpd_queue_getData(ACCOUNT_INFO, ACC_USERNAME, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(SYSTEM, "get username success, ptr_msg = %p", db_msg);
    }
    else
    {
        CGI_LOG_WARN(SYSTEM, "get username failed");
        return ERR_VAL;
    }

    osapi_memset(username, 0, sizeof(username));
    osapi_memcpy(username, db_data, db_size);
    osapi_free(db_msg);

    err = send_format_response(&total_len, pcb, apiflags, "<script>var user_ds ={userNameStr:'%s'};\nvar tip=%d;</script>", username, errtype);
    if(ERR_OK != err)
    {
        return err;
    }

    errtype = ACCOUNT_DEFAULT;
    *length = total_len;

    return ERR_OK;
}
