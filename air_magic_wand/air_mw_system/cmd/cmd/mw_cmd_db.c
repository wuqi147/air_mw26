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

#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_db.h"
#include "db_util.h"
#include "db_cfgfile.h"

/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T _db_cmd_clrCfgFile(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _db_cmd_getCfgFile(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _db_cmd_setCfgFile(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _db_cmd_dumpRaw(const C8_T *tokens[], UI32_T token_idx);
#ifdef DB_SUPPORT_RESTORE_METHOD
static MW_ERROR_NO_T
_db_cmd_restoreTable(
    const C8_T *tokens[],
    UI32_T token_idx);
#endif
static MW_ERROR_NO_T _db_cmd_dumpSub(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _db_cmd_dumpCtrl(const C8_T *tokens[], UI32_T token_idx);

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T _mw_db_cmd_vec[] =
{
    {
        "clear cfg", 2, _db_cmd_clrCfgFile,
        "db clear cfg cfg={ startup | factory }\n"
    },
    {
        "show cfg", 2, _db_cmd_getCfgFile,
        "db show cfg cfg={ startup | factory }\n"
    },
    {
        "save cfg", 2, _db_cmd_setCfgFile,
        "db save cfg cfg={ startup | factory }\n"
    },
    {
        "dump table", 2, _db_cmd_dumpRaw,
        "db dump table [ cfg={ startup | factory | running | default } ] table=<UINT> [ field=<UINT> ]\n"
    },
#ifdef DB_SUPPORT_RESTORE_METHOD
    {
        "restore table", 2, _db_cmd_restoreTable,
        "db restore table table=<UINT> [ field=<UINT> ] [ entry=<UINT> ]\n"
    },
#endif
    {
        "dump sub-info", 2, _db_cmd_dumpSub,
        "db dump sub-info\n"
    },
    {
        "dump ctrl-info", 2, _db_cmd_dumpCtrl,
        "db dump ctrl-info [ detail={ 0(ALL) | 1(Table Size) } ]\n"
    }
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static MW_ERROR_NO_T
_db_cmd_clrCfgFile(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    BOOL_T showstartup = TRUE;
    C8_T cfg[MW_CMD_CMD_MAX_LENGTH] = {0};

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "cfg", cfg), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(cfg, "startup"))
    {
        showstartup = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "factory"))
    {
        showstartup = FALSE;
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = dbapi_clrCfgFile(showstartup);
    return ret;
}

static MW_ERROR_NO_T
_db_cmd_getCfgFile(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    BOOL_T showstartup = TRUE;
    C8_T cfg[MW_CMD_CMD_MAX_LENGTH] = {0};

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "cfg", cfg), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(cfg, "startup"))
    {
        showstartup = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "factory"))
    {
        showstartup = FALSE;
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = dbapi_showCfgFile(showstartup);
    return ret;
}

static MW_ERROR_NO_T
_db_cmd_setCfgFile(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    BOOL_T savestartup = TRUE;
    C8_T cfg[MW_CMD_CMD_MAX_LENGTH] = {0};

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "cfg", cfg), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(cfg, "startup"))
    {
        savestartup = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "factory"))
    {
        savestartup = FALSE;
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = dbapi_saveCfgFile(savestartup);
    return ret;
}

static MW_ERROR_NO_T
_db_cmd_dumpRaw(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    UI8_T cfg_type = DB_CFG_TYPE_RUNNING;
    UI32_T t_idx = TABLES_LAST;
    UI32_T f_idx = DB_ALL_FIELDS;
    C8_T cfg[MW_CMD_CMD_MAX_LENGTH] = {0};

    /* Parser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getString(tokens, token_idx, "cfg", cfg), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "table", &t_idx), tokens, 2);
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "field", &f_idx), tokens, 2);
    if(MW_E_OK == mw_cmd_checkString(cfg, "startup"))
    {
        cfg_type = DB_CFG_TYPE_STARTUP;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "factory"))
    {
        cfg_type = DB_CFG_TYPE_FACTORY;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "running"))
    {
        cfg_type = DB_CFG_TYPE_RUNNING;
    }
    else if(MW_E_OK == mw_cmd_checkString(cfg, "default"))
    {
        cfg_type = DB_CFG_TYPE_DEFAULT;
    }

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    return dbapi_dumpRaw(cfg_type, t_idx, f_idx);
}

#ifdef DB_SUPPORT_RESTORE_METHOD
static MW_ERROR_NO_T
_db_cmd_restoreTable(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    UI32_T t_idx = 0;
    UI32_T f_idx = DB_ALL_FIELDS, e_idx = DB_ALL_ENTRIES;

    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "table", &t_idx), tokens, 2);
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "field", &f_idx), tokens, 2);
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "entry", &e_idx), tokens, 2);

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    return dbapi_restoreTable(t_idx, f_idx, e_idx);
}
#endif /* DB_SUPPORT_RESTORE_METHOD */

static MW_ERROR_NO_T
_db_cmd_dumpSub(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
    dbapi_dumpSubTree();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_db_cmd_dumpCtrl(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    UI32_T detail = 0;

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "detail", &detail), tokens, 2);
    if(DB_CTRL_DUMP_LEVEL_LAST <= detail)
    {
        return MW_E_BAD_PARAMETER;
    }
    return dbapi_dumpTableCtrl((UI8_T) detail);
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_db_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: DB.
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
mw_cmd_db_dispatcher(
    const C8_T                  *tokens[],
    UI32_T                      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_db_cmd_vec, sizeof(_mw_db_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_db_usager
 * PURPOSE:
 *      Command usage for magic wand command: DB.
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
mw_cmd_db_usager(
    void)
{
    return (mw_cmd_usager(_mw_db_cmd_vec, sizeof(_mw_db_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

