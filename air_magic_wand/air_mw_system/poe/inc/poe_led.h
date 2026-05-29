/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  poe_led.h
 * PURPOSE:
 * It provides PoE Led API and definitions.
 *
 * NOTES:
 */

#ifndef POE_LED_H
#define POE_LED_H
/* INCLUDE FILE DECLARATIONS
 */
#include "poe_info.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* Page 0x1 */
#define POE_SYS_INIT_AND_CURR_OVERLOAD_EVENT             (0x84)

#define POE_PSE_CURR_OVERLOAD_EVENT_OFFSET               (5)
#define POE_PSE_CURR_OVERLOAD_EVENT_RELMASK              (0x00000001)
#define POE_PSE_CURR_OVERLOAD_EVENT_MASK                 (POE_PSE_CURR_OVERLOAD_EVENT_RELMASK << POE_PSE_CURR_OVERLOAD_EVENT_OFFSET)
#define POE_PSE_CURR_OVERLOAD_EVENT                      (1)
#define POE_PSE_CURR_OVERLOAD_EVENT_LEN                  (1)
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
*/
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   poe_maxLed_handler
 * PURPOSE:
 *      PoE Max LED handler function.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_poe_control_block         -- Pointer to the related PoE data
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
poe_maxLed_handler(
    POE_CB_T    *ptr_poe_control_block);

/* FUNCTION NAME:   poe_maxLed_init
 * PURPOSE:
 *      PoE Max LED initialization function.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_maxLed_init(
    void);
#endif /* End of POE_LED_H */