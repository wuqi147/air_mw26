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

/* FILE NAME:   LldpConfigRpm.c
 * PURPOSE:
 *      CGI and SSI function of lldp config web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DB_LLDP_INFO_T lldp_cfg = {0, 0, 0, 0, 0};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T cgi_set_handle_lldpconfig(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[])
{
    UI8_T           i = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI8_T           lldp_mode = 0;
    UI8_T           tx_hold = 0;
    UI16_T          tx_interval = 0;
    UI16_T          reinit_delay = 0;
    UI16_T          tx_delay = 0;
    UI8_T           isLldpModeUpdate = 0, isTxHoldUpdate = 0, isTxIntervalUpdate = 0, isReinitDelayUpdate = 0, isTxDelayUpdate = 0;
    UI32_T          temp = 0;

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if((!strcmp(ptr_pcParam[i], "state")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            lldp_mode = (UI8_T)temp;
            isLldpModeUpdate = 1;
        }
        if((!strcmp(ptr_pcParam[i], "tx_hold")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            tx_hold = (UI8_T)temp;
            isTxHoldUpdate = 1;
        }
        if((!strcmp(ptr_pcParam[i], "tx_interval")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            tx_interval = (UI16_T)temp;
            isTxIntervalUpdate = 1;
        }
        if((!strcmp(ptr_pcParam[i], "reinit_delay")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            reinit_delay = (UI16_T)temp;
            isReinitDelayUpdate = 1;
        }
        if((!strcmp(ptr_pcParam[i], "tx_delay")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            tx_delay = (UI16_T)temp;
            isTxDelayUpdate = 1;
        }
    }

    if(isLldpModeUpdate)
    {
        CGI_LOG_DEBUG(LLDP, "[%s] lldp_mode = %d\n", __FUNCTION__, lldp_mode);
        rc = httpd_queue_setData(M_UPDATE, LLDP_INFO, GLOBAL_ENABLE, DB_ALL_ENTRIES, &lldp_mode, sizeof(lldp_mode));
    }

    if(isTxHoldUpdate)
    {
        CGI_LOG_DEBUG(LLDP, "[%s] tx_hold = %d\n", __FUNCTION__, tx_hold);
        rc = httpd_queue_setData(M_UPDATE, LLDP_INFO, TX_HOLD, DB_ALL_ENTRIES, &tx_hold, sizeof(tx_hold));
    }

    if(isTxIntervalUpdate)
    {

        CGI_LOG_DEBUG(LLDP, "[%s] tx_interval = %d\n", __FUNCTION__, tx_interval);
        rc = httpd_queue_setData(M_UPDATE, LLDP_INFO, TX_INTERVAL, DB_ALL_ENTRIES, &tx_interval, sizeof(tx_interval));
    }

    if(isReinitDelayUpdate)
    {
        CGI_LOG_DEBUG(LLDP, "[%s] reinit_delay = %d\n", __FUNCTION__, reinit_delay);
        rc = httpd_queue_setData(M_UPDATE, LLDP_INFO, REINIT_DELAY, DB_ALL_ENTRIES, &reinit_delay, sizeof(reinit_delay));
    }

    if(isTxDelayUpdate)
    {
        CGI_LOG_DEBUG(LLDP, "[%s] tx_delay = %d\n", __FUNCTION__, tx_delay);
        rc = httpd_queue_setData(M_UPDATE, LLDP_INFO, TX_DELAY, DB_ALL_ENTRIES, &tx_delay, sizeof(tx_delay));
    }

    return rc;
}

MW_ERROR_NO_T cgi_set_handle_lldpportconfig(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[])
{
    UI8_T           i = 0;
    UI8_T           isPortAdminUpdate = 0;
    UI8_T           port_status[MAX_PORT_NUM] = {0};
    UI32_T          port_admin1 = 0;
    UI32_T          port_admin2 = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          temp = 0;

    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if((!strcmp(ptr_pcParam[i], "port_admin1")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            port_admin1 = temp;
            isPortAdminUpdate = 1;
            CGI_LOG_DEBUG(LLDP, "[%s] port_admin1 [%d]\n", __FUNCTION__, port_admin1);
        }
        if((!strcmp(ptr_pcParam[i], "port_admin2")) && (MW_E_OK == osapi_strtou32(ptr_pcValue[i], &temp)))
        {
            port_admin2 = temp;
            isPortAdminUpdate = 1;
            CGI_LOG_DEBUG(LLDP, "[%s] port_admin2 [%d]\n", __FUNCTION__, port_admin2);
        }
    }
    if(isPortAdminUpdate)
    {
        for(i = 0; i < PLAT_MAX_PORT_NUM && i < MAX_PORT_NUM; i++)
        {
            if(i < 14)
            {
                port_status[i] = (port_admin1 & (0x3 << (i * 2))) >> (i * 2);
            }
            else
            {
                port_status[i] = (port_admin2 & (0x3 << ((i - 14) * 2))) >> ((i - 14) * 2);
            }
        }
        rc = httpd_queue_setData(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, port_status, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    }
    return rc;
}

char ssi_get_lldp_config_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    C8_T err = 0;
    UI32_T len = 0, total_len = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;
    UI8_T lldp_mode = 0;
    UI8_T tx_hold = 0;
    UI16_T tx_interval = 0;
    UI16_T reinit_delay = 0;
    UI16_T tx_delay = 0;
    UI8_T i = 0;
    //UI8_T *ptr_data = NULL;
    C8_T tmpbuf[MAX_PORT_NUM * 2] = {0};

    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "<script>");
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "var portNumber = %d;\n", PLAT_MAX_PORT_NUM);
    if(err != ERR_OK)
    {
        return err;
    }

    total_len += len;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "var lldp_cfg = {");
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get lldp global cfg info*/
    rc = httpd_queue_getData(LLDP_INFO, GLOBAL_ENABLE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp global enable success, ptr_msg =%p\n", db_msg);
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp global enable failed \n");
        return ERR_VAL;
    }
    memcpy(&lldp_mode, db_data, db_size);
    osapi_free(db_msg);
    lldp_cfg.enable = lldp_mode;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "state : %d,", lldp_cfg.enable);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get lldp tx_hold info*/
    rc = httpd_queue_getData(LLDP_INFO, TX_HOLD, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx hold success, ptr_msg =%p\n", db_msg);
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx hold failed \n");
        return ERR_VAL;
    }
    memcpy(&tx_hold, db_data, db_size);
    osapi_free(db_msg);
    lldp_cfg.tx_hold_multipler = tx_hold;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "tx_hold : %d,", lldp_cfg.tx_hold_multipler);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get lldp tx_interval info*/
    rc = httpd_queue_getData(LLDP_INFO, TX_INTERVAL, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx interval success, ptr_msg =%p\n", db_msg);
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx interval failed \n");
        return ERR_VAL;
    }
    memcpy(&tx_interval, db_data, db_size);
    osapi_free(db_msg);
    lldp_cfg.tx_interval = tx_interval;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "tx_interval : %d,", lldp_cfg.tx_interval);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get lldp reinit delay info */
    rc = httpd_queue_getData(LLDP_INFO, REINIT_DELAY, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp reinit delay success, ptr_msg =%p\n", db_msg);
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp reinit delay failed \n");
        return ERR_VAL;
    }
    memcpy(&reinit_delay, db_data, db_size);
    osapi_free(db_msg);
    lldp_cfg.reinit_delay = reinit_delay;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "reinit_delay : %d,", lldp_cfg.reinit_delay);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get lldp tx delay info*/
    rc = httpd_queue_getData(LLDP_INFO, TX_DELAY, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx delay success, ptr_msg =%p\n", db_msg);
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp tx delay failed \n");
        return ERR_VAL;
    }
    memcpy(&tx_delay, db_data, db_size);
    osapi_free(db_msg);
    lldp_cfg.tx_delay = tx_delay;
    err = send_format_response((UI16_T*)&len, pcb, apiflags, "tx_delay : %d};\n", lldp_cfg.tx_delay);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;
    /*get port lldp status*/
    rc = httpd_queue_getData(LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(LLDP, "get lldp port state success, ptr_msg =%p\n", db_msg);
        len = 0;
        memset(tmpbuf, 0, MAX_PORT_NUM * 2);
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(i == (PLAT_MAX_PORT_NUM - 1))
            {
                len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%d", ((UI8_T *)db_data)[i]);
            }
            else
            {
                len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%d,", ((UI8_T *)db_data)[i]);
            }
        }
        osapi_free(db_msg);
        err = send_format_response((UI16_T*)&len, pcb, apiflags, "var portState = [%s];\n</script>\n", tmpbuf);
        if(MW_E_OK != err)
        {
            return err;
        }
        total_len += len;
    }
    else
    {
        CGI_LOG_DEBUG(LLDP, "get lldp port state failed \n");
        return ERR_VAL;
    }
    *length = total_len;

    CGI_LOG_DEBUG(LLDP, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);
    return ERR_OK;
}

