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

/* FILE NAME:  hal_coral_stag.c
 * PURPOSE:
 *  Implement STAG module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_stag.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_STAG_BUF_LEN             (4)
#define HAL_CORAL_STAG_ALIGN_BIT_WIDTH     (8)
#define HAL_CORAL_STAG_REPLACE_MODE_MAX_DP (10)

/* cpu tx stag offset */
#define HAL_CORAL_STAG_TX_OPC_BIT_OFFSET (5)
#define HAL_CORAL_STAG_TX_OPC_BIT_WIDTH  (3)
#define HAL_CORAL_STAG_TX_VPM_BIT_OFFSET (0)
#define HAL_CORAL_STAG_TX_VPM_BIT_WIDTH  (2)
#define HAL_CORAL_STAG_TX_PCP_BIT_OFFSET (5)
#define HAL_CORAL_STAG_TX_PCP_BIT_WIDTH  (3)
#define HAL_CORAL_STAG_TX_DEI_BIT_OFFSET (4)
#define HAL_CORAL_STAG_TX_DEI_BIT_WIDTH  (1)

/* cpu rx stag offset */
#define HAL_CORAL_STAG_RX_RSN_BIT_OFFSET (2)
#define HAL_CORAL_STAG_RX_RSN_BIT_WIDTH  (3)
#define HAL_CORAL_STAG_RX_VPM_BIT_OFFSET (0)
#define HAL_CORAL_STAG_RX_VPM_BIT_WIDTH  (2)
#define HAL_CORAL_STAG_RX_SP_BIT_OFFSET  (0)
#define HAL_CORAL_STAG_RX_SP_BIT_WIDTH   (7)
#define HAL_CORAL_STAG_RX_PCP_BIT_OFFSET (5)
#define HAL_CORAL_STAG_RX_PCP_BIT_WIDTH  (3)
#define HAL_CORAL_STAG_RX_DEI_BIT_OFFSET (4)
#define HAL_CORAL_STAG_RX_DEI_BIT_WIDTH  (1)

/* cpu tx stag opc portmap bypass trunk translation */
#define HAL_CORAL_STAG_TX_OPC_INSERT_MODE_PORTMAP_BYPASS_TRUNK (0x1)

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
DIAG_SET_MODULE_INFO(AIR_MODULE_STAG, "hal_coral_stag.c");

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_stag_init
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
hal_coral_stag_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, port = 0, mac_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_STAG) & HAL_INIT_STAGE(unit))
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            /*
             * Enable Bypass trunk feature of CPU tag.
             * Note: The CPU tag feature remains disabled by default.
             */
            AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
            {
                HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

                aml_readReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));
                u32dat |= BIT(CORAL_PVC_CPU_BYPASS_TRUNK_OFFT);
                aml_writeReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_STAG) |= HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_stag_deinit
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
hal_coral_stag_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (!(HAL_MODULE_INITED(unit, AIR_MODULE_STAG) & HAL_INIT_STAGE(unit)))
    {
        rc = AIR_E_NOT_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            /* Do nothing */
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_STAG) &= ~HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_stag_setPort
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
hal_coral_stag_setPort(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T stag_en)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));

    /* Write data to register */
    if (TRUE == stag_en)
    {
        u32dat |= BIT(CORAL_PVC_SPTAG_EN_OFFT);
    }
    else
    {
        u32dat &= ~BIT(CORAL_PVC_SPTAG_EN_OFFT);
    }
    aml_writeReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_stag_getPort
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
hal_coral_stag_getPort(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *stag_en)
{
    UI32_T u32dat = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));
    *stag_en = (u32dat & CORAL_PVC_SPTAG_EN_MASK) >> CORAL_PVC_SPTAG_EN_OFFT;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_stag_setMode
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
hal_coral_stag_setMode(
    const UI32_T          unit,
    const UI32_T          port,
    const AIR_STAG_MODE_T mode)
{
    UI32_T u32dat, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));

    /* covert to stag_mode_val */
    switch (mode)
    {
        case AIR_STAG_MODE_INSERT:
            u32dat &= ~BIT(CORAL_PVC_SPTAG_MODE_OFFT);
            break;
        case AIR_STAG_MODE_REPLACE:
            u32dat |= BIT(CORAL_PVC_SPTAG_MODE_OFFT);
            break;
        default:
            /* do nothing */
            break;
    }

    /* Write data to register */
    aml_writeReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_stag_getMode
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
hal_coral_stag_getMode(
    const UI32_T     unit,
    const UI32_T     port,
    AIR_STAG_MODE_T *mode)
{
    UI32_T u32dat, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &u32dat, sizeof(u32dat));

    /* covert to stag_mode_val */
    u32dat = (u32dat & CORAL_PVC_SPTAG_MODE_MASK) >> CORAL_PVC_SPTAG_MODE_OFFT;

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

/* FUNCTION NAME: hal_coral_stag_encodeTxStag
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
hal_coral_stag_encodeTxStag(
    const UI32_T          unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T   *ptr_stag_tx,
    UI8_T                *ptr_buf,
    UI32_T               *ptr_len)
{
    UI32_T            port = 0, byte_off = 0, bit_off = 0;
    BOOL_T            found = FALSE;
    AIR_PORT_BITMAP_T mac_pbmp;

    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_tx->opc, AIR_STAG_OPC_PORTMAP, AIR_STAG_OPC_PORTID_BYPASS_TRUNK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_tx->vpm, AIR_STAG_VPM_UNTAG, AIR_STAG_VPM_TPID_PRE_DEFINED);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_stag_tx->pbm, mac_pbmp);

    /* insert mode only support port map */
    if ((AIR_STAG_MODE_INSERT == mode) &&
        ((ptr_stag_tx->opc != AIR_STAG_OPC_PORTMAP) && (ptr_stag_tx->opc != AIR_STAG_OPC_LOOKUP) &&
         (ptr_stag_tx->opc != AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* replace mode, max DP will be limited to 10 */
    if ((AIR_STAG_MODE_REPLACE == mode) &&
        ((AIR_STAG_OPC_PORTMAP == ptr_stag_tx->opc) || (AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK == ptr_stag_tx->opc)))
    {
        AIR_PORT_FOREACH(mac_pbmp, port)
        {
            if (port > HAL_CORAL_STAG_REPLACE_MODE_MAX_DP)
            {
                return AIR_E_BAD_PARAMETER;
            }
        }
    }

    /* clear output buffer */
    if (*ptr_len != HAL_CORAL_STAG_BUF_LEN)
    {
        return AIR_E_BAD_PARAMETER;
    }
    osal_memset(ptr_buf, 0, *ptr_len);

    if ((AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK == ptr_stag_tx->opc) ||
        (AIR_STAG_OPC_PORTID_BYPASS_TRUNK == ptr_stag_tx->opc))
    {
        if (AIR_STAG_MODE_INSERT == mode)
        {
            if (AIR_STAG_OPC_PORTID_BYPASS_TRUNK == ptr_stag_tx->opc)
            {
                return AIR_E_BAD_PARAMETER;
            }
            /* Need to translate OPC due to CORAL defined OPC 0x1 is bypass trunk by portmap in insert mode.
             */
            ptr_stag_tx->opc = HAL_CORAL_STAG_TX_OPC_INSERT_MODE_PORTMAP_BYPASS_TRUNK;
        }
        else
        {
            /* Need to translate OPC due to CORAL defined OPC 0x2 is bypass trunk by portmap,
             * OPC 0x3 is bypass trunk by port id in replace mode.
             */
            ptr_stag_tx->opc -= 0x1;
        }
    }
    else if (AIR_STAG_OPC_LOOKUP == ptr_stag_tx->opc)
    {
        /* Translate OPC to the last */
        ptr_stag_tx->opc = AIR_STAG_OPC_LAST;
    }
    else
    {
        /* do nothing */
    }

    ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->opc, HAL_CORAL_STAG_TX_OPC_BIT_OFFSET, HAL_CORAL_STAG_TX_OPC_BIT_WIDTH);
    if (AIR_STAG_MODE_INSERT == mode)
    {
        AIR_PORT_FOREACH(mac_pbmp, port)
        {
            byte_off = port / HAL_CORAL_STAG_ALIGN_BIT_WIDTH;
            /* byte offset should be in range [0, 3] */
            HAL_CHECK_PARAM((byte_off >= HAL_CORAL_STAG_BUF_LEN), AIR_E_BAD_PARAMETER);

            bit_off = port % HAL_CORAL_STAG_ALIGN_BIT_WIDTH;

            ptr_buf[HAL_CORAL_STAG_BUF_LEN - byte_off - 1] |= (0x1 << bit_off);
        }
    }
    else
    {
        ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->vpm, HAL_CORAL_STAG_TX_VPM_BIT_OFFSET, HAL_CORAL_STAG_TX_VPM_BIT_WIDTH);
        switch (ptr_stag_tx->opc)
        {
            case AIR_STAG_OPC_PORTMAP:
            case AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK:
                AIR_PORT_FOREACH(mac_pbmp, port)
                {
                    bit_off = port % HAL_CORAL_STAG_ALIGN_BIT_WIDTH;
                    if (port >= HAL_CORAL_STAG_ALIGN_BIT_WIDTH)
                    {
                        ptr_buf[0] |= 0x1 << (bit_off + HAL_CORAL_STAG_RX_VPM_BIT_WIDTH);
                    }
                    else
                    {
                        ptr_buf[1] |= 0x1 << bit_off;
                    }
                }
                break;
            case AIR_STAG_OPC_PORTID:
            case AIR_STAG_OPC_PORTID_BYPASS_TRUNK:
                AIR_PORT_FOREACH(mac_pbmp, port)
                {
                    if (TRUE == found)
                    {
                        return AIR_E_BAD_PARAMETER;
                    }
                    ptr_buf[1] |= port;
                    found = TRUE;
                }
                break;
            default:
                /* do nothing */
                break;
        }

        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->pri, HAL_CORAL_STAG_TX_PCP_BIT_OFFSET, HAL_CORAL_STAG_TX_PCP_BIT_WIDTH);
        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->cfi, HAL_CORAL_STAG_TX_DEI_BIT_OFFSET, HAL_CORAL_STAG_TX_DEI_BIT_WIDTH);
        ptr_buf[2] |= BITS_OFF_L(
            (ptr_stag_tx->vid >> HAL_CORAL_STAG_ALIGN_BIT_WIDTH), 0,
            (HAL_CORAL_STAG_ALIGN_BIT_WIDTH - HAL_CORAL_STAG_TX_PCP_BIT_WIDTH - HAL_CORAL_STAG_TX_DEI_BIT_WIDTH));
        ptr_buf[3] |= BITS_OFF_L((ptr_stag_tx->vid & 0xFF), 0, HAL_CORAL_STAG_ALIGN_BIT_WIDTH);
    }
    *ptr_len = HAL_CORAL_STAG_BUF_LEN;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_stag_decodeRxStag
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
hal_coral_stag_decodeRxStag(
    const UI32_T        unit,
    const UI8_T        *ptr_buf,
    const UI32_T        len,
    AIR_STAG_RX_PARA_T *ptr_stag_rx)
{
    UI32_T port = 0, mac_port = 0;

    ptr_stag_rx->rsn = BITS_OFF_R(ptr_buf[0], HAL_CORAL_STAG_RX_RSN_BIT_OFFSET, HAL_CORAL_STAG_RX_RSN_BIT_WIDTH);
    ptr_stag_rx->vpm = BITS_OFF_R(ptr_buf[0], HAL_CORAL_STAG_RX_VPM_BIT_OFFSET, HAL_CORAL_STAG_RX_VPM_BIT_WIDTH);
    ptr_stag_rx->pri = BITS_OFF_R(ptr_buf[2], HAL_CORAL_STAG_RX_PCP_BIT_OFFSET, HAL_CORAL_STAG_RX_PCP_BIT_WIDTH);
    ptr_stag_rx->cfi = BITS_OFF_R(ptr_buf[2], HAL_CORAL_STAG_RX_DEI_BIT_OFFSET, HAL_CORAL_STAG_RX_DEI_BIT_WIDTH);
    ptr_stag_rx->vid = BITS_OFF_R(
        ptr_buf[2], 0,
        (HAL_CORAL_STAG_ALIGN_BIT_WIDTH - HAL_CORAL_STAG_TX_PCP_BIT_WIDTH - HAL_CORAL_STAG_TX_DEI_BIT_WIDTH));
    ptr_stag_rx->vid = (ptr_stag_rx->vid << HAL_CORAL_STAG_ALIGN_BIT_WIDTH) | ptr_buf[3];
    mac_port = BITS_OFF_R(ptr_buf[1], HAL_CORAL_STAG_RX_SP_BIT_OFFSET, HAL_CORAL_STAG_RX_SP_BIT_WIDTH);
    HAL_MAC_PORT_TO_AIR_PORT(unit, mac_port, port);
    ptr_stag_rx->sp = port;

    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_rx->rsn, AIR_STAG_REASON_CODE_NORMAL, AIR_STAG_REASON_CODE_PORT_MOVE_ERR);
    HAL_CHECK_MIN_MAX_RANGE(ptr_stag_rx->vpm, AIR_STAG_VPM_UNTAG, AIR_STAG_VPM_TPID_PRE_DEFINED);

    return AIR_E_OK;
}
