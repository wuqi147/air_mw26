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

/* FILE NAME:  igmp_snoop_queue.c
 * PURPOSE:
 *  It provides IGMP Snooping queue module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_queue.h"
#include "igmp_snoop_log.h"
#include "osapi_message.h"

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
static IGMP_SNP_QUEUE_CONTEXT_T     _igmp_snp_queue_ctx;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: igmp_snp_queue_init
 * PURPOSE:
 *      Initialize the queues for IGMP Snooping.
 *
 * INPUT:
 *      None
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
igmp_snp_queue_init(
    void)
{
    MW_ERROR_NO_T           ret = MW_E_OK;
    NET_FILTER_T            *ptr_pkt_netf = NULL;

    osapi_memset(&_igmp_snp_queue_ctx, 0, sizeof(_igmp_snp_queue_ctx));

    /* Create a static queue for the interaction with the IGMP SNP task. */
    ret = osapi_msgCreateStatic(IGMP_SNP_QUEUE_NAME,
                                IGMP_SNP_QUEUE_TOTAL_LENGTH,
                                IGMP_SNP_QUEUE_MSG_SIZE,
                                (UI8_T *)(_igmp_snp_queue_ctx.msgQueueStorage),
                                &(_igmp_snp_queue_ctx.msgQueueStruct));
    if(MW_E_OK != ret)
    {
        IGMP_SNP_LOG_DEBUG("igmp snp queue not init.");
        return  MW_E_NOT_INITED;
    }

    ptr_pkt_netf = (NET_FILTER_T *)&(_igmp_snp_queue_ctx.pkt_netf);
    ptr_pkt_netf->handle = osapi_msgFindHandle(IGMP_SNP_QUEUE_NAME);
    if (NULL != ptr_pkt_netf->handle)
    {
        osapi_snprintf((C8_T*)ptr_pkt_netf->name, (MSG_MAX_NAME_LEN - 1), "%s", (C8_T*)IGMP_SNP_QUEUE_NAME);
        ptr_pkt_netf->protocol = PROTO_IGMP_SNP_V4;
        ptr_pkt_netf->state = NET_FILTER_REGISTER;

        ret = osapi_netRegister(ptr_pkt_netf);
        if (MW_E_OK == ret)
        {
            IGMP_SNP_LOG_DEBUG("osapi_netRegister to %s OK", MAC_PKT_REGISTER);
        }
        else
        {
            IGMP_SNP_LOG_ERROR("osapi_netRegister to %s fail(%d)", MAC_PKT_REGISTER, ret);
            return ret;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: igmp_snp_queue_deinit
 * PURPOSE:
 *      Release all allocated memory in igmp snp queue.
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
igmp_snp_queue_deinit(
    void)
{
    NET_FILTER_T            *ptr_pkt_netf = NULL;

    ptr_pkt_netf = (NET_FILTER_T *)&(_igmp_snp_queue_ctx.pkt_netf);

    ptr_pkt_netf->state = NET_FILTER_DEREGISTER;
    if (MW_E_OK != osapi_netRegister(ptr_pkt_netf))
    {
        IGMP_SNP_LOG_DEBUG("osapi_netRegister deregister to %s failed !", MAC_PKT_REGISTER);
    }

    if (osapi_msgDelete(IGMP_SNP_QUEUE_NAME) != MW_E_OK)
    {
        IGMP_SNP_LOG_ERROR("osapi_msgDelete for %s failed !", IGMP_SNP_QUEUE_NAME);
    }

    return;
}
