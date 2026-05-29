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

/* FILE NAME:  igmp_snoop_msg.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop messages.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_msg.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop.h"
#include "osapi_message.h"

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

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   igmp_snp_msg_clearEntry
 * PURPOSE:
 *      This API is used to trigger igmp snp task to clear group entry and dynamic mrouter entry by specified type.
 *
 * INPUT:
 *      ptr_data      --  pointer of clear entry message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_INITED
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_msg_clearEntry(
   const IGMP_SNP_MSG_CLEAR_ENTRY_T *ptr_data)
{
    IGMP_SNP_MSG_T      *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              total_size = 0;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if(FALSE == ptr_igmp_snp->cfg_info.enable)
    {
        return MW_E_NOT_INITED;
    }
    MW_CHECK_PTR(ptr_data);
    total_size = sizeof(IGMP_SNP_MSG_T) + sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T);
    osapi_calloc(total_size, IGMP_SNP_MODULE_NAME, (void**)&ptr_msg);
    if(NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    ptr_msg->msg_id = IGMP_SNP_MSG_CLEAR_ENTRY;
    osapi_memcpy(ptr_msg->data, ptr_data, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));

    rc = osapi_msgSend(IGMP_SNP_QUEUE_NAME, (const UI8_T *) ptr_msg, 0, 0);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
    }
    IGMP_SNP_LOG_INFO("clear group entry, type:%d, portbmp:0x%x, vid:%d", ptr_data->type, ptr_data->portbmp[0], ptr_data->vid);
    return rc;
}

#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
/* FUNCTION NAME:   igmp_snp_msg_updatePortGroupEntry
 * PURPOSE:
 *      This API is used to trigger igmp snp task to update port's group entry.
 *
 * INPUT:
 *      portbmp      --  Port bit map of to be cleared.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_msg_updatePortGroupEntry(
    MW_PORT_BITMAP_T portbmp)
{
    IGMP_SNP_MSG_T *ptr_msg = NULL;
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI16_T         total_size = 0;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;

    ptr_igmp_snp = igmp_snp_getConfig();
    if(FALSE == ptr_igmp_snp->cfg_info.enable)
    {
        return MW_E_NOT_INITED;
    }
    total_size = sizeof(IGMP_SNP_MSG_T) + sizeof(MW_PORT_BITMAP_T);
    osapi_calloc(total_size, IGMP_SNP_MODULE_NAME, (void**)&ptr_msg);
    if(NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    ptr_msg->msg_id = IGMP_SNP_MSG_UPDATE_PORT_GROUP_ENTRY;
    osapi_memcpy(ptr_msg->data, portbmp, sizeof(MW_PORT_BITMAP_T));
    rc = osapi_msgSend(IGMP_SNP_QUEUE_NAME, (const UI8_T *) ptr_msg, 0, 0);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
    }
    IGMP_SNP_LOG_INFO("update port group entry, portbmp:0x%x", portbmp[0]);
    return rc;
}
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

