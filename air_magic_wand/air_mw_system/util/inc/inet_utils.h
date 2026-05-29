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

/* FILE NAME:  inet_utils.h
 * PURPOSE:
 *      Define inet address convert utils API in AIR Magic Wand system.
 *
 * NOTES:
 *
 */

#ifndef INET_UTILS_H
#define INET_UTILS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/ip4_addr.h"
#include "lwip/inet.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/ip6_addr.h"
#endif
#include "mw_utils.h"
#include "mw_types.h"

/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
 */

#define MW_IPV4_IS_MULTICAST(addr)                 (0xE0000000 == ((addr) & 0xF0000000))
#define MW_IPV6_IS_MULTICAST(addr)                 (0xFF == (((UI8_T *)(addr))[0]))
#define MW_L3_IP_IS_MULTICAST(ptr_ip)                       \
    ((TRUE == (ptr_ip)->ipv4)?                               \
        MW_IPV4_IS_MULTICAST((ptr_ip)->ip_addr.ipv4_addr) : \
        MW_IPV6_IS_MULTICAST((ptr_ip)->ip_addr.ipv6_addr))

#define IP4ADDR_STRLEN_MAX          16

#define IPv6_ADDR_LEN               16

#define MW_DHCP_CLIENT_PORT         68
#define MW_IPPROTO_IGMP             2
#define MW_IPPROTO_UDP              17
#define MW_IPPROTO_PIM              103
#define MW_IPV6PROTO_ICMPV6         58
#ifdef AIR_SUPPORT_IPV6
#define IPV6_ADDR_LEN   (128)
#define IPV6_CONFIG_MODE_AUTO                 (0)
#define IPV6_CONFIG_MODE_MANUAL               (1)
#define MW_IPV6_SLAAC_PREFIX_LEN              (64)
#endif

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


UI32_T getIpaddr(const char *cp);
char *inet_host_ntoa(const ip4_addr_t *ptr_addr);
#ifdef AIR_SUPPORT_IPV6
int
url_decode(
    const char *ptr_src,
    char *ptr_dst);
#endif

#endif  /* End of INET_UTILS_H */

