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

/* FILE NAME:  hal_sco_sflow.h
 * PURPOSE:
 *  Define sFlow module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_SFLOW_H
#define HAL_SCO_SFLOW_H

/* INCLUDE FILE DECLARTIONS
 */
#include "air_error.h"
#include "air_port.h"
#include "air_types.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_SFLOW_MIN_SAMPLING_RATE (0)
#define HAL_SCO_SFLOW_MAX_SAMPLING_RATE (BITS(0, (SFLOW_SAMPLING_RATE_LENG - 1))) /* 0xffffff */
#define HAL_SCO_SFLOW_MIN_SAMPLING_NUM  (1)
#define HAL_SCO_SFLOW_MAX_SAMPLING_NUM  (0xff)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: hal_sco_sflow_init
 * PURPOSE:
 *      Initialization of sFlow.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_sflow_deinit
 * PURPOSE:
 *      Deinitialization of sFlow.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_sflow_setSampling
 * PURPOSE:
 *      Set sFlow sampling rate, number
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      rate            --  sFlow sampling rate
 *      number          --  sFlow sampling number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1.  Don't set sflow sampling on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_sflow_setSampling(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T rate,
    const UI32_T number);

/* FUNCTION NAME: hal_sco_sflow_getSampling
 * PURPOSE:
 *      Get sFlow sampling rate, number.
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 * OUTPUT:
 *      ptr_rate        --  sFlow sampling rate
 *      ptr_number      --  sFlow sampling number
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sflow_getSampling(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_rate,
    UI32_T      *ptr_number);

/* FUNCTION NAME: hal_sco_sflow_setMngFrm
 * PURPOSE:
 *      Set sFlow sampling to include/exclude management frame
 * INPUT:
 *      unit            --  Select device ID
 *      state           --  FALSE: Include management frames
 *                          TRUE: Exclude management frames
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 * NOTES:
 *      1. This API is internal now.
 *          Because MRD didn't require this function, HEC didn't verify this part.
 *          Currently, it occur expected counter when setting state state to exclude.
 */
AIR_ERROR_NO_T
hal_sco_sflow_setMngFrm(
    const UI32_T unit,
    const BOOL_T state);

/* FUNCTION NAME: hal_sco_sflow_getMngFrm
 * PURPOSE:
 *      Get sFlow sampling state of include/exclude management frame.
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_state       --  FALSE: Include management frames
 *                          TRUE: Exclude management frames
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 * NOTES:
 *      1. This API is internal now.
 *          Because MRD didn't require this function, HEC didn't verify this part.
 *          Currently, it occur expected counter when setting state state to exclude.
 */
AIR_ERROR_NO_T
hal_sco_sflow_getMngFrm(
    const UI32_T unit,
    BOOL_T      *ptr_state);

#endif /* end of HAL_SCO_SFLOW_H */
