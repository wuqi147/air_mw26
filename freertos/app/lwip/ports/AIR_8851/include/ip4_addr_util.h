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

/* FILE NAME:  ip4_addr_util.h
 * PURPOSE:
 *      APIs defines for IPv4 address check.
 * NOTES:
 *      None
 */

#ifndef LWIP_HDR_IP4_ADDR_UTIL_H
#define LWIP_HDR_IP4_ADDR_UTIL_H
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <FreeRTOS.h>
#include "dsh_parser.h"
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include <lwip/ip.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    ip4_addr_t *ptr_netmask);

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
    ip4_addr_t *ptr_netmask);

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
    ip4_addr_t *ptr_netmask);

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
    ip4_addr_t *ptr_netmask);

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
    ip_addr_t *ptr_dnsSrvAddr);

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
    ip4_addr_t *ptr_snmpSrvAddr);

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
    ip4_addr_t *ptr_pingHost);


#endif /* LWIP_HDR_IP4_ADDR_UTIL_H */