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

/* FILE NAME:  air_l2.h
 * PURPOSE:
 *      It provides L2 module API.
 *
 * NOTES:
 */

#ifndef AIR_L2_H
#define AIR_L2_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_L2_MAC_MAX_AGE_OUT_TIME (1000000)
#define AIR_L2_MAC_DEF_AGE_OUT_TIME (0xFFFFFFFF)

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_MAC_ADDR_LEN       (6)
#define AIR_MAX_ETHER_TYPE_VAL (0xFFFF)

/* Field for Forward type */
typedef enum
{
    AIR_FORWARD_TYPE_BCST,
    AIR_FORWARD_TYPE_MCST,
    AIR_FORWARD_TYPE_UMCST = AIR_FORWARD_TYPE_MCST,
    AIR_FORWARD_TYPE_UCST,
    AIR_FORWARD_TYPE_UUCST = AIR_FORWARD_TYPE_UCST,
    AIR_FORWARD_TYPE_UIPMCST,
    AIR_FORWARD_TYPE_LAST
} AIR_FORWARD_TYPE_T;

/* Field for Forward action */
typedef enum
{
    AIR_FORWARD_ACTION_DROP,
    AIR_FORWARD_ACTION_FLOODING,
    AIR_FORWARD_ACTION_TO_PBM,
    AIR_FORWARD_ACTION_LAST
} AIR_FORWARD_ACTION_T;

/* Field for MAC entry forward control when hit source MAC */
typedef enum
{
    AIR_L2_FWD_CTRL_DEFAULT,
    AIR_L2_FWD_CTRL_CPU_INCLUDE,
    AIR_L2_FWD_CTRL_CPU_EXCLUDE,
    AIR_L2_FWD_CTRL_CPU_ONLY,
    AIR_L2_FWD_CTRL_DROP,
    AIR_L2_FWD_CTRL_LAST
} AIR_L2_FWD_CTRL_T;

/* DATA TYPE DECLARATIONS
 */
/* Entry structure of MAC table */
typedef struct AIR_MAC_ENTRY_S
{
    /* L2 MAC entry keys */
    /* MAC Address */
    AIR_MAC_T mac;

    /* Customer VID (12bits) */
    UI16_T    cvid;

    /* Filter ID */
    UI16_T    fid;

    /* l2 mac entry attributes */
#define AIR_L2_MAC_ENTRY_FLAGS_IVL                        (1U << 0)
#define AIR_L2_MAC_ENTRY_FLAGS_STATIC                     (1U << 1)
#define AIR_L2_MAC_ENTRY_FLAGS_UNAUTH                     (1U << 2)
#define AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER (1U << 3)
    UI32_T            flags;

    /* Destination Port Map */
    AIR_PORT_BITMAP_T port_bitmap;

    /* Source MAC address hit forward control */
    AIR_L2_FWD_CTRL_T sa_fwd;

    /* Age Timer only for getting information */
    UI32_T            timer;
} AIR_MAC_ENTRY_T;

/* MAC table flush type */
typedef enum
{
    AIR_L2_MAC_FLUSH_TYPE_VID,
    AIR_L2_MAC_FLUSH_TYPE_FID,
    AIR_L2_MAC_FLUSH_TYPE_PORT,
    AIR_L2_MAC_FLUSH_TYPE_LAST,
} AIR_L2_MAC_FLUSH_TYPE_T;

/* MAC table access type */
typedef enum
{
    AIR_L2_MAC_SEARCH_TYPE_VID,
    AIR_L2_MAC_SEARCH_TYPE_FID,
    AIR_L2_MAC_SEARCH_TYPE_PORT,
    AIR_L2_MAC_SEARCH_TYPE_LAST,
} AIR_L2_MAC_SEARCH_TYPE_T;

#ifdef AIR_EN_L2_SHADOW
typedef enum
{
    AIR_L2_MAC_NOTIFY_REASON_ADD = 0,
    AIR_L2_MAC_NOTIFY_REASON_MODIFY,
    AIR_L2_MAC_NOTIFY_REASON_DELETE,
    AIR_L2_MAC_NOTIFY_REASON_LAST,
} AIR_L2_MAC_NOTIFY_REASON_T;

typedef AIR_ERROR_NO_T (*AIR_L2_TRAVERSE_FUNC_T)(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry,
    void                  *ptr_cookie);

typedef void (*AIR_L2_MAC_NOTIFY_FUNC_T)(
    const UI32_T                     unit,
    const AIR_L2_MAC_NOTIFY_REASON_T reason,
    const AIR_MAC_ENTRY_T           *ptr_entry,
    void                            *ptr_cookie);
#endif /* End of AIR_EN_L2_SHADOW */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const AIR_MAC_ENTRY_T *ptr_mac_entry);

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
    const AIR_MAC_ENTRY_T *ptr_mac_entry);

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
    AIR_MAC_ENTRY_T *ptr_mac_entry);

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
    AIR_MAC_ENTRY_T *ptr_mac_entry);

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
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

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
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

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
    const UI32_T unit);

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
    const UI32_T                  value);

/* FUNCTION NAME:   air_l2_setMacAddrAgeOut
 * PURPOSE:
 *      Set the age out time of L2 MAC address entries.
 * INPUT:
 *      unit                     -- Device ID
 *      age_time                 -- Age out time (second)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      age_time is 1 to AIR_L2_MAC_MAX_AGE_OUT_TIME.
 */
AIR_ERROR_NO_T
air_l2_setMacAddrAgeOut(
    const UI32_T unit,
    const UI32_T age_time);

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
    UI32_T      *ptr_age_time);

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
    const BOOL_T enable);

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
    BOOL_T      *ptr_enable);

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
    UI32_T      *ptr_size);

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
    const AIR_PORT_BITMAP_T    port_bitmap);

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
    AIR_PORT_BITMAP_T        port_bitmap);

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
    void                        *ptr_cookie);

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
    void                          *ptr_cookie);

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
    void                          *ptr_cookie);

#endif /* End of AIR_EN_L2_SHADOW */
#endif /* End of AIR_L2_H */
