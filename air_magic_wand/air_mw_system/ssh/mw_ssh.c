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

/* FILE NAME:   mw_ssh.c
 * PURPOSE:
 *      Implement the basic functions of a ssh server.
 * NOTES:
 *
 */

#ifdef AIR_SUPPORT_SSH
/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_message.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "mw_msg.h"
#include "mw_cmd_util.h"
#include "mw_ssh.h"
#include "mw_ssh_msg.h"
#include "cmd_interpreter.h"
#include "dropbear_api.h"
#include "db_api.h"
#include "mw_log.h"
#include "cli.h"
/* NAMING CONSTANT DECLARATIONS
 */
#ifndef AIR_SUPPORT_CLI
#define BACKSPACE                   '\b'
#define ENTER                       '\r'
#define SPACE                       ' '
#define NEWLINE                     '\n'
#define ESC                         '\x1b'
#define LEFT_SQUARE_BRACKET         '['
#define RIGHT                       'C'
#define LEFT                        'D'

#define CONTROL_CODE_RIGHT          "\x1b\x5b\x43"
#define CONTROL_CODE_LEFT           "\x1b\x5b\x44"
#define CONTROL_CODE_ARROW_LEN      3
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static MW_SSH_INFO_T *ptr_ssh_info = NULL;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

#ifndef AIR_SUPPORT_CLI
static const UI8_T backspace_echo[] = { BACKSPACE, SPACE, BACKSPACE };

/*
 * FUNCTION NAME:   _mw_ssh_handle_direction_operation
 * PURPOSE:
 *      Handle direction keys (Arrows) for SSH in non-CLI mode.
 *
 * INPUT:
 *      input_char  - [IN] The input character.
 *      ptr_cmd     - [IN] Pointer to the command buffer.
 *      ptr_cmd_len - [IN/OUT] Pointer to the current command length.
 *      session_id  - [IN] The session ID.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */
static MW_ERROR_NO_T
_mw_ssh_handle_direction_operation(
    UI8_T inputChar,
    C8_T *ptr_cmd,
    UI32_T *ptr_cmd_len,
    int session_id)
{
    if (LEFT_SQUARE_BRACKET == inputChar)
    {
        return MW_E_OP_INCOMPLETE;
    }
    else if (RIGHT == inputChar)
    {
        /* Just echo for now, as simple buffer doesn't support cursor movement */
        common_session_reply(session_id, (const unsigned char*)CONTROL_CODE_RIGHT, CONTROL_CODE_ARROW_LEN);
    }
    else if (LEFT == inputChar)
    {
        common_session_reply(session_id, (const unsigned char*)CONTROL_CODE_LEFT, CONTROL_CODE_ARROW_LEN);
    }

    return MW_E_OK;
}
#endif

/*
 * FUNCTION NAME:   _mw_ssh_deinit
 * PURPOSE:
 *      De-initialize the SSH server resources.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */

static MW_ERROR_NO_T
_mw_ssh_deinit(
    void)
{
    msghandle_t ssh_q_handle = osapi_msgFindHandle(SSH_QUEUE_NAME);
    threadhandle_t task_handle = NULL;

    osapi_printf("SSH deinit!\n");

    if ((NULL != ssh_q_handle) && (osapi_msgDelete(SSH_QUEUE_NAME) != MW_E_OK))
    {
        MW_LOG_ERROR(SSH, "SSH delete queue fail!");
    }

    task_handle = ptr_ssh_info->ssh_task_handle;
    ptr_ssh_info->ssh_task_handle = NULL;
    MW_FREE(ptr_ssh_info);
    if (NULL != task_handle)
    {
        return osapi_threadDelete(task_handle);
    }

    return MW_E_OK;
}

/*
 * FUNCTION NAME:   _mw_ssh_task
 * PURPOSE:
 *      Main task for the SSH server.
 *
 * INPUT:
 *      ptr_vParameters - [IN] Task parameters.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */

static void
_mw_ssh_task(
    void *ptr_vParameters)
{
    while (dbapi_dbisReady() != MW_E_OK) {
        osapi_delay(SSH_DELAY_TIME);
    }

    dropbear_task(0, NULL);
    _mw_ssh_deinit();
}

#ifdef AIR_SUPPORT_REMOTE_DEBUG
/*
 * FUNCTION NAME:   _mw_ssh_send_log_msg
 * PURPOSE:
 *      Send a log entry to the SSH message queue.
 *
 * INPUT:
 *      ptr_entry  - [IN] Pointer to the log entry.
 *      ptr_cookie - [IN] User-defined cookie.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */

static MW_ERROR_NO_T
_mw_ssh_send_log_msg(
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry,
    void *ptr_cookie)
{
#ifdef AIR_SUPPORT_CLI
    if (0 != ((mw_log_get_global_output_type()) & MW_LOG_OUTPUT_TYPE_BITMAP_SSH))
    {
        return osapi_msgSend(SSH_QUEUE_NAME, (UI8_T *)ptr_entry, 0, 0);
    }
    return MW_E_NOT_SUPPORT;
#else
    return osapi_msgSend(SSH_QUEUE_NAME, (UI8_T *)ptr_entry, 0, 0);
#endif
}

/*
 * FUNCTION NAME:   _mw_ssh_filter_log_msg
 * PURPOSE:
 *      Filter messages in ssh queue.
 *
 * INPUT:
 *      ptr_msg    - [IN] Pointer to the message.
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE if the message should be deleted.
 *
 * NOTES:
 *      None.
 */
static BOOL_T
_mw_ssh_filter_log_msg(
    void *ptr_msg,
    void *ptr_ctx)
{
    MW_MSG_T *ptr_log_msg = (MW_MSG_T *)ptr_msg;

    if (NULL == ptr_log_msg)
    {
        return FALSE;
    }

    if (REMOTE_DEBUG_LOG_MSG_ID_NOTIFY == ptr_log_msg->msg_id)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

/*
 * FUNCTION NAME:   _mw_ssh_send_cli_output
 * PURPOSE:
 *      This callback function handles CLI command output and forwards it to the SSH task for further processing.
 *
 * INPUT:
 *      session    - [IN] Session ID.
 *      ptr_output - [IN] Pointer to the output string.
 *      len        - [IN] Length of the output string.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */

#ifdef AIR_SUPPORT_CLI
static void
_mw_ssh_handle_cli_output_cb(
    UI8_T session,
    C8_T *ptr_output,
    UI32_T len)
{
    int i;
    MW_SSH_CLI_MSG_T *ptr_cli_msg = NULL;
    MW_SSH_CONNECTION_T *ptr_conn = NULL;

    if ((NULL == ptr_output) || (0 == len))
    {
        MW_LOG_ERROR(SSH, "Invalid parameters: session=%d, ptr_output=%p, len=%d", session, ptr_output, len);
        return;
    }

    for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_ssh_info->client_conn[i];
        if ((session == ptr_conn->session_id))
        {
            if (MW_E_OK != osapi_calloc(sizeof(MW_SSH_CLI_MSG_T), SSH_TASK_NAME, (void **)&ptr_cli_msg))
            {
                MW_LOG_ERROR(SSH, "Failed to allocate memory for cli msg");
                return;
            }

            ptr_cli_msg->msg_id = SSH_MSG_ID_CLI_OUTPUT;
            ptr_cli_msg->session_id = session;
            ptr_cli_msg->len = len;
            ptr_cli_msg->ptr_str = ptr_output;

            if (MW_E_OK != osapi_msgSend(SSH_QUEUE_NAME, (UI8_T *)ptr_cli_msg, 0, SSH_QUEUE_SEND_WAIT_TIME))
            {
                MW_LOG_ERROR(SSH, "Failed to send message to SSH queue");
                MW_FREE(ptr_cli_msg);
            }
            break;
        }
    }
}
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/*
 * FUNCTION NAME:   mw_ssh_init
 * PURPOSE:
 *      Initialize the SSH server.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */

MW_ERROR_NO_T
mw_ssh_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_LOG_INIT_PRINTF("Initializing SSH server...\n");
    rc = osapi_calloc(sizeof(MW_SSH_INFO_T), SSH_TASK_NAME, (void **)&ptr_ssh_info);
    if (MW_E_OK != rc)
    {
        osapi_printf("SSH init memory fail!\n");
        return rc;
    }

    rc = osapi_msgCreate(SSH_QUEUE_NAME, SSH_QUEUE_LEN, sizeof(void *));
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_ssh_info);
        osapi_printf("SSH init queue fail!\n");
        return rc;
    }

    rc = osapi_threadCreateStatic(SSH_TASK_NAME, SSH_STACK_SIZE, SSH_TASK_PRIORITY, _mw_ssh_task, NULL,
                                  ptr_ssh_info->ssh_task_stack,
                                  &ptr_ssh_info->ssh_task_tcb,
                                  &ptr_ssh_info->ssh_task_handle);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_ssh_info);
        osapi_printf("SSH init task fail!\n");
        return rc;
    }

    return rc;
}

/*
 * FUNCTION NAME:   mw_ssh_login
 * PURPOSE:
 *      Register a new SSH session and initialize resources if needed.
 *
 * INPUT:
 *      ptr_session_id - [OUT] Pointer to store the allocated session ID.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */

MW_ERROR_NO_T
mw_ssh_login(
    int *ptr_session_id)
{
    UI8_T i;
    MW_SSH_CONNECTION_T *ptr_conn = NULL;

    if (NULL == ptr_session_id)
    {
        return MW_E_BAD_PARAMETER;
    }

    if(NULL == ptr_ssh_info)
    {
        return MW_E_OP_INCOMPLETE;
    }

    for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_ssh_info->client_conn[i];
        if (FALSE == ptr_conn->login_status)
        {
#ifdef AIR_SUPPORT_CLI
            ptr_conn->session_id = cli_session_register(_mw_ssh_handle_cli_output_cb);
            if (ptr_conn->session_id < 0)
            {
                MW_LOG_ERROR(SSH, "Failed to register CLI session");
                return MW_E_OP_INCOMPLETE;
            }
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
            if (FALSE == ptr_ssh_info->log_enable)
            {
                remote_debug_log_register(_mw_ssh_send_log_msg, NULL);
                ptr_ssh_info->log_enable = TRUE;
            }
#endif
            MW_LOG_INFO(SSH, "SSH session registered");
            ptr_ssh_info->client_num++;
            *ptr_session_id = ptr_conn->session_id;
            ptr_conn->login_status = TRUE;
#ifndef AIR_SUPPORT_CLI
            ptr_conn->session_id = i;
            *ptr_session_id = ptr_conn->session_id;
            ptr_conn->input_index = 0;
            ptr_conn->last_input_char = 0;
            ptr_conn->is_esc = FALSE;
            osapi_memset(ptr_conn->input_string, 0, CMD_MAX_INPUT_SIZE);
#endif
            return MW_E_OK;
        }
    }

    return MW_E_NO_MEMORY;
}

/*
 * FUNCTION NAME:   mw_ssh_logout
 * PURPOSE:
 *      Deregister an SSH session and release associated resources.
 *
 * INPUT:
 *      ptr_session_id - [IN/OUT] Pointer to the session ID; set to -1 on success.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */

void
mw_ssh_logout(
    int *ptr_session_id)
{
    UI8_T i;
    MW_SSH_CONNECTION_T *ptr_conn = NULL;

    if ((ptr_ssh_info == NULL) || (ptr_session_id == NULL) || (*ptr_session_id < 0))
    {
        return;
    }

    for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_ssh_info->client_conn[i];
        if ((*ptr_session_id == ptr_conn->session_id) && (ptr_conn->login_status == TRUE))
        {
#ifdef AIR_SUPPORT_CLI
            cli_session_deregister(ptr_conn->session_id);
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
            if ((TRUE == ptr_ssh_info->log_enable) && (1 == ptr_ssh_info->client_num))
            {
                remote_debug_log_deregister(_mw_ssh_send_log_msg, NULL);
                osapi_msgFilter(SSH_QUEUE_NAME, _mw_ssh_filter_log_msg, NULL);
                ptr_ssh_info->log_enable = FALSE;
            }
#endif
            MW_LOG_INFO(SSH, "SSH session %d logged out", *ptr_session_id);
            *ptr_session_id = -1;
            ptr_conn->session_id = -1;
            ptr_conn->login_status = FALSE;
            ptr_ssh_info->client_num--;
        }
    }
}

/*
 * FUNCTION NAME:   mw_ssh_receive_handle_msg
 * PURPOSE:
 *      Process messages from the SSH queue and broadcast to active sessions.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */


void
mw_ssh_receive_handle_msg(
    void)
{
    UI8_T i;
    MW_ERROR_NO_T rc = MW_E_OK;
    MW_MSG_T *ptr_msg = NULL;
    MW_SSH_CONNECTION_T *ptr_conn = NULL;
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_log_msg = NULL;
    MW_SSH_CLI_MSG_T *ptr_cli_msg = NULL;

    rc = osapi_msgRecv(SSH_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, SSH_QUEUE_TIMEOUT);
    if ((MW_E_OK == rc) && (NULL != ptr_msg))
    {
        if (REMOTE_DEBUG_LOG_MSG_ID_NOTIFY == ptr_msg->msg_id)
        {
            ptr_log_msg = (REMOTE_DEBUG_LOG_ENTRY_T *)ptr_msg;
            if ((ptr_log_msg->ref_cnt > 0) && ptr_log_msg->data)
            {
                for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
                {
                    ptr_conn = &ptr_ssh_info->client_conn[i];
                    if (ptr_conn->login_status == TRUE)
                    {
                        common_session_reply(ptr_conn->session_id, (const unsigned char*)ptr_log_msg->data, ptr_log_msg->len);
                    }
                }
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                remote_debug_log_release_entry(ptr_log_msg);
#endif
            }
        }
        else if (SSH_MSG_ID_CLI_OUTPUT == ptr_msg->msg_id)
        {
            ptr_cli_msg = (MW_SSH_CLI_MSG_T *)ptr_msg;
            for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
            {
                ptr_conn = &ptr_ssh_info->client_conn[i];
                if (ptr_cli_msg->session_id == ptr_conn->session_id)
                {
                    if (ptr_conn->login_status == TRUE)
                    {
                        common_session_reply(ptr_conn->session_id, (const unsigned char*)ptr_cli_msg->ptr_str, ptr_cli_msg->len);
                    }
                    break;
                }
            }
            MW_FREE(ptr_cli_msg->ptr_str);
            MW_FREE(ptr_cli_msg);
        }
    }
}

/*
 * FUNCTION NAME:   mw_ssh_handle_input
 * PURPOSE:
 *      Pass the input character from SSH client to CLI interpreter.
 *
 * INPUT:
 *      ptr_session_id - [IN] The session ID.
 *      ptr_cmd_string- [IN] The input character.
 *      input_len - [IN] The length of the input character.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T - MW_E_OK if successful.
 *
 * NOTES:
 *      None.
 */

MW_ERROR_NO_T
mw_ssh_handle_input(
    I32_T ptr_session_id,
    UI8_T *ptr_cmd_string,
    UI32_T input_len)
{
    UI32_T i;
    MW_SSH_CONNECTION_T *ptr_conn = NULL;

    if (NULL == ptr_cmd_string)
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Find connection context */
    for (i = 0; i < SSH_MAX_CLIENT_NUM; i++)
    {
        if ((ptr_ssh_info->client_conn[i].session_id == ptr_session_id) &&
            (ptr_ssh_info->client_conn[i].login_status == TRUE))
        {
            ptr_conn = &ptr_ssh_info->client_conn[i];
            break;
        }
    }

    if (NULL == ptr_conn)
    {
        return MW_E_BAD_PARAMETER;
    }

    for (i = 0; i < input_len; i++)
    {
        UI8_T input_char = *(ptr_cmd_string + i);
#ifdef AIR_SUPPORT_CLI
        cli_input(ptr_session_id, input_char);
#else
        if ((ENTER == input_char) || (NEWLINE == input_char))
        {
            if ((input_char == NEWLINE) && (ptr_conn->last_input_char == ENTER))
            {
                ptr_conn->last_input_char = input_char;
                continue;
            }

            ptr_conn->last_input_char = input_char;

            /* Submit the buffered command to interpreter */
            QueueHandle_t cmd_queue_handle = cmd_queue_get();
            if (NULL != cmd_queue_handle)
            {
                UI32_T j;
                C8_T nl = NEWLINE;
                for (j = 0; j < ptr_conn->input_index; j++)
                {
                    xQueueSend(cmd_queue_handle, &ptr_conn->input_string[j], MSG_TIMEOUT_WAIT_INDEFINITELY);
                }
                xQueueSend(cmd_queue_handle, &nl, MSG_TIMEOUT_WAIT_INDEFINITELY);
            }

            /* Echo CRLF to client */
            common_session_reply(ptr_session_id, (const unsigned char*)"\r\n", 2);

            /* Clear buffer */
            ptr_conn->input_index = 0;
            osapi_memset(ptr_conn->input_string, 0, CMD_MAX_INPUT_SIZE);
        }
        else if (BACKSPACE == input_char)
        {
            if (ptr_conn->input_index > 0)
            {
                ptr_conn->input_index--;
                ptr_conn->input_string[ptr_conn->input_index] = '\0';
                common_session_reply(ptr_session_id, backspace_echo, sizeof(backspace_echo));
            }
        }
        else if (ESC == input_char)
        {
            ptr_conn->is_esc = TRUE;
            continue;
        }
        else if (TRUE == ptr_conn->is_esc)
        {
            if (MW_E_OP_INCOMPLETE == _mw_ssh_handle_direction_operation(input_char, ptr_conn->input_string, &ptr_conn->input_index, ptr_session_id))
            {
                continue;
            }
            ptr_conn->is_esc = FALSE;
        }
        else
        {
            if (ptr_conn->input_index < CMD_MAX_INPUT_SIZE)
            {
                ptr_conn->input_string[ptr_conn->input_index] = input_char;
                ptr_conn->input_index++;
                /* Echo character back to client */
                common_session_reply(ptr_session_id, &input_char, 1);
            }
        }
        ptr_conn->last_input_char = input_char;
#endif
    }

    return MW_E_OK;
}
#endif