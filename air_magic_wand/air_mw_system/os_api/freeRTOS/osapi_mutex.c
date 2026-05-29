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

/* FILE NAME:   osapi_mutex.c
 * PURPOSE:
 *      Wrapper APIs for freeRTOS mutex function call.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_mutex.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MUTEX_TIMEOUT_WAIT_INDEFINITELY    (0xFFFFFFFF)
#define MUTEX_TIMEOUT_RETRUN_IMMEDIATELY   (0)

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
/* FUNCTION NAME: osapi_mutexCreate
 * PURPOSE:
 *      Create mutex lock
 *
 * INPUT:
 *      ptr_name        --  Name of module
 *
 * OUTPUT:
 *      pptr_mutex      --  A pointer to pointer of mutex handle
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
osapi_mutexCreate(
    const C8_T *ptr_name,
    semaphorehandle_t *pptr_mutex)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(pptr_mutex);

    *pptr_mutex = xSemaphoreCreateMutex(ptr_name);
    if(NULL == *pptr_mutex)
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_mutexDelete
 * PURPOSE:
 *      Delete mutex lock
 *
 * INPUT:
 *      ptr_mutex       --  A pointer of mutex handle
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
osapi_mutexDelete(
    const semaphorehandle_t ptr_mutex)
{
    MW_CHECK_PTR(ptr_mutex);

    vSemaphoreDelete(ptr_mutex);

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_mutexTake
 * PURPOSE:
 *      Get mutex lock
 *
 * INPUT:
 *      ptr_mutex       --  A pointer of mutex handle
 *      timeout         --  The time to wiat for the mutex to become availabe
 *                          unit: millisecond
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_mutexTake(
    const semaphorehandle_t ptr_mutex,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_mutex);

    if(pdTRUE != xSemaphoreTake(ptr_mutex, (timeout / portTICK_RATE_MS)))
    {
        ret = MW_E_TIMEOUT;
    }

    return ret;
}

/* FUNCTION NAME: osapi_mutexGive
 * PURPOSE:
 *      Give mutex lock
 *
 * INPUT:
 *      ptr_mutex       --  A pointer of mutex handle
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
osapi_mutexGive(
    const semaphorehandle_t ptr_mutex)
{
    MW_CHECK_PTR(ptr_mutex);

    xSemaphoreGive(ptr_mutex);

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_semaphoreCreate
 * PURPOSE:
 *      Create binary semaphore
 *
 * INPUT:
 *      ptr_name        --  Name of module
 *
 * OUTPUT:
 *      pptr_semaphore  --  A pointer to pointer of semaphore handle
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
osapi_semaphoreCreate(
    const C8_T *ptr_name,
    semaphorehandle_t *pptr_semaphore)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_name);
    MW_CHECK_PTR(pptr_semaphore);

    *pptr_semaphore = xSemaphoreCreateBinary(ptr_name);
    if(NULL == *pptr_semaphore)
    {
        ret = MW_E_NO_MEMORY;
    }

    return ret;
}

/* FUNCTION NAME: osapi_semaphoreDelete
 * PURPOSE:
 *      Delete semaphore binary
 *
 * INPUT:
 *      ptr_semaphore   --  A pointer of semaphore handle
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
osapi_semaphoreDelete(
    const semaphorehandle_t ptr_semaphore)
{
    MW_CHECK_PTR(ptr_semaphore);

    vSemaphoreDelete(ptr_semaphore);

    return MW_E_OK;
}

/* FUNCTION NAME: osapi_semaphoreTake
 * PURPOSE:
 *      Get semaphore binary
 *
 * INPUT:
 *      ptr_semaphore   --  A pointer of mutex handle
 *      timeout         --  The time to wiat for the mutex to become availabe
 *                          unit: millisecond
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_semaphoreTake(
    const semaphorehandle_t ptr_semaphore,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_semaphore);

    if(pdTRUE != xSemaphoreTake(ptr_semaphore, (timeout / portTICK_RATE_MS)))
    {
        ret = MW_E_TIMEOUT;
    }

    return ret;
}

/* FUNCTION NAME: osapi_semaphoreGive
 * PURPOSE:
 *      Give semaphore binary
 *
 * INPUT:
 *      ptr_semaphore   --  A pointer of mutex handle
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
osapi_semaphoreGive(
    const semaphorehandle_t ptr_semaphore)
{
    MW_CHECK_PTR(ptr_semaphore);

    xSemaphoreGive(ptr_semaphore);

    return MW_E_OK;
}

