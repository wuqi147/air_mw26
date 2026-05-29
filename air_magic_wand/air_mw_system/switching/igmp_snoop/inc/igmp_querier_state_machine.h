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

/* FILE NAME:  igmp_querier_state_machine.h
 * PURPOSE:
 *      This file defines the data structure for IGMP querier state machine.
 *
 * NOTES:
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER

#ifndef IGMP_QUERYER_STATE_MACHINE_H
#define IGMP_QUERYER_STATE_MACHINE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "igmp_querier_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    IGMP_QUERIER_STATE_DISABLED = 0,
    IGMP_QUERIER_STATE_QUERIER,
    IGMP_QUERIER_STATE_NONQUERIER,
    IGMP_QUERIER_STATE_LAST
}IGMP_QUERIER_STATE_TYPE_T;

typedef enum
{
    IGMP_QUERIER_EVENT_START_QUERIER = 0,
    IGMP_QUERIER_EVENT_TO_NON_QUERIER,
    IGMP_QUERIER_EVENT_QET,                 /* Querier Expiry Timer Expired */
    IGMP_QUERIER_EVENT_QQT,                 /* Querier Query Timer Expired */
    IGMP_QUERIER_EVENT_GST,                 /* Group specific query timer Expired */
    IGMP_QUERIER_EVENT_DISABLE,             /* Disable querier or Non-querier */
    IGMP_QUERIER_EVENT_LAST
}IGMP_QUERIER_EVENT_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void                        *ptr_param);

#endif /* IGMP_QUERYER_STATE_MACHINE_H */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
