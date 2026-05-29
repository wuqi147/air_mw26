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

/* FILE NAME:  air_stag.h
 * PURPOSE:
 *      It provides STAG module API.
 * NOTES:
 */

#ifndef AIR_STAG_H
#define AIR_STAG_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_STAG_MODE_INSERT,
    AIR_STAG_MODE_REPLACE,
    AIR_STAG_MODE_LAST
} AIR_STAG_MODE_T;

typedef enum
{
    /* Egress DP is port map */
    AIR_STAG_OPC_PORTMAP,

    /* Egress DP is port id */
    AIR_STAG_OPC_PORTID,

    /* Forward the packet according to lookup result */
    AIR_STAG_OPC_LOOKUP,

    /* Egress DP is port map bypass trunk */
    AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK,

    /* Egress DP is port id bypass trunk */
    AIR_STAG_OPC_PORTID_BYPASS_TRUNK,
    AIR_STAG_OPC_LAST
} AIR_STAG_OPC_T;

typedef enum
{
    AIR_STAG_REASON_CODE_NORMAL,
    AIR_STAG_REASON_CODE_SFLOW,
    AIR_STAG_REASON_CODE_TTL_ERR,
    AIR_STAG_REASON_CODE_ACL,
    AIR_STAG_REASON_CODE_SA_FULL,
    AIR_STAG_REASON_CODE_PORT_MOVE_ERR,
    AIR_STAG_REASON_CODE_LAST,
} AIR_STAG_REASON_CODE_T;

typedef enum
{
    AIR_STAG_VPM_UNTAG,
    AIR_STAG_VPM_TPID_8100,
    AIR_STAG_VPM_TPID_88A8,
    AIR_STAG_VPM_TPID_PRE_DEFINED,
    AIR_STAG_VPM_LAST,
} AIR_STAG_VPM_T;

typedef struct AIR_STAG_TX_PARA_S
{
    /* destination port operation code */
    AIR_STAG_OPC_T    opc;

    /* tag attribute */
    AIR_STAG_VPM_T    vpm;

    /* destination port map */
    AIR_PORT_BITMAP_T pbm;

    /* PRI in vlan tag */
    UI16_T            pri : 3;

    /* CFI in vlan tag */
    UI16_T            cfi : 1;

    /* VID in vlan tag */
    UI16_T            vid : 12;
} AIR_STAG_TX_PARA_T;

typedef struct AIR_STAG_RX_PARA_S
{
    /* reason code */
    AIR_STAG_REASON_CODE_T rsn;

    /* tag attribute */
    AIR_STAG_VPM_T         vpm;

    /* source port */
    UI32_T                 sp;

    /* PRI in vlan tag */
    UI16_T                 pri : 3;

    /* CFI in vlan tag */
    UI16_T                 cfi : 1;

    /* VID in vlan tag */
    UI16_T                 vid : 12;
} AIR_STAG_RX_PARA_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_stag_setSpecialTagCfg
 * PURPOSE:
 *      Set the STAG port state for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- Enable stag
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_setSpecialTagCfg(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_stag_getSpecialTagCfg
 * PURPOSE:
 *      Set the STAG port state for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- Enable stag
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_getSpecialTagCfg(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_stag_setSpecialTagMode
 * PURPOSE:
 *      Set the STAG port mode for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the stag mode
 *                                  AIR_STAG_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_setSpecialTagMode(
    const UI32_T          unit,
    const UI32_T          port,
    const AIR_STAG_MODE_T mode);

/* FUNCTION NAME:   air_stag_getSpecialTagMode
 * PURPOSE:
 *      Get the STAG port mode for a specifiec port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the stag mode
 *                                  AIR_STAG_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_getSpecialTagMode(
    const UI32_T     unit,
    const UI32_T     port,
    AIR_STAG_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_stag_encodeTxStag
 * PURPOSE:
 *      Encode tx special tag into buffer.
 * INPUT:
 *      unit                     -- Device ID
 *      mode                     -- Stag mode
 *                                  AIR_STAG_MODE_T
 *      ptr_stag_tx              -- Stag parameters
 *                                  AIR_STAG_TX_PARA_T
 *      ptr_buf                  -- Buffer address
 *      ptr_len                  -- Buffer length
 * OUTPUT:
 *      ptr_len                  -- Written buffer length
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_encodeTxStag(
    const UI32_T          unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T   *ptr_stag_tx,
    UI8_T                *ptr_buf,
    UI32_T               *ptr_len);

/* FUNCTION NAME:   air_stag_decodeRxStag
 * PURPOSE:
 *      Decode rx special tag from buffer.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_buf                  -- Buffer address
 *      len                      -- Buffer length
 * OUTPUT:
 *      ptr_stag_rx              -- Stag parameters
 *                                  AIR_STAG_RX_PARA_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_stag_decodeRxStag(
    const UI32_T        unit,
    const UI8_T        *ptr_buf,
    const UI32_T        len,
    AIR_STAG_RX_PARA_T *ptr_stag_rx);

#endif /* End of AIR_STAG_H */
