/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  voice_vlan.h
 * PURPOSE:
 *      Voice vlan header file.
 *
 * NOTES:
 */

#ifndef VOICE_VLAN_H
#define VOICE_VLAN_H
/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "osapi_thread.h"
#include "osapi_message.h"
#include "osapi_timer.h"
#include "osapi_mutex.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "db_api.h"
#include "default_config.h"


/* NAMING CONSTANT DECLARATIONS
 */
/* Task */
#define VOICE_VLAN_TASK_NAME        "voice"
#define VOICE_VLAN_QUEUE_NAME       "vvq"
#define VOICE_VLAN_STACK_SIZE       (360)
/* DB */
#define VOICE_VLAN_QUEUE_LENGTH     (32)
#define VOICE_VLAN_MSG_SIZE         (4)
#define VOICE_VLAN_PAYLOAD_NUMBER   (1)
#define VOICE_VLAN_INVALID          (0xffff)
/* Features */
#define VOICE_VLAN_EACH_OUI_RULES   (2)
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
#define VOICE_VLAN_MAX_ACL_NUM      (MAX_OUI_NUM * VOICE_VLAN_EACH_OUI_RULES + 1)
#else
#define VOICE_VLAN_MAX_ACL_NUM      (MAX_OUI_NUM * VOICE_VLAN_EACH_OUI_RULES)
#endif
/* Rate Limit */
#define VOICE_VLAN_LIMIT_PKT_THLD    (30)
#define VOICE_VLAN_LIMIT_SPAN        (1000)
#define VOICE_VLAN_LIMIT_BLK_TIME    (1)
#define VOICE_VLAN_LIMIT_TICK        (AIR_DOS_RATE_TICKSEL_1MS)

//#define VOICE_DEBUG            (1)
/* MACRO FUNCTION DECLARATIONS
 */

#define VOICE_DBG_PRINT(fmt, ...)               \
              MW_LOG_DEBUG(VLAN, "%s:%d>" fmt "\n", __func__, __LINE__, ##__VA_ARGS__ )
#define VOICE_ERR_PRINT(fmt, ...)               \
              MW_LOG_ERROR(VLAN, "%s:%d>" fmt "\n", __func__, __LINE__, ##__VA_ARGS__ )

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: voice_vlan_deinit
 * PURPOSE:
 *      Stop voice vlan function
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
voice_vlan_deinit(
    void);

/* FUNCTION NAME: voice_vlan_init
 * PURPOSE:
 *      Start voice vlan function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
voice_vlan_init(
    void);

/* FUNCTION NAME: voice_vlan_get_status
 * PURPOSE:
 *      Show the current voice vlan state
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
voice_vlan_get_status(
    void);
#endif /* end of VOICE_VLAN_H */
