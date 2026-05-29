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

/* FILE NAME:  air_mirror.h
 * PURPOSE:
 *      It provides MIRROR module API.
 * NOTES:
 */

#ifndef AIR_MIRROR_H
#define AIR_MIRROR_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define AIR_MIRROR_SESSION_NUMBER (4)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* mirror session */
typedef struct AIR_MIR_SESSION_S
{
#define AIR_MIR_SESSION_FLAGS_ENABLE          (1U << 0)
#define AIR_MIR_SESSION_FLAGS_DIR_TX          (1U << 1)
#define AIR_MIR_SESSION_FLAGS_DIR_RX          (1U << 2)
#define AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG (1U << 3)

    /* flags refer to AIR_MIR_SESSION_FLAGS_XXX */
    UI32_T flags;
    UI32_T dst_port;
    UI32_T src_port;
} AIR_MIR_SESSION_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_mir_addSession
 * PURPOSE:
 *      This API is used to add or set a mirror session.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      ptr_session          -- Session information
 *                              AIR_MIR_SESSION_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_addSession(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME:   air_mir_delSession
 * PURPOSE:
 *      This API is used to delete a mirror session.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_delSession(
    const UI32_T unit,
    const UI32_T session_id);

/* FUNCTION NAME:   air_mir_getSession
 * PURPOSE:
 *      This API is used to get mirror session information.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      ptr_session          -- The information of the session to be
 *                              obtained
 *                              AIR_MIR_SESSION_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getSession(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME:   air_mir_setSessionAdminMode
 * PURPOSE:
 *      This API is used to set mirror session state.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      enable               -- State of session
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_setSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T enable);

/* FUNCTION NAME:   air_mir_getSessionAdminMode
 * PURPOSE:
 *      This API is used to get mirror session state.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- mirror session id
 * OUTPUT:
 *      ptr_enable           -- State of session
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_mir_setMirrorPort
 * PURPOSE:
 *      This API is used to set mirror port mirroring type.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      ptr_session          -- Session information
 *                              AIR_MIR_SESSION_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_setMirrorPort(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME:   air_mir_getMirrorPort
 * PURPOSE:
 *      This API is used to get mirror port mirroring type.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      ptr_session          -- The information of this session to be
 *                              obtained.
 *                              AIR_MIR_SESSION_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getMirrorPort(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

#endif /* End of AIR_MIRROR_H */
