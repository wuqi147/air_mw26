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

/* FILE NAME:  osal.h
 * PURPOSE:
 *  osal.h provide an OS abstration layer's API for different OS. The APIs
 *  include task/thread, semaphore, time, memory, string and C library.
 * NOTES:
 *
 */

#ifndef OSAL_H
#define OSAL_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_types.h>
#include <osal/osal_lib.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define OSAL_THREAD_HIGHEST_PRIORITY (99)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct OSAL_TM_S
{
    UI32_T year;  /* year,   20XX */
    UI32_T month; /* month,  1~12 */
    UI32_T day;   /* day,    1~31 */
    UI32_T hour;  /* hour,   0~23 */
    UI32_T min;   /* minute, 0~59 */
    UI32_T sec;   /* second, 0~59 */
} OSAL_TM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  osal_init
 * PURPOSE:
 *      OS abstration API to initialize osal module.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_init(
    void);

/* FUNCTION NAME:  osal_deinit
 * PURPOSE:
 *      Deinitialize the OSAL module
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 */
AIR_ERROR_NO_T
osal_deinit(
    void);

/* FUNCTION NAME:  osal_alloc
 * PURPOSE:
 *      OS abstration API to allocate memory.
 * INPUT:
 *      size        -- size of memory to be allocate
 *      module_name -- point of the module name.
 * OUTPUT:
 *      None
 * RETURN:
 *      Point to memory
 * NOTES:
 *      None
 */
void *
osal_alloc(
    const UI32_T size,
    const C8_T  *module_name);

/* FUNCTION NAME:  osal_free
 * PURPOSE:
 *      OS abstration API to free allocated memory.
 * INPUT:
 *      ptr_mem     -- point of the memory to be freed.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
osal_free(
    const void *ptr_mem);

/* FUNCTION NAME:  osal_delayUs
 * PURPOSE:
 *      OS abstration API to delay the current thread for microseconds.
 * INPUT:
 *      usecond       -- microseconds to delay
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully delay the thread
 * NOTES:
 *      Only support ms in EN8851 freeRTOS version
 */
AIR_ERROR_NO_T
osal_delayUs(
    const UI32_T usecond);

/* FUNCTION NAME:  osal_getTime
 * PURPOSE:
 *      OS abstration API to get current time since Unix Epoch.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_time     -- time in micro-seconds
 * RETURN:
 *      AIR_E_OK     -- Successfully get time.
 *      AIR_E_OTHERS -- Fail to get time.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_getTime(
    AIR_TIME_T *ptr_time);

/* FUNCTION NAME:  osal_createSemaphore
 * PURPOSE:
 *      OS abstration API to create semaphore.
 * INPUT:
 *      *ptr_sema_name    -- pointer to the string of semaphore name
 *      sema_count        -- the init value of semaphore
 *                           AIR_SEMAPHORE_BINARY: this means the semaphore is as
 *                             mutex for protecting critical section
 *                           AIR_SEMAPHORE_SYNC: this means the semaphore is as
 *                             signal for syncing.
 *      module_name       -- point of the module name.
 * OUTPUT:
 *      *ptr_semaphore_id -- Pointer to semaphore ID
 * RETURN:
 *      AIR_E_OK      -- Successfully create the semaphore.
 *      AIR_E_OTHERS  -- Fail to create the semaphore.
 * NOTES:
 *      The proper way to invoke osal_createSemaphore is
 *      1. Caller define a AIR_SEMAPHORE_ID_T id,
 *      2. Invoke with id's address, i.e. osal_createSemaphore(&id).
 */
AIR_ERROR_NO_T
osal_createSemaphore(
    const C8_T         *ptr_sema_name,
    const UI32_T        sema_count,
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id,
    const C8_T         *module_name);

/* FUNCTION NAME:  osal_destroySemaphore
 * PURPOSE:
 *      OS abstration API to destroy semaphore.
 * INPUT:
 *      ptr_semaphore_id    -- Pointer to semaphore ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Successfully destory the semaphore.
 *      AIR_E_OTHERS        -- Fail to destory the semaphore.
 * NOTES:
 *      Similar with osal_createSemaphore, when invoke osal_destroySemaphore(),
 *      the caller should pass the semaphore_id's address.
 */
AIR_ERROR_NO_T
osal_destroySemaphore(
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id);

/* FUNCTION NAME:  osal_takeSemaphore
 * PURPOSE:
 *      OS abstration API to take semaphore.
 * INPUT:
 *      ptr_semaphore_id    -- Pointer to semaphore ID
 *      time_out            -- Time out before waiting semaphore in usec.
 *                             Wait forever. (0xFFFFFFFF)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Successfully take the semaphore.
 *      AIR_E_OTHERS        -- Timeout.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_takeSemaphore(
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id,
    UI32_T              time_out);

/* FUNCTION NAME:  osal_giveSemaphore
 * PURPOSE:
 *      OS abstration API to give semaphore.
 * INPUT:
 *      ptr_semaphore_id    -- Pointer to semaphore ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Successfully give the semaphore.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_giveSemaphore(
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id);

/* FUNCTION NAME:  osal_sleepTask
 * PURPOSE:
 *      OS abstration API to delay the current thread for microseconds, taks will suspended.
 * INPUT:
 *      msecond       -- milliseconds to delay
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully delay the thread.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_sleepTask(
    const UI32_T msecond);

/* FUNCTION NAME:  osal_createThread
 * PURPOSE:
 *      OS abstration API to create thread.
 * INPUT:
 *      ptr_thread_name -- pointer to the string for name of thread
 *      stack_size      -- size of stack
 *      priority        -- thread priority (Highest : 99, Lowest : 1)
 *      function        -- function pointer of thread
 *      ptr_arg         -- pointer to agrument for callback function
 * OUTPUT:
 *      ptr_thread_id   -- pointer to thread ID
 * RETURN:
 *      AIR_E_OK        -- Successfully create the thread.
 *      AIR_E_OTHERS    -- Fail to create the thread.
 * NOTES:
 *      The proper way to invoke osal_createThread is
 *      1. Caller define a AIR_THREAD_ID_T thread_id,
 *      2. Invoke with thread_id's address, i.e. osal_createThread(&thread_id).
 */
AIR_ERROR_NO_T
osal_createThread(
    const C8_T  *ptr_thread_name,
    const UI32_T stack_size,
    const UI32_T priority,
    void(function)(void *),
    void            *ptr_arg,
    AIR_THREAD_ID_T *ptr_thread_id);

/* FUNCTION NAME:  osal_destroyThread
 * PURPOSE:
 *      OS abstration API to destroy thread.
 * INPUT:
 *      ptr_thread_id   -- thread ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK        -- Successfully destroy the thread.
 * NOTES:
 *      Similar with osal_createThread, when invoke osal_destroyThread(),
 *      the caller should pass the thread_id's address.
 */
AIR_ERROR_NO_T
osal_destroyThread(
    AIR_THREAD_ID_T *ptr_thread_id);

/* FUNCTION NAME:  osal_stopThread
 * PURPOSE:
 *      OS abstration API to stop thread.
 * INPUT:
 *      ptr_thread_id   -- thread ID.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK        -- Successfully destroy the thread
 * NOTES:
 *      Similar with osal_createThread, when invoke osal_stopThread(),
 *      the caller should pass the thread_id's address.
 */
AIR_ERROR_NO_T
osal_stopThread(
    AIR_THREAD_ID_T *ptr_thread_id);

/* FUNCTION NAME:  osal_exitRunThread
 * PURPOSE:
 *      OS abstration API to release the OS resource
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
osal_exitRunThread(
    void);

/* FUNCTION NAME:  osal_isRunThread
 * PURPOSE:
 *      OS abstration API to check if the thread is in run state.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OTHERS -- the thread is not in run state
 *      AIR_E_OK     -- the thread is in run state
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_isRunThread(
    void);

/* FUNCTION NAME:  osal_sleepThread
 * PURPOSE:
 *      OS abstration API to suspend the current thread for microseconds.
 * INPUT:
 *      usecond       -- microseconds to suspend
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully suspend the thread
 *      AIR_E_OTHERS  -- Fail to suspend
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_sleepThread(
    const UI32_T usecond);

#endif /* End of OSAL_H */
