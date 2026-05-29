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

/* FILE NAME:  mw_gpio_isr.h
 * PURPOSE:
 *      This file defines the data structure for GPIO interrupt service routine for MW.
 *
 * NOTES:
 */
#ifndef MW_GPIO_ISR_H
#define MW_GPIO_ISR_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef void
(*MW_GPIO_ISR_CALLBACK_FUNC_T)(
    void                    *ptr_arg);

typedef enum
{
    MW_GPIO_ISR_USER_BTN_REST = 0,
#ifdef AIR_SUPPORT_ERPS
    MW_GPIO_ISR_USER_ERPS,
#endif

    MW_GPIO_ISR_USER_LAST
}MW_GPIO_ISR_USER_T;

typedef struct MW_GPIO_ISR_PARAM_S
{
    UI8_T                           in_use;
    MW_GPIO_ISR_CALLBACK_FUNC_T     callback;
    void                            *ptr_arg;
} MW_GPIO_ISR_PARAM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   mw_gpio_isr_register
 * PURPOSE:
 *      This API is used to register a GPIO ISR callback.
 *
 * INPUT:
 *      user                 -- user ID
 *      callback             -- callback function
 *      ptr_arg              -- argument to pass to the callback function
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ALREADY_INITED
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
mw_gpio_isr_register(
    UI8_T                           user,
    MW_GPIO_ISR_CALLBACK_FUNC_T     callback,
    void                            *ptr_arg);

/* FUNCTION NAME:   mw_gpio_isr_unregister
 * PURPOSE:
 *      This API is used to unregister a GPIO ISR callback.
 *
 * INPUT:
 *      user                 -- user ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_INITED
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
mw_gpio_isr_unregister(
    UI8_T       user);

    /* FUNCTION NAME:   mw_gpio_isr_user_is_registered
 * PURPOSE:
 *      This API is used to check if a user is registered for GPIO ISR.
 *
 * INPUT:
 *      user                 -- user ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_NOT_INITED
 *      MW_E_ALREADY_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
mw_gpio_isr_user_is_registered(
    UI8_T       user);

#endif  /* MW_GPIO_ISR_H */
