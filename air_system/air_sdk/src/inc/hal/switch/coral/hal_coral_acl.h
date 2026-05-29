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

/* FILE NAME:  hal_coral_acl.h
 * PURPOSE:
 *  Define ACL module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_CORAL_ACL_H
#define HAL_CORAL_ACL_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_acl.h>
#include <air_error.h>
#include <air_l2.h>
#include <air_port.h>
#include <air_qos.h>
#include <air_swc.h>
#include <air_types.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_MAX_NUM_OF_ACL_ENTRY       (128)
#define HAL_CORAL_MAX_NUM_OF_MIB_ID          (64)
#define HAL_CORAL_MAX_NUM_OF_UDF_ENTRY       (16)
#define HAL_CORAL_MAX_NUM_OF_METER_ID        (32)
#define HAL_CORAL_MAX_NUM_OF_TOKEN           (0xffff)
#define HAL_CORAL_MAX_NUM_OF_VLAN_ID         (4096)
#define HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY     (32)
#define HAL_CORAL_MAX_NUM_OF_DROP_PCD        (8)
#define HAL_CORAL_MAX_NUM_OF_CLASS_SLR       (8)
#define HAL_CORAL_MAX_NUM_OF_USER_PRI        (8)
#define HAL_CORAL_MAX_NUM_OF_MIRROR_PORT     (0xf)
#define HAL_CORAL_MAX_NUM_OF_ATTACK_RATE_ID  (96)
#define HAL_CORAL_MAX_NUM_OF_OFST_TP         (8)
#define HAL_CORAL_MAX_NUM_OF_WORD_OFST       (128)
#define HAL_CORAL_MAX_NUM_OF_CMP_SEL         (2)
#define HAL_CORAL_MAX_NUM_OF_CMP_PAT         (0xffff)
#define HAL_CORAL_MAX_NUM_OF_CMP_BIT         (0xffff)
#define HAL_CORAL_MAX_NUM_OF_DMAC_MASK       (0x3f)
#define HAL_CORAL_MAX_NUM_OF_SMAC_MASK       (0x3f)
#define HAL_CORAL_MAX_NUM_OF_ETYPE_MASK      (0x3)
#define HAL_CORAL_MAX_NUM_OF_STAG_MASK       (0x3)
#define HAL_CORAL_MAX_NUM_OF_CTAG_MASK       (0x3)
#define HAL_CORAL_MAX_NUM_OF_DIP_MASK_IPV4   (0xf)
#define HAL_CORAL_MAX_NUM_OF_SIP_MASK_IPV4   (0xf)
#define HAL_CORAL_MAX_NUM_OF_DIP_MASK_IPV6   (0xffff)
#define HAL_CORAL_MAX_NUM_OF_SIP_MASK_IPV6   (0xffff)
#define HAL_CORAL_MAX_NUM_OF_FLOW_LABEL_MASK (0x7)
#define HAL_CORAL_MAX_NUM_OF_DPORT_MASK      (0x3)
#define HAL_CORAL_MAX_NUM_OF_SPORT_MASK      (0x3)
#define HAL_CORAL_ACL_MAX_BUSY_TIME          (10)
#define HAL_CORAL_MAX_NUM_OF_CBS             (0x3ffff)
#define HAL_CORAL_MAX_NUM_OF_CIR             (0x3ffff)
#define HAL_CORAL_MAX_NUM_OF_PBS             (0x3ffff)
#define HAL_CORAL_MAX_NUM_OF_PIR             (0x3ffff)
#define HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP   (4)
#define HAL_CORAL_IPV6_ADDR_WORD_SIZE        (4)
/* Drop threshold value should not exceed switch free page capacity size */
#define HAL_CORAL_MAX_NUM_OF_DROP_THRSH (1535)

/*ACL rule field offset and bit width*/
/* RULE_TYPE_0 for general rule */
#define RULE_TYPE_OFFSET (0)
#define DMAC_OFFSET      (2)
#define SMAC_OFFSET      (50)
#define STAG_OFFSET      (98)
#define CTAG_OFFSET      (114)
#define ETYPE_OFFSET     (130)
#define DIP_OFFSET       (146)
#define SIP_OFFSET       (178)
#define DSCP_OFFSET      (210)
#define PROTOCOL_OFFSET  (218)
#define DPORT_OFFSET     (226)
#define SPORT_OFFSET     (242)
#define UDF_OFFSET       (258)
#define FIELDMAP_OFFSET  (274)
#define IS_IPV6_OFFSET   (287)
#define PORTMAP_OFFSET   (288)
/* RULE_TYPE_1 for IPv6 */
#define DIP_IPV6_OFFSET   (2)
#define SIP_IPV6_OFFSET   (98)
#define FLOW_LABEL_OFFSET (194)
/* RULE_TYPE_2 for ARP */
#define ARP_THA_OFFSET        (130)
#define ARP_SHA_OFFSET        (178)
#define ARP_TPA_OFFSET        (226)
#define ARP_SPA_OFFSET        (258)
#define RULE2_UDF_OFFSET      (290)
#define RULE2_PORTMAP_OFFSET  (306)
#define RULE2_FIELDMAP_OFFSET (317)

#define RULE_TYPE_WIDTH (2)
/* Fill out per byte to change MAC endian*/
#define DMAC_WIDTH     (8)
#define SMAC_WIDTH     (8)
#define STAG_WIDTH     (16)
#define CTAG_WIDTH     (16)
#define ETYPE_WIDTH    (16)
#define DIP_WIDTH      (32)
#define SIP_WIDTH      (32)
#define DSCP_WIDTH     (8)
#define PROTOCOL_WIDTH (8)
#define DPORT_WIDTH    (16)
#define SPORT_WIDTH    (16)
#define UDF_WIDTH      (16)
#define FIELDMAP_WIDTH (13)
#define IS_IPV6_WIDTH  (1)
#define PORTMAP_WIDTH  (11)

#define DIP_IPV6_WIDTH       (32)
#define SIP_IPV6_WIDTH       (32)
#define FLOW_LABEL_WIDTH     (20)
#define RULE2_FIELDMAP_WIDTH (2)

/*ACL action offset and bit width*/
#define ACL_VLAN_VID_OFFSET   (0)
#define ACL_VLAN_HIT_OFFSET   (12)
#define ACL_CLASS_IDX_OFFSET  (13)
#define ACL_TCM_OFFSET        (18)
#define ACL_TCM_SEL_OFFSET    (20)
#define ACL_DROP_PCD_G_OFFSET (21)
#define ACL_DROP_PCD_Y_OFFSET (24)
#define ACL_DROP_PCD_R_OFFSET (27)
#define CLASS_SLR_OFFSET      (30)
#define CLASS_SLR_SEL_OFFSET  (33)
#define DROP_PCD_SEL_OFFSET   (34)
#define TRTCM_EN_OFFSET       (35)
#define ACL_MANG_OFFSET       (36)
#define LKY_VLAN_OFFSET       (37)
#define LKY_VLAN_EN_OFFSET    (38)
#define EG_TAG_OFFSET         (39)
#define EG_TAG_EN_OFFSET      (42)
#define PRI_USER_OFFSET       (43)
#define PRI_USER_EN_OFFSET    (46)
#define MIRROR_OFFSET         (47)
#define DONT_LEARN_OFFSET     (49)
#define FW_PORT_OFFSET        (50)
#define PORT_FW_EN_OFFSET     (53)
#define RATE_INDEX_OFFSET     (54)
#define RATE_EN_OFFSET        (59)
#define ATTACK_RATE_ID_OFFSET (60)
#define ATTACK_RATE_EN_OFFSET (67)
#define ACL_MIB_ID_OFFSET     (68)
#define ACL_MIB_EN_OFFSET     (74)
#define VLAN_PORT_SWAP_OFFSET (75)
#define DST_PORT_SWAP_OFFSET  (76)
#define BPDU_OFFSET           (77)
#define PORT_OFFSET           (78)
#define PORT_FORCE_OFFSET     (89)

#define ACL_VLAN_VID_WIDTH   (12)
#define ACL_VLAN_HIT_WIDTH   (1)
#define ACL_CLASS_IDX_WIDTH  (5)
#define ACL_TCM_WIDTH        (2)
#define ACL_TCM_SEL_WIDTH    (1)
#define ACL_DROP_PCD_G_WIDTH (3)
#define ACL_DROP_PCD_Y_WIDTH (3)
#define ACL_DROP_PCD_R_WIDTH (3)
#define CLASS_SLR_WIDTH      (3)
#define CLASS_SLR_SEL_WIDTH  (1)
#define DROP_PCD_SEL_WIDTH   (1)
#define TRTCM_EN_WIDTH       (1)
#define ACL_MANG_WIDTH       (1)
#define LKY_VLAN_WIDTH       (1)
#define LKY_VLAN_EN_WIDTH    (1)
#define EG_TAG_WIDTH         (3)
#define EG_TAG_EN_WIDTH      (1)
#define PRI_USER_WIDTH       (3)
#define PRI_USER_EN_WIDTH    (1)
#define MIRROR_WIDTH         (2)
#define DONT_LEARN_WIDTH     (1)
#define FW_PORT_WIDTH        (3)
#define PORT_FW_EN_WIDTH     (1)
#define RATE_INDEX_WIDTH     (5)
#define RATE_EN_WIDTH        (1)
#define ATTACK_RATE_ID_WIDTH (7)
#define ATTACK_RATE_EN_WIDTH (1)
#define ACL_MIB_ID_WIDTH     (6)
#define ACL_MIB_EN_WIDTH     (1)
#define VLAN_PORT_SWAP_WIDTH (1)
#define DST_PORT_SWAP_WIDTH  (1)
#define BPDU_WIDTH           (1)
#define PORT_WIDTH           (11)
#define PORT_FORCE_WIDTH     (1)

/*ACL UDF table offset and width*/
#define UDF_RULE_EN_OFFSET  (0)
#define UDF_PKT_TYPE_OFFSET (1)
#define WORD_OFST_OFFSET    (4)
#define CMP_SEL_OFFSET      (11)
#define CMP_PAT_OFFSET      (32)
#define CMP_MASK_OFFSET     (48)
#define PORT_BITMAP_OFFSET  (64)

#define UDF_RULE_EN_WIDTH  (1)
#define UDF_PKT_TYPE_WIDTH (3)
#define WORD_OFST_WIDTH    (7)
#define CMP_SEL_WIDTH      (1)
#define CMP_PAT_WIDTH      (16)
#define CMP_MASK_WIDTH     (16)
#define PORT_BITMAP_WIDTH  (29)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    HAL_CORAL_ACL_RULE_TYPE_0 = 0,
    HAL_CORAL_ACL_RULE_TYPE_1,
    HAL_CORAL_ACL_RULE_TYPE_2,
    HAL_CORAL_ACL_RULE_TYPE_LAST
} HAL_CORAL_ACL_RULE_TYPE_T;

typedef enum
{
    HAL_CORAL_ACL_RULE_T_CELL,
    HAL_CORAL_ACL_RULE_C_CELL,
    HAL_CORAL_ACL_RULE_TCAM_LAST
} HAL_CORAL_ACL_RULE_TCAM_T;

typedef enum
{
    HAL_CORAL_ACL_MEM_SEL_RULE = 0,
    HAL_CORAL_ACL_MEM_SEL_ACTION,
    HAL_CORAL_ACL_MEM_SEL_LAST
} HAL_CORAL_ACL_MEM_SEL_T;

typedef enum
{
    HAL_CORAL_ACL_MEM_FUNC_READ = 0,
    HAL_CORAL_ACL_MEM_FUNC_WRITE,
    HAL_CORAL_ACL_MEM_FUNC_CLEAR,
    HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ = 4,
    HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
    HAL_CORAL_ACL_MEM_FUNC_LAST
} HAL_CORAL_ACL_MEM_FUNC_T;

typedef enum
{
    HAL_CORAL_ACL_RULE_CONFIG_ENABLE,
    HAL_CORAL_ACL_RULE_CONFIG_END,
    HAL_CORAL_ACL_RULE_CONFIG_REVERSE,
    HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE,
    HAL_CORAL_ACL_RULE_CONFIG_LAST
} HAL_CORAL_ACL_RULE_CONFIG_T;

typedef enum
{
    HAL_CORAL_ACL_CHECK_ACL,
    HAL_CORAL_ACL_CHECK_UDF,
    HAL_CORAL_ACL_CHECK_TRTCM,
    HAL_CORAL_ACL_CHECK_METER,
    HAL_CORAL_ACL_CHECK_TYPE_LAST
} HAL_CORAL_ACL_CHECK_TYPE_T;

typedef enum
{
    HAL_CORAL_ACL_DMAC = 0,
    HAL_CORAL_ACL_SMAC,
    HAL_CORAL_ACL_STAG,
    HAL_CORAL_ACL_CTAG,
    HAL_CORAL_ACL_ETYPE,
    HAL_CORAL_ACL_DIP,
    HAL_CORAL_ACL_SIP,
    HAL_CORAL_ACL_DSCP,
    HAL_CORAL_ACL_PROTOCOL,
    HAL_CORAL_ACL_DPORT,
    HAL_CORAL_ACL_SPORT,
    HAL_CORAL_ACL_UDF,
    HAL_CORAL_ACL_FLOW_LABEL,
    HAL_CORAL_ACL_FIELD_TYPE_LAST
} HAL_CORAL_ACL_FIELD_TYPE_T;

typedef enum
{
    HAL_CORAL_ACL_ARP_STAG = 0,
    HAL_CORAL_ACL_ARP_CTAG,
    HAL_CORAL_ACL_ARP_FIELD_TYPE_LAST
} HAL_CORAL_ACL_ARP_FIELD_TYPE_T;

typedef enum
{
    HAL_CORAL_ACL_ACT_FWD_DIS,        /* Don't change forwarding behaviour by ACL */
    HAL_CORAL_ACL_ACT_FWD_CPU_EX = 4, /* Forward by system default & CPU port is excluded */
    HAL_CORAL_ACL_ACT_FWD_CPU_IN,     /* Forward by system default & CPU port is included */
    HAL_CORAL_ACL_ACT_FWD_CPU,        /* Forward to CPU port only */
    HAL_CORAL_ACL_ACT_FWD_DROP,       /* Frame dropped */
    HAL_CORAL_ACL_ACT_FWD_LAST
} HAL_CORAL_ACL_ACT_FWD_T;

typedef enum
{
    HAL_CORAL_ACL_ACT_EGTAG_DIS,
    HAL_CORAL_ACL_ACT_EGTAG_CONSISTENT,
    HAL_CORAL_ACL_ACT_EGTAG_UNTAG = 4,
    HAL_CORAL_ACL_ACT_EGTAG_SWAP,
    HAL_CORAL_ACL_ACT_EGTAG_TAG,
    HAL_CORAL_ACL_ACT_EGTAG_STACK,
    HAL_CORAL_ACL_ACT_EGTAG_LAST
} HAL_CORAL_ACL_ACT_EGTAG_T;

typedef enum
{
    HAL_CORAL_ACL_ACT_USR_TCM_DEFAULT, /* Normal packets, don't work based on color */
    HAL_CORAL_ACL_ACT_USR_TCM_GREEN,   /* Green */
    HAL_CORAL_ACL_ACT_USR_TCM_YELLOW,  /* Yellow */
    HAL_CORAL_ACL_ACT_USR_TCM_RED,     /* Red */
    HAL_CORAL_ACL_ACT_USR_TCM_LAST
} HAL_CORAL_ACL_ACT_USR_TCM_T;

typedef struct HAL_CORAL_ACL_CTRL_S
{
    BOOL_T rule_en;
    BOOL_T reverse;
    BOOL_T end;
    BOOL_T end_reverse; /* The second-reverse to reverse the aggregation-hit */
} HAL_CORAL_ACL_CTRL_T;

typedef struct HAL_CORAL_ACL_ACT_TRTCM_S
{
    BOOL_T                      cls_slr_sel;  /* FALSE: Select original class selector value
                                                 TRUE:  Select ACL control table defined class selector value */
    UI8_T                       cls_slr;      /* User defined class selector */

    BOOL_T                      drop_pcd_sel; /* FALSE: Select original drop precedence value
                                                 TRUE:  Select ACL control table defined drop precedence value */
    UI8_T                       drop_pcd_r;   /* User defined drop precedence for red packets */
    UI8_T                       drop_pcd_y;   /* User defined drop precedence for yellow packets */
    UI8_T                       drop_pcd_g;   /* User defined drop precedence for green packets */

    BOOL_T                      tcm_sel;      /* FALSE: Select user defined color value
                                                 TRUE:  Select color remark by trtcm table */
    HAL_CORAL_ACL_ACT_USR_TCM_T usr_tcm;      /* User defined color remark */
    UI8_T                       tcm_idx;      /* Index for the 32-entries trtcm table */
} HAL_CORAL_ACL_ACT_TRTCM_T;

typedef struct HAL_CORAL_ACL_ACTION_S
{
    BOOL_T                    port_en;
    BOOL_T                    dest_port_sel; /* Swap destination port member by portmap when port_en=1 */
    BOOL_T                    vlan_port_sel; /* Swap VLAN port member by portmap when port_en=1 */
    AIR_PORT_BITMAP_T         portmap;

    BOOL_T                    cnt_en;
    UI32_T                    cnt_idx; /* Counter index */

    BOOL_T                    attack_en;
    UI32_T                    attack_idx; /* Attack rate index */

    BOOL_T                    rate_en;
    UI32_T                    rate_idx; /* Index of meter table */

    BOOL_T                    vlan_en;
    UI32_T                    vlan_idx;   /* Vid from ACL */

    UI8_T                     mirrormap;  /* mirror session bitmap */

    BOOL_T                    dont_learn; /* L2 Address table don't learn */

    BOOL_T                    pri_user_en;
    UI8_T                     pri_user; /* User Priority from ACL */

    BOOL_T                    lyvlan_en;
    BOOL_T                    lyvlan; /* Leaky VLAN */

    BOOL_T                    mang;   /* Management frame attribute */

    BOOL_T                    bpdu;   /* Regard as BPDU frame attribute */

    BOOL_T                    fwd_en;
    HAL_CORAL_ACL_ACT_FWD_T   fwd; /* Frame TO_CPU Forwarding */

    BOOL_T                    egtag_en;
    HAL_CORAL_ACL_ACT_EGTAG_T egtag; /* Egress tag control */

    BOOL_T                    trtcm_en;
    HAL_CORAL_ACL_ACT_TRTCM_T trtcm; /* TRTCM control */

} HAL_CORAL_ACL_ACTION_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:
 *      hal_coral_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      ptr_enable       -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getGlobalState(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_coral_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit        -- unit id
 *      enable      -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setGlobalState(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_coral_acl_getPortState
 * PURPOSE:
 *      Get enable status of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getPortState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_coral_acl_setPortState
 * PURPOSE:
 *      Set enable state of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 *      enable          -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setPortState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_coral_acl_clearAll
 * PURPOSE:
 *      Clear ACL all rule and action HW memory.
 * INPUT:
 *      unit        -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_clearAll(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_coral_acl_getRule
 * PURPOSE:
 *      Get ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule         -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_setRule
 * PURPOSE:
 *      Add/Update ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 *      ptr_rule         -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1.If isipv6=1, dip&sip&dscp&next header will hit only when packet is IPv6;
 *      2.Every 4 rule can do aggregation. For rule aggregation, final hit id rule-end must be 1;
 *        When rule-end=0, means that it can be aggregated with next rule;
 *        Rule3 cannot aggregate with rule4, if rule3 end=0, rule3 will be an invalid rule;
 *      3.For reverse function use, do reverse before rule aggregation.
 *      4.When byte mask of each field is set 0, key of each field must be set 0;
 *      5.Udf-list is bit mask of udf-rule entry, range 1-16;
 */
AIR_ERROR_NO_T
hal_coral_acl_setRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_delRule
 * PURPOSE:
 *      Delete ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_delRule(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:
 *      hal_coral_acl_getAction
 * PURPOSE:
 *      Get ACL action of specified entry index.
 * INPUT:
 *      unit              -- unit id
 *      entry_idx         -- ACL action entry index
 * OUTPUT:
 *      ptr_action        -- pointer buffer of action
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:
 *      hal_coral_acl_setAction
 * PURPOSE:
 *      Add/Update ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 *      ptr_action       -- pointer buffer of action
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If acl-tcm=defined, the corresponding action is defined-color; If acl-tcm=meter, the corresponding action is meter-id;
 */
AIR_ERROR_NO_T
hal_coral_acl_setAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

/* FUNCTION NAME:
 *      hal_coral_acl_delAction
 * PURPOSE:
 *      Delete ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_delAction(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME:
 *      hal_coral_acl_getMibCnt
 * PURPOSE:
 *      Get ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 *      cnt_index        -- mib counter index
 * OUTPUT:
 *      ptr_cnt          -- pointer to receive count
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has total 64 mib counters, the counter index can be set by hal_coral_acl_setAction.
 */
AIR_ERROR_NO_T
hal_coral_acl_getMibCnt(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt);

/* FUNCTION NAME:
 *      hal_coral_acl_clearMibCnt
 * PURPOSE:
 *      Clear ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      For clear all mib counters, ACL has total 64 mib counters, the counter index can be set by hal_coral_acl_setAction.
 */
AIR_ERROR_NO_T
hal_coral_acl_clearMibCnt(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_coral_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule     -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 *      ptr_udf_rule     -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If cmp=pattern, the corresponding setting is cmp-key&cmp-mask; If cmp=threshold, the corresponding setting is cmp-low&cmp-high;
 */
AIR_ERROR_NO_T
hal_coral_acl_setUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_delUdfRule(
    const UI32_T unit,
    const UI8_T  entry_idx);

/* FUNCTION NAME:
 *      hal_coral_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_clearUdfRule(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_coral_acl_getMeterTable
 * PURPOSE:
 *      get meter table configuration.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 * OUTPUT:
 *      ptr_enable          -- meter enable state
 *      ptr_rate            -- ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate);

/* FUNCTION NAME:
 *      hal_coral_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 *      enable              -- meter enable state
 *      rate                -- ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate);

/* FUNCTION NAME:
 *      hal_coral_acl_getDropEn
 * PURPOSE:
 *      Get enable state of drop precedence on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getDropEn(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_coral_acl_setDropEn
 * PURPOSE:
 *      Set enable state of drop precedence on specified port.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      enable      -- enable state of drop precedence
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setDropEn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:
 *      hal_coral_acl_getDropThrsh
 * PURPOSE:
 *      Get ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_high        --  High threshold
 *      ptr_low         --  Low threshold
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_coral_acl_getDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low);

/* FUNCTION NAME:
 *      hal_coral_acl_setDropThrsh
 * PURPOSE:
 *      Set ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      high            --  High threshold
 *      low             --  Low threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_coral_acl_setDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low);

/* FUNCTION NAME:
 *      hal_coral_acl_getDropPbb
 * PURPOSE:
 *      Get ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_pbb         --  Drop probability(unit:1/1023)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_coral_acl_getDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb);

/* FUNCTION NAME:
 *      hal_coral_acl_setDropPbb
 * PURPOSE:
 *      Set ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      pbb             --  Drop probability(unit:1/1023)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_coral_acl_setDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb);

/* FUNCTION NAME:
 *      hal_coral_acl_getDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            -- unit id
 * OUTPUT:
 *      ptr_type        -- TRUE: Exclude management frame
 *                         FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getDropExMfrm(
    const UI32_T unit,
    BOOL_T      *ptr_type);

/* FUNCTION NAME:
 *      hal_coral_acl_setDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            --  unit id
 *      type            --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setDropExMfrm(
    const UI32_T unit,
    const BOOL_T type);

/* FUNCTION NAME:
 *      hal_coral_acl_getTrtcmEn
 * PURPOSE:
 *      Get TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_enable      --  Enable/Disable trTCM
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getTrtcmEn(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:
 *      hal_coral_acl_setTrtcmEn
 * PURPOSE:
 *      Set TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  Enable/Disable TRTCM
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setTrtcmEn(
    const UI32_T unit,
    BOOL_T       enable);

/* FUNCTION NAME:
 *      hal_coral_acl_getTrtcmTable
 * PURPOSE:
 *      Get a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *
 * OUTPUT:
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:
 *      hal_coral_acl_setTrtcmTable
 * PURPOSE:
 *      Set a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME:
 *      hal_coral_acl_init
 * PURPOSE:
 *      This API is used to init acl.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_init(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_coral_acl_deinit
 * PURPOSE:
 *      This API is used to deinit acl.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_deinit(
    const UI32_T unit);

/* FUNCTION NAME:
 *      hal_coral_acl_getArpRule
 * PURPOSE:
 *      Get ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 * OUTPUT:
 *      ptr_rule         -- pointer buffer of ARP rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_setArpRule
 * PURPOSE:
 *      Add ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 *      ptr_rule         -- pointer buffer of ARP rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

/* FUNCTION NAME:
 *      hal_coral_acl_delArpRule
 * PURPOSE:
 *      Delete ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_delArpRule(
    const UI32_T unit,
    const UI32_T entry_idx);

/* FUNCTION NAME: hal_coral_acl_getCapacity
 * PURPOSE:
 *      Get the acl resource capacity
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_coral_acl_getUsage
 * PURPOSE:
 *      Get the acl resource usage
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_cnt         --  Count of usage
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* end of HAL_CORAL_ACL_H */
