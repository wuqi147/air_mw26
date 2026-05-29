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

/* FILE NAME:   portTrunk.c
 * PURPOSE:
 *      CGI and SSI function of port trunk web page.
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
#include "httpd_queue.h"
#include "web.h"
#include "port_utils.h"
#include "syncd_api_lag.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_TRUNK_SUCCESS    (0)
#define MW_TRUNK_DENIED     (1)
#define MW_TRUNK_UNKNOWN    (2)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static UI8_T _resultCode = MW_TRUNK_SUCCESS;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_lag_process_state_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T total_len = 0, buf_len = 0, len = 0;
    C8_T *ptr_buf = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0;
    UI8_T proc_st = 0;

    ptr_buf = ptr_tag_param->ptr_tag_insert;
    buf_len = ptr_tag_param->tag_insert_max_len;

    err = httpd_queue_getData(LAG_OPER_INFO, LAG_PROCESS_STATE, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        memcpy(&proc_st, (void *)ptr_data, sizeof(UI8_T));
        MW_FREE(ptr_msg);
    }

    len += snprintf(ptr_buf + len, buf_len - len, "%d,%d", proc_st, _resultCode);
    CGI_LOG_INFO(LAG, "lag process state: %d, _resultCode: %d", proc_st, _resultCode);
    _resultCode = MW_TRUNK_SUCCESS;

    total_len += len;
#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    return err;
}

MW_ERROR_NO_T
ssi_get_porttrunk_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T total_len = 0;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    /* Trunk variable */
    UI8_T *ptr_trunk = NULL;
    DB_TRUNK_PORT_T trunk_info;
    UI8_T *ptr_algorithm = NULL;
    UI8_T *ptr_lacp_state = NULL;
    UI8_T lacp_state[MAX_PORT_NUM] = { 0 };
    UI8_T is_aggregated = 0;
    UI32_T member = 0;
    UI16_T size = 0, i = 0, j = 0;
    /* Buffer variable */
    UI32_T tmplen = 0;
    UI32_T str_len = HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN;
    C8_T *ptr_ssi_str = NULL;

    if (MW_E_NO_MEMORY == osapi_calloc(str_len, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        return MW_E_NO_MEMORY;
    }

    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "var trunk_conf = {\n");
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "result:%d,\n", _resultCode);
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "maxTrunkNum:%d,\n", MAX_TRUNK_NUM);
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "portNum:%d,\n", PLAT_MAX_PORT_NUM);
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "maxportNumPerTrunk:%d,\n", MAX_TRUNK_MEMBER_NUM);

#ifdef AIR_SUPPORT_LACP
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "supportLag:%d,\n", 1);
#else
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "supportLag:%d,\n", 0);
#endif
    /* var algorithm */
    err = httpd_queue_getData(TRUNK_ALGORITHM, ALGORITHM, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_algorithm);
    if (MW_E_OK != err)
    {
        osapi_free(ptr_ssi_str);
        CGI_LOG_ERROR(LAG, "httpd_queue_getData(TRUNK_ALGORITHM:ALGORITHM) failed, err = %d", err);
        return err;
    }
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "algo:%d,\n", *ptr_algorithm);
    MW_FREE(ptr_msg);

    err = httpd_queue_getData(PORT_OPER_INFO, PORT_LACP_STATE, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_lacp_state);
    if (MW_E_OK != err)
    {
        osapi_free(ptr_ssi_str);
        CGI_LOG_ERROR(LAG, "httpd_queue_getData(PORT_OPER_INFO:PORT_LACP_STATE) failed, err = %d", err);
        return err;
    }
    memcpy((void *)lacp_state, (void *)ptr_lacp_state, size);
    MW_FREE(ptr_msg);

    /* Request DB for port trunk group member */
    err = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_trunk);
    if (MW_E_OK != err)
    {
        osapi_free(ptr_ssi_str);
        CGI_LOG_ERROR(LAG, "httpd_queue_getData(TRUNK_PORT:TRUNK_MEMBERS) failed, err = %d", err);
        return err;
    }

    /* var pMode = [trunk_conf.mode_g1, trunk_conf.mode_g2, trunk_conf.mode_g3, trunk_conf.mode_g4] */
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "mode_g%d:", (i + 1));
        memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "%d,", trunk_info.members.mode);
    }

    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "portStr_g%d:[", (i + 1));
        for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
        {
            memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            member = trunk_info.members.member_bmp;
            tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "%s,",
                            ((member & (0x01 << j)) ? "1" : "0"));
        }
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "],\n");
    }

    /* var pAggStr = [trunk_conf.portAggStr_g1, trunk_conf.portAggStr_g2, trunk_conf.portAggStr_g3, trunk_conf.portAggStr_g4];*/
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "portAggStr_g%d:[", (i + 1));
        for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
        {
            memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            member = trunk_info.members.member_bmp;
            is_aggregated = (((LAG_MODE_LACP_ACTIVE == trunk_info.members.mode) ||
                              (LAG_MODE_LACP_PASSIVE == trunk_info.members.mode)) &&
                            (member & (0x01 << j)) && (PORT_LACP_STATE_AGGREGATED == lacp_state[j]));
            tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "%s,",
                            (is_aggregated ? "1" : "0"));
        }
        tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "],\n");
    }

    MW_FREE(ptr_msg);
    tmplen += snprintf(ptr_ssi_str + tmplen, str_len - tmplen, "};");
    str_len -= tmplen;
    tmplen += ssi_send_port_mirror_ssi_string(ptr_ssi_str + tmplen, (UI16_T *)&str_len);

    err = send_format_response(&total_len, ptr_pcb, apiflags, "<script>%s</script>", ptr_ssi_str);
    osapi_free(ptr_ssi_str);
    if (MW_E_OK != err)
    {
        return err;
    }

    *ptr_length = total_len;

    return err;
}

MW_ERROR_NO_T
cgi_set_handle_portTrunk(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T group = 0, groupMap = 0;
    UI32_T portMap = 0, i = 0, j = 0;

    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_trunk = NULL;
    UI8_T  mode = LAG_MODE_NONE;
    DB_TRUNK_PORT_T trunk_info;
    UI32_T member = 0;
    UI16_T size = 0;
    UI8_T value = LAG_PROCESS_STATE_PROCESSING;
    UI32_T temp = 0;

    osapi_memset(&trunk_info, 0, sizeof(DB_TRUNK_PORT_T));
    /* Get current trunk group information */
    rc = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_trunk);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "index")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            groupMap = (UI8_T)temp;
        }
        if ((0 == osapi_strcmp(ptr_pcParam[i], "groupId")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            group = (UI8_T)temp;
        }
        if ((0 == osapi_strcmp(ptr_pcParam[i], "portBit")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            portMap = (UI32_T)temp;
        }
        if ((0 == osapi_strcmp(ptr_pcParam[i], "trunkMode")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            mode = (UI8_T)temp;
        }
    }

    CGI_LOG_INFO(LAG, "cgi_set_handle_portTrunk: action=%s group=%d, portMap=0x%x, mode=%d", (0 == groupMap) ? "Create" : "Delete", group, portMap, mode);

    if (0 == groupMap)
    {
        UI8_T *ptr_port_mode = NULL;
        DB_MSG_T *ptr_oper_msg = NULL;
        UI16_T mode_size = 0;
        PORT_DB_TYPE_T portType = PORT_DB_TYPE_LAST;
        PORT_DB_SERDES_MODE_T portSType = PORT_DB_SERDES_MODE_UNKNOWN;
        PORT_DB_TYPE_T chkType = PORT_DB_TYPE_LAST;
        PORT_DB_SERDES_MODE_T chkSType = PORT_DB_SERDES_MODE_UNKNOWN;
        BOOL_T ret = TRUE;

        _resultCode = MW_TRUNK_DENIED;
        rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, &ptr_oper_msg, &mode_size, (void **)&ptr_port_mode);
        if (MW_E_OK != rc)
        {
            MW_FREE(ptr_msg);
            CGI_LOG_ERROR(LAG, "httpd_queue_getData(PORT_OPER_INFO:PORT_OPER_MODE) failed, rc = %d", rc);
            return rc;
        }
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (portMap & (0x01 << i)))
            {
                port_db_getPortType(0, i + 1, ptr_port_mode[i], &chkType);
                port_db_parsePortMode(ptr_port_mode[i], NULL, NULL, NULL, &chkSType);
                CGI_LOG_INFO(LAG, "port %d type:%d, serdes mode:%d", i + 1, chkType, chkSType);
                if (PORT_DB_TYPE_LAST == portType)
                {
                    /* First port */
                    if (PORT_DB_SERDES_MODE_UNKNOWN == chkSType)
                    {
                        ret = FALSE;
                        break;
                    }
                    portType = chkType;
                    portSType = chkSType;
                }
                else
                {
                    /* Other ports */
                    if ((chkType != portType) || (chkSType != portSType) || (PORT_DB_SERDES_MODE_UNKNOWN == chkSType))
                    {
                        ret = FALSE;
                        break;
                    }
                }
            }
        }

        MW_FREE(ptr_oper_msg);
        if (FALSE == ret)
        {
            if (PORT_DB_SERDES_MODE_UNKNOWN == chkSType)
            {
                _resultCode = MW_TRUNK_UNKNOWN;
            }
            MW_FREE(ptr_msg);
            CGI_LOG_DEBUG(LAG, "port type or serdes mode is inconsistent! first port[type=%d, serdes mode=%d], different port[type=%d, serdes mode=%d]",
                            portType, portSType, chkType, chkSType);
            return MW_E_OP_INVALID;
        }

        _resultCode = MW_TRUNK_SUCCESS;
    }

    rc = httpd_queue_setData(M_UPDATE, LAG_OPER_INFO, LAG_PROCESS_STATE, DB_ALL_ENTRIES, &value, sizeof(UI8_T));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(LAG, "httpd_queue_setData(LAG_OPER_INFO:LAG_PROCESS_STATE) failed, rc = %d", rc);
        return rc;
    }

    /* Update port trunk to DB */
    if (0 == groupMap)
    {
        /* Clear old member port trunk id to zero of port configuration information */
        value = 0;
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            memcpy(&trunk_info, (void *)(ptr_trunk + ((group - 1) * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            member = trunk_info.members.member_bmp;

            if (0 != (member & (0x01 << i)))
            {
                rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_TRUNK_ID, (UI16_T)(i + 1), &value, sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_CFG_INFO: PORT_TRUNK_ID) failed, rc = %d", __LINE__, rc);
                    MW_FREE(ptr_msg);
                    return rc;
                }
                rc = httpd_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_LACP_STATE, (UI16_T)(i + 1), &value, sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_OPER_INFO: PORT_LACP_STATE) failed, rc = %d", __LINE__, rc);
                    MW_FREE(ptr_msg);
                    return rc;
                }
            }
        }

        /* Add group id to port configuration information of specific port */
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (portMap & (0x01 << i)))
            {
                rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_TRUNK_ID, (UI16_T)(i + 1), &group, sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_CFG_INFO: PORT_TRUNK_ID) failed, rc = %d", __LINE__, rc);
                    MW_FREE(ptr_msg);
                    return rc;
                }

                value = PORT_LACP_STATE_SELECTED;
                rc = httpd_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_LACP_STATE, (UI16_T)(i + 1), &value, sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_OPER_INFO: PORT_LACP_STATE) failed, rc = %d", __LINE__, rc);
                    MW_FREE(ptr_msg);
                    return rc;
                }
            }
        }

        /* Add new member to trunk group of specific group */
        trunk_info.members.member_bmp = portMap;
        trunk_info.members.mode = mode;
        rc = httpd_queue_setData(M_UPDATE, TRUNK_PORT, TRUNK_MEMBERS, (UI16_T)group, &trunk_info, sizeof(trunk_info));
        if (MW_E_OK != rc)
        {
            MW_FREE(ptr_msg);
            return rc;
        }
    }
    else
    {
        /* Delete port trunk of specific group */
        value = 0;
        portMap = 0;
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if (0 != (groupMap & (0x01 << i)))
            {
                /* Clear member port trunk id to zero of port configuration information */
                for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
                {
                    memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
                    member = trunk_info.members.member_bmp;
                    if (0 != (member & (0x01 << j)))
                    {
                        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_TRUNK_ID, (UI16_T)(j + 1), &value, sizeof(UI8_T));
                        if (MW_E_OK != rc)
                        {
                            CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_CFG_INFO:PORT_TRUNK_ID) failed, rc = %d", __LINE__, rc);
                            MW_FREE(ptr_msg);
                            return rc;
                        }

                        rc = httpd_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_LACP_STATE, (UI16_T)(i + 1), &value, sizeof(UI8_T));
                        if (MW_E_OK != rc)
                        {
                            CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(PORT_OPER_INFO:PORT_LACP_STATE) failed, rc = %d", __LINE__, rc);
                            MW_FREE(ptr_msg);
                            return rc;
                        }
                    }
                }

                /* Clear trunk member of specific group */
                trunk_info.members.member_bmp = portMap;
                trunk_info.members.mode = mode;
                rc = httpd_queue_setData(M_UPDATE, TRUNK_PORT, TRUNK_MEMBERS, (UI16_T)(i + 1), &trunk_info, sizeof(trunk_info));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(TRUNK_PORT:TRUNK_MEMBERS) failed, rc = %d", __LINE__, rc);
                    MW_FREE(ptr_msg);
                    return rc;
                }
            }
        }
    }

    MW_FREE(ptr_msg);

    return rc;
}

MW_ERROR_NO_T
cgi_set_handle_portTrunkAlgo(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T algorithm = 0;
    UI32_T i, temp = 0;

    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "algorithm")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            algorithm = (UI8_T)temp;
        }
    }

    MW_CHECK_MIN_MAX_RANGE(algorithm, LAG_ALGORITHM_MAC_SA_DA, LAG_ALGORITHM_MAC_SA);

    rc = httpd_queue_setData(M_UPDATE, TRUNK_ALGORITHM, ALGORITHM, DB_ALL_ENTRIES, &algorithm, sizeof(UI8_T));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(LAG, "[%d]httpd_queue_setData(TRUNK_ALGORITHM:ALGORITHM) failed, rc = %d", __LINE__, rc);
        return rc;
    }

    return rc;
}
