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

/* FILE NAME:   cli.c
 * PURPOSE:
 *      Implement the basic functions of a command line interface.
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdarg.h>
#include "cli_internal.h"
#include "cli_db.h"
#include "db_api.h"
#include "mw_cmd_util.h"
#include "mw_cmd_parser.h"
#include "osapi_message.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "osapi_mutex.h"
#include "web.h"
#include "air_chipscu.h"
#include "mw_account.h"
#ifdef AIR_SUPPORT_LP
#include "cli_lp.h"
#endif
#include "cli_cmd_ipsetting.h"
#include "lwip/opt.h"
#ifdef AIR_SUPPORT_HTTPD_MUTEX
#include "httpd_util.h"
#endif
#include "cli_port.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define G_CLI (*(_ptr_cli->ptr_current_ctx))
#define CLI_DEBUG 0

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef void(*uart_input_callback)(char c);

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
extern void uart_register_input_cb(uart_input_callback cb);

static void cli_uart_input_cb(char c)
{
    cli_input(CLI_CONSOLE_SESSION_ID, c);
}

static void
_cli_flush_output(
    void);

static BOOL_T
_cli_match_node(
    const CLI_CMD_NODE_T *ptr_node,
    const C8_T *ptr_token,
    BOOL_T *ptr_is_partial);

static void
_cli_putc(
    C8_T c);

static void
_cli_puts(
    const C8_T *ptr_s);

static void
_cli_redraw_line(
    void);

static BOOL_T
_cli_check_filter(
    const C8_T *ptr_line);

static BOOL_T
_cli_validate_hostname(
    C8_T *ptr_hostname,
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    UI32_T *ptr_len);

#ifdef CLI_CFG_USER_MODE
static UI32_T
_cli_cmd_privilegedMode_enable(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

static UI32_T
_cli_cmd_privilegedMode_disable(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_set_conf_term(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

#ifdef CLI_CFG_AAA
static UI32_T
_cli_cmd_set_enable_conf(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

static UI32_T
_cli_cmd_mode_exit(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_privilegedMode_end(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_show_run(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_show_config(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx,
    I32_T conf_mode_flag);

#ifdef CLI_CFG_HISTORY_CMD
static UI32_T
_cli_cmd_show_history(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

static UI32_T
_cli_cmd_show_version(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

#if CLI_DEBUG
static UI32_T
_cli_cmd_show_task_stats(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_show_run_time_stats(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

static UI32_T
_cli_cmd_copyRunningToStartup(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_system_reload(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_show_startup(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_set_hostname(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_show_hostname(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode);

#ifdef CLI_CFG_LINE_MODE
static UI32_T
_cli_cmd_set_line_mode(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

#ifdef CLI_CFG_AAA
static UI32_T
_cli_cmd_set_username(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

#ifdef CLI_CFG_TERMINAL_LENGTH
static UI32_T
_cli_cmd_set_term_length(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);
#endif

#ifdef CLI_CFG_HISTORY_CMD
static void
_cli_free_history(
    CLI_CONTEXT_T *ptr_ctx);

static void
_cli_add_to_history(
    void);
#endif

static MW_ERROR_NO_T
_cli_find_command_node(
    I32_T argc,
    C8_T **pptr_argv,
    I32_T start_idx,
    BOOL_T is_do_cmd,
    BOOL_T is_no_cmd,
    const CLI_CMD_NODE_T **pptr_ret_node,
    I32_T *ptr_ret_depth);

static void
_cli_execute_command(
    void);

static void
_cli_parse_pipe(
    C8_T *ptr_cmd_line);

static void
_cli_complete_tab(
    void);

static BOOL_T
_cli_has_children(
    const CLI_CMD_NODE_T *ptr_node);

static void
_cli_show_help(
    void);

static void
_cli_task(
    void *ptr_param);

static void
_cli_handle_auth(
    C8_T c);

static void
_cli_handle_escape(
    C8_T c);

static void
_cli_handle_normal(
    C8_T c);

static void
_cli_handle_cmd_input_msg(
    I32_T session_id,
    C8_T c);

static UI32_T
_cli_cmd_if_enterMode(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_if_setDuplex(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_if_setEEE(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_if_setFlowcontrol(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_if_setSpeed(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

static UI32_T
_cli_cmd_if_setAdminState(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);


/* STATIC VARIABLE DECLARATIONS
 */
static CLI_DATA_T *_ptr_cli = NULL;
/* RAM buffer for dynamic command names (e.g., interface port range) */
static C8_T _cli_dyn_port_label[CLI_DYN_LABEL_MAX_LEN] = CLI_PORT_LIST_LABEL;
static const CLI_DYN_LABEL_ENTRY_T _cli_dyn_labels[] = {
    { CLI_PORT_LIST_LABEL, _cli_dyn_port_label, CLI_DYN_LABEL_MAX_LEN }
};

/* The Command Tree Definition */
static const CLI_CMD_NODE_T _cli_cmd_tree[] = {
#ifdef CLI_CFG_USER_MODE
    /* Root Commands (User Mode) */
    { NULL, "enable", "Turn on privileged commands", _cli_cmd_privilegedMode_enable, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
    { NULL, "exit", "Exit current mode", _cli_cmd_mode_exit, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#endif

    { NULL, "show", "Show running system information", NULL, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#ifdef CLI_CFG_TERMINAL_LENGTH
    { NULL, "terminal", "Set terminal line parameters", NULL, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#endif

    /* Show Subcommands */
    { "show", "running-config", "Current operating configuration", _cli_cmd_show_run, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { "show", "startup-config", "Contents of startup configuration", _cli_cmd_show_startup, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
#ifdef CLI_CFG_HISTORY_CMD
    { "show", "history", "Display the session command history", _cli_cmd_show_history, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#endif
    { "show", "version", "System hardware and software status", _cli_cmd_show_version, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#ifdef AIR_SUPPORT_LP
    { "show", "loopback-detection", "Show Loopback Detection status", cli_cmd_lp_showState, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#endif

    /* Terminal Commands */
#ifdef CLI_CFG_TERMINAL_LENGTH
    { "terminal", "length", "Set number of lines on a screen", _cli_cmd_set_term_length, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
#endif

    /* Privileged Mode Root */
    { NULL, "configure", "Enter configuration mode", NULL, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { "configure", "terminal", "Configure from the terminal", _cli_cmd_set_conf_term, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { NULL, "disable", "Turn off privileged commands", _cli_cmd_privilegedMode_disable, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    /* Note: exit matches previous def, usually handled by walking logic, but we can register explicitly for help text if needed */
    { NULL, "exit", "Exit current mode", _cli_cmd_mode_exit, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { NULL, "reload", "Halt and perform a cold restart", _cli_cmd_system_reload, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { NULL, "copy", "Copy configuration or image data", NULL, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0},
    /* Copy Subcommands */
    { "copy", "running-config", "Copy from current system configuration", NULL, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0},
    { "copy running-config", "startup-config", "Copy to startup configuration", _cli_cmd_copyRunningToStartup, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0},

#if CLI_DEBUG
    { NULL, "task-stats", "Displays task statistics", _cli_cmd_show_task_stats, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
    { NULL, "run-time-stats", "Displays run time statistics", _cli_cmd_show_run_time_stats, NULL, CLI_MODE_PRIVILEGED, FALSE, FALSE, 0 },
#endif

    /* Global Config Mode */
#ifdef CLI_CFG_AAA
    { NULL, "enable", "Modify enable password parameters", _cli_cmd_set_enable_conf, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
#endif
    { NULL, "exit", "Exit current mode", _cli_cmd_mode_exit, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { NULL, "hostname", "Set device's host name", _cli_cmd_set_hostname, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "hostname", "WORD", "This device's host name", _cli_cmd_set_hostname, _cli_cmd_show_hostname, CLI_MODE_CONFIG, TRUE, FALSE, 0 },
#ifdef CLI_CFG_LINE_MODE
    { NULL, "line", "Configure a terminal line", _cli_cmd_set_line_mode, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
#endif
    { NULL, "end", "Exit to enable mode", _cli_cmd_privilegedMode_end, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },

#ifdef CLI_CFG_AAA
    { NULL, "username", "Establish User Name Authentication", NULL, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "username", "WORD", "User name", NULL, NULL, CLI_MODE_CONFIG, TRUE, TRUE, 0 },
    { "username WORD", "password", "Specify the password for the user", NULL, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "username WORD password", "WORD", "The user password", _cli_cmd_set_username, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },

    { "username WORD", "secret", "Specify the secret for the user", NULL, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "username WORD secret", "WORD", "The user secret", _cli_cmd_set_username, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },

    { "username WORD", "privilege", "Set user privilege level", NULL, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "username WORD privilege", "<0-15>", "Privilege level", NULL, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },

    /* username <name> privilege <lvl> ... */
    { "username WORD privilege <0-15>", "password", "Specify the password for the user", NULL, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "username WORD privilege <0-15> password", "WORD", "The user password", _cli_cmd_set_username, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },

    { "username WORD privilege <0-15>", "secret", "Specify the secret for the user", NULL, NULL, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "username WORD privilege <0-15> secret", "WORD", "The user secret", _cli_cmd_set_username, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },
#endif

    /* Config Management IPv4 */
    { NULL, "management", "IP management", NULL, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "management", "vlan", "VLAN commands", NULL, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "management vlan", "ip-address", "IP address", cli_cmd_ipSetting_setIp, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "management vlan ip-address", "A.B.C.D", "IP Address format is A.B.C.D where (A/B/C/D = 0 ~ 255)", NULL, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },
    { "management vlan ip-address A.B.C.D", "mask", "IP subnet mask", NULL, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "management vlan ip-address A.B.C.D mask", "A.B.C.D", "IP subnet mask", cli_cmd_ipSetting_setIp, cli_cmd_ipSetting_showIp, CLI_MODE_CONFIG, TRUE, TRUE, 0 },

    { NULL, "ip", "Global IP configuration commands", NULL, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "ip", "default-gateway", "Specify default gateway", cli_cmd_ipSetting_SetGw, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "ip default-gateway", "A.B.C.D", "IP address of default gateway", cli_cmd_ipSetting_SetGw, cli_cmd_ipSetting_showGw, CLI_MODE_CONFIG, TRUE, FALSE, 0 },
    /* Config Management IPv6 */

    /* Config Account Settings */

    /* Config Vlan */

    /* Config Security (Storm) */

    /* Config Security (Traffic) */

    /* Config Security (Mac Manage) */

    /* Config Security (DHCP Snooping) */

    /* Config Loop Prevention */
#ifdef AIR_SUPPORT_LP
    { NULL, "loopback-detection", "Loopback detection configuration", NULL, cli_cmd_lp_showCfg, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
    { "loopback-detection", "enable", "Enable loopback detection", cli_cmd_lp_setLoopPreventionState, NULL, CLI_MODE_CONFIG, FALSE, TRUE, 0 },
#endif

    /* Config LLDP */

    /* Config QoS */

    /* Config Interface */
    { NULL, "interface", "Select an interface to configure", NULL, cli_cmd_if_showCfg, CLI_MODE_CONFIG, FALSE, FALSE, 0 },
    { "interface", _cli_dyn_port_label, "Port number", _cli_cmd_if_enterMode, NULL, CLI_MODE_CONFIG, TRUE, FALSE, 0 },

    /* Config Duplex */
    { NULL, "duplex", "Duplex configuration", _cli_cmd_if_setDuplex, NULL, CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },
    { "duplex", "auto", "Auto duplex configuration", _cli_cmd_if_setDuplex, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "duplex", "full", "Force full duplex configuration", _cli_cmd_if_setDuplex, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "duplex", "half", "Force half duplex configuration", _cli_cmd_if_setDuplex, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },

    /* Config Interface EEE */
    { NULL, "eee", "EEE configuration", _cli_cmd_if_setEEE, NULL, CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },
    { "eee", "enable", "Enable EEE", _cli_cmd_if_setEEE, NULL,  CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },

    { NULL, "exit", "Exit current mode", _cli_cmd_mode_exit, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },

    /* Config Interface Flow-control */
    { NULL, "flow-control", "Flow-control configuration", _cli_cmd_if_setFlowcontrol, NULL, CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },
    { "flow-control", "auto", "Auto flow-control configuration", _cli_cmd_if_setFlowcontrol, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "flow-control", "off", "Force disabled flow-control configuration", _cli_cmd_if_setFlowcontrol, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "flow-control", "on", "Force enabled flow-control configuration", _cli_cmd_if_setFlowcontrol, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },

    /* Config Interface Shutdown */
    { NULL, "shutdown", "Disable the admin state of a port", _cli_cmd_if_setAdminState, NULL, CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },

    /* Config Interface Speed */
    { NULL, "speed", "Speed configuration", _cli_cmd_if_setSpeed, NULL, CLI_MODE_CONFIG_IF, FALSE, TRUE, 0 },
    { "speed", "10", "Force 10 Mbps configuration", _cli_cmd_if_setSpeed, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "speed", "100", "Force 100 Mbps configuration", _cli_cmd_if_setSpeed, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },
    { "speed", "auto", "Auto speed configuration", _cli_cmd_if_setSpeed, NULL, CLI_MODE_CONFIG_IF, FALSE, FALSE, 0 },

    /* Show Interface Subcommands */
    { "show", "interface", "Show the port status", NULL, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
    { "show interface", "status", "Show port link status", NULL, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
    { "show interface status", "all", "All Ports", cli_cmd_if_showStatus, NULL, CLI_MODE_USER, FALSE, FALSE, 0 },
    { "show interface status", _cli_dyn_port_label, "Port number", cli_cmd_if_showStatus, NULL, CLI_MODE_USER, TRUE, FALSE, 0 },
    { "show interface", _cli_dyn_port_label, "Port number", cli_cmd_if_show, NULL, CLI_MODE_USER, TRUE, FALSE, 0 },

};

/* LOCAL SUBPROGRAM BODIES
 */
static void
_cli_putc(
    C8_T c)
{
    /* Auto-convert \n to \r\n */
    if (CLI_KEY_NEWLINE == c)
    {
        _cli_putc(CLI_KEY_ENTER);
    }

    /* Always store in Fixed Output Buffer */
    if (G_CLI.output_pos < G_CLI.output_buffer_size - 1)
    {
        G_CLI.output_buffer[G_CLI.output_pos++] = c;
        G_CLI.output_buffer[G_CLI.output_pos] = 0; /* Null terminate */
    }
}

static void
_cli_puts(
    const C8_T *ptr_s)
{
    while (*ptr_s)
    {
        _cli_putc(*ptr_s++);
    }
}

static void
_cli_flush_output(
    void)
{
    I32_T i;
    C8_T *ptr_rsp = NULL;
    MW_ERROR_NO_T rc;
    UI32_T read_idx = 0;
    UI32_T write_idx = 0;
    UI32_T line_start = 0;
    BOOL_T keep_line = FALSE;
    UI32_T line_len = 0;
    UI32_T expanded_len = 0;
    UI32_T dest_idx = 0;

    if (0 == G_CLI.output_pos)
    {
        return;
    }

    /* Filter Output if pipe is active */
    if (FILTER_NONE != G_CLI.filter_type)
    {
        /* Ensure null termination for string operations */
        G_CLI.output_buffer[G_CLI.output_pos] = CLI_KEY_NUL;

        for (read_idx = 0; read_idx < G_CLI.output_pos; read_idx++)
        {
            if (CLI_KEY_NEWLINE == G_CLI.output_buffer[read_idx])
            {
                /* Temporarily null-terminate the line to check filter */
                G_CLI.output_buffer[read_idx] = CLI_KEY_NUL;

                keep_line = _cli_check_filter(&G_CLI.output_buffer[line_start]);

                /* Restore newline */
                G_CLI.output_buffer[read_idx] = CLI_KEY_NEWLINE;

                if (TRUE == keep_line)
                {
                    line_len = read_idx - line_start + 1; /* Include \n */
                    if (write_idx != line_start)
                    {
                        osapi_memmove(&G_CLI.output_buffer[write_idx], &G_CLI.output_buffer[line_start], line_len);
                    }
                    write_idx += line_len;
                }
                line_start = read_idx + 1;
            }
        }

        /* Handle remaining data (e.g. prompt or partial line) - Always keep prompts */
        if (line_start < G_CLI.output_pos)
        {
            line_len = G_CLI.output_pos - line_start;
            if (write_idx != line_start)
            {
                osapi_memmove(&G_CLI.output_buffer[write_idx], &G_CLI.output_buffer[line_start], line_len);
            }
            write_idx += line_len;
        }

        /* Update buffer position to filtered length */
        G_CLI.output_pos = write_idx;
        G_CLI.output_buffer[G_CLI.output_pos] = CLI_KEY_NUL;
    }

    if (CLI_CONSOLE_SESSION_ID == _ptr_cli->current_session_id)
    {
        /* Console: Direct UART output */
        for (i = 0; i < G_CLI.output_pos; i++)
        {
            if ((CLI_KEY_NEWLINE == G_CLI.output_buffer[i]) &&
                ((0 == i) || (CLI_KEY_ENTER != G_CLI.output_buffer[i - 1])))
            {
                outbyte(CLI_KEY_ENTER);
            }
            outbyte(G_CLI.output_buffer[i]);
        }
    }
    else
    {
        if (NULL != G_CLI.ptr_out_fn)
        {
            /* Calculate required size with CR expansion */
            expanded_len = 0;
            for (i = 0; i < G_CLI.output_pos; i++)
            {
                if ((CLI_KEY_NEWLINE == G_CLI.output_buffer[i]) &&
                    ((0 == i) || (CLI_KEY_ENTER != G_CLI.output_buffer[i - 1])))
                {
                    expanded_len++;
                }
                expanded_len++;
            }

            /* Telnet/SSH: Allocate heap based on length */
            rc = osapi_calloc(expanded_len + 1, CLI_TASK_NAME, (void **)&ptr_rsp);
            if (MW_E_OK == rc)
            {
                /* Copy string with expansion */
                dest_idx = 0;
                for (i = 0; i < G_CLI.output_pos; i++)
                {
                    if ((CLI_KEY_NEWLINE == G_CLI.output_buffer[i]) &&
                        ((0 == i) || (CLI_KEY_ENTER != G_CLI.output_buffer[i - 1])))
                    {
                        ptr_rsp[dest_idx++] = CLI_KEY_ENTER;
                    }
                    ptr_rsp[dest_idx++] = G_CLI.output_buffer[i];
                }
                ptr_rsp[dest_idx] = CLI_KEY_NUL;

                /* Invoke Telnet/SSH callback (Send & Free handled by callback/receiver) */
                G_CLI.ptr_out_fn((UI8_T)_ptr_cli->current_session_id, ptr_rsp, dest_idx);
            }
            else
            {
                CLI_LOG_ERROR("Failed to allocate heap for output flush");
            }
        }
    }

    /* Reset buffer pos */
    G_CLI.output_pos = 0;
}

static void
_cli_redraw_line(
    void)
{
    I32_T i;

    if (CLI_AUTH_STATE_WAIT_USERNAME == G_CLI.auth_state)
    {
        _cli_putc(CLI_KEY_ENTER);
        _cli_puts(CLI_USERNAME_PROMPT);
        for (i = 0; i < G_CLI.auth_pos; i++)
        {
            _cli_putc(G_CLI.input_password[i]);
        }
        return;
    }
    if ((CLI_AUTH_STATE_WAIT_PASSWORD == G_CLI.auth_state) || (CLI_AUTH_STATE_ENABLE == G_CLI.auth_state))
    {
        _cli_putc(CLI_KEY_ENTER);
        _cli_puts(CLI_PASSWORD_PROMPT);
        for (i = 0; i < G_CLI.auth_pos; i++)
        {
            _cli_putc(CLI_PASSWORD_MASK);
        }
        return;
    }

    if (CLI_AUTH_STATE_RELOAD_CONFIRM == G_CLI.auth_state)
    {
        /* Wait for confirmation key, do not draw prompt */
        return;
    }

    /* Clear line from cursor to left */
    _cli_putc(CLI_KEY_ENTER);

    /* Print prompt */
    switch (G_CLI.current_mode)
    {
#ifdef CLI_CFG_USER_MODE
        case CLI_MODE_USER:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_USER_MODE_PROMPT);
            break;
#endif
        case CLI_MODE_PRIVILEGED:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_PRIVILEGED_MODE_PROMPT);
            break;
        case CLI_MODE_CONFIG:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_GLOBAL_CONFIG_MODE_PROMPT);
            break;
        case CLI_MODE_CONFIG_IF:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_INTERFACE_CONFIG_MODE_PROMPT);
            break;
#ifdef CLI_CFG_LINE_MODE
        case CLI_MODE_CONFIG_LINE:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_LINE_CONFIG_MODE_PROMPT);
            break;
#endif
        default:
            _cli_puts(_ptr_cli->conf.hostname);
            _cli_puts(CLI_PRIVILEGED_MODE_PROMPT);
            break;
    }

    /* Print buffer */
    for (i = 0; i < G_CLI.len; i++)
    {
        _cli_putc(G_CLI.line_buf[i]);
    }

    /* Clear to end of line (VT100: Esc[K) */
    _cli_puts(CLI_ERASE_IN_LINE);

    /* If in auth mode, do not print buffer or move cursor back visibly in a way that reveals length */
    if (CLI_AUTH_STATE_SUCCESS != G_CLI.auth_state)
    {
        if (CLI_AUTH_STATE_WAIT_USERNAME == G_CLI.auth_state)
        {
            /* Echo input for username */
            /* Cursor logic is complex here, simplified to just return as user types at end */
            return;
        }
        return;
    }

    /* Move cursor back to pos if needed */
    if (G_CLI.pos < G_CLI.len)
    {
        for (i = 0; i < (G_CLI.len - G_CLI.pos); i++)
        {
            _cli_putc(CLI_KEY_BACKSPACE);
        }
    }
}

static BOOL_T
_cli_check_filter(
    const C8_T *ptr_line)
{
    switch (G_CLI.filter_type)
    {
        case FILTER_NONE:
            return TRUE;
        case FILTER_INCLUDE:
            return (osapi_strstr(ptr_line, G_CLI.filter_text) != NULL);
        case FILTER_EXCLUDE:
            return (osapi_strstr(ptr_line, G_CLI.filter_text) == NULL);
        case FILTER_BEGIN:
            if (TRUE == G_CLI.filter_triggered)
            {
                return TRUE;
            }
            if (osapi_strstr(ptr_line, G_CLI.filter_text) != NULL)
            {
                G_CLI.filter_triggered = TRUE;
                return TRUE;
            }
            return FALSE;
        case FILTER_SECTION:
        {
            BOOL_T match = (osapi_strstr(ptr_line, G_CLI.filter_text) != NULL);
            BOOL_T is_indented = ((ptr_line[0] == ' ') || (ptr_line[0] == '\t'));

            if (TRUE == match)
            {
                G_CLI.filter_triggered = TRUE;
                return TRUE;
            }
            if (TRUE == G_CLI.filter_triggered)
            {
                if (TRUE == is_indented)
                {
                    return TRUE;
                }
                else
                {
                    G_CLI.filter_triggered = FALSE;
                    return FALSE;
                }
            }
            return FALSE;
        }
        default:
            return FALSE;
    }
}


/* -------------------------------------------------------------------------- */
/* Handlers */
/* -------------------------------------------------------------------------- */

#ifdef CLI_CFG_USER_MODE
static UI32_T
_cli_cmd_privilegedMode_enable(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
    /* Level 1 to 15 transition */
    if (G_CLI.current_mode >= CLI_MODE_PRIVILEGED)
    {
        return len;
    }

    /* Check if secret or password is set */
    if ((osapi_strlen(_ptr_cli->conf.enable_secret) > 0) || (osapi_strlen(_ptr_cli->conf.enable_password) > 0))
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, "%s", CLI_PASSWORD_PROMPT);
        G_CLI.auth_state = CLI_AUTH_STATE_ENABLE;
        G_CLI.auth_pos = 0;
        G_CLI.input_password[0] = 0;
    }
    else
    {
        /* No password set, allow entry */
        G_CLI.current_mode = CLI_MODE_PRIVILEGED;
    }
    return len;
}
#endif

static UI32_T
_cli_cmd_privilegedMode_disable(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
#ifndef CLI_CFG_USER_MODE
    /* Return to Login state (Logout) */
    len = osapi_snprintf(ptr_out_buf, out_buf_len, "%s", CLI_LOG_OUT_STRING);
    G_CLI.auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
    G_CLI.auth_pos = 0;
    G_CLI.input_password[0] = 0;
#else
    G_CLI.current_mode = CLI_MODE_USER;
#endif
    return len;
}
static UI32_T
_cli_cmd_set_conf_term(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    G_CLI.current_mode = CLI_MODE_CONFIG;
    return 0;
}

#ifdef CLI_CFG_AAA
static UI32_T
_cli_cmd_set_enable_conf(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
    /* enable password <pwd> OR enable secret <pwd> */
    if (NULL == pptr_argv[token_idx + 2])
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    if (0 == osapi_strcmp(pptr_argv[token_idx + 1], "password"))
    {
        osapi_strncpy(_ptr_cli->conf.enable_password, pptr_argv[token_idx + 2], MAX_PASSWORD_SIZE - 1);
    }
    else if (0 == osapi_strcmp(pptr_argv[token_idx + 1], "secret"))
    {
        osapi_strncpy(_ptr_cli->conf.enable_secret, pptr_argv[token_idx + 2], MAX_PASSWORD_SIZE - 1);
    }
    else
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_UNKNOWN_CMD);
    }
    return len;
}
#endif

static UI32_T
_cli_cmd_mode_exit(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
    switch (G_CLI.current_mode)
    {
#ifdef CLI_CFG_USER_MODE
        case CLI_MODE_USER:
            /* Return to Login state (Logout) */
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%s", CLI_LOG_OUT_STRING);
            G_CLI.auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
            G_CLI.auth_pos = 0;
            G_CLI.input_password[0] = 0;
            break;
#endif
        case CLI_MODE_PRIVILEGED:
#ifndef CLI_CFG_USER_MODE
            /* Return to Login state (Logout) */
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%s", CLI_LOG_OUT_STRING);
            G_CLI.auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
            G_CLI.auth_pos = 0;
            G_CLI.input_password[0] = 0;
#else
            G_CLI.current_mode = CLI_MODE_USER;
#endif
            break;
        case CLI_MODE_CONFIG:
            G_CLI.current_mode = CLI_MODE_PRIVILEGED;
            break;
        case CLI_MODE_CONFIG_IF:
#ifdef CLI_CFG_LINE_MODE
        case CLI_MODE_CONFIG_LINE:
#endif
            /* Return to Global Config Mode */
            G_CLI.current_mode = CLI_MODE_CONFIG;
            break;
        default:
            /* Default fallback to config if deeper */
            G_CLI.current_mode = CLI_MODE_CONFIG;
            break;
    }
    return len;
}

static UI32_T
_cli_cmd_privilegedMode_end(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    if (G_CLI.current_mode > CLI_MODE_PRIVILEGED)
    {
        G_CLI.current_mode = CLI_MODE_PRIVILEGED;
    }
    return 0;
}

/* Show Run Logic */
static UI32_T
_cli_cmd_show_run(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    return _cli_cmd_show_config(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, CLI_SHOW_RUNNING_CONFIG);
}

static UI32_T
_cli_cmd_show_config(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx,
    I32_T show_mode)
{
    I32_T i;
    UI32_T total_len = 0;
    UI32_T str_len = 0;
#ifdef CLI_CFG_AAA
    C8_T priv_str[16];
#endif
    I32_T node_count;
    const CLI_CMD_NODE_T *ptr_node;
    C8_T *ptr_ver = NULL;

    if ((MW_E_OK != dbapi_isStartUpConfigExist()) && (CLI_SHOW_STARTUP_CONFIG == show_mode))
    {
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, CLI_ERR_NO_STARTUP_CONFIG);
        return total_len;
    }

    if (total_len < out_buf_len)
    {
        if (MW_E_OK != osapi_calloc(MAX_VERSION_SIZE, CLI_TASK_NAME, (void **)&ptr_ver))
        {
            CLI_LOG_ERROR("Failed to allocate memory for version string");
            return 0;
        }
        if (MW_E_OK != mw_get_version(ptr_ver))
        {
            CLI_LOG_ERROR("Failed to get version string");
            MW_FREE(ptr_ver);
            return 0;
        }
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "%s\n", ptr_ver);
        MW_FREE(ptr_ver);

        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "CLI v1.0\n");
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "@\n");
    }

#ifdef CLI_CFG_AAA
    if (osapi_strlen(_ptr_cli->conf.enable_secret) > 0)
    {
        if (total_len < out_buf_len)
        {
            total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "enable secret %s\n", _ptr_cli->conf.enable_secret);
        }
    }
    else if (osapi_strlen(_ptr_cli->conf.enable_password) > 0)
    {
        if (total_len < out_buf_len)
        {
            total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "enable password %s\n", _ptr_cli->conf.enable_password);
        }
    }
#endif /* CLI_CFG_AAA */
    if (total_len < out_buf_len)
    {
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "!\n");
    }

#ifdef CLI_CFG_AAA
    for (i = 0; i < CLI_MAX_USERS; i++)
    {
        if (TRUE == _ptr_cli->conf.users[i].active)
        {
            priv_str[0] = '\0';
            if (_ptr_cli->conf.users[i].privilege != 1)
            {
                if (total_len < out_buf_len)
                {
                    total_len += osapi_snprintf(priv_str, sizeof(priv_str), " privilege %d", _ptr_cli->conf.users[i].privilege);
                }
            }
            if (total_len < out_buf_len)
            {
                total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "username %s%s %s 0 %s\n",
                                            _ptr_cli->conf.users[i].username,
                                            priv_str,
                                            _ptr_cli->conf.users[i].is_secret ? "secret" : "password",
                                            _ptr_cli->conf.users[i].password);
            }
        }
    }
#endif

    if (total_len < out_buf_len)
    {
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, "!\n");
    }

    node_count = (I32_T)(sizeof(_cli_cmd_tree) / sizeof(CLI_CMD_NODE_T));
    for (i = 0; (i < node_count) && (total_len < out_buf_len); i++)
    {
        ptr_node = &_cli_cmd_tree[i];

        if (NULL == ptr_node->ptr_show_handler)
        {
            continue;
        }

        str_len = ptr_node->ptr_show_handler(ptr_out_buf + total_len, out_buf_len - total_len, show_mode);
        total_len += str_len;
    }

    if ((total_len + sizeof(CLI_SHOW_CONFIG_END)) < out_buf_len)
    {
        total_len += osapi_snprintf(ptr_out_buf + total_len, out_buf_len - total_len, CLI_SHOW_CONFIG_END);
    }

    return total_len;
}

#ifdef CLI_CFG_HISTORY_CMD
static UI32_T
_cli_cmd_show_history(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    I32_T i;
    I32_T curr;
    UI32_T len = 0;

    for (i = 0; i < CLI_HISTORY_SIZE; i++)
    {
        curr = (G_CLI.hist_head + i) % CLI_HISTORY_SIZE;
        if (curr == G_CLI.hist_tail && ((i > 0) || (NULL == G_CLI.history[curr].ptr_buffer)))
        {
            break;
        }
        if (G_CLI.history[curr].len > 0 && G_CLI.history[curr].ptr_buffer)
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "  %s\n", G_CLI.history[curr].ptr_buffer);
        }
    }
    return len;
}
#endif

static UI32_T
_cli_cmd_show_version(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    C8_T *ptr_ver = NULL;
    UI32_T len = 0;

    if (MW_E_OK != osapi_calloc(MAX_VERSION_SIZE, CLI_TASK_NAME, (void **)&ptr_ver))
    {
        CLI_LOG_ERROR("Failed to allocate memory for version string");
        return 0;
    }
    if (MW_E_OK != mw_get_version(ptr_ver))
    {
        CLI_LOG_ERROR("Failed to get version string");
        MW_FREE(ptr_ver);
        return 0;
    }
    len = osapi_snprintf(ptr_out_buf, out_buf_len, "Version: %s\n", ptr_ver);
    MW_FREE(ptr_ver);
    return len;
}

#if CLI_DEBUG
static UI32_T
_cli_cmd_show_task_stats(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    extern void vTaskList(void);
    extern void xPortModuleMallocDumpInfo(void);
    extern size_t xPortGetHeapUsableBytes(void);
    extern size_t xPortGetFreeHeapSize(void);
    extern size_t xPortGetMinimumEverFreeHeapSize(void);

    UI32_T len = 0;
    vTaskList();

    xPortModuleMallocDumpInfo();

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "\ntotal heap size : %7d Bytes\n\n", (int)configTOTAL_HEAP_SIZE);
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "heap used size  : %7d Bytes\n",
        (int)(xPortGetHeapUsableBytes() - xPortGetFreeHeapSize()));

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "free heap size  : %7d Bytes (%d %%)\n",
        (int)xPortGetFreeHeapSize(), (int)((xPortGetFreeHeapSize() * 100) / xPortGetHeapUsableBytes()));
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "free heap size  : %7d Bytes (%d %%, Minimum ever)\n",
        (int)xPortGetMinimumEverFreeHeapSize(),
        (int)((xPortGetMinimumEverFreeHeapSize() * 100) / xPortGetHeapUsableBytes()));

#ifdef NON_FIXED_HEAP_SIZE_DBG
    extern uint32_t ucHeap_malloc_cnt(void);
    extern uint32_t ucHeap_free_cnt(void);
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "     malloc cnt : %7d\n", ucHeap_malloc_cnt());
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "       free cnt : %7d\n", ucHeap_free_cnt());
#endif
    return len;
}

static UI32_T
_cli_cmd_show_run_time_stats(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    airTaskGetRunTimeStats(1);
    return 0;
}
#endif

static UI32_T
_cli_cmd_copyRunningToStartup(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI8_T           save_runing = TRUE;
    UI32_T          len = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }

    _cli_puts("Building configuration...\n");
    _cli_flush_output();
    rc = cli_queue_setData(M_UPDATE, SYSTEM, SYS_SAVE_RUNNING, DB_ALL_ENTRIES, &save_runing, sizeof(save_runing));
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "[%s]\n", ((MW_E_OK != rc) ? "FAIL" : "OK"));
    return len;
}

static UI32_T
_cli_cmd_system_reload(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
    len = osapi_snprintf(ptr_out_buf, out_buf_len, "Proceed with reload? [confirm]\n");

    CLI_LOG_INFO("System reload initiated by User '%s' (Session %d)", G_CLI.input_username, _ptr_cli->current_session_id);

    /* Set state to wait for Enter key confirmation */
    G_CLI.auth_state = CLI_AUTH_STATE_RELOAD_CONFIRM;
    return len;
}

static UI32_T
_cli_cmd_show_startup(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    return _cli_cmd_show_config(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, CLI_SHOW_STARTUP_CONFIG);
}

/* -------------------------------------------------------------------------- */
/* Other Config Handlers */
/* -------------------------------------------------------------------------- */

static BOOL_T
_cli_validate_hostname(
    C8_T *ptr_hostname,
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    UI32_T *ptr_len)
{
    I32_T name_len;
    I32_T i;

    if ((NULL == ptr_hostname) || (NULL == ptr_out_buf) || (NULL == ptr_len))
    {
        return FALSE;
    }

    *ptr_len = 0;

    name_len = osapi_strlen(ptr_hostname);
    if (name_len >= MAX_SYS_NAME_SIZE)
    {
        *ptr_len += osapi_snprintf(ptr_out_buf + *ptr_len, out_buf_len - *ptr_len, CLI_ERR_HOSTNAME_TOO_LONG_FMT, MAX_SYS_NAME_SIZE - 1);
        return FALSE;
    }

    /* Hostname validation:
     * - Only letters, digits and hyphens
     * - Hyphen cannot be the first or last character
     */
    if ((name_len <= 0) ||
        ('-' == ptr_hostname[0]) ||
        ('-' == ptr_hostname[name_len - 1]))
    {
        *ptr_len += osapi_snprintf(ptr_out_buf + *ptr_len, out_buf_len - *ptr_len, CLI_ERR_INVALID_HOSTNAME);
        return FALSE;
    }

    for (i = 0; i < name_len; i++)
    {
        if (!(((ptr_hostname[i] >= 'A') && (ptr_hostname[i] <= 'Z')) ||
              ((ptr_hostname[i] >= 'a') && (ptr_hostname[i] <= 'z')) ||
              ((ptr_hostname[i] >= '0') && (ptr_hostname[i] <= '9')) ||
              ('-' == ptr_hostname[i])))
        {
            *ptr_len += osapi_snprintf(ptr_out_buf + *ptr_len, out_buf_len - *ptr_len, CLI_ERR_INVALID_HOSTNAME);
            return FALSE;
        }
    }

    return TRUE;
}

static UI32_T
_cli_cmd_set_hostname(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI16_T data_size = 0;
    C8_T *ptr_default_sysname = NULL;
    C8_T *ptr_cgi_str = NULL;
    UI32_T len = 0;

    if (FALSE == no_flag && NULL == pptr_argv[token_idx + 1])
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    if (TRUE == no_flag)
    {
        if (MW_E_OK != osapi_calloc(MAX_SYS_NAME_SIZE, CLI_TASK_NAME, (void **)&ptr_default_sysname))
        {
            CLI_LOG_ERROR("Failed to allocate memory for hostname CGI string");
            return 0;
        }
        if (MW_E_OK != dbapi_getFactoryDefault(SYS_INFO, SYS_NAME, DB_ALL_ENTRIES, &data_size, (void **)&ptr_default_sysname))
        {
            MW_FREE(ptr_default_sysname);
            osapi_strncpy(_ptr_cli->conf.hostname, CLI_HOSTNAME_DEFAULT, MAX_SYS_NAME_SIZE);
            _ptr_cli->conf.hostname[MAX_SYS_NAME_SIZE - 1] = '\0';
            CLI_LOG_WARN("Failed to get default DB hostname, using default hostname '%s'", CLI_HOSTNAME_DEFAULT);
            return 0;
        }
        osapi_strncpy(_ptr_cli->conf.hostname, ptr_default_sysname, MAX_SYS_NAME_SIZE);
        _ptr_cli->conf.hostname[MAX_SYS_NAME_SIZE - 1] = '\0';
        CLI_LOG_INFO("Hostname restored to default '%s'", _ptr_cli->conf.hostname);
        MW_FREE(ptr_default_sysname);
    }
    else
    {
        if (FALSE == _cli_validate_hostname(pptr_argv[token_idx + 1], ptr_out_buf, out_buf_len, &len))
        {
            return len;
        }

        osapi_strncpy(_ptr_cli->conf.hostname, pptr_argv[token_idx + 1], MAX_SYS_NAME_SIZE);
        _ptr_cli->conf.hostname[MAX_SYS_NAME_SIZE - 1] = '\0';
        CLI_LOG_INFO("Hostname changed to '%s'", _ptr_cli->conf.hostname);
    }

    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        CLI_LOG_ERROR("Failed to allocate memory for hostname CGI string");
        return 0;
    }
    osapi_snprintf(ptr_cgi_str, CLI_CGI_STR_MAX_LEN, "/system_name_set.cgi?sysName=%s", _ptr_cli->conf.hostname);
    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_cgi_str);
    return len;
}

static UI32_T
_cli_cmd_show_hostname(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode)
{
    UI16_T data_size = 0;
    DB_MSG_T *ptr_msg = NULL;
    C8_T *ptr_data = NULL;
    C8_T *ptr_hostname = NULL;
    UI32_T len = 0;

    if (MW_E_OK != osapi_calloc(MAX_SYS_NAME_SIZE, CLI_TASK_NAME, (void **)&ptr_hostname))
    {
        CLI_LOG_ERROR("Failed to allocate memory for hostname string");
        return len;
    }

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "!\n");

    if (CLI_SHOW_RUNNING_CONFIG == show_mode)
    {
        /* Get running hostname */
        if (MW_E_OK != cli_queue_getData(SYS_INFO, SYS_NAME, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
        {
            CLI_LOG_ERROR("Failed to get running hostname");
            MW_FREE(ptr_hostname);
            return len;
        }

        osapi_strncpy(ptr_hostname, ptr_data, MAX_SYS_NAME_SIZE);
        ptr_hostname[MAX_SYS_NAME_SIZE - 1] = '\0';

        /* ptr_data is part of ptr_msg payload; only free ptr_msg */
        MW_FREE(ptr_msg);
        ptr_msg = NULL;
    }
    else
    {
        /* Get startup hostname */
        if (MW_E_OK != dbapi_getStartUp(SYS_INFO, SYS_NAME, DB_ALL_ENTRIES, &data_size, (void **)&ptr_data))
        {
            CLI_LOG_WARN("Failed to get startup hostname");
            MW_FREE(ptr_hostname);
            return len;
        }

        osapi_strncpy(ptr_hostname, ptr_data, MAX_SYS_NAME_SIZE);
        ptr_hostname[MAX_SYS_NAME_SIZE - 1] = '\0';

        /* dbapi_getStartUp allocates ptr_data, free it after use */
        MW_FREE(ptr_data);
        ptr_data = NULL;
    }

    if (len < out_buf_len)
    {
        /* Print as: hostname "..." */
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "hostname \"%s\"\n", ptr_hostname);
    }

    MW_FREE(ptr_hostname);
    return len;
}

UI32_T
_cli_cmd_if_enterMode(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    I32_T  id = -1;
    UI32_T len = 0;
    UI32_T max_port = cli_cmd_port_getMaxNum();
    C8_T  *ptr_p = NULL;

    if (argc > 2)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    /* Port id string check */
    ptr_p = pptr_argv[1];
    while (0 != (*ptr_p))
    {
        if (0 == isdigit((I32_T)*ptr_p))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }
        ptr_p++;
    }

    /* Parse "GigabitEthernet0/X" or just "0/X" or "X" */
    /* Try to find the last '/' to identify the port number */
    C8_T *ptr_slash = strrchr(pptr_argv[1], '/');
    if ((NULL != ptr_slash) && (0 != (*(ptr_slash + 1))))
    {
        id = atoi(ptr_slash + 1);
    }
    else
    {
        /* Fallback: parse the first number found (e.g. "2") */
        ptr_p = pptr_argv[1];
        while ((0 != (*ptr_p)) && (0 == isdigit((I32_T)*ptr_p)))
        {
            ptr_p++;
        }
        if (0 != (*ptr_p))
        {
            id = atoi(ptr_p);
        }
    }

    if ((id > 0) && (id <= max_port))
    {
        G_CLI.current_if_idx = id;
        G_CLI.current_mode = CLI_MODE_CONFIG_IF;
        return len;
    }

    len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
    return len;
}

static UI32_T
_cli_cmd_if_setDuplex(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T   port_id;
    UI32_T   len = 0;

    if (argc > 2)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
        return len;
    }

    if (0 == osapi_strcmp(pptr_argv[0], "duplex"))
    {
        if ((osapi_strcmp(pptr_argv[1], "auto") != 0) &&
             (osapi_strcmp(pptr_argv[1], "full") != 0) &&
             (osapi_strcmp(pptr_argv[1], "half") != 0))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }

    if (CLI_MODE_CONFIG_IF == G_CLI.current_mode)
    {
        port_id = (UI32_T)G_CLI.current_if_idx;
        len = cli_cmd_port_setDuplex(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, port_id);
    }

    return len;
}

static UI32_T
_cli_cmd_if_setEEE(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T   port_id = 0;
    UI32_T   len = 0;

    if (0 == osapi_strcmp(pptr_argv[0], "no"))
    {
        if (argc > 3)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
        if (osapi_strcmp(pptr_argv[2], "enable") != 0)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }
    else
    {
        if (argc > 2)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
        if (osapi_strcmp(pptr_argv[1], "enable") != 0)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }

    if (CLI_MODE_CONFIG_IF == G_CLI.current_mode)
    {
        port_id = (UI32_T)G_CLI.current_if_idx;
        len = cli_cmd_port_setEEE(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, port_id);
    }

    return len;
}

static UI32_T
_cli_cmd_if_setFlowcontrol(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T   port_id;
    UI32_T   len = 0;

    if (argc > 2)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
        return len;
    }

    if (0 == osapi_strcmp(pptr_argv[0], "flow-control"))
    {
        if ((osapi_strcmp(pptr_argv[1], "auto") != 0) &&
             (osapi_strcmp(pptr_argv[1], "off") != 0) &&
             (osapi_strcmp(pptr_argv[1], "on") != 0))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }

    if (CLI_MODE_CONFIG_IF == G_CLI.current_mode)
    {
        port_id = (UI32_T)G_CLI.current_if_idx;
        len = cli_cmd_port_setFlowcontrol(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, port_id);
    }

    return len;
}

static UI32_T
_cli_cmd_if_setSpeed(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T   port_id;
    UI32_T   len = 0;

    if (argc > 2)
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
        return len;
    }

    if (0 == osapi_strcmp(pptr_argv[0], "speed"))
    {
        if ((osapi_strcmp(pptr_argv[1], "auto") != 0) &&
             (osapi_strcmp(pptr_argv[1], "10") != 0) &&
             (osapi_strcmp(pptr_argv[1], "100") != 0))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }

    if (CLI_MODE_CONFIG_IF == G_CLI.current_mode)
    {
        port_id = (UI32_T)G_CLI.current_if_idx;
        len = cli_cmd_port_setSpeed(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, port_id);
    }

    return len;
}

static UI32_T
_cli_cmd_if_setAdminState(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T   port_id;
    UI32_T   len = 0;

    if (0 == osapi_strcmp(pptr_argv[0], "no"))
    {
        if (argc > 2)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }
    else
    {
        if (argc > 1)
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, "%% Incomplete command.\n");
            return len;
        }
    }

    if (CLI_MODE_CONFIG_IF == G_CLI.current_mode)
    {
        port_id = (UI32_T)G_CLI.current_if_idx;
        len = cli_port_setAdminState(ptr_out_buf, out_buf_len, no_flag, argc, pptr_argv, token_idx, port_id);
    }

    return len;
}

#ifdef CLI_CFG_LINE_MODE
static UI32_T
_cli_cmd_set_line_mode(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI32_T len = 0;
    if (NULL == pptr_argv[token_idx + 1])
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }
    if (0 == osapi_strcmp(pptr_argv[token_idx + 1], "ssh") || 0 == osapi_strcmp(pptr_argv[token_idx + 1], "telnet"))
    {
        G_CLI.current_mode = CLI_MODE_CONFIG_LINE;
    }
    else
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INVALID_INPUT_DETECTED);
    }
    return len;
}
#endif

#ifdef CLI_CFG_AAA
static UI32_T
_cli_cmd_set_username(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    C8_T *ptr_name;
    I32_T priv = 1;
    C8_T *ptr_pwd = NULL;
    BOOL_T is_secret = FALSE;
    I32_T i;
    I32_T arg_idx = token_idx + 2;
    I32_T slot = -1;
    UI32_T len = 0;

    /* username <name> [privilege <0-15>] {password|secret} <pwd> */
    if (NULL == pptr_argv[token_idx + 3])
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }

    ptr_name = pptr_argv[token_idx + 1];

    /* Check for privilege option */
    if (0 == osapi_strcmp(pptr_argv[arg_idx], "privilege"))
    {
        if (NULL == pptr_argv[token_idx + 5])
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
            return len;
        }
        priv = atoi(pptr_argv[arg_idx + 1]);
        if ((priv < 0) || (priv > 15))
        {
            len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INVALID_PRIVILEGE);
            return len;
        }
        arg_idx += 2;
    }

    /* Check type */
    if (0 == osapi_strcmp(pptr_argv[arg_idx], "password"))
    {
        is_secret = FALSE;
    }
    else if (0 == osapi_strcmp(pptr_argv[arg_idx], "secret"))
    {
        is_secret = TRUE;
    }
    else
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INVALID_INPUT);
        return len;
    }
    ptr_pwd = pptr_argv[arg_idx + 1];

    /* Find existing or free slot */
    for (i = 0; i < CLI_MAX_USERS; i++)
    {
        if ((TRUE == _ptr_cli->conf.users[i].active) && (0 == osapi_strcmp(_ptr_cli->conf.users[i].username, ptr_name)))
        {
            slot = i;
            break; /* Update existing */
        }
    }
    if (-1 == slot)
    {
        for (i = 0; i < CLI_MAX_USERS; i++)
        {
            if (!_ptr_cli->conf.users[i].active)
            {
                slot = i;
                break; /* New slot */
            }
        }
    }

    if (-1 != slot)
    {
        osapi_strncpy(_ptr_cli->conf.users[slot].username, ptr_name, 31);
        osapi_strncpy(_ptr_cli->conf.users[slot].password, ptr_pwd, 31);
        _ptr_cli->conf.users[slot].privilege = priv;
        _ptr_cli->conf.users[slot].is_secret = is_secret;
        _ptr_cli->conf.users[slot].active = TRUE;
        CLI_LOG_INFO("User '%s' configured (Priv: %d, Slot: %d)", ptr_name, priv, slot);
    }
    else
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_MAX_USERS);
        CLI_LOG_WARN("Failed to configure user '%s': Max users reached", ptr_name);
    }
    return len;
}
#endif

#ifdef CLI_CFG_TERMINAL_LENGTH
static UI32_T
_cli_cmd_set_term_length(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    I32_T len_val;
    UI32_T len = 0;
    if (NULL == pptr_argv[token_idx + 2])
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INCOMPLETE_CMD);
        return len;
    }
    len_val = atoi(pptr_argv[token_idx + 2]);
    if ((len_val < 0) || (len_val > CLI_INPUT_MAX_LEN))
    {
        len = osapi_snprintf(ptr_out_buf, out_buf_len, CLI_ERR_INVALID_VALUE_FMT, CLI_INPUT_MAX_LEN);
        return len;
    }
    _ptr_cli->conf.term_length = len_val;
    return len;
}
#endif

#ifdef CLI_CFG_HISTORY_CMD
static void
_cli_free_history(
    CLI_CONTEXT_T *ptr_ctx)
{
    I32_T i;
    if (NULL == ptr_ctx)
    {
        return;
    }

    for (i = 0; i < CLI_HISTORY_SIZE; i++)
    {
        if (NULL != ptr_ctx->history[i].ptr_buffer)
        {
            MW_FREE(ptr_ctx->history[i].ptr_buffer);
            ptr_ctx->history[i].ptr_buffer = NULL;
        }
        ptr_ctx->history[i].len = 0;
    }
}

static void
_cli_add_to_history(
    void)
{
    BOOL_T was_full;

    if (G_CLI.len == 0)
    {
        return;
    }

    was_full = (G_CLI.hist_tail == G_CLI.hist_head) && (G_CLI.history[G_CLI.hist_head].ptr_buffer != NULL);

    if (NULL != G_CLI.history[G_CLI.hist_tail].ptr_buffer)
    {
        MW_FREE(G_CLI.history[G_CLI.hist_tail].ptr_buffer);
    }

    if (MW_E_OK != osapi_calloc(G_CLI.len + 1, CLI_TASK_NAME, (void **)&G_CLI.history[G_CLI.hist_tail].ptr_buffer))
    {
        G_CLI.history[G_CLI.hist_tail].len = 0;
        return;
    }

    /* Use osapi_memcpy to copy only valid characters, excluding potential garbage at line_buf[len] */
    osapi_memcpy(G_CLI.history[G_CLI.hist_tail].ptr_buffer, G_CLI.line_buf, G_CLI.len);
    G_CLI.history[G_CLI.hist_tail].ptr_buffer[G_CLI.len] = CLI_KEY_NUL;
    G_CLI.history[G_CLI.hist_tail].len = G_CLI.len;

    G_CLI.hist_tail = (G_CLI.hist_tail + 1) % CLI_HISTORY_SIZE;

    if (was_full)
    {
        G_CLI.hist_head = (G_CLI.hist_head + 1) % CLI_HISTORY_SIZE;
    }
    G_CLI.hist_idx = CLI_HIST_IDX_NONE;
}
#endif

/* Helper to validate a node against a token */
static BOOL_T
_cli_match_node(
    const CLI_CMD_NODE_T *ptr_node,
    const C8_T *ptr_token,
    BOOL_T *ptr_is_partial)
{
    UI32_T name_len;
    UI32_T token_len;

    if ((NULL == ptr_node) || (NULL == ptr_token) || (NULL == ptr_is_partial))
    {
        return FALSE;
    }

    *ptr_is_partial = FALSE;
    if (FALSE == ptr_node->is_param)
    {
        if (NULL == ptr_node->ptr_name)
        {
            return FALSE;
        }

        name_len = osapi_strlen(ptr_node->ptr_name);
        token_len = osapi_strlen(ptr_token);

        /* Exact match: Length must be equal first, then compare content safely */
        if ((name_len == token_len) && (0 == osapi_strncmp(ptr_node->ptr_name, ptr_token, name_len)))
        {
            return TRUE; /* Exact */
        }

        /* Partial match: Token must be shorter than keyword, and match the prefix */
        if ((token_len > 0) && (token_len < name_len) && (0 == osapi_strncmp(ptr_node->ptr_name, ptr_token, token_len)))
        {
            *ptr_is_partial = TRUE;
            return TRUE;
        }
    }
    else
    {
        /* Wildcard parameter, match anything */
        return TRUE;
    }
    return FALSE;
}

static BOOL_T
_cli_is_node_visible(
    const CLI_CMD_NODE_T *ptr_node,
    const C8_T *ptr_parent_path,
    BOOL_T is_do_cmd,
    BOOL_T is_no_cmd)
{
    /* 1. Check Parent Path Match */
    if (NULL == ptr_parent_path)
    {
        /* Root node check */
        if (NULL != ptr_node->ptr_parent_path)
        {
            return FALSE;
        }
    }
    else
    {
        if (NULL == ptr_node->ptr_parent_path)
        {
            return FALSE;
        }
        if (0 != osapi_strcmp(ptr_node->ptr_parent_path, ptr_parent_path))
        {
            return FALSE;
        }
    }

    /* 2. Check Mode Visibility */
    if (TRUE == is_do_cmd)
    {
        /* 'do' allows Privileged/Exec commands in Config modes */
        if ((G_CLI.current_mode >= CLI_MODE_CONFIG) && (ptr_node->mode <= CLI_MODE_PRIVILEGED))
        {
            /* Visible */
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        /* Normal Mode Check */
        if (CLI_MODE_USER == G_CLI.current_mode)
        {
            if (CLI_MODE_USER != ptr_node->mode)
            {
                return FALSE;
            }
        }
        else if (CLI_MODE_PRIVILEGED == G_CLI.current_mode)
        {
            if (ptr_node->mode > CLI_MODE_PRIVILEGED)
            {
                return FALSE;
            }
        }
        else
        {
            if (ptr_node->mode != G_CLI.current_mode)
            {
                return FALSE;
            }
        }
    }

    /* 3. Check 'no' support */
    if ((TRUE == is_no_cmd) && (FALSE == ptr_node->no_supported))
    {
        return FALSE;
    }

    return TRUE;
}

/*
 * Search static table for the best match against a token in specific context
 */
static void
_cli_search_best_match(
    const C8_T *ptr_parent_path,
    const C8_T *ptr_token,
    BOOL_T is_do_cmd,
    BOOL_T is_no_cmd,
    const CLI_CMD_NODE_T **pptr_match,
    I32_T *ptr_match_count)
{
    UI32_T i;
    UI32_T count = sizeof(_cli_cmd_tree) / sizeof(CLI_CMD_NODE_T);
    const CLI_CMD_NODE_T *ptr_curr;
    BOOL_T is_partial;

    *pptr_match = NULL;
    *ptr_match_count = 0;

    for (i = 0; i < count; i++)
    {
        ptr_curr = &_cli_cmd_tree[i];

        if (TRUE == _cli_is_node_visible(ptr_curr, ptr_parent_path, is_do_cmd, is_no_cmd))
        {
            if (TRUE == _cli_match_node(ptr_curr, ptr_token, &is_partial))
            {
                /* Prioritize exact keyword match immediately */
                if ((FALSE == ptr_curr->is_param) && (0 == osapi_strcmp(ptr_curr->ptr_name, ptr_token)))
                {
                    *pptr_match = ptr_curr;
                    *ptr_match_count = 1;
                    return;
                }

                if (0 == *ptr_match_count)
                {
                    *pptr_match = ptr_curr;
                }
                (*ptr_match_count)++;
            }
        }
    }
}

static void
_cli_parse_pipe(
    C8_T *ptr_cmd_line)
{
    C8_T *ptr_pipe;

    /* Filter Reset */
    G_CLI.filter_type = FILTER_NONE;

    /* Handle Pipe */
    ptr_pipe = osapi_strchr(ptr_cmd_line, '|');
    if (NULL != ptr_pipe)
    {
        *ptr_pipe = 0; /* Terminate command string */
        ptr_pipe++;
        while (CLI_KEY_SPACE == *ptr_pipe)
        {
            ptr_pipe++;
        }

        /* Parse filter */
        if (0 == osapi_strncmp(ptr_pipe, CLI_FILTER_INCLUDE_STR, osapi_strlen(CLI_FILTER_INCLUDE_STR)))
        {
            G_CLI.filter_type = FILTER_INCLUDE;
            osapi_strncpy(G_CLI.filter_text, ptr_pipe + osapi_strlen(CLI_FILTER_INCLUDE_STR) + 1, CLI_FILTER_CXT_SIZE - 1);
        }
        else if (0 == osapi_strncmp(ptr_pipe, CLI_FILTER_EXCLUDE_STR, osapi_strlen(CLI_FILTER_EXCLUDE_STR)))
        {
            G_CLI.filter_type = FILTER_EXCLUDE;
            osapi_strncpy(G_CLI.filter_text, ptr_pipe + osapi_strlen(CLI_FILTER_EXCLUDE_STR) + 1, CLI_FILTER_CXT_SIZE - 1);
        }
        else if (0 == osapi_strncmp(ptr_pipe, CLI_FILTER_BEGIN_STR, osapi_strlen(CLI_FILTER_BEGIN_STR)))
        {
            G_CLI.filter_type = FILTER_BEGIN;
            G_CLI.filter_triggered = FALSE;
            osapi_strncpy(G_CLI.filter_text, ptr_pipe + osapi_strlen(CLI_FILTER_BEGIN_STR) + 1, CLI_FILTER_CXT_SIZE - 1);
        }
        else if (0 == osapi_strncmp(ptr_pipe, CLI_FILTER_SECTION_STR, osapi_strlen(CLI_FILTER_SECTION_STR)))
        {
            G_CLI.filter_type = FILTER_SECTION;
            G_CLI.filter_triggered = FALSE;
            osapi_strncpy(G_CLI.filter_text, ptr_pipe + osapi_strlen(CLI_FILTER_SECTION_STR) + 1, CLI_FILTER_CXT_SIZE - 1);
        }
    }
}

/* Resolve user input tokens into defined command tree paths */
static void
_cli_build_resolved_path(
    C8_T *ptr_buf,
    UI32_T buf_len,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T start_idx,
    I32_T depth_to_resolve,
    BOOL_T is_do_cmd,
    BOOL_T is_no_cmd)
{
    I32_T i;
    const CLI_CMD_NODE_T *ptr_match = NULL;
    I32_T matches = 0;
    C8_T *ptr_current_path = NULL;

    if (MW_E_OK != osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_current_path))
    {
        return;
    }

    ptr_buf[0] = '\0';

    for (i = 0; i < depth_to_resolve; i++)
    {
        /* Use a separate buffer to hold the parent path for searching to avoid overlap issues */
        C8_T *ptr_parent_path = (i == 0) ? NULL : ptr_current_path;

        _cli_search_best_match(ptr_parent_path, pptr_argv[start_idx + i], is_do_cmd, is_no_cmd, &ptr_match, &matches);

        if ((1 == matches) && (NULL != ptr_match))
        {
            if (i > 0)
            {
                osapi_strncat(ptr_current_path, " ", CLI_INPUT_MAX_LEN - osapi_strlen(ptr_current_path) - 1);
            }
            /* Use the node's defined name instead of user input, resolving parameter values properly */
            osapi_strncat(ptr_current_path, ptr_match->ptr_name, CLI_INPUT_MAX_LEN - osapi_strlen(ptr_current_path) - 1);
        }
        else
        {
            if (i > 0)
            {
                osapi_strncat(ptr_current_path, " ", CLI_INPUT_MAX_LEN - osapi_strlen(ptr_current_path) - 1);
            }
            osapi_strncat(ptr_current_path, pptr_argv[start_idx + i], CLI_INPUT_MAX_LEN - osapi_strlen(ptr_current_path) - 1);
        }
    }

    osapi_strncpy(ptr_buf, ptr_current_path, buf_len - 1);
    ptr_buf[buf_len - 1] = '\0';
    MW_FREE(ptr_current_path);
}

static MW_ERROR_NO_T
_cli_find_command_node(
    I32_T argc,
    C8_T **pptr_argv,
    I32_T start_idx,
    BOOL_T is_do_cmd,
    BOOL_T is_no_cmd,
    const CLI_CMD_NODE_T **pptr_ret_node,
    I32_T *ptr_ret_depth)
{
    C8_T *ptr_path_buf = NULL;
    const CLI_CMD_NODE_T *ptr_match = NULL;
    I32_T matches = 0;
    I32_T depth = start_idx;
    C8_T *ptr_current_path = NULL;

    *pptr_ret_node = NULL;
    *ptr_ret_depth = depth;

    if (MW_E_OK != osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_path_buf))
    {
        return MW_E_NO_MEMORY;
    }

    while (depth < argc)
    {
        /* 1. Construct Parent Path for current level using the resolved path builder */
        /* E.g. Depth 0 -> NULL, Depth 1 -> "show", Depth 2 -> "show ip" */
        if (depth > start_idx)
        {
            _cli_build_resolved_path(ptr_path_buf, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, depth - start_idx, is_do_cmd, is_no_cmd);
            ptr_current_path = ptr_path_buf;
        }
        else
        {
            ptr_current_path = NULL;
        }

        /* 2. Search in static table */
        matches = 0;
        ptr_match = NULL;
        _cli_search_best_match(ptr_current_path, pptr_argv[depth], is_do_cmd, is_no_cmd, &ptr_match, &matches);

        if (1 == matches)
        {
            /* Update the argv token to the full command name (for abbreviated inputs) */
            if (FALSE == ptr_match->is_param)
            {
                pptr_argv[depth] = (C8_T *)ptr_match->ptr_name;
            }

            depth++;
            *ptr_ret_depth = depth;

            /* Check for end of input */
            if (depth == argc)
            {
                *pptr_ret_node = ptr_match;
                MW_FREE(ptr_path_buf);
                if (NULL == ptr_match->ptr_handler)
                {
                    return MW_E_OP_INCOMPLETE;
                }
                return MW_E_OK;
            }
            /* Continue to next level */
        }
        else if (matches > 1)
        {
            *ptr_ret_depth = depth;
            MW_FREE(ptr_path_buf);
            return MW_E_ENTRY_HASH_CONFLICT;
        }
        else
        {
            *ptr_ret_depth = depth;
            MW_FREE(ptr_path_buf);
            return MW_E_ENTRY_NOT_FOUND;
        }
    }

    MW_FREE(ptr_path_buf);
    return MW_E_ENTRY_NOT_FOUND;
}

static void
_cli_execute_command(
    void)
{
    C8_T **pptr_argv = NULL;
    I32_T argc = 0;
    C8_T *ptr_cmd_copy = NULL;
    C8_T *ptr_p;
    BOOL_T is_do_cmd = FALSE;
    BOOL_T is_no_cmd = FALSE;
    MW_ERROR_NO_T rc;
    const CLI_CMD_NODE_T *ptr_found = NULL;
    I32_T depth = 0, exec_argc = 0, exec_token_idx = 0;
    C8_T **pptr_exec_argv = NULL;
    I32_T offset = 0, prompt_len = 0, i = 0;

    cli_output("\n");
    /* Flush prompt \n immediately before execution logic */
    _cli_flush_output();

    if (0 == G_CLI.len)
    {
        _cli_redraw_line();
        return;
    }

    /* Allocate argv on heap (pointer array) */
    rc = osapi_calloc(CLI_MAX_TOKEN * sizeof(C8_T *), CLI_TASK_NAME, (void **)&pptr_argv);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("memory fail");
        G_CLI.len = 0;
        G_CLI.pos = 0;
        _cli_redraw_line();
        return;
    }

#ifdef CLI_CFG_HISTORY_CMD
    _cli_add_to_history();
#endif

    rc = osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cmd_copy);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("Command execution failed: memory allocation error");
        MW_FREE(pptr_argv);
        G_CLI.len = 0;
        G_CLI.pos = 0;
        _cli_redraw_line();
        return;
    }

    osapi_memcpy(ptr_cmd_copy, G_CLI.line_buf, G_CLI.len);
    ptr_cmd_copy[G_CLI.len] = 0;

    /* Log the command execution (Audit Trail) */
    CLI_LOG_INFO("Session %d Exec: %s", _ptr_cli->current_session_id, ptr_cmd_copy);

    _cli_parse_pipe(ptr_cmd_copy);

    /* Tokenize */
    ptr_p = osapi_strtok(ptr_cmd_copy, " ");
    while ((NULL != ptr_p) && (argc < CLI_MAX_TOKEN))
    {
        pptr_argv[argc++] = ptr_p;
        ptr_p = osapi_strtok(NULL, " ");
    }

    if (argc > 0)
    {
        I32_T token_idx = 0;

        /* Handle 'do' command without shifting */
        if ((argc > token_idx) && (0 == osapi_strcmp(pptr_argv[token_idx], "do")))
        {
            is_do_cmd = TRUE;
            token_idx++;
        }

        /* Handle 'no' command without shifting */
        if ((argc > token_idx) && (0 == osapi_strcmp(pptr_argv[token_idx], "no")))
        {
            is_no_cmd = TRUE;
            token_idx++;
        }

        /* Find and Execute */
        rc = _cli_find_command_node(argc, pptr_argv, token_idx, is_do_cmd, is_no_cmd, &ptr_found, &depth);

        if (MW_E_OK == rc)
        {
            if ((TRUE == is_no_cmd) && (FALSE == ptr_found->no_supported))
            {
                cli_output(CLI_ERR_CMD_NO_SUPPORT_NO);
            }
            else
            {
                G_CLI.output_pos = 0;
                G_CLI.output_buffer[0] = 0;

                /* Add token_idx (which is the offset caused by do/no) to the configured index to keep it relative */
                pptr_exec_argv = pptr_argv;
                exec_argc = argc;
                exec_token_idx = token_idx;

                if (TRUE == is_do_cmd)
                {
                    pptr_exec_argv++;
                    exec_argc--;
                    exec_token_idx--;
                }

                G_CLI.output_pos = ptr_found->ptr_handler(G_CLI.output_buffer, G_CLI.output_buffer_size, is_no_cmd, exec_argc, pptr_exec_argv,
                                       (ptr_found->token_idx != 0) ? (ptr_found->token_idx + exec_token_idx) : exec_token_idx);
            }
        }
        else if (MW_E_ENTRY_HASH_CONFLICT == rc)
        {
            cli_output(CLI_ERR_AMBIGUOUS_CMD_FMT, pptr_argv[depth]);
        }
        else if (MW_E_OP_INCOMPLETE == rc)
        {
            cli_output(CLI_ERR_INCOMPLETE_CMD);
        }
        else
        {
            offset = 0;
            prompt_len = osapi_strlen(_ptr_cli->conf.hostname);

            /* Calculate the character offset based on pointer distance */
            if ((pptr_argv[depth] >= ptr_cmd_copy) && (pptr_argv[depth] < (ptr_cmd_copy + CLI_INPUT_MAX_LEN)))
            {
                offset = (I32_T)(pptr_argv[depth] - ptr_cmd_copy);
            }

            /* Add the prompt string length according to the current mode */
            switch (G_CLI.current_mode)
            {
#ifdef CLI_CFG_USER_MODE
                case CLI_MODE_USER:
                    prompt_len += osapi_strlen(CLI_USER_MODE_PROMPT);
                    break;
#endif
                case CLI_MODE_PRIVILEGED:
                    prompt_len += osapi_strlen(CLI_PRIVILEGED_MODE_PROMPT);
                    break;
                case CLI_MODE_CONFIG:
                    prompt_len += osapi_strlen(CLI_GLOBAL_CONFIG_MODE_PROMPT);
                    break;
                case CLI_MODE_CONFIG_IF:
                    prompt_len += osapi_strlen(CLI_INTERFACE_CONFIG_MODE_PROMPT);
                    break;
#ifdef CLI_CFG_LINE_MODE
                case CLI_MODE_CONFIG_LINE:
                    prompt_len += osapi_strlen(CLI_LINE_CONFIG_MODE_PROMPT);
                    break;
#endif
                default:
                    prompt_len += osapi_strlen(CLI_PRIVILEGED_MODE_PROMPT);
                    break;
            }

            /* Print spaces up to the error token and then point with '^' */
            for (i = 0; i < prompt_len + offset; i++)
            {
                cli_output(" ");
            }
            cli_output("^\n");
            cli_output(CLI_ERR_INVALID_INPUT_AT_MARKER);
        }
    }

    MW_FREE(ptr_cmd_copy);
    MW_FREE(pptr_argv);
    G_CLI.len = 0;
    G_CLI.pos = 0;
    _cli_redraw_line();
}

static void
_cli_complete_tab(
    void)
{
    C8_T *ptr_buf_copy = NULL;
    C8_T **pptr_argv = NULL;
    I32_T argc = 0;
    C8_T *ptr_p;
    MW_ERROR_NO_T rc;
    I32_T ends_with_space;
    BOOL_T is_do_cmd = FALSE;
    BOOL_T is_no_cmd = FALSE;
    I32_T start_idx = 0;

    C8_T *ptr_path_buf = NULL;
    const C8_T *ptr_parent_path = NULL;
    C8_T *ptr_prefix = "";
    I32_T prefix_len = 0;

    const CLI_CMD_NODE_T *ptr_curr;
    const CLI_CMD_NODE_T *ptr_match = NULL;
    I32_T match_count = 0;
    UI32_T i;
    UI32_T total_nodes = sizeof(_cli_cmd_tree) / sizeof(CLI_CMD_NODE_T);

    I32_T saved_term_len;
    I32_T col_cnt;

    if (0 == G_CLI.len)
    {
        return;
    }

    rc = osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_buf_copy);
    if (MW_E_OK != rc)
    {
        return;
    }

    rc = osapi_calloc(CLI_MAX_TOKEN * sizeof(C8_T *), CLI_TASK_NAME, (void **)&pptr_argv);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_buf_copy);
        return;
    }

    rc = osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_path_buf);
    if (MW_E_OK != rc)
    {
        MW_FREE(pptr_argv);
        MW_FREE(ptr_buf_copy);
        return;
    }

    osapi_memcpy(ptr_buf_copy, G_CLI.line_buf, G_CLI.len);
    ptr_buf_copy[G_CLI.len] = 0;

    ends_with_space = (G_CLI.line_buf[G_CLI.len - 1] == ' ');

    ptr_p = osapi_strtok(ptr_buf_copy, " ");
    while ((NULL != ptr_p) && (argc < CLI_MAX_TOKEN))
    {
        pptr_argv[argc++] = ptr_p;
        ptr_p = osapi_strtok(NULL, " ");
    }

    /* Check Context */
    if (argc > 0)
    {
        if (0 == osapi_strcmp(pptr_argv[0], "do"))
        {
            is_do_cmd = TRUE;
            start_idx = 1;
        }
        else if (0 == osapi_strcmp(pptr_argv[0], "no"))
        {
            is_no_cmd = TRUE;
            start_idx = 1;
        }
    }

    /* Calculate Parent Path and Prefix */
    if (0 != ends_with_space)
    {
        /* Completing next argument, prefix is empty */
        ptr_prefix = "";
        if (argc > start_idx)
        {
            _cli_build_resolved_path(ptr_path_buf, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, argc - start_idx, is_do_cmd, is_no_cmd);
            ptr_parent_path = ptr_path_buf;
        }
        else
        {
            ptr_parent_path = NULL;
        }
    }
    else
    {
        /* Completing current argument */
        if (argc > start_idx)
        {
            ptr_prefix = pptr_argv[argc - 1];
            if (argc - 1 > start_idx)
            {
                _cli_build_resolved_path(ptr_path_buf, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, argc - 1 - start_idx, is_do_cmd, is_no_cmd);
                ptr_parent_path = ptr_path_buf;
            }
            else
            {
                ptr_parent_path = NULL;
            }
        }
        else
        {
            ptr_prefix = "";
            ptr_parent_path = NULL;
        }
    }

    prefix_len = osapi_strlen(ptr_prefix);

    /* Search Loop */
    for (i = 0; i < total_nodes; i++)
    {
        ptr_curr = &_cli_cmd_tree[i];

        if (TRUE == _cli_is_node_visible(ptr_curr, ptr_parent_path, is_do_cmd, is_no_cmd))
        {
            /* Only autocomplete Keywords, not Params */
            if (FALSE == ptr_curr->is_param)
            {
                if (0 == osapi_strncmp(ptr_curr->ptr_name, ptr_prefix, prefix_len))
                {
                    match_count++;
                    ptr_match = ptr_curr;
                }
            }
        }
    }

    /* Apply Completion */
    if ((1 == match_count) && (NULL != ptr_match))
    {
        const C8_T *ptr_completion = ptr_match->ptr_name + prefix_len;
        while (*ptr_completion)
        {
            if (G_CLI.len < CLI_INPUT_MAX_LEN - 1)
            {
                G_CLI.line_buf[G_CLI.pos++] = *ptr_completion;
                G_CLI.len++;
                _cli_putc(*ptr_completion);
                ptr_completion++;
            }
            else
            {
                break;
            }
        }
        if (G_CLI.len < CLI_INPUT_MAX_LEN - 1)
        {
            G_CLI.line_buf[G_CLI.pos++] = ' ';
            G_CLI.len++;
            _cli_putc(' ');
        }
        G_CLI.consecutive_tabs = 0;
    }
    else if ((match_count > 1) && (G_CLI.consecutive_tabs >= 2))
    {
        /* List matches */
        saved_term_len = _ptr_cli->conf.term_length;
        col_cnt = 0;
        _ptr_cli->conf.term_length = 0;

        cli_output("\n");
        for (i = 0; i < total_nodes; i++)
        {
            ptr_curr = &_cli_cmd_tree[i];
            if (TRUE == _cli_is_node_visible(ptr_curr, ptr_parent_path, is_do_cmd, is_no_cmd))
            {
                if (FALSE == ptr_curr->is_param)
                {
                    if (0 == osapi_strncmp(ptr_curr->ptr_name, ptr_prefix, prefix_len))
                    {
                        cli_output("%-19s ", ptr_curr->ptr_name);
                        col_cnt++;
                    }
                }
            }
        }
        if (col_cnt > 0)
        {
            cli_output("\n");
        }
        _ptr_cli->conf.term_length = saved_term_len;
        _cli_redraw_line();
    }

    MW_FREE(ptr_path_buf);
    MW_FREE(pptr_argv);
    MW_FREE(ptr_buf_copy);
}

static BOOL_T
_cli_has_children(
    const CLI_CMD_NODE_T *ptr_node)
{
    UI32_T i;
    UI32_T total_nodes = sizeof(_cli_cmd_tree) / sizeof(CLI_CMD_NODE_T);
    C8_T *ptr_full_path = NULL;

    if (MW_E_OK != osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_full_path))
    {
        return FALSE;
    }

    if (NULL != ptr_node->ptr_parent_path)
    {
        osapi_snprintf(ptr_full_path, CLI_INPUT_MAX_LEN, "%s %s", ptr_node->ptr_parent_path, ptr_node->ptr_name);
    }
    else
    {
        osapi_snprintf(ptr_full_path, CLI_INPUT_MAX_LEN, "%s", ptr_node->ptr_name);
    }

    for (i = 0; i < total_nodes; i++)
    {
        if ((NULL != _cli_cmd_tree[i].ptr_parent_path) && (0 == osapi_strcmp(_cli_cmd_tree[i].ptr_parent_path, ptr_full_path)))
        {
            if (_cli_cmd_tree[i].mode == ptr_node->mode)
            {
                MW_FREE(ptr_full_path);
                return TRUE;
            }
        }
    }
    MW_FREE(ptr_full_path);
    return FALSE;
}

static void
_cli_show_help(
    void)
{
    C8_T *ptr_buf_copy = NULL;
    C8_T **pptr_argv = NULL;
    I32_T argc = 0;
    C8_T *ptr_p;
    MW_ERROR_NO_T rc;
    I32_T ends_with_space;
    BOOL_T is_do_cmd = FALSE;
    BOOL_T is_no_cmd = FALSE;
    I32_T start_idx = 0;

    C8_T *ptr_path_buf = NULL;
    const C8_T *ptr_parent_path = NULL;
    C8_T *ptr_prefix = "";
    I32_T prefix_len = 0;

    const CLI_CMD_NODE_T *ptr_curr;
    UI32_T i;
    UI32_T total_nodes = sizeof(_cli_cmd_tree) / sizeof(CLI_CMD_NODE_T);

    BOOL_T show_cr = FALSE;
    I32_T max_len = CLI_HELP_ALIGN_MIN_WIDTH;
    I32_T name_len = 0;

    cli_output("\n");

    rc = osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_buf_copy);
    if (MW_E_OK != rc)
    {
        return;
    }

    rc = osapi_calloc(CLI_MAX_TOKEN * sizeof(C8_T *), CLI_TASK_NAME, (void **)&pptr_argv);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_buf_copy);
        return;
    }

    rc = osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_path_buf);
    if (MW_E_OK != rc)
    {
        MW_FREE(pptr_argv);
        MW_FREE(ptr_buf_copy);
        return;
    }

    /* Parse current line up to cursor position */
    if (G_CLI.pos > 0)
    {
        osapi_memcpy(ptr_buf_copy, G_CLI.line_buf, G_CLI.pos);
    }
    ptr_buf_copy[G_CLI.pos] = 0;

    /* Check if we are looking for next argument or completing current word */
    ends_with_space = (G_CLI.pos == 0) || (G_CLI.line_buf[G_CLI.pos - 1] == ' ');

    ptr_p = osapi_strtok(ptr_buf_copy, " ");
    while ((NULL != ptr_p) && (argc < CLI_MAX_TOKEN))
    {
        pptr_argv[argc++] = ptr_p;
        ptr_p = osapi_strtok(NULL, " ");
    }

    /* Check Context */
    if (argc > 0)
    {
        if (0 == osapi_strcmp(pptr_argv[0], "do"))
        {
            is_do_cmd = TRUE;
            start_idx = 1;
        }
        else if (0 == osapi_strcmp(pptr_argv[0], "no"))
        {
            is_no_cmd = TRUE;
            start_idx = 1;
        }
    }

    /* Calculate Parent Path and Prefix */
    if (0 != ends_with_space)
    {
        ptr_prefix = "";
        if (argc > start_idx)
        {
            _cli_build_resolved_path(ptr_path_buf, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, argc - start_idx, is_do_cmd, is_no_cmd);
            ptr_parent_path = ptr_path_buf;
        }
        else
        {
            ptr_parent_path = NULL;
        }

        /* Check <cr> condition: If path forms a valid command with handler */
        if (argc > start_idx)
        {
            C8_T *ptr_full_cmd_path = NULL;
            const C8_T *p_parent = NULL;
            const C8_T *p_name = pptr_argv[argc - 1];
            const CLI_CMD_NODE_T *ptr_best_match = NULL;
            I32_T matches = 0;

            if (MW_E_OK == osapi_calloc(CLI_INPUT_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_full_cmd_path))
            {
                if (argc - 1 > start_idx)
                {
                    _cli_build_resolved_path(ptr_full_cmd_path, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, argc - 1 - start_idx, is_do_cmd, is_no_cmd);
                    p_parent = ptr_full_cmd_path;
                }

                _cli_search_best_match(p_parent, p_name, is_do_cmd, is_no_cmd, &ptr_best_match, &matches);

                if ((1 == matches) && (ptr_best_match != NULL))
                {
                    if (NULL != ptr_best_match->ptr_handler)
                    {
                        if ((FALSE == is_no_cmd) && _cli_has_children(ptr_best_match))
                        {
                            /* Intermediate node requiring parameters, no <cr> */
                        }
                        else
                        {
                            show_cr = TRUE;
                        }
                    }
                }
                MW_FREE(ptr_full_cmd_path);
            }
        }
    }
    else
    {
        if (argc > start_idx)
        {
            ptr_prefix = pptr_argv[argc - 1];
            if (argc - 1 > start_idx)
            {
                _cli_build_resolved_path(ptr_path_buf, CLI_INPUT_MAX_LEN, argc, pptr_argv, start_idx, argc - 1 - start_idx, is_do_cmd, is_no_cmd);
                ptr_parent_path = ptr_path_buf;
            }
            else
            {
                ptr_parent_path = NULL;
            }
        }
        else
        {
            ptr_prefix = "";
            ptr_parent_path = NULL;
        }
    }

    prefix_len = osapi_strlen(ptr_prefix);

    /* First pass: Calculate max length */
    for (i = 0; i < total_nodes; i++)
    {
        ptr_curr = &_cli_cmd_tree[i];
        if (TRUE == _cli_is_node_visible(ptr_curr, ptr_parent_path, is_do_cmd, is_no_cmd))
        {
            if (FALSE == ptr_curr->is_param)
            {
                if (0 == osapi_strncmp(ptr_curr->ptr_name, ptr_prefix, prefix_len))
                {
                    name_len = osapi_strlen(ptr_curr->ptr_name);
                    if (name_len > max_len) max_len = name_len;
                }
            }
            else
            {
                name_len = osapi_strlen(ptr_curr->ptr_name);
                if (name_len > max_len) max_len = name_len;
            }
        }
    }

    /* Second pass: Print */
    for (i = 0; i < total_nodes; i++)
    {
        ptr_curr = &_cli_cmd_tree[i];
        if (TRUE == _cli_is_node_visible(ptr_curr, ptr_parent_path, is_do_cmd, is_no_cmd))
        {
            BOOL_T match = FALSE;
            if (FALSE == ptr_curr->is_param)
            {
                if (0 == osapi_strncmp(ptr_curr->ptr_name, ptr_prefix, prefix_len)) match = TRUE;
            }
            else
            {
                match = TRUE;
            }

            if (TRUE == match)
            {
                cli_output("  %s", ptr_curr->ptr_name);
                name_len = osapi_strlen(ptr_curr->ptr_name);
                while (name_len < max_len)
                {
                    cli_output(" ");
                    name_len++;
                }
                cli_output("    %s\n", ptr_curr->ptr_help ? ptr_curr->ptr_help : "");
            }
        }
    }

    if (TRUE == show_cr)
    {
        cli_output("  <cr>");
        name_len = 4;
        while (name_len < max_len)
        {
            cli_output(" ");
            name_len++;
        }
        cli_output("\n");
    }

    _cli_redraw_line();
    MW_FREE(ptr_path_buf);
    MW_FREE(pptr_argv);
    MW_FREE(ptr_buf_copy);
}

static void
_cli_handle_auth(
    C8_T c)
{
    I32_T i;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    C8_T *ptr_username = NULL;
    C8_T *ptr_password = NULL;
    BOOL_T login_success;
#ifdef CLI_CFG_AAA
    BOOL_T auth_ok;
#endif

    if ((CLI_KEY_ENTER == c) || (CLI_KEY_NEWLINE == c))
    {
        _cli_putc(CLI_KEY_NEWLINE);

        if (CLI_AUTH_STATE_WAIT_USERNAME == G_CLI.auth_state)
        {
            if (G_CLI.auth_pos > 0)
            {
                /* Store username in input_username */
                osapi_strncpy(G_CLI.input_username, G_CLI.input_password, 31);
                /* Move to Password */
                G_CLI.auth_state = CLI_AUTH_STATE_WAIT_PASSWORD;
                G_CLI.auth_pos = 0;
                G_CLI.input_password[0] = 0;
            }
        }
        else if (CLI_AUTH_STATE_WAIT_PASSWORD == G_CLI.auth_state)
        {
            /* Verify Login */
            login_success = FALSE;
            for (i = 0; i < CLI_MAX_USERS; i++)
            {
                if (TRUE == _ptr_cli->conf.users[i].active)
                {
                    if (MW_E_OK != osapi_calloc(MAX_USER_NAME_SIZE, CLI_TASK_NAME, (void **)&ptr_username))
                    {
                        CLI_LOG_ERROR("Failed to allocate memory for username");
                        break;
                    }
                    if (MW_E_OK != osapi_calloc(MAX_PASSWORD_SIZE, CLI_TASK_NAME, (void **)&ptr_password))
                    {
                        MW_FREE(ptr_username);
                        CLI_LOG_ERROR("Failed to allocate memory for password");
                        break;
                    }

                    if (MW_E_OK != cli_queue_getData(ACCOUNT_INFO, ACC_USERNAME, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
                    {
                        MW_FREE(ptr_username);
                        MW_FREE(ptr_password);
                        CLI_LOG_ERROR("Failed to get username from DB");
                        break;
                    }
                    osapi_memcpy(ptr_username, ptr_data, data_size);
                    MW_FREE(ptr_msg);

                    if (MW_E_OK != cli_queue_getData(ACCOUNT_INFO, ACC_PASSWD, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
                    {
                        MW_FREE(ptr_username);
                        MW_FREE(ptr_password);
                        CLI_LOG_ERROR("Failed to get password from DB");
                        break;
                    }
                    osapi_memcpy(ptr_password, ptr_data, data_size);
                    MW_FREE(ptr_msg);

                    if ((MW_E_OK == mw_account_verifyUser(G_CLI.input_username, ptr_username)) && (MW_E_OK == mw_account_verifyPass(G_CLI.input_password, ptr_password, FALSE)))
                    {
                        login_success = TRUE;
                        MW_FREE(ptr_username);
                        MW_FREE(ptr_password);
                        break;
                    }
                    MW_FREE(ptr_username);
                    MW_FREE(ptr_password);
                }
            }

            if (TRUE == login_success)
            {
                CLI_LOG_INFO("User '%s' logged in (Session %d)", G_CLI.input_username, _ptr_cli->current_session_id);
                G_CLI.auth_state = CLI_AUTH_STATE_SUCCESS;
#ifndef CLI_CFG_USER_MODE
                G_CLI.current_mode = CLI_MODE_PRIVILEGED;
#else
                G_CLI.current_mode = CLI_MODE_USER;
#endif
            }
            else
            {
                CLI_LOG_WARN("Failed login attempt for '%s' (Session %d)", G_CLI.input_username, _ptr_cli->current_session_id);
                cli_output(CLI_ERR_AUTH_FAILED);
                /* Reset to Username input */
                G_CLI.auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
                G_CLI.auth_pos = 0;
                G_CLI.input_password[0] = 0;
            }
        }
#ifdef CLI_CFG_AAA
        else if (CLI_AUTH_STATE_ENABLE == G_CLI.auth_state)
        {
            G_CLI.auth_state = CLI_AUTH_STATE_SUCCESS;
            /* Check Enable Password */
            auth_ok = FALSE;
            if (osapi_strlen(_ptr_cli->conf.enable_secret) > 0)
            {
                if (0 == osapi_strcmp(G_CLI.input_password, _ptr_cli->conf.enable_secret))
                {
                    auth_ok = TRUE;
                }
            }
            else if (osapi_strlen(_ptr_cli->conf.enable_password) > 0)
            {
                if (0 == osapi_strcmp(G_CLI.input_password, _ptr_cli->conf.enable_password))
                {
                    auth_ok = TRUE;
                }
            }
            else
            {
                auth_ok = TRUE;
            }

            if (auth_ok)
            {
                G_CLI.current_mode = CLI_MODE_PRIVILEGED;
            }
            else
            {
                cli_output(CLI_ERR_BAD_PASSWORD);
            }
        }
#endif
        else if (CLI_AUTH_STATE_RELOAD_CONFIRM == G_CLI.auth_state)
        {
            /* Confirm reload upon Enter key */
            _cli_putc(CLI_KEY_NEWLINE);
            _cli_puts("Reload confirmed\n");
            _cli_flush_output();
            CLI_LOG_WARN("System reload by session %d", _ptr_cli->current_session_id);
            osapi_delay(CLI_RELOAD_DELAY_TIME);
            air_chipscu_resetSystem(0);
        }
        _cli_redraw_line();
    }
    else if ((CLI_KEY_BACKSPACE == c) || (CLI_KEY_DEL == c))
    {
        if (G_CLI.auth_pos > 0)
        {
            G_CLI.auth_pos--;
            G_CLI.input_password[G_CLI.auth_pos] = 0;
            /* Echo backspace for USER mode input */
            if ((CLI_AUTH_STATE_WAIT_USERNAME == G_CLI.auth_state) ||
                (CLI_AUTH_STATE_WAIT_PASSWORD == G_CLI.auth_state) ||
                (CLI_AUTH_STATE_ENABLE == G_CLI.auth_state))
            {
                _cli_putc(CLI_KEY_BACKSPACE);
                _cli_putc(' ');
                _cli_putc(CLI_KEY_BACKSPACE);
            }
        }
    }
    else if (CLI_KEY_CTRL_L == c)
    {
        _cli_redraw_line();
    }
    else if (CLI_AUTH_STATE_RELOAD_CONFIRM == G_CLI.auth_state)
    {
        /* Any key other than Enter (handled in Enter block) cancels reload */
        G_CLI.auth_state = CLI_AUTH_STATE_SUCCESS;
        cli_output("\n%% Aborted.\n");
        _cli_redraw_line();
    }
    else if (isprint((int)c))
    {
        if (G_CLI.auth_pos < 31)
        {
            G_CLI.input_password[G_CLI.auth_pos++] = c;
            G_CLI.input_password[G_CLI.auth_pos] = 0;
            /* Echo char for USER mode input */
            if (CLI_AUTH_STATE_WAIT_USERNAME == G_CLI.auth_state)
            {
                _cli_putc(c);
            }
            else if ((CLI_AUTH_STATE_WAIT_PASSWORD == G_CLI.auth_state) ||
                    (CLI_AUTH_STATE_ENABLE == G_CLI.auth_state))
            {
                _cli_putc('*');
            }
        }
    }
}

static void
_cli_handle_escape(
    C8_T c)
{
    if (CLI_ESC_STATE_ESC == G_CLI.escape_seq_state)
    {
        if ('[' == c)
        {
            G_CLI.escape_seq_state = CLI_ESC_STATE_BRACKET;
        }
        else
        {
            G_CLI.escape_seq_state = CLI_ESC_STATE_NONE;
        }
    }
    else if (CLI_ESC_STATE_BRACKET == G_CLI.escape_seq_state)
    {
        switch (c)
        {
            case 'A': /* Up */
                _cli_handle_normal(CLI_KEY_CTRL_P);
                break;
            case 'B': /* Down */
                _cli_handle_normal(CLI_KEY_CTRL_N);
                break;
            case 'C': /* Right */
                _cli_handle_normal(CLI_KEY_CTRL_F);
                break;
            case 'D': /* Left */
                _cli_handle_normal(CLI_KEY_CTRL_B);
                break;
            default:
                break;
        }
        G_CLI.escape_seq_state = CLI_ESC_STATE_NONE;
    }
}

static void
_cli_handle_normal(
    C8_T c)
{
    I32_T i;

    switch (c)
    {
        case CLI_KEY_ENTER:
        case CLI_KEY_NEWLINE:
            _cli_execute_command();
            break;

        case CLI_KEY_BACKSPACE:
        case CLI_KEY_DEL:
            if (G_CLI.pos > 0)
            {
                osapi_memmove(&G_CLI.line_buf[G_CLI.pos - 1], &G_CLI.line_buf[G_CLI.pos], G_CLI.len - G_CLI.pos);
                G_CLI.pos--;
                G_CLI.len--;
                _cli_putc(CLI_KEY_BACKSPACE);
                for (i = G_CLI.pos; i < G_CLI.len; i++)
                {
                    _cli_putc(G_CLI.line_buf[i]);
                }
                _cli_putc(' ');
                for (i = G_CLI.len; i >= G_CLI.pos; i--)
                {
                    _cli_putc(CLI_KEY_BACKSPACE);
                }
            }
            break;

        case CLI_KEY_TAB:
            G_CLI.consecutive_tabs++;
            _cli_complete_tab();
            break;

        case CLI_KEY_QUESTION:
            _cli_show_help();
            break;

        case CLI_KEY_CTRL_C:
            _cli_puts("^C");
            G_CLI.len = 0;
            G_CLI.pos = 0;
            G_CLI.line_buf[0] = 0;
#ifdef CLI_CFG_HISTORY_CMD
            G_CLI.hist_idx = CLI_HIST_IDX_NONE;
#endif
            _cli_putc(CLI_KEY_NEWLINE);
            _cli_redraw_line();
            break;

        /* Emacs / Cisco Shortcuts */
        case CLI_KEY_CTRL_A: /* Start of line */
            while (G_CLI.pos > 0)
            {
                _cli_putc(CLI_KEY_BACKSPACE);
                G_CLI.pos--;
            }
            break;

        case CLI_KEY_CTRL_E: /* End of line */
            while (G_CLI.pos < G_CLI.len)
            {
                _cli_putc(G_CLI.line_buf[G_CLI.pos++]);
            }
            break;

        case CLI_KEY_CTRL_F: /* Forward char */
            if (G_CLI.pos < G_CLI.len)
            {
                _cli_putc(G_CLI.line_buf[G_CLI.pos++]);
            }
            break;

        case CLI_KEY_CTRL_B: /* Backward char */
            if (G_CLI.pos > 0)
            {
                _cli_putc(CLI_KEY_BACKSPACE);
                G_CLI.pos--;
            }
            break;

        case CLI_KEY_CTRL_P: /* Up */
#ifdef CLI_CFG_HISTORY_CMD
            {
                UI8_T current_idx;

                if (G_CLI.history[G_CLI.hist_head].ptr_buffer == NULL)
                {
                    break;
                }
                if (G_CLI.hist_idx >= CLI_HISTORY_SIZE)
                {
                    current_idx = (G_CLI.hist_tail + CLI_HISTORY_SIZE - 1) % CLI_HISTORY_SIZE;
                }
                else
                {
                    if (G_CLI.hist_idx == G_CLI.hist_head)
                    {
                        break;
                    }
                    current_idx = (G_CLI.hist_idx + CLI_HISTORY_SIZE - 1) % CLI_HISTORY_SIZE;
                }

                G_CLI.hist_idx = current_idx;
                if (G_CLI.history[G_CLI.hist_idx].ptr_buffer)
                {
                    osapi_strcpy(G_CLI.line_buf, G_CLI.history[G_CLI.hist_idx].ptr_buffer);
                    G_CLI.len = G_CLI.history[G_CLI.hist_idx].len;
                    G_CLI.pos = G_CLI.len;
                    _cli_redraw_line();
                }
            }
#endif
            break;

        case CLI_KEY_CTRL_N: /* Down */
#ifdef CLI_CFG_HISTORY_CMD
            {
                UI8_T current_idx;

                if (G_CLI.hist_idx >= CLI_HISTORY_SIZE)
                {
                    break;
                }

                current_idx = (G_CLI.hist_idx + 1) % CLI_HISTORY_SIZE;

                if (current_idx == G_CLI.hist_tail)
                {
                    G_CLI.hist_idx = CLI_HIST_IDX_NONE;
                    G_CLI.line_buf[0] = 0;
                    G_CLI.len = 0;
                    G_CLI.pos = 0;
                }
                else
                {
                    G_CLI.hist_idx = current_idx;
                    if (G_CLI.history[G_CLI.hist_idx].ptr_buffer)
                    {
                        osapi_strcpy(G_CLI.line_buf, G_CLI.history[G_CLI.hist_idx].ptr_buffer);
                        G_CLI.len = G_CLI.history[G_CLI.hist_idx].len;
                        G_CLI.pos = G_CLI.len;
                    }
                }
                _cli_redraw_line();
            }
#endif
            break;

        case CLI_KEY_CTRL_U: /* Kill line */
            G_CLI.pos = 0;
            G_CLI.len = 0;
            _cli_redraw_line();
            break;

        case CLI_KEY_CTRL_K: /* Kill to end */
            G_CLI.len = G_CLI.pos;
            _cli_puts(CLI_ERASE_IN_LINE);
            break;

        case CLI_KEY_CTRL_W: /* Delete word backward */
            while (G_CLI.pos > 0 && G_CLI.line_buf[G_CLI.pos - 1] == ' ')
            {
                /* Skip trailing spaces */
                /* Delete char logic */
                osapi_memmove(&G_CLI.line_buf[G_CLI.pos - 1], &G_CLI.line_buf[G_CLI.pos], G_CLI.len - G_CLI.pos);
                G_CLI.pos--;
                G_CLI.len--;
            }
            while (G_CLI.pos > 0 && G_CLI.line_buf[G_CLI.pos - 1] != ' ')
            {
                /* Delete non-spaces */
                osapi_memmove(&G_CLI.line_buf[G_CLI.pos - 1], &G_CLI.line_buf[G_CLI.pos], G_CLI.len - G_CLI.pos);
                G_CLI.pos--;
                G_CLI.len--;
            }
            _cli_redraw_line();
            break;

        case CLI_KEY_CTRL_L: /* Redraw */
            _cli_redraw_line();
            break;

        case CLI_KEY_CTRL_Z: /* Return to privileged */
            if (G_CLI.current_mode > CLI_MODE_PRIVILEGED)
            {
                G_CLI.current_mode = CLI_MODE_PRIVILEGED;
                _cli_putc(CLI_KEY_NEWLINE);
                _cli_redraw_line();
            }
            break;

        case CLI_KEY_ESC:
            G_CLI.escape_seq_state = CLI_ESC_STATE_ESC;
            break;

        default:
            if ((isprint((int)c)) && (G_CLI.len < CLI_INPUT_MAX_LEN - 1))
            {
                if (G_CLI.pos == G_CLI.len)
                {
                    G_CLI.line_buf[G_CLI.pos++] = c;
                    G_CLI.len++;
                    _cli_putc(c);
                }
                else
                {
                    osapi_memmove(&G_CLI.line_buf[G_CLI.pos + 1], &G_CLI.line_buf[G_CLI.pos], G_CLI.len - G_CLI.pos);
                    G_CLI.line_buf[G_CLI.pos] = c;
                    G_CLI.len++;
                    for (i = G_CLI.pos; i < G_CLI.len; i++)
                    {
                        _cli_putc(G_CLI.line_buf[i]);
                    }
                    G_CLI.pos++;
                    for (i = 0; i < (G_CLI.len - G_CLI.pos); i++)
                    {
                        _cli_putc(CLI_KEY_BACKSPACE);
                    }
                }
            }
            break;
    }
}

static void
_cli_handle_cmd_input_msg(
    I32_T session_id,
    C8_T c)
{
    /* Set the active session context for this execution slice */
    if ((session_id >= 0) && (session_id < CLI_MAX_SESSIONS) && (_ptr_cli->ptr_sessions[session_id] != NULL))
    {
        _ptr_cli->ptr_current_ctx = _ptr_cli->ptr_sessions[session_id];
        _ptr_cli->current_session_id = session_id;
    }
    else
    {
        CLI_LOG_WARN("Invalid session ID: %d or session context not found", session_id);
        return;
    }

    /* Safety Check */
    if (NULL == _ptr_cli->ptr_current_ctx)
    {
        return;
    }

    /* Reset consecutive tabs on any key other than TAB */
    if (CLI_KEY_TAB != c)
    {
        G_CLI.consecutive_tabs = 0;
    }

    /* Dispatch to Sub-Handlers based on state */
    if (CLI_AUTH_STATE_SUCCESS != G_CLI.auth_state)
    {
        _cli_handle_auth(c);
    }
    else if (CLI_ESC_STATE_NONE != G_CLI.escape_seq_state)
    {
        _cli_handle_escape(c);
    }
    else
    {
        _cli_handle_normal(c);
    }

    _cli_flush_output();

    /* Reset filter state to ensure it doesn't persist to subsequent inputs/prompts */
    G_CLI.filter_type = FILTER_NONE;
    G_CLI.filter_triggered = FALSE;
}

static void
_cli_task(
    void *ptr_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    void *ptr_rx_item = NULL;
    MW_MSG_T *ptr_msg = NULL;
    DB_MSG_T *ptr_db_msg = NULL;
    CLI_CMD_INPUT_T *ptr_packed = NULL;
    CLI_SESSION_DEREGISTER_MSG_T *ptr_dereg_msg = NULL;

    do
    {
        osapi_delay(CLI_CHECK_DB_READY_MS);
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

    cli_queue_send(M_SUBSCRIBE, SYS_INFO, SYS_NAME, DB_ALL_ENTRIES, 0, 0, NULL);

    while (1)
    {
        rc = osapi_msgRecv(CLI_QUEUE_NAME, (UI8_T **)&ptr_rx_item, 0, CLI_QUEUE_TIMEOUT);
        if (MW_E_OK == rc)
        {
            ptr_packed = (CLI_CMD_INPUT_T *)&ptr_rx_item;
            if (CLI_CMD_INPUT_MAGIC_NUM == ptr_packed->magic_num)
            {
                if (CLI_CMD_INPUT_MSG_ID == ptr_packed->msg_id)
                {
                    _cli_handle_cmd_input_msg(ptr_packed->session_id, ptr_packed->c);
                }
            }
            else
            {
                ptr_msg = (MW_MSG_T *)ptr_rx_item;

                if (NULL != ptr_msg)
                {
                    if (MW_MSG_ID_DB == ptr_msg->msg_id)
                    {
                        ptr_db_msg = (DB_MSG_T *)ptr_msg;
                        cli_db_handle_msg(ptr_db_msg);
                    }
                    else if (CLI_SESSION_DEREGISTER_MSG_ID == ptr_msg->msg_id)
                    {
                        ptr_dereg_msg = (CLI_SESSION_DEREGISTER_MSG_T *)ptr_msg;
                        cli_session_deregister(ptr_dereg_msg->session_id);
                    }
                    MW_FREE(ptr_msg);
                }
            }
        }
    }
}

static I32_T
_cli_cgi_extract_parameters(
    C8_T *ptr_params,
    C8_T **pptr_param,
    C8_T **pptr_param_val)
{
    C8_T *ptr_pair;
    C8_T *ptr_equals;
    I32_T loop;

    /* If we have no parameters at all, return immediately. */
    if ((NULL == ptr_params) || (ptr_params[0] == '\0'))
    {
        return (0);
    }

    /* Get a pointer to our first parameter */
    ptr_pair = ptr_params;

    /* Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string and ignore the
     * remainder (if any) */
    for (loop = 0; (loop < LWIP_HTTPD_MAX_CGI_PARAMETERS) && (NULL != ptr_pair); loop++)
    {
        /* Save the name of the parameter */
        pptr_param[loop] = ptr_pair;

        /* Remember the start of this name=value pair */
        ptr_equals = ptr_pair;

        /* Find the start of the next name=value pair and replace the delimiter
         * with a 0 to terminate the previous pair string. */
        ptr_pair = osapi_strchr(ptr_pair, '&');
        if (NULL != ptr_pair)
        {
            *ptr_pair = '\0';
            ptr_pair++;
        }
        else
        {
            /* We didn't find a new parameter so find the end of the URI and
             * replace the space with a '\0' */
            ptr_pair = osapi_strchr(ptr_equals, ' ');
            if (NULL != ptr_pair)
            {
                *ptr_pair = '\0';
            }

            /* Revert to NULL so that we exit the loop as expected. */
            ptr_pair = NULL;
        }

        /* Now find the '=' in the previous pair, replace it with '\0' and save
         * the parameter value string. */
        ptr_equals = osapi_strchr(ptr_equals, '=');
        if (NULL != ptr_equals)
        {
            *ptr_equals = '\0';
            pptr_param_val[loop] = ptr_equals + 1;
        }
        else
        {
            pptr_param_val[loop] = NULL;
        }
    }

    return loop;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: cli_modify_cmd_node_name
 * PURPOSE:
 *      Traverse the command tree to find nodes with a specific name and modify them to a new name.
 * INPUT:
 *      ptr_old_name -- Pointer to the old name of the command node to be replaced.
 *      ptr_new_name -- Pointer to the new name of the command node.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      The ptr_new_name should point to a constant string or valid memory with a lifetime matching the system.
 */
void
cli_modify_cmd_node_name(
    const C8_T *ptr_old_name,
    const C8_T *ptr_new_name)
{
    UI32_T i;

    if ((NULL == ptr_old_name) || (NULL == ptr_new_name))
    {
        return;
    }

    /* Since the command tree is static const (Flash), we cannot modify the pointers.
     * Instead, ptr_old_name is used to identify the target dynamic string buffer.
     * We just overwrite the buffer contents in RAM. */
    for (i = 0; i < (sizeof(_cli_dyn_labels) / sizeof(CLI_DYN_LABEL_ENTRY_T)); i++)
    {
        if (0 == osapi_strcmp(_cli_dyn_labels[i].ptr_original_name, ptr_old_name))
        {
            osapi_strncpy(_cli_dyn_labels[i].ptr_ram_buffer, ptr_new_name, _cli_dyn_labels[i].buffer_len - 1);
            _cli_dyn_labels[i].ptr_ram_buffer[_cli_dyn_labels[i].buffer_len - 1] = '\0';
            break;
        }
    }
}

/* FUNCTION NAME: cli_session_register
 * PURPOSE:
 *      Register a new CLI session (e.g. for Telnet/SSH).
 * INPUT:
 *      ptr_out_func -- Callback function for outputting data.
 * OUTPUT:
 *      None
 * RETURN:
 *      I32_T        -- Session ID (>=1) on success, -1 on failure.
 * NOTES:
 *      None
 */
I32_T
cli_session_register(
    void (*ptr_out_func)(UI8_T session, C8_T *ptr_str, UI32_T len))
{
    I32_T i;

    /* Find free slot, start from 1 (0 is reserved for Console) */
    for (i = 1; i < CLI_MAX_SESSIONS; i++)
    {
        if (NULL == _ptr_cli->ptr_sessions[i])
        {
            if (MW_E_OK == osapi_calloc(sizeof(CLI_CONTEXT_T), CLI_TASK_NAME, (void **)&_ptr_cli->ptr_sessions[i]))
            {
                /* Allocate Output Buffer */
                if (MW_E_OK != osapi_calloc(CLI_OUTPUT_BUF_SIZE, CLI_TASK_NAME, (void **)&_ptr_cli->ptr_sessions[i]->output_buffer))
                {
                    CLI_LOG_ERROR("Failed to allocate output buffer for Session %d", i);
                    MW_FREE(_ptr_cli->ptr_sessions[i]);
                    return -1;
                }
                _ptr_cli->ptr_sessions[i]->output_buffer_size = CLI_OUTPUT_BUF_SIZE;
                _ptr_cli->ptr_sessions[i]->ptr_out_fn = ptr_out_func;
                _ptr_cli->ptr_sessions[i]->current_mode = CLI_MODE_USER;
                _ptr_cli->ptr_sessions[i]->auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
                CLI_LOG_INFO("Session %d registered", i);
                return i;
            }
            else
            {
                CLI_LOG_ERROR("Failed to allocate memory for Session %d", i);
            }
            break;
        }
    }
    CLI_LOG_WARN("Session register failed: Max sessions reached or memory error");
    return -1;
}

/* FUNCTION NAME: cli_session_deregister
 * PURPOSE:
 *      Deregister a CLI session (e.g. Telnet/SSH disconnect) and free resources.
 * INPUT:
 *      session_id   -- The Session ID to free.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_session_deregister(
    I32_T session_id)
{
    if (NULL == _ptr_cli)
    {
        return MW_E_NOT_INITED;
    }

    /* Safety Check: Session 0 is Console, it should never be deregistered */
    if ((session_id <= 0) || (session_id >= CLI_MAX_SESSIONS))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Check if session is actually in use */
    if (NULL != _ptr_cli->ptr_sessions[session_id])
    {
#ifdef CLI_CFG_HISTORY_CMD
        _cli_free_history(_ptr_cli->ptr_sessions[session_id]);
#endif
        if (_ptr_cli->ptr_sessions[session_id]->output_buffer)
        {
            MW_FREE(_ptr_cli->ptr_sessions[session_id]->output_buffer);
        }
        /* Free the context memory associated with this session */
        MW_FREE(_ptr_cli->ptr_sessions[session_id]);
        CLI_LOG_INFO("Session %d deregistered", session_id);

        return MW_E_OK;
    }

    CLI_LOG_WARN("Attempted to deregister invalid/inactive session %d", session_id);
    return MW_E_ENTRY_NOT_FOUND;
}

/* FUNCTION NAME: cli_send_session_deregster_msg
 * PURPOSE:
 *      Send a message to the CLI queue to deregister a session.
 * INPUT:
 *      session_id   -- The ID of the session to be deregistered.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_send_session_deregster_msg(
    UI8_T session_id)
{
    CLI_SESSION_DEREGISTER_MSG_T *ptr_cli_dr_msg = NULL;

    if (MW_E_OK != osapi_calloc(sizeof(CLI_SESSION_DEREGISTER_MSG_T), CLI_TASK_NAME, (void **)&ptr_cli_dr_msg))
    {
        CLI_LOG_ERROR("Failed to allocate memory for session deregister msg");
        return MW_E_NO_MEMORY;
    }

    ptr_cli_dr_msg->msg_id = CLI_SESSION_DEREGISTER_MSG_ID;
    ptr_cli_dr_msg->session_id = session_id;

    if (MW_E_OK != osapi_msgSend(CLI_QUEUE_NAME, (UI8_T *)ptr_cli_dr_msg, 0, CLI_SESSION_MSG_TIMEOUT))
    {
        return cli_session_deregister(session_id);
    }

    return MW_E_OK;
}
/* FUNCTION NAME: cli_output
 * PURPOSE:
 *      Print formatted output to the current session.
 * INPUT:
 *      fmt          -- Format string.
 *      ...          -- Variable arguments.
 * OUTPUT:
 *      None
 * RETURN:
 *      void
 * NOTES:
 *      None
 */
void
cli_output(
    const C8_T *ptr_fmt,
    ...)
{
    va_list args;
    I32_T len;
    UI32_T available;

    if (NULL == _ptr_cli || NULL == _ptr_cli->ptr_current_ctx)
    {
        return;
    }

    /* Ensure space for at least some chars + null */
    if (G_CLI.output_pos >= G_CLI.output_buffer_size - 1)
    {
        return;
    }

    available = G_CLI.output_buffer_size - G_CLI.output_pos;

    va_start(args, ptr_fmt);
    /* Write directly to output buffer */
    len = vsnprintf(G_CLI.output_buffer + G_CLI.output_pos, available, ptr_fmt, args);
    va_end(args);

    if (len <= 0)
    {
        return;
    }

    /* Handle truncation if output exceeds available buffer */
    if ((UI32_T)len >= available)
    {
        len = available - 1;
        G_CLI.output_buffer[G_CLI.output_pos + len] = 0;
    }

    G_CLI.output_pos += len;
    /* Ensure null termination (vsnprintf adds it, but shifting/truncation might affect it) */
    G_CLI.output_buffer[G_CLI.output_pos] = 0;
}

/* FUNCTION NAME: cli_get_info
 * PURPOSE:
 *      Get the CLI global context pointer.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      CLI_DATA_T * -- Pointer to the CLI context.
 * NOTES:
 *      None
 */
CLI_DATA_T *
cli_get_info(
    void)
{
    return _ptr_cli;
}

/* FUNCTION NAME: cli_init
 * PURPOSE:
 *      Initialize the CLI module, task, and data structures.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success, otherwise error code.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_LOG_INIT_PRINTF("Initializing Command Line Interface...\n");

    rc = osapi_msgCreate(CLI_QUEUE_NAME, CLI_QUEUE_LEN, sizeof(void *));
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("CLI init queue fail!");
        return rc;
    }

    rc = osapi_msgCreate(CLI_GET_QUEUE_NAME, CLI_GET_QUEUE_LEN, sizeof(void *));
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("CLI init get queue fail!");
        return rc;
    }

    rc = osapi_calloc(sizeof(CLI_DATA_T), CLI_TASK_NAME, (void **)&_ptr_cli);
    if (MW_E_OK != rc)
    {
        CLI_LOG_ERROR("CLI init memory fail!");
        return rc;
    }

    _ptr_cli->cli_queue_handle = osapi_msgFindHandle(CLI_QUEUE_NAME);

    rc = osapi_threadCreateStatic(CLI_TASK_NAME, CLI_TASK_STACK_SIZE, CLI_TASK_PRIORITY, _cli_task, NULL,
                                    _ptr_cli->cli_task_stack,
                                    &_ptr_cli->cli_task_tcb,
                                    &_ptr_cli->cli_task_handle);
    if (MW_E_OK != rc)
    {
        cli_deinit();
        CLI_LOG_ERROR("CLI init task fail!");
        return rc;
    }

    /* Register Console Session (Session ID 0) */
    rc = osapi_calloc(sizeof(CLI_CONTEXT_T), CLI_TASK_NAME, (void **)&_ptr_cli->ptr_sessions[0]);
    if (MW_E_OK != rc)
    {
        cli_deinit();
        CLI_LOG_ERROR("CLI init context memory fail!");
        return rc;
    }

    /* Allocate Output Buffer for Console */
    rc = osapi_calloc(CLI_OUTPUT_BUF_SIZE, CLI_TASK_NAME, (void **)&_ptr_cli->ptr_sessions[0]->output_buffer);
    if (MW_E_OK != rc)
    {
        cli_deinit();
        CLI_LOG_ERROR("CLI init output buffer fail!");
        return rc;
    }
    _ptr_cli->ptr_sessions[0]->output_buffer_size = CLI_OUTPUT_BUF_SIZE;
    _ptr_cli->ptr_sessions[0]->ptr_out_fn = NULL;

    /* Initialize Global Context Pointer */
    _ptr_cli->ptr_current_ctx = _ptr_cli->ptr_sessions[0];
    _ptr_cli->current_session_id = 0;

    osapi_strncpy(_ptr_cli->conf.hostname, CLI_HOSTNAME_DEFAULT, 31);
    _ptr_cli->conf.term_length = CLI_PAGE_LINES_DEFAULT;

    /* Initialize Console Context defaults */
    G_CLI.current_mode = CLI_MODE_USER;

    /* Add default user for demo */
#ifdef CLI_CFG_AAA
    osapi_strncpy(_ptr_cli->conf.users[0].username, "admin", MAX_USER_NAME_SIZE - 1);
    osapi_strncpy(_ptr_cli->conf.users[0].password, "admin", MAX_PASSWORD_SIZE - 1);
    _ptr_cli->conf.users[0].privilege = 15;
    osapi_strncpy(_ptr_cli->conf.enable_password, "airoha", 31);
#endif
    _ptr_cli->conf.users[0].active = TRUE;

    /* Start in Login Mode */
    G_CLI.auth_state = CLI_AUTH_STATE_WAIT_USERNAME;
    G_CLI.auth_pos = 0;

    uart_register_input_cb(cli_uart_input_cb);

    return rc;
}

/* FUNCTION NAME: cli_deinit
 * PURPOSE:
 *      De-initialize the CLI module and free all resources.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_deinit(
    void)
{
    I32_T i;
    threadhandle_t temp;

    if (NULL == _ptr_cli)
    {
        return MW_E_NOT_INITED;
    }

    /* Free Session Contexts */
    for (i = 0; i < CLI_MAX_SESSIONS; i++)
    {
        if (NULL != _ptr_cli->ptr_sessions[i])
        {
#ifdef CLI_CFG_HISTORY_CMD
            _cli_free_history(_ptr_cli->ptr_sessions[i]);
#endif
            if (_ptr_cli->ptr_sessions[i]->output_buffer)
            {
                MW_FREE(_ptr_cli->ptr_sessions[i]->output_buffer);
            }
            MW_FREE(_ptr_cli->ptr_sessions[i]);
        }
    }

    /* Delete Queue */
    if (NULL != _ptr_cli->cli_queue_handle)
    {
        osapi_msgDelete(CLI_QUEUE_NAME);
    }

    temp = _ptr_cli->cli_task_handle;
    MW_FREE(_ptr_cli);

    if (NULL != temp)
    {
        osapi_threadDelete(temp);
    }

    return MW_E_OK;
}

/* FUNCTION NAME: cli_input
 * PURPOSE:
 *      Send an input character to the CLI task queue.
 * INPUT:
 *      session_id   -- The ID of the session.
 *      c            -- The character.
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_ERROR_NO_T -- MW_E_OK if success.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
cli_input(
    UI8_T session_id,
    UI8_T c)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    MW_ERROR_NO_T rc = MW_E_OK;
    CLI_CMD_INPUT_T item = { CLI_CMD_INPUT_MAGIC_NUM, CLI_CMD_INPUT_MSG_ID, session_id, c };

    if ((NULL != _ptr_cli) && (NULL != _ptr_cli->cli_task_handle))
    {
        if (CLI_CONSOLE_SESSION_ID == session_id)
        {
            /* Use xHigherPriorityTaskWoken to reduce latency */
            xQueueSendFromISR(_ptr_cli->cli_queue_handle, (void *)&item, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            xQueueSend(_ptr_cli->cli_queue_handle, (void *)&item, 0);
        }
    }

    return rc;
}

/* FUNCTION NAME:  cli_cgi_proxy
 * PURPOSE:
 *      Proxy the cli request to CGI function
 *
 * INPUT:
 *      ptr_cgi_str  --  cgi string
 *      len          --  cgi string length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      Only support MW html/cgi
 */
void
cli_cgi_proxy(
    C8_T *ptr_cgi_str,
    UI16_T buffer_len)
{
    C8_T *ptr_param_str = NULL;
    const tCGI *ptr_cgis = CGIURLs;
    UI16_T cgi_num = get_numCgiHandler();
    UI16_T i = 0;
    I32_T cgi_paramcount;
    C8_T *ptr_params[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Params extracted from the request ptr_cgi_str */
    C8_T *ptr_param_vals[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Values for each extracted param */

    if (NULL == ptr_cgi_str)
    {
        return;
    }
    if ((buffer_len > 0) && (cgi_num > 0))
    {
        ptr_cgi_str[buffer_len - 1] = '\0';
        CLI_LOG_DEBUG("cli cgi request: %s", ptr_cgi_str);
        ptr_param_str = (char *)osapi_strchr(ptr_cgi_str, '?');
        if (NULL != ptr_param_str)
        {
            /* ptr_cgi_str contains parameters. NULL-terminate the base ptr_cgi_str */
            *ptr_param_str = '\0';
            ptr_param_str++;
        }

        for (i = 0; i < cgi_num; i++)
        {
            if (0 == osapi_strcmp(ptr_cgi_str, ptr_cgis[i].pcCGIName))
            {
                /*
                 * We found a CGI that handles this ptr_cgi_str so extract the
                 * parameters and call the handler.
                 */
                cgi_paramcount = _cli_cgi_extract_parameters(ptr_param_str, ptr_params, ptr_param_vals);
                /* CGI handle function */
                CLI_LOG_DEBUG("handle=[%d] paramCnt=[%d]", i, cgi_paramcount);
#ifdef AIR_SUPPORT_HTTPD_MUTEX
                if (MW_E_OK == mw_httpd_mutex_take(CLI_CGI_MUX_LOCK_TIME))
                {
                    ptr_cgis[i].pfnCGIHandler(i, cgi_paramcount, ptr_params, ptr_param_vals);
                    CLI_LOG_DEBUG("CGI Done");
                    mw_httpd_mutex_give();
                }
                else
                {
                    CLI_LOG_DEBUG("Failed to take httpd mutex");
                }
#else
                ptr_cgis[i].pfnCGIHandler(i, cgi_paramcount, ptr_params, ptr_param_vals);
                CLI_LOG_DEBUG("CGI Done");
#endif
                break;
            }
        }
    }
}

void
create_queue_recv_task(
    void)
{
    if (MW_E_OK != cli_init())
    {
        MW_LOG_INIT_PRINTF("Failed to initialize Command Line Interface!\n");
    }
}
