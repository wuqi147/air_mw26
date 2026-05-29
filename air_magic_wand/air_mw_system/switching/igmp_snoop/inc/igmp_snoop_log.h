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

/* FILE NAME:  igmp_snoop_log.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping log.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_LOG_H
#define IGMP_SNOOP_LOG_H
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "mw_log.h"
#include "mw_cmd_util.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMPSNP_DEBUG

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef IGMPSNP_DEBUG
#ifdef IGMP_SNP_MW_SUPPORT
#define IGMP_SNP_LOG_ERROR(fmt, ...)                    MW_LOG_ERROR(IGMP_SNP, "[%s][%s:%d] " fmt, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define IGMP_SNP_LOG_WARN(fmt, ...)                     MW_LOG_WARN(IGMP_SNP, "[%s][%s:%d] " fmt, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define IGMP_SNP_LOG_INFO(fmt, ...)                     MW_LOG_INFO(IGMP_SNP, "[%s][%s:%d] " fmt, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define IGMP_SNP_LOG_DEBUG(fmt, ...)                    MW_LOG_DEBUG(IGMP_SNP, "[%s][%s:%d] " fmt, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)

#else /* IGMP_SNP_MW_SUPPORT */
/* Not support system log */
#ifndef MW_CMD_OUTPUT
#define MW_CMD_OUTPUT(...)                              osapi_xprintf(__VA_ARGS__)
#endif /* MW_CMD_OUTPUT */

#define IGMP_SNP_DBG_PRINT(level, fmt, ...)                           \
                if (level <= igmpsnp_debug_level && level > 0)       \
                    MW_CMD_OUTPUT("%s:%d  " fmt "\n",  __func__, __LINE__, ##__VA_ARGS__ )

#define IGMP_SNP_LOG_ERROR(fmt, ...)                    IGMP_SNP_DBG_PRINT(IGMP_SNP_DEBUG_LEVEL_ERROR,    fmt, ##__VA_ARGS__)
#define IGMP_SNP_LOG_WARN(fmt, ...)                     IGMP_SNP_DBG_PRINT(IGMP_SNP_DEBUG_LEVEL_WARNING,  fmt, ##__VA_ARGS__)
#define IGMP_SNP_LOG_INFO(fmt, ...)                     IGMP_SNP_DBG_PRINT(IGMP_SNP_DEBUG_LEVEL_INFO,     fmt, ##__VA_ARGS__)
#define IGMP_SNP_LOG_DEBUG(fmt, ...)                    IGMP_SNP_DBG_PRINT(IGMP_SNP_DEBUG_LEVEL_DEBUG,    fmt, ##__VA_ARGS__)

#endif /* IGMP_SNP_MW_SUPPORT */

#else /* IGMPSNP_DEBUG */
#define IGMP_SNP_LOG_ERROR(fmt, ...)
#define IGMP_SNP_LOG_WARN(fmt, ...)
#define IGMP_SNP_LOG_INFO(fmt, ...)
#define IGMP_SNP_LOG_DEBUG(fmt, ...)

#endif/* IGMPSNP_DEBUG */

/* DATA TYPE DECLARATIONS
 */
#ifndef IGMP_SNP_MW_SUPPORT
typedef enum
{
    IGMP_SNP_DEBUG_LEVEL_DISABLE = 0,
    IGMP_SNP_DEBUG_LEVEL_ERROR,
    IGMP_SNP_DEBUG_LEVEL_WARNING,
    IGMP_SNP_DEBUG_LEVEL_INFO,
    IGMP_SNP_DEBUG_LEVEL_DEBUG,
    IGMP_SNP_DEBUG_LEVEL_LAST
} IGMP_SNP_DEBUG_LEVEL_T;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#ifndef IGMP_SNP_MW_SUPPORT
extern UI8_T igmpsnp_debug_level;
#endif

/* FUNCTION NAME:   igmp_snp_log_dumpInfo
 * PURPOSE:
 *      This API is used to dump IGMP snooping group and mrouter entry.
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
igmp_snp_log_dumpInfo(
    void);

#endif  /* IGMP_SNOOP_LOG_H */
