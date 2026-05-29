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

/* FILE NAME:  cmlib_util.h
 * PURPOSE:
 *   1. provide common macro for other modules.
 * NOTES:
 *
 *
 *
 */
#ifndef CMLIB_UTIL_H
#define CMLIB_UTIL_H
/* INCLUDE FILE DECLARATIONS
 */
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define CMLIB_UTIL_MAC_STR_SIZE (15)
#define CMLIB_UTIL_MAC_TO_STR(__buf__, __mac__)                                                       \
    osal_snprintf(__buf__, CMLIB_UTIL_MAC_STR_SIZE, "%02X%02X-%02X%02X-%02X%02X",                     \
                  (__mac__)[0], (__mac__)[1], (__mac__)[2], (__mac__)[3], (__mac__)[4], (__mac__)[5])

#define CMLIB_UTIL_IPV4_STR_SIZE (16)
#define CMLIB_UTIL_IPV4_TO_STR(__buf__, __ipv4__)                                   \
    osal_snprintf(__buf__, CMLIB_UTIL_IPV4_STR_SIZE, "%d.%d.%d.%d",                 \
                  ((__ipv4__) & 0xFF000000) >> 24, ((__ipv4__) & 0x00FF0000) >> 16, \
                  ((__ipv4__) & 0x0000FF00) >> 8, ((__ipv4__) & 0x000000FF))

#define CMLIB_UTIL_IPV6_STR_SIZE (40)
#define CMLIB_UTIL_IPV6_TO_STR(__buf__, __ipv6__)                                            \
    osal_snprintf(__buf__, CMLIB_UTIL_IPV6_STR_SIZE,                                         \
                  "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X", \
                  (__ipv6__)[0], (__ipv6__)[1], (__ipv6__)[2], (__ipv6__)[3],                \
                  (__ipv6__)[4], (__ipv6__)[5], (__ipv6__)[6], (__ipv6__)[7],                \
                  (__ipv6__)[8], (__ipv6__)[9], (__ipv6__)[10], (__ipv6__)[11],              \
                  (__ipv6__)[12], (__ipv6__)[13], (__ipv6__)[14], (__ipv6__)[15])

#define CMLIB_UTIL_IP_ADDR_STR_SIZE    (CMLIB_UTIL_IPV6_STR_SIZE)
#define CMLIB_UTIL_IP_NETWORK_STR_SIZE (CMLIB_UTIL_IPV6_STR_SIZE)

#define CMLIB_UTIL_IP_ADDR_IS_ZERO(__ip__)                                         \
    (__ip__.ipv4 == TRUE) ?                                                        \
        (__ip__.ip_addr.ipv4_addr == AIR_IPV4_ZERO) :                              \
        (__ip__.ip_addr.ipv6_addr[0] == 0 && __ip__.ip_addr.ipv6_addr[1] == 0 &&   \
         __ip__.ip_addr.ipv6_addr[2] == 0 && __ip__.ip_addr.ipv6_addr[3] == 0 &&   \
         __ip__.ip_addr.ipv6_addr[4] == 0 && __ip__.ip_addr.ipv6_addr[5] == 0 &&   \
         __ip__.ip_addr.ipv6_addr[6] == 0 && __ip__.ip_addr.ipv6_addr[7] == 0 &&   \
         __ip__.ip_addr.ipv6_addr[8] == 0 && __ip__.ip_addr.ipv6_addr[9] == 0 &&   \
         __ip__.ip_addr.ipv6_addr[10] == 0 && __ip__.ip_addr.ipv6_addr[11] == 0 && \
         __ip__.ip_addr.ipv6_addr[12] == 0 && __ip__.ip_addr.ipv6_addr[13] == 0 && \
         __ip__.ip_addr.ipv6_addr[14] == 0 && __ip__.ip_addr.ipv6_addr[15] == 0)

#define CMLIB_UTIL_AIR_MAC_TO_HAL_MAC(__air_mac__, __hal_mac__) \
    do                                                          \
    {                                                           \
        (__hal_mac__)[0] = ((UI32_T)((__air_mac__)[2]) << 24 |  \
                            (__air_mac__)[3] << 16 |            \
                            (__air_mac__)[4] << 8 |             \
                            (__air_mac__)[5]);                  \
        (__hal_mac__)[1] = ((__air_mac__)[0] << 8 |             \
                            (__air_mac__)[1]);                  \
    } while (0)

#define CMLIB_UTIL_HAL_MAC_TO_AIR_MAC(__hal_mac__, __air_mac__)   \
    do                                                            \
    {                                                             \
        (__air_mac__)[0] = ((__hal_mac__)[1] & 0x0000FF00) >> 8;  \
        (__air_mac__)[1] = ((__hal_mac__)[1] & 0x000000FF);       \
        (__air_mac__)[2] = ((__hal_mac__)[0] & 0xFF000000) >> 24; \
        (__air_mac__)[3] = ((__hal_mac__)[0] & 0x00FF0000) >> 16; \
        (__air_mac__)[4] = ((__hal_mac__)[0] & 0x0000FF00) >> 8;  \
        (__air_mac__)[5] = ((__hal_mac__)[0] & 0x000000FF);       \
    } while (0)

#define CMLIB_UTIL_AIR_IPV6_TO_HAL_IPV6(__air_ipv6__, __hal_ipv6__) \
    do                                                              \
    {                                                               \
        (__hal_ipv6__)[0] = (UI32_T)((__air_ipv6__)[12]) << 24 |    \
                            (__air_ipv6__)[13] << 16 |              \
                            (__air_ipv6__)[14] << 8 |               \
                            (__air_ipv6__)[15];                     \
        (__hal_ipv6__)[1] = (UI32_T)((__air_ipv6__)[8]) << 24 |     \
                            (__air_ipv6__)[9] << 16 |               \
                            (__air_ipv6__)[10] << 8 |               \
                            (__air_ipv6__)[11];                     \
        (__hal_ipv6__)[2] = (UI32_T)((__air_ipv6__)[4]) << 24 |     \
                            (__air_ipv6__)[5] << 16 |               \
                            (__air_ipv6__)[6] << 8 |                \
                            (__air_ipv6__)[7];                      \
        (__hal_ipv6__)[3] = (UI32_T)((__air_ipv6__)[0]) << 24 |     \
                            (__air_ipv6__)[1] << 16 |               \
                            (__air_ipv6__)[2] << 8 |                \
                            (__air_ipv6__)[3];                      \
    } while (0)

#define CMLIB_UTIL_HAL_IPV6_TO_AIR_IPV6(__hal_ipv6__, __air_ipv6__)  \
    do                                                               \
    {                                                                \
        (__air_ipv6__)[0] = ((__hal_ipv6__)[3] & 0xFF000000) >> 24;  \
        (__air_ipv6__)[1] = ((__hal_ipv6__)[3] & 0x00FF0000) >> 16;  \
        (__air_ipv6__)[2] = ((__hal_ipv6__)[3] & 0x0000FF00) >> 8;   \
        (__air_ipv6__)[3] = ((__hal_ipv6__)[3] & 0x000000FF);        \
        (__air_ipv6__)[4] = ((__hal_ipv6__)[2] & 0xFF000000) >> 24;  \
        (__air_ipv6__)[5] = ((__hal_ipv6__)[2] & 0x00FF0000) >> 16;  \
        (__air_ipv6__)[6] = ((__hal_ipv6__)[2] & 0x0000FF00) >> 8;   \
        (__air_ipv6__)[7] = ((__hal_ipv6__)[2] & 0x000000FF);        \
        (__air_ipv6__)[8] = ((__hal_ipv6__)[1] & 0xFF000000) >> 24;  \
        (__air_ipv6__)[9] = ((__hal_ipv6__)[1] & 0x00FF0000) >> 16;  \
        (__air_ipv6__)[10] = ((__hal_ipv6__)[1] & 0x0000FF00) >> 8;  \
        (__air_ipv6__)[11] = ((__hal_ipv6__)[1] & 0x000000FF);       \
        (__air_ipv6__)[12] = ((__hal_ipv6__)[0] & 0xFF000000) >> 24; \
        (__air_ipv6__)[13] = ((__hal_ipv6__)[0] & 0x00FF0000) >> 16; \
        (__air_ipv6__)[14] = ((__hal_ipv6__)[0] & 0x0000FF00) >> 8;  \
        (__air_ipv6__)[15] = ((__hal_ipv6__)[0] & 0x000000FF);       \
    } while (0)

#define CMLIB_UTIL_CEIL(a, b) ((a / b) + ((a % b) != 0))

/* DATA TYPE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void
cmlib_util_getIpAddrStr(
    const AIR_IP_ADDR_T *ptr_ip_addr,
    C8_T                *ptr_str);

/* FUNCTION NAME:   cmlib_util_popcount
 * PURPOSE:
 *      cmlib_util_popcount() is a function to count number of one in a word.
 *
 * INPUT:
 *      count      -- The specified word.
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T   -- Return number of one in a word.
 *
 * NOTES:
 *      None
 *
 */
UI32_T
cmlib_util_popcount(
    UI32_T count);

/* FUNCTION NAME:   cmlib_util_findFirstZero
 * PURPOSE:
 *      cmlib_util_findFirstZero() is a function to get first zero bit from LSB.
 *
 * INPUT:
 *      ui32_map  -- The specified word.
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T   -- Return available bit in a word.
 *
 * NOTES:
 *
 */
UI32_T
cmlib_util_findFirstZero(
    UI32_T ui32_map);

/* FUNCTION NAME:   cmlib_util_getIpv4Str
 * PURPOSE:
 *      cmlib_util_getIpv4Str() is a function to convert ipv4 address to string
 *
 * INPUT:
 *      ptr_ipv4            --  IPv4 address
 *
 * OUTPUT:
 *      ptr_str             --  A pointer to string buffer
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
cmlib_util_getIpv4Str(
    const AIR_IPV4_T *ptr_ipv4,
    C8_T             *ptr_str);

/* FUNCTION NAME:   cmlib_util_getIpv6Str
 * PURPOSE:
 *      cmlib_util_getIpv6Str() is a function to convert ipv6 address to string
 *
 * INPUT:
 *      ptr_ipv6            --  IPv6 address
 *
 * OUTPUT:
 *      ptr_str             --  A pointer to string buffer
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
cmlib_util_getIpv6Str(
    const AIR_IPV6_T *ptr_ipv6,
    C8_T             *ptr_str);
#endif /* End of CMLIB_UTIL_H */
