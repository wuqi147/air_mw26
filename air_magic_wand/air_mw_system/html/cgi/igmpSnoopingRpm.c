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

/* FILE NAME:   igmpSnoopingRpm.c
 * PURPOSE:
 *      CGI and SSI function of IGMP Snoop web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#include "vlan_utils.h"
#include "inet_utils.h"
#include "mac_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMP_SNP_TEMP_BUF_LEN               (256)

#define IGMP_SNP_READ_NUM                   (8) /* The number of group entry in a single web request */
#define MAX_MAC_ENTRY_STR_LEN               (MAC_ADDR_STR_LEN + MAX_VID_STR_LEN + MAX_PORTBMP_STR_LEN + 3)
#define MAX_IPMC_ENTRY_STR_LEN              (IP4ADDR_STRLEN_MAX + MAX_VID_STR_LEN + MAX_PORTBMP_STR_LEN + 3)
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
#define MAX_GROUP_ENTRY_STR_LEN             (MAX_MAC_ENTRY_STR_LEN)
#else
#define MAX_GROUP_ENTRY_STR_LEN             (MAX_IPMC_ENTRY_STR_LEN)
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#define IGMP_SNP_QUERIER_READ_NUM           (8) /* The number of querier entry in a single web request */
#define MAX_QUERIER_ENTRY_STR_LEN           (MAX_VID_STR_LEN + 3 + IP4ADDR_STRLEN_MAX + 5)
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
UI16_T _entry_num = 0;
igmp_snooping_info_t igmp_snooping =
{
        0,
        0,
        0,
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
static UI8_T _querier_entry_num = 0;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_l2mc_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T rc = MW_E_OP_INVALID;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T   db_size = 0;
    DB_L2_MC_ENTRY_T *ptr_l2mc_info = NULL;
    UI16_T   index = 0, count = 0;
    UI16_T   total_len = 0, tmp_len = 0;
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    MW_MAC_T mac_addr = {0}, zero_mac_addr = {0};
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    C8_T *ptr_buf = NULL;
    UI32_T buf_len = 0;

    /*  expected xml string
     *  0;
     *  16;
     *  01-00-5e-00-0e-6d,0x2&
     *  01-00-5e-00-0e-6f,0x4&
     *  01-00-5e-00-0e-6a,0x2&
     *  01-00-5e-00-0e-6b,0x4&
     *  01-00-5e-00-0e-6c,0x2&;
     *  5
     */

    if (TRUE == igmp_snooping.igmp_mode)
    {
        /* Get l2mc info */
        rc = httpd_queue_getData(L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &db_size, (void**)&ptr_l2mc_info);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(IGMP_SNP, "Get igmp l2mc info failed!");
            return rc;
        }

        ptr_buf = ptr_tag_param->ptr_tag_insert;
        buf_len = ptr_tag_param->tag_insert_max_len;
        if (NULL == ptr_buf)
        {
            MW_FREE(ptr_msg);
            return MW_E_BAD_PARAMETER;
        }

        index = _entry_num;
        tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), "%d;%d;",
                                                        index, IGMP_SNP_READ_NUM);

        for (index = _entry_num; ((index < (_entry_num + IGMP_SNP_READ_NUM)) && (index < MAX_L2MC_NUM)); index++)
        {
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
            osapi_memcpy(mac_addr, ptr_l2mc_info->mac_addr[index], sizeof(MW_MAC_T));
            if (1 == macAddrCmp((UI8_T *)(&mac_addr), (UI8_T *)(&zero_mac_addr)))
            {
                    tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), "%02x%02x%02x%02x%02x%02x,%d,0x%x&",
                                                                mac_addr[0],
                                                                mac_addr[1],
                                                                mac_addr[2],
                                                                mac_addr[3],
                                                                mac_addr[4],
                                                                mac_addr[5],
                                                                ptr_l2mc_info->vid[index],
                                                                ptr_l2mc_info->portbitmap[index]);
                    count++;
            }
#else
            if (0 != (ptr_l2mc_info->ip_addr[index]))
            {
                    tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), "0x%x,%d,0x%x&",
                                                                ptr_l2mc_info->ip_addr[index],
                                                                ptr_l2mc_info->vid[index],
                                                                ptr_l2mc_info->portbitmap[index]);
                    count++;
            }
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
        }
        MW_FREE(ptr_msg);
        tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), ";%d", count);

        total_len += tmp_len;
#if LWIP_HTTPD_SSI_MULTIPART
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
        ptr_tag_param->tag_insert_len = total_len;
        ptr_tag_param->chunk_enable = TRUE;

        _entry_num += IGMP_SNP_READ_NUM;
    }

    if(MAX_L2MC_NUM <= _entry_num)
    {
        _entry_num = 0;
    }
    return rc;
}

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
MW_ERROR_NO_T
ssi_get_igmp_snp_querier_info_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T rc = MW_E_OP_INVALID;
    DB_IGMP_SNP_QUERIER_INFO_T *ptr_querier_info = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T   db_size;
    UI16_T   total_len = 0, tmp_len = 0;
    UI16_T   index = 0, count = 0;
    C8_T     *ptr_buf = NULL;
    UI32_T   buf_len = IGMP_SNP_TEMP_BUF_LEN;

    /*  expected xml string
     *  0;
     *  16;
     *  1,0,1,0,192.168.0.200&
     *  3,1,1,1,192.168.0.201&
     *  4,1,0,0,192.168.0.202&
     *  6,2,1,0,192.168.0.203&
     *  7,1,0,1,0.0.0.0&;
     *  5
     */

    if (TRUE == igmp_snooping.igmp_mode)
    {
        /* get igmp snooping querier info */
        rc = httpd_queue_getData(IGMP_SNP_QUERIER_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &db_size, (void**)&ptr_querier_info);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(IGMP_SNP, "Get igmp snooping querier info failed!");
            return rc;
        }
        ptr_buf = ptr_tag_param->ptr_tag_insert;
        buf_len = ptr_tag_param->tag_insert_max_len;
        if (NULL == ptr_buf)
        {
            MW_FREE(ptr_msg);
            return MW_E_BAD_PARAMETER;
        }

        index = _querier_entry_num;
        tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), "%d;%d;",
                                                        index, IGMP_SNP_QUERIER_READ_NUM);

        for (index = _querier_entry_num; ((index < (_querier_entry_num + IGMP_SNP_QUERIER_READ_NUM)) && (index < MAX_VLAN_ENTRY_NUM)); index++)
        {
            if(VLAN_MIN_VID != ptr_querier_info->vlan_id[index])
            {
                tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), "%d,%d,%d,%d,%d.%d.%d.%d&",
                                                ptr_querier_info->vlan_id[index],
                                                ptr_querier_info->state[index],
                                                ptr_querier_info->election[index],
                                                ptr_querier_info->version[index],
                                                (0xFF & ((UI8_T*)(&(ptr_querier_info->address[index])))[0]),
                                                (0xFF & ((UI8_T*)(&(ptr_querier_info->address[index])))[1]),
                                                (0xFF & ((UI8_T*)(&(ptr_querier_info->address[index])))[2]),
                                                (0xFF & ((UI8_T*)(&(ptr_querier_info->address[index])))[3]));
                count++;
            }
        }
        MW_FREE(ptr_msg);
        tmp_len += osapi_snprintf(ptr_buf + tmp_len, (buf_len - tmp_len), ";%d", count);

        total_len += tmp_len;
#if LWIP_HTTPD_SSI_MULTIPART
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
        ptr_tag_param->tag_insert_len = total_len;
        ptr_tag_param->chunk_enable = TRUE;

        _querier_entry_num += IGMP_SNP_QUERIER_READ_NUM;
    }

    if(MAX_VLAN_ENTRY_NUM <= _querier_entry_num)
    {
        _querier_entry_num = 0;
    }
    return rc;
}
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

MW_ERROR_NO_T cgi_set_handle_igmpSnoopingRpm(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    I32_T i = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T igmp_mode = 0, reportSu_mode = 0, fastleave_mode = 0;
    UI8_T isIgmpModeUpdate = 0, isRptSuppUpdate = 0, isFastLeaveUpdate = 0, isStaticRouter = 0;
    UI32_T routerPort = 0;
    UI8_T rData[MAX_PORT_NUM] = {0};
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    UI8_T  state, election, version;
    UI16_T vlan_id;
    MW_IPV4_T address;
    UI8_T isQuerVidUpdate = 0, isQuerModeUpdate = 0, isQuerElectUpdate = 0, isQuerVerUpdate = 0, isQuerAddrUpdate = 0;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;
    UI8_T db_idx = 0, index = 0;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if(!osapi_strcmp(pcParam[i], "igmp_mode"))
        {
            igmp_mode = atoi(pcValue[i]);
            isIgmpModeUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "reportSu_mode"))
        {
            reportSu_mode = atoi(pcValue[i]);
            isRptSuppUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "fastleave_mode"))
        {
            fastleave_mode = atoi(pcValue[i]);
            isFastLeaveUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "mrouter"))
        {
            routerPort = atol(pcValue[i]);
            isStaticRouter = 1;
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == osapi_strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
        if(!osapi_strcmp(pcParam[i], "vlan_id"))
        {
            vlan_id = atol(pcValue[i]);
            isQuerVidUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "querier_mode"))
        {
            state = (0 < atol(pcValue[i]) ? ENABLE : DISABLE);
            isQuerModeUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "querier_elect"))
        {
            election = (0 < atol(pcValue[i]) ? ENABLE : DISABLE);
            isQuerElectUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "querier_ver"))
        {
            version = atol(pcValue[i]);
            isQuerVerUpdate = 1;
        }
        if(!osapi_strcmp(pcParam[i], "querier_addr"))
        {
            address = getIpaddr(pcValue[i]);
            isQuerAddrUpdate = 1;
        }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
    }

    if(isIgmpModeUpdate)
    {
        rc = httpd_queue_setData(M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_ENABLE, DB_ALL_ENTRIES, &igmp_mode, sizeof(igmp_mode));
    }

    if(isRptSuppUpdate)
    {
        rc = httpd_queue_setData(M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_RPT_SUPPRESS, DB_ALL_ENTRIES, &reportSu_mode, sizeof(reportSu_mode));
    }

    if(isFastLeaveUpdate)
    {
        rc = httpd_queue_setData(M_UPDATE, IGMP_SNP_INFO, IGMP_SNP_FAST_LEAVE, DB_ALL_ENTRIES, &fastleave_mode, sizeof(fastleave_mode));
    }

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    if(0 != isQuerVidUpdate)
    {
        /* get igmp snooping querier vlan info */
        rc = httpd_queue_getData(IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(IGMP_SNP, "get igmp_snp querier vlan id failed");
            return ERR_VAL;
        }

        for (index = 0; index < MAX_VLAN_ENTRY_NUM; index++)
        {
            if (vlan_id == ((UI16_T *)ptr_db_data)[index])
            {
                db_idx = index + 1;
                break;
            }
        }
        MW_FREE(ptr_db_msg);
    }
    if((0 != db_idx) && (0 != isQuerModeUpdate))
    {
        httpd_queue_setData(M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_STATE, db_idx, &state, sizeof(UI8_T));
    }
    if((0 != db_idx) && (0 != isQuerElectUpdate))
    {
        httpd_queue_setData(M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_ELECTION, db_idx, &election, sizeof(UI8_T));
    }
    if((0 != db_idx) && (0 != isQuerVerUpdate))
    {
        httpd_queue_setData(M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VERSION, db_idx, &version, sizeof(UI8_T));
    }
    if((0 != db_idx) && (0 != isQuerAddrUpdate))
    {
        rc = httpd_queue_setData(M_UPDATE, IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_ADDRESS, db_idx, &address, sizeof(MW_IPV4_T));
    }
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    if(isStaticRouter)
    {
        memset(rData, 0, PLAT_MAX_PORT_NUM);
#if(defined(AIR_SUPPORT_SFP))
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&routerPort,0);
#endif
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(0 != (routerPort &(0x01 << i)))
            {
                rData[i] = 1;
            }
        }
        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_IGMP_ROUTER, DB_ALL_ENTRIES, rData, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    }

    return rc;
}

char ssi_get_igmp_snooping_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags)
{
    UI16_T len = 0, total_len = 0;
    char err = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void   *ptr_db_data = NULL;
    UI16_T index = 0, count = 0;
    DB_IGMP_SNP_INFO_T igmp_snp_info = {0};
    UI8_T    searchmacTab = DISABLE;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;
    UI8_T vlan_state = VLAN_NONE;
    UI32_T tmpvalue = 0, rsticPort = 0, rdynaPort = 0;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    UI8_T     vlan_count = 0, buf_len = 0;
    C8_T      *ptr_vid_buf = NULL;
    UI8_T    vid_str_len = 6; /* max vid string length(4B) + ","(1B) + space(1B) */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    *ptr_length = 0;
    /* get igmp snoop enable info */
    rc = httpd_queue_getData(IGMP_SNP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(IGMP_SNP, "Get IGMP SNP INFO data failed");
        return ERR_VAL;
    }

    osapi_memcpy(&igmp_snp_info, ptr_db_data, db_size);
    MW_FREE(ptr_db_msg);
    igmp_snooping.igmp_mode = igmp_snp_info.enable;
    igmp_snooping.igmp_fastleave_mode = igmp_snp_info.fast_leave;
    igmp_snooping.reportSu_mode = igmp_snp_info.rpt_suppress;

    _entry_num = 0;
    err = send_format_response(&len, ptr_pcb, apiflags, "<script>var igmp_ds = {state:%d,fastleaveState:%d,suppressionState:%d,",
                                                                          igmp_snooping.igmp_mode,
                                                                          igmp_snooping.igmp_fastleave_mode,
                                                                          igmp_snooping.reportSu_mode);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    /* get igmp vlan info */
    rc = httpd_queue_getData(IGMP_SNP_QUERIER_INFO, IGMP_SNP_QUERIER_VLAN_ID, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        return ERR_VAL;
    }
    osapi_calloc((MAX_VLAN_ENTRY_NUM * vid_str_len), "igmp_snp", (void**)&ptr_vid_buf);
    if(NULL == ptr_vid_buf)
    {
        MW_FREE(ptr_db_msg);
        return ERR_MEM;
    }
    for (index = 0; index < MAX_VLAN_ENTRY_NUM; index++)
    {
        if (VLAN_MIN_VID != ((UI16_T *)ptr_db_data)[index])
        {
            vlan_count ++;
            buf_len += osapi_snprintf(ptr_vid_buf + buf_len, (MAX_VLAN_ENTRY_NUM * vid_str_len) - buf_len, "%u,", ((UI16_T *)ptr_db_data)[index]);
        }
    }
    MW_FREE(ptr_db_msg);
    err = send_format_response(&len, ptr_pcb, apiflags, "vids:[%s],vlan_count:%d,", ptr_vid_buf, vlan_count);
    if(err != ERR_OK)
    {
        MW_FREE(ptr_vid_buf);
        return err;
    }
    total_len += len;
    MW_FREE(ptr_vid_buf);

    _querier_entry_num = 0;
    err = send_format_response(&len, ptr_pcb, apiflags, "querierState:[],querierElect:[],querierAddr:[],");
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3
    err = send_format_response(&len, ptr_pcb, apiflags, "querierVer:[],");
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    /* get igmp snoop static router info */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_IGMP_ROUTER, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(IGMP_SNP, "Get static router data failed");
        return ERR_VAL;
    }
    for(index = 0; index < PLAT_MAX_PORT_NUM; index++)
    {
        tmpvalue = ((UI8_T*)ptr_db_data)[index];
        rsticPort  |= (tmpvalue << index);
    }
    MW_FREE(ptr_db_msg);

    /* get igmp snoop dynamic router info */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_ROUTER, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, &ptr_db_data);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(IGMP_SNP, "Get dynamic router data failed");
        return ERR_VAL;
    }
    for(index = 0; index < PLAT_MAX_PORT_NUM; index++)
    {
        tmpvalue = ((UI8_T*)ptr_db_data)[index];
        if(0 != tmpvalue)
        {
            rdynaPort |= (0x01 << index);
        }
    }
    MW_FREE(ptr_db_msg);

    /* Get DB VLAN_CFG_INFO */
    rc = httpd_queue_getData(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_db_msg, &db_size, (void **)&ptr_vlan_cfg);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(IGMP_SNP, "Get VLAN config info data failed ");
        return rc;
    }

    vlan_state = (TRUE == ptr_vlan_cfg->enable_port_b)? VLAN_PORT_ENABLE : VLAN_1Q_ENABLE;
    MW_FREE(ptr_db_msg);
    if (TRUE == igmp_snooping.igmp_mode)
    {
        count = MAX_L2MC_NUM;
    }
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    searchmacTab = ENABLE;
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    err = send_format_response(&len, ptr_pcb, apiflags, "rstic:%d,rdyna:%d,vlanState:%d,count:%d,searchmacTab:%d}</script>",
                                                                                                                        rsticPort ,
                                                                                                                        rdynaPort,
                                                                                                                        vlan_state,
                                                                                                                        count,
                                                                                                                        searchmacTab);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    *ptr_length = total_len;

    return ERR_OK;
}

