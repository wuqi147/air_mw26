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

/* FILE NAME:  igmp_snoop_cmd.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop commands.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_cmd.h"
#include "igmp_snoop.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_log.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_snp_cmd_setAdminState
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable IGMP snooping admin mode.
 *
 * INPUT:
 *      enable       --  admin mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_cmd_setAdminState(
    UI8_T enable)
{
    MW_ERROR_NO_T rc = MW_E_OK;

#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_LOG_DEBUG("Set igmp snp admin mode, enable=%d", enable);
    rc = igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_ENABLE, DB_ALL_ENTRIES, &enable, IGMP_SNP_SEMA_DELAY);
#endif

    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

/* FUNCTION NAME:   igmp_snp_cmd_setRptSuppressMode
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable IGMP snooping report suppress.
 *
 * INPUT:
 *      enable       --  report suppress mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_cmd_setRptSuppressMode(
    UI8_T enable)
{
    MW_ERROR_NO_T rc = MW_E_OK;

#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_LOG_DEBUG("Set igmp snp suppress mode, enable=%d", enable);
    rc = igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_RPT_SUPPRESS, DB_ALL_ENTRIES, &enable, IGMP_SNP_SEMA_DELAY);
#endif

    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

/* FUNCTION NAME:   igmp_snp_setFastLeaveMode
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable IGMP snooping fast-leave.
 *
 * INPUT:
 *      enable       --  fast-leave mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_cmd_setFastLeaveMode(
    UI8_T enable)
{
    MW_ERROR_NO_T rc = MW_E_OK;

#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_LOG_DEBUG("Set igmp snp fast leave mode, enable=%d", enable);
    rc = igmp_snp_db_queue_send(NULL, M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_FAST_LEAVE, DB_ALL_ENTRIES, &enable, IGMP_SNP_SEMA_DELAY);
#endif

    return (MW_E_OK == rc) ? MW_E_OK : MW_E_OTHERS;
}

/* FUNCTION NAME: igmp_snp_cmd_dumpInfo
 * PURPOSE:
 *      Show the current status of IGMP SNP module.
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
igmp_snp_cmd_dumpInfo(
    void)
{
    igmp_snp_log_dumpInfo();

    return MW_E_OK;
}