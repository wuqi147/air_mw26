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

/* FILE NAME:  hal_coral_stp.c
 * PURPOSE:
 *  Implement STP module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_STP, "hal_coral_stp.c");

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_stp_setPortState
 * PURPOSE:
 *      Set the STP port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      fid             --  Filter ID for MSTP
 *      state           --  AIR_STP_STATE_DISABLE
 *                          AIR_STP_STATE_LISTEN
 *                          AIR_STP_STATE_LEARN
 *                          AIR_STP_STATE_FORWARD
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_stp_setPortState(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          fid,
    const AIR_STP_STATE_T state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0;

    /* translate air port to hardware mac port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do stp set action port=%d fid=%d state=%d \n", mac_port, fid, state);

    /* Read data from register */
    aml_readReg(unit, CORAL_SSC(mac_port), &u32dat, sizeof(u32dat));

    /* Write data to register */
    u32dat &= ~BITS(fid * 2, (fid * 2) + 1);
    u32dat |= BITS_OFF_L(state, (fid * 2), 2);
    aml_writeReg(unit, CORAL_SSC(mac_port), &u32dat, sizeof(u32dat));

    return rc;
}

/* FUNCTION NAME: hal_coral_stp_getPortState
 * PURPOSE:
 *      Get the STP port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      fid             --  Filter ID for MSTP
 *
 * OUTPUT:
 *      ptr_state       --  AIR_STP_STATE_DISABLE
 *                          AIR_STP_STATE_LISTEN
 *                          AIR_STP_STATE_LEARN
 *                          AIR_STP_STATE_FORWARD
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
hal_coral_stp_getPortState(
    const UI32_T     unit,
    const UI32_T     port,
    const UI32_T     fid,
    AIR_STP_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0;

    /* translate air port to hardware mac port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_SSC(mac_port), &u32dat, sizeof(u32dat));
    (*ptr_state) = BITS_OFF_R(u32dat, fid * 2, 2);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do stp get action port=%d fid=%d state=%d \n", mac_port, fid, *ptr_state);

    return rc;
}
