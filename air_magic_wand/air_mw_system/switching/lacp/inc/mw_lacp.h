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

/* FILE NAME:  mw_lacp.h
 * PURPOSE:
 *      It provides mw lacp functions.
 *
 * NOTES:
 */

#ifndef _MW_LACP_H_
#define _MW_LACP_H_

#ifdef AIR_SUPPORT_LACP
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "db_api.h"
#include "mw_msg.h"
#include "ieee8023ad_lacp.h"
#include "cmd_interpreter.h"
/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define LACP_MODULE             "lacp"
#define LACP_TIMER              "lacp_timer"
#define LACP_TIMER_INTERVAL     (1000)
#define LACP_TIMER_QUEUE_LEN    (1)
#define LACP_TIMER_MSG_ID_SM    (MW_MSG_ID_WELL_KNOWN_BASE + 1) /* Timer message ID for state machine */
#define LACP_TIMER_MSG_ID_SD    (LACP_TIMER_MSG_ID_SM + 1)      /* Timer message ID for suppress distributing */
#if LACP_MUTEX
#define LACP_MUTEX_NAME         "LACP_MUTEX"
#define LACP_MUTEX_WAITTIME     (0xFFFFFFFF)
#endif
/* DATA TYPE DECLARATIONS
*/
typedef struct lacp_tmr_msg {
    UI8_T msg_id;
    UI32_T param;
} lacp_tmr_msg_t;
/* GLOBAL VARIABLE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void
lacp_timer_suppress_distributing(
    timehandle_t Timer);

MW_ERROR_NO_T
lag_lacp_timer_init(
    void);

#if LACP_MUTEX
MW_ERROR_NO_T
lag_lacp_mutex_init(
    void);

MW_ERROR_NO_T
lag_lacp_mutex_take(
    void);

MW_ERROR_NO_T
lag_lacp_mutex_give(
    void);
#endif

MW_ERROR_NO_T
lag_lacp_destroy_port(
    UI16_T port);

MW_ERROR_NO_T
lag_lacp_create_group(
    UI8_T idx,
    UI8_T mode,
    UI32_T member);

MW_ERROR_NO_T
lag_lacp_destroy_group(
    UI8_T idx);

void
lag_sort_lacp_ports(
    UI8_T group_idx);

void
lag_lacp_acl_init(
    void);

MW_ERROR_NO_T
lag_lacp_add_acl(
    UI32_T port_bmp);

MW_ERROR_NO_T
lag_lacp_del_acl(
    UI32_T port_bmp);

BOOL_T
lag_lacp_get_timer_flag(
    void);

void
lag_lacp_set_timer_flag(
    BOOL_T flag);

MW_ERROR_NO_T
mw_cmd_lacp_show_info(
    void);

#endif
#endif