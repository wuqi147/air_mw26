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

/* FILE NAME:  erps_queue.h
 * PURPOSE:
 *      This file defines the data structure for ERPS queue.
 *
 * NOTES:
 */
#ifndef ERPS_QUEUE_H
#define ERPS_QUEUE_H
/* INCLUDE FILE DECLARATIONS
 */
#include    "mw_msg.h"
#include    "osapi_message.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define ERPS_QUEUE_SET_NAME                 "eps"
#define ERPS_PKT_QUEUE_NAME                 "epp"
#define ERPS_MW_MSG_QUEUE_NAME              "epm"
#define ERPS_EVT_QUEUE_NAME                 "epe"
#define ERPS_PKT_QUEUE_LENGTH               (32)
#define ERPS_MW_MSG_QUEUE_LENGTH            (8)
#define ERPS_EVT_QUEUE_LENGTH               (16)
#define ERPS_QUEUE_TOTAL_LENGTH             (ERPS_PKT_QUEUE_LENGTH + ERPS_MW_MSG_QUEUE_LENGTH + ERPS_EVT_QUEUE_LENGTH)
#define ERPS_NORMAL_QUEUE_MSG_SIZE          (sizeof(MW_MSG_T *))
#define ERPS_EVT_QUEUE_MSG_SIZE             (sizeof(ERPS_QUEUE_EVT_MSG_T))

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct ERPS_QUEUE_EVT_MSG_S
{
    UI8_T                       msg_id;
    UI32_T                      data;                            /* User data */
} ERPS_QUEUE_EVT_MSG_T;

typedef struct
{
    QueueSetHandle_t                        erps_queue_set_handle;
    staticMsghandle_t                       pktQueueStruct;     /* Packet queue is a static queue. */
    staticMsghandle_t                       mwQueueStruct;      /* MW message queue is a static queue. */
    staticMsghandle_t                       evtQueueStruct;     /* Event queue is a static queue. */
    UI8_T                                   pktQueueStorage[ERPS_NORMAL_QUEUE_MSG_SIZE * ERPS_PKT_QUEUE_LENGTH];
    UI8_T                                   mwQueueStorage[ERPS_NORMAL_QUEUE_MSG_SIZE * ERPS_MW_MSG_QUEUE_LENGTH];
    UI8_T                                   evtQueueStorage[ERPS_EVT_QUEUE_MSG_SIZE * ERPS_EVT_QUEUE_LENGTH];
} ERPS_QUEUE_CONTEXT_T;

typedef enum
{
    MW_MSG_ID_ERPS_LINK_STATUS_CHANGE_NOTI         = MW_MSG_ID_SELF_DEFINED_BASE,
    MW_MSG_ID_ERPS_SFP_RX_LOSS_CHANGE_NOTI,
    MW_MSG_ID_ERPS_PORT_ADMIN_STATE_CHANGE_NOTI,
    MW_MSG_ID_ERPS_SW_TIMER_EXPIRED_NOTI,
    MW_MSG_ID_ERPS_HW_TIMER_EXPIRED_NOTI,

    MW_MSG_ID_ERPS_LAST
} ERPS_MSG_ID_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);


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
    void);

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
    UI32_T timeout);

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
    const UI32_T timeout);


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
    const UI32_T timeout);

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
    void);

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
    void);

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
    void);

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
    void *ptr_arg);

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
    void *ptr_arg);

#endif  /* ERPS_QUEUE_H */
