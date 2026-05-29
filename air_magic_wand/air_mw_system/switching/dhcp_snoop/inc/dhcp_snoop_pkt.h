/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:  dhcp_snoop_pkt.h
 * PURPOSE:
 * It provides DHCP Snooping module API and definitions.
 *
 * NOTES:
 */

#ifndef __DHCP_SNOOP_PKT_H__
#define __DHCP_SNOOP_PKT_H__
/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>

#include <lwip/prot/iana.h>
#include <lwip/prot/dhcp.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define IP_HDRLEN                       (20)
/* DHCP_SNP_PKT_BUFF_SIZE = 1500 - 20 - 8 = 1472 */
#define DHCP_SNP_PKT_BUFF_SIZE          (1500 - IP_HDRLEN - UDP_HLEN)

#define DHCP_SNP_UDP_PORT_DHCP_CLIENT   LWIP_IANA_PORT_DHCP_CLIENT
#define DHCP_SNP_UDP_PORT_DHCP_SERVER   LWIP_IANA_PORT_DHCP_SERVER
/* RFC3046: DHCP Relay Agent Information Ooption */
#define DHCP_OPTION_RELAY_AGENT         82
#define DHCP_SUBOPTION_CIRCUIT_ID       1
#define DHCP_SUBOPTION_REMOTE_ID        2

/* Packet actions */
#define DHCP_SNP_PKT_DROP               0
#define DHCP_SNP_PKT_FORWARD            BIT(0)
#define DHCP_SNP_PKT_OPTION82_ADD       BIT(1)
#define DHCP_SNP_PKT_OPTION82_REMOVE    BIT(2)
#define DHCP_SNP_PKT_OPTION82_REPLACE   BIT(3)
#define DHCP_SNP_PKT_OPTION82_APPEND    BIT(4)
#ifdef DHCP_SNP_BINDING_DB
#define DHCP_SNP_BINDING_ADD_ENTRY      BIT(5)
#define DHCP_SNP_BINDING_UPDATE_ENTRY   BIT(6)
#define DHCP_SNP_BINDING_DEL_ENTRY      BIT(7)
#endif /* DHCP_SNP_BINDING_DB */

/* Option field */
#define DHCP_OPTION_OP                  0
#define DHCP_OPTION_LEN                 1
#define DHCP_OPTION_DATA                2

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct DHCP_SNP_PKT_INFO_S
{
    /* Process action: DHCP_SNP_PKT_XXX/DHCP_SNP_BINDING_XXX */
    UI16_T              action;
    /* DHCP request flag, 1: request, 0: response */
    UI8_T               dhcp_request;
    /* DHCP message type */
    UI8_T               msg_type;

    /* Packet buffer from lwip */
    struct pbuf         *ptr_pbuf;
    ip4_addr_t          src_ip;
    ip4_addr_t          dest_ip;
    UI16_T              ethdr_offset;
    UI16_T              port_id;
    UI16_T              vlan_id;
    UI16_T              trunk_id;

    /* DHCP header */
    struct dhcp_msg     *ptr_dhcphdr;
    /* DHCP relay agent option */
    UI8_T               *ptr_opt82;
    UI8_T               *ptr_subopt_circuit_id;
    UI8_T               *ptr_subopt_remote_id;
    /* DHCP end option */
    UI8_T               *ptr_opt_end;
} DHCP_SNP_PKT_INFO_T;


typedef struct DHCP_SNP_PKT_MSG_S
{
    UI8_T               msg_id;
    struct pbuf         *ptr_pbuf;
    ip4_addr_t          src_ip;
    ip4_addr_t          dest_ip;
    UI8_T               dhcp_request;
    UI16_T              ethdr_offset;
} DHCP_SNP_PKT_MSG_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_pkt_process
 * PURPOSE:
 *      DHCP snooping packet process function
 *
 * INPUT:
 *      ptr_msg   -  DHCP snooping packet message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_pkt_process(
    DHCP_SNP_PKT_MSG_T *ptr_msg);

/* FUNCTION NAME: dhcp_snp_pkt_deinit
 * PURPOSE:
 *      Deinit DHCP snooping packet function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_pkt_deinit(
    void);

/* FUNCTION NAME: dhcp_snp_pkt_init
 * PURPOSE:
 *      Init DHCP snooping packet function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_pkt_init(
    void);

#endif /* End of __DHCP_SNOOP_PKT_H__ */
