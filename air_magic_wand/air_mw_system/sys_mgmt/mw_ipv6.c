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

/* FILE NAME:   ipv6.c
 * PURPOSE:
 *      Provide API to config IPv6.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef AIR_SUPPORT_IPV6
#include "mw_ipv6.h"
#include "lwip/err.h"
#include "queue.h"
#include "osapi_message.h"
#include "osapi.h"
#include "inet_utils.h"
#include "osapi_string.h"
#include "osapi_thread.h"
#include <mw_utils.h>
#include "mw_portbmp.h"
#include "ethernetif.h"
#include "sys_mgmt.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct MW_IPV6_TCPIP_CALL_MSG_S
{
    struct tcpip_api_call_data call;
    struct netif *netif;
    ip6_addr_t ipv6_addr;
    s8_t result_index;
    u8_t prefix_len;
} MW_IPV6_TCPIP_CALL_MSG_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
MW_IPV6_INFO_T mw_ipv6_info;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   mw_ipv6_get_gateway_hook
 * PURPOSE:
 *      Get the IPv6 default gateway.
 *
 * INPUT:
 *      ptr_netif                -- netif structure
 *      ptr_dest                 -- destination IPv6 address
 * OUTPUT:
 *      None
 * RETURN:
 *      void*                    -- Default gateway IPv6 address
 * NOTES:
 *      None.
 */
const void*
mw_ipv6_get_gateway_hook(
    void *ptr_netif,
    const void *ptr_dest)
{
    MW_LOG_DEBUG(IP6, "%s[%d]: default_gw_valid = %d\n", __func__, __LINE__,
            mw_ipv6_info.default_gw_valid);

    if (mw_ipv6_info.default_gw_valid == MW_IPV6_DEFAULT_GATEWAY_MODE_MANUAL)
    {
        if (mw_ipv6_info.prefix_len != 0)
        {
            if (mw_ipv6_match_prefix(ptr_dest, &mw_ipv6_info.default_gw, mw_ipv6_info.prefix_len))
            {
                MW_LOG_DEBUG(IP6, "%s[%d]: prefix match\n", __func__, __LINE__);
            }
            else
            {
                MW_LOG_DEBUG(IP6, "%s[%d]: prefix not match\n", __func__, __LINE__);
            }
        }
        return &mw_ipv6_info.default_gw;
    }
    else
    {
        return NULL;
    }
}

/* FUNCTION NAME:   _mw_ipv6_add_addr
 * PURPOSE:
 *      Add a IPv6 address to netif and set the life time.
 *
 * INPUT:
 *      ptr_arg                  -- tcp_api_call_data structure
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
_mw_ipv6_add_addr(
    struct tcpip_api_call_data *ptr_arg)
{
    err_t err = ERR_OK;
    MW_IPV6_TCPIP_CALL_MSG_T *ptr_callback_msg = (MW_IPV6_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    MW_LOG_DEBUG(IP6, "%s[%d]: ptr_callback_msg->ipv6_addr = %p\n", __func__, __LINE__, &ptr_callback_msg->ipv6_addr);
    MW_LOG_DEBUG(IP6, "%s[%d]: prefix_len = %d\n", __func__, __LINE__, ptr_callback_msg->prefix_len);

    err = netif_add_ip6_address_with_prefix_len(ptr_callback_msg->netif, (const ip6_addr_t *)&ptr_callback_msg->ipv6_addr,
            &ptr_callback_msg->result_index, ptr_callback_msg->prefix_len);
    /* get the index of the added ipv6 address in netif */
    mw_ipv6_info.ip6_addr_netif_idx = ptr_callback_msg->result_index;
    MW_LOG_DEBUG(IP6, "%s[%d]: ip6_addr_netif_idx = %d\n", __func__, __LINE__, mw_ipv6_info.ip6_addr_netif_idx);
    if (0 == mw_ipv6_info.ip6_addr_netif_idx)
    {
        mw_ipv6_info.ip6_addr_dad_result = IP6_ADDR_DUPLICATED;
        mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_INIT;
        MW_IPV6_SEND_DB_MSG(M_UPDATE, SYS_OPER_INFO, SYS_IP6_DAD_RESULT, DB_ALL_ENTRIES, &mw_ipv6_info.ip6_addr_dad_result, sizeof(UI8_T));
    }
    else
    {
        /* For the manual address, the life time is 0 means the address is manual and has no lifetimes. */
        netif_ip6_addr_set_valid_life(ptr_callback_msg->netif, ptr_callback_msg->result_index, 0);
        netif_ip6_addr_set_pref_life(ptr_callback_msg->netif, ptr_callback_msg->result_index, 0);
    }

    return err;
}

/* FUNCTION NAME:   _mw_ipv6_set_addr_state_invalid
 * PURPOSE:
 *      Set the IPv6 addresses' states to invalid.
 *
 * INPUT:
 *      ptr_arg                  -- tcp_api_call_data structure
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
_mw_ipv6_set_addr_state_invalid(
    struct tcpip_api_call_data *ptr_arg)
{
    err_t err = ERR_OK;
    MW_IPV6_TCPIP_CALL_MSG_T *ptr_callback_msg = (MW_IPV6_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    int i = 0;

    /* Delete dynamic ipv6 address if exist */
    for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        if (ptr_callback_msg->netif->ip6_addr_state[i] != IP6_ADDR_INVALID)
        {
            MW_LOG_DEBUG(IP6, "%s[%d]: netif_ip6_addr_set_state()\n", __func__, __LINE__);
            netif_ip6_addr_set_state(ptr_callback_msg->netif, i, IP6_ADDR_INVALID);
            netif_ip6_addr_set(ptr_callback_msg->netif, i, IP6_ADDR_ANY6);
            netif_ip6_set_prefix_len(ptr_callback_msg->netif, i, ptr_callback_msg->prefix_len);
        }
    }

#if LWIP_IPV6_AUTOCONFIG
    /* Disable ipv6 address autoconfig */
    if (IPV6_CONFIG_MODE_MANUAL == mw_ipv6_info.ipv6_mode)
    {
        ptr_callback_msg->netif->ip6_autoconfig_enabled = 0;
    }
    else
    {
        ptr_callback_msg->netif->ip6_autoconfig_enabled = 1;
    }

    MW_LOG_DEBUG(IP6, "%s[%d]: ip6_autoconfig_enabled = %d\n", __func__, __LINE__, ptr_callback_msg->netif->ip6_autoconfig_enabled);
#endif

    return err;
}

/* FUNCTION NAME:   _mw_ipv6_set_prefix_len
 * PURPOSE:
 *      Config the prefix length for the IPv6 address.
 *
 * INPUT:
 *      ptr_arg                  -- tcp_api_call_data structure
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
_mw_ipv6_set_prefix_len(
    struct tcpip_api_call_data *ptr_arg)
{
    err_t err = ERR_OK;
    MW_IPV6_TCPIP_CALL_MSG_T *ptr_callback_msg = (MW_IPV6_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    MW_LOG_DEBUG(IP6, "%s[%d]: ptr_callback_msg->result_index = %d\n", __func__, __LINE__, ptr_callback_msg->result_index);
    MW_LOG_DEBUG(IP6, "%s[%d]: ptr_callback_msg->prefix_len = %d\n", __func__, __LINE__, ptr_callback_msg->prefix_len);

    err = netif_ip6_set_prefix_len(ptr_callback_msg->netif, ptr_callback_msg->result_index, ptr_callback_msg->prefix_len);

    return err;
}

/* FUNCTION NAME:   mw_ipv6_add_addr
 * PURPOSE:
 *      Add a new IPv6 address.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
mw_ipv6_add_addr(
    void)
{
    err_t err;
    MW_IPV6_TCPIP_CALL_MSG_T msg;

    msg.netif = netif_find_default();
    if (NULL == msg.netif)
    {
        MW_LOG_ERROR(IP6, "%s[%d]: msg.netif is NULL\n", __func__, __LINE__);
        return ERR_VAL;
    }

    ip6_addr_copy(msg.ipv6_addr, mw_ipv6_info.manual_ip6_addr);
    msg.prefix_len = mw_ipv6_info.prefix_len;

    err = tcpip_api_call(_mw_ipv6_add_addr, &msg.call);

    return err;
}

/* FUNCTION NAME:   mw_ipv6_set_addr_state_invalid
 * PURPOSE:
 *      Set the state of the new IPv6 address to invalid.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
mw_ipv6_set_addr_state_invalid(
    void)
{
    err_t err;
    MW_IPV6_TCPIP_CALL_MSG_T msg;

    msg.netif = netif_find_default();
    if (msg.netif == NULL)
    {
        MW_LOG_WARN(IP6, "%s[%d]: msg.netif is NULL\n", __func__, __LINE__);
        return ERR_VAL;
    }
    msg.prefix_len = mw_ipv6_info.prefix_len;

    err = tcpip_api_call(_mw_ipv6_set_addr_state_invalid, &msg.call);

    MW_LOG_DEBUG(IP6, "%s[%d]: mw_ipv6_set_addr_state_invalid NORMAL\n", __func__, __LINE__);

    return err;
}

/* FUNCTION NAME:   mw_ipv6_set_prefix_len
 * PURPOSE:
 *      Config the prefix length for the IPv6 address.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      ERR_OK                   -- Operation success
 * NOTES:
 *      None.
 */
err_t
mw_ipv6_set_prefix_len(
    void)
{
    err_t err = ERR_OK;
    MW_IPV6_TCPIP_CALL_MSG_T msg;

    msg.netif = netif_find_default();
    if (NULL == msg.netif)
    {
        MW_LOG_ERROR(IP6, "%s[%d]: msg.netif is NULL\n", __func__, __LINE__);
        return ERR_VAL;
    }

    msg.prefix_len = mw_ipv6_info.prefix_len;
    msg.result_index = mw_ipv6_info.ip6_addr_netif_idx;

    if (0 != msg.result_index)
    {
        err = tcpip_api_call(_mw_ipv6_set_prefix_len, &msg.call);
    }

    return err;
}

/* FUNCTION NAME:   mw_ipv6_set_netif
 * PURPOSE:
 *      Set a new IPv6 address into netif structure.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None.
 */
void
mw_ipv6_set_netif(
    void)
{
    /* Clear the state of auto generation ipv6 addresses in netif, this operation can delete the auto ipv6 addresses */
    mw_ipv6_set_addr_state_invalid();
    /* Add the new manual address to netif */
    mw_ipv6_add_addr();

}

/* FUNCTION NAME:   mw_ipv6_add_new_addr
 * PURPOSE:
 *      Add a new IPv6 address.
 *
 * INPUT:
 *      ipv6_addr                -- IPv6 address.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None.
 */
void
mw_ipv6_add_new_addr(
    ip6_addr_t ipv6_addr)
{
    ip6_addr_copy(mw_ipv6_info.manual_ip6_addr, ipv6_addr);

    MW_LOG_DEBUG(IP6, "%s[%d]: ip6_addr = %s\n", __func__, __LINE__, (UI8_T *)ip6addr_ntoa(&ipv6_addr));

    mw_ipv6_set_netif();

    return;
}

/* FUNCTION NAME:   mw_ipv6_config_prefix_len
 * PURPOSE:
 *      Config the prefix length for the IPv6 address.
 *
 * INPUT:
 *      prefix_len               -- Prefix length.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None.
 */
void
mw_ipv6_config_prefix_len(
    UI8_T prefix_len)
{
    mw_ipv6_info.prefix_len = prefix_len;

    MW_LOG_DEBUG(IP6, "%s[%d]: prefix_len = %d\n", __func__, __LINE__, mw_ipv6_info.prefix_len);

    mw_ipv6_set_prefix_len();

    return;
}

/* FUNCTION NAME:   mw_ipv6_match_prefix
 * PURPOSE:
 *      Check if the prefixes are the same.
 *
 * INPUT:
 *      ptr_addr1                -- IP6 address 1
 *      ptr_addr2                -- IP6 address 2
 *      prefix_len               -- Prefix length
 * OUTPUT:
 *      None
 * RETURN:
 *      1                        -- Same prefix
 *      0                        -- Different prefix
 * NOTES:
 *      None.
 */
UI8_T
mw_ipv6_match_prefix(
    const ip6_addr_t *ptr_addr1,
    const ip6_addr_t *ptr_addr2,
    UI8_T prefix_len)
{
    UI8_T full_bytes = prefix_len / 8;
    UI8_T remain_bits = prefix_len % 8;
    const UI8_T *ptr_a = (const UI8_T *)ptr_addr1->addr;
    const UI8_T *ptr_b = (const UI8_T *)ptr_addr2->addr;
    UI8_T i = 0;

    /* Check the full bytes */
    for (i = 0; i < full_bytes; i++)
    {
        if (ptr_a[i] != ptr_b[i])
        {
            /* Different prefix */
            return 0;
        }
    }

    /* Check the remaining bits */
    if (remain_bits != 0)
    {
        UI8_T mask = 0xFF << (8 - remain_bits);

        if ((ptr_a[full_bytes] & mask) != (ptr_b[full_bytes] & mask))
        {
            /* Different prefix */
            return 0;
        }
    }

    /* Same prefix */
    return 1;
}

/* FUNCTION NAME:   mw_ipv6_handle_config_change
 * PURPOSE:
 *      Handle the configuration change event from the DB messages.
 *
 * INPUT:
 *      ptr_sys_info                 -- DB system information
 * OUTPUT:
 *      None
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void
mw_ipv6_handle_config_change(
    DB_SYS_INFO_T *ptr_sys_info)
{
    ip6_addr_t ip6_address;

    /* Last time was auto config mode, now switch to manual config mode OR Last time was manual config mode, now change manual ipv6 addr */
    if (((IPV6_CONFIG_MODE_MANUAL == ptr_sys_info->config_mode) && (mw_ipv6_info.ipv6_mode != ptr_sys_info->config_mode)) ||
        ((IPV6_CONFIG_MODE_MANUAL == ptr_sys_info->config_mode) && (mw_ipv6_info.ipv6_mode == ptr_sys_info->config_mode) &&
        (!ip6_addr_cmp_zoneless(&mw_ipv6_info.manual_ip6_addr, &ptr_sys_info->manual_ip6_addr))) ||
        ((IPV6_CONFIG_MODE_MANUAL == ptr_sys_info->config_mode) && (mw_ipv6_info.ipv6_mode == ptr_sys_info->config_mode) &&
        (!ip6_addr_cmp_zoneless(&mw_ipv6_info.default_gw, &ptr_sys_info->manual_ip6_default_gw))) ||
        ((IPV6_CONFIG_MODE_MANUAL == ptr_sys_info->config_mode) && (mw_ipv6_info.ipv6_mode == ptr_sys_info->config_mode) &&
        (mw_ipv6_info.prefix_len != ptr_sys_info->manual_ip6_prefix_len)))
    {
        mw_ipv6_info.ipv6_mode = IPV6_CONFIG_MODE_MANUAL;

        /* Update ipv6 default gateway */
        if ((!ip6_addr_isany(&ptr_sys_info->manual_ip6_default_gw)) && (!ip6_addr_cmp_zoneless(&mw_ipv6_info.default_gw, &ptr_sys_info->manual_ip6_default_gw)))
        {
            mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_MANUAL;
            ip6_addr_copy(mw_ipv6_info.default_gw, ptr_sys_info->manual_ip6_default_gw);
            MW_LOG_DEBUG(SYSTEM, "dbmsg proc: default_gw = %s\n", (UI8_T *)ip6addr_ntoa(&mw_ipv6_info.default_gw));
        }
        /* Delete ipv6 default gateway */
        else if ((ip6_addr_isany(&ptr_sys_info->manual_ip6_default_gw)) && (!ip6_addr_cmp_zoneless(&mw_ipv6_info.default_gw, &ptr_sys_info->manual_ip6_default_gw)))
        {
            mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_DELETE;
            /* Set :: into global variable default_gw which will be read in lwip when call LWIP_HOOK_ND6_GET_GW(). */
            ip6_addr_copy(mw_ipv6_info.default_gw, ptr_sys_info->manual_ip6_default_gw);
            MW_LOG_DEBUG(SYSTEM, "dbmsg proc: default_gw = %s\n", (UI8_T *)ip6addr_ntoa(&mw_ipv6_info.default_gw));
        }

        /* Update ipv6 prefix len */
        if ((0 != ptr_sys_info->manual_ip6_prefix_len) && (mw_ipv6_info.prefix_len != ptr_sys_info->manual_ip6_prefix_len))
        {
            MW_LOG_DEBUG(SYSTEM, "dbmsg proc: Update ipv6 prefix len: %d\n", ptr_sys_info->manual_ip6_prefix_len);
            mw_ipv6_config_prefix_len(ptr_sys_info->manual_ip6_prefix_len);
        }

        /* Update ipv6 manual address */
        if ((!ip6_addr_isany(&ptr_sys_info->manual_ip6_addr)) && (!ip6_addr_cmp_zoneless(&mw_ipv6_info.manual_ip6_addr, &ptr_sys_info->manual_ip6_addr)))
        {
            memcpy(&ip6_address, &ptr_sys_info->manual_ip6_addr, sizeof(ip6_address));
            MW_LOG_DEBUG(SYSTEM, "dbmsg proc: Update ipv6 manual address : %s\n", ip6addr_ntoa(&ip6_address));
            mw_ipv6_add_new_addr(ip6_address);
            /* Set flag to 1 to indicate that the address has been issued. */
            mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_SET;
            ip6_addr_copy(mw_ipv6_info.manual_ip6_addr, ptr_sys_info->manual_ip6_addr);
        }
        /* Delete ipv6 manual address */
        else if ((ip6_addr_isany(&ptr_sys_info->manual_ip6_addr)) && (!ip6_addr_cmp_zoneless(&mw_ipv6_info.manual_ip6_addr, &ptr_sys_info->manual_ip6_addr)))
        {
            memcpy(&ip6_address, &ptr_sys_info->manual_ip6_addr, sizeof(ip6_address));
            MW_LOG_DEBUG(SYSTEM, "dbmsg proc: Delete ipv6 manual address : %s\n", ip6addr_ntoa(&ip6_address));
            mw_ipv6_info.prefix_len = 0;
            mw_ipv6_set_addr_state_invalid();
            ip6_addr_set_zero(&mw_ipv6_info.manual_ip6_addr);
        }
    }
    /* Last time was manual config mode, now switch to auto config mode OR Delete manual config */
    else if ((IPV6_CONFIG_MODE_AUTO == ptr_sys_info->config_mode) && (mw_ipv6_info.ipv6_mode != ptr_sys_info->config_mode))
    {
        /* Set ip6_autoconfig_enabled flag to 1 to enable auto-configuration */
        mw_ipv6_info.ipv6_mode = IPV6_CONFIG_MODE_AUTO;
        mw_ipv6_info.prefix_len = ptr_sys_info->manual_ip6_prefix_len;
        mw_ipv6_set_addr_state_invalid();

        /* Init other mw_ipv6_info fields with default values */
        ip6_addr_set_zero(&mw_ipv6_info.default_gw);
        mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_AUTO;
        ip6_addr_set_zero(&mw_ipv6_info.manual_ip6_addr);
        mw_ipv6_info.ip6_addr_netif_idx = 0;
        mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_INIT;
        mw_ipv6_info.ip6_addr_dad_result = IP6_ADDR_INVALID;
        mw_ipv6_info.ptr_ip6_link_local_addr = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_1 = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_2 = NULL;
    }
}

/* FUNCTION NAME:   mw_ipv6_check_and_process_dad_result
 * PURPOSE:
 *      Check and process the DAD result.
 *
 * INPUT:
 *      ptr_args                 -- args for ext callback
 *      ptr_netif                -- netif
 * OUTPUT:
 *      None
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void
mw_ipv6_check_and_process_dad_result(
    const netif_ext_callback_args_t *ptr_args,
    struct netif *ptr_netif)
{
    I8_T index = ptr_args->ipv6_addr_state_changed.addr_index;

    if ((index < 0) || (index >= LWIP_IPV6_NUM_ADDRESSES))
    {
        MW_LOG_ERROR(IP6, "%s[%d]: Invalid index %d\n", __func__, __LINE__);
        return;
    }

    mw_ipv6_info.ip6_addr_dad_result = ptr_netif->ip6_addr_state[index];
    MW_LOG_DEBUG(IP6, "%s[%d]: index: %d, new state : %d\n", __func__, __LINE__, index, mw_ipv6_info.ip6_addr_dad_result);
    switch (mw_ipv6_info.ip6_addr_dad_result)
    {
        case IP6_ADDR_INVALID:
        case IP6_ADDR_VALID:
        case IP6_ADDR_PREFERRED:
        case IP6_ADDR_DUPLICATED:
            mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_INIT;
            break;
        case IP6_ADDR_TENTATIVE:
        case IP6_ADDR_TENTATIVE_1:
        case IP6_ADDR_TENTATIVE_2:
        case IP6_ADDR_TENTATIVE_3:
        case IP6_ADDR_TENTATIVE_4:
        case IP6_ADDR_TENTATIVE_5:
        case IP6_ADDR_TENTATIVE_6:
        case IP6_ADDR_TENTATIVE_7:
            break;
        default:
            MW_LOG_ERROR(IP6, "%s[%d]: Invalid dad state: %d\n", __func__, __LINE__, mw_ipv6_info.ip6_addr_dad_result);
            return;
    }
}

/* FUNCTION NAME:   mw_ipv6_check_and_process_auto_addr
 * PURPOSE:
 *      Set the newest IPv6 address(es) which have been obtained from lwip to the global variable.
 *
 * INPUT:
 *      None.
 * OUTPUT:
 *      None
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void
mw_ipv6_check_and_process_auto_addr(
    void)
{
    UI8_T i = 0;
    UI8_T addr_state = IP6_ADDR_INVALID;
    struct netif *netif;

    NETIF_FOREACH(netif)
    {
        for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; ++i)
        {
            if (!ip_addr_isany(&((netif)->ip6_addr[i])))
            {
                MW_LOG_DEBUG(SYSTEM, "%s[%d]: Ready to get ip6AutoAddr\n", __func__, __LINE__);

                addr_state = netif_ip6_addr_state(netif, i);
                if (ip6_addr_isvalid(addr_state))
                {
                    MW_LOG_DEBUG(SYSTEM, "%s[%d]: ip6AutoAddr is valid\n", __func__, __LINE__);
                    if (1 == i)
                    {
                        mw_ipv6_info.ptr_ip6_auto_addr_1 = netif_ip6_addr(netif, i);
                    }
                    else if (2 == i)
                    {
                        mw_ipv6_info.ptr_ip6_auto_addr_2 = netif_ip6_addr(netif, i);
                    }
                }
                else
                {
                    MW_LOG_DEBUG(SYSTEM, "%s[%d]: ip6AutoAddr is invalid\n", __func__, __LINE__);
                    if (1 == i)
                    {
                        mw_ipv6_info.ptr_ip6_auto_addr_1 = NULL;
                    }
                    else if (2 == i)
                    {
                        mw_ipv6_info.ptr_ip6_auto_addr_2 = NULL;
                    }
                }
            }
            else
            {
                if (1 == i)
                {
                    MW_LOG_DEBUG(SYSTEM, "%s[%d]: Noip6AutoAddr_1\n", __func__, __LINE__);
                    mw_ipv6_info.ptr_ip6_auto_addr_1 = NULL;
                }
                else if (2 == i)
                {
                    MW_LOG_DEBUG(SYSTEM, "%s[%d]: Noip6AutoAddr_2\n", __func__, __LINE__);
                    mw_ipv6_info.ptr_ip6_auto_addr_2 = NULL;
                }
            }
        }
        mw_ipv6_info.ptr_ip6_link_local_addr = netif_ip6_addr(netif, 0);
    }
}

/* FUNCTION NAME:   mw_ipv6_get_info
 * PURPOSE:
 *      Get the address of global variable mw_ipv6_info.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      Address of global variable mw_ipv6_info.
 * NOTES:
 *      None.
 */
MW_IPV6_INFO_T*
mw_ipv6_get_info(
    void)
{
    return &mw_ipv6_info;
}

/* FUNCTION NAME:   mw_ipv6_check_and_process_netif_event
 * PURPOSE:
 *      Handle IPv6 DAD result event and addr state changed event.
 *
 * INPUT:
 *      ptr_args                 -- args for ext callback
 *      ptr_netif                -- netif
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
mw_ipv6_check_and_process_netif_event(
    const netif_ext_callback_args_t *ptr_args,
    struct netif *ptr_netif)
{
    if (MW_IPV6_WAIT_DAD_SET == mw_ipv6_info.ip6_addr_wait_for_dad)
    {
        mw_ipv6_check_and_process_dad_result(ptr_args, ptr_netif);
        if (IP6_ADDR_INVALID == mw_ipv6_info.ip6_addr_dad_result || IP6_ADDR_VALID == mw_ipv6_info.ip6_addr_dad_result ||
            IP6_ADDR_PREFERRED == mw_ipv6_info.ip6_addr_dad_result || IP6_ADDR_DUPLICATED == mw_ipv6_info.ip6_addr_dad_result)
        {
            MW_IPV6_SEND_DB_MSG(M_UPDATE, SYS_OPER_INFO, SYS_IP6_DAD_RESULT, DB_ALL_ENTRIES, &mw_ipv6_info.ip6_addr_dad_result, sizeof(UI8_T));
        }
    }
    else
    {
        mw_ipv6_check_and_process_auto_addr();

        if (mw_ipv6_info.ptr_ip6_auto_addr_1)
        {
            MW_IPV6_SEND_DB_MSG(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP6_ADDR_1, DB_ALL_ENTRIES, mw_ipv6_info.ptr_ip6_auto_addr_1, sizeof(ip6_addr_t));
        }

        if (mw_ipv6_info.ptr_ip6_auto_addr_2)
        {
            MW_IPV6_SEND_DB_MSG(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP6_ADDR_2, DB_ALL_ENTRIES, mw_ipv6_info.ptr_ip6_auto_addr_2, sizeof(ip6_addr_t));
        }

        if (mw_ipv6_info.ptr_ip6_link_local_addr)
        {
            MW_IPV6_SEND_DB_MSG(M_UPDATE, SYS_OPER_INFO, SYS_OPER_IP6_LINK_LOCAL_ADDR, DB_ALL_ENTRIES, mw_ipv6_info.ptr_ip6_link_local_addr, sizeof(ip6_addr_t));
        }
    }
}

/* FUNCTION NAME:   mw_ipv6_init_info
 * PURPOSE:
 *      Initialize the global variable mw_ipv6_info when the device startups.
 *
 * INPUT:
 *      ptr_sys_info                 -- DB system information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
mw_ipv6_init_info(
    DB_SYS_INFO_T *ptr_sys_info)
{
    MW_LOG_DEBUG(SYSTEM, "%s[%d]: Initialize mw_ipv6_info\n", __func__, __LINE__);

    if (IPV6_CONFIG_MODE_AUTO == ptr_sys_info->config_mode)
    {
        /* Set ip6_autoconfig_enabled flag to 1 to enable auto-configuration */
        mw_ipv6_info.ipv6_mode = IPV6_CONFIG_MODE_AUTO;
        mw_ipv6_info.prefix_len = MW_IPV6_SLAAC_PREFIX_LEN;
        mw_ipv6_set_addr_state_invalid();

        /* Init other mw_ipv6_info fields with default values */
        ip6_addr_set_zero(&mw_ipv6_info.default_gw);
        mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_AUTO;
        ip6_addr_set_zero(&mw_ipv6_info.manual_ip6_addr);
        mw_ipv6_info.ip6_addr_netif_idx = 0;
        mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_INIT;
        mw_ipv6_info.ip6_addr_dad_result = IP6_ADDR_INVALID;
        mw_ipv6_info.ptr_ip6_link_local_addr = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_1 = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_2 = NULL;
    }
    else
    {
        /* Set ip6_autoconfig_enabled flag to 0 to disable auto-configuration AND Check manual ipv6 addr for dad */
        mw_ipv6_info.ipv6_mode = IPV6_CONFIG_MODE_MANUAL;
        mw_ipv6_info.prefix_len = ptr_sys_info->manual_ip6_prefix_len;
        mw_ipv6_set_addr_state_invalid();

        if (ip6_addr_isany(&ptr_sys_info->manual_ip6_addr))
        {
            ip6_addr_set_zero(&mw_ipv6_info.manual_ip6_addr);
            mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_INIT;
        }
        else
        {
            ip6_addr_copy(mw_ipv6_info.manual_ip6_addr, ptr_sys_info->manual_ip6_addr);
            mw_ipv6_info.ip6_addr_wait_for_dad = MW_IPV6_WAIT_DAD_SET;
            mw_ipv6_add_new_addr(mw_ipv6_info.manual_ip6_addr);
        }

        if (ip6_addr_isany(&ptr_sys_info->manual_ip6_default_gw))
        {
            ip6_addr_set_zero(&mw_ipv6_info.default_gw);
            mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_AUTO;
        }
        else
        {
            ip6_addr_copy(mw_ipv6_info.default_gw, ptr_sys_info->manual_ip6_default_gw);
            mw_ipv6_info.default_gw_valid = MW_IPV6_DEFAULT_GATEWAY_MODE_MANUAL;
        }

        /* Init other mw_ipv6_info fields with default values */
        mw_ipv6_info.ip6_addr_netif_idx = 0;
        mw_ipv6_info.ip6_addr_dad_result = IP6_ADDR_INVALID;
        mw_ipv6_info.ptr_ip6_link_local_addr = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_1 = NULL;
        mw_ipv6_info.ptr_ip6_auto_addr_2 = NULL;
    }
}
#endif
