/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:   mw_telnet.h
 * PURPOSE:
 *      It provides telnet API and definitions.
 * NOTES:
 *
 */

#ifndef MW_TELNET_H
#define MW_TELNET_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_log.h"
#include "osapi_thread.h"
#include "lwip/sockets.h"
#ifdef AIR_SUPPORT_REMOTE_DEBUG
#include "remote_debug_log.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define TELNET_TASK_NAME            "telnet"
#define TELNET_TASK_PRIORITY        5
#define TELNET_STACK_SIZE           400
#define TELNET_QUEUE_NAME           "tnq"
#define TELNET_GET_QUEUE_NAME       "tng"

#ifdef AIR_SUPPORT_REMOTE_DEBUG
#define TELNET_LOG_MSG_LEN          (REMOTE_DEBUG_LOG_MAX_ENTRY_NUM)
#define TELNET_LOG_MSG_ID           (REMOTE_DEBUG_LOG_MSG_ID_NOTIFY)
#else
#define TELNET_LOG_MSG_LEN          0
#define TELNET_LOG_MSG_ID           (MW_MSG_ID_SELF_DEFINED_BASE)
#endif

#ifdef AIR_SUPPORT_CLI
#define TELNET_CLI_MSG_MAX_LEN      6
#define TELNET_CMD_RSP_MSG_ID       (TELNET_LOG_MSG_ID + 1)
#define TELNET_CMD_RSP_TIMEOUT_MS   pdMS_TO_TICKS(100) /* 100 ms */
#else
#define TELNET_CLI_MSG_MAX_LEN      1
#endif

#define TELNET_QUEUE_LEN            (TELNET_LOG_MSG_LEN + TELNET_CLI_MSG_MAX_LEN)
#define TELNET_GET_QUEUE_LEN        1
#define TELNET_QUEUE_TIMEOUT        pdMS_TO_TICKS(50) /* 50ms */
#define TELNET_LOGIN_TIMEOUT_MS     pdMS_TO_TICKS(60000) /* 1 minutes */
#define TELNET_DELAY_CLOSE_TIME     pdMS_TO_TICKS(50) /* 50 ms */
#define TELNET_MAX_CLIENT_NUM       1

/* MACRO FUNCTION DECLARATIONS
 */
#define TELNET_LOG_ERROR(fmt, ...)     MW_LOG_ERROR(TELNET, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define TELNET_LOG_WARN(fmt, ...)      MW_LOG_WARN(TELNET, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define TELNET_LOG_INFO(fmt, ...)      MW_LOG_INFO(TELNET, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define TELNET_LOG_DEBUG(fmt, ...)     MW_LOG_DEBUG(TELNET, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#ifndef AIR_SUPPORT_REMOTE_DEBUG
extern int uart_printf(const char *fmt, ...);
#endif
/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    TELNET_LOGIN_INIT = 0,
    TELNET_LOGIN_INPUT_NAME,
    TELNET_LOGIN_INPUT_PASSWORD,
    TELNET_LOGIN_PHASE_END
} MW_TELNET_LOGIN_PHASE_T;

typedef enum {
    TELNET_LOGIN_STATUS_INIT = 0,
    TELNET_LOGIN_INPUT_WRONG = TELNET_LOGIN_STATUS_INIT,
    TELNET_LOGIN_NAME_WRONG = TELNET_LOGIN_INPUT_WRONG,
    TELNET_LOGIN_PASSWORD_WRONG = TELNET_LOGIN_NAME_WRONG,
    TELNET_LOGIN_NAME_CORRECT,
    TELNET_LOGIN_PASSWORD_CORRECT,
    TELNET_LOGIN_SUCCESS = TELNET_LOGIN_PASSWORD_CORRECT
} MW_TELNET_LOGIN_STATUS_T;

typedef struct MW_TELNET_OPTION_S
{
    const C8_T *ptr_option;
    UI8_T value;
} MW_TELNET_OPTION_T;

typedef struct MW_TELNET_CLI_MSG_S {
    UI8_T msg_id;
    UI8_T session_id;
    UI16_T len;
    const C8_T *ptr_str;
} MW_TELNET_CLI_MSG_T;

typedef struct MW_TELNET_CONNECTION_S {
    I32_T session_id;
    I32_T client_fd;
    UI8_T login_phase;
    UI8_T login_status;
    UI8_T login_times;
    TickType_t login_start_tick;
    UI8_T last_input_char;
} MW_TELNET_CONNECTION_T;

typedef struct MW_TELNET_INFO_S {
    threadhandle_t telnet_task_handle;
    StackType_t telnet_task_stack[TELNET_STACK_SIZE];
    StaticTask_t telnet_task_tcb;
    BOOL_T log_enable;
    I32_T server_fd;
    struct sockaddr_in local_addr;
    MW_TELNET_CONNECTION_T client_conn[TELNET_MAX_CLIENT_NUM];
} MW_TELNET_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T
mw_telnet_init(
    void);

BOOL_T
mw_telnet_get_connection_stats(
    void);
#endif /* MW_TELNET_H_ */
