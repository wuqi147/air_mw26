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

/* FILE NAME:   mw_utils.c
 * PURPOSE:
 *       Define the initialization in AIR Magic Wand system.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "mw_tlv.h"
#include "osapi_string.h"
#include "sys_mgmt.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
MW_PRODUCT_ID_T _mw_product_id = MW_PRODUCT_ID_DEFAULT;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
UI8_T *
put32(UI8_T *cp, UI32_T x)
{
    *cp++ = x >> 24;
    *cp++ = x >> 16;
    *cp++ = x >> 8 ;
    *cp++ = x;

    return cp;
}

UI32_T get32(UI8_T *cp)
{
    UI32_T rval;

    rval = *cp++;
    rval <<= 8;
    rval |= *cp++;
    rval <<= 8;
    rval |= *cp++;
    rval <<= 8;
    rval |= *cp++;

    return rval;
}

UI8_T *
put16(UI8_T *cp, UI16_T x)
{
    *cp++ = x >> 8 ;
    *cp++ = x;

    return cp;
}

UI16_T get16(UI8_T *cp)
{
    UI16_T rval;

    rval = *cp++;
    rval <<= 8;
    rval |= *cp++;

    return rval;
}

C8_T *_printUI64_T(UI64_T value, C8_T *varStr, UI16_T strLen)
{
    UI64_T u64_tmp;
    UI32_T u32_H, u32_M, u32_L, len = 0;

    u32_H = (value) / 10000000000;
    u64_tmp = (value) % 10000000000;
    u32_M = u64_tmp / 100;
    u32_L = u64_tmp % 100;

    if (0 != u32_H)
    {
        len += snprintf(varStr + len, (strLen - len), "%u", u32_H);
        len += snprintf(varStr + len, (strLen - len), "%08u", u32_M);
        len += snprintf(varStr + len, (strLen - len), "%02u", u32_L);
    }
    else
    {
        if (0 != u32_M)
        {
            len += snprintf(varStr + len, (strLen - len), "%u", u32_M);
            len += snprintf(varStr + len, (strLen - len), "%02u", u32_L);
        }
        else
        {
            len += snprintf(varStr + len, (strLen - len), "%u", u32_L);
        }
    }
    return varStr;
}

MW_ERROR_NO_T
mw_get_version(C8_T *ptr_str)
{
    int len = 0;

    MW_CHECK_PTR(ptr_str);
    len = osapi_sprintf(ptr_str, MW_VER_STR);
    if (0 >= len)
    {
        return MW_E_BAD_PARAMETER;
    }
    return MW_E_OK;
}

MW_ERROR_NO_T
mw_pbuf_init(struct pbuf *ptr_pbuf)
{
    MW_CHECK_PTR(ptr_pbuf);
    ptr_pbuf->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
    ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = 0;

    ptr_pbuf->vlan_num = 1;
    ptr_pbuf->vlan_hdr.tpid = 0x8100;
    ptr_pbuf->vlan_hdr.priority = 7;
    ptr_pbuf->vlan_hdr.vid = 1;
    return MW_E_OK;
}

MW_ERROR_NO_T
mw_get_lights_array(
    UI8_T *ptr_up,
    UI8_T *ptr_dwn)
{
    UI8_T i = 0;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T tlv_port_up_addr = 0;
    UI32_T tlv_port_dwn_addr = 0;
    UI8_T tlv_port_up_len = 0;
    UI8_T tlv_port_dwn_len = 0;
    UI32_T light_array_size = mw_product_getLightsArraySize();
    const MW_LIGHTS_ARRAY_T *ptr_lightArray = mw_product_getLightsArray();

    ret = mw_is_tlv_data_exist(MW_TLV_TYPE_UP_PORT_LAYOUT, &tlv_port_up_addr);
    if(MW_E_OK == ret)
    {
        mw_read_tlv_data(sizeof(UI8_T), (tlv_port_up_addr + TLV_DATA_TYPE_SIZE), (void*)&tlv_port_up_len);
        osapi_memcpy(ptr_up, (C8_T*)(tlv_port_up_addr + TLV_DATA_HEADER_SIZE), tlv_port_up_len);
    }
    ret = mw_is_tlv_data_exist(MW_TLV_TYPE_DWN_PORT_LAYOUT, &tlv_port_dwn_addr);
    if(MW_E_OK == ret)
    {
        mw_read_tlv_data(sizeof(UI8_T), (tlv_port_dwn_addr + TLV_DATA_TYPE_SIZE), (void*)&tlv_port_dwn_len);
        osapi_memcpy(ptr_dwn, (C8_T*)(tlv_port_dwn_addr + TLV_DATA_HEADER_SIZE), tlv_port_dwn_len);
    }

    if(((0 == tlv_port_up_addr) && (0 == tlv_port_dwn_addr)) || ((0 == tlv_port_up_len) && (0 == tlv_port_dwn_len)))
    {
        /* Can't find TLV of port panel, use layout based on product ID */
        for(i = 0; i < light_array_size; i++)
        {
            if(_mw_product_id == ptr_lightArray[i].productId)
            {
                osapi_memcpy(ptr_up, ptr_lightArray[i].up_row, strlen(ptr_lightArray[i].up_row));
                osapi_memcpy(ptr_dwn, ptr_lightArray[i].dwn_row, strlen(ptr_lightArray[i].dwn_row));
                ret = MW_E_OK;
                break;
            }
        }
    }

    return ret;
}

MW_PRODUCT_ID_T
mw_get_productID(
    void)
{
    return _mw_product_id;
}

void
mw_set_productID(
    MW_PRODUCT_ID_T product_id)
{
    _mw_product_id = product_id;
}

#ifdef AIR_SUPPORT_SNTP
void
mw_timeToDate(
    UI64_T time,
    MW_DATE_T *ptr_st_date)
{
    UI32_T a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;

    ptr_st_date->seconds = time % 60;
    time = time/60;
    ptr_st_date->minutes = time % 60;
    time = time/60;
    ptr_st_date->hours = time % 24;
    time = time/24;

    a = (UI32_T)(((4 * time) + 102032) / 146097 + 15);
    b = (UI32_T)(time + 2442113 + a - (a / 4));
    c = (20 * b - 2442) / 7305;
    d = b - 365 * c - (c / 4);
    e = d * 1000 / 30601;
    f = d - e * 30 - e * 601 / 1000;

    if(e <= 13)
    {
        c -= 4716;
        e -= 1;
    }
    else
    {
        c -= 4715;
        e -= 13;
    }

    ptr_st_date->year = c;
    ptr_st_date->month = e;
    ptr_st_date->day = f;
}

UI64_T
mw_dateToTime(
    MW_DATE_T st_date)
{
    UI16_T y = 0, m = 0, d = 0;
    UI64_T time = 0;

    y = st_date.year;
    m = st_date.month;
    d = st_date.day;

    if(m <= 2)
    {
        m += 12;
        y -= 1;
    }
    time = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    time += (30 * m) + (3 * (m + 1) / 5) + d;
    time -= 719561;
    time *= 86400;
    time += (3600 * st_date.hours) + (60 * st_date.minutes) + st_date.seconds;

    return time;
}
#endif
