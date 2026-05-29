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

#include <cmd/ipmc_cmd.h>

#include <air_cfg.h>
#include <air_error.h>
#include <air_ipmc.h>
#include <air_port.h>
#include <air_types.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_util.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define IPMC_CMD_WDOG_KICK_NUM (100)

/* MACRO FUNCTION DECLARATIONS
 */
#define _ipmc_cmd_error(fmt, ...) osal_printf("***Error***, " fmt, ##__VA_ARGS__)
/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* STATIC VARIABLE DECLARATIONS
 */
const static C8_T *_str_unit = "unit";
const static C8_T *_str_vid = "vid";
const static C8_T *_str_gaddr = "group-address";
const static C8_T *_str_saddr = "source-address";
const static C8_T *_str_portlist = "portlist";
const static C8_T *_str_memCnt = "member-count";
const static C8_T *_str_na = "---";
const static C8_T *_str_en = "enable";
const static C8_T *_str_dis = "disable";
const static C8_T *_str_ipmc_type[AIR_IPMC_MATCH_TYPE_LAST] = {"group ipv4", "group & source ipv4", "group ipv6",
                                                               "group & source ipv6"};

static void
_ipmc_cmd_getMcastStr(
    AIR_IP_ADDR_T *group_addr,
    AIR_IP_ADDR_T *source_addr,
    C8_T          *dip_str,
    C8_T          *sip_str)
{
    osal_memset(dip_str, 0, CMLIB_UTIL_IP_ADDR_STR_SIZE);
    osal_memset(sip_str, 0, CMLIB_UTIL_IP_ADDR_STR_SIZE);
    cmlib_util_getIpAddrStr(group_addr, dip_str);
    cmlib_util_getIpAddrStr(source_addr, sip_str);
    return;
}

static AIR_IPMC_MATCH_TYPE_T
_ipmc_cmd_getMatchType(
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr))
    {
        if (TRUE == ptr_entry->group_addr.ipv4)
        {
            return AIR_IPMC_MATCH_TYPE_IPV4_GRP;
        }
        else
        {
            return AIR_IPMC_MATCH_TYPE_IPV6_GRP;
        }
    }
    else
    {
        if (TRUE == ptr_entry->group_addr.ipv4)
        {
            return AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC;
        }
        else
        {
            return AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC;
        }
    }
}

static void
_ipmc_cmd_printTblLine(
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
_ipmc_cmd_printTblHead(
    const AIR_IPMC_MATCH_TYPE_T match_type,
    const BOOL_T                is_memCnt)
{
    if (match_type < AIR_IPMC_MATCH_TYPE_IPV6_GRP)
    {
        osal_printf("%5s %-5s%-16s%-16s%s\n", _str_unit, _str_vid, _str_gaddr, _str_saddr,
                    (TRUE == is_memCnt) ? _str_memCnt : _str_portlist);
        _ipmc_cmd_printTblLine(56);
    }
    else
    {
        osal_printf("%5s %-5s%-40s%-40s%s\n", _str_unit, _str_vid, _str_gaddr, _str_saddr,
                    (TRUE == is_memCnt) ? _str_memCnt : _str_portlist);
        _ipmc_cmd_printTblLine(105);
    }
}

static void
_ipmc_cmd_printTblEntry(
    const UI32_T                unit,
    const AIR_IPMC_MATCH_TYPE_T match_type,
    const BOOL_T                is_memCnt,
    const UI32_T                count,
    AIR_IPMC_ENTRY_T           *ptr_entry)
{
    C8_T dip_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];
    C8_T sip_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];
    C8_T pbmp_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];

    _ipmc_cmd_getMcastStr(&ptr_entry->group_addr, &ptr_entry->source_addr, dip_str, sip_str);
    switch (match_type)
    {
        case AIR_IPMC_MATCH_TYPE_IPV4_GRP:
            osal_printf("%5u %-5u%-16s%-16s", unit, ptr_entry->vid, dip_str, _str_na);
            break;
        case AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC:
            osal_printf("%5u %-5u%-16s%-16s", unit, ptr_entry->vid, dip_str, sip_str);
            break;
        case AIR_IPMC_MATCH_TYPE_IPV6_GRP:
            osal_printf("%5u %-5u%-40s%-40s", unit, ptr_entry->vid, dip_str, _str_na);
            break;
        case AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC:
            osal_printf("%5u %-5u%-40s%-40s", unit, ptr_entry->vid, dip_str, sip_str);
            break;
        default:
            _ipmc_cmd_error("unknown match type:%u\n", match_type);
    }

    if (TRUE == is_memCnt)
    {
        osal_printf("%u", count);
    }
    else
    {
        CMD_STRING_PORTLIST(pbmp_str, ptr_entry->port_bitmap);
        osal_printf("%s", pbmp_str);
    }
    osal_printf("\n");
}

static void
_ipmc_cmd_printMcastStr(
    C8_T             *str_op,
    AIR_IPMC_TYPE_T   type,
    UI32_T            vid,
    AIR_IP_ADDR_T    *group_addr,
    AIR_IP_ADDR_T    *source_addr,
    AIR_PORT_BITMAP_T portmap,
    UI32_T            flags,
    AIR_ERROR_NO_T    rc)
{
    C8_T dip_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];
    C8_T sip_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];
    C8_T pbmp_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];

    _ipmc_cmd_getMcastStr(group_addr, source_addr, dip_str, sip_str);
    CMD_STRING_PORTLIST(pbmp_str, portmap);
    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        if (type == AIR_IPMC_TYPE_GRP_SRC)
        {
            _ipmc_cmd_error("can not find vid %d group %s source %s\n", vid, dip_str, sip_str);
        }
        else
        {
            _ipmc_cmd_error("can not find vid %d group %s\n", vid, dip_str);
        }
    }
    else if (AIR_E_ENTRY_EXISTS == rc)
    {
        if (AIR_IPMC_TYPE_GRP_SRC == type)
        {
            _ipmc_cmd_error("vid %d group %s source %s exists\n", vid, dip_str, sip_str);
        }
        else
        {
            _ipmc_cmd_error("vid %d group %s exists\n", vid, dip_str);
        }
    }
    else
    {
        if (AIR_IPMC_TYPE_GRP_SRC == type)
        {
            _ipmc_cmd_error("%s vid %d group %s source %s portlist %s failed\n", str_op, vid, dip_str, sip_str,
                            pbmp_str);
        }
        else
        {
            _ipmc_cmd_error("%s vid %d group %s portlist %s failed\n", str_op, vid, dip_str, pbmp_str);
        }
    }
    return;
}

static AIR_ERROR_NO_T
_ipmc_cmd_addMcastAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0;
    AIR_PORT_BITMAP_T pbm;
    AIR_IPMC_ENTRY_T  entry;
    AIR_IP_ADDR_T     group_addr, source_addr;

    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    AIR_PORT_BITMAP_CLEAR(pbm);
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "disable-egrs-vlan-filter"))
    {
        token_idx++;
        entry.flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_BITMAP_COPY(entry.port_bitmap, pbm);

    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));

    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }

    if ((AIR_IPMC_TYPE_GRP_SRC == entry.type) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_ipmc_addMcastAddr(unit, &entry);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_printMcastStr("add", entry.type, entry.vid, &entry.group_addr, &entry.source_addr, entry.port_bitmap,
                                entry.flags, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_getMcastAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, vid = 0;
    AIR_IPMC_MATCH_TYPE_T match_type;
    AIR_IPMC_ENTRY_T      entry;
    AIR_IP_ADDR_T         group_addr, source_addr;

    /* cmd:
     * ipmc show mcast-addr [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]
     * */
    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    /* parse the input command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* input parameters checking */
    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));

    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }

    if ((AIR_IPMC_TYPE_GRP_SRC == entry.type) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    /* try to get the IPMC entry */
    rc = air_ipmc_getMcastAddr(unit, &entry);
    if (rc == AIR_E_OK)
    {
        match_type = _ipmc_cmd_getMatchType(&entry);
        _ipmc_cmd_printTblHead(match_type, FALSE);
        _ipmc_cmd_printTblEntry(unit, match_type, FALSE, 0, &entry);
    }
    else
    {
        _ipmc_cmd_error("get ipmc entry failed, rc=(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_dumpMcastAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, i = 0, set_num = 0;
    C8_T                  search_type_str[DSH_CMD_MAX_LENGTH];
    AIR_IPMC_MATCH_TYPE_T match_type;
    UI32_T                rt_cnt = 0, total_cnt = 0, wdog_cnt = 0;
    AIR_IPMC_ENTRY_T     *ptr_entry;
    AIR_IPMC_ENTRY_T      entry;
    AIR_IP_ADDR_T         group_addr, source_addr;

    /* cmd:
     * ipmc dump mcast-addr [ unit=<UINT> ] search-type={ dip4 | dip4sip4 | dip6 | dip6sip6 }
     * */
    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    /* parse the input command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "search-type", search_type_str), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(search_type_str, "dip4"))
    {
        match_type = AIR_IPMC_MATCH_TYPE_IPV4_GRP;
    }
    else if (AIR_E_OK == dsh_checkString(search_type_str, "dip4sip4"))
    {
        match_type = AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC;
    }
    else if (AIR_E_OK == dsh_checkString(search_type_str, "dip6"))
    {
        match_type = AIR_IPMC_MATCH_TYPE_IPV6_GRP;
    }
    else if (AIR_E_OK == dsh_checkString(search_type_str, "dip6sip6"))
    {
        match_type = AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* get the bucket size */
    rc = air_ipmc_getMcastBucketSize(unit, &set_num);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_error("get bucket size failed, rc=(%u)\n", rc);
        return rc;
    }

    /* allocate temporary entry buffer */
    ptr_entry = osal_alloc(sizeof(AIR_IPMC_ENTRY_T) * set_num, "cmd");
    if (NULL == ptr_entry)
    {
        _ipmc_cmd_error("allocate memory failed\n");
        return AIR_E_NO_MEMORY;
    }

    /* try to get the 1st IPMC entry */
    osal_memset(ptr_entry, 0, sizeof(AIR_IPMC_ENTRY_T) * set_num);
    rt_cnt = 0;
    rc = air_ipmc_getFirstMcastAddr(unit, match_type, &rt_cnt, ptr_entry);
    if (AIR_E_OK != rc)
    {
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            osal_printf("ipmc table(%s) is empty\n", _str_ipmc_type[match_type]);
        }
        else
        {
            _ipmc_cmd_error("dump ipmc table failed\n");
        }
        osal_free(ptr_entry);
        return rc;
    }
    total_cnt += rt_cnt;
    wdog_cnt += rt_cnt;

    _ipmc_cmd_printTblHead(match_type, FALSE);
    for (i = 0; i < rt_cnt; i++)
    {
        _ipmc_cmd_printTblEntry(unit, match_type, FALSE, 0, &(ptr_entry[i]));
    }

    while (AIR_E_OK == rc)
    {
        /* Try to get the next IPMC entry */
        osal_memset(ptr_entry, 0, sizeof(AIR_IPMC_ENTRY_T) * set_num);
        rt_cnt = 0;
        rc = air_ipmc_getNextMcastAddr(unit, match_type, &rt_cnt, ptr_entry);
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            break;
        }
        else if (AIR_E_OK != rc)
        {
            _ipmc_cmd_error("dump mac table failed, rc=(%u)\n", rc);
            break;
        }
        for (i = 0; i < rt_cnt; i++)
        {
            _ipmc_cmd_printTblEntry(unit, match_type, FALSE, 0, &(ptr_entry[i]));
        }
        total_cnt += rt_cnt;
        wdog_cnt += rt_cnt;

        if (wdog_cnt >= IPMC_CMD_WDOG_KICK_NUM)
        {
            osal_wdog_kick();
            wdog_cnt -= IPMC_CMD_WDOG_KICK_NUM;
        }
    }

    osal_printf("\ntotal %u %s %s\n", total_cnt, _str_ipmc_type[match_type], (total_cnt > 1) ? "entries" : "entry");
    osal_free(ptr_entry);
    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_delMcastAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0, vid = 0;
    AIR_IPMC_ENTRY_T entry;
    AIR_IP_ADDR_T    group_addr, source_addr;

    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));

    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }
    if ((entry.type == AIR_IPMC_TYPE_GRP_SRC) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_ipmc_delMcastAddr(unit, &entry);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_printMcastStr("delete", entry.type, entry.vid, &entry.group_addr, &entry.source_addr,
                                entry.port_bitmap, entry.flags, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_delAllMcastAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ipmc_delAllMcastAddr(unit);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_error("delete all entry failed, rc=(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_addMcastMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0;
    AIR_IP_ADDR_T     group_addr, source_addr;
    AIR_PORT_BITMAP_T pbm;
    AIR_IPMC_ENTRY_T  entry;

    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));
    AIR_PORT_BITMAP_CLEAR(pbm);
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_BITMAP_COPY(entry.port_bitmap, pbm);

    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }

    if ((entry.type == AIR_IPMC_TYPE_GRP_SRC) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_ipmc_addMcastMember(unit, &entry);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_printMcastStr("add", entry.type, entry.vid, &entry.group_addr, &entry.source_addr, entry.port_bitmap,
                                entry.flags, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_delMcastMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, vid = 0;
    AIR_PORT_BITMAP_T pbm;
    AIR_IPMC_ENTRY_T  entry;
    AIR_IP_ADDR_T     group_addr, source_addr;

    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));
    AIR_PORT_BITMAP_CLEAR(pbm);
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_BITMAP_COPY(entry.port_bitmap, pbm);

    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));

    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }

    if ((entry.type == AIR_IPMC_TYPE_GRP_SRC) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_ipmc_delMcastMember(unit, &entry);
    if (AIR_E_OK != rc)
    {
        _ipmc_cmd_printMcastStr("remove", entry.type, entry.vid, &entry.group_addr, &entry.source_addr,
                                entry.port_bitmap, entry.flags, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_getMcastMemberCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                count = 0, unit = 0, vid = 0;
    AIR_IPMC_MATCH_TYPE_T match_type;
    AIR_IPMC_ENTRY_T      entry;
    AIR_IP_ADDR_T         group_addr, source_addr;

    /* cmd:
     * ipmc show mcast-member-cnt [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]
     * */
    osal_memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    /* parse the input command */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "vid", &vid), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "gaddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "gaddr", &group_addr.ip_addr.ipv6_addr), group_addr.ipv4,
                          token_idx, 2);
        if (!AIR_L3_IP_IS_MULTICAST(&group_addr))
        {
            _ipmc_cmd_error("input gaddr shall be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "saddr"))
    {
        DSH_CHECK_IP_ADDR(dsh_getIpv4Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv4_addr),
                          dsh_getIpv6Addr(tokens, token_idx, "saddr", &source_addr.ip_addr.ipv6_addr), source_addr.ipv4,
                          token_idx, 2);
        if (AIR_L3_IP_IS_MULTICAST(&source_addr))
        {
            _ipmc_cmd_error("input saddr shall not be a multicast address\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* input parameters checking */
    entry.vid = vid;
    osal_memcpy(&entry.group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
    osal_memcpy(&entry.source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));

    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(entry.source_addr))
    {
        entry.type = AIR_IPMC_TYPE_GRP;
    }
    else
    {
        entry.type = AIR_IPMC_TYPE_GRP_SRC;
    }

    if ((AIR_IPMC_TYPE_GRP_SRC == entry.type) && (group_addr.ipv4 != source_addr.ipv4))
    {
        _ipmc_cmd_error("ip family of group and source are not the same\n");
        return AIR_E_BAD_PARAMETER;
    }

    /* try to get the amount of member of IPMC entry */
    rc = air_ipmc_getMcastMemberCnt(unit, &entry, &count);
    if (AIR_E_OK == rc)
    {
        match_type = _ipmc_cmd_getMatchType(&entry);
        _ipmc_cmd_printTblHead(match_type, TRUE);
        _ipmc_cmd_printTblEntry(unit, match_type, TRUE, count, &entry);
    }
    else if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        _ipmc_cmd_error("get ipmc entry member amount failed, rc=(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_setMcastLookupType(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    C8_T            type_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_IPMC_TYPE_T type = AIR_IPMC_TYPE_LAST;

    /* cmd:
     * ipmc set ipmc-lookup-type [ unit=<UINT> ] type={ grp | grp-src | grp-src-grp }
     * */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(type_str, "grp"))
    {
        type = AIR_IPMC_TYPE_GRP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "grp-src"))
    {
        type = AIR_IPMC_TYPE_GRP_SRC;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "grp-src-grp"))
    {
        type = AIR_IPMC_TYPE_GRP_SRC_AND_GRP;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ipmc_setMcastLookupType(unit, type);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        _ipmc_cmd_error("this feature is not supported on this chip\n");
    }
    else if (AIR_E_OK != rc)
    {
        _ipmc_cmd_error("set ipmc lookup type failed, rc=(%u)\n", rc);
        return rc;
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_getMcastLookupType(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    AIR_IPMC_TYPE_T type = AIR_IPMC_TYPE_LAST;

    /* cmd:
     * ipmc show ipmc-lookup-type [ unit=<UINT> ]
     * */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ipmc_getMcastLookupType(unit, &type);
    if (rc == AIR_E_OK)
    {
        osal_printf("ipmc lookup type : %s\n", (AIR_IPMC_TYPE_GRP == type) ?
                                                   "grp" :
                                                   ((AIR_IPMC_TYPE_GRP_SRC == type) ? "grp-src" : "grp-src-grp"));
    }
    else
    {
        _ipmc_cmd_error("get ipmc lookup type failed, rc=(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_setPortIpmcMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, mode = 0, port = 0;
    C8_T              str[DSH_CMD_MAX_LENGTH];
    AIR_PORT_BITMAP_T pbm;

    osal_memset(str, 0, sizeof(str));
    osal_memset(pbm, 0, sizeof(AIR_PORT_BITMAP_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);

    AIR_PORT_FOREACH(pbm, port)
    {
        if (!AIR_PORT_CHK(HAL_PORT_BMP(unit), port))
        {
            _ipmc_cmd_error("member portlist invalid\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
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
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_ipmc_setPortIpmcMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            _ipmc_cmd_error("set port=%u multicast data forward mode %s failed, rc=(%u)\n", port,
                            (mode ? _str_en : _str_dis), rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_getPortIpmcMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    BOOL_T            mode = FALSE;
    AIR_PORT_BITMAP_T pbm;

    /* cmd:
     * ipmc show ipmc-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */
    osal_memset(pbm, 0, sizeof(AIR_PORT_BITMAP_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &pbm), token_idx, 2);

    AIR_PORT_FOREACH(pbm, port)
    {
        if (!AIR_PORT_CHK(HAL_PORT_BMP(unit), port))
        {
            _ipmc_cmd_error("member portlist invalid\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%5s/%-4s%8s\n", _str_unit, "port", "mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_ipmc_getPortIpmcMode(unit, port, &mode);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5u/%2u  %8s\n", unit, port, (TRUE == mode) ? _str_en : _str_dis);
        }
        else
        {
            _ipmc_cmd_error("get port=%u multicast data forward mode failed, rc=(%u)\n", port, rc);
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_showResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         param = 0, size = 0, cnt = 0;

    /* cmd:
     * ipmc show resource [ unit=<UINT> ]
     * */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == rc)
    {
        rc = air_swc_getCapacity(unit, AIR_SWC_RSRC_IPMC_FDB, param, &size);
    }
    if (AIR_E_OK == rc)
    {
        rc = air_swc_getUsage(unit, AIR_SWC_RSRC_IPMC_FDB, param, &cnt);
    }
    if (AIR_E_OK == rc)
    {
        osal_printf("unit %d\n", unit);
        osal_printf("%-25s %6s/%6s\n", "type", "used", "total");
        osal_printf("%-25s %6u/%6u\n", "ip multicast address", cnt, size);
    }
    else
    {
        _ipmc_cmd_error("get resource info failed, rc=(%u)\n", rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_ipmc_cmd_setPortLookupIpTypeCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T bitmap = {0};
    UI32_T            port = 0;
    BOOL_T            mode = FALSE;
    AIR_IP_TYPE_T     ip_type = AIR_IP_TYPE_IPV4;
    C8_T              str[DSH_CMD_MAX_LENGTH];

    /* cmd:
     * ipmc set lookup-ip-type-ctrl [ unit=<UINT> ] portlist=<UINTLIST> type={ ipv4 | ipv6 } mode={ enable | disable }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "ipv4"))
    {
        ip_type = AIR_IP_TYPE_IPV4;
    }
    else if (AIR_E_OK == dsh_checkString(str, "ipv6"))
    {
        ip_type = AIR_IP_TYPE_IPV6;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
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
        rc = air_ipmc_setPortLookupIpTypeCtrl(unit, port, ip_type, mode);
        if (AIR_E_OK != rc)
        {
            _ipmc_cmd_error("set lookup-ip-type-ctrl at port %u failed, rc=(%u)\n", port, rc);
            break;
        }
    }

    return (rc);
}

static void
_ipmc_cmd_printPortLookupIpTypeCtrlPortlist(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T cmd_bitmap,
    const AIR_PORT_BITMAP_T ipv4_bitmap,
    const AIR_PORT_BITMAP_T ipv6_bitmap)
{
    UI32_T port;
    osal_printf("%5s/%-4s%8s%8s\n", _str_unit, "port", "ipv4", "ipv6");
    AIR_PORT_FOREACH(cmd_bitmap, port)
    {
        osal_printf("%5u/%2u  %8s%8s\n", unit, port, AIR_PORT_CHK(ipv4_bitmap, port) ? _str_en : _str_dis,
                    AIR_PORT_CHK(ipv6_bitmap, port) ? _str_en : _str_dis);
    }
}

static AIR_ERROR_NO_T
_ipmc_cmd_showPortLookupIpTypeCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T cmd_bitmap = {0};
    AIR_PORT_BITMAP_T ipv4_bitmap = {0};
    AIR_PORT_BITMAP_T ipv6_bitmap = {0};
    UI32_T            port = 0;
    BOOL_T            mode = FALSE;
    AIR_IP_TYPE_T     ip_type = AIR_IP_TYPE_IPV4;

    /* cmd:
     * ipmc show lookup-ip-type-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, _str_portlist, unit, &cmd_bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(cmd_bitmap, port)
    {
        for (ip_type = AIR_IP_TYPE_IPV4; ip_type < AIR_IP_TYPE_LAST; ip_type++)
        {
            rc = air_ipmc_getPortLookupIpTypeCtrl(unit, port, ip_type, &mode);
            if (AIR_E_OK != rc)
            {
                _ipmc_cmd_error("show lookup-ip-type-ctrl at port %u failed, rc=(%u)\n", port, rc);
                break;
            }
            if (TRUE == mode)
            {
                AIR_PORT_ADD((AIR_IP_TYPE_IPV4 == ip_type) ? ipv4_bitmap : ipv6_bitmap, port);
            }
        }
    }
    if (AIR_E_OK == rc)
    {
        _ipmc_cmd_printPortLookupIpTypeCtrlPortlist(unit, cmd_bitmap, ipv4_bitmap, ipv6_bitmap);
    }

    return (rc);
}

/* clang-format off */
const static DSH_VEC_T  _ipmc_cmd_vec[] =
{
    {
        "set ipmc-lookup-type", 2, _ipmc_cmd_setMcastLookupType,
        "ipmc set ipmc-lookup-type [ unit=<UINT> ] type={ grp | grp-src | grp-src-grp }\n"
    },
    {
        "show ipmc-lookup-type", 2, _ipmc_cmd_getMcastLookupType,
        "ipmc show ipmc-lookup-type [ unit=<UINT> ]\n"
    },
    {
        "add mcast-addr", 2, _ipmc_cmd_addMcastAddr,
        "ipmc add mcast-addr [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]\n"
        "portlist=<UINTLIST> [ disable-egrs-vlan-filter ]\n"
    },
    {
        "del mcast-addr", 2, _ipmc_cmd_delMcastAddr,
        "ipmc del mcast-addr [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]\n"
    },
    {
        "show mcast-addr", 2, _ipmc_cmd_getMcastAddr,
        "ipmc show mcast-addr [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]\n"
    },
    {
        "dump mcast-addr", 2, _ipmc_cmd_dumpMcastAddr,
        "ipmc dump mcast-addr [ unit=<UINT> ]\n"
        "search-type={ dip4 | dip4sip4 | dip6 | dip6sip6 }\n"
    },
    {
        "clear mcast-addr", 2, _ipmc_cmd_delAllMcastAddr,
        "ipmc clear mcast-addr [ unit=<UINT> ]\n"
    },
    {
        "add mcast-member", 2, _ipmc_cmd_addMcastMember,
        "ipmc add mcast-member [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]\n"
        "portlist=<UINTLIST>\n"
    },
    {
        "del mcast-member", 2, _ipmc_cmd_delMcastMember,
        "ipmc del mcast-member [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR> [ saddr=<IPADDR> ]\n"
        "portlist=<UINTLIST>\n"
    },
    {
        "show mcast-member-cnt", 2, _ipmc_cmd_getMcastMemberCnt,
        "ipmc show mcast-member-cnt [ unit=<UINT> ] vid=<UINT> gaddr=<IPADDR>\n"
        "[ saddr=<IPADDR> ]\n"
    },
    {
        "set ipmc-mode", 2, _ipmc_cmd_setPortIpmcMode,
        "ipmc set ipmc-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show ipmc-mode", 2, _ipmc_cmd_getPortIpmcMode,
        "ipmc show ipmc-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show resource", 2, _ipmc_cmd_showResource,
        "ipmc show resource [ unit=<UINT> ]\n"
    },
    {
        "set lookup-ip-type-ctrl", 2, _ipmc_cmd_setPortLookupIpTypeCtrl,
        "ipmc set lookup-ip-type-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ ipv4 | ipv6 } mode={ enable | disable }\n"
    },
    {
        "show lookup-ip-type-ctrl", 2, _ipmc_cmd_showPortLookupIpTypeCtrl,
        "ipmc show lookup-ip-type-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
ipmc_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _ipmc_cmd_vec, sizeof(_ipmc_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
ipmc_cmd_usager()
{
    return (dsh_usager(_ipmc_cmd_vec, sizeof(_ipmc_cmd_vec) / sizeof(DSH_VEC_T)));
}
