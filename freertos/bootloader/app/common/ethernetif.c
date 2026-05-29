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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timer.h"
#include "pbuf.h"
#include "ethernetif.h"


#define BIT(nr)                           (1UL << (nr))
#define BITS(m, n)                        (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#define BITS_OFF_R(val, offset, range)    (((val) >> offset) & (BITS(0, (range) - 1)))

#define MAC_RCV_RX_PKT_WDOG_KICK_NUM      (80)

#define PKT_FIRST_VLAN_TAG                (0)
#define PKT_MAX_VLAN_TAG                  (3)

#define GET_32(__ptr_data__, __rval__) do           \
    {                                               \
        unsigned char *ptr_data = (__ptr_data__);   \
        (__rval__) = *ptr_data++;                   \
        (__rval__) <<= 8;                           \
        (__rval__) |= *ptr_data++;                  \
        (__rval__) <<= 8;                           \
        (__rval__) |= *ptr_data++;                  \
        (__rval__) <<= 8;                           \
        (__rval__) |= *ptr_data++;                  \
    } while(0)

#define GET_16(__ptr_data__, __rval__) do           \
    {                                               \
        unsigned char *ptr_data = (__ptr_data__);   \
        (__rval__) = *ptr_data++;                   \
        (__rval__) <<= 8;                           \
        (__rval__) |= *ptr_data++;                  \
    } while(0)

extern void* pdma_get_ethernet_pkt_buf(unsigned int index, unsigned int* ptr_len);
extern void pdma_release_ethernet_pkt_buf(unsigned int index);
extern int pdma_tx_pkt(unsigned char* data, int len, int txq, int cp_to_sec_desc);
extern void fpga_init();

/* queue recv pkt from underlayer */
QueueHandle_t g_pkt_queue = NULL;
SemaphoreHandle_t g_pkt_tx_mutex;
QueueSetHandle_t g_pkt_queue_set;
QueueHandle_t g_pkt_reg_queue = NULL;

/* middleware register info */
static NET_FILTER_T *net_protos[PROTO_MAX] = {0};
static int mac_rcv_is_ready = false;

static int prvLowLevelOutput(struct pbuf *p)
{
    static unsigned char ucBuffer[ 1520 ];
    unsigned char *pucBuffer = ucBuffer;
    int res = 0;

    xSemaphoreTake(g_pkt_tx_mutex, portMAX_DELAY);

    /* No pbuf chain, don't have to copy -> faster. */
    pucBuffer = &((unsigned char *) p->payload)[0];

    /* signal that packet should be sent */
    if(pdma_tx_pkt(pucBuffer, p->len, 0, 0) < 0)
    {
        res = -1;
    }

    xSemaphoreGive(g_pkt_tx_mutex);
    return res;
}

static struct pbuf *prvLowLevelInput(const unsigned char *const pucInputData, long lDataLength)
{
    struct pbuf *p = NULL;

    if(lDataLength > 0)
    {
        /* allocate a pbuf chain of pbufs from the pool. */
        /* Note: We suppose packet length not over, so here not iterate over the pbuf chain */
        p = pbuf_alloc(lDataLength);

        if(p != NULL)
        {
            lDataLength = 0;

            memcpy(p->payload, &(pucInputData[lDataLength]), p->len);
            // unsigned char *c = p;
        }
    }

    return p;
}

static void prvEtherHeaderParse(struct pbuf *p)
{
    unsigned char i = 0;
    unsigned short u16dat = 0;
    unsigned long u32dat = 0;

    /* Ethernet DA */
    memcpy((void *) &(p->ether_hdr.dest), (void *) p->payload, ETH_HWADDR_LEN);
    p->payload += ETH_HWADDR_LEN;
    p->len -= ETH_HWADDR_LEN;

    /* Ethernet SA */
    memcpy((void *) &(p->ether_hdr.src), (void *) p->payload, ETH_HWADDR_LEN);
    p->payload += ETH_HWADDR_LEN;
    p->len -= ETH_HWADDR_LEN;

    /* Special Tag */
    GET_32(p->payload, u32dat);
    u32dat = htonl(u32dat);

    p->payload += SIZEOF_STAG_HDR;
    p->len -= SIZEOF_STAG_HDR;

    p->stag_hdr.rx_hdr.rsn = BITS_OFF_R(ntohl(u32dat), 26, 3);
    p->stag_hdr.rx_hdr.vpm = BITS_OFF_R(ntohl(u32dat), 24, 2);
    p->stag_hdr.rx_hdr.sp  = BITS_OFF_R(ntohl(u32dat), 16, 5);
    p->stag_hdr.rx_hdr.pcp = BITS_OFF_R(ntohl(u32dat), 13, 3);
    p->stag_hdr.rx_hdr.dei = BITS_OFF_R(ntohl(u32dat), 12, 1);
    p->stag_hdr.rx_hdr.vid = BITS_OFF_R(ntohl(u32dat), 0, 12);

    /* VLAN header */
    for(i = PKT_FIRST_VLAN_TAG; i < PKT_MAX_VLAN_TAG; i++)
    {
        GET_32(p->payload, u32dat);

        if((0x8100 == BITS_OFF_R(u32dat, 16, 16)) || (0x88A8 == BITS_OFF_R(u32dat, 16, 16)) || (0x9100 == BITS_OFF_R(u32dat, 16, 16)))
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
}

// #define DUMP_ETHER_HEADER
#ifdef DUMP_ETHER_HEADER
static void prvEtherHeaderDump(struct pbuf *p)
{
    DPRINTF("PBUF: ether DA:%02x:%02x:%02x:%02x:%02x:%02x \n",
           (unsigned char)p->ether_hdr.dest.addr[0], (unsigned char)p->ether_hdr.dest.addr[1], (unsigned char)p->ether_hdr.dest.addr[2],
           (unsigned char)p->ether_hdr.dest.addr[3], (unsigned char)p->ether_hdr.dest.addr[4], (unsigned char)p->ether_hdr.dest.addr[5]);
    DPRINTF("PBUF: ether SA:%02x:%02x:%02x:%02x:%02x:%02x \n",
           (unsigned char)p->ether_hdr.src.addr[0], (unsigned char)p->ether_hdr.src.addr[1], (unsigned char)p->ether_hdr.src.addr[2],
           (unsigned char)p->ether_hdr.src.addr[3], (unsigned char)p->ether_hdr.src.addr[4], (unsigned char)p->ether_hdr.src.addr[5]);
    DPRINTF("PBUF: ethertype:%x \n", p->ether_hdr.type);
    DPRINTF("PBUF: stag rsn:%d, vpm:%d, sp:%d \n", p->stag_hdr.rx_hdr.rsn, p->stag_hdr.rx_hdr.vpm, p->stag_hdr.rx_hdr.sp);
    DPRINTF("PBUF: stag pcp:%d, dei:%d, vid:%d \n", p->stag_hdr.rx_hdr.pcp, p->stag_hdr.rx_hdr.dei, p->stag_hdr.rx_hdr.vid);
    DPRINTF("PBUF: vlan numbrer:%d \n", p->vlan_num);
    DPRINTF("PBUF: vlan tpid:%x \n", p->vlan_hdr.tpid);
    DPRINTF("PBUF: vlan priority:%d, cfi:%d, vlanID:%d \n", p->vlan_hdr.priority, p->vlan_hdr.cfi, p->vlan_hdr.vid);
}
#endif

#ifdef AIR_SUPPORT_ACE
static void prvEthernetSendQueue(struct pbuf *p, unsigned char proto)
{
    int hash;
    hash = (proto) & (PROTO_MAX - 1);

    if((net_protos[hash]->protocol == proto) && (net_protos[hash]->handle != NULL))
    {
        if( pdPASS == (xQueueSend(net_protos[hash]->handle, &p, 0UL)))
        {
            return;
        }
    }

    /* Cannot found any handle */
    pbuf_free(p);
    p = NULL;
}
#endif

static void prvEtherDispatch(struct pbuf *p)
{
#ifdef AIR_SUPPORT_ACE
    if (ETHTYPE_ACE == p->ether_hdr.type)
    {
        DPRINTF("got ETHETYPE_ACE.\n");
        return prvEthernetSendQueue(p, PROTO_ACE);
    }
#endif

    /* Cannot match any ether conditions */
    pbuf_free(p);
    p = NULL;
}

/**
 * Register the NET_FILTER_T to the global hash table
 *
 * @param netf the point to the NET_FILTER_T
 */
static int prvRegisterHandle(NET_FILTER_T *netf)
{
    int ret = 0;
    int hash;

    hash = (netf->protocol) & (PROTO_MAX - 1);

    if(net_protos[hash]) {
        ret = -1;
    } else {
        net_protos[hash] = netf;
    }

    return ret;
}

/**
 * Deregister the NET_FILTER_T to the global hash table
 *
 * @param netf the point to the NET_FILTER_T
 */
static int prvDeregisterHandle(NET_FILTER_T *netf)
{
    int ret = 0;
    int hash;

    hash = (netf->protocol) & (PROTO_MAX - 1);

    if(!net_protos[hash]) {
        ret = -1;
    } else {
        net_protos[hash] = NULL;
    }

    return ret;
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

    /* move received packet into a new pbuf */
    p = prvLowLevelInput( pucInputData, lInputLength);

    if( p != NULL )
    {
        prvEtherHeaderParse(p);
#ifdef DUMP_ETHER_HEADER
        prvEtherHeaderDump(p);
#endif

        // according register info send to module
        prvEtherDispatch(p);
    }
}

static void prvInterruptSimulator( void *pvParameters )
{
    unsigned int index = 0;
    unsigned char* ptr = NULL;
    unsigned int len = 0;

    NET_FILTER_T *netf = NULL;
    QueueSetMemberHandle_t xActivatedMember;
    unsigned char pkt_process_cnt = 0;

    /* Just to kill the compiler warning. */
    (void)pvParameters;

    mac_rcv_is_ready = true;

    while(1)
    {
        xActivatedMember = xQueueSelectFromSet(g_pkt_queue_set, 200 / portTICK_PERIOD_MS);

        // if(xActivatedMember != 0)
        // {
        //     DPRINTF("%s[%d] get event, xActivatedMember=%p \n", __func__, __LINE__, xActivatedMember);
        // }

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
                prvRegisterHandle(netf);
            } else if(NET_FILTER_DEREGISTER == netf->state) {
                prvDeregisterHandle(netf);
            }
        }
    }
}

int mac_rcv_init(TaskHandle_t *pxHandle)
{
    g_pkt_queue_set = xQueueCreateSet(24 + 24);
    if (!g_pkt_queue_set)
    {
        return -1;
    }

    g_pkt_queue = xQueueCreate(24, sizeof(unsigned long));
    if (!g_pkt_queue)
    {
        return -1;
    }

    g_pkt_tx_mutex = xSemaphoreCreateMutex();
    if (!g_pkt_tx_mutex)
    {
        return -1;
    }

    g_pkt_reg_queue = xQueueCreate(24, sizeof(unsigned long));
    if (!g_pkt_reg_queue)
    {
        return -1;
    }

    xQueueAddToSet(g_pkt_queue, g_pkt_queue_set);
    xQueueAddToSet(g_pkt_reg_queue, g_pkt_queue_set);

    xTaskCreate( prvInterruptSimulator, "MAC_RCV", 1024, NULL, configMAC_ISR_SIMULATOR_PRIORITY, pxHandle);

    return 0;
}

int ethernetif_regHandleFind(QueueHandle_t *pptr_hanlde)
{
    if(NULL == pptr_hanlde) {
        return -1;
    }

    if(NULL != g_pkt_reg_queue) {
        *pptr_hanlde = g_pkt_reg_queue;
    } else {
        return -1;
    }

    return 0;
}

int mac_rcv_readyGet(void)
{
    if(false == mac_rcv_is_ready)
    {
        return -1;
    }

    return 0;
}

int ethernet_output(
    struct pbuf * p,
    const struct eth_addr * src,
    const struct eth_addr * dst,
    unsigned short eth_type)
{
    struct eth_hdr *ethhdr;
    unsigned short eth_type_be = htons(eth_type);
    unsigned short eth_type_offset = ETH_HWADDR_LEN + ETH_HWADDR_LEN;
    unsigned long u32dat = 0;

    if (pbuf_add_header(p, (SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + (p->vlan_num * SIZEOF_VLAN_HDR))) != 0)
    {
        return -1;
    }

    /* vlan hdr */
    if (p->vlan_num > 0)
    {
        u32dat |= (p->vlan_hdr.tpid << 16); // TPID
        u32dat |= (p->vlan_hdr.priority << 13); // User priority
        u32dat |= (p->vlan_hdr.cfi << 12); // CFI
        u32dat |= (p->vlan_hdr.vid << 0); // VLAN ID

        u32dat = htonl(u32dat);
        memcpy((void *)(p->payload + 12 + SIZEOF_STAG_HDR), &u32dat, SIZEOF_VLAN_HDR);
        eth_type_offset += (p->vlan_num * SIZEOF_VLAN_HDR);
    }

    /* stag */
    u32dat = 0;
    if (STAG_HDR_MODE_INSERT == p->stag_hdr.tx_hdr.mode) // insert mode
    {
        u32dat |= (p->stag_hdr.tx_hdr.tx_stag_insert.opc << 29); // OPC
        u32dat |= (p->stag_hdr.tx_hdr.tx_stag_insert.dp << 0); // DP
    }
    else
    {
        u32dat |= (1 << 29); // OPC: not specify egress port
    }

    u32dat = htonl(u32dat);

    memcpy((void *)(p->payload + 12), &u32dat, SIZEOF_STAG_HDR);
    eth_type_offset += SIZEOF_STAG_HDR;

    /* ethernet header */
    ethhdr = (struct eth_hdr *)p->payload;
    memcpy(&ethhdr->dest, dst, ETH_HWADDR_LEN);
    memcpy(&ethhdr->src,  src, ETH_HWADDR_LEN);
    memcpy((void *) p->payload + eth_type_offset, &eth_type_be, SIZEOF_ETH_TYPE);

    /* send the packet */
    return prvLowLevelOutput(p);
}

