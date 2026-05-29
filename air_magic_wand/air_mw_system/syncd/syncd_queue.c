/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:  syncd_queue.c
 * PURPOSE:
 *  Implement internal queue function of synchronized daemon.
 *
 * NOTES:
 *
 */

#include <string.h>
#include "syncd.h"
#include "syncd_in.h"
#include "syncd_opts.h"
#ifdef SYNCD_TEST_ENABLE
#include "syncd_msg.h"
#endif
#include "mw_error.h"
#include "osapi.h"
#include "osapi_memory.h"
#include "osapi_message.h"
#include "osapi_string.h"

/* NAMING CONSTANT DECLARATIONS
*/
/* DB_MSG_T.type.count is not supposed to be greater than 127. */
#define SYNCD_QUEUE_COUNT_REUSE_FLAG    BIT(7)
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/
typedef struct
{
    staticMsghandle_t msg_queue_handle;
    staticMsghandle_t db_queue_handle;
    /* msg_queue_storage must be 4-bytes aligned. */
    MW_MSG_T *msg_queue_storage[SYNCD_MSG_QUEUE_LEN]; /* Task queue is a static queue. */
    /* db_queue_storage must be 4-bytes aligned. */
    MW_MSG_T *db_queue_storage[SYNCD_DB_QUEUE_LEN]; /* Task queue is a static queue. */
} SYNCD_QUEUE_CONTEXT_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */
GDMPSRAM_BSS_EX static SYNCD_QUEUE_CONTEXT_T _syncd_queue_ctx;

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _syncd_queue_db_genericGetData
 * PURPOSE:
 *      Get data from DB. It can get data for a single request or multiple
 *      requests depend on pptr_msg inputted. It's a blocking API.
 *
 * INPUT:
 *      pptr_msg        --  A double pointer points to a DB message with one or
 *                          more requests
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a DB message with data
 *                          received from DB for the single or multiple requests
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      If any error occors, the message pointed by *pptr_msg will be freed
 *      before the function returns. As a result the message pointed by *pptr_msg
 *      need be freed after use only when MW_E_OK is returned.
 */
static MW_ERROR_NO_T
_syncd_queue_db_genericGetData(
    DB_MSG_T **pptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    C8_T cq_name[DB_Q_NAME_SIZE] = {0};

    if ((NULL == pptr_msg) || (NULL == (*pptr_msg)))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_msg = *pptr_msg;
    osapi_memcpy(cq_name, ptr_msg->cq_name, DB_Q_NAME_SIZE - 1);
    /* There is no need to clear the SYNCD_DB_QUEUE_NAME queue
     * before sending a new message and waiting for its response because the
     * queue is waited indefinitely every time it is used.
     */
    /* Send the message to DB */
    SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Send");
    ret = dbapi_sendMsg(ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    *pptr_msg = NULL;
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        SYNCD_LOG_ERROR("Fail to send the msg to DB. ret:%d msg:0x%x", ret, ptr_msg);
        return ret;
    }

    ret = syncd_queue_recv(cq_name, (UI8_T **)&ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK == ret)
    {
        SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Receive");
        ret = ptr_msg->type.result;
        if (MW_E_OK == ret)
        {
            (*pptr_msg) = ptr_msg;
        }
        else
        {
            SYNCD_LOG_ERROR("Fail at request %d ret:%d ptr_msg:0x%x", ptr_msg->type.result, ret, ptr_msg);
            MW_FREE(ptr_msg);
        }
    }
    /* else there is no need to free the message sent because:
     * 1. The timeout error should not be returned because the response is waited
     *    indefinitely.
     * 2. If DB fails to send response back to SYNCD queue, DB should free the
     *    message instead of SYNCD.
     */
    return ret;
}

/* FUNCTION NAME: _syncd_queue_db_setQueueName
 * PURPOSE:
 *      Set ptr_msg->cq_name[] based on block input parameter.
 *
 * INPUT:
 *      ptr_msg         --  A pointer points to a DB message with one or more
 *                          requests
 *      block           --  Indicate if a response from DB is wanted and how to
 *                          receive the response
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_syncd_queue_db_setQueueName(
    DB_MSG_T *ptr_msg,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block)
{
    if (NULL == ptr_msg)
    {
        return MW_E_BAD_PARAMETER;
    }

    osapi_memset(ptr_msg->cq_name, 0, DB_Q_NAME_SIZE);
    switch (block)
    {
        case SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP:
        {
            /* cq_name is set to be empty. */
            break;
        }

        case SYNCD_QUEUE_DB_NONBLOCK_WITHRSP:
        {
            osapi_memcpy(ptr_msg->cq_name, SYNCD_MSG_QUEUE_NAME, DB_Q_NAME_SIZE - 1);
            break;
        }

        case SYNCD_QUEUE_DB_BLOCK:
        {
            /* There is no need to clear the SYNCD_DB_QUEUE_NAME queue
             * before sending a new message and waiting for its resonse because the
             * queue is waited indefinitely every time it is used.
             */
            osapi_memcpy(ptr_msg->cq_name, SYNCD_DB_QUEUE_NAME, DB_Q_NAME_SIZE - 1);
            break;
        }

        default:
        {
            return MW_E_BAD_PARAMETER;
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: _syncd_queue_db_genericSetData
 * PURPOSE:
 *      Set data to DB. It can set data for a single request or multiple
 *      requests depend on ptr_msg inputted.
 *
 * INPUT:
 *      ptr_msg         --  A pointer points to a DB message with one or more
 *                          requests
 *      block           --  Indicate if a response from DB is wanted and how to
 *                          receive the response
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      1. Call _syncd_queue_db_setQueueName() to set cq_name for the DB message
 *      before using this function.
 *      2. The message pointed by *pptr_msg need not be freed no matter what
 *      error code is returned.
 */
static MW_ERROR_NO_T
_syncd_queue_db_genericSetData(
    DB_MSG_T *ptr_msg,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    C8_T cq_name[DB_Q_NAME_SIZE] = {0};

    if (SYNCD_QUEUE_DB_BLOCK == block)
    {
        osapi_memcpy(cq_name, ptr_msg->cq_name, DB_Q_NAME_SIZE);
    }

    /* Send the message to DB */
    SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Send");
    ret = dbapi_sendMsg(ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        SYNCD_LOG_ERROR("Fail to send the msg to DB");
    }

    if ((MW_E_OK != ret) || (SYNCD_QUEUE_DB_BLOCK != block))
    {
        return ret;
    }

    ret = syncd_queue_recv(cq_name, (UI8_T **)&ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK == ret)
    {
        SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Receive");
        ret = ptr_msg->type.result;
        if (MW_E_OK != ret)
        {
            SYNCD_LOG_ERROR("Fail at request %d", ret);
        }
        MW_FREE(ptr_msg);
        SYNCD_LOG_DEBUG("response received. result:%d", ret);
    }
    /* else there is no need to free the message because:
     * 1. The timeout error should not be returned becayse the response is waited
     *    indefinitely.
     * 2. If DB fails to send response back to SYNCD queue, DB should free the
     *    message instead of SYNCD.
     */

    return ret;
}

/* FUNCTION NAME: _syncd_queue_send
 * PURPOSE:
 *      Send data to a specific queue.
 *
 * INPUT:
 *      ptr_name        --  A descriptive name for the queue
 *      ptr_msg         --  A pointer to the message buffer
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_syncd_queue_send(
    const C8_T *ptr_name,
    const UI8_T *ptr_msg,
    const UI32_T timeout)
{
    return osapi_msgSend(ptr_name, ptr_msg, 0, timeout);
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_queue_init
 * PURPOSE:
 *      Initialize the queues for SYNCD.
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
syncd_queue_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Create a static queue for the interaction with the DB task. */
    ret |= osapi_msgCreateStatic(SYNCD_MSG_QUEUE_NAME,
                                 SYNCD_MSG_QUEUE_LEN,
                                 sizeof(MW_MSG_T *),
                                 (UI8_T *)_syncd_queue_ctx.msg_queue_storage,
                                 &_syncd_queue_ctx.msg_queue_handle);

    /* Create a static queue for the blocking interaction with the DB task, such as
     * getting data.
     */
    ret |= osapi_msgCreateStatic(SYNCD_DB_QUEUE_NAME,
                                 SYNCD_DB_QUEUE_LEN,
                                 sizeof(MW_MSG_T *),
                                 (UI8_T *)_syncd_queue_ctx.db_queue_storage,
                                 &_syncd_queue_ctx.db_queue_handle);

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_queue_free
 * PURPOSE:
 *      Release all allocated memory in syncd queue.
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
syncd_queue_free(
    void)
{
    osapi_msgDelete(SYNCD_MSG_QUEUE_NAME);
    osapi_msgDelete(SYNCD_DB_QUEUE_NAME);
}

/* FUNCTION NAME: syncd_queue_recv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer to the message buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_recv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T timeout)
{
    return osapi_msgRecv(ptr_name, pptr_msg, 0, timeout);
}

/* FUNCTION NAME: syncd_queue_db_send
 * PURPOSE:
 *      Package the message and send it to DB. If ptr_queue_name is NULL, DB
 *      will not send any response and the message itself will be freed by DB.
 *
 * INPUT:
 *      ptr_queue_name   --  A pointer to the queue that DB will return a
 *                           response to. If it is NULL, no response is expected.
 *      method           --  the method bitmap
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      e_idx            --  the entry index in the table
 *      ptr_data         --  A pointer to the message data
 *      timeout          --  The maximun amout of time the thread will wait for
 *                           send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
syncd_queue_db_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx};

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    ptr_msg = dbapi_createMsgByReq(ptr_queue_name, method, 1, &request, &msg_size, (UI8_T **)&ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    SYNCD_LOG_DEBUG("cq_name=%s method=0x%X count=%u t_idx=%u f_idx=%u e_idx=%u",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx);

    ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_data, &ptr_msg, &msg_size, (UI8_T **)&ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    /* Send the message to DB */
    SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Send");
    ret = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        SYNCD_LOG_ERROR("Fail to send the msg to DB");
    }

    return ret;
}

/* FUNCTION NAME: syncd_queue_db_appendMsg
 * PURPOSE:
 *      Append db msg
 *
 * INPUT:
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      e_idx            --  the entry index in the table
 *      ptr_data         --  A pointer to the message data
 *
 * OUTPUT:
 *      ptr_msg           -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      msg_size          -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      ptr_payload       -- A double pointer returns the position of the
 *                                 payload to store the next request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_db_appendMsg(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    DB_MSG_T  **ptr_msg,
    UI16_T     *msg_size,
    UI8_T     **ptr_payload)
{
    DB_REQUEST_TYPE_T req;

    req.t_idx = t_idx;
    req.f_idx = f_idx;
    req.e_idx = e_idx;

    return dbapi_appendMsgPayload(&req, (UI8_T *)ptr_data, ptr_msg, msg_size, ptr_payload);
}

/* FUNCTION NAME: syncd_queue_db_getData
 * PURPOSE:
 *      Get the raw data based on t_idx, f_idx and e_idx from DB. It is a
 *      blocking API.
 *
 * INPUT:
 *      method          --  The method of the getting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a db message received from DB
 *      ptr_data_size   --  A pointer returns the size of raw data obtained from DB
 *      pptr_data       --  A double pointer returns the raw data obtained from DB
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. If MW_E_OK is returned, the db message returned by pptr_msg need be
 *      freed after use.
 *      2. dbapi_parseMsg() cannot be used for the message returned by
 *         pptr_msg.
 */
MW_ERROR_NO_T
syncd_queue_db_getData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx};
    UI8_T *ptr_payload_data = NULL;

    if ((t_idx >= TABLES_LAST) || (ptr_data_size == NULL) ||
        (pptr_msg == NULL) || (*pptr_msg != NULL) || (pptr_data == NULL))
    {
        SYNCD_LOG_DEBUG("Invalid param: t_idx:%d ptr_data_size:%d pptr_msg:0x%p pptr_data:0x%p *pptr_msg:0x%p *pptr_data:0x%p",
                    t_idx, ptr_data_size, pptr_msg, pptr_data,
                    pptr_msg ? *pptr_msg : (void *)0xFF,
                    pptr_data ? *pptr_data : (void *)0xFF);
        return MW_E_BAD_PARAMETER;
    }

    *ptr_data_size = 0;
    ptr_msg = dbapi_createMsgByReq(SYNCD_DB_QUEUE_NAME, method, 1, &request, &msg_size, (UI8_T **)&ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    SYNCD_LOG_DEBUG("qname=%s method=0x%X count=%u tid=%u fid=%u eid=%u msg:0x%x msize:%d payload:0x%x",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx, ptr_msg, msg_size, ptr_payload);

    ret = dbapi_appendMsgPayload(&request, NULL, &ptr_msg, &msg_size, (UI8_T **)&ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    ret = _syncd_queue_db_genericGetData(&ptr_msg);
    if (MW_E_OK == ret)
    {
        ret = dbapi_parseMsg(ptr_msg, 1, &request, ptr_data_size, (UI8_T **)pptr_data, &ptr_payload_data);
        if (MW_E_OK == ret)
        {
            *pptr_msg = ptr_msg;
        }
        else
        {
            MW_FREE(ptr_msg);
            *ptr_data_size = 0;
            *pptr_data = NULL;
        }
    }

    return ret;
}

/* FUNCTION NAME: syncd_queue_db_setData
 * PURPOSE:
 *      Set the raw data based on t_idx, f_idx and e_idx to DB. If
 *      SYNCD_QUEUE_DB_BLOCK is set to the block input parameter, it will work
 *      as a blocking API.
 *
 * INPUT:
 *      method          --  The method of the setting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *      ptr_data        --  A pointer to the raw data to be set
 *      block           --  Indicate if a response from DB is wanted or waited for
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      ptr_data need be freed after the function returns if it is allocated
 *      from heap and will not be used any more.
 */
MW_ERROR_NO_T
syncd_queue_db_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx};

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    /* cq_name will be set based on block input parameter within _syncd_queue_db_setQueueName(). */
    ptr_msg = dbapi_createMsgByReq(NULL, method, 1, &request, &msg_size, (UI8_T **)&ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    SYNCD_LOG_DEBUG("cq_name=%s method=0x%X count=%u t_idx=%u f_idx=%u e_idx=%u block:%d",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx, block);

    ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_data, &ptr_msg, &msg_size, (UI8_T **)&ptr_payload);
    ret |= _syncd_queue_db_setQueueName(ptr_msg, block);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    return _syncd_queue_db_genericSetData(ptr_msg, block);
}

/* FUNCTION NAME: syncd_queue_db_getMultiData
 * PURPOSE:
 *      Get multiple raw data from DB indicated by the payload of pptr_msg
 *      inputted.
 *
 * INPUT:
 *      pptr_msg        --  A double pointer points to a DB message with one or
 *                          more requests
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a DB message with data
 *                          received from DB for the single or multiple requests
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      If any error occors, the message pointed by *pptr_msg will be freed
 *      before the function returns. As a result the message pointed by *pptr_msg
 *      need be freed after use only when MW_E_OK is returned.
 */
MW_ERROR_NO_T
syncd_queue_db_getMultiData(
    DB_MSG_T **pptr_msg)
{
    if ((NULL != pptr_msg) && (NULL != (*pptr_msg)))
    {
        osapi_memcpy((*pptr_msg)->cq_name, SYNCD_DB_QUEUE_NAME, DB_Q_NAME_SIZE - 1);
        (*pptr_msg)->cq_name[DB_Q_NAME_SIZE - 1] = '\0';
    }

    return _syncd_queue_db_genericGetData(pptr_msg);
}

/* FUNCTION NAME: syncd_queue_db_setMultiData
 * PURPOSE:
 *      Set multiple raw data to DB based on the payload of ptr_msg inputted.
 *
 * INPUT:
 *      ptr_msg         --  A pointer points to a DB message with one or more
 *                          requests
 *      block           --  Indicate if a response from DB is wanted and how to
 *                          receive the response
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      The message pointed by *pptr_msg need not be freed no matter what error
 *      code is returned. It will be freed by SYNCD or DB.
 */
MW_ERROR_NO_T
syncd_queue_db_setMultiData(
    DB_MSG_T *ptr_msg,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    ret = _syncd_queue_db_setQueueName(ptr_msg, block);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    return _syncd_queue_db_genericSetData(ptr_msg, block);
}

/* FUNCTION NAME: syncd_queue_task_sendMsg
 * PURPOSE:
 *      Send a message to the SYNCD task.
 *
 * INPUT:
 *      ptr_msg         --  A pointer to the message buffer
 *      solo_msg        --  Indicate if there is at most one message in the queue
 *                          for a specific msg_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_task_sendMsg(
    MW_MSG_T *ptr_msg,
    BOOL_T solo_msg)
{
    if (NULL == ptr_msg)
    {
        return MW_E_BAD_PARAMETER;
    }

    if (TRUE == solo_msg)
    {
        UI32_T i = 0, j = 0;
        MW_MSG_T *ptr_peekMsg = NULL;
        UI32_T msg_waiting = 0;

        osapi_msgWaiting(SYNCD_MSG_QUEUE_NAME, &msg_waiting);

        if (0 != msg_waiting)
        {
            osapi_msgPeek(SYNCD_MSG_QUEUE_NAME, (UI8_T **)&ptr_peekMsg, sizeof(MW_MSG_T *), MSG_TIMEOUT_RETRUN_IMMEDIATELY);
            if (NULL != ptr_peekMsg)
            {
                /* Traverse the queue storage to check if a message with the same
                 * msg_id has already been sent to the queue.
                 */
                for (; i < SYNCD_MSG_QUEUE_LEN; i++)
                {
                    /* Point to the same message. It is the start of the messages in queue. */
                    if (ptr_peekMsg == _syncd_queue_ctx.msg_queue_storage[i])
                    {
                        for (j = 0; j < msg_waiting; j++)
                        {
                            ptr_peekMsg = _syncd_queue_ctx.msg_queue_storage[(i + j) % SYNCD_MSG_QUEUE_LEN];
                            if ((NULL != ptr_peekMsg) && (ptr_msg->msg_id == ptr_peekMsg->msg_id))
                            {
                                SYNCD_LOG_DEBUG("For solo msg:%d, there has been one in the queue already", ptr_msg->msg_id);
                                return MW_E_OP_STOPPED;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return _syncd_queue_send(SYNCD_MSG_QUEUE_NAME, (UI8_T *)ptr_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
}

#ifdef SYNCD_TEST_ENABLE
void
syncd_queue_test_handleTestMsg(
    SYNCD_MSG_TEST_REQ_T *ptr_msg)
{
    SYNCD_LOG_DEBUG("line:%d msg_id:%d solo:%d", __LINE__, ptr_msg->msg_id, ptr_msg->solo);
    syncd_queue_test_dbSingleData();
    syncd_queue_test_dbSetMultiData();
    syncd_queue_test_dbGetMultiData();
    syncd_queue_test_dbGetMultiDataByReq();
}

void
syncd_queue_test_taskQueue(
    void)
{
    MW_MSG_TEST_REQ_T *ptr_req = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T i = 0;

    SYNCD_LOG_DEBUG("line:%d enter", __LINE__);
    for (i; i < 6; i++)
    {
        ptr_req = NULL;
        osapi_calloc(sizeof(MW_MSG_TEST_REQ_T), SYNCD_NAME, &ptr_req);
        if (NULL == ptr_req)
        {
            ret = MW_E_NO_MEMORY;
            break;
        }

        ptr_req->solo = (i < 3) ? TRUE : FALSE;

        ptr_req->msg_id = MSG_ID_SYNCD_SYNCD_TEST_REQ;
        ret = syncd_queue_task_sendMsg((MW_MSG_T *)ptr_req, ptr_req->solo);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_req);
        }
    }

    SYNCD_LOG_DEBUG("line:%d i:%d ret:%d", __LINE__, i, ret);
}

void
syncd_queue_test_dbSingleData(
    void)
{
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size;
    void *ptr_data = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T i = 0;
    UI8_T adminState = 0;
    SYNCD_QUEUE_DB_BLOCK_ENUM block = SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP;

    SYNCD_LOG_DEBUG("line:%d enter", __LINE__);
    for (i = 0; i < 4; i++)
    {
        block = SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP + i;
        adminState = !adminState;

        ret = syncd_queue_db_setData(M_UPDATE, PORT_CFG_INFO, PORT_ADMIN_STATUS, 1, &adminState, block);
        if (MW_E_OK == ret)
        {
            ret = syncd_queue_db_getData(M_GET, PORT_CFG_INFO, PORT_ADMIN_STATUS, 1, &ptr_msg, &data_size, &ptr_data);
            if (MW_E_OK == ret)
            {
                SYNCD_LOG_DEBUG("tid%d fid%d eid:%d data_size:%d data:%d ptr_msg:0x%x",
                            PORT_CFG_INFO, PORT_ADMIN_STATUS, 1, data_size, ((UI8_T *)ptr_data)[0], ptr_msg);
                MW_FREE(ptr_msg);
            }
            else
            {
                SYNCD_LOG_DEBUG("fail ret:%d", ret);
            }
        }
    }

    ret = syncd_queue_db_getData(M_GETFLASH, PORT_CFG_INFO, PORT_ADMIN_STATUS, 1, &ptr_msg, &data_size, &ptr_data);
    if (MW_E_OK == ret)
    {
        SYNCD_LOG_DEBUG("tid%d fid%d eid:%d data_size:%d data:%d ptr_msg:0x%x",
                    PORT_CFG_INFO, PORT_ADMIN_STATUS, 1, data_size, ((UI8_T *)ptr_data)[0], ptr_msg);
        MW_FREE(ptr_msg);
    }
    else
    {
        SYNCD_LOG_DEBUG("fail ret:%d", ret);
    }
}

void
syncd_queue_test_dbSetMultiData(
    void)
{
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0, adminState = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_msg = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T i = 0, mtu_size = 2;
    SYNCD_QUEUE_DB_BLOCK_ENUM block = SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP;

    SYNCD_LOG_DEBUG("line:%d enter", __LINE__);

    /* payload_size should be set large enough to store all requests. Here is
     * just a test for the case that it is not set large enough for two requests.
     * It will still work, but it is not recommended.
     */
    payload_size = DB_MSG_PAYLOAD_SIZE + 1;

    for (i = 0; i < 4; i++)
    {
        ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
        if (NULL == ptr_msg)
        {
            ret = MW_E_NO_MEMORY;
            break;
        }

        adminState = !adminState;
        mtu_size +=i;
        block = SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP + i;

        /* Set the first request */
        request.t_idx = PORT_CFG_INFO;
        request.f_idx = PORT_ADMIN_STATUS;
        request.e_idx = 1;
        ret = dbapi_appendMsgPayload(&request, &adminState, &ptr_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            break;
        }

        /* Set the second request. ptr_msg will be enlarged by reallocation. */
        request.t_idx = JUMBO_FRAME_INFO;
        request.f_idx = JUMBO_FRAME_CFG;
        request.e_idx = DB_ALL_ENTRIES;
        ret = dbapi_appendMsgPayload(&request, &mtu_size, &ptr_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            break;
        }

        ret = syncd_queue_db_setMultiData(ptr_msg, block);
        SYNCD_LOG_DEBUG("setMultiData block:%d i:%d, ret:%d", block, i, ret);
    }
    SYNCD_LOG_DEBUG("setMultiData block:%d i:%d, ret:%d", block, i, ret);
}

void
syncd_queue_test_dbGetMultiData(
    void)
{
    UI16_T msg_size = 0, data_size = 0;
    UI8_T *ptr_payload = NULL, *ptr_payload_data = NULL, method = M_GET, payload_size = 0, count = 0;
    DB_REQUEST_TYPE_T request = {0}, out_request;
    DB_MSG_T *ptr_msg = NULL;
    void *ptr_data = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T i = 0;

    SYNCD_LOG_DEBUG("line:%d enter", __LINE__);
    /* payload_size should be set large enough to store all requests. Here is
     * just a test for the case that it is not set large enough for two requests.
     * It will still work, but it is not recommended.
     */
    payload_size = DB_MSG_PAYLOAD_SIZE + 1;

    for (i = 0; i < 2; i++)
    {
        if (i == 1)
        {
            method = M_GETFLASH;
        }

        ptr_msg = dbapi_createMsg(NULL, method, 1, payload_size, &msg_size, &ptr_payload);
        if (NULL == ptr_msg)
        {
            ret = MW_E_NO_MEMORY;
            break;
        }
        count = 0;

        /* Set the first request */
        request.t_idx = PORT_CFG_INFO;
        request.f_idx = PORT_ADMIN_STATUS;
        request.e_idx = 1;
        ret = dbapi_appendMsgPayload(&request, NULL, &ptr_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            break;
        }
        count++;

        /* Set the second request. ptr_msg will be enlarged by reallocation. */
        request.t_idx = ACCOUNT_INFO;
        request.f_idx = ACC_PASSWD;
        request.e_idx = 1;
        ret = dbapi_appendMsgPayload(&request, NULL, &ptr_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            break;
        }
        count++;

        ret = syncd_queue_db_getMultiData(&ptr_msg);
        if (MW_E_OK != ret)
        {
            break;
        }

        osapi_memset(&out_request, 0, sizeof(DB_REQUEST_TYPE_T));
        ptr_payload_data = NULL;
        /* Parse the first request. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d tid%d fid%d eid:%d data_size:%d data:%x",
                    ret, out_request.t_idx, out_request.f_idx, out_request.e_idx, data_size, ((UI8_T *)ptr_data)[0]);

        /* Continue to parse the next out_request. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d tid%d fid%d eid:%d data_size:%d data:%x %x %x %x",
                    ret, out_request.t_idx, out_request.f_idx, out_request.e_idx, data_size,
                    ((UI8_T *)ptr_data)[0], ((UI8_T *)ptr_data)[1], ((UI8_T *)ptr_data)[2], ((UI8_T *)ptr_data)[3]);

        /* Continue to parse and it is supposed to fail. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d", ret);

        MW_FREE(ptr_msg);
    }

    SYNCD_LOG_DEBUG("line:%d i:%d ret:%d", __LINE__, i, ret);
}

void
syncd_queue_test_dbGetMultiDataByReq(
    void)
{
    DB_REQUEST_TYPE_T request[] = {
        {PORT_CFG_INFO, PORT_ADMIN_STATUS, 1},
        {ACCOUNT_INFO, ACC_PASSWD, 1},
    };
    UI16_T msg_size = 0, data_size = 0;
    UI8_T *ptr_payload = NULL, *ptr_payload_data = NULL, method = M_GET, count = 0;
    DB_REQUEST_TYPE_T out_request;
    DB_MSG_T *ptr_msg = NULL;
    void *ptr_data = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T i = 0;

    SYNCD_LOG_DEBUG("line:%d enter", __LINE__);
    count = sizeof(request) / sizeof(DB_REQUEST_TYPE_T);

    for (i = 0; i < 2; i++)
    {
        if (i == 1)
        {
            method = M_GETFLASH;
        }

        ptr_msg = dbapi_createMsgByReq(NULL, method, count, &request, &msg_size, &ptr_payload);
        if (NULL == ptr_msg)
        {
            ret = MW_E_NO_MEMORY;
            break;
        }

        ret = dbapi_appendMsgPayload(&request[0], NULL, &ptr_msg, &msg_size, &ptr_payload);
        ret |= dbapi_appendMsgPayload(&request[1], NULL, &ptr_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            break;
        }

        ret = syncd_queue_db_getMultiData(&ptr_msg);
        if (MW_E_OK != ret)
        {
            break;
        }

        osapi_memset(&out_request, 0, sizeof(DB_REQUEST_TYPE_T));
        ptr_payload_data = NULL;

        /* Parse the first request. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d tid%d fid%d eid:%d data_size:%d data:%x",
                    ret, out_request.t_idx, out_request.f_idx, out_request.e_idx, data_size, ((UI8_T *)ptr_data)[0]);

        /* Continue to parse the next request. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d tid%d fid%d eid:%d data_size:%d data:%x %x %x %x",
                    ret, out_request.t_idx, out_request.f_idx, out_request.e_idx, data_size,
                    ((UI8_T *)ptr_data)[0], ((UI8_T *)ptr_data)[1], ((UI8_T *)ptr_data)[2], ((UI8_T *)ptr_data)[3]);


        /* Continue to parse and it is supposed to fail. */
        ret = dbapi_parseMsg(ptr_msg, count, &out_request, &data_size, &ptr_data, &ptr_payload_data);
        SYNCD_LOG_DEBUG("ret:%d", ret);

        MW_FREE(ptr_msg);
    }

    SYNCD_LOG_DEBUG("line:%d i:%d ret:%d", __LINE__, i, ret);
}
#endif /* SYNCD_TEST_ENABLE */

