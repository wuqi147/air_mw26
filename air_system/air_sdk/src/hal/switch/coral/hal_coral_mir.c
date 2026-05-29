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

/* FILE NAME:  hal_coral_mir.c
 * PURPOSE:
 *  Implement Mirror module.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_mir.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/coral/hal_coral_reg.h>

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
DIAG_SET_MODULE_INFO(AIR_MODULE_MIR, "hal_coral_mir.c");

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_mir_init
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
hal_coral_mir_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, session = 0;

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        for (session = 0; session < HAL_CORAL_MIR_MAX_SESSION; session++)
        {
            /* Set mirroring port */
            reg_data &= ~BITS_RANGE(CORAL_MIR_PORT_OFFSET(session), CORAL_MIR_PORT_LEN);
            reg_data |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, CORAL_MIR_PORT_OFFSET(session), CORAL_MIR_PORT_LEN);
        }
        rc = aml_writeReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_addSession
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
hal_coral_mir_addSession(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;
    UI32_T         dst_mac_port, src_mac_port;
    BOOL_T         enable = FALSE, tx_tag_enable = FALSE;

    /* Mistake proofing */
    HAL_CHECK_ENUM_RANGE(session_id, HAL_CORAL_MIR_MAX_SESSION);
    HAL_CHECK_PORT(unit, ptr_session->dst_port);

    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->dst_port, dst_mac_port);
    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        /* Set mirroring port */
        reg_data &= ~BITS_RANGE(CORAL_MIR_PORT_OFFSET(session_id), CORAL_MIR_PORT_LEN);
        reg_data |= BITS_OFF_L(dst_mac_port, CORAL_MIR_PORT_OFFSET(session_id), CORAL_MIR_PORT_LEN);

        /* Set mirroring port tx tag state */
        if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG)
        {
            tx_tag_enable = TRUE;
        }
        reg_data &= ~BITS_RANGE(CORAL_MIR_TAG_TX_EN_OFFSET(session_id), CORAL_MIR_TAG_TX_EN_LEN);
        reg_data |= BITS_OFF_L(tx_tag_enable, CORAL_MIR_TAG_TX_EN_OFFSET(session_id), CORAL_MIR_TAG_TX_EN_LEN);

        /* Set mirroring port state */
        if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_ENABLE)
        {
            enable = TRUE;
        }
        reg_data &= ~BITS_RANGE(CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
        reg_data |= BITS_OFF_L(enable, CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);

        /* Write CORAL_MIR */
        rc = aml_writeReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]dst_port=%d, flags=0x%x, regMIR=0x%x\n", dst_mac_port, ptr_session->flags,
                   reg_data);
    }

    if (AIR_E_OK == rc)
    {
        /* Read CORAL_PCR */
        rc = aml_readReg(unit, CORAL_PCR(src_mac_port), &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            /* Set mirroring source port */
            reg_data &= ~BIT(CORAL_PCR_PORT_TX_MIR_OFFT + session_id);
            reg_data &= ~BIT(CORAL_PCR_PORT_RX_MIR_OFFT + session_id);
            if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
            {
                reg_data |= BIT(CORAL_PCR_PORT_TX_MIR_OFFT + session_id);
            }

            if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
            {
                reg_data |= BIT(CORAL_PCR_PORT_RX_MIR_OFFT + session_id);
            }

            /* Write CORAL_PCR */
            rc = aml_writeReg(unit, CORAL_PCR(src_mac_port), &reg_data, sizeof(reg_data));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]src_port=%d, flags=0x%x, regPCR=0x%x\n", src_mac_port, ptr_session->flags,
                       reg_data);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_delSession
 * PURPOSE:
 *      This API is used to delete a mirror session.
 * INPUT:
 *      unit        --   Device unit number
 *      session     --   The session information
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
hal_coral_mir_delSession(
    const UI32_T unit,
    const UI32_T session_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        reg_data &= ~BITS_RANGE(CORAL_MIR_PORT_OFFSET(session_id), CORAL_MIR_PORT_LEN);
        reg_data |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, CORAL_MIR_PORT_OFFSET(session_id), CORAL_MIR_PORT_LEN);
        reg_data &= ~BITS_RANGE(CORAL_MIR_TAG_TX_EN_OFFSET(session_id), CORAL_MIR_TAG_TX_EN_LEN);
        reg_data &= ~BITS_RANGE(CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
        rc = aml_writeReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_getSession
 * PURPOSE:
 *      This API is used to get mirror session information.
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --   The session information
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
hal_coral_mir_getSession(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, dst_mac_port = 0;
    BOOL_T         enable = FALSE, tx_tag_enable = FALSE;

    /* Mistake proofing */
    HAL_CHECK_ENUM_RANGE(session_id, HAL_CORAL_MIR_MAX_SESSION);

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        dst_mac_port = BITS_OFF_R(reg_data, CORAL_MIR_PORT_OFFSET(session_id), CORAL_MIR_PORT_LEN);
        enable = BITS_OFF_R(reg_data, CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
        tx_tag_enable = BITS_OFF_R(reg_data, CORAL_MIR_TAG_TX_EN_OFFSET(session_id), CORAL_MIR_TAG_TX_EN_LEN);

        HAL_MAC_PORT_TO_AIR_PORT(unit, dst_mac_port, ptr_session->dst_port);
        if (enable)
        {
            ptr_session->flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
        }
        if (tx_tag_enable)
        {
            ptr_session->flags |= AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_setSessionAdminMode
 * PURPOSE:
 *      This API is used to set mirror session state
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  mirror session id
 *      enable       --  FALSE: disable
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
hal_coral_mir_setSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        reg_data &= ~BITS_RANGE(CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
        reg_data |= BITS_OFF_L(enable, CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
        rc = aml_writeReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_getSessionAdminMode
 * PURPOSE:
 *      This API is used to get mirror session state
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --  mirror session id
 * OUTPUT:
 *      ptr_enable   --  FALSE: disable
 *                       TRUE:  enable
 * RETURN:
 *       AIR_E_OK
 *       AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *       None
 */
AIR_ERROR_NO_T
hal_coral_mir_getSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;

    rc = aml_readReg(unit, CORAL_MIR, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        *ptr_enable = BITS_OFF_R(reg_data, CORAL_MIR_EN_OFFSET(session_id), CORAL_MIR_EN_LEN);
    }
    return rc;
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_coral_mir_setMirrorPort
 * PURPOSE:
 *      This API is used to set mirror port mirroring type
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
hal_coral_mir_setMirrorPort(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, src_mac_port = 0;

    /* Mistake proofing */
    HAL_CHECK_PORT(unit, ptr_session->src_port);
    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    rc = aml_readReg(unit, CORAL_PCR(src_mac_port), &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        reg_data &= ~BIT(CORAL_PCR_PORT_TX_MIR_OFFT + session_id);
        reg_data &= ~BIT(CORAL_PCR_PORT_RX_MIR_OFFT + session_id);

        if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
        {
            reg_data |= BIT(CORAL_PCR_PORT_TX_MIR_OFFT + session_id);
        }

        if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
        {
            reg_data |= BIT(CORAL_PCR_PORT_RX_MIR_OFFT + session_id);
        }

        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]src_port=%d, flags=0x%x regPCR=0x%x\n", src_mac_port, ptr_session->flags,
                   reg_data);
        rc = aml_writeReg(unit, CORAL_PCR(src_mac_port), &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_mir_getMirrorPort
 * PURPOSE:
 *      This API is used to get mirror port mirroring type
 * INPUT:
 *      unit         --  Device unit number
 *      session_id   --   The session information
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
hal_coral_mir_getMirrorPort(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, src_mac_port = 0;

    /* Mistake proofing */
    HAL_CHECK_PORT(unit, ptr_session->src_port);
    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    rc = aml_readReg(unit, CORAL_PCR(src_mac_port), &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        if (reg_data & BIT(CORAL_PCR_PORT_TX_MIR_OFFT + session_id))
        {
            ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
        }

        if (reg_data & BIT(CORAL_PCR_PORT_RX_MIR_OFFT + session_id))
        {
            ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
        }
    }
    return rc;
}
