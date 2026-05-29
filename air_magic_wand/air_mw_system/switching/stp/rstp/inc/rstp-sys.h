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

/* FILE NAME:  rstp-sys.h
 * PURPOSE:
 * It provides RSTP module API and definitions.
 *
 * NOTES:
 */

#ifndef RSTP_SYS_H
#define RSTP_SYS_H
/* INCLUDE FILE DECLARATIONS
 */
#include <mw_error.h>
#include <osapi_message.h>
#include <db_api.h>
#include "default_config.h"
#include "../ovs/lib/rstp.h"
#include "mw_msg.h"
#include "mw_cmd_util.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define RSTP_INFO_PRINT(cmd_flag, fmt, ...)          \
    do {                                        \
        if (TRUE == (cmd_flag))                 \
        {                                       \
            MW_CMD_OUTPUT(fmt, ##__VA_ARGS__);   \
        }                                       \
        else                                    \
        {                                       \
            VLOG_DBG(fmt, ##__VA_ARGS__);       \
        }                                       \
     } while (0)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: rstp_enable_set
 * PURPOSE:
 *      enable rstp
 *
 * INPUT:
 *      enable          -  status
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
rstp_enable_set(
    UI8_T   enable);

/* FUNCTION NAME: rstp_force_version_set
 * PURPOSE:
 *      switch to rstp
 *
 * INPUT:
 *      force_version          -  rstp/stp
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
rstp_force_version_set(
    UI32_T   force_version);

/* FUNCTION NAME: rstp_forward_delay_set
 * PURPOSE:
 *      Setting forward delay
 *
 * INPUT:
 *      forward_delay          -  fwd delay time
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
rstp_forward_delay_set(
    UI16_T  forward_delay);

/* FUNCTION NAME: rstp_max_age_set
 * PURPOSE:
 *      Setting max age
 *
 * INPUT:
 *      max_age          -  max age time
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
rstp_max_age_set(
    UI16_T   max_age);

/* FUNCTION NAME: rstp_transmit_hold_count_set
 * PURPOSE:
 *      Setting tx hold time
 *
 * INPUT:
 *      tx_hold_cnt          -  tx hold time
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
rstp_transmit_hold_count_set(
    UI16_T  tx_hold_cnt);

/* FUNCTION NAME: rstp_priority_set
 * PURPOSE:
 *      Setting bridge priority
 *
 * INPUT:
 *      priority          -  bridge priority
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
rstp_priority_set(
    UI16_T  priority);

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: rstp_port_compute_oper_cost
 * PURPOSE:
 *      Calc port cost
 *
 * INPUT:
 *      port          -  port id
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto cost support status
 *      ptr_oper_cost            - oper cost
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_compute_oper_cost(
    UI32_T  port,
    UI8_T  *ptr_auto_cost_support,
    UI32_T  *ptr_oper_cost);

/* FUNCTION NAME: rstp_auto_cost_support_set
 * PURPOSE:
 *      Set auto cost
 *
 * INPUT:
 *      enable          -  TRUE/FALSE
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
rstp_auto_cost_support_set(
    UI8_T   enable);

/* FUNCTION NAME: rstp_port_auto_cost_set
 * PURPOSE:
 *      Set port auto cost
 *
 * INPUT:
 *      port          -  port id
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
rstp_port_auto_cost_set(
    UI32_T  port);

/* FUNCTION NAME: rstp_set_port_auto_cost_config
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port          -  port id
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
rstp_set_port_auto_cost_config(
    UI32_T  port);

/* FUNCTION NAME: rstp_port_oper_cost_set
 * PURPOSE:
 *      Set port cost config
 *
 * INPUT:
 *      port          -  port id
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
rstp_port_oper_cost_set(
    UI32_T   port_number);
#endif
/* FUNCTION NAME: rstp_port_priority_set
 * PURPOSE:
 *      Set port priority config
 *
 * INPUT:
 *      port          -  port id
 *      priority      -  priority
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
rstp_port_priority_set(
    UI32_T  port,
    UI8_T   priority);

/* FUNCTION NAME: rstp_port_cost_set
 * PURPOSE:
 *      Set port cost
 *
 * INPUT:
 *      port          -  port id
 *      cost          -  port cost
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
rstp_port_cost_set(
    UI32_T  port,
    UI32_T   cost);

/* FUNCTION NAME: rstp_port_enable_status_set
 * PURPOSE:
 *      Set port status
 *
 * INPUT:
 *      port          -  port id
 *      status        -  status
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
rstp_port_enable_status_set(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: rstp_port_admin_edge_set
 * PURPOSE:
 *      Set port admin edge
 *
 * INPUT:
 *      port          -  port id
 *      admin_edge    -  admin edge
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
rstp_port_admin_edge_set(
    UI32_T  port,
    UI8_T   admin_edge);

/* FUNCTION NAME: rstp_port_mcheck_set
 * PURPOSE:
 *      Set port mcheck
 *
 * INPUT:
 *      port          -  port id
 *      perform       -  mcheck status
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
rstp_port_mcheck_set(
    UI32_T  port,
    UI8_T   perform);

/* FUNCTION NAME: rstp_trunk_update
 * PURPOSE:
 *      Set trunk info
 *
 * INPUT:
 *      e_idx          -  entry-id
 *      ptr_data       -  db msg data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_trunk_update(
    UI16_T e_idx,
    void   *ptr_data);

/* FUNCTION NAME: rstp_port_link_status_set
 * PURPOSE:
 *      Set port link status
 *
 * INPUT:
 *      port         -  port id
 *      is_p2p       -  is_p2p
 *      link         -  link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_link_status_set(
    UI32_T  port,
    UI8_T   is_p2p,
    UI8_T   link);

/* FUNCTION NAME: rstp_trunk_link_status_set
 * PURPOSE:
 *      Set trunk link status
 *
 * INPUT:
 *      port         -  port id
 *      link         -  link status
 *      mode         -  trunk mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_trunk_link_status_set(
    UI32_T  port,
    UI8_T   link,
    UI8_T   mode);

/* FUNCTION NAME: rstp_port_mac_operational_set
 * PURPOSE:
 *      Set port mac-operational status
 *
 * INPUT:
 *      port         -  port id
 *      link         -  link status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_mac_operational_set(
    UI32_T  port,
    UI8_T   link);

/* FUNCTION NAME: rstp_port_p2p_mac_set
 * PURPOSE:
 *      Set port point-to-point mac status
 *
 * INPUT:
 *      port         -  port id
 *      is_p2p       -  is_p2p
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_port_p2p_mac_set(
    UI32_T  port,
    UI8_T   is_p2p);

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_port_bpdu_status_set
 * PURPOSE:
 *      Set port bpdu protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_bpdu_status_set(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: rstp_port_tc_status_set
 * PURPOSE:
 *      Set port tc protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_status_set(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: rstp_port_root_guard_status_set
 * PURPOSE:
 *      Set port root protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_root_guard_status_set(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: rstp_port_loop_guard_status_set
 * PURPOSE:
 *      Set port loop protect status
 *
 * INPUT:
 *      port         -  port id
 *      status       -  enable status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_loop_guard_status_set(
    UI32_T  port,
    UI8_T   status);

/* FUNCTION NAME: rstp_port_tc_interval_set
 * PURPOSE:
 *      Set port tc protect interval
 *
 * INPUT:
 *      port         -  port id
 *      interval     -  interval time
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_interval_set(
    UI32_T  port,
    UI8_T   interval);

/* FUNCTION NAME: rstp_port_tc_threshold_set
 * PURPOSE:
 *      Set port tc protect threshold
 *
 * INPUT:
 *      port         -  port id
 *      threshold    -  threshold
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_port_tc_threshold_set(
    UI32_T  port,
    UI8_T   threshold);
#endif

/* FUNCTION NAME: rstp_pkt_rx
 * PURPOSE:
 *      rstp bpdu handle
 *
 * INPUT:
 *      port_num         -  port id
 *      ptr_data         -  bpdu data
 *      bpdu_size        -  bpdu size
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_pkt_rx(
    UI32_T  port_num,
    void    *ptr_data,
    UI32_T  bpdu_size);

/* FUNCTION NAME: rstp_data_init
 * PURPOSE:
 *      rstp data init
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void
 *
 * NOTES:
 *      None
 */
void
rstp_data_init(
    void);

#ifdef STP_DEBUG
/* FUNCTION NAME: rstp_cmd_showOvsInfo
 * PURPOSE:
 *      Show info
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
rstp_cmd_showOvsInfo(
    void);

MW_ERROR_NO_T
rstp_print_bridge_info(
    BOOL_T              cmd_flag);

MW_ERROR_NO_T
rstp_cmd_showPortParameters(
    UI32_T  port);

MW_ERROR_NO_T
rstp_cmd_showBridgeParameters(
    void);

MW_ERROR_NO_T
rstp_cmd_showUsedEntry(
    void);
#endif

void
rstp_tick_timers_start(
    void);

#endif /* End of RSTP_SYS_H */

