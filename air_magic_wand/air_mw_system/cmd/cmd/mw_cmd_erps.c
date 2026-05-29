/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_SUPPORT_ERPS
/* FILE NAME:  mw_cmd_erps.c
 * PURPOSE:
 *    This file contains the implementation of ERPS CMD.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_cmd_erps.h"
#include "erps.h"
#include "erps_timer.h"
#include "erps_gpio.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static MW_ERROR_NO_T
_mw_cmd_erps_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_erps_timer_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_erps_gpio_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx);

/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T _erps_cmd_vec[] =
{
    /* ERPS test cmds */
    {
        "dump info", 2, _mw_cmd_erps_dump,
        "erps dump info; Dump ERPS module info\r\n"
    },
    {
        "dump timer-info", 2, _mw_cmd_erps_timer_dump,
        "erps dump timer-info; Dump ERPS module timer info\r\n"
    },
    {
        "dump gpio-info", 2, _mw_cmd_erps_gpio_dump,
        "erps dump gpio-info; Dump ERPS module GPIO info\r\n"
    },
};

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _mw_cmd_erps_dump
 * PURPOSE:
 *      Function to dump ERPS information.
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
static MW_ERROR_NO_T
_mw_cmd_erps_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
    erps_dump();

    return MW_E_OK;
}

/* FUNCTION NAME: _mw_cmd_erps_timer_dump
 * PURPOSE:
 *      Function to dump ERPS module timer information.
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
static MW_ERROR_NO_T
_mw_cmd_erps_timer_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
    erps_timer_dump();

    return MW_E_OK;
}

/* FUNCTION NAME: _mw_cmd_erps_gpio_dump
 * PURPOSE:
 *      Function to dump ERPS module gpio information.
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
static MW_ERROR_NO_T
_mw_cmd_erps_gpio_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
    erps_gpio_dump();

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_erps_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: erps.
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
mw_cmd_erps_dispatcher(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _erps_cmd_vec, sizeof(_erps_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_erps_usager
 * PURPOSE:
 *      Command usage for magic wand command: erps.
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
mw_cmd_erps_usager(
    void)
{
    return (mw_cmd_usager(_erps_cmd_vec, sizeof(_erps_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

#endif /* AIR_SUPPORT_ERPS */