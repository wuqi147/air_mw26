/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:   product.c
 * PURPOSE:
 *       Implement the interfaces related to the product settings.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "mw_error.h"
#include "mw_tlv.h"
#include "product.h"
#include "mw_log.h"

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

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
const MW_PRODUCT_ID_T
mw_product_getProductId(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;
    UI8_T tlv_value_len = 0, *ptr_productIDStr = NULL;
    UI32_T tlv_type_addr = 0, i = 0, count = mw_product_getProductIdArraySize(), product_id_str_len = 0;
    const MW_PRODUCT_ID_INFO_T *ptr_productInfo = mw_product_getProductIdArray();

    ret = mw_is_tlv_data_exist(MW_TLV_TYPE_PRODUCT_ID, &tlv_type_addr);
    if (MW_E_OK == ret)
    {
        ret = mw_read_tlv_data(sizeof(UI8_T), (tlv_type_addr + TLV_DATA_TYPE_SIZE), (void *)&tlv_value_len);
    }
    if ((MW_E_OK == ret) && (0 != tlv_value_len) && (TLV_DATA_PRODUCT_ID_VALUE_SIZE >= tlv_value_len))
    {
        ptr_productIDStr = (UI8_T *)(tlv_type_addr + TLV_DATA_HEADER_SIZE);
        product_id_str_len = osapi_strlen((const C8_T *)ptr_productIDStr);
        if (tlv_value_len != (product_id_str_len + 1))
        {
            MW_LOG_CONSOLE_PRINTF("Error: TLV length:%d is not equal to PRODUCT ID string length:%d\n", tlv_value_len, product_id_str_len);
            ptr_productIDStr = NULL;
        }
    }

    if (NULL != ptr_productIDStr)
    {
        MW_LOG_CONSOLE_PRINTF("Initializing PRODUCT_ID: %s, count: %d ...\n", ptr_productIDStr, count);

        for (i = 0; i < count; i++)
        {
            if ((osapi_strlen(ptr_productInfo[i].descp) == product_id_str_len) &&
                (0 == osapi_strncmp((const C8_T *)ptr_productIDStr, ptr_productInfo[i].descp, product_id_str_len)))
            {
                MW_LOG_CONSOLE_PRINTF("[PRODUCT_ID] Find %s(%d)\n", ptr_productIDStr, ptr_productInfo[i].id);
                return ptr_productInfo[i].id;
            }
        }
    }

    MW_LOG_CONSOLE_PRINTF("Warning: Use default PRODUCT_ID:%d\n", MW_PRODUCT_ID_DEFAULT);
    return MW_PRODUCT_ID_DEFAULT;
}

