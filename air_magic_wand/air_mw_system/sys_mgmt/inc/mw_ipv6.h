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

/* FILE NAME:   mw_ipv6.h
 * PURPOSE:
 *      Provide API to config IPv6.
 * NOTES:
 *
 */

#ifndef IPV6_H
#define IPV6_H
#ifdef AIR_SUPPORT_IPV6
/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/nd6.h"
#include "lwip/ip6_addr.h"
#include "lwip/priv/api_msg.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    MW_IPV6_DEFAULT_GATEWAY_MODE_AUTO = 0,
    MW_IPV6_DEFAULT_GATEWAY_MODE_MANUAL,
    MW_IPV6_DEFAULT_GATEWAY_MODE_DELETE
} MW_IPV6_DEFAULT_GATEWAY_MODE;

#define MW_IPV6_WAIT_DAD_INIT              (0)
#define MW_IPV6_WAIT_DAD_SET               (1)

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_IPV6_SEND_DB_MSG(method, t_idx, f_idx, e_idx, ptr_data, size) sys_mgmt_queue_send(method, t_idx, f_idx, e_idx, ptr_data, size)

/* DATA TYPE DECLARATIONS
 */
typedef struct MW_IPV6_INFO_S
{
    ip6_addr_t default_gw;
    UI8_T default_gw_valid;
    UI8_T prefix_len;
    UI8_T ipv6_mode;
    ip6_addr_t manual_ip6_addr;
    UI8_T ip6_addr_netif_idx;
    UI8_T ip6_addr_wait_for_dad;
    UI8_T ip6_addr_dad_result;
    const ip6_addr_t *ptr_ip6_link_local_addr;
    const ip6_addr_t *ptr_ip6_auto_addr_1;
    const ip6_addr_t *ptr_ip6_auto_addr_2;
} MW_IPV6_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const void *ptr_dest);

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
    void);

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
    void);

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
    void);

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
    void);

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
    ip6_addr_t ipv6_addr);

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
    UI8_T prefix_len);

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
    UI8_T prefix_len);

/* FUNCTION NAME:   mw_ipv6_handle_config_change
 * PURPOSE:
 *      Handle the configuration change event from the DB messages.
 *
 * INPUT:
 *      ptr_sys_info             -- DB system information
 * OUTPUT:
 *      None
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void
mw_ipv6_handle_config_change(
    DB_SYS_INFO_T *ptr_sys_info);

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
    struct netif *ptr_netif);

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
    void);

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
    void);

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
    struct netif *ptr_netif);

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
    DB_SYS_INFO_T *ptr_sys_info);

#endif
#endif /* End of IPV6_H */
