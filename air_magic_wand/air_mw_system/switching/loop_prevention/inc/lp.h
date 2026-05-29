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

/* FILE NAME:  lp.h
 * PURPOSE:
 *      Define loop prevention function.
 *
 * NOTES:
 */

#ifndef LP_H
#define LP_H
/* INCLUDE FILE DECLARATIONS
 */
#include    "mw_error.h"
#include    "osapi_string.h"
#include    "osapi_memory.h"
#include    "mw_platform.h"
#include    "mw_portbmp.h"
#include    "air_port.h"
#include    "switch.h"
#include    "lp_utils.h"
#include    "lp_mode.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define LP_MW_SUPPORT                       (1)     /* Define if magicwand support is enabled */
#if defined(AIR_LITE_MW) || defined(AIR_EN_CORAL)
#define AIR_HARDWARE_SEND_LDF_SUPPORT       (1)     /* Define if hardware send LDF is enabled */
#endif
//#define LP_LOOP_DECTECTION_SUPPORT          (1)     /* Define if loop detection is enabled */
//#define LP_GLOBAL_LED_SUPPORT               (1)     /* Define if global led support is enabled */
//#define LP_CUSTOMER_CONFIG_SUPPORT          (1)     /* Define if customer config support is enabled */

#define LP_DEBUG                            (1)     /* Define if debug is enabled */

#define LP_MODULE_NAME      "lp"
#define LP_MODULE_ID        (1)
#define LP_TASK_NAME        "lpd"
#define LP_STACK_SIZE       (configMINIMAL_STACK_SIZE*2)
#define LP_DELAY_100MS      (100/portTICK_RATE_MS)
#define LP_DELAY_INFINITY   (0xFFFFFFFF)
#define LP_LDF_TIMESTAPE_VALID_TIME   (6)
#define MAX_PORT_NUM         AIR_MAX_PORT_NUM       /* Maximum physical ports */
#define MAX_TRUNK_NUM        AIR_MAX_TRUNK_GROUP    /* Maximum trunk ports */


/* MACRO FUNCTION DECLARATIONS
 */
#define ATTRIBUTE_PACK __attribute__((packed))

/* DATA TYPE DECLARATIONS
 */
typedef struct LP_TRUNK_S
{
    UI32_T group[MAX_TRUNK_NUM];
}LP_TRUNK_T;

typedef struct LP_MSG_S {
    UI16_T      opCode;
#define LP_TRUNK_NONE   (0xFFFF)
#define LP_TRUNK_GP1    (0x1000)
#define LP_TRUNK_GP2    (0x2000)
#define LP_TRUNK_GP3    (0x4000)
#define LP_TRUNK_GP4    (0x8000)
#define LP_TRUNK_MSK    (0xF000)
    UI16_T      portId;
    UI8_T       moduleId[6];
    UI8_T       timeStamp[6];
} LP_MSG_T;

typedef enum {
    LP_NORMAL = 0,
    LP_BLOCK = 1,
    LP_LOOP = 2,
} LP_STATE_T;

typedef struct LP_MCR_CONFIG_S
{
    UI32_T                  value;
    UI8_T                   ref_cnt;
} ATTRIBUTE_PACK LP_MCR_CONFIG_T;

typedef struct LP_CONFIG_INFO_S
{
    MW_LP_MODE_T            lp_admin;
    AIR_PORT_BITMAP_T       blocked_pbmp;
    AIR_PORT_BITMAP_T       looped_pbmp;
    AIR_PORT_BITMAP_T       loop_led_pbmp;
    AIR_PORT_BITMAP_T       *ptr_to_blk_pbmp;
    AIR_PORT_BITMAP_T       *ptr_to_loop_pbmp;
    UI32_T                  timestamp;
    LP_TRUNK_T              trunk_info;
    UI8_T                   *ptr_fc_resume_timer;
    UI8_T                   loop_timer[MAX_PORT_NUM];
    UI8_T                   block_timer[MAX_PORT_NUM];
    LP_MCR_CONFIG_T         record_mcr[MAX_PORT_NUM];
    UI8_T                   looped_port_blink;
} ATTRIBUTE_PACK LP_CONFIG_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: lp_init
 * PURPOSE:
 *      Start loop prevention function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_init(
    void);

/* FUNCTION NAME:   lp_deinit
 * PURPOSE:
 *      Free the resources in lp init function.
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
lp_deinit(void);

/* FUNCTION NAME: lp_getConfig
 * PURPOSE:
 *      Get the loop prevention configuration
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to the loop prevention configuration
 * NOTES:
 *      None
 */
LP_CONFIG_INFO_T *
lp_getConfig(
    void);

#endif /* end of LP_H */
