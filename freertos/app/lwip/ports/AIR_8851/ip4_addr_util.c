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

/* FILE NAME:   ip4_addr_util.c
 * PURPOSE:
 *      Implement the IPv4 address check.
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "ip4_addr_util.h"

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: ip4_addr_isBroadcast
 * PURPOSE:
 *      Check if the IP address is a broadcast address for the given netmask.
 * INPUT:
 *      ptr_ip      -- IP address to check.
 *      ptr_netmask -- Netmask to use.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE         -- IP address is a broadcast address.
 *      FALSE        -- IP address is not a broadcast address.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_isBroadcast(
    ip4_addr_t *ptr_ip,
    ip4_addr_t *ptr_netmask)
{
    u32_t addr;

    if ((NULL == ptr_ip) || (NULL == ptr_netmask))
    {
        return FALSE;
    }

    addr = ip4_addr_get_u32(ptr_ip);

    /* all ones (broadcast) or all zeroes (old skool broadcast) */
    if ((~addr == IPADDR_ANY) || (addr == IPADDR_ANY))
    {
        return TRUE;
    }
    else if ((addr & (~ip4_addr_get_u32(ptr_netmask))) ==
             (IPADDR_BROADCAST & (~ip4_addr_get_u32(ptr_netmask))))
    {
        /* => network broadcast address */
        return TRUE;
    }

    return FALSE;
}

/* FUNCTION NAME: ip4_addr_netmask_isValid
 * PURPOSE:
 *      Check if the netmask is valid (starting with ones, then only zeros).
 * INPUT:
 *      ptr_netmask -- Netmask to check.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE         -- Netmask is valid.
 *      FALSE        -- Netmask is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_netmask_isValid(
    ip4_addr_t *ptr_netmask)
{
    if ((0 == ip_addr_netmask_valid(ptr_netmask)) || (TRUE == ip4_addr_isany_val(*ptr_netmask)))
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: ip4_addr_ipAddr_isValid
 * PURPOSE:
 *      Check if the IP address is valid.
 * INPUT:
 *      ptr_ip       -- IP address to check.
 *      ptr_netmask  -- Netmask to use.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE         -- IP address is valid.
 *      FALSE        -- IP address is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_ipAddr_isValid(
    ip4_addr_t *ptr_ip,
    ip4_addr_t *ptr_netmask)
{
    if ((TRUE == ip4_addr_isBroadcast(ptr_ip, ptr_netmask)) ||
        (TRUE == ip4_addr_isloopback(ptr_ip)) ||
        (TRUE == ip4_addr_ismulticast(ptr_ip)) ||
        (0 == ip4_addr1_val(*ptr_ip)) ||
        (0 == (ip4_addr_get_u32(ptr_ip) & (~ip4_addr_get_u32(ptr_netmask)))))
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: ip4_addr_gateway_isValid
 * PURPOSE:
 *      Check if the gateway is valid.
 * INPUT:
 *      ptr_gateway  -- Gateway IP address to check.
 *      ptr_netmask  -- Netmask to use.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE         -- Gateway is valid.
 *      FALSE        -- Gateway is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_gateway_isValid(
    ip4_addr_t *ptr_gateway,
    ip4_addr_t *ptr_netmask)
{
    if ((TRUE == ip4_addr_isBroadcast(ptr_gateway, ptr_netmask)) ||
        (TRUE == ip4_addr_isloopback(ptr_gateway)) ||
        (TRUE == ip4_addr_ismulticast(ptr_gateway)) ||
        (0 == ip4_addr1_val(*ptr_gateway)) ||
        (0 == (ip4_addr_get_u32(ptr_gateway) & (~ip4_addr_get_u32(ptr_netmask)))))
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: ip4_addr_dnsSrvAddr_isValid
 * PURPOSE:
 *      Check if the DNS server address is valid.
 * INPUT:
 *      ptr_dnsSrvAddr -- DNS server address to check.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE           -- DNS server address is valid.
 *      FALSE          -- DNS server address is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_dnsSrvAddr_isValid(
    ip_addr_t *ptr_dnsSrvAddr)
{
    if ((0 == ip4_addr1_val(*ip_2_ip4(ptr_dnsSrvAddr))) || (TRUE == ip4_addr_isloopback(ip_2_ip4(ptr_dnsSrvAddr))))
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: ip4_addr_snmpSrvAddr_isValid
 * PURPOSE:
 *      Check if the SNMP trap server address is valid.
 * INPUT:
 *      ptr_snmpSrvAddr -- SNMP trap server address to check.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE            -- SNMP trap server address is valid.
 *      FALSE           -- SNMP trap server address is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_snmpSrvAddr_isValid(
    ip4_addr_t *ptr_snmpSrvAddr)
{
    if ((TRUE == ip4_addr_isloopback(ptr_snmpSrvAddr)) ||
        (0 == ip4_addr1_val(*ptr_snmpSrvAddr)))
    {
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: ip4_addr_pingHost_isValid
 * PURPOSE:
 *      Check if the ping host server address is valid.
 * INPUT:
 *      ptr_pingHost -- ping host address to check.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE         -- ping host address is valid.
 *      FALSE        -- ping host address is not valid.
 * NOTES:
 *      None
 */
BOOL_T
ip4_addr_pingHost_isValid(
    ip4_addr_t *ptr_pingHost)
{
    if (0 == ip4_addr1_val(*ptr_pingHost))
    {
        return FALSE;
    }

    return TRUE;
}