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

#include <cmd/vlan_cmd.h>

#include <air_port.h>
#include <air_vlan.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define VLAN_CMD_VLAN_ENTRY_NUM (4096)
#define VLAN_CMD_VLAN_START_NUM (1)

/* MACRO FUNCTION DECLARATIONS
 */
#define VLAN_CMD_PRINT_VLAN_TBL_INDEX()                                         \
    do                                                                          \
    {                                                                           \
        osal_printf("unit  vid fid                             member-port\n"); \
    } while (0)

#define VLAN_CMD_RSRC_PRINT_TBL_TITLE(__unit__)                  \
    do                                                           \
    {                                                            \
        osal_printf("unit %d\n", __unit__);                      \
        osal_printf("%-25s %6s/%6s\n", "type", "used", "total"); \
    } while (0);

#define VLAN_CMD_RSRC_PRINT_TBL_CONTENT(__type__, __used__, __total__) \
    do                                                                 \
    {                                                                  \
        osal_printf("%-25s %6d/%6d\n", __type__, __used__, __total__); \
    } while (0);

#define VLAN_CMD_RSRC_TYPE_NAME(__type__, __type_name__) \
    do                                                   \
    {                                                    \
        switch (__type__)                                \
        {                                                \
            case AIR_SWC_RSRC_VLAN_MAC:                  \
                __type_name__ = "mac";                   \
                break;                                   \
            case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:          \
                __type_name__ = "subnet-ipv4";           \
                break;                                   \
            case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:          \
                __type_name__ = "subnet-ipv6";           \
                break;                                   \
            case AIR_SWC_RSRC_VLAN_PROTOCOL:             \
                __type_name__ = "protocol";              \
                break;                                   \
            default:                                     \
                __type_name__ = "";                      \
                break;                                   \
        }                                                \
    } while (0);

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/***********************************
 * Command
 ***********************************/
static AIR_ERROR_NO_T
_vlan_cmd_createEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0, vid = 0;
    AIR_VLAN_ENTRY_T entry;

    /* cmd: vlan create entry [ unit=<UINT> ] vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&entry, 0, sizeof(AIR_VLAN_ENTRY_T));

    entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
    entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL;
    rc = air_vlan_createVlan(unit, vid, &entry);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, create vlan fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_destroyEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, vid = 0;

    /* cmd: vlan destroy entry [ unit=<UINT> ] vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_destroyVlan(unit, vid);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, destroy vlan fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setFid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, vid = 0, fid = 0;

    /* cmd: vlan set fid [ unit=<UINT> ] vid=<UINT> fid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "fid", &fid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_setFid(unit, vid, fid);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set vlan fid fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setMapSrvVid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, vid = 0, srv_vid = 0;

    /* cmd: vlan set map-srv-vid [ unit=<UINT> ] vid=<UINT> srv-vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "srv-vid", &srv_vid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_setVlanStag(unit, vid, srv_vid);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set vlan srv-vid fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setIvlCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0, vid = 0;
    C8_T             str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_VLAN_ENTRY_T entry;

    /* cmd: vlan set ivl-ctrl [ unit=<UINT> ] vid=<UINT> mode={ enable | disable } */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_getVlan(unit, vid, &entry);
    if (AIR_E_OK == rc)
    {
        rc = air_vlan_destroyVlan(unit, vid);
    }
    if (AIR_E_OK == rc)
    {
        if (AIR_E_OK == dsh_checkString(str, "enable"))
        {
            entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
        }
        else if (AIR_E_OK == dsh_checkString(str, "disable"))
        {
            entry.flags &= ~(AIR_VLAN_ENTRY_FLAGS_EN_IVL);
        }
        else
        {
            osal_printf("***Error***, invalid parameter\n");
            rc = AIR_E_BAD_PARAMETER;
        }
        if (AIR_E_OK == rc)
        {
            rc = air_vlan_createVlan(unit, vid, &entry);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set ivl ctrl fail(%u)\n", rc);
            }
        }
    }
    else
    {
        osal_printf("***Error***, set ivl ctrl fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_showEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0, port = 0;
    AIR_VLAN_ENTRY_T *ptr_entry = NULL;
    AIR_PORT_BITMAP_T member_pbm = {0}, untagged_pbm = {0}, tagged_pbm = {0}, stack_pbm = {0}, swap_pbm = {0};
    UI16_T            fid = 0, stag = 0;
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_type = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_LAST;
    C8_T                             *ptr_str = NULL;
    BOOL_T                            is_ivl = FALSE;

    /* cmd: vlan show entry [ unit=<UINT> ] vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);

    ptr_entry = osal_alloc(sizeof(AIR_VLAN_ENTRY_T), "cmd");
    if (NULL != ptr_entry)
    {
        osal_memset(ptr_entry, 0, sizeof(AIR_VLAN_ENTRY_T));
        rc = air_vlan_getVlan(unit, vid, ptr_entry);
        if (AIR_E_OK == rc)
        {
            is_ivl = (ptr_entry->flags & AIR_VLAN_ENTRY_FLAGS_EN_IVL) ? TRUE : FALSE;
        }
        osal_free(ptr_entry);
    }
    else
    {
        osal_printf("***Error***, allocate memory fail\n");
        rc = AIR_E_NO_MEMORY;
    }
    if (AIR_E_OK == rc)
    {
        rc = air_vlan_getFid(unit, vid, &fid);
    }
    if (AIR_E_OK == rc)
    {
        rc = air_vlan_getMember(unit, vid, member_pbm);
    }
    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(member_pbm, port)
        {
            rc = air_vlan_getPortEgressTagCtrl(unit, vid, port, &tag_type);
            if (AIR_E_OK == rc)
            {
                switch (tag_type)
                {
                    case AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED:
                        AIR_PORT_ADD(untagged_pbm, port);
                        break;
                    case AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_SWAP:
                        AIR_PORT_ADD(swap_pbm, port);
                        break;
                    case AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED:
                        AIR_PORT_ADD(tagged_pbm, port);
                        break;
                    case AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK:
                        AIR_PORT_ADD(stack_pbm, port);
                        break;
                    default:
                        continue;
                }
            }
            else
            {
                break;
            }
        }
    }
    if (AIR_E_OK == rc)
    {
        rc = air_vlan_getVlanStag(unit, vid, &stag);
    }

    if (AIR_E_OK == rc)
    {
        ptr_str = osal_alloc(AIR_MAX_LEN_OF_PORT_STR, "cmd");
        if (NULL != ptr_str)
        {
            osal_printf("unit : %u\n", unit);
            osal_printf("vlan : %u\n", vid);
            osal_printf("    ivl           : %s\n", (TRUE == is_ivl) ? "enable" : "disable");
            osal_printf("    fid           : %u\n", fid);
            osal_printf("    map-srv-vid   : %u\n", stag);
            CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, member_pbm);
            osal_printf("    member-port   : %s\n", ptr_str);
            CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, untagged_pbm);
            osal_printf("    untagged-port : %s\n", ptr_str);
            CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, tagged_pbm);
            osal_printf("    tagged-port   : %s\n", ptr_str);
            CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, swap_pbm);
            osal_printf("    swap-port     : %s\n", ptr_str);
            CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, stack_pbm);
            osal_printf("    stack-port    : %s\n", ptr_str);
            osal_free(ptr_str);
        }
        else
        {
            osal_printf("***Error***, allocate memory fail\n");
            rc = AIR_E_NO_MEMORY;
        }
    }

    if (AIR_E_OK != rc)
    {
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            osal_printf("***Error***, vlan %d not found\n", vid);
        }
        else
        {
            osal_printf("***Error***, show vlan %d fail\n", vid);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_dumpEntry(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    BOOL_T            index_enable = TRUE;
    UI32_T            unit = 0, vid = 0, count = 0, wdog_count = 0;
    AIR_VLAN_ENTRY_T *ptr_entry = NULL;
    C8_T             *ptr_str = NULL;

    /* cmd: vlan dump entry [ unit=<UINT> ] */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    ptr_entry = osal_alloc(sizeof(AIR_VLAN_ENTRY_T), "cmd");
    ptr_str = osal_alloc(DSH_CMD_MAX_LENGTH, "cmd");
    if (NULL != ptr_entry && NULL != ptr_str)
    {
        for (vid = VLAN_CMD_VLAN_START_NUM; vid < VLAN_CMD_VLAN_ENTRY_NUM; vid++)
        {
            osal_memset(ptr_entry, 0, sizeof(AIR_VLAN_ENTRY_T));
            rc = air_vlan_getVlan(unit, vid, ptr_entry);
            if (wdog_count >= AIR_VLAN_WDOG_KICK_NUM)
            {
                osal_wdog_kick();
                wdog_count -= AIR_VLAN_WDOG_KICK_NUM;
            }
            if (AIR_E_OK == rc)
            {
                if (TRUE == index_enable)
                {
                    VLAN_CMD_PRINT_VLAN_TBL_INDEX();
                    index_enable = FALSE;
                }
                osal_memset(ptr_str, 0, AIR_MAX_LEN_OF_PORT_STR);
                osal_printf("%4d ", unit);
                osal_printf("%4d ", vid);
                osal_printf("%3d ", ptr_entry->fid);
                CMD_STRING_SIZE_PORTLIST(ptr_str, AIR_MAX_LEN_OF_PORT_STR, ptr_entry->port_bitmap);
                osal_printf("%39s", ptr_str);
                osal_printf("\n");
                count++;
            }
            wdog_count++;
        }

        if (0 == count)
        {
            osal_printf("vlan table is empty\n");
        }
        else
        {
            osal_printf("\ntotal valid vlan %s: %d\n", (count > 1) ? "entries" : "entry", count);
        }
    }
    else
    {
        osal_printf("***Error***, allocate memory fail\n");
        rc = AIR_E_NO_MEMORY;
    }

    if (NULL != ptr_entry)
    {
        osal_free(ptr_entry);
    }
    if (NULL != ptr_str)
    {
        osal_free(ptr_str);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPvid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0, port = 0;
    AIR_PORT_BITMAP_T portlist = {0};

    /* vlan set pvid [ unit=<UINT> ] portlist=<UINTLIST> vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_setPortCvid(unit, port, vid);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set pvid at port %u fail(%u)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPvid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    UI16_T            vid = 0;
    AIR_PORT_BITMAP_T portlist = {0};

    /* cmd: vlan show pvid [ unit=<UINT> ] portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %6s\n", "unit/port", "pvid");
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_getPortCvid(unit, port, &vid);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get pvid at port %u fail(%u)\n", port, rc);
            break;
        }
        osal_printf("%5u/%2u %8u\n", unit, port, vid);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPsvid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0, port = 0;
    AIR_PORT_BITMAP_T portlist = {0};

    /* cmd: vlan set psvid [ unit=<UINT> ] portlist=<UINTLIST> vid=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_setPortSvid(unit, port, vid);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set psvid at port %u fail(%u)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPsvid(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    UI16_T            vid = 0;
    AIR_PORT_BITMAP_T portlist = {0};

    /* cmd: vlan show psvid [ unit=<UINT> ] portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %7s\n", "unit/port", "psvid");
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_getPortSvid(unit, port, &vid);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get psvid at port %u fail(%u)\n", port, rc);
            break;
        }
        osal_printf("%5u/%2u %9u\n", unit, port, vid);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPortAcceptFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0, port = 0;
    AIR_PORT_BITMAP_T            portlist = {0};
    C8_T                         str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_VLAN_ACCEPT_FRAME_TYPE_T type = AIR_VLAN_ACCEPT_FRAME_TYPE_LAST;

    /* cmd: vlan set port-accept-frame [ unit=<UINT> ] portlist=<UINTLIST>
     *      type={ all | tagged | untagged } */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "all"))
    {
        type = AIR_VLAN_ACCEPT_FRAME_TYPE_ALL;
    }
    else if (AIR_E_OK == dsh_checkString(str, "tagged"))
    {
        type = AIR_VLAN_ACCEPT_FRAME_TYPE_TAG_ONLY;
    }
    else if (AIR_E_OK == dsh_checkString(str, "untagged"))
    {
        type = AIR_VLAN_ACCEPT_FRAME_TYPE_UNTAG_ONLY;
    }
    else
    {
        osal_printf("***Error***, invalid parameter\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(portlist, port)
        {
            rc = air_vlan_setPortAcceptFrameType(unit, port, type);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set accept frame at port %u fail(%u)\n", port, rc);
                break;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPortAcceptFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0, port = 0;
    AIR_PORT_BITMAP_T            portlist = {0};
    AIR_VLAN_ACCEPT_FRAME_TYPE_T type = AIR_VLAN_ACCEPT_FRAME_TYPE_LAST;

    /* cmd: vlan show port-accept-frame [ unit=<UINT> ] portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s\n", "unit/port", "type");
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_getPortAcceptFrameType(unit, port, &type);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get accept frame at port %u fail(%u)\n", port, rc);
            break;
        }
        if (AIR_VLAN_ACCEPT_FRAME_TYPE_LAST <= type)
        {
            osal_printf("***Error***, unknown accept frame type");
            break;
        }
        osal_printf("%5u/%2u %10s\n", unit, port,
                    (AIR_VLAN_ACCEPT_FRAME_TYPE_ALL == type)      ? "all" :
                    (AIR_VLAN_ACCEPT_FRAME_TYPE_TAG_ONLY == type) ? "tagged" :
                                                                    "untagged");
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPortVlanCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0, port = 0;
    AIR_PORT_BITMAP_T    portlist = {0};
    C8_T                 str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_VLAN_PORT_ATTR_T mode = AIR_VLAN_PORT_ATTR_LAST;

    /* cmd: vlan set port-vlan-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     *      mode={ user | transparent | stack } */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "user"))
    {
        mode = AIR_VLAN_PORT_ATTR_USER_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(str, "transparent"))
    {
        mode = AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(str, "stack"))
    {
        mode = AIR_VLAN_PORT_ATTR_STACK_PORT;
    }
    else
    {
        osal_printf("***Error***, invalid parameter\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(portlist, port)
        {
            rc = air_vlan_setPortAttr(unit, port, mode);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port vlan ctrl at port %u fail(%u)\n", port, rc);
                break;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPortVlanCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0, port = 0;
    AIR_PORT_BITMAP_T    portlist = {0};
    AIR_VLAN_PORT_ATTR_T mode = AIR_VLAN_PORT_ATTR_LAST;

    /* cmd: vlan show port-vlan-ctrl [ unit=<UINT> ] portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %11s\n", "unit/port", "mode");
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_vlan_getPortAttr(unit, port, &mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get port vlan ctrl at port %u fail(%u)\n", port, rc);
            break;
        }
        if (AIR_VLAN_PORT_ATTR_LAST <= mode)
        {
            osal_printf("***Error***, unknown port vlan ctrl mode");
            rc = AIR_E_BAD_PARAMETER;
            break;
        }
        osal_printf("%5u/%2u %13s\n", unit, port,
                    (AIR_VLAN_PORT_ATTR_USER_PORT == mode)  ? "user" :
                    (AIR_VLAN_PORT_ATTR_STACK_PORT == mode) ? "stack" :
                                                              "transparent");
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_addMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T                    rc = AIR_E_OK;
    UI32_T                            unit = 0, vid = 0, port = 0;
    AIR_PORT_BITMAP_T                 pbm, pbm_cur;
    C8_T                              str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_ctl = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_LAST;

    /* cmd: vlan add member [ unit=<UINT> ] vid=<UINT> portlist=<UINTLIST>
            egrs-tag-ctrl={ untagged | tagged | swap | stack } */

    AIR_PORT_BITMAP_CLEAR(pbm);
    AIR_PORT_BITMAP_CLEAR(pbm_cur);

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "egrs-tag-ctrl", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_getMember(unit, vid, pbm_cur);
    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(pbm, port)
        {
            if (AIR_PORT_CHK(pbm_cur, port))
            {
                osal_printf("***Error***, port %d already in vlan %d\n", port, vid);
                rc = AIR_E_BAD_PARAMETER;
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        if (AIR_E_OK == dsh_checkString(str, "untagged"))
        {
            tag_ctl = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
        }
        else if (AIR_E_OK == dsh_checkString(str, "tagged"))
        {
            tag_ctl = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED;
        }
        else if (AIR_E_OK == dsh_checkString(str, "swap"))
        {
            tag_ctl = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_SWAP;
        }
        else if (AIR_E_OK == dsh_checkString(str, "stack"))
        {
            tag_ctl = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK;
        }
        else
        {
            osal_printf("***Error***, invalid parameter\n");
            rc = AIR_E_BAD_PARAMETER;
        }

        if (AIR_E_OK == rc)
        {
            AIR_PORT_FOREACH(pbm, port)
            {
                rc = air_vlan_addMemberPort(unit, vid, port);
                if (AIR_E_OK == rc)
                {
                    rc = air_vlan_setPortEgressTagCtrl(unit, vid, port, tag_ctl);
                }
                if (AIR_E_OK != rc)
                {
                    osal_printf("***Error***, add vlan member for port %u fail(%u)\n", port, rc);
                    break;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_delMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0, port = 0;
    AIR_PORT_BITMAP_T pbm;

    /* cmd: vlan del member [ unit=<UINT> ] vid=<UINT> portlist=<UINTLIST> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_vlan_delMemberPort(unit, vid, port);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, del vlan member for port %u fail(%u)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_addMacBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     unit = 0;
    UI32_T                     idx = 0;
    UI32_T                     vid = 0;
    UI32_T                     pri = 0;
    AIR_MAC_T                  mac;
    AIR_MAC_T                  mask;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;

    /* cmd: vlan add mac-based [ unit=<UINT> ] idx=<UINT>
     *      mac=<MACADDR> mac-mask=<MACADDR> vid=<UINT> pri=<UINT> */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "mac", &mac), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "mac-mask", &mask), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memcpy(entry.mac, mac, sizeof(AIR_MAC_T));
    osal_memcpy(entry.mac_mask, mask, sizeof(AIR_MAC_T));
    entry.vid = vid;
    entry.pri = pri;

    rc = air_vlan_addMacBasedVlan(unit, idx, &entry);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add mac based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getMacBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     unit = 0;
    UI32_T                     idx = 0;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;

    /* cmd: vlan get mac-based [ unit=<UNIT> ] idx=<UINT> */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_getMacBasedVlan(unit, idx, &entry);

    if (AIR_E_OK == rc)
    {
        osal_printf("unit      : %u\n", unit);
        osal_printf("idx       : %u\n", idx);
        osal_printf("mac       : ");
        dsh_showMacAddr((const AIR_MAC_T *)&entry.mac);
        osal_printf("\n");
        osal_printf("mac-mask  : ");
        dsh_showMacAddr((const AIR_MAC_T *)&entry.mac_mask);
        osal_printf("\n");
        osal_printf("vid       : %u\n", entry.vid);
        osal_printf("pri       : %u\n", entry.pri);
    }
    else
    {
        osal_printf("***Error***, show mac based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_delMacBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         idx = 0;

    /* cmd: vlan del mac-based [ unit=<UNIT> ] idx=<UINT> */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_delMacBasedVlan(unit, idx);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete mac based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_addSubnetBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    UI32_T                        unit = 0;
    UI32_T                        idx = 0;
    UI32_T                        vid = 0;
    UI32_T                        pri = 0;
    AIR_IP_ADDR_T                 ipAddr;
    AIR_IP_ADDR_T                 ipMask;
    AIR_VLAN_SUBNET_BASED_ENTRY_T entry;

    /* cmd: vlan add subnet-based [ unit=<UINT> ] idx=<UINT>
     *      ip=<IPADDR> ip-mask=<IPADDR> vid=<UINT> pri=<UINT>
     */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_SUBNET_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    if (AIR_E_OK == (rc = dsh_checkString(tokens[token_idx], "ip")))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "ip", &ipAddr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "ip", &ipAddr.ip_addr.ipv6_addr), ipAddr.ipv4, token_idx,
                          2);
    }
    else
    {
        return rc;
    }
    if (AIR_E_OK == (rc = dsh_checkString(tokens[token_idx], "ip-mask")))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "ip-mask", &ipMask.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "ip-mask", &ipMask.ip_addr.ipv6_addr), ipMask.ipv4,
                          token_idx, 2);
    }
    else
    {
        return rc;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memcpy(&entry.ip_addr, &ipAddr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.ip_mask, &ipMask, sizeof(AIR_IP_ADDR_T));
    entry.vid = vid;
    entry.pri = pri;

    rc = air_vlan_addSubnetBasedVlan(unit, idx, &entry);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add subnet based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getSubnetBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    UI32_T                        unit = 0;
    UI32_T                        idx = 0;
    AIR_VLAN_SUBNET_BASED_ENTRY_T entry;

    /* cmd: vlan get subnet-based [ unit=<UINT> ] idx=<UINT>
     */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_SUBNET_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_getSubnetBasedVlan(unit, idx, &entry);

    if (AIR_E_OK == rc)
    {
        osal_printf("unit    : %u\n", unit);
        osal_printf("idx     : %u\n", idx);
        osal_printf("ip      : ");
        dsh_showIpAddr(&entry.ip_addr);
        osal_printf("\n");
        osal_printf("ip-mask : ");
        dsh_showIpAddr(&entry.ip_mask);
        osal_printf("\n");
        osal_printf("vid     : %u\n", entry.vid);
        osal_printf("pri     : %u\n", entry.pri);
    }
    else
    {
        osal_printf("***Error***, show subnet based fail(%u)\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_delSubnetBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         idx = 0;

    /* cmd: vlan del subnet-based [ unit=<UINT> ] idx=<UINT>
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_delSubnetBasedVlan(unit, idx);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, del subnet based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_addProtocolBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T                       rc = AIR_E_OK;
    UI32_T                               unit = 0;
    UI32_T                               idx = 0;
    UI32_T                               group_id = 0;
    UI32_T                               type_data[2];
    C8_T                                 frame_type_str[DSH_CMD_MAX_LENGTH];
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_T frame_type;
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T      entry;

    /* cmd: vlan add protocol-based [ unit=<UINT> ] idx=<UINT>
     *      frame-type={ ethernet | rfc1042 | llc | snap } type-data=<HEX> group-id=<UINT>
     */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_PROTOCOL_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "frame-type", frame_type_str), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(frame_type_str, "ethernet"))
    {
        frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET;
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ethertype", type_data, sizeof(UI32_T)), token_idx, 2);
    }
    else if (AIR_E_OK == dsh_checkString(frame_type_str, "rfc1042"))
    {
        frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042;
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "ethertype", type_data, sizeof(UI32_T)), token_idx, 2);
    }
    else if (AIR_E_OK == dsh_checkString(frame_type_str, "llc-other"))
    {
        frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER;
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "sap", type_data, sizeof(UI32_T)), token_idx, 2);
    }
    else if (AIR_E_OK == dsh_checkString(frame_type_str, "snap-other"))
    {
        frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER;
        DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "pid", type_data, sizeof(UI32_T) * 2), token_idx, 2);
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "group-id", &group_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    entry.frame_type = frame_type;
    osal_memcpy(&entry.type_data, type_data, sizeof(UI32_T) * 2);
    entry.group_id = group_id;

    rc = air_vlan_addProtocolBasedVlan(unit, idx, &entry);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, add protocol based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getProtocolBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    UI32_T                          unit = 0;
    UI32_T                          idx = 0;
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T entry;

    /* cmd: vlan get protocol-based [ unit=<UINT> ] idx=<UINT>
     */

    osal_memset(&entry, 0, sizeof(AIR_VLAN_PROTOCOL_BASED_ENTRY_T));

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_getProtocolBasedVlan(unit, idx, &entry);

    if (AIR_E_OK == rc)
    {
        osal_printf("unit       : %u\n", unit);
        osal_printf("idx        : %u\n", idx);
        osal_printf("frame-type : %s\n",
                    ((AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET == entry.frame_type)   ? "ethernet" :
                     (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042 == entry.frame_type)    ? "rfc1042" :
                     (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER == entry.frame_type)  ? "llc-other" :
                     (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER == entry.frame_type) ? "snap-other" :
                                                                                           ""));
        if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET == entry.frame_type)
        {
            osal_printf("ethertype  : 0x%04x\n", entry.type_data.eth_type);
        }
        else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042 == entry.frame_type)
        {
            osal_printf("ethertype  : 0x%04x\n", entry.type_data.rfc1042_eth_type);
        }
        else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER == entry.frame_type)
        {
            osal_printf("sap        : 0x%04x\n", entry.type_data.llc_dsap_ssap);
        }
        else if (entry.frame_type == AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER)
        {
            osal_printf("pid        : 0x%02x%08x\n", entry.type_data.snap_pid[1], entry.type_data.snap_pid[0]);
        }
        else
        {
            osal_printf("***Error***, unknown frame type\n");
        }
        osal_printf("group-id   : %u\n", entry.group_id);
    }
    else
    {
        osal_printf("***Error***, show protocol based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_delProtocolBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         idx = 0;

    /* cmd: vlan del protocol-based [ unit=<UINT> ] idx=<UINT>
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "idx", &idx), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_vlan_delProtocolBasedVlan(unit, idx);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, delete protocol based fail(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPortProtocolBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            group_id = 0;
    UI32_T            vid = 0;
    UI32_T            pri = 0;
    UI32_T            port = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: vlan set port-protocol-based [ unit=<UINT> ]
     *      portlist=<UINTLIST> group-id=<UINT> vid=<UINT> pri=<UINT>
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "group-id", &group_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_vlan_setProtocolBasedVlanPortAttr(unit, port, group_id, vid, pri);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set protocol based attr at port %u fail(%u)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPortProtocolBased(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            group_id = 0;
    UI16_T            vid = 0;
    UI16_T            pri = 0;
    UI32_T            port = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: vlan get port-protocol-based [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "group-id", &group_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s %5s %5s\n", "unit/port", "group-id", "vid", "pri");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_vlan_getProtocolBasedVlanPortAttr(unit, port, group_id, &vid, &pri);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***,show protocol based attr at port %u fail(%u)\n", port, rc);
            break;
        }
        osal_printf("%5u/%2u %10u %5u %5u\n", unit, port, group_id, vid, pri);
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_setPortIngressVlanFilterMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port = 0;
    BOOL_T            mode = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};

    /* cmd: vlan set port-ingress-filter [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(st, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        osal_printf("***Error***, invalid parameter\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(pbm, port)
        {
            rc = air_vlan_setIngressVlanFilter(unit, port, mode);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port(%u) vlan ingress filter mode fail(%d)\n", port, rc);
                break;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_getPortIngressVlanFilterMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            port = 0;
    BOOL_T            state = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: vlan show port-ingress-filter [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %7s\n", "unit/port", "mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_vlan_getIngressVlanFilter(unit, port, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5u/%2u %9s\n", unit, port, ((TRUE == state) ? "enable" : "disable"));
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("***Error***, not support\n");
            break;
        }
        else
        {
            osal_printf("***Error***, show port(%u) vlan ingress filter mode fail(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_vlan_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, i = 0, capacity = 0, usage = 0, param = 0;
    C8_T           type_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T           subnet_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T          *ptr_type_name = NULL;
    AIR_SWC_RSRC_T rsrc_type = AIR_SWC_RSRC_LAST;
    AIR_SWC_RSRC_T rsrc_list[4] = {AIR_SWC_RSRC_VLAN_MAC, AIR_SWC_RSRC_VLAN_SUBNET_IPV4, AIR_SWC_RSRC_VLAN_SUBNET_IPV6,
                                   AIR_SWC_RSRC_VLAN_PROTOCOL};

    /* cmd: vlan show resource [ unit=<UINT> ] [ type={ mac | subnet ip-type={ ipv4 | ipv6 } | protocol } ]
     */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "type"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(type_str, "mac"))
        {
            rsrc_type = AIR_SWC_RSRC_VLAN_MAC;
        }
        else if (AIR_E_OK == dsh_checkString(type_str, "subnet"))
        {
            DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "ip-type", subnet_str), token_idx, 2);
            if (AIR_E_OK == dsh_checkString(subnet_str, "ipv4"))
            {
                rsrc_type = AIR_SWC_RSRC_VLAN_SUBNET_IPV4;
            }
            else if (AIR_E_OK == dsh_checkString(subnet_str, "ipv6"))
            {
                rsrc_type = AIR_SWC_RSRC_VLAN_SUBNET_IPV6;
            }
            else
            {
                rc = AIR_E_BAD_PARAMETER;
            }
        }
        else if (AIR_E_OK == dsh_checkString(type_str, "protocol"))
        {
            rsrc_type = AIR_SWC_RSRC_VLAN_PROTOCOL;
        }
        else
        {
            osal_printf("***Error***, invalid parameter\n");
            rc = AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == rc)
    {
        if (AIR_SWC_RSRC_LAST == rsrc_type)
        {
            VLAN_CMD_RSRC_PRINT_TBL_TITLE(unit);
            for (i = 0; i < 4; i++)
            {
                rc = air_swc_getCapacity(unit, rsrc_list[i], param, &capacity);
                if (AIR_E_OK == rc)
                {
                    rc = air_swc_getUsage(unit, rsrc_list[i], param, &usage);
                    if (AIR_E_OK == rc)
                    {
                        VLAN_CMD_RSRC_TYPE_NAME(rsrc_list[i], ptr_type_name);
                        VLAN_CMD_RSRC_PRINT_TBL_CONTENT(ptr_type_name, usage, capacity);
                    }
                }
                if (AIR_E_OK != rc)
                {
                    VLAN_CMD_RSRC_TYPE_NAME(rsrc_list[i], ptr_type_name);
                    osal_printf("***Error***, get %s resource info fail(%u)\n", ptr_type_name, rc);
                    break;
                }
            }
        }
        else
        {
            rc = air_swc_getCapacity(unit, rsrc_type, param, &capacity);
            if (AIR_E_OK == rc)
            {
                rc = air_swc_getUsage(unit, rsrc_type, param, &usage);
                if (AIR_E_OK == rc)
                {
                    VLAN_CMD_RSRC_PRINT_TBL_TITLE(unit);
                    VLAN_CMD_RSRC_TYPE_NAME(rsrc_type, ptr_type_name);
                    VLAN_CMD_RSRC_PRINT_TBL_CONTENT(ptr_type_name, usage, capacity);
                }
            }
            if (AIR_E_OK != rc)
            {
                VLAN_CMD_RSRC_TYPE_NAME(rsrc_list[i], ptr_type_name);
                osal_printf("***Error***, get %s resource info fail(%u)\n", ptr_type_name, rc);
            }
        }
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _vlan_cmd_vec[] =
{
    {
        "create entry", 2, _vlan_cmd_createEntry,
        "vlan create entry [ unit=<UINT> ] vid=<UINT>\n"
    },
    {
        "destroy entry", 2, _vlan_cmd_destroyEntry,
        "vlan destroy entry [ unit=<UINT> ] vid=<UINT>\n"
    },
    {
        "show entry", 2, _vlan_cmd_showEntry,
        "vlan show entry [ unit=<UINT> ] vid=<UINT>\n"
    },
    {
        "dump entry", 2, _vlan_cmd_dumpEntry,
        "vlan dump entry [ unit=<UINT> ]\n"
    },
    {
        "set fid", 2, _vlan_cmd_setFid,
        "vlan set fid [ unit=<UINT> ] vid=<UINT> fid=<UINT>\n"
    },
    {
        "set map-srv-vid", 2, _vlan_cmd_setMapSrvVid,
        "vlan set map-srv-vid [ unit=<UINT> ] vid=<UINT> srv-vid=<UINT>\n"
    },
    {
        "set ivl-ctrl", 2, _vlan_cmd_setIvlCtrl,
        "vlan set ivl-ctrl [ unit=<UINT> ] vid=<UINT> mode={ enable | disable }\n"
    },
    {
        "set pvid", 2, _vlan_cmd_setPvid,
        "vlan set pvid [ unit=<UINT> ] portlist=<UINTLIST> vid=<UINT>\n"
    },
    {
        "show pvid", 2, _vlan_cmd_getPvid,
        "vlan show pvid [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set psvid", 2, _vlan_cmd_setPsvid,
        "vlan set psvid [ unit=<UINT> ] portlist=<UINTLIST> vid=<UINT>\n"
    },
    {
        "show psvid", 2, _vlan_cmd_getPsvid,
        "vlan show psvid [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set port-accept-frame", 2, _vlan_cmd_setPortAcceptFrame,
        "vlan set port-accept-frame [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ all | tagged | untagged }\n"
    },
    {
        "show port-accept-frame", 2, _vlan_cmd_getPortAcceptFrame,
        "vlan show port-accept-frame [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set port-vlan-ctrl", 2, _vlan_cmd_setPortVlanCtrl,
        "vlan set port-vlan-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ user | transparent | stack }\n"
    },
    {
        "show port-vlan-ctrl", 2, _vlan_cmd_getPortVlanCtrl,
        "vlan show port-vlan-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "add member", 2, _vlan_cmd_addMember,
        "vlan add member [ unit=<UINT> ] vid=<UINT> portlist=<UINTLIST>\n"
        "egrs-tag-ctrl={ untagged | tagged | swap | stack }\n"
    },
    {
        "del member", 2, _vlan_cmd_delMember,
        "vlan del member [ unit=<UINT> ] vid=<UINT> portlist=<UINTLIST>\n"
    },
    {
        "add mac-based", 2, _vlan_cmd_addMacBased,
        "vlan add mac-based [ unit=<UINT> ] idx=<UINT>\n"
        "mac=<MACADDR> mac-mask=<MACADDR> vid=<UINT> pri=<UINT>\n"
    },
    {
        "del mac-based", 2, _vlan_cmd_delMacBased,
        "vlan del mac-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "show mac-based", 2, _vlan_cmd_getMacBased,
        "vlan show mac-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "add subnet-based", 2, _vlan_cmd_addSubnetBased,
        "vlan add subnet-based [ unit=<UINT> ] idx=<UINT>\n"
        "ip=<IPADDR> ip-mask=<IPADDR> vid=<UINT> pri=<UINT>\n"
    },
    {
        "del subnet-based", 2, _vlan_cmd_delSubnetBased,
        "vlan del subnet-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "show subnet-based", 2, _vlan_cmd_getSubnetBased,
        "vlan show subnet-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "add protocol-based", 2, _vlan_cmd_addProtocolBased,
        "vlan add protocol-based [ unit=<UINT> ] idx=<UINT>\n"
        "frame-type={ ethernet ethertype=<HEX> | rfc1042 ethertype=<HEX> |\n"
        "llc-other sap=<HEX> | snap-other pid=<HEX> } group-id=<UINT>\n"
    },
    {
        "del protocol-based", 2, _vlan_cmd_delProtocolBased,
        "vlan del protocol-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "show protocol-based", 2, _vlan_cmd_getProtocolBased,
        "vlan show protocol-based [ unit=<UINT> ] idx=<UINT>\n"
    },
    {
        "set port-protocol-based", 2, _vlan_cmd_setPortProtocolBased,
        "vlan set port-protocol-based [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> group-id=<UINT> vid=<UINT> pri=<UINT>\n"
    },
    {
        "show port-protocol-based", 2, _vlan_cmd_getPortProtocolBased,
        "vlan show port-protocol-based [ unit=<UINT> ]\n"
        "portlist=<UINTLIST> group-id=<UINT>\n"
    },
    {
        "set port-ingress-filter", 2, _vlan_cmd_setPortIngressVlanFilterMode,
        "vlan set port-ingress-filter [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ enable | disable }\n"
    },
    {
        "show port-ingress-filter", 2, _vlan_cmd_getPortIngressVlanFilterMode,
        "vlan show port-ingress-filter [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show resource", 2, _vlan_cmd_showResource,
        "vlan show resource [ unit=<UINT> ]\n"
        "[ type={ mac | subnet ip-type={ ipv4 | ipv6 } | protocol } ]\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
vlan_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    int rv = 0;
    rv = (dsh_dispatcher(tokens, token_idx, _vlan_cmd_vec, sizeof(_vlan_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}

AIR_ERROR_NO_T
vlan_cmd_usager()
{
    int rv = 0;
    rv = (dsh_usager(_vlan_cmd_vec, sizeof(_vlan_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}
