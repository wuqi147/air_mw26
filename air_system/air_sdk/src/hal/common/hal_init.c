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

/* FILE NAME:  hal_init.h
 * PURPOSE:
 *      It provide hal related initial APIs.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_init.h>
#include <air_module.h>
#include <air_types.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_HAL, "hal_init.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* help function to check sdk port entry is valid */
static AIR_ERROR_NO_T
_hal_init_checkSdkPortParam(
    const UI32_T               unit,
    const AIR_INIT_PORT_MAP_T *ptr_port_map_entry)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    const HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    UI32_T                    idx;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, (ptr_port_map_entry->port)));

    /* check if air port is less than AIR_PORT_NUM */
    if (ptr_port_map_entry->port >= AIR_PORT_NUM)
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid sdk port=%u, valid 0-%d, rc=%d\n", unit, ptr_port_map_entry->port,
                   AIR_PORT_NUM, rc);
        return rc;
    }

    /* check if there are duplicated sdk port */
    if (ptr_sdk_port_map_entry->valid)
    {
        rc = AIR_E_ENTRY_EXISTS;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated port for sdk port=%u, rc=%d\n", unit, ptr_port_map_entry->port, rc);
        return rc;
    }

    /* check if there are duplicated cpu port */
    if ((ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_CPU) && (HAL_CPU_PORT(unit) != AIR_PORT_INVALID))
    {
        rc = AIR_E_ENTRY_EXISTS;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated sdk cpu port=%u, rc=%d\n", unit, ptr_port_map_entry->port, rc);
        return rc;
    }

    /* check if gphy id valid range is 0-7 */

    /* check if xsgmii id valid range is 0-3 and pkg id valid range is 0-7 */

    /* check if there are duplicated baset ports or xgmii ports */
    for (idx = 0; idx < AIR_PORT_NUM; idx++)
    {
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, idx));

        if ((ptr_sdk_port_map_entry->valid) && (ptr_sdk_port_map_entry->port_type == ptr_port_map_entry->port_type))
        {
            if ((ptr_sdk_port_map_entry->port_type == AIR_INIT_PORT_TYPE_BASET) &&
                (ptr_sdk_port_map_entry->baset_port.phy_pkg_id == ptr_port_map_entry->baset_port.phy_pkg_id))
            {
                rc = AIR_E_ENTRY_EXISTS;
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated baset port for phy-pkg-id=%u, rc=%d\n", unit,
                           ptr_port_map_entry->baset_port.phy_pkg_id, rc);
                return rc;
            }

            if ((ptr_sdk_port_map_entry->port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET) &&
                (ptr_sdk_port_map_entry->enhanced_baset_port.phy_pkg_id ==
                 ptr_port_map_entry->enhanced_baset_port.phy_pkg_id))
            {
                rc = AIR_E_ENTRY_EXISTS;
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated enhanced baset port for phy-pkg-id=%u, rc=%d\n", unit,
                           ptr_port_map_entry->enhanced_baset_port.phy_pkg_id, rc);
                return rc;
            }

            if ((ptr_sdk_port_map_entry->port_type == AIR_INIT_PORT_TYPE_XSGMII) &&
                (ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id == ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id) &&
                (ptr_sdk_port_map_entry->xsgmii_port.phy_pkg_id == ptr_port_map_entry->xsgmii_port.phy_pkg_id))
            {
                rc = AIR_E_ENTRY_EXISTS;
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated xsgmii port for xsgmii-id=%u phy-pkg-id=%u, rc=%d\n", unit,
                           ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id, ptr_port_map_entry->xsgmii_port.phy_pkg_id,
                           rc);
                return rc;
            }
        }
    }

    return AIR_E_OK;
}

/* help function to update sdk port(air port) attributes */
static AIR_ERROR_NO_T
_hal_init_updateSdkPortInfo(
    const UI32_T               unit,
    const AIR_INIT_PORT_MAP_T *ptr_port_map_entry)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    HAL_SDK_PORT_MAP_T           *ptr_sdk_port_map_entry;
    const HAL_DRV_MAC_PORT_MAP_T *ptr_mac_port_map_entry;
    UI32_T                        idx, xsgmii_port_gphy_id;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, (ptr_port_map_entry->port)));

    /* update sdk port attributes */
    ptr_sdk_port_map_entry->port_type = ptr_port_map_entry->port_type;
    ptr_sdk_port_map_entry->max_speed = ptr_port_map_entry->max_speed;

    switch (ptr_port_map_entry->port_type)
    {
        case AIR_INIT_PORT_TYPE_BASET:
            ptr_sdk_port_map_entry->baset_port.phy_pkg_id = ptr_port_map_entry->baset_port.phy_pkg_id;
            break;
        case AIR_INIT_PORT_TYPE_ENHANCED_BASET:
            ptr_sdk_port_map_entry->enhanced_baset_port.flags = ptr_port_map_entry->enhanced_baset_port.flags;
            ptr_sdk_port_map_entry->enhanced_baset_port.phy_pkg_id = ptr_port_map_entry->enhanced_baset_port.phy_pkg_id;
            break;
        case AIR_INIT_PORT_TYPE_XSGMII:
            ptr_sdk_port_map_entry->xsgmii_port.flags = ptr_port_map_entry->xsgmii_port.flags;
            ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id = ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id;
            ptr_sdk_port_map_entry->xsgmii_port.phy_pkg_id = ptr_port_map_entry->xsgmii_port.phy_pkg_id;
            break;
        case AIR_INIT_PORT_TYPE_CPU:
            /* do nothing */
            break;
        default:
            /* impossible case due to have a check before entry this function */
            rc = AIR_E_BAD_PARAMETER;
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid port type=%d, rc=%d\n", unit, ptr_port_map_entry->port_type, rc);
            break;
    }

    if (AIR_E_OK == rc)
    {
        /* get first entry of mac port map informaiton */
        ptr_mac_port_map_entry = HAL_CHIP_MAC_PORT_INFO_PTR(unit)->ptr_mac_port_map;

        /* search for corresponding mac port id of this sdk port (air port) */
        for (idx = 0; idx < (HAL_CHIP_MAC_PORT_INFO_PTR(unit)->mac_port_cnt); idx++)
        {
            if ((ptr_mac_port_map_entry->type == ptr_port_map_entry->port_type) ||
                ((ptr_mac_port_map_entry->type == AIR_INIT_PORT_TYPE_BASET) &&
                 (ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET)))
            {
                if ((ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_BASET) &&
                    (ptr_port_map_entry->baset_port.phy_pkg_id == ptr_mac_port_map_entry->phy_pkg_id))
                {
                    /* found this baset port's corresonding mac port id */
                    ptr_sdk_port_map_entry->mac_port = ptr_mac_port_map_entry->mac_port_id;

                    /* update related port bitmap */
                    AIR_PORT_ADD(HAL_PORT_BMP_TOTAL(unit), (ptr_port_map_entry->port));
                    AIR_PORT_ADD(HAL_PORT_BMP(unit), (ptr_port_map_entry->port));
                    AIR_PORT_ADD(HAL_PORT_BMP_GPHY(unit), (ptr_port_map_entry->port));

                    /* break out for-loop */
                    break;
                }

                if ((ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET) &&
                    (ptr_port_map_entry->enhanced_baset_port.phy_pkg_id == ptr_mac_port_map_entry->phy_pkg_id))
                {
                    /* found this baset port's corresonding mac port id */
                    ptr_sdk_port_map_entry->mac_port = ptr_mac_port_map_entry->mac_port_id;

                    /* update related port bitmap */
                    AIR_PORT_ADD(HAL_PORT_BMP_TOTAL(unit), (ptr_port_map_entry->port));
                    AIR_PORT_ADD(HAL_PORT_BMP(unit), (ptr_port_map_entry->port));
                    AIR_PORT_ADD(HAL_PORT_BMP_GPHY(unit), (ptr_port_map_entry->port));

                    if ((ptr_port_map_entry->enhanced_baset_port.flags & AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE))
                    {
                        /* update air poe port bitmap */
                        AIR_PORT_ADD(HAL_PORT_BMP_POE(unit), (ptr_port_map_entry->port));
                    }
                    /* break out for-loop */
                    break;
                }

                if ((ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_XSGMII) &&
                    (ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id == ptr_mac_port_map_entry->xsgmii_pkg_id))
                {
                    /* check xsgmii port's gphy id > 4 or not */
                    xsgmii_port_gphy_id = (ptr_port_map_entry->xsgmii_port.phy_pkg_id >= 4) ?
                                              (ptr_port_map_entry->xsgmii_port.phy_pkg_id - 4) :
                                              ptr_port_map_entry->xsgmii_port.phy_pkg_id;

                    if (xsgmii_port_gphy_id == ptr_mac_port_map_entry->phy_pkg_id)
                    {
                        /* found this xsgmii port's corresonding mac port id */
                        ptr_sdk_port_map_entry->mac_port = ptr_mac_port_map_entry->mac_port_id;

                        /* check for external CPU port */
                        if ((ptr_port_map_entry->xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_CPU))
                        {
                            /* update air cpu port information (external) */
                            HAL_CPU_PORT(unit) = ptr_port_map_entry->port;
                        }
                        if ((ptr_port_map_entry->xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_COMBO))
                        {
                            /* update air combo port bitmap (external) */
                            AIR_PORT_ADD(HAL_PORT_BMP_COMBO(unit), (ptr_port_map_entry->port));
                        }
                        if ((ptr_port_map_entry->xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_CASCADE))
                        {
                            /* update air combo port bitmap (external) */
                            AIR_PORT_ADD(HAL_PORT_BMP_CASCADE(unit), (ptr_port_map_entry->port));
                        }
                        if ((ptr_port_map_entry->xsgmii_port.flags & AIR_INIT_XSGMII_PORT_FLAGS_POE))
                        {
                            /* update air poe port bitmap */
                            AIR_PORT_ADD(HAL_PORT_BMP_POE(unit), (ptr_port_map_entry->port));
                        }

                        /* update related port bitmap */
                        AIR_PORT_ADD(HAL_PORT_BMP(unit), (ptr_port_map_entry->port));
                        AIR_PORT_ADD(HAL_PORT_BMP_TOTAL(unit), (ptr_port_map_entry->port));
                        AIR_PORT_ADD(HAL_PORT_BMP_XSGMII(unit), (ptr_port_map_entry->port));

                        /* break out for-loop */
                        break;
                    }
                }

                if (ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_CPU)
                {
                    /* found this xsgmii port's corresonding mac port id */
                    ptr_sdk_port_map_entry->mac_port = ptr_mac_port_map_entry->mac_port_id;

                    /* update air cpu port information (internal) */
                    HAL_CPU_PORT(unit) = ptr_port_map_entry->port;

                    /* update related port bitmap */
                    AIR_PORT_ADD(HAL_PORT_BMP_TOTAL(unit), (ptr_port_map_entry->port));

                    /* break out for-loop */
                    break;
                }
            }

            /* move to next mac port map entry */
            ptr_mac_port_map_entry++;
        }

        if (idx != (HAL_CHIP_MAC_PORT_INFO_PTR(unit)->mac_port_cnt))
        {
            /* has found mac port id for this sdk port (air port) */

            /* update mac port to air port mapping */
            HAL_MAC_PORT_MAP_INFO(unit, ptr_sdk_port_map_entry->mac_port) = ptr_port_map_entry->port;

            /* set this entry to be valid */
            ptr_sdk_port_map_entry->valid = 1;
        }
        else
        {
            /* doesn't find mac port id for this sdk port (air port) */
            rc = AIR_E_ENTRY_NOT_FOUND;
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, no found mac port for sdk port=%u, rc=%d\n", unit, ptr_port_map_entry->port,
                       rc);
        }
    }

    return rc;
}

/* help function to reset sdk port(air port) attributes and all port bitmaps */
static void
_hal_init_resetSdkPortInfo(
    const UI32_T unit)
{
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    UI32_T              idx;

    /* reset sdk port entries */
    for (idx = 0; idx < AIR_PORT_NUM; idx++)
    {
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, idx));

        if (ptr_sdk_port_map_entry->valid)
        {
            ptr_sdk_port_map_entry->valid = 0;
            ptr_sdk_port_map_entry->port_type = AIR_INIT_PORT_TYPE_LAST;
            ptr_sdk_port_map_entry->max_speed = AIR_INIT_PORT_SPEED_LAST;
            ptr_sdk_port_map_entry->mac_port = AIR_PORT_INVALID;
        }
    }

    /* reset cpu port */
    HAL_CPU_PORT(unit) = AIR_PORT_INVALID;

    /* reset related port bitmaps */
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_GPHY(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_XSGMII(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_COMBO(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_CASCADE(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_FORCE_MCR(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_POE(unit));
    AIR_PORT_BITMAP_CLEAR(HAL_PORT_BMP_TOTAL(unit));

    /* reset related reverse port mapping */
    if (NULL != PTR_HAL_EXT_CHIP_INFO(unit)->ptr_mac_port_map_info)
    {
        osal_memset(PTR_HAL_EXT_CHIP_INFO(unit)->ptr_mac_port_map_info, 0xFF, (AIR_PORT_NUM * sizeof(UI32_T)));
    }

    return;
}

#ifdef AIR_EN_POE
static AIR_ERROR_NO_T
_hal_init_checkPoePortParam(
    const UI32_T                   unit,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map_entry)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    const HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, (ptr_port_map_entry->port)));

    /* check if air port is valid */
    if (!ptr_sdk_port_map_entry->valid)
    {
        rc = AIR_E_NOT_INITED;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, sdk port not inited=%u, rc=%d\n", unit, ptr_port_map_entry->port, rc);
        return rc;
    }

    /* check if air port has poe function */
    if (!CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_POE(unit), ptr_port_map_entry->port))
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, sdk port without poe function=%u, rc=%d\n", unit, ptr_port_map_entry->port, rc);
        return rc;
    }

    /* check if air port's poe type already assigned */
    if (AIR_INIT_POE_PORT_TYPE_LAST != ptr_sdk_port_map_entry->poe_port_info.poe_type)
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, sdk port already assigned=%u, rc=%d\n", unit, ptr_port_map_entry->port, rc);
        return rc;
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_init_updatePoePortInfo(
    const UI32_T                   unit,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map_entry)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;

    ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, (ptr_port_map_entry->port)));

    /* update sdk port attributes */
    ptr_sdk_port_map_entry->poe_port_info.poe_type = ptr_port_map_entry->port_type;

    if (AIR_INIT_POE_PORT_TYPE_AF_AT == ptr_sdk_port_map_entry->poe_port_info.poe_type)
    {
        /* update sdk air port's poe information */
        ptr_sdk_port_map_entry->poe_port_info.primary_port.poe_device_idx =
            ptr_port_map_entry->primary_port.poe_device_idx;
        ptr_sdk_port_map_entry->poe_port_info.primary_port.hw_package_id =
            ptr_port_map_entry->primary_port.hw_package_id;

        /* update poe port to air port mapping */
        HAL_POE_PORT_MAP_INFO(unit, ptr_port_map_entry->primary_port.poe_device_idx,
                              ptr_port_map_entry->primary_port.hw_package_id) = ptr_port_map_entry->port;
    }
    else if (AIR_INIT_POE_PORT_TYPE_BT == ptr_sdk_port_map_entry->poe_port_info.poe_type)
    {
        /* update sdk air port's poe information */
        ptr_sdk_port_map_entry->poe_port_info.primary_port.poe_device_idx =
            ptr_port_map_entry->primary_port.poe_device_idx;
        ptr_sdk_port_map_entry->poe_port_info.primary_port.hw_package_id =
            ptr_port_map_entry->primary_port.hw_package_id;

        ptr_sdk_port_map_entry->poe_port_info.secondary_port.poe_device_idx =
            ptr_port_map_entry->secondary_port.poe_device_idx;
        ptr_sdk_port_map_entry->poe_port_info.secondary_port.hw_package_id =
            ptr_port_map_entry->secondary_port.hw_package_id;

        /* update poe port to air port mapping */
        HAL_POE_PORT_MAP_INFO(unit, ptr_port_map_entry->primary_port.poe_device_idx,
                              ptr_port_map_entry->primary_port.hw_package_id) = ptr_port_map_entry->port;

        HAL_POE_PORT_MAP_INFO(unit, ptr_port_map_entry->secondary_port.poe_device_idx,
                              ptr_port_map_entry->secondary_port.hw_package_id) = ptr_port_map_entry->port;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid poe port type=%d, rc=%d\n", unit, ptr_port_map_entry->port_type, rc);
    }

    return rc;
}

static void
_hal_init_resetPoePortInfo(
    const UI32_T unit)
{
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_entry;
    UI32_T              idx = 0;

    /* reset poe port entries */
    for (idx = 0; idx < AIR_PORT_NUM; idx++)
    {
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, idx));

        if (ptr_sdk_port_map_entry->valid)
        {
            ptr_sdk_port_map_entry->poe_port_info.poe_type = AIR_INIT_POE_PORT_TYPE_LAST;
        }
    }

    /* reset poe device's poe port to air port mapping */
    for (idx = 0; idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; idx++)
    {
        /* destory chip control block */
        if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, idx))
        {
            /* free poe port to air port map information memory */
            if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, idx)->ptr_poe_port_map_info)
            {
                osal_memset(PTR_HAL_POE_CB_CONTEXT(unit, idx)->ptr_poe_port_map_info, 0xFF,
                            (AIR_POE_MAXIMUM_PORTS_PER_DEVICE * sizeof(UI32_T)));
            }
        }
    }

    return;
}

static AIR_ERROR_NO_T
_hal_init_checkPoeDeviceParam(
    const UI32_T                     unit,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* check if duplicated poe device */
    if (HAL_POE_DEVICE_BMP(unit) & (1 << (ptr_device_map_entry->poe_device_idx)))
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, duplicated poe device=%u, rc=%d\n", unit, ptr_device_map_entry->poe_device_idx,
                   rc);
        return rc;
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_init_updatePoeDeviceInfo(
    const UI32_T                     unit,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_POE_CB_T  *ptr_poe_cb = NULL;

    /* allocate memory for this poe chip control block */
    ptr_poe_cb = (HAL_POE_CB_T *)osal_alloc(sizeof(HAL_POE_CB_T), air_module_getModuleName(AIR_MODULE_POE));
    if (NULL == ptr_poe_cb)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc mem size=%zu failed\n", unit, sizeof(HAL_POE_CB_T));
        return AIR_E_NO_MEMORY;
    }

    /* initialize poe control block as zero */
    osal_memset(ptr_poe_cb, 0x00, sizeof(HAL_POE_CB_T));

    /* assign poe control block pointer to chip info */
    PTR_HAL_POE_CB_CONTEXT(unit, ptr_device_map_entry->poe_device_idx) = ptr_poe_cb;

    /* update poe device chip info */
    HAL_POE_DEVICE_I2C_BUS_ID(unit, ptr_device_map_entry->poe_device_idx) = ptr_device_map_entry->i2c_bus_id;
    HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, ptr_device_map_entry->poe_device_idx) = ptr_device_map_entry->i2c_address;

    /* update poe device bitmap */
    HAL_POE_DEVICE_BMP(unit) |= (1 << ptr_device_map_entry->poe_device_idx);

    /* allocate memory for this poe chip poe mapping */
    PTR_HAL_POE_CB_CONTEXT(unit, ptr_device_map_entry->poe_device_idx)->ptr_poe_port_map_info =
        osal_alloc(AIR_POE_MAXIMUM_PORTS_PER_DEVICE * sizeof(UI32_T), air_module_getModuleName(AIR_MODULE_POE));

    if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, ptr_device_map_entry->poe_device_idx)->ptr_poe_port_map_info)
    {
        osal_memset(PTR_HAL_POE_CB_CONTEXT(unit, ptr_device_map_entry->poe_device_idx)->ptr_poe_port_map_info, 0xFF,
                    (AIR_POE_MAXIMUM_PORTS_PER_DEVICE * sizeof(UI32_T)));
    }
    else
    {
        rc = AIR_E_NO_MEMORY;
    }

    return rc;
}

static void
_hal_init_resetPoeDeviceInfo(
    const UI32_T unit)
{
    UI8_T device_idx = 0;

    for (device_idx = 0; device_idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device_idx++)
    {
        /* destory chip control block */
        if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, device_idx))
        {
            /* free poe port to air port map information memory */
            if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, device_idx)->ptr_poe_port_map_info)
            {
                osal_free(PTR_HAL_POE_CB_CONTEXT(unit, device_idx)->ptr_poe_port_map_info);
            }

            /* free the allocated memory */
            osal_free(PTR_HAL_POE_CB_CONTEXT(unit, device_idx));
            PTR_HAL_POE_CB_CONTEXT(unit, device_idx) = NULL;
        }
    }

    /* clear poe device bitmap */
    HAL_POE_DEVICE_BMP(unit) = 0;
    return;
}
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_init_initSdkPortMap
 * PURPOSE:
 *      This API is used to initialize SDK port mapping. All SDK API's port
 *      is based on those ports initialized by this API.
 * INPUT:
 *      unit                -- The unit ID
 *      port_map_cnt        -- The port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initSdkPortMap(
    const UI32_T               unit,
    const UI32_T               port_map_cnt,
    const AIR_INIT_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     idx;
    const AIR_INIT_PORT_MAP_T *ptr_port_map_entry;

    ptr_port_map_entry = ptr_port_map;

    for (idx = 0; idx < port_map_cnt; idx++)
    {
        /* check entry parameters  */
        rc = _hal_init_checkSdkPortParam(unit, ptr_port_map_entry);

        if (AIR_E_OK == rc)
        {
            /* update sdk port information */
            rc = _hal_init_updateSdkPortInfo(unit, ptr_port_map_entry);

            if (AIR_E_OK != rc)
            {
                /* update fail, break out for-loop */
                break;
            }
        }
        else
        {
            /* check parameter fail, break out for-loop */
            break;
        }

        ptr_port_map_entry++;
    }

    if (AIR_E_OK != rc)
    {
        /* clean previous entry configuration */
        _hal_init_resetSdkPortInfo(unit);

        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init sdk port map fail, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   hal_init_getSdkPortMap
 * PURPOSE:
 *      This API is used to get SDK port mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt    -- The pointer of port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_port_map
 *      parameter to get the port map.
 */
AIR_ERROR_NO_T
hal_init_getSdkPortMap(
    const UI32_T         unit,
    UI32_T              *ptr_port_map_cnt,
    AIR_INIT_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               port;
    UI8_T                count = 0;
    AIR_INIT_PORT_MAP_T *ptr_port_map_entry;
    HAL_SDK_PORT_MAP_T  *ptr_sdk_port_map_entry;

    ptr_port_map_entry = ptr_port_map;

    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

        ptr_port_map_entry->port = port;
        ptr_port_map_entry->port_type = ptr_sdk_port_map_entry->port_type;
        ptr_port_map_entry->max_speed = ptr_sdk_port_map_entry->max_speed;

        switch (ptr_port_map_entry->port_type)
        {
            case AIR_INIT_PORT_TYPE_BASET:
                ptr_port_map_entry->baset_port.phy_pkg_id = ptr_sdk_port_map_entry->baset_port.phy_pkg_id;
                break;
            case AIR_INIT_PORT_TYPE_ENHANCED_BASET:
                ptr_port_map_entry->enhanced_baset_port.flags = ptr_sdk_port_map_entry->enhanced_baset_port.flags;
                ptr_port_map_entry->enhanced_baset_port.phy_pkg_id =
                    ptr_sdk_port_map_entry->enhanced_baset_port.phy_pkg_id;
                break;
            case AIR_INIT_PORT_TYPE_XSGMII:
                ptr_port_map_entry->xsgmii_port.flags = ptr_sdk_port_map_entry->xsgmii_port.flags;
                ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id = ptr_sdk_port_map_entry->xsgmii_port.xsgmii_pkg_id;
                ptr_port_map_entry->xsgmii_port.phy_pkg_id = ptr_sdk_port_map_entry->xsgmii_port.phy_pkg_id;
                break;
            case AIR_INIT_PORT_TYPE_CPU:
                /* do nothing */
                break;
            default:
                rc = AIR_E_BAD_PARAMETER;
                DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid port type=%d, rc=%d\n", unit, ptr_port_map_entry->port_type, rc);
                break;
        }

        ptr_port_map_entry++;
        count++;
    }

    (*ptr_port_map_cnt) = count;

    return rc;
}

#ifdef AIR_EN_POE
/* FUNCTION NAME:   hal_init_initPoePortMap
 * PURPOSE:
 *      This API is used to initialize PoE port mapping.
 * INPUT:
 *      unit                -- The unit ID
 *      port_map_cnt        -- The port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Parameter is wrong.
 *      AIR_E_NOT_INITED    -- SDK port is not initialized.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initPoePortMap(
    const UI32_T                   unit,
    const UI32_T                   port_map_cnt,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T                 rc = AIR_E_OK;
    UI32_T                         idx;
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map_entry;

    ptr_port_map_entry = ptr_port_map;

    for (idx = 0; idx < port_map_cnt; idx++)
    {
        if (unit == ptr_port_map_entry->unit)
        {
            /* check entry parameters  */
            rc = _hal_init_checkPoePortParam(unit, ptr_port_map_entry);

            if (AIR_E_OK == rc)
            {
                /* update poe port information */
                rc = _hal_init_updatePoePortInfo(unit, ptr_port_map_entry);

                if (AIR_E_OK != rc)
                {
                    /* update fail, break out for-loop */
                    break;
                }
            }
            else
            {
                /* check parameter fail, break out for-loop */
                break;
            }
        }
        ptr_port_map_entry++;
    }

    if (AIR_E_OK != rc)
    {
        /* clean previous entry configuration */
        _hal_init_resetPoePortInfo(unit);

        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init poe port map fail, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   hal_init_getPoePortMap
 * PURPOSE:
 *      This API is used to get PoE port mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt    -- The pointer of port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_port_map
 *      parameter to get the port map.
 */
AIR_ERROR_NO_T
hal_init_getPoePortMap(
    const UI32_T             unit,
    UI32_T                  *ptr_port_map_cnt,
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   port = 0;
    UI8_T                    count = 0;
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map_entry;
    HAL_SDK_PORT_MAP_T      *ptr_sdk_port_map_entry;

    ptr_port_map_entry = ptr_port_map;

    AIR_PORT_FOREACH(HAL_PORT_BMP_POE(unit), port)
    {
        ptr_sdk_port_map_entry = &(HAL_SDK_PORT_MAP_INFO(unit, port));

        ptr_port_map_entry->unit = unit;
        ptr_port_map_entry->port = port;
        ptr_port_map_entry->port_type = ptr_sdk_port_map_entry->poe_port_info.poe_type;

        if (AIR_INIT_POE_PORT_TYPE_AF_AT == ptr_port_map_entry->port_type)
        {
            ptr_port_map_entry->primary_port.poe_device_idx =
                ptr_sdk_port_map_entry->poe_port_info.primary_port.poe_device_idx;
            ptr_port_map_entry->primary_port.hw_package_id =
                ptr_sdk_port_map_entry->poe_port_info.primary_port.hw_package_id;
        }
        else if (AIR_INIT_POE_PORT_TYPE_BT == ptr_port_map_entry->port_type)
        {
            ptr_port_map_entry->primary_port.poe_device_idx =
                ptr_sdk_port_map_entry->poe_port_info.primary_port.poe_device_idx;
            ptr_port_map_entry->primary_port.hw_package_id =
                ptr_sdk_port_map_entry->poe_port_info.primary_port.hw_package_id;
            ptr_port_map_entry->secondary_port.poe_device_idx =
                ptr_sdk_port_map_entry->poe_port_info.secondary_port.poe_device_idx;
            ptr_port_map_entry->secondary_port.hw_package_id =
                ptr_sdk_port_map_entry->poe_port_info.secondary_port.hw_package_id;
        }
        else
        {
            rc = AIR_E_BAD_PARAMETER;
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid port=%u type=%d, rc=%d\n", unit, ptr_port_map_entry->port,
                       ptr_port_map_entry->port_type, rc);
            break;
        }

        ptr_port_map_entry++;
        count++;
    }

    (*ptr_port_map_cnt) = count;

    return rc;
}

/* FUNCTION NAME:   hal_init_initPoeDeviceMap
 * PURPOSE:
 *      This API is used to initialize PoE device mapping.
 * INPUT:
 *      unit                -- The unit ID
 *      device_map_cnt      -- The device mapping count
 *      ptr_device_map      -- The pointer of device mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Parameter is wrong.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initPoeDeviceMap(
    const UI32_T                     unit,
    const UI32_T                     device_map_cnt,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map)
{
    AIR_ERROR_NO_T                   rc = AIR_E_OK;
    UI32_T                           idx;
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map_entry;

    ptr_device_map_entry = ptr_device_map;

    for (idx = 0; idx < device_map_cnt; idx++)
    {
        if (unit == ptr_device_map_entry->unit)
        {
            /* check entry parameters  */
            rc = _hal_init_checkPoeDeviceParam(unit, ptr_device_map_entry);

            if (AIR_E_OK == rc)
            {
                /* update poe port information */
                rc = _hal_init_updatePoeDeviceInfo(unit, ptr_device_map_entry);

                if (AIR_E_OK != rc)
                {
                    /* update fail, break out for-loop */
                    break;
                }
            }
            else
            {
                /* check parameter fail, break out for-loop */
                break;
            }
        }
        ptr_device_map_entry++;
    }

    if (AIR_E_OK != rc)
    {
        /* clean previous entry configuration */
        _hal_init_resetPoeDeviceInfo(unit);

        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init poe device map fail, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   hal_init_getPoeDeviceMap
 * PURPOSE:
 *      This API is used to get PoE device mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_device_map_cnt  -- The pointer of device mapping count
 *      ptr_device_map      -- The pointer of device mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_device_map
 *      parameter to get the device map.
 */
AIR_ERROR_NO_T
hal_init_getPoeDeviceMap(
    const UI32_T               unit,
    UI32_T                    *ptr_device_map_cnt,
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     device = 0, count = 0;
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map_entry;

    ptr_device_map_entry = ptr_device_map;

    for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
    {
        if (HAL_POE_DEVICE_BMP(unit) & (1 << (device)))
        {
            ptr_device_map_entry->unit = unit;
            ptr_device_map_entry->poe_device_idx = device;
            ptr_device_map_entry->i2c_bus_id = HAL_POE_DEVICE_I2C_BUS_ID(unit, device);
            ptr_device_map_entry->i2c_address = HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, device);

            ptr_device_map_entry++;
            count++;
        }
    }

    (*ptr_device_map_cnt) = count;

    return rc;
}

#endif
