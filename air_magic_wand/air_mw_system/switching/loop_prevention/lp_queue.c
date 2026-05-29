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

/* FILE NAME:  lp_queue.c
 * PURPOSE:
 *    This file contains the implementation of queue for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_queue.h"
#include    "lp_log.h"
#include    "osapi_message.h"

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
static LP_QUEUE_CONTEXT_T     _lp_queue_ctx;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: lp_queue_init
 * PURPOSE:
 *      Initialize the queues for loop prevention module.
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
lp_queue_init(
    void)
{
    MW_ERROR_NO_T           ret = MW_E_OK;
    NET_FILTER_T *ptr_pkt_netf = NULL;

    osapi_memset(&_lp_queue_ctx, 0, sizeof(_lp_queue_ctx));

    /* Create a static queue for the interaction with the LP task. */
    ret = osapi_msgCreateStatic(LP_QUEUE_NAME,
                                LP_QUEUE_TOTAL_LENGTH,
                                LP_QUEUE_MSG_SIZE,
                                (UI8_T *)(_lp_queue_ctx.msgQueueStorage),
                                &(_lp_queue_ctx.msgQueueStruct));
    if (MW_E_OK != ret)
    {
        LP_LOG_DEBUG("lp queue not init.");
        return  MW_E_NOT_INITED;
    }

    ptr_pkt_netf = (NET_FILTER_T *)&(_lp_queue_ctx.pkt_netf);
    ptr_pkt_netf->handle = osapi_msgFindHandle(LP_QUEUE_NAME);
    if (NULL != ptr_pkt_netf->handle)
    {
        osapi_snprintf((C8_T *)ptr_pkt_netf->name, (MSG_MAX_NAME_LEN - 1), "%s", (C8_T *)LP_QUEUE_NAME);
        ptr_pkt_netf->protocol = PROTO_LP;
        ptr_pkt_netf->state = NET_FILTER_REGISTER;

        ret = osapi_netRegister(ptr_pkt_netf);
        if (MW_E_OK != ret)
        {
            LP_LOG_DEBUG("osapi_netRegister to %s fail(%d)", MAC_PKT_REGISTER, ret);
            return ret;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: lp_queue_deinit
 * PURPOSE:
 *      Release all allocated memory in loop prevention module.
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
lp_queue_deinit(
    void)
{
    NET_FILTER_T *ptr_pkt_netf = NULL;

    ptr_pkt_netf = (NET_FILTER_T *)&(_lp_queue_ctx.pkt_netf);
    ptr_pkt_netf->state = NET_FILTER_DEREGISTER;
    osapi_netRegister(ptr_pkt_netf);
    osapi_msgDelete(LP_QUEUE_NAME);

    return MW_E_OK;
}