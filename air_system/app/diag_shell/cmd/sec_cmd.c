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

#include <cmd/sec_cmd.h>

#include <air_error.h>
#include <air_port.h>
#include <air_sec.h>
#include <air_types.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define _SEC_CHECK_FLAG_STRING(__cmd__, __str__, __cond1__, __cond2__, __struc__, __flag__)    \
    do                                                                                         \
    {                                                                                          \
        osal_memset((__str__), 0, DSH_CMD_MAX_LENGTH);                                         \
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, (__cmd__), (__str__)), token_idx, 2); \
        if (AIR_E_OK == dsh_checkString((__str__), (__cond1__)))                               \
        {                                                                                      \
            (__struc__).flags |= (__flag__);                                                   \
        }                                                                                      \
        else if (AIR_E_OK != dsh_checkString((__str__), (__cond2__)))                          \
        {                                                                                      \
            return DSH_E_SYNTAX_ERR;                                                           \
        }                                                                                      \
    } while (0)

#define _SEC_CHECK_FLAG_ENABLE(__cmd__, __str__, __struc__, __flag__)                \
    _SEC_CHECK_FLAG_STRING(__cmd__, __str__, _str_en, _str_dis, __struc__, __flag__)

#define _SEC_OPT_FLAG_STRING(__cmd__, __str__, __cond1__, __cond2__, __struc__, __flag__)    \
    do                                                                                       \
    {                                                                                        \
        osal_memset((__str__), 0, DSH_CMD_MAX_LENGTH);                                       \
        DSH_CHECK_OPT(dsh_getString(tokens, token_idx, (__cmd__), (__str__)), token_idx, 2); \
        if (osal_strlen(__str__))                                                            \
        {                                                                                    \
            if (AIR_E_OK == dsh_checkString((__str__), (__cond1__)))                         \
            {                                                                                \
                (__struc__).flags |= (__flag__);                                             \
            }                                                                                \
            else if (AIR_E_OK == dsh_checkString((__str__), (__cond2__)))                    \
            {                                                                                \
                (__struc__).flags &= ~(__flag__);                                            \
            }                                                                                \
            else                                                                             \
            {                                                                                \
                return DSH_E_SYNTAX_ERR;                                                     \
            }                                                                                \
        }                                                                                    \
    } while (0)

#define _SEC_OPT_FLAG_ENABLE(__cmd__, __str__, __struc__, __flag__)                \
    _SEC_OPT_FLAG_STRING(__cmd__, __str__, _str_en, _str_dis, __struc__, __flag__)

#define _sec_cmd_error(fmt, ...) osal_printf("\n***Error***, " fmt, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/
static C8_T *_str_unit = "unit";
static C8_T *_str_portlist = "portlist";
static C8_T *_str_port = "port";
static C8_T *_str_storm_type_in[AIR_SEC_STORM_TYPE_LAST] = {"bc", "uuc", "umc"};
static C8_T *_str_storm_type_out[AIR_SEC_STORM_TYPE_LAST] = {"bc", "uuc", "umc"};
static C8_T *_str_en = "enable";
static C8_T *_str_dis = "disable";
static C8_T *_str_allow = "allow";
static C8_T *_str_deny = "deny";
static C8_T *_str_pps = "pps";
static C8_T *_str_kbps = "kbps";
static C8_T *_str_fwd = "fwd";
static C8_T *_str_drop = "drop";
static C8_T *_str_cpu = "cpu";
static C8_T *_str_in = "include";
static C8_T *_str_ex = "exclude";
static C8_T *_str_mode = "mode";

/* LOCAL SUBPROGRAM BODIES
*/

/* STATIC VARIABLE DECLARATIONS
 */
static AIR_ERROR_NO_T
_sec_cmd_set8021x(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    AIR_SEC_8021X_MODE_T base;
    C8_T                 str[DSH_CMD_MAX_LENGTH];

    /* cmd: sec set 8021x [ unit=<UINT> ] base={ mac | port }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "base", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "mac"))
    {
        base = AIR_SEC_8021X_MODE_MAC;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_port))
    {
        base = AIR_SEC_8021X_MODE_PORT;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_set8021xGlobalMode(unit, base);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set 8021x fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_show8021x(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    AIR_SEC_8021X_MODE_T base;

    /* cmd: sec show 8021x [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_get8021xGlobalMode(unit, &base);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("show 8021x fail\n");
        return (rc);
    }
    osal_printf("802.1x authentication base : ");
    if (AIR_SEC_8021X_MODE_MAC == base)
    {
        osal_printf("mac base\n");
    }
    else
    {
        osal_printf("port base\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_set8021xConfig(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0;
    C8_T                str[DSH_CMD_MAX_LENGTH];
    AIR_SEC_8021X_CFG_T config;

    /* cmd: sec set 8021x config [ unit=<UINT> ]
     * sec set 8021x config [ unit=<UINT> ] pb-pm={ allow | deny }
     *                      auth={ fwd | drop | cpu }
     *                      unauth={ fwd | drop | cpu }
     * Note: pb-pm means Portbase-Port-Move
     */

    osal_memset(&config, 0, sizeof(AIR_SEC_8021X_CFG_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);

    _SEC_CHECK_FLAG_STRING("pb-pm", str, _str_allow, _str_deny, config, AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE);

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "auth", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_fwd))
    {
        config.auth_fwd = AIR_SEC_8021X_FWD_MAC;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_drop))
    {
        config.auth_fwd = AIR_SEC_8021X_FWD_DROP;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_cpu))
    {
        config.auth_fwd = AIR_SEC_8021X_FWD_CPU;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "unauth", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_fwd))
    {
        config.unauth_fwd = AIR_SEC_8021X_FWD_MAC;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_drop))
    {
        config.unauth_fwd = AIR_SEC_8021X_FWD_DROP;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_cpu))
    {
        config.unauth_fwd = AIR_SEC_8021X_FWD_CPU;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_set8021xGlobalCfg(unit, &config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set 8021x fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_show8021xConfig(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    AIR_SEC_8021X_CFG_T  config;
    AIR_SEC_8021X_FWD_T *ptr_fwd;
    UI8_T                i;

    /* cmd: sec show 8021x config [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_get8021xGlobalCfg(unit, &config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("show 8021x fail\n");
        return (rc);
    }
    osal_printf("802.1x authenticate configurations\n");
    osal_printf("%-20s: ", "port base port move");
    osal_printf("%s\n", (config.flags & AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE) ? _str_allow : _str_deny);
    for (i = 0; i < 2; i++)
    {
        if (!(i % 2))
        {
            osal_printf("%-20s: ", "authenticate");
            ptr_fwd = &config.auth_fwd;
        }
        else
        {
            osal_printf("%-20s: ", "unauthenticate");
            ptr_fwd = &config.unauth_fwd;
        }
        switch (*ptr_fwd)
        {
            case AIR_SEC_8021X_FWD_MAC:
                osal_printf("forwarding according to mac table\n");
                break;
            case AIR_SEC_8021X_FWD_DROP:
                osal_printf("drop\n");
                break;
            case AIR_SEC_8021X_FWD_CPU:
                osal_printf("trap to cpu\n");
                break;
            default:
                _sec_cmd_error("unknown option\n");
                break;
        }
    }
    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_set8021xMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0;
    C8_T                    str[DSH_CMD_MAX_LENGTH];
    AIR_SEC_8021X_MAC_CFG_T mac_config;

    /* cmd: sec set 8021x mac [ unit=<UINT> ]
     *                        auth-port-move={ allow | deny }
     *                        unauth-port-move={ allow | deny }
     */

    osal_memset(&mac_config, 0, sizeof(AIR_SEC_8021X_MAC_CFG_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);

    _SEC_CHECK_FLAG_STRING("auth-port-move", str, _str_allow, _str_deny, mac_config,
                           AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH);

    _SEC_CHECK_FLAG_STRING("unauth-port-move", str, _str_allow, _str_deny, mac_config,
                           AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH);

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_set8021xMacBasedCfg(unit, &mac_config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set 8021x mac fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_show8021xMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0;
    AIR_SEC_8021X_MAC_CFG_T mac_config;

    /* cmd: sec show 8021x mac [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_get8021xMacBasedCfg(unit, &mac_config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("show 8021x mac fail\n");
        return (rc);
    }
    osal_printf("802.1x mac base authenticate configurations\n");
    osal_printf("%-30s: %s\n", "authenticated mac port move",
                (mac_config.flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH) ? _str_allow : _str_deny);
    osal_printf("%-30s: %s\n", "unauthenticated mac port move",
                (mac_config.flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH) ? _str_allow : _str_deny);

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_set8021xPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0;
    AIR_PORT_BITMAP_T        bitmap = {0};
    UI32_T                   port;
    C8_T                     str[DSH_CMD_MAX_LENGTH];
    AIR_SEC_8021X_PORT_CFG_T port_config;

    /* cmd: sec set 8021x port [ unit=<UINT> ] portlist=<UINTLIST> rx-auth={ auth | unauth }
     */

    osal_memset(&port_config, 0, sizeof(AIR_SEC_8021X_PORT_CFG_T));
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "rx-auth", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "unauth"))
    {
        port_config.rx_auth = AIR_SEC_8021X_AUTH_UNAUTH;
    }
    else if (AIR_E_OK == dsh_checkString(str, "auth"))
    {
        port_config.rx_auth = AIR_SEC_8021X_AUTH_AUTH;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_sec_set8021xPortBasedCfg(unit, port, &port_config);
        if (AIR_E_OK != rc)
        {
            _sec_cmd_error("set 8021x port fail\n");
            break;
        }
    }
    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_show8021xPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0;
    AIR_PORT_BITMAP_T        bitmap = {0};
    UI32_T                   port;
    AIR_SEC_8021X_PORT_CFG_T port_config;

    /* cmd: sec show 8021x port [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%5s/%-4s%8s\n", _str_unit, _str_port, _str_mode);
    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_sec_get8021xPortBasedCfg(unit, port, &port_config);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5u/%2u  %8s\n", unit, port,
                        (AIR_SEC_8021X_AUTH_AUTH == port_config.rx_auth) ? "auth" : "unauth");
        }
        else
        {
            _sec_cmd_error("show 8021x port fail\n");
            return (rc);
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_MacLimitCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0;
    C8_T                    str[DSH_CMD_MAX_LENGTH];
    AIR_SEC_MAC_LIMIT_CFG_T config;
    BOOL_T                  mode;

    /* cmd:
     * sec set mac-limit-ctrl [ unit=<UINT> ] mode={ enable | disable }
     *                        [ mgmt-frame={ include | exclude } ]
     *                        [ port-move-drop={ enable | disable } ]
     *                        [ sa-full-drop={ enable | disable } ]
     *                        [ port-move-tocpu={ enable | disable } ]
     *                        [ sa-full-tocpu={ enable | disable } ]
     */

    rc = air_sec_getMacLimitGlobalCfg(unit, &config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("get mac-limit-ctrl fail\n");
        return rc;
    }

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, _str_mode, str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_en))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_dis))
    {
        mode = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    _SEC_OPT_FLAG_STRING("mgmt-frame", str, _str_ex, _str_in, config, AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT);
    _SEC_OPT_FLAG_ENABLE("port-move-drop", str, config, AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE);
    _SEC_OPT_FLAG_ENABLE("sa-full-drop", str, config, AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL);
    _SEC_OPT_FLAG_ENABLE("port-move-tocpu", str, config, AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE);
    _SEC_OPT_FLAG_ENABLE("sa-full-tocpu", str, config, AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_setMacLimitGlobalMode(unit, mode);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set mac-limit global mode fail\n");
    }

    rc = air_sec_setMacLimitGlobalCfg(unit, &config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set mac-limit global config fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_showMacLimitCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0;
    AIR_SEC_MAC_LIMIT_CFG_T config;
    BOOL_T                  mode;

    /* cmd: sec show mac-limit-ctrl [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_sec_getMacLimitGlobalMode(unit, &mode);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("show mac-limit global mode fail\n");
        return (rc);
    }

    rc = air_sec_getMacLimitGlobalCfg(unit, &config);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("show mac-limit global config fail\n");
        return (rc);
    }

    osal_printf("port security configurations\n");
    osal_printf("%-33s : %s\n", "mac limit status", (mode & TRUE) ? _str_en : _str_dis);
    osal_printf("%-33s : %s\n", "management frame",
                (config.flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT) ? _str_ex : _str_in);
    osal_printf("%-33s : %s\n", "drop port move error frame",
                (config.flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE) ? _str_en : _str_dis);
    osal_printf("%-33s : %s\n", "drop sa full error frame",
                (config.flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL) ? _str_en : _str_dis);
    osal_printf("%-33s : %s\n", "trap port move error frame to cpu",
                (config.flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE) ? _str_en : _str_dis);
    osal_printf("%-33s : %s\n", "trap sa full error frame to cpu",
                (config.flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL) ? _str_en : _str_dis);
    osal_printf("\n");
    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_setMacLimitPortCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0;
    AIR_PORT_BITMAP_T            bitmap = {0};
    UI32_T                       port;
    C8_T                         str[DSH_CMD_MAX_LENGTH];
    AIR_SEC_MAC_LIMIT_PORT_CFG_T port_config;

    /*  cmd:sec set mac-limit-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     *                                  sa-learn={ disable | enable }
     *                                  sa-limit-mode={ disable | enable }
     *                                  sa-limit-count=<UINT>
     */

    osal_memset(&port_config, 0, sizeof(AIR_SEC_MAC_LIMIT_PORT_CFG_T));
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);

    _SEC_CHECK_FLAG_ENABLE("sa-learn", str, port_config, AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN);

    _SEC_CHECK_FLAG_ENABLE("sa-limit-mode", str, port_config, AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT);

    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sa-limit-count", &(port_config.sa_lmt_cnt)), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_sec_setMacLimitPortCfg(unit, port, &port_config);
        if (AIR_E_OK != rc)
        {
            _sec_cmd_error("set mac-limit-port-ctrl fail\n");
            break;
        }
    }
    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_showMacLimitPortCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0;
    AIR_PORT_BITMAP_T            bitmap = {0};
    UI32_T                       port;
    AIR_SEC_MAC_LIMIT_PORT_CFG_T port_config;

    /* cmd: sec show mac-limit-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%5s/%-6s  %-8s  %-13s  %-14s\n", _str_unit, _str_port, "sa-learn", "sa-limit-mode", "sa-limit-count");
    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_sec_getMacLimitPortCfg(unit, port, &port_config);
        if (AIR_E_OK != rc)
        {
            _sec_cmd_error("show mac-limit-port-ctrl fail\n");
            return (rc);
        }
        osal_printf("%5u/%-6u  %-8s  %-13s  %-14u\n", unit, port,
                    (port_config.flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN) ? _str_en : _str_dis,
                    (port_config.flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT) ? _str_en : _str_dis,
                    port_config.sa_lmt_cnt);
    }
    return (rc);
}

static AIR_ERROR_NO_T
_sec_cmd_setStormCtlCfg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0;
    AIR_PORT_BITMAP_T         portlist;
    UI32_T                    port;
    C8_T                      str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_SEC_STORM_TYPE_T      type;
    AIR_SEC_STORM_RATE_MODE_T rate_type;
    BOOL_T                    mode;
    UI32_T                    rate;

    /* Command format
     * sec set storm-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     *                    type={ bc | uuc | umc }
     *                    mode={ enable rate=<UINT> rate-type={ pps | kbps } | disable }
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);

    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_storm_type_in[AIR_SEC_STORM_TYPE_BC]))
    {
        type = AIR_SEC_STORM_TYPE_BC;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_storm_type_in[AIR_SEC_STORM_TYPE_UUC]))
    {
        type = AIR_SEC_STORM_TYPE_UUC;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_storm_type_in[AIR_SEC_STORM_TYPE_UMC]))
    {
        type = AIR_SEC_STORM_TYPE_UMC;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, _str_mode, str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_en))
    {
        mode = TRUE;

        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "rate", &rate), token_idx, 2);
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "rate-type", str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(str, _str_kbps))
        {
            rate_type = AIR_SEC_STORM_RATE_MODE_BPS;
        }
        else if (AIR_E_OK == dsh_checkString(str, _str_pps))
        {
            rate_type = AIR_SEC_STORM_RATE_MODE_PPS;
        }
        else
        {
            return DSH_E_SYNTAX_ERR;
        }
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_dis))
    {
        mode = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_sec_setPortStormCtrl(unit, port, type, mode);
        if (AIR_E_OK != rc)
        {
            _sec_cmd_error("set port:%u type:%s storm ctrl failed:%u\n", port, _str_storm_type_out[type], rc);
            break;
        }

        if (TRUE == mode)
        {
            rc = air_sec_setPortStormCtrlRate(unit, port, type, rate_type, rate);
            if (AIR_E_OK != rc)
            {
                _sec_cmd_error("set port:%u type:%s storm ctrl rate failed:%u\n", port, _str_storm_type_out[type], rc);
                break;
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_sec_cmd_showStormCtlCfg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0;
    AIR_PORT_BITMAP_T         portlist;
    UI32_T                    port;
    BOOL_T                    mode;
    AIR_SEC_STORM_RATE_MODE_T rate_type;
    UI32_T                    rate;
    AIR_SEC_STORM_TYPE_T      type = AIR_SEC_STORM_TYPE_LAST;

    /* Command format
    * "sec show storm-ctrl [ unit=<UINT> ] portlist=<UINTLIST>"
    */
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    osal_printf("%5s/%-4s", _str_unit, _str_port);
    for (type = AIR_SEC_STORM_TYPE_BC; type < AIR_SEC_STORM_TYPE_LAST; type++)
    {
        osal_printf(" %3s-mode %3s-rate %3s-unit", _str_storm_type_out[type], _str_storm_type_out[type],
                    _str_storm_type_out[type]);
    }
    osal_printf("\n");
    AIR_PORT_FOREACH(portlist, port)
    {
        osal_printf("%5u/ %-3u", unit, port);
        for (type = AIR_SEC_STORM_TYPE_BC; type < AIR_SEC_STORM_TYPE_LAST; type++)
        {
            rc = air_sec_getPortStormCtrl(unit, port, type, &mode);
            if (AIR_E_OK == rc)
            {
                osal_printf("%9s", (TRUE == mode) ? _str_en : _str_dis);
            }
            else
            {
                _sec_cmd_error("get port:%u type:%s storm ctrl failed:%u\n", port, _str_storm_type_out[type], rc);
                return rc;
            }
            rc = air_sec_getPortStormCtrlRate(unit, port, type, &rate_type, &rate);
            if (AIR_E_OK == rc)
            {
                osal_printf("%9u %8s", rate, (AIR_SEC_STORM_RATE_MODE_PPS == rate_type) ? _str_pps : _str_kbps);
            }
            else
            {
                _sec_cmd_error("get port:%u type:%s storm ctrl rate failed:%u\n", port, _str_storm_type_out[type], rc);
                return rc;
            }
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_sec_cmd_setStormCtrlMgmt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0;
    AIR_SEC_STORM_CTRL_MODE_T mng_t;
    C8_T                      type[DSH_CMD_MAX_LENGTH] = {0};

    /* Command format
     * sec set storm-ctrl-mgmt-frame [ unit=<UINT> ] type={ include | exclude }
     */
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    if (AIR_E_OK == dsh_checkString(type, _str_in))
    {
        mng_t = AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE;
    }
    else if (AIR_E_OK == dsh_checkString(type, _str_ex))
    {
        mng_t = AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    rc = air_sec_setStormCtrlMgmtMode(unit, mng_t);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("set storm control management mode %s value failed \n", (mng_t & TRUE) ? _str_ex : _str_in);
    }
    return rc;
}

static AIR_ERROR_NO_T
_sec_cmd_showStormCtrlMgmt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0;
    AIR_SEC_STORM_CTRL_MODE_T mng_t = AIR_SEC_STORM_CTRL_MODE_LAST;

    /* Command format
     * sec show storm-ctrl-mgmt-frame [ unit=<UINT> ]
     */
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    rc = air_sec_getStormCtrlMgmtMode(unit, &mng_t);
    if (AIR_E_OK != rc)
    {
        _sec_cmd_error("get storm control management mode failed:%u\n", rc);
    }
    else
    {
        osal_printf("storm control management mode : %s \n",
                    (mng_t == AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE) ? _str_ex : _str_in);
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T  _sec_cmd_vec[] =
{
    {
        "set 8021x config", 3, _sec_cmd_set8021xConfig,
        "sec set 8021x config [ unit=<UINT> ] pb-pm={ allow | deny }\n"
        "auth={ fwd | drop | cpu }\n"
        "unauth={ fwd | drop | cpu }\n"
    },
    {
        "show 8021x config", 3, _sec_cmd_show8021xConfig,
        "sec show 8021x config [ unit=<UINT> ]\n"
    },
    {
        "set 8021x mac", 3, _sec_cmd_set8021xMac,
        "sec set 8021x mac [ unit=<UINT> ]\n"
        "auth-port-move={ allow | deny }\n"
        "unauth-port-move={ allow | deny }\n"
    },
    {
        "show 8021x mac", 3, _sec_cmd_show8021xMac,
        "sec show 8021x mac [ unit=<UINT> ]\n"
    },
    {
        "set 8021x port", 3, _sec_cmd_set8021xPort,
        "sec set 8021x port [ unit=<UINT> ] portlist=<UINTLIST> rx-auth={ auth | unauth }\n"
    },
    {
        "show 8021x port", 3, _sec_cmd_show8021xPort,
        "sec show 8021x port [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set 8021x", 2, _sec_cmd_set8021x,
        "sec set 8021x [ unit=<UINT> ] base={ mac | port }\n"
    },
    {
        "show 8021x", 2, _sec_cmd_show8021x,
        "sec show 8021x [ unit=<UINT> ]\n"
    },
    {
        "set mac-limit-ctrl", 2, _sec_cmd_MacLimitCtrl,
        "sec set mac-limit-ctrl [ unit=<UINT> ] mode={ enable | disable }\n"
        "[ mgmt-frame={ include | exclude } ]\n"
        "[ port-move-drop={ enable | disable } ]\n"
        "[ sa-full-drop={ enable | disable } ]\n"
        "[ port-move-tocpu={ enable | disable } ]\n"
        "[ sa-full-tocpu={ enable | disable } ]\n"
    },
    {
        "show mac-limit-ctrl", 2, _sec_cmd_showMacLimitCtrl,
        "sec show mac-limit-ctrl [ unit=<UINT> ]\n"
    },
    {
        "set mac-limit-port-ctrl", 2, _sec_cmd_setMacLimitPortCtrl,
        "sec set mac-limit-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "sa-learn={ enable | disable }\n"
        "sa-limit-mode={ enable | disable }\n"
        "sa-limit-count=<UINT>\n"
    },
    {
        "show mac-limit-port-ctrl", 2, _sec_cmd_showMacLimitPortCtrl,
        "sec show mac-limit-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set storm-ctrl", 2, _sec_cmd_setStormCtlCfg,
        "sec set storm-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ bc | uuc | umc }\n"
        "mode={ enable rate=<UINT> rate-type={ pps | kbps } | disable }\n"
    },
    {
        "show storm-ctrl", 2, _sec_cmd_showStormCtlCfg,
        "sec show storm-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n",
    },
    {
        "set storm-ctrl-mgmt-frame", 2, _sec_cmd_setStormCtrlMgmt,
        "sec set storm-ctrl-mgmt-frame [ unit=<UINT> ] type={ include | exclude }\n",
    },
    {
        "show storm-ctrl-mgmt-frame", 2, _sec_cmd_showStormCtrlMgmt,
        "sec show storm-ctrl-mgmt-frame [ unit=<UINT> ] \n",
    },
};
/* clang-format on */

AIR_ERROR_NO_T
sec_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _sec_cmd_vec, sizeof(_sec_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
sec_cmd_usager()
{
    return (dsh_usager(_sec_cmd_vec, sizeof(_sec_cmd_vec) / sizeof(DSH_VEC_T)));
}
