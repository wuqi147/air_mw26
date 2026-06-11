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
    {"AN8855M_5P",                    MW_PRODUCT_ID_AN8855M_5P},
    {"AN8855M_5P_1SFP",               MW_PRODUCT_ID_AN8855M_5P_1SFP},
    {"AN8855M_5P_1SFP_A",             MW_PRODUCT_ID_AN8855M_5P_1SFP_A},
#ifdef AIR_EN_AN8801SB_PHY
    {"AN8855M_5P_AN8801SB_1P",        MW_PRODUCT_ID_AN8855M_5P_AN8801SB_1P},
#endif
#ifdef AIR_SUPPORT_POE
    {"AN8855M_5P_AN8502_4P",          MW_PRODUCT_ID_AN8855M_5P_AN8502_4P},
#endif
};

static const MW_LIGHTS_ARRAY_T _mw_product_lights_array[] = {
    {
        MW_PRODUCT_ID_AN8855M_5P,
        "",
        "1,2,3,4,5"
    },
    {
        MW_PRODUCT_ID_AN8855M_5P_1SFP,
        "",
        "1,2,3,4,5,0,106"
    },
    {
        MW_PRODUCT_ID_AN8855M_5P_1SFP_A,
        "",
        "1,2,3,4,5,0,106"
    },
#ifdef AIR_EN_AN8801SB_PHY
    {
        MW_PRODUCT_ID_AN8855M_5P_AN8801SB_1P,
        "",
        "1,2,3,4,5,6"
    },
#endif
#ifdef AIR_SUPPORT_POE
    {
        MW_PRODUCT_ID_AN8855M_5P_AN8502_4P,
        "",
        "1,2,3,4,5"
    },
#endif
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

