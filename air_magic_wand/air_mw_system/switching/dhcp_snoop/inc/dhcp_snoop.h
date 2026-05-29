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

/* FILE NAME:  dhcp_snoop.h
 * PURPOSE:
 * It provides DHCP Snooping module API and definitions.
 *
 * NOTES:
 */

#ifndef __DHCP_SNOOP_H__
#define __DHCP_SNOOP_H__
/* INCLUDE FILE DECLARATIONS
 */
#include <mw_error.h>
#include <mw_msg.h>
#include <mw_portbmp.h>
#include <osapi.h>
#include <osapi_message.h>
#include <osapi_mutex.h>
#include <osapi_thread.h>
#include <db_api.h>

#include <lwip/prot/dhcp.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* DHCP snooping features */
// #define DHCP_SNP_DEBUG
// #define DHCP_SNP_DEBUG_DUMP
// #define DHCP_SNP_DEBUG_PROFILING
// #define DHCP_SNP_DEBUG_PKT_PROFILING

// #define DHCP_SNP_BINDING_DB
// #define DHCP_SNP_VLAN

#define DHCP_SNP_MODULE_NAME               "DHCP_SNP"
#define DHCP_SNP_QUEUE_NAME                "dhs"
#define DHCP_SNP_DB_GET_QUEUE_NAME         "dsg"

#define DHCP_SNP_TASK_DELAY                10
#define DHCP_SNP_DB_READY_DELAY            10
#define DHCP_SNP_MUTEX_TIMEOUT             0xFFFFFFFF
#define DHCP_SNP_QUEUE_TIMEOUT             0xFFFFFFFF
#define DHCP_SNP_GET_QUEUE_TIMEOUT         0xFFFFFFFF

#define DHCP_SNP_OPT82_MAX_LEN             255
#define DHCP_SNP_OPT82_CIR_ID_MAX_LEN      MAX_OPT82_ID_LEN
#define DHCP_SNP_OPT82_REM_ID_MAX_LEN      MAX_OPT82_ID_LEN
/* Option 82 Default config */
#define DHCP_SNP_OPT82_CIR_ID_DEFAULT_LEN  3
#define DHCP_SNP_OPT82_REM_ID_DEFAULT_LEN  DHCP_SNP_OPT82_REM_ID_MAC_LEN
/* Option 82 remote id */
#define DHCP_SNP_OPT82_REM_ID_MAC_LEN      MAC_ADDRESS_LEN
#define DHCP_SNP_OPT82_REM_ID_IP_LEN       4

#define DHCP_SNP_PKT_QUEUE_LENGTH          AIR_MAX_SNP_PKT_QUEUE_LEN
#define DHCP_SNP_DB_QUEUE_LENGTH           AIR_MAX_SNP_DB_QUEUE_LEN
#define DHCP_SNP_QUEUE_LENGTH              (DHCP_SNP_DB_QUEUE_LENGTH + DHCP_SNP_PKT_QUEUE_LENGTH)
#define DHCP_SNP_DB_GET_QUEUE_LENGTH       1

typedef enum
{
    DHCP_SNP_OPT82_MODE_NOTOUCH = OPT82_MODE_KEEP,
    DHCP_SNP_OPT82_MODE_REPLACE = OPT82_MODE_REPLACE,
    DHCP_SNP_OPT82_MODE_DISCARD = OPT82_MODE_DROP,
    DHCP_SNP_OPT82_MODE_LAST,
} DHCP_SNP_OPT82_MODE_T;

typedef enum
{
    DHCP_SNP_MSG_NONE   = 0,
    DHCP_SNP_MSG_DB     = MW_MSG_ID_DB,
    DHCP_SNP_MSG_PKT    = 0xff,
} DHCP_SNP_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct DHCP_SNOOP_TASK_S
{
    threadhandle_t    task_handle;
    StaticTask_t      task_tcb;
    StackType_t       task_stack[DHCP_SNP_TASK_STACK_SIZE];
    staticMsghandle_t queue_handle;                        /* Task queue is a static queue. */
    void              *queue_pool[DHCP_SNP_QUEUE_LENGTH];  /* Task queue is a static queue. */
} DHCP_SNOOP_TASK_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_init
 * PURPOSE:
 *      Init DHCP snooping function
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
dhcp_snp_init(
    void);

/* Command APIs for CLI */
/* FUNCTION NAME: dhcp_snp_cmd_setEnable
 * PURPOSE:
 *      Set DHCP snooping enable command API
 *
 * INPUT:
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setEnable(
    const UI8_T enable);

/* FUNCTION NAME: dhcp_snp_cmd_setTrustPort
 * PURPOSE:
 *      Set DHCP snooping trust port command API
 *
 * INPUT:
 *      port     -  port index
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setTrustPort(
    const UI16_T port,
    const UI8_T  enable);

/* FUNCTION NAME: dhcp_snp_cmd_setOption82Enable
 * PURPOSE:
 *      Set DHCP snooping option 82 enable command API
 *
 * INPUT:
 *      port     -  port index
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82Enable(
    const UI16_T port,
    const UI8_T  enable);

/* FUNCTION NAME: dhcp_snp_cmd_setOption82Mode
 * PURPOSE:
 *      Set DHCP snooping option 82 operation mode command API
 *
 * INPUT:
 *      port     -  port index
 *      mode     -  0: no process, 1: replace, 2: drop
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82Mode(
    const UI16_T port,
    const UI8_T  mode);

/* FUNCTION NAME: dhcp_snp_cmd_setOption82CircuitId
 * PURPOSE:
 *      Set DHCP snooping option 82 circuit id command API
 *
 * INPUT:
 *      port     -  port index
 *      type     -  circuit ID type
 *      ptr_id   -  circuit ID string
 *      len      -  circuit ID string length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82CircuitId(
    const UI16_T  port,
    const UI16_T  type,
    const C8_T    *ptr_id,
    const UI8_T   len);

/* FUNCTION NAME: dhcp_snp_cmd_setOption82RemoteId
 * PURPOSE:
 *      Set DHCP snooping option 82 remote id command API
 *
 * INPUT:
 *      port     -  port index
 *      type     -  remote ID type
 *      ptr_id   -  remote ID string
 *      len      -  remote ID string length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82RemoteId(
    const UI16_T  port,
    const UI16_T  type,
    const C8_T    *ptr_id,
    const UI8_T   len);

/* FUNCTION NAME: dhcp_snp_db_dump
 * PURPOSE:
 *      Dump DHCP snooping database function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_db_dump(
    void);

#endif /* End of __DHCP_SNOOP_H__ */
