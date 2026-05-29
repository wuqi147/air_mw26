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

/* FILE NAME:   cli.h
 * PURPOSE:
 *      Define the data structures, enums and APIs for the CLI (Command Line Interface) module.
 * NOTES:
 *      None
 */

#ifndef CLI_H
#define CLI_H

#ifdef AIR_SUPPORT_CLI
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CLI_TASK_NAME           "CLI"
#define CLI_TASK_STACK_SIZE     (500)
#define CLI_TASK_PRIORITY       (3)
#define CLI_QUEUE_NAME          "CLQ"
#define CLI_QUEUE_LEN           (128)
#define CLI_GET_QUEUE_NAME      "CLG"
#define CLI_GET_QUEUE_LEN       (1)
#define CLI_QUEUE_TIMEOUT       (0xFFFFFFFF)

#define CLI_CONSOLE_SESSION_ID  (0)

#define CLI_SHOW_RUNNING_CONFIG (0)
#define CLI_SHOW_STARTUP_CONFIG (1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef UI32_T (*CLI_SHOW_CFG_HANDLER)(C8_T *ptr_out_buf, UI32_T out_buf_len, I32_T show_mode);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

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
    void);

/* FUNCTION NAME: cli_output
 * PURPOSE:
 *      Print formatted output to the current session (supports paging/filtering).
 * INPUT:
 *      ptr_fmt      -- Format string.
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
    ...);

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
    const C8_T *ptr_new_name);

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
    void (*ptr_out_func)(UI8_T session, C8_T *ptr_str, UI32_T len));

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
 *      Session 0 (Console) cannot be deregistered.
 */
MW_ERROR_NO_T
cli_session_deregister(
    I32_T session_id);

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
    UI8_T session_id);
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
    UI8_T c);

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
    void);

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
    UI16_T buffer_len);

#endif
#endif /* CLI_H */