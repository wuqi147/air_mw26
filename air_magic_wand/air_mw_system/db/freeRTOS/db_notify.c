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

/* FILE NAME:   db_notify.c
 * PURPOSE:
 *      Database subscribers and notifications handling functions
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "osapi_memory.h"
#include "db_data.h"
#include "db_notify.h"
#include "db_main.h"
#include "db_util.h"
#include "db_avl_tree.h"
#include "mw_utils.h"
/* Special client queue name */
#include "syncd_queue.h"
#if defined(AIR_SUPPORT_MQTTD)
#include "mqttd_queue.h"
#endif


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

/* LOCAL SUBPROMGRAM BODIES
*/
static void
_db_notifyFree(
    DB_MULTI_NOTIFY_T *ptr_notify)
{
    DB_NOTIFY_REQUEST_T *ptr_req = NULL;
    if (ptr_notify == NULL)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    /* free the requests list */
    ptr_req = ptr_notify->r_head;
    while (NULL != ptr_req)
    {
        ptr_notify->r_head = ptr_req->r_next;
        osapi_free(ptr_req);
        ptr_req = ptr_notify->r_head;
    }

    /* free the notify structure */
    osapi_free(ptr_notify);
}

static void
_db_notifyClientSend(
    DB_MULTI_NOTIFY_T *ptr_notifys,
    DB_MSG_T *ptr_notifyMsg)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    DB_NOTIFY_REQUEST_T *ptr_req = NULL;
    DB_REQUEST_TYPE_T req;
    DB_MSG_T *ptr_notify_msg = NULL;
    UI8_T *ptr_in_data = NULL;
    UI8_T *ptr_out_data = NULL;
    UI16_T msg_size = 0;
    UI16_T data_size = 0;
    UI8_T count = 0;

    if ((ptr_notifys == NULL) || (ptr_notifyMsg == NULL))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    /* Calculate the message size */
    ptr_req = ptr_notifys->r_head;
    while (NULL != ptr_req)
    {
        data_size = 0;
        db_getDataSize(ptr_req->req, &data_size);
        msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
        ptr_req = ptr_req->r_next;
        count++;
    }
    if (count == 0)
    {
        DB_LOG_ERROR("%s: no need to send notification", DB_INTERNAL_ERROR);
        _db_notifyFree(ptr_notifys);
        return;
    }
    msg_size += DB_MSG_HEADER_SIZE;

    /* Allocate a buffer for the message */
    ret = db_calloc(msg_size, (void **)&ptr_notify_msg);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: cannot allocate memory to notify %s", DB_INTERNAL_ERROR, ptr_notifys->cq_name);
        _db_notifyFree(ptr_notifys);
        return;
    }

    /* message header */
    memcpy(ptr_notify_msg->cq_name, ptr_notifys->cq_name, DB_Q_NAME_SIZE);
    ptr_notify_msg->method = ptr_notifyMsg->method;
    ptr_notify_msg->type.count = count;
    DB_LOG_DEBUG("Send to client [%s] method %02X count %d",ptr_notify_msg->cq_name, ptr_notify_msg->method, count);

    /* payload */
    ptr_req = ptr_notifys->r_head;
    ptr_out_data = (UI8_T *)&(ptr_notify_msg->ptr_payload);
    ptr_in_data = (UI8_T *)&(ptr_notifyMsg->ptr_payload);
    count = 0;
    while (NULL != ptr_req)
    {
        data_size = 0;
        while (count < ptr_notifyMsg->type.count)
        {
            count++;
            memcpy((void *)&req, (const void *)ptr_in_data, sizeof(DB_REQUEST_TYPE_T));
            memcpy((void *)&data_size, (const void *)(ptr_in_data + sizeof(DB_REQUEST_TYPE_T)), sizeof(data_size));
            data_size += DB_MSG_PAYLOAD_SIZE;
            if (0 == memcmp(&req, &(ptr_req->req), sizeof(DB_REQUEST_TYPE_T)))
            {
                memcpy((void *)ptr_out_data, (const void *)ptr_in_data, data_size);
                memcpy((void *)&req, (const void *)ptr_out_data, sizeof(DB_REQUEST_TYPE_T));
                ptr_out_data += data_size;
                ptr_in_data += data_size;
                break;
            }
            ptr_in_data += data_size;
        }
        ptr_req = ptr_req->r_next;
    }

    /* Send the notification if any */
    if (((void *)ptr_out_data) > ((void *)&(ptr_notify_msg->ptr_payload)))
    {
        DB_LOG_INFO("Notify client: %s with %d requests", ptr_notify_msg->cq_name, ptr_notify_msg->type.count);
        (void)db_sendNotification((C8_T *)ptr_notifys->cq_name, ptr_notify_msg);
    }
    else
    {
        DB_LOG_INFO("No data to send, free the msg %p", ptr_notify_msg);
        osapi_free(ptr_notify_msg);
    }

    /* Free the notify structure */
    _db_notifyFree(ptr_notifys);
}

static MW_ERROR_NO_T
_db_notifyClientReqAdd(
    C8_T *updater,
    DB_REQUEST_TYPE_T org_req,
    DB_REQUEST_TYPE_T find_req,
    DB_MULTI_NOTIFY_T **pptr_rootNotify)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    DB_SUB_CLIENT_T *ptr_e = NULL;
    DB_MULTI_NOTIFY_T *ptr_notify = NULL;
    DB_MULTI_NOTIFY_T *ptr_newNotify = NULL;
    DB_NOTIFY_REQUEST_T *ptr_req = NULL;
    DB_NOTIFY_REQUEST_T *ptr_newReq = NULL;
    DB_AVL_NODE_T **pptr_root= db_getDbSubTreeRoot();

    if ((updater == NULL) || (pptr_rootNotify == NULL))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return ret;
    }

    ret = db_find_client_head(pptr_root, &find_req, &ptr_e);
    if (MW_E_OK != ret)
    {
        /* No one subscribed this key */
        return MW_E_OK;
    }

    while (NULL != ptr_e)
    {
        /* Skip if the subscriber entity is the updater */
        if (0 == memcmp((const void *)ptr_e->cq_name,
                            (const void *)updater,
                            DB_Q_NAME_SIZE))
        {
            if (MW_E_OK != db_find_client_next(ptr_e, &ptr_e))
            {
                /* No next subscriber of this searching key */
                ret = MW_E_OK;
                break;
            }
            continue;
        }
        /* Search the to be notified client queue exist or not */
        ptr_notify = NULL;
        ptr_newNotify = *pptr_rootNotify;
        while (NULL != ptr_newNotify)
        {
            ptr_notify = ptr_newNotify;
            if (0 == memcmp((const void *)ptr_notify->cq_name,
                            (const void *)ptr_e->cq_name,
                            DB_Q_NAME_SIZE))
            {
                /* To be sent Client exist */
                break;
            }
            ptr_newNotify = ptr_notify->n_next;
        }
        /* If not found, then create one */
        if (ptr_newNotify == NULL)
        {
            ret = db_calloc(sizeof(DB_MULTI_NOTIFY_T), (void **)&ptr_newNotify);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: cannot allocate memory for notification", DB_INTERNAL_ERROR);
                return ret;
            }
            memcpy((void *)ptr_newNotify->cq_name, (const void *)ptr_e->cq_name, DB_Q_NAME_SIZE);
            DB_LOG_DEBUG("New notify recipient: %s", ptr_newNotify->cq_name);
            ptr_newNotify->r_head = NULL;
            ptr_newNotify->n_next = NULL;
            /* Added in queue */
            if (ptr_notify != NULL)
            {
                ptr_notify->n_next = ptr_newNotify;
            }
            else
            {
                (*pptr_rootNotify) = ptr_newNotify;
            }
        }

        /* Search the find_req exist or not */
        ptr_req = NULL;
        ptr_newReq = ptr_newNotify->r_head;
        while (NULL != ptr_newReq)
        {
            ptr_req = ptr_newReq;
            if (0 == memcmp((const void *)&(ptr_req->req),
                            (const void *)&org_req,
                            sizeof(DB_REQUEST_TYPE_T)))
            {
                /* To be sent req exist */
                ret = MW_E_OK;
                break;
            }
            ptr_newReq = ptr_req->r_next;
        }
        /* If not found, then create the request key */
        if (ptr_newReq == NULL)
        {
            ret = db_calloc(sizeof(DB_NOTIFY_REQUEST_T), (void **)&ptr_newReq);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: cannot allocate memory for notification", DB_INTERNAL_ERROR);
                if (NULL == ptr_req)
                {
                    /* free the client context if there is no request in list */
                    osapi_free(ptr_newNotify);
                }
                return ret;
            }
            memcpy((void *)&(ptr_newReq->req), (const void *)&org_req, sizeof(DB_REQUEST_TYPE_T));
            ptr_newReq->r_next = NULL;
            if (ptr_req != NULL)
            {
                ptr_req->r_next = ptr_newReq;
            }
            else
            {
                ptr_newNotify->r_head = ptr_newReq;
            }
            ret = MW_E_OK;
        }
        /* Next subscriber */
        if (MW_E_OK != db_find_client_next(ptr_e, &ptr_e))
        {
            break;
        }
    }
    return ret;
}

extern BOOL_T syncd_api_is_subscribed(UI8_T t_idx, UI8_T f_idx, UI16_T e_idx);
#if defined(AIR_SUPPORT_MQTTD)
extern BOOL_T mqttd_is_subscribed(UI8_T t_idx, UI8_T f_idx, UI16_T e_idx);
#endif

static MW_ERROR_NO_T
_db_notifySpecialClientReqAdd(
    const C8_T *ptr_client,
    const DB_REQUEST_TYPE_T org_req,
    DB_MULTI_NOTIFY_T **pptr_rootNotify)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    DB_MULTI_NOTIFY_T *ptr_notify = NULL;
    DB_MULTI_NOTIFY_T *ptr_newNotify = NULL;
    DB_NOTIFY_REQUEST_T *ptr_req = NULL;
    DB_NOTIFY_REQUEST_T *ptr_newReq = NULL;

    if ((ptr_client == NULL) || (pptr_rootNotify == NULL))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return ret;
    }

    /* Search the to be notified client queue exist or not */
    ptr_notify = NULL;
    ptr_newNotify = *pptr_rootNotify;
    while (NULL != ptr_newNotify)
    {
        ptr_notify = ptr_newNotify;
        if (0 == memcmp((const void *)ptr_notify->cq_name,
                        (const void *)ptr_client,
                        DB_Q_NAME_SIZE - 1))
        {
            /* To be sent Client exist */
            break;
        }
        ptr_newNotify = ptr_notify->n_next;
    }
    /* If not found, then create one */
    if (ptr_newNotify == NULL)
    {
        ret = db_calloc(sizeof(DB_MULTI_NOTIFY_T), (void **)&ptr_newNotify);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s: cannot allocate memory for notification", DB_INTERNAL_ERROR);
            return ret;
        }
        memcpy((void *)ptr_newNotify->cq_name, (const void *)ptr_client, DB_Q_NAME_SIZE - 1);
        ptr_newNotify->cq_name[DB_Q_NAME_SIZE - 1] = 0;
        DB_LOG_DEBUG("New notify recipient: %s", ptr_newNotify->cq_name);
        ptr_newNotify->r_head = NULL;
        ptr_newNotify->n_next = NULL;
        /* Added in queue */
        if (ptr_notify != NULL)
        {
            ptr_notify->n_next = ptr_newNotify;
        }
        else
        {
            (*pptr_rootNotify) = ptr_newNotify;
        }
    }

    /* Search the find_req exist or not */
    ptr_req = NULL;
    ptr_newReq = ptr_newNotify->r_head;
    while (NULL != ptr_newReq)
    {
        ptr_req = ptr_newReq;
        if (0 == memcmp((const void *)&(ptr_req->req),
                        (const void *)&org_req,
                        sizeof(DB_REQUEST_TYPE_T)))
        {
            /* To be sent req exist */
            ret = MW_E_OK;
            break;
        }
        ptr_newReq = ptr_req->r_next;
    }
    /* If not found, then create the request key */
    if (ptr_newReq == NULL)
    {
        ret = db_calloc(sizeof(DB_NOTIFY_REQUEST_T), (void **)&ptr_newReq);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s: cannot allocate memory for notification", DB_INTERNAL_ERROR);
            if (NULL == ptr_req)
            {
                /* free the client context if there is no request in list */
                osapi_free(ptr_newNotify);
            }
            return ret;
        }
        memcpy((void *)&(ptr_newReq->req), (const void *)&org_req, sizeof(DB_REQUEST_TYPE_T));
        ptr_newReq->r_next = NULL;
        if (ptr_req != NULL)
        {
            ptr_req->r_next = ptr_newReq;
        }
        else
        {
            ptr_newNotify->r_head = ptr_newReq;
        }
        ret = MW_E_OK;
    }
    return ret;
}

/* Notify the special client that is not in the subscriber tree */
static MW_ERROR_NO_T
_db_notifySpecialClientAdd(
    C8_T *updater,
    DB_REQUEST_TYPE_T org_req,
    DB_MULTI_NOTIFY_T **pptr_rootNotify)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    if ((updater == NULL) || (pptr_rootNotify == NULL))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    if  ((strcmp(updater, SYNCD_MSG_QUEUE_NAME)) && (syncd_api_is_subscribed(org_req.t_idx, org_req.f_idx, org_req.e_idx)))
    {
        ret = _db_notifySpecialClientReqAdd(SYNCD_MSG_QUEUE_NAME, org_req, pptr_rootNotify);
    }
    if (MW_E_OK != ret)
    {
        return ret;
    }

#if defined(AIR_SUPPORT_MQTTD)
    if ((strcmp(updater, MQTTD_QUEUE_NAME)) && (mqttd_is_subscribed(org_req.t_idx, org_req.f_idx, org_req.e_idx)))
    {
        ret = _db_notifySpecialClientReqAdd(MQTTD_QUEUE_NAME, org_req, pptr_rootNotify);
    }
#endif
    return ret;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: db_subscribe
 * PURPOSE:
 *      Generate the subscription entity
 *
 * INPUT:
 *      ptr_msg       -- A pointer to the request msg
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
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T pay_count = 0;
    DB_REQUEST_TYPE_T req;
    C8_T client[DB_Q_NAME_SIZE] = {0};
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_SUB_CLIENT_T *ptr_entity = NULL;
    DB_AVL_NODE_T **ptr_root= db_getDbSubTreeRoot();
    UI16_T sub_count = db_getDbSubCount();



    if (NULL == ptr_msg)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (!(strcmp(ptr_msg->cq_name, SYNCD_MSG_QUEUE_NAME))
#if defined(AIR_SUPPORT_MQTTD)
    || !(strcmp(ptr_msg->cq_name, MQTTD_QUEUE_NAME))
#endif
    )
    {
        /* For saving memory, only generate the sub nodes for the other tasks */
        DB_LOG_DEBUG("No need to add the special client in Sub tree: %s", ptr_msg->cq_name);
        return ret;
    }
    if (MAX_SUB_NUM <= (sub_count + 1))
    {
        DB_LOG_ERROR("the subscriptions is full: %d", MAX_SUB_NUM);
        return MW_E_NO_MEMORY;
    }

    /* Subscriber name */
    if (ptr_msg->cq_name[0] == 0)
    {
        DB_LOG_ERROR("%s", "empty client name for subscription");
        return MW_E_BAD_PARAMETER;
    }
    osapi_strncpy(client, ptr_msg->cq_name, DB_Q_NAME_SIZE);
    client[DB_Q_NAME_SIZE - 1] = '\0';

    /* Subscription count */
    pay_count = ptr_msg->type.count;
    if (0 == pay_count)
    {
        DB_LOG_ERROR("%s", "empty request payload in subscription");
        return MW_E_BAD_PARAMETER;
    }

    /* Subscription payload */
    ptr_payload = (DB_PAYLOAD_T *)(&(ptr_msg->ptr_payload));
    while (pay_count > 0)
    {
        /* Subscription should not contain any raw data */
        if (0 != ptr_payload->data_size)
        {
            DB_LOG_ERROR("%s", "with raw data in subscription");
            ret = MW_E_BAD_PARAMETER;
            break;
        }
        /* parse and verify the request type */
        memcpy((void *)&req, (const void *)&(ptr_payload->request), sizeof(DB_REQUEST_TYPE_T));
        /* Search the subscription */
        ret = db_find_client_sub(ptr_root, &req, client, &ptr_entity);
        if (MW_E_OK == ret)
        {
            DB_LOG_ERROR("%s", "the subscription existed");
            break;
        }

        /* Add to subscription structure */
        ret = db_add_sub(ptr_root, &req, client);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s", "count not add a new subscription");
            break;
        }
        /* Next */
        pay_count--;
        sub_count = db_updateDbSubCount(TRUE);
        if ((pay_count > 0) && (MAX_SUB_NUM <= (sub_count + 1)))
        {
            DB_LOG_ERROR("the subscriptions is full: %d", MAX_SUB_NUM);
            ret = MW_E_NO_MEMORY;
            break;
        }
        ptr_payload = (DB_PAYLOAD_T *)(&(ptr_payload->ptr_data));
    }

    return ret;
}

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
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T pay_count = 0;
    DB_REQUEST_TYPE_T req;
    C8_T client[DB_Q_NAME_SIZE] = {0};
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_AVL_NODE_T **ptr_root = db_getDbSubTreeRoot();
    UI16_T sub_count = db_getDbSubCount();


    if (NULL == ptr_msg)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (!(strcmp(ptr_msg->cq_name, SYNCD_MSG_QUEUE_NAME))
#if defined(AIR_SUPPORT_MQTTD)
    || !(strcmp(ptr_msg->cq_name, MQTTD_QUEUE_NAME))
#endif
    )
    {
        /* No need to remove the sub nodes for special client node since not exist */
        DB_LOG_DEBUG("No need to remove the special client in Sub tree: %s", ptr_msg->cq_name);
        return ret;
    }
    if (0 == sub_count)
    {
        DB_LOG_ERROR("the subscriptions is empty: %d", MAX_SUB_NUM);
        return MW_E_ENTRY_NOT_FOUND;
    }

    /* Unsubscriber name */
    if (ptr_msg->cq_name[0] == 0)
    {
        DB_LOG_ERROR("%s", "empty client name for unsubscription");
        return MW_E_BAD_PARAMETER;
    }
    osapi_strncpy(client, ptr_msg->cq_name, DB_Q_NAME_SIZE);
    client[DB_Q_NAME_SIZE - 1] = '\0';

    /* Unsubscription count */
    pay_count = ptr_msg->type.count;
    if (0 == pay_count)
    {
        DB_LOG_ERROR("%s", "empty request payload in subscription");
        return MW_E_BAD_PARAMETER;
    }

    /* Unsubscription payload */
    ptr_payload = (DB_PAYLOAD_T *)(&(ptr_msg->ptr_payload));
    while (pay_count > 0)
    {
        /* Unsubscription should not contain any raw data */
        if (0 != ptr_payload->data_size)
        {
            DB_LOG_ERROR("%s", "with raw data in subscription");
            ret = MW_E_BAD_PARAMETER;
            break;
        }
        /* parse and verify the request type */
        memcpy((void *)&req, (const void *)&(ptr_payload->request), sizeof(DB_REQUEST_TYPE_T));
        /* delete to subscription structure */
        ret = db_del_sub(ptr_root, &req, client);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s", "count not remove a subscription");
            break;
        }
        /* Next */
        pay_count--;
        sub_count = db_updateDbSubCount(FALSE);
        if ((pay_count > 0) && (0 == sub_count))
        {
            DB_LOG_ERROR("the subscriptions in DB is empty, current: %d", sub_count);
            ret = MW_E_ENTRY_NOT_FOUND;
            break;
        }
        ptr_payload = (DB_PAYLOAD_T *)(&(ptr_payload->ptr_data));
    }

    return ret;
}

/* FUNCTION NAME: db_notificationsHandle
 * PURPOSE:
 *      Search the subscribers of each request
 *
 * INPUT:
 *      notify              -- The updated data message
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
    DB_NOTIFY_T *ptr_notify)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI8_T field = 0;
    UI8_T count = 0;
    UI16_T entry = 0;
    UI16_T data_size = 0;
    DB_REQUEST_TYPE_T req;
    DB_REQUEST_TYPE_T org_req;
    UI8_T *ptr_in = NULL;
    DB_CTRL_T *p_ctrl = NULL;
    DB_MULTI_NOTIFY_T *ptr_rootNotify = NULL;
    DB_MULTI_NOTIFY_T *ptr_notifys = NULL;
    C8_T updater[DB_Q_NAME_SIZE] = {0};



    if ((NULL == ptr_notify) || (NULL == ptr_notify->msg))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    memcpy((void *)updater, ptr_notify->msg->cq_name, DB_Q_NAME_SIZE);
    updater[DB_Q_NAME_SIZE - 1] = '\0';
    count = ptr_notify->msg->type.count;
    ptr_in = (UI8_T *)(&(ptr_notify->msg->ptr_payload));
    while (count > 0)
    {
        /* parse the request type */
        memcpy((void *)&org_req, (const void *)ptr_in, sizeof(DB_REQUEST_TYPE_T));
        ptr_in += sizeof(DB_REQUEST_TYPE_T);
        memcpy((void *)&data_size, (const void *)ptr_in, sizeof(data_size));
        ptr_in += (sizeof(data_size) + data_size);
        count--;
        /* Search the exact subscription */
        ret = _db_notifyClientReqAdd(updater, org_req, org_req, &ptr_rootNotify);
        if (MW_E_NO_MEMORY == ret)
        {
            break;
        }
        /* For sending any notification to special client */
        ret = _db_notifySpecialClientAdd(updater, org_req, &ptr_rootNotify);
        if (MW_E_NO_MEMORY == ret)
        {
            break;
        }
        /* Search the client that subscribed whole table */
        req.t_idx = org_req.t_idx;
        if ((DB_ALL_FIELDS != org_req.f_idx) ||
            (DB_ALL_ENTRIES != org_req.e_idx))
        {
            req.f_idx = DB_ALL_FIELDS;
            req.e_idx = DB_ALL_ENTRIES;
            ret = _db_notifyClientReqAdd(updater, org_req, req, &ptr_rootNotify);
            if (MW_E_NO_MEMORY == ret)
            {
                break;
            }
        }

        /* Search the allfield and allentry subscriptions */
        if ((DB_ALL_FIELDS != org_req.f_idx) &&
            (DB_ALL_ENTRIES != org_req.e_idx))
        {
            /* subscribed all entries */
            req.f_idx = org_req.f_idx;
            req.e_idx = DB_ALL_ENTRIES;
            ret = _db_notifyClientReqAdd(updater, org_req, req, &ptr_rootNotify);
            if (MW_E_NO_MEMORY == ret)
            {
                break;
            }
            /* subscribed all fields */
            req.f_idx = DB_ALL_FIELDS;
            req.e_idx = org_req.e_idx;
            ret = _db_notifyClientReqAdd(updater, org_req, req, &ptr_rootNotify);
            if (MW_E_NO_MEMORY == ret)
            {
                break;
            }
        }

        /* Search the in-range subscriptions */
        p_ctrl = db_getDbCoreCtrl();
        p_ctrl += org_req.t_idx;
        if ((DB_ALL_FIELDS == org_req.f_idx) &&
            (DB_ALL_ENTRIES != org_req.e_idx))
        {
            memcpy((void *)&req, (const void *)&org_req, sizeof(DB_REQUEST_TYPE_T));
            for (field = 1; field <= p_ctrl->field_num; field++)
            {
                req.f_idx = field;
                ret = _db_notifyClientReqAdd(updater, org_req, req, &ptr_rootNotify);
                if (MW_E_NO_MEMORY == ret)
                {
                    break;
                }
            }
        }
        if ((DB_ALL_FIELDS != org_req.f_idx) &&
            (DB_ALL_ENTRIES == org_req.e_idx))
        {
            memcpy((void *)&req, (const void *)&org_req, sizeof(DB_REQUEST_TYPE_T));
            for (entry = 1; entry <= p_ctrl->entry_num; entry++)
            {
                req.e_idx = entry;
                ret = _db_notifyClientReqAdd(updater, org_req, req, &ptr_rootNotify);
                if (MW_E_NO_MEMORY == ret)
                {
                    break;
                }
           }
        }

        if (MW_E_NO_MEMORY == ret)
        {
            break;
        }
    }

    while (ptr_rootNotify != NULL)
    {
        ptr_notifys = ptr_rootNotify;
        ptr_rootNotify = ptr_notifys->n_next;
        /* Send the message and free the notify structure */
        _db_notifyClientSend(ptr_notifys, ptr_notify->msg);
    }


    return ret;
}

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
    DB_NOTIFY_T *ptr_notify)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_REQUEST_TYPE_T req;
    DB_CTRL_T *p_ctrl = NULL;
    DB_MSG_T *ptr_notify_msg = NULL;
    UI8_T *ptr_field = NULL;
    UI8_T *ptr_raw_data = NULL;
    UI8_T *ptr_offset = NULL;
    UI8_T *ptr_out_data = NULL;
    UI16_T total_size = 0;
    UI16_T payload_size = 0;
    UI16_T pay_count = 0;
    UI8_T field = 0;
    UI8_T field_size = 0;

    if (NULL == ptr_msg)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    /* Count the total size */
    pay_count = ptr_msg->type.count;
    /* Subscription payload */
    ptr_offset = (UI8_T *)(&(ptr_msg->ptr_payload));
    while (pay_count > 0)
    {
        payload_size = 0;
        /* parse and verify the request type */
        memcpy((void *)&req, (const void *)ptr_offset, sizeof(DB_REQUEST_TYPE_T));
        ptr_offset += sizeof(DB_REQUEST_TYPE_T);
        ret = db_getDataSize(req, &payload_size);
        if (MW_E_OK != ret)
        {
            return ret;
        }
        total_size += (DB_MSG_PAYLOAD_SIZE + payload_size);
        /* keep the size in payload */
        memcpy((void *)ptr_offset, (const void *)&payload_size, sizeof(UI16_T));
        ptr_offset += sizeof(UI16_T);
        pay_count--;
    }

    /* Generate the message body */
    total_size += DB_MSG_HEADER_SIZE;
    ptr_notify->msg_size = total_size;
    ret = db_calloc(total_size, (void **)&(ptr_notify->msg));
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: Cannot allocate %d bytes for first notification", DB_INTERNAL_ERROR, total_size);
        return ret;
    }
    ptr_notify_msg = ptr_notify->msg;
    /* copy the name */
    osapi_strncpy(ptr_notify_msg->cq_name, ptr_msg->cq_name, DB_Q_NAME_SIZE);
    ptr_notify_msg->cq_name[DB_Q_NAME_SIZE - 1] = '\0';

    /* set the method */
    ptr_notify_msg->method = M_GET;

    /* copy the payload count */
    ptr_notify_msg->type.count = ptr_msg->type.count;
    pay_count = ptr_msg->type.count;
    ptr_offset = (UI8_T *)(&(ptr_msg->ptr_payload));
    ptr_out_data = (UI8_T *)(&(ptr_notify_msg->ptr_payload));
    while (pay_count > 0)
    {
        memcpy((void *)&req, (const void *)ptr_offset, sizeof(DB_REQUEST_TYPE_T));
        memcpy((void *)&payload_size, (const void *)(ptr_offset + sizeof(DB_REQUEST_TYPE_T)), sizeof(UI16_T));

        /* copy the payload header to notification buffer */
        memcpy((void *)ptr_out_data, (const void *)ptr_offset, DB_MSG_PAYLOAD_SIZE);
        ptr_out_data += DB_MSG_PAYLOAD_SIZE;
        ptr_offset += DB_MSG_PAYLOAD_SIZE;

        /* copy the raw data to notification buffer */
        p_ctrl = db_getDbCoreCtrl();
        p_ctrl += req.t_idx;
        ptr_field = p_ctrl->fields_size;
        ptr_raw_data = p_ctrl->table_handle;
        if (DB_ALL_FIELDS == req.f_idx)
        {
            /* all table */
            if (DB_ALL_ENTRIES == req.e_idx)
            {
               memcpy(ptr_out_data, (const void *)ptr_raw_data, payload_size);
               ptr_out_data += payload_size;
            }
            else
            {
                /* all fields required with specific entry */
                for (field = 0; field < p_ctrl->field_num; field++)
                {
                    field_size = *ptr_field;
                    memcpy(ptr_out_data, (const void *)(ptr_raw_data + ((req.e_idx - 1) * field_size)), field_size);
                    ptr_out_data += field_size;
                    ptr_raw_data += (p_ctrl->entry_num * field_size);
                    ptr_field++;
                }
            }
        }
        else
        {
            /* Switch the correct field */
            field_size = *ptr_field;
            for (field = 1; field < req.f_idx; field++)
            {
                ptr_raw_data += (p_ctrl->entry_num * field_size);
                ptr_field++;
                field_size = *ptr_field;
            }
            /* Switch to specific entry if needed */
            if (DB_ALL_ENTRIES != req.e_idx)
            {
                ptr_raw_data += ((req.e_idx - 1) * field_size);
            }
            memcpy(ptr_out_data, (const void *)ptr_raw_data, payload_size);
            ptr_out_data += payload_size;
        }

        /* Next */
        pay_count--;
    }


    return MW_E_OK;
}

/* FUNCTION NAME: db_genNotification
 * PURPOSE:
 *      genarate the notification prototype of this update
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
    UI8_T *ptr_updatedIdx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_notify_msg = NULL;
    void *ptr_org_data = NULL;
    void *ptr_out_data = NULL;
    UI8_T pcount = 1;
    UI8_T updatedcount = 0;
    UI8_T pidx = 0;
    UI16_T payload_size = 0;
    UI16_T total_size = 0;



    if ((NULL == ptr_msg) || (NULL == ptr_updatedIdx))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return pcount;
    }

    /* Only pulbish valid requests */
    if (count == MW_E_OK)
    {
        pcount = ptr_msg->type.count;
    }
    else
    {
        pcount = count - 1;
    }
    DB_LOG_DEBUG("the notification req count is %d", pcount);

    /* Calculate the message size */
    ptr_org_data = (void *)&(ptr_msg->ptr_payload);
    for (pidx = 0; pidx < pcount; pidx++)
    {
        memcpy((void *)&payload_size, (const void *)(ptr_org_data + sizeof(DB_REQUEST_TYPE_T)), sizeof(UI16_T));
        ptr_org_data += (DB_MSG_PAYLOAD_SIZE + payload_size);
        if (ptr_updatedIdx[pidx] == FALSE)
        {
            continue;
        }
        total_size += (DB_MSG_PAYLOAD_SIZE + payload_size);
        updatedcount++;
    }
    total_size += DB_MSG_HEADER_SIZE;

    /* Generate the message body */
    ret = db_calloc(total_size, (void **)&(ptr_notify->msg));
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("Cannot allocate memory size (%d) for notify message", total_size);
        return 1;
    }
    ptr_notify->msg_size = total_size;
    ptr_notify_msg = ptr_notify->msg;
    /* copy the name */
    osapi_strncpy(ptr_notify_msg->cq_name, ptr_msg->cq_name, DB_Q_NAME_SIZE);
    ptr_notify_msg->cq_name[DB_Q_NAME_SIZE - 1] = '\0';

    /* set the method */
    ptr_notify_msg->method = ptr_msg->method;

    /* copy the payload count */
    ptr_notify_msg->type.count = updatedcount;

    /* copy the data to notification buffer */
    ptr_org_data = (void *)&(ptr_msg->ptr_payload);
    ptr_out_data = (void *)&(ptr_notify_msg->ptr_payload);
    for (pidx = 0; pidx < pcount; pidx++)
    {
        memcpy((void *)&payload_size, (const void *)(ptr_org_data + sizeof(DB_REQUEST_TYPE_T)), sizeof(UI16_T));
        total_size = DB_MSG_PAYLOAD_SIZE + payload_size;
        if (ptr_updatedIdx[pidx] == TRUE)
        {
            memcpy(ptr_out_data, (const void *)ptr_org_data, total_size);
            ptr_out_data += total_size;
        }
        ptr_org_data += total_size;
    }


    return MW_E_OK;
}

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
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_AVL_NODE_T **ptr_root= db_getDbSubTreeRoot();
    DB_AVL_NODE_T *p_sub = NULL;
    DB_AVL_NODE_T *p_next = NULL;
    UI16_T count = db_getDbSubCount();
    C8_T table_key[DB_MAX_KEY_SIZE] = {0};
    C8_T field_key[DB_MAX_KEY_SIZE] = {0};

    MW_CMD_OUTPUT("Dump subscription tree\n");
    MW_CMD_OUTPUT("The total subscription nodes : %d\n", count);
    MW_CMD_OUTPUT("root subscription node : 0x%p\n", *ptr_root);
    ret = db_find_sub_first(*ptr_root, &p_next);
    count = 0;
    while (MW_E_OK == ret)
    {
        count++;
        p_sub = p_next;
        p_next = NULL;
        db_cfgfile_getTableName(p_sub->node.key.t_idx, DB_MAX_KEY_SIZE, table_key);
        db_cfgfile_getFieldName(p_sub->node.key.t_idx, p_sub->node.key.f_idx, DB_MAX_KEY_SIZE, field_key);
        MW_CMD_OUTPUT("%-3d. [%10s(%2u)/%10s(%2u)/%u](%p):\n", count,
                table_key, p_sub->node.key.t_idx,
                field_key, p_sub->node.key.f_idx,
                p_sub->node.key.e_idx, p_sub);
        if (NULL == p_sub->node.c_head)
        {
            MW_CMD_OUTPUT("No client subscribe this key...\n");
        }
        else
        {
            DB_SUB_CLIENT_T *ptr_client = p_sub->node.c_head;
            UI32_T clidx = 1;
            while (ptr_client != NULL)
            {
                MW_CMD_OUTPUT(" - %2d: %s\n", clidx, ptr_client->cq_name);
                ptr_client = ptr_client->c_next;
                clidx++;
            }
        }
        ret = db_find_sub_next(p_sub, &p_next);
    }
}

