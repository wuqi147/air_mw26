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

/* FILE NAME:   cli_db.h
 * PURPOSE:
 *      It provides cli db API and definitions.
 * NOTES:
 *
 */
#ifndef CLI_DB_H
#define CLI_DB_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T
cli_queue_send_to_db(
    DB_MSG_T *ptr_msg,
    UI32_T size);

MW_ERROR_NO_T
cli_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg);

MW_ERROR_NO_T
cli_queue_recv(
    const C8_T *ptr_q_name,
    void **pptr_buf);

MW_ERROR_NO_T
cli_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data);

/* FUNCTION NAME:   cli_queue_setData
 * PURPOSE:
 *      package message and call sending function to DB directly.
 *
 * INPUT:
 *      method      --  the method bitmap
 *      t_idx       --  the enum of the table
 *      f_idx       --  the enum of the field
 *      e_idx       --  the entry index in the table
 *      ptr_data    --  pointer to message data
 *      size        --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
cli_queue_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size);

void
cli_db_handle_msg(
    DB_MSG_T *ptr_msg);
#endif