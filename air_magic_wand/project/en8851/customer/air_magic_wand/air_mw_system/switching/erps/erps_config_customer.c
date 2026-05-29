/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:   erps_config_customer.c
 * PURPOSE:
 *      Configure ERPS.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "erps_config_customer.h"
#include "mw_utils.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T product_id;
    const UI8_T *ptr_portSettings;
    UI8_T port_count;
} ERPS_CONFIG_PORT_PRODUCT_ID_MAP_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */


/* EN8851C RFB example */
static const UI8_T _erps_config_port_settings_en8851c_rfb[] = {
/*   PORT_NUM */
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
#ifdef AIR_SUPPORT_SFP
    9,
    10,
#endif
};


/* EN8853C RFB example */
static const UI8_T _erps_config_port_settings_en8853c_rfb[] = {
/*   PORT_NUM */
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
#ifdef AIR_SUPPORT_SFP
    25,
    26,
    27,
    28,
#endif
};

static const ERPS_CONFIG_PORT_PRODUCT_ID_MAP_T _erps_config_port_product_id_map[] = {
    {MW_PRODUCT_ID_EN8851C_RFB, _erps_config_port_settings_en8851c_rfb, (sizeof(_erps_config_port_settings_en8851c_rfb) / sizeof(UI8_T))},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8502_4P, _erps_config_port_settings_en8851c_rfb, (sizeof(_erps_config_port_settings_en8851c_rfb) / sizeof(UI8_T))},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8503_8P, _erps_config_port_settings_en8851c_rfb, (sizeof(_erps_config_port_settings_en8851c_rfb) / sizeof(UI8_T))},
    {MW_PRODUCT_ID_EN8853C_RFB, _erps_config_port_settings_en8853c_rfb, (sizeof(_erps_config_port_settings_en8853c_rfb) / sizeof(UI8_T))},
    {MW_PRODUCT_ID_EN8853C_AN8808Q_RFB, _erps_config_port_settings_en8853c_rfb, (sizeof(_erps_config_port_settings_en8853c_rfb) / sizeof(UI8_T))},
};

static const UI8_T *_erps_port_settings_array = NULL;
static UI8_T _erps_port_settings_array_size = 0;

/* FUNCTION NAME:   erps_config_getPortSettingsArray
* PURPOSE:
*      Get the port settings array.
*
* INPUT:
*      None
* OUTPUT:
*      None
* RETURN:
*      A pointer which points to the port settings array
*
* NOTES:
*      None
*/
const UI8_T *
erps_config_getPortSettingsArray(
    void)
{
    static UI8_T inited = FALSE;

    if (FALSE == inited)
    {
        MW_PRODUCT_ID_T product_id = mw_get_productID();
        UI32_T i = 0, count = (sizeof(_erps_config_port_product_id_map) / sizeof(ERPS_CONFIG_PORT_PRODUCT_ID_MAP_T));

        inited = TRUE;
        for (; i < count; i++)
        {
            if (product_id == _erps_config_port_product_id_map[i].product_id)
            {
                _erps_port_settings_array = _erps_config_port_product_id_map[i].ptr_portSettings;
                _erps_port_settings_array_size = _erps_config_port_product_id_map[i].port_count;
                break;
            }
        }
    }
    return _erps_port_settings_array;
}

/* FUNCTION NAME:   erps_config_getPortSettingsArraySize
* PURPOSE:
*      Get the size of the port settings array.
*
* INPUT:
*      None
* OUTPUT:
*      None
* RETURN:
*      The size of the port settings array
*
* NOTES:
*      None
*/
UI32_T
erps_config_getPortSettingsArraySize(
    void)
{
    if (NULL == _erps_port_settings_array)
    {
        /* Init first */
        erps_config_getPortSettingsArray();
    }

    return _erps_port_settings_array_size;
}

