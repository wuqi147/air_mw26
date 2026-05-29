/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:   snmp_mib2_ifmib_ifxtable.c
 * PURPOSE:
 *      IFMIB definition and function of SNMP.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmp_table.h"
#include "lwip/apps/snmp_scalar.h"
#include "lwip/stats.h"
#include "db_api.h"
#include <string.h>

/* MACRO FUNCTION DECLARATIONS
 */
#if LWIP_SNMP && SNMP_LWIP_MIB2

#define IF_LINKUPDOWN_ENABLE  1
#define IF_LINKUPDOWN_DISABLE 2

#define IFINMULTICASTPKTS         2
#define IFINBROADCASTPKTS         3
#define IFOUTMULTICASTPKTS        4
#define IFOUTBROADCASTPKTS        5
#define IFHCINOCTETS              6
#define IFHCINUCASTPKTS           7
#define IFHCINMULTICASTPKTS       8
#define IFHCINBROADCASTPKTS       9
#define IFHCOUTOCTETS             10
#define IFHCOUTUCASTPKTS          11
#define IFHCOUTMULTICASTPKTS      12
#define IFHCOUTBROADCASTPKTS      13
#define IFLINKUPDOWNTRAPENABLE    14
#define IFHIGHSPEED               15

#if SNMP_USE_NETCONN
#define SYNC_NODE_NAME(node_name) node_name ## _synced
#define CREATE_LWIP_SYNC_NODE(oid, node_name) \
     static const struct snmp_threadsync_node node_name ## _synced = SNMP_CREATE_THREAD_SYNC_NODE(oid, &node_name.node, &snmp_mib2_lwip_locks);
#else
#define SYNC_NODE_NAME(node_name) node_name
#define CREATE_LWIP_SYNC_NODE(oid, node_name)
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static snmp_err_t
_ifxtable_get_cell_instance(
    const u32_t *ptr_column,
    const u32_t *ptr_row_oid,
    u8_t row_oid_len,
    struct snmp_node_instance *ptr_cell_instance);
static snmp_err_t
_ifxtable_get_next_cell_instance(
    const u32_t *ptr_column,
    struct snmp_obj_id *ptr_row_oid,
    struct snmp_node_instance *ptr_cell_instance);
static s16_t
_ifxtable_get_value(
    struct snmp_node_instance *ptr_instance,
    void *ptr_value);
static snmp_err_t
_ifxtable_set_test(
    struct snmp_node_instance *ptr_instance,
    u16_t len,
    void *ptr_value);
static snmp_err_t
_ifxtable_set_value(
    struct snmp_node_instance *ptr_instance,
    u16_t len,
    void *ptr_value);

/* STATIC VARIABLE DECLARATIONS
 */
/* list of allowed value ranges for incoming OID */
static const struct snmp_oid_range _ifxtable_oid_ranges[] =
{
    { 1, 0xff }
};

static const struct snmp_table_col_def _ifxtable_columns[] =
{
    {  2, SNMP_ASN1_TYPE_COUNTER,           SNMP_NODE_INSTANCE_READ_ONLY },  /* ifInMulticastPkts */
    {  3, SNMP_ASN1_TYPE_COUNTER,           SNMP_NODE_INSTANCE_READ_ONLY },  /* ifInBroadcastPkts */
    {  4, SNMP_ASN1_TYPE_COUNTER,           SNMP_NODE_INSTANCE_READ_ONLY },  /* ifOutMulticastPkts */
    {  5, SNMP_ASN1_TYPE_COUNTER,           SNMP_NODE_INSTANCE_READ_ONLY },  /* ifOutBroadcastPkts */
    {  6, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCInoctets */
    {  7, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCInUcastPkts */
    {  8, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCInMulticastPkts */
    {  9, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCInBroadcastPkts */
    { 10, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCOutoctets */
    { 11, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCOutUcastPkts */
    { 12, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCOutMulticastPkts */
    { 13, SNMP_ASN1_TYPE_COUNTER64,         SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHCOutBroadcastPkts */
    { 14, SNMP_ASN1_TYPE_INTEGER,           SNMP_NODE_INSTANCE_READ_WRITE }, /* ifLinkUpDownTrapEnable */
    { 15, SNMP_ASN1_TYPE_GAUGE,             SNMP_NODE_INSTANCE_READ_ONLY },  /* ifHighSpeed */
};

#if !SNMP_SAFE_REQUESTS
static const struct snmp_table_node _ifxtable = SNMP_TABLE_CREATE(
      1, _ifxtable_columns,
      _ifxtable_get_cell_instance, _ifxtable_get_next_cell_instance,
      _ifxtable_get_value, _ifxtable_set_test, _ifxtable_set_value);
#else
static const struct snmp_table_node _ifxtable = SNMP_TABLE_CREATE(
      1, _ifxtable_columns,
      _ifxtable_get_cell_instance, _ifxtable_get_next_cell_instance,
      _ifxtable_get_value, NULL, NULL);
#endif

/* the following nodes access variables in LWIP stack from SNMP worker thread and must therefore be synced to LWIP (TCPIP) thread */
CREATE_LWIP_SYNC_NODE(1, _ifxtable)

static const struct snmp_node *const _ifMIBObjects_nodes[] =
{
    &SYNC_NODE_NAME(_ifxtable).node.node
};

static const struct snmp_tree_node _snmp_mib2_ifMIBObjects_root = SNMP_CREATE_TREE_NODE(1, _ifMIBObjects_nodes);

static const struct snmp_node *const _ifmib_nodes[] =
{
    &_snmp_mib2_ifMIBObjects_root.node
};

/* GLOBAL VARIABLE DECLARATIONS
 */
const struct snmp_tree_node _ext_snmp_mib2_ifmib_root = SNMP_CREATE_TREE_NODE(31, _ifmib_nodes);

/* LOCAL SUBPROMGRAM BODIES
*/
static snmp_err_t
_ifxtable_get_cell_instance(
    const u32_t *ptr_column,
    const u32_t *ptr_row_oid,
    u8_t row_oid_len,
    struct snmp_node_instance *ptr_cell_instance)
{
    u32_t ifIndex;
    struct netif *netif;

    LWIP_UNUSED_ARG(ptr_column);

    /* check if incoming OID length and if values are in plausible range */
    if (0 == snmp_oid_in_range(ptr_row_oid, row_oid_len, _ifxtable_oid_ranges, LWIP_ARRAYSIZE(_ifxtable_oid_ranges)))
    {
        return SNMP_ERR_NOSUCHINSTANCE;
    }

    /* get ifindex from incoming OID */
    ptr_cell_instance->reference.u32 = ptr_row_oid[0];

    /* not found */
    if (ptr_cell_instance->reference.u32 > PLAT_MAX_PORT_NUM)
    {
        return SNMP_ERR_NOSUCHINSTANCE;
    }

    return SNMP_ERR_NOERROR;
}

static snmp_err_t
_ifxtable_get_next_cell_instance(
    const u32_t *ptr_column,
    struct snmp_obj_id *ptr_row_oid,
    struct snmp_node_instance *ptr_cell_instance)
{
    struct snmp_next_oid_state state;
    u8_t i;
    u32_t ifIndex;
    u32_t result_temp[LWIP_ARRAYSIZE(_ifxtable_oid_ranges)];

    LWIP_UNUSED_ARG(ptr_column);

    /* init struct to search next oid */
    snmp_next_oid_init(&state, ptr_row_oid->id, ptr_row_oid->len, result_temp, LWIP_ARRAYSIZE(_ifxtable_oid_ranges));

    /* iterate over all possible OIDs to find the next one */
    for(i = 1; i <= PLAT_MAX_PORT_NUM; i++)
    {
        u32_t test_oid[LWIP_ARRAYSIZE(_ifxtable_oid_ranges)];
        test_oid[0] = i;
        ifIndex = i;

        /* check generated OID: is it a candidate for the next one? */
        if(0 != snmp_next_oid_check(&state, test_oid, LWIP_ARRAYSIZE(_ifxtable_oid_ranges), &ifIndex))
        {
          break;
        }
    }

    /* did we find a next one? */
    if (SNMP_NEXT_OID_STATUS_SUCCESS == state.status)
    {
        snmp_oid_assign(ptr_row_oid, state.next_oid, state.next_oid_len);
        /* store ifIndex for subsequent operations (get/test/set) */
        ptr_cell_instance->reference.u32 = *(u32_t *)(state.reference);
        return SNMP_ERR_NOERROR;
    }

    /* not found */
    return SNMP_ERR_NOSUCHINSTANCE;
}

static s16_t
_ifxtable_get_value(
    struct snmp_node_instance *ptr_instance,
    void *ptr_value)
{
    u32_t ifIndex = ptr_instance->reference.u32;
    u32_t *ptr_value_u32 = (u32_t *)ptr_value;
    s32_t *ptr_value_s32 = (s32_t *)ptr_value;
    u64_t *ptr_value_u64 = (u64_t *)ptr_value;
    u64_t multicast_pkts, broadcast_pkts, all_packets;
    u16_t value_len;
    u16_t result = 0;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;

    switch (SNMP_TABLE_GET_COLUMN_FROM_OID(ptr_instance->instance_oid.id))
    {
        case IFINMULTICASTPKTS: /* ifInMulticastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u32 = *(u32_t *)ptr_data;
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        case IFINBROADCASTPKTS: /* ifInBroadcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u32 = *(u32_t *)ptr_data;
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        case IFOUTMULTICASTPKTS: /* ifOutMulticastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u32 = *(u32_t *)ptr_data;
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        case IFOUTBROADCASTPKTS: /* ifOutBroadcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u32 = *(u32_t *)ptr_data;
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        case IFHCINOCTETS: /* ifHCInoctets */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_OCTETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCINUCASTPKTS: /* ifHCInUcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_UNICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCINMULTICASTPKTS: /* ifHCInMulticastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCINBROADCASTPKTS: /* ifHCInBroadcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_RX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCOUTOCTETS: /* ifHCOutoctets */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_OCTETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCOUTUCASTPKTS: /* ifHCOutUcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_UNICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCOUTMULTICASTPKTS: /* ifHCOutMulticastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFHCOUTBROADCASTPKTS: /* ifHCOutBroadcastPkts */
            if (MW_E_OK != snmp_queue_getData(MIB_CNT, MIB_CNT_TX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            *ptr_value_u64 = *(u64_t *)ptr_data;
            value_len = sizeof(*ptr_value_u64);
            MW_FREE(ptr_msg);
            break;
        case IFLINKUPDOWNTRAPENABLE: /* ifLinkUpDownTrapEnable */
            if (MW_E_OK != snmp_queue_getData(SNMP_INFO, SNMP_TRAP_TYPE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            if((*ptr_data) & SNMP_LINKUP_DOWN_SUPPORT)
            {
                *ptr_value_u32 = IF_LINKUPDOWN_ENABLE;
            }
            else
            {
                *ptr_value_u32 = IF_LINKUPDOWN_DISABLE;
            }
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        case IFHIGHSPEED: /* ifHighSpeed */
            if (MW_E_OK != snmp_queue_getData(PORT_OPER_INFO, PORT_OPER_STATUS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
            {
                return result;
            }
            if (0 == *ptr_data)
            {
                *ptr_value_u32 = 0;
            }
            else
            {
                MW_FREE(ptr_msg);
                if (MW_E_OK != snmp_queue_getData(PORT_OPER_INFO, PORT_OPER_SPEED, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data))
                {
                    return result;
                }
                if (*ptr_data == SNMP_SPEED_10M)
                {
                    *ptr_value_u32 = 10;
                }
                else if (*ptr_data == SNMP_SPEED_100M)
                {
                    *ptr_value_u32 = 100;
                }
                else if (*ptr_data == SNMP_SPEED_1000M)
                {
                    *ptr_value_u32 = 1000;
                }
                else if (*ptr_data == SNMP_SPEED_2500M)
                {
                    *ptr_value_u32 = 2500;
                }
            }
            value_len = sizeof(*ptr_value_u32);
            MW_FREE(ptr_msg);
            break;
        default:
            return 0;
    }

    return value_len;
}

#if !SNMP_SAFE_REQUESTS

static snmp_err_t
_ifxtable_set_test(
    struct snmp_node_instance *ptr_instance,
    u16_t len,
    void *ptr_value)
{
    s32_t *sint_ptr = (s32_t *)ptr_value;

    /* stack should never call this method for another column,
    because all other columns are set to readonly */
    LWIP_ASSERT("Invalid column", (IFLINKUPDOWNTRAPENABLE == SNMP_TABLE_GET_COLUMN_FROM_OID(ptr_instance->instance_oid.id)));
    LWIP_UNUSED_ARG(len);

    if(SNMP_ASN1_TYPE_INTEGER == ptr_instance->asn1_type)
    {
        if(IFLINKUPDOWNTRAPENABLE == SNMP_TABLE_GET_COLUMN_FROM_OID(ptr_instance->instance_oid.id))
        {
            if (IF_LINKUPDOWN_ENABLE == *sint_ptr || IF_LINKUPDOWN_DISABLE == *sint_ptr)
            {
                return SNMP_ERR_NOERROR;
            }
        }
    }

    return SNMP_ERR_WRONGVALUE;
}

static snmp_err_t
_ifxtable_set_value(
    struct snmp_node_instance *ptr_instance,
    u16_t len,
    void *ptr_value)
{
    s32_t *sint_ptr = (s32_t *)ptr_value;
    u32_t ifIndex = ptr_instance->reference.u32;

    /* stack should never call this method for another column,
    because all other columns are set to readonly */
    LWIP_ASSERT("Invalid column", (IFLINKUPDOWNTRAPENABLE == SNMP_TABLE_GET_COLUMN_FROM_OID(ptr_instance->instance_oid.id)));
    LWIP_UNUSED_ARG(len);

    u8_t trap_type = snmp_check_trap_type_support();

    if(SNMP_ASN1_TYPE_INTEGER == ptr_instance->asn1_type)
    {
        if(IFLINKUPDOWNTRAPENABLE == SNMP_TABLE_GET_COLUMN_FROM_OID(ptr_instance->instance_oid.id))
        {
            if (IF_LINKUPDOWN_ENABLE == *sint_ptr)
            {
                trap_type |= SNMP_LINKUP_DOWN_SUPPORT;
            }
            else if (IF_LINKUPDOWN_DISABLE == *sint_ptr)
            {
                trap_type &= (~SNMP_LINKUP_DOWN_SUPPORT);
            }
        }
    }

    if(MW_E_OK == snmp_queue_setData(M_UPDATE, SNMP_INFO, SNMP_TRAP_TYPE, DB_ALL_ENTRIES, &trap_type, sizeof(u8_t)))
    {
        return SNMP_ERR_NOERROR;
    }

    return SNMP_ERR_NOERROR;
}

#endif /* SNMP_SAFE_REQUESTS */
#endif /* LWIP_SNMP && SNMP_LWIP_MIB2 */

/* EXPORTED SUBPROGRAM BODIES
 */
