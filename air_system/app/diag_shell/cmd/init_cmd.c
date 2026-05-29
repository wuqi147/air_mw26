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

#include <cmd/init_cmd.h>

#include <air_error.h>
#include <air_init.h>
#include <air_port.h>
#include <air_stp.h>
#include <air_types.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define BASET_PORT_FLAGS_POE (AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE)
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
static AIR_INIT_PORT_MAP_T *_ptr_init_cmd_sdk_port_info = NULL;
static UI32_T               _init_cmd_sdk_port_cnt = 0;

const static C8_T          *ptr_port_type_str[AIR_INIT_PORT_TYPE_LAST] = {"baset", "xsgmii", "cpu", "baset"};

/* LOCAL SUBPROGRAM BODIES
 */

/***********************************
 * Command
 ***********************************/
static AIR_ERROR_NO_T
_init_cmd_startSdk(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /*
     * Command format
     * init start sdk [ unit=<UINT> ] { low-level | module | task-rsrc | task }
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* parse and get */
    /* Should not check unit valid before init low-level */
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "unit", &unit), tokens, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "low-level"))
    {
        token_idx += 1;
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        osal_printf("low-level init...");
        rc = air_init_initLowLevel(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, init low-level fail, please check the aml/osal are ok.\n");
        }
        else
        {
            osal_printf("Success\n");
        }
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "module"))
    {
        token_idx += 1;
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        osal_printf("module init...");
        rc = air_init_initModule(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, init module fail, please check the low-level is inited.\n");
        }
        else
        {
            osal_printf("Success\n");
        }
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "task-rsrc"))
    {
        token_idx += 1;
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        osal_printf("task-rsrc init...");
        rc = air_init_initTaskRsrc(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, init task-rsrc fail, please check the low-level is inited.\n");
        }
        else
        {
            osal_printf("Success\n");
        }
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "task"))
    {
        token_idx += 1;
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        osal_printf("task init...");
        rc = air_init_initTask(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, init task fail, please check the low-level is inited.\n");
        }
        else
        {
            osal_printf("Success\n");
        }
    }
    else
    {
        rc = DSH_E_NOT_FOUND;
    }

    return rc;
}

static AIR_ERROR_NO_T
_init_cmd_addSdkPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    UI32_T               unit = 0;
    AIR_INIT_PORT_MAP_T *ptr_port_entry;

    /* cmd:
     * init add sdk-port [ unit=<UINT> ] port=<UINT> max-speed={ 1000m | 2500m }
     * type={ baset pkg-id=<UINT> |
     *        enhanced-baset pkg-id=<UINT> [poe] |
     *        xsgmii xsgmii-id=<UINT> pkg-id=<UINT> [ ext-cpu ] [ combo-port ] [poe] |
     *        cpu }
     */
    if (_init_cmd_sdk_port_cnt >= AIR_PORT_NUM)
    {
        return AIR_E_OTHERS;
    }

    if (NULL == _ptr_init_cmd_sdk_port_info)
    {
        _ptr_init_cmd_sdk_port_info = (AIR_INIT_PORT_MAP_T *)osal_alloc(sizeof(AIR_INIT_PORT_MAP_T) * AIR_PORT_NUM,
                                                                        air_module_getModuleName(AIR_MODULE_INIT));
        if (NULL == _ptr_init_cmd_sdk_port_info)
        {
            osal_printf("***Error***, allocate memory fail\n");
            return AIR_E_NO_MEMORY;
        }
        osal_memset(_ptr_init_cmd_sdk_port_info, 0x00, sizeof(AIR_INIT_PORT_MAP_T) * AIR_PORT_NUM);
    }
    ptr_port_entry = &(_ptr_init_cmd_sdk_port_info[_init_cmd_sdk_port_cnt]);

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &(ptr_port_entry->port)), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "max-speed"))
    {
        token_idx += 1;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "1000m"))
        {
            ptr_port_entry->max_speed = AIR_INIT_PORT_SPEED_1000M;
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "2500m"))
        {
            ptr_port_entry->max_speed = AIR_INIT_PORT_SPEED_2500M;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR);
        }
        token_idx += 1;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "type"))
    {
        token_idx += 1;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "baset"))
        {
            token_idx += 1;

            /* baset type port */
            ptr_port_entry->port_type = AIR_INIT_PORT_TYPE_BASET;

            /* get package id */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pkg-id", &(ptr_port_entry->baset_port.phy_pkg_id)),
                            token_idx, 2);
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "enhanced-baset"))
        {
            token_idx += 1;

            /* baset type port */
            ptr_port_entry->port_type = AIR_INIT_PORT_TYPE_ENHANCED_BASET;

            /* get package id */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pkg-id", &(ptr_port_entry->enhanced_baset_port.phy_pkg_id)),
                            token_idx, 2);

            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "poe"))
            {
                token_idx += 1;
                ptr_port_entry->enhanced_baset_port.flags |= AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE;
            }
        }

        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "xsgmii"))
        {
            token_idx += 1;

            /* xsgmii type port */
            ptr_port_entry->port_type = AIR_INIT_PORT_TYPE_XSGMII;

            /* get xsgmii id */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "xsgmii-id", &(ptr_port_entry->xsgmii_port.xsgmii_pkg_id)),
                            token_idx, 2);

            /* get package id */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pkg-id", &(ptr_port_entry->xsgmii_port.phy_pkg_id)),
                            token_idx, 2);

            /* check optional external CPU setting */
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "ext-cpu"))
            {
                token_idx += 1;
                ptr_port_entry->xsgmii_port.flags |= AIR_INIT_XSGMII_PORT_FLAGS_CPU;
            }
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "combo-port"))
            {
                token_idx += 1;
                ptr_port_entry->xsgmii_port.flags |= AIR_INIT_XSGMII_PORT_FLAGS_COMBO;
            }
            if (AIR_E_OK == dsh_checkString(tokens[token_idx], "poe"))
            {
                token_idx += 1;
                ptr_port_entry->xsgmii_port.flags |= AIR_INIT_XSGMII_PORT_FLAGS_POE;
            }
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "cpu"))
        {
            token_idx += 1;

            /* internal cpu port type */
            ptr_port_entry->port_type = AIR_INIT_PORT_TYPE_CPU;
        }
        else
        {
            return (DSH_E_SYNTAX_ERR);
        }
    }
    else
    {
        return (DSH_E_SYNTAX_ERR);
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* update sdk port (air port) counts */
    _init_cmd_sdk_port_cnt++;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_init_cmd_applySdkPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    UI32_T         unit = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* cmd:
     * init apply sdk-port [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (NULL != _ptr_init_cmd_sdk_port_info)
    {
        rc = air_init_initSdkPortMap(unit, _init_cmd_sdk_port_cnt, &(_ptr_init_cmd_sdk_port_info[0]));
        /* reset _init_cmd_sdk_port_cnt to be 0 */
        _init_cmd_sdk_port_cnt = 0;
        osal_free(_ptr_init_cmd_sdk_port_info);
        _ptr_init_cmd_sdk_port_info = NULL;
    }
    else
    {
        rc = AIR_E_OP_INVALID;
        osal_printf("***Error***, sdk-port map is null, port map cnt=%u, rc=%u\n", _init_cmd_sdk_port_cnt, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_init_cmd_showSdkPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    UI32_T               unit = 0, count = 0, idx = 0;
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    AIR_INIT_PORT_MAP_T *ptr_port_map = NULL;

    /* cmd:
     * init show sdk-port [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* allocate resource */
    ptr_port_map = (AIR_INIT_PORT_MAP_T *)osal_alloc(sizeof(AIR_INIT_PORT_MAP_T) * AIR_PORT_NUM,
                                                     air_module_getModuleName(AIR_MODULE_INIT));
    if (NULL == ptr_port_map)
    {
        osal_printf("***Error***, allocate memory fail\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_port_map, 0x00, sizeof(AIR_INIT_PORT_MAP_T) * AIR_PORT_NUM);

    /* get port map info */
    rc = air_init_getSdkPortMap(unit, &count, ptr_port_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get sdk-port failed \n");
    }
    else
    {
        osal_printf("sdk port map info :\n");
        osal_printf("total port count : %d \n", count);
        osal_printf("%10s %7s %12s %12s %10s %10s %9s %9s\n", "unit/port", "type", "max-speed", "xsgmii-id", "gphy-id",
                    "ext-cpu", "combo", "poe");

        for (idx = 0; idx < count; idx++)
        {
            osal_printf("%4d/%3d", unit, ptr_port_map[idx].port);
            osal_printf("%10s", ptr_port_type_str[ptr_port_map[idx].port_type]);
            osal_printf("%11d", ptr_port_map[idx].max_speed);

            if (ptr_port_map[idx].port_type == AIR_INIT_PORT_TYPE_XSGMII)
            {
                /* xsgmii port */
                osal_printf("%12d", ptr_port_map[idx].xsgmii_port.xsgmii_pkg_id);
                osal_printf("%12d", ptr_port_map[idx].xsgmii_port.phy_pkg_id);
                osal_printf("%11s",
                            ((ptr_port_map[idx].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_CPU) ? "yes" : "no"));
                osal_printf("%11s",
                            ((ptr_port_map[idx].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_COMBO) ? "yes" : "no"));
                osal_printf("%11s",
                            ((ptr_port_map[idx].xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_POE) ? "yes" : "no"));
            }
            else if (ptr_port_map[idx].port_type == AIR_INIT_PORT_TYPE_BASET)
            {
                /* gphy port */
                osal_printf("%12s", "---");
                osal_printf("%12d", ptr_port_map[idx].baset_port.phy_pkg_id);
                osal_printf("%11s", "---");
                osal_printf("%11s", "---");
                osal_printf("%11s", "---");
            }
            else if (ptr_port_map[idx].port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET)
            {
                /* gphy port */
                osal_printf("%12s", "---");
                osal_printf("%12d", ptr_port_map[idx].enhanced_baset_port.phy_pkg_id);
                osal_printf("%11s", "---");
                osal_printf("%11s", "---");
                osal_printf("%11s",
                            ((ptr_port_map[idx].enhanced_baset_port.flags & BASET_PORT_FLAGS_POE) ? "yes" : "no"));
            }

            else if (ptr_port_map[idx].port_type == AIR_INIT_PORT_TYPE_CPU)
            {
                /* cpu port */
                osal_printf("%12s", "---");
                osal_printf("%12s", "---");
                osal_printf("%11s", "---");
                osal_printf("%11s", "---");
                osal_printf("%11s", "---");
            }
            osal_printf("\n");
        }
    }
    if (NULL != ptr_port_map)
    {
        osal_free(ptr_port_map);
    }

    return rc;
}

/* clang-format off */
static const DSH_VEC_T  _init_cmd_vec[] =
{
    {
        "start sdk", 2, _init_cmd_startSdk,
        "init start sdk [ unit=<UINT> ] { low-level | module | task-rsrc | task } \n"
    },
    {
        "add sdk-port", 2, _init_cmd_addSdkPort,
        "init add sdk-port [ unit=<UINT> ] port=<UINT> max-speed={ 1000m | 2500m } \n"
        "type={ baset pkg-id=<UINT> | \n"
        "       enhanced-baset pkg-id=<UINT> [poe] | \n"
        "       xsgmii xsgmii-id=<UINT> pkg-id=<UINT> [ ext-cpu ] [ combo-port ] [poe] | \n"
        "       cpu } \n"
    },
    {
        "apply sdk-port", 2, _init_cmd_applySdkPort,
        "init apply sdk-port [ unit=<UINT> ] \n"
    },
    {
        "show sdk-port", 2, _init_cmd_showSdkPort,
        "init show sdk-port [ unit=<UINT> ] \n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
init_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _init_cmd_vec, sizeof(_init_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
init_cmd_usager()
{
    return (dsh_usager(_init_cmd_vec, sizeof(_init_cmd_vec) / sizeof(DSH_VEC_T)));
}
