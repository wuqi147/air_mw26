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

#ifndef MW_CMD_UTIL_H
#define MW_CMD_UTIL_H

#include "mw_error.h"
#include "mw_types.h"
#include "mw_portbmp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_CMD_MAX_HEX_NUM     (16)
#define MW_CMD_MAX_BITMAP_NUM  (16)
#define MW_CMD_MAX_RANGE_NUM   (16)

#define MW_CMLIB_BITMAP_BIT_ADD(bitmap, bit) (((bitmap)[(bit)/32]) |= (1U << ((bit)%32)))
#define MW_CMLIB_BITMAP_BIT_DEL(bitmap, bit) (((bitmap)[(bit)/32]) &= ~(1U << ((bit)%32)))
#define MW_CMLIB_BITMAP_BIT_CHK(bitmap, bit) ((((bitmap)[(bit)/32] & (1U << ((bit)%32)))) != 0)

typedef UI32_T  MW_IPV4_T;
typedef UI8_T   MW_IPV6_T[16];
/* MACRO FUNCTION DECLARATIONS
 */

#define MW_CMD_OUTPUT(...)      osapi_xprintf(__VA_ARGS__)
#define MW_CMD_CHECK_LAST_TOKEN(__token__) do               \
    {                                                       \
        if (NULL != (__token__))                            \
        {                                                   \
            return (MW_CMD_E_SYNTAX_ERR);                   \
        }                                                   \
    } while(0)

#define MW_CMD_CHECK_OPT(__rc__, __token__, __shift__) do   \
    {                                                       \
        MW_ERROR_NO_T __ret__ = (__rc__);                   \
        if (MW_E_OK ==  (__ret__))                          \
        {                                                   \
            (__token__) += (__shift__);                     \
        }                                                   \
        else if (MW_CMD_E_KEY_MISMATCH !=  (__ret__) )      \
        {                                                   \
            return (MW_CMD_E_SYNTAX_ERR);                   \
        }                                                   \
    } while(0)

#define MW_CMD_CHECK_PARAM(__rc__, __token__, __shift__) do \
    {                                                       \
        MW_ERROR_NO_T __ret__ = (__rc__);                   \
        if (MW_E_OK ==  (__ret__))                          \
        {                                                   \
            (__token__) += (__shift__);                     \
        }                                                   \
        else                                                \
        {                                                   \
            return (MW_CMD_E_SYNTAX_ERR);                   \
        }                                                   \
    } while(0)

#define MW_CMD_CHECK_IP_ADDR(__rc1__, __rc2__, __param1__, __token__, __shift__) do \
    {                                                                               \
        MW_ERROR_NO_T __ret__ = (__rc1__);                                          \
        if (MW_E_OK ==  (__ret__))                                                  \
        {                                                                           \
            __param1__ = TRUE;                                                      \
            (__token__) += (__shift__);                                             \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            __ret__ = (__rc2__);                                                    \
            if (MW_E_OK ==  (__ret__))                                              \
            {                                                                       \
                __param1__ = FALSE;                                                 \
                (__token__) += (__shift__);                                         \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                return (MW_CMD_E_SYNTAX_ERR);                                       \
            }                                                                       \
        }                                                                           \
    } while(0)

#define MW_CMD_RANGE_FOREACH(__range__, __range_idx__, __idx__) \
    for ((__range_idx__) = 0;                                \
         (__range_idx__) < MW_CMD_MAX_RANGE_NUM;                \
         (__range_idx__)++)                                  \
    for ((__idx__) = (__range__).start[(__range_idx__)];     \
         (__idx__) <= (__range__).end[(__range_idx__)];      \
         (__idx__)++)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T              start[MW_CMD_MAX_RANGE_NUM];
    UI32_T              end[MW_CMD_MAX_RANGE_NUM];
    UI32_T              cnt;  /* ex: 12,14,16-47, cnt = 34 */
    BOOL_T              key_match;
} MW_CMD_RANGE_T;

/* EXPORTED SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------- public */
UI32_T
mw_cmd_calcPow(
    UI32_T  base,
    UI32_T  power);

/* -------------------------------------------------------------- print */
MW_ERROR_NO_T
mw_cmd_showMacAddr(
    const MW_MAC_T      *ptr_value);

MW_ERROR_NO_T
mw_cmd_showIpv4Addr(
    const MW_IPV4_T     *ptr_value);

MW_ERROR_NO_T
mw_cmd_showIpv6Addr(
    const MW_IPV6_T     *ptr_value);

MW_ERROR_NO_T
mw_cmd_showIpAddr(
    const MW_IP_ADDR_T  *ptr_value);

/* -------------------------------------------------------------- check key */
MW_ERROR_NO_T
mw_cmd_checkString(
    const C8_T          *ptr_token,
    const C8_T          *ptr_key);

/* -------------------------------------------------------------- check key and get value */
MW_ERROR_NO_T
mw_cmd_getUint(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    UI32_T              *ptr_value);

MW_ERROR_NO_T
mw_cmd_getHex(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    UI32_T              *ptr_value,
    UI32_T              size);

MW_ERROR_NO_T
mw_cmd_getHexInBytes(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    UI8_T               *ptr_value,
    UI32_T              size);

MW_ERROR_NO_T
mw_cmd_getBitmap(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    UI32_T              *ptr_value,
    UI32_T              size);

MW_ERROR_NO_T
mw_cmd_getPortBitmap(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    const UI32_T        unit,
    MW_PORT_BITMAP_T    *ptr_value);

MW_ERROR_NO_T
mw_cmd_getRangeList(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_CMD_RANGE_T      *ptr_value);

MW_ERROR_NO_T
mw_cmd_getMacAddr(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_MAC_T            *ptr_value);

MW_ERROR_NO_T
mw_cmd_getIpv4Addr(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_IPV4_T           *ptr_value);

MW_ERROR_NO_T
mw_cmd_getIpv4Netmask(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_IPV4_T           *ptr_value,
    MW_IPV4_T           *ptr_mask);

MW_ERROR_NO_T
mw_cmd_getIpv6Addr(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_IPV6_T           *ptr_value);

MW_ERROR_NO_T
mw_cmd_getIpv6Netmask(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    MW_IPV6_T           *ptr_value,
    MW_IPV6_T           *ptr_mask);

MW_ERROR_NO_T
mw_cmd_getString(
    const C8_T          *ptr_tokens[],
    const UI32_T        token_idx,
    const C8_T          *ptr_key,
    C8_T                *ptr_value);

/* -------------------------------------------------------------- string */
MW_ERROR_NO_T
mw_cmd_inverseWords(
    UI32_T              *ptr_value,
    UI32_T              size);

MW_ERROR_NO_T
mw_cmd_inverseBytes(
    UI8_T               *ptr_value,
    UI32_T              size);

MW_ERROR_NO_T
mw_cmd_transStrToWords(
    const C8_T          *ptr_str,
    UI32_T              *ptr_value,
    UI32_T              *ptr_size);

MW_ERROR_NO_T
mw_cmd_transStrToBytes(
    const C8_T          *ptr_str,
    UI8_T               *ptr_value,
    UI32_T              *ptr_size);

MW_ERROR_NO_T
mw_cmd_transStrToUpper(
    C8_T                *ptr_str);

MW_ERROR_NO_T
mw_cmd_transStrToLower(
    C8_T                *ptr_str);

MW_ERROR_NO_T
mw_cmd_transStrToIpv4Addr(
    const C8_T          *ptr_str,
    MW_IPV4_T           *ptr_addr);

MW_ERROR_NO_T
mw_cmd_transStrToIpv4Netmask(
    const C8_T          *ptr_str,
    MW_IPV4_T           *ptr_addr,
    MW_IPV4_T           *ptr_mask);

MW_ERROR_NO_T
mw_cmd_transStrToIpv6Addr(
    const C8_T          *ptr_str,
    MW_IPV6_T           *ptr_addr);

MW_ERROR_NO_T
mw_cmd_transStrToIpv6Netmask(
    const C8_T          *ptr_str,
    MW_IPV6_T           *ptr_addr,
    MW_IPV6_T           *ptr_mask);

MW_ERROR_NO_T
mw_cmd_splitString(
    C8_T                *ptr_tokens[],
    C8_T                *ptr_str);

#endif /* end of MW_CMD_UTIL_H */
