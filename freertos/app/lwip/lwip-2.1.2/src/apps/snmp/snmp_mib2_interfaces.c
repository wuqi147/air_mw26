/**
 * @file
 * Management Information Base II (RFC1213) INTERFACES objects and functions.
 */

/*
 * Copyright (c) 2006 Axon Digital Design B.V., The Netherlands.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *         Christiaan Simons <christiaan.simons@axon.tv>
 */

#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmp_table.h"
#include "lwip/apps/snmp_scalar.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "db_api.h"

#include <string.h>

#if LWIP_SNMP && SNMP_LWIP_MIB2
#if SNMP_USE_NETCONN
#define SYNC_NODE_NAME(node_name) node_name ## _synced
#define CREATE_LWIP_SYNC_NODE(oid, node_name) \
   static const struct snmp_threadsync_node node_name ## _synced = SNMP_CREATE_THREAD_SYNC_NODE(oid, &node_name.node, &snmp_mib2_lwip_locks);
#else
#define SYNC_NODE_NAME(node_name) node_name
#define CREATE_LWIP_SYNC_NODE(oid, node_name)
#endif


/* --- interfaces .1.3.6.1.2.1.2 ----------------------------------------------------- */

/* refer to AIR_SWC_JUMBO_SIZE_T */
static const u32_t _snmpMtuSize[] = {
  SNMP_IF_MTU_DEFAULT,
  0,
  SNMP_IF_MTU_2K,
  SNMP_IF_MTU_3K,
  SNMP_IF_MTU_4K,
  SNMP_IF_MTU_5K,
  SNMP_IF_MTU_6K,
  SNMP_IF_MTU_7K,
  SNMP_IF_MTU_8K,
  SNMP_IF_MTU_9K,
  SNMP_IF_MTU_12K,
  SNMP_IF_MTU_15K
};

static s16_t
interfaces_get_value(struct snmp_node_instance *instance, void *value)
{
  if (instance->node->oid == 1) {
    s32_t *sint_ptr = (s32_t *)value;
/* The following line does not apply to MW
    s32_t num_netifs = 0;

    struct netif *netif;
    NETIF_FOREACH(netif) {
      num_netifs++;
    }

    *sint_ptr = num_netifs;
 */
    *sint_ptr = PLAT_MAX_PORT_NUM;
    return sizeof(*sint_ptr);
  }

  return 0;
}

/* list of allowed value ranges for incoming OID */
static const struct snmp_oid_range interfaces_Table_oid_ranges[] = {
  { 1, 0xff } /* netif->num is u8_t */
};


static const u8_t iftable_ifOperStatus_up   = 1;
static const u8_t iftable_ifOperStatus_down = 2;

static const u8_t iftable_ifAdminStatus_up             = 1;
static const u8_t iftable_ifAdminStatus_lowerLayerDown = 7;
static const u8_t iftable_ifAdminStatus_down           = 2;

static snmp_err_t
interfaces_Table_get_cell_instance(const u32_t *column, const u32_t *row_oid, u8_t row_oid_len, struct snmp_node_instance *cell_instance)
{
  u32_t ifIndex;
  struct netif *netif;

  LWIP_UNUSED_ARG(column);

  /* check if incoming OID length and if values are in plausible range */
  if (!snmp_oid_in_range(row_oid, row_oid_len, interfaces_Table_oid_ranges, LWIP_ARRAYSIZE(interfaces_Table_oid_ranges))) {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

/* The following line does not apply to MW
  ifIndex = row_oid[0];

  // find netif with index
  NETIF_FOREACH(netif) {
    if (netif_to_num(netif) == ifIndex) {
      // store netif pointer for subsequent operations (get/test/set)
      cell_instance->reference.ptr = netif;
      return SNMP_ERR_NOERROR;
    }
  }
 */
  /* get netif index from incoming OID */
  if (PLAT_MAX_PORT_NUM >= row_oid[0]) {
    cell_instance->reference.u32 = row_oid[0];
    return SNMP_ERR_NOERROR;
  }

  /* not found */
  return SNMP_ERR_NOSUCHINSTANCE;
}

static snmp_err_t
interfaces_Table_get_next_cell_instance(const u32_t *column, struct snmp_obj_id *row_oid, struct snmp_node_instance *cell_instance)
{
  struct netif *netif;
  struct snmp_next_oid_state state;
  u8_t i;
  u32_t ifIndex;
  u32_t result_temp[LWIP_ARRAYSIZE(interfaces_Table_oid_ranges)];

  LWIP_UNUSED_ARG(column);

  /* init struct to search next oid */
  snmp_next_oid_init(&state, row_oid->id, row_oid->len, result_temp, LWIP_ARRAYSIZE(interfaces_Table_oid_ranges));

  /* iterate over all possible OIDs to find the next one */
  for(i = 1; i <= PLAT_MAX_PORT_NUM; i++) {
      u32_t test_oid[LWIP_ARRAYSIZE(interfaces_Table_oid_ranges)];
      test_oid[0] = i;
      ifIndex = i;

      /* check generated OID: is it a candidate for the next one? */
      if(snmp_next_oid_check(&state, test_oid, LWIP_ARRAYSIZE(interfaces_Table_oid_ranges), &ifIndex)){
        break;
      }
  }

  /* did we find a next one? */
  if (state.status == SNMP_NEXT_OID_STATUS_SUCCESS) {
    snmp_oid_assign(row_oid, state.next_oid, state.next_oid_len);
    /* store netif pointer for subsequent operations (get/test/set) */
    cell_instance->reference.u32 = *(u32_t *)(state.reference);
    return SNMP_ERR_NOERROR;
  }

  /* not found */
  return SNMP_ERR_NOSUCHINSTANCE;
}

static s16_t
interfaces_Table_get_value(struct snmp_node_instance *instance, void *value)
{
  u32_t ifIndex = instance->reference.u32;
  u32_t *value_u32 = (u32_t *)value;
  s32_t *value_s32 = (s32_t *)value;
  u16_t value_len;
  u16_t result = 0;
  u64_t multicast_pkts, broadcast_pkts, all_packets;
  DB_MSG_T *ptr_msg = NULL;
  UI16_T data_size = 0;
  UI8_T *ptr_data = NULL;

  switch (SNMP_TABLE_GET_COLUMN_FROM_OID(instance->instance_oid.id)) {
    case 1: /* ifIndex */
      *value_s32 = ifIndex;
      value_len = sizeof(*value_s32);
      break;
    /* The following line does not apply to MW
    case 2: /* ifDescr *//*
      value_len = sizeof(netif->name);
      MEMCPY(value, netif->name, value_len);
      break;
    case 3: /* ifType *//*
      *value_s32 = netif->link_type;
      value_len = sizeof(*value_s32);
      break;
     */
    case 4: /* ifMtu */
      if (snmp_queue_getData(JUMBO_FRAME_INFO, JUMBO_FRAME_CFG, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_s32 = _snmpMtuSize[(*ptr_data)];
      value_len = sizeof(*value_s32);
      MW_FREE(ptr_msg);
      break;
    case 5: /* ifSpeed */
      if (MW_E_OK != snmp_queue_getData(PORT_OPER_INFO, PORT_OPER_STATUS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data)) {
        return result;
      }
      if (0 == *ptr_data) {
        *value_u32 = 0;
      } else {
        MW_FREE(ptr_msg);
        if (MW_E_OK != snmp_queue_getData(PORT_OPER_INFO, PORT_OPER_SPEED, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data)) {
          return result;
        }
        if(*ptr_data == SNMP_SPEED_10M) {
          *value_u32 = 10 * 1000 * 1000;
        } else if (*ptr_data == SNMP_SPEED_100M) {
          *value_u32 = 100 * 1000 * 1000;
        } else if (*ptr_data == SNMP_SPEED_1000M) {
          *value_u32 = 1000 * 1000 * 1000;
        } else if (*ptr_data == SNMP_SPEED_2500M) {
          *value_u32 = 2500 * 1000 * 1000;
        }
      }
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    /* The following line does not apply to MW
    case 6: /* ifPhysAddress *//*
      value_len = sizeof(netif->hwaddr);
      MEMCPY(value, &netif->hwaddr, value_len);
      break;
     */
    case 7: /* ifAdminStatus */
      if (snmp_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      if (*ptr_data) {
        *value_s32 = iftable_ifAdminStatus_up;
      } else {
        *value_s32 = iftable_ifAdminStatus_down;
      }
      value_len = sizeof(*value_s32);
      MW_FREE(ptr_msg);
      break;
    case 8: /* ifOperStatus */
      if (snmp_queue_getData(PORT_OPER_INFO, PORT_OPER_STATUS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      if (*ptr_data) {
        *value_s32 = iftable_ifOperStatus_up;
      } else {
        *value_s32 = iftable_ifOperStatus_down;
      }
      value_len = sizeof(*value_s32);
      MW_FREE(ptr_msg);
      break;
    /* The following line does not apply to MW
    case 9: /* ifLastChange *//*
      *value_u32 = netif->ts;
      value_len = sizeof(*value_u32);
      break;
     */
    case 10: /* ifInOctets */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_OCTETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 11: /* ifInUcastPkts */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_UNICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 12: /* ifInNUcastPkts */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      multicast_pkts = *(u64_t *)ptr_data;
      MW_FREE(ptr_msg);
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      broadcast_pkts = *(u64_t *)ptr_data;
      *value_u32 = (u32_t)(multicast_pkts + broadcast_pkts);
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 13: /* ifInDiscards */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_DISCARDS_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 14: /* ifInErrors */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_RX_ERRORS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 15: /* ifInUnkownProtos */
      *value_u32 = 0;
      value_len = sizeof(*value_u32);
      break;
    case 16: /* ifOutOctets */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_OCTETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 17: /* ifOutUcastPkts */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_UNICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 18: /* ifOutNUcastPkts */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_MULTICAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      multicast_pkts = *(u64_t *)ptr_data;
      MW_FREE(ptr_msg);
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_BROADCAST_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      broadcast_pkts = *(u64_t *)ptr_data;
      *value_u32 = (u32_t)(multicast_pkts + broadcast_pkts);
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 19: /* ifOutDiscarts */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_DISCARDS_PACKETS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    case 20: /* ifOutErrors */
      if (snmp_queue_getData(MIB_CNT, MIB_CNT_TX_ERRORS, ifIndex, &ptr_msg, &data_size, (void **)&ptr_data) != MW_E_OK)
      {
        return result;
      }
      *value_u32 = *(u32_t *)ptr_data;
      value_len = sizeof(*value_u32);
      MW_FREE(ptr_msg);
      break;
    /* The following line does not apply to MW
    case 21: /* ifOutQLen *//*
      *value_u32 = iftable_ifOutQLen;
      value_len = sizeof(*value_u32);
      break;
    /** @note returning zeroDotZero (0.0) no media specific MIB support *//*
    case 22: /* ifSpecific *//*
      value_len = snmp_zero_dot_zero.len * sizeof(u32_t);
      MEMCPY(value, snmp_zero_dot_zero.id, value_len);
      break;
     */
    default:
      return 0;
  }

  return value_len;
}

#if !SNMP_SAFE_REQUESTS

static snmp_err_t
interfaces_Table_set_test(struct snmp_node_instance *instance, u16_t len, void *value)
{
  s32_t *sint_ptr = (s32_t *)value;

  /* stack should never call this method for another column,
  because all other columns are set to readonly */
  LWIP_ASSERT("Invalid column", (SNMP_TABLE_GET_COLUMN_FROM_OID(instance->instance_oid.id) == 7));
  LWIP_UNUSED_ARG(len);

  if (*sint_ptr == 1 || *sint_ptr == 2) {
    return SNMP_ERR_NOERROR;
  }

  return SNMP_ERR_WRONGVALUE;
}

static snmp_err_t
interfaces_Table_set_value(struct snmp_node_instance *instance, u16_t len, void *value)
{
  struct netif *netif = (struct netif *)instance->reference.ptr;
  u32_t ifIndex = instance->reference.u32;
  s32_t *sint_ptr = (s32_t *)value;
  u8_t db_set_value = 0;

  /* stack should never call this method for another column,
  because all other columns are set to readonly */
  LWIP_ASSERT("Invalid column", (SNMP_TABLE_GET_COLUMN_FROM_OID(instance->instance_oid.id) == 7));
  LWIP_UNUSED_ARG(len);

  if (*sint_ptr == iftable_ifAdminStatus_up) {
    db_set_value = 1;
  } else if (*sint_ptr == iftable_ifAdminStatus_down) {
    db_set_value = 0;
  }
  if(snmp_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_ADMIN_STATUS, ifIndex, &db_set_value, sizeof(u8_t)) == MW_E_OK){
    return SNMP_ERR_NOERROR;
  }

  return SNMP_ERR_GENERROR;
}

#endif /* SNMP_SAFE_REQUESTS */

static const struct snmp_scalar_node interfaces_Number = SNMP_SCALAR_CREATE_NODE_READONLY(1, SNMP_ASN1_TYPE_INTEGER, interfaces_get_value);

static const struct snmp_table_col_def interfaces_Table_columns[] = {
  {  1, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifIndex */
  /* The following line does not apply to MW
  {  2, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY }, /* ifDescr *//*
  {  3, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifType *//*
   */
  {  4, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifMtu */
  {  5, SNMP_ASN1_TYPE_GAUGE,        SNMP_NODE_INSTANCE_READ_ONLY }, /* ifSpeed */
  /* The following line does not apply to MW
  {  6, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY }, /* ifPhysAddress *//*
   */
#if !SNMP_SAFE_REQUESTS
  {  7, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_WRITE }, /* ifAdminStatus */
#else
  {  7, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifAdminStatus */
#endif
  {  8, SNMP_ASN1_TYPE_INTEGER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOperStatus */
  //{  9, SNMP_ASN1_TYPE_TIMETICKS,    SNMP_NODE_INSTANCE_READ_ONLY }, /* ifLastChange */
  { 10, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInOctets */
  { 11, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInUcastPkts */
  { 12, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInNUcastPkts */
  { 13, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInDiscarts */
  { 14, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInErrors */
  { 15, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifInUnkownProtos */
  { 16, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutOctets */
  { 17, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutUcastPkts */
  { 18, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutNUcastPkts */
  { 19, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutDiscarts */
  { 20, SNMP_ASN1_TYPE_COUNTER,      SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutErrors */
  /* The following line does not apply to MW
  { 21, SNMP_ASN1_TYPE_GAUGE,        SNMP_NODE_INSTANCE_READ_ONLY }, /* ifOutQLen *//*
  { 22, SNMP_ASN1_TYPE_OBJECT_ID,    SNMP_NODE_INSTANCE_READ_ONLY }  /* ifSpecific *//*
  */
};

#if !SNMP_SAFE_REQUESTS
static const struct snmp_table_node interfaces_Table = SNMP_TABLE_CREATE(
      2, interfaces_Table_columns,
      interfaces_Table_get_cell_instance, interfaces_Table_get_next_cell_instance,
      interfaces_Table_get_value, interfaces_Table_set_test, interfaces_Table_set_value);
#else
static const struct snmp_table_node interfaces_Table = SNMP_TABLE_CREATE(
      2, interfaces_Table_columns,
      interfaces_Table_get_cell_instance, interfaces_Table_get_next_cell_instance,
      interfaces_Table_get_value, NULL, NULL);
#endif

/* the following nodes access variables in LWIP stack from SNMP worker thread and must therefore be synced to LWIP (TCPIP) thread */
CREATE_LWIP_SYNC_NODE(1, interfaces_Number)
CREATE_LWIP_SYNC_NODE(2, interfaces_Table)

static const struct snmp_node *const interface_nodes[] = {
  &SYNC_NODE_NAME(interfaces_Number).node.node,
  &SYNC_NODE_NAME(interfaces_Table).node.node
};

const struct snmp_tree_node snmp_mib2_interface_root = SNMP_CREATE_TREE_NODE(2, interface_nodes);

#endif /* LWIP_SNMP && SNMP_LWIP_MIB2 */
