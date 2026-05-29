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
 * PURPOSE:
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <osal/osali.h>
#include <air_types.h>
#include <hal/common/hal.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_OSAL, "osal.c");

/* NAMING CONSTANT DECLARATIONS
 */

#define OSAL_US_PER_SECOND      (1000000)   /* macro second per second      */
#define OSAL_NS_PER_USECOND     (1000)      /* nano second per macro second */
#define OSAL_TIME_YEAR_OFFSET   (1900)

/* MACRO FUNCTION DECLARATIONS
 */
#define OSAL_LOG_ERR(msg, ...) \
            osal_printf("\033[31m<osal:%d>\033[0m"msg, __LINE__, ##__VA_ARGS__)

#define OSAL_PRINT_ERR(msg, ...)                    printk(msg, ##__VA_ARGS__)


#define OSAL_CHECK_MEM_SIZE(__size__)               _osal_chkMemSizeRange(__FUNCTION__, __size__)
#define OSAL_CHECK_MEM_PTR(__ptr_mem__, __len__)    _osal_chkMemPtrRange(__FUNCTION__, __ptr_mem__, __len__)

/* DATA TYPE DECLARATIONS
 */

typedef sem_t               LINUX_SEMA_T;
typedef pthread_t           LINUX_THREAD_T;
typedef pthread_mutex_t     LINUX_MUTEX_T;
typedef pthread_attr_t      LINUX_THREAD_ATTR_T;
typedef struct sched_param  LINUX_SCHED_PARAM_T;
typedef pthread_cond_t      LINUX_COND_T;
typedef pthread_mutexattr_t LINUX_MUTEXATTR_T;
typedef time_t              LINUX_TIME_T;
typedef struct timeval      LINUX_TIMEVAL_T;
typedef struct tm           LINUX_TM_T;

typedef struct OSAL_SEMA_CB_S
{
    LINUX_SEMA_T sema;
    C8_T sema_name[OSAL_SEMA_NAME_LEN + 1];
} OSAL_SEMA_CB_T;

typedef struct OSAL_THREAD_CB_S
{
    struct OSAL_THREAD_CB_S     *ptr_next_thread; /* next thread          */
    AIR_THREAD_ID_T             thread_id;        /* thread id            */
    C8_T                        thread_name[OSAL_THREAD_NAME_LEN + 1];/* thread name */
    UI32_T                      priority;         /* priority, 0~99       */
    UI32_T                      stack_size;       /* stack size           */
    BOOL_T                      is_stop;          /* thread status        */
} OSAL_THREAD_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* Thread */
static OSAL_THREAD_CB_T         *_ptr_osal_thread_list_head = NULL;
static LINUX_MUTEX_T            _osal_thread_cb_lock = PTHREAD_MUTEX_INITIALIZER;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static AIR_ERROR_NO_T
_osal_chainThread(
    OSAL_THREAD_CB_T *ptr_thread_cb)
{
    OSAL_THREAD_CB_T *ptr_cb = NULL;

    pthread_mutex_lock(&_osal_thread_cb_lock);
    if (NULL == _ptr_osal_thread_list_head)
    {
        _ptr_osal_thread_list_head = ptr_thread_cb;
        _ptr_osal_thread_list_head->ptr_next_thread = NULL;
    }
    else
    {
        /* Traverse the whole thread list. */
        for (ptr_cb = _ptr_osal_thread_list_head;
             ptr_cb != NULL;
             ptr_cb = ptr_cb->ptr_next_thread)
        {
            if (NULL == ptr_cb->ptr_next_thread)
            {
                ptr_cb->ptr_next_thread = ptr_thread_cb;
                ptr_thread_cb->ptr_next_thread = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&_osal_thread_cb_lock);

    return (AIR_E_OK);
}

static AIR_ERROR_NO_T
_osal_unchainThread(
    AIR_THREAD_ID_T     *ptr_thread_id)
{
    AIR_ERROR_NO_T      rc = AIR_E_OTHERS;
    OSAL_THREAD_CB_T    *ptr_cb = NULL;
    OSAL_THREAD_CB_T    *ptr_cb_prev = NULL;

    pthread_mutex_lock(&_osal_thread_cb_lock);
    /* Traverse the whole thread list. */
    for (ptr_cb = _ptr_osal_thread_list_head;
         ptr_cb != NULL;
         ptr_cb = ptr_cb->ptr_next_thread)
    {
        if (*ptr_thread_id != (ptr_cb->thread_id))
        {
            ptr_cb_prev = ptr_cb;
        }
        else
        {
            /* remove the current thread cb out of the link list */
            if (ptr_cb == _ptr_osal_thread_list_head)
            {
                _ptr_osal_thread_list_head = ptr_cb->ptr_next_thread;
            }
            else
            {
                ptr_cb_prev->ptr_next_thread = ptr_cb->ptr_next_thread;
            }
            rc = AIR_E_OK;
            break;
        }
    }
    pthread_mutex_unlock(&_osal_thread_cb_lock);

    return (rc);
}

static OSAL_THREAD_CB_T *
_osal_getCBByID(
    const AIR_THREAD_ID_T thread_id)
{
    OSAL_THREAD_CB_T *ptr_tmp = NULL;

    pthread_mutex_lock(&_osal_thread_cb_lock);
    ptr_tmp =_ptr_osal_thread_list_head;
    while (NULL != ptr_tmp)
    {
        if (ptr_tmp->thread_id == thread_id)
        {
            break;
        }
        ptr_tmp = ptr_tmp->ptr_next_thread;
    }
    pthread_mutex_unlock(&_osal_thread_cb_lock);
    return ptr_tmp;
}




static AIR_ERROR_NO_T
_osal_chkMemSizeRange(
    const C8_T                  *ptr_fname,
    const UI32_T                size)
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
    const C8_T                  *ptr_fname,
    const void                  *ptr_mem,
    const UI32_T                len)
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
    const UI32_T    size,
    const C8_T      *module_name)
{
    if (AIR_E_OK == OSAL_CHECK_MEM_SIZE(size))
    {
        return malloc(size);
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
    if(AIR_E_OK == OSAL_CHECK_MEM_PTR((void *)ptr_mem, 0))
    {
        free((void*)ptr_mem);
    }
}

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
    AIR_TIME_T          *ptr_time)
{
    LINUX_TIMEVAL_T     usec_time;
    UI64_T              sec_to_usec;

    HAL_CHECK_PTR(ptr_time);

    gettimeofday(&usec_time, NULL);
    UI64_ASSIGN(sec_to_usec, 0, usec_time.tv_sec);
    UI64_MULT_UI32(sec_to_usec, OSAL_US_PER_SECOND);
    UI64_ADD_UI32(sec_to_usec, usec_time.tv_usec);

    /* give the lower 32-bit usec to user */
    *(AIR_TIME_T *)ptr_time = UI64_LOW(sec_to_usec);

    return (AIR_E_OK);
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
    AIR_TIME_T          cur_time;
    AIR_TIME_T          init_time;
    AIR_ERROR_NO_T      rc = AIR_E_OK;

    if (0 != usecond)
    {
        osal_getTime(&init_time);
        while (1)
        {
            osal_getTime(&cur_time);

            if (cur_time == init_time)
                continue;

            if (cur_time > init_time)
            {
                /* normal case */
                if ((cur_time - init_time) >= usecond)
                {
                    /* wait timeout */
                    break;
                }
            }
            else
            {
                /* wrap case */
                if (((0xFFFFFFFF - init_time) + cur_time) >= usecond)
                {
                    /* wait timeout */
                    break;
                }
            }
        }
    }

    return (rc);
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
    const C8_T          *ptr_sema_name,
    const UI32_T        sema_count,
    AIR_SEMAPHORE_ID_T  *ptr_semaphore_id,
    const C8_T          *module_name)
{
    OSAL_SEMA_CB_T      *ptr_sema_cb = NULL;
    C8_T                tmp_sema_name[OSAL_SEMA_NAME_LEN + 1] = { 0 };

    HAL_CHECK_MIN_MAX_RANGE(sema_count, 0, OSAL_SEMA_MAX_CNT);
    HAL_CHECK_PTR(ptr_semaphore_id);

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
    sem_init(&ptr_sema_cb->sema, 0, sema_count);
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
    AIR_SEMAPHORE_ID_T  *ptr_semaphore_id)
{
    OSAL_SEMA_CB_T      *ptr_sema_cb = NULL;

    HAL_CHECK_PTR(ptr_semaphore_id);
    ptr_sema_cb = (OSAL_SEMA_CB_T *)*ptr_semaphore_id;
    HAL_CHECK_PTR(ptr_sema_cb);

    sem_destroy(&ptr_sema_cb->sema);
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
    AIR_SEMAPHORE_ID_T  *ptr_semaphore_id,
    UI32_T              time_out)
{
    OSAL_SEMA_CB_T      *ptr_sema_cb = NULL;
    LINUX_TIMEVAL_T     time;
    struct timespec     max_wait;

    HAL_CHECK_PTR(ptr_semaphore_id);
    ptr_sema_cb = (OSAL_SEMA_CB_T *)(* ptr_semaphore_id);
    HAL_CHECK_PTR(ptr_sema_cb);

    /* Wait Semaphore. */
    if (0xFFFFFFFF == time_out)
    {
        if (0 != sem_wait(&ptr_sema_cb->sema))
        {
            return (AIR_E_OTHERS);
        }
    }
    else
    {
        /* Get timeout. */
        gettimeofday(&time, NULL);
        time.tv_sec  += (time_out / OSAL_US_PER_SECOND);
        time.tv_usec += (time_out % OSAL_US_PER_SECOND);
        if (time.tv_usec >= OSAL_US_PER_SECOND)
        {
            time.tv_usec -= OSAL_US_PER_SECOND;
            time.tv_sec++;
        }
        max_wait.tv_sec  = time.tv_sec;
        max_wait.tv_nsec = time.tv_usec * OSAL_NS_PER_USECOND;

        if (0 != sem_timedwait(&ptr_sema_cb->sema, &max_wait))
        {
            return (AIR_E_OTHERS);
        }
    }

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
    AIR_SEMAPHORE_ID_T  *ptr_semaphore_id)
{
    OSAL_SEMA_CB_T      *ptr_sema_cb = NULL;

    HAL_CHECK_PTR(ptr_semaphore_id);
    ptr_sema_cb = (OSAL_SEMA_CB_T *)(*ptr_semaphore_id);
    HAL_CHECK_PTR(ptr_sema_cb);

    sem_post(&ptr_sema_cb->sema);

    return (AIR_E_OK);
}

/* FUNCTION NAME:  osal_sleepTask
 * PURPOSE:
 *      OS abstration API to delay the current thread for microseconds, taks will suspended.
 * INPUT:
 *      usecond       -- microseconds to delay
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
    usleep(msecond * 1000);
    return AIR_E_OK;
}
/* FUNCTION NAME:  osal_initRunThread
 * PURPOSE:
 *      OS abstration API to init the running thread's attribute
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
osal_initRunThread(
    void)
{
    AIR_THREAD_ID_T thread_id = (AIR_THREAD_ID_T)pthread_self();
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); /* DO NOT use PTHREAD_CANCEL_ASYNCHRONOUS */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (NULL == _osal_getCBByID(thread_id))
    {
        usleep(1);
    }
}

/* FUNCTION NAME:  osal_createThread
 * PURPOSE:
 *      OS abstration API to create thread.
 * INPUT:
 *      ptr_thread_name -- Point to the string for name of thread
 *      stack_size      -- size of stack
 *      priority        -- thread priority (Highest : 99, Lowest : 1)
 *      function        -- function point of thread
 *      ptr_arg         -- Point to agrument for callback function
 * OUTPUT:
 *      ptr_thread_id   -- pointer to thread ID
 * RETURN:
 *      AIR_E_OK        -- Successfully create the thread
 *      AIR_E_OTHERS    -- Fail to create the thread.
 * NOTES:
 *      The proper way to invoke osal_createThread is
 *      1. Caller define a AIR_THREAD_ID_T thread_id,
 *      2. Invoke with thread_id's address, i.e. osal_createThread(&thread_id).
 */
AIR_ERROR_NO_T
osal_createThread (
    const C8_T          *ptr_thread_name,
    const UI32_T        stack_size,
    const UI32_T        priority,
    void                (function)(void*),
    void                *ptr_arg,
    AIR_THREAD_ID_T     *ptr_thread_id)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    C8_T                tmp_thread_name[OSAL_THREAD_NAME_LEN + 1] = { 0 };
    OSAL_THREAD_CB_T    *ptr_thread_cb = NULL;
    LINUX_THREAD_T      thread_id;
    LINUX_THREAD_ATTR_T thread_attr;
    LINUX_SCHED_PARAM_T thread_param;
    UI32_T              thread_stack_size = 0;
    UI32_T              thread_priority = 0;

    HAL_CHECK_PTR(function);
    HAL_CHECK_PTR(ptr_thread_id);

    /* Process the thread name. */
    if ((NULL != ptr_thread_name) && (0 != osal_strlen(ptr_thread_name)))
    {
        osal_strncpy(tmp_thread_name, ptr_thread_name, OSAL_THREAD_NAME_LEN);
        tmp_thread_name[OSAL_THREAD_NAME_LEN] = '\0';
    }

    /* Initialize the attribute structure. */
    if (pthread_attr_init(&thread_attr))
    {
        rc = (AIR_E_OTHERS);
    }
    else
    {
        /* Set stack size. */
        thread_stack_size = stack_size;
        pthread_attr_setstacksize(&thread_attr, thread_stack_size);

        /* Set the schedule policy and real-time priority. */
        pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
        thread_priority = (priority <= OSAL_THREAD_HIGHEST_PRIORITY)?
                           priority : OSAL_THREAD_HIGHEST_PRIORITY;
        thread_param.sched_priority = thread_priority;
        pthread_attr_setschedparam(&thread_attr, &thread_param);

        /* If you want to use the current scheduling policy, try this:
         * (notice: set PTHREAD_EXPLICIT_SCHED need root permission)
         * pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
         */

        /* If you want to set the thread detach state (i.e. without pthread_join()), try this:
         * pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
         */

        /* Memory allocate for the thread control block. */
        ptr_thread_cb = (OSAL_THREAD_CB_T *)osal_alloc(sizeof(OSAL_THREAD_CB_T), "osal");
        if (NULL == ptr_thread_cb)
        {
            OSAL_LOG_ERR("osal_createThread alloc fail\n");
            return (AIR_E_NO_MEMORY);
        }

        /* Create pthread through Linux API and check its result. */
        if (pthread_create(&thread_id, &thread_attr, (void *(*)(void *))function, (void *)ptr_arg))
        {
            osal_free(ptr_thread_cb);
            OSAL_LOG_ERR("osal_createThread create fail\n");
            return (AIR_E_OTHERS);
        }

        /* Fill up the control block. */
        *ptr_thread_id = (AIR_THREAD_ID_T)thread_id;
        ptr_thread_cb->thread_id = (AIR_THREAD_ID_T)thread_id;
        osal_strncpy(ptr_thread_cb->thread_name, tmp_thread_name, OSAL_THREAD_NAME_LEN);
        ptr_thread_cb->thread_name[OSAL_THREAD_NAME_LEN] = '\0';
        ptr_thread_cb->priority = priority;
        ptr_thread_cb->stack_size = stack_size;
        ptr_thread_cb->is_stop = FALSE;

        /* Chain the control block. */
        _osal_chainThread(ptr_thread_cb);
    }
    return (rc);
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
    AIR_THREAD_ID_T  *ptr_thread_id)
{
    OSAL_THREAD_CB_T    *ptr_thread_cb = NULL;

    HAL_CHECK_PTR(ptr_thread_id);

    ptr_thread_cb = _osal_getCBByID(*ptr_thread_id);
    if (NULL == ptr_thread_cb)
    {
        return (AIR_E_ENTRY_NOT_FOUND);
    }
    ptr_thread_cb->is_stop = TRUE;

    return (AIR_E_OK);
}

/* FUNCTION NAME:  osal_destroyThread
 * PURPOSE:
 *      OS abstration API to destroy thread.
 * INPUT:
 *      ptr_thread_id   -- thread ID.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK        -- Successfully destroy the thread
 * NOTES:
 *      Similar with osal_createThread, when invoke osal_destroyThread(),
 *      the caller should pass the thread_id's address.
 */
AIR_ERROR_NO_T
osal_destroyThread(
    AIR_THREAD_ID_T     *ptr_thread_id)
{
    OSAL_THREAD_CB_T    *ptr_thread_cb = NULL;

    HAL_CHECK_PTR(ptr_thread_id);

    ptr_thread_cb = _osal_getCBByID(*ptr_thread_id);
    if (NULL == ptr_thread_cb)
    {
        return (AIR_E_ENTRY_NOT_FOUND);
    }

    /* wait the thread exit */
    if (pthread_join((pthread_t)(*ptr_thread_id), NULL))
    {
        OSAL_LOG_ERR("osal_destroyThread %s fail\n", ptr_thread_cb->thread_name);
        return (AIR_E_OTHERS);
    }
    _osal_unchainThread(ptr_thread_id);
    osal_free(ptr_thread_cb);
    *ptr_thread_id = 0;

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
    pthread_exit(NULL);
}

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
    void)
{
    OSAL_THREAD_CB_T    *ptr_thread_cb = NULL;

    ptr_thread_cb = _osal_getCBByID((AIR_THREAD_ID_T)pthread_self());
    if (NULL == ptr_thread_cb)
    {
        return (AIR_E_ENTRY_NOT_FOUND);
    }

    if (TRUE == ptr_thread_cb->is_stop)
    {
        return (AIR_E_OTHERS);
    }
    else
    {
        return (AIR_E_OK);
    }
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
    const UI32_T        usecond)
{
    LINUX_TIMEVAL_T     time_out;
    UI32_T              time_out_usec;
    AIR_ERROR_NO_T      rc = AIR_E_OK;

    if (0 != usecond)
    {
        time_out_usec = usecond;
        time_out.tv_sec = (time_t) (time_out_usec / OSAL_US_PER_SECOND);
        time_out.tv_usec = (suseconds_t) (time_out_usec % OSAL_US_PER_SECOND);
        if (0 != select(0, NULL, NULL, NULL, &time_out))
        {
            rc = AIR_E_OTHERS;
        }
    }

    return (rc);
}

