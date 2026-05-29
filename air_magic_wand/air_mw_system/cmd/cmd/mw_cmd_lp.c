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

/* FILE NAME:  mw_cmd_lp.c
 * PURPOSE:
 *    This file contains the implementation of loop prevention command.
 *
 * NOTES:
 *
 */
#ifdef AIR_SUPPORT_LP

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_types.h"
#include "osapi.h"
#include "osapi_string.h"
#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_lp.h"
#include "lp_cmd.h"

/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T
_lp_cmd_getLoopInfo(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_lp_cmd_setLoopPrevention(
    const C8_T *tokens[],
    UI32_T token_idx);

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T
_mw_lp_cmd_vec[] =
{
    {
        "set admin", 2, _lp_cmd_setLoopPrevention,
        "lp set admin mode={ enable | disable }\n"
    },
    {
        "show info", 2, _lp_cmd_getLoopInfo,
        "lp show info\n"
    },
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T
_lp_cmd_getLoopInfo(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Parser tokens */
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = lp_cmd_getLoopInfo();

    return ret;
}

static MW_ERROR_NO_T
_lp_cmd_setLoopPrevention(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    C8_T st[MW_CMD_CMD_MAX_LENGTH] = {0};
    BOOL_T mode = 0;

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "mode", st), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(st, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(st, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = lp_cmd_setLoopAdminState(mode);

    return ret;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_lp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: lp.
 *
 * INPUT:
 *      tokens      --  Command tokens
 *      token_idx   --  The index of 1st valid token
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_cmd_lp_dispatcher(
    const C8_T                  *tokens[],
    UI32_T                      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_lp_cmd_vec, sizeof(_mw_lp_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_lp_usager
 * PURPOSE:
 *      Command usage for magic wand command: lp.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_cmd_lp_usager(
    void)
{
    return (mw_cmd_usager(_mw_lp_cmd_vec, sizeof(_mw_lp_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}
#endif /* AIR_SUPPORT_LP */
