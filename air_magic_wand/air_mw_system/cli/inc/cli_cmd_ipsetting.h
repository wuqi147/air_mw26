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

/* FILE NAME:   cli_cmd_ipsetting.h
 * PURPOSE:
 *      Define the data structures, enums and APIs for the ipsetting module.
 * NOTES:
 *      None
 */

#ifndef CLI_CMD_IPSETTING_H
#define CLI_CMD_IPSETTING_H

#ifdef AIR_SUPPORT_CLI
/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: cli_cmd_ipSetting_setIp
 * PURPOSE:
 *      Management ip command handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      no_flag     -- Flag indicating whether the command is no.
 *      argc        -- Argument count.
 *      pptr_argv   -- Argument vector.
 *      token_idx   -- Token index.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_setIp(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

/* FUNCTION NAME: cli_cmd_ipSetting_SetGw
 * PURPOSE:
 *      IP default gateway command handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      no_flag     -- Flag indicating whether the command is no.
 *      argc        -- Argument count.
 *      pptr_argv   -- Argument vector.
 *      token_idx   -- Token index.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_SetGw(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx);

/* FUNCTION NAME: cli_cmd_ipSetting_showIp
 * PURPOSE:
 *      Management ip show handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      show_mode   -- Indicate show mode, 0: running-config, 1: startup-config.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_showIp(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode);

/* FUNCTION NAME: cli_cmd_ipSetting_showGw
 * PURPOSE:
 *      IP default gateway show handler.
 * INPUT:
 *      ptr_out_buf -- Output buffer for the command response.
 *      out_buf_len -- Length of the output buffer.
 *      show_mode   -- Indicate show mode, 0: running-config, 1: startup-config.
 * OUTPUT:
 *      None
 * RETURN:
 *      len         -- Length of the output buffer used.
 * NOTES:
 *      None
 */
UI32_T
cli_cmd_ipSetting_showGw(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode);

#endif /* AIR_SUPPORT_CLI */
#endif /* CLI_CMD_IPSETTING_H */