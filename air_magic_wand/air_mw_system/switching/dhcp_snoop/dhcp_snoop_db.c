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

/* FILE NAME:   dhcp_snoop_db.c
 * PURPOSE:
 *      Define DHCP snooping database function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>
#include <dhcp_snoop_log.h>
#include <dhcp_snoop_db.h>
#include <dhcp_snoop_pkt.h>

#include <platform.h>
#include <sys_mgmt.h>
#include <air_acl.h>
#include <air_port.h>
#include "mw_acl.h"
#include "mw_cmd_util.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define dhcp_snp_db_chkBool(param)  ((param) == TRUE || (param) == FALSE)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DHCP_SNP_CFG_T       dhcpsnp_db_cfg        = {0};
semaphorehandle_t    dhcpsnp_db_mutex      = NULL;
C8_T                 dhcpsnp_db_hostname[MAX_SYS_NAME_SIZE] = {0};
DB_TRUNK_PORT_T      dhcpsnp_db_trunk_group[MAX_TRUNK_NUM] = {{{0, 0}}};
UI8_T                dhcpsnp_db_port_state[MAX_PORT_NUM] = {0};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
const UI32_T         dhcpsnp_req_acl = MW_ACL_ID_STATIC_DHCP_SNP_REQ;
const UI32_T         dhcpsnp_ack_acl = MW_ACL_ID_STATIC_DHCP_SNP_ACK;
static UI32_T        dhcpsnp_req_acl_ratelimit = MW_ATTACK_ID_INVALID;
static UI32_T        dhcpsnp_ack_acl_ratelimit = MW_ATTACK_ID_INVALID;
static BOOL_T        dhcpsnp_db_ready = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_dhcp_snp_acl_setAction(
    UI32_T      rule_id,
    UI32_T      ap_id,
    UI8_T       action)
{
    const UI32_T     unit = 0;
    AIR_ACL_ACTION_T acl_action =
    {
        .port_fw        = action,
        .attack_rate_id = ap_id,
        .pri_user       = MW_ACL_RX_PRIORITY_NOMRAL_PACKET,
        .field_valid    = BIT(AIR_ACL_FW_PORT) | BIT(AIR_ACL_PRI) |
                          ((action != MW_ACL_ACT_PORT_FW_DEFAULT)? BIT(AIR_ACL_ATTACK): 0),
    };
    AIR_ERROR_NO_T   rc;

    MW_PARAM_CHK((rule_id == MW_ACL_ID_INVALID), MW_E_OTHERS);
    MW_PARAM_CHK((ap_id == MW_ATTACK_ID_INVALID), MW_E_OTHERS);

    if ((rc = air_acl_setAction(unit, rule_id, &acl_action)) != AIR_E_OK)
    {
        return MW_E_OTHERS;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_acl_add(
    const UI32_T acl_id,
    UI32_T      *ptr_ap_id,
    const UI8_T sport,
    const UI8_T dport,
    const UI8_T action)
{
    const UI32_T     unit = 0;
    UI32_T           ap_id = 0;
    AIR_ACL_RULE_T   acl_rule = {};
    AIR_DOS_RATE_LIMIT_CFG_T ap_cfg;
    I32_T            rc = MW_E_OK;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        rc = MW_ATTACK_ID_GET_AVAILABLERULE(&ap_id, &ap_cfg, unit);
        if (MW_E_OK == rc)
        {
            ap_cfg.pkt_thld   = AIR_MAX_SNP_DHCP_RATE_PER_SEC;
            ap_cfg.time_span  = 1000;
            ap_cfg.block_time = 1;
            ap_cfg.tick_sel   = AIR_DOS_RATE_TICKSEL_1MS;

            rc = air_dos_setRateLimitCfg(unit, ap_id, &ap_cfg);
        }
        if (AIR_E_OK != rc)
        {
            dhcp_snp_err("Add DHCP SNP rate limit %d action fail, rc=%d.\n", ap_id, rc);
            mw_acl_mutex_release();
            return MW_E_OP_INCOMPLETE;
        }

        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = TRUE;
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        acl_rule.end = TRUE;
        acl_rule.key.etype = ETHTYPE_IP;
        acl_rule.mask.etype = 0x3;
        acl_rule.key.next_header = MW_IPPROTO_UDP;
        acl_rule.key.dport = dport;
        acl_rule.mask.dport = 0x3;
        acl_rule.key.sport = sport;
        acl_rule.mask.sport = 0x3;
        acl_rule.field_valid = ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_NEXT_HEADER_KEY) |
                                (1U << AIR_ACL_SPORT_KEY) | (1U << AIR_ACL_DPORT_KEY));
        rc = air_acl_setRule(unit, acl_id, &acl_rule);
        if (AIR_E_OK == rc)
        {
            if (MW_E_OK == _dhcp_snp_acl_setAction(acl_id, ap_id, action))
            {
                *ptr_ap_id = ap_id;
                dhcp_snp_dbg("Add ACL done, rule id = %d", acl_id);
                mw_acl_mutex_release();
                return MW_E_OK;
            }
            else
            {
                air_acl_delRule(unit, acl_id);
                dhcp_snp_err("Add DHCP SNP ACL rule entry-id %d ap-id %d action fail.\n", acl_id, ap_id);
            }
        }
        else
        {
            dhcp_snp_err("Add DHCP SNP ACL rule entry-id %d failed, rc=%d.\n", acl_id, rc);
        }
        mw_acl_mutex_release();
    }
    return rc;
}

static MW_ERROR_NO_T
_dhcp_snp_acl_del(
    const UI32_T acl_id)
{
    UI32_T           unit = 0;
    AIR_ERROR_NO_T   rc;


    rc = air_acl_delAction(unit, acl_id);
    if (rc != AIR_E_OK)
    {
        dhcp_snp_info("Failed to delete DHCP ACL rule entry-id %d action, rc %d.\n", acl_id, rc);
    }
    rc = air_acl_delRule(unit, acl_id);
    if (rc != AIR_E_OK)
    {
        dhcp_snp_info("Failed to delete DHCP ACL rule entry-id %d rule, rc %d.\n", acl_id, rc);
    }
    return MW_E_OK;
}

static void
_dhcp_snp_acl_enable(
    UI8_T enable)
{

    const UI32_T     unit = 0;
    AIR_ERROR_NO_T rc;

    AIR_ACL_RULE_T acl_rule;
    dhcp_snp_info("_dhcp_snp_acl_enable , enable = %d\n", enable);

    if (MW_E_OK == mw_acl_mutex_take())
    {
        rc = air_acl_getRule(unit, dhcpsnp_req_acl, &acl_rule);
        if (AIR_E_OK == rc)
        {
            acl_rule.rule_en = enable;
            rc = air_acl_setRule(unit, dhcpsnp_req_acl, &acl_rule);

            if (AIR_E_OK != rc)
            {
                dhcp_snp_err("_dhcp_snp_acl_enable , set req rule failed, rc = %d\n", rc);
                mw_acl_mutex_release();
                return;
            }
        }
        else
        {
            dhcp_snp_err("_dhcp_snp_acl_enable , get req rule failed, rc = %d\n", rc);
            mw_acl_mutex_release();
            return;
        }

        rc = air_acl_getRule(unit, dhcpsnp_ack_acl, &acl_rule);
        if (AIR_E_OK == rc)
        {
            acl_rule.rule_en = enable;
            rc = air_acl_setRule(unit, dhcpsnp_ack_acl, &acl_rule);

            if (AIR_E_OK != rc)
            {
                dhcp_snp_err("_dhcp_snp_acl_enable , set ack rule failed, rc = %d\n", rc);
                mw_acl_mutex_release();
                return;
            }
        }
        else
        {
            dhcp_snp_err("_dhcp_snp_acl_enable , get ack rule failed, rc = %d\n", rc);
            mw_acl_mutex_release();
            return;
        }

        mw_acl_mutex_release();
    }

    if (enable)
    {
        dhcp_snp_dbg("Enable ACL done");
    }
    else
    {
        dhcp_snp_dbg("Disable ACL done");
    }
}

static MW_ERROR_NO_T
_dhcp_snp_db_recvMsg(
    DB_MSG_T **pptr_msg)
{
    MW_ERROR_NO_T   rc       = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;

    MW_CHECK_PTR(pptr_msg);
    rc = osapi_msgRecv(DHCP_SNP_DB_GET_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, DHCP_SNP_GET_QUEUE_TIMEOUT);
    if (rc != MW_E_OK)
    {
        return rc;
    }
    *pptr_msg = ptr_msg;

    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_db_queue_create()
{
    return osapi_msgCreate(DHCP_SNP_DB_GET_QUEUE_NAME, DHCP_SNP_DB_GET_QUEUE_LENGTH, sizeof(void *));
}

static MW_ERROR_NO_T
_dhcp_snp_db_queue_delete()
{
    return osapi_msgDelete(DHCP_SNP_DB_GET_QUEUE_NAME);
}

static MW_ERROR_NO_T
_dhcp_snp_db_getData(
    const UI8_T             method,
    const DB_REQUEST_TYPE_T req_type,
    DB_MSG_T                **pptr_msg,
    UI8_T                   **pptr_data,
    UI16_T                  *ptr_size)
{
    MW_ERROR_NO_T   rc           = MW_E_OK;
    DB_MSG_T        *ptr_msg     = NULL;

    rc = dbapi_getDataSize(req_type, ptr_size);
    if (MW_E_OK != rc)
    {
        dhcp_snp_err("Get data size failed(%d)", rc);
        return rc;
    }

    rc = dhcp_snp_db_sendMsg(DHCP_SNP_DB_GET_QUEUE_NAME,
                             method,
                             req_type,
                             NULL,
                             *ptr_size,
                             &ptr_msg);
    if (rc != MW_E_OK)
    {
        dhcp_snp_err("Send Msg failed(%d)", rc);
        return rc;
    }

    dhcp_snp_dbg("wait for get msg");

    /* wait for DB response messgae */
    rc = _dhcp_snp_db_recvMsg(&ptr_msg);
    if (rc != MW_E_OK)
    {
        dhcp_snp_err("Recv Msg failed(%d)", rc);
        osapi_free(ptr_msg);
        return rc;
    }

    rc = ptr_msg->type.result;
    if (rc == MW_E_OK)
    {
        *pptr_msg      = ptr_msg;
        *pptr_data     = dhcp_snp_dbpayload_data(dhcp_snp_dbmsg_payload(ptr_msg));
    }
    else
    {
        osapi_free(ptr_msg);
    }
    return rc;
}

static void
_dhcp_snp_db_resetConfig(
    UI32_T pbmp)
{
    if (pbmp == 0)
    {
        dhcp_snp_dbg("no port to reset");
        return;
    }

    dhcp_snp_dbg("reset pbmp = %x", pbmp);

    if (_dhcp_snp_db_queue_create() == MW_E_OK)
    {
        MW_ERROR_NO_T     rc        = MW_E_OK;
        DB_REQUEST_TYPE_T req_type;
        DB_MSG_T          *ptr_msg  = NULL;
        UI8_T             *ptr_data = NULL;
        UI16_T            data_size = 0;
        UI8_T             p = 0, f_idx = 0;

        for (p = 0; p < MW_PORT_NUM; ++p)
        {
            if (pbmp & BIT(p))
            {
                DB_DHCP_SNP_PORT_INFO_T  entry      = {};
                for (f_idx = DHCP_SNP_TRUST_PORT; f_idx < DHCP_SNP_PORT_INFO_LAST; ++f_idx)
                {
                    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, f_idx, p + 1);
                    rc = _dhcp_snp_db_getData(M_GETFLASH, req_type, &ptr_msg, &ptr_data, &data_size);
                    if (rc == MW_E_OK)
                    {
                        dhcp_snp_dbg("Get data port = %d, field = %d", p + 1, f_idx);
                        switch (f_idx)
                        {
                        case DHCP_SNP_TRUST_PORT:
                            entry.trust_port = *ptr_data;
                            break;
                        case DHCP_SNP_OPT82:
                            entry.opt82_enable = *ptr_data;
                            break;
                        case DHCP_SNP_OPT82_MODE:
                            entry.opt82_mode = *ptr_data;
                            break;
                        case DHCP_SNP_OPT82_CIRCUIT_ID_TYPE:
                            entry.circuit_id_type = *ptr_data;
                            break;
                        case DHCP_SNP_OPT82_CIRCUIT_ID:
                            osapi_strncpy((C8_T *)entry.circuit_id, (const C8_T *)ptr_data, (MAX_OPT82_ID_LEN - 1));
                            break;
                        case DHCP_SNP_OPT82_REMOTE_ID_TYPE:
                            entry.remote_id_type = *ptr_data;
                            break;
                        case DHCP_SNP_OPT82_REMOTE_ID:
                            osapi_strncpy((C8_T *)entry.remote_id, (const C8_T *)ptr_data, (MAX_OPT82_ID_LEN - 1));
                            break;
                        default:
                            break;
                        }
                        osapi_free(ptr_msg);
                    }
                    else
                    {
                        dhcp_snp_err("Get data fail(%d)", rc);
                    }
                }
                if (rc == MW_E_OK)
                {
                    rc = dhcp_snp_db_update(DHCP_SNP_PORT_INFO, DB_ALL_FIELDS, p + 1, &entry, sizeof(entry));
                }
                if (rc != MW_E_OK)
                {
                    dhcp_snp_err("Reset port fail(%d)", rc);
                }
            }
        }
        _dhcp_snp_db_queue_delete();
    }
    return;
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_db_sendMsg
 * PURPOSE:
 *      Send to DB request message
 *
 * INPUT:
 *      ptr_module     -  sender's name
 *      ptr_target     -  target queue's name
 *      method         -  DB request method
 *      req_type       -  DB request type
 *      ptr_data       -  DB request data
 *      size           -  data size
 *      pptr_msg       -  output msg pointer
 *      ptr_send_func  -  customize sending callback function
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
dhcp_snp_db_sendMsg(
    const C8_T                   *ptr_target,
    const UI8_T                  method,
    const DB_REQUEST_TYPE_T      req_type,
    void  *const                 ptr_data,
    const UI16_T                 size,
    DB_MSG_T                     **pptr_msg)
{
    MW_ERROR_NO_T   rc           = MW_E_OK;
    DB_MSG_T        *ptr_msg     = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T          msg_size     = 0;

    MW_PARAM_CHK((req_type.t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    if (osapi_calloc(msg_size, DHCP_SNP_MODULE_NAME, (void **)&ptr_msg) != MW_E_OK)
    {
        dhcp_snp_err("Fail to allocate memory");
        return MW_E_NO_MEMORY;
    }
    /* message */
    osapi_strncpy(ptr_msg->cq_name, ptr_target, DB_Q_NAME_SIZE - 1);  /* max length of name: 3 */
    ptr_msg->cq_name[DB_Q_NAME_SIZE - 1] = 0;
    ptr_msg->method = method;
    ptr_msg->type.count = 1;
    /* payload */
    ptr_payload = dhcp_snp_dbmsg_payload(ptr_msg);
    ptr_payload->request = req_type;
    ptr_payload->data_size = size;

    if (size > 0 && method != M_GET && method != M_GETFLASH)
    {
        osapi_memcpy(dhcp_snp_dbpayload_data(ptr_payload), ptr_data, size);
    }

    dhcp_snp_db_dbmsg_dump(msg_size, ptr_msg);
    rc = dbapi_sendMsg(ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
    if (rc == MW_E_OK)
    {
        if (pptr_msg)
        {
            *pptr_msg = ptr_msg;
        }
    }
    else
    {
        osapi_free(ptr_msg);
    }
    return rc;
}

/* FUNCTION NAME: dhcp_snp_db_update
 * PURPOSE:
 *      Send to DB request message
 *
 * INPUT:
 *      method         -  DB request method
 *      t_idx          -  DB table index
 *      f_idx          -  DB field index
 *      e_idx          -  DB entry index
 *      req_type       -  DB request type
 *      ptr_data       -  DB request data
 *      size           -  data size
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_update(
    const UI8_T  t_idx,
    const UI8_T  f_idx,
    const UI16_T e_idx,
    void  *const ptr_data,
    const UI16_T size)
{
    const DB_REQUEST_TYPE_T req_type = dhcp_snp_dbmsg_init_reqtype(t_idx, f_idx, e_idx);
    if (dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME,
                            M_UPDATE,
                            req_type,
                            ptr_data,
                            size,
                            NULL) != MW_E_OK)
    {
        dhcp_snp_err("Fail to send to DB");
    }
    return MW_E_OK;
}

/* FUNCTION NAME: _dhcp_snp_db_handle
 * PURPOSE:
 *      Handle DB message and update local database
 *
 * INPUT:
 *      ptr_msg        -  DB message
 *      ptr_req        -  DB request
 *      ptr_data       -  DB data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_dhcp_snp_db_handle(
    DB_MSG_T           *const ptr_msg,
    DB_REQUEST_TYPE_T  *const ptr_req,
    UI8_T              *const ptr_data)
{
    switch (ptr_msg->method)
    {
    case M_RESPONSE:
        dhcp_snp_dbg("response result");
        /** Fall through **/
    case M_GET:
    case M_UPDATE:
        if (ptr_req->t_idx == DHCP_SNP_INFO)
        {
            const UI8_T    f_idx     = ptr_req->f_idx;
            const UI8_T    data      = ptr_data[0];

            dhcp_snp_dbg("cmd(%d)", f_idx);
            if (f_idx == DHCP_SNP_ENABLE)
            {
                if (!dhcp_snp_db_chkBool(data))
                {
                    dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, data);
                    return MW_E_OK;
                }
                dhcp_snp_db_lock();
                dhcp_snp_db_set_cfg(global_enable, data);
                _dhcp_snp_acl_enable(data);
                dhcp_snp_db_unlock();
            }
        }
        else if (ptr_req->t_idx == TRUNK_PORT)
        {
            if (ptr_req->f_idx == TRUNK_MEMBERS)
            {
                const UI16_T  e_idx     = ptr_req->e_idx;  /* port index */
                UI8_T *const ptr_mbr = (UI8_T *)ptr_data;
                DB_TRUNK_PORT_T trunk_info;
                UI32_T        del_pbmp  = 0;
                UI8_T         i         = 0;

                if (e_idx == DB_ALL_ENTRIES)
                {
                    for (i = 0; i < MAX_TRUNK_NUM; ++i)
                    {
                        memcpy(&trunk_info, (void *)(ptr_mbr + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
                        del_pbmp = dhcpsnp_db_trunk_group[i].members.member_bmp & ~(trunk_info.members.member_bmp);
                        if (del_pbmp != 0)
                        {
                            _dhcp_snp_db_resetConfig(del_pbmp);
                        }
                        /* Update trunk group */
                        dhcp_snp_db_lock();
                        dhcpsnp_db_trunk_group[i].members.member_bmp = trunk_info.members.member_bmp;
                        dhcpsnp_db_trunk_group[i].members.mode = trunk_info.members.mode;
                        dhcp_snp_db_unlock();
                        dhcp_snp_dbg("Update trunk %d group [0x%x]", i, trunk_info.members.member_bmp);
                    }
                }
                else
                {
                    memcpy(&trunk_info, (void *)ptr_mbr, sizeof(DB_TRUNK_PORT_T));
                    del_pbmp = dhcpsnp_db_trunk_group[e_idx - 1].members.member_bmp & ~(trunk_info.members.member_bmp);
                    if (del_pbmp != 0)
                    {
                        _dhcp_snp_db_resetConfig(del_pbmp);
                    }
                    /* Update trunk group */
                    dhcp_snp_db_lock();
                    dhcpsnp_db_trunk_group[e_idx - 1].members.member_bmp = trunk_info.members.member_bmp;
                    dhcpsnp_db_trunk_group[e_idx - 1].members.mode = trunk_info.members.mode;
                    dhcp_snp_db_unlock();
                    dhcp_snp_dbg("Update trunk group(%d) [0x%x]", e_idx,
                        trunk_info.members.member_bmp);
                }
            }
        }
        else if (ptr_req->t_idx == PORT_OPER_INFO)
        {
            if (ptr_req->f_idx == PORT_LACP_STATE)
            {
                if (ptr_req->e_idx == DB_ALL_ENTRIES)
                {
                    UI8_T         i;
                    UI8_T         *lacp_state;

                    for (i = 0; i < MAX_PORT_NUM; i++)
                    {
                        lacp_state = (UI8_T *)(ptr_data + (i * sizeof(UI8_T)));
                        dhcp_snp_db_lock();
                        dhcpsnp_db_port_state[i] = *lacp_state;
                        dhcp_snp_db_unlock();
                    }
                }
            }
        }
        else if (ptr_req->t_idx == DHCP_SNP_PORT_INFO)
        {
            const UI8_T       f_idx     = ptr_req->f_idx;
            const UI16_T      e_idx     = ptr_req->e_idx;  /* port index */
            UI16_T            i         = 0;
            UI16_T            port      = 0;
            UI16_T            first_port = 0, last_port = 0;

            dhcp_snp_dbg("cmd(%d), port(%d)", f_idx, e_idx);
            /* Write 1 entry with all fields */
            if (f_idx == DB_ALL_FIELDS)
            {
                if (e_idx != DB_ALL_ENTRIES)
                {
                    DB_DHCP_SNP_PORT_INFO_T  *ptr_entry = (DB_DHCP_SNP_PORT_INFO_T *)ptr_data;
                    C8_T                     *ptr_buff  = NULL;
                    UI8_T                    len        = 0;

                    if (!dhcp_snp_db_chkBool(ptr_entry->trust_port))
                    {
                        dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, ptr_entry->trust_port);
                        return MW_E_OK;
                    }
                    else if (!dhcp_snp_db_chkBool(ptr_entry->opt82_enable))
                    {
                        dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, ptr_entry->opt82_enable);
                        return MW_E_OK;
                    }
                    else if ((ptr_entry->opt82_mode < DHCP_SNP_OPT82_MODE_NOTOUCH) || (ptr_entry->opt82_mode > DHCP_SNP_OPT82_MODE_LAST))
                    {
                        dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, ptr_entry->opt82_mode);
                        return MW_E_OK;
                    }
                    else if ((osapi_strlen((const C8_T *)ptr_entry->circuit_id)) >= DHCP_SNP_OPT82_CIR_ID_MAX_LEN)
                    {
                        dhcp_snp_err("cmd(%d): Invalid length - %lu", f_idx, osapi_strlen((const C8_T *)ptr_entry->circuit_id));
                        return MW_E_OK;
                    }
                    else if ((osapi_strlen((const C8_T *)ptr_entry->remote_id)) >= DHCP_SNP_OPT82_REM_ID_MAX_LEN)
                    {
                        dhcp_snp_err("cmd(%d): Invalid length - %lu", f_idx, osapi_strlen((const C8_T *)ptr_entry->remote_id));
                        return MW_E_OK;
                    }

                    dhcp_snp_db_lock();
                    dhcp_snp_db_set_portcfg(trust_port, e_idx, ptr_entry->trust_port);
                    dhcp_snp_db_set_portcfg(option82_enable, e_idx, ptr_entry->opt82_enable);
                    dhcp_snp_db_set_portcfg(option82_mode, e_idx, ptr_entry->opt82_mode);
                    dhcp_snp_db_set_portcfg(circuit_id_type, e_idx, ptr_entry->circuit_id_type);
                    dhcp_snp_db_set_portcfg(remote_id_type, e_idx, ptr_entry->remote_id_type);
                    len = osapi_strlen((const C8_T *)ptr_entry->circuit_id);
                    if (len > 0)
                    {
                        ptr_buff = NULL;
                        if (osapi_calloc(len + 1, DHCP_SNP_MODULE_NAME, (void **)&ptr_buff) != MW_E_OK)
                        {
                            dhcp_snp_err("Fail to allocate memory");
                            dhcp_snp_db_unlock();
                            return MW_E_OK;
                        }
                        if (dhcp_snp_db_portcfg(circuit_id, e_idx))
                        {
                            dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(circuit_id, e_idx));
                        }
                        dhcp_snp_db_portcfg(circuit_id, e_idx) = ptr_buff;
                        dhcp_snp_db_set_portcfg(circuit_id_len, e_idx, len);
                        dhcp_snp_db_set_str(dhcp_snp_db_portcfg(circuit_id, e_idx), ptr_entry->circuit_id, len);
                    }
                    else
                    {
                        dhcp_snp_db_set_portcfg(circuit_id_len, e_idx, 0);
                        dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(circuit_id, e_idx));
                    }
                    len = osapi_strlen((const C8_T *)ptr_entry->remote_id);
                    if (len > 0)
                    {
                        ptr_buff = NULL;
                        if (osapi_calloc(len + 1, DHCP_SNP_MODULE_NAME, (void **)&ptr_buff) != MW_E_OK)
                        {
                            dhcp_snp_err("Fail to allocate memory");
                            dhcp_snp_db_unlock();
                            return MW_E_OK;
                        }
                        if (dhcp_snp_db_portcfg(remote_id, e_idx))
                        {
                            dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(remote_id, e_idx));
                        }
                        dhcp_snp_db_portcfg(remote_id, e_idx) = ptr_buff;
                        dhcp_snp_db_set_portcfg(remote_id_len, e_idx, len);
                        dhcp_snp_db_set_str(dhcp_snp_db_portcfg(remote_id, e_idx), ptr_entry->remote_id, len);
                    }
                    else
                    {
                        dhcp_snp_db_set_portcfg(remote_id_len, e_idx, 0);
                        dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(remote_id, e_idx));
                    }
                    dhcp_snp_db_unlock();
                }
                else
                {
                    /* ignore all entries case */
                }
                return MW_E_OK;
            }

            /* Write 1/all entry with specified field */
            if (e_idx == DB_ALL_ENTRIES)
            {
                first_port = 1;
                last_port = PLAT_MAX_PORT_NUM;
            }
            else
            {
                first_port = last_port = e_idx;
            }

            for (port = first_port, i = 0; port <= last_port; ++port, ++i)
            {
                const UI8_T   data        = ptr_data[i];
                UI8_T *const  ptr_p_data  = ptr_data + i * sizeof(OPT82_ID_T);

                switch (f_idx)
                {
                case DHCP_SNP_TRUST_PORT:
                case DHCP_SNP_OPT82:
                    if (!dhcp_snp_db_chkBool(data))
                    {
                        dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, data);
                        return MW_E_OK;
                    }
                    break;
                case DHCP_SNP_OPT82_MODE:
                    if ((data < DHCP_SNP_OPT82_MODE_NOTOUCH) || (data > DHCP_SNP_OPT82_MODE_LAST))
                    {
                        dhcp_snp_err("cmd(%d): Invalid parameter - %d", f_idx, data);
                        return MW_E_OK;
                    }
                    break;
                case DHCP_SNP_OPT82_CIRCUIT_ID_TYPE:
                case DHCP_SNP_OPT82_REMOTE_ID_TYPE:
                    break;
                case DHCP_SNP_OPT82_CIRCUIT_ID:
                    if ((osapi_strlen((const C8_T *)ptr_p_data)) >= DHCP_SNP_OPT82_CIR_ID_MAX_LEN)
                    {
                        dhcp_snp_err("cmd(%d): Invalid length - %lu", f_idx, osapi_strlen((const C8_T *)ptr_p_data));
                        return MW_E_OK;
                    }
                    break;
                case DHCP_SNP_OPT82_REMOTE_ID:
                    if ((osapi_strlen((const C8_T *)ptr_p_data)) >= DHCP_SNP_OPT82_REM_ID_MAX_LEN)
                    {
                        dhcp_snp_err("cmd(%d): Invalid length - %lu", f_idx, osapi_strlen((const C8_T *)ptr_p_data));
                        return MW_E_OK;
                    }
                    break;
                default:
                    dhcp_snp_err("recv unknown field: [%d]", f_idx);
                    return MW_E_OK;
                }

                dhcp_snp_db_lock();
                switch (f_idx)
                {
                case DHCP_SNP_TRUST_PORT:
                    dhcp_snp_db_set_portcfg(trust_port, port, data);
                    break;
                case DHCP_SNP_OPT82:
                    dhcp_snp_db_set_portcfg(option82_enable, port, data);
                    break;
                case DHCP_SNP_OPT82_MODE:
                    dhcp_snp_db_set_portcfg(option82_mode, port, data);
                    break;
                case DHCP_SNP_OPT82_CIRCUIT_ID_TYPE:
                    dhcp_snp_db_set_portcfg(circuit_id_type, port, data);
                    break;
                case DHCP_SNP_OPT82_REMOTE_ID_TYPE:
                    dhcp_snp_db_set_portcfg(remote_id_type, port, data);
                    break;
                case DHCP_SNP_OPT82_CIRCUIT_ID:
                {
                    const UI8_T len = osapi_strlen((const C8_T *)ptr_p_data);
                    if (len > 0)
                    {
                        C8_T *ptr_buff = NULL;
                        if (osapi_calloc(len + 1, DHCP_SNP_MODULE_NAME, (void **)&ptr_buff) != MW_E_OK)
                        {
                            dhcp_snp_err("Fail to allocate memory");
                            dhcp_snp_db_unlock();
                            return MW_E_OK;
                        }
                        if (dhcp_snp_db_portcfg(circuit_id, port))
                        {
                            dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(circuit_id, port));
                        }
                        dhcp_snp_db_portcfg(circuit_id, port) = ptr_buff;
                        dhcp_snp_db_set_portcfg(circuit_id_len, port, len);
                        dhcp_snp_db_set_str(dhcp_snp_db_portcfg(circuit_id, port), ptr_p_data, len);
                    }
                    else
                    {
                        dhcp_snp_db_set_portcfg(circuit_id_len, port, 0);
                        dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(circuit_id, port));
                    }
                    break;
                }
                case DHCP_SNP_OPT82_REMOTE_ID:
                {
                    const UI8_T len = osapi_strlen((const C8_T *)ptr_p_data);
                    if (len > 0)
                    {
                        C8_T *ptr_buff = NULL;
                        if (osapi_calloc(len + 1, DHCP_SNP_MODULE_NAME, (void **)&ptr_buff) != MW_E_OK)
                        {
                            dhcp_snp_err("Fail to allocate memory");
                            dhcp_snp_db_unlock();
                            return MW_E_OK;
                        }
                        if (dhcp_snp_db_portcfg(remote_id, port))
                        {
                            dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(remote_id, port));
                        }
                        dhcp_snp_db_portcfg(remote_id, port) = ptr_buff;
                        dhcp_snp_db_set_portcfg(remote_id_len, port, len);
                        dhcp_snp_db_set_str(dhcp_snp_db_portcfg(remote_id, port), ptr_p_data, len);
                    }
                    else
                    {
                        dhcp_snp_db_set_portcfg(remote_id_len, port, 0);
                        dhcp_snp_db_clean_str(dhcp_snp_db_portcfg(remote_id, port));
                    }
                    break;
                }
                default:
                    break;
                }
                dhcp_snp_db_unlock();
            }
        }
        else if (ptr_req->t_idx == SYS_INFO)
        {
            if (ptr_req->f_idx == SYS_NAME)
            {
                dhcp_snp_db_lock();
                osapi_snprintf(dhcpsnp_db_hostname, sizeof(dhcpsnp_db_hostname), (const C8_T *)ptr_data);
                dhcp_snp_db_unlock();
                dhcp_snp_dbg("Update sys name: %s", dhcpsnp_db_hostname);
            }
        }
        break;
    case M_CREATE:
        dhcp_snp_dbg("create method");
        break;
    case M_ACK:
        dhcp_snp_dbg("ack result code: %d", ptr_msg->type.result);
        break;
    case M_DELETE:
        dhcp_snp_err("recv delete method");
        break;
    default:
        dhcp_snp_err("recv unknown method: [%02X]", ptr_msg->method);
        break;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_db_handle
 * PURPOSE:
 *      Handle and parse DB message
 *
 * INPUT:
 *      ptr_msg        -  DB message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_handle(
    DB_MSG_T *const ptr_msg)
{
    MW_ERROR_NO_T     rc                  = MW_E_OK;
    UI16_T            data_size           = 0;
    UI8_T             *ptr_data           = NULL;
    UI8_T             *ptr_next_payload   = NULL;
    UI8_T             req_cnt             = 0;
    DB_REQUEST_TYPE_T request             = {0};

    dhcp_snp_db_dbmsg_dump(0, ptr_msg);

    if (dhcpsnp_db_ready != TRUE)
    {
        return MW_E_OK;
    }
    /* Only 1 message when receive response, or refer to count */
    req_cnt = (ptr_msg->method != M_RESPONSE) ? ptr_msg->type.count : 1;
    do
    {
        rc = dbapi_parseMsg(ptr_msg, req_cnt, &request, &data_size, &ptr_data, &ptr_next_payload);
        if (MW_E_OK == rc)
        {
            _dhcp_snp_db_handle(ptr_msg, &request, ptr_data);
        }
    }
    while ((MW_E_OK == rc) && (NULL != ptr_next_payload));

    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_db_subscribe
 * PURPOSE:
 *      DHCP snooping database subscribe function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_subscribe(
    void)
{
    DB_REQUEST_TYPE_T req_type = {0};

    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_INFO, DHCP_SNP_ENABLE, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_TRUST_PORT, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_MODE, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID_TYPE, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID_TYPE, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = dhcp_snp_dbmsg_init_reqtype(SYS_INFO, SYS_NAME, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
    req_type = dhcp_snp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_LACP_STATE, DB_ALL_ENTRIES);
    dhcp_snp_db_sendMsg(DHCP_SNP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);


    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_db_dump
 * PURPOSE:
 *      Dump DHCP snooping database function
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
dhcp_snp_db_dump(
    void)
{
    const C8_T  *ptr_enable                      = "On";
    const C8_T  *ptr_disable                     = "Off";
    const C8_T  *ptr_default                     = "<default config>";
    const C8_T  *ptr_empty                       = "";
    UI16_T      port_id                          = 0;
    const C8_T  *str_mode[DHCP_SNP_OPT82_MODE_LAST] =
    {
        [DHCP_SNP_OPT82_MODE_NOTOUCH] = "Keep",
        [DHCP_SNP_OPT82_MODE_REPLACE] = "Repl",
        [DHCP_SNP_OPT82_MODE_DISCARD] = "Drop",
    };

    dhcp_snp_db_lock();
    MW_CMD_OUTPUT("\nDHCP snooping:\n\n");
    MW_CMD_OUTPUT("  Status: %s\n", (dhcp_snp_db_cfg(global_enable)) ? ptr_enable : ptr_disable);
    MW_CMD_OUTPUT("  Trust port:\n    ");
    for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; ++port_id)
    {
        if (dhcp_snp_db_portcfg(trust_port, port_id))
        {
            MW_CMD_OUTPUT("%d,", port_id);
        }
    }
    MW_CMD_OUTPUT("\n\n");
#ifdef DHCP_SNP_DEBUG
    MW_CMD_OUTPUT("  Trunk group:\n");
    for (port_id = 0; port_id < MAX_TRUNK_NUM; ++port_id)
    {
        MW_CMD_OUTPUT("    Group %d member: 0x%x\n", port_id + 1, dhcpsnp_db_trunk_group[port_id].members.member_bmp);
    }
    MW_CMD_OUTPUT("\n");
#endif /* DHCP_SNP_DEBUG */
    MW_CMD_OUTPUT("  Option 82:\n");
    MW_CMD_OUTPUT("    port | stat |  op  |  type  | len | id\n");
    for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; ++port_id)
    {
        MW_CMD_OUTPUT("    %4d |", port_id);
        MW_CMD_OUTPUT(" %4s |", (dhcp_snp_db_portcfg(option82_enable, port_id)) ? ptr_enable : ptr_disable);
        MW_CMD_OUTPUT(" %4s |", str_mode[dhcp_snp_db_portcfg(option82_mode, port_id)]);
        MW_CMD_OUTPUT(" (c)%3d |", dhcp_snp_db_portcfg(circuit_id_type, port_id));
        MW_CMD_OUTPUT(" %3d |", dhcp_snp_db_portcfg(circuit_id_len, port_id));
        MW_CMD_OUTPUT(" %s\n", (dhcp_snp_db_portcfg(circuit_id_len, port_id) > 0) ? dhcp_snp_db_portcfg(circuit_id, port_id) : ptr_default);
        MW_CMD_OUTPUT("%24s", ptr_empty);
        MW_CMD_OUTPUT(" (r)%3d |", dhcp_snp_db_portcfg(remote_id_type, port_id));
        MW_CMD_OUTPUT(" %3d |", dhcp_snp_db_portcfg(remote_id_len, port_id));
        MW_CMD_OUTPUT(" %s\n", (dhcp_snp_db_portcfg(remote_id_len, port_id) > 0) ? dhcp_snp_db_portcfg(remote_id, port_id) : ptr_default);
    }
    dhcp_snp_db_unlock();
}

/* FUNCTION NAME: dhcp_snp_db_deinit
 * PURPOSE:
 *      Deinit DHCP snooping database function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_deinit(
    void)
{
    dhcpsnp_db_ready = FALSE;
    _dhcp_snp_acl_del(dhcpsnp_req_acl);
    _dhcp_snp_acl_del(dhcpsnp_ack_acl);

    if (dhcpsnp_db_mutex)
    {
        osapi_mutexDelete(dhcpsnp_db_mutex);
        dhcpsnp_db_mutex = NULL;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_db_init
 * PURPOSE:
 *      Init DHCP snooping database function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_init(
    void)
{
    osapi_mutexCreate(DHCP_SNP_MODULE_NAME, &dhcpsnp_db_mutex);
    if (dhcpsnp_db_mutex == NULL)
    {
        dhcp_snp_err("create mutex fail !");
        return MW_E_NO_MEMORY;
    }
    if (_dhcp_snp_acl_add(dhcpsnp_req_acl,
            &dhcpsnp_req_acl_ratelimit,
            DHCP_SNP_UDP_PORT_DHCP_CLIENT,
            DHCP_SNP_UDP_PORT_DHCP_SERVER,
            MW_ACL_ACT_PORT_FW_CPU_ONLY) != MW_E_OK)
    {
        return MW_E_OTHERS;
    }
    if (_dhcp_snp_acl_add(dhcpsnp_ack_acl,
            &dhcpsnp_ack_acl_ratelimit,
            DHCP_SNP_UDP_PORT_DHCP_SERVER,
            DHCP_SNP_UDP_PORT_DHCP_CLIENT,
            MW_ACL_ACT_PORT_FW_CPU_ONLY) != MW_E_OK)
    {
        return MW_E_OTHERS;
    }

    dhcp_snp_db_cfg(fwd_miss_match) = TRUE;
    dhcpsnp_db_ready = TRUE;
    return MW_E_OK;
}

MW_ERROR_NO_T dhcp_snp_db_getVlanMember(const UI16_T vlan_id, AIR_PORT_BITMAP_T tx_pbmp)
{

    MW_ERROR_NO_T     rc        = MW_E_OK;

    if (_dhcp_snp_db_queue_create() == MW_E_OK)
    {
        DB_REQUEST_TYPE_T req_type;
        DB_MSG_T          *ptr_msg  = NULL;
        UI16_T            data_size = 0;
        DB_VLAN_ENTRY_T  *ptr_vlan_entry_tbl = NULL;
        UI16_T            vid = 0;
        UI32_T            pbmp;
        UI32_T            port = 0;

        req_type = dhcp_snp_dbmsg_init_reqtype(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = _dhcp_snp_db_getData(M_GET, req_type, &ptr_msg, (UI8_T **)&ptr_vlan_entry_tbl, &data_size);
        if (rc == MW_E_OK)
        {
            dhcp_snp_dbg("Get VLAN DB done");

            for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
            {
                if (vlan_id == ptr_vlan_entry_tbl->vlan_id[vid])
                {
                    pbmp = ptr_vlan_entry_tbl->port_member[vid] | ptr_vlan_entry_tbl->tagged_member[vid] | ptr_vlan_entry_tbl->untagged_member[vid];

                    dhcp_snp_dbg("vid = %d, pbmp = 0x%x", vlan_id, pbmp);

                    BITMAP_PORT_FOREACH(pbmp, port)
                    {
                        AIR_PORT_ADD(tx_pbmp, port);
                    }
                }
            }
            osapi_free(ptr_msg);
        }
        else
        {
            dhcp_snp_err("Get data fail(%d)", rc);
        }
        _dhcp_snp_db_queue_delete();
    }
    return rc;
}
