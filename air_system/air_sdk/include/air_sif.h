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

/* FILE NAME:  air_sif.h
 * PURPOSE:
 *      It provides SIF module API.
 * NOTES:
 */
#ifndef AIR_SIF_H
#define AIR_SIF_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_SIF_MAX_ADDR_LEN (4)
#define AIR_SIF_MAX_DATA_LEN (256)

/* DATA TYPE DECLARATIONS
 */

typedef struct AIR_SIF_INFO_S
{
    UI16_T channel;
    UI16_T slave_id;
} AIR_SIF_INFO_T;

typedef struct AIR_SIFM_PARAM_S
{
    /* Target device subaddress */
    UI32_T addr_len;
    UI32_T addr;

    UI32_T data_len;

    /* data */
    union
    {
        UI32_T  data;
        UI32_T *ptr_data;
    } info;
} AIR_SIF_PARAM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_sif_write
 * PURPOSE:
 *      This API is used to do the I2C write operation
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sif_write(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   air_sif_read
 * PURPOSE:
 *      This API is used to do the I2C read operation
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sif_read(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   air_sif_writeByRemote
 * PURPOSE:
 *      This API is used to do the I2C write operation
 *      by remote device
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_remote_info  -- Pointer of remote sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation is invalid.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sif_writeByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   air_sif_readByRemote
 * PURPOSE:
 *      This API is used to do the I2C read operation
 *      by remote device
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_sif_info         -- Pointer of sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_remote_info  -- Pointer of remote sif information
 *                              AIR_SIF_INFO_T
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * OUTPUT:
 *      ptr_sif_param        -- Pointer of sif parameter
 *                              AIR_SIF_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- Other errors.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_OP_INVALID     -- Operation failed.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sif_readByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

#endif
