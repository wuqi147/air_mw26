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

/* FILE NAME:  hal.c
 * PURPOSE:
 *  1. Provide whole HAL resource initialization API.
 *  2. Provide HAL per-unit initialization and de-initialization function APIs.
 *  3. Provide HAL database access APIs.
 *  4. Provide a HAL multiplexing vector retrival.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal.h>

#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_IS_INITED(unit) (_hal_is_inited[unit])

#define PTR_HAL_CHIP_INFO(unit)   (_ext_chip_control_block[unit].ptr_chip_info)
#define PTR_HAL_DRIVER_INFO(unit) (_ext_chip_control_block[unit].ptr_driver_info)
#define PTR_HAL_MODULE_INFO(unit) (_ext_chip_control_block[unit].ptr_module_info)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_HAL, "hal.c");

// HAL_CMN_FUNC_VEC_T
//*_ext_ptr_chip_cmn_func_vector[2];

const HAL_FUNC_VEC_T *_ext_ptr_chip_func_vector[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

HAL_CHIP_INFO_T
*_ext_ptr_chip_info[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

HAL_CHIP_CB_T
_ext_chip_control_block[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T _hal_is_inited[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM] = {FALSE};

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _hal_constructChipBlock
 * PURPOSE:
 *      _hal_constructChipBlock() is a helper to construct chip control block
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Construct successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_NO_MEMORY     -- No memory to construct
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_constructChipBlock(
    const UI32_T unit)
{
    /* check if input unit is over the supported maximum unit */
    if (2 <= unit)
    {
        DIAG_PRINT(HAL_DBG_WARN, "invalid unit setting, unit=%u > max=%u\n", unit, 2);
        OSAL_ASSERT(2 > unit);

        return AIR_E_BAD_PARAMETER;
    }

    /* allocate memory for this chip control block */
    PTR_HAL_CHIP_INFO(unit) =
        (HAL_CHIP_INFO_T *)osal_alloc(sizeof(HAL_CHIP_INFO_T), air_module_getModuleName(AIR_MODULE_HAL));

    if (NULL == PTR_HAL_CHIP_INFO(unit))
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc mem size=%zu failed\n", unit, sizeof(HAL_CHIP_INFO_T));
        return AIR_E_NO_MEMORY;
    }

    /* memory set the allocated memory as zero */
    osal_memset(PTR_HAL_CHIP_INFO(unit), 0x00, sizeof(HAL_CHIP_INFO_T));
    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_deConstructChipBlock
 * PURPOSE:
 *      _hal_deConstructChipBlock() is a helper to deconstruct chip control block
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deConstructChipBlock(
    const UI32_T unit)
{
    if (NULL != PTR_HAL_CHIP_INFO(unit))
    {
        /* free the allocated memory */
        osal_free(PTR_HAL_CHIP_INFO(unit));
        PTR_HAL_CHIP_INFO(unit) = NULL;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_updateChipPortInfo
 * PURPOSE:
 *      _hal_updateChipPortInfo is used to update chip port info
 *
 * INPUT:
 *      unit                -- The chip unit number
 *      ptr_chip_info       -- Pointer to chip information structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_updateChipPortInfo(
    UI32_T           unit,
    HAL_CHIP_INFO_T *ptr_chip_info)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* initialize chip specific information */
    if (HAL_IS_DEVICE_SCO_FAMILY(unit))
    {
        /* cpu port(air port) will be assigned on air_init_initSdkPortMap() */
        ptr_chip_info->cpu_port = AIR_PORT_INVALID;

        switch (ptr_chip_info->device_id)
        {
            case HAL_SCO_DEVICE_ID_EN8851E:
                ptr_chip_info->total_port = 9; /* 8 GPHY + 1 CPU */
                break;
            case HAL_SCO_DEVICE_ID_EN8851C:
                ptr_chip_info->total_port = 14; /* 8 GPHY + 2 xSGMII (or 4 combo) + 1 CPU */
                break;
            case HAL_SCO_DEVICE_ID_EN8853C:
                ptr_chip_info->total_port = 29; /* 8 GPHY + 5 xSGMII(20x1G) + 1 CPU */
                break;
            case HAL_SCO_DEVICE_ID_EN8860C:
                ptr_chip_info->total_port = 6; /* 5 xSGMII + 1 CPU */
                break;
            default:
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else if (HAL_IS_DEVICE_PEARL_FAMILY(unit))
    {
        /* cpu port(air port) will be assigned on air_init_initSdkPortMap() */
        ptr_chip_info->cpu_port = AIR_PORT_INVALID;

        switch (ptr_chip_info->device_id)
        {
            case HAL_PEARL_DEVICE_ID_AN8855M:
                ptr_chip_info->total_port = 7; /* 5 GPHY + 1 xSGMII(1 single) + 1 CPU */
                break;
            case HAL_PEARL_DEVICE_ID_AN8855H:
                ptr_chip_info->total_port = 6; /* 5 GPHY + 1 xSGMII(1 single) */
                break;
            default:
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else if (HAL_IS_DEVICE_CORAL_FAMILY(unit))
    {
        /* cpu port(air port) will be assigned on air_init_initSdkPortMap() */
        ptr_chip_info->cpu_port = AIR_PORT_INVALID;

        switch (ptr_chip_info->device_id)
        {
            case HAL_CORAL_DEVICE_ID_AN8858C:
            case HAL_CORAL_DEVICE_ID_AN8858H:
                ptr_chip_info->total_port = 11; /* 8 GPHY + 2 xSGMII(2 single) + 1 CPU */
                break;
            case HAL_CORAL_DEVICE_ID_AN8858E:   /* 8 GPHY + 1 CPU */
            case HAL_CORAL_DEVICE_ID_AN8858B:   /* 6 GPHY + 2 xSGMII(2 single) + 1 CPU */
                ptr_chip_info->total_port = 9;
                break;
#ifdef AIR_LINUX
            case HAL_CORAL_DEVICE_ID_AN8858F: /* 8 GPHY */
                ptr_chip_info->total_port = 8;
                break;
            case HAL_CORAL_DEVICE_ID_AN8858D: /* 8 GPHY + 2 xSGMII(2 single) */
                ptr_chip_info->total_port = 10;
                break;
#endif
            default:
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else
    {
        rc = AIR_E_OTHERS;
    }

    return (rc);
}

/* FUNCTION NAME:   _hal_mapChipDeviceId
 * PURPOSE:
 *      _hal_mapChipDeviceId is used to map customer device ID to chip.
 *
 * INPUT:
 *      unit                -- The chip unit number
 *      cfg_device_id       -- The customer device ID
 *
 * OUTPUT:
 *      ptr_chip_device_id  -- Pointer to chip device ID
 *
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_mapChipDeviceId(
    UI32_T  unit,
    UI32_T  cfg_device_id,
    UI32_T *ptr_chip_device_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_IS_DEVICE_SCO_FAMILY(unit))
    {
        /* 8851 family */
        switch (cfg_device_id)
        {
            case 0x8851C:
                (*ptr_chip_device_id) = HAL_SCO_DEVICE_ID_EN8851C;
                break;
            case 0x8851E:
                (*ptr_chip_device_id) = HAL_SCO_DEVICE_ID_EN8851E;
                break;
            case 0x8853C:
                (*ptr_chip_device_id) = HAL_SCO_DEVICE_ID_EN8853C;
                break;
            case 0x8860C:
                (*ptr_chip_device_id) = HAL_SCO_DEVICE_ID_EN8860C;
                break;
            default:
                (*ptr_chip_device_id) = 0;
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else if (HAL_IS_DEVICE_PEARL_FAMILY(unit))
    {
        /* 8855 family */
        switch (cfg_device_id)
        {
            case 0x8855:
                (*ptr_chip_device_id) = HAL_PEARL_DEVICE_ID_AN8855M;
                break;
            default:
                (*ptr_chip_device_id) = 0;
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else if (HAL_IS_DEVICE_CORAL_FAMILY(unit))
    {
        /* 8858 family */
        switch (cfg_device_id)
        {
            case 0x8858C:
                (*ptr_chip_device_id) = HAL_CORAL_DEVICE_ID_AN8858C;
                break;
            case 0x8858E:
                (*ptr_chip_device_id) = HAL_CORAL_DEVICE_ID_AN8858E;
                break;
            case 0x8858:
                (*ptr_chip_device_id) = HAL_CORAL_DEVICE_ID_AN8858H;
                break;
            case 0x8858B:
                (*ptr_chip_device_id) = HAL_CORAL_DEVICE_ID_AN8858B;
                break;
            default:
                (*ptr_chip_device_id) = 0;
                rc = AIR_E_OTHERS;
                break;
        }
    }
    else
    {
        rc = AIR_E_OTHERS;
    }

    return rc;
}

/* FUNCTION NAME:   _hal_updateChipDeviceInfo
 * PURPOSE:
 *      _hal_updateChipDeviceInfo is used to update chip device info.
 *
 * INPUT:
 *      unit                -- The chip unit number
 *      ptr_chip_info       -- Pointer to chip information structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_updateChipDeviceInfo(
    UI32_T           unit,
    HAL_CHIP_INFO_T *ptr_chip_info)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          device_id = 0, revision_id = 0;
    AIR_CFG_VALUE_T cfg_val;

    /* get device id, reversion id from efuse */
    rc = HAL_FUNC_CALL(unit, chip, readDeviceInfo, (unit, &device_id, &revision_id));
    if ((AIR_E_OK != rc) || ((0 == device_id) && (0 == revision_id)))
    {
        /* get id from efuse failed, try to get force device-ID */
        osal_memset(&cfg_val, 0, sizeof(AIR_CFG_VALUE_T));
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_FORCE_DEVICE_ID, &cfg_val);
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "u=%u, get cfg device id 0x%x\n", unit, cfg_val.value);
            if (0 == cfg_val.value)
            {
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, no efuse device id and force device id invalid\n", unit);
                rc = AIR_E_OTHERS;
            }
            else
            {
                rc = _hal_mapChipDeviceId(unit, cfg_val.value, &device_id);
                if (AIR_E_OK == rc)
                {
                    ptr_chip_info->device_id = device_id;
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_ERR, "u=%u, cfg device id invalid\n", unit);
                }
            }
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, get cfg device id failed, rc=%d\n", unit, rc);
        }
    }
    else
    {
        if (ptr_chip_info->revision_id != revision_id)
        {
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, chip revision(%d) is not equal to reg val(%d). \n", unit, revision_id,
                       ptr_chip_info->revision_id);
        }
        ptr_chip_info->device_id = device_id;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, family 0x%x, device 0x%x, revision 0x%x\n", unit, HAL_DEVICE_FAMILY_ID(unit),
               HAL_DEVICE_CHIP_ID(unit), HAL_DEVICE_REV_ID(unit));

    return rc;
}

/* FUNCTION NAME:   _hal_initSdkPortMapInfo
 * PURPOSE:
 *      This function is used to init sdk port map database
 *
 * INPUT:
 *      unit                -- The chip unit number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initSdkPortMapInfo(
    UI32_T unit)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_CHIP_INFO_T *ptr_chip_info = PTR_HAL_CHIP_INFO(unit);
    UI32_T           port;

    /* allocate sdk air port to chip port map information memory */
    ptr_chip_info->ptr_sdk_port_map_info =
        osal_alloc(AIR_PORT_NUM * sizeof(HAL_SDK_PORT_MAP_T), air_module_getModuleName(AIR_MODULE_HAL));

    if (NULL != ptr_chip_info->ptr_sdk_port_map_info)
    {
        osal_memset(ptr_chip_info->ptr_sdk_port_map_info, 0x0, (AIR_PORT_NUM * sizeof(HAL_SDK_PORT_MAP_T)));

        /* initialize valid and flag fields */
        for (port = 0; port < AIR_PORT_NUM; port++)
        {
            ptr_chip_info->ptr_sdk_port_map_info[port].valid = 0;
            ptr_chip_info->ptr_sdk_port_map_info[port].port_type = AIR_INIT_PORT_TYPE_LAST;
            ptr_chip_info->ptr_sdk_port_map_info[port].max_speed = AIR_INIT_PORT_SPEED_LAST;
            ptr_chip_info->ptr_sdk_port_map_info[port].mac_port = AIR_PORT_INVALID;
#ifdef AIR_EN_POE
            ptr_chip_info->ptr_sdk_port_map_info[port].poe_port_info.poe_type = AIR_INIT_POE_PORT_TYPE_LAST;
#endif
        }
    }
    else
    {
        rc = AIR_E_NO_MEMORY;
    }

    /* allocate reverse mac port to sdk air port map information memory */
    if (AIR_E_OK == rc)
    {
        ptr_chip_info->ptr_mac_port_map_info =
            osal_alloc(AIR_PORT_NUM * sizeof(UI32_T), air_module_getModuleName(AIR_MODULE_HAL));

        if (NULL != ptr_chip_info->ptr_mac_port_map_info)
        {
            osal_memset(ptr_chip_info->ptr_mac_port_map_info, 0xFF, (AIR_PORT_NUM * sizeof(UI32_T)));
        }
        else
        {
            rc = AIR_E_NO_MEMORY;
        }
    }

    return rc;
}

/* FUNCTION NAME:   _hal_deinitSdkPortMapInfo
 * PURPOSE:
 *      This function is used to deinit sdk port map database
 *
 * INPUT:
 *      unit                -- The chip unit number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK            -- Operation successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitSdkPortMapInfo(
    UI32_T unit)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_CHIP_INFO_T *ptr_chip_info = PTR_HAL_CHIP_INFO(unit);

    /* free sdk air port to mac port map information memory */
    if (NULL != ptr_chip_info->ptr_sdk_port_map_info)
    {
        osal_free(ptr_chip_info->ptr_sdk_port_map_info);
    }

    /* free mac port to sdk air port map information memory */
    if (NULL != ptr_chip_info->ptr_mac_port_map_info)
    {
        osal_free(ptr_chip_info->ptr_mac_port_map_info);
    }

    return rc;
}

/* FUNCTION NAME:   _hal_initChipInfo
 * PURPOSE:
 *      _hal_initChipInfo is used to init and get chip info from AML
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initChipInfo(
    const UI32_T unit)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    HAL_CHIP_INFO_T *ptr_chip_info = NULL;
    UI32_T           family_id = 0, rev_id = 0;

    /* call API provided by AML to get family ID and revision-ID */
    rc = aml_getDeviceInfo(unit, &family_id, &rev_id);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invoke aml_getDeviceId failed, rc=%d\n", unit, rc);
        return rc;
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, family ID=0x%x, revision=%d\n", unit, family_id, rev_id);

    /* get chip info pointer from chip control block */
    ptr_chip_info = PTR_HAL_CHIP_INFO(unit);
    PTR_HAL_EXT_CHIP_INFO(unit) = ptr_chip_info;

    /* update chip information */
    ptr_chip_info->family_id = family_id;
    ptr_chip_info->revision_id = rev_id;

    return (rc);
}

/* FUNCTION NAME:   _hal_deinitChipInfo
 * PURPOSE:
 *      _hal_deinitChipInfo is used to deinit chip info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Deconstruct successfully
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitChipInfo(
    const UI32_T unit)
{
    /* memory set this chip information as zero */
    osal_memset(PTR_HAL_CHIP_INFO(unit), 0x00, sizeof(HAL_CHIP_INFO_T));
    PTR_HAL_EXT_CHIP_INFO(unit) = NULL;
    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_initDriverInfo
 * PURPOSE:
 *      _hal_initDriverInfo is used to init chip driver info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Chip driver initialized successfully
 *      AIR_E_ENTRY_NOT_FOUND   -- Find driver failed
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initDriverInfo(
    const UI32_T unit)
{
    HAL_CHIP_INFO_T *ptr_chip_info = NULL;
    HAL_DRIVER_T    *ptr_driver_info = NULL;
    AIR_ERROR_NO_T   rc = AIR_E_OK;

    /* get chip info from chip control block */
    ptr_chip_info = PTR_HAL_CHIP_INFO(unit);
    /* initialize driver information */
    rc = hal_drv_initDeviceDriver(ptr_chip_info->family_id, ptr_chip_info->revision_id, &ptr_driver_info);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* update driver information here */
    PTR_HAL_DRIVER_INFO(unit) = ptr_driver_info;

    /* update chip function vector here */
    PTR_HAL_FUNC_VECTOR(unit) = ptr_driver_info->ptr_func_vector;

    return rc;
}

/* FUNCTION NAME:   _hal_deinitDriverInfo
 * PURPOSE:
 *      _hal_deinitDriverInfo is used to deinit chip driver info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Chip driver deinitialized successfully
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitDriverInfo(
    const UI32_T unit)
{
    /* clean driver information pointer */
    PTR_HAL_DRIVER_INFO(unit) = NULL;

    /* clean chip function vector pointer */
    PTR_HAL_FUNC_VECTOR(unit) = NULL;

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_initChipPortInfo
 * PURPOSE:
 *      _hal_initChipPortInfo is used to init and get chip port info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Chip port info initialized successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initChipPortInfo(
    const UI32_T unit)
{
    AIR_ERROR_NO_T                 rc = AIR_E_OK;
    const HAL_DRV_MAC_PORT_INFO_T *ptr_mac_port_info_entry;
    HAL_CHIP_INFO_T               *ptr_chip_info = NULL;
    HAL_DRIVER_T                  *ptr_driver_info = NULL;

    /* get chip, driver info from control block */
    ptr_chip_info = PTR_HAL_CHIP_INFO(unit);
    ptr_driver_info = PTR_HAL_DRIVER_INFO(unit);

    /* update chip device info */
    rc = _hal_updateChipDeviceInfo(unit, ptr_chip_info);
    if (AIR_E_OK == rc)
    {
        /* update chip port info */
        rc = _hal_updateChipPortInfo(unit, ptr_chip_info);
        if (AIR_E_OK == rc)
        {
            /* update chip mac port information */
            if (ptr_driver_info->ptr_mac_port_info != NULL)
            {
                /* find correct mac port information for this device family */
                ptr_mac_port_info_entry = ptr_driver_info->ptr_mac_port_info;

                while (ptr_mac_port_info_entry->device_id != HAL_INVALID_DEVICE_ID)
                {
                    if (ptr_chip_info->device_id == ptr_mac_port_info_entry->device_id)
                    {
                        /* found and record this pointer for this unit */
                        HAL_CHIP_MAC_PORT_INFO_PTR(unit) = ptr_mac_port_info_entry;
                        break;
                    }
                    ptr_mac_port_info_entry++;
                }

                if (HAL_INVALID_DEVICE_ID == ptr_mac_port_info_entry->device_id)
                {
                    /* can't find correct MAC macro mapping for this device */
                    HAL_CHIP_MAC_PORT_INFO_PTR(unit) = NULL;
                    DIAG_PRINT(HAL_DBG_ERR, "u=%u, hook dev id=0x%x mac port info failed\n", unit,
                               ptr_chip_info->device_id);
                    rc = AIR_E_OTHERS;
                }
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, driver for dev id=0x%x without mac port info\n", unit,
                           ptr_chip_info->device_id);
                rc = AIR_E_OTHERS;
            }
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, update chip port info failed(%d). \n", unit, rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, update chip device info failed(%d). \n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   _hal_deinitChipPortInfo
 * PURPOSE:
 *      _hal_deinitChipPortInfo is used to deinit chip port info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Chip port info deinitialized successfully
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitChipPortInfo(
    const UI32_T unit)
{
    /* clean chip related info */
    HAL_DEVICE_REV_ID(unit) = 0;
    HAL_TOTAL_PORT_NUM(unit) = 0;
    HAL_CHIP_MAC_PORT_INFO_PTR(unit) = NULL;

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_initModuleInfo
 * PURPOSE:
 *      _hal_initModuleInfo is used to init module info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Module info initialized successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initModuleInfo(
    const UI32_T unit)
{
    HAL_MODULE_INFO_T *ptr_module_info = NULL;
    UI32_T             module = 0;
    AIR_ERROR_NO_T     rc = AIR_E_OK;

    /* allocate memory for this chip control block */
    ptr_module_info = (HAL_MODULE_INFO_T *)osal_alloc(AIR_MODULE_LAST * sizeof(HAL_MODULE_INFO_T),
                                                      air_module_getModuleName(AIR_MODULE_HAL));

    if (NULL == ptr_module_info)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc mem size=%zu failed\n", unit,
                   (AIR_MODULE_LAST * sizeof(HAL_MODULE_INFO_T)));
        rc = AIR_E_NO_MEMORY;
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(ptr_module_info, 0x00, AIR_MODULE_LAST * sizeof(HAL_MODULE_INFO_T));

        PTR_HAL_MODULE_INFO(unit) = ptr_module_info;

        for (module = 0; module < AIR_MODULE_LAST; module++)
        {
            HAL_MODULE_INITED(unit, module) = HAL_INIT_STAGE_NONE;
        }
    }

    return rc;
}

/* FUNCTION NAME:   _hal_deinitModuleInfo
 * PURPOSE:
 *      _hal_deinitModuleInfo is used to deinit module info
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- module info deinitialized successfully
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitModuleInfo(
    const UI32_T unit)
{
    /* free the allocated memory */
    osal_free(PTR_HAL_MODULE_INFO(unit));
    PTR_HAL_MODULE_INFO(unit) = NULL;

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_initRsrc
 * PURPOSE:
 *      _hal_initRsrc is used to init HAL software resource
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Resource is initialized successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc;

    /* initialize chip control block as default value */
    osal_memset(&_ext_chip_control_block[unit], 0x00, sizeof(HAL_CHIP_CB_T));
    /* initialize chip function vector to be NULL */
    _ext_ptr_chip_func_vector[unit] = NULL;
    //_ext_ptr_chip_cmn_func_vector[unit] = NULL;
    _ext_ptr_chip_info[unit] = NULL;

    /* construct chip control block */
    rc = _hal_constructChipBlock(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize chip information */
    rc = _hal_initChipInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize driver information */
    rc = _hal_initDriverInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize chip port information */
    rc = _hal_initChipPortInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize module information */
    rc = _hal_initModuleInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* initialize sdk port map database */
    rc = _hal_initSdkPortMapInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_deinitRsrc
 * PURPOSE:
 *      _hal_deinitRsrc is used to deinit HAL software resource
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Resource is deinitialized successfully
 *      AIR_E_BAD_PARAMETER -- Invalid parameter
 *      AIR_E_OTHERS        -- Internal error
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_deinitRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* check if input unit is over the supported maximum unit */
    if (AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM <= unit)
    {
        DIAG_PRINT(HAL_DBG_WARN, "invalid unit setting, unit=%u > max=%u\n", unit, AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM);
        OSAL_ASSERT(AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM > unit);
        return AIR_E_BAD_PARAMETER;
    }

    /* de-initialize sdk port map information */
    rc = _hal_deinitSdkPortMapInfo(unit);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit sdk port map info failed, rc=%d\n", unit, rc);
        return rc;
    }

    /* de-initialize module information */
    rc = _hal_deinitModuleInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* de-initialize chip port information */
    rc = _hal_deinitChipPortInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* de-initialize driver information */
    rc = _hal_deinitDriverInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* de-initialize chip information */
    rc = _hal_deinitChipInfo(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* de-construct chip control block */
    rc = _hal_deConstructChipBlock(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_initCfg
 * PURPOSE:
 *      _hal_initCfg is used to do chip-level early stage init
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Init successfully
 *
 * NOTES:
 *      None
 *
 */
static AIR_ERROR_NO_T
_hal_initCfg(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_init
 * PURPOSE:
 *      hal_init() is responsible for HAL initialization, it will do
 *      the following:
 *      1. Construct chip control block.
 *      2. Initialize chip information.
 *      3. Initialize driver information.
 *
 * INPUT:
 *      unit          -- The unit number that would like to be initialized.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully initialize HAL.
 *      AIR_E_OTHERS  -- Fail to complete initialization procedure.
 *
 * NOTES:
 *      This function will be invoked by init module's initialization
 *      framework.
 *
 */
AIR_ERROR_NO_T
hal_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (TRUE == HAL_IS_INITED(unit))
    {
        return AIR_E_ALREADY_INITED;
    }

    rc = _hal_initRsrc(unit);
    if (AIR_E_OK == rc)
    {
        rc = _hal_initCfg(unit);
    }

    /* hardware register and table is not configured, and thread is not created in hal_init() */
    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_LOW_LEVEL;
        HAL_MODULE_INITED(unit, AIR_MODULE_HAL) |= HAL_INIT_STAGE_LOW_LEVEL;
        HAL_IS_INITED(unit) = TRUE;
    }

    return rc;
}

/* FUNCTION NAME:   hal_deinit
 * PURPOSE:
 *      hal_deinit() is responsible for HAL de-initialization, it will
 *      do the following:
 *      1. Reset driver information.
 *      2. Reset chip information.
 *      3. Free the constructed chip control block.
 *
 * INPUT:
 *      unit          -- The unit number that would like to de-initialized.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully de-initialize HAL.
 *      AIR_E_OTHERS  -- Fail to complete de-initialization procedure.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_E_OK == rc)
    {
        rc = _hal_deinitRsrc(unit);
    }

    if (AIR_E_OK == rc)
    {
        HAL_IS_INITED(unit) = FALSE;
    }

    return rc;
}

/* FUNCTION NAME:   hal_getSystemUnitNum
 * PURPOSE:
 *      hal_getSystemUnitNum() is an API that allows to get current valid
 *      unit number information for this system.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_unit_num         -- The total unit number information.
 * RETURN:
 *      AIR_E_OK             -- Get the system unit information successfully.
 *      AIR_E_BAD_PARAMETER  -- Parameter, ptr_unit_num, is a NULL pointer.
 *
 * NOTES:
 *      Please note this API will return current valid unit number
 *      information. For example, if there are two units on this system,
 *      it will return two from this API. If one of these two units has been
 *      removed, it will return one from this API.
 *
 */
AIR_ERROR_NO_T
hal_getSystemUnitNum(
    UI32_T *ptr_unit_num)
{
    UI32_T i = 0;
    UI32_T unit_num = 0;

    HAL_CHECK_PTR(ptr_unit_num);

    /* calculate how many valid units are there */
    for (i = 0; i < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; i++)
    {
        if (TRUE == HAL_IS_INITED(i))
        {
            unit_num++;
        }
    }

    /* assign valid unit number for output */
    *ptr_unit_num = unit_num;

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_getUnitPortNum
 * PURPOSE:
 *      hal_getUnitPortNum() is an API that allows to get total port
 *      number information for this unit.
 *
 * INPUT:
 *      unit                 -- The specified unit number.
 * OUTPUT:
 *      ptr_port_num         -- The total port number information on the specified
 *                              unit.
 * RETURN:
 *      AIR_E_OK             -- Get the unit's port number information successfully.
 *      AIR_E_BAD_PARAMETER  -- Parameter, ptr_port_num, is a NULL pointer.
 *      AIR_E_OTHERS         -- Fail to get the port number information.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_getUnitPortNum(
    const UI32_T unit,
    UI32_T      *ptr_port_num)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_port_num);

    /* check if the control block's chip has chip information ready */
    HAL_CHECK_PTR(PTR_HAL_CHIP_INFO(unit));

    /* assign the the total port number of this unit */
    *ptr_port_num = PTR_HAL_CHIP_INFO(unit)->total_port;

    return AIR_E_OK;
}
