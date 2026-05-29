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

#include <cmd/stag_cmd.h>

#include <air_port.h>
#include <air_stag.h>
#include <air_vlan.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define STAG_DEFAULT_TAG_LEN             (4)
#define STAG_OPC_PORTID_MAX_PORT_NUM_LEN (1)

/* MACRO FUNCTION DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
const static C8_T *_stag_rsn[] = {"normal", "sflow", "ttl_err", "acl", "sa_full", "port_move_err", "unknown"};

const static C8_T *_stag_vpm[] = {"untagged", "8100", "88a8", "predefined", "unknown"};

static AIR_ERROR_NO_T
_stag_cmd_setTagCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port;
    AIR_PORT_BITMAP_T pbm;
    BOOL_T            stag_en;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        stag_en = FALSE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        stag_en = TRUE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_stag_setSpecialTagCfg(unit, port, stag_en);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port stag fail\n");
            break;
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_stag_cmd_getTagCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port;
    AIR_PORT_BITMAP_T pbm;
    BOOL_T            stag_en;
    C8_T             *stag_en_str;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %11s\n", "unit/port", "port-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_stag_getSpecialTagCfg(unit, port, &stag_en);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get-port stag fail\n");
            break;
        }

        if (FALSE == stag_en)
        {
            stag_en_str = "disable";
        }
        else if (TRUE == stag_en)
        {
            stag_en_str = "enable";
        }
        else
        {
            rc = AIR_E_BAD_PARAMETER;
            break;
        }
        osal_printf("%5d/%2d %13s\n", unit, port, stag_en_str);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_stag_cmd_setTagMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port;
    AIR_PORT_BITMAP_T pbm;
    UI32_T            mode;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    if (AIR_E_OK == dsh_checkString(str, "insert"))
    {
        mode = AIR_STAG_MODE_INSERT;
    }
    else if (AIR_E_OK == dsh_checkString(str, "replace"))
    {
        mode = AIR_STAG_MODE_REPLACE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_stag_setSpecialTagMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, stag set-mode fail\n");
            break;
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_stag_cmd_getTagMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port;
    AIR_PORT_BITMAP_T pbm;
    UI32_T            mode;
    C8_T             *mode_str;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %10s\n", "unit/port", "tag-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_stag_getSpecialTagMode(unit, port, &mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, stag get-mode fail\n");
            break;
        }

        if (AIR_STAG_MODE_INSERT == mode)
        {
            mode_str = "insert";
        }
        else if (AIR_STAG_MODE_REPLACE == mode)
        {
            mode_str = "replace";
        }
        else
        {
            rc = AIR_E_BAD_PARAMETER;
            break;
        }
        osal_printf("%5d/%2d %12s\n", unit, port, mode_str);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_stag_cmd_encodeTxStag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    UI32_T             mode;
    C8_T               str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T             pri;
    UI32_T             cfi;
    UI32_T             vid;
    UI32_T             port, port_cnt = 0;

    UI8_T              buf[STAG_DEFAULT_TAG_LEN] = {0};
    UI32_T             len = STAG_DEFAULT_TAG_LEN;
    AIR_STAG_TX_PARA_T stag_tx = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "insert"))
    {
        mode = AIR_STAG_MODE_INSERT;
    }
    else if (AIR_E_OK == dsh_checkString(str, "replace"))
    {
        mode = AIR_STAG_MODE_REPLACE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "opc", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "portmap"))
    {
        stag_tx.opc = AIR_STAG_OPC_PORTMAP;
    }
    else if (AIR_E_OK == dsh_checkString(str, "portid"))
    {
        stag_tx.opc = AIR_STAG_OPC_PORTID;
    }
    else if (AIR_E_OK == dsh_checkString(str, "lookup"))
    {
        stag_tx.opc = AIR_STAG_OPC_LOOKUP;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    /* check optional bypass-trunk setting */
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "bypass-trunk"))
    {
        token_idx += 1;
        if (AIR_STAG_OPC_PORTMAP == stag_tx.opc)
        {
            stag_tx.opc = AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK;
        }
        else if (AIR_STAG_OPC_PORTID == stag_tx.opc)
        {
            if (AIR_STAG_MODE_INSERT == mode)
            {
                osal_printf("***Error***, insert mode does not support the port id bypass-trunk feature.\n");
                return DSH_E_SYNTAX_ERR;
            }
            stag_tx.opc = AIR_STAG_OPC_PORTID_BYPASS_TRUNK;
        }
        else if (AIR_STAG_OPC_LOOKUP == stag_tx.opc)
        {
            return DSH_E_SYNTAX_ERR;
        }
        else
        {
            /* do nothing */
        }
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dp"))
    {
        if (AIR_STAG_OPC_LOOKUP == stag_tx.opc)
        {
            return DSH_E_SYNTAX_ERR;
        }

        DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "dp", unit, &stag_tx.pbm), token_idx, 2);

        if (AIR_STAG_OPC_PORTID == stag_tx.opc)
        {
            AIR_PORT_FOREACH(stag_tx.pbm, port)
            {
                port_cnt++;
            }

            if (port_cnt > STAG_OPC_PORTID_MAX_PORT_NUM_LEN)
            {
                return DSH_E_SYNTAX_ERR;
            }
        }
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "vpm"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "vpm", str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(str, "untagged"))
        {
            stag_tx.vpm = AIR_STAG_VPM_UNTAG;
        }
        else if (AIR_E_OK == dsh_checkString(str, "8100"))
        {
            stag_tx.vpm = AIR_STAG_VPM_TPID_8100;
        }
        else if (AIR_E_OK == dsh_checkString(str, "88a8"))
        {
            stag_tx.vpm = AIR_STAG_VPM_TPID_88A8;
        }
        else
        {
            return DSH_E_SYNTAX_ERR;
        }
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "pri"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
        if (pri >= AIR_VLAN_MAX_PRIORITY)
        {
            return DSH_E_SYNTAX_ERR;
        }
        stag_tx.pri = pri;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cfi"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "cfi", &cfi), token_idx, 2);
        if (cfi > AIR_VLAN_MAX_TAG_CFI)
        {
            return DSH_E_SYNTAX_ERR;
        }
        stag_tx.cfi = cfi;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "vid"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
        if (vid > AIR_VLAN_ID_MAX)
        {
            return DSH_E_SYNTAX_ERR;
        }
        stag_tx.vid = vid;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_stag_encodeTxStag(unit, mode, &stag_tx, buf, &len);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, stag encode fail\n");
        return (rc);
    }

    osal_printf("stag encode sucess, returned len=%d\n", len);
    osal_printf("encoded stag: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);

    return (rc);
}

static AIR_ERROR_NO_T
_stag_cmd_decodeRxStag(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    UI32_T             tmp;
    UI8_T              buf[STAG_DEFAULT_TAG_LEN] = {0};
    UI32_T             len = 0, i = 0;
    AIR_STAG_RX_PARA_T stag_rx = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    while (AIR_E_OK == dsh_checkString(tokens[token_idx], "byte"))
    {
        len++;
        if (len > STAG_DEFAULT_TAG_LEN)
        {
            return DSH_E_SYNTAX_ERR;
        }
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "byte", &tmp, sizeof(UI32_T)), token_idx, 2);
        if (tmp > 0xff)
        {
            return AIR_E_BAD_PARAMETER;
        }
        buf[len - 1] = (UI8_T)tmp;
    }

    if (0 == len)
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("stag to be decoded: ");
    for (i = 0; i < len; i++)
    {
        osal_printf("%02x ", buf[i]);
    }
    osal_printf("\n");

    rc = air_stag_decodeRxStag(unit, buf, len, &stag_rx);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, stag decode fail\n");
        return (rc);
    }

    osal_printf("stag decode success, len=%d:\n", len);
    osal_printf("rsn : %s\n", _stag_rsn[stag_rx.rsn]);
    osal_printf("vpm : %s\n", _stag_vpm[stag_rx.vpm]);
    osal_printf("sp  : %d\n", stag_rx.sp);
    osal_printf("pri : %d\n", stag_rx.pri);
    osal_printf("cfi : %d\n", stag_rx.cfi);
    osal_printf("vid : %d\n", stag_rx.vid);

    return (rc);
}

/* clang-format off */
/* -------------------------------------------------------------- callback */
const static DSH_VEC_T  _stag_cmd_vec[] =
{
    {
        "set tag-ctrl", 2, _stag_cmd_setTagCtrl,
        "stag set tag-ctrl [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show tag-ctrl", 2, _stag_cmd_getTagCtrl,
        "stag show tag-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set tag-mode", 2, _stag_cmd_setTagMode,
        "stag set tag-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ insert | replace }\n"
    },
    {
        "show tag-mode", 2, _stag_cmd_getTagMode,
        "stag show tag-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "encode", 1, _stag_cmd_encodeTxStag,
        "stag encode [ unit=<UINT> ] mode={ insert | replace }\n"
        "opc={ portmap [ bypass-trunk ] | portid [ bypass-trunk ] | lookup }\n"
        "[ dp=<UINTLIST> ] [ vpm={ untagged | 8100 | 88a8 } ]\n"
        "[ pri=<UINT> ] [ cfi=<UINT> ] [ vid=<UINT> ]\n"
    },
    {
        "decode", 1, _stag_cmd_decodeRxStag,
        "stag decode [ unit=<UINT> ] byte=<HEX> byte=<HEX> byte=<HEX> byte=<HEX>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
stag_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _stag_cmd_vec, sizeof(_stag_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
stag_cmd_usager()
{
    return (dsh_usager(_stag_cmd_vec, sizeof(_stag_cmd_vec) / sizeof(DSH_VEC_T)));
}
