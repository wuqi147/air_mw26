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

/* FILE NAME:  igmp_querier_log.c
 * PURPOSE:
 *    This file contains the implementation of IGMP querier log.
 *
 * NOTES:
 *
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_timer.h"
#include "igmp_querier_utils.h"
#include "igmp_querier_state_machine.h"
#include "igmp_querier_log.h"

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
/* FUNCTION NAME:   igmp_querier_dumpInfo
 * PURPOSE:
 *      This API is used to dump IGMP querier info.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
igmp_querier_dumpInfo()
{
    UI16_T                          i = 0;
    MW_IPV4_T                       addr = 0;
    IGMP_SNP_LIST_T                 *ptr_igmp_snp = NULL;
    struct IGMP_QUERIER_LIST_S  *ptr_querier = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if(NULL != ptr_igmp_snp)
    {
        MW_CMD_OUTPUT("\nIGMP SNOOP Querier:\n");
        CSLIST_FOREACH(ptr_querier, &(ptr_igmp_snp->querier_head), next)
        {
            MW_CMD_OUTPUT("\t[%d]: vid:%d, db_idx:%d, ",i, ptr_querier->vlan_id, ptr_querier->db_idx);
            if(IGMP_QUERIER_STATE_DISABLED == ptr_querier->state)
            {
                MW_CMD_OUTPUT("state:Disabled, ");
            }
            else if(IGMP_QUERIER_STATE_QUERIER == ptr_querier->state)
            {
                MW_CMD_OUTPUT("state:Querier, Query Timer:%d Sec, ", (ptr_querier->general_query_cfg.querier_query_timer / IGMP_SNP_TMR_RATE_S));
            }
            else
            {
                MW_CMD_OUTPUT("state:Non-querier, Querier Expiry Timer:%d Sec, ", (ptr_querier->querier_expiry_timer / IGMP_SNP_TMR_RATE_S));
            }
            MW_CMD_OUTPUT("election:%s, ", ((ENABLE == ptr_querier->election) ? "Enable" : "Disable"));
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
            MW_CMD_OUTPUT("version:%s, ", ((IGMP_QUERIER_VERSION_V2 == ptr_querier->version) ? "V2" : "V3"));
#else
            MW_CMD_OUTPUT("version:%s, ", "V2");
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
            addr = ip4_addr_get_u32(&(ptr_querier->source_address));
            MW_CMD_OUTPUT("source address:%d.%d.%d.%d\n",
                        (0xFF & ((UI8_T*)&addr)[0]),
                        (0xFF & ((UI8_T*)&addr)[1]),
                        (0xFF & ((UI8_T*)&addr)[2]),
                        (0xFF & ((UI8_T*)&addr)[3]));

            i++;
        }
    }

    return;
}

#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
