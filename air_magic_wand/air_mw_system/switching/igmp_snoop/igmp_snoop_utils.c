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

/* FILE NAME:  igmp_snoop_utils.c
 * PURPOSE:
 *    This file contains the implementation of IGMP Snooping utility functions.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_utils.h"
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_lag.h"
#endif
#include "igmp_snoop_port.h"
#include "igmp_snoop_timer.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_vlan.h"
#include "lwip/ip4_addr.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/igmp.h"
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier_utils.h"
#endif
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_db.h"
#include "igmp_snoop_msg.h"
#else
#include "igmp_snoop_syncd.h"
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
/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_snp_clearAllEntry
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_clearAllEntry(
    void)
{
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr_next = NULL;
    IGMP_SNP_LIST_T                *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);

    CSLIST_FOREACH_SAFE(ptr_grp, ptr_grp_next, &ptr_igmp_snp->group_head, next)
    {
#ifdef IGMP_SNP_MW_SUPPORT
        igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
#endif
        CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
        MW_FREE(ptr_grp);
    }
    CSLIST_INIT(&ptr_igmp_snp->group_head);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
    osapi_memset(ptr_igmp_snp->port_group_num, 0, sizeof(ptr_igmp_snp->port_group_num));
#endif

#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_LOG_DEBUG("M_DELETE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0");
    igmp_snp_db_queue_send(NULL, M_DELETE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL, IGMP_SNP_SEMA_DELAY);
#else
    igmp_snp_syncd_updateGroupEntry(M_DELETE, TRUE, NULL);
#endif

    CSLIST_FOREACH_SAFE(ptr_mrtr, ptr_mrtr_next, &ptr_igmp_snp->mrouter_head, next)
    {
        CSLIST_REMOVE(&ptr_igmp_snp->mrouter_head, ptr_mrtr, IGMP_SNP_MROUTER_LIST_S, next);
        MW_FREE(ptr_mrtr);
    }
    CSLIST_INIT(&ptr_igmp_snp->mrouter_head);

#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T                          *ptr_state = NULL;

    osapi_calloc(sizeof(UI8_T) * PLAT_MAX_PORT_NUM, IGMP_SNP_MODULE_NAME, (void**)&ptr_state);
    if(NULL != ptr_state)
    {
        igmp_snp_db_queue_send(NULL, M_UPDATE, PORT_OPER_INFO, PORT_OPER_ROUTER, DB_ALL_FIELDS, ptr_state, IGMP_SNP_SEMA_DELAY);
        MW_FREE(ptr_state);
    }
    igmp_snp_db_initVariable();
#endif /* IGMP_SNP_MW_SUPPORT */

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    igmp_querier_clearAllEntry();
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
    IGMPSNP_STATS_CLEAR();

    return MW_E_OK;
}

#ifdef IGMP_SNP_MW_SUPPORT
/* FUNCTION NAME:   igmp_snp_clearEntryByType
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry by specific type.
 *
 * INPUT:
 *      ptr_data      --  pointer of clear entry data structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_clearEntryByType(
    void *ptr_data)
{
    I32_T                          rc = MW_E_OK;
    UI8_T                          *ptr_update_payload = NULL, *ptr_delete_payload = NULL;
    UI16_T                         update_msg_size = 0, delete_msg_size = 0;
    DB_MSG_T                       *ptr_update_multi_msg = NULL, *ptr_delete_multi_msg = NULL;
    DB_REQUEST_TYPE_T              request =
    {
        .t_idx = PORT_OPER_INFO,
        .f_idx = PORT_OPER_ROUTER,
        .e_idx = 0
    };
    UI8_T                          state = 0;
    UI16_T                         vid = 0;
    UI32_T                         port_id = 0;
    IGMP_SNP_LIST_T                *ptr_igmp_snp = NULL;
    MW_PORT_BITMAP_T               portbmp = {0};
    IGMP_SNP_MSG_CLEAR_ENTRY_T     *ptr_clear_msg = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if ((NULL == ptr_data) || (NULL == ptr_igmp_snp))
    {
        return MW_E_BAD_PARAMETER;
    }
    ptr_clear_msg = (IGMP_SNP_MSG_CLEAR_ENTRY_T *)ptr_data;
    osapi_memcpy(portbmp, ptr_clear_msg->portbmp, sizeof(MW_PORT_BITMAP_T));
    switch (ptr_clear_msg->type)
    {
        case IGMP_SNP_ENTRY_FLUSH_TYPE_PORT:
        {
            MW_PORT_FOREACH(portbmp, port_id)
            {
                if(PLAT_CPU_PORT == port_id)
                {
                    /* Skip cpu port */
                    continue;
                }
                igmp_snp_port_clearEntry(port_id);
            }
            break;
        }

        case IGMP_SNP_ENTRY_FLUSH_TYPE_VID:
        {
            MW_CHECK_MIN_MAX_RANGE(ptr_clear_msg->vid, 1, IGMP_SNP_CONFIG_MAX_VID);
            vid = ptr_clear_msg->vid;

            ptr_update_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, sizeof(UI8_T), &update_msg_size, &ptr_update_payload);
            if (NULL == ptr_update_multi_msg)
            {
                IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
                return MW_E_NO_MEMORY;
            }

            ptr_mrtr = igmp_snp_searchMrouterEntry(vid);
            if(NULL != ptr_mrtr)
            {
                MW_PORT_FOREACH(ptr_mrtr->portbmp, port_id)
                {
                    if(PLAT_CPU_PORT == port_id)
                    {
                        /* Skip cpu port */
                        continue;
                    }
                    if((IGMP_SNP_STATIC_MROUTER != ptr_mrtr->timer[(port_id - 1)]) &&
                    (0 != ptr_mrtr->timer[(port_id - 1)]))
                    {
                        /* dynamic router port */
                        IGMP_SNP_LOG_DEBUG("delete mrouter port, vid:%d, port %d.", ptr_mrtr->vlan_id, port_id);
                        ptr_mrtr->timer[(port_id - 1)] = 0;
                        MW_PORT_DEL(ptr_mrtr->portbmp, port_id);
                        if (MW_E_OK != igmp_snp_port_isMrouter(port_id))
                        {
                            /* If the port is not a mrouter port, update info to DB */
                            request.e_idx = port_id;
                            rc = dbapi_appendMsgPayload(&request, &state, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
                            if (MW_E_OK != rc)
                            {
                                MW_FREE(ptr_update_multi_msg);
                                return MW_E_OP_INCOMPLETE;
                            }
                        }
                        if (0 == ptr_mrtr->portbmp[0])
                        {
                            IGMP_SNP_LOG_DEBUG("remove mrouter entry");
                            CSLIST_REMOVE(&ptr_igmp_snp->mrouter_head, ptr_mrtr, IGMP_SNP_MROUTER_LIST_S, next);
                            MW_FREE(ptr_mrtr);
                            break;
                        }
                    }
                }
            }

            ptr_delete_multi_msg = dbapi_createMsg(NULL, M_DELETE, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T), &delete_msg_size, &ptr_delete_payload);
            if (NULL == ptr_delete_multi_msg)
            {
                IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
                MW_FREE(ptr_update_multi_msg);
                return MW_E_NO_MEMORY;
            }

            CSLIST_FOREACH_SAFE(ptr_grp, ptr_grp_next, &ptr_igmp_snp->group_head, next)
            {
                if(vid == ptr_grp->vlan_id)
                {
                /* If a port removed from the vlan, delete the group entry under this port */
                    MW_PORT_FOREACH(ptr_grp->portbmp, port_id)
                    {
                        if(PLAT_CPU_PORT == port_id)
                        {
                            /* Skip cpu port */
                            continue;
                        }
                        MW_PORT_DEL(ptr_grp->portbmp, port_id);
                        ptr_grp->timer[(port_id-1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                        ptr_igmp_snp->port_group_num[(port_id-1)]--;
#endif
                    }

                    if (0 == ptr_grp->portbmp[0])
                    {
                        rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_DELETE, &ptr_delete_multi_msg, &delete_msg_size, &ptr_delete_payload);
                        igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
                        CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
                        IGMP_SNP_LOG_DEBUG("delete group entry OK, vid:%d, db_idx %d", ptr_grp->vlan_id, ptr_grp->db_idx);
                        MW_FREE(ptr_grp);
                    }
                    else
                    {
                        rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
                    }

                    if(MW_E_OK != rc)
                    {
                        MW_FREE(ptr_update_multi_msg);
                        MW_FREE(ptr_delete_multi_msg);
                        IGMP_SNP_LOG_ERROR("Append msg payload failed, rc:%d\n", rc);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
            }

            break;
        }

        default:
        {
            rc = MW_E_NOT_SUPPORT;
            break;
        }
    }

    if(NULL != ptr_update_multi_msg)
    {
        if(0 != ptr_update_multi_msg->type.count)
        {
            IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_update_multi_msg->type.count);
            rc = dbapi_sendMsg(ptr_update_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
        }
        else
        {
            MW_FREE(ptr_update_multi_msg);
        }
    }

    if(NULL != ptr_delete_multi_msg)
    {
        if(0 != ptr_delete_multi_msg->type.count)
        {
            IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_delete_multi_msg->type.count);
            rc |= dbapi_sendMsg(ptr_delete_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
        }
        else
        {
            MW_FREE(ptr_delete_multi_msg);
        }
    }

    return rc;
}
#endif /* IGMP_SNP_MW_SUPPORT */

/* FUNCTION NAME:   igmp_snp_addGroupEntry
 * PURPOSE:
 *      This API is used to add a IGMP snooping group entry.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      ptr_ipaddr   --  multicast group address
 *      port_id      --  ingress front port ID
 *      sec          --  update time in seconds
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_TABLE_FULL
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_addGroupEntry(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr,
    UI8_T port_id,
    UI16_T sec)
{
    struct IGMP_SNP_GROUP_LIST_S *ptr_new = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("vid=%d, gip=%s, port=%d, sec=%d", vlan_id, ip4addr_ntoa(ptr_ipaddr), port_id, sec);
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    MW_MAC_T mac = {0};
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

    if ((IGMP_SNP_MIN_VID == vlan_id) || (0 == port_id))
    {
        IGMP_SNP_LOG_ERROR("Parameter is invalid!");
        return MW_E_BAD_PARAMETER;
    }

    if (CSLIST_TOTAL(&ptr_igmp_snp->group_head) >= IGMP_SNP_MAX_L2MC_ENTRY_NUM)
    {
        IGMP_SNP_LOG_ERROR("Error: Table(size:%d) is full!", IGMP_SNP_MAX_L2MC_ENTRY_NUM);
        return MW_E_TABLE_FULL;
    }

#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
    if (ptr_igmp_snp->port_group_num[(port_id - 1)] >= IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT)
    {
        IGMP_SNP_LOG_INFO("Num of group entry on port %d is %d, which reachs the max num %d", port_id, ptr_igmp_snp->port_group_num[(port_id - 1)], IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT);
        return MW_E_TABLE_FULL;
    }
#endif

    osapi_calloc(sizeof(struct IGMP_SNP_GROUP_LIST_S), IGMP_SNP_MODULE_NAME, (void**)&ptr_new);
    if(NULL != ptr_new)
    {
        IGMP_SNP_L2_MC_ENTRY_T         l2mc;

        osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
        ptr_new->vlan_id = vlan_id;
        MW_PORT_ADD(ptr_new->portbmp, port_id);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
        ptr_igmp_snp->port_group_num[(port_id - 1)]++;
#endif
        ptr_new->timer[(port_id - 1)] = sec * IGMP_SNP_TMR_RATE_S;
        ptr_new->next.sle_next = NULL;
        CSLIST_INSERT_HEAD(&ptr_igmp_snp->group_head, ptr_new, next);
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
        IGMP_SNP_DMAC_FROM_DIP(ntohl(ptr_ipaddr->addr), mac);
        osapi_memcpy(ptr_new->mac_address, mac, sizeof(MW_MAC_T));
#else
        ptr_new->group_address.addr = ptr_ipaddr->addr;
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

        igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_new);
        IGMP_SNP_LOG_DEBUG("ipaddr:%s, vid=%d, portbmp=0x%x", ip4addr_ntoa(ptr_ipaddr), l2mc.vid, l2mc.portbmp);
#ifdef IGMP_SNP_MW_SUPPORT
        ptr_new->db_idx = igmp_snp_db_getFreeIdx();
        IGMP_SNP_LOG_DEBUG("count=%d, db_idx=%d", CSLIST_TOTAL(&ptr_igmp_snp->group_head), ptr_new->db_idx);
        IGMP_SNP_LOG_DEBUG("M_UPDATE, L2_MC_ENTRY, DB_ALL_FIELDS, db_idx=%d", ptr_new->db_idx);
        igmp_snp_db_queue_send(NULL, M_CREATE, L2_MC_ENTRY, DB_ALL_FIELDS, ptr_new->db_idx+1, &l2mc, IGMP_SNP_SEMA_DELAY);
#else
        igmp_snp_syncd_updateGroupEntry(M_CREATE, FALSE, &l2mc);
#endif /* IGMP_SNP_MW_SUPPORT */
    }
    else
    {
        IGMP_SNP_LOG_ERROR("Error: Failed to allocated memory for group_list!");
        return MW_E_NO_MEMORY;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_delGroupEntry
 * PURPOSE:
 *      This API is used to remove an IGMP snooping group entry.
 *
 * INPUT:
 *      pptr_grp      --  pointer to pointer of group entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      The IGMP snooping group entry will be freed.
 */
MW_ERROR_NO_T
igmp_snp_delGroupEntry(
    void **pptr_grp)
{
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(pptr_grp);
    ptr_grp = *pptr_grp;
    if ((NULL == ptr_grp) || (NULL == ptr_igmp_snp))
    {
        IGMP_SNP_LOG_ERROR("Group entry is invalid!");
        return MW_E_BAD_PARAMETER;
    }

#ifdef IGMP_SNP_MW_SUPPORT
    igmp_snp_db_queue_send(NULL, M_DELETE, L2_MC_ENTRY, DB_ALL_FIELDS, ptr_grp->db_idx+1, NULL, IGMP_SNP_SEMA_DELAY);
    igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
    IGMP_SNP_LOG_DEBUG("delete group entry OK, db_idx %d", ptr_grp->db_idx);
#else
    IGMP_SNP_L2_MC_ENTRY_T      l2mc;

    osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
    igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
    igmp_snp_syncd_updateGroupEntry(M_DELETE, FALSE, (void *) &l2mc);
#endif /* IGMP_SNP_MW_SUPPORT */

    CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);

    MW_FREE(*pptr_grp);

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_searchGroupEntry
 * PURPOSE:
 *      This API is used to search IGMP snooping group entries.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      ptr_ipaddr   --  multicast group address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of entry.
 *
 * NOTES:
 *      None
 */
struct IGMP_SNP_GROUP_LIST_S*
igmp_snp_searchGroupEntry(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr)
{
    struct IGMP_SNP_GROUP_LIST_S *ptr_cur = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    IGMP_SNP_LOG_DEBUG("vid=%d, gip=%s", vlan_id, ip4addr_ntoa(ptr_ipaddr));

    if ((IGMP_SNP_MIN_VID == vlan_id) || (IGMP_SNP_CONFIG_MAX_VID < vlan_id) || (NULL == ptr_igmp_snp))
    {
        IGMP_SNP_LOG_ERROR("vlan_id:%d is invalid!", vlan_id);
        return NULL;
    }

    CSLIST_FOREACH(ptr_cur, &ptr_igmp_snp->group_head, next)
    {
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
        MW_MAC_T mac = {0};
        IGMP_SNP_DMAC_FROM_DIP(ntohl(ptr_ipaddr->addr), mac);
        if ((vlan_id == ptr_cur->vlan_id) &&
            (0 == osapi_memcmp(ptr_cur->mac_address, mac, sizeof(mac))))
#else
        if ((vlan_id == ptr_cur->vlan_id) &&
            (TRUE == ip4_addr_cmp(&(ptr_cur->group_address), ptr_ipaddr)))
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
        {
            return ptr_cur;
        }
    }

    return NULL;
}

/* FUNCTION NAME:   igmp_snp_updateGroupEntry
 * PURPOSE:
 *      This API is used to update an IGMP snooping group entry to DB.
 *
 * INPUT:
 *      ptr_grp      --  pointer of group entry
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
 *      None
 */
MW_ERROR_NO_T
igmp_snp_updateGroupEntry(
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    IGMP_SNP_L2_MC_ENTRY_T      l2mc;

    osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
    rc = igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
    if(MW_E_OK != rc)
    {
        return rc;
    }

#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_LOG_DEBUG("M_UPDATE, L2_MC_ENTRY, DB_ALL_FIELDS, db_idx=%d", ptr_grp->db_idx);
    /* The update_flag is used to ensure that igmp snooping group entry can be updated to DB
     * when l2mc's mac_addr/ip_addr, vid, portbitmap have not changed.
     */
    l2mc.update_flag = TRUE;
    rc = igmp_snp_db_queue_send(NULL, M_UPDATE, L2_MC_ENTRY, DB_ALL_FIELDS, ptr_grp->db_idx+1, &l2mc, IGMP_SNP_SEMA_DELAY);
    if(MW_E_OK == rc)
    {
        l2mc.update_flag = FALSE;
        rc = igmp_snp_db_queue_send(NULL, M_UPDATE, L2_MC_ENTRY, DB_ALL_FIELDS, ptr_grp->db_idx+1, &l2mc, IGMP_SNP_SEMA_DELAY);
    }
#else
    l2mc.update_flag = TRUE;
    igmp_snp_syncd_updateGroupEntry(M_UPDATE, FALSE, (void *) &l2mc);
#endif /* IGMP_SNP_MW_SUPPORT */

    return rc;
}

/* FUNCTION NAME:   igmp_snp_searchMrouterEntry
 * PURPOSE:
 *      This API is used to look for IGMP snooping mrouter entries.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of entry.
 *
 * NOTES:
 *      None
 */
struct IGMP_SNP_MROUTER_LIST_S*
igmp_snp_searchMrouterEntry(
    UI16_T vlan_id)
{
    struct IGMP_SNP_MROUTER_LIST_S *ptr_cur = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();

    if ((IGMP_SNP_MIN_VID == vlan_id) || (IGMP_SNP_CONFIG_MAX_VID < vlan_id) || (NULL == ptr_igmp_snp))
    {
        IGMP_SNP_LOG_ERROR("vlan_id:%d is invalid!", vlan_id);
        return NULL;
    }

    CSLIST_FOREACH(ptr_cur, &ptr_igmp_snp->mrouter_head, next)
    {
        if (vlan_id == ptr_cur->vlan_id)
        {
            IGMP_SNP_LOG_DEBUG("vid=%d, mrouter found, portbmp=0x%x", vlan_id, ptr_cur->portbmp[0]);
            return ptr_cur;
        }
    }

    IGMP_SNP_LOG_DEBUG("vid=%d, mrouter not found!", vlan_id);
    return NULL;
}

/* FUNCTION NAME:   igmp_snp_convertGroupEntryToL2mcEntry
 * PURPOSE:
 *      This API is used to convert group entry to L2MC entry.
 *
 * INPUT:
 *      ptr_l2mc_data           -- pointer to the L2MC entry
 *      ptr_grp_data            -- pointer to the group entry
 *
 * OUTPUT:
 *      ptr_l2mc                -- pointer to the L2MC entry(updated)
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_convertGroupEntryToL2mcEntry(
    void    *ptr_l2mc_data,
    void    *ptr_grp_data)
{
    IGMP_SNP_L2_MC_ENTRY_T          *ptr_l2mc = NULL;
    struct IGMP_SNP_GROUP_LIST_S    *ptr_grp = NULL;
    struct IGMP_SNP_MROUTER_LIST_S  *ptr_mrtr = NULL;

    if((NULL == ptr_l2mc_data) || (NULL == ptr_grp_data))
    {
        IGMP_SNP_LOG_ERROR("Input parameter is invalid!");
        return MW_E_BAD_PARAMETER;
    }

    ptr_l2mc = (IGMP_SNP_L2_MC_ENTRY_T *)ptr_l2mc_data;
    osapi_memset(ptr_l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
    ptr_grp = (struct IGMP_SNP_GROUP_LIST_S *)ptr_grp_data;
    ptr_l2mc->vid = ptr_grp->vlan_id;
    ptr_l2mc->portbmp = ptr_grp->portbmp[0];
    ptr_mrtr = igmp_snp_searchMrouterEntry(ptr_grp->vlan_id);
    if (NULL != ptr_mrtr)
    {
#ifdef IGMP_SNP_MW_SUPPORT
        ptr_l2mc->portbmp |= igmp_snp_lag_updateLinkStatusBmpWithLagInfo(ptr_mrtr->portbmp[0]);
#else
        ptr_l2mc->portbmp |= ptr_mrtr->portbmp[0];
#endif /* IGMP_SNP_MW_SUPPORT */
    }
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    memcpy(ptr_l2mc->mac_addr, ptr_grp->mac_address, sizeof(MW_MAC_T));
    IGMP_SNP_LOG_DEBUG("mac_addr=%02x:%02x:%02x:%02x:%02x:%02x, vid=%d, portbmp=0x%x",
                                                    ptr_l2mc->mac_addr[0],
                                                    ptr_l2mc->mac_addr[1],
                                                    ptr_l2mc->mac_addr[2],
                                                    ptr_l2mc->mac_addr[3],
                                                    ptr_l2mc->mac_addr[4],
                                                    ptr_l2mc->mac_addr[5],
                                                    ptr_l2mc->vid, ptr_l2mc->portbmp);
#else
    ptr_l2mc->ip_addr = (UI32_T) lwip_ntohl(ptr_grp->group_address.addr);
    IGMP_SNP_LOG_DEBUG("ip_addr=0x%x, vid=%d, portbmp=0x%x", ptr_l2mc->ip_addr, ptr_l2mc->vid, ptr_l2mc->portbmp);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_convertPortbitmapToString
 * PURPOSE:
 *      This API is used to convert port bitmap to string.
 *
 * INPUT:
 *      portbmp                 -- port bitmap to convert
 *
 * OUTPUT:
 *      ptr_pbmp_str            -- pointer to the string buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_convertPortbitmapToString(
    MW_PORT_BITMAP_T portbmp,
    char             *ptr_pbmp_str)
{
    UI32_T port, first, span = 0;
    C8_T   tmp_str[IGMP_SNP_PORTBMP_STRING_LEN];

    osapi_memset(ptr_pbmp_str, 0, IGMP_SNP_PORTBMP_STRING_LEN);
    if (!portbmp[0])
    {
        osapi_snprintf(ptr_pbmp_str, IGMP_SNP_PORTBMP_STRING_LEN, "NULL");
        return MW_E_BAD_PARAMETER;
    }
    else
    {
        first = 1;
        MWLIB_BITMAP_BIT_FOREACH((portbmp), port, MW_PORT_BITMAP_SIZE)
        {
            osapi_memset(tmp_str, 0, IGMP_SNP_PORTBMP_STRING_LEN);
            osapi_snprintf(tmp_str, IGMP_SNP_PORTBMP_STRING_LEN, "%s%d", first ? "" : ",", port);
            osapi_snprintf(ptr_pbmp_str + osapi_strlen(ptr_pbmp_str),
                IGMP_SNP_PORTBMP_STRING_LEN - osapi_strlen(ptr_pbmp_str), "%s", tmp_str);
            first = 0;
            for (span = 1; (++port < MW_PORT_NUM) && MWLIB_BITMAP_BIT_CHK((portbmp), port); span++);
            if (span > 1)
            {
                osapi_memset(tmp_str, 0, IGMP_SNP_PORTBMP_STRING_LEN);
                osapi_snprintf(tmp_str, IGMP_SNP_PORTBMP_STRING_LEN, "-%d", port - 1);
                osapi_snprintf(ptr_pbmp_str + osapi_strlen(ptr_pbmp_str),
                    IGMP_SNP_PORTBMP_STRING_LEN - osapi_strlen(ptr_pbmp_str), "%s", tmp_str);
            }
        }
    }

    return MW_E_OK;
}