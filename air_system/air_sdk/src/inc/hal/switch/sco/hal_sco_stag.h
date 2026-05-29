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

/* FILE NAME:  hal_sco_stag.h
 * PURPOSE:
 *  Define STAG module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_STAG_H
#define HAL_SCO_STAG_H

/* INCLUDE FILE DECLARTIONS
 */
#include "air_error.h"
#include "air_port.h"
#include "air_stag.h"
#include "air_types.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI32_T unit);

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
    const UI32_T unit);

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
    const BOOL_T stag_en);

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
    BOOL_T      *stag_en);

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
    const AIR_STAG_MODE_T mode);

/* FUNCTION NAME: hal_sco_stag_setMode
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
    AIR_STAG_MODE_T *mode);

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
    UI32_T               *ptr_len);

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
    AIR_STAG_RX_PARA_T *ptr_stag_rx);

#endif /* end of HAL_SCO_STAG_H */
