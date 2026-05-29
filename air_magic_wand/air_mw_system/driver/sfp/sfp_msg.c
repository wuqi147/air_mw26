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

/* FILE NAME:   sfp_msg.c
 * PURPOSE:
 *      Provide the interfaces for SFP messages.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_memory.h"
#include "sfp_task.h"
#include "sfp_msg.h"
#include "sfp_util.h"

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
#ifndef AIR_SUPPORT_SFP_WITH_THREAD
static const C8_T * _ptr_queue_name = NULL;  /* Point to a const string. */
#endif

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_msg_set_queue_name
 * PURPOSE:
 *      Set the queue name. The SFP messages will be sent to the queue.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                 -- All port settings are set.
 *      FALSE                -- Not all port settings are set.
 *
 * NOTES:
 *      None.
 */
void
sfp_msg_set_queue_name(
    const C8_T * const ptr_queue_name)
{
#ifndef AIR_SUPPORT_SFP_WITH_THREAD
    _ptr_queue_name = ptr_queue_name;
#endif
}

/* FUNCTION NAME:   sfp_get_port_inited
 * PURPOSE:
 *      Check if all port settings are complete for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                 -- All port settings are set.
 *      FALSE                -- Not all port settings are set.
 *
 * NOTES:
 *      None.
 */
const C8_T * const
sfp_msg_get_queue_name(
    void)
{
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
    return SFP_MSG_QUEUE_NAME;
#else
    return _ptr_queue_name;
#endif
}

/* FUNCTION NAME:   sfp_msg_create
 * PURPOSE:
 *      Create a message which is to be sent to the SFP task queue.
 *
 * INPUT:
 *      unit            -- Device unit number
 *      msg_id          -- Message ID
 *      data_size       -- The size of data pointed by ptr_data of the message
 * OUTPUT:
 *      None
 * RETURN:
 *      Not NULL        -- A pointer points to the message created.
 *      NULL            -- Failed.
 *
 * NOTES:
 *      None
 */
SFP_MSG_T *
sfp_msg_create(
    const UI8_T unit,
    const UI8_T msg_id,
    const UI32_T data_size)
{
    SFP_MSG_T *ptr_message = NULL;

    osapi_calloc(sizeof(SFP_MSG_T) + data_size, SFP_TASK_NAME, (void**)&ptr_message);
    if (NULL != ptr_message)
    {
        ptr_message->unit = unit;
        ptr_message->msg_id = msg_id;
        if (0 == data_size)
        {
            ptr_message->ptr_data = NULL;
        }
        else
        {
            ptr_message->ptr_data = (UI8_T *)ptr_message + sizeof(SFP_MSG_T);
            memset(ptr_message->ptr_data, 0, data_size);
        }
    }

    return ptr_message;
}

/* FUNCTION NAME:   sfp_msg_send
 * PURPOSE:
 *      Send a message to the SFP task queue.
 *
 * INPUT:
 *      ptr_message            -- A pointer points to the message to be sent
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK               -- Operation success
 *      MW_E_ENTRY_NOT_FOUND  -- The SFP task queue is not found
 *      MW_E_TIMEOUT          -- Timeout
 *      MW_E_BAD_PARAMETER    -- Parameter is wrong
 *
 * NOTES:
 *      Please free the message if an error returns.
 */
MW_ERROR_NO_T
sfp_msg_send(
    SFP_MSG_T *ptr_message)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    const C8_T *ptr_queue_name = sfp_msg_get_queue_name();

    ret = osapi_msgSend(ptr_queue_name, (UI8_T *)ptr_message, 0, 0);
    if (MW_E_OK != ret)
    {
        SFP_LOG_ERROR("Fail to send SFP msg. msg_id:%d", ptr_message->msg_id);
    }

    return ret;
}

