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

/* FILE NAME:  db_main.h
 * PURPOSE:
 *      Define the DB context and structures.
 *
 * NOTES:
 */

#ifndef DB_MAIN_H
#define DB_MAIN_H

/* INCLUDE FILE DECLARATIONS
*/
#include "osapi.h"
#include "osapi_string.h"
#include "osapi_thread.h"
#include "osapi_message.h"
#include "db_util.h"
#include "db_api.h"
#include "db_data.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/

/* FUNCTION NAME: db_readyFlagGet
 * PURPOSE:
 *      Get the readyFlag to show the DB task status.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      The return value will be MW_E_OK if the database is initialized,
 *      otherwise return MW_E_NOT_INITED.
 */
MW_ERROR_NO_T
db_readyFlagGet(
    void);

/* FUNCTION NAME: db_sendMsg
 * PURPOSE:
 *      Send a message to client
 *
 * INPUT:
 *      ptr_client          -- The queue name of the client
 *      ptr_msg             -- The message to be sent
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      For sliming down the MW memory footprint, the first byte of sending message will
 *      be replace to DB_Q_MSG_ID for identifying the message send from DB.
 *
 */
MW_ERROR_NO_T
db_sendMsg(
    C8_T *ptr_client,
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: db_sendNotification
 * PURPOSE:
 *      Send a notification to client
 *
 * INPUT:
 *      ptr_client         -- The queue name of the client
 *      ptr_msg            -- The notification message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_sendNotification(
    C8_T *ptr_client,
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: db_task_deinit
 * PURPOSE:
 *      Delete the DB task and test tasks
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
 *
 */
void
db_task_deinit(
    void);

/* FUNCTION NAME: db_task_init
 * PURPOSE:
 *      Create the DB task and initialize
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
 *
 */
void
db_task_init(
    void);
#endif  /* End of DB_MAIN_H */
