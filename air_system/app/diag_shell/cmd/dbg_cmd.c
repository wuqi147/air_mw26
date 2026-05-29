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

#include <cmd/dbg_cmd.h>

#include <air_error.h>
#include <air_module.h>
#include <air_types.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* DIAG_SET_MODULE_INFO(AIR_MODULE_DIAG, "dbg_cmd.c"); */

static AIR_ERROR_NO_T
_dbg_cmd_getEnable(
    C8_T   *str,
    BOOL_T *is_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_E_OK == dsh_checkString(str, "on"))
    {
        *is_enable = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "off"))
    {
        *is_enable = FALSE;
    }
    else
    {
        rc = DSH_E_SYNTAX_ERR;
    }

    return rc;
}

static AIR_ERROR_NO_T
_dbg_cmd_setCommonFlag(
    const C8_T  *tokens[],
    UI32_T       token_idx,
    AIR_MODULE_T module_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T         dbg_flag = 0;
    BOOL_T         is_enable = FALSE;

    if (dsh_getString(tokens, token_idx, "all", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_INFO | HAL_DBG_WARN | HAL_DBG_ERR;
    }
    else if (dsh_getString(tokens, token_idx, "info", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_INFO;
    }
    else if (dsh_getString(tokens, token_idx, "warn", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_WARN;
    }
    else if (dsh_getString(tokens, token_idx, "err", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_ERR;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    token_idx += 2;
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (rc == AIR_E_OK)
    {
        rc = _dbg_cmd_getEnable(str, &is_enable);
    }

    if (rc == AIR_E_OK)
    {
        rc = diag_setDebugFlag(module_id, dbg_flag, is_enable);
    }

    if (rc != AIR_E_OK)
    {
        osal_printf("***Error***, set %s debug flag fail.\n", air_module_getModuleName(module_id));
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dbg_cmd_setHalFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_HAL);
}

static AIR_ERROR_NO_T
_dbg_cmd_setAmlFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_AML);
}
static AIR_ERROR_NO_T
_dbg_cmd_setPortFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_PORT);
}

static AIR_ERROR_NO_T
_dbg_cmd_setL2Flag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_L2);
}

static AIR_ERROR_NO_T
_dbg_cmd_setMibFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_MIB);
}

static AIR_ERROR_NO_T
_dbg_cmd_setVlanFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_VLAN);
}

static AIR_ERROR_NO_T
_dbg_cmd_setMirFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_MIR);
}

static AIR_ERROR_NO_T
_dbg_cmd_setQosFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_QOS);
}

static AIR_ERROR_NO_T
_dbg_cmd_setSwcFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_SWC);
}

static AIR_ERROR_NO_T
_dbg_cmd_setMdioFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_MDIO);
}

static AIR_ERROR_NO_T
_dbg_cmd_setIfMonFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_IFMON);
}

static AIR_ERROR_NO_T
_dbg_cmd_setPhyFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T         module_id = AIR_MODULE_PHY;
    UI32_T         dbg_flag = 0;
    BOOL_T         is_enable = FALSE;

    if (dsh_getString(tokens, token_idx, "all", str) == AIR_E_OK)
    {
        dbg_flag = (HAL_DBG_INFO | HAL_DBG_WARN | HAL_DBG_ERR | HAL_DBG_PHY_CABLE_DIAG);
    }
    else if (dsh_getString(tokens, token_idx, "cable-diag", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_PHY_CABLE_DIAG;
    }
    else if (dsh_getString(tokens, token_idx, "info", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_INFO;
    }
    else if (dsh_getString(tokens, token_idx, "warn", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_WARN;
    }
    else if (dsh_getString(tokens, token_idx, "err", str) == AIR_E_OK)
    {
        dbg_flag = HAL_DBG_ERR;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    token_idx += 2;
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (rc == AIR_E_OK)
    {
        rc = _dbg_cmd_getEnable(str, &is_enable);
    }

    if (rc == AIR_E_OK)
    {
        rc = diag_setDebugFlag(module_id, dbg_flag, is_enable);
    }

    if (rc != AIR_E_OK)
    {
        osal_printf("***Error***, set PHY debug flag fail.\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dbg_cmd_setSifFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_SIF);
}

static AIR_ERROR_NO_T
_dbg_cmd_setPerifFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_PERIF);
}

static AIR_ERROR_NO_T
_dbg_cmd_setStpFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_STP);
}

static AIR_ERROR_NO_T
_dbg_cmd_setLagFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_LAG);
}

static AIR_ERROR_NO_T
_dbg_cmd_setIpmcFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_IPMC);
}

static AIR_ERROR_NO_T
_dbg_cmd_setSecFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_SEC);
}

static AIR_ERROR_NO_T
_dbg_cmd_setSflowFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_SFLOW);
}

static AIR_ERROR_NO_T
_dbg_cmd_setSvlanFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_SVLAN);
}

static AIR_ERROR_NO_T
_dbg_cmd_setStagFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_STAG);
}

static AIR_ERROR_NO_T
_dbg_cmd_setDosFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_DOS);
}

static AIR_ERROR_NO_T
_dbg_cmd_setAclFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_ACL);
}

#ifdef AIR_EN_POE
static AIR_ERROR_NO_T
_dbg_cmd_setPoeFlag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return _dbg_cmd_setCommonFlag(tokens, token_idx, AIR_MODULE_POE);
}
#endif

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _dbg_cmd_vec[] =
{
    {
        "set hal", 2, _dbg_cmd_setHalFlag,
        "dbg set hal { all | info | warn | err }={ on | off }\n"
    },
    {
        "set aml", 2, _dbg_cmd_setAmlFlag,
        "dbg set aml { all | info | warn | err }={ on | off }\n"
    },
    {
        "set port", 2, _dbg_cmd_setPortFlag,
        "dbg set port { all | info | warn | err }={ on | off }\n"
    },
    {
        "set l2", 2, _dbg_cmd_setL2Flag,
        "dbg set l2 { all | info | warn | err }={ on | off }\n"
    },
    {
        "set mib", 2, _dbg_cmd_setMibFlag,
        "dbg set mib { all | info | warn | err }={ on | off }\n"
    },
    {
        "set vlan", 2, _dbg_cmd_setVlanFlag,
        "dbg set vlan { all | info | warn | err }={ on | off }\n"
    },
    {
        "set mirror", 2, _dbg_cmd_setMirFlag,
        "dbg set mirror { all | info | warn | err }={ on | off }\n"
    },
    {
        "set qos", 2, _dbg_cmd_setQosFlag,
        "dbg set qos { all | info | warn | err }={ on | off }\n"
    },
    {
        "set swc", 2, _dbg_cmd_setSwcFlag,
        "dbg set swc { all | info | warn | err }={ on | off }\n"
    },
    {
        "set mdio", 2, _dbg_cmd_setMdioFlag,
        "dbg set mdio { all | info | warn | err }={ on | off }\n"
    },
    {
        "set ifmon", 2, _dbg_cmd_setIfMonFlag,
        "dbg set ifmon { all | info | warn | err }={ on | off }\n"
    },
    {
        "set phy", 2, _dbg_cmd_setPhyFlag,
        "dbg set phy { all | info | warn | err | cable-diag }={ on | off }\n"
    },
    {
        "set sif", 2, _dbg_cmd_setSifFlag,
        "dbg set sif { all | info | warn | err }={ on | off }\n"
    },
    {
        "set perif", 2, _dbg_cmd_setPerifFlag,
        "dbg set perif { all | info | warn | err }={ on | off }\n"
    },
    {
        "set stp", 2, _dbg_cmd_setStpFlag,
        "dbg set stp { all | info | warn | err }={ on | off }\n"
    },
    {
        "set lag", 2, _dbg_cmd_setLagFlag,
        "dbg set lag { all | info | warn | err }={ on | off }\n"
    },
    {
        "set ipmc", 2, _dbg_cmd_setIpmcFlag,
        "dbg set ipmc { all | info | warn | err }={ on | off }\n"
    },
    {
        "set sec", 2, _dbg_cmd_setSecFlag,
        "dbg set sec { all | info | warn | err }={ on | off }\n"
    },
    {
        "set sflow", 2, _dbg_cmd_setSflowFlag,
        "dbg set sflow { all | info | warn | err }={ on | off }\n"
    },
    {
        "set svlan", 2, _dbg_cmd_setSvlanFlag,
        "dbg set svlan { all | info | warn | err }={ on | off }\n"
    },
    {
        "set stag", 2, _dbg_cmd_setStagFlag,
        "dbg set stag { all | info | warn | err }={ on | off }\n"
    },
    {
        "set dos", 2, _dbg_cmd_setDosFlag,
        "dbg set dos { all | info | warn | err }={ on | off }\n"
    },
    {
        "set acl", 2, _dbg_cmd_setAclFlag,
        "dbg set acl { all | info | warn | err }={ on | off }\n"
    },
#ifdef AIR_EN_POE
    {
        "set poe", 2, _dbg_cmd_setPoeFlag,
        "dbg set poe { all | info | warn | err }={ on | off }\n"
    },
#endif
};
/* clang-format on */

AIR_ERROR_NO_T
dbg_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _dbg_cmd_vec, sizeof(_dbg_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
dbg_cmd_usager()
{
    return (dsh_usager(_dbg_cmd_vec, sizeof(_dbg_cmd_vec) / sizeof(DSH_VEC_T)));
}
