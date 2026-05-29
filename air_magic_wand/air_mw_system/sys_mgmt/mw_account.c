/*******************************************************************************
 *  Copyright Statement:
 *
 *  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:   mw_account.c
 * PURPOSE:
 *      Provide API to verify account/password.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "default_config.h"
#include "mbedtls/sha256.h"
#include "mw_log.h"
#include "osapi_string.h"

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: mw_account_verifyUser
 * PURPOSE:
 *      Verify username
 *
 * INPUT:
 *      ptr_username    --  Pointer to input username string
 *      ptr_cfg         --  Pointer to username config string from DB
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_SUPPORT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Username length must not exceed MAX_USER_NAME_SIZE
 */
MW_ERROR_NO_T
mw_account_verifyUser(
    const C8_T *ptr_username,
    const C8_T *ptr_cfg)
{
    UI32_T len = 0, i = 0;

    if ((NULL == ptr_username) || (NULL == ptr_cfg))
    {
        return MW_E_BAD_PARAMETER;
    }

    len = osapi_strlen(ptr_username);
    if ((len > MAX_USER_NAME_SIZE) || (len != osapi_strlen(ptr_cfg)))
    {
        return MW_E_BAD_PARAMETER;
    }

    for (i = 0; i < len; ++i)
    {
        if (ptr_username[i] != ptr_cfg[i])
        {
            return MW_E_NOT_SUPPORT;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: mw_account_verifyPass
 * PURPOSE:
 *      Verify password, with optional hash
 *
 * INPUT:
 *      ptr_pass        --  Pointer to input password string or hash
 *      ptr_cfg         --  Pointer to password hash config from DB
 *      is_hashPass     --  TRUE if ptr_pass is already hashed, FALSE to hash before compare
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_SUPPORT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Uses constant-time comparison to prevent timing attacks
 *      Password length must not exceed MAX_PASSWORD_SIZE
 *      If MBEDTLS_SHA256_C is defined and is_hashPass is FALSE, SHA256 is used
 */
MW_ERROR_NO_T
mw_account_verifyPass(
    const C8_T *ptr_pass,
    const C8_T *ptr_cfg,
    const UI8_T is_hashPass)
{
#ifdef MBEDTLS_SHA256_C
    UI8_T hashPass[MAX_PASSWORD_HASH_SIZE] = {0};
#endif /* MBEDTLS_SHA256_C */
    UI32_T i = 0;
    UI8_T *ptr_hashPass = (UI8_T *)ptr_pass;
    UI8_T  diff = 0;

    if ((NULL == ptr_pass) || (NULL == ptr_cfg))
    {
        return MW_E_BAD_PARAMETER;
    }

#ifdef MBEDTLS_SHA256_C
    if (TRUE != is_hashPass)
    {
        UI32_T len = osapi_strlen(ptr_pass);

        if (len > MAX_PASSWORD_SIZE)
        {
            return MW_E_BAD_PARAMETER;
        }

        if (0 != mbedtls_sha256((UI8_T *)ptr_pass, len, hashPass, 0))
        {
            return MW_E_BAD_PARAMETER;
        }
        ptr_hashPass = hashPass;
    }
#endif /* MBEDTLS_SHA256_C */

    /* Constant-time comparison */
    for (i = 0; i < MAX_PASSWORD_HASH_SIZE; ++i)
    {
        diff |= ptr_hashPass[i] ^ ptr_cfg[i];
    }
    if (diff != 0)
    {
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}
