/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2025
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

/* FILE NAME:   poeWatchDog.c
 * PURPOSE:
 *      CGI and SSI function of poe WatchDog web page.
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
#include "poe_config_customer.h"
#include "poe_main.h"
#include "osapi_thread.h"

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
ssi_get_poe_watchdog_Handle(
    I32_T          *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T          apiflags)
{
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
    MW_ERROR_NO_T                err = MW_E_OK;
    UI16_T                       total_len = 0;
    UI32_T                       len = 0;
    UI32_T                       strlen = HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN;
    UI32_T                       tmplen = 0;
    C8_T                        *ptr_ssi_str = NULL;
    C8_T                        *ptr_tmpbuf = NULL;
    /* DB variable */
    DB_MSG_T                    *ptr_msg = NULL;
    UI8_T                       *ptr_data = NULL;
    UI8_T                        f_num = 0, f_size = 0, data = 0;
    UI16_T                       e_idx = 0, e_num = 0, size = 0, wdata = 0;
    DB_POE_CFG_T                 poe_cfg = {0};
    DB_POE_STATUS_T              poe_st = {0};

    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL == ptr_poeCfg)
    {
        return err;
    }

    if (MW_E_OK != dbapi_getFieldsNum(POE_PORT_CFG, &f_num) ||
        MW_E_OK != dbapi_getEntriesNum(POE_PORT_CFG, &e_num))
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
    osapi_memset(&poe_cfg, 0, sizeof(DB_POE_CFG_T));
    osapi_memset(&poe_st, 0, sizeof(DB_POE_STATUS_T));
    err = httpd_queue_getData(POE_STATUS, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        memcpy(&poe_st, (void *)ptr_data, sizeof(DB_POE_STATUS_T));
        MW_FREE(ptr_msg);
    }
    err = httpd_queue_getData(POE_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK == err)
    {
        memcpy(&poe_cfg, (void *)ptr_data, sizeof(DB_POE_CFG_T));
        MW_FREE(ptr_msg);
    }

    len += snprintf(ptr_ssi_str, strlen, "var poe_wdg={is_poe_port:%d,max_poe_port:%d,", poe_st.is_poe_port, ptr_poeCfg->poe_port_number);

    /* DB_POE_PORT_CFG_T */
    if (MW_E_OK == dbapi_getFieldSize(POE_PORT_CFG, PORT_POE_WATCHDOG_ENABLE, &f_size))
    {
        err = httpd_queue_getData(POE_PORT_CFG, PORT_POE_WATCHDOG_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_data);
        if (MW_E_OK == err)
        {
            osapi_memset(ptr_tmpbuf, 0, tmplen);
            UI32_T tmpidx = 0;
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
            len += snprintf(ptr_ssi_str + len, strlen - len, "wd_mode:[%s],", ptr_tmpbuf);
        }
    }
    MW_FREE(ptr_tmpbuf);
    len += snprintf(ptr_ssi_str + len, strlen - len,
        "watchdog_period:%u, watchdog_threshold:%u};",
        poe_cfg.poe_watchdog_period, poe_cfg.poe_watchdog_threshold);

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
cgi_set_handle_poeWatchDogConfig(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T        i = 0;
    UI16_T       wd_period = 0, wd_rx_packet_diff = 0;
    UI32_T       temp = 0;

    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "wd_period")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            wd_period = (UI16_T)temp;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "wd_rx_packet_diff")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            wd_rx_packet_diff = (UI16_T)temp;
        }
        else
        {
            CGI_LOG_ERROR(POE, "Not support this input para %s", ptr_pcParam[i]);
            return MW_E_NOT_SUPPORT;
        }
    }

    CGI_LOG_DEBUG(POE, "WatchDog period = %d", wd_period);
    CGI_LOG_DEBUG(POE, "WatchDog receive packet difference = %d", wd_rx_packet_diff);

    /* Update db */
    rc = httpd_queue_setData(M_UPDATE, POE_CFG, POE_WATCHDOG_PERIOD, DB_ALL_ENTRIES, &wd_period, sizeof(wd_period));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(POE, "Update DB POE_CFG failed(%d)", rc);
    }
    rc = httpd_queue_setData(M_UPDATE, POE_CFG, POE_WATCHDOG_THRESHOLD, DB_ALL_ENTRIES, &wd_rx_packet_diff, sizeof(wd_rx_packet_diff));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(POE, "Update DB POE_CFG failed(%d)", rc);
    }

    return rc;
}
MW_ERROR_NO_T
cgi_set_handle_poeWatchDog(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[])
{
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;
    /* DB variable */
    UI8_T         wd_mode = 0;
    UI16_T        i = 0;
    UI32_T        portbmp = 0, temp = 0;

    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL == ptr_poeCfg)
    {
        return rc;
    }
    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if ((0 == osapi_strcmp(ptr_pcParam[i], "selBit")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            portbmp = (UI32_T)temp;
        }
        else if ((0 == osapi_strcmp(ptr_pcParam[i], "state")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            if ((temp != POE_PORT_WATCHDOG_EN) && (temp != POE_PORT_WATCHDOG_DIS))
            {
                CGI_LOG_ERROR(POE, "the %s is out of range: %u", ptr_pcParam[i], wd_mode);
                return MW_E_BAD_PARAMETER;
            }
            wd_mode = (UI8_T)temp;
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
            if (1 == ((portbmp >> i) & 0x1))
            {
                rc = httpd_queue_setData(M_UPDATE, POE_PORT_CFG, PORT_POE_WATCHDOG_ENABLE, (i + 1), &wd_mode, sizeof(wd_mode));
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(POE, "Update DB POE_PORT_CFG %u PORT_POE_WATCHDOG_ENABLE failed(%d)", i, rc);
                }
            }
        }
    }
    return rc;
}
