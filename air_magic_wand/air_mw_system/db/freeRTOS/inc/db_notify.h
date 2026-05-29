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

/* FILE NAME:  db_notify.h
 * PURPOSE:
 *      Database subscribers and notifications handling functions
 *
 * NOTES:
 */

#ifndef DB_NOTIFY_H
#define DB_NOTIFY_H

/* INCLUDE FILE DECLARATIONS
*/
#include "db_api.h"
#include "db_util.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
/* Client link-list of the subscription entity */
typedef struct DB_SUB_CLIENT_S
{
    C8_T                     cq_name[DB_Q_NAME_SIZE];  /* The queue name of client */
    struct DB_SUB_CLIENT_S   *c_next;           /* The next client */
}DB_SUB_CLIENT_T;

/* Subscription entities */
typedef struct DB_SUB_ENTITY_S
{
    DB_REQUEST_TYPE_T        key;               /* The subscription key */
    DB_SUB_CLIENT_T          *c_head;           /* The first client of the entity */
}DB_SUB_ENTITY_T;

/* Noification structure */
typedef struct DB_NOTIFY_S
{
    UI32_T                   msg_size;          /* The message size of the notification */
    DB_MSG_T                 *msg;              /* The notification message */
}DB_NOTIFY_T;

/* Multiple Updates Notification */
typedef struct DB_NOTIFY_REQUEST_S
{
    DB_REQUEST_TYPE_T        req;               /* TOBE sending requests key */
    struct DB_NOTIFY_REQUEST_S *r_next;           /* The next client */
}DB_NOTIFY_REQUEST_T;

typedef struct DB_MULTI_NOTIFY_S
{
    C8_T                     cq_name[DB_Q_NAME_SIZE];  /* The queue name of client */
    DB_NOTIFY_REQUEST_T      *r_head;           /* The first request of the notification */
    struct DB_MULTI_NOTIFY_S *n_next;           /* The next client */
}DB_MULTI_NOTIFY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: db_subscribe
 * PURPOSE:
 *      Generate the subscription entity
 *
 * INPUT:
 *      ptr_msg             --  A pointer to the request message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *      MW_E_ENTRY_EXISTS
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_subscribe(
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: db_unsubscribe
 * PURPOSE:
 *      Remove a subscription entity
 *
 * INPUT:
 *      ptr_msg             --  A pointer to the request message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_unsubscribe(
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: db_genFirstNotification
 * PURPOSE:
 *      generate the notification for first subscription
 *
 * INPUT:
 *      ptr_msg            --  The original client's request
 *
 * OUTPUT:
 *      ptr_notify         --  A pointer to the notify message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      This function generate message contained the current
 *      db data when first subscribed.
 *
 */
MW_ERROR_NO_T
db_genFirstNotification(
    DB_MSG_T *ptr_msg,
    DB_NOTIFY_T *ptr_notify);

/* FUNCTION NAME: db_genNotification
 * PURPOSE:
 *      genarate the notification for the other subscribers
 *
 * INPUT:
 *      count              --  The successed requests count or 0 indicates all
 *      ptr_msg            --  The original client's request
 *      ptr_updatedIdx     --  A pointer to the updated index array
 *
 * OUTPUT:
 *      ptr_notify         --  A pointer to the notify message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      This function generate the notification message
 *
 */
MW_ERROR_NO_T
db_genNotification(
    const UI8_T count,
    DB_MSG_T *ptr_msg,
    DB_NOTIFY_T *ptr_notify,
    UI8_T *ptr_updatedIdx);

/* FUNCTION NAME: db_notificationsHandle
 * PURPOSE:
 *      Search the subscribers of each request
 *
 * INPUT:
 *      ptr_notify              -- The updated data message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Will try to search the other subscriber that matched or in the range.
 *
 */
MW_ERROR_NO_T
db_notificationsHandle(
    DB_NOTIFY_T *ptr_notify);

/* FUNCTION NAME: db_dumpSubTree
 * PURPOSE:
 *      Dump the DB current sub tree
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
db_dumpSubTree(
    void);

#endif  /* End of DB_NOTIFY_H */
