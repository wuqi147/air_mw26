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



/* FILE NAME:   spanningTreeInstance.c
 * PURPOSE:
 *      CGI and SSI function of spanning tree instance configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <osapi.h>

#include <mw_utils.h>
#include <db_api.h>
#include <httpd_queue.h>
#include <mw_log.h>
#include <web.h>
#ifdef AIR_SUPPORT_MSTP
#include "mstp_sys.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

#define MSTP_MODULE_NAME         "MSTP"
#define STP_MIN_REVISION         (0)
#define STP_MAX_REVISION         (65535)

#define MAX_MSTP_VLAN_LIST_SIZE  (165)
#define MAX_MSTP_VLAN_STR_LENGTH (165)

#if defined(AIR_SUPPORT_MSTP)

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
static UI32_T _instance_sel = 0;

/* Function to decode %22 to " */
UI8_T _mstp_decode_percent22(char *str) {
    char *ptr_src = str, *ptr_dst = str;

    if (ptr_src[0] != '%')
    {
        return MW_E_OTHERS;
    }
    while (*ptr_src)
    {
        if ((ptr_src[0] == '%') && (ptr_src[1] == '2') && (ptr_src[2] == '2'))
        {
            *ptr_dst++ = '"';
            ptr_src += 3;
        }
        else
        {
            *(ptr_dst++) = *(ptr_src++);
        }
    }
    *ptr_dst = '\0';

    return MW_E_OK;
}

MW_ERROR_NO_T
cgi_set_handle_regionInstance(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *pcParam[],
    C8_T *pcValue[])
{

    MW_ERROR_NO_T           rc = MW_E_OK;

    UI8_T                   i;
    UI8_T                   regionName[MAX_MSTP_REGION_NAME_SIZE] = "";
    UI16_T                  revision = 0;

    /* db variables */
    DB_MSG_T                *ptr_msg = NULL;
    UI16_T                  size = 0;
    DB_MSTP_REGION_CONFIG_T *ptr_mstp_region = NULL;

    /* get value from url params */
    CGI_LOG_DEBUG(STP, "<%s:%d> handling >>>>\n", __func__, __LINE__);

    for(i = 0; i < iNumParams; i++)
    {
        CGI_LOG_DEBUG(STP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, pcParam[i], pcValue[i]);

        if (0 == strcmp(pcParam[i], "region"))
        {
            snprintf((C8_T *)regionName, sizeof(regionName), "%s", pcValue[i]);
        }
        else if (0 == strcmp(pcParam[i], "revision"))
        {
            revision = atoi(pcValue[i]);
            if ((revision < STP_MIN_REVISION) || (revision > STP_MAX_REVISION))
            {
                CGI_LOG_WARN(STP, "invalid revision");
                return MW_E_OK;
            }
        }
        else
        {
            CGI_LOG_DEBUG(STP, "Not support this input para %s, value %s", pcParam[i], pcValue[i]);
            continue;
        }
    }

    rc = httpd_queue_getData(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_mstp_region);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
        CGI_LOG_ERROR(STP, "[MSTP] Get MSTP REGION failed\n");
        return rc;
    }

    ptr_mstp_region->revision = revision;
    memcpy(ptr_mstp_region->region_name, regionName, sizeof(regionName));

    rc = httpd_queue_setData(M_UPDATE, MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_mstp_region, sizeof(DB_MSTP_REGION_CONFIG_T));
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
       CGI_LOG_ERROR(STP, "[MSTP] Set MSTP REGION failed\n");
        return rc;
    }

    MW_FREE(ptr_msg);

    CGI_LOG_DEBUG(STP, "<%s:%d> <<<<< handled\n", __func__, __LINE__);

    return MW_E_OK;
}


MW_ERROR_NO_T
cgi_set_handle_spanningTreeInstance(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *pcParam[],
    C8_T *pcValue[])
{
    MW_ERROR_NO_T               rc = MW_E_OK;

    UI8_T                       i, j;

    C8_T                        *ptr_token;
    UI8_T                       ins_count = 0;

    UI16_T                      *ptr_vid;
    UI16_T                      data_size;
    UI32_T                      vlanBmp = 0, idx, num;

    /* db variable*/
    DB_MSTP_INSTANCE_TABLE_T    *ptr_ins_info_table = NULL;

    data_size = sizeof(UI16_T) * MAX_VLAN_ENTRY_NUM;
    osapi_calloc(data_size, HTTPD_QUEUE_CLI, (void **)&ptr_vid);
    if (NULL == ptr_vid)
    {
        CGI_LOG_ERROR(STP, "alloc vid failed.\n");
        return MW_E_NO_MEMORY;
    }

    rc = osapi_calloc(sizeof(DB_MSTP_INSTANCE_TABLE_T), HTTPD_QUEUE_CLI, (void **)&ptr_ins_info_table);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_vid);
        return MW_E_NO_MEMORY;
    }

    /* get value from url params */
    CGI_LOG_DEBUG(STP, "<%s:%d> handling >>>>\n", __func__, __LINE__);

    for(i = 0; i < iNumParams; i++)
    {
        CGI_LOG_DEBUG(STP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, pcParam[i], pcValue[i]);

        if (0 == strcmp(pcParam[i], "ins"))
        {
            j = 0;
            ptr_token = strtok(pcValue[i], ",");
            ins_count = 0;
            while (ptr_token != NULL)
            {
                num = atoi(ptr_token);
                ptr_ins_info_table->instance[j++] = num;
                ptr_token = strtok(NULL, ",");
                ins_count++;
            }
        }
        else if (0 == strcmp(pcParam[i], "pri"))
        {
            j = 0;
            ptr_token = strtok(pcValue[i], ",");
            while (ptr_token != NULL)
            {
                num = atoi(ptr_token);
                ptr_ins_info_table->priority[j++] = num;
                ptr_token = strtok(NULL, ",");
            }
        }
        else if (0 == strcmp(pcParam[i], "vlan_id"))
        {
            char *ptr_start = pcValue[i];
            char *ptr_end;
            int count = 0;
            UI8_T tmp_vlan_str[MAX_MSTP_VLAN_LIST_SIZE] = {0};

            CGI_LOG_DEBUG(STP, "before decode, input = %s\n", pcValue[i]);

            /* Step 1: Decode %22 to " */
            rc = _mstp_decode_percent22(pcValue[i]);
            if (rc != MW_E_OK)
            {
                CGI_LOG_ERROR(STP, "invalid input = %s\n", pcValue[i]);
                break;
            }

            CGI_LOG_DEBUG(STP, "after decode, input = %s\n", pcValue[i]);

            /* Step 2: Extract strings between quotes */
            ptr_start = pcValue[i];

            while ((ptr_start = strchr(ptr_start, '"')) != NULL &&
                (count < ins_count))
            {
                /* Move past the opening quote */
                ptr_start++;
                ptr_end = strchr(ptr_start, '"');
                if (NULL != ptr_end)
                {
                    size_t len = ptr_end - ptr_start;
                    osapi_strncpy((C8_T *)tmp_vlan_str, ptr_start, len);
                    /* Null-terminate */
                    tmp_vlan_str[len] = '\0';
                    vlanBmp = 0;
                    mstp_get_vlanArray(tmp_vlan_str, ptr_vid, &num);
                    for (i = 0; i < num; i++)
                    {
                        if (0 == ptr_vid[i])
                        {
                            break;
                        }
                        mstp_getVlanIndex(ptr_vid[i], &idx);
                        BIT_SET(vlanBmp, idx);
                    }
                    ptr_ins_info_table->vlanbmp[count++] = vlanBmp;
                    /* Move past the closing quote */
                    ptr_start = ptr_end + 1;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            CGI_LOG_DEBUG(STP, "[MSTP] Not support this input para %s, value %s\n", pcParam[i], pcValue[i]);
            continue;
        }
    }

    rc = httpd_queue_setData(M_UPDATE, MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_ins_info_table, sizeof(DB_MSTP_INSTANCE_TABLE_T));
    MW_FREE(ptr_vid);
    MW_FREE(ptr_ins_info_table);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(STP, "[MSTP] Set MSTP INSTANCE failed\n");
        return rc;
    }

    CGI_LOG_DEBUG(STP, "<%s:%d> <<<<< handled\n", __func__, __LINE__);

    return MW_E_OK;
}

MW_ERROR_NO_T
cgi_set_handle_spanningTreePortInstance(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *pcParam[],
    C8_T *pcValue[])
{
    MW_ERROR_NO_T                   rc = MW_E_OK;

    UI8_T                           i;
    UI8_T                           instance = 0;
    UI32_T                          pbmp = 0;
    UI8_T                           priority = 0;
    UI32_T                          cost = 0;
    UI8_T                           pri_cost_updated = 0;

    /* db variable*/
    DB_MSG_T                        *ptr_msg = NULL;
    UI16_T                          size = 0;
    void                            *ptr_db_data = NULL;
    DB_MSTP_INSTANCE_PORT_INFO_T    port_info_list;
    DB_STP_PORT_INFO_T              port_info;

    osapi_memset(&port_info_list, 0, sizeof(DB_MSTP_INSTANCE_PORT_INFO_T));
    osapi_memset(&port_info, 0, sizeof(DB_STP_PORT_INFO_T));

    /* get value from url params */
    CGI_LOG_DEBUG(STP, "<%s:%d> handling >>>>\n", __func__, __LINE__);

    for(i = 0; i < iNumParams; i++)
    {
        CGI_LOG_DEBUG(STP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, pcParam[i], pcValue[i]);

        if (0 == strcmp(pcParam[i], "ins"))
        {
            instance = atoi(pcValue[i]);
            _instance_sel = instance;
        }
        else if (0 == strcmp(pcParam[i], "pbmp"))
        {
            pbmp = atoi(pcValue[i]);
        }
        else if (0 == strcmp(pcParam[i], "portPriority"))
        {
            priority = atoi(pcValue[i]);
            pri_cost_updated = 1;
        }
        else if (0 == strcmp(pcParam[i], "cost"))
        {
            cost = atoi(pcValue[i]);
            pri_cost_updated = 1;
        }
        else
        {
            CGI_LOG_DEBUG(STP, "[MSTP] Not support this input para %s, value %s\n", pcParam[i], pcValue[i]);
            continue;
        }
    }

    if (0 == pri_cost_updated)
    {
        CGI_LOG_DEBUG(STP, "[MSTP] Only set selected instance %d, don't update DB \n", _instance_sel);
        return rc;
    }

    for (i = 0 ; i < PLAT_MAX_PORT_NUM ; i++)
    {
        if ((pbmp >> i & 1) == 1)
        {
            rc = httpd_queue_getData(MSTP_INSTANCE_PORT, DB_ALL_FIELDS, i+1, &ptr_msg, &size, &ptr_db_data);

            osapi_memcpy(&port_info_list, ptr_db_data, size);
            MW_FREE(ptr_msg);
            if (MW_E_OK != rc)
            {
                CGI_LOG_ERROR(STP, "Get MSTP_INSTANCE_PORT failed\n");
                return rc;
            }

            port_info_list.priority[instance] = priority;
            port_info_list.cost[instance] = cost;

            rc = httpd_queue_setData(M_UPDATE, MSTP_INSTANCE_PORT, DB_ALL_FIELDS, i+1, &port_info_list, sizeof(DB_MSTP_INSTANCE_PORT_INFO_T));
            if(MW_E_OK != rc)
            {
                CGI_LOG_ERROR(STP, "Set MSTP INSTANCE PORT failed\n");
                return rc;
            }
            if( 0 == instance)
            {
                /* also update RSTP_PORT_INFO priority & cost here */
                rc = httpd_queue_getData(STP_PORT_INFO, DB_ALL_FIELDS, i+1, &ptr_msg, &size, (void **)&ptr_db_data);

                osapi_memcpy(&port_info, ptr_db_data, size);
                MW_FREE(ptr_msg);
                if (rc != MW_E_OK)
                {
                    CGI_LOG_ERROR(STP, "Get RSTP_PORT_INFO failed\n");
                    return ERR_INPROGRESS;
                }

                port_info.priority = priority;
                port_info.cost = cost;

                rc = httpd_queue_setData(M_UPDATE, STP_PORT_INFO, DB_ALL_FIELDS, i+1, &port_info, sizeof(DB_STP_PORT_INFO_T));
                if(MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(STP, "Set RSTP_PORT_INFO failed\n");
                    return rc;
                }
            }
        }
    }

    CGI_LOG_DEBUG(STP, "<%s:%d> <<<<< handled\n", __func__, __LINE__);

    return MW_E_OK;
}

char
ssi_get_spanning_tree_region_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int                         rc = MW_E_OK;
    unsigned int                total_len = 0;

    /* db variable*/
    DB_MSG_T                    *ptr_msg = NULL;
    UI16_T                      size = 0;
    void                        *ptr_db_data = NULL;
    DB_MSTP_REGION_CONFIG_T     mstp_region;

    C8_T                        *ptr_ssi_str = NULL;
    UI32_T                      tag_insert_max_len = 0;
    UI16_T                      tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = httpd_queue_getData(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, &ptr_db_data);
    if(MW_E_OK == rc)
    {
        memcpy(&mstp_region, ptr_db_data, size);
        MW_FREE(ptr_msg);
    }
    else
    {
        CGI_LOG_ERROR(STP, "Get MSTP REGION failed\n");
        MW_FREE(ptr_msg);
        return rc;
    }

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>var revision = %d;", mstp_region.revision);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var region_name = \"%s\";", mstp_region.region_name);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var max_mstp_ins_num = %d;", MAX_MSTP_INSTANCE_NUM);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "</script>");
    total_len = tmplen;

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

char
ssi_get_spanning_tree_instance_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int                         rc = MW_E_OK, i, j, k;
    unsigned int                total_len = 0;

    UI16_T                      *ptr_vid;
    UI16_T                      data_size;
    UI32_T                      num;
    UI8_T                       *ptr_str = NULL;
    UI32_T                      pbmp = 0;
    UI32_T                      vlan_index;

    /* db variable*/
    DB_MSG_T                    *ptr_msg = NULL;
    UI16_T                      size = 0;
    void                        *ptr_db_data = NULL;
    DB_MSTP_INSTANCE_TABLE_T    *ptr_ins_info_table = NULL;
    DB_VLAN_ENTRY_T             *ptr_vlan_entry = NULL;

    C8_T                        *ptr_ssi_str = NULL;
    UI32_T                      tag_insert_max_len = 0;
    UI16_T                      tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    data_size = sizeof(UI16_T) * MAX_VLAN_ENTRY_NUM;
    osapi_calloc(data_size, HTTPD_QUEUE_CLI, (void **)&ptr_vid);
    if (NULL == ptr_vid)
    {
        CGI_LOG_ERROR(STP, "alloc vid failed.\n");
        return MW_E_NO_MEMORY;
    }

    rc = osapi_calloc(MAX_MSTP_VLAN_STR_LENGTH, HTTPD_QUEUE_CLI, (void **)&ptr_str);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_vid);
        return MW_E_NO_MEMORY;
    }

    rc = osapi_calloc(sizeof(DB_MSTP_INSTANCE_TABLE_T), HTTPD_QUEUE_CLI, (void **)&ptr_ins_info_table);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_vid);
        MW_FREE(ptr_str);
        return MW_E_NO_MEMORY;
    }

    rc = httpd_queue_getData(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, &ptr_db_data);

    osapi_memcpy(ptr_ins_info_table, ptr_db_data, size);
    MW_FREE(ptr_msg);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(STP, "Fail to get MSTP_INSTANCE DB");
        MW_FREE(ptr_vid);
        MW_FREE(ptr_str);
        MW_FREE(ptr_ins_info_table);
        return rc;
    }

    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_vid);
        MW_FREE(ptr_str);
        MW_FREE(ptr_ins_info_table);
        return rc;
    }

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>var ins_cfg=[");

    for (k = 0 ; k < MAX_VLAN_ENTRY_NUM; k++)
    {
        mstp_getVlanIndex(ptr_vlan_entry->vlan_id[k], &vlan_index);
        ptr_ins_info_table->vlanbmp[0] |= BIT(vlan_index);
    }

    for (i = 1; i < MSTP_INSTANCE_MAX_NUM ; i++)
    {
        if((i > 0) && (0 == ptr_ins_info_table->instance[i]))
        {
            break;
        }
        mstp_getMstVlanArrayByBmp(ptr_ins_info_table->vlanbmp[i], ptr_vid, &num);

        for (j = 0 ; j< num ; j++)
        {
            mstp_getVlanIndex(ptr_vid[j], &vlan_index);
            ptr_ins_info_table->vlanbmp[0] &= ~(BIT(vlan_index));
        }
    }

    for (i = 0; i < MSTP_INSTANCE_MAX_NUM ; i++)
    {
        if((i > 0) && (0 == ptr_ins_info_table->instance[i]))
        {
            break;
        }
        mstp_getMstVlanArrayByBmp(ptr_ins_info_table->vlanbmp[i], ptr_vid, &num);
        pbmp = 0;
        CGI_LOG_DEBUG(STP, "    vlan dump : ");
        for (j = 0 ; j< num ; j++)
        {
            CGI_LOG_DEBUG(STP, "%d,", ptr_vid[j]);

            for (k = 0 ; k < MAX_VLAN_ENTRY_NUM; k++)
            {
                if (ptr_vid[j] == ptr_vlan_entry->vlan_id[k])
                {
                    pbmp |= (ptr_vlan_entry->tagged_member[k] | ptr_vlan_entry->untagged_member[k] | ptr_vlan_entry->port_member[k]);
                }
            }
        }
        CGI_LOG_DEBUG(STP, "\n");
        if (0 != ptr_ins_info_table->vlanbmp[i])
        {
            mstp_vlanArray_to_str(num, ptr_vid, ptr_str);
        }
        else
        {
            snprintf((char *)ptr_str, 2, "-");
        }
        CGI_LOG_DEBUG(STP, "    str = %s\n", ptr_str);

        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "{ins:%d,priority:%d,pbmp:%d,vlan:\"%s\"},\n", ptr_ins_info_table->instance[i], ptr_ins_info_table->priority[i], pbmp, ptr_str);
    }
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "];</script>");
    total_len = tmplen;

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    MW_FREE(ptr_msg);
    MW_FREE(ptr_vid);
    MW_FREE(ptr_str);
    MW_FREE(ptr_ins_info_table);

    return ERR_OK;
}

char
ssi_get_spanning_tree_instance_port_state_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int                             rc = MW_E_OK, i, j;
    unsigned int                    total_len = 0, len_1 = 0;

    /* db variable*/
    DB_MSG_T                        *ptr_msg = NULL;
    UI16_T                          size = 0;
    void                            *ptr_db_data = NULL;
    DB_MSTP_INSTANCE_PORT_OPER_T    *ptr_port_oper_list;
    UI16_T                          data_size = 0;

    char                            *ptr_buf_state_all_p = NULL;

    C8_T                            *ptr_ssi_str = NULL;
    UI32_T                          tag_insert_max_len = 0;
    UI16_T                          tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = osapi_calloc(PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8 , HTTPD_QUEUE_CLI, (void **)&ptr_buf_state_all_p);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    data_size = sizeof(DB_MSTP_INSTANCE_PORT_OPER_T) * PLAT_MAX_PORT_NUM;
    osapi_calloc(data_size, HTTPD_QUEUE_CLI, (void **)&ptr_port_oper_list);
    if (NULL == ptr_port_oper_list)
    {
        CGI_LOG_ERROR(STP, "alloc failed.\n");
        MW_FREE(ptr_buf_state_all_p);
        return MW_E_NO_MEMORY;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        rc = httpd_queue_getData(MSTP_PORT_OPER_INFO, DB_ALL_FIELDS, i+1, &ptr_msg, &size, &ptr_db_data);

        osapi_memcpy(&ptr_port_oper_list[i], ptr_db_data, sizeof(DB_MSTP_INSTANCE_PORT_OPER_T));
        MW_FREE(ptr_msg);
        if (MW_E_OK != rc)
        {
            CGI_LOG_ERROR(STP, "Get MSTP_PORT_OPER_INFO failed");
            MW_FREE(ptr_buf_state_all_p);
            MW_FREE(ptr_port_oper_list);
            return rc;
        }
    }

    for (i = 0; i < MSTP_INSTANCE_MAX_NUM; i++)
    {
        len_1 += snprintf(ptr_buf_state_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%s", "[");
        for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
        {
            len_1 += snprintf(ptr_buf_state_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%d,", ptr_port_oper_list[j].state[i]);
        }
        len_1 += snprintf(ptr_buf_state_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%s", "],");
    }

    ptr_buf_state_all_p[len_1 - 1] = '\0';

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>port_state = [");
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", ptr_buf_state_all_p);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "];</script>");
    total_len = tmplen;

    MW_FREE(ptr_buf_state_all_p);
    MW_FREE(ptr_port_oper_list);

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return ERR_OK;
}

char
ssi_get_spanning_tree_instance_port_role_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int                             rc = MW_E_OK, i, j;
    unsigned int                    total_len = 0, len_1 = 0;

    /* db variable*/
    DB_MSG_T                        *ptr_msg = NULL;
    UI16_T                          size = 0;
    void                            *ptr_db_data = NULL;
    DB_MSTP_INSTANCE_PORT_OPER_T    *ptr_port_oper_list;
    UI16_T                          data_size = 0;
    char                            *ptr_buf_role_all_p = NULL;

    C8_T                            *ptr_ssi_str = NULL;
    UI32_T                          tag_insert_max_len = 0;
    UI16_T                          tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = osapi_calloc(PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8 , HTTPD_QUEUE_CLI, (void **)&ptr_buf_role_all_p);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    data_size = sizeof(DB_MSTP_INSTANCE_PORT_OPER_T) * PLAT_MAX_PORT_NUM;
    osapi_calloc(data_size, HTTPD_QUEUE_CLI, (void **)&ptr_port_oper_list);
    if (NULL == ptr_port_oper_list)
    {
        CGI_LOG_ERROR(STP, "alloc failed.\n");
        MW_FREE(ptr_buf_role_all_p);
        return MW_E_NO_MEMORY;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        rc = httpd_queue_getData(MSTP_PORT_OPER_INFO, DB_ALL_FIELDS, i+1, &ptr_msg, &size, &ptr_db_data);

        osapi_memcpy(&ptr_port_oper_list[i], ptr_db_data, size);
        MW_FREE(ptr_msg);
        if (MW_E_OK != rc)
        {
            CGI_LOG_ERROR(STP, "Get MSTP_PORT_OPER_INFO failed");
            MW_FREE(ptr_buf_role_all_p);
            MW_FREE(ptr_port_oper_list);
            return rc;
        }
    }

    for (i = 0; i < MSTP_INSTANCE_MAX_NUM; i++)
    {
        len_1 += snprintf(ptr_buf_role_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%s", "[");
        for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
        {
            len_1 += snprintf(ptr_buf_role_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%d,", ptr_port_oper_list[j].role[i]);
        }
        len_1 += snprintf(ptr_buf_role_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%s", "],");
    }

    ptr_buf_role_all_p[len_1 - 1] = '\0';

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>port_role = [");
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", ptr_buf_role_all_p);
    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "];</script>");
    total_len = tmplen;

    MW_FREE(ptr_buf_role_all_p);
    MW_FREE(ptr_port_oper_list);

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return ERR_OK;
}

char
ssi_get_spanning_tree_instance_port_cost_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int                             rc = MW_E_OK, i, j;
    unsigned int                    total_len = 0, len_1 = 0;

    /* db variable*/
    DB_MSG_T                        *ptr_msg = NULL;
    UI16_T                          size = 0;
    void                            *ptr_db_data = NULL;
    DB_MSTP_INSTANCE_PORT_INFO_T    *ptr_port_info_list;
    UI16_T                          data_size;

    char                            *ptr_buf_cost_all_p = NULL;

    C8_T                            *ptr_ssi_str = NULL;
    UI32_T                          tag_insert_max_len = 0;
    UI16_T                          tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = osapi_calloc(PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8 , HTTPD_QUEUE_CLI, (void **)&ptr_buf_cost_all_p);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    data_size = sizeof(DB_MSTP_INSTANCE_PORT_INFO_T) * PLAT_MAX_PORT_NUM;
    osapi_calloc(data_size, HTTPD_QUEUE_CLI, (void **)&ptr_port_info_list);
    if (NULL == ptr_port_info_list)
    {
        CGI_LOG_ERROR(STP, "alloc failed.\n");
        MW_FREE(ptr_buf_cost_all_p);
        return MW_E_NO_MEMORY;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        rc = httpd_queue_getData(MSTP_INSTANCE_PORT, DB_ALL_FIELDS, i+1, &ptr_msg, &size, &ptr_db_data);

        osapi_memcpy(&ptr_port_info_list[i], ptr_db_data, size);
        MW_FREE(ptr_msg);
        if (MW_E_OK != rc)
        {
            CGI_LOG_ERROR(STP, "Get MSTP_INSTANCE_PORT failed");
            MW_FREE(ptr_buf_cost_all_p);
            MW_FREE(ptr_port_info_list);
            return rc;
        }
    }

    len_1 += snprintf(ptr_buf_cost_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%d;", _instance_sel);
    for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
    {
        len_1 += snprintf(ptr_buf_cost_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%d,", ptr_port_info_list[j].cost[_instance_sel]);
    }
    len_1 += snprintf(ptr_buf_cost_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, ";");
    for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
    {
        len_1 += snprintf(ptr_buf_cost_all_p + len_1, (PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM * 8) - len_1, "%d,", ptr_port_info_list[j].priority[_instance_sel]);
    }

    ptr_buf_cost_all_p[len_1 - 1] = '\0';

    tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", ptr_buf_cost_all_p);
    total_len = tmplen;

    MW_FREE(ptr_buf_cost_all_p);
    MW_FREE(ptr_port_info_list);
#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return ERR_OK;

}
#endif
