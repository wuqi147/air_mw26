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

/* FILE NAME:  remote_debug_log.c
 * PURPOSE:
 * It provides remote dubug log module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef AIR_SUPPORT_REMOTE_DEBUG

#include "mw_types.h"
#include "mw_error.h"
#include "osapi_thread.h"
#include "osapi_message.h"
#include "osapi_memory.h"
#include "osapi_mutex.h"
#include "inc/remote_debug_log.h"
#include "mw_telnet.h"
#include "sys_mgmt.h"

/* GLOBAL VARIABLE DECLARATIONS
*/

static REMOTE_DEBUG_LOG_CNTX_T   remote_debug_log_cntx;
static REMOTE_DEBUG_LOG_ENTRY_T  *ptr_entry = NULL;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
*/

/* FUNCTION NAME:   _remote_debug_log_find_empty_entry
 * PURPOSE:
 *      Find empty log ptr_entry.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      empty ptr_entry index
 *
 * NOTES:
 *      None
 */
static UI8_T
_remote_debug_log_find_empty_entry(void)
{
    UI8_T count = 0;
    UI8_T check_idx;

    check_idx = (remote_debug_log_cntx.current_idx + 1) % REMOTE_DEBUG_LOG_MAX_ENTRY_NUM;

    while (count < REMOTE_DEBUG_LOG_MAX_ENTRY_NUM)
    {
        if(0 == ptr_entry[check_idx].ref_cnt)
        {
            return check_idx;
        }
        else
        {
            check_idx = (check_idx + 1) % REMOTE_DEBUG_LOG_MAX_ENTRY_NUM;
        }
        count++;
    }
    return REMOTE_DEBUG_LOG_NO_EMPTY_ENTRY;
}


static void
_remote_debug_log_lock(void)
{
#ifdef AIR_SUPPORT_CLI
    taskENTER_CRITICAL();
#endif /*#ifdef AIR_SUPPORT_CLI*/
}

static void
_remote_debug_log_unlock(void)
{
#ifdef AIR_SUPPORT_CLI
    taskEXIT_CRITICAL();
#endif /*#ifdef AIR_SUPPORT_CLI*/
}

static void
_remote_debug_log_notify_consumers(
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry)
{
    UI8_T i;

    for (i = 0; i < REMOTE_DEBUG_LOG_MAX_SESSION_NUM; i++)
    {
        if (NULL != remote_debug_log_cntx.sessions[i].notify_func)
        {
            ptr_entry->ref_cnt++;
            if (MW_E_OK != remote_debug_log_cntx.sessions[i].notify_func(ptr_entry, remote_debug_log_cntx.sessions[i].ptr_cookie))
            {
                /* Decrement ref_cnt if notification failed */
                if (ptr_entry->ref_cnt > 0)
                {
                    ptr_entry->ref_cnt--;
                }
            }
            /* If MW_E_OK, the consumer is responsible for calling remote_debug_log_release_entry */
        }
    }
}

/* EXPORTED SUBPROGRAM BODIES
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
remote_debug_log_has_active_session(void)
{
    UI8_T i;

    for (i = 0; i < REMOTE_DEBUG_LOG_MAX_SESSION_NUM; i++)
    {
        if (NULL != remote_debug_log_cntx.sessions[i].notify_func)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* FUNCTION NAME:   sys_log_write_to_ram
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
int remote_debug_log_write_to_ram(
    int c)
{
    UI8_T index = 0;

    if (FALSE == remote_debug_log_has_active_session())
    {
        return c;
    }

    _remote_debug_log_lock();

    if (REMOTE_DEBUG_LOG_MAX_DATA_SIZE == remote_debug_log_cntx.used_size)
    {
        index = _remote_debug_log_find_empty_entry();
        if (REMOTE_DEBUG_LOG_NO_EMPTY_ENTRY == index)
        {
            /* drop log when table full */
            _remote_debug_log_unlock();
            return c;
        }
        else
        {
            remote_debug_log_cntx.current_idx = index;
            remote_debug_log_cntx.used_size = 0;
        }
    }
    ptr_entry[remote_debug_log_cntx.current_idx].data[remote_debug_log_cntx.used_size] = (UI8_T)c;
    remote_debug_log_cntx.used_size++;

    if (REMOTE_DEBUG_LOG_MAX_DATA_SIZE == remote_debug_log_cntx.used_size)
    {
        /* Prepare to notify consumers */
        REMOTE_DEBUG_LOG_ENTRY_T *p_notify_entry = &ptr_entry[remote_debug_log_cntx.current_idx];

        p_notify_entry->len = REMOTE_DEBUG_LOG_MAX_DATA_SIZE;
        p_notify_entry->msg_id = REMOTE_DEBUG_LOG_MSG_ID_NOTIFY;

        /* Notify consumers inside critical section */
        _remote_debug_log_notify_consumers(p_notify_entry);
    }

    _remote_debug_log_unlock();
    return c;
}

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
    void)
{
    /* Allocate remote debug RAM */
    if (osapi_calloc(REMOTE_DEBUG_LOG_MAX_ENTRY_NUM * sizeof(REMOTE_DEBUG_LOG_ENTRY_T), SYS_MGMT_MODULE_NAME, (void **)&ptr_entry) != MW_E_OK)
    {
        MW_LOG_DEBUG(REMOTE_DEBUG, "allocate remote debug RAM fail!");
        return MW_E_NO_MEMORY;
    }

    MW_LOG_DEBUG(REMOTE_DEBUG, "remote debug log init success");

    return MW_E_OK;
}

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
    void)
{
    if(NULL != ptr_entry)
    {
        return MW_E_OK;
    }
    else
    {
        return MW_E_NOT_INITED;
    }
}

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
remote_debug_log_free_resource()
{
    MW_FREE(ptr_entry);
}

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
    void)
{
    TickType_t check_tick = 0;
    REMOTE_DEBUG_LOG_ENTRY_T *p_notify_entry = NULL;

    /* send msg to each module when an ptr_entry is not full for a long time */
    if (TRUE == remote_debug_log_has_active_session())
    {
        if ((remote_debug_log_cntx.used_size > 0) &&
            (remote_debug_log_cntx.used_size < REMOTE_DEBUG_LOG_MAX_DATA_SIZE))
        {
            check_tick = osapi_sysTickGet() - remote_debug_log_get_tick_count();
            if (check_tick > REMOTE_DEBUG_LOG_MAX_WAIT_TIME)
            {
                _remote_debug_log_lock();

                /* Prepare notification */
                p_notify_entry = &ptr_entry[remote_debug_log_cntx.current_idx];
                p_notify_entry->len = remote_debug_log_cntx.used_size;
                p_notify_entry->msg_id = REMOTE_DEBUG_LOG_MSG_ID_NOTIFY;

                _remote_debug_log_notify_consumers(p_notify_entry);

                remote_debug_log_cntx.used_size = REMOTE_DEBUG_LOG_MAX_DATA_SIZE;

                _remote_debug_log_unlock();
            }
        }
    }
    else
    {
        /* Just setting up used_size
         * Consumer should free entry when deregister
         */
         remote_debug_log_cntx.used_size = REMOTE_DEBUG_LOG_MAX_DATA_SIZE;
    }
}

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
    TickType_t tick_count)
{
    remote_debug_log_cntx.tick_count = tick_count;
    return MW_E_OK;
}

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
    void)
{
    return remote_debug_log_cntx.tick_count;
}

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
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry)
{
    if (NULL == ptr_entry)
    {
        return;
    }

    _remote_debug_log_lock();
    if (ptr_entry->ref_cnt > 0)
    {
        ptr_entry->ref_cnt--;
    }
    _remote_debug_log_unlock();
}

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
 *      If notify_func return MW_E_OK, the consumer is responsible for decrementing ref_cnt
 */
MW_ERROR_NO_T
remote_debug_log_register(
    const REMOTE_DEBUG_LOG_NOTIFY_FUNC_T notify_func,
    void                                *ptr_cookie)
{
    UI8_T i;
    MW_ERROR_NO_T ret = MW_E_TABLE_FULL;

    if (NULL == notify_func)
    {
        return MW_E_BAD_PARAMETER;
    }

    _remote_debug_log_lock();

    for (i = 0; i < REMOTE_DEBUG_LOG_MAX_SESSION_NUM; i++)
    {
        if (NULL == remote_debug_log_cntx.sessions[i].notify_func)
        {
            remote_debug_log_cntx.sessions[i].notify_func = notify_func;
            remote_debug_log_cntx.sessions[i].ptr_cookie = ptr_cookie;
            ret = MW_E_OK;
            break;
        }
    }

    _remote_debug_log_unlock();

    if (MW_E_OK == ret)
    {
        MW_LOG_DEBUG(REMOTE_DEBUG, "register session[%d] success", i);
    }
    else
    {
        MW_LOG_DEBUG(REMOTE_DEBUG, "register session fail, table full");
    }

    return ret;
}

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
    void                                *ptr_cookie)
{
    UI8_T i;
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;

    _remote_debug_log_lock();

    for (i = 0; i < REMOTE_DEBUG_LOG_MAX_SESSION_NUM; i++)
    {
        if ((remote_debug_log_cntx.sessions[i].notify_func == notify_func) &&
            (remote_debug_log_cntx.sessions[i].ptr_cookie == ptr_cookie))
        {
            remote_debug_log_cntx.sessions[i].notify_func = NULL;
            remote_debug_log_cntx.sessions[i].ptr_cookie = NULL;

            /* If no more active sessions, set used_size to MAX value
             */
            if (FALSE == remote_debug_log_has_active_session())
            {
                remote_debug_log_cntx.used_size = REMOTE_DEBUG_LOG_MAX_DATA_SIZE;
            }

            ret = MW_E_OK;
            break;
        }
    }

    _remote_debug_log_unlock();

    if (MW_E_OK == ret)
    {
        MW_LOG_DEBUG(REMOTE_DEBUG, "deregister session[%d] success", i);
    }
    else
    {
        MW_LOG_DEBUG(REMOTE_DEBUG, "deregister session fail, not found");
    }

    return ret;
}
#endif /*#ifdef AIR_SUPPORT_REMOTE_DEBUG*/
