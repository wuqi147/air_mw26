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

/* FILE NAME:  poe_db_relate.h
 * PURPOSE:
 * It provides PoE DB API and definitions.
 *
 * NOTES:
 */

#ifndef POE_DB_RELATE_H
#define POE_DB_RELATE_H
/* INCLUDE FILE DECLARATIONS
 */
#include <poe_info.h>
#include <db_api.h>
/* NAMING CONSTANT DECLARATIONS
 */
/* Page 0x0 */
#define POE_808AR_V24_CHECK_VERSION                      (0xFF)         /* Only 808ARv24 has it */
/* Page 0x1 */
#define POE_AN8502_IP804AR_TOTAL_POWER_LIMITER_EVENT     (0x60)
#define POE_AN8503_TOTAL_POWER_LIMITER_EVENT             (0x8A)         /* Only AN8503 have it */
#define POE_808AR_V24_808BR_DISABLE_IVT_AUTO_CLEAR       (0xD6)         /* Only 808ARv24 and 804BR have it */
/* Page 0x2 */
#define POE_I2C_BUS_TIMEOUT_CONTROL                      (0xC7)

#define POE_REG_ADDRESS_MASK                             (0xFF)
#define POE_TIMEOUT_SELETCION_OFFSET                     (0)
#define POE_TIMEOUT_SELETCION_RELMASK                    (0x00000003)
#define POE_TIMEOUT_SELETCION_MASK                       (POE_TIMEOUT_SELETCION_RELMASK << POE_TIMEOUT_SELETCION_OFFSET)
#define POE_CHECK_VERSION_OFFSET                         (1)
#define POE_CHECK_VERSION_RELMASK                        (0x00000001)
#define POE_CHECK_VERSION_MASK                           (POE_CHECK_VERSION_RELMASK << POE_CHECK_VERSION_OFFSET)

#define POE_DEVICE_ID_IP808AR                            (0x0000808A)
#define POE_DEVICE_ID_AN8503                             (0x00008503)
#define POE_DEVICE_ID_IP804AR                            (0x000004A2)
#define POE_DEVICE_ID_AN8502                             (0x000004B0)
#define POE_TIMEOUT_SELECTION_DISABLE                    (0)
#define POE_DISABLE_IVT_AUTO_CLEAR_OFFSET                (1)
#define POE_DISABLE_IVT_AUTO_CLEAR                       (1)            /* 1: Disable, 0: Enable */
#define POE_NO_POWER_EVENT                               (0)
#define POE_INSUFFICIENT_POWER                           (10)
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* The PoE port information table */
typedef struct POE_PORT_INFO_S
{
    UI8_T           unit;           /* switch unit */
    UI8_T           port;           /* air port */
    UI8_T           poe_port;
    UI8_T           poe_device_idx; /* PoE device idx */
} POE_PORT_INFO_T;

typedef struct POE_DEVICE_INFO_S
{
    UI16_T          revision_id;
    UI32_T          insufficient_pbmp;
} POE_DEVICE_INFO_T;
/* GLOBAL VARIABLE DECLARATIONS
*/
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: poe_db_handleMsg
 * PURPOSE:
 *      Process message received from DB
 *
 * INPUT:
 *      method                     -- method of the message
 *      t_idx                      -- index of the Table
 *      f_idx                      -- index of the field
 *      e_idx                      -- index of the entry
 *      ptr_data                   -- pointer to the data
 *      size                       -- size of the data
 *
 * OUTPUT:
 *      pptr_out_msg               -- pointer to the output message
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_db_sendMsgToDB(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg);

/* FUNCTION NAME: poe_db_handleMsg
 * PURPOSE:
 *      Process message received from DB
 *
 * INPUT:
 *      ptr_msg                    -- pointer to the database message
 *
 * OUTPUT:
 *      ptr_poe_control_block      -- Pointer to the related PoE data
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
poe_db_handleMsg(
    POE_CB_T *ptr_poe_control_block,
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: poe_db_periodicallyUpdate
 * PURPOSE:
 *      Periodically update PoE DB.
 *
 * INPUT:
 *      dur_tick                   -- Duration in system ticks
 *
 * OUTPUT:
 *      ptr_poe_control_block      -- Pointer to the related PoE data
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
poe_db_periodicallyUpdate(
    const UI32_T     dur_tick,
    POE_CB_T        *ptr_poe_control_block);

/* FUNCTION NAME: poe_chip_init
 * PURPOSE:
 *      Initialization PoE chip
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
poe_chip_init(
    void);

/* FUNCTION NAME: poe_db_init
 * PURPOSE:
 *      Initialization db chip
 *
 * INPUT:
 *      ptr_poe_control_block -- pointer to the control block
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
poe_db_init(
    POE_CB_T *ptr_poe_control_block);

/* FUNCTION NAME: poe_db_deinit
 * PURPOSE:
 *      Deinitialization resource.
 *
 * INPUT:
 *      None
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
poe_db_deinit(
    void);
#endif /* End of POE_DB_RELATE_H */