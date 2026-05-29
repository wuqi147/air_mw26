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

/* FILE NAME:  hal_coral_l2.h
 * PURPOSE:
 *  Define L2 module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_L2_H
#define HAL_CORAL_L2_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_l2.h>
#include <air_swc.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_L2_MAX_BUSY_TIME (200)
#define HAL_CORAL_L2_MAX_AGE_CNT   (BITS(0, (AAC_AGE_CNT_LENGTH - 1)))
#define HAL_CORAL_L2_MAX_AGE_UNIT  (BITS(0, (AAC_AGE_UNIT_LENGTH - 1)))
#define HAL_CORAL_L2_MAC_SET_NUM   (4)
#define HAL_CORAL_L2_MAX_ADDR_NUM  (1024)
#define HAL_CORAL_L2_MAX_ENTRY_NUM (HAL_CORAL_L2_MAC_SET_NUM * HAL_CORAL_L2_MAX_ADDR_NUM)

typedef enum
{
    HAL_CORAL_L2_FDB_FULL_NONE = 0,
    HAL_CORAL_L2_FDB_FULL_HW_REPLACE_OLDEST_DYNAMIC_ENTRY,
    HAL_CORAL_L2_FDB_FULL_LAST
} HAL_CORAL_L2_FDB_FULL_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_L2_IS_UC_ADDR(__ptr_mac__) ((1 == (((__ptr_mac__)[0]) & 0x1)) ? 0 : 1)
#define HAL_CORAL_L2_FDB_LOCK(__unit__)      hal_coral_l2_lockL2FdbResource(__unit__)
#define HAL_CORAL_L2_FDB_UNLOCK(__unit__)    hal_coral_l2_unlockL2FdbResource(__unit__)

/* DATA TYPE DECLARATIONS
 */
typedef struct HAL_CORAL_L2_FDB_CB_S
{
    AIR_SEMAPHORE_ID_T hw_tbl_mutex;
} HAL_CORAL_L2_FDB_CB_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_coral_l2_init
 * PURPOSE:
 *      Initialization of L2 MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_l2_deinit
 * PURPOSE:
 *      Deinitialization of L2 MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_l2_addMacAddr
 * PURPOSE:
 *      Add or set a L2 unicast MAC address entry.
 *      If the address entry doesn't exist, it will add the entry.
 *      If the address entry already exists, it will set the entry
 *      with user input value.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TABLE_FULL
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_addMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: hal_coral_l2_delMacAddr
 * PURPOSE:
 *      Delete a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_delMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME:  hal_coral_l2_getMacAddr
 * PURPOSE:
 *      Get a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than ((# of Bank) * (Size of entry structure))
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is empty.
 *      It means to search the first valid MAC address entry in MAC address table.
 *      Otherwise, to search the specific MAC address entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
hal_coral_l2_getMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: hal_coral_l2_getNextMacAddr
 * PURPOSE:
 *      Get the next L2 unicast MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than ((# of Bank) * (Table size))
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is empty.
 *      It means to search the next valid MAC address entries of last searching result.
 *      Otherwise, to search the next valid MAC address entry of the specific MAC address
 *      entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
hal_coral_l2_getNextMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: hal_coral_l2_clearMacAddr
 * PURPOSE:
 *      Clear all L2 unicast MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_clearMacAddr(
    const UI32_T unit);

/* FUNCTION NAME:  hal_coral_l2_searchMacAddr
 * PURPOSE:
 *      Search L2 unicast MAC address entries by filter.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *                          AIR_MAC_ENTRY_T
 *      type            --  The type of searching by specific filter
 *                          AIR_L2_MAC_SEARCH_TYPE_T
 *      value           --  The value of filter
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 *                          AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      hal_coral_l2_searchNextMacAddr(). It should not be used interchangeably
 *      with hal_coral_l2_getMacAddr()/hal_coral_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_coral_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

/* FUNCTION NAME: hal_coral_l2_searchNextMacAddr
 * PURPOSE:
 *      Search the next L2 unicast MAC address entries by filter.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *                          AIR_MAC_ENTRY_T
 *      type            --  The type of searching by specific filter
 *                          AIR_L2_MAC_SEARCH_TYPE_T
 *      value           --  The value of filter
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 *                          AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      hal_coral_l2_searchMacAddr(). It should not be used interchangeably
 *      with hal_coral_l2_getMacAddr()/hal_coral_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_coral_l2_searchNextMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

/* FUNCTION NAME: hal_coral_l2_flushMacAddr
 * PURPOSE:
 *      Flush all L2 unicast MAC address entries by vid, by fid or by port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      type            --  Flush l2 mac address by vid/fid/port
 *      value           --  value of vid/fid/port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_flushMacAddr(
    const UI32_T                  unit,
    const AIR_L2_MAC_FLUSH_TYPE_T type,
    const UI32_T                  value);

/* FUNCTION NAME: hal_coral_l2_setMacAddrAgeOut
 * PURPOSE:
 *      Set the age out time of L2 MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *      age_time        --  age out time (second)
 *                          (1..AIR_L2_MAC_MAX_AGE_OUT_TIME)
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_setMacAddrAgeOut(
    const UI32_T unit,
    const UI32_T age_time);

/* FUNCTION NAME: hal_coral_l2_getMacAddrAgeOut
 * PURPOSE:
 *      Get the age out time of unicast MAC address.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_age_time    --  age out time
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getMacAddrAgeOut(
    const UI32_T unit,
    UI32_T      *ptr_age_time);

/* FUNCTION NAME: hal_coral_l2_setMacAddrAgeOutMode
 * PURPOSE:
 *      Set the age out mode for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      enable          --  TRUE:   Enable L2 MAC table aging out.
 *                          FALSE:  Disable L2 MAC table aging out.
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_setMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME: hal_coral_l2_getMacAddrAgeOutMode
 * PURPOSE:
 *      Get the age out mode for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_enable      --  TRUE:   Enable L2 MAC table aging out.
 *                          FALSE:  Disable L2 MAC table aging out.
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_coral_l2_getMacBucketSize
 * PURPOSE:
 *      Get the bucket size of one MAC address set when searching L2 table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_size        --  The bucket size
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getMacBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size);

/* FUNCTION NAME: hal_coral_l2_setForwardMode
 * PURPOSE:
 *      Set per port forwarding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  AIR_FORWARD_TYPE_BCST
 *                          AIR_FORWARD_TYPE_MCST
 *                          AIR_FORWARD_TYPE_UCST
 *                          AIR_FORWARD_TYPE_UIPMCST
 *      action          --  AIR_FORWARD_DROP
 *                          AIR_FORWARD_FLOODING
 *                          AIR_FORWARD_TO_PBM
 *      port_bitmap     --  Forwarding port bitmap
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_setForwardMode(
    const UI32_T               unit,
    const AIR_FORWARD_TYPE_T   type,
    const AIR_FORWARD_ACTION_T action,
    const AIR_PORT_BITMAP_T    port_bitmap);

/* FUNCTION NAME: hal_coral_l2_getForwardMode
 * PURPOSE:
 *      Get per port forwarding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  AIR_FORWARD_TYPE_BCST
 *                          AIR_FORWARD_TYPE_MCST
 *                          AIR_FORWARD_TYPE_UCST
 *                          AIR_FORWARD_TYPE_UIPMCST
 * OUTPUT:
 *      ptr_action      --  AIR_FORWARD_DROP
 *                          AIR_FORWARD_FLOODING
 *                          AIR_FORWARD_TO_PBM
 *      port_bitmap     --  Forwarding port bitmap
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getForwardMode(
    const UI32_T             unit,
    const AIR_FORWARD_TYPE_T type,
    AIR_FORWARD_ACTION_T    *ptr_action,
    AIR_PORT_BITMAP_T        port_bitmap);

/* FUNCTION NAME: hal_coral_l2_lockL2FdbResource
 *
 * PURPOSE:
 *      Lock the resource of L2 table.
 * INPUT:
 *      unit            --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_l2_lockL2FdbResource(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_l2_unlockL2FdbResource
 *
 * PURPOSE:
 *      Unlock the resource of L2 table.
 * INPUT:
 *      unit            --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_l2_unlockL2FdbResource(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_l2_getCapacity
 * PURPOSE:
 *      Get the l2 resource capacity
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_coral_l2_getUsage
 * PURPOSE:
 *      Get the l2 resource usage
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_cnt         --  Count of usage
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_l2_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* end of HAL_CORAL_L2_H */
