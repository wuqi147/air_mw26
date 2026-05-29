/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

#ifdef AIR_SUPPORT_IGMP_SNP
#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_igmp_snoop.h"
#include "db_data.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T
_mw_cmd_igmp_snp_enable(
    const C8_T  *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_igmp_snp_rpt_suppress(
    const C8_T  *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_igmp_snp_fast_leave(
    const C8_T  *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_igmp_snp_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx);

/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T
_mw_igmp_snp_cmd_vec[] =
{
    {
        "set admin", 2, _mw_cmd_igmp_snp_enable,
        "igsn set admin mode={ enable | disable }\n"
    },
    {
        "set rptsupp", 2, _mw_cmd_igmp_snp_rpt_suppress,
        "igsn set rptsupp mode={ enable | disable }\n"
    },
    {
        "set fastleave", 2, _mw_cmd_igmp_snp_fast_leave,
        "igsn set fastleave mode={ enable | disable }\n"
    },
    {
        "dump info", 2, _mw_cmd_igmp_snp_dump,
        "igsn dump info\n"
    },
};

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_mw_cmd_igmp_snp_enable(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
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
    MW_CMD_OUTPUT("Set Admin Mode: %d\r\n", mode);
    igmp_snp_cmd_setAdminState(mode);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_mw_cmd_igmp_snp_rpt_suppress(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
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
    MW_CMD_OUTPUT("Set Report-Suppress Mode: %d\r\n", mode);
    igmp_snp_cmd_setRptSuppressMode(mode);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_mw_cmd_igmp_snp_fast_leave(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
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
    MW_CMD_OUTPUT("Set Fast-Leave Mode: %d\r\n", mode);
    igmp_snp_cmd_setFastLeaveMode(mode);

    return MW_E_OK;
}

/* FUNCTION NAME: _mw_cmd_igmp_snp_dump
 * PURPOSE:
 *      Function to dump IGMP snooping information.
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
_mw_cmd_igmp_snp_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
    igmp_snp_cmd_dumpInfo();

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_igmp_snp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: igsn.
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
mw_cmd_igmp_snp_dispatcher(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_igmp_snp_cmd_vec, sizeof(_mw_igmp_snp_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_igmp_snp_usager
 * PURPOSE:
 *      Command usage for magic wand command: igsn.
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
mw_cmd_igmp_snp_usager(
    void)
{
    return (mw_cmd_usager(_mw_igmp_snp_cmd_vec, sizeof(_mw_igmp_snp_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}
#endif/*#ifdef AIR_SUPPORT_IGMP_SNP*/

