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
 *      This file defines the data structure for ERPS timer.
 *
 * NOTES:
 */
#ifndef ERPS_TIMER_H
#define ERPS_TIMER_H
/* INCLUDE FILE DECLARATIONS
 */
#include        "erps.h"
#include        "osapi_timer.h"
#include        "air_util.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define ERPS_MAX_TIMER_NUM                   ((4) * ERPS_MAX_INSTANCE_NUM)
#define ERPS_INVALID_TIMER_IDX               (0xFF)

#define ERPS_SW_TMR_INTERVAL                 (1000) /* 1000 Milliseconds */
#define ERPS_SW_TMR_NAME                     "erps_xtmr"

/* MACRO FUNCTION DECLARATIONS
 */
#define ERPS_WTR_TIMER_TIME_IS_VALID(time)          \
            (((time) >= 1) && ((time) <= 12))

#define ERPS_GUARD_TIMER_TIME_IS_VALID(time)        \
            (((time) >= 1) && ((time) <= 200))

#define ERPS_HOLD_OFF_TIMER_TIME_IS_VALID(time)     \
            (((time) >= 0) && ((time) <= 100))

/* DATA TYPE DECLARATIONS
 */
typedef MW_ERROR_NO_T
(*ERPS_TIMEOUT_CALLBACK_FUNC_T)(
    UI8_T                   id,
    void                    *ptr_arg);

typedef struct ERPS_TIMER_S
{
    void                            *ptr_data;      /* data */
    UI32_T                          time_ms;        /* timer timeout in ms*/
    ERPS_TIMEOUT_CALLBACK_FUNC_T    callback;       /* callback function */
    UI8_T                           in_use;         /* in use flag */
} ERPS_TIMER_T;

typedef enum
{
    ERPS_TIMER_STATE_OFF = 0,
    ERPS_TIMER_STATE_ON,

    ERPS_TIMER_STATE_LAST
} ERPS_TIMER_STATE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   erps_sw_timer_init
 * PURPOSE:
 *      This API is used to initial ERPS software timer.
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
erps_sw_timer_init(
    void);

/* FUNCTION NAME:   erps_sw_timer_deinit
 * PURPOSE:
 *      This API is used to de-initial ERPS software timer.
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
erps_sw_timer_deinit(
    void);

/* FUNCTION NAME:   erps_sw_timer_start
 * PURPOSE:
 *      This API is used to start ERPS software timer.
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
MW_ERROR_NO_T
erps_sw_timer_start(
    void);

/* FUNCTION NAME:   erps_sw_timer_stop
 * PURPOSE:
 *      This API is used to stop ERPS software timer.
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
MW_ERROR_NO_T
erps_sw_timer_stop(
    void);

/* FUNCTION NAME:   erps_hw_timer_start
 * PURPOSE:
 *      This API is used to start a hw timer instance for ERPS.
 *
 * INPUT:
 *      ptr_data             -- user data pointer
 *      time_ms              -- timeout value, unit ms
 *      callback             -- callback function
 *
 * OUTPUT:
 *      ptr_id               -- pointer to timer index
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_start(
    UI8_T                           *ptr_id,
    UI32_T                          time_ms,
    void                            *ptr_data,
    ERPS_TIMEOUT_CALLBACK_FUNC_T    callback);

/* FUNCTION NAME:   erps_hw_timer_stop
 * PURPOSE:
 *      This API is used to start an erps hw timer instance.
 *
 * INPUT:
 *      id                   -- index of timer
 *
 * OUTPUT:
 *      pptr_data            -- double pointer to user data pointer
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_stop(
    UI8_T                           id,
    void                            **pptr_data);

/* FUNCTION NAME:   erps_hw_timer_timeout_check
 * PURPOSE:
 *      This API is used to check wherether the timer timeout or not.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      MW_E_OK.
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
erps_hw_timer_timeout_check
    (void);

/* FUNCTION NAME:   erps_timer_dump_timer
 * PURPOSE:
 *      This API is used to dump specific ERPS timer info.
 *
 * INPUT:
 *      id                   -- index of timer
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
erps_timer_dump_timer(
    UI8_T   idx);

/* FUNCTION NAME:   erps_timer_dump
 * PURPOSE:
 *      This API is used to dump ERPS module timer info.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
erps_timer_dump(void);

#endif  /* ERPS_TIMER_H */
