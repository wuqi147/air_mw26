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

/* FILE NAME:   db_cfgfile.c
 * PURPOSE:
 *      Database configuration file handling functions
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdarg.h>
#include "osapi_memory.h"
#include "db_cfgfile.h"
#include "db_util.h"
#include "db_main.h"
#include "db_data.h"
#include "mac_utils.h"
#include "inet_utils.h"
#include "mbedtls/build_info.h"
#include "mbedtls/sha256.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DB_PRINT_KEY(ptr_buf, key_str) \
        osapi_sprintf((ptr_buf), "\"%s\":", (key_str))

/* Use C11 static assertion to check array size at compile time.
 */
#define DB_CHECK_CFGFILE_SIZE(_cfgfile, _expected_f_num) \
    _Static_assert((sizeof(_cfgfile) / sizeof(DB_CFG_T)) == (_expected_f_num), \
                   "The number of fields in " #_cfgfile " does not match DB enum (" #_expected_f_num ") definition!")

#define DB_HASH_PREFIX              "#"
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* The fields in the SYS_INFO table */
static const DB_CFG_T _cfgfile_sys_info[] =
{
    {"name",        CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"ver",         CFGTYPE(JSONString, DB_CFG_MINOR_VER)},
    {"dhcpEn",      CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"autodnsEn",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"sIp",         CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"sMask",       CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"sGw",         CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"sDns",        CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
#ifdef AIR_SUPPORT_SECOND_NETIF
    {"sIf2Ip",      CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"sIf2Mask",    CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    {"mgmt_vlan",   CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
#endif
#ifdef AIR_SUPPORT_IPV6
    {"prefixLen",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"ip6Addr",     CFGTYPE(JSONString, DB_CFG_MINOR_IPV6_ADDR)},
    {"ip6Gw",       CFGTYPE(JSONString, DB_CFG_MINOR_IPV6_ADDR)},
    {"ip6Mode",     CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_sys_info, SYS_INFO_LAST);

/* The fields in the ACCOUNT_INFO table */
static const DB_CFG_T _cfgfile_account_info[] =
{
    {"user",    CFGTYPE(JSONString, DB_CFG_MINOR_NAME)},
    {"passwd",  CFGTYPE(JSONString, DB_CFG_MINOR_PASSWD)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_account_info, ACC_INFO_LAST);

/* The fields in the PORT_CFG_INFO table */
static const DB_CFG_T _cfgfile_port_cfg[] =
{
    {"type",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"status",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"settings", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORT_SETTINGS)},
    {"isolate", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {"inRate",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"egRate",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"greenEt", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"eeeEn",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"pvid",    CFGTYPE(JSONNumber, DB_CFG_MINOR_PVID)},
    {"trunkId", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE4)},
    {"mirrId",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"vlans",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"macLim",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE100)},
    {"stormBR", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1M)},
    {"stormMR", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1M)},
    {"stormUR", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1M)},
    {"stormBM", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"stormMM", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"stormUM", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"stormBC", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"stormMC", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"stormUC", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"mrouter", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"inFilt",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_port_cfg, PORT_CFG_INFO_LAST);

/* The fields in the PORT_QOS table */
static const DB_CFG_T _cfgfile_port_qos[] =
{
    {"pri",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE7)},
    {"q0WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q1WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q2WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q3WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q4WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q5WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q6WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q7WWrr",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q0WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q1WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q2WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q3WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q4WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q5WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q6WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"q7WWfq",  CFGTYPE(JSONNumber, DB_CFG_MINOR_WEIGHT)},
    {"qosSche", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_port_qos, PORT_QOS_LAST);

/* The fields in the TRUNK_PORT table */
static const DB_CFG_T _cfgfile_trunk_port[] =
{
    {"members", CFGTYPE(JSONArray, DB_CFG_MINOR_TRUNK_MEMBERS)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_trunk_port, TRUNK_PORT_LAST);

/* The fields in the TRUNK_ALGORITHM table */
static const DB_CFG_T _cfgfile_trunk_algo[] =
{
    {"algorithm", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_trunk_algo, TRUNK_ALGORITHM_LAST);

#ifdef AIR_SUPPORT_LP
/* The fields in the LOOP_PREVEN_INFO table */
static const DB_CFG_T _cfgfile_loop_preven[] =
{
    {"enable",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_loop_preven, LOOP_PREVEN_INFO_LAST);
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_IGMP_SNP
/* The fields in the IGMP_SNP_INFO table */
static const DB_CFG_T _cfgfile_igmp_snp[] =
{
    {"enable",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"rptSupp", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"fastLev", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_igmp_snp, IGMP_SNP_INFO_LAST);

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
/* The fields in the IGMP_SNP_QUERIER_INFO table */
static const DB_CFG_T _cfgfile_igmp_snp_querier[] =
{
    {"Vid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},     /* VLAN ID of IGMP snooping querier*/
    {"State",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)}, /* State of IGMP snooping querier*/
    {"Elect",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)}, /* Supported Election of IGMP snooping querier */
    {"Ver",     CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)}, /* Supported version of IGMP snooping querier */
    {"Addr",    CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},    /* IGMP snooping querier's address */
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_igmp_snp_querier, IGMP_SNP_QUERIER_INFO_LAST);
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */

/* The fields in the PORT_MIRROR_INFO table */
static const DB_CFG_T _cfgfile_port_mirror[] =
{
    {"enable",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"dPort",   CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTNUM)},
    {"sInPort", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {"sEgPort", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_port_mirror, PORT_MIRROR_INFO_LAST);

/* The fields in the JUMBO_FRAME_INFO table */
static const DB_CFG_T _cfgfile_jumbo_frame[] =
{
    {"cfg", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE11)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_jumbo_frame, JUMBO_FRAME_INFO_LAST);

/* The fields in the VLAN_CFG_INFO table */
static const DB_CFG_T _cfgfile_vlan_cfg[] =
{
    {"portBEn", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"8021qEn", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"mtuEn",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_vlan_cfg, VLAN_CFG_INFO_LAST);

/* The fields in the VLAN_ENTRY table */
static const DB_CFG_T _cfgfile_vlan_entry[] =
{
    {"vid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    {"fid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
#endif
    {"descr",   CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"ports",   CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {"tagPs",   CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {"untagPs", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_vlan_entry, VLAN_ENTRY_LAST);

#ifdef AIR_SUPPORT_VOICE_VLAN
/* The fields in the VOICE_VLAN_INFO table */
static const DB_CFG_T _cfgfile_voice_vlan[] =
{
    {"state",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"vid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"pri",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE7)},
#ifdef AIR_SUPPORT_VOICE_VLAN
    {"prtMode", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    {"secMode", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
#endif
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_voice_vlan, VOICE_VLAN_INFO_LAST);
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/* The fields in the SURVEI_VLAN_INFO table */
static const DB_CFG_T _cfgfile_survei_vlan[] =
{
    {"state",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"vid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"pri",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE7)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_survei_vlan, SURVEI_VLAN_INFO_LAST);
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
/* The fields in the OUI_ENTRY table */
static const DB_CFG_T _cfgfile_oui_entry[] =
{
    {"mac",     CFGTYPE(JSONString, DB_CFG_MINOR_OUI)},
    {"type",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"descr",   CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_oui_entry, OUI_ENTRY_LAST);
#endif
/* The fields in the QOS_INFO table */
static const DB_CFG_T _cfgfile_qos_info[] =
{
    {"qosMode", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"pri2Q",   CFGTYPE(JSONArray,  DB_CFG_MINOR_RANGE7)},
    {"dscp2pr", CFGTYPE(JSONArray,  DB_CFG_MINOR_RANGE7)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_qos_info, QOS_INFO_LAST);

/* The fields in the STATIC_MAC_ENTRY table */
static const DB_CFG_T _cfgfile_static_macentry[] =
{
    {"mac",     CFGTYPE(JSONString, DB_CFG_MINOR_MAC)},
    {"vid",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"port",    CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTNUM)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_static_macentry, STATIC_MAC_ENTRY_LAST);

#ifdef AIR_SUPPORT_DHCP_SNOOP
/* The fields in the DHCP_SNP_INFO table */
static const DB_CFG_T _cfgfile_dhcp_snp[] =
{
    {"enable", CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_dhcp_snp, DHCP_SNP_INFO_LAST);

/* The fields in the DHCP_SNP_PORT_INFO table */
static const DB_CFG_T _cfgfile_dhcp_snp_port[] =
{
    {"trstPs",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"en",      CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"op",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"ctype",   CFGTYPE(JSONNumber, DB_CFG_MINOR_DHCPSNPC)},
    {"cid",     CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"rtype",   CFGTYPE(JSONNumber, DB_CFG_MINOR_DHCPSNPR)},
    {"rid",     CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_dhcp_snp_port, DHCP_SNP_PORT_INFO_LAST);
#endif /* AIR_SUPPORT_DHCP_SNOOP */

#ifdef AIR_SUPPORT_SNMP
static const DB_CFG_T _cfgfile_snmp[] =
{
    {"ver",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"trapName",  CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"trapEn",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"trapTy",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE7)},
    {"trapIp",    CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"readCm",    CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"writeCm",   CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"trapCm",    CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_snmp, SNMP_LAST);
#endif
/* Below tables will not keep in configuration file */
static const DB_CFG_T _cfgfile_sys_oper_info[] =
{
    {"mac",       CFGTYPE(JSONString, DB_CFG_MINOR_MAC)},
    {"hwVer",     CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"ip",        CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"mask",      CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"gw",        CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"dns",       CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
#ifdef AIR_SUPPORT_SECOND_NETIF
    {"if2Ip",      CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"if2Mask",    CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
#endif
#ifdef AIR_SUPPORT_IPV6
    {"ip61",      CFGTYPE(JSONString, DB_CFG_MINOR_IPV6_ADDR)},
    {"ip62",      CFGTYPE(JSONString, DB_CFG_MINOR_IPV6_ADDR)},
    {"linklocal", CFGTYPE(JSONString, DB_CFG_MINOR_IPV6_ADDR)},
    {"dad",       CFGTYPE(JSONString, DB_CFG_MINOR_RANGE255)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_sys_oper_info, SYS_OPER_INFO_LAST);

static const DB_CFG_T _cfgfile_port_oper_info[] =
{
    {"status",    CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"speed",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE4)},
    {"duplex",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"flowCtr",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {"loopSt",    CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"router",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"clrMib",    CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"mode",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"lacpSt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE2)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_port_oper_info, PORT_OPER_INFO_LAST);

static const DB_CFG_T _cfgfile_lag_oper_info[] =
{
    {"lag_oper",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_lag_oper_info, LAG_OPER_INFO_LAST);

static const DB_CFG_T _cfgfile_lag_member_0_info[] =
{
    {"lag_mem_0", CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTNUM)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_lag_member_0_info, LAG_MEMBER_0_LAST);

#ifdef AIR_SUPPORT_CABLE_DIAG
static const DB_CFG_T _cfgfile_port_diag[] =
{
    {"set",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"length",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"state",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"impednce",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"port",      CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTNUM)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_port_diag, PORT_CABLE_DIAG_LAST);
#endif

static const DB_CFG_T _cfgfile_logon_info[] =
{
    {"logonFlCnt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_logon_info, LOGON_INFO_LAST);

static const DB_CFG_T _cfgfile_l2_mc_entry[] =
{
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    {"mac",       CFGTYPE(JSONString, DB_CFG_MINOR_MAC)},
#else
    {"ip",        CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
#endif
    {"vid",       CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"ports",     CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {"flag",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_l2_mc_entry, L2_MC_ENTRY_LAST);

static const DB_CFG_T _cfgfile_system[] =
{
    {"saveRun",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"reset",     CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"factory",   CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_system, SYSTEM_LAST);

#ifdef AIR_SUPPORT_ICMP_CLIENT
static const DB_CFG_T _cfgfile_icmp_client_info[] =
{
    {"hostname",  CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"pingIp",    CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"num",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"status",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"sndPkt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"rcvPkt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"lostPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"minRtt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"maxRtt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"aveRtt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_icmp_client_info, ICMP_CLIENT_INFO_LAST);
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
static const DB_CFG_T _cfgfile_sfp_module_info[] =
{
    {"portNum",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"ddmType",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"temp",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"voltage",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"current",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"txPwr",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"rxPwr",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE80K)},
    {"lossSig",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_sfp_module_info, SFP_MODULE_INFO_LAST);
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
static const DB_CFG_T _cfgfile_voice_oper_info[] =
{
    {"members",   CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTMAP)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_voice_oper_info, VOICE_OPER_INFO_LAST);
#endif

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* The fields in the RSTP_INFO table */
static const DB_CFG_T _cfgfile_stp[] =
{
    {"enable",    CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"frcVer",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_FORCE_VER)},
    {"fwdDelay",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_FORWARD_DELAY)},
    {"maxAge",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_MAX_AGE)},
    {"txHoldCnt", CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_TX_HOLD_CNT)},
    {"bpri",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_PRIORITY)},
#ifdef AIR_SUPPORT_STP_AUTO_COST
    {"autoCstS",  CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
#endif
#ifdef AIR_SUPPORT_RSTP_SECURITY
    {"bgState",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"rgState",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"lgState",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
#endif
    {"portMode", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_stp, STP_INFO_LAST);

/* The fields in the RSTP_PORT_INFO table */
static const DB_CFG_T _cfgfile_stp_port[] =
{
    {"ppri",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_PORT_PRIORITY)},
    {"cost",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_PORT_COST)},
#ifdef AIR_SUPPORT_STP_AUTO_COST
    {"autoCstM",  CFGTYPE(JSONTrue, DB_CFG_MINOR_RESERVE)},
#endif
    {"admEdge",   CFGTYPE(JSONTrue, DB_CFG_MINOR_RESERVE)},
    {"mcheck",    CFGTYPE(JSONTrue, DB_CFG_MINOR_RESERVE)},
    {"status",    CFGTYPE(JSONTrue, DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_stp_port, STP_PORT_INFO_LAST);

/* The fields in the RSTP_PORT_OPER_INFO table */
static const DB_CFG_T _cfgfile_stp_port_oper[] =
{
    {"state",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"role",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"block",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#ifdef AIR_SUPPORT_STP_AUTO_COST
    {"operCost",CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_stp_port_oper, STP_PORT_OPER_INFO_LAST);

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* The fields in the RSTP_PORT_SECURITY table */
static const DB_CFG_T _cfgfile_rstp_port_sec[] =
{
    {"bg-status",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"tc-status",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"rg-status",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"lg-status",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"interval",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_PORT_TC_INTERVAL)},
    {"threshold",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE100)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_rstp_port_sec, RSTP_PORT_SEC_LAST);
#endif

#ifdef AIR_SUPPORT_MSTP
/* The fields in the MSTP_INSTANCE table */
static const DB_CFG_T _cfgfile_mstp_region[] =
{
    {"revision", CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"rgName",   CFGTYPE(JSONString, DB_CFG_MINOR_MSTP_REGION_NAME)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mstp_region, MSTP_REGION_LAST);

/* The fields in the MSTP_INSTANCE table */
static const DB_CFG_T _cfgfile_mstp_instance[] =
{
    {"instance",        CFGTYPE(JSONNumber, DB_CFG_MINOR_MSTP_INSTANCE)},
    {"ins-pri",         CFGTYPE(JSONNumber, DB_CFG_MINOR_RSTP_PRIORITY)},
    {"vlanbmp",         CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mstp_instance, MSTP_INSTANCE_LAST);

/* The fields in the MSTP_INSTANCE_PORT table */
static const DB_CFG_T _cfgfile_mstp_instance_port[] =
{
    {"ins-ppri",        CFGTYPE(JSONArray, DB_CFG_MINOR_RSTP_PORT_PRIORITY)},
    {"ins-pcost",       CFGTYPE(JSONArray, DB_CFG_MINOR_RSTP_PORT_COST)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mstp_instance_port, MSTP_INSTANCE_PORT_LAST);

/* The fields in the MSTP_INSTANCE_PORT_OPER table */
static const DB_CFG_T _cfgfile_mstp_instance_port_oper[] =
{
    {"ins-state",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"ins-role",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#ifdef AIR_SUPPORT_STP_AUTO_COST
    {"ins-ocost",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mstp_instance_port_oper, MSTP_INSTANCE_PORT_OPER_INFO_LAST);
#endif

#endif /* AIR_SUPPORT_RSTP */

#ifdef AIR_SUPPORT_LLDPD
static const DB_CFG_T _cfgfile_lldp_info[] =
{
    {"gloEn",      CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {"txHold",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE11)},
    {"txInter",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"reInitDelay",CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"txdelay",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_lldp_info, LLDP_INFO_LAST);

static const DB_CFG_T _cfgfile_lldp_port_info[] =
{
    {"enable",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE3)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_lldp_port_info, LLDP_PORT_INFO_LAST);

static const DB_CFG_T _cfgfile_lldp_client_info[] =
{
    {"chassId",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"portId",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"leaveTime",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"sysName",    CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"medCapabi",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"medNetPol",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"medExtPow",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_lldp_client_info, LLDP_CLIENT_INFO_LAST);
#endif
#ifdef AIR_SUPPORT_POE
static const DB_CFG_T _cfgfile_poe_cfg[] =
{
    {"totAvaPwr",            CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
#ifdef AIR_SUPPORT_POE_WATCHDOG
    {"poeWDtime",            CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"poeWDthld",            CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_poe_cfg, POE_CFG_LAST);

static const DB_CFG_T _cfgfile_poe_port_cfg[] =
{
    {"pwrCtrl",                CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"AvPwrMode",              CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"AvaPwr",                 CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"priority",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"delayTime",              CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"AfAtMode",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
#ifdef AIR_SUPPORT_POE_WATCHDOG
    {"poeWDen",                CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
#endif
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_poe_port_cfg, POE_PORT_CFG_LAST);

static const DB_CFG_T _cfgfile_poe_status[] =
{
    {"supplyV",                 CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"totCmPwr",                CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"totCmCur",                CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"isPoePort",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {"pwrStrategy",             CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_poe_status, POE_STATUS_LAST);

static const DB_CFG_T _cfgfile_poe_port_status[] =
{
    {"pwrStatus",             CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"pdClass",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"CmPwr",                 CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"portVol",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"portCur",               CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE65535)},
    {"pwrEvent",              CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_poe_port_status, POE_PORT_STATUS_LAST);
#endif
static const DB_CFG_T _cfgfile_mib_cnt[] =
{
    {"rxPkt",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#ifdef AIR_SUPPORT_SNMP
    {"rxUCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"rxMCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"rxBCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"rxDisPkt",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#endif
    {"rxOctet",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"rxError",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"txPkt",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#ifdef AIR_SUPPORT_SNMP
    {"txUCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"txMCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"txBCPkt",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"txDisPkt",  CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
#endif
    {"txOctet",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"txError",   CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mib_cnt, MIB_CNT_LAST);

static const DB_CFG_T _cfgfile_dynamic_mac_address_entry_cfg[] =
{
    {"actRst",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"etyCnt",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_dynamic_mac_address_entry_cfg, DYNAMIC_MAC_ADDRESS_ENTRY_CFG_LAST);

static const DB_CFG_T _cfgfile_dynamic_mac_address_entry[] =
{
    {"mac_addr",     CFGTYPE(JSONString, DB_CFG_MINOR_MAC)},
    {"vid",          CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"port",         CFGTYPE(JSONNumber, DB_CFG_MINOR_PORTNUM)},
    {"age",          CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGEMAX)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_dynamic_mac_address_entry, DYNAMIC_MAC_ADDRESS_ENTRY_LAST);

#ifdef AIR_SUPPORT_MQTTD
static const DB_CFG_T _cfgfile_mqttd_cfg[] =
{
    {"enable",       CFGTYPE(JSONTrue,   DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_mqttd_cfg, MQTTD_CFG_INFO_LAST);
#endif

#ifdef AIR_SUPPORT_SNTP
static const DB_CFG_T _cfgfile_sntp_cfg[] =
{
    {"mode",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"timezone",    CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"server1",     CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"server2",     CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {"server3",     CFGTYPE(JSONString, DB_CFG_MINOR_ADDR)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_sntp_cfg, SNTP_CFG_LAST);
#endif
#ifdef AIR_SUPPORT_ERPS
static const DB_CFG_T _cfgfile_erps_info[] =
{
    {"ringId",          CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"controlVlan",     CFGTYPE(JSONNumber, DB_CFG_MINOR_VID)},
    {"dataVlan",        CFGTYPE(JSONString, DB_CFG_MINOR_STRING)},
    {"revertMode",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE1)},
    {"westPort",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"eastPort",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"wtrTimer",        CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"guardTimer",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE255)},
    {"holdOfTimer",     CFGTYPE(JSONNumber, DB_CFG_MINOR_RANGE100)},
    {"wPortState",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"ePortState",      CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {"instState",       CFGTYPE(JSONNumber, DB_CFG_MINOR_RESERVE)},
    {{0}, 0}
};
DB_CHECK_CFGFILE_SIZE(_cfgfile_erps_info, ERPS_INFO_LAST);
#endif /*AIR_SUPPORT_ERPS*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
static const C8_T * _db_cfg_getTableName(UI8_T t_idx);
static const DB_CFG_T *_db_cfg_getFieldnames(UI8_T t_idx);

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROMGRAM BODIES
*/
static const C8_T *
_db_cfg_getTableName(
    UI8_T t_idx)
{
    switch (t_idx)
    {
        case SYS_INFO:
            return "sysInfo";

        case ACCOUNT_INFO:
            return "account";

        case PORT_CFG_INFO:
            return "portCfg";

        case PORT_QOS:
            return "portQos";

        case TRUNK_PORT:
            return "trunk";

        case TRUNK_ALGORITHM:
            return "lagAlgo";

#ifdef AIR_SUPPORT_LP
        case LOOP_PREVEN_INFO:
            return "loopPre";
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_IGMP_SNP
        case IGMP_SNP_INFO:
            return "igmpSnp";
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
        case IGMP_SNP_QUERIER_INFO:
            return "igmpQur";
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */

        case PORT_MIRROR_INFO:
            return "portMir";

        case JUMBO_FRAME_INFO:
            return "jumbFrm";

        case VLAN_CFG_INFO:
            return "vlanCfg";

        case VLAN_ENTRY:
            return "vlanEty";
#ifdef AIR_SUPPORT_VOICE_VLAN
        case VOICE_VLAN_INFO:
            return "voVlan";
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
        case SURVEI_VLAN_INFO:
            return "surVlan";
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
        case OUI_ENTRY:
            return "ouiEty";
#endif
        case QOS_INFO:
            return "qosInfo";

        case STATIC_MAC_ENTRY:
            return "sMacEty";
#ifdef AIR_SUPPORT_DHCP_SNOOP
        case DHCP_SNP_INFO:
            return "dhcpSnp";

        case DHCP_SNP_PORT_INFO:
            return "dsPort";
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
        case SNMP_INFO:
            return "snmpInfo";
#endif
#ifdef AIR_SUPPORT_POE
        case POE_CFG:
            return "poeCfg";
        case POE_PORT_CFG:
            return "poePCfg";
        case POE_STATUS:
            return "poeSt";
        case POE_PORT_STATUS:
            return "poePSt";
#endif
        /* Below tables will not keep in configuration file */
        case SYS_OPER_INFO:
            return "sysOper";
        case PORT_OPER_INFO:
            return "portOper";
        case LAG_OPER_INFO:
            return "lagOper";
        case LAG_MEMBER_0_INFO:
            return "lagMem0";
#ifdef AIR_SUPPORT_CABLE_DIAG
        case PORT_DIAG:
            return "portDiag";
#endif
        case LOGON_INFO:
            return "logonInfo";
        case L2_MC_ENTRY:
            return "l2MC";
        case SYSTEM:
            return "system";
#ifdef AIR_SUPPORT_ICMP_CLIENT
        case ICMP_CLIENT_INFO:
            return "icmpInfo";
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
        case SFP_MODULE_INFO:
            return "sfpInfo";
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
        case VOICE_OPER_INFO:
            return "voVlanOp";
#endif
        case MIB_CNT:
            return "mib";
        case DYNAMIC_MAC_ADDRESS_ENTRY_CFG:
            return "dMacEtyCfg";
        case DYNAMIC_MAC_ADDRESS_ENTRY:
            return "dMacEty";
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
        case STP_INFO:
            return "rstpGlb";
        case STP_PORT_INFO:
            return "rstpPort";
        case STP_PORT_OPER_INFO:
            return "rstpPrtOp";
#ifdef AIR_SUPPORT_RSTP_SECURITY
        case RSTP_PORT_SECURITY:
            return "rstpSec";
#endif
#ifdef AIR_SUPPORT_MSTP
        case MSTP_REGION:
            return "mstpReg";
        case MSTP_INSTANCE:
            return "mstpIns";
        case MSTP_INSTANCE_PORT:
            return "mstpPort";
        case MSTP_PORT_OPER_INFO:
            return "mstpPrtOp";
#endif
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
        case MQTTD_CFG_INFO:
            return "mqttdCfg";
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_LLDPD
        case LLDP_INFO:
            return "lldpGlb";
        case LLDP_PORT_INFO:
            return "lldpPort";
        case LLDP_CLIENT_INFO:
            return "lldpClien";
#endif
#ifdef AIR_SUPPORT_SNTP
        case SNTP_CFG:
            return "sntpCfg";
#endif
#ifdef AIR_SUPPORT_ERPS
        case ERPS_INFO:
            return "erpsInfo";
#endif
        default:
            return "null";
    }
}

static const DB_CFG_T*
_db_cfg_getFieldnames(
    UI8_T t_idx)
{
    switch (t_idx)
    {
        case SYS_INFO:
            return _cfgfile_sys_info;

        case ACCOUNT_INFO:
            return _cfgfile_account_info;

        case PORT_CFG_INFO:
            return _cfgfile_port_cfg;

        case PORT_QOS:
            return _cfgfile_port_qos;

        case TRUNK_PORT:
            return _cfgfile_trunk_port;

        case TRUNK_ALGORITHM:
            return _cfgfile_trunk_algo;

#ifdef AIR_SUPPORT_LP
        case LOOP_PREVEN_INFO:
            return _cfgfile_loop_preven;
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_IGMP_SNP
        case IGMP_SNP_INFO:
            return _cfgfile_igmp_snp;

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
        case IGMP_SNP_QUERIER_INFO:
            return _cfgfile_igmp_snp_querier;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */

        case PORT_MIRROR_INFO:
            return _cfgfile_port_mirror;

        case JUMBO_FRAME_INFO:
            return _cfgfile_jumbo_frame;

        case VLAN_CFG_INFO:
            return _cfgfile_vlan_cfg;

        case VLAN_ENTRY:
            return _cfgfile_vlan_entry;
#ifdef AIR_SUPPORT_VOICE_VLAN
        case VOICE_VLAN_INFO:
            return _cfgfile_voice_vlan;
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
        case SURVEI_VLAN_INFO:
            return _cfgfile_survei_vlan;
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
        case OUI_ENTRY:
            return _cfgfile_oui_entry;
#endif
        case QOS_INFO:
            return _cfgfile_qos_info;

        case STATIC_MAC_ENTRY:
            return _cfgfile_static_macentry;
#ifdef AIR_SUPPORT_DHCP_SNOOP
        case DHCP_SNP_INFO:
            return _cfgfile_dhcp_snp;

        case DHCP_SNP_PORT_INFO:
            return _cfgfile_dhcp_snp_port;
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
        case SNMP_INFO:
            return _cfgfile_snmp;
#endif
#ifdef AIR_SUPPORT_MQTTD
        case MQTTD_CFG_INFO:
            return _cfgfile_mqttd_cfg;
#endif
        /* Below tables will not keep in configuration file */
        case SYS_OPER_INFO:
            return _cfgfile_sys_oper_info;
        case PORT_OPER_INFO:
            return _cfgfile_port_oper_info;
        case LAG_OPER_INFO:
            return _cfgfile_lag_oper_info;
        case LAG_MEMBER_0_INFO:
            return _cfgfile_lag_member_0_info;
#ifdef AIR_SUPPORT_CABLE_DIAG
        case PORT_DIAG:
            return _cfgfile_port_diag;
#endif
        case LOGON_INFO:
            return _cfgfile_logon_info;
        case L2_MC_ENTRY:
            return _cfgfile_l2_mc_entry;
        case SYSTEM:
            return _cfgfile_system;
#ifdef AIR_SUPPORT_ICMP_CLIENT
        case ICMP_CLIENT_INFO:
            return _cfgfile_icmp_client_info;
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
        case SFP_MODULE_INFO:
            return _cfgfile_sfp_module_info;
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
        case VOICE_OPER_INFO:
            return _cfgfile_voice_oper_info;
#endif
        case MIB_CNT:
            return _cfgfile_mib_cnt;
        case DYNAMIC_MAC_ADDRESS_ENTRY_CFG:
            return _cfgfile_dynamic_mac_address_entry_cfg;
        case DYNAMIC_MAC_ADDRESS_ENTRY:
            return _cfgfile_dynamic_mac_address_entry;
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
        case STP_INFO:
            return _cfgfile_stp;
        case STP_PORT_INFO:
            return _cfgfile_stp_port;
        case STP_PORT_OPER_INFO:
            return _cfgfile_stp_port_oper;
#ifdef AIR_SUPPORT_RSTP_SECURITY
        case RSTP_PORT_SECURITY:
            return _cfgfile_rstp_port_sec;
#endif
#ifdef AIR_SUPPORT_MSTP
        case MSTP_REGION:
            return _cfgfile_mstp_region;
        case MSTP_INSTANCE:
            return _cfgfile_mstp_instance;
        case MSTP_INSTANCE_PORT:
            return _cfgfile_mstp_instance_port;
        case MSTP_PORT_OPER_INFO:
            return _cfgfile_mstp_instance_port_oper;
#endif
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_LLDPD
        case LLDP_INFO:
            return _cfgfile_lldp_info;
        case LLDP_PORT_INFO:
            return _cfgfile_lldp_port_info;
        case LLDP_CLIENT_INFO:
            return _cfgfile_lldp_client_info;
#endif
#ifdef AIR_SUPPORT_POE
        case POE_CFG:
            return _cfgfile_poe_cfg;
        case POE_PORT_CFG:
            return _cfgfile_poe_port_cfg;
        case POE_STATUS:
            return _cfgfile_poe_status;
        case POE_PORT_STATUS:
            return _cfgfile_poe_port_status;
#endif
#ifdef AIR_SUPPORT_SNTP
        case SNTP_CFG:
            return _cfgfile_sntp_cfg;
#endif
#ifdef AIR_SUPPORT_ERPS
        case ERPS_INFO:
            return _cfgfile_erps_info;
#endif
        default:
            return NULL;
    }
}

static MW_ERROR_NO_T
_db_cfgfile_json_to_raw (
    const DB_REQUEST_TYPE_T req,
    const UI8_T field_size,
    void *ptr_raw,
    C8_T *ptr_value,
    C8_T *ptr_value_str,
    const UI32_T value_len,
    const JSONTypes_t value_type)
{
    UI8_T           cur_fidx = req.f_idx - 1;
    UI8_T           major = JSONInvalid;
    UI8_T           chardata = 0;
    UI16_T          minor = DB_CFG_MINOR_LAST;
    UI16_T          shortdata = 0;
    UI32_T          intdata = 0;
#ifdef AIR_SUPPORT_MSTP
    UI32_T          key_len = 0;
#endif
    ether_addr_t    mac;
    MW_ERROR_NO_T   ret = MW_E_OK;
    const DB_CFG_T  *ptr_fieldnames;

    if((NULL == ptr_raw) || (NULL == ptr_value) || (NULL == ptr_value_str))
    {
        return MW_E_BAD_PARAMETER;
    }
    ptr_fieldnames = _db_cfg_getFieldnames(req.t_idx);
    if (0 == osapi_strlen(ptr_fieldnames[cur_fidx].ptr_name))
    {
        return MW_E_BAD_PARAMETER;
    }
    /* Get type */
    major = CFGMAJOR(ptr_fieldnames[cur_fidx].type);
    minor = CFGMINOR(ptr_fieldnames[cur_fidx].type);

    switch (value_type)
    {
        case JSONString:
            if((DB_CFG_MINOR_MAC == minor) ||
                (DB_CFG_MINOR_OUI == minor))
            {
                osapi_memset(&mac, 0, sizeof(ether_addr_t));
                if(strToMac((UI8_T *)ptr_value_str, &mac))
                {
                    osapi_memcpy(ptr_raw, mac.octet, field_size);
                }
                else
                {
                    DB_LOG_ERROR("%s:%s, Minor parsing fail. Type[%d:%d]", ptr_fieldnames[cur_fidx].ptr_name, ptr_value_str, major, minor);
                    ret = MW_E_NOT_SUPPORT;
                }
            }
            else if(DB_CFG_MINOR_VER == minor)
            {
                mw_get_version(ptr_raw);
                if (0 != osapi_strcmp(ptr_value_str, ptr_raw))
                {
                    DB_LOG_INFO("The config file version[%s] is different to image's verson[%s]", ptr_value_str, ptr_raw);
                }
            }
            else if((DB_CFG_MINOR_NAME == minor) ||
                    (DB_CFG_MINOR_PASSWD == minor))
            {
                if (0 == value_len)
                {
                    /* If the account is empty, user cannot login to web */
                    DB_LOG_ERROR("The user account is invalid: %s", ptr_value_str);
                    ret = MW_E_NOT_SUPPORT;
                    break;
                }
                if(DB_CFG_MINOR_PASSWD == minor)
                {
                    /* Detect the data is HEX string or plain text */
                    if (0 == osapi_strncmp(ptr_value_str, DB_HASH_PREFIX, osapi_strlen(DB_HASH_PREFIX)))
                    {
                        /* convert HEX string to hashed data */
                        UI8_T  mchar = 0;
                        UI8_T  *ptr_passwdRaw = (UI8_T *)ptr_raw;
                        UI16_T field_len = field_size;

                        if (value_len != (osapi_strlen(DB_HASH_PREFIX) + (2 * field_size)))
                        {
                            /* If the account is empty, user cannot login to web */
                            DB_LOG_ERROR("The password size is not correct: %d", value_len);
                            ret = MW_E_NOT_SUPPORT;
                            break;
                        }
                        ptr_value = ptr_value_str + osapi_strlen(DB_HASH_PREFIX);
                        while (field_len > 0)
                        {
                            mchar = charToInt(*ptr_value++);
                            if (mchar > 0x0F)
                            {
                                /* If the account is empty, user cannot login to web */
                                DB_LOG_ERROR("The value is invalid: %s", ptr_value_str);
                                ret = MW_E_NOT_SUPPORT;
                                break;
                            }
                            chardata = charToInt(*ptr_value++);
                            if (chardata > 0x0F)
                            {
                                /* If the account is empty, user cannot login to web */
                                DB_LOG_ERROR("The value is invalid: %s", ptr_value_str);
                                ret = MW_E_NOT_SUPPORT;
                                break;
                            }
                            *ptr_passwdRaw = (mchar << 4U) | chardata;
                            ptr_passwdRaw++;
                            field_len--;
                        }
                    }
                    else
                    {
#ifdef MBEDTLS_SHA256_C
                        /* Use SHA-256 to hash plain-text password */
                        UI8_T hashPass[MAX_PASSWORD_SIZE] = {0};
                        if (MW_E_OK != mbedtls_sha256((const UI8_T *)ptr_value_str,
                                    osapi_strlen(ptr_value_str),
                                    hashPass,
                                    FALSE))
                        {
                            DB_LOG_ERROR("%s, failed to convert the password", DB_INTERNAL_ERROR);
                            ret = MW_E_NOT_SUPPORT;
                            break;
                        }
                        osapi_memcpy(ptr_raw, hashPass, field_size);
#else
                        ret = MW_E_NOT_SUPPORT;
                        break;
#endif
                    }
                }
                else if (DB_CFG_MINOR_MSTP_REGION_NAME == minor)
                {
                    osapi_memcpy(ptr_raw, ptr_value_str, field_size);
                    *((UI8_T *)(ptr_raw + field_size - 1)) = '\0';
                }
                else
                {
                    osapi_strncpy(ptr_raw, ptr_value_str, field_size);
                    *((UI8_T *)(ptr_raw + field_size - 1)) = '\0';
                }
            }
            else if(DB_CFG_MINOR_ADDR == minor)
            {
                intdata = getIpaddr((const char *)ptr_value_str);
                osapi_memcpy(ptr_raw, &intdata, field_size);
            }
#ifdef AIR_SUPPORT_IPV6
            else if(DB_CFG_MINOR_IPV6_ADDR == minor)
            {
                ip6_addr_t ipv6_ip = {{0}};

                inet6_aton((const char *)ptr_value_str, &ipv6_ip);
                if (sizeof(ipv6_ip) == field_size)
                {
                    osapi_memcpy(ptr_raw, &ipv6_ip, field_size);
                }
                else
                {
                    DB_LOG_ERROR("Ipv6 data size error, expected size: %d, actual size: %d", field_size, sizeof(ipv6_ip));
                }
            }
#endif
            else
            {
                osapi_strncpy(ptr_raw, ptr_value_str, field_size);
                *((UI8_T *)(ptr_raw + field_size - 1)) = '\0';
            }
            break;
        case JSONNumber:
            if (field_size == sizeof(UI8_T))
            {
                chardata = atoi(ptr_value_str);
                switch(minor)
                {
                    case DB_CFG_MINOR_RANGE1:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE1_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE2:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE2_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE3:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE3_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE4:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE4_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE7:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE7_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE11:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE11_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE100:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE100_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE255:
                        break;
                    case DB_CFG_MINOR_SPD_ABILITY:
                        if((DB_CFG_MINOR_RANGE_MIN1 > chardata) || (DB_CFG_MINOR_RANGESPD_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_WEIGHT:
                        if((DB_CFG_MINOR_RANGE_MIN1 > chardata) || (DB_CFG_MINOR_RANGEWIEGHT_MAX < chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_PORTNUM:
                        if(PLAT_MAX_PORT_NUM < chardata)
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_DHCPSNPC:
                        if((DB_CFG_MINOR_RANGE_MIN0 != chardata) && (DB_CFG_MINOR_RANGEDHCP_MAX != chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_DHCPSNPR:
                        if((DB_CFG_MINOR_RANGE_MIN0 != chardata) && (DB_CFG_MINOR_RANGE_MIN1 != chardata) && (DB_CFG_MINOR_RANGEDHCP_MAX != chardata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_PORT_PRIORITY:
                        if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE240_MAX < chardata) || (0 != (chardata % DB_CFG_MINOR_RANGE_STEP16)))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RESERVE:
                        /* Do not check */
                        break;
                    default:
                        ret = MW_E_NOT_SUPPORT;
                        DB_LOG_ERROR("%s, CHAR number mismatch,%s: [%d:%d]", DB_INTERNAL_ERROR, ptr_value_str, major, minor);
                        break;
                }
                *((UI8_T *)ptr_raw) = chardata;
            }
            else if (field_size == sizeof(UI16_T))
            {
                shortdata = atoi(ptr_value_str);
                switch(minor)
                {
                    case DB_CFG_MINOR_RANGE100:
                        if((DB_CFG_MINOR_RANGE_MIN0 > shortdata) || (DB_CFG_MINOR_RANGE100_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_PORTNUM:
                        if (PLAT_MAX_PORT_NUM < shortdata)
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_PVID:
                        if((DB_CFG_MINOR_RANGE_MIN1 > shortdata) || (DB_CFG_MINOR_RANGEVID_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_VID:
                        if((DB_CFG_MINOR_RANGE_MIN0 > shortdata) || (DB_CFG_MINOR_RANGEVID_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_FORWARD_DELAY:
                        if((DB_CFG_MINOR_RANGE_MIN4 > shortdata) || (DB_CFG_MINOR_RANGE30_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_MAX_AGE:
                        if((DB_CFG_MINOR_RANGE_MIN6 > shortdata) || (DB_CFG_MINOR_RANGE40_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_TX_HOLD_CNT:
                        if((DB_CFG_MINOR_RANGE_MIN1 > shortdata) || (DB_CFG_MINOR_RANGE10_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_PRIORITY:
                        if((DB_CFG_MINOR_RANGE_MIN0 > shortdata) || (DB_CFG_MINOR_RANGE61440_MAX < shortdata) || (0 != (shortdata % DB_CFG_MINOR_RANGE_STEP4096)))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE65535:
                        break;
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
                    case DB_CFG_MINOR_RSTP_PORT_TC_INTERVAL:
                        if((DB_CFG_MINOR_RANGE_MIN1 > shortdata) || (DB_CFG_MINOR_RANGE10_MAX < shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
#endif
                    case DB_CFG_MINOR_PORT_SETTINGS:
                        if (MW_E_OK != port_checkPortSettingsValid(shortdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_MSTP_INSTANCE:
                        break;
                    case DB_CFG_MINOR_RESERVE:
                        /* Do not check */
                        break;
                    default:
                        ret = MW_E_NOT_SUPPORT;
                        DB_LOG_ERROR("%s, SHORT number mismatch,%s: [%d:%d]", DB_INTERNAL_ERROR, ptr_value_str, major, minor);
                        break;
                }
                osapi_memcpy(ptr_raw, &shortdata, field_size);
            }
            else
            {
                intdata = strtoul(ptr_value_str, NULL, 10);
                switch(minor)
                {
                    case DB_CFG_MINOR_RANGE80K:
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || (DB_CFG_MINOR_RANGE80K_MAX < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE1M:
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || (DB_CFG_MINOR_RANGE1M_MAX < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGEMAX:
                        break;
                    case DB_CFG_MINOR_PORTMAP:
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || ((0x01 << (PLAT_MAX_PORT_NUM + 1))< intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RANGE11:
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || (DB_CFG_MINOR_RANGE11_MAX < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_FORCE_VER:
                        if((DB_CFG_MINOR_RANGE_MIN0 != intdata) && (DB_CFG_MINOR_RANGE_MIN2 != intdata)
                            && (DB_CFG_MINOR_RANGE_MIN3 != intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
                    case DB_CFG_MINOR_RSTP_PORT_COST:
                        if((DB_CFG_MINOR_RANGE_MIN1 > intdata) || (DB_CFG_MINOR_RANGE200M_MAX < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
                    case DB_CFG_MINOR_RANGE65535:
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || (DB_CFG_MINOR_RANGE65535_MAX < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                        }
                        break;
#endif
                    case DB_CFG_MINOR_RESERVE:
                        /* Do not check */
                        break;

                    case DB_CFG_MINOR_TRUNK_MEMBERS:
                        /* For compatibility with older config file versions */
                        if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || ((0x01 << (PLAT_MAX_PORT_NUM + 1)) < intdata))
                        {
                            ret = MW_E_NOT_SUPPORT;
                            break;
                        }
                        break;
                    default:
                        ret = MW_E_NOT_SUPPORT;
                        DB_LOG_ERROR("%s, INT number mismatch,%s: [%d:%d]", DB_INTERNAL_ERROR, ptr_value_str, major, minor);
                        break;
                }
                osapi_memcpy(ptr_raw, &intdata, field_size);
            }
            break;
        case JSONTrue:
        case JSONFalse:
            chardata = FALSE;
            if((0 == osapi_strncmp(ptr_value_str, "true", osapi_strlen("true"))) ||
               (0 == osapi_strncmp(ptr_value_str, "1", osapi_strlen("1"))))
            {
                chardata = TRUE;
            }
            osapi_memcpy(ptr_raw, &chardata, sizeof(UI8_T));
            break;
        case JSONArray:
            {
                const C8_T delim[] = ",";
                C8_T *ptr_save = NULL;
                /* Remove the '['']' */
                ptr_value++;
                osapi_strncpy(ptr_value_str, ptr_value, (value_len - 2));
                ptr_value_str[value_len-2] = '\0';
                /* Split the number */
                ptr_value = strtok_r(ptr_value_str, delim, &ptr_save);
                chardata = 0;
                intdata = 0;
#ifdef AIR_SUPPORT_MSTP
                key_len = 0;
#endif
                if (DB_CFG_MINOR_TRUNK_MEMBERS == minor)
                {
                    intdata = strtoul(ptr_value, NULL, 10);
                    if((DB_CFG_MINOR_RANGE_MIN0 > intdata) || ((0x01 << (PLAT_MAX_PORT_NUM + 1)) < intdata))
                    {
                        ret = MW_E_NOT_SUPPORT;
                        break;
                    }
                    osapi_memcpy(ptr_raw, &intdata, sizeof(UI32_T));
                    ptr_value = strtok_r(NULL, delim, &ptr_save);
                    chardata = atoi(ptr_value);
                    if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE3_MAX < chardata))
                    {
                        ret = MW_E_NOT_SUPPORT;
                        break;
                    }
                    *((UI8_T *)(ptr_raw + sizeof(UI32_T))) = chardata;
                    break;
                }
#ifdef AIR_SUPPORT_MSTP
                else if ((DB_CFG_MINOR_RSTP_PORT_PRIORITY == minor)
                        || (DB_CFG_MINOR_RSTP_PORT_COST == minor))
                {
                    while (ptr_value)
                    {
                        if (key_len >= field_size)
                        {
                            DB_LOG_ERROR("The data is oversized: [%u/%u/%u], field_size: %d key_len: %d",
                                req.t_idx, cur_fidx, req.e_idx, field_size, key_len);
                            break;
                        }
                        switch (minor)
                        {
                            case DB_CFG_MINOR_RSTP_PORT_PRIORITY:
                                chardata = atoi(ptr_value);
                                if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE240_MAX < chardata) || (0 != (chardata % DB_CFG_MINOR_RANGE_STEP16)))
                                {
                                    ret = MW_E_NOT_SUPPORT;
                                    break;
                                }
                                osapi_memcpy(ptr_raw + key_len, &chardata, sizeof(UI8_T));
                                key_len += sizeof(UI8_T);
                                break;
                            case DB_CFG_MINOR_RSTP_PORT_COST:
                                intdata = strtoul(ptr_value, NULL, 10);
                                if((DB_CFG_MINOR_RANGE_MIN1 > intdata) || (DB_CFG_MINOR_RANGE200M_MAX < intdata))
                                {
                                    ret = MW_E_NOT_SUPPORT;
                                    break;
                                }
                                osapi_memcpy(ptr_raw + key_len, &intdata, sizeof(UI32_T));
                                key_len += sizeof(UI32_T);
                                break;
                            default:
                                ret = MW_E_NOT_SUPPORT;
                                DB_LOG_ERROR("%s, ARRAY number mismatch, %s [%d:%d]", DB_INTERNAL_ERROR, ptr_value_str, major, minor);
                                break;
                        }
                        ptr_value = strtok_r(NULL, delim, &ptr_save);
                    }
                    break;
                }
#endif
                while (ptr_value)
                {
                    if (intdata >= field_size)
                    {
                        DB_LOG_ERROR("The data is oversized: [%u/%u/%u], field_size: %d", req.t_idx, cur_fidx, req.e_idx, field_size);
                        break;
                    }
                    chardata = atoi(ptr_value);
                    switch(minor)
                    {
                        case DB_CFG_MINOR_RANGE2:
                            if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE2_MAX < chardata))
                            {
                                ret = MW_E_NOT_SUPPORT;
                            }
                            break;
                        case DB_CFG_MINOR_RANGE7:
                            if((DB_CFG_MINOR_RANGE_MIN0 > chardata) || (DB_CFG_MINOR_RANGE7_MAX < chardata))
                            {
                                ret = MW_E_NOT_SUPPORT;
                            }
                            break;
                        default:
                            ret = MW_E_NOT_SUPPORT;
                            DB_LOG_ERROR("%s, ARRAY number mismatch,%s: [%d:%d]", DB_INTERNAL_ERROR, ptr_value_str, major, minor);
                            break;
                    }
                    *((UI8_T *)(ptr_raw + intdata)) = chardata;
                    intdata++;
                    ptr_value = strtok_r(NULL, delim, &ptr_save);
                }
                break;
            }
        default:
            break;
    }

    return ret;
}

/* FUNCTION NAME: db_cfgfile_saveField
 * PURPOSE:
 *      parse the configuration file to DB raw data
 *
 * INPUT:
 *      t_idx         -- The table index
 *      f_idx         -- The field index
 *      e_idx         -- The element index
 *      ptr_cfgtext   -- A pointer to the config text stream
 *      ptr_data_size -- The size of the data to be parsed
 *
 * OUTPUT:
 *      ptr_raw       -- A pointer to temp db raw data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER_
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_saveField(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI8_T e_idx,
    UI8_T       *ptr_raw,
    UI8_T       *ptr_cfgtext,
    UI16_T      *ptr_data_size)
{
    void                *ptr_value = NULL;
    C8_T                *ptr_str = NULL;
    C8_T                *ptr_key_value = NULL;
    UI8_T               field_size = 0;
    UI8_T               key_len = 0;
    UI8_T               chardata = 0;
    UI8_T               major = JSONInvalid;
    UI16_T              minor = DB_CFG_MINOR_LAST;
    UI16_T              index = 0, entry_num = 0, write_cnt = 0;
    UI16_T              value_len = 0;
    UI16_T              str_len = 0;
    UI16_T              shortdata = 0;
    UI32_T              data = 0;
    UI32_T              data_len = 0;
    JSONTypes_t         type = JSONString;
    MW_ERROR_NO_T       ret = MW_E_NO_MEMORY;
    const DB_CFG_T      *ptr_fieldnames = NULL;

    MW_CHECK_PTR(ptr_raw);
    MW_CHECK_PTR(ptr_cfgtext);
    MW_CHECK_PTR(ptr_data_size);

    dbapi_getFieldSize(t_idx, f_idx, &field_size);
    dbapi_getEntriesNum(t_idx, &entry_num);
    ptr_fieldnames = _db_cfg_getFieldnames(t_idx);

    ret = db_calloc(DB_BUFFER_SIZE_256, (void **)&ptr_str);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: Cannot allocate memory for str", DB_INTERNAL_ERROR);
        return ret;
    }

    ret = db_calloc(DB_BUFFER_SIZE_256, (void **)&ptr_key_value);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: Cannot allocate memory for key_value", DB_INTERNAL_ERROR);
        MW_FREE(ptr_str);
        return ret;
    }

    if(0 != osapi_strlen(ptr_fieldnames[f_idx - 1].ptr_name))
    {
        /* Get type */
        major = CFGMAJOR(ptr_fieldnames[f_idx - 1].type);
        minor = CFGMINOR(ptr_fieldnames[f_idx - 1].type);
        if((DB_ALL_ENTRIES == e_idx) || (1 == e_idx))
        {
            /* Print the field key name */
            key_len += DB_PRINT_KEY((ptr_key_value + key_len), ptr_fieldnames[f_idx - 1].ptr_name);
            if(1 < entry_num)
            {
                /* Print the fist '[' */
                key_len += osapi_snprintf(ptr_key_value + key_len, DB_BUFFER_SIZE_256 - key_len, "[");
            }
            value_len = DB_BUFFER_SIZE_256 - key_len;
        }

        write_cnt = ((DB_ALL_ENTRIES == e_idx) ? entry_num : 1);
        for(index = 0; index < write_cnt; index++)
        {
            ptr_value = NULL;
            osapi_memset(ptr_str, 0, DB_BUFFER_SIZE_256);
            value_len = DB_BUFFER_SIZE_256 - key_len;
            switch (major)
            {
                case JSONString:
                    type = JSONString;
                    ptr_value = ptr_str;
                    if (DB_CFG_MINOR_ADDR == minor)
                    {
                        ip4_addr_t ip = { 0 };

                        osapi_memcpy(&data, (ptr_raw + index * field_size), field_size);
                        ip4_addr_set_u32(&ip, data);
                        ptr_value = (void *)inet_ntoa_r(ip, ptr_str, DB_BUFFER_SIZE_256);
                    }
#ifdef AIR_SUPPORT_IPV6
                    else if (DB_CFG_MINOR_IPV6_ADDR == minor)
                    {
                        ip6_addr_t ipv6_ip = {{0}};

                        osapi_memcpy(&ipv6_ip, (ptr_raw + index * field_size), field_size);
                        ptr_value = (void *)inet6_ntoa_r(ipv6_ip, ptr_str, DB_BUFFER_SIZE_256);
                    }
#endif
                    else if(DB_CFG_MINOR_MAC == minor)
                    {
                        ether_addr_t mac;

                        osapi_memcpy(mac.octet, (ptr_raw + index * field_size), field_size);
                        macToStr(&mac, (UI8_T *)ptr_str, TRUE);
                    }
                    else if(DB_CFG_MINOR_OUI == minor)
                    {
                        ether_addr_t mac;

                        osapi_memcpy(mac.octet, (ptr_raw + index * field_size), field_size);
                        osapi_sprintf(ptr_str, "%02x:%02x:%02x", mac.octet[0], mac.octet[1], mac.octet[2]);
                    }
                    else if(DB_CFG_MINOR_PASSWD == minor)
                    {
                        C8_T    *pptr_str = ptr_str;
                        UI8_T   passwdstr[MAX_PASSWORD_SIZE] = {0};
                        UI8_T   *ptr_pass = passwdstr;
                        UI8_T   f_size = field_size;

                        /* Indicate the data is Hashed HEX string */
                        pptr_str = ptr_str + osapi_sprintf(ptr_str, "%s", DB_HASH_PREFIX);
                        /* Translate hash to HEX string */
                        osapi_memcpy(passwdstr, (ptr_raw + index * field_size), field_size);
                        while (f_size > 0)
                        {
                            osapi_sprintf(pptr_str, "%02X", *ptr_pass);
                            pptr_str += 2;
                            f_size--;
                            ptr_pass++;
                        }
                    }
                    else
                    {
                        osapi_memcpy(ptr_str, (ptr_raw + index * field_size), field_size);
                    }
                    break;

                case JSONTrue:
                case JSONFalse:
                    chardata = 0;
                    osapi_memcpy(&chardata, (ptr_raw + index * field_size), sizeof(chardata));
                    type = ((FALSE == chardata) ? JSONFalse : JSONTrue);
                    ptr_value = NULL;
                    break;

                case JSONNumber:
                    data = 0;
                    chardata = 0;
                    shortdata = 0;
                    if(DB_CFG_MINOR_RESERVE != minor)
                    {
                        if(sizeof(chardata) == field_size)
                        {
                            osapi_memcpy(&chardata, (ptr_raw + index * field_size), field_size);
                            data = chardata;
                        }
                        else if(sizeof(shortdata) == field_size)
                        {
                            osapi_memcpy(&shortdata, (ptr_raw + index * field_size), field_size);
                            data = shortdata;
                        }
                        else
                        {
                            osapi_memcpy(&data, (ptr_raw + index * field_size), field_size);
                        }
                    }
                    type = JSONNumber;
                    ptr_value = (void *)&data;
                    break;

                case JSONArray:
                    data_len = 0;
                    chardata = 0;
                    if(DB_CFG_MINOR_TRUNK_MEMBERS == minor)
                    {
                        TRUNK_MEMBER_INFO_T member = {0};

                        osapi_memcpy(&member, (ptr_raw + index * field_size), field_size);
                        data_len += osapi_snprintf((ptr_str + data_len), (DB_BUFFER_SIZE_256 - data_len), "%u,", member.member_bmp);
                        data_len += osapi_snprintf((ptr_str + data_len), (DB_BUFFER_SIZE_256 - data_len), "%u,", member.mode);
                    }
#ifdef AIR_SUPPORT_MSTP
                    else if(DB_CFG_MINOR_RSTP_PORT_PRIORITY == minor)
                    {
                        UI8_T     pri[MAX_MSTP_INSTANCE_NUM] = {0};
                        UI32_T    i;

                        osapi_memcpy(pri, (ptr_raw + index * field_size), field_size);
                        for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
                        {
                            data_len += osapi_snprintf((ptr_str + data_len), (DB_BUFFER_SIZE_256 - data_len), "%u,", pri[i]);
                        }
                    }
                    else if(DB_CFG_MINOR_RSTP_PORT_COST == minor)
                    {
                        UI32_T    cost[MAX_MSTP_INSTANCE_NUM] = {0};
                        UI32_T    i;

                        osapi_memcpy(cost, (ptr_raw + index * field_size), field_size);
                        for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
                        {
                            data_len += osapi_snprintf((ptr_str + data_len), (DB_BUFFER_SIZE_256 - data_len), "%u,", cost[i]);
                        }
                    }
#endif
                    else
                    {
                        UI8_T               *ptr_datastr = NULL;

                        ret = db_calloc(DB_BUFFER_SIZE_128, (void **)&ptr_datastr);
                        if (MW_E_OK != ret)
                        {
                            MW_FREE(ptr_str);
                            MW_FREE(ptr_key_value);
                            return ret;
                        }
                        osapi_memcpy(ptr_datastr, (ptr_raw + index * field_size), field_size);
                        while (chardata < field_size)
                        {
                            data_len += osapi_snprintf((ptr_str + data_len), (DB_BUFFER_SIZE_256 - data_len), "%u,", ptr_datastr[chardata]);
                            chardata++;
                        }
                        MW_FREE(ptr_datastr);
                    }
                    if (data_len > 0)
                    {
                        ptr_str[data_len - 1] = '\0';
                    }
                    type = JSONArray;
                    ptr_value = ptr_str;
                    break;

                default:
                    osapi_memcpy(ptr_str, (ptr_raw + index * field_size), field_size);
                    type = major;
                    ptr_value = ptr_str;
                    break;
            }

            ret = db_cfgfile_bufAppend((ptr_key_value + key_len), &value_len, type, (void *)ptr_value);
            if (MW_E_OK != ret)
            {
                break;
            }

            /* Print the last */
            if ((1 < entry_num) &&
                ((entry_num == e_idx) || ((DB_ALL_ENTRIES == e_idx) && (entry_num == (index + 1)))))
            {
                DB_APPEND_CHAR(ptr_key_value, value_len, "]");
            }
            DB_APPEND_CHAR(ptr_key_value, value_len, ",");

            if ((*ptr_data_size) < (str_len + key_len + value_len))
            {
                DB_LOG_WARN("[%d/%d/%d]: Temp buffer(cfgtext) is fulled, need to release the buffer", t_idx, f_idx, e_idx);
                ret = MW_E_NO_MEMORY;
                break;
            }
            else
            {
                osapi_strcat((C8_T *)ptr_cfgtext, ptr_key_value);
                str_len += (key_len + value_len);
                osapi_memset(ptr_key_value, 0, DB_BUFFER_SIZE_256);
                key_len = 0;
            }
        }
    }

    MW_FREE(ptr_str);
    MW_FREE(ptr_key_value);
    *ptr_data_size = str_len;

    return ret;
}

/* FUNCTION NAME: db_cfgfile_parsing
 * PURPOSE:
 *      parse the configuration file to DB raw data
 *
 * INPUT:
 *      req           -- Current handled request type
 *      ptr_fdata     -- A pointer to the config tmp buffer
 *      field_size    -- The field size
 *      ptr_cfgtext   -- A pointer to the config text stream
 *      data_size     -- The config text size
 *      flag          -- The type of parsing method
 *
 * OUTPUT:
 *      ptr_raw       -- A pointer to temp db raw data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER_
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_parsing(
    DB_REQUEST_TYPE_T req,
    DB_PAYLOAD_T *ptr_fdata,
    UI8_T field_size,
    void *ptr_raw,
    void *ptr_cfgtext,
    UI16_T data_size,
    UI8_T flag)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    const DB_CFG_T *p_fieldnames;
    JSONStatus_t result = 0;
    JSONTypes_t value_type = 0;
    C8_T *ptr_key_str = NULL;
    C8_T *ptr_value_str = NULL;
    C8_T *value = NULL;
    UI16_T key_len = 0;
    UI32_T value_len = 0;
    UI8_T cur_fidx = req.f_idx - 1;
    UI8_T major = JSONInvalid;
    UI16_T minor = DB_CFG_MINOR_LAST;

    MW_CHECK_PTR(ptr_raw);
    MW_CHECK_PTR(ptr_cfgtext);

    ret = db_calloc(DB_BUFFER_SIZE_128, (void **)&ptr_key_str);
    if (MW_E_OK != ret)
    {
        return ret;
    }
    ret = db_calloc(DB_BUFFER_SIZE_512, (void **)&ptr_value_str);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_key_str);
        return ret;
    }

    ptr_fdata->data_size = field_size;

    p_fieldnames = _db_cfg_getFieldnames(req.t_idx);
    if (0 == osapi_strlen(p_fieldnames[cur_fidx].ptr_name))
    {
        DB_LOG_ERROR("The table is not a config table: %u", req.t_idx);
        MW_FREE(ptr_key_str);
        MW_FREE(ptr_value_str);
        return MW_E_BAD_PARAMETER;
    }
    if (0 != osapi_strlen(p_fieldnames[cur_fidx].ptr_name))
    {
        /* Set the query string */
        key_len = osapi_snprintf(ptr_key_str, DB_BUFFER_SIZE_128, "%s", p_fieldnames[cur_fidx].ptr_name);

        /* Search the field value */
        result = JSON_SearchT(ptr_cfgtext, (size_t)data_size, (const char *)ptr_key_str, (size_t)key_len,
                (char **)&value, (size_t *)&value_len, &value_type);
        if (JSONSuccess != result)
        {
            /* Data in config file is not found */
            DB_LOG_WARN("Data in config file is not found");
            MW_FREE(ptr_key_str);
            MW_FREE(ptr_value_str);
            return ret;
        }
        if (value_len >= DB_BUFFER_SIZE_512)
        {
            value_len = DB_BUFFER_SIZE_512 - 1;
        }
        osapi_strncpy(ptr_value_str, value, value_len);

        /* Get type */
        major = CFGMAJOR(p_fieldnames[cur_fidx].type);
        minor = CFGMINOR(p_fieldnames[cur_fidx].type);

        DB_LOG_DEBUG("Found:[%u/%u/%u] %s -> %s, type:%d-%d", req.t_idx, cur_fidx, req.e_idx, ptr_key_str, ptr_value_str, major, minor);

        /* Check major type */
        ret = _db_cfgfile_json_to_raw(req, field_size, ptr_raw, value, ptr_value_str, value_len, value_type);
        if((MW_E_OK == ret) && (0 == (DB_PARSE_SPECIAL_DATA & flag)))
        {
            if(MW_E_OK != db_setData(1, M_UPDATE, ptr_fdata, NULL))
            {
                ret = MW_E_NOT_SUPPORT;
            }
        }
    }
    MW_FREE(ptr_key_str);
    MW_FREE(ptr_value_str);
    return ret;
}

/* FUNCTION NAME: db_cfgfile_parsing_ext
 * PURPOSE:
 *      parse the configuration file to DB raw data
 *
 * INPUT:
 *      req           -- Current handled request type
 *      ptr_data      -- A pointer to the config tmp buffer
 *      field_size    -- The field size
 *      ptr_cfgtext   -- A pointer to the config text stream
 *      data_size     -- The config text size
 *      flag          -- The type of parsing method
 *
 * OUTPUT:
 *      ptr_raw       -- A pointer to temp db raw data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER_
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_parsing_ext(
    DB_REQUEST_TYPE_T req,
    DB_PAYLOAD_T *ptr_data,
    UI8_T field_size,
    void *ptr_raw,
    void *ptr_cfgtext,
    UI16_T data_size,
    UI8_T flag)
{
    C8_T                        *ptr_value_str = NULL;
    UI8_T                       cur_fidx = req.f_idx - 1;
    UI8_T                       major = JSONInvalid;
    UI16_T                      minor = DB_CFG_MINOR_LAST;
    MW_ERROR_NO_T               ret = MW_E_ENTRY_NOT_FOUND;
    const DB_CFG_T              *ptr_fieldnames;


    MW_CHECK_PTR(ptr_raw);
    MW_CHECK_PTR(ptr_cfgtext);

    ret = db_calloc(DB_BUFFER_SIZE_512, (void **)&ptr_value_str);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    ptr_data->data_size = field_size;
    ptr_fieldnames = _db_cfg_getFieldnames(req.t_idx);
    if (0 == osapi_strlen(ptr_fieldnames[cur_fidx].ptr_name))
    {
        DB_LOG_ERROR("The table is not a config table: %u", req.t_idx);
        MW_FREE(ptr_value_str);
        return MW_E_BAD_PARAMETER;
    }

    if (data_size >= DB_BUFFER_SIZE_512)
    {
        data_size = DB_BUFFER_SIZE_512 - 1;
    }
    osapi_strncpy(ptr_value_str, ptr_cfgtext, data_size);
    DB_LOG_DEBUG("value:%s", ptr_value_str);

    /* Get type */
    major = CFGMAJOR(ptr_fieldnames[cur_fidx].type);
    minor = CFGMINOR(ptr_fieldnames[cur_fidx].type);

    ret = _db_cfgfile_json_to_raw(req, field_size, ptr_raw, ptr_cfgtext, ptr_value_str, data_size, major);
    DB_LOG_DEBUG(", field_size:%d, maj:%d, min:%d, ret:%d\n", field_size, major, minor, ret);
    if((MW_E_OK == ret) && (0 == (DB_PARSE_SPECIAL_DATA & flag)))
    {
        if(MW_E_OK != db_setData(1, M_UPDATE, ptr_data, NULL))
        {
            ret = MW_E_NOT_SUPPORT;
        }
    }

    MW_FREE(ptr_value_str);
    return ret;
}

/* FUNCTION NAME: db_cfgfile_printTable
 * PURPOSE:
 *      print table name in JSON format
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_cfgtext     -- The output json string
 *      data_size       -- The output json string length
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Append the key string to the end of the buffer
 *
 */
MW_ERROR_NO_T
db_cfgfile_printTable(
    TABLES_T t_idx,
    UI8_T *ptr_cfgtext,
    UI16_T *data_size)
{
    MW_ERROR_NO_T ret = MW_E_NO_MEMORY;
    C8_T *ptr_key_str = NULL;
    UI32_T str_len = 0;

    MW_CHECK_PTR(ptr_cfgtext);
    MW_CHECK_PTR(data_size);

    ret = db_calloc(DB_BUFFER_SIZE_128, (void **)&ptr_key_str);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Print the table name */
    str_len = DB_PRINT_KEY(ptr_key_str, _db_cfg_getTableName(t_idx));
    if (str_len < *data_size)
    {
        osapi_strcat((C8_T *)ptr_cfgtext, ptr_key_str);
        *data_size = str_len;
        ret = MW_E_OK;
    }
    else
    {
        *data_size = 0;
    }
    MW_FREE(ptr_key_str);

    return ret;
}

/* FUNCTION NAME: db_cfgfile_searchTable
 * PURPOSE:
 *      find the table index in JSON file
 *
 * INPUT:
 *      pptr_cfgtext    -- A dpointer to the config text
 *      t_idx           -- Current table index
 *
 * OUTPUT:
 *      pptr_cfgtext    -- A dpointer to the config text
 *      t_idx           -- Founded table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Will move the pointer to the start of the table string if founded
 *
 */
MW_ERROR_NO_T
db_cfgfile_searchTable(
    UI8_T **pptr_cfgtext,
    UI8_T *t_idx)
{
    C8_T *key_str = NULL;
    UI8_T idx = 0;

    MW_CHECK_PTR(pptr_cfgtext);
    MW_CHECK_PTR(t_idx);

    idx = *t_idx;
    while (idx < DB_FIRST_OPER_TABLE)
    {
        key_str = strstr((C8_T *)*pptr_cfgtext, _db_cfg_getTableName(idx));
        if (NULL != key_str)
        {
            break;
        }
        idx++;
    }

    if (NULL == key_str)
    {
        return MW_E_ENTRY_NOT_FOUND;
    }

    *t_idx = idx;
    *pptr_cfgtext = (UI8_T *)(key_str);

    return MW_E_OK;
}


/* FUNCTION NAME: db_cfgfile_bufAppend
 * PURPOSE:
 *      Append the value to config text sting safely.
 *
 * INPUT:
 *      type            -- The type of the data
 *      value           -- a pointer to the data value
 *
 * OUTPUT:
 *      ptr_cfgtext     -- The output json string
 *      data_size       -- The output json string length
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Append data to the end of the buffer
 *
 */
MW_ERROR_NO_T
db_cfgfile_bufAppend(
    C8_T *ptr_cfgtext,
    UI16_T *data_size,
    JSONTypes_t type,
    void *value)
{
    MW_ERROR_NO_T ret = MW_E_NO_MEMORY;
    C8_T *ptr_str = NULL;
    UI32_T str_len = 0;

    MW_CHECK_PTR(ptr_cfgtext);
    MW_CHECK_PTR(data_size);

    ret = db_calloc(DB_BUFFER_SIZE_256, (void **)&ptr_str);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    switch (type)
    {
        case JSONString:
            MW_CHECK_PTR(value);
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "\"%s\"", (C8_T *)value);
            break;
        case JSONNumber:
            MW_CHECK_PTR(value);
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "%u", *((UI32_T *)value));
            break;
        case JSONTrue:
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "%s", "1");
            break;
        case JSONFalse:
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "%s", "0");
            break;
        case JSONNull:
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "%s", "null");
            break;
        case JSONObject:
            MW_CHECK_PTR(value);
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "{%s}", (C8_T *)value);
            break;
        case JSONArray:
            MW_CHECK_PTR(value);
            str_len = osapi_snprintf(ptr_str, DB_BUFFER_SIZE_256, "[%s]", (C8_T *)value);
            break;
        default:
            MW_FREE(ptr_str);
            return MW_E_BAD_PARAMETER;
    }

    if (str_len < *data_size)
    {
        osapi_strcat((C8_T *)ptr_cfgtext, ptr_str);
        *data_size = str_len;
        ret = MW_E_OK;
    }
    else
    {
        DB_LOG_WARN("no enough buffer size, str_len:%d, data_size:%d", str_len, *data_size);
        *data_size = 0;
    }
    MW_FREE(ptr_str);
    return ret;
}

/* FUNCTION NAME: db_cfgfile_getTableName
 * PURPOSE:
 *      Return the table name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_tbltext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getTableName(
    UI8_T t_idx,
    UI8_T data_size,
    C8_T *ptr_tbltext)
{
    MW_ERROR_NO_T ret;
    C8_T key_str[DB_MAX_KEY_SIZE] = {0};
    UI32_T str_len = 0;

    MW_CHECK_PTR(ptr_tbltext);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    /* Print the table name */
    str_len = osapi_snprintf(key_str, DB_MAX_KEY_SIZE, "%s", _db_cfg_getTableName(t_idx));
    if (str_len < data_size)
    {
        osapi_memcpy((void *)ptr_tbltext, (const void *)key_str, str_len);
        ptr_tbltext[str_len] = '\0';
        ret = MW_E_OK;
    }
    else
    {
        osapi_memcpy((void *)ptr_tbltext, (const void *)key_str, data_size);
        ptr_tbltext[data_size - 1] = '\0';
        /* buffer size is not enough */
        ret = MW_E_NO_MEMORY;
    }
    return ret;
}

/* FUNCTION NAME: db_cfgfile_getFieldName
 * PURPOSE:
 *      Return the Field name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_fldtext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getFieldName(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T data_size,
    C8_T *ptr_fldtext)
{
    MW_ERROR_NO_T ret;
    C8_T key_str[DB_MAX_KEY_SIZE] = {0};
    UI32_T str_len = 0;
    const DB_CFG_T *p_fieldnames;

    MW_CHECK_PTR(ptr_fldtext);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((f_idx > db_getTableFieldsNum(t_idx)), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((data_size == 0), MW_E_BAD_PARAMETER);

    /* Get the Field name */
    if (f_idx == DB_ALL_FIELDS)
    {
        str_len = osapi_snprintf(key_str, DB_MAX_KEY_SIZE, "%s", DB_ALL_KEY);
    }
    else
    {
        p_fieldnames = _db_cfg_getFieldnames(t_idx);
        if ((p_fieldnames == NULL) || (0 == osapi_strlen(p_fieldnames[f_idx - 1].ptr_name)))
        {
            return MW_E_BAD_PARAMETER;
        }
        str_len = osapi_snprintf(key_str, DB_MAX_KEY_SIZE, "%s", p_fieldnames[f_idx - 1].ptr_name);
    }
    /* Copy to output buffer */
    if (str_len < data_size)
    {
        osapi_memcpy((void *)ptr_fldtext, (const void *)key_str, str_len);
        ptr_fldtext[str_len] = '\0';
        ret = MW_E_OK;
    }
    else
    {
        osapi_memcpy((void *)ptr_fldtext, (const void *)key_str, data_size);
        ptr_fldtext[data_size - 1] = '\0';
        /* buffer size is not enough */
        ret = MW_E_NO_MEMORY;
    }
    return ret;
}

/* FUNCTION NAME: db_cfgfile_getTableIdx
 * PURPOSE:
 *      Return the table index of the specific table name
 *
 * INPUT:
 *      table_name      -- The table name string
 *
 * OUTPUT:
 *      ptr_tblidx      -- The table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getTableIdx(
    C8_T *table_name,
    UI8_T *ptr_tblidx)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI8_T t_idx = 0;

    MW_CHECK_PTR(table_name);
    MW_CHECK_PTR(ptr_tblidx);

    while (t_idx < TABLES_LAST)
    {
        if (0 == osapi_strcmp(table_name, _db_cfg_getTableName(t_idx)))
        {
            *ptr_tblidx = t_idx;
            ret = MW_E_OK;
            break;
        }
        t_idx++;
    }
    return ret;
}

/* FUNCTION NAME: db_cfgfile_getFieldIdx
 * PURPOSE:
 *      Return the Field index of the specific field name
 *
 * INPUT:
 *      t_idx           -- The table index
 *      field_name      -- The field name
 *
 * OUTPUT:
 *      ptr_fldidx      -- The field index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getFieldIdx(
    UI8_T t_idx,
    C8_T *field_name,
    UI8_T *ptr_fldidx)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    const DB_CFG_T *p_fieldnames;
    UI8_T f_idx = 0;

    MW_CHECK_PTR(ptr_fldidx);
    MW_CHECK_PTR(field_name);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    if (0 == osapi_strcmp(field_name, DB_ALL_KEY))
    {
        *ptr_fldidx = f_idx;
        ret = MW_E_OK;
    }
    else
    {
        p_fieldnames = _db_cfg_getFieldnames(t_idx);
        if (p_fieldnames == NULL)
        {
            return MW_E_BAD_PARAMETER;
        }
        while (0 != osapi_strlen(p_fieldnames[f_idx].ptr_name))
        {
            if (0 == osapi_strcmp(field_name, p_fieldnames[f_idx].ptr_name))
            {
                *ptr_fldidx = f_idx + 1;
                ret = MW_E_OK;
                break;
            }
            f_idx++;
        }
    }

    return ret;
}

