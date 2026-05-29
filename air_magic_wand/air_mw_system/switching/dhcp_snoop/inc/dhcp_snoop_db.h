/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:  dhcp_snoop_db.h
 * PURPOSE:
 * It provides DHCP Snooping module API and definitions.
 *
 * NOTES:
 */

#ifndef __DHCP_SNOOP_DB_H__
#define __DHCP_SNOOP_DB_H__
/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* circuit/remote id type */
#define DHCP_SNP_OPT82_CIR_ID_DEFAULT         OPT82_CIR_DEFAULT_ID
#define DHCP_SNP_OPT82_CIR_ID_USER_CONFIG     OPT82_CIR_USER_ID
#define DHCP_SNP_OPT82_REM_ID_DEFAULT_MAC     OPT82_REM_DEFAULT_ID_MAC
#define DHCP_SNP_OPT82_REM_ID_DEFAULT_IP      OPT82_REM_DEFAULT_ID_IP
#define DHCP_SNP_OPT82_REM_ID_USER_CONFIG     OPT82_REM_USER_ID
/* circuit/remote id special token */
#define DHCP_SNP_OPT82_ID_HOSTNAME_TOKEN      "%hostname"
#define DHCP_SNP_OPT82_ID_HOSTNAME_TOKEN_LEN  9
#define DHCP_SNP_OPT82_ID_MAC_TOKEN           "%macaddr"
#define DHCP_SNP_OPT82_ID_MAC_TOKEN_LEN       8
#define DHCP_SNP_OPT82_ID_MAC_LEN             17                // 00:11:22:33:44:55
#define DHCP_SNP_OPT82_ID_PORT_TOKEN          "%port"
#define DHCP_SNP_OPT82_ID_PORT_TOKEN_LEN      5
#define DHCP_SNP_OPT82_ID_PORT_LEN            2                 // 01

/* MACRO FUNCTION DECLARATIONS
 */
#define dhcp_snp_db_lock()                    osapi_mutexTake(dhcpsnp_db_mutex, DHCP_SNP_MUTEX_TIMEOUT)
#define dhcp_snp_db_unlock()                  osapi_mutexGive(dhcpsnp_db_mutex)
#define dhcp_snp_db_cfg(f)                    (dhcpsnp_db_cfg.cfg_##f)
#define dhcp_snp_db_set_cfg(f, v)             (dhcp_snp_db_cfg(f) = v)
#define dhcp_snp_db_portcfg(f, e)             (*(dhcp_snp_db_cfg(f) + (e - 1)))
#define dhcp_snp_db_set_portcfg(f, e, v)      (dhcp_snp_db_portcfg(f, e) = v)
#define dhcp_snp_db_clean_port(f)             MW_PORT_BITMAP_CLEAR(dhcp_snp_db_cfg(f))
#define dhcp_snp_db_set_str(f, str, len) { \
        osapi_strncpy((f), (const C8_T *)(str), (len)); \
        ((C8_T *)(f))[len] = 0; \
    }
#define dhcp_snp_db_clean_str(f) { \
        if (f) { osapi_free((f)); } \
        (f) = NULL; \
    }

#define dhcp_snp_dbmsg_payload(ptr_msg)       ((DB_PAYLOAD_T *)(&((ptr_msg)->ptr_payload)))
#define dhcp_snp_dbpayload_data(ptr_payload)  ((UI8_T *)(&((ptr_payload)->ptr_data)))
#define dhcp_snp_dbmsg_init_reqtype(t, f, e)  ((DB_REQUEST_TYPE_T){.t_idx = (t), .f_idx = (f), .e_idx = (e)})

#ifdef DHCP_SNP_DEBUG_DUMP
#define dhcp_snp_db_dbmsg_dump(size, ptr_msg) \
    do {                                                                  \
        DB_PAYLOAD_T *ptr_payload = dhcp_snp_dbmsg_payload((ptr_msg));    \
        C8_T         s[16] = {0};                                         \
        MW_LOG_PRINTF("Msg Dump:");                                        \
        MW_LOG_PRINTF("  ptr_msg=    0x%p ", (ptr_msg));                   \
        osapi_strncpy(s, (ptr_msg)->cq_name, 16);                         \
        MW_LOG_PRINTF("    cq_name=%d, method=0x%X, count=%u, size=%u\n",  \
            s[0], (ptr_msg)->method, (ptr_msg)->type.count, (size));      \
        MW_LOG_PRINTF("  ptr_payload=0x%p ", ptr_payload);                 \
        MW_LOG_PRINTF("    t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u\n",  \
            ptr_payload->request.t_idx, ptr_payload->request.f_idx,       \
            ptr_payload->request.e_idx, ptr_payload->data_size);          \
    } while(0)
#else
#define dhcp_snp_db_dbmsg_dump(size, ptr_msg)
#endif /* DHCP_SNP_DEBUG_DUMP */

/* DATA TYPE DECLARATIONS
 */
typedef struct DHCP_SNP_CFG_S
{
    UI8_T               cfg_global_enable;
    UI8_T               cfg_fwd_miss_match;
    UI8_T               cfg_trust_port[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_option82_enable[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_option82_mode[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_circuit_id_type[MW_DEFAULT_MAX_PORT_NUM];
    C8_T                *cfg_circuit_id[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_circuit_id_len[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_remote_id_type[MW_DEFAULT_MAX_PORT_NUM];
    C8_T                *cfg_remote_id[MW_DEFAULT_MAX_PORT_NUM];
    UI8_T               cfg_remote_id_len[MW_DEFAULT_MAX_PORT_NUM];
} DHCP_SNP_CFG_T;

typedef MW_ERROR_NO_T(*dhcp_snp_send_func_t)(UI32_T, DB_MSG_T *);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
extern DHCP_SNP_CFG_T     dhcpsnp_db_cfg;
extern semaphorehandle_t  dhcpsnp_db_mutex;
extern C8_T               dhcpsnp_db_hostname[MAX_SYS_NAME_SIZE];
extern DB_TRUNK_PORT_T    dhcpsnp_db_trunk_group[MAX_TRUNK_NUM];
extern UI8_T              dhcpsnp_db_port_state[MAX_PORT_NUM];

/* FUNCTION NAME: dhcp_snp_db_sendMsg
 * PURPOSE:
 *      Send to DB request message
 *
 * INPUT:
 *      ptr_target     -  target queue's name
 *      method         -  DB request method
 *      req_type       -  DB request type
 *      ptr_data       -  DB request data
 *      size           -  data size
 *      pptr_msg       -  output msg pointer
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
dhcp_snp_db_sendMsg(
    const C8_T                   *ptr_target,
    const UI8_T                  method,
    const DB_REQUEST_TYPE_T      req_type,
    void  *const                 ptr_data,
    const UI16_T                 size,
    DB_MSG_T                     **pptr_msg);

/* FUNCTION NAME: dhcp_snp_db_update
 * PURPOSE:
 *      Send to DB request message
 *
 * INPUT:
 *      t_idx          -  DB table index
 *      f_idx          -  DB field index
 *      e_idx          -  DB entry index
 *      req_type       -  DB request type
 *      ptr_data       -  DB request data
 *      size           -  data size
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
dhcp_snp_db_update(
    const UI8_T  t_idx,
    const UI8_T  f_idx,
    const UI16_T e_idx,
    void  *const ptr_data,
    const UI16_T size);

/* FUNCTION NAME: dhcp_snp_db_handle
 * PURPOSE:
 *      Handle DB message and update local database
 *
 * INPUT:
 *      ptr_msg        -  DB message
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
dhcp_snp_db_handle(
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: dhcp_snp_db_subscribe
 * PURPOSE:
 *      DHCP snooping database subscribe function
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
dhcp_snp_db_subscribe(
    void);

/* FUNCTION NAME: dhcp_snp_db_deinit
 * PURPOSE:
 *      Deinit DHCP snooping database function
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
dhcp_snp_db_deinit(
    void);

/* FUNCTION NAME: dhcp_snp_db_init
 * PURPOSE:
 *      Init DHCP snooping database function
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
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_init(
    void);

/* FUNCTION NAME: dhcp_snp_db_getVlanMember
 * PURPOSE:
 *      Get VLAN member port bitmap for a certain VLAN id
 *
 * INPUT:
 *      vlan_id
 *
 * OUTPUT:
 *      tx_pbmp
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_db_getVlanMember(
    const UI16_T vlan_id,
    AIR_PORT_BITMAP_T tx_pbmp);

#endif /* End of __DHCP_SNOOP_DB_H__ */
