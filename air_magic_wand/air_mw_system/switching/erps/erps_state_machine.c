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

/* FILE NAME:  erps_state_machine.c
 * PURPOSE:
 *    This file contains the implementation of ERPS state machine.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "osapi.h"
#include    "erps.h"
#include    "erps_state_machine.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* State machine function */
static MW_ERROR_NO_T
_erps_raps_fs_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_fs_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_fs_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 46, _erps_raps_fs_fs_func: No action */

static MW_ERROR_NO_T
_erps_raps_fs_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_sf_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_sf_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_sf_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 47, _erps_sf_fs_func: No action */

static MW_ERROR_NO_T
_erps_sf_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 6, _erps_clr_sf_idle_func: No action */

static MW_ERROR_NO_T
_erps_clr_sf_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 34, _erps_clr_sf_ms_func: No action */

/* Table 10-2, row 48, _erps_clr_sf_fs_func: No action */

/* Table 10-2, row 62, _erps_clr_sf_pend_func: No action */

static MW_ERROR_NO_T
_erps_raps_sf_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 21, _erps_raps_sf_prot_func: No action */

static MW_ERROR_NO_T
_erps_raps_sf_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 49, _erps_raps_sf_fs_func: No action */

static MW_ERROR_NO_T
_erps_raps_sf_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_ms_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 22, _erps_raps_ms_prot_func: No action */

static MW_ERROR_NO_T
_erps_raps_ms_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 50, _erps_raps_ms_fs_func: No action */

static MW_ERROR_NO_T
_erps_raps_ms_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 10, _erps_wtr_exp_idle_func: No action */

/* Table 10-2, row 24, _erps_wtr_exp_prot_func: No action */

/* Table 10-2, row 38, _erps_wtr_exp_ms_func: No action */

/* Table 10-2, row 52, _erps_wtr_exp_fs_func: No action */

static MW_ERROR_NO_T
_erps_wtr_exp_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 11, _erps_wtr_run_idle_func: No action */

/* Table 10-2, row 25, _erps_wtr_run_prot_func: No action */

/* Table 10-2, row 39, _erps_wtr_run_ms_func: No action */

/* Table 10-2, row 53, _erps_wtr_run_fs_func: No action */

/* Table 10-2, row 67, _erps_wtr_run_pend_func: No action */

/* Table 10-2, row 12, _erps_wtb_exp_idle_func: No action */

/* Table 10-2, row 26, _erps_wtb_exp_prot_func: No action */

/* Table 10-2, row 40, _erps_wtb_exp_ms_func: No action */

/* Table 10-2, row 54, _erps_wtb_exp_fs_func: No action */

static MW_ERROR_NO_T
_erps_wtb_exp_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* Table 10-2, row 13, _erps_wtb_run_idle_func: No action */

/* Table 10-2, row 27, _erps_wtb_run_prot_func: No action */

/* Table 10-2, row 41, _erps_wtb_run_ms_func: No action */

/* Table 10-2, row 55, _erps_wtb_run_fs_func: No action */

/* Table 10-2, row 69, _erps_wtb_run_pend_func: No action */

static MW_ERROR_NO_T
_erps_raps_nr_nb_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_nb_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_nb_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_nb_fs_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_nb_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_fs_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_nr_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg);

/* STATIC VARIABLE DECLARATIONS
 */

const static ERPS_STATE_REQ_ACT_FUNC_T
_erps_state_req_act_func_vec[ERPS_RING_REQ_LAST][ERPS_RING_STATE_LAST] =
{
/*  Idle(row:2~15),              Protection(row:16~29),      Manual Switch(row:30~43)  Forced Switch(row:44~57)  Pending(row:58~71) */
    {NULL,/* Not support */      NULL,/* Not support */      NULL,/* Not support */    NULL,/* Not support */    NULL/* Not support */},      /* Clear */
    {NULL,/* Not support */      NULL,/* Not support */      NULL,/* Not support */    NULL,/* Not support */    NULL/* Not support */},      /* FS */
    {_erps_raps_fs_idle_func,    _erps_raps_fs_prot_func,    _erps_raps_fs_ms_func,    NULL,/* No Action   */    _erps_raps_fs_pend_func},    /* R-APS(FS) */
    {_erps_sf_idle_func,         _erps_sf_prot_func,         _erps_sf_ms_func,         NULL,/* No Action   */    _erps_sf_pend_func},         /* Local SF */
    {NULL,/* No Action   */      _erps_clr_sf_prot_func,     NULL,/* No Action   */    NULL,/* No Action   */    NULL,/* No Action   */},     /* Local Clear SF */
    {_erps_raps_sf_idle_func,    NULL,/* No Action   */      _erps_raps_sf_ms_func,    NULL,/* No Action   */    _erps_raps_sf_pend_func},    /* R-APS(SF) */
    {_erps_raps_ms_idle_func,    NULL,/* No Action   */      _erps_raps_ms_ms_func,    NULL,/* No Action   */    _erps_raps_ms_pend_func},    /* R-APS(MS) */
    {NULL,/* Not support */      NULL,/* Not support */      NULL,/* Not support */    NULL,/* Not support */    NULL/* Not support */},      /* MS */
    {NULL,/* No Action   */      NULL,/* No Action   */      NULL,/* No Action   */    NULL,/* No Action   */    _erps_wtr_exp_pend_func},    /* WTR Expires */
    {NULL,/* No Action   */      NULL,/* No Action   */      NULL,/* No Action   */    NULL,/* No Action   */    NULL/* Not support */},      /* WTR Running */
    {NULL,/* No Action   */      NULL,/* No Action   */      NULL,/* No Action   */    NULL,/* No Action   */    _erps_wtb_exp_pend_func},    /* WTB Expires */
    {NULL,/* No Action   */      NULL,/* No Action   */      NULL,/* No Action   */    NULL,/* No Action   */    NULL/* Not support */},      /* WTB Running */
    {_erps_raps_nr_nb_idle_func, _erps_raps_nr_nb_prot_func, _erps_raps_nr_nb_ms_func, _erps_raps_nr_nb_fs_func, _erps_raps_nr_nb_pend_func}, /* R-APS(NR, NB) */
    {_erps_raps_nr_idle_func,    _erps_raps_nr_prot_func,    _erps_raps_nr_ms_func,    _erps_raps_nr_fs_func,    _erps_raps_nr_pend_func},    /* R-APS(NR) */
};

/* LOCAL SUBPROGRAM BODIES
 */
/* Table 10-2, row 4 */
static MW_ERROR_NO_T
_erps_raps_fs_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_FORCED_SWITCH;

    UNUSED(ptr_arg);
    rc = erps_ring_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 18 */
static MW_ERROR_NO_T
_erps_raps_fs_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_raps_fs_idle_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 32 */
static MW_ERROR_NO_T
_erps_raps_fs_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_raps_fs_idle_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 60 */
static MW_ERROR_NO_T
_erps_raps_fs_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_FORCED_SWITCH;

    UNUSED(ptr_arg);
    rc = erps_ring_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        /* Stop WTR, WTB */
        erps_wtr_timer_stop(ptr_ring);
        erps_wtb_timer_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 5 */
static MW_ERROR_NO_T
_erps_sf_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    UI8_T                   status = 0;
    ERPS_PORT_T             *ptr_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PROTECTION;

    MW_CHECK_PTR(ptr_arg);
    ptr_ring->local_request = request;
    ptr_port = (ERPS_PORT_T *)ptr_arg;
    status = (ERPS_PORT_GET_ATTR(ptr_port) ? ERPS_RAPS_STATUS_BPR : 0);
    if(ERPS_PORT_STATE_DISCARD == ptr_port->port_state)
    {
        /* Failed port is already blocked */
        status |= ERPS_RAPS_STATUS_DNF;
        erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_SF, status);
        erps_tx_start(ptr_ring);
        rc |= erps_non_fail_port_unblock(ptr_ring);
    }
    else
    {
        /* Block failed port */
        rc |= erps_port_block(ptr_ring, ptr_port, TRUE);
        erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_SF, status);
        erps_tx_start(ptr_ring);
        rc |= erps_non_fail_port_unblock(ptr_ring);
        rc |= erps_fdb_flush(ptr_ring, ptr_port);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 19 */
static MW_ERROR_NO_T
_erps_sf_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_sf_idle_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 33 */
static MW_ERROR_NO_T
_erps_sf_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_sf_idle_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 61 */
static MW_ERROR_NO_T
_erps_sf_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    UI8_T                   status = 0;
    ERPS_PORT_T             *ptr_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PROTECTION;

    MW_CHECK_PTR(ptr_arg);
    ptr_ring->local_request = request;
    ptr_port = (ERPS_PORT_T *)ptr_arg;
    status = (ERPS_PORT_GET_ATTR(ptr_port) ? ERPS_RAPS_STATUS_BPR : 0);
    if(ERPS_PORT_STATE_DISCARD == ptr_port->port_state)
    {
        /* Failed port is already blocked */
        status |= ERPS_RAPS_STATUS_DNF;
        erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_SF, status);
        erps_tx_start(ptr_ring);
        rc |= erps_non_fail_port_unblock(ptr_ring);
    }
    else
    {
        /* Block failed port */
        rc |= erps_port_block(ptr_ring, ptr_port, TRUE);
        erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_SF, status);
        erps_tx_start(ptr_ring);
        rc |= erps_non_fail_port_unblock(ptr_ring);
        rc |= erps_fdb_flush(ptr_ring, ptr_port);
    }

    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        /* Stop WTR, WTB */
        erps_wtr_timer_stop(ptr_ring);
        erps_wtb_timer_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 20 */
static MW_ERROR_NO_T
_erps_clr_sf_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    UI8_T                   status = 0;
    ERPS_PORT_T             *ptr_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    MW_CHECK_PTR(ptr_arg);
    ptr_port = (ERPS_PORT_T *)ptr_arg;
    erps_guard_timer_start(ptr_ring);
    status = (ERPS_PORT_GET_ATTR(ptr_port) ? ERPS_RAPS_STATUS_BPR : 0);
    erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, status);
    erps_tx_start(ptr_ring);
    if((ERPS_ROLE_OWNER == ptr_ring->node_type) &&
        (TRUE == ptr_ring->revertive_mode))
    {
        erps_wtr_timer_start(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 7 */
static MW_ERROR_NO_T
_erps_raps_sf_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PROTECTION;

    UNUSED(ptr_arg);
    rc = erps_non_fail_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 35 */
static MW_ERROR_NO_T
_erps_raps_sf_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_raps_sf_idle_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 63 */
static MW_ERROR_NO_T
_erps_raps_sf_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PROTECTION;

    UNUSED(ptr_arg);
    rc = erps_non_fail_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        /* Stop WTR, WTB */
        erps_wtr_timer_stop(ptr_ring);
        erps_wtb_timer_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 8 */
static MW_ERROR_NO_T
_erps_raps_ms_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_MANUAL_SWITCH;

    UNUSED(ptr_arg);
    rc = erps_non_fail_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 36 */
static MW_ERROR_NO_T
_erps_raps_ms_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_MANUAL_SWITCH;


    if((ERPS_PORT_STATE_DISCARD == ptr_ring->west_port.port_state) ||
        (ERPS_PORT_STATE_DISCARD == ptr_ring->east_port.port_state))
    {
        /* If any ring port blocked */
        erps_guard_timer_start(ptr_ring);
        erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, 0);
        erps_tx_start(ptr_ring);
        if((ERPS_ROLE_OWNER == ptr_ring->node_type) &&
            (TRUE == ptr_ring->revertive_mode))
        {
            erps_wtb_timer_start(ptr_ring);
        }
        /* If both ring port are unblocked, next node state is manual switch */
        new_state = ERPS_RING_STATE_PENDING;
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* Table 10-2, row 64 */
static MW_ERROR_NO_T
_erps_raps_ms_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_MANUAL_SWITCH;

    UNUSED(ptr_arg);
    rc = erps_non_fail_port_unblock(ptr_ring);
    erps_tx_stop(ptr_ring);
    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        /* Stop WTR, WTB */
        erps_wtr_timer_stop(ptr_ring);
        erps_wtb_timer_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 66 */
static MW_ERROR_NO_T
_erps_wtr_exp_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    UI8_T                   status = 0;
    ERPS_PORT_T             *ptr_non_rpl_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_IDLE;

    UNUSED(ptr_arg);
    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        if(ERPS_PORT_ATTR_EAST_PORT == ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port))
        {
            ptr_non_rpl_port = &(ptr_ring->west_port);
        }
        else
        {
            ptr_non_rpl_port = &(ptr_ring->east_port);
        }
        /* Stop WTB */
        erps_wtb_timer_stop(ptr_ring);
        status = (ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port) ? ERPS_RAPS_STATUS_BPR : 0);
        if(ERPS_PORT_STATE_DISCARD == ptr_ring->ptr_rpl_port->port_state)
        {
            /* If RPL port is blocked */
            status |= (ERPS_RAPS_STATUS_DNF | ERPS_RAPS_STATUS_RB);
            erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, status);
            erps_tx_start(ptr_ring);
        }
        else
        {
            rc |= erps_port_block(ptr_ring, ptr_ring->ptr_rpl_port, TRUE);
            status |= ERPS_RAPS_STATUS_RB;
            erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, status);
            erps_tx_start(ptr_ring);
            rc |= erps_fdb_flush(ptr_ring, ptr_ring->ptr_rpl_port);
        }
        /* Unblock non-RPL port */
        rc |= erps_port_block(ptr_ring, ptr_non_rpl_port, FALSE);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 68 */
static MW_ERROR_NO_T
_erps_wtb_exp_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    UI8_T                   status = 0;
    ERPS_PORT_T             *ptr_non_rpl_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_IDLE;

    UNUSED(ptr_arg);
    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        if(ERPS_PORT_ATTR_EAST_PORT == ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port))
        {
            ptr_non_rpl_port = &(ptr_ring->west_port);
        }
        else
        {
            ptr_non_rpl_port = &(ptr_ring->east_port);
        }
        /* Stop WTR */
        erps_wtr_timer_stop(ptr_ring);
        status = (ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port) ? ERPS_RAPS_STATUS_BPR : 0);
        if(ERPS_PORT_STATE_DISCARD == ptr_ring->ptr_rpl_port->port_state)
        {
            /* If RPL port is blocked */
            status |= (ERPS_RAPS_STATUS_DNF | ERPS_RAPS_STATUS_RB);
            erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, status);
            erps_tx_start(ptr_ring);
        }
        else
        {
            rc |= erps_port_block(ptr_ring, ptr_ring->ptr_rpl_port, TRUE);
            status |= ERPS_RAPS_STATUS_RB;
            erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, status);
            erps_tx_start(ptr_ring);
            rc |= erps_fdb_flush(ptr_ring, ptr_ring->ptr_rpl_port);
        }
        /* Unblock non-RPL port */
        rc |= erps_port_block(ptr_ring, ptr_non_rpl_port, FALSE);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 14 */
static MW_ERROR_NO_T
_erps_raps_nr_nb_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_IDLE;

    UNUSED(ptr_arg);
    /* Unblock non-RPL port */
    if(ERPS_ROLE_TRANSFER == ERPS_PORT_GET_ROLE(&(ptr_ring->west_port)))
    {
        rc = erps_port_block(ptr_ring, &ptr_ring->west_port, FALSE);
    }
    if(ERPS_ROLE_TRANSFER == ERPS_PORT_GET_ROLE(&(ptr_ring->east_port)))
    {
        rc |= erps_port_block(ptr_ring, &ptr_ring->east_port, FALSE);
    }
    if(ERPS_ROLE_OWNER != ptr_ring->node_type)
    {
        erps_tx_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 28 */
static MW_ERROR_NO_T
_erps_raps_nr_nb_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    /* No action, only switch state */
    UNUSED(ptr_arg);
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 42 */
static MW_ERROR_NO_T
_erps_raps_nr_nb_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    /* No action, only switch state */
    return _erps_raps_nr_nb_prot_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 56 */
static MW_ERROR_NO_T
_erps_raps_nr_nb_fs_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    /* No action, only switch state */
    return _erps_raps_nr_nb_prot_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 70 */
static MW_ERROR_NO_T
_erps_raps_nr_nb_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    ERPS_PORT_T             *ptr_non_rpl_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_IDLE;

    UNUSED(ptr_arg);

    if(ERPS_ROLE_OWNER == ptr_ring->node_type)
    {
        /* Stop WTR, WTB */
        erps_wtr_timer_stop(ptr_ring);
        erps_wtb_timer_stop(ptr_ring);
    }
    else if(ERPS_ROLE_NEIGHBOR == ptr_ring->node_type)
    {
        if(ERPS_PORT_ATTR_EAST_PORT == ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port))
        {
            ptr_non_rpl_port = &(ptr_ring->west_port);
        }
        else
        {
            ptr_non_rpl_port = &(ptr_ring->east_port);
        }
        /* Block RPL port */
        rc |= erps_port_block(ptr_ring, ptr_ring->ptr_rpl_port, TRUE);
        /* Unblock non-RPL port */
        rc |= erps_port_block(ptr_ring, ptr_non_rpl_port, FALSE);
        erps_tx_stop(ptr_ring);
    }
    else
    {
        rc |=  erps_ring_port_unblock(ptr_ring);
        erps_tx_stop(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 15 */
static MW_ERROR_NO_T
_erps_raps_nr_idle_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_IDLE;

    MW_CHECK_PTR(ptr_arg);
    if(ERPS_ROLE_TRANSFER == ptr_ring->node_type)
    {
        if(0 > osapi_memcmp(ptr_ring->node_id, ptr_arg, ERPS_RAPS_NODE_ID_LEN))
        {
            /* Remote node ID is higher than own node ID */
            rc |=  erps_non_fail_port_unblock(ptr_ring);
            erps_tx_stop(ptr_ring);
        }
    }

    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 29 */
static MW_ERROR_NO_T
_erps_raps_nr_prot_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    if((ERPS_ROLE_OWNER == ptr_ring->node_type) &&
        (TRUE == ptr_ring->revertive_mode))
    {
        erps_wtr_timer_start(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 43 */
static MW_ERROR_NO_T
_erps_raps_nr_ms_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    if((ERPS_ROLE_OWNER == ptr_ring->node_type) &&
        (TRUE == ptr_ring->revertive_mode))
    {
        erps_wtb_timer_start(ptr_ring);
    }
    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* Table 10-2, row 57 */
static MW_ERROR_NO_T
_erps_raps_nr_fs_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    return _erps_raps_nr_ms_func(ptr_ring, request, ptr_arg);
}

/* Table 10-2, row 71 */
static MW_ERROR_NO_T
_erps_raps_nr_pend_func(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    MW_CHECK_PTR(ptr_arg);
    if(0 > osapi_memcmp(ptr_ring->node_id, ptr_arg, ERPS_RAPS_NODE_ID_LEN))
    {
        /* Remote node ID is higher than own node ID */
        rc |=  erps_non_fail_port_unblock(ptr_ring);
        erps_tx_stop(ptr_ring);
    }

    ERPS_LOG_DEBUG("request:%s, cur_state:%s, new_state:%s, rc:%d\n",
                        erps_record_ring_req_vec[request], erps_record_ring_state_vec[ptr_ring->ring_state], erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   erps_state_machine_init
 * PURPOSE:
 *      This function initializes the state machine of ERPS.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to ERPS ring
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
erps_state_machine_init(
    ERPS_RING_T             *ptr_ring)
{
    ERPS_PORT_T             *ptr_non_rpl_port = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    ERPS_RING_STATE_T       new_state = ERPS_RING_STATE_PENDING;

    ptr_ring->local_request = ERPS_RING_REQ_LAST;
    ptr_ring->west_port.port_state = ERPS_PORT_STATE_FORWARD;
    ptr_ring->east_port.port_state = ERPS_PORT_STATE_FORWARD;
    /* Stop WTR, WTB, guard timer */
    erps_wtr_timer_stop(ptr_ring);
    erps_wtb_timer_stop(ptr_ring);
    erps_guard_timer_stop(ptr_ring);
    erps_hold_off_timer_stop(ptr_ring);

    if((ERPS_ROLE_OWNER == ptr_ring->node_type) ||
       (ERPS_ROLE_NEIGHBOR == ptr_ring->node_type))
    {
        if(ERPS_PORT_ATTR_EAST_PORT == ERPS_PORT_GET_ATTR(ptr_ring->ptr_rpl_port))
        {
            ptr_non_rpl_port = &(ptr_ring->west_port);
        }
        else
        {
            ptr_non_rpl_port = &(ptr_ring->east_port);
        }
        /* Block RPL port */
        rc |= erps_port_block(ptr_ring, ptr_ring->ptr_rpl_port, TRUE);
        /* Unblock non-RPL port */
        rc |= erps_port_block(ptr_ring, ptr_non_rpl_port, FALSE);

        if((ERPS_ROLE_OWNER == ptr_ring->node_type) &&
           (TRUE == ptr_ring->revertive_mode))
        {
            erps_wtr_timer_start(ptr_ring);
        }
    }
    else
    {
        /*  Block one port, unblock other port */
        rc = erps_port_block(ptr_ring, &ptr_ring->west_port, TRUE);
        rc |= erps_port_block(ptr_ring, &ptr_ring->east_port, FALSE);
    }
    erps_raps_msg_send(ptr_ring, ERPS_RAPS_REQ_NR, 0);
    erps_tx_start(ptr_ring);

    ERPS_LOG_DEBUG("Instance init done, new_state:%s, rc:%d\n",
                        erps_record_ring_state_vec[new_state], rc);
    erps_ring_state_set(ptr_ring, new_state);

    return rc;
}

/* FUNCTION NAME:   erps_state_machine_handle
 * PURPOSE:
 *      This function handles the state machine of ERPS.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to ERPS ring
 *      request              -- Request type
 *      ptr_arg              -- Pointer to argument
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OHTERS
 *      MW_E_NOT_SUPPORT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_state_machine_handle(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_REQUEST_T     request,
    void                    *ptr_arg)
{
    MW_ERROR_NO_T               rc = MW_E_NOT_SUPPORT;
    ERPS_STATE_REQ_ACT_FUNC_T   ptr_func = NULL;
    MW_CHECK_PTR(ptr_ring);
    ERPS_LOG_DEBUG("Request:%s, cur state:%s, local request:%s",
                                erps_record_ring_req_vec[request],
                                erps_record_ring_state_vec[ptr_ring->ring_state],
                                erps_record_ring_req_vec[ptr_ring->local_request]);
    if((ERPS_RING_REQ_CLEAR_SF == request) &&
       (ERPS_RING_REQ_SF == ptr_ring->local_request))
    {
        /* Spec 10.1.1, the priorty logic retains SF as the current top priority request, until either a local clear SF */
        ptr_ring->local_request = ERPS_RING_REQ_LAST;
    }
    if(request > ptr_ring->local_request)
    {
        ERPS_LOG_DEBUG("A higher priority request exists locally.");
        return rc;
    }
    ptr_ring->local_request = ERPS_RING_REQ_LAST;
    ptr_func = _erps_state_req_act_func_vec[request][ptr_ring->ring_state];
    if(NULL != ptr_func)
    {
        rc = ptr_func(ptr_ring, request, ptr_arg);
    }

    return rc;
}


