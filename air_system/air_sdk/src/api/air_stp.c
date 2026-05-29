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

/* FILE NAME:  air_stp.c
 * PURPOSE:
 *    It provide STP module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_stp.h>

#include <air_error.h>
#include <air_init.h>
#include <air_types.h>
#include <hal/common/hal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_STP, "air_stp.c");
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_stp_setPortState
 * PURPOSE:
 *      Set the STP port state for a specifiec port.
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 *      fid                  -- Filter ID for MSTP
 *      state                -- STP state
 *                              AIR_STP_STATE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stp_setPortState(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          fid,
    const AIR_STP_STATE_T state)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(fid, AIR_STP_FID_NUMBER);
    HAL_CHECK_ENUM_RANGE(state, AIR_STP_STATE_LAST);

    rc = HAL_FUNC_CALL(unit, stp, setPortState, (unit, port, fid, state));
    return rc;
}

/* FUNCTION NAME:   air_stp_getPortState
 * PURPOSE:
 *      Get the STP port state for a specifiec port.
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 *      fid                  -- Filter ID for MSTP
 * OUTPUT:
 *      ptr_state            -- STP state
 *                              AIR_STP_STATE_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stp_getPortState(
    const UI32_T     unit,
    const UI32_T     port,
    const UI32_T     fid,
    AIR_STP_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(fid, AIR_STP_FID_NUMBER);
    HAL_CHECK_PTR(ptr_state);

    rc = HAL_FUNC_CALL(unit, stp, getPortState, (unit, port, fid, ptr_state));
    return rc;
}
