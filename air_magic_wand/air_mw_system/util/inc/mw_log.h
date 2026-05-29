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

/* FILE NAME:   mw_log.h
 * PURPOSE:
 *      Declare the common interfaces for mw log.
 * NOTES:
 */

#ifndef MW_LOG_H
#define MW_LOG_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "osapi_string.h"
#include "osapi_printf.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#define MW_LOG_ERROR(module, fmt, ...)             mw_log_print(MW_LOG_MODULE_##module, MW_LOG_LEVEL_ERROR, TRUE, fmt, ##__VA_ARGS__)
#define MW_LOG_WARN(module, fmt, ...)              mw_log_print(MW_LOG_MODULE_##module, MW_LOG_LEVEL_WARNING, TRUE, fmt, ##__VA_ARGS__)
#define MW_LOG_INFO(module, fmt, ...)              mw_log_print(MW_LOG_MODULE_##module, MW_LOG_LEVEL_INFO, TRUE, fmt, ##__VA_ARGS__)
#define MW_LOG_DEBUG(module, fmt, ...)             mw_log_print(MW_LOG_MODULE_##module, MW_LOG_LEVEL_DEBUG, TRUE, fmt, ##__VA_ARGS__)
#define MW_LOG_RAW(module, level, fmt, ...)        mw_log_print(MW_LOG_MODULE_##module, level, FALSE, fmt, ##__VA_ARGS__)
#define MW_LOG_PRINTF(...)                         osapi_xprintf(__VA_ARGS__)
#define MW_LOG_CONSOLE_PRINTF(fmt, ...)            osapi_console_printf(fmt, ##__VA_ARGS__)
#define MW_LOG_INIT_PRINTF(fmt, ...)               osapi_console_printf("[MW INIT] "fmt, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    MW_LOG_MODULE_SYSTEM,
    MW_LOG_MODULE_HTTPD,
    MW_LOG_MODULE_SYNCD,
    MW_LOG_MODULE_CMD,
    MW_LOG_MODULE_LED,
#ifdef AIR_SUPPORT_SNMP
    MW_LOG_MODULE_SNMP,
#endif
#ifdef AIR_SUPPORT_POE
    MW_LOG_MODULE_POE,
#endif
    MW_LOG_MODULE_LAG,
#ifdef AIR_SUPPORT_LACP
    MW_LOG_MODULE_LACP,
#endif
#ifdef AIR_SUPPORT_DHCP_SNOOP
    MW_LOG_MODULE_DHCPSNP,
#endif
#ifdef AIR_SUPPORT_LLDPD
    MW_LOG_MODULE_LLDP,
#endif
    MW_LOG_MODULE_QOS,
#ifdef AIR_SUPPORT_SNTP
    MW_LOG_MODULE_SNTP,
#endif
    MW_LOG_MODULE_STORM_CTRL,
    MW_LOG_MODULE_TRAFFIC_MON,
#ifdef AIR_SUPPORT_CABLE_DIAG
    MW_LOG_MODULE_CABLE_DIAG,
#endif
#ifdef AIR_SUPPORT_REMOTE_DEBUG
    MW_LOG_MODULE_REMOTE_DEBUG,
#endif
#ifdef AIR_SUPPORT_ERPS
    MW_LOG_MODULE_ERPS,
#endif
    MW_LOG_MODULE_DB,
#ifdef AIR_SUPPORT_ICMP_CLIENT
    MW_LOG_MODULE_PING,
#endif
    MW_LOG_MODULE_MAC,
#ifdef AIR_SUPPORT_LP
    MW_LOG_MODULE_LP,
#endif
#ifdef AIR_SUPPORT_IGMP_SNP
    MW_LOG_MODULE_IGMP_SNP,
#endif
#ifdef AIR_SUPPORT_MQTTD
    MW_LOG_MODULE_MQTTD,
#endif
    MW_LOG_MODULE_VLAN,
    MW_LOG_MODULE_PORT_SETTING,
    MW_LOG_MODULE_PORT_MIRROR,
    MW_LOG_MODULE_BANDWIDTH,
    MW_LOG_MODULE_JUMBO_FRAME,
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    MW_LOG_MODULE_STP,
#endif
#ifdef AIR_SUPPORT_SFP
    MW_LOG_MODULE_SFP,
#endif
#ifdef AIR_SUPPORT_IPV6
    MW_LOG_MODULE_IP6,
#endif
#ifdef AIR_SUPPORT_CLI
    MW_LOG_MODULE_CLI,
#endif
#ifdef AIR_SUPPORT_TELNET
    MW_LOG_MODULE_TELNET,
#endif
#ifdef AIR_SUPPORT_SSH
    MW_LOG_MODULE_SSH,
#endif
    MW_LOG_MODULE_LAST
} MW_LOG_MODULE_T;

typedef enum
{
    MW_LOG_LEVEL_OFF = 0,
    MW_LOG_LEVEL_ERROR,
    MW_LOG_LEVEL_WARNING,
    MW_LOG_LEVEL_INFO,
    MW_LOG_LEVEL_DEBUG,
    MW_LOG_LEVEL_LAST
} MW_LOG_LEVEL_T;

#ifdef AIR_SUPPORT_REMOTE_DEBUG
typedef enum MW_LOG_OUTPUT_TYPE_BITMAP_S
{
    MW_LOG_OUTPUT_TYPE_BITMAP_TELNET = 0x1,
    MW_LOG_OUTPUT_TYPE_BITMAP_SSH = 0x2,
    MW_LOG_OUTPUT_TYPE_BITMAP_LAST = 0x4
} MW_LOG_OUTPUT_TYPE_BITMAP_T;
#endif

typedef struct MW_LOG_MODULE_S
{
    const C8_T **ptr_module;
    UI8_T *ptr_log_level;
#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
    UI8_T log_output_type;
#endif
} MW_LOG_SETTING_T;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void mw_log_print(const MW_LOG_MODULE_T module, MW_LOG_LEVEL_T level, BOOL_T format, const C8_T *ptr_fmt, ...);

MW_LOG_SETTING_T *
mw_log_get_setttings(
    void);

MW_ERROR_NO_T
mw_log_show_settings(
    void);

MW_ERROR_NO_T
mw_log_get_level(
    const MW_LOG_MODULE_T module,
    MW_LOG_LEVEL_T *ptr_level);

MW_ERROR_NO_T
mw_log_set_level(
    const MW_LOG_MODULE_T module,
    MW_LOG_LEVEL_T level);

MW_ERROR_NO_T
mw_log_set_global_level(
    MW_LOG_LEVEL_T level);

#if defined AIR_SUPPORT_REMOTE_DEBUG && (defined AIR_SUPPORT_TELNET || defined AIR_SUPPORT_SSH)
MW_ERROR_NO_T
mw_log_set_global_output_type(
    UI32_T type_bitmap);

UI8_T
mw_log_get_global_output_type(
    void);
#endif
#endif  /* End of MW_LOG_H */

