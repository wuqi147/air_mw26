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

/* FILE NAME:  remote_debug_log.h
 * PURPOSE:
 * It provides remote debug log module API and definitions.
 *
 * NOTES:
 */

#ifndef REMOTE_DEBUG_LOG_H
#define REMOTE_DEBUG_LOG_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_msg.h"
#include "osapi.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define REMOTE_DEBUG_LOG_NAME                            "rmtdbg"
#define REMOTE_DEBUG_LOG_MAX_ENTRY_NUM                   (10)
#define REMOTE_DEBUG_LOG_MAX_DATA_SIZE                   (64)
#define REMOTE_DEBUG_LOG_NO_EMPTY_ENTRY                  (0xFF)
#define REMOTE_DEBUG_LOG_MAX_WAIT_TIME                   (500)
#define REMOTE_DEBUG_LOG_MSG_ID_NOTIFY                   (MW_MSG_ID_SELF_DEFINED_BASE)
#define REMOTE_DEBUG_LOG_MAX_SESSION_NUM                 (3)
#define REMOTE_DEBUG_LOG_WAITTIME                        (0xFFFFFFFF)

/* MACRO FUNCTION DECLARATIONS
 */
extern int uart_printf(const char *fmt, ...);

/* DATA TYPE DECLARATIONS
 */
typedef struct REMOTE_DEBUG_LOG_ENTRY_S{
    UI8_T  msg_id;
    UI8_T  ref_cnt;  /* Counter of consumers currently using this ptr_entry */
    UI8_T  len;      /* length of the data sent */
    UI8_T  data[REMOTE_DEBUG_LOG_MAX_DATA_SIZE];
}REMOTE_DEBUG_LOG_ENTRY_T;

/* If notify_func return MW_E_OK, the consumer is responsible for calling remote_debug_log_release_entry */
/* The callback function should not be blocked. Otherwise, the system may hang. */
typedef MW_ERROR_NO_T (*REMOTE_DEBUG_LOG_NOTIFY_FUNC_T)(
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry,
    void                     *ptr_cookie);

typedef struct REMOTE_DEBUG_LOG_REG_ENTRY_S{
    REMOTE_DEBUG_LOG_NOTIFY_FUNC_T notify_func;
    void                          *ptr_cookie;
} REMOTE_DEBUG_LOG_REG_ENTRY_T;

typedef struct REMOTE_DEBUG_LOG_CNTX_S{
    TickType_t                  tick_count;
    UI8_T                       current_idx;
    UI8_T                       used_size;
    REMOTE_DEBUG_LOG_REG_ENTRY_T sessions[REMOTE_DEBUG_LOG_MAX_SESSION_NUM];
} __attribute__((packed)) REMOTE_DEBUG_LOG_CNTX_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   remote_debug_log_has_active_session
 * PURPOSE:
 *      Check if any session is registered.
 * INPUT:
 *      c   - input char
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE if at least one session is active.
 *
 * NOTES:
 *      None
 */
BOOL_T
remote_debug_log_has_active_session(void);

/* FUNCTION NAME:   remote_debug_log_write_to_ram
 * PURPOSE:
 *      Save log to RAM.
 *
 * INPUT:
 *      c   - input char
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      c
 *
 * NOTES:
 *      None
 */
int
remote_debug_log_write_to_ram(
    int c);

/* FUNCTION NAME:   remote_debug_log_init_resource
 * PURPOSE:
 *      Initialize remote debug log module RAM.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK : allocate memory successfully
 *      MW_E_NO_MEMORY : allocate memory failed
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
remote_debug_log_init_resource(
    void);

/* FUNCTION NAME:   remote_debug_log_init_done
 * PURPOSE:
 *      Check if remote debug log module init done.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK if inited
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
remote_debug_log_init_done(
    void);

/* FUNCTION NAME:   remote_debug_log_free_resource
 * PURPOSE:
 *      Free remote debug log module RAM.
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
remote_debug_log_free_resource(
    void);

/* FUNCTION NAME:   remote_debug_log_task_handler
 * PURPOSE:
 *      remote debug log task handler.
 *      It will send msg to each module when an ptr_entry is not full for a long time.
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
remote_debug_log_task_handler(
   void);

/* FUNCTION NAME:   remote_debug_set_tick_count
 * PURPOSE:
 *      Set update tick
 *
 * INPUT:
 *      tick_count
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
remote_debug_set_tick_count(
    TickType_t tick_count);

/* FUNCTION NAME:   remote_debug_log_get_tick_count
 * PURPOSE:
 *      Get update tick
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      update_tick
 *
 * NOTES:
 *      None
 */
TickType_t
remote_debug_log_get_tick_count(
    void);

/* FUNCTION NAME:   remote_debug_log_release_entry
 * PURPOSE:         Decrease ref_cnt of the log entry.
 *
 * INPUT:
 *      ptr_entry       -- Pointer to the log entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      This function is thread-safe.
 */
void
remote_debug_log_release_entry(
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   remote_debug_log_register
 * PURPOSE:         Register a callback function to handle log notification.
 *
 * INPUT:
 *      notify_func             -- Callback function
 *      ptr_cookie              -- Cookie data of callback function
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK                -- Operation success.
 *      MW_E_BAD_PARAMETER     -- Parameter is wrong.
 *      MW_E_TABLE_FULL        -- Table is full.
 *
 * NOTES:
 *      If notify_func return MW_E_OK, the consumer is responsible for calling remote_debug_log_release_entry
 */
MW_ERROR_NO_T
remote_debug_log_register(
    const REMOTE_DEBUG_LOG_NOTIFY_FUNC_T notify_func,
    void                                *ptr_cookie);

/* FUNCTION NAME:   remote_debug_log_deregister
 * PURPOSE:         Deregister a callback function.
 *
 * INPUT:
 *      notify_func             -- Callback function
 *      ptr_cookie              -- Cookie data of callback function
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK                -- Operation success.
 *      MW_E_ENTRY_NOT_FOUND   -- Entry is not found.
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
remote_debug_log_deregister(
    const REMOTE_DEBUG_LOG_NOTIFY_FUNC_T notify_func,
    void                                *ptr_cookie);

#endif /* End of REMOTE_DEBUG_LOG_H */
