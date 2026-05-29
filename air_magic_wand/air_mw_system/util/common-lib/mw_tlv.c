/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2024
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

/* FILE NAME:  mw_tlv.c
 * PURPOSE:
 *    This file contains the implementation of the TLV APIs.
 *
 * NOTES:
 *
 */
/*INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "mw_tlv.h"
#include <platform.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_is_tlv_data_exist()
 * PURPOSE:
 *      Check if the TLV data of given TLV type exist in the flash.
 *
 * INPUT:
 *      type                 -- The TLV type to check
 *
 * OUTPUT:
 *      ptr_tlv_addr         -- The Pointer of tlv type address
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_ENTRY_REACH_END
 *      MW_E_OK
 *
 * NOTES:
 *      If return MW_E_OK, will output the starting address(UI32_T) of a tlv data.
 */
MW_ERROR_NO_T
mw_is_tlv_data_exist(
    UI8_T       type,
    UI32_T      *ptr_tlv_addr)
{
    UI8_T  tlv_type = 0;
    UI8_T  tlv_data_len = 0;
    UI32_T tlv_type_addr = 0;

    if((MW_TLV_TYPE_INVLAID >= type) || (MW_TLV_TYPE_LAST <= type) || (NULL == ptr_tlv_addr))
    {
        return MW_E_BAD_PARAMETER;
    }
    *ptr_tlv_addr = 0;
    /* ManufactureBase Layout: [MAC Address(6B)][MAC Enable(1B)][ACE Enable(1B)][TLV data]...[TLV data] */
    tlv_type_addr = (UI32_T)(ManufactureBase + sizeof(flash_mac_t));
    while ((ManufactureBase + ManufactureSize) > (tlv_type_addr + TLV_DATA_HEADER_SIZE + 1))
    {
        tlv_type = *(UI8_T *)(tlv_type_addr);
        if((MW_TLV_TYPE_INVLAID < tlv_type) && (MW_TLV_TYPE_LAST > tlv_type))
        {
            if(type == tlv_type)
            {
                *ptr_tlv_addr = tlv_type_addr;
                return MW_E_OK;
            }
            else
            {
                tlv_data_len = *(UI8_T *)(tlv_type_addr + TLV_DATA_TYPE_SIZE);
                if(0 != tlv_data_len)
                {
                    /* Check next tlv data */
                    tlv_type_addr += (TLV_DATA_HEADER_SIZE + tlv_data_len);
                    continue;
                }
                else
                {
                    /* No more tlv data */
                    return MW_E_ENTRY_NOT_FOUND;
                }
            }
        }
        else
        {
            /* No more tlv data */
            return MW_E_ENTRY_NOT_FOUND;
        }
    }
    return MW_E_ENTRY_REACH_END;
}

/* FUNCTION NAME: mw_read_tlv_data()
 * PURPOSE:
 *      This function reads the TLV data from the given flash address and returns the data in the given buffer.
 *
 * INPUT:
 *      length               -- The length of the TLV data to read
 *      tlv_data_addr        -- The flash address of the TLV data
 *      ptr_out_data         -- The buffer to store the TLV data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_read_tlv_data(
    UI8_T       length,
    UI32_T      tlv_data_addr,
    void        *ptr_out_data)
{
    UI32_T  read_offset = 0;

    if((0 == length) ||
       (NULL == ptr_out_data) ||
       ((ManufactureBase + sizeof(flash_mac_t)) > tlv_data_addr) ||
       ((ManufactureBase + ManufactureSize) < (tlv_data_addr + length)))
    {
        return MW_E_BAD_PARAMETER;
    }

    for(read_offset = 0; read_offset < length; read_offset++)
    {
        *((UI8_T *)ptr_out_data + read_offset) = *(UI8_T *)(tlv_data_addr + read_offset);
    }
    return MW_E_OK;
}

/* FUNCTION NAME: mw_tlv_get_logo_info()
 * PURPOSE:
 *      Check if the logo exist in the flash.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_tlv_logo_addr    -- The logo address in the flash
 *      ptr_tlv_logo_len     -- The fs data size of logo in the flash
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_tlv_get_logo_info(
    UI32_T      *ptr_tlv_logo_addr,
    UI16_T      *ptr_tlv_logo_len)
{
    UI32_T   logo_addr = 0x0;
    UI16_T   logo_len = 0;
    UI32_T   tlv_type_addr_logoAddr = 0, tlv_type_addr_logoLen = 0;

    if((NULL == ptr_tlv_logo_addr) || (NULL == ptr_tlv_logo_len))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Check if there is a logo in flash */
    if((MW_E_OK != mw_is_tlv_data_exist(MW_TLV_TYPE_LOGO_ADDRESS, &tlv_type_addr_logoAddr)) ||
        (MW_E_OK != mw_is_tlv_data_exist(MW_TLV_TYPE_LOGO_LENGTH, &tlv_type_addr_logoLen)))
    {
        return MW_E_ENTRY_NOT_FOUND;
    }
    mw_read_tlv_data(sizeof(UI32_T), (tlv_type_addr_logoAddr + TLV_DATA_HEADER_SIZE), &logo_addr);
    mw_read_tlv_data(sizeof(UI16_T), (tlv_type_addr_logoLen + TLV_DATA_HEADER_SIZE), &logo_len);
    if((0 != logo_addr) && (0 != logo_len))
    {
        *ptr_tlv_logo_addr = logo_addr;
        *ptr_tlv_logo_len = logo_len;
        return MW_E_OK;
    }
    return MW_E_ENTRY_NOT_FOUND;
}
