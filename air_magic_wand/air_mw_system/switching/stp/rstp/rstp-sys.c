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

/* FILE NAME:   rstp-sys.c
 * PURPOSE:
 *      Define RSTP main function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "stp.h"
#include <rstp-sys.h>
#include <openvswitch/vlog.h>
#include "ethernetif.h"
#include "ovs/lib/rstp-common.h"
#include <air_port.h>
#include <air_stp.h>
#include <air_ifmon.h>
#include <air_error.h>
#include <air_l2.h>
#include <../src/inc/aml/aml.h>
#include <sys_mgmt.h>
#include "mw_msg.h"
#include "mw_utils.h"
#include "syncd_api_lag.h"
#include "air_swc.h"

#include "stp_db.h"
#include "ovs/lib/rstp.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static struct rstp          *_ptr_rstp           = NULL;

/* LOCAL SUBPROMGRAM BODIES
*/

#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_rstp_db_trunk_port_oper_cost_update(
    UI32_T          trunk_id,
    const UI32_T    oper_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              p = 0;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              member = 0;

    member = stp_db_trunk_getMemberBmp(trunk_id);
    for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
    {
        if ((member & BIT(p)) > 0)
        {
            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, (p+1));
            rc = stp_db_update_msg(req_type, (void *)(&oper_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send port %d cost %d to db", p+1, oper_cost);
                break;
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_db_port_oper_cost_update(
    const UI32_T    port,
    const UI32_T    oper_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (TRUE == stp_db_getAutoCostState())
    {
        if (PLAT_MAX_PORT_NUM < port)
        {
            return _rstp_db_trunk_port_oper_cost_update(((port - 1) - PLAT_MAX_PORT_NUM), oper_cost);
        }

        if (MW_E_OK == rc)
        {
            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, port);
            rc = stp_db_update_msg(req_type, (void *)(&oper_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to update port %d cost %d data", port, oper_cost);
            }
        }
    }

    return rc;
}
#endif /* AIR_SUPPORT_STP_AUTO_COST */

static MW_ERROR_NO_T
_rstp_get_force_version_config(
    UI32_T  *ptr_frc_ver_cfg)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORCE_VERSION, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_frc_ver_cfg);

    return rc;
}

static MW_ERROR_NO_T
_rstp_get_port_priority_config(
    UI32_T  port,
    UI8_T   *ptr_pri)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = stp_db_trunk_getLowerPort(trunk_id);
    }
    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_PRIORITY, port_number);
    rc = stp_db_get_msg(req_type, ptr_pri);

    return rc;
}

#ifdef AIR_SUPPORT_RSTP_SECURITY
static MW_ERROR_NO_T
_rstp_get_port_security_param(
    UI32_T  port,
    DB_RSTP_PORT_SEC_T *sec_info)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_RSTP_PORT_SEC_T          info;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, DB_ALL_FIELDS, port_number);
    rc = stp_db_get_msg(req_type, &info);
    if (MW_E_OK == rc)
    {
        sec_info->bpdu_status = info.bpdu_status;
        sec_info->tc_status = info.tc_status;
        sec_info->tc_interval = info.tc_interval;
        sec_info->tc_threshold = info.tc_threshold;
        sec_info->root_status = info.root_status;
        sec_info->loop_status = info.loop_status;
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_bpdu_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %u bpdu guard status to %d", port_number, status);
            rstp_port_set_bpdu_guard_status(ptr_rstp_port, status);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_tc_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %u tc protect status to %d", port_number, status);
            rstp_port_set_tc_guard_status(ptr_rstp_port, status);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_root_guard_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
     {
         ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
         if (NULL != ptr_rstp_port)
         {
             STP_LOG_DBG("Set port %u root protect status to %d", port_number, status);
             rstp_port_set_root_guard_status(ptr_rstp_port, status);
         }
         else
         {
             STP_LOG_WARN("Get rstp port data with port %u failed!", port);
             rc = MW_E_BAD_PARAMETER;
         }
    }

     return rc;
}

static MW_ERROR_NO_T
_rstp_port_loop_guard_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %u loop protect status to %d", port_number, status);
            rstp_port_set_loop_guard_status(ptr_rstp_port, status);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_tc_interval_set(
    UI32_T  port,
    UI8_T   interval)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if ((RSTP_MIN_SEC_INTERVAL > interval) || (RSTP_MAX_SEC_INTERVAL < interval))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %u tc interval to %d", port_number, interval);
            rstp_port_set_tc_guard_interval(ptr_rstp_port, interval);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_tc_threshold_set(
    UI32_T  port,
    UI8_T   threshold)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if ((RSTP_MIN_SEC_THRESHOLD > threshold) || (RSTP_MAX_SEC_THRESHOLD < threshold))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %u tc threshold to %d", port_number, threshold);
            rstp_port_set_tc_guard_threshold(ptr_rstp_port, threshold);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

#endif

static MW_ERROR_NO_T
_rstp_construct(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    AIR_MAC_T           mac;
    UI32_T              unit = 0;
    rstp_identifier     mac_bridge = {0};
    UI32_T              port;
    UI32_T              port_number;
    UI32_T              p;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              trunk_id = 0;
    BOOL_T              is_lower_port = FALSE;
    UI8_T               link = 0;
    UI8_T               duplex = 0;
    UI8_T               is_p2p = 0;
    UI32_T              member = 0;

    memset(mac, 0, sizeof(AIR_MAC_T));
    if (AIR_E_OK == air_swc_getSystemMac(unit, mac))
    {
        osapi_memcpy(mac_bridge+2, mac, sizeof(AIR_MAC_T));

        _ptr_rstp = rstp_create(NULL, mac_bridge, NULL, NULL);
        if (NULL == _ptr_rstp)
        {
            return MW_E_NO_MEMORY;
        }

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            stp_db_get_port_status(port, &link);
            stp_db_get_port_duplex(port, &duplex);
            is_p2p = (AIR_PORT_DUPLEX_HALF == duplex) ? FALSE : TRUE;

            port_number = port;
            stp_db_getTrunkID(port, &trunk_id);
            is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                            stp_db_checkIsLowerPort(port, trunk_id) : FALSE;

            if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
            {
                port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                /*
                    From the point of view of determining the value of operPointToPointMAC (6.4.3), the MAC is
                    considered to be connected to a point-to-point LAN if any of the following conditions are true:
                    a) The MAC entity concerned contains a Link Aggregation sublayer, and the set of physical MACs
                    associated with the Aggregator are all aggregatable; or
                */
                is_p2p = TRUE;

                if (LAG_MODE_STATIC == stp_db_trunk_getMode(trunk_id))
                {
                    if (FALSE == link)
                    {
                        /*
                         * If the port does not link out,
                         * check the other members is link out or not
                         */
                        member = stp_db_trunk_getMemberBmp(trunk_id);
                        for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
                        {
                            if (((member & (1 << p)) > 0) && (p != (port-1)))
                            {
                                rc = stp_db_get_port_status((p+1), &link);
                                if (MW_E_OK == rc)
                                {
                                    if (TRUE == link)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    link = FALSE;
                    member = stp_db_trunk_getAggBmp(trunk_id);
                    for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
                    {
                        if (((member & (1 << p)) > 0) && (p != (port-1)))
                        {
                            link = TRUE;
                            break;
                        }
                    }
                }
            }
            else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
            {
                continue;
            }
            if (MW_E_OK == rc)
            {
                STP_LOG_DBG("Create rstp port %u data", port_number);
                ptr_rstp_port = rstp_add_port_w_port_number(_ptr_rstp, port_number);
                if (NULL == ptr_rstp_port)
                {
                    return MW_E_NO_MEMORY;
                }
                rstp_port_set_oper_point_to_point_mac(ptr_rstp_port, is_p2p);
                rstp_port_set_mac_operational(ptr_rstp_port,
                    !!(link));
            }
        }

        if (MW_E_OK == rc)
        {
            _ptr_rstp->running = TRUE;
        }
    }
    else
    {
        STP_LOG_ERR("Rstp enable, get sys-mac failed!");
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

static MW_ERROR_NO_T
_rstp_destruct(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port;
    UI32_T              port_number;
    UI32_T              trunk_id = 0;
    UI8_T               data = ROLE_DISABLED;

    if (NULL == _ptr_rstp)
    {
        return rc;
    }

    _ptr_rstp->running = FALSE;
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }
        port_number = port;
        STP_LOG_DBG("Delete rstp port %u data", port_number);
        rstp_del_port(_ptr_rstp, port_number);
    }
    for (trunk_id = 0 ; trunk_id < MAX_TRUNK_NUM ; trunk_id++)
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
        rstp_del_port(_ptr_rstp, port_number);
    }

    rc |= stp_db_port_update_allEntries(STP_PORT_OPER_INFO, STP_PORT_OPER_ROLE, &data);

    stp_db_clear_all_port_block();

    rstp_destroy(_ptr_rstp);
    _ptr_rstp = NULL;

    return rc;
}

static MW_ERROR_NO_T
_rstp_port_recover_forwarding(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T data = RSTP_FORWARDING;
    UI32_T i;

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        stp_port_state_data_update(i+1, data);
    }

    data = ROLE_DISABLED;
    rc |= stp_db_port_update_allEntries(STP_PORT_OPER_INFO, STP_PORT_OPER_ROLE, &data);

    return rc;
}

static MW_ERROR_NO_T
_rstp_load_configuration(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T              port = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI32_T              trunk_id = 0;
#endif
    UI32_T              force_version = 0;
    UI16_T              fwd_delay = 0;
    UI16_T              tx_hold_cnt = 0;
    UI16_T              max_age = 0;
    UI16_T              bridge_pri = 0;
    UI8_T               port_pri = 0;
    UI32_T              port_cost = 0;
    UI8_T               admin_edge = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI8_T               auto_enable = 0;
    UI32_T              lower_port = 0;
    UI8_T               link = 0;
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    UI8_T               status = false;

    STP_LOG_DBG("Load Configuration From DB");

    _rstp_get_force_version_config(&force_version);
    stp_db_get_forward_delay_config(&fwd_delay);
    stp_db_get_transmit_hold_count_config(&tx_hold_cnt);
    stp_db_get_max_age_config(&max_age);
    stp_db_get_bridge_priority_config(&bridge_pri);

    STP_LOG_DBG("force_version:%u", force_version);
    STP_LOG_DBG("forward_delay:%u", fwd_delay);
    STP_LOG_DBG("transmit_hold_count:%u", tx_hold_cnt);
    STP_LOG_DBG("max_age:%u", max_age);
    STP_LOG_DBG("bridge priority:%u", bridge_pri);

    rstp_force_version_set(force_version);
    rstp_forward_delay_set(fwd_delay);
    rstp_max_age_set(max_age);
    rstp_transmit_hold_count_set(tx_hold_cnt);
    rstp_priority_set(bridge_pri);

    for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
    {
        _rstp_get_port_priority_config(port, &port_pri);
        stp_db_get_port_cost_config(port, &port_cost);
#ifdef AIR_SUPPORT_STP_AUTO_COST
        stp_db_get_port_auto_enable_config(port, &auto_enable);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
        stp_db_get_port_admin_edge_config(port, &admin_edge);
        STP_LOG_DBG("port %u priority:%u", port, port_pri);
        STP_LOG_DBG("port %u cost:%u", port, port_cost);
#ifdef AIR_SUPPORT_STP_AUTO_COST
        STP_LOG_DBG("port %u auto_enable:%u", port, auto_enable);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
        STP_LOG_DBG("port %u admin_edge:%u", port, admin_edge);

        rstp_port_priority_set(port, port_pri);
#ifdef AIR_SUPPORT_STP_AUTO_COST
        if (TRUE == stp_db_getAutoCostState())
        {
            lower_port = port;
            stp_db_getTrunkID(port, &trunk_id);
            if (MAX_TRUNK_NUM > trunk_id)
            {
                lower_port = stp_db_trunk_getLowerPort(trunk_id);
                stp_db_get_port_auto_enable_config(lower_port, &auto_enable);
            }
            if (FALSE == auto_enable)
            {
                rstp_port_cost_set(port, port_cost);
            }
            else
            {
                stp_db_get_port_status(port, &link);
                if (TRUE == link)
                {
                    rstp_port_auto_cost_set(lower_port);
                }
            }
        }
        else
        {
            rstp_port_cost_set(port, port_cost);
        }
#else
        rstp_port_cost_set(port, port_cost);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
        stp_db_get_port_enable_status(port, &status);
        rstp_port_enable_status_set(port, status);

#ifdef AIR_SUPPORT_RSTP_SECURITY
        DB_RSTP_PORT_SEC_T sec_info;

        osapi_memset(&sec_info, 0, sizeof(DB_RSTP_PORT_SEC_T));
        sec_info.tc_interval = 5;
        _rstp_get_port_security_param(port, &sec_info);
        STP_LOG_DBG("port %u bpdu-guard %d tc-status %d tc-interval %d tc-threshold %d root-protect %d loop-protect",
            port, sec_info.bpdu_status, sec_info.tc_status, sec_info.tc_interval, sec_info.tc_threshold,
            sec_info.root_status, sec_info.loop_status);
        _rstp_port_bpdu_status_set(port, sec_info.bpdu_status);
        _rstp_port_tc_status_set(port, sec_info.tc_status);
        _rstp_port_tc_interval_set(port, sec_info.tc_interval);
        _rstp_port_tc_threshold_set(port, sec_info.tc_threshold);
        _rstp_port_root_guard_status_set(port, sec_info.root_status);
        _rstp_port_loop_guard_status_set(port, sec_info.loop_status);
#endif
        rstp_port_admin_edge_set(port, admin_edge);
    }
    STP_LOG_DBG("Load Configuration From DB Complete");

    return rc;
}

static void
_rstp_trunk_member_port_del(
    UI32_T port_bmp)
{
    UI32_T port;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if ((port_bmp & BIT(port)) > 0)
        {
            STP_LOG_DBG("Delete trunk member port %d data", port+1);
            rstp_del_port(_ptr_rstp, port+1);
        }
    }
}

static void
_rstp_trunk_member_port_add(
    UI32_T port_bmp)
{
    UI8_T link = 0;
    UI8_T duplex = 0;
    UI8_T is_p2p = 0;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T port;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if ((port_bmp & BIT(port)) > 0)
        {
            stp_db_get_port_status(port+1, &link);
            stp_db_get_port_duplex(port+1, &duplex);
            is_p2p = (AIR_PORT_DUPLEX_HALF == duplex) ? FALSE : TRUE;

            rstp_del_port(_ptr_rstp, port+1);
            STP_LOG_DBG("Create trunk member port %d data", port+1);
            ptr_rstp_port = rstp_add_port_w_port_number(_ptr_rstp, port+1);
            if (NULL == ptr_rstp_port)
            {
                STP_LOG_ERR("Fail to create trunk member port %d data", port+1);
                return;
            }
            stp_port_state_data_update(port+1, RSTP_DISCARDING);
            stp_db_port_role_update(port+1, ROLE_DISABLED);
            rstp_port_set_oper_point_to_point_mac(ptr_rstp_port, is_p2p);
            rstp_port_set_mac_operational(ptr_rstp_port,
                !!(link));
        }
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: rstp_enable_set
 * PURPOSE:
 *      enable rstp
 *
 * INPUT:
 *      enable          -  status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_enable_set(
    UI8_T   enable)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (TRUE == enable)
    {
        rc = _rstp_construct();
        if (MW_E_NO_MEMORY == rc)
        {
            _rstp_destruct();
            _rstp_port_recover_forwarding();
            return MW_E_OK;
        }
        _rstp_load_configuration();
    }
    else
    {
        _rstp_destruct();
        _rstp_port_recover_forwarding();
    }

    STP_LOG_DBG("Rstp status: %s", enable ? "Enable" : "Disable");

    return rc;
}

/* FUNCTION NAME: rstp_force_version_set
 * PURPOSE:
 *      switch to rstp
 *
 * INPUT:
 *      force_version          -  rstp/stp
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_force_version_set(
    UI32_T   force_version)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (NULL != _ptr_rstp)
    {
        if (FPV_STP_COMPATIBILITY == force_version ||
            FPV_DEFAULT == force_version)
        {
            stp_db_clear_all_port_block();
            STP_LOG_DBG("Force version to: %s", (FPV_DEFAULT == force_version)?"rstp":"stp");
            rstp_set_bridge_force_protocol_version(_ptr_rstp, force_version);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_forward_delay_set
 * PURPOSE:
 *      Setting forward delay
 *
 * INPUT:
 *      forward_delay          -  fwd delay time
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_forward_delay_set(
    UI16_T  forward_delay)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (NULL != _ptr_rstp)
    {
        if (stp_check_range(forward_delay,
                                RSTP_MIN_BRIDGE_FORWARD_DELAY,
                                RSTP_MAX_BRIDGE_FORWARD_DELAY))
        {
            STP_LOG_DBG("Forward delay %d", forward_delay);
            rstp_set_bridge_forward_delay(_ptr_rstp, forward_delay);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_max_age_set
 * PURPOSE:
 *      Setting max age
 *
 * INPUT:
 *      max_age          -  max age time
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_max_age_set(
    UI16_T   max_age)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (NULL != _ptr_rstp)
    {
        if (stp_check_range(max_age,
                                RSTP_MIN_BRIDGE_MAX_AGE,
                                RSTP_MAX_BRIDGE_MAX_AGE))
        {
            STP_LOG_DBG("Max age %d", max_age);
            rstp_set_bridge_max_age(_ptr_rstp, max_age);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_transmit_hold_count_set
 * PURPOSE:
 *      Setting tx hold time
 *
 * INPUT:
 *      tx_hold_cnt          -  tx hold time
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_transmit_hold_count_set(
    UI16_T  tx_hold_cnt)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (NULL != _ptr_rstp)
    {
        if (stp_check_range(tx_hold_cnt,
                                RSTP_MIN_TRANSMIT_HOLD_COUNT,
                                RSTP_MAX_TRANSMIT_HOLD_COUNT))
        {
            STP_LOG_DBG("Transmit hold count %d", tx_hold_cnt);
            rstp_set_bridge_transmit_hold_count(_ptr_rstp, tx_hold_cnt);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_priority_set
 * PURPOSE:
 *      Setting bridge priority
 *
 * INPUT:
 *      priority          -  bridge priority
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_priority_set(
    UI16_T  priority)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (NULL != _ptr_rstp)
    {
        if (stp_check_range(priority, RSTP_MIN_PRIORITY, RSTP_MAX_PRIORITY) &&
            (priority % RSTP_PRIORITY_STEP) == 0)
        {
            STP_LOG_DBG("Bridge priority %u", priority);
            rstp_set_bridge_priority(_ptr_rstp, priority);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: rstp_port_compute_oper_cost
 * PURPOSE:
 *      Calc port cost
 *
 * INPUT:
 *      port          -  port id
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto cost support status
 *      ptr_oper_cost            - oper cost
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_compute_oper_cost(
    UI32_T  port,
    UI8_T  *ptr_auto_cost_support,
    UI32_T  *ptr_oper_cost)
{
    UI32_T  trunk_id = 0;
    UI8_T   auto_enable = 0, speed_enum = 0;
    UI32_T  speed_int = 0, config_cost = 0, auto_cost = 0;

    stp_db_get_port_cost_config(port, &config_cost);
    stp_db_get_port_auto_enable_config(port, &auto_enable);

    if (TRUE == stp_db_getAutoCostState())
    {
        if (FALSE == auto_enable)
        {
            *ptr_oper_cost = config_cost;
        }
        else
        {
            if (port > PLAT_MAX_PORT_NUM)
            {
                trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
                stp_trunk_compute_oper_cost(trunk_id, &auto_cost);
            }
            else
            {
                stp_db_get_port_speed(port, &speed_enum);
                stp_speed_enum_to_integer(speed_enum, &speed_int);
                auto_cost = rstp_convert_speed_to_cost(speed_int);
            }
            *ptr_oper_cost = auto_cost;
        }
    }
    else
    {
        *ptr_oper_cost = config_cost;
    }

    if (NULL != ptr_auto_cost_support)
    {
        *ptr_auto_cost_support = stp_db_getAutoCostState();
    }

    STP_LOG_DBG("port=%u, auto_enable=%u, auto_cost=%u, oper_cost=%u",
        port, auto_enable, auto_cost, *ptr_oper_cost);

    return MW_E_OK;
}

/* FUNCTION NAME: rstp_auto_cost_support_set
 * PURPOSE:
 *      Set auto cost
 *
 * INPUT:
 *      enable          -  TRUE/FALSE
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_auto_cost_support_set(
    UI8_T   enable)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          port_number = 0, p = 0, trunk_id = 0, oper_cost = 0;
    BOOL_T              is_lower_port = FALSE;
    struct rstp_port    *ptr_port = NULL;

    if (stp_db_getAutoCostState() != enable)
    {
        stp_db_setAutoCostState(enable);
        if (NULL != _ptr_rstp)
        {
            /* if _rstp_auto_cost_support is 1,
             *      change the operational cost to dynamic cost or static cost
             *      accorting to auto cost mode configuration.
             * else
             *      change the operational cost to static cost.
             */
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, p)
            {
                if (PLAT_CPU_PORT == p)
                {
                    continue;
                }
                port_number = p;
                stp_db_getTrunkID(p, &trunk_id);
                is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                                stp_db_checkIsLowerPort(p, trunk_id) : FALSE;
                if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
                {
                    port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                }
                rstp_port_compute_oper_cost(port_number, NULL, &oper_cost);
                ptr_port = rstp_get_port(_ptr_rstp, port_number);
                if (NULL != ptr_port)
                {
                    if (TRUE == stp_db_getAutoCostState())
                    {
                        _rstp_db_port_oper_cost_update(port_number, oper_cost);
                    }
                    STP_LOG_DBG("Set port %d path cost %u", port_number, oper_cost);
                    rstp_port_set_path_cost(ptr_port, oper_cost);
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_port_auto_cost_set
 * PURPOSE:
 *      Set port auto cost
 *
 * INPUT:
 *      port          -  port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_auto_cost_set(
    UI32_T  port)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port_number = port, trunk_id;
    UI32_T              speed_int, auto_cost;
    UI32_T              oper_cost;
    UI8_T               speed_enum;
    struct rstp_port    *ptr_port;

    stp_db_get_port_speed(port, &speed_enum);
    stp_speed_enum_to_integer(speed_enum, &speed_int);
    auto_cost = rstp_convert_speed_to_cost(speed_int);

    stp_db_getTrunkID(port, &trunk_id);
    if (MAX_TRUNK_NUM > trunk_id)
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
        stp_trunk_compute_oper_cost(trunk_id, &oper_cost);
    }
    else
    {
        oper_cost = auto_cost;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_port)
        {
            _rstp_db_port_oper_cost_update(port_number, oper_cost);
            STP_LOG_DBG("Set port %d path cost %u", port_number, oper_cost);
            rstp_port_set_path_cost(ptr_port, oper_cost);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_set_port_auto_cost_config
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port          -  port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_set_port_auto_cost_config(
    UI32_T  port)
{
    UI32_T              auto_cost;
    UI32_T              trunk_id, lower_port = port;
    UI32_T              portNum = port;
    struct rstp_port    *ptr_port;

    stp_db_getTrunkID(port, &trunk_id);
    if (MAX_TRUNK_NUM > trunk_id)
    {
        lower_port = stp_db_trunk_getLowerPort(trunk_id);
        if (lower_port != port)
        {
            return MW_E_OK;
        }
        portNum = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }

    stp_db_get_port_cost_config(lower_port, &auto_cost);
    if (NULL != _ptr_rstp)
    {
        ptr_port = rstp_get_port(_ptr_rstp, portNum);
        if (NULL != ptr_port)
        {
            STP_LOG_DBG("Set port %d path cost %u", portNum, auto_cost);
            rstp_port_set_path_cost(ptr_port, auto_cost);
        }
    }
    _rstp_db_port_oper_cost_update(portNum, auto_cost);

    return MW_E_OK;
}

/* FUNCTION NAME: rstp_port_oper_cost_set
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port          -  port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
void
rstp_port_oper_cost_set(
    UI32_T   port_number)
{
    struct rstp_port    *ptr_port = NULL;
    UI32_T              oper_cost;

    if (NULL == _ptr_rstp)
    {
        return;
    }
    ptr_port = rstp_get_port(_ptr_rstp, port_number);
    if (NULL != ptr_port)
    {
        rstp_port_compute_oper_cost(port_number, NULL, &oper_cost);
        /* Send oper cost to DB */
        _rstp_db_port_oper_cost_update(port_number, oper_cost);
        STP_LOG_DBG("Set port %d path cost %u", port_number, oper_cost);
        /* Set oper cost to port instance */
        rstp_port_set_path_cost(ptr_port, oper_cost);
    }
}
#endif

/* FUNCTION NAME: rstp_port_priority_set
 * PURPOSE:
 *      Set port priority config
 *
 * INPUT:
 *      port          -  port id
 *      priority      -  priority
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_priority_set(
    UI32_T  port,
    UI8_T   priority)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        if (stp_check_range(priority, RSTP_MIN_PORT_PRIORITY, RSTP_MAX_PORT_PRIORITY) &&
            (priority % RSTP_STEP_PORT_PRIORITY) == 0)
        {
            ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
            if (NULL != ptr_rstp_port)
            {
                STP_LOG_DBG("Set port %d priority %u", port_number, priority);
                rstp_port_set_priority(ptr_rstp_port, priority);
            }
            else
            {
                STP_LOG_WARN("Get rstp port data with port %u failed!", port);
                rc = MW_E_BAD_PARAMETER;
            }
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_port_cost_set
 * PURPOSE:
 *      Set port cost
 *
 * INPUT:
 *      port          -  port id
 *      cost          -  port cost
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_cost_set(
    UI32_T  port,
    UI32_T  cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %d path cost %u", port_number, cost);
            rstp_port_set_path_cost(ptr_rstp_port, cost);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_port_enable_status_set
 * PURPOSE:
 *      Set port status
 *
 * INPUT:
 *      port          -  port id
 *      status        -  status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_enable_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }
    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %d status %u", port_number, status);
            rstp_port_set_enable_status(ptr_rstp_port, status);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_port_admin_edge_set
 * PURPOSE:
 *      Set port admin edge
 *
 * INPUT:
 *      port          -  port id
 *      admin_edge    -  admin edge
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_admin_edge_set(
    UI32_T  port,
    UI8_T   admin_edge)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %d admin edge %u", port_number, admin_edge);
            rstp_port_set_admin_edge(ptr_rstp_port, admin_edge);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_port_mcheck_set
 * PURPOSE:
 *      Set port mcheck
 *
 * INPUT:
 *      port          -  port id
 *      perform       -  mcheck status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_mcheck_set(
    UI32_T  port,
    UI8_T   perform)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_rstp_port = NULL;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    if (NULL != _ptr_rstp)
    {
        ptr_rstp_port = rstp_get_port(_ptr_rstp, port_number);
        if (NULL != ptr_rstp_port)
        {
            STP_LOG_DBG("Set port %d mcheck %u", port_number, perform);
            rstp_port_set_mcheck(ptr_rstp_port, perform);
        }
        else
        {
            STP_LOG_WARN("Get rstp port data with port %u failed!", port);
            rc = MW_E_BAD_PARAMETER;
        }
    }

    return rc;
}

/* FUNCTION NAME: rstp_trunk_update
 * PURPOSE:
 *      Set trunk info
 *
 * INPUT:
 *      e_idx          -  entry-id
 *      ptr_data       -  db msg data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_trunk_update(
    UI16_T e_idx,
    void   *ptr_data)
{
    UI32_T              i, old_trunk_member;
    DB_TRUNK_PORT_T     members[MAX_TRUNK_NUM];
    UI32_T              port_number = 0;
    struct rstp_port    *ptr_port = NULL;
    UI8_T               link = 0;

    if (NULL == _ptr_rstp)
    {
        return;
    }

    if (DB_ALL_ENTRIES == e_idx)
    {
        osapi_memcpy(&members, ptr_data, sizeof(DB_TRUNK_PORT_T) * MAX_TRUNK_NUM);
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            /* PROCESS OLD MEMBER */
            old_trunk_member = stp_db_trunk_getMemberBmp(i);
            stp_db_trunk_setMemberBmp(i, 0);
            stp_db_trunk_setAggBmp(i, 0);
            stp_db_trunk_setMode(i, members[i].members.mode);
            _rstp_trunk_member_port_add(old_trunk_member);

            /* PROCESS NEW MEMBER */
            _rstp_trunk_member_port_del(members[i].members.member_bmp);
            stp_db_trunk_setMemberBmp(i, members[i].members.member_bmp);

            port_number = STP_TRUNK_ID_TO_PORT_NUMBER(i);
            /* PROCESS TRUNK PORT */
            if (0 != stp_db_trunk_getMemberBmp(i))
            {
                rstp_del_port(_ptr_rstp, port_number);
                STP_LOG_DBG("Create trunk %d port data", i);
                ptr_port = rstp_add_port_w_port_number(_ptr_rstp, port_number);
                if (NULL != ptr_port)
                {
                    stp_db_port_role_update(port_number, ROLE_DISABLED);
                    link = stp_trunk_check_link_status(i);
                    rstp_port_set_mac_operational(ptr_port,
                        !!(link));
                }
            }
            else
            {
                /* DEL TRUNK PORT DATA */
                rstp_del_port(_ptr_rstp, port_number);
            }
        }
    }
    else
    {
        osapi_memcpy(&members, ptr_data, sizeof(DB_TRUNK_PORT_T));

        /* PROCESS OLD MEMBER */
        old_trunk_member = stp_db_trunk_getMemberBmp(e_idx-1);
        stp_db_trunk_setMemberBmp(e_idx-1, 0);
        stp_db_trunk_setAggBmp(e_idx-1, 0);
        stp_db_trunk_setMode(e_idx-1, members[0].members.mode);
        _rstp_trunk_member_port_add(old_trunk_member);

        /* PROCESS NEW MEMBER */
        _rstp_trunk_member_port_del(members[0].members.member_bmp);
        stp_db_trunk_setMemberBmp(e_idx-1, members[0].members.member_bmp);

        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(e_idx-1);
        /* PROCESS TRUNK PORT */
        if (0 != stp_db_trunk_getMemberBmp(e_idx-1))
        {
            rstp_del_port(_ptr_rstp, port_number);
            STP_LOG_DBG("Create trunk %d port data", e_idx-1);
            ptr_port = rstp_add_port_w_port_number(_ptr_rstp, port_number);
            if (NULL != ptr_port)
            {
                stp_db_port_role_update(port_number, ROLE_DISABLED);
                link = stp_trunk_check_link_status(e_idx-1);
                rstp_port_set_mac_operational(ptr_port,
                    !!(link));
            }
        }
        else
        {
            /* DEL TRUNK PORT DATA */
            rstp_del_port(_ptr_rstp, port_number);
        }
    }
    _rstp_load_configuration();
    rstp_run_stm(_ptr_rstp);
}

/* FUNCTION NAME: rstp_port_link_status_set
 * PURPOSE:
 *      Set port link status
 *
 * INPUT:
 *      port         -  port id
 *      is_p2p       -  is_p2p
 *      link         -  link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_link_status_set(
    UI32_T  port,
    UI8_T   is_p2p,
    UI8_T   link)
{
    struct rstp_port *ptr_port;

    if (NULL == _ptr_rstp)
    {
        return;
    }

    ptr_port = rstp_get_port(_ptr_rstp, port);
    if (NULL != ptr_port)
    {
        STP_LOG_DBG("Set port %d point-to-point %d, mac-operational %d", port, is_p2p, link);
        rstp_port_set_oper_point_to_point_mac(ptr_port, is_p2p);
        rstp_port_set_mac_operational(ptr_port, link);
    }
}

/* FUNCTION NAME: rstp_trunk_link_status_set
 * PURPOSE:
 *      Set trunk link status
 *
 * INPUT:
 *      port         -  port id
 *      link         -  link status
 *      mode         -  trunk mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_trunk_link_status_set(
    UI32_T  port,
    UI8_T   link,
    UI8_T   mode)
{
    struct rstp_port *ptr_port;

    if (NULL == _ptr_rstp)
    {
        return;
    }

    ptr_port = rstp_get_port(_ptr_rstp, port);
    if (NULL != ptr_port)
    {
        STP_LOG_DBG("Set trunk port %d point-to-point true", port);
        rstp_port_set_oper_point_to_point_mac(ptr_port, TRUE);
        if (LAG_MODE_STATIC == mode)
        {
            STP_LOG_DBG("Set trunk port mac-operational %d", port, link);
            rstp_port_set_mac_operational(ptr_port, link);
        }
    }
}

/* FUNCTION NAME: rstp_port_mac_operational_set
 * PURPOSE:
 *      Set port mac-operational status
 *
 * INPUT:
 *      port         -  port id
 *      link         -  link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_mac_operational_set(
    UI32_T  port,
    UI8_T   link)
{
    struct rstp_port *ptr_port;

    if (NULL == _ptr_rstp)
    {
        return;
    }

    ptr_port = rstp_get_port(_ptr_rstp, port);
    if (NULL != ptr_port)
    {
        STP_LOG_DBG("Set port mac-operational %d", port, link);
        rstp_port_set_mac_operational(ptr_port, link);
    }
}

/* FUNCTION NAME: rstp_port_p2p_mac_set
 * PURPOSE:
 *      Set port point-to-point mac status
 *
 * INPUT:
 *      port         -  port id
 *      is_p2p       -  is_p2p
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_p2p_mac_set(
    UI32_T  port,
    UI8_T   is_p2p)
{
    struct rstp_port *ptr_port;

    if (NULL == _ptr_rstp)
    {
        return;
    }

    ptr_port = rstp_get_port(_ptr_rstp, port);
    if (NULL != ptr_port)
    {
        STP_LOG_DBG("Set port point-to-point %d", port, is_p2p);
        rstp_port_set_oper_point_to_point_mac(ptr_port, is_p2p);
    }
}

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_port_bpdu_status_set
 * PURPOSE:
 *      Set port bpdu protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_bpdu_status_set(
    UI32_T  port,
    UI8_T   status)
{
    return _rstp_port_bpdu_status_set(port, status);
}

/* FUNCTION NAME: rstp_port_tc_status_set
 * PURPOSE:
 *      Set port tc protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_status_set(
    UI32_T  port,
    UI8_T   status)
{
    return _rstp_port_tc_status_set(port, status);
}

/* FUNCTION NAME: rstp_port_root_guard_status_set
 * PURPOSE:
 *      Set port root protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_root_guard_status_set(
    UI32_T  port,
    UI8_T   status)
{
    return _rstp_port_root_guard_status_set(port, status);
}

/* FUNCTION NAME: rstp_port_loop_guard_status_set
 * PURPOSE:
 *      Set port loop protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_loop_guard_status_set(
    UI32_T  port,
    UI8_T   status)
{
    return _rstp_port_loop_guard_status_set(port, status);
}

/* FUNCTION NAME: rstp_port_tc_interval_set
 * PURPOSE:
 *      Set port tc protect interval
 *
 * INPUT:
 *      port         -  port id
 *      interval     -  interval time
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_interval_set(
    UI32_T  port,
    UI8_T   interval)
{
    return _rstp_port_tc_interval_set(port, interval);
}

/* FUNCTION NAME: rstp_port_tc_threshold_set
 * PURPOSE:
 *      Set port tc protect threshold
 *
 * INPUT:
 *      port         -  port id
 *      threshold    -  threshold
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_threshold_set(
    UI32_T  port,
    UI8_T   threshold)
{
    return _rstp_port_tc_threshold_set(port, threshold);
}
#endif

/* FUNCTION NAME: rstp_pkt_rx
 * PURPOSE:
 *      rstp bpdu handle
 *
 * INPUT:
 *      port_num         -  port id
 *      ptr_data         -  bpdu data
 *      bpdu_size        -  bpdu size
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_pkt_rx(
    UI32_T  port_num,
    void    *ptr_data,
    UI32_T  bpdu_size)
{
    struct rstp_port    *ptr_port = NULL;

    ptr_port = rstp_get_port(_ptr_rstp, (UI16_T)port_num);
    if (NULL != ptr_port)
    {
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
        if (ptr_port->port_enabled && ptr_port->bpdu_status)
        {
            stp_set_port_bpdu_down(port_num, true);
            return;
        }
#endif
        rstp_port_received_bpdu(ptr_port, ptr_data, bpdu_size);
    }
}

/* FUNCTION NAME: rstp_data_init
 * PURPOSE:
 *      rstp data init
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_data_init(
    void)
{
    rstp_init();
}

#ifdef STP_DEBUG
/* FUNCTION NAME: rstp_cmd_showOvsInfo
 * PURPOSE:
 *      Show info
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_cmd_showOvsInfo(
    void)
{
    struct rstp_port                    *ptr_port = NULL;
    UI16_T                              port = 0;
    UI16_T                              priority;
    enum rstp_force_protocol_version    version;
    UI16_T                              max_age;
    UI16_T                              forward_delay;
    UI16_T                              tx_hold_cnt;
    UI8_T                               port_priority;
    UI32_T                              port_cost;
    enum rstp_state                     state;
    enum rstp_port_role                 role;
    UI8_T                               admin_edge;

    MW_CMD_OUTPUT("Global State: %s\n", stp_db_getStpGlobalState() ? "Enable" : "Disable");

    if ((TRUE == stp_db_getStpGlobalState()) && (NULL != _ptr_rstp))
    {
        priority = rstp_get_bridge_priority(_ptr_rstp);
        version = rstp_get_bridge_force_protocol_version(_ptr_rstp);
        max_age = rstp_get_bridge_max_age(_ptr_rstp);
        forward_delay = rstp_get_bridge_forward_delay(_ptr_rstp);
        tx_hold_cnt = rstp_get_bridge_transmit_hold_count(_ptr_rstp);

        MW_CMD_OUTPUT("Force Version: %s\n", version ? "RSTP" : "STP");
        MW_CMD_OUTPUT("Forward Delay: %u\n", forward_delay);
        MW_CMD_OUTPUT("Max Age: %u\n", max_age);
        MW_CMD_OUTPUT("Transmit Hold Count: %u\n", tx_hold_cnt);
        MW_CMD_OUTPUT("Bridge Priority: %u\n", priority);
        MW_CMD_OUTPUT("\n");
        MW_CMD_OUTPUT("%5s  %8s  %7s  %9s  %7s  %10s\n", "Port", "Priority", "Edge", "Cost", "State", "Role");
        MW_CMD_OUTPUT("-----------------------------------------------\n");
        for (port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
        {
            ptr_port = rstp_get_port(_ptr_rstp, port);
            if (NULL != ptr_port)
            {
                port_priority = rstp_port_get_priority(ptr_port);
                port_cost = rstp_port_get_path_cost(ptr_port);
                state = rstp_port_get_state(ptr_port);
                role = rstp_port_get_role(ptr_port);
                admin_edge = rstp_port_get_admin_edge(ptr_port);

                MW_CMD_OUTPUT("%5u  %8u  %7s  %9u  %7s  %10s\n",
                    port,
                    port_priority,
                    admin_edge ? "Enable" : "Disable",
                    port_cost,
                    (state == RSTP_FORWARDING ? "Forward" :
                        state == RSTP_DISCARDING ? "Discard" :
                        state == RSTP_LEARNING ? "Learn" : "Disable"),
                    (role == ROLE_ROOT ? "Root" :
                        role == ROLE_DESIGNATED ? "Designated" :
                        role == ROLE_ALTERNATE ? "Alternate" :
                        role == ROLE_BACKUP ? "Backup" : "Disable"));
            }
        }
        MW_CMD_OUTPUT("\n%5s  %8s  %9s  %7s  %10s\n", "Trunk", "Priority", "Cost", "State", "Role");
        MW_CMD_OUTPUT("-----------------------------------------------\n");
        for (port = (PLAT_MAX_PORT_NUM + 1) ; port < (PLAT_MAX_PORT_NUM + MAX_TRUNK_NUM) ; port++)
        {
            ptr_port = rstp_get_port(_ptr_rstp, port);
            if (NULL != ptr_port)
            {
                port_priority = rstp_port_get_priority(ptr_port);
                port_cost = rstp_port_get_path_cost(ptr_port);
                state = rstp_port_get_state(ptr_port);
                role = rstp_port_get_role(ptr_port);
                admin_edge = rstp_port_get_admin_edge(ptr_port);

                MW_CMD_OUTPUT("%5u  %8u  %7s  %9u  %7s  %10s\n",
                    (port - PLAT_MAX_PORT_NUM),
                    port_priority,
                    admin_edge ? "Enable" : "Disable",
                    port_cost,
                    (state == RSTP_FORWARDING ? "Forward" :
                        state == RSTP_DISCARDING ? "Discard" :
                        state == RSTP_LEARNING ? "Learn" : "Disable"),
                    (role == ROLE_ROOT ? "Root" :
                        role == ROLE_DESIGNATED ? "Designated" :
                        role == ROLE_ALTERNATE ? "Alternate" :
                        role == ROLE_BACKUP ? "Backup" : "Disable"));
            }
        }
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
rstp_print_priority_vector(
    struct rstp_priority_vector vector,
    BOOL_T                      cmd_flag)
{
    RSTP_INFO_PRINT(cmd_flag, "root_bridge_id: "RSTP_ID_FMT"\n", RSTP_ID_ARGS(vector.root_bridge_id));
    RSTP_INFO_PRINT(cmd_flag, "root_path_cost: %u\n", (UI32_T)vector.root_path_cost);
    RSTP_INFO_PRINT(cmd_flag, "designated_bridge_id: "RSTP_ID_FMT"\n", RSTP_ID_ARGS(vector.designated_bridge_id));
    RSTP_INFO_PRINT(cmd_flag, "designated_port_id: 0x%04x\n", vector.designated_port_id);
    RSTP_INFO_PRINT(cmd_flag, "bridge_port_id: 0x%04x\n", vector.bridge_port_id);

    return MW_E_OK;
}

MW_ERROR_NO_T
rstp_print_port_info(
    struct rstp_port    *ptr_port,
    BOOL_T              cmd_flag)
{
    RSTP_INFO_PRINT(cmd_flag, "[Port %u]\n", ptr_port->port_number);
    RSTP_INFO_PRINT(cmd_flag, "------------------------------------------------------------\n");
    RSTP_INFO_PRINT(cmd_flag, "== designated_priority_vector ==\n");
    rstp_print_priority_vector(ptr_port->designated_priority_vector, cmd_flag);

    RSTP_INFO_PRINT(cmd_flag, "\n== msg_priority ==\n");
    rstp_print_priority_vector(ptr_port->msg_priority, cmd_flag);

    RSTP_INFO_PRINT(cmd_flag, "\n== port_priority ==\n");
    rstp_print_priority_vector(ptr_port->port_priority, cmd_flag);

    RSTP_INFO_PRINT(cmd_flag, "\n");

    RSTP_INFO_PRINT(cmd_flag, "mac_operational: %u\n", ptr_port->mac_operational);
    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "is_administrative_bridge_port: %u\n", ptr_port->is_administrative_bridge_port);
    RSTP_INFO_PRINT(cmd_flag, "oper_point_to_point_mac: %u\n", ptr_port->oper_point_to_point_mac);
    RSTP_INFO_PRINT(cmd_flag, "admin_point_to_point_mac: %u\n", ptr_port->admin_point_to_point_mac);
    RSTP_INFO_PRINT(cmd_flag, "admin_edge: %u\n", ptr_port->admin_edge);
    RSTP_INFO_PRINT(cmd_flag, "auto_edge: %u\n", ptr_port->auto_edge);
    RSTP_INFO_PRINT(cmd_flag, "edge_delay_while: %u\n", ptr_port->edge_delay_while);
    RSTP_INFO_PRINT(cmd_flag, "fd_while: %u\n", ptr_port->fd_while);
    RSTP_INFO_PRINT(cmd_flag, "mdelay_while: %u\n", ptr_port->mdelay_while);
    RSTP_INFO_PRINT(cmd_flag, "rb_while: %u\n", ptr_port->rb_while);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_info_while: %u\n", ptr_port->rcvd_info_while);
    RSTP_INFO_PRINT(cmd_flag, "rr_while: %u\n", ptr_port->rr_while);
    RSTP_INFO_PRINT(cmd_flag, "tc_while: %u\n", ptr_port->tc_while);
    RSTP_INFO_PRINT(cmd_flag, "agree: %u\n", ptr_port->agree);
    RSTP_INFO_PRINT(cmd_flag, "agreed: %u\n", ptr_port->agreed);
    RSTP_INFO_PRINT(cmd_flag, "disputed: %u\n", ptr_port->disputed);
    RSTP_INFO_PRINT(cmd_flag, "fdb_flush: %u\n", ptr_port->fdb_flush);
    RSTP_INFO_PRINT(cmd_flag, "forward: %u\n", ptr_port->forward);
    RSTP_INFO_PRINT(cmd_flag, "forwarding: %u\n", ptr_port->forwarding);
    RSTP_INFO_PRINT(cmd_flag, "info_is: %u\n", ptr_port->info_is);
    RSTP_INFO_PRINT(cmd_flag, "learn: %u\n", ptr_port->learn);
    RSTP_INFO_PRINT(cmd_flag, "learning: %u\n", ptr_port->learning);
    RSTP_INFO_PRINT(cmd_flag, "mcheck: %u\n", ptr_port->mcheck);
    RSTP_INFO_PRINT(cmd_flag, "new_info: %u\n", ptr_port->new_info);
    RSTP_INFO_PRINT(cmd_flag, "oper_edge: %u\n", ptr_port->oper_edge);
    RSTP_INFO_PRINT(cmd_flag, "port_enabled: %u\n", ptr_port->port_enabled);
    RSTP_INFO_PRINT(cmd_flag, "proposed: %u\n", ptr_port->proposed);
    RSTP_INFO_PRINT(cmd_flag, "proposing: %u\n", ptr_port->proposing);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_bpdu: %u\n", ptr_port->rcvd_bpdu);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_msg: %u\n", ptr_port->rcvd_msg);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_rstp: %u\n", ptr_port->rcvd_rstp);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_stp: %u\n", ptr_port->rcvd_stp);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_tc: %u\n", ptr_port->rcvd_tc);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_tc_ack: %u\n", ptr_port->rcvd_tc_ack);

    RSTP_INFO_PRINT(cmd_flag, "rcvd_tcn: %u\n", ptr_port->rcvd_tcn);
    RSTP_INFO_PRINT(cmd_flag, "re_root: %u\n", ptr_port->re_root);
    RSTP_INFO_PRINT(cmd_flag, "reselect: %u\n", ptr_port->reselect);
    RSTP_INFO_PRINT(cmd_flag, "role: %s\n", rstp_port_role_name(ptr_port->role));
    RSTP_INFO_PRINT(cmd_flag, "selected: %u\n", ptr_port->selected);
    RSTP_INFO_PRINT(cmd_flag, "selected_role: %s\n", rstp_port_role_name(ptr_port->selected_role));
    RSTP_INFO_PRINT(cmd_flag, "send_rstp: %u\n", ptr_port->send_rstp);
    RSTP_INFO_PRINT(cmd_flag, "sync: %u\n", ptr_port->sync);
    RSTP_INFO_PRINT(cmd_flag, "synced: %u\n", ptr_port->synced);
    RSTP_INFO_PRINT(cmd_flag, "tc_ack: %u\n", ptr_port->tc_ack);
    RSTP_INFO_PRINT(cmd_flag, "tc_prop: %u\n", ptr_port->tc_prop);
    RSTP_INFO_PRINT(cmd_flag, "tick: %u\n", ptr_port->tick);
    RSTP_INFO_PRINT(cmd_flag, "tx_count: %u\n", ptr_port->tx_count);
    RSTP_INFO_PRINT(cmd_flag, "updt_info: %u\n", ptr_port->rcvd_tc_ack);

    RSTP_INFO_PRINT(cmd_flag, "rx_rstp_bpdu_cnt: %u\n", (UI32_T)ptr_port->rx_rstp_bpdu_cnt);
    RSTP_INFO_PRINT(cmd_flag, "error_count: %u\n", (UI32_T)ptr_port->error_count);
    RSTP_INFO_PRINT(cmd_flag, "uptime: %u\n", (UI32_T)ptr_port->uptime);
    RSTP_INFO_PRINT(cmd_flag, "rstp_state: %s\n", (ptr_port->rstp_state == RSTP_FORWARDING ? "Forward" :
                    ptr_port->rstp_state == RSTP_DISCARDING ? "Discard" :
                    ptr_port->rstp_state == RSTP_LEARNING ? "Learn" : "Disable"));
    RSTP_INFO_PRINT(cmd_flag, "state_changed: %u\n", ptr_port->state_changed);

    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "designated_times.forward_delay: %u\n", ptr_port->designated_times.forward_delay);
    RSTP_INFO_PRINT(cmd_flag, "designated_times.hello_time: %u\n", ptr_port->designated_times.hello_time);
    RSTP_INFO_PRINT(cmd_flag, "designated_times.max_age: %u\n", ptr_port->designated_times.max_age);
    RSTP_INFO_PRINT(cmd_flag, "designated_times.message_age: %u\n", ptr_port->designated_times.message_age);

    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "msg_times.forward_delay: %u\n", ptr_port->msg_times.forward_delay);
    RSTP_INFO_PRINT(cmd_flag, "msg_times.hello_time: %u\n", ptr_port->msg_times.hello_time);
    RSTP_INFO_PRINT(cmd_flag, "msg_times.max_age: %u\n", ptr_port->msg_times.max_age);
    RSTP_INFO_PRINT(cmd_flag, "msg_times.message_age: %u\n", ptr_port->msg_times.message_age);

    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "port_times.forward_delay: %u\n", ptr_port->port_times.forward_delay);
    RSTP_INFO_PRINT(cmd_flag, "port_times.hello_time: %u\n", ptr_port->port_times.hello_time);
    RSTP_INFO_PRINT(cmd_flag, "port_times.max_age: %u\n", ptr_port->port_times.max_age);
    RSTP_INFO_PRINT(cmd_flag, "port_times.message_age: %u\n", ptr_port->port_times.message_age);

    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "bridge.port_role_selection_sm_state: %u\n", ptr_port->rstp->port_role_selection_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_receive_sm_state: %u\n", ptr_port->port_receive_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_protocol_migration_sm_state: %u\n", ptr_port->port_protocol_migration_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "bridge_detection_sm_state: %u\n", ptr_port->bridge_detection_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_transmit_sm_state: %u\n", ptr_port->port_transmit_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_information_sm_state: %u\n", ptr_port->port_information_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_role_transition_sm_state: %u\n", ptr_port->port_role_transition_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "port_state_transition_sm_state: %u\n", ptr_port->port_state_transition_sm_state);
    RSTP_INFO_PRINT(cmd_flag, "topology_change_sm_state: %u\n", ptr_port->topology_change_sm_state);

#ifdef AIR_SUPPORT_RSTP_SECURITY
    RSTP_INFO_PRINT(cmd_flag, "bpdu_status: %u\n", ptr_port->bpdu_status);
    RSTP_INFO_PRINT(cmd_flag, "loop_status: %u\n", ptr_port->loop_status);
    RSTP_INFO_PRINT(cmd_flag, "root_status: %u\n", ptr_port->root_status);
    RSTP_INFO_PRINT(cmd_flag, "tc_status: %u\n", ptr_port->tc_status);
    RSTP_INFO_PRINT(cmd_flag, "tc_interval: %u\n", ptr_port->tc_interval);
    RSTP_INFO_PRINT(cmd_flag, "tc_threshold: %u\n", ptr_port->tc_threshold);
    RSTP_INFO_PRINT(cmd_flag, "rcvd_tc_bpdu: %u\n", (UI32_T)ptr_port->rcvd_tc_bpdu);
    RSTP_INFO_PRINT(cmd_flag, "tc_guard_while: %u\n", ptr_port->tc_guard_while);
    RSTP_INFO_PRINT(cmd_flag, "is_interval_rcvd: %u\n", ptr_port->is_interval_rcvd);
    RSTP_INFO_PRINT(cmd_flag, "is_loop_inc: %u\n", ptr_port->is_loop_inc);
    RSTP_INFO_PRINT(cmd_flag, "is_root_inc: %u\n", ptr_port->is_root_inc);
    RSTP_INFO_PRINT(cmd_flag, "loop_inc_block_while: %u\n", ptr_port->loop_inc_block_while);
    RSTP_INFO_PRINT(cmd_flag, "root_inc_block_while: %u\n", ptr_port->root_inc_block_while);
#endif
    RSTP_INFO_PRINT(cmd_flag, "\n");
    return MW_E_OK;
}

MW_ERROR_NO_T
rstp_print_bridge_info(
    BOOL_T              cmd_flag)
{
    RSTP_INFO_PRINT(cmd_flag, "\n== bridge_priority ==\n");
    rstp_print_priority_vector(_ptr_rstp->bridge_priority, cmd_flag);

    RSTP_INFO_PRINT(cmd_flag, "\n== root_priority ==\n");
    rstp_print_priority_vector(_ptr_rstp->root_priority, cmd_flag);

    RSTP_INFO_PRINT(cmd_flag, "\n");
    RSTP_INFO_PRINT(cmd_flag, "root_times.forward_delay: %u\n", _ptr_rstp->root_times.forward_delay);
    RSTP_INFO_PRINT(cmd_flag, "root_times.hello_time: %u\n", _ptr_rstp->root_times.hello_time);
    RSTP_INFO_PRINT(cmd_flag, "root_times.max_age: %u\n", _ptr_rstp->root_times.max_age);
    RSTP_INFO_PRINT(cmd_flag, "root_times.message_age: %u\n", _ptr_rstp->root_times.message_age);
    return MW_E_OK;
}

MW_ERROR_NO_T
rstp_cmd_showPortParameters(
    UI32_T  port)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    struct rstp_port    *ptr_port = NULL;

    if (TRUE == stp_db_getStpGlobalState())
    {
        if (NULL == _ptr_rstp)
        {
            return rc;
        }
        ptr_port = rstp_get_port(_ptr_rstp, port);
        if (NULL != ptr_port)
        {
            rstp_print_port_info(ptr_port, TRUE);
        }
    }
    else
    {
        rc = MW_E_OP_INVALID;
    }

    return rc;
}

MW_ERROR_NO_T
rstp_cmd_showBridgeParameters(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    if (TRUE == stp_db_getStpGlobalState() && (NULL != _ptr_rstp))
    {
        rstp_print_bridge_info(TRUE);
    }
    else
    {
        rc = MW_E_OP_INVALID;
    }

    return rc;
}
#endif /* STP_DEBUG */

void
rstp_tick_timers_start(
    void)
{
    rstp_tick_timers(_ptr_rstp);
}
