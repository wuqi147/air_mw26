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

/* FILE NAME:   voice_vlan.c
 * PURPOSE:
 *      Define voice_vlan function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "voice_vlan.h"
#include "sys_mgmt.h"
#include "vlan_utils.h"
#include "mw_utils.h"
#include "mac_utils.h"
#include "mw_msg.h"
#include "mw_portbmp.h"
#include "ethernetif.h"
#include "air_acl.h"
#include "mw_acl.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef VOICE_DEBUG
#define voice_vlan_db_dbmsg_dump(size, ptr_msg) \
    do {                                                                        \
        DB_PAYLOAD_T *ptr_pload = ((DB_PAYLOAD_T *)(&((ptr_msg)->ptr_payload)));   \
        MW_LOG_DEBUG(VLAN, "Voice DB Msg Dump:");                              \
        MW_LOG_DEBUG(VLAN, "  method=0x%X, cnt/result=%u, size=%u",            \
            (ptr_msg)->method, (ptr_msg)->type.count, (size));                  \
        MW_LOG_DEBUG(VLAN, "  t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u\n",   \
            ptr_pload->request.t_idx, ptr_pload->request.f_idx,                 \
            ptr_pload->request.e_idx, ptr_pload->data_size);                    \
    } while(0)
#else
#define voice_vlan_db_dbmsg_dump(size, ptr_msg)
#endif /* VOICE_DEBUG */

#define OUI_CMP_LEN                 (3)

#define DEFAULT_VOICE_PORT_MODE     (0x1)
#define AUTH_PORT_CHANGE            (1)

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
#define DEFAULT_VOICE_SEC_MODE      (0x0)
#define SEC_PORT_CHANGE             (2)
#endif

#define DB_SEND_WAITTIME            (100)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    VOICE_ACL_PORT_JOINED = 0,    /* for the joined voice vlan ports */
    VOICE_ACL_PORT_UNAUTH,        /* for the auto mode ports and not yet join voice vlan */
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    VOICE_ACL_PORT_SECMODE,       /* for the ports that enable security mode */
#endif
    VOICE_ACL_PORT_LAST
} VOICE_ACL_PORT_T;

/* The structure Voice VLAN */
typedef struct VOICE_VLAN_S
{
    UI8_T            state;       /* The voice VLAN administrate state */
    UI16_T           vid;         /* The voice VLAN Id */
    UI16_T           vlan_eidx;   /* The voice VLAN entry index in VLAN_ENTRY */
    UI16_T           pri;         /* The voice VLAN priority */
    ether_addr_t     ouis[MAX_OUI_NUM]; /* The oui list */
    UI32_T           default_auto_ports;  /* The auto mode port bitmap in factory defaults or code */
    UI32_T           auto_ports;  /* The auto mode port bitmap including auth and unauth */
    UI32_T           auth_auto_ports;  /* The authorized auto mode port bitmap */
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    UI32_T           default_sec_ports;  /* The security mode port bitmap in factory defaults or code */
    UI32_T           sec_ports;   /* The security mode port bitmap */
#endif
} VOICE_VLAN_T;

/* The structure for ACL function call */
typedef struct VOICE_ACL_S
{
    MW_PORT_BITMAP_T port_bmp;    /* The method bitmap */
    UI8_T            index;       /* The number of acl */
    UI8_T            type;        /* The rule type in VOICE_ACL_PORT_T */
    UI16_T           vid;         /* The voice VLAN Id */
    UI8_T            pri;         /* The voice VLAN priority */
    ether_addr_t     mac;         /* The OUI */
} VOICE_ACL_T;

typedef struct
{
    threadhandle_t      ptr_voice_vland;
    StaticTask_t        voice_stack;
    StackType_t         voice_task_stack[VOICE_VLAN_STACK_SIZE];
    NET_FILTER_T        voice_pkt_netf;
    msghandle_t         voice_q_handle;
} VOICE_CONTEXT_T;


/* GLOBAL VARIABLE DECLARATIONS
 */
static VOICE_CONTEXT_T  voice_ctx = {0};
static VOICE_VLAN_T voice_cfg;
static UI16_T       voice_vlan_acl_id[VOICE_VLAN_MAX_ACL_NUM];
static UI32_T       voice_vlan_member;            /* The operational member ports of the voice VLAN */
static VLAN_ENTRY_INFO_T voice_vlan_entry;
static UI32_T       trunk_members[MAX_TRUNK_NUM]; /* The trunk member records */
static UI32_T       _voice_vlan_ratelimit_id;     /* All oui shared */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* DB */
static MW_ERROR_NO_T _voice_vlan_db_send(UI8_T method, UI8_T table, UI8_T field, UI16_T entry, UI16_T size, const void* ptr_data, DB_MSG_T **pptr_db_msg);
static MW_ERROR_NO_T _voice_vlan_db_notify(DB_MSG_T *ptr_msg);
static MW_ERROR_NO_T _voice_vlan_db_subscribe(void);
/* MAC_RCV */
static MW_ERROR_NO_T _voice_vlan_rx(struct pbuf *ptr_pbuf);
static MW_ERROR_NO_T _voice_vlan_socket_create(void);
/* ACL */
static MW_ERROR_NO_T _voice_vlan_rate_limit_add(void);
static void _voice_vlan_rate_limit_del(void);
static MW_ERROR_NO_T _voice_vlan_acl_add(UI8_T backsearch, const VOICE_ACL_T voice_acl);
static MW_ERROR_NO_T _voice_vlan_acl_del(UI8_T acl_index);
static void _voice_vlan_acl_port_change(UI8_T change_type);
static void _voice_vlan_acl_oui_delete(UI8_T oui_idx);
static void _voice_vlan_acl_oui_update(void);
static void _voice_vlan_acl_oui_add(UI8_T oui_idx, ether_addr_t oui);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
static void _voice_vlan_acl_secmode_add(void);
#endif
/* Task */
static void _voice_vlan_free_resource(void);
static MW_ERROR_NO_T _voice_vlan_data_init(void);
static void _voice_vlan_process(void *ptr_pvParameters);

/* LOCAL SUBPROGRAM DEFINITIONS
 */
static MW_ERROR_NO_T
_voice_vlan_db_send(
    UI8_T method,
    UI8_T table,
    UI8_T field,
    UI16_T entry,
    UI16_T size,
    const void *ptr_data,
    DB_MSG_T **pptr_db_msg)
{
    MW_ERROR_NO_T ret;
    DB_MSG_T *ptr_msg = NULL;
    DB_PAYLOAD_T *ptr_pload = NULL;
    UI32_T msg_size;

    /* Malloc communitcation format */
    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    ret = osapi_calloc(msg_size, VOICE_VLAN_TASK_NAME, (void**)&ptr_msg);
    if(MW_E_OK != ret)
    {
        VOICE_ERR_PRINT("%s", "Error! Failed to allocate memory");
        return ret;
    }
    VOICE_DBG_PRINT("Allocate DB MSG %p", ptr_msg);
    osapi_strncpy(ptr_msg->cq_name, VOICE_VLAN_QUEUE_NAME, DB_Q_NAME_SIZE);

    ptr_msg->method = method;
    ptr_msg->type.count = VOICE_VLAN_PAYLOAD_NUMBER;
    ptr_pload = (DB_PAYLOAD_T*)&(ptr_msg->ptr_payload);
    ptr_pload->request.t_idx = table;
    ptr_pload->request.f_idx = field;
    ptr_pload->request.e_idx = entry;
    ptr_pload->data_size = size;
    if (size > 0)
    {
        memcpy((void *)&(ptr_pload ->ptr_data), ptr_data, size);
    }

    if (dbapi_dbisReady() != MW_E_OK)
    {
        VOICE_DBG_PRINT("%s", "DB is not yet ready.");
        return MW_E_NOT_INITED;
    }
    voice_vlan_db_dbmsg_dump(size, ptr_msg);
    ret = dbapi_sendMsg(ptr_msg, DB_SEND_WAITTIME);
    if(MW_E_OK != ret)
    {
        /* This message could not be send, drop it */
        VOICE_ERR_PRINT("%s", "Error! Failed to send data to DB.");
        osapi_free(ptr_msg);
    }
    else
    {
        if (pptr_db_msg != NULL)
        {
            (*pptr_db_msg) = ptr_msg;
        }
    }
    return ret;
}


static MW_ERROR_NO_T
_voice_vlan_update_db_voice_vlan(
    void)
{
    DB_VOICE_VLAN_INFO_T voice_data;

    voice_data.vlan_state = voice_cfg.state;
    voice_data.vlan_id = voice_cfg.vid;
    voice_data.vlan_priority = voice_cfg.pri;
    voice_data.port_mode = voice_cfg.auto_ports;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    voice_data.security_mode = voice_cfg.sec_ports;
#endif

    VOICE_DBG_PRINT("update voice_vlan state: %s\n", (voice_cfg.state == TRUE)?"ENABLED":"DISABLED");
    VOICE_DBG_PRINT("update voice_vlan ID: %d\n", voice_cfg.vid);
    VOICE_DBG_PRINT("update voice_vlan priority: %d\n", voice_cfg.pri);
    VOICE_DBG_PRINT("Update voice_vlan portmode 0x%08X", voice_cfg.auto_ports);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    VOICE_DBG_PRINT("Update voice_vlan secmode 0x%08X", voice_cfg.sec_ports);
#endif
    if (MW_E_OK != _voice_vlan_db_send(M_UPDATE, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, sizeof(DB_VOICE_VLAN_INFO_T), (const void *)&voice_data, NULL))
    {
        VOICE_ERR_PRINT("%s", "Cannot update voice vlan info configuration");
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_update_db_voice_oper_member(
    void)
{
    VOICE_DBG_PRINT("Update voice_vlan operational member 0x%08X", voice_vlan_member);
    if (MW_E_OK != _voice_vlan_db_send(M_UPDATE, VOICE_OPER_INFO, VOICE_OPER_MEMBER_STATE, 0, sizeof(voice_vlan_member), (const void *)&voice_vlan_member, NULL))
    {
        VOICE_ERR_PRINT("%s", "Cannot update voice_vlan operational member");
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_get_db_port_cfg_vlanlist(
        void)
{
    UI16_T data_size = 0;
    DB_REQUEST_TYPE_T req = {
        .t_idx = PORT_CFG_INFO,
        .f_idx = PORT_VLAN_LIST,
        .e_idx = DB_ALL_ENTRIES,
    };

    if (MW_E_OK != dbapi_getDataSize(req, &data_size))
    {
        VOICE_ERR_PRINT("%s", "Cannot get data size of this request.");
        return MW_E_OP_INCOMPLETE;
    }

    if (MW_E_OK != _voice_vlan_db_send(M_GET, req.t_idx, req.f_idx, req.e_idx, data_size, NULL, NULL))
    {
        VOICE_ERR_PRINT("%s", "Cannot get data");
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_get_db_vlan_entry(
    UI8_T field,
    UI16_T entry)
{
    UI16_T data_size = 0;
    DB_REQUEST_TYPE_T req = {
        .t_idx = VLAN_ENTRY,
        .f_idx = field,
        .e_idx = entry,
    };

    VOICE_DBG_PRINT("Request to get VLAN entry/%d/%d", field, entry);

    if (MW_E_OK != dbapi_getDataSize(req, &data_size))
    {
        VOICE_ERR_PRINT("%s", "Cannot get data size of this request.");
        return MW_E_OP_INCOMPLETE;
    }

    if (MW_E_OK != _voice_vlan_db_send(M_GET, req.t_idx, req.f_idx, req.e_idx, data_size, NULL, NULL))
    {
        VOICE_ERR_PRINT("%s", "Cannot get vlan entry");
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_update_voice_member(
    void)
{
    /* Update DB data */
    UI32_T orig_member;
    UI32_T tmp_member;

    if (voice_cfg.vid == 0)
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return MW_E_OK;
    }
    if (voice_vlan_entry.vlan_id != 0)
    {
        /* vlan entry */
        orig_member = voice_vlan_entry.tagged_member;
        tmp_member = (voice_cfg.auth_auto_ports << 1U) & ~(voice_vlan_entry.untagged_member);
        voice_vlan_entry.tagged_member |= tmp_member;
        if (orig_member != voice_vlan_entry.tagged_member)
        {
            VOICE_DBG_PRINT("Update VOICE VLAN ENTRY %d tagged member: %08x", voice_vlan_entry.vlan_id, voice_vlan_entry.tagged_member);
            if (MW_E_OK != _voice_vlan_get_db_port_cfg_vlanlist())
            {
                return MW_E_OP_INCOMPLETE;
            }
            if (MW_E_OK != _voice_vlan_db_send(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, voice_cfg.vlan_eidx, sizeof(VLAN_ENTRY_INFO_T), (const void *)&voice_vlan_entry, NULL))
            {
                VOICE_ERR_PRINT("%s", "Cannot update voice vlan operational member");
                return MW_E_OP_INCOMPLETE;
            }
        }
    }

    /* voice vlan operational memberstate */
    _voice_vlan_update_db_voice_oper_member();

    /* Update OUI ACL rules port bitmap */
    _voice_vlan_acl_port_change(VOICE_ACL_PORT_JOINED);
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_get_trunk_member(
    UI8_T source_port,
    UI32_T *trunk_member)
{
    UI8_T i;
    MW_ERROR_NO_T rc = MW_E_ENTRY_NOT_FOUND;

    MW_PARAM_CHK((source_port >= PLAT_MAX_PORT_NUM), MW_E_BAD_PARAMETER);

    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        if (trunk_members[i] & (1U << source_port))
        {
            // Get all members of the trunk group
            *trunk_member = trunk_members[i];
            rc = MW_E_OK;
            break;
        }
    }
    return rc;
}

static UI8_T
_voice_vlan_get_trunk_root_port(
    UI32_T trunk_member)
{
    UI8_T root_port = 0;
    UI8_T port = 0;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        if(trunk_member & (1U << port))
        {
            // Get the root port of the trunk group
            root_port = port;
            break;
        }
    }

    return root_port;
}

static UI8_T
_voice_vlan_reset_trunk_port_config(
    UI32_T trunk_members)
{
    UI8_T port_change = 0;
    UI8_T i = 0;
    UI32_T port_mode_bmp = voice_cfg.auto_ports;
    UI32_T voice_vlan_bmp = voice_vlan_member;
    UI32_T tmp_member;
    UI8_T port_mode;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    UI8_T sec_mode;
    UI32_T sec_mode_bmp = voice_cfg.sec_ports;
#endif

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (trunk_members & (1U << i))
        {
            port_mode = ((voice_cfg.default_auto_ports & (1U << i)) != 0);
            if (port_mode == TRUE)
            {
                port_mode_bmp |= (1U << i);
            }
            else
            {
                port_mode_bmp &= ~(1U << i);
            }

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
            sec_mode = ((voice_cfg.default_sec_ports & (1U << i)) != 0);
            if (sec_mode == TRUE)
            {
                sec_mode_bmp |= (1U << i);
            }
            else
            {
                sec_mode_bmp &= ~(1U << i);
            }
#endif
        }
    }
    voice_cfg.auth_auto_ports &= ~trunk_members;
    BITMAP_PORT_ADD(tmp_member, voice_vlan_entry.tagged_member, voice_vlan_entry.untagged_member);
    voice_vlan_member = voice_cfg.auth_auto_ports | (tmp_member >> 1U);
    if ((voice_vlan_member != voice_vlan_bmp) || (port_mode_bmp != voice_cfg.auto_ports))
    {
        port_change |= AUTH_PORT_CHANGE;
        voice_cfg.auto_ports = port_mode_bmp;
    }

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    if (sec_mode_bmp != voice_cfg.sec_ports)
    {
        port_change |= SEC_PORT_CHANGE;
        voice_cfg.sec_ports = sec_mode_bmp;
    }
#endif
    return port_change;
}

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
static UI8_T
_voice_vlan_set_trunk_port_voice_config(
    UI32_T trunk_members,
    UI8_T port_mode,
    UI8_T auth_port_mode,
    UI8_T sec_mode)
#else
static UI8_T
_voice_vlan_set_trunk_port_voice_config(
    UI32_T trunk_members,
    UI8_T port_mode,
    UI8_T auth_port_mode)
#endif
{
    UI8_T port_change = 0;
    UI32_T port_mode_bmp = voice_cfg.auto_ports;
    UI32_T voice_vlan_bmp = voice_vlan_member;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    UI32_T sec_mode_bmp = voice_cfg.sec_ports;
#endif

    VOICE_DBG_PRINT("update Trunk member 0x%08X with portmode %s and state %s", trunk_members,
            (port_mode == TRUE)? "TRUE": "FALSE",
            (auth_port_mode == TRUE)? "TRUE": "FALSE");
    if (port_mode == TRUE)
    {
        port_mode_bmp |= trunk_members;
    }
    else
    {
        port_mode_bmp &= ~trunk_members;
    }

    if (voice_cfg.auto_ports != port_mode_bmp)
    {
        port_change |= AUTH_PORT_CHANGE;
        voice_cfg.auto_ports = port_mode_bmp;
    }

    if (auth_port_mode == TRUE)
    {
        voice_vlan_member |= trunk_members;
        voice_cfg.auth_auto_ports |= trunk_members;
    }
    else
    {
        voice_vlan_member &= ~trunk_members;
        voice_cfg.auth_auto_ports &= ~trunk_members;
    }
    if (voice_vlan_member != voice_vlan_bmp)
    {
        port_change |= AUTH_PORT_CHANGE;
    }

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    if (sec_mode == TRUE)
    {
        sec_mode_bmp |= trunk_members;
    }
    else
    {
        sec_mode_bmp &= ~trunk_members;
    }

    if (sec_mode_bmp != voice_cfg.sec_ports)
    {
        port_change |= SEC_PORT_CHANGE;
        voice_cfg.sec_ports = sec_mode_bmp;
    }
#endif
    return port_change;
}

/* handle the trunk port notification  */
static MW_ERROR_NO_T
_voice_vlan_db_trunk_port(
    UI8_T method,
    DB_REQUEST_TYPE_T request,
    void *ptr_data)
{
    UI32_T tmp_member = 0;
    UI8_T change = 0;
    UI8_T root_port;
    UI8_T port_mode;
    UI8_T auth_port_mode;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    UI8_T sec_mode;
#endif
    UI32_T old_member = 0;

    MW_CHECK_PTR(ptr_data);
    if ((request.f_idx == DB_ALL_FIELDS) ||
        (request.f_idx == TRUNK_MEMBERS))
    {
        if (request.e_idx == DB_ALL_ENTRIES)
        {
            UI8_T *ptr_entries = (UI8_T *)ptr_data;
            DB_TRUNK_PORT_T trunk_info;
            UI8_T i;
            if (method == M_DELETE)
            {
                /* delete all trunk groups, reset to default */
                for (i = 0; i < MAX_TRUNK_NUM; i++)
                {
                    tmp_member |= trunk_members[i];
                }
                memset(trunk_members, 0, sizeof(DB_TRUNK_PORT_T));
                change = _voice_vlan_reset_trunk_port_config(tmp_member);
            }
            else
            {
                for (i = 0; i < MAX_TRUNK_NUM; i++)
                {
                    memcpy(&trunk_info, (void *)(ptr_entries + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
                    trunk_members[i] = trunk_info.members.member_bmp;
                    if (trunk_members[i] == 0)
                    {
                        continue;
                    }
                    tmp_member = trunk_members[i];
                    /* find the root port of the turnk */
                    root_port = _voice_vlan_get_trunk_root_port(tmp_member);
                    port_mode = ((voice_cfg.auto_ports & (1U << root_port)) != 0);
                    auth_port_mode = ((voice_cfg.auth_auto_ports & (1U << root_port)) != 0);
                    old_member = voice_vlan_member;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
                    sec_mode = ((voice_cfg.sec_ports & (1U << root_port)) != 0);
                    change |= _voice_vlan_set_trunk_port_voice_config(tmp_member, port_mode, auth_port_mode, sec_mode);
#else
                    change |= _voice_vlan_set_trunk_port_voice_config(tmp_member, port_mode, auth_port_mode);
#endif
                    if ((old_member != voice_vlan_member) && (old_member & (1U << root_port)))
                    {
                        voice_vlan_member |= tmp_member;
                        change |= AUTH_PORT_CHANGE;
                    }
                }
            }
        }
        else
        {
            if (method == M_DELETE)
            {
                tmp_member = trunk_members[request.e_idx - 1];
                trunk_members[request.e_idx - 1] = 0;
                change = _voice_vlan_reset_trunk_port_config(tmp_member);
            }
            else
            {
                /* to reslove coverity */
                UI8_T *ptr_entries = (UI8_T*)ptr_data;
                DB_TRUNK_PORT_T trunk_info;

                memcpy(&trunk_info, (void *)ptr_entries, sizeof(DB_TRUNK_PORT_T));
                if (trunk_members[request.e_idx - 1] != trunk_info.members.member_bmp)
                {
                    /* reset to default port mode */
                    tmp_member = trunk_members[request.e_idx - 1];
                    trunk_members[request.e_idx - 1] = 0;
                    change = _voice_vlan_reset_trunk_port_config(tmp_member);
                }
                /* update to members */
                trunk_members[request.e_idx - 1] = trunk_info.members.member_bmp;
                VOICE_DBG_PRINT("DB Trunk id %d member port: 0x%08X", request.e_idx, trunk_members[request.e_idx - 1]);
                if (trunk_members[request.e_idx - 1] != 0)
                {
                    tmp_member = trunk_members[request.e_idx - 1];
                    /* find the root port of the turnk */
                    root_port = _voice_vlan_get_trunk_root_port(tmp_member);
                    port_mode = ((voice_cfg.auto_ports & (1U << root_port)) != 0);
                    auth_port_mode = ((voice_cfg.auth_auto_ports & (1U << root_port)) != 0);
                    old_member = voice_vlan_member;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
                    sec_mode = ((voice_cfg.sec_ports & (1U << root_port)) != 0);
                    change = _voice_vlan_set_trunk_port_voice_config(tmp_member, port_mode, auth_port_mode, sec_mode);
#else
                    change = _voice_vlan_set_trunk_port_voice_config(tmp_member, port_mode, auth_port_mode);
#endif
                    if ((old_member != voice_vlan_member) && (old_member & (1U << root_port)))
                    {
                        voice_vlan_member |= tmp_member;
                        change |= AUTH_PORT_CHANGE;
                    }
                }
            }
        }
    }

    if (change == 0)
    {
        return MW_E_OK;
    }

    _voice_vlan_update_db_voice_vlan();
    if (change & AUTH_PORT_CHANGE)
    {
        _voice_vlan_update_voice_member();
    }
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    else if (change & SEC_PORT_CHANGE)
    {
        _voice_vlan_acl_port_change(VOICE_ACL_PORT_SECMODE);
    }
#endif
    else {/* no chagne do nothing */}

    return MW_E_OK;
}

/* handle the vlan entry notification  */
static MW_ERROR_NO_T
_voice_vlan_db_vlan_entry(
    UI8_T method,
    DB_REQUEST_TYPE_T request,
    void *ptr_data)
{
    UI32_T org_vlan_member = voice_vlan_member;
    UI32_T tmp_member = 0;
    UI32_T org_member;

    MW_CHECK_PTR(ptr_data);

    if (voice_cfg.vid == 0)
    {
        /* The Voice VLAN not yet enabled or not yet assign a valid VID */
        return MW_E_OK;
    }

    if (request.f_idx == DB_ALL_FIELDS)
    {
        if (request.e_idx == DB_ALL_ENTRIES)
        {
            DB_VLAN_ENTRY_T *ptr_entries = (DB_VLAN_ENTRY_T *)ptr_data;
            /* whole VLAN_ENTRY table */
            UI16_T i;
            for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
            {
                if (voice_cfg.vid != ptr_entries->vlan_id[i])
                {
                    continue;
                }
                VOICE_DBG_PRINT("DB VLAN_ENTRY id %d at eidx %d  member port: 0x%08X, tagged: 0x%08X, untagged: 0x%08X",
                        ptr_entries->vlan_id[i], i, ptr_entries->port_member[i], ptr_entries->tagged_member[i], ptr_entries->untagged_member[i]);
                if (method == M_DELETE)
                {
                    /* Should not happen, just in case */
                    /* clear manual port members since the entry been deleted */
                    voice_vlan_member = 0;
                    voice_cfg.auth_auto_ports = 0;
                    voice_cfg.vlan_eidx = VOICE_VLAN_INVALID;
                    memset((void *)&voice_vlan_entry, 0, sizeof(VLAN_ENTRY_INFO_T));
                }
                else
                {
                    /* Update the voice_vlan members and vlan_eidx exlcude the CPU port */
                    voice_vlan_member = voice_cfg.auth_auto_ports;
                    BITMAP_PORT_ADD(tmp_member, ptr_entries->tagged_member[i], ptr_entries->untagged_member[i]);
                    voice_vlan_member |= (tmp_member >> 1U);
                    voice_cfg.vlan_eidx = i + 1;
                    voice_vlan_entry.vlan_id = ptr_entries->vlan_id[i];
                    voice_vlan_entry.port_member = ptr_entries->port_member[i];
                    voice_vlan_entry.tagged_member = ptr_entries->tagged_member[i];
                    voice_vlan_entry.untagged_member = ptr_entries->untagged_member [i];
                }
                break;
            }
        }
        else
        {
            VLAN_ENTRY_INFO_T *vlan_entry = (VLAN_ENTRY_INFO_T *)ptr_data;
            /* specific entry in VLAN_ENTRY */
            if (voice_cfg.vid != vlan_entry->vlan_id)
            {
                return MW_E_OK;
            }
            VOICE_DBG_PRINT("DB VLAN_ENTRY id %d member port: 0x%08X, tagged: 0x%08X, untagged: 0x%08X",
                        vlan_entry->vlan_id, vlan_entry->port_member, vlan_entry->tagged_member, vlan_entry->untagged_member);
            if (method == M_DELETE)
            {
                /* clear manual port members since the entry been deleted */
                voice_vlan_member = 0;
                voice_cfg.auth_auto_ports = 0;
                voice_cfg.vlan_eidx = VOICE_VLAN_INVALID;
                memset((void *)&voice_vlan_entry, 0, sizeof(VLAN_ENTRY_INFO_T));
            }
            else
            {
                /* Update the voice_vlan members and vlan_eidx and exclude the CPU port */
                BITMAP_PORT_ADD(tmp_member, vlan_entry->tagged_member, vlan_entry->untagged_member);
                BITMAP_PORT_ADD(org_member, voice_vlan_entry.tagged_member, voice_vlan_entry.untagged_member);
                tmp_member = (tmp_member >> 1U);
                org_member = (org_member >> 1U);
                if ((org_member & tmp_member) == org_member)
                {
                    /* add some ports */
                    voice_vlan_member = tmp_member | voice_cfg.auth_auto_ports;
                }
                else
                {
                    /* both add and delte ports */
                    voice_cfg.auth_auto_ports &= tmp_member;
                    voice_vlan_member = tmp_member | voice_cfg.auth_auto_ports;
                }
                voice_cfg.vlan_eidx = request.e_idx;
                memcpy((void *)&voice_vlan_entry, (const void *)vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
            }
        }
    }
    else
    {
        if ((request.f_idx == VLAN_ID) &&
           (request.e_idx == DB_ALL_ENTRIES))
        {
            UI16_T *vlan_ids = (UI16_T *)ptr_data;
            UI16_T eidx;
            for (eidx = 0; eidx < MAX_VLAN_ENTRY_NUM; eidx++)
            {
                if (voice_cfg.vid == vlan_ids[eidx])
                {
                    voice_cfg.vlan_eidx = eidx + 1;
                    /* Get the vlan_entry port_member only */
                    _voice_vlan_get_db_vlan_entry(DB_ALL_FIELDS, eidx);
                    break;
                }
            }
        }
        else
        {
            return MW_E_OK;
        }
    }

    if (org_vlan_member != voice_vlan_member)
    {
        _voice_vlan_update_voice_member();
    }
    return MW_E_OK;
}

/* handle the oui entry notification  */
static MW_ERROR_NO_T
_voice_vlan_db_oui_entry(
    UI8_T method,
    DB_REQUEST_TYPE_T request,
    void *ptr_data)
{
    MW_CHECK_PTR(ptr_data);
    UI8_T oui_idx;

    if (request.f_idx == DB_ALL_FIELDS)
    {
        ether_addr_t tmp_mac;
        if (request.e_idx == DB_ALL_ENTRIES)
        {
            UI8_T i;
            DB_OUI_ENTRY_T *oui_entries = (DB_OUI_ENTRY_T *)ptr_data;
            for (i = 0; i < MAX_OUI_NUM; i++)
            {
                if (VLAN_MAC_VOICE_ENABLE != oui_entries->type[i])
                {
                    continue;
                }
                memset(&tmp_mac, 0, sizeof(ether_addr_t));
                VOICE_DBG_PRINT("DB update OUI: %02x:%02x:%02x", tmp_mac.octet[0], tmp_mac.octet[1], tmp_mac.octet[2]);
                for (oui_idx = 0; oui_idx < MAX_OUI_NUM; oui_idx++)
                {
                    /* clear the oui */
                    if (method == M_DELETE)
                    {
                        memcpy(tmp_mac.octet, oui_entries->mac[i], OUI_CMP_LEN);
                        if (0 == memcmp(tmp_mac.octet, voice_cfg.ouis[oui_idx].octet, ETHER_ADDR_LEN))
                        {
                            if (voice_cfg.vid != 0)
                            {
                                _voice_vlan_acl_oui_delete(oui_idx);
                            }
                            /* reset the OUI in voice_cfg */
                            memset(voice_cfg.ouis[oui_idx].octet, 0, ETHER_ADDR_LEN);
                            break;
                        }
                    }
                    else
                    {
                        /* Add oui entries */
                        if (0 == memcmp(tmp_mac.octet, voice_cfg.ouis[oui_idx].octet, ETHER_ADDR_LEN))
                        {
                            memcpy(tmp_mac.octet, oui_entries->mac[i], OUI_CMP_LEN);
                            if (voice_cfg.vid != 0)
                            {
                                _voice_vlan_acl_oui_add(oui_idx, tmp_mac);
                            }
                            /* add the OUI in voice_cfg */
                            memcpy(voice_cfg.ouis[oui_idx].octet, tmp_mac.octet, ETHER_ADDR_LEN);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            VOVLAN_OUI_T oui;
            UI8_T type;
            memcpy(&oui, ptr_data, sizeof(VOVLAN_OUI_T));
            type = *((UI8_T *)(ptr_data + sizeof(VOVLAN_OUI_T)));
            if (VLAN_MAC_VOICE_ENABLE != type)
            {
                return MW_E_OK;
            }
            memset(&tmp_mac, 0, sizeof(ether_addr_t));
            for (oui_idx = 0; oui_idx < MAX_OUI_NUM; oui_idx++)
            {
                /* clear the oui */
                if (method == M_DELETE)
                {
                    memcpy(tmp_mac.octet, oui, OUI_CMP_LEN);
                    if (0 == memcmp(tmp_mac.octet, voice_cfg.ouis[oui_idx].octet, ETHER_ADDR_LEN))
                    {
                        if (voice_cfg.vid != 0)
                        {
                            _voice_vlan_acl_oui_delete(oui_idx);
                        }
                        /* reset the OUI in voice_cfg */
                        memset(voice_cfg.ouis[oui_idx].octet, 0, ETHER_ADDR_LEN);
                        break;
                    }
                }
                else
                {
                    /* Add oui entries */
                    if (0 == memcmp(tmp_mac.octet, voice_cfg.ouis[oui_idx].octet, ETHER_ADDR_LEN))
                    {
                        memcpy(tmp_mac.octet, oui, OUI_CMP_LEN);
                        if (voice_cfg.vid != 0)
                        {
                            _voice_vlan_acl_oui_add(oui_idx, tmp_mac);
                        }
                        /* add the OUI in voice_cfg */
                        memcpy(voice_cfg.ouis[oui_idx].octet, tmp_mac.octet, ETHER_ADDR_LEN);
                        break;
                    }
                }
            }
        }
    }
    return MW_E_OK;
}

/* handle the voice vlan info notification  */
static MW_ERROR_NO_T
_voice_vlan_db_voice_vlan_info(
    UI8_T method,
    DB_REQUEST_TYPE_T request,
    void *ptr_data)
{
    UI8_T update_port = FALSE;
    UI8_T update_oui = FALSE;
    UI32_T org_vlan_member = voice_vlan_member;

    MW_CHECK_PTR(ptr_data);

    if (method == M_RESPONSE)
    {
        if (request.f_idx == VOICE_PORT_MODE)
        {
            memcpy(&(voice_cfg.default_auto_ports), ptr_data, sizeof(UI32_T));
        }
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        if (request.f_idx == VOICE_SECURITY_MODE)
        {
            memcpy(&(voice_cfg.default_sec_ports), ptr_data, sizeof(UI32_T));
        }
#endif
        return MW_E_OK;
    }

    if ((request.f_idx == DB_ALL_FIELDS) &&
        (request.e_idx == DB_ALL_ENTRIES))
    {
        DB_VOICE_VLAN_INFO_T *ptr_vdata = (DB_VOICE_VLAN_INFO_T *)ptr_data;
        if (voice_cfg.state != ptr_vdata->vlan_state)
        {
            voice_cfg.state = ptr_vdata->vlan_state;
            update_oui = TRUE;
        }
        if (ptr_vdata->vlan_state == TRUE)
        {
            if (update_oui == TRUE)
            {
                if ((voice_cfg.vid == ptr_vdata->vlan_id) && (voice_cfg.vid != 0))
                {
                    /* get VLAN member to set the rules */
                    _voice_vlan_get_db_vlan_entry(DB_ALL_FIELDS, voice_cfg.vlan_eidx);
                }
            }
            /* auto mode port member */
            if (voice_cfg.auto_ports != ptr_vdata->port_mode)
            {
                /* Must update portbitmap for all OUI rules */
                if (voice_cfg.vid != 0)
                {
                  update_port = TRUE;
                }
                voice_cfg.auto_ports = ptr_vdata->port_mode;
            }
            /* voice priority */
            if (voice_cfg.pri != ptr_vdata->vlan_priority)
            {
                if (voice_cfg.vid != 0)
                {
                    /* Rules created, must update all rules */
                    update_oui = TRUE;
                }
                voice_cfg.pri = ptr_vdata->vlan_priority;
            }
            /* VID */
            if (voice_cfg.vid != ptr_vdata->vlan_id)
            {
                if (voice_cfg.vid != 0)
                {
                    /* Rules created, must update all rules */
                    update_oui = TRUE;
                    voice_vlan_member = 0;
                    voice_cfg.auth_auto_ports = 0;
                    voice_cfg.vlan_eidx = VOICE_VLAN_INVALID;
                    memset((void *)&voice_vlan_entry, 0, sizeof(VLAN_ENTRY_INFO_T));
                }
                voice_cfg.vid = ptr_vdata->vlan_id;
                if (ptr_vdata->vlan_id != 0)
                {
                    /* get VLAN entry to set the rules */
                    _voice_vlan_get_db_vlan_entry(DB_ALL_FIELDS, DB_ALL_ENTRIES);
                }
            }
            if (update_oui == TRUE)
            {
                _voice_vlan_acl_oui_update();
            }
            else if (update_port == TRUE)
            {
                /* Update OUI ACL rules port bitmap */
                _voice_vlan_acl_port_change(VOICE_ACL_PORT_JOINED);
            }
            else {/* Do nothing */}
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
            if (voice_cfg.sec_ports != ptr_vdata->security_mode)
            {
                voice_cfg.sec_ports = ptr_vdata->security_mode;
                /* Update OUI ACL rules port bitmap */
                _voice_vlan_acl_port_change(VOICE_ACL_PORT_SECMODE);
            }
#endif
        }
        else
        {
            if ((voice_cfg.vid != 0) && (update_oui == TRUE))
            {
                UI8_T oui_idx = 0;
                /* The voice vlan is enabled. delete the rules */
                for (oui_idx = 0; oui_idx < MAX_OUI_NUM; oui_idx++)
                {
                    _voice_vlan_acl_oui_delete(oui_idx);
                }
                voice_vlan_member = 0;
            }
            voice_cfg.vid = ptr_vdata->vlan_id;
            voice_cfg.pri = ptr_vdata->vlan_priority;
            voice_cfg.auto_ports = ptr_vdata->port_mode;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
            voice_cfg.sec_ports = ptr_vdata->security_mode;
#endif
        }
    }

    if (org_vlan_member != voice_vlan_member)
    {
        _voice_vlan_update_db_voice_oper_member();
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_db_port_cfg_info(
    UI8_T method,
    DB_REQUEST_TYPE_T request,
    void *ptr_data)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T *ptr_vlan_list_tbl = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_senddata = NULL;
    UI16_T port, count = 0;
    UI16_T offset = 0;
    UI32_T msg_size = 0;
    UI32_T changedBmp = 0;

    MW_CHECK_PTR(ptr_data);

    if (method == M_RESPONSE)
    {
        if (request.e_idx != DB_ALL_ENTRIES)
        {
            /* respond to update the VLAN_LIST of specific port */
            return MW_E_OK;
        }

        ptr_vlan_list_tbl = (UI32_T *)ptr_data;
        for(port = 0; port < PLAT_MAX_PORT_NUM; port++)
        {
            if (BIT_CHK(voice_cfg.auth_auto_ports, port))
            {
                if (BIT_CHK(ptr_vlan_list_tbl[port], voice_cfg.vlan_eidx - 1))
                {
                    continue;
                }
                BIT_SET(ptr_vlan_list_tbl[port], (voice_cfg.vlan_eidx- 1));
                BIT_SET(changedBmp, port);
                count++;
            }
        }

        if (0 == count)
        {
            return MW_E_OK;
        }
        /* prepare to update DB */
        /* Malloc communitcation format */
        msg_size = DB_MSG_HEADER_SIZE + count * (DB_MSG_PAYLOAD_SIZE + sizeof(UI32_T));
        ret = osapi_calloc(msg_size, VOICE_VLAN_TASK_NAME, (void**)&ptr_msg);
        if(MW_E_OK != ret)
        {
            VOICE_ERR_PRINT("%s", "Error! Failed to allocate memory");
            return ret;
        }
        VOICE_DBG_PRINT("Allocate DB MSG %p", ptr_msg);
        offset = dbapi_setMsgHeader(ptr_msg, VOICE_VLAN_QUEUE_NAME, M_UPDATE, count);
        ptr_senddata = (UI8_T *)ptr_msg;
        for(port = 0; port < PLAT_MAX_PORT_NUM; port++)
        {
            if (BIT_CHK(changedBmp, port))
            {
                offset += dbapi_setMsgPayload(M_UPDATE,
                    PORT_CFG_INFO, PORT_VLAN_LIST, port + 1,
                    &ptr_vlan_list_tbl[port], ptr_senddata + offset);
                count--;
            }
            if (0 == count)
            {
                break;
            }
        }
        /* send request */
        ret = dbapi_sendRequesttoDb(msg_size, ptr_msg);
        if(MW_E_OK != ret)
        {
            /* This message could not be send, drop it */
            VOICE_ERR_PRINT("%s", "Error! Failed to send data to DB.");
            osapi_free(ptr_msg);
        }
    }
    return ret;
}

static MW_ERROR_NO_T
_voice_vlan_db_notify(
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_next_payload = NULL;

    ptr_data = (UI8_T *)&(ptr_msg->ptr_payload);
    do
    {
        if ((ptr_msg->method & M_RESPONSE) != 0)
        {
            if (MW_E_OK != ptr_msg->type.result)
            {
                VOICE_DBG_PRINT("Failed to update DB: ret %u", ptr_msg->type.result);
                break;
            }
            osapi_memcpy((void *)&request, (const void *)ptr_data, sizeof(DB_REQUEST_TYPE_T));
            ptr_data += (sizeof(DB_REQUEST_TYPE_T) + sizeof(UI16_T));
        }
        else
        {
            ret = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_next_payload);
            if (MW_E_OK != ret)
            {
                VOICE_ERR_PRINT("Failed to parse DB msg: ret %u", ret);
                break;
            }
        }
        voice_vlan_db_dbmsg_dump(data_size, ptr_msg);
        /* Handle the db notifications */
        switch(request.t_idx)
        {
            case TRUNK_PORT:
                ret = _voice_vlan_db_trunk_port(ptr_msg->method, request, (void *)ptr_data);
                break;
            case VLAN_ENTRY:
                ret = _voice_vlan_db_vlan_entry(ptr_msg->method, request, (void *)ptr_data);
                break;
            case OUI_ENTRY:
                ret = _voice_vlan_db_oui_entry(ptr_msg->method, request, (void *)ptr_data);
                break;
            case VOICE_VLAN_INFO:
                ret = _voice_vlan_db_voice_vlan_info(ptr_msg->method, request, (void *)ptr_data);
                break;
            case VOICE_OPER_INFO:
                ret = MW_E_OK;
                break;
            case PORT_CFG_INFO:
                ret = _voice_vlan_db_port_cfg_info(ptr_msg->method, request, (void *)ptr_data);
                break;
            default:
                VOICE_DBG_PRINT("Received unexpected message with tidx: %u", request.t_idx);
                break;
        }
        if (MW_E_OK != ret)
        {
            VOICE_ERR_PRINT("%s","Internal error");
            break;
        }
    } while ((MW_E_OK == ret) && (NULL != ptr_next_payload));
    VOICE_DBG_PRINT("free the notify message");
    MW_FREE(ptr_msg);

    return ret;
}

static MW_ERROR_NO_T
_voice_vlan_db_subscribe(
    void)
{
    DB_REQUEST_TYPE_T sub_req[] = {
        {VOICE_VLAN_INFO,  DB_ALL_FIELDS,  DB_ALL_ENTRIES},
        {VLAN_ENTRY,       DB_ALL_FIELDS,  DB_ALL_ENTRIES},
        {OUI_ENTRY,        DB_ALL_FIELDS,  DB_ALL_ENTRIES},
        {TRUNK_PORT,       TRUNK_MEMBERS,  DB_ALL_ENTRIES}
    };

    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI32_T msg_size = DB_MSG_HEADER_SIZE;
    UI16_T offset= 0;
    UI8_T db_idx = 0;
    UI8_T sub_count = 0;

    sub_count = (UI8_T)(sizeof(sub_req)/sizeof(DB_REQUEST_TYPE_T));
    msg_size += (DB_MSG_PAYLOAD_SIZE * sub_count);
    /* create the subscribe data payload */
    rc = osapi_calloc(msg_size, VOICE_VLAN_TASK_NAME, (void**)&ptr_msg);
    if(MW_E_OK != rc)
    {
        VOICE_ERR_PRINT("Error! Failed to allocate memory");
        return rc;
    }

    offset = dbapi_setMsgHeader(ptr_msg, VOICE_VLAN_QUEUE_NAME, M_SUBSCRIBE, sub_count);
    ptr_data = (UI8_T *)ptr_msg;
    for (db_idx = 0; db_idx < sub_count; db_idx++)
    {
        /* message header */
        offset += dbapi_setMsgPayload(M_SUBSCRIBE, sub_req[db_idx].t_idx, sub_req[db_idx].f_idx, sub_req[db_idx].e_idx, NULL, ptr_data + offset);
    }

    /* send request */
    rc = dbapi_sendRequesttoDb(msg_size, ptr_msg);
    if (MW_E_OK != rc)
    {
        VOICE_ERR_PRINT("Failed to send message to DB Queue");
        osapi_free(ptr_msg);
        return rc;
    }
    VOICE_DBG_PRINT("Allocate DB MSG %p with size %d", ptr_msg, msg_size);
    return rc;
}

/* MAC_RCV */
static MW_ERROR_NO_T
_voice_vlan_rx(
    struct pbuf *ptr_pbuf)
{
    UI8_T source_port;
    UI8_T oui_idx;
    UI32_T tmp_member;

    if ((voice_cfg.vid == 0) || (ptr_pbuf == NULL))
    {
        /* Not yet enabled */
        return MW_E_OK;
    }
    VOICE_DBG_PRINT("Voice VLAN received the packet %p from MAC_RCV", ptr_pbuf);
    if (ptr_pbuf->vlan_hdr.vid == voice_cfg.vid)
    {
        VOICE_DBG_PRINT("Pkt Voice VLAN with VLAN ID %u and priority %d", ptr_pbuf->vlan_hdr.vid, ptr_pbuf->vlan_hdr.priority);
        source_port = ptr_pbuf->stag_hdr.rx_hdr.sp - 1;

        for (oui_idx = 0; oui_idx < MAX_OUI_NUM; oui_idx++)
        {
            if (0 == memcmp((void *)&(ptr_pbuf->ether_hdr.src), voice_cfg.ouis[oui_idx].octet, OUI_CMP_LEN))
            {
                if ((voice_vlan_member & (1U << source_port)) == 0)
                {
                    /* Check if a trunk member */
                    if (MW_E_OK == _voice_vlan_get_trunk_member(source_port, &tmp_member))
                    {
                        VOICE_DBG_PRINT("Add trunk members (%08x) to oui index %d", tmp_member, oui_idx);
                        voice_cfg.auth_auto_ports |= tmp_member;
                    }
                    else
                    {
                        VOICE_DBG_PRINT("Add port (%u) to oui index %d", source_port, oui_idx);
                        voice_cfg.auth_auto_ports |=  (1U << source_port);
                    }
                    voice_vlan_member |= voice_cfg.auth_auto_ports;
                    _voice_vlan_update_voice_member();
                }
                break;
            }
        }
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_vlan_socket_create(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    memset(voice_ctx.voice_pkt_netf.name, 0, MSG_MAX_NAME_LEN);
    osapi_snprintf((C8_T*)voice_ctx.voice_pkt_netf.name, MSG_MAX_NAME_LEN, "%s", (C8_T*)VOICE_VLAN_QUEUE_NAME);
    voice_ctx.voice_pkt_netf.protocol = PROTO_VOICE_VLAN;
    voice_ctx.voice_pkt_netf.state = NET_FILTER_REGISTER;
    voice_ctx.voice_pkt_netf.handle = voice_ctx.voice_q_handle;
    ret = osapi_netRegister(&(voice_ctx.voice_pkt_netf));
    if (MW_E_OK != ret)
    {
        VOICE_ERR_PRINT("%s", "Error! Failed to register Voice VLAN netif");
    }

    return ret;
}

/* ACL */
static MW_ERROR_NO_T
_voice_acl_rule_form(
    const VOICE_ACL_T voice_acl,
    AIR_ACL_RULE_T   *ptr_rule)
{
    MW_CHECK_PTR(ptr_rule);
    ptr_rule->rule_en = TRUE;
    AIR_PORT_BITMAP_COPY(ptr_rule->portmap, voice_acl.port_bmp);
    AIR_PORT_DEL(ptr_rule->portmap, PLAT_CPU_PORT);
    ptr_rule->end = TRUE;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    if (voice_acl.type != VOICE_ACL_PORT_SECMODE)
    {
#endif
    memcpy(ptr_rule->key.smac, (const void *)&voice_acl.mac, 6);
    ptr_rule->mask.smac= 0x38;
    ptr_rule->field_valid |= (1U << AIR_ACL_SMAC_KEY);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    }
#endif
    ptr_rule->key.ctag= voice_acl.vid;
    ptr_rule->mask.ctag= 0x01;
    ptr_rule->field_valid |= (1U << AIR_ACL_CTAG_KEY);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_voice_acl_action_form(
    const VOICE_ACL_T voice_acl,
    AIR_ACL_ACTION_T *ptr_action)
{
    MW_CHECK_PTR(ptr_action);
    if (voice_acl.type == VOICE_ACL_PORT_JOINED)
    {
        ptr_action->port_fw = MW_ACL_ACT_PORT_FW_CPU_EXCLUDE;
        ptr_action->pri_user = voice_acl.pri;
        ptr_action->acl_vlan_vid = voice_acl.vid;
        ptr_action->field_valid |= ((1U << AIR_ACL_FW_PORT) | (1U << AIR_ACL_PRI) | (1U << AIR_ACL_VLAN));
    }
    else if(voice_acl.type == VOICE_ACL_PORT_UNAUTH)
    {
        ptr_action->port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
        ptr_action->pri_user = voice_acl.pri;
        ptr_action->acl_vlan_vid = voice_acl.vid;
        ptr_action->field_valid |= ((1U << AIR_ACL_FW_PORT) | (1U << AIR_ACL_PRI) | (1U << AIR_ACL_VLAN));
    }
    else
    {
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        ptr_action->port_fw = MW_ACL_ACT_PORT_FW_DROP;
        ptr_action->field_valid |= (1U << AIR_ACL_FW_PORT);
#endif
    }

    return MW_E_OK;
}

/* The caller should take mw_acl_mutex */
static MW_ERROR_NO_T
_voice_vlan_rate_limit_add(
    void)
{
    MW_ERROR_NO_T               ret;
    AIR_ERROR_NO_T              rc;
    AIR_DOS_RATE_LIMIT_CFG_T    rate_limit_cfg;
    UI32_T                      rate_limit_id = 0;
    UI32_T                      unit = 0;

    /* Get rate limit entry */
    ret = mw_acl_getAvailableAttackID(&rate_limit_id, &rate_limit_cfg, unit, MW_ATTACK_ID_DYNAMIC_MIN);
    if (MW_E_OK != ret)
    {
        VOICE_ERR_PRINT("Get VOICE_VLAN attack id failed\n");
        return ret;
    }

    VOICE_DBG_PRINT("Try to set VOICE_VLAN rate limit entry-id %d", rate_limit_id);
    rate_limit_cfg.pkt_thld = VOICE_VLAN_LIMIT_PKT_THLD;
    rate_limit_cfg.time_span = VOICE_VLAN_LIMIT_SPAN;
    rate_limit_cfg.block_time = VOICE_VLAN_LIMIT_BLK_TIME;
    rate_limit_cfg.tick_sel = VOICE_VLAN_LIMIT_TICK;
    rc = air_dos_setRateLimitCfg(unit, rate_limit_id, &rate_limit_cfg);
    if (AIR_E_OK != rc)
    {
        VOICE_ERR_PRINT("Add VOICE_VLAN attack-id %u failed, rc=%d", rate_limit_id, rc);
        return MW_E_OTHERS;
    }

    /* keep attack ID */
    _voice_vlan_ratelimit_id = rate_limit_id;

    return MW_E_OK;
}

/* The caller should take mw_acl_mutex */
static void
_voice_vlan_rate_limit_del(
    void)
{
    UI32_T i;
    UI32_T unit = 0;

    for (i = 0; i < VOICE_VLAN_MAX_ACL_NUM; i++)
    {
        if (VOICE_VLAN_INVALID != voice_vlan_acl_id[i])
        {
            return;
        }
    }

    if (VOICE_VLAN_INVALID != _voice_vlan_ratelimit_id)
    {
        air_dos_clearRateLimitCfg(unit, _voice_vlan_ratelimit_id);
        _voice_vlan_ratelimit_id = VOICE_VLAN_INVALID;
    }
}

/* Add an OUI */
static MW_ERROR_NO_T
_voice_vlan_acl_add(
    UI8_T backsearch,
    const VOICE_ACL_T voice_acl)
{
    MW_ERROR_NO_T ret;
    UI32_T unit = 0;
    UI16_T acl_rule_id;
    UI16_T acl_search_end;
    AIR_ACL_RULE_T acl_rule;
    AIR_ACL_ACTION_T acl_action;

    if (voice_cfg.vid == 0)
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return MW_E_OK;
    }

    if (VOICE_VLAN_INVALID != voice_vlan_acl_id[voice_acl.index])
    {
        /* rules been added before */
        VOICE_DBG_PRINT("rule (%d) is added", voice_acl.index);
        return MW_E_OK;
    }

    if (backsearch == TRUE)
    {
        /* for Security mode */
        acl_rule_id = MW_ACL_ID_DYNAMIC_MAX;
        acl_search_end = MW_ACL_ID_DYNAMIC_MIN;
    }
    else
    {
        /* for normal OUI rules */
        acl_rule_id = MW_ACL_ID_DYNAMIC_MIN;
        acl_search_end = MW_ACL_ID_DYNAMIC_MAX;
    }

    if(MW_E_OK == mw_acl_mutex_take())
    {
        if ((VOICE_ACL_PORT_UNAUTH == voice_acl.type)
                && (VOICE_VLAN_INVALID == _voice_vlan_ratelimit_id))
        {
            ret = _voice_vlan_rate_limit_add();
            if (MW_E_OK != ret)
            {
                mw_acl_mutex_release();
                return ret;
            }
        }

        while (acl_rule_id != acl_search_end)
        {
            VOICE_DBG_PRINT("Try to get VOICE_VLAN ACL entry-id %d", acl_rule_id);
            if (air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK)
            {
                AIR_ERROR_NO_T   rc;
                if (FALSE == acl_rule.rule_en)
                {
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    osapi_memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
                    ret = _voice_acl_rule_form(voice_acl, &acl_rule);
                    if (ret != MW_E_OK)
                    {
                        VOICE_ERR_PRINT("Internal error, result=%d", ret);
                        break;
                    }

                    ret = _voice_acl_action_form(voice_acl, &acl_action);
                    if (ret != MW_E_OK)
                    {
                        VOICE_ERR_PRINT("Internal error, result=%d", ret);
                        break;
                    }

                    if ((VOICE_ACL_PORT_UNAUTH == voice_acl.type)
                            && (VOICE_VLAN_INVALID != _voice_vlan_ratelimit_id))
                    {
                         acl_action.field_valid |= (1U << AIR_ACL_ATTACK);
                         acl_action.attack_rate_id = _voice_vlan_ratelimit_id;
                         VOICE_DBG_PRINT("Add VOICE_VLAN acl list %d attack-id %u",
                            voice_acl.index, _voice_vlan_ratelimit_id);
                    }

                    VOICE_DBG_PRINT("Try to set VOICE_VLAN ACL Rule entry-id %d", acl_rule_id);
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc != AIR_E_OK)
                    {
                        VOICE_ERR_PRINT("Add VOICE_VLAN ACL rule entry-id %d failed, rc=%d", acl_rule_id, rc);
                        break;
                    }
                    VOICE_DBG_PRINT("Try to set VOICE_VLAN ACL Action entry-id %d", acl_rule_id);
                    rc = air_acl_setAction(unit, acl_rule_id, &acl_action);
                    if (AIR_E_OK == rc)
                    {
                        voice_vlan_acl_id[voice_acl.index] = acl_rule_id;
                        VOICE_DBG_PRINT("Added VOICE_VLAN ACL entry-id %d in list %d.", acl_rule_id, voice_acl.index);
                    }
                    else
                    {
                        VOICE_ERR_PRINT("Add VOICE_VLAN ACL action entry-id %d failed, rc=%d.", acl_rule_id, rc);
                        air_acl_delRule(unit, acl_rule_id);
                    }
                    break;
                }
            }

            if (backsearch == TRUE)
            {
                acl_rule_id--;
            }
            else
            {
                acl_rule_id++;
            }
        }
        mw_acl_mutex_release();
    }
    if (acl_rule_id == acl_search_end)
    {
        VOICE_ERR_PRINT("Cannot add voice VLAN rule. Get free ACL rule-id failed!\n");
        return MW_E_ENTRY_REACH_END;
    }

    return MW_E_OK;
}

/* Delete an ACL rule and action */
static MW_ERROR_NO_T
_voice_vlan_acl_del(
    UI8_T acl_index)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T acl_rule_id;
    AIR_ACL_RULE_T acl_rule;

    MW_PARAM_CHK((acl_index >= VOICE_VLAN_MAX_ACL_NUM), MW_E_BAD_PARAMETER);
    acl_rule_id = voice_vlan_acl_id[acl_index];
    MW_PARAM_CHK((acl_rule_id >= MW_ACL_ID_DYNAMIC_MAX), MW_E_BAD_PARAMETER);
    VOICE_DBG_PRINT("Try to delete VOICE_VLAN ACL entry-id %d in list %d", acl_rule_id, acl_index);

    if(MW_E_OK == mw_acl_mutex_take())
    {
        if ((air_acl_getRule(unit, acl_rule_id, &acl_rule) != AIR_E_OK) ||
            (FALSE == acl_rule.rule_en))
        {
            VOICE_ERR_PRINT("Cannot delete voice VLAN rule. Get ACL rule-id %u failed!", acl_rule_id);
            voice_vlan_acl_id[acl_index] = VOICE_VLAN_INVALID;
            mw_acl_mutex_release();
            return MW_E_ENTRY_NOT_FOUND;
        }

        rc = air_acl_delRule(unit, acl_rule_id);
        if (rc != AIR_E_OK)
        {
            VOICE_ERR_PRINT("Delete VOICE_VLAN ACL rule entry-id %d failed, rc=%d", acl_rule_id, rc);
            mw_acl_mutex_release();
            return MW_E_OP_INCOMPLETE;
        }
        rc = air_acl_delAction(unit, acl_rule_id);
        if (AIR_E_OK != rc)
        {
            VOICE_ERR_PRINT("Delete VOICE_VLAN ACL action entry-id %d failed, rc=%d.", acl_rule_id, rc);
        }
        voice_vlan_acl_id[acl_index] = VOICE_VLAN_INVALID;

        _voice_vlan_rate_limit_del();
        mw_acl_mutex_release();
    }
    return MW_E_OK;
}

static void
_voice_vlan_acl_port_change(
    UI8_T change_type)
{
    /* Switch the port from Unauth to auth member for all acl rules */
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T acl_rule_id;
    AIR_ACL_RULE_T acl_rule;
    MW_PORT_BITMAP_T port_bmp;

    if ((voice_cfg.state == FALSE) || (voice_cfg.vid == 0))
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return;
    }
    VOICE_DBG_PRINT("Try to update port member 0x%08X with type: %d", voice_vlan_member, change_type);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    if (change_type == VOICE_ACL_PORT_SECMODE)
    {
        acl_rule_id = voice_vlan_acl_id[VOICE_VLAN_MAX_ACL_NUM - 1];
        if ((acl_rule_id != VOICE_VLAN_INVALID) && (air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK))
        {
            if (FALSE == acl_rule.rule_en)
            {
                VOICE_DBG_PRINT("The ACL not enabled: %d", acl_rule_id);
                voice_vlan_acl_id[VOICE_VLAN_MAX_ACL_NUM - 1] = VOICE_VLAN_INVALID;
            }
            else
            {
                if(MW_E_OK == mw_acl_mutex_take())
                {
                    /* authorized oui */
                    port_bmp[unit] = (voice_cfg.sec_ports << 1U);
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, port_bmp);
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc != AIR_E_OK)
                    {
                        VOICE_ERR_PRINT("change VOICE_VLAN ACL sec rule entry-id %d port bitmap, rc=%d", acl_rule_id, rc);
                    }
                    mw_acl_mutex_release();
                }
            }
        }
        else
        {
            VOICE_DBG_PRINT("The ACL sec rule %d does not exist", acl_rule_id);
            /* add again */
            _voice_vlan_acl_secmode_add();
        }
    }
    else
    {
#endif
    UI32_T unauth_ports = voice_cfg.auto_ports & (~voice_vlan_member);
    ether_addr_t empty_mac;
    UI8_T acl_index;
    UI8_T j = 0;
    memset(&empty_mac, 0, sizeof(ether_addr_t));
    for(j = 0; j < MAX_OUI_NUM; j++)
    {
        if (0 == memcmp(empty_mac.octet, voice_cfg.ouis[j].octet, ETHER_ADDR_LEN))
        {
            continue;
        }
        acl_index = j * VOICE_VLAN_EACH_OUI_RULES;
        /* joined */
        acl_rule_id = voice_vlan_acl_id[acl_index];
        if ((acl_rule_id != VOICE_VLAN_INVALID) && (air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK))
        {
            if (FALSE == acl_rule.rule_en)
            {
                VOICE_DBG_PRINT("The ACL not enabled: %d", acl_rule_id);
            }
            else
            {
                /* authorized ports */
                if(MW_E_OK == mw_acl_mutex_take())
                {
                    port_bmp[unit] = (voice_vlan_member << 1U);
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, port_bmp);
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc != AIR_E_OK)
                    {
                        VOICE_ERR_PRINT("change VOICE_VLAN ACL rule entry-id %d port bitmap, rc=%d", acl_rule_id, rc);
                    }
                    mw_acl_mutex_release();
                }
            }
        }
        else
        {
            VOICE_DBG_PRINT("The JOINED ACL rule %d does not exist", acl_rule_id);
            /* add again */
            _voice_vlan_acl_oui_add(j, voice_cfg.ouis[j]);
            continue;
        }
        /* Unauth */
        acl_index++;
        memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule_id = voice_vlan_acl_id[acl_index];
        if ((acl_rule_id != VOICE_VLAN_INVALID) && (air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK))
        {
            if (FALSE == acl_rule.rule_en)
            {
                VOICE_DBG_PRINT("The ACL rule %d not enabled", acl_rule_id);
            }
            else
            {
                if(MW_E_OK == mw_acl_mutex_take())
                {
                    /* authorized ports */
                    port_bmp[unit] = (unauth_ports << 1U);
                    AIR_PORT_BITMAP_COPY(acl_rule.portmap, port_bmp);
                    rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                    if (rc != AIR_E_OK)
                    {
                        VOICE_ERR_PRINT("change VOICE_VLAN ACL rule entry-id %d port bitmap, rc=%d", acl_rule_id, rc);
                    }
                    mw_acl_mutex_release();
                }
            }
        }
        else
        {
            VOICE_DBG_PRINT("The UNAUTH ACL rule %d does not exist", acl_rule_id);
            /* should not happen, add again */
            _voice_vlan_acl_oui_add(j, voice_cfg.ouis[j]);
        }
    }
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    }
#endif
}

static void
_voice_vlan_acl_oui_delete(
    UI8_T oui_idx)
{
    MW_ERROR_NO_T ret;
    UI8_T acl_index = VOICE_VLAN_EACH_OUI_RULES * oui_idx;
    ether_addr_t empty_mac;

    memset(&empty_mac, 0, sizeof(ether_addr_t));
    if (0 == memcmp(empty_mac.octet, voice_cfg.ouis[oui_idx].octet, ETHER_ADDR_LEN))
    {
        return;
    }
    VOICE_DBG_PRINT("Try to delete OUI from ACL rules: %02x:%02x:%02x", voice_cfg.ouis[oui_idx].octet[0], voice_cfg.ouis[oui_idx].octet[1], voice_cfg.ouis[oui_idx].octet[2]);
    ret = _voice_vlan_acl_del(acl_index);
    if (ret != MW_E_OK)
    {
        VOICE_ERR_PRINT("Cannot delete OUI index: %d", oui_idx);
    }

    ret = _voice_vlan_acl_del(++acl_index);
    if (ret != MW_E_OK)
    {
        VOICE_ERR_PRINT("Cannot delete OUI index: %d", oui_idx);
    }
}

static void
_voice_vlan_acl_oui_update(
    void)
{
    MW_ERROR_NO_T ret;
    VOICE_ACL_T voice_acl_auth;
    VOICE_ACL_T voice_acl_unauth;
    AIR_ACL_RULE_T acl_rule;
    AIR_ACL_ACTION_T acl_action;
    UI32_T unauth_ports = voice_cfg.auto_ports & (~voice_vlan_member);
    UI8_T j, count, unit = 0;
    UI16_T acl_rule_id = 0;
    ether_addr_t  empty_oui = {.octet = {0}};

    if ((voice_cfg.state == FALSE) || (voice_cfg.vid == 0))
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return;
    }
    /* joined */
    voice_acl_auth.port_bmp[0] = (voice_vlan_member << 1U);
    voice_acl_auth.type = VOICE_ACL_PORT_JOINED;
    voice_acl_auth.vid = voice_cfg.vid;
    voice_acl_auth.pri = voice_cfg.pri;
    /* unauthorized */
    voice_acl_unauth.port_bmp[0] = (unauth_ports << 1U);
    voice_acl_unauth.type = VOICE_ACL_PORT_UNAUTH;
    voice_acl_unauth.vid = voice_cfg.vid;
    voice_acl_unauth.pri = voice_cfg.pri;

    for(j = 0; j < MAX_OUI_NUM; j++)
    {
        if (0 == memcmp(voice_cfg.ouis[j].octet, empty_oui.octet, ETHER_ADDR_LEN))
        {
            continue;
        }
        memcpy(voice_acl_auth.mac.octet, voice_cfg.ouis[j].octet, ETHER_ADDR_LEN);
        memcpy(voice_acl_unauth.mac.octet, voice_cfg.ouis[j].octet, ETHER_ADDR_LEN);

        for (count=0; count < VOICE_VLAN_EACH_OUI_RULES; count++)
        {
            UI8_T acl_idx = j * VOICE_VLAN_EACH_OUI_RULES + count;
            acl_rule_id = voice_vlan_acl_id[acl_idx];
            if (acl_rule_id == VOICE_VLAN_INVALID)
            {
                VOICE_DBG_PRINT("Try to add VOICE_VLAN ACL entry back");
                _voice_vlan_acl_oui_add(j, voice_cfg.ouis[j]);
                continue;
            }

            if(MW_E_OK == mw_acl_mutex_take())
            {
                VOICE_DBG_PRINT("Try to update VOICE_VLAN ACL entry-id %d", acl_rule_id);
                if (air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK)
                {
                    AIR_ERROR_NO_T rc;
                    if (TRUE == acl_rule.rule_en)
                    {
                        memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
                        if (count == 0)
                        {
                            ret = _voice_acl_rule_form(voice_acl_auth, &acl_rule);
                            ret |= _voice_acl_action_form(voice_acl_auth, &acl_action);
                        }
                        else
                        {
                            ret = _voice_acl_rule_form(voice_acl_unauth, &acl_rule);
                            ret |= _voice_acl_action_form(voice_acl_unauth, &acl_action);
                        }
                        if (ret != MW_E_OK)
                        {
                            VOICE_ERR_PRINT("Internal error, result=%d", ret);
                            mw_acl_mutex_release();
                            break;
                        }
                        rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
                        if (rc != AIR_E_OK)
                        {
                            VOICE_ERR_PRINT("Update VOICE_VLAN ACL rule entry-id %d failed, rc=%d", acl_rule_id, rc);
                            mw_acl_mutex_release();
                            break;
                        }
                        rc = air_acl_setAction(unit, acl_rule_id, &acl_action);
                        if (AIR_E_OK != rc)
                        {
                            VOICE_ERR_PRINT("Update VOICE_VLAN ACL action entry-id %d failed, rc=%d.", acl_rule_id, rc);
                        }
                    }
                }
                else
                {
                    VOICE_DBG_PRINT("Cannot find the ACL rule entry-id %d.", acl_rule_id);
                    voice_vlan_acl_id[acl_idx] = VOICE_VLAN_INVALID;
                }
                mw_acl_mutex_release();
            }
        }
    }
}

static void
_voice_vlan_acl_oui_add(
    UI8_T oui_idx,
    ether_addr_t oui)
{
    MW_ERROR_NO_T ret;
    VOICE_ACL_T voice_acl_auth;
    VOICE_ACL_T voice_acl_unauth;
    UI32_T unauth_ports = voice_cfg.auto_ports & (~voice_vlan_member);

    if ((voice_cfg.state == FALSE) || (voice_cfg.vid == 0))
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return;
    }
    VOICE_DBG_PRINT("Try to add OUI to JOINED ACL rule: %02x:%02x:%02x", oui.octet[0], oui.octet[1], oui.octet[2]);

    if (oui_idx == MAX_OUI_NUM)
    {
        VOICE_DBG_PRINT("%s", "Cannot add OUI to ACL rule, oui list is full");
        return;
    }

    /* joined */
    voice_acl_auth.port_bmp[0] = (voice_vlan_member << 1U);
    VOICE_DBG_PRINT("set auth ports= 0x%X\n", voice_vlan_member);
    voice_acl_auth.index = oui_idx * VOICE_VLAN_EACH_OUI_RULES;
    voice_acl_auth.type = VOICE_ACL_PORT_JOINED;
    voice_acl_auth.vid = voice_cfg.vid;
    voice_acl_auth.pri = voice_cfg.pri;
    memcpy(voice_acl_auth.mac.octet, oui.octet, ETHER_ADDR_LEN);
    /* unauthorized */
    voice_acl_unauth.port_bmp[0] = (unauth_ports << 1U);
    VOICE_DBG_PRINT("set unauth ports= 0x%X\n", unauth_ports);
    voice_acl_unauth.index = voice_acl_auth.index + 1;
    voice_acl_unauth.type = VOICE_ACL_PORT_UNAUTH;
    voice_acl_unauth.vid = voice_cfg.vid;
    voice_acl_unauth.pri = voice_cfg.pri;
    memcpy(voice_acl_unauth.mac.octet, oui.octet, ETHER_ADDR_LEN);

    ret = _voice_vlan_acl_add(FALSE, voice_acl_auth);
    if (ret != MW_E_OK)
    {
        VOICE_ERR_PRINT("Cannot add OUI to JOINED ACL rule: %02x:%02x:%02x", oui.octet[0], oui.octet[1], oui.octet[2]);
        return;
    }

    ret = _voice_vlan_acl_add(FALSE, voice_acl_unauth);
    if (ret != MW_E_OK)
    {
        VOICE_ERR_PRINT("Cannot add OUI to UNAUTH ACL rule: %02x:%02x:%02x", oui.octet[0], oui.octet[1], oui.octet[2]);
        _voice_vlan_acl_oui_delete(oui_idx);
        return;
    }
}

#ifdef AIR_SUPPORT_VOVLAN_SECURITY
static void
_voice_vlan_acl_secmode_add(
    void)
{
    VOICE_ACL_T voice_acl = {0};

    if ((voice_cfg.state == FALSE) || (voice_cfg.vid == 0))
    {
        /* Ignore the voice vlan members if the vid is not configured */
        return MW_E_OK;
    }
    VOICE_DBG_PRINT("%s","Try to add VOICE_VLAN Security ACL rule");
    /* authorized oui */
    voice_acl.port_bmp[0] = (voice_cfg.sec_ports << 1U);
    voice_acl.index = VOICE_VLAN_MAX_ACL_NUM - 1;
    voice_acl.type = VOICE_ACL_PORT_SECMODE;
    voice_acl.vid = voice_cfg.vid;
    if (_voice_vlan_acl_add(TRUE, voice_acl) != MW_E_OK)
    {
        VOICE_ERR_PRINT("%s", "Cannot add SECURITY ACL rule");
        return;
    }
}
#endif

/* Task */
static void
_voice_vlan_free_resource(
    void)
{
    if (NET_FILTER_REGISTER == voice_ctx.voice_pkt_netf.state)
    {
        voice_ctx.voice_pkt_netf.state = NET_FILTER_DEREGISTER;
        if (MW_E_OK != osapi_netRegister(&(voice_ctx.voice_pkt_netf)))
        {
            VOICE_ERR_PRINT("%s", "Error! Failed to deregister voice netif");
        }
    }

    if (MW_E_OK != osapi_msgDelete(VOICE_VLAN_QUEUE_NAME))
    {
        VOICE_ERR_PRINT("Error! osapi_msgDelete for %s failed", VOICE_VLAN_QUEUE_NAME);
    }

    if (voice_ctx.ptr_voice_vland)
    {
        osapi_processDelete(voice_ctx.ptr_voice_vland);
        voice_ctx.ptr_voice_vland = NULL;
    }
}

static MW_ERROR_NO_T
_voice_vlan_data_init(
    void)
{
    MW_ERROR_NO_T ret;
    UI8_T port = 0;

    memset(voice_vlan_acl_id, 0xff, sizeof(UI16_T) * VOICE_VLAN_MAX_ACL_NUM);
    voice_vlan_member = 0;
    memset(&voice_cfg, 0, sizeof(VOICE_VLAN_T));
    memset(trunk_members, 0, sizeof(UI32_T) * MAX_TRUNK_NUM);
    voice_cfg.state = FALSE;
    voice_cfg.vlan_eidx = VOICE_VLAN_INVALID;
    _voice_vlan_ratelimit_id = VOICE_VLAN_INVALID;

    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        voice_cfg.default_auto_ports |= (DEFAULT_VOICE_PORT_MODE << port);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        voice_cfg.default_sec_ports |= (DEFAULT_VOICE_SEC_MODE << port);
#endif
    }

    ret = _voice_vlan_db_send(M_GETFLASH, VOICE_VLAN_INFO, VOICE_PORT_MODE, 1,
            sizeof(UI32_T), NULL, NULL);
    if (MW_E_OK != ret)
    {
        return ret;
    }
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    ret = _voice_vlan_db_send(M_GETFLASH, VOICE_VLAN_INFO, VOICE_SECURITY_MODE, 1,
            sizeof(UI32_T), NULL, NULL);
    if (MW_E_OK != ret)
    {
        return ret;
    }
#endif

    ret = _voice_vlan_db_subscribe();

    return ret;
}

static void
_voice_vlan_process(
    void *ptr_pvParameters)
{
    MW_ERROR_NO_T ret;
    NET_MSG_T     *ptr_data = NULL;
    struct pbuf   *ptr_pbuf = NULL;
    DB_MSG_T      *ptr_msg = NULL;
    int           rc = 0;

    /* Just to kill the compiler warning. */
    (void)ptr_pvParameters;

    /* Check DB is ready or not */
    do{
        ret = dbapi_dbisReady();
    }while(MW_E_OK != ret);

    /* Check mac_rcv is ready or not */
    do{
        rc = mac_rcv_readyGet();
    }while(0 != rc);

    /* Create socket to mac_rcv */
    ret = _voice_vlan_socket_create();
    if (MW_E_OK != ret)
    {
        _voice_vlan_free_resource();
        return;
    }

    /* DB subscriptions */
    ret = _voice_vlan_data_init();
    if (MW_E_OK != ret)
    {
        _voice_vlan_free_resource();
        return;
    }

    while(1)
    {
        ret = osapi_msgRecv(VOICE_VLAN_QUEUE_NAME, (UI8_T **)&ptr_data, 0, 500);
        if (MW_E_OK != ret)
        {
            continue;
        }
        VOICE_DBG_PRINT("Received message handle with id %u", ptr_data->msg_id);
        if (ptr_data->msg_id == MW_MSG_ID_ETHERNET_PBUF)
        {
            /* Recv: mac_rcv */
            ptr_pbuf = (struct pbuf *)(ptr_data->ptr_pbuf);
            MW_FREE(ptr_data);
            ret = MW_E_OK;
            VOICE_DBG_PRINT("received packet handle: %p, vpm: %d", ptr_pbuf, ptr_pbuf->stag_hdr.rx_hdr.vpm);
            if(0 == ptr_pbuf->stag_hdr.rx_hdr.vpm && ptr_pbuf->ether_hdr.type == ETHTYPE_ARP)
            {
                VOICE_DBG_PRINT("Bypass untag ARP packet.");
            }
            else
            {
                ret = _voice_vlan_rx(ptr_pbuf);
            }
            VOICE_DBG_PRINT("Free pbuf handle");
            pbuf_free(ptr_pbuf);
            if (MW_E_OK != ret)
            {
                /* Unexpected error happened */
                _voice_vlan_free_resource();
                break;
            }
        }
        else if (ptr_data->msg_id == MW_MSG_ID_DB)
        {
            /* Recv db notification */
            ptr_msg = (DB_MSG_T *)ptr_data;
            VOICE_DBG_PRINT("Received db msg method %02X count %d", ptr_msg->method, ptr_msg->type.count);
            if (ptr_msg->method == M_ACK)
            {
                /* The response should be ok. */
                VOICE_DBG_PRINT("Free the subscribe response message %p", ptr_msg);
                MW_FREE(ptr_msg);
            }
            else
            {
                (void)_voice_vlan_db_notify(ptr_msg);
            }
        }
        else
        {
            /* unknown notification */
            MW_FREE(ptr_data);
        }
    }

    return;
}

/* FUNCTION NAME: voice_vlan_init
 * PURPOSE:
 *      Start voice vlan
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
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
voice_vlan_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if(NULL != voice_ctx.ptr_voice_vland)
    {
        MW_LOG_PRINTF("Voice VLAN task is running.\n");
        return MW_E_ALREADY_INITED;
    }
    MW_LOG_INIT_PRINTF("Initializing Voice VLAN task...\n");

    /* Create message queue to receive both DB and Socket message */
    ret = osapi_msgCreate(VOICE_VLAN_QUEUE_NAME, VOICE_VLAN_QUEUE_LENGTH, VOICE_VLAN_MSG_SIZE);
    if (ret != MW_E_OK)
    {
        VOICE_DBG_PRINT("%s", "Error! Failed to create Voice VLAN Queue");
        _voice_vlan_free_resource();
        return MW_E_NO_MEMORY;
    }
    /* Get queue handle */
    voice_ctx.voice_q_handle = osapi_msgFindHandle(VOICE_VLAN_QUEUE_NAME);

    /* Create task for voice_vlan */
    ret = osapi_threadCreateStatic(VOICE_VLAN_TASK_NAME, VOICE_VLAN_STACK_SIZE, MW_TASK_PRIORITY_VOICE_VLAN,
            _voice_vlan_process, NULL, voice_ctx.voice_task_stack, &(voice_ctx.voice_stack),
            &(voice_ctx.ptr_voice_vland));
    if(MW_E_OK != ret)
    {
        /* Error create task */
        VOICE_DBG_PRINT("%s", "Error! Failed to create Voice VLAN main task");
        _voice_vlan_free_resource();
        return ret;
    }

    return ret;
}

/* FUNCTION NAME: voice_vlan_get_status
 * PURPOSE:
 *      Show the current voice vlan state
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
voice_vlan_get_status(
    void)
{
    UI16_T acl_rule_id = 0;
    MW_CMD_OUTPUT("Voice_Vlan State: %s\n", (voice_cfg.state == TRUE)?"ENABLED":"DISABLED");
    if (FALSE == voice_cfg.state)
    {
        return;
    }
    MW_CMD_OUTPUT("Vlan ID: %d\n", voice_cfg.vid);
    MW_CMD_OUTPUT("Priority: %d\n", voice_cfg.pri);
    MW_CMD_OUTPUT("Portmode: 0x%08X\n", voice_cfg.auto_ports);
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    MW_CMD_OUTPUT("Secmode: 0x%08X\n", voice_cfg.sec_ports);
#endif
    MW_CMD_OUTPUT("Voice_Vlan Member: 0x%08X\n", voice_vlan_member);

    while (acl_rule_id < VOICE_VLAN_MAX_ACL_NUM)
    {
        if (VOICE_VLAN_INVALID != voice_vlan_acl_id[acl_rule_id])
        {
            MW_CMD_OUTPUT("Voice_Vlan ACL rule %-2d: %0d\n", acl_rule_id, voice_vlan_acl_id[acl_rule_id]);
        }
        acl_rule_id++;
    }
}
