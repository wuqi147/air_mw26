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

/* FILE NAME:  syncd_api_l2.c
 * PURPOSE:
 *  Implement L2 API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include "osapi_message.h"
#include <syncd_in.h>
#include <air_l2.h>
#include <air_port.h>
#include <air_vlan.h>
#include <syncd_api_l2.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_bitmap.h>
#include "vlan_utils.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define STATIC_MAC_ENTRY_SIZE   (sizeof(AIR_MAC_T) + sizeof(UI16_T) + sizeof(UI16_T))

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
/* Transform MAC entry format from syncd style to SDK style */
static void
_syncd_api_l2_transform(
    AIR_MAC_ENTRY_T *ptr_mt,
    AIR_MAC_T *ptr_mac_addr,
    const UI16_T vid,
    const UI16_T port)
{
    memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));

    /* MAC */
    memcpy(ptr_mt ->mac, ptr_mac_addr, sizeof(AIR_MAC_T));
    /* VID */
    ptr_mt ->cvid = vid;
    /* FID don't care */
    /* IVL */
    ptr_mt ->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;

    /* Port */
    AIR_PORT_ADD(ptr_mt ->port_bitmap, port);
    /* Status */
    ptr_mt ->flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;
}

static MW_ERROR_NO_T
_syncd_api_dynamic_mac_address_entry_action_get_start(
    const SYNCD_API_ARG_T *ptr_api_arg);

static MW_ERROR_NO_T
_syncd_api_dynamic_mac_address_entry_action_get_continue(
    const SYNCD_API_ARG_T *ptr_api_arg);

static MW_ERROR_NO_T
_syncd_api_update_db_dynamic_mac_address_entry(
    AIR_MAC_ENTRY_T     *ptr_mt,
    UI16_T              idx,
    UI8_T              *ptr_count);

/* STATIC VARIABLE DECLARATIONS
 */
static DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T _syncd_dynamic_mac_address_entry_cache_table[SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE];
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
static ONE_DB_STATIC_MAC_ENTRY_T _syncd_static_mac_entry_cache_table[MAX_STATIC_MAC_NUM];
#endif

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _syncd_api_dynamic_mac_address_entry_action_get_start
 * PURPOSE:
 *      Process syncd dynamic configuration info.
 *
 * INPUT:
 *      ptr_api_arg              -- porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_syncd_api_dynamic_mac_address_entry_action_get_start(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T dynamic_mac_address_entrycfg = {0};

    UI32_T unit = 0;
    AIR_MAC_ENTRY_T     *ptr_mt;
    UI8_T               count;
    UI32_T              bucket_size;

    MW_CHECK_PTR(ptr_api_arg);
    memset(_syncd_dynamic_mac_address_entry_cache_table, 0, sizeof(DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T) * SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE);
    rc = air_l2_getMacBucketSize(unit, &bucket_size);
    if (AIR_E_OK != rc)
    {
        SYNCD_LOG_ERROR("***Error***, get max. set number fail\n");
        return MW_E_OP_INCOMPLETE;
    }

    ptr_mt = osal_alloc(sizeof(AIR_MAC_ENTRY_T) * bucket_size, "syncd");
    if (NULL == ptr_mt)
    {
        SYNCD_LOG_ERROR("***Error***, allocate memory fail\n");
        return MW_E_NO_MEMORY;
    }
    osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);

    /* Get 1st MAC entry */
    rc = air_l2_getMacAddr(unit, &count, ptr_mt);
    if (AIR_E_OK != rc)
    {
        osal_free(ptr_mt);
        dynamic_mac_address_entrycfg.action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END;
        dynamic_mac_address_entrycfg.dynamic_entry_count = 0;
        mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                    M_UPDATE,
                                    DYNAMIC_MAC_ADDRESS_ENTRY_CFG,
                                    DB_ALL_FIELDS,
                                    DB_ALL_ENTRIES,
                                    &dynamic_mac_address_entrycfg,
                                    MSG_TIMEOUT_WAIT_INDEFINITELY);
        return MW_E_ENTRY_NOT_FOUND;
    }

    mw_rc = _syncd_api_update_db_dynamic_mac_address_entry(ptr_mt, (dynamic_mac_address_entrycfg.dynamic_entry_count + 1), &count);
    if (MW_E_OK != mw_rc)
    {
       SYNCD_LOG_ERROR("***Error***, update dynamic entry to DB fail\n");
       osal_free(ptr_mt);
       return mw_rc;
    }
    dynamic_mac_address_entrycfg.dynamic_entry_count += count;

    while(1)
    {
        /* Get the other MAC entries */
        osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
        rc = air_l2_getNextMacAddr(unit, &count, ptr_mt);
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            break;
        }
        else if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("get MAC table fail\n");
            break;
        }
        mw_rc = _syncd_api_update_db_dynamic_mac_address_entry(ptr_mt, (dynamic_mac_address_entrycfg.dynamic_entry_count + 1), &count);
        if (MW_E_OK != mw_rc)
        {
           SYNCD_LOG_ERROR("***Error***, update dynamic entry to DB fail\n");
           osal_free(ptr_mt);
           return mw_rc;
        }
        dynamic_mac_address_entrycfg.dynamic_entry_count += count;
        if(PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM <= (dynamic_mac_address_entrycfg.dynamic_entry_count))
        {
            dynamic_mac_address_entrycfg.action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_DONE;
            dynamic_mac_address_entrycfg.dynamic_entry_count = PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM;
            mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                        M_UPDATE,
                                        DYNAMIC_MAC_ADDRESS_ENTRY_CFG,
                                        DB_ALL_FIELDS,
                                        DB_ALL_ENTRIES,
                                        &dynamic_mac_address_entrycfg,
                                        MSG_TIMEOUT_WAIT_INDEFINITELY);
            break;
        }
    }
    if (AIR_E_OK != rc)
    {
        dynamic_mac_address_entrycfg.action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END;
        mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                    M_UPDATE,
                                    DYNAMIC_MAC_ADDRESS_ENTRY_CFG,
                                    DB_ALL_FIELDS,
                                    DB_ALL_ENTRIES,
                                    &dynamic_mac_address_entrycfg,
                                    MSG_TIMEOUT_WAIT_INDEFINITELY);
    }
    osal_free(ptr_mt);
    return mw_rc;
}

/* FUNCTION NAME: _syncd_api_dynamic_mac_address_entry_action_get_continue
 * PURPOSE:
 *      Process syncd dynamic configuration info.
 *
 * INPUT:
 *      ptr_api_arg              -- porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_syncd_api_dynamic_mac_address_entry_action_get_continue(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T dynamic_mac_address_entrycfg = {0};

    UI32_T unit = 0;
    AIR_MAC_ENTRY_T     *ptr_mt;
    UI8_T               count;
    UI32_T              bucket_size;

    UI8_T i = 0;
    MW_CHECK_PTR(ptr_api_arg);

    for(i = 0; i < SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE; i++)
    {
        if(0 != _syncd_dynamic_mac_address_entry_cache_table[i].vid)
        {
            syncd_queue_db_send(NULL,
                                M_UPDATE,
                                DYNAMIC_MAC_ADDRESS_ENTRY,
                                DB_ALL_FIELDS,
                                (i + 1),
                                &(_syncd_dynamic_mac_address_entry_cache_table[i]),
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
        }
        else
        {
            break;
        }
    }
    dynamic_mac_address_entrycfg.dynamic_entry_count += i;
    memset(_syncd_dynamic_mac_address_entry_cache_table, 0, sizeof(DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T) * SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE);
    rc = air_l2_getMacBucketSize(unit, &bucket_size);
    if (AIR_E_OK != rc)
    {
        SYNCD_LOG_ERROR("***Error***, get max. set number fail\n");
        return MW_E_OP_INCOMPLETE;
    }

    ptr_mt = osal_alloc(sizeof(AIR_MAC_ENTRY_T) * bucket_size, "syncd");
    if (NULL == ptr_mt)
    {
        SYNCD_LOG_ERROR("***Error***, allocate memory fail\n");
        return MW_E_NO_MEMORY;
    }
    osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);

    while(1)
    {
        /* Get the other MAC entries */
        osal_memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
        rc = air_l2_getNextMacAddr(unit, &count, ptr_mt);
        if (AIR_E_ENTRY_NOT_FOUND == rc)
        {
            break;
        }
        else if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("get MAC table fail\n");
            break;
        }
        mw_rc = _syncd_api_update_db_dynamic_mac_address_entry(ptr_mt, (dynamic_mac_address_entrycfg.dynamic_entry_count + 1), &count);
        if (MW_E_OK != mw_rc)
        {
           SYNCD_LOG_ERROR("***Error***, update dynamic entry to DB fail\n");
           osal_free(ptr_mt);
           return mw_rc;
        }
        dynamic_mac_address_entrycfg.dynamic_entry_count += count;
        if(PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM <= (dynamic_mac_address_entrycfg.dynamic_entry_count))
        {
            dynamic_mac_address_entrycfg.action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_DONE;
            dynamic_mac_address_entrycfg.dynamic_entry_count = PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM;
            mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                        M_UPDATE,
                                        DYNAMIC_MAC_ADDRESS_ENTRY_CFG,
                                        DB_ALL_FIELDS,
                                        DB_ALL_ENTRIES,
                                        &dynamic_mac_address_entrycfg,
                                        MSG_TIMEOUT_WAIT_INDEFINITELY);
            break;
        }
    }
    if (AIR_E_OK != rc)
    {
        dynamic_mac_address_entrycfg.action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_END;
        mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                    M_UPDATE,
                                    DYNAMIC_MAC_ADDRESS_ENTRY_CFG,
                                    DB_ALL_FIELDS,
                                    DB_ALL_ENTRIES,
                                    &dynamic_mac_address_entrycfg,
                                    MSG_TIMEOUT_WAIT_INDEFINITELY);
    }
    osal_free(ptr_mt);
    return mw_rc;
}

/* FUNCTION NAME: _syncd_api_update_db_dynamic_mac_address_entry
 * PURPOSE:
 *      Update dynamic mac entry to DB.
 *
 * INPUT:
 *      ptr_mac_entry            -- Structure of MAC Address table for searching result.
 *      idx                      -- DYNAMIC_MAC_ADDRESS_ENTRY_TBL idx of update dynamic mac entry.
 *
 * OUTPUT:
 *      ptr_count                -- Number of dynamic entry that updated to the DB.
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
static MW_ERROR_NO_T
_syncd_api_update_db_dynamic_mac_address_entry(
    AIR_MAC_ENTRY_T     *ptr_mt,
    UI16_T              idx,
    UI8_T              *ptr_count)
{
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI16_T i = 0;
    UI16_T offset = 0;
    UI8_T cache_idx = 0;
    DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T *ptr_dynamic_mac_address_entry = NULL;

    MW_CHECK_PTR(ptr_mt);
    MW_CHECK_PTR(ptr_count);
    UNUSED(cache_idx);
    mw_rc = osapi_calloc(sizeof(DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T), "syncd", (void**)&ptr_dynamic_mac_address_entry);
    if(MW_E_OK != mw_rc)
    {
        return mw_rc;
    }
    for(i = 0; i < *ptr_count; i++)
    {
        if(((ptr_mt[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_STATIC) || (0 == (ptr_mt[i].port_bitmap)[0]))
        {
            continue;
        }
        if(0 == ((ptr_mt[i].flags) & AIR_L2_MAC_ENTRY_FLAGS_IVL))
        {
            /* Not support display SVL entry in webpage */
            continue;
        }
        osapi_memcpy(ptr_dynamic_mac_address_entry->mac_addr, ptr_mt[i].mac, sizeof(MW_MAC_T));
        ptr_dynamic_mac_address_entry->vid = ptr_mt[i].cvid;
        ptr_dynamic_mac_address_entry->port= (ptr_mt[i].port_bitmap)[0];
        ptr_dynamic_mac_address_entry->age = ptr_mt[i].timer;
        if(PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM >= (idx + offset))
        {
            syncd_queue_db_send(NULL,
                                M_UPDATE,
                                DYNAMIC_MAC_ADDRESS_ENTRY,
                                DB_ALL_FIELDS,
                                (idx + offset),
                                ptr_dynamic_mac_address_entry,
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
            offset++;
        }
        else
        {
            if(SYNCD_DYNAMIC_MAC_ADDRESS_ENTRY_CACHE_TABLE_SIZE <= cache_idx)
            {
                osapi_free(ptr_dynamic_mac_address_entry);
                return MW_E_BAD_PARAMETER;
            }
            memcpy(&_syncd_dynamic_mac_address_entry_cache_table[cache_idx], ptr_dynamic_mac_address_entry, sizeof(DYNAMIC_MAC_ADDRESS_ENTRY_INFO_T));
            cache_idx ++;
        }
    }
    osapi_free(ptr_dynamic_mac_address_entry);
    *ptr_count = offset;
    return mw_rc;
}

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_l2_static_mac
 * PURPOSE:
 *      process L2 static MAC entry request from DB.
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
syncd_api_l2_static_mac(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    AIR_MAC_ENTRY_T mt;
    UI32_T unit = 0;
    UI16_T ety_idx;
    UI16_T expect_entry_num;
    UI8_T *ptr_data = NULL;
    AIR_MAC_T *ptr_mac_addr = NULL;
    UI8_T *ptr_vid = NULL;
    UI16_T vid;
    UI8_T *ptr_port = NULL;
    UI32_T port;

    MW_CHECK_PTR(ptr_api_arg);
    ety_idx = ptr_api_arg ->ptr_type ->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == ety_idx)?MAX_STATIC_MAC_NUM:1;
    SYNCD_LOG_DEBUG("expect_entry_num=%u", expect_entry_num);
    if ((STATIC_MAC_ENTRY_SIZE * expect_entry_num) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_data = (UI8_T *)ptr_api_arg ->ptr_data;
    ptr_mac_addr = (AIR_MAC_T *)ptr_data;
    ptr_vid = ptr_data + (sizeof(AIR_MAC_T) * expect_entry_num);
    ptr_port = ptr_vid + (sizeof(UI16_T) * expect_entry_num);
    vid = ptr_vid[0] | (ptr_vid[1] << 8);
    port = ptr_port[0] | ((UI32_T)ptr_port[1] << 8);

    SYNCD_LOG_DEBUG("data_size=%u", ptr_api_arg ->data_size);
    SYNCD_LOG_DEBUG("ptr_data=%p", ptr_data);
    SYNCD_LOG_DEBUG("ptr_mac_addr=%p", &(ptr_mac_addr[0]));
    SYNCD_LOG_DEBUG("ptr_vid=%p, vid=%u", ptr_vid, vid);
    SYNCD_LOG_DEBUG("ptr_port=%p, port=%u", ptr_port, port);

    if (M_GET == ptr_api_arg ->method)
    {
        /* Receive M_GET from DB means the 1st notification */
        if (DB_ALL_ENTRIES != ety_idx)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of 1st notification is wrong", ety_idx);
            return MW_E_BAD_PARAMETER;
        }
        for (ety_idx = 0; ety_idx < MAX_STATIC_MAC_NUM; ety_idx++)
        {
            SYNCD_LOG_DEBUG("ety_idx=%u", ety_idx);
            /* offset 2 bytes for the type of vid & port */
            vid = (ptr_vid[ety_idx * 2] | ((UI16_T)ptr_vid[ety_idx * 2 + 1] << 8));
            port = (ptr_port[ety_idx * 2] | ((UI32_T)ptr_port[ety_idx * 2 + 1] << 8));

            _syncd_api_l2_transform(&mt, &ptr_mac_addr[ety_idx], vid, port);
            SYNCD_LOG_DUMP_MACADDR(mt.mac, NULL);
            SYNCD_LOG_DEBUG("mt.cvid=%u", mt.cvid);
            SYNCD_LOG_DEBUG("mt.port_bitmap=%X", mt.port_bitmap[0]);
            /* DB default value is all-zero, VID 0 is invalid.
             * Don't execute when VID = 0 */
            if (AIR_VLAN_ID_MIN == mt.cvid)
            {
                SYNCD_LOG_DEBUG("Default VID(%d)", mt.cvid);
                continue;
            }
            if (mt.cvid >= AIR_VLAN_ID_MAX)
            {
                SYNCD_LOG_ERROR("Invalid VID(%d)", mt.cvid);
                mw_rc = MW_E_BAD_PARAMETER;
                continue;
            }
            if(0 != (mt.port_bitmap[0] & (~BITS_RANGE(1, PLAT_MAX_PORT_NUM))))
            {
                SYNCD_LOG_ERROR("Invalid port_bitmap(%X)", mt.port_bitmap[0]);
                mw_rc = MW_E_BAD_PARAMETER;
                continue;
            }
            rc = air_l2_addMacAddr(unit, &mt);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Create MAC entry failed(%d)", rc);
                SYNCD_LOG_DUMP_MACADDR(mt.mac, NULL);
                mw_rc = MW_E_OP_INCOMPLETE;
                continue;
            }
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
            osapi_memcpy(_syncd_static_mac_entry_cache_table[ety_idx].mac_addr, &ptr_mac_addr[ety_idx], sizeof(AIR_MAC_T));
            _syncd_static_mac_entry_cache_table[ety_idx].port = port;
            _syncd_static_mac_entry_cache_table[ety_idx].vid = vid;
#endif
        }
        return mw_rc;
    }
    else if (M_CREATE == ptr_api_arg ->method)
    {
        /* Receive M_CREATE from DB means create a new MAC entry */
        if (DB_ALL_ENTRIES == ety_idx)
        {
            SYNCD_LOG_ERROR("Not support all entries.");
            return MW_E_BAD_PARAMETER;
        }
        _syncd_api_l2_transform(&mt, ptr_mac_addr, vid, port);
        SYNCD_LOG_DUMP_MACADDR(mt.mac, NULL);
        SYNCD_LOG_DEBUG("mt.cvid=%u", mt.cvid);
        SYNCD_LOG_DEBUG("mt.port_bitmap=%X", mt.port_bitmap[0]);
        if ((AIR_VLAN_ID_MIN == mt.cvid) ||
            (mt.cvid >= AIR_VLAN_ID_MAX))
        {
            SYNCD_LOG_ERROR("Invalid VID(%d)", mt.cvid);
            mw_rc = MW_E_BAD_PARAMETER;
        }
        if ((MW_E_OK == mw_rc) && (0 != (mt.port_bitmap[0] & (~BITS_RANGE(1, PLAT_MAX_PORT_NUM)))))
        {
            SYNCD_LOG_ERROR("Invalid port_bitmap(%X)", mt.port_bitmap[0]);
            mw_rc = MW_E_BAD_PARAMETER;
        }
        if (MW_E_OK == mw_rc)
        {
            rc = air_l2_addMacAddr(unit, &mt);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Create MAC entry failed(%d)", rc);
                mw_rc = MW_E_OP_INCOMPLETE;
            }
        }
        if(MW_E_OK != mw_rc)
        {
            /* Create new MAC entry failed. Request DB to delete this mac entry */
            syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                M_DELETE,
                                ptr_api_arg ->ptr_type ->t_idx,
                                DB_ALL_FIELDS,
                                ptr_api_arg ->ptr_type ->e_idx,
                                NULL,
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
        }
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
        osapi_memcpy(_syncd_static_mac_entry_cache_table[(ptr_api_arg->ptr_type->e_idx - 1)].mac_addr, ptr_mac_addr, sizeof(AIR_MAC_T));
        _syncd_static_mac_entry_cache_table[(ptr_api_arg->ptr_type->e_idx - 1)].port = port;
        _syncd_static_mac_entry_cache_table[(ptr_api_arg->ptr_type->e_idx - 1)].vid = vid;
#endif
        return mw_rc;
    }
    else if (M_DELETE == ptr_api_arg ->method)
    {
        /* Receive M_DELETE from DB means delete a new MAC entry */
        if (DB_ALL_ENTRIES == ptr_api_arg ->ptr_type ->e_idx)
        {
            /* DB_ALL_ENTRIES means to clear all MAC entry */
            rc = air_l2_clearMacAddr(unit);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Clear MAC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
            osapi_memset(&(_syncd_static_mac_entry_cache_table[0]), 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T) * MAX_STATIC_MAC_NUM);
#endif
        }
        else
        {
            _syncd_api_l2_transform(&mt, ptr_mac_addr, vid, port);
            if ((AIR_VLAN_ID_MIN == mt.cvid) ||
                (mt.cvid >= AIR_VLAN_ID_MAX))
            {
                SYNCD_LOG_ERROR("Invalid VID(%d)", mt.cvid);
                return MW_E_BAD_PARAMETER;
            }
            if(0 != (mt.port_bitmap[0] & (~BITS_RANGE(1, PLAT_MAX_PORT_NUM))))
            {
                SYNCD_LOG_ERROR("Invalid port_bitmap(%X)", mt.port_bitmap[0]);
                return MW_E_BAD_PARAMETER;
            }
            /* Delete a specific MAC entry with MAC & VID */
            rc = air_l2_delMacAddr(unit, &mt);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Delete MAC entry failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
            osapi_memset(&(_syncd_static_mac_entry_cache_table[(ptr_api_arg->ptr_type->e_idx - 1)]), 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T));
#endif
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_dynamic_mac_address_entry
 * PURPOSE:
 *      Process syncd dynamic configuration info.
 *
 * INPUT:
 *      ptr_api_arg              -- porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INVALID
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_dynamic_mac_address_entry(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI8_T *ptr_data;
    UI8_T action_result = AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_DEFAULT;

    MW_CHECK_PTR(ptr_api_arg);

    ptr_data = (UI8_T *)ptr_api_arg ->ptr_data;
    if ((M_GET == ptr_api_arg ->method))
    {
        return MW_E_OK;
    }
    action_result = *(UI8_T *)ptr_data;
    switch(ptr_api_arg ->ptr_type ->f_idx)
    {
        case ACTION_RESULT:
            if(AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_START == action_result)
            {
                mw_rc = _syncd_api_dynamic_mac_address_entry_action_get_start(ptr_api_arg);
            }
            else if(AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_CONTINUE == action_result)
            {
                mw_rc = _syncd_api_dynamic_mac_address_entry_action_get_continue(ptr_api_arg);
            }
            break;
        default:
            return MW_E_NOT_SUPPORT;
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_flushMacAddr
 * PURPOSE:
 *      This API is used to flush port or vlan dynamic mac address.
 *
 * INPUT:
 *      unit        -- Device ID
 *      value       -- Port id or vlan id
 *      type        -  Port or vlan
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_flushMacAddr(
    const UI32_T    unit,
    const UI32_T    value,
    const SYNCD_API_L2_TYPE_T type)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
    UI8_T                       index = 0;
    AIR_MAC_ENTRY_T             mt;
    ONE_DB_STATIC_MAC_ENTRY_T   *ptr_entry = NULL;
    ONE_DB_STATIC_MAC_ENTRY_T   zero_entry;
#endif
    AIR_L2_MAC_FLUSH_TYPE_T    flush_type;

    if (SYNCD_API_L2_FLUSH_PORT == type)
    {
        MW_CHECK_MIN_MAX_RANGE(value, 1, PLAT_MAX_PORT_NUM);
        flush_type = AIR_L2_MAC_FLUSH_TYPE_PORT;
    }
    else if (SYNCD_API_L2_FLUSH_VLAN == type)
    {
        MW_CHECK_MIN_MAX_RANGE(value, 1, VLAN_CONFIG_MAX_VID);
        flush_type = AIR_L2_MAC_FLUSH_TYPE_VID;
    }
    else
    {
        return  MW_E_BAD_PARAMETER;
    }

    rc = air_l2_flushMacAddr(unit, flush_type, value);

#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
    osapi_memset(&mt, 0, sizeof(mt));
    osapi_memset(&zero_entry, 0, sizeof(zero_entry));
    if(AIR_E_OK == rc)
    {
        /* Re-setting the static mac entry */
        for( ; index < MAX_STATIC_MAC_NUM; index++)
        {
            ptr_entry = (ONE_DB_STATIC_MAC_ENTRY_T *)&(_syncd_static_mac_entry_cache_table[index]);
            if((NULL == ptr_entry) ||
               (0 == osapi_memcmp(ptr_entry, &zero_entry, sizeof(ONE_DB_STATIC_MAC_ENTRY_T))))
            {
                continue;
            }
            if(((SYNCD_API_L2_FLUSH_PORT == type) && (value == ptr_entry->port))
                    || ((SYNCD_API_L2_FLUSH_VLAN == type) && (value == ptr_entry->vid)))
            {
                _syncd_api_l2_transform(&mt, &(ptr_entry->mac_addr), ptr_entry->vid, ptr_entry->port);
                if((VLAN_CONFIG_MIN_VID > mt.cvid) || (VLAN_CONFIG_MAX_VID < mt.cvid))
                {
                    osapi_memset(ptr_entry, 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T));
                    continue;
                }
                if(0 != (mt.port_bitmap[0] & (~BITS_RANGE(1, PLAT_MAX_PORT_NUM))))
                {
                    SYNCD_LOG_ERROR("Invalid port_bitmap(%X)", mt.port_bitmap[0]);
                    osapi_memset(ptr_entry, 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T));
                    continue;
                }
                rc = air_l2_addMacAddr(unit, &mt);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Create MAC entry failed(%d)", rc);
                }
            }
        }
    }
#endif

    if(AIR_E_OK != rc)
    {
        return MW_E_OTHERS;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_transferPortStaticMacAddr
 * PURPOSE:
 *      This API is used to transfer the static MAC address from one port to another port.
 *
 * INPUT:
 *      src_port    -- Source port
 *      dst_port    -- Destination port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If dest_port is 0, it means flush source port static mac address.
 */
MW_ERROR_NO_T
syncd_api_transferPortStaticMacAddr(
    const UI32_T    src_port,
    const UI32_T    dst_port)
{
    UI8_T                       *ptr_data = NULL;
    UI8_T                       *ptr_del_payload = NULL, *ptr_add_payload = NULL;
    UI16_T                      del_msg_size = 0, add_msg_size = 0;
    UI16_T                      size = 0, idx = 0;
    DB_MSG_T                    *ptr_msg = NULL;
    DB_MSG_T                    *ptr_del_multi_msg = NULL, *ptr_add_multi_msg = NULL;
    MW_ERROR_NO_T               ret = MW_E_OK;
    DB_STATIC_MAC_ENTRY_T       *ptr_mac_entry = NULL;
    ONE_DB_STATIC_MAC_ENTRY_T   static_mac_entry;
    DB_REQUEST_TYPE_T           request =
    {
        .t_idx = STATIC_MAC_ENTRY,
        .f_idx = DB_ALL_FIELDS,
        .e_idx = 0
    };

    if((PLAT_CPU_PORT == src_port) || (PLAT_MAX_PORT_NUM < src_port) || (PLAT_MAX_PORT_NUM < dst_port))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_del_multi_msg = dbapi_createMsg(NULL, M_DELETE, 0, (DB_MSG_PAYLOAD_SIZE + 1), &del_msg_size, &ptr_del_payload);
    if (NULL == ptr_del_multi_msg)
    {
        return MW_E_OTHERS;
    }
    osapi_memset(&static_mac_entry, 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T));
    if(0 != dst_port)
    {
        ptr_add_multi_msg = dbapi_createMsg(NULL, M_CREATE, 0, sizeof(ONE_DB_STATIC_MAC_ENTRY_T), &add_msg_size, &ptr_add_payload);
        if (NULL == ptr_add_multi_msg)
        {
            MW_FREE(ptr_del_multi_msg);
            return MW_E_OTHERS;
        }
    }

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
            if(ptr_mac_entry->port[idx] != src_port)
            {
                continue;
            }
            request.e_idx = (idx + 1);
            ret = dbapi_appendMsgPayload(&request, NULL, &ptr_del_multi_msg, &del_msg_size, &ptr_del_payload);
            if (MW_E_OK != ret)
            {
                MW_FREE(ptr_msg);
                MW_FREE(ptr_del_multi_msg);
                MW_FREE(ptr_add_multi_msg);
                SYNCD_LOG_ERROR("%s: [mac setting]Set multiple DB data payload fail, ret:%d\n", __func__, ret);
                return MW_E_OTHERS;
            }

            if(0 != dst_port)
            {
                static_mac_entry.port = dst_port;
                static_mac_entry.vid = ptr_mac_entry->vid[idx];
                osapi_memcpy(static_mac_entry.mac_addr, ptr_mac_entry->mac_addr[idx], sizeof(MW_MAC_T));
                ret = dbapi_appendMsgPayload(&request, (UI8_T *) &static_mac_entry, &ptr_add_multi_msg, &add_msg_size, &ptr_add_payload);
                if (MW_E_OK != ret)
                {
                    MW_FREE(ptr_msg);
                    MW_FREE(ptr_del_multi_msg);
                    MW_FREE(ptr_add_multi_msg);
                    SYNCD_LOG_ERROR("%s: [mac setting]Set multiple DB data payload fail, ret:%d\n", __func__, ret);
                    return MW_E_OTHERS;
                }
            }
        }
        MW_FREE(ptr_msg);
    }

    if((NULL != ptr_del_multi_msg) && (0 != ptr_del_multi_msg->type.count))
    {
        ret = dbapi_sendMsg(ptr_del_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_del_multi_msg);
    }
    if((NULL != ptr_add_multi_msg) && (0 != ptr_add_multi_msg->type.count))
    {
        ret |= dbapi_sendMsg(ptr_add_multi_msg, MSG_TIMEOUT_RETRUN_IMMEDIATELY);
    }
    else
    {
        MW_FREE(ptr_add_multi_msg);
    }

    if(MW_E_OK != ret)
    {
        SYNCD_LOG_ERROR("%s: Send DB message fail, ret:%d\n", __func__, ret);
        ret = MW_E_OTHERS;
    }

    return MW_E_OK;
}
