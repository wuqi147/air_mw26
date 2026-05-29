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

/* FILE NAME:  sfp_trunk.h
 * PURPOSE:
 *      Handle trunk for SFP ports.
 * NOTES:
 *
 */

#ifndef SFP_TRUNK_H
#define SFP_TRUNK_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_error.h"
#include "sfp_task.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
AIR_ERROR_NO_T
sfp_trunk_init(
    UI32_T unit);


/* FUNCTION NAME:   sfp_trunk_check
 * PURPOSE:
 *      Check if a port is a trunk member. If it is, obtain all trunk member also.
 *
 * INPUT:
 *      port               -- The port number
 *      ptr_trunk_id       -- A pointer returns the trunk ID
 *      ptr_trunk_member   -- A pointer returns the trunk member
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK           -- It is a trunk member
 *      Other              -- It is not a trunk member
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_trunk_check(
    UI32_T port,
    UI32_T *ptr_trunk_id,
    UI32_T *ptr_trunk_member);

BOOL_T
sfp_trunk_determineComboSerdesMode(
    const UI32_T unit,
    const UI32_T port);

void
sfp_trunk_handleLinkStatusChange(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void *ptr_cookie);

AIR_ERROR_NO_T
sfp_trunk_getComboSerdes(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode,
    AIR_PORT_SERDES_MODE_T *ptr_serdes_mode);

void
sfp_trunk_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port);

#endif /* End of SFP_TRUNK_H */

