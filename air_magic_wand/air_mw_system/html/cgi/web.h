/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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
/* FILE NAME:  web.h
 * PURPOSE:
 *      It provides CGI/SSI functions of local webpages.
 *
 * NOTES:
 */
#ifndef __WEB_H__
#define __WEB_H__

/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/opt.h"
#include "lwip/apps/httpd_opts.h"
#include "lwip/tcp.h"
#include "lwip/apps/fs.h"
#include "mw_error.h"
#include <timer.h>
#include "lwip/apps/httpd.h"
#include "httpd_util.h"
#ifdef AIR_SUPPORT_MQTTD
#include "FreeRTOS.h"
#include "semphr.h"
#endif
#include "db_api.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define FAIL                (1)
#define SUCCESS             (0)

#define FINISH              (0)
#define STRING              (1)
#define UINT32              (2)
#define UCHAR               (3)
#define STOI                (4)
#define INT32               (5)
#define CHAR                (6)

#define LANGUAGE_LEN        (10)

typedef enum MENU_OPTION_S
{
    MENU_OPTION_NULL = 0,
    MENU_OPTION_DHCPSNOOP,
    MENU_OPTION_IGSNP,
    MENU_OPTION_LOOP,
    MENU_OPTION_ICMPC,
    MENU_OPTION_VOICE,
    MENU_OPTION_SURVEILLANCE,
    MENU_OPTION_SNMP,
    MENU_OPTION_RSTP,
    MENU_OPTION_MSTP,
    MENU_OPTION_STP_SECURITY,
    MENU_OPTION_MQTTD,
    MENU_OPTION_LLDP,
    MENU_OPTION_LLDP_NEIGHBOR,
    MENU_OPTION_DDM,
    MENU_OPTION_POE,
    MENU_OPTION_POE_CONFIG,
    MENU_OPTION_POE_WATCHDOG,
    MENU_OPTION_DIAG,
    MENU_OPTION_SNTP,
    MENU_OPTION_ERPS,
    MENU_OPTION_LAST
}MENU_OPTION_T;

typedef enum FWE_STATE_E
{
    FW_E_SUCCESS = 0,
    FW_E_UPLOAD,
    FW_E_ERASE_WRITE,
    FW_E_ERROR_CRC,
    FW_E_ERROR_UPLOAD,
    FW_E_ERROR_WRITE,
    FW_E_WAIT_FINISH,
    FW_E_STATE_LAST
}FW_STATE_T;

typedef enum LOGON_ERR_TYPE_E
{
    LOGON_CFM_OK = 0,     /* Username and password confirm ok */
    LOGON_CFM_FAIL,       /* Username or password confirm failed */
    LOGON_CFM_FULL,       /* Reached the maximum number of connections */
    LOGON_LAST
}LOGON_ERR_TYPE_T;

typedef enum PORTSETTING_ERR_TYPE_E
{
    PORTSETTING_NO_ERR = 0,             /* No error */
    PORTSETTING_PORT_STATE_CHANGED,     /* Port state state changed */
    PORTSETTING_LAST
}PORTSETTING_ERR_TYPE_T;
typedef enum ACCOUNT_ERR_TYPE_E
{
    ACCOUNT_CFM_FAIL = -1, /* Old password confirm failed */
    ACCOUNT_DEFAULT,       /* Do nothing */
    ACCOUNT_UPDATED,       /* Username and password updated */
    ACCOUNT_LAST
}ACCOUNT_ERR_TYPE_T;

typedef enum LANG_S
{
    LANG_CHINESE = 0,
    LANG_ENGLISH,
    LANG_LAST
}LANG_T;

/* MACRO FUNCTION DECLARATIONS
*/
#define GET_BIT(x,y) ((x)>>(y) & 1)
#define SET_BIT(x,y) ((x) |= (1 << (y)))
#define CLR_BIT(x,y) ((x) &= ~(1 << (y)))

#if HTTPD_DEBUG
#define CGI_LOG_ERROR(module, fmt, ...) MW_LOG_ERROR(module, "[CGI]"fmt, ##__VA_ARGS__)
#define CGI_LOG_WARN(module, fmt, ...) MW_LOG_WARN(module, "[CGI]"fmt, ##__VA_ARGS__)
#define CGI_LOG_INFO(module, fmt, ...) MW_LOG_INFO(module, "[CGI]"fmt, ##__VA_ARGS__)
#define CGI_LOG_DEBUG(module, fmt, ...) MW_LOG_DEBUG(module, "[CGI]"fmt, ##__VA_ARGS__)
#else /* HTTPD_DEBUG */
#define CGI_LOG_ERROR(module, fmt, ...)
#define CGI_LOG_WARN(module, fmt, ...)
#define CGI_LOG_INFO(module, fmt, ...)
#define CGI_LOG_DEBUG(module, fmt, ...)
#endif /* HTTPD_DEBUG */

#if HTTPD_DEBUG
# define CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkPbm,ptr_portBitMap,indexStart) \
    cgi_utility_revisePortBitMapByTrunkInfo(trunkPbm,ptr_portBitMap,indexStart, __func__);
#else
# define CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkPbm,ptr_portBitMap,indexStart) \
    cgi_utility_revisePortBitMapByTrunkInfo(trunkPbm,ptr_portBitMap,indexStart, NULL);
#endif

/* DATA TYPE DECLARATIONS
*/
typedef struct igmp_snooping_info_s
{
    unsigned int igmp_mode;
    unsigned int igmp_fastleave_mode;
    unsigned int reportSu_mode;
}igmp_snooping_info_t;

typedef struct sys_info_s
{
    char sysname[32];           //rw
    unsigned char sysmac[8];    //rw
    char firmware_version[64];  //r
    char hardware_version[64];  //r
    unsigned int dhcp_enable;   //rw
    unsigned int autodns_enable;//rw
    unsigned int mgmt_ip_addr;  //rw
    unsigned int mgmt_ip_mask;  //rw
    unsigned int mgmt_gw;       //rw
    unsigned int mgmt_dns;      //rw
    unsigned int cur_ip_addr;   //r
    unsigned int cur_ip_mask;   //r
    unsigned int cur_gw;        //r
    unsigned int cur_dns;       //r
    char sysmac_tmp[16];
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    unsigned int mgmt_vlan;     //rw
#endif
}sys_info_t;

typedef struct account_info_s
{
    C8_T username[MAX_USER_NAME_SIZE];
    C8_T old_pwd[MAX_PASSWORD_SIZE];
    C8_T new_pwd[MAX_PASSWORD_SIZE];
    C8_T passwd[MAX_PASSWORD_SIZE];
    UI8_T hashPass[MAX_PASSWORD_HASH_SIZE];
}account_info_t;

typedef struct system_info_s
{
    UI8_T sys_name[MAX_SYS_NAME_SIZE];
    MW_MAC_T sys_mac;
    C8_T sw_version[MAX_VERSION_SIZE];
    UI8_T hw_version[MAX_VERSION_SIZE];
    MW_IPV4_T current_ip;
    MW_IPV4_T current_mask;
    MW_IPV4_T current_gw;
    MW_IPV4_T current_dns;

}system_info_rpm_t;

typedef struct language_info_s
{
    LANG_T  lang_idx;
}language_info_t;

typedef struct
{
    /* [I/O] Buffer to store the tag insert string. And the tag insert string returned. */
    char *ptr_tag_insert;
    /* [In] Buffer size */
    int tag_insert_max_len;
#if LWIP_HTTPD_SSI_MULTIPART
    /* [In] Indicate which part of the tag insert string is to be returned in the current function call. */
    u16_t current_tag_part;
    /* [Out] Indicate the next part of the tag insert string to be returned in the next function call. If
     *       there is no part left, please do set it to HTTPD_LAST_TAG_PART.*/
    u16_t next_tag_part;
#endif
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
    /* [In] File state */
    void *ptr_connection_state;
#endif
    /* [Out] The length of the tag insert string returned. */
    int tag_insert_len;
    /* [Out] Chunk encode or not. */
    u8_t chunk_enable;
} HTTPD_TAG_PARAM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
extern FW_STATE_T _imgUploadStatus;
extern const tCGI CGIURLs[];
extern const char* const ppcTAGs[];
extern language_info_t language_info;

/* CGI handle function START */
MW_ERROR_NO_T cgi_set_handle_qos_scheduler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_qos_priority(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_stormCtrl(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_dummy_func(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_logon(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_bandWidthControl(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_reset_factory(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_conf_restore(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_system_reboot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_language(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_loopPrevention(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#ifdef AIR_SUPPORT_CABLE_DIAG
MW_ERROR_NO_T cgi_set_handle_cableDiag(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif
MW_ERROR_NO_T cgi_set_handle_portSetting(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_portTrunk(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_portTrunkAlgo(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_igmpSnoopingRpm(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_greenEtherSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_eeeConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#ifdef AIR_SUPPORT_SNMP
MW_ERROR_NO_T cgi_set_handle_snmpConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif
MW_ERROR_NO_T
cgi_set_handle_maxMacNumSet(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[]);

MW_ERROR_NO_T
cgi_set_handle_addStaticMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[]);

MW_ERROR_NO_T
cgi_set_handle_delStaticMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[]);

MW_ERROR_NO_T
cgi_set_handle_freshDynamicMac(
    I32_T iIndex,
    I32_T iNumParams,
    C8_T *ptr_pcParam[],
    C8_T *ptr_pcValue[]);

MW_ERROR_NO_T cgi_set_handle_systemInfoRpm(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_account(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_mirrorPortSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_jumboFrame(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_dhcpSnooping(int iIndex, int iNumParams, char *ptr_param[], char *ptr_value[]);
MW_ERROR_NO_T cgi_set_handle_pvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_pVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_qvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_qVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_qvlanPvidSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_mVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_mvlanModeSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#ifdef AIR_SUPPORT_VOICE_VLAN
MW_ERROR_NO_T cgi_set_handle_voiceVlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_voiceOuiSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_voiceOuiDel(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_voicePortSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
MW_ERROR_NO_T cgi_set_handle_surveilvlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_surveilOuiSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_surveilOuiDel(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif
MW_ERROR_NO_T cgi_set_handle_ipsetting(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_fwupdate(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_logout(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#ifdef AIR_SUPPORT_ICMP_CLIENT
MW_ERROR_NO_T cgi_set_handle_pinginfo(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif /* AIR_SUPPORT_ICMP_CLIENT */
MW_ERROR_NO_T cgi_set_handle_clearMibCounter(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
MW_ERROR_NO_T cgi_set_handle_spanningTree(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
#ifdef AIR_SUPPORT_RSTP_SECURITY
MW_ERROR_NO_T cgi_set_handle_spanningTreeSecurity(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
#endif
#ifdef AIR_SUPPORT_MSTP
MW_ERROR_NO_T cgi_set_handle_regionInstance(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
MW_ERROR_NO_T cgi_set_handle_spanningTreeInstance(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
MW_ERROR_NO_T cgi_set_handle_spanningTreePortInstance(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
#endif /*AIR_SUPPORT_MSTP*/
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
MW_ERROR_NO_T cgi_set_handle_mqttd(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_save_running(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_LLDPD
MW_ERROR_NO_T cgi_set_handle_lldpconfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_lldpportconfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif /* AIR_SUPPORT_LLDPD */
#ifdef AIR_SUPPORT_POE
MW_ERROR_NO_T cgi_set_handle_poeConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_poePortConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#ifdef AIR_SUPPORT_POE_WATCHDOG
MW_ERROR_NO_T cgi_set_handle_poeWatchDogConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
MW_ERROR_NO_T cgi_set_handle_poeWatchDog(I32_T iIndex, I32_T iNumParams, C8_T *ptr_pcParam[], C8_T *ptr_pcValue[]);
#endif
#endif
#ifdef AIR_SUPPORT_SNTP
MW_ERROR_NO_T cgi_set_systime_info_handle(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif
#ifdef AIR_SUPPORT_ERPS
MW_ERROR_NO_T cgi_set_handle_erpsConfig(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
#endif /* AIR_SUPPORT_ERPS */
/* CGI handle function END */

/*=============================================sys start=================================== ==========*/
const char* cgi_set_logon_info_handle(char* pcValue[]);
#if LWIP_HTTPD
err_t cgi_set_handle_conf_backup(struct fs_file *file);
#endif
/* CGI handle function END */

/* SSI handle function START */
MW_ERROR_NO_T ssi_get_port_state_xmlHandle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_l2mc_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
MW_ERROR_NO_T
ssi_get_igmp_snp_querier_info_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
MW_ERROR_NO_T ssi_get_port_state_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_fuState_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_portCur_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
MW_ERROR_NO_T ssi_get_dhcpState_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_IPV6
MW_ERROR_NO_T ssi_get_dadResult_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
#endif
#ifdef AIR_SUPPORT_CABLE_DIAG
MW_ERROR_NO_T ssi_get_cableCur_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
#endif
char ssi_get_language_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_errtype_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_system_name_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_ipsetting_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
void ssi_get_system_name_html_Handle(char** sysName_setting);
void ssi_get_ip_address_Handle(char** ip_address);
void ssi_get_ip_netmask_Handle(char** ip_netmask);
void ssi_get_ip_gateway_Handle(char** ip_gateway);
void ssi_get_dhcp_state_Handle(char** check_dhcp);
char ssi_get_username_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_port_setting_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);

MW_ERROR_NO_T
ssi_get_portmode_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);

MW_ERROR_NO_T ssi_get_mib_state_xmlHandle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_clear_mib_counter_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_connection_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_cookie_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_set_save_setting_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
void ssi_get_igmp_mode_info_Handle(char** igmp_mode);
void ssi_get_reportSu_mode_info_Handle(char** reportSu_mode);
char ssi_get_igmp_snooping_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_porttrunk_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_loopprevention_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_SFP
MW_ERROR_NO_T ssi_get_sfp_module_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif /* AIR_SUPPORT_SFP */
char ssi_get_mirroringport_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_bandwidth_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_jumboframe_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_greenether_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_eeeconfig_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_port_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_port_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_8021q_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_8021q_vlan_pvid_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_8021q_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_mtu_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_vlan_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
#ifdef AIR_SUPPORT_VOICE_VLAN
char ssi_get_voice_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_voice_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
char ssi_get_serveil_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif
char ssi_get_qos_basic_info_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);

char ssi_get_qos_advance_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_ICMP_CLIENT
char ssi_get_ping_info_Handle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
char ssi_get_pinginfo_xmlHandle(int *ptr_length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
#endif /* AIR_SUPPORT_ICMP_CLIENT */
UI16_T ssi_send_port_mirror_ssi_string(C8_T *ptr_ssi_str, UI16_T *ptr_strlen);
char ssi_get_storm_ctrl_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_static_mac_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);

MW_ERROR_NO_T
ssi_get_static_mac_address_entry_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);

C8_T
ssi_get_port_maxmac_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

C8_T ssi_get_vlan_mac_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

C8_T
ssi_get_dynamic_mac_address_entry_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

C8_T
ssi_get_dynamic_mac_address_entry_xmlHandle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

char ssi_get_port_data_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_CABLE_DIAG
MW_ERROR_NO_T ssi_get_cable_diag_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif
char ssi_get_mac_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_DHCP_SNOOP
char ssi_get_dhcp_snooping_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_dhcp_snooping_opt82_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
MW_ERROR_NO_T ssi_get_snmp_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
char ssi_get_spanning_tree_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_spanning_tree_port_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#ifdef AIR_SUPPORT_MSTP
char ssi_get_spanning_tree_region_info_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
char ssi_get_spanning_tree_instance_info_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
char ssi_get_spanning_tree_instance_port_state_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
char ssi_get_spanning_tree_instance_port_role_Handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
char ssi_get_spanning_tree_instance_port_cost_xmlHandle(HTTPD_TAG_PARAM_T *ptr_tag_param);
#endif /*AIR_SUPPORT_MSTP*/
char ssi_get_spanning_tree_security_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
char ssi_get_mqttd_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
char ssi_get_mqttd_info_xmlHandle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_LLDPD
char ssi_get_lldp_config_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags);
MW_ERROR_NO_T ssi_get_lldp_neighbor_handle(HTTPD_TAG_PARAM_T *ptr_tag_param);
MW_ERROR_NO_T ssi_get_neighbor_info_xmlHandle(HTTPD_TAG_PARAM_T *ptr_tag_param);
#endif
#ifdef AIR_SUPPORT_SNTP
char ssi_get_systime_info_handle(int *length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
char ssi_get_timeCur_xmlHandle(int *length, struct tcp_pcb *ptr_pcb, unsigned int apiflags);
#endif

char
ssi_get_port_trunk_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_fw_info_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_chunk_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_lag_process_state_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
char
ssi_get_topLed_info_xmlHandle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);
char
ssi_get_topLed_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

#ifdef AIR_SUPPORT_POE
MW_ERROR_NO_T
ssi_get_poe_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);
MW_ERROR_NO_T
ssi_get_poe_state_xmlHandle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);
#ifdef AIR_SUPPORT_POE_WATCHDOG
MW_ERROR_NO_T
ssi_get_poe_watchdog_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);
#endif
#endif
#ifdef AIR_SUPPORT_ERPS
MW_ERROR_NO_T
ssi_get_erps_info_Handle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);

MW_ERROR_NO_T
ssi_get_erps_state_xmlHandle(
    HTTPD_TAG_PARAM_T *ptr_tag_param);
#endif /* AIR_SUPPORT_ERPS */

MW_ERROR_NO_T
ssi_get_sechdr_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_js_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_css_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_jshdr_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_eraseTime_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

char
ssi_get_sfp_port_data_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

/*=============================================Web color customize start========================================*/

/* FUNCTION NAME: ssi_get_topLoginFormBackground_info_Handle
 * PURPOSE:
 *      To get the top login form background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_topLoginFormBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_bottomLoginFormBackground_info_Handle
 * PURPOSE:
 *      To get the bottom login form background color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_bottomLoginFormBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_loginFormBorder_info_Handle
 * PURPOSE:
 *      To get the login form border color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_loginFormBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_loginFormFont_info_Handle
 * PURPOSE:
 *      To get the login form font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_loginFormFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_topBannerBackground_info_Handle
 * PURPOSE:
 *      To get the top banner background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_topBannerBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_portPanelBackground_info_Handle
 * PURPOSE:
 *      To get the port panel background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_portPanelBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_portNumberFont_info_Handle
 * PURPOSE:
 *      To get the port number font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_portNumberFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_deviceTypeFont_info_Handle
 * PURPOSE:
 *      To get the device type font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_deviceTypeFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_menuBackground_info_Handle
 * PURPOSE:
 *      To get the menu background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_menuText_info_Handle
 * PURPOSE:
 *      To get the menu text color info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuText_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_menuHoverBackground_info_Handle
 * PURPOSE:
 *      To get the menu hover background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuHoverBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_selectedMenuBackground_info_Handle
 * PURPOSE:
 *      To get the selected menu background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_selectedMenuBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageBackground_info_Handle
 * PURPOSE:
 *      To get the sub page background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageFont_info_Handle
 * PURPOSE:
 *      To get the sub page font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageFunctionGroupTitleFont_info_Handle
 * PURPOSE:
 *      To get the sub page function group title font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFunctionGroupTitleFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageFunctionGroupBorder_info_Handle
 * PURPOSE:
 *      To get the sub page function group border info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageFunctionGroupBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageTableHeaderBackground_info_Handle
 * PURPOSE:
 *      To get the sub page table header background info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageTableHeaderBackground_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageTableBorder_info_Handle
 * PURPOSE:
 *      To get the sub page table border info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageTableBorder_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_subPageDivider_info_Handle
 * PURPOSE:
 *      To get the sub page divider info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_subPageDivider_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/* FUNCTION NAME: ssi_get_menuHoverFont_info_Handle
 * PURPOSE:
 *      To get the menu hover font info from the TLV data
 *
 * INPUT:
 *      ptr_pcb              -- The pointer which points to the TCP protocol control block
 *      apiflags             -- HTTP state
 * OUTPUT:
 *      ptr_length           -- Return the total length of bytes number writing to html
 * RETURN:
 *      ERR_OK
 *      ERR_VAL
 *      ERR_INPROGRESS
 * NOTES:
 *      None
 */
char
ssi_get_menuHoverFont_info_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags);

/*=============================================Web color customize end=============================================*/

/*=============================================security end=============================================*/
MW_ERROR_NO_T ssi_get_menu_option_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags, MENU_OPTION_T opList);
/* SSI handle function END */

/* Tools function START */
char
send_format_response(
    u16_t *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags,
    const char *ptr_format, ...);

char
send_format_response_no_chunk(
    u16_t *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags,
    const char *ptr_format, ...);

u16_t SSIHandler(
#if LWIP_HTTPD_SSI_RAW
    const char* ssi_tag_name,
#else /* LWIP_HTTPD_SSI_RAW */
    int iIndex,
#endif /* LWIP_HTTPD_SSI_RAW */
    char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
    , u16_t current_tag_part, u16_t *next_tag_part
#endif /* LWIP_HTTPD_SSI_MULTIPART */
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
    , void *connection_state
#endif /* LWIP_HTTPD_FILE_STATE */
    , char *ptr_chunk_enable);

int get_numCgiHandler();
int get_numSsiTag();
/* Tools function END */
/*=============================================tools end=============================================*/
MW_ERROR_NO_T
ssi_get_customer_homepage_logo_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_login_logo_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_login_copyright_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_addrbar_favicon_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_addrbar_production_version_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_homepage_product_version_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

MW_ERROR_NO_T
ssi_get_customer_menu_copyright_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags);

/*=============================================customization end=============================================*/
/* EXPORTED SUBPROGRAM BODIES
 */
/*FUNCTION NAME: cgi_utility_revisePortBitMapByTrunkInfo
* PURPOSE:
*        to revise the bit map of ports by trunk information
* INPUT:
*       trunkPbm -- the bit map of all trunk members
*       indexStart -- the index of bitmap start from 1 or 0
*       ptr_parent_func -- the pointer which point to the string of parent function
* OUTPUT:
*       ptr_portBitMap -- the port bit map which has been revised by refering trunk information
* RETURN:
*       NONE
* NOTES:
*      None
*/
void
cgi_utility_revisePortBitMapByTrunkInfo(
    UI32_T trunkPbm,
    UI32_T *ptr_portBitMap,
    UI32_T indexStart,
    const C8_T *ptr_parent_func);

/*FUNCTION NAME: ssi_utility_getAllPortType
* PURPOSE:
*        to revise the bit map of ports by trunk information
* INPUT:
*       ptr_list -- the type list of all ports
*       list_len -- the length of type list
* OUTPUT:
*       NONE
* RETURN:
*       MW_E_OK
* NOTES:
*      None
*/
MW_ERROR_NO_T
ssi_utility_getAllPortType(
    UI8_T *ptr_list,
    UI32_T list_len);

/*FUNCTION NAME: cgi_utility_checkPortAvailForCableDiag
 * PURPOSE:
 *        to check the selected port whether available for cable diagnostic
 * INPUT:
 *       port_idx -- the index of port
 * OUTPUT:
 *       NONE
 * RETURN:
 *       FALSE
 *       TRUE
 * NOTES:
 *      None
*/
BOOL_T
cgi_utility_checkPortAvailForCableDiag(
    UI8_T port_idx);

#endif /* __WEB_H__ */
