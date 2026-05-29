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
/* FILE NAME:  air_svlan.h
 * PURPOSE:
 *      It provides SVLAN module API.
 *
 * NOTES:
 *
 */

#ifndef AIR_SVLAN_H
#define AIR_SVLAN_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct AIR_SVLAN_ENTRY_S
{
#define AIR_SVLAN_ENTRY_FLAGS_CHK_CVID     (1U << 0)
#define AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI     (1U << 1)
#define AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE (1U << 2)

    /* Svlan cfg flags*/
    UI32_T flags;

    /* Customer VID (12bits) */
    UI16_T c_vid;

    /* Customer Vlan Priority */
    UI16_T c_pri;

    /* Ether Type */
    UI32_T eth_type;

    /* Service VID (12bits) */
    UI16_T s_vid;

    /* Service Vlan Priority */
    UI16_T s_pri;
} AIR_SVLAN_ENTRY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_svlan_addSvlan
 * PURPOSE:
 *      Add or set a svlan entry.by index
 * INPUT:
 *      unit                     -- Device ID
 *      idx                      -- Entry Index ID
 *      ptr_entry                -- Structure of svlan table
 *                                  AIR_SVLAN_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_addSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_svlan_getSvlan
 * PURPOSE:
 *      Get a svlan entry.by index
 * INPUT:
 *      unit                     -- Device ID
 *      idx                      -- Entry Index ID
 * OUTPUT:
 *      ptr_entry                -- Structure of svlan table
 *                                  AIR_SVLAN_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_svlan_delSvlan
 * PURPOSE:
 *      Delete a svlan entry by index
 * INPUT:
 *      unit                     -- Device ID
 *      idx                      -- Entry Index ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_delSvlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   air_svlan_setCustomPort
 * PURPOSE:
 *      Set a svlan port to custom port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      enable                   -- Enable svlan custom port
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_setCustomPort(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_svlan_getCustomPort
 * PURPOSE:
 *      Get svlan custom port enable status
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_enable               -- Svlan custom port enable status
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getCustomPort(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_svlan_setServicePort
 * PURPOSE:
 *      Set a svlan port to service port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      enable                   -- Enable svlan custom port
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_setServicePort(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_svlan_getServicePort
 * PURPOSE:
 *      Get svlan service port enable status
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_enable               -- Svlan custom port enable status
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getServicePort(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_svlan_setEgressOuterTpid
 * PURPOSE:
 *      Set svlan outer tag-protocol identifier(TPID) for egress frame
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      tpid                     -- TPID value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_setEgressOuterTpid(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T tpid);

/* FUNCTION NAME:   air_svlan_getEgressOuterTpid
 * PURPOSE:
 *      Get svlan outer tag-protocol identifier(TPID) value
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_tpid                 -- TPID value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getEgressOuterTpid(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_tpid);

/* FUNCTION NAME:   air_svlan_setEgressInnerTpid
 * PURPOSE:
 *      Set svlan inner tag-protocol identifier(TPID) for egress frame
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      tpid                     -- TPID value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_setEgressInnerTpid(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T tpid);

/* FUNCTION NAME:   air_svlan_getEgressInnerTpid
 * PURPOSE:
 *      Get svlan inner tag-protocol identifier(TPID) value
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_tpid                 -- TPID value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getEgressInnerTpid(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_tpid);

/* FUNCTION NAME:   air_svlan_setIngressTpid
 * PURPOSE:
 *      Set svlan stack tag TPID(outer TPID), it will be regarded as
 *      legal stack tag frame for incomming stack frame check
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      tpid                     -- TPID value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_setIngressTpid(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T tpid);

/* FUNCTION NAME:   air_svlan_getIngressTpid
 * PURPOSE:
 *      Get svlan stack tag TPID(outer TPID) value
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_tpid                 -- TPID value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_svlan_getIngressTpid(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_tpid);

#endif /* End of AIR_SVLAN_H */
