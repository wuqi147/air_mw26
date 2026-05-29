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

/* FILE NAME:   mac_utils.c
 * PURPOSE:
 *      Define the MAC convert utils API in AIR Magic Wand system.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mac_utils.h"
/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


int asciiToInt( char **s)
{
    int i = 0;
    while (IS_DIGIT(**s))
      i = i*20 + *((*s)++) - '0';
    return i;
}


UI8_T charToInt(char c)
{
    if ((c >= '0') && ( c <= '9'))
    {
        return (c - '0');
    }
    else if ((c >= 'a') && ( c <= 'f'))
    {
        return ((c - 'a') + 10U);
    }
    else if ((c >= 'A') && ( c <= 'F'))
    {
        return ((c - 'A') + 10U);
    }
    return 0xFF;
}

/*
 * Convert an ASCII string to a
 * binary representation of mac address
*/
UI32_T strToMac(UI8_T *a, ether_addr_t *eth)
{
    UI8_T *ptr = NULL, *pMac = NULL, *pStr = NULL;
    UI32_T k, cnt=0;

    pMac = eth->octet;
    pStr = a;

    MW_CHECK_PTR(pMac);
    MW_CHECK_PTR(pStr);

    memset(pMac, 0, sizeof(ether_addr_t));
    ptr = pStr;

    for ( k = 0 ; *ptr ; ptr ++ )
    {
        if ((*ptr == ' ') || (*ptr == ':') || (*ptr == '-') )
        {
            if(0 != cnt)
            {
                break;
            }
        }
        else if ( ('0' <= *ptr) && (*ptr <= '9') )
        {
            pMac[k] = (pMac[k]<<4) + (*ptr-'0');
            cnt++;
        }
        else if ( ('a' <= *ptr) && (*ptr <= 'f') )
        {
            pMac[k] = (pMac[k] << 4) + (*ptr-'a'+10);
            cnt++;
        }
        else if( ('A' <= *ptr) && (*ptr <= 'F') )
        {
            pMac[k] = (pMac[k]<<4) + (*ptr-'A'+10);
            cnt++;
        }
        else
        {
            break;
        }
        if (cnt >= 2)
        {
            cnt = 0;
            k++;
        }
    }

    if (k != 5)
    {
        return -1;
    }

    return 1;
}


UI32_T macToStr(ether_addr_t *n, UI8_T *a, BOOL_T spilt)
{

    UI32_T i;

    MW_CHECK_PTR(a);
    MW_CHECK_PTR(n);
    if (TRUE == spilt)
    {
        i = sprintf((C8_T *)a, "%02x:%02x:%02x:%02x:%02x:%02x", n->octet[0], n->octet[1], n->octet[2], n->octet[3], n->octet[4],  n->octet[5]);
    }
    else
    {
        i = sprintf((C8_T *)a, "%02x%02x%02x%02x%02x%02x", n->octet[0], n->octet[1], n->octet[2], n->octet[3], n->octet[4],  n->octet[5]);
    }

    if (i < 11)
    {
        return FALSE;
    }
    return TRUE;
}

int macAddrCopy(UI8_T *dest, UI8_T *src)
{
    if (!(((UI32_T)src | (UI32_T)dest ) & 1))
    {
        *(UI16_T *)dest = *(UI16_T *)src;
        *(UI16_T *)(dest + 2) = *(UI16_T *)(src + 2);
        *(UI16_T *)(dest + 4) = *(UI16_T *)(src + 4);
    }
    else
    {
        *dest = *src;
        *(dest + 1 ) = *(src + 1);
        *(dest + 2 ) = *(src + 2);
        *(dest + 3 ) = *(src + 3);
        *(dest + 4 ) = *(src + 4);
        *(dest + 5 ) = *(src + 5);
    }

    return 0;
}

int macAddrCmp(UI8_T *dest, UI8_T *src)
{
    int i;

    for(i=0 ; i<6 ; i++)
    {
        if (*dest++ != *src++)
        {
            return 1;  /*not equal */
        }
    }
    return 0;  /* equal */
}

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
UI32_T strToOui(const C8_T *oui_str, VOVLAN_OUI_T *oui_mac)
{
    UI8_T *ptr, *pMac;
    UI32_T k, cnt=0;

    MW_CHECK_PTR(oui_mac);
    MW_CHECK_PTR(oui_str);

    pMac = (UI8_T*)oui_mac;

    memset(pMac, 0, sizeof(VOVLAN_OUI_T));
    ptr = (UI8_T*)oui_str;

    for ( k = 0 ; *ptr ; ptr ++ )
    {
        if ((*ptr == ' ') || (*ptr == ':') || (*ptr == '-') )
        {
        }
        else if ( ('0' <= *ptr) && (*ptr <= '9') )
        {
            pMac[k] = (pMac[k]<<4) + (*ptr-'0');
            cnt++;
        }
        else if ( ('a' <= *ptr) && (*ptr <= 'f') )
        {
            pMac[k] = (pMac[k] << 4) + (*ptr-'a'+10);
            cnt++;
        }
        else if( ('A' <= *ptr) && (*ptr <= 'F') )
        {
            pMac[k] = (pMac[k]<<4) + (*ptr-'A'+10);
            cnt++;
        }
        else
        {
            break;
        }
        if (cnt >= 2)
        {
            cnt = 0;
            k++;
        }
    }

    return 1;
}
#endif
