/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2024
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

/* FILE NAME:   poeSetting.c
 * PURPOSE:
 *      CGI and SSI function of poe setting web page.
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
#include "poe_main.h"
#include "poe_config_customer.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MAX_WORDNUM_STRING_LEN          (7)

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
ssi_get_poe_state_xmlHandle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T total_len = 0;
    UI32_T len = 0, strlen = 0, tmpidx = 0;
    UI32_T tmplen = 0;
    C8_T *ptr_ssi_str = NULL;
    C8_T *ptr_tmpbuf = NULL;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI8_T f_num = 0, data = 0;
    UI16_T e_idx, e_num = 0, size = 0;
    DB_POE_STATUS_T poe_st;
    DB_POE_PORT_STATUS_T poe_port_st;

    if (MW_E_OK != dbapi_getFieldsNum(POE_STATUS, &f_num))
    {
        return err;
    }
    if (MW_E_OK != dbapi_getEntriesNum(POE_STATUS, &e_num))
    {
        return err;
    }
    strlen = (MAX_WORDNUM_STRING_LEN * (f_num - 1) * e_num);
    if (MW_E_OK != dbapi_getFieldsNum(POE_PORT_STATUS, &f_num))
    {
        return err;
    }
    if (MW_E_OK != dbapi_getEntriesNum(POE_PORT_STATUS, &e_num))
    {
        return err;
    }
    strlen += (MAX_WORDNUM_STRING_LEN * (f_num - 1) * e_num);

    if (MW_E_NO_MEMORY == osapi_calloc(strlen, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        return MW_E_NO_MEMORY;
    }

    tmplen = e_num * MAX_WORDNUM_STRING_LEN;
    if (MW_E_NO_MEMORY == osapi_calloc(tmplen, HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf))
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_NO_MEMORY;
    }

    /* DB_POE_STATUS_T */
    memset(&poe_st, 0, sizeof(DB_POE_STATUS_T));
    err = httpd_queue_getData(POE_STATUS, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        memcpy(&poe_st, (void *)ptr_data, sizeof(DB_POE_STATUS_T));
        MW_FREE(ptr_msg);
    }
    len = snprintf(ptr_ssi_str, strlen, "%u,%u,%u;",
                poe_st.supply_voltage,
                poe_st.total_consumed_power,
                poe_st.total_consumed_current);

    /* DB_POE_CFG_T, PORT_POWER_CONTROL */
    err = httpd_queue_getData(POE_PORT_CFG, PORT_POWER_CONTROL, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        tmpidx = 0;
        memset(ptr_tmpbuf, 0, tmplen);
        for (e_idx = 0; e_idx < e_num; e_idx++)
        {
            data = *ptr_data++;
            tmpidx += snprintf(ptr_tmpbuf + tmpidx, tmplen - tmpidx, "%u,", data);
        }
        MW_FREE(ptr_msg);
        len += snprintf(ptr_ssi_str + len, strlen - len, "%s;", ptr_tmpbuf);
    }
    MW_FREE(ptr_tmpbuf);

    /* DB_POE_PORT_STATUS_T */
    for (e_idx = 1; e_idx <= e_num; e_idx++)
    {
        memset(&poe_port_st, 0, sizeof(DB_POE_PORT_STATUS_T));
        err = httpd_queue_getData(POE_PORT_STATUS, DB_ALL_FIELDS, e_idx, &ptr_msg, &size, (void **)&ptr_data);
        if (MW_E_OK == err)
        {
            memcpy(&poe_port_st, (void *)ptr_data, sizeof(DB_POE_PORT_STATUS_T));
            MW_FREE(ptr_msg);
        }
        len += snprintf(ptr_ssi_str + len, strlen - len, "%u,%u,%u,%u,%u,%u;",
                poe_port_st.port_power_status,
                poe_port_st.port_pd_class,
                poe_port_st.port_consumed_power,
                poe_port_st.port_voltage,
                poe_port_st.port_current,
                poe_port_st.port_power_event);
    }
    err = send_format_response(&total_len, ptr_pcb, apiflags, ptr_ssi_str);
    MW_FREE(ptr_ssi_str);
    if (MW_E_OK != err)
    {
        return err;
    }
    *ptr_length = total_len;

    return err;
}

MW_ERROR_NO_T
ssi_get_poe_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T total_len = 0;
    UI32_T len = 0, tmpidx = 0, is_poe_port = 0;
    UI32_T strlen = HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN;
    UI32_T tmplen = 0;
    C8_T *ptr_ssi_str = NULL;
    C8_T* ptr_tmpbuf = NULL;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
    UI16_T max_tot_pwr = 0;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI8_T f_idx, f_num = 0, f_size = 0, data = 0, poe_pwr_strategy = 0;
    UI16_T e_idx, e_num = 0, size = 0, wdata = 0;
    DB_POE_CFG_T poe_cfg;
    /* POE Port variables */
    C8_T param[][16] =
    {
        "pw_ctrl",
        "pw_avail_mode",
        "pw_avail_wat",
        "pw_pri",
        "delay_time",
        "pw_mode"
    };

    max_tot_pwr = poe_getPoeMaxPower();
    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL == ptr_poeCfg)
    {
        return err;
    }

    if (MW_E_OK != dbapi_getFieldsNum(POE_PORT_CFG, &f_num))
    {
        return err;
    }
#ifdef AIR_SUPPORT_POE_WATCHDOG
    f_num -= 1;
#endif
    if (MW_E_OK != dbapi_getEntriesNum(POE_PORT_CFG, &e_num))
    {
        return err;
    }
    if (MW_E_NO_MEMORY == osapi_calloc(strlen, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        return MW_E_NO_MEMORY;
    }
    tmplen = e_num * MAX_WORDNUM_STRING_LEN;
    if (MW_E_NO_MEMORY == osapi_calloc(tmplen, HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf))
    {
        MW_FREE(ptr_ssi_str);
        return MW_E_NO_MEMORY;
    }

    err = httpd_queue_getData(POE_STATUS, IS_POE_PORT, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        is_poe_port = *ptr_data;
        MW_FREE(ptr_msg);
    }

    err = httpd_queue_getData(POE_STATUS, POE_POWER_STRATEGY, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        poe_pwr_strategy = *ptr_data;
        MW_FREE(ptr_msg);
    }

    len = snprintf(ptr_ssi_str, strlen,
        "var poe_cfg={is_poe_port:%d,poe_power_strategy:%d,max_poe_port:%d,max_avail_wat:%d,max_delay_time:%d,max_port_at_wat:%d,max_port_af_wat:%d,",
        is_poe_port, poe_pwr_strategy, ptr_poeCfg->poe_port_number, max_tot_pwr, POE_MAX_POE_DELAY_TIME, POE_MAX_POE_PORT_AT_PWR, POE_MAX_POE_PORT_AF_PWR);
    /* DB_POE_CFG_T */
    memset(&poe_cfg, 0, sizeof(DB_POE_CFG_T));
    err = httpd_queue_getData(POE_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        memcpy(&poe_cfg, (void *)ptr_data, sizeof(DB_POE_CFG_T));
        MW_FREE(ptr_msg);
    }
    if (poe_cfg.total_available_power > max_tot_pwr)
    {
        poe_cfg.total_available_power = max_tot_pwr;
        err = httpd_queue_setData(M_UPDATE, POE_CFG, TOTAL_AVAILABLE_POWER, DB_ALL_ENTRIES, &max_tot_pwr, sizeof(max_tot_pwr));
        if (MW_E_OK != err)
        {
            CGI_LOG_ERROR(POE, "Update DB total available power failed(%d)", err);
        }
    }
    len += snprintf(ptr_ssi_str + len, strlen - len, "avail_wat:%u,\n", poe_cfg.total_available_power);

    /* DB_POE_PORT_CFG_T */
    for(f_idx = PORT_POWER_CONTROL; f_idx < f_num; f_idx++)
    {
        if (MW_E_OK != dbapi_getFieldSize(POE_PORT_CFG, f_idx, &f_size))
        {
            MW_FREE(ptr_tmpbuf);
            MW_FREE(ptr_ssi_str);
            return err;
        }

        err = httpd_queue_getData(POE_PORT_CFG, f_idx, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
        if (MW_E_OK == err)
        {
            tmpidx = 0;
            memset(ptr_tmpbuf, 0, tmplen);
            for (e_idx = 0; e_idx < e_num; e_idx++)
            {
                if (f_size == sizeof(UI8_T))
                {
                    data = *ptr_data++;
                    tmpidx += snprintf(ptr_tmpbuf + tmpidx, tmplen - tmpidx, "%u,", data);
                }
                else
                {
                    wdata = (((*(ptr_data + 1)) << 8) | (*ptr_data));
                    tmpidx += snprintf(ptr_tmpbuf + tmpidx, tmplen - tmpidx, "%u,", wdata);
                    ptr_data += 2;
                }
            }
            MW_FREE(ptr_msg);
            len += snprintf(ptr_ssi_str + len, strlen - len, "%s:[%s],", param[f_idx - PORT_POWER_CONTROL], ptr_tmpbuf);
        }
        else
        {
            break;
        }
    }
    MW_FREE(ptr_tmpbuf);
    len += snprintf(ptr_ssi_str + len, strlen - len, "};");

    err = send_format_response(&total_len, ptr_pcb, apiflags, ptr_ssi_str);
    MW_FREE(ptr_ssi_str);
    if (MW_E_OK != err)
    {
        return err;
    }

    *ptr_length = total_len;

    return err;
}

//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_poeConfig(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T total_avail_power = 0;
    UI8_T i = 0;
    UI32_T temp = 0;

    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "avail_wat")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            total_avail_power = (UI16_T)temp;
        }
        else
        {
            CGI_LOG_ERROR(POE, "Not support this input para %s", ptr_pcParam[i]);
            return MW_E_NOT_SUPPORT;
        }
    }

    CGI_LOG_DEBUG(POE, "total available power = %d", total_avail_power);

    /* Update db */
    rc = httpd_queue_setData(M_UPDATE, POE_CFG, TOTAL_AVAILABLE_POWER, DB_ALL_ENTRIES, &total_avail_power, sizeof(total_avail_power));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(POE, "Update DB POE_CFG failed(%d)", rc);
    }

    return rc;
}

MW_ERROR_NO_T
cgi_set_handle_poePortConfig(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* DB variable */
    UI8_T i = 0;
    UI8_T pwr_ctrl_set = FALSE, power_control = 0;
    UI8_T priority_set = FALSE, priority = 0;
    UI8_T delay_set = FALSE, delay_time = 0;
    UI8_T mode_set = FALSE, af_at_mode = 0;
    UI8_T avail_pwr_mode_set = FALSE, avail_pwr_mode = 0;
    UI16_T ava_pwr_set = FALSE, available_power;
    UI32_T portbmp = 0;
    UI32_T temp = 0;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;

    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL == ptr_poeCfg)
    {
        return rc;
    }
    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "pbmp")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            portbmp = (UI32_T)temp;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "pw_ctrl")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp != POE_PORT_PSE_CTRL_EN) && (temp != POE_PORT_PSE_CTRL_DIS))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            power_control = (UI8_T)temp;
            pwr_ctrl_set = TRUE;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "pw_pri")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp != POE_PORT_PRI_LOW) &&
                (temp != POE_PORT_PRI_HIGH) &&
                (temp != POE_PORT_PRIO_CRITICAL))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            priority = (UI8_T)temp;
            priority_set = TRUE;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "pw_mode")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp != POE_AT_MODE) && (temp != POE_AF_MODE))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            af_at_mode = (UI8_T)temp;
            mode_set = TRUE;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "delay_time")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if (temp > POE_MAX_POE_DELAY_TIME)
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            delay_time = (UI8_T)temp;
            delay_set = TRUE;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "pw_avail_mode")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp != POE_PORT_AVAI_PWR_SELF_DEFINED) &&
                (temp != POE_PORT_AVAI_PWR_DEFAULT))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            avail_pwr_mode = (UI8_T)temp;
            avail_pwr_mode_set = TRUE;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "pw_avail")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp & 0x7FFF) > (POE_APP_DEFAULT_PORT_AVAI_PWR << 4))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], temp);
                continue;
            }
            available_power = (UI16_T)temp;
            ava_pwr_set = TRUE;
        }
        else
        {
            CGI_LOG_ERROR(POE, "Not support this input para %s", ptr_pcParam[i]);
            return rc;
        }
    }

    /* Update db */
    if (0 != portbmp)
    {
        for (i = 0; i < ptr_poeCfg->poe_port_number; i++)
        {
            if (0 != (portbmp & (1 << i)))
            {
                if (TRUE == pwr_ctrl_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_POWER_CONTROL, (i + 1), &power_control, sizeof(power_control));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_POWER_CONTROL failed(%d)", rc);
                    }
                }
                if (TRUE == avail_pwr_mode_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_AVAILABLE_POWER_MODE, (i + 1), &avail_pwr_mode, sizeof(avail_pwr_mode));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_AVAILABLE_POWER_MODE failed(%d)", rc);
                    }
                    if (POE_PORT_AVAI_PWR_DEFAULT == avail_pwr_mode)
                    {
                        available_power = POE_APP_DEFAULT_PORT_AVAI_PWR;
                        rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_AVAILABLE_POWER, (i + 1), &available_power, sizeof(available_power));
                        if (MW_E_OK != rc)
                        {
                            LWIP_DEBUGF(HTTPD_DB_DEBUG, ("Update DB POE_PORT_CFG PORT_AVAILABLE_POWER failed(%d)\n", rc));
                        }
                    }
                }
                if (TRUE == ava_pwr_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_AVAILABLE_POWER, (i + 1), &available_power, sizeof(available_power));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_AVAILABLE_POWER failed(%d)", rc);
                    }
                }
                if (TRUE == priority_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_PRIORITY, (i + 1), &priority, sizeof(priority));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_PRIORITY failed(%d)", rc);
                    }
                }
                if (TRUE == delay_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_DELAY_TIME, (i + 1), &delay_time, sizeof(delay_time));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_DELAY_TIME failed(%d)", rc);
                    }
                }
                if (TRUE == mode_set)
                {
                    rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_AF_AT_MODE, (i + 1), &af_at_mode, sizeof(af_at_mode));
                    if (MW_E_OK != rc)
                    {
                        CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG PORT_AF_AT_MODE failed(%d)", rc);
                    }
                }
            }
        }
    }
    return rc;
}

