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
/* FILE NAME:  mw_tlv.h
 * PURPOSE:
 *      This file defines the data structure for TLV.
 *
 * NOTES:
 */
#ifndef MW_TLV_H
#define MW_TLV_H
/* INCLUDE FILE DECLARATIONS
 */
#include <mw_utils.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define TLV_DATA_TYPE_SIZE              (1)
#define TLV_DATA_LENGTH_SIZE            (1)
#define TLV_DATA_HEADER_SIZE            (TLV_DATA_TYPE_SIZE + TLV_DATA_LENGTH_SIZE)
#define TLV_DATA_PRODUCT_ID_VALUE_SIZE  (64)
#define TLV_DATA_LOGO_NAME              "/logo.png"
#define TLV_DATA_LOGO_NAME_HEX_SIZE     (12)
#define TLV_DATA_MAX_COLOR_VALUE_LEN    (3)                 /* Maximum length of color value string */
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    MW_TLV_TYPE_INVLAID = 0,
    MW_TLV_TYPE_PRODUCT_ID,                                 /* Product ID */
    MW_TLV_TYPE_SERIAL_NUMBER,                              /* Serial Number */
    MW_TLV_TYPE_DEVICE_TYPE,                                /* Device Type */
    MW_TLV_TYPE_HW_VERSION,                                 /* Hardware Version */
    MW_TLV_TYPE_IP_ADDRESS,                                 /* IP Address */
    MW_TLV_TYPE_SUBNET_MASK,                                /* Subnet Mask */
    MW_TLV_TYPE_GATEWAY,                                    /* Gateway */
    MW_TLV_TYPE_WEB_ADDRESS,                                /* The hyperlink of the Logo */
    MW_TLV_TYPE_LANGUAGE,                                   /* The System Language */
    MW_TLV_TYPE_LOGO_ADDRESS,                               /* The Logo storage Address */
    MW_TLV_TYPE_LOGO_LENGTH,                                /* The Logo storage Length */
    MW_TLV_TYPE_UP_PORT_LAYOUT,                             /* Arrangement of ports on the top row */
    MW_TLV_TYPE_DWN_PORT_LAYOUT,                            /* Arrangement of ports on the bottom row */
    MW_TLV_TYPE_TOP_LOGIN_FORM_BACKGROUND_COLOR,            /* Background color of top login form */
    MW_TLV_TYPE_BOTTOM_LOGIN_FORM_BACKGROUND_COLOR,         /* Background color of bottom login form */
    MW_TLV_TYPE_LOGIN_FORM_BORDER_COLOR,                    /* Border color of login forms */
    MW_TLV_TYPE_LOGIN_FORM_FONT_COLOR,                      /* Font color of login forms */
    MW_TLV_TYPE_TOP_BANNER_BACKGROUND_COLOR,                /* Background color of top banner */
    MW_TLV_TYPE_PORT_PANEL_BACKGROUND_COLOR,                /* Background color of port panel */
    MW_TLV_TYPE_PORT_NUMBER_FONT_COLOR,                     /* Font color of port number */
    MW_TLV_TYPE_DEVICE_TYPE_FONT_COLOR,                     /* Font color of device type */
    MW_TLV_TYPE_MENU_BACKGROUND_COLOR,                      /* Background color of menu bar */
    MW_TLV_TYPE_MENU_TEXT_COLOR,                            /* Text color of menu items */
    MW_TLV_TYPE_MENU_HOVER_BACKGROUND_COLOR,                /* Background color of hovered menu item */
    MW_TLV_TYPE_SELECTED_MENU_BACKGROUND_COLOR,             /* Background color of selected menu */
    MW_TLV_TYPE_SUBPAGE_BACKGROUND_COLOR,                   /* Background color of sub-pages */
    MW_TLV_TYPE_SUBPAGE_FONT_COLOR,                         /* Font color of sub-pages */
    MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_TITLE_FONT_COLOR,    /* Font color of function group title */
    MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_BORDER_COLOR,        /* Border color of function groups */
    MW_TLV_TYPE_SUBPAGE_TABLE_HEADER_BACKGROUND_COLOR,      /* Background color of table header */
    MW_TLV_TYPE_SUBPAGE_TABLE_BORDER_COLOR,                 /* Border color of tables */
    MW_TLV_TYPE_SUBPAGE_DIVIDER_COLOR,                      /* Color of dividers */
    MW_TLV_TYPE_MENU_HOVER_FONT_COLOR,                      /* Font of hovered menu item */

    MW_TLV_TYPE_POE_ENABLE = 50,                            /* POE enabled or not */
    MW_TLV_TYPE_POE_TOTAL_POWER,                            /* POE total available power */

    MW_TLV_TYPE_LAST
}MW_TLV_DATA_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    UI32_T      *ptr_tlv_addr);

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
    void        *ptr_out_data);

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
    UI16_T      *ptr_tlv_logo_len);

#endif  /* MW_TLV_H */
