/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "mw_log.h"
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"

#define BANDWIDTH_BUF_SIZE 308

MW_ERROR_NO_T cgi_set_handle_bandWidthControl(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

    UI32_T i, pbm = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T ingSpeed = 0, egrSpeed = 0;
    UI8_T ingSpeedUpdate = FALSE, egrSpeedUpdate = FALSE;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0L;
#endif
    /* get value from url params */
    for(i=0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "selMask")){
            pbm = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "ingressSpeed")){
            ingSpeed = atoi(pcValue[i]);
            ingSpeedUpdate = TRUE;
        }
        if(!strcmp(pcParam[i], "egressSpeed")){
            egrSpeed = atoi(pcValue[i]);
            egrSpeedUpdate = TRUE;
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    /* parser params to db format */
    if(pbm != 0)
    {
#if(defined(AIR_SUPPORT_SFP))
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&pbm,0);
#endif
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(pbm & 1 << i)
            {
                if(ingSpeedUpdate){
                    CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> port = %d, ingress speed = %d\n", __func__, __LINE__, i+1, ingSpeed);
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_INGRESS_RATE, i+1, &ingSpeed, sizeof(ingSpeed));
                }
                if(egrSpeedUpdate){
                    CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> port = %d, egrSpeed speed = %d\n", __func__, __LINE__, i+1, egrSpeed);
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_EGRESS_RATE, i+1, &egrSpeed, sizeof(egrSpeed));
                }

            }

        }
    }

    return rc;
}

char ssi_get_bandwidth_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    UI32_T i = 0;
    UI32_T total_len = 0;
    C8_T err = 0;
    UI16_T len = 0;

    UI32_T *ptr_rate_limit = NULL;
    C8_T *ptr_buf_inlimit = NULL;
    C8_T *ptr_buf_eglimit = NULL;
    UI32_T len_limit = 0;

    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;

    CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> entered\n", __FUNCTION__, __LINE__);

    /* get port ingress rate info */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_INGRESS_RATE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> get ingress rate data success, ptr_msg =%p\n", __FUNCTION__, __LINE__, db_msg);
    } else {
        CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> get ingress rate data failed \n", __FUNCTION__, __LINE__);
        return ERR_VAL;
    }

    rc = osapi_calloc(MAX_PORT_NUM * sizeof(UI32_T), HTTPD_QUEUE_CLI, (void **)&ptr_rate_limit);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(BANDWIDTH, "<%s:%d> alloc failed!", __FUNCTION__, __LINE__);
        return rc;
    }

    rc = osapi_calloc(BANDWIDTH_BUF_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_buf_inlimit);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(BANDWIDTH, "<%s:%d> alloc failed!", __FUNCTION__, __LINE__);
        MW_FREE(ptr_rate_limit);
        return rc;
    }

    rc = osapi_calloc(BANDWIDTH_BUF_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_buf_eglimit);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(BANDWIDTH, "<%s:%d> alloc failed!", __FUNCTION__, __LINE__);
        MW_FREE(ptr_rate_limit);
        MW_FREE(ptr_buf_inlimit);
        return rc;
    }

    memcpy(ptr_rate_limit, db_data, db_size);
    osapi_free(db_msg);

    for(i = 0; i < PLAT_MAX_PORT_NUM; i++){
        len_limit += snprintf(ptr_buf_inlimit + len_limit, BANDWIDTH_BUF_SIZE - len_limit, "%d,", ptr_rate_limit[i]);
    }

    /* get port egress rate info */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_EGRESS_RATE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> get egress rate data success, ptr_msg =%p\n", __FUNCTION__, __LINE__, db_msg);
    } else {
        CGI_LOG_ERROR(BANDWIDTH, "<%s:%d> get egress rate data failed!", __FUNCTION__, __LINE__);
        MW_FREE(ptr_rate_limit);
        MW_FREE(ptr_buf_inlimit);
        MW_FREE(ptr_buf_eglimit);
        return ERR_VAL;
    }

    memset(ptr_rate_limit, 0, MAX_PORT_NUM);
    memcpy(ptr_rate_limit, db_data, db_size);
    osapi_free(db_msg);

    len_limit = 0;
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++){
        len_limit += snprintf(ptr_buf_eglimit + len_limit, BANDWIDTH_BUF_SIZE - len_limit, "%d,", ptr_rate_limit[i]);
    }

    err = send_format_response(&len, pcb, apiflags,
            "<script>var portNumber=%d;var portspeed_info={ingress:[%s], egress:[%s]};\n</script>",
            PLAT_MAX_PORT_NUM, ptr_buf_inlimit, ptr_buf_eglimit);
    MW_FREE(ptr_rate_limit);
    MW_FREE(ptr_buf_inlimit);
    MW_FREE(ptr_buf_eglimit);

    if(err != ERR_OK)
        return err;

    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(BANDWIDTH, "<%s:%d> leave\n", __FUNCTION__, __LINE__);

    return ERR_OK;
}

