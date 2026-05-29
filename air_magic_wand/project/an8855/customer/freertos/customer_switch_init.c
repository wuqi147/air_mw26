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

/* FILE NAME:  customer_switch_init.c
 * PURPOSE:
 *  Specify customer switch init.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/apps/httpd_opts.h"
#include "lwip/tcpip.h"
#ifdef AIR_SUPPORT_HTTPD_MUTEX
#include "httpd_util.h"
#endif

#include "sdk_ref.h"
#include "osapi_string.h"
#include "mw_error.h"
#include "product.h"
#include "mw_init.h"
#ifdef AIR_SUPPORT_SNMP
#include "lwip/apps/snmp.h"
#if SNMP_USE_NETCONN
#include "lwip/apps/snmp_threadsync.h"
#endif
#endif
#include "customer_switch_init.h"
#include "customer_system.h"
#include "mw_log.h"

extern void pdma_init (void);
extern err_t ethernetif_init( struct netif *xNetIf );
extern err_t mac_rcv_init(void);
extern void sys_mem_init(void);
#if SNMP_USE_NETCONN
extern void snmp_mib2_lwip_synchronizer(snmp_threadsync_called_fn fn, void *arg);
#endif /* SNMP_USE_NETCONN */
extern void customer_ref_preInit(void);
extern void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver);
extern void tftp_init(void);
#if LWIP_HTTPD
extern void httpd_init(void);
#endif

/* NAMING CONSTANT DECLARATIONS
*/
/* MACRO FUNCTION DECLARATIONS
*/
/* GLOBAL VARIABLE DECLARATIONS
 */
#if SNMP_USE_NETCONN
extern struct snmp_threadsync_instance snmp_mib2_lwip_locks;
#endif /* SNMP_USE_NETCONN */

/* LOCAL SUBPROGRAM BODIES
 */
/*FUNCTION NAME: _customer_determine_product_id
* PURPOSE:
*       Determine the product ID. If customers have their own mechanisms to determine
*       the product ID, this function should be rewritten.
* INPUT:
*       None
* OUTPUT:
*       None
* RETURN:
*       The product ID
*/
static const MW_PRODUCT_ID_T
_customer_determine_product_id(
    void)
{
    return mw_product_getProductId();
}

/* EXPORTED SUBPROGRAM BODIES
 */
void lwip_app_init()
{
    /* This call creates the TCP/IP thread. */
    tcpip_init(NULL, NULL);

    /* Create the httpd server from the standard lwIP code.  This demonstrates
    use of the lwIP raw API. */
#if LWIP_HTTPD
    httpd_init();
#endif
#ifdef AIR_SUPPORT_HTTPD_MUTEX
    mw_httpd_mutex_init();
#endif

#if AIR_SUPPORT_SNMP
#if SNMP_USE_NETCONN
    snmp_threadsync_init(&snmp_mib2_lwip_locks, snmp_mib2_lwip_synchronizer);
#endif /* SNMP_USE_NETCONN */
    snmp_init();
#endif
}

void net_interface_init()
{
    ip4_addr_t xIPAddr, xNetMask, xGateway;
    ip_addr_t xDns = IPADDR4_INIT(0);
    static struct netif xNetIf;

    /* Set up the network interface. */
    ip4_addr_set_zero(&xGateway);
    ip4_addr_set_zero(&xIPAddr);
    ip4_addr_set_zero(&xNetMask);

    LWIP_PORT_INIT_GW(&xGateway);
    LWIP_PORT_INIT_IPADDR(&xIPAddr);
    LWIP_PORT_INIT_NETMASK(&xNetMask);
    LWIP_PORT_INIT_DNS(&xDns);

    netif_set_default(netif_add(&xNetIf, &xIPAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input));

#if LWIP_IPV6
    netif_create_ip6_linklocal_address(&xNetIf, 1);
#if LWIP_IPV6_AUTOCONFIG
    xNetIf.ip6_autoconfig_enabled = 1;
#endif /* LWIP_IPV6_AUTOCONFIG */
#endif /* LWIP_IPV6 */

#if LWIP_DNS
    dns_setserver(0,&xDns);
#endif /* LWIP_DNS */
    netif_set_up(&xNetIf);

#if LWIP_IPV6
    MW_LOG_CONSOLE_PRINTF("Initializing IPv6 linklocal address: %s\n", ip6addr_ntoa(netif_ip6_addr(&xNetIf, 0)));
#endif /* LWIP_IPV6 */
}

#ifdef AIR_SUPPORT_SECOND_NETIF
void net_interface2_init()
{
    ip4_addr_t xIPAddr, xNetMask, xGateway;
    ip_addr_t xDns;
    static struct netif xNetIf;

    /* Set up the network interface. */
    ip4_addr_set_zero(&xGateway);
    ip4_addr_set_zero(&xIPAddr);
    ip4_addr_set_zero(&xNetMask);
    ip_addr_set_zero(&xDns);

    LWIP_PORT2_INIT_GW(&xGateway);
    LWIP_PORT2_INIT_IPADDR(&xIPAddr);
    LWIP_PORT2_INIT_NETMASK(&xNetMask);
    LWIP_PORT2_INIT_DNS(&xDns);

    netif_add(&xNetIf, &xIPAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input);

#if LWIP_IPV6
    netif_create_ip6_linklocal_address(&xNetIf, 0);
#if LWIP_IPV6_AUTOCONFIG
    xNetIf.ip6_autoconfig_enabled = 1;
#endif /* LWIP_IPV6_AUTOCONFIG */
#endif /* LWIP_IPV6 */

#if LWIP_DNS
    dns_setserver(0,&xDns);
#endif /* LWIP_DNS */
    netif_set_up(&xNetIf);
    netif_set_link_up(&xNetIf);

#if LWIP_IPV6
    MW_LOG_CONSOLE_PRINTF("Initializing IPv6 linklocal address: %s\n", ip6addr_ntoa(netif_ip6_addr(&xNetIf, 0)));
#endif /* LWIP_IPV6 */
}
#endif

int customer_switch_init(void)
{
    int rc;
    MW_PRODUCT_ID_T product_id = _customer_determine_product_id();

    /* Set the product ID before the system initialization. */
    mw_set_productID(product_id);

    /* Customer system initialization */
    MW_LOG_CONSOLE_PRINTF("Initializing customer system...\n");
    customer_ref_preInit();
    rc = customer_system_init();
    if (E_OK == rc)
    {
        /* sdk references initialization */
        MW_LOG_CONSOLE_PRINTF("Initializing sdk reference...\n");
        rc = sdk_ref_init();
        if (AIR_E_OK == rc)
        {
            /* Customer system post-initialization */
            MW_LOG_CONSOLE_PRINTF("Post-initializing customer system...\n");
            rc = customer_system_post_init();
            if (E_OK != rc)
            {
                MW_LOG_CONSOLE_PRINTF("Error: Failed to post-initialize customer system!\n");
            }
            pdma_init();
            mac_rcv_init();
            sys_mem_init();
            MW_LOG_CONSOLE_PRINTF("Initializing TCP/IP stack...\n");
            lwip_app_init();
#ifndef AIR_SUPPORT_CLI
#if LWIP_UDP
            tftp_init();
#endif
#endif
            /* It should be inited before net_interface_init() to set sys MAC addr first. */
            mw_init_preInitModule(0);
            net_interface_init();
#ifdef AIR_SUPPORT_SECOND_NETIF
            net_interface2_init();
#endif
            /* Middleware module initialization. It should be inited after net_interface_init()
             * to get IPv6 solicited-node address when setting ACL.
             */
            MW_LOG_CONSOLE_PRINTF("Initializing middleware module...\n");
            rc = mw_init_initModule(0);
            if (MW_E_OK != rc)
            {
                MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize middleware module!\n");
            }
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize sdk reference!\n");
        }
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize customer system!\n");
    }

    /* customer TODO */

    return rc;
}
