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

/* FILE NAME:  syncd_api.c
 * PURPOSE:
 *  Implement API function table of syncd.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include "syncd_in.h"
#include "syncd_opts.h"
#include "syncd_api_port.h"
#include "syncd_api_l2.h"
#include "syncd_api_l2mc.h"
#include "syncd_api_swc.h"
#include "syncd_api_lag.h"
#include "syncd_api_ratelimit.h"
#include "syncd_api_sec.h"
#include "syncd_api_mir.h"
#include "syncd_api_qos.h"
#include "syncd_api_vlan.h"
#include "syncd_api_macvlan.h"
#include "syncd_api_stormctrl.h"
#include "syncd_api_monitor.h"
#include "syncd_api_stp.h"
#include "air_error.h"
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* The initalization of VLAN_CFG_INFO relates to PORT_CFG_INFO Ingress filter,
 * VLAN_CFG_INFO must be put in front of PORT_CFG_INFO
 */
static const AIR_API_TABLE_T _tb_api_func[] = {
#if 0/* TODO: undefined API functions */
    /*
     * No need notification from DB
     * TRUNK_PORT notify is enough for trunk setting
     * {PORT_CFG_INFO,     PORT_TRUNK_ID,              syncd_api_},
     */
    {PORT_CFG_INFO,     PORT_MIRROR_ID,             syncd_api_},
#endif/* TODO: undefined API functions */

    /* Table-ID         Filed-ID                    Handle function */
    {VLAN_CFG_INFO,     DB_ALL_FIELDS,              syncd_api_vlan_config},
    {PORT_CFG_INFO,     PORT_SETTINGS,              syncd_api_port_settings},
    {PORT_CFG_INFO,     PORT_ADMIN_STATUS,          syncd_api_port_status},
    {PORT_CFG_INFO,     PORT_ISOLATION,             syncd_api_vlan_matrix},
    {PORT_CFG_INFO,     PORT_INGRESS_RATE,          syncd_api_ratelimit},
    {PORT_CFG_INFO,     PORT_EGRESS_RATE,           syncd_api_ratelimit},
    {PORT_CFG_INFO,     PORT_EEE_ENABLE,            syncd_api_port_eeeConfig},
    {PORT_CFG_INFO,     PORT_PVID,                  syncd_api_vlan_pvid},
    {PORT_CFG_INFO,     PORT_VLAN_IG_FILTER,        syncd_api_vlan_mode},
    {PORT_CFG_INFO,     PORT_MAC_LIMIT,             syncd_api_sec_macLimit},
    {PORT_CFG_INFO,     PORT_STORM_BC_MODE,         syncd_api_stormctrl_cfg},
    {PORT_CFG_INFO,     PORT_STORM_MC_MODE,         syncd_api_stormctrl_cfg},
    {PORT_CFG_INFO,     PORT_STORM_UC_MODE,         syncd_api_stormctrl_cfg},
    {PORT_CFG_INFO,     PORT_STORM_BC_RATE,         syncd_api_stormctrl_rate},
    {PORT_CFG_INFO,     PORT_STORM_MC_RATE,         syncd_api_stormctrl_rate},
    {PORT_CFG_INFO,     PORT_STORM_UC_RATE,         syncd_api_stormctrl_rate},
    {PORT_CFG_INFO,     PORT_STORM_BC_CFG,          syncd_api_stormctrl_en},
    {PORT_CFG_INFO,     PORT_STORM_MC_CFG,          syncd_api_stormctrl_en},
    {PORT_CFG_INFO,     PORT_STORM_UC_CFG,          syncd_api_stormctrl_en},
    {PORT_QOS,          PORT_QOS_PRIORITY,          syncd_api_qos_portPri},
    {PORT_QOS,          PORT_QOS_Q0_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q1_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q2_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q3_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q4_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q5_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q6_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q7_WEIGHT_WRR,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q0_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q1_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q2_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q3_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q4_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q5_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q6_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_Q7_WEIGHT_WFQ,     syncd_api_qos_shaperweight},
    {PORT_QOS,          PORT_QOS_SCHEDULE,          syncd_api_qos_shapermode},
    {TRUNK_PORT,        TRUNK_MEMBERS,              syncd_api_lag_config},
#ifdef AIR_SUPPORT_LACP
    {PORT_OPER_INFO,    PORT_LACP_STATE,            syncd_api_lag_lacp_config},
#endif
    {TRUNK_ALGORITHM,   ALGORITHM,                  syncd_api_lag_algo_config},
    {PORT_MIRROR_INFO,  PORT_MIRROR_ENABLE,         syncd_api_mir_setState},
    {PORT_MIRROR_INFO,  PORT_MIRROR_DEST_PORT,      syncd_api_mir_setSession},
    {PORT_MIRROR_INFO,  DB_ALL_FIELDS,              syncd_api_mir_setList},
#ifdef AIR_SUPPORT_IGMP_SNP
    {IGMP_SNP_INFO,     IGMP_SNP_ENABLE,            syncd_api_igmp_snp_enable},
#endif /* AIR_SUPPORT_IGMP_SNP */
    {JUMBO_FRAME_INFO,  JUMBO_FRAME_CFG,            syncd_api_port_jumbo},
    {VLAN_ENTRY,        DB_ALL_FIELDS,              syncd_api_vlan_entry},
#ifdef AIR_SUPPORT_VOICE_VLAN
    {VOICE_VLAN_INFO,   DB_ALL_FIELDS,              syncd_api_macvlan_setVoiceVlanInfo},
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    {SURVEI_VLAN_INFO,  DB_ALL_FIELDS,              syncd_api_macvlan_setSurveilVlanInfo},
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    {OUI_ENTRY,         DB_ALL_FIELDS,              syncd_api_macvlan_setMacOUIEntry},
#endif
    {QOS_INFO,          QOS_MODE,                   syncd_api_qos_mode},
    {QOS_INFO,          QOS_PRI_TO_QUEUE,           syncd_api_qos_queue},
    {QOS_INFO,          QOS_DSCP_TO_PRI,            syncd_api_qos_dscpPri},
    {STATIC_MAC_ENTRY,  DB_ALL_FIELDS,              syncd_api_l2_static_mac},
    {PORT_OPER_INFO,    PORT_LOOP_STATE,            syncd_api_vlan_matrix},
    {PORT_OPER_INFO,    PORT_MIB_COUNTER_CLEAR,     syncd_api_clear_mib_counter},
#ifdef AIR_SUPPORT_CABLE_DIAG
    {PORT_DIAG,         PORT_CABLE_SET,             syncd_api_cableDiagnostic},
#endif
    {L2_MC_ENTRY,       DB_ALL_FIELDS,              syncd_api_l2mc_addr},
    {SYSTEM,            DB_ALL_FIELDS,              syncd_api_system},
    {DYNAMIC_MAC_ADDRESS_ENTRY_CFG, ACTION_RESULT,  syncd_api_dynamic_mac_address_entry},
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    {STP_INFO,             STP_ENABLE,                syncd_api_stp_setBpduCtrl},
    {STP_PORT_OPER_INFO,   STP_PORT_OPER_STATE,       syncd_api_stp_setPortState},
    {STP_PORT_OPER_INFO,   STP_PORT_OPER_BLOCK,       syncd_api_stp_blockPort},
#endif
#ifdef AIR_SUPPORT_LLDPD
    {LLDP_INFO,         GLOBAL_ENABLE,              syncd_api_set_lldp_fwd_to_cpu},
#ifdef AIR_EN_CORAL
    {LLDP_PORT_INFO,    LLDP_ENABLE,              syncd_api_set_lldp_portMgmtFrameCfg},
#endif
#endif
#ifdef AIR_SUPPORT_MSTP
    {MSTP_PORT_OPER_INFO,   MSTP_INSTANCE_PORT_OPER_STATE, syncd_api_stp_setInstancePortState},
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_is_subscribed
 * PURPOSE:
 *      Subscribe each item to DB.
 *
 * INPUT:
 *      t_idx       --  table index
 *      f_idx       --  field index
 *      e_idx       --  entry index
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
BOOL_T
syncd_api_is_subscribed(
    UI8_T t_idx,
    UI8_T f_idx,
    UI16_T e_idx)
{
    UI8_T   i, tb_size;

    tb_size = sizeof(_tb_api_func)/sizeof(AIR_API_TABLE_T);
    for (i = 0; i < tb_size; i++)
    {
        if ((t_idx == _tb_api_func[i].t_id) && (f_idx == _tb_api_func[i].f_id))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/* FUNCTION NAME: syncd_api_subscribe
 * PURPOSE:
 *      Subscribe each item to DB.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *      method      --  M_SUBSCRIBE:    Subscribe all field in API function table
 *                      M_UNSUBSCRIBE:  Unsubscribe all field in API function table
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_subscribe(
    SYNCD_CFG_T *ptr_cfg,
    const UI8_T method)
{
    MW_ERROR_NO_T   rc;
    DB_MSG_T        *ptr_msg = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI8_T           i, tb_size;
    UI32_T          msg_size;

    if ((M_SUBSCRIBE != method) &&
        (M_UNSUBSCRIBE != method) )
    {
        return MW_E_BAD_PARAMETER;
    }
    MW_CHECK_PTR(ptr_cfg);

    tb_size = sizeof(_tb_api_func)/sizeof(AIR_API_TABLE_T);
    msg_size = DB_MSG_HEADER_SIZE + (DB_MSG_PAYLOAD_SIZE * tb_size);
    SYNCD_LOG_DEBUG("tb_size=%u\nmsg_size=%u\nsyncd_name=%s", tb_size, msg_size, SYNCD_NAME);

    rc = osapi_calloc(
            msg_size,
            SYNCD_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("allocate memory failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }
    SYNCD_LOG_DEBUG("ptr_msg=%p", ptr_msg);

    /* message */
    osapi_strncpy(ptr_msg->cq_name, SYNCD_MSG_QUEUE_NAME, 4);
    SYNCD_LOG_DEBUG("cq_name=%s", ptr_msg ->cq_name);
    ptr_msg ->method = method;
    ptr_msg ->type.count = tb_size;

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg ->ptr_payload);
    for (i = 0; i < tb_size; i++)
    {
        ptr_payload ->request.t_idx = _tb_api_func[i].t_id;
        ptr_payload ->request.f_idx = _tb_api_func[i].f_id;
        ptr_payload ->request.e_idx = 0;
        ptr_payload ->data_size = 0;
        SYNCD_LOG_DEBUG("ptr_payload=%p", ptr_payload);
        SYNCD_LOG_DEBUG("t_idx=%u", ptr_payload ->request.t_idx);
        SYNCD_LOG_DEBUG("f_idx=%u", ptr_payload ->request.f_idx);
        SYNCD_LOG_DEBUG("e_idx=%u", ptr_payload ->request.e_idx);
        SYNCD_LOG_DEBUG("data_size=%u", ptr_payload ->data_size);

        ptr_payload = (DB_PAYLOAD_T *)(((C8_T *)ptr_payload) + DB_MSG_PAYLOAD_SIZE);
    }

    SYNCD_LOG_DEBUG("Prepare to Send to DB");
    SYNCD_LOG_DUMP_DBMSG(ptr_msg, "Subscribe");
    rc = dbapi_sendRequesttoDb(msg_size, ptr_msg);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Subscribe to DB failed(%d)", rc);
        MW_FREE(ptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_process
 * PURPOSE:
 *      Process API function of reveiced DB notify message
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *      method      --  the method bitmap
 *      ptr_type    --  pointer to request type which includes table-id, field-id and entry-id
 *      ptr_data    --  pointer to message data
 *      data_size   --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_process(
    SYNCD_CFG_T *ptr_cfg,
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_type,
    const UI16_T data_size,
    const void * ptr_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T api_tb_idx;
    SYNCD_API_ARG_T api_arg;
    UI16_T port_idx;
    UI8_T f_num = 0;
    UI8_T tb_size;

    MW_CHECK_PTR(ptr_cfg);
    MW_CHECK_PTR(ptr_type);
    MW_PARAM_CHK((ptr_type ->t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    rc = dbapi_getFieldsNum((UI8_T)ptr_type ->t_idx, &f_num);
    if (MW_E_OK != rc)
    {
        return rc;
    }
    MW_PARAM_CHK((ptr_type ->f_idx >= f_num), MW_E_BAD_PARAMETER);
    if (data_size > 0)
    {
        MW_CHECK_PTR(ptr_data);
    }

    tb_size = sizeof(_tb_api_func)/sizeof(AIR_API_TABLE_T);
    for (api_tb_idx = 0; api_tb_idx < tb_size; api_tb_idx++)
    {
        if ((ptr_type ->t_idx == _tb_api_func[api_tb_idx].t_id)
            && (ptr_type ->f_idx == _tb_api_func[api_tb_idx].f_id))
        {
            break;
        }
    }

    if (api_tb_idx == tb_size)
    {
        SYNCD_LOG_ERROR("[%u/%u/-] is not support in syncd",
            ptr_type ->t_idx, ptr_type ->f_idx);
        return MW_E_NOT_SUPPORT;
    }
#ifndef AIR_LITE_MW
    if (NULL == ptr_cfg ->api_tid[ptr_type ->t_idx].ptr_fid)
    {
        SYNCD_LOG_ERROR("[%u/-/-] is not support in syncd", ptr_type ->t_idx);
        return MW_E_NOT_SUPPORT;
    }

    api_tb_idx = ptr_cfg ->api_tid[ptr_type ->t_idx].ptr_fid[(ptr_type ->f_idx)].fid_offset;
#endif

    SYNCD_LOG_DEBUG("api_tb_idx=%u", api_tb_idx);
    if ((SYNCD_NOT_SUPPORT == api_tb_idx)       ||
        (NULL == _tb_api_func[api_tb_idx].func)  )
    {
        SYNCD_LOG_ERROR("[%u/%u/-] is not support in syncd",
            ptr_type ->t_idx, ptr_type ->f_idx);
        return MW_E_NOT_SUPPORT;
    }

    /* Call API function */
    api_arg.method = method;
    api_arg.ptr_type = (DB_REQUEST_TYPE_T*)ptr_type;
    api_arg.data_size = data_size;
    api_arg.ptr_data = (void*)ptr_data;
    rc = _tb_api_func[api_tb_idx].func(&api_arg);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("[%u/%u/%u]Call API function failed(%d)",
            ptr_type ->t_idx, ptr_type ->f_idx, ptr_type ->e_idx, rc);
        return MW_E_OP_INCOMPLETE;
    }

    /* Update port admin status */
    if ((PORT_CFG_INFO == ptr_type ->t_idx) &&
        (PORT_ADMIN_STATUS == ptr_type ->f_idx) &&
        (ptr_data) )
    {
        if ((M_GET == method) &&
            (DB_ALL_ENTRIES == ptr_type ->e_idx) )
        {
            /* Receive M_GET from DB means the 1st notification */
            for (port_idx = 0; port_idx < PLAT_MAX_PORT_NUM; port_idx++)
            {
                SYNCD_LOG_DEBUG("Update Port[%u] admin state=%u", port_idx + 1, (*((UI8_T *)ptr_data + port_idx)));

                syncd_api_setPortAdmin(port_idx, (!!(*((UI8_T *)ptr_data + port_idx)))?TRUE:FALSE);
            }
        }
        else if ((M_UPDATE == method) &&
                (DB_ALL_ENTRIES != ptr_type ->e_idx) )
        {
            /* Receive M_UPDATE from DB means Update data */
            SYNCD_LOG_DEBUG("Update Port[%u] admin state=%u", ptr_type ->e_idx, (*((UI8_T *)ptr_data)));
            syncd_api_setPortAdmin(ptr_type ->e_idx - 1, (!!(*((UI8_T *)ptr_data)))?TRUE:FALSE);
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_init
 * PURPOSE:
 *      Initialize function index of API function table.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */

MW_ERROR_NO_T
syncd_api_init(
    SYNCD_CFG_T *ptr_cfg)
{
#ifndef AIR_LITE_MW
    MW_ERROR_NO_T rc;
    UI16_T tb_idx, tb_size;
    UI16_T last_tid = TABLES_LAST;
    UI16_T cur_fid = SYNCD_NOT_SUPPORT;
    SYNCD_API_FLD_T *ptr_fid;
    UI32_T mem_size;
    UI8_T f_num = 0;
#endif /* AIR_LITE_MW */

    MW_CHECK_PTR(ptr_cfg);

    /* Initialize lag info */
    syncd_api_lag_init();

    /* Initialize stp info */
    syncd_api_stp_init();

#ifndef AIR_LITE_MW
    /* Initialize api_tid */
    osapi_memset(ptr_cfg ->api_tid, 0, sizeof(SYNCD_API_TB_T)*TABLES_LAST);

    tb_size = (sizeof(_tb_api_func)/sizeof(AIR_API_TABLE_T));
    /* Calculate API table offset into global configuration */
    for (tb_idx = 0; tb_idx < tb_size; tb_idx++)
    {
        if ((SYS_INFO == _tb_api_func[tb_idx].t_id) ||
            (ACCOUNT_INFO == _tb_api_func[tb_idx].t_id) ||
            (SYS_OPER_INFO == _tb_api_func[tb_idx].t_id) ||
            (TABLES_LAST <= _tb_api_func[tb_idx].t_id))
        {
            /* Skip specific configuration and wrong configuration. */
            continue;
        }
        /* Try to find the 1st field ID of table */
        if (last_tid != _tb_api_func[tb_idx].t_id)
        {
            last_tid = _tb_api_func[tb_idx].t_id;
            if (NULL == ptr_cfg ->api_tid[last_tid].ptr_fid)
            {
                rc = dbapi_getFieldsNum((UI8_T)last_tid, &f_num);
                if( MW_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("get field amount failed(%d)", rc);
                    return MW_E_OTHERS;
                }
                mem_size = (f_num * sizeof(UI16_T));

                /* Allocate new array for this table id */
                rc = osapi_malloc(
                    mem_size,
                    SYNCD_NAME,
                    (void **)&ptr_fid);
                if (MW_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("allocate memory failed(%d)", rc);
                    return MW_E_NO_MEMORY;
                }
                /* set default value as 0xFF beacuse default is SYNCD_NOT_SUPPORT */
                osapi_memset(ptr_fid, 0xFF, mem_size);

                ptr_cfg ->api_tid[last_tid].ptr_fid = ptr_fid;
            }
        }
        cur_fid = _tb_api_func[tb_idx].f_id;
        ptr_cfg ->api_tid[last_tid].ptr_fid[cur_fid].fid_offset = tb_idx;
    }
#endif /* AIR_LITE_MW */

    return MW_E_OK;
}

/* FUNCTION NAME:   syncd_api_free
 * PURPOSE:
 *      Unsubscribe all field in API function table.
 *      Release all allocated memory in syncd queue.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_free(
    SYNCD_CFG_T *ptr_cfg)
{
#ifndef AIR_LITE_MW
    UI16_T tb_idx;
#endif /* AIR_LITE_MW */

    MW_CHECK_PTR(ptr_cfg);
    /* Unsubscribe */
    syncd_api_subscribe(ptr_cfg, M_UNSUBSCRIBE);

#ifndef AIR_LITE_MW
    /* Free API table */
    for (tb_idx = 0; tb_idx < TABLES_LAST; tb_idx++)
    {
        MW_FREE(ptr_cfg->api_tid[tb_idx].ptr_fid);
    }
#endif /* AIR_LITE_MW */
    return MW_E_OK;
}

