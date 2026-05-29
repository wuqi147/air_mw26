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

/* FILE NAME:   mw_ssh.h
 * PURPOSE:
 *      It provides ssh API and definitions.
 * NOTES:
 *
 */

#ifndef MW_SSH_H
#define MW_SSH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_log.h"
#include "remote_debug_log.h"
#include "osapi_thread.h"
#include "lwip/sockets.h"
#include "switch.h"
#include "mw_types.h"
#include "dropbear_api.h"
/* NAMING CONSTANT DECLARATIONS
 */
#ifndef AIR_SUPPORT_CLI
#define CMD_MAX_INPUT_SIZE          256
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct MW_SSH_CONNECTION_S {
    I32_T session_id;
    BOOL_T login_status;
#ifndef AIR_SUPPORT_CLI
    C8_T   input_string[CMD_MAX_INPUT_SIZE];
    UI32_T input_index;
    UI8_T  last_input_char;
    BOOL_T is_esc;
#endif
} MW_SSH_CONNECTION_T;

typedef struct MW_SSH_INFO_S {
    threadhandle_t ssh_task_handle;
    StackType_t ssh_task_stack[SSH_STACK_SIZE];
    StaticTask_t ssh_task_tcb;
    BOOL_T log_enable;
    UI32_T client_num;
    MW_SSH_CONNECTION_T client_conn[SSH_MAX_CLIENT_NUM];
} MW_SSH_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);

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
    int *ptr_session_id);

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
    int *ptr_session_id);

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
    void);

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
    UI32_T input_len);

#endif /* MW_SSH_H */
