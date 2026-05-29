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

/* FILE NAME:  mstp_sys.h
 * PURPOSE:
 *      This file contains the declaration of setting mstp for the Magic Wand module.
 *
 * NOTES:
 */
#ifndef MSTP_SYS_H
#define MSTP_SYS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "default_config.h"
#include "stp_db.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

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
void mstp_show_port_member(void);

/* FUNCTION NAME:   mstp_create
 * PURPOSE:
 *      This API is used to initialize the mstp data.
 *
 * INPUT:
 *      portCount      - port num
 *      trunkCount     - trunk num
 *      mstiCount      - msti num
 *      maxVlanNumber  - max supported vlan num
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_create(
    const UI32_T  portCount,
    const UI32_T  trunkCount,
    const UI32_T  mstiCount,
    const UI32_T  maxVlanNumber);

/* FUNCTION NAME:   mstp_delete
 * PURPOSE:
 *      This API is used to delete the mstp data.
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
mstp_delete(
    void);

/* FUNCTION NAME:   mstp_setRegionConfig
 * PURPOSE:
 *      This API is used to set mstp region configuration
 *
 * INPUT:
 *      revision    - revision id
 *      ptr_name    - region name
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
mstp_setRegionConfig(
    UI16_T  revision,
    UI8_T   *ptr_name);

/* FUNCTION NAME:   mstp_setInstanceMappingVlan
 * PURPOSE:
 *      This API is used to set instance to vlan mapping.
 *
 * INPUT:
 *      ptr_entry     - instance & vid
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
mstp_setInstanceMappingVlan(
    MST_INSTANCE_VLAN_ENTRY *ptr_entry);

/* FUNCTION NAME:   mstp_setBridgePriority
 * PURPOSE:
 *      This API is used to set bridge priority.
 *
 * INPUT:
 *      instanceIdx  - instance index & instance id
 *      priority     - bridge priority
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
mstp_setBridgePriority(
    UI16_T instanceIdx,
    UI16_T priority);

/* FUNCTION NAME:   mstp_setPortPriority
 * PURPOSE:
 *      This API is used to set port priority.
 *
 * INPUT:
 *      port_id         - port number
 *      instanceIdx     - instance index
 *      portPriority    - port priority
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
mstp_setPortPriority(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI8_T  portPriority);

/* FUNCTION NAME:   mstp_setPortCost
 * PURPOSE:
 *      This API is used to set port cost.
 *
 * INPUT:
 *      port_id         - port number
 *      portPriority    - port priority
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
mstp_setPortPathCost(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI32_T portCost);

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
    UI32_T *ptr_num);

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
    UI8_T  *ptr_outStr);

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
    UI32_T  *ptr_outIdx);

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
    UI32_T  *ptr_num);

/* FUNCTION NAME: mstp_tick_timers
 * PURPOSE:
 *      One second tick timer
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_tick_timers(
    void);

/* FUNCTION NAME: mstp_bpdu_recv
 * PURPOSE:
 *      Mstp bpdu handle
 *
 * INPUT:
 *      portNum         - port id
 *      ptr_bpdu        - bpdu
 *      bpduSize        - bpdu size
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_bpdu_recv(
    UI32_T      portNum,
    const UI8_T *ptr_bpdu,
    UI32_T      bpduSize);

/* FUNCTION NAME: mstp_add_port
 * PURPOSE:
 *      Add port & all instance tree
 *
 * INPUT:
 *      port         - port id
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_add_port(
    UI32_T   port);

/* FUNCTION NAME: mstp_add_instance_port
 * PURPOSE:
 *      Add instance & port tree
 *
 * INPUT:
 *      info         - port & instance info
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_add_instance_port(
    MSTP_INS_PARAM_T *info);

/* FUNCTION NAME: mstp_del_port
 * PURPOSE:
 *      Del port & all instance tree
 *
 * INPUT:
 *      port         - port id
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_del_port(
    UI32_T   port);

/* FUNCTION NAME: mstp_del_instance_port
 * PURPOSE:
 *      Del port data of instance specify
 *
 * INPUT:
 *      info         - port & instance info
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
void
mstp_del_instance_port(
    MSTP_INS_PARAM_T *info);

/* FUNCTION NAME: mstp_del_instance_port
 * PURPOSE:
 *      Del port data of instance specify
 *      Del instance data
 *
 * INPUT:
 *      info         - instance
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
void
mstp_del_instance(
    MSTP_INS_PARAM_T *info);


/* FUNCTION NAME: mstp_set_bridge_forward_delay
 * PURPOSE:
 *      Set bridge forward delay time
 *
 * INPUT:
 *      forward_delay         - forward delay time
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_bridge_forward_delay(
    UI16_T  forward_delay);

/* FUNCTION NAME: mstp_set_bridge_max_age
 * PURPOSE:
 *      Set bridge max age time
 *
 * INPUT:
 *      max_age         - max age
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_bridge_max_age(
    UI16_T  max_age);

/* FUNCTION NAME: mstp_set_bridge_transmit_hold_count
 * PURPOSE:
 *      Set bridge tx hold count
 *
 * INPUT:
 *      tx_cnt         - tx hold count
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_bridge_transmit_hold_count(
    UI16_T  tx_cnt);

/* FUNCTION NAME: mstp_set_port_macOperational
 * PURPOSE:
 *      Set port mac operational flag
 *
 * INPUT:
 *      port         - port ID
 *      status       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_port_macOperational(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: mstp_set_port_oper_point_to_point_mac
 * PURPOSE:
 *      Set port oper p2p mac flag
 *
 * INPUT:
 *      port                   - port ID
 *      oper_mac_p2p_mac       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_port_oper_point_to_point_mac(
    UI32_T  port,
    UI8_T   oper_mac_p2p_mac);

/* FUNCTION NAME: mstp_set_port_administrative_bridge_port
 * PURPOSE:
 *      Set port administrative port flag
 *
 * INPUT:
 *      port         - port ID
 *      status       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_port_administrative_bridge_port(
    UI32_T  port,
    BOOL_T  status);

/* FUNCTION NAME: mstp_set_port_admin_edge
 * PURPOSE:
 *      Set port admin edge flag
 *
 * INPUT:
 *      port         - port ID
 *      status       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_port_admin_edge(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: mstp_set_port_mcheck
 * PURPOSE:
 *      Set port mcheck flag
 *
 * INPUT:
 *      port         - port ID
 *      status       - enable/disable
 *
 * OUTPUT:
 *      void
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_set_port_mcheck(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: mstp_get_mcid_digest
 * PURPOSE:
 *      Get mcid digest
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      ptr_digest   - mcid digest
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
mstp_get_mcid_digest(
    UI8_T  *ptr_digest);


/* FUNCTION NAME: mstp_get_mcid_digest
 * PURPOSE:
 *      Get mcid digest
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      ptr_digest   - mcid digest
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */

void mstp_SetLogPrintFlag(unsigned char port);


/* FUNCTION NAME: mstp_show_port_instance_param
 * PURPOSE:
 *      Show mstp port instance param
 *
 * INPUT:
 *      treeId       - instance id
 *      portId       - port id
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
void
mstp_show_port_instance_param(
    UI32_T   treeId,
    UI32_T   portId);

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
    UI8_T   enable);

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
    UI32_T   force_version);

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
    UI16_T  forward_delay);

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
    UI16_T   max_age);

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
    UI16_T  tx_hold_cnt);

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
    UI16_T  priority);

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
    UI16_T  priority);

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
    void);

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
    UI8_T  portPriority);

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
    UI32_T portCost);

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
    UI8_T   enable);

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
    UI32_T  port);

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
    UI32_T  port);

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
    UI32_T   port_number);
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
    UI8_T   priority);

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
    UI32_T  cost);

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
    UI8_T   admin_edge);


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
    UI8_T   perform);

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
    UI8_T   status);

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
    void   *ptr_data);

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
    UI8_T   link);

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
    UI8_T   mode);

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
    UI8_T   link);

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
    UI8_T   is_p2p);

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
    UI8_T   *ptr_name);

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
    void);

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
    UI32_T  *ptr_num);

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
    UI16_T vid);

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
    void);

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
    UI32_T  *ptr_num);

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
    void    *ptr_data);

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
    UI8_T mode);

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
    UI16_T  e_idx);

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
    MSTP_INSTANCE_CONFIG_T *ptr_info);

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
    UI32_T  *ptr_num);

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
    UI32_T    *ptr_out);

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
    UI8_T    *ptr_out);

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
    UI32_T  bpdu_size);

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
    void);

void
mstp_tick_timers_start(
    void);

#endif  /* MSTP_SYS_H */
