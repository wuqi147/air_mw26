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

/* FILE NAME:  igmp_querier_util.h
 * PURPOSE:
 *      This file defines the data structure for IGMP querier utilities.
 *
 * NOTES:
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

#ifndef IGMP_QUERYER_UTILS_H
#define IGMP_QUERYER_UTILS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/ip_addr.h"
#include "mw_queue.h"

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef ATTRIBUTE_PACK
#define ATTRIBUTE_PACK __attribute__((packed))
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct IGMP_QUERIER_TCP_CALLBACK_MSG_S
{
    struct pbuf               *ptr_pbuf;
    ip4_addr_t                 src_addr;
    ip4_addr_t                 dest_addr;
} IGMP_QUERIER_TCP_CALLBACK_MSG_T;

typedef enum
{
    IGMP_QUERIER_VERSION_V3,                /* Ver. 3 */
    IGMP_QUERIER_VERSION_V2,                /* Ver. 2 */
}IGMP_QUERIER_VERSION_TYPE_T;

typedef struct IGMP_QUERIER_GENERAL_QUERY_S
{
    UI8_T                           startup_query_count;  /* startup query count */
    UI16_T                          querier_query_timer;  /* general query timer */
}ATTRIBUTE_PACK IGMP_QUERIER_GENERAL_QUERY_T;

typedef struct IGMP_QUERIER_GS_QUERY_LIST_S
{
    UI8_T                                     last_mem_query_count; /* RFC2236 8.9. last member query count */
    UI8_T                                     gs_query_timer;       /* RFC2236 8.8. last member query interval */
    ip4_addr_t                                group_addr;
    CSLIST_ENTRY(IGMP_QUERIER_GS_QUERY_LIST_S) next;
}ATTRIBUTE_PACK IGMP_QUERIER_GS_QUERY_LIST_T;

typedef struct IGMP_QUERIER_LIST_S
{
    UI16_T                                         vlan_id;            /* The VLAN ID of IGMP snooping querier */
    UI8_T                                          state;              /* The state of IGMP snooping querier */
    UI8_T                                          election;           /* Supported election of IGMP snooping querier */
    UI8_T                                          version;            /* Supported version of IGMP snooping querier */
    ip4_addr_t                                     source_address;     /* IGMP snooping querier's IP address */
    UI16_T                                         querier_expiry_timer;
    UI16_T                                         db_idx;
    IGMP_QUERIER_GENERAL_QUERY_T               general_query_cfg;
    CSLIST_HEAD(_igmp_snp_querier_gs_query_list, IGMP_QUERIER_GS_QUERY_LIST_S) gs_query_head;
    CSLIST_ENTRY(IGMP_QUERIER_LIST_S) next;
}ATTRIBUTE_PACK IGMP_QUERIER_LIST_T;

typedef struct IGMP_QUERIER_INFO_S
{
    UI16_T          vlan_id;          /* The VLAN ID of IGMP snooping querier */
    UI8_T           state;            /* The state of IGMP snooping querier */
    UI8_T           election;         /* The state of IGMP snooping querier election */
    UI8_T           version;          /* The version of IGMP snooping querier */
    MW_IPV4_T       address;          /* The source address of IGMP snooping querier */
} ATTRIBUTE_PACK IGMP_QUERIER_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    ip4_addr_t  group_address);

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
    UI16_T db_idx);

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
    UI8_T  version);

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
    UI8_T   *ptr_version);

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
    UI8_T   enable);

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
    UI8_T  *ptr_enable);

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
    UI8_T  enable);

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
    UI8_T  *ptr_enable);

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
    ip4_addr_t addr);

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
    ip4_addr_t *ptr_addr);

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
    UI16_T vlan_id);

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
    UI16_T vlan_id);

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
    ip4_addr_t src_addr);

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
    ip4_addr_t group_addr);

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
    ip4_addr_t group_ip);

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
igmp_querier_clearAllEntry();

#endif /* IGMP_QUERYER_UTILS_H */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
