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

/* FILE NAME:  igmp_snoop_mode.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping mode.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_MODE_H
#define IGMP_SNOOP_MODE_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   igmp_snp_setAdminMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping admin mode.
 *
 * INPUT:
 *      enable       --  admin mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setAdminMode(
    UI8_T enable);

/* FUNCTION NAME:   igmp_snp_setRptSuppressMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping report suppress.
 *
 * INPUT:
 *      enable       --  report suppress mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setRptSuppressMode(
    UI8_T enable);

/* FUNCTION NAME:   igmp_snp_setFastLeaveMode
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping fast-leave.
 *
 * INPUT:
 *      enable       --  fast-leave mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setFastLeaveMode(
    UI8_T enable);


/* FUNCTION NAME:   igmp_snp_setUnIpmcForwardMode
 * PURPOSE:
 *      This API is used to enable/disable unknown ip multicast packet drop function.
 *
 * INPUT:
 *      enable       --  enable or disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setUnIpmcForwardMode(
    UI8_T enable);

#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
/* FUNCTION NAME:   igmp_snp_setModeWithCfg
 * PURPOSE:
 *      Set the igmp snp with the data of config structure input.
 *
 * INPUT:
 *      ptr_data     --  Pointer to the igmp_snp config structure filled with the config data
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
igmp_snp_setModeWithCfg(
    const void *ptr_data);

/* FUNCTION NAME:   igmp_snp_setDefaultCfgMode
 * PURPOSE:
 *      Set the igmp snp to default configuration mode.
 *
 * INPUT:
 *      None
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
igmp_snp_setDefaultCfgMode(
    void);

/* FUNCTION NAME:   igmp_snp_setDipSwitchCfgMode
 * PURPOSE:
 *      Set the igmp snp dip switch config mode.
 *
 * INPUT:
 *      None
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
igmp_snp_setDipSwitchCfgMode(
    void);

#endif /* IGMP_SNP_CUSTOMER_CONFIG_SUPPORT */

#ifdef AIR_SUPPORT_IGMPV3_AWARE
/* FUNCTION NAME:   igmp_snp_v3ware_enable_set
 * PURPOSE:
 *      This API is used to enable/disable IGMP snooping v3 aware function.
 *
 * INPUT:
 *      enable       --  enable or disable IGMP snooping v3 aware function
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_setV3AwareMode(
    UI8_T enable);
#endif

#endif  /* IGMP_SNOOP_MODE_H */
