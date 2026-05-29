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

/* FILE NAME:  erps_queue.c
 * PURPOSE:
 *    This file contains the implementation of ERPS queue.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "erps.h"
#include "erps_queue.h"

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
static ERPS_QUEUE_CONTEXT_T     _erps_queue_ctx = {0};

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: erps_queue_init
 * PURPOSE:
 *      Initialize the queues for ERPS.
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
erps_queue_init(
    void)
{
    MW_ERROR_NO_T           ret = MW_E_OK;
    QueueHandle_t           ptr_handle = NULL;

    osapi_memset(&_erps_queue_ctx, 0, sizeof(_erps_queue_ctx));

    ret = osapi_msgPollCreate(ERPS_QUEUE_SET_NAME, ERPS_QUEUE_TOTAL_LENGTH);
    if (ret == MW_E_OK)
    {
        _erps_queue_ctx.erps_queue_set_handle = osapi_msgFindHandle(ERPS_QUEUE_SET_NAME);
    }
    else
    {
        ERPS_LOG_ERROR("Create msg poll for ERPS set failed, ret:%d.", ret);
        erps_queue_deinit();
        return  MW_E_NOT_INITED;
    }
    /* Create a static queue for the interaction with the ERPS task. */
    ret = osapi_msgCreateStatic(ERPS_PKT_QUEUE_NAME,
                                ERPS_PKT_QUEUE_LENGTH,
                                ERPS_NORMAL_QUEUE_MSG_SIZE,
                                (UI8_T *)(_erps_queue_ctx.pktQueueStorage),
                                &(_erps_queue_ctx.pktQueueStruct));

    ret |= osapi_msgCreateStatic(ERPS_MW_MSG_QUEUE_NAME,
                                ERPS_MW_MSG_QUEUE_LENGTH,
                                ERPS_NORMAL_QUEUE_MSG_SIZE,
                                (UI8_T *)(_erps_queue_ctx.mwQueueStorage),
                                &(_erps_queue_ctx.mwQueueStruct));

    ret |= osapi_msgCreateStatic(ERPS_EVT_QUEUE_NAME,
                                ERPS_EVT_QUEUE_LENGTH,
                                ERPS_EVT_QUEUE_MSG_SIZE,
                                (UI8_T *)(_erps_queue_ctx.evtQueueStorage),
                                &(_erps_queue_ctx.evtQueueStruct));
    if(MW_E_OK != ret)
    {
        ERPS_LOG_ERROR("erps queue not init.");
        erps_queue_deinit();
        return  MW_E_NOT_INITED;
    }

    ptr_handle = (QueueHandle_t) &(_erps_queue_ctx.pktQueueStruct);
    ret = osapi_msgPollCtrlAdd(ERPS_QUEUE_SET_NAME, ptr_handle);
    ptr_handle = (QueueHandle_t) &(_erps_queue_ctx.mwQueueStruct);
    ret |= osapi_msgPollCtrlAdd(ERPS_QUEUE_SET_NAME, ptr_handle);
    ptr_handle = (QueueHandle_t) &(_erps_queue_ctx.evtQueueStruct);
    ret |= osapi_msgPollCtrlAdd(ERPS_QUEUE_SET_NAME, ptr_handle);
    if(MW_E_OK != ret)
    {
        ERPS_LOG_ERROR("Failed to add msg poll for ERPS queues, ret:%d.", ret);
        erps_queue_deinit();
        return  MW_E_NOT_INITED;
    }


    return MW_E_OK;
}

/* FUNCTION NAME: erps_queue_deinit
 * PURPOSE:
 *      Release all allocated memory in erps queue.
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
erps_queue_deinit(
    void)
{
    osapi_msgDelete(ERPS_PKT_QUEUE_NAME);
    osapi_msgDelete(ERPS_MW_MSG_QUEUE_NAME);
    osapi_msgDelete(ERPS_EVT_QUEUE_NAME);
    osapi_msgDelete(ERPS_QUEUE_SET_NAME);

    return;
}

/* FUNCTION NAME: erps_queue_db_send
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
erps_queue_db_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    void *ptr_data,
    UI32_T timeout)
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

    ptr_msg = dbapi_createMsgByReq(ptr_queue_name, method, 1, &request, (UI16_T *)&msg_size, (UI8_T **)&ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    ERPS_LOG_DEBUG("cq_name=%s method=0x%X count=%u t_idx=%u f_idx=%u e_idx=%u",
                ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, t_idx, f_idx, e_idx);

    ret = dbapi_appendMsgPayload(&request, ptr_data, &ptr_msg, (UI16_T *)&msg_size, (UI8_T **)&ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    /* Send the message to DB */
    ret = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        ERPS_LOG_ERROR("Fail to send the msg to DB");
    }

    return ret;
}

/* FUNCTION NAME: erps_queue_recv
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
erps_queue_recv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T timeout)
{
    return osapi_msgRecv(ptr_name, pptr_msg, 0, timeout);
}

/* FUNCTION NAME: erps_queue_send
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
MW_ERROR_NO_T
erps_queue_send(
    const C8_T *ptr_name,
    const UI8_T *ptr_msg,
    const UI32_T timeout)
{
    return osapi_msgSend(ptr_name, ptr_msg, 0, timeout);
}

/* FUNCTION NAME: erps_pkt_queue_handle_get
 * PURPOSE:
 *      Get erps packet queue handle.
 *
 * INPUT:
  *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      erps queue handle
 *
 * NOTES:
 *      None
 */
msghandle_t
erps_pkt_queue_handle_get(
    void)
{
    return (msghandle_t)&(_erps_queue_ctx.pktQueueStruct);
}

/* FUNCTION NAME: erps_evt_queue_handle_get
 * PURPOSE:
 *      Get erps event queue handle.
 *
 * INPUT:
  *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      erps queue handle
 *
 * NOTES:
 *      None
 */
msghandle_t
erps_evt_queue_handle_get(
    void)
{
    return (msghandle_t)&(_erps_queue_ctx.evtQueueStruct);
}

/* FUNCTION NAME: erps_queue_context_get
 * PURPOSE:
 *      Get erps queue context.
 *
 * INPUT:
  *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to erps queue context
 *
 * NOTES:
 *      None
 */
ERPS_QUEUE_CONTEXT_T *
erps_queue_context_get(
    void)
{
    return ((ERPS_QUEUE_CONTEXT_T *)&(_erps_queue_ctx));
}

/* FUNCTION NAME: erps_queue_link_change_noti
 * PURPOSE:
 *      Notify the link change event to erps queue.
 *
 * INPUT:
 *      ptr_data        --  pointer to data
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
erps_queue_link_change_noti(
    void *ptr_arg)
{
    QueueHandle_t               ptr_erps_evt_queue = erps_evt_queue_handle_get();
    ERPS_QUEUE_EVT_MSG_T        evt_msg = {0};

    evt_msg.msg_id = MW_MSG_ID_ERPS_LINK_STATUS_CHANGE_NOTI;
    osapi_memcpy(&evt_msg.data, ptr_arg, sizeof(UI32_T));
    /* This function will be called by the GSW interrupt handler. */
    xQueueSendFromISR(ptr_erps_evt_queue, &evt_msg, NULL);

    return;
}

/* FUNCTION NAME: erps_queue_port_admin_state_change_noti
 * PURPOSE:
 *      Notify the port admin state change event to erps queue.
 *
 * INPUT:
 *      ptr_data        --  pointer to data
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
erps_queue_port_admin_state_change_noti(
    void *ptr_arg)
{
    QueueHandle_t               ptr_erps_evt_queue = erps_evt_queue_handle_get();
    ERPS_QUEUE_EVT_MSG_T        evt_msg = {0};

    evt_msg.msg_id = MW_MSG_ID_ERPS_PORT_ADMIN_STATE_CHANGE_NOTI;
    if(NULL != ptr_arg)
    {
        osapi_memcpy(&evt_msg.data, ptr_arg, sizeof(UI32_T));
    }
    xQueueSend(ptr_erps_evt_queue, &evt_msg, 0);

    return;
}