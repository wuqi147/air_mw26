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

/* FILE NAME:  erps_timer.c
 * PURPOSE:
 *    This file contains the implementation of ERPS timer.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "erps.h"
#include    "erps_timer.h"
#include    "erps_queue.h"
#include    "erps_hw_init.h"

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
static void
_erps_sw_timer(
    TimerHandle_t xTimer);

static void
_erps_hw_timer(
    void *);

MW_ERROR_NO_T
_erps_hw_timer_start(
    UI32_T  time_ms);

MW_ERROR_NO_T
_erps_hw_timer_stop(
    void);

/* STATIC VARIABLE DECLARATIONS
 */
static ERPS_TIMER_T         _erps_timer_instance[ERPS_MAX_TIMER_NUM] = {{0}};
static TimerHandle_t        _erps_timer_handle = NULL;
static ERPS_TIMER_STATE_T   _erps_timer_state = ERPS_TIMER_STATE_OFF;

/* LOCAL SUBPROGRAM BODIES
 */
static void
_erps_sw_timer(
    TimerHandle_t xTimer)
{
    QueueHandle_t               ptr_erps_evt_queue = erps_evt_queue_handle_get();
    ERPS_QUEUE_EVT_MSG_T        evt_msg = {0};

    UNUSED(xTimer);
    evt_msg.msg_id = MW_MSG_ID_ERPS_SW_TIMER_EXPIRED_NOTI;
    xQueueSend(ptr_erps_evt_queue, &evt_msg, 0);
    return;
}

static void
_erps_hw_timer(
    void *ptr_arg)
{
    QueueHandle_t               ptr_erps_evt_queue = erps_evt_queue_handle_get();
    ERPS_QUEUE_EVT_MSG_T        evt_msg = {0};

    UNUSED(ptr_arg);
    evt_msg.msg_id = MW_MSG_ID_ERPS_HW_TIMER_EXPIRED_NOTI;
    /* This function will be called by the hardware timer interrupt handler. */
    xQueueSendFromISR(ptr_erps_evt_queue, &evt_msg, NULL);
    return;
}

MW_ERROR_NO_T
_erps_hw_timer_start(
    UI32_T  time_ms)
{
    UI32_T          time_len = (time_ms / portTICK_RATE_MS);

    erps_timer_start(time_len, NULL, _erps_hw_timer);
    return MW_E_OK;
}

MW_ERROR_NO_T
_erps_hw_timer_stop(
    void)
{
    erps_timer_stop();

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   erps_sw_timer_init
 * PURPOSE:
 *      This API is used to initial ERPS software timer.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_sw_timer_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    rc = osapi_timerCreate(ERPS_SW_TMR_NAME, _erps_sw_timer, TRUE, ERPS_SW_TMR_INTERVAL, 0, &_erps_timer_handle);
    if(MW_E_OK != rc)
    {
        ERPS_LOG_ERROR("Create erps timer failed, rc:%d", rc);
        return MW_E_NOT_INITED;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   erps_sw_timer_deinit
 * PURPOSE:
 *      This API is used to de-initial ERPS software timer.
 *
 * INPUT:
 *      None
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
erps_sw_timer_deinit(
    void)
{
    if(NULL != _erps_timer_handle)
    {
        if(MW_E_OK == osapi_timerActive(_erps_timer_handle))
        {
            osapi_timerStop(_erps_timer_handle);
            _erps_timer_state = ERPS_TIMER_STATE_OFF;
        }
        osapi_timerDelete(_erps_timer_handle);
        _erps_timer_handle = NULL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   erps_sw_timer_start
 * PURPOSE:
 *      This API is used to start ERPS software timer.
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
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_sw_timer_start(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL == _erps_timer_handle)
    {
        ERPS_LOG_ERROR("Timer not initialized.");
        return MW_E_OTHERS;
    }
    if (ERPS_TIMER_STATE_OFF == _erps_timer_state)
    {
        rc = osapi_timerStart(_erps_timer_handle);
        if (MW_E_OK == rc)
        {
            _erps_timer_state = ERPS_TIMER_STATE_ON;
        }
    }
    return rc;
}

/* FUNCTION NAME:   erps_sw_timer_stop
 * PURPOSE:
 *      This API is used to stop ERPS software timer.
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
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_sw_timer_stop(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL == _erps_timer_handle)
    {
        ERPS_LOG_ERROR("Timer not initialized.");
        return MW_E_OTHERS;
    }
    if (ERPS_TIMER_STATE_ON == _erps_timer_state)
    {
        rc = osapi_timerStop(_erps_timer_handle);
        if (MW_E_OK == rc)
        {
            _erps_timer_state = ERPS_TIMER_STATE_OFF;
        }
    }
    return rc;
}

/* FUNCTION NAME:   erps_hw_timer_start
 * PURPOSE:
 *      This API is used to start a hw timer instance for ERPS.
 *
 * INPUT:
 *      ptr_data             -- user data pointer
 *      time_ms              -- timeout value, unit ms
 *      callback             -- callback function
 *
 * OUTPUT:
 *      ptr_id               -- pointer to timer index
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_start(
    UI8_T                           *ptr_id,
    UI32_T                          time_ms,
    void                            *ptr_data,
    ERPS_TIMEOUT_CALLBACK_FUNC_T    callback)
{
    UI8_T                       idx = 0;
    UI32_T                      cur = 0, min_time_remain = time_ms;
    MW_ERROR_NO_T               rc = MW_E_OK;

    if((NULL == ptr_id) || (NULL == callback))
    {
        return MW_E_BAD_PARAMETER;
    }
    *ptr_id = ERPS_INVALID_TIMER_IDX;
    cur = air_util_getSystemTick();

    for (idx = 0; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if (FALSE == _erps_timer_instance[idx].in_use)
        {
            osapi_memset(&_erps_timer_instance[idx], 0, sizeof(ERPS_TIMER_T));
            _erps_timer_instance[idx].in_use = TRUE;
            _erps_timer_instance[idx].time_ms = time_ms + cur; /* Absolute expire time */
            _erps_timer_instance[idx].ptr_data = ptr_data;
            _erps_timer_instance[idx].callback = callback;
            break;
        }
    }
    if(ERPS_MAX_TIMER_NUM == idx)
    {
        ERPS_LOG_ERROR("No free timer instance");
        return MW_E_NO_MEMORY;
    }

    *ptr_id = idx;
    ERPS_LOG_DEBUG("Start hw timer[%d]", idx);
    for (idx = 0; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if (TRUE == _erps_timer_instance[idx].in_use)
        {
            if(min_time_remain > (_erps_timer_instance[idx].time_ms - cur))
            {
                min_time_remain = _erps_timer_instance[idx].time_ms - cur;
            }
        }
    }
    if(min_time_remain >= time_ms)
    {
        _erps_hw_timer_start(time_ms);
        ERPS_LOG_DEBUG("Restart timer with new time:%d", time_ms);
    }

    return rc;
}

/* FUNCTION NAME:   erps_hw_timer_stop
 * PURPOSE:
 *      This API is used to start an erps hw timer instance.
 *
 * INPUT:
 *      id                   -- index of timer
 *
 * OUTPUT:
 *      pptr_data            -- double pointer to user data pointer
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_stop(
    UI8_T                           id,
    void                            **pptr_data)
{
    UI8_T                       idx = 0, is_timer_exist = FALSE;
    UI32_T                      cur = 0, min_time_remain = 0, stop_time_remain = 0;
    MW_ERROR_NO_T               rc = MW_E_OK;

    if(ERPS_MAX_TIMER_NUM <= id)
    {
        ERPS_LOG_WARN("Invalid id:%d", id);
        return MW_E_BAD_PARAMETER;
    }
    if(FALSE == _erps_timer_instance[id].in_use)
    {
        ERPS_LOG_WARN("Timer[%d] not running", id);
        return MW_E_OP_INCOMPLETE;
    }
    cur = air_util_getSystemTick();
    if(NULL != pptr_data)
    {
        *pptr_data = _erps_timer_instance[id].ptr_data;
    }
    stop_time_remain = _erps_timer_instance[id].time_ms - cur;
    min_time_remain = stop_time_remain;
    /* Free timer instance memory */
    osapi_memset(&_erps_timer_instance[id], 0, sizeof(ERPS_TIMER_T));

    /* Check whether there are other timers exist */
    for (; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if (TRUE == _erps_timer_instance[idx].in_use)
        {
            if(min_time_remain > (_erps_timer_instance[idx].time_ms - cur))
            {
                min_time_remain = _erps_timer_instance[idx].time_ms - cur;
            }
            is_timer_exist = TRUE;
        }
    }
    if(TRUE == is_timer_exist)
    {
        if(min_time_remain >= stop_time_remain)
        {
            _erps_hw_timer_start(min_time_remain);
            ERPS_LOG_DEBUG("Restart timer with new time:%d", min_time_remain);
        }
    }
    else
    {
        _erps_hw_timer_stop();
        ERPS_LOG_DEBUG("No timer need run");
    }

    return rc;
}

/* FUNCTION NAME:   erps_hw_timer_timeout_check
 * PURPOSE:
 *      This API is used to check wherether the timer timeout or not.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      MW_E_OK.
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_timeout_check
    (void)
{
    UI8_T                       idx = 0, is_timer_exist = FALSE;
    UI32_T                      cur = 0, min_time_remain = 0xFFFFFFFF;
    I32_T                       time_diff = 0;

    cur = air_util_getSystemTick();
    for (idx = 0; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if (TRUE == _erps_timer_instance[idx].in_use)
        {
            time_diff = _erps_timer_instance[idx].time_ms - cur;
            if(0 >= time_diff)
            {
                ERPS_LOG_DEBUG("Timer[%d] timeout.", idx);
                /* Timeout */
                if(NULL != _erps_timer_instance[idx].callback)
                {
                    _erps_timer_instance[idx].callback(idx, _erps_timer_instance[idx].ptr_data);
                }
                /* Free timer instance memory */
                osapi_memset(&_erps_timer_instance[idx], 0, sizeof(ERPS_TIMER_T));
            }
        }
    }
    _erps_hw_timer_stop();
    /* Check whether there are other timers exist */
    for (idx = 0; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if (TRUE == _erps_timer_instance[idx].in_use)
        {
            if(min_time_remain > (_erps_timer_instance[idx].time_ms - cur))
            {
                min_time_remain = _erps_timer_instance[idx].time_ms - cur;
            }
            is_timer_exist = TRUE;
        }
    }
    if(TRUE == is_timer_exist)
    {
        /* Need restart timer */
        _erps_hw_timer_start(min_time_remain);
        ERPS_LOG_DEBUG("Restart timer with new time:%d", min_time_remain);
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   erps_timer_dump_timer
 * PURPOSE:
 *      This API is used to dump specific ERPS timer info.
 *
 * INPUT:
 *      id                   -- index of timer
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
erps_timer_dump_timer(
    UI8_T   idx)
{
    UI32_T                  cur = 0;
    ERPS_TIMER_T            *ptr_timer = NULL;

    cur = air_util_getSystemTick();
    if(ERPS_MAX_TIMER_NUM <= idx)
    {
        return;
    }
    if(TRUE == _erps_timer_instance[idx].in_use)
    {
        ptr_timer = (ERPS_TIMER_T *)&(_erps_timer_instance[idx]);
        MW_CMD_OUTPUT("\t\t\t[idx%d] time ms: %d, time remian: %dms, callback: 0x%x\r\n",
                        idx, ptr_timer->time_ms, (ptr_timer->time_ms - cur), (UI32_T)ptr_timer->callback);
    }

    return;
}

/* FUNCTION NAME:   erps_timer_dump
 * PURPOSE:
 *      This API is used to dump ERPS module timer info.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
erps_timer_dump(void)
{
    UI8_T                   idx = 0, num = 0;
    UI32_T                  cur = 0;
    ERPS_TIMER_T            *ptr_timer = NULL;

    cur = air_util_getSystemTick();
    MW_CMD_OUTPUT("\nCur system tick: %u\n", cur);
    MW_CMD_OUTPUT("\nERPS Timer:\n");
    MW_CMD_OUTPUT("HW Timer instance list:\n");
    for(idx = 0; idx < ERPS_MAX_TIMER_NUM; idx++)
    {
        if(TRUE == _erps_timer_instance[idx].in_use)
        {
            ptr_timer = (ERPS_TIMER_T *)&(_erps_timer_instance[idx]);
            MW_CMD_OUTPUT("[%d]\ttime ms: %d, in use: %s, time remian: %dms, data: 0x%x, callback: 0x%x\r\n",
                            idx, ptr_timer->time_ms, (ptr_timer->in_use ? "true" : "false"), (ptr_timer->time_ms - cur), (UI32_T)ptr_timer->ptr_data, (UI32_T)ptr_timer->callback);
           num ++;
        }
    }
    MW_CMD_OUTPUT("Total hw timer instance num: %d\r\n", num);
    MW_CMD_OUTPUT("SW Timer:\n");
    MW_CMD_OUTPUT("\trunning: %s\r\n", ((MW_E_OK == osapi_timerActive(_erps_timer_handle)) ? "true" : "false"));

    return;
}

