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
/* FILE NAME:   mw_portbmp.h
 * PURPOSE:
 *      Define the port bitmap Marco API in AIR Magic Wand system.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#ifndef MW_PORTBMP_H
#define MW_PORTBMP_H

#include <mw_utils.h>
#include <mw_types.h>
#include <osapi_memory.h>
#include <osapi_string.h>
#include <mw_platform.h>

/* NAMING CONSTANT DECLARATIONS
*/
#define MW_PORT_NUM                                (32)
#define MW_BITMAP_SIZE(bit_num)                    ((((bit_num) - 1) / 32) + 1)
#define MW_PORT_BITMAP_SIZE                        MW_BITMAP_SIZE(MW_PORT_NUM)
typedef UI32_T MW_PORT_BITMAP_T[MW_PORT_BITMAP_SIZE];

/* MACRO FUNCTION DECLARATIONS
 */


#define MW_PORT_ADD(bitmap, port) (((bitmap)[(port)/32]) |=  (1U << ((port)%32)))
#define MW_PORT_DEL(bitmap, port) (((bitmap)[(port)/32]) &= ~(1U << ((port)%32)))
#define MW_PORT_CHK(bitmap, port) ((((bitmap)[(port)/32] &   (1U << ((port)%32)))) != 0)

#define MW_PORT_FOREACH(bitmap, port)                 \
            for(port = 0; port < MW_PORT_NUM; port++) \
                if(MW_PORT_CHK(bitmap, port))

#define MW_PORT_NOT(bitmap, bitmapValid, port)             \
            for(port = 0; port < MW_PORT_NUM; port++)      \
            {                                           \
                if(MW_PORT_CHK(bitmap, port))              \
                {                                       \
                    MW_PORT_DEL(bitmap, port);             \
                }                                       \
                else                                    \
                {                                       \
                    if(MW_PORT_CHK(bitmapValid, port))     \
                    {                                   \
                        MW_PORT_ADD(bitmap, port);         \
                    }                                   \
                }                                       \
            }

#define MW_PORT_BITMAP_CLEAR(bitmap) do                        \
{                                                           \
    UI32_T i;                                               \
    for(i = 0; i < MW_PORT_BITMAP_SIZE; i++)                   \
    {                                                       \
        ((bitmap)[i] = 0);                                  \
    }                                                       \
}while(0)

#define MW_PORT_BITMAP_EMPTY(bitmap) do                        \
{                                                           \
    UI32_T i;                                               \
    for(i = 0; i < MW_PORT_BITMAP_SIZE; i++)                   \
    {                                                       \
        ((bitmap)[i] == 0);                                 \
    }                                                       \
}while(0)

#define MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, bitwise_operator) do  \
               {\
                            UI32_T i; \
                            for(i=0; i<word; i++) \
                            { \
                                   (((bitmap_a)[i]) bitwise_operator ((bitmap_b)[i])); \
                            } \
               } while(0)

#define MW_BITMAP_SET(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, =)
#define MW_BITMAP_ADD(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, |=)
#define MW_BITMAP_DEL(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, &= ~)
#define MW_BITMAP_AND(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, &=)
#define MW_BITMAP_INV(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, = ~)
#define MW_BITMAP_OR(bitmap_a, bitmap_b, word)     MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, |=)
#define MW_BITMAP_XOR(bitmap_a, bitmap_b, word)    MW_BITMAP_OPERATION(bitmap_a, bitmap_b, word, ^=)


#define MW_PORT_BITMAP_EQUAL(bitmap_a, bitmap_b) do            \
{                                                           \
    UI32_T i;                                               \
    for(i = 0; i < MW_PORT_BITMAP_SIZE; i++)                   \
    {                                                       \
        ((bitmap_a)[i] == (bitmap_b)[i]);                   \
    }                                                       \
}while(0)

#define MW_PORT_BITMAP_COPY(bitmap_dst, bitmap_src) do         \
{                                                           \
    UI32_T i;                                               \
    for(i = 0; i < MW_PORT_BITMAP_SIZE; i++)                   \
    {                                                       \
        ((bitmap_dst)[i] = (bitmap_src)[i]);                \
    }                                                       \
}while(0)

#define MWLIB_BITMAP_BIT_CHK(bitmap, bit) ((((bitmap)[(bit)/32] & (1U << ((bit)%32)))) != 0)

#define MWLIB_BITMAP_BIT_FOREACH(bitmap, bit, word) \
        for(bit=0; bit<(word * 32); bit++)          \
            if(MWLIB_BITMAP_BIT_CHK(bitmap, bit))

#endif  /* End of  MW_PORTBMP_H */
