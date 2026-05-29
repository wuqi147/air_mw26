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
/* FILE NAME:   upgrade.c
 * PURPOSE:
 *      SSI and CGI function of switch firmware upgrade web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#ifdef AIR_SUPPORT_MQTTD
#include "mqttd.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
FW_STATE_T _imgUploadStatus = FW_E_SUCCESS;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_fuState_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    if(FW_E_ERASE_WRITE == _imgUploadStatus)
    {
        update_upgrade_flag(FW_UPGRADE_PROCESS);
    }
    *ptr_length = 0;

    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_chunk_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T len = 0;

    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags,
                        "<script>var SLICE_SIZE = %d;</script>", LWIP_HTTPD_MAX_SESSION_LEN);
    *ptr_length = len;

    return err;
}

MW_ERROR_NO_T
ssi_get_eraseTime_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T len = 0;

    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, "%d", AIR_WEBPAGE_UPGRADE_TIME);
    *ptr_length = len;

    return err;
}

MW_ERROR_NO_T
ssi_get_fw_info_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T len = 0;

    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, "%d", _imgUploadStatus);
    *ptr_length = len;

    return err;
}

MW_ERROR_NO_T
cgi_set_handle_fwupdate(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    if(!strcmp(ptr_pcValue[0], "End"))
    {
        if(FW_CRC_MATCH == crc_check((unsigned char *)TempSystemBase))
        {
            CGI_LOG_DEBUG(SYSTEM, "FW CRC check OK.");
            _imgUploadStatus = FW_E_ERASE_WRITE;
#if AIR_SUPPORT_MQTTD
            if (TRUE == mqttd_get_state())
            {
                UI8_T enable = FALSE;
                if (MW_E_OK != httpd_queue_setData(M_UPDATE, MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(enable)))
                {
                    CGI_LOG_DEBUG(SYSTEM, "Update DB mqttd_cfg_info failed");
                }
            }
#endif
        }
        else
        {
            CGI_LOG_DEBUG(SYSTEM, "FW CRC32 is mismatch");
            _imgUploadStatus = FW_E_ERROR_CRC;
        }
    }

    return MW_E_OK;
}
