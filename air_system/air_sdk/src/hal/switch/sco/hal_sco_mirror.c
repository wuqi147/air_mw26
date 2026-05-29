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

/* FILE NAME:  hal_sco_mir.c
 * PURPOSE:
 *  Implement Mirror module.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_mirror.h>

#include <air_cfg.h>
#include <aml/aml.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_MAX_MIRROR_SESSION        (4)
#define HAL_SCO_MIR_REG_DEFAULT_VALUE (0)
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_MIR, "hal_sco_mirror.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
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
    const UI32_T unit)
{
    UI32_T regMIR, session;

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    for (session = 0; session < AIR_MAX_MIRROR_SESSION; session++)
    {
        /* Set mirroring port */
        regMIR &= ~BITS_RANGE(MIR_MIRROR_PORT_OFFSER(session), MIR_MIRROR_PORT_LEN);
        regMIR |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, MIR_MIRROR_PORT_OFFSER(session), MIR_MIRROR_PORT_LEN);
    }
    /* Write MIR */
    aml_writeReg(unit, MIR, &regMIR, sizeof(regMIR));

    return AIR_E_OK;
}

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
    const UI32_T unit)
{
    UI32_T u32dat;

    u32dat = HAL_SCO_MIR_REG_DEFAULT_VALUE;
    aml_writeReg(unit, MIR, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

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
    const AIR_MIR_SESSION_T *ptr_session)
{
    UI32_T regMIR, regPCR;
    UI32_T dst_mac_port, src_mac_port;
    BOOL_T enable = FALSE, tx_tag_enable = FALSE;

    /* Mistake proofing */
    HAL_CHECK_ENUM_RANGE(session_id, AIR_MAX_MIRROR_SESSION);
    HAL_CHECK_PORT(unit, ptr_session->dst_port);

    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->dst_port, dst_mac_port);
    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    /* Set mirroring port */
    regMIR &= ~BITS_RANGE(MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    regMIR |= BITS_OFF_L(dst_mac_port, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);

    /* Set mirroring port tx tag state */
    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG)
    {
        tx_tag_enable = TRUE;
    }
    regMIR &= ~BITS_RANGE(MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);
    regMIR |= BITS_OFF_L(tx_tag_enable, MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);

    /* Set mirroring port state */
    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_ENABLE)
    {
        enable = TRUE;
    }
    regMIR &= ~BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);
    regMIR |= BITS_OFF_L(enable, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, &regMIR, sizeof(regMIR));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:dst_port=%d, flags=0x%x, regMIR=0x%x \n", dst_mac_port, ptr_session->flags, regMIR);

    /* Read PCR */
    aml_readReg(unit, PCR(src_mac_port), &regPCR, sizeof(regPCR));

    /* Set mirroring source port */
    regPCR &= ~BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    regPCR &= ~BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
    {
        regPCR |= BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    }

    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
    {
        regPCR |= BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    }

    /* Write PCR */
    aml_writeReg(unit, PCR(src_mac_port), &regPCR, sizeof(regPCR));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:src_port=%d, flags=0x%x, regPCR=0x%x \n", src_mac_port, ptr_session->flags, regPCR);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_delSession
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
hal_sco_mir_delSession(
    const UI32_T unit,
    const UI32_T session_id)
{
    UI32_T regMIR;

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    /* Set mirroring port */
    regMIR &= ~BITS_RANGE(MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    regMIR |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    /* Set mirroring port tx tag state */
    regMIR &= ~BITS_RANGE(MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);
    /* Set mirroring port state */
    regMIR &= ~BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, &regMIR, sizeof(regMIR));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_getSession
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
hal_sco_mir_getSession(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session)
{
    UI32_T regMIR;
    UI32_T dst_mac_port;
    BOOL_T enable, tx_tag_enable;

    /* Mistake proofing */
    HAL_CHECK_ENUM_RANGE(session_id, AIR_MAX_MIRROR_SESSION);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    /* Get mirroring port */
    dst_mac_port = BITS_OFF_R(regMIR, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);

    /* Get mirroring port state */
    enable = BITS_OFF_R(regMIR, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Get mirroring tx tag state*/
    tx_tag_enable = BITS_OFF_R(regMIR, MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);

    HAL_MAC_PORT_TO_AIR_PORT(unit, dst_mac_port, ptr_session->dst_port);
    if (enable)
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    if (tx_tag_enable)
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_setSessionAdminMode
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
hal_sco_mir_setSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T enable)
{
    UI32_T regMIR;

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    /* Set mirroring port state */
    regMIR &= ~BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);
    regMIR |= BITS_OFF_L(enable, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, &regMIR, sizeof(regMIR));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_getSessionAdminMode
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
hal_sco_mir_getSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T      *ptr_enable)
{
    UI32_T regMIR;
    BOOL_T enable;

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR, sizeof(regMIR));

    /* Get mirroring port state */
    enable = BITS_OFF_R(regMIR, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    *ptr_enable = enable;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_setMirrorPort
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
hal_sco_mir_setMirrorPort(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{
    UI32_T regPCR;
    UI32_T src_mac_port;

    /* Mistake proofing */
    HAL_CHECK_PORT(unit, ptr_session->src_port);

    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    /* Read data from register */
    aml_readReg(unit, PCR(src_mac_port), &regPCR, sizeof(regPCR));

    regPCR &= ~BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    regPCR &= ~BIT(PCR_PORT_RX_MIR_OFFT + session_id);

    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
    {
        regPCR |= BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    }

    if (ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
    {
        regPCR |= BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:src_port=%d, flags=0x%x regPCR=0x%x \n", src_mac_port, ptr_session->flags, regPCR);
    /* Write data to register */
    aml_writeReg(unit, PCR(src_mac_port), &regPCR, sizeof(regPCR));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mir_getMirrorPort
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
hal_sco_mir_getMirrorPort(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session)
{
    UI32_T regPCR;
    UI32_T src_mac_port;

    /* Mistake proofing */
    HAL_CHECK_PORT(unit, ptr_session->src_port);

    HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_session->src_port, src_mac_port);

    /* Read data from register */
    aml_readReg(unit, PCR(src_mac_port), &regPCR, sizeof(regPCR));

    if (regPCR & BIT(PCR_PORT_TX_MIR_OFFT + session_id))
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
    }
    else
    {
        ptr_session->flags &= ~(AIR_MIR_SESSION_FLAGS_DIR_TX);
    }

    if (regPCR & BIT(PCR_PORT_RX_MIR_OFFT + session_id))
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else
    {
        ptr_session->flags &= ~(AIR_MIR_SESSION_FLAGS_DIR_RX);
    }

    return AIR_E_OK;
}
