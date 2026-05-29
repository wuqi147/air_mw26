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

/* FILE NAME:  html_config_customer.c
 * PURPOSE:
 *    This file contains customer configurations about html pages.
 *
 * NOTES:
 *
 */
/*INCLUDE FILE DECLARATIONS
 */
#include "html_config_customer.h"
#include "osapi_string.h"
#include "httpd_queue.h"
#include "web.h"

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
static MW_ERROR_NO_T
_html_config_get_color_info_by_tlvType(
    UI8_T                       tlv_type,
    MW_WEB_COLOR_INFO_T         *ptr_color_info);

static char
_html_config_send_web_color_response(
    UI8_T   tlv_type,
    I32_T   *ptr_length,
    struct  tcp_pcb *ptr_pcb,
    UI32_T  apiflags);

/* STATIC VARIABLE DECLARATIONS
 */
/* Deafault web color info array */
static MW_WEB_COLOR_INFO_T _mw_web_color_info_array[] = {
    /* Color value(3B: R(1St Byte), G(2nd Byte), B(3rd Byte))               TLV type  */
    {{((MW_WEB_COLOR_TOP_LOGIN_FORM_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_TOP_LOGIN_FORM_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_TOP_LOGIN_FORM_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_TOP_LOGIN_FORM_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_BOTTOM_LOGIN_FORM_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_BOTTOM_LOGIN_FORM_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_BOTTOM_LOGIN_FORM_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_BOTTOM_LOGIN_FORM_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_LOGIN_FORM_BORDER >> 16) & 0xFF),
      ((MW_WEB_COLOR_LOGIN_FORM_BORDER >> 8) & 0xFF),
      (MW_WEB_COLOR_LOGIN_FORM_BORDER & 0xFF)},
                                                                            MW_TLV_TYPE_LOGIN_FORM_BORDER_COLOR},
    {{((MW_WEB_COLOR_LOGIN_FORM_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_LOGIN_FORM_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_LOGIN_FORM_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_LOGIN_FORM_FONT_COLOR},
    {{((MW_WEB_COLOR_TOP_BANNER_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_TOP_BANNER_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_TOP_BANNER_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_TOP_BANNER_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_PORT_PANEL_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_PORT_PANEL_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_PORT_PANEL_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_PORT_PANEL_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_PORT_NUMBER_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_PORT_NUMBER_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_PORT_NUMBER_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_PORT_NUMBER_FONT_COLOR},
    {{((MW_WEB_COLOR_DEVICE_TYPE_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_DEVICE_TYPE_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_DEVICE_TYPE_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_DEVICE_TYPE_FONT_COLOR},
    {{((MW_WEB_COLOR_MENU_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_MENU_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_MENU_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_MENU_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_MENU_TEXT >> 16) & 0xFF),
      ((MW_WEB_COLOR_MENU_TEXT >> 8) & 0xFF),
      (MW_WEB_COLOR_MENU_TEXT & 0xFF)},
                                                                            MW_TLV_TYPE_MENU_TEXT_COLOR},
    {{((MW_WEB_COLOR_MENU_HOVER_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_MENU_HOVER_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_MENU_HOVER_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_MENU_HOVER_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_SELECTED_MENU_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_SELECTED_MENU_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_SELECTED_MENU_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_SELECTED_MENU_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_FONT_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_TITLE_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_TITLE_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_TITLE_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_TITLE_FONT_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_BODER >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_BODER >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_BODER & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_BORDER_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_TABLE_HEADER_BACKGROUND >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_TABLE_HEADER_BACKGROUND >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_TABLE_HEADER_BACKGROUND & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_TABLE_HEADER_BACKGROUND_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_TABLE_BORDER >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_TABLE_BORDER >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_TABLE_BORDER & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_TABLE_BORDER_COLOR},
    {{((MW_WEB_COLOR_SUBPAGE_DIVIDER >> 16) & 0xFF),
      ((MW_WEB_COLOR_SUBPAGE_DIVIDER >> 8) & 0xFF),
      (MW_WEB_COLOR_SUBPAGE_DIVIDER & 0xFF)},
                                                                            MW_TLV_TYPE_SUBPAGE_DIVIDER_COLOR},
    {{((MW_WEB_COLOR_MENU_HOVER_FONT >> 16) & 0xFF),
      ((MW_WEB_COLOR_MENU_HOVER_FONT >> 8) & 0xFF),
      (MW_WEB_COLOR_MENU_HOVER_FONT & 0xFF)},
                                                                            MW_TLV_TYPE_MENU_HOVER_FONT_COLOR},
};

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_html_config_get_color_info_by_tlvType(
    UI8_T                       tlv_type,
    MW_WEB_COLOR_INFO_T         *ptr_color_info)
{
    UI8_T   i = 0;
    UI8_T   len = sizeof(_mw_web_color_info_array) / sizeof(MW_WEB_COLOR_INFO_T);

    if((MW_TLV_TYPE_INVLAID >= tlv_type) || (MW_TLV_TYPE_LAST <= tlv_type) || (NULL == ptr_color_info))
    {
        return MW_E_BAD_PARAMETER;
    }
    for(i = 0; i < len; i++)
    {
        if(tlv_type == _mw_web_color_info_array[i].tlv_type)
        {
            osapi_memcpy(ptr_color_info, &_mw_web_color_info_array[i], sizeof(MW_WEB_COLOR_INFO_T));
            return MW_E_OK;
        }
    }
    CGI_LOG_DEBUG(SYSTEM, "Error: No such color info found, tlv type %d.", tlv_type);
    return MW_E_OTHERS;
}

static char
_html_config_send_web_color_response(
    UI8_T   tlv_type,
    I32_T   *ptr_length,
    struct  tcp_pcb *ptr_pcb,
    UI32_T  apiflags)
{
    char                    err = ERR_OK;
    UI16_T                  len = 0;
    MW_ERROR_NO_T           rc = MW_E_OTHERS;
    MW_WEB_COLOR_INFO_T     color_info;

    if(NULL == ptr_length)
    {
        return ERR_ARG;
    }
    osapi_memset(&color_info, 0, sizeof(MW_WEB_COLOR_INFO_T));
    /* Get Color Info by type */
    rc = _html_config_get_color_info_by_tlvType(tlv_type, &color_info);
    if(MW_E_OK != rc)
    {
        return ERR_INPROGRESS;
    }
    /* expected string: #EBEBEB */
    err = send_format_response(&len, ptr_pcb, apiflags, "#%02x%02x%02x",
                                            color_info.color_value[0],
                                            color_info.color_value[1],
                                            color_info.color_value[2]);
    if(err != ERR_OK)
    {
        return err;
    }
    *ptr_length = len;

    return ERR_OK;
}
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: html_config_init_color_info
 * PURPOSE:
 *      To initilize the color information from the TLV data
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
html_config_init_color_info(
    void)
{
    UI8_T   i = 0;
    UI8_T   len = 0, color_vlaue_len = 0;
    UI32_T  tlv_type_addr = 0;

    if(0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & WEB_COLOR_CUSTOMIZE_ITEM_BITMASK))
    {
        len = sizeof(_mw_web_color_info_array) / sizeof(MW_WEB_COLOR_INFO_T);
        for(i = 0; i < len; i++)
        {
            /* Check if the color value exists and is valid */
            if((MW_E_OK == mw_is_tlv_data_exist(_mw_web_color_info_array[i].tlv_type, &tlv_type_addr)) &&
               (MW_E_OK == mw_read_tlv_data(sizeof(UI8_T), (tlv_type_addr + TLV_DATA_TYPE_SIZE), &color_vlaue_len)) &&
               (TLV_DATA_MAX_COLOR_VALUE_LEN == color_vlaue_len))
            {
                /* Read the color value from the flash */
                mw_read_tlv_data(TLV_DATA_MAX_COLOR_VALUE_LEN, (tlv_type_addr + TLV_DATA_HEADER_SIZE), _mw_web_color_info_array[i].color_value);
            }
        }
    }

    return  MW_E_OK;
}

/* FUNCTION NAME: ssi_get_topLoginFormBackground_info_Handle
 * PURPOSE:
 *      To get the top login form background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_topLoginFormBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_TOP_LOGIN_FORM_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_bottomLoginFormBackground_info_Handle
 * PURPOSE:
 *      To get the bottom login form background color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_bottomLoginFormBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_BOTTOM_LOGIN_FORM_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_loginFormBorder_info_Handle
 * PURPOSE:
 *      To get the login form border color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_loginFormBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_LOGIN_FORM_BORDER_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_loginFormFont_info_Handle
 * PURPOSE:
 *      To get the login form font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_loginFormFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_LOGIN_FORM_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_topBannerBackground_info_Handle
 * PURPOSE:
 *      To get the top banner background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_topBannerBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_TOP_BANNER_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_portPanelBackground_info_Handle
 * PURPOSE:
 *      To get the port panel background info  from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_portPanelBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_PORT_PANEL_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_portNumberFont_info_Handle
 * PURPOSE:
 *      To get the port number font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_portNumberFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_PORT_NUMBER_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_deviceTypeFont_info_Handle
 * PURPOSE:
 *      To get the device type font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_deviceTypeFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_DEVICE_TYPE_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_menuBackground_info_Handle
 * PURPOSE:
 *      To get the menu background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_MENU_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_menuText_info_Handle
 * PURPOSE:
 *      To get the menu text color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuText_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_MENU_TEXT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_menuHoverBackground_info_Handle
 * PURPOSE:
 *      To get the menu hover background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuHoverBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_MENU_HOVER_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_selectedMenuBackground_info_Handle
 * PURPOSE:
 *      To get the selected menu background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_selectedMenuBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SELECTED_MENU_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageBackground_info_Handle
 * PURPOSE:
 *      To get the sub page background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageFont_info_Handle
 * PURPOSE:
 *      To get the sub page font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageFunctionGroupTitleFont_info_Handle
 * PURPOSE:
 *      To get the sub page function group title font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFunctionGroupTitleFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_TITLE_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageFunctionGroupBorder_info_Handle
 * PURPOSE:
 *      To get the sub page function group border info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFunctionGroupBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_FUNCTION_GROUP_BORDER_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageTableHeaderBackground_info_Handle
 * PURPOSE:
 *      To get the sub page table header background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageTableHeaderBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_TABLE_HEADER_BACKGROUND_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageTableBorder_info_Handle
 * PURPOSE:
 *      To get the sub page table border info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageTableBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_TABLE_BORDER_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_subPageDivider_info_Handle
 * PURPOSE:
 *      To get the sub page divider info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageDivider_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_SUBPAGE_DIVIDER_COLOR, ptr_length, ptr_pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_menuHoverFont_info_Handle
 * PURPOSE:
 *      To get the menu hover font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuHoverFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    return _html_config_send_web_color_response(MW_TLV_TYPE_MENU_HOVER_FONT_COLOR, ptr_length, ptr_pcb, apiflags);
}

/*FUNCTION NAME: ssi_get_customer_homepage_logo_Handle
 * PURPOSE:
 *      Display the LOGO on the homepage.
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *      The file path of the LOGO is air_magic_wand\air_mw_system\html\webpage\lwip_http_fs\logo.png.
 */
MW_ERROR_NO_T
ssi_get_customer_homepage_logo_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    C8_T     err = ERR_OK;
    C8_T     *ptr_web_addr = NULL;
    C8_T     space_str[] = {32, '\0'};
    UI32_T   tlv_type_addr_webAddr = 0;
    UI32_T   logo_addr = 0x0;
    UI16_T   logo_len = 0;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        ptr_web_addr = (C8_T *) CUSTOMER_HOMEPAGE_LOGO_HREF;
    }
    else if(0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        ptr_web_addr = (C8_T *) space_str;
        /* Check if there is a webaddress in flash */
        if(MW_E_OK == mw_is_tlv_data_exist(MW_TLV_TYPE_WEB_ADDRESS, &tlv_type_addr_webAddr))
        {
            ptr_web_addr = (C8_T *) (tlv_type_addr_webAddr + TLV_DATA_HEADER_SIZE);
        }
    }

    if(((0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK)) ||
        ((0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK)) &&
         (MW_E_OK == mw_tlv_get_logo_info(&logo_addr, &logo_len)))) &&
       (NULL != ptr_web_addr))
    {
        const char *ptr_sendBuff = "<script>document.getElementById(\"company\").href= \"%s\";</script><img src=\"./logo.png\" border=\"0\" alt=\"logo\">\n";

        CGI_LOG_DEBUG(SYSTEM, "homepage logo webaddr:%s", ptr_web_addr);
        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff, ptr_web_addr);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;

    return err;
}

/*FUNCTION NAME: ssi_get_customer_addrbar_favicon_Handle
 * PURPOSE:
 *      Customize the icon displayed on the address bar of the browser.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *       The file path of the icon is air_magic_wand\air_mw_system\html\webpage\lwip_http_fs\favicon.ico.
 */
MW_ERROR_NO_T
ssi_get_customer_addrbar_favicon_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    char err = 0;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        const char *ptr_sendBuff = "<script>document.getElementById(\"web_ico\").href=\"/favicon.ico\";</script>";

        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;
    return err;
}

/*FUNCTION NAME: ssi_get_customer_addrbar_production_version_Handle
 * PURPOSE:
 *      Display the product version on the address bar of the browser.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *       Modify CUSTOMER_PRODUCT_VERSION to customize the production version.
 */
MW_ERROR_NO_T
ssi_get_customer_addrbar_production_version_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    char err = 0;
    C8_T    *ptr_device_type_addr = NULL;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        ptr_device_type_addr = (C8_T *) CUSTOMER_PRODUCT_VERSION;
    }
    else if(0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        /* Check TLV data Device Type is exist */
        UI32_T  tlv_type_addr_devType = 0;

        if(MW_E_OK == mw_is_tlv_data_exist(MW_TLV_TYPE_DEVICE_TYPE, &tlv_type_addr_devType))
        {
            ptr_device_type_addr = (C8_T *) (tlv_type_addr_devType + TLV_DATA_HEADER_SIZE);
            CGI_LOG_DEBUG(SYSTEM, "Device type is: %s", ptr_device_type_addr);
        }
    }

    if(NULL != ptr_device_type_addr)
    {
        const char *ptr_sendBuff = "<script>document.title = \"%s\";</script>";

        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff, ptr_device_type_addr);
        if(err != ERR_OK)
        {
            return err;
        }
    }

    *ptr_length = len;

    return err;
}

/*FUNCTION NAME: ssi_get_customer_login_logo_Handle
 * PURPOSE:
 *      Display the LOGO on the login webpage.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *       The file path of the LOGO is air_magic_wand\air_mw_system\html\webpage\lwip_http_fs\logo.png.
 */
MW_ERROR_NO_T
ssi_get_customer_login_logo_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T      len = 0;
    char        err = 0;
    UI32_T      logo_addr = 0x0;
    UI16_T      logo_len = 0;

    if((0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK)) ||
       ((0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK)) &&
        (MW_E_OK == mw_tlv_get_logo_info(&logo_addr, &logo_len))))
    {
        const char *ptr_sendBuff = "banner = \'<div class=\"TOP\"><IMG src=\"logo.png\" id=\"img_logo\" align=\"center\"></IMG></DIV>\';";
        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;
    return err;
}
/*FUNCTION NAME: ssi_get_customer_login_copyright_Handle
 * PURPOSE:
 *      Display the copyright string on the login webpage.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *       None
 */
MW_ERROR_NO_T
ssi_get_customer_login_copyright_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    char err = 0;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        const char *ptr_sendBuff = "<span id=\"t_copyright\"></span><span class=\"COPY\">&nbsp;©&nbsp;</span><span id=\"t_corporation\"></span>" \
                            "<script>$id(\"t_copyright\").innerHTML=str_info.spn_copyright; " \
                            "$id(\"t_corporation\").innerHTML=str_info.spn_copyrightyear+\" \"+str_info.spn_copyrightinfo;</script>";

        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;

    return err;
}

/*FUNCTION NAME: ssi_get_customer_homepage_product_version_Handle
 * PURPOSE:
 *      Display the product version on the homepage.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *       Modify CUSTOMER_PRODUCT_VERSION to customize the production version.
 */
MW_ERROR_NO_T
ssi_get_customer_homepage_product_version_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    char err = 0;
    C8_T   *ptr_device_type_addr = NULL;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        ptr_device_type_addr = (C8_T *) CUSTOMER_PRODUCT_VERSION;
    }
    else if(0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        /* Check TLV data Device Type is exist */
        UI32_T  tlv_type_addr_devType = 0;

        if(MW_E_OK == mw_is_tlv_data_exist(MW_TLV_TYPE_DEVICE_TYPE, &tlv_type_addr_devType))
        {
            ptr_device_type_addr = (C8_T *) (tlv_type_addr_devType + TLV_DATA_HEADER_SIZE);
            CGI_LOG_DEBUG(SYSTEM, "Device type is: %s", ptr_device_type_addr);
        }
    }

    if(NULL != ptr_device_type_addr)
    {
        const char *ptr_sendBuff = "<script>document.write(\"<span>\" + \"%s\" + \"</span>\");</script>";

        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff, ptr_device_type_addr);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;

    return err;
}

/*FUNCTION NAME: ssi_get_customer_menu_copyright_Handle
 * PURPOSE:
 *      Display the copyright string on the homepage.
 * INPUT:
 *      ptr_pcb              -- the pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *      ERR_VAL
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ssi_get_customer_menu_copyright_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI16_T len = 0;
    char err = 0;

    if(0 != (CUSTOMIZE_ITEM_LOAD_DEFAULT & HTML_CUSTOMIZE_ITEM_BITMASK))
    {
        const char *ptr_sendBuff = "<span id=\"t_copyright\" class=\"copyright\"></span><span class=\"copy\">&nbsp;©&nbsp;</span>" \
                            "<span id=\"t_corporation\" class=\"copyright\"></span>" \
                            "<script>var str_info = parent.window.str_info; $id(\"t_copyright\").innerHTML=str_info.spn_copyright; " \
                            "$id(\"t_corporation\").innerHTML=str_info.spn_copyrightyear+\" \"+str_info.spn_copyrightinfo;</script>";

        err = send_format_response(&len, ptr_pcb, apiflags, ptr_sendBuff);
        if(err != ERR_OK)
        {
            return err;
        }
    }
    *ptr_length = len;

    return err;
}