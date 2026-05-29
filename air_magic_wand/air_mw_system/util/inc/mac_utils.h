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

/* FILE NAME:   mac_utils.h
 * PURPOSE:
 *      Define the MAC convert utils API in AIR Magic Wand system.
 * NOTES:
 */

#ifndef MAC_UTILS_H
#define MAC_UTILS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "mw_types.h"
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN      6
#endif

#define MAX_PER_WEBPAGE_SHOW_DYNAMIC_MAC_ADDRESS_ENTRY_NUM    (AIR_MAX_PER_WEBPAGE_DYNAMIC_MAC_ADDRESS_ENTRY_NUM)   /* Maximum number of dynamic MAC displayed per webpage (N) */
#define PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM           (AIR_MAX_PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM) /* Number of dynamic MAC requests per webpage (n) */
/* MAC ENTRY STRING LENGTH DECLARATIONS */
#define MAX_VID_STR_LEN                                       (4)   /* MAX VID string length, e.g. "4094" */
#define MAC_ADDR_STR_LEN                                      (12)  /* MAC address string length, e.g. "aabbcc112233" */
#define MAX_MAC_ADDR_STR_LEN                                  (17)  /* MAC address string length, e.g. "aa-bb-cc-11-22-33" */
#define MAX_PORTBMP_STR_LEN                                   (10)  /* MAX port bitmap string length, e.g. "0xfffffffe" */
#define MAX_AGE_TIME_STR_LEN                                  (3)   /* MAX age time string length, e.g. "300" */
/* DATA TYPE DECLARATIONS
 */
typedef struct ether_addr_s {
    UI8_T octet[ETHER_ADDR_LEN];
} ether_addr_t;

/* The dynamic mac entries info */
typedef struct DYNAMIC_MAC_ADDRESS_ENTRY_INFO_S
{
    MW_MAC_T        mac_addr;                            /* The dynamic MAC address */
    UI16_T          vid;                                 /* The VID of the dynamic MAC */
    UI32_T          port;                                /* The port bitmap of the dynamic MAC */
    UI32_T          age;                                 /* The age of the dynamic MAC */
} ATTRIBUTE_PACK DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T;

typedef struct ONE_DB_STATIC_MAC_ENTRY_S{
    MW_MAC_T    mac_addr; /* The static MAC address */
    UI16_T      vid;      /* The VID of the static MAC */
    UI16_T      port;     /* The port of the static MAC */
}ONE_DB_STATIC_MAC_ENTRY_T;

#define MW_IS_V4MCAST_MAC_ADDR(addr)                 (0x01 == (((UI8_T *)(addr))[0])) && \
                                                    (0x00 == (((UI8_T *)(addr))[1])) && \
                                                    (0x5E == (((UI8_T *)(addr))[2])))

#define MW_IS_V6MCAST_MAC_ADDR(addr)                 (0x33 == (((UI8_T *)(addr))[0])) && \
                                                    (0x33 == (((UI8_T *)(addr))[1])))

#define MW_IS_BCAST_MAC_ADDR(addr) do                        \
{                                                           \
    UI32_T i;                                               \
    for(i = 0; i < ETHER_ADDR_LEN; i++)                     \
    {                                                       \
        (0xFF == (((UI8_T *)(addr))[i]));                   \
    }                                                       \
}while(0)



int asciiToInt( char **s);

UI8_T charToInt(char c);

UI32_T strToMac(UI8_T *a, ether_addr_t *eth);

UI32_T macToStr(ether_addr_t *n, UI8_T *a, BOOL_T spilt);

int macAddrCopy(UI8_T *dest, UI8_T *src);

int macAddrCmp(UI8_T *dest, UI8_T *src);

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
UI32_T strToOui(const C8_T *oui_str, VOVLAN_OUI_T *oui_mac);
#endif
#endif  /* End of MAC_UTILS_H */

