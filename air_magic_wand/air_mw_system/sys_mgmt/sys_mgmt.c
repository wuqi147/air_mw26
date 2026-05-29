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

/* FILE NAME:  sys_mgmt.c
 * PURPOSE:
 * It provides SYS_MGMT module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "inc/sys_mgmt.h"
#include "lwip/api.h"
#include "lwip/netifapi.h"
#ifdef AIR_SUPPORT_MQTTD
#include "mqttd.h"
#endif
#include "mw_tlv.h"
#include "mw_msg.h"
#include "web.h"
#include "html_config_customer.h"
#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
#include "sfp_task.h"
#include "sfp_msg.h"
#endif
#include "osapi_timer.h"
#include "vlan_utils.h"
#ifdef AIR_SUPPORT_LACP
#include "lacp.h"
#endif
#include "mw_acl.h"
#include "air_vlan.h"
#include "mgmt_vlan.h"
#include "air_util.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/nd6.h"
#include "mw_ipv6.h"
#endif

#ifdef AIR_SUPPORT_REMOTE_DEBUG
#include "remote_debug_log.h"
#ifdef AIR_SUPPORT_TELNET
#include "mw_telnet.h"
#endif
#endif

#define BC_MAC_UDF_NUM (3)

/* GLOBAL VARIABLE DECLARATIONS
*/
static SYS_MGMT_T           sys_mgmt_info;
static SYS_MGMT_TASK_T      sys_mgmt_task_ctx = {0};
#if LWIP_NETIF_EXT_STATUS_CALLBACK
static netif_ext_callback_t sys_mgmt_netif_callback;
#endif

const UI16_T                dhcp_acl_id = MW_ACL_ID_STATIC_DHCP_ACK;
const UI16_T                dhcp_acl_id_tagged = MW_ACL_ID_STATIC_DHCP_ACK_TAGGED;
const UI16_T                dhcp_acl_id_priority_tagged = MW_ACL_ID_STATIC_DHCP_ACK_PRIORITY_TAGGED;
const UI16_T                dhcp_acl_id_low = MW_ACL_ID_STATIC_DHCP_ACK_LOW;
static UI16_T               dhcp_acl_ap_id = MW_ATTACK_ID_INVALID;
static UI32_T               uni_acl_ap_id = MW_ATTACK_ID_INVALID;

#ifdef AIR_SUPPORT_SNTP
static UI64_T st_second = 0, original_st_second = 0;
#ifdef AIR_SUPPORT_CLI
portBASE_TYPE sntp_on = FALSE;
#endif
int g_local_timezone = 0;
UI32_T tickstart = 0, tickend = 0;
#endif
static timehandle_t         _dhcp_time_handle = NULL;

UI16_T _bcast_mac_udf_id[BC_MAC_UDF_NUM] = {MW_ACL_ID_INVALID, MW_ACL_ID_INVALID, MW_ACL_ID_INVALID};

C8_T * const _unknown_acl_name = "UNKNOWN";

static SYS_MGMT_VLAN_INFO_T      sys_mgmt_vlan_info;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
void sys_mgmt_get_default_ip(void);
void sys_mgmt_update_default_to_oper(void);
#ifdef AIR_SUPPORT_SECOND_NETIF
void sys_mgmt_if2_update_default_to_oper(void);
#endif

#ifdef AIR_SUPPORT_ICMP_CLIENT
static MW_ERROR_NO_T
_sys_mgmt_handle_db_ping_client(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data);
#endif /* AIR_SUPPORT_ICMP_CLIENT */

static BOOL_T
_sys_mgmt_update_mgmt_vlan_info(void);

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
static void
_sys_mgmt_update_acl(void);
#endif

static MW_ERROR_NO_T
_sys_mgmt_acl_add(
    C8_T             *ptr_acl_name,
    UI32_T           rule_id,
    void             *ptr_acl_rule,
    AIR_ACL_ACTION_T *ptr_acl_action);

#ifndef AIR_EN_CORAL
static MW_ERROR_NO_T
_sys_mgmt_udf_add(
    C8_T                 *ptr_acl_udf_name,
    UI32_T               *ptr_acl_udf_id,
    AIR_ACL_UDF_RULE_T   *ptr_acl_udf_rule);
#endif /* AIR_EN_CORAL */

static MW_ERROR_NO_T
_sys_mgmt_arp_acl(void);


/* LOCAL SUBPROGRAM BODIES
*/
#ifdef AIR_SUPPORT_SNTP
void
_mw_systime(
    void * arg)
{
    UNUSED(arg);

    tickend = air_util_getSystemSecTick();
    st_second = original_st_second + (tickend - tickstart);

    sys_timeout(1000, _mw_systime, NULL);
}
#endif

static void sys_mgmt_netif_ip_set(UI8_T is_dhcp, UI8_T is_callback)
{
    ip4_addr_t ip, mask, gw;
    ip_addr_t dns;
    struct netif *xNetIf = netif_find_default();

    if (NULL == xNetIf)
    {
        MW_LOG_ERROR(SYSTEM, "netif_ip_set: netif is NULL");
        return;
    }

    if (FALSE == is_dhcp)
    {
        if ((FALSE == ip4_addr_isany_val(sys_mgmt_info.static_ip)) && (FALSE == ip4_addr_isany_val(sys_mgmt_info.static_mask)))
        {
            ip4_addr_copy(ip, sys_mgmt_info.static_ip);
            ip4_addr_copy(mask, sys_mgmt_info.static_mask);
            ip4_addr_copy(gw, sys_mgmt_info.static_gw);
            ip_addr_copy_from_ip4(dns, sys_mgmt_info.static_dns);
        }
        else
        {
            ip4_addr_copy(ip, sys_mgmt_info.def_ip);
            ip4_addr_copy(mask, sys_mgmt_info.def_mask);
            ip4_addr_copy(gw, sys_mgmt_info.def_gw);
            ip_addr_copy_from_ip4(dns, sys_mgmt_info.def_dns);
        }
        MW_LOG_PRINTF("Set interface IP as %s.\n", ip4addr_ntoa(&ip));
    }
    else
    {
        ip4_addr_set_u32(&ip, sys_mgmt_info.oper_ip);
        ip4_addr_set_u32(&mask, sys_mgmt_info.oper_mask);
        ip4_addr_set_u32(&gw, sys_mgmt_info.oper_gw);
        ip_addr_set_ip4_u32_val(dns, sys_mgmt_info.oper_dns);
        MW_LOG_PRINTF("Set interface IP as %s(DHCP).\n", ip4addr_ntoa(&ip));
    }

    MW_LOG_DEBUG(SYSTEM, "netif_ip_set: set netif=%c%c%d, ip=0x%lx, mask=0x%lx, gw=0x%lx, dns=0x%lx",
                   xNetIf->name[0], xNetIf->name[1], xNetIf->num,
                   ip4_addr_get_u32(&ip),
                   ip4_addr_get_u32(&mask),
                   ip4_addr_get_u32(&gw),
                   ip4_addr_get_u32(ip_2_ip4(&dns)));

    if ((FALSE == ip4_addr_isany_val(ip)) && (FALSE == ip4_addr_isany_val(mask)))
    {
        dns_setserver(0, &dns);
        if (FALSE == is_callback)
        {
            netifapi_netif_set_addr(xNetIf, &ip, &mask, &gw);
        }
        else
        {
            netif_set_addr(xNetIf, &ip, &mask, &gw);
        }

        if (FALSE == is_dhcp)
        {
            MW_IPV4_T dns_val = (MW_IPV4_T)ip4_addr_get_u32(ip_2_ip4(&dns));
            sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_DNS, DB_ALL_ENTRIES, &dns_val, sizeof(MW_IPV4_T));
        }
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "netif_ip_set: Invalid parameter.");
    }
}

#ifdef AIR_SUPPORT_SECOND_NETIF
static void
sys_mgmt_netif2_ip_set(
    UI8_T is_callback)
{
    ip4_addr_t ip, mask, gw;
    struct netif *xNetIf;
    struct netif *defNetIf = netif_find_default();

    NETIF_FOREACH(xNetIf)
    {
        if (defNetIf->num != netif_get_index(xNetIf))
        {
            break;
        }
    }

    if (NULL == xNetIf)
    {
        MW_LOG_ERROR(SYSTEM, "netif2_ip_set: The second netif is NULL");
        return;
    }

    if ((FALSE == ip4_addr_isany_val(sys_mgmt_info.static_if2_ip)) && (FALSE == ip4_addr_isany_val(sys_mgmt_info.static_if2_mask)))
    {
        ip4_addr_copy(ip, sys_mgmt_info.static_if2_ip);
        ip4_addr_copy(mask, sys_mgmt_info.static_if2_mask);
    }
    else
    {
        ip4_addr_copy(ip, sys_mgmt_info.def_if2_ip);
        ip4_addr_copy(mask, sys_mgmt_info.def_if2_mask);
    }
    ip4_addr_set_any(&gw);
    MW_LOG_DEBUG(SYSTEM, "netif2_ip_set: set netif=%c%c%d, ip=0x%x, mask=0x%x, gw=0x%x",
                   xNetIf->name[0], xNetIf->name[1], xNetIf->num,
                   ip4_addr_get_u32(&ip),
                   ip4_addr_get_u32(&mask),
                   ip4_addr_get_u32(&gw));

    if ((FALSE == ip4_addr_isany_val(ip)) && (FALSE == ip4_addr_isany_val(mask)))
    {
        if (FALSE == is_callback)
        {
            netifapi_netif_set_addr(xNetIf, &ip, &mask, &gw);
        }
        else
        {
            netif_set_addr(xNetIf, &ip, &mask, &gw);
        }
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "netif2_ip_set: Invalid parameter.");
    }
}
#endif

static MW_ERROR_NO_T
_sys_mgmt_dhcp_set_tagged_acl(void)
{
    const UI32_T unit = 0;
    AIR_ACL_RULE_T acl_rule;
    AIR_ACL_ACTION_T tagged_action;
    I32_T rc = MW_E_OP_INCOMPLETE;

    UI16_T              mgmt_vlan_id;
    AIR_PORT_BITMAP_T   mgmt_vlan_port_bmp_total;
    sys_mgmt_get_mgmt_vid_pbmp(NULL, &mgmt_vlan_id, &mgmt_vlan_port_bmp_total, NULL);

    if (MW_E_OK == mw_acl_mutex_take())
    {
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = TRUE;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, mgmt_vlan_port_bmp_total);
#else
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
#endif
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        acl_rule.end = TRUE;
        acl_rule.key.etype = ETHTYPE_IP;
        acl_rule.mask.etype = 0x3;
        acl_rule.key.next_header = MW_IPPROTO_UDP;
        acl_rule.key.dip = IPADDR_BROADCAST;
        acl_rule.mask.dip = 0xf;
        acl_rule.key.dport = MW_DHCP_CLIENT_PORT;
        acl_rule.mask.dport = 0x3;

        /* Tagged */
        acl_rule.key.ctag = mgmt_vlan_id;
        acl_rule.mask.ctag = 0x1;
        acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_NEXT_HEADER_KEY) |
                                (1U << AIR_ACL_DIP_KEY) | (1U << AIR_ACL_DPORT_KEY) | (1U << AIR_ACL_CTAG_KEY));
        rc = air_acl_setRule(unit, dhcp_acl_id_tagged, &acl_rule);

        /* Priority Tagged */
        acl_rule.key.ctag = 0;
        rc |= air_acl_setRule(unit, dhcp_acl_id_priority_tagged, &acl_rule);

        if (AIR_E_OK == rc)
        {
            osapi_memset(&tagged_action, 0, sizeof(AIR_ACL_ACTION_T));
            tagged_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
            tagged_action.pri_user = MW_ACL_RX_PRIORITY_NOMRAL_PACKET;
            tagged_action.attack_rate_id = dhcp_acl_ap_id;
            tagged_action.field_valid |= (1U << AIR_ACL_FW_PORT) | (1U << AIR_ACL_PRI) | (1U << AIR_ACL_ATTACK) | (1U << AIR_ACL_MIB);
            tagged_action.acl_mib_id = MW_ACL_GEN_MIBID(dhcp_acl_id_tagged);
            rc = air_acl_setAction(unit, dhcp_acl_id_tagged, &tagged_action);

            tagged_action.acl_mib_id = MW_ACL_GEN_MIBID(dhcp_acl_id_priority_tagged);
            rc |= air_acl_setAction(unit, dhcp_acl_id_priority_tagged, &tagged_action);

            if (AIR_E_OK != rc)
            {
                MW_LOG_ERROR(SYSTEM, "set_tagged_acl: Add DHCP ACL rule entry-id %d action fail, rc=%d.", dhcp_acl_id_tagged, rc);
                air_acl_delRule(unit, dhcp_acl_id_tagged);
                air_acl_delRule(unit, dhcp_acl_id_priority_tagged);
            }
        }
        else
        {
            MW_LOG_ERROR(SYSTEM, "set_tagged_acl: Add DHCP ACL rule entry-id %d failed, rc=%d.", dhcp_acl_id_tagged, rc);
        }
        mw_acl_mutex_release();
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "set_tagged_acl: Get ACL mutex failed");
    }
    return rc;
}

static MW_ERROR_NO_T
_sys_mgmt_dhcp_set_untagged_acl(void)
{
    const UI32_T unit = 0;
    AIR_ACL_RULE_T acl_rule;
    AIR_ACL_ACTION_T action;
    I32_T rc = MW_E_OP_INCOMPLETE;

    UI16_T              mgmt_vlan_id;
    AIR_PORT_BITMAP_T   mgmt_vlan_port_bmp_total_untag;
    sys_mgmt_get_mgmt_vid_pbmp(NULL, &mgmt_vlan_id, NULL, &mgmt_vlan_port_bmp_total_untag);

    if (MW_E_OK == mw_acl_mutex_take())
    {
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = TRUE;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, mgmt_vlan_port_bmp_total_untag);
#else
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
#endif
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);

        acl_rule.key.etype = ETHTYPE_IP;
        acl_rule.mask.etype = 0x3;
        acl_rule.key.next_header = MW_IPPROTO_UDP;
        acl_rule.key.dip = IPADDR_BROADCAST;
        acl_rule.mask.dip = 0xf;
        acl_rule.key.dport = MW_DHCP_CLIENT_PORT;
        acl_rule.mask.dport = 0x3;

        /* Untagged Aggregate*/
        acl_rule.end = FALSE;
        acl_rule.field_valid = 0;
        acl_rule.field_valid |= ((1U << AIR_ACL_CTAG_KEY));
        acl_rule.key.ctag = 0x0;
        acl_rule.mask.ctag = 0x0;
        acl_rule.reverse = TRUE;
        rc = air_acl_setRule(unit, dhcp_acl_id_low, &acl_rule);

        /* Untagged */
        acl_rule.end = TRUE;
        acl_rule.reverse = FALSE;
        acl_rule.field_valid = 0;
        acl_rule.field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_NEXT_HEADER_KEY) |
                                (1U << AIR_ACL_DIP_KEY) | (1U << AIR_ACL_DPORT_KEY));
        rc = air_acl_setRule(unit, dhcp_acl_id, &acl_rule);

        if (AIR_E_OK == rc)
        {
            osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
            action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
            action.pri_user = MW_ACL_RX_PRIORITY_NOMRAL_PACKET;
            action.attack_rate_id = dhcp_acl_ap_id;
            action.field_valid |= (1U << AIR_ACL_FW_PORT) | (1U << AIR_ACL_PRI) | (1U << AIR_ACL_ATTACK) | (1U << AIR_ACL_MIB);
            action.acl_mib_id = MW_ACL_GEN_MIBID(dhcp_acl_id);
            rc = air_acl_setAction(unit, dhcp_acl_id, &action);
            if (AIR_E_OK != rc)
            {
                MW_LOG_ERROR(SYSTEM, "set_untagged_acl: Add DHCP ACL rule entry-id %d action fail, rc=%d.", dhcp_acl_id, rc);
                air_acl_delRule(unit, dhcp_acl_id);
            }
        }
        else
        {
            MW_LOG_ERROR(SYSTEM, "set_untagged_acl: Add DHCP ACL rule entry-id %d failed, rc=%d.", dhcp_acl_id, rc);
        }

        mw_acl_mutex_release();
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "set_untagged_acl: Get ACL mutex failed");
    }
    return rc;
}

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
/* FUNCTION NAME:   sys_mgmt_set_mgmt_vlan_id
 * PURPOSE:
 *      This API is used to set MGMT VLAN ID.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_set_mgmt_vlan_id(UI16_T vlan_id)
{

    DB_MSG_T                *db_msg = NULL;
    UI16_T                  db_size = 0;
    DB_VLAN_ENTRY_T         *ptr_vlan_entry = NULL;
    UI8_T                   i;
    UI32_T                  sys_mgmt_vlan_pbmap = 0;
    BOOL_T                  found = FALSE;

    /* get mgmt vlan pbmp */
    sys_mgmt_queue_getData(M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&ptr_vlan_entry);

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        MW_LOG_DEBUG(SYSTEM, "set_mgmt_vlan_id: {idx:%u,vid:%u,name:%s,mbrs:%u,tagMbrs:%u,untagMbrs:%u}",
            i+1,
            ptr_vlan_entry->vlan_id[i],
            ptr_vlan_entry->descr[i],
            ptr_vlan_entry->port_member[i],
            ptr_vlan_entry->tagged_member[i],
            ptr_vlan_entry->untagged_member[i]);

        if (ptr_vlan_entry->vlan_id[i] == vlan_id)
        {
            sys_mgmt_vlan_pbmap = ptr_vlan_entry->port_member[i] | ptr_vlan_entry->tagged_member[i] | ptr_vlan_entry->untagged_member[i];

            if (0 != sys_mgmt_vlan_pbmap)
            {
                found = TRUE;
                break;
            }
        }
    }

    if (TRUE == found)
    {
        sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_MGMT_VLAN, DB_ALL_ENTRIES, &vlan_id, sizeof(vlan_id));
    }
    else
    {
        MW_CMD_OUTPUT("\nInvalid VLAN ID: %u\n", vlan_id);
    }
}
#endif /* AIR_SUPPORT_MGMT_VLAN_CFG */

/* FUNCTION NAME: sys_mgmt_get_uni_acl_ap_id
 * PURPOSE:
 *      Get the unicast ACL Attack Prevention ID.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The unicast ACL Attack Prevention ID.
 *
 * NOTES:
 *      None
 */
UI32_T
sys_mgmt_get_uni_acl_ap_id(
    void)
{
    return uni_acl_ap_id;
}

/* FUNCTION NAME: sys_mgmt_set_uni_acl_ap_id
 * PURPOSE:
 *      Set the unicast ACL Attack Prevention ID.
 *
 * INPUT:
 *      id          --  The unicast ACL Attack Prevention ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
sys_mgmt_set_uni_acl_ap_id(
    UI32_T id)
{
    uni_acl_ap_id = id;
}



/* FUNCTION NAME:   sys_mgmt_get_dhcp_enable_settings
 * PURPOSE:
 *      This API is used to return sys_mgmt dhcp mode.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      BOOL_T
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
BOOL_T
sys_mgmt_get_dhcp_enable_settings()
{
    return ((MW_DHCP_DISABLE != sys_mgmt_info.dhcp_enable) ? TRUE : FALSE);
}

/* FUNCTION NAME:   sys_mgmt_dhcp_set
 * PURPOSE:
 *      This API is used to enable/disable sys_mgmt dhcp mode.
 *
 * INPUT:
 *      enable       --  admin mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_dhcp_set(UI8_T enable, UI8_T is_callback)
{
    struct netif *xNetIf = netif_find_default();
    const UI32_T unit = 0;
    UI32_T rc = MW_E_OP_INCOMPLETE;

    MW_LOG_DEBUG(SYSTEM, "dhcp set: %d", enable);

    if (enable == sys_mgmt_info.dhcp_enable)
        return;

    if (enable)
    {
        sys_mgmt_info.dhcp_enable = enable;

        if (MW_ATTACK_ID_INVALID == dhcp_acl_ap_id)
        {
            UI32_T                   ap_id = 0;
            AIR_DOS_RATE_LIMIT_CFG_T ap_cfg;

            if (MW_E_OK == mw_acl_mutex_take())
            {
                rc = MW_ATTACK_ID_GET_AVAILABLERULE(&ap_id, &ap_cfg, unit);
                if (MW_E_OK == rc)
                {
                    ap_cfg.pkt_thld   = AIR_MAX_SYS_MGMT_DHCP_RATE_PER_SEC;
                    ap_cfg.time_span  = 1000;
                    ap_cfg.block_time = 1;
                    ap_cfg.tick_sel   = AIR_DOS_RATE_TICKSEL_1MS;

                    rc = air_dos_setRateLimitCfg(unit, ap_id, &ap_cfg);
                }
                mw_acl_mutex_release();

                if (AIR_E_OK != rc)
                {
                    MW_LOG_ERROR(SYSTEM, "dhcp set: Add DHCP rate limit %d action fail, rc=%d.", ap_id, rc);
                    return;
                }
                dhcp_acl_ap_id = ap_id;
            }
        }

        _sys_mgmt_dhcp_set_tagged_acl();
        _sys_mgmt_dhcp_set_untagged_acl();

#if MW_DHCP
        sys_mgmt_info.oper_ip = 0;
        sys_mgmt_info.oper_mask = 0;
        sys_mgmt_info.oper_gw = 0;
        sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_ADDR, DB_ALL_ENTRIES, &sys_mgmt_info.oper_ip, sizeof(MW_IPV4_T));
        sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_MASK, DB_ALL_ENTRIES, &sys_mgmt_info.oper_mask, sizeof(MW_IPV4_T));
        sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_GW, DB_ALL_ENTRIES, &sys_mgmt_info.oper_gw, sizeof(MW_IPV4_T));
        if(MW_AUTODNS_ENABLE == sys_mgmt_info.autodns_enable)
        {
            sys_mgmt_info.oper_dns = 0;
            sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_DNS, DB_ALL_ENTRIES, &sys_mgmt_info.oper_dns, sizeof(MW_IPV4_T));
        }
        if (is_callback)
        {
            dhcp_start(xNetIf);
        }
        else
        {
            netifapi_dhcp_start(xNetIf);
        }
#else
        MW_LOG_WARN(SYSTEM, "dhcp set: MW_DHCP is not active");
#endif
    }
    else
    {
        sys_mgmt_info.autodns_enable = MW_AUTODNS_DISABLE;
        sys_mgmt_info.dhcp_enable = MW_DHCP_DISABLE;

        /* Set up the network interface. */
#if MW_DHCP
        if (is_callback)
        {
            dhcp_stop(xNetIf);
        }
        else
        {
            netifapi_dhcp_stop(xNetIf);
        }
#else
        MW_LOG_WARN(SYSTEM, "dhcp set: MW_DHCP is not active");
#endif
        sys_mgmt_netif_ip_set(FALSE, is_callback);

        if (MW_E_OK == mw_acl_mutex_take())
        {

            rc = air_acl_delAction(unit, dhcp_acl_id_tagged);
            if (rc != AIR_E_OK)
            {
                MW_LOG_ERROR(SYSTEM, "dhcp set: Delete ACL action entry-id %d failed, rc %d.", dhcp_acl_id_tagged, rc);
            }
            rc = air_acl_delRule(unit, dhcp_acl_id_tagged);
            if (rc != AIR_E_OK)
            {
                MW_LOG_ERROR(SYSTEM, "dhcp set: Delete ACL rule entry-id %d failed, rc %d.", dhcp_acl_id_tagged, rc);
            }
            rc = air_acl_delRule(unit, dhcp_acl_id_low);
            if (rc != AIR_E_OK)
            {
                MW_LOG_ERROR(SYSTEM, "dhcp set: Delete ACL rule entry-id %d failed, rc %d.", dhcp_acl_id_low, rc);
            }
            rc = air_acl_delAction(unit, dhcp_acl_id);
            if (rc != AIR_E_OK)
            {
                MW_LOG_ERROR(SYSTEM, "dhcp set: Delete ACL action entry-id %d failed, rc %d.", dhcp_acl_id, rc);
            }
            rc = air_acl_delRule(unit, dhcp_acl_id);
            if (rc != AIR_E_OK)
            {
                MW_LOG_ERROR(SYSTEM, "dhcp set: Delete ACL rule entry-id %d failed, rc %d.", dhcp_acl_id, rc);
            }
            mw_acl_mutex_release();
        }

    }
    MW_LOG_DEBUG(SYSTEM, "dhcp set end");
    return;
}

/* FUNCTION NAME:   sys_mgmt_ip_config_set
 * PURPOSE:
 *      This API is used to set ip config.
 *
 * INPUT:
 *      ip_addr
 *      ip_mask
 *      ip_gw
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
static void sys_mgmt_ip_config_set(UI32_T ip_addr, UI32_T ip_mask, UI32_T ip_gw, UI32_T ip_dns)
{
    UI8_T ip_change = FALSE;

    if (ip_addr != ip4_addr_get_u32(&sys_mgmt_info.static_ip))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_ip, ip_addr);
        MW_LOG_INFO(SYSTEM, "ip config: new ip = 0x%lx", ip4_addr_get_u32(&sys_mgmt_info.static_ip));
    }

    if (ip_mask != ip4_addr_get_u32(&sys_mgmt_info.static_mask))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_mask, ip_mask);
        MW_LOG_INFO(SYSTEM, "ip config: new mask = 0x%lx", ip4_addr_get_u32(&sys_mgmt_info.static_mask));
    }

    if (ip_gw != ip4_addr_get_u32(&sys_mgmt_info.static_gw))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_gw, ip_gw);
        MW_LOG_INFO(SYSTEM, "ip config: new gw = 0x%lx", ip4_addr_get_u32(&sys_mgmt_info.static_gw));
    }

    if (ip_dns != ip4_addr_get_u32(&sys_mgmt_info.static_dns))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_dns, ip_dns);
        MW_LOG_INFO(SYSTEM, "ip config: new dns = 0x%lx", ip4_addr_get_u32(&sys_mgmt_info.static_dns));
    }

    if (ip4_addr_cmp(&sys_mgmt_info.static_ip, &sys_mgmt_info.def_ip) &&
        ip4_addr_cmp(&sys_mgmt_info.static_mask, &sys_mgmt_info.def_mask) &&
        ip4_addr_cmp(&sys_mgmt_info.static_gw, &sys_mgmt_info.def_gw) &&
        ip4_addr_cmp(&sys_mgmt_info.static_dns, &sys_mgmt_info.def_dns))
    {
        if ((IPADDR_ANY == sys_mgmt_info.oper_ip) ||
            (IPADDR_ANY == sys_mgmt_info.oper_mask) ||
            (IPADDR_ANY == sys_mgmt_info.oper_gw) ||
            (IPADDR_ANY == sys_mgmt_info.oper_dns))
        {
            ip_change = TRUE;
            MW_LOG_INFO(SYSTEM, "ip config: Set to default IP");
        }
    }
    else/* Not equal to default */
    {
        ip_change = TRUE;
    }

    if (TRUE == ip_change)
    {
        sys_mgmt_netif_ip_set(FALSE, FALSE);
    }

    MW_LOG_DEBUG(SYSTEM, "ip config: end");

    return;
}

#ifdef AIR_SUPPORT_SECOND_NETIF
/* FUNCTION NAME:   sys_mgmt_if2_ip_config_set
 * PURPOSE:
 *      This API is used to set netif2 ip config.
 *
 * INPUT:
 *      ip_addr
 *      ip_mask
 *      ip_gw
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
static void sys_mgmt_if2_ip_config_set(UI32_T ip_addr, UI32_T ip_mask)
{
    UI8_T ip_change = FALSE;

    if (ip_addr != ip4_addr_get_u32(&sys_mgmt_info.static_if2_ip))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_if2_ip, ip_addr);
        MW_LOG_INFO(SYSTEM, "if2 ip config: new ip = 0x%x", ip4_addr_get_u32(&sys_mgmt_info.static_if2_ip));
    }

    if (ip_mask != ip4_addr_get_u32(&sys_mgmt_info.static_if2_mask))
    {
        ip4_addr_set_u32(&sys_mgmt_info.static_if2_mask, ip_mask);
        MW_LOG_INFO(SYSTEM, "if2 ip config: new mask = 0x%x", ip4_addr_get_u32(&sys_mgmt_info.static_if2_mask));
    }

    if (ip4_addr_cmp(&sys_mgmt_info.static_if2_ip, &sys_mgmt_info.def_if2_ip) &&
        ip4_addr_cmp(&sys_mgmt_info.static_if2_mask, &sys_mgmt_info.def_if2_mask))
    {
        if ((IPADDR_ANY == sys_mgmt_info.oper_if2_ip) ||
            (IPADDR_ANY == sys_mgmt_info.oper_if2_mask))
        {
            ip_change = TRUE;
            MW_LOG_INFO(SYSTEM, "if2 ip config: Set to default IP");
        }
    }
    else/* Not equal to default */
    {
        ip_change = TRUE;
    }

    if (TRUE == ip_change)
    {
        sys_mgmt_netif2_ip_set(FALSE);
    }

    MW_LOG_DEBUG(SYSTEM, "if2 ip config: end");

    return;
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */


/* FUNCTION NAME: sys_mgmt_queue_recv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer to the message buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_queue_recv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T timeout)
{
    return osapi_msgRecv(ptr_name, pptr_msg, 0, timeout);
}


/* FUNCTION NAME: _sys_mgmt_queue_db_genericGetData
 * PURPOSE:
 *      Get data from DB. It can get data for a single request or multiple
 *      requests depend on pptr_msg inputted. It's a blocking API.
 *
 * INPUT:
 *      pptr_msg        --  A double pointer points to a DB message with one or
 *                          more requests
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a DB message with data
 *                          received from DB for the single or multiple requests
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      If any error occors, the message pointed by *pptr_msg will be freed
 *      before the function returns. As a result the message pointed by *pptr_msg
 *      need be freed after use only when MW_E_OK is returned.
 */
static MW_ERROR_NO_T
_sys_mgmt_queue_db_genericGetData(
    DB_MSG_T **pptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    C8_T cq_name[DB_Q_NAME_SIZE] = {0};

    if ((NULL == pptr_msg) || (NULL == (*pptr_msg)))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_msg = *pptr_msg;
    osapi_memcpy(cq_name, ptr_msg->cq_name, DB_Q_NAME_SIZE - 1);
    /* There is no need to clear the SYS_MGMT_DB_BLOCK_QUEUE_NAME queue
     * before sending a new message and waiting for its response because the
     * queue is waited indefinitely every time it is used.
     */
    /* Send the message to DB */
    MW_LOG_DEBUG(SYSTEM, "g_get data: Send 0x%p", ptr_msg);
    ret = dbapi_sendMsg(ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    *pptr_msg = NULL;
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        MW_LOG_ERROR(SYSTEM, "g_get data: Fail to send the msg to DB. ret:%d msg:0x%p", ret, ptr_msg);
        return ret;
    }

    ret = sys_mgmt_queue_recv(cq_name, (UI8_T **)&ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK == ret)
    {
        MW_LOG_DEBUG(SYSTEM, "g_get data: Receive 0x%p", ptr_msg);
        ret = ptr_msg->type.result;
        if (MW_E_OK == ret)
        {
            (*pptr_msg) = ptr_msg;
        }
        else
        {
            MW_LOG_ERROR(SYSTEM, "g_get data: Fail at request %d ret:%d ptr_msg:0x%p", ptr_msg->type.result, ret, ptr_msg);
            MW_FREE(ptr_msg);
        }
    }
    /* else there is no need to free the message sent because:
     * 1. The timeout error should not be returned because the response is waited
     *    indefinitely.
     * 2. If DB fails to send response back to SYNCD queue, DB should free the
     *    message instead of SYNCD.
     */
    return ret;
}

/* FUNCTION NAME: sys_mgmt_queue_getData
 * PURPOSE:
 *      Get the raw data based on t_idx, f_idx and e_idx from DB. It is a
 *      blocking API.
 *
 * INPUT:
 *      method          --  The method of the getting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a db message received from DB
 *      ptr_data_size   --  A pointer returns the size of raw data obtained from DB
 *      pptr_data       --  A double pointer returns the raw data obtained from DB
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. If MW_E_OK is returned, the db message returned by pptr_msg need be
 *      freed after use.
 *      2. dbapi_parseMsg() cannot be used for the message returned by
 *         pptr_msg.
 */
MW_ERROR_NO_T
sys_mgmt_queue_getData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx};
    UI8_T *ptr_payload_data = NULL;

    if ((t_idx >= TABLES_LAST) || (ptr_data_size == NULL) ||
        (pptr_msg == NULL) || (*pptr_msg != NULL) || (pptr_data == NULL))
    {
        MW_LOG_DEBUG(SYSTEM, "get data: Invalid param: t_idx:%d ptr_data_size:%p pptr_msg:0x%p pptr_data:0x%p *pptr_msg:0x%p *pptr_data:0x%p",
                    t_idx, ptr_data_size, pptr_msg, pptr_data,
                    pptr_msg != NULL ? *pptr_msg : (void *)0xFFFFFFFF,
                    pptr_data != NULL ? *pptr_data : (void *)0xFFFFFFFF);
        return MW_E_BAD_PARAMETER;
    }

    *ptr_data_size = 0;
    ptr_msg = dbapi_createMsgByReq(SYS_MGMT_DB_BLOCK_QUEUE_NAME, method, 1, &request, &msg_size, (UI8_T **)&ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    MW_LOG_DEBUG(SYSTEM, "get data: qname=%s method=0x%X count=%u tid=%u fid=%u eid=%u msg:0x%p msize:%d payload:0x%p",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx, ptr_msg, msg_size, ptr_payload);

    ret = dbapi_appendMsgPayload(&request, NULL, &ptr_msg, &msg_size, (UI8_T **)&ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    ret = _sys_mgmt_queue_db_genericGetData(&ptr_msg);
    if (MW_E_OK == ret)
    {
        ret = dbapi_parseMsg(ptr_msg, 1, &request, ptr_data_size, (UI8_T **)pptr_data, (UI8_T **)&ptr_payload_data);
        if (MW_E_OK == ret)
        {
            *pptr_msg = ptr_msg;
        }
        else
        {
            MW_FREE(ptr_msg);
            *ptr_data_size = 0;
            *pptr_data = NULL;
        }
    }

    return ret;
}

/* FUNCTION NAME: _sys_mgmt_queue_send
 * PURPOSE:
 *      package message and call sending function to DB.
 *
 * INPUT:
 *      ptr_msg     -- A pointer to the item to be tranmitted
 *      size        --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
_sys_mgmt_queue_send(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    MW_ERROR_NO_T rc;
    MW_CHECK_PTR(ptr_msg);
    rc = dbapi_dbisReady();
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        osapi_free(ptr_msg);
        return rc;
    }
    rc = dbapi_sendRequesttoDb(size, ptr_msg);
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_LOG_ERROR(SYSTEM, "_send queue: send failed");
        osapi_free(ptr_msg);
    }
    return rc;
}

/* FUNCTION NAME: sys_mgmt_queue_send
 * PURPOSE:
 *      package message and call sending function to DB.
 *
 * INPUT:
 *      method      --  the method bitmap
 *      t_idx       --  the enum of the table
 *      f_idx       --  the enum of the field
 *      e_idx       --  the entry index in the table
 *      ptr_data    --  pointer to message data
 *      size        --  size of ptr_data
 *      ptr_name    --  name of module
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
sys_mgmt_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T          msg_size;

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(
            msg_size,
            SYS_MGMT_MODULE_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "send queue: alloc mem failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }
    /* message */
    if (method == M_SUBSCRIBE)
    {
        osapi_strncpy(ptr_msg->cq_name, SYS_MGMT_DB_QUEUE_NAME, DB_Q_NAME_SIZE);
    }
    else
    {
        ptr_msg->cq_name[0] = 0;
    }
    ptr_msg->method = method;
    ptr_msg->type.count = 1;
    MW_LOG_DEBUG(SYSTEM, "send queue: ptr_msg=%p, method=0x%X, count=%u, size=%u",
                 ptr_msg, ptr_msg->method, ptr_msg->type.count, size);
    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    if (size > 0 && method != M_GET)
    {
        memcpy(&(ptr_payload->ptr_data), ptr_data, size);
    }
    MW_LOG_DEBUG(SYSTEM, "send queue: ptr_payload=%p, req=(%u, %u, %u), data_size=%u",
                  ptr_payload,
                  ptr_payload->request.t_idx,
                  ptr_payload->request.f_idx,
                  ptr_payload->request.e_idx,
                  ptr_payload->data_size);
    /* Send message to DB */
    rc = _sys_mgmt_queue_send(ptr_msg, msg_size);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "send queue: Send to DB failed(%d)", rc);
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_sys_mgmt_db_msg_process(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    ip_addr_t dns;

    if ((NULL == ptr_request) || (NULL == ptr_data) || (0 == data_size))
    {
        return MW_E_BAD_PARAMETER;
    }

    switch (method)
    {
    case M_GET:
    case M_UPDATE:
        switch (ptr_request->t_idx)
        {
        case SYS_INFO:
            switch (ptr_request->f_idx)
            {
            case SYS_DHCP_ENABLE:
                if ((MW_DHCP_ENABLE == ((UI8_T *)ptr_data)[0]) ||
                    (MW_DHCP_WEB_ENABLE == ((UI8_T *)ptr_data)[0]) ||
                    (MW_DHCP_DISABLE == ((UI8_T *)ptr_data)[0]))
                {
                    if(NULL != _dhcp_time_handle)
                    {
                        MW_LOG_INFO(SYSTEM, "dbmsg proc: Delete DHCP timer");
                        if(MW_E_OK == osapi_timerActive(_dhcp_time_handle))
                        {
                            osapi_timerStop(_dhcp_time_handle);
                        }
                        osapi_timerDelete(_dhcp_time_handle);
                        _dhcp_time_handle = NULL;
                    }
                    sys_mgmt_dhcp_set(((UI8_T *)ptr_data)[0], FALSE);
                }
                else if (MW_DHCP_DONE == ((UI8_T *)ptr_data)[0])
                {
                    if(NULL != _dhcp_time_handle)
                    {
                        MW_LOG_INFO(SYSTEM, "dbmsg proc: Delete DHCP timer");
                        if(MW_E_OK == osapi_timerActive(_dhcp_time_handle))
                        {
                            osapi_timerStop(_dhcp_time_handle);
                        }
                        osapi_timerDelete(_dhcp_time_handle);
                        _dhcp_time_handle = NULL;
                    }
                    /* Active dhcp address */
                    sys_mgmt_info.dhcp_enable = MW_DHCP_DONE;
                    sys_mgmt_netif_ip_set(TRUE, FALSE);
                }
                else
                {
                    MW_LOG_ERROR(SYSTEM, "dbmsg proc: DHCP_ENABLE Invalid parameter = %d", ((UI8_T *)ptr_data)[0]);
                }
                break;
            case SYS_AUTODNS_ENABLE:
                sys_mgmt_info.autodns_enable = ((UI8_T *)ptr_data)[0];
                if ((MW_DHCP_DONE == sys_mgmt_info.dhcp_enable) && (MW_AUTODNS_ENABLE == sys_mgmt_info.autodns_enable))
                {
                    /*For DHCP on and turn on the autoDNS,then set temp dns */
                    sys_mgmt_info.oper_dns = ip4_addr_get_u32(&(sys_mgmt_info.temp_dns));
                    ip_addr_set_ip4_u32_val(dns, sys_mgmt_info.oper_dns);
                }
                else if ((MW_DHCP_DISABLE != sys_mgmt_info.dhcp_enable) && (MW_AUTODNS_DISABLE == sys_mgmt_info.autodns_enable))
                {
                    /*For DHCP on and turn off the autoDNS,then set static dns */
                    sys_mgmt_info.oper_dns = ip4_addr_get_u32(&sys_mgmt_info.static_dns);
                    ip_addr_set_ip4_u32_val(dns, ip4_addr_get_u32(&sys_mgmt_info.static_dns));
                }
                else
                {
                    /*other case do nothing!,can't be deleted!*/
                    break;
                }
                dns_setserver(0, &dns);
                sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_DNS, DB_ALL_ENTRIES, &sys_mgmt_info.oper_dns, sizeof(MW_IPV4_T));
                break;
            case SYS_STATIC_IP_DNS:
                /* For DHCP ON and AutoDNS OFF that get DNS data by user seting from db. */
                if ((MW_DHCP_DISABLE != sys_mgmt_info.dhcp_enable) && (MW_AUTODNS_DISABLE == sys_mgmt_info.autodns_enable))
                {
                    memcpy(&sys_mgmt_info.oper_dns, ptr_data, sizeof(MW_IPV4_T));
                    ip4_addr_set_u32(&sys_mgmt_info.static_dns, sys_mgmt_info.oper_dns);
                    ip_addr_set_ip4_u32_val(dns, sys_mgmt_info.oper_dns);

                    dns_setserver(0, &dns);
                    sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_DNS, DB_ALL_ENTRIES, &sys_mgmt_info.oper_dns, sizeof(MW_IPV4_T));
                }
                break;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
            case SYS_MGMT_VLAN:
                osapi_memcpy(&sys_mgmt_vlan_info.vlan_id, ptr_data, sizeof(UI16_T));
                _sys_mgmt_update_mgmt_vlan_info();
                _sys_mgmt_update_acl();
                break;
#endif
            case DB_ALL_FIELDS:
            {
                DB_SYS_INFO_T cfg_sys_info;

                memcpy(&cfg_sys_info, ptr_data, sizeof(DB_SYS_INFO_T));

                if ((!cfg_sys_info.static_ip) && (!cfg_sys_info.static_mask))
                {
                    MW_LOG_INFO(SYSTEM, "dbmsg proc: update ip config to 0");
                    sys_mgmt_update_default_to_oper();
                }
                else
                {
                    sys_mgmt_ip_config_set(cfg_sys_info.static_ip, cfg_sys_info.static_mask, cfg_sys_info.static_gw, cfg_sys_info.static_dns);
                }
                if (MW_DHCP_ENABLE == cfg_sys_info.dhcp_enable)
                {
                    /* This could be happened after swtich power on */
                    sys_mgmt_info.autodns_enable = cfg_sys_info.autodns_enable;
                    sys_mgmt_dhcp_set(MW_DHCP_ENABLE, FALSE);
                }
#ifdef AIR_SUPPORT_SECOND_NETIF
                if ((!cfg_sys_info.static_if2_ip) && (!cfg_sys_info.static_if2_mask))
                {
                    MW_LOG_INFO(SYSTEM, "dbmsg proc: update the 2nd netif ip config to 0");
                    sys_mgmt_if2_update_default_to_oper();
                }
                else
                {
                    sys_mgmt_if2_ip_config_set(cfg_sys_info.static_if2_ip, cfg_sys_info.static_if2_mask);
                }
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
                if (sys_mgmt_vlan_info.vlan_id != cfg_sys_info.mgmt_vlan)
                {
                    sys_mgmt_vlan_info.vlan_id = cfg_sys_info.mgmt_vlan;
                    MW_LOG_INFO(SYSTEM, "dbmsg proc: update mgmt vlan id to %d", sys_mgmt_vlan_info.vlan_id);
                    _sys_mgmt_update_mgmt_vlan_info();
                    _sys_mgmt_update_acl();
                }
#endif
#ifdef AIR_SUPPORT_IPV6
                if (M_GET == method)
                {
                    mw_ipv6_init_info(&cfg_sys_info);
                }
                else if (M_UPDATE == method)
                {
                    mw_ipv6_handle_config_change(&cfg_sys_info);
                }
#endif
                break;
            }
            default:
                MW_LOG_DEBUG(SYSTEM, "dbmsg proc: not handle field: [%d]", ptr_request->f_idx);
                break;
            }
            break;
#ifdef AIR_SUPPORT_ICMP_CLIENT
        case ICMP_CLIENT_INFO:
            _sys_mgmt_handle_db_ping_client(ptr_request, ptr_data);
            break;
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_MQTTD
        case MQTTD_CFG_INFO:
            if (ptr_request->f_idx == MQTTD_CFG_ENABLE)
            {
                if (TRUE == ((UI8_T *)ptr_data)[0])
                {
                    /* mqttd initialization */
                    mqttd_init(NULL);
                }
                else
                {
                    /* mqttd close */
                    mqttd_shutdown();
                }
            }
            break;
#endif
#ifdef AIR_SUPPORT_SNTP
        case SNTP_CFG:
        {
            ip_addr_t sntp_ip;
            ip4_addr_t sntp_ip4;
            DB_SNTP_CFG_T sntp_cfg;
            switch (ptr_request->f_idx)
            {
                case DB_ALL_ENTRIES:
                    osapi_memcpy(&sntp_cfg, ptr_data, sizeof(DB_SNTP_CFG_T));
                    g_local_timezone = (I8_T)sntp_cfg.sntp_timezone;
                    ip_addr_set_ip4_u32_val(sntp_ip, sntp_cfg.sntp_server1);
                    sntp_setserver(0, &sntp_ip);
                    ip_addr_set_ip4_u32_val(sntp_ip, sntp_cfg.sntp_server2);
                    sntp_setserver(1, &sntp_ip);
                    ip_addr_set_ip4_u32_val(sntp_ip, sntp_cfg.sntp_server3);
                    sntp_setserver(2, &sntp_ip);
                    if(TRUE == sntp_cfg.sntp_mode)
                    {
                        if(TRUE != sntp_on)
                        {
                            sntp_setoperatingmode(SNTP_OPMODE_POLL);
                            sntp_init();
                            sntp_on = TRUE;
                        }
                    }
                    break;
                case SNTP_CLOCK_MODE:
                    if(TRUE == ((UI8_T*)ptr_data)[0])
                    {
                        if(TRUE != sntp_on)
                        {
                            sntp_setoperatingmode(SNTP_OPMODE_POLL);
                            sntp_init();
                            sntp_on = TRUE;
                        }
                    }
                    else
                    {
                        sntp_stop();
                        sntp_on = FALSE;
                    }
                    break;
                case SNTP_TIMEZONE:
                    g_local_timezone = ((I8_T*)ptr_data)[0];
                    break;
                case SNTP_SERVER_1:
                    osapi_memcpy(&sntp_ip4, (UI8_T*)ptr_data, data_size);
                    if (!ip4_addr_isany_val(sntp_ip4))
                    {
                        ip_addr_copy_from_ip4(sntp_ip, sntp_ip4);
                        sntp_setserver(0, &sntp_ip);
                    }
                    break;
                case SNTP_SERVER_2:
                    osapi_memcpy(&sntp_ip4, (UI8_T*)ptr_data, data_size);
                    if (!ip4_addr_isany_val(sntp_ip4))
                    {
                        ip_addr_copy_from_ip4(sntp_ip, sntp_ip4);
                        sntp_setserver(1, &sntp_ip);
                    }
                    break;
                case SNTP_SERVER_3:
                    osapi_memcpy(&sntp_ip4, (UI8_T*)ptr_data, data_size);
                    if (!ip4_addr_isany_val(sntp_ip4))
                    {
                        ip_addr_copy_from_ip4(sntp_ip, sntp_ip4);
                        sntp_setserver(2, &sntp_ip);
                    }
                    break;
            }
            break;
        }
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
        case PORT_CFG_INFO:
        case VLAN_CFG_INFO:
        case VLAN_ENTRY:
            if ( TRUE == _sys_mgmt_update_mgmt_vlan_info())
            {
                _sys_mgmt_update_acl();
            }
            break;
#endif
#ifdef AIR_SUPPORT_LACP
        case PORT_OPER_INFO:
        case TRUNK_PORT:
        {
            lacp_handle_db_msg(method, ptr_request, data_size, ptr_data);
            break;
        }
#endif
        default:
            break;
        }
        break;
    case M_RESPONSE:
        /*
         *
         */
        break;
    case M_ACK:
        /*
         *
         */
        break;
    default:
        MW_LOG_WARN(SYSTEM, "dbmsg proc: unknown method: [%02X]", method);
        break;
    }
    return rc;
}

static void
_sys_mgmt_handle_db_msg(
    DB_MSG_T *ptr_msg)
{
    UI8_T i = 0;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    if (0 != (ptr_msg->method & M_B_RESPONSE))
    {
        /* Process Response message. Do nothing currently. */
        MW_LOG_INFO(SYSTEM, "handle dbmsg: Response msg");
    }
    else
    {
        /* Process the notification and other messages. */
        do
        {
            rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
            if (MW_E_OK == rc)
            {
                MW_LOG_DEBUG(SYSTEM, "handle dbmsg: index=%u, ptr_data=%p, req=(%u, %u, %u), data_size=%u, count=%d, method=%02X",
                               i++,
                               ptr_data,
                               request.t_idx,
                               request.f_idx,
                               request.e_idx,
                               data_size,
                               ptr_msg->type.count,
                               ptr_msg->method);

                rc = _sys_mgmt_db_msg_process(ptr_msg->method, &request, data_size, ptr_data);
                if (MW_E_OK != rc)
                {
                    MW_LOG_WARN(SYSTEM, "handle dbmsg: failed!(%d)", rc);
                }
            }
            /* Continue to parse the next request within the payload. */
        } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
    }
}


static BOOL_T
_sys_mgmt_update_mgmt_vlan_info()
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    const UI32_T    unit = 0;
    UI16_T          i = 0;
    UI16_T          pvid = 0;
    UI32_T          sys_mgmt_vlan_pbmap = 0;

    DB_MSG_T                *db_msg = NULL;
    UI16_T                  db_size = 0;
    void                    *db_data = NULL;
    DB_VLAN_ENTRY_T         *ptr_vlan_entry;
    DB_VLAN_CFG_INFO_T      vlan_cfg = {0};
    UI8_T port = 0;

    AIR_PORT_BITMAP_T    vlan_pbmp_total;
    AIR_PORT_BITMAP_T    vlan_pbmp_total_untag;
    BOOL_T               pbmp_change = FALSE;

    /* get vlan cfg */
    rc = sys_mgmt_queue_getData(M_GET, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "update mgmtVlan: get VLAN_CFG_INFO failed");
        return rc;
    }
    memcpy(&vlan_cfg, db_data, db_size);
    MW_FREE(db_msg);

    /* get mgmt vlan pbmp */
    rc = sys_mgmt_queue_getData(M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&ptr_vlan_entry);
    if(MW_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "update mgmtVlan: get VLAN_ENTRY failed");
        return rc;
    }

    AIR_PORT_BITMAP_COPY(vlan_pbmp_total, sys_mgmt_vlan_info.vlan_pbmp_total);
    AIR_PORT_BITMAP_COPY(vlan_pbmp_total_untag, sys_mgmt_vlan_info.vlan_pbmp_total_untag);

    AIR_PORT_BITMAP_CLEAR(sys_mgmt_vlan_info.vlan_pbmp_total);
    AIR_PORT_BITMAP_CLEAR(sys_mgmt_vlan_info.vlan_pbmp_total_untag);

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        MW_LOG_DEBUG(SYSTEM, "update mgmtVlan: {idx:%u,vid:%u,name:%s,mbrs:%u,tagMbrs:%u,untagMbrs:%u}",
            i+1,
            ptr_vlan_entry->vlan_id[i],
            ptr_vlan_entry->descr[i],
            ptr_vlan_entry->port_member[i],
            ptr_vlan_entry->tagged_member[i],
            ptr_vlan_entry->untagged_member[i]);

        if (ptr_vlan_entry->vlan_id[i] == sys_mgmt_vlan_info.vlan_id)
        {
            sys_mgmt_vlan_pbmap = ptr_vlan_entry->port_member[i] | ptr_vlan_entry->tagged_member[i] | ptr_vlan_entry->untagged_member[i];

            BITMAP_PORT_FOREACH(sys_mgmt_vlan_pbmap, port)
            {
                AIR_PORT_ADD(sys_mgmt_vlan_info.vlan_pbmp_total, port);
                if (VLAN_STATE_ENABLE == vlan_cfg.enable_port_b ||
                    VLAN_STATE_ENABLE == vlan_cfg.enable_mtu)
                {
                    sys_mgmt_vlan_info.vlan_mode = (VLAN_STATE_ENABLE == vlan_cfg.enable_port_b ? VLAN_PORT_ENABLE : VLAN_MTU_ENABLE);
                    AIR_PORT_ADD(sys_mgmt_vlan_info.vlan_pbmp_total_untag, port);
                }
                else if(VLAN_STATE_ENABLE == vlan_cfg.enable_8021q_b)
                {
                    sys_mgmt_vlan_info.vlan_mode = VLAN_1Q_ENABLE;
                    air_vlan_getPortCvid(unit, port, &pvid);
                    if(pvid == sys_mgmt_vlan_info.vlan_id)
                    {
                        AIR_PORT_ADD(sys_mgmt_vlan_info.vlan_pbmp_total_untag, port);
                    }
                }
            }
            break;
        }
    }

    if (VLAN_STATE_ENABLE == vlan_cfg.enable_port_b ||
        VLAN_STATE_ENABLE == vlan_cfg.enable_mtu)
    {
        /* update CPU PVID to default VLAN 1*/
        air_vlan_setPortCvid(unit, 0, 1);
    }
    else if(VLAN_STATE_ENABLE == vlan_cfg.enable_8021q_b)
    {
        /* update CPU PVID */
        air_vlan_setPortCvid(unit, 0, sys_mgmt_vlan_info.vlan_id);
    }

    if (!AIR_PORT_BITMAP_EQUAL(vlan_pbmp_total, sys_mgmt_vlan_info.vlan_pbmp_total) ||
        !AIR_PORT_BITMAP_EQUAL(vlan_pbmp_total_untag, sys_mgmt_vlan_info.vlan_pbmp_total_untag))
    {
        pbmp_change = TRUE;
    }

    MW_LOG_DEBUG(SYSTEM, "update mgmtVlan: VLAN = 0x%x, pbmp = 0x%x pbmp_change = %d", sys_mgmt_vlan_info.vlan_id, sys_mgmt_vlan_pbmap, pbmp_change);

    MW_FREE(db_msg);
    return pbmp_change;
}

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
static void
_sys_mgmt_update_acl(void)
{
    mgmt_vlan_acl_update(0);
}
#endif

/* FUNCTION NAME:   sys_mgmt_get_mgmt_vid_pbmp
 * PURPOSE:
 *      Management VLAN id and pbmp get function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_vlan_mode           : vlan mode
 *      ptr_vid                 : management VLAN id
 *      pbmp_total              : management VLAN pbmp for tag packet
 *      ptr_pbmp_total_untag    : management VLAN pbmp for untag packet
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */

MW_ERROR_NO_T
sys_mgmt_get_mgmt_vid_pbmp(
    UI8_T *ptr_vlan_mode,
    UI16_T *ptr_vid,
    AIR_PORT_BITMAP_T *ptr_pbmp_total,
    AIR_PORT_BITMAP_T *ptr_pbmp_total_untag)
{

    if (NULL != ptr_vlan_mode)
    {
        *ptr_vlan_mode = sys_mgmt_vlan_info.vlan_mode;
    }

    if (NULL != ptr_vid)
    {
        *ptr_vid = sys_mgmt_vlan_info.vlan_id;
    }

    if (NULL != ptr_pbmp_total)
    {
        AIR_PORT_BITMAP_COPY(*ptr_pbmp_total, sys_mgmt_vlan_info.vlan_pbmp_total);
    }
    if (NULL != ptr_pbmp_total_untag)
    {
        AIR_PORT_BITMAP_COPY(*ptr_pbmp_total_untag, sys_mgmt_vlan_info.vlan_pbmp_total_untag);
    }

    MW_LOG_DEBUG(SYSTEM,
        "get vlan pbmp: mode = 0x%x, MGMT VLAN = 0x%x, pbmp = 0x%x, pbmp_untag = 0x%x",
        sys_mgmt_vlan_info.vlan_mode, sys_mgmt_vlan_info.vlan_id, sys_mgmt_vlan_info.vlan_pbmp_total[0], sys_mgmt_vlan_info.vlan_pbmp_total_untag[0]);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_sys_mgmt_acl_add(
    C8_T             *ptr_acl_name,
    UI32_T           rule_id,
    void             *ptr_acl_rule,
    AIR_ACL_ACTION_T *ptr_acl_action)
{
    const UI32_T     unit = 0;
    UI32_T           acl_rule_id = rule_id;
    AIR_ACL_RULE_T   cur_acl_rule;
    AIR_ERROR_NO_T   rc;

    if ((ptr_acl_rule == NULL))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (ptr_acl_name == NULL)
    {
        ptr_acl_name = _unknown_acl_name;
    }

    if (MW_E_OK == mw_acl_mutex_take())
    {
        if (FALSE == MW_ACL_ID_IS_STATIC(acl_rule_id))
        {
            if (MW_E_OK != MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &cur_acl_rule, unit))
            {
                MW_LOG_PRINTF("[MAC INIT] Get avail ACL rule-id for %s failed !\n", ptr_acl_name);
                mw_acl_mutex_release();
                return MW_E_OP_INCOMPLETE;
            }
        }
#ifndef AIR_EN_CORAL
        rc = air_acl_setRule(unit, acl_rule_id, (AIR_ACL_RULE_T *)ptr_acl_rule);
#else
        rc = air_acl_setArpRule(unit, acl_rule_id, (AIR_ACL_ARP_RULE_T *)ptr_acl_rule);
#endif
        if (rc != AIR_E_OK)
        {
            MW_LOG_PRINTF("[MAC INIT] Add %s ACL rule entry-id %d failed, rc=%d.\n", ptr_acl_name, acl_rule_id, rc);
            mw_acl_mutex_release();
            return MW_E_OP_INCOMPLETE;
        }


        if((ptr_acl_action != NULL))
        {
            if (ptr_acl_action->field_valid & BIT(AIR_ACL_MIB))
            {
                ptr_acl_action->acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);
            }

            rc = air_acl_setAction(unit, acl_rule_id, ptr_acl_action);
            if (rc != AIR_E_OK)
            {
                MW_LOG_PRINTF("[MAC INIT] Add %s ACL action entry-id %d fail, rc=%d.\n", ptr_acl_name, acl_rule_id, rc);
#ifndef AIR_EN_CORAL
                air_acl_delRule(unit, acl_rule_id);
#else
                air_acl_delArpRule(unit, acl_rule_id);
#endif
                mw_acl_mutex_release();
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    mw_acl_mutex_release();

    return MW_E_OK;
}

#ifndef AIR_EN_CORAL
static MW_ERROR_NO_T
_sys_mgmt_udf_add(
    C8_T                 *ptr_acl_udf_name,
    UI32_T               *ptr_acl_udf_id,
    AIR_ACL_UDF_RULE_T   *ptr_acl_udf_rule)
{
    UI32_T             acl_udf_id = 0;
    const UI32_T       unit = 0;
    AIR_ERROR_NO_T     rc;
    AIR_ACL_UDF_RULE_T acl_udf_rule;

    if ((ptr_acl_udf_id == NULL) || (ptr_acl_udf_rule == NULL))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (ptr_acl_udf_name == NULL)
    {
        ptr_acl_udf_name = _unknown_acl_name;
    }

    if (MW_E_OK == mw_acl_mutex_take())
    {
        for (; acl_udf_id < AIR_MAX_NUM_OF_UDF_ENTRY; acl_udf_id++)
        {
            if (air_acl_getUdfRule(unit, acl_udf_id, &acl_udf_rule) == AIR_E_OK)
            {
                if (FALSE == acl_udf_rule.udf_rule_en)
                {
                    break;
                }
            }
        }
        if (acl_udf_id == AIR_MAX_NUM_OF_UDF_ENTRY)
        {
            MW_LOG_ERROR(SYSTEM, "udf add: get avail udf failed, name=%s!", ptr_acl_udf_name);
            mw_acl_mutex_release();
            return MW_E_ENTRY_REACH_END;
        }

        rc = air_acl_setUdfRule(unit, acl_udf_id, ptr_acl_udf_rule);
        if (rc != AIR_E_OK)
        {
            MW_LOG_ERROR(SYSTEM, "udf add: Add %s udf-id %d failed, rc=%d!", ptr_acl_udf_name, acl_udf_id, rc);
            mw_acl_mutex_release();
            return MW_E_OP_INCOMPLETE;
        }
        *ptr_acl_udf_id = acl_udf_id;
    }
    mw_acl_mutex_release();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_sys_mgmt_arp_acl(void)
{
    const UI8_T        udf_wofs_base = 0;
    UI8_T              index;
    UI32_T             br_acl_ap_id = MW_ACL_ID_INVALID;
    UI32_T             unit = 0;
    UI32_T             acl_udf_id = 0;
    AIR_ACL_UDF_RULE_T acl_udf_rule = {};
    AIR_ACL_RULE_T     acl_rule = {};
    AIR_ACL_ACTION_T   acl_action = {};
    AIR_DOS_RATE_LIMIT_CFG_T br_dos_rate_limit = {0};
    I32_T              rc = MW_E_OP_INCOMPLETE;

    UI16_T              mgmt_vlan_id;
    AIR_PORT_BITMAP_T   mgmt_vlan_port_bmp_total;
    sys_mgmt_get_mgmt_vid_pbmp(NULL, &mgmt_vlan_id, &mgmt_vlan_port_bmp_total, NULL);

    /* Broadcast MAC UDF */
    acl_udf_rule.udf_rule_en = TRUE;
    AIR_PORT_BITMAP_COPY(acl_udf_rule.portmap, PLAT_PORT_BMP_TOTAL);
    AIR_PORT_DEL(acl_udf_rule.portmap, PLAT_CPU_PORT);
    acl_udf_rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
    acl_udf_rule.word_ofst = 0;/* offset */
    acl_udf_rule.cmp_sel = 0;/* pattern */
    acl_udf_rule.cmp_pat = 0xFFFF;/* CMP pattern */
    acl_udf_rule.cmp_mask = 0xFFFF;/* CMP mask */

    for (index = 0; index < BC_MAC_UDF_NUM; ++index)
    {
        acl_udf_id = MW_ACL_ID_INVALID;
        acl_udf_rule.word_ofst = udf_wofs_base + index;
        _sys_mgmt_udf_add("Broadcast MAC", &acl_udf_id, &acl_udf_rule);
        _bcast_mac_udf_id[index] = acl_udf_id;
    }

    if (MW_ATTACK_ID_INVALID == uni_acl_ap_id)
    {
        MW_LOG_ERROR(SYSTEM, "Add ARP unicast dos failed, ID is invalid.");
        return MW_E_OP_INCOMPLETE;
    }

    rc = MW_ATTACK_ID_GET_AVAILABLERULE(&br_acl_ap_id, &br_dos_rate_limit, unit);
    if (MW_E_OK == rc)
    {
        br_dos_rate_limit.pkt_thld = MW_ARP_PACKET_THRESHOLD;
        br_dos_rate_limit.time_span = 1000;
        br_dos_rate_limit.block_time = 1;
        br_dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
        air_dos_setRateLimitCfg(unit, br_acl_ap_id, &br_dos_rate_limit);
    }
    if (AIR_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "Add ARP broadcast dos failed, rc=%d.", rc);
        return MW_E_OP_INCOMPLETE;
    }

    /* ARP ACL */
    acl_rule.rule_en = TRUE;
    acl_rule.end = TRUE;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    AIR_PORT_BITMAP_COPY(acl_rule.portmap, mgmt_vlan_port_bmp_total);
#else
    AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
#endif
    AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
    memcpy(acl_rule.key.dmac, _mw_mac_addr, MAC_ADDRESS_LEN);
    acl_rule.mask.dmac = 0x3f;
    acl_rule.key.etype = ETHTYPE_ARP;
    acl_rule.mask.etype = 0x3;
    for (index = 0; index < BC_MAC_UDF_NUM; ++index)
    {
        if (_bcast_mac_udf_id[index] == MW_ACL_ID_INVALID)
        {
            return MW_E_BAD_PARAMETER;
        }
        acl_rule.key.udf |= BIT(_bcast_mac_udf_id[index]);
    }
    acl_rule.mask.udf = acl_rule.key.udf;
    acl_action.pri_user = MW_ACL_RX_PRIORITY_ARP;

    /* Unicast */
    acl_action.field_valid = BIT(AIR_ACL_MIB) | BIT(AIR_ACL_FW_PORT) | BIT(AIR_ACL_PRI);
    if(AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID != uni_acl_ap_id)
    {
        acl_action.attack_rate_id = uni_acl_ap_id;
        acl_action.field_valid |= (1U << AIR_ACL_ATTACK);
    }

    /* Tagged */
    acl_rule.field_valid = BIT(AIR_ACL_DMAC_KEY) | BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_CTAG_KEY);
    acl_rule.key.ctag = mgmt_vlan_id;
    acl_rule.mask.ctag = 0x1;
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
    _sys_mgmt_acl_add("System Unicast ARP Tagged", MW_ACL_ID_STATIC_UNICAST_ARP_TAGGED, &acl_rule, &acl_action);

    /* Priority tag */
    acl_rule.key.ctag = 0;
    acl_rule.mask.ctag = 0x1;
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
    _sys_mgmt_acl_add("System Unicast ARP Priority Tagged", MW_ACL_ID_STATIC_UNICAST_ARP_PRIORITY_TAGGED, &acl_rule, &acl_action);

    /* Untagged Aggregate */
    acl_rule.end = FALSE;
    acl_rule.field_valid = 0;
    acl_rule.field_valid = BIT(AIR_ACL_CTAG_KEY);
    acl_rule.key.ctag = 0x0;
    acl_rule.mask.ctag = 0x0;
    acl_rule.reverse = TRUE;
    _sys_mgmt_acl_add("System Unicast ARP aggregate", MW_ACL_ID_STATIC_UNICAST_ARP_LOW, &acl_rule, NULL);

    /* Untagged */
    acl_rule.end = TRUE;
    acl_rule.reverse = FALSE;
    acl_rule.field_valid = 0;
    acl_rule.field_valid = BIT(AIR_ACL_DMAC_KEY) | BIT(AIR_ACL_ETYPE_KEY);
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
    _sys_mgmt_acl_add("System Unicast ARP", MW_ACL_ID_STATIC_UNICAST_ARP, &acl_rule, &acl_action);

    /* Broadcast*/
    if(AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID != br_acl_ap_id)
    {
        acl_action.attack_rate_id = br_acl_ap_id;
        acl_action.field_valid |= (1U << AIR_ACL_ATTACK);
    }

    /* Tagged */
    acl_rule.field_valid = BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_UDF_KEY) | BIT(AIR_ACL_CTAG_KEY);
    acl_rule.key.ctag = mgmt_vlan_id;
    acl_rule.mask.ctag = 0x1;
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
    _sys_mgmt_acl_add("System Broadcast ARP Tagged", MW_ACL_ID_STATIC_BCAST_ARP_TAGGED, &acl_rule, &acl_action);

    /* Priority Tagged */
    acl_rule.key.ctag = 0;
    acl_rule.mask.ctag = 0x1;
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
    _sys_mgmt_acl_add("System Broadcast ARP Priority Tagged", MW_ACL_ID_STATIC_BCAST_ARP_PRIORITY_TAGGED, &acl_rule, &acl_action);

    /* Untagged Aggregate */
    acl_rule.end = FALSE;
    acl_rule.field_valid = 0;
    acl_rule.field_valid = BIT(AIR_ACL_CTAG_KEY);
    acl_rule.key.ctag = 0x0;
    acl_rule.mask.ctag = 0x0;
    acl_rule.reverse = TRUE;
    _sys_mgmt_acl_add("System Broadcast ARP aggregate", MW_ACL_ID_STATIC_BCAST_ARP_LOW, &acl_rule, NULL);

    /* Untagged */
    acl_rule.end = TRUE;
    acl_rule.reverse = FALSE;
    acl_rule.field_valid = 0;
    acl_rule.field_valid = BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_UDF_KEY);
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
    _sys_mgmt_acl_add("System Broadcast ARP", MW_ACL_ID_STATIC_BCAST_ARP, &acl_rule, &acl_action);

    return MW_E_OK;
}
#else
static MW_ERROR_NO_T
_sys_mgmt_arp_acl(void)
{
    UI32_T             br_acl_ap_id = MW_ACL_ID_INVALID;
    UI32_T             unit = 0;
    AIR_ACL_ARP_RULE_T arp_acl_rule = {};
    AIR_ACL_ACTION_T   acl_action = {};
    AIR_DOS_RATE_LIMIT_CFG_T uni_dos_rate_limit = {0}, br_dos_rate_limit = {0};
    I32_T              rc = MW_E_OP_INCOMPLETE;

    UI16_T              mgmt_vlan_id;
    AIR_PORT_BITMAP_T   mgmt_vlan_port_bmp_total;
    sys_mgmt_get_mgmt_vid_pbmp(NULL, &mgmt_vlan_id, &mgmt_vlan_port_bmp_total, NULL);


    mw_dos_setGlobalCfg(unit, TRUE);
    rc = MW_ATTACK_ID_GET_AVAILABLERULE(&uni_acl_ap_id, &uni_dos_rate_limit, unit);
    if (MW_E_OK == rc)
    {
        uni_dos_rate_limit.pkt_thld = MW_ARP_PACKET_THRESHOLD;
        uni_dos_rate_limit.time_span = 1000;
        uni_dos_rate_limit.block_time = 1;
        uni_dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
        rc = air_dos_setRateLimitCfg(unit, uni_acl_ap_id, &uni_dos_rate_limit);
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "Add ARP unicast dos failed, rc=%d.", rc);
        mw_dos_setGlobalCfg(unit, FALSE);
        return MW_E_OP_INCOMPLETE;
    }

    rc = MW_ATTACK_ID_GET_AVAILABLERULE(&br_acl_ap_id, &br_dos_rate_limit, unit);
    if (MW_E_OK == rc)
    {
        br_dos_rate_limit.pkt_thld = MW_ARP_PACKET_THRESHOLD;
        br_dos_rate_limit.time_span = 1000;
        br_dos_rate_limit.block_time = 1;
        br_dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
        air_dos_setRateLimitCfg(unit, br_acl_ap_id, &br_dos_rate_limit);
    }
    else
    {
        MW_LOG_ERROR(SYSTEM, "Add ARP broadcast dos failed, rc=%d.", rc);
        mw_dos_setGlobalCfg(unit, FALSE);
        return MW_E_OP_INCOMPLETE;
    }

    /* ARP ACL */
    arp_acl_rule.rule_en = TRUE;
    arp_acl_rule.end = TRUE;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    AIR_PORT_BITMAP_COPY(arp_acl_rule.portmap, mgmt_vlan_port_bmp_total);
#else
    AIR_PORT_BITMAP_COPY(arp_acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
#endif

    AIR_PORT_DEL(arp_acl_rule.portmap, PLAT_CPU_PORT);
    osapi_memcpy(arp_acl_rule.key.dmac, _mw_mac_addr, MAC_ADDRESS_LEN);
    arp_acl_rule.mask.dmac = 0x3f;

    acl_action.pri_user = MW_ACL_RX_PRIORITY_ARP;
    /* Unicast */
    acl_action.field_valid = BIT(AIR_ACL_MIB) | BIT(AIR_ACL_FW_PORT) | BIT(AIR_ACL_PRI);

    if(AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID != uni_acl_ap_id)
    {
        acl_action.attack_rate_id = uni_acl_ap_id;
        acl_action.field_valid |= (1U << AIR_ACL_ATTACK);
    }

    /* For CORAL E1, it can aggregate but can not distiguish between untagged or another VID, will fix on E2 */
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;

    /* Tagged */
    arp_acl_rule.field_valid = BIT(AIR_ACL_ARP_DMAC_KEY) | BIT(AIR_ACL_ARP_CTAG_KEY);
    arp_acl_rule.key.ctag = mgmt_vlan_id;
    arp_acl_rule.mask.ctag = 0x1;
    _sys_mgmt_acl_add("System Unicast ARP", MW_ACL_ID_STATIC_UNICAST_ARP_TAGGED, &arp_acl_rule, &acl_action);

    /* Priority Tagged, Untagged */
    arp_acl_rule.field_valid = BIT(AIR_ACL_ARP_DMAC_KEY);
    _sys_mgmt_acl_add("System Unicast ARP", MW_ACL_ID_STATIC_UNICAST_ARP, &arp_acl_rule, &acl_action);

    /* Broadcast*/
    osapi_memset(arp_acl_rule.key.dmac, 0xFF, MAC_ADDRESS_LEN);
    if(AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID != br_acl_ap_id)
    {
        acl_action.attack_rate_id = br_acl_ap_id;
        acl_action.field_valid |= (1U << AIR_ACL_ATTACK);
    }

    /* For CORAL E1, it can aggregate but can not distiguish between untagged or another VID, will fix on E2 */
    acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;

    /* Tagged */
    arp_acl_rule.field_valid = BIT(AIR_ACL_ARP_DMAC_KEY) | BIT(AIR_ACL_ARP_CTAG_KEY);
    arp_acl_rule.key.ctag = mgmt_vlan_id;
    arp_acl_rule.mask.ctag = 0x1;
    _sys_mgmt_acl_add("System Broadcast ARP", MW_ACL_ID_STATIC_BCAST_ARP_TAGGED, &arp_acl_rule, &acl_action);

    /* Priority Tagged, Untagged */
    arp_acl_rule.field_valid = BIT(AIR_ACL_ARP_DMAC_KEY);
    _sys_mgmt_acl_add("System Broadcast ARP", MW_ACL_ID_STATIC_BCAST_ARP, &arp_acl_rule, &acl_action);

    return MW_E_OK;
}

#endif

/* FUNCTION NAME:   sys_mgmt_task
 * PURPOSE:
 *      This DHCP task.
 *
 * INPUT:
 *      ptr_pvParameters
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
static void sys_mgmt_task( void *ptr_pvParameters )
{
    MW_ERROR_NO_T rc = MW_E_OK;
    MW_MSG_T *ptr_msg = NULL;
    UI32_T unit = 0;
#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
    UI32_T start_tick = 0, end_tick = 0, dur_tick = 0;
#endif

    /* Just to kill the compiler warning. */
    (void)ptr_pvParameters;
    memset(&sys_mgmt_info, 0, sizeof(SYS_MGMT_T));

    sys_mgmt_get_default_ip();

    MW_LOG_INFO(SYSTEM, "Check DB is ready or not...");
    /* Check DB is ready or not */
    do{
        rc = dbapi_dbisReady();
    } while(MW_E_OK != rc);

    MW_LOG_INFO(SYSTEM, "send subscribe message to DB");
    sys_mgmt_queue_send(M_SUBSCRIBE, SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
#ifdef AIR_SUPPORT_ICMP_CLIENT
    sys_mgmt_queue_send(M_SUBSCRIBE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, 0, 0);
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_MQTTD
    sys_mgmt_queue_send(M_SUBSCRIBE, MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, 0, 0);
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_SNTP
    sys_mgmt_queue_send(M_SUBSCRIBE, SNTP_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
#endif
#ifdef AIR_SUPPORT_LACP
    sys_mgmt_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_SPEED, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_FLOW_CTRL, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_DUPLEX, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, 0, 0);
#endif

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    sys_mgmt_queue_send(M_SUBSCRIBE, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES, 0, 0);
    sys_mgmt_queue_send(M_SUBSCRIBE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
#endif

#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
    sfp_init(0, SYS_MGMT_DB_QUEUE_NAME);
#endif

    sys_mgmt_vlan_info.vlan_mode = VLAN_PORT_ENABLE;
    sys_mgmt_vlan_info.vlan_id = 1;

    _sys_mgmt_update_mgmt_vlan_info();

    /* Set ARP ACL */
    _sys_mgmt_arp_acl();

    mgmt_vlan_acl_init(unit);
    mgmt_vlan_acl_update(unit);

#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
    start_tick = osapi_sysTickGet();
#endif

    while (1)
    {
        rc = osapi_msgRecv(SYS_MGMT_DB_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, SYS_MGMT_TASK_DELAY);
        if (rc == MW_E_OK && NULL != ptr_msg)
        {
            if (MW_MSG_ID_DB == ptr_msg->msg_id)
            {
                _sys_mgmt_handle_db_msg((DB_MSG_T *)ptr_msg);
            }

#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
            if ((ptr_msg->msg_id >= MW_MSG_ID_SFP_BASE) && (ptr_msg->msg_id < MW_MSG_ID_SFP_LAST))
            {
                sfp_handle_msg(ptr_msg);
            }
#endif
#ifdef AIR_SUPPORT_LACP
            lacp_handle_pkt_and_timer_msg(ptr_msg);
#endif
            MW_FREE(ptr_msg);
        }

#ifdef AIR_SUPPORT_REMOTE_DEBUG
        remote_debug_log_task_handler();
#endif

#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
        end_tick = osapi_sysTickGet();
        dur_tick = end_tick - start_tick;
        if (dur_tick >= SFP_TASK_TIMER_BASE_INTERVAL_MS)
        {
            start_tick = osapi_sysTickGet();
            sfp_handle_timerExpired();
        }
#endif
    }
}

/* FUNCTION NAME:   sys_mgmt_get_default_ip
 * PURPOSE:
 *      sys_mgmt get default ip/mask/gw function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_get_default_ip(void)
{
    C8_T   ip_str[SYS_MGMT_IPV4_STR_SIZE];
    struct netif *xNetIf = netif_find_default();

    if (xNetIf != NULL)
    {
        const ip_addr_t *ptr_dns = dns_getserver(0);

        memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
#ifdef AIR_SUPPORT_SECOND_NETIF
        MW_LOG_INFO(SYSTEM, "default ip: Interface Name: %c%c%d", xNetIf->name[0], xNetIf->name[1], xNetIf->num);
#else
        MW_LOG_INFO(SYSTEM, "default ip: Interface Name: %c%c", xNetIf->name[0], xNetIf->name[1]);
#endif
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&xNetIf->ip_addr))));
        MW_LOG_INFO(SYSTEM, "default ip: IP Address    : %s", ip_str);
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&xNetIf->netmask))));
        MW_LOG_INFO(SYSTEM, "default ip: Net Mask      : %s", ip_str);
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&xNetIf->gw))));
        MW_LOG_INFO(SYSTEM, "default ip: Gateway       : %s", ip_str);
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(ptr_dns))));
        MW_LOG_INFO(SYSTEM, "default ip: DNS server    : %s", ip_str);

        ip4_addr_copy(sys_mgmt_info.def_ip, *ip_2_ip4(&xNetIf->ip_addr));
        ip4_addr_copy(sys_mgmt_info.def_mask, *ip_2_ip4(&xNetIf->netmask));
        ip4_addr_copy(sys_mgmt_info.def_gw, *ip_2_ip4(&xNetIf->gw));
        ip4_addr_copy(sys_mgmt_info.def_dns, *ip_2_ip4(ptr_dns));
    }

#ifdef AIR_SUPPORT_SECOND_NETIF
    struct netif *netIf;
    NETIF_FOREACH(netIf)
    {
        if ((xNetIf != NULL) && (xNetIf->num != netIf->num))
        {
            memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
            MW_LOG_INFO(SYSTEM, "default ip: Interface Name: %c%c%d", netIf->name[0], netIf->name[1], netIf->num);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&netIf->ip_addr))));
            MW_LOG_INFO(SYSTEM, "default ip: IP Address    : %s", ip_str);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&netIf->netmask))));
            MW_LOG_INFO(SYSTEM, "default ip: Net Mask      : %s", ip_str);

            ip4_addr_copy(sys_mgmt_info.def_if2_ip, *ip_2_ip4(&netIf->ip_addr));
            ip4_addr_copy(sys_mgmt_info.def_if2_mask, *ip_2_ip4(&netIf->netmask));
            break;
        }
    }
#endif
    return;
}

/* FUNCTION NAME:   sys_mgmt_update_default_to_oper
 * PURPOSE:
 *      sys_mgmt update default ip/mask/gw to oper function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_update_default_to_oper(void)
{
    if ((FALSE == ip4_addr_isany_val(sys_mgmt_info.def_ip)) && (FALSE == ip4_addr_isany_val(sys_mgmt_info.def_mask)))
    {
        sys_mgmt_ip_config_set(0, 0, 0, 0);
    }
    return;
}

#ifdef AIR_SUPPORT_SECOND_NETIF
/* FUNCTION NAME:   sys_mgmt_if2_update_default_to_oper
 * PURPOSE:
 *      sys_mgmt update default ip/mask/gw to netif2 oper function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_if2_update_default_to_oper(void)
{
    if ((FALSE == ip4_addr_isany_val(sys_mgmt_info.def_if2_ip)) && (FALSE == ip4_addr_isany_val(sys_mgmt_info.def_if2_mask)))
    {
        sys_mgmt_if2_ip_config_set(0, 0);
    }
    return;
}
#endif

static void
_dhcp_timer(
    TimerHandle_t xTimer)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_LOG_DEBUG(SYSTEM, "DHCP timer: expired, xTimer = %p", xTimer);

    if (NULL != xTimer)
    {
        sys_mgmt_info.dhcp_enable = MW_DHCP_DONE;
        ret = sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &sys_mgmt_info.dhcp_enable, sizeof(UI8_T));
        if(MW_E_OK != ret)
        {
            MW_LOG_WARN(SYSTEM, "DHCP timer: update DB SYS_DHCP_ENABLE fail.");
            sys_mgmt_info.dhcp_enable = MW_DHCP_ENABLE;
        }
        sys_mgmt_netif_ip_set(TRUE, FALSE);
    }
}

/* FUNCTION NAME:   sys_mgmt_netif_ext_status_callback
 * PURPOSE:
 *      sys_mgmt callback function for netif status change.
 *      Ex: IP/Mask/GW is changed.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
#if LWIP_NETIF_EXT_STATUS_CALLBACK
static void
sys_mgmt_netif_ext_status_callback(struct netif *netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t *args)
{
    C8_T ip_str[SYS_MGMT_IPV4_STR_SIZE];
    MW_ERROR_NO_T ret = MW_E_OK;
    const netif_ext_callback_args_t *cb_args = args;
    MW_IPV4_T new_ip = 0, new_mask = 0, new_gw = 0;
    ip_addr_t new_dns = IPADDR4_INIT(0);
    UI8_T autodns_state = 0;
    UI8_T state = 0;
    struct netif *defNetif = netif_find_default();

    MW_LOG_INFO(SYSTEM, "netif callback: reason: 0x%x", reason);

    if (NULL == netif)
    {
        MW_LOG_ERROR(SYSTEM, "netif callback: netif is NULL");
        return;
    }
    else if (reason & (LWIP_NSC_IPV4_ADDRESS_CHANGED | LWIP_NSC_IPV4_NETMASK_CHANGED | LWIP_NSC_IPV4_GATEWAY_CHANGED |
                       LWIP_NSC_IPV4_SETTINGS_CHANGED))
    {
        memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
        MW_LOG_INFO(SYSTEM, "netif callback: interface Name: %c%c%d", netif->name[0], netif->name[1], netif->num);

        if (netif->num == defNetif->num)
        {
            if((MW_DHCP_DONE == sys_mgmt_info.dhcp_enable) && (IPADDR_ANY == sys_mgmt_info.oper_ip))
            {
                /* This case should occur when dhcp release ip address */
                sys_mgmt_info.dhcp_enable = MW_DHCP_ENABLE;
            }
            if((MW_DHCP_ENABLE == sys_mgmt_info.dhcp_enable) || (MW_DHCP_WEB_ENABLE == sys_mgmt_info.dhcp_enable))
            {
                ip4_addr_set_any(&sys_mgmt_info.temp_dns);
                if (NULL != cb_args)
                {
                    /* Use struct old ip info to carry dhcp ip address */
                    if (NULL != cb_args->ipv4_changed.old_address)
                    {
                        new_ip = ip4_addr_get_u32(ip_2_ip4(cb_args->ipv4_changed.old_address));
                    }
                    if (NULL != cb_args->ipv4_changed.old_netmask)
                    {
                        new_mask = ip4_addr_get_u32(ip_2_ip4(cb_args->ipv4_changed.old_netmask));
                    }
                    if (NULL != cb_args->ipv4_changed.old_gw)
                    {
                        new_gw = ip4_addr_get_u32(ip_2_ip4(cb_args->ipv4_changed.old_gw));
                    }
                    if (NULL != cb_args->ipv4_changed.old_dns)
                    {
                        ip4_addr_copy(sys_mgmt_info.temp_dns, *ip_2_ip4(cb_args->ipv4_changed.old_dns));
                    }
                }

                if (MW_AUTODNS_ENABLE == sys_mgmt_info.autodns_enable)
                {
                    if (FALSE == ip4_addr_isany_val(sys_mgmt_info.temp_dns))
                    {
                        ip_addr_set_ip4_u32_val(new_dns, ip4_addr_get_u32(&(sys_mgmt_info.temp_dns)));
                    }
                    else if (FALSE == ip4_addr_isany_val(sys_mgmt_info.static_dns))
                    {
                        ip_addr_set_ip4_u32_val(new_dns, ip4_addr_get_u32(&sys_mgmt_info.static_dns));
                    }
                    else
                    {
                        ip_addr_set_ip4_u32_val(new_dns, ip4_addr_get_u32(&sys_mgmt_info.def_dns));
                    }
                }
                else
                {
                    ip_addr_copy(new_dns, *dns_getserver(0));
                }
            }
            else
            {
                ip_addr_copy(new_dns, *dns_getserver(0));
                new_ip = ip4_addr_get_u32(ip_2_ip4(&netif->ip_addr));
                new_mask = ip4_addr_get_u32(ip_2_ip4(&netif->netmask));
                new_gw = ip4_addr_get_u32(ip_2_ip4(&netif->gw));

                if ((NULL != cb_args) && (LWIP_NSC_IPV4_SETTINGS_CHANGED == reason) && (MW_AUTODNS_ENABLE == sys_mgmt_info.autodns_enable))
                {
                    /* This is for DHCP renew case with AUTO_DNS enabled. */
                    const ip_addr_t *ptr_dns = cb_args->ipv4_changed.old_dns;
                    if (NULL != ptr_dns)
                    {
                        ip_addr_copy(new_dns, *ptr_dns);
                        dns_setserver(0, &new_dns);
                    }
                }
            }

            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(new_ip));
            MW_LOG_INFO(SYSTEM, "netif callback: IP Address    : %s", ip_str);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(new_mask));
            MW_LOG_INFO(SYSTEM, "netif callback: Net Mask      : %s", ip_str);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(new_gw));
            MW_LOG_INFO(SYSTEM, "netif callback: Gateway       : %s", ip_str);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(ip_2_ip4(&new_dns))));
            MW_LOG_INFO(SYSTEM, "netif callback: DNS           : %s", ip_str);

            if((sys_mgmt_info.oper_ip != new_ip) ||
               (sys_mgmt_info.oper_mask != new_mask) ||
               (sys_mgmt_info.oper_gw != new_gw) ||
               (sys_mgmt_info.oper_dns != ip4_addr_get_u32(ip_2_ip4(&new_dns))))
            {
                sys_mgmt_info.oper_ip = new_ip;
                sys_mgmt_info.oper_mask = new_mask;
                sys_mgmt_info.oper_gw = new_gw;
                sys_mgmt_info.oper_dns = ip4_addr_get_u32(ip_2_ip4(&new_dns));
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP_ADDR");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_ADDR, DB_ALL_ENTRIES, &sys_mgmt_info.oper_ip, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_ip = IPADDR_ANY;
                }
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP_MASK");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_MASK, DB_ALL_ENTRIES, &sys_mgmt_info.oper_mask, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_mask = IPADDR_ANY;
                }
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP_GW");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_GW, DB_ALL_ENTRIES, &sys_mgmt_info.oper_gw, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_gw = IPADDR_ANY;
                }
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP_DNS");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP_DNS, DB_ALL_ENTRIES, &sys_mgmt_info.oper_dns, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_dns = IPADDR_ANY;
                }

                if(MW_DHCP_ENABLE == sys_mgmt_info.dhcp_enable)
                {
                    sys_mgmt_info.dhcp_enable = MW_DHCP_DONE;
                    ret = sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &sys_mgmt_info.dhcp_enable, sizeof(UI8_T));
                    if(MW_E_OK != ret)
                    {
                        MW_LOG_WARN(SYSTEM, "netif callback: update DB SYS_DHCP_ENABLE fail.");
                        sys_mgmt_info.dhcp_enable = MW_DHCP_ENABLE;
                    }
                    sys_mgmt_netif_ip_set(TRUE, TRUE);
                }
                else if(MW_DHCP_WEB_ENABLE == sys_mgmt_info.dhcp_enable)
                {
                    if(NULL == _dhcp_time_handle)
                    {
                        /* Create timer */
                        ret = osapi_timerCreate(SYS_MGMT_TIMER_NAME,
                            _dhcp_timer,
                            FALSE,
                            SYS_MGMT_TIMER_PERIOD,
                            NULL,
                            &_dhcp_time_handle);

                        if (MW_E_OK == ret)
                        {
                            ret = osapi_timerStart(_dhcp_time_handle);
                            if (MW_E_OK != ret)
                            {
                                MW_LOG_INFO(SYSTEM, "netif callback: start dhcp timer failed! ret = %d", ret);
                                return;
                            }
                        }
                        else
                        {
                            MW_LOG_INFO(SYSTEM, "netif callback: create dhcp timer failed! ret = %d", ret);
                            return;
                        }
                    }
                    else
                    {
                        ret = osapi_timerStop(_dhcp_time_handle);
                        if (MW_E_OK != ret)
                        {
                            MW_LOG_INFO(SYSTEM, "netif callback: stop dhcp timer failed! ret = %d", ret);
                            return;
                        }
                        ret = osapi_timerStart(_dhcp_time_handle);
                        if (MW_E_OK != ret)
                        {
                            MW_LOG_INFO(SYSTEM, "netif callback: start dhcp timer failed! rc = %d", ret);
                            return;
                        }
                    }
                }
            }
        }
#ifdef AIR_SUPPORT_SECOND_NETIF
        else
        {
            new_ip = ip4_addr_get_u32(ip_2_ip4(&netif->ip_addr));
            new_mask = ip4_addr_get_u32(ip_2_ip4(&netif->netmask));
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(new_ip));
            MW_LOG_INFO(SYSTEM, "netif callback: Netif2 IP Address    : %s", ip_str);
            MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(new_mask));
            MW_LOG_INFO(SYSTEM, "netif callback: Netif2 INet Mask      : %s", ip_str);

            if((sys_mgmt_info.oper_if2_ip != new_ip) ||
               (sys_mgmt_info.oper_if2_mask != new_mask))
            {
                sys_mgmt_info.oper_if2_ip = new_ip;
                sys_mgmt_info.oper_if2_mask = new_mask;
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP2_ADDR");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP2_ADDR, DB_ALL_ENTRIES, &sys_mgmt_info.oper_if2_ip, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_if2_ip = IPADDR_ANY;
                }
                MW_LOG_INFO(SYSTEM, "netif callback: update DB SYS_OPER_IP2_MASK");
                ret = sys_mgmt_queue_send(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP2_MASK, DB_ALL_ENTRIES, &sys_mgmt_info.oper_if2_mask, sizeof(MW_IPV4_T));
                if(MW_E_OK != ret)
                {
                    sys_mgmt_info.oper_if2_mask = IPADDR_ANY;
                }
            }
        }
#endif
    }
    else if ((reason & LWIP_NSC_IPV4_DHCP_FAIL) && (netif->num == defNetif->num))
    {
        /* DHCP timeout, disable dhcp and set ip to default ip address */
        autodns_state = MW_AUTODNS_DISABLE;
        state = MW_DHCP_DISABLE;
        sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_AUTODNS_ENABLE, DB_ALL_ENTRIES, &autodns_state, sizeof(UI8_T));
        sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &state, sizeof(UI8_T));
        sys_mgmt_dhcp_set(state, TRUE);
    }
#ifdef AIR_SUPPORT_IPV6
    /* Only when the state changes after the address is issued will it be considered a result of DAD. */
    else if (LWIP_NSC_IPV6_ADDR_STATE_CHANGED & reason)
    {
        mw_ipv6_check_and_process_netif_event(cb_args, netif);
    }
#endif

    return;
}
#endif

/* FUNCTION NAME:   sys_mgmt_free_resource
 * PURPOSE:
 *      Free the resources in sys_mgmt init function.
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
MW_ERROR_NO_T sys_mgmt_free_resource(void)
{
    if (osapi_msgDelete(SYS_MGMT_DB_QUEUE_NAME) != MW_E_OK)
    {
        MW_LOG_ERROR(SYSTEM, "Delete queue %s failed !", SYS_MGMT_DB_QUEUE_NAME);
    }

    osapi_threadDelete(sys_mgmt_task_ctx.task_handle);
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    remote_debug_log_free_resource();
#endif
    return MW_E_OK;
}

/* FUNCTION NAME:   sys_mgmt_init
 * PURPOSE:
 *      This sys_mgmt init function.
 *
 * INPUT:
 *      pvParameters
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T sys_mgmt_init(void)
{
    memset(&sys_mgmt_info, 0, sizeof(SYS_MGMT_T));

    /* Create DB client socket */
    if (osapi_msgCreateStatic(SYS_MGMT_DB_QUEUE_NAME,
                              SYS_MGMT_QUEUE_LENGTH,
                              sizeof(void *),
                              (UI8_T *)sys_mgmt_task_ctx.queue_pool,
                              &sys_mgmt_task_ctx.queue_handle) != MW_E_OK)
    {
        MW_LOG_ERROR(SYSTEM, "create queue %s fail", SYS_MGMT_DB_QUEUE_NAME);
        return MW_E_NO_MEMORY;
    }

    if (osapi_msgCreateStatic(SYS_MGMT_DB_BLOCK_QUEUE_NAME,
                              SYS_MGMT_DB_BLOCK_QUEUE_LENGTH,
                              SYS_MGMT_DB_BLOCK_QUEUE_SIZE,
                              (UI8_T *)sys_mgmt_task_ctx.db_queue_pool,
                              &sys_mgmt_task_ctx.db_queue_handle) != MW_E_OK)
    {
        MW_LOG_ERROR(SYSTEM, "create queue %s fail", SYS_MGMT_DB_BLOCK_QUEUE_NAME);
        return MW_E_NO_MEMORY;
    }

#ifdef AIR_SUPPORT_LACP
    if (MW_E_OK != lacp_init((NET_FILTER_T *)&(sys_mgmt_task_ctx.pkt_netf)))
    {
        MW_LOG_ERROR(SYSTEM, "lacp_init fail");
        return MW_E_NOT_INITED;
    }
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    remote_debug_log_init_resource();
#endif
    if (osapi_threadCreateStatic(SYS_MGMT_TASK_NAME,
                       AIR_MAX_SYSMGMT_STACK_SIZE,
                       MW_TASK_PRIORITY_SYSMGMT,
                       sys_mgmt_task,
                       NULL,
                       sys_mgmt_task_ctx.task_stack,
                       &sys_mgmt_task_ctx.task_tcb,
                       &sys_mgmt_task_ctx.task_handle) != MW_E_OK)
    {
        MW_LOG_ERROR(SYSTEM, "create task failed !");
        sys_mgmt_free_resource();
        return MW_E_NO_MEMORY;
    }

#if LWIP_NETIF_EXT_STATUS_CALLBACK
    /* register for netif events when started on first netif */
    netif_add_ext_callback(&sys_mgmt_netif_callback, sys_mgmt_netif_ext_status_callback);
#endif
    /* Initialize system language */
    sys_mgmt_language_init();
    /* Initialize web color info */
    html_config_init_color_info();

#ifdef AIR_SUPPORT_SNTP
    /* Enable system clock count */
    sys_timeout(1000, _mw_systime, NULL);
#endif

    return MW_E_OK;
}

/* FUNCTION NAME:   sys_mgmt_dump
 * PURPOSE:
 *      This API is used to dump SYS_MGMT group and mrouter entry.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_dump()
{
    C8_T   ip_str[SYS_MGMT_IPV4_STR_SIZE];

    MW_CMD_OUTPUT("\nSYS_MGMT:\n");

#ifdef AIR_SUPPORT_SECOND_NETIF
    MW_CMD_OUTPUT("Main Default Netif:\n");
#endif
    MW_CMD_OUTPUT("\tDHCP Mode: %s\n", sys_mgmt_info.dhcp_enable ? "Enable" : "Disable");

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_ip)));
    MW_CMD_OUTPUT("\tStatic IP: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_mask)));
    MW_CMD_OUTPUT("\tStatic MASK: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_gw)));
    MW_CMD_OUTPUT("\tStatic GATEWAY: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_dns)));
    MW_CMD_OUTPUT("\tStatic DNS: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.def_ip)));
    MW_CMD_OUTPUT("\n\tDefault IP: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.def_mask)));
    MW_CMD_OUTPUT("\tDefault MASK: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.def_gw)));
    MW_CMD_OUTPUT("\tDefault GATEWAY: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.def_dns)));
    MW_CMD_OUTPUT("\tDefault DNS: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_ip)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_ip)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_ip));
    }
    MW_CMD_OUTPUT("\n\tOper IP: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_mask)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_mask)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_mask));
    }
    MW_CMD_OUTPUT("\tOper MASK: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_gw)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_gw)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_gw));
    }
    MW_CMD_OUTPUT("\tOper GATEWAY: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_dns)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_dns)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_dns));
    }
    MW_CMD_OUTPUT("\n\tOper DNS: %s\n\n", ip_str);

#ifdef AIR_SUPPORT_SECOND_NETIF
    MW_CMD_OUTPUT("Second Netif:\n");
    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_if2_ip)));
    MW_CMD_OUTPUT("\tStatic IP: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_if2_mask)));
    MW_CMD_OUTPUT("\tStatic MASK: %s\n\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_if2_ip)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_if2_ip)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_if2_ip));
    }
    MW_CMD_OUTPUT("\tOper IP: %s\n", ip_str);

    memset(ip_str, 0, SYS_MGMT_IPV4_STR_SIZE);
    if(IPADDR_ANY == sys_mgmt_info.oper_if2_mask)
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(ip4_addr_get_u32(&sys_mgmt_info.static_if2_mask)));
    }
    else
    {
        MW_UTIL_IPV4_TO_STR(ip_str, PP_HTONL(sys_mgmt_info.oper_if2_mask));
    }
    MW_CMD_OUTPUT("\tOper MASK: %s\n\n", ip_str);
#endif

    MW_CMD_OUTPUT("\tDHCP ACL rule entry-id: %d\n\n", dhcp_acl_id);

    return;
}

/* FUNCTION NAME:   sys_mgmt_dhcp_enable_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable SYS MGMT DHCP admin mode.
 *
 * INPUT:
 *      enable       --  dhcp mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_dhcp_enable_cmd_set(UI8_T enable)
{
    sys_mgmt_queue_send(M_UPDATE, SYS_INFO, SYS_DHCP_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(UI8_T));
    sys_mgmt_dhcp_set(enable, FALSE);

    return;
}

/* FUNCTION NAME:   sys_mgmt_autodns_enable_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable SYS MGMT Auto DNS admin mode.
 *
 * INPUT:
 *      enable       --  Auto DNS mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_autodns_enable_cmd_set(UI8_T enable)
{
    sys_mgmt_info.autodns_enable = enable;
    return;
}
#ifdef AIR_SUPPORT_ICMP_CLIENT
/* FUNCTION NAME:   _sys_mgmt_handle_db_ping_client()
 * PURPOSE:
 *      This API is used for sys_mgmt task handle db notify about ping client.
 *
 * INPUT:
 *      ptr_pload       --  db payload
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_sys_mgmt_handle_db_ping_client(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    if((NULL == ptr_data) || (NULL == ptr_request))
    {
        return MW_E_BAD_PARAMETER;
    }
    if(ptr_request->f_idx == STATUS)
    {
        UI16_T status;
        osapi_memcpy(&status, ptr_data, sizeof(UI16_T));
#if LWIP_RAW
        ping_set_db_ping_status(status);
        ret = MW_E_NOT_SUPPORT;
        if(status == AIR_ICMP_CLIENT_ERR_START)
        {
            ret = ping_create_ping_thread();
            if(MW_E_OK != ret)
            {
                status = AIR_ICMP_CLIENT_ERR_STOPPED;
                sys_mgmt_queue_send(M_UPDATE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &status,sizeof(UI16_T));
                ping_set_db_ping_status(status);
            }
        }
#else  /* LWIP_RAW */
        status = AIR_ICMP_CLIENT_ERR_STOPPED;
        sys_mgmt_queue_send(M_UPDATE, ICMP_CLIENT_INFO, STATUS, DB_ALL_ENTRIES, &status,sizeof(UI16_T));
#endif /* LWIP_RAW */
    }
    return ret;
}
#endif /* AIR_SUPPORT_ICMP_CLIENT */

#ifdef AIR_SUPPORT_MQTTD
/* FUNCTION NAME:   sys_mgmt_mqttd_enable_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable MQTTD admin mode.
 *
 * INPUT:
 *      enable       --  mqttd administrative mode
 *      server_ip    --  remote cloud server
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_mqttd_enable_cmd_set(UI8_T enable, void *server_ip)
{
    sys_mgmt_queue_send(M_UPDATE, MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(UI8_T));
    if (TRUE == enable)
    {
        /* mqttd initialization */
        mqttd_init(server_ip);
    }
    else
    {
        /* mqttd close */
        mqttd_shutdown();
    }
}
#endif

/* FUNCTION NAME: sys_mgmt_language_init()
 * PURPOSE:
 *      Initialize system language.
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
sys_mgmt_language_init(void)
{
    UI32_T          tlv_type_addr = 0;
    UI8_T           lang_idx = 0;

    if(MW_E_OK == mw_is_tlv_data_exist(MW_TLV_TYPE_LANGUAGE, &tlv_type_addr))
    {
        if(MW_E_OK == mw_read_tlv_data(sizeof(UI8_T), (tlv_type_addr + TLV_DATA_HEADER_SIZE), (void *)&lang_idx))
        {
            if(LANG_LAST > lang_idx)
            {
                language_info.lang_idx = lang_idx;
            }
        }
    }
    return MW_E_OK;
}

#ifdef AIR_SUPPORT_SNTP
/* FUNCTION NAME: sys_mgmt_getTimes()
 * PURPOSE:
 *      Get local system time in seconds
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_times   - A point of System times
 *      ptr_date    - A point of struct of Date
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_getTimes(
    UI64_T *ptr_times,
    MW_DATE_T *ptr_date)
{
    UI64_T sec = 0;

    if((NULL == ptr_times) || (NULL == ptr_date))
    {
        return MW_E_BAD_PARAMETER;
    }

    if(TRUE == sntp_on)
    {
        sec = st_second + ((UI64_T)g_local_timezone * 60 * 60);
    }
    else
    {
        sec = st_second;
    }
    *ptr_times = sec;
    mw_timeToDate(sec, ptr_date);

    return MW_E_OK;
}

/* FUNCTION NAME: sys_mgmt_setTimes()
 * PURPOSE:
 *      Convert date to times
 *
 * INPUT:
 *      ptr_date    - A point of struct of Date
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
sys_mgmt_setTimes(
    MW_DATE_T date)
{

    original_st_second = mw_dateToTime(date);
    tickstart = air_util_getSystemSecTick();

    return MW_E_OK;
}
#endif

/* FUNCTION NAME:   sys_mgmt_convert_port_mac
 * PURPOSE:
 *      This API is used to convert mac address to port mac(mac + port ID).
 *
 * INPUT:
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_mac         --  Pointer to the mac address
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
sys_mgmt_convert_port_mac(
    AIR_MAC_T mac,
    UI32_T port)
{
    UI32_T mac_low_bytes = 0;

    osapi_memcpy(&mac_low_bytes, &mac[2], sizeof(UI32_T));
    mac_low_bytes = ntohl(mac_low_bytes);
    mac_low_bytes += port;
    mac_low_bytes = htonl(mac_low_bytes);
    osapi_memcpy(&mac[3], ((UI8_T *)&mac_low_bytes) + 1, 3);
}

