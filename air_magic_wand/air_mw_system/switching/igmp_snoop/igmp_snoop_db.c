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

/* FILE NAME:  igmp_snoop_db.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop database.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_db.h"
#include "igmp_snoop_utils.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_mode.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_lag.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_port.h"
#include "igmp_snoop_timer.h"
#include "syncd_api_lag.h"
#include "mac_utils.h"
#ifdef AIR_SUPPORT_LACP
#include "air_lag.h"
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier_db.h"
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

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
#ifdef IGMP_SNP_MW_SUPPORT
static MW_ERROR_NO_T
_igmp_snp_db_handleIgmpSnpInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handleL2mcEntryMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handlePortCfgInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handleVlanCfgInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handleTrunkPortMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handleVlanEntryMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handlePortOperInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_igmp_snp_db_handleLagMemberChangeEvent(
    UI32_T  lag_id,
    UI32_T  old_member);

static MW_ERROR_NO_T
_igmp_snp_db_parseMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);
#endif /* IGMP_SNP_MW_SUPPORT */

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef IGMP_SNP_MW_SUPPORT
static MW_IGMP_SNP_DB_IDX_MAP_T _igmp_snp_db_idx;
#endif

/* LOCAL SUBPROGRAM BODIES
 */
#ifdef IGMP_SNP_MW_SUPPORT
static MW_ERROR_NO_T
_igmp_snp_db_handleIgmpSnpInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T   state = FALSE;

    switch(request.f_idx)
    {
        case IGMP_SNP_ENABLE:
        case IGMP_SNP_RPT_SUPPRESS:
        case IGMP_SNP_FAST_LEAVE:
            osapi_memcpy(&state, ptr_data, sizeof(UI8_T));
            if ((TRUE == state) || (FALSE == state))
            {
                if(IGMP_SNP_ENABLE == request.f_idx)
                {
                    /* igmp snoop admin enable/disable */
                    igmp_snp_setAdminMode(state);
                }
                else if(IGMP_SNP_RPT_SUPPRESS == request.f_idx)
                {
                    /* igmp snoop report suppression enable/disable */
                    igmp_snp_setRptSuppressMode(state);
                }
                else
                {
                    /* igmp snoop fast leave enable/disable */
                    igmp_snp_setFastLeaveMode(state);
                }
            }
            else
            {
                IGMP_SNP_LOG_ERROR("Field: %d, Invalid parameter - %d", request.f_idx, state);
            }
            break;

        default:
            IGMP_SNP_LOG_ERROR("recv unknown field: [%d]", request.f_idx);
            break;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handleL2mcEntryMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI32_T                         port_id = 0;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
    IGMP_SNP_L2_MC_ENTRY_T         *ptr_l2mc_entry = NULL;
    IGMP_SNP_LIST_T                *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    if(M_DELETE == method)
    {
        if(DB_ALL_ENTRIES == request.e_idx)
        {
            CSLIST_FOREACH_SAFE(ptr_grp, ptr_grp_next, &ptr_igmp_snp->group_head, next)
            {
#ifdef IGMP_SNP_MW_SUPPORT
                igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
#endif
                CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
                MW_FREE(ptr_grp);
            }
            CSLIST_INIT(&ptr_igmp_snp->group_head);
            IGMP_SNP_LOG_DEBUG("Clear all group entry!");
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
            osapi_memset(ptr_igmp_snp->port_group_num, 0, sizeof(ptr_igmp_snp->port_group_num));
#endif
        }
        else
        {
            ptr_l2mc_entry = (IGMP_SNP_L2_MC_ENTRY_T *)ptr_data;
            CSLIST_FOREACH(ptr_grp, &ptr_igmp_snp->group_head, next)
            {
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
                if ((ptr_l2mc_entry->vid == ptr_grp->vlan_id) &&
                    (0 == macAddrCmp(ptr_l2mc_entry->mac_addr, ptr_grp->mac_address)))
#else
                ip4_addr_t    ip_addr;
                ip4_addr_set_u32(&ip_addr, ptr_l2mc_entry->ip_addr);
                if ((ptr_l2mc_entry->vid == ptr_grp->vlan_id) &&
                    (TRUE == ip4_addr_cmp(&ip_addr, &(ptr_grp->group_address))))
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
                {
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                    MW_PORT_FOREACH(ptr_grp->portbmp, port_id)
                    {
                        if(PLAT_CPU_PORT == port_id)
                        {
                            continue;
                        }
                        if(0 < ptr_igmp_snp->port_group_num[(port_id - 1)])
                        {
                            ptr_igmp_snp->port_group_num[(port_id - 1)]--;
                        }
                    }
#endif /* IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT */
#ifdef IGMP_SNP_MW_SUPPORT
                    IGMP_SNP_LOG_DEBUG("delete db_idx %d", ptr_grp->db_idx);
                    igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
#endif
                    CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
                    MW_FREE(ptr_grp);
                    break;
                }
            }
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handlePortCfgInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T                       i = 0, entry_num = 0, router_state = 0;
    UI32_T                      port_id = 0;
    UI32_T                      new_router = 0, add_member = 0, del_member = 0;
    IGMP_SNP_LIST_T             *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                                request.f_idx,
                                                request.e_idx,
                                                data_size,
                                                method);

    if(PORT_IGMP_ROUTER == request.f_idx)
    {
        new_router = ptr_igmp_snp->cfg_info.static_router[0];
        entry_num = (((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size) ? PLAT_MAX_PORT_NUM : 1);
        for(i = 0; i < entry_num; i++)
        {
            port_id = ((DB_ALL_ENTRIES == request.e_idx) ? (i + 1): request.e_idx);
            osapi_memcpy(&router_state, ((UI8_T*)ptr_data + i), sizeof(UI8_T));
            /* Bit 0 in the IGMP ROUTER bitmap represents port 1 not port 0, refer to DB table PORT_CFG_INFO, PORT_IGMP_ROUTER */
            if(0 != router_state)
            {
                new_router |= (1 << (port_id - 1));
            }
            else
            {
                new_router &= (~(1 << (port_id - 1)));
            }

            add_member = new_router & (~ptr_igmp_snp->cfg_info.static_router[0]);
            del_member = ptr_igmp_snp->cfg_info.static_router[0] & (~new_router);
        }

        ptr_igmp_snp->cfg_info.static_router[0] = new_router;
        IGMP_SNP_LOG_DEBUG("Set static_router, add_member = 0x%x, del_member = 0x%x, static router bmp = 0x%x",
                                                    add_member,
                                                    del_member,
                                                    ptr_igmp_snp->cfg_info.static_router[0]);

        if((0 != add_member) || (0 != del_member))
        {
            igmp_snp_port_addStaticMrouterPort(add_member, del_member);
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handleVlanCfgInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    DB_VLAN_CFG_INFO_T          *ptr_vlan_cfg = NULL;
    IGMP_SNP_VLAN_CFG_T         *ptr_igmp_snp_vlan = NULL;

    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp_vlan);
    ptr_vlan_cfg = (DB_VLAN_CFG_INFO_T *)ptr_data;
    MW_CHECK_PTR(ptr_vlan_cfg);
    if(VLAN_STATE_ENABLE == ptr_vlan_cfg->enable_port_b)
    {
        ptr_igmp_snp_vlan->vlan_mode = IGMP_SNP_VLAN_PORT;
    }
    else if(VLAN_STATE_ENABLE == ptr_vlan_cfg->enable_8021q_b)
    {
        ptr_igmp_snp_vlan->vlan_mode = IGMP_SNP_VLAN_TAG;
    }
    else if(VLAN_STATE_ENABLE == ptr_vlan_cfg->enable_mtu)
    {
        ptr_igmp_snp_vlan->vlan_mode = IGMP_SNP_VLAN_MTU;
    }
    IGMP_SNP_LOG_DEBUG("Change VLAN mode = %d, reset IGMP Snp", ptr_igmp_snp_vlan->vlan_mode);
    osapi_memset(ptr_igmp_snp_vlan->vlan_entry, 0, (sizeof(IGMP_SNP_VLAN_ENTRY_T) * IGMP_SNP_MAX_VLAN_ENTRY_NUM));
    if(M_UPDATE == method)
    {
        igmp_snp_clearAllEntry();
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handleTrunkPortMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T                       i = 0, entry_num = 0;
    UI16_T                      db_idx = 0;
    UI32_T                      port_bitmap = 0, old_member = 0;
    UI8_T                       *ptr_trunk;
    DB_TRUNK_PORT_T             trunk_info;
    IGMP_SNP_LAG_INFO_T         *ptr_lag_info = NULL;

    ptr_lag_info = igmp_snp_lag_getConfig();
    MW_CHECK_PTR(ptr_lag_info);
    IGMP_SNP_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                                request.f_idx,
                                                request.e_idx,
                                                data_size,
                                                method);

    if(TRUNK_MEMBERS == request.f_idx)
    {
        ptr_trunk = (UI8_T *)ptr_data;
        entry_num = (((sizeof(DB_TRUNK_PORT_T) * MAX_TRUNK_NUM) == data_size) ? MAX_TRUNK_NUM : 1);
        for(i = 0; i < entry_num; i++)
        {
            db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (i + 1): request.e_idx);
            memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            port_bitmap = trunk_info.members.member_bmp;
            old_member = (ptr_lag_info->lag[(db_idx - 1)]).member_bmp;
            osapi_memcpy((void *)&(ptr_lag_info->lag[(db_idx - 1)]), (void *)&(trunk_info.members), sizeof(TRUNK_MEMBER_INFO_T));

            if(((0 != port_bitmap) && (LAG_MODE_STATIC == trunk_info.members.mode)) ||
               ((0 == port_bitmap) && (LAG_MODE_NONE == trunk_info.members.mode)))
            {
                _igmp_snp_db_handleLagMemberChangeEvent(db_idx, old_member);
            }
        }
    }

    return MW_E_OK;
}


static MW_ERROR_NO_T
_igmp_snp_db_handleVlanEntryMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI32_T                      i = 0;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    BOOL_T                      vlan_is_update = FALSE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
    UI16_T                      db_idx = 0;
    UI16_T                      add_vid = IGMP_SNP_MIN_VID;
    UI16_T                      vlan_deled_vid = IGMP_SNP_MIN_VID; /* the vid of the deleted vlan */
    UI16_T                      port_deled_vid = IGMP_SNP_MIN_VID; /* the vid of the vlan which port has been deleted */
    UI32_T                      deled_port_member = 0;
    UI32_T                      add_member = 0;
    IGMP_SNP_VLAN_CFG_T         *ptr_igmp_snp_vlan = NULL;
    IGMP_SNP_LIST_T             *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp_vlan);
    IGMP_SNP_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                                request.f_idx,
                                                request.e_idx,
                                                data_size,
                                                method);

    if((M_GET == method) || (M_UPDATE == method) || (M_CREATE == method) || (M_RESPONSE == method))
    {
        if ((VLAN_ENTRY_SIZE * MAX_VLAN_ENTRY_NUM) == data_size)
        {
            /* ALL VLAN entries */
            DB_VLAN_ENTRY_T *ptr_vlan_entry = (DB_VLAN_ENTRY_T *)ptr_data;

            for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
            {
                if(FALSE == VLAN_IS_VID_VALID(ptr_vlan_entry->vlan_id[i]))
                {
                    IGMP_SNP_LOG_DEBUG("Invalid VID: [%d], Do not handle", ptr_vlan_entry->vlan_id[i]);
                    continue;
                }

                if(IGMP_SNP_VLAN_TAG == ptr_igmp_snp_vlan->vlan_mode)
                {
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                    vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                    if(ptr_vlan_entry->vlan_id[i] != ptr_igmp_snp_vlan->vlan_entry[i].vlan_id)
                    {
                        add_vid = ptr_vlan_entry->vlan_id[i];
                    }
                    else if(0 == deled_port_member)
                    {
                        deled_port_member = ((ptr_igmp_snp_vlan->vlan_entry[i].tagged_member | ptr_igmp_snp_vlan->vlan_entry[i].untagged_member) &
                                                (~(ptr_vlan_entry->tagged_member[i] | ptr_vlan_entry->untagged_member[i])));
                        port_deled_vid = ((0 == deled_port_member) ? IGMP_SNP_MIN_VID : ptr_vlan_entry->vlan_id[i]);
                    }
                    ptr_igmp_snp_vlan->vlan_entry[i].vlan_id = ptr_vlan_entry->vlan_id[i];
                    ptr_igmp_snp_vlan->vlan_entry[i].tagged_member = ptr_vlan_entry->tagged_member[i];
                    ptr_igmp_snp_vlan->vlan_entry[i].untagged_member = ptr_vlan_entry->untagged_member[i];

                    IGMP_SNP_LOG_DEBUG("Set vlan_entry.vlan_id=%d,tag_mbr=0x%x, untag_mbr=0x%x",
                                                ptr_vlan_entry->vlan_id[i],
                                                ptr_vlan_entry->tagged_member[i],
                                                ptr_vlan_entry->untagged_member[i]);
                }
                else
                {
                    /* Because of patch [ENBCPM-596], port base vlan need check port member */
                    if(0 != ptr_vlan_entry->port_member[i])
                    {
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                        vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                        if(ptr_vlan_entry->vlan_id[i] != ptr_igmp_snp_vlan->vlan_entry[i].vlan_id)
                        {
                            add_vid = ptr_vlan_entry->vlan_id[i];
                        }
                        else if(0 == deled_port_member)
                        {
                            deled_port_member = ptr_igmp_snp_vlan->vlan_entry[i].port_member & (~(ptr_vlan_entry->port_member[i]));
                            port_deled_vid = ((0 == deled_port_member) ? IGMP_SNP_MIN_VID : ptr_vlan_entry->vlan_id[i]);
                        }

                        ptr_igmp_snp_vlan->vlan_entry[i].vlan_id = ptr_vlan_entry->vlan_id[i];
                        IGMP_SNP_LOG_DEBUG("Set vlan_entry.vlan_id=%d, port_mbr=0x%x",
                                                ptr_vlan_entry->vlan_id[i],
                                                ptr_vlan_entry->port_member[i]);
                    }
                    else
                    {
                        if(IGMP_SNP_MIN_VID != ptr_igmp_snp_vlan->vlan_entry[i].vlan_id)
                        {
                            vlan_deled_vid = ptr_igmp_snp_vlan->vlan_entry[i].vlan_id;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                            vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                            IGMP_SNP_LOG_DEBUG("Delete vlan_entry.vlan_id=%d, port_mbr=0x%x",
                                                ptr_vlan_entry->vlan_id[i],
                                                ptr_vlan_entry->port_member[i]);
                        }
                        ptr_igmp_snp_vlan->vlan_entry[i].vlan_id = IGMP_SNP_MIN_VID;
                    }
                    ptr_igmp_snp_vlan->vlan_entry[i].port_member = ptr_vlan_entry->port_member[i];
                }
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                /* Notify the querier that VLAN has changed */
                if(TRUE == vlan_is_update)
                {
                    igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, (i + 1), &(ptr_igmp_snp_vlan->vlan_entry[i].vlan_id), IGMP_SNP_SEMA_DELAY);
                    vlan_is_update = FALSE;
                }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
            }
        }
        else if (VLAN_ENTRY_SIZE == data_size)
        {
            /* Single VLAN entry */
            db_idx = request.e_idx;
            VLAN_ENTRY_INFO_T *vlan_entry = (VLAN_ENTRY_INFO_T *)ptr_data;
            if(IGMP_SNP_VLAN_TAG == ptr_igmp_snp_vlan->vlan_mode)
            {
                /* Before VLAN change reset router port of specific VID */
                igmp_snp_port_delStaticMrouterPort(vlan_entry->vlan_id);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                if(vlan_entry->vlan_id != ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id)
                {
                    add_vid = vlan_entry->vlan_id;
                }
                else if(0 == deled_port_member)
                {
                    deled_port_member = (ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].tagged_member | ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].untagged_member) &
                                        (~(vlan_entry->tagged_member | vlan_entry->untagged_member));
                    port_deled_vid = ((0 == deled_port_member) ? IGMP_SNP_MIN_VID : vlan_entry->vlan_id);
                }
                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id = vlan_entry->vlan_id;
                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].tagged_member = vlan_entry->tagged_member;
                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].untagged_member = vlan_entry->untagged_member;
                IGMP_SNP_LOG_DEBUG("Set vlan_entry.vlan_id=%d, tag_mbr=0x%x, untag_mbr=0x%x",
                                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id,
                                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].tagged_member,
                                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].untagged_member);

                /* Update reouter port of specific VID */
                add_member = (vlan_entry->tagged_member | vlan_entry->untagged_member);
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {

                    if((0 != (add_member & (0x01 << (i + 1)))) && (0 != (ptr_igmp_snp->cfg_info.static_router[0] & (0x01 << i))))
                    {
                        igmp_snp_port_addMrouterPort(vlan_entry->vlan_id, (i + 1), IGMP_SNP_STATIC_MROUTER);
                    }
                }
            }
            else
            {
                /* Before VLAN change reset static router */
                igmp_snp_port_addStaticMrouterPort(0, ptr_igmp_snp->cfg_info.static_router[0]);
                /* Because of patch [ENBCPM-596], port base vlan need check port member */
                if(0 != vlan_entry->port_member)
                {
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                    vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                    if(vlan_entry->vlan_id != ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id)
                    {
                        add_vid = vlan_entry->vlan_id;
                    }
                    else if(0 == deled_port_member)
                    {
                        deled_port_member = (ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].port_member) & (~(vlan_entry->port_member));
                        port_deled_vid = ((0 == deled_port_member) ? IGMP_SNP_MIN_VID : vlan_entry->vlan_id);
                    }
                    ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id = vlan_entry->vlan_id;

                    IGMP_SNP_LOG_DEBUG("Set vlan_entry.vlan_id=%d, port_mbr=0x%x",
                                vlan_entry->vlan_id,
                                vlan_entry->port_member);
                }
                else
                {
                    if(IGMP_SNP_MIN_VID != ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id)
                    {
                        vlan_deled_vid = ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                        vlan_is_update = TRUE;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                        IGMP_SNP_LOG_DEBUG("Delete vlan_entry.vlan_id=%d, port_mbr=0x%x",
                                    vlan_entry->vlan_id,
                                    vlan_entry->port_member);
                    }
                    ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id = IGMP_SNP_MIN_VID;
                }
                ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].port_member = vlan_entry->port_member;

                /* Add static router to new VLAN */
                igmp_snp_port_addStaticMrouterPort(ptr_igmp_snp->cfg_info.static_router[0], 0);
            }
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
            /* Notify the querier that VLAN has changed */
            if(TRUE == vlan_is_update)
            {
                igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, db_idx, &(ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id), IGMP_SNP_SEMA_DELAY);
                vlan_is_update = FALSE;
            }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
        }
    }
    else if(M_DELETE == method)
    {
        db_idx = request.e_idx;
        /* Before VLAN change reset router port of specific VID */
        if(IGMP_SNP_MIN_VID != ptr_igmp_snp_vlan->vlan_entry[db_idx- 1].vlan_id)
        {
            vlan_deled_vid = ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id;
            igmp_snp_port_delStaticMrouterPort(ptr_igmp_snp_vlan->vlan_entry[db_idx - 1].vlan_id);
            osapi_memset(&ptr_igmp_snp_vlan->vlan_entry[db_idx- 1], 0, sizeof(IGMP_SNP_VLAN_ENTRY_T));
            IGMP_SNP_LOG_DEBUG("Clear vlan_entry[%d]", db_idx - 1);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
            igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, db_idx, &(ptr_igmp_snp_vlan->vlan_entry[db_idx - 1]), IGMP_SNP_SEMA_DELAY);
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
        }
    }

    /* If a vlan is deleted or a port removed from a vlan, the group entry and mrouter need update */
    if((IGMP_SNP_MIN_VID != vlan_deled_vid) || ((0 != deled_port_member) && (IGMP_SNP_MIN_VID != port_deled_vid)))
    {
        UI32_T                         del_port = 0;
        MW_PORT_BITMAP_T               port_bmp = {0};
        struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
        struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
        struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
        struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr_next = NULL;

        IGMP_SNP_LOG_DEBUG("add_vid:%d, vlan_deled_vid:%d, port_deled_vid:%d, deled_port_member:0x%08x, VLAN mode:%d",
                        add_vid,
                        vlan_deled_vid,
                        port_deled_vid,
                        deled_port_member,
                        ptr_igmp_snp_vlan->vlan_mode);

        /* Update group entry */
        CSLIST_FOREACH_SAFE(ptr_grp,ptr_grp_next, &ptr_igmp_snp->group_head, next)
        {
            if((IGMP_SNP_MIN_VID != vlan_deled_vid) && (vlan_deled_vid == ptr_grp->vlan_id))
            {
                /* If a vlan is deleted, delete the group entry under this vlan */
                MW_PORT_FOREACH(ptr_grp->portbmp, del_port)
                {
                    if(PLAT_CPU_PORT == del_port)
                    {
                        continue;
                    }
                    MW_PORT_DEL(ptr_grp->portbmp, del_port);
                    ptr_grp->timer[(del_port-1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                    ptr_igmp_snp->port_group_num[(del_port-1)]--;
#endif /* IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT */
                }
                igmp_snp_delGroupEntry((void **)&ptr_grp);
            }
            else if((IGMP_SNP_MIN_VID != port_deled_vid) && (port_deled_vid == ptr_grp->vlan_id) && (0 != (ptr_grp->portbmp[0] & deled_port_member)))
            {
                /* If a port removed from the vlan, delete the group entry under this port */
                osapi_memcpy(&(port_bmp[0]), &(deled_port_member), sizeof(UI32_T));
                MW_PORT_FOREACH(port_bmp, del_port)
                {
                    if((TRUE == MW_PORT_CHK(ptr_grp->portbmp, del_port)) && (0 != del_port))
                    {
                        MW_PORT_DEL(ptr_grp->portbmp, del_port);
                        ptr_grp->timer[(del_port-1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                        ptr_igmp_snp->port_group_num[(del_port-1)]--;
#endif
                    }
                }

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
        }
        /* Update mrouter entry */
        CSLIST_FOREACH_SAFE(ptr_mrtr, ptr_mrtr_next, &ptr_igmp_snp->mrouter_head, next)
        {
            /* If a vlan is deleted, delete the mrouter under this vlan */
            if((IGMP_SNP_MIN_VID != vlan_deled_vid) && (vlan_deled_vid == ptr_mrtr->vlan_id))
            {
                MW_PORT_FOREACH(ptr_mrtr->portbmp, del_port)
                {
                    igmp_snp_port_delMrouterPort((void **)&ptr_mrtr, del_port);
                }
            }
            else if((IGMP_SNP_MIN_VID != port_deled_vid) && (port_deled_vid == ptr_mrtr->vlan_id) && (0 != (ptr_mrtr->portbmp[0] & deled_port_member)))
            {
                /* If a port removed from the vlan, delete the mrouter under this port */
                osapi_memcpy(&(port_bmp[0]), &(deled_port_member), sizeof(UI32_T));
                MW_PORT_FOREACH(port_bmp, del_port)
                {
                    if(TRUE == MW_PORT_CHK(ptr_mrtr->portbmp, del_port))
                    {
                        igmp_snp_port_delMrouterPort((void **)&ptr_mrtr, del_port);
                    }
                }
            }
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handlePortOperInfoMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T                       i = 0, entry_num = 0, port_state = 0;
    UI32_T                      link_down_port = 0;
    UI16_T                      db_idx = 0;
    UI32_T                      link_up_member = 0, link_down_member = 0;
    IGMP_SNP_LIST_T             *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                            request.f_idx,
                                            request.e_idx,
                                            data_size,
                                            method);

    if(PORT_OPER_STATUS == request.f_idx)
    {
        entry_num = (((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size) ? PLAT_MAX_PORT_NUM : 1);
        for(i = 0; i < entry_num; i++)
        {
            db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (i + 1): request.e_idx);
            osapi_memcpy(&port_state, ((UI8_T*)ptr_data + i), sizeof(UI8_T));
            if(0 != port_state)
            {
                link_up_member |= (0x01 << db_idx);
            }
            else
            {
                link_down_member |= (0x01 << db_idx);
            }
        }

        IGMP_SNP_LOG_DEBUG("link_up_member:0x%x, link_down_member:0x%x",
                                                link_up_member,
                                                link_down_member);

        if(0 != link_down_member)
        {
            UI8_T             trunk_id = 0;
            UI32_T            trunk_member;

            if((0 != CSLIST_TOTAL(&ptr_igmp_snp->group_head)) || (0 != CSLIST_TOTAL(&ptr_igmp_snp->mrouter_head)))
            {
                for(link_down_port = 1; link_down_port <= PLAT_MAX_PORT_NUM; link_down_port++)
                {
                    if(0 != (link_down_member & (1 << link_down_port)))
                    {
                        trunk_id = igmp_snp_lag_getPortLagId(link_down_port);
                        if(0 == trunk_id)
                        {
                            /* Port not belongs any trunk */
                            igmp_snp_port_clearEntry(link_down_port);
                        }
                        else
                        {
                            /* Check trunk other port status */
                            trunk_member = igmp_snp_lag_getMembers(trunk_id);
                            /* Bit 0 in the trunk member bitmap represents port 1 not port 0, refer to DB table TRUNK_PORT, TRUNK_MEMBER */
                            if(0 == igmp_snp_port_getPortBmpLinkStatus((trunk_member << 1)))
                            {
                                /* Trunk does link down */
                                for(link_down_port = 1; link_down_port <= PLAT_MAX_PORT_NUM; link_down_port++)
                                {
                                    if(0 != (trunk_member & (0x01 << (link_down_port - 1))))
                                    {
                                        igmp_snp_port_clearEntry(link_down_port);
                                    }
                                }
                                IGMP_SNP_LOG_INFO("Trunk link down, member:0x%x", trunk_member);

                            }

                        }
                    }
                }
            }

        }

        if(0 != link_up_member)
        {
            UI32_T                          static_mrouter_bmp = 0, vlan_member;
            struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;

            /* Bit 0 in the IGMP ROUTER bitmap represents port 1 not port 0, refer to DB table PORT_CFG_INFO, PORT_IGMP_ROUTER */
            static_mrouter_bmp = (ptr_igmp_snp->cfg_info.static_router[0] << 1);

            if(0 != (link_up_member & static_mrouter_bmp))
            {
                IGMP_SNP_LOG_DEBUG("Static mrouter(portbmp: 0x%x) has been linked up", static_mrouter_bmp);
                /* The static port has been linked up, it needs to be updated to all group entries */
                CSLIST_FOREACH(ptr_grp, &ptr_igmp_snp->group_head, next)
                {
                    vlan_member = igmp_snp_vlan_getMember(ptr_grp->vlan_id);
                    if(0 != (vlan_member & (link_up_member & static_mrouter_bmp)))
                    {
                        igmp_snp_updateGroupEntry(ptr_grp);
                    }
                }
            }
        }
    }

#ifdef AIR_SUPPORT_LACP
    else if(PORT_LACP_STATE == request.f_idx)
    {
        UI8_T                   port = 0;
        UI8_T                   lacp_state[MAX_PORT_NUM] = { 0 };
        UI32_T                  aggregated_ports = 0;
        UI32_T                  old_member = 0;
        TRUNK_MEMBER_INFO_T     trunk_info = { 0 };
        IGMP_SNP_LAG_INFO_T     *ptr_lag_info = NULL;

        ptr_lag_info = igmp_snp_lag_getConfig();
        MW_CHECK_PTR(ptr_lag_info);

        if(DB_ALL_ENTRIES != request.e_idx)
        {
            IGMP_SNP_LOG_WARN("Not support LACP state for specific entry\n");
            return MW_E_NOT_SUPPORT;
        }
        osapi_memcpy((void *)&lacp_state, ptr_data, data_size);
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if(LAG_MODE_STATIC == trunk_info.mode)
            {
                continue;
            }
            aggregated_ports = 0;
            osapi_memcpy((void *)&trunk_info, (void *)&(ptr_lag_info->lag[i]), sizeof(TRUNK_MEMBER_INFO_T));
            if (0 != trunk_info.member_bmp)
            {
                for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
                {
                    if ((0 != (trunk_info.member_bmp & (1 << port))) && PORT_LACP_STATE_AGGREGATED == lacp_state[port])
                    {
                        aggregated_ports |= (1 << port);
                    }
                }
            }
            old_member = ptr_lag_info->aggregated_mbr[i];
            ptr_lag_info->aggregated_mbr[i] = aggregated_ports;
            if(old_member != aggregated_ports)
            {
                _igmp_snp_db_handleLagMemberChangeEvent((i + 1), old_member);
            }
        }
    }
#endif /* AIR_SUPPORT_LACP */

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_handleLagMemberChangeEvent(
    UI32_T  lag_id,
    UI32_T  old_member)
{
    UI32_T                  min_member = 0;
    UI32_T                  add_port = 0, del_port = 0;
    UI32_T                  add_member = 0, del_member = 0;
    UI32_T                  new_member = 0;

    new_member = igmp_snp_lag_getMembers(lag_id);
    add_member = new_member & (~old_member);
    del_member = old_member & (~new_member);
    min_member = igmp_snp_lag_getMinMemberPortByLagId(lag_id);

    if(0 != add_member)
    {
        add_port = min_member;
        for(del_port = 1; del_port <= PLAT_MAX_PORT_NUM; del_port++)
        {
            if((0 != (add_member & (0x01 << (del_port - 1)))) && (add_port != del_port))
            {
                igmp_snp_port_transferEntry(add_port, del_port);
            }
        }
        IGMP_SNP_LOG_INFO("add_port:%d, add_member:0x%x", add_port, add_member);
    }

    if(0 != del_member)
    {
        if(0 != new_member)
        {
            add_port = min_member;
            for(del_port = 1; del_port <= PLAT_MAX_PORT_NUM; del_port++)
            {
                if(0 != (del_member & (0x01 << (del_port - 1))))
                {
                    igmp_snp_port_transferEntry(add_port, del_port);
                }
            }
            IGMP_SNP_LOG_INFO("add_port:%d, del_member:0x%x", add_port, del_member);
        }
        else
        {
            /* Trunk has been deleted */
            for(del_port = 1; del_port <= PLAT_MAX_PORT_NUM; del_port++)
            {
                if(0 != (del_member & (0x01 << (del_port - 1))))
                {
                    igmp_snp_port_clearEntry(del_port);
                }
            }
            IGMP_SNP_LOG_INFO("Trunk deleted, del_member:0x%x", del_member);
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_igmp_snp_db_parseMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    if((NULL == ptr_data) || (0 == data_size))
    {
        return MW_E_BAD_PARAMETER;
    }
    switch(method)
    {
        case M_GET:
        case M_UPDATE:
        case M_CREATE:
            switch(request.t_idx)
            {
                case IGMP_SNP_INFO:
                    _igmp_snp_db_handleIgmpSnpInfoMsg(method, request, data_size, ptr_data);
                    break;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
                case IGMP_SNP_QUERIER_INFO:
                    igmp_querier_db_parseMsg(method, request, data_size, ptr_data);
                    break;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
                case VLAN_ENTRY:
                    _igmp_snp_db_handleVlanEntryMsg(method, request, data_size, ptr_data);
                    break;
                case PORT_CFG_INFO:
                    _igmp_snp_db_handlePortCfgInfoMsg(method, request, data_size, ptr_data);
                    break;
                case PORT_OPER_INFO:
                    _igmp_snp_db_handlePortOperInfoMsg(method, request, data_size, ptr_data);
                    break;
                case TRUNK_PORT:
                    _igmp_snp_db_handleTrunkPortMsg(method, request, data_size, ptr_data);
                    break;
                case VLAN_CFG_INFO:
                    _igmp_snp_db_handleVlanCfgInfoMsg(method, request, data_size, ptr_data);
                    break;
                default:
                    break;
            }
            break;
        case M_DELETE:
            if (VLAN_ENTRY == request.t_idx)
            {
                _igmp_snp_db_handleVlanEntryMsg(method, request, data_size, ptr_data);
            }
            else if(L2_MC_ENTRY == request.t_idx)
            {
                _igmp_snp_db_handleL2mcEntryMsg(method, request, data_size, ptr_data);
            }
            break;
        default:
                IGMP_SNP_LOG_ERROR("recv unknown method: [%02X]", method);
            break;
    }

    return MW_E_OK;
}
#endif /* IGMP_SNP_MW_SUPPORT */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: igmp_snp_db_queue_send
 * PURPOSE:
 *      Package the message and send it to DB. If ptr_queue_name is NULL, DB
 *      will not send any response and the message itself will be freed by DB.
 *
 * INPUT:
 *      ptr_queue_name          --  A pointer to the queue that DB will return a
 *                                  response to. If it is NULL, no response is expected.
 *      method                  --  the method bitmap
 *      t_idx                   --  the enum of the table
 *      f_idx                    --  the enum of the field
 *      e_idx                   --  the entry index in the table
 *      ptr_data                --  A pointer to the message data
 *      timeout                 --  The maximun amout of time the thread will wait for
 *                                  send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
igmp_snp_db_queue_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx };

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    ptr_msg = dbapi_createMsgByReq(ptr_queue_name, method, 1, &request, &msg_size, (UI8_T **) &ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    IGMP_SNP_LOG_DEBUG("cq_name=%s method=0x%X count=%u t_idx=%u f_idx=%u e_idx=%u",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx);

    ret = dbapi_appendMsgPayload(&request, (UI8_T *) ptr_data, &ptr_msg, &msg_size, (UI8_T **) &ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    /* Send the message to DB */
    ret = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        IGMP_SNP_LOG_DEBUG("Fail to send the msg to DB(%d)\n", ret);
    }

    return ret;
}

/* FUNCTION NAME: igmp_snp_db_queue_sendWithRsp
 * PURPOSE:
 *      Package the message and send it to DB with response.
 *
 * INPUT:
 *      method                  --  the method bitmap
 *      t_idx                   --  the enum of the table
 *      f_idx                    --  the enum of the field
 *      e_idx                   --  the entry index in the table
 *      ptr_data                --  A pointer to the message data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
igmp_snp_db_queue_sendWithRsp(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data)
{
    return igmp_snp_db_queue_send(IGMP_SNP_QUEUE_NAME, method, t_idx, f_idx, e_idx, ptr_data, IGMP_SNP_SEMA_DELAY);
}

#ifdef IGMP_SNP_MW_SUPPORT
/* FUNCTION NAME: igmp_snp_db_initVariable
 * PURPOSE:
 *      Initialize the IGMP SNP DB variables.
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
igmp_snp_db_initVariable(
    void)
{
    MW_IGMP_SNP_DB_IDX_MAP_CLEAR(_igmp_snp_db_idx);

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_db_getFreeIdx
 * PURPOSE:
 *      This API is used to get the free database index for IGMP snooping.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Free database index for IGMP snooping
 *
 * NOTES:
 *      None
 */
UI16_T
igmp_snp_db_getFreeIdx(
    void)
{
    UI16_T db_idx = 0;

    for(db_idx = 0; db_idx < MAX_L2MC_NUM; db_idx++)
    {
        if(!MW_PORT_CHK(_igmp_snp_db_idx, db_idx))
        {
            IGMP_SNP_LOG_DEBUG("Get free db_idx=%d", db_idx);
            MW_PORT_ADD(_igmp_snp_db_idx, db_idx);
            return db_idx;
        }
    }

    IGMP_SNP_LOG_ERROR("Cannot get free db_idx");
    return MAX_L2MC_NUM;
}

/* FUNCTION NAME:   igmp_snp_db_freeDbIdx
 * PURPOSE:
 *      This API is used to free database index for IGMP snooping.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Free database index for IGMP snooping
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_db_freeDbIdx(
    UI16_T db_idx)
{
    if (MAX_L2MC_NUM > db_idx)
    {
        MW_PORT_DEL(_igmp_snp_db_idx, db_idx);
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_db_appendMsgPayload
 * PURPOSE:
 *      This API is used to append DB message payload for L2MC entry.
 *
 * INPUT:
 *      ptr_grp_data            -- pointer to the group entry
 *      method                  -- The method of this request message
 *      pptr_msg                -- A double pointer to the DB message
 *      ptr_msg_size            -- A pointer to the size of the DB message
 *      pptr_shifted_payload    -- A double pointer to the position of the payload
 *                                 to store the current request. If it is the first
 *                                 request, the start address of the ptr_payload
 *                                 member of the DB message should be inputted
 *
 *
 *
 * OUTPUT:
 *      pptr_msg                -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      ptr_msg_size            -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      pptr_shifted_payload    -- A double pointer returns the position of the
 *                                 payload to store the next request
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
igmp_snp_db_appendMsgPayload(
    void *ptr_grp_data,
    const UI8_T method,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    IGMP_SNP_L2_MC_ENTRY_T          l2mc;
    DB_REQUEST_TYPE_T               request =
    {
        .t_idx = L2_MC_ENTRY,
        .f_idx = DB_ALL_FIELDS,
        .e_idx = 0
    };
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;

    if(((M_UPDATE != method) && (M_DELETE != method)) || (NULL == *pptr_msg) || (NULL == ptr_grp_data))
    {
        return MW_E_BAD_PARAMETER;
    }
    ptr_grp = (struct IGMP_SNP_GROUP_LIST_S *)ptr_grp_data;
    request.e_idx = ptr_grp->db_idx + 1;
    osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
    if((0 != (((*pptr_msg)->type.count + 2) & DB_COUNT_REUSE_FLAG)) ||
       (0 != (((*pptr_msg)->type.count + 1) & DB_COUNT_REUSE_FLAG)))
    {
        rc = dbapi_sendMsg((*pptr_msg), MSG_TIMEOUT_RETRUN_IMMEDIATELY);
        if (MW_E_OK == rc)
        {
            IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", (*pptr_msg)->type.count);
            (*pptr_msg) = dbapi_createMsg(NULL, method, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T), ptr_msg_size, pptr_shifted_payload);
            if (NULL == *pptr_msg)
            {
                return MW_E_NO_MEMORY;
            }
        }
        else
        {
            /* Not need free memory */
            *pptr_msg = NULL;
            return rc;
        }
    }

    if(M_UPDATE == method)
    {
            igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
            /* The update_flag is used to ensure that igmp snooping group entry can be updated to DB
            * when l2mc's mac_addr/ip_addr, vid, portbitmap have not changed.
            */
            l2mc.update_flag = TRUE;
            rc = dbapi_appendMsgPayload(&request, (UI8_T *) &l2mc, pptr_msg, ptr_msg_size, pptr_shifted_payload);
            l2mc.update_flag = FALSE;
            rc |= dbapi_appendMsgPayload(&request, (UI8_T *) &l2mc, pptr_msg, ptr_msg_size, pptr_shifted_payload);
    }
    else if(M_DELETE == method)
    {
        rc = dbapi_appendMsgPayload(&request, NULL, pptr_msg, ptr_msg_size, pptr_shifted_payload);
    }

    if (MW_E_OK != rc)
    {
        MW_FREE(*pptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    return rc;
}

/* FUNCTION NAME: igmp_snp_db_subscribeModule
 * PURPOSE:
 *      Subscribe to the database module
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
igmp_snp_db_subscribeModule(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* Check DB is ready or not */
    do
    {
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_INFO, IGMP_SNP_ENABLE, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_INFO, IGMP_SNP_RPT_SUPPRESS, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_INFO, IGMP_SNP_FAST_LEAVE, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, NULL);
#ifdef AIR_SUPPORT_LACP
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, PORT_OPER_INFO, PORT_LACP_STATE, DB_ALL_ENTRIES, NULL);
#endif /* AIR_SUPPORT_LACP */
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, PORT_CFG_INFO, PORT_IGMP_ROUTER, DB_ALL_ENTRIES, NULL);

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    igmp_querier_db_subscribeModule();
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    return MW_E_OK;
}

/* FUNCTION NAME: igmp_snp_db_handleMsg
 * PURPOSE:
 *      Handle the database message.
 *
 * INPUT:
 *      ptr_msg                 --  pointer to the database message
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
igmp_snp_db_handleMsg(
    DB_MSG_T *ptr_msg)
{
    UI8_T i = 0;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL != ptr_msg)
    {
        IGMP_SNP_LOG_DEBUG("[%d]recv method - %02X", ptr_msg->type.count, ptr_msg->method);
        if(0 != (M_B_RESPONSE & ptr_msg->method))
        {
            IGMP_SNP_LOG_DEBUG("response message");
        }
        else
        {
            /* Process the notification message */
            do {
                rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
                if (MW_E_OK == rc)
                {
                    IGMP_SNP_LOG_DEBUG("index=%u, t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u",
                                                            i++,
                                                            request.t_idx,
                                                            request.f_idx,
                                                            request.e_idx,
                                                            data_size);

                    rc = _igmp_snp_db_parseMsg(ptr_msg->method, request, data_size, ptr_data);
                    if (MW_E_OK != rc)
                    {
                        IGMP_SNP_LOG_ERROR("handle_db_msg failed!(%d)", rc);
                    }
                }
                /* Continue to parse the next request within the payload. */
            } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
        }
    }
    else
    {
        IGMP_SNP_LOG_ERROR("Message from DB is NULL");
    }
    return;
}
#endif /* IGMP_SNP_MW_SUPPORT */