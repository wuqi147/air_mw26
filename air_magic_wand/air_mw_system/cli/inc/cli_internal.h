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

/* FILE NAME:   cli_internal.h
 * PURPOSE:
 *      Define the data structures, enums and APIs for the CLI (Command Line Interface) module.
 * NOTES:
 *      None
 */

#ifndef CLI_INTERNAL_H
#define CLI_INTERNAL_H

#ifdef AIR_SUPPORT_CLI
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "mw_error.h"
#include "mw_log.h"
#include "mw_msg.h"
#include "osapi_thread.h"
#include "default_config.h"
#include "cli.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CLI_LOG_ERROR(fmt, ...)     MW_LOG_ERROR(CLI, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CLI_LOG_WARN(fmt, ...)      MW_LOG_WARN(CLI, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CLI_LOG_INFO(fmt, ...)      MW_LOG_INFO(CLI, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define CLI_LOG_DEBUG(fmt, ...)     MW_LOG_DEBUG(CLI, "[%s][%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define CLI_CHECK_DB_READY_MS           (50)
#define CLI_CGI_STR_MAX_LEN             (512)
#define CLI_HELP_ALIGN_MIN_WIDTH        (15)
#define CLI_CMD_INPUT_MSG_ID            (MW_MSG_ID_WELL_KNOWN_LAST + 1)
#define CLI_SESSION_DEREGISTER_MSG_ID   (CLI_CMD_INPUT_MSG_ID + 1)
#define CLI_CMD_INPUT_MAGIC_NUM         (0xFF)
#define CLI_SESSION_MSG_TIMEOUT         (200)
#define CLI_RELOAD_DELAY_TIME           (500)
#define CLI_DYN_LABEL_MAX_LEN           (16)


/* Feature Flags */
/* #define CLI_CFG_USER_MODE */         /* Define to enable User Exec Mode */
/* #define CLI_CFG_TERMINAL_LENGTH */   /* Define to enable terminal length */
/* #define CLI_CFG_AAA */               /* Define to enable AAA */
#define CLI_CFG_HISTORY_CMD             /* Define to enable command history */
/* #define CLI_CFG_LINE_MODE */         /* Define to enable Line Config Mode */

#define CLI_MAX_TOKEN           (16)
#define CLI_CGI_MUX_LOCK_TIME   (50)
#define CLI_INPUT_MAX_LEN        (128)
#define CLI_HISTORY_SIZE        (5)
#define CLI_HIST_IDX_NONE       (0xFF)
#define CLI_HOSTNAME_DEFAULT    "Switch"
#define CLI_PAGE_LINES_DEFAULT  (24)
#define CLI_MAX_USERS           (1)
#define CLI_MAX_SESSIONS        (3)
#define CLI_OUTPUT_BUF_SIZE     (3072)
#define CLI_FILTER_CXT_SIZE     (32)
#define CLI_FILTER_INCLUDE_STR  "include"
#define CLI_FILTER_EXCLUDE_STR  "exclude"
#define CLI_FILTER_BEGIN_STR    "begin"
#define CLI_FILTER_SECTION_STR  "section"
#define CLI_SHOW_CONFIG_END     "end\n"

/* Auth States */
#define CLI_AUTH_STATE_SUCCESS         (0)
#define CLI_AUTH_STATE_ENABLE          (1)
#define CLI_AUTH_STATE_WAIT_USERNAME   (2)
#define CLI_AUTH_STATE_WAIT_PASSWORD   (3)
#define CLI_AUTH_STATE_RELOAD_CONFIRM  (4)

/* Keys & VT100 */
#define CLI_KEY_ENTER       '\r'
#define CLI_KEY_NEWLINE     '\n'

#define CLI_KEY_BACKSPACE   '\b'
#define CLI_KEY_DEL         0x7F
#define CLI_KEY_TAB         '\t'
#define CLI_KEY_ESC         0x1B
#define CLI_KEY_SPACE       ' '
#define CLI_KEY_Q           'q'
#define CLI_KEY_QUESTION    '?'
#define CLI_KEY_NUL         '\0'

#define CLI_KEY_CTRL_A      0x01
#define CLI_KEY_CTRL_B      0x02
#define CLI_KEY_CTRL_C      0x03
#define CLI_KEY_CTRL_E      0x05
#define CLI_KEY_CTRL_F      0x06
#define CLI_KEY_CTRL_G      0x07
#define CLI_KEY_CTRL_I      0x09
#define CLI_KEY_CTRL_K      0x0B
#define CLI_KEY_CTRL_L      0x0C
#define CLI_KEY_CTRL_N      0x0E
#define CLI_KEY_CTRL_P      0x10
#define CLI_KEY_CTRL_U      0x15
#define CLI_KEY_CTRL_W      0x17
#define CLI_KEY_CTRL_Z      0x1A

#define CLI_CRLF            "\r\n"
#define CLI_ERASE_IN_LINE   "\x1B[K"

/* Escape Sequence States */
#define CLI_ESC_STATE_NONE      (0)
#define CLI_ESC_STATE_ESC       (1)
#define CLI_ESC_STATE_BRACKET   (2)

#define CLI_USERNAME_PROMPT                 "Username: "
#define CLI_PASSWORD_PROMPT                 "Password: "
#define CLI_PASSWORD_MASK                   '*'
#define CLI_USER_MODE_PROMPT                ">"
#define CLI_PRIVILEGED_MODE_PROMPT          "#"
#define CLI_GLOBAL_CONFIG_MODE_PROMPT       "(config)#"
#define CLI_INTERFACE_CONFIG_MODE_PROMPT    "(config-if)#"
#ifdef CLI_CFG_LINE_MODE
#define CLI_LINE_CONFIG_MODE_PROMPT         "(config-line)#"
#endif
#define CLI_LOG_OUT_STRING                  "Log out\n"

/* Error Messages */
#define CLI_ERR_INCOMPLETE_CMD              "%% Incomplete command.\n"
#define CLI_ERR_UNKNOWN_CMD                 "%% Unknown command.\n"
#define CLI_ERR_INVALID_INPUT               "%% Invalid input.\n"
#define CLI_ERR_INVALID_INPUT_DETECTED      "%% Invalid input detected.\n"
#define CLI_ERR_CMD_NO_SUPPORT_NO           "%% Command does not support 'no'.\n"
#define CLI_ERR_AMBIGUOUS_CMD_FMT           "%% Ambiguous command:  \"%s\"\n"
#define CLI_ERR_INVALID_INPUT_AT_MARKER     "%% Invalid input detected at '^' marker.\n"
#define CLI_ERR_INVALID_IPV4_ADDR           "%% Invalid IPv4 address.\n"
#define CLI_ERR_INVALID_IPV4_SUBNET_MASK    "%% Invalid IPv4 subnet mask.\n"
#define CLI_ERR_INVALID_IPV4_GATEWAY        "%% Invalid IPv4 gateway address.\n"
#define CLI_ERR_AUTH_FAILED                 "%% Authentication failed.\n"
#define CLI_ERR_BAD_PASSWORD                "%% Bad password\n"
#define CLI_ERR_HOSTNAME_TOO_LONG_FMT       "%% Invalid input: Hostname too long (Max %d characters).\n"
#define CLI_ERR_INVALID_PRIVILEGE           "%% Invalid privilege level.\n"
#define CLI_ERR_MAX_USERS                   "%% Max users reached.\n"
#define CLI_ERR_INVALID_VALUE_FMT           "%% Invalid value (0-%d).\n"
#define CLI_ERR_NO_STARTUP_CONFIG           "%% startup-config is not present.\n"
#define CLI_ERR_INVALID_HOSTNAME            "%% Hostnames are limited to letters, digits, and hyphens; however, they cannot start or end with a hyphen.\n"
#define CLI_ERR_LOOP_STATE_PORT             "%% Ports in loop state are not allowed to be configured.\n"

extern I32_T outbyte(I32_T c);

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef UI32_T(*CLI_CMD_HANDLER_FUNC_T)(C8_T *ptr_out_buf, UI32_T out_buf_len, BOOL_T no_flag, I32_T argc, C8_T **pptr_argv, I32_T token_idx);

/* CLI Modes */
typedef enum
{
    CLI_MODE_USER = 0,          /* > */
    CLI_MODE_PRIVILEGED,        /* # */
    CLI_MODE_CONFIG,            /* (config)# */
    CLI_MODE_CONFIG_IF,         /* (config-if)# */
    CLI_MODE_CONFIG_VLAN,       /* (config-vlan)# */
#ifdef CLI_CFG_LINE_MODE
    CLI_MODE_CONFIG_LINE,       /* (config-line)# */
#endif
    CLI_MODE_MAX
} CLI_MODE_T;

/* Filtering Types */
typedef enum
{
    FILTER_NONE = 0,
    FILTER_INCLUDE,
    FILTER_EXCLUDE,
    FILTER_BEGIN,
    FILTER_SECTION
} CLI_FILTER_TYPE_T;

typedef struct CLI_DYN_LABEL_ENTRY_S
{
    const C8_T *ptr_original_name;
    C8_T *ptr_ram_buffer;
    UI32_T buffer_len;
} CLI_DYN_LABEL_ENTRY_T;

/* Queue Item Structure */
typedef struct CLI_CMD_INPUT_S
{
    UI8_T magic_num;
    UI8_T msg_id;
    UI8_T session_id; /* 0 for Console, >0 for Telnet/SSH */
    UI8_T c;
} CLI_CMD_INPUT_T;

typedef struct CLI_SESSION_DEREGISTER_MSG_S
{
    UI8_T msg_id;
    UI8_T session_id; /* 0 for Console, >0 for Telnet/SSH */
} CLI_SESSION_DEREGISTER_MSG_T;

/* Command Node */
typedef struct CLI_CMD_NODE_S
{
    const C8_T *ptr_parent_path;    /* Space separated path, e.g., "show ip". NULL for root */
    const C8_T *ptr_name;           /* Keyword or Param Name */
    const C8_T *ptr_help;
    CLI_CMD_HANDLER_FUNC_T ptr_handler;
    CLI_SHOW_CFG_HANDLER ptr_show_handler;     /* For show running/startup config */
    CLI_MODE_T mode;
    BOOL_T is_param;            /* TRUE for parameters, FALSE for keywords */
    BOOL_T no_supported;        /* Whether this node supports 'no' prefix */
    I32_T token_idx;            /* Override token index for handler if non-zero */
} CLI_CMD_NODE_T;

#ifdef CLI_CFG_HISTORY_CMD
/* History */
typedef struct CLI_HISTORY_ENTRY_S
{
    C8_T *ptr_buffer;
    UI16_T len;
} CLI_HISTORY_ENTRY_T;
#endif

/* User Database */
typedef struct USER_CONFIG_S
{
#ifdef CLI_CFG_AAA
    C8_T username[MAX_USER_NAME_SIZE];
    C8_T password[MAX_PASSWORD_SIZE]; /* Plain text or 'secret' hash storage */
    I32_T privilege;     /* 0-15 */
    BOOL_T is_secret;     /* 1 if password field is secret, 0 if plain */
#endif
    BOOL_T active;
} USER_CONFIG_T;

typedef struct SYS_CONFIG_S
{
    C8_T hostname[MAX_SYS_NAME_SIZE];
    I32_T term_length;

    /* AAA & Security */
#ifdef CLI_CFG_AAA
    C8_T enable_password[MAX_PASSWORD_SIZE];
    C8_T enable_secret[MAX_PASSWORD_SIZE];
#endif
    USER_CONFIG_T users[CLI_MAX_USERS];
} SYS_CONFIG_T;

/* Main Context */
typedef struct CLI_CONTEXT_S
{
    CLI_MODE_T current_mode;

    /* Auth Context */
    I32_T auth_state;
    C8_T input_password[MAX_PASSWORD_SIZE];
    I32_T auth_pos;
    C8_T input_username[MAX_USER_NAME_SIZE];

    /* Config Context */
    I32_T current_if_idx;
    I32_T current_vlan_id;

    /* Line Editing */
    C8_T line_buf[CLI_INPUT_MAX_LEN];
    UI16_T pos;       /* Current cursor position */
    UI16_T len;       /* Current line length */

    /* History */
#ifdef CLI_CFG_HISTORY_CMD
    CLI_HISTORY_ENTRY_T history[CLI_HISTORY_SIZE];
    UI8_T hist_head;
    UI8_T hist_tail;
    UI8_T hist_idx;   /* Current navigation index */
#endif

    /* VT100 State */
    UI8_T escape_seq_state;

    /* Output Control */
    CLI_FILTER_TYPE_T filter_type;
    C8_T filter_text[CLI_FILTER_CXT_SIZE];
    BOOL_T filter_triggered; /* For 'begin' */
    I32_T consecutive_tabs; /* Track consecutive TAB presses */

    /* Output Buffer */
    C8_T *output_buffer;
    UI32_T output_buffer_size;
    UI32_T output_pos;

    /* Output redirection for multi-user (Telnet/SSH) */
    void (*ptr_out_fn)(UI8_T session, C8_T *ptr_buf, UI32_T len);

} CLI_CONTEXT_T;

typedef struct CLI_DATA_S
{
    threadhandle_t cli_task_handle;
    StackType_t cli_task_stack[CLI_TASK_STACK_SIZE];
    StaticTask_t cli_task_tcb;
    msghandle_t cli_queue_handle;

    /* Session Management */
    CLI_CONTEXT_T *ptr_sessions[CLI_MAX_SESSIONS];
    I32_T current_session_id;
    CLI_CONTEXT_T *ptr_current_ctx; /* Pointer to active context */

    /* System Configuration */
    SYS_CONFIG_T conf;

} CLI_DATA_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
CLI_DATA_T *
cli_get_info(
    void);

#endif
#endif /* CLI_INTERNAL_H */