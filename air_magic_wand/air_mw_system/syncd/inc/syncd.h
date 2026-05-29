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

/* FILE NAME:  syncd.h
 * PURPOSE:
 *      It provides syncd moudle functions.
 *
 * NOTES:
 */

#ifndef _SYNCD_H_
#define _SYNCD_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    SYNCD_TMR_ENABLE_BIT_PORT = 0,
    SYNCD_TMR_ENABLE_BIT_MIB,
    SYNCD_TMR_ENABLE_BIT_LAST
}SYNCD_TMR_ENABLE_BIT_T;

#define SYNCD_TMR_ENABLE_PORT   BIT(SYNCD_TMR_ENABLE_BIT_PORT)
#define SYNCD_TMR_ENABLE_MIB    BIT(SYNCD_TMR_ENABLE_BIT_MIB)

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: syncd_init
 * PURPOSE:
 *      Initialization of syncd daemon.
 *
 * INPUT:
 *      None
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
syncd_init(
    void);

/* FUNCTION NAME: syncd_log_set_debug
 * PURPOSE:
 *      Set debug function for external application.
 *
 * INPUT:
 *      lv_mask     --  Debugging level, defined in SYNCD_DBG_LV_MASK_T
 *      state       --  State of debugging function of SyncD
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_log_set_debug(
    const UI32_T lv_mask,
    const BOOL_T  state);

/* FUNCTION NAME:   syncd_timer_setFlag
 * PURPOSE:
 *      Set flags of syncd timer.
 *
 * INPUT:
 *      flags       --  Enable bits of timer in SyncD
 *      state       --  State of timer in SyncD
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_timer_setFlag(
    const UI32_T flags,
    const BOOL_T state);

#endif  /*_SYNCD_H_*/
