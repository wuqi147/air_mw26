/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  poe_info.h
 * PURPOSE:
 *      The data handling in PoE application.
 * NOTES:
 */

#ifndef POE_INFO_H
#define POE_INFO_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_port.h>
#include <poe_api.h>
#include <osapi.h>
#include <switch.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define POE_PORT_INVALID    (0xFF)
#define POE_TOTAL_PORTS_NUM (AIR_MAX_PORT_NUM)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    POE_SYS_STATUS_ABNORMAL = 0,
    POE_SYS_STATUS_NORMAL,
    POE_SYS_STATUS_LAST
} POE_SYS_STATUS_T;

typedef struct POE_DEVICE_MAP_S
{
    UI16_T device_id;
} POE_DEVICE_MAP_T;

typedef struct PRI_PORT_INFO_S
{
    UI8_T unit;
    UI8_T air_port;
} PRI_PORT_INFO_T;

typedef struct POE_PORT_MAP_S
{
    /* config data */
    POE_PORT_TYPE_T     type;
    BOOL_T              pse_control;
    UI32_T              available_power;    /* gain factor is 0.1W */
    UI8_T               device_id;
    UI8_T               delay_time;         /* gain factor is 1s */
    UI8_T               event_restart_time; /* gain factor is 1s */
#ifdef AIR_SUPPORT_POE_WATCHDOG
    UI64_T              rx_pkt;
    UI64_T              rx_pkt_diff;
    UI32_T              poe_watchdog_timer;
    BOOL_T              poe_watchdog_mode;
    BOOL_T              poe_watchdog_pre_mode;
#endif /* AIR_SUPPORT_POE_WATCHDOG */
    /* status data */
    POE_PORT_PSE_MODE_T pse_mode;
    UI16_T              delay_count;
    UI16_T              restart_count;
    UI16_T              event_flags;
} POE_PORT_MAP_T;

typedef struct POE_CHIP_INFO_S
{
    /* config data */
    POE_PORT_MAP_T   *ptr_port_map_info[POE_TOTAL_PORTS_NUM];
    UI8_T             device_pbmp;
    AIR_PORT_BITMAP_T low_pbmp;
    AIR_PORT_BITMAP_T high_pbmp;
    AIR_PORT_BITMAP_T critical_pbmp;
    /* status data */
    AIR_PORT_BITMAP_T delay_done_pbmp;
    AIR_PORT_BITMAP_T port_ready_pbmp;
    AIR_PORT_BITMAP_T pwr_insufficient_pbmp;
    AIR_PORT_BITMAP_T event_restart_pbmp;
    AIR_PORT_BITMAP_T pwr_status_on_pbmp;
#ifdef AIR_SUPPORT_POE_WATCHDOG
    AIR_PORT_BITMAP_T wd_pwr_status_on_pbmp;
#endif /* AIR_SUPPORT_POE_WATCHDOG */
} POE_CHIP_INFO_T;

typedef struct POE_CB_S
{
    /* config data */
    UI8_T                  inited;
    UI8_T                  total_port_cnt;
    UI8_T                  total_device_cnt;
    POE_CHIP_INFO_T       *ptr_chip_info[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
    POE_SYS_POWER_MODE_T   system_power_mode;
    POE_PWR_CONTROL_T      power_strategy_control;
    UI32_T                 total_available_power; /* gain factor is 0.1W */
#ifdef AIR_SUPPORT_POE_WATCHDOG
    UI32_T                 poe_watchdog_period;
    UI16_T                 poe_watchdog_threshold;
#endif /* AIR_SUPPORT_POE_WATCHDOG */
    /* status data */
    UI8_T                  pri_plug_list_cnt;
    UI8_T                  poe_hw_reset_cnt;
    UI8_T                  poe_init_ctrl_module_cnt;
    UI32_T                 total_allocated_power;
    UI32_T                 total_required_power;
    UI32_T                 total_pd_req_power;
    BOOL_T                 system_suspend_mode;
    BOOL_T                 hw_reset_done;
    BOOL_T                 reset_init_done;
    POE_SYS_STATUS_T       pre_system_status;
    PRI_PORT_INFO_T       *ptr_pri_port_list;
    PRI_PORT_INFO_T       *ptr_pri_plug_list;
#ifdef AIR_SUPPORT_POE_WATCHDOG
    BOOL_T                 poe_watchdog_updated;
    TickType_t             poe_mib_counter_dur;
#endif /* AIR_SUPPORT_POE_WATCHDOG */
    UI8_T                  max_led_counter;
    BOOL_T                 max_led_solid_flag;
    TickType_t             poe_periodic_update_dur;
} POE_CB_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif /* POE_INFO_H */
