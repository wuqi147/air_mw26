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

/* FILE NAME:  osapi_string.h
 * PURPOSE:
 *      Wrapper APIs for freeRTOS string function call.
 *
 * NOTES:
 */

#ifndef OSAPI_STRING_H
#define OSAPI_STRING_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "mw_error.h"
#include <stddef.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

extern int xprintf(const char *fmt, ...);

#define osapi_printf(...)               printf(__VA_ARGS__)
#define osapi_xprintf(...)              xprintf(__VA_ARGS__)
#define osapi_vprintf(fmt, ap)          vprintf(fmt, ap)
#define osapi_xvprintf(fmt, ap)         xvprintf(fmt, ap)
#define osapi_console_printf(...)       uart_printf(__VA_ARGS__)
#define osapi_console_vprintf(fmt, ap)  uart_vprintf(fmt, ap)
#define osapi_sprintf(...)              sprintf(__VA_ARGS__)
#define osapi_snprintf(...)             snprintf(__VA_ARGS__)
#define osapi_strcpy(...)               strcpy(__VA_ARGS__)
#define osapi_strncpy(...)              strncpy(__VA_ARGS__)
#define osapi_strcmp(...)               strcmp(__VA_ARGS__)
#define osapi_strncmp(...)              strncmp(__VA_ARGS__)
#define osapi_strcat(...)               strcat(__VA_ARGS__)
#define osapi_strncat(...)              strncat(__VA_ARGS__)
#define osapi_strlen(...)               strlen(__VA_ARGS__)
#define osapi_strstr(...)               strstr(__VA_ARGS__)
#define osapi_strtok(...)               strtok(__VA_ARGS__)
#define osapi_strchr(...)               strchr(__VA_ARGS__)
#define osapi_memcpy(...)               memcpy(__VA_ARGS__)
#define osapi_memset(...)               memset(__VA_ARGS__)
#define osapi_memcmp(...)               memcmp(__VA_ARGS__)
#define osapi_memmove(...)              memmove(__VA_ARGS__)
#define osapi_srand(seed)               srand(seed)
#define osapi_rand()                    rand()

/* DATA TYPE DECLARATIONS
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
 *        2. there are leftover characters after the number.
 *        3. the converted integer is larger than LLONG_MAX or smaller than LLONG_MIN
 *        4. the converted integer is larger than UI32T_MAX
 */
MW_ERROR_NO_T
osapi_strtou32(
    const C8_T *ptr_str,
    UI32_T *ptr_output_int);

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
 *      int                 -- Number of characters written (or would be written)
 */
int osapi_xsnprintf(
    C8_T *ptr_buf,
    size_t buf_size,
    size_t *ptr_consumed_size,
    const C8_T *ptr_fmt,
    ...);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/

#endif  /* #ifndef OSAPI_STRING_H */

