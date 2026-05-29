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

#include <cmd/lag_cmd.h>

#include <air_error.h>
#include <air_lag.h>
#include <air_types.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

#define AIR_LAG_CMD_MAX_GROUP_NUM (0xFF)

static AIR_ERROR_NO_T
_lag_cmd_create(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         lag_id = 0;

    /* cmd: lag create group [ unit=<UINT> ] lag-id=<UINT>
    */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "lag-id", &lag_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_createGroup(unit, lag_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, create lag fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_destroy(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_PORT_T     lag_id = 0;

    /* cmd: lag destroy group [ unit=<UINT> ] lag-id=<UINT>
    */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "lag-id", &lag_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_destroyGroup(unit, lag_id);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, destroy lag fail\n");
    }

    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_addMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_T        lag_id = 0;
    AIR_PORT_BITMAP_T bitmap;
    UI32_T            port = 0;
    BOOL_T            state = FALSE;
    UI32_T            max_group_cnt = 0;
    UI32_T            max_member_cnt = 0;
    UI32_T            new_member_cnt = 0;
    UI32_T            member_cnt = 0;
    UI32_T           *ptr_member = NULL;
    UI32_T            member_idx = 0;

    AIR_PORT_BITMAP_CLEAR(bitmap);

    /* lag add member [ unit=<UINT> ] lag-id=<UINT> portlist=<UINTLIST> */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "lag-id", &lag_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_getGroup(unit, lag_id, &state);
    if (AIR_E_OK == rc)
    {
        if (TRUE == state)
        {
            rc = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
            AIR_PORT_FOREACH(bitmap, port)
            {
                new_member_cnt++;
            }
            ptr_member = (UI32_T *)osal_alloc(sizeof(UI32_T) * max_member_cnt, "cmd");
            if (NULL != ptr_member)
            {
                /* check new member is duplicated or not. */
                rc = air_lag_getMember(unit, lag_id, &member_cnt, ptr_member);
                AIR_PORT_FOREACH(bitmap, port)
                {
                    for (member_idx = 0; member_idx < member_cnt; member_idx++)
                    {
                        if (port == ptr_member[member_idx])
                        {
                            osal_printf("***Error***, port %u is duplicated\n", port);
                            rc = AIR_E_BAD_PARAMETER;
                            break;
                        }
                    }
                }
                if (AIR_E_OK == rc)
                {
                    if (0 == (max_member_cnt - member_cnt))
                    {
                        osal_printf("***Error***, the count of lag-id=%u members has acheived maximum.\n", lag_id);
                        rc = AIR_E_BAD_PARAMETER;
                    }
                    if (AIR_E_OK == rc)
                    {
                        if (new_member_cnt <= (max_member_cnt - member_cnt))
                        {
                            /* append new member */
                            AIR_PORT_FOREACH(bitmap, port)
                            {
                                ptr_member[member_cnt++] = port;
                            }
                            rc = air_lag_setMember(unit, lag_id, member_cnt, ptr_member);
                            if (AIR_E_OK != rc)
                            {
                                osal_printf("***Error***, set lag memer fail\n");
                            }
                        }
                        else
                        {
                            osal_printf("***Error***, %u lag member%s %s allowed.\n", (max_member_cnt - member_cnt),
                                        ((max_member_cnt - member_cnt) == 1 ? "" : "s"),
                                        ((max_member_cnt - member_cnt) == 1 ? "is" : "are"));
                            rc = AIR_E_BAD_PARAMETER;
                        }
                    }
                }
            }
            else
            {
                osal_printf("***Error***, out of memory\n");
                rc = AIR_E_NO_MEMORY;
            }
        }
        else
        {
            osal_printf("***Error***, lag_id=%u is disabled.\n", lag_id);
            rc = AIR_E_BAD_PARAMETER;
        }
    }
    else
    {
        osal_printf("***Error***, get lag fail\n");
    }

    if (NULL != ptr_member)
    {
        osal_free(ptr_member);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_delMember(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_PORT_T        lag_id = 0;
    AIR_PORT_BITMAP_T bitmap;
    UI32_T            port = 0;
    UI32_T            max_group_cnt = 0;
    UI32_T            max_member_cnt = 0;
    UI32_T            member_cnt = 0;
    UI32_T            new_member_cnt = 0;
    UI32_T           *ptr_member = NULL;
    UI32_T           *ptr_new_member = NULL;
    UI32_T            member_idx = 0;
    BOOL_T            is_exist = FALSE;

    AIR_PORT_BITMAP_CLEAR(bitmap);

    /* lag del member [ unit=<UINT> ] lag-id=<UINT> portlist=<UINTLIST> */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "lag-id", &lag_id), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);

    ptr_member = (UI32_T *)osal_alloc(sizeof(UI32_T) * max_member_cnt, "cmd");
    ptr_new_member = (UI32_T *)osal_alloc(sizeof(UI32_T) * max_member_cnt, "cmd");
    osal_memset(ptr_member, 0, sizeof(UI32_T) * max_member_cnt);
    osal_memset(ptr_new_member, 0, sizeof(UI32_T) * max_member_cnt);
    if (NULL != ptr_member && NULL != ptr_new_member)
    {
        rc = air_lag_getMember(unit, lag_id, &member_cnt, ptr_member);
        if (AIR_E_OK == rc)
        {
            AIR_PORT_FOREACH(bitmap, port)
            {
                is_exist = FALSE;
                for (member_idx = 0; member_idx < member_cnt; member_idx++)
                {
                    if (port == ptr_member[member_idx])
                    {
                        is_exist = TRUE;
                        break;
                    }
                }
                if (FALSE == is_exist)
                {
                    osal_printf("***Error***, port %u is not lag member.\n", port);
                    rc = AIR_E_BAD_PARAMETER;
                }
            }
            if (AIR_E_OK == rc)
            {
                for (member_idx = 0; member_idx < member_cnt; member_idx++)
                {
                    if (!AIR_PORT_CHK(bitmap, ptr_member[member_idx]))
                    {
                        ptr_new_member[new_member_cnt++] = ptr_member[member_idx];
                    }
                }
                rc = air_lag_setMember(unit, lag_id, new_member_cnt, ptr_new_member);
                if (AIR_E_OK != rc)
                {
                    osal_printf("***Error***, set lag members fail.\n");
                }
            }
        }
        else
        {
            osal_printf("***Error***, get lag members fail.\n");
        }
    }
    else
    {
        osal_printf("***Error***, out of memory\n");
        rc = AIR_E_NO_MEMORY;
    }

    if (NULL != ptr_member)
    {
        osal_free(ptr_member);
    }
    if (NULL != ptr_new_member)
    {
        osal_free(ptr_new_member);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_showLagMember(
    const UI32_T      unit,
    const AIR_PORT_T *ptr_member_arr,
    const UI32_T      member_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         member_idx = 0;
    UI32_T         first_print = 1;

    if (0 == member_cnt)
    {
        osal_printf("memberlist: null");
    }
    else
    {
        osal_printf("memberlist: ");
        while (member_idx < member_cnt)
        {
            if (ptr_member_arr[member_idx] != AIR_PORT_INVALID)
            {
                if (first_print)
                {
                    first_print = 0;
                }
                else
                {
                    osal_printf(",");
                }
                osal_printf("%d", ptr_member_arr[member_idx]);
            }
            member_idx++;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_lag_cmd_show(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_PORT_T     lag_id = 0;
    BOOL_T         state = FALSE;
    UI32_T         max_group_cnt = 0;
    UI32_T         max_member_cnt = 0;
    UI32_T        *ptr_member = NULL;
    UI32_T         member_cnt = 0;

    /* cmd: lag show group [ unit=<UINT> ] lag-id=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "lag-id", &lag_id), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_getGroup(unit, lag_id, &state);
    if (AIR_E_OK == rc)
    {
        rc = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
        ptr_member = (UI32_T *)osal_alloc(sizeof(UI32_T) * max_member_cnt, "cmd");
        if (NULL != ptr_member)
        {
            osal_memset(ptr_member, 0, sizeof(UI32_T) * max_member_cnt);
            rc = air_lag_getMember(unit, lag_id, &member_cnt, ptr_member);
            if (AIR_E_OK == rc)
            {
                osal_printf("unit: %d\n", unit);
                osal_printf("lag id: %d\n", lag_id);
                osal_printf("state: %s\n", (state) ? "enable" : "disable");
                osal_printf("member cnt: %d\n", member_cnt);
                rc = _lag_cmd_showLagMember(unit, ptr_member, member_cnt);
                osal_printf("\n");
            }
            else
            {
                osal_printf("***Error***, get lag member fail\n");
            }
        }
        else
        {
            osal_printf("***Error***, out of memory\n");
            rc = AIR_E_NO_MEMORY;
        }
    }
    else
    {
        osal_printf("***Error***, get lag fail.\n");
    }

    if (NULL != ptr_member)
    {
        osal_free(ptr_member);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_setHashCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0;
    AIR_LAG_HASH_CTRL_T hashControl;
    C8_T                type_str[DSH_CMD_MAX_LENGTH] = {0};

    /* cmd: lag set hash-ctrl [ unit=<UINT> ] type={ crc32lsb | crc32msb| crc16 | xor4 }
        [ sp ] [ sa ] [ da ] [ sip ] [ dip ] [ sport ] [ dport ]
     */

    osal_memset(&hashControl, 0, sizeof(AIR_LAG_HASH_CTRL_T));

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(type_str, "crc32lsb"))
    {
        hashControl.hash_algo = AIR_LAG_HASH_ALGO_CRC32_LSB;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "crc32msb"))
    {
        hashControl.hash_algo = AIR_LAG_HASH_ALGO_CRC32_MSB;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "crc16"))
    {
        hashControl.hash_algo = AIR_LAG_HASH_ALGO_CRC16;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "xor4"))
    {
        hashControl.hash_algo = AIR_LAG_HASH_ALGO_XOR4;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sp"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sa"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "da"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sip"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dip"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sport"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT;
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "dport"))
    {
        hashControl.flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT;
        token_idx += 1;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_lag_setHashControl(unit, &hashControl);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set lag hash control fail\n");
    }
    return (rc);
}

static AIR_ERROR_NO_T
_lag_cmd_showHashControl(
    const UI32_T              unit,
    const AIR_LAG_HASH_CTRL_T hashControl)
{
    UI32_T hashType;

    hashType = hashControl.hash_algo;
    osal_printf("lag hash type : ");
    switch (hashType)
    {
        case AIR_LAG_HASH_ALGO_CRC32_LSB:
            osal_printf("crc32lsb \n");
            break;
        case AIR_LAG_HASH_ALGO_CRC32_MSB:
            osal_printf("crc32msb \n");
            break;
            break;
        case AIR_LAG_HASH_ALGO_CRC16:
            osal_printf("crc16 \n");
            break;
            break;
        case AIR_LAG_HASH_ALGO_XOR4:
            osal_printf("xor4 \n");
            break;
            break;
        default:
            /* do nothing */
            break;
    }

    /* hash field */
    osal_printf("lag hash field : \n");
    osal_printf(" - sp:    %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT) ? "enable" : "disable");
    osal_printf(" - sa:    %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC) ? "enable" : "disable");
    osal_printf(" - da:    %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC) ? "enable" : "disable");
    osal_printf(" - sip:   %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP) ? "enable" : "disable");
    osal_printf(" - dip:   %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP) ? "enable" : "disable");
    osal_printf(" - sport: %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT) ? "enable" : "disable");
    osal_printf(" - dport: %s \n", (hashControl.flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT) ? "enable" : "disable");

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_lag_cmd_getHashCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0;
    AIR_LAG_HASH_CTRL_T hashControl;

    /* cmd: lag show hash-ctrl [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&hashControl, 0, sizeof(AIR_LAG_HASH_CTRL_T));
    rc = air_lag_getHashControl(unit, &hashControl);

    if (AIR_E_OK == rc)
    {
        osal_printf("unit: %d\n", unit);
        rc = _lag_cmd_showHashControl(unit, hashControl);
    }
    else
    {
        osal_printf("***Error***, get lag hash control fail\n");
    }
    return (rc);
}

static AIR_ERROR_NO_T
_lag_showUsage(
    UI32_T unit,
    UI32_T type,
    UI32_T group_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         param = 0, count = 0, size = 0;

    if (AIR_SWC_RSRC_LAG_GROUP == type)
    {
        rc = air_swc_getCapacity(unit, type, param, &size);
        if (AIR_E_OK == rc)
        {
            rc = air_swc_getUsage(unit, type, param, &count);
            if (AIR_E_OK == rc)
            {
                osal_printf("%-25s %10s %6u/%6u\n", "lag-grp", "---", count, size);
            }
        }
    }
    else if (AIR_SWC_RSRC_LAG_GROUP_MEMBER == type)
    {
        rc = air_lag_getMaxGroupCnt(unit, &param, &size);
        if (AIR_E_OK == rc)
        {
            if (AIR_LAG_CMD_MAX_GROUP_NUM == group_id)
            {
                for (group_id = 0; group_id < param; group_id++)
                {
                    rc = air_swc_getUsage(unit, type, group_id, &count);
                    if (AIR_E_OK == rc)
                    {
                        osal_printf("%-25s %10u %6u/%6u\n", "lag-grp-member", group_id, count, size);
                    }
                }
            }
            else
            {
                rc = air_swc_getUsage(unit, type, group_id, &count);
                if (AIR_E_OK == rc)
                {
                    osal_printf("%-25s %10u %6u/%6u\n", "lag-grp-member", group_id, count, size);
                }
            }
        }
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
_lag_cmd_getResource(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, group_id = AIR_LAG_CMD_MAX_GROUP_NUM;
    C8_T           type_str[DSH_CMD_MAX_LENGTH] = {0};

    /* cmd: lag show resource [ unit=<UINT> ] [type={ lag-grp | lag-grp-member [ id=<UINT> ] } ]
     */

    osal_strncpy(type_str, "both", sizeof("both"));
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "id"))
    {
        DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "id", &group_id), token_idx, 2);
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit 0\n");
    osal_printf("%-25s %10s %6s/%6s\n", "type", "id", "used", "total");

    if (AIR_E_OK == dsh_checkString(type_str, "both"))
    {
        rc = _lag_showUsage(unit, AIR_SWC_RSRC_LAG_GROUP, group_id);
        if (AIR_E_OK == rc)
        {
            rc = _lag_showUsage(unit, AIR_SWC_RSRC_LAG_GROUP_MEMBER, group_id);
        }
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "lag-grp"))
    {
        rc = _lag_showUsage(unit, AIR_SWC_RSRC_LAG_GROUP, group_id);
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "lag-grp-member"))
    {
        rc = _lag_showUsage(unit, AIR_SWC_RSRC_LAG_GROUP_MEMBER, group_id);
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show lag resource fail\n");
    }

    return (rc);
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _lag_cmd_vec[] =
{
    {
        "create group", 2, _lag_cmd_create,
        "lag create group [ unit=<UINT> ] lag-id=<UINT>\n"
    },
    {
        "destroy group", 2, _lag_cmd_destroy,
        "lag destroy group [ unit=<UINT> ] lag-id=<UINT>\n"
    },
    {
        "add member", 2, _lag_cmd_addMember,
        "lag add member [ unit=<UINT> ] lag-id=<UINT> portlist=<UINTLIST>\n"
    },
    {
        "del member", 2, _lag_cmd_delMember,
        "lag del member [ unit=<UINT> ] lag-id=<UINT> portlist=<UINTLIST>\n"
    },
    {
        "show group", 2, _lag_cmd_show,
        "lag show group [ unit=<UINT> ] lag-id=<UINT>\n"
    },
    {
        "set hash-ctrl", 2, _lag_cmd_setHashCtrl,
        "lag set hash-ctrl [ unit=<UINT> ] type={ crc32lsb | crc32msb| crc16 | xor4 }\n"
        "[ sp ] [ sa ] [ da ] [ sip ] [ dip ] [ sport ] [ dport ]\n"
    },
    {
        "show hash-ctrl", 2, _lag_cmd_getHashCtrl,
        "lag show hash-ctrl [ unit=<UINT> ]\n"
    },
    {
        "show resource", 2, _lag_cmd_getResource,
        "lag show resource [ unit=<UINT> ]\n"
        "[ type={ lag-grp | lag-grp-member [ id=<UINT> ] } ]\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
lag_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _lag_cmd_vec, sizeof(_lag_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
lag_cmd_usager()
{
    return (dsh_usager(_lag_cmd_vec, sizeof(_lag_cmd_vec) / sizeof(DSH_VEC_T)));
}
