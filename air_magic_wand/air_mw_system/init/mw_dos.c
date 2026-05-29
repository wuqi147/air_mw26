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

/* FILE NAME:  mw_dos.c
 * PURPOSE:
 * It provides the API for DOS module.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "mw_dos.h"
#include "mw_error.h"
#include "air_dos.h"

/* GLOBAL VARIABLE DECLARATIONS
*/
static UI8_T  _mw_attack_prevention_global_state_ref_cnt = 0;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM
 */
/* FUNCTION NAME: mw_dos_setGlobalCfg
 * PURPOSE:
 *      Sets the global configuration for the DoS module.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      enable               -- Enable/disable DoS module
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OTHERS
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_dos_setGlobalCfg(
    UI32_T     unit,
    BOOL_T     enable)
{
    BOOL_T              dos_global_state = FALSE;
    AIR_ERROR_NO_T      rc = AIR_E_OK;

    if(TRUE == enable)
    {
        air_dos_getGlobalCfg(unit, &dos_global_state);
        if(FALSE == dos_global_state)
        {
            rc = air_dos_setGlobalCfg(unit,TRUE);
        }
        if(AIR_E_OK == rc)
        {
            _mw_attack_prevention_global_state_ref_cnt ++;
        }
    }
    else
    {
        if(1 == _mw_attack_prevention_global_state_ref_cnt)
        {
            rc = air_dos_setGlobalCfg(unit, FALSE);
            if(AIR_E_OK == rc)
            {
                _mw_attack_prevention_global_state_ref_cnt --;
            }
        }
        else if(1 < _mw_attack_prevention_global_state_ref_cnt)
        {
            _mw_attack_prevention_global_state_ref_cnt --;
            rc = AIR_E_OK;
        }
        else
        {
            rc = AIR_E_OTHERS;
        }
    }

    return ((AIR_E_OK == rc) ? MW_E_OK : MW_E_OTHERS);
}
