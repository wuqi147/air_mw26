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

/* FILE NAME:  igmp_snoop_port.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop port.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_port.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_db.h"
#else
#include "igmp_snoop_syncd.h"
#endif
#include "igmp_snoop_log.h"
#include "igmp_snoop_timer.h"
#include "igmp_snoop_utils.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "sfp_port.h"
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
/* FUNCTION NAME:   igmp_snp_port_addMrouterPort
 * PURPOSE:
 *      This API is used to add a IGMP snooping mrouter entry.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      port_id      --  ingress front port ID
 *      time         --  Mrouter timer in seconds
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TABLE_FULL
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_addMrouterPort(
    UI16_T vlan_id,
    UI32_T port_id,
    UI16_T time)
{
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;
    UI8_T  state = 0;
    BOOL_T update_group_entry = FALSE;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    UNUSED(state);
    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("vid=%d, port=%d", vlan_id, port_id);
    if ((IGMP_SNP_MIN_VID == vlan_id) || (0 == port_id) || (AIR_MAX_PORT_NUM < port_id))
    {
        IGMP_SNP_LOG_ERROR("Parameter is invalid!");
        return MW_E_BAD_PARAMETER;
    }

    ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
    if (ptr_mrtr != NULL)
    {
        /* update timer */
        IGMP_SNP_LOG_DEBUG("add port %d, update timer to %d sec", port_id, time);
        if(IGMP_SNP_STATIC_MROUTER != ptr_mrtr->timer[(port_id - 1)])
        {
            ptr_mrtr->timer[(port_id - 1)] = ((IGMP_SNP_STATIC_MROUTER == time) ? time : (time * IGMP_SNP_TMR_RATE_S));
        }
        if (MW_PORT_CHK(ptr_mrtr->portbmp, port_id) != TRUE)
        {
            MW_PORT_ADD(ptr_mrtr->portbmp, port_id);
            state = IGMP_SNP_ROUTER_PORT_VALID;
            update_group_entry = TRUE;
        }
        else
        {
            if(IGMP_SNP_STATIC_MROUTER == time)
            {
#ifdef IGMP_SNP_MW_SUPPORT
                /* Update dynamic router port to static router port */
                igmp_snp_db_queue_send(NULL, M_UPDATE, PORT_OPER_INFO, PORT_OPER_ROUTER, port_id, &state, IGMP_SNP_SEMA_DELAY);
#endif
                update_group_entry = TRUE;
            }
        }
    }
    else
    {
        /* Add new mrouter entry */
        if (CSLIST_TOTAL(&ptr_igmp_snp->mrouter_head) >= IGMP_SNP_MAX_VLAN_ENTRY_NUM)
        {
            IGMP_SNP_LOG_ERROR("Error: Table is full!");
            return MW_E_TABLE_FULL;
        }
        osapi_calloc(sizeof(struct IGMP_SNP_MROUTER_LIST_S), IGMP_SNP_MODULE_NAME, (void**)&ptr_mrtr);
        if(NULL != ptr_mrtr)
        {
            ptr_mrtr->vlan_id = vlan_id;
            MW_PORT_ADD(ptr_mrtr->portbmp, port_id);
            ptr_mrtr->timer[(port_id - 1)] = ((IGMP_SNP_STATIC_MROUTER == time) ? time : (time * IGMP_SNP_TMR_RATE_S));
            ptr_mrtr->next.sle_next = NULL;

            CSLIST_INSERT_HEAD(&ptr_igmp_snp->mrouter_head, ptr_mrtr, next);
            IGMP_SNP_LOG_DEBUG("count=%d", CSLIST_TOTAL(&ptr_igmp_snp->mrouter_head));
            state = IGMP_SNP_ROUTER_PORT_VALID;
            update_group_entry = TRUE;
        }
        else
        {
            IGMP_SNP_LOG_ERROR("Error: Failed to allocated memory for mrouter_list!");
            return MW_E_NO_MEMORY;
        }
    }
#ifdef IGMP_SNP_MW_SUPPORT
    if((IGMP_SNP_STATIC_MROUTER != time) && (IGMP_SNP_ROUTER_PORT_VALID == state))
    {
        igmp_snp_db_queue_send(NULL, M_UPDATE, PORT_OPER_INFO, PORT_OPER_ROUTER, port_id, &state, IGMP_SNP_SEMA_DELAY);
    }
#endif

    if(TRUE == update_group_entry)
    {
        CSLIST_FOREACH(ptr_grp, &ptr_igmp_snp->group_head, next)
        {
            if(ptr_grp->vlan_id == vlan_id)
            {
                igmp_snp_updateGroupEntry(ptr_grp);
            }
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_port_addStaticMrouterPort
 * PURPOSE:
 *      This API is used to add static mrouter port for IGMP snooping.
 *
 * INPUT:
 *      add_member   --  Port list of static router to add
 *      del_member   --  Port list of static router to remove
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
igmp_snp_port_addStaticMrouterPort(
    UI32_T add_member,
    UI32_T del_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T index = 0, i = 0;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    IGMP_SNP_VLAN_CFG_T         *ptr_igmp_snp_vlan = NULL;

    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp_vlan);
    if(IGMP_SNP_VLAN_MTU == ptr_igmp_snp_vlan->vlan_mode)
    {
        if (0 < add_member)
        {
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if(add_member & (0x01 << i))
                {
                    igmp_snp_port_addMrouterPort(IGMP_SNP_DEFAULT_VID, (i + 1), IGMP_SNP_STATIC_MROUTER);
                }
            }
        }
        if(0 < del_member)
        {
            ptr_mrtr = igmp_snp_searchMrouterEntry(IGMP_SNP_DEFAULT_VID);
            if(NULL != ptr_mrtr)
            {
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if((del_member & (0x01 << i)) && (ptr_mrtr->portbmp[0] & (0x01 << (i + 1))))
                    {
                        igmp_snp_port_delMrouterPort((void **)&ptr_mrtr, (i + 1));
                        /* mrouter may have been deleted */
                        if(NULL == ptr_mrtr)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (index = 0; index < IGMP_SNP_MAX_VLAN_ENTRY_NUM; index++)
        {
            if (0 != ptr_igmp_snp_vlan->vlan_entry[index].vlan_id)
            {
                if (0 < add_member)
                {
                    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                    {
                        if(add_member & (0x01 << i))
                        {
                            igmp_snp_port_addMrouterPort(ptr_igmp_snp_vlan->vlan_entry[index].vlan_id, (i + 1), IGMP_SNP_STATIC_MROUTER);
                        }
                    }
                }
                if (0 < del_member)
                {
                    ptr_mrtr = igmp_snp_searchMrouterEntry(ptr_igmp_snp_vlan->vlan_entry[index].vlan_id);
                    if(NULL != ptr_mrtr)
                    {
                        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                        {
                            if((del_member & (0x01 << i)) && (ptr_mrtr->portbmp[0] & (0x01 << (i + 1))))
                            {
                                igmp_snp_port_delMrouterPort((void **)&ptr_mrtr, (i + 1));
                                /* mrouter may have been deleted */
                                if(NULL == ptr_mrtr)
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

/* FUNCTION NAME:   igmp_snp_port_delMrouterPort
 * PURPOSE:
 *      This API is used to delete the mrouter entry for the specified port.
 *
 * INPUT:
 *      pptr_mrtr    --  the pointer to pointer of the mrouter entry
 *      port_id      --  ingress front port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 */
MW_ERROR_NO_T
igmp_snp_port_delMrouterPort(
    void    **pptr_mrtr,
    UI32_T  port_id)
{
    IGMP_SNP_LIST_T              *ptr_igmp_snp = NULL;
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = NULL;

    MW_CHECK_PTR(pptr_mrtr);
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = *pptr_mrtr;
    ptr_igmp_snp = igmp_snp_getConfig();
    if((0 == port_id) || (NULL == ptr_mrtr) || (NULL == ptr_igmp_snp))
    {
        return MW_E_BAD_PARAMETER;
    }
    IGMP_SNP_LOG_DEBUG("vid=%d, port=%d", ptr_mrtr->vlan_id, port_id);

    MW_PORT_DEL(ptr_mrtr->portbmp, port_id);

#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T state = 0;

    if((IGMP_SNP_STATIC_MROUTER != ptr_mrtr->timer[(port_id - 1)]) &&
       (MW_E_OK != igmp_snp_port_isMrouter(port_id)))
    {
        /* If the port is not a mrouter port, update info to DB */
        igmp_snp_db_queue_send(NULL, M_UPDATE, PORT_OPER_INFO, PORT_OPER_ROUTER, port_id, &state, IGMP_SNP_SEMA_DELAY);
    }
#endif /* IGMP_SNP_MW_SUPPORT */

    ptr_mrtr->timer[(port_id - 1)] = 0;
    CSLIST_FOREACH(ptr_grp, &(ptr_igmp_snp->group_head), next)
    {
        if(ptr_grp->vlan_id == ptr_mrtr->vlan_id)
        {
            igmp_snp_updateGroupEntry(ptr_grp);
        }
    }

    if (0 == ptr_mrtr->portbmp[0])
    {
        IGMP_SNP_LOG_DEBUG("remove mrouter entry");
        CSLIST_REMOVE(&(ptr_igmp_snp->mrouter_head), ptr_mrtr, IGMP_SNP_MROUTER_LIST_S, next);
        MW_FREE(*pptr_mrtr);
    }

    return  MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_port_delStaticMrouterPort
 * PURPOSE:
 *      This API is used to remove static router port of specific VID.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
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
igmp_snp_port_delStaticMrouterPort(
    UI16_T vlan_id)
{
    UI32_T i = 0;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    IGMP_SNP_LIST_T              *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    ptr_mrtr = igmp_snp_searchMrouterEntry(vlan_id);
    if ((NULL != ptr_mrtr) && (NULL != ptr_igmp_snp))
    {
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if((0 != (ptr_igmp_snp->cfg_info.static_router[0] & (0x01 << i))) && (0 != (ptr_mrtr->portbmp[0] & (0x01 << (i + 1)))))
            {
                igmp_snp_port_delMrouterPort((void **)&ptr_mrtr, (i + 1));
                /* mrouter may have been deleted */
                if(NULL == ptr_mrtr)
                {
                    break;
                }
            }
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_port_isMrouter
 * PURPOSE:
 *      This API is used to check whether the given port is a mrouter port or not.
 *
 * INPUT:
 *      port_id      --  Port number to check
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If the port is mrouter port, MW_E_OK will be returned, otherwise MW_E_OTHERS will be returned.
 */
MW_ERROR_NO_T
igmp_snp_port_isMrouter(
    UI32_T port_id)
{
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    if(PLAT_CPU_PORT == port_id)
    {
        return MW_E_OTHERS;
    }
    CSLIST_FOREACH(ptr_mrtr, &ptr_igmp_snp->mrouter_head, next)
    {
        if(TRUE == MW_PORT_CHK(ptr_mrtr->portbmp, port_id))
        {
            return MW_E_OK;
        }
    }

    return MW_E_OTHERS;
}

/* FUNCTION NAME:   igmp_snp_port_clearEntry
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry with port_id.
 *
 * INPUT:
 *      port_id
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
igmp_snp_port_clearEntry(
    UI32_T port_id)
{
#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T                          *ptr_update_payload = NULL, *ptr_delete_payload = NULL;
    UI16_T                         update_msg_size = 0, delete_msg_size = 0;
    DB_MSG_T                       *ptr_update_multi_msg = NULL, *ptr_delete_multi_msg = NULL;
    DB_REQUEST_TYPE_T              request =
    {
        .t_idx = PORT_OPER_INFO,
        .f_idx = PORT_OPER_ROUTER,
        .e_idx = port_id
    };
    UI8_T                          state = 0;
    UI32_T                         static_mrouter_bmp = 0;
#else
    IGMP_SNP_L2_MC_ENTRY_T         l2mc;
#endif
    UI8_T                          b_mrouter_port_is_clear = FALSE;
    UI32_T                         vlan_member = 0;
    MW_ERROR_NO_T                  rc = MW_E_OK;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr_next = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("clear port %d", port_id);
    if(0 == port_id)
    {
        return MW_E_BAD_PARAMETER;
    }

#ifdef IGMP_SNP_MW_SUPPORT
    ptr_update_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, sizeof(UI8_T), &update_msg_size, &ptr_update_payload);
    if (NULL == ptr_update_multi_msg)
    {
        IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
        return MW_E_NO_MEMORY;
    }
    /* Bit 0 in the IGMP ROUTER bitmap represents port 1 not port 0, refer to DB table PORT_CFG_INFO, PORT_IGMP_ROUTER */
    static_mrouter_bmp = (ptr_igmp_snp->cfg_info.static_router[0] << 1);
#endif

    CSLIST_FOREACH_SAFE(ptr_mrtr, ptr_mrtr_next, &ptr_igmp_snp->mrouter_head, next)
    {
        /* dynamic router port */
        if((TRUE == MW_PORT_CHK(ptr_mrtr->portbmp, port_id)) &&
            (IGMP_SNP_STATIC_MROUTER != ptr_mrtr->timer[(port_id - 1)]) &&
            (0 != ptr_mrtr->timer[(port_id - 1)]))
        {
            IGMP_SNP_LOG_DEBUG("delete mrouter port, vid:%d, port %d.", ptr_mrtr->vlan_id, port_id);
            ptr_mrtr->timer[(port_id - 1)] = 0;
            MW_PORT_DEL(ptr_mrtr->portbmp, port_id);
#ifdef IGMP_SNP_MW_SUPPORT
            if(FALSE == b_mrouter_port_is_clear)
            {
                rc = dbapi_appendMsgPayload(&request, &state, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
                if (MW_E_OK != rc)
                {
                    MW_FREE(ptr_update_multi_msg);
                    return MW_E_OP_INCOMPLETE;
                }
            }
#endif
            b_mrouter_port_is_clear = TRUE;
            if (0 == ptr_mrtr->portbmp[0])
            {
                IGMP_SNP_LOG_DEBUG("remove mrouter entry");
                CSLIST_REMOVE(&ptr_igmp_snp->mrouter_head, ptr_mrtr, IGMP_SNP_MROUTER_LIST_S, next);
                MW_FREE(ptr_mrtr);
            }
        }
    }

#ifdef IGMP_SNP_MW_SUPPORT
    ptr_delete_multi_msg = dbapi_createMsg(NULL, M_DELETE, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T), &delete_msg_size, &ptr_delete_payload);
    if (NULL == ptr_delete_multi_msg)
    {
        IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
        MW_FREE(ptr_update_multi_msg);
        return MW_E_NO_MEMORY;
    }
#endif

    CSLIST_FOREACH_SAFE(ptr_grp, ptr_grp_next, &ptr_igmp_snp->group_head, next)
    {
        vlan_member = igmp_snp_vlan_getMember(ptr_grp->vlan_id);
        if (MW_PORT_CHK(ptr_grp->portbmp, port_id) == TRUE)
        {
            IGMP_SNP_LOG_DEBUG("delete group entry, vid:%d, port %d", ptr_grp->vlan_id, port_id);
            MW_PORT_DEL(ptr_grp->portbmp, port_id);
            ptr_grp->timer[(port_id - 1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
            ptr_igmp_snp->port_group_num[(port_id - 1)]--;
#endif
            if (0 == ptr_grp->portbmp[0])
            {
#ifdef IGMP_SNP_MW_SUPPORT
                rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_DELETE, &ptr_delete_multi_msg, &delete_msg_size, &ptr_delete_payload);
                igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
                IGMP_SNP_LOG_DEBUG("delete group entry OK, db_idx %d", ptr_grp->db_idx);
#else
                osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
                igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
                igmp_snp_syncd_updateGroupEntry(M_DELETE, FALSE, (void *) &l2mc);
#endif
                CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
                MW_FREE(ptr_grp);
            }
            else
            {
#ifdef IGMP_SNP_MW_SUPPORT
                rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
#else
                osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
                igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
                l2mc.update_flag = TRUE;
                igmp_snp_syncd_updateGroupEntry(M_UPDATE, FALSE, (void *) &l2mc);
#endif /* IGMP_SNP_MW_SUPPORT */
            }
        }
        else if((TRUE == b_mrouter_port_is_clear) && (0 != (vlan_member & (1 << port_id))))
        {
#ifdef IGMP_SNP_MW_SUPPORT
            /* The dynamic mrouter port has been cleard and the group entry under the same VLAN needs to be updated */
            rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
#else
            osapi_memset(&l2mc, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T));
            igmp_snp_convertGroupEntryToL2mcEntry((void *)&l2mc, (void *)ptr_grp);
            l2mc.update_flag = TRUE;
            igmp_snp_syncd_updateGroupEntry(M_UPDATE, FALSE, (void *) &l2mc);
#endif /* IGMP_SNP_MW_SUPPORT */
        }
#ifdef IGMP_SNP_MW_SUPPORT
        else if((0 != (static_mrouter_bmp & (1 << port_id))) &&
                (0 != (vlan_member & (1 << port_id))))
        {
            /* port is static mrouter port, update all group entry in the same VLAN as the port */
            IGMP_SNP_LOG_DEBUG("clear static mrouter port form group entry");
            rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
        }

        if(MW_E_OK != rc)
        {
            MW_FREE(ptr_update_multi_msg);
            MW_FREE(ptr_delete_multi_msg);
            IGMP_SNP_LOG_ERROR("Append msg payload failed, rc:%d", rc);
            return MW_E_OP_INCOMPLETE;
        }
#endif /* IGMP_SNP_MW_SUPPORT */
    }

#ifdef IGMP_SNP_MW_SUPPORT
    if(0 != ptr_update_multi_msg->type.count)
    {
        IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_update_multi_msg->type.count);
        rc = dbapi_sendMsg(ptr_update_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_update_multi_msg);
    }

    if(0 != ptr_delete_multi_msg->type.count)
    {
        IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_delete_multi_msg->type.count);
        rc |= dbapi_sendMsg(ptr_delete_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_delete_multi_msg);
    }
#endif /* IGMP_SNP_MW_SUPPORT */

    return rc;
}

/* FUNCTION NAME:   igmp_snp_port_transferEntry
 * PURPOSE:
 *      This API is used to transfer port entry from one port to another port.
 *
 * INPUT:
 *      add_port     --  to add port
 *      del_port     --  to delete port
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
igmp_snp_port_transferEntry(
    UI32_T add_port,
    UI32_T del_port)
{
#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T                          state = ENABLE;
    UI8_T                          b_mrouter_port_is_update = FALSE;
    UI8_T                          *ptr_update_payload = NULL, *ptr_delete_payload = NULL;
    UI16_T                         update_msg_size = 0, delete_msg_size = 0;
    DB_MSG_T                       *ptr_update_multi_msg = NULL, *ptr_delete_multi_msg = NULL;
    DB_REQUEST_TYPE_T              request =
    {
        .t_idx = PORT_OPER_INFO,
        .f_idx = PORT_OPER_ROUTER,
        .e_idx = 0
    };
    MW_ERROR_NO_T                  rc = MW_E_OK;
    IGMP_SNP_LIST_T                *ptr_igmp_snp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr_next = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    if((add_port == del_port) || (0 == del_port) || (0 == add_port))
    {
        return MW_E_BAD_PARAMETER;
    }
    IGMP_SNP_LOG_DEBUG("add_port %d, del_port %d", add_port, del_port);

    ptr_update_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, sizeof(UI8_T), &update_msg_size, &ptr_update_payload);
    if (NULL == ptr_update_multi_msg)
    {
        IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
        return MW_E_NO_MEMORY;
    }
    CSLIST_FOREACH_SAFE(ptr_mrtr, ptr_mrtr_next, &ptr_igmp_snp->mrouter_head, next)
    {
        if(TRUE == MW_PORT_CHK(ptr_mrtr->portbmp, del_port))
        {
            if((0 != ptr_mrtr->timer[(del_port - 1)]) && (IGMP_SNP_STATIC_MROUTER != ptr_mrtr->timer[(del_port - 1)]))
            {
                IGMP_SNP_LOG_DEBUG("delete mrouter port %d, set ptr_mrtr->timer[%d] = 0", del_port, del_port);
                if(MW_E_OK != igmp_snp_port_isMrouter(add_port))
                {
                    /* port is not mrouter port, set state to enable */
                    state = ENABLE;
                    request.e_idx = add_port;
                    rc = dbapi_appendMsgPayload(&request, &state, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
                }
                MW_PORT_ADD(ptr_mrtr->portbmp, add_port);
                ptr_mrtr->timer[(add_port - 1)] = ptr_mrtr->timer[(del_port - 1)];

                ptr_mrtr->timer[(del_port - 1)] = 0;
                MW_PORT_DEL(ptr_mrtr->portbmp, del_port);
                if(MW_E_OK != igmp_snp_port_isMrouter(del_port))
                {
                    /* port is not mrouter port, send update message to db */
                    state = DISABLE;
                    request.e_idx = del_port;
                    rc |= dbapi_appendMsgPayload(&request, &state, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
                }
                b_mrouter_port_is_update = TRUE;
                if (0 == ptr_mrtr->portbmp[0])
                {
                    IGMP_SNP_LOG_DEBUG("remove mrouter entry, vid:%d", ptr_mrtr->vlan_id);
                    CSLIST_REMOVE(&ptr_igmp_snp->mrouter_head, ptr_mrtr, IGMP_SNP_MROUTER_LIST_S, next);
                    MW_FREE(ptr_mrtr);
                }
            }

            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_update_multi_msg);
                return MW_E_OP_INCOMPLETE;
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
        if(TRUE == MW_PORT_CHK(ptr_grp->portbmp, del_port))
        {
            IGMP_SNP_LOG_DEBUG("delete port %d, set ptr_grp->timer[portId=%d] = 0", del_port, del_port);
            if((0 != ptr_grp->timer[(del_port - 1)]) && (FALSE == MW_PORT_CHK(ptr_grp->portbmp, add_port)))
            {
                MW_PORT_ADD(ptr_grp->portbmp, add_port);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                ptr_igmp_snp->port_group_num[(add_port - 1)]++;
#endif
                ptr_grp->timer[(add_port - 1)] = ptr_grp->timer[(del_port - 1)];
            }
            MW_PORT_DEL(ptr_grp->portbmp, del_port);
            ptr_grp->timer[(del_port - 1)] = 0;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
            ptr_igmp_snp->port_group_num[(del_port - 1)]--;
#endif
            if (0 == ptr_grp->portbmp[0])
            {
                rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_DELETE, &ptr_delete_multi_msg, &delete_msg_size, &ptr_delete_payload);
                igmp_snp_db_freeDbIdx(ptr_grp->db_idx);
                CSLIST_REMOVE(&ptr_igmp_snp->group_head, ptr_grp, IGMP_SNP_GROUP_LIST_S, next);
                IGMP_SNP_LOG_DEBUG("Delete group entry OK, db_idx %d", ptr_grp->db_idx);
                MW_FREE(ptr_grp);
            }
            else
            {
                rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
            }
        }
        else if((TRUE == b_mrouter_port_is_update) && (0 != (igmp_snp_vlan_getMember(ptr_grp->vlan_id) & (1 << del_port))))
        {
            /* The dynamic mrouter port has been updated and the group entry under the same VLAN needs to be updated */
            rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_update_multi_msg, &update_msg_size, &ptr_update_payload);
        }
        if(MW_E_OK != rc)
        {
            IGMP_SNP_LOG_ERROR("Append msg payload failed, rc:%d\n", rc);
            MW_FREE(ptr_update_multi_msg);
            MW_FREE(ptr_delete_multi_msg);
            return MW_E_OP_INCOMPLETE;
        }
    }

    if(0 != ptr_update_multi_msg->type.count)
    {
        IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_update_multi_msg->type.count);
        rc = dbapi_sendMsg(ptr_update_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_update_multi_msg);
    }

    if(0 != ptr_delete_multi_msg->type.count)
    {
        IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_delete_multi_msg->type.count);
        rc |= dbapi_sendMsg(ptr_delete_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_delete_multi_msg);
    }
#endif /* IGMP_SNP_MW_SUPPORT */

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_port_updateEntry
 * PURPOSE:
 *      This API is used to update port's group entry.
 *
 * INPUT:
 *      portbmp       --  Port bit map of to be update.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_updateEntry(
    MW_PORT_BITMAP_T portbmp)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T                           *ptr_payload = NULL;
    UI8_T                           b_mrouter_port_is_update = FALSE;
    UI16_T                          msg_size = 0;
    UI32_T                          vlan_member = 0;
    DB_MSG_T                        *ptr_multi_msg = NULL;
    MW_PORT_BITMAP_T                to_update_portmbr = {0};
    struct IGMP_SNP_GROUP_LIST_S    *ptr_grp = NULL;
    struct IGMP_SNP_MROUTER_LIST_S  *ptr_mrtr = NULL;
    IGMP_SNP_LIST_T    *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp);
    IGMP_SNP_LOG_DEBUG("To update portbmp:0x%x", portbmp[0]);
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, sizeof(IGMP_SNP_L2_MC_ENTRY_T), &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        IGMP_SNP_LOG_ERROR("No memory for dbapi_createMsg!\n");
        return MW_E_NO_MEMORY;
    }

    CSLIST_FOREACH(ptr_mrtr, &ptr_igmp_snp->mrouter_head, next)
    {
        MW_PORT_BITMAP_COPY(to_update_portmbr, ptr_mrtr->portbmp);
        MW_BITMAP_AND(to_update_portmbr, portbmp, MW_PORT_BITMAP_SIZE);
        if(FALSE == AIR_PORT_BITMAP_EMPTY(to_update_portmbr))
        {
            b_mrouter_port_is_update = TRUE;
            IGMP_SNP_LOG_DEBUG("Mrouter port is update");
            break;
        }
    }

    CSLIST_FOREACH(ptr_grp, &ptr_igmp_snp->group_head, next)
    {
        vlan_member = igmp_snp_vlan_getMember(ptr_grp->vlan_id);
        MW_PORT_BITMAP_COPY(to_update_portmbr, ptr_grp->portbmp);
        MW_BITMAP_AND(to_update_portmbr, portbmp, MW_PORT_BITMAP_SIZE);
        if(FALSE == AIR_PORT_BITMAP_EMPTY(to_update_portmbr))
        {
            rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_multi_msg, &msg_size, &ptr_payload);
        }
        else if((TRUE == b_mrouter_port_is_update) && (0 != (vlan_member & portbmp[0])))
        {
            /* The mrouter port has been updated and the group entry under the same VLAN needs to be updated */
            rc = igmp_snp_db_appendMsgPayload((void *)ptr_grp, M_UPDATE, &ptr_multi_msg, &msg_size, &ptr_payload);
        }

        if(MW_E_OK != rc)
        {
            MW_FREE(ptr_multi_msg);
            IGMP_SNP_LOG_ERROR("Append msg payload failed, rc:%d\n", rc);
            return MW_E_OP_INCOMPLETE;
        }
    }

    if(0 != ptr_multi_msg->type.count)
    {
        IGMP_SNP_LOG_INFO("Send DB message, req_count:%d", ptr_multi_msg->type.count);
        rc = dbapi_sendMsg(ptr_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }
#endif /* IGMP_SNP_MW_SUPPORT */

    return rc;
}

/* FUNCTION NAME:   igmp_snp_port_getPortBmpLinkStatus
 * PURPOSE:
 *      This API is used to update port member based on port link staus.
 *
 * INPUT:
 *      PortMap   --  port bit map of to get link status.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map meaning the port status.
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_port_getPortBmpLinkStatus(
    UI32_T PortMap)
{
    UI32_T port = 0;
    UI32_T newPortMap = PortMap;
    AIR_PORT_STATUS_T ps;
    AIR_ERROR_NO_T    rc = AIR_E_OTHERS;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            continue;
        }
        if(0 != (PortMap & BIT(port)))
        {
#ifdef AIR_SUPPORT_SFP
            if ((TRUE == sfp_port_is_serdesPort(0, port)) ||
                (TRUE == sfp_port_is_comboPort(0, port)))
            {
                rc = sfp_port_getPortStatus(0, port, &ps);
            }
            else
#endif /* AIR_SUPPORT_SFP */
            {
                rc = air_port_getPortStatus(0, port, &ps);
            }

            if((AIR_E_OK == rc) && (0 == (ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)))
            {
                newPortMap &= ~BIT(port);
            }
        }
    }
    IGMP_SNP_LOG_DEBUG("PortMap:0x%x, newPortMap:0x%x", PortMap, newPortMap);
    return newPortMap;
}

