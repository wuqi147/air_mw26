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

/* FILE NAME:  inet_utils.c
 * PURPOSE:
 *      Define inet address convert utils API in AIR Magic Wand system.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "inet_utils.h"

/* NAMING CONSTANT DECLARATIONS
*/

/** 255.255.255.255 */
#ifndef IPADDR_NONE
#define IPADDR_NONE         ((UI32_T)0xffffffffUL)
#endif
/** 127.0.0.1 */
#ifndef IPADDR_LOOPBACK
#define IPADDR_LOOPBACK     ((UI32_T)0x7f000001UL)
#endif
/** 0.0.0.0 */
#ifndef IPADDR_ANY
#define IPADDR_ANY          ((UI32_T)0x00000000UL)
#endif
/** 255.255.255.255 */
#ifndef IPADDR_BROADCAST
#define IPADDR_BROADCAST    ((UI32_T)0xffffffffUL)
#endif

#define _in_range(c, lo, up)    ((UI8_T)(c) >= (lo) && (UI8_T)(c) <= (up))
#define _isdigit(c)             _in_range((c), '0', '9')

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


UI32_T
getIpaddr(const char *cp)
{
  ip4_addr_t val;

  if (inet_aton(cp, &val)) {
    return val.addr;
  }
  return (IPADDR_NONE);
}

/* For ipv4 address defined type is ip4_addr_t */
char *inet_host_ntoa(const ip4_addr_t *ptr_addr)
{
    static char str[IP4ADDR_STRLEN_MAX];
    ip4_addr_t addr_inNetworkOrder;

    if (NULL == ptr_addr)
    {
        return NULL;
    }

    memset(&addr_inNetworkOrder, 0, sizeof(ip4_addr_t));
    addr_inNetworkOrder.addr = htonl(ptr_addr->addr);
    return inet_ntoa_r(addr_inNetworkOrder, str, IP4ADDR_STRLEN_MAX);
}

#ifdef AIR_SUPPORT_IPV6
/* FUNCTION NAME:   url_decode
 * PURPOSE:
 *      Convert IPv6 address format from lowercase to uppercase.
 *
 * INPUT:
 *      ptr_src                  -- Address to be converted
 *      ptr_dst                  -- Adress after convert
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
int
url_decode(
  const char *ptr_src,
  char *ptr_dst)
{
    char a, b;
    while (*ptr_src)
    {
        if (*ptr_src == '%' && isxdigit((unsigned char)ptr_src[1]) && isxdigit((unsigned char)ptr_src[2]))
        {
            a = ptr_src[1];
            b = ptr_src[2];

            if (a >= 'a')
            {
                a -= 'a' - 'A';
            }

            if (a >= 'A')
            {
                a = a - 'A' + 10;
            }
            else
            {
                a -= '0';
            }

            if (b >= 'a')
            {
                b -= 'a' - 'A';
            }

            if (b >= 'A')
            {
                b = b - 'A' + 10;
            }
            else
            {
                b -= '0';
            }

            *ptr_dst++ = a * 16 + b;
            ptr_src += 3;
        }
        else if (*ptr_src == '+')
        {
            *ptr_dst++ = ' ';
            ptr_src++;
        }
        else
        {
            *ptr_dst++ = *ptr_src++;
        }
    }
    *ptr_dst = '\0';
    return 0;
}
#endif
