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

/* FILE NAME:  hal_coral_svlan.h
 * PURPOSE:
 *      It provides hal svlan module API
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_SVLAN_H
#define HAL_CORAL_SVLAN_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_svlan.h>
#include <air_swc.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_SVLAN_PRI_MAX (7)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_coral_svlan_addSvlan
 * PURPOSE:
 *      Add or set a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 *      ptr_entry       --  Structure of svlan table
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_EXISTS
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_addSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_svlan_getSvlan
 * PURPOSE:
 *      Get a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      ptr_entry       --  Structure of svlan table
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_coral_svlan_delSvlan
 * PURPOSE:
 *      Delete a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_delSvlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME: hal_coral_svlan_setCustomPort
 * PURPOSE:
 *      Set a svlan port to custom port
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      enable          --  Enable svlan custom port
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
hal_coral_svlan_setCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable);

/* FUNCTION NAME: hal_coral_svlan_getCustomPort
 * PURPOSE:
 *      Get svlan custom port enable status
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_enable      --  Svlan custom port enable status
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_coral_svlan_setServicePort
 * PURPOSE:
 *      Set a svlan port to service port
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      enable          --  Enable svlan custom port
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
hal_coral_svlan_setServicePort(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable);

/* FUNCTION NAME: hal_coral_svlan_getServicePort
 * PURPOSE:
 *      Get svlan service port enable status
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_enable      --  Svlan custom port enable status
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getServicePort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_coral_svlan_setEgsOuterTPID
 * PURPOSE:
 *      Set svlan outer tag-protocol identifier(TPID) for egress frame
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
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
hal_coral_svlan_setEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

/* FUNCTION NAME: hal_coral_svlan_getEgsOuterTPID
 * PURPOSE:
 *      Get svlan outer tag-protocol identifier(TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid);

/* FUNCTION NAME: hal_coral_svlan_setEgsInnerTPID
 * PURPOSE:
 *      Set svlan inner tag-protocol identifier(TPID) for egress frame
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
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
hal_coral_svlan_setEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

/* FUNCTION NAME: hal_coral_svlan_getEgsInnerTPID
 * PURPOSE:
 *      Get svlan inner tag-protocol identifier(TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid);

/* FUNCTION NAME: hal_coral_svlan_setRecvTagTPID
 * PURPOSE:
 *      Set svlan stack tag TPID(outer TPID), it will be regarded as legal
 *      stack tag frame for incomming stack frame check
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
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
hal_coral_svlan_setRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

/* FUNCTION NAME: hal_coral_svlan_getRecvTagTPID
 * PURPOSE:
 *      Get svlan stack tag TPID(outer TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_svlan_getRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid);

/* FUNCTION NAME: hal_coral_svlan_getCapacity
 * PURPOSE:
 *      Get the svlan resource capacity
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
hal_coral_svlan_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_coral_svlan_getUsage
 * PURPOSE:
 *      Get the svlan resource usage
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
hal_coral_svlan_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* End of HAL_CORAL_SVLAN_H */
