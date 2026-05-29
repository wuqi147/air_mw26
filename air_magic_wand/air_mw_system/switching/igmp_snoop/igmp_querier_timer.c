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
#include "igmp_querier_timer.h"
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#include "igmp_querier.h"
#include "igmp_querier_state_machine.h"
#include "platform.h"

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
/* FUNCTION NAME:   igmp_querier_timer_handleExpirationEvent
 * PURPOSE:
 *      This API is used to
 *
 * INPUT:
 *      None
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
igmp_querier_timer_handleExpirationEvent(
    void)
{
    const UI8_T                             timer_count = 1;
    IGMP_SNP_LIST_T                         *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S              *ptr_querier =NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S     *ptr_gs_query = NULL, *ptr_gs_query_next = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if((NULL != ptr_igmp_snp) && (ENABLE == ptr_igmp_snp->cfg_info.enable))
    {
        CSLIST_FOREACH(ptr_querier, &(ptr_igmp_snp->querier_head), next)
        {
            /* Check general query timer */
            if (0 < ptr_querier->general_query_cfg.querier_query_timer)
            {
                /* Update the remaining expiration time of timer */
                if(ptr_querier->general_query_cfg.querier_query_timer > timer_count)
                {
                    ptr_querier->general_query_cfg.querier_query_timer -= timer_count;
                }
                else
                {
                    ptr_querier->general_query_cfg.querier_query_timer = 0;
                }

                if (0 == ptr_querier->general_query_cfg.querier_query_timer)
                {
                    igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_QQT, NULL);
                }
            }

            /* Check querier expiry timer */
            if (0 < ptr_querier->querier_expiry_timer)
            {
                /* Update the remaining expiration time of timer */
                if(ptr_querier->querier_expiry_timer > timer_count)
                {
                    ptr_querier->querier_expiry_timer -= timer_count;
                }
                else
                {
                    ptr_querier->querier_expiry_timer = 0;
                }
                if (0 == ptr_querier->querier_expiry_timer)
                {
                    igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_QET, NULL);
                }
            }

            /* Check Group specific query timer */
            CSLIST_FOREACH_SAFE(ptr_gs_query, ptr_gs_query_next, &(ptr_querier->gs_query_head), next)
            {
                if (0 < ptr_gs_query->gs_query_timer)
                {
                    /* Update the remaining expiration time of timer */
                    if(ptr_gs_query->gs_query_timer > timer_count)
                    {
                        ptr_gs_query->gs_query_timer -= timer_count;
                    }
                    else
                    {
                        ptr_gs_query->gs_query_timer = 0;
                    }
                    if (0 == ptr_gs_query->gs_query_timer)
                    {
                        igmp_querier_stateMachineHandler(ptr_querier, IGMP_QUERIER_EVENT_GST, ptr_gs_query);
                    }
                }
            }
        }
    }

    return;
}

#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
