/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
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
#include "customer_switch_init.h"
#include "customer_system.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"

#include "sdk_ref.h"
#include "timer.h"

/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/

extern void pdma_init (void);
extern err_t ethernetif_init( struct netif *xNetIf );
extern err_t mac_rcv_init(void);
extern void sys_mem_init(void);

#if SNMP_USE_NETCONN
#include "lwip/apps/snmp_threadsync.h"

extern struct snmp_threadsync_instance snmp_mib2_lwip_locks;
extern void snmp_mib2_lwip_synchronizer(snmp_threadsync_called_fn fn, void *arg);
#endif /* SNMP_USE_NETCONN */

void lwip_app_init()
{
    /* This call creates the TCP/IP thread. */
    tcpip_init(NULL, NULL);

#if LWIP_SNMP
#if SNMP_USE_NETCONN
    snmp_threadsync_init(&snmp_mib2_lwip_locks, snmp_mib2_lwip_synchronizer);
#endif /* SNMP_USE_NETCONN */
    snmp_init();
#endif
}

void net_interface_init()
{
    ip4_addr_t xIPAddr, xNetMask, xGateway;
    ip_addr_t xDns;
    static struct netif xNetIf;

    /* Set up the network interface. */
    ip4_addr_set_zero(&xGateway);
    ip4_addr_set_zero(&xIPAddr);
    ip4_addr_set_zero(&xNetMask);
    ip_addr_set_zero(&xDns);

    LWIP_PORT_INIT_GW(&xGateway);
    LWIP_PORT_INIT_IPADDR(&xIPAddr);
    LWIP_PORT_INIT_NETMASK(&xNetMask);
    LWIP_PORT_INIT_DNS(&xDns);

    netif_set_default(netif_add(&xNetIf, &xIPAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input));

#if LWIP_IPV6
    printf("MAC address: %x%x%x%x%x%x\n", xNetIf.hwaddr[0], xNetIf.hwaddr[1], xNetIf.hwaddr[2], xNetIf.hwaddr[3], xNetIf.hwaddr[4], xNetIf.hwaddr[5]);
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
    printf("IPv6 linklocal address: %s; addr_state:%d\n", ip6addr_ntoa(netif_ip6_addr(&xNetIf, 0)), netif_ip6_addr_state(&xNetIf, 0));
#endif /* LWIP_IPV6 */
}


int customer_switch_init(void)
{
    int rc;

    /* Customer system initialization */
    printf("Initializing customer system...\n");
    rc = customer_system_init();
    if (E_OK == rc)
    {
        /* sdk references initialization */
        printf("Initializing sdk reference...\n");
        rc = sdk_ref_init();
        if (E_OK == rc)
        {
            /* Customer system post-initialization */
            printf("Post-initializing customer system...\n");
            rc = customer_system_post_init();
            if (E_OK != rc)
            {
                printf("Error: Failed to post-initialize customer system!\n");
            }
            pdma_init();
            mac_rcv_init();
            sys_mem_init();
            lwip_app_init();
#if LWIP_UDP
            tftp_init();
#endif
            net_interface_init();
        }
        else
        {
            printf("Error: Failed to initialize sdk reference!\n");
        }
    }
    else
    {
        printf("Error: Failed to initialize customer system!\n");
    }

    return E_OK;
}
