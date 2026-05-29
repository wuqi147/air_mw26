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

/* FILE NAME:   osapi_message.c
 * PURPOSE:
 *      Wrapper APIs for freeRTOS message queue function call.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_message.h"
#include "osapi_memory.h"
#include "mw_msg.h"
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static SLIST_HEAD(_msg_list, MSG_S) _msg_head;
static BOOL_T init_flag = FALSE;
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static msghandle_t
_list_find(
    const C8_T *ptr_name)
{
    msghandle_t handle = NULL;
    struct MSG_S *ptr_cur = NULL;

    SLIST_FOREACH(ptr_cur, &_msg_head, next)
    {
        if((!strcmp(ptr_cur->name, ptr_name)) && (strlen(ptr_cur->name) == strlen(ptr_name)))
        {
            handle = ptr_cur->handle;
            break;
        }
    }

    return handle;
}

static void
_list_add(
    const C8_T *ptr_name,
    const QueueHandle_t handle)
{
    struct MSG_S *ptr_new = NULL;

    ptr_new = pvPortMalloc(sizeof(struct MSG_S), "osmsgq");
    if(NULL != ptr_new)
    {
        memset(ptr_new->name, 0, MSG_MAX_NAME_LEN);
        strncpy(ptr_new->name, ptr_name, MSG_MAX_NAME_LEN - 1);
        ptr_new->handle = handle;
        ptr_new->next.sle_next = NULL;

        SLIST_INSERT_HEAD(&_msg_head, ptr_new, next);
    }
}

static void
_list_delete(
    const C8_T *ptr_name)
{
    struct MSG_S *ptr_cur = NULL;

    SLIST_FOREACH(ptr_cur, &_msg_head, next)
    {
        if((!strcmp(ptr_cur->name, ptr_name)) && (strlen(ptr_cur->name) == strlen(ptr_name)))
        {
            SLIST_REMOVE(&_msg_head, ptr_cur, MSG_S, next);
            vPortFree(ptr_cur);
            break;
        }
    }
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: osapi_msgCreate
 * PURPOSE:
 *      Create message queue of specific name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      length          --  The maximun munber of items the queue can hold
 *      size            --  The size of each element in the queue and the unit is byte
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgCreate(
    const C8_T *ptr_name,
    const UI32_T length,
    const UI32_T size)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);

    if(FALSE == init_flag)
    {
        SLIST_INIT(&_msg_head);
        init_flag = TRUE;
    }

    handle = xQueueCreate(length, size, ptr_name);
    if(NULL != handle)
    {
        _list_add(ptr_name, handle);
    }
    else
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgCreateStatic
 * PURPOSE:
 *      Create a static message queue of specific name
 *
 * INPUT:
 *      ptr_name         --  A pointer to the queue name
 *      length           --  The maximun munber of items the queue can hold
 *      size             --  The size of each element in the queue and the unit is byte
 *      ptr_queueStorage --  A pointer points to a uint8_t array which can hold
 *                           at least length x size bytes to store the queue items
 *      ptr_staticQueue  --  A pointer points to a variable of type
 *                           staticMsghandle_t to hold the queue's data structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgCreateStatic(
    const C8_T *ptr_name,
    const UI32_T length,
    const UI32_T size,
    UI8_T *ptr_queueStorage,
    staticMsghandle_t *ptr_staticQueue)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);

    if(FALSE == init_flag)
    {
        SLIST_INIT(&_msg_head);
        init_flag = TRUE;
    }

    handle = xQueueCreateStatic(length, size, ptr_queueStorage, ptr_staticQueue);
    if(NULL != handle)
    {
        _list_add(ptr_name, handle);
    }
    else
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgSend
 * PURPOSE:
 *      Send data to specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      ptr_msg         --  A pointer to the message buffer
 *      size            --  Size of the data in message buffer
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
osapi_msgSend(
    const C8_T *ptr_name,
    const UI8_T *ptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(ptr_msg);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueSend(handle, &ptr_msg, (timeout / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgSendToFront
 * PURPOSE:
 *      Send data to the front of a specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      ptr_msg         --  A pointer to the message buffer
 *      size            --  Size of the data in message buffer
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
osapi_msgSendToFront(
    const C8_T *ptr_name,
    const UI8_T *ptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(ptr_msg);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueSendToFront(handle, &ptr_msg, (timeout / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgRecv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      size            --  Size of the data in message buffer
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A pointer to pointer of the message buffer
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
osapi_msgRecv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(pptr_msg);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueReceive(handle, &(*pptr_msg), (timeout / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgPeek
 * PURPOSE:
 *      Peek data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      size            --  Size of the data in message buffer
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A pointer to pointer of the message buffer
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
osapi_msgPeek(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(pptr_msg);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueuePeek(handle, &(*pptr_msg), (timeout / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgDelete
 * PURPOSE:
 *      Delete message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
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
MW_ERROR_NO_T
osapi_msgDelete(
    const C8_T *ptr_name)
{
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);

    handle = _list_find(ptr_name);
    if(NULL != handle)
    {
        _list_delete(ptr_name);
        vQueueDelete(handle);
    }

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_msgWaiting
 * PURPOSE:
 *      Get the total number of messages stored in a specific queue.
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *
 * OUTPUT:
 *      ptr_waiting     --  The number of messages stored in the queue
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgWaiting(
    const C8_T *ptr_name,
    UI32_T *ptr_waiting)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(ptr_waiting);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        *ptr_waiting = uxQueueMessagesWaiting(handle);
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgFindHandle
 * PURPOSE:
 *      Find message queue handle which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      message queue handle
 *
 * NOTES:
 *      None
 */
msghandle_t
osapi_msgFindHandle(
    const C8_T *ptr_name)
{
    if(NULL == ptr_name)
        return NULL;

    return _list_find(ptr_name);
}

/* FUNCTION NAME: osapi_msgFilter
 * PURPOSE:
 *      Filter messages in the queue.
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      filter_func     --  A pointer to the filter function.
 *                          Returns TRUE if the message should be deleted.
 *                          Returns FALSE if the message should be reserved.
 *      ptr_ctx         --  A pointer to the context
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgFilter(
    const C8_T *ptr_name,
    msg_filter_func filter_func,
    void *ptr_ctx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;
    UI32_T msg_count = 0;
    UI32_T i = 0;
    void *ptr_msg = NULL;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(filter_func);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        /* Enter critical section to protect queue operation sequence */
        taskENTER_CRITICAL();

        msg_count = (UI32_T)uxQueueMessagesWaiting(handle);
        for(i = 0; i < msg_count; i++)
        {
            /* NOTE: xTicksToWait MUST be 0 inside critical section */
            if(pdPASS == xQueueReceive(handle, &ptr_msg, 0))
            {
                if(TRUE == filter_func(ptr_msg, ptr_ctx))
                {
                    /* Drop the message */
                    continue;
                }
                else
                {
                    /* Put it back to the tail of the queue */
                    xQueueSend(handle, &ptr_msg, 0);
                }
            }
            else
            {
                ret = MW_E_OP_INVALID;
                break;
            }
        }

        taskEXIT_CRITICAL();
    }

    return ret;
}

/* FUNCTION NAME: osapi_socketCreate
 * PURPOSE:
 *      Create message queue of specific name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      length          --  The maximun munber of items the queue can hold
 *      size            --  The size of each element in the queue and the unit is byte
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer of the queue handle
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_socketCreate(
    const C8_T *ptr_name,
    const UI32_T length,
    const UI32_T size,
    msghandle_t *pptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t ptr_handle = NULL;

    MW_CHECK_PTR(pptr_handle);

    ptr_handle = xQueueCreate(length, size, ptr_name);
    if(NULL != ptr_handle)
    {
        *pptr_handle = ptr_handle;
    }
    else
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_socketSend
 * PURPOSE:
 *      Send data to specific message queue which depending on name
 *
 * INPUT:
 *      ptr_handle      --  A pointer to the queue handle
 *      ptr_msg         --  A pointer to the message buffer
 *      size            --  Size of the data in message buffer
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_socketSend(
    const msghandle_t ptr_handle,
    const UI8_T *ptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    NET_MSG_T *ptr_netMsg = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_handle);
    MW_CHECK_PTR(ptr_msg);

    ptr_netMsg = pvPortMalloc(sizeof(NET_MSG_T), "pbufQ");
    if (NULL != ptr_netMsg)
    {
        ptr_netMsg->msg_id = MW_MSG_ID_ETHERNET_PBUF;
        ptr_netMsg->ptr_pbuf = (void *)ptr_msg;
        if (pdPASS != xQueueSend(ptr_handle, &ptr_netMsg, (timeout / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
            MW_FREE(ptr_netMsg);
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_socketRecv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_handle      --  A pointer to the queue handle
 *      size            --  Size of the data in message buffer
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A pointer to pointer to the message buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_socketRecv(
    const msghandle_t ptr_handle,
    UI8_T **pptr_msg,
    const UI32_T size,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    NET_MSG_T *ptr_netMsg = NULL;

    UNUSED(size);
    MW_CHECK_PTR(ptr_handle);
    MW_CHECK_PTR(pptr_msg);

    if(pdPASS != xQueueReceive(ptr_handle, &ptr_netMsg, (timeout / portTICK_RATE_MS)))
    {
        ret = MW_E_TIMEOUT;
    }
    else
    {
        *pptr_msg = ptr_netMsg->ptr_pbuf;
        MW_FREE(ptr_netMsg);
        ret = ((*pptr_msg) == NULL) ? MW_E_OTHERS : MW_E_OK;
    }

    return ret;
}

/* FUNCTION NAME: osapi_socketDelete
 * PURPOSE:
 *      Delete message queue which depending on name
 *
 * INPUT:
 *      ptr_handle      --  A pointer to the queue handle
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
MW_ERROR_NO_T
osapi_socketDelete(
    const msghandle_t ptr_handle)
{
    MW_CHECK_PTR(ptr_handle);

    vQueueDelete(ptr_handle);

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_netRegister
 * PURPOSE:
 *      Register net filter to mac driver
 *
 * INPUT:
 *      ptr_netf        --  A pointer to the OS_NET_FILTER_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_netRegister(
    const NET_FILTER_T *ptr_netf)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_netf);

    err_t res = ethernetif_regHandleFind(&handle);
    if((ERR_OK != res) || (NULL == handle))
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueSend(handle, &ptr_netf, (100 / portTICK_RATE_MS)))
        {
            ret = MW_E_TIMEOUT;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgPollCreate
 * PURPOSE:
 *      Create queue to block on a read operation from multiple queue
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name set
 *      length          --  The maximun munber of items the queue can hold
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgPollCreate(
    const C8_T *ptr_name,
    const UI32_T length)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);
    MW_PARAM_CHK((0 == length), MW_E_BAD_PARAMETER);

    if(FALSE == init_flag)
    {
        SLIST_INIT(&_msg_head);
        init_flag = TRUE;
    }

    handle = xQueueCreateSet(length, ptr_name);
    if(NULL != handle)
    {
        _list_add(ptr_name, handle);
    }
    else
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgPollCtrlAdd
 * PURPOSE:
 *      Add a queue handle to the polling list for waiting message
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name set
 *      ptr_handle      --  A pointer to the queue handle need to add to queue set
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgPollCtrlAdd(
    const C8_T *ptr_name,
    const msghandle_t ptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(ptr_handle);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueAddToSet(ptr_handle, handle))
        {
            ret = MW_E_OTHERS;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgPollCtrlRmv
 * PURPOSE:
 *      Remove a queue handle to the polling list
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      ptr_handle      --  A pointer to the queue handle need to remove from queue set
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgPollCtrlRmv(
    const C8_T *ptr_name,
    const msghandle_t ptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(ptr_handle);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if(pdPASS != xQueueRemoveFromSet(ptr_handle, handle))
        {
            ret = MW_E_OTHERS;
        }
    }

    return ret;
}

/* FUNCTION NAME: osapi_msgPoll
 * PURPOSE:
 *      Polling queue handle status and return change queue handle
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name set
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for status change of specific queue handle
 *                          (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer of the queue handle
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_msgPoll(
    const C8_T *ptr_name,
    const UI32_T timeout,
    msghandle_t *pptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    msghandle_t handle = NULL;

    MW_CHECK_PTR(ptr_name);

    handle = _list_find(ptr_name);
    if(NULL == handle)
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    else
    {
        *pptr_handle = xQueueSelectFromSet(handle, (timeout / portTICK_RATE_MS));
    }

    return ret;
}
