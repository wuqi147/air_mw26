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

/* FILE NAME:  osapi_message.h
 * PURPOSE:
 *      Wrapper APIs for freeRTOS message queue function call.
 *
 * NOTES:
 */

#ifndef OSAPI_MESSAGE_H
#define OSAPI_MESSAGE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MSG_MAX_NAME_LEN                 (10)
#define MAC_ADDR_MAX_LEN                 (6)
#define MSG_TIMEOUT_WAIT_INDEFINITELY    (0xFFFFFFFF)
#define MSG_TIMEOUT_RETRUN_IMMEDIATELY   (0)

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T(*msg_filter_func)(void *ptr_msg, void *ptr_ctx);

struct MSG_S
{
    C8_T name[MSG_MAX_NAME_LEN];
    msghandle_t handle;
    SLIST_ENTRY(MSG_S) next;
};

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
    const UI32_T size);

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
    staticMsghandle_t *ptr_staticQueue);

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
    const UI32_T timeout);

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
    const UI32_T timeout);

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
    const UI32_T timeout);

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
    const UI32_T timeout);

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
    const C8_T *ptr_name);

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
    UI32_T *ptr_waiting);

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
    const C8_T *ptr_name);

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
    void *ptr_ctx);

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
    msghandle_t *pptr_handle);

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
    const UI32_T timeout);

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
    const UI32_T timeout);

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
    const msghandle_t ptr_handle);

/* FUNCTION NAME: osapi_netRegister
 * PURPOSE:
 *      Register net filter to mac driver
 *
 * INPUT:
 *      ptr_netf        --  A pointer to the NET_FILTER_T
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
    const NET_FILTER_T *ptr_netf);

/* FUNCTION NAME: osapi_msgPollCreate
 * PURPOSE:
 *      Create queue to block on a read operation from multiple queue
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
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
    const UI32_T length);

/* FUNCTION NAME: osapi_msgPollCtrlAdd
 * PURPOSE:
 *      Add a queue handle to the polling list for waitting message
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      ptr_handle      --  A pointer to the queue handle need to add to list
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
    const msghandle_t ptr_handle);

/* FUNCTION NAME: osapi_msgPollCtrlRmv
 * PURPOSE:
 *      Remove a queue handle to the polling list
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      ptr_handle      --  A pointer to the queue handle need to remove from list
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
    const msghandle_t ptr_handle);

/* FUNCTION NAME: osapi_msgPoll
 * PURPOSE:
 *      Polling queue handle status and return change queue handle
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
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
    msghandle_t *pptr_handle);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/

#endif  /* #ifndef OSAPI_MESSAGE_H */

