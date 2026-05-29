/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

#include <mw_error.h>
#include <mw_types.h>

#include <osapi.h>
#include <osapi_string.h>

#include <mw_cmd_parser.h>
#include <mw_cmd_util.h>
#include <mw_cmd_stp.h>

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#include <stp.h>
#include <stp_db.h>

#ifdef AIR_SUPPORT_RSTP
#include <rstp-sys.h>
#include "../../switching/stp/rstp/ovs/lib/rstp.h"
#endif

#ifdef AIR_SUPPORT_MSTP
#include <mstp_sys.h>
#endif
/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T
_stp_cmd_setGlobalState(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setForceVersion(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setForwardDelay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setMaxAge(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setTransmitHoldCount(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setBridgePriority(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setPortPriority(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setPortCost(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setPortAdminEdge(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setPortMcheck(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_setPortEnableStatus(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_stp_cmd_set_auto_cost_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_auto_cost_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif
#ifdef AIR_SUPPORT_STP_UNIFIED_SA
static MW_ERROR_NO_T
_stp_cmd_set_unified_sa_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif /* AIR_SUPPORT_STP_UNIFIED_SA */
#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
static MW_ERROR_NO_T
_stp_cmd_set_clear_mc_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif /* AIR_SUPPORT_STP_CLEAR_MULTICAST */
#ifdef AIR_SUPPORT_RSTP_SECURITY
static MW_ERROR_NO_T
_stp_cmd_set_bpdu_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_interval(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_threshold(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_root_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_set_loop_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_stp_cmd_showSecInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif

#ifdef AIR_SUPPORT_MSTP
static MW_ERROR_NO_T
_mstp_cmd_setRegionConfig(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#ifdef STP_DEBUG
static MW_ERROR_NO_T
_mstp_cmd_setInstanceVlan(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif
static MW_ERROR_NO_T
_mstp_cmd_showMcidInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif
static MW_ERROR_NO_T
_stp_cmd_showInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#ifdef STP_DEBUG
static MW_ERROR_NO_T
_stp_cmd_showPortParam(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

#ifdef AIR_SUPPORT_RSTP
static MW_ERROR_NO_T
_stp_cmd_showOvsInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_stp_cmd_showBridgeParam(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_stp_cmd_stm_dbg_port(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif
static MW_ERROR_NO_T
_stp_cmd_showUsedEntry(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif /* STP_DEBUG */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T _mw_stp_cmd_vec[] =
{
    {
        "set global-ctrl", 2, _stp_cmd_setGlobalState,
        "stp set global-ctrl mode={ enable | disable }\n"
    },
    {
        "set force-version", 2, _stp_cmd_setForceVersion,
        "stp set force-version version={ stp | rstp | mstp}\n"
    },
    {
        "set forward-delay", 2, _stp_cmd_setForwardDelay,
        "stp set forward-delay time=<UINT>\n"
    },
    {
        "set max-age", 2, _stp_cmd_setMaxAge,
        "stp set max-age time=<UINT>\n"
    },
    {
        "set transmit-hold-count", 2, _stp_cmd_setTransmitHoldCount,
        "stp set transmit-hold-count value=<UINT>\n"
    },
    {
        "set bridge-priority", 2, _stp_cmd_setBridgePriority,
#ifdef AIR_SUPPORT_MSTP
        "stp set bridge-priority [instance=<UINT>] priority=<UINT>\n"
#else
        "stp set bridge-priority priority=<UINT>\n"
#endif
    },
    {
        "set port-priority", 2, _stp_cmd_setPortPriority,
#ifdef AIR_SUPPORT_MSTP
        "stp set port-priority [instance=<UINT>] portlist=<UINTLIST> priority=<UINT>\n"
#else
        "stp set port-priority portlist=<UINTLIST> priority=<UINT>\n"
#endif
    },
    {
        "set path-cost", 2, _stp_cmd_setPortCost,
#ifdef AIR_SUPPORT_MSTP
        "stp set path-cost [instance=<UINT>] portlist=<UINTLIST> cost=<UINT>\n"
#else
        "stp set path-cost portlist=<UINTLIST> cost=<UINT>\n"
#endif
    },
    {
        "set admin-edge", 2, _stp_cmd_setPortAdminEdge,
        "stp set admin-edge portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "set mcheck-perform", 2, _stp_cmd_setPortMcheck,
        "stp set mcheck-perform portlist=<UINTLIST>\n"
    },
    {
        "set port-ctrl", 2, _stp_cmd_setPortEnableStatus,
        "stp set port-ctrl portlist=<UINTLIST> mode={ enable | disable }\n"
    },
#ifdef AIR_SUPPORT_STP_AUTO_COST
    {
        "set auto-cost", 2, _stp_cmd_set_auto_cost_mode,
        "stp set auto-cost portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "set auto-cost-support", 2, _stp_cmd_set_auto_cost_support_mode,
        "stp set auto-cost-support mode={ enable | disable }\n"
    },
#endif
#ifdef AIR_SUPPORT_STP_UNIFIED_SA
    {
        "set unified-sa-support", 2, _stp_cmd_set_unified_sa_support_mode,
        "stp set unified-sa-support mode={ enable | disable }\n"
    },
#endif /* AIR_SUPPORT_STP_UNIFIED_SA */
#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
    {
        "set clear-mc-support", 2, _stp_cmd_set_clear_mc_support_mode,
        "stp set clear-mc-support mode={ enable | disable }\n"
    },
#endif /* AIR_SUPPORT_STP_CLEAR_MULTICAST */
#ifdef AIR_SUPPORT_RSTP_SECURITY
    {
        "set bpdu-guard-ctrl", 2, _stp_cmd_set_bpdu_guard_state,
        "stp set bpdu-guard-ctrl portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "set tc-protect-ctrl", 2, _stp_cmd_set_tc_guard_state,
        "stp set tc-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "set tc-protect-interval", 2, _stp_cmd_set_tc_guard_interval,
        "stp set tc-protect-interval portlist=<UINTLIST> interval=<UINT>\n"
    },
    {
        "set tc-protect-threshold", 2, _stp_cmd_set_tc_guard_threshold,
        "stp set tc-protect-threshold portlist=<UINTLIST> threshold=<UINT>\n"
    },
    {
        "set root-protect-ctrl", 2, _stp_cmd_set_root_guard_state,
        "stp set root-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "set loop-protect-ctrl", 2, _stp_cmd_set_loop_guard_state,
        "stp set loop-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show security-info", 2, _stp_cmd_showSecInfo,
        "stp show security-info\n"
    },
#endif /* AIR_SUPPORT_STP_SECURITY */
    {
        "show info", 2, _stp_cmd_showInfo,
#ifdef AIR_SUPPORT_MSTP
        "stp show info [instance=<UINT>]\n"
#else
        "stp show info\n"
#endif
    },
#ifdef AIR_SUPPORT_MSTP
    {
        "show mcid-info", 2, _mstp_cmd_showMcidInfo,
        "stp show mcid-info\n"
    },
    {
        "set region-config", 2, _mstp_cmd_setRegionConfig,
        "stp set region-config region-name={} revision-id=<UINT>\n"
    },
#ifdef STP_DEBUG
    {
        "set instance-vlan-mapping", 2, _mstp_cmd_setInstanceVlan,
        "stp set instance-vlan-mapping instance=<UINT> vlanlist=<UINTLIST>\n"
    },
#endif
#endif
#ifdef STP_DEBUG
    {
        "show port-param", 2, _stp_cmd_showPortParam,
#ifdef AIR_SUPPORT_MSTP
        "stp show port-param portlist=<UINTLIST> [fid=<UINT>]\n"
#else
        "stp show port-param portlist=<UINTLIST>\n"
#endif
    },
#ifdef AIR_SUPPORT_RSTP
    {
        "show ovs-info", 2, _stp_cmd_showOvsInfo,
        "stp show ovs-info\n"
    },
    {
        "show bridge-param", 2, _stp_cmd_showBridgeParam,
        "stp show bridge-param\n",
    },
#endif
    {
        "debug-sm-port", 1, _stp_cmd_stm_dbg_port,
        "stp debug-sm-port portlist <UINTLIST> state=< on | off>\n"
    },
    {
        "show used-entry", 2, _stp_cmd_showUsedEntry,
        "stp show used-entry\n"
    },
#endif
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
/* cmd: stp set global-state state={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_setGlobalState(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    C8_T            state_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T           state = 0;

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", state_str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(state_str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(state_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (MW_E_OK == rc)
    {
        rc = stp_cmd_setEnable(state);
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_cmd_setForceVersion(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T     rc = MW_E_OK;
    C8_T              version_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T             version = 0;

    /* Parser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "version", version_str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(version_str, "stp"))
    {
        version = STP_VERSION_COMPATIBILITY;
    }
    else if(MW_E_OK == mw_cmd_checkString(version_str, "rstp"))
    {
        version = STP_VERSION_DEFAULT;
    }
    else if(MW_E_OK == mw_cmd_checkString(version_str, "mstp"))
    {
        version = STP_VERSION_MSTP;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (MW_E_OK == rc)
    {
        rc = stp_cmd_setForceVersion(version);
    }

    return rc;
}

/* cmd: stp set forward-delay time=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setForwardDelay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          time = 0;
    UI16_T          max_age = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "time", &time), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if (STP_MIN_BRIDGE_FORWARD_DELAY <= time &&
        STP_MAX_BRIDGE_FORWARD_DELAY >= time)
    {
        rc = stp_cmd_getMaxAge(&max_age);
        if (MW_E_OK == rc)
        {
            if ((2 * (time - 1)) >= max_age)
            {
                rc = stp_cmd_setForwardDelay(time);
            }
            else
            {
                rc = MW_E_BAD_PARAMETER;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set max-age time=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setMaxAge(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          time = 0;
    UI16_T          fwd_delay = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "time", &time), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if (STP_MIN_BRIDGE_MAX_AGE <= time &&
        STP_MAX_BRIDGE_MAX_AGE >= time)
    {
        rc = stp_cmd_getForwardDelay(&fwd_delay);
        if (MW_E_OK == rc)
        {
            if ((2 * (fwd_delay - 1)) >= time)
            {
                rc = stp_cmd_setMaxAge(time);
            }
            else
            {
                rc = MW_E_BAD_PARAMETER;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set transmit-hold-count value=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setTransmitHoldCount(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          count = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "value", &count), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if (STP_MIN_TRANSMIT_HOLD_COUNT <= count &&
        STP_MAX_TRANSMIT_HOLD_COUNT >= count)
    {
        rc = stp_cmd_setTransmitHoldCount(count);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set bridge-priority [instance=<UINT>] priority=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setBridgePriority(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          priority = 0;
    UI32_T          instance = 0;
    BOOL_T          exist_ins = FALSE;

    if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "instance", &instance))
    {
        token_idx += 2;
        exist_ins = TRUE;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "priority", osapi_strlen("priority")))
    {
        if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "priority", &priority))
        {
            token_idx += 2;
        }
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (STP_MIN_PRIORITY <= priority &&
        STP_MAX_PRIORITY >= priority &&
        0 == (priority % STP_PRIORITY_STEP))
    {
        rc = stp_cmd_setBridgePriority(priority, instance, exist_ins);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set port-priority [instance=<UINT>] portlist=<UINTLIST> priority=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setPortPriority(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI32_T              priority = 0;
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI32_T              instance = 0;
    BOOL_T              exist_ins = FALSE;

    if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "instance", &instance))
    {
        token_idx += 2;
        exist_ins = TRUE;
    }

    if (MW_E_OK == mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist))
    {
        token_idx += 2;
        if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "priority", &priority))
        {
            token_idx += 2;
        }
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (STP_MIN_PORT_PRIORITY <= priority &&
        STP_MAX_PORT_PRIORITY >= priority &&
        0 == (priority % STP_STEP_PORT_PRIORITY))
    {
        MW_PORT_FOREACH(portlist, port)
        {
            rc = stp_cmd_setPortPriority(port, priority, instance, exist_ins);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: set path-cost [instance=<UINT>] portlist=<UINTLIST> cost=<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_setPortCost(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI32_T              cost = 0;
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI32_T              instance = 0;
    BOOL_T              exist_ins = FALSE;

    if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "instance", &instance))
    {
        token_idx += 2;
        exist_ins = TRUE;
    }

    if (MW_E_OK == mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist))
    {
        token_idx += 2;
        if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "cost", &cost))
        {
            token_idx += 2;
        }
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (STP_MIN_PORT_PATH_COST <= cost &&
        STP_MAX_PORT_PATH_COST >= cost)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            rc = stp_cmd_setPortCost(port, cost, instance, exist_ins);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_cmd_setPortAdminEdge(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                mode_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               mode = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", mode_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = stp_cmd_setPortAdminEdge(port, mode);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    return rc;
}

/* cmd: stp set mcheck-perform portlist=<UINTLIST> perform
*/
static MW_ERROR_NO_T
_stp_cmd_setPortMcheck(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI32_T              perform = TRUE;
    UI16_T              port = 0;
    UI32_T              unit = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if (TRUE == perform)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = stp_cmd_setPortMCheck(port, perform);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }

    return rc;
}


/* cmd: stp set port-enable-status portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_setPortEnableStatus(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               status = FALSE;
    C8_T                status_str[MW_CMD_CMD_MAX_LENGTH] = {0};

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", status_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    /* Parser tokens */
    if(MW_E_OK == mw_cmd_checkString(status_str, "enable"))
    {
        status = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(status_str, "disable"))
    {
        status = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = stp_cmd_setPortEnableStatus(port, status);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* cmd: stp set auto-cost portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_auto_cost_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                mode_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               mode = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", mode_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = stp_cmd_setPortAutoCostMode(port, mode);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    return rc;
}

static MW_ERROR_NO_T
_stp_cmd_set_auto_cost_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    C8_T                mode_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T               mode = 0;
    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", mode_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        rc = stp_cmd_setAutoCostSupportMode(mode);
    }
    return rc;
}
#endif

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
/* cmd rstp set unified-sa-support-mode mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_unified_sa_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    C8_T                mode_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T               mode = 0;
    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", mode_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        rc = stp_cmd_setUnifiedSASupportMode(mode);
    }
    return rc;
}
#endif

#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
/* cmd rstp set clear-mc-support-mode mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_clear_mc_support_mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    C8_T                mode_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T               mode = 0;
    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", mode_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        rc = stp_cmd_setClearMulticastSupportMode(mode);
    }
    return rc;
}
#endif /* AIR_SUPPORT_STP_CLEAR_MULTICAST */

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* cmd: stp set bpdu-guard-ctrl portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_bpdu_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                status_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               state = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", status_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(status_str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(status_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = rstp_cmd_setPortBpduGuardState(port, state);
            {
                break;
            }
        }
    }
    return rc;
}

/* cmd: stp set tc-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                status_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               state = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", status_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(status_str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(status_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = rstp_cmd_setPortTcGuardState(port, state);
            {
                break;
            }
        }
    }
    return rc;
}

/* cmd: stp set tc-protect-interval portlist=<UINTLIST> interval==<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_interval(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI32_T              interval = 0;
    UI16_T              cfg_interval;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "interval", &interval), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if ((RSTP_MIN_SEC_INTERVAL <= interval) &&
        (RSTP_MAX_SEC_INTERVAL >= interval))
    {
        cfg_interval = interval;
        MW_PORT_FOREACH(portlist, port)
        {
            rc = rstp_cmd_setPortTcGuardInterval(port, cfg_interval);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set tc-protect-threshold portlist=<UINTLIST> threshold==<UINT>
*/
static MW_ERROR_NO_T
_stp_cmd_set_tc_guard_threshold(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI32_T              threshold = 0;
    UI16_T              cfg_threshold;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "threshold", &threshold), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if ((RSTP_MIN_SEC_THRESHOLD <= threshold) &&
        (RSTP_MAX_SEC_THRESHOLD >= threshold))
    {
        cfg_threshold = threshold;
        MW_PORT_FOREACH(portlist, port)
        {
            rc = rstp_cmd_setPortTcGuardThreshold(port, cfg_threshold);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* cmd: stp set root-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_root_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                status_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               state = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", status_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(status_str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(status_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = rstp_cmd_setPortRootGuardState(port, state);
            {
                break;
            }
        }
    }
    return rc;
}

/* cmd: stp set loop-protect-ctrl portlist=<UINTLIST> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_stp_cmd_set_loop_guard_state(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    C8_T                status_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI16_T              port = 0;
    UI32_T              unit = 0;
    UI8_T               state = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "mode", status_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(status_str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(status_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    if (MW_E_OK == rc)
    {
        MW_PORT_FOREACH(portlist, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            rc = rstp_cmd_setPortLoopGuardState(port, state);
            {
                break;
            }
        }
    }
    return rc;
}

/* cmd: stp show security info
*/
static MW_ERROR_NO_T
_stp_cmd_showSecInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    rstp_cmd_showSecInfo();
    return MW_E_OK;
}
#endif

#ifdef AIR_SUPPORT_MSTP

/* cmd: mstp set region name
*/
static MW_ERROR_NO_T
_mstp_cmd_setRegionConfig(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    C8_T                name[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI32_T              len;
    UI32_T              revision;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "region-name", name), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "revision-id", &revision), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    len = osapi_strlen(name);
    if ((0 == len) || (len > 32))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (MSTP_REVISION_INVALID(revision))
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = mstp_cmd_setRegionConfig((const UI8_T *)name, revision);
    return rc;
}

#ifdef STP_DEBUG
/* cmd: mstp set instance-vlan
*/
static MW_ERROR_NO_T
_mstp_cmd_setInstanceVlan(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_CMD_RANGE_T      range;
    UI32_T              instance, i;
    C8_T                vlanlist[MW_CMD_CMD_MAX_LENGTH];

    osapi_memset(vlanlist, 0, MW_CMD_CMD_MAX_LENGTH);
    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(ptr_tokens, token_idx, "instance", &instance), token_idx, 2);
    if (MW_CMD_CMD_MAX_LENGTH <= osapi_strlen(ptr_tokens[token_idx + 1]))
    {
        return MW_E_BAD_PARAMETER;
    }
    osapi_strncpy(vlanlist, ptr_tokens[token_idx + 1], MW_CMD_CMD_MAX_LENGTH);
    MW_CMD_CHECK_PARAM(mw_cmd_getRangeList(ptr_tokens, token_idx, "vlanlist", &range), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    for (i = 0; i < MW_CMD_MAX_RANGE_NUM; i++)
    {
        if (0 == range.start[i])
        {
            return MW_E_BAD_PARAMETER;
        }
        /* 0 & 0xffffffff is getrangelist()'s initial value */
        if ((0xffffffff == range.start[i]) && (0 == range.end[i]))
        {
            break;
        }
        if (MSTP_VLAN_INVALID(range.start[i]) || MSTP_VLAN_INVALID(range.end[i]))
        {
            return MW_E_BAD_PARAMETER;
        }
    }

    if (MSTP_INSTANCE_INVALID(instance))
    {
        MW_CMD_OUTPUT("Instance range is 0~4095\n");
        return MW_E_BAD_PARAMETER;
    }

    return mstp_cmd_setInstanceVlan(instance, (const UI8_T *)vlanlist);
}
#endif

/* cmd: mstp show mcid info
*/
static MW_ERROR_NO_T
_mstp_cmd_showMcidInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    mstp_cmd_showMcidInfo();
    return MW_E_OK;
}
#endif

/* cmd: stp show info
*/
static MW_ERROR_NO_T
_stp_cmd_showInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T  instance = 0;
    BOOL_T  exsit_fid = FALSE;

#ifdef AIR_SUPPORT_MSTP
    /* Parser ptr_tokens */
    if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "instance", &instance))
    {
        token_idx += 2;
        exsit_fid = TRUE;
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if (MSTP_INSTANCE_INVALID(instance))
    {
        return MW_E_BAD_PARAMETER;
    }
#endif

    stp_cmd_showInfo(instance, exsit_fid);
    return MW_E_OK;
}

#ifdef STP_DEBUG
static MW_ERROR_NO_T
_stp_cmd_showPortParam(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI32_T              port = 0;
    UI32_T              unit = 0;
    UI32_T              fid = 0;
    BOOL_T              exist_ins = FALSE;

    /* Parser ptr_tokens */
    if (0 == osapi_strncmp(ptr_tokens[token_idx], "portlist", osapi_strlen("portlist")))
    {
        if (MW_E_OK == mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist))
        {
            token_idx += 2;
            if (MW_E_OK == mw_cmd_getUint(ptr_tokens, token_idx, "fid", &fid))
            {
                token_idx += 2;
                exist_ins = TRUE;
            }
        }
    }
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    MW_PORT_FOREACH(portlist, port)
    {
        if (exist_ins)
        {
#ifdef AIR_SUPPORT_MSTP
            rc = mstp_cmd_showInstancePort(port, fid);
#endif
        }
        else
        {
#ifdef AIR_SUPPORT_RSTP
            rc = rstp_cmd_showPortParameters(port);
#endif
        }

        if (MW_E_OK != rc)
        {
            break;
        }
    }

    return rc;
}

#ifdef AIR_SUPPORT_RSTP
static MW_ERROR_NO_T
_stp_cmd_showOvsInfo(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    rstp_cmd_showOvsInfo();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_stp_cmd_showBridgeParam(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    rc = rstp_cmd_showBridgeParameters();

    return rc;
}
#endif

static MW_ERROR_NO_T
_stp_cmd_stm_dbg_port(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    MW_PORT_BITMAP_T    portlist = {0};
    UI32_T              port = 0;
    UI32_T              unit = 0;
    C8_T                state_str[MW_CMD_CMD_MAX_LENGTH] = {0};
    UI8_T               state = 0;

    /* Parser ptr_tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(ptr_tokens, token_idx, "state", state_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(state_str, "on"))
    {
        state = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(state_str, "off"))
    {
        state = FALSE;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    MW_PORT_FOREACH(portlist, port)
    {
        rc = stp_cmd_stm_port_debug(port, state);
        if (MW_E_OK != rc)
        {
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_cmd_showUsedEntry(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    /* Parser ptr_tokens */
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    stp_cmd_showUsedEntry();
    return MW_E_OK;
}

#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_stp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: RSTP.
 *
 * INPUT:
 *      ptr_tokens  --  Command tokens
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
mw_cmd_stp_dispatcher(
    const C8_T *ptr_tokens[],
    UI32_T     token_idx)
{
    return (mw_cmd_dispatcher(ptr_tokens, token_idx, _mw_stp_cmd_vec, sizeof(_mw_stp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_stp_usager
 * PURPOSE:
 *      Command usage for magic wand command: RSTP.
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
mw_cmd_stp_usager(
    void)
{
    return (mw_cmd_usager(_mw_stp_cmd_vec, sizeof(_mw_stp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}
#endif /* AIR_SUPPORT_RSTP */
