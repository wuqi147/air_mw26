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

/* FILE NAME:  osapi_thread.h
 * PURPOSE:
 *      Wrapper APIs for freeRTOS process and thread function call.
 *
 * NOTES:
 */

#ifndef OSAPI_THREAD_H
#define OSAPI_THREAD_H

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: osapi_threadCreate
 * PURPOSE:
 *      Create task
 *
 * INPUT:
 *      ptr_name        --  A descriptive name for the task
 *      stack_size      --  Size of stack
 *      priority        --  Priority of task
 *      func            --  Pointer to the callback routine
 *      ptr_param       --  A value that is passed as the parameter
 *                          to the callback routine
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer used to pass a handle
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
osapi_threadCreate(
    const C8_T *ptr_name,
    const UI32_T stack_size,
    const UI32_T priority,
    const threadfunc_t func,
    const void *ptr_param,
    threadhandle_t *pptr_handle);

/* FUNCTION NAME: osapi_threadCreateStatic
 * PURPOSE:
 *      Create a task with static variables.
 *
 * INPUT:
 *      ptr_name        --  A descriptive name for the task
 *      stack_size      --  Size of stack
 *      priority        --  Priority of task
 *      func            --  Pointer to the callback routine
 *      ptr_param       --  A value that is passed as the parameter
 *                          to the callback routine
 *      ptr_stackBuffer --  Pointer to stack buffer
 *      ptr_taskBuffer  --  Pointer to task buffer
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer used to pass a handle
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
osapi_threadCreateStatic(
    const C8_T *ptr_name,
    const UI32_T stack_size,
    const UI32_T priority,
    const threadfunc_t func,
    const void *ptr_param,
    StackType_t  *ptr_stackBuffer,
    StaticTask_t *ptr_taskBuffer,
    threadhandle_t *pptr_handle);

/* FUNCTION NAME: osapi_threadDelete
 * PURPOSE:
 *      Delete task
 *
 * INPUT:
 *      ptr_handle      -- A pointer of the task handle
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
osapi_threadDelete(
    const threadhandle_t ptr_handle);

/* FUNCTION NAME: osapi_processCreate
 * PURPOSE:
 *      Create main task
 *
 * INPUT:
 *      ptr_name        --  A descriptive name for the task
 *      stack_size      --  Size of stack
 *      priority        --  Priority of task
 *      func            --  Pointer to the callback routine
 *      ptr_param       --  A value that is passed as the parameter
 *                          to the callback routine
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer used to pass a handle
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
osapi_processCreate(
    const C8_T *ptr_name,
    const UI32_T stack_size,
    const UI32_T priority,
    const threadfunc_t func,
    const void *param,
    threadhandle_t *pptr_handle);

/* FUNCTION NAME: osapi_processDelete
 * PURPOSE:
 *      Delete main task
 *
 * INPUT:
 *      ptr_handle      -- A pointer of the task handle
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
osapi_processDelete(
    const threadhandle_t ptr_handle);

/* FUNCTION NAME: osapi_delay
 * PURPOSE:
 *      Task delay time
 *
 * INPUT:
 *      ms_delay        -- Delay unit of millisecond
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
osapi_delay(
    const UI16_T ms_delay);

/* FUNCTION NAME: osapi_threadPrioritySet
 * PURPOSE:
 *      Set task priority
 *
 * INPUT:
 *      ptr_handle      -- A pointer of the task handle
 *      priority        -- Task priority
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
osapi_threadPrioritySet(
    const threadhandle_t ptr_handle,
    const UI32_T priority);

/* FUNCTION NAME: osapi_threadPriorityGet
 * PURPOSE:
 *      Get task priority
 *
 * INPUT:
 *      ptr_handle      -- A pointer to the task handle
 *
 * OUTPUT:
 *      ptr_priority    -- Task priority
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_threadPriorityGet(
    const threadhandle_t ptr_handle,
    UI32_T *const ptr_priority);

/* FUNCTION NAME: osapi_processPrioritySet
 * PURPOSE:
 *      Set task priority
 *
 * INPUT:
 *      ptr_handle      -- A pointer to the task handle
 *      priority        -- Task priority
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_processPrioritySet(
    const threadhandle_t ptr_handle,
    const UI32_T priority);

/* FUNCTION NAME: osapi_processPriorityGet
 * PURPOSE:
 *      Get task priority
 *
 * INPUT:
 *      ptr_handle      -- A pointer to the task handle
 *
 * OUTPUT:
 *      ptr_priority    -- Task priority
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_processPriorityGet(
    const threadhandle_t ptr_handle,
    UI32_T *const ptr_priority);

/* FUNCTION NAME: osapi_sysTickGet
 * PURPOSE:
 *      Get the current system tick count
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      Tick count value
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
UI32_T
osapi_sysTickGet();

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/

#endif  /* #ifndef OSAPI_THREAD_H */

