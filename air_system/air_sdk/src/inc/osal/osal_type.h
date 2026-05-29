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

/* FILE NAME:   osal_types.h
 * PURPOSE:
 *      Define the commom data type in AIR SDK.
 * NOTES:
 */

#ifndef OSAL_TYPE_H
#define OSAL_TYPE_H

/* DATA TYPE DECLARATIONS
 */
typedef int                BOOL_T;
typedef signed char        I8_T;
typedef unsigned char      UI8_T;
typedef signed short       I16_T;
typedef unsigned short     UI16_T;
typedef signed int         I32_T;
typedef unsigned int       UI32_T;
typedef char               C8_T;
typedef unsigned long long UI64_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define UI64_HI(dst)                ((UI32_T)((dst) >> 32))
#define UI64_LOW(dst)               ((UI32_T)((dst) & 0xffffffff))
#define UI64_ASSIGN(dst, high, low) ((dst) = (((UI64_T)(high)) << 32 | (UI64_T)(low)))
#define UI64_SET(dst, src)          ((dst) = (src))
#define UI64_ADD_UI32(dst, src)     ((dst) += ((UI64_T)(src)))
#define UI64_SUB_UI32(dst, src)     ((dst) -= ((UI64_T)(src)))
#define UI64_ADD_UI64(dst, src)     ((dst) += (src))
#define UI64_SUB_UI64(dst, src)     ((dst) -= (src))
#define UI64_AND(dst, src)          ((dst) &= (src))
#define UI64_OR(dst, src)           ((dst) |= (src))
#define UI64_XOR(dst, src)          ((dst) ^= (src))
#define UI64_NOT(dst)               ((dst) = ~(dst))
#define UI64_MULT_UI32(dst, src)    ((dst) *= (src))
#define UI64_MULT_UI64(dst, src)    ((dst) *= (src))

#endif /* End of OSAL_TYPE_H */
