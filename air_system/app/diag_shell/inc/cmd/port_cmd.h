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

#ifndef PORT_CMD_H
#define PORT_CMD_H

#include <air_error.h>
#include <air_types.h>

#define CMD_PRINT_PORTLIST(__pbmp__)                                                                      \
    do                                                                                                    \
    {                                                                                                     \
        UI32_T cnt, port, first, span = 0;                                                                \
        CMLIB_PORT_BITMAP_COUNT((__pbmp__), cnt);                                                         \
        if (0 == cnt)                                                                                     \
        {                                                                                                 \
            osal_printf("null\n");                                                                        \
        }                                                                                                 \
        else                                                                                              \
        {                                                                                                 \
            first = 1;                                                                                    \
            CMLIB_BITMAP_BIT_FOREACH((__pbmp__), port, AIR_PORT_BITMAP_SIZE)                              \
            {                                                                                             \
                osal_printf("%s%d", first ? "" : ",", port);                                              \
                first = 0;                                                                                \
                for (span = 1; (++port < AIR_PORT_NUM) && CMLIB_BITMAP_BIT_CHK((__pbmp__), port); span++) \
                    ;                                                                                     \
                if (span > 1)                                                                             \
                {                                                                                         \
                    osal_printf("-%d", port - 1);                                                         \
                }                                                                                         \
            }                                                                                             \
        }                                                                                                 \
    } while (0)

#define CMD_STRING_PORTLIST(ptr_str, __pbmp__)                                                            \
    do                                                                                                    \
    {                                                                                                     \
        UI32_T cnt, port, first, span = 0;                                                                \
        C8_T   tmp_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];                                                      \
        osal_memset(ptr_str, 0, CMLIB_UTIL_IP_ADDR_STR_SIZE);                                             \
        CMLIB_PORT_BITMAP_COUNT((__pbmp__), cnt);                                                         \
        if (0 == cnt)                                                                                     \
        {                                                                                                 \
            osal_strcat(ptr_str, "null");                                                                 \
        }                                                                                                 \
        else                                                                                              \
        {                                                                                                 \
            first = 1;                                                                                    \
            CMLIB_BITMAP_BIT_FOREACH((__pbmp__), port, AIR_PORT_BITMAP_SIZE)                              \
            {                                                                                             \
                osal_memset(tmp_str, 0, CMLIB_UTIL_IP_ADDR_STR_SIZE);                                     \
                osal_snprintf(tmp_str, CMLIB_UTIL_IP_ADDR_STR_SIZE, "%s%d", first ? "" : ",", port);      \
                osal_strcat(ptr_str, tmp_str);                                                            \
                first = 0;                                                                                \
                for (span = 1; (++port < AIR_PORT_NUM) && CMLIB_BITMAP_BIT_CHK((__pbmp__), port); span++) \
                    ;                                                                                     \
                if (span > 1)                                                                             \
                {                                                                                         \
                    osal_memset(tmp_str, 0, CMLIB_UTIL_IP_ADDR_STR_SIZE);                                 \
                    osal_snprintf(tmp_str, CMLIB_UTIL_IP_ADDR_STR_SIZE, "-%d", port - 1);                 \
                    osal_strcat(ptr_str, tmp_str);                                                        \
                }                                                                                         \
            }                                                                                             \
        }                                                                                                 \
    } while (0)

#define CMD_STRING_SIZE_PORTLIST(ptr_str, __size__, __pbmp__)                                             \
    do                                                                                                    \
    {                                                                                                     \
        UI32_T cnt, port, first, span = 0;                                                                \
        C8_T   tmp_str[__size__];                                                                         \
        osal_memset(ptr_str, 0, __size__);                                                                \
        CMLIB_PORT_BITMAP_COUNT((__pbmp__), cnt);                                                         \
        if (0 == cnt)                                                                                     \
        {                                                                                                 \
            osal_strcat(ptr_str, "null");                                                                 \
        }                                                                                                 \
        else                                                                                              \
        {                                                                                                 \
            first = 1;                                                                                    \
            CMLIB_BITMAP_BIT_FOREACH((__pbmp__), port, AIR_PORT_BITMAP_SIZE)                              \
            {                                                                                             \
                osal_memset(tmp_str, 0, __size__);                                                        \
                osal_snprintf(tmp_str, __size__, "%s%d", first ? "" : ",", port);                         \
                osal_strcat(ptr_str, tmp_str);                                                            \
                first = 0;                                                                                \
                for (span = 1; (++port < AIR_PORT_NUM) && CMLIB_BITMAP_BIT_CHK((__pbmp__), port); span++) \
                    ;                                                                                     \
                if (span > 1)                                                                             \
                {                                                                                         \
                    osal_memset(tmp_str, 0, __size__);                                                    \
                    osal_snprintf(tmp_str, __size__, "-%d", port - 1);                                    \
                    osal_strcat(ptr_str, tmp_str);                                                        \
                }                                                                                         \
            }                                                                                             \
        }                                                                                                 \
    } while (0)

AIR_ERROR_NO_T
port_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx);

AIR_ERROR_NO_T
port_cmd_usager(void);

#endif
