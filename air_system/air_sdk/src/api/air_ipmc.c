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

/* FILE NAME:  air_ipmc.c
 * PURPOSE:
 *  Implement the IP multicast module AIR layer APIs.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <air_ipmc.h>

#include <api/diag.h>
#include <hal/common/hal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_IPMC, "air_ipmc.c");

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   air_ipmc_setMcastLookupType
 * PURPOSE:
 *      This API is used to set multicast lookup type.
 * INPUT:
 *      unit                     -- Device unit number
 *      type                     -- The multicast lookup type
 *                                  AIR_IPMC_TYPE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_setMcastLookupType(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_IPMC_TYPE_LAST);

    return HAL_FUNC_CALL(unit, ipmc, setMcastLookupType, (unit, type));
}

/* FUNCTION NAME:   air_ipmc_getMcastLookupType
 * PURPOSE:
 *      This API is used to get multicast lookup type.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      ptr_type                 -- The multicast lookup type
 *                                  AIR_IPMC_TYPE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastLookupType(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_type);

    return HAL_FUNC_CALL(unit, ipmc, getMcastLookupType, (unit, ptr_type));
}

/* FUNCTION NAME:   air_ipmc_addMcastAddr
 * PURPOSE:
 *      This API is used to add a multicast MAC address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_addMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, addMcastAddr, (unit, (AIR_IPMC_ENTRY_T *)ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_getMcastAddr
 * PURPOSE:
 *      This API is used to get a multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast key
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      ptr_entry                -- The multicast info
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastAddr(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, getMcastAddr, (unit, ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_delMcastAddr
 * PURPOSE:
 *      This API is used to delete a multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast key
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, delMcastAddr, (unit, ptr_entry));
}

/* FUNCTION NAME: air_ipmc_delAllMcastAddr
 * PURPOSE:
 *      Delete all multicast address entries.
 * INPUT:
 *      unit                     -- Select device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delAllMcastAddr(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);

    return HAL_FUNC_CALL(unit, ipmc, delAllMcastAddr, (unit));
}

/* FUNCTION NAME:   air_ipmc_addMcastMember
 * PURPOSE:
 *      This API is used to add member for a multicast ID.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_addMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, addMcastMember, (unit, ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_delMcastMember
 * PURPOSE:
 *      This API is used to delete member for a multicast ID.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, delMcastMember, (unit, ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_getMcastMemberCnt
 * PURPOSE:
 *      This API is used to get member count for a multicast ID.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      ptr_count                -- The member count
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastMemberCnt(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_count)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    HAL_CHECK_PTR(ptr_count);
    HAL_CHECK_VLAN(ptr_entry->vid);

    return HAL_FUNC_CALL(unit, ipmc, getMcastMemberCnt, (unit, ptr_entry, ptr_count));
}

/* FUNCTION NAME:   air_ipmc_getMcastBucketSize
 * PURPOSE:
 *      Get the bucket size of one multicast address set when searching
 *      multicast.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_size                 -- The bucket size
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_size);

    return HAL_FUNC_CALL(unit, ipmc, getMcastBucketSize, (unit, ptr_size));
}

/* FUNCTION NAME:   air_ipmc_getFirstMcastAddr
 * PURPOSE:
 *      This API is used to get the first multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      match_type               -- The type to search multicast entry.
 *                                  AIR_IPMC_MATCH_TYPE_T
 * OUTPUT:
 *      ptr_entry_cnt            -- The number of multicast address
 *                                  entries.
 *      ptr_entry                -- The multicast entry
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getFirstMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(match_type, AIR_IPMC_MATCH_TYPE_LAST);
    HAL_CHECK_PTR(ptr_entry_cnt);
    HAL_CHECK_PTR(ptr_entry);

    return HAL_FUNC_CALL(unit, ipmc, getFirstMcastAddr, (unit, match_type, ptr_entry_cnt, ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_getNextMcastAddr
 * PURPOSE:
 *      This API is used to get next multicast address entry.
 * INPUT:
 *      unit                     -- Select device ID
 *      match_type               -- The type to search multicast entry.
 *                                  AIR_IPMC_MATCH_TYPE_T
 * OUTPUT:
 *      ptr_entry_cnt            -- The number of returned multicast
 *                                  entries.
 *      ptr_entry                -- The multicast searching result.
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getNextMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(match_type, AIR_IPMC_MATCH_TYPE_LAST);
    HAL_CHECK_PTR(ptr_entry_cnt);
    HAL_CHECK_PTR(ptr_entry);

    return HAL_FUNC_CALL(unit, ipmc, getNextMcastAddr, (unit, match_type, ptr_entry_cnt, ptr_entry));
}

/* FUNCTION NAME:   air_ipmc_setPortIpmcMode
 * PURPOSE:
 *      This API is used to set IPMC mode.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      enable                   -- IPMC mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_setPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, ipmc, setPortIpmcMode, (unit, port, enable));
}

/* FUNCTION NAME:   air_ipmc_getPortIpmcMode
 * PURPOSE:
 *      This API is used to get IPMC mode.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 * OUTPUT:
 *      ptr_enable               -- IPMC mode
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_OTHERS             -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, ipmc, getPortIpmcMode, (unit, port, ptr_enable));
}

/* FUNCTION NAME:   air_ipmc_setPortLookupIpTypeCtrl
 * PURPOSE:
 *      This API is used to set which type of IP packets will be
 *      treated as IP Multicast for lookup purposes.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      type                     -- IP family type
 *                                  AIR_IP_TYPE_T
 *      enable                   -- Treated as IPMC or not
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_setPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    const BOOL_T        enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(ip_type, AIR_IP_TYPE_LAST);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, ipmc, setPortLookupIpTypeCtrl, (unit, port, ip_type, enable));
}

/* FUNCTION NAME:   air_ipmc_getPortLookupIpTypeCtrl
 * PURPOSE:
 *      This API is used to get which type of IP packets will be
 *      treated as IP Multicast for lookup purposes.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      type                     -- IP family type
 *                                  AIR_IP_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- Treated as IPMC or not
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_OTHERS             -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    BOOL_T             *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(ip_type, AIR_IP_TYPE_LAST);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, ipmc, getPortLookupIpTypeCtrl, (unit, port, ip_type, ptr_enable));
}
