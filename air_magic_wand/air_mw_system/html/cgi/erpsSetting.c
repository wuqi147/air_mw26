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

/* FILE NAME:   erpsSetting.c
 * PURPOSE:
 *      CGI and SSI function of ERPS setting web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "mw_error.h"
#include "db_api.h"
#include "default_config.h"
#include "httpd_queue.h"
#include "web.h"
#include "vlan_utils.h"
#include "erps_config_customer.h"
#include "erps.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


#define ERPS_MAX_VLAN_SIZE            (128)
#define ERPS_MAX_PORT_SIZE            (128)
#define ERPS_MAX_BUF_SIZE             (256)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  SSI handler
//==========================================================================================
MW_ERROR_NO_T
ssi_get_erps_state_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    UI8_T i = 0, j = 0;
    UI16_T total_len = 0;
    C8_T *ptr_ssi_str = NULL;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_ERPS_INFO_T *ptr_erps_entry = NULL;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = httpd_queue_getData(ERPS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_erps_entry);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    for(i = 0; i < MAX_ERPS_INSTANCE_NUM; i++)
    {
        CGI_LOG_DEBUG(ERPS, "%s[%d] {idx:%u,west_port_state:%u,east_port_state:%u,instance_state:%u,}", __func__, __LINE__,
            i+1,
            ptr_erps_entry->west_port_state[i],
            ptr_erps_entry->east_port_state[i],
            ptr_erps_entry->instance_state[i]);

        j += sprintf(ptr_ssi_str + j, "%u,%u,%u",
            ptr_erps_entry->west_port_state[i],
            ptr_erps_entry->east_port_state[i],
            ptr_erps_entry->instance_state[i]);

        total_len += j;
    }

    MW_FREE(ptr_msg);

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_erps_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    C8_T *ptr_erps_port = NULL, *ptr_ssi_str = NULL;
    UI16_T i = 0, j = 0;
    UI16_T total_len = 0;
    UI16_T tmplen = 0;
    UI8_T vlan_state = VLAN_NONE;

    const UI8_T *erps_port_settings_array = erps_config_getPortSettingsArray();
    UI32_T erps_port_settings_array_size = erps_config_getPortSettingsArraySize(), tag_insert_max_len = 0;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;
    DB_ERPS_INFO_T *ptr_erps_entry = NULL;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = osapi_calloc(ERPS_MAX_PORT_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_erps_port);
    if (MW_E_OK != rc)
    {
        return MW_E_NO_MEMORY;
    }

    /* Get DB VLAN_CFG_INFO */
    rc = httpd_queue_getData(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_cfg);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_erps_port);
        return rc;
    }

    if (TRUE == ptr_vlan_cfg->enable_port_b)
    {
        vlan_state = VLAN_PORT_ENABLE;
    }
    else if (TRUE == ptr_vlan_cfg->enable_8021q_b)
    {
        vlan_state = VLAN_1Q_ENABLE;
    }
    else if (TRUE == ptr_vlan_cfg->enable_mtu)
    {
        vlan_state = VLAN_MTU_ENABLE;
    }

    MW_FREE(ptr_msg);

    for(i = 0; i < erps_port_settings_array_size; i++)
    {
        if(0 != erps_port_settings_array[i])
        {
            j += sprintf(ptr_erps_port + j, "%u,", erps_port_settings_array[i]);
            total_len += j;
        }
    }

    MW_FREE(ptr_msg);

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>var currentVlanState = %d;", vlan_state);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var erpsPort = [%s];", ptr_erps_port);

    /* GET DB ERPS ENTRY */
    rc = httpd_queue_getData(ERPS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_erps_entry);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_erps_port);
        return rc;
    }

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var erpsInfo = [");
    for(i = 0; i < MAX_ERPS_INSTANCE_NUM; i++)
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "[%d,", ptr_erps_entry->ring_id[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->control_vlan[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "\"%s\",", ptr_erps_entry->data_vlan[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->revertive_mode[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->west_port[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->east_port[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->wtr_timer[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", (ptr_erps_entry->guard_timer[i]) * 10 );
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", (ptr_erps_entry->hold_off_timer[i]) * 100);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->west_port_state[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", ptr_erps_entry->east_port_state[i]);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d],", ptr_erps_entry->instance_state[i]);
    }
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "];</script>");
    total_len = tmplen;

    MW_FREE(ptr_msg);
    MW_FREE(ptr_erps_port);

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_erpsConfig(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *pcParam[],
    C8_T *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;

    UI8_T i = 0, is_edit = 0, ringId = 0, revertiveMode = 0, westPort = 0, eastPort = 0;
    UI8_T wtrTimer = 0, guardTimer = 0, holdOffTimer = 0;
    UI16_T ctrlVlan = 0, instanceId = 0;
    C8_T  dataVlan[MAX_ERPS_DATA_VLAN_SIZE] = "";
    C8_T action[4] = {0};
    UI32_T vidxBmpForFid = 0;

    ERPS_ENTRY_INFO_T erpsInfo;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;
    ERPS_ENTRY_INFO_T *ptr_erps_entry = NULL;


    for(i = 0; i < iNumParams; i++)
    {
        if (!strcmp(pcParam[i], "instanceId"))
        {
            instanceId = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "action"))
        {
            snprintf(action, sizeof(action), "%s", pcValue[i]);
            if(strcmp(action, "add") == 0)
            {
                is_edit = 0;
            }
            else if(strcmp(action, "mod") == 0)
            {
                is_edit = 1;
            }
        }
        else if (!strcmp(pcParam[i], "ringId"))
        {
            ringId = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "ctrlVlan"))
        {
            ctrlVlan = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "dataVlan"))
        {
            osapi_snprintf(dataVlan, sizeof(dataVlan), "%s", pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "revMode"))
        {
            revertiveMode = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "westPort"))
        {
            westPort = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "eastPort"))
        {
            eastPort = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "wtrTimer"))
        {
            wtrTimer = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "guardTimer"))
        {
            guardTimer = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "holdOffTimer"))
        {
            holdOffTimer = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "fidx"))
        {
            vidxBmpForFid = atoi(pcValue[i]);
        }
        else
        {
            continue;
        }
    }

    CGI_LOG_DEBUG(ERPS, "instanceId: %d, ringId: %d, ctrlVlan:%d, dataVlan:%s, revMode:%d, westPort:%d, eastPort:%d, wtrTimer:%d, guardTimer:%d, holdOffTimer:%d, vidxBmpForFid:0x%x",
                        instanceId, ringId, ctrlVlan, dataVlan, revertiveMode, westPort, eastPort, wtrTimer, guardTimer, holdOffTimer, vidxBmpForFid);

    osapi_memset(&erpsInfo, 0, sizeof(erpsInfo));
    if ((0 != instanceId) && (0 != ringId) && (0 == ctrlVlan))
    {
        UI8_T fid[32] = {0};

        CGI_LOG_DEBUG(ERPS, "Delete ring id: %d", ringId);
        /* delete table */
        rc = httpd_queue_setData(M_DELETE, ERPS_INFO, DB_ALL_FIELDS, instanceId, NULL, 0);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Delete entry failed");
        }

        cgi_set_handle_qVlanSet(iIndex, iNumParams, pcParam, pcValue);

        rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Get data vlan fid failed");
        }

        memcpy(ptr_vlan_entry_tbl->vlan_fid, fid, sizeof(fid));

        rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Update data vlan fid failed");
        }

        MW_FREE(ptr_msg);
        return rc;
    }

    if (1 == is_edit)
    {
        rc = httpd_queue_getData(ERPS_INFO, DB_ALL_FIELDS, instanceId, &ptr_msg, &size, (void **)&ptr_erps_entry);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Get ERPS current info failed");
        }

        erpsInfo.ring_id = ringId;
        erpsInfo.control_vlan = ctrlVlan;
        strncpy((C8_T *) erpsInfo.data_vlan, dataVlan, MAX_ERPS_DATA_VLAN_SIZE);
        erpsInfo.data_vlan[MAX_ERPS_DATA_VLAN_SIZE - 1] = 0; /* ensure NULL termination (strncpy does NOT guarantee it!) */
        erpsInfo.revertive_mode = revertiveMode;
        erpsInfo.wtr_timer = wtrTimer;
        erpsInfo.guard_timer = guardTimer;
        erpsInfo.hold_off_timer = holdOffTimer;

        if (ptr_erps_entry->west_port != westPort)
        {
            erpsInfo.west_port = westPort;
            erpsInfo.west_port_state = 0;
        }
        else
        {
            erpsInfo.west_port = ptr_erps_entry->west_port;
            erpsInfo.west_port_state = ptr_erps_entry->west_port_state;
        }

        if (ptr_erps_entry->east_port != eastPort)
        {
            erpsInfo.east_port = eastPort;
            erpsInfo.east_port_state = 0;
        }
        else
        {
            erpsInfo.east_port = ptr_erps_entry->east_port;
            erpsInfo.east_port_state = ptr_erps_entry->east_port_state;
        }

        erpsInfo.instance_state = ptr_erps_entry->instance_state;

        rc = httpd_queue_setData(M_UPDATE, ERPS_INFO, DB_ALL_FIELDS, instanceId, &erpsInfo, sizeof(erpsInfo));
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Set ERPS info failed");
        }

        MW_FREE(ptr_msg);
    }
    else
    {
        erpsInfo.ring_id = ringId;
        erpsInfo.control_vlan = ctrlVlan;
        strncpy((C8_T *) erpsInfo.data_vlan, dataVlan, MAX_ERPS_DATA_VLAN_SIZE);
        erpsInfo.data_vlan[MAX_ERPS_DATA_VLAN_SIZE - 1] = 0; /* ensure NULL termination (strncpy does NOT guarantee it!) */
        erpsInfo.revertive_mode = revertiveMode;
        erpsInfo.wtr_timer = wtrTimer;
        erpsInfo.guard_timer = guardTimer;
        erpsInfo.hold_off_timer = holdOffTimer;
        erpsInfo.west_port = westPort;
        erpsInfo.east_port = eastPort;
        erpsInfo.east_port_state = 0;
        erpsInfo.west_port_state = 0;
        erpsInfo.instance_state = 0;

        rc = httpd_queue_setData(M_CREATE, ERPS_INFO, DB_ALL_FIELDS, instanceId, &erpsInfo, sizeof(erpsInfo));
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Set ERPS info failed");
        }
    }

    /* Add control VLAN */
    cgi_set_handle_qVlanSet(iIndex, iNumParams, pcParam, pcValue);

    /* Update FID */
    CGI_LOG_DEBUG(ERPS, "update FID bmp = %d, fid = %d", vidxBmpForFid, instanceId);

    if (vidxBmpForFid != 0)
    {
        UI8_T fid[32] = {0};
        for (i = 0 ; i < 32; i++)
        {
            if (0 != (( BIT(i)) & vidxBmpForFid))
            {
                fid[i] = instanceId;
                CGI_LOG_DEBUG(ERPS, "fid[%d] = %d", i, fid[i]);
            }
        }

        rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Get data vlan fid failed");
        }

        memcpy(ptr_vlan_entry_tbl->vlan_fid, fid, sizeof(fid));

        rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(ERPS, "Update data vlan fid failed");
        }

        MW_FREE(ptr_msg);
    }

    return rc;
}
