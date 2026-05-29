/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  mw_sif_clock.h
 * PURPOSE:
 *      This file contains the declaration of setting sif clock functionality for the Magic Wand module.
 *
 * NOTES:
 */
#ifndef MW_SIF_CLOCK_H
#define MW_SIF_CLOCK_H
/* INCLUDE FILE DECLARATIONS
 */
#if (!defined(AIR_LITE_MW))
#include <mw_utils.h>
#include "osapi.h"
#include "air_sif.h"
#ifdef AIR_LITE_MW
#include <hal/switch/pearl/hal_pearl_sif.h>
#elif AIR_EN_CORAL
#include <hal/switch/coral/hal_coral_sif.h>
#else
#include <hal/switch/sco/hal_sco_sif.h>
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_LITE_MW
#define MW_SIF_CLOCK_MASTER_COUNT       (HAL_PEARL_SIF_MASTER_COUNT)
#elif AIR_EN_CORAL
#define MW_SIF_CLOCK_MASTER_COUNT       (HAL_CORAL_SIF_MASTER_COUNT)
#else
#define MW_SIF_CLOCK_MASTER_COUNT       (HAL_SCO_SIF_MASTER_COUNT)
#endif
#define MW_SIF_CLOCK_WAITTIME           (0xFFFFFFFF)
#define MW_SIF_OPERATION_RETRY_COUNT    (3)
#define MW_SIF_CLOCK_MODULE_NAME        "mw-sif"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum MW_SIF_CLOCK_TYPE_S
{
    MW_SIF_CLOCK_TYPE_100K = 0,
    MW_SIF_CLOCK_TYPE_400K,
    MW_SIF_CLOCK_TYPE_1M,

    MW_SIF_CLOCK_TYPE_LAST
}MW_SIF_CLOCK_TYPE_T;

typedef struct MW_SIF_CONTEXT_S
{
    semaphorehandle_t           sif_mutex;
    MW_SIF_CLOCK_TYPE_T         sif_clock[MW_SIF_CLOCK_MASTER_COUNT];
}MW_SIF_CONTEXT_T;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   mw_sif_init
 * PURPOSE:
 *      This API is used to initialize the mw sif module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_init(
    void);

/* FUNCTION NAME:   mw_sif_read
 * PURPOSE:
 *      This API is used to read data by sif with specific clock.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_read(
    const UI32_T            unit,
    MW_SIF_CLOCK_TYPE_T     clock,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param);

/* FUNCTION NAME:   mw_sif_write
 * PURPOSE:
 *      This API is used to write data by sif with specific clock.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_sif_write(
    const UI32_T            unit,
    MW_SIF_CLOCK_TYPE_T     clock,
    const AIR_SIF_INFO_T    *ptr_sif_info,
    AIR_SIF_PARAM_T         *ptr_sif_param);
#endif
#endif  /* MW_SIF_CLOCK_H */
