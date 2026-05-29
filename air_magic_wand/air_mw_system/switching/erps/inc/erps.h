/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  erps.h
 * PURPOSE:
 *      This file defines the data structure for ERPS.
 *
 * NOTES:
 */
#ifndef ERPS_H
#define ERPS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "osapi_string.h"
#include <mw_utils.h>
#include "db_api.h"
#include "mw_msg.h"
#include "erps_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ERPS_TASK_STACK_SIZE                (404)
#define ERPS_TASK_NAME                      "ERPS"
#define ERPS_TASK_DELAY                     (10)
#define ERPS_INFINITE_DELAY                 (0xFFFFFFFF) /* infinity */
#define ERPS_POLLING_DELAY                  (500)     /* 500 ms */
#define ERPS_DOS_RATE_LIMIT_PKT_THLD        (30)

#define ERPS_ETHTYPE_RAPS                   (0x8902)
#define ERPS_RAPS_OP_CODE                   (40)
#define ERPS_RAPS_MEL                       (7)
/* R-APS message status */
#define ERPS_RAPS_STATUS_BPR                BIT(5)
#define ERPS_RAPS_STATUS_DNF                BIT(6)
#define ERPS_RAPS_STATUS_RB                 BIT(7)
#define ERPS_SEND_RAPS_MSG_INTERVAL         (5) /* in seconds */

#define ERPS_RAPS_TX_BUFFER_LEN             (64)
#define ERPS_RAPS_NODE_ID_LEN               (6)

#define ERPS_MAX_INSTANCE_NUM               (MAX_ERPS_INSTANCE_NUM)
#define ERPS_INVALID_INSTANCE_IDX           (0xFF)
#define ERPS_DATA_VLAN_SIZE                 (MAX_ERPS_DATA_VLAN_SIZE)

/* MACRO FUNCTION DECLARATIONS
 */
#define ERPS_RING_ID_IS_VALID(ring_id)               \
            (((ring_id) > 0) && ((ring_id) <= 0xFF))

#define ERPS_PORT_GET_ROLE(ptr_port)               \
            (((ptr_port)->attr_role ) & (0xF))

#define ERPS_PORT_GET_ATTR(ptr_port)               \
            ((((ptr_port)->attr_role ) >> 4) & (0xF))

#define ERPS_PORT_SET_ROLE(ptr_port, role)         \
            (((ptr_port)->attr_role ) &= (0xF0))   \
            (((ptr_port)->attr_role ) |= (role & 0xF))

#define ERPS_PORT_SET_ATTR(ptr_port, attr)    \
            (((ptr_port)->attr_role ) &= (0x0F))   \
            (((ptr_port)->attr_role ) |= ((attr << 4) & 0xF0))

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    ERPS_PORT_STATE_FORWARD = 0,
    ERPS_PORT_STATE_DISCARD,

    ERPS_PORT_STATE_LAST
} ERPS_PORT_STATE_T;

typedef enum
{
    ERPS_ROLE_TRANSFER = 0,
    ERPS_ROLE_NEIGHBOR,
    ERPS_ROLE_OWNER,

    ERPS_PORT_ROLE_LAST
} ERPS_ROLE_T;

typedef enum
{
    ERPS_PORT_ATTR_WEST_PORT = 0,
    ERPS_PORT_ATTR_EAST_PORT,

    ERPS_PORT_ATTR_LAST
} ERPS_PORT_ATTR_T;

/* ring requests, as defined in table 10-1 */
typedef enum
{
    ERPS_RING_REQ_CLEAR = 0,
    ERPS_RING_REQ_FS,
    ERPS_RING_REQ_RAPS_FS,
    ERPS_RING_REQ_SF,
    ERPS_RING_REQ_CLEAR_SF,
    ERPS_RING_REQ_RAPS_SF,
    ERPS_RING_REQ_RAPS_MS,
    ERPS_RING_REQ_MS,
    ERPS_RING_REQ_WTR_EXPIRES,
    ERPS_RING_REQ_WTR_RUNNING,
    ERPS_RING_REQ_WTB_EXPIRES,
    ERPS_RING_REQ_WTB_RUNNING,
    ERPS_RING_REQ_RAPS_NR_RB,
    ERPS_RING_REQ_RAPS_NR,

    ERPS_RING_REQ_LAST
}ERPS_RING_REQUEST_T;

typedef enum
{
    ERPS_RING_STATE_IDLE = 0,
    ERPS_RING_STATE_PROTECTION,
    ERPS_RING_STATE_MANUAL_SWITCH,
    ERPS_RING_STATE_FORCED_SWITCH,
    ERPS_RING_STATE_PENDING,
    ERPS_RING_STATE_INIT,

    ERPS_RING_STATE_LAST = ERPS_RING_STATE_INIT,

} ERPS_RING_STATE_T;

typedef enum
{
    ERPS_RAPS_REQ_NR    = 0b0000,
    ERPS_RAPS_REQ_MS    = 0b0111,
    ERPS_RAPS_REQ_SF    = 0b1011,
    ERPS_RAPS_REQ_FS    = 0b1101,
    ERPS_RAPS_REQ_EVENT = 0b1110,
}ERPS_RAPS_REQUEST_T;

typedef struct ERPS_NODE_ID_BPR_S
{
    UI8_T                       node_id[ERPS_RAPS_NODE_ID_LEN];
    UI8_T                       bpr;                            /* Block port reference */
} ERPS_NODE_ID_BPR_T;

typedef struct ERPS_PORT_S
{
    UI8_T                       port_id;
    UI8_T                       attr_role;                  /* High 4Bits: port attribution(west port or east port), Low 4Bits: port role(owner, neighber, transfer) */
    UI8_T                       is_failed;
    UI8_T                       port_state;                 /* Forwarding, Discarding */
    ERPS_NODE_ID_BPR_T          node_id_bpr;
} ERPS_PORT_T;

typedef struct ERPS_RAPS_INFO_S
{
    ERPS_NODE_ID_BPR_T          node_id_bpr;                /* Node ID, BPR pair */
    UI8_T                       status;                     /* R-APS msg status */
    UI8_T                       recv_port;                  /* Recv port ID */
} ERPS_RAPS_INFO_T;

typedef struct ERPS_RING_S
{
    UI8_T                       ring_id;
    UI8_T                       revertive_mode;
    UI16_T                      raps_vid;
    C8_T                        data_vid[ERPS_DATA_VLAN_SIZE];/* e.g. 1,4,6-9,17-23 */
    UI32_T                      ring_acl_id;
    ERPS_PORT_T                 west_port;
    ERPS_PORT_T                 east_port;
    ERPS_PORT_T                 *ptr_rpl_port;
    UI8_T                       node_type;
    UI8_T                       ring_state;
    UI8_T                       node_id[ERPS_RAPS_NODE_ID_LEN];
    /* timer configuration */
    UI8_T                       guard_time;                 /* Unit: 10ms */
    UI8_T                       guard_timer_idx;            /* Index of guard timer in _erps_timer_instance[], 0xFF means timer in not running */
    UI8_T                       wtr_time;                   /* Unit: 1min */
    UI8_T                       wtr_timer_idx;              /* Index of WTR timer in _erps_timer_instance[], 0xFF means timer in not running */
    UI8_T                       wtb_timer_idx;              /* Index of WTB timer in _erps_timer_instance[], 0xFF means timer in not running */
    UI8_T                       hold_off_time;              /* Unit: 100ms */
    UI8_T                       hold_off_timer_idx;         /* Index of hold-off timer in _erps_timer_instance[], 0xFF means timer in not running */
    UI8_T                       raps_status;                /* R-APS msg status */
    ERPS_RAPS_REQUEST_T         raps_request;               /* R-APS msg request */
    UI8_T                       is_send_raps;
    UI8_T                       send_raps_time_remain;      /* Timer remain to send R-APS msg, unit: 1s */
    ERPS_RING_REQUEST_T         local_request;              /* Local request, top priority local request */
    UI16_T                      db_idx;
} ERPS_RING_T;

typedef struct ERPS_RAPS_MSG_S
{
    UI8_T                       mel_ver;            /* MEL: High 3bit, Version: Low 5bit */
    UI8_T                       op_code;            /* R-APS(0x28) */
    UI8_T                       flags;              /* Flags(0) */
    UI8_T                       tlv_offset;         /* TLV Offset(32) */
    UI8_T                       req_state_sub_code; /* Request/State: High 4bit, Sub Code: Low 4bit */
    UI8_T                       status;
    UI8_T                       node_id[6];
    UI8_T                       reserved[24];       /* Reserverd 24 Byte */
    UI8_T                       end_tlv;            /* End TLV(0) */
} ERPS_RAPS_MSG_T;

typedef struct ERPS_ENTRY_INFO_S
{
    UI8_T       ring_id;
    UI16_T      control_vlan;
    UI8_T       data_vlan[MAX_ERPS_DATA_VLAN_SIZE];
    UI8_T       revertive_mode;
    UI8_T       west_port;
    UI8_T       east_port;
    UI8_T       wtr_timer;          /* in mins */
    UI8_T       guard_timer;        /* in 10ms */
    UI8_T       hold_off_timer;     /* in 100ms */
    UI8_T       west_port_state;
    UI8_T       east_port_state;
    UI8_T       instance_state;
} ATTRIBUTE_PACK ERPS_ENTRY_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   erps_init
 * PURPOSE:
 *      This ERPS init function.
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
erps_init(void);


/* FUNCTION NAME:   erps_resource_free
 * PURPOSE:
 *      Free the resources in ERPS init function.
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
erps_resource_free(void);

/* FUNCTION NAME:   erps_instance_list_search
 * PURPOSE:
 *      This API is used to search erps instance entries.
 *
 * INPUT:
 *      ring_id              -- ring id of the erps instance
 *      vlan_id              -- control vlan id of the erps instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to erps instance.
 *
 * NOTES:
 *      None
 */
ERPS_RING_T *
erps_instance_list_search(
    UI8_T           ring_id,
    UI16_T          vlan_id);

/* FUNCTION NAME:   erps_instance_get_by_index
 * PURPOSE:
 *      This API is used to get erps instance by index.
 *
 * INPUT:
 *      index                -- index of the erps instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to erps instance.
 *
 * NOTES:
 *      None
 */
ERPS_RING_T *
erps_instance_get_by_index(
    UI8_T           index);

/* FUNCTION NAME:   erps_port_interrupt_set
 * PURPOSE:
 *      This API is used to set port interrupt state.
 *
 * INPUT:
 *      portmap              -- bit map of ports which need to enable/disable interrupt
 *      is_enable            -- TRUE for enable, FALSE for disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK.
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_port_interrupt_set(
    AIR_PORT_BITMAP_T   portmap,
    UI8_T               is_enable);

/* FUNCTION NAME:   erps_check_buffer_is_raps
 * PURPOSE:
 *      Check if buffer is raps message or not.
 *
 * INPUT:
 *      ptr_raw_data         -- Pointer to raw data
 *      len                  -- Length of raw data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Address of RAPS message packet header if it's a valid RAPS message, otherwise NULL.
 *
 * NOTES:
 *      None
 */
UI8_T *
erps_check_buffer_is_raps(
    const void     *ptr_raw_data,
    UI32_T          len);

/* FUNCTION NAME:   erps_check_port_is_ring port
 * PURPOSE:
 *      Check if port is in ring.
 *
 * INPUT:
 *      port_id              -- Pength of raw data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_check_port_is_ring_port(
    const UI32_T    port_id);

/* FUNCTION NAME:   erps_fdb_flush
 * PURPOSE:
 *      This function is used to flush the FDB entry for the specified port.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      ptr_port             -- Pointer to the erps port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_fdb_flush(
    ERPS_RING_T     *ptr_ring,
    ERPS_PORT_T     *ptr_port);

/* FUNCTION NAME:   erps_port_block
 * PURPOSE:
 *      This function is used to block or unblock the specified port.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      ptr_port             -- Pointer to the erps port
 *      is_block             -- TRUE: block the port, FALSE: unblock the port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_port_block(
    ERPS_RING_T     *ptr_ring,
    ERPS_PORT_T     *ptr_port,
    UI8_T           is_block);

/* FUNCTION NAME:   erps_non_fail_port_unblock
 * PURPOSE:
 *      This function is used to unblock non fail erps ports.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_non_fail_port_unblock(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_ring_port_unblock
 * PURPOSE:
 *      This function is used to unblock erps ports.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_ring_port_unblock(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_wtr_timer_start
 * PURPOSE:
 *      This function is used to start WTR timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtr_timer_start(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_wtr_timer_stop
 * PURPOSE:
 *      This function is used to stop WTR timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtr_timer_stop(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_wtb_timer_start
 * PURPOSE:
 *      This function is used to start WTB timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtb_timer_start(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_wtb_timer_stop
 * PURPOSE:
 *      This function is used to stop WTB timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtb_timer_stop(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_hold_off_timer_start
 * PURPOSE:
 *      This function is used to start hold-off timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_hold_off_timer_start(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_hold_off_timer_stop
 * PURPOSE:
 *      This function is used to stop hold-off timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_hold_off_timer_stop(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_guard_timer_start
 * PURPOSE:
 *      This function is used to start guard timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_guard_timer_start(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_guard_timer_stop
 * PURPOSE:
 *      This function is used to stop guard timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_guard_timer_stop(
    ERPS_RING_T     *ptr_ring);

/* FUNCTION NAME:   erps_tx_start
 * PURPOSE:
 *      This function is used to start transmit on the ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
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
erps_tx_start(
    ERPS_RING_T             *ptr_ring);

/* FUNCTION NAME:   erps_tx_stop
 * PURPOSE:
 *      This function is used to stop transmit on the ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
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
erps_tx_stop(
    ERPS_RING_T             *ptr_ring);

/* FUNCTION NAME:   erps_raps_msg_send
 * PURPOSE:
 *      This function is used to send RAPS message.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      request              -- Request type
 *      status               -- Status type
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_raps_msg_send(
    ERPS_RING_T             *ptr_ring,
    ERPS_RAPS_REQUEST_T     request,
    UI8_T                   status);

/* FUNCTION NAME:   erps_ring_state_set
 * PURPOSE:
 *      This function is used to set the state of ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      state                -- State of ERPS ring
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
erps_ring_state_set(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_STATE_T       state);

/* FUNCTION NAME:   erps_available_instance_check
 * PURPOSE:
 *      This API is used to check if there is available erps instance.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_available_instance_check(
    void);

#endif  /* ERPS_H */
