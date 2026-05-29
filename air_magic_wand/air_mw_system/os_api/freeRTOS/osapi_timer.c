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

/* FILE NAME:   osapi_timer.c
 * PURPOSE:
 *      Wrapper APIs for freeRTOS timer function call.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_timer.h"

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

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: osapi_timerCreate
 * PURPOSE:
 *      Create timer
 *
 * INPUT:
 *      ptr_name        --  Name of timer module
 *      func            --  Timer callback function
 *      reload          --  TRUE: auto reload
 *                          FALSE: one-shot
 *      period          --  The period of the timer
 *                          unit: millisecond
 *      timerid         --  An identifier that is assigned to the timer
 *
 * OUTPUT:
 *      pptr_handle     --  A pointer to pointer of timer handle
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
osapi_timerCreate(
    const C8_T *ptr_name,
    const timeCbk_t func,
    const BOOL_T reload,
    const UI32_T period,
    const void *timerid,
    timehandle_t *pptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(pptr_handle);

    *pptr_handle = xTimerCreate(ptr_name, (period / portTICK_RATE_MS), reload, (void*)timerid, func);
    if(NULL == *pptr_handle)
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_timerStart
 * PURPOSE:
 *      Start timer of specific timer handle
 *
 * INPUT:
 *      ptr_handle      --  A pointer of timer handle
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
osapi_timerStart(
    const timehandle_t ptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_handle);

    if(pdFAIL == xTimerStart(ptr_handle, 0))
    {
        ret = MW_E_OTHERS;
    }

    return ret;
}

/* FUNCTION NAME: osapi_timerStop
 * PURPOSE:
 *      Stop timer of specific timer handle
 *
 * INPUT:
 *      ptr_handle      --  A pointer of timer handle
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
osapi_timerStop(
    const timehandle_t ptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_handle);

    if(pdFAIL == xTimerStop(ptr_handle, 0))
    {
        ret = MW_E_OTHERS;
    }

    return ret;
}

/* FUNCTION NAME: osapi_timerDelete
 * PURPOSE:
 *      Delete timer of specific timer handle
 *
 * INPUT:
 *      ptr_handle      --  A pointer of timer handle
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
osapi_timerDelete(
    const timehandle_t ptr_handle)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_handle);

    if(pdFAIL == xTimerDelete(ptr_handle, 0))
    {
        ret = MW_E_OTHERS;
    }

    return ret;
}

/* FUNCTION NAME: osapi_timerIdGet
 * PURPOSE:
 *      Get an identifier is assigned to a timer
 *
 * INPUT:
 *      ptr_handle      --  A pointer of timer handle
 *
 * OUTPUT:
 *      ptr_id          --  An identifier is assigned to a timer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_timerIdGet(
    const timehandle_t ptr_handle,
    UI32_T *const ptr_id)
{
    MW_CHECK_PTR(ptr_handle);
    MW_CHECK_PTR(ptr_id);

    *ptr_id = (UI32_T)pvTimerGetTimerID(ptr_handle);

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_timerActive
 * PURPOSE:
 *      Check timer is in atcive or not
 *
 * INPUT:
 *      ptr_handle      --  A pointer of timer handle
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK        -- In active
 *      !MW_E_OK       -- Not in active
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_timerActive(
    const timehandle_t ptr_handle)
{
    MW_CHECK_PTR(ptr_handle);

    if(pdFALSE != xTimerIsTimerActive(ptr_handle))
    {
        return MW_E_OK;
    }

    return MW_E_OTHERS;
}
