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

/* FILE NAME:   igmp_snoop_querier.c
 * PURPOSE:
 *      It provides IGMP querier module API.
 * NOTES:
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_querier.h"
#include "igmp_snoop.h"
#include "igmp_snoop_db.h"
#include "vlan_utils.h"
#include "igmp_querier_utils.h"
#ifdef AIR_SUPPORT_LP
#include "lp.h"
#endif
#include "lwip/tcpip.h"
#include "igmp_snoop_log.h"

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

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   igmp_querier_init
 * PURPOSE:
 *      This is IGMP querier init function.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_querier_init(
    UI16_T  vlan_id)
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    if(FALSE == VLAN_IS_VID_VALID(vlan_id))
    {
        return MW_E_NOT_INITED;
    }
    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    if(NULL == ptr_querier)
    {
        osapi_calloc(sizeof(IGMP_QUERIER_LIST_T), IGMP_SNP_MODULE_NAME, (void**)&ptr_querier);
        if(NULL != ptr_querier)
        {
            ptr_querier->vlan_id = vlan_id;
            ptr_querier->election = ENABLE;
            ptr_igmp_snp = igmp_snp_getConfig();
            if(NULL != ptr_igmp_snp)
            {
                CSLIST_INSERT_HEAD(&(ptr_igmp_snp->querier_head), ptr_querier, next);
            }
            CSLIST_INIT(&(ptr_querier->gs_query_head));
        }
        else
        {
            IGMP_SNP_LOG_ERROR("Error: Failed to allocated memory for querier_list!");
            return MW_E_NO_MEMORY;
        }
    }
    else
    {
        IGMP_SNP_LOG_WARN("Warning: querier has been inited, vid:%d", vlan_id);
        return MW_E_ALREADY_INITED;
    }
    IGMP_SNP_LOG_INFO("init querier, vlan_id:%d, rc:%d", vlan_id, rc);
    return rc;
}

/* FUNCTION NAME:   igmp_querier_deInit
 * PURPOSE:
 *      This is IGMP querier deinit function.
 *
 * INPUT:
 *      vlan_id           --  ingress VLAN ID
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
igmp_querier_deInit(
    UI16_T vlan_id)
{
    UI16_T                                  db_idx = 0;
    IGMP_SNP_LIST_T                         *ptr_igmp_snp = NULL;
    IGMP_QUERIER_INFO_T                 *ptr_query_entry = NULL;
    struct IGMP_QUERIER_LIST_S          *ptr_querier = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query = NULL;
    struct IGMP_QUERIER_GS_QUERY_LIST_S *ptr_gs_query_next = NULL;

    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
    ptr_igmp_snp = igmp_snp_getConfig();
    if((NULL == ptr_querier) || (NULL == ptr_igmp_snp))
    {
        return MW_E_OK;
    }
    db_idx = ptr_querier->db_idx;
    CSLIST_FOREACH_SAFE(ptr_gs_query, ptr_gs_query_next, &(ptr_querier->gs_query_head), next)
    {
        CSLIST_REMOVE(&(ptr_querier->gs_query_head), ptr_gs_query, IGMP_QUERIER_GS_QUERY_LIST_S, next);
        MW_FREE(ptr_gs_query);
    }
    CSLIST_INIT(&(ptr_querier->gs_query_head));

    CSLIST_REMOVE(&(ptr_igmp_snp->querier_head), ptr_querier, IGMP_QUERIER_LIST_S, next);
    MW_FREE(ptr_querier);

    /* Update DB to its default value */
    osapi_calloc(sizeof(IGMP_QUERIER_INFO_T), IGMP_SNP_MODULE_NAME, (void**)&ptr_query_entry);
    if(NULL != ptr_query_entry)
    {
        ptr_query_entry->election = TRUE;
        igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_QUERIER_INFO, DB_ALL_FIELDS, db_idx, ptr_query_entry, IGMP_SNP_SEMA_DELAY);
        MW_FREE(ptr_query_entry);
    }
    IGMP_SNP_LOG_DEBUG("deinit qierier, vlan_id:%d, db_idx:%d", vlan_id, db_idx);
    return MW_E_OK;
}

#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

