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

/* FILE NAME:  hal_coral_ipmc.h
 * PURPOSE:
 *  Define the declartion for IP multicast module.
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_IPMC_H
#define HAL_CORAL_IPMC_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_ipmc.h>
#include <air_swc.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_IPMC_MAX_BUSY_TIME (20)
#define HAL_CORAL_IPMC_DELAY_US      (1000)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   hal_coral_ipmc_setMcastLookupType
 * PURPOSE:
 *      This API is used to set multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *      type            --  The multicast type
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setMcastLookupType(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type);

/* FUNCTION NAME:   hal_coral_ipmc_getMcastLookupType
 * PURPOSE:
 *      This API is used to get multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *
 * OUTPUT:
 *      ptr_type        --  The multicast type
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastLookupType(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type);

/* FUNCTION NAME: hal_coral_ipmc_addMcastAddr
 * PURPOSE:
 *      Add or set a multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_EXISTS
 *
 * NOTES:
 *      1.Can't add group if the group address is exist.
 */
AIR_ERROR_NO_T
hal_coral_ipmc_addMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_getMcastAddr
 * PURPOSE:
 *      Get multicast address entry for specific group and source address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      ptr_entry       --  The multicast entries
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastAddr(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_delMcastAddr
 * PURPOSE:
 *      Delete multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      1.del group address will delete group address relate all source address.
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_delAllMcastAddr
 * PURPOSE:
 *      Delete all multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delAllMcastAddr(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ipmc_addMcastMember
 * PURPOSE:
 *      Add member for a specific multicast entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_addMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_delMcastMember
 * PURPOSE:
 *      Delete member for a specific multicast entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_getMcastMemberCnt
 * PURPOSE:
 *      Get member count for specific multicast group.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      ptr_count       --  Member count
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastMemberCnt(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_count);

/* FUNCTION NAME:   hal_coral_ipmc_getMcastBucketSize
 * PURPOSE:
 *      Get the bucket size of one multicast address set when searching multicast.
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
hal_coral_ipmc_getMcastBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size);

/* FUNCTION NAME:   hal_coral_ipmc_getFirstMcastAddr
 * PURPOSE:
 *      This API is used to get a the first multicast address entry.
 *
 * INPUT:
 *      unit            --  Device unit number
 *      match_type      --  The type to search multicast entry
 *
 * OUTPUT:
        ptr_entry_cnt   --  The number of multicast address entries
 *      ptr_entry       --  The multicast entry
 *
 * RETURN:
 *      AIR_E_OK        --  Operation succeeded.
 *      Others          --  Operation failed.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getFirstMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry);

/* FUNCTION NAME: hal_coral_ipmc_getNextMcastAddr
 * PURPOSE:
 *      Get next multicast source address entry for specific group and source address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      match_type      --  The type to search multicast entry
 *
 * OUTPUT:
 *      ptr_entry_cnt   --  The number of returned multicast entries
 *      ptr_entry       --  The multicast searching result.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getNextMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry);

/* FUNCTION NAME:   hal_coral_ipmc_setMcastLookupType
 * PURPOSE:
 *      This API is used to set multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *      type            --  The multicast type
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *     AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setMcastLookupType(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type);

/* FUNCTION NAME:   hal_coral_ipmc_getMcastLookupType
 * PURPOSE:
 *      This API is used to get multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *
 * OUTPUT:
 *      ptr_type        --  The multicast type
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastLookupType(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type);

/* FUNCTION NAME:   hal_coral_ipmc_setPortIpmcMode
 * PURPOSE:
 *      This API is used to set IGMP snooping mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 *      enable          --  The IGMP snooping mode refer to
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_coral_ipmc_getPortIpmcMode
 * PURPOSE:
 *      This API is used to get IGMP snooping mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 *
 * OUTPUT:
 *      ptr_enable      --  The IGMP snooping mode refer to
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_coral_ipmc_getCapacity
 * PURPOSE:
 *      Get the ipmc resource capacity
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
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_coral_ipmc_getUsage
 * PURPOSE:
 *      Get the ipmc resource usage
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
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

/* FUNCTION NAME:   hal_coral_ipmc_setPortLookupIpTypeCtrl
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
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    const BOOL_T        enable);

/* FUNCTION NAME:   hal_coral_ipmc_getPortLookupIpTypeCtrl
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
 *      AIR_E_OK
 *      AIR_E_OTHERS
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    BOOL_T             *ptr_enable);

#endif /* End of HAL_CORAL_IPMC_H */
