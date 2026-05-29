/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:  sfp_module_handle.h
 * PURPOSE:
 *      1. Process the events of SFP module plugging in and pulling out.
 *      2. Check if the SFP module just plugged in is on the whitelist or not.
 * NOTES:
 *
 */

#ifndef SFP_MODULE_HANDLE_H
#define SFP_MODULE_HANDLE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_error.h"
#include "sfp_task.h"
#include "sfp_msg.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* The high value for a GPIO pin. */
#define SFP_MODULE_PIN_HIGH (1)
/* The low value for a GPIO pin. */
#define SFP_MODULE_PIN_LOW (0)

/* The initial status for a port. */
#define SFP_TRUNK_MEMBER_STATUS_NONE       (0x0)
/* The port is a trunk member */
#define SFP_TRUNK_MEMBER_STATUS_TRUE       (0x1)
/* The port is not a trunk member */
#define SFP_TRUNK_MEMBER_STATUS_FALSE      (0x2)
/* The port is just removed from a trunk */
#define SFP_TRUNK_MEMBER_STATUS_REMOVED    (0x3)

/* The maximum number of errors accepted. */
#define SFP_MODULE_ERROR_COUNT_LIMITATION     (10)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* The port state of a serdes port or a combo serdes port. */
typedef enum
{
    /* No SFP module is inserted. */
    SFP_MODULE_PORT_STATE_ABSENT,
    /* An SFP module is detected. But it has not been handled. */
    SFP_MODULE_PORT_STATE_PRESENT,
    /* An SFP module is detected and configured properly. Even if the SFP module
     * is not on the whitelist, it may be in ACTIVE state as long as it is
     * accessible and can be configured properly.
     */
    SFP_MODULE_PORT_STATE_ACTIVE,
    /* The SFP module inserted is inaccessible or beyond the support of the switch. */
    SFP_MODULE_PORT_STATE_UNKNOWN,
    /* An SFP module is detected and it need another serdes mode or change to
     * combo serdes mode from combo PHY mode. However, serdes mode and COMBO
     * mode are not allowed to change because the port is in a trunk currently.
     */
    SFP_MODULE_PORT_STATE_PENDING,

    /* The number of the definition of this enum. */
    SFP_MODULE_PORT_STATE_LAST
} SFP_MODULE_PORT_STATE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   sfp_module_handleMessage
 * PURPOSE:
 *      Handle the messages related to SFP module handling.
 *
 * INPUT:
 *      ptr_message          -- The message to be handled
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_handleMessage(
    SFP_MSG_T *ptr_message);

/* FUNCTION NAME:   sfp_module_pollingSFPModuleStatus
 * PURPOSE:
 *      Poll the SFP module status for an serdes port or a combo serdes port to
 *      check if there is an SFP module just inserted or removed. The polling
 *      interval is SFP_TASK_TIMER_BASE_INTERVAL_MS.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer points to the port information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_pollingSFPModuleStatus(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info);

/* FUNCTION NAME:   sfp_module_state_getState
 * PURPOSE:
 *      Get the SFP module state for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the SFP module state
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_state_getState(
    UI8_T port,
    SFP_MODULE_PORT_STATE_T *ptr_state);

/* FUNCTION NAME:   sfp_module_state_initPortBasedOnDB
 * PURPOSE:
 *      Initialize the port based on the port settings in DB. Port need be
 *      initalized when its combo mode or serdes mode changes.
 *
 * INPUT:
 *      ptr_port_info        -- A pointer points to the port information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_module_state_initPortBasedOnDB(
    SFP_TASK_PORT_INFO_T *ptr_port_info);

/* FUNCTION NAME:   sfp_module_fakeModuleRemoved
 * PURPOSE:
 *      Fake that an SFP module is removed even if there is no SFP module inserted at all.
 *      It will reset the (COMBO) SFP port and detect the SFP module again if there is one.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_module_fakeModuleRemoved(
    const UI32_T unit,
    const UI32_T port);

void
sfp_module_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port);

#endif /*End of SFP_MODULE_HANDLE_H*/
