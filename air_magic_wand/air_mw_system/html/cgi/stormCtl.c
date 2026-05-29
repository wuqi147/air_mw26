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
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include <osal/osal_lib.h>
#include "httpd_queue.h"
#if(defined(AIR_SUPPORT_SFP))
#include "sfp_task.h"
#endif

/* MACRO FUNCTION DECLARATIONS
 */
/*for web ratelimit range 0-1000000, so set array width to 8*/
#define STORMCTRL_CFG_NOSETTING          0xff
#define STORMCTRL_RATE_NOSETTING         0xffffffff
#define PORT_TYPE_LIST_STRING_LEN        (64)
#define SFP_MODE_LIST_STRING_LEN         (128)
#define STORMCTRL_MAX_BUF_SIZE            (1024)
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
/*Set Stormctl BC/UC/MC mode & rate */
MW_ERROR_NO_T
cgi_set_handle_stormCtrl(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    UI8_T  i = 0;
    UI8_T        ucmode = STORMCTRL_CFG_NOSETTING, mcmode = STORMCTRL_CFG_NOSETTING, bcmode = STORMCTRL_CFG_NOSETTING;
    UI8_T        ucstate = STORMCTRL_CFG_NOSETTING, mcstate = STORMCTRL_CFG_NOSETTING, bcstate = STORMCTRL_CFG_NOSETTING;
    UI32_T       ucspeed = STORMCTRL_RATE_NOSETTING, mcspeed = STORMCTRL_RATE_NOSETTING, bcspeed = STORMCTRL_RATE_NOSETTING;
    UI32_T       pbm = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T       trunkBitMap = 0;
#endif
    int rc = MW_E_OK;

/*storm_ctrl_set.cgi*/
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "selMask"))
        {
            pbm = atoi(pcValue[i]);
            if(0 == pbm)
            {
                break;
            }
        }
        else if (!strcmp(pcParam[i], "UCstate"))
        {
            ucstate = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "UCspeed"))
        {
            ucspeed = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "UCmode"))
        {
            ucmode = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "MCstate"))
        {
            mcstate = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "MCspeed"))
        {
            mcspeed = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "MCmode"))
        {
            mcmode = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "BCstate"))
        {
            bcstate = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "BCspeed"))
        {
            bcspeed = atoi(pcValue[i]);
        }
        else if (!strcmp(pcParam[i], "BCmode"))
        {
            bcmode = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        else if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "Not support this input para %s\n", pcParam[i]);
            continue;
        }
    }

    if(0 != pbm)
    {
#if(defined(AIR_SUPPORT_SFP))
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&pbm,0);
#endif
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(0 != (pbm & (1 << i)))
            {
                if(STORMCTRL_CFG_NOSETTING != ucstate)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_UC_CFG, (i + 1), &ucstate, sizeof(ucstate));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set uc cfg failed\n");
                    }
                }
                if(STORMCTRL_CFG_NOSETTING != mcstate)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_MC_CFG, (i + 1), &mcstate, sizeof(mcstate));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set mc cfg failed\n");
                    }
                }
                if(STORMCTRL_CFG_NOSETTING != bcstate)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_BC_CFG, (i + 1), &bcstate, sizeof(bcstate));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set bc cfg failed\n");
                    }
                }
                if(STORMCTRL_CFG_NOSETTING != bcmode)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_BC_MODE, (i + 1), &bcmode, sizeof(bcmode));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set bc mode failed\n");
                    }
                }
                if(STORMCTRL_CFG_NOSETTING != ucmode)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_UC_MODE, (i + 1), &ucmode, sizeof(ucmode));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set uc mode failed\n");
                    }
                }
                if(STORMCTRL_CFG_NOSETTING != mcmode)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_MC_MODE, (i + 1), &mcmode, sizeof(mcmode));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set mc mode failed\n");
                    }
                }
                if(STORMCTRL_RATE_NOSETTING != bcspeed)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_BC_RATE, (i + 1), &bcspeed, sizeof(bcspeed));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set bc spd failed\n");
                    }
                }
                if(STORMCTRL_RATE_NOSETTING != ucspeed)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_UC_RATE, (i + 1), &ucspeed, sizeof(ucspeed));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set uc spd failed\n");
                    }
                }
                if(STORMCTRL_RATE_NOSETTING != mcspeed)
                {
                    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_STORM_MC_RATE, (i + 1), &mcspeed, sizeof(mcspeed));
                    if(MW_E_OK != rc)
                    {
                        CGI_LOG_DEBUG(STORM_CTRL, "Set mc spd failed\n");
                    }
                }
            }
        }

    }
    else
    {
        return MW_E_OP_INVALID;
    }

    return rc;

}

char
ssi_get_storm_ctrl_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    I32_T      i = 0;
    I32_T      rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    u16_t    db_size = 0;
    void     *db_data = NULL;
    UI16_T len = 0, total_len = 0;

    UI32_T *ptr_rate_tmp = NULL;
    UI8_T  *ptr_cfg_tmp = NULL;

    C8_T err = ERR_OK;
    C8_T *ptr_buf_state = NULL;
    I32_T len_state = 0;

    rc = osapi_calloc(MAX_PORT_NUM * sizeof(UI32_T), HTTPD_QUEUE_CLI, (void **)&ptr_rate_tmp);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    rc = osapi_calloc(MAX_PORT_NUM, HTTPD_QUEUE_CLI, (void **)&ptr_cfg_tmp);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_rate_tmp);
        return rc;
    }

    do
    {
        CGI_LOG_DEBUG(STORM_CTRL, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);
        osapi_calloc(STORMCTRL_MAX_BUF_SIZE,__func__, (void **)&ptr_buf_state);
        if (NULL == ptr_buf_state)
        {
            err = ERR_MEM;
            break;
        }
        len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "<script>\nvar portNumber = %d;\n", PLAT_MAX_PORT_NUM);
        /*cfg setting*/
        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_BC_CFG, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "var sc_info={BCstate:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_UC_CFG, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "UCstate:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_MC_CFG, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "MCstate:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        /*mode setting*/
        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_BC_MODE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "BCmode:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_MC_MODE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "MCmode:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_UC_MODE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d cfg size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_cfg_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "UCmode:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_cfg_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }
        err = send_format_response(&len, ptr_pcb, apiflags, "%s", ptr_buf_state);
        if(ERR_OK != err)
        {
            break;
        }
        total_len += len;
        /*rate value*/
        len_state = 0;
        memset(ptr_buf_state, 0, STORMCTRL_MAX_BUF_SIZE);
        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_BC_RATE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI32_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d rate size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_rate_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "BCspeed:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_rate_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_MC_RATE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI32_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d rate size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_rate_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "MCspeed:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_rate_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }

        rc = httpd_queue_getData(PORT_CFG_INFO, PORT_STORM_UC_RATE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
        if(MW_E_OK == rc)
        {
            if((sizeof(UI32_T) * PLAT_MAX_PORT_NUM) != db_size)
            {
                CGI_LOG_DEBUG(STORM_CTRL, "%s: %d rate size error \n",__func__, __LINE__);
                osapi_free(ptr_msg);
                err = MW_E_BAD_PARAMETER;
                break;
            }
            memcpy(ptr_rate_tmp, db_data, db_size);
            osapi_free(ptr_msg);
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "UCspeed:[");
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "%u,", ptr_rate_tmp[i]);
            }
            len_state += snprintf( (ptr_buf_state + len_state), (STORMCTRL_MAX_BUF_SIZE - len_state), "],}\n");
        }
        else
        {
            CGI_LOG_DEBUG(STORM_CTRL, "get Stormctl value failed \n");
            err = ERR_VAL;
            break;
        }
        err = send_format_response(&len, ptr_pcb, apiflags, "%s", ptr_buf_state);
        if(ERR_OK != err)
        {
            break;
        }
        total_len += len;
        *length = total_len;

        CGI_LOG_DEBUG(STORM_CTRL, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);
    } while (0);

    if (MW_E_OK == err)
    {
        err = send_format_response(&len, ptr_pcb, apiflags, "</script>");
    }

    osapi_free(ptr_buf_state);
    MW_FREE(ptr_rate_tmp);
    MW_FREE(ptr_cfg_tmp);
    return err;
}


