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

/* FILE NAME:  hal_pearl_sif.h
 * PURPOSE:
 *  Define SIF module HAL function.
 *
 * NOTES:
 *
 */
#ifndef HAL_PEARL_SIF_H
#define HAL_PEARL_SIF_H
/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_sif.h>
#include <air_types.h>
#include <osal/osal.h>

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_PEARL_SIF_MASTER_COUNT (2)

/* Maximum Page size*/
#define HAL_PEARL_SIF_MAX_PAGE_SIZE (8)

/* DATA TYPE DECLARATIONS
 */
/* This enumerator is for the clock setting API,
   however, clock setting API haven't open yet */
typedef enum AIR_SIF_CLK_SPEED_E
{
    AIR_SIF_CLK_STANDARD_MODE = 0,
    AIR_SIF_CLK_FAST_MODE,
    AIR_SIF_CLK_FAST_MODE_PLUS,
    AIR_SIF_CLK_LAST,
} HAL_PEARL_SIF_CLK_SPEED_T;

/* This enum number depends on the i2c master number
 */
typedef enum HAL_PEARL_SIF_DEVICE_E
{
    HAL_PEARL_SIF_LOCAL_DEVICE = 0,
    HAL_PEARL_SIF_REMOTE_DEVICE,
    HAL_PEARL_SIF_DEVICE_LAST
} HAL_PEARL_SIF_DEVICE_T;

typedef enum HAL_PEARL_SIFM_STATUS_E
{
    HAL_PEARL_SIFM_SLAVE = 0,
    HAL_PEARL_SIFM_MASTER,
    HAL_PEARL_SIFM_LAST
} HAL_PEARL_SIFM_STATUS_T;

typedef enum HAL_PEARL_SIFM_OP_E
{
    HAL_PEARL_SIFM_WRITE = 0,
    HAL_PEARL_SIFM_READ,
    HAL_PEARL_SIFM_BIT_LAST
} HAL_PEARL_SIFM_OP_T;

typedef struct HAL_PEARL_SIF_INFO_S
{
    UI16_T channel;
    UI16_T slave_id;
    UI16_T remote_channel;
    UI16_T remote_slave_id;
} HAL_PEARL_SIF_INFO_T;

typedef struct HAL_PEARL_SIF_PARAM_S
{
    UI32_T               addr_len;
    UI32_T               addr;
    UI32_T               data_len;
    UI32_T              *ptr_data;
    HAL_PEARL_SIF_INFO_T device_info;
} HAL_PEARL_SIF_PARAM_T;

/* This enum start number is according to  programming guide
 * Configuration Register 1, 0x10008044 and 0x10022044
 */
typedef enum HAL_PEARL_SIF_CR1_OP_MODE_E
{
    HAL_PEARL_SIF_START_BIT = 1,
    HAL_PEARL_SIF_WRITE_DATA,
    HAL_PEARL_SIF_STOP_BIT,
    HAL_PEARL_SIF_READ_FINAL_BIT,
    HAL_PEARL_SIF_READ_BIT,
    HAL_PEARL_SIF_BIT_LAST
} HAL_PEARL_SIF_CR1_OP_MODE_T;

/* Local function declaration
 */

/* FUNCTION NAME:   hal_pearl_sif_init
 * PURPOSE:
 *      Initialize sif module.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sif_init(
    const UI32_T unit);

/* FUNCTION NAME:   hal_pearl_sif_deinit
 * PURPOSE:
 *      Deinitialize sif module
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sif_deinit(
    const UI32_T unit);

/* FUNCTION NAME:   hal_pearl_sif_write
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
hal_pearl_sif_write(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   hal_pearl_sif_read
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
hal_pearl_sif_read(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   hal_pearl_sif_writeByRemote
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
hal_pearl_sif_writeByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

/* FUNCTION NAME:   hal_pearl_sif_readByRemote
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
hal_pearl_sif_readByRemote(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_sif_info,
    const AIR_SIF_INFO_T *ptr_sif_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);
#endif