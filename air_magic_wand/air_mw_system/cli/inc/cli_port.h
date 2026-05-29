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

#ifndef CLI_PORT_H
#define CLI_PORT_H

#include "mw_error.h"
#include "mw_types.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#define CLI_PORT_SETTINGS_DUPLEX_HALF_BITMASK      (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100HFDX)

#define CLI_PORT_SETTINGS_DUPLEX_FULL_BITMASK      (PORT_SETTINGS_RATE_DUPLEX_10FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_1000FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_2500M)

#define CLI_PORT_SETTINGS_DUPLEX_AUTO_10_BITMASK   (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_10FUDX)

#define CLI_PORT_SETTINGS_DUPLEX_AUTO_100_BITMASK  (PORT_SETTINGS_RATE_DUPLEX_100HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_1000FUDX)

#define CLI_PORT_SETTINGS_DUPLEX_AUTO_1000_BITMASK (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_10FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_1000FUDX)

#define CLI_PORT_SETTINGS_DUPLEX_AUTO_2500_BITMASK (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_10FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_1000FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_2500M)

#define CLI_PORT_SETTINGS_SPEED_10_BITMASK         (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_10FUDX)

#define CLI_PORT_SETTINGS_SPEED_100_BITMASK        (PORT_SETTINGS_RATE_DUPLEX_100HFDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_100FUDX)

#define CLI_PORT_SETTINGS_SPEED_AUTO_BITMASK       (PORT_SETTINGS_RATE_DUPLEX_1000FUDX | \
                                                 PORT_SETTINGS_RATE_DUPLEX_2500M)


#define CLI_PORT_STRING_LEN    (32)

#define CLI_PORT_LIST_STR_SIZE (8)
#define CLI_PORT_LIST_LABEL    "PORT-LIST"

/* DATA TYPE DECLARATIONS
 */

typedef enum
{
    CLI_PORT_DUPLEX_TYPE_AUTO,
    CLI_PORT_DUPLEX_TYPE_FULL,
    CLI_PORT_DUPLEX_TYPE_HALF,

    CLI_PORT_DUPLEX_TYPE_LAST,
} CLI_PORT_DUPLEX_TYPE_T;

typedef enum
{
    CLI_PORT_FC_TYPE_AUTO,
    CLI_PORT_FC_TYPE_OFF,
    CLI_PORT_FC_TYPE_ON,

    CLI_PORT_FC_TYPE_LAST,
} CLI_PORT_FC_TYPE_T;

typedef enum
{
    CLI_PORT_SPEED_TYPE_AUTO,
    CLI_PORT_SPEED_TYPE_10,
    CLI_PORT_SPEED_TYPE_100,

    CLI_PORT_SPEED_TYPE_LAST,
} CLI_PORT_SPEED_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   cli_cmd_port_setDuplex
 * PURPOSE:
 *      Execute port cmd to set duplex.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setDuplex(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id);

/* FUNCTION NAME:   cli_cmd_port_setEEE
 * PURPOSE:
 *      Execute port cmd to set EEE.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setEEE(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id);

/* FUNCTION NAME:   cli_cmd_port_setFlowcontrol
 * PURPOSE:
 *      Execute port cmd to set flow control.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setFlowcontrol(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id);

/* FUNCTION NAME:   cli_port_setAdminState
 * PURPOSE:
 *      Execute port cmd to set admin state.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_port_setAdminState(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id);

/* FUNCTION NAME:   cli_cmd_port_setSpeed
 * PURPOSE:
 *      Execute port cmd to set speed.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 *      port_id                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      use buffer size
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_setSpeed(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx,
    UI32_T port_id);

/* FUNCTION NAME:   cli_cmd_if_showStatus
 * PURPOSE:
 *      Display port current operating configuration.
 *
 * INPUT:
 *      ptr_out_buf            -- print buffer
 *      out_buf_len            -- print buffer size
 *      no_flag                -- no cmd flag
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 *      token_idx              -- token index
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_if_showStatus(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx);

/* FUNCTION NAME:   cli_cmd_if_show
 * PURPOSE:
 *      Display port status and packets statistics.
 *
 * INPUT:
 *      argc                   -- cmd string number
 *      pptr_argv              -- cmd string
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_if_show(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T  argc,
    C8_T **pptr_argv,
    I32_T  token_idx);

/* FUNCTION NAME:   cli_cmd_if_showCfg
 * PURPOSE:
 *      Show port config.
 *
 * INPUT:
 *      ptr_out_buf       -- print buffer
 *      out_buf_len       -- print buffer size
 *      conf_mode_flag    -- running-config/start-up config
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      output interface Gigabitethernet 1.
 */
UI32_T
cli_cmd_if_showCfg(
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag);

/* FUNCTION NAME:   cli_cmd_port_replacePortLable
 * PURPOSE:
 *      Repalce port-list string label.
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
cli_cmd_port_replacePortLable(
    void);

/* FUNCTION NAME:   cli_cmd_port_getMaxNum
 * PURPOSE:
 *      Get max port num which can config by cmd.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      _cli_max_port_num
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_getMaxNum(
    void);

/* FUNCTION NAME:   cli_cmd_port_showDuplexCfg
 * PURPOSE:
 *      Show port duplex config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showDuplexCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id);

/* FUNCTION NAME:   cli_cmd_port_showEEECfg
 * PURPOSE:
 *      Show port eee config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showEEECfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id);

/* FUNCTION NAME:   cli_cmd_port_showFcCfg
 * PURPOSE:
 *      Show port fc config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showFcCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id);

/* FUNCTION NAME:   cli_cmd_port_showAdminStateCfg
 * PURPOSE:
 *      Show port admin state config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showAdminStateCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id);

/* FUNCTION NAME:   cli_cmd_port_showSpeedCfg
 * PURPOSE:
 *      Show port speed config.
 *
 * INPUT:
 *      ptr_db_startup_head  -- port setting startup table head
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 *      conf_mode_flag       -- running-config/start-up config
 *      port                 -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
UI32_T
cli_cmd_port_showSpeedCfg(
    UI8_T *ptr_db_startup_head,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T  conf_mode_flag,
    UI8_T  port_id);

/* FUNCTION NAME:   cli_cmd_port_checkValid
 * PURPOSE:
 *      Check if port is not configurable.
 *
 * INPUT:
 *      port_id              -- port id
 *      ptr_out_buf          -- print buffer
 *      out_buf_len          -- print buffer size
 * OUTPUT:
 *      ptr_out_res          -- TRUE(valid), FALSE(invalid)
 * RETURN:
 *      None
 *
 * NOTES:
 *      1. Loop state port it's not allowed to be configured.
 */
UI32_T cli_cmd_port_checkValid(
    UI32_T port_id,
    C8_T  *ptr_out_buf,
    UI32_T out_buf_len,
    UI8_T *ptr_out_res);

#endif
