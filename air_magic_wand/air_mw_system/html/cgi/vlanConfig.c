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

/* FILE NAME:  vlanConfig.c
 * PURPOSE:
 *  CGI and SSI handler for Port-based, 802.1Q VLAN and PVID webpage.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include "vlan_utils.h"
#include "osapi_memory.h"
#include "osapi_string.h"
#include "httpd_queue.h"
#include "mw_log.h"
#include "web.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define CGI_VLAN_LIMIT          (MAX_VLAN_ENTRY_NUM / 2)
#define CGI_PORT_LIMIT          (PLAT_MAX_PORT_NUM / 2)
#define DEFAULT_UPLINK_PORT_IDX (0)
#define INVALID_VID             (0)
#define INVALID_INGRESS_CHECK   (7)
#define DEFAULT_VLAN_IG_FILTER  (0)
#define MAX_VLAN_SIZE           (128)


/* MACRO FUNCTION DECLARATIONS
*/
#define HTTPD_QUEUE_DEBUG(__ret__, __method__, __tbl__, __fid__, __eid__) do                    \
    {                                                                                           \
        if (MW_E_OK ==  (__ret__))                                                              \
        {                                                                                       \
            CGI_LOG_DEBUG(VLAN, "%s[%d] httpd %u set %u.%u.%u success \n", __func__,__LINE__,  \
                            (UI16_T)(__method__), (__tbl__), (__fid__), (__eid__));             \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            CGI_LOG_ERROR(VLAN, "%s[%d] httpd %u set %u.%u.%u failed(%u) \n",__func__,__LINE__,\
                            (UI16_T)(__method__), (__tbl__), (__fid__), (__eid__), (__ret__));  \
        }                                                                                       \
    } while(0)

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/
/* FUNCTION NAME: _cgi_set_handle_vlanModeSet
 * PURPOSE:
 *      handle VLAN mode change
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vlan_mode       -- User specified VLAN mode
 *      changeVbmp      -- Affected VLAN bitmap
 *      defVidx         -- Default vlan idx in DB
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_vlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T i, idx, tid;
    BOOL_T is_changed = FALSE;
    UI32_T plat_max_bitmap = 0;
    UI32_T vlan_list = 0;

    /* cgi parameter */
    UI8_T vlan_mode = 0;
    UI16_T defVidx = 0;
    UI32_T changeVbmp = 0;
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    UI16_T mgmt_vlan = 1;
#endif

    /* db parameter */
    UI16_T size = 0;
    DB_MSG_T *ptr_msg = NULL;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;
    DB_VLAN_CFG_INFO_T vlan_cfg = {0};
    UI16_T *ptr_pvid_tbl = NULL;
    UI8_T *ptr_incheck_tbl = NULL;
    UI32_T *ptr_isolation_tbl= NULL;
    UI32_T *ptr_vlan_list_tbl = NULL;
    VLAN_ENTRY_INFO_T vlan_entry = {0};
    UI8_T *ptr_trunk = NULL;
    DB_TRUNK_PORT_T trunk_info;
    UI32_T member = 0;

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vlan_mode")){
            vlan_mode = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "changeVbmp"))
        {
            changeVbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "defVidx")){
            defVidx = atoi(pcValue[i]);
        }
    }

    CGI_LOG_DEBUG(VLAN, "%s[%d] vlan_mode: %d, changeVbmp: 0x%x, defVidx: %d\n",
                __func__, __LINE__, vlan_mode, changeVbmp, defVidx);

    /* Delete VLAN_ENTRY */
    BITMAP_VLAN_FOREACH(changeVbmp, idx)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d] vlan idx %d delete\n", __func__, __LINE__, idx);

        rc = httpd_queue_setData(M_DELETE, VLAN_ENTRY, DB_ALL_FIELDS, (idx + 1), NULL, 0);
        HTTPD_QUEUE_DEBUG(rc, M_DELETE, VLAN_ENTRY, DB_ALL_FIELDS, (idx + 1));
        if(MW_E_OK != rc)
        {
            return rc;
        }
    }

    /* Get PORT_VLAN_LIST */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    /* Update PORT_VLAN_LIST */
    SET_BIT(vlan_list, (defVidx - 1));
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if(ptr_vlan_list_tbl[i] != vlan_list)
        {
            ptr_vlan_list_tbl[i] = vlan_list;
            is_changed = TRUE;

            CGI_LOG_DEBUG(VLAN, "%s[%d] port %d, new vlan_list_tbl 0x%x \n", __func__, __LINE__, i, ptr_vlan_list_tbl[i]);
        }
    }
    if(TRUE == is_changed)
    {
        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, ptr_vlan_list_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
        if(MW_E_OK != rc)
        {
            MW_FREE(ptr_msg);
            return rc;
        }
    }
    MW_FREE(ptr_msg);

    /* Update VLAN_CFG_INFO */
    if(VLAN_PORT_ENABLE == vlan_mode) /* change to port vlan */
    {
        vlan_cfg.enable_port_b = VLAN_STATE_ENABLE;
        vlan_cfg.enable_8021q_b = VLAN_STATE_DISABLE;
        vlan_cfg.enable_mtu = VLAN_STATE_DISABLE;
    }
    else if(VLAN_1Q_ENABLE == vlan_mode) /* change to 1Q vlan */
    {
        vlan_cfg.enable_port_b = VLAN_STATE_DISABLE;
        vlan_cfg.enable_8021q_b = VLAN_STATE_ENABLE;
        vlan_cfg.enable_mtu = VLAN_STATE_DISABLE;
    }
    else if(VLAN_MTU_ENABLE == vlan_mode) /* change to MTU vlan */
    {
        vlan_cfg.enable_port_b = VLAN_STATE_DISABLE;
        vlan_cfg.enable_8021q_b = VLAN_STATE_DISABLE;
        vlan_cfg.enable_mtu = VLAN_STATE_ENABLE;
    }
    rc = httpd_queue_setData(M_UPDATE, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &vlan_cfg, sizeof(DB_VLAN_CFG_INFO_T));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    /* Reset management VLAN to default 1*/
    rc = httpd_queue_setData(M_UPDATE, SYS_INFO, SYS_MGMT_VLAN, DB_ALL_ENTRIES, &mgmt_vlan, sizeof(mgmt_vlan));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);

    if(MW_E_OK != rc)
    {
        return rc;
    }
#endif

    /* Clear all MAC address table entries when change VLAN mode. */
    rc = httpd_queue_setData(M_DELETE, STATIC_MAC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL, 0);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    rc = vlan_get_plat_max_portBmp(&plat_max_bitmap);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Create VLAN_ENTRY default VLAN */
    if(VLAN_PORT_ENABLE == vlan_mode) /* change to port vlan */
    {
        vlan_entry.vlan_id = VLAN_DEFAULT_VID;
        vlan_entry.port_member = plat_max_bitmap;
    }
    else if(VLAN_1Q_ENABLE == vlan_mode) /* change to 1Q vlan */
    {
        vlan_entry.vlan_id = VLAN_DEFAULT_VID;
        vlan_entry.untagged_member = plat_max_bitmap;
        vlan_entry.tagged_member = 0;
    }
    else if(VLAN_MTU_ENABLE == vlan_mode) /* change to MTU vlan */
    {
        vlan_entry.vlan_id = VLAN_DEFAULT_VID;
        vlan_entry.port_member = plat_max_bitmap;
    }
    rc = httpd_queue_setData(M_CREATE, VLAN_ENTRY, DB_ALL_FIELDS, defVidx, &vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
    HTTPD_QUEUE_DEBUG(rc, M_CREATE, VLAN_ENTRY, DB_ALL_FIELDS, defVidx);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    if(VLAN_PORT_ENABLE == vlan_mode)
    {
        /* Get DB VLAN_ENTRY */
        rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
        HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        /* Update VLAN_ENTRY */
        for(i = 1; i < PLAT_MAX_PORT_NUM; i++)
        {
            ptr_vlan_entry_tbl->vlan_id[i] = (i+1);
            ptr_vlan_entry_tbl->port_member[i] = 0;
        }
        rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        MW_FREE(ptr_msg);
        if(MW_E_OK != rc)
        {
            return rc;
        }
    }

    if((VLAN_1Q_ENABLE == vlan_mode) || (VLAN_PORT_ENABLE == vlan_mode))
    {
        /* Update PORT_ISOLATION */
        rc = osapi_calloc((sizeof(UI32_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_isolation_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(VLAN, "%s[%d]: alloc isolation_tbl failed(%d) \n", __func__, __LINE__, rc);
            return rc;
        }
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            ptr_isolation_tbl[i] = plat_max_bitmap;

            CGI_LOG_DEBUG(VLAN, "%s[%d]: port %d, isolation_tbl=0x%x \n", __func__, __LINE__, i+1, ptr_isolation_tbl[i]);
        }

        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, ptr_isolation_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
        MW_FREE(ptr_isolation_tbl);
        if(MW_E_OK != rc)
        {
            return rc;
        }
    }

    if(VLAN_MTU_ENABLE == vlan_mode)
    {
        /* Update PORT_ISOLATION */
        rc = osapi_calloc((sizeof(UI32_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_isolation_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(VLAN, "%s[%d]: alloc isolation_tbl failed(%d) \n", __func__, __LINE__, rc);
            return rc;
        }
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(DEFAULT_UPLINK_PORT_IDX == i)
            {
                ptr_isolation_tbl[i] = plat_max_bitmap;
            }
            else
            {
                ptr_isolation_tbl[i] = ((1 << (i+1)) | (1 << (DEFAULT_UPLINK_PORT_IDX+1)));
            }
        }
        /* If any porttrunk exists */
        rc = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_trunk);
        HTTPD_QUEUE_DEBUG(rc, M_GET, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES);
        if(MW_E_OK != rc)
        {
            return rc;
        }
        for(tid = 0; tid < MAX_TRUNK_NUM; tid++)
        {
            memcpy(&trunk_info, (void *)(ptr_trunk + (tid * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
            member = trunk_info.members.member_bmp;
            CGI_LOG_DEBUG(VLAN, "%s[%d]: ptr_trunk[%d]=0x%x \n", __func__, __LINE__, tid, member);
            if (0 != (member & 0x1))
            {
                BITMAP_PORT_FOREACH(member, i)
                {
                    ptr_isolation_tbl[i] = plat_max_bitmap;
                }
            }
        }
        MW_FREE(ptr_msg);
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            CGI_LOG_DEBUG(VLAN, "%s[%d]: port %d, isolation_tbl=0x%x \n", __func__, __LINE__, i+1, ptr_isolation_tbl[i]);
        }

        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, ptr_isolation_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
        MW_FREE(ptr_isolation_tbl);
        if(MW_E_OK != rc)
        {
            return rc;
        }
    }

    if((VLAN_MTU_ENABLE == vlan_mode) || (VLAN_PORT_ENABLE == vlan_mode))
    {
        /* Update PORT_PVID */
        rc = osapi_calloc((sizeof(UI16_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_pvid_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(VLAN, "%s[%d]: alloc pvid_tbl failed(%d) \n", __func__, __LINE__, rc);
            return rc;
        }
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            ptr_pvid_tbl[i] = VLAN_DEFAULT_VID;
            CGI_LOG_DEBUG(VLAN, "%s[%d]: port %d, pvid_tbl=0x%x \n", __func__, __LINE__, i+1, ptr_pvid_tbl[i]);
        }

        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES, ptr_pvid_tbl, (sizeof(UI16_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES);
        MW_FREE(ptr_pvid_tbl);
        if(MW_E_OK != rc)
        {
            return rc;
        }

        /* Update PORT_VLAN_IG_FILTER */
        rc = osapi_calloc((sizeof(UI8_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_incheck_tbl);
        if(MW_E_OK != rc)
        {
            CGI_LOG_ERROR(VLAN, "%s[%d]: alloc incheck_tbl failed(%d) \n", __func__, __LINE__, rc);
            return rc;
        }
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            ptr_incheck_tbl[i] = DEFAULT_VLAN_IG_FILTER;
            CGI_LOG_DEBUG(VLAN, "%s[%d]: port %d, incheck_tbl=0x%x \n", __func__, __LINE__, i+1, ptr_incheck_tbl[i]);
        }

        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES, ptr_incheck_tbl, (sizeof(UI8_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES);
        MW_FREE(ptr_incheck_tbl);
        if(MW_E_OK != rc)
        {
            return rc;
        }
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d]: ============== leave ================\n", __func__, __LINE__);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlan_matrix_rerun
 * PURPOSE:
 *      recalculate all port's matrix bitmap when mode is port-based VLAN
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlan_matrix_rerun()
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T i = 0, port = 0;
    UI32_T tmp_bmp;

    /* DB parameter */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;
    UI32_T *ptr_port_matrix_tbl = NULL;

    /* Get DB VLAN_ENTRY */
    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, newVidPbmp: 0x%x \n", __func__, __LINE__, i+1, ptr_vlan_entry_tbl->port_member[i]);
    }

    /* Allocate resource for PORT_ISOLATION */
    rc = osapi_calloc((sizeof(UI32_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_port_matrix_tbl);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: alloc port_matrix_tbl failed(%d)", __func__, __LINE__, rc);
        MW_FREE(ptr_msg);
        return rc;
    }

    /* Recalculate */
    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if((VLAN_MIN_VID == ptr_vlan_entry_tbl->vlan_id[i]) || (0 == ptr_vlan_entry_tbl->port_member[i]))
        {
            continue;
        }

        BITMAP_PORT_FOREACH(ptr_vlan_entry_tbl->port_member[i], port)
        {
            if(PLAT_CPU_PORT == port)
            {
                continue;
            }

            BITMAP_PORT_ADD(tmp_bmp, ptr_port_matrix_tbl[port-1], ptr_vlan_entry_tbl->port_member[i]);
            ptr_port_matrix_tbl[port-1] = tmp_bmp;
        }
    }
    MW_FREE(ptr_msg);

    /* debug */
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            continue;
        }
        CGI_LOG_DEBUG(VLAN, "%s[%d], port=%d, new port_matrix_tbl=0x%x \n", __func__, __LINE__, port, ptr_port_matrix_tbl[port-1]);
    }

    /* Update DB PORT_ISOLATION */
    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, ptr_port_matrix_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
    MW_FREE(ptr_port_matrix_tbl);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlan_matrix_update
 * PURPOSE:
 *      Update port matrix bitmap when mode is port-based VLAN
 * INPUT:
 *      action      -- PORT_VLAN_ACTION_ADD
 *                  -- PORT_VLAN_ACTION_DELETE
 *      vlanPbmp    -- updated port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlan_matrix_update(UI8_T action, UI32_T vlanPbmp)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i = 0;
    UI32_T tmp_bmp = 0;

    /* DB parameter */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI32_T *ptr_port_matrix_tbl = NULL;

    /* Get DB PORT_ISOLATION */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_port_matrix_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update DB PORT_ISOLATION */
    BITMAP_PORT_FOREACH(vlanPbmp, i)
    {
        if(PLAT_CPU_PORT == i)
        {
            continue;
        }

        if(PORT_VLAN_ACTION_ADD == action)
        {
            BITMAP_PORT_ADD(tmp_bmp, ptr_port_matrix_tbl[i-1], vlanPbmp);
        }
        ptr_port_matrix_tbl[i-1] = tmp_bmp;
        CGI_LOG_DEBUG(VLAN, "%s[%d], port=%d, new port_matrix_tbl=0x%x \n", __func__, __LINE__, i, ptr_port_matrix_tbl[i-1]);
    }

    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, ptr_port_matrix_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
    MW_FREE(ptr_msg);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlan_vlanlist_update
 * PURPOSE:
 *      Update port vlanlist when mode is port-based VLAN
 * INPUT:
 *      action      -- PORT_VLAN_ACTION_ADD
 *                  -- PORT_VLAN_ACTION_DELETE
 *                  -- PORT_VLAN_ACTION_ADD_DELETE
 *      vlanIdx     -- VLAN index
 *      addVlanPbmp -- added port bitmap
 *      delVlanPbmp -- deleted port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlan_vlanlist_update(UI8_T action, UI16_T vlanIdx, UI32_T addVlanPbmp, UI32_T delVlanPbmp)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i = 0;

    /* DB parameter */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;

    /* Get DB PORT_VLAN_LIST */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update PORT_VLAN_LIST */
    if((PORT_VLAN_ACTION_DELETE == action) || (PORT_VLAN_ACTION_ADD_DELETE == action))
    {
        BITMAP_PORT_FOREACH(delVlanPbmp, i)
        {
            if(PLAT_CPU_PORT == i)
            {
                continue;
            }

            CLR_BIT(ptr_vlan_list_tbl[i-1], (vlanIdx - 1));
            CGI_LOG_DEBUG(VLAN, "%s[%d], port=%d, new vlan_list_tbl=0x%x \n", __func__, __LINE__, i, ptr_vlan_list_tbl[i-1]);
        }
    }
    if((PORT_VLAN_ACTION_ADD == action) || (PORT_VLAN_ACTION_ADD_DELETE == action))
    {
        BITMAP_PORT_FOREACH(addVlanPbmp, i)
        {
            if(PLAT_CPU_PORT == i)
            {
                continue;
            }

            SET_BIT(ptr_vlan_list_tbl[i-1], (vlanIdx - 1));
            CGI_LOG_DEBUG(VLAN, "%s[%d], port=%d, new vlan_list_tbl=0x%x \n", __func__, __LINE__, i, ptr_vlan_list_tbl[i-1]);
        }
    }
    /* If port does not exist in any VLAN, restore default settings */
    for(i = 0; i <= PLAT_MAX_PORT_NUM; i++)
    {
        if(PLAT_CPU_PORT == i)
        {
            continue;
        }
        if(0 == ptr_vlan_list_tbl[i-1])
        {
            ptr_vlan_list_tbl[i-1] = VLAN_DEFAULT_VID;
        }
    }
    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, ptr_vlan_list_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    MW_FREE(ptr_msg);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlan_vlanentry_update
 * PURPOSE:
 *      Update vlan entry port member when mode is port-based VLAN
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlan_vlanentry_update()
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* cgi parameter */
    UI16_T i = 0, vid = 0;

    /* DB parameter */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI32_T port = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
    UI32_T *ptr_port_list = NULL;
    DB_VLAN_ENTRY_T *ptr_vlan_entry_tbl = NULL;

    rc = osapi_calloc((sizeof(UI32_T) * MAX_VLAN_ENTRY_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_port_list);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: alloc ptr_port_list failed(%d) \n", __func__, __LINE__, rc);
        return rc;
    }

    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: get db failed!", __func__, __LINE__);
        MW_FREE(ptr_port_list);
        return rc;
    }

    /* Recalculation VLAN_ENTRY port member*/
    for (port = 0; port < PLAT_MAX_PORT_NUM; port++)
    {
        BITMAP_VLAN_FOREACH(ptr_vlan_list_tbl[port], vid)
        {
            SET_BIT(ptr_port_list[vid], (port+1));
        }
    }
    MW_FREE(ptr_msg);

    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_entry_tbl);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: get db failed!", __func__, __LINE__);
        MW_FREE(ptr_port_list);
        return rc;
    }

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (ptr_vlan_entry_tbl->port_member[i] != ptr_port_list[i])
        {
            ptr_vlan_entry_tbl->port_member[i] = ptr_port_list[i];
        }
    }

    /* Update VLAN_ENTRY */
    rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, ptr_vlan_entry_tbl, sizeof(DB_VLAN_ENTRY_T));
    MW_FREE(ptr_msg);
    MW_FREE(ptr_port_list);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_qVlan_defvlan_member_add
 * PURPOSE:
 *      Update default vlan member when mode is 802.1q VLAN
 * INPUT:
 *      addVlanPbmp -- added port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_qVlan_defvlan_member_add(UI32_T addVlanPbmp)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    VLAN_ENTRY_INFO_T *ptr_vlan_entry = NULL;

    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, VLAN_DEFAULT_VID, &ptr_msg, &size, (void **)&ptr_vlan_entry);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: get db failed!", __func__, __LINE__);
        return rc;
    }

    ptr_vlan_entry->untagged_member |= addVlanPbmp;
    rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, VLAN_DEFAULT_VID, ptr_vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
    MW_FREE(ptr_msg);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlanSet_add
 * PURPOSE:
 *      handle add action when mode is port-based VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- added VLAN ID
 *      vidx            -- VLAN_ENTRY_TBL idx of added VLAN
 *      vidPbmp         -- added VLAN port bitmap
 *      matrixChange    -- matrix change action
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlanSet_add(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* cgi parameter */
    UI8_T matrixChange = 0;
    UI16_T i = 0, vid = 0, vidx = 0;
    UI32_T vlanPbmp = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    /* DB parameter */
    VLAN_ENTRY_INFO_T vlan_entry = {0};

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vid = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx")){
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidPbmp"))
        {
            vlanPbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "matrixChange"))
        {
            matrixChange = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vid) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&vlanPbmp,1);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d]: vid: %d, vidx: %d, vlanPbmp: 0x%x, matrixChange: %d\n",
                __func__, __LINE__, vid, vidx, vlanPbmp, matrixChange);
    /* Update VLAN_ENTRY */
    vlan_entry.vlan_id = vid;
    vlan_entry.port_member = vlanPbmp;
    rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, vidx, &vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, vidx);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update PORT_VLAN_LIST */
    rc = _cgi_set_handle_pVlan_vlanlist_update(PORT_VLAN_ACTION_ADD, vidx, vlanPbmp, 0);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update PORT_ISOLATION */
    if (PORT_VLAN_ACTION_ADD == matrixChange)
    {
        rc = _cgi_set_handle_pVlan_matrix_update(PORT_VLAN_ACTION_ADD, vlanPbmp);
    }

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlanSet_mod
 * PURPOSE:
 *      handle modify action when mode is port-based VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- modified VLAN ID
 *      vidx            -- VLAN_ENTRY_TBL idx of modified VLAN
 *      newVidPbmp      -- modified VLAN new port bitmap
 *      oldVidPbmp      -- modified VLAN old port bitmap
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlanSet_mod(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* cgi parameter */
    UI16_T i = 0, vid = 0, vidx = 0;
    UI32_T newVidPbmp = 0, oldVidPbmp = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid"))
        {
            vid = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx"))
        {
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "newVidPbmp"))
        {
            newVidPbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "oldVidPbmp"))
        {
            oldVidPbmp = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vid) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&newVidPbmp,1);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&oldVidPbmp,1);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, vidx: %d, newVidPbmp: 0x%x, oldVidPbmp: 0x%x \n",
                __func__, __LINE__, vid, vidx, newVidPbmp, oldVidPbmp);
    /* Update PORT_VLAN_LIST */
    rc = _cgi_set_handle_pVlan_vlanlist_update(PORT_VLAN_ACTION_ADD_DELETE, vidx, newVidPbmp, oldVidPbmp);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update VLAN_ENTRY */
    rc = _cgi_set_handle_pVlan_vlanentry_update();
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update PORT_ISOLATION */
    rc = _cgi_set_handle_pVlan_matrix_rerun();
    if(MW_E_OK != rc)
    {
        return rc;
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_pVlanSet_del
 * PURPOSE:
 *      handle delete action when mode is port-based VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- deleted VLAN ID
 *      vidx            -- VLAN_ENTRY_TBL idx of deleted VLAN
 *      oldVidPbmp      -- deleted VLAN port bitmap
 *      matrixChange    -- change matrix action
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_pVlanSet_del(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* cgi parameter */
    UI8_T matrixChange = 0;
    UI16_T i = 0, vid = 0, vidx = 0;
    UI32_T oldVidPbmp = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap =0;
#endif

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    UI16_T mgmt_vlan = 0;
    void *db_data = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
#endif

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vid = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx")){
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "oldVidPbmp")){
            oldVidPbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "matrixChange")){
            matrixChange = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vid) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
        CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&oldVidPbmp,1);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, vidx: %d, oldVidPbmp: 0x%x, matrixChange: %d \n",
                __func__, __LINE__, vid, vidx, oldVidPbmp, matrixChange);

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    /* get mgmt vlan  */
    rc = httpd_queue_getData(SYS_INFO, SYS_MGMT_VLAN, DB_ALL_ENTRIES, &ptr_msg, &size, &db_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s[%d] get SYS_MGMT_VLAN success, ptr_msg =%p\n", __func__, __LINE__, ptr_msg);
    }
    else
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] get SYS_MGMT_VLAN failed \n", __func__, __LINE__);
        return ERR_VAL;
    }

    memcpy(&mgmt_vlan, db_data, size);

    if(vid == mgmt_vlan)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d] Can not delete management VLAN \n", __func__, __LINE__);
        return ERR_VAL;
    }
#endif

    /* Update PORT_VLAN_LIST */
    rc = _cgi_set_handle_pVlan_vlanlist_update(PORT_VLAN_ACTION_DELETE, vidx, 0, oldVidPbmp);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update VLAN_ENTRY */
    rc = _cgi_set_handle_pVlan_vlanentry_update();
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update PORT_ISOLATION */
    if((PORT_VLAN_ACTION_DELETE == matrixChange) || (PORT_VLAN_ACTION_RERUN == matrixChange))
    {
        rc = _cgi_set_handle_pVlan_matrix_rerun();
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return rc;
}

/* FUNCTION NAME: _cgi_set_handle_qVlanSet_add
 * PURPOSE:
 *      handle add action when mode is 802.1Q VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- added VLAN ID
 *      vidx            -- the first empty idx in VLAN_ENTRY_TBL
 *      name            -- added VLAN's name
 *      untagMbrs       -- untagged port member bitmap of added VLAN
 *      tagMbrs         -- tagged port member bitmap of added VLAN
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_qVlanSet_add(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i, idx;
    UI16_T vidx = 0;
    UI32_T unionMbrs = 0, updated_vlan_list = 0;
    VLAN_ENTRY_INFO_T vlan_entry = {0};
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI32_T vlan_list_bit = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
    BOOL_T is_changed = FALSE;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
    UI32_T tempBitMap = 0;
#endif
    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vlan_entry.vlan_id = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx")){
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "name")){
            snprintf((C8_T *)vlan_entry.descr, sizeof(VLAN_DESCR_T), "%s", pcValue[i]);
        }
        if(!strcmp(pcParam[i], "untagMbrs")){
            vlan_entry.untagged_member = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "tagMbrs")){
            vlan_entry.tagged_member = atoi(pcValue[i]);
        }
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
        if(!strcmp(pcParam[i], "fid")){
            vlan_entry.vlan_fid = atoi(pcValue[i]);
        }
#endif
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vlan_entry.vlan_id) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    tempBitMap = vlan_entry.untagged_member;
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&tempBitMap,1);
    vlan_entry.untagged_member = tempBitMap;
    tempBitMap = vlan_entry.tagged_member;
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&tempBitMap,1);
    vlan_entry.tagged_member = tempBitMap;
#endif
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, vidx: %d, fid: %d, name: %s, untagMbrs: %d, tagMbrs: %d \n", __func__, __LINE__,
            vlan_entry.vlan_id, vidx, vlan_entry.vlan_fid, vlan_entry.descr, vlan_entry.untagged_member, vlan_entry.tagged_member);
#else
    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, vidx: %d, name: %s, untagMbrs: %d, tagMbrs: %d \n", __func__, __LINE__,
        vlan_entry.vlan_id, vidx, vlan_entry.descr, vlan_entry.untagged_member, vlan_entry.tagged_member);
#endif
    /* Add VLAN in VLAN_ENTRY table */
    rc = httpd_queue_setData(M_CREATE, VLAN_ENTRY, DB_ALL_FIELDS, vidx, &vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
    HTTPD_QUEUE_DEBUG(rc, M_CREATE, VLAN_ENTRY, DB_ALL_FIELDS, vidx);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Get DB, all port VLAN_LIST entry */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update affected port vlan_list */
    SET_BIT(vlan_list_bit, (vidx - 1));
    BITMAP_PORT_ADD(unionMbrs, vlan_entry.untagged_member, vlan_entry.tagged_member);
    BITMAP_PORT_FOREACH(unionMbrs, idx)
    {
        if(0 == idx)
        {
            continue;
        }
        BITMAP_VLAN_ADD(updated_vlan_list, ptr_vlan_list_tbl[idx-1], vlan_list_bit);
        ptr_vlan_list_tbl[idx-1] = updated_vlan_list;

        is_changed = TRUE;
    }

    if(TRUE == is_changed)
    {
        /* Update DB, all port VLAN_LIST entry */
        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, ptr_vlan_list_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    }
    MW_FREE(ptr_msg);

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return MW_E_OK;
}

/* FUNCTION NAME: _cgi_set_handle_qVlanSet_mod
 * PURPOSE:
 *      handle modify action when mode is 802.1Q VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- modified VLAN ID
 *      vidx            -- VLAN_ENTRY_TBL idx of modified VLAN
 *      name            -- modified VLAN name
 *      untagMbrs       -- untagged port member bitmap of modified VLAN
 *      tagMbrs         -- tagged port member bitmap of modified VLAN
 *      addPbmp         -- add port member bitmap of modified VLAN
 *      delPbmp         -- delete port member bitmap of modified VLAN
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_qVlanSet_mod(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    BOOL_T name_change = FALSE, is_changed = FALSE;
    UI16_T i, idx;
    UI16_T vid = 0, vidx = 0;
    UI32_T untagMbrs = 0, tagMbrs = 0, updated_vlan_list = 0;
    UI32_T addPbmp = 0, delPbmp = 0;
    /*db variable*/
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    VLAN_ENTRY_INFO_T *ptr_vlan_entry = NULL;
    VLAN_DESCR_T vlan_name = {0};
    UI32_T vlan_list_bit = 0, portbmp_novlan = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vid= atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx")){
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "name")){
            name_change = TRUE;
            snprintf((C8_T *)vlan_name, sizeof(VLAN_DESCR_T), "%s", pcValue[i]);
        }
        if(!strcmp(pcParam[i], "untagMbrs")){
            untagMbrs = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "tagMbrs")){
            tagMbrs = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "addPbmp")){
            addPbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "delPbmp")){
            delPbmp = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vid) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&untagMbrs,0);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&tagMbrs,0);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&addPbmp,0);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&delPbmp,0);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d]  vid: %d, vidx: %d, name: %s, untagMbrs: %d, tagMbrs: %d, addPbmp: %d, delPbmp: %d \n", __func__, __LINE__,
            vid, vidx, vlan_name, untagMbrs, tagMbrs, addPbmp, delPbmp);

    /* Get DB, all port VLAN_LIST */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    /* Update port VLAN_LIST */
    SET_BIT(vlan_list_bit, (vidx - 1));
    BITMAP_PORT_FOREACH(addPbmp, idx)
    {
        if(0 == idx)
        {
            continue;
        }
        BITMAP_VLAN_ADD(updated_vlan_list, ptr_vlan_list_tbl[idx-1], vlan_list_bit);
        if(ptr_vlan_list_tbl[idx-1] != updated_vlan_list)
        {
            ptr_vlan_list_tbl[idx-1] = updated_vlan_list;
            is_changed = TRUE;
        }
    }

    BITMAP_PORT_FOREACH(delPbmp, idx)
    {
        if(0 == idx)
        {
            continue;
        }
        BITMAP_VLAN_SUBSTRACT(updated_vlan_list, ptr_vlan_list_tbl[idx-1], vlan_list_bit);
        if(ptr_vlan_list_tbl[idx-1] != updated_vlan_list)
        {
            ptr_vlan_list_tbl[idx-1] = updated_vlan_list;
            is_changed = TRUE;
        }
    }

    /* If port does not exist in any VLAN, restore default settings */
    for(i = 0; i <= PLAT_MAX_PORT_NUM; i++)
    {
        if(PLAT_CPU_PORT == i)
        {
            continue;
        }
        if(0 == ptr_vlan_list_tbl[i-1])
        {
            SET_BIT(portbmp_novlan, i);
            ptr_vlan_list_tbl[i-1] = VLAN_DEFAULT_VID;
        }
    }

    if(TRUE == is_changed)
    {
        rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, ptr_vlan_list_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
        HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    }
    MW_FREE(ptr_msg);

    /* Update modify vlan member */
    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, vidx, &ptr_msg, &size, (void **)&ptr_vlan_entry);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] get db failed!", __func__, __LINE__);
        return rc;
    }

    ptr_vlan_entry->tagged_member = tagMbrs;
    ptr_vlan_entry->untagged_member = untagMbrs;
    if(TRUE == name_change) {
        strncpy((C8_T *)ptr_vlan_entry->descr, (C8_T *)vlan_name, sizeof(VLAN_DESCR_T));
    }
    rc = httpd_queue_setData(M_UPDATE, VLAN_ENTRY, DB_ALL_FIELDS, vidx, ptr_vlan_entry, sizeof(VLAN_ENTRY_INFO_T));
    MW_FREE(ptr_msg);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] send to db failed!", __func__, __LINE__);
        return rc;
    }

    /* Update defalut vlan member */
    rc = _cgi_set_handle_qVlan_defvlan_member_add(portbmp_novlan);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return MW_E_OK;
}

/* FUNCTION NAME: _cgi_set_handle_qVlanSet_del
 * PURPOSE:
 *      handle delete action when mode is 802.1Q VLAN
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- deleted VLAN ID
 *      vidx            -- VLAN_ENTRY_TBL idx of deleted VLAN
 *      untagMbrs       -- untagged port member bitmap of deleted VLAN
 *      tagMbrs         -- tagged port member bitmap of deleted VLAN
 * NOTES:
 *      None
 */
MW_ERROR_NO_T _cgi_set_handle_qVlanSet_del(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i, idx;
    UI16_T vid = 0, vidx = 0;
    UI32_T untagMbrs = 0, tagMbrs = 0, unionMbrs = 0, updated_vlan_list = 0;
    /*db variable*/
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI32_T vlan_list_bit = 0, portbmp_novlan = 0;
    UI32_T *ptr_vlan_list_tbl = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    UI16_T mgmt_vlan = 0;
    void *db_data = NULL;
#endif

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vid = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "vidx")){
            vidx = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "untagMbrs")){
            untagMbrs = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "tagMbrs")){
            tagMbrs = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
    if(VLAN_MIN_VID == vid) return MW_E_OK;
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&untagMbrs,1);
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&tagMbrs,1);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, vidx: %d, untagMbrs: %d, tagMbrs: %d \n", __func__, __LINE__,
            vid, vidx, untagMbrs, tagMbrs);

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    /* get mgmt vlan  */
    rc = httpd_queue_getData(SYS_INFO, SYS_MGMT_VLAN, DB_ALL_ENTRIES, &ptr_msg, &size, &db_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s[%d] get SYS_MGMT_VLAN success, ptr_msg =%p\n", __func__, __LINE__, ptr_msg);
    }
    else
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] get SYS_MGMT_VLAN failed \n", __func__, __LINE__);
        return ERR_VAL;
    }

    memcpy(&mgmt_vlan, db_data, size);

    if(vid == mgmt_vlan)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d] Can not delete management VLAN \n", __func__, __LINE__);
        return ERR_VAL;
    }
#endif

    /* Get DB, all port VLAN_LIST entry */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_vlan_list_tbl);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] get db failed!", __func__, __LINE__);
        return rc;
    }

    /* Update affected port vlan_list */
    SET_BIT(vlan_list_bit, (vidx - 1));
    BITMAP_PORT_ADD(unionMbrs, untagMbrs, tagMbrs);
    BITMAP_PORT_FOREACH(unionMbrs, idx)
    {
        if(0 == idx)
        {
            continue;
        }
        BITMAP_VLAN_SUBSTRACT(updated_vlan_list, ptr_vlan_list_tbl[idx-1], vlan_list_bit);
        ptr_vlan_list_tbl[idx-1] = updated_vlan_list;
    }

    /* If port does not exist in any VLAN, restore default settings */
    for(i = 0; i <= PLAT_MAX_PORT_NUM; i++)
    {
        if(PLAT_CPU_PORT == i)
        {
            continue;
        }
        if(0 == ptr_vlan_list_tbl[i-1])
        {
            SET_BIT(portbmp_novlan, i);
            ptr_vlan_list_tbl[i-1] = VLAN_DEFAULT_VID;
        }
    }

    /* Update DB, all port VLAN_LIST entry */
    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES, ptr_vlan_list_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_LIST, DB_ALL_ENTRIES);
    MW_FREE(ptr_msg);

    /* Delete VLAN in VLAN_ENTRY table */
    rc = httpd_queue_setData(M_DELETE, VLAN_ENTRY, DB_ALL_FIELDS, vidx, NULL, 0);
    if(MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d] send to db failed!", __func__, __LINE__);
        return rc;
    }

    /* Update defalut vlan member */
    rc = _cgi_set_handle_qVlan_defvlan_member_add(portbmp_novlan);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return MW_E_OK;
}

/* FUNCTION NAME: _ssi_get_vlan_extra_info_Handle
 * PURPOSE:
 *      handle get VLAN extra information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 * expected value:
 *      <script>
 *          var voiceinfo = {state:1,vid:3};
 *          var surveilinfo = {state:1,vid:2};
 *          var trunk_conf = {
 *              maxTrunkNum :4,
 *              info: [
 *                  {valid: 1, mbrs: [0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,] },
 *                  {valid: 0, mbrs: [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,] },
 *                  {valid: 1, mbrs: [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,] },
 *                  {valid: 0, mbrs: [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,] },
 *              ], count: 2};
 *      </script>
 */
char _ssi_get_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI16_T ety_cnt = 0, tmp_cnt = 0;
    UI16_T i = 0, j = 0, val = 0, len = 0;
    UI32_T total_len = 0;
    C8_T *ptr_tmpbuf = NULL;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
#ifdef AIR_SUPPORT_VOICE_VLAN
    DB_VOICE_VLAN_INFO_T *ptr_voiceVlan = NULL;
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    DB_SURVEI_VLAN_INFO_T *ptr_surveilVlan = NULL;
#endif
    UI8_T *ptr_trunk = NULL;
    DB_TRUNK_PORT_T trunk_info;

    UI32_T member = 0;

    err = send_format_response(&len, pcb, apiflags, "<script>\n");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_VOICE_VLAN
    /* Get DB VOICE_VLAN_INFO */
    rc = httpd_queue_getData(VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_voiceVlan);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    err = send_format_response(&len, pcb, apiflags,
        "var voiceinfo = {state:%d, vid:%d}; \n",
       ptr_voiceVlan->vlan_state,
       ptr_voiceVlan->vlan_id);

    MW_FREE(ptr_msg);
#else
    err = send_format_response(&len, pcb, apiflags,
        "var voiceinfo = {state:%d, vid:%d}; \n", 0, 0);
#endif
    if (MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    /* Get DB SURVEI_VLAN_INFO */
    rc = httpd_queue_getData(SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_surveilVlan);
    HTTPD_QUEUE_DEBUG(rc, M_GET, SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    err = send_format_response(&len, pcb, apiflags,
        "var surveilinfo = {state:%d, vid:%d}; \n",
       ptr_surveilVlan->vlan_state,
       ptr_surveilVlan->vlan_id);

    MW_FREE(ptr_msg);
#else
    err = send_format_response(&len, pcb, apiflags,
        "var surveilinfo = {state:%d, vid:%d}; \n", 0, 0);
#endif
    if (MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "var trunk_conf = {\n");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "maxTrunkNum:%d,\n", MAX_TRUNK_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "info: [\n");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB TRUNK_PORT */
    rc = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_trunk);
    HTTPD_QUEUE_DEBUG(rc, M_GET, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    rc = osapi_calloc(sizeof(C8_T) * (PLAT_MAX_PORT_NUM * 2 + 20), HTTPD_QUEUE_CLI, (void **)&ptr_tmpbuf);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: alloc ptr_tmpbuf failed(%d) \n", __func__, __LINE__, rc);
        return rc;
    }
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        len = 0; tmp_cnt = 0;
        memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
        member = trunk_info.members.member_bmp;
        for(j = 0; j < PLAT_MAX_PORT_NUM; j++)
        {
            val = (member & (0x01 << j))? 1 : 0;
            len += snprintf(ptr_tmpbuf + len, (PLAT_MAX_PORT_NUM * 2 + 20) - len, "%u,", val);

            if(val != 0)
            {
                tmp_cnt++;
            }
        }
        if(tmp_cnt != 0)
        {
            ety_cnt++;
        }

        err = send_format_response(&len, pcb, apiflags,
                "{valid:%d,mbrs:[%s]},\n",
                (tmp_cnt != 0)? 1 : 0,
                ptr_tmpbuf);
        if(MW_E_OK != err)
        {
            MW_FREE(ptr_msg);
            MW_FREE(ptr_tmpbuf);
            return err;
        }
        total_len += len;
    }
    MW_FREE(ptr_msg);
    MW_FREE(ptr_tmpbuf);

    err = send_format_response(&len, pcb, apiflags, "],count:%u};</script> \n", ety_cnt);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return err;
}


/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME: cgi_set_handle_pvlanModeSet
 * PURPOSE:
 *      port-based VLAN handle VLAN mode change
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_pvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    return _cgi_set_handle_vlanModeSet(iIndex, iNumParams, pcParam, pcValue);
}

/* FUNCTION NAME: cgi_set_handle_qvlanModeSet
 * PURPOSE:
 *      802.1Q VLAN handle VLAN mode change
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_qvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    return _cgi_set_handle_vlanModeSet(iIndex, iNumParams, pcParam, pcValue);
}

/* FUNCTION NAME: cgi_set_handle_mvlanModeSet
 * PURPOSE:
 *      MTU VLAN handle VLAN mode change
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_mvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    return _cgi_set_handle_vlanModeSet(iIndex, iNumParams, pcParam, pcValue);
}

/* FUNCTION NAME: cgi_set_handle_pVlanSet
 * PURPOSE:
 *      port-based VLAN handle action
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_pVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i;
    C8_T action[4] = {0};

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "action")){
            snprintf(action, sizeof(action), "%s", pcValue[i]);
        }
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d]  action: %s\n", __func__, __LINE__, action);

    if(strcmp(action, "add") == 0)
    {
        rc = _cgi_set_handle_pVlanSet_add(iIndex, iNumParams, pcParam, pcValue);
    }
    else if(strcmp(action, "mod") == 0)
    {
        rc = _cgi_set_handle_pVlanSet_mod(iIndex, iNumParams, pcParam, pcValue);
    }
    else if(strcmp(action, "del") == 0)
    {
        rc = _cgi_set_handle_pVlanSet_del(iIndex, iNumParams, pcParam, pcValue);
    }

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return rc;
}

/* FUNCTION NAME: cgi_set_handle_qVlanSet
 * PURPOSE:
 *      802.1Q VLAN handle action
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_qVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i;
    C8_T action[4] = {0};

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "action")){
            snprintf(action, sizeof(action), "%s", pcValue[i]);
        }
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] action: %s\n", __func__, __LINE__, action);

    if(strcmp(action, "add") == 0)
    {
        rc = _cgi_set_handle_qVlanSet_add(iIndex, iNumParams, pcParam, pcValue);
    }
    else if(strcmp(action, "mod") == 0)
    {
        rc = _cgi_set_handle_qVlanSet_mod(iIndex, iNumParams, pcParam, pcValue);
    }
    else if(strcmp(action, "del") == 0)
    {
        rc = _cgi_set_handle_qVlanSet_del(iIndex, iNumParams, pcParam, pcValue);
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);
    return rc;
}

/* FUNCTION NAME: cgi_set_handle_mVlanSet
 * PURPOSE:
 *      MTU VLAN handle action
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_mVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i = 0;
    UI32_T uplink_portbmp = 0;
    UI32_T *ptr_port_matrix_tbl = NULL;
    UI32_T port_total_mbr = 0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(0 == strcmp(pcParam[i], "portbmp"))
        {
            uplink_portbmp = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i],"trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&uplink_portbmp,1);
#endif
    CGI_LOG_DEBUG(VLAN, "%s[%d] uplink portbmp: 0x%x\n", __func__, __LINE__, uplink_portbmp);
    rc = osapi_calloc((sizeof(UI32_T) * PLAT_MAX_PORT_NUM), HTTPD_QUEUE_CLI, (void **)&ptr_port_matrix_tbl);
    vlan_get_plat_max_portBmp(&port_total_mbr);
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (0 != ((uplink_portbmp >> 1) & (1 << i)))
        {
            ptr_port_matrix_tbl[i] = port_total_mbr;
        }
        else
        {
            ptr_port_matrix_tbl[i] = uplink_portbmp;
        }
        CGI_LOG_DEBUG(VLAN, "%s[%d] ptr_port_matrix_tbl[%d]: 0x%x\n", __func__, __LINE__, i, ptr_port_matrix_tbl[i]);
    }
    /* Update DB PORT_ISOLATION */
    rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, ptr_port_matrix_tbl, (sizeof(UI32_T) * PLAT_MAX_PORT_NUM));
    HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
    MW_FREE(ptr_port_matrix_tbl);

    return rc;
}

/* FUNCTION NAME: cgi_set_handle_qvlanPvidSet
 * PURPOSE:
 *      802.1Q VLAN handle pvid and port ingress check setting
 * INPUT:
 *      iIndex          -- CGI handler index
 *      iNumParams      -- CGI parameter count
 *      pcParam         -- pointer to CGI parameter extracted from the request URI
 *      pcValue         -- pointer to CGI parameter value extracted from the request URI
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * EXPECTED CGI FORMAT:
 *      vid             -- specified PVID
 *      ingressCheck    -- port ingress filter state
 *      changePbmp      -- bitmap of specified ports
 *      changePcnt      -- count of specified ports
 * NOTES:
 *      None
 */
MW_ERROR_NO_T cgi_set_handle_qvlanPvidSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i, idx;
    UI16_T vid = 0, changePcnt = 0;
    UI8_T ingressCheck = 0;
    UI32_T changePbmp = 0;
    /*DB variable*/
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0;
    UI16_T *ptr_pvid_tbl= NULL;
    UI8_T *ptr_ingress_check_tbl = NULL;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T trunkBitMap = 0;
#endif
    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "vid")){
            vid = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "ingressCheck")){
            ingressCheck = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "changePbmp")){
            changePbmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "changePcnt")){
            changePcnt = atoi(pcValue[i]);
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
        }
#endif
    }
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&changePbmp,1);
#endif

    CGI_LOG_DEBUG(VLAN, "%s[%d] vid: %d, ingressCheck: %d, changePbmp: %d, changePcnt: %d\n", __func__, __LINE__, vid, ingressCheck, changePbmp, changePcnt);

    /* Update DB PORT_PVID */
    if(INVALID_VID != vid)
    {
        if(changePcnt >= CGI_PORT_LIMIT)
        {
            rc = httpd_queue_getData(PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_pvid_tbl);
            HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES);
            if(MW_E_OK != rc)
            {
                return rc;
            }

            BITMAP_PORT_FOREACH(changePbmp, idx)
            {
                if (0 == idx)
                {
                    continue;
                }
                ptr_pvid_tbl[idx-1] = vid;
            }
            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES, ptr_pvid_tbl, (sizeof(UI16_T) * PLAT_MAX_PORT_NUM));
            HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES);
            MW_FREE(ptr_msg);
            if(MW_E_OK != rc)
            {
                return rc;
            }
        }
        else
        {
            BITMAP_PORT_FOREACH(changePbmp, idx)
            {
                rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_PVID, idx, &vid, sizeof(UI16_T));
                HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_PVID, idx);
                if(MW_E_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    /* Update DB PORT_VLAN_MODE */
    if(INVALID_INGRESS_CHECK != ingressCheck)
    {
        if(changePcnt >= CGI_PORT_LIMIT)
        {
            rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_ingress_check_tbl);
            HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES);
            if(MW_E_OK != rc)
            {
                return rc;
            }

            BITMAP_PORT_FOREACH(changePbmp, idx)
            {
                if (0 == idx)
                {
                    continue;
                }
                ptr_ingress_check_tbl[idx-1] = ingressCheck;
            }
            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES, ptr_ingress_check_tbl, (sizeof(UI8_T) * PLAT_MAX_PORT_NUM));
            HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES);
            MW_FREE(ptr_msg);
            if(MW_E_OK != rc)
            {
                return rc;
            }
        }
        else
        {
            BITMAP_PORT_FOREACH(changePbmp, idx)
            {
                rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, idx, &ingressCheck, sizeof(UI8_T));
                HTTPD_QUEUE_DEBUG(rc, M_UPDATE, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, idx);
                if(MW_E_OK != rc)
                {
                    return rc;
                }
            }
        }
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return MW_E_OK;
}

/* FUNCTION NAME: ssi_get_port_vlan_info_Handle
 * PURPOSE:
 *      port-based VLAN get information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      expected javascript string:
 *      <script>
 *          var portMaxNum = 28;
 *          var vlanState = 1;
 *          var vlan_ds = {vlan_info:[], count:0};
 *      </script>
 */
char ssi_get_port_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI32_T total_len = 0;
    UI8_T i = 0, j = 0, vlan_state = VLAN_NONE, vlan_ety_cnt = 0;
    UI16_T len = 0;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;
    DB_VLAN_ENTRY_T *ptr_vlan_entry = NULL;
    C8_T *ptr_current_vlan_id = NULL;

    err = send_format_response(&len, pcb, apiflags,
        "<script>var portMaxNum = %d; var vlanMaxNum = %d;\n",
        PLAT_MAX_PORT_NUM,
        VLAN_CONFIG_MAX_VID);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_CFG_INFO */
    rc = httpd_queue_getData(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_cfg);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
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

    err = send_format_response(&len, pcb, apiflags, "var vlanState = %d;\n", vlan_state);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_ENTRY */
    rc = osapi_calloc(MAX_VLAN_SIZE, HTTPD_QUEUE_CLI, (void **)&ptr_current_vlan_id);
    if (MW_E_OK != rc)
    {
        return MW_E_NO_MEMORY;
    }

    rc = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_entry);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        MW_FREE(ptr_current_vlan_id);
        return rc;
    }

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d] {idx:%u,vid:%u,name:%s,mbrs:%u,tagMbrs:%u,untagMbrs:%u}\n", __func__, __LINE__,
            i+1,
            ptr_vlan_entry->vlan_id[i],
            ptr_vlan_entry->descr[i],
            ptr_vlan_entry->port_member[i],
            ptr_vlan_entry->tagged_member[i],
            ptr_vlan_entry->untagged_member[i]);

        if (0 != ptr_vlan_entry->port_member[i] ||
            0 != ptr_vlan_entry->tagged_member[i] ||
            0 != ptr_vlan_entry->untagged_member[i])
        {
            j += sprintf(ptr_current_vlan_id + j, "%u,", ptr_vlan_entry->vlan_id[i]);
        }
        else
        {
            j += sprintf(ptr_current_vlan_id + j, "%u,", 0);
        }

        total_len += j;

        if(VLAN_MIN_VID != ptr_vlan_entry->vlan_id[i])
        {
            vlan_ety_cnt++;
        }
    }
    MW_FREE(ptr_msg);

    err = send_format_response(&len, pcb, apiflags, "var currentVlanId = [%s], vlan_ds = {vlan_info:[], count:%d}; </script>\n", ptr_current_vlan_id, vlan_ety_cnt);
    MW_FREE(ptr_current_vlan_id);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return err;
}

/* FUNCTION NAME: ssi_get_port_vlan_info_Handle
 * PURPOSE:
 *      port-based VLAN get extra information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
char ssi_get_port_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    return _ssi_get_vlan_extra_info_Handle(length, pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_8021q_vlan_info_Handle
 * PURPOSE:
 *      802.1Q VLAN get information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      expected javascript string:
 *      <script>
 *          var portMaxNum = 28;
 *          var vlanMaxNum = 4094;
 *          var vlan1qMaxNum = 32;
 *          var vlanState = 1;
 *          var pvlan_ds = {vlan_info:[], count:0};
 *          var qvlan_ds = {vlan_info:[], count:0};
 *      </script>
 */
char ssi_get_8021q_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI32_T total_len = 0;
    UI8_T i = 0, vlan_state = VLAN_NONE, vlan_ety_cnt = 0;
    UI16_T len = 0;

    /* DB parameter */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;
    UI16_T *ptr_vlan_entry = NULL;

    err = send_format_response(&len, pcb, apiflags,
        "<script>var portMaxNum = %d; var vlanMaxNum = %d; var vlan1qMaxNum = %d;\n",
        PLAT_MAX_PORT_NUM,
        VLAN_CONFIG_MAX_VID,
        MAX_VLAN_ENTRY_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_CFG_INFO */
    rc = httpd_queue_getData(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_cfg);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
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

    err = send_format_response(&len, pcb, apiflags, "var vlanState = %d;\n", vlan_state);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_ENTRY */
    rc = httpd_queue_getData(VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_entry);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if(VLAN_MIN_VID != ptr_vlan_entry[i])
        {
            vlan_ety_cnt++;
        }
    }
    MW_FREE(ptr_msg);

    err = send_format_response(&len, pcb, apiflags, "var vlan_ds = {vlan_info:[], count:%d}; </script>\n", vlan_ety_cnt);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== %s: leave ================\n", __func__, __LINE__);

    return err;
}

/* FUNCTION NAME: ssi_get_8021q_vlan_pvid_info_Handle
 * PURPOSE:
 *      802.1Q VLAN get information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      expected javascript string:
 *      <script>
 *          var pvids = [1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,];
 *          var ingressChecks = [1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,];
 *      </script>
 */
char ssi_get_8021q_vlan_pvid_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI16_T i = 0, len = 0;
    UI32_T buf_len = 0, total_len = 0;
    C8_T *ptr_pvid_buf = NULL;
    C8_T *ptr_incheck_buf = NULL;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    UI16_T *ptr_port_pvid = NULL;
    UI8_T *ptr_port_incheck = NULL;

    /* Get DB PVID */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_port_pvid);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_PVID, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    rc = osapi_calloc(sizeof(C8_T) * (PLAT_MAX_PORT_NUM * 5), HTTPD_QUEUE_CLI, (void **)&ptr_pvid_buf);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: alloc ptr_pvid_buf failed(%d) \n", __func__, __LINE__, rc);
        return rc;
    }
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        buf_len += osapi_snprintf(ptr_pvid_buf + buf_len, (PLAT_MAX_PORT_NUM * 5) - buf_len, "%u,", ptr_port_pvid[i]);
    }

    err = send_format_response(&len, pcb, apiflags,
        "<script>var pvids = [%s];\n",
        ptr_pvid_buf);

    MW_FREE(ptr_msg);
    MW_FREE(ptr_pvid_buf);

    if (ERR_OK != err)
    {
        return err;
    }
    total_len += len;
    buf_len = 0;

    /* Get DB INGRESS_CHECK */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_port_incheck);
    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_VLAN_IG_FILTER, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    rc = osapi_calloc(sizeof(C8_T) * (PLAT_MAX_PORT_NUM * 2), HTTPD_QUEUE_CLI, (void **)&ptr_incheck_buf);
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(VLAN, "%s[%d]: alloc ptr_incheck_buf failed(%d) \n", __func__, __LINE__, rc);
        return rc;
    }
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        buf_len += osapi_snprintf(ptr_incheck_buf + buf_len, (PLAT_MAX_PORT_NUM * 2) - buf_len, "%u,", ptr_port_incheck[i]);
    }

    err = send_format_response(&len, pcb, apiflags, "var ingressChecks = [%s]; </script>\n", ptr_incheck_buf);
    MW_FREE(ptr_msg);
    MW_FREE(ptr_incheck_buf);
    if (MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return err;
}

/* FUNCTION NAME: ssi_get_8021q_vlan_extra_info_Handle
 * PURPOSE:
 *      802.1Q VLAN get extra information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      None
 */
char ssi_get_8021q_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    return _ssi_get_vlan_extra_info_Handle(length, pcb, apiflags);
}

/* FUNCTION NAME: ssi_get_mtu_vlan_info_Handle
 * PURPOSE:
 *      MTU VLAN get information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      expected javascript string:
 *      <script>
 *          var portMaxNum = 28;
 *          var vlanState = 3;
 *          var uplinkPort = 1;
 *          var vlan_ds = {vlan_info:[], count:0};
 *      </script>
 */
char ssi_get_mtu_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI32_T total_len = 0, port_total_mbr = 0;
    UI16_T len = 0;
    UI8_T i = 0, vlan_state = VLAN_NONE, vlan_ety_cnt = 0;
    UI32_T uplink_port = 0;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = NULL;
    UI16_T *ptr_vlan_entry = NULL;
    UI32_T *ptr_port_matrix_tbl = NULL;

    vlan_get_plat_max_portBmp(&port_total_mbr);
    err = send_format_response(&len, pcb, apiflags, "<script> var portMaxNum = %d;\n", PLAT_MAX_PORT_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_CFG_INFO */
    rc = httpd_queue_getData(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_cfg);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
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

    err = send_format_response(&len, pcb, apiflags, "var vlanState = %d;\n", vlan_state);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB PORT_ISOLATION */
    rc = httpd_queue_getData(PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_port_matrix_tbl);

    HTTPD_QUEUE_DEBUG(rc, M_GET, PORT_CFG_INFO, PORT_ISOLATION, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        CGI_LOG_DEBUG(VLAN, "%s[%d]: ptr_port_matrix_tbl[%d]=0x%x\n", __func__, __LINE__, i, ptr_port_matrix_tbl[i]);
        if(port_total_mbr == ptr_port_matrix_tbl[i])
        {
            SET_BIT(uplink_port, i+1);
        }
    }
    MW_FREE(ptr_msg);

    err = send_format_response(&len, pcb, apiflags, "var uplinkPort = %d;\n", uplink_port);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB VLAN_ENTRY */
    rc = httpd_queue_getData(VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_entry);
    HTTPD_QUEUE_DEBUG(rc, M_GET, VLAN_ENTRY, VLAN_ID, DB_ALL_ENTRIES);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    for(i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if(VLAN_MIN_VID != ptr_vlan_entry[i])
        {
            vlan_ety_cnt++;
        }
    }
    MW_FREE(ptr_msg);

    err = send_format_response(&len, pcb, apiflags, "var vlan_ds = {vlan_info:[], count:%d}; </script>\n", vlan_ety_cnt);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;
    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== leave ================\n", __func__, __LINE__);

    return err;
}

/* FUNCTION NAME: ssi_get_vlan_entry_xmlHandle
 * PURPOSE:
 *      VLAN get VLAN_ENTRY information
 * INPUT:
 *      length          -- data length would be sent
 *      pcb             -- TCP pcb
 *      apiflags        -- HTTP state
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 * NOTES:
 *      expected javascript string:
 *          example: <idx>,<vid>,<name><mbrs><tagMbrs><untagMbrs>;
 *          1,1,VLAN0001,0,0,268435455;
 *          12,3,,0,129,24576;
 */
MW_ERROR_NO_T
ssi_get_vlan_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T i = 0, max_num = MAX_VLAN_ENTRY_NUM;
    UI16_T len = 0, total_len = 0;

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    DB_VLAN_ENTRY_T *ptr_vlan_entry = NULL;

    /* Get DB VLAN_ENTRY */
    err = httpd_queue_getData(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_vlan_entry);
    HTTPD_QUEUE_DEBUG(err, M_GET, VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    if(MW_E_OK != err)
    {
        return err;
    }

#if LWIP_HTTPD_SSI_MULTIPART
    if (0 == ptr_tag_param->current_tag_part)
    {
        i = 0;
        max_num = MAX_VLAN_ENTRY_NUM / 2;
    }
    else
    {
        i = MAX_VLAN_ENTRY_NUM / 2;
        max_num = MAX_VLAN_ENTRY_NUM;
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] i:%d max_num:%d\r\n", __func__, __LINE__, i, max_num);
#endif

    for(; i < max_num; i++)
    {
        if(VLAN_MIN_VID != ptr_vlan_entry->vlan_id[i])
        {
            CGI_LOG_DEBUG(VLAN, "%s[%d] {idx:%u,vid:%u,name:%s,mbrs:%u,tagMbrs:%u,untagMbrs:%u}\n", __func__, __LINE__,
                i+1,
                ptr_vlan_entry->vlan_id[i],
                ptr_vlan_entry->descr[i],
                ptr_vlan_entry->port_member[i],
                ptr_vlan_entry->tagged_member[i],
                ptr_vlan_entry->untagged_member[i]);

            err = send_format_response(&len, (struct tcp_pcb *)ptr_tag_param, 0,
                "%u,%u,%s,%u,%u,%u;",
                i+1,
                ptr_vlan_entry->vlan_id[i],
                ptr_vlan_entry->descr[i],
                ptr_vlan_entry->port_member[i],
                ptr_vlan_entry->tagged_member[i],
                ptr_vlan_entry->untagged_member[i]);
            if (MW_E_OK != err)
            {
                MW_FREE(ptr_msg);
                return err;
            }
            total_len += len;
        }
    }
    MW_FREE(ptr_msg);

#if LWIP_HTTPD_SSI_MULTIPART
    if (0 == ptr_tag_param->current_tag_part)
    {
        ptr_tag_param->next_tag_part = 1;
    }
    else
    {
        ptr_tag_param->next_tag_part = HTTPD_LAST_TAG_PART;
    }
#endif
    ptr_tag_param->tag_insert_len = total_len;
    ptr_tag_param->chunk_enable = TRUE;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ============== %s: leave ================\n", __func__, __LINE__);

    return MW_E_OK;
}

