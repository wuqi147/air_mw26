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

/* FILE NAME:   mw_telnet_db.c
 * PURPOSE:
 *      Implement the basic functions of telnet queue operation.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_message.h"
#include "mw_telnet.h"
#include "mw_telnet_queue.h"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
mw_telnet_queue_send_to_db(
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
        TELNET_LOG_ERROR("dbapi_sendRequesttoDb() failed");
        MW_FREE(ptr_msg);
    }
    return rc;
}

MW_ERROR_NO_T
mw_telnet_queue_send(
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
    rc = osapi_calloc(msg_size, TELNET_TASK_NAME, (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        TELNET_LOG_ERROR("allocate memory failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }

    /* message */
    osapi_strncpy(ptr_msg->cq_name, TELNET_GET_QUEUE_NAME, DB_Q_NAME_SIZE);

    ptr_msg->method = method;
    ptr_msg->type.count = 1;

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    TELNET_LOG_DEBUG("[%p]send method:0x%x, table:%d, field:%d, entry:%d, queue:[%s], data:[%d], size:[%d]",
            ptr_msg,
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
    rc = mw_telnet_queue_send_to_db(ptr_msg, msg_size);

    if (rc == MW_E_OK)
    {
        if (NULL != pptr_out_msg)
        {
            *pptr_out_msg = ptr_msg;
        }
    }
    else
    {
        TELNET_LOG_ERROR("Send message to DB failed(%d)", rc);
        MW_FREE(ptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
mw_telnet_queue_recv(
    void **pptr_buf)
{
    MW_ERROR_NO_T rc;
    UI8_T *ptr_msg = NULL;

    rc = osapi_msgRecv(TELNET_GET_QUEUE_NAME, &ptr_msg, 0, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    TELNET_LOG_DEBUG("ptr_msg=%p", ptr_msg);
    (*pptr_buf) = ptr_msg;

    return MW_E_OK;
}

MW_ERROR_NO_T
mw_telnet_queue_getData(
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
        TELNET_LOG_ERROR("dbapi_getDataSize failed(%d)", rc);
        return rc;
    }
    TELNET_LOG_DEBUG("dbapi_getDataSize sucess, total_size = %d", total_size);

    rc = mw_telnet_queue_send(M_GET, in_t_idx, in_f_idx, in_e_idx, NULL, total_size, &ptr_msg);
    if (MW_E_OK != rc)
    {
        TELNET_LOG_ERROR("mw_telnet_queue_send failed(%d)", rc);
        return rc;
    }

    /* wait for DB response messgae */
    rc = mw_telnet_queue_recv((void **)&ptr_msg);
    if (MW_E_OK == rc)
    {
        TELNET_LOG_DEBUG("mw_telnet_queue_recv success");
    }
    else
    {
        TELNET_LOG_ERROR("mw_telnet_queue_recv failed(%d)");
        MW_FREE(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    (*ptr_out_size) = total_size;

    ptr_pload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    (*pptr_out_data) = &(ptr_pload->ptr_data);

    TELNET_LOG_DEBUG("pptr_out_msg = %p, *pptr_out_data = %p", *pptr_out_msg, *pptr_out_data);

    return MW_E_OK;
}