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


/* FILE NAME:  mgmt_vlan.h
 * PURPOSE:
 * It provides management vlan module API and definitions.
 *
 * NOTES:
 */

#ifndef MGMT_VLAN_H
#define MGMT_VLAN_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "air_port.h"

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    MGMT_VLAN_ACL_TYPE_TAG,
    MGMT_VLAN_ACL_TYPE_PRIORITY_TAG,
    MGMT_VLAN_ACL_TYPE_UNTAG,

    MGMT_VLAN_ACL_TYPE_LAST
} MGMT_VLAN_ACL_TYPE;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef MW_ERROR_NO_T
(*MGMT_VLAN_ACL_INIT_FUNC_T)(
    UI32_T unit,
    UI32_T acl_rule_id);

typedef BOOL_T
(*MGMT_VLAN_ACL_GET_ENABLE_SETTINGS_FUNC_T)(
    UI32_T unit,
    UI32_T acl_rule_id);

typedef struct
{
    UI32_T acl_rule_id;
    MGMT_VLAN_ACL_TYPE acl_type;
    MGMT_VLAN_ACL_INIT_FUNC_T acl_init_func;
    MGMT_VLAN_ACL_GET_ENABLE_SETTINGS_FUNC_T acl_get_enable_settings_func;
} MGMT_VLAN_ACL_SETTINGS;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T
mgmt_vlan_get_mgmt_vlan_info(
    UI8_T *ptr_vlan_mode,
    UI16_T *ptr_vid,
    AIR_PORT_BITMAP_T *ptr_member_ports,
    AIR_PORT_BITMAP_T *ptr_pvid_member_ports);

MW_ERROR_NO_T
mgmt_vlan_acl_update(
    UI32_T unit);

MW_ERROR_NO_T
mgmt_vlan_acl_init(
    UI32_T unit);
#endif /* End of MGMT_VLAN_H */
