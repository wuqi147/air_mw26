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

/* FILE NAME:  ace_main.h
 * PURPOSE:
 *      Define Airoha Command Environment module main functions.
 *
 * NOTES:
 */

#ifndef ACE_MAIN_H
#define ACE_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "timer.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ACE_TASK_NAME        "ACE_task"
#define ACE_QUEUE_SIZE       (128)
#define ACE_PKT_QUEUE_NAME   "ACE_pktQ"
#define ACE_STACK_SIZE       (448)
#define ACE_TASK_PRI         (2)
#define ACE_TASK_DELAY       (10)

/* #define ACE_DEBUG */

/* EN8851 register map perport offset is 0x200 */
#define PORT_CTRL_PORT_OFFSET               (0x200)

/* ARL Register Base */
#define REG_ARL_BASE_ADDRESS                (0x10200000)
#define REG_CPU_EN_OFFT                     (15)
#define REG_CPU_PORT_OFFT                   (8)

/* fields of MAC Forward Control */
#define MFC                                 (REG_ARL_BASE_ADDRESS + 0x0010)

/* Register of Forwarding Control Base address */
#define UNUF                                (REG_ARL_BASE_ADDRESS + 0x00B4)
#define UNMF                                (REG_ARL_BASE_ADDRESS + 0x00B8)
#define BCF                                 (REG_ARL_BASE_ADDRESS + 0x00BC)
#define UNIPMF                              (REG_ARL_BASE_ADDRESS + 0x00DC)

/* fields of ingress and egress rate control */
#define ERLCR(p)                            (REG_ARL_BASE_ADDRESS + (p * PORT_CTRL_PORT_OFFSET) + 0xC040)
#define REG_RATE_CIR_OFFT                   (0)
#define REG_RATE_CIR_LENG                   (17)
#define REG_TB_EN_OFFT                      (19)
#define REG_RATE_TB_OFFT                    (20)
#define REG_RATE_TB_LENG                    (4)
#define REG_RATE_CBS_OFFT                   (24)
#define REG_RATE_CBS_LENG                   (7)
#define REG_RATE_EN_OFFT                    (31)

/* fields of global ingress and egress rate control */
#define GERLCR                              (REG_ARL_BASE_ADDRESS + 0xFE00)
#define REG_IPG_BYTE_OFFT                   (0)
#define REG_IPG_BYTE_LENG                   (8)

/* Port Control Base */
#define PORT_CTRL_BASE                      (0x8000)
#define REG_ARL_PORT_BASE_ADDR              (REG_ARL_BASE_ADDRESS + PORT_CTRL_BASE)
#define PORT_CTRL_REG(p, r)                 (PORT_CTRL_BASE + (p) * PORT_CTRL_PORT_OFFSET + (r))

/* fields of PSC */
#define PSC(p)                              (REG_ARL_BASE_ADDRESS + PORT_CTRL_REG(p, 0x0C))
#define PSC_DIS_LRN_OFFSET                  (4)

#define PVC(p)                              (REG_ARL_BASE_ADDRESS + PORT_CTRL_REG(p, 0x10))
#define PVC_SPTAG_EN_OFFT                   (5)
#define PVC_SPTAG_MODE_OFFT                 (11)
#define PVC_SPTAG_EN_RELMASK                (0x00000001)
#define PVC_SPTAG_EN_MASK                   (PVC_SPTAG_EN_RELMASK << PVC_SPTAG_EN_OFFT)
#define PVC_VLAN_ATTR_OFFT                  (6)
#define PVC_VLAN_ATTR_LENG                  (2)
#define PVC_VLAN_ATTR_RELMASK               (0x00000003)
#define PVC_VLAN_ATTR_MASK                  (PVC_VLAN_ATTR_RELMASK << PVC_VLAN_ATTR_OFFT)

#define HAL_SCO_QOS_TOKEN_PERIOD_4MS        (9)
#define HAL_SCO_QOS_L1_RATE_LIMIT           (0x18)

/* MACRO FUNCTION DECLARATIONS
 */

//#define ACE_DEBUG
#ifdef ACE_DEBUG
#define ACE_DBG_PRINT(...) printf(__VA_ARGS__)
#else
#define ACE_DBG_PRINT(...)
#endif

/* DATA TYPE DECLARATIONS
 */

typedef enum
{
    ACE_E_OK = 0,           /* Ok and no error */
    ACE_E_BAD_PARAMETER,    /* Parameter is wrong */
    ACE_E_NO_MEMORY,        /* No memory is available */
    ACE_E_TABLE_FULL,       /* Table is full */
    ACE_E_ENTRY_NOT_FOUND,  /* Entry is not found */
    ACE_E_ENTRY_EXISTS,     /* Entry already exists */
    ACE_E_NOT_SUPPORT,      /* Feature is not supported */
    ACE_E_ALREADY_INITED,   /* Module is reinitialized */
    ACE_E_NOT_INITED,       /* Module is not initialized */
    ACE_E_OTHERS,           /* Other errors */
    ACE_E_ENTRY_IN_USE,     /* Entry is in use */
    ACE_E_TIMEOUT,          /* Time out error */
    ACE_E_OP_INVALID,       /* Operation is invalid */
    ACE_E_OP_STOPPED,       /* Operation is stopped by user callback */
    ACE_E_OP_INCOMPLETE,    /* Operation is incomplete */
    ACE_E_ENTRY_HASH_CONFLICT,  /* Hash index conflict */
    ACE_E_ENTRY_REACH_END,      /* Reach the end of table  */
    ACE_E_LAST
} ACE_ERR_T;

typedef enum
{
    AIR_VLAN_PORT_ATTR_USER_PORT = 0,              /* user port        */
    AIR_VLAN_PORT_ATTR_STACK_PORT,                 /* stack port       */
    AIR_VLAN_PORT_ATTR_TRANSLATION_PORT,           /* translation port */
    AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT,           /* transparent port */
    AIR_VLAN_PORT_ATTR_LAST
} AIR_VLAN_PORT_ATTR_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

ACE_ERR_T ACE_init(void);
void dumb_switch_init();

#endif /* end of ACE_MAIN_H */
