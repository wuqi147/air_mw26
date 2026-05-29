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

/* FILE NAME:  hal_sco_mirror.h
 * PURPOSE:
 *  Define the declartion for mirror module.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_MIR_H
#define HAL_SCO_MIR_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_mirror.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* INTERNAL SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_mir_init
 * PURPOSE:
 *      This API is used to init mirror session.
 * INPUT:
 *      unit        --   Device unit number
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_mir_deinit
 * PURPOSE:
 *      This API is used to deinit mirror session.
 * INPUT:
 *      unit        --   Device unit number
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       AIR_E_OK
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_deinit(
    const UI32_T unit);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_mir_addSession
 * PURPOSE:
 *      This API is used to add or set a mirror session.
 * INPUT:
 *      unit        --   Device unit number
 *      session_id  --   The session information
 *      ptr_session --   The session information
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_addSession(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME: hal_sco_mir_delSession
 * PURPOSE:
 *      This API is used to delete a mirror session.
 * INPUT:
 *      unit        --   Device unit number
 *      session_id  --   The session information
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_delSession(
    const UI32_T unit,
    const UI32_T session_id);

/* FUNCTION NAME: hal_sco_mir_getSession
 * PURPOSE:
 *      This API is used to get mirror session information.
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  The session information
 * OUTPUT:
 *      ptr_session  --  The information of this session to be obtained
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_getSession(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME: hal_sco_mir_setSessionAdminMode
 * PURPOSE:
 *      This API is used to set mirror session state
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  mirror session id
 *      state        --  FALSE: disable
 *                       TRUE:  enable
 * OUTPUT:
 *      None
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_setSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T state);

/* FUNCTION NAME: hal_sco_mir_getSessionAdminMode
 * PURPOSE:
 *      This API is used to get mirror session state
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  mirror session id
 * OUTPUT:
 *      state        --  FALSE: disable
 *                       TRUE:  enable
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_getSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T      *state);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_mir_setMirrorPort
 * PURPOSE:
 *      This API is used to set mirror port mirroring type
 * INPUT:
 *      unit        --   Device unit number
 *      session_id  --   mirror session id
 *      ptr_session --   The session information
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_setMirrorPort(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME: hal_sco_mir_getMirrorPort
 * PURPOSE:
 *      This API is used to get mirror port mirroring type
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  mirror session id
 * OUTPUT:
 *      ptr_session  --  The information of this session to be obtained
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_sco_mir_getMirrorPort(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

#endif /* end of HAL_SCO_MIR_H */
