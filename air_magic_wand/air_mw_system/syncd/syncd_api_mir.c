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

/* FILE NAME:  syncd_api_mir.c
 * PURPOSE:
 *  Implement port Mirror API function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <syncd_api_mir.h>
#include <air_mirror.h>
#include <air_port.h>
#include <mw_platform.h>

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: syncd_api_mir_setSession
 * PURPOSE:
 *      Set Mirror session and dst port from DB.
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
syncd_api_mir_setSession(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = MW_E_OK;
    AIR_MIR_SESSION_T session;
    UI32_T dst_port = 0;
    UI16_T sid = 0, e_id = 0;
    UI8_T *ptr_mirrorInfo;

    MW_CHECK_PTR(ptr_api_arg);

    e_id = (ptr_api_arg->ptr_type->e_idx);

    if (M_GET == ptr_api_arg ->method && DB_ALL_ENTRIES == e_id )
    {
        ptr_mirrorInfo = (ptr_api_arg ->ptr_data);

        for (sid = 0; sid < MAX_MIRROR_SESS_NUM; sid++, ptr_mirrorInfo++)
        {
            memset(&session, 0, sizeof(session));
            rc = air_mir_getSession(0, sid, &session);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Get Session = %d failed(%d)", sid, rc);
                return MW_E_OP_INCOMPLETE;
            }
            session.dst_port = *ptr_mirrorInfo;

            SYNCD_LOG_DEBUG("[%s] Session-id:%d, dst-port:%d\n",__FUNCTION__, sid, session.dst_port);
            /* DB default value is all-zero, port 0 is invalid.
             * Don't execute when dst_port = 0 case */
            if (0 == session.dst_port)
            {
                continue;
            }

            rc = air_mir_addSession(0, sid, &session);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Create Session = %d, port = %dfailed(%d)", sid, session.dst_port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else if (M_UPDATE == ptr_api_arg ->method)
    {
        sid = e_id - 1;
        dst_port = *((UI8_T *)ptr_api_arg ->ptr_data);

        SYNCD_LOG_DEBUG("[%s] Session-id:%d, dst-port:%d\n",__FUNCTION__, sid, dst_port);

        memset(&session, 0, sizeof(session));
        rc = air_mir_getSession(0, sid, &session);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Get Session = %d failed(%d)", sid, rc);
            return MW_E_OP_INCOMPLETE;
        }
        session.dst_port = dst_port;

        rc = air_mir_addSession(0, sid, &session);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Create Session = %d, port = %dfailed(%d)", sid, session.dst_port, rc);
            return MW_E_OP_INCOMPLETE;
        }

    }

    return rc;
}

/* FUNCTION NAME: syncd_api_mir_setState
 * PURPOSE:
 *      Set Mirror enable state from DB.
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
syncd_api_mir_setState(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = MW_E_OK;
    UI8_T state;
    UI16_T sid = 0, e_id = 0;
    UI8_T *ptr_mirrorInfo;
    MW_CHECK_PTR(ptr_api_arg);

    e_id = (ptr_api_arg->ptr_type->e_idx);
    if (M_GET == ptr_api_arg ->method && DB_ALL_ENTRIES == e_id )
    {
        ptr_mirrorInfo = (ptr_api_arg ->ptr_data);

        for (sid = 0; sid < MAX_MIRROR_SESS_NUM; sid++, ptr_mirrorInfo++)
        {
            state = *(ptr_mirrorInfo);

            SYNCD_LOG_DEBUG("[%s] Session-id:%d, State:%d\n",__FUNCTION__ , sid, state);

            if (FALSE == state)
            {
                rc = air_mir_setSessionAdminMode(0, sid, FALSE);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Disable Session %d failed(%d)", sid, rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
            else
            {
                rc = air_mir_setSessionAdminMode(0, sid, TRUE);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Enable Session %d failed(%d)", sid, rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
        }
    }
    else if (M_UPDATE == ptr_api_arg ->method)
    {
        sid = e_id - 1;
        state = *((UI8_T *)ptr_api_arg ->ptr_data);

        SYNCD_LOG_DEBUG("[%s] Session-id:%d, State:%d\n",__FUNCTION__ , sid, state);

        if (FALSE == state)
        {
            rc = air_mir_setSessionAdminMode(0, sid, FALSE);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Disable Session %d failed(%d)", sid, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            rc = air_mir_setSessionAdminMode(0, sid, TRUE);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Enable Session %d failed(%d)", sid, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: syncd_api_mir_setList
 * PURPOSE:
 *      Set Mirrored port Pbm from DB.
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
syncd_api_mir_setList(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = MW_E_OK;
    AIR_MIR_SESSION_T session;
    UI16_T expect_entry_num, sidLoop = 0, i = 0, j = 0;
    UI16_T e_id = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_ingress = NULL;
    UI8_T *ptr_egress = NULL;
    UI32_T ingressPbm[MAX_MIRROR_SESS_NUM] = {0}, egressPbm[MAX_MIRROR_SESS_NUM] = {0};
    UI8_T sessionID[MAX_MIRROR_SESS_NUM] = {0};
    UI32_T port, tmp_port;

    MW_CHECK_PTR(ptr_api_arg);

    e_id = (ptr_api_arg->ptr_type->e_idx);
    expect_entry_num = (DB_ALL_ENTRIES == e_id)? MAX_MIRROR_SESS_NUM : 1;

    ptr_data = (UI8_T *)ptr_api_arg ->ptr_data;
    ptr_ingress = ptr_data + (sizeof(UI8_T) * expect_entry_num)+ (sizeof(UI8_T) * expect_entry_num);
    ptr_egress = ptr_ingress + (sizeof(UI32_T) * expect_entry_num);


    if (M_GET == ptr_api_arg ->method && DB_ALL_ENTRIES == e_id )
    {
        /* Init from DB, it means the 1st notification*/
        for (i = 0; i < MAX_MIRROR_SESS_NUM; i ++)
        {
            ingressPbm[sidLoop] = htonl(get32(ptr_ingress));
            egressPbm[sidLoop] = htonl(get32(ptr_egress));
            sessionID[sidLoop] = i;
            SYNCD_LOG_DEBUG("[ALL] Session-id:%d, TX_PBM:%x,RX_PBM:%x\n", sessionID[i], egressPbm[i], ingressPbm[i]);
            ptr_ingress = ptr_ingress + sizeof(UI32_T); /* next ptr */
            ptr_egress = ptr_egress + sizeof(UI32_T);
            sidLoop ++;
        }

    }
    else if (M_UPDATE == ptr_api_arg ->method)
    {
        ingressPbm[0] = htonl(get32(ptr_ingress));
        egressPbm[0] = htonl(get32(ptr_egress));
        sessionID[0] = e_id - 1; /* 0-based session id */
        SYNCD_LOG_DEBUG("Session-id:%d, TX_PBM:%x,RX_PBM:%x\n", sessionID[0], egressPbm[0], ingressPbm[0]);
        sidLoop ++;
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }

    for (j = 0; j < sidLoop; j ++)
    {

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            /* Init session struct */
            memset(&session, 0, sizeof(session));

            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }

            tmp_port = (port > PLAT_CPU_PORT) ? (port - 1) : port;
            if ((ingressPbm[j] & (0x01 << tmp_port)) && (egressPbm[j] & (0x01 << tmp_port)))
            {
                session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
                session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
            }
            else if (ingressPbm[j] & (0x01 << tmp_port))
            {
                session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
            }
            else if (egressPbm[j] & (0x01 << tmp_port))
            {
                session.flags = AIR_MIR_SESSION_FLAGS_DIR_TX;
            }
            else
            {
                session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_TX;
                session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_RX;
            }
            /* Set config */
            session.src_port = port;
            SYNCD_LOG_DEBUG("src_port:%d, session_id:%d, flags:%x\n", session.src_port, sessionID[j], session.flags);
            rc = air_mir_setMirrorPort(0, sessionID[j], &session);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Session %d set src_port %d, flags:%x failed(%d)", sessionID[j], session.src_port, session.flags, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    return rc;
}
