/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
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

/* FILE NAME:  lldp.h
 * PURPOSE:
 * It provides LLDP module API and definitions.
 *
 * NOTES:
 */

#ifndef LLDP_H
#define LLDP_H
/* INCLUDE FILE DECLARATIONS
 */
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/ip4.h"
#include "lwip/ip4_addr.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/err.h"
#include "lwip/stats.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/igmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "osapi.h"
#include "osapi_message.h"
#include "osapi_memory.h"
#include "osapi_timer.h"
#include "osapi_thread.h"
#include "osapi_string.h"
#include "osapi_mutex.h"
#include "mw_msg.h"
#include "mw_portbmp.h"
#include "inet_utils.h"
#include "db_api.h"
#include "vlan_utils.h"
#include "ethernetif.h"
#include "platform.h"
#include "sys_mgmt.h"
#include "air_acl.h"

#define LLDP_MODULE_NAME      "lldp"
#define LLDP_QUEUE_NAME   "ldp"
#define LLDP_TASK_NAME        "lldp"
#define LLDP_CLI_QUEUE_NAME   "dpc"
#define LLDP_QUEUE_TIMEOUT         (0xFFFFFFFF)
#define LLDP_MAX_TIMER_NOTIFY      (10)
#define LLDP_MAX_STR_SIZE          (64)
#define LLDP_TASK_DELAY            (10)
#define LLDP_CLI_QUEUE_LENGTH      (1)
#define LLDP_ACCEPTMBOX_SIZE       (4)
#define LLDP_PORT_ID_SIZE          (10)
#define LLDP_STACK_SIZE            (configMINIMAL_STACK_SIZE * 2)

#define LLDP_TX_HOLD_MIN           (2)
#define LLDP_TX_HOLD_MAX           (10)
#define LLDP_TX_HOLD_DEFAULT       (4)
#define LLDP_TX_INTERVAL_MIN       (5)
#define LLDP_TX_INTERVAL_MAX       (32768)
#define LLDP_TX_INTERVAL_DEFAULT   (30)
#define LLDP_REINIT_DELAY_MIN      (1)
#define LLDP_REINIT_DELAY_MAX      (10)
#define LLDP_REINIT_DELAY_DEFAULT  (2)
#define LLDP_TX_DELAY_MIN          (1)
#define LLDP_TX_DELAY_MAX          (8192)
#define LLDP_TX_DELAY_DEFAULT      (2)
#define LLDP_TX_TTL_MAX            (65535)
#define LLDP_PORT_DISABLE          (0)
#define LLDP_PORT_TX_ONLY          (1)
#define LLDP_PORT_RX_ONLY          (2)
#define LLDP_PORT_TX_RX            (3)

#define LLDP_HASH_VALUE_LENGTH     (16)
#define LLDP_TMR_INTERVAL          (1000) /* 1000 Milliseconds */
#define LLDP_ETHER_TYPE            (0x88cc)

#define STAG_BUF_DEFAULT_LEN       (4)
//#define 8021Q_VLAN_HEADER_LEN      (4)
#define ETHER_TYPE_LEN             (2)
#define LLDP_TLV_CHASSIS_ID_LEN    (9)
#define LLDP_TLV_TTL_LEN           (4)
#define LLDP_TLV_MED_CAP_LEN       (9)
#define LLDP_TLV_MED_NET_POLICY_LEN (10)
#define LLDP_TLV_END_LEN           (2)

#define LLDP_TLV_END_TYPE           (0)
#define LLDP_TLV_CHASSIS_ID_TYPE    (1)
#define LLDP_TLV_PORT_ID_TYPE       (2)
#define LLDP_TLV_TTL_TYPE           (3)
#define LLDP_TLV_SYSTEM_NAME_TYPE   (5)
#define LLDP_TLV_MED_TYPE           (127)
#define LLDP_TLV_MED_CAP_SUB_TYPE   (1)
#define LLDP_TLV_MED_NET_SUB_TYPE   (2)
#define LLDP_TLV_MED_EXT_SUB_TYPE   (4)

#define LLDP_DEBUG
#define LLDP_RSTP_PORT_OPER_STAT_DISCARD   (0x3)

typedef struct lldp_cfg_info_s
{
    UI8_T   global_enable;              /* the lldp global status */
    UI8_T   tx_hold;                    /* the multiplier on the msgTxInterval */
    UI16_T  reinit_delay;               /* if port's adminstatus becomes from "enabled" to "disabled", exec reinit after reinit_delay */
    UI16_T  tx_interval;                /* the interval at which LLDP frames are trans */
    UI16_T  tx_delay;                   /* the minimum delay between successive lldp transmissions because of value or status change */
    UI8_T   lldp_admin[MAX_PORT_NUM];  /* the port lldp status */
    UI8_T   port_admin[MAX_PORT_NUM];  /* the port admin status */
    UI8_T   oper_status[MAX_PORT_NUM]; /* the port oper status*/
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    UI8_T   rstp_status[MAX_PORT_NUM]; /* the port rstp status*/
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
    UI8_T   voice_vlan_state;                 /* the state of the Voice VLAN */
    UI16_T  voice_vlan_id;                    /* the ID of the Voice VLAN */
    UI8_T   voice_vlan_pri;                   /* the priority level of the Voice VLAN */
    UI8_T   survei_vlan_state;                /* the state of the Surveillance VLAN */
    UI16_T  survei_vlan_id;                   /* the ID of the Surveillance VLAN */
    UI8_T   survei_vlan_pri;                  /* the priority level of the Surveillance VLAN */
    C8_T    sys_name[MAX_SYS_NAME_SIZE];      /*the system name*/
} lldp_cfg_info_t;

typedef struct lldp_rx_info_s
{
    UI8_T            hash_value[LLDP_HASH_VALUE_LENGTH];
}lldp_rx_info_t;

typedef struct
{
    staticMsghandle_t   msg_queue_handle;
    staticMsghandle_t   cli_queue_handle;
    MW_MSG_T           *msgQueueStorage[LLDP_QUEUE_LENGTH];
    MW_MSG_T           *cliQueueStorage[LLDP_CLI_QUEUE_LENGTH];
} LLDP_QUEUE_CONTEXT_T;

typedef enum
{
    LLDP_GLOBAL_TX = 1,
    LLDP_PORT_INFO_CHANGE_TX,
    LOOP_TX_TYPE_INFO_LAST
}LLDP_TX_TYPE_INFO_T;

typedef union
{
    struct _HEADER_INFO
    {
#ifdef __BIG_ENDIAN
        UI16_T    tlv_length:9;
        UI16_T    tlv_type:7;
#else
        UI16_T    tlv_type:7;
        UI16_T    tlv_length:9;
#endif
    } info;
    UI16_T        header;
} TLV_HEADER_T;

typedef struct lldp_tlv_chassis_id_s
{
    TLV_HEADER_T      tlv_header;
    UI8_T             sub_type;
}lldp_tlv_chassis_id_t;

typedef struct lldp_tlv_port_id_s
{
    TLV_HEADER_T      tlv_header;
    UI8_T             sub_type;
}lldp_tlv_port_id_t;

typedef struct lldp_tlv_ttl_s
{
    TLV_HEADER_T      tlv_header;
    UI16_T            ttl;
}lldp_tlv_ttl_t;

typedef struct lldp_tlv_sysname_s
{
    TLV_HEADER_T      tlv_header;
}lldp_tlv_sysname_t;

typedef struct lldp_tlv_med_cap_s
{
    TLV_HEADER_T      tlv_header;
    UI8_T             oui[3];
    UI8_T             sub_type;
    UI16_T            med_cap;
    UI8_T             device_type;
}lldp_tlv_med_cap_t;

typedef struct lldp_tlv_med_net_policy_s
{
    TLV_HEADER_T      tlv_header;
    UI8_T             oui[3];
    UI8_T             sub_type;
    NETWORK_POLICY_T  policy;
}lldp_tlv_med_net_policy_t;

typedef struct lldp_tlv_med_ext_power_s
{
    TLV_HEADER_T      tlv_header;
    UI8_T             oui[3];
    UI8_T             sub_type;
    EXTEN_POW_VIA_MDI_T ext_power;
}lldp_tlv_med_ext_power_t;

typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[4];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}
md5_context;

typedef enum
{
    LLDP_DEBUG_LEVEL_DISABLE = 0,
    LLDP_DEBUG_LEVEL_ERROR,
    LLDP_DEBUG_LEVEL_WARNING,
    LLDP_DEBUG_LEVEL_INFO,
    LLDP_DEBUG_LEVEL_DEBUG,
    LLDP_DEBUG_LEVEL_LAST
} LLDP_DEBUG_LEVEL_T;

typedef enum
{
    /* lldp msg id base cant be the same with MW_MSG_ID_ETHERNET_PBUF & MW_MSG_ID_DB*/
    LLDP_MSG_ID_BASE = MW_MSG_ID_SELF_DEFINED_BASE,
    LLDP_MSG_ID_TIMER_NOTIFY = LLDP_MSG_ID_BASE,

    /* Define lldp message IDs above this line. */
    LLDP_MSG_ID_LAST = 255
} LLDP_MSG_ID_T;

MW_ERROR_NO_T lldp_init(void);
/* FUNCTION NAME:   lldp_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg lldp glo info.
 *
 * INPUT:
 *      type        --  0-5
                        0:lldp glo enable
                        1:lldp tx interval
                        2:lldp tx hold
                        3:lldp reinit delay
                        4:lldp tx delay
                        5:lldp debug level
                        6:lldp per sys per_mac
                        7:lldp not send blocking
 *      value       --  value
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_cmd_set(UI32_T type, UI32_T value);
/* FUNCTION NAME:   lldp_port_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg lldp port info.
 *
 * INPUT:
 *      port        --  1-28
 *      mode       --   0:disable;1:tx only;2:rx only;3:tx & rx
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_port_cmd_set(UI32_T port_num, UI32_T mode);
#ifdef AIR_SUPPORT_VOICE_VLAN
/* FUNCTION NAME:   lldp_voice_cfg_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg voice info.
 *
 * INPUT:
 *      state        --   0-1(0:voice vlan disable;1:voice vlan enable)
 *      vlanid       --   1-4094
 *      vlanpri        -  0-7
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_voice_cfg_set(UI32_T state, UI32_T vlanid, UI32_T vlanpri);
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/* FUNCTION NAME:   lldp_survei_cfg_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg survei info.
 *
 * INPUT:
 *      state        --   0-1(0:survei vlan disable;1:survei vlan enable)
 *      vlanid       --   1-4094
 *      vlanpri        -  0-7
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_survei_cfg_set(UI32_T state, UI32_T vlanid, UI32_T vlanpri);
#endif

/* FUNCTION NAME:   lldp_dump_cmd
 * PURPOSE:
 *      This API is used for mw cmd to dump lldp port client info.
 *
 * INPUT:
 *      port        --  0-28
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_dump_cmd(UI32_T port_num, UI32_T ttl);

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
/* FUNCTION NAME:   lldp_rstp_port_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg rstp port state.
 *
 * INPUT:
 *      port_num    --  0-28
 *      state       --  0:disable;1:learning;2:forwarding;3:discarding
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lldp_rstp_port_cmd_set(UI32_T port_num, UI32_T state);
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */

#ifndef AIR_EN_CORAL
/* FUNCTION NAME:   lldp_get_global_en
 * PURPOSE:
 *      This API is used for mw stp module to get lldp global enable on EN8851.
 *
 * INPUT:
 *      NONE
 *
 * OUTPUT:
 *      NONE
 *
 * RETURN:
 *      UI8_T
 *
 * NOTES:
 *      None
 */
UI8_T lldp_get_global_en(void);
#endif

#endif
