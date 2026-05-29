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

/* FILE NAME:  mw_sfp_led.h
 * PURPOSE:
 *      It provide SFP LED application API in MW.
 * NOTES:
 */

#ifndef MW_SFP_LED_H
#define MW_SFP_LED_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_led.h"
#include "mw_error.h"
#include "mw_led.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T product_id;
    UI8_T count;
    const SFP_LED_PORT_MAP_T *ptr_portMap;
} MW_SFP_LED_PORT_MAP_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
/* FUNCTION NAME: mw_sfpLed_getPortStatus
 *
 * PURPOSE:
 *      Get sfp port status.
 * INPUT:
 *      unit                --  Device unit number
 *      port                --  Index of port number
 * OUTPUT:
 *      ptr_status          --  Strcut of the port status
 *                              SFP_LED_PORT_STATUS_T
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
mw_sfpLed_getPortStatus(
    const UI32_T            unit,
    const UI32_T            port,
    SFP_LED_PORT_STATUS_T   *ptr_status);

void
mw_sfpLed_setPortMap(
    const MW_SFP_LED_PORT_MAP_T *ptr_portMap);

const UI32_T
mw_sfpLed_getLedCount(
    void);

MW_ERROR_NO_T
mw_sfpLed_getPortPin(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T led_id,
    UI8_T *ptr_pin);

MW_ERROR_NO_T
mw_sfpLed_getPortActiveType(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T led_id,
    MW_LED_ACTIVE_TYPE_T *ptr_active_type);

#endif /* defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED) */
#endif /* MW_SFP_LED_H */
