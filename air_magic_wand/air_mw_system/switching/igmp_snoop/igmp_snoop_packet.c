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

/* FILE NAME:  igmp_snoop_packet.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop packet processing.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_packet.h"
#include "igmp_snoop.h"
#include "igmp_snoop_utils.h"
#include "igmp_snoop_log.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_lag.h"
#endif
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_port.h"
#include "igmp_snoop_timer.h"
#include "netif/ethernet.h"
#include "lwip/ip4_addr.h"
#include <osal/osal.h>
#include "lwip/inet_chksum.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/igmp.h"
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier_utils.h"
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#ifdef AIR_SUPPORT_LP
#include "lp.h"
#endif

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
static MW_ERROR_NO_T
_igmp_snp_packet_resumePbufHeader(
    struct pbuf *ptr_pbuf,
    UI16_T header_size_increment);

static MW_ERROR_NO_T
_igmp_snp_packet_send(
    struct pbuf *ptr_pbuf,
    UI8_T igmp_type,
    UI32_T forward_bmp);

static MW_ERROR_NO_T
_igmp_snp_packet_getIngressVlanInfo(
    struct pbuf     *ptr_pbuf,
    UI16_T          *ptr_vlan_id,
    UI8_T           *ptr_vlan_count);

#ifdef AIR_SUPPORT_IGMPV3_AWARE
static MW_ERROR_NO_T
_igmp_snp_packet_verifyV3ReportMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info);

static MW_ERROR_NO_T
_igmp_snp_packet_handleV3ReportMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info);
#endif

static MW_ERROR_NO_T
_igmp_snp_packet_handleJoinMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info);

static MW_ERROR_NO_T
_igmp_snp_packet_handleLeaveMSg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info);

static MW_ERROR_NO_T
_igmp_snp_packet_handleQueryMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info);

static MW_ERROR_NO_T
_igmp_snp_packet_parsePacket(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info,
    UI8_T igmpv3_query);

MW_ERROR_NO_T
igmp_snp_packet_reportSuppProcess(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr,
    UI8_T port_id,
    UI8_T time);

/* STATIC VARIABLE DECLARATIONS
 */
static ip4_addr_t           _igmp_snp_allsystems;
static ip4_addr_t           _igmp_snp_allrouters;
#ifdef AIR_SUPPORT_IGMPV3_AWARE
static const C8_T*          _igmpv3_record_type_vec[] =
{
    "INVALID",
    "IS_INCLUDE",
    "IS_EXCLUDE",
    "CHANGE_TO_INCLUDE",
    "CHANGE_TO_EXCLUDE",
    "ALLOW_NEW_SOURCE",
    "BLOCK_OLD_SOURCE",
};
#endif /* AIR_SUPPORT_IGMPV3_AWARE */
static UI8_T                _igmp_snp_process_cnt = 0;

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_igmp_snp_packet_resumePbufHeader(
    struct pbuf *ptr_pbuf,
    UI16_T header_size_increment)
{
    if (ptr_pbuf == NULL)
    {
        IGMP_SNP_LOG_ERROR("p is NULL, or header_size_increment(%d) is invalid", header_size_increment);
        return MW_E_BAD_PARAMETER;
    }
    if (header_size_increment == 0)
    {
        IGMP_SNP_LOG_ERROR("header_size_increment is 0");
        return MW_E_BAD_PARAMETER;
    }

    /* decrease payload pointer (guarded by length check above) */
    ptr_pbuf->payload = (UI8_T *)ptr_pbuf->payload - header_size_increment;
    /* modify pbuf length fields */
    ptr_pbuf->len = (UI16_T)(ptr_pbuf->len + header_size_increment);
    ptr_pbuf->tot_len = (UI16_T)(ptr_pbuf->tot_len + header_size_increment);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_packet_send(
    struct pbuf *ptr_pbuf,
    UI8_T igmp_type,
    UI32_T forward_bmp)
{
    UI32_T              port_id = ptr_pbuf->stag_hdr.rx_hdr.sp;
    UI32_T              unit = 0;
    AIR_PORT_BITMAP_T   port_matrix_pbmap = {0};

    IGMP_SNP_LOG_DEBUG("sp=%d, igmp_type 0x%x, forward_bmp=0x%x",
                    port_id, igmp_type, forward_bmp);

#ifdef IGMP_SNP_MW_SUPPORT
    /* Check trunk member */
    forward_bmp = igmp_snp_lag_updateTxBmpBySrcPort(forward_bmp, port_id);
    IGMP_SNP_LOG_DEBUG("Forward with trunk 0x%08x", forward_bmp);
#endif

    if (0 != forward_bmp)
    {
        if(AIR_E_OK == air_port_getPortMatrix(unit, port_id, port_matrix_pbmap))
        {
            AIR_PORT_DEL(port_matrix_pbmap, PLAT_CPU_PORT);
            forward_bmp &= port_matrix_pbmap[0];
            IGMP_SNP_LOG_DEBUG("Forward with port-marix 0x%08x", forward_bmp);
        }
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = (forward_bmp &= ~(1U << (port_id % 32)));
        ptr_pbuf->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = 0;
        IGMP_SNP_LOG_INFO("Forward to ports 0x%x",
                        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp);
        ethernet_output_use_default_netif(ptr_pbuf, ETHTYPE_IP);
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_packet_getIngressVlanInfo(
    struct pbuf     *ptr_pbuf,
    UI16_T          *ptr_vlan_id,
    UI8_T           *ptr_vlan_count)
{
    UI32_T              index = 0, vlan_count = 0, port_id = 0;
    IGMP_SNP_VLAN_CFG_T         *ptr_igmp_snp_vlan = NULL;

    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp_vlan);
    MW_CHECK_PTR(ptr_pbuf);
    MW_CHECK_PTR(ptr_vlan_id);

    *ptr_vlan_count = 0;
    if (IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode)
    {
        port_id = ptr_pbuf->stag_hdr.rx_hdr.sp;
        /* A port may be in multiple vlans at the same time */
        for(index = 0; index < PLAT_MAX_PORT_NUM; index++)
        {
            if((IGMP_SNP_MIN_VID != ptr_igmp_snp_vlan->vlan_entry[index].vlan_id) && (0 != (ptr_igmp_snp_vlan->vlan_entry[index].port_member & BIT(port_id))))
            {
                ptr_vlan_id[vlan_count] = ptr_igmp_snp_vlan->vlan_entry[index].vlan_id;
                vlan_count ++;
                IGMP_SNP_LOG_DEBUG("vlan mode:%d, port in vlan:%d", ptr_igmp_snp_vlan->vlan_mode, ptr_igmp_snp_vlan->vlan_entry[index].vlan_id);
            }
        }
    }
    else
    {
        vlan_count = 1;
        ptr_vlan_id[0] = ptr_pbuf->vlan_hdr.vid;
        IGMP_SNP_LOG_DEBUG("vlan mode:%d, port in vlan:%d", ptr_igmp_snp_vlan->vlan_mode, ptr_vlan_id[0]);
    }

    *ptr_vlan_count = vlan_count;
    return MW_E_OK;
}


#ifdef AIR_SUPPORT_IGMPV3_AWARE
static MW_ERROR_NO_T
_igmp_snp_packet_verifyV3ReportMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info)
{
    UI8_T                                   *p = ptr_pkt_info->ptr_pbuf->payload;
    IGMP_SNP_V3REPORT_MSG_T                 igmpv3_report_msg;
    IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T    igmpv3_report_group_record_msg;
    UI16_T                                  num_of_grp_rec;
    UI16_T                                  num_of_src;
    UI16_T                                  i;
#ifdef IGMPSNP_DEBUG
    UI32_T                                  group_ip;
#endif
    UI16_T                                  payload_len = 0;

    IGMP_SNP_LOG_DEBUG("ptr_pkt_info->payload_len=%d", ptr_pkt_info->payload_len);
    osapi_memcpy(&igmpv3_report_msg, p, sizeof(IGMP_SNP_V3REPORT_MSG_T));
    num_of_grp_rec = ntohs(igmpv3_report_msg.num_of_group_records);
    p += sizeof(IGMP_SNP_V3REPORT_MSG_T);//point to start of group record[]
    payload_len += sizeof(IGMP_SNP_V3REPORT_MSG_T);
    IGMP_SNP_LOG_DEBUG("checking payload_len=%d", payload_len);
    if (payload_len > ptr_pkt_info->payload_len)
    {
        IGMP_SNP_LOG_ERROR("payload_len=%d > ptr_pkt_info->payload_len=%d, return MW_E_BAD_PARAMETER", payload_len, ptr_pkt_info->payload_len);
        return MW_E_BAD_PARAMETER;
    }

    for (i = 0; ((i < num_of_grp_rec) && (i < IGMP_SNP_REPORTV3_CHECK_GROUP_REC_NUM)); i++)
    {
        memcpy(&igmpv3_report_group_record_msg, p, sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T));
#ifdef IGMPSNP_DEBUG
        group_ip = igmpv3_report_group_record_msg.multicast_address;
#endif
        num_of_src = ntohs(igmpv3_report_group_record_msg.num_of_sources);
        IGMP_SNP_LOG_INFO("i=%d, gip=0x%x, record_type=%d, num_of_src=%d", i, group_ip, igmpv3_report_group_record_msg.record_type, num_of_src);

        if ((igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_RECORD_TYPE_INVALID) || (igmpv3_report_group_record_msg.record_type >= IGMP_REPORTV3_RECORD_TYPE_LAST))
        {
            IGMP_SNP_LOG_ERROR("i=%d, record_type=%d, return MW_E_BAD_PARAMETER", i, igmpv3_report_group_record_msg.record_type);
            return MW_E_BAD_PARAMETER;
        }

        p += (sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T) + (num_of_src * sizeof(UI32_T)) + igmpv3_report_group_record_msg.aux_data_len);

        payload_len += (sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T) + (num_of_src * sizeof(UI32_T)) + igmpv3_report_group_record_msg.aux_data_len);
        IGMP_SNP_LOG_DEBUG("checking payload_len=%d", payload_len);
        if (payload_len > ptr_pkt_info->payload_len)
        {
            IGMP_SNP_LOG_ERROR("payload_len=%d > ptr_pkt_info->payload_len=%d, return MW_E_BAD_PARAMETER", payload_len, ptr_pkt_info->payload_len);
            return MW_E_BAD_PARAMETER;
        }
    }

    IGMP_SNP_LOG_DEBUG("payload_len=%d, ptr_pkt_info->payload_len=%d", payload_len, ptr_pkt_info->payload_len);
    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_packet_handleV3ReportMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info)
{
    I32_T                                   rc = MW_E_OK;
#ifdef IGMPSNP_DEBUG
    UI32_T                                  src_ip;
    UI32_T                                  dest_ip;
#endif
    ip4_addr_t                              group_ip;
    UI8_T                                   vlan_count = 0, index = 0;
    UI16_T                                  ptr_vlan_id[MAX_PORT_NUM] = {0};
    UI16_T                                  vlan_id = IGMP_SNP_MIN_VID;
    UI8_T                                   port_id = ptr_pkt_info->ptr_pbuf->stag_hdr.rx_hdr.sp;
    IGMP_SNP_V3REPORT_MSG_T                 igmpv3_report_msg;
    IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T    igmpv3_report_group_record_msg;
    UI16_T                                  num_of_grp_rec;
    UI16_T                                  num_of_src;
    UI16_T                                  i;
    UI8_T                                   *p = ptr_pkt_info->ptr_pbuf->payload;
    struct IGMP_SNP_GROUP_LIST_S            *ptr_grp = NULL;
    struct IGMP_SNP_MROUTER_LIST_S          *ptr_mrtr = NULL;
    UI32_T                                  tx_pbmp = 0;
    MW_PORT_BITMAP_T                        portbmp = {0};
    UI32_T                                  port = 0;
    UI32_T                                  min_trunk_port = port_id;
    IGMP_SNP_LIST_T                         *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
#ifdef IGMP_SNP_MW_SUPPORT
    min_trunk_port = igmp_snp_lag_getMinMemberPortByPortId(port_id);
#endif

    IGMP_SNP_LOG_INFO("port=%d, min_trunk_port=%d", port_id, min_trunk_port);
#ifdef IGMPSNP_DEBUG
    src_ip = ptr_pkt_info->ptr_iphdr->src.addr;
    dest_ip = ptr_pkt_info->ptr_iphdr->dest.addr;
#endif
    memcpy(&igmpv3_report_msg, p, sizeof(IGMP_SNP_V3REPORT_MSG_T));

    _igmp_snp_packet_getIngressVlanInfo(ptr_pkt_info->ptr_pbuf, ptr_vlan_id, &vlan_count);
    if(0 == vlan_count)
    {
        IGMP_SNP_LOG_ERROR("Port%d not in any vlan",min_trunk_port);
        return MW_E_BAD_PARAMETER;
    }
    IGMP_SNP_LOG_DEBUG("sip=0x%x, dip=0x%x", src_ip, dest_ip);

    num_of_grp_rec = ntohs(igmpv3_report_msg.num_of_group_records);
    IGMP_SNP_LOG_INFO("msg_type=0x%x, igmp_checksum=0x%x, num_of_grp_rec=%d", igmpv3_report_msg.msg_type, igmpv3_report_msg.igmp_checksum, num_of_grp_rec);

    if (_igmp_snp_packet_verifyV3ReportMsg(ptr_pkt_info) != MW_E_OK)
    {
        IGMP_SNP_LOG_ERROR("return MW_E_BAD_PARAMETER");
        return MW_E_BAD_PARAMETER;
    }

    p += sizeof(IGMP_SNP_V3REPORT_MSG_T);//point to start of group record[]
    for (i = 0; ((i < num_of_grp_rec) && (i < IGMP_SNP_REPORTV3_CHECK_GROUP_REC_NUM)); i++)
    {
        memcpy(&igmpv3_report_group_record_msg, p, sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T));
        group_ip.addr = igmpv3_report_group_record_msg.multicast_address;
        num_of_src = ntohs(igmpv3_report_group_record_msg.num_of_sources);
        IGMP_SNP_LOG_INFO("i=%d, gip=%s, record_type=%s, num_of_src=%d", i, ip4addr_ntoa(&group_ip), _igmpv3_record_type_vec[igmpv3_report_group_record_msg.record_type], num_of_src);

        /* Check to see if the group address is in the 224.0.0.x range.  */
        /* If it is, don't process it... just forward to all ports in the vlan except incoming, otherwise add it to the table. */
        if (((ntohl(group_ip.addr) >= 0xE0000000) && (ntohl(group_ip.addr) <= 0xE00000FF)) ||
             (ntohl(group_ip.addr) == 0xEFFFFFFA))
        {
            /* Flood to all other ports in VLAN.  */
            for(index = 0; index < vlan_count; index++)
            {
                portbmp[0] |= igmp_snp_vlan_getMember(ptr_vlan_id[index]);
            }
            portbmp[0] &= ~(1U << (min_trunk_port % PLAT_MAX_PORT_NUM));/*Remove the rx port*/
            if(0 != portbmp[0])
            {
                ptr_pkt_info->forward_bmp |= portbmp[0]; /* Forward IN VLAN */
                IGMP_SNP_LOG_INFO("Forward to member ports 0x%x in VLAN", portbmp[0]);

                MWLIB_BITMAP_BIT_FOREACH(portbmp, port, MW_PORT_BITMAP_SIZE)
                {
                    IGMPSNP_STATS_INC(igmpsnoop.tx_join);
                }
            }
            p += (sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T) + (num_of_src * sizeof(UI32_T)) + igmpv3_report_group_record_msg.aux_data_len);
            continue;
        }

        /* Check if group ip is multicast address */
        else if (!ip4_addr_ismulticast(&group_ip))
        {
            IGMP_SNP_LOG_INFO("[%d] group ip is not multicast address, group_ip: %s", i, ip4addr_ntoa(&group_ip));
            p += (sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T) + (num_of_src * sizeof(UI32_T)) + igmpv3_report_group_record_msg.aux_data_len);
            continue;
        }

        /* TO_IN(NULL)/IS_IN(NULL) is treated as Leave. */
        /* IS_IN(A)/IS_EX(A)/IS_EX(NULL)/TO_IN(A)/TO_EX(A)/TO_EX(NULL)/ALLOW(A)/BLOCK(A) is treated as Join. */
        /* ALLOW(NULL)/BLOCK(NULL) is treated as Join. */
        if (((igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_CHANGE_TO_INCLUDE) ||
             ((igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_IS_INCLUDE))) && (num_of_src == 0))
        {
            for(index = 0; index < vlan_count; index++)
            {
                vlan_id = ptr_vlan_id[index];
                ptr_grp = NULL;
                /* Leave */
                IGMP_SNP_LOG_INFO("Leave");
                /* Check the fast-leave mode. */
                ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
                if((ptr_grp != NULL) && (TRUE == MW_PORT_CHK(ptr_grp->portbmp, min_trunk_port)))
                {
                    /* RFC2236 3. Queriers should ignore Leave Group message for which there are no group members on reception interface */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                    ip4_addr_t    group_addr;
                    MW_ERROR_NO_T err = MW_E_OK;

                    ip4_addr_set_u32(&group_addr, igmpv3_report_group_record_msg.multicast_address);
                    err = igmp_querier_handleLeaveMsg(vlan_id, group_addr);
                    if(MW_E_OK != err)
                    {
                        IGMP_SNP_LOG_WARN("igmp_querier_handleLeaveMsg failed err:%x", err);
                    }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

                    if (TRUE == ptr_igmp_snp->cfg_info.fast_leave)
                    {
                        IGMP_SNP_LOG_DEBUG("delete port %d, set ptr_grp->timer[port=%d] = 0", min_trunk_port, min_trunk_port);
                        MW_PORT_DEL(ptr_grp->portbmp, min_trunk_port);
                        ptr_grp->timer[(min_trunk_port - 1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                        ptr_igmp_snp->port_group_num[(min_trunk_port - 1)]--;
#endif
                        if (0 == ptr_grp->portbmp[0])
                        {
                            IGMP_SNP_LOG_DEBUG("remove group");
                            igmp_snp_delGroupEntry((void **)&ptr_grp);
                        }
                        else
                        {
                            igmp_snp_updateGroupEntry(ptr_grp);
                        }
                    }
                    else
                    {
                        if (ptr_grp->timer[(min_trunk_port - 1)] > ((IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL)) * IGMP_SNP_TMR_RATE_S)
                        {
                            IGMP_SNP_LOG_DEBUG("port=%d, update timer to %d sec", min_trunk_port, (IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL));
                            ptr_grp->timer[(min_trunk_port - 1)] = ((IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL) * IGMP_SNP_TMR_RATE_S);
                        }
                    }
                }

                /* Flood the leave on the mrouter interfaces in all VLANs. */
                ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
                if(NULL != ptr_mrtr)
                {
                    tx_pbmp = (ptr_mrtr->portbmp[0] & igmp_snp_vlan_getMember(vlan_id));
                    IGMP_SNP_LOG_DEBUG("Forward to mrouter ports 0x%x in VLAN %d", ptr_mrtr->portbmp[0], ptr_mrtr->vlan_id);
                }
                if (tx_pbmp)/* When router port exist, IGMP v1v2 report and v2 leave only forward to router port.*/
                {
                    ptr_pkt_info->forward_bmp |= tx_pbmp; /* Forward to router */
                }
                else/* When no router port, IGMP v1v2 report and v2 leave flood to all ports.*/
                {
                    ptr_pkt_info->forward_bmp |= igmp_snp_vlan_getMember(vlan_id); /* Forward in VLAN */
                }
            }
        }
        else if ((((igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_IS_INCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_IS_EXCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_CHANGE_TO_INCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_CHANGE_TO_EXCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_ALLOW_NEW_SOURCE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_BLOCK_OLD_SOURCE)) && (num_of_src != 0)) ||
                 (((igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_IS_EXCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_CHANGE_TO_EXCLUDE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_ALLOW_NEW_SOURCE) ||
                   (igmpv3_report_group_record_msg.record_type == IGMP_REPORTV3_BLOCK_OLD_SOURCE)) && (num_of_src == 0)))
        {
            for(index = 0; index < vlan_count; index++)
            {
                vlan_id = ptr_vlan_id[index];
                /* Join */
                IGMP_SNP_LOG_INFO("Join");
                ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
                if (ptr_grp != NULL)
                {
                    /* entry exists. */
                    if (MW_PORT_CHK(ptr_grp->portbmp, min_trunk_port) == TRUE)
                    {
                        IGMP_SNP_LOG_DEBUG("update timer to %d sec", IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
                        ptr_grp->timer[(min_trunk_port - 1)] = (IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL) * IGMP_SNP_TMR_RATE_S;
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
                        igmp_snp_updateGroupEntry(ptr_grp);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

                    }
                    else
                    {
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                        if (ptr_igmp_snp->port_group_num[(min_trunk_port - 1)] >= IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT)
                        {
                            IGMP_SNP_LOG_INFO("Num of group entry on port %d is %d, which reachs the max num %d", min_trunk_port, ptr_igmp_snp->port_group_num[(min_trunk_port - 1)], IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT);
                        }
                        else
#endif /* IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT */
                        {
                            MW_PORT_ADD(ptr_grp->portbmp, min_trunk_port);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                            ptr_igmp_snp->port_group_num[(min_trunk_port - 1)]++;
#endif
                            igmp_snp_updateGroupEntry(ptr_grp);
                            IGMP_SNP_LOG_DEBUG("update timer to %d sec", IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
                            ptr_grp->timer[(min_trunk_port - 1)] = (IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL) * IGMP_SNP_TMR_RATE_S;
                        }
                    }
                }
                else
                {
                    /* create new entry */
                    rc = igmp_snp_addGroupEntry(vlan_id, &group_ip, min_trunk_port, IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
                    if (MW_E_OK == rc)
                    {
                        ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
                    }
                    else
                    {
                        IGMP_SNP_LOG_ERROR("add group failed, rc:%d", rc);
                        return rc;
                    }
                }

                if (ptr_grp != NULL)
                {
                    /* rfc4541, 2.1.1, 1) A snooping switch should forward IGMP Membership Reports only to those ports where multicast routers are attached. */
                    tx_pbmp = 0;
                    ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
                    if(NULL != ptr_mrtr)
                    {
                        tx_pbmp = (ptr_mrtr->portbmp[0] & igmp_snp_vlan_getMember(vlan_id));
                        IGMP_SNP_LOG_DEBUG("Forward to mrouter ports 0x%x in VLAN %d", ptr_mrtr->portbmp[0], ptr_mrtr->vlan_id);
                    }
                    if (tx_pbmp)/* When router port exist, IGMP v1v2 report and v2 leave only forward to router port.*/
                    {
                        ptr_pkt_info->forward_bmp |= tx_pbmp; /* Forward to router */
                    }
                    else/* When no router port, IGMP v1v2 report and v2 leave flood to all ports.*/
                    {
                        ptr_pkt_info->forward_bmp |= igmp_snp_vlan_getMember(vlan_id); /* Forward in VLAN */
                    }
                }
                else
                {
                    IGMP_SNP_LOG_ERROR("Error: ptr_grp is NULL.");
                    return MW_E_OTHERS;
                }
            }
        }

        p += (sizeof(IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T) + (num_of_src * sizeof(UI32_T)) + igmpv3_report_group_record_msg.aux_data_len);
    }

    IGMP_SNP_LOG_INFO("return");
    return MW_E_OK;
}
#endif

static MW_ERROR_NO_T
_igmp_snp_packet_handleJoinMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info)
{
    I32_T              rc = MW_E_OK;
    struct igmp_msg    igmp_msg;
#ifdef IGMPSNP_DEBUG
    ip4_addr_t         src_ip;
#endif
    ip4_addr_t         dest_ip;
    ip4_addr_t         group_ip;
    UI8_T              dmac[MW_MAC_ADDR_LEN];
    UI8_T              vlan_count = 0, index = 0;
    UI16_T             ptr_vlan_id[MAX_PORT_NUM] = {0};
    UI16_T             vlan_id = IGMP_SNP_MIN_VID;
    UI8_T              port_id = ptr_pkt_info->ptr_pbuf->stag_hdr.rx_hdr.sp;
    UI32_T             port = 0;
    MW_PORT_BITMAP_T   portbmp = {0};
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;
    UI32_T             min_trunk_port = port_id;
    IGMP_SNP_LIST_T    *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
#ifdef IGMP_SNP_MW_SUPPORT
    min_trunk_port = igmp_snp_lag_getMinMemberPortByPortId(port_id);
#endif
    IGMPSNP_STATS_INC(igmpsnoop.rx_report);
#ifdef IGMPSNP_DEBUG
    src_ip.addr = ptr_pkt_info->ptr_iphdr->src.addr;
#endif
    dest_ip.addr = ptr_pkt_info->ptr_iphdr->dest.addr;
    memcpy(&igmp_msg, ptr_pkt_info->ptr_pbuf->payload, sizeof(struct igmp_msg));
    group_ip.addr = igmp_msg.igmp_group_address.addr;

    IGMP_SNP_LOG_DEBUG("sip=%s, dip=%s", ip4addr_ntoa(&src_ip), ip4addr_ntoa(&dest_ip));
    IGMP_SNP_LOG_INFO("gip=%s, port=%d, min_trunk_port=%d", ip4addr_ntoa(&group_ip), port_id, min_trunk_port);

    _igmp_snp_packet_getIngressVlanInfo(ptr_pkt_info->ptr_pbuf, ptr_vlan_id, &vlan_count);
    if(0 == vlan_count)
    {
        IGMP_SNP_LOG_ERROR("Port%d not in any vlan",min_trunk_port);
        return MW_E_BAD_PARAMETER;
    }

    /* Validate group address in payload. */
    if (!ip4_addr_ismulticast(&group_ip))
    {
        IGMP_SNP_LOG_ERROR("group IP %s is not valid multicast IP address", ip4addr_ntoa(&group_ip));
        IGMPSNP_STATS_INC(igmpsnoop.proterr);
        return MW_E_BAD_PARAMETER;
    }

    if (!ip4_addr_cmp(&dest_ip, &group_ip))
    {
        IGMP_SNP_LOG_ERROR("dest IP %s is different from group IP %s", ip4addr_ntoa(&dest_ip), ip4addr_ntoa(&group_ip));
        IGMPSNP_STATS_INC(igmpsnoop.proterr);
        return MW_E_BAD_PARAMETER;
    }

    /* Validate group address. */
    osapi_memset(dmac, 0, sizeof(dmac));
    osapi_memcpy(dmac, ptr_pkt_info->ptr_pbuf->ether_hdr.dest.addr, sizeof(dmac));
    if (FALSE == IGMP_SNP_DMAC_IS_MCAST(dmac))
    {
        IGMP_SNP_LOG_ERROR("Invalid dest mac %02x:%02x:%02x:%02x:%02x:%02x from group address", dmac[0], dmac[1], dmac[2], dmac[3], dmac[4], dmac[5]);
        return MW_E_BAD_PARAMETER;
    }

    /* Check to see if the group address is in the 224.0.0.x range.  */
    /* If it is, don't process it... just forward to all ports in the vlan except incoming, otherwise add it to the table. */
    if (((ntohl(group_ip.addr) >= 0xE0000000) && (ntohl(group_ip.addr) <= 0xE00000FF)) ||
         (ntohl(group_ip.addr) == 0xEFFFFFFA))
    {
        /* Flood to all other ports in VLAN.  */
        for(index = 0; index < vlan_count; index++)
        {
            portbmp[0] |= igmp_snp_vlan_getMember(ptr_vlan_id[index]);
        }
        portbmp[0] &= ~(1U << (min_trunk_port % PLAT_MAX_PORT_NUM));/*Remove the rx port*/
        if(0 != portbmp[0])
        {
            ptr_pkt_info->forward_bmp |= portbmp[0]; /* Forward IN VLAN */
            IGMP_SNP_LOG_INFO("Forward to member ports 0x%x in VLAN", portbmp[0]);

            MWLIB_BITMAP_BIT_FOREACH(portbmp, port, MW_PORT_BITMAP_SIZE)
            {
                IGMPSNP_STATS_INC(igmpsnoop.tx_join);
            }
        }
        return MW_E_OK;
    }

    for(index = 0; index < vlan_count; index++)
    {
        vlan_id = ptr_vlan_id[index];
        ptr_grp = NULL;
        ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
        if (NULL != ptr_grp)
        {
            ptr_grp->leave_flag_bmp &= ~(1 << min_trunk_port);
            /* entry exists. */
            if (MW_PORT_CHK(ptr_grp->portbmp, min_trunk_port) == TRUE)
            {
                IGMP_SNP_LOG_DEBUG("update timer to %d sec", IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
                ptr_grp->timer[(min_trunk_port - 1)] = (IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL) * IGMP_SNP_TMR_RATE_S;
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
                igmp_snp_updateGroupEntry(ptr_grp);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
            }
            else
            {
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                if (ptr_igmp_snp->port_group_num[(min_trunk_port - 1)] >= IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT)
                {
                    IGMP_SNP_LOG_INFO("Num of group entry on port %d is %d, which reachs the max num %d", min_trunk_port, ptr_igmp_snp->port_group_num[(min_trunk_port - 1)], IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT);
                }
                else
#endif /* IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT */
                {
                    MW_PORT_ADD(ptr_grp->portbmp, min_trunk_port);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                    ptr_igmp_snp->port_group_num[(min_trunk_port - 1)]++;
#endif
                    igmp_snp_updateGroupEntry(ptr_grp);
                    IGMP_SNP_LOG_DEBUG("update timer to %d sec", IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
                    ptr_grp->timer[(min_trunk_port - 1)] = (IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL) * IGMP_SNP_TMR_RATE_S;
                }
            }
        }
        else
        {
            /* create new entry */
            rc = igmp_snp_addGroupEntry(vlan_id, &group_ip, min_trunk_port, IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL);
            if (MW_E_OK == rc)
            {
                ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
            }
            else
            {
                IGMP_SNP_LOG_ERROR("add group failed, rc:%d", rc);
                return rc;
            }
        }

        if (ptr_grp != NULL)
        {
            /* Report Suppress[mode, flag] - send: [0, x], [1, 0]; no send: [1,1]. */
            if ((!ptr_igmp_snp->cfg_info.rpt_suppress) ||
                (!ptr_grp->rpt_suppress_flag))
            {
                /* rfc4541, 2.1.1, 1) A snooping switch should forward IGMP Membership Reports only to those ports where multicast routers are attached. */
                struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
                UI32_T tx_pbmp = 0;
                ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
                if(NULL != ptr_mrtr)
                {
                    tx_pbmp |= (ptr_mrtr->portbmp[0] & igmp_snp_vlan_getMember(vlan_id));
                }
                if (0 < tx_pbmp)/* When router port exist, IGMP v1v2 report and v2 leave only forward to router port.*/
                {
                    ptr_pkt_info->forward_bmp |= tx_pbmp; /* Forward to router */
                    portbmp[0] = (tx_pbmp &= ~(1U << (min_trunk_port % 32)));
                    IGMP_SNP_LOG_INFO("Forward IGMP report to mrouter portbmp 0x%x in VLAN %d", portbmp[0], vlan_id);
                    MWLIB_BITMAP_BIT_FOREACH(portbmp, port, MW_PORT_BITMAP_SIZE)
                    {
                        IGMPSNP_STATS_INC(igmpsnoop.tx_join);
                    }
                }
                else/* When no router port, IGMP v1v2 report and v2 leave flood to all ports.*/
                {
                    ptr_pkt_info->forward_bmp |= igmp_snp_vlan_getMember(vlan_id); /* Forward in VLAN */
                }

                if (ptr_igmp_snp->cfg_info.rpt_suppress)
                {
                    if ((ptr_grp->rpt_suppress_timer || ptr_igmp_snp->glb_suppress_timer) &&   /* Supress timer is counting down. */
                        (!ptr_grp->rpt_suppress_flag))     /* received the 1st report. */
                    {
                        IGMP_SNP_LOG_INFO("Set rpt_suppress_flag = TRUE");
                        ptr_grp->rpt_suppress_flag = TRUE;
                    }
                }
            }
        }
        else
        {
            IGMP_SNP_LOG_ERROR("Error: ptr_grp is NULL.");
            return MW_E_OTHERS;
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_packet_handleLeaveMSg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info)
{
    MW_ERROR_NO_T      rc = MW_E_OK;
    struct igmp_msg    igmp_msg;
    ip4_addr_t         src_ip;
    ip4_addr_t         dest_ip;
    ip4_addr_t         group_ip;
    UI8_T              dmac[MW_MAC_ADDR_LEN];
    UI8_T              vlan_count = 0, index = 0;
    UI16_T             ptr_vlan_id[MAX_PORT_NUM] = {0};
    UI16_T             vlan_id = IGMP_SNP_MIN_VID;
    UI8_T              port_id = ptr_pkt_info->ptr_pbuf->stag_hdr.rx_hdr.sp;
    UI32_T             port = 0;
    MW_PORT_BITMAP_T   portbmp;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    UI32_T             tx_pbmp = 0;
    UI32_T             min_trunk_port = port_id;
    IGMP_SNP_LIST_T    *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
#ifdef IGMP_SNP_MW_SUPPORT
    min_trunk_port = igmp_snp_lag_getMinMemberPortByPortId(port_id);
#endif
    IGMPSNP_STATS_INC(igmpsnoop.rx_leave);
    memcpy(&igmp_msg, ptr_pkt_info->ptr_pbuf->payload, sizeof(struct igmp_msg));
    src_ip.addr = ptr_pkt_info->ptr_iphdr->src.addr;
    dest_ip.addr = ptr_pkt_info->ptr_iphdr->dest.addr;
    group_ip.addr = igmp_msg.igmp_group_address.addr;
    IGMP_SNP_LOG_DEBUG("sip=%s", ip4addr_ntoa((const ip4_addr_t *)&src_ip));
    IGMP_SNP_LOG_DEBUG("dip=%s", ip4addr_ntoa((const ip4_addr_t *)&dest_ip));
    IGMP_SNP_LOG_INFO("gip=%s, port=%d, min_trunk_port=%d", ip4addr_ntoa((const ip4_addr_t *)&group_ip), port_id, min_trunk_port);

    _igmp_snp_packet_getIngressVlanInfo(ptr_pkt_info->ptr_pbuf, ptr_vlan_id, &vlan_count);
    if(0 == vlan_count)
    {
        IGMP_SNP_LOG_ERROR("Port%d not in any vlan",min_trunk_port);
        return MW_E_BAD_PARAMETER;
    }

    if (!ip4_addr_ismulticast(&group_ip))
    {
        IGMP_SNP_LOG_ERROR("group IP %s is not valid multicast IP address", ip4addr_ntoa(&group_ip));
        IGMPSNP_STATS_INC(igmpsnoop.proterr);
        return MW_E_BAD_PARAMETER;
    }

    /* Validate group address. */
    osapi_memset(dmac, 0, sizeof(dmac));
    osapi_memcpy(dmac, ptr_pkt_info->ptr_pbuf->ether_hdr.dest.addr, sizeof(dmac));
    if (FALSE == IGMP_SNP_DMAC_IS_MCAST(dmac))
    {
        IGMP_SNP_LOG_ERROR("Invalid dest mac address %02x:%02x:%02x:%02x:%02x:%02x", dmac[0], dmac[1], dmac[2], dmac[3], dmac[4], dmac[5]);
        return MW_E_BAD_PARAMETER;
    }

    /* Check if the DIP is 224.0.0.2. */
    if (!ip4_addr_cmp(&dest_ip, &_igmp_snp_allrouters))
    {
        IGMP_SNP_LOG_ERROR("dest IP %s is different from all router IP 224.0.0.2", ip4addr_ntoa((const ip4_addr_t *)&dest_ip));
        IGMPSNP_STATS_INC(igmpsnoop.proterr);
        return MW_E_BAD_PARAMETER;
    }

    for(index = 0; index < vlan_count; index++)
    {
        vlan_id = ptr_vlan_id[index];
        ptr_grp = NULL;
        ptr_mrtr = NULL;

        /* Check the fast-leave mode. */
        ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
        if((ptr_grp != NULL) && (TRUE == MW_PORT_CHK(ptr_grp->portbmp, min_trunk_port)))
        {
            /* RFC2236 3. Queriers should ignore Leave Group message for which there are no group members on reception interface */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
            ip4_addr_t    group_addr;
            MW_ERROR_NO_T err = MW_E_OK;

            ip4_addr_copy(group_addr, igmp_msg.igmp_group_address);
            err = igmp_querier_handleLeaveMsg(vlan_id, group_addr);
            if(MW_E_OK != err)
            {
                IGMP_SNP_LOG_WARN("igmp_querier_handleLeaveMsg failed err:%x, vid:%d", err, vlan_id);
            }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

            if (TRUE == ptr_igmp_snp->cfg_info.fast_leave)
            {
                IGMP_SNP_LOG_DEBUG("delete port %d", min_trunk_port);
                MW_PORT_DEL(ptr_grp->portbmp, min_trunk_port);
                ptr_grp->timer[(min_trunk_port - 1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                ptr_igmp_snp->port_group_num[(min_trunk_port - 1)]--;
#endif
                if (0 == ptr_grp->portbmp[0])
                {
                    IGMP_SNP_LOG_DEBUG("remove group");
                    igmp_snp_delGroupEntry((void **)&ptr_grp);
                }
                else
                {
                    igmp_snp_updateGroupEntry(ptr_grp);
                }
            }
            else
            {
                if (ptr_grp->timer[(min_trunk_port - 1)] > ((IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL)) * IGMP_SNP_TMR_RATE_S)
                {
                    IGMP_SNP_LOG_DEBUG("port=%d, update timer to %d sec", min_trunk_port, (IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL));
                    ptr_grp->timer[(min_trunk_port - 1)] = ((IGMP_SNP_LAST_MEM_QUERY_CNT * IGMP_SNP_LAST_MEM_QUERY_INTERVAL) * IGMP_SNP_TMR_RATE_S);
                }
                ptr_grp->leave_flag_bmp |= (1 << min_trunk_port);
            }
        }
        /* Flood the leave on the mrouter interfaces in all VLANs. */
        ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
        if(NULL != ptr_mrtr)
        {
            tx_pbmp |= (ptr_mrtr->portbmp[0] & igmp_snp_vlan_getMember(vlan_id));
        }
        if (0 < tx_pbmp)/* When router port exist, IGMP v1v2 report and v2 leave only forward to router port.*/
        {
            ptr_pkt_info->forward_bmp |= tx_pbmp; /* Forward to router */
            portbmp[0] = (tx_pbmp &= ~(1U << (min_trunk_port % 32)));
            IGMP_SNP_LOG_INFO("Forward IGMP report to mrouter portbmp 0x%x in VLAN %d", portbmp[0], vlan_id);
            MWLIB_BITMAP_BIT_FOREACH(portbmp, port, MW_PORT_BITMAP_SIZE)
            {
                IGMPSNP_STATS_INC(igmpsnoop.tx_leave);
            }
        }
        else/* When no router port, IGMP v1v2 report and v2 leave flood to all ports.*/
        {
            ptr_pkt_info->forward_bmp |= igmp_snp_vlan_getMember(vlan_id); /* Forward in VLAN */
        }

    }

    return rc;
}

static MW_ERROR_NO_T
_igmp_snp_packet_handleQueryMsg(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info)
{
    MW_ERROR_NO_T      rc = MW_E_OTHERS;
    struct             igmp_msg igmp_msg;
#ifdef IGMPSNP_DEBUG
    ip4_addr_t         src_ip;
#endif
    ip4_addr_t         dest_ip;
    ip4_addr_t         group_ip;
    UI8_T              dmac[MW_MAC_ADDR_LEN] = {0};
    UI8_T              vlan_count = 0, index = 0;
    UI16_T             ptr_vlan_id[MAX_PORT_NUM] = {0};
    UI16_T             vlan_id = IGMP_SNP_MIN_VID;
    UI8_T              port_id = ptr_pkt_info->ptr_pbuf->stag_hdr.rx_hdr.sp;
    UI32_T             tx_pbmp = 0;
    UI32_T             port = 0;
    MW_PORT_BITMAP_T   portbmp;
    UI32_T             min_trunk_port = port_id;
    BOOL_T             b_recv_gs_query = FALSE;
    IGMP_SNP_LIST_T    *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
#ifdef IGMP_SNP_MW_SUPPORT
    min_trunk_port = igmp_snp_lag_getMinMemberPortByPortId(port_id);
#endif
#ifdef IGMPSNP_DEBUG
    src_ip.addr = ptr_pkt_info->ptr_iphdr->src.addr;
#endif
    dest_ip.addr = ptr_pkt_info->ptr_iphdr->dest.addr;
    memcpy(&igmp_msg, ptr_pkt_info->ptr_pbuf->payload, sizeof(struct igmp_msg));
    group_ip.addr = igmp_msg.igmp_group_address.addr;
    IGMP_SNP_LOG_DEBUG("sip=%s, dip=%s", ip4addr_ntoa((const ip4_addr_t *)&src_ip), ip4addr_ntoa((const ip4_addr_t *)&dest_ip));
    IGMP_SNP_LOG_INFO("port=%d, min_trunk_port=%d, maxresp=%d, gip=%s", port_id, min_trunk_port, igmp_msg.igmp_maxresp, ip4addr_ntoa((const ip4_addr_t *)&group_ip));

    _igmp_snp_packet_getIngressVlanInfo(ptr_pkt_info->ptr_pbuf, ptr_vlan_id, &vlan_count);
    if(0 == vlan_count)
    {
        IGMP_SNP_LOG_ERROR("Port%d not in any vlan",min_trunk_port);
        return MW_E_BAD_PARAMETER;
    }

    /* Validate group address. */
    osapi_memcpy(dmac, ptr_pkt_info->ptr_pbuf->ether_hdr.dest.addr, sizeof(dmac));
    if (FALSE == IGMP_SNP_DMAC_IS_MCAST(dmac))
    {
        IGMP_SNP_LOG_ERROR("Invalid dest mac address %02x:%02x:%02x:%02x:%02x:%02x", dmac[0], dmac[1], dmac[2], dmac[3], dmac[4], dmac[5]);
        rc = MW_E_BAD_PARAMETER;
    }

    /* rfc4541, 2.1.1, 1) b)The arrival port for IGMP Queries where sip is not 0.  */
    if (ptr_pkt_info->ptr_iphdr->src.addr != 0)
    {
        if(IPADDR_ANY == group_ip.addr)
        {
            /* General query */
            if (!ip4_addr_cmp(&ptr_pkt_info->ptr_iphdr->dest, &_igmp_snp_allsystems))
            {
                IGMP_SNP_LOG_ERROR("Invalid destIp %s", ip4addr_ntoa((const ip4_addr_t *) &dest_ip));
                IGMPSNP_STATS_INC(igmpsnoop.proterr);
            }
            else
            {
                if (0 == igmp_msg.igmp_maxresp)
                {
                    IGMP_SNP_LOG_INFO("got an all hosts query with time== 0 - this is V1 and not implemented - treat as v2");
                    IGMPSNP_STATS_INC(igmpsnoop.rx_v1);
                }
                else
                {
                    IGMPSNP_STATS_INC(igmpsnoop.rx_general);
                }
                rc = MW_E_OK;
            }
        }
        else
        {
            /* Group specific query */
            if (!ip4_addr_ismulticast(&group_ip))
            {
                if (!ip4_addr_cmp(&dest_ip, &group_ip))
                {
                    IGMP_SNP_LOG_ERROR("Non-matching destIp:%s and groupIp:%s", ip4addr_ntoa(&dest_ip), ip4addr_ntoa(&group_ip));
                    IGMPSNP_STATS_INC(igmpsnoop.proterr);
                }
                else
                {
                    b_recv_gs_query = TRUE;
                    IGMPSNP_STATS_INC(igmpsnoop.rx_group);
                    rc = MW_E_OK;
                }
            }
            else
            {
                IGMP_SNP_LOG_ERROR("Invalid groupIp %s", ip4addr_ntoa(&group_ip));
                IGMPSNP_STATS_INC(igmpsnoop.proterr);
            }
        }
    }

    if (MW_E_OK == rc)
    {
        for(index = 0; index < vlan_count; index++)
        {
            vlan_id = ptr_vlan_id[index];
            tx_pbmp = 0;
            if(IGMP_SNP_MIN_VID == vlan_id)
            {
                break;
            }
            igmp_snp_port_addMrouterPort(vlan_id, min_trunk_port, IGMP_SNP_OTHER_QUERIER_PRESENT_INTERVAL);
            if(TRUE ==  b_recv_gs_query)
            {
                struct IGMP_SNP_GROUP_LIST_S *ptr_grp = igmp_snp_searchGroupEntry(vlan_id, &group_ip);
                if((NULL != ptr_grp) && (0 != ptr_grp->leave_flag_bmp))
                {
                    if((0 != igmp_msg.igmp_maxresp))
                    {
                        /* update group timer to max response time(in GS query packet) * Robustness value(local setting) */
                        for(port = 1; port <= PLAT_MAX_PORT_NUM; port ++)
                        {
                            if(0 != (ptr_grp->leave_flag_bmp & (1 << port)))
                            {
                                /* Adjust the order of operations, ptr_grp->timer[port] = (((igmp_msg.igmp_maxresp / 10) * IGMP_SNP_ROBUSTNESS) * IGMP_SNP_TMR_RATE_S) */
                                ptr_grp->timer[(port - 1)] = ((igmp_msg.igmp_maxresp * IGMP_SNP_ROBUSTNESS * IGMP_SNP_TMR_RATE_S) / 10);
                                IGMP_SNP_LOG_DEBUG("Received GS query packet, update ptr_grp->timer[%d] to %d Sec\n", (port-1), (ptr_grp->timer[(port-1)] / IGMP_SNP_TMR_RATE_S));
                            }
                        }
                    }
                    ptr_grp->leave_flag_bmp = 0;
                }
            }
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
            ip4_addr_t    src_addr;
            MW_ERROR_NO_T err = MW_E_OK;

            ip4_addr_copy(src_addr, ptr_pkt_info->ptr_iphdr->src);
            err = igmp_querier_handleQueryMsg(vlan_id, src_addr);
            if(MW_E_OK != err)
            {
                IGMP_SNP_LOG_WARN("igmp_querier_handleQueryMsg failed err:%x", err);
            }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

            /* Report Suppress operation */
            if(TRUE == ptr_igmp_snp->cfg_info.rpt_suppress)
            {
                if(IPADDR_ANY != group_ip.addr)
                {
                    /* Suppression of specific query [unit:sec] */
                    igmp_snp_packet_reportSuppProcess(vlan_id, &group_ip, min_trunk_port, ((0 != igmp_msg.igmp_maxresp) ? (igmp_msg.igmp_maxresp / 10) : IGMP_SNP_QUERY_RESP_TIME));
                }
                else
                {
                    /* Suppression of general query [unit:sec] */
                    if(0 != igmp_msg.igmp_maxresp)
                    {
                        ptr_igmp_snp->glb_suppress_timer = ((igmp_msg.igmp_maxresp / 10) * IGMP_SNP_TMR_RATE_S);
                    }
                }
            }

            /* Flood to all other ports in VLAN.  */
            tx_pbmp = igmp_snp_vlan_getMember(vlan_id);
            IGMP_SNP_LOG_INFO("Forward to member ports 0x%x in VLAN %d", tx_pbmp, vlan_id);

            /* Forward query to all router ports of specific VLANs. */
            if(0 != tx_pbmp)
            {
                ptr_pkt_info->forward_bmp |= tx_pbmp; /* Forward in VLAN */

                portbmp[0] = (tx_pbmp &= ~(1U << (min_trunk_port % 32)));
                MWLIB_BITMAP_BIT_FOREACH(portbmp, port, MW_PORT_BITMAP_SIZE)
                {
                    if(IPADDR_ANY == group_ip.addr)
                    {
                        IGMPSNP_STATS_INC(igmpsnoop.tx_general);
                    }
                    else
                    {
                        IGMPSNP_STATS_INC(igmpsnoop.tx_group);
                    }
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   _igmp_snp_packet_parsePacket
 * PURPOSE:
 *      This API is used to process a IGMP message.
 *
 * INPUT:
 *      ptr_pkt_info      --  the data struct include L2,L3 info and igmp payload
 *      igmpv3_query      --  igmpv3 query or not
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      While IGMP Snooping is Enabled.
 *      1. IGMPv2 Report and Leave: Forward to router port.
 *      2. IGMP Query: Forward to router port and flood in VLAN.
 *      3. IGMPv3 Report: Forward to router port.
 *      4. Fault Report/Leave/Query: Forward to router port and flood in VLAN.
 *      5. Unknown or checksum error packet: Forward to router port and flood in VLAN.
 */
static MW_ERROR_NO_T
_igmp_snp_packet_parsePacket(
    IGMP_SNP_PACKET_INFO_T *ptr_pkt_info,
    UI8_T igmpv3_query)
{
    UI8_T               forward_opt = 0;
    I32_T               rc = MW_E_OK;
    struct igmp_msg     *ptr_igmp = ptr_pkt_info->ptr_pbuf->payload;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    IGMPSNP_STATS_INC(igmpsnoop.recv);
    ptr_pkt_info->forward_bmp = 0;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    if (ENABLE == ptr_igmp_snp->cfg_info.enable)
    {
        /* Note that the length CAN be greater than 8 but only 8 are used - All are included in the checksum */
        if (ptr_pkt_info->ptr_pbuf->len < IGMP_MINLEN)
        {
            IGMPSNP_STATS_INC(igmpsnoop.lenerr);
            IGMP_SNP_LOG_ERROR("length error");
            return MW_E_BAD_PARAMETER;
        }

        switch(IGMP_TYPE(ptr_igmp))
        {
            case IGMP_MEMB_QUERY:
                if (igmpv3_query)
                {
                    /* IGMP v3 Query flooding to all ports. */
                    /* The port receive IGMP v3 Query will not become router port. */
                    IGMP_SNP_LOG_INFO("IGMP V3 Query");
#ifdef AIR_SUPPORT_IGMPV3_AWARE
                    if (ENABLE == ptr_igmp_snp->cfg_info.igmpv3_aware)
                    {
                        if (MW_E_OK != _igmp_snp_packet_handleQueryMsg(ptr_pkt_info))
                        {
                            ptr_pkt_info->forward_bmp = 0;
                        }
                    }
                    else
#endif
                    {
                        forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                    }

                    break;
                }

                /* Now calculate and check the checksum */
                if (inet_chksum(ptr_igmp, IGMP_MINLEN))
                {
                    IGMPSNP_STATS_INC(igmpsnoop.chkerr);
                    IGMP_SNP_LOG_ERROR("checksum error");
                    ptr_pkt_info->forward_bmp = 0;
                }
                else
                {
                    rc = _igmp_snp_packet_handleQueryMsg(ptr_pkt_info);
                    if (MW_E_BAD_PARAMETER == rc)
                    {
                        ptr_pkt_info->forward_bmp = 0;
                    }
                    else if (MW_E_OK != rc)
                    {
                        forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                    }
                }
                break;
            case IGMP_V1_MEMB_REPORT:
            case IGMP_V2_MEMB_REPORT:
                /* Now calculate and check the checksum */
                if (inet_chksum(ptr_igmp, IGMP_MINLEN))
                {
                    IGMPSNP_STATS_INC(igmpsnoop.chkerr);
                    IGMP_SNP_LOG_ERROR("checksum error");
                    ptr_pkt_info->forward_bmp = 0;
                }
                else
                {
                    rc = _igmp_snp_packet_handleJoinMsg(ptr_pkt_info);
                    if (MW_E_BAD_PARAMETER == rc)
                    {
                        ptr_pkt_info->forward_bmp = 0;
                    }
                    else if (MW_E_OK != rc)
                    {
                        forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                    }
                }
                break;
            case IGMP_LEAVE_GROUP:
                /* Now calculate and check the checksum */
                if (inet_chksum(ptr_igmp, IGMP_MINLEN))
                {
                    IGMPSNP_STATS_INC(igmpsnoop.chkerr);
                    IGMP_SNP_LOG_ERROR("checksum error");
                    ptr_pkt_info->forward_bmp = 0;
                }
                else
                {
                    rc = _igmp_snp_packet_handleLeaveMSg(ptr_pkt_info);
                    if (MW_E_BAD_PARAMETER == rc)
                    {
                        ptr_pkt_info->forward_bmp = 0;
                    }
                    else if (MW_E_OK != rc)
                    {
                        forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                    }
                }
                break;
            case IGMP_V3_MEMB_REPORT:
                IGMP_SNP_LOG_INFO("IGMP V3 Report");
#ifdef AIR_SUPPORT_IGMPV3_AWARE
                if (ENABLE == ptr_igmp_snp->cfg_info.igmpv3_aware)
                {
                    /* Now calculate and check the checksum */
                    if (inet_chksum(ptr_igmp, ptr_pkt_info->payload_len))
                    {
                        IGMPSNP_STATS_INC(igmpsnoop.chkerr);
                        IGMP_SNP_LOG_ERROR("checksum error");
                        ptr_pkt_info->forward_bmp = 0;
                    }
                    else
                    {
                        rc =  _igmp_snp_packet_handleV3ReportMsg(ptr_pkt_info);
                        if (MW_E_BAD_PARAMETER == rc)
                        {
                            ptr_pkt_info->forward_bmp = 0;
                            IGMP_SNP_LOG_INFO("Invalid IGMPv3 Report, drop!");
                        }
                        else if (MW_E_OK != rc)
                        {
                            forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                        }

                    }
                }
                else
#endif
                {
                    /* IGMP v3 Report flooding to all ports, whatever router port exist or not. */
                    /* The port receive IGMP v3 Report will not become member port. */
                    forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                }

                break;

            default:
                IGMP_SNP_LOG_INFO("Unknown IGMP type=0x%x", IGMP_TYPE(ptr_igmp));
                /*rfc4541, 2.1.1, 3) The switch that supports IGMP snooping must flood all unrecognized IGMP messages to all other ports and
                 *                              must not attempt to make use use of any information beyond the end of the network layer header.
                 */
                forward_opt = IGMP_SNP_FORWARD_IN_VLAN;
                break;
        }

        if(IGMP_SNP_FORWARD_IN_VLAN == forward_opt)
        {
            UI8_T          vlan_count = 0, index = 0;
            UI16_T         ptr_vlan_id[MAX_PORT_NUM] = {0};

            _igmp_snp_packet_getIngressVlanInfo(ptr_pkt_info->ptr_pbuf, ptr_vlan_id, &vlan_count);
            if(0 != vlan_count)
            {
                for(index = 0; index < vlan_count; index++)
                {
                    ptr_pkt_info->forward_bmp |= igmp_snp_vlan_getMember(ptr_vlan_id[index]);
                }
            }

        }
    }

    if (IGMP_SNP_WDOG_KICK_NUM == ++_igmp_snp_process_cnt)
    {
        osal_wdog_kick();
        _igmp_snp_process_cnt = 0;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
igmp_snp_packet_reportSuppProcess(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr,
    UI8_T port_id,
    UI8_T time)
{
    I32_T  rc = MW_E_OK;
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;
    IGMP_SNP_LOG_DEBUG("vid=%d, gip=%s, port=%d", vlan_id, ip4addr_ntoa(ptr_ipaddr), port_id);

    ptr_grp = igmp_snp_searchGroupEntry(vlan_id, ptr_ipaddr);
    if (NULL == ptr_grp)
    {
        rc = igmp_snp_addGroupEntry(vlan_id, ptr_ipaddr, port_id, IGMP_SNP_RPT_SUPP_TMP_INTERVAL);
        if (MW_E_OK != rc)
        {
            return rc;
        }

        ptr_grp = igmp_snp_searchGroupEntry(vlan_id, ptr_ipaddr);
        if (NULL == ptr_grp)
        {
            return MW_E_OTHERS;
        }
    }
    ptr_grp->rpt_suppress_timer = (time * IGMP_SNP_TMR_RATE_S);
    IGMP_SNP_LOG_DEBUG("Set rpt_suppress_timer = %d", ptr_grp->rpt_suppress_timer);

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_snp_packet_input
 * PURPOSE:
 *      This API is for extern compnent to input a IGMP message.
 *
 * INPUT:
 *      ptr_pbuf          -- pointer to the pbuf containing the IGMP message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_packet_input(
    struct pbuf *ptr_pbuf)
{
    IGMP_SNP_PACKET_INFO_T igmp_snp_pkt_info;
    struct ip_hdr *iphdr;
    UI16_T              iphdr_hlen;
    struct igmp_msg     *ptr_igmp = ptr_pbuf->payload;
#ifdef IGMPSNP_DEBUG
    UI8_T               igmp_type;
#endif
    UI16_T              iphdr_total_len = 0;
    UI8_T               igmpv3_query = 0;
    IGMP_SNP_LIST_T    *ptr_igmp_snp = NULL;

#ifdef AIR_SUPPORT_LP
    UI8_T               rx_u_portId;
    AIR_PORT_BITMAP_T   lp_blocked_pbmp;

    lp_getBlockPbmp(lp_blocked_pbmp);
    rx_u_portId = ptr_pbuf->stag_hdr.rx_hdr.sp;
    if (AIR_PORT_CHK(lp_blocked_pbmp, rx_u_portId))/*port x is blocked*/
    {
        pbuf_free(ptr_pbuf);
        return MW_E_OP_INCOMPLETE;
    }
#endif /* AIR_SUPPORT_LP */

    ptr_igmp_snp = igmp_snp_getConfig();
    if (NULL == ptr_igmp_snp)
    {
        pbuf_free(ptr_pbuf);
        return MW_E_BAD_PARAMETER;
    }
    osapi_memset(&igmp_snp_pkt_info, 0, sizeof(IGMP_SNP_PACKET_INFO_T));

    /* identify the IP header */
    iphdr = (struct ip_hdr *)ptr_pbuf->payload;
    /* obtain IP header length in bytes */
    iphdr_hlen = IPH_HL_BYTES(iphdr);
    iphdr_total_len = ntohs(IPH_LEN(iphdr));
    ptr_igmp = (struct igmp_msg*)((UI8_T*)(ptr_pbuf->payload) + iphdr_hlen);
#ifdef IGMPSNP_DEBUG
    igmp_type = IGMP_TYPE(ptr_igmp);
#endif
    IGMP_SNP_LOG_DEBUG("IGMP type=0x%x", igmp_type);

    pbuf_remove_header(ptr_pbuf, iphdr_hlen); /* Move to payload, no check necessary. */
    if (IPH_PROTO(iphdr) == IGMP_SNP_IPPROTO_IGMP)
    {
        if ((IGMP_TYPE(ptr_igmp) == IGMP_MEMB_QUERY) &&
            ((iphdr_total_len - iphdr_hlen) > IGMP_MINLEN))
        {
            IGMP_SNP_LOG_DEBUG("Set igmpv3_query = 1");
            igmpv3_query = 1;
        }

        igmp_snp_pkt_info.ptr_iphdr = iphdr;
        igmp_snp_pkt_info.ptr_pbuf= ptr_pbuf;
#ifdef AIR_SUPPORT_IGMPV3_AWARE
        igmp_snp_pkt_info.payload_len = (iphdr_total_len - iphdr_hlen);
#endif
        _igmp_snp_packet_parsePacket(&igmp_snp_pkt_info, igmpv3_query);
    }
    else
    {
        IGMP_SNP_LOG_INFO("IPH_PROTO(iphdr) = %d, not IGMP", IPH_PROTO(iphdr));
#ifdef AIR_SUPPORT_PIM_AWARE
        if (IPH_PROTO(iphdr) == IGMP_SNP_IPPROTO_PIM)
        {
            if (ptr_igmp_snp->cfg_info.enable == TRUE)
            {
                UI16_T  ptr_vlan_id[MAX_PORT_NUM] = {0};
                UI16_T  vlan_id = IGMP_SNP_MIN_VID;
                UI8_T   port_id = ptr_pbuf->stag_hdr.rx_hdr.sp;
                UI8_T   index = 0, vlan_count;
                UI32_T  min_trunk_port = port_id;

#ifdef IGMP_SNP_MW_SUPPORT
                min_trunk_port = igmp_snp_lag_getMinMemberPortByPortId(port_id);
#endif
                _igmp_snp_packet_getIngressVlanInfo(ptr_pbuf, ptr_vlan_id, &vlan_count);
                if(0 != vlan_count)
                {
                    IGMP_SNP_LOG_INFO("Receive %s packet, add as mrouter port", (IPH_PROTO(iphdr) == IGMP_SNP_IPPROTO_PIM ) ? "PIM" : "UNKNOWN");
                    for(index = 0; index < vlan_count; index++)
                    {
                        vlan_id = ptr_vlan_id[index];
                        igmp_snp_port_addMrouterPort(vlan_id, min_trunk_port, IGMP_SNP_OTHER_QUERIER_PRESENT_INTERVAL);
                    }
                    igmp_snp_pkt_info.forward_bmp = 0; /* Drop the trapped PIM packet. */
                }

            }
            else
            {
                IGMP_SNP_LOG_DEBUG("IGMP SNP is not enabled, ignore this PIM packet.");
            }
        }
#endif
    }

    _igmp_snp_packet_resumePbufHeader(ptr_pbuf, iphdr_hlen);
    _igmp_snp_packet_send(ptr_pbuf, IGMP_TYPE(ptr_igmp), igmp_snp_pkt_info.forward_bmp);
    pbuf_free(ptr_pbuf);
    return MW_E_OK;
}


/* FUNCTION NAME:   igmp_snp_packet_initVariable
 * PURPOSE:
 *      This API is used to initialize IGMP snooping module variables.
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
igmp_snp_packet_initVariable(
    void)
{
    IP4_ADDR(&_igmp_snp_allsystems, 224, 0, 0, 1);
    IP4_ADDR(&_igmp_snp_allrouters, 224, 0, 0, 2);

    return MW_E_OK;
}
