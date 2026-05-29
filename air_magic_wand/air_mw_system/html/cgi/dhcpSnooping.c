/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:   dhcpSnooping.c
 * PURPOSE:
 *      CGI and SSI function of DHCP snooping configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <osapi.h>
#include <mw_utils.h>
#include <db_api.h>
#include <httpd_queue.h>
#include <web.h>
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DHCP_SNP_ID_ARRAY_SIZE           256

#define DHCP_SNP_WEB_OPT82_MODE_KEEP     0
#define DHCP_SNP_WEB_OPT82_MODE_REPLACE  1
#define DHCP_SNP_WEB_OPT82_MODE_DROP     2
#define DHCP_SNP_WEB_CIRCUIT_ID_DEFAULT  0
#define DHCP_SNP_WEB_CIRCUIT_USER_ID     1
#define DHCP_SNP_WEB_REMOTE_ID_MAC       0
#define DHCP_SNP_WEB_REMOTE_ID_IP        1
#define DHCP_SNP_WEB_REMOTE_USER_ID      2

#define DHCP_SNP_WEB_OPT82_INFO_MAX_RESULT_SIZE  8

/* MACRO FUNCTION DECLARATIONS
 */
#define dhcp_snoop_http_appendResponse(len, pcb, apiflags, fmt, ...) ({ \
        char send_err = send_format_response((UI16_T *)&(len), (pcb), (apiflags), fmt, __VA_ARGS__); \
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> rc = %d, len = %d, Send string: "fmt"\n", __func__, __LINE__, send_err, len, __VA_ARGS__); \
        if (ERR_OK != send_err) { \
            return send_err; \
        } \
        len; \
    })

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

static UI8_T opt82_res_cnt = 0;

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_dhcpSnooping(
    int  index,
    int  numParams,
    char *ptr_param[],
    char *ptr_value[])
{
    MW_ERROR_NO_T    rc = MW_E_OK;
    UI32_T           i, port, fid_bmp = 0, pfid_bmp = 0, p_bmp = 0;
    UI32_T           data = 0;
    UI8_T            enable = 0, trust_en = 0, opt82_en = 0, opt82_op = 0, circuit_type = 0, remote_type = 0;
    C8_T             *ptr_cirId = NULL, *ptr_rmId = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T           trunkBitMap = 0;
#endif
    /* expected CGI format:
     *   dhcpSnoopEnable.cgi?state=1
     *   dhcpSnoopPortSet.cgi?pbmp=16383&trust=1&option82=1&opt82Mode=1&
                              circuitCustom=1&circuitCustomId=Airoha-cid&
                              remoteCustom=1&remoteCustomId=Airoha-rid
     */
    /* get value from url params */
    CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> handling >>>>\n", __func__, __LINE__);
    for (i = 0; i < numParams; ++i)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, ptr_param[i], ptr_value[i]);

        if ((!strcmp(ptr_param[i], "state")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            if ((data != FALSE) && (data != TRUE))
            {
                continue;
            }
            enable = (UI8_T)data;
            fid_bmp |= BIT(DHCP_SNP_ENABLE);
        }
        else if (!strcmp(ptr_param[i], "pbmp"))
        {
            p_bmp = (UI32_T)atol(ptr_value[i]);
        }
        else if ((!strcmp(ptr_param[i], "trust")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            if ((data != FALSE) && (data != TRUE))
            {
                continue;
            }
            trust_en = (UI8_T)data;
            pfid_bmp |= BIT(DHCP_SNP_TRUST_PORT);
        }
        else if ((!strcmp(ptr_param[i], "option82")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            if ((data != FALSE) && (data != TRUE))
            {
                continue;
            }
            opt82_en = (UI8_T)data;
            pfid_bmp |= BIT(DHCP_SNP_OPT82);
        }
        else if ((!strcmp(ptr_param[i], "opt82Mode")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            switch (data)
            {
            case DHCP_SNP_WEB_OPT82_MODE_KEEP:
                opt82_op = OPT82_MODE_KEEP;
                break;
            case DHCP_SNP_WEB_OPT82_MODE_REPLACE:
                opt82_op = OPT82_MODE_REPLACE;
                break;
            case DHCP_SNP_WEB_OPT82_MODE_DROP:
                opt82_op = OPT82_MODE_DROP;
                break;
            default:
                continue;
            }
            pfid_bmp |= BIT(DHCP_SNP_OPT82_MODE);
        }
        else if ((!strcmp(ptr_param[i], "circuitCustom")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            switch (data)
            {
            case DHCP_SNP_WEB_CIRCUIT_ID_DEFAULT :
                circuit_type = OPT82_CIR_DEFAULT_ID;
                break;
            case DHCP_SNP_WEB_CIRCUIT_USER_ID:
                circuit_type = OPT82_CIR_USER_ID;
                break;
            default:
                continue;
            }
            pfid_bmp |= BIT(DHCP_SNP_OPT82_CIRCUIT_ID_TYPE);
        }
        else if (!strcmp(ptr_param[i], "circuitCustomId"))
        {
            if (ptr_value[i] == NULL)
            {
                continue;
            }
            else if ((osapi_strlen(ptr_value[i]) + 1) > MAX_OPT82_ID_LEN)
            {
                continue;
            }
            ptr_cirId = ptr_value[i];
        }
        else if ((!strcmp(ptr_param[i], "remoteCustom")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            switch (data)
            {
            case DHCP_SNP_WEB_REMOTE_ID_MAC:
                remote_type = OPT82_REM_DEFAULT_ID_MAC;
                break;
            case DHCP_SNP_WEB_REMOTE_ID_IP :
                remote_type = OPT82_REM_DEFAULT_ID_IP;
                break;
            case DHCP_SNP_WEB_REMOTE_USER_ID:
                remote_type = OPT82_REM_USER_ID;
                break;
            default:
                continue;
            }
            pfid_bmp |= BIT(DHCP_SNP_OPT82_REMOTE_ID_TYPE);
        }
        else if (!strcmp(ptr_param[i], "remoteCustomId"))
        {
            if (ptr_value[i] == NULL)
            {
                continue;
            }
            else if ((osapi_strlen(ptr_value[i]) + 1) > MAX_OPT82_ID_LEN)
            {
                continue;
            }
            ptr_rmId = ptr_value[i];
        }
#if(defined(AIR_SUPPORT_SFP))
        else if ((0 == strcmp(ptr_param[i], "trunkBitMap")) && (MW_E_OK == osapi_strtou32(ptr_value[i], &data)))
        {
            trunkBitMap = data;
        }
#endif
    }
    CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> <<<<< handled\n", __func__, __LINE__);
    if (fid_bmp & BIT(DHCP_SNP_ENABLE))
    {
        rc = httpd_queue_setData(M_UPDATE, DHCP_SNP_INFO, DHCP_SNP_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(UI8_T));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }
    if (p_bmp != 0)
    {
#if(defined(AIR_SUPPORT_SFP))
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap, &p_bmp, 0);
#endif
        p_bmp <<= 1;
        for (port = 1; port <= PLAT_MAX_PORT_NUM; ++port)
        {
            if (p_bmp & BIT(port))
            {
                DB_MSG_T           *ptr_msg = NULL;
                UI16_T             data_size = 0;
                UI8_T              *ptr_data = NULL;
                DB_DHCP_SNP_PORT_INFO_T entry = {};

                CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> >> set port %d\n", __func__, __LINE__, port);
                rc = httpd_queue_getData(DHCP_SNP_PORT_INFO, DB_ALL_FIELDS, port, &ptr_msg, &data_size, (void **)&ptr_data);
                MW_PARAM_CHK(rc != MW_E_OK, rc);
                entry = *(DB_DHCP_SNP_PORT_INFO_T *)ptr_data;
                MW_FREE(ptr_msg);

                if (pfid_bmp & BIT(DHCP_SNP_TRUST_PORT))
                {
                    entry.trust_port = trust_en;
                }
                if (pfid_bmp & BIT(DHCP_SNP_OPT82))
                {
                    entry.opt82_enable = opt82_en;
                }
                if (pfid_bmp & BIT(DHCP_SNP_OPT82_MODE))
                {
                    entry.opt82_mode = opt82_op;
                }
                if (pfid_bmp & BIT(DHCP_SNP_OPT82_CIRCUIT_ID_TYPE))
                {
                    if (circuit_type == OPT82_CIR_USER_ID)
                    {
                        if (ptr_cirId != NULL)
                        {
                            entry.circuit_id_type = circuit_type;
                            osapi_strncpy((C8_T *)(entry.circuit_id), ptr_cirId, sizeof(OPT82_ID_T));
                        }
                        else
                        {
                            /* Not to change current configuration if set user config type but no string. */
                            return MW_E_OK;
                        }
                    }
                    else
                    {
                        entry.circuit_id_type = circuit_type;
                        osapi_memset(entry.circuit_id, 0, sizeof(OPT82_ID_T));
                    }
                }

                if (pfid_bmp & BIT(DHCP_SNP_OPT82_REMOTE_ID_TYPE))
                {
                    if (remote_type == OPT82_REM_USER_ID)
                    {
                        if (ptr_rmId != NULL)
                        {
                            entry.remote_id_type = remote_type;
                            osapi_strncpy((C8_T *)(entry.remote_id), ptr_rmId, sizeof(OPT82_ID_T));
                        }
                        else
                        {
                            /* Not to change current configuration if set user config type but no string. */
                            return MW_E_OK;
                        }
                    }
                    else
                    {
                        entry.remote_id_type = remote_type;
                        osapi_memset(entry.remote_id, 0, sizeof(OPT82_ID_T));
                    }
                }
                CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> >> trust(%d), en(%d), mode(%d), cid(%d:%s), rid(%d:%s)\n", __func__, __LINE__,
                      entry.trust_port,
                      entry.opt82_enable,
                      entry.opt82_mode,
                      entry.circuit_id_type,
                      entry.circuit_id,
                      entry.remote_id_type,
                      entry.remote_id);
                rc = httpd_queue_setData(M_UPDATE, DHCP_SNP_PORT_INFO, DB_ALL_FIELDS, port, &entry, sizeof(entry));
                MW_PARAM_CHK(rc != MW_E_OK, rc);
            }
        }
    }
    return MW_E_OK;
}

//==========================================================================================
//  SSI handler
//==========================================================================================
char
ssi_get_dhcp_snooping_info_Handle(
    int              *ptr_length,
    struct tcp_pcb   *ptr_pcb,
    unsigned int     apiflags)
{
    UI16_T             len = 0;
    UI32_T             total_len = 0, p = 0;
    DB_MSG_T           *ptr_msg = NULL;
    UI8_T              *ptr_data = NULL, data = 0;
    UI16_T             data_size = 0;
    C8_T               port_array[MW_PORT_NUM * 2 + 1] = {0};
    UI32_T             offset = 0;

    /*  expected javascript string
     *  var dhcp_ds = {
     *      portNum: 28,
     *      state: 0,
     *      trust: [1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
     *      option82: [1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
     *      opt82Mode: [1,1,1,0,2,0,0,2,2,2,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
     *      circuitCustom: [1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
     *      circuitCustomId: [],
     *      remoteCustom: [1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
     *      remoteCustomId: [],
     *      trunkInfo: [1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
     *  };
     */

    /* Field: portNum, state, trust */
    if (httpd_queue_getData(DHCP_SNP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    data = *ptr_data;
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "var dhcp_ds={portNum:%d,state:%d,", PLAT_MAX_PORT_NUM, (data != 0) ? 1 : 0);

    /* Field: trust port */
    if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_TRUST_PORT, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    for (p = 1, offset = 0; p <= PLAT_MAX_PORT_NUM; ++p)
    {
        offset += osapi_snprintf(port_array + offset, sizeof(port_array) - offset, "%s%d", ((p == 1) ? "" : ","), ((ptr_data[p - 1]) ? 1 : 0));
    }
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "trust:[%s],", port_array);

    /* Field: option82 */
    if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    for (p = 1, offset = 0; p <= PLAT_MAX_PORT_NUM; ++p)
    {
        offset += osapi_snprintf(port_array + offset, sizeof(port_array) - offset, "%s%d", ((p == 1) ? "" : ","), ((ptr_data[p - 1]) ? 1 : 0));
    }
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "option82:[%s],", port_array);

    /* Field: opt82Mode */
    if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_MODE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    for (p = 1, offset = 0; p <= PLAT_MAX_PORT_NUM; ++p)
    {
        offset += osapi_snprintf(port_array + offset, sizeof(port_array) - offset, "%s%d", ((p == 1) ? "" : ","), ptr_data[p - 1]);
    }
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "opt82Mode:[%s],", port_array);

    /* Field: circuitCustom */
    if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID_TYPE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    for (p = 1, offset = 0; p <= PLAT_MAX_PORT_NUM; ++p)
    {
        offset += osapi_snprintf(port_array + offset, sizeof(port_array) - offset, "%s%d", ((p == 1) ? "" : ","),
                                 (ptr_data[p - 1] == OPT82_CIR_USER_ID) ? DHCP_SNP_WEB_CIRCUIT_USER_ID : ptr_data[p - 1]);
    }
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "circuitCustom:[%s],", port_array);

    /* Field: circuitCustom */
    if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID_TYPE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    for (p = 1, offset = 0; p <= PLAT_MAX_PORT_NUM; ++p)
    {
        offset += osapi_snprintf(port_array + offset, sizeof(port_array) - offset, "%s%d", ((p == 1) ? "" : ","),
                                 (ptr_data[p - 1] == OPT82_REM_USER_ID) ? DHCP_SNP_WEB_REMOTE_USER_ID : ptr_data[p - 1]);
    }
    MW_FREE(ptr_msg);
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "remoteCustom:[%s],circuitCustomId:[],remoteCustomId:[]};", port_array);

    *ptr_length = total_len;
    /* init opt82 result counter */
    opt82_res_cnt = 1;
    return ERR_OK;
}

char
ssi_get_dhcp_snooping_opt82_info_Handle(
    int              *ptr_length,
    struct tcp_pcb   *ptr_pcb,
    unsigned int     apiflags)
{
    UI16_T             len = 0;
    UI32_T             total_len = 0, p = 0;
    DB_MSG_T           *ptr_msg = NULL;
    C8_T               *ptr_data = NULL;
    UI16_T             data_size = 0;
    C8_T               id_tmp[DHCP_SNP_ID_ARRAY_SIZE] = {0};
    UI32_T             offset = 0;

    CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> opt82_res_cnt = %d\n", __func__, __LINE__, opt82_res_cnt);

    /*  expected option 82 id string (8 port config in every response)
     *
     *  circuit-id-port1,circuit-id-port2,circuit-id-port3,circuit-id-port4,
     *  circuit-id-port5,circuit-id-port6,circuit-id-port7,circuit-id-port8;
     *  remote-id-port1,remote-id-port2,remote-id-port3,remote-id-port4,
     *  remote-id-port5,remote-id-port6,remote-id-port7,remote-id-port8;
     */

    if ((opt82_res_cnt == 0) || (opt82_res_cnt > PLAT_MAX_PORT_NUM))
    {
        *ptr_length = 0;
        return ERR_OK;
    }
    /* Field: circuitCustomId */
    for (p = opt82_res_cnt; (p <= PLAT_MAX_PORT_NUM) && (p < (opt82_res_cnt + DHCP_SNP_WEB_OPT82_INFO_MAX_RESULT_SIZE)); ++p)
    {
        if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID, p, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
        {
            CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
            return ERR_INPROGRESS;
        }
        offset += osapi_snprintf(id_tmp + offset, sizeof(id_tmp) - offset, "%s%s", ((p == opt82_res_cnt) ? "" : ","), ptr_data);
        MW_FREE(ptr_msg);
        /* Split response if data size is more than buffer size */
        if ((offset + MAX_OPT82_ID_LEN) > DHCP_SNP_ID_ARRAY_SIZE)
        {
            total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "%s", id_tmp);
            offset = 0;
            osapi_memset(id_tmp, 0, DHCP_SNP_ID_ARRAY_SIZE);
        }
    }
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "%s;", id_tmp);
    offset = 0;
    osapi_memset(id_tmp, 0, DHCP_SNP_ID_ARRAY_SIZE);

    /* Field: remoteCustomId */
    for (p = opt82_res_cnt; (p <= PLAT_MAX_PORT_NUM) && (p < (opt82_res_cnt + DHCP_SNP_WEB_OPT82_INFO_MAX_RESULT_SIZE)); ++p)
    {
        if (httpd_queue_getData(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID, p, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
        {
            CGI_LOG_DEBUG(DHCPSNP, "<%s:%d> Error out\n", __func__, __LINE__);
            return ERR_INPROGRESS;
        }
        offset += osapi_snprintf(id_tmp + offset, sizeof(id_tmp) - offset, "%s%s", ((p == opt82_res_cnt) ? "" : ","), ptr_data);
        MW_FREE(ptr_msg);
        /* Split response if data size is more than buffer size */
        if ((offset + MAX_OPT82_ID_LEN) > DHCP_SNP_ID_ARRAY_SIZE)
        {
            total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "%s", id_tmp);
            offset = 0;
            osapi_memset(id_tmp, 0, DHCP_SNP_ID_ARRAY_SIZE);
        }
    }
    total_len += dhcp_snoop_http_appendResponse(len, ptr_pcb, apiflags, "%s;", id_tmp);
    *ptr_length = total_len;

    opt82_res_cnt += DHCP_SNP_WEB_OPT82_INFO_MAX_RESULT_SIZE;
    return ERR_OK;
}

