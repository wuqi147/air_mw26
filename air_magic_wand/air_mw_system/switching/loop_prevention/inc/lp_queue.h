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

/* FILE NAME:  lp_queue.h
 * PURPOSE:
 *      This file defines the data structure for loop prevention queue.
 *
 * NOTES:
 */
#ifndef LP_QUEUE_H
#define LP_QUEUE_H
/* INCLUDE FILE DECLARATIONS
 */
#include "lp.h"
#include "osapi.h"
#include "mw_msg.h"
#include "ethernet.h"
#ifdef LP_MW_SUPPORT
#include "lp_db.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define LP_QUEUE_NAME           "lpq"
#define LP_PKT_QUEUE_LENGTH     (AIR_MAX_LP_PKT_QUEUE_LEN)
#ifdef LP_MW_SUPPORT
#define LP_QUEUE_TOTAL_LENGTH   (LP_PKT_QUEUE_LENGTH + LP_DB_QUEUE_LENGTH)
#else
#define LP_QUEUE_TOTAL_LENGTH   (LP_PKT_QUEUE_LENGTH)
#endif /* LP_MW_SUPPORT */

#define LP_QUEUE_MSG_SIZE       (sizeof(MW_MSG_T *))

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    NET_FILTER_T        pkt_netf;
    staticMsghandle_t   msgQueueStruct; /* Task queue is a static queue. */
    UI8_T               msgQueueStorage[LP_QUEUE_MSG_SIZE * LP_QUEUE_TOTAL_LENGTH];
} LP_QUEUE_CONTEXT_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);

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
    void);

#endif  /* LP_QUEUE_H */
