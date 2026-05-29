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

#include <parser/dsh_util.h>

#include <air_error.h>
#include <air_init.h>
#include <air_port.h>
#include <air_types.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_util.h>
#include <hal/common/hal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>

#define DSH_CH_PER_WORDS (8)
#define DSH_CH_PER_BYTES (2)
#define DSH_START_DEC    (0 - 1)
#define DSH_START_HEX    (2 - 1)

#define DSH_CHECK_KEY_GET_VAL(__tok__, __idx__, __key__, __val__, __len__) \
    do                                                                     \
    {                                                                      \
        /* check key */                                                    \
        if (AIR_E_OK != dsh_checkString(__tok__[__idx__], __key__))        \
        {                                                                  \
            return (DSH_E_KEY_MISMATCH);                                   \
        }                                                                  \
                                                                           \
        /* get value */                                                    \
        (__val__) = (C8_T *)(__tok__[__idx__ + 1]);                        \
        if (NULL == (__val__))                                             \
        {                                                                  \
            return (DSH_E_KEY_MISMATCH);                                   \
        }                                                                  \
        (__len__) = osal_strlen(__val__);                                  \
        if (0 == (__len__))                                                \
        {                                                                  \
            return (DSH_E_KEY_MISMATCH);                                   \
        }                                                                  \
    } while (0)

#define DSH_CHECK_PTR(__ptr__)           \
    do                                   \
    {                                    \
        if (NULL == (__ptr__))           \
        {                                \
            return (DSH_E_KEY_MISMATCH); \
        }                                \
    } while (0)

#define DSH_CHECK_PTR_GET_LEN(__ptr__, __len__) \
    do                                          \
    {                                           \
        if (NULL == (__ptr__))                  \
        {                                       \
            return (DSH_E_KEY_MISMATCH);        \
        }                                       \
        (__len__) = osal_strlen(__ptr__);       \
        if (0 == (__len__))                     \
        {                                       \
            return (DSH_E_KEY_MISMATCH);        \
        }                                       \
    } while (0)

#define DSH_CHECK_COND(__shift__, __op__, __size__) \
    do                                              \
    {                                               \
        if ((__shift__)__op__(__size__))            \
        {                                           \
            ;                                       \
        }                                           \
        else                                        \
        {                                           \
            return (DSH_E_SYNTAX_ERR);              \
        }                                           \
    } while (0)

#define DSH_CHECK_HEX(__ptr__)                                                                          \
    ('0' != (__ptr__)[0]) ? FALSE : ('x' == (__ptr__)[1]) ? TRUE : ('X' == (__ptr__)[1]) ? TRUE : FALSE

/* -------------------------------------------------------------- public */

UI32_T
dsh_calcPow(
    UI32_T base,
    UI32_T power)
{
    return (0 == power) ? 1 : (base * dsh_calcPow(base, power - 1));
}

/* -------------------------------------------------------------- print */

AIR_ERROR_NO_T
dsh_showMacAddr(
    const AIR_MAC_T *ptr_value)
{
    UI32_T idx = 0, last = 6;

    DSH_CHECK_PTR(ptr_value);

    for (idx = 0; idx < last; idx++)
    {
        osal_printf("%02x", (*ptr_value)[idx]);
        if (idx != last - 1)
        {
            osal_printf(":");
        }
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_showIpv4Addr(
    const AIR_IPV4_T *ptr_value)
{
    C8_T str[CMLIB_UTIL_IPV4_STR_SIZE];

    DSH_CHECK_PTR(ptr_value);

    osal_memset(str, 0x0, sizeof(str));
    cmlib_util_getIpv4Str(ptr_value, str);
    osal_printf("%s", str);

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_showIpv6Addr(
    const AIR_IPV6_T *ptr_value)
{
    C8_T str[CMLIB_UTIL_IPV6_STR_SIZE];

    DSH_CHECK_PTR(ptr_value);

    osal_memset(str, 0x0, sizeof(str));
    cmlib_util_getIpv6Str(ptr_value, str);
    osal_printf("%s", str);

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_showIpAddr(
    const AIR_IP_ADDR_T *ptr_value)
{
    DSH_CHECK_PTR(ptr_value);

    if (TRUE == ptr_value->ipv4)
    {
        dsh_showIpv4Addr(&(ptr_value->ip_addr.ipv4_addr));
    }
    else
    {
        dsh_showIpv6Addr(&(ptr_value->ip_addr.ipv6_addr));
    }

    return (AIR_E_OK);
}
/* FUNCTION NAME:   dsh_checkString
 * PURPOSE:
 *      Change input string to lowercase and compared with key word.
 *
 * INPUT:
 *      ptr_token           - String
 *      ptr_key             - Key word
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      AIR_E_OK
 *      DSH_E_KEY_MISMATCH
 *
 * NOTES:
 *      The key word must be lowercase
 */
AIR_ERROR_NO_T
dsh_checkString(
    const C8_T *ptr_token,
    const C8_T *ptr_key)
{
    UI32_T token_len = 0;
    UI32_T key_len = 0;

    DSH_CHECK_PTR_GET_LEN(ptr_token, token_len);
    DSH_CHECK_PTR_GET_LEN(ptr_key, key_len);

    dsh_transStrToLower((C8_T *)ptr_token);

    if ((token_len != key_len) || (0 != osal_strncmp(ptr_token, ptr_key, key_len)))
    {
        return (DSH_E_KEY_MISMATCH); /* Error: key mismatch */
    }

    return (AIR_E_OK);
}

/* -------------------------------------------------------------- check key and get value */
AIR_ERROR_NO_T
dsh_getUint(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T idx = 0, ch = 0;
    UI32_T value = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset(ptr_value, 0, sizeof(UI32_T));

    if (TRUE == DSH_CHECK_HEX(ptr_token_val))
    {
        /* e.g. 0x400, token_val_len = 5 */
        for (idx = token_val_len - 1; idx != DSH_START_HEX; idx--)
        {
            /* 0: value = 0
             * 1: value += 0U << (4 * 0)
             * 2: value += 0U << (4 * 1)
             * 3: value += 4U << (4 * 2)
             */
            if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - '0')) << (4 * ch);
                ch++;
            }
            else if (('a' <= ptr_token_val[idx]) && ('f' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'a' + 10)) << (4 * ch);
                ch++;
            }
            else if (('A' <= ptr_token_val[idx]) && ('F' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'A' + 10)) << (4 * ch);
                ch++;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        *ptr_value = value;
    }
    else
    {
        /* e.g. 1024, token_val_len = 4 */
        for (idx = token_val_len - 1; idx != DSH_START_DEC; idx--)
        {
            /* 0: value = 0
             * 1: value += 4 * (10 ^ 0)
             * 2: value += 2 * (10 ^ 1)
             * 3: value += 0 * (10 ^ 2)
             * 4: value += 1 * (10 ^ 3)
             */
            if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
            {
                value += (ptr_token_val[idx] - '0') * dsh_calcPow(10, ch);
                ch++;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        *ptr_value = value;
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getHex(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value,
    UI32_T       size)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T idx = 0, ch = 0, shift = 0;
    UI32_T value = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset(ptr_value, 0, size);

    if (TRUE == DSH_CHECK_HEX(ptr_token_val))
    {
        /* e.g. str = 0x1000023456789abcd,   str_len = 19
         *          = 0x1.00002345.6789abcd, str_len = 21
         *          = 0x1.2345.6789abcd,     str_len = 17
         *
         *   value[0] = 0x6789abcd
         *   value[1] = 0x2345
         *   value[2] = 0x1
         */
        for (idx = token_val_len - 1; idx != DSH_START_HEX; idx--)
        {
            if (('.' == ptr_token_val[idx]) || (8 == ch))
            {
                DSH_CHECK_COND(shift, <, size / sizeof(UI32_T)); /* Error: out-of-memory */
                ptr_value[shift++] = value;
                value = 0;
                ch = 0;
            }

            if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - '0')) << (4 * ch);
                ch++;
            }
            else if (('a' <= ptr_token_val[idx]) && ('f' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'a' + 10)) << (4 * ch);
                ch++;
            }
            else if (('A' <= ptr_token_val[idx]) && ('F' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'A' + 10)) << (4 * ch);
                ch++;
            }
            else if ('.' == ptr_token_val[idx])
            {
                continue;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        DSH_CHECK_COND(shift, <, size / sizeof(UI32_T)); /* Error: out-of-memory */
        ptr_value[shift++] = value;
    }
    else
    {
        return (DSH_E_SYNTAX_ERR); /* Error: not a heximal number */
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getHexInBytes(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI8_T       *ptr_value,
    UI32_T       size)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T idx = 0, ch = 0, shift = 0;
    UI8_T  value = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset(ptr_value, 0, sizeof(UI8_T) * size);

    if (TRUE == DSH_CHECK_HEX(ptr_token_val))
    {
        /* e.g. str = 0x10234,   str_len = 7
         *          = 0x1.02.34, str_len = 9
         *          = 0x1.2.34,  str_len = 8
         *
         *   value[0] = 0x34
         *   value[1] = 0x2
         *   value[2] = 0x1
         */
        for (idx = token_val_len - 1; idx != DSH_START_HEX; idx--)
        {
            if (('.' == ptr_token_val[idx]) || (2 == ch))
            {
                DSH_CHECK_COND(shift, <, size); /* Error: out-of-memory */
                ptr_value[shift++] = value;
                value = 0;
                ch = 0;
            }

            if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - '0')) << (4 * ch);
                ch++;
            }
            else if (('a' <= ptr_token_val[idx]) && ('f' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'a' + 10)) << (4 * ch);
                ch++;
            }
            else if (('A' <= ptr_token_val[idx]) && ('F' >= ptr_token_val[idx]))
            {
                value += ((UI32_T)(ptr_token_val[idx] - 'A' + 10)) << (4 * ch);
                ch++;
            }
            else if ('.' == ptr_token_val[idx])
            {
                continue;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        DSH_CHECK_COND(shift, <, size); /* Error: out-of-memory */
        ptr_value[shift++] = value;
    }
    else
    {
        return (DSH_E_SYNTAX_ERR); /* Error: not a heximal number */
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getBitmap(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    UI32_T      *ptr_value,
    UI32_T       size)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T idx = 0, value = 0, start = 0, end = 0;
    BOOL_T dash = FALSE, valid = FALSE;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset(ptr_value, 0, sizeof(UI32_T) * size);

    /* e.g. 12,14,16-47 , token_len = 11 */
    for (idx = 0; idx < token_val_len; idx++)
    {
        if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
        {
            value = (value * 10) + (ptr_token_val[idx] - '0');
            valid = TRUE;
        }
        else if (',' == ptr_token_val[idx])
        {
            DSH_CHECK_COND(valid, ==, TRUE); /* Error: value is invalid */

            /* 1: value = 12, dash = 0
             * 2: value = 14, dash = 0
             */
            if (FALSE == dash)
            {
                DSH_CHECK_COND(value, <, size * 32); /* Error: mem-overwrite */
                CMLIB_BITMAP_BIT_ADD(ptr_value, value);
            }
            /* 4: value = 47, start = 16, end = 47, dash = 0 */
            else
            {
                end = value;
                for (value = start; value <= end; value++)
                {
                    DSH_CHECK_COND(value, <, size * 32); /* Error: mem-overwrite */
                    CMLIB_BITMAP_BIT_ADD(ptr_value, value);
                }
            }
            start = 0;
            end = 0;
            value = 0;
            dash = FALSE;
            valid = FALSE;
        }
        /* 3: value = 16, start = 16, end = 0, dash = 1 */
        else if ('-' == ptr_token_val[idx])
        {
            DSH_CHECK_COND(valid, ==, TRUE); /* Error: value is invalid */
            DSH_CHECK_COND(dash, ==, FALSE); /* Error: double-dash */
            start = value;
            end = 0;
            value = 0;
            dash = TRUE;
            valid = FALSE;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* not a dec number */
        }
    }

    /* last data */
    DSH_CHECK_COND(valid, ==, TRUE); /* Error: value is invalid */

    if (FALSE == dash)
    {
        DSH_CHECK_COND(value, <, size * 32); /* Error: mem-overwrite */
        CMLIB_BITMAP_BIT_ADD(ptr_value, value);
    }
    else
    {
        end = value;
        for (value = start; value <= end; value++)
        {
            DSH_CHECK_COND(value, <, size * 32); /* Error: mem-overwrite */
            CMLIB_BITMAP_BIT_ADD(ptr_value, value);
        }
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getPortBitmap(
    const C8_T        *tokens[],
    const UI32_T       token_idx,
    const C8_T        *ptr_key,
    const UI32_T       unit,
    AIR_PORT_BITMAP_T *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset((*ptr_value), 0, sizeof(AIR_PORT_BITMAP_T));

    /* e.g. portlist=all, untag-portlist=all */
    if (AIR_E_OK == dsh_checkString(ptr_token_val, "all"))
    {
        osal_printf("\r\n current not support");
    }
    else if (AIR_E_OK == dsh_checkString(ptr_token_val, "none"))
    {
        /* do nothing */
        return AIR_E_OK;
    }
    else
    {
        return (dsh_getBitmap(tokens, token_idx, ptr_key, (UI32_T *)(*ptr_value), AIR_PORT_BITMAP_SIZE));
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getRangeList(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    DSH_RANGE_T *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T idx = 0, value = 0;
    BOOL_T dash = FALSE, valid = FALSE;
    UI32_T range_idx = 0;

    DSH_CHECK_PTR(ptr_value);

    /* init the range, keep 1st entry to 0 */
    ptr_value->key_match = FALSE;
    ptr_value->cnt = 0;
    for (range_idx = 0; range_idx < DSH_MAX_RANGE_NUM; range_idx++)
    {
        ptr_value->start[range_idx] = 0xffffffff;
        ptr_value->end[range_idx] = 0;
    }
    range_idx = 0;

    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    /* raise the key_match to init entry 0 */
    ptr_value->key_match = TRUE;

    /* e.g. 12,14,16-47 , token_len = 11 */
    for (idx = 0; idx < token_val_len; idx++)
    {
        if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
        {
            value = (value * 10) + (ptr_token_val[idx] - '0');
            valid = TRUE;
        }
        else if (',' == ptr_token_val[idx])
        {
            DSH_CHECK_COND(valid, ==, TRUE); /* Error: value is invalid */

            /* 1: value = 12, dash = 0
             * 2: value = 14, dash = 0
             */
            if (FALSE == dash)
            {
                DSH_CHECK_COND(range_idx, <, DSH_MAX_RANGE_NUM); /* Error: mem-overwrite */
                ptr_value->start[range_idx] = value;
                ptr_value->end[range_idx] = value;
            }
            /* 4: value = 47, start = 16, end = 47, dash = 0 */
            else
            {
                DSH_CHECK_COND(range_idx, <, DSH_MAX_RANGE_NUM); /* Error: mem-overwrite */
                ptr_value->end[range_idx] = value;
            }
            value = 0;
            dash = FALSE;
            valid = FALSE;
            range_idx += 1;
        }
        /* 3: value = 16, start = 16, end = 0, dash = 1 */
        else if ('-' == ptr_token_val[idx])
        {
            DSH_CHECK_COND(valid, ==, TRUE);                 /* Error: value is invalid */
            DSH_CHECK_COND(range_idx, <, DSH_MAX_RANGE_NUM); /* Error: mem-overwrite */
            if (TRUE == dash)
            {
                return (DSH_E_SYNTAX_ERR);
            }

            ptr_value->start[range_idx] = value;
            value = 0;
            dash = TRUE;
            valid = FALSE;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a dec number */
        }
    }

    /* last data */
    DSH_CHECK_COND(valid, ==, TRUE); /* Error: value is invalid */

    if (FALSE == dash)
    {
        DSH_CHECK_COND(range_idx, <, DSH_MAX_RANGE_NUM); /* Error: mem-overwrite */
        ptr_value->start[range_idx] = value;
        ptr_value->end[range_idx] = value;
    }
    else
    {
        DSH_CHECK_COND(range_idx, <, DSH_MAX_RANGE_NUM); /* Error: mem-overwrite */
        ptr_value->end[range_idx] = value;
    }

    /* get cnt */
    ptr_value->cnt = 0;
    for (range_idx = 0; range_idx < DSH_MAX_RANGE_NUM; range_idx++)
    {
        if (ptr_value->start[range_idx] <= ptr_value->end[range_idx])
        {
            ptr_value->cnt += (ptr_value->end[range_idx] - ptr_value->start[range_idx] + 1);
        }
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getMacAddr(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_MAC_T   *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;
    UI32_T value = 0, idx = 0, shift = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);
    osal_memset(ptr_value, 0, sizeof(AIR_MAC_T));

    /* e.g. 00:0A:00:1C:2D:3E, token_len = 16
     *      00-0A-00-1C-2D-3E, token_len = 16
     */
    for (idx = 0; idx < token_val_len; idx++)
    {
        if (('0' <= ptr_token_val[idx]) && ('9' >= ptr_token_val[idx]))
        {
            value = (value << 4) + (ptr_token_val[idx] - '0');
        }
        else if (('a' <= ptr_token_val[idx]) && ('f' >= ptr_token_val[idx]))
        {
            value = (value << 4) + (ptr_token_val[idx] - 'a') + 10;
        }
        else if (('A' <= ptr_token_val[idx]) && ('F' >= ptr_token_val[idx]))
        {
            value = (value << 4) + (ptr_token_val[idx] - 'A') + 10;
        }
        else if ((':' == ptr_token_val[idx]) || ('-' == ptr_token_val[idx]))
        {
            DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
            DSH_CHECK_COND(shift, <, 6);   /* Error: mem-overwrite */
            (*ptr_value)[shift] = (UI8_T)value;
            shift += 1;
            value = 0;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a hex number or colon */
        }
    }

    /* last data */
    DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
    DSH_CHECK_COND(shift, ==, 5);  /* Error: not a mac */
    (*ptr_value)[shift] = (UI8_T)value;

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_getIpv4Addr(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV4_T  *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    return dsh_transStrToIpv4Addr(ptr_token_val, ptr_value);
}

AIR_ERROR_NO_T
dsh_getIpv4Netmask(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV4_T  *ptr_value,
    AIR_IPV4_T  *ptr_mask)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    return dsh_transStrToIpv4Netmask(ptr_token_val, ptr_value, ptr_mask);
}

AIR_ERROR_NO_T
dsh_getIpv6Addr(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV6_T  *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    return dsh_transStrToIpv6Addr(ptr_token_val, ptr_value);
}

AIR_ERROR_NO_T
dsh_getIpv6Netmask(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    AIR_IPV6_T  *ptr_value,
    AIR_IPV6_T  *ptr_mask)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    return dsh_transStrToIpv6Netmask(ptr_token_val, ptr_value, ptr_mask);
}

AIR_ERROR_NO_T
dsh_getString(
    const C8_T  *tokens[],
    const UI32_T token_idx,
    const C8_T  *ptr_key,
    C8_T        *ptr_value)
{
    C8_T  *ptr_token_val = NULL;
    UI32_T token_val_len = 0;

    DSH_CHECK_PTR(ptr_value);
    DSH_CHECK_KEY_GET_VAL(tokens, token_idx, ptr_key, ptr_token_val, token_val_len);

    if (token_val_len > DSH_CMD_MAX_LENGTH)
    {
        return (DSH_E_KEY_MISMATCH);
    }

    osal_memset(ptr_value, 0, DSH_CMD_MAX_LENGTH);
    osal_strncpy(ptr_value, ptr_token_val, token_val_len);

    return (AIR_E_OK);
}

/* -------------------------------------------------------------- trans string to value */
AIR_ERROR_NO_T
dsh_inverseWords(
    UI32_T *ptr_value,
    UI32_T  size)
{
    UI32_T idx = 0;
    UI32_T temp_value = 0;

    for (idx = 0; idx < (size / DSH_CH_PER_WORDS); idx++)
    {
        temp_value = ptr_value[idx];
        ptr_value[idx] = ptr_value[size - idx - 1];
        ptr_value[size - idx - 1] = temp_value;
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_inverseBytes(
    UI8_T *ptr_value,
    UI32_T size)
{
    UI32_T idx = 0;
    UI8_T  temp_value = 0;

    for (idx = 0; idx < (size / DSH_CH_PER_BYTES); idx++)
    {
        temp_value = ptr_value[idx];
        ptr_value[idx] = ptr_value[size - idx - 1];
        ptr_value[size - idx - 1] = temp_value;
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToWords(
    const C8_T *ptr_str,
    UI32_T     *ptr_value,
    UI32_T     *ptr_size)
{
    UI32_T str_len = 0;
    UI32_T idx = 0, ch = 0, shift = 0;
    UI32_T value = 0;

    DSH_CHECK_PTR_GET_LEN(ptr_str, str_len);

    if (TRUE == DSH_CHECK_HEX(ptr_str))
    {
        /* e.g. str = 0x1000023456789abcd,   str_len = 19
         *          = 0x1.00002345.6789abcd, str_len = 21
         *          = 0x1.2345.6789abcd,     str_len = 17
         *
         *   value[0] = 0x6789abcd
         *   value[1] = 0x2345
         *   value[2] = 0x1
         */
        for (idx = str_len - 1; idx != DSH_START_HEX; idx--)
        {
            if (('.' == ptr_str[idx]) || (DSH_CH_PER_WORDS == ch))
            {
                DSH_CHECK_COND(shift, <, DSH_CMD_MAX_WORDS); /* Error: out-of-memory */
                ptr_value[shift++] = value;
                value = 0;
                ch = 0;
            }

            if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - '0')) << (4 * ch);
                ch++;
            }
            else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - 'a' + 10)) << (4 * ch);
                ch++;
            }
            else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - 'A' + 10)) << (4 * ch);
                ch++;
            }
            else if ('.' == ptr_str[idx])
            {
                continue;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        DSH_CHECK_COND(shift, <, DSH_CMD_MAX_WORDS); /* Error: out-of-memory */
        ptr_value[shift++] = value;
    }
    else
    {
        return (DSH_E_SYNTAX_ERR); /* Error: not a heximal number */
    }

    *ptr_size = shift;

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToBytes(
    const C8_T *ptr_str,
    UI8_T      *ptr_value,
    UI32_T     *ptr_size)
{
    UI32_T str_len = 0;
    UI32_T idx = 0, ch = 0, shift = 0;
    UI8_T  value = 0;

    DSH_CHECK_PTR_GET_LEN(ptr_str, str_len);

    if (TRUE == DSH_CHECK_HEX(ptr_str))
    {
        /* e.g. str = 0x10234,   str_len = 7
         *          = 0x1.02.34, str_len = 9
         *          = 0x1.2.34,  str_len = 8
         *
         *   value[0] = 0x34
         *   value[1] = 0x2
         *   value[2] = 0x1
         */
        for (idx = str_len - 1; idx != DSH_START_HEX; idx--)
        {
            if (('.' == ptr_str[idx]) || (DSH_CH_PER_BYTES == ch))
            {
                DSH_CHECK_COND(shift, <, DSH_CMD_MAX_BYTES); /* Error: out-of-memory */
                ptr_value[shift++] = value;
                value = 0;
                ch = 0;
            }

            if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - '0')) << (4 * ch);
                ch++;
            }
            else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - 'a' + 10)) << (4 * ch);
                ch++;
            }
            else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
            {
                value += ((UI32_T)(ptr_str[idx] - 'A' + 10)) << (4 * ch);
                ch++;
            }
            else if ('.' == ptr_str[idx])
            {
                continue;
            }
            else
            {
                return (DSH_E_SYNTAX_ERR);
            }
        }

        /* last data */
        DSH_CHECK_COND(shift, <, DSH_CMD_MAX_BYTES); /* Error: out-of-memory */
        ptr_value[shift++] = value;
    }
    else
    {
        return (DSH_E_SYNTAX_ERR); /* Error: not heximal */
    }

    *ptr_size = shift;

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToUpper(
    C8_T *ptr_str)
{
    UI32_T str_len = 0;
    UI32_T idx = 0;

    DSH_CHECK_PTR_GET_LEN(ptr_str, str_len);

    while (ptr_str[idx])
    {
        if ((ptr_str[idx] >= 'a') && (ptr_str[idx] <= 'z'))
        {
            ptr_str[idx] = ptr_str[idx] - 'a' + 'A';
        }
        idx++;
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToLower(
    C8_T *ptr_str)
{
    UI32_T str_len = 0;
    UI32_T idx = 0;

    DSH_CHECK_PTR_GET_LEN(ptr_str, str_len);

    while (ptr_str[idx])
    {
        if ((ptr_str[idx] >= 'A') && (ptr_str[idx] <= 'Z'))
        {
            ptr_str[idx] = ptr_str[idx] - 'A' + 'a';
        }
        idx++;
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToIpv4Addr(
    const C8_T *ptr_str,
    AIR_IPV4_T *ptr_addr)
{
    UI32_T value = 0, idx = 0, shift = 0;

    osal_memset(ptr_addr, 0, sizeof(AIR_IPV4_T));

    /* e.g. 192.168.1.2, token_len = 11 */
    for (idx = 0; idx < osal_strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            value = (value * 10) + (ptr_str[idx] - '0');
        }
        else if ('.' == ptr_str[idx])
        {
            DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
            DSH_CHECK_COND(shift, <, 4);   /* Error: mem-overwrite */
            *ptr_addr |= value << (24 - shift * 8);
            shift += 1;
            value = 0;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a digit number or dot */
        }
    }

    /* last data */
    DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
    DSH_CHECK_COND(shift, ==, 3);  /* Error: not an ipv4 addr */
    *ptr_addr |= value << (24 - shift * 8);

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToIpv4Netmask(
    const C8_T *ptr_str,
    AIR_IPV4_T *ptr_addr,
    AIR_IPV4_T *ptr_mask)
{
    UI32_T value = 0, idx = 0, shift = 0, slash = 0;

    osal_memset(ptr_addr, 0, sizeof(AIR_IPV4_T));
    osal_memset(ptr_mask, 0, sizeof(AIR_IPV4_T));

    /* e.g. 192.168.1.2/16, token_len = 14 */
    for (idx = 0; idx < osal_strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            value = (value * 10) + (ptr_str[idx] - '0');
        }
        else if ('.' == ptr_str[idx])
        {
            DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
            DSH_CHECK_COND(shift, <, 4);   /* Error: mem-overwrite */
            *ptr_addr |= value << (24 - shift * 8);
            shift += 1;
            value = 0;
        }
        else if ('/' == ptr_str[idx])
        {
            slash = 1;

            /* last data */
            DSH_CHECK_COND(value, <, 256); /* Error: invalid value */
            DSH_CHECK_COND(shift, <, 4);   /* Error: mem-overwrite */
            *ptr_addr |= value << (24 - shift * 8);
            value = 0;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a digit number or dot */
        }
    }

    /* the shift must be 3 */
    DSH_CHECK_COND(shift, ==, 3); /* Error: not an ipv4 addr */

    /* the slash must be 1 */
    DSH_CHECK_COND(slash, ==, 1); /* Error: no prefix length */

    /* mask-data, the prefiex length should be less than or equal to 32 */
    DSH_CHECK_COND(value, <=, 32); /* Error: invalid value */

    /* e.g. value = 0,  mask = 0x00000000
     *      value = 8,  mask = 0xFF000000
     *      value = 20, mask = 0xFFFFF000
     */
    *ptr_mask = (0 == value) ? 0 : ~((0x1U << (32 - value)) - 1);
    *ptr_addr &= *ptr_mask;

    return AIR_E_OK;
}

AIR_ERROR_NO_T
dsh_transStrToIpv6Addr(
    const C8_T *ptr_str,
    AIR_IPV6_T *ptr_addr)
{
    UI32_T     hex_value = 0, dec_value = 0, idx = 0;
    BOOL_T     double_colon = FALSE, ipv4_compatible = FALSE;
    UI32_T     double_colon_pos = 0, last_pos = 0;
    AIR_IPV6_T tmp_ipv6 = {0};

    osal_memset(ptr_addr, 0, sizeof(AIR_IPV6_T));

    /* e.g. invalid:
     * 3ffe::c0a8:0:      last cannot be colon except double-colon
     * 3ffe:::c0a8:0      triple-colon
     * 3ffe::c0a8::0      two double-colons
     */

    /* e.g. valid:
     * 3ffe::c0a8:0       token_len = 12 (double-colon in middle)
     * 3ffe:c0a8:0::      token_len = 13 (double-colon in last)
     * ::3ffe:c0a8:0      token_len = 13 (double-colon in first)
     * 3ffe::192.168.0.0  token_len = 17 (IPv4-compatible address)
     */
    for (idx = 0; idx < osal_strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - '0');
            dec_value = (dec_value * 10) + (ptr_str[idx] - '0');
        }
        else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'a') + 10;
        }
        else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'A') + 10;
        }
        else if (':' == ptr_str[idx])
        {
            /* must belong to double-colon, calculate from last */
            if (0 == idx)
            {
                continue;
            }
            /* not the first ch but a double-colon */
            else if (':' == ptr_str[idx - 1])
            {
                DSH_CHECK_COND(double_colon, ==, FALSE); /* Error: triple-colon or two double-colons */
                double_colon = TRUE;
            }
            /* not the first ch and a double-colon */
            else
            {
                DSH_CHECK_COND(double_colon_pos, <, 15); /* Error: only 16 units for UI8_T  */
                DSH_CHECK_COND(last_pos, <, 15);         /* Error: only 16 units for UI8_T  */
                tmp_ipv6[last_pos] = (UI8_T)((hex_value >> 8) & 0xff);
                tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
                double_colon_pos += (FALSE == double_colon) ? 2 : 0;
                last_pos += 2;
                hex_value = 0;
                dec_value = 0;
            }
        }
        else if ('.' == ptr_str[idx])
        {
            DSH_CHECK_COND(last_pos, <, 16); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos] = dec_value;
            last_pos += 1;
            dec_value = 0;
            ipv4_compatible = TRUE;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a hex number or colon */
        }
    }

    if (2 > idx)
    {
        return (DSH_E_SYNTAX_ERR);
    }

    /* last data */
    if (':' != ptr_str[idx - 1])
    {
        if (FALSE == ipv4_compatible)
        {
            DSH_CHECK_COND(last_pos, <, 15); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos] = (UI8_T)((hex_value >> 8) & 0xff);
            tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
            last_pos += 2;
        }
        else
        {
            DSH_CHECK_COND(last_pos, <, 16); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos] = dec_value;
            last_pos += 1;
        }
    }
    else
    {
        if (':' != ptr_str[idx - 2])
        {
            return (DSH_E_SYNTAX_ERR); /* Error: last cannot be colon except double-colon */
        }
    }

    /* move tmp_ipv6 to ptr_value */
    if (TRUE == double_colon)
    {
        /* e.g.
         * 3ffe::c0a8:0       double_colon_pos = 2, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * 3ffe:c0a8:0::      double_colon_pos = 6, last_pos = 6,   tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * ::3ffe:c0a8:0      double_colon_pos = 0, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * 3ffe::192.168.0.0  double_colon_pos = 2, last_pos = 5+1, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         *
         *                                 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
         * 3ffe::c0a8:0       ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         * 3ffe:c0a8:0::      ptr_value = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
         * ::3ffe:c0a8:0      ptr_value = {--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         * 3ffe::192.168.0.0  ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         */
        for (idx = 0; idx < double_colon_pos; idx++)
        {
            (*ptr_addr)[idx] = tmp_ipv6[idx];
        }
        /* e.g.
         *                                 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
         * 3ffe::c0a8:0       ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
         * 3ffe:c0a8:0::      ptr_value = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
         * ::3ffe:c0a8:0      ptr_value = {--,--,--,--,--,--,--,--,--,--,3f,fe,c0,a8,00,00}
         * 3ffe::192.168.0.0  ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
         */
        for (idx = double_colon_pos; idx < last_pos; idx++)
        {
            (*ptr_addr)[16 - (last_pos - idx)] = tmp_ipv6[idx];
        }
    }
    else
    {
        for (idx = 0; idx < 16; idx++)
        {
            (*ptr_addr)[idx] = tmp_ipv6[idx];
        }
    }

    return (AIR_E_OK);
}

AIR_ERROR_NO_T
dsh_transStrToIpv6Netmask(
    const C8_T *ptr_str,
    AIR_IPV6_T *ptr_addr,
    AIR_IPV6_T *ptr_mask)
{
    UI32_T     hex_value = 0, dec_value = 0, idx = 0, jdx = 0;
    BOOL_T     double_colon = FALSE, ipv4_compatible = FALSE;
    UI32_T     double_colon_pos = 0, last_pos = 0, slash = 0;
    AIR_IPV6_T tmp_ipv6 = {0};

    osal_memset(ptr_addr, 0, sizeof(AIR_IPV6_T));
    osal_memset(ptr_mask, 0, sizeof(AIR_IPV6_T));

    /* e.g. invalid:
     * 3ffe::c0a8:0:/16      last cannot be colon except double-colon
     * 3ffe:::c0a8:0/16      triple-colon
     * 3ffe::c0a8::0/16      two double-colons
     */

    /* e.g. valid:
     * 3ffe::c0a8:0/16       token_len = 15 (double-colon in middle)
     * 3ffe:c0a8:0::/16      token_len = 16 (double-colon in last)
     * ::3ffe:c0a8:0/16      token_len = 16 (double-colon in first)
     * 3ffe::192.168.0.0/16  token_len = 20 (IPv4-compatible address)
     */
    for (idx = 0; idx < osal_strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - '0');
            dec_value = (dec_value * 10) + (ptr_str[idx] - '0');
        }
        else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'a') + 10;
        }
        else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'A') + 10;
        }
        else if (':' == ptr_str[idx])
        {
            /* must belong to double-colon, calculate from last */
            if (0 == idx)
            {
                continue;
            }
            /* not the first ch but a double-colon */
            else if (':' == ptr_str[idx - 1])
            {
                DSH_CHECK_COND(double_colon, ==, FALSE); /* Error: triple-colon or two double-colons */
                double_colon = TRUE;
            }
            /* not the first ch and a double-colon */
            else
            {
                DSH_CHECK_COND(double_colon_pos, <, 15); /* Error: only 16 units for UI8_T  */
                DSH_CHECK_COND(last_pos, <, 15);         /* Error: only 16 units for UI8_T  */
                tmp_ipv6[last_pos] = (UI8_T)((hex_value >> 8) & 0xff);
                tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
                double_colon_pos += (FALSE == double_colon) ? 2 : 0;
                last_pos += 2;
                hex_value = 0;
                dec_value = 0;
            }
        }
        else if ('.' == ptr_str[idx])
        {
            if (last_pos >= 16)
            {
                return (DSH_E_SYNTAX_ERR); /* Error: only 16 units for UI8_T  */
            }

            tmp_ipv6[last_pos] = dec_value;
            last_pos += 1;
            dec_value = 0;
            ipv4_compatible = TRUE;
        }
        else if ('/' == ptr_str[idx])
        {
            if (2 > idx)
            {
                return (DSH_E_SYNTAX_ERR);
            }
            slash = 1;

            /* last data */
            if (':' != ptr_str[idx - 1])
            {
                if (FALSE == ipv4_compatible)
                {
                    DSH_CHECK_COND(last_pos, <, 15); /* Error: only 16 units for UI8_T  */
                    tmp_ipv6[last_pos] = (UI8_T)((hex_value >> 8) & 0xff);
                    tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
                    last_pos += 2;
                }
                else
                {
                    DSH_CHECK_COND(last_pos, <, 16); /* Error: only 16 units for UI8_T  */
                    tmp_ipv6[last_pos] = dec_value;
                    last_pos += 1;
                }
            }
            else
            {
                if (':' != ptr_str[idx - 2])
                {
                    return (DSH_E_SYNTAX_ERR); /* Error: last cannot be colon except double-colon */
                }
            }

            /* move tmp_ipv6 to ptr_addr */
            if (TRUE == double_colon)
            {
                /* e.g.
                 * 3ffe::c0a8:0/16       double_colon_pos = 2, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
                 * 3ffe:c0a8:0::/16      double_colon_pos = 6, last_pos = 6,   tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
                 * ::3ffe:c0a8:0/16      double_colon_pos = 0, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
                 * 3ffe::192.168.0.0/16  double_colon_pos = 2, last_pos = 5+1, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
                 *
                 *                                    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
                 * 3ffe::c0a8:0/16       ptr_addr = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
                 * 3ffe:c0a8:0::/16      ptr_addr = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
                 * ::3ffe:c0a8:0/16      ptr_addr = {--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
                 * 3ffe::192.168.0.0/16  ptr_addr = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
                 */
                for (jdx = 0; jdx < double_colon_pos; jdx++)
                {
                    (*ptr_addr)[jdx] = tmp_ipv6[jdx];
                }
                /* e.g.
                 *                                    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
                 * 3ffe::c0a8:0/16       ptr_addr = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
                 * 3ffe:c0a8:0::/16      ptr_addr = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
                 * ::3ffe:c0a8:0/16      ptr_addr = {--,--,--,--,--,--,--,--,--,--,3f,fe,c0,a8,00,00}
                 * 3ffe::192.168.0.0/16  ptr_addr = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
                 */
                for (jdx = double_colon_pos; jdx < last_pos; jdx++)
                {
                    (*ptr_addr)[16 - (last_pos - jdx)] = tmp_ipv6[jdx];
                }
            }
            else
            {
                for (jdx = 0; jdx < 16; jdx++)
                {
                    (*ptr_addr)[jdx] = tmp_ipv6[jdx];
                }
            }
            dec_value = 0; /* To get the prefix length */
        }
        else
        {
            return (DSH_E_SYNTAX_ERR); /* Error: not a hex number or colon */
        }
    }

    /* the slash must be 1 */
    DSH_CHECK_COND(slash, ==, 1); /* Error: no prefix length */

    /* mask-data, the prefiex length should be less than or equal to 128 */
    DSH_CHECK_COND(dec_value, <=, 128); /* Error: invalid value */

    for (idx = 0; idx < 16; idx++)
    {
        /* e.g. value = 0,  mask = {00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}
         *      value = 8,  mask = {ff,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}
         *      value = 20, mask = {ff,ff,f0,00,00,00,00,00,00,00,00,00,00,00,00,00}
         */
        if (dec_value >= 8)
        {
            (*ptr_mask)[idx] = 0xff;
            (*ptr_addr)[idx] &= (*ptr_mask)[idx];
            dec_value -= 8;
        }
        else if (0 == dec_value)
        {
            (*ptr_mask)[idx] = 0;
            (*ptr_addr)[idx] &= (*ptr_mask)[idx];
        }
        else
        {
            (*ptr_mask)[idx] = ~((0x1U << (8 - dec_value)) - 1);
            (*ptr_addr)[idx] &= (*ptr_mask)[idx];
            dec_value = 0;
        }
    }

    return AIR_E_OK;
}

AIR_ERROR_NO_T
dsh_splitString(
    C8_T *tokens[],
    C8_T *ptr_str)
{
    UI32_T str_len = 0;
    UI32_T idx = 0, begin = 0, shift = 0;

    DSH_CHECK_PTR(tokens);
    DSH_CHECK_PTR_GET_LEN(ptr_str, str_len);
    /* e.g. *ptr_str = "pkt  set rx init unit=1", token_len = 23
     *      *tokens[] = {*+0, *+5, *+9, *+12, *+17, *+22, 0, 0, ...}
     */
    for (idx = 0; idx < str_len; idx++)
    {
        /* 1: idx = 3,  begin = 0,  shift = 0
         * 2: idx = 4,  begin = 4,  shift = 1
         * 3: idx = 8,  begin = 5,  shift = 1
         * 4: idx = 11, begin = 9,  shift = 2
         * 5: idx = 16, begin = 12, shift = 3
         * 6: idx = 21, begin = 17, shift = 4
         */
        if ((' ' == ptr_str[idx]) || ('=' == ptr_str[idx]))
        {
            if (idx > begin)
            {
                DSH_CHECK_COND(shift, <, DSH_CMD_MAX_TOKENS); /* Error: out-of-memory */
                tokens[shift++] = ptr_str + begin;
            }
            ptr_str[idx] = '\0';
            begin = idx + 1;
        }
    }

    /* last data
     * 6: idx = 23, begin = 22, shift = 5
     */
    if (' ' != ptr_str[idx])
    {
        if (idx > begin)
        {
            DSH_CHECK_COND(shift, <, DSH_CMD_MAX_TOKENS); /* Error: out-of-memory */
            tokens[shift++] = ptr_str + begin;
        }
        ptr_str[idx] = '\0';
    }

    return (AIR_E_OK);
}
