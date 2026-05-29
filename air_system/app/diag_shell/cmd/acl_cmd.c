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
#include <cmd/acl_cmd.h>

#include <air_acl.h>
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_bit.h>
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
#define AIR_CMD_ENABLE_STATE(state) (TRUE == state) ? "enable" : "disable"

/* DATA TYPE DECLARATIONS
 */
const static C8_T *_acl_rsrc_type[] = {"rule", "action", "udf", "counter", "meter", "trtcm"};
/* LOCAL SUBPROGRAM BODIES
 */
static void
_port_cmd_printPortList(
    const C8_T             *ptr_str,
    const AIR_PORT_BITMAP_T pbm)
{
    UI32_T cnt, port, first, span = 0;

    CMLIB_PORT_BITMAP_COUNT(pbm, cnt);
    if (0 == cnt)
    {
        osal_printf("%snull\n", ptr_str);
    }
    else
    {
        first = 1;
        CMLIB_BITMAP_BIT_FOREACH(pbm, port, AIR_PORT_BITMAP_SIZE)
        {
            osal_printf("%s%d", first ? ptr_str : ",", port);
            first = 0;

            for (span = 1; (++port < AIR_PORT_NUM) && CMLIB_BITMAP_BIT_CHK(pbm, port); span++)
                ;

            if (span > 1)
            {
                osal_printf("-%d", port - 1);
            }
        }
        osal_printf("\n");
    }
}

static AIR_ERROR_NO_T
_acl_cmd_showGlobalCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         state = 0;

    /* cmd: acl get global-ctrl [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getGlobalState(unit, &state);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("acl global control: %s\n", AIR_CMD_ENABLE_STATE(state));
    }
    else
    {
        osal_printf("***Error***, get acl global ctrl error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setGlobalCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           mode_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         mode = 0;

    /* cmd: acl set global-ctrl [ unit=<UINT> ] mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    rc = air_acl_setGlobalState(unit, mode);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set acl global ctrl error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showPortCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    BOOL_T            state = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            port = 0;

    /* cmd: acl get port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_getPortState(unit, port, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf("acl port %d control: %s\n", port, AIR_CMD_ENABLE_STATE(state));
        }
        else
        {
            osal_printf("***Error***, get acl port %d ctrl error\n", port);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setPortCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    C8_T              mode_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T            mode = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            port = 0;

    /* cmd: acl set port-ctrl [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_setPortState(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set acl port %d ctrl error\n", port);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delRuleActionAll(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: acl del rule-action-all [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_clearAll(unit);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, del acl rule-action-all error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         entry_id = 0;
    UI32_T         i = 0;
    AIR_ACL_RULE_T rule;

    /* cmd: acl show rule [ unit=<UINT> ] entry-id=<UINT>
     */
    osal_memset(&rule, 0, sizeof(AIR_ACL_RULE_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getRule(unit, entry_id, &rule);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        if (FALSE == rule.rule_en)
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(rule.rule_en));
        }
        else
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(rule.rule_en));
            osal_printf("isipv6:    %d\n", rule.isipv6);
            osal_printf("reverse:   %d\n", rule.reverse);
            osal_printf("rule-end:  %d\n", rule.end);
            osal_printf("end-reverse:   %d\n", rule.end_reverse);
            _port_cmd_printPortList("portlist:  ", &rule.portmap[0]);

            for (i = AIR_ACL_DMAC_KEY; i < AIR_ACL_RULE_FIELD_LAST; i++)
            {
                if ((1U << i) & rule.field_valid)
                {
                    switch (i)
                    {
                        case AIR_ACL_DMAC_KEY:
                            osal_printf("dmac: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.dmac[0], rule.key.dmac[1],
                                        rule.key.dmac[2], rule.key.dmac[3], rule.key.dmac[4], rule.key.dmac[5]);
                            osal_printf(", dmac-mask: 0x%x\n", rule.mask.dmac);
                            break;
                        case AIR_ACL_SMAC_KEY:
                            osal_printf("smac: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.smac[0], rule.key.smac[1],
                                        rule.key.smac[2], rule.key.smac[3], rule.key.smac[4], rule.key.smac[5]);
                            osal_printf(", smac-mask: 0x%x\n", rule.mask.smac);
                            break;
                        case AIR_ACL_ETYPE_KEY:
                            osal_printf("ethertype: 0x%x, ethertype-mask: 0x%x\n", rule.key.etype, rule.mask.etype);
                            break;
                        case AIR_ACL_STAG_KEY:
                            osal_printf("stag: 0x%x, stag-mask: 0x%x\n", rule.key.stag, rule.mask.stag);
                            break;
                        case AIR_ACL_CTAG_KEY:
                            osal_printf("ctag: 0x%x, ctag-mask: 0x%x\n", rule.key.ctag, rule.mask.ctag);
                            break;
                        case AIR_ACL_DPORT_KEY:
                            osal_printf("dport: 0x%x, dport-mask: 0x%x\n", rule.key.dport, rule.mask.dport);
                            break;
                        case AIR_ACL_SPORT_KEY:
                            osal_printf("sport: 0x%x, sport-mask: 0x%x\n", rule.key.sport, rule.mask.sport);
                            break;
                        case AIR_ACL_UDF_KEY:
                            osal_printf("udf-list: 0x%x, reverse: 0x%x\n", rule.mask.udf,
                                        (rule.key.udf ^ rule.mask.udf));
                            break;
                        case AIR_ACL_DIP_KEY:
                            osal_printf("%s: ", "dip");
                            dsh_showIpv4Addr(&rule.key.dip);
                            osal_printf(", dip-mask: 0x%x\n", rule.mask.dip);
                            break;
                        case AIR_ACL_SIP_KEY:
                            osal_printf("%s: ", "sip");
                            dsh_showIpv4Addr(&rule.key.sip);
                            osal_printf(", sip-mask: 0x%x\n", rule.mask.sip);
                            break;
                        case AIR_ACL_DSCP_KEY:
                            osal_printf("dscp: 0x%x\n", rule.key.dscp);
                            break;
                        case AIR_ACL_NEXT_HEADER_KEY:
                            osal_printf("next-header: 0x%x\n", rule.key.next_header);
                            break;
                        case AIR_ACL_FLOW_LABEL_KEY:
                            osal_printf("flow-label: 0x%x, flow-label-mask: 0x%x\n", rule.key.flow_label,
                                        rule.mask.flow_label);
                            break;
                        case AIR_ACL_DIPV6_KEY:
                            osal_printf("%s: ", "dip");
                            dsh_showIpv6Addr((const AIR_IPV6_T *)&rule.key.dipv6);
                            osal_printf(", dip-mask: 0x%x\n", rule.mask.dipv6);
                            break;
                        case AIR_ACL_SIPV6_KEY:
                            osal_printf("%s: ", "sip");
                            dsh_showIpv6Addr((const AIR_IPV6_T *)&rule.key.sipv6);
                            osal_printf(", sip-mask: 0x%x\n", rule.mask.sipv6);
                            break;
                        default:
                            osal_printf("error\n");
                            break;
                    }
                }
            }
        }
    }
    else
    {
        osal_printf("***Error***, get acl rule %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_addRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_ACL_RULE_T rule;
    UI32_T         entry_id = 0;
    C8_T           tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T         tmp = 0;

    /* cmd: acl add rule [ unit=<UINT> ] entry-id=<UINT>
     *      mode={ enable | disable } portlist=<UINTLIST>
     *      [ isipv6 ] [ reverse ] [ rule-end ] [ end-reverse ]
     *      [ dmac=<MACADDR> [ dmac-mask=<HEX> ] ]
     *      [ smac=<MACADDR> [ smac-mask=<HEX> ] ]
     *      [ ethertype=<HEX> [ ethertype-mask=<HEX> ] ]
     *      [ stag=<HEX> [ stag-mask=<HEX> ] ]
     *      [ ctag=<HEX> [ ctag-mask=<HEX> ] ]
     *      [ dip=<IPADDR> [ dip-mask=<HEX> ] ]
     *      [ sip=<IPADDR> [ sip-mask=<HEX> ] ]
     *      [ dscp=<HEX> ]
     *      [ next-header=<HEX> ]
     *      [ flow-label=<HEX> [ flow-label-mask=<HEX> ] ]
     *      [ dport=<HEX> [ dport-mask=<HEX> ] ]
     *      [ sport=<HEX> [ sport-mask=<HEX> ] ]
     *      [ udf-list=<UINTLIST> [ reverse=<UINTLIST> ] ]
     */

    osal_memset(&rule, 0, sizeof(AIR_ACL_RULE_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "enable"))
    {
        rule.rule_en = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "disable"))
    {
        rule.rule_en = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (TRUE == rule.rule_en)
    {
        DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &(rule.portmap)), token_idx, 2);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "isipv6"))
    {
        rule.isipv6 = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "reverse"))
    {
        rule.reverse = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rule-end"))
    {
        rule.end = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "end-reverse"))
    {
        rule.end_reverse = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dmac"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "dmac", (AIR_MAC_T *)rule.key.dmac), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dmac-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dmac-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.dmac = tmp;
        }
        else
        {
            rule.mask.dmac = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_DMAC_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "smac"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "smac", (AIR_MAC_T *)rule.key.smac), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "smac-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "smac-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.smac = tmp;
        }
        else
        {
            rule.mask.smac = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_SMAC_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ethertype"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ethertype", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.etype = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ethertype-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ethertype-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.etype = tmp;
        }
        else
        {
            rule.mask.etype = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_ETYPE_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "stag"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "stag", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.stag = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "stag-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "stag-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.stag = tmp;
        }
        else
        {
            rule.mask.stag = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_STAG_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ctag"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ctag", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.ctag = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ctag-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ctag-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.ctag = tmp;
        }
        else
        {
            rule.mask.ctag = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_CTAG_KEY);
    }
    if (0 == rule.isipv6)
    {
        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip"))
        {
            DSH_CHECK_PARAM(dsh_getIpv4Addr(tokens, token_idx, "dip", &(rule.key.dip)), token_idx, 2);
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip-mask"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.mask.dip = tmp;
            }
            else
            {
                rule.mask.dip = 0xf;
            }

            rule.field_valid |= (1U << AIR_ACL_DIP_KEY);
        }

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip"))
        {
            DSH_CHECK_PARAM(dsh_getIpv4Addr(tokens, token_idx, "sip", &(rule.key.sip)), token_idx, 2);

            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip-mask"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.mask.sip = tmp;
            }
            else
            {
                rule.mask.sip = 0xf;
            }

            rule.field_valid |= (1U << AIR_ACL_SIP_KEY);
        }
    }
    else
    {
        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip"))
        {
            if ((entry_id % 4) != 0)
            {
                osal_printf("***Error***, illegal rule id, must be multiples of 4\n");
                return AIR_E_BAD_PARAMETER;
            }
            DSH_CHECK_PARAM(dsh_getIpv6Addr(tokens, token_idx, "dip", &(rule.key.dipv6)), token_idx, 2);

            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip-mask"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.mask.dipv6 = tmp;
            }
            else
            {
                rule.mask.dipv6 = 0xffff;
            }
            rule.field_valid |= (1U << AIR_ACL_DIPV6_KEY);
        }

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip"))
        {
            if ((entry_id % 4) != 0)
            {
                osal_printf("***Error***, illegal rule id, must be multiples of 4\n");
                return AIR_E_BAD_PARAMETER;
            }
            DSH_CHECK_PARAM(dsh_getIpv6Addr(tokens, token_idx, "sip", &(rule.key.sipv6)), token_idx, 2);

            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip-mask"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.mask.sipv6 = tmp;
            }
            else
            {
                rule.mask.sipv6 = 0xffff;
            }
            rule.field_valid |= (1U << AIR_ACL_SIPV6_KEY);
        }
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dscp"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dscp", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.dscp = tmp;
        rule.field_valid |= (1U << AIR_ACL_DSCP_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "next-header"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "next-header", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.next_header = tmp;
        rule.field_valid |= (1U << AIR_ACL_NEXT_HEADER_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "flow-label"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "flow-label", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.flow_label = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "flow-label-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "flow-label-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.flow_label = tmp;
        }
        else
        {
            rule.mask.flow_label = 0x7;
        }

        rule.field_valid |= (1U << AIR_ACL_FLOW_LABEL_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dport"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dport", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.dport = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dport-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dport-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.dport = tmp;
        }
        else
        {
            rule.mask.dport = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_DPORT_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sport"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sport", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.sport = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sport-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sport-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.sport = tmp;
        }
        else
        {
            rule.mask.sport = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_SPORT_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "udf-list"))
    {
        DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "udf-list", &tmp, 1), token_idx, 2);
        rule.mask.udf = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "reverse"))
        {
            DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "reverse", &tmp, 1), token_idx, 2);
            rule.key.udf = (~tmp) & rule.mask.udf;
        }
        else
        {
            rule.key.udf = rule.mask.udf;
        }

        rule.field_valid |= (1U << AIR_ACL_UDF_KEY);
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_acl_setRule(unit, entry_id, &rule);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, some feature is not supported on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add acl rule error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         entry_id = 0;

    /* cmd: acl del rule [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_delRule(unit, entry_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete acl rule %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_addAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0;
    AIR_ACL_ACTION_T action;
    UI32_T           entry_id = 0;
    C8_T             tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T           type = 0;
    UI32_T           tmp = 0;

    /* cmd: acl add action [ unit=<UINT> ] entry-id=<UINT>
     *      [ force-port={ dst | vlan } portlist=<UINTLIST> ]
     *      [ mib-id=<UINT> ]
     *      [ attack-rate-id=<UINT> ]
     *      [ meter-id=<UINT> ]
     *      [ port-forward={ default | cpu-port-excluded | cpu-port-included | cpu-port-only | drop } ]
     *      [ mirror-port=<HEX> ]
     *      [ user-pri=<UINT> ]
     *      [ egrs-tag={ default | consistent | untagged | swap | tagged | stack } ]
     *      [ lky-vlan ]
     *      [ acl-mgmt ]
     *      [ drop-pcd={ original | defined } [red-drop-pcd=<UINT>] [yellow-drop-pcd=<UINT>] [green-drop-pcd=<UINT>] ]
     *      [ class-slr={ original | defined } [class-slr=<UINT>] ]
     *      [ acl-tcm={ defined | trtcm } {defined-color ={ default | green | yellow | red } | trtcm-id=<UINT> } ]
     *      [ acl-vid=<UINT> ]
     *      [ mac-chg da-idx=<UINT> sa-idx=<UINT>]
     *      [ dont-learn ]
     *      [ as-bpdu ]
     */
    osal_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "force-port"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "force-port", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "dst"))
        {
            action.dst_port_swap = 1;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "vlan"))
        {
            action.vlan_port_swap = 1;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "portlist"))
        {
            DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &action.portmap), token_idx, 2);
        }
        action.field_valid |= (1U << AIR_ACL_PORT);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mib-id"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mib-id", &tmp), token_idx, 2);
        action.acl_mib_id = tmp;
        action.field_valid |= (1U << AIR_ACL_MIB);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "attack-rate-id"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "attack-rate-id", &tmp), token_idx, 2);
        action.attack_rate_id = tmp;
        action.field_valid |= (1U << AIR_ACL_ATTACK);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "meter-id"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "meter-id", &tmp), token_idx, 2);
        action.meter_id = tmp;
        action.field_valid |= (1U << AIR_ACL_METER);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "port-forward"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "port-forward", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "default"))
        {
            type = 0;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "cpu-port-excluded"))
        {
            type = 4;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "cpu-port-included"))
        {
            type = 5;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "cpu-port-only"))
        {
            type = 6;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "drop"))
        {
            type = 7;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        action.port_fw = type;
        action.field_valid |= (1U << AIR_ACL_FW_PORT);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mirror-port"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "mirror-port", &tmp, sizeof(UI32_T)), token_idx, 2);
        action.mirror_port = tmp;
        action.field_valid |= (1U << AIR_ACL_MIRROR);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "user-pri"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "user-pri", &tmp), token_idx, 2);
        action.pri_user = tmp;
        action.field_valid |= (1U << AIR_ACL_PRI);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "egrs-tag"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "egrs-tag", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "default"))
        {
            type = 0;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "consistent"))
        {
            type = 1;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "untagged"))
        {
            type = 4;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "swap"))
        {
            type = 5;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "tagged"))
        {
            type = 6;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "stack"))
        {
            type = 7;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        action.eg_tag = type;
        action.field_valid |= (1U << AIR_ACL_EGTAG);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "lky-vlan"))
    {
        action.lky_vlan = 1;
        action.field_valid |= (1U << AIR_ACL_LKY_VLAN);
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "acl-mgmt"))
    {
        action.acl_mang = 1;
        action.field_valid |= (1U << AIR_ACL_MANG);
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "drop-pcd"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "drop-pcd", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "original"))
        {
            action.drop_pcd_sel = 0;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "defined"))
        {
            action.drop_pcd_sel = 1;
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "red-drop-pcd"))
            {
                DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "red-drop-pcd", &tmp), token_idx, 2);
                action.drop_pcd_r = tmp;
            }
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "yellow-drop-pcd"))
            {
                DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "yellow-drop-pcd", &tmp), token_idx, 2);
                action.drop_pcd_y = tmp;
            }
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "green-drop-pcd"))
            {
                DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "green-drop-pcd", &tmp), token_idx, 2);
                action.drop_pcd_g = tmp;
            }
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        action.field_valid |= (1U << AIR_ACL_DROP_PCD);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "class-slr"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "class-slr", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "original"))
        {
            action.class_slr_sel = 0;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "defined"))
        {
            action.class_slr_sel = 1;
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "class-slr", &tmp), token_idx, 2);
            action.class_slr = tmp;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        action.field_valid |= (1U << AIR_ACL_CLASS_SLR);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "acl-tcm"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "acl-tcm", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "defined"))
        {
            action.acl_tcm_sel = 0;
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "defined-color"))
            {
                DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "defined-color", tmp_str), token_idx, 2);
                if (AIR_E_OK == dsh_checkString(tmp_str, "default"))
                {
                    type = 0;
                }
                else if (AIR_E_OK == dsh_checkString(tmp_str, "green"))
                {
                    type = 1;
                }
                else if (AIR_E_OK == dsh_checkString(tmp_str, "yellow"))
                {
                    type = 2;
                }
                else if (AIR_E_OK == dsh_checkString(tmp_str, "red"))
                {
                    type = 3;
                }
                else
                {
                    return AIR_E_BAD_PARAMETER;
                }
                action.def_color = type;
            }
            else
            {
                osal_printf("***Error***, when acl-tcm=defined, the corresponding setting is defined-color\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "trtcm"))
        {
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "trtcm-id"))
            {
                DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "trtcm-id", &tmp), token_idx, 2);
                action.acl_tcm_sel = 1;
                action.trtcm_id = tmp;
            }
            else
            {
                osal_printf("***Error***, when acl-tcm=trtcm, the corresponding setting is trtcm-id\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
        action.field_valid |= (1U << AIR_ACL_TRTCM);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "acl-vid"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "acl-vid", &tmp), token_idx, 2);
        action.acl_vlan_vid = tmp;
        action.field_valid |= (1U << AIR_ACL_VLAN);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mac-chg"))
    {
        action.field_valid |= (1U << AIR_ACL_MAC_CHG);
        token_idx += 1;
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "da-idx", &tmp), token_idx, 2);
        action.da_index = tmp;
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sa-idx", &tmp), token_idx, 2);
        action.sa_index = tmp;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dont-learn"))
    {
        action.dont_learn = 1;
        action.field_valid |= (1U << AIR_ACL_DONT_LEARN);
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "as-bpdu"))
    {
        action.as_bpdu = 1;
        action.field_valid |= (1U << AIR_ACL_BPDU);
        token_idx += 1;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_acl_setAction(unit, entry_id, &action);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, some feature is not supported on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add acl action error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_ACL_ACTION_T  action;
    UI32_T            entry_id = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            i = 0;

    /* cmd: acl get action [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getAction(unit, entry_id, &action);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("entry-id: %d\n", entry_id);
        for (i = AIR_ACL_PORT; i < AIR_ACL_ACTION_FIELD_LAST; i++)
        {
            if (0 != ((1U << i) & action.field_valid))
            {
                switch (i)
                {
                    case AIR_ACL_PORT:
                        if (1 == action.dst_port_swap)
                        {
                            AIR_PORT_BITMAP_COPY(pbm, action.portmap);
                            _port_cmd_printPortList("dst-port:  ", &pbm[0]);
                        }
                        else if (1 == action.vlan_port_swap)
                        {
                            AIR_PORT_BITMAP_COPY(pbm, action.portmap);
                            _port_cmd_printPortList("vlan-port:  ", &pbm[0]);
                        }
                        break;
                    case AIR_ACL_MIB:
                        osal_printf("mib-id: %d\n", action.acl_mib_id);
                        break;
                    case AIR_ACL_ATTACK:
                        osal_printf("attack-rate-id: %d\n", action.attack_rate_id);
                        break;
                    case AIR_ACL_METER:
                        osal_printf("meter-id: %d\n", action.meter_id);
                        break;
                    case AIR_ACL_FW_PORT:
                        osal_printf("port-forward: ");
                        switch (action.port_fw)
                        {
                            case 0:
                                osal_printf("default\n");
                                break;
                            case 4:
                                osal_printf("cpu-port-excluded\n");
                                break;
                            case 5:
                                osal_printf("cpu-port-included\n");
                                break;
                            case 6:
                                osal_printf("cpu-port-only\n");
                                break;
                            case 7:
                                osal_printf("drop\n");
                                break;
                            default:
                                osal_printf("error\n");
                                break;
                        }
                        break;
                    case AIR_ACL_MIRROR:
                        osal_printf("mirror-port: 0x%x\n", action.mirror_port);
                        break;
                    case AIR_ACL_PRI:
                        osal_printf("user-pri: %d\n", action.pri_user);
                        break;
                    case AIR_ACL_EGTAG:
                        osal_printf("egrs-tag: ");
                        switch (action.eg_tag)
                        {
                            case 0:
                                osal_printf("default\n");
                                break;
                            case 1:
                                osal_printf("consistent\n");
                                break;
                            case 4:
                                osal_printf("untagged\n");
                                break;
                            case 5:
                                osal_printf("swap\n");
                                break;
                            case 6:
                                osal_printf("tagged\n");
                                break;
                            case 7:
                                osal_printf("stack\n");
                                break;
                            default:
                                osal_printf("error\n");
                                break;
                        }
                        break;
                    case AIR_ACL_LKY_VLAN:
                        osal_printf("%s\n", "enable leaky vlan");
                        break;
                    case AIR_ACL_MANG:
                        osal_printf("%s\n", "enable management frame attribute");
                        break;
                    case AIR_ACL_DROP_PCD:
                        if (1 == action.drop_pcd_sel)
                        {
                            osal_printf("red-drop-pcd:    0x%x\n", action.drop_pcd_r);
                            osal_printf("yellow-drop-pcd: 0x%x\n", action.drop_pcd_y);
                            osal_printf("green-drop-pcd:  0x%x\n", action.drop_pcd_g);
                        }
                        else
                        {
                            osal_printf("use original drop precedence\n");
                        }
                        break;
                    case AIR_ACL_CLASS_SLR:
                        if (1 == action.class_slr_sel)
                        {
                            osal_printf("class-slr: %d\n", action.class_slr);
                        }
                        else
                        {
                            osal_printf("use original class selector\n");
                        }
                        break;
                    case AIR_ACL_TRTCM:
                        if (0 == action.acl_tcm_sel)
                        {
                            osal_printf("user defined color: ");
                            switch (action.def_color)
                            {
                                case 0:
                                    osal_printf("default\n");
                                    break;
                                case 1:
                                    osal_printf("green\n");
                                    break;
                                case 2:
                                    osal_printf("yellow\n");
                                    break;
                                case 3:
                                    osal_printf("red\n");
                                    break;
                                default:
                                    osal_printf("error\n");
                                    break;
                            }
                        }
                        else
                        {
                            osal_printf("acl-trtcm-id: %d\n", action.trtcm_id);
                        }
                        break;
                    case AIR_ACL_VLAN:
                        osal_printf("acl-vlan-vid: %d\n", action.acl_vlan_vid);
                        break;
                    case AIR_ACL_MAC_CHG:
                        osal_printf("acl-da-idx: %d\n", action.da_index);
                        osal_printf("acl-sa-idx: %d\n", action.sa_index);
                        break;
                    case AIR_ACL_DONT_LEARN:
                        osal_printf("%s\n", "l2 don't learn");
                        break;
                    case AIR_ACL_BPDU:
                        osal_printf("%s\n", "regard as bpdu");
                        break;
                    default:
                        osal_printf("%s\n", "error");
                        break;
                }
            }
        }
    }
    else
    {
        osal_printf("***Error***, get acl action %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         entry_id = 0;

    /* cmd: acl del action [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_delAction(unit, entry_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete acl action %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showMibCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         cnt = 0;
    UI32_T         index = 0;

    /* cmd: acl show mib-counter [ unit=<UINT> ] mib-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);

    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mib-id", &index), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getMibCnt(unit, index, &cnt);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("acl mib counter %d = 0x%08x\n", index, cnt);
    }
    else
    {
        osal_printf("***Error***, get acl mib counter %d error\n", index);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_clearMibCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: acl clear mib-counter [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_clearMibCnt(unit);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, clear acl mib counter error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_addUdfRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_ACL_UDF_RULE_T rule;
    UI32_T             entry_id = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T             tmp;

    /* cmd: acl add udf-rule [ unit=<UINT> ] entry-id=<UINT>
     *      mode={ enable | disable } field={ mac | l2-payload | ipv4 | ipv6 | l3-payload | tcp | udp | l4-payload }
     *      word-ofst=<UINT> cmp={pattern | threshold} {cmp-key=<HEX> cmp-mask=<HEX> | cmp-low=<HEX> cmp-high=<HEX> }
     *      portlist=<UINTLIST>
     */
    osal_memset(&rule, 0, sizeof(AIR_ACL_UDF_RULE_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "enable"))
    {
        rule.udf_rule_en = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "disable"))
    {
        rule.udf_rule_en = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (TRUE == rule.udf_rule_en)
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "field", tmp_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tmp_str, "mac"))
        {
            rule.udf_pkt_type = AIR_ACL_MAC_HEADER;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "l2-payload"))
        {
            rule.udf_pkt_type = AIR_ACL_L2_PAYLOAD;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "ipv4"))
        {
            rule.udf_pkt_type = AIR_ACL_IPV4_HEADER;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "ipv6"))
        {
            rule.udf_pkt_type = AIR_ACL_IPV6_HEADER;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "l3-payload"))
        {
            rule.udf_pkt_type = AIR_ACL_L3_PAYLOAD;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "tcp"))
        {
            rule.udf_pkt_type = AIR_ACL_TCP_HEADER;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "udp"))
        {
            rule.udf_pkt_type = AIR_ACL_UDP_HEADER;
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "l4-payload"))
        {
            rule.udf_pkt_type = AIR_ACL_L4_PAYLOAD;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }

        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "word-ofst", &tmp), token_idx, 2);
        rule.word_ofst = tmp;
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "cmp", tmp_str), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tmp_str, "pattern"))
        {
            rule.cmp_sel = 0;
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cmp-key"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "cmp-key", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.cmp_pat = tmp;
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "cmp-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.cmp_mask = tmp;
            }
            else
            {
                osal_printf("***Error***, when cmp=pattern, the corresponding setting is cmp-key&cmp-mask\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else if (AIR_E_OK == dsh_checkString(tmp_str, "threshold"))
        {
            rule.cmp_sel = 1;
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cmp-low"))
            {
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "cmp-low", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.cmp_pat = tmp;
                DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "cmp-high", &tmp, sizeof(UI32_T)), token_idx, 2);
                rule.cmp_mask = tmp;
            }
            else
            {
                osal_printf("***Error***, when cmp=threshold, the corresponding setting is cmp-low&cmp-high\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }

        DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &rule.portmap), token_idx, 2);
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_acl_setUdfRule(unit, entry_id, &rule);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add acl udf rule error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showUdfRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_ACL_UDF_RULE_T rule;
    UI32_T             entry_id = 0;
    BOOL_T             mode = 0;
    AIR_PORT_BITMAP_T  pbm = {0};

    /* cmd: acl show udf-rule [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getUdfRule(unit, entry_id, &rule);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        mode = rule.udf_rule_en;
        if (FALSE == mode)
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(mode));
        }
        else
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(mode));
            AIR_PORT_BITMAP_COPY(pbm, rule.portmap);
            _port_cmd_printPortList("portlist:  ", &pbm[0]);
            osal_printf("field:     ");
            switch (rule.udf_pkt_type)
            {
                case (AIR_ACL_MAC_HEADER):
                    osal_printf("mac\n");
                    break;
                case (AIR_ACL_L2_PAYLOAD):
                    osal_printf("l2-payload\n");
                    break;
                case (AIR_ACL_IPV4_HEADER):
                    osal_printf("ipv4\n");
                    break;
                case (AIR_ACL_IPV6_HEADER):
                    osal_printf("ipv6\n");
                    break;
                case (AIR_ACL_L3_PAYLOAD):
                    osal_printf("l3-payload\n");
                    break;
                case (AIR_ACL_TCP_HEADER):
                    osal_printf("tcp\n");
                    break;
                case (AIR_ACL_UDP_HEADER):
                    osal_printf("udp\n");
                    break;
                case (AIR_ACL_L4_PAYLOAD):
                    osal_printf("l4-payload\n");
                    break;
                default:
                    osal_printf("error\n");
                    break;
            }
            osal_printf("word-ofst: %u\n", rule.word_ofst);
            if (0 == rule.cmp_sel)
            {
                osal_printf("cmp-key:   0x%x\n", rule.cmp_pat);
                osal_printf("cmp-mask:  0x%x\n", rule.cmp_mask);
            }
            else
            {
                osal_printf("cmp-low:   0x%x\n", rule.cmp_pat);
                osal_printf("cmp-high:  0x%x\n", rule.cmp_mask);
            }
        }
    }
    else
    {
        osal_printf("***Error***, get acl udf rule %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delUdfRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         entry_id = 0;

    /* cmd: acl del udf-rule [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_delUdfRule(unit, entry_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete acl udf rule %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delUdfRuleAll(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: acl del udf-rule-all [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_clearUdfRule(unit);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, clear acl udf rule error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showBlockCombination(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    UI32_T          tmp = 0;
    AIR_ACL_BLOCK_T block = 0;
    BOOL_T          mode = 0;

    /* cmd: acl show block-combination [ unit=<UINT> ] block-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "block-id", &tmp), token_idx, 2);
    block = tmp;
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getBlockState(unit, block, &mode);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("block_%d combination mode: %s\n", block, AIR_CMD_ENABLE_STATE(mode));
    }
    else if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, not support block combination on this chip\n");
    }
    else
    {
        osal_printf("***Error***, get acl block combination mode error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setBlockCombination(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    C8_T            tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T          tmp = 0;
    AIR_ACL_BLOCK_T block = 0;
    BOOL_T          state = 0;

    /* cmd: acl set block-combination [ unit=<UINT> ] block-id=<UINT> mode={ enable | disable }
     */

    /* "mode=enable" means this block can combine with next block */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "block-id", &tmp), token_idx, 2);
    block = tmp;
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", tmp_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(tmp_str, "enable"))
    {
        state = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    rc = air_acl_setBlockState(unit, block, state);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, not support block combination on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set acl block combination mode error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showMeterTable(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         mode = 0;
    UI32_T         meter_id = 0;
    UI32_T         rate = 0;

    /* cmd: acl show meter [ unit=<UINT> ] meter-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "meter-id", &meter_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getMeterTable(unit, meter_id, &mode, &rate);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("meter id:  %d\n", meter_id);
        osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(mode));
        if ((TRUE == mode) || (0 != rate))
        {
            osal_printf("ratelimit: %dkbps\n", rate * 64);
        }
    }
    else
    {
        osal_printf("***Error***, get meter table error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setMeterTable(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         meter_id = 0;
    UI32_T         rate = 0;
    C8_T           mode_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         mode = 0;

    /* cmd: acl set meter [ unit=<UINT> ] meter-id=<UINT> mode={ enable | disable } [rate=<UINT>]
     * Note: limit rate = rate * 64Kbps
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "meter-id", &meter_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
    {
        mode = TRUE;
        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rate"))
        {
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "rate", &rate), token_idx, 2);
        }
        else
        {
            osal_printf("***Error***, when mode=enable, please set rate\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
    {
        mode = FALSE;
        if (NULL != tokens[token_idx])
        {
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rate"))
            {
                DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "rate", &rate), token_idx, 2);
            }
            else
            {
                osal_printf("***Error***, invalid parameter\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_setMeterTable(unit, meter_id, mode, rate);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set meter table error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showDropMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    BOOL_T            state = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            port = 0;

    /* cmd: acl show drop-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, (AIR_PORT_BITMAP_T *)&pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %9s\n", "unit/port", "drop-en");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_acl_getDropEn(unit, port, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %11s\n", AIR_CMD_ENABLE_STATE(state));
        }
        else
        {
            osal_printf("***Error***, get acl port %d drop mode error\n", port);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setDropMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    C8_T              state_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T            mode = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            port = 0;

    /* cmd: acl set drop-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, (AIR_PORT_BITMAP_T *)&pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", state_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(state_str, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_setDropEn(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set acl port %d drop mode error\n", port);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showDropThreshold(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_PORT_BITMAP_T  pbm = {0};
    UI32_T             port = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_ACL_DP_COLOR_T color = 0;
    UI32_T             queue = 0, high = 0, low = 0;

    /* cmd: acl show drop-threshold [ unit=<UINT> ] portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, (AIR_PORT_BITMAP_T *)&pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "color", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "green"))
    {
        color = AIR_ACL_DP_COLOR_GREEN;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "yellow"))
    {
        color = AIR_ACL_DP_COLOR_YELLOW;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "red"))
    {
        color = AIR_ACL_DP_COLOR_RED;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_getDropThrsh(unit, port, color, queue, &high, &low);
        if (AIR_E_OK == rc)
        {
            osal_printf("(port %u, color:%s, queue:%u):\n", port, tmp_str, queue);
            osal_printf("\tlow threshold :%u\n", low);
            osal_printf("\thigh threshold  :%u\n", high);
        }
        else
        {
            osal_printf("***Error***, get acl drop threshold error\n");
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setDropThreshold(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_BITMAP_T  pbm = {0};
    UI32_T             port = 0;
    AIR_ACL_DP_COLOR_T color = 0;
    UI32_T             queue = 0, high = 0, low = 0;

    /* cmd: acl set drop-threshold [ unit=<UINT> ] portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>
     *      low=<UINT> high=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, (AIR_PORT_BITMAP_T *)&pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "color", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "green"))
    {
        color = AIR_ACL_DP_COLOR_GREEN;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "yellow"))
    {
        color = AIR_ACL_DP_COLOR_YELLOW;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "red"))
    {
        color = AIR_ACL_DP_COLOR_RED;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "low", &low), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "high", &high), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    if (high < low)
    {
        osal_printf("***Error***, high threshold must be greater than or equal to low threshold\n");
    }
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_setDropThrsh(unit, port, color, queue, high, low);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set acl drop threshold on port %d %s in queue %d error\n", port, tmp_str, queue);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showDropPbb(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_PORT_BITMAP_T  pbm = {0};
    UI32_T             port = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_ACL_DP_COLOR_T color = 0;
    UI32_T             queue = 0, pbb = 0;

    /* cmd: acl show drop-pbb [ unit=<UINT> ] portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, (AIR_PORT_BITMAP_T *)&pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "color", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "green"))
    {
        color = AIR_ACL_DP_COLOR_GREEN;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "yellow"))
    {
        color = AIR_ACL_DP_COLOR_YELLOW;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "red"))
    {
        color = AIR_ACL_DP_COLOR_RED;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_getDropPbb(unit, port, color, queue, &pbb);
        if (AIR_E_OK == rc)
        {
            osal_printf("(port %u, color:%s, queue:%u):\n", port, tmp_str, queue);
            osal_printf("\tdrop probability :%d(unit=1/1023)\n", pbb);
        }
        else
        {
            osal_printf("***Error***, get acl drop probability error\n");
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setDropPbb(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_BITMAP_T  pbm = {0};
    UI32_T             port = 0;
    AIR_ACL_DP_COLOR_T color = 0;
    UI32_T             queue = 0, pbb = 0;

    /* cmd: acl set drop-pbb [ unit=<UINT> ] portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>
     *      probability=<UINT>
     * Note: Drop probability = probability/1023
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "color", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "green"))
    {
        color = AIR_ACL_DP_COLOR_GREEN;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "yellow"))
    {
        color = AIR_ACL_DP_COLOR_YELLOW;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "red"))
    {
        color = AIR_ACL_DP_COLOR_RED;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "probability", &pbb), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_acl_setDropPbb(unit, port, color, queue, pbb);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set acl drop probability error\n");
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showDropMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         type = 0;

    /* cmd: acl show drop-mgmt-frame [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getDropExMfrm(unit, &type);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("acl management frame drop type: %s\n", type ? "exclude" : "include");
    }
    else
    {
        osal_printf("***Error***, get acl management frame drop type error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setDropMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           type_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         type = 0;

    /* cmd: acl set drop-mgmt-frame [ unit=<UINT> ] type={ include | exclude }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(type_str, "exclude"))
    {
        type = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "include"))
    {
        type = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_acl_setDropExMfrm(unit, type);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set acl drop exclude management frames error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showTrtcmMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         mode = 0;

    /* cmd: acl show trtcm-mode [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getTrtcmEn(unit, &mode);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("trtcm mode: %s\n", mode ? "enable" : "disable");
    }
    else
    {
        osal_printf("***Error***, get acl trtcm mode error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setTrtcmMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           state_str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         mode = 0;

    /* cmd: acl set trtcm-mode [ unit=<UINT> ] mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", state_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(state_str, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_acl_setTrtcmEn(unit, mode);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set acl trtcm mode error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showTrtcmRate(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    UI32_T          trtcm_id = 0;
    AIR_ACL_TRTCM_T trtcm;

    /* cmd: acl get trtcm-rate [ unit=<UINT> ] trtcm-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "trtcm-id", &trtcm_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getTrtcmTable(unit, trtcm_id, &trtcm);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("trtcm id:  %d\n", trtcm_id);
        osal_printf("cir: %u (unit:64Kbps)\n", trtcm.cir);
        osal_printf("pir: %u (unit:64Kbps)\n", trtcm.pir);
        osal_printf("cbs: %u (unit:byte)\n", trtcm.cbs);
        osal_printf("pbs: %u (unit:byte)\n", trtcm.pbs);
    }
    else
    {
        osal_printf("***Error***, get trtcm rate error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setTrtcmRate(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    UI32_T          trtcm_id = 0;
    UI32_T          cir = 0, pir = 0, cbs = 0, pbs = 0;
    AIR_ACL_TRTCM_T trtcm;

    /* cmd: acl set trtcm-rate [ unit=<UINT> ] trtcm-id=<UINT> cir=<UINT> pir=<UINT> cbs=<UINT> pbs=<UINT>
     * Note: cir/pir unit is 64kbps, cbs/pbs unit is byte
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "trtcm-id", &trtcm_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "cir", &cir), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pir", &pir), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "cbs", &cbs), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pbs", &pbs), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    trtcm.cir = cir;
    trtcm.pir = pir;
    trtcm.cbs = cbs;
    trtcm.pbs = pbs;

    rc = air_acl_setTrtcmTable(unit, trtcm_id, &trtcm);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set trtcm rate error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showMacChg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_MAC_T      mac;
    UI32_T         mac_sel = 0, id = 0;

    /* cmd: acl show mac-chg [ unit=<UINT> ] mac-sel={ src | dst } id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mac-sel", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "src"))
    {
        mac_sel = AIR_RAT_MAC_SEL_SA;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "dst"))
    {
        mac_sel = AIR_RAT_MAC_SEL_DA;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getL3RoutingTable(unit, mac_sel, id, mac);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf(" - mac-sel = %s\n", (mac_sel) ? "dst" : "src");
        osal_printf(" - id = %d\n", id);
        osal_printf(" - mac = %02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, not support mac change on this chip\n");
    }
    else
    {
        osal_printf("***Error***, get mac-chg error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_setMacChg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_MAC_T      mac;
    UI32_T         mac_sel = 0, id = 0;

    /* cmd: acl set mac-chg [ unit=<UINT> ] mac-sel={ src | dst } id=<UINT> mac=<MACADDR>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mac-sel", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "src"))
    {
        mac_sel = AIR_RAT_MAC_SEL_SA;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "dst"))
    {
        mac_sel = AIR_RAT_MAC_SEL_DA;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "id", &id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "mac", &mac), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_setL3RoutingTable(unit, mac_sel, id, mac);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, not support mac change on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set mac-chg error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showArpRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    UI32_T             i, entry_id = 0;
    AIR_ACL_ARP_RULE_T rule;

    /* cmd: acl show arp-rule [ unit=<UINT> ] entry-id=<UINT>
     */
    osal_memset(&rule, 0, sizeof(AIR_ACL_ARP_RULE_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_getArpRule(unit, entry_id, &rule);
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        if (FALSE == rule.rule_en)
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(rule.rule_en));
        }
        else
        {
            osal_printf("entry-id:  %d\n", entry_id);
            osal_printf("mode:      %s\n", AIR_CMD_ENABLE_STATE(rule.rule_en));
            osal_printf("reverse:   %d\n", rule.reverse);
            osal_printf("rule-end:  %d\n", rule.end);
            osal_printf("end-reverse:   %d\n", rule.end_reverse);
            _port_cmd_printPortList("portlist:  ", &(rule.portmap[0]));

            for (i = AIR_ACL_ARP_DMAC_KEY; i < AIR_ACL_ARP_RULE_FIELD_LAST; i++)
            {
                if BIT_CHK (rule.field_valid, i)
                {
                    switch (i)
                    {
                        case AIR_ACL_ARP_DMAC_KEY:
                            osal_printf("dmac: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.dmac[0], rule.key.dmac[1],
                                        rule.key.dmac[2], rule.key.dmac[3], rule.key.dmac[4], rule.key.dmac[5]);
                            osal_printf(", dmac-mask: 0x%x\n", rule.mask.dmac);
                            break;
                        case AIR_ACL_ARP_SMAC_KEY:
                            osal_printf("smac: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.smac[0], rule.key.smac[1],
                                        rule.key.smac[2], rule.key.smac[3], rule.key.smac[4], rule.key.smac[5]);
                            osal_printf(", smac-mask: 0x%x\n", rule.mask.smac);
                            break;
                        case AIR_ACL_ARP_STAG_KEY:
                            osal_printf("stag: 0x%x, stag-mask: 0x%x\n", rule.key.stag, rule.mask.stag);
                            break;
                        case AIR_ACL_ARP_CTAG_KEY:
                            osal_printf("ctag: 0x%x, ctag-mask: 0x%x\n", rule.key.ctag, rule.mask.ctag);
                            break;
                        case AIR_ACL_ARP_THA_KEY:
                            osal_printf("tha: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.tha[0], rule.key.tha[1],
                                        rule.key.tha[2], rule.key.tha[3], rule.key.tha[4], rule.key.tha[5]);
                            osal_printf(", tha-mask: 0x%x\n", rule.mask.tha);
                            break;
                        case AIR_ACL_ARP_SHA_KEY:
                            osal_printf("sha: ");
                            osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", rule.key.sha[0], rule.key.sha[1],
                                        rule.key.sha[2], rule.key.sha[3], rule.key.sha[4], rule.key.sha[5]);
                            osal_printf(", sha-mask: 0x%x\n", rule.mask.sha);
                            break;
                        case AIR_ACL_ARP_DIP_KEY:
                            osal_printf("%s: ", "dip");
                            dsh_showIpv4Addr(&rule.key.dip);
                            osal_printf(", dip-mask: 0x%x\n", rule.mask.dip);
                            break;
                        case AIR_ACL_ARP_SIP_KEY:
                            osal_printf("%s: ", "sip");
                            dsh_showIpv4Addr(&rule.key.sip);
                            osal_printf(", sip-mask: 0x%x\n", rule.mask.sip);
                            break;
                        case AIR_ACL_ARP_UDF_KEY:
                            osal_printf("udf-list: 0x%x, reverse: 0x%x\n", rule.mask.udf,
                                        (rule.key.udf ^ rule.mask.udf));
                            break;
                        default:
                            osal_printf("error\n");
                            break;
                    }
                }
            }
        }
    }
    else
    {
        osal_printf("***Error***, get acl arp rule %d error\n", entry_id);
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_addArpRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_ACL_ARP_RULE_T rule;
    UI32_T             entry_id = 0;
    C8_T               tmp_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T             tmp = 0;

    /* cmd: acl add arp-rule [ unit=<UINT> ] entry-id=<UINT>
     *         mode={ enable | disable } portlist=<UINTLIST>
     *         [ reverse ] [ rule-end ] [ end-reverse ]
     *         [ dmac=<MACADDR> [ dmac-mask=<HEX> ] ]
     *         [ smac=<MACADDR> [ smac-mask=<HEX> ] ]
     *         [ stag=<HEX> [ stag-mask=<HEX> ] ]
     *         [ ctag=<HEX> [ ctag-mask=<HEX> ] ]
     *         [ tha=<MACADDR> [ tha-mask=<HEX> ] ]
     *         [ sha=<MACADDR> [ sha-mask=<HEX> ] ]
     *         [ dip=<IPADDR> [ dip-mask=<HEX> ] ]
     *         [ sip=<IPADDR> [ sip-mask=<HEX> ] ]
     *         [ udf-list=<UINTLIST> [ reverse=<UINTLIST> ] ]
     */

    osal_memset(&rule, 0, sizeof(AIR_ACL_ARP_RULE_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", tmp_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tmp_str, "enable"))
    {
        rule.rule_en = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tmp_str, "disable"))
    {
        rule.rule_en = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &rule.portmap), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "reverse"))
    {
        rule.reverse = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "rule-end"))
    {
        rule.end = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "end-reverse"))
    {
        rule.end_reverse = TRUE;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dmac"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "dmac", (AIR_MAC_T *)rule.key.dmac), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dmac-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dmac-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.dmac = tmp;
        }
        else
        {
            rule.mask.dmac = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_DMAC_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "smac"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "smac", (AIR_MAC_T *)rule.key.smac), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "smac-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "smac-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.smac = tmp;
        }
        else
        {
            rule.mask.smac = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_SMAC_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "stag"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "stag", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.stag = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "stag-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "stag-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.stag = tmp;
        }
        else
        {
            rule.mask.stag = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_STAG_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ctag"))
    {
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ctag", &tmp, sizeof(UI32_T)), token_idx, 2);
        rule.key.ctag = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ctag-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ctag-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.ctag = tmp;
        }
        else
        {
            rule.mask.ctag = 0x3;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_CTAG_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tha"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "tha", (AIR_MAC_T *)rule.key.tha), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "tha-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "tha-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.tha = tmp;
        }
        else
        {
            rule.mask.tha = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_THA_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sha"))
    {
        DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "sha", (AIR_MAC_T *)rule.key.sha), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sha-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sha-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.sha = tmp;
        }
        else
        {
            rule.mask.sha = 0x3f;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_SHA_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip"))
    {
        DSH_CHECK_PARAM(dsh_getIpv4Addr(tokens, token_idx, "dip", &(rule.key.dip)), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "dip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.dip = tmp;
        }
        else
        {
            rule.mask.dip = 0xf;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_DIP_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip"))
    {
        DSH_CHECK_PARAM(dsh_getIpv4Addr(tokens, token_idx, "sip", &(rule.key.sip)), token_idx, 2);

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip-mask"))
        {
            DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sip-mask", &tmp, sizeof(UI32_T)), token_idx, 2);
            rule.mask.sip = tmp;
        }
        else
        {
            rule.mask.sip = 0xf;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_SIP_KEY);
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "udf-list"))
    {
        DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "udf-list", &tmp, 1), token_idx, 2);
        rule.mask.udf = tmp;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "reverse"))
        {
            DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "reverse", &tmp, 1), token_idx, 2);
            rule.key.udf = (~tmp) & rule.mask.udf;
        }
        else
        {
            rule.key.udf = rule.mask.udf;
        }

        rule.field_valid |= (1U << AIR_ACL_ARP_UDF_KEY);
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_acl_setArpRule(unit, entry_id, &rule);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, this rule type is not supported on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add acl arp rule error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_delArpRule(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         entry_id = 0;

    /* cmd: acl del arp-rule [ unit=<UINT> ] entry-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "entry-id", &entry_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_acl_delArpRule(unit, entry_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete acl arp rule %d error\n", entry_id);
    }
    return rc;
}

static void
_acl_printHeader(
    const UI32_T unit)
{
    osal_printf("unit %d\n", unit);
    osal_printf(" %8s  %6s  %6s  %8s  %11s  %s\n", "entry-id", "action", "policy", "rule-end", "end-reverse",
                "portlist");
}

static AIR_ERROR_NO_T
_acl_cmd_dumpRuleAction(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    AIR_ACL_RULE_T     rule;
    AIR_ACL_ARP_RULE_T arp_rule;
    AIR_ACL_ACTION_T   action;
    UI32_T             entry_id = 0, capacity = 0;
    BOOL_T             mode = FALSE, prn_first = TRUE;
    BOOL_T             reverse = FALSE, rule_end = FALSE, end_reverse = FALSE;
    C8_T               pbmp_str[CMLIB_UTIL_IP_ADDR_STR_SIZE] = {0};
    /* cmd: acl dump rule-action-all [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_getCapacity(unit, AIR_SWC_RSRC_ACL_RULE, 0, &capacity);
    if ((AIR_E_OK == rc) && (capacity > 0))
    {
        while (entry_id < capacity)
        {
            /* rule */
            osal_memset(&rule, 0, sizeof(AIR_ACL_RULE_T));
            rc = air_acl_getRule(unit, entry_id, &rule);
            if (AIR_E_OK == rc)
            {
                mode = rule.rule_en;
                if (TRUE == mode)
                {
                    reverse = rule.reverse;
                    rule_end = rule.end;
                    end_reverse = rule.end_reverse;
                    CMD_STRING_PORTLIST(pbmp_str, rule.portmap);
                }
            }
            else
            {
                osal_memset(&arp_rule, 0, sizeof(AIR_ACL_ARP_RULE_T));
                rc = air_acl_getArpRule(unit, entry_id, &arp_rule);
                if (AIR_E_OK == rc)
                {
                    mode = arp_rule.rule_en;
                    if (TRUE == mode)
                    {
                        reverse = arp_rule.reverse;
                        rule_end = arp_rule.end;
                        end_reverse = arp_rule.end_reverse;
                        CMD_STRING_PORTLIST(pbmp_str, arp_rule.portmap);
                    }
                }
            }
            /* action */
            if ((AIR_E_OK == rc) && (TRUE == mode))
            {
                rc = air_acl_getAction(unit, entry_id, &action);
            }
            /* print enabled acl rule */
            if ((AIR_E_OK == rc) && (TRUE == mode))
            {
                if (TRUE == prn_first)
                {
                    _acl_printHeader(unit);
                    prn_first = FALSE;
                }
                /* entry-id  action  policy  rule-end  end-reverse  portlist */
                osal_printf(" %8u  %6s  %6s  %8s  %11s  %s\n", entry_id, (0 != action.field_valid) ? " v " : "---",
                            reverse ? "deny" : "permit", rule_end ? " v " : "---", end_reverse ? " v " : "---",
                            pbmp_str);
            }
            entry_id++;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_acl_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         size = 0, count = 0;
    C8_T           type_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T         type = AIR_SWC_RSRC_LAST;

    /*
     * Command format
     * acl show resource [ unit=<UINT> ] [ type={ rule | action | udf | counter | meter | trtcm } ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (NULL != tokens[token_idx])
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
        rc = AIR_E_BAD_PARAMETER;
        for (type = AIR_SWC_RSRC_ACL_RULE; type <= AIR_SWC_RSRC_ACL_TRTCM; type++)
        {
            if (AIR_E_OK == dsh_checkString(type_str, _acl_rsrc_type[type]))
            {
                rc = AIR_E_OK;
                break;
            }
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("%-25s %6s/%6s\n", "type", "used", "total");
        if (AIR_SWC_RSRC_LAST == type)
        {
            for (type = AIR_SWC_RSRC_ACL_RULE; type <= AIR_SWC_RSRC_ACL_TRTCM; type++)
            {
                size = 0;
                count = 0;
                rc = air_swc_getCapacity(unit, type, 0, &size);
                if (AIR_E_OK == rc)
                {
                    rc = air_swc_getUsage(unit, type, 0, &count);
                    if (AIR_E_OK == rc)
                    {
                        osal_printf("%-25s %6u/%6u\n", _acl_rsrc_type[type], count, size);
                    }
                    else
                    {
                        osal_printf("%-25s %6s/%6u\n", _acl_rsrc_type[type], "---", size);
                    }
                }
            }
        }
        else
        {
            rc = air_swc_getCapacity(unit, type, 0, &size);
            if (AIR_E_OK == rc)
            {
                rc = air_swc_getUsage(unit, type, 0, &count);
                if (AIR_E_OK == rc)
                {
                    osal_printf("%-25s %6u/%6u\n", _acl_rsrc_type[type], count, size);
                }
                else
                {
                    osal_printf("%-25s %6s/%6u\n", _acl_rsrc_type[type], "---", size);
                }
            }
        }
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, acl show resource may not fully support in this chip\n");
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _acl_cmd_vec[] =
{
    {
        "set global-ctrl", 2, _acl_cmd_setGlobalCtrl,
        "acl set global-ctrl [ unit=<UINT> ] mode={ enable | disable }\n"
    },
    {
        "show global-ctrl", 2, _acl_cmd_showGlobalCtrl,
        "acl show global-ctrl [ unit=<UINT> ]\n"
    },
    {
        "set port-ctrl", 2, _acl_cmd_setPortCtrl,
        "acl set port-ctrl [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show port-ctrl", 2, _acl_cmd_showPortCtrl,
        "acl show port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "add rule", 2, _acl_cmd_addRule,
        "acl add rule [ unit=<UINT> ] entry-id=<UINT>\n"
        "mode={ enable | disable } portlist=<UINTLIST>\n"
        "[ isipv6 ] [ reverse ] [ rule-end ] [ end-reverse ]\n"
        "[ dmac=<MACADDR> [ dmac-mask=<HEX> ] ]\n"
        "[ smac=<MACADDR> [ smac-mask=<HEX> ] ]\n"
        "[ ethertype=<HEX> [ ethertype-mask=<HEX> ] ]\n"
        "[ stag=<HEX> [ stag-mask=<HEX> ] ]\n"
        "[ ctag=<HEX> [ ctag-mask=<HEX> ] ]\n"
        "[ dip=<IPADDR> [ dip-mask=<HEX> ] ]\n"
        "[ sip=<IPADDR> [ sip-mask=<HEX> ] ]\n"
        "[ dscp=<HEX> ]\n"
        "[ next-header=<HEX> ]\n"
        "[ flow-label=<HEX> [ flow-label-mask=<HEX> ] ]\n"
        "[ dport=<HEX> [ dport-mask=<HEX> ] ]\n"
        "[ sport=<HEX> [ sport-mask=<HEX> ] ]\n"
        "[ udf-list=<UINTLIST> [ reverse=<UINTLIST> ] ]\n"
    },
    {
        "del rule", 2, _acl_cmd_delRule,
        "acl del rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "show rule", 2, _acl_cmd_showRule,
        "acl show rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "add arp-rule", 2, _acl_cmd_addArpRule,
        "acl add arp-rule [ unit=<UINT> ] entry-id=<UINT>\n"
        "mode={ enable | disable } portlist=<UINTLIST>\n"
        "[ reverse ] [ rule-end ] [ end-reverse ]\n"
        "[ dmac=<MACADDR> [ dmac-mask=<HEX> ] ]\n"
        "[ smac=<MACADDR> [ smac-mask=<HEX> ] ]\n"
        "[ stag=<HEX> [ stag-mask=<HEX> ] ]\n"
        "[ ctag=<HEX> [ ctag-mask=<HEX> ] ]\n"
        "[ tha=<MACADDR> [ tha-mask=<HEX> ] ]\n"
        "[ sha=<MACADDR> [ sha-mask=<HEX> ] ]\n"
        "[ dip=<IPADDR> [ dip-mask=<HEX> ] ]\n"
        "[ sip=<IPADDR> [ sip-mask=<HEX> ] ]\n"
        "[ udf-list=<UINTLIST> [ reverse=<UINTLIST> ] ]\n"
    },
    {
        "del arp-rule", 2, _acl_cmd_delArpRule,
        "acl del arp-rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "show arp-rule", 2, _acl_cmd_showArpRule,
        "acl show arp-rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "add action", 2, _acl_cmd_addAction,
        "acl add action [ unit=<UINT> ] entry-id=<UINT>\n"
        "[ force-port={ dst | vlan } portlist=<UINTLIST> ]\n"
        "[ mib-id=<UINT> ]\n"
        "[ attack-rate-id=<UINT> ]\n"
        "[ meter-id=<UINT> ]\n"
        "[ port-forward=\n"
        "{ default | cpu-port-excluded | cpu-port-included | cpu-port-only | drop } ]\n"
        "[ mirror-port=<HEX> ]\n"
        "[ user-pri=<UINT> ]\n"
        "[ egrs-tag={ default | consistent | untagged | swap | tagged | stack } ]\n"
        "[ lky-vlan ]\n"
        "[ acl-mgmt ]\n"
        "[ drop-pcd={ original | defined }\n"
        "[ red-drop-pcd=<UINT> ] [ yellow-drop-pcd=<UINT> ] [ green-drop-pcd=<UINT> ] ]\n"
        "[ class-slr={ original | defined } [ class-slr=<UINT>] ]\n"
        "[ acl-tcm={ defined | trtcm }\n"
        "{ defined-color={ default | green | yellow | red } | trtcm-id=<UINT> } ]\n"
        "[ acl-vid=<UINT> ]\n"
        "[ mac-chg da-idx=<UINT> sa-idx=<UINT> ]\n"
        "[ dont-learn ]\n"
        "[ as-bpdu ]\n"
    },
    {
        "del action", 2, _acl_cmd_delAction,
        "acl del action [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "show action", 2, _acl_cmd_showAction,
        "acl show action [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "del rule-action-all", 2, _acl_cmd_delRuleActionAll,
        "acl del rule-action-all [ unit=<UINT> ]\n"
    },
    {
        "dump rule-action-all", 2, _acl_cmd_dumpRuleAction,
        "acl dump rule-action-all [ unit=<UINT> ]\n"
    },
    {
        "add udf-rule", 2, _acl_cmd_addUdfRule,
        "acl add udf-rule [ unit=<UINT> ] entry-id=<UINT>\n"
        "mode={ enable | disable }\n"
        "field={ mac | l2-payload | ipv4 | ipv6 | l3-payload | tcp | udp | l4-payload }\n"
        "word-ofst=<UINT>\n"
        "cmp={ pattern | threshold }\n"
        "{ cmp-key=<HEX> cmp-mask=<HEX> | cmp-low=<HEX> cmp-high=<HEX> }\n"
        "portlist=<UINTLIST>\n"
    },
    {
        "del udf-rule", 2, _acl_cmd_delUdfRule,
        "acl del udf-rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "del udf-rule-all", 2, _acl_cmd_delUdfRuleAll,
        "acl del udf-rule-all [ unit=<UINT> ]\n"
    },
    {
        "show udf-rule", 2, _acl_cmd_showUdfRule,
        "acl show udf-rule [ unit=<UINT> ] entry-id=<UINT>\n"
    },
    {
        "set mac-chg", 2, _acl_cmd_setMacChg,
        "acl set mac-chg [ unit=<UINT> ] mac-sel={ src | dst } id=<UINT> mac=<MACADDR>\n"
    },
    {
        "show mac-chg", 2, _acl_cmd_showMacChg,
        "acl show mac-chg [ unit=<UINT> ] mac-sel={ src | dst } id=<UINT>\n"
    },
    {
        "set meter", 2, _acl_cmd_setMeterTable,
        "acl set meter [ unit=<UINT> ] meter-id=<UINT> mode={ enable | disable }\n"
        "[ rate=<UINT> ]\n"
    },
    {
        "show meter", 2, _acl_cmd_showMeterTable,
        "acl show meter [ unit=<UINT> ] meter-id=<UINT>\n"
    },
    {
        "show mib-counter", 2, _acl_cmd_showMibCnt,
        "acl show mib-counter [ unit=<UINT> ] mib-id=<UINT>\n"
    },
    {
        "clear mib-counter", 2, _acl_cmd_clearMibCnt,
        "acl clear mib-counter [ unit=<UINT> ]\n"
    },
    {
        "set trtcm-mode", 2, _acl_cmd_setTrtcmMode,
        "acl set trtcm-mode [ unit=<UINT> ] mode={ enable | disable }\n"
    },
    {
        "show trtcm-mode", 2, _acl_cmd_showTrtcmMode,
        "acl show trtcm-mode [ unit=<UINT> ]\n"
    },
    {
        "set trtcm-rate", 2, _acl_cmd_setTrtcmRate,
        "acl set trtcm-rate [ unit=<UINT> ] trtcm-id=<UINT>\n"
        "cir=<UINT> pir=<UINT> cbs=<UINT> pbs=<UINT>\n"
    },
    {
        "show trtcm-rate", 2, _acl_cmd_showTrtcmRate,
        "acl show trtcm-rate [ unit=<UINT> ] trtcm-id=<UINT>\n"
    },
    {
        "set drop-mode", 2, _acl_cmd_setDropMode,
        "acl set drop-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show drop-mode", 2, _acl_cmd_showDropMode,
        "acl show drop-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set drop-threshold", 2, _acl_cmd_setDropThreshold,
        "acl set drop-threshold [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>\n"
        "low=<UINT> high=<UINT>\n"
    },
    {
        "show drop-threshold", 2, _acl_cmd_showDropThreshold,
        "acl show drop-threshold [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>\n"
    },
    {
        "set drop-pbb", 2, _acl_cmd_setDropPbb,
        "acl set drop-pbb [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>\n"
        "probability=<UINT>\n"
    },
    {
        "show drop-pbb", 2, _acl_cmd_showDropPbb,
        "acl show drop-pbb [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> color={ green | yellow | red } queue=<UINT>\n"
    },
    {
        "set drop-mgmt-frame", 2, _acl_cmd_setDropMgmtFrame,
        "acl set drop-mgmt-frame [ unit=<UINT> ] type={ include | exclude }\n"
    },
    {
        "show drop-mgmt-frame", 2, _acl_cmd_showDropMgmtFrame,
        "acl show drop-mgmt-frame [ unit=<UINT> ]\n"
    },
    {
        "show resource", 2, _acl_cmd_showResource,
        "acl show resource [ unit=<UINT> ] [ type={ rule | action | udf | counter |\n"
        "meter | trtcm } ]\n"
    },
    {
        "set block-combination", 2, _acl_cmd_setBlockCombination,
        "acl set block-combination [ unit=<UINT> ] block-id=<UINT>\n"
        "mode={ enable | disable }\n"
    },
    {
        "show block-combination", 2, _acl_cmd_showBlockCombination,
        "acl show block-combination [ unit=<UINT> ] block-id=<UINT>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
acl_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _acl_cmd_vec, sizeof(_acl_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
acl_cmd_usager()
{
    return (dsh_usager(_acl_cmd_vec, sizeof(_acl_cmd_vec) / sizeof(DSH_VEC_T)));
}
