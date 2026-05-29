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

/* FILE NAME:  ethernetif.h
 * PURPOSE:
 *      Data structure and APIs defines for freeRTOS mac_rcv.
 *
 * NOTES:
 */

#ifndef LWIP_HDR_ETHERNETIF_H
#define LWIP_HDR_ETHERNETIF_H

/* INCLUDE FILE DECLARATIONS
 */
#include "FreeRTOS.h"
#include "queue.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/prot/ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define MAC_PKT_REGISTER "pkt_reg"
#define STP_LLC_DSAP     (0x42)
#define STP_LLC_SSAP     (0x42)
#define STP_LLC_CTRL     (0x3)

#define PKT_FIRST_VLAN_TAG                (0)
#define PKT_MAX_VLAN_TAG                  (3)

#ifdef AIR_LITE_MW
#define NET_PROTOCOL_MAX_NUM    (5)
#else
#define NET_PROTOCOL_MAX_NUM    (10)
#endif

#ifdef AIR_MW_SUPPORT
#define ETHERNET_MSG_ID_ETHERNET_PBUF    (MW_MSG_ID_ETHERNET_PBUF)
#else
#define ETHERNET_MSG_ID_ETHERNET_PBUF    (0)
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define MSG_MAX_NAME_LEN        (10)

/* DATA TYPE DECLARATIONS
 */
typedef struct NET_FILTER_S
{
    u8_t name[MSG_MAX_NAME_LEN];
    u8_t protocol;
    QueueHandle_t handle;
    u8_t state;
} NET_FILTER_T;

typedef enum {
    NET_FILTER_DEREGISTER = 0,
    NET_FILTER_REGISTER,
    NET_FILTER_MAX
} NET_FILTER_STATE_T;

typedef enum {
    PROTO_LLDP = 1,
    PROTO_IGMP_SNP_V4 = 2,
    PROTO_IGMP_SNP_V6 = 3,
    PROTO_IP = 4,
    PROTO_ARP = 5,
    PROTO_LP = 6,
    PROTO_VOICE_VLAN = 7,
    PROTO_IPV6 = 8,
    PROTO_BPDU = 9,
    PROTO_LACP_SLOW = 10,
    PROTO_MAX
} MW_PROTO_T;

typedef struct
{
    u8_t msg_id;
    void *ptr_pbuf;
} __attribute__((packed)) NET_MSG_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


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
    QueueHandle_t *pptr_hanlde);

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
    u8_t protocol);

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
int mac_rcv_readyGet(void);

/**
 * Send the pbuf to the mapping protocol's queue handle
 *
 * @param p the point to the packet buffer
 * @param proto the mapping protocol number in enum
 */
void sendPbufToNetProtoQueue(struct pbuf *p, u8_t proto);

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
    u16_t bufLen);

#endif /* AIR_SUPPORT_ERPS */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_ETHERNETIF_H */

