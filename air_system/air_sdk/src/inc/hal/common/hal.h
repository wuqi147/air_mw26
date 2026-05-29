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

/* FILE NAME:  hal.h
 * PURPOSE:
 *  1. Provide whole HAL resource initialization API.
 *  2. Provide HAL per-unit initialization and de-initialization function
 *     APIs.
 *  3. Provide HAL database access APIs.
 *  4. Provide a HAL multiplexing function vector.
 *
 * NOTES:
 */

#ifndef HAL_H
#define HAL_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_error.h>
#include <air_module.h>
#include <air_port.h>
#include <air_types.h>
#include <api/diag.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_dev.h>
#include <hal/common/hal_drv.h>
#include <hal/common/hal_poe.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_DEBUG                (1)
#define HAL_INVALID_ID           (AIR_INVALID_ID)
#define HAL_ENABLE_RESOURCE_LOCK (1)

/* MACRO FUNCTION DECLARATIONS
 */
#define PTR_HAL_FUNC_VECTOR(__unit__)   _ext_ptr_chip_func_vector[__unit__]
#define PTR_HAL_EXT_CHIP_INFO(__unit__) _ext_ptr_chip_info[__unit__]

/* hal related check macros */
#define HAL_IS_UNIT_VALID(__unit__)                     \
    (((__unit__) < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM) && \
     (NULL != _ext_ptr_chip_func_vector[(__unit__)]))

#define HAL_IS_POE_UNIT_VALID(__unit__, __device_idx__)                      \
    (((__device_idx__) < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP) &&            \
     (NULL != PTR_HAL_POE_CB_CONTEXT((__unit__), (__device_idx__))) &&       \
     (NULL != PTR_HAL_POE_CB_CONTEXT((__unit__), (__device_idx__))->driver))

#define HAL_IS_VLAN_VALID(__vlan__)             \
    (((__vlan__) >= 1) && ((__vlan__) <= 4095))

#define HAL_IS_PORT_VALID(__unit__, __port__)                            \
    (((__port__) < AIR_PORT_NUM) &&                                      \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_TOTAL((__unit__)), (__port__))))

#define HAL_IS_ETH_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                    \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_ETH((__unit__)), (__port__))))

#define HAL_IS_GPHY_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                     \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_GPHY((__unit__)), (__port__))))

#define HAL_IS_XSGMII_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                       \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_XSGMII((__unit__)), (__port__))))

#define HAL_IS_COMBO_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                      \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_COMBO((__unit__)), (__port__))))

#define HAL_IS_CASCADE_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                        \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_CASCADE((__unit__)), (__port__))))

#define HAL_IS_FORCE_MCR_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                          \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_FORCE_MCR((__unit__)), (__port__))))

#define HAL_IS_POE_PORT_VALID(__unit__, __port__)                      \
    (((__port__) < AIR_PORT_NUM) &&                                    \
     (CMLIB_BITMAP_BIT_CHK(HAL_PORT_BMP_POE((__unit__)), (__port__))))

#define HAL_CHECK_ERROR(__rc__)                              \
    do                                                       \
    {                                                        \
        AIR_ERROR_NO_T __rc = (__rc__);                      \
        if (__rc != AIR_E_OK)                                \
        {                                                    \
            DIAG_PRINT(HAL_DBG_WARN, #__rc__ "=%d\n", __rc); \
            return __rc;                                     \
        }                                                    \
    } while (0)

#define HAL_CHECK_UNIT(__unit__)            \
    do                                      \
    {                                       \
        if (!HAL_IS_UNIT_VALID((__unit__))) \
        {                                   \
            return AIR_E_BAD_PARAMETER;     \
        }                                   \
    } while (0)

#define HAL_CHECK_POE_UNIT(__unit__, __device__)              \
    do                                                        \
    {                                                         \
        if (!HAL_IS_POE_UNIT_VALID((__unit__), (__device__))) \
        {                                                     \
            return AIR_E_NOT_SUPPORT;                         \
        }                                                     \
    } while (0)

#define HAL_CHECK_VLAN(__vlan_id__)                                 \
    do                                                              \
    {                                                               \
        if (!HAL_IS_VLAN_VALID((__vlan_id__)))                      \
        {                                                           \
            DIAG_PRINT(HAL_DBG_WARN, "invalid " #__vlan_id__ "=%u," \
                                     " range=1-4095, rc=%d\n",      \
                       __vlan_id__, AIR_E_BAD_PARAMETER);           \
            return AIR_E_BAD_PARAMETER;                             \
        }                                                           \
    } while (0)

#define HAL_CHECK_PORT(__unit__, __port__)                             \
    do                                                                 \
    {                                                                  \
        if (!HAL_IS_PORT_VALID((__unit__), (__port__)))                \
        {                                                              \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid port=%u, rc=%d\n", \
                       __unit__, __port__, AIR_E_BAD_PARAMETER);       \
            return AIR_E_BAD_PARAMETER;                                \
        }                                                              \
    } while (0)

#define HAL_CHECK_ETH_PORT(__unit__, __port__)                             \
    do                                                                     \
    {                                                                      \
        if (!HAL_IS_ETH_PORT_VALID((__unit__), (__port__)))                \
        {                                                                  \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid eth port=%u, rc=%d\n", \
                       __unit__, __port__, AIR_E_BAD_PARAMETER);           \
            return AIR_E_BAD_PARAMETER;                                    \
        }                                                                  \
    } while (0)

#define HAL_CHECK_GPHY_PORT(__unit__, __port__)                             \
    do                                                                      \
    {                                                                       \
        if (!HAL_IS_GPHY_PORT_VALID((__unit__), (__port__)))                \
        {                                                                   \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid gphy port=%u, rc=%d\n", \
                       __unit__, __port__, AIR_E_BAD_PARAMETER);            \
            return AIR_E_BAD_PARAMETER;                                     \
        }                                                                   \
    } while (0)

#define HAL_CHECK_XSGMII_PORT(__unit__, __port__)                    \
    do                                                               \
    {                                                                \
        if (!HAL_IS_XSGMII_PORT_VALID((__unit__), (__port__)))       \
        {                                                            \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid xsgmii port=%u," \
                                     " rc=%d\n",                     \
                       __unit__, __port__, AIR_E_BAD_PARAMETER);     \
            return AIR_E_BAD_PARAMETER;                              \
        }                                                            \
    } while (0)

#define HAL_CHECK_POE_PORT(__unit__, __port__)                             \
    do                                                                     \
    {                                                                      \
        if (!HAL_IS_POE_PORT_VALID((__unit__), (__port__)))                \
        {                                                                  \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid poe port=%u, rc=%d\n", \
                       __unit__, __port__, AIR_E_NOT_SUPPORT);             \
            return AIR_E_NOT_SUPPORT;                                      \
        }                                                                  \
    } while (0)

#define HAL_CHECK_PORT_BITMAP(__unit__, __port_bitmap__)                   \
    do                                                                     \
    {                                                                      \
        AIR_PORT_BITMAP_T __bitmap__;                                      \
                                                                           \
        CMLIB_PORT_BITMAP_INV(__bitmap__, HAL_PORT_BMP_TOTAL((__unit__))); \
        CMLIB_PORT_BITMAP_AND(__bitmap__, (__port_bitmap__));              \
                                                                           \
        if (!AIR_PORT_BITMAP_EMPTY(__bitmap__))                            \
        {                                                                  \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid port bitmap, rc=%d\n", \
                       __unit__, AIR_E_BAD_PARAMETER);                     \
            return AIR_E_BAD_PARAMETER;                                    \
        }                                                                  \
    } while (0)

#define HAL_CHECK_ETH_PORT_BITMAP(__unit__, __port_bitmap__)             \
    do                                                                   \
    {                                                                    \
        AIR_PORT_BITMAP_T __bitmap__;                                    \
                                                                         \
        CMLIB_PORT_BITMAP_INV(__bitmap__, HAL_PORT_BMP_ETH((__unit__))); \
        CMLIB_PORT_BITMAP_AND(__bitmap__, (__port_bitmap__));            \
                                                                         \
        if (!AIR_PORT_BITMAP_EMPTY(__bitmap__))                          \
        {                                                                \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid eth port bitmap,"    \
                                     " rc=%d\n",                         \
                       __unit__, AIR_E_BAD_PARAMETER);                   \
            return AIR_E_BAD_PARAMETER;                                  \
        }                                                                \
    } while (0)

#define HAL_CHECK_GPHY_PORT_BITMAP(__unit__, __port_bitmap__)             \
    do                                                                    \
    {                                                                     \
        AIR_PORT_BITMAP_T __bitmap__;                                     \
                                                                          \
        CMLIB_PORT_BITMAP_INV(__bitmap__, HAL_PORT_BMP_GPHY((__unit__))); \
        CMLIB_PORT_BITMAP_AND(__bitmap__, (__port_bitmap__));             \
                                                                          \
        if (!AIR_PORT_BITMAP_EMPTY(__bitmap__))                           \
        {                                                                 \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid gphy port bitmap, "   \
                                     "rc=%d\n",                           \
                       __unit__, AIR_E_BAD_PARAMETER);                    \
            return AIR_E_BAD_PARAMETER;                                   \
        }                                                                 \
    } while (0)

#define HAL_CHECK_XSGMII_PORT_BITMAP(__unit__, __port_bitmap__)             \
    do                                                                      \
    {                                                                       \
        AIR_PORT_BITMAP_T __bitmap__;                                       \
                                                                            \
        CMLIB_PORT_BITMAP_INV(__bitmap__, HAL_PORT_BMP_XSGMII((__unit__))); \
        CMLIB_PORT_BITMAP_AND(__bitmap__, (__port_bitmap__));               \
                                                                            \
        if (!AIR_PORT_BITMAP_EMPTY(__bitmap__))                             \
        {                                                                   \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid xsgmii port bitmap, "   \
                                     "rc=%d\n",                             \
                       __unit__, AIR_E_BAD_PARAMETER);                      \
            return AIR_E_BAD_PARAMETER;                                     \
        }                                                                   \
    } while (0)

#define HAL_CHECK_PTR(__ptr__)          \
    do                                  \
    {                                   \
        if (NULL == (__ptr__))          \
        {                               \
            return AIR_E_BAD_PARAMETER; \
        }                               \
    } while (0)

#define HAL_CHECK_PARAM(expr, errCode) \
    do                                 \
    {                                  \
        if ((I32_T)(expr))             \
        {                              \
            return errCode;            \
        }                              \
    } while (0)

#define HAL_CHECK_ENUM_RANGE(__value__, __max__) \
    do                                           \
    {                                            \
        if ((__value__) >= (__max__))            \
        {                                        \
            return AIR_E_BAD_PARAMETER;          \
        }                                        \
    } while (0)

#define HAL_CHECK_MIN_MAX_RANGE(__value__, __min__, __max__)          \
    do                                                                \
    {                                                                 \
        if (0 == (__min__))                                           \
        {                                                             \
            if ((__value__) > (__max__))                              \
            {                                                         \
                DIAG_PRINT(HAL_DBG_WARN, "invalid " #__value__ "=%u," \
                                         " range=0-%u, rc=%d\n",      \
                           __value__, __max__,                        \
                           AIR_E_BAD_PARAMETER);                      \
                return AIR_E_BAD_PARAMETER;                           \
            }                                                         \
        }                                                             \
        else                                                          \
        {                                                             \
            if (((__value__) > (__max__)) ||                          \
                ((__value__) < (__min__)))                            \
            {                                                         \
                DIAG_PRINT(HAL_DBG_WARN, "invalid " #__value__ "=%u," \
                                         " range=%u-%u, rc=%d\n",     \
                           __value__, __min__, __max__,               \
                           AIR_E_BAD_PARAMETER);                      \
                return AIR_E_BAD_PARAMETER;                           \
            }                                                         \
        }                                                             \
    } while (0)

#define HAL_CHECK_MIN_MAX_RANGE_SIGNED(__value__, __min__, __max__) \
    do                                                              \
    {                                                               \
        if (((__value__) > (__max__)) ||                            \
            ((__value__) < (__min__)))                              \
        {                                                           \
            DIAG_PRINT(HAL_DBG_WARN, "invalid " #__value__ "=%d,"   \
                                     " range=(%d)-(%d), rc=%d\n",   \
                       __value__, __min__, __max__,                 \
                       AIR_E_BAD_PARAMETER);                        \
            return AIR_E_BAD_PARAMETER;                             \
        }                                                           \
    } while (0)

#define HAL_CHECK_BOOL(__value__)                                          \
    do                                                                     \
    {                                                                      \
        if (((FALSE) != (__value__)) &&                                    \
            ((TRUE) != (__value__)))                                       \
        {                                                                  \
            DIAG_PRINT(HAL_DBG_WARN, #__value__ "=%u isn't bool, rc=%d\n", \
                       __value__, AIR_E_BAD_PARAMETER);                    \
            return AIR_E_BAD_PARAMETER;                                    \
        }                                                                  \
    } while (0)

#define HAL_CHECK_INIT(__unit__, __module_id__)                           \
    do                                                                    \
    {                                                                     \
        if (HAL_INIT_STAGE_NONE ==                                        \
            HAL_MODULE_INITED((__unit__), (__module_id__)))               \
        {                                                                 \
            DIAG_PRINT(HAL_DBG_WARN, "u=%u, %s module isn't inited, "     \
                                     "rc=%d\n",                           \
                       __unit__, air_module_getModuleName(__module_id__), \
                       AIR_E_NOT_INITED);                                 \
            return AIR_E_NOT_INITED;                                      \
        }                                                                 \
    } while (0)

#define HAL_INIT_STAGE(__unit__)                 _ext_chip_control_block[__unit__].init_stage
#define HAL_MODULE_INFO(__unit__, __module_id__) _ext_chip_control_block[__unit__].ptr_module_info[__module_id__]
#define HAL_CHIP_MAC_PORT_INFO_PTR(__unit__)     _ext_chip_control_block[__unit__].ptr_chip_mac_port_info
#define HAL_MODULE_INITED(__unit__, __module__)  (HAL_MODULE_INFO(__unit__, __module__).inited)

#define HAL_FUNC_CALL(__unit__, __module__, __func__, __param__) ({                                             \
    AIR_ERROR_NO_T __rc = AIR_E_OK;                                                                             \
    if ((NULL == PTR_HAL_FUNC_VECTOR(__unit__)->__module__##_func_vec) ||                                       \
        (NULL == PTR_HAL_FUNC_VECTOR(__unit__)->__module__##_func_vec->hal_##__module__##_##__func__))          \
    {                                                                                                           \
        __rc = AIR_E_NOT_SUPPORT;                                                                               \
    }                                                                                                           \
    else                                                                                                        \
    {                                                                                                           \
        __rc = (PTR_HAL_FUNC_VECTOR(__unit__)->__module__##_func_vec->hal_##__module__##_##__func__ __param__); \
    }                                                                                                           \
    __rc;                                                                                                       \
})

/* Macros for chip related information */
#define HAL_DEVICE_FAMILY_ID(__unit__) PTR_HAL_EXT_CHIP_INFO(__unit__)->family_id
#define HAL_DEVICE_CHIP_ID(__unit__)   PTR_HAL_EXT_CHIP_INFO(__unit__)->device_id
#define HAL_DEVICE_REV_ID(__unit__)    PTR_HAL_EXT_CHIP_INFO(__unit__)->revision_id
#define HAL_CPU_PORT(__unit__)         PTR_HAL_EXT_CHIP_INFO(__unit__)->cpu_port
#define HAL_TOTAL_PORT_NUM(__unit__)   PTR_HAL_EXT_CHIP_INFO(__unit__)->total_port
#define HAL_PORT_BMP(__unit__)         PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap
#define HAL_PORT_BMP_ETH(__unit__)     PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap

#define HAL_PORT_BMP_GPHY(__unit__)      PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_gphy
#define HAL_PORT_BMP_XSGMII(__unit__)    PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_xsgmii
#define HAL_PORT_BMP_COMBO(__unit__)     PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_combo
#define HAL_PORT_BMP_CASCADE(__unit__)   PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_cascade
#define HAL_PORT_BMP_POE(__unit__)       PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_poe
#define HAL_PORT_BMP_TOTAL(__unit__)     PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_total
#define HAL_PORT_BMP_FORCE_MCR(__unit__) PTR_HAL_EXT_CHIP_INFO(__unit__)->port_bitmap_force_mcr

#define HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__)                    \
    PTR_HAL_EXT_CHIP_INFO(__unit__)->ptr_sdk_port_map_info[__air_port__]
#define HAL_MAC_PORT_MAP_INFO(__unit__, __mac_port__)                    \
    PTR_HAL_EXT_CHIP_INFO(__unit__)->ptr_mac_port_map_info[__mac_port__]

/* Macros for PoE chip related information */
#define HAL_SDK_PORT_MAP_POE_INFO(__unit__, __air_port__)       \
    HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).poe_port_info
#define HAL_POE_DEVICE_BMP(__unit__)                  PTR_HAL_EXT_CHIP_INFO(__unit__)->poe_deivce_bitmap
#define PTR_HAL_POE_CB_CONTEXT(__unit__, __dev_idx__) PTR_HAL_EXT_CHIP_INFO(__unit__)->ptr_poe_cb[__dev_idx__]

/* Macros for AIR port related attributes */
#define HAL_AIR_PORT_TO_MAC_PORT(__unit__, __air_port__, __mac_port__)         \
    do                                                                         \
    {                                                                          \
        __mac_port__ = HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).mac_port; \
    } while (0)

#define HAL_MAC_PORT_TO_AIR_PORT(__unit__, __mac_port__, __air_port__) \
    do                                                                 \
    {                                                                  \
        __air_port__ = HAL_MAC_PORT_MAP_INFO(__unit__, __mac_port__);  \
    } while (0)

#define HAL_AIR_PORT_TO_SERDES_ID(__unit__, __air_port__, __serdes_id__)                         \
    do                                                                                           \
    {                                                                                            \
        __serdes_id__ = HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).xsgmii_port.xsgmii_pkg_id; \
    } while (0)

#define HAL_AIR_PORT_TO_PHY_PORT(__unit__, __air_port__, __phy_port__)                                         \
    do                                                                                                         \
    {                                                                                                          \
        if (HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).port_type == AIR_INIT_PORT_TYPE_BASET)               \
        {                                                                                                      \
            __phy_port__ = HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).baset_port.phy_pkg_id;                \
        }                                                                                                      \
        else if (HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET) \
        {                                                                                                      \
            __phy_port__ = HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).enhanced_baset_port.phy_pkg_id;       \
        }                                                                                                      \
        else                                                                                                   \
        {                                                                                                      \
            __phy_port__ = HAL_SDK_PORT_MAP_INFO(__unit__, __air_port__).xsgmii_port.phy_pkg_id;               \
        }                                                                                                      \
    } while (0)

#define HAL_AIR_PBMP_TO_MAC_PBMP(__unit__, __air_pbmp__, __mac_pbmp__) \
    do                                                                 \
    {                                                                  \
        UI32_T __air_p, __mac_p;                                       \
        AIR_PORT_BITMAP_CLEAR(__mac_pbmp__);                           \
        AIR_PORT_FOREACH(__air_pbmp__, __air_p)                        \
        {                                                              \
            HAL_AIR_PORT_TO_MAC_PORT(__unit__, __air_p, __mac_p);      \
            AIR_PORT_ADD(__mac_pbmp__, __mac_p);                       \
        }                                                              \
    } while (0)

#define HAL_MAC_PBMP_TO_AIR_PBMP(__unit__, __mac_pbmp__, __air_pbmp__) \
    do                                                                 \
    {                                                                  \
        UI32_T __air_p, __mac_p;                                       \
        AIR_PORT_BITMAP_CLEAR(__air_pbmp__);                           \
        AIR_PORT_FOREACH(__mac_pbmp__, __mac_p)                        \
        {                                                              \
            HAL_MAC_PORT_TO_AIR_PORT(__unit__, __mac_p, __air_p);      \
            if (0xFFFFFFFF != __air_p)                                 \
            {                                                          \
                AIR_PORT_ADD(__air_pbmp__, __air_p);                   \
            }                                                          \
        }                                                              \
    } while (0)

/* Macors for chip familys */
#define HAL_IS_DEVICE_SCO_FAMILY(__unit__)                     \
    (HAL_DEVICE_FAMILY_ID(__unit__) == HAL_CHIP_FAMILY_ID_SCO)

#define HAL_IS_DEVICE_PEARL_FAMILY(__unit__)                     \
    (HAL_DEVICE_FAMILY_ID(__unit__) == HAL_CHIP_FAMILY_ID_PEARL)

#define HAL_IS_DEVICE_CORAL_FAMILY(__unit__)                     \
    (HAL_DEVICE_FAMILY_ID(__unit__) == HAL_CHIP_FAMILY_ID_CORAL)

/* Resource management related definitions */
#ifdef HAL_ENABLE_RESOURCE_LOCK
#define HAL_COMMON_CREATE_LOCK_RESOURCE(ptr_sema_name, ptr_semaphore_id, module_name) \
    osal_createSemaphore(ptr_sema_name, 1, ptr_semaphore_id, module_name)
#define HAL_COMMON_DESTROY_LOCK_RESOURCE(ptr_semaphore_id) \
    osal_destroySemaphore(ptr_semaphore_id)
#define HAL_COMMON_LOCK_RESOURCE(ptr_sema, timeout) \
    osal_takeSemaphore((ptr_sema), (timeout))
#define HAL_COMMON_FREE_RESOURCE(ptr_sema) \
    osal_giveSemaphore((ptr_sema))
#else
#define HAL_COMMON_CREATE_LOCK_RESOURCE(ptr_sema_name, ptr_semaphore_id) AIR_E_OK
#define HAL_COMMON_DESTROY_LOCK_RESOURCE(ptr_semaphore_id)               AIR_E_OK
#define HAL_COMMON_LOCK_RESOURCE(ptr_sema, timeout)                      AIR_E_OK
#define HAL_COMMON_FREE_RESOURCE(ptr_sema)                               AIR_E_OK
#endif /* HAL_ENABLE_RESOURCE_LOCK */

/* DATA TYPE DECLARATIONS
 */
typedef struct HAL_POE_PORT_INFO_S
{
    AIR_INIT_POE_PORT_TYPE_T poe_type;
    AIR_INIT_POE_PORT_ALT_T  primary_port;
    AIR_INIT_POE_PORT_ALT_T  secondary_port;
} HAL_POE_PORT_INFO_T;

typedef struct HAL_SDK_PORT_MAP_S
{
    UI32_T                valid;
    AIR_INIT_PORT_TYPE_T  port_type;
    AIR_INIT_PORT_SPEED_T max_speed;
    union
    {
        AIR_INIT_BASET_PORT_T          baset_port;
        AIR_INIT_XSGMII_PORT_T         xsgmii_port;
        AIR_INIT_ENHANCED_BASET_PORT_T enhanced_baset_port;
    };
    /* attributes of this AIR port */
    UI32_T              mac_port;
    HAL_POE_PORT_INFO_T poe_port_info;
} HAL_SDK_PORT_MAP_T;

typedef struct
{
    UI32_T              family_id;             /* family ID                                */
    UI32_T              device_id;             /* device ID                                */
    UI32_T              revision_id;           /* revision ID                              */
    UI32_T              mode;                  /* Chip mode or FPGA mode                   */
    UI32_T              cpu_port;              /* cpu port id (air port view)              */
    UI32_T              total_port;            /* total port numbers(gphy + xsgmii + cpu)  */
    AIR_PORT_BITMAP_T   port_bitmap;           /* total active port bitmap(gphy + xsgmii)  */
    AIR_PORT_BITMAP_T   port_bitmap_gphy;      /* total active gphy port bitmap            */
    AIR_PORT_BITMAP_T   port_bitmap_xsgmii;    /* total active xsgmii(2.5G) port bitmap    */
    AIR_PORT_BITMAP_T   port_bitmap_combo;     /* total active combo port bitmap           */
    AIR_PORT_BITMAP_T   port_bitmap_cascade;   /* total active cascade port bitmap         */
    AIR_PORT_BITMAP_T   port_bitmap_poe;       /* total active poe port bitmap             */
    AIR_PORT_BITMAP_T   port_bitmap_total;     /* total port bitmap(gphy + xsgmii + cpu)   */
    AIR_PORT_BITMAP_T   port_bitmap_force_mcr; /* total used mcr force mode port bitmap    */
    HAL_SDK_PORT_MAP_T *ptr_sdk_port_map_info; /* sdk air port to chip port map info       */
    UI32_T             *ptr_mac_port_map_info; /* chip mac port to air port map info       */
    UI8_T               poe_deivce_bitmap;     /* total active poe device bitmap           */
    HAL_POE_CB_T       *ptr_poe_cb[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP];
} HAL_CHIP_INFO_T;

typedef enum
{
    HAL_INIT_STAGE_NONE = 0,
    HAL_INIT_STAGE_LOW_LEVEL = (0x1U << 0),
    HAL_INIT_STAGE_TASK_RSRC = (0x1U << 1),
    HAL_INIT_STAGE_MODULE = (0x1U << 2),
    HAL_INIT_STAGE_TASK = (0x1U << 3),
} HAL_INIT_STAGE_T;

typedef struct
{
    HAL_INIT_STAGE_T inited;
} HAL_MODULE_INFO_T;

typedef struct
{
    HAL_CHIP_INFO_T               *ptr_chip_info;          /* chip information pointer */
    HAL_DRIVER_T                  *ptr_driver_info;        /* chip driver information pointer */
    HAL_INIT_STAGE_T               init_stage;
    HAL_MODULE_INFO_T             *ptr_module_info;        /* module information pointer */
    const HAL_DRV_MAC_PORT_INFO_T *ptr_chip_mac_port_info; /* pointer of mac port information for this chip */
} HAL_CHIP_CB_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI32_T unit);

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
    const UI32_T unit);

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
    UI32_T *ptr_unit_num);

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
    UI32_T      *ptr_port_num);

/* FUNCTION NAME:   hal_dumpChipInfo
 * PURPOSE:
 *      hal_dumpChipInfo() is a function to dump chip information.
 *
 * INPUT:
 *      unit          -- The specified unit number.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Dump chip information successfully.
 *      AIR_E_OTHERS  -- Fail to dump chip information.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_dumpChipInfo(
    const UI32_T unit);

/* FUNCTION NAME:   hal_dumpDriverInfo
 * PURPOSE:
 *      hal_dumpDriverInfo() is a function to dump driver information.
 *
 * INPUT:
 *      unit          -- The specified unit number.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Dump driver information successfully.
 *      AIR_E_OTHERS  -- Fail to dump driver information.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_dumpDriverInfo(
    const UI32_T unit);

/* FUNCTION NAME:   hal_sema_lock
 * PURPOSE:
 *      hal_sema_lock() is responsible for lock specific table
 *
 * INPUT:
 *      unit                        -- The unit number.
 *      tbl_id                      -- The table id.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully read table.
 *      AIR_E_OTHERS  -- Fail to read table.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_sema_lock(
    const UI32_T unit,
    const UI32_T tbl_id);

/* FUNCTION NAME:   hal_sema_unlock
 * PURPOSE:
 *      hal_sema_unlock() is responsible for unlock specific table
 *
 * INPUT:
 *      unit                        -- The unit number.
 *      tbl_id                      -- The table id.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK      -- Successfully read table.
 *      AIR_E_OTHERS  -- Fail to read table.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_sema_unlock(
    const UI32_T unit,
    const UI32_T tbl_id);

/* FUNCTION NAME:   hal_dumpDb
 * PURPOSE:
 *      hal_dumpDb is used to dump hal swdb
 *
 * INPUT:
 *      unit                -- The chip unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operate success.
 *      AIR_E_BAD_PARAMETER -- Bad parameter.
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_dumpDb(
    const UI32_T unit,
    const UI32_T flags);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
 */

extern const HAL_FUNC_VEC_T
    *_ext_ptr_chip_func_vector[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

extern HAL_CHIP_INFO_T
    *_ext_ptr_chip_info[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

extern HAL_CHIP_CB_T
    _ext_chip_control_block[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

#endif /* #ifndef HAL_H */
