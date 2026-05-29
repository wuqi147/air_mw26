/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:   spanningTree.c
 * PURPOSE:
 *      CGI and SSI function of spanning tree configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <osapi.h>

#include <mw_utils.h>
#include <db_api.h>
#include <httpd_queue.h>
#include "mw_log.h"
#include <web.h>
#include "stp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define FORCE_VERSION_STP 0
#define FORCE_VERSION_RSTP 2
#define FORCE_VERSION_MSTP 3
#define STP_MIN_PRIORITY 0
#define STP_MAX_PRIORITY 61440
#define STP_PRIORITY_STEP 4096
#define STP_MIN_PORT_PRIORITY 0
#define STP_MAX_PORT_PRIORITY 240
#define STP_STEP_PORT_PRIORITY 16
#define STP_MIN_MAX_AGE 6
#define STP_MAX_MAX_AGE 40
#define STP_MIN_FORWARD_DELAY 4
#define STP_MAX_FORWARD_DELAY 30
#define STP_MIN_TRANSMIT_HOLD_COUNT 1
#define STP_MAX_TRANSMIT_HOLD_COUNT 10
#define STP_MIN_PORT_PATH_COST 1
#define STP_MAX_PORT_PATH_COST 200000000

/* MACRO FUNCTION DECLARATIONS
 */
#define spanning_tree_http_appendResponse(len, pcb, apiflags, fmt, ...) ({ \
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
static UI16_T  _fwd_delay = 0;
static UI16_T  _max_age = 0;


/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_spanningTree(
    int  index,
    int  numParams,
    char *ptr_param[],
    char *ptr_value[])
{
    MW_ERROR_NO_T                   rc = MW_E_OK;
    DB_STP_INFO_T                   info;
    DB_STP_PORT_INFO_T              port_info;
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_PORT_INFO_T    port_info_list;
#endif
    UI8_T                           enable = 0, port_priority = 0, status = 0;
    UI32_T                          i = 0, p = 0, force_version = 0, cost = 0, pbmp = 0;
    UI16_T                          fwd_delay = 0, max_age = 0;
    UI16_T                          tx_hold_cnt = 0, priority = 0;
    UI16_T                          tmp_fwd_delay = 0, tmp_max_age = 0;
    UI8_T                           auto_cost_support = 0, auto_cost_enable = 0;
    UI8_T                           admin_edge = 0;
    UI8_T                           mcheck = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T                          trunkBitMap =0;
#endif

    /* db variable*/
#ifdef AIR_SUPPORT_MSTP
    DB_MSG_T                        *ptr_msg = NULL;
    UI16_T                          size = 0;
    void                            *ptr_db_data = NULL;
#endif

    STP_PARAM_FLAG_INIT(state);
    STP_PARAM_FLAG_INIT(forceProtocolVersion);
    STP_PARAM_FLAG_INIT(forwardDelay);
    STP_PARAM_FLAG_INIT(maxAge);
    STP_PARAM_FLAG_INIT(transmitHoldCount);
    STP_PARAM_FLAG_INIT(priority);
    STP_PARAM_FLAG_INIT(pbmp);
    /* expected CGI format:
     *   stpCfg.cgi?state=1
     *   stpCfg.cgi?forceProtocolVersion=0
     *   stpCfg.cgi?forwardDelay=10
     *   stpCfg.cgi?maxAge=25
     *   stpCfg.cgi?transmitHoldCount=5
     *   stpCfg.cgi?priority=16
     *   stpCfg.cgi?state1&forceProtocolVersion=0&forwardDelay=10&maxAge=25&
     *              transmitHoldCount=5&priority=16
     *   stpCfg.cgi?<with other combination>
     *   stpPortCfg.cgi?pbmp=4095&portPriority=16&cost=200
     */

    /* init */
    memset(&info, 0, sizeof(DB_STP_INFO_T));
    memset(&port_info, 0, sizeof(DB_STP_PORT_INFO_T));
#ifdef AIR_SUPPORT_MSTP
    memset(&port_info_list, 0, sizeof(DB_MSTP_INSTANCE_PORT_INFO_T));
#endif
    /* get value from url params */
    CGI_LOG_DEBUG(STP, "<%s:%d> handling >>>>\n", __func__, __LINE__);
    for (i = 0; i < numParams; ++i)
    {
        CGI_LOG_DEBUG(STP, "<%s:%d> >> %s: %s\n", __func__, __LINE__, ptr_param[i], ptr_value[i]);
        if (!strcmp(ptr_param[i], "state"))
        {
            enable = atoi(ptr_value[i]);
            if ((TRUE != enable) && (FALSE != enable))
            {
                CGI_LOG_WARN(STP, "invalid state");
                return MW_E_OK;
            }
            STP_SET_FLAG(state);
        }
        else if (!strcmp(ptr_param[i], "forceProtocolVersion"))
        {
            force_version = atoi(ptr_value[i]);
            if ((FORCE_VERSION_STP!= force_version) && (FORCE_VERSION_RSTP!= force_version) && (FORCE_VERSION_MSTP!= force_version))
            {
                CGI_LOG_WARN(STP, "invalid forceProtocolVersion");
                return MW_E_OK;
            }
            STP_SET_FLAG(forceProtocolVersion);
        }
        else if (!strcmp(ptr_param[i], "forwardDelay"))
        {
            fwd_delay = atoi(ptr_value[i]);
            if ((fwd_delay < STP_MIN_FORWARD_DELAY) || (fwd_delay > STP_MAX_FORWARD_DELAY))
            {
                CGI_LOG_WARN(STP, "invalid forwardDelay");
                return MW_E_OK;
            }
            STP_SET_FLAG(forwardDelay);
        }
        else if (!strcmp(ptr_param[i], "maxAge"))
        {
            max_age = atoi(ptr_value[i]);
            if ((max_age < STP_MIN_MAX_AGE) || (max_age > STP_MAX_MAX_AGE))
            {
                CGI_LOG_WARN(STP, "invalid forwardDelay");
                return MW_E_OK;
            }
            STP_SET_FLAG(maxAge);
        }
        else if (!strcmp(ptr_param[i], "transmitHoldCount"))
        {
            tx_hold_cnt = atoi(ptr_value[i]);
            if ((tx_hold_cnt < STP_MIN_TRANSMIT_HOLD_COUNT) || (tx_hold_cnt > STP_MAX_TRANSMIT_HOLD_COUNT))
            {
                CGI_LOG_WARN(STP, "invalid transmitHoldCount");
                return MW_E_OK;
            }
            STP_SET_FLAG(transmitHoldCount);
        }
        else if (!strcmp(ptr_param[i], "priority"))
        {
            priority = atoi(ptr_value[i]);
            if ((priority < STP_MIN_PRIORITY) || (priority > STP_MAX_PRIORITY) || (0 != (priority % STP_PRIORITY_STEP)))
            {
                CGI_LOG_WARN(STP, "invalid priority");
                return MW_E_OK;
            }
            STP_SET_FLAG(priority);
        }
        else if (!strcmp(ptr_param[i], "pbmp"))
        {
            pbmp = atoi(ptr_value[i]);
            STP_SET_FLAG(pbmp);
        }
        else if (!strcmp(ptr_param[i], "portStatus"))
        {
            status = atoi(ptr_value[i]);
            if ((TRUE != status) && (FALSE != status))
            {
                CGI_LOG_WARN(STP, "invalid status");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "portPriority"))
        {
            port_priority = atoi(ptr_value[i]);
            if ((port_priority < STP_MIN_PORT_PRIORITY) || (port_priority > STP_MAX_PORT_PRIORITY) || (0 != (port_priority % STP_STEP_PORT_PRIORITY)))
            {
                CGI_LOG_WARN(STP, "invalid portPriority");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "cost"))
        {
            cost = atoi(ptr_value[i]);
            if (0 == cost)
            {
                continue;
            }
            if ((cost < STP_MIN_PORT_PATH_COST) || (cost > STP_MAX_PORT_PATH_COST))
            {
                CGI_LOG_WARN(STP, "invalid cost");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "autoCostSupport"))
        {
            auto_cost_support = atoi(ptr_value[i]);
            if ((TRUE != auto_cost_support) && (FALSE != auto_cost_support))
            {
                CGI_LOG_WARN(STP, "invalid autoCostSupport");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "autoCostEnable"))
        {
            auto_cost_enable = atoi(ptr_value[i]);
            if ((TRUE != auto_cost_enable) && (FALSE != auto_cost_enable))
            {
                CGI_LOG_WARN(STP, "invalid autoCostEnable");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "adminEdge"))
        {
            admin_edge = atoi(ptr_value[i]);
            if ((TRUE != admin_edge) && (FALSE != admin_edge))
            {
                CGI_LOG_WARN(STP, "invalid adminEdge");
                return MW_E_OK;
            }
        }
        else if (!strcmp(ptr_param[i], "mcheck"))
        {
            mcheck = atoi(ptr_value[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        else if (0 == strcmp(ptr_param[i],"trunkBitMap"))
        {
            trunkBitMap = atoi(ptr_value[i]);
        }
#endif
    }

#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&pbmp,0);
#endif
    pbmp <<= 1;

    if (TRUE == STP_FLAG(maxAge))
    {
        tmp_max_age = max_age;
    }
    else
    {
        tmp_max_age = _max_age;
    }
    if (TRUE == STP_FLAG(forwardDelay))
    {
        tmp_fwd_delay = fwd_delay;
    }
    else
    {
        tmp_fwd_delay = _fwd_delay;
    }
    if (2 * (tmp_fwd_delay - 1) < tmp_max_age)
    {
        CGI_LOG_WARN(STP, "illegal: 2 * (Forward Delay - 1) >= Max Age");
        return MW_E_OK;
    }

    if (TRUE == STP_FLAG(forceProtocolVersion))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_FORCE_VERSION, DB_ALL_ENTRIES, &force_version, sizeof(force_version));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }

    if (TRUE == STP_FLAG(state))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(enable));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }

    if (TRUE == STP_FLAG(maxAge))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_MAX_AGE, DB_ALL_ENTRIES, &max_age, sizeof(max_age));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }

    if (TRUE == STP_FLAG(forwardDelay))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_FORWARD_DELAY, DB_ALL_ENTRIES, &fwd_delay, sizeof(fwd_delay));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }

    if (TRUE == STP_FLAG(transmitHoldCount))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_TRANSMIT_HOLD_COUNT, DB_ALL_ENTRIES, &tx_hold_cnt, sizeof(tx_hold_cnt));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
    }

    if (TRUE == STP_FLAG(priority))
    {
        rc = httpd_queue_setData(M_UPDATE, STP_INFO, STP_BRIDGE_PRIORITY, DB_ALL_ENTRIES, &priority, sizeof(priority));
        MW_PARAM_CHK(rc != MW_E_OK, rc);

#ifdef AIR_SUPPORT_MSTP
        rc = httpd_queue_setData(M_UPDATE, MSTP_INSTANCE, MSTP_INSTANCE_PRIORITY, 1, &priority, sizeof(priority));
        MW_PARAM_CHK(rc != MW_E_OK, rc);
#endif
    }

    if (TRUE == STP_FLAG(pbmp))
    {
        for (p = 1; p <= PLAT_MAX_PORT_NUM ; p++)
        {
            if (pbmp & BIT(p))
            {
                memset(&port_info, 0, sizeof(DB_STP_PORT_INFO_T));
                port_info.priority = port_priority;
                port_info.cost = cost;
                port_info.admin_edge = admin_edge;
                port_info.mcheck = mcheck;
                port_info.status = status;
#ifdef AIR_SUPPORT_STP_AUTO_COST
                if (TRUE == auto_cost_support)
                {
                    port_info.auto_cost_enable = auto_cost_enable;
                } else {
                    port_info.auto_cost_enable = 0;
                }
#endif
                rc = httpd_queue_setData(M_UPDATE, STP_PORT_INFO, DB_ALL_FIELDS, p, &port_info, sizeof(DB_STP_PORT_INFO_T));
                MW_PARAM_CHK(rc != MW_E_OK, rc);

#ifdef AIR_SUPPORT_MSTP
                /* also update MSTP_PORT_INFO priority & cost here */
                rc = httpd_queue_getData(MSTP_INSTANCE_PORT, DB_ALL_FIELDS, p, &ptr_msg, &size, &ptr_db_data);
                osapi_memcpy(&port_info_list, ptr_db_data, size);
                MW_FREE(ptr_msg);
                if (MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(STP, "Get MSTP_INSTANCE_PORT failed\n");
                    return rc;
                }

                port_info_list.priority[0] = port_priority;
                port_info_list.cost[0] = cost;

                rc = httpd_queue_setData(M_UPDATE, MSTP_INSTANCE_PORT, DB_ALL_FIELDS, p, &port_info_list, sizeof(DB_MSTP_INSTANCE_PORT_INFO_T));
                if(MW_E_OK != rc)
                {
                    CGI_LOG_ERROR(STP, "Set MSTP INSTANCE PORT failed\n");
                    return rc;
                }
#endif
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
ssi_get_spanning_tree_info_Handle(
    int              *ptr_length,
    struct tcp_pcb   *ptr_pcb,
    unsigned int     apiflags)
{
    UI32_T                  total_len = 0, p = 0;
    DB_MSG_T                *ptr_msg = NULL;
    UI8_T                   *ptr_data = NULL;
    UI16_T                  data_size = 0, len = 0;
    C8_T                    *ptr_array = NULL, *ptr_portArray1 = NULL, *ptr_portArray2 = NULL, *ptr_edgeArray = NULL;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    C8_T                    *ptr_portArray3 = NULL;
#endif
    C8_T                    *ptr_statusArray = NULL, *ptr_modeArray = NULL;
    UI32_T                  offset1 = 0, offset2 = 0;
    UI32_T                  status_offset = 0, edge_offset = 0, mode_offset = 0;
    UI8_T                   stp_state = 0;
    UI32_T                  data, array_size;
    DB_STP_PORT_INFO_T      port_info;
    DB_STP_INFO_T           stp_info;
#ifdef AIR_SUPPORT_LP
    DB_LOOP_PREVEN_INFO_T   *ptr_lp = NULL;
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI32_T                  offset3 = 0;
#endif

    /*  expected javascript string
     *  var stp_ds = {
     *      portNum: 28,
     *      state: 0,
     *      forceProtocolVer: 2,
     *      forwardDelay:   15,
     *      maxAge: 20,
     *      transmitHoldCount:  6,
     *      priority:   32768,
     *      autoCostSupport:    1,
     *      portStauts: [1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
     *                      1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
     *                      0, 0, 0, 0, 0, 0, 0, 0],
     *      portPriority: [128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
     *                      128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
     *                      128, 128, 128, 128, 128, 128, 128, 128],
     *      cost: [2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000,
     *              2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000,
     *              2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000],
     *      edge: [1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
     *                      1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
     *                      0, 0, 0, 0, 0, 0, 0, 0],
     *      autoCostEnable: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *      portState: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *      role: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     *              0, 0, 0, 0, 0, 0, 0, 0],
     *      operCost: [20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000,
     *              20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000,
     *              20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000],
     *  };
     *  var lpEn = 0;
     */

    if (httpd_queue_getData(STP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    memcpy(&stp_info, ptr_data, data_size);
    stp_state = stp_info.enable;
    MW_FREE(ptr_msg);

    /* Field: portNum, state */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var stp_ds={portNum:%d,state:%d,", PLAT_MAX_PORT_NUM, (stp_state != 0) ? 1 : 0);

    /* Field: forceProtocolVer */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "forceProtocolVersion:%d,", stp_info.force_version);

    /* Field: forwardDelay */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "forwardDelay:%d,", stp_info.forward_delay);
    _fwd_delay = stp_info.forward_delay;

    /* Field: maxAge */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "maxAge:%d,", stp_info.max_age);
    _max_age = stp_info.max_age;

    /* Field: transmitHoldCount */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "transmitHoldCount:%d,", stp_info.transmit_hold_count);

    /* Field: priority */
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "priority:%d,", stp_info.priority);

#ifdef AIR_SUPPORT_STP_AUTO_COST
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "autoCostSupport:%d,", (stp_info.auto_cost_support != 0) ? 1 : 0);
#else
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "autoCostSupport:%d,", 0);
#endif

    array_size = MW_PORT_NUM * 10;
    osapi_calloc(array_size * 6, HTTPD_QUEUE_CLI, (void **)&ptr_array);
    if (NULL == ptr_array)
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Insufficient memory\n", __func__, __LINE__);
        return ERR_MEM;
    }

    ptr_portArray1 = ptr_array;
    ptr_portArray2 = ptr_portArray1 + array_size;
    ptr_statusArray = ptr_portArray2 + array_size;
    ptr_modeArray = ptr_statusArray + array_size;
    ptr_edgeArray = ptr_modeArray + array_size;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    ptr_portArray3 = ptr_edgeArray + array_size;
#endif

    /* Field: port mode */
    data = stp_info.portMode;

    /* Field: portPriority & cost */
    for (p = 1, offset1 = 0, offset2 = 0; p <= PLAT_MAX_PORT_NUM; p++)
    {
        if (httpd_queue_getData(STP_PORT_INFO, DB_ALL_FIELDS, p, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
        {
            CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
            MW_FREE(ptr_array);
            return ERR_INPROGRESS;
        }
        memcpy(&port_info, ptr_data, data_size);
        MW_FREE(ptr_msg);
        offset1 += osapi_snprintf(ptr_portArray1 + offset1, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_info.priority);
        offset2 += osapi_snprintf(ptr_portArray2 + offset2, (array_size-offset2), "%s%d", ((p == 1) ? "" : ","), port_info.cost);
#ifdef AIR_SUPPORT_STP_AUTO_COST
        offset3 += osapi_snprintf(ptr_portArray3 + offset3, (array_size-offset3), "%s%d", ((p == 1) ? "" : ","), port_info.auto_cost_enable);
#endif
        status_offset += osapi_snprintf(ptr_statusArray + status_offset, (array_size-status_offset), "%s%d", ((p == 1) ? "" : ","), port_info.status);
        mode_offset += osapi_snprintf(ptr_modeArray + mode_offset, (array_size-mode_offset), "%s%d", ((p == 1) ? "" : ","),
                                    (port_info.status ? ((stp_info.force_version == STP_VERSION_MSTP) ? \
                                    (((data >> (p-1)) & 0x1)? 2 : 0):((data >> (p-1)) & 0x1)):3));
        edge_offset += osapi_snprintf(ptr_edgeArray + edge_offset, (array_size-edge_offset), "%s%d", ((p == 1) ? "" : ","), port_info.admin_edge);
    }
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "portStatus:[%s],", ptr_statusArray);
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "portMode:[%s],", ptr_modeArray);
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "portPriority:[%s],", ptr_portArray1);
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "cost:[%s],", ptr_portArray2);
#ifdef AIR_SUPPORT_STP_AUTO_COST
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "autoCostEnable:[%s],", ptr_portArray3);
#endif
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "adminEdge:[%s]};\n", ptr_edgeArray);

#ifdef AIR_SUPPORT_LP
    if(MW_E_OK == httpd_queue_getData(LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void**)&ptr_lp))
    {
        total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var lpEn = %d;\n", ptr_lp->enable);
        MW_FREE(ptr_msg);
    }
    else
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
        MW_FREE(ptr_array);
        return ERR_INPROGRESS;
    }
#else
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var lpEn = %d;\n", 0);
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_RSTP
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var rstpSupport = %d;\n", 1);
#else
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var rstpSupport = %d;\n", 0);
#endif

#ifdef AIR_SUPPORT_MSTP
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var mstpSupport = %d;\n", 1);
#else
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var mstpSupport = %d;\n", 0);
#endif

    *ptr_length = total_len;
    MW_FREE(ptr_array);

    return ERR_OK;
}

char
ssi_get_spanning_tree_port_info_Handle(
    int              *ptr_length,
    struct tcp_pcb   *ptr_pcb,
    unsigned int     apiflags)
{
    C8_T                    *ptr_array = NULL, *ptr_portArray1 = NULL, *ptr_portArray2 = NULL;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    C8_T                    *ptr_portArray3 = NULL;
#endif
    UI32_T                  offset1 = 0, offset2 = 0;
    DB_MSG_T                *ptr_msg = NULL;
    UI8_T                   *ptr_data = NULL;
    UI16_T                  data_size = 0;
    DB_STP_PORT_OPER_T      port_oper;
    DB_STP_INFO_T           stp_info;
    UI32_T                  total_len = 0, p = 0, array_size;
    UI16_T                  len = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI32_T                  offset3 = 0;
#endif

    if (httpd_queue_getData(STP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }
    memcpy(&stp_info, ptr_data, data_size);
    MW_FREE(ptr_msg);

    array_size = MW_PORT_NUM * 10;
    osapi_calloc(array_size * 3, HTTPD_QUEUE_CLI, (void **)&ptr_array);
    if (NULL == ptr_array)
    {
        CGI_LOG_ERROR(STP, "<%s:%d> Insufficient memory\n", __func__, __LINE__);
        return ERR_MEM;
    }

    ptr_portArray1 = ptr_array;
    ptr_portArray2 = ptr_portArray1 + array_size;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    ptr_portArray3 = ptr_portArray2 + array_size;
#endif

    osapi_memset(ptr_portArray1, 0, array_size);
    osapi_memset(ptr_portArray2, 0, array_size);
#ifdef AIR_SUPPORT_STP_AUTO_COST
    osapi_memset(ptr_portArray3, 0, array_size);
#endif

    /* Field: state & role */
    for (p = 1, offset1 = 0, offset2 = 0; p <= PLAT_MAX_PORT_NUM; p++)
    {
        if (httpd_queue_getData(STP_PORT_OPER_INFO, DB_ALL_FIELDS, p, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
        {
            CGI_LOG_ERROR(STP, "<%s:%d> Error out\n", __func__, __LINE__);
            MW_FREE(ptr_array);
            return ERR_INPROGRESS;
        }
        memcpy(&port_oper, ptr_data, sizeof(DB_STP_PORT_OPER_T));
        MW_FREE(ptr_msg);

#ifdef  AIR_SUPPORT_RSTP_SECURITY
        if ((stp_info.lg_state >> (p-1)) & 0x1)
        {
            port_oper.state = STP_LOOP_INC;          /* loop inconsistent */
        }
        else if ((stp_info.rg_state>> (p-1)) & 0x1)
        {
            port_oper.state = STP_ROOT_INC;          /* root inconsistent */
        }

        if ((stp_info.bg_state>> (p-1)) & 0x1)
        {
            port_oper.state = STP_GUARD_DOWN;        /* bpdu guard down */
        }
        else if (STP_ROLE_DISABLED == port_oper.role)
        {
            port_oper.state = STP_DISABLED;
        }
#else
        if (STP_ROLE_DISABLED == port_oper.role)
        {
            port_oper.state = STP_DISABLED;
        }
#endif
        offset1 += osapi_snprintf(ptr_portArray1 + offset1, (array_size-offset1), "%s%d", ((p == 1) ? "" : ","), port_oper.state);
        offset2 += osapi_snprintf(ptr_portArray2 + offset2, (array_size-offset2), "%s%d", ((p == 1) ? "" : ","), port_oper.role);
#ifdef AIR_SUPPORT_STP_AUTO_COST
        offset3 += osapi_snprintf(ptr_portArray3 + offset3, (array_size-offset3), "%s%d", ((p == 1) ? "" : ","), port_oper.oper_cost);
#endif
    }
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "var stp_oper_ds={portState:[%s],", ptr_portArray1);

#ifdef AIR_SUPPORT_STP_AUTO_COST
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "operCost:[%s],", ptr_portArray3);
#endif
    total_len += spanning_tree_http_appendResponse(len, ptr_pcb, apiflags, "role:[%s]};\n", ptr_portArray2);

    *ptr_length = total_len;
    MW_FREE(ptr_array);

    return ERR_OK;
}
