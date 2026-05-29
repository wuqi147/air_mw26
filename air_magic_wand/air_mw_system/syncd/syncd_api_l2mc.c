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

/* FILE NAME:  syncd_api_l2mc.c
 * PURPOSE:
 *  Implement L2MC API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <air_ipmc.h>
#include <air_swc.h>
#include <air_l2.h>
#ifdef AIR_SUPPORT_IGMP_SNP
#include "igmp_snoop.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_db.h"
#endif
#include "syncd_api_port.h"
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
*/
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
#define L2_MC_ENTRY_SIZE            (sizeof(MW_MAC_T) + sizeof(UI16_T) + sizeof(UI32_T) + sizeof(UI8_T))
#else  /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
#define L2_MC_ENTRY_SIZE            (sizeof(MW_IPV4_T) + sizeof(UI16_T) + sizeof(UI32_T) + sizeof(UI8_T))
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

#define L2_MC_ENTRY_WDOG_KICK_NUM   50

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
UI8_T _l2mc_process_cnt = 0;

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
#ifdef AIR_SUPPORT_IGMP_SNP
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
static void
_syncd_api_dynamic_mac_transform(
    AIR_MAC_ENTRY_T     *ptr_mt,
    UI8_T               *ptr_data,
    UI8_T               method);

static MW_ERROR_NO_T
_syncd_api_l2mc_addr_mac_process(
    const SYNCD_API_ARG_T *ptr_api_arg);

#else
static MW_ERROR_NO_T
_syncd_api_l2mc_addr_ipmc_process(
    const SYNCD_API_ARG_T *ptr_api_arg);

static void
_syncd_api_l2mc_transform(
    AIR_IPMC_ENTRY_T    *entry,
    UI8_T               *ptr_data);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
#endif /* AIR_SUPPORT_IGMP_SNP */

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef AIR_SUPPORT_IGMP_SNP
static UI8_T _igmp_snp_is_subscribe_l2mc_ = FALSE;
#endif

/* LOCAL SUBPROGRAM BODIES
 */
/* Transform L2MC entry format from syncd style to SDK style */

#ifdef AIR_SUPPORT_IGMP_SNP
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
/* Transform dynamic MAC entry format from syncd style to SDK style */
static void
_syncd_api_dynamic_mac_transform(
    AIR_MAC_ENTRY_T     *ptr_mt,
    UI8_T               *ptr_data,
    UI8_T               method)
{
    UI8_T                   count = 0;
    UI32_T                  vlan_mbr = 0;
    MW_PORT_BITMAP_T        add_portmbr = {0}, del_portmbr = {0};
    UI32_T                  unit = 0, port = 0;
    AIR_MAC_T               mac_addr = {0};
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    AIR_MAC_ENTRY_T         dynamic_mac_entry;
    IGMP_SNP_VLAN_CFG_T     *ptr_igmp_snp_vlan = NULL;
    IGMP_SNP_L2_MC_ENTRY_T  *ptr_igsn_grp_ety = NULL;

    if((NULL == ptr_mt) || (NULL == ptr_data))
    {
        return;
    }
    osapi_memset(&dynamic_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
    osapi_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));
    ptr_igsn_grp_ety = (IGMP_SNP_L2_MC_ENTRY_T *)ptr_data;
    osapi_memcpy(mac_addr, ptr_igsn_grp_ety->mac_addr, sizeof(MW_MAC_T));

    /* MAC */
    osapi_memcpy(ptr_mt->mac, mac_addr, sizeof(AIR_MAC_T));
    /* FID don't care */
    /* IVL */
    ptr_mt->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    /* Port bitmap*/
    osapi_memcpy(&(ptr_mt->port_bitmap[0]), &(ptr_igsn_grp_ety->portbmp), sizeof(UI32_T));
    /* sa_fwd field settings */
    ptr_mt->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
    /* age timer */
    ptr_mt->timer = IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL;
    /* VID */
    osapi_memcpy(&(ptr_mt->cvid), &(ptr_igsn_grp_ety->vid), sizeof(UI16_T));
    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    if(NULL == ptr_igmp_snp_vlan)
    {
        return;
    }
    if((IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode))
    {
        ptr_mt->cvid = IGMP_SNP_DEFAULT_VID;
    }

    /* Because multicast MAC address represents 32 distinct IP multicast address, so need check entry is not exist */
    if(IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode)
    {
        osapi_memcpy(&dynamic_mac_entry, ptr_mt, sizeof(AIR_MAC_ENTRY_T));
        rc = air_l2_getMacAddr(unit, &count, &dynamic_mac_entry);
        if (AIR_E_OK == rc)
        {
            MW_PORT_BITMAP_COPY(add_portmbr, ptr_mt->port_bitmap);
            MW_PORT_BITMAP_COPY(del_portmbr, dynamic_mac_entry.port_bitmap);
            MW_BITMAP_DEL(add_portmbr, dynamic_mac_entry.port_bitmap, MW_PORT_BITMAP_SIZE);
            MW_BITMAP_DEL(del_portmbr, ptr_mt->port_bitmap, MW_PORT_BITMAP_SIZE);
            vlan_mbr = igmp_snp_vlan_getMember(ptr_igsn_grp_ety->vid);
            switch(method)
            {
                case M_CREATE:
                    if(0 == AIR_PORT_BITMAP_EMPTY(add_portmbr))
                    {
                        MW_BITMAP_ADD(ptr_mt->port_bitmap, dynamic_mac_entry.port_bitmap, MW_PORT_BITMAP_SIZE);
                    }
                    break;

                case M_UPDATE:
                    if(0 == AIR_PORT_BITMAP_EMPTY(add_portmbr))
                    {
                        MW_BITMAP_ADD(dynamic_mac_entry.port_bitmap, add_portmbr, MW_PORT_BITMAP_SIZE);
                    }
                    if(0 == AIR_PORT_BITMAP_EMPTY(del_portmbr))
                    {
                        MW_PORT_FOREACH(del_portmbr, port)
                        {
                            if((0 != (vlan_mbr & (0x01 << port))))
                            {
                                AIR_PORT_DEL(dynamic_mac_entry.port_bitmap, port);
                            }
                        }
                    }
                    AIR_PORT_BITMAP_COPY(ptr_mt->port_bitmap, dynamic_mac_entry.port_bitmap);
                    break;

                case M_DELETE:
                    MW_BITMAP_DEL(dynamic_mac_entry.port_bitmap, ptr_mt->port_bitmap, MW_PORT_BITMAP_SIZE);
                    AIR_PORT_BITMAP_COPY(ptr_mt->port_bitmap, dynamic_mac_entry.port_bitmap);
                    ptr_mt->timer = dynamic_mac_entry.timer;
                    break;
                default:
                    break;
            }
        }
        else
        {
            /* Entry not found */
            if(M_DELETE == method)
            {
                MW_PORT_BITMAP_CLEAR(ptr_mt->port_bitmap);
            }
        }
    }
    else
    {
        if(M_DELETE == method)
        {
            /* Port bitmap */
            MW_PORT_BITMAP_CLEAR(ptr_mt->port_bitmap);
        }
    }

    return;
}

static MW_ERROR_NO_T
_syncd_api_l2mc_addr_mac_process(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    UI8_T                   *ptr_data = NULL;
    UI32_T                  unit = 0;
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    AIR_MAC_ENTRY_T         dMacEty;
    IGMP_SNP_L2_MC_ENTRY_T *ptr_igsn_grp_ety = NULL;

    MW_CHECK_PTR(ptr_api_arg);
    osapi_memset(&dMacEty, 0, sizeof(AIR_MAC_ENTRY_T));
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    ptr_igsn_grp_ety = (IGMP_SNP_L2_MC_ENTRY_T *)ptr_data;
    if (M_CREATE == ptr_api_arg->method)
    {
        /* Create a dynamic mac entry */
        _syncd_api_dynamic_mac_transform(&dMacEty, ptr_data, M_CREATE);
        rc = air_l2_addMacAddr(unit, &dMacEty);
        if (AIR_E_OK != rc)
        {
            /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
            if(FALSE == _igmp_snp_is_subscribe_l2mc_)
            {
                SYNCD_LOG_DEBUG("M_SUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                _igmp_snp_is_subscribe_l2mc_ = TRUE;
            }
            syncd_queue_db_send(NULL,
                                M_DELETE,
                                L2_MC_ENTRY,
                                DB_ALL_FIELDS,
                                ptr_api_arg->ptr_type->e_idx,
                                NULL,
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
            SYNCD_LOG_ERROR("Create MAC entry failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }
        /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
        if(TRUE == _igmp_snp_is_subscribe_l2mc_)
        {
            SYNCD_LOG_DEBUG("M_UNSUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
            igmp_snp_db_queue_sendWithRsp(M_UNSUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
            _igmp_snp_is_subscribe_l2mc_ = FALSE;
        }
    }
    else if(M_UPDATE == ptr_api_arg->method)
    {
        if(TRUE == ptr_igsn_grp_ety->update_flag)
        {
            /* Update a dynamic mac entry */
            _syncd_api_dynamic_mac_transform(&dMacEty, ptr_data, M_UPDATE);
            rc = air_l2_addMacAddr(unit, &dMacEty);
            if (AIR_E_OK != rc)
            {
                if(FALSE == _igmp_snp_is_subscribe_l2mc_)
                {
                    SYNCD_LOG_DEBUG("M_SUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                    _igmp_snp_is_subscribe_l2mc_ = TRUE;
                }
                syncd_queue_db_send(NULL,
                                    M_DELETE,
                                    L2_MC_ENTRY,
                                    DB_ALL_FIELDS,
                                    ptr_api_arg->ptr_type->e_idx,
                                    NULL,
                                    MSG_TIMEOUT_WAIT_INDEFINITELY);
                SYNCD_LOG_ERROR("Update MAC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
            /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
            if(TRUE == _igmp_snp_is_subscribe_l2mc_)
            {
                SYNCD_LOG_DEBUG("M_UNSUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                igmp_snp_db_queue_sendWithRsp(M_UNSUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                _igmp_snp_is_subscribe_l2mc_ = FALSE;
            }
        }
    }
    else if (M_DELETE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx)
        {
            if (MW_E_OK != syncd_api_flushPortBmpMacAddr(unit, PLAT_PORT_BMP_TOTAL))
            {
                SYNCD_LOG_ERROR("Clear MAC entry failed");
                return MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            /* Delete a specific dynamic mac entry */
            _syncd_api_dynamic_mac_transform(&dMacEty, ptr_data, M_DELETE);
            if(0 == dMacEty.port_bitmap[0])
            {
                rc = air_l2_delMacAddr(unit, &dMacEty);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Delete MAC entry failed(%d)", rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
            else
            {
                /* Update a specific dynamic mac entry */
                rc = air_l2_addMacAddr(unit, &dMacEty);
                if (AIR_E_OK != rc)
                {
                    if(FALSE == _igmp_snp_is_subscribe_l2mc_)
                    {
                        SYNCD_LOG_DEBUG("M_SUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                        igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                        _igmp_snp_is_subscribe_l2mc_ = TRUE;
                    }
                    syncd_queue_db_send(NULL,
                                        M_DELETE,
                                        L2_MC_ENTRY,
                                        DB_ALL_FIELDS,
                                        ptr_api_arg->ptr_type->e_idx,
                                        NULL,
                                        MSG_TIMEOUT_WAIT_INDEFINITELY);
                    SYNCD_LOG_ERROR("Update MAC entry failed(%d)", rc);
                    return MW_E_OP_INCOMPLETE;
                }
                /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
                if(TRUE == _igmp_snp_is_subscribe_l2mc_)
                {
                    SYNCD_LOG_DEBUG("M_UNSUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                    igmp_snp_db_queue_sendWithRsp(M_UNSUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                    _igmp_snp_is_subscribe_l2mc_ = FALSE;
                }
            }
        }
    }
    return MW_E_OK;
}

#else  /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
static void
_syncd_api_l2mc_transform(
    AIR_IPMC_ENTRY_T    *entry,
    UI8_T               *ptr_data)
{
    MW_IPV4_T               ip_addr = 0;
    IGMP_SNP_VLAN_CFG_T     *ptr_igmp_snp_vlan = NULL;

    memset(entry, 0, sizeof(AIR_IPMC_ENTRY_T));

    entry->type = AIR_IPMC_TYPE_GRP;

    /* Mcast IP */
    entry->group_addr.ipv4 = TRUE;
    osapi_memcpy(&ip_addr, ptr_data, sizeof(MW_IPV4_T));
    entry->group_addr.ip_addr.ipv4_addr = ip_addr;
    ptr_data +=  sizeof(MW_IPV4_T);

    /* VID */
    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    if((NULL != ptr_igmp_snp_vlan) && ((IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode)))
    {
        entry->vid = IGMP_SNP_DEFAULT_VID;
    }
    else
    {
        memcpy(&entry->vid, ptr_data, sizeof(UI16_T));
    }
    ptr_data += sizeof(UI16_T);

    /* Port BitMap */
    /* The portmap in ptr_data from DB is user port. */
    memcpy(&entry->port_bitmap[0], ptr_data, sizeof(UI32_T));
    SYNCD_LOG_DEBUG("mcstinfo->group_addr=0x%x, vid=%d, portmap=0x%x",
                entry->group_addr.ip_addr.ipv4_addr, entry->vid, entry->port_bitmap[0]);
    return;
}

static MW_ERROR_NO_T
_syncd_api_l2mc_addr_ipmc_process(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI8_T *ptr_data;
    AIR_IPMC_ENTRY_T entry;
    IGMP_SNP_L2_MC_ENTRY_T *ptr_igsn_grp_ety = NULL;
    IGMP_SNP_VLAN_CFG_T    *ptr_igmp_snp_vlan = NULL;
    UI32_T unit = 0;

    MW_CHECK_PTR(ptr_api_arg);
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    ptr_igsn_grp_ety = (IGMP_SNP_L2_MC_ENTRY_T *)ptr_data;
    ptr_igmp_snp_vlan = igmp_snp_vlan_getConfig();
    MW_CHECK_PTR(ptr_igmp_snp_vlan);
    if (M_CREATE == ptr_api_arg->method)
    {
        /* Create/Modify a l2mc entry */
        _syncd_api_l2mc_transform(&entry, ptr_data);

        if(IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode)
        {
            AIR_IPMC_ENTRY_T ipmc_entry = {0};

            osapi_memcpy(&ipmc_entry, &entry, sizeof(AIR_IPMC_ENTRY_T));
            rc = air_ipmc_getMcastAddr(unit, &ipmc_entry);
            if(AIR_E_OK == rc)
            {
                rc = air_ipmc_addMcastMember(unit, &entry);
            }
            else if(AIR_E_ENTRY_NOT_FOUND == rc)
            {
                rc = air_ipmc_addMcastAddr(unit, &entry);
            }

        }
        else
        {
            rc = air_ipmc_addMcastAddr(unit, &entry);
        }

        if (AIR_E_OK != rc)
        {
            /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
            if(FALSE == _igmp_snp_is_subscribe_l2mc_)
            {
                SYNCD_LOG_DEBUG("M_SUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                _igmp_snp_is_subscribe_l2mc_ = TRUE;
            }
            syncd_queue_db_send(NULL,
                                M_DELETE,
                                L2_MC_ENTRY,
                                DB_ALL_FIELDS,
                                ptr_api_arg->ptr_type->e_idx,
                                NULL,
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
            SYNCD_LOG_ERROR("Create L2MC entry failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        if(TRUE == ptr_igsn_grp_ety->update_flag)
        {
            AIR_IPMC_ENTRY_T ipmc_entry = {0};
            MW_PORT_BITMAP_T  add_portmbr = {0}, del_portmbr = {0};

            _syncd_api_l2mc_transform(&entry, ptr_data);
            osapi_memcpy(&ipmc_entry, &entry, sizeof(AIR_IPMC_ENTRY_T));
            rc = air_ipmc_getMcastAddr(unit, &ipmc_entry);
            if(AIR_E_OK == rc)
            {
                MW_PORT_BITMAP_COPY(add_portmbr, entry.port_bitmap);
                MW_PORT_BITMAP_COPY(del_portmbr, ipmc_entry.port_bitmap);
                MW_BITMAP_DEL(add_portmbr, ipmc_entry.port_bitmap, MW_PORT_BITMAP_SIZE);
                MW_BITMAP_DEL(del_portmbr, entry.port_bitmap, MW_PORT_BITMAP_SIZE);
                if(0 == AIR_PORT_BITMAP_EMPTY(add_portmbr))
                {
                    MW_PORT_BITMAP_COPY(ipmc_entry.port_bitmap, add_portmbr);
                    rc = air_ipmc_addMcastMember(unit, &ipmc_entry);
                }
                if((AIR_E_OK == rc) && (0 == AIR_PORT_BITMAP_EMPTY(del_portmbr)))
                {
                    ipmc_entry.port_bitmap[0] = del_portmbr[0] & (igmp_snp_vlan_getMember(ptr_igsn_grp_ety->vid));
                    if(0 == AIR_PORT_BITMAP_EMPTY(ipmc_entry.port_bitmap))
                    {
                        rc = air_ipmc_delMcastMember(unit, &ipmc_entry);
                    }
                }
            }
            else if(AIR_E_ENTRY_NOT_FOUND == rc)
            {
                rc = air_ipmc_addMcastAddr(unit, &entry);
            }

            if (AIR_E_OK != rc)
            {
                /* ENBTP5104-14 && ENBTP5104-7, to reduce the burden of the IGMP_SNP task */
                if(FALSE == _igmp_snp_is_subscribe_l2mc_)
                {
                    SYNCD_LOG_DEBUG("M_SUBSCRIBE, L2_MC_ENTRY:DB_ALL_FIELDS");
                    igmp_snp_db_queue_sendWithRsp(M_SUBSCRIBE, L2_MC_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL);
                    _igmp_snp_is_subscribe_l2mc_ = TRUE;
                }
                syncd_queue_db_send(NULL,
                                    M_DELETE,
                                    L2_MC_ENTRY,
                                    DB_ALL_FIELDS,
                                    ptr_api_arg->ptr_type->e_idx,
                                    NULL,
                                    MSG_TIMEOUT_WAIT_INDEFINITELY);
                SYNCD_LOG_ERROR("Create L2MC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else if (M_DELETE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx)
        {
            /* DB_ALL_ENTRIES means to clear all l2mc entry */
            rc = air_ipmc_delAllMcastAddr(unit);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Clear L2MC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            _syncd_api_l2mc_transform(&entry, ptr_data);
            if(IGMP_SNP_VLAN_PORT == ptr_igmp_snp_vlan->vlan_mode)
            {
                AIR_IPMC_ENTRY_T ipmc_entry = {0};

                osapi_memcpy(&ipmc_entry, &entry, sizeof(AIR_IPMC_ENTRY_T));
                rc = air_ipmc_getMcastAddr(unit, &ipmc_entry);
                if(AIR_E_OK == rc)
                {
                    MW_BITMAP_DEL(ipmc_entry.port_bitmap, entry.port_bitmap, MW_PORT_BITMAP_SIZE);
                    if(0 == AIR_PORT_BITMAP_EMPTY(ipmc_entry.port_bitmap))
                    {
                        rc = air_ipmc_delMcastMember(unit, &entry);
                    }
                    else
                    {
                        rc = air_ipmc_delMcastAddr(unit, &entry);
                    }
                }
            }
            else
            {
                rc = air_ipmc_delMcastAddr(unit, &entry);
            }

            if((AIR_E_OK != rc) && (AIR_E_ENTRY_NOT_FOUND != rc))
            {
                SYNCD_LOG_ERROR("Delete L2MC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    return MW_E_OK;
}
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
#endif /* AIR_SUPPORT_IGMP_SNP */

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_igmp_snp_enable
 * PURPOSE:
 *      process IGMP SNP entry request from DB.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_igmp_snp_enable(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
#ifndef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    UI8_T *ptr_data;
    UI32_T unit = 0;
    UI32_T port = 0;

    /* igmp search IPMC table */
    MW_CHECK_PTR(ptr_api_arg);
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    MW_CHECK_PTR(ptr_data);

    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d, *ptr_data=%d",
                ptr_api_arg->method, ptr_api_arg->data_size, *ptr_data);
    if (*ptr_data)
    {
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            air_ipmc_setPortIpmcMode(unit, port, ENABLE);
        }
    }
    else
    {
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            air_ipmc_setPortIpmcMode(unit, port, DISABLE);
        }
    }
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    return MW_E_OK;
}


/* FUNCTION NAME: syncd_api_l2mc_addr
 * PURPOSE:
 *      process L2MC entry request from DB.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_l2mc_addr(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
#ifdef AIR_SUPPORT_IGMP_SNP
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_CHECK_PTR(ptr_api_arg);
    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d",
                ptr_api_arg->method, ptr_api_arg->data_size);
    if ((DB_ALL_ENTRIES != ptr_api_arg->ptr_type->e_idx) &&
        (L2_MC_ENTRY_SIZE != ptr_api_arg->data_size))
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if ((M_GET != ptr_api_arg->method) &&
        (M_CREATE != ptr_api_arg->method) &&
        (M_DELETE != ptr_api_arg->method) &&
        (M_UPDATE != ptr_api_arg->method))
    {
        SYNCD_LOG_ERROR("method=%d, return MW_E_BAD_PARAMETER", ptr_api_arg->method);
        return MW_E_BAD_PARAMETER;
    }

#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    rc = _syncd_api_l2mc_addr_mac_process(ptr_api_arg);
#else
    rc = _syncd_api_l2mc_addr_ipmc_process(ptr_api_arg);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    if(MW_E_OK != rc)
    {
        return MW_E_OP_INCOMPLETE;
    }

    if (L2_MC_ENTRY_WDOG_KICK_NUM == ++_l2mc_process_cnt)
    {
        osal_wdog_kick();
        _l2mc_process_cnt = 0;
    }
#endif /* AIR_SUPPORT_IGMP_SNP */

    return MW_E_OK;
}

