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
/* FILE NAME:  html_config_customer.h
 * PURPOSE:
 *      It provides defines of customized UI elements.
 *
 * NOTES:
 */
#ifndef HTML_CONFIG_CUSTOMER_H
#define HTML_CONFIG_CUSTOMER_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_tlv.h"
#include "web.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CUSTOMIZE_ITEM_LOAD_DISABLED                    (0)          /* Do not load customized items, such as logo, favicon, webpage color, copyright, etc. */
#define CUSTOMIZE_ITEM_LOAD_DEFAULT                     BIT(0)       /* Load default config for all customized items, such as logo favicon, webpage color, copyright, etc. */
#define CUSTOMIZE_ITEM_LOAD_TLV_DATA                    BIT(1)       /* Load TLV data config for all customized items, such as logo favicon, webpage color, etc. */

/* Default not load customized items(logo, favicon, production version, copyright, etc.),
 * can be configured to 'CUSTOMIZE_ITEM_LOAD_DEFAULT' to load default config
 * or 'CUSTOMIZE_ITEM_LOAD_TLV_DATA' to load TLV data config */
#define HTML_CUSTOMIZE_ITEM_BITMASK                     CUSTOMIZE_ITEM_LOAD_DISABLED

/* Default load default color configurations(login form, bottom login form, port panel, etc.),
 * can be configured to 'CUSTOMIZE_ITEM_LOAD_TLV_DATA' to load TLV data config */
#define WEB_COLOR_CUSTOMIZE_ITEM_BITMASK                CUSTOMIZE_ITEM_LOAD_DEFAULT

/* Default color configurations */
#define MW_WEB_COLOR_TOP_LOGIN_FORM_BACKGROUND          (0x36444B)   /* Default top login form background color */
#define MW_WEB_COLOR_BOTTOM_LOGIN_FORM_BACKGROUND       (0xF7F8F9)   /* Default bottom login form background color */
#define MW_WEB_COLOR_LOGIN_FORM_BORDER                  (0x388BA9)   /* Default login form border color */
#define MW_WEB_COLOR_LOGIN_FORM_FONT                    (0x000000)   /* Default login form font color */
#define MW_WEB_COLOR_TOP_BANNER_BACKGROUND              (0x36444B)   /* Default top banner background color */
#define MW_WEB_COLOR_PORT_PANEL_BACKGROUND              (0xEBEBEB)   /* Default port panel background color */
#define MW_WEB_COLOR_PORT_NUMBER_FONT                   (0x000000)   /* Default port number font color */
#define MW_WEB_COLOR_DEVICE_TYPE_FONT                   (0xF86666)   /* Default device type font color */
#define MW_WEB_COLOR_MENU_BACKGROUND                    (0xEBEBEB)   /* Default menu background color */
#define MW_WEB_COLOR_MENU_TEXT                          (0x36444B)   /* Default menu text color */
#define MW_WEB_COLOR_MENU_HOVER_BACKGROUND              (0x005564)   /* Default menu hover background color */
#define MW_WEB_COLOR_SELECTED_MENU_BACKGROUND           (0xFFFFFF)   /* Default selected menu background color */
#define MW_WEB_COLOR_SUBPAGE_BACKGROUND                 (0xFFFFFF)   /* Default subpage background color */
#define MW_WEB_COLOR_SUBPAGE_FONT                       (0x000000)   /* Default subpage font color */
#define MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_TITLE_FONT  (0x005564)   /* Default subpage function group title font color */
#define MW_WEB_COLOR_SUBPAGE_FUNCTION_GROUP_BODER       (0x36444B)   /* Default subpage function group border color */
#define MW_WEB_COLOR_SUBPAGE_TABLE_HEADER_BACKGROUND    (0xEBEBEB)   /* Default subpage table header background color */
#define MW_WEB_COLOR_SUBPAGE_TABLE_BORDER               (0xA9A9A9)   /* Default subpage table border color */
#define MW_WEB_COLOR_SUBPAGE_DIVIDER                    (0x005564)   /* Default subpage divider color */
#define MW_WEB_COLOR_MENU_HOVER_FONT                    (0xFFFFFF)   /* Default menu hover font color */

/* Default customize items definitions */
#define CUSTOMER_HOMEPAGE_LOGO_HREF                     "http://www.airoha.com/"
#define CUSTOMER_PRODUCT_VERSION                        "EN8851"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct MW_WEB_COLOR_INFO_S
{
    UI8_T   color_value[TLV_DATA_MAX_COLOR_VALUE_LEN];               /* Color value(3B: R(1St Byte), G(2nd Byte), B(3rd Byte)) */
    UI8_T   tlv_type;                                                /* TLV type */
}MW_WEB_COLOR_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);

#endif  /* HTML_CONFIG_CUSTOMER_H */
