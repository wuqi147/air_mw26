/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:   spanningTreeSecurity.c
 * PURPOSE:
 *      CGI and SSI function of spanning tree security configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <osapi.h>
#include <mw_utils.h>
#include <db_api.h>
#include <httpd_queue.h>
#include <mw_log.h>
#include <web.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define STP_MIN_INTERVAL 1
#define STP_MAX_INTERVAL 10
#define STP_MIN_THRESHOLD 0
#define STP_MAX_THRESHOLD 100

#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
/* MACRO FUNCTION DECLARATIONS
 */
#define spanning_tree_sec_http_appendResponse(len, pcb, apiflags, fmt, ...) ({ \
        char send_err = send_format_response(&(len), (pcb), (apiflags), fmt, __VA_ARGS__); \
        CGI_LOG_DEBUG(STP, "<%s:%d> rc = %d, len = %d, Send string: "fmt"\n", __FUNCTION__, __LINE__, send_err, len, __VA_ARGS__); \
        if (ERR_OK != send_err) { \
            return send_err; \
        } \
        len; \
    })

#define STP_FLAG(param) \
    is_updt_##param

#define STP_PARAM_FLAG_INIT(param) \
    UI8_T STP_FLAG(param) = 0

#define STP_SET_FLAG(param) \
    STP_FLAG(param) = 1


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
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_spanningTreeSecurity(
    int  index,
    int  numParams,
    char *ptr_param[],
    char *ptr_value[])
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_RSTP_PORT_SEC_T  port_info;
    UI8_T               LP_enable = 0, RP_enable = 0, BPDUG = 0, TC_state = 0;
    UI32_T              i = 0, p = 0, pbmp = 0;
    UI8_T               interval = 0, threshold = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap =0;
#endif
    STP_PARAM_FLAG_INIT(pbmp);
    /* expected CGI format:
     *   stpSecCfg.cgi?portLP=1
     *   stpSecCfg.cgi?portRP=0
     *   stpSecCfg.cgi?portBPDUG=1
     *   stpSecCfg.cgi?portTC_st=1
     *   stpSecCfg.cgi?portTC_intvl=5
     *   stpSecCfg.cgi?portTC_thrhld=0
     */

    /* init */
    memset(&port_info, 0, sizeof(DB_RSTP_PORT_SEC_T));

    /* get value from url params */
    CGI_LOG_DEBUG(STP, "<%s:%d> handling >>>>\n", __func__, __LINE__);
    for (i = 0; i < numParams; ++i)
    {
        CGI_LOG_DEBUG(STP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, ptr_param[i], ptr_value[i]);
        if (!strcmp(ptr_param[i], "portLP"))
        {
            LP_enable = atoi(ptr_value[i]);
            if ((TRUE != LP_enable) && (FALSE != LP_enable))
            {
                CGI_LOG_WARN(STP, "invalid loop protect");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "portRP"))
        {
            RP_enable = atoi(ptr_value[i]);
            if ((TRUE != RP_enable) && (FALSE != RP_enable))
            {
                CGI_LOG_WARN(STP, "invalid root protect");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "portBPDUG"))
        {
            BPDUG = atoi(ptr_value[i]);
            if ((TRUE != BPDUG) && (FALSE != BPDUG))
            {
                CGI_LOG_WARN(STP, "invalid BPDU Guard");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "portTC_st"))
        {
            TC_state = atoi(ptr_value[i]);
            if ((TRUE != TC_state) && (FALSE != TC_state))
            {
                CGI_LOG_WARN(STP, "invalid TC guard state");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "pbmp"))
        {
            pbmp = atoi(ptr_value[i]);
            STP_SET_FLAG(pbmp);
        }
        else if (!strcmp(ptr_param[i], "portTC_intvl"))
        {
            interval = atoi(ptr_value[i]);
            if ((interval < STP_MIN_INTERVAL) || (interval > STP_MAX_INTERVAL))
            {
                CGI_LOG_WARN(STP, "invalid TC guard interval");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "portTC_thrhld"))
        {
            threshold = atoi(ptr_value[i]);
            if ((threshold < STP_MIN_THRESHOLD) || (threshold > STP_MAX_THRESHOLD))
            {
                CGI_LOG_WARN(STP, "invalid TC guard threshold");
                return MW_E_OK;
            }
        }
    }

#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&pbmp,0);
#endif
    pbmp <<= 1;

    if (TRUE == STP_FLAG(pbmp))
    {
        for (p = 1; p <= PLAT_MAX_PORT_NUM ; p++)
        {
            if (pbmp & BIT(p))
            {
                memset(&port_info, 0, sizeof(DB_RSTP_PORT_SEC_T));
                port_info.bpdu_status = BPDUG;
                port_info.tc_status = TC_state;
                port_info.tc_interval = interval;
                port_info.tc_threshold = threshold;
                port_info.root_status = RP_enable;
                port_info.loop_status = LP_enable;
                rc = httpd_queue_setData(M_UPDATE, RSTP_PORT_SECURITY, DB_ALL_FIELDS, p, &port_info, sizeof(DB_RSTP_PORT_SEC_T));
                MW_PARAM_CHK(rc != MW_E_OK, rc);
            }
        }
    }

    CGI_LOG_DEBUG(STP, "<%s:%d> <<<<< handled\n", __func__, __LINE__);


    return MW_E_OK;
}

//==========================================================================================
//  SSI handler
//==========================================================================================
char
ssi_get_spanning_tree_security_info_Handle(
    int              *ptr_length,
    struct tcp_pcb   *ptr_pcb,
    unsigned int     apiflags)
{
    UI32_T                  total_len = 0, p = 0;
    DB_MSG_T                *ptr_msg = NULL;
    UI8_T                   *ptr_data = NULL;
    UI16_T                  data_size = 0, len = 0;
    C8_T                    *ptr_array = NULL, *ptr_portArray1 = NULL, *ptr_portArray2 = NULL, *ptr_portArray3 = NULL;
    C8_T                    *ptr_portArray4 = NULL, *ptr_portArray5 = NULL, *ptr_portArray6 = NULL;
    UI32_T                  offset1 = 0, offset2 = 0, offset3 = 0, offset4 = 0, offset5 = 0, offset6 = 0;
    UI32_T                  array_size;
    DB_RSTP_PORT_SEC_T      port_info;

    /*  expected javascript string
     *  var stp_sec_ds = {
     *      portNum: 28,
     *      portLP: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     *                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     *                      1, 1, 1, 1, 1, 1, 1, 1],
     *      portRP: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     *              1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     *              1, 1, 1, 1, 1, 1, 1, 1],
     *      portBPDUG: [1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
     *                      1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
     *                      0, 0, 0, 0, 0, 0, 0, 0],
     *      portTC_st: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *      portTC_intvl: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *      portTC_thrhld: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *  };
     */

    /* Field: portNum */
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "var stp_sec_ds={portNum:%d,", PLAT_MAX_PORT_NUM);

    array_size = MW_PORT_NUM * 10;
    osapi_calloc(array_size * 6, HTTPD_QUEUE_CLI, (void **)&ptr_array);
    if (NULL == ptr_array)
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Insufficient memory\n", __func__, __LINE__);
        return ERR_MEM;
    }

    ptr_portArray1 = ptr_array;
    ptr_portArray2 = ptr_portArray1 + array_size;
    ptr_portArray3 = ptr_portArray2 + array_size;
    ptr_portArray4 = ptr_portArray3 + array_size;
    ptr_portArray5 = ptr_portArray4 + array_size;
    ptr_portArray6 = ptr_portArray5 + array_size;

    for (p = 1, offset1 = 0, offset2 = 0, offset3 = 0, offset4 = 0, offset5 = 0, offset6 = 0; p <= PLAT_MAX_PORT_NUM; p++)
    {
        if (httpd_queue_getData(RSTP_PORT_SECURITY, DB_ALL_FIELDS, p, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
        {
            CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
            MW_FREE(ptr_array);
            return ERR_INPROGRESS;
        }
        memcpy(&port_info, ptr_data, data_size);
        MW_FREE(ptr_msg);

        offset1 += osapi_snprintf(ptr_portArray1 + offset1, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.loop_status);
        offset2 += osapi_snprintf(ptr_portArray2 + offset2, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.root_status);
        offset3 += osapi_snprintf(ptr_portArray3 + offset3, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.bpdu_status);
        offset4 += osapi_snprintf(ptr_portArray4 + offset4, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.tc_status);
        offset5 += osapi_snprintf(ptr_portArray5 + offset5, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.tc_interval);
        offset6 += osapi_snprintf(ptr_portArray6 + offset6, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.tc_threshold);
    }
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portLP:[%s],", ptr_portArray1);
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portRP:[%s],", ptr_portArray2);
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portBPDUG:[%s],", ptr_portArray3);
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portTC_st:[%s],", ptr_portArray4);
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portTC_intvl:[%s],", ptr_portArray5);
    total_len += spanning_tree_sec_http_appendResponse(len, ptr_pcb, apiflags, "portTC_thrhld:[%s]};", ptr_portArray6);

    *ptr_length = total_len;
    MW_FREE(ptr_array);

    return ERR_OK;
}
#endif
