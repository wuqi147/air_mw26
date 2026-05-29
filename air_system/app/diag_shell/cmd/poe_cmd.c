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
#include <cmd/poe_cmd.h>

#include <air_poe.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>
/* NAMING CONSTANT DECLARATIONS
 */
#define POE_CMD_REG_PAGE_LEN (256)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
static AIR_INIT_POE_DEVICE_MAP_T *_ptr_init_poe_device_info = NULL;
static UI32_T                     _init_poe_device_cnt = 0;
static AIR_INIT_POE_PORT_MAP_T   *_ptr_init_poe_port_info = NULL;
static UI32_T                     _init_poe_port_cnt = 0;

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_poe_cmd_addPoeDevice(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_entry;

    /* cmd:
     * poe add poe-device [ unit=<UINT> ] device=<UINT> bus-id=<UINT> address=<HEX>
     */

    if (_init_poe_device_cnt >= AIR_POE_MAXIMUM_PORTS_PER_DEVICE)
    {
        return AIR_E_OTHERS;
    }
    if (NULL == _ptr_init_poe_device_info)
    {
        _ptr_init_poe_device_info = (AIR_INIT_POE_DEVICE_MAP_T *)osal_alloc(
            sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_POE_MAXIMUM_PORTS_PER_DEVICE,
            air_module_getModuleName(AIR_MODULE_POE));
        if (NULL == _ptr_init_poe_device_info)
        {
            return AIR_E_NO_MEMORY;
        }
        osal_memset(_ptr_init_poe_device_info, 0x00,
                    sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_POE_MAXIMUM_PORTS_PER_DEVICE);
    }
    ptr_device_entry = &(_ptr_init_poe_device_info[_init_poe_device_cnt]);

    DSH_CHECK_GET_UNIT(tokens, token_idx, &(ptr_device_entry->unit), 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &(ptr_device_entry->poe_device_idx)), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "bus-id", &(ptr_device_entry->i2c_bus_id)), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "address", &(ptr_device_entry->i2c_address), sizeof(UI32_T)),
                    token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* update poe device counts */
    _init_poe_device_cnt++;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_poe_cmd_applyPoeDevice(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    UI32_T         unit = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* cmd:
     * poe apply poe-device [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (NULL != _ptr_init_poe_device_info)
    {
        rc = air_init_initPoeDeviceMap(unit, _init_poe_device_cnt, &(_ptr_init_poe_device_info[0]));

        _init_poe_device_cnt = 0;
        osal_free(_ptr_init_poe_device_info);
        _ptr_init_poe_device_info = NULL;
    }
    else
    {
        rc = AIR_E_OP_INVALID;
        osal_printf("***Error***, poe-device map is null, device cnt=%u, rc=%u\n", _init_poe_device_cnt, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_addPoePort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_INIT_POE_PORT_MAP_T *ptr_port_entry;

    /* cmd:
     * poe add poe-port [ unit=<UINT> ] port=<UINT> type={ af | at }
     *     primary device=<UINT> poe-port=<UINT>
     *     [ secondary device=<UINT> poe-port=<UINT> ]
     */

    if (_init_poe_port_cnt >= AIR_PORT_NUM)
    {
        return AIR_E_OTHERS;
    }

    if (NULL == _ptr_init_poe_port_info)
    {
        _ptr_init_poe_port_info = (AIR_INIT_POE_PORT_MAP_T *)osal_alloc(sizeof(AIR_INIT_POE_PORT_MAP_T) * AIR_PORT_NUM,
                                                                        air_module_getModuleName(AIR_MODULE_POE));
        if (NULL == _ptr_init_poe_port_info)
        {
            return AIR_E_NO_MEMORY;
        }
        osal_memset(_ptr_init_poe_port_info, 0x00, sizeof(AIR_INIT_POE_PORT_MAP_T) * AIR_PORT_NUM);
    }
    ptr_port_entry = &(_ptr_init_poe_port_info[_init_poe_port_cnt]);

    DSH_CHECK_GET_UNIT(tokens, token_idx, &(ptr_port_entry->unit), 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &(ptr_port_entry->port)), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "type"))
    {
        token_idx += 1;

        if (AIR_E_OK == dsh_checkString(tokens[token_idx], "af"))
        {
            ptr_port_entry->port_type = AIR_INIT_POE_PORT_TYPE_AF_AT;
        }
        else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "at"))
        {
            ptr_port_entry->port_type = AIR_INIT_POE_PORT_TYPE_AF_AT;
        }
        else
        {
            return DSH_E_SYNTAX_ERR;
        }
        token_idx += 1;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "primary"))
    {
        token_idx += 1;

        /* get device index */
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &(ptr_port_entry->primary_port.poe_device_idx)),
                        token_idx, 2);

        /* get hw package id */
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "poe-port", &(ptr_port_entry->primary_port.hw_package_id)),
                        token_idx, 2);
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "secondary"))
    {
        token_idx += 1;

        if (AIR_INIT_POE_PORT_TYPE_BT == ptr_port_entry->port_type)
        {
            /* get device index */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &(ptr_port_entry->secondary_port.poe_device_idx)),
                            token_idx, 2);

            /* get hw package id */
            DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "poe-port", &(ptr_port_entry->secondary_port.hw_package_id)),
                            token_idx, 2);
        }
        else
        {
            osal_printf("***Error***, cannot assign secondary in af/at type\n");
            return DSH_E_SYNTAX_ERR;
        }
    }
    else
    {
        if (AIR_INIT_POE_PORT_TYPE_BT == ptr_port_entry->port_type)
        {
            osal_printf("***Error***, need to assign secondary in bt type\n");
            return DSH_E_SYNTAX_ERR;
        }
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* update poe port counts */
    _init_poe_port_cnt++;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_poe_cmd_applyPoePort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    UI32_T         unit = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* cmd:
     * poe apply poe-port [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (NULL != _ptr_init_poe_port_info)
    {
        rc = air_init_initPoePortMap(unit, _init_poe_port_cnt, &(_ptr_init_poe_port_info[0]));

        _init_poe_port_cnt = 0;
        osal_free(_ptr_init_poe_port_info);
        _ptr_init_poe_port_info = NULL;
    }
    else
    {
        rc = AIR_E_OP_INVALID;
        osal_printf("***Error***, poe-port map is null, port cnt=%u, rc=%u\n", _init_poe_port_cnt, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortInfo(
    const UI32_T unit)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   count = 0, idx = 0;
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map = NULL;
    HAL_POE_PAIR_T           pair = HAL_POE_PAIR_LAST;

    /* display poe port bitmap information */
    /* allocate resource */
    ptr_port_map = (AIR_INIT_POE_PORT_MAP_T *)(osal_alloc(sizeof(AIR_INIT_POE_PORT_MAP_T) * AIR_PORT_NUM,
                                                          air_module_getModuleName(AIR_MODULE_POE)));
    if (NULL == ptr_port_map)
    {
        osal_printf("***Error***, allocate memory failed\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_port_map, 0x00, sizeof(AIR_INIT_POE_PORT_MAP_T) * AIR_PORT_NUM);

    /* get port map info */
    rc = air_init_getPoePortMap(unit, &count, ptr_port_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get poe port map failed\n");
    }
    else
    {
        osal_printf("poe port map info    :\n");

        /* display sdk poe port bitmap information */
        osal_printf(" air port bitmap poe : ");
        for (idx = 0; idx < AIR_PORT_BITMAP_SIZE; idx++)
        {
            osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_poe[(AIR_PORT_BITMAP_SIZE - (1 + idx))]);
        }
        osal_printf("\n");

        osal_printf(" total port count    : %d \n", count);
        osal_printf("%10s %7s %15s %17s %12s\n", "unit/port", "type", "primary poe/id", "secondary poe/id",
                    "alternative");

        for (idx = 0; idx < count; idx++)
        {
            osal_printf("%5d/%3d", ptr_port_map[idx].unit, ptr_port_map[idx].port);
            osal_printf("%9s", ((ptr_port_map[idx].port_type == AIR_INIT_POE_PORT_TYPE_AF_AT) ? "af/at" : "bt"));

            if (AIR_INIT_POE_PORT_TYPE_AF_AT == ptr_port_map[idx].port_type)
            {
                osal_printf("%13d/%2d", ptr_port_map[idx].primary_port.poe_device_idx,
                            ptr_port_map[idx].primary_port.hw_package_id);
                osal_printf("%18s", "---");
            }
            else if (AIR_INIT_POE_PORT_TYPE_BT == ptr_port_map[idx].port_type)
            {
                osal_printf("%13d/%2d", ptr_port_map[idx].primary_port.poe_device_idx,
                            ptr_port_map[idx].primary_port.hw_package_id);
                osal_printf("%15d/%2d", ptr_port_map[idx].secondary_port.poe_device_idx,
                            ptr_port_map[idx].secondary_port.hw_package_id);
            }
            else
            {
                osal_printf("%16s", "---");
                osal_printf("%18s", "---");
            }

            /* alternative mode */
            rc = HAL_POE_FUNC_CALL(unit, ptr_port_map[idx].primary_port.poe_device_idx, getPortPairMode,
                                   (ptr_port_map[idx].unit, ptr_port_map[idx].port, &pair));
            if (AIR_E_OK != rc)
            {
                osal_printf("%13s", "invalid");
            }
            else
            {
                osal_printf("%13s", (HAL_POE_PAIR_A == pair) ? "pair-a" : "pair-b");
            }
            osal_printf("\n");
        }
    }
    if (NULL != ptr_port_map)
    {
        osal_free(ptr_port_map);
    }
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getDeviceInfo(
    const UI32_T unit)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     count = 0, idx = 0;
    UI16_T                     value = 0;
    C8_T                       buf[8] = {0};
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map = NULL;
    HAL_POE_OPERATING_MODE_T   mode = HAL_POE_OPERATING_MODE_LAST;

    /* display poe device map information */
    /* allocate resource */
    ptr_device_map = (AIR_INIT_POE_DEVICE_MAP_T *)(osal_alloc(
        sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP,
        air_module_getModuleName(AIR_MODULE_POE)));
    if (NULL == ptr_device_map)
    {
        osal_printf("***Error***, allocate memory failed\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_device_map, 0x00, sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP);

    /* get device map info */
    rc = air_init_getPoeDeviceMap(unit, &count, ptr_device_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get poe device map failed\n");
    }
    else
    {
        osal_printf("poe device map info :\n");
        osal_printf(" total device count : %d \n", count);
        osal_printf("%5s %6s %10s %7s %11s %8s\n", "unit", "index", "device-id", "bus-id", "slave-addr", "op-mode");

        for (idx = 0; idx < count; idx++)
        {
            osal_printf("%5d", ptr_device_map[idx].unit);
            osal_printf("%7d", ptr_device_map[idx].poe_device_idx);
            /* hardware revision id */
            rc = air_poe_getDeviceInfo(unit, ptr_device_map[idx].poe_device_idx, &value);
            if (AIR_E_OK != rc)
            {
                osal_printf("%11s", "invalid");
            }
            else
            {
                switch (value)
                {
                    case HAL_POE_DEVICE_ID_AN8502:
                        osal_snprintf(buf, sizeof(buf), "%s", "8502");
                        break;
                    case HAL_POE_DEVICE_ID_IP804AR:
                        osal_snprintf(buf, sizeof(buf), "%s", "804ar");
                        break;
                    case HAL_POE_DEVICE_ID_IP808AR:
                        osal_snprintf(buf, sizeof(buf), "%s", "808ar");
                        break;
                    case HAL_POE_DEVICE_ID_AN8503:
                        osal_snprintf(buf, sizeof(buf), "%s", "8503");
                        break;
                    default:
                        osal_snprintf(buf, sizeof(buf), "%s", "invalid");
                }
                osal_printf("%11s", buf);
            }
            osal_printf("%8d", ptr_device_map[idx].i2c_bus_id);
            osal_printf("%12x", ptr_device_map[idx].i2c_address);
            /* operation mode */
            rc = HAL_POE_FUNC_CALL(unit, ptr_device_map[idx].poe_device_idx, getDeviceOperatingMode,
                                   (ptr_device_map[idx].unit, ptr_device_map[idx].poe_device_idx, &mode));
            if (AIR_E_OK != rc)
            {
                osal_printf("%9s", "invalid");
            }
            else
            {
                osal_printf("%9s", (HAL_POE_OPERATING_MODE_AUTO == mode) ? "auto" : "manual");
            }
            osal_printf("\n");
        }
    }
    if (NULL != ptr_device_map)
    {
        osal_free(ptr_device_map);
    }
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getInfo(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /*
     * Command format
     * poe show info [ unit=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc |= _poe_cmd_getDeviceInfo(unit);
    rc |= _poe_cmd_getPortInfo(unit);

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setDevicePowerUpMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0, device = 0;
    C8_T                    str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_POE_POWER_UP_MODE_T mode = AIR_POE_POWER_UP_MODE_LAST;

    /*
     * Command format
     * poe set power-up [ unit=<UINT> ] device=<UINT> mode={ normal | manual }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "normal"))
    {
        mode = AIR_POE_POWER_UP_MODE_NORMAL;
    }
    else if (AIR_E_OK == dsh_checkString(str, "manual"))
    {
        mode = AIR_POE_POWER_UP_MODE_MANUAL;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_poe_setDevicePowerUpMode(unit, device, mode);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set poe power-up mode failed(%d)\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getDevicePowerUpMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  unit = 0, device = AIR_INVALID_ID;
    AIR_POE_POWER_UP_MODE_T mode = AIR_POE_POWER_UP_MODE_LAST;

    /*
     * Command format
     * poe show power-up [ unit=<UINT> ] [ device=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "device"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%12s %6s\n", "unit/device", "mode");
    if (AIR_INVALID_ID == device)
    {
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            osal_printf("%5d/%2d", unit, device);
            rc = air_poe_getDevicePowerUpMode(unit, device, &mode);
            if (AIR_E_OK == rc)
            {
                osal_printf("%13s", (AIR_POE_POWER_UP_MODE_NORMAL == mode) ? "normal" : "manual");
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%10s", "---");
            }
            else
            {
                osal_printf("***Error***, get device=%u power-up failed(%d)\n", device, rc);
                break;
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("%5d/%2d", unit, device);
        rc = air_poe_getDevicePowerUpMode(unit, device, &mode);
        if (AIR_E_OK == rc)
        {
            osal_printf("%13s", (AIR_POE_POWER_UP_MODE_NORMAL == mode) ? "normal" : "manual");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%10s", "---");
        }
        else
        {
            osal_printf("***Error***, get device=%u power-up failed(%d)\n", device, rc);
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setDeviceStrategy(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 unit = 0, device = 0;
    C8_T                   str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_POE_PWR_STRATEGY_T strategy = AIR_POE_PWR_STRATEGY_LAST;

    /*
     * Command format
     * poe set strategy [ unit=<UINT> ] device=<UINT> strategy={ plug | priority }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "strategy", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "plug"))
    {
        strategy = AIR_POE_PWR_STRATEGY_PLUG;
    }
    else if (AIR_E_OK == dsh_checkString(str, "priority"))
    {
        strategy = AIR_POE_PWR_STRATEGY_PRIORITY;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_poe_setDevicePowerStrategy(unit, device, strategy);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set poe power strategy failed(%d)\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getDeviceStrategy(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 unit = 0, device = AIR_INVALID_ID;
    AIR_POE_PWR_STRATEGY_T strategy = AIR_POE_PWR_STRATEGY_LAST;

    /*
     * Command format
     * poe show strategy [ unit=<UINT> ] [ device=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "device"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%12s %9s\n", "unit/device", "strategy");
    if (AIR_INVALID_ID == device)
    {
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            osal_printf("%5d/%2d", unit, device);
            rc = air_poe_getDevicePowerStrategy(unit, device, &strategy);
            if (AIR_E_OK == rc)
            {
                osal_printf("%14s", (AIR_POE_PWR_STRATEGY_PLUG == strategy) ? "plug" : "priority");
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%14s", "---");
            }
            else
            {
                osal_printf("***Error***, get device=%u strategy failed(%d)\n", device, rc);
                break;
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("%5d/%2d", unit, device);
        rc = air_poe_getDevicePowerStrategy(unit, device, &strategy);
        if (AIR_E_OK == rc)
        {
            osal_printf("%14s", (AIR_POE_PWR_STRATEGY_PLUG == strategy) ? "plug" : "priority");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%14s", "---");
        }
        else
        {
            osal_printf("***Error***, get device=%u strategy failed(%d)\n", device, rc);
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setDevicePowerLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, device = 0, limit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         enable = FALSE;

    /*
     * Command format
     * poe set power-limit [ unit=<UINT> ] device=<UINT> mode={ disable | enable } limit=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        enable = FALSE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        enable = TRUE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "limit", &limit), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_poe_setDevicePowerLimit(unit, device, enable, limit);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set poe current limit failed(%d)\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getDevicePowerLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, device = AIR_INVALID_ID, limit = 0;
    BOOL_T         enable = FALSE;

    /*
     * Command format
     * poe show power-limit [ unit=<UINT> ] [ device=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "device"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%12s %7s %17s\n", "unit/device", "mode", "power-limit(W)");
    if (AIR_INVALID_ID == device)
    {
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            osal_printf("%5d/%2d", unit, device);
            rc = air_poe_getDevicePowerLimit(unit, device, &enable, &limit);
            if (AIR_E_OK == rc)
            {
                osal_printf("%12s", (TRUE == enable) ? "enable" : "disable");
                osal_printf("%9d.%02d", limit / 100, limit % 100);
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%12s%11s", "---", "---");
            }
            else
            {
                osal_printf("***Error***, get device=%u power limit failed(%d)\n", device, rc);
                break;
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("%5d/%2d", unit, device);
        rc = air_poe_getDevicePowerLimit(unit, device, &enable, &limit);
        if (AIR_E_OK == rc)
        {
            osal_printf("%12s", (TRUE == enable) ? "enable" : "disable");
            osal_printf("%9d.%02d", limit / 100, limit % 100);
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%12s%11s", "---", "---");
        }
        else
        {
            osal_printf("***Error***, get device=%u power limit failed(%d)\n", device, rc);
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setDeviceScratch(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, device = 0;
    UI32_T         scratch = 0;

    /*
     * Command format
     * poe set scratch [ unit=<UINT> ] device=<UINT> scratch=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "scratch", &scratch), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_poe_setDeviceScratch(unit, device, scratch);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set poe scratch failed(%d)\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getDeviceScratch(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, device = AIR_INVALID_ID;
    UI32_T         scratch = 0;

    /*
     * Command format
     * poe show scratch [ unit=<UINT> ] [ device=<UINT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "device"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%12s %8s\n", "unit/device", "scratch");
    if (AIR_INVALID_ID == device)
    {
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            osal_printf("%5d/%2d", unit, device);
            rc = air_poe_getDeviceScratch(unit, device, &scratch);
            if (AIR_E_OK == rc)
            {
                osal_printf("%11d", scratch);
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%11s", "---");
            }
            else
            {
                osal_printf("***Error***, get device=%u scratch failed(%d)\n", device, rc);
                break;
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("%5d/%2d", unit, device);
        rc = air_poe_getDeviceScratch(unit, device, &scratch);
        if (AIR_E_OK == rc)
        {
            osal_printf("%11d", scratch);
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%11s", "---");
        }
        else
        {
            osal_printf("***Error***, get device=%u scratch failed(%d)\n", device, rc);
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_triggerPortPowerUp(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe trigger manual-power-up [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_triggerPortPowerUp(unit, port, alt);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, trigger port=%u power-up failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortPowerUp(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    BOOL_T                ready = FALSE;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show manual-power-up [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %15s\n", "unit/port", "power-up-state");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortPowerUp(unit, port, alt, &ready);
        if (AIR_E_OK == rc)
        {
            if (TRUE == ready)
            {
                osal_printf("%13s", "ready");
            }
            else
            {
                osal_printf("%15s", "not-ready");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%12s", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u power-up failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setPortType(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    C8_T                  str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_POE_TYPE_T        type = AIR_POE_TYPE_LAST;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe set type [ unit=<UINT> ] portlist=<UINTLIST> type={ af | at }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "af"))
    {
        type = AIR_POE_TYPE_AF;
    }
    else if (AIR_E_OK == dsh_checkString(str, "at"))
    {
        type = AIR_POE_TYPE_AT;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_setPortType(unit, port, alt, type);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u type failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortType(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_TYPE_T        type = AIR_POE_TYPE_LAST;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show type [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %6s\n", "unit/port", "type");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortType(unit, port, alt, &type);
        if (AIR_E_OK == rc)
        {
            osal_printf("%8s", (AIR_POE_TYPE_AF == type) ? "af" : "at");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%8s", "--");
        }
        else
        {
            osal_printf("***Error***, get port=%u type failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setPortPse(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    C8_T                  str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_POE_PSE_MODE_T    mode = AIR_POE_PSE_MODE_LAST;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe set pse [ unit=<UINT> ] portlist=<UINTLIST> mode={ disable | enable | force-power }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        mode = AIR_POE_PSE_MODE_ENABLE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        mode = AIR_POE_PSE_MODE_DISABLE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "force-power"))
    {
        mode = AIR_POE_PSE_MODE_FORCE_POWER;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_setPortPse(unit, port, alt, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u pse failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortPse(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_PSE_MODE_T    mode = AIR_POE_PSE_MODE_LAST;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show pse [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %10s\n", "unit/port", "pse-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortPse(unit, port, alt, &mode);
        if (AIR_E_OK == rc)
        {
            if (AIR_POE_PSE_MODE_ENABLE == mode)
            {
                osal_printf("%11s", "enable");
            }
            else if (AIR_POE_PSE_MODE_FORCE_POWER == mode)
            {
                osal_printf("%16s", "force-power");
            }
            else
            {
                osal_printf("%12s", "disable");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%10s", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u mode failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getMeasurement(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0, port = 0, device = AIR_INVALID_ID;
    AIR_PORT_BITMAP_T            pbm = {0};
    BOOL_T                       get_by_port = FALSE;
    AIR_POE_PORT_MEASUREMENT_T   port_meas = {0};
    AIR_POE_DEVICE_MEASUREMENT_T device_meas = {0};
    AIR_POE_ALTERNATIVE_T        alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show measurement [ unit=<UINT> ] { [ device=<UINT> ] | portlist=<UINTLIST> }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "device"))
    {
        get_by_port = FALSE;
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "portlist"))
    {
        get_by_port = TRUE;
        DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    }
    else
    {
        get_by_port = FALSE;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (TRUE == get_by_port)
    {
        osal_printf("%10s %14s %11s %15s\n", "unit/port", "current(mA)", "voltage(V)", "temperature(C)");
        AIR_PORT_FOREACH(pbm, port)
        {
            osal_printf("%5d/%2d", unit, port);
            rc = air_poe_getPortMeasurement(unit, port, alt, &port_meas);
            if (AIR_E_OK == rc)
            {
                osal_printf("%12d.%02d", port_meas.current / 100, port_meas.current % 100);
                osal_printf("%10d.%02d", port_meas.voltage / 100, port_meas.voltage % 100);
                osal_printf("%13d.%02d", port_meas.temperature / 100, port_meas.temperature % 100);
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%15s%13s%16s", "---", "---", "---");
            }
            else
            {
                osal_printf("***Error***, get port=%u measurement failed(%d)\n", port, rc);
                break;
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("%12s %12s %11s %9s\n", "unit/device", "current(mA)", "voltage(V)", "power(W)");
        if (AIR_INVALID_ID == device)
        {
            for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
            {
                osal_printf("%5d/%2d", unit, device);
                rc = air_poe_getDeviceMeasurement(unit, device, &device_meas);
                if (AIR_E_OK == rc)
                {
                    osal_printf("%12d.%02d", device_meas.total_current / 100, device_meas.total_current % 100);
                    osal_printf("%10d.%02d", device_meas.supply_voltage / 100, device_meas.supply_voltage % 100);
                    osal_printf("%7d.%02d", device_meas.consume_power / 100, device_meas.consume_power % 100);
                }
                else if (AIR_E_NOT_SUPPORT == rc)
                {
                    osal_printf("%15s%13s%10s", "---", "---", "---");
                }
                else
                {
                    osal_printf("***Error***, get device=%u measurement failed(%d)\n", device, rc);
                    break;
                }
                osal_printf("\n");
            }
        }
        else
        {
            osal_printf("%5d/%2d", unit, device);
            rc = air_poe_getDeviceMeasurement(unit, device, &device_meas);
            if (AIR_E_OK == rc)
            {
                osal_printf("%12d.%02d", device_meas.total_current / 100, device_meas.total_current % 100);
                osal_printf("%10d.%02d", device_meas.supply_voltage / 100, device_meas.supply_voltage % 100);
                osal_printf("%7d.%02d", device_meas.consume_power / 100, device_meas.consume_power % 100);
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("%15s%13s%10s", "---", "---", "---");
            }
            else
            {
                osal_printf("***Error***, get device=%u measurement failed(%d)\n", device, rc);
            }
            osal_printf("\n");
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortStatus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show status [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %12s %11s %15s\n", "unit/port", "signature", "pd-class", "power-status");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortStatus(unit, port, alt, &status);
        if (AIR_E_OK == rc)
        {
            switch (status.pd_signature)
            {
                case AIR_POE_SIGNATURE_BAD:
                    osal_printf("%13s", "bad");
                    break;
                case AIR_POE_SIGNATURE_GOOD:
                    osal_printf("%13s", "good");
                    break;
                case AIR_POE_SIGNATURE_OPEN:
                    osal_printf("%13s", "open");
                    break;
                case AIR_POE_SIGNATURE_LARGE:
                    osal_printf("%13s", "large");
                    break;
                case AIR_POE_SIGNATURE_LOW:
                    osal_printf("%13s", "low");
                    break;
                case AIR_POE_SIGNATURE_HIGH:
                    osal_printf("%13s", "high");
                    break;
                default:
                    osal_printf("%13s", "unknown");
                    break;
            }

            switch (status.pd_class)
            {
                case AIR_POE_CLASS_0:
                    osal_printf("%14s", "class 0");
                    break;
                case AIR_POE_CLASS_1:
                    osal_printf("%14s", "class 1");
                    break;
                case AIR_POE_CLASS_2:
                    osal_printf("%14s", "class 2");
                    break;
                case AIR_POE_CLASS_3:
                    osal_printf("%14s", "class 3");
                    break;
                case AIR_POE_CLASS_4:
                    osal_printf("%14s", "class 4");
                    break;
                default:
                    osal_printf("%14s", "unknown");
                    break;
            }

            osal_printf("%11s", (AIR_POE_POWER_STATUS_OFF == status.power_status) ? "off" : "on");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%13s%12s%13s", "---", "---", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u status failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setPortCurrentLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, limit = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /*
     * Command format
     * poe set current-limit [ unit=<UINT> ] portlist=<UINTLIST> limit=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "limit", &limit), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_setPortCurrentLimit(unit, port, limit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u current-limit failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortCurrentLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, limit = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /*
     * Command format
     * poe show current-limit [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %19s\n", "unit/port", "current-limit(mA)");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortCurrentLimit(unit, port, &limit);
        if (AIR_E_OK == rc)
        {
            /* limit gain factor is 0.01mA */
            osal_printf("%13d.%02d", limit / 100, limit % 100);
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%16s", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u current-limit failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_setPortPriority(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0, priority = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe set priority [ unit=<UINT> ] portlist=<UINTLIST> priority=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "priority", &priority), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_setPortPriority(unit, port, alt, priority);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u priority failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortPriority(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0, priority = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show priority [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %9s\n", "unit/port", "priority");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortPriority(unit, port, alt, &priority);
        if (AIR_E_OK == rc)
        {
            osal_printf("%12d", priority);
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%12s", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u priority failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_getPortEvent(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_EVENT_T       event = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe show event [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s\n", "unit/port", "event");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_poe_getPortEvent(unit, port, alt, &event);
        if (AIR_E_OK == rc)
        {
            osal_printf("%11x", event.event_flags);
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%11s", "---");
        }
        else
        {
            osal_printf("***Error***, get port=%u status failed(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_clearPortEvent(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    /*
     * Command format
     * poe clear event [ unit=<UINT> ] portlist=<UINTLIST>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_poe_clearPortEvent(unit, port, alt);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear port=%u event failed(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_poe_cmd_dumpReg(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device, unit = 0;
    UI16_T         i = 0, j = 0, idx = 0;
    UI8_T          register_val[POE_CMD_REG_PAGE_LEN] = {0};

    /*
     * Command format
     * poe dump reg [ unit=<UINT> ] device=<UINT>
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "device", &device), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("device 0x%02x\n", (UI8_T)HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, device));
    for (i = HAL_POE_PAGE_0; i <= HAL_POE_PAGE_2; i++)
    {
        rc = hal_poe_readReg(unit, device, i, 0x00, POE_CMD_REG_PAGE_LEN, (UI32_T *)register_val);
        if (rc != AIR_E_OK)
        {
            osal_printf("***Error***, dump device=%u info failed(%d)\n", device, rc);
            return rc;
        }
        osal_printf("page %d:\n", i);
        osal_printf("         ");
        for (idx = 0; idx < 16; idx++)
        {
            osal_printf(" [%02x]", idx);
        }
        osal_printf("\n");
        for (idx = 0; idx < POE_CMD_REG_PAGE_LEN / 16; idx++)
        {
            osal_printf("[0x%02x] = ", idx * 16);
            for (j = 0; j < 16; j++)
            {
                osal_printf("  %02x ", register_val[idx * 16 + j]);
            }
            osal_printf("\n");
        }
        osal_printf("\n");
    }

    return rc;
}

/* clang-format off */
const static DSH_VEC_T _poe_cmd_vec[] =
{
    {
        "add poe-device", 2, _poe_cmd_addPoeDevice,
        "poe add poe-device [ unit=<UINT> ] device=<UINT> bus-id=<UINT> address=<HEX> \n"
    },
    {
        "apply poe-device", 2, _poe_cmd_applyPoeDevice,
        "poe apply poe-device [ unit=<UINT> ] \n"
    },
    {
        "add poe-port", 2, _poe_cmd_addPoePort,
        "poe add poe-port [ unit=<UINT> ] port=<UINT> type={ af | at } \n"
        "primary device=<UINT> poe-port=<UINT> \n"
        "[ secondary device=<UINT> poe-port=<UINT> ] \n"
    },
    {
        "apply poe-port", 2, _poe_cmd_applyPoePort,
        "poe apply poe-port [ unit=<UINT> ] \n"
    },
    {
        "show info", 2, _poe_cmd_getInfo,
        "poe show info [ unit=<UINT> ]\n"
    },
    {
        "set power-up", 2, _poe_cmd_setDevicePowerUpMode,
        "poe set power-up [ unit=<UINT> ] device=<UINT> mode={ normal | manual }\n"
    },
    {
        "show power-up", 2, _poe_cmd_getDevicePowerUpMode,
        "poe show power-up [ unit=<UINT> ] [ device=<UINT> ]\n"
    },
    {
        "set strategy", 2, _poe_cmd_setDeviceStrategy,
        "poe set strategy [ unit=<UINT> ] device=<UINT> strategy={ plug | priority }\n"
    },
    {
        "show strategy", 2, _poe_cmd_getDeviceStrategy,
        "poe show strategy [ unit=<UINT> ] [ device=<UINT> ]\n"
    },
    {
        "set power-limit", 2, _poe_cmd_setDevicePowerLimit,
        "poe set power-limit [ unit=<UINT> ] device=<UINT> mode={ disable | enable }\n"
        "limit=<UINT>\n"
    },
    {
        "show power-limit", 2, _poe_cmd_getDevicePowerLimit,
        "poe show power-limit [ unit=<UINT> ] [ device=<UINT> ]\n"
    },
    {
        "set scratch", 2, _poe_cmd_setDeviceScratch,
        "poe set scratch [ unit=<UINT> ] device=<UINT> scratch=<UINT>\n"
    },
    {
        "show scratch", 2, _poe_cmd_getDeviceScratch,
        "poe show scratch [ unit=<UINT> ] [ device=<UINT> ]\n"
    },
    {
        "set pse", 2, _poe_cmd_setPortPse,
        "poe set pse [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ disable | enable | force-power }\n"
    },
    {
        "show pse", 2, _poe_cmd_getPortPse,
        "poe show pse [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set type", 2, _poe_cmd_setPortType,
        "poe set type [ unit=<UINT> ] portlist=<UINTLIST> type={ af | at }\n"
    },
    {
        "show type", 2, _poe_cmd_getPortType,
        "poe show type [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set current-limit", 2, _poe_cmd_setPortCurrentLimit,
        "poe set current-limit [ unit=<UINT> ] portlist=<UINTLIST> limit=<UINT>\n"
    },
    {
        "show current-limit", 2, _poe_cmd_getPortCurrentLimit,
        "poe show current-limit [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set priority", 2, _poe_cmd_setPortPriority,
        "poe set priority [ unit=<UINT> ] portlist=<UINTLIST> priority=<UINT>\n"
    },
    {
        "show priority", 2, _poe_cmd_getPortPriority,
        "poe show priority [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "trigger manual-power-up", 2, _poe_cmd_triggerPortPowerUp,
        "poe trigger manual-power-up [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show manual-power-up", 2, _poe_cmd_getPortPowerUp,
        "poe show manual-power-up [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show measurement", 2, _poe_cmd_getMeasurement,
        "poe show measurement [ unit=<UINT> ] { [ device=<UINT> ] | portlist=<UINTLIST> }\n"
    },
    {
        "show status", 2, _poe_cmd_getPortStatus,
        "poe show status [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show event", 2, _poe_cmd_getPortEvent,
        "poe show event [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "clear event", 2, _poe_cmd_clearPortEvent,
        "poe clear event [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "dump reg", 2, _poe_cmd_dumpReg,
        "poe dump reg [ unit=<UINT> ] device=<UINT>\n"
    },
 };
/* clang-format on */

AIR_ERROR_NO_T
poe_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _poe_cmd_vec, sizeof(_poe_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
poe_cmd_usager()
{
    return (dsh_usager(_poe_cmd_vec, sizeof(_poe_cmd_vec) / sizeof(DSH_VEC_T)));
}
