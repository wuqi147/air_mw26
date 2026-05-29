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

/* FILE NAME:  mw_rate_limit.h
 * PURPOSE:
 *      This file contains the declaration of rate limit functionality for the Magic Wand module.
 *
 * NOTES:
 */
#ifndef MW_RATE_LIMIT_H
#define MW_RATE_LIMIT_H
/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "default_config.h"
#include <mw_utils.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define LP_PER_PORT_DOS_RATE_LIMIT_PKT_THLD             (40)
#define RSTP_PER_PORT_DOS_RATE_LIMIT_PKT_THLD           (10)
#define MW_RATE_LIMIT_MODULE_NAME                       "rateLmt"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct MW_PORT_RATE_LIMIT_S
{
    UI8_T                       ref_cnt;         /* Reference count */
    UI16_T                      attack_id;       /* Attack prevention ID  */
} ATTRIBUTE_PACK MW_PORT_RATE_LIMIT_T;

typedef enum MW_PORT_RATE_LIMIT_USER_S
{
    MW_PORT_RATE_LIMIT_USER_LP = 0,
    MW_PORT_RATE_LIMIT_USER_RSTP,

    MW_PORT_RATE_LIMIT_USER_LAST
}MW_PORT_RATE_LIMIT_USER_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   mw_rate_limit_init
 * PURPOSE:
 *      This API is used to initialize the mw rate limit module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *      MW_E_NO_MEMORY
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_rate_limit_init(
    void);

#if defined(AIR_SUPPORT_LP) || defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* FUNCTION NAME:   mw_rate_limit_lp_stp_port_attack_id_get
 * PURPOSE:
 *      This API is used to get port's attack ID for the loop prevention or stp.
 *
 * INPUT:
 *      unit                 -- Unit id
 *      port                 -- Port id
 *      user                 -  User id
 *
 * OUTPUT:
 *      ptr_attack_id        -- Pointer to the attack ID
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_rate_limit_lp_stp_port_attack_id_get(
    const UI32_T    unit,
    const UI8_T     port,
    MW_PORT_RATE_LIMIT_USER_T  user,
    UI32_T          *ptr_attack_id);

/* FUNCTION NAME:   mw_rate_limit_lp_stp_port_attack_clear
 * PURPOSE:
 *      This API is used to clear DoS attack rate limit rule on specific port for loop prevention or stp.
 *
 * INPUT:
 *      unit                 -- Unit id
 *      port                 -- Port id
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
mw_rate_limit_lp_stp_port_attack_clear(
    const UI32_T    unit,
    const UI8_T     port);
#endif /* AIR_SUPPORT_LP || AIR_SUPPORT_RSTP || defined(AIR_SUPPORT_MSTP)*/

#endif  /* MW_RATE_LIMIT_H */
