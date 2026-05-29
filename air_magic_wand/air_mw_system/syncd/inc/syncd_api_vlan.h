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

/* FILE NAME:  syncd_api_vlan.h
 * PURPOSE:
 *      It provides VLAN API functions in syncd.
 *
 * NOTES:
 */

#ifndef _SYNCD_API_VLAN_H_
#define _SYNCD_API_VLAN_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "syncd_api.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: syncd_api_vlan_pvid
 * PURPOSE:
 *      process Port PVID setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_pvid(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_vlan_mode
 * PURPOSE:
 *      process Port ingress check setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_mode(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_vlan_config
 * PURPOSE:
 *      process VLAN state config setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_config(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_vlan_entry
 * PURPOSE:
 *      process VLAN entry setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_entry(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_vlan_matrix
 * PURPOSE:
 *      Set port isolation setting from DB.
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_matrix(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_update_port_matrix
 * PURPOSE:
 *      This API is used to update the port matrix.
 * INPUT:
 *      portbmp     --  Port bit map of to be updated
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_update_port_matrix(
    MW_PORT_BITMAP_T portbmp);

#endif  /*_SYNCD_API_VLAN_H_*/

