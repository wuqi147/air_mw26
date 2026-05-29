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

/* FILE NAME:  sfp_db.h
 * PURPOSE:
 *      DB operation for SFP.
 * NOTES:
 *
 */

#ifndef SFP_DB_H
#define SFP_DB_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_error.h"
#include "air_port.h"
#include "db_api.h"
#include "air_init.h"
#include "sfp_task.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Unknown serdes mode. It is used for unsupported SFP module. */
#define SFP_DB_PORT_MODE_SERDES_MODE_UNKNOWN (0x07)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   sfp_db_updatePortMode
 * PURPOSE:
 *      Update combo_mode and serdes_mode flags of port_oper_mode DB field.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 *      combo_mode           -- The new combo mode to be updated
 *      serdes_mode          -- The new serdes mode to be updated
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *      AIR_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
sfp_db_updatePortMode(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    AIR_PORT_COMBO_MODE_T combo_mode,
    AIR_PORT_SERDES_MODE_T serdes_mode);

/* FUNCTION NAME:   sfp_db_updateSFPInfo
 * PURPOSE:
 *      Update the information of an SFP module to DB.
 *
 * INPUT:
 *      index                -- The entry ID
 *      ptr_module_info_db   -- A pointer to DB data
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_db_updateSFPInfo(
    const UI16_T index,
    DB_SFP_MODULE_INFO_T *ptr_module_info_db);

/* FUNCTION NAME:   sfp_db_queue_create
 * PURPOSE:
 *      Create a queue to communicate with DB queue.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_db_queue_create(
    void);

/* FUNCTION NAME:   sfp_db_queue_free
 * PURPOSE:
 *      Free the queue created by sfp_db_queue_create().
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *
 * NOTES:
 *      Refer to sfp_db_queue_create().
 */
AIR_ERROR_NO_T
sfp_db_queue_free(
    void);

/* FUNCTION NAME:   sfp_db_queue_getData
 * PURPOSE:
 *      1. Calculate db data size based on tid,fid,eid and then alloc memory
 *      2. Send a message to DB queue to get data and receive data from DB queue
 *
 * INPUT:
 *      t_idx               -- The enum of the table
 *      f_idx               -- The enum of the field
 *      e_idx               -- The entry index in the table
 * OUTPUT:
 *      pptr_out_msg        -- A double pointer to the received DB message
 *      ptr_out_size        -- A pointer to the size of ptr_data
 *      pptr_out_data       -- A double pointer to the DB data of the DB message
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *
 * NOTES:
 *      When return AIR_E_OK, caller need to free the memory which is pointed by *pptr_out_msg!
 */
AIR_ERROR_NO_T
sfp_db_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data);
#endif/* End of SFP_DB_H */

