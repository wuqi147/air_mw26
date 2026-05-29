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

/* FILE NAME:  httpd_util.h
 * PURPOSE:
 *  Define types and declare functions for httpd.
 *
 * NOTES:
 *
 */

#ifndef _HTTPD_UTIL_H_
#define _HTTPD_UTIL_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/opt.h"
#include "lwip/apps/httpd_opts.h"
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
#if !LWIP_HTTPD
// TODO: Wrap SSI handlers with LWIP_HTTPD_SSI
#define HTTPD_LAST_TAG_PART 0xFFFF
#endif

#if defined(AIR_LITE_MW)
#define HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN    (416)
#else
#define HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN    (1024)
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#if !LWIP_HTTPD
typedef MW_ERROR_NO_T(*tCGIHandler)(int iIndex, int iNumParams, char *pcParam[],
                      char *pcValue[]);
typedef struct
{
    const char *pcCGIName;
    const char *retUri;
    tCGIHandler pfnCGIHandler;
}tCGI;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#ifdef AIR_SUPPORT_HTTPD_MUTEX
MW_ERROR_NO_T
mw_httpd_mutex_init(
    void);


MW_ERROR_NO_T
mw_httpd_mutex_deinit(
    void);


MW_ERROR_NO_T
mw_httpd_mutex_take(
    const UI32_T timeout);


MW_ERROR_NO_T
mw_httpd_mutex_give(
    void);
#endif /* AIR_SUPPORT_HTTPD_MUTEX */

#endif  /*_HTTPD_UTIL_H_*/
