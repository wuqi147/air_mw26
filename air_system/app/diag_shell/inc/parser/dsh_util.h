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

#ifndef DSH_UTIL_H
#define DSH_UTIL_H

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
#include <hal/common/hal.h>

/* -------------------------------------------------------------- check macro */
#define DSH_CHECK_LAST_TOKEN(__token__) \
    do                                  \
    {                                   \
        if (NULL != (__token__))        \
        {                               \
            return (DSH_E_SYNTAX_ERR);  \
        }                               \
    } while (0)

#define DSH_CHECK_OPT(__rc__, __token__, __shift__) \
    do                                              \
    {                                               \
        AIR_ERROR_NO_T __ret__ = (__rc__);          \
        if (AIR_E_OK == (__ret__))                  \
        {                                           \
            (__token__) += (__shift__);             \
        }                                           \
        else if (DSH_E_KEY_MISMATCH != (__ret__))   \
        {                                           \
            return (DSH_E_SYNTAX_ERR);              \
        }                                           \
    } while (0)

#define DSH_CHECK_PARAM(__rc__, __token__, __shift__) \
    do                                                \
    {                                                 \
        AIR_ERROR_NO_T __ret__ = (__rc__);            \
        if (AIR_E_OK == (__ret__))                    \
        {                                             \
            (__token__) += (__shift__);               \
        }                                             \
        else                                          \
        {                                             \
            return (DSH_E_SYNTAX_ERR);                \
        }                                             \
    } while (0)

#define DSH_CHECK_IP_ADDR(__rc1__, __rc2__, __param1__, __token__, __shift__) \
    do                                                                        \
    {                                                                         \
        AIR_ERROR_NO_T __ret__ = (__rc1__);                                   \
        if (AIR_E_OK == (__ret__))                                            \
        {                                                                     \
            __param1__ = TRUE;                                                \
            (__token__) += (__shift__);                                       \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            __ret__ = (__rc2__);                                              \
            if (AIR_E_OK == (__ret__))                                        \
            {                                                                 \
                __param1__ = FALSE;                                           \
                (__token__) += (__shift__);                                   \
            }                                                                 \
            else                                                              \
            {                                                                 \
                return (DSH_E_SYNTAX_ERR);                                    \
            }                                                                 \
        }                                                                     \
    } while (0)

#define DSH_CHECK_GET_UNIT(__ptr_tokens__, __token_idx__, __ptr_unit__, __shift_ori__)  \
    do                                                                                  \
    {                                                                                   \
        DSH_CHECK_OPT(dsh_getUint(__ptr_tokens__, __token_idx__, "unit", __ptr_unit__), \
                      (__ptr_tokens__), (__shift_ori__));                               \
        if (!HAL_IS_UNIT_VALID(*(__ptr_unit__)))                                        \
        {                                                                               \
            osal_printf("***Error***, unit %d is invalid.\n", *(__ptr_unit__));         \
            return (AIR_E_BAD_PARAMETER);                                               \
        }                                                                               \
    } while (0)

/* -------------------------------------------------------------- <HEX> or <Uintlist> */
#define DSH_MAX_HEX_NUM    (16)
#define DSH_MAX_BITMAP_NUM (16)
#define DSH_MAX_RANGE_NUM  (16)

typedef struct
{
    UI32_T start[DSH_MAX_RANGE_NUM];
    UI32_T end[DSH_MAX_RANGE_NUM];
    UI32_T cnt; /* ex: 12,14,16-47, cnt = 34 */
    BOOL_T key_match;
} DSH_RANGE_T;

#define DSH_RANGE_FOREACH(__range__, __range_idx__, __idx__) \
    for ((__range_idx__) = 0;                                \
         (__range_idx__) < DSH_MAX_RANGE_NUM;                \
         (__range_idx__)++)                                  \
        for ((__idx__) = (__range__).start[(__range_idx__)]; \
             (__idx__) <= (__range__).end[(__range_idx__)];  \
             (__idx__)++)

/* -------------------------------------------------------------- public */
UI32_T
dsh_calcPow(
    UI32_T base,
    UI32_T power);

/* -------------------------------------------------------------- print */
AIR_ERROR_NO_T
dsh_showMacAddr(
    const AIR_MAC_T *ptr_value);

AIR_ERROR_NO_T
dsh_showIpv4Addr(
    const AIR_IPV4_T *ptr_value);

AIR_ERROR_NO_T
dsh_showIpv6Addr(
    const AIR_IPV6_T *ptr_value);

AIR_ERROR_NO_T
dsh_showIpAddr(
    const AIR_IP_ADDR_T *ptr_value);

/* -------------------------------------------------------------- check key */
AIR_ERROR_NO_T
dsh_checkString(
    const C8_T *ptr_token,
    const C8_T *ptr_key);

/* -------------------------------------------------------------- check key and get value */
AIR_ERROR_NO_T
dsh_getUint(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value);

AIR_ERROR_NO_T
dsh_getHex(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value,
    UI32_T       size);

AIR_ERROR_NO_T
dsh_getHexInBytes(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI8_T       *ptr_value,
    UI32_T       size);

AIR_ERROR_NO_T
dsh_getBitmap(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value,
    UI32_T       size);

AIR_ERROR_NO_T
dsh_getPortBitmap(
    const C8_T        *ptr_tokens[],
    const UI32_T       token_idx,
    const C8_T        *ptr_key,
    const UI32_T       unit,
    AIR_PORT_BITMAP_T *ptr_value);

AIR_ERROR_NO_T
dsh_getRangeList(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    DSH_RANGE_T *ptr_value);

AIR_ERROR_NO_T
dsh_getMacAddr(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_MAC_T   *ptr_value);

AIR_ERROR_NO_T
dsh_getIpv4Addr(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV4_T  *ptr_value);

AIR_ERROR_NO_T
dsh_getIpv4Netmask(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV4_T  *ptr_value,
    AIR_IPV4_T  *ptr_mask);

AIR_ERROR_NO_T
dsh_getIpv6Addr(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV6_T  *ptr_value);

AIR_ERROR_NO_T
dsh_getIpv6Netmask(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV6_T  *ptr_value,
    AIR_IPV6_T  *ptr_mask);

AIR_ERROR_NO_T
dsh_getString(
    const C8_T  *ptr_tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    C8_T        *ptr_value);

/* -------------------------------------------------------------- string */
AIR_ERROR_NO_T
dsh_inverseWords(
    UI32_T *ptr_value,
    UI32_T  size);

AIR_ERROR_NO_T
dsh_inverseBytes(
    UI8_T *ptr_value,
    UI32_T size);

AIR_ERROR_NO_T
dsh_transStrToWords(
    const C8_T *ptr_str,
    UI32_T     *ptr_value,
    UI32_T     *ptr_size);

AIR_ERROR_NO_T
dsh_transStrToBytes(
    const C8_T *ptr_str,
    UI8_T      *ptr_value,
    UI32_T     *ptr_size);

AIR_ERROR_NO_T
dsh_transStrToUpper(
    C8_T *ptr_str);

AIR_ERROR_NO_T
dsh_transStrToLower(
    C8_T *ptr_str);

AIR_ERROR_NO_T
dsh_transStrToIpv4Addr(
    const C8_T *ptr_str,
    AIR_IPV4_T *ptr_addr);

AIR_ERROR_NO_T
dsh_transStrToIpv4Netmask(
    const C8_T *ptr_str,
    AIR_IPV4_T *ptr_addr,
    AIR_IPV4_T *ptr_mask);

AIR_ERROR_NO_T
dsh_transStrToIpv6Addr(
    const C8_T *ptr_str,
    AIR_IPV6_T *ptr_addr);

AIR_ERROR_NO_T
dsh_transStrToIpv6Netmask(
    const C8_T *ptr_str,
    AIR_IPV6_T *ptr_addr,
    AIR_IPV6_T *ptr_mask);

AIR_ERROR_NO_T
dsh_splitString(
    C8_T *ptr_tokens[],
    C8_T *ptr_str);

#endif /* end of DSH_UTIL_H */
