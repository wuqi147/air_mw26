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

/* FILE NAME:   mw_utils.h
 * PURPOSE:
 *      Define the initialization in AIR Magic Wand system.
 * NOTES:
 */

#ifndef MW_UTILS_H
#define MW_UTILS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "mw_error.h"
#include "product_ref.h"
#include "lwip/pbuf.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_UTIL_IPV4_STR_SIZE        (16)
#define MW_UTIL_IPV4_TO_STR(__buf__,__ipv4__)    \
                        osapi_snprintf(__buf__,MW_UTIL_IPV4_STR_SIZE,"%ld.%ld.%ld.%ld",   \
                        ((__ipv4__)&0xFF000000)>>24,((__ipv4__)&0x00FF0000)>>16,    \
                        ((__ipv4__)&0x0000FF00)>>8, ((__ipv4__)&0x000000FF))

/* MACRO FUNCTION DECLARATIONS
 */
#undef BIG_ENDIAN
#if defined (BIG_ENDIAN)
#define htons(x) ((UI16_T)(x))
#define ntohs(x) ((UI16_T)(x))
#define htonl(x) ((UI32_T)(x))
#define ntohl(x) ((UI32_T)(x))
#else /* LITTLE_ENDIAN */
#ifndef htons
#define htons(x) ((UI16_T)((((x) & (UI16_T)0x00ffU) << 8) | (((x) & (UI16_T)0xff00U) >> 8)))
#endif
#ifndef ntohs
#define ntohs(x) htons(x)
#endif
#ifndef htonl
#define htonl(x) ((((x) & (UI32_T)0x000000ffUL) << 24) | \
                     (((x) & (UI32_T)0x0000ff00UL) <<  8) | \
                     (((x) & (UI32_T)0x00ff0000UL) >>  8) | \
                     (((x) & (UI32_T)0xff000000UL) >> 24))
#endif
#ifndef ntohl
#define ntohl(x) htonl(x)
#endif
#endif

UI8_T * put32(UI8_T *cp, UI32_T x);
UI32_T get32(UI8_T *cp);
UI8_T *put16(UI8_T *cp, UI16_T x);
UI16_T get16(UI8_T *cp);
C8_T *_printUI64_T(UI64_T value, C8_T *varStr, UI16_T strLen);
MW_ERROR_NO_T
mw_get_version(C8_T *ptr_str);
MW_ERROR_NO_T
mw_pbuf_init(struct pbuf *ptr_pbuf);

#define MW_CHECK_PTR(__ptr__) do                                            \
{                                                                           \
    if (NULL == (__ptr__))                                                  \
    {                                                                       \
        return  MW_E_BAD_PARAMETER;                                         \
    }                                                                       \
} while (0)

#define MW_PARAM_CHK(expr, errCode) do                                      \
{                                                                           \
    if ((I32_T)(expr))                                                      \
    {                                                                       \
        return errCode;                                                     \
    }                                                                       \
} while (0)

#define MW_CHECK_BOOL(__bool__, errCode)  \
    MW_PARAM_CHK(((TRUE != (__bool__)) && (FALSE != (__bool__))), errCode)

#define MW_FREE(ptr) do                                                     \
{                                                                           \
    if (ptr)                                                                \
    {                                                                       \
        osapi_free(ptr);                                                    \
        ptr = NULL;                                                         \
    }                                                                       \
}while (0)

#define MW_CHECK_MIN_MAX_RANGE(__value__, __min__, __max__) do            \
    {                                                                       \
        if ( ((__value__) > (__max__))  ||                                  \
             ((__value__) < (__min__)) )                                    \
        {                                                                   \
            return  MW_E_BAD_PARAMETER;                                   \
        }                                                                   \
    } while (0)

#define IN_RANGE(c, lo, up)   ((UI8_T)(c) >= (lo) && (UI8_T)(c) <= (up))
#define IS_DIGIT(c)           IN_RANGE((c), '0', '9')
#define IS_XDIGIT(c)          (IS_DIGIT(c) || IN_RANGE((c), 'a', 'f') || IN_RANGE((c), 'A', 'F'))
#define IS_LOWER(c)           IN_RANGE((c), 'a', 'z')
#define IS_UPPER(c)           IN_RANGE((c), 'A', 'Z')
#define IS_SPACE(c)           ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
#define TO_LOWER(c)           (IS_UPPER(c) ? (c) - 'A' + 'a' : c)
#define TO_UPPER(c)           (IS_LOWER(c) ? (c) - 'a' + 'A' : c)

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif	/* End of BIT */

/* bits range: for example BITS(16,23) = 0xFF0000*/
#ifndef BITS
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#endif	/* End of BITS */

/* bits range: for example BITS_RANGE(16,4) = 0x0F0000*/
#ifndef BITS_RANGE
#define BITS_RANGE(offset, range)           BITS((offset), ((offset)+(range)-1))
#endif	/* End of BITS_RANGE */

/* bits offset right: for example BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef BITS_OFF_R
#define BITS_OFF_R(val, offset, range)      (((val) >> offset) & (BITS(0, (range) - 1)))
#endif	/* End of BITS_OFF_R */

/* bits offset left: for example BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef BITS_OFF_L
#define BITS_OFF_L(val, offset, range)      (((val) & (BITS(0, (range) - 1))) << (offset))
#endif	/* End of BITS_OFF_L */

#ifndef BIT_DEL
#define BIT_DEL(value, offset) ((value) &= ~(BIT(offset)))
#endif

#ifndef MAX
#define MAX(a, b)   (((a)>(b))?(a):(b))
#endif	/* End of MAX */

#ifndef MIN
#define MIN(a, b)   (((a)<(b))?(a):(b))
#endif	/* End of MIN */

#ifndef BIT_CHK
#define BIT_CHK(x,y)    (((x) & (1 << (y))) != 0)
#endif	/* End of BIT_CHK */

#ifndef BIT_SET
#define BIT_SET(x,y)    ((x) |= (1 << (y)))
#endif	/* End of BIT_SET */

/* DATA TYPE DECLARATIONS
 */
typedef MW_ERROR_NO_T
(*MW_PRINTF)(
    C8_T* fmt,
    ...);

#ifdef AIR_SUPPORT_SNTP
typedef struct MW_DATE_S {
    UI16_T year;
    UI8_T month;
    UI8_T day;
    UI8_T hours;
    UI8_T minutes;
    UI8_T seconds;
}MW_DATE_T;
#endif
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_PRODUCT_ID_T
mw_get_productID(
    void);

void
mw_set_productID(
    MW_PRODUCT_ID_T product_id);

MW_ERROR_NO_T
mw_get_lights_array(
    UI8_T *ptr_up,
    UI8_T *ptr_dwn);

#ifdef AIR_SUPPORT_SNTP
void
mw_timeToDate(
    UI64_T time,
    MW_DATE_T *ptr_st_date);

UI64_T
mw_dateToTime(
    MW_DATE_T st_date);

/* FUNCTION NAME: sys_mgmt_getTimes()
 * PURPOSE:
 *      Get local system time in seconds
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_times   - A point of System times
 *      ptr_date    - A point of struct of Date
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_getTimes(
    UI64_T *ptr_times,
    MW_DATE_T *ptr_date);

/* FUNCTION NAME: sys_mgmt_setTimes()
 * PURPOSE:
 *      Convert date to times
 *
 * INPUT:
 *      date    - struct of Date
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_setTimes(
    MW_DATE_T date);
#endif /* AIR_SUPPORT_SNTP */

#endif  /* MW_UTILS_H */

