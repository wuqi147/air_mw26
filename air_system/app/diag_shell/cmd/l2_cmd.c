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

#include <cmd/l2_cmd.h>

#include <air_error.h>
#include <air_l2.h>
#include <air_port.h>
#include <air_sec.h>
#include <air_types.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define L2_CMD_INVALID_ENTRY (0xFFFFFFFF)
#define L2_WDOG_KICK_NUM     (100)
#define L2_MAX_ENTRY_NUM     (0x2000)

/* MACRO FUNCTION DECLARATIONS
 */
#define _l2_cmd_error(fmt, ...) osal_printf("***Error***, " fmt, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
*/
typedef struct _L2_CMD_DUMPINFO_S
{
    AIR_L2_MAC_SEARCH_TYPE_T type;
    UI32_T                   value;
    UI32_T                   traverse_cnt;
} _L2_CMD_DUMPINFO_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* STATIC VARIABLE DECLARATIONS
 */
const static C8_T *_str_unit = "unit";
const static C8_T *_str_vid = "vid";
const static C8_T *_str_fid = "fid";
const static C8_T *_str_mac = "mac";
const static C8_T *_str_port = "port";
const static C8_T *_str_portlist = "portlist";
const static C8_T *_str_na = "---";
const static C8_T *_str_en = "enable";
const static C8_T *_str_dis = "disable";
const static C8_T *_str_mode = "mode";

/* clang-format off */
const static C8_T *_air_forward_type_string [] =
{
    "broadcast",
    "unknown multicast",
    "unknown unicast",
    "unknown ip multicast"
};
const static C8_T *_air_forward_action_string [] =
{
    "drop",
    "flooding",
    "forward to portlist"
};

const static C8_T *_air_mac_address_forward_control_string [] =
{
    "default",
    "cpu include",
    "cpu exclude",
    "cpu only",
    "drop"
};
/* clang-format on */

#ifdef AIR_EN_L2_SHADOW
static UI32_T           _l2_cmd_notifyEntryNum = 0;
static AIR_MAC_ENTRY_T *_ptr_l2_cmd_sw_table = NULL;
#endif /* End of AIR_EN_L2_SHADOW */

static void
_l2_cmd_printTblLine(
    const UI32_T len)
{
    UI32_T i;
    for (i = 0; i < len; i++)
    {
        osal_printf("-");
    }
    osal_printf("\n");
}

static void
_l2_cmd_printTblHead(
    void)
{
    osal_printf(" %4s  %-17s  %-3s  %-4s  %-3s  %-6s  %11s  %-11s %-11s  %s\n", _str_unit, _str_mac, "ivl", _str_vid,
                _str_fid, "802.1x", "age-time", "forward", "egress-vlan", _str_portlist);
    _l2_cmd_printTblLine(94);
}

static void
_l2_cmd_printTblEntry(
    const UI32_T           unit,
    const UI32_T           count,
    const AIR_MAC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               i;
    AIR_SEC_8021X_MODE_T base;
    UI32_T               real_time;

    rc = air_sec_get8021xGlobalMode(unit, &base);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("get 802.1x mode fail(%u)\n", rc);
        return;
    }
    for (i = 0; i < count; i++)
    {
        osal_printf(" %-4d  ", (unit));
        osal_printf("%02x-%02x-%02x-%02x-%02x-%02x  ", ptr_entry[i].mac[0], ptr_entry[i].mac[1], ptr_entry[i].mac[2],
                    ptr_entry[i].mac[3], ptr_entry[i].mac[4], ptr_entry[i].mac[5]);
        if ((ptr_entry[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_IVL)
        {
            osal_printf("%-3s  ", "ivl");
            osal_printf("%-4d  ", ptr_entry[i].cvid);
            osal_printf("%-3s  ", _str_na);
        }
        else
        {
            osal_printf("%-3s  ", "svl");
            osal_printf("%-4s  ", _str_na);
            osal_printf("%-3d  ", ptr_entry[i].fid);
        }
        if (AIR_SEC_8021X_MODE_MAC == base)
        {
            if ((ptr_entry[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_UNAUTH)
            {
                osal_printf("%-6s  ", "unauth");
            }
            else
            {
                osal_printf("%-6s  ", "auth");
            }
        }
        else
        {
            osal_printf("%-6s  ", _str_na);
        }
        if ((ptr_entry[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_STATIC)
        {
            osal_printf("%11s  ", "static");
        }
        else
        {
            real_time = ptr_entry[i].timer;
            osal_printf("%7d sec  ", (UI32_T)real_time);
        }
        osal_printf("%-11s ", _air_mac_address_forward_control_string[ptr_entry[i].sa_fwd]);
        if ((ptr_entry[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
        {
            osal_printf("%-11s  ", "ignore");
        }
        else
        {
            osal_printf("%-11s  ", "filter");
        }
        CMD_PRINT_PORTLIST(ptr_entry[i].port_bitmap);
        osal_printf("\n");
    }
}

#ifdef AIR_EN_L2_SHADOW
static void
_l2_cmd_notify_cbk(
    const UI32_T                     unit,
    const AIR_L2_MAC_NOTIFY_REASON_T reason,
    const AIR_MAC_ENTRY_T           *ptr_entry,
    void                            *ptr_cookie)
{
    if (NULL == ptr_entry || NULL == ptr_cookie)
    {
        return;
    }

    /* Print MAC entry */
    switch (reason)
    {
        case AIR_L2_MAC_NOTIFY_REASON_ADD:
            osal_printf("\ndetected new l2 mac entry\n");
            (*((UI32_T *)ptr_cookie))++;
            break;
        case AIR_L2_MAC_NOTIFY_REASON_MODIFY:
            osal_printf("\ndetected l2 mac entry modified\n");
            break;
        case AIR_L2_MAC_NOTIFY_REASON_DELETE:
            osal_printf("\ndetected l2 mac entry deleted\n");
            (*((UI32_T *)ptr_cookie))--;
            break;
        default:
            osal_printf("\nunrecognized reason=(%u)\n", reason);
            return;
    }
    _l2_cmd_printTblHead();
    _l2_cmd_printTblEntry(unit, 1, ptr_entry);
}
#endif /* End of AIR_EN_L2_SHADOW */

static AIR_ERROR_NO_T
_l2_cmd_addMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_MAC_T         mac;
    AIR_PORT_BITMAP_T bitmap = {0};
    UI32_T            vid = 0;
    UI32_T            fid = 0;
    UI32_T            timer = AIR_L2_MAC_DEF_AGE_OUT_TIME;
    BOOL_T            is_ivl = FALSE;
    BOOL_T            is_static = FALSE;
    BOOL_T            is_unauth = FALSE;
    BOOL_T            dis_egrs_vlan = FALSE;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_L2_FWD_CTRL_T forward_control = AIR_L2_FWD_CTRL_DEFAULT;
    AIR_MAC_ENTRY_T   mt;

    /* cmd: l2 add mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> }
     *                      mac=<MACADDR> portlist=<UINTLIST> [ static ] [ unauth ]
     *                      [ age-time=<UINT> ] [ disable-egrs-vlan-filter ]
     *                      [ src-hit-ctrl={ default | cpu-exclude | cpu-include | cpu-only | drop } ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_vid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_vid, &vid), token_idx, 2);
        is_ivl = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_fid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_fid, &fid), token_idx, 2);
        is_ivl = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, _str_mac, &mac), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "static"))
    {
        token_idx++;
        is_static = TRUE;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "unauth"))
    {
        token_idx++;
        is_unauth = TRUE;
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "age-time"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "age-time", &timer), token_idx, 2);
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "disable-egrs-vlan-filter"))
    {
        token_idx++;
        dis_egrs_vlan = TRUE;
    }

    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "src-hit-ctrl", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "default"))
    {
        forward_control = AIR_L2_FWD_CTRL_DEFAULT;
    }
    else if (AIR_E_OK == dsh_checkString(str, "cpu-exclude"))
    {
        forward_control = AIR_L2_FWD_CTRL_CPU_EXCLUDE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "cpu-include"))
    {
        forward_control = AIR_L2_FWD_CTRL_CPU_INCLUDE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "cpu-only"))
    {
        forward_control = AIR_L2_FWD_CTRL_CPU_ONLY;
    }
    else if (AIR_E_OK == dsh_checkString(str, "drop"))
    {
        forward_control = AIR_L2_FWD_CTRL_DROP;
    }
    else
    {
        forward_control = AIR_L2_FWD_CTRL_DEFAULT;
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));

    osal_memcpy(mt.mac, mac, sizeof(AIR_MAC_T));
    mt.flags = 0;
    if (TRUE == is_ivl)
    {
        mt.cvid = (UI16_T)vid;
        mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    else
    {
        mt.fid = (UI16_T)fid;
    }

    if (TRUE == is_static)
    {
        mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;
    }

    if (TRUE == is_unauth)
    {
        mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_UNAUTH;
    }
    if (TRUE == dis_egrs_vlan)
    {
        mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
    }

    AIR_PORT_BITMAP_COPY(mt.port_bitmap, bitmap);

    /* sa_fwd field settings */
    mt.sa_fwd = forward_control;
    mt.timer = timer;

    rc = air_l2_addMacAddr(unit, &mt);

    if (AIR_E_TABLE_FULL == rc)
    {
        _l2_cmd_error("hash bucket is full\n");
    }
    else if (AIR_E_OK != rc)
    {
        _l2_cmd_error("add address fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_delMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    AIR_MAC_T       mac;
    UI32_T          vid = 0;
    UI32_T          fid = 0;
    BOOL_T          is_ivl = FALSE;
    AIR_MAC_ENTRY_T mt;

    /* cmd: l2 del mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> } mac=<MACADDR>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_vid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_vid, &vid), token_idx, 2);
        is_ivl = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_fid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_fid, &fid), token_idx, 2);
        is_ivl = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, _str_mac, &mac), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));

    osal_memcpy(mt.mac, mac, sizeof(AIR_MAC_T));
    mt.flags = 0;
    if (TRUE == is_ivl)
    {
        mt.cvid = (UI16_T)vid;
        mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    else
    {
        mt.fid = (UI16_T)fid;
    }

    rc = air_l2_delMacAddr(unit, &mt);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("delete mac address fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_clrMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: l2 clear mac-addr [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_clearMacAddr(unit);

    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("clear l2 mac table fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_showMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0;
    AIR_MAC_T        mac;
    UI32_T           vid = 0;
    UI32_T           fid = 0;
    BOOL_T           is_ivl = FALSE;
    AIR_MAC_ENTRY_T *ptr_mt;
    UI8_T            count;

    /* cmd: l2 show mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> } mac=<MACADDR>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_vid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_vid, &vid), token_idx, 2);
        is_ivl = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], _str_fid))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, _str_fid, &fid), token_idx, 2);
        is_ivl = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, _str_mac, &mac), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    ptr_mt = osal_alloc(sizeof(AIR_MAC_ENTRY_T), "cmd");
    if (NULL == ptr_mt)
    {
        _l2_cmd_error("allocate memory fail\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));

    osal_memcpy(ptr_mt->mac, mac, sizeof(AIR_MAC_T));
    if (TRUE == is_ivl)
    {
        ptr_mt->cvid = (UI16_T)vid;
        ptr_mt->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    else
    {
        ptr_mt->fid = (UI16_T)fid;
    }

    rc = air_l2_getMacAddr(unit, &count, ptr_mt);

    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        osal_printf("target mac entry does not exist\n");
        osal_free(ptr_mt);
        return (rc);
    }
    else if (AIR_E_OK != rc)
    {
        _l2_cmd_error("show mac address fail\n");
        osal_free(ptr_mt);
        return (rc);
    }

    /* Print MAC entry */
    _l2_cmd_printTblHead();
    _l2_cmd_printTblEntry(unit, 1, ptr_mt);

    osal_free(ptr_mt);
    return (rc);
}

#ifdef AIR_EN_L2_SHADOW
static AIR_ERROR_NO_T
_l2_cmd_traverse_cbk(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry,
    void                  *ptr_cookie)
{
    _L2_CMD_DUMPINFO_T *ptr_dump_info = (_L2_CMD_DUMPINFO_T *)ptr_cookie;

    /* Check if the pointer to the MAC entry and software table are valid */
    HAL_CHECK_PTR(ptr_mac_entry);
    HAL_CHECK_PTR(_ptr_l2_cmd_sw_table);
    /* Check if the dump info is valid */
    if (ptr_dump_info)
    {
        /* Check if the value matches the search criteria */
        switch (ptr_dump_info->type)
        {
            case AIR_L2_MAC_SEARCH_TYPE_VID:
                if (ptr_mac_entry->cvid != ptr_dump_info->value)
                {
                    return AIR_E_OK;
                }
                break;
            case AIR_L2_MAC_SEARCH_TYPE_FID:
                if (ptr_mac_entry->fid != ptr_dump_info->value)
                {
                    return AIR_E_OK;
                }
                break;
            case AIR_L2_MAC_SEARCH_TYPE_PORT:
                if (!CMLIB_BITMAP_BIT_CHK(ptr_mac_entry->port_bitmap, ptr_dump_info->value))
                {
                    return AIR_E_OK;
                }
                break;
            default:
                break;
        }

        /* Copy the MAC entry to the software table */
        osal_memcpy(&_ptr_l2_cmd_sw_table[ptr_dump_info->traverse_cnt], ptr_mac_entry, sizeof(AIR_MAC_ENTRY_T));
        ptr_dump_info->traverse_cnt++;
    }
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_l2_cmd_displayTable(
    const UI32_T     unit,
    AIR_MAC_ENTRY_T *ptr_mac_entry,
    const UI32_T     entry_cnt)
{
    HAL_CHECK_PTR(ptr_mac_entry);

    if (0 == entry_cnt)
    {
        osal_printf("mac table is empty\n");
    }
    else
    {
        _l2_cmd_printTblHead();
        _l2_cmd_printTblEntry(unit, entry_cnt, ptr_mac_entry);
        osal_printf("\nfound %d mac %s\n", entry_cnt, (entry_cnt > 1) ? "entries" : "entry");
    }
    return AIR_E_OK;
}
#endif /* End of AIR_EN_L2_SHADOW */

static AIR_ERROR_NO_T
_l2_cmd_dumpMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    C8_T                     str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                   unit = 0;
    UI32_T                   value = 0;
    AIR_L2_MAC_SEARCH_TYPE_T type = AIR_L2_MAC_SEARCH_TYPE_LAST;
#ifdef AIR_EN_L2_SHADOW
    _L2_CMD_DUMPINFO_T dump_info;
#else  /* Else of AIR_EN_L2_SHADOW */
    AIR_MAC_ENTRY_T *ptr_mt;
    UI8_T            count;
    UI32_T           bucket_size = 0;
    UI32_T           total_count = 0;
    UI32_T           wdog_count = 0;
#endif /* End of AIR_EN_L2_SHADOW */

    /* cmd: l2 dump mac-addr [ unit=<UINT> ] [ type={ vid | fid | port } value=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "type"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "value", &value), token_idx, 2);
        if (AIR_E_OK == dsh_checkString(str, _str_vid))
        {
            type = AIR_L2_MAC_SEARCH_TYPE_VID;
        }
        else if (AIR_E_OK == dsh_checkString(str, _str_fid))
        {
            type = AIR_L2_MAC_SEARCH_TYPE_FID;
        }
        else if (AIR_E_OK == dsh_checkString(str, _str_port))
        {
            type = AIR_L2_MAC_SEARCH_TYPE_PORT;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

#ifdef AIR_EN_L2_SHADOW
    /* Varables initialization */
    dump_info.type = type;
    dump_info.value = value;
    dump_info.traverse_cnt = 0;
    _ptr_l2_cmd_sw_table = osal_alloc(sizeof(AIR_MAC_ENTRY_T) * L2_MAX_ENTRY_NUM, "cmd");
    if (NULL == _ptr_l2_cmd_sw_table)
    {
        _l2_cmd_error("allocate memory for dump mac table failed\n");
        return AIR_E_NO_MEMORY;
    }

    rc = air_l2_traverseMacAddr(unit, _l2_cmd_traverse_cbk, &dump_info);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("traverse mac address failed, rc=(%u)\n", rc);
        osal_free(_ptr_l2_cmd_sw_table);
        _ptr_l2_cmd_sw_table = NULL;
        return rc;
    }

    _l2_cmd_displayTable(unit, _ptr_l2_cmd_sw_table, dump_info.traverse_cnt);
    osal_free(_ptr_l2_cmd_sw_table);
    _ptr_l2_cmd_sw_table = NULL;
#else  /* Else of AIR_EN_L2_SHADOW */
    rc = air_l2_getMacBucketSize(unit, &bucket_size);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("get max. set number failed, rc=(%u)\n", rc);
        return (rc);
    }

    ptr_mt = osal_alloc(sizeof(AIR_MAC_ENTRY_T) * bucket_size, "cmd");
    if (NULL == ptr_mt)
    {
        _l2_cmd_error("allocate memory failed\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);

    /* Get 1st MAC entry */
    total_count = 0;
    wdog_count = 0;
    if (AIR_L2_MAC_SEARCH_TYPE_LAST == type)
    {
        rc = air_l2_getMacAddr(unit, &count, ptr_mt);
    }
    else
    {
        rc = air_l2_searchMacAddr(unit, type, value, &count, ptr_mt);
    }
    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        osal_printf("mac table is empty\n");
        osal_free(ptr_mt);
        return (rc);
    }
    else if (AIR_E_OK != rc)
    {
        _l2_cmd_error("dump mac table failed, rc=(%u)\n", rc);
        osal_free(ptr_mt);
        return (rc);
    }
    total_count += count;
    wdog_count += count;

    /* Print 1st MAC entry */
    _l2_cmd_printTblHead();
    _l2_cmd_printTblEntry(unit, count, ptr_mt);

    while (1)
    {
        /* Get the other MAC entries */
        osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
        if (AIR_L2_MAC_SEARCH_TYPE_LAST == type)
        {
            rc = air_l2_getNextMacAddr(unit, &count, ptr_mt);
        }
        else
        {
            rc = air_l2_searchNextMacAddr(unit, type, value, &count, ptr_mt);
        }
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            break;
        }
        else if (AIR_E_OK != rc)
        {
            _l2_cmd_error("dump mac table failed, rc=(%u)\n", rc);
            break;
        }
        total_count += count;
        wdog_count += count;

        /* Print the other MAC entries */
        _l2_cmd_printTblEntry(unit, count, ptr_mt);
        if (wdog_count >= L2_WDOG_KICK_NUM)
        {
            osal_wdog_kick();
            wdog_count -= L2_WDOG_KICK_NUM;
        }
    }

    osal_printf("\ntotal %u %s\n", total_count, (total_count > 1) ? "entries" : "entry");
    osal_free(ptr_mt);
#endif /* End of AIR_EN_L2_SHADOW */
    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_flushMacAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    C8_T                    str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                  unit = 0, value = 0;
    AIR_L2_MAC_FLUSH_TYPE_T type = AIR_L2_MAC_FLUSH_TYPE_LAST;

    /* cmd: l2 flush mac-addr [ unit=<UINT> ] type={ vid | fid | port } value=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_vid))
    {
        type = AIR_L2_MAC_FLUSH_TYPE_VID;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_fid))
    {
        type = AIR_L2_MAC_FLUSH_TYPE_FID;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_port))
    {
        type = AIR_L2_MAC_FLUSH_TYPE_PORT;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "value", &value), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_flushMacAddr(unit, type, value);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("flush mac address failed, rc=(%u)\n", rc);
    }
    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_setMacAddrAgeOut(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         second;

    /* cmd:
     * l2 set age-time [ unit=<UINT> ] second=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "second", &second), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_setMacAddrAgeOut(unit, second);

    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("set age-time failed, rc=(%u)\n", rc);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_showMacAddrAgeOut(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         second;

    /* cmd:
     * l2 show age-time [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_getMacAddrAgeOut(unit, &second);

    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("show age-time failed, rc=(%u)\n", rc);
    }
    else
    {
        osal_printf("mac age time\n");
        osal_printf("%-10s: %u\n", "unit", unit);
        osal_printf("%-10s: %u second\n", "age time", second);
        osal_printf("\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_setMacAddrAgeOutMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T bitmap = {0};
    UI32_T            port;
    BOOL_T            mode;
    C8_T              str[DSH_CMD_MAX_LENGTH];

    /* cmd:
     * l2 set age-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, _str_mode, str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, _str_dis))
    {
        mode = FALSE;
    }
    else if (AIR_E_OK == dsh_checkString(str, _str_en))
    {
        mode = TRUE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_l2_setMacAddrAgeOutMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            _l2_cmd_error("set age-port-ctrl at port %u failed, rc=(%u)\n", port, rc);
            break;
        }
    }

    return (rc);
}

static void
_l2_cmd_printMacAddrAgeOutModePortlist(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T cmd_bitmap,
    const AIR_PORT_BITMAP_T val_bitmap)
{
    UI32_T port;
    osal_printf("%5s/%-4s%8s\n", _str_unit, _str_port, _str_mode);
    AIR_PORT_FOREACH(cmd_bitmap, port)
    {
        osal_printf("%5u/%2u  %8s\n", unit, port, AIR_PORT_CHK(val_bitmap, port) ? _str_en : _str_dis);
    }
}

static AIR_ERROR_NO_T
_l2_cmd_showMacAddrAgeOutMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T cmd_bitmap = {0};
    AIR_PORT_BITMAP_T val_bitmap = {0};
    UI32_T            port;
    BOOL_T            mode;

    /* cmd:
     * l2 show age-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &cmd_bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(cmd_bitmap, port)
    {
        rc = air_l2_getMacAddrAgeOutMode(unit, port, &mode);
        if (AIR_E_OK != rc)
        {
            _l2_cmd_error("show age-port-ctrl at port %u failed, rc=(%u)\n", port, rc);
            return (rc);
        }
        if (TRUE == mode)
        {
            AIR_PORT_ADD(val_bitmap, port);
        }
    }
    _l2_cmd_printMacAddrAgeOutModePortlist(unit, cmd_bitmap, val_bitmap);

    return (rc);
}

static AIR_ERROR_NO_T
_l2_cmd_setForwardMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    C8_T                 str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_FORWARD_TYPE_T   forwardType;
    AIR_FORWARD_ACTION_T forwardAction;
    AIR_PORT_BITMAP_T    pbm = {0};

    /* cmd:
     * l2 set forward-mode [ unit=<UINT> ] type={ bc | uuc | umc | uipmc }
     * action={ drop | flooding | forward-pbm portlist=<UINTLIST> }
     * */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "bc"))
    {
        forwardType = AIR_FORWARD_TYPE_BCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "uuc"))
    {
        forwardType = AIR_FORWARD_TYPE_UUCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "umc"))
    {
        forwardType = AIR_FORWARD_TYPE_UMCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "uipmc"))
    {
        forwardType = AIR_FORWARD_TYPE_UIPMCST;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "action", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "drop"))
    {
        forwardAction = AIR_FORWARD_ACTION_DROP;
    }
    else if (AIR_E_OK == dsh_checkString(str, "flooding"))
    {
        forwardAction = AIR_FORWARD_ACTION_FLOODING;
    }
    else if (AIR_E_OK == dsh_checkString(str, "forward-pbm"))
    {
        forwardAction = AIR_FORWARD_ACTION_TO_PBM;
        /* if action = forward-pbm, paser the portlist */
        DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_setForwardMode(unit, forwardType, forwardAction, pbm);

    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("set forward mode failed, rc=(%u)\n", rc);
        return rc;
    }

    return rc;
}

static AIR_ERROR_NO_T
_l2_cmd_getForwardMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    C8_T                 str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_FORWARD_TYPE_T   forwardType;
    AIR_FORWARD_ACTION_T forwardAction;
    AIR_PORT_BITMAP_T    pbm = {0};

    /* cmd:
     * l2 show forward-mode [ unit=<UINT> ] type={ bc | uuc | umc | uipmc }
     * */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "bc"))
    {
        forwardType = AIR_FORWARD_TYPE_BCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "uuc"))
    {
        forwardType = AIR_FORWARD_TYPE_UUCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "umc"))
    {
        forwardType = AIR_FORWARD_TYPE_UMCST;
    }
    else if (AIR_E_OK == dsh_checkString(str, "uipmc"))
    {
        forwardType = AIR_FORWARD_TYPE_UIPMCST;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_l2_getForwardMode(unit, forwardType, &forwardAction, pbm);
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("get forward mode failed, rc=(%u)\n", rc);
        return rc;
    }

    osal_printf("forward mode status\n");
    osal_printf("%-9s: %u\n", "unit", unit);
    osal_printf("%-9s: %s\n", "type", _air_forward_type_string[forwardType]);
    osal_printf("%-9s: %s", "action", _air_forward_action_string[forwardAction]);
    if (AIR_FORWARD_ACTION_TO_PBM == forwardAction)
    {
        CMD_PRINT_PORTLIST(pbm);
    }
    else
    {
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_l2_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         param = 0, size = 0, cnt = 0;
    AIR_SWC_RSRC_T rsrc_type = AIR_SWC_RSRC_L2_FDB;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};

    /* cmd:
     * l2 show resource [ unit=<UINT> ] [ type={ mac } ]
     * */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);

    /* It should check whether the input argument string matches, but there is only one string type here. */
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == rc)
    {
        rc = air_swc_getCapacity(unit, rsrc_type, param, &size);
    }
    if (AIR_E_OK == rc)
    {
        rc = air_swc_getUsage(unit, rsrc_type, param, &cnt);
    }
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("%-25s %6s/%6s\n", "type", "used", "total");
        osal_printf("%-25s %6u/%6u\n", "l2 address", cnt, size);
    }
    else
    {
        _l2_cmd_error("get resource info failed, rc=(%u)\n", rc);
    }

    return rc;
}

#ifdef AIR_EN_L2_SHADOW
static AIR_ERROR_NO_T
_l2_cmd_setNotifyCallback(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         mode = FALSE;

    /* cmd:l2 set notify [ unit=<UINT> ] mode={ enable | disable } */
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        mode = FALSE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        mode = TRUE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (TRUE == mode)
    {
        rc = air_l2_registerMacAddrNotifyCallback(unit, _l2_cmd_notify_cbk, &_l2_cmd_notifyEntryNum);
        if (AIR_E_TABLE_FULL == rc)
        {
            osal_printf("l2 notification table full\n");
            return rc;
        }
        if (AIR_E_ENTRY_EXISTS == rc)
        {
            osal_printf("the notification has been enabled\n");
            return rc;
        }
    }
    else
    {
        rc = air_l2_deregisterMacAddrNotifyCallback(unit, _l2_cmd_notify_cbk, &_l2_cmd_notifyEntryNum);
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            osal_printf("the notification has been disabled\n");
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("set l2 notification failed, rc=(%u)\n", rc);
        return rc;
    }

    return rc;
}

static AIR_ERROR_NO_T
_l2_cmd_setShadowCfg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         polling_en;
    UI32_T         interval;

    /* cmd:l2 set shadow-cfg [ unit=<UINT> ] polling={ enable | disable } [ interval=<UINT> ] */
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "polling", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        polling_en = FALSE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        polling_en = TRUE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "interval", &interval), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = HAL_FUNC_CALL(unit, l2, setShadowCfg, (unit, polling_en, interval));
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("set l2 shadow cfg failed, rc=(%u)\n", rc);
        return rc;
    }

    return rc;
}

static AIR_ERROR_NO_T
_l2_cmd_showShadowCfg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    BOOL_T         polling_en = 0;
    UI32_T         interval = 0;

    /* cmd:l2 show shadow-cfg [ unit=<UINT> ] */
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = HAL_FUNC_CALL(unit, l2, getShadowCfg, (unit, &polling_en, &interval));
    if (AIR_E_OK != rc)
    {
        _l2_cmd_error("show l2 shadow cfg failed, rc=(%u)\n", rc);
        return rc;
    }
    osal_printf("l2 shadow configurations:\n");
    osal_printf("unit          : %u\n", unit);
    osal_printf("polling mode  : %s\n", (TRUE == polling_en) ? "enabled" : "disable");
    osal_printf("interval time : %u\n", interval);
    osal_printf("\n");

    return rc;
}

#endif /* End of AIR_EN_L2_SHADOW */

/* clang-format off */
const static DSH_VEC_T  _l2_cmd_vec[] =
{
    {
        "add mac-addr", 2, _l2_cmd_addMacAddr,
        "l2 add mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> } mac=<MACADDR>\n"
        "portlist=<UINTLIST> [ static ] [ unauth ] [ age-time=<UINT> ]\n"
        "[ disable-egrs-vlan-filter ]\n"
        "[ src-hit-ctrl={ default | cpu-exclude | cpu-include | cpu-only | drop } ]\n"
    },
    {
        "del mac-addr", 2, _l2_cmd_delMacAddr,
        "l2 del mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> } mac=<MACADDR>\n"
    },
    {
        "show mac-addr", 2, _l2_cmd_showMacAddr,
        "l2 show mac-addr [ unit=<UINT> ] { vid=<UINT> | fid=<UINT> } mac=<MACADDR>\n"
    },
    {
        "dump mac-addr", 2, _l2_cmd_dumpMacAddr,
        "l2 dump mac-addr [ unit=<UINT> ] [ type={ vid | fid | port } value=<UINT> ]\n"
    },
    {
        "flush mac-addr", 2, _l2_cmd_flushMacAddr,
        "l2 flush mac-addr [ unit=<UINT> ] type={ vid | fid | port } value=<UINT>\n"
    },
    {
        "clear mac-addr", 2, _l2_cmd_clrMacAddr,
        "l2 clear mac-addr [ unit=<UINT> ]\n"
    },
    {
        "set age-time", 2, _l2_cmd_setMacAddrAgeOut,
        "l2 set age-time [ unit=<UINT> ] second=<UINT>\n"
    },
    {
        "show age-time", 2, _l2_cmd_showMacAddrAgeOut,
        "l2 show age-time [ unit=<UINT> ]\n"
    },
    {
        "set age-port-ctrl", 2, _l2_cmd_setMacAddrAgeOutMode,
        "l2 set age-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show age-port-ctrl", 2, _l2_cmd_showMacAddrAgeOutMode,
        "l2 show age-port-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set forward-mode", 2, _l2_cmd_setForwardMode,
        "l2 set forward-mode [ unit=<UINT> ] type={ bc | uuc | umc | uipmc }\n"
        "action={ drop | flooding | forward-pbm portlist=<UINTLIST> }\n"
    },
    {
        "show forward-mode", 2, _l2_cmd_getForwardMode,
        "l2 show forward-mode [ unit=<UINT> ] type={ bc | uuc | umc | uipmc }\n"
    },
    {
        "show resource", 2, _l2_cmd_showResource,
        "l2 show resource [ unit=<UINT> ] [ type={ mac } ]\n"
    },
#ifdef  AIR_EN_L2_SHADOW
    {
        "set notify", 2, _l2_cmd_setNotifyCallback,
        "l2 set notify [ unit=<UINT> ] mode={ enable | disable }\n"
    },
    {
        "set shadow-cfg", 2, _l2_cmd_setShadowCfg,
        "l2 set shadow-cfg [ unit=<UINT> ] polling={ enable | disable } [ interval=<UINT> ]\n"
    },
    {
        "show shadow-cfg", 2, _l2_cmd_showShadowCfg,
        "l2 show shadow-cfg [ unit=<UINT> ]\n"
    },
#endif/* End of AIR_EN_L2_SHADOW */
};
/* clang-format on */

AIR_ERROR_NO_T
l2_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _l2_cmd_vec, sizeof(_l2_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
l2_cmd_usager()
{
    return (dsh_usager(_l2_cmd_vec, sizeof(_l2_cmd_vec) / sizeof(DSH_VEC_T)));
}
