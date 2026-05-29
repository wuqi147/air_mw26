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

/* FILE NAME:  hal_sco_stag.c
 * PURPOSE:
 *  Implement STAG module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_stag.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_STAG_BUF_LEN             (4)
#define HAL_SCO_STAG_ALIGN_BIT_WIDTH     (8)
#define HAL_SCO_STAG_REPLACE_MODE_MAX_DP (10)

/* cpu tx stag offset */
#define HAL_SCO_STAG_TX_OPC_BIT_OFFSET (5)
#define HAL_SCO_STAG_TX_OPC_BIT_WIDTH  (3)
#define HAL_SCO_STAG_TX_VPM_BIT_OFFSET (0)
#define HAL_SCO_STAG_TX_VPM_BIT_WIDTH  (2)
#define HAL_SCO_STAG_TX_PCP_BIT_OFFSET (5)
#define HAL_SCO_STAG_TX_PCP_BIT_WIDTH  (3)
#define HAL_SCO_STAG_TX_DEI_BIT_OFFSET (4)
#define HAL_SCO_STAG_TX_DEI_BIT_WIDTH  (1)

/* cpu rx stag offset */
#define HAL_SCO_STAG_RX_RSN_BIT_OFFSET (2)
#define HAL_SCO_STAG_RX_RSN_BIT_WIDTH  (3)
#define HAL_SCO_STAG_RX_VPM_BIT_OFFSET (0)
#define HAL_SCO_STAG_RX_VPM_BIT_WIDTH  (2)
#define HAL_SCO_STAG_RX_SP_BIT_OFFSET  (0)
#define HAL_SCO_STAG_RX_SP_BIT_WIDTH   (5)
#define HAL_SCO_STAG_RX_PCP_BIT_OFFSET (5)
#define HAL_SCO_STAG_RX_PCP_BIT_WIDTH  (3)
#define HAL_SCO_STAG_RX_DEI_BIT_OFFSET (4)
#define HAL_SCO_STAG_RX_DEI_BIT_WIDTH  (1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_STAG, "hal_sco_stag.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_sco_stag_init
 * PURPOSE:
 *      This API is used to init STAG
 * INPUT:
 *      unit                 -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_ALREADY_INITED -- Module is reinitialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_init(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_stag_deinit
 * PURPOSE:
 *      This API is used to deinit STAG
 * INPUT:
 *      unit                 -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_NOT_INITED     -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_deinit(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_setPort
 * PURPOSE:
 *      Set the STAG port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      stag_en         --  enable stag
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_setPort(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T stag_en)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));

    /* Write data to register */
    if (TRUE == stag_en)
    {
        u32dat |= BIT(PVC_SPTAG_EN_OFFT);
    }
    else
    {
        u32dat &= ~BIT(PVC_SPTAG_EN_OFFT);
    }
    aml_writeReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_getPort
 * PURPOSE:
 *      Get the STAG port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      stag_en         --  enable stag
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_getPort(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *stag_en)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));
    *stag_en = (u32dat & PVC_SPTAG_EN_MASK) >> PVC_SPTAG_EN_OFFT;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_setMode
 * PURPOSE:
 *      Set the STAG port mode for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  AIR_STAG_MODE_INSERT
 *                      --  AIR_STAG_MODE_REPLACE
 * OUTPUT:
 *      None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_setMode(
    const UI32_T          unit,
    const UI32_T          port,
    const AIR_STAG_MODE_T mode)
{
    UI32_T u32dat, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));

    /* covert to stag_mode_val */
    switch (mode)
    {
        case AIR_STAG_MODE_INSERT:
            u32dat &= ~BIT(PVC_SPTAG_MODE_OFFT);
            break;
        case AIR_STAG_MODE_REPLACE:
            u32dat |= BIT(PVC_SPTAG_MODE_OFFT);
            break;
        default:
            /* do nothing */
            break;
    }

    /* Write data to register */
    aml_writeReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_getMode
 * PURPOSE:
 *      Get the STAG port mode for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      mode            --  AIR_STAG_MODE_INSERT
 *                      --  AIR_STAG_MODE_REPLACE
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_getMode(
    const UI32_T     unit,
    const UI32_T     port,
    AIR_STAG_MODE_T *mode)
{
    UI32_T u32dat, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, PVC(mac_port), &u32dat, sizeof(u32dat));

    /* covert to stag_mode_val */
    u32dat = (u32dat & PVC_SPTAG_MODE_MASK) >> PVC_SPTAG_MODE_OFFT;

    switch (u32dat)
    {
        case 0:
            *mode = AIR_STAG_MODE_INSERT;
            break;
        case 1:
            *mode = AIR_STAG_MODE_REPLACE;
            break;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_encodeTxStag
 * PURPOSE:
 *      Encode tx special tag into buffer.
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  Stag mode
 *      ptr_stag_tx     --  Stag parameters
 *      ptr_buf         --  Buffer address
 *      ptr_len         --  Buffer length
 * OUTPUT:
 *      ptr_len         --  Written buffer length
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_encodeTxStag(
    const UI32_T          unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T   *ptr_stag_tx,
    UI8_T                *ptr_buf,
    UI32_T               *ptr_len)
{
    UI32_T            port = 0, byte_off = 0, bit_off = 0;
    BOOL_T            found = FALSE;
    AIR_PORT_BITMAP_T mac_pbmp;

    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_tx->opc, AIR_STAG_OPC_PORTMAP, AIR_STAG_OPC_LOOKUP);
    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_tx->vpm, AIR_STAG_VPM_UNTAG, AIR_STAG_VPM_TPID_PRE_DEFINED);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_stag_tx->pbm, mac_pbmp);

    /* insert mode only support port map */
    if ((AIR_STAG_MODE_INSERT == mode) &&
        ((ptr_stag_tx->opc != AIR_STAG_OPC_PORTMAP) && (ptr_stag_tx->opc != AIR_STAG_OPC_LOOKUP)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* replace mode, max DP will be limited to 10 */
    if ((AIR_STAG_MODE_REPLACE == mode) && (AIR_STAG_OPC_PORTMAP == ptr_stag_tx->opc))
    {
        AIR_PORT_FOREACH(mac_pbmp, port)
        {
            if (port > HAL_SCO_STAG_REPLACE_MODE_MAX_DP)
            {
                return AIR_E_BAD_PARAMETER;
            }
        }
    }

    /* clear output buffer */
    osal_memset(ptr_buf, 0, HAL_SCO_STAG_BUF_LEN);

    ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->opc, HAL_SCO_STAG_TX_OPC_BIT_OFFSET, HAL_SCO_STAG_TX_OPC_BIT_WIDTH);
    if (AIR_STAG_MODE_INSERT == mode)
    {
        AIR_PORT_FOREACH(mac_pbmp, port)
        {
            byte_off = port / HAL_SCO_STAG_ALIGN_BIT_WIDTH;
            /* byte offset should be in range [0, 3] */
            HAL_CHECK_PARAM((byte_off >= HAL_SCO_STAG_BUF_LEN), AIR_E_BAD_PARAMETER);

            bit_off = port % HAL_SCO_STAG_ALIGN_BIT_WIDTH;

            ptr_buf[HAL_SCO_STAG_BUF_LEN - byte_off - 1] |= (0x1 << bit_off);
        }
    }
    else
    {
        ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->vpm, HAL_SCO_STAG_TX_VPM_BIT_OFFSET, HAL_SCO_STAG_TX_VPM_BIT_WIDTH);
        if (AIR_STAG_OPC_PORTMAP == ptr_stag_tx->opc)
        {
            AIR_PORT_FOREACH(mac_pbmp, port)
            {
                bit_off = port % HAL_SCO_STAG_ALIGN_BIT_WIDTH;
                if (port >= HAL_SCO_STAG_ALIGN_BIT_WIDTH)
                {
                    ptr_buf[0] |= 0x1 << (bit_off + HAL_SCO_STAG_RX_VPM_BIT_WIDTH);
                }
                else
                {
                    ptr_buf[1] |= 0x1 << bit_off;
                }
            }
        }
        else if (AIR_STAG_OPC_PORTID == ptr_stag_tx->opc)
        {
            AIR_PORT_FOREACH(mac_pbmp, port)
            {
                if (TRUE == found)
                {
                    return AIR_E_BAD_PARAMETER;
                }
                ptr_buf[1] |= port;
                found = TRUE;
            }
        }

        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->pri, HAL_SCO_STAG_TX_PCP_BIT_OFFSET, HAL_SCO_STAG_TX_PCP_BIT_WIDTH);
        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->cfi, HAL_SCO_STAG_TX_DEI_BIT_OFFSET, HAL_SCO_STAG_TX_DEI_BIT_WIDTH);
        ptr_buf[2] |=
            BITS_OFF_L((ptr_stag_tx->vid >> HAL_SCO_STAG_ALIGN_BIT_WIDTH), 0,
                       (HAL_SCO_STAG_ALIGN_BIT_WIDTH - HAL_SCO_STAG_TX_PCP_BIT_WIDTH - HAL_SCO_STAG_TX_DEI_BIT_WIDTH));
        ptr_buf[3] |= BITS_OFF_L((ptr_stag_tx->vid & 0xFF), 0, HAL_SCO_STAG_ALIGN_BIT_WIDTH);
    }

    *ptr_len = HAL_SCO_STAG_BUF_LEN;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_stag_decodeRxStag
 * PURPOSE:
 *      Decode rx special tag from buffer.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_buf         --  Buffer address
 *      len             --  Buffer length
 * OUTPUT:
 *      ptr_stag_rx     --  Stag parameters
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_stag_decodeRxStag(
    const UI32_T        unit,
    const UI8_T        *ptr_buf,
    const UI32_T        len,
    AIR_STAG_RX_PARA_T *ptr_stag_rx)
{
    UI32_T port = 0, mac_port = 0;

    ptr_stag_rx->rsn = BITS_OFF_R(ptr_buf[0], HAL_SCO_STAG_RX_RSN_BIT_OFFSET, HAL_SCO_STAG_RX_RSN_BIT_WIDTH);
    ptr_stag_rx->vpm = BITS_OFF_R(ptr_buf[0], HAL_SCO_STAG_RX_VPM_BIT_OFFSET, HAL_SCO_STAG_RX_VPM_BIT_WIDTH);
    ptr_stag_rx->pri = BITS_OFF_R(ptr_buf[2], HAL_SCO_STAG_RX_PCP_BIT_OFFSET, HAL_SCO_STAG_RX_PCP_BIT_WIDTH);
    ptr_stag_rx->cfi = BITS_OFF_R(ptr_buf[2], HAL_SCO_STAG_RX_DEI_BIT_OFFSET, HAL_SCO_STAG_RX_DEI_BIT_WIDTH);
    ptr_stag_rx->vid = BITS_OFF_R(
        ptr_buf[2], 0, (HAL_SCO_STAG_ALIGN_BIT_WIDTH - HAL_SCO_STAG_TX_PCP_BIT_WIDTH - HAL_SCO_STAG_TX_DEI_BIT_WIDTH));
    ptr_stag_rx->vid = (ptr_stag_rx->vid << HAL_SCO_STAG_ALIGN_BIT_WIDTH) | ptr_buf[3];
    mac_port = BITS_OFF_R(ptr_buf[1], HAL_SCO_STAG_RX_SP_BIT_OFFSET, HAL_SCO_STAG_RX_SP_BIT_WIDTH);
    HAL_MAC_PORT_TO_AIR_PORT(unit, mac_port, port);
    ptr_stag_rx->sp = port;

    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_rx->rsn, AIR_STAG_REASON_CODE_NORMAL, AIR_STAG_REASON_CODE_PORT_MOVE_ERR);
    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_rx->vpm, AIR_STAG_VPM_UNTAG, AIR_STAG_VPM_TPID_PRE_DEFINED);

    return AIR_E_OK;
}
