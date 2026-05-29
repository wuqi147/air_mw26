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
#include "web.h"
#include "db_api.h"

#if LWIP_HTTPD
err_t cgi_set_handle_conf_backup(struct fs_file *file)
{
    unsigned int file_size = S_32K;
    unsigned char *ptr_temp = (unsigned char *)(SystemConfigBase + (SystemConfigSize >> 1));

    if (file == NULL)
    {
        return ERR_ARG;
    }
    if (MW_E_OK != dbapi_getCfgFile(ptr_temp, &file_size))
    {
        CGI_LOG_ERROR(SYSTEM, "Startup config file does not exist.");
        return ERR_ARG;
    }
#if LWIP_HTTPD_CUSTOM_FILES
    file->is_custom_file = 0;
#endif /* LWIP_HTTPD_CUSTOM_FILES */
    file->data = (const char *)ptr_temp;
    file->len = file_size;
    file->index = file_size;
    file->pextension = NULL;
    file->flags = FS_FILE_FLAGS_HEADER_HTTPVER_1_1 | FS_FILE_FLAGS_HEADER_PERSISTENT;

    return ERR_OK;
}
#endif

MW_ERROR_NO_T cgi_set_handle_conf_restore(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    unsigned int file_size = S_32K;
    unsigned char *ptr_temp = (unsigned char *)TempSystemBase;

    if(!strcmp(pcValue[0], "End"))
    {
        /* Check the temp-config is valid */
        if (MW_E_OK != dbapi_getCfgFile(ptr_temp, &file_size))
        {
            CGI_LOG_ERROR(SYSTEM, "Invalid config file.");
            _imgUploadStatus = FW_E_ERROR_CRC;
            return MW_E_OK;
        }

        /* Copy to startup-config partition */
        CGI_LOG_DEBUG(SYSTEM, "Config file size: %u", file_size);
        if (MW_E_OK != flash_partition_copy(TempSystem, SystemConfig1, file_size))
        {
            CGI_LOG_ERROR(SYSTEM, "Cannot write to startup config partition.");
            _imgUploadStatus = FW_E_ERROR_WRITE;
            return MW_E_OK;
        }
        _imgUploadStatus = FW_E_SUCCESS;
    }

    return MW_E_OK;
}

