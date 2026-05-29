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

/* FILE NAME:  lp_utils.h
 * PURPOSE:
 *      This file defines the utility functions for loop prevention.
 *
 * NOTES:
 */
#ifndef LP_UTILS_H
#define LP_UTILS_H
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lwip/pbuf.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   lp_port_comparePortId
 * PURPOSE:
 *      This API is used to check if the port number is bigger than the received port number.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *      rcvPort              -- Received port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_port_comparePortId(
    UI16_T chkPort,
    UI16_T rcvPort);

/* FUNCTION NAME:   lp_port_getLinkStatus
 * PURPOSE:
 *      This API is used to check the link state of the port.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_port_getLinkStatus(
    UI16_T chkPort);

/* FUNCTION NAME:   lp_lag_getLinkStatus
 * PURPOSE:
 *      This API is used to check the trunk link state for the given port.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_lag_getLinkStatus(
    UI16_T chkPort);

/* FUNCTION NAME:   lp_lag_getGroupId
 * PURPOSE:
 *      This API is used to parse the trunk group for the given port.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk group index if valid, otherwise LP_TRUNK_NONE
 *
 * NOTES:
 *      None
 */
UI16_T
lp_lag_getGroupId(
    UI16_T chkPort);

/* FUNCTION NAME:   lp_process_rxPacket
 * PURPOSE:
 *      Process the rx lp frame..
 *
 * INPUT:
 *      ptr_pbuf
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lp_process_rxPacket(
    struct pbuf *ptr_pbuf);

/* FUNCTION NAME: lp_updatePortLoopState
 * PURPOSE:
 *      Update the loop prevention port status
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_updatePortLoopState(
    void);

/* FUNCTION NAME:   lp_port_setFlowControl
 * PURPOSE:
 *      Set flow control for a port
 *
 * INPUT:
 *      block            --  TRUE to block, FALSE to unblock
 *      port             --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
lp_port_setFlowControl(
    BOOL_T block,
    UI16_T port);

/* FUNCTION NAME:   lp_port_block
 * PURPOSE:
 *      Block/Unblock port.
 *
 * INPUT:
 *      block            --  TRUE to block, FALSE to unblock
 *      u_portId         --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lp_port_block(
    BOOL_T block,
    UI16_T u_portId);

#ifndef AIR_HARDWARE_SEND_LDF_SUPPORT
/* FUNCTION NAME:   lp_sendLdf
 * PURPOSE:
 *      Send lp detect frame to all ports.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
lp_sendLdf();
#endif /* AIR_HARDWARE_SEND_LDF_SUPPORT  */

/* FUNCTION NAME:   lp_getBlockPbmp
 * PURPOSE:
 *      Get lp block port bitmap.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      pbmp
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
lp_getBlockPbmp(
    AIR_PORT_BITMAP_T pbmp);

/* FUNCTION NAME:   lp_port_convertPortIdToBitmap
 * PURPOSE:
 *      Block/Unblock port.
 *
 * INPUT:
 *      port_id          --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Bitmap value
 *
 * NOTES:
 *      None
 */
UI32_T
lp_port_convertPortIdToBitmap(
    UI16_T port_id);


/* FUNCTION NAME: lp_port_delFromLoopPbmpGroup
 * PURPOSE:
 *      This function is used to delete the port from the to loop bitmap.
 *
 * INPUT:
 *      port             --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
lp_port_delFromLoopPbmpGroup(
    UI16_T port);

/* FUNCTION NAME: to_blk_pbmp_del_por
 * PURPOSE:
 *      This function is used to delete the port from the to block bitmap.
 *
 * INPUT:
 *      port             --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
lp_port_delFromBlockPbmpGroup(
    UI16_T port);
#endif  /* LP_UTILS_H */
