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

/* FILE NAME:  igmp_snoop_db.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping database.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_DB_H
#define IGMP_SNOOP_DB_H
/* INCLUDE FILE DECLARATIONS
 */
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMP_SNP_DB_QUEUE_LENGTH                (AIR_MAX_IGMP_DB_QUEUE_LEN)
#define MW_IGMP_SNP_DB_IDX_MAP_SIZE             MW_BITMAP_SIZE(MAX_L2MC_NUM)

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_IGMP_SNP_DB_IDX_MAP_FOREACH(bitmap, db_idx)           \
                for(db_idx = 0; db_idx < MAX_L2MC_NUM; db_idx++) \
                    if(MW_PORT_CHK(bitmap, db_idx))

#define MW_IGMP_SNP_DB_IDX_MAP_CLEAR(bitmap) do                 \
    {                                                           \
        UI32_T i;                                               \
        for(i = 0; i < MW_IGMP_SNP_DB_IDX_MAP_SIZE; i++)        \
        {                                                       \
            ((bitmap)[i] = 0);                                  \
        }                                                       \
    }while(0)

/* DATA TYPE DECLARATIONS
 */
typedef UI32_T MW_IGMP_SNP_DB_IDX_MAP_T[MW_IGMP_SNP_DB_IDX_MAP_SIZE];

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: igmp_snp_db_queue_send
 * PURPOSE:
 *      Package the message and send it to DB. If ptr_queue_name is NULL, DB
 *      will not send any response and the message itself will be freed by DB.
 *
 * INPUT:
 *      ptr_queue_name          --  A pointer to the queue that DB will return a
 *                                  response to. If it is NULL, no response is expected.
 *      method                  --  the method bitmap
 *      t_idx                   --  the enum of the table
 *      f_idx                    --  the enum of the field
 *      e_idx                   --  the entry index in the table
 *      ptr_data                --  A pointer to the message data
 *      timeout                 --  The maximun amout of time the thread will wait for
 *                                  send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
igmp_snp_db_queue_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI32_T timeout);

/* FUNCTION NAME: igmp_snp_db_queue_sendWithRsp
 * PURPOSE:
 *      Package the message and send it to DB with response.
 *
 * INPUT:
 *      method                  --  the method bitmap
 *      t_idx                   --  the enum of the table
 *      f_idx                    --  the enum of the field
 *      e_idx                   --  the entry index in the table
 *      ptr_data                --  A pointer to the message data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
igmp_snp_db_queue_sendWithRsp(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data);

/* FUNCTION NAME: igmp_snp_db_initVariable
 * PURPOSE:
 *      Initialize the IGMP SNP DB variables.
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
igmp_snp_db_initVariable(
    void);

/* FUNCTION NAME:   igmp_snp_db_getFreeIdx
 * PURPOSE:
 *      This API is used to get the free database index for IGMP snooping.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Free database index for IGMP snooping
 *
 * NOTES:
 *      None
 */
UI16_T
igmp_snp_db_getFreeIdx(
    void);

/* FUNCTION NAME:   igmp_snp_db_freeDbIdx
 * PURPOSE:
 *      This API is used to free database index for IGMP snooping.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Free database index for IGMP snooping
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_db_freeDbIdx(
    UI16_T db_idx);

/* FUNCTION NAME:   igmp_snp_db_appendMsgPayload
 * PURPOSE:
 *      This API is used to append DB message payload for L2MC entry.
 *
 * INPUT:
 *      ptr_grp_data            -- pointer to the group entry
 *      method                  -- The method of this request message
 *      pptr_msg                -- A double pointer to the DB message
 *      ptr_msg_size            -- A pointer to the size of the DB message
 *      pptr_shifted_payload    -- A double pointer to the position of the payload
 *                                 to store the current request. If it is the first
 *                                 request, the start address of the ptr_payload
 *                                 member of the DB message should be inputted
 *
 *
 *
 * OUTPUT:
 *      pptr_msg                -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      ptr_msg_size            -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      pptr_shifted_payload    -- A double pointer returns the position of the
 *                                 payload to store the next request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_db_appendMsgPayload(
    void *ptr_grp_data,
    const UI8_T method,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload);

/* FUNCTION NAME: igmp_snp_db_subscribeModule
 * PURPOSE:
 *      Subscribe to the database module
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
igmp_snp_db_subscribeModule(
    void);

/* FUNCTION NAME: igmp_snp_db_handleMsg
 * PURPOSE:
 *      Handle the database message.
 *
 * INPUT:
 *      ptr_msg                 --  pointer to the database message
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
igmp_snp_db_handleMsg(
    DB_MSG_T *ptr_msg);

#endif  /* IGMP_SNOOP_DB_H */
