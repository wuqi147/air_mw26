/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  lp_acl.h
 * PURPOSE:
 *      This file contains the declaration of ACL for loop prevention.
 *
 * NOTES:
 */
#ifndef LP_ACL_H
#define LP_ACL_H
/* INCLUDE FILE DECLARATIONS
 */
#include "lp.h"
#ifdef LP_MW_SUPPORT
#include "syncd_api_stp.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define LP_MAC_ACL_NUM      (7)
#define LP_MAC_UDF_NUM      (8)

#ifndef LP_MW_SUPPORT
#if defined(AIR_EN_CORAL) || defined(AIR_EN_PEARL)
#define AIR_LP_USE_STP_BLOCK   (1)
#endif
#endif/* LP_MW_SUPPORT */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: lp_acl_setMacComparationAcl
 * PURPOSE:
 *      Set ACL for loop prevention mac comparison ACL
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_REACH_END
 *      MW_E_NOT_INITED
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_setMacComparationAcl(
    void);

/* FUNCTION NAME: lp_acl_removeMacComparationAcl
 * PURPOSE:
 *      Remove ACL for loop prevention mac comparison ACL
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_removeMacComparationAcl(
    void);

/* FUNCTION NAME: lp_acl_setGenericAcl
 * PURPOSE:
 *      This function is used to set the generic ACL for the loop prevention.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_REACH_END
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_setGenericAcl(
    void);

/* FUNCTION NAME: lp_acl_removeGenericAcl
 * PURPOSE:
 *      This function is used to remove the generic ACL for the loop prevention.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_removeGenericAcl(
    void);

/* FUNCTION NAME: lp_acl_initVariable
 * PURPOSE:
 *      Initialize the ACL variables.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_initVariable(
    void);

/* FUNCTION NAME: lp_acl_set
 * PURPOSE:
 *      Set ACL for loop prevention
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_set(
    void);

/* FUNCTION NAME: lp_acl_remove
 * PURPOSE:
 *      Remove ACL for loop prevention
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_remove(
    void);

#ifdef AIR_LP_USE_STP_BLOCK
/* FUNCTION NAME: lp_acl_setLdfAsBpduAcl
 * PURPOSE:
 *      Create the LDF AS BPDU ACL.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_setLdfAsBpduAcl(
    void);

/* FUNCTION NAME: lp_acl_removeLdfAsBpduAcl
 * PURPOSE:
 *      Remove the LDF AS BPDU ACL.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_acl_removeLdfAsBpduAcl(
    void);
#endif /* AIR_LP_USE_STP_BLOCK */

/* FUNCTION NAME: lp_acl_dumpInfo
 * PURPOSE:
 *      Dump lp ACL information.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
lp_acl_dumpInfo(
    void);

#endif  /* LP_ACL_H */
