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
#include "igmp_querier_db.h"
#include "igmp_snoop_db.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_log.h"
#include "igmp_querier.h"
#include "igmp_querier_state_machine.h"
#include "vlan_utils.h"

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
/* FUNCTION NAME:   igmp_querier_db_parseMsg
 * PURPOSE:
 *      This is IGMP querier handle DB data function.
 *
 * INPUT:
 *      method            --  the method bitmap
 *      request           --  request type which includes table-id, field-id and entry-id
 *      ptr_data          --  pointer to message data
 *      data_size         --  size of ptr_data
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
igmp_querier_db_parseMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T                           index = 0, state = 0, entry_num = 0;
    UI16_T                          db_idx = 0, vlan_id = IGMP_SNP_MIN_VID;
    MW_IPV4_T                       ip_addr = 0;
    ip4_addr_t                      querier_addr;
    struct IGMP_QUERIER_LIST_S      *ptr_querier = NULL;

    MW_CHECK_PTR(ptr_data);
    IGMP_SNP_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                                request.f_idx,
                                                request.e_idx,
                                                data_size,
                                                method);

    entry_num = ((DB_ALL_ENTRIES == request.e_idx) ? MAX_VLAN_ENTRY_NUM : 1);
    switch(request.f_idx)
    {
        case IGMP_SNP_QUERIER_VLAN_ID:
            for(index = 0; index < entry_num; index++)
            {
                db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (index + 1): request.e_idx);
                osapi_memcpy(&vlan_id, ((UI16_T *)ptr_data + index), sizeof(UI16_T));
                /* Update */
                if(TRUE == VLAN_IS_VID_VALID(vlan_id))
                {
                    igmp_querier_init(vlan_id);
                    ptr_querier = igmp_querier_getOperQuerierInfo(vlan_id);
                    if(NULL != ptr_querier)
                    {
                        ptr_querier->db_idx = db_idx;
                    }
                }
                /* Delete */
                else if(IGMP_SNP_MIN_VID == vlan_id)
                {
                    vlan_id = igmp_querier_getVidByDbIdx(db_idx);
                    igmp_querier_deInit(vlan_id);
                }
            }
            break;

        case IGMP_SNP_QUERIER_STATE:
        case IGMP_SNP_QUERIER_ELECTION:
        case IGMP_SNP_QUERIER_VERSION:
            for(index = 0; index < entry_num; index++)
            {
                db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (index + 1): request.e_idx);
                osapi_memcpy(&state, ((UI8_T*)ptr_data + index), sizeof(UI8_T));
                if ((ENABLE == state) || (DISABLE == state))
                {
                    vlan_id = igmp_querier_getVidByDbIdx(db_idx);
                    if(FALSE == VLAN_IS_VID_VALID(vlan_id))
                    {
                        continue;
                    }
                    if(IGMP_SNP_QUERIER_STATE == request.f_idx)
                    {
                        /* igmp snoop querier enable/disable */
                        igmp_querier_setAdminState(vlan_id, state);
                    }
                    else if(IGMP_SNP_QUERIER_ELECTION == request.f_idx)
                    {
                        /* igmp snoop querier election enable/disable */
                        igmp_querier_setElectMode(vlan_id, state);
                    }
                    else
                    {
                        /* igmp snoop querier version v3/v2 */
                        igmp_querier_SetVersion(vlan_id, state);
                    }

                }
            }
            break;

        case IGMP_SNP_QUERIER_ADDRESS:
            for(index = 0; index < entry_num; index++)
            {
                db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (index + 1): request.e_idx);
                osapi_memcpy(&ip_addr, ((MW_IPV4_T *)ptr_data + index), sizeof(MW_IPV4_T));
                ip4_addr_set_u32(&querier_addr, ip_addr);
                vlan_id = igmp_querier_getVidByDbIdx(db_idx);
                if((TRUE == ip4_addr_isany_val(querier_addr)) || (FALSE == VLAN_IS_VID_VALID(vlan_id)))
                {
                    continue;
                }
                igmp_querier_setQuerierAddr(vlan_id, querier_addr);
            }
            break;

        default:
            break;

    }

    return MW_E_OK;
}

/* FUNCTION NAME: igmp_querier_db_subscribeModule
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
igmp_querier_db_subscribeModule(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* Check DB is ready or not */
    do
    {
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_STATE, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_ELECTION, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VERSION, DB_ALL_ENTRIES, NULL);
    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_ADDRESS, DB_ALL_ENTRIES, NULL);
    return MW_E_OK;
}

#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */