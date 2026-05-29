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

/* FILE NAME:  syncd_api_lag.c
 * PURPOSE:
 *  Implement lag API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <syncd_api_lag.h>
#include <syncd_api_l2.h>
#include <air_lag.h>
#include <air_port.h>
#include <mw_platform.h>
#include <mw_log.h>
#include <vlan_utils.h>
#include <air_l2.h>
#include "air_stp.h"
#include "air_swc.h"
#include <default_config.h>
#ifdef AIR_SUPPORT_LACP
#include "mw_lacp.h"
#include "syncd_api_stp.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
MW_ERROR_NO_T
_re_port_setting(
    UI32_T del_member,
    UI32_T add_member);

MW_ERROR_NO_T
_re_pvlan_setting(
    UI32_T del_member,
    UI32_T add_member);

MW_ERROR_NO_T
_re_qvlan_setting(
    UI32_T del_member,
    UI32_T add_member);

MW_ERROR_NO_T
_re_mvlan_setting(
    UI32_T del_member,
    UI32_T add_member);

MW_ERROR_NO_T
_re_mac_setting(
    UI32_T member);

/* STATIC VARIABLE DECLARATIONS
 */
static TRUNK_MEMBER_INFO_T _g_trunkMem[MAX_TRUNK_NUM];
static UI8_T lag_member_0[MAX_TRUNK_NUM] = { 0 };
#ifdef AIR_SUPPORT_LACP
static UI32_T _g_aggregatedMem[MAX_TRUNK_NUM];
#endif

/* LOCAL SUBPROGRAM BODIES
 */
AIR_ERROR_NO_T
_flushTrunkMacAddr(
    UI32_T portlist)
{
    UI32_T i = 0, unit = 0;
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (0 != (portlist & BIT(i)))
        {
            rc = air_l2_flushMacAddr(unit, AIR_L2_MAC_FLUSH_TYPE_PORT, i + 1);
            if (AIR_E_OK != rc)
            {
                MW_LOG_ERROR(LAG, "flush port %d MAC addr fail", i + 1);
                break;
            }
        }
    }

    return rc;
}

UI8_T
_getLowerPort(
    UI32_T portlist)
{
    UI32_T i = 0;
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (0 != (portlist & BIT(i)))
        {
            break;
        }
    }

    return i;
}

MW_ERROR_NO_T
_re_port_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL, *value = NULL;
    UI16_T size = 0, i = 0, idx = 0;
    UI8_T lowerPort = 0;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;
    UI8_T    adminState[MAX_PORT_NUM] = { 0 };
    UI8_T    adminStatus = FALSE, data = 0;

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        return MW_E_OTHERS;
    }
    osapi_memset(adminState, 0, sizeof(UI8_T) * MAX_PORT_NUM);

    if (0 != del_member)
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (del_member & BIT(i)))
            {
                for (idx = PORT_ADMIN_STATUS; idx < PORT_CFG_INFO_LAST; idx++)
                {
                    BOOL_T  get_default_value = FALSE;
                    BOOL_T  get_db_flash = FALSE;
                    if (PORT_ISOLATION == idx || PORT_MIRROR_ID == idx)
                    {
                        continue;
                    }
                    data = 0;
                    ret = syncd_queue_db_getData(M_GETFLASH,
                            PORT_CFG_INFO,
                            idx,
                            (i + 1),
                            &ptr_msg,
                            &size,
                            (void **)&ptr_data);
                    if (MW_E_OK != ret)
                    {
                        ret = dbapi_getFactoryDefault(
                            PORT_CFG_INFO,
                            idx,
                            (i + 1),
                            &size,
                            (void **)&ptr_data);
                        if (MW_E_OK == ret)
                        {
                            get_default_value = TRUE;
                        }
                    }
                    else
                    {
                        get_db_flash = TRUE;
                    }

                    if (get_db_flash || get_default_value)
                    {
                        (PORT_TRUNK_ID == idx) ? (value = (UI8_T *)&data) : (value = ptr_data);
                        if (PORT_ADMIN_STATUS == idx)
                        {
                            memcpy(&adminStatus, ptr_data, sizeof(adminStatus));
                            if (TRUE == adminStatus)
                            {
                                adminState[i] = TRUE;
                                value = (UI8_T *)&data;
                            }
                        }
                        request.t_idx = PORT_CFG_INFO;
                        request.f_idx = idx;
                        request.e_idx = (i + 1);
                        ret = dbapi_appendMsgPayload(&request, (UI8_T *)value, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            if (TRUE == get_db_flash)
                            {
                                MW_FREE(ptr_msg);
                            }
                            if (TRUE == get_default_value)
                            {
                                MW_FREE(ptr_data);
                            }
                            MW_FREE(ptr_multi_msg);
                            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                            return ret;
                        }
                        else if (FALSE == need_send_msg)
                        {
                            need_send_msg = TRUE;
                        }

                        if (TRUE == get_db_flash)
                        {
                            MW_FREE(ptr_msg);
                        }
                        if (TRUE == get_default_value)
                        {
                            MW_FREE(ptr_data);
                        }
                    }
                    else
                    {
                        MW_LOG_ERROR(LAG, "%s: Trunk get flash or default value fail", __func__);
                    }
                }
            }
        }
    }

    if (0 != add_member)
    {
        lowerPort = _getLowerPort(add_member);
        for (idx = PORT_ADMIN_STATUS; idx < PORT_CFG_INFO_LAST; idx++)
        {
            if (PORT_ISOLATION == idx || PORT_MIRROR_ID == idx)
            {
                continue;
            }
            ret = syncd_queue_db_getData(M_GET,
                    PORT_CFG_INFO,
                    idx,
                    (lowerPort + 1),
                    &ptr_msg,
                    &size,
                    (void **)&ptr_data);
            if (MW_E_OK != ret)
            {
                MW_LOG_ERROR(LAG, "%s: syncd_queue_db_getData fail", __func__);
                continue;
            }
            memcpy(&adminStatus, ptr_data, sizeof(adminStatus));
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {

                if ((0 != (add_member & BIT(i))) && (i != lowerPort))
                {
                    if (PORT_ADMIN_STATUS == idx)
                    {
                        if (TRUE == adminStatus)
                        {
                            adminState[i] = TRUE;
                            *ptr_data = FALSE;
                        }
                    }
                    request.t_idx = PORT_CFG_INFO;
                    request.f_idx = idx;
                    request.e_idx = (i + 1);
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_data, &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_msg);
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        return ret;
                    }
                    else if (FALSE == need_send_msg)
                    {
                        need_send_msg = TRUE;
                    }
                }
            }
            MW_FREE(ptr_msg);
        }
    }

    if (TRUE == need_send_msg)
    {
        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk set multiple DB data fail", __func__, __LINE__);
        }
        payload_size = DB_MSG_PAYLOAD_SIZE + 1;
        ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
        if (NULL == ptr_multi_msg)
        {
            MW_LOG_ERROR(LAG, "%s: Trunk create DB msg fail", __func__);
            return MW_E_OTHERS;
        }

        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 == adminState[i])
            {
                continue;
            }
            request.t_idx = PORT_CFG_INFO;
            request.f_idx = PORT_ADMIN_STATUS;
            request.e_idx = (i + 1);

            ret = dbapi_appendMsgPayload(&request, (UI8_T *)&adminState[i], &ptr_multi_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != ret)
            {
                MW_FREE(ptr_multi_msg);
                MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                return ret;
            }
        }
        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk set multiple DB data fail", __func__, __LINE__);
        }
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }

    return ret;
}

MW_ERROR_NO_T
_re_pvlan_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0, port = 0, vid = 0, i = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
    UI32_T *ptr_port_list = NULL;
    UI32_T *ptr_port_matrix_tbl = NULL;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;
    BOOL_T is_changed = FALSE;
    UI32_T tmp_bmp;
    UI8_T factory_vlan_mode = 0;
    UI8_T *ptr_data = NULL;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;
    BOOL_T get_db_flash = FALSE;
    BOOL_T get_default_value = FALSE;

    /* Get factory vlan mode */
    ret = syncd_queue_db_getData(M_GETFLASH, VLAN_CFG_INFO, VLAN_PORT_B_ENABLE, 1, &ptr_msg, &size, (void **)&ptr_data);
    if (MW_E_OK != ret)
    {
        ret = dbapi_getFactoryDefault(
            VLAN_CFG_INFO,
            VLAN_PORT_B_ENABLE,
            1,
            &size,
            (void **)&ptr_data);
        if (MW_E_OK == ret)
        {
            get_default_value = TRUE;
        }
    }
    else
    {
        get_db_flash = TRUE;
    }

    if ((FALSE == get_db_flash) && (FALSE == get_default_value))
    {
        MW_LOG_ERROR(LAG, "%s: Trunk get flash or default value fail", __func__);
        return ret;
    }

    factory_vlan_mode = (TRUE == *ptr_data) ? VLAN_PORT_ENABLE : VLAN_NONE;
    MW_LOG_INFO(LAG, "factory_vlan_mode %d", factory_vlan_mode);
    if (TRUE == get_db_flash)
    {
        MW_FREE(ptr_msg);
    }
    if (TRUE == get_default_value)
    {
        MW_FREE(ptr_data);
    }

    /* Get DB PORT_VLAN_LIST */
    ret = syncd_queue_db_getData(M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    if (MW_E_OK != ret)
    {
        return ret;
    }
    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        return MW_E_OTHERS;
    }
    if ((VLAN_NONE == factory_vlan_mode) && (0 != del_member))
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (del_member & BIT(i)))
            {
                ptr_vlan_list_tbl[i] = VLAN_DEFAULT_VID;
            }
        }
        request.t_idx = PORT_CFG_INFO;
        request.f_idx = PORT_VLAN_LIST;
        request.e_idx = DB_ALL_ENTRIES;
        ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_vlan_list_tbl, &ptr_multi_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            MW_FREE(ptr_multi_msg);
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
            return ret;
        }
        else
        {
            need_send_msg = TRUE;
        }
    }
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        MW_LOG_INFO(LAG, "port=%d, new vlan_list_tbl=0x%x", i + 1, ptr_vlan_list_tbl[i]);
    }

    /* Recalculation VLAN_ENTRY port member*/
    ret = osapi_calloc((sizeof(UI32_T) * MAX_VLAN_ENTRY_NUM), SYNCD_NAME, (void **)&ptr_port_list);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_multi_msg);
        MW_LOG_ERROR(LAG, "%s: alloc ptr_port_list failed(%d) ", __func__, ret);
        return ret;
    }
    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        BITMAP_VLAN_FOREACH(ptr_vlan_list_tbl[port], vid)
        {
            ptr_port_list[vid] |= BIT(port + 1);
        }
    }
    MW_FREE(ptr_msg);

    /* Get DB VLAN_ENTRY */
    ret = syncd_queue_db_getData(M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_multi_msg);
        MW_FREE(ptr_port_list);
        return ret;
    }
    for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
    {
        MW_LOG_INFO(LAG, "vid=%d port_member=0x%x", vid + 1, ptr_vlan_entry_tbl->port_member[vid]);
    }

    /* Update VLAN_ENTRY port member*/
    for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
    {
        if (ptr_vlan_entry_tbl->port_member[vid] != ptr_port_list[vid])
        {
            ptr_vlan_entry_tbl->port_member[vid] = ptr_port_list[vid];
            is_changed = TRUE;
        }
    }
    MW_FREE(ptr_port_list);

    if (TRUE == is_changed)
    {
        /* Update VLAN_ENTRY */
        request.t_idx = VLAN_ENTRY;
        request.f_idx = DB_ALL_FIELDS;
        request.e_idx = DB_ALL_ENTRIES;
        ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_vlan_entry_tbl, &ptr_multi_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            MW_FREE(ptr_multi_msg);
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail(VLAN ENTRY)", __func__, __LINE__);
            return ret;
        }
        else
        {
            need_send_msg = TRUE;
        }

        /* Update PORT_ISOLATION */
        ret = osapi_calloc((sizeof(UI32_T) * MAX_PORT_NUM), SYNCD_NAME, (void **)&ptr_port_matrix_tbl);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            MW_FREE(ptr_multi_msg);
            MW_LOG_ERROR(LAG, "%s: alloc ptr_port_matrix_tbl failed(%d) ", __func__, ret);
            return ret;
        }
        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            BITMAP_PORT_FOREACH(ptr_vlan_entry_tbl->port_member[vid], port)
            {
                if (PLAT_CPU_PORT == port)
                {
                    continue;
                }

                BITMAP_PORT_ADD(tmp_bmp, ptr_port_matrix_tbl[port - 1], ptr_vlan_entry_tbl->port_member[vid]);
                ptr_port_matrix_tbl[port - 1] = tmp_bmp;
            }
        }

        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            MW_LOG_INFO(LAG, "port=%d, new port_matrix=0x%x", i + 1, ptr_port_matrix_tbl[i]);
        }

        request.t_idx = PORT_CFG_INFO;
        request.f_idx = PORT_ISOLATION;
        request.e_idx = DB_ALL_ENTRIES;
        ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_port_matrix_tbl, &ptr_multi_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg);
            MW_FREE(ptr_multi_msg);
            MW_FREE(ptr_port_matrix_tbl);
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail(port matrix)", __func__, __LINE__);
            return ret;
        }
        else
        {
            need_send_msg = TRUE;
        }
    }
    MW_FREE(ptr_msg);
    MW_FREE(ptr_port_matrix_tbl);

    if (TRUE == need_send_msg)
    {
        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
        }
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }

    return ret;
}

MW_ERROR_NO_T
_re_qvlan_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL, *ptr_msg_mode = NULL, *ptr_msg_tag = NULL, *ptr_msg_untag = NULL, *ptr_msg_vlan = NULL;
    UI16_T size = 0, port = 0, vid = 0, i = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
    UI32_T *ptr_vlan_entry_tagged_member = NULL;
    UI32_T *ptr_vlan_entry_untagged_member = NULL;
    UI32_T *ptr_vlan_list = NULL;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;
    DB_VLAN_ENTRY_T vlan_tbl;
    UI8_T lowerPort = 0;
    UI8_T *ptr_data = NULL;
    UI16_T *ptr_vlan = NULL;
    UI8_T factory_vlan_mode = 0;
    UI16_T vlan_fact = 0, cnt = 0;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;
    BOOL_T get_db_flash = FALSE;
    BOOL_T get_default_value = FALSE;
    BOOL_T get_db_flash2 = FALSE;
    BOOL_T get_default_value2 = FALSE;

    osapi_memset(&vlan_tbl, 0, sizeof(vlan_tbl));
    /* Get DB PORT_VLAN_LIST */
    ret = syncd_queue_db_getData(M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    if (MW_E_OK != ret)
    {
        return ret;
    }
    ret = osapi_calloc((sizeof(UI32_T) * MAX_PORT_NUM), SYNCD_NAME, (void **)&ptr_vlan_list);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        MW_LOG_ERROR(LAG, "%s: alloc ptr_vlan_list failed(%d) ", __func__, ret);
        return ret;
    }
    osapi_memcpy(ptr_vlan_list, ptr_vlan_list_tbl, size);
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        MW_LOG_INFO(LAG, "port=%d, new vlan_list_tbl=0x%x", i + 1, ptr_vlan_list[i]);
    }
    MW_FREE(ptr_msg);

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        MW_FREE(ptr_vlan_list);
        return MW_E_OTHERS;
    }

    if (0 != del_member)
    {
        lowerPort = _getLowerPort(del_member);
        MW_LOG_INFO(LAG, "lowerPort %d", lowerPort);
        /* Get DB VLAN_ENTRY */
        ret = syncd_queue_db_getData(M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_multi_msg);
            MW_FREE(ptr_vlan_list);
            return ret;
        }
        osapi_memcpy(&vlan_tbl, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
        MW_FREE(ptr_msg);

        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            MW_LOG_INFO(LAG, "vid=%d tagged_member=0x%x untagged_member=0x%x", vid + 1, vlan_tbl.tagged_member[vid], vlan_tbl.untagged_member[vid]);
        }
        /* Delete trunk port first from all vlan member */
        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            vlan_tbl.tagged_member[vid] &= ~(del_member << 1);
            vlan_tbl.untagged_member[vid] &= ~(del_member << 1);
        }
        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            MW_LOG_INFO(LAG, "vid=%d tagged_member=0x%x untagged_member=0x%x", vid + 1, vlan_tbl.tagged_member[vid], vlan_tbl.untagged_member[vid]);
        }

        /* Recover trunk port default settings */
        /* Get factory vlan mode */
        ret = syncd_queue_db_getData(M_GETFLASH, VLAN_CFG_INFO, VLAN_8021Q_B_ENABLE, 1, &ptr_msg_mode, &size, (void **)&ptr_data);
        if (MW_E_OK != ret)
        {
            ret = dbapi_getFactoryDefault(
                VLAN_CFG_INFO,
                VLAN_8021Q_B_ENABLE,
                1,
                &size,
                (void **)&ptr_data);
            if (MW_E_OK == ret)
            {
                get_default_value = TRUE;
            }
        }
        else
        {
            get_db_flash = TRUE;
        }

        if ((FALSE == get_db_flash) && (FALSE == get_default_value))
        {
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk get flash or default value fail", __func__, __LINE__);
            MW_FREE(ptr_multi_msg);
            MW_FREE(ptr_vlan_list);
            return ret;
        }
        factory_vlan_mode = (TRUE == *ptr_data) ? VLAN_1Q_ENABLE : VLAN_NONE;
        MW_LOG_INFO(LAG, "factory_vlan_mode %d", factory_vlan_mode);
        if (TRUE == get_db_flash)
        {
            MW_FREE(ptr_msg_mode);
        }
        if (TRUE == get_default_value)
        {
            MW_FREE(ptr_data);
        }

/* current vlan mode == factory vlan mode */
        if (VLAN_1Q_ENABLE == factory_vlan_mode)
        {
            BITMAP_PORT_FOREACH(del_member, port)
            {
                /* factory vlan idx */
                BITMAP_VLAN_FOREACH(ptr_vlan_list[port], vid)
                {
                    get_db_flash = FALSE;
                    get_default_value = FALSE;
                    ret = syncd_queue_db_getData(M_GETFLASH, VLAN_ENTRY, VLAN_ID, vid + 1, &ptr_msg_vlan, &size, (void **)&ptr_vlan);
                    if (MW_E_OK != ret)
                    {
                        ret = dbapi_getFactoryDefault(
                            VLAN_ENTRY,
                            VLAN_ID,
                            vid + 1,
                            &size,
                            (void **)&ptr_vlan);
                        if (MW_E_OK == ret)
                        {
                            get_default_value = TRUE;
                        }
                    }
                    else
                    {
                        get_db_flash = TRUE;
                    }
                    if ((FALSE == get_db_flash) && (FALSE == get_default_value))
                    {
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk get flash or default value fail", __func__, __LINE__);
                        MW_FREE(ptr_multi_msg);
                        MW_FREE(ptr_vlan_list);
                        return ret;
                    }
                    vlan_fact = *ptr_vlan;
                    if (TRUE == get_db_flash)
                    {
                        MW_FREE(ptr_msg_vlan);
                    }
                    if (TRUE == get_default_value)
                    {
                        MW_FREE(ptr_vlan);
                    }

                    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
                    {
                        if (vlan_fact == vlan_tbl.vlan_id[i])
                        {
                            get_db_flash = FALSE;
                            get_default_value = FALSE;
                            ret = syncd_queue_db_getData(M_GETFLASH, VLAN_ENTRY, VLAN_TAGGED_MEMBER, vid + 1, &ptr_msg_tag, &size, (void **)&ptr_vlan_entry_tagged_member);
                            if (MW_E_OK != ret)
                            {
                                ret = dbapi_getFactoryDefault(
                                    VLAN_ENTRY,
                                    VLAN_TAGGED_MEMBER,
                                    vid + 1,
                                    &size,
                                    (void **)&ptr_vlan_entry_tagged_member);
                                if (MW_E_OK == ret)
                                {
                                    get_default_value = TRUE;
                                }
                            }
                            else
                            {
                                get_db_flash = TRUE;
                            }
                            if ((FALSE == get_db_flash) && (FALSE == get_default_value))
                            {
                                MW_LOG_ERROR(LAG, "%s[%d]: Trunk get flash or default value fail", __func__, __LINE__);
                                MW_FREE(ptr_multi_msg);
                                MW_FREE(ptr_vlan_list);
                                return ret;
                            }

                            get_db_flash2 = FALSE;
                            get_default_value2 = FALSE;
                            ret = syncd_queue_db_getData(M_GETFLASH, VLAN_ENTRY, VLAN_UNTAGGED_MEMBER, vid + 1, &ptr_msg_untag, &size, (void **)&ptr_vlan_entry_untagged_member);
                            if (MW_E_OK != ret)
                            {
                                ret = dbapi_getFactoryDefault(
                                    VLAN_ENTRY,
                                    VLAN_UNTAGGED_MEMBER,
                                    vid + 1,
                                    &size,
                                    (void **)&ptr_vlan_entry_untagged_member);
                                if (MW_E_OK == ret)
                                {
                                    get_default_value2 = TRUE;
                                }
                            }
                            else
                            {
                                get_db_flash2 = TRUE;
                            }
                            if ((FALSE == get_db_flash2) && (FALSE == get_default_value2))
                            {
                                MW_LOG_ERROR(LAG, "%s[%d]: Trunk get flash or default value fail", __func__, __LINE__);
                                MW_FREE(ptr_multi_msg);
                                if (TRUE == get_db_flash)
                                {
                                    MW_FREE(ptr_msg_tag);
                                }
                                if (TRUE == get_default_value)
                                {
                                    MW_FREE(ptr_vlan_entry_tagged_member);
                                }
                                MW_FREE(ptr_vlan_list);
                                return ret;
                            }

                            MW_LOG_INFO(LAG, "factory vid=%d tagged_member=0x%x untagged_member=0x%x", vid + 1, *ptr_vlan_entry_tagged_member, *ptr_vlan_entry_untagged_member);
                            if (0 != (*ptr_vlan_entry_tagged_member & BIT(port + 1)))
                            {
                                vlan_tbl.tagged_member[i] |= BIT(port + 1);
                            }
                            else if (0 != (*ptr_vlan_entry_untagged_member & BIT(port + 1)))
                            {
                                vlan_tbl.untagged_member[i] |= BIT(port + 1);
                            }

                            if (TRUE == get_db_flash)
                            {
                                MW_FREE(ptr_msg_tag);
                            }
                            if (TRUE == get_default_value)
                            {
                                MW_FREE(ptr_vlan_entry_tagged_member);
                            }
                            if (TRUE == get_db_flash2)
                            {
                                MW_FREE(ptr_msg_untag);
                            }
                            if (TRUE == get_default_value2)
                            {
                                MW_FREE(ptr_vlan_entry_untagged_member);
                            }
                            cnt++;
                        }
                    }
                    if (0 == cnt)
                    {
                        vlan_tbl.untagged_member[VLAN_DEFAULT_VID - 1] |= BIT(port + 1);
                    }
                }
            }
        }
        /* current vlan mode != factory vlan mode, add port members of the deleted trunk to defalut vlan */
        else
        {
            vlan_tbl.untagged_member[VLAN_DEFAULT_VID - 1] |= (del_member << 1);
        }

        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            MW_LOG_INFO(LAG, "new vid=%d tagged_member=0x%x  untagged_member=0x%x", vid + 1, vlan_tbl.tagged_member[vid], vlan_tbl.untagged_member[vid]);
        }

        need_send_msg = TRUE;
    }

    MW_FREE(ptr_vlan_list);
    if (0 != add_member)
    {
        lowerPort = _getLowerPort(add_member);
        MW_LOG_INFO(LAG, "lowerPort %d", lowerPort);
        if (FALSE == need_send_msg)
        {
            /* Get DB VLAN_ENTRY */
            ret = syncd_queue_db_getData(M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
            if (MW_E_OK != ret)
            {
                MW_FREE(ptr_multi_msg);
                return ret;
            }
            osapi_memcpy(&vlan_tbl, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
            MW_FREE(ptr_msg);
        }

        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            MW_LOG_INFO(LAG, "vid=%d tagged_member=0x%x untagged_member=0x%x", vid + 1, vlan_tbl.tagged_member[vid], vlan_tbl.untagged_member[vid]);
        }
        for (vid = 0; vid < MAX_VLAN_ENTRY_NUM; vid++)
        {
            if (0 != (vlan_tbl.tagged_member[vid] + vlan_tbl.untagged_member[vid]))
            {
                /* If original vlan member include lowerport, add other trunk port to vlan member. */
                if (0 != (vlan_tbl.tagged_member[vid] & BIT(lowerPort + 1)))
                {
                    vlan_tbl.tagged_member[vid] |= (add_member << 1);
                    vlan_tbl.untagged_member[vid] &= ~(add_member << 1);
                }
                else if (0 != (vlan_tbl.untagged_member[vid] & BIT(lowerPort + 1)))
                {
                    vlan_tbl.untagged_member[vid] |= (add_member << 1);
                    vlan_tbl.tagged_member[vid] &= ~(add_member << 1);
                }
                else
                {
                    /* If original vlan member exclude lowerport and include other trunk port, delete other trunk port from vlan member. */
                    BITMAP_PORT_FOREACH(add_member, port)
                    {
                        if (lowerPort == port)
                        {
                            continue;
                        }
                        else
                        {
                            if (0 != (vlan_tbl.tagged_member[vid] & BIT(port + 1)))
                            {
                                vlan_tbl.tagged_member[vid] &= ~BIT(port + 1);
                            }
                            else if (0 != (vlan_tbl.untagged_member[vid] & BIT(port + 1)))
                            {
                                vlan_tbl.untagged_member[vid] &= ~BIT(port + 1);
                            }
                        }
                    }
                }
                MW_LOG_INFO(LAG, "new vid=%d tagged_member=0x%x untagged_member=0x%x", vid + 1, vlan_tbl.tagged_member[vid], vlan_tbl.untagged_member[vid]);
            }
        }

        need_send_msg = TRUE;
    }

    if (TRUE == need_send_msg)
    {
        request.t_idx = VLAN_ENTRY;
        request.f_idx = DB_ALL_FIELDS;
        request.e_idx = DB_ALL_ENTRIES;
        ret = dbapi_appendMsgPayload(&request, (UI8_T *)&vlan_tbl, &ptr_multi_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_multi_msg);
            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
            return ret;
        }

        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
        }
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }

    return ret;
}

MW_ERROR_NO_T
_re_mvlan_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0, i = 0;
    UI32_T *ptr_port_matrix_tbl = NULL;
    UI32_T *ptr_matrix_tbl = NULL;
    UI8_T lowerPort = 0;
    UI32_T port_total_mbr = 0, uplink_portbmp = 0;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;

    vlan_get_plat_max_portBmp(&port_total_mbr);

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        return MW_E_OTHERS;
    }

    ret = syncd_queue_db_getData(M_GET, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_port_matrix_tbl);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_multi_msg);
        return ret;
    }

    ret = osapi_calloc((sizeof(UI32_T) * MAX_PORT_NUM), SYNCD_NAME, (void **)&ptr_matrix_tbl);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        MW_FREE(ptr_multi_msg);
        MW_LOG_ERROR(LAG, "%s: alloc ptr_matrix_tbl failed(%d) ", __func__, ret);
        return ret;
    }
    osapi_memcpy(ptr_matrix_tbl, ptr_port_matrix_tbl, size);
    MW_FREE(ptr_msg);

    /* MTU VLAN remains unchanged when trunk is deleted */
    if (0 != add_member)
    {
        /* If there is an uplink port in add member port, all add members ports are set to uplink ports, otherwise they remain unchanged */
        lowerPort = _getLowerPort(add_member);
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if ((0 != (add_member & BIT(i))) && (port_total_mbr == ptr_matrix_tbl[i]))
            {
                osapi_memcpy(&(ptr_matrix_tbl[lowerPort]), &port_total_mbr, sizeof(UI32_T));
                break;
            }
        }
        if (port_total_mbr == ptr_matrix_tbl[lowerPort])
        {
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if ((0 != (add_member & BIT(i))) && (i != lowerPort))
                {

                    osapi_memcpy(&(ptr_matrix_tbl[i]), &port_total_mbr, sizeof(UI32_T));
                }
            }
        }
    }

    /* Get new uplink ports */
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        MW_LOG_INFO(LAG, "%s: ptr_matrix_tbl[%d]=0x%x", __func__, i, ptr_matrix_tbl[i]);
        if (port_total_mbr == ptr_matrix_tbl[i])
        {
            uplink_portbmp |= BIT(i + 1);
        }
    }
    /* Update portmatrix */
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (port_total_mbr != ptr_matrix_tbl[i])
        {
            ptr_matrix_tbl[i] = (BIT(i + 1) | uplink_portbmp);
        }
        MW_LOG_INFO(LAG, "%s: new ptr_matrix_tbl[%d]=0x%x", __func__, i, ptr_matrix_tbl[i]);
    }

    request.t_idx = PORT_CFG_INFO;
    request.f_idx = PORT_ISOLATION;
    request.e_idx = DB_ALL_ENTRIES;
    ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_matrix_tbl, &ptr_multi_msg, &msg_size, &ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_multi_msg);
        MW_FREE(ptr_matrix_tbl);
        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
        return ret;
    }
    MW_FREE(ptr_matrix_tbl);

    ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
    if (MW_E_OK != ret)
    {
        MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
    }

    return ret;
}

MW_ERROR_NO_T
_re_vlan_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T vlan_state = VLAN_NONE;

    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;

    /* Get DB VLAN_CFG_INFO */
    ret = syncd_queue_db_getData(M_GET, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_cfg);
    if (MW_E_OK != ret)
    {
        return ret;
    }
    if (TRUE == ptr_vlan_cfg->enable_port_b)
    {
        vlan_state = VLAN_PORT_ENABLE;
    }
    else if (TRUE == ptr_vlan_cfg->enable_8021q_b)
    {
        vlan_state = VLAN_1Q_ENABLE;
    }
    else if (TRUE == ptr_vlan_cfg->enable_mtu)
    {
        vlan_state = VLAN_MTU_ENABLE;
    }
    MW_FREE(ptr_msg);

    MW_LOG_INFO(LAG, "VLAN state(%d)", vlan_state);

    if (VLAN_PORT_ENABLE == vlan_state)
    {
        _re_pvlan_setting(del_member, add_member);
    }
    else if (VLAN_1Q_ENABLE == vlan_state)
    {
        _re_qvlan_setting(del_member, add_member);
    }
    else if (VLAN_MTU_ENABLE == vlan_state)
    {
        _re_mvlan_setting(del_member, add_member);
    }
    else
    {
        MW_LOG_ERROR(LAG, "invalid state(%d)", vlan_state);
    }
    return ret;
}

MW_ERROR_NO_T
_re_mac_setting(
    UI32_T member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0, i = 0, idx = 0;
    DB_STATIC_MAC_ENTRY_T *ptr_mac_entry = NULL;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;

    if (0 != member)
    {
        payload_size = DB_MSG_PAYLOAD_SIZE + 1;
        ptr_multi_msg = dbapi_createMsg(NULL, M_DELETE, 1, payload_size, &msg_size, &ptr_payload);
        if (NULL == ptr_multi_msg)
        {
            return MW_E_OTHERS;
        }
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (member & BIT(i)))
            {
                ret = syncd_queue_db_getData(M_GET,
                        STATIC_MAC_ENTRY,
                        DB_ALL_FIELDS,
                        DB_ALL_ENTRIES,
                        &ptr_msg,
                        &size,
                        (void **)&ptr_data);
                if (MW_E_OK == ret)
                {
                    ptr_mac_entry = (DB_STATIC_MAC_ENTRY_T *)ptr_data;
                    for (idx = 0; idx < MAX_STATIC_MAC_NUM; idx++)
                    {
                        if ((i + 1) == ptr_mac_entry->port[idx])
                        {
                            request.t_idx = STATIC_MAC_ENTRY;
                            request.f_idx = DB_ALL_FIELDS;
                            request.e_idx = (idx + 1);
                            ret = dbapi_appendMsgPayload(&request, NULL, &ptr_multi_msg, &msg_size, &ptr_payload);
                            if (MW_E_OK != ret)
                            {
                                MW_FREE(ptr_msg);
                                MW_FREE(ptr_multi_msg);
                                MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                                return ret;
                            }
                            else if (FALSE == need_send_msg)
                            {
                                need_send_msg = TRUE;
                            }
                        }
                    }
                    MW_FREE(ptr_msg);
                }
            }
        }
        if (TRUE == need_send_msg)
        {
            ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
            if (MW_E_OK != ret)
            {
                MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
            }
        }
        else
        {
            MW_FREE(ptr_multi_msg);
        }
    }
    return ret;
}

MW_ERROR_NO_T
_re_qos_setting(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0, i = 0, idx = 0;
    UI8_T lowerPort = 0;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        return MW_E_OTHERS;
    }

    if (0 != del_member)
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (del_member & BIT(i)))
            {
                for (idx = PORT_QOS_PRIORITY; idx < PORT_QOS_LAST; idx++)
                {
                    BOOL_T  get_default_value = FALSE;
                    BOOL_T  get_db_flash = FALSE;
                    ret = syncd_queue_db_getData(M_GETFLASH,
                            PORT_QOS,
                            idx,
                            (i + 1),
                            &ptr_msg,
                            &size,
                            (void **)&ptr_data);
                    if (MW_E_OK != ret)
                    {
                        ret = dbapi_getFactoryDefault(
                            PORT_QOS,
                            idx,
                            i + 1,
                            &size,
                            (void **)&ptr_data);
                        if (MW_E_OK == ret)
                        {
                            get_default_value = TRUE;
                        }
                    }
                    else
                    {
                        get_db_flash = TRUE;
                    }

                    if (get_db_flash || get_default_value)
                    {
                        request.t_idx = PORT_QOS;
                        request.f_idx = idx;
                        request.e_idx = (i + 1);
                        ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            if (TRUE == get_db_flash)
                            {
                                MW_FREE(ptr_msg);
                            }
                            if (TRUE == get_default_value)
                            {
                                MW_FREE(ptr_data);
                            }
                            MW_FREE(ptr_multi_msg);
                            MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                            return ret;
                        }
                        else if (FALSE == need_send_msg)
                        {
                            need_send_msg = TRUE;
                        }
                        if (TRUE == get_db_flash)
                        {
                            MW_FREE(ptr_msg);
                        }
                        if (TRUE == get_default_value)
                        {
                            MW_FREE(ptr_data);
                        }
                    }
                    else
                    {
                        MW_LOG_ERROR(LAG, "%s: Trunk get flash or default value fail", __func__);
                    }
                }
            }
        }
    }
    if (0 != add_member)
    {
        lowerPort = _getLowerPort(add_member);
        for (idx = PORT_QOS_PRIORITY; idx < PORT_QOS_LAST; idx++)
        {
            ret = syncd_queue_db_getData(M_GET,
                    PORT_QOS,
                    idx,
                    (lowerPort + 1),
                    &ptr_msg,
                    &size,
                    (void **)&ptr_data);
            if (MW_E_OK != ret)
            {
                MW_LOG_ERROR(LAG, "%s: [M_GET][QoS]syncd_queue_db_getData fail", __func__);
                continue;
            }
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if ((0 != (add_member & BIT(i))) && (i != lowerPort))
                {
                    request.t_idx = PORT_QOS;
                    request.f_idx = idx;
                    request.e_idx = (i + 1);
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_data, &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_msg);
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        return ret;
                    }
                    else if (FALSE == need_send_msg)
                    {
                        need_send_msg = TRUE;
                    }
                }
            }
            MW_FREE(ptr_msg);
        }
    }

    if (TRUE == need_send_msg)
    {
        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
        }
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }

    return ret;
}

MW_ERROR_NO_T
_re_port_mirror(
    UI32_T del_member,
    UI32_T add_member)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg1 = NULL, *ptr_msg2 = NULL, *ptr_msg3 = NULL, *ptr_msg4 = NULL;
    UI16_T size1 = 0, size2 = 0, size3 = 0, size4 = 0, i = 0, j = 0;
    UI8_T *ptr_enable = NULL, *ptr_dest = NULL, *ptr_in_port = NULL, *ptr_eg_port = NULL;
    UI8_T *ptr_port_mirror_info = NULL;
    UI8_T lowerPort = 0, lowerPortBit = 0;
    BOOL_T update = FALSE;
    SYNCD_PORT_MIRROR_DB_ONE_ENTRY_T mirror_info;
    BOOL_T need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL, payload_size = 0;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_multi_msg = NULL;
    UI32_T in_port = 0, eg_port = 0;

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_multi_msg)
    {
        return MW_E_OTHERS;
    }

    if (del_member || add_member)
    {
        /* Set ptr_port_mirror_info to 0 no matter if it is deleting trunk or adding trunk because only when trunk/port is not used by an enabled session
         * can it be deleted or added.
         */
        if (MW_E_OK == syncd_queue_db_getData(M_GET, PORT_CFG_INFO, PORT_MIRROR_ID, DB_ALL_ENTRIES, &ptr_msg1, &size1, (void **)&ptr_port_mirror_info))
        {
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if ((BIT(i) & del_member) || (BIT(i) & add_member))
                {
                    ptr_port_mirror_info[i] = 0;
                }
            }

            request.t_idx = PORT_CFG_INFO;
            request.f_idx = PORT_MIRROR_ID;
            request.e_idx = DB_ALL_ENTRIES;
            ret = dbapi_appendMsgPayload(&request, (UI8_T *)ptr_port_mirror_info, &ptr_multi_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != ret)
            {
                MW_FREE(ptr_msg1);
                MW_FREE(ptr_multi_msg);
                MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                return ret;
            }
            else
            {
                need_send_msg = TRUE;
            }
            MW_FREE(ptr_msg1);
            ptr_msg1 = NULL;
        }

        /* Do not get factory settings from flash because factory settings may be conflict with the settings of other ports which are not trunk members. */
        ret = syncd_queue_db_getData(M_GET, PORT_MIRROR_INFO, PORT_MIRROR_ENABLE, DB_ALL_ENTRIES, &ptr_msg1, &size1, (void **)&ptr_enable);
        ret |= syncd_queue_db_getData(M_GET, PORT_MIRROR_INFO, PORT_MIRROR_DEST_PORT, DB_ALL_ENTRIES, &ptr_msg2, &size2, (void **)&ptr_dest);
        ret |= syncd_queue_db_getData(M_GET, PORT_MIRROR_INFO, PORT_MIRROR_SRC_IN_PORT, DB_ALL_ENTRIES, &ptr_msg3, &size3, (void **)&ptr_in_port);
        ret |= syncd_queue_db_getData(M_GET, PORT_MIRROR_INFO, PORT_MIRROR_SRC_EG_PORT, DB_ALL_ENTRIES, &ptr_msg4, &size4, (void **)&ptr_eg_port);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_msg1);
            MW_FREE(ptr_msg2);
            MW_FREE(ptr_msg3);
            MW_FREE(ptr_msg4);
            return ret;
        }

        if (add_member)
        {
            lowerPort = _getLowerPort(add_member);
            for (i = 0; i < MAX_MIRROR_SESS_NUM; i++)
            {
                /* Process dst_port */
                if (ptr_enable[i] &&
                    (BIT(ptr_dest[i] - 1) & add_member) &&
                    (lowerPort + 1) != ptr_dest[i])
                {
                    /* Set dst_port to 0 for session i if its dst_port is a trunk member except the member with lowest port id. */
                    if (0 != ptr_enable[i])
                    {
                        ret = MW_E_NOT_SUPPORT;
                        MW_LOG_ERROR(LAG, "Session:%d is not disabled when add a trunk. dst_port:%d add_member:%d lowerPort:%d",
                                    (i + 1), ptr_dest[i], add_member, lowerPort);
                        break;
                    }
                    ptr_dest[i] = 0;
                    request.t_idx = PORT_MIRROR_INFO;
                    request.f_idx = PORT_MIRROR_DEST_PORT;
                    request.e_idx = i + 1;
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)&ptr_dest[i], &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        MW_FREE(ptr_msg1);
                        MW_FREE(ptr_msg2);
                        MW_FREE(ptr_msg3);
                        MW_FREE(ptr_msg4);
                        return ret;
                    }
                    else
                    {
                        need_send_msg = TRUE;
                    }
                }

                /* Process src_port. Set all other trunk members to be the same as the trunk member with lowest port ID. */
                for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
                {
                    if (j != lowerPort && (BIT(j) & add_member))
                    {
                        memcpy((void *)&in_port, (UI32_T *)ptr_in_port + i, sizeof(UI32_T));
                        update = TRUE;
                        lowerPortBit = (in_port & BIT(lowerPort)) ? 1 : 0;
                        if (lowerPortBit)
                        {
                            in_port |= BIT(j);
                        }
                        else
                        {
                            in_port &= ~BIT(j);
                        }

                        lowerPortBit = (in_port & BIT(lowerPort)) ? 1 : 0;
                        if (lowerPortBit)
                        {
                            ((UI32_T *)ptr_eg_port)[i] |= BIT(j);
                        }
                        else
                        {
                            ((UI32_T *)ptr_eg_port)[i] &= ~BIT(j);
                        }
                    }
                }

                if (update)
                {
                    update = FALSE;
                    mirror_info.enable = ptr_enable[i];
                    mirror_info.dest_port = ptr_dest[i];
                    memcpy((void *)&(mirror_info.src_in_port), (UI32_T *)ptr_in_port + i, sizeof(UI32_T));
                    memcpy((void *)&(mirror_info.src_eg_port), (UI32_T *)ptr_eg_port + i, sizeof(UI32_T));
                    request.t_idx = PORT_MIRROR_INFO;
                    request.f_idx = DB_ALL_FIELDS;
                    request.e_idx = i + 1;
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)&mirror_info, &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        MW_FREE(ptr_msg1);
                        MW_FREE(ptr_msg2);
                        MW_FREE(ptr_msg3);
                        MW_FREE(ptr_msg4);
                        return ret;
                    }
                    else
                    {
                        need_send_msg = TRUE;
                    }
                }
            }
        }

        if (del_member)
        {
            for (i = 0; i < MAX_MIRROR_SESS_NUM; i++)
            {
                /* Process dst_port */
                if (BIT(ptr_dest[i] - 1) & del_member)
                {
                    /* Set dst_port to 0 for session i if its dst_port is the trunk to be deleted. */
                    if (0 != ptr_enable[i])
                    {
                        ret = MW_E_NOT_SUPPORT;
                        MW_LOG_ERROR(LAG, "Session:%d is not disabled when delete a trunk. dst_port:%d", (i + 1), ptr_dest[i]);
                        break;
                    }
                    ptr_dest[i] = 0;
                    request.t_idx = PORT_MIRROR_INFO;
                    request.f_idx = PORT_MIRROR_DEST_PORT;
                    request.e_idx = i + 1;
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)&ptr_dest[i], &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        MW_FREE(ptr_msg1);
                        MW_FREE(ptr_msg2);
                        MW_FREE(ptr_msg3);
                        MW_FREE(ptr_msg4);
                        return ret;
                    }
                    else
                    {
                        need_send_msg = TRUE;
                    }
                }

                /* Process src_port. Clear all. */
                for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
                {
                    if (BIT(j) & del_member)
                    {
                        update = TRUE;
                        memcpy((void *)&in_port, (UI32_T *)ptr_in_port + i, sizeof(UI32_T));
                        memcpy((void *)&eg_port, (UI32_T *)ptr_eg_port + i, sizeof(UI32_T));
                        in_port &= ~BIT(j);
                        eg_port &= ~BIT(j);
                    }
                }

                if (update)
                {
                    update = FALSE;
                    mirror_info.enable = ptr_enable[i];
                    mirror_info.dest_port = ptr_dest[i];
                    memcpy((void *)&(mirror_info.src_in_port), (UI32_T *)ptr_in_port + i, sizeof(UI32_T));
                    memcpy((void *)&(mirror_info.src_eg_port), (UI32_T *)ptr_eg_port + i, sizeof(UI32_T));
                    request.t_idx = PORT_MIRROR_INFO;
                    request.f_idx = DB_ALL_FIELDS;
                    request.e_idx = i + 1;
                    ret = dbapi_appendMsgPayload(&request, (UI8_T *)&mirror_info, &ptr_multi_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_multi_msg);
                        MW_LOG_ERROR(LAG, "%s[%d]: Trunk append multiple DB data payload fail", __func__, __LINE__);
                        MW_FREE(ptr_msg1);
                        MW_FREE(ptr_msg2);
                        MW_FREE(ptr_msg3);
                        MW_FREE(ptr_msg4);
                        return ret;
                    }
                    else
                    {
                        need_send_msg = TRUE;
                    }
                }
            }
        }

        MW_FREE(ptr_msg1);
        MW_FREE(ptr_msg2);
        MW_FREE(ptr_msg3);
        MW_FREE(ptr_msg4);
    }

    if (TRUE == need_send_msg)
    {
        ret = syncd_queue_db_setMultiData(ptr_multi_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LAG, "%s: Trunk set multiple DB data fail", __func__);
        }
    }
    else
    {
        MW_FREE(ptr_multi_msg);
    }

    return ret;
}

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_lag_init
 * PURPOSE:
 *      Initialization lag information
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
syncd_api_lag_init()
{
    osapi_memset(_g_trunkMem, 0, MAX_TRUNK_MEMBER_NUM);

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_lag_config
 * PURPOSE:
 *      process lag function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
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
syncd_api_lag_config(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    TRUNK_MEMBER_INFO_T *ptr_member = NULL;
    UI32_T unit = 0;
    UI32_T member = 0;
    UI8_T trunk_mode = 0;
    UI32_T *ptr_member_arr = NULL;
    UI8_T idx = 0, group = 0, num = 0, j = 0, value = 0;
    UI32_T port;
    UI32_T add_member = 0, del_member = 0;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL;
    UI16_T payload_size = sizeof(DB_LAG_MEMBER_0_T);
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T process_state = LAG_PROCESS_STATE_IDLE;

    MW_CHECK_PTR(ptr_api_arg);

    ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_msg)
    {
        MW_LOG_ERROR(LAG, "Failed to create DB message");
        return MW_E_NO_MEMORY;
    }

    rc = osapi_calloc((sizeof(UI32_T) * MAX_TRUNK_MEMBER_NUM), SYNCD_NAME, (void **)&ptr_member_arr);
    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_msg);
        MW_LOG_ERROR(LAG, "%s: alloc ptr_member_arr failed(%d) ", __func__, rc);
        return rc;
    }

    num = (ptr_api_arg->data_size / sizeof(TRUNK_MEMBER_INFO_T));
    for (j = 0; j < num; j++)
    {
        idx = 0;
        osapi_memset(ptr_member_arr, 0, MAX_TRUNK_MEMBER_NUM);
        if (1 == num)
        {
            /* For single group */
            group = ptr_api_arg->ptr_type->e_idx;
            memcpy((void *)&member, &((TRUNK_MEMBER_INFO_T *)ptr_api_arg->ptr_data)->member_bmp, sizeof(UI32_T));
            memcpy((void *)&trunk_mode, &((TRUNK_MEMBER_INFO_T *)ptr_api_arg->ptr_data)->mode, sizeof(UI8_T));
        }
        else
        {
            /* For multiple group */
            group = (j + 1);
            ptr_member = (TRUNK_MEMBER_INFO_T *)ptr_api_arg->ptr_data;
            ptr_member += j;
            memcpy((void *)&member, &ptr_member->member_bmp, sizeof(UI32_T));
            memcpy((void *)&trunk_mode, &ptr_member->mode, sizeof(UI8_T));
        }
        MW_LOG_INFO(LAG, "%s: LAG configuration: group %d, member 0x%x, mode %d", __func__, group, member, trunk_mode);
        group -= 1; /* group id is 0-based in SDK */
        del_member = _g_trunkMem[group].member_bmp;
        if (0 == member)
        {
            request.t_idx = LAG_MEMBER_0_INFO;
            request.f_idx = MEMBER_0;
            request.e_idx = group + 1;
            rc = dbapi_appendMsgPayload(&request, (UI8_T *)&value, &ptr_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != rc)
            {
                MW_LOG_ERROR(LAG, "Fail to append LACP member 0 msg payload. rc=%d", rc);
                MW_FREE(ptr_member_arr);
                MW_FREE(ptr_msg);
                return rc;
            }

            /* Clear group member of specific group id */
            air_rc = air_lag_setMember(unit, group, 0, ptr_member_arr);
            if (AIR_E_OK != air_rc)
            {
                MW_FREE(ptr_member_arr);
                MW_FREE(ptr_msg);
                MW_LOG_ERROR(LAG, "Clear group %d member fail(%d)", group + 1, air_rc);
                return MW_E_OP_INCOMPLETE;
            }

            /* Destroy trunk group of specific group id */
            air_rc = air_lag_destroyGroup(unit, group);
            if (AIR_E_OK != air_rc)
            {
                MW_FREE(ptr_member_arr);
                MW_FREE(ptr_msg);
                MW_LOG_ERROR(LAG, "Destroy group %d fail(%d)", group + 1, air_rc);
                return MW_E_OP_INCOMPLETE;
            }

            /* Clear member of specific group id */
            _g_trunkMem[group].member_bmp = 0;
        }
        else
        {
            /* Record member of specific group id */
            _g_trunkMem[group].member_bmp = member;

            /* Enable trunk function of specific group id */
            air_rc = air_lag_createGroup(unit, group);
            if (AIR_E_OK != air_rc)
            {
                MW_FREE(ptr_member_arr);
                MW_FREE(ptr_msg);
                MW_LOG_ERROR(LAG, "Create group %d fail(%d)", group + 1, air_rc);
                return MW_E_OP_INCOMPLETE;
            }

            if ((0 != lag_member_0[group]) && (member & BIT(lag_member_0[group] - 1)))
            {
                ptr_member_arr[idx++] = lag_member_0[group];
            }
            /* Set group member of specific group id */
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
            {
                if ((PLAT_CPU_PORT == port) || (port == lag_member_0[group]))
                {
                    continue;
                }

                if (0 != (member & BIT(port - 1))) /* member uses 0-based port number */
                {
                    if (idx >= 1)
                    {
                        rc = syncd_api_transferPortStaticMacAddr(port, ptr_member_arr[0]);
                        MW_LOG_INFO(LAG, "LACP transfer port[%d] static MAC entry to member0 port[%d]. rc=%d", port, ptr_member_arr[0], rc);
                    }
                    ptr_member_arr[idx++] = port;
                }
            }

            MW_LOG_INFO(LAG, "LAG group %d member_0 = %d", group, ptr_member_arr[0]);

            lag_member_0[group] = ptr_member_arr[0];
            request.t_idx = LAG_MEMBER_0_INFO;
            request.f_idx = MEMBER_0;
            request.e_idx = group + 1;
            rc = dbapi_appendMsgPayload(&request, (UI8_T *)&lag_member_0[group], &ptr_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != rc)
            {
                MW_LOG_ERROR(LAG, "Fail to append LACP trunk port msg payload. rc=%d", rc);
                MW_FREE(ptr_member_arr);
                MW_FREE(ptr_msg);
                return rc;
            }

            if (LAG_MODE_STATIC == trunk_mode)
            {
                air_rc = air_lag_setMember(unit, group, idx, ptr_member_arr);
                if (AIR_E_OK != air_rc)
                {
                    MW_FREE(ptr_member_arr);
                    MW_FREE(ptr_msg);
                    MW_LOG_ERROR(LAG, "Add group %d member %x fail(%d)", group + 1, member, air_rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
        }
        add_member = _g_trunkMem[group].member_bmp;
#ifdef AIR_SUPPORT_LACP
        if (LAG_MODE_STATIC != _g_trunkMem[group].mode)
        {
            UI8_T i;
            AIR_SWC_MGMT_FRAME_CFG_T mgmt_frame_cfg = { 0 };
            mgmt_frame_cfg.flags = (AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE | AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU);
            mgmt_frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_02;
            if ((LAG_MODE_STATIC != trunk_mode) && (0 != add_member))
            {
                for (i = 1; i <= PLAT_MAX_PORT_NUM; i++)
                {
                    if (add_member & BIT(i - 1))
                    {
                        rc = syncd_api_stp_setPortStateByUser(i, AIR_STP_STATE_LISTEN, SYNCD_API_STP_USER_LACP);
                        if (MW_E_OK != rc)
                        {
                            MW_LOG_ERROR(LAG, "LACP block port[%d] failed, rc(%d)", i, rc);
                            rc = MW_E_OP_INCOMPLETE;
                        }
                        mgmt_frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
                        air_rc = air_swc_setPortMgmtFrameCfg(0, i, &mgmt_frame_cfg);
                        if (AIR_E_OK != air_rc)
                        {
                            MW_LOG_ERROR(LAG, "air_swc_setPortMgmtFrameCfg() to CPU only failed(%d) port:%d", air_rc, i);
                            rc = MW_E_OP_INCOMPLETE;
                        }
                    }
                }
            }
            if ((LAG_MODE_STATIC != _g_trunkMem[group].mode) && (0 != del_member))
            {
                for (i = 1; i <= PLAT_MAX_PORT_NUM; i++)
                {
                    if (del_member & BIT(i - 1))
                    {
                        rc = syncd_api_stp_setPortStateByUser(i, AIR_STP_STATE_FORWARD, SYNCD_API_STP_USER_LACP);
                        if (MW_E_OK != rc)
                        {
                            MW_LOG_ERROR(LAG, "LACP unblock port[%d] failed, rc(%d)", i, rc);
                            rc = MW_E_OP_INCOMPLETE;
                        }
                        mgmt_frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
                        air_rc = air_swc_setPortMgmtFrameCfg(0, i, &mgmt_frame_cfg);
                        if (AIR_E_OK != air_rc)
                        {
                            MW_LOG_ERROR(LAG, "air_swc_setPortMgmtFrameCfg() to Drop failed(%d) port:%d", air_rc, i);
                            rc = MW_E_OP_INCOMPLETE;
                        }
                    }
                }
            }
        }
#endif
        _g_trunkMem[group].mode = trunk_mode;
        /* Update related functions setting of specific group members */
        if ((0 != add_member) || (0 != del_member))
        {
            UI32_T oper_member = add_member | del_member;

            if ((M_GET == ptr_api_arg->method) || (oper_member & BIT(lag_member_0[group] - 1)))
            {
                oper_member &= ~BIT(lag_member_0[group] - 1);
            }
            _flushTrunkMacAddr(oper_member);
            _re_port_setting(del_member, add_member);
            _re_vlan_setting(del_member, add_member);
            _re_qos_setting(del_member, add_member);
            _re_mac_setting(oper_member);
            _re_port_mirror(del_member, add_member);
        }
    }

    request.t_idx = LAG_OPER_INFO;
    request.f_idx = LAG_PROCESS_STATE;
    request.e_idx = DB_ALL_ENTRIES;
    rc = dbapi_appendMsgPayload(&request, &process_state, &ptr_msg, &msg_size, &ptr_payload);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LAG, "Fail to append lag process state msg payload. rc=%d", rc);
        MW_FREE(ptr_member_arr);
        MW_FREE(ptr_msg);
        return rc;
    }
    if (NULL != ptr_msg)
    {
        if (ptr_msg->type.count > 0)
        {
            rc = syncd_queue_db_setMultiData(ptr_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        }
        else
        {
            MW_FREE(ptr_msg);
        }
    }

    MW_FREE(ptr_member_arr);

    return rc;
}

/* FUNCTION NAME: syncd_api_lag_lacp_config
 * PURPOSE:
 *      process lag lacp function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
#ifdef AIR_SUPPORT_LACP
MW_ERROR_NO_T
syncd_api_lag_lacp_config(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    UI32_T unit = 0;
    UI32_T *ptr_member_arr = NULL;
    UI8_T idx = 0, i = 0, j = 0;
    UI32_T port;
    UI8_T lacp_state[MAX_PORT_NUM] = { 0 };
    UI32_T aggregated_ports[MAX_TRUNK_NUM] = { 0 };
    UI32_T block_bmp = 0, unblock_bmp = 0;
    BOOL_T adjust_member_0 = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL;
    UI16_T payload_size = sizeof(DB_LAG_MEMBER_0_T);
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_msg = NULL;
    AIR_STP_STATE_T stp_state;

    MW_CHECK_PTR(ptr_api_arg);

    ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_msg)
    {
        MW_LOG_ERROR(LACP, "Failed to create DB message");
        return MW_E_NO_MEMORY;
    }

    rc = osapi_calloc((sizeof(UI32_T) * MAX_TRUNK_MEMBER_NUM), SYNCD_NAME, (void **)&ptr_member_arr);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LACP, "%s: alloc ptr_member_arr failed(%d) ", __func__, rc);
        MW_FREE(ptr_msg);
        return rc;
    }
    if (DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx)
    {
        memcpy((void *)&lacp_state, ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        for (i = 1; i <= PLAT_MAX_PORT_NUM; i++)
        {
            if (PORT_LACP_STATE_IDLE == lacp_state[i - 1])
            {
                if (MW_E_OK == syncd_api_stp_getPortStateByUser(i, SYNCD_API_STP_USER_LACP, &stp_state))
                {
                    if (AIR_STP_STATE_LISTEN == stp_state)
                    {
                        if (MW_E_OK != syncd_api_stp_setPortStateByUser(i, AIR_STP_STATE_FORWARD, SYNCD_API_STP_USER_LACP))
                        {
                            MW_LOG_ERROR(LACP, "LACP unblock port[%d] fail", i);
                            MW_FREE(ptr_member_arr);
                            rc = MW_E_OP_INCOMPLETE;
                            return rc;
                        }
                        else
                        {
                            MW_LOG_INFO(LACP, "LACP unblock port[%d] success", i);
                        }
                    }
                }
            }
        }

        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if ((0 != _g_trunkMem[i].member_bmp) && (LAG_MODE_STATIC != _g_trunkMem[i].mode))
            {
                for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
                {
                    if ((0 != (_g_trunkMem[i].member_bmp & BIT(j))) && PORT_LACP_STATE_AGGREGATED == lacp_state[j])
                    {
                        aggregated_ports[i] |= BIT(j);
                    }
                }
            }
            adjust_member_0 = FALSE;
            idx = 0;
            if ((0 != aggregated_ports[i]) && (0 == (aggregated_ports[i] & BIT(lag_member_0[i] - 1))))
            {
                adjust_member_0 = TRUE;
            }

            osapi_memset(ptr_member_arr, 0, MAX_TRUNK_MEMBER_NUM);
            if (_g_aggregatedMem[i] != aggregated_ports[i])
            {
                block_bmp = (_g_aggregatedMem[i] & ~aggregated_ports[i]) & _g_trunkMem[i].member_bmp;
                unblock_bmp = (aggregated_ports[i] & ~_g_aggregatedMem[i]) & _g_trunkMem[i].member_bmp;
                MW_LOG_INFO(LACP, "LACP aggregated ports:0x%x, block bmp:0x%x, unblock bmp:0x%x", aggregated_ports[i], block_bmp, unblock_bmp);
                _g_aggregatedMem[i] = aggregated_ports[i];
                if (FALSE == adjust_member_0 && (0 != aggregated_ports[i]))
                {
                    ptr_member_arr[idx++] = lag_member_0[i];
                }
                AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
                {
                    if ((PLAT_CPU_PORT == port) || !(_g_trunkMem[i].member_bmp & BIT(port - 1)))
                    {
                        continue;
                    }
                    if (0 != (block_bmp & BIT(port - 1)))
                    {
                        rc = syncd_api_stp_setPortStateByUser(port, AIR_STP_STATE_LISTEN, SYNCD_API_STP_USER_LACP);
                        if (MW_E_OK != rc)
                        {
                            MW_FREE(ptr_member_arr);
                            MW_LOG_ERROR(LACP, "LACP block port[%d] failed, rc(%d)", port, rc);
                            return rc;
                        }
                        else
                        {
                            MW_LOG_INFO(LACP, "LACP block port[%d] success", port);
                        }
                    }
                    if (0 != (unblock_bmp & BIT(port - 1)))
                    {
                        rc = syncd_api_stp_setPortStateByUser(port, AIR_STP_STATE_FORWARD, SYNCD_API_STP_USER_LACP);
                        if (MW_E_OK != rc)
                        {
                            MW_FREE(ptr_member_arr);
                            MW_LOG_ERROR(LACP, "LACP unblock port[%d] failed, rc(%d)", port, rc);
                            return rc;
                        }
                        else
                        {
                            MW_LOG_INFO(LACP, "LACP unblock port[%d] success", port);
                        }
                    }
                    if ((FALSE == adjust_member_0) && (port == lag_member_0[i]))
                    {
                        continue;
                    }
                    if (0 != (aggregated_ports[i] & BIT(port - 1)))
                    {
                        ptr_member_arr[idx++] = port;
                    }
                }

                if (0 != _g_trunkMem[i].member_bmp)
                {
                    /* Not deleting lag operation  */
                    air_rc = air_lag_setMember(unit, i, idx, ptr_member_arr);
                    if (AIR_E_OK != air_rc)
                    {
                        MW_FREE(ptr_member_arr);
                        MW_FREE(ptr_msg);
                        MW_LOG_ERROR(LACP, "Set group %d member %x fail(%d)", i + 1, aggregated_ports[i], air_rc);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
                if ((TRUE == adjust_member_0) && (ptr_member_arr[0] != lag_member_0[i]))
                {
                    MW_LOG_INFO(LACP, "LACP group[%d] member0 changed! old member0 port:%d to new member0 port:%d", i + 1, lag_member_0[i], ptr_member_arr[0]);
                    rc = syncd_api_transferPortStaticMacAddr(lag_member_0[i], ptr_member_arr[0]);
                    MW_LOG_INFO(LACP, "LACP transfer port[%d] static MAC entry to port[%d]. rc=%d", lag_member_0[i], ptr_member_arr[0], rc);
                    rc = air_l2_flushMacAddr(unit, AIR_L2_MAC_FLUSH_TYPE_PORT, lag_member_0[i]);
                    MW_LOG_INFO(LACP, "LACP flush port[%d] MAC entry. rc=%d", lag_member_0[i], rc);
                    lag_member_0[i] = ptr_member_arr[0];

                    request.t_idx = LAG_MEMBER_0_INFO;
                    request.f_idx = MEMBER_0;
                    request.e_idx = i + 1;
                    rc = dbapi_appendMsgPayload(&request, (UI8_T *)&lag_member_0[i], &ptr_msg, &msg_size, &ptr_payload);
                    if (MW_E_OK != rc)
                    {
                        MW_LOG_ERROR(LACP, "Fail to append LACP trunk port msg payload. rc=%d", rc);
                        MW_FREE(ptr_member_arr);
                        MW_FREE(ptr_msg);
                        return rc;
                    }
                }
            }
        }
    }

    if (NULL != ptr_msg)
    {
        if (ptr_msg->type.count > 0)
        {
            rc = syncd_queue_db_setMultiData(ptr_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        }
        else
        {
            MW_FREE(ptr_msg);
        }
    }

    MW_FREE(ptr_member_arr);
    return rc;
}
#endif

/* FUNCTION NAME: syncd_api_lag_algo_config
 * PURPOSE:
 *      process lag lacp function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_lag_algo_config(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    UI8_T trunk_algorithm = 0;
    AIR_LAG_HASH_CTRL_T hash_ctrl = { 0 };
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;
    UI32_T unit = 0;

    MW_CHECK_PTR(ptr_api_arg);

    osapi_memcpy((void *)&trunk_algorithm, ptr_api_arg->ptr_data, sizeof(UI8_T));
    MW_LOG_INFO(LAG, "set LAG Load Balance Algorithm: %d", trunk_algorithm);
    if (LAG_ALGORITHM_MAC_SA_DA == trunk_algorithm)
    {
        hash_ctrl.flags = (AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC | AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC);
    }
    else if (LAG_ALGORITHM_MAC_SA == trunk_algorithm)
    {
        hash_ctrl.flags = AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC;
    }
    else if (LAG_ALGORITHM_MAC_DA == trunk_algorithm)
    {
        hash_ctrl.flags = AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC;
    }

    if (0 != hash_ctrl.flags)
    {
        if (AIR_E_OK == air_lag_setHashControl(unit, &hash_ctrl))
        {
            rc = MW_E_OK;
        }
    }

    return rc;
}

/* FUNCTION NAME: syncd_api_lag_get_member0
 * PURPOSE:
 *      Get the point of the lag member0
 *
 * INPUT:
 *      Node
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The point of the lag member0
 *
 * NOTES:
 *      None
 */
UI8_T *
syncd_api_lag_get_member0(
    void)
{
    return (UI8_T *)lag_member_0;
}