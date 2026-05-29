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

/* FILE NAME:  sfp_auto_adaptation.h
 * PURPOSE:
 *      1. The SFP SDA PIN driver and/or the SFP SDA IO driver are not initialized successfully. Adopt the SFP auto-adaption algorithm..
 *      2. Check if the SFP module just plugged and automatically adapt to SerDes mode.
 * NOTES:
 *
 */

#ifndef SFP_AUTO_ADAPTION_H
#define SFP_AUTO_ADAPTION_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_port.h"
#include "air_cfg.h"
#include <hal/common/hal_phy.h>
#ifdef AIR_LITE_MW
#include <hal/switch/pearl/hal_pearl_serdes.h>
#elif AIR_EN_CORAL
#include <hal/switch/coral/hal_coral_serdes.h>
#else
#include <hal/switch/sco/hal_sco_serdes.h>
#endif
#include "sfp_task.h"

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_LITE_MW
#define SFP_SERDES_RX_CTRL_14                         (HAL_PEARL_SERDES_RX_CTRL_14)
#define SFP_SERDES_RX_CTRL_14_BIT23_SIGDET_CAL_RDY    (1 << 23)

#define SFP_SERDES_PCS_STATE_REG_2                    (HAL_PEARL_SERDES_PCS_STATE_REG_2)
#define SFP_SERDES_PCS_STATE_REG_2_BIT5_RXSYNC        (1 << 5)
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
#define SFP_SERDES_PCS_STATE_AN_DONE_BIT0             (1 << 0)
#endif

#define SFP_SERDES_AN_REG_5                           (HAL_PEARL_SERDES_AN_REG_5)
#define SFP_SERDES_AN_REG_5_BIT0                      (1 << 0)
#define SFP_SERDES_AN_REG_5_BIT15                     (1 << 15)
#define SFP_SERDES_ID_LAST                            (HAL_PEARL_SERDES_ID_LAST)
/* AIR_LITE_MW */
#elif AIR_EN_CORAL
#define SFP_SERDES_RX_CTRL_14                         (HAL_CORAL_SERDES_RX_CTRL_14)
#define SFP_SERDES_RX_CTRL_14_BIT23_SIGDET_CAL_RDY    (1 << 23)

#define SFP_SERDES_PCS_STATE_REG_2                    (HAL_CORAL_SERDES_PCS_STATE_REG_2)
#define SFP_SERDES_PCS_STATE_REG_2_BIT5_RXSYNC        (1 << 5)
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
#define SFP_SERDES_PCS_STATE_AN_DONE_BIT0             (1 << 0)
#endif

#define SFP_SERDES_AN_REG_5                           (HAL_CORAL_SERDES_AN_REG_5)
#define SFP_SERDES_AN_REG_5_BIT0                      (1 << 0)
#define SFP_SERDES_AN_REG_5_BIT15                     (1 << 15)
#define SFP_SERDES_ID_LAST                            (HAL_CORAL_SERDES_ID_LAST)
#define SFP_SERDES_DEBUG_XPON_0                       (DEBUG_XPON_0)
#define SFP_SERDES_RO_DEBUG_0_BIT30_CDR_LCK2DATA      (1 << 30)

/* AIR_EN_CORAL */
#else
#define SFP_SERDES_RO_DEBUG_0                         (0xA500)
#define SFP_SERDES_RO_DEBUG_0_BIT0_CDR_LCK2REF        (1 << 0)

#define SFP_SERDES_PCS_STATE_REG_2                    (SERDES_PCS_STATE_REG_2)
#define SFP_SERDES_PCS_STATE_REG_2_BIT5_RXSYNC        (1 << 5)
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
#define SFP_SERDES_PCS_STATE_AN_DONE_BIT0             (1 << 0)
#endif

#define SFP_SERDES_AN_REG_5                           (SERDES_AN_REG_5)
#define SFP_SERDES_AN_REG_5_BIT0                      (1 << 0)
#define SFP_SERDES_AN_REG_5_BIT15                     (1 << 15)
#define SFP_SERDES_ID_LAST                            (SERDES_ID_LAST)
#endif


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFP_TASK_PORT_ADAPTION_STATE_IDLE,
    SFP_TASK_PORT_ADAPTION_STATE_TRY,
    SFP_TASK_PORT_ADAPTION_STATE_ACTIVE,

    SFP_TASK_PORT_ADAPTION_STATE_LAST
} SFP_TASK_PORT_ADAPTION_STATE_T;

/* FUNCTION NAME:   sfp_adaptation_polling
 * PURPOSE:
 *      Polling SerDes register state information for each SFP port about PCS state and AN state to determine what SerDes mode to set.
 * INPUT:
 *      unit                 -- Device unit number
 *      port_index           -- The SFP id index
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *      AIR_E_NOT_INITED     -- Not initialized
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_OTHERS         -- Other errors.
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_adaptation_polling(
    const UI32_T unit,
    UI8_T port_index,
    SFP_TASK_PORT_INFO_T *ptr_port_info);

/* FUNCTION NAME:   sfp_auto_adaptation_getState
 * PURPOSE:
 *      Get the SFP auto_adaptation state for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the SFP auto_adaptation state
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_auto_adaptation_getState(
    UI8_T port,
    SFP_TASK_PORT_ADAPTION_STATE_T *ptr_state);

MW_ERROR_NO_T
sfp_auto_adaption_setState(
    const UI32_T unit,
    const UI32_T port,
    UI8_T state);

MW_ERROR_NO_T
sfp_auto_adaption_init(
    const UI32_T unit);

void
sfp_trunk_handleTrunkDelete(
    const UI32_T unit,
    const UI32_T port);

MW_ERROR_NO_T
sfp_adaption_readReg(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI32_T *ptr_reg_val);

void
sfp_auto_adaption_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port);

#endif /* End of SFP_AUTO_ADAPTION_H */
