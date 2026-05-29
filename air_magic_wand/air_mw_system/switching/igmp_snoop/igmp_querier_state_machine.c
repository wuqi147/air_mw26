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

/* FILE NAME:  igmp_querier_db.c
 * PURPOSE:
 *    This file contains the implementation of IGMP querier database.
 *
 * NOTES:
 *
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_db.h"
#include "igmp_snoop_timer.h"
#include "igmp_querier.h"
#include "igmp_querier_state_machine.h"
#include "igmp_querier_utils.h"

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
_igmp_querier_disabledStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                        *ptr_param);

static MW_ERROR_NO_T
_igmp_querier_querierStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                            *ptr_param);

static MW_ERROR_NO_T
_igmp_querier_nonQuerierStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                        *ptr_param);

static MW_ERROR_NO_T
_igmp_querier_handleStartQuerierEvent(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    void                       *ptr_param);

static MW_ERROR_NO_T
_igmp_querier_handleToDisableEvent(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    void                       *ptr_param);

static void
_igmp_querier_setQuerierState(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_STATE_TYPE_T   state);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_igmp_querier_disabledStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                            *ptr_param)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    MW_CHECK_PTR(ptr_querier);
    IGMP_SNP_LOG_DEBUG("querier current state:0x%x, event:0x%x.", ptr_querier->state, event);
    switch(event)
    {
        case IGMP_QUERIER_EVENT_START_QUERIER:
            _igmp_querier_handleStartQuerierEvent(ptr_querier, ptr_param);
            break;

        default:
            break;
    }
    return rc;
}

static MW_ERROR_NO_T
_igmp_querier_querierStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                            *ptr_param)
{
    ip4_addr_t                              group_address;
    MW_ERROR_NO_T                           rc = MW_E_OK;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query_next = NULL;

    MW_CHECK_PTR(ptr_querier);
    IGMP_SNP_LOG_DEBUG("querier current state:0x%x, event:0x%x.", ptr_querier->state, event);
    switch(event)
    {
        case IGMP_QUERIER_EVENT_START_QUERIER:
            _igmp_querier_handleStartQuerierEvent(ptr_querier, ptr_param);
            break;

        case IGMP_QUERIER_EVENT_TO_NON_QUERIER:
            _igmp_querier_setQuerierState(ptr_querier, IGMP_QUERIER_STATE_NONQUERIER);
            ptr_querier->general_query_cfg.startup_query_count = 0;
            /* Clear General query timer */
            ptr_querier->general_query_cfg.querier_query_timer = 0;
            /* Clear Group specific query entry */
            CSLIST_FOREACH_SAFE(ptr_gs_query, ptr_gs_query_next, &(ptr_querier->gs_query_head), next)
            {
                CSLIST_REMOVE(&(ptr_querier->gs_query_head), ptr_gs_query, IGMP_QUERIER_GS_QUERY_LIST_S, next);
                MW_FREE(ptr_gs_query);
            }
            CSLIST_INIT(&(ptr_querier->gs_query_head));

            /* Start querier query timer */
            ptr_querier->querier_expiry_timer = (IGMP_SNP_OTHER_QUERIER_PRESENT_INTERVAL * IGMP_SNP_TMR_RATE_S);
            break;

        /* Querier Query Timer Expired */
        case IGMP_QUERIER_EVENT_QQT:
            /* send igmp query */
            IP4_ADDR(&group_address, 224, 0, 0, 1);
            igmp_querier_sendQuery(ptr_querier, group_address);
            if(0 < ptr_querier->general_query_cfg.startup_query_count)
            {
                ptr_querier->general_query_cfg.startup_query_count--;
            }
            ptr_querier->general_query_cfg.querier_query_timer = (((0 < ptr_querier->general_query_cfg.startup_query_count) ? IGMP_QUERIER_STARTUP_QUERY_INTERVAL : IGMP_SNP_QUERY_INTERVAL) * IGMP_SNP_TMR_RATE_S);
            break;

        /* Group specific query timer Expired */
        case IGMP_QUERIER_EVENT_GST:
            if(NULL != ptr_param)
            {
                ptr_gs_query = (struct IGMP_QUERIER_GS_QUERY_LIST_S *)ptr_param;
                if(0 < ptr_gs_query->last_mem_query_count)
                {
                    ip4_addr_copy(group_address, ptr_gs_query->group_addr);
                    igmp_querier_sendQuery(ptr_querier, group_address);
                    ptr_gs_query->last_mem_query_count--;
                    if(0 < ptr_gs_query->last_mem_query_count)
                    {
                        ptr_gs_query->gs_query_timer = (IGMP_SNP_LAST_MEM_QUERY_INTERVAL * IGMP_SNP_TMR_RATE_S);
                    }
                    else
                    {
                        CSLIST_REMOVE(&(ptr_querier->gs_query_head), ptr_gs_query, IGMP_QUERIER_GS_QUERY_LIST_S, next);
                        MW_FREE(ptr_gs_query);
                    }
                }
            }
            break;

        case IGMP_QUERIER_EVENT_DISABLE:
            _igmp_querier_handleToDisableEvent(ptr_querier, NULL);
            break;

        default:
            break;
    }
    return rc;
}

static MW_ERROR_NO_T
_igmp_querier_nonQuerierStateHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                            *ptr_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_CHECK_PTR(ptr_querier);
    IGMP_SNP_LOG_DEBUG("querier current state:0x%x, event:0x%x.", ptr_querier->state, event);
    switch(event)
    {
        case IGMP_QUERIER_EVENT_TO_NON_QUERIER:
            /* Update querier query timer */
            ptr_querier->querier_expiry_timer = (IGMP_SNP_OTHER_QUERIER_PRESENT_INTERVAL * IGMP_SNP_TMR_RATE_S);
            break;

        /* Querier Expired Timer Expired */
        case IGMP_QUERIER_EVENT_QET:
        case IGMP_QUERIER_EVENT_START_QUERIER:
            _igmp_querier_handleStartQuerierEvent(ptr_querier, ptr_param);
            break;

        case IGMP_QUERIER_EVENT_DISABLE:
            _igmp_querier_handleToDisableEvent(ptr_querier, NULL);
            break;

        default:
            break;
    }
    return rc;
}

static MW_ERROR_NO_T
_igmp_querier_handleStartQuerierEvent(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    void                            *ptr_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    ip4_addr_t  group_address;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query_next = NULL;

    MW_CHECK_PTR(ptr_querier);
    _igmp_querier_setQuerierState(ptr_querier, IGMP_QUERIER_STATE_QUERIER);
    ptr_querier->general_query_cfg.startup_query_count = IGMP_SNP_ROBUSTNESS;
    /* Clear General query timer */
    ptr_querier->general_query_cfg.querier_query_timer = 0;
    /* Clear Group specific query entry */
    CSLIST_FOREACH_SAFE(ptr_gs_query, ptr_gs_query_next, &(ptr_querier->gs_query_head), next)
    {
        CSLIST_REMOVE(&(ptr_querier->gs_query_head), ptr_gs_query, IGMP_QUERIER_GS_QUERY_LIST_S, next);
        MW_FREE(ptr_gs_query);
    }
    CSLIST_INIT(&(ptr_querier->gs_query_head));

    /* Clear querier expiry timer */
    ptr_querier->querier_expiry_timer = 0;

    IP4_ADDR(&group_address, 224, 0, 0, 1);
    igmp_querier_sendQuery(ptr_querier, group_address);
    ptr_querier->general_query_cfg.startup_query_count--;
    ptr_querier->general_query_cfg.querier_query_timer = (IGMP_QUERIER_STARTUP_QUERY_INTERVAL * IGMP_SNP_TMR_RATE_S);

    return rc;
}

static MW_ERROR_NO_T
_igmp_querier_handleToDisableEvent(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    void                            *ptr_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query_next = NULL;

    MW_CHECK_PTR(ptr_querier);
    _igmp_querier_setQuerierState(ptr_querier, IGMP_QUERIER_STATE_DISABLED);
    ptr_querier->general_query_cfg.startup_query_count = 0;
    /* Clear General query timer */
    ptr_querier->general_query_cfg.querier_query_timer = 0;
    /* Clear Group specific query entry */
    CSLIST_FOREACH_SAFE(ptr_gs_query, ptr_gs_query_next, &(ptr_querier->gs_query_head), next)
    {
        CSLIST_REMOVE(&(ptr_querier->gs_query_head), ptr_gs_query, IGMP_QUERIER_GS_QUERY_LIST_S, next);
        MW_FREE(ptr_gs_query);
    }
    CSLIST_INIT(&(ptr_querier->gs_query_head));

    /* Clear querier expiry timer */
    ptr_querier->querier_expiry_timer = 0;

    return rc;
}

static void
_igmp_querier_setQuerierState(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_STATE_TYPE_T   state)
{
    if(NULL != ptr_querier)
    {
        IGMP_SNP_LOG_INFO("Set querier state, ori:%x, cur:%x", ptr_querier->state, state);
        if(state != ptr_querier->state)
        {
            ptr_querier->state = state;
            igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_STATE, ptr_querier->db_idx, &state, IGMP_SNP_SEMA_DELAY);
        }
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_querier_stateMachineHandler
 * PURPOSE:
 *      This API is used to querier process event.
 *
 * INPUT:
 *      ptr_querier       --  the pointer of igmp snooping querier entry
 *      event             --  the event that triggers the igmp snooping querier to action
 *      ptr_param         --  the param associated with the event
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_stateMachineHandler(
    struct IGMP_QUERIER_LIST_S *ptr_querier,
    IGMP_QUERIER_EVENT_TYPE_T   event,
    void                        *ptr_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_CHECK_PTR(ptr_querier);
    IGMP_SNP_LOG_DEBUG("querier current state:0x%x, event:0x%x.", ptr_querier->state, event);
    switch(ptr_querier->state)
    {
        case IGMP_QUERIER_STATE_DISABLED:
            rc = _igmp_querier_disabledStateHandler(ptr_querier, event, ptr_param);
            break;
        case IGMP_QUERIER_STATE_QUERIER:
            rc = _igmp_querier_querierStateHandler(ptr_querier, event, ptr_param);
            break;
        case IGMP_QUERIER_STATE_NONQUERIER:
            rc = _igmp_querier_nonQuerierStateHandler(ptr_querier,event, ptr_param);
            break;
        default:
            break;
    }
    return rc;
}

#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
