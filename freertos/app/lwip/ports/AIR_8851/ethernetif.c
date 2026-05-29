/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* WinPCap includes. */
#define HAVE_REMOTE

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* lwIP includes. */
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include <lwip/ethip6.h>
#include "ethernetif.h"
#include "customer_network.h"

#include "air_stag.h"
#ifdef AIR_MW_SUPPORT
#include <mw_platform.h>
#include "mw_msg.h"
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
#include "vlan_utils.h"
#endif /*AIR_SUPPORT_MGMT_VLAN_CFG*/
#endif

/* Define those to better describe your network interface. */
#define IFNAME0 'w'
#define IFNAME1 'p'
#define netifMAX_MTU 1500
#define netifTX_BUF_LEN 1520

#define NET_PDMA_MSG_QUEUE_SIZE  (24)

#if STAG_INSERT_MODE
#ifdef AIR_LITE_HTTPD
#define NET_PROTOCOL_REGISTER_PBUF_QUEUE_SIZE (4)
#else
#define NET_PROTOCOL_REGISTER_PBUF_QUEUE_SIZE (8)
#endif

#define BIT(nr)                           (1UL << (nr))
#define BITS(m, n)                        (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#define BITS_OFF_R(val, offset, range)    (((val) >> offset) & (BITS(0, (range) - 1)))

#define MAC_RCV_RX_PKT_WDOG_KICK_NUM      (80)

#define GET_32(__ptr_data__, __rval__) do   \
    {                                       \
        u8_t *ptr_data = (__ptr_data__);    \
        (__rval__) = *ptr_data++;           \
        (__rval__) <<= 8;                   \
        (__rval__) |= *ptr_data++;          \
        (__rval__) <<= 8;                   \
        (__rval__) |= *ptr_data++;          \
        (__rval__) <<= 8;                   \
        (__rval__) |= *ptr_data++;          \
    } while(0)

#define GET_16(__ptr_data__, __rval__) do   \
    {                                       \
        u8_t *ptr_data = (__ptr_data__);    \
        (__rval__) = *ptr_data++;           \
        (__rval__) <<= 8;                   \
        (__rval__) |= *ptr_data++;          \
    } while(0)

#endif

struct xEthernetIf
{
    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
};


#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
extern u32_t sys_mgmt_get_mgmt_vid_pbmp();
#endif
extern void* pdma_get_ethernet_pkt_buf(unsigned int index, unsigned int* ptr_len);
extern void pdma_release_ethernet_pkt_buf(unsigned int index);
extern int pdma_tx_pkt(unsigned char* data, int len, int txq, int cp_to_sec_desc);
#if STAG_INSERT_MODE
static NET_FILTER_T *prvFoundRegisterHandle(u8_t protocol);
#endif


/* queue recv pkt from underlayer */
GDMPSRAM_DATA QueueHandle_t g_pkt_queue = NULL;
GDMPSRAM_BSS SemaphoreHandle_t g_pkt_tx_mutex;

#if STAG_INSERT_MODE
GDMPSRAM_BSS QueueSetHandle_t g_pkt_queue_set;
GDMPSRAM_DATA QueueHandle_t g_pkt_reg_queue = NULL;
/* middleware register info */
GDMPSRAM_DATA static NET_FILTER_T *net_protos[NET_PROTOCOL_MAX_NUM] = {0};
#endif

GDMPSRAM_DATA static int mac_rcv_is_ready = FALSE;

/* The network interface that was opened. */
GDMPSRAM_DATA static struct netif *pxlwIPNetIf = NULL;

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param pxNetIf the already initialized lwip network interface structure
 *      for this ethernetif.
 */
static void prvLowLevelInit( struct netif *pxNetIf )
{
    /* set MAC hardware address length */
    pxNetIf->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
#ifdef PLAT_MAC_ADDR
    pxNetIf->hwaddr[ 0 ] = PLAT_MAC_ADDR[0];
    pxNetIf->hwaddr[ 1 ] = PLAT_MAC_ADDR[1];
    pxNetIf->hwaddr[ 2 ] = PLAT_MAC_ADDR[2];
    pxNetIf->hwaddr[ 3 ] = PLAT_MAC_ADDR[3];
    pxNetIf->hwaddr[ 4 ] = PLAT_MAC_ADDR[4];
    pxNetIf->hwaddr[ 5 ] = PLAT_MAC_ADDR[5];
#else//PLAT_MAC_ADDR
    pxNetIf->hwaddr[ 0 ] = 0x00;
    pxNetIf->hwaddr[ 1 ] = 0xaa;
    pxNetIf->hwaddr[ 2 ] = 0xbb;
    pxNetIf->hwaddr[ 3 ] = 0x11;
    pxNetIf->hwaddr[ 4 ] = 0x22;
    pxNetIf->hwaddr[ 5 ] = 0x33;
#endif//PLAT_MAC_ADDR

    /* device capabilities */
    /* don't set pxNetIf_FLAG_ETHARP if this device is not an ethernet one */
#if LWIP_IPV6
    netif_set_flags(pxNetIf, (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_MLD6));
#else
#ifdef AIR_MW_SUPPORT
    netif_set_flags(pxNetIf, (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP));
#else
    netif_set_flags(pxNetIf, (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP));
#endif /* AIR_MW_SUPPORT */
#endif /* LWIP_IPV6 */
#ifndef AIR_SUPPORT_SECOND_NETIF
    /* Remember which interface was opened as it is used in the interrupt simulator task. */
    pxlwIPNetIf = pxNetIf;
#endif
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param pxNetIf the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *       an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *     strange results. You might consider waiting for space in the DMA queue
 *     to become availale since the stack doesn't retry to send a packet
 *     dropped because of memory failure (except for the TCP timers).
 */
static err_t prvLowLevelOutput( struct netif *pxNetIf, struct pbuf *p )
{
    struct pbuf *q;
    GDMPSRAM_BSS static unsigned char ucBuffer[ netifTX_BUF_LEN ];
    unsigned char *pucBuffer = NULL;
    unsigned char *pucChar;
    struct eth_hdr *pxHeader;
    u16_t usTotalLength = p->tot_len - ETH_PAD_SIZE;
    err_t xReturn = ERR_OK;

    #if defined(LWIP_DEBUG) && LWIP_NETIF_TX_SINGLE_PBUF
        LWIP_ASSERT("p->next == NULL && p->len == p->tot_len", p->next == NULL && p->len == p->tot_len);
    #endif

    xSemaphoreTake(g_pkt_tx_mutex, portMAX_DELAY);

    #ifdef AIR_LITE_HTTPD
    pucBuffer = pdma_get_tx_pkt_buf(usTotalLength, 0);
    if (!pucBuffer)
    {
        LINK_STATS_INC( link.memerr );
        LINK_STATS_INC( link.drop );
        snmp_inc_ifoutdiscards( pxNetIf );
        xSemaphoreGive(g_pkt_tx_mutex);
        return ERR_BUF;
    }
    #else
    pucBuffer = ucBuffer;
    #endif

    /* Initiate transfer. */
    if( p->len == p->tot_len )
    {
        /* No pbuf chain, don't have to copy -> faster. */
        #ifdef AIR_LITE_HTTPD
        memcpy(pucBuffer, &( ( unsigned char * ) p->payload )[ ETH_PAD_SIZE ], p->len);
        #else
        pucBuffer = &( ( unsigned char * ) p->payload )[ ETH_PAD_SIZE ];
        #endif
    }
    else
    {
        /* pbuf chain, copy into contiguous ucBuffer. */
        if( p->tot_len >= netifTX_BUF_LEN )
        {
            LINK_STATS_INC( link.lenerr );
            LINK_STATS_INC( link.drop );
            snmp_inc_ifoutdiscards( pxNetIf );
            xReturn = ERR_BUF;
        }
        else
        {
            #ifdef AIR_LITE_HTTPD
            pucChar = pucBuffer;
            #else
            pucChar = ucBuffer;
            #endif

            for( q = p; q != NULL; q = q->next )
            {
                /* Send the data from the pbuf to the interface, one pbuf at a
                time. The size of the data in each pbuf is kept in the ->len
                variable. */
                /* send data from(q->payload, q->len); */
                LWIP_DEBUGF( NETIF_DEBUG, ("NETIF: send pucChar %p q->payload %p q->len %i q->next %p\n", pucChar, q->payload, ( int ) q->len, ( void* ) q->next ) );
                if( q == p )
                {
                    memcpy( pucChar, &( ( char * ) q->payload )[ ETH_PAD_SIZE ], q->len - ETH_PAD_SIZE );
                    pucChar += q->len - ETH_PAD_SIZE;
                }
                else
                {
                    memcpy( pucChar, q->payload, q->len );
                    pucChar += q->len;
                }
            }
        }
    }

    if( xReturn == ERR_OK )
    {
        /* signal that packet should be sent */
        if(pdma_tx_pkt(pucBuffer, usTotalLength, 0, 0) < 0)
        {
            LINK_STATS_INC( link.memerr );
            LINK_STATS_INC( link.drop );
            snmp_inc_ifoutdiscards( pxNetIf );
            xReturn = ERR_BUF;
        }
        else
        {
            LINK_STATS_INC( link.xmit );
            snmp_add_ifoutoctets( pxNetIf, usTotalLength );
            pxHeader = ( struct eth_hdr * )p->payload;

            if( ( pxHeader->dest.addr[ 0 ] & 1 ) != 0 )
            {
                /* broadcast or multicast packet*/
                snmp_inc_ifoutnucastpkts( pxNetIf );
            }
            else
            {
                /* unicast packet */
                snmp_inc_ifoutucastpkts( pxNetIf );
            }
        }
    }

    xSemaphoreGive(g_pkt_tx_mutex);
    return xReturn;
}

/**
 * Send the pbuf to the mapping protocol's queue handle
 *
 * @param p the point to the packet buffer
 * @param proto the mapping protocol number in enum
 */
void sendPbufToNetProtoQueue(struct pbuf *p, u8_t proto)
{
#ifdef AIR_MW_SUPPORT
    return ethernetif_sendPbufToRegisterQueue(p, proto);
#else
#if STAG_INSERT_MODE
    NET_FILTER_T *ptr_net_proto = NULL;

    ptr_net_proto = prvFoundRegisterHandle(proto);
    if (NULL == ptr_net_proto)
    {
        LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Invalid proto\n"));
        pbuf_free(p);
        return;
    }

    if (ptr_net_proto->handle != NULL)
    {
        LWIP_DEBUGF(PBUF_PARSE_DEBUG, ( "Send to protocol %d, handle %p \n", proto, ptr_net_proto->handle) );

        if (pdPASS == (xQueueSend(ptr_net_proto->handle, &p, 0UL))) {
            return;
        }
    }
#endif

    /* Cannot found any handle */
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Cannot found any module handle\n"));
    pbuf_free(p);
#endif
}

#if STAG_INSERT_MODE
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
static AIR_ERROR_NO_T prvEtherHeaderParseForARP(struct pbuf *p)
{
    u8_t i = 0;
    u16_t u16dat = 0;
    u32_t u32dat = 0;
    /* set rc as bad parameter, and set as ok when find valid ARP */
    AIR_ERROR_NO_T rc = AIR_E_BAD_PARAMETER;

    AIR_PORT_BITMAP_T            mgmt_vlan_pbmp_total_untag;
    u16_t                        mgmt_vlan_id;
    u8_t                         mgmt_vlan_mode;
    sys_mgmt_get_mgmt_vid_pbmp(&mgmt_vlan_mode, &mgmt_vlan_id, NULL, &mgmt_vlan_pbmp_total_untag);

    /* untag frame */
    if (0 == p->vlan_num)
    {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
            ("etharp_input: untag frame\n"));

        if(0 != AIR_PORT_CHK(mgmt_vlan_pbmp_total_untag, p->stag_hdr.rx_hdr.sp))
        {
            rc = AIR_E_OK;
        }
        else
        {
            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
                ("etharp_input: untag frame, sp not in mgmt vlan untag port (%"U16_F")\n",
                 p->stag_hdr.rx_hdr.sp));
        }
    }
    /* tag frame */
    else
    {
        u16_t  tmp_len = p->len;
        u16_t  tot_len = p->tot_len;
        u8_t   pri_tag_processed = 0;
        u8_t   ctag_processed = 0;

        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
            ("etharp_input: tag frame\n"));

        /* move pointer to check VLAN header */
        pbuf_add_header(p, (SIZEOF_ETH_TYPE + (p->vlan_num * SIZEOF_VLAN_HDR)));

        for(i = PKT_FIRST_VLAN_TAG; i < PKT_MAX_VLAN_TAG; i++)
        {
            GET_32(p->payload, u32dat);

            if((ETHTYPE_VLAN == BITS_OFF_R(u32dat, 16, 16)) || (0x88A8 == BITS_OFF_R(u32dat, 16, 16)) || (ETHTYPE_QINQ == BITS_OFF_R(u32dat, 16, 16)))
            {
                p->payload += SIZEOF_VLAN_HDR;
                p->len -= SIZEOF_VLAN_HDR;
                p->tot_len -= SIZEOF_VLAN_HDR;

                if((ETHTYPE_VLAN == BITS_OFF_R(u32dat, 16, 16)) && AIR_E_OK != rc)
                {
                    /* Priority Tag */
                    if(0 == BITS_OFF_R(u32dat, 0, 12))
                    {
                        pri_tag_processed = 1;
                        if(0 != AIR_PORT_CHK(mgmt_vlan_pbmp_total_untag, p->stag_hdr.rx_hdr.sp))
                        {
                            rc = AIR_E_OK;
                        }
                        else
                        {
                            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
                                    ("etharp_input: priority frame, sp not in mgmt vlan untag port (%"U16_F")\n",
                                     p->stag_hdr.rx_hdr.sp));
                        }
                    }
                    /* CTAG */
                    else
                    {
                        ctag_processed = 1;
                        /* VID match */
                        if (VLAN_1Q_ENABLE == mgmt_vlan_mode)
                        {
                            if(mgmt_vlan_id == BITS_OFF_R(u32dat, 0, 12))
                            {
                                rc = AIR_E_OK;
                            }
                            else
                            {
                                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
                                            ("etharp_input: tagged frame, vid not match (%"U16_F"/%"U16_F")\n",
                                             BITS_OFF_R(u32dat, 0, 12), mgmt_vlan_id));
                            }
                        }
                        else
                        {
                            rc = AIR_E_OK;
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }

        /* without ctag */
        if (0 == pri_tag_processed && 0 == ctag_processed)
        {
            if(0 != AIR_PORT_CHK(mgmt_vlan_pbmp_total_untag, p->stag_hdr.rx_hdr.sp))
            {
                rc = AIR_E_OK;
            }
            else
            {
                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING,
                    ("etharp_input: no ctag, sp not in mgmt vlan untag port (%"U16_F")\n",
                     p->stag_hdr.rx_hdr.sp));
            }
        }

        /* compare length between before and after pointer move */
        p->payload += SIZEOF_ETH_TYPE;
        p->len -= SIZEOF_ETH_TYPE;
        p->tot_len -= SIZEOF_ETH_TYPE;
        if (p->len != tmp_len || p->tot_len != tot_len)
        {
            LWIP_ASSERT("Payload pointer move error!", FALSE);
        }
    }

    return rc;

}
#endif

/**
 * Parse the ethernet header in pbuf payload and store the infomation in
 * pbuf structure.
 *
 * @param p the point to the packet buffer
 */
static AIR_ERROR_NO_T prvEtherHeaderParse(struct pbuf *p)
{
    u8_t i = 0;
    u16_t u16dat = 0;
    u32_t u32dat = 0;
    AIR_STAG_RX_PARA_T stag_rx = {0};
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Ethernet DA */
    MEMCPY((void *) &(p->ether_hdr.dest), (void *) p->payload, ETH_HWADDR_LEN);
    p->payload += ETH_HWADDR_LEN;
    p->len -= ETH_HWADDR_LEN;
    p->tot_len -= ETH_HWADDR_LEN;

    /* Ethernet SA */
    MEMCPY((void *) &(p->ether_hdr.src), (void *) p->payload, ETH_HWADDR_LEN);
    p->payload += ETH_HWADDR_LEN;
    p->len -= ETH_HWADDR_LEN;
    p->tot_len -= ETH_HWADDR_LEN;

    /* Special Tag */
    GET_32(p->payload, u32dat);
    u32dat = htonl(u32dat);
    rc = air_stag_decodeRxStag(0, (const UI8_T*)&u32dat, sizeof(u32dat), &stag_rx);
    if (AIR_E_OK != rc)
    {
        LWIP_ASSERT("stag decode fail in prvEtherHeaderParse!", (AIR_E_OK == rc));
    }

    p->payload += SIZEOF_STAG_HDR;
    p->len -= SIZEOF_STAG_HDR;
    p->tot_len -= SIZEOF_STAG_HDR;

    p->stag_hdr.rx_hdr.rsn = stag_rx.rsn;
    p->stag_hdr.rx_hdr.vpm = stag_rx.vpm;
    p->stag_hdr.rx_hdr.sp  = stag_rx.sp;
    p->stag_hdr.rx_hdr.pcp = stag_rx.pri;
    p->stag_hdr.rx_hdr.dei = stag_rx.cfi;
    p->stag_hdr.rx_hdr.vid = stag_rx.vid;

    /* VLAN header */
    for(i = PKT_FIRST_VLAN_TAG; i < PKT_MAX_VLAN_TAG; i++)
    {
        GET_32(p->payload, u32dat);

        if((ETHTYPE_VLAN == BITS_OFF_R(u32dat, 16, 16)) || (0x88A8 == BITS_OFF_R(u32dat, 16, 16)) || (ETHTYPE_QINQ == BITS_OFF_R(u32dat, 16, 16)))
        {
            if(PKT_FIRST_VLAN_TAG == i)
            {
                p->vlan_hdr.tpid = BITS_OFF_R(u32dat, 16, 16); // TPID
                p->vlan_hdr.priority = BITS_OFF_R(u32dat, 13, 3); // User Priority
                p->vlan_hdr.cfi = BITS_OFF_R(u32dat, 12, 1); // CFI
                p->vlan_hdr.vid = BITS_OFF_R(u32dat, 0, 12); // VLAN ID
            }

            p->vlan_num += 1;
            p->payload += SIZEOF_VLAN_HDR;
            p->len -= SIZEOF_VLAN_HDR;
            p->tot_len -= SIZEOF_VLAN_HDR;
        }
        else
        {
            break;
        }
    }

    /* Ethernet Type */
    GET_16(p->payload, u16dat);
    p->ether_hdr.type = u16dat;
    p->payload += SIZEOF_ETH_TYPE;
    p->len -= SIZEOF_ETH_TYPE;
    p->tot_len -= SIZEOF_ETH_TYPE;

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    /* Unexpected ARP handling */
    if (ETHTYPE_ARP == p->ether_hdr.type)
    {
        rc = prvEtherHeaderParseForARP(p);
    }
#endif /*AIR_SUPPORT_MGMT_VLAN_CFG*/

#ifdef LLC_SUPPORT
    if (p->ether_hdr.type < netifMAX_MTU)
    {
        p->flags |= PBUF_FLAG_802_3;
        MEMCPY((void *) &(p->llc_hdr), (void *) p->payload, SIZEOF_LLC_HDR);
        p->payload += SIZEOF_LLC_HDR;
        p->len -= SIZEOF_LLC_HDR;
        p->tot_len -= SIZEOF_LLC_HDR;
    }
#endif

    return rc;
}

/**
 * Dump the stored infomation in pbuf structure
 *
 * @param p the point to the packet buffer
 */
static void prvEtherHeaderDump(struct pbuf *p)
{
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: ether DA:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F"\n",
              (unsigned char)p->ether_hdr.dest.addr[0], (unsigned char)p->ether_hdr.dest.addr[1], (unsigned char)p->ether_hdr.dest.addr[2],
              (unsigned char)p->ether_hdr.dest.addr[3], (unsigned char)p->ether_hdr.dest.addr[4], (unsigned char)p->ether_hdr.dest.addr[5] ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: ether SA:%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F":%"X8_F"\n",
              (unsigned char)p->ether_hdr.src.addr[0], (unsigned char)p->ether_hdr.src.addr[1], (unsigned char)p->ether_hdr.src.addr[2],
              (unsigned char)p->ether_hdr.src.addr[3], (unsigned char)p->ether_hdr.src.addr[4], (unsigned char)p->ether_hdr.src.addr[5] ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: ethertype:%x\n", p->ether_hdr.type ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: stag rsn:%d, vpm:%d, sp:%d\n", p->stag_hdr.rx_hdr.rsn, p->stag_hdr.rx_hdr.vpm, p->stag_hdr.rx_hdr.sp ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: stag pcp:%d, dei:%d, vid:%d\n", p->stag_hdr.rx_hdr.pcp, p->stag_hdr.rx_hdr.dei, p->stag_hdr.rx_hdr.vid ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: vlan numbrer:%d\n", p->vlan_num ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: vlan tpid:%x\n", p->vlan_hdr.tpid ) );
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("PBUF: vlan priority:%d, cfi:%d, vlanID:%d\n", p->vlan_hdr.priority, p->vlan_hdr.cfi, p->vlan_hdr.vid ) );
}

#ifdef AIR_SUPPORT_RSTP
/**
 * Copy the pbuf headers from the source pbuf to the destination pbuf.
 *
 * @param ptr_dstPbuf the pointer to the destination pbuf
 * @param ptr_srcPbuf the pointer to the source pbuf
 */
static void prvEthernetCopyPbufHeader(struct pbuf *ptr_dstPbuf, struct pbuf *ptr_srcPbuf)
{
#if STAG_INSERT_MODE
    MEMCPY((void *)&ptr_dstPbuf->ether_hdr, (void *)&ptr_srcPbuf->ether_hdr, sizeof(struct eth_hdr));
    MEMCPY((void *)&ptr_dstPbuf->vlan_hdr, (void *)&ptr_srcPbuf->vlan_hdr, sizeof(struct eth_vlan_hdr));
    ptr_dstPbuf->vlan_num = ptr_srcPbuf->vlan_num;
    MEMCPY((void *)&ptr_dstPbuf->stag_hdr.rx_hdr, (void *)&ptr_srcPbuf->stag_hdr.rx_hdr, sizeof(struct eth_rx_stag_hdr));
#ifdef LLC_SUPPORT
    MEMCPY((void *)&ptr_dstPbuf->llc_hdr, (void *)&ptr_srcPbuf->llc_hdr, sizeof(struct llc_hdr));
#endif
#endif
}
#endif

/**
 * Send the pbuf to the mapping protocol's queue handle
 *
 * @param p the point to the packet buffer
 * @param proto the mapping protocol number in enum
 */
static void prvEthernetSendQueue(struct pbuf *p, u8_t proto)
{
#ifdef AIR_SUPPORT_VOICE_VLAN
    u16_t default_vlan = 1;
#endif
#ifdef AIR_SUPPORT_SECOND_NETIF
    pxlwIPNetIf = netif_find_default();
#endif
    switch(proto)
    {
        /* IP or ARP packet? */
        case PROTO_IP:
        case PROTO_ARP:
#if LWIP_IPV6
        case PROTO_IPV6:
#endif
#ifdef AIR_SUPPORT_VOICE_VLAN
            if ((p->vlan_hdr.tpid == ETHTYPE_VLAN) && (p->vlan_hdr.vid != default_vlan))
            {
                /* support voice vlan auto join mode */
                struct pbuf * p_cpy = NULL;
                p_cpy = pbuf_alloc(PBUF_RAW, sizeof(struct pbuf), PBUF_RAM);
                if (p_cpy != NULL)
                {
                    MEMCPY((void *) &(p_cpy->ether_hdr.src), (void *) &(p->ether_hdr.src), ETH_HWADDR_LEN);
                    p_cpy->ether_hdr.type = p->ether_hdr.type;
                    p_cpy->vlan_hdr.tpid = p->vlan_hdr.tpid;
                    p_cpy->vlan_hdr.priority = p->vlan_hdr.priority;
                    p_cpy->vlan_hdr.vid = p->vlan_hdr.vid;
                    p_cpy->stag_hdr.rx_hdr.sp = p->stag_hdr.rx_hdr.sp;
                    p_cpy->stag_hdr.rx_hdr.vpm = p->stag_hdr.rx_hdr.vpm;
                    sendPbufToNetProtoQueue(p_cpy, PROTO_VOICE_VLAN);
                }
                else
                {
                    LWIP_DEBUGF(NETIF_DEBUG, ( "failed to allocate pbuf memory\n" ));
                }
            }
#endif
            /* full packet send to tcpip_thread to process */
            if( pxlwIPNetIf->input( p, pxlwIPNetIf ) != ERR_OK )
            {
                LWIP_DEBUGF(NETIF_DEBUG, ( "ethernetif_input: IP input error\n" ) );
                pbuf_free(p);
                p = NULL;
            }
            return;
#ifdef AIR_SUPPORT_RSTP
        case PROTO_BPDU:
        {
            struct pbuf *ptr_newPbuf = pbuf_clone(PBUF_RAW, PBUF_RAM, p);
            if (NULL != ptr_newPbuf)
            {
                prvEthernetCopyPbufHeader(ptr_newPbuf, p);
                pbuf_free(p);
                p = NULL;
                sendPbufToNetProtoQueue(ptr_newPbuf, proto);
            }
            else
            {
                LWIP_DEBUGF(NETIF_DEBUG, ( "Fail to allocate pbuf from PBUF_RAM for RSTP\n" ) );
                sendPbufToNetProtoQueue(p, proto);
            }
            break;
        }
#endif
        default:
            sendPbufToNetProtoQueue(p, proto);
            break;
    }
}

/**
 * Map the pbuf's ethernet header info to the protocol's rule
 *
 * @param p the point to the packet buffer
 */
static void prvEtherDispatch(struct pbuf *p)
{
    struct eth_addr const ether_da_1 = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};
    struct eth_addr const ether_da_2 = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
    struct eth_addr const ether_da_3 = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
    struct eth_addr const ether_da_4 = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};

    p = customer_ethernetif_dispatchPbuf(p);
    if (NULL == p)
    {
        /* pbuf is dispatched. */
        return;
    }

    /* this might be a multicast or broadcast packet */
    if(p->ether_hdr.dest.addr[0] & 1) {
        switch(p->ether_hdr.type) {
            case ETHTYPE_LP:
                return prvEthernetSendQueue(p, PROTO_LP);
            default:
                break;
        }
    }

    if(eth_addr_cmp(&p->ether_hdr.dest, &ether_da_1)) {
        switch(p->ether_hdr.type) {
            case ETHTYPE_LLDP:
                return prvEthernetSendQueue(p, PROTO_LLDP);
            default:
                break;
        }
    } else if(eth_addr_cmp(&p->ether_hdr.dest, &ether_da_2)) {
        switch(p->ether_hdr.type) {
            case ETHTYPE_LLDP:
                return prvEthernetSendQueue(p, PROTO_LLDP);
            default:
                break;
        }
    } else if(eth_addr_cmp(&p->ether_hdr.dest, &ether_da_3)) {
        switch(p->ether_hdr.type) {
            case ETHTYPE_LLDP:
                return prvEthernetSendQueue(p, PROTO_LLDP);
            default:
#ifdef LLC_SUPPORT
                if (STP_LLC_DSAP == p->llc_hdr.llc_dsap &&
                    STP_LLC_SSAP == p->llc_hdr.llc_ssap &&
                    STP_LLC_CTRL == p->llc_hdr.llc_ctrl)
                {
                    return prvEthernetSendQueue(p, PROTO_BPDU);
                }
#endif
                break;
        }
    }
    else if (eth_addr_cmp(&p->ether_hdr.dest, &ether_da_4))
    {
        switch (p->ether_hdr.type)
        {
            case ETHTYPE_LACP_SLOW:
                return prvEthernetSendQueue(p, PROTO_LACP_SLOW);
            default:
                break;
        }
    }

    switch(p->ether_hdr.type) {
        case ETHTYPE_IP:
            return prvEthernetSendQueue(p, PROTO_IP);
#if LWIP_IPV6
        case ETHTYPE_IPV6:
        {
            return prvEthernetSendQueue(p, PROTO_IPV6);
        }
#endif
        case ETHTYPE_ARP:
            return prvEthernetSendQueue(p, PROTO_ARP);
        default:
            break;
    }

    /* Cannot match any ether conditions */
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Cannot match any ether conditions\n"));
    pbuf_free(p);
    p = NULL;
}

static NET_FILTER_T *prvFoundRegisterHandle(u8_t protocol)
{
    int i = 0;

    for (; i < NET_PROTOCOL_MAX_NUM; i++)
    {
        if ((NULL != net_protos[i]) && (protocol == net_protos[i]->protocol))
        {
            return net_protos[i];
        }
    }

    return NULL;
}
/**
 * Register the NET_FILTER_T to the global hash table
 *
 * @param netf the point to the NET_FILTER_T
 */
static err_t prvRegisterHandle(NET_FILTER_T *netf)
{
    int i = 0;

    for (; i < NET_PROTOCOL_MAX_NUM; i++)
    {
        if (NULL == net_protos[i])
        {
            net_protos[i] = netf;
            return ERR_OK;
        }
    }

    return ERR_MEM;
}

/**
 * Deregister the NET_FILTER_T to the global hash table
 *
 * @param netf the point to the NET_FILTER_T
 */
static err_t prvDeregisterHandle(NET_FILTER_T *netf)
{
    int i = 0;

    for (; i < NET_PROTOCOL_MAX_NUM; i++)
    {
        if (netf == net_protos[i])
        {
            net_protos[i] = NULL;
            return ERR_OK;
        }
    }

    return ERR_VAL;
}

#endif

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param pxNetIf the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *       NULL on memory error
 */
static struct pbuf *prvLowLevelInput( const unsigned char * const pucInputData, long lDataLength )
{
    struct pbuf *p = NULL, *q;

    if( lDataLength > 0 )
    {
        #if ETH_PAD_SIZE
            len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
        #endif

        /* We allocate a pbuf chain of pbufs from the pool. */
        p = pbuf_alloc( PBUF_RAW, lDataLength, PBUF_POOL );

        if( p != NULL )
        {
            #if ETH_PAD_SIZE
                pbuf_header( p, -ETH_PAD_SIZE ); /* drop the padding word */
            #endif

            /* We iterate over the pbuf chain until we have read the entire
            * packet into the pbuf. */
            lDataLength = 0;
            for( q = p; q != NULL; q = q->next )
            {
                /* Read enough bytes to fill this pbuf in the chain. The
                * available data in the pbuf is given by the q->len
                * variable.
                * This does not necessarily have to be a memcpy, you can also preallocate
                * pbufs for a DMA-enabled MAC and after receiving truncate it to the
                * actually received size. In this case, ensure the usTotalLength member of the
                * pbuf is the sum of the chained pbuf len members.
                */
                memcpy( q->payload, &( pucInputData[ lDataLength ] ), q->len );
                lDataLength += q->len;
            }

            #if ETH_PAD_SIZE
                pbuf_header( p, ETH_PAD_SIZE ); /* reclaim the padding word */
            #endif

            LINK_STATS_INC( link.recv );
        }
    }

    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function prvLowLevelInput() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param pxNetIf the lwip network interface structure for this ethernetif
 */
static void prvEthernetInput( const unsigned char * const pucInputData, long lInputLength )
{
    struct pbuf *p;
    AIR_ERROR_NO_T rc;
#ifdef AIR_SUPPORT_SECOND_NETIF
    pxlwIPNetIf = netif_find_default();
#endif

    /* move received packet into a new pbuf */
    p = prvLowLevelInput( pucInputData, lInputLength );

    /* no packet could be read, silently ignore this */
    if( p != NULL )
    {
#if STAG_INSERT_MODE
        rc = prvEtherHeaderParse(p);
        prvEtherHeaderDump(p);

        if (AIR_E_OK != rc)
        {
            LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Unexpected ARP frame, dropped\n"));
            pbuf_free(p);
            p = NULL;
            return;
        }

        // according register info send to module
        prvEtherDispatch(p);
#else
        struct eth_hdr *pxHeader;
        /* points to packet payload, which starts with an Ethernet header */
        pxHeader = p->payload;

        switch( htons( pxHeader->type ) )
        {
            /* IP or ARP packet? */
            case ETHTYPE_IP:
            case ETHTYPE_ARP:
                /* full packet send to tcpip_thread to process */
                if( pxlwIPNetIf->input( p, pxlwIPNetIf ) != ERR_OK )
                {
                    LWIP_DEBUGF(NETIF_DEBUG, ( "ethernetif_input: IP input error\n" ) );
                    pbuf_free(p);
                    p = NULL;
                }
                break;

            default:
                pbuf_free( p );
                p = NULL;
            break;
        }
#endif /* STAG_INSERT_MODE */
    }
}

static void prvInterruptSimulator( void *pvParameters )
{
    unsigned int index = 0;
    unsigned char* ptr = NULL;
    unsigned int len = 0;
#if STAG_INSERT_MODE
    err_t ret = ERR_OK;
    NET_FILTER_T *netf = NULL;
    QueueSetMemberHandle_t xActivatedMember;
    u8_t pkt_process_cnt = 0;
#endif

    /* Just to kill the compiler warning. */
    (void)pvParameters;

    mac_rcv_is_ready = TRUE;

    while(1)
    {
#if STAG_INSERT_MODE
        xActivatedMember = xQueueSelectFromSet(g_pkt_queue_set, 200 / portTICK_PERIOD_MS);
        if(xActivatedMember == g_pkt_queue)
        {
            if (pdTRUE == xQueueReceive(g_pkt_queue, &index, 0))
            {
                ptr = (unsigned char*)pdma_get_ethernet_pkt_buf(index, &len);
                if (ptr)
                {
                    prvEthernetInput(ptr, len);
                    pdma_release_ethernet_pkt_buf(index);
                }

                if (MAC_RCV_RX_PKT_WDOG_KICK_NUM == ++pkt_process_cnt)
                {
                    air_wdog_kick();
                    pkt_process_cnt = 0;
                }
            }
        }
        else if(xActivatedMember == g_pkt_reg_queue)
        {
            xQueueReceive(g_pkt_reg_queue, &netf, 0);

            if(NET_FILTER_REGISTER == netf->state) {
                ret = prvRegisterHandle(netf);
            } else if(NET_FILTER_DEREGISTER == netf->state) {
                ret = prvDeregisterHandle(netf);
            }
            if(ERR_OK != ret) {
                LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Fail to register module handle\n"));
            }
        }
#else
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        if (pdTRUE == xQueueReceive(g_pkt_queue, &index, portMAX_DELAY))
        {
            ptr = (unsigned char*)pdma_get_ethernet_pkt_buf(index, &len);
            if (ptr)
            {
                prvEthernetInput(ptr, len);
                pdma_release_ethernet_pkt_buf(index);
            }
        }
#endif
    }
}


/* FUNCTION NAME: ethernetif_regHandleFind
 * PURPOSE:
 *      Find mac_rcv message queue for module register
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer of the queue handle
 *
 * RETURN:
 *      ERR_OK
 *      ERR_ARG
 *      ERR_VAL
 *
 * NOTES:
 *      None
 */
err_t
ethernetif_regHandleFind(
    QueueHandle_t *pptr_hanlde)
{
#if STAG_INSERT_MODE
    if(NULL == pptr_hanlde) {
        return ERR_ARG;
    }

    if(NULL != g_pkt_reg_queue) {
        *pptr_hanlde = g_pkt_reg_queue;
    } else {
        return ERR_VAL;
    }

    return ERR_OK;
#else
    return ERR_VAL;
#endif
}

/* FUNCTION NAME: ethernetif_sendPbufToRegisterQueue
 * PURPOSE:
 *      Send pbuf to the queue registered by the NET_FILTER_REGISTER message.
 *
 * INPUT:
 *      ptr_buf              -- the pbuf to be sent
 *      protocol             -- the protocol registered used to find the queue
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
ethernetif_sendPbufToRegisterQueue(
    struct pbuf *p,
    u8_t protocol)
{
#if STAG_INSERT_MODE
    NET_FILTER_T *ptr_net_proto = NULL;
    NET_MSG_T *ptr_msg = NULL;

    ptr_net_proto = prvFoundRegisterHandle(protocol);
    if ((NULL != ptr_net_proto) && (NULL != ptr_net_proto->handle))
    {
        LWIP_DEBUGF(PBUF_PARSE_DEBUG, ( "Send to protocol %d, handle %p \n", protocol, ptr_net_proto->handle) );

        ptr_msg = pvPortMalloc(sizeof(NET_MSG_T), "pbufQ");
        if (NULL != ptr_msg)
        {
            ptr_msg->msg_id = ETHERNET_MSG_ID_ETHERNET_PBUF;
            ptr_msg->ptr_pbuf = p;
            if (pdPASS == (xQueueSend(ptr_net_proto->handle, &ptr_msg, 5UL)))
            {
                return;
            }
        }
    }
#endif

    /* Cannot found any handle */
    LWIP_DEBUGF(PBUF_PARSE_DEBUG, ("Cannot found any module handle or msg send failed\n"));
    pbuf_free(p);
}

/* FUNCTION NAME: mac_rcv_readyGet
 * PURPOSE:
 *      Check if  mac_rcv is ready or not
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      0 or -1
 *
 * NOTES:
 *      None
 */
int mac_rcv_readyGet(void)
{
    if(FALSE == mac_rcv_is_ready)
    {
        return -1;
    }

    return 0;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function prvLowLevelInit() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param pxNetIf the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *       ERR_MEM if private data couldn't be allocated
 *       any other err_t on error
 */
err_t ethernetif_init(struct netif *pxNetIf)
{
    struct xEthernetIf *pxEthernetIf;

    LWIP_ASSERT("pxNetIf != NULL", (pxNetIf != NULL));

    pxEthernetIf = mem_malloc(sizeof(struct xEthernetIf));
    if(!pxEthernetIf)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

#if LWIP_NETIF_HOSTNAME
    {
        /* Initialize interface hostname */
        pxNetIf->hostname = "netIf";
    }
#endif /* LWIP_NETIF_HOSTNAME */

    pxNetIf->state = pxEthernetIf;
    pxNetIf->name[ 0 ] = IFNAME0;
    pxNetIf->name[ 1 ] = IFNAME1;

    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    pxNetIf->output = etharp_output;
#if LWIP_IPV6
    pxNetIf->output_ip6 = ethip6_output;
#endif
    pxNetIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    pxNetIf->hwaddr_len = ETHARP_HWADDR_LEN;
    pxNetIf->mtu = netifMAX_MTU;
    pxNetIf->linkoutput = prvLowLevelOutput;

    pxEthernetIf->ethaddr = (struct eth_addr *) &(pxNetIf->hwaddr[0]);

    /* initialize the hardware */
    prvLowLevelInit(pxNetIf);

    return ERR_OK;
}

/**
 * This function is used to do MAC_RCV task init, including queue create,
 * mutex create and task create.
 *
 * @return ERR_OK if initialization finished
 *       ERR_MEM if private data couldn't be allocated
 *       any other err_t on error
 */
err_t mac_rcv_init(void)
{
#if STAG_INSERT_MODE
    g_pkt_queue_set = xQueueCreateSet(NET_PDMA_MSG_QUEUE_SIZE + NET_PROTOCOL_REGISTER_PBUF_QUEUE_SIZE, "mac_rcv_set");
    if (!g_pkt_queue_set)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("mac_rcv_init: pkt queue set creation failed\n"));
        return ERR_MEM;
    }
#endif /* STAG_INSERT_MODE */

    g_pkt_queue = xQueueCreate(NET_PDMA_MSG_QUEUE_SIZE, sizeof(uint32_t), "mac_rcv");
    if (!g_pkt_queue)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("mac_rcv_init: pkt queue creation failed\n"));
        return ERR_MEM;
    }

    g_pkt_tx_mutex = xSemaphoreCreateMutex("mac_rcv");
    if (!g_pkt_tx_mutex)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("mac_rcv_init: tx pkt mutex creation failed\n"));
        return ERR_MEM;
    }

#if STAG_INSERT_MODE
    g_pkt_reg_queue = xQueueCreate(NET_PROTOCOL_REGISTER_PBUF_QUEUE_SIZE, sizeof(uint32_t), MAC_PKT_REGISTER);
    if (!g_pkt_reg_queue)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("mac_rcv_init: pkt register queue creation failed\n"));
        return ERR_MEM;
    }

    xQueueAddToSet(g_pkt_queue, g_pkt_queue_set);
    xQueueAddToSet(g_pkt_reg_queue, g_pkt_queue_set);
#endif /* STAG_INSERT_MODE */

    xTaskCreate( prvInterruptSimulator, "MAC_RCV", configMACRCV_STACK_SIZE, NULL, configMAC_ISR_SIMULATOR_PRIORITY, NULL );

    return ERR_OK;
}

#ifdef AIR_SUPPORT_ERPS
/* FUNCTION NAME:   erpsLowLevelOutput
 * PURPOSE:
 *      This function is used to send ERPS packets through PDMA interface.
 *
 * INPUT:
 *      ptr_pxNetIf          -- the lwip network interface structure for this ethernetif
 *      ptr_buf              -- the buffer containing the packet to be sent
 *      bufLen               -- the length of the packet to send
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ERR_OK
 *      ERR_BUF
 *
 * NOTES:
 *      None
 */
err_t
erpsLowLevelOutput(
    struct netif *ptr_pxNetIf,
    u8_t *ptr_buf,
    u16_t bufLen)
{
    err_t err = ERR_OK;

    xSemaphoreTake(g_pkt_tx_mutex, portMAX_DELAY);
    if(pdma_tx_pkt(ptr_buf, bufLen, 0, 0) < 0)
    {
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        snmp_inc_ifoutdiscards(ptr_pxNetIf);
        err = ERR_BUF;
    }
    else
    {
        LINK_STATS_INC( link.xmit );
        snmp_add_ifoutoctets(ptr_pxNetIf, bufLen);
        if((ptr_buf[0] & 1) != 0 )
        {
            /* broadcast or multicast packet */
            snmp_inc_ifoutnucastpkts(ptr_pxNetIf);
        }
        else
        {
            /* unicast packet */
            snmp_inc_ifoutucastpkts(ptr_pxNetIf);
        }
    }
    xSemaphoreGive(g_pkt_tx_mutex);
    return err;
}
#endif /* AIR_SUPPORT_ERPS */
