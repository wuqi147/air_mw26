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

/* FILE NAME:  igmp_querier_utils.c
 * PURPOSE:
 *    This file contains the implementation of IGMP querier utilities.
 *
 * NOTES:
 *
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_utils.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_db.h"
#include "igmp_snoop_lag.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_timer.h"
#include "igmp_querier.h"
#include "igmp_querier_utils.h"
#include "igmp_querier_state_machine.h"
#include "lwip/tcpip.h"
#include "inet_utils.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip4.h"
#include "lwip/ip4_addr.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/igmp.h"
#include "air_stag.h"
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
static void
_igmp_querier_sendQueryCallback(
    void *ptr_data);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _igmp_querier_sendQueryCallback
 * PURPOSE:
 *      This API is for the querier to send IGMP packet.
 *
 * INPUT:
 *      ptr_data          --  the data containing pbuf, source address, destination address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      This api function is used to ensure thread safety when sending IGMP packets in IGMP_SNP task
 */
static void
_igmp_querier_sendQueryCallback(
    void *ptr_data)
{
    UI8_T                           tos = IGMP_QUERIER_TOS_DSCP;
#if IP_OPTIONS_SEND
    UI16_T                          router_alert[2];
#endif
    struct netif                    *ptr_xNetIf = NULL;
    IGMP_QUERIER_TCP_CALLBACK_MSG_T *ptr_msg = NULL;

    if(NULL == ptr_data)
    {
        return;
    }
    ptr_msg = (IGMP_QUERIER_TCP_CALLBACK_MSG_T *)ptr_data;
    if(NULL != ptr_msg->ptr_pbuf)
    {
        ptr_xNetIf = netif_find_default();
        if(NULL != ptr_xNetIf)
        {
#if IP_OPTIONS_SEND
            /* This is the "router alert" option */
            router_alert[0] = PP_HTONS(ROUTER_ALERT);
            router_alert[1] = 0x0000; /* Router shall examine packet */
            IGMP_STATS_INC(igmp.xmit);
            ip4_output_if_opt(ptr_msg->ptr_pbuf, &(ptr_msg->src_addr), &(ptr_msg->dest_addr), IGMP_TTL, tos, IP_PROTO_IGMP, ptr_xNetIf, router_alert, ROUTER_ALERTLEN);
#else
            ip4_output_if(ptr_msg->ptr_pbuf, &(ptr_msg->src_addr), &(ptr_msg->dest_addr), IGMP_TTL, tos, IP_PROTO_IGMP, ptr_xNetIf);
#endif /* IP_OPTIONS_SEND */
        IGMP_SNP_LOG_DEBUG("output packet, pbuf:0x%x, source addr:0x%x", (UI32_T)(ptr_msg->ptr_pbuf), (UI32_T) ip4_addr_get_u32(&(ptr_msg->src_addr)));
        }
        pbuf_free(ptr_msg->ptr_pbuf);
    }
    MW_FREE(ptr_msg);
    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_querier_sendQuery
 * PURPOSE:
 *      This API is used to send IGMP query message.
 *
 * INPUT:
 *      ptr_querier       --  the pointer of igmp querier entry
 *      group_address     --  the group address to send the query
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_sendQuery(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    ip4_addr_t  group_address)
{
    UI8_T                           index = 0 ,len = IGMP_MINLEN;
    UI8_T                           version = 0;
    UI8_T                           forward_opt = 0;
    err_t                           err = ERR_OK;
    ip4_addr_t                      general_query_addr;
    struct pbuf                     *ptr_pbuf  = NULL;
    struct igmp_msg                 *ptr_igmp = NULL;
    MW_PORT_BITMAP_T                portbmp = {0};
#ifdef AIR_SUPPORT_LP
    MW_PORT_BITMAP_T               lp_blocked_pbmp;
#endif
    IGMP_SNP_VLAN_CFG_T         *ptr_igmp_snp_vlan = NULL;
    struct IGMP_SNP_GROUP_LIST_S    *ptr_grp = NULL;
    IGMP_QUERIER_TCP_CALLBACK_MSG_T *ptr_cbk_msg = NULL;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
    IGMP_SNP_V3QUERY_MSG_T  *ptr_igmpv3 = NULL;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */

    if(FALSE == ip4_addr_ismulticast(&group_address))
    {
        MW_IPV4_T addr_u32 = ip4_addr_get_u32(&group_address);
        IGMP_SNP_LOG_ERROR("input address is not a multicast ip addrees, %d.%d.%d.%d",
                                                    (0xFF & ((UI8_T*)&addr_u32)[0]),
                                                    (0xFF & ((UI8_T*)&addr_u32)[1]),
                                                    (0xFF & ((UI8_T*)&addr_u32)[2]),
                                                    (0xFF & ((UI8_T*)&addr_u32)[3]));
        return MW_E_BAD_PARAMETER;
    }

    igmp_querier_GetVersion(ptr_querier->vlan_id, &version);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
    if(IGMP_QUERIER_VERSION_V3 == version)
    {
        len = sizeof(IGMP_SNP_V3QUERY_MSG_T);
        ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    }
    else
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
    {
        ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    }

    if (NULL != ptr_pbuf)
    {
        memset(ptr_pbuf->payload, 0, len);
        ptr_pbuf->stag_hdr.tx_hdr.mode               = STAG_HDR_MODE_INSERT;
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP;
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp  = 0;

        ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
        if(IGMP_SNP_VLAN_TAG == ptr_igmp_snp_vlan->vlan_mode)
        {
            ptr_pbuf->vlan_num = 1;
            ptr_pbuf->vlan_hdr.tpid = ETHTYPE_VLAN;
            ptr_pbuf->vlan_hdr.priority = 6;
            ptr_pbuf->vlan_hdr.vid  = ptr_querier->vlan_id;
        }

        IP4_ADDR(&general_query_addr, 224, 0, 0, 1);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
        if(IGMP_QUERIER_VERSION_V3 == version)
        {
            ptr_igmpv3 = (IGMP_SNP_V3QUERY_MSG_T  *)ptr_pbuf->payload;
            ptr_igmpv3->igmp_msgtype = IGMP_MEMB_QUERY;
            ptr_igmpv3->igmp_maxresp = IGMP_SNP_QUERY_RESP_TIME * 10;
            ptr_igmpv3->igmp_resv = 0;
            ptr_igmpv3->igmp_s = 0;
            ptr_igmpv3->igmp_qqic = 0;
            ptr_igmpv3->igmp_n_srcs = 0;
            if(TRUE == ip4_addr_cmp(&group_address, &general_query_addr))
            {
                /* general query */
                ptr_igmpv3->igmp_group_address = 0;
                forward_opt |= IGMP_SNP_FORWARD_IN_VLAN;
            }
            else
            {
                /* group specific query */
                ptr_igmpv3->igmp_group_address = ip4_addr_get_u32(&group_address);
                forward_opt |= IGMP_SNP_FORWARD_TO_GROUP;
            }
            ptr_igmpv3->igmp_checksum = 0;
            ptr_igmpv3->igmp_checksum = inet_chksum(ptr_igmpv3, len);
        }
        else
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
        {
            ptr_igmp = (struct igmp_msg *)ptr_pbuf->payload;
            ptr_igmp->igmp_msgtype = IGMP_MEMB_QUERY;
            /* RFC2236, maximum response time in units of 1/10 second */
            ptr_igmp->igmp_maxresp = IGMP_SNP_QUERY_RESP_TIME * 10;
            if(1 == ip4_addr_cmp(&group_address, &general_query_addr))
            {
                /* general query */
                ip4_addr_set_zero(&(ptr_igmp->igmp_group_address));
                forward_opt |= IGMP_SNP_FORWARD_IN_VLAN;
            }
            else
            {
                /* group specific query */
                ip4_addr_copy(ptr_igmp->igmp_group_address, group_address);
                forward_opt |= IGMP_SNP_FORWARD_TO_GROUP;
            }
            ptr_igmp->igmp_checksum = 0;
            ptr_igmp->igmp_checksum = inet_chksum(ptr_igmp, IGMP_MINLEN);
        }

        /* Send to all other ports in VLAN. */
        if(0 != (forward_opt & IGMP_SNP_FORWARD_IN_VLAN))
        {
            portbmp[0] = igmp_snp_vlan_getMember(ptr_querier->vlan_id);
        }
        else if(0 != (forward_opt & IGMP_SNP_FORWARD_TO_GROUP))
        {
            ip4_addr_t ip_addr;
            ip_addr.addr = ntohl(group_address.addr);
            ptr_grp = igmp_snp_searchGroupEntry(ptr_querier->vlan_id, &ip_addr);
            if(NULL != ptr_grp)
            {
                MW_PORT_BITMAP_COPY(portbmp, ptr_grp->portbmp);
            }
            else
            {
                IGMP_SNP_LOG_WARN("group entry not found!");
            }
        }
        /* Check trunk member */
        IGMP_SNP_LOG_DEBUG("Forward with trunk 0x%08x --> , forward_opt:%x", portbmp[0], forward_opt);
        portbmp[0] = igmp_snp_lag_updateTxBmpBySrcPort(portbmp[0], PLAT_CPU_PORT);
#ifdef AIR_SUPPORT_LP
        lp_getBlockPbmp(lp_blocked_pbmp);
        portbmp[0] &= (~lp_blocked_pbmp[0]);
#endif /* AIR_SUPPORT_LP */

        AIR_PORT_FOREACH(portbmp, index)
        {
            if(PLAT_CPU_PORT == index)
            {
                /* Skip cpu port */
                continue;
            }
            ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp |= (1U << index);
        }
        if((0 != forward_opt) && (0 != ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp))
        {
            IGMP_SNP_LOG_INFO("Send query to ports 0x%x, igmp version:%s, vid:%d, source ip addr:0x%x",
                          ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp,
                          (version ? "v2" : "v3"),
                          ptr_querier->vlan_id,
                          (UI32_T) ip4_addr_get_u32(&(ptr_querier->source_address)));

            osapi_calloc(sizeof(IGMP_QUERIER_TCP_CALLBACK_MSG_T), IGMP_SNP_MODULE_NAME, (void**)&ptr_cbk_msg);
            if(NULL == ptr_cbk_msg)
            {
                IGMP_SNP_LOG_ERROR("Error: Failed to allocated memory for ptr_cbk_msg!");
                pbuf_free(ptr_pbuf);
                return MW_E_OP_STOPPED;
            }
            ptr_cbk_msg->ptr_pbuf = ptr_pbuf;
            ip4_addr_copy(ptr_cbk_msg->dest_addr, group_address);
            /* Get IGMP_SNP querier source IP address */
            ip4_addr_copy(ptr_cbk_msg->src_addr, ptr_querier->source_address);
            IGMP_SNP_LOG_DEBUG("tcpip_callback, pbuf:0x%x", (UI32_T)ptr_pbuf);
            err = tcpip_callback(_igmp_querier_sendQueryCallback, (void *)ptr_cbk_msg);
            /* if ERR_OK, pbuf will be freed in function _igmp_querier_sendQueryCallback() */
            if(ERR_OK != err)
            {
                IGMP_SNP_LOG_WARN("Warning: call tcpip_callback() failed, not send packet, err:%d", err);
                pbuf_free(ptr_pbuf);
            }
        }
        else
        {
            pbuf_free(ptr_pbuf);
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_getVidByDbIdx
 * PURPOSE:
 *      This API is used to get VLAN ID by database entry index.
 *
 * INPUT:
 *      db_idx            --  database entry index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      vlan_id           --  VLAN ID
 *
 * NOTES:
 *      None
 */
UI16_T
igmp_querier_getVidByDbIdx(
    UI16_T db_idx)
{
    UI16_T                          vlan_id = IGMP_SNP_MIN_VID;
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if((NULL != ptr_igmp_snp) && (0 != db_idx))
    {
        CSLIST_FOREACH(ptr_querier, &(ptr_igmp_snp->querier_head), next)
        {
            if(ptr_querier->db_idx == db_idx)
            {
                vlan_id = ptr_querier->vlan_id;
                IGMP_SNP_LOG_DEBUG("db_idx:%d, vid:%d", db_idx, vlan_id);
                break;
            }
        }
    }
    return vlan_id;
}


/* FUNCTION NAME:   igmp_querier_SetVersion
 * PURPOSE:
 *      This API is used to set IGMP querier version.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      version           --  IGMP querier version
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_SetVersion(
    UI16_T vlan_id,
    UI8_T  version)
{
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if(NULL == ptr_querier)
    {
        return MW_E_NOT_INITED;
    }

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
    if (version != ptr_querier->version)
    {
        ptr_querier->version = version;
    }
#else
    ptr_querier->version = IGMP_QUERIER_VERSION_V2;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
    IGMP_SNP_LOG_DEBUG("querier version set, version:%s, vid:%d", (ptr_querier->version ? "v2" : "v3"), vlan_id);
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_GetVersion
 * PURPOSE:
 *      This API is used to set IGMP querier version.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      version           --  IGMP querier version
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_GetVersion(
    UI16_T  vlan_id,
    UI8_T   *ptr_version)
{
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    MW_CHECK_PTR(ptr_version);
    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    MW_CHECK_PTR(ptr_querier);

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
    *ptr_version = ptr_querier->version;
#else
    *ptr_version = IGMP_QUERIER_VERSION_V2;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_setAdminState
 * PURPOSE:
 *      This API is used to enable/disable IGMP querier admin state.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      enable            --  querier admin state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_setAdminState(
    UI16_T  vlan_id,
    UI8_T   enable)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if((NULL != ptr_querier) && (ptr_querier->state != enable))
    {
        if(ENABLE == enable)
        {
            ptr_querier->state = enable;
            rc = igmp_querier_isReady(vlan_id);
            if(MW_E_OK == rc)
            {
                igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_START_QUERIER, NULL);
            }
        }
        else
        {
            igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_DISABLE, NULL);
        }
    }
    IGMP_SNP_LOG_DEBUG("querier enable set, state:%x, vid:%d", enable, vlan_id);
    return rc;
}

/* FUNCTION NAME:   igmp_querier_getAdminState
 * PURPOSE:
 *      This API is used to get IGMP querier admin state.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      ptr_enable        --  pointer of IGMP querier admin state
 *
 * OUTPUT:
 *      IGMP querier admin state
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_getAdminState(
    UI16_T vlan_id,
    UI8_T  *ptr_enable)
{
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    MW_CHECK_PTR(ptr_enable);
    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    MW_CHECK_PTR(ptr_querier);
    if(IGMP_QUERIER_STATE_DISABLED == ptr_querier->state)
    {
        *ptr_enable = DISABLE;
    }
    else
    {
        *ptr_enable = ENABLE;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_setElectMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP querier election mode.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      enable            --  querier election mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_setElectMode(
    UI16_T vlan_id,
    UI8_T  enable)
{
    MW_ERROR_NO_T                  rc = MW_E_OK;
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if(NULL == ptr_querier)
    {
        return MW_E_NOT_INITED;
    }
    if (enable != ptr_querier->election)
    {
        ptr_querier->election = enable;
    }
    IGMP_SNP_LOG_DEBUG("querier election set, state:%x, vid:%d", ptr_querier->election, vlan_id);
    return rc;
}

/* FUNCTION NAME:   igmp_querier_getElectMode
 * PURPOSE:
 *      This API is used to get IGMP querier election mode.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      ptr_enable        --  pointer of IGMP querier election mode
 *
 * OUTPUT:
 *      IGMP querier election mode
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_getElectMode(
    UI16_T vlan_id,
    UI8_T  *ptr_enable)
{
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    MW_CHECK_PTR(ptr_enable);
    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    MW_CHECK_PTR(ptr_querier);
    *ptr_enable = ptr_querier->election;

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_setQuerierAddr
 * PURPOSE:
 *      This API is used to set IGMP querier IP address.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      addr              --  IGMP querier IP address to set
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_setQuerierAddr(
    UI16_T     vlan_id,
    ip4_addr_t addr)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    /* Valid IP address except 0.x.x.x and 127.x.x.x */
    if((0 == ip4_addr1(&addr)) || (127 == ip4_addr1(&addr)) || (NULL == ptr_querier))
    {
        IGMP_SNP_LOG_WARN("Warning: In valid IP address:%x!", (UI32_T) ip4_addr_get_u32(&addr));
        return MW_E_NOT_INITED;
    }
    ip4_addr_copy(ptr_querier->source_address, addr);
    IGMP_SNP_LOG_DEBUG("set querier address:%x, vid:%d", (UI32_T) ip4_addr_get_u32(&((ptr_querier->source_address))), vlan_id);
    rc = igmp_querier_isReady(vlan_id);
    if(MW_E_OK == rc)
    {
        igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_START_QUERIER, NULL);
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_getQuerierAddr
 * PURPOSE:
 *      This API is used to get IGMP querier IP address.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      ptr_addr          --  pointer of IGMP querier IP address
 *
 * OUTPUT:
 *      IGMP querier IP address
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_getQuerierAddr(
    UI16_T      vlan_id,
    ip4_addr_t *ptr_addr)
{
    struct IGMP_QUERIER_LIST_S *ptr_querier = NULL;

    MW_CHECK_PTR(ptr_addr);
    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    MW_CHECK_PTR(ptr_querier);
    ip4_addr_copy(*ptr_addr, ptr_querier->source_address);
    IGMP_SNP_LOG_DEBUG("get querier addr:%x, vid:%d", (UI32_T) ip4_addr_get_u32(&(ptr_querier->source_address)),vlan_id);
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_querier_getOperQuerierInfo
 * PURPOSE:
 *      This API is used to look for IGMP querier.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer of IGMP querier.
 *
 * NOTES:
 *      None
 */
struct IGMP_QUERIER_LIST_S *
igmp_querier_getOperQuerierInfo(
    UI16_T vlan_id)
{
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if((NULL != ptr_igmp_snp) && ((IGMP_SNP_MIN_VID < vlan_id) && (IGMP_SNP_MAX_VID > vlan_id)))
    {
        CSLIST_FOREACH(ptr_querier, &(ptr_igmp_snp->querier_head), next)
        {
            if(ptr_querier->vlan_id == vlan_id)
            {
                break;
            }
        }
    }
    return ptr_querier;
}

/* FUNCTION NAME:   igmp_querier_isReady
 * PURPOSE:
 *      This API is used to get IGMP querier is not ready.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      The return value will be MW_E_OK if the querier is readied,
 *      otherwise return MW_E_NOT_INITED.
 */
MW_ERROR_NO_T
igmp_querier_isReady(
    UI16_T vlan_id)
{
    UI8_T                           igmp_snp_mode = DISABLE;
    UI8_T                           querier_mode = DISABLE;
    ip4_addr_t                      querier_addr;
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if(NULL == ptr_querier)
    {
        return MW_E_NOT_INITED;
    }
    ptr_igmp_snp = igmp_snp_getConfig();
    if(NULL != ptr_igmp_snp)
    {
        igmp_snp_mode = ptr_igmp_snp->cfg_info.enable;
    }
    igmp_querier_getAdminState(vlan_id, &querier_mode);
    memset(&querier_addr, 0, sizeof(ip4_addr_t));
    igmp_querier_getQuerierAddr(vlan_id, &querier_addr);
    IGMP_SNP_LOG_DEBUG("igmp_snp_mode:%x, querier_mode:%x, querier addr:%x",
                                                igmp_snp_mode, querier_mode, (UI32_T) ip4_addr_get_u32(&querier_addr));
    if((ENABLE == igmp_snp_mode) && (ENABLE == querier_mode) && ((FALSE == ip4_addr_isany_val(querier_addr))))
    {
        return MW_E_OK;
    }
    return MW_E_NOT_INITED;
}

/* FUNCTION NAME:   igmp_querier_handleQueryMsg
 * PURPOSE:
 *      This API is used to querier process a IGMP query message.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      src_addr          --  the source address carried in the IP packet
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_handleQueryMsg(
    UI16_T     vlan_id,
    ip4_addr_t src_addr)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    rc = igmp_querier_isReady(vlan_id);
    if(MW_E_OK == rc)
    {
        ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
        if(DISABLE == ptr_querier->election)
        {
            igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_TO_NON_QUERIER, NULL);
        }
        else
        {
            if(htonl(ip4_addr_get_u32(&src_addr)) <= htonl(ip4_addr_get_u32(&(ptr_querier->source_address))))
            {
                igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_TO_NON_QUERIER, NULL);
            }
        }
    }
    IGMP_SNP_LOG_DEBUG("querier recv igmp query packet, vid:%d, rc:%d", vlan_id, rc);
    return rc;
}

/* FUNCTION NAME:   igmp_querier_handleLeaveMsg
 * PURPOSE:
 *      This API is used to querier process a IGMP leave message.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      group_addr        --  the group address carried in the IGMP leave packet
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_handleLeaveMsg(
    UI16_T     vlan_id,
    ip4_addr_t group_addr)
{
    MW_ERROR_NO_T                           rc = MW_E_OK;
    struct IGMP_QUERIER_LIST_S          *ptr_querier = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;

    rc = igmp_querier_isReady(vlan_id);
    if(MW_E_OK == rc)
    {
        ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
        if(IGMP_QUERIER_STATE_QUERIER == ptr_querier->state)
        {
            ptr_gs_query = igmp_querier_searchGsQueryList(vlan_id, group_addr);
            /* If it is existed, don't process it... just ignore */
            if(NULL == ptr_gs_query)
            {
                osapi_calloc(sizeof(struct IGMP_QUERIER_GS_QUERY_LIST_S), IGMP_SNP_MODULE_NAME, (void**)&ptr_gs_query);
                if(NULL != ptr_gs_query)
                {
                    ip4_addr_copy(ptr_gs_query->group_addr, group_addr);
                    ptr_gs_query->last_mem_query_count = IGMP_SNP_ROBUSTNESS;
                    CSLIST_INSERT_HEAD(&(ptr_querier->gs_query_head), ptr_gs_query, next);
                    igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_GST, ptr_gs_query);
                }
            }
        }
    }
    IGMP_SNP_LOG_DEBUG("querier recv igmp leave packet, vid:%d, rc:%d", vlan_id, rc);
    return rc;
}

/* FUNCTION NAME:   igmp_querier_searchGsQueryList
 * PURPOSE:
 *      This API is used to look for IGMP querier group specific query list entries.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *      group_ip          --  group addres of group specific query
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of group specific query entry.
 *
 * NOTES:
 *      None
 */
struct IGMP_QUERIER_GS_QUERY_LIST_S*
igmp_querier_searchGsQueryList(
    UI16_T     vlan_id,
    ip4_addr_t group_ip)
{
    struct IGMP_QUERIER_LIST_S          *ptr_querier = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if(NULL != ptr_querier)
    {
        CSLIST_FOREACH(ptr_gs_query, &(ptr_querier->gs_query_head), next)
        {
            if (TRUE == (ip4_addr_cmp(&(group_ip), &(ptr_gs_query->group_addr))))
            {
                break;
            }
        }
    }
    IGMP_SNP_LOG_DEBUG("gs query entry:0x%x", (UI32_T)ptr_gs_query);
    return ptr_gs_query;
}

/* FUNCTION NAME:   igmp_querier_clearAllEntry
 * PURPOSE:
 *      This API is used to clear IGMP querier entry.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_clearAllEntry()
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S      *ptr_querier = NULL;
    struct IGMP_QUERIER_LIST_S      *ptr_querier_next = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if(NULL != ptr_igmp_snp)
    {
        CSLIST_FOREACH_SAFE(ptr_querier, ptr_querier_next, &(ptr_igmp_snp->querier_head), next)
        {
            if (IGMP_SNP_MIN_VID != ptr_querier->vlan_id)
            {
                igmp_querier_deInit(ptr_querier->vlan_id);
            }
        }
        CSLIST_INIT(&(ptr_igmp_snp->querier_head));
    }
    rc = igmp_snp_db_queue_sendWithRsp(M_UNSUBSCRIBE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
    rc |= igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);

    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
