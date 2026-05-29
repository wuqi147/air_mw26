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
#include <cmd/mir_cmd.h>

#include <air_error.h>
#include <air_mirror.h>
#include <air_port.h>
#include <air_types.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

static void
_mir_cmd_printPortList(
    const C8_T             *ptr_str,
    const AIR_PORT_BITMAP_T pbm)
{
    UI32_T cnt = 0, port = 0, first = 0, span = 0;

    CMLIB_PORT_BITMAP_COUNT(pbm, cnt);
    osal_printf("%-12s : ", ptr_str);
    if (0 == cnt)
    {
        osal_printf("null\n");
    }
    else
    {
        first = 1;
        CMLIB_BITMAP_BIT_FOREACH(pbm, port, AIR_PORT_BITMAP_SIZE)
        {
            osal_printf("%s%d", first ? "" : ",", port);
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

static void
_mir_cmd_printSrcPortList(
    const UI32_T unit,
    const UI32_T sessionid)
{
    UI32_T            port = 0;
    AIR_MIR_SESSION_T session;
    AIR_PORT_BITMAP_T txPbm = {0}, rxPbm = {0};

    AIR_PORT_FOREACH(HAL_PORT_BMP(unit), port)
    {
        osal_memset(&session, 0, sizeof(session));
        session.src_port = port;
        air_mir_getMirrorPort(unit, sessionid, &session);

        if (session.flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
        {
            AIR_PORT_ADD(txPbm, port);
        }
        if (session.flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
        {
            AIR_PORT_ADD(rxPbm, port);
        }
    }
    osal_printf("%-12s\n", "src portlist");
    _mir_cmd_printPortList("    rx", rxPbm);
    _mir_cmd_printPortList("    tx", txPbm);
}

static void
_mir_cmd_printSession(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{
    BOOL_T enable = FALSE;
    air_mir_getSessionAdminMode(unit, session_id, &enable);
    osal_printf("%-12s : %d\n", "session id", session_id);
    osal_printf("%-12s : %s \n", "mode", (enable == TRUE) ? "enable" : "disable");
    osal_printf("%-12s : %s \n", "tag obey",
                (ptr_session->flags & AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG) ? "egress" : "ingress");
    osal_printf("%-12s : %d \n", "dst port", ptr_session->dst_port);
    _mir_cmd_printSrcPortList(unit, session_id);
}

static AIR_ERROR_NO_T
_mir_cmd_addSessionSrcPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port = 0;
    UI32_T            session_id = 0;
    AIR_MIR_SESSION_T session;
    AIR_PORT_BITMAP_T srcPbm = {0};
    C8_T              dir_str[DSH_CMD_MAX_LENGTH];

    osal_memset(&session, 0, sizeof(session));
    osal_memset(dir_str, 0, sizeof(dir_str));

    /* paser port mirror diag command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &session_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &srcPbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", dir_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(srcPbm, port)
    {
        session.src_port = port;
        rc = air_mir_getMirrorPort(unit, session_id, &session);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,get port=%u error\n", port);
            return rc;
        }

        if (AIR_E_OK == dsh_checkString(dir_str, "ingress"))
        {
            session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
        }
        else if (AIR_E_OK == dsh_checkString(dir_str, "egress"))
        {
            session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
        }
        else if (AIR_E_OK == dsh_checkString(dir_str, "both"))
        {
            session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
            session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
        }
        else
        {
            return DSH_E_SYNTAX_ERR;
        }

        rc = air_mir_setMirrorPort(unit, session_id, &session);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,port=%u error\n", port);
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_mir_cmd_delSessionSrcPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port = 0;
    UI32_T            session_id = 0;
    AIR_MIR_SESSION_T session;
    AIR_PORT_BITMAP_T srcPbm = {0};

    osal_memset(&session, 0, sizeof(session));

    /* paser port mirror diag command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &session_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &srcPbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(srcPbm, port)
    {
        session.src_port = port;
        rc = air_mir_getMirrorPort(unit, session_id, &session);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,get port=%u error\n", port);
            return rc;
        }

        session.flags &= ~(AIR_MIR_SESSION_FLAGS_DIR_TX | AIR_MIR_SESSION_FLAGS_DIR_RX);

        session.src_port = port;
        rc = air_mir_setMirrorPort(unit, session_id, &session);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,port=%u error\n", port);
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_mir_cmd_addSession(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            session_id = 0;
    AIR_MIR_SESSION_T session;
    C8_T              mode_str[DSH_CMD_MAX_LENGTH], tx_tag_str[DSH_CMD_MAX_LENGTH], dir_str[DSH_CMD_MAX_LENGTH];
    AIR_PORT_BITMAP_T srcPbm = {0};
    UI32_T            port = 0;

    osal_memset(&session, 0, sizeof(session));
    osal_memset(mode_str, 0, sizeof(mode_str));
    osal_memset(tx_tag_str, 0, sizeof(tx_tag_str));
    osal_memset(dir_str, 0, sizeof(dir_str));

    /* default setting "ingress" */
    osal_strncpy(tx_tag_str, "ingress", sizeof("ingress"));

    /* paser port mirror diag command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &session_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "src-portlist", unit, &srcPbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", dir_str), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dst-port", &session.dst_port), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "tag-obey", tx_tag_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
    {
        session.flags &= ~AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK == dsh_checkString(dir_str, "ingress"))
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else if (AIR_E_OK == dsh_checkString(dir_str, "egress"))
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
    }
    else if (AIR_E_OK == dsh_checkString(dir_str, "both"))
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK == dsh_checkString(tx_tag_str, "egress"))
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }
    else if (AIR_E_OK == dsh_checkString(tx_tag_str, "ingress"))
    {
        session.flags &= ~AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    AIR_PORT_FOREACH(srcPbm, port)
    {
        session.src_port = port;
        /* set port mirror session */
        rc = air_mir_addSession(unit, session_id, &session);

        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,dst-port=%u, src-port=%u error\n", session.dst_port, session.src_port);
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_mir_cmd_delSession(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    UI32_T            session_id = 0;
    AIR_MIR_SESSION_T session;

    osal_memset(&session, 0, sizeof(session));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &session_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_mir_delSession(unit, session_id);

    /* clear tx rx portlist*/
    AIR_PORT_FOREACH(HAL_PORT_BMP(unit), port)
    {
        session.src_port = port;
        rc = air_mir_setMirrorPort(unit, session_id, &session);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,port=%u error\n", port);
            return rc;
        }
    }

    if (rc != AIR_E_OK)
    {
        osal_printf("***Error***, delete mirror session fail\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_mir_cmd_showSession(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            session_id = 0;
    AIR_MIR_SESSION_T session;

    osal_memset(&session, 0, sizeof(session));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &session_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_mir_getSession(unit, session_id, &session);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get mirror session fail\n");
        return rc;
    }
    /* print session information */
    if (session.dst_port == AIR_PORT_INVALID)
    {
        osal_printf("session id %d not found\n", session_id);
    }
    else
    {
        osal_printf("%-12s : %d\n", "unit", unit);
        _mir_cmd_printSession(unit, session_id, &session);
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T _mir_cmd_vec[] =
{
    {
        "add session", 2, _mir_cmd_addSession,
        "mir add session [ unit=<UINT> ] sid=<UINT> mode={ enable | disable }\n"
        "src-portlist=<UINTLIST> dir={ ingress | egress | both } dst-port=<UINT>\n"
        "[ tag-obey={ ingress | egress } ]\n"
    },
    {
        "del session", 2, _mir_cmd_delSession,
        "mir del session [ unit=<UINT> ] sid=<UINT>\n"
    },
    {
        "show session", 2, _mir_cmd_showSession,
        "mir show session [ unit=<UINT> ] sid=<UINT>\n"
    },
    {
        "add session-src-port", 2, _mir_cmd_addSessionSrcPort,
        "mir add session-src-port [ unit=<UINT> ] sid=<UINT> portlist=<UINTLIST>\n"
        "dir={ ingress | egress | both }\n"
    },
    {
        "del session-src-port", 2, _mir_cmd_delSessionSrcPort,
        "mir del session-src-port [ unit=<UINT> ] sid=<UINT> portlist=<UINTLIST>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
mir_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _mir_cmd_vec, sizeof(_mir_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
mir_cmd_usager()
{
    return (dsh_usager(_mir_cmd_vec, sizeof(_mir_cmd_vec) / sizeof(DSH_VEC_T)));
}
