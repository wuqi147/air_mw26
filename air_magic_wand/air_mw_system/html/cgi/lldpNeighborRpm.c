/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
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
/* FILE NAME:   LldpNeighborRpm.c
 * PURPOSE:
 *      CGI and SSI function of lldp neighbor web page.
 * NOTES:
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "mw_error.h"
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "osapi_string.h"
#include "httpd_queue.h"

/* MACRO FUNCTION DECLARATIONS
 */
#define MED_CAPABILITIES_TYPE           (6)
#define MED_CAPABILITIES_LEN            (18)
#define CHASSIS_ID_SUBTYPE_MAC          (4)
#define CHASSIS_ID_SUBTYPE_NETWORK      (5)
#define PORT_ID_SUBTYPE_MAC             (3)
#define PORT_ID_SUBTYPE_NETWORK         (4)
#define NEIGHBOR_INFO_TMP_BUF_SIZE      (1024)
#define LLDP_READ_NUM                   (2)

/* Helper macro to perform safe osapi_snprintf and update cur_len */
#define LLDP_APPEND_SNPRINTF(fmt, ...) \
    osapi_xsnprintf(buf, buf_length, &cur_len, fmt, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
UI16_T _lldp_entry_num = 0;
extern UI8_T lldp_global_cfg;
extern UI8_T lldp_neibor_index[MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT];
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROMGRAM BODIES
*/
/* EXPORTED SUBPROGRAM BODIES
 */
void analysisLldpClientInfo(UI8_T port, UI16_T index, DB_LLDP_CLIENT_INFO_T * info, UI16_T * len, C8_T * buf)
{
    I32_T ret = 0;
    I8_T i = 0;
    UI8_T tmp_len = 0, tmp_count = 0;
    UI16_T tmp_value = 0;
    UI16_T tmp_med = 0;
    UI32_T tmp_net = 0;
    C8_T port_name[16] = {0};
    UI8_T * ptr = NULL;
    UI16_T buf_length = *len;
    size_t cur_len = 0;
    const C8_T param[MED_CAPABILITIES_TYPE][MED_CAPABILITIES_LEN] =
    {
        "medCapabilities",
        "networkPolicy",
        "locationIdentify",
        "extendPowVPSE",
        "extendPowVPD",
        "inventory"
    };
    LLDP_APPEND_SNPRINTF("%d&", index);
    /* local port */
    ret = osapi_snprintf(port_name, sizeof(port_name), "port %d", port);
    if(ret < 0)
    {
        CGI_LOG_DEBUG(LLDP, "sprintf error\n");
    }
    LLDP_APPEND_SNPRINTF("%s&", port_name);
    /* chassis id */
    for(i = (MAX_LLDP_STR_SIZE - 2); i >= 0; i--)
    {
        if((info->chassis_id[i] & 0xff) != 0x00)
        {
            tmp_len = i + 1;
            break;
        }
    }
    if(tmp_len == 0)
    {
        LLDP_APPEND_SNPRINTF("00&");
    }
    else
    {
        if(info->chassis_id[MAX_LLDP_STR_SIZE - 1] == CHASSIS_ID_SUBTYPE_MAC)
        {
            LLDP_APPEND_SNPRINTF("%02x-%02x-%02x-%02x-%02x-%02x&", \
                                info->chassis_id[0] & 0xff, info->chassis_id[1] & 0xff, info->chassis_id[2] & 0xff, \
                                info->chassis_id[3] & 0xff, info->chassis_id[4] & 0xff, info->chassis_id[5] & 0xff);
        }
        else if(info->chassis_id[MAX_LLDP_STR_SIZE - 1] == CHASSIS_ID_SUBTYPE_NETWORK)
        {
            /* only support ipv4 now*/
            for(i = 0; i < tmp_len; i++)
            {
                if(i == (tmp_len - 1))
                {
                    LLDP_APPEND_SNPRINTF("%d&", info->chassis_id[i] & 0xff);
                }
                else
                {
                    LLDP_APPEND_SNPRINTF("%d.", info->chassis_id[i] & 0xff);
                }
            }
        }
        else
        {
            LLDP_APPEND_SNPRINTF("%s&", info->chassis_id);
        }
    }
    /* port id */
    for(i = (MAX_LLDP_STR_SIZE - 2); i >= 0; i--)
    {
        if((info->port_id[i] & 0xff) != 0x00)
        {
            tmp_len = i + 1;
            break;
        }
    }
    if(tmp_len == 0)
    {
        LLDP_APPEND_SNPRINTF("00&");
    }
    else
    {
        if(info->port_id[MAX_LLDP_STR_SIZE - 1] == PORT_ID_SUBTYPE_MAC)
        {
            LLDP_APPEND_SNPRINTF("%02x-%02x-%02x-%02x-%02x-%02x&", \
                                info->port_id[0] & 0xff, info->port_id[1] & 0xff, info->port_id[2] & 0xff, \
                                info->port_id[3] & 0xff, info->port_id[4] & 0xff, info->port_id[5] & 0xff);
        }
        else if(info->port_id[MAX_LLDP_STR_SIZE - 1] == PORT_ID_SUBTYPE_NETWORK)
        {
            /* only support ipv4 now*/
            for(i = 0; i < tmp_len; i++)
            {
                if(i == (tmp_len - 1))
                {
                    LLDP_APPEND_SNPRINTF("%d&", info->port_id[i] & 0xff);
                }
                else
                {
                    LLDP_APPEND_SNPRINTF("%d.", info->port_id[i] & 0xff);
                }
            }
        }
        else
        {
            LLDP_APPEND_SNPRINTF("%s&", info->port_id);
        }
    }
    /* system name */
    if(!osapi_strlen(info->system_name))
    {
        LLDP_APPEND_SNPRINTF("----&");
    }
    else
    {
        LLDP_APPEND_SNPRINTF("%s&", info->system_name);
    }
    /* ttl */
    LLDP_APPEND_SNPRINTF("%d&", info->time_to_live);
    /* med capability */
    tmp_med = info->med_capabi.med_capability;
    if(tmp_med == 0)
    {
        LLDP_APPEND_SNPRINTF("----&");
    }
    else
    {
        for(i = 0; i < MED_CAPABILITIES_TYPE; i++)
        {
            tmp_value = (tmp_med & (1 << i)) >> i;
            if(tmp_value)
            {
                tmp_count++;
                if(tmp_count == 1)
                {
                    LLDP_APPEND_SNPRINTF("%s", param[i]);
                }
                else
                {
                    LLDP_APPEND_SNPRINTF(",%s", param[i]);
                }
            }
        }
        LLDP_APPEND_SNPRINTF("&");
    }
    /* med device type */
    tmp_value = info->med_capabi.med_device_type;
    if(tmp_value == 0)
    {
        LLDP_APPEND_SNPRINTF("notDefined&");
    }
    else if(tmp_value == 1)
    {
        LLDP_APPEND_SNPRINTF("endClass_I&");
    }
    else if(tmp_value == 2)
    {
        LLDP_APPEND_SNPRINTF("endClass_II&");
    }
    else if(tmp_value == 3)
    {
        LLDP_APPEND_SNPRINTF("endClass_III&");
    }
    else if(tmp_value == 4)
    {
        LLDP_APPEND_SNPRINTF("netConnectivity&");
    }
    else
    {
        LLDP_APPEND_SNPRINTF("reserved&");
    }
    /* network policy */
    tmp_value = 0;
    for(i = 0; i < MAX_NETWORK_POLICY_TLV_NUM; i++)
    {
        tmp_net = info->med_network_policy[i].net_policy;
        if(tmp_net == 0)
        {
            tmp_value |= (1 << i);
        }
        else
        {
            LLDP_APPEND_SNPRINTF("policy%d-", (i+1));
            /* app type */
            switch((tmp_net & 0xff000000) >> 24)
            {
                case 1:
                    LLDP_APPEND_SNPRINTF("voice,");
                    break;
                case 2:
                    LLDP_APPEND_SNPRINTF("voiceSignaling,");
                    break;
                case 3:
                    LLDP_APPEND_SNPRINTF("guestVoice,");
                    break;
                case 4:
                    LLDP_APPEND_SNPRINTF("guestVoiceSignaling,");
                    break;
                case 5:
                    LLDP_APPEND_SNPRINTF("softphoneVoice,");
                    break;
                case 6:
                    LLDP_APPEND_SNPRINTF("videoConference,");
                    break;
                case 7:
                    LLDP_APPEND_SNPRINTF("streamingVideo,");
                    break;
                case 8:
                    LLDP_APPEND_SNPRINTF("videoSignaling,");
                    break;
                default:
                    LLDP_APPEND_SNPRINTF("typeReserved,");
                    break;
            }
            /* policy unknown */
            if((tmp_net & 0x00800000) >> 23)
            {
                LLDP_APPEND_SNPRINTF("policyUnknown,");
            }
            else
            {
                LLDP_APPEND_SNPRINTF("policyDefined,");
            }
            /* tagged */
            if((tmp_net & 0x00400000) >> 22)
            {
                LLDP_APPEND_SNPRINTF("tagged,");
                LLDP_APPEND_SNPRINTF("vlan_%d,", (tmp_net & 0x001ffe00) >> 9);
                LLDP_APPEND_SNPRINTF("l2pri_%d,", (tmp_net & 0x000001c0) >> 6);
                LLDP_APPEND_SNPRINTF("dscpPri_%d  ", tmp_net & 0x0000003f);
            }
            else
            {
                LLDP_APPEND_SNPRINTF("noTagged  ");
            }
        }
    }
    if(tmp_value == 0x3)
    {
        /* no net policy */
        LLDP_APPEND_SNPRINTF("----&");
    }
    else
    {
        LLDP_APPEND_SNPRINTF("&");
    }
    /* extended power via mdi */
    tmp_value = 0;
    tmp_value = info->med_exten_power.power_info.p_info;
    if(!tmp_value)
    {
        LLDP_APPEND_SNPRINTF("----;");
    }
    else
    {
        if(((tmp_value & 0xc0) >> 6) == 0x0)
        {
            LLDP_APPEND_SNPRINTF("powTypePSE,");
            if(((tmp_value & 0x30) >> 4) == 0x0)
            {
                LLDP_APPEND_SNPRINTF("powSourceUnknown,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x1)
            {
                LLDP_APPEND_SNPRINTF("powSourcePrimary,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x2)
            {
                LLDP_APPEND_SNPRINTF("powSourceBackup,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x3)
            {
                LLDP_APPEND_SNPRINTF("powSourceReserved,");
            }
        }
        else if(((tmp_value & 0xc0) >> 6) == 0x1)
        {
            LLDP_APPEND_SNPRINTF("powTypePD,");
            if(((tmp_value & 0x30) >> 4) == 0x0)
            {
                LLDP_APPEND_SNPRINTF("powSourceUnknown,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x1)
            {
                LLDP_APPEND_SNPRINTF("powSourcePSE,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x2)
            {
                LLDP_APPEND_SNPRINTF("powSourceLocal,");
            }
            else if(((tmp_value & 0x30) >> 4) == 0x3)
            {
                LLDP_APPEND_SNPRINTF("powSourcePSEandLocal,");
            }
        }
        else if((((tmp_value & 0xc0) >> 6) == 0x2) || (((tmp_value & 0xc0) >> 6) == 0x3))
        {
            LLDP_APPEND_SNPRINTF("powTypeReserved,");
        }
        /* pow priority */
        if((tmp_value & 0x0f) == 0x0)
        {
            LLDP_APPEND_SNPRINTF("powPriUnknown,");
        }
        else if((tmp_value & 0x0f) == 0x1)
        {
            LLDP_APPEND_SNPRINTF("powPriCritical,");
        }
        else if((tmp_value & 0x0f) == 0x2)
        {
            LLDP_APPEND_SNPRINTF("powPriHigh,");
        }
        else if((tmp_value & 0x0f) == 0x3)
        {
            LLDP_APPEND_SNPRINTF("powPriLow,");
        }
        else
        {
            LLDP_APPEND_SNPRINTF("powPriReserved,");
        }
        /* power value */
        ptr = (UI8_T *)info;
        tmp_value = (*(ptr+LLDP_CLIENT_INFO_SIZE-1) << 8) | (*(ptr+LLDP_CLIENT_INFO_SIZE-2));
        if(tmp_value <= 1023)
        {
            LLDP_APPEND_SNPRINTF("powValue %d mw;", tmp_value * 100);
        }
        else
        {
            LLDP_APPEND_SNPRINTF("powValueReserved;");
        }
    }
    /* Safely clamp returned length to buffer size */
    if (cur_len > buf_length)
    {
        *len = buf_length;
    }
    else
    {
        *len = cur_len;
    }
}
MW_ERROR_NO_T
ssi_get_lldp_neighbor_handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    I8_T i = 0;
    C8_T *ptr_ssi_str = NULL;
    UI16_T total_len = 0;

    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);
    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }
    _lldp_entry_num = 0;
    if(lldp_global_cfg == ENABLE)
    {
        for(i = 0; i < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; i++)
        {
            if(lldp_neibor_index[i] != 0)
            {
                _lldp_entry_num++;
            }
            else
            {
                break;
            }
        }
    }
    total_len += osapi_snprintf(ptr_ssi_str, ptr_tag_param->tag_insert_max_len,
                          "<script>\nvar lldp_global_cfg = %d;\nvar lldp_neighbor_num = %d;\n</script>\n",
                          lldp_global_cfg, _lldp_entry_num);
#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;
    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] exit\n", __FUNCTION__, __LINE__);
    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_neighbor_info_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T tmp_port;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0, index = 0;
    DB_LLDP_CLIENT_INFO_T info;
    UI16_T buf_len_param = 0;
    void *db_data = NULL;
    C8_T *ptr_ssi_str = NULL;
    UI32_T tag_insert_max_len = 0;
    UI16_T total_len = 0;
    UI16_T remaining_len = 0;
    UI16_T start_index = 0;
    UI16_T i = 0;

    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);
    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

#if LWIP_HTTPD_SSI_MULTIPART
    start_index = ptr_tag_param->current_tag_part * LLDP_READ_NUM;
#else
    start_index = 0;
#endif

    osapi_memset(&info, 0, sizeof(info));
    for (i = start_index; (i < start_index + LLDP_READ_NUM) && (i < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT); i++)
    {
        if (lldp_neibor_index[i] != 0)
        {
            index = lldp_neibor_index[i];
            rc = httpd_queue_getData(LLDP_CLIENT_INFO, DB_ALL_FIELDS, index, &db_msg, &db_size, &db_data);
            if(MW_E_OK == rc)
            {
                CGI_LOG_DEBUG(LLDP, "get lldp client index [%d] info success, ptr_msg =%p\n", index, db_msg);
                osapi_memcpy(&info, db_data, db_size);
                osapi_free(db_msg);
            }
            else
            {
                CGI_LOG_DEBUG(LLDP, "get lldp client index [%d] info failed\n", index);
                return MW_E_BAD_PARAMETER;
            }
            tmp_port = (index - 1) / MAX_LLDP_CLIENT_PER_PORT + 1;

            if (total_len >= tag_insert_max_len)
            {
                remaining_len = 0;
            }
            else
            {
                remaining_len = tag_insert_max_len - total_len;
            }
            buf_len_param = remaining_len;

            /* Write directly to SSI buffer */
            analysisLldpClientInfo(tmp_port, index, &info, &buf_len_param, ptr_ssi_str + total_len);

            if (total_len + buf_len_param <= tag_insert_max_len)
            {
                total_len += buf_len_param;
            }
            else
            {
                total_len = tag_insert_max_len;
            }
        }
    }

#if LWIP_HTTPD_SSI_MULTIPART
    if (start_index + LLDP_READ_NUM < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT)
    {
        ptr_tag_param->next_tag_part = ptr_tag_param->current_tag_part + 1;
    }
    else
    {
        /* All data processed. Add the end flag "888;" if it fits. */
        if (total_len + 4 < tag_insert_max_len)
        {
             total_len += osapi_snprintf(ptr_ssi_str + total_len, tag_insert_max_len - total_len, "888;");
        }
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
    }
#else
    /* If multipart not supported, we just finish here. */
    if (total_len + 4 < tag_insert_max_len)
    {
         total_len += osapi_snprintf(ptr_ssi_str + total_len, tag_insert_max_len - total_len, "888;");
    }
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;
    return MW_E_OK;
}
