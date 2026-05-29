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

#ifndef ETHERNETIF_H
#define ETHERNETIF_H

/* INCLUDE FILE DECLARATIONS
 */
#include "queue.h"
#include "pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define MSG_MAX_NAME_LEN        (10)

/* DATA TYPE DECLARATIONS
 */
typedef struct NET_FILTER_S
{
    unsigned char name[MSG_MAX_NAME_LEN];
    unsigned char protocol;
    QueueHandle_t handle;
    unsigned char state;
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
#ifdef AIR_SUPPORT_LP
    PROTO_LP = 6,
#endif
#ifdef AIR_SUPPORT_ACE
    PROTO_ACE = 7,
#endif
    PROTO_MAX
} MW_PROTO_T;

typedef enum {
    ETHTYPE_IP   = 0x0800,
    ETHTYPE_IPV6 = 0x86DD,
#ifdef AIR_SUPPORT_LP
    ETHTYPE_LP   = 0x8070,
#endif
#ifdef AIR_SUPPORT_ACE
    ETHTYPE_ACE   = 0x88B5,
#endif
    ETHTYPE_MAX
} MW_ETHTYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
int ethernetif_regHandleFind(
    QueueHandle_t *pptr_hanlde);

int mac_rcv_readyGet(void);

int ethernet_output(
    struct pbuf * p,
    const struct eth_addr * src,
    const struct eth_addr * dst,
    unsigned short eth_type);


#ifdef __cplusplus
}
#endif

#endif /* ETHERNETIF_H */

