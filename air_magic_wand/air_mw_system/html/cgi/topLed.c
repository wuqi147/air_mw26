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

/* FILE NAME:   topLed.c
 * PURPOSE:
 *      SSI function of port led state web page.
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
#include "port_utils.h"
#include "mw_utils.h"
#ifdef AIR_SUPPORT_POE
#include "poe_config_customer.h"
#include "poe_main.h"
#endif /* AIR_SUPPORT_POE */

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_PORT_MAX_ROW_NUM       (10)
#define MW_PORT_SERDES_OFFT       (100)
#define MW_PORT_BLOCK             (1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    MW_COPPER_STATE_DOWN = 0,
    MW_COPPER_STATE_HIGHSPD,
    MW_COPPER_STATE_HSPDLOOP,
    MW_COPPER_STATE_LOWSPD,
    MW_COPPER_STATE_LSPDLOOP,
    MW_COPPER_STATE_POE,
    MW_COPPER_STATE_BLOCK,
    MW_COPPER_STATE_LAST
}MW_PORT_LED_STATE_T;

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
char
ssi_get_topLed_info_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    C8_T err = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T total_len = 0, i = 0;
    UI16_T len = 0;
    DB_MSG_T *ptr_linkMsg = NULL, *ptr_pmMsg = NULL, *ptr_lpMsg = NULL, *ptr_spdMsg = NULL;
    void *ptr_linkData = NULL, *ptr_pmData = NULL, *ptr_lpData = NULL, *ptr_spdData = NULL;
    UI16_T linkSize = 0, pmSize = 0, lpSize = 0, spdSize = 0;
    char tmpbuf[(3 * MAX_PORT_NUM)] = {0};
    PORT_DB_TYPE_T port_mode = PORT_DB_TYPE_LAST;
    UI8_T state_offt = 0;
#ifdef AIR_SUPPORT_POE
    DB_MSG_T *ptr_poeMsg = NULL;
    void *ptr_poeData = NULL;
    UI16_T poeSize = 0;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
#endif /* AIR_SUPPORT_POE */

    /* Reqeust DB for port link status information */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, &ptr_linkMsg, &linkSize, (void**)&ptr_linkData);
    if(MW_E_OK != rc)
    {
        return err;
    }
    /* Reqeust DB for port speed information */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_SPEED, DB_ALL_ENTRIES, &ptr_spdMsg, &spdSize, (void**)&ptr_spdData);
    if(MW_E_OK != rc)
    {
        osapi_free(ptr_linkMsg);
        return err;
    }
    /* Reqeust DB for port mode information */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, &ptr_pmMsg, &pmSize, (void**)&ptr_pmData);
    if(MW_E_OK != rc)
    {
        osapi_free(ptr_spdMsg);
        osapi_free(ptr_linkMsg);
        return err;
    }
    /* Reqeust DB for loop state information */
    rc = httpd_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, DB_ALL_ENTRIES, &ptr_lpMsg, &lpSize, (void**)&ptr_lpData);
    if(MW_E_OK != rc)
    {
        osapi_free(ptr_spdMsg);
        osapi_free(ptr_linkMsg);
        osapi_free(ptr_pmMsg);
        return err;
    }
#ifdef AIR_SUPPORT_POE
    rc = httpd_queue_getData(POE_PORT_STATUS, PORT_POWER_STATUS, DB_ALL_ENTRIES, &ptr_poeMsg, &poeSize, (void**)&ptr_poeData);
    if(MW_E_OK != rc)
    {
        osapi_free(ptr_spdMsg);
        osapi_free(ptr_linkMsg);
        osapi_free(ptr_pmMsg);
        osapi_free(ptr_lpMsg);
        return err;
    }
#endif /* AIR_SUPPORT_POE */
    len = 0;
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        rc = port_db_getPortType(0, (i + 1), ((UI8_T*)ptr_pmData)[i], &port_mode);
        if(MW_E_OK != rc)
        {
            port_mode = PORT_DB_TYPE_COPPER;
        }

        if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
        {
            state_offt = MW_PORT_SERDES_OFFT;
        }
        else
        {
            state_offt = 0;
        }
#ifdef AIR_SUPPORT_POE
        ptr_poeCfg = poe_config_getPoeCfg();
        if ((NULL != ptr_poeCfg) && (i < ptr_poeCfg->poe_port_number) && (POE_PORT_PWR_STATUS_ON == ((UI8_T *)ptr_poeData)[i]))
        {
            len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (state_offt + MW_COPPER_STATE_POE));
        }
        else
#endif /* AIR_SUPPORT_POE */
        if(MW_COPPER_STATE_DOWN == ((UI8_T*)ptr_linkData)[i])
        {
            len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", state_offt);
        }
        else if(0 != ((UI8_T*)ptr_lpData)[i])
        {
            if(MW_PORT_BLOCK == ((UI8_T*)ptr_lpData)[i])
            {
                len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (state_offt + MW_COPPER_STATE_BLOCK));
            }
            else
            {
                len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (state_offt + MW_PORT_BLOCK + ((((UI8_T*)ptr_spdData)[i] >= AIR_PORT_SPEED_1000M) ? MW_COPPER_STATE_HIGHSPD : MW_COPPER_STATE_LOWSPD)));
            }
        }
        else
        {
            len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (state_offt + ((((UI8_T*)ptr_spdData)[i] >= AIR_PORT_SPEED_1000M) ? MW_COPPER_STATE_HIGHSPD : MW_COPPER_STATE_LOWSPD)));
        }
    }
    total_len += len;

    err = send_format_response(&len, ptr_pcb, apiflags, tmpbuf);
    total_len += len;

    osapi_free(ptr_linkMsg);
    osapi_free(ptr_pmMsg);
    osapi_free(ptr_lpMsg);
    osapi_free(ptr_spdMsg);
#ifdef AIR_SUPPORT_POE
    osapi_free(ptr_poeMsg);
#endif /* AIR_SUPPORT_POE */

    *ptr_length = total_len;

    return err;
}

char
ssi_get_topLed_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    UI32_T total_len = 0;
    UI16_T len = 0;
    C8_T err = 0;
    char up_row[(3 * MAX_PORT_NUM)] = {0};
    char dwn_row[(3 * MAX_PORT_NUM)] = {0};
    char tmpbuf[(3 * MAX_PORT_NUM)] = {0};
#ifdef AIR_SUPPORT_SFP
    BOOL_T isSerdes = FALSE;
#endif
    DB_MSG_T *ptr_msg = NULL;
    void *ptr_data = NULL;
    UI16_T size = 0;
    PORT_DB_TYPE_T port_mode = PORT_DB_TYPE_LAST;
    UI8_T serdes_offt = 0;
    UI16_T j = 0;
    UI16_T idx = 0;
    MW_ERROR_NO_T rc = MW_E_OK;

    CGI_LOG_DEBUG(SYSTEM, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>");
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    rc = mw_get_lights_array((UI8_T *)up_row, (UI8_T *)dwn_row);
    if(MW_E_OK == rc)
    {
        if(0 != osapi_strlen(up_row))
        {
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_up_row = [%s];", up_row);
        }
        else
        {
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_up_row = [];");
        }
        if(ERR_OK != err)
        {
            return err;
        }
        total_len += len;

        if(0 != osapi_strlen(dwn_row))
        {
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_down_row = [%s];", dwn_row);
        }
        else
        {
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_down_row = [];");
        }
        if(ERR_OK != err)
        {
            return err;
        }
        total_len += len;
    }
    else
    {
        rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        if(MW_PORT_MAX_ROW_NUM < PLAT_MAX_PORT_NUM)
        {
            len = 0;
            for(idx = 0; idx < (PLAT_MAX_PORT_NUM / 2); idx++)
            {
                if((0 != idx) && (0 == (idx % 4)))
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                }
                rc = port_db_getPortType(0, ((2 * idx) + 1), ((UI8_T*)ptr_data)[(2 * idx)], &port_mode);
                if(MW_E_OK != rc)
                {
                    port_mode = PORT_DB_TYPE_COPPER;
                }
                if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_COPPER == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
                {
                    if(FALSE == isSerdes)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                        isSerdes = TRUE;
                    }
                    if(PORT_DB_TYPE_SERDES != port_mode)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", ((2 * idx) + 1));
                    }
                    if(0 == serdes_offt)
                    {
                        serdes_offt = (2 * idx) + 1;
                    }
                }
                else
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", ((2 * idx) + 1));
                }
            }
            if(0 != serdes_offt)
            {
                if(PORT_DB_TYPE_SERDES != port_mode)
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                }
                for(j = 0; j <= (PLAT_MAX_PORT_NUM - serdes_offt); j++)
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                }
            }
            if(0 != (PLAT_MAX_PORT_NUM % 2))
            {
                rc = (MW_ERROR_NO_T)port_db_getPortType(0, PLAT_MAX_PORT_NUM, ((UI8_T*)ptr_data)[(PLAT_MAX_PORT_NUM - 1)], &port_mode);
                if(MW_E_OK != rc)
                {
                    port_mode = PORT_DB_TYPE_COPPER;
                }
                if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_COPPER == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
                {
                    if(FALSE == isSerdes)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,0,");
                        isSerdes = TRUE;
                    }
                }
                else
                {
                    if(0 == (((PLAT_MAX_PORT_NUM - 1) % 4)))
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                    }
                }
                len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
            }
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_up_row = [%s];", tmpbuf);
            if(ERR_OK != err)
            {
                osapi_free(ptr_msg);
                return err;
            }
            total_len += len;

            len = 0;
            isSerdes = FALSE;
            osapi_memset(tmpbuf, 0, (3 * MAX_PORT_NUM));
            for(idx = 0; idx < (PLAT_MAX_PORT_NUM / 2); idx++)
            {
                if((0 != idx) && (0 == (idx % 4)))
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                }

                rc = (MW_ERROR_NO_T)port_db_getPortType(0, (2 * (idx + 1)), ((UI8_T*)ptr_data)[((2 * (idx + 1)) - 1)], &port_mode);
                if(MW_E_OK != rc)
                {
                    port_mode = PORT_DB_TYPE_COPPER;
                }
                if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_COPPER == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
                {
                    if(FALSE == isSerdes)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                        isSerdes = TRUE;
                    }
                    if(PORT_DB_TYPE_SERDES != port_mode)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (2 * (idx + 1)));
                    }
                }
                else
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (2 * (idx + 1)));
                }
            }
            if(0 != (PLAT_MAX_PORT_NUM % 2))
            {

                rc = (MW_ERROR_NO_T)port_db_getPortType(0, PLAT_MAX_PORT_NUM, ((UI8_T*)ptr_data)[(PLAT_MAX_PORT_NUM - 1)], &port_mode);
                if(MW_E_OK != rc)
                {
                    port_mode = PORT_DB_TYPE_COPPER;
                }
                if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_COPPER == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
                {
                    if(FALSE == isSerdes)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,0,");
                        isSerdes = TRUE;
                    }
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (PLAT_MAX_PORT_NUM + MW_PORT_SERDES_OFFT));
                }
                else
                {
                    if(0 == (((PLAT_MAX_PORT_NUM - 1) % 4)))
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                    }
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", PLAT_MAX_PORT_NUM);
                }
            }

            if(0 != serdes_offt)
            {
                if(PORT_DB_TYPE_SERDES != port_mode)
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                }
                for(j = 0; j <= (PLAT_MAX_PORT_NUM - serdes_offt); j++)
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (serdes_offt + j + MW_PORT_SERDES_OFFT));
                }
            }

            err = send_format_response(&len, ptr_pcb, apiflags, "var port_down_row = [%s];", tmpbuf);
            if(ERR_OK != err)
            {
                osapi_free(ptr_msg);
                return err;
            }
            total_len += len;
        }
        else
        {
            len = 0;
            err = send_format_response(&len, ptr_pcb, apiflags, "var port_up_row = [];");
            if(ERR_OK != err)
            {
                osapi_free(ptr_msg);
                return err;
            }
            total_len += len;
            len = 0;
            for(idx = 0; idx < PLAT_MAX_PORT_NUM; idx++)
            {
                rc = (MW_ERROR_NO_T)port_db_getPortType(0, (idx + 1), ((UI8_T*)ptr_data)[idx], &port_mode);
                if(MW_E_OK != rc)
                {
                    port_mode = PORT_DB_TYPE_COPPER;
                }
                if(PORT_DB_TYPE_SERDES == port_mode || PORT_DB_TYPE_COMBO_COPPER == port_mode || PORT_DB_TYPE_COMBO_SERDES == port_mode)
                {
                    if(FALSE == isSerdes)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,0,");
                        isSerdes = TRUE;
                    }
                    if(PORT_DB_TYPE_SERDES != port_mode)
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (idx + 1));
                        if(0 == serdes_offt)
                        {
                            serdes_offt = (idx + 1);
                        }
                    }
                    else
                    {
                        len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", ((idx + 1) + MW_PORT_SERDES_OFFT));
                    }
                }
                else
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (idx + 1));
                }
            }

            if(0 != serdes_offt)
            {
                len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "0,");
                for(j = 0; j <= (PLAT_MAX_PORT_NUM - serdes_offt); j++)
                {
                    len += snprintf(tmpbuf + len, (sizeof(tmpbuf) - len), "%d,", (serdes_offt + j + MW_PORT_SERDES_OFFT));
                }
            }

            err = send_format_response(&len, ptr_pcb, apiflags, "var port_down_row = [%s];", tmpbuf);
            if(ERR_OK != err)
            {
                osapi_free(ptr_msg);
                return err;
            }
            total_len += len;
        }
        osapi_free(ptr_msg);
    }
    err = send_format_response(&len, ptr_pcb, apiflags, "</script>");
    if(ERR_OK != err)
    {
        return err;
    }
    total_len += len;
    *ptr_length = total_len;

    CGI_LOG_DEBUG(SYSTEM, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    return ERR_OK;
}
