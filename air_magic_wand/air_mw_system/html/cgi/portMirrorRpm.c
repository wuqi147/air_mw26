/*  Copyright Statement:
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

/* FILE NAME:   portMirrorRpm.c
 * PURPOSE:
 *      CGI and SSI function of port mirror web page.
 * NOTES:
*/

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "httpd_queue.h"
#include "mw_log.h"
#include "web.h"
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SSI_PORT_MIRROR_STRING_LENGTH (64)

/* There are at most 28 ports. Therefore, port bitmap with 32 bit1s is invalid. */
#define CGI_PORT_MIRROR_INVALID_PORT_BITMAP (0xFFFFFFFF)

#define CGI_PORT_MIRROR_DESTINATION_PORT_INFO_OFFSET   (4)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* Session state */
typedef enum
{
    CGI_PORT_MIRROR_SESSION_STATE_DISABLE,         /* Session is disabled. */
    CGI_PORT_MIRROR_SESSION_STATE_ENABLE,        /* Session is enabled. */

    CGI_PORT_MIRROR_SESSION_STATE_INVALIDE = 0xFF /* Session state is invalid. */
} CGI_PORT_MIRROR_SESSION_STATE_ENUM;

/* The port mirror information table */
typedef struct ONE_DB_PORT_MIRROR_INFO_S
{
    UI8_T           enable;      /* The admin status of the port mirror session */
    UI8_T           dest_port;   /* The destination port of the mirror session */
    UI32_T          src_in_port; /* The ingress traffic of the source port */
    UI32_T          src_eg_port; /* The egress traffic of the source port */
} ATTRIBUTE_PACK ONE_DB_PORT_MIRROR_INFO_T;
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

/* mirror_port_set.cgi */
MW_ERROR_NO_T
cgi_set_handle_mirrorPortSet(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    CGI_PORT_MIRROR_SESSION_STATE_ENUM session_state[MAX_MIRROR_SESS_NUM];
    UI32_T old_dst_port_bitmap[MAX_MIRROR_SESS_NUM], dst_port_bitmap[MAX_MIRROR_SESS_NUM];
    UI32_T src_in_port = 0, src_eg_port = 0;
    C8_T *ptr_num = NULL;
    UI8_T session_id = 0xFF, i, num = 0, session_bitmap = 0, rc = MW_E_BAD_PARAMETER, dst_port = 0;
    ONE_DB_PORT_MIRROR_INFO_T *ptr_session_mirror_info;
    DB_MSG_T *ptr_db_msg1 = NULL, *ptr_db_msg2 = NULL;
    UI16_T db_size = 0;
    UI8_T *ptr_port_mirror_info = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;
    UI32_T temp = 0;

    memset(session_state, CGI_PORT_MIRROR_SESSION_STATE_INVALIDE, MAX_MIRROR_SESS_NUM);
    /* Initialize port bitmap to be invalid. */
    memset(old_dst_port_bitmap, 0xFF, MAX_MIRROR_SESS_NUM * sizeof(UI32_T));
    memset(dst_port_bitmap, 0xFF, MAX_MIRROR_SESS_NUM * sizeof(UI32_T));

    for(i = 0; i < iNumParams; i++)
    {
        CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> i:%d, pcParam:%s pcValue:%s\n", __func__, __LINE__,
                      i, pcParam[i], pcValue[i]);
        if(!strncmp(pcParam[i], "state", 5)) /* ex. state1, state2,... */
        {
            ptr_num = pcParam[i];
            ret = osapi_strtou32(ptr_num + 5, &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            num = (UI8_T)temp; /* E.g., convert "state1" to 1 */
            if(num <= MAX_MIRROR_SESS_NUM && num > 0)
            {
                ret = osapi_strtou32(pcValue[i], &temp);
                if (MW_E_OK != ret)
                {
                    break;
                }
                session_state[num - 1] = temp ? CGI_PORT_MIRROR_SESSION_STATE_ENABLE : CGI_PORT_MIRROR_SESSION_STATE_DISABLE;
            }
        }
        if(!strncmp(pcParam[i], "oldDstPort", 10)) /* ex. oldDstPort1, oldDstPort2,... */
        {
            ptr_num = pcParam[i];
            ret = osapi_strtou32(ptr_num + 10, &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            num = (UI8_T)temp;
            if(num <= MAX_MIRROR_SESS_NUM && num > 0)
            {
                ret = osapi_strtou32(pcValue[i], &temp);
                if (MW_E_OK != ret)
                {
                    break;
                }
                old_dst_port_bitmap[num - 1] = temp;
            }
        }
        if(!strncmp(pcParam[i], "dstPort", 7)) /* ex. dstPort1, dstPort2,... */
        {
            ptr_num = pcParam[i];
            ret = osapi_strtou32(ptr_num + 7, &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            num = (UI8_T)temp;
            if(num <= MAX_MIRROR_SESS_NUM && num > 0)
            {
                ret = osapi_strtou32(pcValue[i], &temp);
                if (MW_E_OK != ret)
                {
                    break;
                }
                dst_port_bitmap[num - 1] = temp;
            }
        }
        if(!strcmp(pcParam[i], "session"))
        {
            ret = osapi_strtou32(pcValue[i], &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            session_id = (UI8_T)temp; /* ex. 0, 1, 2, 3 */
        }
        if(!strcmp(pcParam[i], "ingressState")) /* bitN represents port (N+1) */
        {
            ret = osapi_strtou32(pcValue[i], &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            src_in_port = temp;
        }
        if(!strcmp(pcParam[i], "egressState")) /* bitN represents port (N+1) */
        {
            ret = osapi_strtou32(pcValue[i], &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            src_eg_port = temp;
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            ret = osapi_strtou32(pcValue[i], &temp);
            if (MW_E_OK != ret)
            {
                break;
            }
            trunkBitMap = temp;
        }
#endif
    }

    if (MW_E_OK != ret)
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d>Invalid parameter, i:%d, pcParam:%s pcValue:%s\n", __func__, __LINE__,
                      i, pcParam[i], pcValue[i]);
        return MW_E_BAD_PARAMETER;
    }

#if(defined(AIR_SUPPORT_SFP))
    if (num != 0)
    {
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap, &old_dst_port_bitmap[num - 1],0);
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap, &dst_port_bitmap[num - 1],0);
    }
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap, &src_in_port,0);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap, &src_eg_port,0);
#endif
    /* For port i+1(ptr_port_mirror_info[i]): bit[1 << (j + 4)] represents dst_port of enabled session (j+1); bit[1 << j] represents src_port of enabled session (j+1).
     * If ptr_port_mirror_info[i] is equal to 0, it means that port i+1 is not used by any enabled session. */
    if(MW_E_OK != httpd_queue_getData(PORT_CFG_INFO, PORT_MIRROR_ID, DB_ALL_ENTRIES, &ptr_db_msg1, &db_size, (void**)&ptr_port_mirror_info))
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d> get db failed!", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    /* Update session_state and dst_port */
    for(i = 0; i < MAX_MIRROR_SESS_NUM; i++)
    {
        CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> ---i:%d, session_state:%d old_dst_port_bitmap:%x dst_port_bitmap:%x\n",
                      __func__, __LINE__, i, session_state[i], old_dst_port_bitmap[i], dst_port_bitmap[i]);
        if (CGI_PORT_MIRROR_SESSION_STATE_INVALIDE != session_state[i] &&
            CGI_PORT_MIRROR_INVALID_PORT_BITMAP != old_dst_port_bitmap[i] &&
            CGI_PORT_MIRROR_INVALID_PORT_BITMAP != dst_port_bitmap[i] &&
            !(CGI_PORT_MIRROR_SESSION_STATE_ENABLE == session_state[i] && 0 == dst_port_bitmap[i]))
        {
            /* Valid dst_port parameters for session i+1 */
            rc = MW_E_OP_INCOMPLETE;

            /* dst_port changes. Port 0 is invalid for dst_port. However, dst_port_bitmap[] will only be 0 when session is disabled. Therefore,
             * it is OK to update dst_port when dst_port_bitmap[] is 0. SSI will change dst_port to 0 if session is disabled to browser also.
             */
            if (old_dst_port_bitmap[i] != dst_port_bitmap[i])
            {
                dst_port = 0;
                for (num = 0; num < PLAT_MAX_PORT_NUM; num++)
                {
                    /* Set the port with lowest port id of the trunk as the dst_port. HW will recognize it as a trunk. */
                    if ((1 << num) & dst_port_bitmap[i])
                    {
                        dst_port = num + 1;
                        break;
                    }
                }

                if (dst_port_bitmap[i] && 0 == dst_port)
                {
                    MW_FREE(ptr_db_msg1);
                    return MW_E_BAD_PARAMETER;
                }

                if (CGI_PORT_MIRROR_SESSION_STATE_ENABLE == session_state[i])
                {
                    /* Set dst port first before enabling the session to prevent the traffic from being forwared to the previous dst port. */
                    rc = httpd_queue_setData(M_UPDATE, PORT_MIRROR_INFO, PORT_MIRROR_DEST_PORT, i + 1, &dst_port, sizeof(UI8_T));
                    CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> i:%d, rc:%d, dst_port:%d\n", __func__, __LINE__, i, rc, dst_port);
                }
                else
                {
                    rc = MW_E_OK;
                }
            }
            else
            {
                rc = MW_E_OK;
            }

            if (MW_E_OK == rc)
            {
                /* Set session state. It might not change. Set it anyway. */
                rc = httpd_queue_setData(M_UPDATE, PORT_MIRROR_INFO, PORT_MIRROR_ENABLE, i + 1, &session_state[i], sizeof(UI8_T));
                CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> i:%d, rc:%d, session_state:%d\n", __func__, __LINE__, i, rc, session_state[i]);
                if (MW_E_OK != rc)
                {
                    MW_FREE(ptr_db_msg1);
                    return rc;
                }

                if ((CGI_PORT_MIRROR_SESSION_STATE_DISABLE == session_state[i]) && (old_dst_port_bitmap[i] != dst_port_bitmap[i]))
                {
                    /* Set dst port after disabling the session to prevent the traffic from being forwarded to port 0. */
                    rc = httpd_queue_setData(M_UPDATE, PORT_MIRROR_INFO, PORT_MIRROR_DEST_PORT, i + 1, &dst_port, sizeof(UI8_T));
                    CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> i:%d, rc:%d, dst_port:%d\n", __func__, __LINE__, i, rc, dst_port);
                    if (MW_E_OK != rc)
                    {
                        MW_FREE(ptr_db_msg1);
                        return rc;
                    }
                }

                /* Update ptr_port_mirror_info[num] of dst_port selection status of (port num + 1) for session i. High 4 bits of ptr_port_mirror_info[num]
                 * represent dst_port selection status of (port num + 1) for each session, while low 4 bits of it represent src_port selection status of
                 * (port num + 1) for each session. Bit1 represents that session is enabled and the port is selected. */
                session_bitmap = (1 << (CGI_PORT_MIRROR_DESTINATION_PORT_INFO_OFFSET + i));
                for (num = 0; num < PLAT_MAX_PORT_NUM; num++)
                {
                    /* set bit to 1 only when its corresponding port is dst_port of the session and the session is enabled. */
                    if (old_dst_port_bitmap[i] && (old_dst_port_bitmap[i] != dst_port_bitmap[i] || CGI_PORT_MIRROR_SESSION_STATE_DISABLE == session_state[i]) &&
                        ((1 << num) & old_dst_port_bitmap[i]))
                    {
                        ptr_port_mirror_info[num] &= ~session_bitmap;
                    }

                    if (CGI_PORT_MIRROR_SESSION_STATE_ENABLE == session_state[i] && ((1 << num) & dst_port_bitmap[i]))
                    {
                        ptr_port_mirror_info[num] |= session_bitmap;
                    }
                }
            }
        }
    }

    /* Update src_port */
    if (CGI_PORT_MIRROR_SESSION_STATE_INVALIDE != session_id)
    {
        CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> session_id:%d, src_in_port:%d, src_eg_port:%d\n", __func__, __LINE__, session_id, src_in_port, src_eg_port);
        /* Valid src_port parameters */
        if(MW_E_OK != httpd_queue_getData(PORT_MIRROR_INFO, DB_ALL_FIELDS, (session_id + 1), &ptr_db_msg2, &db_size, (void **)&ptr_session_mirror_info))
        {
            MW_FREE(ptr_db_msg1);
            return ERR_INPROGRESS;
        }

        /* src_in_port, src_eg_port: bit[i] represents port i+1 is ingress port or egress port of the session. */
        ptr_session_mirror_info->src_in_port = src_in_port;
        ptr_session_mirror_info->src_eg_port = src_eg_port;
        rc = httpd_queue_setData(M_UPDATE, PORT_MIRROR_INFO, DB_ALL_FIELDS, (session_id + 1), ptr_session_mirror_info, sizeof(ONE_DB_PORT_MIRROR_INFO_T));

        CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> rc:%d\n", __func__, __LINE__, rc);
        session_bitmap = 1 << session_id;
        for(i = 0; i < PLAT_MAX_PORT_NUM; i ++)
        {
            CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> %s(%d) port=%d DB_cfg=%x\n", __FUNCTION__, __LINE__, i, ptr_port_mirror_info[i]);
            if (1 == ptr_session_mirror_info->enable && (((1 << i) & ptr_session_mirror_info->src_in_port) ||
                ((1 << i) & ptr_session_mirror_info->src_eg_port)))
            {
                ptr_port_mirror_info[i] |= session_bitmap;
            }
            else
            {
                ptr_port_mirror_info[i] &= ~session_bitmap;
            }

            CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> port=%d update_cfg=%x \r\n", __func__, __LINE__, i, ptr_port_mirror_info[i]);
        }
    }

    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_MIRROR_ID, DB_ALL_ENTRIES, ptr_port_mirror_info, (sizeof(UI8_T) * PLAT_MAX_PORT_NUM));
    MW_FREE(ptr_db_msg1);
    MW_FREE(ptr_db_msg2);

    return rc;
}


UI16_T
ssi_send_port_mirror_ssi_string(
    C8_T *ptr_ssi_str,
    UI16_T *ptr_strlen)
{
    C8_T *ptr_string_buf_1 = NULL, *ptr_string_buf_2 = NULL;
    UI32_T string_len_1 = 0, string_len_2 = 0, i = 0;
    DB_MSG_T *ptr_msg_1 = NULL, *ptr_msg_2 = NULL;
    UI16_T db_size_1 = 0, db_size_2 = 0;
    UI8_T *ptr_data_1 = NULL, *ptr_data_2 = NULL;
    UI32_T *ptr_data_3 = NULL, *ptr_data_4 = NULL;
    MW_ERROR_NO_T rc = MW_E_NO_MEMORY;
    UI32_T egressPbm = 0, ingressPbm = 0, sid = 0;
    UI8_T ingState = 0, egState = 0;
    UI32_T tmplen = 0;

    if ((NULL == ptr_ssi_str) || (NULL == ptr_strlen))
    {
        return tmplen;
    }
    rc = osapi_calloc(SSI_PORT_MIRROR_STRING_LENGTH, "prtMiro", (void **)&ptr_string_buf_1);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d> alloc failed!", __func__, __LINE__);
        return tmplen;
    }
    rc = osapi_calloc(SSI_PORT_MIRROR_STRING_LENGTH, "prtMiro", (void **)&ptr_string_buf_2);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d> alloc failed!", __func__, __LINE__);
        osapi_free((const void *)ptr_string_buf_1);
        return tmplen;
    }

    /* Request DB for port mirror member */
    rc = httpd_queue_getData(PORT_MIRROR_INFO, PORT_MIRROR_ENABLE, DB_ALL_ENTRIES, &ptr_msg_1, &db_size_1, (void **)&ptr_data_1);
    if(MW_E_OK == rc)
    {
        rc = httpd_queue_getData(PORT_MIRROR_INFO, PORT_MIRROR_DEST_PORT, DB_ALL_ENTRIES, &ptr_msg_2, &db_size_2, (void **)&ptr_data_2);
        if(MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> get data success, ptr_enable = %p, ptr_dest_port = %p\n", __func__, __LINE__, ptr_msg_1, ptr_msg_2);
        }
        else
        {
            MW_FREE(ptr_msg_1);
        }
    }
    if(MW_E_OK != rc)
    {
        CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> get data failed \n", __func__, __LINE__);
        osapi_free((const void *)ptr_string_buf_1);
        osapi_free((const void *)ptr_string_buf_2);
        return tmplen;
    }

    tmplen += snprintf(ptr_ssi_str + tmplen, *ptr_strlen - tmplen, "\nvar max_mirror_num=%d;", MAX_MIRROR_SESS_NUM);

    for(i = 0; i < MAX_MIRROR_SESS_NUM; i++)
    {
        string_len_1 += snprintf(ptr_string_buf_1 + string_len_1, SSI_PORT_MIRROR_STRING_LENGTH - string_len_1, "%d,", ptr_data_1[i]);
        string_len_2 += snprintf(ptr_string_buf_2 + string_len_2, SSI_PORT_MIRROR_STRING_LENGTH - string_len_2, "%d,", ((ptr_data_1[i]) ? ptr_data_2[i] : 0));
    }
    MW_FREE(ptr_msg_1);
    MW_FREE(ptr_msg_2);

    tmplen += snprintf(ptr_ssi_str + tmplen, *ptr_strlen - tmplen, "\nvar mirr_info={mirrEn:[%s],mirrPort:[%s]",
        ptr_string_buf_1, ptr_string_buf_2);

    memset(ptr_string_buf_1, 0, SSI_PORT_MIRROR_STRING_LENGTH);
    memset(ptr_string_buf_2, 0, SSI_PORT_MIRROR_STRING_LENGTH);
    string_len_1 = 0;
    string_len_2 = 0;

    rc = httpd_queue_getData(PORT_MIRROR_INFO, PORT_MIRROR_SRC_IN_PORT, DB_ALL_ENTRIES, &ptr_msg_1, &db_size_1, (void **)&ptr_data_3);
    if(MW_E_OK == rc)
    {
        rc = httpd_queue_getData(PORT_MIRROR_INFO, PORT_MIRROR_SRC_EG_PORT, DB_ALL_ENTRIES, &ptr_msg_2, &db_size_2, (void **)&ptr_data_4);
        if(MW_E_OK == rc)
        {
            CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> get data success, ptr_src_in_port =%p, ptr_src_eg_port = %p\n",  __func__, __LINE__, ptr_msg_1, ptr_msg_2);
        }
        else
        {
            MW_FREE(ptr_msg_1);
        }
    }
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d> get data failed \n", __func__, __LINE__);
        osapi_free((const void *)ptr_string_buf_1);
        osapi_free((const void *)ptr_string_buf_2);
        return tmplen;
    }

    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        ingState = 0;
        egState = 0;

        for(sid = 0 ; sid < MAX_MIRROR_SESS_NUM; sid++)
        {
            memcpy((void*)&ingressPbm, &ptr_data_3[sid], sizeof(UI32_T));
            memcpy((void*)&egressPbm, &ptr_data_4[sid], sizeof(UI32_T));
            ingState |= !!(ingressPbm & (1 << i)) ? (1 << sid) : 0;
            egState  |= !!(egressPbm & (1 << i))  ? (1 << sid) : 0;
        }
        string_len_1 += snprintf(ptr_string_buf_1 + string_len_1, SSI_PORT_MIRROR_STRING_LENGTH - string_len_1, "%d,", ingState);
        string_len_2 += snprintf(ptr_string_buf_2 + string_len_2, SSI_PORT_MIRROR_STRING_LENGTH - string_len_2, "%d,", egState);
    }
    MW_FREE(ptr_msg_1);
    MW_FREE(ptr_msg_2);

    CGI_LOG_DEBUG(PORT_MIRROR, "<%s:%d> String ingresspbm = %u, egressPbm = %u \n", __func__, __LINE__, string_len_1, string_len_2);
    tmplen += snprintf(ptr_ssi_str + tmplen, *ptr_strlen - tmplen, ",ingress:[%s],egress:[%s]};", ptr_string_buf_1, ptr_string_buf_2);

    osapi_free((const void *)ptr_string_buf_1);
    osapi_free((const void *)ptr_string_buf_2);

    return tmplen;
}


char
ssi_get_mirroringport_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags)
{
    C8_T err = 0;
    UI16_T total_len = 0;
    UI16_T tmplen = 0;
    UI16_T str_len = HTTPD_MAX_RESPONSE_CHUNKBUFF_LEN;
    C8_T *ptr_ssi_str = NULL;
    if (MW_E_NO_MEMORY == osapi_calloc(str_len, HTTPD_QUEUE_CLI, (void **)&ptr_ssi_str))
    {
        CGI_LOG_ERROR(PORT_MIRROR, "<%s:%d> alloc failed!", __func__, __LINE__);
        return ERR_MEM;
    }

    tmplen = snprintf(ptr_ssi_str, str_len, "var max_port_num=%d;", PLAT_MAX_PORT_NUM);
    str_len -= tmplen;

    tmplen += ssi_send_port_mirror_ssi_string(ptr_ssi_str + tmplen, &str_len);

    err = send_format_response(&total_len, pcb, apiflags, "<script>%s</script>", ptr_ssi_str);
    osapi_free(ptr_ssi_str);
    if(ERR_OK != err)
    {
        return err;
    }

    *length = total_len;

#if HTTPD_DBG_ON
    CGI_LOG_DEBUG(PORT_MIRROR, "[%s] line [%d] leave\n", __FUNCTION__, __LINE__);
#endif

    return ERR_OK;
}

