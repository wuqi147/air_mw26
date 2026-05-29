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

#define  QOS_SCHEDULER_NOSETTING  0xff
#define  QOS_SCHEDULER_WRR        1
#define  QOS_SCHEDULER_WFQ        2


UI32_T g_display_port = 0;
UI8_T  g_display_mode = 0;
/*Set Qos Scheduler , WRR/WFQ weight */
MW_ERROR_NO_T cgi_set_handle_qos_scheduler(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{

    int         i, j;
    UI32_T      temp;
    UI8_T       mode = 0xff;
    UI8_T       weight = 0;
    UI32_T      bm_low = 0;
    UI32_T      pbm = 0;
    int         rc = MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T      trunkBitMap = 0;
#endif

    for(i = 0; i < iNumParams; i++)
    {
        /*qos_policy_mode_set.cgi*/
        if((!strcmp(pcParam[i], "policy_mode")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            mode = (UI8_T)temp;
            g_display_mode = mode;
        }
        else if((!strcmp(pcParam[i], "selBitPort")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            pbm = temp;
        }
        /*qos_queue_weight_set.cgi*/
        if ((!strcmp(pcParam[i], "mode")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            mode = (UI8_T)temp;
            g_display_mode = mode;
        }
        else if ((!strcmp(pcParam[i], "weightval")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            weight = (UI8_T)temp;
        }
        /*for EN8851 only support 8 queue, so didn't parse selBit_64*/
        else if((!strcmp(pcParam[i], "selBit")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            bm_low = temp;
        }
        else if((!strcmp(pcParam[i], "selBitPort")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            pbm = temp;
        }
#if(defined(AIR_SUPPORT_SFP))
        else if ((0 == strcmp(pcParam[i], "trunkBitMap")) && (MW_E_OK == osapi_strtou32(pcValue[i], &temp)))
        {
            trunkBitMap = temp;
        }
#endif
    }

    CGI_LOG_DEBUG(QOS, "%s: %d mode is %hhu  weight is %hhu   bm_low %u pbm %u\n"
        , __func__, __LINE__, mode,  weight, bm_low, pbm);

    if(QOS_SCHEDULER_NOSETTING != mode)
    {
        if(0 != pbm)
        {
#if(defined(AIR_SUPPORT_SFP))
            CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&pbm,0);
#endif
            if(0 == bm_low)
            {
                for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if (0 != (pbm & (1 << i)))
                    {
                        g_display_port = i;
                        rc = httpd_queue_setData(M_UPDATE, PORT_QOS, PORT_QOS_SCHEDULE, (i+1), &mode, sizeof(mode));
                    }
                }
            }
            else
            {
                for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if (0 != (pbm & (1 << i)))
                    {
                        g_display_port = i;
                        for (j = 0; j < MAX_QUEUE_NUM; j++)
                        {
                            if (0 != (bm_low & (1 << j)))
                            {
                                if (QOS_SCHEDULER_WRR == mode)
                                {
                                    rc = httpd_queue_setData(M_UPDATE, PORT_QOS, PORT_QOS_Q0_WEIGHT_WRR + j, (i + 1), &weight, sizeof(weight));
                                }
                                else if (QOS_SCHEDULER_WFQ == mode)
                                {
                                    rc = httpd_queue_setData(M_UPDATE, PORT_QOS, PORT_QOS_Q0_WEIGHT_WFQ + j, (i + 1), &weight, sizeof(weight));
                                }
                                else
                                {
                                    CGI_LOG_ERROR(QOS, "mode %u no support setting weight", mode);
                                    return MW_E_BAD_PARAMETER;
                                }
                            }
                        }
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

char ssi_get_qos_basic_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    int      rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    u16_t    db_size = 0;
    void     *db_data = NULL;

    int i = 0, j = 0;
    unsigned int total_len = 0;
    UI8_T port_qos[PLAT_MAX_PORT_NUM*(PORT_QOS_LAST-1)];

    C8_T *ptr_ssi_str = NULL;
    UI32_T tag_insert_max_len = 0;
    UI16_T tmplen = 0;

    ptr_ssi_str = ptr_tag_param->ptr_tag_insert;
    tag_insert_max_len = ptr_tag_param->tag_insert_max_len;
    if (NULL == ptr_ssi_str)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = httpd_queue_getData(PORT_QOS, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg , &db_size, &db_data);

    if(MW_E_OK == rc)
    {
        if(sizeof(port_qos) != db_size)
        {
            CGI_LOG_ERROR(QOS, "%s: %d get q_weight_val failed, sizeof(port_qos) = %lu, db_size = %d, PLAT_MAX_PORT_NUM = %d\n",__func__, __LINE__, sizeof(port_qos), db_size, PLAT_MAX_PORT_NUM);
            osapi_free(ptr_msg);

            return MW_E_BAD_PARAMETER;
        }
        memcpy(&port_qos, db_data, db_size);
        osapi_free(ptr_msg);
    }
    else
    {
        CGI_LOG_DEBUG(QOS, "get QoS scheduler weight failed \n");
        return ERR_VAL;
    }

    CGI_LOG_DEBUG(QOS, "[%s] line [%d] entered\n", __FUNCTION__, __LINE__);

#if LWIP_HTTPD_SSI_MULTIPART
    if (0 == ptr_tag_param->current_tag_part)
    {
#endif
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "<script>var portNumber = %d;", PLAT_MAX_PORT_NUM);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var queueNumber = %d;", MAX_QUEUE_NUM);
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var display_port = %d;\n", g_display_port);

        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var schedule_all_p = new Array(");
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++){
            tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", port_qos[(PORT_QOS_LAST-2)*PLAT_MAX_PORT_NUM + i]);
        }
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, ");");
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var weight_info_all_p = new Array(");
#if LWIP_HTTPD_SSI_MULTIPART
    }
    else if (1 == ptr_tag_param->current_tag_part)
    {
#endif
        for(j = 0 ; j < (PLAT_MAX_PORT_NUM /2) ; j++)
        {
            tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", "[");
            for (i = 1 ;i < (PORT_QOS_LAST-2); i++)
            {
                tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", port_qos[i*PLAT_MAX_PORT_NUM + j]);
            }
            tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", "],");
        }
#if LWIP_HTTPD_SSI_MULTIPART
    }
    else
    {
#endif
        for(j = (PLAT_MAX_PORT_NUM /2) ; j < PLAT_MAX_PORT_NUM; j++)
        {
            tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", "[");
            for (i = 1 ;i < (PORT_QOS_LAST-2); i++)
            {
                tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%d,", port_qos[i*PLAT_MAX_PORT_NUM + j]);
            }
            tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "%s", "],");
        }

        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, ");");
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "var policyMode = schedule_all_p[display_port];\n");
        tmplen += snprintf(ptr_ssi_str + tmplen, tag_insert_max_len - tmplen, "</script>");
#if LWIP_HTTPD_SSI_MULTIPART
    }
#endif
    total_len = tmplen;

    CGI_LOG_DEBUG(QOS, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);

#if LWIP_HTTPD_SSI_MULTIPART
    if (0 == ptr_tag_param->current_tag_part)
    {
        ptr_tag_param->next_tag_part = 1;
    }
    else if (1 == ptr_tag_param->current_tag_part)
    {
        ptr_tag_param->next_tag_part = 2;
    }
    else
    {
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
    }
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;


    return ERR_OK;
}


