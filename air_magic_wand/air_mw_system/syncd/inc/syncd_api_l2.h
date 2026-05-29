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

/* FILE NAME:  syncd_api_l2.h
 * PURPOSE:
 *      It provides L2 API functions in syncd.
 *
 * NOTES:
 */

#ifndef _SYNCD_API_L2_H_
#define _SYNCD_API_L2_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "syncd_api.h"
#include "mac_utils.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE            (3)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    SYNCD_API_L2_FLUSH_PORT,
    SYNCD_API_L2_FLUSH_VLAN,

    SYNCD_API_L2_FLUSH_LAST
}SYNCD_API_L2_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: syncd_api_l2_static_mac
 * PURPOSE:
 *      process L2 static MAC entry request from DB.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_l2_static_mac(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_dynamic_mac_address_entry
 * PURPOSE:
 *      Process syncd dynamic configuration info.
 *
 * INPUT:
 *      ptr_api_arg              -- porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INVALID
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_dynamic_mac_address_entry(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_flushMacAddr
 * PURPOSE:
 *      This API is used to flush port or vlan dynamic mac address.
 *
 * INPUT:
 *      unit        -- Device ID
 *      value       -- Port id or vlan id
 *      type        -  Port or vlan
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_flushMacAddr(
    const UI32_T    unit,
    const UI32_T    value,
    const SYNCD_API_L2_TYPE_T type);

/* FUNCTION NAME: syncd_api_transferPortStaticMacAddr
 * PURPOSE:
 *      This API is used to transfer the static MAC address from one port to another port.
 *
 * INPUT:
 *      src_port    -- Source port
 *      dst_port    -- Destination port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If dest_port is 0, it means flush source port static mac address.
 */
MW_ERROR_NO_T
syncd_api_transferPortStaticMacAddr(
    const UI32_T    src_port,
    const UI32_T    dst_port);
#endif  /*_SYNCD_API_L2_H_*/
