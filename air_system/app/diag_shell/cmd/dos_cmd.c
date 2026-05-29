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

#include <cmd/dos_cmd.h>

#include <air_dos.h>
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    DOS_CMD_ATTACK_KIND_LAND = 0,
    DOS_CMD_ATTACK_KIND_BLAT,
    DOS_CMD_ATTACK_KIND_LENG,
    DOS_CMD_ATTACK_KIND_TCP0,
    DOS_CMD_ATTACK_KIND_TCP1,
    DOS_CMD_ATTACK_KIND_TCP2,
    DOS_CMD_ATTACK_KIND_PING,
    DOS_CMD_ATTACK_KIND_TCP_MIN_HDR,
    DOS_CMD_ATTACK_KIND_IP_LAND,
    DOS_CMD_ATTACK_KIND_LAST
} DOS_CMD_ATTACK_KIND_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define DOS_CMD_ACTION_CHECK(__rc__, __attack_set__, __attack_class__) \
    do                                                                 \
    {                                                                  \
        if (TRUE == (__attack_set__)[(__attack_class__)])              \
        {                                                              \
            return (AIR_E_BAD_PARAMETER);                              \
        }                                                              \
        AIR_ERROR_NO_T __ret__ = (__rc__);                             \
        if (AIR_E_OK == (__ret__))                                     \
        {                                                              \
            (__attack_set__)[(__attack_class__)] = TRUE;               \
        }                                                              \
        else                                                           \
        {                                                              \
            return (AIR_E_BAD_PARAMETER);                              \
        }                                                              \
    } while (0)

#define DOS_CMD_PRINT_TCP_TBL_INDEX()                                            \
    do                                                                           \
    {                                                                            \
        osal_printf(" id         ns  cwr  ece  urg  ack  psh  rst  syn  fin\n"); \
        osal_printf("------------------------------------------------------\n"); \
    } while (0)

#define DOS_CMD_PRINT_TCP_SETTING(__unit__, __id__, __ptr_flags__)                                 \
    do                                                                                             \
    {                                                                                              \
        AIR_DOS_TCP_CFG_T *ptr_flags;                                                              \
        ptr_flags = (__ptr_flags__);                                                               \
        osal_printf("%3d%6s", id, "key");                                                          \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_NS) ? " v " : "---");     \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_CWR) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_ECE) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_URG) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_ACK) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_PSH) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_RST) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_SYN) ? " v " : "---");    \
        osal_printf("%5s\n", (ptr_flags->key_flags & AIR_DOS_TCP_CFG_FLAGS_FIN) ? " v " : "---");  \
        osal_printf("%3s%6s", "", "mask");                                                         \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_NS) ? " v " : "---");    \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_CWR) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_ECE) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_URG) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_ACK) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_PSH) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_RST) ? " v " : "---");   \
        osal_printf("%5s", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_SYN) ? " v " : "---");   \
        osal_printf("%5s\n", (ptr_flags->mask_flags & AIR_DOS_TCP_CFG_FLAGS_FIN) ? " v " : "---"); \
    } while (0)

#define DOS_CMD_PRINT_ACTION_TBL_INDEX()                                 \
    do                                                                   \
    {                                                                    \
        osal_printf(" port    attack        drop    limit   rate id\n"); \
        osal_printf("----------------------------------------------\n"); \
    } while (0)

#define DOS_CMD_PRINT_PORT_ACTION_CFG(__ptr_attack_set__)                                                             \
    do                                                                                                                \
    {                                                                                                                 \
        osal_printf("%-8s", ((__ptr_attack_set__)->action_flags & AIR_DOS_ACTION_FLAGS_DROP) ? "enable" : "-");       \
        osal_printf("%-8s", ((__ptr_attack_set__)->action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT) ? "enable" : "-"); \
        osal_printf("%-8d\n", (__ptr_attack_set__)->rate_cfg_idx);                                                    \
    } while (0)

#define DOS_CMD_PRINT_PORT_ACTION(__port__, __ptr_port_cfg__)                  \
    do                                                                         \
    {                                                                          \
        AIR_DOS_PORT_ACTION_CFG_T *ptr_port_cfg = (__ptr_port_cfg__);          \
        osal_printf("%-9d", (__port__));                                       \
        osal_printf("%-14s", "land");                                          \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->land));                  \
        osal_printf("         %-14s", "blat");                                 \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->blat));                  \
        osal_printf("         %-14s", "length");                               \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->length));                \
        osal_printf("         %-14s", "tcp0");                                 \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->tcp[AIR_DOS_TCP_ID_0])); \
        osal_printf("         %-14s", "tcp1");                                 \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->tcp[AIR_DOS_TCP_ID_1])); \
        osal_printf("         %-14s", "tcp2");                                 \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->tcp[AIR_DOS_TCP_ID_2])); \
        osal_printf("         %-14s", "ping");                                 \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->ping));                  \
        osal_printf("         %-14s", "tcp-min-hdr");                          \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->tcp_hdr_min));           \
        osal_printf("         %-14s", "ip-land");                              \
        DOS_CMD_PRINT_PORT_ACTION_CFG(&(ptr_port_cfg->ip_land));               \
    } while (0)

/* STATIC VARIABLE DECLARATIONS
 */
static AIR_ERROR_NO_T
_dos_cmd_setGlobalCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         state = FALSE;

    /* cmd: dos set global-ctrl [ unit=<UINT> ] mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        state = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_dos_setGlobalCfg(unit, state);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set global ctrl error(%d)\n", rc);
    }

    return rc;
};

static AIR_ERROR_NO_T
_dos_cmd_showGlobalCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         state = 0;

    /* cmd: dos show global-ctrl [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    rc = air_dos_getGlobalCfg(unit, &state);
    if (AIR_E_OK == rc)
    {
        osal_printf(" - attack prevention global control = ");
        if (TRUE == state)
        {
            osal_printf("enable\n");
        }
        else
        {
            osal_printf("disable\n");
        }
    }
    else
    {
        osal_printf("***Error***, show global ctrl error(%d)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_dos_cmd_setLengthLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         length = 0;

    /* cmd:  dos set length-limit [ unit=<UINT> ] length=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "length", &length), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_dos_setIcmpLengthLimit(unit, length);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set length-limit error(%d)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_dos_cmd_showLengthLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         length = 0;

    /* cmd: dos show length-limit [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    rc = air_dos_getIcmpLengthLimit(unit, &length);
    if (AIR_E_OK == rc)
    {
        osal_printf(" - length attack limit = %u\n", length);
    }
    else
    {
        osal_printf("***Error***, show length-limit error(%d)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_dos_cmd_setTcpFlagKey(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            id = 0;
    AIR_DOS_TCP_CFG_T tcp_flags;

    /* cmd: dos set tcp-flag [ unit=<UINT> ] id={ 0 | 1 | 2 }
     * [ ns ] [ cwr ] [ ece ] [ urg ] [ ack ]
     * [ psh ] [ rst ] [ syn ] [ fin ]
     */
    osal_memset(&tcp_flags, 0, sizeof(AIR_DOS_TCP_CFG_T));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &id), token_idx, 2);

    if (AIR_DOS_TCP_ID_LAST <= id)
    {
        osal_printf("***Error***, invalid tcp setting id %d.\n", id);
        return AIR_E_BAD_PARAMETER;
    }
    rc = air_dos_getTcpFlagCfg(unit, id, &tcp_flags);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get tcp flag key error(%d)\n", rc);
        return rc;
    }
    tcp_flags.key_flags = 0;
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ns"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_NS;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cwr"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_CWR;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ece"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_ECE;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "urg"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_URG;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ack"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_ACK;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "psh"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_PSH;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rst"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_RST;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "syn"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_SYN;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "fin"))
    {
        tcp_flags.key_flags |= AIR_DOS_TCP_CFG_FLAGS_FIN;
        token_idx += 1;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_dos_setTcpFlagCfg(unit, id, &tcp_flags);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set tcp flag key error(%d)\n", rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_setTcpFlagMask(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            id = 0;
    AIR_DOS_TCP_CFG_T tcp_flags;

    /* cmd: dos set tcp-mask [ unit=<UINT> ] id={ 0 | 1 | 2 }
     * [ ns ] [ cwr ] [ ece ] [ urg ] [ ack ]
     * [ psh ] [ rst ] [ syn ] [ fin ]
     */
    osal_memset(&tcp_flags, 0, sizeof(AIR_DOS_TCP_CFG_T));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &id), token_idx, 2);

    if (AIR_DOS_TCP_ID_LAST <= id)
    {
        osal_printf("***Error***, invalid tcp setting id %d.\n", id);
        return AIR_E_BAD_PARAMETER;
    }
    rc = air_dos_getTcpFlagCfg(unit, id, &tcp_flags);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get tcp flag mask error(%d)\n", rc);
        return rc;
    }
    tcp_flags.mask_flags = 0;
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ns"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_NS;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cwr"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_CWR;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ece"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_ECE;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "urg"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_URG;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ack"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_ACK;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "psh"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_PSH;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rst"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_RST;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "syn"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_SYN;
        token_idx += 1;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "fin"))
    {
        tcp_flags.mask_flags |= AIR_DOS_TCP_CFG_FLAGS_FIN;
        token_idx += 1;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_dos_setTcpFlagCfg(unit, id, &tcp_flags);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set tcp flag mask error(%d)\n", rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_showTcpSetting(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            id = AIR_DOS_TCP_ID_LAST;
    AIR_DOS_TCP_CFG_T tcp_flags = {0};

    /* cmd: dos show tcp-setting [ unit=<UINT> ] [ id=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (NULL != tokens[token_idx])
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &id), token_idx, 2);
        if (AIR_DOS_TCP_ID_LAST <= id)
        {
            osal_printf("***Error***, invalid tcp setting id %d.\n", id);
            return AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    DOS_CMD_PRINT_TCP_TBL_INDEX();
    if (AIR_DOS_TCP_ID_LAST == id) /* get all tcp-id setting */
    {
        for (id = 0; id < AIR_DOS_TCP_ID_LAST; id++)
        {
            rc = air_dos_getTcpFlagCfg(unit, id, &tcp_flags);
            if (AIR_E_OK == rc)
            {
                DOS_CMD_PRINT_TCP_SETTING(unit, id, &tcp_flags);
            }
            else
            {
                osal_printf("***Error***, get tcp-id(%u) setting error(%d)\n", id, rc);
                break;
            }
            tcp_flags.key_flags = 0;
            tcp_flags.mask_flags = 0;
        }
    }
    else /* get specific tcp-id setting */
    {
        rc = air_dos_getTcpFlagCfg(unit, id, &tcp_flags);
        if (AIR_E_OK == rc)
        {
            DOS_CMD_PRINT_TCP_SETTING(unit, id, &tcp_flags);
        }
        else
        {
            osal_printf("***Error***, get tcp-id(%u) setting error(%d)\n", id, rc);
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_action_parser(
    const C8_T       *tokens[],
    UI32_T           *ptr_token_idx,
    UI32_T            attack_class,
    AIR_DOS_ACTION_T *ptr_attack_cfg)
{
    UI32_T rate_id = 0;
    C8_T   str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T  *attack_name[DOS_CMD_ATTACK_KIND_LAST] = {"land", "blat", "leng",        "tcp0",   "tcp1",
                                                    "tcp2", "ping", "tcp-min-hdr", "ip-land"};

    DSH_CHECK_PARAM(dsh_getString(tokens, *ptr_token_idx, attack_name[attack_class], str), *ptr_token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "drop"))
    {
        ptr_attack_cfg->action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    else if (AIR_E_OK == dsh_checkString(str, "limit"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, *ptr_token_idx, "rate-id", &rate_id), *ptr_token_idx, 2);
        ptr_attack_cfg->action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
        ptr_attack_cfg->rate_cfg_idx = rate_id;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_dos_cmd_setAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0, port = 0;
    UI32_T                    star_idx = 0;
    BOOL_T                    atk_set[DOS_CMD_ATTACK_KIND_LAST] = {FALSE};
    AIR_DOS_PORT_ACTION_CFG_T port_cfg = {{0}};

    /* cmd: dos set action [ unit=<UINT> ] port=<UINT>
     *          [ land={ drop | limit rate-id=<UINT> } ]
     *          [ blat={ drop | limit rate-id=<UINT> } ]
     *          [ leng={ drop | limit rate-id=<UINT> } ]
     *          [ tcp0={ drop | limit rate-id=<UINT> } ]
     *          [ tcp1={ drop | limit rate-id=<UINT> } ]
     *          [ tcp2={ drop | limit rate-id=<UINT> } ]
     *          [ ping={ drop | limit rate-id=<UINT> } ]
     *          [ tcp-min-hdr={ drop | limit rate-id=<UINT> } ]
     *          [ ip-land={ drop | limit rate-id=<UINT> } ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);

    star_idx = (token_idx);
    for (;;)
    {
        if (NULL == tokens[token_idx])
        {
            break;
        }
        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "land"))
        {
            DOS_CMD_ACTION_CHECK(_dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_LAND, &port_cfg.land),
                                 atk_set, DOS_CMD_ATTACK_KIND_LAND);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "blat"))
        {
            DOS_CMD_ACTION_CHECK(_dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_BLAT, &port_cfg.blat),
                                 atk_set, DOS_CMD_ATTACK_KIND_BLAT);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "leng"))
        {
            DOS_CMD_ACTION_CHECK(_dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_LENG, &port_cfg.length),
                                 atk_set, DOS_CMD_ATTACK_KIND_LENG);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tcp0"))
        {
            DOS_CMD_ACTION_CHECK(
                _dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_TCP0, &port_cfg.tcp[AIR_DOS_TCP_ID_0]),
                atk_set, DOS_CMD_ATTACK_KIND_TCP0);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tcp1"))
        {
            DOS_CMD_ACTION_CHECK(
                _dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_TCP1, &port_cfg.tcp[AIR_DOS_TCP_ID_1]),
                atk_set, DOS_CMD_ATTACK_KIND_TCP1);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tcp2"))
        {
            DOS_CMD_ACTION_CHECK(
                _dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_TCP2, &port_cfg.tcp[AIR_DOS_TCP_ID_2]),
                atk_set, DOS_CMD_ATTACK_KIND_TCP2);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ping"))
        {
            DOS_CMD_ACTION_CHECK(_dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_PING, &port_cfg.ping),
                                 atk_set, DOS_CMD_ATTACK_KIND_PING);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tcp-min-hdr"))
        {
            DOS_CMD_ACTION_CHECK(
                _dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_TCP_MIN_HDR, &port_cfg.tcp_hdr_min),
                atk_set, DOS_CMD_ATTACK_KIND_TCP_MIN_HDR);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ip-land"))
        {
            DOS_CMD_ACTION_CHECK(
                _dos_cmd_action_parser(tokens, &token_idx, DOS_CMD_ATTACK_KIND_IP_LAND, &port_cfg.ip_land), atk_set,
                DOS_CMD_ATTACK_KIND_IP_LAND);
        }
        else
        {
            osal_printf("***Error***, set unknown attack action\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (star_idx == token_idx)
    {
        osal_printf("***Error***, set action without any attack\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_dos_setPortActionCfg(unit, port, &port_cfg);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set port(%d) action error(%d)\n", port, rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_showAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0, port = 0;
    AIR_DOS_PORT_ACTION_CFG_T port_cfg = {{0}};

    /* cmd: dos show action [ unit=<UINT> ] port=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    rc = air_dos_getPortActionCfg(unit, port, &port_cfg);
    if (AIR_E_OK == rc)
    {
        DOS_CMD_PRINT_ACTION_TBL_INDEX();
        DOS_CMD_PRINT_PORT_ACTION(port, &port_cfg);
    }
    else
    {
        osal_printf("***Error***, get port=%u attack action failed\n", port);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_clearAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         port = 0;

    /* cmd: dos clear action [ unit=<UINT> ] [ port=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (port == 0) /* clear all port setting */
    {
        rc = air_dos_clearActionCfg(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear all port action error(%d)\n", rc);
        }
    }
    else /* clear specified port setting */
    {
        rc = air_dos_clearPortActionCfg(unit, port);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear port(%d) action error(%d)\n", port, rc);
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_setRateLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0;
    UI32_T                   rate_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_cfg = {0};
    C8_T                     str[DSH_CMD_MAX_LENGTH] = {0};

    /* cmd: dos set rate-limit [ unit=<UINT> ] id=<UINT>
     * pkt-threshold=<UINT> time-span=<UINT> block-time=<UINT> tick={ 1ms | 8ms }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &rate_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pkt-threshold", &rate_cfg.pkt_thld), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "time-span", &rate_cfg.time_span), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "block-time", &rate_cfg.block_time), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "tick", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "1ms"))
    {
        rate_cfg.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
    }
    else if (AIR_E_OK == dsh_checkString(str, "8ms"))
    {
        rate_cfg.tick_sel = AIR_DOS_RATE_TICKSEL_8MS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_dos_setRateLimitCfg(unit, rate_id, &rate_cfg);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set rate-id(%d) action error(%d)\n", rate_id, rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_showRateLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0;
    UI32_T                   rate_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_cfg = {0};

    /* cmd: dos show rate-limit [ unit=<UINT> ] id=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &rate_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_dos_getRateLimitCfg(unit, rate_id, &rate_cfg);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf(" - rate id = %u\n", rate_id);
        osal_printf(" - packet number threshold = %u\n", rate_cfg.pkt_thld);
        osal_printf(" - time span config        = %u\n", rate_cfg.time_span);
        osal_printf(" - block timeout           = %u\n", rate_cfg.block_time);
        osal_printf(" - tick selection          = ");
        if (AIR_DOS_RATE_TICKSEL_1MS == rate_cfg.tick_sel)
        {
            osal_printf("%s\n", "1 ms");
        }
        else if (AIR_DOS_RATE_TICKSEL_8MS == rate_cfg.tick_sel)
        {
            osal_printf("%s\n", "8 ms");
        }
    }
    else
    {
        osal_printf("***Error***, get rate-id=%u cfg failed\n", rate_id);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_clearRateLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         rate_id = 0;
    BOOL_T         is_all = FALSE;

    /* cmd: dos clear rate-limit [ unit=<UINT> ] { id=<UINT> | all }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "id"))
    {
        DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "id", &rate_id), token_idx, 2);
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "all"))
    {
        is_all = TRUE;
        token_idx += 1;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (TRUE == is_all)
    {
        rc = air_dos_clearAllRateLimitCfg(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear all rate limit error(%d)\n", rc);
        }
    }
    else
    {
        rc = air_dos_clearRateLimitCfg(unit, rate_id);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear rate limit id(%d) error(%d)\n", rate_id, rc);
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_showDropCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         drop_cnt;

    /* cmd: dos show drop-counter [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_dos_getDropCnt(unit, &drop_cnt);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf(" - attack prevent drop packet     : %u\n", drop_cnt);
    }
    else
    {
        osal_printf("***Error***, get drop counter error(%d)\n", rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_clearDropCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: dos clear drop-counter [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_dos_clearDropCnt(unit);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, clear drop counter error(%d)\n", rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_dos_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         size = 0, count = 0;

    /* cmd: dos show resource [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    osal_printf("%-25s %6s/%6s\n", "type", "used", "total");

    rc = air_swc_getCapacity(unit, AIR_SWC_RSRC_DOS_RATELIMIT, 0, &size);
    if (AIR_E_OK == rc)
    {
        rc = air_swc_getUsage(unit, AIR_SWC_RSRC_DOS_RATELIMIT, 0, &count);
        if (AIR_E_OK == rc)
        {
            osal_printf("%-25s %6u/%6u\n", "rate-limit", count, size);
        }
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, acl show resource error\n");
    }

    return rc;
}

/* clang-format off */
const static DSH_VEC_T  _dos_cmd_vec[] =
{
    {
        "set global-ctrl", 2, _dos_cmd_setGlobalCtrl,
        "dos set global-ctrl [ unit=<UINT> ] mode={ enable | disable }\n"
    },
    {
        "show global-ctrl", 2, _dos_cmd_showGlobalCtrl,
        "dos show global-ctrl [ unit=<UINT> ]\n"
    },
    {
        "set length-limit", 2, _dos_cmd_setLengthLimit,
        "dos set length-limit [ unit=<UINT> ] length=<UINT>\n"
    },
    {
        "show length-limit", 2, _dos_cmd_showLengthLimit,
        "dos show length-limit [ unit=<UINT> ]\n"
    },
    {
        "set tcp-flag", 2, _dos_cmd_setTcpFlagKey,
        "dos set tcp-flag [ unit=<UINT> ] id=<UINT> [ ns ] [ cwr ] [ ece ] [ urg ] [ ack ]\n"
        "[ psh ] [ rst ] [ syn ] [ fin ]\n"
    },
    {
        "set tcp-mask", 2, _dos_cmd_setTcpFlagMask,
        "dos set tcp-mask [ unit=<UINT> ] id=<UINT> [ ns ] [ cwr ] [ ece ] [ urg ] [ ack ]\n"
        "[ psh ] [ rst ] [ syn ] [ fin ]\n"
    },
    {
        "show tcp-setting", 2, _dos_cmd_showTcpSetting,
        "dos show tcp-setting [ unit=<UINT> ] [ id=<UINT> ]\n"
    },
    {
        "set action", 2, _dos_cmd_setAction,
        "dos set action [ unit=<UINT> ] port=<UINT>\n"
        "[ land={ drop | limit rate-id=<UINT> } ]\n"
        "[ blat={ drop | limit rate-id=<UINT> } ]\n"
        "[ leng={ drop | limit rate-id=<UINT> } ]\n"
        "[ tcp0={ drop | limit rate-id=<UINT> } ]\n"
        "[ tcp1={ drop | limit rate-id=<UINT> } ]\n"
        "[ tcp2={ drop | limit rate-id=<UINT> } ]\n"
        "[ ping={ drop | limit rate-id=<UINT> } ]\n"
        "[ tcp-min-hdr={ drop | limit rate-id=<UINT> } ]\n"
        "[ ip-land={ drop | limit rate-id=<UINT> } ]\n"
    },
    {
        "show action", 2, _dos_cmd_showAction,
        "dos show action [ unit=<UINT> ] port=<UINT>\n"
    },
    {
        "clear action", 2, _dos_cmd_clearAction,
        "dos clear action [ unit=<UINT> ] [ port=<UINT> ]\n"
    },
    {
        "set rate-limit", 2, _dos_cmd_setRateLimit,
        "dos set rate-limit [ unit=<UINT> ] id=<UINT>\n"
        "pkt-threshold=<UINT> time-span=<UINT> block-time=<UINT> tick={ 1ms | 8ms }\n"
    },
    {
        "show rate-limit", 2, _dos_cmd_showRateLimit,
        "dos show rate-limit [ unit=<UINT> ] id=<UINT>\n"
    },
    {
        "clear rate-limit", 2, _dos_cmd_clearRateLimit,
        "dos clear rate-limit [ unit=<UINT> ] { id=<UINT> | all }\n"
    },
    {
        "show drop-counter", 2, _dos_cmd_showDropCnt,
        "dos show drop-counter [ unit=<UINT> ]\n"
    },
    {
        "clear drop-counter", 2, _dos_cmd_clearDropCnt,
        "dos clear drop-counter [ unit=<UINT> ]\n"
    },
    {
        "show resource", 2, _dos_cmd_showResource,
        "dos show resource [ unit=<UINT> ]\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
dos_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _dos_cmd_vec, sizeof(_dos_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
dos_cmd_usager()
{
    return (dsh_usager(_dos_cmd_vec, sizeof(_dos_cmd_vec) / sizeof(DSH_VEC_T)));
}
