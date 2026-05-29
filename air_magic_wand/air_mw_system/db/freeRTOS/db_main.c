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

/* FILE NAME:   db_main.c
 * PURPOSE:
 *      Database task main loop.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "db_main.h"
#include "db_api.h"
#include "db_data.h"
#include "db_notify.h"
#include "osapi_message.h"
#include "osapi_memory.h"
#include "osapi_thread.h"
#include "sys_mgmt.h"
#include "mw_msg.h"
#ifdef DB_DBG_TEST
#include "db_test.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define DB_REQUEST_Q_LEN      AIR_MAX_DB_REQUEST_LEN
#define DB_STACK_SIZE         AIR_MAX_DB_STACK_SIZE

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    threadhandle_t      ptr_dbmain;
    StaticTask_t        db_stack;
    StackType_t         db_task_stack[DB_STACK_SIZE];
    staticMsghandle_t   db_q;
    UI8_T               db_msgStorage[DB_REQUEST_Q_LEN * DB_MSG_PTR_SIZE];
} DB_CONTEXT_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void _db_dumpMsg(DB_MSG_T *ptr_msg);
static UI8_T _db_msgHandle(DB_MSG_T *ptr_msg, DB_NOTIFY_T *ptr_notify);
static void _db_releaseResource(void);
static void _db_main(void *args);

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T db_is_ready = FALSE;
static DB_CONTEXT_T _db_ctx = {0};
#ifdef DB_DBG_TEST
static threadhandle_t ptr_dbtesting1 = NULL;
static threadhandle_t ptr_dbtesting2 = NULL;
#endif

/* LOCAL SUBPROMGRAM BODIES
*/
static void
_db_dumpMsg(
    DB_MSG_T *ptr_msg)
{
    UI32_T count = 0;
    UI32_T n = 0;
#ifdef DB_DBG_VRB
    UI32_T c = 0, r = 0;
#endif
    DB_REQUEST_TYPE_T req;
    UI16_T payload_size = 0;
    UI8_T *rawdata = NULL;

#ifndef DB_DBG
    return;
#endif

    if (NULL == ptr_msg)
    {
        return;
    }
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "_db_dumpMsg:\n");

    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Client Name: %s\n", ptr_msg->cq_name);
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Method: 0x%02x ", ptr_msg->method);
    if (ptr_msg->method & M_B_RESPONSE)
    {
        /* It's a repsonse to client msg */
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Result: %d\n", ptr_msg->type.result);
        count = 1;
    }
    else
    {
        /* It's a request msg */
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Payload Count: %d\n", ptr_msg->type.count);
        count = ptr_msg->type.count;
    }

    rawdata = (UI8_T *)(&(ptr_msg->ptr_payload));
    if (NULL == rawdata)
    {
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "    NULL\n");
        return;
    }

    n = 0;
    while (n < count)
    {
        memcpy((void *)&req, (const void *)rawdata, sizeof(DB_REQUEST_TYPE_T));
        rawdata += sizeof(DB_REQUEST_TYPE_T);
        memcpy((void *)&payload_size, (const void *)rawdata, sizeof(UI16_T));
        rawdata+= sizeof(UI16_T);
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Paylod %d:\n", n);
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\tT_idx/F_idx/E_idx: %u/%u/%u\n", req.t_idx, req.f_idx, req.e_idx);
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\tRaw Data Size: %u\n", payload_size);

#ifdef DB_DBG_VRB
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\tRaw Data:\n");
        /* No raw data */
        if (0 == payload_size)
        {
            MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\tNULL\n");
            n++;
            continue;
        }
        r = 0;
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\t");
        for (c = 0; c < payload_size; c++)
        {
            MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "%02X ", *rawdata++);
            r++;
            if ((r >= 16) || (c == (payload_size - 1)))
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\n\t");
                r = 0;
            }
        }
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "\n");
#else
        rawdata += payload_size;
#endif
        n++;
    }
}

static UI8_T _db_msgHandle(
    DB_MSG_T *ptr_msg, DB_NOTIFY_T *ptr_notify)
{
    UI8_T ret = MW_E_OK;
    DB_PAYLOAD_T *data = NULL;
    UI8_T *ptr_updatedIdx = NULL;


    MW_PARAM_CHK((NULL == ptr_msg), MW_E_NOT_SUPPORT);
    /* Verify the method */
    if (ptr_msg->method & ~(M_CLIENT_REQ))
    {
        ret = MW_E_BAD_PARAMETER;
        DB_LOG_ERROR("Client[%s]: wrong method type in request", ptr_msg->cq_name);
        return ret;
    }
    /* Verify the request count */
    if (ptr_msg->type.count == 0)
    {
        ret = MW_E_BAD_PARAMETER;
        DB_LOG_ERROR("Client[%s]: request count is zero", ptr_msg->cq_name);
        return ret;
    }

    data = (DB_PAYLOAD_T *)(&(ptr_msg->ptr_payload));
    switch (ptr_msg->method) {
        case M_SUBSCRIBE:
            DB_LOG_DEBUG("Client[%s]: request a subscription", ptr_msg->cq_name);
            ret = db_subscribe(ptr_msg);
            /* if frist subscribe succeed, get the data to notify subscribers */
            if (MW_E_OK == ret)
            {
                ret = db_genFirstNotification(ptr_msg, ptr_notify);
            }
            ptr_msg->method = M_ACK;
            break;
        case M_UNSUBSCRIBE:
            DB_LOG_DEBUG("Client[%s]: request a unsubscription", ptr_msg->cq_name);
            ret = db_unsubscribe(ptr_msg);
            ptr_msg->method = M_ACK;
            break;
        case M_GET:
        case M_GETFLASH:
            DB_LOG_DEBUG("Client[%s]: request Get data", ptr_msg->cq_name);
            /* call the get data function */
            ret = db_getData(ptr_msg->type.count, ptr_msg->method, data);
            ptr_msg->method = M_RESPONSE;
            break;
        case M_CREATE:
        case M_UPDATE:
            DB_LOG_DEBUG("Client[%s]: request Update data", ptr_msg->cq_name);
            ret = db_calloc(ptr_msg->type.count, (void **)&(ptr_updatedIdx));
            if (MW_E_OK != ret)
            {
                /* Cannot allocate new memory, return the first failed index as 1 */
                ptr_msg->method = M_RESPONSE;
                ret = 1;
                break;
            }
            /* update the database */
            ret = db_setData(ptr_msg->type.count, ptr_msg->method, data, ptr_updatedIdx);
            if ((ret == MW_E_OK) ||
               (ret > 1))
            {
                /* Only notify the changed updates until the first failed index */
                ret = db_genNotification((UI8_T)ret, ptr_msg, ptr_notify, ptr_updatedIdx);
            }
            osapi_free(ptr_updatedIdx);
            ptr_msg->method = M_RESPONSE;
            break;
        case M_DELETE:
#ifdef DB_SUPPORT_RESTORE_METHOD
        case M_RESTORE:
#endif
            DB_LOG_DEBUG("Client[%s]: request %s data", ptr_msg->cq_name, (M_DELETE == ptr_msg->method) ? "Delete" : "Restore");
            ret = db_resetData(ptr_msg->method, ptr_msg, ptr_notify);
            ptr_msg->method = M_RESPONSE;
            break;
        default:
            DB_LOG_DEBUG("Client[%s]: Invalid request", ptr_msg->cq_name);
            ret = MW_E_BAD_PARAMETER;
            if ((ptr_msg->method) & M_B_SUBSCRIBED)
            {
                ptr_msg->method = M_ACK;
            }
            else
            {
                ptr_msg->method = M_RESPONSE;
            }
            break;
    }

    return ret;
}

static void _db_releaseResource(
    void)
{


    /* release the main database request queue */
    osapi_msgDelete(DB_REQUEST_Q_NAME);

    /* release the database data resource */
    db_freeData();

    return;
}

static void _db_main(
    void *args)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    C8_T client_qname[DB_Q_NAME_SIZE] = {0};
    DB_MSG_T *db_msg = NULL;
    DB_NOTIFY_T db_notify;

    db_is_ready = FALSE;

    /* Create the main database request queue */
    ret = osapi_msgCreateStatic(DB_REQUEST_Q_NAME, DB_REQUEST_Q_LEN, DB_MSG_PTR_SIZE,
                    _db_ctx.db_msgStorage, &(_db_ctx.db_q));

    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: create main message queue(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
        osapi_processDelete(_db_ctx.ptr_dbmain);
        return;
    }

    /* Get configuration data from file */
    ret = db_initData();
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: create in-memory database(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
        _db_releaseResource();
        osapi_processDelete(_db_ctx.ptr_dbmain);
        return;
    }
    db_is_ready = TRUE;

    /* main loop */
    for (;;)
    {
        db_msg = NULL;
        db_notify.msg_size = 0;
        db_notify.msg= NULL;
        memset(client_qname, 0, DB_Q_NAME_SIZE);
        /* Receive the messages from client */
        if (FALSE == db_is_ready)
        {
            osapi_delay(5);
            continue;
        }
        ret = osapi_msgRecv(DB_REQUEST_Q_NAME, (UI8_T **)&db_msg, DB_MSG_PTR_SIZE, 1000);
        if (MW_E_TIMEOUT == ret)
        {
            continue;
        }
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s: receive a message(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
            break;
        }
        /* Dump the received message for debuging */
        _db_dumpMsg(db_msg);

        /* Verify and parse the message from client */
        osapi_strncpy(client_qname, db_msg->cq_name, DB_Q_NAME_SIZE);
        client_qname[DB_Q_NAME_SIZE - 1] = '\0';
        ret = _db_msgHandle(db_msg, &db_notify);

        /* Assign the result in message */
        db_msg->type.result = ret;

        /* Send the reponse to client */
        if (0 != client_qname[0])
        {
            ret = db_sendMsg(client_qname, db_msg);
        }
        if ((MW_E_OK != ret) || (0 == client_qname[0]))
        {
            /* Need to free the db_msg, no retry */
            osapi_free((void *)(db_msg));
        }

        /* Sends the notification if the other client needed */
        if (0 != db_notify.msg_size)
        {
            if (M_GET == db_notify.msg->method || M_GETFLASH == db_notify.msg->method)
            {
                if (0 == client_qname[0])
                {
                    DB_LOG_ERROR("Empty client_qname with MSG to send");
                }
                else
                {
                    ret = db_sendMsg(client_qname, db_notify.msg);
                }

                if ((MW_E_OK != ret) || (0 == client_qname[0]))
                {
                    osapi_free((void *)(db_notify.msg));
                }
            }
            else
            {
                db_notificationsHandle(&db_notify);
                osapi_free((void *)(db_notify.msg));
            }
        }
    }

    /* End of the main loop */
    _db_releaseResource();
    osapi_processDelete(_db_ctx.ptr_dbmain);
    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: db_sendMsg
 * PURPOSE:
 *      Send a message to client
 *
 * INPUT:
 *      client            -- The queue name of the client
 *      ptr_msg           -- The message to be sent
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
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_PAYLOAD_T  *ptr_pload;

    MW_CHECK_PTR(ptr_client);
    MW_CHECK_PTR(ptr_msg);

    _db_dumpMsg(ptr_msg);
    /* Replace the first byte of message as DB message ID */
    ptr_msg->cq_name[0] = MW_MSG_ID_DB;
    ret = osapi_msgSend(ptr_client, (UI8_T *)ptr_msg, DB_MSG_PTR_SIZE, DB_Q_WAITTIME);
    if (MW_E_TIMEOUT == ret)
    {
        DB_LOG_ERROR("failed to send message to client: %s (ret = %u)", ptr_client, (UI8_T)(ret));
    }
    else if (MW_E_OK != ret)
    {
        ptr_pload = (DB_PAYLOAD_T *)(&(ptr_msg->ptr_payload));
        DB_LOG_ERROR("%s: send message to client: %s (ret = %u)", DB_INTERNAL_ERROR, ptr_client, (UI8_T)(ret));
        if (NULL != ptr_pload)
        {
            DB_LOG_ERROR("method:%u. cq_name:%s t_id:%u, f_id:%u\n", ptr_msg->method, ptr_msg->cq_name, ptr_pload->request.t_idx, ptr_pload->request.f_idx);
        }
    }
    else
    {
        DB_LOG_DEBUG("send message successfully to : %s", ptr_client);
    }

    return ret;
}

/* FUNCTION NAME: db_sendNotification
 * PURPOSE:
 *      Send a notification to client
 *
 * INPUT:
 *      ptr_client        -- The queue name of the client
 *      ptr_msg           -- The notification message
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
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;


    MW_CHECK_PTR(ptr_client);
    MW_CHECK_PTR(ptr_msg);

    DB_LOG_INFO("Send notification to client: %s", ptr_client);
    ret = db_sendMsg(ptr_client, ptr_msg);
    if (MW_E_OK != ret)
    {
        DB_LOG_INFO("Send failed, free msg %p", ptr_msg);
        osapi_free((void *)(ptr_msg));
    }

    return ret;
}

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
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if (FALSE == db_is_ready)
    {
        ret = MW_E_NOT_INITED;
    }
    return ret;
}

/* FUNCTION NAME: db_task_deinit
 * PURPOSE:
 *      Release the DB task and test task
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
    void)
{

#ifdef DB_DBG_TEST
    if (NULL != ptr_dbtesting1)
    {
        DB_LOG_DEBUG("delete testing 1 task (0x%p)", ptr_dbtesting1);
        osapi_processDelete(ptr_dbtesting1);
    }

    if (NULL != ptr_dbtesting2)
    {
        DB_LOG_DEBUG("delete testing 2 task (0x%p)", ptr_dbtesting2);
        osapi_processDelete(ptr_dbtesting2);
    }
#endif
    if (NULL != _db_ctx.ptr_dbmain)
    {
        DB_LOG_DEBUG("delete DB main task (0x%p)", _db_ctx.ptr_dbmain);
        osapi_processDelete(_db_ctx.ptr_dbmain);
    }

    return;
}

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
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;


    ret = osapi_threadCreateStatic(DB_MAIN_TASK_NAME, DB_STACK_SIZE, MW_TASK_PRIORITY_DB,
            _db_main, NULL, _db_ctx.db_task_stack, &(_db_ctx.db_stack), &(_db_ctx.ptr_dbmain));

    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: create task(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
    }

    /* For Debugging */
#ifdef DB_DBG_TEST
    ret = osapi_processCreate("db_test1", DB_STACK_SIZE, (DB_THREAD_PRI - 1),
            db_test_task_1, (void *)&ptr_dbtesting1, &ptr_dbtesting1);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: create testing 1 task(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
    }
    ret = osapi_processCreate("db_test2", DB_STACK_SIZE, (DB_THREAD_PRI - 1),
            db_test_task_2, (void *)&ptr_dbtesting2, &ptr_dbtesting2);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: create testing 2 task(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
    }
#endif


    return;
}

