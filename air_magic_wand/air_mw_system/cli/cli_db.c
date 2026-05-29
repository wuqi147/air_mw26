/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:   cli_db.c
 * PURPOSE:
 *      Implement the basic functions of cli db operation.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_message.h"
#include "cli_internal.h"
#include "cli_db.h"

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

/* LOCAL SUBPROGRAM BODIES
*/
static void
_cli_db_handle_sys_info(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T e_idx,
    UI16_T size,
    const void *ptr_data)
{
    CLI_DATA_T *ptr_cli = cli_get_info();

    if (NULL == ptr_cli)
    {
        CLI_LOG_ERROR("get info failed");
        return;
    }

    if (SYS_INFO == t_idx)
    {
        if (SYS_NAME == f_idx)
        {
            if (0 == osapi_strcmp(ptr_data, ptr_cli->conf.hostname))
            {
                CLI_LOG_DEBUG("Hostname already set to %s", ptr_cli->conf.hostname);
            }
            else
            {
                osapi_strncpy(ptr_cli->conf.hostname, ptr_data, size);
                ptr_cli->conf.hostname[MAX_SYS_NAME_SIZE - 1] = '\0';
                CLI_LOG_INFO("Hostname set to %s by DB", ptr_cli->conf.hostname);
            }
        }
    }
}

static MW_ERROR_NO_T
_cli_db_process_requests(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void *ptr_data)
{
    if ((NULL == ptr_request) || (NULL == ptr_data) || (0 == data_size))
    {
        CLI_LOG_ERROR("bad parameter! ptr_request=%p, ptr_data=%p, data_size=%d", ptr_request, ptr_data, data_size);
        return MW_E_BAD_PARAMETER;
    }

    if ((M_GET == method) || (M_UPDATE == method))
    {
        _cli_db_handle_sys_info(ptr_request->t_idx, ptr_request->f_idx, ptr_request->e_idx, data_size, ptr_data);
    }

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: cli_queue_send_to_db
 * PURPOSE:
 *      Send a message to the database queue.
 * INPUT:
 *      ptr_msg      -- Pointer to the DB message.
 *      size         -- Size of the message.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_queue_send_to_db(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    MW_ERROR_NO_T rc;

    MW_CHECK_PTR(ptr_msg);

    rc = dbapi_dbisReady();
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
        return rc;
    }

    rc = dbapi_sendRequesttoDb(size, ptr_msg);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("dbapi_sendRequesttoDb() failed");
        MW_FREE(ptr_msg);
    }
    return rc;
}

/* FUNCTION NAME: cli_queue_send
 * PURPOSE:
 *      Construct and send a DB message.
 * INPUT:
 *      method       -- DB method (e.g. M_GET).
 *      t_idx        -- Table Index.
 *      f_idx        -- Field Index.
 *      e_idx        -- Entry Index.
 *      ptr_data     -- Pointer to the payload data.
 *      size         -- Size of the payload data.
 * OUTPUT:
 *      pptr_out_msg -- Address to store the allocated message pointer.
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    DB_PAYLOAD_T *ptr_payload = NULL;
    UI32_T msg_size;

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(msg_size, CLI_TASK_NAME, (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("allocate memory failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }

    /* message */
    if (M_GET == method || M_UPDATE == method)
    {
        osapi_strncpy(ptr_msg->cq_name, CLI_GET_QUEUE_NAME, DB_Q_NAME_SIZE);
    }
    else
    {
        osapi_strncpy(ptr_msg->cq_name, CLI_QUEUE_NAME, DB_Q_NAME_SIZE);
    }

    ptr_msg->method = method;
    ptr_msg->type.count = 1;

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    CLI_LOG_DEBUG("send method:0x%x, table:%d, field:%d, entry:%d, queue:[%s], data:[%d], size:[%d]",
            method,
            ptr_payload->request.t_idx,
            ptr_payload->request.f_idx,
            ptr_payload->request.e_idx,
            ptr_msg->cq_name,
            ((UI8_T *)&(ptr_payload->ptr_data))[0],
            ptr_payload->data_size);
    if ((size > 0) && (method != M_GET))
    {
        osapi_memcpy(&(ptr_payload->ptr_data), ptr_data, size);

    }

    /* Send message to DB */
    rc = cli_queue_send_to_db(ptr_msg, msg_size);

    if (rc == MW_E_OK)
    {
        if (NULL != pptr_out_msg)
        {
            *pptr_out_msg = ptr_msg;
        }
    }
    else
    {
        CLI_LOG_ERROR("Send message to DB failed(%d)", rc);
        MW_FREE(ptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: cli_queue_recv
 * PURPOSE:
 *      Receive a message from the CLI GET queue.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_q_name   -- Name of the queue to receive from.
 *      pptr_buf     -- Address to store the received message pointer.
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_queue_recv(
    const C8_T *ptr_q_name,
    void **pptr_buf)
{
    MW_ERROR_NO_T rc;
    UI8_T *ptr_msg = NULL;

    if (NULL == ptr_q_name)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = osapi_msgRecv(ptr_q_name, &ptr_msg, 0, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    CLI_LOG_DEBUG("ptr_msg=%p", ptr_msg);
    (*pptr_buf) = ptr_msg;

    return MW_E_OK;
}

/* FUNCTION NAME: cli_queue_getData
 * PURPOSE:
 *      Synchronously get data from the DB.
 * INPUT:
 *      in_t_idx     -- Table Index.
 *      in_f_idx     -- Field Index.
 *      in_e_idx     -- Entry Index.
 * OUTPUT:
 *      pptr_out_msg -- Address to store the response message pointer.
 *      ptr_out_size -- Address to store the size of the data.
 *      pptr_out_data-- Address to store the pointer to the data payload.
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T total_size = 0;
    DB_PAYLOAD_T *ptr_pload = NULL;

    DB_REQUEST_TYPE_T request = {
        .t_idx = in_t_idx,
        .f_idx = in_f_idx,
        .e_idx = in_e_idx
    };

    rc = dbapi_getDataSize(request, &total_size);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("dbapi_getDataSize failed(%d)", rc);
        return rc;
    }
    CLI_LOG_DEBUG("dbapi_getDataSize sucess, total_size = %d", total_size);

    rc = cli_queue_send(M_GET, in_t_idx, in_f_idx, in_e_idx, NULL, total_size, &ptr_msg);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("cli_queue_send failed(%d)", rc);
        return rc;
    }

    /* wait for DB response messgae */
    rc = cli_queue_recv(CLI_GET_QUEUE_NAME, (void **)&ptr_msg);
    if (MW_E_OK == rc)
    {
        CLI_LOG_DEBUG("cli_queue_recv success");
    }
    else
    {
        CLI_LOG_ERROR("cli_queue_recv failed(%d)", rc);
        MW_FREE(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    (*ptr_out_size) = total_size;

    ptr_pload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    (*pptr_out_data) = &(ptr_pload->ptr_data);

    CLI_LOG_DEBUG("pptr_out_msg = %p, *pptr_out_data = %p", *pptr_out_msg, *pptr_out_data);

    return MW_E_OK;
}

/* FUNCTION NAME:   cli_queue_setData
 * PURPOSE:
 *      package message and call sending function to DB directly.
 *
 * INPUT:
 *      method      --  the method bitmap
 *      t_idx       --  the enum of the table
 *      f_idx       --  the enum of the field
 *      e_idx       --  the entry index in the table
 *      ptr_data    --  pointer to message data
 *      size        --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
cli_queue_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;

    CLI_LOG_DEBUG("set DB data");
    rc = cli_queue_send(method, t_idx, f_idx, e_idx, ptr_data, size, NULL);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("cli_queue_send failed(%d)", rc);
        return rc;
    }

    /* wait for DB response messgae */
    rc = cli_queue_recv(CLI_GET_QUEUE_NAME, (void **) & ptr_msg);
    if (MW_E_OK == rc)
    {
        CLI_LOG_DEBUG("cli_queue_recv success");
    }
    else
    {
        CLI_LOG_ERROR("cli_queue_recv failed(%d)", rc);
    }

    MW_FREE(ptr_msg);

    return MW_E_OK;
}

void
cli_db_handle_msg(
    DB_MSG_T *ptr_msg)
{
    DB_REQUEST_TYPE_T request = { 0 };
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    CLI_LOG_DEBUG("Received DB message(msg id:%d).", ((MW_MSG_T *)ptr_msg)->msg_id);
    if (0 != (ptr_msg->method & M_B_RESPONSE))
    {
        /* Process Response message. Do nothing currently. */
        CLI_LOG_DEBUG("Received response message.");
    }
    else
    {
        /* Process the notification and other messages. */
        do
        {
            rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
            if (MW_E_OK == rc)
            {
                CLI_LOG_DEBUG("recv method:0x%x, [t/f/e]:[%d/%d/%d]", ptr_msg->method, request.t_idx, request.f_idx, request.e_idx);
                _cli_db_process_requests(ptr_msg->method, &request, data_size, ptr_data);
            }
            /* Continue to parse the next request within the payload. */
        } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
    }
}
