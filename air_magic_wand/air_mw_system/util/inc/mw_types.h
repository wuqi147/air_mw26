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


/* FILE NAME:   mw_types.h
 * PURPOSE:
 *      Define the commom data type in AIR Magic Wand system.
 * NOTES:
 */

#ifndef MW_TYPES_H
#define MW_TYPES_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
/* NAMING CONSTANT DECLARATIONS
 */
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef NULL
#define NULL                (void *)0
#endif

#ifndef LOW
#define LOW                 0
#endif

#ifndef HIGH
#define HIGH                1
#endif

#define MW_MAC_ADDR_LEN     (6)


/* DATA TYPE DECLARATIONS
 */
typedef int                 BOOL_T;
typedef signed char         I8_T;
typedef unsigned char       UI8_T;
typedef signed short        I16_T;
typedef unsigned short      UI16_T;
typedef signed int          I32_T;
typedef unsigned int        UI32_T;
typedef char                C8_T;
typedef unsigned long long  UI64_T;
typedef float               FLOAT_T;


typedef UI8_T   MW_BIT_MASK_8_T;
typedef UI16_T  MW_BIT_MASK_16_T;
typedef UI32_T  MW_BIT_MASK_32_T;
typedef UI64_T  MW_BIT_MASK_64_T;

typedef UI8_T   MW_MAC_T[6];
typedef UI32_T  MW_IPV4_T;
typedef UI8_T   MW_IPV6_T[16];

typedef UI32_T  MW_TIME_T;

/* Bridge Domain id data type. */
typedef UI32_T MW_BRIDGE_DOMAIN_T;

typedef union MW_IP_U
{
    MW_IPV4_T     ipv4_addr;
    MW_IPV6_T     ipv6_addr;

}MW_IP_T;

typedef struct MW_IP_ADDR_S
{
   MW_IP_T      ip_addr;
   BOOL_T       ipv4 ;
}MW_IP_ADDR_T;



/***************************/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif  /* MW_TYPES_H */

