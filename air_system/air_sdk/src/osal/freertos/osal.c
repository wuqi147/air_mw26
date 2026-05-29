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

/* FILE NAME:  osal.c
 * AUTHOR: ChiaHung Lee, Xianfeng Pan
 * PURPOSE:
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <osal/osal.h>

#include <air_types.h>
#include <osal/osali.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define OSAL_US_PER_SECOND    (1000000) /* macro second per second      */
#define OSAL_NS_PER_USECOND   (1000)    /* nano second per macro second */
#define OSAL_TIME_YEAR_OFFSET (1900)

/* MACRO FUNCTION DECLARATIONS
 */
#define OSAL_LOG_ERR(msg, ...) osal_printf("\033[31m<osal:%d>\033[0m" msg, __LINE__, ##__VA_ARGS__)

#define OSAL_LOG_WARNING(msg, ...)                \
    if (_osal_log_flag & OSAL_WARNING_LOG_ENABLE) \
    osal_printf(msg, ##__VA_ARGS__)

#define OSAL_LOG_DBG(msg, ...)                  \
    if (_osal_log_flag & OSAL_DEBUG_LOG_ENABLE) \
    osal_printf(msg, ##__VA_ARGS__)

#define OSAL_PRINT_ERR(msg, ...) printk(msg, ##__VA_ARGS__)

#define OSAL_CHECK_MEM_SIZE(__size__)            _osal_chkMemSizeRange(__FUNCTION__, __size__)
#define OSAL_CHECK_MEM_PTR(__ptr_mem__, __len__) _osal_chkMemPtrRange(__FUNCTION__, __ptr_mem__, __len__)

/* DATA TYPE DECLARATIONS
 */
typedef QueueHandle_t SemaphoreHandle_t;
typedef struct OSAL_SEMA_CB_S
{
    SemaphoreHandle_t sema; /* kernel semaphore */
    C8_T              sema_name[OSAL_SEMA_NAME_LEN + 1];
} OSAL_SEMA_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
unsigned long long
vGetSystem1msTick(void);

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_osal_chkMemSizeRange(
    const C8_T  *ptr_fname,
    const UI32_T size)
{
#if defined OSAL_EN_MEM_CHK
    if (0 == size)
    {
        OSAL_LOG_ERR("%s: size is 0.\n", ptr_fname);
        return (AIR_E_OTHERS);
    }
#endif
    return (AIR_E_OK);
}

static AIR_ERROR_NO_T
_osal_chkMemPtrRange(
    const C8_T  *ptr_fname,
    const void  *ptr_mem,
    const UI32_T len)
{
#if defined OSAL_EN_MEM_CHK
    if (NULL == ptr_mem)
    {
        OSAL_LOG_ERR("%s: NULL pointer.\n", ptr_fname);
        return (AIR_E_OTHERS);
    }
#endif
    return (AIR_E_OK);
}

/* EXPORTED SUBPROGRAM BODIES
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
    void)
{
    return (AIR_E_OK);
}

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
    void)
{
    return (AIR_E_OK);
}

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
    const C8_T  *module_name)
{
    if (AIR_E_OK == OSAL_CHECK_MEM_SIZE(size))
    {
        return pvPortMalloc(size, (C8_T *)module_name);
    }

    return NULL;
}

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
    const void *ptr_mem)
{
    if (AIR_E_OK == OSAL_CHECK_MEM_PTR((void *)ptr_mem, 0))
    {
        vPortFree((void *)ptr_mem);
    }
}

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
    const UI32_T usecond)
{
    delay1ms(usecond / 1000);
    return AIR_E_OK;
}

/* FUNCTION NAME:  osal_getTime
 * PURPOSE:
 *      OS abstration API to get current time.
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
    AIR_TIME_T *ptr_time)
{
    *ptr_time = vGetSystem1msTick();
    return (AIR_E_OK);
}

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
    const C8_T         *module_name)
{
    OSAL_SEMA_CB_T *ptr_sema_cb = NULL;
    C8_T            tmp_sema_name[OSAL_SEMA_NAME_LEN + 1] = OSAL_SEMA_DFT_NAME;

    if (OSAL_SEMA_MAX_CNT < sema_count)
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (NULL == ptr_semaphore_id)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Process the semaphore name. */
    if ((NULL != ptr_sema_name) && (0 != osal_strlen(ptr_sema_name)))
    {
        osal_strncpy(tmp_sema_name, ptr_sema_name, OSAL_SEMA_NAME_LEN);
        tmp_sema_name[OSAL_SEMA_NAME_LEN] = '\0';
    }

    /* Memory allocate for the semaphore control block. */
    ptr_sema_cb = (OSAL_SEMA_CB_T *)osal_alloc(sizeof(OSAL_SEMA_CB_T), module_name);
    if (NULL == ptr_sema_cb)
    {
        OSAL_LOG_ERR("osal_createSemaphore: alloc fail\n");
        return (AIR_E_NO_MEMORY);
    }

    /* Init the semaphore. */
    osal_memset(ptr_sema_cb, 0, sizeof(*ptr_sema_cb));
    /* No matter AIR_SEMAPHORE_BINARY or AIR_SEMAPHORE_SYNC semaphore type, it can implement by xSemaphoreCreateBinary()
     */
    vSemaphoreCreateBinary(ptr_sema_cb->sema, module_name); /* use freertos sema create */
    osal_strncpy(ptr_sema_cb->sema_name, tmp_sema_name, OSAL_SEMA_NAME_LEN);
    ptr_sema_cb->sema_name[OSAL_SEMA_NAME_LEN] = '\0';
    *ptr_semaphore_id = (AIR_SEMAPHORE_ID_T)ptr_sema_cb;

    return (AIR_E_OK);
}

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
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id)
{
    OSAL_SEMA_CB_T *ptr_sema_cb = NULL;

    if (NULL == ptr_semaphore_id)
    {
        return AIR_E_BAD_PARAMETER;
    }

    ptr_sema_cb = (OSAL_SEMA_CB_T *)*ptr_semaphore_id;
    if (NULL == ptr_sema_cb)
    {
        return AIR_E_BAD_PARAMETER;
    }

    vSemaphoreDelete(ptr_sema_cb->sema); /* use freertos sema create */
    osal_free(ptr_sema_cb);
    *ptr_semaphore_id = 0;

    return (AIR_E_OK);
}

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
    UI32_T              time_out)
{
    OSAL_SEMA_CB_T *ptr_sema_cb = NULL;
    UI32_T          xTicksToWait;

    if (NULL == ptr_semaphore_id)
    {
        return AIR_E_BAD_PARAMETER;
    }

    ptr_sema_cb = (OSAL_SEMA_CB_T *)(*ptr_semaphore_id);
    if (NULL == ptr_sema_cb)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /*  ticks = millisec / portTICK_PERIOD_MS  */
    xTicksToWait = (time_out / 1000) / portTICK_PERIOD_MS;

    xSemaphoreTake(ptr_sema_cb->sema, xTicksToWait); /* use freertos sema take */
    return (AIR_E_OK);
}

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
    AIR_SEMAPHORE_ID_T *ptr_semaphore_id)
{
    OSAL_SEMA_CB_T *ptr_sema_cb = NULL;

    if (NULL == ptr_semaphore_id)
    {
        return AIR_E_BAD_PARAMETER;
    }

    ptr_sema_cb = (OSAL_SEMA_CB_T *)(*ptr_semaphore_id);
    if (NULL == ptr_sema_cb)
    {
        return AIR_E_BAD_PARAMETER;
    }
    xSemaphoreGive(ptr_sema_cb->sema); /* use freertos sema create */
    return (AIR_E_OK);
}

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
    const UI32_T msecond)
{
    vTaskDelay(pdMS_TO_TICKS(msecond));
    return AIR_E_OK;
}

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
    AIR_THREAD_ID_T *ptr_thread_id)
{
    if (pdPASS !=
        xTaskCreate(
            function,        /* The function that implements the task. */
            ptr_thread_name, /* The text name assigned to the task - for debug only as it is not used by the kernel. */
            stack_size,      /* The size of the stack to allocate to the task. */
            ptr_arg,         /* The parameter passed to the task - not used in this simple case. */
            priority,        /* The priority assigned to the task. */
            (TaskHandle_t *)ptr_thread_id)) /* The task handle is not required, so NULL is passed. */
    {
        return AIR_E_OTHERS;
    }
    else
    {
        return AIR_E_OK;
    }
}

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
    AIR_THREAD_ID_T *ptr_thread_id)
{
    return (AIR_E_OK);
}

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
    void)
{
    return;
}

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
    AIR_THREAD_ID_T *ptr_thread_id)
{
    vTaskDelete((TaskHandle_t)(*ptr_thread_id));
    return AIR_E_OK;
}

/* FUNCTION NAME:  osal_isRunThread
 * PURPOSE:
 *      OS abstration API to check if the thread is in run state.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- the thread is in run state
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_isRunThread(
    void)
{
    return AIR_E_OK;
}

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
    const UI32_T usecond)
{
    return AIR_E_OK;
}
