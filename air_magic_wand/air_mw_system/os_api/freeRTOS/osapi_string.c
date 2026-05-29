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


/* FILE NAME:  osapi_string.c
 * PURPOSE:
 * It provides string API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "mw_utils.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define LLONG_MAX_STR   "9223372036854775807"
#define LLONG_MIN_STR   "-9223372036854775808"

#ifndef LLONG_MAX
#define LLONG_MAX       9223372036854775807LL
#endif

#ifndef LLONG_MIN
#define LLONG_MIN       (-9223372036854775807LL - 1)
#endif

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

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: osapi_strtou32
 * PURPOSE:
 *      Function to convert a string to U32.
 *
 * INPUT:
 *      ptr_str             -- Input string
 *
 * OUTPUT:
 *      ptr_output_int      -- Output U32 integer
 *
 * RETURN:
 *      MW_E_OK             -- Convert to U32 integer successfully
 *      MW_E_BAD_PARAMETER  -- Conversion error
 *
 * NOTES:
 *      The function will return BAD_PARAMETER if
 *        1. no character is converted
 *        2. there are leftover characters after the number
 *        3. the converted integer is larger than LLONG_MAX or smaller than LLONG_MIN
 *        4. the converted integer is larger than UINT32_MAX
 */
MW_ERROR_NO_T
osapi_strtou32(
    const C8_T *ptr_str,
    UI32_T *ptr_output_int)
{
    C8_T        *ptr_end = NULL;
    long long   temp = 0;

    MW_CHECK_PTR(ptr_str);
    MW_CHECK_PTR(ptr_output_int);

    temp = strtoll(ptr_str, &ptr_end, 0);

    /* Check for conversion errors */
    if ((ptr_end == ptr_str) || ('\0' != *ptr_end))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Check for positive overflow */
    if ((LLONG_MAX == temp) && (strncmp(ptr_str, LLONG_MAX_STR, strlen(LLONG_MAX_STR)) != 0))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Check for negative overflow */
    if ((LLONG_MIN == temp) && (strncmp(ptr_str, LLONG_MIN_STR, strlen(LLONG_MIN_STR)) != 0))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Check for negative overflow */
    if (UINT32_MAX < temp)
    {
        return MW_E_BAD_PARAMETER;
    }

    *ptr_output_int = (UI32_T)temp;
    return MW_E_OK;
}

/* FUNCTION NAME: osapi_xsnprintf
 * PURPOSE:
 *      Thread-safe snprintf that appends to a buffer tracking current length.
 *
 * INPUT:
 *      buf                 -- Destination buffer
 *      buf_size            -- Total size of the buffer
 *      ptr_consumed_size   -- Pointer to the current consumed size (offset)
 *      fmt                 -- Format string
 *
 * OUTPUT:
 *      buf                 -- Appended string
 *      ptr_consumed_size   -- Updated consumed size
 *
 * RETURN:
 *      int                 -- The number of characters that would have been written
 *                             (excluding the terminating null byte).
 *                             Returns 0 if buffer is full.
 */
int osapi_xsnprintf(
    C8_T *ptr_buf,
    size_t buf_size,
    size_t *ptr_consumed_size,
    const C8_T *ptr_fmt,
    ...)
{
    int ret = 0;
    size_t cur_len = 0;
    va_list args;

    if ((NULL == ptr_buf) || (NULL == ptr_consumed_size) || (NULL == ptr_fmt))
    {
        return 0;
    }

    cur_len = *ptr_consumed_size;

    if (cur_len >= buf_size)
    {
        return 0;
    }

    va_start(args, ptr_fmt);
    ret = vsnprintf(ptr_buf + cur_len, buf_size - cur_len, ptr_fmt, args);
    va_end(args);

    if (ret > 0)
    {
        *ptr_consumed_size += ret;
    }

    return ret;
}

