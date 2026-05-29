/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_SUPPORT_MSTP

/* FILE NAME:   mstp_sys.c
 * PURPOSE:
 *      Define MSTP main function.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "ethernetif.h"
#include <sys_mgmt.h>
#include "mw_msg.h"
#include "mw_utils.h"
#include "db_api.h"
#include "syncd_api_lag.h"

#include "stp.h"
#include "mstp_sys.h"
#include "stp_db.h"
#include "vlan_utils.h"

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
static MW_ERROR_NO_T
_mstp_port_load_configuration(
    UI32_T   port);

static void
_mstp_db_port_role_init_update(
    const UI32_T    port,
    const UI8_T     role);

/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T                    _vlan_mode = VLAN_1Q_ENABLE;
static MST_INSTANCE_VLAN_ENTRY  _mstp_instance_vlan[MAX_VLAN_ENTRY_NUM];

/* LOCAL SUBPROMGRAM BODIES
 */

inline static UI32_T
_mstp_convert_speed_to_cost(unsigned int speed)
{
    UI32_T value;

    value = speed >= 10000000 ? 2 /* 10 Tb/s. */
          : speed >= 1000000 ? 20 /* 1 Tb/s. */
          : speed >= 100000 ? 200 /* 100 Gb/s. */
          : speed >= 10000 ? 2000 /* 10 Gb/s. */
          : speed >= 1000 ? 20000 /* 1 Gb/s. */
          : speed >= 100 ? 200000 /* 100 Mb/s. */
          : speed >= 10 ? 2000000 /* 10 Mb/s. */
          : speed >= 1 ? 20000000 /* 1 Mb/s. */
          : 200000000; /* <=100 Kbps. */

    return value;
}

static MW_ERROR_NO_T
_mstp_get_vlan_entry(
    UI32_T  vlanIdx,
    void   *ptr_entry)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(VLAN_ENTRY, DB_ALL_FIELDS, vlanIdx);
    rc = stp_db_get_msg(req_type, ptr_entry);

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_mstp_get_port_cost_config(
    UI32_T  port,
    UI32_T  *ptr_cost)
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

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_COST, port_number);
    rc = stp_db_get_msg(req_type, ptr_cost);

    return rc;
}
#endif

static MW_ERROR_NO_T
_mstp_region_config_get(
    UI16_T *ptr_revision,
    UI8_T  *ptr_name)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    DB_REQUEST_TYPE_T       req_type;
    DB_MSTP_REGION_CONFIG_T info;

    req_type = stp_dbmsg_init_reqtype(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, &info);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    osapi_memcpy(ptr_revision, &info.revision, sizeof(UI16_T));
    osapi_memcpy(ptr_name, info.region_name, MAX_MSTP_REGION_NAME_SIZE);

    return MW_E_OK;
}


static MW_ERROR_NO_T
_mstp_update_port_priority_config(
    UI32_T  port)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    UI8_T                       priority[MAX_MSTP_INSTANCE_NUM];
    UI16_T                      insIdx;
    BOOL_T                      is_lower_port = FALSE;

    port_number = port;
    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = stp_db_trunk_getLowerPort(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_PRIORITY, port_number);
    rc = stp_db_get_msg(req_type, priority);
    if (MW_E_OK == rc)
    {
        for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
        {
            mstp_setPortPriority(port, insIdx, priority[insIdx]);
        }
    }
    else
    {
        STP_LOG_ERR("Get port %d priority config failed!", port);
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_port_cost_set(
    UI32_T  port)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    BOOL_T              is_lower_port = FALSE;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              cost[MAX_MSTP_INSTANCE_NUM];
    UI16_T              insIdx;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = stp_db_trunk_getLowerPort(trunk_id);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }
    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_COST, port_number);
    rc = stp_db_get_msg(req_type, cost);
    if (MW_E_OK == rc)
    {
        if (MAX_TRUNK_NUM > trunk_id)
        {
            port = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
        }
        for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
        {
            mstp_setPortPathCost(port, insIdx, cost[insIdx]);
        }
    }
    else
    {
        STP_LOG_ERR("Get port %d cost config failed!", port);
    }

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_mstp_db_trunk_port_oper_cost_update(
    UI32_T          trunk_id,
    UI32_T          *ptr_cost)
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
            req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_COST, (p+1));
            rc = stp_db_update_msg(req_type, (void *)(ptr_cost), (sizeof(UI32_T) * MAX_MSTP_INSTANCE_NUM));

            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, (p+1));
            rc |= stp_db_update_msg(req_type, (void *)(ptr_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send trunk member cost to db");
                break;
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_db_port_oper_cost_update(
    UI32_T    port,
    UI32_T    *ptr_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (TRUE == stp_db_getAutoCostState())
    {
        if (PLAT_MAX_PORT_NUM < port)
        {
            return _mstp_db_trunk_port_oper_cost_update(((port - 1) - PLAT_MAX_PORT_NUM), ptr_cost);
        }

        if (MW_E_OK == rc)
        {
            req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_COST, port);
            rc = stp_db_update_msg(req_type, (void *)(ptr_cost), (sizeof(UI32_T) * MAX_MSTP_INSTANCE_NUM));

            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, port);
            rc |= stp_db_update_msg(req_type, (void *)(ptr_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send port %d instance cost to db", port);
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_db_update_trunk_port_oper_cost(
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
_mstp_db_update_port_oper_cost(
    const UI32_T    port,
    const UI32_T    oper_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (TRUE == stp_db_getAutoCostState())
    {
        if (PLAT_MAX_PORT_NUM < port)
        {
            return _mstp_db_update_trunk_port_oper_cost(((port - 1) - PLAT_MAX_PORT_NUM), oper_cost);
        }

        if (MW_E_OK == rc)
        {
            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, port);
            rc = stp_db_update_msg(req_type, (void *)(&oper_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send port %d cost %d to db", port, oper_cost);
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_port_auto_cost_set(
    UI32_T  port,
    UI32_T  auto_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    UI32_T              cost = 0;
    UI8_T               is_lower_port = 0;
    UI32_T              oper_cost[MAX_MSTP_INSTANCE_NUM];
    UI16_T              insIdx;

    stp_db_getTrunkID(port, &trunk_id);
    is_lower_port = (MAX_TRUNK_NUM > trunk_id) ?
                    stp_db_checkIsLowerPort(port, trunk_id) : FALSE;
    if ((MAX_TRUNK_NUM > trunk_id) && (TRUE == is_lower_port))
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
        stp_trunk_compute_oper_cost(trunk_id, &cost);
    }
    else if ((MAX_TRUNK_NUM > trunk_id) && (FALSE == is_lower_port))
    {
        return rc;
    }
    else
    {
        cost = auto_cost;
    }

    for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
    {
        oper_cost[insIdx] = cost;
    }

    _mstp_db_port_oper_cost_update(port_number, oper_cost);
    _mstp_db_update_port_oper_cost(port_number, oper_cost[0]);
    for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
    {
        mstp_setPortPathCost(port_number, insIdx, oper_cost[insIdx]);
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_port_compute_oper_cost(
    UI32_T  port,
    UI8_T  *ptr_auto_cost_support,
    UI32_T  *ptr_oper_cost)
{
    UI32_T  i = 0, trunk_id = 0;
    UI8_T   auto_enable = 0, speed_enum = 0;
    UI32_T  speed_int = 0, auto_cost = 0;

    stp_db_get_port_auto_enable_config(port, &auto_enable);

    if (TRUE == stp_db_getAutoCostState())
    {
        if (FALSE == auto_enable)
        {
            _mstp_get_port_cost_config(port, ptr_oper_cost);
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
                auto_cost = _mstp_convert_speed_to_cost(speed_int);
            }
            for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
            {
                ptr_oper_cost[i] = auto_cost;
            }
        }
    }
    else
    {
        _mstp_get_port_cost_config(port, ptr_oper_cost);
    }

    if (NULL != ptr_auto_cost_support)
    {
        *ptr_auto_cost_support = stp_db_getAutoCostState();
    }

    STP_LOG_DBG("port=%u, auto_enable=%u, auto_cost=%u, oper_cost=%u",
        port, auto_enable, auto_cost, *ptr_oper_cost);

    return MW_E_OK;
}

#endif /* AIR_SUPPORT_STP_AUTO_COST */

static inline void
_mstp_port_all_instance_state_update(
    UI32_T port,
    UI8_T  state)
{
    UI32_T    i;

    for (i = 0; i < MSTP_INSTANCE_MAX_NUM; i++)
    {
        mstp_port_instance_state_update(i, port, state);
    }
}

inline static UI8_T
_isMstpPortBelongToInstance(
    UI32_T  port,
    UI8_T   insIdex)
{
    UI32_T    i;
    UI16_T    idx;

    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        idx = (_mstp_instance_vlan[i].instance & 0xf000) >> 12;
        if (idx != insIdex)
        {
            continue;
        }

        if (BIT_CHK(_mstp_instance_vlan[i].memBmp, port))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void
_mstp_update_all_vlan_port(
    void)
{
    MW_ERROR_NO_T       ret;
    MSTP_INS_PARAM_T    data;
    UI32_T              i, port, trunk_id;
    UI16_T              idx;
    BOOL_T              is_del = FALSE;
    VLAN_ENTRY_INFO_T   vlan_entry;
    DB_REQUEST_TYPE_T   req_type;
    UI8_T               is_load[MAX_PORT_NUM] = {0};

    osapi_memset(is_load, 0, sizeof(UI8_T) * MAX_PORT_NUM);
    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (0 == _mstp_instance_vlan[i].vid)
        {
            continue;
        }

        idx = (_mstp_instance_vlan[i].instance & 0xf000) >> 12;
        ret = _mstp_get_vlan_entry(i+1, &vlan_entry);
        if(MW_E_OK != ret)
        {
            return;
        }
        if (vlan_entry.vlan_fid == idx)
        {
            continue;
        }

        vlan_entry.vlan_fid = idx;
        req_type = stp_dbmsg_init_reqtype(VLAN_ENTRY, DB_ALL_FIELDS, i+1);
        ret = stp_db_update_msg(req_type, (void *)(&vlan_entry), sizeof(VLAN_ENTRY_INFO_T));
        if (MW_E_OK != ret)
        {
            STP_LOG_ERR("Fail to update vlan %d entry to db", vlan_entry.vlan_id);
            return;
        }
    }

    for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
    {
        for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
        {
            if (0 == _mstp_instance_vlan[i].vid)
            {
                continue;
            }

            if (BIT_CHK(_mstp_instance_vlan[i].memBmp, port))
            {
                //add
                data.insIdx = _mstp_instance_vlan[i].instance;
                data.portNum = port;
                stp_db_getTrunkID(port, &trunk_id);
                if (trunk_id < MAX_TRUNK_NUM)
                {
                    data.portNum = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                }
                ret = mstp_add_instance_port(&data);
                if (MW_E_ENTRY_EXISTS == ret)
                {
                    continue;
                }
                if (MW_E_OK != ret)
                {
                    STP_LOG_ERR("Mstp add port %d instance %d failed!", port, (data.insIdx & 0xfff));
                    return;
                }
                is_load[port-1] = TRUE;
            }
        }
    }

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        for (idx = 0; idx < MAX_MSTP_INSTANCE_NUM; idx++)
        {
            if (FALSE == _isMstpPortBelongToInstance(port+1, idx))
            {
                data.insIdx = idx;
                data.portNum = port+1;
                mstp_del_instance_port(&data);
            }
        }

        if (TRUE == is_load[port])
        {
            _mstp_port_load_configuration(port+1);
        }
    }

    /* SKIP CIST */
    for (i = 1; i < MAX_MSTP_INSTANCE_NUM; i++)
    {
        is_del = TRUE;
        for (idx = 0; idx < MAX_VLAN_ENTRY_NUM; idx++)
        {
            if (i == ((_mstp_instance_vlan[idx].instance & 0xf000) >> 12))
            {
                is_del = FALSE;
                break;
            }
        }

        if (TRUE == is_del)
        {
            data.insIdx = ((i << 12) & 0xf000);
            data.portNum = 0;
            mstp_del_instance(&data);
        }
    }
}

static MW_ERROR_NO_T
_mstp_construct(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port;
    UI8_T               link = 0;
    UI8_T               duplex = 0;
    UI8_T               is_p2p = 0;
    UI32_T              port_number;
    BOOL_T              is_lower_port = FALSE;
    UI32_T              trunk_id = 0;
    UI32_T              p;
    UI32_T              member = 0;

    rc = mstp_create(PLAT_MAX_PORT_NUM, MAX_TRUNK_NUM, (MSTP_INSTANCE_MAX_NUM-1),
                      MAX_VLAN_ENTRY_NUM);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Mstp create instance failed!");
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
            STP_LOG_DBG("Create port %u instance data", port_number);
            rc = mstp_add_port(port_number);
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Create port %u instance data failed!", port_number);
                return MW_E_NO_MEMORY;
            }
            mstp_set_port_oper_point_to_point_mac(port_number, is_p2p);
            mstp_set_port_macOperational(port_number, !!(link));
        }
    }

    _mstp_update_all_vlan_port();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_mstp_port_recover_forwarding(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T data = STP_FORWARDING;
    UI32_T i;
    VLAN_ENTRY_INFO_T vlan_entry;
    DB_REQUEST_TYPE_T req_type;

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        _mstp_port_all_instance_state_update(i+1, data);
    }

    data = STP_ROLE_DISABLED;
    rc |= stp_db_port_update_allEntries(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_ROLE, &data);

    data = STP_ROLE_DISABLED;
    rc |= stp_db_port_update_allEntries(STP_PORT_OPER_INFO, STP_PORT_OPER_ROLE, &data);

    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (0 == _mstp_instance_vlan[i].vid)
        {
            continue;
        }

        rc = _mstp_get_vlan_entry(i+1, &vlan_entry);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        if (vlan_entry.vlan_fid == 0)
        {
            continue;
        }

        vlan_entry.vlan_fid = 0;
        req_type = stp_dbmsg_init_reqtype(VLAN_ENTRY, DB_ALL_FIELDS, i+1);
        rc = stp_db_update_msg(req_type, (void *)(&vlan_entry), sizeof(VLAN_ENTRY_INFO_T));
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to send all vlan entry to db");
            return rc;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_bridge_priority_set(
    void)
{
    MW_ERROR_NO_T          rc = MW_E_OK;
    UI16_T                 insIdx;
    MSTP_INSTANCE_CONFIG_T info;

    if (TRUE == stp_db_getStpGlobalState())
    {
        rc = mstp_db_get_instance_vlan_config(&info);
        if (MW_E_OK != rc)
        {
            return rc;
        }
        if (stp_check_range(info.priority[0], STP_MIN_PRIORITY, STP_MAX_PRIORITY) &&
            (info.priority[0] % STP_PRIORITY_STEP) == 0)
        {
            mstp_setBridgePriority(0, info.priority[0]);
        }
        else
        {
            rc = MW_E_BAD_PARAMETER;
        }
        for (insIdx = 1; insIdx < MSTP_INSTANCE_MAX_NUM; insIdx++)
        {
            if (0 == info.instance[insIdx])
            {
                continue;
            }

            if (stp_check_range(info.priority[insIdx], STP_MIN_PRIORITY, STP_MAX_PRIORITY) &&
                (info.priority[insIdx] % STP_PRIORITY_STEP) == 0)
            {
                mstp_setBridgePriority(info.instance[insIdx], info.priority[insIdx]);
            }
            else
            {
                rc = MW_E_BAD_PARAMETER;
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_mstp_port_load_configuration(
    UI32_T   port)
{
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI8_T               auto_enable = 0, speed_enum = 0;
    UI32_T              speed_int = 0, auto_cost = 0;
    UI32_T              lower_port = 0;
    UI8_T               link = 0;
    UI32_T              trunk_id = 0;
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    UI8_T               status = false;
    UI8_T               admin_edge = 0;

    _mstp_update_port_priority_config(port);
#ifdef AIR_SUPPORT_STP_AUTO_COST
    stp_db_get_port_auto_enable_config(port, &auto_enable);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    stp_db_get_port_admin_edge_config(port, &admin_edge);
#ifdef AIR_SUPPORT_STP_AUTO_COST
    STP_LOG_DBG("port %u auto_enable:%u", port, auto_enable);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    STP_LOG_DBG("port %u admin_edge:%u", port, admin_edge);

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
            _mstp_port_cost_set(port);
        }
        else
        {
            stp_db_get_port_status(port, &link);
            if (TRUE == link)
            {
                stp_db_get_port_speed(port, &speed_enum);
                stp_speed_enum_to_integer(speed_enum, &speed_int);
                auto_cost = _mstp_convert_speed_to_cost(speed_int);
                _mstp_port_auto_cost_set(lower_port, auto_cost);
            }
        }
    }
    else
    {
        _mstp_port_cost_set(port);
    }
#else
    _mstp_port_cost_set(port);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    stp_db_get_port_enable_status(port, &status);
    mstp_port_enable_status_set(port, status);

    mstp_port_admin_edge_set(port, admin_edge);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_mstp_load_configuration(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T              port = 0;
    UI16_T              fwd_delay = 0;
    UI16_T              tx_hold_cnt = 0;
    UI16_T              max_age = 0;
    UI16_T              revision = 0;
    UI8_T               name[MAX_MSTP_REGION_NAME_SIZE+1];

    STP_LOG_DBG("Load Configuration From DB");

    stp_db_get_forward_delay_config(&fwd_delay);
    stp_db_get_transmit_hold_count_config(&tx_hold_cnt);
    stp_db_get_max_age_config(&max_age);
    _mstp_region_config_get(&revision, name);

    STP_LOG_DBG("forward_delay:%u", fwd_delay);
    STP_LOG_DBG("transmit_hold_count:%u", tx_hold_cnt);
    STP_LOG_DBG("max_age:%u", max_age);
    STP_LOG_DBG("revision:%d, region name %s", revision, name);

    mstp_forward_delay_set(fwd_delay);
    mstp_max_age_set(max_age);
    mstp_transmit_hold_count_set(tx_hold_cnt);
    _mstp_bridge_priority_set();
    mstp_setRegionConfig(revision, name);

    mstp_setInstanceMappingVlan(_mstp_instance_vlan);

    for(port = 1; port <= PLAT_MAX_PORT_NUM; port++)
    {
        _mstp_port_load_configuration(port);
    }
    STP_LOG_DBG("Load Configuration From DB Complete");

    return rc;
}

static void
_mstp_trunk_member_port_add(
    UI32_T port_bmp)
{
    MW_ERROR_NO_T   rc;
    UI8_T           link = 0;
    UI8_T           duplex = 0;
    UI8_T           is_p2p = 0;
    UI32_T          port;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if ((port_bmp & BIT(port)) > 0)
        {
            stp_db_get_port_status(port+1, &link);
            stp_db_get_port_duplex(port+1, &duplex);
            is_p2p = (AIR_PORT_DUPLEX_HALF == duplex) ? FALSE : TRUE;

            mstp_del_port(port+1);
            rc = mstp_add_port(port+1);
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to add trunk member(%d) data", (port+1));
                return;
            }
            _mstp_port_all_instance_state_update(port+1, STP_DISCARDING);
            _mstp_db_port_role_init_update(port+1, STP_ROLE_DISABLED);
            stp_db_port_role_update(port+1, STP_ROLE_DISABLED);

            mstp_set_port_oper_point_to_point_mac(port+1, is_p2p);
            mstp_set_port_macOperational(port+1, !!(link));
        }
    }
}

static void
_mstp_trunk_member_port_del(
    UI32_T port_bmp)
{
    UI32_T port;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if ((port_bmp & BIT(port)) > 0)
        {
            mstp_del_port(port+1);
        }
    }
}

inline static void
_mstpGetVidByFid(
    UI16_T  fid,
    UI16_T  *ptr_vlanid,
    UI32_T  *ptr_num)
{
    UI32_T    i, idx = 0;
    UI16_T    insIdx;

    *ptr_num = 0;
    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (0 == _mstp_instance_vlan[i].vid)
        {
            continue;
        }
        insIdx = (_mstp_instance_vlan[i].instance & 0xf000) >> 12;
        if (fid == insIdx)
        {
            ptr_vlanid[idx++] = _mstp_instance_vlan[i].vid;
        }
    }

    *ptr_num = idx;
}

inline static void
_mstpGetVid(
    UI8_T   *ptr_vlanlist,
    UI16_T  *ptr_vlanid,
    UI32_T  *ptr_num)
{
    BOOL_T      dash = FALSE, valid = FALSE;
    UI32_T      idx, value = 0, vid_idx = 0;
    UI32_T      i;
    UI16_T      start_vid = 0, end_vid = 0;

    *ptr_num = 0;
    /* e.g. 12,14,16-47 , len = 11 */
    for (idx = 0; idx < osapi_strlen((C8_T *)ptr_vlanlist); idx++)
    {
        if (('0' <= ptr_vlanlist[idx]) && ('9' >= ptr_vlanlist[idx]))
        {
            value = (value * 10) + (ptr_vlanlist[idx] - '0');
            valid = TRUE;
        }
        else if (',' == ptr_vlanlist[idx])
        {
            if (TRUE != valid)
            {
                break;
            }

            /* 1: value = 12, dash = 0
             * 2: value = 14, dash = 0
             */
            if (FALSE == dash)
            {
                if (MAX_VLAN_ENTRY_NUM <= vid_idx)
                {
                    break;
                }
                start_vid = value;
                end_vid = value;
                ptr_vlanid[vid_idx++] = value;
            }
            /* 4: value = 47, start = 16, end = 47, dash = 0 */
            else
            {
                end_vid = value;
                for (i = start_vid; (i <= end_vid) && (vid_idx < MAX_VLAN_ENTRY_NUM); i++)
                {
                    ptr_vlanid[vid_idx++] = i;
                }
            }
            value = 0;
            dash = FALSE;
            valid = FALSE;
        }
        /* 3: value = 16, start = 16, end = 0, dash = 1 */
        else if ('-' == ptr_vlanlist[idx])
        {
            if ((TRUE != valid) || (TRUE == dash))
            {
                break;
            }

            start_vid = value;
            value = 0;
            dash = TRUE;
            valid = FALSE;
        }
        else
        {
            break;/* Error: not a dec number */
        }
    }

    if (TRUE != valid)
    {
        return;
    }

    if (FALSE == dash)
    {
        ptr_vlanid[vid_idx++] = value;
    }
    else
    {
        end_vid = value;
        for (i = start_vid; (i <= end_vid) && (vid_idx < MAX_VLAN_ENTRY_NUM); i++)
        {
            ptr_vlanid[vid_idx++] = i;
        }
    }
    *ptr_num = vid_idx;
}

inline static MST_INSTANCE_VLAN_ENTRY*
_mstpGetMstTableByVid(
    UI16_T vid)
{
    UI16_T i;
    MST_INSTANCE_VLAN_ENTRY *entry;

    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        entry = &_mstp_instance_vlan[i];

        if (entry->vid == vid)
        {
            return entry;
        }
    }

    return NULL;
}

inline static void
_mstpClearMstTableInstance(
    void)
{
    UI16_T i;

    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        _mstp_instance_vlan[i].instance = 0;
    }
}

static int
_cmp(
    const void *a,
    const void *b)
{
    return (*(UI16_T *)a) - (*(UI16_T *)b);
}

inline static int
_mstpGetMstVlanStrByArray(
    UI32_T   num,
    UI16_T   *ptr_array,
    UI8_T    *ptr_out)
{
    UI16_T i = 0;
    UI16_T startVid, endVid;
    UI32_T len = 0;

    if (0 == num)
    {
        return -1;
    }

    qsort(ptr_array, num, sizeof(UI16_T), _cmp);

    while (i < num)
    {
        startVid = ptr_array[i];
        endVid = startVid;

        while (((i + 1) < num) && (ptr_array[i+1] == (ptr_array[i] + 1)))
        {
            endVid = ptr_array[++i];
        }

        if (startVid == endVid)
        {
            len += osapi_sprintf((C8_T *)(ptr_out+len), "%d", startVid);
        }
        else
        {
            len += osapi_sprintf((C8_T *)(ptr_out+len), "%d-%d", startVid, endVid);
        }

        if (i < (num - 1))
        {
            len += osapi_sprintf((C8_T *)(ptr_out+len), ",");
        }
        i++;
    }

    return 0;
}

inline static void
_mstpGetMstVlanIdxByVid(
    UI16_T    vlanId,
    UI32_T    *ptr_out)
{
    UI32_T i;

    *ptr_out = 0;
    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (vlanId == _mstp_instance_vlan[i].vid)
        {
            *ptr_out = i;
            break;
        }
    }

    return;
}

inline static int
_mstpGetMstVlanArrayByBmp(
    UI32_T  bmp,
    UI16_T  *ptr_array,
    UI32_T  *ptr_num)
{
    UI32_T i, idx = 0;

    *ptr_num = 0;
    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (!BIT_CHK(bmp,i))
        {
            continue;
        }

        if (0 != _mstp_instance_vlan[i].vid)
        {
            ptr_array[idx++] = _mstp_instance_vlan[i].vid;
        }
    }

    if (0 == idx)
    {
        return -1;
    }

    *ptr_num = idx;

    return 0;
}

static void
_mstp_db_port_role_init_update(
    const UI32_T    port,
    const UI8_T     role)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI8_T               role_array[MAX_MSTP_INSTANCE_NUM];
    UI32_T              i;
    UI32_T              member = 0, trunk_id;

    osapi_memset(role_array, role, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);

    if (PLAT_MAX_PORT_NUM < port)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        member = stp_db_trunk_getMemberBmp(trunk_id);
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if ((member & BIT(i)) > 0)
            {
                req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_ROLE, (i+1));
                rc = stp_db_update_msg(req_type, (void *)role_array, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);
                if (MW_E_OK != rc)
                {
                    STP_LOG_ERR("Fail to send port %d instance role to DB", i+1);
                    break;
                }
            }
        }

        return;
    }

    req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_ROLE, port);
    rc = stp_db_update_msg(req_type, (void *)role_array, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to update port %d instance role to DB", port);
    }

    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: mstp_show_port_member
 * PURPOSE:
 *      show mstp instance-vlan-mapping
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void mstp_show_port_member(void)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        MW_CMD_OUTPUT("vlan idx %d, instance %d,. vid %d, member %d\n", i, _mstp_instance_vlan[i].instance,
                _mstp_instance_vlan[i].vid, _mstp_instance_vlan[i].memBmp);
    }
}

/* FUNCTION NAME: mstp_enable_set
 * PURPOSE:
 *      Set mstp enable
 *
 * INPUT:
 *      enable       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_enable_set(
    UI8_T   enable)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (TRUE == enable)
    {
        rc = _mstp_construct();
        if (MW_E_OK != rc)
        {
            mstp_delete();
            _mstp_port_recover_forwarding();
            return MW_E_OK;
        }
        _mstp_load_configuration();
    }
    else
    {
        mstp_delete();
        _mstp_port_recover_forwarding();
    }

    STP_LOG_DBG("Mstp_status: %s", enable ? "Enable" : "Disable");

    return rc;
}

/* FUNCTION NAME: mstp_force_version_set
 * PURPOSE:
 *      Set mstp mode
 *
 * INPUT:
 *      force_version       - mstp
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_force_version_set(
    UI32_T   force_version)
{
    UI8_T   version = (UI8_T)force_version;

    if (stp_db_getVersion() == version)
    {
        return MW_E_OK;
    }

    STP_LOG_DBG("Set version to mstp");
    stp_db_setVersion(version);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_forward_delay_set
 * PURPOSE:
 *      Set fwd delay
 *
 * INPUT:
 *      forward_delay       - fwd delay
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_forward_delay_set(
    UI16_T  forward_delay)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (stp_check_range(forward_delay,
                            STP_MIN_BRIDGE_FORWARD_DELAY,
                            STP_MAX_BRIDGE_FORWARD_DELAY))
    {
        mstp_set_bridge_forward_delay(forward_delay);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: mstp_max_age_set
 * PURPOSE:
 *      Set max age
 *
 * INPUT:
 *      max_age       - max age
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_max_age_set(
    UI16_T   max_age)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (stp_check_range(max_age,
                            STP_MIN_BRIDGE_MAX_AGE,
                            STP_MAX_BRIDGE_MAX_AGE))
    {
        mstp_set_bridge_max_age(max_age);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: mstp_transmit_hold_count_set
 * PURPOSE:
 *      Set tx hold count
 *
 * INPUT:
 *      tx_hold_cnt       - tx hold count
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_transmit_hold_count_set(
    UI16_T  tx_hold_cnt)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (stp_check_range(tx_hold_cnt,
                            STP_MIN_TRANSMIT_HOLD_COUNT,
                            STP_MAX_TRANSMIT_HOLD_COUNT))
    {
        mstp_set_bridge_transmit_hold_count(tx_hold_cnt);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: mstp_priority_set
 * PURPOSE:
 *      Set cist priority
 *
 * INPUT:
 *      priority       - cist priority
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_priority_set(
    UI16_T  priority)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (stp_check_range(priority, STP_MIN_PRIORITY, STP_MAX_PRIORITY) &&
        (priority % STP_PRIORITY_STEP) == 0)
    {
        mstp_setBridgePriority(0, priority);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }
    return rc;
}

/* FUNCTION NAME: mstp_instance_priority_set
 * PURPOSE:
 *      Set instance priority
 *
 * INPUT:
 *      instance       - instance
 *      priority       - cist priority
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_instance_priority_set(
    UI16_T  instance,
    UI16_T  priority)
{
    if (stp_check_range(priority, STP_MIN_PRIORITY, STP_MAX_PRIORITY) &&
        (priority % STP_PRIORITY_STEP) == 0)
    {
        mstp_setBridgePriority(instance, priority);
    }
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_set_instanceMappingVlan
 * PURPOSE:
 *      Set instance-vlan-mapping
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_set_instanceMappingVlan(
    void)
{
    mstp_setInstanceMappingVlan(_mstp_instance_vlan);
}

/* FUNCTION NAME: mstp_set_portPriority
 * PURPOSE:
 *      Set port priority
 *
 * INPUT:
 *      port_id            - port id
 *      instanceIdx        - instance id
 *      portPriority       - priority
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_set_portPriority(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI8_T  portPriority)
{
    mstp_setPortPriority(port_id, instanceIdx, portPriority);
}

/* FUNCTION NAME: mstp_set_portPathCost
 * PURPOSE:
 *      Set port cost
 *
 * INPUT:
 *      port_id            - port id
 *      instanceIdx        - instance id
 *      portCost           - cost
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_set_portPathCost(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI32_T portCost)
{
    mstp_setPortPathCost(port_id, instanceIdx, portCost);
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: mstp_auto_cost_support_set
 * PURPOSE:
 *      Set auto cost
 *
 * INPUT:
 *      enable            - status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_auto_cost_support_set(
    UI8_T   enable)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          port_number = 0, p = 0;
    UI32_T          trunk_id = 0, insIdx;
    BOOL_T          is_lower_port = FALSE;
    UI32_T          costArray[MAX_MSTP_INSTANCE_NUM];

    if (stp_db_getAutoCostState() != enable)
    {
        stp_db_setAutoCostState(enable);
        if (TRUE == stp_db_getStpGlobalState())
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
                _mstp_port_compute_oper_cost(port_number, NULL, costArray);

                for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
                {
                    mstp_setPortPathCost(port_number, insIdx, costArray[insIdx]);
                }
                if (TRUE == stp_db_getAutoCostState())
                {
                    _mstp_db_update_port_oper_cost(port_number, costArray[0]);
                    _mstp_db_port_oper_cost_update(port_number, costArray);
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: mstp_port_auto_cost_set
 * PURPOSE:
 *      Set port auto cost
 *
 * INPUT:
 *      port            - port id
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
mstp_port_auto_cost_set(
    UI32_T  port)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              port_number = port;
    UI32_T              trunk_id;
    UI32_T              cost = 0, auto_cost, speed_int;
    UI8_T               speed_enum;
    UI32_T              oper_cost[MAX_MSTP_INSTANCE_NUM];
    UI16_T              insIdx;

    stp_db_get_port_speed(port, &speed_enum);
    stp_speed_enum_to_integer(speed_enum, &speed_int);
    auto_cost = _mstp_convert_speed_to_cost(speed_int);

    stp_db_getTrunkID(port, &trunk_id);
    if (MAX_TRUNK_NUM > trunk_id)
    {
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
        stp_trunk_compute_oper_cost(trunk_id, &cost);
    }
    else
    {
        cost = auto_cost;
    }

    for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
    {
        oper_cost[insIdx] = cost;
    }

    _mstp_db_port_oper_cost_update(port_number, oper_cost);
    for (insIdx = 0; insIdx < MAX_MSTP_INSTANCE_NUM; insIdx++)
    {
        mstp_setPortPathCost(port_number, insIdx, oper_cost[insIdx]);
    }

    return rc;
}

/* FUNCTION NAME: mstp_set_port_auto_cost_config
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port            - port id
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
mstp_set_port_auto_cost_config(
    UI32_T  port)
{
    UI32_T              trunk_id, lower_port = port;
    UI32_T              portNum = port, i;
    UI32_T              costArray[MAX_MSTP_INSTANCE_NUM];

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

    _mstp_get_port_cost_config(lower_port, costArray);
    for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
    {
        mstp_setPortPathCost(portNum, i, costArray[i]);
    }
    _mstp_db_port_oper_cost_update(portNum, costArray);

    return MW_E_OK;
}

/* FUNCTION NAME: mstp_port_oper_cost_set
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port_number            - port id
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
mstp_port_oper_cost_set(
    UI32_T   port_number)
{
    UI32_T    autocost[MAX_MSTP_INSTANCE_NUM];
    UI32_T    i;

    _mstp_port_compute_oper_cost(port_number, NULL, autocost);
    for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
    {
        /* Set oper cost to port instance */
        mstp_setPortPathCost(port_number, i, autocost[i]);
    }
    /* Send oper cost to DB */
    _mstp_db_port_oper_cost_update(port_number, autocost);
}
#endif

/* FUNCTION NAME: mstp_port_instance_priority_set
 * PURPOSE:
 *      Set port instance priority config
 *
 * INPUT:
 *      port_number            - port id
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
mstp_port_instance_priority_set(
    UI32_T  port,
    UI16_T  instance,
    UI8_T   priority)
{
    mstp_setPortPriority(port, instance, priority);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_port_instance_cost_set
 * PURPOSE:
 *      Set port instance cost config
 *
 * INPUT:
 *      port_number            - port id
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
mstp_port_instance_cost_set(
    UI32_T  port,
    UI16_T  instance,
    UI32_T  cost)
{
    mstp_setPortPathCost(port, instance, cost);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_port_admin_edge_set
 * PURPOSE:
 *      Set port admin edge
 *
 * INPUT:
 *      port_number            - port id
 *      admin_edge             - admin edge
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
mstp_port_admin_edge_set(
    UI32_T  port,
    UI8_T   admin_edge)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
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

    mstp_set_port_admin_edge(port_number, admin_edge);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_port_mcheck_set
 * PURPOSE:
 *      Set port mcheck
 *
 * INPUT:
 *      port_number            - port id
 *      perform                - mcheck status
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
mstp_port_mcheck_set(
    UI32_T  port,
    UI8_T   perform)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
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

    mstp_set_port_mcheck(port_number, perform);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_port_enable_status_set
 * PURPOSE:
 *      Set port enable status
 *
 * INPUT:
 *      port                  - port id
 *      status                - status
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
mstp_port_enable_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
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

    if (status)
    {
        mstp_set_port_administrative_bridge_port(port_number, status);
    }
    else
    {
        mstp_set_port_administrative_bridge_port(port_number, status);
    }

    return rc;
}

/* FUNCTION NAME: mstp_trunk_update
 * PURPOSE:
 *      Set trunk
 *
 * INPUT:
 *      e_idx                  - entryid
 *      ptr_data               - db msg data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_trunk_update(
    UI16_T e_idx,
    void   *ptr_data)
{
    UI32_T              i, old_trunk_member;
    DB_TRUNK_PORT_T     members[MAX_TRUNK_NUM];
    UI32_T              port_number = 0;
    UI8_T               link = 0;

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
            _mstp_trunk_member_port_add(old_trunk_member);

            /* PROCESS NEW MEMBER */
            _mstp_trunk_member_port_del(members[i].members.member_bmp);
            stp_db_trunk_setMemberBmp(i, members[i].members.member_bmp);

            port_number = STP_TRUNK_ID_TO_PORT_NUMBER(i);
            /* PROCESS TRUNK PORT */
            if (0 != stp_db_trunk_getMemberBmp(i))
            {
                mstp_del_port(port_number);
                if (MW_E_OK != mstp_add_port(port_number))
                {
                    STP_LOG_ERR("Fail to add trunk %d member port %d", i, port_number);
                    return;
                }

                _mstp_db_port_role_init_update(port_number, STP_ROLE_DISABLED);
                link = stp_trunk_check_link_status(i);

                mstp_set_port_oper_point_to_point_mac(port_number, TRUE);
                mstp_set_port_macOperational(port_number, !!(link));
            }
            else
            {
                /* DEL TRUNK PORT DATA */
                mstp_del_port(port_number);
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
        _mstp_trunk_member_port_add(old_trunk_member);

        /* PROCESS NEW MEMBER */
        _mstp_trunk_member_port_del(members[0].members.member_bmp);
        stp_db_trunk_setMemberBmp(e_idx-1, members[0].members.member_bmp);
        port_number = STP_TRUNK_ID_TO_PORT_NUMBER(e_idx-1);
        /* PROCESS TRUNK PORT */
        if (0 != stp_db_trunk_getMemberBmp(e_idx-1))
        {
            mstp_del_port(port_number);
            if (MW_E_OK != mstp_add_port(port_number))
            {
                STP_LOG_ERR("Fail to add trunk %d member port %d", e_idx-1, port_number);
                return;
            }

            _mstp_db_port_role_init_update(port_number, STP_ROLE_DISABLED);
            link = stp_trunk_check_link_status(e_idx-1);
            mstp_set_port_macOperational(port_number, !!(link));

            mstp_set_port_oper_point_to_point_mac(port_number, TRUE);
        }
        else
        {
            /* DEL TRUNK PORT DATA */
            mstp_del_port(port_number);
        }
    }
    _mstp_load_configuration();
}

/* FUNCTION NAME: mstp_port_link_status_set
 * PURPOSE:
 *      Set port link status
 *
 * INPUT:
 *      port                 - port id
 *      is_p2p               - is_p2p
 *      link                 - link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_port_link_status_set(
    UI32_T  port,
    UI8_T   is_p2p,
    UI8_T   link)
{
    mstp_set_port_oper_point_to_point_mac(port, is_p2p);
    mstp_set_port_macOperational(port, link);
}

/* FUNCTION NAME: mstp_trunk_link_status_set
 * PURPOSE:
 *      Set trunk link status
 *
 * INPUT:
 *      port                 - port id
 *      link                 - link status
 *      mode                 - trunk mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_trunk_link_status_set(
    UI32_T  port,
    UI8_T   link,
    UI8_T   mode)
{
    mstp_set_port_oper_point_to_point_mac(port, TRUE);
    if (LAG_MODE_STATIC == mode)
    {
        mstp_set_port_macOperational(port, link);
    }
}

/* FUNCTION NAME: mstp_port_mac_operational_set
 * PURPOSE:
 *      Set port mac operational status
 *
 * INPUT:
 *      port                 - port id
 *      link                 - link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_port_mac_operational_set(
    UI32_T  port,
    UI8_T   link)
{
    mstp_set_port_macOperational(port, link);
}

/* FUNCTION NAME: mstp_port_p2p_mac_set
 * PURPOSE:
 *      Set port p2p mac status
 *
 * INPUT:
 *      port                 - port id
 *      is_p2p               - is_p2p
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_port_p2p_mac_set(
    UI32_T  port,
    UI8_T   is_p2p)
{
    mstp_set_port_oper_point_to_point_mac(port, is_p2p);
}

/* FUNCTION NAME: mstp_set_region_config
 * PURPOSE:
 *      Set mstp region config
 *
 * INPUT:
 *      revision               - revision
 *      ptr_name               - region name
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_set_region_config(
    UI16_T  revision,
    UI8_T   *ptr_name)
{
    mstp_setRegionConfig(revision, ptr_name);
}

/* FUNCTION NAME: mstp_clear_mstTableInstance
 * PURPOSE:
 *      Set mstp clear vlan-instance data
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_clear_mstTableInstance(
    void)
{
    _mstpClearMstTableInstance();
}

/* FUNCTION NAME: mstp_get_mstVlanArrayByBmp
 * PURPOSE:
 *      Get mstp vlan array by vlan bmp
 *
 * INPUT:
 *      bmp             - vlan bmp
 *
 * OUTPUT:
 *      ptr_array       - vlan array
 *      ptr_num         - arraysize
 *
 * RETURN:
 *      -1/0
 *
 * NOTES:
 *      None
 */
int
mstp_get_mstVlanArrayByBmp(
    UI32_T  bmp,
    UI16_T  *ptr_array,
    UI32_T  *ptr_num)
{
    return _mstpGetMstVlanArrayByBmp(bmp, ptr_array, ptr_num);
}

/* FUNCTION NAME: mstp_get_mstTableByVid
 * PURPOSE:
 *      Get mstp vlan entry by vid
 *
 * INPUT:
 *      vid             - vlan id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MST_INSTANCE_VLAN_ENTRY
 *
 * NOTES:
 *      None
 */
MST_INSTANCE_VLAN_ENTRY*
mstp_get_mstTableByVid(
    UI16_T vid)
{
    return _mstpGetMstTableByVid(vid);
}

/* FUNCTION NAME: mstp_update_all_vlan_port
 * PURPOSE:
 *      Get update all port
 *
 * INPUT:
 *      vid             - vlan id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_update_all_vlan_port(
    void)
{
    return _mstp_update_all_vlan_port();
}

/* FUNCTION NAME: mstp_get_vlanidByFid
 * PURPOSE:
 *      Get update vlan id by fid
 *
 * INPUT:
 *      fid             - fid
 *
 * OUTPUT:
 *      ptr_vlanid      - vlan id array
 *      ptr_num         - vlan array num
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_get_vlanidByFid(
    UI16_T  fid,
    UI16_T  *ptr_vlanid,
    UI32_T  *ptr_num)

{
    _mstpGetVidByFid(fid, ptr_vlanid, ptr_num);
}

/* FUNCTION NAME: mstp_get_vlanArray
 * PURPOSE:
 *      Get the vlan array
 *
 * INPUT:
 *      ptr_vlanStr     - VLAN list str
 *
 * OUTPUT:
 *      ptr_outArray    - VLAN table array
 *      ptr_num         - outArray size
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      ptr_outArray size must be MAX_VLAN_ENTRY_NUM
 */
MW_ERROR_NO_T
mstp_get_vlanArray(
    UI8_T  *ptr_vlanStr,
    UI16_T *ptr_outArray,
    UI32_T *ptr_num)
{
    MW_CHECK_PTR(ptr_vlanStr);
    MW_CHECK_PTR(ptr_outArray);
    MW_CHECK_PTR(ptr_num);

    _mstpGetVid(ptr_vlanStr, ptr_outArray, ptr_num);

    return MW_E_OK;
}

/* FUNCTION NAME: mstp_vlanArray_to_str
 * PURPOSE:
 *      Transfer the vlan array to str
 *
 * INPUT:
 *      num             - VLAN Ids
 *      ptr_array       - VLAN table array
 *
 * OUTPUT:
 *      ptr_outStr      - VLAN list str
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      ptr_outStr size must be enough
 */
MW_ERROR_NO_T
mstp_vlanArray_to_str(
    UI32_T num,
    UI16_T *ptr_array,
    UI8_T  *ptr_outStr)
{
    MW_CHECK_PTR(ptr_array);
    MW_CHECK_PTR(ptr_outStr);

    _mstpGetMstVlanStrByArray(num, ptr_array, ptr_outStr);

    return MW_E_OK;
}

/* FUNCTION NAME: mstp_getVlanIndex
 * PURPOSE:
 *      Get vlan index by vlan specify
 *
 * INPUT:
 *      vid             - vlan id
 *
 * OUTPUT:
 *      ptr_outIdx      - vlan index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_getVlanIndex(
    UI16_T  vid,
    UI32_T  *ptr_outIdx)
{
    MW_CHECK_PTR(ptr_outIdx);

    _mstpGetMstVlanIdxByVid(vid, ptr_outIdx);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_getMstVlanArrayByBmp
 * PURPOSE:
 *      Get vlan array by vlan bitmap
 *
 * INPUT:
 *      bmp             - vlan bitmap
 *
 * OUTPUT:
 *      ptr_array       - vlan array
 *      ptr_num         - ptr_array size
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      ptr_outArray size must be MAX_VLAN_ENTRY_NUM
 */
MW_ERROR_NO_T
mstp_getMstVlanArrayByBmp(
    UI32_T  bmp,
    UI16_T  *ptr_array,
    UI32_T  *ptr_num)
{
    MW_CHECK_PTR(ptr_array);
    MW_CHECK_PTR(ptr_num);

    _mstpGetMstVlanArrayByBmp(bmp, ptr_array, ptr_num);
    return MW_E_OK;
}

/* FUNCTION NAME: mstp_update_all_vlan_msg
 * PURPOSE:
 *      Get update all vlan msg
 *
 * INPUT:
 *      method             - db method
 *      f_idx              - field id
 *      e_idx              - entry id
 *      ptr_data           - db msg data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_update_all_vlan_msg(
    UI8_T   method,
    UI8_T   f_idx,
    UI16_T  e_idx,
    void    *ptr_data)
{
    MSTP_INSTANCE_CONFIG_T  info;
    VLAN_ENTRY_INFO_T       *ptr_vlaninfo;
    UI32_T                  i, idx;
    UI32_T                  len, newBmp;
    UI16_T                  *ptr_vlan;
    UI32_T                  *ptr_tagmem;
    UI32_T                  *ptr_untagmem;
    UI8_T                   is_change = FALSE;

    if (VLAN_1Q_ENABLE != _vlan_mode)
    {
        return;
    }

    if (DB_ALL_FIELDS == f_idx)
    {
        if (MW_E_OK != mstp_db_get_instance_vlan_config(&info))
        {
            STP_LOG_ERR("Vlan updata, get mstp-instance data failed!");
            return;
        }

        if (DB_ALL_ENTRIES != e_idx)
        {
            ptr_vlaninfo = (VLAN_ENTRY_INFO_T *)ptr_data;
            idx = e_idx;
            newBmp = ptr_vlaninfo->tagged_member | ptr_vlaninfo->untagged_member;
            if (0 == _mstp_instance_vlan[idx-1].vid)
            {
                /* new create */
                _mstp_instance_vlan[idx-1].instance = 0;
                _mstp_instance_vlan[idx-1].memBmp = newBmp;
                _mstp_instance_vlan[idx-1].vid = ptr_vlaninfo->vlan_id;
                BIT_SET(info.vlanbmp[0], idx-1);
                is_change = TRUE;
            }
            else
            {
                /* member change */
                _mstp_instance_vlan[idx-1].memBmp = newBmp;
                _mstp_instance_vlan[idx-1].vid = ptr_vlaninfo->vlan_id;
            }
        }
        else
        {
            len = offsetof(VLAN_ENTRY_INFO_T, vlan_id) * MAX_VLAN_ENTRY_NUM;
            ptr_vlan = (UI16_T *)(((UI8_T *)ptr_data) + len);
            len = offsetof(VLAN_ENTRY_INFO_T, tagged_member) * MAX_VLAN_ENTRY_NUM;
            ptr_tagmem = (UI32_T *)(((UI8_T *)ptr_data) + len);
            len = offsetof(VLAN_ENTRY_INFO_T, untagged_member) * MAX_VLAN_ENTRY_NUM;
            ptr_untagmem = (UI32_T *)(((UI8_T *)ptr_data) + len);
            for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
            {
                newBmp = ptr_tagmem[i] | ptr_untagmem[i];
                if (0 == _mstp_instance_vlan[i].vid)
                {
                    /* new create */
                    if (0 != ptr_vlan[i])
                    {
                        BIT_SET(info.vlanbmp[0], i);
                        is_change = TRUE;
                    }
                    _mstp_instance_vlan[i].instance = 0;
                    _mstp_instance_vlan[i].memBmp = newBmp;
                    _mstp_instance_vlan[i].vid = ptr_vlan[i];
                }
                else
                {
                    /* member change */
                    _mstp_instance_vlan[i].memBmp = newBmp;
                    _mstp_instance_vlan[i].vid = ptr_vlan[i];
                }
            }
        }
        if ((TRUE == is_change) && (M_GET != method))
        {
            mstp_db_update_instance_vlan_config(&info);
        }
        if (STP_VERSION_MSTP == stp_db_getVersion())
        {
            _mstp_update_all_vlan_port();
        }
    }
}

/* FUNCTION NAME: mstp_update_vlan_mode_change
 * PURPOSE:
 *      update vlan mode
 *
 * INPUT:
 *      mode             - vlan mode
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_update_vlan_mode_change(
    UI8_T mode)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    MSTP_INSTANCE_CONFIG_T  info;
    UI32_T                  i, memBmp = 0;

    if (_vlan_mode == mode)
    {
        return;
    }

    if (VLAN_1Q_ENABLE != _vlan_mode)
    {
        _vlan_mode = mode;
        return;
    }

    rc = mstp_db_get_instance_vlan_config(&info);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Get mstp-instance db data failed!");
        return;
    }

    if (VLAN_1Q_ENABLE == _vlan_mode)
    {
        /* only vlan 1 to cist */
        for (i = 1; i < MAX_MSTP_INSTANCE_NUM; i++)
        {
            if (0 == info.vlanbmp[i])
            {
                continue;
            }
            info.vlanbmp[i] = 0;
            info.instance[i] = 0;
            info.priority[i] = STP_DEFAULT_PRIORITY;
        }

        if (1 != info.vlanbmp[0])
        {
            info.vlanbmp[0] = 1;
        }
        _mstp_instance_vlan[0].instance = 0;
        _mstp_instance_vlan[0].vid = 1;
        vlan_get_plat_max_portBmp(&memBmp);
        _mstp_instance_vlan[0].memBmp = memBmp;
    }

    rc = mstp_db_update_instance_vlan_config(&info);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Update mstp-instance to db failed!");
        return;
    }

    _vlan_mode = mode;
}

/* FUNCTION NAME: mstp_update_vlan_del_msg
 * PURPOSE:
 *      update vlan del msg
 *
 * INPUT:
 *      f_idx             - field id
 *      e_idx             - entry id
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_update_vlan_del_msg(
    UI8_T   f_idx,
    UI16_T  e_idx)
{
    UI32_T  i, idx;
    MW_ERROR_NO_T       rc = MW_E_OK;
    MSTP_INSTANCE_CONFIG_T info;
    UI32_T              instance;

    if (DB_ALL_FIELDS == f_idx)
    {
        rc = mstp_db_get_instance_vlan_config(&info);
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Vlan del msg, get mstp-instance-vlan failed!");
            return;
        }

        if (DB_ALL_ENTRIES != e_idx)
        {
            idx = e_idx - 1;
            for (instance = 0; instance < MAX_MSTP_INSTANCE_NUM; instance++)
            {
                BIT_DEL(info.vlanbmp[instance], idx);
            }
            _mstp_instance_vlan[idx].vid = 0;
            _mstp_instance_vlan[idx].instance = 0;
            _mstp_instance_vlan[idx].memBmp = 0;
        }
        else
        {
            for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
            {
                for (instance = 0; instance < MAX_MSTP_INSTANCE_NUM; instance++)
                {
                    BIT_DEL(info.vlanbmp[instance], i);
                }
                _mstp_instance_vlan[i].vid = 0;
                _mstp_instance_vlan[i].instance = 0;
                _mstp_instance_vlan[i].memBmp = 0;
            }
        }

        mstp_db_update_instance_vlan_config(&info);
    }
}

/* FUNCTION NAME: mstp_db_update_instance_vlan_config
 * PURPOSE:
 *      Send instance-vlan-mapping data to db
 *
 * INPUT:
 *      ptr_info             - instance-vlan-mapping
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
mstp_db_update_instance_vlan_config(
    MSTP_INSTANCE_CONFIG_T *ptr_info)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    DB_REQUEST_TYPE_T       req_type;

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)ptr_info, sizeof(MSTP_INSTANCE_CONFIG_T));

    return rc;
}

/* FUNCTION NAME: mstp_get_vid_by_vlanstr
 * PURPOSE:
 *      transfer vlan str to vlan id array
 *
 * INPUT:
 *      port_num             - port id
 *      ptr_data             - bpdu data
 *      bpdu_size            - bpdu size
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_get_vid_by_vlanstr(
    UI8_T   *ptr_vlanlist,
    UI16_T  *ptr_vlanid,
    UI32_T  *ptr_num)
{
    _mstpGetVid(ptr_vlanlist, ptr_vlanid, ptr_num);
}

/* FUNCTION NAME: mstp_GetMstVlanIdxByVid
 * PURPOSE:
 *      Get vlan index by vlan id
 *
 * INPUT:
 *      port_num             - port id
 *      ptr_data             - bpdu data
 *      bpdu_size            - bpdu size
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_GetMstVlanIdxByVid(
    UI16_T    vlanId,
    UI32_T    *ptr_out)
{
    _mstpGetMstVlanIdxByVid(vlanId, ptr_out);
}

/* FUNCTION NAME: mstp_getMstVlanStrByArray
 * PURPOSE:
 *      Get vlan str by vlan array
 *
 * INPUT:
 *      num             - array size
 *      ptr_array       - array
 *      ptr_out         - str
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      -1/0
 *
 * NOTES:
 *      None
 */
int
mstp_getMstVlanStrByArray(
    UI32_T   num,
    UI16_T   *ptr_array,
    UI8_T    *ptr_out)
{
    return _mstpGetMstVlanStrByArray(num, ptr_array, ptr_out);
}
/* FUNCTION NAME: mstp_pkt_rx
 * PURPOSE:
 *      mstp bpdu handle
 *
 * INPUT:
 *      port_num             - port id
 *      ptr_data             - bpdu data
 *      bpdu_size            - bpdu size
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_pkt_rx(
    UI32_T  port_num,
    void    *ptr_data,
    UI32_T  bpdu_size)
{
    mstp_bpdu_recv(port_num, ptr_data, bpdu_size);
}

/* FUNCTION NAME: mstp_data_init
 * PURPOSE:
 *      mstp data init
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mstp_data_init(
    void)
{
    UI32_T          memBmp = 0;

    osapi_memset(_mstp_instance_vlan, 0x0, sizeof(_mstp_instance_vlan));
    _mstp_instance_vlan[0].vid = 1;
    vlan_get_plat_max_portBmp(&memBmp);
    _mstp_instance_vlan[0].memBmp = memBmp;
}

void
mstp_tick_timers_start(
    void)
{
    mstp_tick_timers();
}

#endif
