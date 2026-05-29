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
/* FILE NAME:   mw_cmd_log.c
 * PURPOSE:
 *      Implementation the commands for mw log module.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "osapi_string.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_CMD_LOG    "cmd_log"

static MW_ERROR_NO_T
_mw_cmd_log_show_info(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_mw_cmd_log_set_level(
    const C8_T *tokens[],
    UI32_T token_idx);

#ifdef AIR_SUPPORT_CLI
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
static MW_ERROR_NO_T
_mw_cmd_log_set_output(
    const C8_T *tokens[],
    UI32_T token_idx);
#endif
#endif
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T _mw_log_cmd_vec[] =
{
    {
        "show info", 2, _mw_cmd_log_show_info,
        "log show info: show the log information for all modules.\n"
    },
    {
        "set level", 2, _mw_cmd_log_set_level,
        "log set level module={ all | module_name } level={0 ~ 4}\r\n"
        "        all: set log level for all modules.\r\n"
        "        module_name: specify the module name to set log level, and it is not case-sensitive.\r\n"
        "        level: 0(OFF), 1(ERROR), 2(WARN), 3(INFO), 4(DEBUG)\n"
    },
#ifdef AIR_SUPPORT_CLI
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
    {
        "set output", 2, _mw_cmd_log_set_output,
        "log set output "
#ifdef AIR_SUPPORT_TELNET
        "[ telnet={ enable | disable } ]"
#endif
#ifdef AIR_SUPPORT_SSH
        "[ ssh={ enable | disable } ]"
#endif
        "\n"
    },
#endif
#endif
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T
_mw_cmd_log_show_info(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Parser tokens */
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = mw_log_show_settings();

    return ret;
}

static MW_ERROR_NO_T
_mw_cmd_log_set_level(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T log_level = 0, i = 0;
    C8_T module[MW_CMD_CMD_MAX_LENGTH];
    const C8_T *ptr_level_string[] = { "OFF", "ERR", "WRN", "INF", "DBG" };
    MW_LOG_SETTING_T *ptr_mw_log_settings = mw_log_get_setttings();

    if (NULL == ptr_mw_log_settings)
    {
        MW_CMD_OUTPUT("log settings is NULL\n");
        return MW_E_ENTRY_NOT_FOUND;
    }

    osapi_memset(module, 0, sizeof(module));
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "module", module), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "level", (UI32_T *)&log_level), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (0 == strcmp(module, "all"))
    {
        ret = mw_log_set_global_level((MW_LOG_LEVEL_T)log_level);
        if (MW_E_OK == ret)
        {
            MW_CMD_OUTPUT("Set all log level to %d(%s) successfully.\n", log_level, ptr_level_string[log_level]);
        }
    }
    else
    {
        mw_cmd_transStrToUpper(module);
        for (i = 0;i < MW_LOG_MODULE_LAST;i++)
        {
            if (0 == osapi_strcmp(ptr_mw_log_settings->ptr_module[i], module))
            {
                ret = mw_log_set_level(i, (MW_LOG_LEVEL_T)log_level);
                if (MW_E_OK == ret)
                {
                    MW_CMD_OUTPUT("Set log level for module %s to %d(%s) successfully.\n", module, log_level, ptr_level_string[log_level]);
                }
                break;
            }
        }
        if (MW_LOG_MODULE_LAST == i)
        {
            MW_CMD_OUTPUT("module %s not found.\n", module);
            ret = MW_E_ENTRY_NOT_FOUND;
        }
    }

    return ret;
}

#ifdef AIR_SUPPORT_CLI
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
static MW_ERROR_NO_T
_mw_cmd_log_set_output(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T output_bmp = 0;
    C8_T *ptr_arg_val = NULL;
    MW_LOG_SETTING_T *ptr_mw_log_settings = mw_log_get_setttings();
    BOOL_T is_known_token = FALSE;

    if (NULL == ptr_mw_log_settings)
    {
        MW_CMD_OUTPUT("log settings is NULL\n");
        return MW_E_ENTRY_NOT_FOUND;
    }

    /* Use one buffer for parsing values sequentially */
    ret = osapi_calloc(MW_CMD_CMD_MAX_LENGTH, MW_CMD_LOG, (void **)&ptr_arg_val);
    if (MW_E_OK != ret)
    {
        MW_CMD_OUTPUT("Failed to allocate memory, ret = %d\n", ret);
        return ret;
    }

    output_bmp = ptr_mw_log_settings->log_output_type;

    while ((NULL != tokens[token_idx]) && (MW_E_OK == ret))
    {
        is_known_token = FALSE;

#ifdef AIR_SUPPORT_TELNET
        if (0 == osapi_strcmp(tokens[token_idx], "telnet"))
        {
            is_known_token = TRUE;
            if (MW_E_OK == mw_cmd_getString(tokens, token_idx, "telnet", ptr_arg_val))
            {
                if (MW_E_OK == mw_cmd_checkString(ptr_arg_val, "enable"))
                {
                    output_bmp |= MW_LOG_OUTPUT_TYPE_BITMAP_TELNET;
                }
                else if (MW_E_OK == mw_cmd_checkString(ptr_arg_val, "disable"))
                {
                    output_bmp &= ~MW_LOG_OUTPUT_TYPE_BITMAP_TELNET;
                }
                else
                {
                    MW_CMD_OUTPUT("Invalid telnet value: %s\n", ptr_arg_val);
                    ret = MW_E_BAD_PARAMETER;
                }
            }
            else
            {
                MW_CMD_OUTPUT("Failed to get telnet value\n");
                ret = MW_E_BAD_PARAMETER;
            }
            token_idx += 2;
        }
#endif

#ifdef AIR_SUPPORT_SSH
        if ((FALSE == is_known_token) && (0 == osapi_strcmp(tokens[token_idx], "ssh")))
        {
            is_known_token = TRUE;
            if (MW_E_OK == mw_cmd_getString(tokens, token_idx, "ssh", ptr_arg_val))
            {
                if (MW_E_OK == mw_cmd_checkString(ptr_arg_val, "enable"))
                {
                    output_bmp |= MW_LOG_OUTPUT_TYPE_BITMAP_SSH;
                }
                else if (MW_E_OK == mw_cmd_checkString(ptr_arg_val, "disable"))
                {
                    output_bmp &= ~MW_LOG_OUTPUT_TYPE_BITMAP_SSH;
                }
                else
                {
                    MW_CMD_OUTPUT("Invalid ssh value: %s\n", ptr_arg_val);
                    ret = MW_E_BAD_PARAMETER;
                }
            }
            else
            {
                MW_CMD_OUTPUT("Failed to get ssh value\n");
                ret = MW_E_BAD_PARAMETER;
            }
            token_idx += 2;
        }
#endif

        if ((FALSE == is_known_token) && (MW_E_OK == ret))
        {
            MW_CMD_OUTPUT("Unknown parameter: %s\n", tokens[token_idx]);
            ret = MW_E_BAD_PARAMETER;
        }
    }

    MW_FREE(ptr_arg_val);

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (MW_E_OK == ret)
    {
        ret = mw_log_set_global_output_type(output_bmp);
    }

    return ret;
}
#endif
#endif
/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_log_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: log.
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
mw_cmd_log_dispatcher(
    const C8_T *tokens[],
    UI32_T                      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_log_cmd_vec, sizeof(_mw_log_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_log_usager
 * PURPOSE:
 *      Command usage for magic wand command: log.
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
mw_cmd_log_usager(
    void)
{
    return (mw_cmd_usager(_mw_log_cmd_vec, sizeof(_mw_log_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

