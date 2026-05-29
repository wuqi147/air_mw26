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

/* FILE NAME:  air_l2.c
 * PURPOSE:
 *    It provide L2 module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_l2.h>

#include <air_error.h>
#include <air_stp.h>
#include <air_types.h>
#include <hal/common/hal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_L2, "air_l2.c");
/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   air_l2_addMacAddr
 * PURPOSE:
 *      Add or set a L2 unicast MAC address entry.
 *      If the address entry doesn't exist, it will add the entry.
 *      If the address entry already exists, it will set the entry
 *      with user input value.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_mac_entry            -- Structure of MAC address entry
 *                                  AIR_MAC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TABLE_FULL         -- Table is full.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_addMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, addMacAddr, (unit, ptr_mac_entry));
}

/* FUNCTION NAME:   air_l2_delMacAddr
 * PURPOSE:
 *      Delete a L2 unicast MAC address entry.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_mac_entry            -- Structure of MAC address entry
 *                                  AIR_MAC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_delMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, delMacAddr, (unit, ptr_mac_entry));
}

/* FUNCTION NAME:   air_l2_getMacAddr
 * PURPOSE:
 *      Get a L2 unicast MAC address entry.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_mac_entry            -- Pointer of the structure of MAC
 *                                  Address table
 *                                  AIR_MAC_ENTRY_T
 * OUTPUT:
 *      ptr_count                -- The number of returned MAC entries
 *      ptr_mac_entry            -- Structure of MAC Address table for
 *                                  searching result.
 *                                  The size of ptr_mac_entry depends
 *                                  on the maximun number of bank.
 *                                  The memory size should greater than
 *                                  ((# of Bank) * (Size of entry
 *                                  structure))
 *                                  AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is
 *      empty. It means to search the first valid MAC address entry
 *      in MAC address table. Otherwise, to search the specific MAC
 *      address entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and
 *      (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
air_l2_getMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_count);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, getMacAddr, (unit, ptr_count, ptr_mac_entry));
}

/* FUNCTION NAME:   air_l2_getNextMacAddr
 * PURPOSE:
 *      Get the next L2 unicast MAC address entries.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_mac_entry            -- Structure of MAC Address table
 *                                  AIR_MAC_ENTRY_T
 * OUTPUT:
 *      ptr_count                -- The number of returned MAC entries
 *      ptr_mac_entry            -- Structure of MAC Address table for
 *                                  searching result.
 *                                  The size of ptr_mac_entry depends
 *                                  on the max. number of bank.
 *                                  The memory size should greater than
 *                                  ((# of Bank) * (Size of entry
 *                                  structure))
 *                                  AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is
 *      empty. It means to search the next valid MAC address entries
 *      of last searching result. Otherwise, to search the next valid
 *      MAC address entry of the specific MAC address entry in input
 *      argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and
 *      (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
air_l2_getNextMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_count);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, getNextMacAddr, (unit, ptr_count, ptr_mac_entry));
}

/* FUNCTION NAME:  air_l2_searchMacAddr
 * PURPOSE:
 *      Search L2 unicast MAC address entries by filter.
 * INPUT:
 *      unit                     --  Device ID
 *      ptr_mac_entry            --  The structure of MAC Address table
 *                                   AIR_MAC_ENTRY_T
 *      type                     --  The type of searching by specific
 *                                   filter
 *                                   AIR_L2_MAC_SEARCH_TYPE_T
 *      value                    --  The value of filter
 * OUTPUT:
 *      ptr_count                --  The number of returned MAC entries
 *      ptr_mac_entry            --  Structure of MAC Address table for
 *                                   searching result. The size of
 *                                   ptr_mac_entry depends on the max.
 *                                   number of bank.
 *                                   The memory size should greater than
 *                                   ((# of Bank) * (Size of entry
 *                                   structure))
 *                                   AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      air_l2_searchNextMacAddr(). It should not be used
 *      interchangeably with air_l2_getMacAddr()/air_l2_getNextMacAddr()
 *      before the search is finished, otherwise the results obtained
 *      will noe meet expectations.
 */
AIR_ERROR_NO_T
air_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_L2_MAC_SEARCH_TYPE_LAST);
    if (AIR_L2_MAC_SEARCH_TYPE_PORT == type)
    {
        HAL_CHECK_PORT(unit, value);
    }
    else if (AIR_L2_MAC_SEARCH_TYPE_VID == type)
    {
        HAL_CHECK_VLAN(value);
    }
    else if (AIR_L2_MAC_SEARCH_TYPE_FID == type)
    {
        HAL_CHECK_MIN_MAX_RANGE(value, 0, (AIR_STP_FID_NUMBER - 1));
    }
    HAL_CHECK_PTR(ptr_count);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, searchMacAddr, (unit, type, value, ptr_count, ptr_mac_entry));
}

/* FUNCTION NAME: air_l2_searchNextMacAddr
 * PURPOSE:
 *      Search the next L2 unicast MAC address entries by filter.
 * INPUT:
 *      unit                     --  Device ID
 *      ptr_mac_entry            --  The structure of MAC Address table
 *                                   AIR_MAC_ENTRY_T
 *      type                     --  The type of searching by specific
 *                                   filter
 *                                   AIR_L2_MAC_SEARCH_TYPE_T
 *      value                    --  The value of filter
 * OUTPUT:
 *      ptr_count                --  The number of returned MAC entries
 *      ptr_mac_entry            --  Structure of MAC Address table for
 *                                   searching result. The size of
 *                                   ptr_mac_entry depends on the
 *                                   max. number of bank.
 *                                   The memory size should greater than
 *                                   ((# of Bank) * (Size of entry
 *                                   structure))
 *                                   AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      air_l2_searchMacAddr(). It should not be used interchangeably
 *      with air_l2_getMacAddr()/air_l2_getNextMacAddr() before the
 *      search is finished, otherwise the results obtained will not
 *      meet expectations.
 */
AIR_ERROR_NO_T
air_l2_searchNextMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_L2_MAC_SEARCH_TYPE_LAST);
    if (AIR_L2_MAC_SEARCH_TYPE_PORT == type)
    {
        HAL_CHECK_PORT(unit, value);
    }
    else if (AIR_L2_MAC_SEARCH_TYPE_VID == type)
    {
        HAL_CHECK_VLAN(value);
    }
    else if (AIR_L2_MAC_SEARCH_TYPE_FID == type)
    {
        HAL_CHECK_MIN_MAX_RANGE(value, 0, (AIR_STP_FID_NUMBER - 1));
    }
    HAL_CHECK_PTR(ptr_count);
    HAL_CHECK_PTR(ptr_mac_entry);
    return HAL_FUNC_CALL(unit, l2, searchNextMacAddr, (unit, type, value, ptr_count, ptr_mac_entry));
}

/* FUNCTION NAME:   air_l2_clearMacAddr
 * PURPOSE:
 *      Clear all L2 unicast MAC address entries.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_clearMacAddr(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    return HAL_FUNC_CALL(unit, l2, clearMacAddr, (unit));
}

/* FUNCTION NAME:   air_l2_flushMacAddr
 * PURPOSE:
 *      Flush all L2 unicast MAC address entries by vid, by fid or by
 *      port.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Flush l2 mac address by
 *                                  vid/fid/port.
 *                                  AIR_L2_MAC_FLUSH_TYPE_T
 *      value                    -- Value of vid/fid/port
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_flushMacAddr(
    const UI32_T                  unit,
    const AIR_L2_MAC_FLUSH_TYPE_T type,
    const UI32_T                  value)
{
    HAL_CHECK_UNIT(unit);
    /* Mistake proofing, fid and port parameters check in Hal_sco_l2 */
    HAL_CHECK_ENUM_RANGE(type, AIR_L2_MAC_FLUSH_TYPE_LAST);
    if (AIR_L2_MAC_FLUSH_TYPE_PORT == type)
    {
        HAL_CHECK_PORT(unit, value);
    }
    else if (AIR_L2_MAC_FLUSH_TYPE_VID == type)
    {
        HAL_CHECK_VLAN(value);
    }
    else if (AIR_L2_MAC_FLUSH_TYPE_FID == type)
    {
        HAL_CHECK_MIN_MAX_RANGE(value, 0, (AIR_STP_FID_NUMBER - 1));
    }
    return HAL_FUNC_CALL(unit, l2, flushMacAddr, (unit, type, value));
}

/* FUNCTION NAME:   air_l2_setMacAddrAgeOut
 * PURPOSE:
 *      Set the age out time of L2 MAC address entries.
 * INPUT:
 *      unit                     -- Device ID
 *      age_time                 -- Age out time (second)
 *                                  (1..AIR_L2_MAC_MAX_AGE_OUT_TIME)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_setMacAddrAgeOut(
    const UI32_T unit,
    const UI32_T age_time)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_MIN_MAX_RANGE(age_time, 1, AIR_L2_MAC_MAX_AGE_OUT_TIME);

    return HAL_FUNC_CALL(unit, l2, setMacAddrAgeOut, (unit, age_time));
}

/* FUNCTION NAME:   air_l2_getMacAddrAgeOut
 * PURPOSE:
 *      Get the age out time of unicast MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_age_time             -- age out time
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_getMacAddrAgeOut(
    const UI32_T unit,
    UI32_T      *ptr_age_time)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_age_time);

    return HAL_FUNC_CALL(unit, l2, getMacAddrAgeOut, (unit, ptr_age_time));
}

/* FUNCTION NAME:   air_l2_setMacAddrAgeOutMode
 * PURPOSE:
 *      Set the age out mode for specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- TRUE
 *                                  FALSE
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_setMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_BOOL(enable);

    return HAL_FUNC_CALL(unit, l2, setMacAddrAgeOutMode, (unit, port, enable));
}

/* FUNCTION NAME:   air_l2_getMacAddrAgeOutMode
 * PURPOSE:
 *      Get the age out mode for specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- TRUE
 *                                  FALSE
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_getMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, l2, getMacAddrAgeOutMode, (unit, port, ptr_enable));
}

/* FUNCTION NAME:   air_l2_getMacBucketSize
 * PURPOSE:
 *      Get the bucket size of one MAC address set when searching L2
 *      table.
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
air_l2_getMacBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_num)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_num);

    return HAL_FUNC_CALL(unit, l2, getMacBucketSize, (unit, ptr_num));
}

/* FUNCTION NAME:   air_l2_setForwardMode
 * PURPOSE:
 *      Set per port forwarding status for unknown type frame.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Frame type
 *                                  AIR_FORWARD_TYPE_T
 *      action                   -- Forwarding action
 *                                  AIR_FORWARD_ACTION_T
 *      port_bitmap              -- Forwarding port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_setForwardMode(
    const UI32_T               unit,
    const AIR_FORWARD_TYPE_T   type,
    const AIR_FORWARD_ACTION_T action,
    const AIR_PORT_BITMAP_T    port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_FORWARD_TYPE_LAST);
    HAL_CHECK_ENUM_RANGE(action, AIR_FORWARD_ACTION_LAST);
    HAL_CHECK_PORT_BITMAP(unit, port_bitmap);
    return HAL_FUNC_CALL(unit, l2, setForwardMode, (unit, type, action, port_bitmap));
}

/* FUNCTION NAME:   air_l2_getForwardMode
 * PURPOSE:
 *      Get per port forwarding status for unknown type frame.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Frame type
 *                                  AIR_FORWARD_TYPE_T
 * OUTPUT:
 *      ptr_action               -- Forwarding action
 *                                  AIR_FORWARD_ACTION_T
 *      port_bitmap              -- Forwarding port bitmap
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_getForwardMode(
    const UI32_T             unit,
    const AIR_FORWARD_TYPE_T type,
    AIR_FORWARD_ACTION_T    *ptr_action,
    AIR_PORT_BITMAP_T        port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_FORWARD_TYPE_LAST);
    HAL_CHECK_PTR(ptr_action);
    HAL_CHECK_PTR(port_bitmap);
    return HAL_FUNC_CALL(unit, l2, getForwardMode, (unit, type, ptr_action, port_bitmap));
}

#ifdef AIR_EN_L2_SHADOW
/* FUNCTION NAME: air_l2_traverseMacAddr
 * PURPOSE:
 *      Traverse all L2 unicast address and handle the address entry by
 *      user's callback.
 * INPUT:
 *      unit                     -- Device unit number
 *      callback                 -- Callback function
 *      ptr_cookie               -- The cookie data as input parameter
 *                                  of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER      -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_traverseMacAddr(
    const UI32_T                 unit,
    const AIR_L2_TRAVERSE_FUNC_T callback,
    void                        *ptr_cookie)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(callback);
    return HAL_FUNC_CALL(unit, l2, traverseMacAddr, (unit, callback, ptr_cookie));
}

/* FUNCTION NAME: air_l2_registerMacAddrNotifyCallback
 * PURPOSE:
 *      Register a callback routine that will be called whenever an
 *      entry is inserted into or deleted from L2 address table.
 * INPUT:
 *      unit                     -- Device unit number
 *      notify_func              -- Callback function
 *      ptr_cookie               -- The cookie data as input parameter
 *                                  of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER      -- Bad parameter.
 *      AIR_E_TABLE_FULL         -- Table is full.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_registerMacAddrNotifyCallback(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(notify_func);
    return HAL_FUNC_CALL(unit, l2, registerMacAddrNotifyCallback, (unit, notify_func, ptr_cookie));
}

/* FUNCTION NAME: air_l2_deregisterMacAddrNotifyCallback
 * PURPOSE:
 *      Deregister a callback routine that will be called whenever an
 *      entry is inserted into or deleted from L2 address table.
 * INPUT:
 *      unit                     -- Device unit number
 *      notify_func              -- Callback function
 *      ptr_cookie               -- The cookie data as input parameter
 *                                  of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER      -- Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_deregisterMacAddrNotifyCallback(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(notify_func);
    return HAL_FUNC_CALL(unit, l2, deregisterMacAddrNotifyCallback, (unit, notify_func, ptr_cookie));
}

#endif /* End of AIR_EN_L2_SHADOW */
