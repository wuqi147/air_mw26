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

#ifndef STP_DB_H
#define STP_DB_H

/* FILE NAME:   stp_db.h
 * PURPOSE:
 *      This file defines the data structure for stp db.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define STP_DEFAULT_PRIORITY    (32768)
#define STP_MIN_PRIORITY        (0)
#define STP_MAX_PRIORITY        (61440)
#define STP_PRIORITY_STEP       (4096)

#define STP_DEFAULT_TRANSMIT_HOLD_COUNT     (6)
#define STP_MIN_TRANSMIT_HOLD_COUNT         (1)
#define STP_MAX_TRANSMIT_HOLD_COUNT         (10)

#define STP_DEFAULT_AGEING_TIME             (300)
#define STP_MIN_AGEING_TIME                 (10)
#define STP_MAX_AGEING_TIME                 (1000000)

#define STP_DEFAULT_BRIDGE_MAX_AGE          (20)
#define STP_MIN_BRIDGE_MAX_AGE              (6)
#define STP_MAX_BRIDGE_MAX_AGE              (40)

#define STP_DEFAULT_BRIDGE_FORWARD_DELAY    (15)
#define STP_MIN_BRIDGE_FORWARD_DELAY        (4)
#define STP_MAX_BRIDGE_FORWARD_DELAY        (30)

/* MACRO FUNCTION DECLARATIONS
 */
#define stp_dbmsg_payload(ptr_msg)                                         \
         ((DB_PAYLOAD_T *)(&((ptr_msg)->ptr_payload)))
#define stp_dbpayload_data(ptr_payload)                                    \
         ((UI8_T *)(&((ptr_payload)->ptr_data)))
#define stp_dbmsg_init_reqtype(t, f, e)                                    \
         ((DB_REQUEST_TYPE_T){.t_idx = (t), .f_idx = (f), .e_idx = (e)})

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T  memberBmp;
    UI32_T  aggregatedBmp;
    UI8_T   mode;
} __attribute__((packed))STP_TRUNK_INFO_T;

#ifdef AIR_SUPPORT_RSTP_SECURITY
typedef struct
{
    UI32_T  bg_state;
    UI32_T  rg_state;
    UI32_T  lg_state;
} RSTP_SCURITY_STATE_T;

typedef enum
{
    RSTP_INCONSISTENT_ROOT,
    RSTP_INCONSISTENT_LOOP,

    RSTP_INCONSISTENT_LAST
}RSTP_INCONSISTENT_STATE_T;
#endif

typedef struct
{
    UI16_T  instance;    /* Fid & Instance ID */
    UI16_T  vid;
    UI32_T  memBmp;      /* Member Port Bmp*/
}MST_INSTANCE_VLAN_ENTRY;

typedef struct
{
    UI16_T  instance[MAX_MSTP_INSTANCE_NUM];
    UI16_T  priority[MAX_MSTP_INSTANCE_NUM];
    UI32_T  vlanbmp[MAX_MSTP_INSTANCE_NUM];
}ATTRIBUTE_PACK MSTP_INSTANCE_CONFIG_T;

typedef struct
{
    UI16_T  insIdx;
    UI32_T  portNum;
}ATTRIBUTE_PACK MSTP_INS_PARAM_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: stp_db_getStpGlobalState
 * PURPOSE:
 *      Get stp status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getStpGlobalState(
    void);

/* FUNCTION NAME: stp_db_getAutoCostState
 * PURPOSE:
 *      Get stp auto cost config status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getAutoCostState(
    void);

/* FUNCTION NAME: stp_db_setAutoCostState
 * PURPOSE:
 *      Set stp auto cost config status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
void
stp_db_setAutoCostState(
    UI8_T enable);

/* FUNCTION NAME: stp_db_getVersion
 * PURPOSE:
 *      Get stp version mode
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      stp_force_version
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getVersion(
    void);

/* FUNCTION NAME: stp_db_setVersion
 * PURPOSE:
 *      Set stp version mode
 *
 * INPUT:
 *      version       - version mode
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
stp_db_setVersion(
    UI8_T  version);

/* FUNCTION NAME: stp_db_getTrunkID
 * PURPOSE:
 *      Get trunk id by port id.
 *
 * INPUT:
 *      port             -  port id
 *
 * OUTPUT:
 *      ptr_trunk_id     -  trunk id
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_getTrunkID(
    UI32_T  port,
    UI32_T  *ptr_trunk_id);

/* FUNCTION NAME: stp_db_checkIsLowerPort
 * PURPOSE:
 *      Check port is lower port of trunk or not.
 *
 * INPUT:
 *      port             -  port id
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE or FALSE
 *
 * NOTES:
 *      None
 */

BOOL_T
stp_db_checkIsLowerPort(
    UI32_T  port,
    UI32_T  trunk_id);

/* FUNCTION NAME: stp_db_trunk_getLowerPort
 * PURPOSE:
 *      Get the lower port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Lower port id
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getLowerPort(
    UI32_T  trunk_id);

/* FUNCTION NAME: stp_db_trunk_getMode
 * PURPOSE:
 *      Get the mode of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk mode
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_trunk_getMode(
    UI32_T  trunk_id);

/* FUNCTION NAME: stp_db_trunk_setMode
 * PURPOSE:
 *      Set the mode of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk mode
 *
 * NOTES:
 *      None
 */
void
stp_db_trunk_setMode(
    UI32_T  trunk_id,
    UI8_T   mode);

/* FUNCTION NAME: stp_db_trunk_getMemberBmp
 * PURPOSE:
 *      Get the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk member
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getMemberBmp(
    UI32_T  trunk_id);

/* FUNCTION NAME: stp_db_trunk_setMemberBmp
 * PURPOSE:
 *      Set the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *      member           -  member port
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
stp_db_trunk_setMemberBmp(
    UI32_T  trunk_id,
    UI32_T   member);

/* FUNCTION NAME: stp_db_trunk_getAggBmp
 * PURPOSE:
 *      Get the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk member
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getAggBmp(
    UI32_T  trunk_id);

/* FUNCTION NAME: stp_db_trunk_setAggBmp
 * PURPOSE:
 *      Set the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *      member           -  member port
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
stp_db_trunk_setAggBmp(
    UI32_T  trunk_id,
    UI32_T   member);

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_trunk_compute_oper_cost
 * PURPOSE:
 *      Calc trunk oper cost.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      ptr_oper_cost    -  trunk oper cost
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
stp_trunk_compute_oper_cost(
    UI32_T  trunk_id,
    UI32_T  *ptr_oper_cost);
#endif

/* FUNCTION NAME: stp_db_get_msg
 * PURPOSE:
 *      Get msg by db
 *
 * INPUT:
 *      req             -  table idx
 *                      -  field idx
 *                      -  entry idx
 *
 * OUTPUT:
 *      ptr_outData     -  output data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_msg(
    DB_REQUEST_TYPE_T   req,
    void                *ptr_outData);

/* FUNCTION NAME: stp_db_update_msg
 * PURPOSE:
 *      Update msg by db
 *
 * INPUT:
 *      req             -  table idx
 *                      -  field idx
 *                      -  entry idx
 *      ptr_inData      -  input data
 *      len             -  data length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_update_msg(
    DB_REQUEST_TYPE_T   req,
    void *const         ptr_inData,
    const UI16_T        len);

/* FUNCTION NAME: stp_db_port_update_allEntries
 * PURPOSE:
 *      Update t_id, f_id, DB_ALL_ENTRIES to DB.
 *
 * INPUT:
 *      t_id           -  DB table ID
 *      f_id           -  DB field ID
 *      ptr_data       -  A pointer to data for each entry.
 *                        Its type must be aligned with DB the field.
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
stp_db_port_update_allEntries(
    const UI8_T t_id,
    const UI8_T f_id,
    const void *ptr_data);

/* FUNCTION NAME: stp_db_port_role_update
 * PURPOSE:
 *      Update port role.
 *
 * INPUT:
 *      port         -  port id
 *      role         -  role
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
stp_db_port_role_update(
    const UI32_T    port,
    const UI8_T     role);

/* FUNCTION NAME: stp_db_get_bpdu_egress_port
 * PURPOSE:
 *      Get egress port to transmit BPDU.
 *
 * INPUT:
 *      port_number    -  port number of RSTP port instance
 *
 * OUTPUT:
 *      ptr_egr_port   -  port to egress BPDU
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
void
stp_db_get_bpdu_egress_port(
    UI32_T  port_number,
    UI32_T  *ptr_egr_port)
;
/* FUNCTION NAME: stp_db_get_lag_lower_port
 * PURPOSE:
 *      Get the lower port of specify lag.
 *
 * INPUT:
 *      port_number    -  port number of RSTP port instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      lower port
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_get_lag_lower_port(
    UI32_T  port_number);

/* FUNCTION NAME: stp_db_get_port_status
 * PURPOSE:
 *      Get port status.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_status    - port status
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_status(
    UI32_T  port,
    UI8_T   *ptr_status);

/* FUNCTION NAME: stp_db_get_port_duplex
 * PURPOSE:
 *      Get port duplex.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_duplex    - port duplex
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_duplex(
    UI32_T  port,
    UI8_T   *ptr_duplex);

/* FUNCTION NAME: stp_db_get_port_speed
 * PURPOSE:
 *      Get port speed.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_speed    - port speed
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_speed(
    UI32_T  port,
    UI8_T   *ptr_speed);

/* FUNCTION NAME: stp_db_get_port_enable_status
 * PURPOSE:
 *      Get port stp status.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_status    - port status
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_enable_status(
    UI32_T  port,
    UI8_T   *ptr_status);

/* FUNCTION NAME: stp_db_get_forward_delay_config
 * PURPOSE:
 *      Get forward delay.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_fwd_delay    - fwd delay
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_forward_delay_config(
    UI16_T  *ptr_fwd_delay);

/* FUNCTION NAME: stp_db_get_max_age_config
 * PURPOSE:
 *      Get max age.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_max_age    - max age
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_max_age_config(
    UI16_T  *ptr_max_age);

/* FUNCTION NAME: stp_db_get_transmit_hold_count_config
 * PURPOSE:
 *      Get max age.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_tx_hold_cnt    - tx hold count
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_transmit_hold_count_config(
    UI16_T  *ptr_tx_hold_cnt);

/* FUNCTION NAME: stp_db_get_bridge_priority_config
 * PURPOSE:
 *      Get bridge priority.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_pri    - bridge priority
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_bridge_priority_config(
    UI16_T  *ptr_pri);

/* FUNCTION NAME: stp_db_get_port_priority_config
 * PURPOSE:
 *      Get port priority.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_pri    - prot priority
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_priority_config(
    UI32_T  port,
    UI8_T   *ptr_pri);

/* FUNCTION NAME: stp_db_get_port_cost_config
 * PURPOSE:
 *      Get port cost.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_cost    - prot cost
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_cost_config(
    UI32_T  port,
    UI32_T   *ptr_cost);

/* FUNCTION NAME: mstp_db_get_instance_vlan_config
 * PURPOSE:
 *      Get instance-vlan-mapping config.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_info    - instance-vlan-mapping config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_db_get_instance_vlan_config(
    MSTP_INSTANCE_CONFIG_T *ptr_info);

/* FUNCTION NAME: stp_trunk_check_link_status
 * PURPOSE:
 *      Get the link status of trunk.
 *
 * INPUT:
 *      trunk_id      - trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_trunk_check_link_status(
    UI32_T trunk_id);

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_db_get_port_auto_enable_config
 * PURPOSE:
 *      Get port auto cost config.
 *
 * INPUT:
 *      port             - port id
 *
 * OUTPUT:
 *      ptr_auto_enable  - config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_auto_enable_config(
    UI32_T  port,
    UI8_T   *ptr_auto_enable);

/* FUNCTION NAME: stp_db_port_compute_oper_cost
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      port                     -  port number
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto_cost_support
 *      ptr_oper_cost            -  oper cost
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_compute_oper_cost(
    UI32_T  port,
    UI8_T  *ptr_auto_cost_support,
    UI32_T  *ptr_oper_cost);


/* FUNCTION NAME: stp_db_port_compute_oper_cost
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      port                     -  port number
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto_cost_support
 *      ptr_oper_cost            -  oper cost
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_db_port_oper_cost_update(
    const UI32_T    port,
    const UI32_T    oper_cost);

#endif

/* FUNCTION NAME: stp_db_get_port_admin_edge_config
 * PURPOSE:
 *      Get port admin edge config.
 *
 * INPUT:
 *      port             - port id
 *
 * OUTPUT:
 *      ptr_admin_edge  - config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_admin_edge_config(
    UI32_T  port,
    UI8_T   *ptr_admin_edge);

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: stp_db_port_bpdu_state_update
 * PURPOSE:
 *      Update port bpdu protect state.
 *
 * INPUT:
 *      port             - port id
 *      bpdu_state       - state
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
stp_db_port_bpdu_state_update(
    UI32_T      port,
    UI8_T       bpdu_state);

/* FUNCTION NAME: stp_set_port_bpdu_down
 * PURPOSE:
 *      Setting port bpdu protect down.
 *
 * INPUT:
 *      port             - port id
 *      bpdu_state       - state
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
stp_set_port_bpdu_down(
    UI32_T  port,
    UI8_T   bpdu_state);

/* FUNCTION NAME: rstp_get_port_security_param
 * PURPOSE:
 *      Get security data
 *
 * INPUT:
 *      port           -  port number
 *
 * OUTPUT:
 *      sec_info       -  security info
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_get_port_security_param(
    UI32_T  port,
    DB_RSTP_PORT_SEC_T *sec_info);
#endif

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: mstp_db_port_role_update
 * PURPOSE:
 *      Send to DB to update port instance role
 *
 * INPUT:
 *      port           -  port number
 *      ptr_role       -  port role
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
mstp_db_port_role_update(
    const UI32_T    port,
    UI8_T           *ptr_role);
#endif

/* FUNCTION NAME: stp_db_update_port_block
 * PURPOSE:
 *      Update port block state.
 *
 * INPUT:
 *      port             - port id
 *      oper_block       - state
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
stp_db_update_port_block(
    const UI32_T    port,
    const BOOL_T    oper_block);

/* FUNCTION NAME: stp_db_clear_all_port_block
 * PURPOSE:
 *      Clear port block state.
 *
 * INPUT:
 *      port             - port id
 *      oper_block       - state
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
stp_db_clear_all_port_block(
    void);

/* FUNCTION NAME: stp_db_port_mcheck_update
 * PURPOSE:
 *      update mcheck.
 *
 * INPUT:
 *      port             - port id
 *      perform          - mcheck status
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
stp_db_port_mcheck_update(
    UI32_T    port,
    UI8_T     perform);

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_db_port_update_inconsistent
 * PURPOSE:
 *      Update port inconsistent state to db.
 *
 * INPUT:
 *      port           -  Port ID.
 *      state          -  inconsistent state.
 *      inc_type       -  root-inc/loop-inc.
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
rstp_db_port_update_inconsistent(
    const UI32_T port,
    const UI8_T  state,
    const UI8_T  inc_type);
#endif
/* FUNCTION NAME: stp_db_port_mode_update
 * PURPOSE:
 *      Send to DB to update port mode
 *
 * INPUT:
 *      portMode       -  port mode bmp
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
stp_db_port_mode_update(
    const UI32_T    portMode);

/* FUNCTION NAME: stp_db_subscribe
 * PURPOSE:
 *      Subscribe db msg for stp.
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
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
void
stp_db_subscribe(
    void);

/* FUNCTION NAME: stp_db_queue_handle
 * PURPOSE:
 *      Handle db msg for stp.
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
stp_db_queue_handle(
    DB_MSG_T    *ptr_msg);

#endif
