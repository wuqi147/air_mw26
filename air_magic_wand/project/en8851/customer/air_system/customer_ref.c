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

/*INCLUDE FILE DECLARATIONS
*/
#include <air_cfg.h>
#include <air_init.h>
#include <osapi_string.h>
#ifdef AIR_EN_SFP_LED
#include <sfp_led.h>
#include "mw_sfp_led.h"
#endif
#include "mw_led.h"
#ifdef AIR_EN_I2C_BITBANG
#include <i2c_bitbang.h>
#endif
#include "product_ref.h"
#include "switch.h"
#include "mw_utils.h"
#include "mw_log.h"

/*NAMING CONSTANT DECLARATIONS
*/
/*MACRO FUNCTION DECLARATIONS
*/
/*DATA TYPE DECLARATIONS
*/
typedef struct SDK_CFG_S
{
    UI32_T                      unit;
    AIR_CFG_TYPE_T              cfg_type;
    UI32_T                      param0;
    UI32_T                      param1;
    UI32_T                      value;
} SDK_CFG_T;

typedef struct MODULE_CFG_MAP_ITEM_S{
    UI8_T                           product_id;
    const SDK_CFG_T                 *ptr_sdk_cfg;
    UI32_T                          sdk_cfg_count;
    const AIR_INIT_PORT_MAP_T       *ptr_init_port_map;
    UI32_T                          init_port_map_count;
}MODULE_CFG_MAP_ITEM_T;

typedef struct
{
    const UI8_T                     product_id;
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_poe_device_map;
    const UI32_T                    poe_device_map_cnt;
    const AIR_INIT_POE_PORT_MAP_T   *ptr_poe_port_map;
    const UI32_T                    poe_port_map_cnt;
} POE_CONFIG_MAP_T;

/*GLOBAL VARIABLE DECLARATIONS
*/
/*LOCAL SUBPROGRAM DECLARATIONS
*/
/*STATIC VARIABLE DECLARATIONS
*/
static const SDK_CFG_T _customer_ref_cfg_en8851e_rfb[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        1},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1},
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8851e_rfb[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    { 2,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    { 3,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    { 4,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
};

static const SDK_CFG_T _customer_ref_cfg_en8851c_rfb[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x7FE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x7FE7},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,        9,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       10,      0,        2},
#endif
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
#endif
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8851c_rfb[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    { 2,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    { 3,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    { 4,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
#ifdef AIR_SUPPORT_SFP
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 1, 0}}
#endif
};

#ifdef AIR_EN_POE
static const SDK_CFG_T _customer_ref_cfg_en8851c_rfb_an8502_4p[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0xFE7},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,        9,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       10,      0,        2},
#endif
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
#endif
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1},
/* For PoE MAX LED. */
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      0,      0,         1},
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8851c_rfb_an8502_4p[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 7}},
    { 2,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 6}},
    { 3,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 5}},
    { 4,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 4}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
#ifdef AIR_SUPPORT_SFP
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 1, 0}}
#endif
};

static const AIR_INIT_POE_DEVICE_MAP_T _poe_en8851c_rfb_an8502_4p_device_map[] =
{
  /* unit,  device index,  i2c bus id, i2c address */
    {0,     0,             0,          0x74},
};

static const AIR_INIT_POE_PORT_MAP_T _poe_en8851c_rfb_an8502_4p_port_map[] =
{
  /* unit,  air port,   type,   primary port,   secondary port */
    {0, 1,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 0} },
    {0, 2,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 1} },
    {0, 3,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 2} },
    {0, 4,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 3} },
};

static const SDK_CFG_T _customer_ref_cfg_en8851c_rfb_an8503_8p[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0xFE7},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,        9,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       10,      0,        2},
#endif
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
#endif
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1},
/* For PoE MAX LED. */
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      0,      0,         1},
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8851c_rfb_an8503_8p[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 7}},
    { 2,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 6}},
    { 3,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 5}},
    { 4,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 4}},
    { 5,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 0}},
    { 6,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 1}},
    { 7,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 2}},
    { 8,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port = {AIR_INIT_ENHANCED_BASET_PORT_FLAGS_POE, 3}},
#ifdef AIR_SUPPORT_SFP
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 1, 0}}
#endif
};

static const AIR_INIT_POE_DEVICE_MAP_T _poe_en8851c_rfb_an8503_8p_device_map[] =
{
  /* unit,  device index,  i2c bus id, i2c address */
    {0,     0,             0,          0x74},
};

static const AIR_INIT_POE_PORT_MAP_T _poe_en8851c_rfb_an8503_8p_port_map[] =
{
  /* unit,  air port,   type,   primary port,   secondary port */
    {0, 1,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 0} },
    {0, 2,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 1} },
    {0, 3,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 2} },
    {0, 4,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 3} },
    {0, 5,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 4} },
    {0, 6,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 5} },
    {0, 7,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 6} },
    {0, 8,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 7} },
};
#endif /* AIR_EN_POE */

static const SDK_CFG_T _customer_ref_cfg_en8853c_rfb[] =
{/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               9,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              10,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              11,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              12,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              17,      0,       23},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              18,      0,       24},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              19,      0,       21},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              20,      0,       22},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              21,      0,       17},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              22,      0,       18},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              23,      0,       19},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              24,      0,       20},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              25,      0,       25},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              26,      0,       26},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              27,      0,       27},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              28,      0,       28},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           9,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          10,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          11,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          12,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          17,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          18,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          19,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          20,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          21,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          22,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          23,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          24,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          25,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          26,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          27,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          28,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           1,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           2,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           3,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           4,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           5,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           6,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           7,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           8,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          17,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          18,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          19,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          20,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          21,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          22,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          23,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          24,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          25,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          26,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          27,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          28,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      0,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    25,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    26,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    27,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    28,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    25,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    26,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    27,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    28,      1,    0x786},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          17,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          18,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          19,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          20,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          21,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          22,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          23,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          24,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          25,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          26,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          27,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          28,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   1,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   2,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   3,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   4,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   5,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   6,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   7,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   8,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  17,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  18,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  19,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  20,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  21,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  22,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  23,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  24,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  25,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  26,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  27,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  28,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       25,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       26,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       27,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       28,      0,        2},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_rfb[] =
{/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 6}},
    { 2,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 7}},
    { 3,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 4}},
    { 4,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 5}},
    { 5,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 0}},
    { 6,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 1}},
    { 7,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 2}},
    { 8,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 3}},
    { 9,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    {10,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    {11,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    {12,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    {13,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    {14,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    {15,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    {16,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    {17,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 6}},
    {18,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 7}},
    {19,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 4}},
    {20,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 5}},
    {21,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
    {22,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 1}},
    {23,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 2}},
    {24,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 3}},
    {25,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 0}},
    {26,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 1}},
    {27,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 2}},
    {28,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 3}},
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_an8808q_rfb[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               9,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              10,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              11,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              12,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              17,      0,       18},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              18,      0,       17},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              19,      0,       20},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              20,      0,       19},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              21,      0,       21},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              22,      0,       22},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              23,      0,       23},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              24,      0,       24},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              25,      0,       25},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              26,      0,       26},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              27,      0,       27},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              28,      0,       28},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           9,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          10,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          11,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          12,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          17,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          18,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          19,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          20,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          21,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          22,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          23,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          24,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          25,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          26,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          27,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          28,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           25,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           26,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           27,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           28,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          25,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          26,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          27,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          28,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      0,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    25,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    26,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    27,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    28,      0,    0x61},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    25,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    26,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    27,      1,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    28,      1,    0x786},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          17,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          18,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          19,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          20,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          21,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          22,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          23,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          24,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          25,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          26,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          27,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          28,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   1,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   2,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   3,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   4,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   5,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   6,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   7,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   8,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  17,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  18,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  19,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  20,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  21,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  22,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  23,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  24,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  25,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  26,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  27,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  28,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       25,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       26,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       27,      0,        2},
    {0, AIR_CFG_TYPE_SERDES_PORT_OPTION,       28,      0,        2},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_an8808q_rfb[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 1}},
    { 2,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 0}},
    { 3,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 3}},
    { 4,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 2}},
    { 5,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 4}},
    { 6,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 5}},
    { 7,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 6}},
    { 8,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 7}},
    { 9,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    {10,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    {11,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    {12,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    {13,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    {14,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    {15,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    {16,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    {17,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 1}},
    {18,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 0}},
    {19,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 3}},
    {20,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 2}},
    {21,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 4}},
    {22,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 5}},
    {23,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 6}},
    {24,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 7}},
    {25,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 0}},
    {26,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 1}},
    {27,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 2}},
    {28,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 3}},
};


static const SDK_CFG_T _customer_ref_cfg_en8851c_8p_2sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,     0x861},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,     0x861},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,     0xF86},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,     0xF86},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
#endif
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
#endif
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      4,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      5,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8851c_8p_2sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    { 2,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    { 3,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    { 4,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
#ifdef AIR_SUPPORT_SFP
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 1, 0}}
#endif
};

static const SDK_CFG_T _customer_ref_cfg_en8851c_8p_2sfp_l[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8851C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x861},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x861},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,    0xF86},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,    0xF86},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
#endif
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
#ifdef AIR_SUPPORT_SFP
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
#endif
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      4,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      5,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1}
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_24p_en8804_4p_4sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               9,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              10,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              11,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              12,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              17,      0,       23},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              18,      0,       24},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              19,      0,       21},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              20,      0,       22},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              21,      0,       17},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              22,      0,       18},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              23,      0,       19},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              24,      0,       20},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              25,      0,       25},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              26,      0,       26},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              27,      0,       27},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              28,      0,       28},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           9,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          10,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          11,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          12,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          17,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          18,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          19,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          20,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          21,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          22,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          23,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          24,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          25,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          26,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          27,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          28,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           1,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           2,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           3,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           4,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           5,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           6,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           7,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           8,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          17,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          18,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          19,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          20,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          21,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          22,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          23,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          24,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          25,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          26,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          27,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          28,      0,     0x52},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         26,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         27,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         28,      1,    0x786},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          17,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          18,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          19,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          20,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          21,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          22,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          23,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          24,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          25,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          26,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          27,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          28,     0,        0},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    25,      0,    0x7E7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    26,      0,    0x7E7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    27,      0,    0x7E7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,    28,      0,    0x7E7},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   1,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   2,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   3,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   4,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   5,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   6,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   7,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   8,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  17,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  18,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  19,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  20,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  21,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  22,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  23,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  24,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  25,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  26,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  27,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  28,      0,        3},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_24p_en8804_4p_4sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 6}},
    { 2,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 7}},
    { 3,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 4}},
    { 4,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 5}},
    { 5,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 0}},
    { 6,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 1}},
    { 7,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 2}},
    { 8,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 3}},
    { 9,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    {10,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    {11,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    {12,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    {13,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    {14,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    {15,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    {16,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    {17,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 6}},
    {18,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 7}},
    {19,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 4}},
    {20,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 5}},
    {21,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
    {22,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 1}},
    {23,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 2}},
    {24,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 3}},
    {25,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 0}},
    {26,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 1}},
    {27,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 2}},
    {28,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 2, 3}},
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_24p_1sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               9,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              10,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              11,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              12,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              17,      0,       23},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              18,      0,       24},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              19,      0,       21},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              20,      0,       22},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              21,      0,       17},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              22,      0,       18},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              23,      0,       19},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              24,      0,       20},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           9,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          10,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          11,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          12,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          17,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          18,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          19,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          20,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          21,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          22,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          23,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          24,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           1,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           2,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           3,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           4,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           5,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           6,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           7,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           8,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          17,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          18,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          19,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          20,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          21,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          22,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          23,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          24,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         19,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         20,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         21,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         22,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         23,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         24,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         25,      1,    0x786},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          17,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          18,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          19,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          20,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          21,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          22,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          23,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          24,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          25,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   1,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   2,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   3,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   4,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   5,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   6,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   7,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   8,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  17,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  18,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  19,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  20,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  21,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  22,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  23,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  24,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  25,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      5,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_24p_1sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 6}},
    { 2,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 7}},
    { 3,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 4}},
    { 4,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 5}},
    { 5,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 0}},
    { 6,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 1}},
    { 7,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 2}},
    { 8,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 3}},
    { 9,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    {10,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    {11,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    {12,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    {13,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    {14,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    {15,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    {16,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    {17,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 6}},
    {18,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 7}},
    {19,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 4}},
    {20,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 5}},
    {21,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
    {22,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 1}},
    {23,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 2}},
    {24,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 3}},
    {25,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 2, 0}},
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_8p_en8804_4sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  11,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  12,      0,        1},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_8p_en8804_4sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    { 2,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    { 3,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    { 4,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
    {11,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 2, 0}},
    {12,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 0}},
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_8p_en8804_8sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           13,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           14,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           15,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_BUS_ID,           16,      0,        1},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          13,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          14,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          15,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,          16,      0,     0x51},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,         13,      0,    0xFE7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,         14,      0,    0xFE7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,         15,      0,    0x7E7},
    {0, AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,         16,      0,    0xFE7},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  11,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  12,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  13,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  14,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  15,      0,        3},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  16,      0,        3},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,      13,      0,        2},
    {0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,      14,      0,        2},
    {0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,      15,      0,        2},
    {0, AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,      16,      0,        2},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        2}
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_8p_en8804_8sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    { 2,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    { 3,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    { 4,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    { 5,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    { 6,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    { 7,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    { 8,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
    {11,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 2, 0}},
    {12,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 0}},
    {13,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 4, 0}},
    {14,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 4, 1}},
    {15,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 4, 2}},
    {16,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={2, 4, 3}},
};

static const SDK_CFG_T _customer_ref_cfg_en8853c_16p_2sfp[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_FORCE_DEVICE_ID,           0,      0,  0x8853C},
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,  MW_TASK_PRIORITY_IFMON},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               9,      0,       15},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              10,      0,       16},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              11,      0,       13},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              12,      0,       14},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              13,      0,        9},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              14,      0,       10},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              15,      0,       11},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,              16,      0,       12},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           1,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           2,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           3,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           4,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           6,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           7,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           8,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,           9,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          10,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          11,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          12,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          13,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          14,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          15,      0,        0},
    {0, AIR_CFG_TYPE_PHY_ACCESS_TYPE,          16,      0,        0},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           1,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           2,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           3,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           4,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           5,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           6,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           7,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_I2C_ADDRESS,           8,      0,     0x50},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      1,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      1,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         11,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         12,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         13,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         14,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         15,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         16,      0,    0x786},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         17,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         18,      0,     0x61},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          1,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          2,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          3,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          4,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          5,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          6,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          7,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          8,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          9,      0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          10,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          11,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          12,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          13,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          14,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          15,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          16,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          17,     0,        0},
    {0, AIR_CFG_TYPE_PORT_ADMIN_STATE,          18,     0,        0},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   1,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   2,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   3,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   4,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   5,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   6,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   7,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   8,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  17,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  18,      0,        1},
    {0, AIR_CFG_TYPE_PHY_LED_TYPE,              0,      0,        0},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_MDIO_CLOCK,                0,      0,        2},
    {0, AIR_CFG_TYPE_SIF_LOCAL_CLOCK,           1,      0,        0},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      1,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      5,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1},
};

static const AIR_INIT_PORT_MAP_T _customer_ref_port_map_en8853c_16p_2sfp[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 6}},
    { 2,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 7}},
    { 3,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 4}},
    { 4,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 3, 5}},
    { 5,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 0}},
    { 6,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 1}},
    { 7,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 2}},
    { 8,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 4, 3}},
    { 9,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={6}},
    {10,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={7}},
    {11,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={4}},
    {12,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={5}},
    {13,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={0}},
    {14,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={1}},
    {15,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={2}},
    {16,  AIR_INIT_PORT_TYPE_BASET, AIR_INIT_PORT_SPEED_1000M,  .baset_port={3}},
    {17,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 0, 0}},
    {18,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M, .xsgmii_port={0, 1, 0}},
};

static const MODULE_CFG_MAP_ITEM_T _customer_system_module_cfg_map[] =
{
    {MW_PRODUCT_ID_EN8851C_8P,
     _customer_ref_cfg_en8851e_rfb, (sizeof(_customer_ref_cfg_en8851e_rfb) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851e_rfb, (sizeof(_customer_ref_port_map_en8851e_rfb) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8851C_RFB,
     _customer_ref_cfg_en8851c_rfb, (sizeof(_customer_ref_cfg_en8851c_rfb) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851c_rfb, (sizeof(_customer_ref_port_map_en8851c_rfb) / sizeof(AIR_INIT_PORT_MAP_T))
    },
#ifdef AIR_EN_POE
    {MW_PRODUCT_ID_EN8851C_RFB_AN8502_4P,
     _customer_ref_cfg_en8851c_rfb_an8502_4p, (sizeof(_customer_ref_cfg_en8851c_rfb_an8502_4p) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851c_rfb_an8502_4p, (sizeof(_customer_ref_port_map_en8851c_rfb_an8502_4p) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8851C_RFB_AN8503_8P,
     _customer_ref_cfg_en8851c_rfb_an8503_8p, (sizeof(_customer_ref_cfg_en8851c_rfb_an8503_8p) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851c_rfb_an8503_8p, (sizeof(_customer_ref_port_map_en8851c_rfb_an8503_8p) / sizeof(AIR_INIT_PORT_MAP_T))
    },
#endif /* AIR_EN_POE */
    {MW_PRODUCT_ID_EN8853C_RFB,
     _customer_ref_cfg_en8853c_rfb,(sizeof(_customer_ref_cfg_en8853c_rfb) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8853c_rfb, (sizeof(_customer_ref_port_map_en8853c_rfb) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8853C_AN8808Q_RFB,
     _customer_ref_cfg_en8853c_an8808q_rfb,(sizeof(_customer_ref_cfg_en8853c_an8808q_rfb) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8853c_an8808q_rfb, (sizeof(_customer_ref_port_map_en8853c_an8808q_rfb) / sizeof(AIR_INIT_PORT_MAP_T))
    },

    {MW_PRODUCT_ID_EN8851C_8P_2SFP,
     _customer_ref_cfg_en8851c_8p_2sfp,(sizeof(_customer_ref_cfg_en8851c_8p_2sfp) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851c_8p_2sfp, (sizeof(_customer_ref_port_map_en8851c_8p_2sfp) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8851C_8P_2SFP_L,
     _customer_ref_cfg_en8851c_8p_2sfp_l,(sizeof(_customer_ref_cfg_en8851c_8p_2sfp_l) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8851c_8p_2sfp, (sizeof(_customer_ref_port_map_en8851c_8p_2sfp) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8853C_24P_EN8804_4P_4SFP,
     _customer_ref_cfg_en8853c_24p_en8804_4p_4sfp,(sizeof(_customer_ref_cfg_en8853c_24p_en8804_4p_4sfp) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8853c_24p_en8804_4p_4sfp, (sizeof(_customer_ref_port_map_en8853c_24p_en8804_4p_4sfp) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8853C_24P_1SFP,
     _customer_ref_cfg_en8853c_24p_1sfp,(sizeof(_customer_ref_cfg_en8853c_24p_1sfp) / sizeof(SDK_CFG_T)),
     _customer_ref_port_map_en8853c_24p_1sfp, (sizeof(_customer_ref_port_map_en8853c_24p_1sfp) / sizeof(AIR_INIT_PORT_MAP_T))
    },
    { MW_PRODUCT_ID_EN8853C_8P_EN8804_4SFP,\
     _customer_ref_cfg_en8853c_8p_en8804_4sfp,(sizeof(_customer_ref_cfg_en8853c_8p_en8804_4sfp)/sizeof(SDK_CFG_T)),\
      _customer_ref_port_map_en8853c_8p_en8804_4sfp, (sizeof(_customer_ref_port_map_en8853c_8p_en8804_4sfp)/sizeof(AIR_INIT_PORT_MAP_T))\
    },
    { MW_PRODUCT_ID_EN8853C_8P_EN8804_8SFP,\
     _customer_ref_cfg_en8853c_8p_en8804_8sfp,(sizeof(_customer_ref_cfg_en8853c_8p_en8804_8sfp)/sizeof(SDK_CFG_T)),\
      _customer_ref_port_map_en8853c_8p_en8804_8sfp, (sizeof(_customer_ref_port_map_en8853c_8p_en8804_8sfp)/sizeof(AIR_INIT_PORT_MAP_T))\
    },
    { MW_PRODUCT_ID_EN8853C_16P_2SFP,\
     _customer_ref_cfg_en8853c_16p_2sfp,(sizeof(_customer_ref_cfg_en8853c_16p_2sfp)/sizeof(SDK_CFG_T)),\
      _customer_ref_port_map_en8853c_16p_2sfp, (sizeof(_customer_ref_port_map_en8853c_16p_2sfp)/sizeof(AIR_INIT_PORT_MAP_T))\
    },
};

#ifdef AIR_EN_SFP_LED
/* SFP port id and LED pin id mapping table
 * Should be defined in compile time
 */
static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8851c_rfb[] =
{
  /* unit,  port id,               led pin id */
    {0,     9,    6},
    {0,     10,   8}
};

static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8851c_8p_2sfp[] =
{
  /* unit,  port id,               led pin id */
    {0,     9,    4},
    {0,     9,    6},
    {0,     10,   5},
    {0,     10,   8}
};

static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8853c_24p_1sfp[] =
{
  /* unit,  port id,               led pin id */
    {0,     25,   8},
    {0,     25,   5}
};

static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8853c_8p_4sfp[] =
{
  /* unit,  port id,               led pin id */
    {0,     9,    1},
    {0,     10,   5},
    {0,     11,   8},
    {0,     12,   10}
};

static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8853c_8p_8sfp[] =
{
  /* unit,  port id,               led pin id */
    {0,     9,    1},
    {0,     10,   5},
    {0,     11,   8},
    {0,     12,   10}
};

static const SFP_LED_PORT_MAP_T _sfp_led_port_map_en8853c_16p_2sfp[] =
{
  /* unit,  port id,               led pin id */
    {0,     17,   1},
    {0,     17,   5},
    {0,     18,   6},
    {0,     18,   8}
};

static const MW_SFP_LED_PORT_MAP_T _sfp_led_port_map[] =
{
    {MW_PRODUCT_ID_EN8851C_RFB, sizeof(_sfp_led_port_map_en8851c_rfb) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8502_4P, sizeof(_sfp_led_port_map_en8851c_rfb) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_RFB_AN8503_8P, sizeof(_sfp_led_port_map_en8851c_rfb) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8851c_rfb},
    {MW_PRODUCT_ID_EN8851C_8P_2SFP, sizeof(_sfp_led_port_map_en8851c_8p_2sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8851c_8p_2sfp},
    {MW_PRODUCT_ID_EN8851C_8P_2SFP_L, sizeof(_sfp_led_port_map_en8851c_8p_2sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8851c_8p_2sfp},
    {MW_PRODUCT_ID_EN8853C_24P_1SFP, sizeof(_sfp_led_port_map_en8853c_24p_1sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8853c_24p_1sfp},
    {MW_PRODUCT_ID_EN8853C_8P_EN8804_4SFP, sizeof(_sfp_led_port_map_en8853c_8p_4sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8853c_8p_4sfp},
    {MW_PRODUCT_ID_EN8853C_8P_EN8804_8SFP, sizeof(_sfp_led_port_map_en8853c_8p_8sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8853c_8p_8sfp},
    {MW_PRODUCT_ID_EN8853C_16P_2SFP, sizeof(_sfp_led_port_map_en8853c_16p_2sfp) / sizeof(SFP_LED_PORT_MAP_T), _sfp_led_port_map_en8853c_16p_2sfp},
};
#endif

#ifdef AIR_EN_I2C_BITBANG
static const I2C_BITBANG_CFG_T i2c_bitbang_cfg[] = {
/**
 * unit  SDA  SCL
 * ----  ---  ---
 */
    {0,   19,  20}
};
#endif

#ifdef AIR_EN_POE
static const POE_CONFIG_MAP_T _poe_config_map[] = {
    {MW_PRODUCT_ID_EN8851C_RFB_AN8502_4P,
    _poe_en8851c_rfb_an8502_4p_device_map, (sizeof(_poe_en8851c_rfb_an8502_4p_device_map) / sizeof(AIR_INIT_POE_DEVICE_MAP_T)),
    _poe_en8851c_rfb_an8502_4p_port_map, (sizeof(_poe_en8851c_rfb_an8502_4p_port_map) / sizeof(AIR_INIT_POE_PORT_MAP_T))
    },
    {MW_PRODUCT_ID_EN8851C_RFB_AN8503_8P,
    _poe_en8851c_rfb_an8503_8p_device_map, (sizeof(_poe_en8851c_rfb_an8503_8p_device_map) / sizeof(AIR_INIT_POE_DEVICE_MAP_T)),
    _poe_en8851c_rfb_an8503_8p_port_map, (sizeof(_poe_en8851c_rfb_an8503_8p_port_map) / sizeof(AIR_INIT_POE_PORT_MAP_T))
    },
};
#endif /* AIR_EN_POE */

/*LOCAL SUBPROGRAM BODIES
*/
/*FUNCTION NAME: _customer_ref_get_cfg
* PURPOSE:
*       Get the configuring item based on the product ID.
* INPUT:
*       none
* OUTPUT:
*       none
* RETURN:
*       NULL -- getting configuring item failed
*       NOT A NULL -- return the pointer which point to the configuring item which of module ID matched the module ID inputed
* NOTES:
*      None
*/
const MODULE_CFG_MAP_ITEM_T *
_customer_ref_get_cfg(
    void)
{
    MW_PRODUCT_ID_T product_id = mw_get_productID();
    int i = 0, count = sizeof(_customer_system_module_cfg_map)/sizeof(MODULE_CFG_MAP_ITEM_T);;

    for (i = 0; i < count; i++)
    {
        if (_customer_system_module_cfg_map[i].product_id == product_id)
        {
            return &_customer_system_module_cfg_map[i];
        }
    }
    return NULL;
}

/*FUNCTION NAME: _ref_getConfigValue
* PURPOSE:
*       find out the configuring value by inputing type,param0,and param1
* INPUT:
*       unit -- Device unit number
        type -- The type of configuring item
* OUTPUT:
*       ptr_value -- The searching result will be put into ptr_value->value
* RETURN:
*       AIR_E_OK -- The configuring value searching successfully
*       OTHERS -- The configuring value searching failed
* NOTES:
*      None
*/
static AIR_ERROR_NO_T
_ref_getConfigValue(
    const UI32_T unit,
    const AIR_CFG_TYPE_T type,
    AIR_CFG_VALUE_T *ptr_value)
{
    AIR_ERROR_NO_T rc = AIR_E_ENTRY_NOT_FOUND;
    const SDK_CFG_T *ptr_cfg = NULL;
    UI32_T i = 0, cfg_num = 0;
    const MODULE_CFG_MAP_ITEM_T *ptr_item = _customer_ref_get_cfg();

    if (NULL != ptr_item)
    {
        cfg_num = ptr_item->sdk_cfg_count;
        ptr_cfg = ptr_item->ptr_sdk_cfg;

        for (i = 0; i < cfg_num; i++)
        {
            if ((ptr_cfg[i].unit == unit)
                && (ptr_cfg[i].cfg_type == type)
                && (ptr_cfg[i].param0 == ptr_value->param0)
                && (ptr_cfg[i].param1 == ptr_value->param1))
            {
                rc = AIR_E_OK;
                ptr_value->value = ptr_cfg[i].value;
                break;
            }
        }
    }

    return  rc;
}

#ifdef AIR_EN_SFP_LED
static const MW_SFP_LED_PORT_MAP_T *
_customer_ref_getSFPLedPortMap(
    void)
{
    UI8_T product_id = mw_get_productID();
    UI32_T i = 0, count = sizeof(_sfp_led_port_map) / sizeof(MW_SFP_LED_PORT_MAP_T);

    for (; i < count; i++)
    {
        if (_sfp_led_port_map[i].product_id == product_id)
        {
            return &_sfp_led_port_map[i];
        }
    }

    return NULL;
}
#endif

#ifdef AIR_EN_POE
static const POE_CONFIG_MAP_T *
_customer_ref_getPoeCfgMap(
    void)
{
    UI8_T product_id = mw_get_productID();
    UI32_T i = 0, count = sizeof(_poe_config_map) / sizeof(POE_CONFIG_MAP_T);

    for (; i < count; i++)
    {
        if (_poe_config_map[i].product_id == product_id)
        {
            return &_poe_config_map[i];
        }
    }
    return NULL;
}
#endif /* AIR_EN_POE */

/* EXPORTED SUBPROGRAM BODIES
*/
/*FUNCTION NAME: customer_ref_registerConfig
* PURPOSE:
*        the API offered to main.cpp call for registering the function pointer of "_ref_getConfigValue"
* INPUT:
*       unit -- Device unit number
* OUTPUT:
*       none
* RETURN:
*       AIR_E_OK -- registering configuration successfully
*       OTHERS -- registering configuration failed
* NOTES:
*      None
*/
AIR_ERROR_NO_T
customer_ref_registerConfig(
    UI32_T unit)
{
     AIR_ERROR_NO_T rc = air_cfg_register(unit, _ref_getConfigValue);

    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to register customized sdk setting!\n");
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Registering customized sdk setting...\n");
    }
    return rc;
}

/*FUNCTION NAME: customer_ref_initPortMap
* PURPOSE:
*       the API offered to main.cpp call for registering port map
* INPUT:
*       unit -- Device unit number
* OUTPUT:
*       none
* RETURN:
*       AIR_E_OK -- initializing port map successfully
*       OTHERS -- initializing port map failed
* NOTES:
*      None
*/
AIR_ERROR_NO_T
customer_ref_initPortMap(
    UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_NOT_SUPPORT;
    const MODULE_CFG_MAP_ITEM_T *ptr_item = _customer_ref_get_cfg();

    if (NULL != ptr_item)
    {
        rc = air_init_initSdkPortMap(unit, ptr_item->init_port_map_count, (AIR_INIT_PORT_MAP_T *)(ptr_item->ptr_init_port_map));
        if (AIR_E_OK != rc)
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize port map setting!\n");
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Initializing port map setting...\n");
        }
    }

    return rc;
}

#ifdef AIR_EN_SFP_LED
AIR_ERROR_NO_T
customer_ref_initSfpLed(
    UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    const MW_SFP_LED_PORT_MAP_T *ptr_portMap = _customer_ref_getSFPLedPortMap();

    if (NULL != ptr_portMap)
    {
#ifdef AIR_SUPPORT_SFP
        rc = sfp_led_init(unit, ptr_portMap->count, (SFP_LED_PORT_MAP_T *)ptr_portMap->ptr_portMap, mw_sfpLed_getPortStatus);
#else
        rc = sfp_led_init(unit, ptr_portMap->count, (SFP_LED_PORT_MAP_T *)ptr_portMap->ptr_portMap, NULL);
#endif
        if (AIR_E_OK != rc)
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize sfp led setting!\n");
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Initializing sfp led setting...\n");
        }
    }
    return rc;
}
#endif

#ifdef AIR_EN_I2C_BITBANG
AIR_ERROR_NO_T
customer_ref_initAppI2cBitbang(
    UI32_T unit)
{
    UI32_T rc = AIR_E_OK;

    rc = i2c_bitbang_init(unit, 1, (I2C_BITBANG_CFG_T *)i2c_bitbang_cfg);
    if(rc != AIR_E_OK)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize i2c bitbang setting!\n", rc);
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Initializing i2c bitbang setting...\n");
    }

    return rc;
}
#endif

void
customer_ref_preInit(void)
{
    const MODULE_CFG_MAP_ITEM_T *ptr_item = _customer_ref_get_cfg();
#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
    const MW_SFP_LED_PORT_MAP_T *ptr_portMap = _customer_ref_getSFPLedPortMap();
#endif

    /* Set Port LED configuration to MW LED. */
    mw_led_setLedCfg((MW_LED_CFG_T *)ptr_item->ptr_sdk_cfg, ptr_item->sdk_cfg_count);
#if defined(AIR_SUPPORT_SFP) && defined(AIR_EN_SFP_LED)
    /* Set SFP LED configuration to MW SFP LED. */
    mw_sfpLed_setPortMap(ptr_portMap);
#endif
}

#ifdef AIR_EN_POE
AIR_ERROR_NO_T customer_ref_initPoePortMap(UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_NOT_SUPPORT;
    const POE_CONFIG_MAP_T *ptr_cfgMap = _customer_ref_getPoeCfgMap();

    if (ptr_cfgMap != NULL)
    {
        rc = air_init_initPoePortMap(unit, ptr_cfgMap->poe_port_map_cnt, ptr_cfgMap->ptr_poe_port_map);
        if (AIR_E_OK != rc)
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize PoE port map setting!\n");
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Initialize PoE port map setting...\n");
        }
    }
    return rc;
}

AIR_ERROR_NO_T customer_ref_initPoeDeviceMap(UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_NOT_SUPPORT;
    const POE_CONFIG_MAP_T *ptr_cfgMap = _customer_ref_getPoeCfgMap();

    if (ptr_cfgMap != NULL)
    {
        rc = air_init_initPoeDeviceMap(unit, ptr_cfgMap->poe_device_map_cnt, ptr_cfgMap->ptr_poe_device_map);
        if (AIR_E_OK != rc)
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize PoE device map setting!\n");
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Initialize PoE device map setting...\n");
        }
    }
    return rc;
}
#endif /* AIR_EN_POE */
