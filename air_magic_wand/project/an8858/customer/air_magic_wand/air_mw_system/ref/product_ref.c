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

/* FILE NAME:   product_ref.c
 * PURPOSE:
 *       Define the arrangement of icons for web page ports.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "product_ref.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static const MW_PRODUCT_ID_INFO_T _mw_product_id_array[] = {
    /* Description                    Product_ID */
    {"AN8858E_8P",                    MW_PRODUCT_ID_AN8858E_8P},
#ifdef AIR_SUPPORT_POE
    {"AN8858E_8P_AN8503_8P",          MW_PRODUCT_ID_AN8858E_8P_AN8503_8P},
#endif
    {"AN8858C_8P_2SFP",               MW_PRODUCT_ID_AN8858C_8P_2SFP},
    {"AN8858C_8P_2SFP_A",             MW_PRODUCT_ID_AN8858C_8P_2SFP_A},
    {"AN8858B_6P_2SFP",               MW_PRODUCT_ID_AN8858B_6P_2SFP},
    {"AN8858B_6P_2SFP_A",             MW_PRODUCT_ID_AN8858B_6P_2SFP_A},
    {"AN8858H_8P_AN8811B_1P",         MW_PRODUCT_ID_AN8858H_8P_AN8811B_1P},
};

static const MW_LIGHTS_ARRAY_T _mw_product_lights_array[] = {
    {
        MW_PRODUCT_ID_AN8858E_8P,
        "",
        "1,2,3,4,5,6,7,8"
    },
    {
        MW_PRODUCT_ID_AN8858E_8P_AN8503_8P,
        "",
        "1,2,3,4,5,6,7,8"
    },
    {
        MW_PRODUCT_ID_AN8858C_8P_2SFP,
        "",
        "1,2,3,4,5,6,7,8,0,109,110"
    },
    {
        MW_PRODUCT_ID_AN8858C_8P_2SFP_A,
        "",
        "1,2,3,4,5,6,7,8,0,109,110"
    },
    {
        MW_PRODUCT_ID_AN8858B_6P_2SFP,
        "",
        "1,2,3,4,5,6,0,107,108"
    },
    {
        MW_PRODUCT_ID_AN8858B_6P_2SFP_A,
        "",
        "1,2,3,4,5,6,0,107,108"
    },
    {
        MW_PRODUCT_ID_AN8858H_8P_AN8811B_1P,
        "",
        "9,0,1,2,3,4,5,6,7,8"
    },
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
const MW_PRODUCT_ID_INFO_T *
mw_product_getProductIdArray(
    void)
{
    return _mw_product_id_array;
}

UI32_T
mw_product_getProductIdArraySize(
    void)
{
    return sizeof(_mw_product_id_array) / sizeof(MW_PRODUCT_ID_INFO_T);
}

const MW_LIGHTS_ARRAY_T *
mw_product_getLightsArray(
    void)
{
    return _mw_product_lights_array;
}

UI32_T
mw_product_getLightsArraySize(
    void)
{
    return sizeof(_mw_product_lights_array) / sizeof(MW_LIGHTS_ARRAY_T);
}

