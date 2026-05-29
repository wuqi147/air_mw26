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

#ifndef STP_H
#define STP_H

/* FILE NAME:   stp.h
 * PURPOSE:
 *      This file defines the data structure for stp.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "default_config.h"
#include "mw_msg.h"
#include "osapi.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
/*#define STP_DEBUG*/
/*#define RSTP_DEBUG_PROFILING*/
/*#define RSTP_DEBUG_STATE_MACHINE */
/*#define RSTP_DEBUG_BPDU */
#define STP_MODULE_NAME            "stp"
#define STP_QUEUE_NAME             "rsq"
#define STP_DB_QUEUE_NAME_CMD      "rcm"
#define STP_DB_QUEUE_NAME_EMPTY    ""
#define STP_TIMER_NAME             "stp_tmr"

#define STP_QUEUE_LENGTH           (STP_QUEUE_LENGTH_PTK + STP_QUEUE_LENGTH_DB)
#define STP_QUEUE_DYN_LENGTH       (1)
#define STP_STACK_SIZE             (512)
#define STP_MSG_SIZE               (sizeof(MW_MSG_T *))
#define STP_TASK_DELAY             (10)
#define STP_DB_DELAY               (0xFFFFFFFF)
#define STP_READY_DELAY            (10)
#define STP_TIMER_PERIOD           (1000)  /* unit:millisecond */
#define STP_WDOG_KICK_NUM          (50)
#define STP_POLLING_INTERVAL       (100)   /* 100 ms */
#define STP_DEFAULT_PRIORITY       (32768)

#define STP_FLUSH_MAC_TIMER_MASK   (0x7f)

#define STP_TRUNK_ID_TO_PORT_NUMBER(__trunk_id__)      (PLAT_MAX_PORT_NUM + (__trunk_id__ + 1))
#define STP_PORT_NUMBER_TO_TRUNK_ID(__port_number__)   (__port_number__ - PLAT_MAX_PORT_NUM - 1)
#define STP_MAC_IS_MCAST(mac)                 ((mac[0] == 0x01) && (mac[1] == 0x00) && (mac[2] == 0x5e))
#define MSTP_INSTANCE_GET_ID(instance)        ((instance) & 0xfff)
#define MSTP_INSTANCE_GET_FID(instance)       (((instance) & 0xf000) >> 12)

#define stp_check_bool(param) ((param) == TRUE || (param) == FALSE)
#define stp_check_range(param, min, max) (((param) >= min && (param) <= max) == TRUE)

/* MACRO FUNCTION DECLARATIONS
 */
/* STP_DEBUG_LOG */
#define STP_LOG_ERR(fmt, ...)   MW_LOG_ERROR(STP, "%s[%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define STP_LOG_WARN(fmt, ...)  MW_LOG_WARN(STP, "%s[%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define STP_LOG_INFO(fmt, ...)  MW_LOG_INFO(STP, fmt"\n", ##__VA_ARGS__)
#define STP_LOG_DBG(fmt, ...)   MW_LOG_DEBUG(STP, fmt"\n", ##__VA_ARGS__)

#ifdef STP_DEBUG_PROFILING
#define STP_PROFILING_INIT()             UI32_T tickstart = 0, tickend = 0
#define STP_PROFILING_START()            tickstart = get_cpu_timer_cnt(0)
#define STP_PROFILING_END(msg) do { \
        tickend = get_cpu_timer_cnt(0); \
        MW_LOG_PRINTF("%s: execute %u ticks(%u ms)\n", msg, (tickend - tickstart), (tickend - tickstart) / (CPUTMRCLK / KHz)); \
    } while(0)
#else
#define STP_PROFILING_INIT()
#define STP_PROFILING_START()
#define STP_PROFILING_END(msg)
#endif

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    NET_FILTER_T        pkt_netf;
    staticMsghandle_t   msgQueueStruct; /* Task queue is a static queue. */
    UI8_T               msgQueueStorage[STP_MSG_SIZE * STP_QUEUE_LENGTH];
} STP_QUEUE_CONTEXT_T;

typedef struct
{
    UI8_T state[MSTP_INSTANCE_MAX_NUM];
} STP_PORT_STATE_T;

typedef struct
{
    UI8_T               stp_tick;
    UI8_T               port_state_changed;
    STP_PORT_STATE_T    *ptr_state;
} STP_SYS_CONTEXT_T;

typedef struct
{
    UI8_T           is_flush:1,
                    flush_time:7;
} __attribute__((packed)) STP_FLUSH_MAC_INFO_T;

typedef struct
{
    STP_FLUSH_MAC_INFO_T portInfo[MAX_MSTP_INSTANCE_NUM];
} __attribute__((packed)) STP_FLUSH_MAC_T;

enum stp_force_version {
    STP_VERSION_COMPATIBILITY = 0,
    STP_VERSION_DEFAULT = 2,
    STP_VERSION_MSTP = 3,

    STP_VERSION_LAST
};

enum stp_state {
    STP_DISABLED,
    STP_LEARNING,
    STP_FORWARDING,
    STP_DISCARDING,
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    STP_ROOT_INC,
    STP_LOOP_INC,
    STP_GUARD_DOWN,
#endif

    STP_LAST
};

enum stp_port_role {
    STP_ROLE_DISABLED,
    STP_ROLE_ROOT,
    STP_ROLE_DESIGNATED,
    STP_ROLE_ALTERNATE,
    STP_ROLE_BACKUP,
    STP_ROLE_MASTER,

    STP_ROLE_LAST,
};

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

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_speed_enum_to_integer
 * PURPOSE:
 *      Transfer speed
 *
 * INPUT:
 *      speed_enum          -  port speed value
 *      ptr_speed_int       -  speed num
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
stp_speed_enum_to_integer(
    const AIR_PORT_SPEED_T  speed_enum,
    UI32_T                  *ptr_speed_int);
#endif

/* FUNCTION NAME: stp_port_instance_state_update
 * PURPOSE:
 *      Update port instance state
 *
 * INPUT:
 *      port               -  port id
 *      state              -  port state
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
stp_port_state_data_update(
    UI32_T port,
    UI8_T  state);

/* FUNCTION NAME: mstp_port_instance_state_update
 * PURPOSE:
 *      Update port instance state
 *
 * INPUT:
 *      instance           -  instance
 *      port               -  port id
 *      state              -  port state
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
mstp_port_instance_state_update(
    UI32_T instance,
    UI32_T port,
    UI8_T  state);

/* FUNCTION NAME: stp_bpdu_rate_limit_ctrl
 * PURPOSE:
 *      Setting bpdu rate limit
 *
 * INPUT:
 *      enable           -  TRUE/FALSE
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_bpdu_rate_limit_ctrl(
    UI8_T enable);

/* FUNCTION NAME: stp_fdb_update_mac_entry_timer_by_port
 * PURPOSE:
 *      Update timer of MAC entry learned on specified port
 *
 * INPUT:
 *      port           -  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_fdb_update_mac_entry_timer_by_port(
    UI32_T  port,
    UI16_T  timer);

/* FUNCTION NAME: stp_sys_deinit
 * PURPOSE:
 *      Deinit STP function
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
 *      None
 */
MW_ERROR_NO_T
stp_sys_deinit(
    void);

/* FUNCTION NAME: stp_sys_init
 * PURPOSE:
 *      Init STP function
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
 *      None
 */
MW_ERROR_NO_T
stp_sys_init(
    void);

#ifdef STP_DEBUG
MW_ERROR_NO_T
stp_cmd_showUsedEntry(
    void);

MW_ERROR_NO_T
stp_cmd_stm_port_debug(
    UI32_T  port,
    UI8_T   enable);

unsigned char
stp_get_stm_dbg_flag(
    unsigned int portIdx);
#endif

#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
/* FUNCTION NAME: stp_cmd_setClearMulticastSupportMode
 * PURPOSE:
 *      Set support mode to clear multicast group when topology change is happended.
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setClearMulticastSupportMode(
    const UI8_T     mode);

/* FUNCTION NAME: rstp_remove_multicast_group
 * PURPOSE:
 *      Remove multicast group with specified port
 *
 * INPUT:
 *      port           -  port number
 *      timer          -  new timer for the MAC entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_remove_multicast_group(
    UI32_T  port);
#endif

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
/* FUNCTION NAME: stp_get_unified_sa_support
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - ptr_support
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_get_unified_sa_support(
    UI8_T   *ptr_support);
#endif

/* FUNCTION NAME: stp_cmd_setEnable
 * PURPOSE:
 *      Set global state
 *
 * INPUT:
 *      enable         -  0: disable; 1: enable
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
stp_cmd_setEnable(
    const UI8_T enable);

/* FUNCTION NAME: stp_cmd_setForceVersion
 * PURPOSE:
 *      Set force version
 *
 * INPUT:
 *      force_version  -  0: STP; 2: RSTP
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
stp_cmd_setForceVersion(
    const UI32_T force_version);

/* FUNCTION NAME: stp_cmd_setForwardDelay
 * PURPOSE:
 *      Set forward delay
 *
 * INPUT:
 *      forward_delay  -  forward delay 4~30 sec
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
stp_cmd_setForwardDelay(
    const UI16_T forward_delay);

/* FUNCTION NAME: stp_cmd_getForwardDelay
 * PURPOSE:
 *      Get forward delay
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_fwd_delay  -  forward_delay
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
stp_cmd_getForwardDelay(
    UI16_T  *ptr_fwd_delay);

/* FUNCTION NAME: stp_cmd_setMaxAge
 * PURPOSE:
 *      Set max age
 *
 * INPUT:
 *      max_age        -  max age, 6~40
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
stp_cmd_setMaxAge(
    const UI16_T max_age);

/* FUNCTION NAME: stp_cmd_getMaxAge
 * PURPOSE:
 *      Get max age
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_max_age    -  max age
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
stp_cmd_getMaxAge(
    UI16_T  *ptr_max_age);

/* FUNCTION NAME: stp_cmd_setTransmitHoldCount
 * PURPOSE:
 *      Set transmit hold count
 *
 * INPUT:
 *      txHoldCnt      -  transmit hold count, 1~10
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
stp_cmd_setTransmitHoldCount(
    const UI16_T txHoldCnt);

/* FUNCTION NAME: stp_cmd_setBridgePriority
 * PURPOSE:
 *      Set bridge priority
 *
 * INPUT:
 *      priority       -  priority, 0~61440 in step 4096
 *      instance       -  4bits-->fid, 12bits-->instance
 *      exist_ins      -  instance is valid
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
stp_cmd_setBridgePriority(
    const UI16_T priority,
    const UI16_T instance,
    const BOOL_T exist_ins);

/* FUNCTION NAME: rstp_cmd_setPortPriority
 * PURPOSE:
 *      Set port priority
 *
 * INPUT:
 *      port           -  port number
 *      priority       -  priority, 0~240 in step 16
 *      instance       -  4bits-->fid, 12bits-->instance
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
stp_cmd_setPortPriority(
    const UI32_T    port,
    const UI8_T     priority,
    const UI16_T    instance,
    const BOOL_T    exist_ins);

/* FUNCTION NAME: stp_cmd_setPortCost
 * PURPOSE:
 *      Set port cost
 *
 * INPUT:
 *      port           -  port number
 *      cost           -  cost, 1~200000000
 *      instance       -  4bits-->fid, 12bits-->instance
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
stp_cmd_setPortCost(
    const UI32_T    port,
    const UI32_T    cost,
    const UI16_T    instance,
    const BOOL_T    exist_ins);

/* FUNCTION NAME: rstp_cmd_setPortAdminEdge
 * PURPOSE:
 *      Set port admin edge
 *
 * INPUT:
 *      port           -  port number
 *      enable         -  0: disable; 1: enable
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
stp_cmd_setPortAdminEdge(
    const UI32_T    port,
    const UI8_T     admin_edge);

/* FUNCTION NAME: stp_cmd_setPortMCheck
 * PURPOSE:
 *      Set port mcheck
 *
 * INPUT:
 *      port           -  port number
 *      perform        -  0: do not trigger mcheck; 1: trigger mcheck
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
stp_cmd_setPortMCheck(
    const UI32_T    port,
    const UI8_T     perform);

/* FUNCTION NAME: stp_cmd_setPortEnableStatus
 * PURPOSE:
 *      Set port enable status
 *
 * INPUT:
 *      port           -  port number
 *      status         -  enable, disable
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
stp_cmd_setPortEnableStatus(
    const UI32_T    port,
    const UI8_T     status);

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_cmd_setPortAutoCostMode
 * PURPOSE:
 *      Set port auto cost mode
 *
 * INPUT:
 *      port           -  port number
 *      mode           -  auto cost mode
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
stp_cmd_setPortAutoCostMode(
    const UI32_T    port,
    const UI8_T     mode);

/* FUNCTION NAME: stp_cmd_setAutoCostSupportMode
 * PURPOSE:
 *      Set port auto cost support mode
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setAutoCostSupportMode(
    const UI8_T     mode);
#endif

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
/* FUNCTION NAME: stp_cmd_setUnifiedSASupportMode
 * PURPOSE:
 *      Set unified SA of BPDU support mode
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setUnifiedSASupportMode(
    const UI8_T     mode);
#endif

#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
/* FUNCTION NAME: stp_cmd_setClearMulticastSupportMode
 * PURPOSE:
 *      Set support mode to clear multicast group when topology change is happended.
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setClearMulticastSupportMode(
    const UI8_T     mode);
#endif

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_cmd_setPortBpduGuardState
 * PURPOSE:
 *      Set port bpdu guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  bpdu guard enable state
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
rstp_cmd_setPortBpduGuardState(
    const UI32_T    port,
    const UI8_T     state);

/* FUNCTION NAME: rstp_cmd_setPortTcGuardState
 * PURPOSE:
 *      Set port tc guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  tc guard enable state
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
rstp_cmd_setPortTcGuardState(
    const UI32_T    port,
    const UI8_T     state);

/* FUNCTION NAME: rstp_cmd_setPortTcGuardInterval
 * PURPOSE:
 *      Set port tc guard interval
 *
 * INPUT:
 *      port           -  port number
 *      interval       -  tc guard interval
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
rstp_cmd_setPortTcGuardInterval(
    const UI32_T    port,
    const UI16_T     interval);

/* FUNCTION NAME: rstp_cmd_setPortTcGuardThreshold
 * PURPOSE:
 *      Set port tc guard threshold
 *
 * INPUT:
 *      port           -  port number
 *      threshold       -  tc guard threshold
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
rstp_cmd_setPortTcGuardThreshold(
    const UI32_T    port,
    const UI16_T     threshold);

/* FUNCTION NAME: rstp_cmd_setPortRootGuardState
 * PURPOSE:
 *      Set port root guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  root guard enable state
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
rstp_cmd_setPortRootGuardState(
    const UI32_T    port,
    const UI8_T     state);

/* FUNCTION NAME: rstp_cmd_setPortLoopGuardState
 * PURPOSE:
 *      Set port loop guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  loop guard enable state
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
rstp_cmd_setPortLoopGuardState(
    const UI32_T    port,
    const UI8_T     state);

/* FUNCTION NAME: rstp_cmd_showSecInfo
 * PURPOSE:
 *      Show security info
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
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_cmd_showSecInfo(
    void);

#endif

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: mstp_cmd_setRegionConfig
 * PURPOSE:
 *      Set region name
 *
 * INPUT:
 *      ptr_name    - region name
 *      revision    - revision id
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
mstp_cmd_setRegionConfig(
    const UI8_T    *ptr_name,
    const UI16_T    revision);

#ifdef STP_DEBUG
/* FUNCTION NAME: mstp_cmd_setInstanceVlan
 * PURPOSE:
 *      Set instance mapping vlan
 *
 * INPUT:
 *      instance        - instance id
 *      ptr_vlanlist    - instance mapping vlan list
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
mstp_cmd_setInstanceVlan(
    const UI16_T    instance,
    const UI8_T     *ptr_vlanlist);
#endif

/* FUNCTION NAME: mstp_cmd_showMcidInfo
 * PURPOSE:
 *      Show mcid info
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
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_cmd_showMcidInfo(
    void);

/* FUNCTION NAME: mstp_cmd_showInstancePort
 * PURPOSE:
 *      Show instance port param
 *
 * INPUT:
 *      fid                 - instance index
 *      portId              - port id
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
mstp_cmd_showInstancePort(
    UI32_T  portId,
    UI32_T  fid);
#endif

/* FUNCTION NAME: stp_cmd_showInfo
 * PURPOSE:
 *      Show info
 *
 * INPUT:
 *      instance    -  instance id
 *      exsit_fid   - fid is valid
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
stp_cmd_showInfo(
    UI32_T  instance,
    BOOL_T  exsit_fid);

#endif
