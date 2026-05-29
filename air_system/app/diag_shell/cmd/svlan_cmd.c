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

#include <cmd/svlan_cmd.h>

#include <air_port.h>
#include <air_svlan.h>
#include <air_vlan.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* MACRO FUNCTION DECLARATIONS
 */

#define SVLAN_CMD_RSRC_PRINT_TBL_TITLE(__unit__)                 \
    do                                                           \
    {                                                            \
        osal_printf("unit %d\n", __unit__);                      \
        osal_printf("%-25s %6s/%6s\n", "type", "used", "total"); \
    } while (0);

#define SVLAN_CMD_RSRC_PRINT_TBL_CONTENT(__type__, __used__, __total__) \
    do                                                                  \
    {                                                                   \
        osal_printf("%-25s %6d/%6d\n", __type__, __used__, __total__);  \
    } while (0);

static AIR_ERROR_NO_T
_svlan_cmd_setPortOuterTPID(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            portid;
    UI32_T            tpid;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: svlan set port-outer-tpid [ unit=<UINT> ] portlist=<UINTLIST> tpid=<HEX> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_OPT(dsh_getHex(tokens, token_idx, "tpid", &tpid, sizeof(UI32_T)), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, portid)
    {
        /* Call api */
        rc = air_svlan_setEgressOuterTpid(unit, portid, tpid);
        if (AIR_E_OK == rc)
        {
            rc = air_svlan_setIngressTpid(unit, portid, tpid);
        }
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port %d outer tpid  %x fail(%u)\n", portid, tpid, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_svlan_cmd_showPortOuterTPID(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            portid;
    UI32_T            tpid;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: svlan show port-outer-tpid [ unit=<UINT> ] portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %6s\n", "unit/port", "tpid");
    AIR_PORT_FOREACH(pbm, portid)
    {
        osal_printf("%5d/%2d", unit, portid);
        /* Call api */
        rc = air_svlan_getEgressOuterTpid(unit, portid, &tpid);
        if (AIR_E_OK == rc)
        {
            osal_printf("   0x%x\n", tpid);
        }
        else
        {
            osal_printf("***Error***, get port %d outer tpid fail(%u)\n", portid, rc);
            break;
        }
    }
    return rc;
}

static void
_show_svlanEntry(
    const UI32_T             unit,
    const UI32_T             idx,
    const AIR_SVLAN_ENTRY_T *field)
{
    osal_printf("unit      : %u\n", unit);
    osal_printf("idx       : %u\n", idx);
    osal_printf("cvid      : %u\n", field->c_vid);
    if (field->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI)
    {
        osal_printf("cpri      : %u\n", field->c_pri);
    }
    if (field->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE)
    {
        osal_printf("ethertype : 0x%x\n", field->eth_type);
    }
    osal_printf("svid      : %u\n", field->s_vid);
    osal_printf("spri      : %u\n", field->s_pri);
}

static AIR_ERROR_NO_T
_svlan_cmd_addEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, svlanIdx;
    UI32_T            cvid = 0, cvlanPri = 0, svid = 0, svlanPri = 0;
    UI32_T            etherType = 0;
    AIR_SVLAN_ENTRY_T svlanEntry;

    /* cmd: svlan add entry [ unit=<UINT> ] idx=<UINT> cvid=<UINT> [ cpri=<UINT> ]
     *      [ ethertype=<HEX> ] svid=<UINT> spri=<UINT> */

    osal_memset(&svlanEntry, 0, sizeof(AIR_SVLAN_ENTRY_T));
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &svlanIdx), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "cvid", &cvid), token_idx, 2);
    svlanEntry.c_vid = cvid;
    svlanEntry.flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CVID;
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cpri"))
    {
        DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "cpri", &cvlanPri), token_idx, 2);
        svlanEntry.c_pri = cvlanPri;
        svlanEntry.flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ethertype"))
    {
        DSH_CHECK_OPT(dsh_getHex(tokens, token_idx, "ethertype", &etherType, sizeof(UI32_T)), token_idx, 2);
        svlanEntry.eth_type = etherType;
        svlanEntry.flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "svid", &svid), token_idx, 2);
    svlanEntry.s_vid = svid;
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "spri", &svlanPri), token_idx, 2);
    svlanEntry.s_pri = svlanPri;
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* Call api */
    rc = air_svlan_addSvlan(unit, svlanIdx, &svlanEntry);
    if (AIR_E_ENTRY_EXISTS == rc)
    {
        osal_printf("***Error***, svlan idx %u already exists\n", svlanIdx);
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set svlan idx %u fail(%u)\n", svlanIdx, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_svlan_cmd_showEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, svlanIdx;
    AIR_SVLAN_ENTRY_T svlanEntry;

    /* cmd: svlan show entry [ unit=<UINT> ] idx=<UINT> */

    osal_memset(&svlanEntry, 0, sizeof(AIR_SVLAN_ENTRY_T));
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &svlanIdx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* Call api */
    rc = air_svlan_getSvlan(unit, svlanIdx, &svlanEntry);
    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        osal_printf("***Error***, svlan idx %u not found\n", svlanIdx);
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get svlan idx %u fail(%u)\n", svlanIdx, rc);
    }
    else
    {
        _show_svlanEntry(unit, svlanIdx, &svlanEntry);
    }

    return rc;
}

static AIR_ERROR_NO_T
_svlan_cmd_delEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, svlanIdx;

    /* cmd: svlan del entry [ unit=<UINT> ] idx=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &svlanIdx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* Call api */
    rc = air_svlan_delSvlan(unit, svlanIdx);
    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        osal_printf("***Error***, svlan idx %u not found\n", svlanIdx);
    }
    else if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, del svlan idx %u fail(%u)\n", svlanIdx, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_svlan_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, capacity = 0, usage = 0, param = 0;
    AIR_SWC_RSRC_T rsrc_type = AIR_SWC_RSRC_SVLAN;

    /* cmd: svlan show resource [ unit=<UINT> ] */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    rc = air_swc_getCapacity(unit, rsrc_type, param, &capacity);
    if (AIR_E_OK == rc)
    {
        rc = air_swc_getUsage(unit, rsrc_type, param, &usage);
        if (AIR_E_OK == rc)
        {
            SVLAN_CMD_RSRC_PRINT_TBL_TITLE(unit);
            SVLAN_CMD_RSRC_PRINT_TBL_CONTENT("svlan", usage, capacity);
        }
    }
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get resource info fail(%u)\n", rc);
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _svlan_cmd_vec[] =
{
    {
        "add entry", 2, _svlan_cmd_addEntry,
        "svlan add entry [ unit=<UINT> ] idx=<UINT> cvid=<UINT> [ cpri=<UINT> ] \n"
        "[ ethertype=<HEX> ] svid=<UINT> spri=<UINT>\n"
    },
    {
        "del entry", 2, _svlan_cmd_delEntry,
        "svlan del entry [ unit=<UINT> ] idx=<UINT> \n"
    },
    {
        "show entry", 2, _svlan_cmd_showEntry,
        "svlan show entry [ unit=<UINT> ] idx=<UINT> \n"
    },
    {
        "set port-outer-tpid", 2, _svlan_cmd_setPortOuterTPID,
        "svlan set port-outer-tpid [ unit=<UINT> ] portlist=<UINTLIST> tpid=<HEX> \n"
    },
    {
        "show port-outer-tpid", 2, _svlan_cmd_showPortOuterTPID,
        "svlan show port-outer-tpid [ unit=<UINT> ] portlist=<UINTLIST> \n"
    },
    {
        "show resource", 2, _svlan_cmd_showResource,
        "svlan show resource [ unit=<UINT> ]\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
svlan_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _svlan_cmd_vec, sizeof(_svlan_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
svlan_cmd_usager()
{
    return (dsh_usager(_svlan_cmd_vec, sizeof(_svlan_cmd_vec) / sizeof(DSH_VEC_T)));
}
