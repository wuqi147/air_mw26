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
#include <db_api.h>
#include <osapi_memory.h>
#include <httpd_queue.h>
#include <air_chipscu.h>
#ifdef AIR_SUPPORT_MQTTD
#include "mqttd.h"
#endif

MW_ERROR_NO_T
ssi_set_save_setting_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    char err = ERR_OK;
    DB_SYSTEM_T sys_cfg;
    C8_T tmpbuf[3] = {0};
    UI32_T len = 0;

    memset(&sys_cfg, 0, sizeof(sys_cfg));
    sys_cfg.save_running = TRUE;
    rc = httpd_queue_setData(M_UPDATE, SYSTEM, DB_ALL_FIELDS, DB_ALL_ENTRIES, &sys_cfg, sizeof(sys_cfg));
    if(MW_E_OK == rc)
    {
        len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "1");
    }
    else
    {
        len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "0");
    }
    err = send_format_response((UI16_T*)&len, ptr_pcb, apiflags, tmpbuf);
    if(ERR_OK != err)
    {
        rc = MW_E_OP_INCOMPLETE;
    }
    *ptr_length = len;

    return rc;
}
MW_ERROR_NO_T cgi_set_handle_system_reboot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    BOOL_T reboot = FALSE;
    DB_SYSTEM_T sys_cfg;

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if ((!strcmp(pcParam[i], "reboot_op")) &&
            (!strcmp(pcValue[i], "reboot")))
        {
            reboot = TRUE;
        }
    }

    /* parser params to db format */
    if (TRUE == reboot)
    {
        memset(&sys_cfg, 0, sizeof(sys_cfg));
        sys_cfg.reset = reboot;
        rc = httpd_queue_setData(M_UPDATE, SYSTEM, DB_ALL_FIELDS, DB_ALL_ENTRIES, &sys_cfg, sizeof(sys_cfg));
#ifdef AIR_SUPPORT_MQTTD
        if ((MW_E_OK == rc) && (TRUE == mqttd_get_state()))
        {
            UI8_T enable = FALSE;
            rc = httpd_queue_setData(M_UPDATE, MQTTD_CFG_INFO, MQTTD_CFG_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(enable));
        }
#endif
        air_chipscu_resetSystem(0);
    }

    return rc;
}

#ifdef AIR_SUPPORT_MQTTD
MW_ERROR_NO_T
cgi_set_handle_save_running(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_SYSTEM_T sys_cfg;

    memset(&sys_cfg, 0, sizeof(sys_cfg));
    sys_cfg.save_running = TRUE;

    /* Update db */
    rc = httpd_queue_setData(M_UPDATE, SYSTEM, DB_ALL_FIELDS, DB_ALL_ENTRIES, &sys_cfg, sizeof(sys_cfg));
    if (MW_E_OK != rc)
    {
        CGI_LOG_WARN(MQTTD, "Update DB sys_cfg save_running failed(%d)", rc);
    }
    return rc;
}
#endif
