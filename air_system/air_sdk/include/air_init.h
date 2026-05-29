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

/* FILE NAME:   air_init.h
 * PURPOSE:
 *      Custom configuration on AIR SDK.
 * NOTES:
 */
#ifndef AIR_INIT_H
#define AIR_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_module.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_INIT_DBG_FLAG_ERR  (1U << 0)
#define AIR_INIT_DBG_FLAG_WARN (1U << 1)
#define AIR_INIT_DBG_FLAG_INFO (1U << 2)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef void (*AIR_INIT_WRITE_FUNC_T)(
    const void *ptr_buf,
    UI32_T      len);

typedef AIR_ERROR_NO_T (*AIR_INIT_OPEN_NONVOLATILE_FUNC_T)(
    void);

typedef AIR_ERROR_NO_T (*AIR_INIT_CLOSE_NONVOLATILE_FUNC_T)(
    void);

typedef I32_T (*AIR_INIT_WRITE_NONVOLATILE_FUNC_T)(
    const void *ptr_buf,
    UI32_T      num_bytes);

typedef I32_T (*AIR_INIT_READ_NONVOLATILE_FUNC_T)(
    void  *ptr_buf,
    UI32_T num_bytes);

typedef struct AIR_INIT_PARAM_S
{
    AIR_INIT_WRITE_FUNC_T             dsh_write_func;
    AIR_INIT_WRITE_FUNC_T             debug_write_func;
    AIR_INIT_OPEN_NONVOLATILE_FUNC_T  open_nv_func;
    AIR_INIT_CLOSE_NONVOLATILE_FUNC_T close_nv_func;
    AIR_INIT_WRITE_NONVOLATILE_FUNC_T write_nv_func;
    AIR_INIT_READ_NONVOLATILE_FUNC_T  read_nv_func;
} AIR_INIT_PARAM_T;

typedef enum
{
    /* used for built-in GPHY port */
    AIR_INIT_PORT_TYPE_BASET,

    /* used for QSGMII/HSGMII/external CPU port */
    AIR_INIT_PORT_TYPE_XSGMII,

    /* used for internal CPU port */
    AIR_INIT_PORT_TYPE_CPU,

    /* used for enhanced built-in GPHY port */
    AIR_INIT_PORT_TYPE_ENHANCED_BASET,

    AIR_INIT_PORT_TYPE_LAST
} AIR_INIT_PORT_TYPE_T;

typedef enum
{
    AIR_INIT_PORT_SPEED_1000M = 1000,
    AIR_INIT_PORT_SPEED_2500M = 2500,
    AIR_INIT_PORT_SPEED_5000M = 5000,
    AIR_INIT_PORT_SPEED_LAST
} AIR_INIT_PORT_SPEED_T;

typedef struct AIR_INIT_BASET_PORT_S
{
    /* PHY port package id */
    UI32_T phy_pkg_id;
} AIR_INIT_BASET_PORT_T;

typedef struct AIR_INIT_ENHANCED_BASET_PORT_S
{
    /* flags defined */
#define AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE (1U << 1)
    UI32_T flags;
    /* PHY port package id */
    UI32_T phy_pkg_id;
} AIR_INIT_ENHANCED_BASET_PORT_T;

typedef struct AIR_INIT_XSGMII_PORT_S
{
    /* flags defined */
#define AIR_INIT_XSGMII_PORT_FLAGS_CPU     (1U << 0)
#define AIR_INIT_XSGMII_PORT_FLAGS_COMBO   (1U << 1)
#define AIR_INIT_XSGMII_PORT_FLAGS_CASCADE (1U << 2)
#define AIR_INIT_XSGMII_PORT_FLAGS_POE     (1U << 3)
    UI32_T flags;
    /* MAC (switch) QSGMII/HSGMII package id */
    UI32_T xsgmii_pkg_id;

    /* PHY port package id (set to be 0 if xSGMII is one port case) */
    UI32_T phy_pkg_id;
} AIR_INIT_XSGMII_PORT_T;

typedef struct AIR_INIT_PORT_MAP_S
{
    UI32_T                port;
    AIR_INIT_PORT_TYPE_T  port_type;
    AIR_INIT_PORT_SPEED_T max_speed;
    union
    {
        /* for port type is AIR_INIT_PORT_TYPE_BASET */
        AIR_INIT_BASET_PORT_T          baset_port;
        /* for port type is AIR_INIT_PORT_TYPE_XSGMII */
        AIR_INIT_XSGMII_PORT_T         xsgmii_port;
        /* for port type is AIR_INIT_ENHANCED_BASET_PORT_T */
        AIR_INIT_ENHANCED_BASET_PORT_T enhanced_baset_port;
    };
} AIR_INIT_PORT_MAP_T;

typedef enum
{
    AIR_INIT_POE_PORT_TYPE_AF_AT,
    AIR_INIT_POE_PORT_TYPE_BT,
    AIR_INIT_POE_PORT_TYPE_LAST
} AIR_INIT_POE_PORT_TYPE_T;

typedef struct AIR_INIT_POE_PORT_ALT_S
{
    /* used for configure POE device index of the specified port */
    UI32_T poe_device_idx;
    /* used for configure POE's hardware package id of the specified port */
    UI32_T hw_package_id;
} AIR_INIT_POE_PORT_ALT_T;

typedef struct AIR_INIT_POE_PORT_MAP_S
{
    UI32_T                   unit;
    UI32_T                   port;
    AIR_INIT_POE_PORT_TYPE_T port_type;
    AIR_INIT_POE_PORT_ALT_T  primary_port;
    AIR_INIT_POE_PORT_ALT_T  secondary_port;
} AIR_INIT_POE_PORT_MAP_T;

typedef struct AIR_INIT_POE_DEVICE_MAP_S
{
    UI32_T unit;
    /* used for configure POE device index */
    UI32_T poe_device_idx;
    /* used for configure I2C bus id */
    UI32_T i2c_bus_id;
    /* used for configure I2C slave address */
    UI32_T i2c_address;
} AIR_INIT_POE_DEVICE_MAP_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_init_initCmnModule
 * PURPOSE:
 *      This API is used to initialize the common modules.
 * INPUT:
 *      ptr_init_param          -- The sdk_demo callback functions.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initCmnModule(
    AIR_INIT_PARAM_T *ptr_init_param);

/* FUNCTION NAME:   air_init_deinitCmnModule
 * PURPOSE:
 *      This API is used to deinitialize the common modules.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitCmnModule(
    void);

/* FUNCTION NAME:   air_init_initLowLevel
 * PURPOSE:
 *      This API is used to initialize the low level modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry is not found.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initLowLevel(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_deinitLowLevel
 * PURPOSE:
 *      This API is used to deinitialize the low level modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitLowLevel(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_initTaskRsrc
 * PURPOSE:
 *      This API is used to initialize the task resources of the
 *      modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_ALREADY_INITED    -- Module is reinitialized.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initTaskRsrc(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_deinitTaskRsrc
 * PURPOSE:
 *      This API is used to deinitialize the task resources of the
 *      modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitTaskRsrc(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_initModule
 * PURPOSE:
 *      This API is used to initialize the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_TABLE_FULL        -- Table is full.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initModule(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_deinitModule
 * PURPOSE:
 *      This API is used to deinitialize the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitModule(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_initTask
 * PURPOSE:
 *      This API is used to initialize the tasks of the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_ALREADY_INITED    -- Module is reinitialized.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initTask(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_deinitTask
 * PURPOSE:
 *      This API is used to deinitialize the tasks of the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitTask(
    const UI32_T unit);

/* FUNCTION NAME:   air_init_getUnitNum
 * PURPOSE:
 *      This API is used to get the unit numbers.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_num                 -- The unit numbers
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_getUnitNum(
    UI32_T *ptr_num);

/* FUNCTION NAME:   air_deinit
 * PURPOSE:
 *      This API is used to deinitialize the AIR SDK.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_deinit(
    void);

/* FUNCTION NAME:   air_init_setModuleDebugFlag
 * PURPOSE:
 *      This API is used to set debug flag on each module. Once
 *      module's debug flag has been set, the corresponding debug
 *      messages will be dumpped by debug_write_func.
 * INPUT:
 *      unit                    -- The unit ID
 *      module_id               -- The module ID
 *      dbg_flag                -- The debug flag defined by
 *                                 AIR_INIT_DBG_FLAG_XXX
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      The unit ID is meaningless on this API.
 */
AIR_ERROR_NO_T
air_init_setModuleDebugFlag(
    const UI32_T       unit,
    const AIR_MODULE_T module_id,
    const UI32_T       dbg_flag);

/* FUNCTION NAME:   air_init_getModuleDebugFlag
 * PURPOSE:
 *      This API is used to get debug flag setting from each module.
 * INPUT:
 *      unit                    -- The unit ID
 *      module_id               -- The module ID
 * OUTPUT:
 *      ptr_dbg_flag            -- The debug flag defined by
 *                                 AIR_INIT_DBG_FLAG_XXX
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      The unit ID is meaningless on this API.
 */
AIR_ERROR_NO_T
air_init_getModuleDebugFlag(
    const UI32_T       unit,
    const AIR_MODULE_T module_id,
    UI32_T            *ptr_dbg_flag);

/* FUNCTION NAME:   air_init_initSdkPortMap
 * PURPOSE:
 *      This API is used to initialize SDK port mapping. All SDK API's
 *      port is based on those ports initialized by this API.
 * INPUT:
 *      unit                    -- The unit ID
 *      port_map_cnt            -- The port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry is not found.
 *      AIR_E_ENTRY_EXISTS      -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initSdkPortMap(
    const UI32_T               unit,
    const UI32_T               port_map_cnt,
    const AIR_INIT_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   air_init_getSdkPortMap
 * PURPOSE:
 *      This API is used to get SDK port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt        -- The pointer of port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_port_map parameter to get the port map.
 */
AIR_ERROR_NO_T
air_init_getSdkPortMap(
    const UI32_T         unit,
    UI32_T              *ptr_port_map_cnt,
    AIR_INIT_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   air_init_initPoePortMap
 * PURPOSE:
 *      This API is used to initialize PoE port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 *      port_map_cnt            -- The port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- SDK port is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initPoePortMap(
    const UI32_T                   unit,
    const UI32_T                   port_map_cnt,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   air_init_getPoePortMap
 * PURPOSE:
 *      This API is used to get PoE port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt        -- The pointer of port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_port_map parameter to get the port map.
 */
AIR_ERROR_NO_T
air_init_getPoePortMap(
    const UI32_T             unit,
    UI32_T                  *ptr_port_map_cnt,
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   air_init_initPoeDeviceMap
 * PURPOSE:
 *      This API is used to initialize PoE device mapping.
 * INPUT:
 *      unit                    -- The unit ID
 *      device_map_cnt          -- The device mapping count
 *      ptr_device_map          -- The pointer of device mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initPoeDeviceMap(
    const UI32_T                     unit,
    const UI32_T                     device_map_cnt,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map);

/* FUNCTION NAME:   air_init_getPoeDeviceMap
 * PURPOSE:
 *      This API is used to get PoE device mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_device_map_cnt      -- The pointer of device mapping count
 *      ptr_device_map          -- The pointer of device mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_device_map parameter to get the device map.
 */
AIR_ERROR_NO_T
air_init_getPoeDeviceMap(
    const UI32_T               unit,
    UI32_T                    *ptr_device_map_cnt,
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map);

#endif /* AIR_INIT_H */
