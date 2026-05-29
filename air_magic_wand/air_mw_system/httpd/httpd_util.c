/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  httpd_util.c
 * PURPOSE:
 *  Implement functions for httpd.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "httpd_util.h"
#include "osapi.h"
#include "osapi_mutex.h"

/* NAMING CONSTANT DECLARATIONS
*/
#ifdef AIR_SUPPORT_HTTPD_MUTEX
#define MW_HTTPD_MUTEX_NAME "HTTPD_MUTEX"
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef AIR_SUPPORT_HTTPD_MUTEX
static semaphorehandle_t _mw_httpd_mutex = NULL;   /* The mutext to protect HTTPD CGI Handlers for httpd/mqttd/cli. */
#endif

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_HTTPD_MUTEX
MW_ERROR_NO_T
mw_httpd_mutex_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if (NULL != _mw_httpd_mutex)
    {
        return MW_E_ALREADY_INITED;
    }
    ret = osapi_mutexCreate(MW_HTTPD_MUTEX_NAME, &_mw_httpd_mutex);
    if (MW_E_OK != ret)
    {
        _mw_httpd_mutex = NULL;
    }
    return ret;
}

MW_ERROR_NO_T
mw_httpd_mutex_deinit(
    void)
{
    if (NULL != _mw_httpd_mutex)
    {
        osapi_mutexDelete(_mw_httpd_mutex);
        _mw_httpd_mutex = NULL;
    }
    return MW_E_OK;
}

MW_ERROR_NO_T
mw_httpd_mutex_take(
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if(NULL != _mw_httpd_mutex)
    {
        ret = osapi_mutexTake(_mw_httpd_mutex, timeout);
    }

    return ret;
}

MW_ERROR_NO_T
mw_httpd_mutex_give(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if (NULL != _mw_httpd_mutex)
    {
        ret = osapi_mutexGive(_mw_httpd_mutex);
    }

    return ret;
}
#endif /* AIR_SUPPORT_HTTPD_MUTEX */