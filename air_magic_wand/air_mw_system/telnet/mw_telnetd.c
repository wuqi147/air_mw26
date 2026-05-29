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

/* FILE NAME:   mw_telnetd.c
 * PURPOSE:
 *      Implement the basic functions of a telnet server.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/prot/iana.h"
#include "osapi_message.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "mw_msg.h"
#include "mw_cmd_util.h"
#include "db_api.h"
#include "mw_telnet.h"
#include "mw_telnet_queue.h"
#include "cmd_interpreter.h"
#include "mw_account.h"
#ifdef AIR_SUPPORT_CLI
#include "cli_internal.h"
#include "cli.h"
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define CMD_MAX_INPUT_SIZE          256
#define CMD_MAX_OUTPUT_SIZE         256
#define CMD_NEWLINE                 "\r\n#"
#define BR                          "\r\n"
#define USERNAME_PROMPT             "Username:"
#define PASSWORD_PROMPT             BR"Password:"
#define PASSWORD_MASK               "*"
#define LOGIN_FAILED                BR"Username or password incorrect! Please Try again."BR
#define LOGIN_SUCCESS               BR"Login success!"
#define FORCE_EXIT_CODE             "telnet force exit"
#define SELECT_TIMEOUT_SEC          0
#define SELECT_TIMEOUT_USEC         0

#define TELNET_PORT                 htons(((UI16_T)LWIP_IANA_PORT_TELNET))
#if LWIP_TCP_KEEPALIVE
#define TELNET_SOCK_KEEPIDLE        150  /* seconds */
#define TELNET_SOCK_KEEPINTVL       5    /* seconds */
#define TELNET_SOCK_KEEPCNT         6
#endif
#if LWIP_SO_LINGER
#define TELNET_LINGER_TIMEOUT       0    /* seconds */
#endif
#define IAC                         255  /* interpret as Command */
#define UP                          'A'
#define DOWN                        'B'
#define RIGHT                       'C'
#define LEFT                        'D'
#define ESC                         '\e'
#define LEFT_SQUARE_BRACKET         '['
#define BACKSPACE                   '\b'
#define ENTER                       '\r'
#define SPACE                       ' '
#define NEWLINE                     '\n'
#define NUL                         '\0'

#define CONTROL_CODE_UP             "\x1b\x5b\x41"  /* ESC[A */
#define CONTROL_CODE_DOWN           "\x1b\x5b\x42"  /* ESC[B */
#define CONTROL_CODE_RIGHT          "\x1b\x5b\x43"  /* ESC[C */
#define CONTROL_CODE_LEFT           "\x1b\x5b\x44"  /* ESC[D */
#define CONTROL_CODE_ARROW_LEN      3

#define TELNET_OPTION_WILL         251
#define TELNET_OPTION_WONT         252
#define TELNET_OPTION_DO           253
#define TELNET_OPTION_DONT         254
#define TELNET_OPTION_IAC          255
#define TELNET_OPTION_ECHO         1
#define TELNET_OPTION_SGA          3
#define TELNET_OPTION_LFLOW        33
#define TELNET_OPTION_LINEMODE     34

#define MAX_LOGIN_ATTEMPTS          5

/* STATIC VARIABLE DECLARATIONS
 */
static MW_TELNET_INFO_T *ptr_telnet_info = NULL;

#ifdef AIR_SUPPORT_CLI
static const C8_T *ptr_welcome_message = "Welcome to CLI via Telnet."BR;
#else
static const C8_T *ptr_welcome_message = "Welcome %s:%d. Please log in with your username and password to continue."BR;
static const C8_T *ptr_attention_message = "Please be aware that you have 1 minute or up to 5 login attempts and unauthorized access is prohibited."BR BR;
static const C8_T *ptr_login_exceed_max_cnt_msg = BR"Maximum login attempts exceeded. The Telnet server will terminate the connection."BR;
static const C8_T *ptr_login_timeout_msg = BR"Login timeout. The Telnet server will terminate the connection."BR;
static const C8_T *ptr_disconnect_msg = BR"Connection terminated."BR;
static const UI8_T backspace_echo[] = { BACKSPACE, SPACE, BACKSPACE };
#endif
static const UI8_T single_character_mode_option[] = {
    TELNET_OPTION_IAC, TELNET_OPTION_DO, TELNET_OPTION_ECHO,
    TELNET_OPTION_IAC, TELNET_OPTION_WILL, TELNET_OPTION_ECHO,
    TELNET_OPTION_IAC, TELNET_OPTION_WILL, TELNET_OPTION_SGA
};

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/
#if LWIP_SO_LINGER
static MW_ERROR_NO_T
_mw_telnet_set_force_hard_close(
    I32_T socket_fd)
{
    struct linger telnet_linger;
    telnet_linger.l_onoff = 1;
    telnet_linger.l_linger = TELNET_LINGER_TIMEOUT;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &telnet_linger, sizeof(telnet_linger)) < 0)
    {
        TELNET_LOG_WARN("Failed to set SO_LINGER on socket %d", socket_fd);
    }
    close(socket_fd);

    return MW_E_OK;
}
#endif

#ifndef AIR_SUPPORT_CLI
static MW_ERROR_NO_T
_mw_telnet_handle_direction_operation(
    UI8_T inputChar,
    C8_T *ptr_cmd,
    UI32_T *ptr_cmd_len,
    I32_T socket)
{
    /* arrow left / right */
    if (LEFT_SQUARE_BRACKET == inputChar) /* '[' */
    {
        return MW_E_OP_INCOMPLETE; /* for NEXT. */
    }
    else if (RIGHT == inputChar) /* right */
    {
        if ((*ptr_cmd_len >= 0) && (ptr_cmd + (*ptr_cmd_len) != NULL))
        {
            (*ptr_cmd_len)++;
            send(socket, CONTROL_CODE_RIGHT, CONTROL_CODE_ARROW_LEN, 0);
        }
    }
    else if (LEFT == inputChar) /* left */
    {
        if (*ptr_cmd_len > 0)
        {
            (*ptr_cmd_len)--;
            send(socket, CONTROL_CODE_LEFT, CONTROL_CODE_ARROW_LEN, 0);
        }
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_mw_telnet_handle_login_operation(
    I32_T socket,
    UI8_T *ptr_login_phase,
    UI8_T *ptr_login_status,
    C8_T *ptr_val)
{
    UI32_T i = 0, idx = 0;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T dataSize = 0;
    UI8_T *ptrData = NULL;
    C8_T *ptr_username = NULL;
    C8_T *ptr_passwd = NULL;

    if ((NULL == ptr_login_phase) || (NULL == ptr_login_status) || (NULL == ptr_val))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (TELNET_LOGIN_INIT == *ptr_login_phase)
    {
        /* default state, show username: */
        send(socket, USERNAME_PROMPT, osapi_strlen(USERNAME_PROMPT), 0);
        *ptr_login_phase = TELNET_LOGIN_INPUT_NAME;
        TELNET_LOG_INFO("Client(socket %d) is logging in...", socket);
    }
    else if (TELNET_LOGIN_INPUT_NAME == *ptr_login_phase)
    {
        if (MW_E_OK != mw_telnet_queue_getData(ACCOUNT_INFO, ACC_USERNAME, DB_ALL_ENTRIES, &ptr_msg, &dataSize, (void **)&ptrData))
        {
            TELNET_LOG_ERROR("Failed to get username from DB");
            return MW_E_OP_STOPPED;
        }

        if (MW_E_OK != osapi_calloc(MAX_USER_NAME_SIZE, TELNET_TASK_NAME, (void **)&ptr_username))
        {
            TELNET_LOG_ERROR("Failed to allocate memory for username");
            MW_FREE(ptr_msg);
            return MW_E_OP_STOPPED;
        }

        osapi_memcpy(ptr_username, ptrData, dataSize);
        MW_FREE(ptr_msg);

        if (MW_E_OK == mw_account_verifyUser(ptr_val, ptr_username))
        {
            *ptr_login_status = TELNET_LOGIN_NAME_CORRECT;
        }
        else
        {
            *ptr_login_status = TELNET_LOGIN_NAME_WRONG;
        }
        send(socket, PASSWORD_PROMPT, osapi_strlen(PASSWORD_PROMPT), 0);
        *ptr_login_phase = TELNET_LOGIN_INPUT_PASSWORD;
        MW_FREE(ptr_username);
    }
    else if (TELNET_LOGIN_INPUT_PASSWORD == *ptr_login_phase)
    {
        for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
        {
            ptr_conn = &ptr_telnet_info->client_conn[i];
            if (socket == ptr_conn->client_fd)
            {
                idx = i;
                break;
            }
        }

        if (MW_E_OK != mw_telnet_queue_getData(ACCOUNT_INFO, ACC_PASSWD, DB_ALL_ENTRIES, &ptr_msg, &dataSize, (void **)&ptrData))
        {
            TELNET_LOG_ERROR("Failed to get password from DB");
            return MW_E_OP_STOPPED;
        }

        if (MW_E_OK != osapi_calloc(MAX_PASSWORD_SIZE, TELNET_TASK_NAME, (void **)&ptr_passwd))
        {
            TELNET_LOG_ERROR("Failed to allocate memory for password");
            MW_FREE(ptr_msg);
            return MW_E_OP_STOPPED;
        }
        osapi_memcpy(ptr_passwd, ptrData, dataSize);
        MW_FREE(ptr_msg);

        if ((TELNET_LOGIN_NAME_CORRECT == *ptr_login_status) && (MW_E_OK == mw_account_verifyPass(ptr_val, ptr_passwd, FALSE)))
        {
            *ptr_login_status = TELNET_LOGIN_PASSWORD_CORRECT;
            *ptr_login_phase = TELNET_LOGIN_PHASE_END;
            send(socket, LOGIN_SUCCESS, osapi_strlen(LOGIN_SUCCESS), 0);
            send(socket, CMD_NEWLINE, osapi_strlen(CMD_NEWLINE), 0);
            ptr_conn->login_times = 0;
            TELNET_LOG_INFO("Client %d(socket %d) login success", idx + 1, socket);
        }
        else
        {
            if (++(ptr_conn->login_times) >= MAX_LOGIN_ATTEMPTS)
            {
                TELNET_LOG_WARN("Client %d(socket %d) login failed too many times(%d), terminate the connection.", idx + 1, socket, MAX_LOGIN_ATTEMPTS);
                return MW_E_ENTRY_REACH_END;
            }
            *ptr_login_phase = TELNET_LOGIN_INPUT_NAME;
            *ptr_login_status = TELNET_LOGIN_STATUS_INIT;
            send(socket, LOGIN_FAILED, osapi_strlen(LOGIN_FAILED), 0);
            send(socket, BR, osapi_strlen(BR), 0);
            send(socket, USERNAME_PROMPT, osapi_strlen(USERNAME_PROMPT), 0);
        }

        MW_FREE(ptr_passwd);
    }

    return MW_E_OK;
}

static void
_mw_telnet_handle_login_timeout(
    void)
{
    UI32_T i;
    TickType_t dur_tick, end_tick;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_telnet_info->client_conn[i];
        if ((0 != ptr_conn->client_fd) && (TELNET_LOGIN_SUCCESS != ptr_conn->login_status))
        {
            end_tick = osapi_sysTickGet();
            if (end_tick >= ptr_conn->login_start_tick)
            {
                dur_tick = end_tick - ptr_conn->login_start_tick;
            }
            else
            {
                dur_tick = 0xFFFFFFFF - ptr_conn->login_start_tick + end_tick + 1;
            }
            if (dur_tick > TELNET_LOGIN_TIMEOUT_MS)
            {
                send(ptr_conn->client_fd, ptr_login_timeout_msg, osapi_strlen(ptr_login_timeout_msg), 0);
                osapi_delay(TELNET_DELAY_CLOSE_TIME);
#if LWIP_SO_LINGER
                _mw_telnet_set_force_hard_close(ptr_conn->client_fd);
#else
                close(ptr_conn->client_fd);
#endif
                TELNET_LOG_WARN("Client %d(socket %d) login timeout, terminate the connection", i + 1, ptr_conn->client_fd);
                ptr_conn->client_fd = 0;
            }
        }
    }
}

static void
_mw_telnet_send_cmd_input(
    C8_T *ptr_cmd_string,
    UI32_T cmd_len)
{
    UI32_T i;
    C8_T newline = NEWLINE;
    QueueHandle_t cmd_queue_handle = cmd_queue_get();

    if (NULL == cmd_queue_handle)
    {
        TELNET_LOG_ERROR("cmd queue is NULL");
        return;
    }

    for (i = 0; i < cmd_len; i++)
    {
        xQueueSend(cmd_queue_handle, ptr_cmd_string + i, MSG_TIMEOUT_WAIT_INDEFINITELY);
    }

    xQueueSend(cmd_queue_handle, &newline, MSG_TIMEOUT_WAIT_INDEFINITELY);
}
#endif

#ifdef AIR_SUPPORT_REMOTE_DEBUG
static MW_ERROR_NO_T
_mw_telnet_send_log_msg(
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_entry,
    void *ptr_cookie)
{
#ifdef AIR_SUPPORT_CLI
    if (0 != (mw_log_get_global_output_type() & MW_LOG_OUTPUT_TYPE_BITMAP_TELNET))
    {
        return osapi_msgSend(TELNET_QUEUE_NAME, (UI8_T *)ptr_entry, 0, 0);
    }

    return MW_E_NOT_SUPPORT;
#else
    return osapi_msgSend(TELNET_QUEUE_NAME, (UI8_T *)ptr_entry, 0, 0);
#endif
}

static BOOL_T
_mw_telnet_filter_log_msg(
    void *ptr_msg,
    void *ptr_ctx)
{
    MW_MSG_T *ptr_log_msg = (MW_MSG_T *)ptr_msg;

    if (NULL == ptr_log_msg)
    {
        return FALSE;
    }

    if (TELNET_LOG_MSG_ID == ptr_log_msg->msg_id)
    {
        return TRUE;
    }

    return FALSE;
}
#endif

#ifdef AIR_SUPPORT_CLI
static void
_mw_telnet_send_cli_output(
    UI8_T session,
    C8_T *ptr_output,
    UI32_T len)
{
    MW_TELNET_CLI_MSG_T *ptr_cli_msg = NULL;

    if ((CLI_CONSOLE_SESSION_ID == session) || (NULL == ptr_output) || (0 == len))
    {
        TELNET_LOG_ERROR("Invalid parameters: session=%d, ptr_output=%p, len=%d", session, ptr_output, len);
        return;
    }

    if (MW_E_OK != osapi_calloc(sizeof(MW_TELNET_CLI_MSG_T), TELNET_TASK_NAME, (void **)&ptr_cli_msg))
    {
        TELNET_LOG_ERROR("Failed to allocate memory for cli msg");
        return;
    }

    ptr_cli_msg->msg_id = TELNET_CMD_RSP_MSG_ID;
    ptr_cli_msg->session_id = session;
    ptr_cli_msg->len = len;
    ptr_cli_msg->ptr_str = ptr_output;

    osapi_msgSend(TELNET_QUEUE_NAME, (UI8_T *)ptr_cli_msg, 0, TELNET_CMD_RSP_TIMEOUT_MS);
}
#endif

#if LWIP_TCP_KEEPALIVE
static MW_ERROR_NO_T
_mw_telnet_set_socket_keepalive(
    I32_T socket_fd)
{
    I32_T keepalive = 1, keepidle = TELNET_SOCK_KEEPIDLE, interval = TELNET_SOCK_KEEPINTVL, cnt = TELNET_SOCK_KEEPCNT;
    /* Total time (in seconds) = keepidle + interval * cnt */

    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
    {
        TELNET_LOG_ERROR("Failed to set SO_KEEPALIVE");
        return MW_E_OP_STOPPED;
    }

    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) < 0)
    {
        TELNET_LOG_ERROR("Failed to set TCP_KEEPIDLE");
        return MW_E_OP_STOPPED;
    }

    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0)
    {
        TELNET_LOG_ERROR("Failed to set TCP_KEEPINTVL");
        return MW_E_OP_STOPPED;
    }

    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt)) < 0)
    {
        TELNET_LOG_ERROR("Failed to set TCP_KEEPCNT");
        return MW_E_OP_STOPPED;
    }

    return MW_E_OK;
}
#endif

#ifdef AIR_SUPPORT_CLI
static void
_mw_telnet_handle_IO_operation(
    fd_set *ptr_fds)
{
    UI32_T i;
    I32_T sd, val_recv;
    UI8_T input_char, client_cnt = 0;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        if (0 != ptr_telnet_info->client_conn[i].client_fd)
        {
            client_cnt++;
        }
    }

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_telnet_info->client_conn[i];
        if (0 == ptr_conn->client_fd)
        {
            continue;
        }
        sd = ptr_conn->client_fd;

        if (FD_ISSET(sd, ptr_fds))
        {
            val_recv = recv(sd, &input_char, sizeof(input_char), 0);
            if (val_recv == 0)
            {
                cli_send_session_deregster_msg(ptr_conn->session_id);
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                if ((TRUE == ptr_telnet_info->log_enable) && (1 == client_cnt))
                {
                    remote_debug_log_deregister(_mw_telnet_send_log_msg, NULL);
                    ptr_telnet_info->log_enable = FALSE;
                    osapi_msgFilter(TELNET_QUEUE_NAME, _mw_telnet_filter_log_msg, NULL);
                }
#endif
                close(sd);
                TELNET_LOG_INFO("Client %d(socket %d) disconnected(recv return %d)", i + 1, ptr_conn->client_fd, val_recv);
                ptr_conn->client_fd = 0;
                ptr_conn->login_status = TELNET_LOGIN_STATUS_INIT;
                client_cnt--;
            }
            else if (val_recv < 0)
            {
                cli_send_session_deregster_msg(ptr_conn->session_id);
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                if ((TRUE == ptr_telnet_info->log_enable) && (1 == client_cnt))
                {
                    remote_debug_log_deregister(_mw_telnet_send_log_msg, NULL);
                    ptr_telnet_info->log_enable = FALSE;
                    osapi_msgFilter(TELNET_QUEUE_NAME, _mw_telnet_filter_log_msg, NULL);
                }
#endif
#if LWIP_SO_LINGER
                _mw_telnet_set_force_hard_close(sd);
#else
                close(sd);
#endif
                TELNET_LOG_INFO("Client %d(socket %d) disconnected(recv return %d)", i + 1, ptr_conn->client_fd, val_recv);
                ptr_conn->client_fd = 0;
                ptr_conn->login_status = TELNET_LOGIN_STATUS_INIT;
                client_cnt--;
            }
            else
            {
                /* Handle Telnet Line Endings (RFC 854):
                   - CR LF (0x0D 0x0A): Standard Telnet Newline -> Map to single ENTER/NEWLINE
                   - CR NUL (0x0D 0x00): Literal CR -> Map to ENTER
                   - LF (0x0A): Unix style -> Map to NEWLINE
                   We effectively strip the LF/NUL if it follows a CR to avoid double processing in CLI.
                */
                if (input_char == NEWLINE && ptr_conn->last_input_char == ENTER)
                {
                    ptr_conn->last_input_char = input_char;
                    continue; /* Ignore LF after CR (CRLF -> CR) */
                }
                else if (input_char == NUL && ptr_conn->last_input_char == ENTER)
                {
                    ptr_conn->last_input_char = input_char;
                    continue; /* Ignore NUL after CR (CR NUL -> CR) */
                }

                ptr_conn->last_input_char = input_char;

                /* Handle IAC */
                if (IAC == input_char)
                {
                    /*ignore it */
                    UI8_T tmp_buf[2];
                    recv(sd, tmp_buf, sizeof(tmp_buf), 0);
                }
                else
                {
                    /* Forward all other characters to CLI */
                    cli_input(ptr_conn->session_id, input_char);
                }
            }
        }
    }
}
#else
static void
_mw_telnet_handle_IO_operation(
    fd_set *ptr_fds,
    C8_T *ptr_input_string,
    UI32_T *ptr_input_index)
{
    UI32_T i;
    I32_T sd, val_recv;
    UI8_T input_char, client_cnt = 0;
    BOOL_T is_ESC = FALSE;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        if (0 != ptr_telnet_info->client_conn[i].client_fd)
        {
            client_cnt++;
        }
    }

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_telnet_info->client_conn[i];
        sd = ptr_conn->client_fd;

        if (FD_ISSET(sd, ptr_fds))
        {
            val_recv = recv(sd, &input_char, sizeof(input_char), 0);
            if (val_recv == 0)
            {
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                if ((TRUE == ptr_telnet_info->log_enable) && (1 == client_cnt))
                {
                    remote_debug_log_deregister(_mw_telnet_send_log_msg, NULL);
                    ptr_telnet_info->log_enable = FALSE;
                    osapi_msgFilter(TELNET_QUEUE_NAME, _mw_telnet_filter_log_msg, NULL);
                }
#endif
                close(sd);
                TELNET_LOG_INFO("Client %d(socket %d) disconnected(recv return %d)", i + 1, ptr_conn->client_fd, val_recv);
                ptr_conn->client_fd = 0;
                client_cnt--;
            }
            else if (val_recv < 0)
            {
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                if ((TRUE == ptr_telnet_info->log_enable) && (1 == client_cnt))
                {
                    remote_debug_log_deregister(_mw_telnet_send_log_msg, NULL);
                    ptr_telnet_info->log_enable = FALSE;
                    osapi_msgFilter(TELNET_QUEUE_NAME, _mw_telnet_filter_log_msg, NULL);
                }
#endif
#if LWIP_SO_LINGER
                _mw_telnet_set_force_hard_close(sd);
#else
                close(sd);
#endif
                TELNET_LOG_INFO("Client %d(socket %d) disconnected(recv return %d)", i + 1, ptr_conn->client_fd, val_recv);
                ptr_conn->client_fd = 0;
                client_cnt--;
            }
            else if (val_recv > 0)
            {
                /* Defferent OS may have different line ending character:
                     Unix/Linux/macOS: ENTER(0x0D, \n)
                     Windows: NEWLINE(0x0A, \n) + ENTER(0x0D, \r)
                     Some old or special terminal: only ENTER(0x0D, \r) */
                if ((ENTER == input_char) || (NEWLINE == input_char))
                {
                    if ((input_char == NEWLINE) && (ptr_conn->last_input_char == ENTER))
                    {
                        ptr_conn->last_input_char = input_char;
                        continue;
                    }

                    ptr_conn->last_input_char = input_char;
                    if (TELNET_LOGIN_SUCCESS != ptr_conn->login_status)
                    {
                        if (MW_E_OK != _mw_telnet_handle_login_operation(sd, &ptr_conn->login_phase, &ptr_conn->login_status, ptr_input_string))
                        {
                            send(sd, ptr_login_exceed_max_cnt_msg, osapi_strlen(ptr_login_exceed_max_cnt_msg), 0);
                            osapi_delay(TELNET_DELAY_CLOSE_TIME);
                            close(ptr_conn->client_fd);
                            ptr_conn->client_fd = 0;
                        }
                        (*ptr_input_index) = 0;
                        osapi_memset(ptr_input_string, 0, CMD_MAX_INPUT_SIZE);
                    }
                    else
                    {
                        /* Check if the input string is the force exit code. */
                        if (osapi_strcmp(FORCE_EXIT_CODE, (const C8_T *)ptr_input_string) == 0)
                        {
                            /* Set val_recv to 0 to close the connection. */
                            send(sd, ptr_disconnect_msg, osapi_strlen(ptr_disconnect_msg), 0);
                            osapi_delay(TELNET_DELAY_CLOSE_TIME);
                            val_recv = 0;
                            TELNET_LOG_INFO("Client %d(socket %d) force exit.", i + 1, ptr_conn->client_fd);
                        }
                        else
                        {
                            /* The input string was not a quit command. Pass the string to the command interpreter. */
                            _mw_telnet_send_cmd_input(ptr_input_string, (*ptr_input_index));

                            /* All the strings generated by the input command have been sent.  Clear the input string ready to receive the next command. */
                            (*ptr_input_index) = 0;
                            osapi_memset(ptr_input_string, 0, CMD_MAX_INPUT_SIZE);

                        }
                    }
                }
                else
                {
                    if (NUL == input_char)
                    {
                        if ((ENTER == ptr_conn->last_input_char) || (NEWLINE == ptr_conn->last_input_char))
                        {
                            ptr_conn->last_input_char = input_char;
                            continue;
                        }
                    }
                    ptr_conn->last_input_char = input_char;
                    if (BACKSPACE == input_char)
                    {
                        /* Backspace was pressed.  Erase the last character in the string - if any. */
                        if (((*ptr_input_index) > 0) && (osapi_strlen(ptr_input_string) <= (*ptr_input_index)))
                        {
                            send(sd, backspace_echo, sizeof(backspace_echo), 0);
                            (*ptr_input_index)--;
                            ptr_input_string[(*ptr_input_index)] = '\0';
                        }
                    }
                    /* add for IAC etc. */
                    else if (IAC == input_char)
                    {
                        /*ignore it */
                        UI8_T tmp_buf[2];
                        recv(sd, tmp_buf, sizeof(tmp_buf), 0);
                    }
                    /* ESC */
                    else if (ESC == input_char)
                    {
                        is_ESC = TRUE;
                        continue; /* for NEXT. */
                    }
                    else if (TRUE == is_ESC)
                    {
                        if (MW_E_OP_INCOMPLETE == _mw_telnet_handle_direction_operation(input_char, ptr_input_string, ptr_input_index, sd))
                        {
                            continue; /* for NEXT. */
                        }
                    }
                    else
                    {
                        /* A character was entered.  Add it to the string entered so far.  When a \n or \r is entered the complete string will be passed to the command interpreter. */
                        if ((*ptr_input_index) < CMD_MAX_INPUT_SIZE)
                        {
                            ptr_input_string[(*ptr_input_index)] = input_char;
                            (*ptr_input_index)++;
                        }

                        if (TELNET_LOGIN_INPUT_PASSWORD == ptr_conn->login_phase)
                        {
                            send(sd, PASSWORD_MASK, 1, 0);
                        }
                        else
                        {
                            send(sd, &input_char, 1, 0);
                        }
                    }

                    is_ESC = FALSE;
                }

                if (0 == val_recv)
                {
                    close(sd);
                    ptr_conn->client_fd = 0;
                }
            }
        }
    }
}
#endif

static MW_ERROR_NO_T
_mw_telnet_handle_msg(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T i;
    MW_MSG_T *ptr_msg = NULL;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;
    (void)ptr_conn;
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    REMOTE_DEBUG_LOG_ENTRY_T *ptr_remote_debug_log_msg = NULL;
#endif
#ifdef AIR_SUPPORT_CLI
    MW_TELNET_CLI_MSG_T *ptr_cli_msg = NULL;
#endif

    (void)i;

    rc = osapi_msgRecv(TELNET_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, TELNET_QUEUE_TIMEOUT);
    if ((MW_E_OK == rc) && (NULL != ptr_msg))
    {
        switch (ptr_msg->msg_id)
        {
            case TELNET_LOG_MSG_ID:
#ifdef AIR_SUPPORT_REMOTE_DEBUG
            {
                ptr_remote_debug_log_msg = (REMOTE_DEBUG_LOG_ENTRY_T *)ptr_msg;
                if ((ptr_remote_debug_log_msg->ref_cnt > 0) && (NULL != ptr_remote_debug_log_msg->data))
                {
                    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
                    {
                        ptr_conn = &ptr_telnet_info->client_conn[i];
                        if ((0 != ptr_conn->client_fd) && (TELNET_LOGIN_SUCCESS == ptr_conn->login_status))
                        {
                            send(ptr_conn->client_fd, ptr_remote_debug_log_msg->data, ptr_remote_debug_log_msg->len, 0);
                        }
                    }
                    remote_debug_log_release_entry(ptr_remote_debug_log_msg);
                }
                break;
            }
#endif
#ifdef AIR_SUPPORT_CLI
            case TELNET_CMD_RSP_MSG_ID:
            {
                ptr_cli_msg = (MW_TELNET_CLI_MSG_T *)ptr_msg;
                for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
                {
                    MW_TELNET_CONNECTION_T *ptr_conn = &ptr_telnet_info->client_conn[i];
                    if (ptr_cli_msg->session_id == ptr_conn->session_id)
                    {
                        if (0 != ptr_conn->client_fd)
                        {
                            send(ptr_conn->client_fd, ptr_cli_msg->ptr_str, ptr_cli_msg->len, 0);
                        }
                        break;
                    }
                }
                MW_FREE(ptr_cli_msg->ptr_str);
                MW_FREE(ptr_cli_msg);
                break;
            }
#endif
            default:
                TELNET_LOG_ERROR("Invalid message received");
                break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mw_telnet_deinit(
    void)
{
    msghandle_t telnet_q_handle = osapi_msgFindHandle(TELNET_QUEUE_NAME);
    msghandle_t telnet_get_q_handle = osapi_msgFindHandle(TELNET_GET_QUEUE_NAME);
    threadhandle_t task_handle = NULL;

    osapi_printf("Telnet deinit!\n");

    MW_FREE(ptr_telnet_info);

    if (NULL != telnet_q_handle && osapi_msgDelete(TELNET_QUEUE_NAME) != MW_E_OK)
    {
        TELNET_LOG_ERROR("Telnet delete queue fail!");
    }

    if (NULL != telnet_get_q_handle && osapi_msgDelete(TELNET_GET_QUEUE_NAME) != MW_E_OK)
    {
        TELNET_LOG_ERROR("Telnet delete get queue fail!");
    }

    task_handle = ptr_telnet_info->telnet_task_handle;
    MW_FREE(ptr_telnet_info);

    if (NULL != task_handle)
    {
        return osapi_threadDelete(task_handle);
    }

    return MW_E_OK;
}

void
_mw_telnet_task(
    void *pvParameters)
{
    I32_T max_sd, sd, activity, new_socket;
    UI32_T i, addr_len = sizeof(struct sockaddr_in);
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;
    UI32_T clients_connected = 0;
    struct sockaddr_in client_addr;
    struct timeval timeout;
    MW_ERROR_NO_T rc = MW_E_OK;
#ifndef AIR_SUPPORT_CLI
    UI32_T input_index = 0;
    C8_T *ptr_input_string;
#endif

    /* Set of socket descriptors */
    fd_set readfds;

    do
    {
        osapi_delay(TELNET_DELAY_CLOSE_TIME);
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

#ifndef AIR_SUPPORT_CLI
    rc = osapi_calloc(sizeof(C8_T) * CMD_MAX_INPUT_SIZE, TELNET_TASK_NAME, (void **)&ptr_input_string);
    if (MW_E_OK != rc)
    {
        TELNET_LOG_ERROR("Failed to allocate memory for input string, rc = %d", rc);
        return;
    }
#endif

    while (1)
    {
#ifndef AIR_SUPPORT_CLI
        osapi_memset(ptr_input_string, 0, sizeof(C8_T) * CMD_MAX_INPUT_SIZE);
#endif
        osapi_memset(ptr_telnet_info->client_conn, 0, sizeof(MW_TELNET_CONNECTION_T) * TELNET_MAX_CLIENT_NUM);

        /* Create a server socket */
        ptr_telnet_info->server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (ptr_telnet_info->server_fd >= 0)
        {
            osapi_memset(&ptr_telnet_info->local_addr, 0, sizeof(ptr_telnet_info->local_addr));
            ptr_telnet_info->local_addr.sin_family = AF_INET;
            ptr_telnet_info->local_addr.sin_len = sizeof(ptr_telnet_info->local_addr);
            ptr_telnet_info->local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            ptr_telnet_info->local_addr.sin_port = TELNET_PORT;

            if (bind(ptr_telnet_info->server_fd, (struct sockaddr *)&ptr_telnet_info->local_addr, sizeof(ptr_telnet_info->local_addr)) < 0)
            {
                close(ptr_telnet_info->server_fd);
                _mw_telnet_deinit();
            }

            if (listen(ptr_telnet_info->server_fd, TELNET_MAX_CLIENT_NUM) != 0)
            {
                close(ptr_telnet_info->server_fd);
                _mw_telnet_deinit();
            }

            while (1)
            {
                /* Clear the socket set */
                FD_ZERO(&readfds);
                max_sd = 0;
                clients_connected = 0;
                for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
                {
                    if (ptr_telnet_info->client_conn[i].client_fd != 0)
                    {
                        clients_connected++;
                    }
                }

                if (clients_connected < TELNET_MAX_CLIENT_NUM)
                {
                    /* Add master socket to set */
                    FD_SET(ptr_telnet_info->server_fd, &readfds);
                    max_sd = ptr_telnet_info->server_fd;
                }

                for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
                {
                    sd = ptr_telnet_info->client_conn[i].client_fd;

                    if (sd > 0)
                    {
                        FD_SET(sd, &readfds);
                    }

                    /* Highest file descriptor number, need it for the select() function */
                    if (sd > max_sd)
                    {
                        max_sd = sd;
                    }
                }


                timeout.tv_sec = SELECT_TIMEOUT_SEC;
                timeout.tv_usec = SELECT_TIMEOUT_USEC;

                activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

                if (activity < 0)
                {
                    close(ptr_telnet_info->server_fd);
                    break;
                }
                else if (activity > 0)
                {
                    /* If something happened on the server socket, then it's an incoming connection */
                    if (FD_ISSET(ptr_telnet_info->server_fd, &readfds))
                    {
                        if ((new_socket = accept(ptr_telnet_info->server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len)) < 0)
                        {
                            close(ptr_telnet_info->server_fd);
                            TELNET_LOG_ERROR("Failed to accept connection");
                            break;
                        }
#if LWIP_TCP_KEEPALIVE
                        if (MW_E_OK != _mw_telnet_set_socket_keepalive(new_socket))
                        {
                            close(ptr_telnet_info->server_fd);
                            break;
                        }
#endif
                        C8_T ip_str[IP4ADDR_STRLEN_MAX] = { 0 };
                        TELNET_LOG_INFO("New connection, socket=%d, ip=%s, port=%d", new_socket, inet_ntoa_r(client_addr.sin_addr, ip_str, IP4ADDR_STRLEN_MAX), ntohs(client_addr.sin_port));

                        for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
                        {
                            ptr_conn = &ptr_telnet_info->client_conn[i];
                            if (0 == ptr_conn->client_fd)
                            {
                                ptr_conn->client_fd = new_socket;
                                TELNET_LOG_INFO("Adding socket(%d) to list of sockets as index: %d", new_socket, i);
#ifdef AIR_SUPPORT_CLI
                                ptr_conn->session_id = cli_session_register(_mw_telnet_send_cli_output);
                                if (ptr_conn->session_id < 0)
                                {
                                    TELNET_LOG_ERROR("Failed to register CLI session for client socket(%d)", new_socket);
                                }
                                /* Skip Login */
                                ptr_conn->login_status = TELNET_LOGIN_SUCCESS;
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                                if (FALSE == ptr_telnet_info->log_enable)
                                {
                                    remote_debug_log_register(_mw_telnet_send_log_msg, NULL);
                                    ptr_telnet_info->log_enable = TRUE;
                                }
#endif
                                break;
                            }
                        }

                        send(new_socket, single_character_mode_option, sizeof(single_character_mode_option), 0);
#ifdef AIR_SUPPORT_CLI
                        send(new_socket, ptr_welcome_message, osapi_strlen(ptr_welcome_message), 0);
                        /* Trigger initial prompt redraw */
                        cli_input(ptr_conn->session_id, NEWLINE);
#else
                        C8_T *ptr_wel_msg = NULL;
                        rc = osapi_calloc(osapi_strlen(ptr_welcome_message) + 40, TELNET_TASK_NAME, (void **)&ptr_wel_msg);
                        if (MW_E_OK != rc)
                        {
                            TELNET_LOG_ERROR("Failed to allocate memory for welcome message, rc: %d", rc);
                            break;
                        }

                        sprintf(ptr_wel_msg, ptr_welcome_message, ip_str, ntohs(client_addr.sin_port));
                        send(new_socket, ptr_wel_msg, osapi_strlen(ptr_wel_msg), 0);
                        send(new_socket, ptr_attention_message, osapi_strlen(ptr_attention_message), 0);
                        MW_FREE(ptr_wel_msg);

                        input_index = 0;
                        osapi_memset(ptr_input_string, 0, CMD_MAX_INPUT_SIZE);
                        ptr_conn->login_phase = TELNET_LOGIN_INIT;
                        ptr_conn->login_status = TELNET_LOGIN_STATUS_INIT;
                        ptr_conn->login_times = 0;
                        _mw_telnet_handle_login_operation(new_socket, &ptr_conn->login_phase, &ptr_conn->login_status, "");

                        ptr_conn->login_start_tick = osapi_sysTickGet();
#endif
                    }

                    /* Else it's some IO operation on some other socket*/
#ifdef AIR_SUPPORT_CLI
                    _mw_telnet_handle_IO_operation(&readfds);
#else
                    _mw_telnet_handle_IO_operation(&readfds, ptr_input_string, &input_index);
#endif
                }

                _mw_telnet_handle_msg();
#ifndef AIR_SUPPORT_CLI
                _mw_telnet_handle_login_timeout();
#endif
            }
        }
        else
        {
            TELNET_LOG_ERROR("Failed to create server socket, fd=%d", ptr_telnet_info->server_fd);
        }
    }

#ifndef AIR_SUPPORT_CLI
    MW_FREE(ptr_input_string);
#endif
    _mw_telnet_deinit();
}

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
mw_telnet_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_LOG_INIT_PRINTF("Initializing Telnet server...\n");

    rc = osapi_calloc(sizeof(MW_TELNET_INFO_T), TELNET_TASK_NAME, (void **)&ptr_telnet_info);
    if (MW_E_OK != rc)
    {
        osapi_printf("Telnet init memory fail!\n");
        return rc;
    }

    rc = osapi_msgCreate(TELNET_QUEUE_NAME, TELNET_QUEUE_LEN, sizeof(void *));
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_telnet_info);
        osapi_printf("Telnet init queue fail!\n");
        return rc;
    }

    rc = osapi_msgCreate(TELNET_GET_QUEUE_NAME, TELNET_GET_QUEUE_LEN, sizeof(void *));
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_telnet_info);
        osapi_printf("Telnet init get queue fail!\n");
        return rc;
    }

    rc = osapi_threadCreateStatic(TELNET_TASK_NAME, TELNET_STACK_SIZE, TELNET_TASK_PRIORITY, _mw_telnet_task, NULL,
                                  ptr_telnet_info->telnet_task_stack,
                                  &ptr_telnet_info->telnet_task_tcb,
                                  &ptr_telnet_info->telnet_task_handle);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_telnet_info);
        osapi_printf("Telnet init task fail!\n");
        return rc;
    }

    return rc;
}

BOOL_T
mw_telnet_get_connection_stats(
    void)
{
    UI32_T i;
    MW_TELNET_CONNECTION_T *ptr_conn = NULL;

    if (NULL == ptr_telnet_info)
    {
        return FALSE;
    }

    for (i = 0; i < TELNET_MAX_CLIENT_NUM; i++)
    {
        ptr_conn = &ptr_telnet_info->client_conn[i];
        if ((0 != ptr_conn->client_fd) && (TELNET_LOGIN_SUCCESS == ptr_conn->login_status))
        {
            return TRUE;
        }
    };

    return FALSE;
}
