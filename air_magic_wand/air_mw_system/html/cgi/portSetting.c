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

/* FILE NAME:   portSetting.c
 * PURPOSE:
 *      CGI and SSI function of port setting web page.
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
#include "mw_log.h"
#include "web.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_task.h"
#include "sfp_pin.h"
#include "sfp_db.h"
#include "sfp_util.h"
#endif /* AIR_SUPPORT_SFP */
#include "air_port.h"
#include "port_utils.h"
#include "osapi_thread.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_PORT_STATE_LNE       (9)
#define MW_PORT_STATE_NUM       (4)
#define MW_SGMII_PORT_START     (24)
#ifdef AIR_SUPPORT_SFP
#define MW_SFP_PORT_STATE       (MAX_PORT_NUM * 6 + 2)
#else
#define MW_SFP_PORT_STATE       (0)
#endif /* AIR_SUPPORT_SFP */

#define MW_STATE_RANGE          (5)
#define MW_STATE_STATUS         (0)
#define MW_STATE_SPEED          (1)
#define MW_STATE_FLOWCTRL       (2)
#define MW_STATE_ABILITY        (3)
#define MW_STATE_NO_CHANGE      (0xFF)

#define MW_PORT_MODE_LEN        (64)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
PORTSETTING_ERR_TYPE_T _portSetting_errType = PORTSETTING_NO_ERR;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
ssi_get_portCur_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    I32_T err = MW_E_BAD_PARAMETER;
    UI32_T total_len = 0, len = 0, buf_len = 0;
    C8_T *ptr_buf = NULL;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T i = 0, num = 0, size = 0;
    UI8_T f_idx = 0;
    UI32_T                unit = 0;
    PORT_DB_TYPE_T        port_type = PORT_DB_TYPE_LAST;
    PORT_DB_SERDES_MODE_T serdes_mode = PORT_DB_SERDES_MODE_UNKNOWN;
    BOOL_T                i2c_flag = FALSE;

    ptr_buf = ptr_tag_param->ptr_tag_insert;
    buf_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_buf)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> tag_buffer is NULL!\n", __func__, __LINE__);
        return MW_E_NO_MEMORY;
    }

    for(i = 0; i < MW_PORT_STATE_NUM; i++)
    {
        f_idx = (PORT_OPER_STATUS + i);
        err = httpd_queue_getData(PORT_OPER_INFO, f_idx, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
        if (MW_E_OK != err)
        {
            CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d", __func__, __LINE__, err);
            return err;
        }
        for(num = 0; num < PLAT_MAX_PORT_NUM; num++)
        {
            if(PORT_OPER_SPEED == f_idx)
            {
                len += snprintf(ptr_buf + len, buf_len - len, "%d,", (ptr_data[num] + 1));
            }
            else
            {
                len += snprintf(ptr_buf + len, buf_len - len, "%d,", ptr_data[num]);
            }
        }
        len += snprintf(ptr_buf + len, buf_len - len, "&");
        MW_FREE(ptr_msg);
    }

    err = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if (MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d", __func__, __LINE__, err);
        return err;
    }

    /* Send port type and serdes mode */
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        i2c_flag = FALSE;
        err = port_db_getPortType(unit, i + 1, ((UI8_T *)ptr_data)[i], &port_type);
        if (MW_E_OK != err)
        {
            CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> parse port type failed, err:%d\n", __func__, __LINE__, err);
            break;
        }
        serdes_mode = PORT_DB_SERDES_MODE_SGMII;
        port_db_parsePortMode(((UI8_T *)ptr_data)[i], NULL, NULL, NULL, &serdes_mode);
        if((PORT_DB_TYPE_SERDES == port_type) || (PORT_DB_TYPE_COMBO_SERDES == port_type))
        {
#ifdef AIR_SUPPORT_SFP
            sfp_port_checkPinInitState(unit, i + 1, (SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED), &i2c_flag);
#endif
        }
        len += snprintf(ptr_buf + len, buf_len - len, "%d-%d-%d,", port_type, serdes_mode, (TRUE == i2c_flag) ? 1 : 0);
    }
    len += snprintf(ptr_buf + len, buf_len - len, "&");
    MW_FREE(ptr_msg);

    if (MW_E_OK == err)
    {
        total_len += len;
#if LWIP_HTTPD_SSI_MULTIPART
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
        ptr_tag_param->tag_insert_len = total_len;
        ptr_tag_param->chunk_enable = TRUE;
    }

    return err;
}

MW_ERROR_NO_T
ssi_get_port_setting_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
#define SSI_PORT_TEMP_STRING_MAX_SIZE (MAX_PORT_NUM * 4)

    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T len = 0;
    UI16_T speedAbility = 0;
    C8_T *ptr_tmpstr = NULL, *ptr_tag_insert = NULL;
    UI16_T portSettings = 0;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL, portMode = 0, flowCtrl = 0;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    const static C8_T param[MW_STATE_RANGE][MW_PORT_STATE_LNE] =
    {
        "state",
        "spd_cfg",
        "fc_cfg",
        "ability"
    };
    UI16_T size = 0, i = 0, num = 0;
    UI32_T tmplen = 0, tag_insert_max_len = 0;

    if ((NULL == ptr_tag_param) || (NULL == ptr_tag_param->ptr_tag_insert) || (0 == ptr_tag_param->tag_insert_max_len))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_tag_insert = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;

    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "<script>");
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "var max_port_num = [%d];\n", PLAT_MAX_PORT_NUM);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "var sgmii_port_begin = [%d];\n", MW_SGMII_PORT_START);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "var all_info = {\n");

    err = osapi_calloc(SSI_PORT_TEMP_STRING_MAX_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_tmpstr);
    if (MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> alloc failed!\n", __func__, __LINE__);
        return err;
    }

    /* Add admin status for all ports. */
    err = httpd_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d\n", __func__, __LINE__, err);
        MW_FREE(ptr_tmpstr);
        return err;
    }
    len = 0;
    for (num = 0; num < PLAT_MAX_PORT_NUM; num++)
    {
        len += snprintf(ptr_tmpstr + len, SSI_PORT_TEMP_STRING_MAX_SIZE - len, "%d,", ptr_data[num]);
    }
    MW_FREE(ptr_msg);
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "%s:[%s],\n", param[i], ptr_tmpstr);

    /* Add port settings for all ports. */
    err = httpd_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d\n", __func__, __LINE__, err);
        MW_FREE(ptr_tmpstr);
        return err;
    }

    for (i = MW_STATE_SPEED; i <= MW_STATE_ABILITY; i++)
    {
        len = 0;
        osapi_memset(ptr_tmpstr, 0, SSI_PORT_TEMP_STRING_MAX_SIZE);
        for (num = 0; num < PLAT_MAX_PORT_NUM; num++)
        {
            osapi_memcpy(&portSettings, (UI8_T *)ptr_data + sizeof(UI16_T) * num, sizeof(UI16_T));
            CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> port:%d port setting:0x%x\n", __func__, __LINE__, (num + 1), portSettings);
            serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
#ifdef AIR_SUPPORT_SFP
            if ((TRUE == sfp_check_ready()) &&
                ((TRUE == sfp_port_is_comboSerdesPort(0, num + 1)) ||
                 (TRUE == sfp_port_is_serdesPort(0, num + 1))))
            {
                err = air_port_getSerdesMode(0, num + 1, &serdes_mode);
            }
#endif
            if (TRUE == port_check25GCopperPort(0, num + 1))
            {
                serdes_mode = AIR_PORT_SERDES_MODE_HSGMII;
            }
            if (MW_E_OK != err)
            {
                CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get port %d serdes mode failed, err:%d\n", __func__, __LINE__, num+1, err);
                MW_FREE(ptr_msg);
                MW_FREE(ptr_tmpstr);
                return (MW_ERROR_NO_T)err;
            }

            if (MW_STATE_SPEED == i)
            {
                port_parsePortSettings(portSettings, &portMode, NULL, NULL);
                port_correctPortMode(0, (num + 1), serdes_mode, &portMode);
                len += snprintf(ptr_tmpstr + len, SSI_PORT_TEMP_STRING_MAX_SIZE - len, "%d,", portMode);
            }
            else if (MW_STATE_FLOWCTRL == i)
            {
                port_parsePortSettings(portSettings, NULL, NULL, &flowCtrl);
                port_correctFlowctrl(0, (num + 1), serdes_mode, &flowCtrl);
                len += snprintf(ptr_tmpstr + len, SSI_PORT_TEMP_STRING_MAX_SIZE - len, "%d,", flowCtrl);
            }
            else if (MW_STATE_ABILITY == i)
            {
                port_parsePortSettings(portSettings, NULL, &speedAbility, NULL);
                port_correctSpeedAbility(0, (num + 1), serdes_mode, &speedAbility);
                len += snprintf(ptr_tmpstr + len, SSI_PORT_TEMP_STRING_MAX_SIZE - len, "%d,", speedAbility);
            }
        }
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "%s:[%s],\n", param[i], ptr_tmpstr);
    }
    MW_FREE(ptr_msg);

    /* Port trunk status */
    err = httpd_queue_getData(PORT_CFG_INFO, PORT_TRUNK_ID, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK == err)
    {
        len = 0;
        osapi_memset(ptr_tmpstr, 0, SSI_PORT_TEMP_STRING_MAX_SIZE);
        for(num = 0; num < PLAT_MAX_PORT_NUM; num++)
        {
            len += snprintf(ptr_tmpstr + len, (MAX_PORT_NUM * 3) - len, "%d,", ptr_data[num]);
        }
        MW_FREE(ptr_msg);
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "trunk_info:[%s],\n", ptr_tmpstr);
    }
    else
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d\n", __func__, __LINE__, err);
        MW_FREE(ptr_tmpstr);
        return err;
    }

    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "};\n");

    /* Reqeust DB for port loop status information */
    err = httpd_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK == err)
    {
        len = 0;
        memset(ptr_tmpstr, 0, (MAX_PORT_NUM * 3));
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            len += snprintf(ptr_tmpstr + len, (MAX_PORT_NUM * 3) - len, "%d,", ptr_data[i]);
        }
        MW_FREE(ptr_msg);
        tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "var portState=[%s];\n", ptr_tmpstr);
    }
    else
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d\n", __func__, __LINE__, err);
        MW_FREE(ptr_tmpstr);
        return err;
    }
    /* return error type */
    tmplen += snprintf(ptr_tag_insert + tmplen, tag_insert_max_len - tmplen, "var errType=%d;\n</script>\n", _portSetting_errType);
    _portSetting_errType = PORTSETTING_NO_ERR;

    MW_FREE(ptr_tmpstr);

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = tmplen;
    ptr_tag_param->chunk_enable = TRUE;

    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_portmode_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T err = MW_E_OK;
    C8_T *ptr_data, *ptr_tag_insert = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T i = 0, size = 0;
    UI32_T len = 0, buf_len = 0, unit = 0;
    UI8_T  port_mode = 0;
    UI8_T *ptr_port_type = NULL;
    UI8_T *ptr_port_serdes = NULL;
    PORT_DB_SERDES_MODE_T serdes_mode = PORT_DB_SERDES_MODE_SGMII;
    PORT_DB_TYPE_T port_type = PORT_DB_TYPE_COPPER;

    ptr_tag_insert = ptr_tag_param->ptr_tag_insert;
    if (NULL == ptr_tag_insert)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> tag_buffer is NULL!\n", __func__, __LINE__);
        return MW_E_NO_MEMORY;
    }

    err = osapi_calloc(MW_PORT_MODE_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_port_type);
    if (MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> alloc failed!\n", __func__, __LINE__);
        return err;
    }

    err = osapi_calloc(MW_PORT_MODE_LEN, HTTPD_QUEUE_CLI, (void **)&ptr_port_serdes);
    if (MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> alloc failed!\n", __func__, __LINE__);
        MW_FREE(ptr_port_type);
        return err;
    }

    err = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if (MW_E_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d", __func__, __LINE__, err);
        MW_FREE(ptr_port_type);
        MW_FREE(ptr_port_serdes);
        return err;
    }

    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        osapi_memcpy(&port_mode, (UI8_T *)ptr_data + sizeof(UI8_T) * i, sizeof(UI8_T));

        port_db_getPortType(unit, i + 1, port_mode, &port_type);
        len += snprintf((C8_T *)ptr_port_type + len, MW_PORT_MODE_LEN - len, "%d,", port_type);

        port_db_parsePortMode(port_mode, NULL, NULL, NULL, &serdes_mode);
        buf_len += snprintf((C8_T *)ptr_port_serdes + buf_len, MW_PORT_MODE_LEN - buf_len, "%d,", serdes_mode);
    }
    MW_FREE(ptr_msg);

    buf_len = ptr_tag_param->tag_insert_max_len;
    len = 0;
    len += snprintf(ptr_tag_insert + len, buf_len - len, "<script>");
    len += snprintf(ptr_tag_insert + len, buf_len - len, "var port_mode = {\n");
    len += snprintf(ptr_tag_insert + len, buf_len - len, "port_type:[%s],\n", ptr_port_type);
    len += snprintf(ptr_tag_insert + len, buf_len - len, "port_serdes:[%s],\n", ptr_port_serdes);
    len += snprintf(ptr_tag_insert + len, buf_len - len, "};\n </script>\n");

#if LWIP_HTTPD_SSI_MULTIPART
    ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
#endif
    ptr_tag_param->tag_insert_len = len;
    ptr_tag_param->chunk_enable = TRUE;

    MW_FREE(ptr_port_type);
    MW_FREE(ptr_port_serdes);

    return MW_E_OK;
}


MW_ERROR_NO_T
cgi_set_handle_portSetting(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T portMap = 0, i = 0;
    UI8_T state_status = 0;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0;
    C8_T loop_state[(MAX_PORT_NUM * 2)] = {0};
#ifdef AIR_SUPPORT_SFP
    UI32_T trunkBitMap = 0;
#endif /* AIR_SUPPORT_SFP */
    UI8_T  forceFc = 0;
    UI8_T  cmdFlag = 0;

    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    UI8_T adminState = 0xFF, portMode = 0xFF, flowCtrl = 0xFF;
    UI16_T speedAbility = 0, portSettings = 0;

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!osapi_strcmp(ptr_pcParam[i], "port_bit"))
        {
            portMap = atoi(ptr_pcValue[i]);
        }
        if(!osapi_strcmp(ptr_pcParam[i], "state"))
        {
            adminState = atoi(ptr_pcValue[i]);
        }
        if(!osapi_strcmp(ptr_pcParam[i], "speed"))
        {
            portMode = atoi(ptr_pcValue[i]);
        }
        if(!osapi_strcmp(ptr_pcParam[i], "flowcontrol"))
        {
            flowCtrl = atoi(ptr_pcValue[i]);
        }
        if(!osapi_strcmp(ptr_pcParam[i], "ability"))
        {
            speedAbility = atoi(ptr_pcValue[i]);
        }
#ifdef AIR_SUPPORT_SFP
        if (0 == osapi_strcmp(ptr_pcParam[i],"trunkBitMap"))
        {
            trunkBitMap = atoi(ptr_pcValue[i]);
        }
#endif /* AIR_SUPPORT_SFP */
        if (0 == osapi_strcmp(ptr_pcParam[i],"cmdFlag"))
        {
            cmdFlag = atoi(ptr_pcValue[i]);
        }
        if (0 == osapi_strcmp(ptr_pcParam[i],"forceFc"))
        {
            forceFc = atoi(ptr_pcValue[i]);
        }
    }

    if ((0 != adminState) && (0xFF == portMode) && (0xFF == flowCtrl) && (0 == speedAbility))
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> portSetting CGI parameter error adminState:0x%x portMode:0x%x flowCtrl:0x%x speedAbility:0x%x!\n",
                      __func__, __LINE__, adminState, portMode, flowCtrl, speedAbility);
        return MW_E_BAD_PARAMETER;
    }

    CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> portMap:0x%x\r\n", __func__, __LINE__, portMap);

#ifdef AIR_SUPPORT_SFP
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&portMap,0);
#endif
    /* Reqeust DB for port loop status information */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK == rc)
    {
        osapi_memcpy(loop_state, ptr_data, PLAT_MAX_PORT_NUM * sizeof(C8_T));
        MW_FREE(ptr_msg);
    }
    else
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> get db failed! rc %d\n", __func__, __LINE__, rc);
        return rc;
    }

    /* Update port setting to DB */
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if ((portMap & (0x01 << i)) && (0 == loop_state[i]))
        {
            if ((0 != adminState) || (TRUE == cmdFlag))
            {
                serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
#ifdef AIR_SUPPORT_SFP
                if ((TRUE == sfp_check_ready()) &&
                     ((TRUE == sfp_port_is_comboSerdesPort(0, i + 1)) ||
                      (TRUE == sfp_port_is_serdesPort(0, i + 1))))
                {
                    rc = air_port_getSerdesMode(0, i + 1, &serdes_mode);
                }
#endif
                if (TRUE == port_check25GCopperPort(0, i + 1))
                {
                    serdes_mode = AIR_PORT_SERDES_MODE_HSGMII;
                }

                port_correctSpeedAbility(0, (i + 1), serdes_mode, &speedAbility);
                port_correctPortMode(0, (i + 1), serdes_mode, &portMode);
                port_correctFlowctrl(0, (i + 1), serdes_mode, &flowCtrl);

                rc = port_genPortSettings(portMode, speedAbility, flowCtrl, &portSettings);
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> gportSetting CGI parameter error!\n", __func__, __LINE__);
                    return rc;
                }
            }

            /* Following SDK note:
             * 1. Disable admin status
             * 2. Configure port based on user settings
             * 3. Enable admin status
             */
            state_status = 0;
            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ADMIN_STATUS, (i + 1), &state_status, sizeof(UI8_T));
            if(MW_E_OK != rc)
            {
                CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> send to db failed! rc %d\n", __func__, __LINE__, rc);
                return rc;
            }
            /* There's no need to configure port if admin status is disabled by user. */
            /* Need to update port_setting to db if it called by cmd */
            if ((0 == adminState) && (FALSE == cmdFlag))
            {
                continue;
            }

            if (TRUE == cmdFlag)
            {
                if (TRUE == forceFc)
                {
                    portSettings |= PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT;
                }
                else
                {
                    portSettings &= ~PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT;
                }
            }

            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_SETTINGS, (i + 1), &portSettings, sizeof(UI16_T));
            CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> update PORT_SETTINGS port:%d rc:%c\r\n", __func__, __LINE__, (i + 1), rc);
            if(MW_E_OK != rc)
            {
                return rc;
            }

            if ((0 == adminState) && (TRUE == cmdFlag))
            {
                continue;
            }

            /* It will link down when change AN mode to Force mode 100M. In such case, wait 50ms before enable admin status again. */
            if ((AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX >= speedAbility) &&
                (PORT_SETTINGS_PORT_MODE_AN != portMode))
            {
                osapi_delay(50);
            }

            state_status = 1;
            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ADMIN_STATUS, (i + 1), &state_status, sizeof(UI8_T));
            if(MW_E_OK != rc)
            {
                CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> send to db failed! rc %d\n", __func__, __LINE__, rc);
                return rc;
            }
        }
    }

    return rc;
}

