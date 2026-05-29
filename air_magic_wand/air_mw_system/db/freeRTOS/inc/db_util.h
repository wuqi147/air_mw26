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

/* FILE NAME:  db_util.h
 * PURPOSE:
 *      DB utility functions and definitions
 *
 * NOTES:
 */

#ifndef DB_UTIL_H
#define DB_UTIL_H


/* INCLUDE FILE DECLARATIONS
*/
#include "mw_error.h"
#include "mw_types.h"
#include "mw_utils.h"
#include "mw_log.h"
#include "mw_cmd_util.h"
#include "osapi_memory.h"
#include "osapi_string.h"

/* NAMING CONSTANT DECLARATIONS
 */
//#define DB_DBG           1
//#define DB_DBG_VRB       1            /* For verbose debug, if not nessary, do not define it */
//#define DB_DBG_TEST      1            /* For testing task */
//#define DB_SUPPORT_RESTORE_METHOD    (1)

#define DB_MAIN_TASK_NAME            "db_main"
#define DB_BUFFER_SIZE_24            (24)
#define DB_BUFFER_SIZE_64            (64)
#define DB_BUFFER_SIZE_128           (128)
#define DB_BUFFER_SIZE_256           (256)
#define DB_BUFFER_SIZE_512           (512)
#define DB_BUFFER_SIZE_1024          (1024)
#ifdef AIR_LITE_MW
#define DB_MAX_PRINTED_SIZE          (512)
#else
#define DB_MAX_PRINTED_SIZE          (4096)
#endif
/* The common log message in DB */
#define DB_INTERNAL_ERROR "Internal error"

/* MACRO FUNCTION DECLARATIONS
*/
#define DB_LOG_ERROR(fmt, ...)                    MW_LOG_ERROR(DB, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define DB_LOG_WARN(fmt, ...)                     MW_LOG_WARN(DB, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define DB_LOG_INFO(fmt, ...)                     MW_LOG_INFO(DB, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define DB_LOG_DEBUG(fmt, ...)                    MW_LOG_DEBUG(DB, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    DB_CTRL_DUMP_LEVEL_ALL = 0,
    DB_CTRL_DUMP_LEVEL_TABLE_SIZE,

    DB_CTRL_DUMP_LEVEL_LAST
} DB_CTRL_DUMP_LEVEL_T;

typedef enum
{
    DB_CFG_TYPE_RUNNING = 0,
    DB_CFG_TYPE_STARTUP,
    DB_CFG_TYPE_FACTORY,
    DB_CFG_TYPE_DEFAULT,

    DB_CFG_TYPE_LAST
} DB_CFG_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: db_calloc
 * PURPOSE:
 *      Allocate the size of memory and initialize
 *
 * INPUT:
 *      size             --  The size of the buffer
 *
 * OUTPUT:
 *      pptr_buf         --  A pointer to the buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_calloc(
    const UI32_T size,
    void **pptr_buf);

#endif  /* End of DB_UTIL_H */
