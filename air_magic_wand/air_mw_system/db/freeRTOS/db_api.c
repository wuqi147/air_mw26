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

/* FILE NAME:   db_api.c
 * PURPOSE:
 *      Wrapper APIs for database task.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "db_api.h"
#include "db_main.h"
#include "db_data.h"
#include "db_cfgfile.h"
#include "osapi_message.h"
#include "osapi_memory.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static const UI8_T _dbfid_size[TABLES_LAST] = {
    SYS_INFO_LAST,
    ACC_INFO_LAST,
    PORT_CFG_INFO_LAST,
    PORT_QOS_LAST,
    TRUNK_PORT_LAST,
    TRUNK_ALGORITHM_LAST,
#ifdef AIR_SUPPORT_LP
    LOOP_PREVEN_INFO_LAST,
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_IGMP_SNP
    IGMP_SNP_INFO_LAST,
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    IGMP_SNP_QUERIER_INFO_LAST,
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */
    PORT_MIRROR_INFO_LAST,
    JUMBO_FRAME_INFO_LAST,
    VLAN_CFG_INFO_LAST,
    VLAN_ENTRY_LAST,
#ifdef AIR_SUPPORT_VOICE_VLAN
    VOICE_VLAN_INFO_LAST,
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    SURVEI_VLAN_INFO_LAST,
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    OUI_ENTRY_LAST,
#endif
    QOS_INFO_LAST,
    STATIC_MAC_ENTRY_LAST,
#ifdef AIR_SUPPORT_DHCP_SNOOP
    DHCP_SNP_INFO_LAST,
    DHCP_SNP_PORT_INFO_LAST,
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    SNMP_LAST,
#endif /* AIR_SUPPORT_SNMP */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    STP_INFO_LAST,
    STP_PORT_INFO_LAST,
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
    MQTTD_CFG_INFO_LAST,
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_LLDPD
    LLDP_INFO_LAST,
    LLDP_PORT_INFO_LAST,
#endif
#ifdef AIR_SUPPORT_POE
    POE_CFG_LAST,
    POE_PORT_CFG_LAST,
#endif
#ifdef AIR_SUPPORT_SNTP
    SNTP_CFG_LAST,
#endif
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    RSTP_PORT_SEC_LAST,
#endif
#ifdef AIR_SUPPORT_ERPS
    ERPS_INFO_LAST,
#endif
#ifdef AIR_SUPPORT_MSTP
    MSTP_REGION_LAST,
    MSTP_INSTANCE_LAST,
    MSTP_INSTANCE_PORT_LAST,
#endif
    SYS_OPER_INFO_LAST,
    PORT_OPER_INFO_LAST,
    LAG_OPER_INFO_LAST,
    LAG_MEMBER_0_LAST,
#ifdef AIR_SUPPORT_CABLE_DIAG
    PORT_CABLE_DIAG_LAST,
#endif
    LOGON_INFO_LAST,
    L2_MC_ENTRY_LAST,
    SYSTEM_LAST,
#ifdef AIR_SUPPORT_ICMP_CLIENT
    ICMP_CLIENT_INFO_LAST,
#endif
#ifdef AIR_SUPPORT_SFP
    SFP_MODULE_INFO_LAST,
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
    VOICE_OPER_INFO_LAST,
#endif
    MIB_CNT_LAST,
    DYNAMIC_MAC_ADDRESS_ENTRY_CFG_LAST,
    DYNAMIC_MAC_ADDRESS_ENTRY_LAST,
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    STP_PORT_OPER_INFO_LAST,
#ifdef AIR_SUPPORT_MSTP
    MSTP_INSTANCE_PORT_OPER_INFO_LAST,
#endif
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_LLDPD
    LLDP_CLIENT_INFO_LAST,
#endif
#ifdef AIR_SUPPORT_POE
    POE_STATUS_LAST,
    POE_PORT_STATUS_LAST,
#endif

};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: dbapi_dbisReady
 * PURPOSE:
 *      Get the state of DB task
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      The return value will be MW_E_OK if the database is initialized,
 *      otherwise return MW_E_NOT_INITED.
 */
MW_ERROR_NO_T
dbapi_dbisReady(
    void)
{
    return db_readyFlagGet();
}

/* FUNCTION NAME: dbapi_getDataSize
 * PURPOSE:
 *      Get the buffer size of the request type
 *
 * INPUT:
 *      req          -- A request type data
 *
 * OUTPUT:
 *      total_size   -- The required data size of
 *                      the request type
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getDataSize(
    DB_REQUEST_TYPE_T req,
    UI16_T *total_size)
{
    return db_getDataSize(req, total_size);
}

/* FUNCTION NAME: dbapi_setMsgHeader
 * PURPOSE:
 *      Fill in the message header
 *
 * INPUT:
 *      client_qname     -- The name of the queue
 *      method           -- The method of this request message
 *      pcount           -- The requests number of this message
 *
 * OUTPUT:
 *      ptr_input        -- The pointer points to the message head
 *
 * RETURN:
 *      The header size
 *
 * NOTES:
 *
 */
UI16_T
dbapi_setMsgHeader(
    void *ptr_input,
    const C8_T *client_qname,
    const UI8_T method,
    const UI8_T pcount)
{
    DB_MSG_T *ptr_msg = NULL;
    if (ptr_input == NULL)
    {
        return 0;
    }

    ptr_msg = (DB_MSG_T *)ptr_input;
    if ((client_qname != NULL) && (*client_qname != 0))
    {
        memcpy((void *)(ptr_msg->cq_name), (const void *)client_qname, DB_Q_NAME_SIZE);
    }
    ptr_msg->method = method;
    ptr_msg->type.count= pcount;
    return DB_MSG_HEADER_SIZE;
}

/* FUNCTION NAME: dbapi_setMsgPayload
 * PURPOSE:
 *      Fill in the data payload
 *
 * INPUT:
 *      method           -- The method of this request message
 *      in_t_idx         -- The enum of the table
 *      in_f_idx         -- The enum of the field
 *      in_e_idx         -- The entry index in the table
 *      ptr_raw_data     -- The pointer of raw data
 *
 * OUTPUT:
 *      ptr_input        -- The pointer points to the request head of the payload
 *
 * RETURN:
 *      The request total size
 *
 * NOTES:
 *
 */
UI16_T
dbapi_setMsgPayload(
    UI8_T method,
    UI8_T in_t_idx,
    UI8_T in_f_idx,
    UI16_T in_e_idx,
    void *ptr_raw_data,
    void *ptr_input)
{
    UI16_T data_size = 0;
    UI8_T *ptr_pload = NULL;
    DB_REQUEST_TYPE_T request =
    {
        .t_idx = in_t_idx,
        .f_idx = in_f_idx,
        .e_idx = in_e_idx
    };

    if (ptr_input == NULL)
    {
        return 0;
    }

    (void)dbapi_getDataSize(request, &data_size);
    /* copy the data to payload buffer */
    ptr_pload = (UI8_T *)ptr_input;
    memcpy((void *)ptr_pload, (const void *)&request, sizeof(DB_REQUEST_TYPE_T));
    ptr_pload += sizeof(DB_REQUEST_TYPE_T);

    if ((0 != (method & M_B_SUBSCRIBED)) ||
#ifdef DB_SUPPORT_RESTORE_METHOD
        (M_RESTORE == method) ||
#endif
        (M_DELETE == method))
    {
        data_size = 0;
    }
    memcpy((void *)ptr_pload, (const void *)&data_size, sizeof(data_size));
    ptr_pload += sizeof(data_size);
    if ((data_size != 0 ) && (NULL != ptr_raw_data))
    {
        memcpy((void *)ptr_pload, (const void *)ptr_raw_data, data_size);
        ptr_pload += data_size;
    }
    data_size += DB_MSG_PAYLOAD_SIZE;

    return data_size;
}

/* FUNCTION NAME: dbapi_sendRequesttoDb
 * PURPOSE:
 *      Send a request to DB request queue
 *
 * INPUT:
 *      size            -- Size of the data in message buffer
 *      ptr_msg         -- A pointer to the item to be tranmitted
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_sendRequesttoDb(
    UI32_T size,
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_msg);
    MW_PARAM_CHK((0 == size), MW_E_BAD_PARAMETER);

    ret = osapi_msgSend(
            DB_REQUEST_Q_NAME,
            (UI8_T *)ptr_msg,
            size,
            DB_Q_WAITTIME);

    return ret;
}

/* FUNCTION NAME: dbapi_sendMsg
 * PURPOSE:
 *      Send a request to DB request queue
 *
 * INPUT:
 *      ptr_msg         -- A pointer to the item to be tranmitted
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      This API will free the message if failed to send.
 *
 */
MW_ERROR_NO_T
dbapi_sendMsg(
    DB_MSG_T *ptr_msg,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    MW_CHECK_PTR(ptr_msg);

    ret = dbapi_dbisReady();
    if (MW_E_OK != ret)
    {
        /* This message could not be send, drop it */
        MW_FREE(ptr_msg);
        DB_LOG_ERROR("DB is not yet ready");
        return ret;
    }

    ret = osapi_msgSend(
            DB_REQUEST_Q_NAME,
            (UI8_T *)ptr_msg,
            0,
            timeout);

    if (MW_E_OK != ret)
    {
        DB_LOG_WARN("Failed to send message to DB queue");
        MW_FREE(ptr_msg);
    }

    return ret;
}

/* FUNCTION NAME: dbapi_recvMsg
 * PURPOSE:
 *      Receive DB message from client's queue.
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      pptr_out_msg    -- The pointer of pointer points to the received message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_recvMsg(
    const C8_T *client_qname,
    DB_MSG_T **pptr_out_msg,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T *ptr_msg = NULL;

    MW_CHECK_PTR(pptr_out_msg);

    ret = dbapi_dbisReady();
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("DB is not yet ready");
        return ret;
    }

    ret = osapi_msgRecv(
            client_qname,
            &ptr_msg,
            0,
            timeout);
    if (MW_E_OK != ret)
    {
        DB_LOG_WARN("%s failed to receive message from DB (ret=%d)",
                client_qname, ret);
        return ret;
    }
    (*pptr_out_msg) = (DB_MSG_T *)ptr_msg;

    return ret;
}

/* FUNCTION NAME: dbapi_getData
 * PURPOSE:
 *      Get data from DB
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      ptr_msg         -- The pointer points to the request
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      DB is a single thread and uses a FIFO queue to manupliate internal data.
 *      The client queue may get notifications from another update if the caller
 *      has any subscriptions from DB. Which means this API is only suitable for
 *      the caller that HAS NO SUBSCRIPTIONS.
 *      This API will not free the requested message. User should free it in the task.
 *
 */
MW_ERROR_NO_T
dbapi_getData(
    const C8_T *client_qname,
    DB_MSG_T *ptr_msg,
    const UI32_T timeout)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_output = NULL;

    MW_CHECK_PTR(ptr_msg);

    rc = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != rc)
    {
        DB_LOG_ERROR("[%s] Failed to send message to DB", client_qname);
        return rc;
    }

    /* wait for respond messgae and free ptr_msg */
    rc = dbapi_recvMsg(client_qname, &ptr_output, timeout);
    if (MW_E_OK == rc)
    {
        if (ptr_output == ptr_msg)
        {
            rc = ptr_msg->type.result;
        }
        else
        {
            DB_LOG_WARN("%s received unknown response from DB(%p).",
                    client_qname, ptr_output);
            osapi_free(ptr_output);
            rc = MW_E_OP_INCOMPLETE;
        }
    }
    return rc;
}

/* FUNCTION NAME: dbapi_setData
 * PURPOSE:
 *      Update data to DB
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      ptr_msg         -- The pointer points to the request
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      DB is a single thread and uses a FIFO queue to manupliate internal data.
 *      The client queue may get notifications from another update if the caller
 *      has any subscriptions from DB. Which means this API is only suitable for
 *      the caller that HAS NO ANY SUBSCRIPTIONS.
 *      This API will free the message automatically. The client queue may get
 *      notifications from another updates. This API will free it directly.
 *
 */
MW_ERROR_NO_T
dbapi_setData(
    const C8_T *client_qname,
    DB_MSG_T *ptr_msg,
    const UI32_T timeout)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_output = NULL;

    MW_CHECK_PTR(ptr_msg);

    rc = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    /* wait for respond messgae and free ptr_msg */
    rc = dbapi_recvMsg(client_qname, &ptr_output, timeout);
    if (MW_E_OK == rc)
    {
        if (ptr_output == ptr_msg)
        {
            rc = ptr_msg->type.result;
        }
        else
        {
            DB_LOG_WARN("%s received unknown response from DB(%p)", client_qname, ptr_output);
            rc = MW_E_OP_INCOMPLETE;
        }
        osapi_free(ptr_output);
    }
    else
    {
        DB_LOG_WARN("%s failed to receive response from DB(%d)", client_qname, rc);
    }

    return rc;
}

/* FUNCTION NAME: dbapi_showCfgFile
 * PURPOSE:
 *      Show the config file in Flash
 *
 * INPUT:
 *      showstartup           -- Show the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Show the config file in console
 *
 */
MW_ERROR_NO_T
dbapi_showCfgFile(
    BOOL_T showstartup)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);

    return db_showFlashCfgData(showstartup);
}

/* FUNCTION NAME: dbapi_saveCfgFile
 * PURPOSE:
 *      Save the running DB to config file in Flash
 *
 * INPUT:
 *      savestartup           -- Save the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Save the config
 *
 */
MW_ERROR_NO_T
dbapi_saveCfgFile(
    BOOL_T savestartup)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);

    return db_saveFlashCfgData(savestartup);
}

/* FUNCTION NAME: dbapi_getCfgFile
 * PURPOSE:
 *      Check the startup config file exist and return the size
 *
 * INPUT:
 *      ptr_file               -- The pointer of the config file
 *
 * OUTPUT:
 *      ptr_ file_size         -- The size of the config file
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Only allow to access the startup config
 *
 */
MW_ERROR_NO_T
dbapi_getCfgFile(
    UI8_T *ptr_file,
    UI32_T *ptr_file_size)
{
    MW_CHECK_PTR(ptr_file);
    MW_CHECK_PTR(ptr_file_size);
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);

    return db_getFlashCfg(ptr_file, ptr_file_size);
}

/* FUNCTION NAME: dbapi_dumpRaw
 * PURPOSE:
 *      Dump the DB Raw data for sepcific table and field
 *
 * INPUT:
 *      cfg_type        -- The config type
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_dumpRaw(
    UI8_T cfg_type,
    UI8_T t_idx,
    UI8_T f_idx)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((cfg_type >= DB_CFG_TYPE_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((f_idx >= _dbfid_size[t_idx]), MW_E_BAD_PARAMETER);
    return db_dumpRaw(cfg_type, t_idx, f_idx);
}

#ifdef DB_SUPPORT_RESTORE_METHOD
/* FUNCTION NAME: dbapi_restoreTable
 * PURPOSE:
 *      Restore the specific table from the database
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      e_idx           -- The entry index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_restoreTable(
    const UI8_T   t_idx,
    const UI8_T   f_idx,
    const UI16_T  e_idx)
{
    UI16_T              msg_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    DB_PAYLOAD_T        *ptr_payload = NULL;
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx };

    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= DB_FIRST_OPER_TABLE), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((f_idx >= _dbfid_size[t_idx]), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((e_idx > db_getTableEntriesNum(t_idx)), MW_E_BAD_PARAMETER);

    ptr_msg = dbapi_createMsgByReq(NULL, M_RESTORE, 1, &request, &msg_size, (UI8_T **) &ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }

    rc = dbapi_appendMsgPayload(&request, NULL, &ptr_msg, &msg_size, (UI8_T **) &ptr_payload);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
        return rc;
    }

    /* Send the message to DB */
    rc = dbapi_sendMsg(ptr_msg, 0);
    if (MW_E_OK != rc)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        DB_LOG_ERROR("Failed to send message to DB, rc = %d", rc);
    }
    return rc;
}
#endif /* DB_SUPPORT_RESTORE_METHOD */

/* FUNCTION NAME: dbapi_clrCfgFile
 * PURPOSE:
 *      Erase the DB config file
 *
 * INPUT:
 *      clrstartup       -- Clear the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_clrCfgFile(
    BOOL_T clrstartup)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);

    return db_clrFlashCfgData(clrstartup);
}

/* FUNCTION NAME: dbapi_getTableName
 * PURPOSE:
 *      Return the table name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_tbltext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getTableName(
    UI8_T t_idx,
    UI8_T data_size,
    C8_T *ptr_tbltext)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((data_size == 0), MW_E_BAD_PARAMETER);

    return db_cfgfile_getTableName(t_idx, data_size, ptr_tbltext);
}

/* FUNCTION NAME: dbapi_getFieldName
 * PURPOSE:
 *      Return the Field name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_fldtext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldName(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T data_size,
    C8_T *ptr_fldtext)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((f_idx >= _dbfid_size[t_idx]), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((data_size == 0), MW_E_BAD_PARAMETER);

    return db_cfgfile_getFieldName(t_idx, f_idx, data_size, ptr_fldtext);
}

/* FUNCTION NAME: dbapi_getFieldsNum
 * PURPOSE:
 *      Return the Fields number of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      ptr_fldnum      -- The output fields number
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldsNum(
    UI8_T t_idx,
    UI8_T *ptr_fldnum)
{
    MW_CHECK_PTR(ptr_fldnum);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    *ptr_fldnum = (UI8_T)_dbfid_size[t_idx];

    return MW_E_OK;
}

/* FUNCTION NAME: dbapi_getFieldSize
 * PURPOSE:
 *      Return the specific Field size of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *
 * OUTPUT:
 *      ptr_fldsize     -- The output field size
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldSize(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T *ptr_fldsize)
{
    MW_CHECK_PTR(ptr_fldsize);
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((f_idx >= _dbfid_size[t_idx]), MW_E_BAD_PARAMETER);

    *ptr_fldsize = db_getTableFieldSize(t_idx, f_idx);

    return MW_E_OK;
}

/* FUNCTION NAME: dbapi_getEntriesNum
 * PURPOSE:
 *      Return the entries number of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      ptr_entriesnum  -- The entreies number
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getEntriesNum(
    UI8_T t_idx,
    UI16_T *ptr_entriesnum)
{
    MW_CHECK_PTR(ptr_entriesnum);
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    *ptr_entriesnum = db_getTableEntriesNum(t_idx);

    return MW_E_OK;
}


/* FUNCTION NAME: dbapi_getTableIdx
 * PURPOSE:
 *      Return the table index of the specific table name
 *
 * INPUT:
 *      table_name      -- The table name string
 *
 * OUTPUT:
 *      ptr_tblidx      -- The table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getTableIdx(
    C8_T *table_name,
    UI8_T *ptr_tblidx)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_CHECK_PTR(table_name);
    MW_CHECK_PTR(ptr_tblidx);

    return db_cfgfile_getTableIdx(table_name, ptr_tblidx);
}

/* FUNCTION NAME: dbapi_getFieldIdx
 * PURPOSE:
 *      Return the Field index of the specific field name
 *
 * INPUT:
 *      t_idx           -- The table index
 *      field_name      -- The field name
 *
 * OUTPUT:
 *      ptr_fldidx      -- The field index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldIdx(
    UI8_T t_idx,
    C8_T *field_name,
    UI8_T *ptr_fldidx)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_CHECK_PTR(field_name);
    MW_CHECK_PTR(ptr_fldidx);

    return db_cfgfile_getFieldIdx(t_idx, field_name, ptr_fldidx);
}

/* FUNCTION NAME: dbapi_dumpSubTree
 * PURPOSE:
 *      Dump the DB Current Subscriptions
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
 *
 */
void
dbapi_dumpSubTree(
    void)
{
    if (MW_E_OK != dbapi_dbisReady())
    {
        MW_CMD_OUTPUT("[%s] DB is not yet ready\n", __func__);
    }

    db_dumpSubTree();
}

/* FUNCTION NAME: dbapi_dumpTableCtrl
 * PURPOSE:
 *      Dump the DB Current control structure information
 *
 * INPUT:
 *      detail          -- Detail level of the dump
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_dumpTableCtrl(
    UI8_T   detail)
{
    if (MW_E_OK != dbapi_dbisReady())
    {
        MW_CMD_OUTPUT("[%s] DB is not yet ready\n", __func__);
    }

    return db_dumpTableCtrl(detail);
}

/* FUNCTION NAME: dbapi_createMsg
 * PURPOSE:
 *      Create a message for DB and set its header. Its payload size is indicated
 *      by payload_size inputted directly.
 *
 * INPUT:
 *      ptr_queue_name  --  A pointer to the queue name
 *      method          --  The method of the message
 *      request_count   --  The total number of requests which is intended be
 *                          filled in the message payload
 *      payload_size    --  The total size of the message payload
 *
 * OUTPUT:
 *      ptr_msg_size    --  A pointer returns the total size of the message
 *      pptr_payload    --  A double pointer returns the start address of the
 *                          message payload
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If request_count is not clear when calling the function, a non-zero value
 *      can be filled in temporarily and then use dbapi_appendMsgPayload()
 *      to set the message payload. dbapi_appendMsgPayload() will update
 *      the count member of the header based on how many requests have been set
 *      into the message payload.
 */
DB_MSG_T *
dbapi_createMsg(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T request_count,
    const UI16_T payload_size,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_payload)
{
    DB_MSG_T *ptr_msg = NULL;
    UI16_T offset = 0, msg_size = 0;

    if ((0 == payload_size) || (NULL == ptr_msg_size) || (NULL == pptr_payload))
    {
        return NULL;
    }

    (*pptr_payload) = NULL;
    (*ptr_msg_size) = 0;
    msg_size = DB_MSG_HEADER_SIZE + payload_size;
    /* Allocate a memory buffer for the client queue using */
    if (ptr_queue_name == NULL)
    {
        /* The message will be free by DB task itself */
        osapi_calloc(msg_size, DB_REQUEST_Q_NAME, (void **)&ptr_msg);
    }
    else
    {
        osapi_calloc(msg_size, ptr_queue_name, (void **)&ptr_msg);
    }
    if (NULL == ptr_msg)
    {
        DB_LOG_ERROR("[%s] Failed to allocate message", ptr_queue_name);
        return NULL;
    }

    offset = dbapi_setMsgHeader(ptr_msg, ptr_queue_name, method, request_count);
    if (0 == offset)
    {
        MW_FREE(ptr_msg);
        return NULL;
    }

    (*pptr_payload) = (UI8_T *)ptr_msg + offset;
    (*ptr_msg_size) = msg_size;
    return ptr_msg;
}

/* FUNCTION NAME: dbapi_createMsgByReq
 * PURPOSE:
 *      Create a message for DB and set its header. Its payload size will be
 *      calculated by the request array inputted.
 *
 * INPUT:
 *      ptr_queue_name  --  A pointer to the queue name
 *      method          --  The method of the message
 *      request_count   --  The total number of requests within the request array
 *      request[]       --  The request array which are supposed to be set in the
 *                          message payload later
 *
 * OUTPUT:
 *      ptr_msg_size    --  A pointer returns the total size of the message
 *      pptr_payload    --  A double pointer returns the start address of the
 *                          message payload
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Please be aware that the requests set after this function returns should
 *      be exactly the same as the requests inputted through the request array.
 */
DB_MSG_T *
dbapi_createMsgByReq(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    UI8_T request_count,
    DB_REQUEST_TYPE_T request[],
    UI16_T *ptr_msg_size,
    UI8_T **pptr_payload)
{
    UI32_T i = 0;
    UI16_T payload_size = 0, size = 0;
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if ((NULL == request) || (NULL == ptr_msg_size) || (NULL == pptr_payload))
    {
        return NULL;
    }

    (*pptr_payload) = NULL;
    (*ptr_msg_size) = 0;
    for (; i < request_count; i++)
    {
        if ((0 != (method & M_B_SUBSCRIBED)) ||
#ifdef DB_SUPPORT_RESTORE_METHOD
            (M_RESTORE != method) ||
#endif
            (M_DELETE == method))
        {
            size = 0;
        }
        else
        {
            ret = dbapi_getDataSize(request[i], &size);
            if (MW_E_OK != ret)
            {
                return NULL;
            }
        }
        payload_size += (size + DB_MSG_PAYLOAD_SIZE);
    }

    return dbapi_createMsg(ptr_queue_name,
                           method,
                           request_count,
                           payload_size,
                           ptr_msg_size,
                           pptr_payload);
}

/* FUNCTION NAME: dbapi_setMsgPayload
 * PURPOSE:
 *      Append a request after the last request of the message payload. If the
 *      message is not large enough, the message will be enlarged by reallocating.
 *
 * INPUT:
 *      ptr_request             -- A pointer to the request to be appended
 *      ptr_data                -- A pointer to the request raw data
 *      pptr_msg                -- A double pointer to the DB message
 *      ptr_msg_size            -- A pointer to the size of the DB message
 *      pptr_shifted_payload    -- A double pointer to the position of the payload
 *                                 to store the current request. If it is the first
 *                                 request, the start address of the ptr_payload
 *                                 member of the DB message should be inputted
 *
 * OUTPUT:
 *      pptr_msg                -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      ptr_msg_size            -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      pptr_shifted_payload    -- A double pointer returns the position of the
 *                                 payload to store the next request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If any error returns, the message should be freed by the caller.
 *      To avoid triggering the reallocation frequently, it is suggested to
 *      allocate large enough buffer for the DB message.
 */
MW_ERROR_NO_T
dbapi_appendMsgPayload(
    DB_REQUEST_TYPE_T *ptr_request,
    UI8_T *ptr_data,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload)
{
    UI16_T offset = 0, data_size = 0, free_size = 0, new_msg_size = 0;
    UI8_T request_count = 0;
    DB_MSG_T *ptr_msg = NULL, *ptr_new_msg = NULL;
    UI8_T *ptr_new_payload = NULL;
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if ((NULL == ptr_request) ||
        (NULL == pptr_msg) || (NULL == (*pptr_msg)) ||
        (NULL == ptr_msg_size) || (0 == (*ptr_msg_size)) ||
        (NULL == pptr_shifted_payload) || (NULL == (*pptr_shifted_payload)) ||
        ((UI8_T *)(*pptr_msg) + DB_MSG_HEADER_SIZE > (*pptr_shifted_payload)) ||
        ((UI8_T *)(*pptr_msg) + (*ptr_msg_size) < (*pptr_shifted_payload)))
    {
        return MW_E_BAD_PARAMETER;
    }

    /* Even if ptr_data is NULL, still reserve buffer for the raw data. */
    if ((0 != ((*pptr_msg)->method & M_B_SUBSCRIBED)) ||
#ifdef DB_SUPPORT_RESTORE_METHOD
        (M_RESTORE == (*pptr_msg)->method) ||
#endif
        (M_DELETE == (*pptr_msg)->method) )
    {
        data_size = 0;
    }
    else
    {
        ret = dbapi_getDataSize((*ptr_request), &data_size);
        if (MW_E_OK != ret)
        {
            return ret;
        }
    }
    data_size += DB_MSG_PAYLOAD_SIZE;

    ptr_msg = *pptr_msg;

    if (((UI8_T *)ptr_msg + DB_MSG_HEADER_SIZE) < (*pptr_shifted_payload))
    {
        /* There is at least one request filled in. */
        request_count = ptr_msg->type.count;
    }

    free_size = (UI8_T *)ptr_msg + (*ptr_msg_size) - (*pptr_shifted_payload);
    if (free_size < data_size)
    {
        /* The message need be enlarged. request_count will be corrected later. */
        ptr_new_msg = dbapi_createMsg(NULL,
                                      ptr_msg->method,
                                      1,
                                      (*ptr_msg_size) + data_size - free_size - DB_MSG_HEADER_SIZE,
                                      &new_msg_size,
                                      &ptr_new_payload);
        if (NULL == ptr_new_msg)
        {
            DB_LOG_ERROR("Fail to enlarge the message");
            return MW_E_NO_MEMORY;
        }

        /* request_count is corrected here. */
        osapi_memcpy(ptr_new_msg, ptr_msg, (*ptr_msg_size) - free_size);
        MW_FREE(ptr_msg);
        ptr_msg = ptr_new_msg;
        *pptr_msg = ptr_new_msg;
        *pptr_shifted_payload = (UI8_T *)ptr_new_msg + (*ptr_msg_size) - free_size;
        *ptr_msg_size = new_msg_size;
    }

    offset = dbapi_setMsgPayload(ptr_msg->method, ptr_request->t_idx, ptr_request->f_idx, ptr_request->e_idx, ptr_data, *pptr_shifted_payload);
    if (0 == offset)
    {
        return MW_E_OP_INCOMPLETE;
    }

    /* Update the count based on the requests that are filled in the payload. */
    ptr_msg->type.count = request_count + 1;
    (*pptr_shifted_payload) += offset;

    return MW_E_OK;
}

/* FUNCTION NAME: dbapi_parseMsg
 * PURPOSE:
 *      Parse a DB_MSG_T notification.
 *
 * INPUT:
 *      ptr_msg           -- A pointer points to a DB_MSG_T message with one or
 *                           more requests.
 *      count             -- The total number of requests within the message
 *      pptr_payload_data -- A double pointer points to the request to be parsed
 *                           if it is the subsequent parsing for ptr_msg. If it is
 *                           the first parsing for ptr_msg, its value is ignored.
 *
 * OUTPUT:
 *      ptr_request       -- A pointer returns the request parsed
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *      pptr_payload_data -- A double pointer returns the next request to be
 *                           parsed if there is any
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. (*pptr_data) points to the data within the buffer pointed by ptr_msg.
 *      2. DB_MSG_T.type.count may be changed when the function returns. As a
 *         result, the message can only be parsed once.
 */
MW_ERROR_NO_T
dbapi_parseMsg(
    DB_MSG_T *ptr_msg,
    UI8_T count,
    DB_REQUEST_TYPE_T *ptr_request,
    UI16_T *ptr_data_size,
    UI8_T **pptr_data,
    UI8_T **pptr_payload_data)
{
    UI8_T *ptr_currReq = NULL;
    UI8_T req_count = 0;

    if ((NULL == ptr_msg) || (NULL == ptr_request) || (NULL == ptr_data_size) ||
        (NULL == pptr_data) || (NULL == pptr_payload_data) ||
        ((0 != (ptr_msg->type.count & DB_COUNT_REUSE_FLAG)) &&
         (NULL == *pptr_payload_data)))
    {
        return MW_E_BAD_PARAMETER;
    }

    osapi_memset(ptr_request, 0, sizeof(DB_REQUEST_TYPE_T));
    (*ptr_data_size) = 0;
    (*pptr_data) = NULL;
    if (0 == (ptr_msg->type.count & DB_COUNT_REUSE_FLAG))
    {
        /* First parsing for ptr_msg. */
        *pptr_payload_data = NULL;
        if ((ptr_msg->method == M_RESPONSE) && (MW_E_OK != ptr_msg->type.result))
        {
            /* Currently ptr_msg->type.result represents the index of the first
             * failed request. The index starts from 1. Do not continue to parse
             * in such case.
             */
            return MW_E_OTHERS;
        }
        ptr_currReq = ((UI8_T *)ptr_msg) + DB_MSG_HEADER_SIZE;
        req_count = count;
    }
    else
    {
        /* Subsequent parsing for ptr_msg. */
        req_count = ptr_msg->type.count & (~DB_COUNT_REUSE_FLAG);
        ptr_currReq = *pptr_payload_data;
        *pptr_payload_data = NULL;
    }

    if (0 == req_count)
    {
        /* All requests have been parsed. */
        return MW_E_OP_INVALID;
    }

    /* Parse the request */
    req_count--;
    osapi_memcpy(ptr_data_size, ((UI8_T *)ptr_currReq) + sizeof(DB_REQUEST_TYPE_T), sizeof(UI16_T));
    /* Validate data_size */
    if ((0 == (ptr_msg->method & M_B_SUBSCRIBED)) &&
        (M_DELETE != ptr_msg->method) &&
#ifdef DB_SUPPORT_RESTORE_METHOD
        (M_RESTORE != ptr_msg->method) &&
#endif
        (0 == *ptr_data_size))
    {
        *ptr_data_size = 0;
        return MW_E_BAD_PARAMETER;
    }

    osapi_memcpy(ptr_request, ptr_currReq, sizeof(DB_REQUEST_TYPE_T));
    if (0 != (*ptr_data_size))
    {
        *pptr_data = ((UI8_T *)ptr_currReq) + DB_MSG_PAYLOAD_SIZE;
    }
    if (0 != req_count)
    {
        *pptr_payload_data = ((UI8_T *)ptr_currReq) + DB_MSG_PAYLOAD_SIZE + *ptr_data_size;
    }

    /* Modify the count value in DB_MSG */
    ptr_msg->type.count = req_count | DB_COUNT_REUSE_FLAG;

    return MW_E_OK;
}

/* FUNCTION NAME: dbapi_getFactoryDefault
 * PURPOSE:
 *      Get the factory default value for specific request.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
dbapi_getFactoryDefault(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    return db_getFactoryDefault(t_idx, f_idx, e_idx, ptr_data_size, pptr_data);
}

/* FUNCTION NAME: dbapi_getStartUp
 * PURPOSE:
 *      Get the startup configuration value for specific request.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
dbapi_getStartUp(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_INITED);
    return db_getStartUp(t_idx, f_idx, e_idx, ptr_data_size, pptr_data);
}

/* FUNCTION NAME: dbapi_getDataFromRawTable
 * PURPOSE:
 *      Get data by specific request form a DB table raw data
 *
 * INPUT:
 *      ptr_raw_data      -- A pointer to the starting address of the raw data in a DB table
 *      req               -- A specific request
 *
 * OUTPUT:
 *      ptr_out_data      -- A pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dbapi_getDataFromRawTable(
    UI8_T *ptr_out_data,
    const UI8_T *ptr_raw_data,
    const DB_REQUEST_TYPE_T req)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_NOT_SUPPORT);
    return db_getDataFromRawTable(ptr_out_data, ptr_raw_data, req);
}

/* FUNCTION NAME: dbapi_isStartUpConfigExist
 * PURPOSE:
 *      Check if the startup configuration exists in the database.
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
 *      MW_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dbapi_isStartUpConfigExist(
    void)
{
    MW_PARAM_CHK((MW_E_NOT_INITED == db_readyFlagGet()), MW_E_BAD_PARAMETER);
    return db_isStartUpConfigExist();
}