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

/* FILE NAME:   macCfg.c
 * PURPOSE:
 *      CGI and SSI function of static MAC address configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include "mw_utils.h"
#include "mac_utils.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "web.h"
#include "air_swc.h"
#include "osapi_thread.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MACCFG_PORT_MAX_HOST_NUM    (100)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct CGI_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_S{
    UI8_T       firstreq; /* First request or not */
    UI8_T       times;    /* The times of DB updates required to load the whole webpage */
}ATTRIBUTE_PACK CGI_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T;

typedef enum MACCFG_ERR_TYPE_E
{
    MACCFG_NO_ERR = 0,             /* No error */
    MACCFG_MAC_TABLE_FULL,         /* MAC table is full  */
    MACCFG_LAST
}MACCFG_ERR_TYPE_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static CGI_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T _cgi_dynamic_mac_address_entry_info = {0};
static MACCFG_ERR_TYPE_T _maccfg_errType = MACCFG_NO_ERR;

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_maxMacNumSet(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    I32_T i;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T portBmp = 0, temp = 0;
    UI16_T maxnum = 0;
    UI32_T portIdx = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkpbm = 0;
#endif
    /* expected CGI format:
     * maxmacnumSet.cgi?port=1&max=10 */

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "portBmp")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            portBmp = temp;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "max")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            maxnum = (UI16_T) temp;
        }
#if(defined(AIR_SUPPORT_SFP))
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "trunkBitMap")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            trunkpbm = temp;
        }
#endif
    }
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkpbm,&portBmp,0);
#endif
    /* parser params to db format */
    CGI_LOG_DEBUG(MAC, "<%s:%u>Set DB port=%u,maxnum=%u,size=%lu", __func__, __LINE__, portBmp, maxnum, sizeof(maxnum));
    for (portIdx = 0; portIdx < PLAT_MAX_PORT_NUM; portIdx++)
    {
        if (0 != ( portBmp & (1<<portIdx) ))
        {
            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_MAC_LIMIT, (portIdx+1), &maxnum, sizeof(maxnum));
        }
        if (MW_E_OK != rc)
        {
            break;
        }
    }
    return rc;
}

MW_ERROR_NO_T
cgi_set_handle_addStaticMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    I32_T i;
    I32_T rc = MW_E_OK;
    C8_T mac_str[13] = {0};
    UI16_T idx = 0;
    ONE_DB_STATIC_MAC_ENTRY_T mac_data;
    ether_addr_t ether_addr;
    UI32_T portIdx = 0, portmap = 0, temp = 0;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0;
    UI8_T  member_0 = 0;
    DB_TRUNK_PORT_T trunk_info;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkpbm = 0;
#endif
    UI32_T unit = 0;
    UI32_T param = 0, capacity = 0, usage = 0;
    AIR_SWC_RSRC_T rsrc_type = AIR_SWC_RSRC_L2_FDB;

    rc = air_swc_getCapacity(unit, rsrc_type, param, &capacity);
    if (AIR_E_OK == rc)
    {
        rc = air_swc_getUsage(unit, rsrc_type, param, &usage);
    }
    if ((AIR_E_OK == rc) && (usage == capacity))
    {
        _maccfg_errType = MACCFG_MAC_TABLE_FULL;
        return MW_E_OK;
    }
    osapi_memset(&mac_data, 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T));
    /* expected CGI format:
     * addstaticMac.cgi?port=0&vlan=1&mac=001122335599&idx=3 */
    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "port")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            portmap = temp;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "vlan")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            mac_data.vid = (UI16_T)temp;
        }
        else if (!strcmp(ptr_pcParam[i], "mac"))
        {
            osapi_memset(mac_str, 0, 13);
            osapi_strncpy(mac_str, ptr_pcValue[i], 12);
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "idx")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            idx = temp;
        }
#if(defined(AIR_SUPPORT_SFP))
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "trunkBitMap")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            trunkpbm = temp;
        }
#endif
    }
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkpbm,&portmap,0);
#endif
    memset(mac_data.mac_addr, 0, sizeof(MW_MAC_T));
    strToMac((UI8_T*)mac_str, &ether_addr);
    memcpy(mac_data.mac_addr, ether_addr.octet, ETHER_ADDR_LEN);

    if(0 != (portmap & (portmap - 1)))
    {
        /* port in trunk */
        rc = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        osapi_memset(&trunk_info, 0, sizeof(DB_TRUNK_PORT_T));
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            osapi_memcpy(&trunk_info, (void *)((UI8_T *) ptr_data + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            if (0 != (trunk_info.members.member_bmp & portmap))
            {
                break;
            }
        }
        MW_FREE(ptr_msg);
        if(MAX_TRUNK_NUM == i)
        {
            return MW_E_OP_INVALID;
        }
        rc = httpd_queue_getData(LAG_MEMBER_0_INFO, MEMBER_0, (i + 1), &ptr_msg, &size, (void**)&ptr_data);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        osapi_memcpy(&member_0, (void *)ptr_data, sizeof(UI8_T));
        MW_FREE(ptr_msg);
        mac_data.port = member_0;
    }
    else
    {
        for (portIdx = 0; portIdx < PLAT_MAX_PORT_NUM; portIdx++)
        {
            if (0 != ( portmap & BIT(portIdx)))
            {
                mac_data.port = (portIdx+1);
                break;
            }
        }
    }
    rc = httpd_queue_setData(M_CREATE, STATIC_MAC_ENTRY, DB_ALL_FIELDS, idx, &mac_data, sizeof(mac_data));
    CGI_LOG_DEBUG(MAC, "Set DB idx=%u,port=%u,vid=%u,mac=", idx, mac_data.port, mac_data.vid);
    /* SyncD will delete the new mac entry if creation failed.
     * Add a few delay to get the correct data from DB */
    osapi_delay(100);

    return rc;
}

MW_ERROR_NO_T
cgi_set_handle_delStaticMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    I32_T i;
    MW_ERROR_NO_T rc, ret = MW_E_OK;
    UI32_T pbm = 0;
    UI16_T idx;

    /* expected CGI format:
     * delstaticmac.cgi?del_bit=3 */

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if(!strcmp(ptr_pcParam[i],"del_bit"))
        {
            pbm = strtoul(ptr_pcValue[i], NULL, 10);
        }
    }
    /* parser params to db format */
    for (idx = 1; idx <= MAX_STATIC_MAC_NUM; idx++)
    {
        if (pbm & BIT(idx - 1))
        {
            CGI_LOG_DEBUG(MAC, "Delete entry(%u)", idx);
            rc = httpd_queue_setData(M_DELETE,
                    STATIC_MAC_ENTRY, DB_ALL_FIELDS, idx,
                    NULL, 0);
            if (MW_E_OK != rc)
            {
                CGI_LOG_ERROR(MAC, "Delete entry(%u) fail(%d)", idx, rc);
                ret = rc;
                continue;
            }
        }
    }

    return ret;
}

MW_ERROR_NO_T
cgi_set_handle_freshDynamicMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    I32_T i;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T action = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_DEFAULT;
    /* expected CGI format:
     * freshdynamicmac.cgi?firstreq=1&times=4 */

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if(0 == strcmp(ptr_pcParam[i],"firstreq"))
        {
            _cgi_dynamic_mac_address_entry_info.firstreq = (1 == atoi(ptr_pcValue[i])) ? TRUE : FALSE;
        }

        if(0 == strcmp(ptr_pcParam[i],"times"))
        {
            _cgi_dynamic_mac_address_entry_info.times = atoi(ptr_pcValue[i]);
        }
    }
    if((TRUE == _cgi_dynamic_mac_address_entry_info.firstreq) && (0 != _cgi_dynamic_mac_address_entry_info.times))
    {
        action = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_START;
    }
    else if((FALSE == _cgi_dynamic_mac_address_entry_info.firstreq) && (0 != _cgi_dynamic_mac_address_entry_info.times))
    {
        action = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_CONTINUE;
    }
    else
    {
        return ERR_VAL;
    }
    rc = httpd_queue_setData(M_UPDATE, DYNAMIC_MAC_ADDRESS_ENTRY_CFG, ACTION_RESULT, DB_ALL_ENTRIES, &action, sizeof(UI8_T));
    return rc;
}

//==========================================================================================
//  SSI handler
//==========================================================================================
MW_ERROR_NO_T
ssi_get_static_mac_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    UI32_T len = 0, total_len = 0;
    C8_T mac_str[13] = {0};
    C8_T *ptr_tag_insert = NULL;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_msg;
    MW_MAC_T *ptr_mac_addr;
    UI16_T data_size;
    UI8_T show_static_mac = TRUE;
    ether_addr_t ether_addr;

    /*  expected javascript string
     *  <script>
     *  var staticinfo_ds = {
     *  sys_mac:'00aabb112233',
     *  maxhostnum:32,
     *  }
     *  </script>
     */
    if ((NULL == ptr_tag_param) || (NULL == ptr_tag_param->ptr_tag_insert) || (0 == ptr_tag_param->tag_insert_max_len))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_tag_insert = ptr_tag_param->ptr_tag_insert;
    total_len = ptr_tag_param->tag_insert_max_len;

    len += snprintf(ptr_tag_insert + len, total_len - len, "<script>var staticinfo_ds={");
    /* Get system MAC address */
    mw_rc = httpd_queue_getData(
            SYS_OPER_INFO, SYS_OPER_MAC, 1,
            &ptr_msg, &data_size, (void**)&ptr_mac_addr);
    if (MW_E_OK != mw_rc)
    {
        return mw_rc;
    }
    osapi_memcpy(ether_addr.octet, *ptr_mac_addr, ETHER_ADDR_LEN);
    MW_FREE(ptr_msg);
    macToStr(&ether_addr, (UI8_T*)mac_str, FALSE);

#ifdef AIR_SUPPORT_ERPS
    /* Not support configure static MAC when ERPS is supported */
    show_static_mac = FALSE;
#endif /* AIR_SUPPORT_ERPS */

    len += snprintf(ptr_tag_insert + len, total_len - len, "sys_mac:'%s', maxhostnum:%u, errType:%u, show:%u};</script>",
                                                            mac_str,
                                                            MAX_STATIC_MAC_NUM,
                                                            _maccfg_errType,
                                                            show_static_mac);
    _maccfg_errType = MACCFG_NO_ERR;

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif

    ptr_tag_param->tag_insert_len = len;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

C8_T
ssi_get_port_maxmac_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    UI8_T alloc_buf_len = 128;
    C8_T *ptr_hostnumbuf = NULL;
    C8_T err = 0;
    UI16_T len = 0, i = 0;
    I32_T buf_len = 0;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_msg;
    UI16_T data_size;
    UI16_T *ptr_mac_limit;        /* The maximum MAC number of the port */

    /*  expected value
     *  <script>
     *  var portinfo_ds = {
     *      portnum:28,
     *      maxhostnum:100,
     *      maxhostnums:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,]}
     *  </script>
     */

    mw_rc = osapi_calloc(alloc_buf_len, HTTPD_QUEUE_CLI, (void **)&ptr_hostnumbuf);
    if(MW_E_OK != mw_rc)
    {
        return ERR_INPROGRESS;
    }

    mw_rc = httpd_queue_getData(
            PORT_CFG_INFO, PORT_MAC_LIMIT, DB_ALL_ENTRIES,
            &ptr_msg, &data_size, (void**)&ptr_mac_limit);
    if (MW_E_OK != mw_rc)
    {
        MW_FREE(ptr_hostnumbuf);
        return ERR_INPROGRESS;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        buf_len += osapi_snprintf(ptr_hostnumbuf + buf_len, alloc_buf_len - buf_len, "%u,", ptr_mac_limit[i]);
    }

    len = 0;
    err = send_format_response(&len, ptr_pcb, apiflags,
        "<script>var portinfo_ds={portnum:%u,maxhostnum:%u,maxhostnums:[%s]}</script>",
        PLAT_MAX_PORT_NUM,
        MACCFG_PORT_MAX_HOST_NUM,
        ptr_hostnumbuf);

    /* Release allocated memory */
    MW_FREE(ptr_msg);
    MW_FREE(ptr_hostnumbuf);

    if (ERR_OK != err)
    {
        return err;
    }

    *ptr_length = len;

    return ERR_OK;
}

C8_T
ssi_get_vlan_mac_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    UI16_T len = 0, i = 0;
    C8_T err = 0;
    UI8_T alloc_buf_len = 161;
    C8_T *ptr_vid_buf = NULL;
    I32_T buf_len = 0;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_msg;
    UI16_T data_size;
    UI16_T *ptr_vlan_id;
    UI8_T ety_cnt = 0;

    /*  expected value
     *  <script>
     *  var vlan_ds = {
     *      vids:[1,100,200,1000,2000],
     *      count:5}
     *  </script>
     */

    mw_rc = osapi_calloc(alloc_buf_len, HTTPD_QUEUE_CLI, (void **)&ptr_vid_buf);
    if(MW_E_OK != mw_rc)
    {
        return ERR_INPROGRESS;
    }

    mw_rc = httpd_queue_getData(
            VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES,
            &ptr_msg, &data_size, (void**)&ptr_vlan_id);
    if (MW_E_OK != mw_rc)
    {
        MW_FREE(ptr_vid_buf);
        return ERR_INPROGRESS;
    }

    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if( 0 != ptr_vlan_id[i])
        {
            ety_cnt++;
            buf_len += osapi_snprintf(ptr_vid_buf + buf_len, alloc_buf_len - buf_len, "%u,", ptr_vlan_id[i]);
        }
    }

    /* Release allocated memory */
    MW_FREE(ptr_msg);

    CGI_LOG_DEBUG(MAC, "<%s:%u> cnt=%u vlans=[%s]", __func__, __LINE__, ety_cnt, ptr_vid_buf);
    err = send_format_response(&len, ptr_pcb, apiflags,
        "<script>var vlan_ds={vids:[%s],count:%u};</script>",
        ptr_vid_buf,
        ety_cnt);

    MW_FREE(ptr_vid_buf);
    if (ERR_OK != err)
    {
        return err;
    }

    *ptr_length = len;

    return ERR_OK;
}

C8_T
ssi_get_dynamic_mac_address_entry_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    C8_T err = 0;
    UI16_T len = 0;
    /*  expected javascript string
     *  <script>
     *  var dynamiccfg = {
     *  maxshownums:200,
     *  perrequests:50,
     *  }
     *  </script>
     */
    err = send_format_response(&len, ptr_pcb, apiflags,
        "<script>var dynamiccfg={maxshownums:%u,perrequests:%u,};</script>",
        MAX_PER_WEBPAGE_SHOW_DYNAMIC_MAC_ADDRESS_ENTRY_NUM,
        PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM
        );
    if (ERR_OK != err)
    {
        return err;
    }
    *ptr_length = len;
    return ERR_OK;
}


MW_ERROR_NO_T
ssi_get_static_mac_address_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    C8_T                    mac_str[13] = {0};
    C8_T                    *ptr_tag_insert = NULL;
    UI32_T                  len = 0, total_len = 0, i = 0;
    UI16_T                  index = 0;
    UI16_T                  data_size;
    DB_MSG_T                *ptr_msg = NULL;
    const UI8_T             per_part_num = 8, tag_part = (MAX_STATIC_MAC_NUM + per_part_num - 1) / per_part_num;
    ether_addr_t            ether_addr;
    static UI16_T           count = 0;
    MW_ERROR_NO_T           mw_rc = MW_E_OK;
    DB_STATIC_MAC_ENTRY_T   *ptr_mac_entry;
    /*  expected xml string
     *  0;
     *  8;
     *  1,1000,001122334455,1&
     *  2,2000,00aabbccddee,8&
     *  3,5,00aabbccddff,5&;
     *  3
     */
    if ((NULL == ptr_tag_param) || (NULL == ptr_tag_param->ptr_tag_insert) || (0 == ptr_tag_param->tag_insert_max_len))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_tag_insert = ptr_tag_param->ptr_tag_insert;
    total_len = ptr_tag_param->tag_insert_max_len;

#if LWIP_HTTPD_SSI_MULTIPART
    index = (ptr_tag_param->current_tag_part) * per_part_num;
    if ((tag_part - 1) > (ptr_tag_param->current_tag_part))
    {
        ptr_tag_param->next_tag_part = ptr_tag_param->current_tag_part + 1;
    }
    else
    {
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
    }
#endif

#if LWIP_HTTPD_SSI_MULTIPART
    /* first part */
    if (0 == ptr_tag_param->current_tag_part)
#endif
    {
        count = 0;
        len += osapi_snprintf(ptr_tag_insert + len, total_len - len,"%d;%d;",
                                0, MAX_STATIC_MAC_NUM);
    }

    /* Get static MAC address information */
    mw_rc = httpd_queue_getData(
            STATIC_MAC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES,
            &ptr_msg, &data_size, (void**)&ptr_mac_entry);
    if (MW_E_OK != mw_rc)
    {
        return mw_rc;
    }
    do
    {
        for (i = index; ((i < (index + per_part_num)) && (i < MAX_STATIC_MAC_NUM)); i++)
        {
            /* Port value in DB is 1-based, 0 means invalid entry */
            if (0 != ptr_mac_entry ->port[i])
            {
                osapi_memcpy(ether_addr.octet, ptr_mac_entry->mac_addr[i], ETHER_ADDR_LEN);
                macToStr(&ether_addr, (UI8_T*)mac_str, FALSE);
                len += osapi_snprintf(ptr_tag_insert + len, total_len - len,
                    "%u,%u,%s,%u&",
                    i + 1,
                    ptr_mac_entry ->vid[i],
                    mac_str,
                    ptr_mac_entry ->port[i]);

                count++;
            }
        }
        if(0 != len)
        {
            break;
        }
        index += per_part_num;
#if LWIP_HTTPD_SSI_MULTIPART
        ptr_tag_param->next_tag_part += 1;
        ptr_tag_param->next_tag_part = (tag_part > ptr_tag_param->next_tag_part) ? ptr_tag_param->next_tag_part : HTTPD_LAST_TAG_PART;
#endif
    } while (MAX_STATIC_MAC_NUM > index);

    MW_FREE(ptr_msg);

#if LWIP_HTTPD_SSI_MULTIPART
    /* last part */
    if(0 == len)
    {
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
    }
    if(HTTPD_LAST_TAG_PART == ptr_tag_param->next_tag_part)
#endif
    {
        len += osapi_snprintf(ptr_tag_insert + len, total_len - len, ";%d", count);
        count = 0;
    }

    ptr_tag_param->tag_insert_len = len;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

C8_T
ssi_get_dynamic_mac_address_entry_xmlHandle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    C8_T err = 0;
    UI16_T len = 0, total_len = 0, i = 0;
    BOOL_T invalid = FALSE;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_db_msg;
    UI16_T db_size;
    void *ptr_db_data = NULL;
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_T *ptr_dynamic_mac_address_entry = NULL;
    C8_T mac_str[13] = {0};
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T dynamic_mac_address_entrycfg = {0};
    UI8_T action = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_DEFAULT;
    ether_addr_t ether_addr;
    /*  expected xml string
     *  8;
     *  3;
     *  1000,001122334455,1,300;
     *  2000,00aabbccddee,8,200;
     *  5,00aabbccddff,5,127;
     */
    UNUSED(action);
    /* Get dynamic mac configuration*/
    mw_rc = httpd_queue_getData(DYNAMIC_MAC_ADDRESS_ENTRY_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES,&ptr_db_msg, &db_size, &ptr_db_data);
    if (MW_E_OK != mw_rc)
    {
        return ERR_INPROGRESS;
    }
    memcpy(&dynamic_mac_address_entrycfg, ptr_db_data, db_size);
    MW_FREE(ptr_db_msg);
    do
    {
        if(((AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_DONE    == dynamic_mac_address_entrycfg.action_result) ||
            (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END      == dynamic_mac_address_entrycfg.action_result) ||
            (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_DONE  == dynamic_mac_address_entrycfg.action_result) ||
            (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_END   == dynamic_mac_address_entrycfg.action_result)) &&
            (0 != dynamic_mac_address_entrycfg.dynamic_entry_count))
        {
            if((TRUE == _cgi_dynamic_mac_address_entry_info.firstreq) &&
               ((AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_DONE != dynamic_mac_address_entrycfg.action_result) &&
                (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END  != dynamic_mac_address_entrycfg.action_result)))
            {
                /* it may be the case the action is GET_START but the result is CONT_DONE or CONT_END */
                invalid = TRUE;
                break;
            }
            if((FALSE == _cgi_dynamic_mac_address_entry_info.firstreq) &&
               ((AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_DONE == dynamic_mac_address_entrycfg.action_result) ||
                (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END  == dynamic_mac_address_entrycfg.action_result)))
            {
                /* it may be the case the action is GET_CONTINUE but the result is START_DONE or START_END */
                invalid = TRUE;
                break;
            }
            if(PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM < dynamic_mac_address_entrycfg.dynamic_entry_count)
            {
                invalid = TRUE;
                break;
            }

            err = send_format_response(&len, ptr_pcb, apiflags,
            "%u;%u;",
            dynamic_mac_address_entrycfg.action_result,
            dynamic_mac_address_entrycfg.dynamic_entry_count);
            if (ERR_OK != err)
            {
                return err;
            }
            total_len += len;
            /* Get dynamic MAC address information */
            mw_rc = httpd_queue_getData(
                DYNAMIC_MAC_ADDRESS_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES,
                &ptr_db_msg, &db_size, (void**)&ptr_dynamic_mac_address_entry);
            if (MW_E_OK != mw_rc)
            {
                return ERR_INPROGRESS;
            }

            _cgi_dynamic_mac_address_entry_info.times--;
            _cgi_dynamic_mac_address_entry_info.firstreq = FALSE;
            /* Update next n dynamic MAC to DB */
            if(((AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END != dynamic_mac_address_entrycfg.action_result) &&
                (AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_END != dynamic_mac_address_entrycfg.action_result)) &&
                (0 < _cgi_dynamic_mac_address_entry_info.times))
            {
                action = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_CONTINUE;
                mw_rc = httpd_queue_setData(M_UPDATE, DYNAMIC_MAC_ADDRESS_ENTRY_CFG, ACTION_RESULT, DB_ALL_ENTRIES, &action, sizeof(UI8_T));
            }

            for (i = 0; i < dynamic_mac_address_entrycfg.dynamic_entry_count; i++)
            {
                memcpy(ether_addr.octet, ptr_dynamic_mac_address_entry->mac_addr[i], ETHER_ADDR_LEN);
                macToStr(&ether_addr, (UI8_T*)mac_str, FALSE);
                /* Send dynamic MAC entry */
                err = send_format_response(&len, ptr_pcb, apiflags,
                                            "%u,%s,0x%x,%u;",
                                            ptr_dynamic_mac_address_entry->vid[i],
                                            mac_str,
                                            ptr_dynamic_mac_address_entry->port[i],
                                            ptr_dynamic_mac_address_entry->age[i]);
                if (ERR_OK != err)
                {
                    continue;
                }
                total_len += len;
                CGI_LOG_DEBUG(MAC, "{i:%u,vids:%u,mac:%s,port:%u,age:%u}",
                        i,
                        ptr_dynamic_mac_address_entry->vid[i],
                        mac_str,
                        ptr_dynamic_mac_address_entry->port[i],
                        ptr_dynamic_mac_address_entry->age[i]);
            }
            /* Release allocated memory before return */
            MW_FREE(ptr_db_msg);
        }
        else
        {
            dynamic_mac_address_entrycfg.dynamic_entry_count = 0;
            err = send_format_response(&len, ptr_pcb, apiflags,
                "%u;%u; ;",
                dynamic_mac_address_entrycfg.action_result,
                dynamic_mac_address_entrycfg.dynamic_entry_count);
            if (ERR_OK != err)
            {
                return err;
            }
            total_len += len;
        }
    }while (0);

    if(TRUE == invalid)
    {
        err = send_format_response(&len, ptr_pcb, apiflags, "%u;%u; ;" ,0 ,0);
        if (ERR_OK != err)
        {
            return err;
        }
        total_len += len;
    }
    *ptr_length = total_len;

    return ERR_OK;
}

