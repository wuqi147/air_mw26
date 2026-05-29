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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "web.h"
#include "db_api.h"
#include "osapi_memory.h"
#include "httpd_queue.h"
#include "mw_log.h"

#define  QOS_PRI_NOSETTING  0xff
/*Set Qos Trust Mode , Port pri, DSCP pri and pri & Queue mapping */
MW_ERROR_NO_T cgi_set_handle_qos_priority(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{

    int         i;
    UI32_T      temp = 0;
    UI8_T       mode = QOS_PRI_NOSETTING;
    UI8_T       queue_val = QOS_PRI_NOSETTING;
    UI8_T       dscp_pri = QOS_PRI_NOSETTING, port_pri = QOS_PRI_NOSETTING;
    PRI_Q_T     pri_queue_bm;
    DSCP_Q_T    dscp_pri_bm;
    UI32_T      bm_low = 0;
    UI32_T      bm_high = 0;
    int         rc = MW_E_OK;
    DB_MSG_T    *ptr_msg = NULL;
    UI16_T      db_size = 0;
    void        *ptr_data = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T      trunkBitMap = 0;
#endif

    for(i = 0; i < iNumParams; i++)
    {
/*qos_base_mode_set.cgi*/
        if((!strcmp(pcParam[i], "qosbasemode")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            mode = (UI8_T)temp;
        }
/*qos_advance_set.cgi*/
        if((!strcmp(pcParam[i], "queueval")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            queue_val = (UI8_T)temp;
        }
        else if ((!strcmp(pcParam[i], "weigthval")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            dscp_pri = (UI8_T)temp;
        }
        else if ((!strcmp(pcParam[i], "portval")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            port_pri = (UI8_T)temp;
        }
        else if((!strcmp(pcParam[i], "selBit")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            bm_low = temp;
        }
        else if((!strcmp(pcParam[i], "selBit_64")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            bm_high = temp;
        }
#if(defined(AIR_SUPPORT_SFP))
        else if ((0 == strcmp(pcParam[i], "trunkBitMap")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            trunkBitMap = temp;
        }
#endif
    }
    CGI_LOG_DEBUG(QOS, "%s: %d mode is %hhu queue_val is %hhu dscp_pri is %hhu port_pri is %hhu bm_low %u bm_high %u\n"
        , __func__, __LINE__, mode, queue_val, dscp_pri, port_pri,bm_low, bm_high);

    if(QOS_PRI_NOSETTING != mode)
    {
        rc = httpd_queue_setData(M_UPDATE, QOS_INFO, QOS_MODE, DB_ALL_ENTRIES, &mode, sizeof(mode));
    }
    else
    {
        if (QOS_PRI_NOSETTING != queue_val)
        {
            if(0 != bm_low)
            {
                /*for glb value, need get all array data, change & send*/
                rc = httpd_queue_getData(QOS_INFO, QOS_PRI_TO_QUEUE, DB_ALL_ENTRIES, &ptr_msg,  &db_size , &ptr_data);
                memset(&pri_queue_bm, 0 , sizeof(PRI_Q_T));
                if(sizeof(pri_queue_bm) != db_size)
                {
                    osapi_free(ptr_msg);
                    return MW_E_BAD_PARAMETER;
                }
                memcpy(&pri_queue_bm, ptr_data, db_size);
                osapi_free(ptr_msg);
                for (i = 0; i < MAX_QUEUE_NUM; i++)
                {
                    if (0 != (bm_low & (1 << i)))
                    {
                        pri_queue_bm[i] = queue_val;
                    }
                }
                rc = httpd_queue_setData(M_UPDATE, QOS_INFO, QOS_PRI_TO_QUEUE, DB_ALL_ENTRIES, &pri_queue_bm, sizeof(PRI_Q_T));
            }
        }
        else if (QOS_PRI_NOSETTING != dscp_pri)
        {
            if((0 != bm_low) || (0 != bm_high))
            {
                /*for glb value, need get all array data, change & send*/
                rc = httpd_queue_getData(QOS_INFO, QOS_DSCP_TO_PRI, DB_ALL_ENTRIES, &ptr_msg,  &db_size , &ptr_data);
                memset(&dscp_pri_bm, 0 , sizeof(DSCP_Q_T));
                if(sizeof(dscp_pri_bm) != db_size)
                {
                    osapi_free(ptr_msg);
                    return MW_E_BAD_PARAMETER;
                }
                memcpy(&dscp_pri_bm, ptr_data, db_size);
                osapi_free(ptr_msg);

                for (i = 0; i < MAX_DSCP_NUM/2; i++)
                {
                    if (0 != (bm_low & (1 << i)))
                    {
                        dscp_pri_bm[i] = dscp_pri;
                    }
                }

                for (i = 0; i < MAX_DSCP_NUM/2; i++)
                {
                    if (0 != (bm_high & (1 << i)))
                    {
                        dscp_pri_bm[i + MAX_DSCP_NUM/2] = dscp_pri;
                    }
                }
                rc = httpd_queue_setData(M_UPDATE, QOS_INFO, QOS_DSCP_TO_PRI, DB_ALL_ENTRIES, &dscp_pri_bm, sizeof(DSCP_Q_T));
            }
        }
        else if (QOS_PRI_NOSETTING != port_pri)
        {
#if(defined(AIR_SUPPORT_SFP))
            CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&bm_low,0);
#endif
            if(0 != bm_low)
            {
                for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if (0 != (bm_low & (1 << i)))
                    {
                        rc = httpd_queue_setData(M_UPDATE, PORT_QOS, PORT_QOS_PRIORITY, (i + 1), &port_pri, sizeof(UI8_T));
                    }
                }
            }
        }
        else
        {
            return MW_E_OP_INVALID;
        }
    }
    return rc;
}

char ssi_get_qos_advance_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags)
{
    int      rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    u16_t    db_size = 0;
    void     *db_data = NULL;

    UI8_T    qos_trustMode = 0;
    UI8_T    *ptr_portPriority = NULL;
    DSCP_Q_T dscpPriority;
    PRI_Q_T  priToQueue;

    int i = 0;
    unsigned int len = 0, total_len = 0;
    char err = 0;
    char *ptr_buf_PriPort = NULL;
    char *ptr_buf_Pri8021p = NULL;
    char *ptr_buf_PriQueue = NULL;
    char *ptr_buf_PriDscp = NULL;

    rc = osapi_calloc(MAX_PORT_NUM, HTTPD_QUEUE_CLI, (void **)&ptr_portPriority);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    rc = osapi_calloc(MAX_PORT_NUM * 2, HTTPD_QUEUE_CLI, (void **)&ptr_buf_PriPort);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_portPriority);
        return rc;
    }

    rc = osapi_calloc(MAX_8021P_NUM * 2, HTTPD_QUEUE_CLI, (void **)&ptr_buf_Pri8021p);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        return rc;
    }
    rc = osapi_calloc(MAX_QUEUE_NUM * 2, HTTPD_QUEUE_CLI, (void **)&ptr_buf_PriQueue);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        return rc;
    }
    rc = osapi_calloc(MAX_DSCP_NUM * 2, HTTPD_QUEUE_CLI, (void **)&ptr_buf_PriDscp);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        return rc;
    }

        CGI_LOG_DEBUG(QOS, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    /*QoS trustmode setting*/
    rc = httpd_queue_getData(QOS_INFO, QOS_MODE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        if(sizeof(qos_trustMode) != db_size)
        {
            CGI_LOG_ERROR(QOS, "%s: %d get qos_trustMode failed\n",__func__, __LINE__);
            osapi_free(ptr_msg);
            MW_FREE(ptr_portPriority);
            MW_FREE(ptr_buf_PriPort);
            MW_FREE(ptr_buf_Pri8021p);
            MW_FREE(ptr_buf_PriQueue);
            MW_FREE(ptr_buf_PriDscp);
            return MW_E_BAD_PARAMETER;
        }
        memcpy(&qos_trustMode, db_data, db_size);
        osapi_free(ptr_msg);
    }
    else
    {
        CGI_LOG_DEBUG(QOS, "get QoS trust mode failed \n");
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        MW_FREE(ptr_buf_PriDscp);
        return ERR_VAL;
    }
    /* QoS priority setting*/
    /* port pri*/
    rc = httpd_queue_getData(PORT_QOS, PORT_QOS_PRIORITY, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        if((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != db_size)
        {
            CGI_LOG_ERROR(QOS, "%s: %d get portPriority failed\n",__func__, __LINE__);
            osapi_free(ptr_msg);
            MW_FREE(ptr_portPriority);
            MW_FREE(ptr_buf_PriPort);
            MW_FREE(ptr_buf_Pri8021p);
            MW_FREE(ptr_buf_PriQueue);
            MW_FREE(ptr_buf_PriDscp);
            return MW_E_BAD_PARAMETER;
        }
        memset(ptr_portPriority, 0, MAX_PORT_NUM);
        memcpy(ptr_portPriority, db_data, db_size);
        osapi_free(ptr_msg);
    }
    else
    {
        CGI_LOG_DEBUG(QOS, "get QoS port priority failed \n");
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        MW_FREE(ptr_buf_PriDscp);
        return ERR_VAL;
    }
    /*dscp pri*/
    rc = httpd_queue_getData(QOS_INFO, QOS_DSCP_TO_PRI, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        if(sizeof(dscpPriority) != db_size)
        {
            CGI_LOG_ERROR(QOS, "%s: %d get dscpPriority failed\n",__func__, __LINE__);
            osapi_free(ptr_msg);
            MW_FREE(ptr_portPriority);
            MW_FREE(ptr_buf_PriPort);
            MW_FREE(ptr_buf_Pri8021p);
            MW_FREE(ptr_buf_PriQueue);
            MW_FREE(ptr_buf_PriDscp);
            return MW_E_BAD_PARAMETER;
        }
        memset(&dscpPriority, 0, sizeof(dscpPriority));
        memcpy(&dscpPriority, db_data, db_size);
        osapi_free(ptr_msg);
    }
    else
    {
        CGI_LOG_DEBUG(QOS, "get QoS dscp priority failed \n");
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        MW_FREE(ptr_buf_PriDscp);
        return ERR_VAL;
    }
    /*pri to queue*/
    rc = httpd_queue_getData(QOS_INFO, QOS_PRI_TO_QUEUE, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        if(sizeof(priToQueue) != db_size)
        {
            CGI_LOG_ERROR(QOS, "%s: %d get priToQueue failed\n",__func__, __LINE__);
            osapi_free(ptr_msg);
            MW_FREE(ptr_portPriority);
            MW_FREE(ptr_buf_PriPort);
            MW_FREE(ptr_buf_Pri8021p);
            MW_FREE(ptr_buf_PriQueue);
            MW_FREE(ptr_buf_PriDscp);
            return MW_E_BAD_PARAMETER;
        }
        memset(&priToQueue, 0, sizeof(priToQueue));
        memcpy(&priToQueue, db_data, db_size);
        osapi_free(ptr_msg);
    }
    else
    {
        CGI_LOG_DEBUG(QOS, "get QoS pri and queue mapping failed \n");
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        MW_FREE(ptr_buf_PriDscp);
        return ERR_VAL;
    }

    CGI_LOG_DEBUG(QOS, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(ptr_buf_PriPort+len, PLAT_MAX_PORT_NUM * 2 - len, "%u,", ptr_portPriority[i]);
    }

    len = 0;
    for(i = 0; i < MAX_8021P_NUM; i++)
    {
        len += snprintf(ptr_buf_Pri8021p+len, (MAX_8021P_NUM * 2)-len, "%u,", 0);
    }

    len = 0;
    for(i = 0; i < MAX_QUEUE_NUM; i++)
    {
        len += snprintf(ptr_buf_PriQueue+len, (MAX_QUEUE_NUM * 2)-len, "%u,", priToQueue[i]);
    }

    len = 0;
    for(i = 0; i < MAX_DSCP_NUM; i++)
    {
        len += snprintf(ptr_buf_PriDscp+len, (MAX_DSCP_NUM * 2)-len, "%u,", dscpPriority[i]);
    }

    err = send_format_response((UI16_T*)&total_len, pcb, apiflags, "<script> var portNumber = %d; var p8021Number = %d; var dscpNumber = %d; var priorityNumber = %d;\
 var qosMode = %u;var PriPort = new Array(%s);var Pri8021p = new Array(%s);var PriQueue = new Array(%s);var PriDscp = new Array(%s);</script>",
                                                    PLAT_MAX_PORT_NUM, MAX_8021P_NUM, MAX_DSCP_NUM, MAX_PRIORITY_NUM,
                                                    qos_trustMode, ptr_buf_PriPort, ptr_buf_Pri8021p, ptr_buf_PriQueue, ptr_buf_PriDscp);
    if(ERR_OK != err)
    {
        MW_FREE(ptr_portPriority);
        MW_FREE(ptr_buf_PriPort);
        MW_FREE(ptr_buf_Pri8021p);
        MW_FREE(ptr_buf_PriQueue);
        MW_FREE(ptr_buf_PriDscp);
        return err;
    }
    *length = total_len;

    CGI_LOG_DEBUG(QOS, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

    MW_FREE(ptr_portPriority);
    MW_FREE(ptr_buf_PriPort);
    MW_FREE(ptr_buf_Pri8021p);
    MW_FREE(ptr_buf_PriQueue);
    MW_FREE(ptr_buf_PriDscp);
    return ERR_OK;
}
