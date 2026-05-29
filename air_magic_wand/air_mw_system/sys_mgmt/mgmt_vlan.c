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


/* FILE NAME:  mgmt_vlan.c
 * PURPOSE:
 * It provides management vlan module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "vlan_utils.h"
#include "mw_acl.h"
#include "mw_acl_settings.h"
#include "mgmt_vlan.h"
#include "sys_mgmt.h"
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

/* STATIC VARIABLE DECLARATIONS
 */
/* !!!Please be aware:
 * 1. Register the high ACL ID of the two aggregation rules for UNTAG frame.
 * 2. acl_init_func and acl_get_enable_settings_func should not use mw_acl_mutex_take() and mw_acl_mutex_release.
 */
const static MGMT_VLAN_ACL_SETTINGS _mgmt_vlan_acl_settings[] =
{
    /* Unicast ARP */
    {MW_ACL_ID_STATIC_UNICAST_ARP_TAGGED, MGMT_VLAN_ACL_TYPE_TAG, NULL, NULL},
    {MW_ACL_ID_STATIC_UNICAST_ARP_PRIORITY_TAGGED, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, NULL, NULL},
    {MW_ACL_ID_STATIC_UNICAST_ARP, MGMT_VLAN_ACL_TYPE_UNTAG, NULL, NULL},
    /* Broadcast ARP */
    {MW_ACL_ID_STATIC_BCAST_ARP_TAGGED, MGMT_VLAN_ACL_TYPE_TAG, NULL, NULL},
    {MW_ACL_ID_STATIC_BCAST_ARP_PRIORITY_TAGGED, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, NULL, NULL},
    {MW_ACL_ID_STATIC_BCAST_ARP, MGMT_VLAN_ACL_TYPE_UNTAG, NULL, NULL},
    /* DHCP */
    {MW_ACL_ID_STATIC_DHCP_ACK_TAGGED, MGMT_VLAN_ACL_TYPE_TAG, NULL, sys_mgmt_get_dhcp_enable_settings},
    {MW_ACL_ID_STATIC_DHCP_ACK_PRIORITY_TAGGED, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, NULL, sys_mgmt_get_dhcp_enable_settings},
    {MW_ACL_ID_STATIC_DHCP_ACK, MGMT_VLAN_ACL_TYPE_UNTAG, NULL, sys_mgmt_get_dhcp_enable_settings},

    /* System MAC address */
    {MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_TAG, MGMT_VLAN_ACL_TYPE_TAG, mw_acl_da_init_sysmac_addr_tag, mw_acl_da_get_sysmac_addr_enable_settings},
    {MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_PRIORITY_TAG, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, mw_acl_da_init_sysmac_addr_priTag, mw_acl_da_get_sysmac_addr_enable_settings},
    {MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_UNTAG, MGMT_VLAN_ACL_TYPE_UNTAG, mw_acl_da_init_sysmac_addr_untag, mw_acl_da_get_sysmac_addr_enable_settings},
#ifdef AIR_SUPPORT_IPV6
    /* IPv6 all nodes address */
    {MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_TAG, MGMT_VLAN_ACL_TYPE_TAG, mw_acl_da_init_ipv6_allnodes_addr_tag, mw_acl_da_get_ipv6_allnodes_addr_enable_settings},
    {MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_PRIORITY_TAG, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, mw_acl_da_init_ipv6_allnodes_addr_priTag, mw_acl_da_get_ipv6_allnodes_addr_enable_settings},
    {MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_UNTAG, MGMT_VLAN_ACL_TYPE_UNTAG, mw_acl_da_init_ipv6_allnodes_addr_untag, mw_acl_da_get_ipv6_allnodes_addr_enable_settings},
    /* IPv6 solicited-node address */
    {MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_TAG, MGMT_VLAN_ACL_TYPE_TAG, mw_acl_da_init_ipv6_solinode_addr_tag, mw_acl_da_get_ipv6_solinode_addr_enable_settings},
    {MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_PRIORITY_TAG, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, mw_acl_da_init_ipv6_solinode_addr_priTag, mw_acl_da_get_ipv6_solinode_addr_enable_settings},
    {MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_UNTAG, MGMT_VLAN_ACL_TYPE_UNTAG, mw_acl_da_init_ipv6_solinode_addr_untag, mw_acl_da_get_ipv6_solinode_addr_enable_settings},
    /* IPv6 ICMPv6 ND address */
    {MW_ACL_ID_STATIC_ICMPV6_ND_TAG, MGMT_VLAN_ACL_TYPE_TAG, mw_acl_ipv6_nd_init_tag, mw_acl_ipv6_nd_get_enable_settings},
    {MW_ACL_ID_STATIC_ICMPV6_ND_PRIORITY_TAG, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG, mw_acl_ipv6_nd_init_priTag, mw_acl_ipv6_nd_get_enable_settings},
    {MW_ACL_ID_STATIC_ICMPV6_ND_UNTAG, MGMT_VLAN_ACL_TYPE_UNTAG, mw_acl_ipv6_nd_init_untag, mw_acl_ipv6_nd_get_enable_settings},
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */

BOOL_T
_mgmt_vlan_acl_is_arp_rule(UI32_T acl_rule_id)
{
    if(((MW_ACL_ID_STATIC_UNICAST_ARP_TAGGED <= acl_rule_id) && (MW_ACL_ID_STATIC_UNICAST_ARP >= acl_rule_id)) ||
       ((MW_ACL_ID_STATIC_BCAST_ARP_TAGGED <= acl_rule_id) && (MW_ACL_ID_STATIC_BCAST_ARP >= acl_rule_id)))
    {
        MW_LOG_DEBUG(SYSTEM, "acl_rule_id = %d, arp rule\r\n", acl_rule_id);
        return TRUE;
    }
    MW_LOG_DEBUG(SYSTEM, "acl_rule_id = %d, not arp rule\r\n", acl_rule_id);
    return FALSE;
}

MW_ERROR_NO_T
_mgmt_vlan_acl_update_rule(
    UI32_T unit,
    UI32_T acl_rule_id,
    BOOL_T enable,
    AIR_PORT_BITMAP_T *ptr_portmap,
    UI16_T *ptr_ctag)
{
    AIR_ACL_RULE_T acl_rule;
#ifdef AIR_EN_CORAL
    AIR_ACL_ARP_RULE_T acl_arp_rule;
#endif
    I32_T rc = MW_E_OP_INCOMPLETE;

    if (NULL != ptr_portmap)
    {
        AIR_PORT_DEL(*ptr_portmap, PLAT_CPU_PORT);
        if (TRUE == AIR_PORT_BITMAP_EMPTY(*ptr_portmap))
        {
            MW_LOG_DEBUG(SYSTEM, "[WARN]For TAG ACL, portmap is 0. Disable ACL ID:%d", acl_rule_id);
            enable = FALSE;
        }
    }

#ifdef AIR_EN_CORAL
    if(FALSE == _mgmt_vlan_acl_is_arp_rule(acl_rule_id))
    {
#endif
        rc = air_acl_getRule(unit, acl_rule_id, &acl_rule);
        if (AIR_E_OK == rc)
        {
            if (NULL != ptr_portmap)
            {
                AIR_PORT_BITMAP_COPY(acl_rule.portmap, *ptr_portmap);
            }

            if (NULL != ptr_ctag)
            {
                acl_rule.key.ctag = *ptr_ctag;
                acl_rule.mask.ctag = 0x01;
                acl_rule.field_valid |= (1U << AIR_ACL_CTAG_KEY);
            }

            acl_rule.rule_en = enable;

            rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
        }
#ifdef AIR_EN_CORAL
    }
    else
    {
        rc = air_acl_getArpRule(unit, acl_rule_id, &acl_arp_rule);
        if (AIR_E_OK == rc)
        {
            if (NULL != ptr_portmap)
            {
                AIR_PORT_BITMAP_COPY(acl_arp_rule.portmap, *ptr_portmap);
            }

            if (NULL != ptr_ctag)
            {
                acl_arp_rule.key.ctag = *ptr_ctag;
                acl_arp_rule.mask.ctag = 0x01;
                acl_arp_rule.field_valid |= (1U << AIR_ACL_CTAG_KEY);
            }

            acl_arp_rule.rule_en = enable;

            rc = air_acl_setArpRule(unit, acl_rule_id, &acl_arp_rule);
        }
    }
#endif

    if (AIR_E_OK != rc)
    {
        MW_LOG_DEBUG(SYSTEM, "[ERR]%s fail. ACL ID:%d enable:%d rc:%d\n", __func__, acl_rule_id, enable, rc);
    }

    return rc;
}

MW_ERROR_NO_T
_mgmt_vlan_acl_update_untag(
    UI32_T unit,
    UI32_T acl_rule_id,
    BOOL_T enable)
{
    I32_T rc = MW_E_OP_INCOMPLETE;
    AIR_PORT_BITMAP_T pvid_member_ports = {0};

    /* There will be two rules aggregated to support the untag frame match. */
    if (FALSE == mw_acl_untag_rule_id_check(acl_rule_id))
    {
        return MW_E_NOT_SUPPORT;
    }

    rc = mgmt_vlan_get_mgmt_vlan_info(NULL, NULL, NULL, &pvid_member_ports);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    rc = _mgmt_vlan_acl_update_rule(unit, acl_rule_id, enable, &pvid_member_ports, NULL);
    rc |= _mgmt_vlan_acl_update_rule(unit, acl_rule_id - 1, enable, &pvid_member_ports, NULL);

    return rc;
}

MW_ERROR_NO_T
_mgmt_vlan_acl_update_priTag(
    UI32_T unit,
    UI32_T acl_rule_id,
    BOOL_T enable)
{
    I32_T rc = MW_E_OP_INCOMPLETE;
    AIR_PORT_BITMAP_T pvid_member_ports = {0};

    rc = mgmt_vlan_get_mgmt_vlan_info(NULL, NULL, NULL, &pvid_member_ports);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    return _mgmt_vlan_acl_update_rule(unit, acl_rule_id, enable, &pvid_member_ports, NULL);
}

MW_ERROR_NO_T
_mgmt_vlan_acl_update_tag(
    UI32_T unit,
    UI32_T acl_rule_id,
    BOOL_T enable)
{
    I32_T rc = MW_E_OP_INCOMPLETE;
    UI16_T vid = 0;
    AIR_PORT_BITMAP_T member_ports = {0};
    UI8_T vlan_mode = VLAN_NONE;

    rc = mgmt_vlan_get_mgmt_vlan_info(&vlan_mode, &vid, &member_ports, NULL);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    if (VLAN_1Q_ENABLE != vlan_mode)
    {
        vid = 1;
    }

    return _mgmt_vlan_acl_update_rule(unit, acl_rule_id, enable, &member_ports, &vid);
}


/* EXPORTED SUBPROGRAM BODIES
 */
/* ptr_port_map: Return the port members of the management vlan.
 * ptr_pvid_port_map: Return the management vlan port members whose pvids are the same as the management vlan ID.
 */
MW_ERROR_NO_T
mgmt_vlan_get_mgmt_vlan_info(
    UI8_T *ptr_vlan_mode,
    UI16_T *ptr_vid,
    AIR_PORT_BITMAP_T *ptr_member_ports,
    AIR_PORT_BITMAP_T *ptr_pvid_member_ports)
{
    sys_mgmt_get_mgmt_vid_pbmp(ptr_vlan_mode, ptr_vid, ptr_member_ports, ptr_pvid_member_ports);

    return MW_E_OK;
}

MW_ERROR_NO_T
mgmt_vlan_acl_update(
    UI32_T unit)
{
    UI32_T acl_count = sizeof(_mgmt_vlan_acl_settings) / sizeof(MGMT_VLAN_ACL_SETTINGS);
    UI32_T i = 0;
    const MGMT_VLAN_ACL_SETTINGS *ptr_acl_settings = NULL;
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE, ret = MW_E_OP_INCOMPLETE;
    BOOL_T acl_enable = FALSE;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (; i < acl_count; i++)
        {
            ptr_acl_settings = &_mgmt_vlan_acl_settings[i];

            if (NULL != ptr_acl_settings->acl_get_enable_settings_func)
            {
                acl_enable = ptr_acl_settings->acl_get_enable_settings_func(unit, ptr_acl_settings->acl_rule_id);
            }
            else
            {
                acl_enable = TRUE;
            }

            if (MGMT_VLAN_ACL_TYPE_UNTAG == ptr_acl_settings->acl_type)
            {
                rc = _mgmt_vlan_acl_update_untag(unit, ptr_acl_settings->acl_rule_id, acl_enable);
            }
            else if (MGMT_VLAN_ACL_TYPE_PRIORITY_TAG == ptr_acl_settings->acl_type)
            {
                rc = _mgmt_vlan_acl_update_priTag(unit, ptr_acl_settings->acl_rule_id, acl_enable);
            }
            else
            {
                rc = _mgmt_vlan_acl_update_tag(unit, ptr_acl_settings->acl_rule_id, acl_enable);
            }

            if (MW_E_OK != rc)
            {
                MW_LOG_DEBUG(SYSTEM, "[ERR]ACL idx:%d entry-id:%d update fail. rc:%d\n", i, _mgmt_vlan_acl_settings[i].acl_rule_id, rc);
                ret = rc;
            }
        }
        mw_acl_mutex_release();
    }

    return ret;
}

MW_ERROR_NO_T
mgmt_vlan_acl_init(
    UI32_T unit)
{
    UI32_T acl_count = sizeof(_mgmt_vlan_acl_settings) / sizeof(MGMT_VLAN_ACL_SETTINGS);
    UI32_T i = 0;
    const MGMT_VLAN_ACL_SETTINGS *ptr_acl_settings = NULL;
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE, ret = MW_E_OP_INCOMPLETE;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (; i < acl_count; i++)
        {
            ptr_acl_settings = &_mgmt_vlan_acl_settings[i];
            if (NULL != ptr_acl_settings->acl_init_func)
            {
                rc = ptr_acl_settings->acl_init_func(unit, _mgmt_vlan_acl_settings[i].acl_rule_id);
                if (MW_E_OK != rc)
                {
                    MW_LOG_ERROR(SYSTEM, "[ERR]ACL idx:%d entry-id:%d INIT fail. rc:%d\n", i, _mgmt_vlan_acl_settings[i].acl_rule_id, rc);
                    ret = rc;
                }
            }

        }
        mw_acl_mutex_release();
    }

    return ret;
}

