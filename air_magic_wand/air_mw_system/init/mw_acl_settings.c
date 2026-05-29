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


/* FILE NAME:  mw_acl_settings.c
 * PURPOSE:
 * It implements the API for MW ACL settings of some modules.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef AIR_SUPPORT_IPV6
#include "lwip/netif.h"
#include "lwip/ip6_addr.h"
#include "inet_utils.h"
#endif /* AIR_SUPPORT_IPV6 */
#include "osapi_string.h"
#include "osapi_memory.h"
#include "air_acl.h"
#include "air_dos.h"
#include "mw_platform.h"
#include "mgmt_vlan.h"
#include "mw_acl_settings.h"
#include "mw_acl.h"
#include "mw_utils.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_ACL_MODULE_NAME    ("MWA")
#define MW_ACL_MAC_ADDR_LEN   (6)

/* 192kbps */
#define MW_ACL_SYSMAC_ADDR_METER_RATE    (3)

/* 64kbps */
#define MW_ACL_IPV6_ADDR_METER_RATE    (1)

#define MW_ACL_INVALID_ID    (0xFFFFFFFF)

/* DATA TYPE DECLARATIONS
 */
typedef union {
    struct
    {
        UI32_T id: 24;
        UI32_T ref_count: 8;
    } field;
    UI32_T value;
} MW_ACL_RESOURCE_ID_T;

typedef struct {
    MW_ACL_RESOURCE_ID_T sysmac_meter_id;
#ifdef AIR_SUPPORT_IPV6
    MW_ACL_RESOURCE_ID_T ipv6_da_meter_id;
    MW_ACL_RESOURCE_ID_T ipv6_nd_rate_limit_id;
    MW_ACL_RESOURCE_ID_T ipv6_nd_udf_id;
    /* IPv6 solicited-node addresses */
    UI8_T ipv6_solinode_addr[MW_ACL_MAC_ADDR_LEN];
#endif
} __attribute__((packed)) MW_ACL_SETTINGS_CNTX_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
static MW_ACL_SETTINGS_CNTX_T *_ptr_mw_acl_settings;

#ifdef AIR_SUPPORT_IPV6
/* IPv6 all nodes address */
static const UI8_T _mw_acl_ipv6_allnodes_addr[] = {0x33, 0x33, 0, 0, 0, 0x01};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_mw_acl_meter_get(
    MW_ACL_RESOURCE_ID_T *ptr_meter_id,
    UI32_T unit,
    UI32_T meter_rate)
{
    UI32_T meter_id = 0, rate = 0;
    I32_T rc = MW_E_OP_INCOMPLETE;
    BOOL_T meter_enable = FALSE;

    if (((ptr_meter_id->field.id < AIR_MAX_NUM_OF_METER_ID) && (0 == ptr_meter_id->field.ref_count)) ||
        ((ptr_meter_id->field.id >= AIR_MAX_NUM_OF_METER_ID) && (0 < ptr_meter_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (ptr_meter_id->field.id < AIR_MAX_NUM_OF_METER_ID)
    {
        ptr_meter_id->field.ref_count++;
        return MW_E_OK;
    }

    /* Get a disabled meter table */
    for (meter_id = 0; meter_id < AIR_MAX_NUM_OF_METER_ID; meter_id++)
    {
        if ((air_acl_getMeterTable(unit, meter_id, &meter_enable, &rate) == AIR_E_OK) && (FALSE == meter_enable))
        {
            break;
        }
    }

    if (meter_id < AIR_MAX_NUM_OF_METER_ID)
    {
        /* Set the rate limitation. */
        rc = air_acl_setMeterTable(unit, meter_id, TRUE, meter_rate);
        if (AIR_E_OK == rc)
        {
            ptr_meter_id->field.id = meter_id;
            ptr_meter_id->field.ref_count++;
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_meter_release(
    MW_ACL_RESOURCE_ID_T *ptr_meter_id,
    UI32_T unit)
{
    I32_T rc = MW_E_OK;

    if (((ptr_meter_id->field.id < AIR_MAX_NUM_OF_METER_ID) && (0 == ptr_meter_id->field.ref_count)) ||
        ((ptr_meter_id->field.id >= AIR_MAX_NUM_OF_METER_ID) && (0 < ptr_meter_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (0 == ptr_meter_id->field.ref_count)
    {
        return MW_E_OK;
    }

    ptr_meter_id->field.ref_count--;
    if (0 == ptr_meter_id->field.ref_count)
    {
        rc = air_acl_setMeterTable(unit, ptr_meter_id->field.id, FALSE, 0);
        if (AIR_E_OK == rc)
        {
            ptr_meter_id->field.id = AIR_MAX_NUM_OF_METER_ID;
        }
        else
        {
            ptr_meter_id->field.ref_count++;
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

#ifdef AIR_SUPPORT_IPV6
static MW_ERROR_NO_T
_mw_acl_rate_limit_get(
    MW_ACL_RESOURCE_ID_T *ptr_rate_limit_id,
    UI32_T unit,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_limit_cfg)
{
    UI32_T rate_limit_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_limit_cfg;
    I32_T rc = MW_E_OP_INCOMPLETE;

    if (((ptr_rate_limit_id->field.id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID) && (0 == ptr_rate_limit_id->field.ref_count)) ||
        ((ptr_rate_limit_id->field.id >= AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID) && (0 < ptr_rate_limit_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (ptr_rate_limit_id->field.id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID)
    {
        ptr_rate_limit_id->field.ref_count++;
        return MW_E_OK;
    }

    for (; rate_limit_id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID; rate_limit_id++)
    {
        if ((AIR_E_OK == air_dos_getRateLimitCfg(unit, rate_limit_id, &rate_limit_cfg)) &&
            ((0 == rate_limit_cfg.pkt_thld) && (0 == rate_limit_cfg.time_span) && (0 == rate_limit_cfg.block_time)))
        {
            rc = air_dos_setRateLimitCfg(unit, rate_limit_id, ptr_rate_limit_cfg);
            if (AIR_E_OK == rc)
            {
                ptr_rate_limit_id->field.id = rate_limit_id;
                ptr_rate_limit_id->field.ref_count++;
            }
            break;
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_rate_limit_release(
    MW_ACL_RESOURCE_ID_T *ptr_rate_limit_id,
    UI32_T unit)
{
    UI32_T rate_limit_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_limit_cfg;
    I32_T rc = AIR_E_OK;

    if (((ptr_rate_limit_id->field.id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID) && (0 == ptr_rate_limit_id->field.ref_count)) ||
        ((ptr_rate_limit_id->field.id >= AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID) && (0 < ptr_rate_limit_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (0 == ptr_rate_limit_id->field.ref_count)
    {
        return MW_E_OK;
    }

    ptr_rate_limit_id->field.ref_count--;
    if (0 == ptr_rate_limit_id->field.ref_count)
    {
        osapi_memset(&rate_limit_cfg, 0, sizeof(AIR_DOS_RATE_LIMIT_CFG_T));
        rc = air_dos_setRateLimitCfg(unit, rate_limit_id, &rate_limit_cfg);
        if (AIR_E_OK != rc)
        {
            ptr_rate_limit_id->field.ref_count++;
        }
        else
        {
            ptr_rate_limit_id->field.id = AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID;
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_udf_get(
    MW_ACL_RESOURCE_ID_T *ptr_udf_id,
    UI32_T unit,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    UI8_T acl_udf_id = 0;
    I32_T rc = MW_E_OP_INCOMPLETE;
    AIR_ACL_UDF_RULE_T acl_udf_rule;

    if (((ptr_udf_id->field.id < AIR_MAX_NUM_OF_UDF_ENTRY) && (0 == ptr_udf_id->field.ref_count)) ||
        ((ptr_udf_id->field.id >= AIR_MAX_NUM_OF_UDF_ENTRY) && (0 < ptr_udf_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (ptr_udf_id->field.id < AIR_MAX_NUM_OF_UDF_ENTRY)
    {
        ptr_udf_id->field.ref_count++;
        return MW_E_OK;
    }

    for (; acl_udf_id < AIR_MAX_NUM_OF_UDF_ENTRY; acl_udf_id++)
    {
        if ((AIR_E_OK == air_acl_getUdfRule(unit, acl_udf_id, &acl_udf_rule)) && (FALSE == acl_udf_rule.udf_rule_en))
        {
            break;
        }
    }

    if (acl_udf_id >= AIR_MAX_NUM_OF_UDF_ENTRY)
    {
        return MW_E_ENTRY_REACH_END;
    }

    rc = air_acl_setUdfRule(unit, acl_udf_id, ptr_udf_rule);
    if (AIR_E_OK == rc)
    {
        ptr_udf_id->field.id = acl_udf_id;
        ptr_udf_id->field.ref_count++;
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_udf_release(
    MW_ACL_RESOURCE_ID_T *ptr_udf_id,
    UI32_T unit)
{
    I32_T rc = MW_E_OK;
    AIR_ACL_UDF_RULE_T acl_udf_rule;

    if (((ptr_udf_id->field.id < AIR_MAX_NUM_OF_UDF_ENTRY) && (0 == ptr_udf_id->field.ref_count)) ||
        ((ptr_udf_id->field.id >= AIR_MAX_NUM_OF_UDF_ENTRY) && (0 < ptr_udf_id->field.ref_count)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (0 == ptr_udf_id->field.ref_count)
    {
        return MW_E_OK;
    }

    ptr_udf_id->field.ref_count--;
    if (0 == ptr_udf_id->field.ref_count)
    {
        if (AIR_E_OK == air_acl_getUdfRule(unit, (const UI8_T)ptr_udf_id->field.id, &acl_udf_rule))
        {
            if (FALSE == acl_udf_rule.udf_rule_en)
            {
                ptr_udf_id->field.id = AIR_MAX_NUM_OF_UDF_ENTRY;
                return MW_E_OK;
            }

            acl_udf_rule.udf_rule_en = FALSE;
            rc = air_acl_setUdfRule(unit, (const UI8_T)ptr_udf_id->field.id, &acl_udf_rule);
            if (AIR_E_OK != rc)
            {
                ptr_udf_id->field.ref_count++;
            }
            else
            {
                ptr_udf_id->field.id = AIR_MAX_NUM_OF_UDF_ENTRY;
            }
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}
#endif /* AIR_SUPPORT_IPV6 */

static MW_ERROR_NO_T
_mw_acl_rule_set_untag(
    UI32_T unit,
    UI32_T rule_id,
    AIR_ACL_RULE_T *ptr_acl_rule)
{
    I32_T rc = MW_E_OP_INCOMPLETE;

    /* Match untag frames */
    osapi_memset(ptr_acl_rule, 0, sizeof(AIR_ACL_RULE_T));
    ptr_acl_rule->rule_en = FALSE;
    AIR_PORT_BITMAP_COPY(ptr_acl_rule->portmap, PLAT_PORT_BMP_TOTAL);
    AIR_PORT_DEL(ptr_acl_rule->portmap, PLAT_CPU_PORT);
    ptr_acl_rule->end = FALSE;
    ptr_acl_rule->reverse = TRUE;
    ptr_acl_rule->key.ctag = 0x0;
    ptr_acl_rule->mask.ctag = 0x0;
    ptr_acl_rule->field_valid |= (1U << AIR_ACL_CTAG_KEY);
    rc = air_acl_setRule(unit, rule_id, ptr_acl_rule);

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_da_init(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type,
    const UI8_T *dst_addr,
    UI32_T meter_id,
    UI8_T rx_priority,
    UI8_T port_fwd)
{
    AIR_ACL_RULE_T acl_rule, acl_rule_low;
    AIR_ACL_ACTION_T acl_action;
    I32_T rc = MW_E_OP_INCOMPLETE;

    if ((MGMT_VLAN_ACL_TYPE_UNTAG == acl_type) && (FALSE == mw_acl_untag_rule_id_check(acl_rule_id)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if ((air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK) && (FALSE == acl_rule.rule_en) &&
        ((MGMT_VLAN_ACL_TYPE_UNTAG != acl_type) ||
         ((air_acl_getRule(unit, acl_rule_id - 1, &acl_rule_low) == AIR_E_OK) && (FALSE == acl_rule_low.rule_en))))
    {
        if (MGMT_VLAN_ACL_TYPE_UNTAG == acl_type)
        {
            /* Match untag frames */
            rc = _mw_acl_rule_set_untag(unit, acl_rule_id - 1, &acl_rule_low);
            if (MW_E_OK != rc)
            {
                return rc;
            }
        }

        /* Match DA frames */
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = FALSE;
        acl_rule.end = TRUE;
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        osapi_memcpy(acl_rule.key.dmac, dst_addr, MW_ACL_MAC_ADDR_LEN);
        acl_rule.mask.dmac = 0x3f;
        acl_rule.field_valid |= (1U << AIR_ACL_DMAC_KEY);
        if (MGMT_VLAN_ACL_TYPE_PRIORITY_TAG == acl_type)
        {
            acl_rule.key.ctag = 0x0;
            acl_rule.mask.ctag = 0x01;
            acl_rule.field_valid |= (1U << AIR_ACL_CTAG_KEY);
        }
        rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
        if (AIR_E_OK != rc)
        {
            return MW_E_OP_INCOMPLETE;
        }

        osapi_memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
        acl_action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
        acl_action.field_valid |= (1U << AIR_ACL_MIB);
        acl_action.port_fw = port_fwd;
        acl_action.field_valid |= (1U << AIR_ACL_FW_PORT);
        acl_action.pri_user = rx_priority;
        acl_action.field_valid |= (1U << AIR_ACL_PRI);
        if (meter_id < AIR_MAX_NUM_OF_METER_ID)
        {
            acl_action.meter_id = meter_id;
            acl_action.field_valid |= (1U << AIR_ACL_METER);
        }
        rc = air_acl_setAction(unit, acl_rule_id, &acl_action);
        if (AIR_E_OK != rc)
        {
            air_acl_delRule(unit, acl_rule_id);
            if (MGMT_VLAN_ACL_TYPE_UNTAG == acl_type)
            {
                air_acl_delRule(unit, acl_rule_id - 1);
            }
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_da_init_sysmac_addr(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type)
{
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;

    if (NULL == _ptr_mw_acl_settings)
    {
        return MW_E_NOT_INITED;
    }

    rc = _mw_acl_meter_get(&_ptr_mw_acl_settings->sysmac_meter_id, unit, MW_ACL_SYSMAC_ADDR_METER_RATE);
    if (MW_E_OK == rc)
    {
        rc = _mw_acl_da_init(unit,
                             acl_rule_id,
                             acl_type,
                             PLAT_MAC_ADDR,
                             _ptr_mw_acl_settings->sysmac_meter_id.field.id,
                             MW_ACL_RX_PRIORITY_NOMRAL_PACKET,
                             MW_ACL_ACT_PORT_FW_CPU_ONLY);
    }

    if (MW_E_OK != rc)
    {
        _mw_acl_meter_release(&_ptr_mw_acl_settings->sysmac_meter_id, unit);
    }

    return rc;
}

#ifdef AIR_SUPPORT_IPV6
static MW_ERROR_NO_T
_mw_acl_da_init_ipv6_solinode_addr(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type)
{
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;

    if (NULL == _ptr_mw_acl_settings)
    {
        return MW_E_NOT_INITED;
    }

    rc = _mw_acl_meter_get(&_ptr_mw_acl_settings->ipv6_da_meter_id, unit, MW_ACL_IPV6_ADDR_METER_RATE);
    if (MW_E_OK == rc)
    {
        rc = _mw_acl_da_init(unit,
                             acl_rule_id,
                             acl_type,
                             _ptr_mw_acl_settings->ipv6_solinode_addr,
                             _ptr_mw_acl_settings->ipv6_da_meter_id.field.id,
                             MW_ACL_RX_PRIORITY_NOMRAL_PACKET,
                             MW_ACL_ACT_PORT_FW_CPU_INCLUDE);
    }

    if (MW_E_OK != rc)
    {
        _mw_acl_meter_release(&_ptr_mw_acl_settings->ipv6_da_meter_id, unit);
    }

    return rc;
}

static MW_ERROR_NO_T
_mw_acl_da_init_ipv6_allnodes_addr(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type)
{
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;

    if (NULL == _ptr_mw_acl_settings)
    {
        return MW_E_NOT_INITED;
    }

    rc = _mw_acl_meter_get(&_ptr_mw_acl_settings->ipv6_da_meter_id, unit, MW_ACL_IPV6_ADDR_METER_RATE);
    if (MW_E_OK == rc)
    {
        rc = _mw_acl_da_init(unit,
                             acl_rule_id,
                             acl_type,
                             _mw_acl_ipv6_allnodes_addr,
                             _ptr_mw_acl_settings->ipv6_da_meter_id.field.id,
                             MW_ACL_RX_PRIORITY_NOMRAL_PACKET,
                             MW_ACL_ACT_PORT_FW_CPU_INCLUDE);
    }

    if (MW_E_OK != rc)
    {
        _mw_acl_meter_release(&_ptr_mw_acl_settings->ipv6_da_meter_id, unit);
    }

    return rc;
}

static MW_ERROR_NO_T
_mw_acl_ipv6_nd_init_rule(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type,
    UI32_T udf_id,
    UI32_T rate_limit_id)
{
    AIR_ACL_RULE_T acl_rule, acl_rule_low;
    AIR_ACL_ACTION_T acl_action;
    I32_T rc = MW_E_OP_INCOMPLETE;

    /* ICMPv6 ND ACL */
    if ((MGMT_VLAN_ACL_TYPE_UNTAG == acl_type) && (FALSE == mw_acl_untag_rule_id_check(acl_rule_id)))
    {
        return MW_E_BAD_PARAMETER;
    }

    if ((air_acl_getRule(unit, acl_rule_id, &acl_rule) == AIR_E_OK) && (FALSE == acl_rule.rule_en) &&
        ((MGMT_VLAN_ACL_TYPE_UNTAG != acl_type) ||
         ((air_acl_getRule(unit, acl_rule_id - 1, &acl_rule_low) == AIR_E_OK) && (FALSE == acl_rule_low.rule_en))))
    {
        if (MGMT_VLAN_ACL_TYPE_UNTAG == acl_type)
        {
            /* Match untag frames */
            rc = _mw_acl_rule_set_untag(unit, acl_rule_id - 1, &acl_rule_low);
            if (MW_E_OK != rc)
            {
                return rc;
            }
        }

        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = FALSE;
        acl_rule.end = TRUE;
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        acl_rule.isipv6 = TRUE;
        acl_rule.key.etype = ETHTYPE_IPV6;
        acl_rule.mask.etype = 0x3;
        acl_rule.key.next_header = MW_IPV6PROTO_ICMPV6;
        acl_rule.key.udf = BIT(udf_id);
        acl_rule.mask.udf = acl_rule.key.udf;
        acl_rule.field_valid = BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_NEXT_HEADER_KEY) |
                               BIT(AIR_ACL_UDF_KEY);
        if (MGMT_VLAN_ACL_TYPE_PRIORITY_TAG == acl_type)
        {
            acl_rule.key.ctag = 0x0;
            acl_rule.mask.ctag = 0x01;
            acl_rule.field_valid |= (1U << AIR_ACL_CTAG_KEY);
        }
        rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
        if (AIR_E_OK != rc)
        {
            if (MGMT_VLAN_ACL_TYPE_UNTAG == acl_type)
            {
                air_acl_delRule(unit, acl_rule_id - 1);
            }
            return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
        }

        osapi_memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
        acl_action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
        acl_action.field_valid |= (1U << AIR_ACL_MIB);
        acl_action.port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
        acl_action.pri_user = MW_ACL_RX_PRIORITY_ICMPV6_ND;
        acl_action.field_valid = BIT(AIR_ACL_MIB) | BIT(AIR_ACL_FW_PORT) | BIT(AIR_ACL_PRI);
        if (rate_limit_id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID)
        {
            acl_action.attack_rate_id = rate_limit_id;
            acl_action.field_valid |= BIT(AIR_ACL_ATTACK);
        }
        rc = air_acl_setAction(unit, acl_rule_id, &acl_action);
        if (AIR_E_OK != rc)
        {
            air_acl_delRule(unit, acl_rule_id);
            if (MGMT_VLAN_ACL_TYPE_UNTAG == acl_type)
            {
                air_acl_delRule(unit, acl_rule_id - 1);
            }
        }
    }

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static void
_mw_acl_ipv6_nd_set_udf_rule(
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    osapi_memset(ptr_udf_rule, 0, sizeof(AIR_ACL_UDF_RULE_T));
    ptr_udf_rule->udf_rule_en = TRUE;
    AIR_PORT_BITMAP_COPY(ptr_udf_rule->portmap, PLAT_PORT_BMP_TOTAL);
    AIR_PORT_DEL(ptr_udf_rule->portmap, PLAT_CPU_PORT);
    ptr_udf_rule->udf_pkt_type = AIR_ACL_L3_PAYLOAD;
    ptr_udf_rule->word_ofst = 0; /* offset 0 */
    ptr_udf_rule->cmp_sel = 1; /* threshold */
    ptr_udf_rule->cmp_pat = 0x8600; /* low threshold */
    ptr_udf_rule->cmp_mask = 0x89FF; /* high threshold */
}

static MW_ERROR_NO_T
_mw_acl_ipv6_nd_init(
    UI32_T unit,
    UI32_T acl_rule_id,
    MGMT_VLAN_ACL_TYPE acl_type)
{
    MW_ERROR_NO_T rc = MW_E_OP_INCOMPLETE;
    AIR_ACL_UDF_RULE_T udf_rule;
    AIR_DOS_RATE_LIMIT_CFG_T rate_limit_cfg = {40, 1000, 1, AIR_DOS_RATE_TICKSEL_1MS}; /* 40pps */

    if (NULL == _ptr_mw_acl_settings)
    {
        return MW_E_NOT_INITED;
    }

    _mw_acl_ipv6_nd_set_udf_rule(&udf_rule);
    rc = _mw_acl_udf_get(&_ptr_mw_acl_settings->ipv6_nd_udf_id, unit, &udf_rule);
    rc |= _mw_acl_rate_limit_get(&_ptr_mw_acl_settings->ipv6_nd_rate_limit_id, unit, &rate_limit_cfg);
    if (MW_E_OK == rc)
    {
         rc = _mw_acl_ipv6_nd_init_rule(unit,
                                        acl_rule_id,
                                        acl_type,
                                        _ptr_mw_acl_settings->ipv6_nd_udf_id.field.id,
                                        _ptr_mw_acl_settings->ipv6_nd_rate_limit_id.field.id);
    }

    if (MW_E_OK != rc)
    {
        _mw_acl_udf_release(&_ptr_mw_acl_settings->ipv6_nd_udf_id, unit);
        _mw_acl_rate_limit_release(&_ptr_mw_acl_settings->ipv6_nd_rate_limit_id, unit);
    }

    return rc;
}
#endif /* AIR_SUPPORT_IPV6 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* acl_rule_id should be the larger one of the two ACL rules aggregated used by untag frames. */
BOOL_T
mw_acl_untag_rule_id_check(
    UI32_T acl_rule_id)
{
    if ((0 == acl_rule_id) || ((acl_rule_id / 4) != ((acl_rule_id - 1) / 4)))
    {
        return FALSE;
    }

    return TRUE;
}

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id)
{

    return _mw_acl_da_init_sysmac_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id)
{

    return _mw_acl_da_init_sysmac_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_sysmac_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id)
{

    return _mw_acl_da_init_sysmac_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_UNTAG);
}

BOOL_T
mw_acl_da_get_sysmac_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return TRUE;
}

#ifdef AIR_SUPPORT_IPV6
MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_solinode_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_solinode_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_ipv6_solinode_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_solinode_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_UNTAG);
}

BOOL_T
mw_acl_da_get_ipv6_solinode_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return TRUE;
}

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_tag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_allnodes_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_priTag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_allnodes_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG);
}

MW_ERROR_NO_T
mw_acl_da_init_ipv6_allnodes_addr_untag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_da_init_ipv6_allnodes_addr(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_UNTAG);
}

BOOL_T
mw_acl_da_get_ipv6_allnodes_addr_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return TRUE;
}

MW_ERROR_NO_T
mw_acl_ipv6_nd_init_tag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_ipv6_nd_init(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_TAG);
}

MW_ERROR_NO_T
mw_acl_ipv6_nd_init_priTag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_ipv6_nd_init(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_PRIORITY_TAG);
}


MW_ERROR_NO_T
mw_acl_ipv6_nd_init_untag(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return _mw_acl_ipv6_nd_init(unit, acl_rule_id, MGMT_VLAN_ACL_TYPE_UNTAG);
}

BOOL_T
mw_acl_ipv6_nd_get_enable_settings(
    UI32_T unit,
    UI32_T acl_rule_id)
{
    return TRUE;
}
#endif /* AIR_SUPPORT_IPV6 */

MW_ERROR_NO_T
mw_acl_settings_init(
    UI32_T unit)
{
#ifdef AIR_SUPPORT_IPV6
    struct netif *ptr_netif = netif_find_default();
    const ip6_addr_t *ptr_link_local_addr = NULL;
    UI16_T addr_block = 0;

    if (NULL == ptr_netif)
    {
        return MW_E_OP_INCOMPLETE;
    }
#endif

    osapi_calloc(sizeof(MW_ACL_SETTINGS_CNTX_T), MW_ACL_MODULE_NAME, (void **)&_ptr_mw_acl_settings);
    if (NULL == _ptr_mw_acl_settings)
    {
        return MW_E_NO_MEMORY;
    }

#ifdef AIR_SUPPORT_IPV6
    ptr_link_local_addr = netif_ip6_addr(ptr_netif, 0);

    /* Init MAC address of the Solicited-node multicast addresses. */
    _ptr_mw_acl_settings->ipv6_solinode_addr[0] = 0x33;
    _ptr_mw_acl_settings->ipv6_solinode_addr[1] = 0x33;
    _ptr_mw_acl_settings->ipv6_solinode_addr[2] = 0xFF;
    addr_block = IP6_ADDR_BLOCK7(ptr_link_local_addr);
    _ptr_mw_acl_settings->ipv6_solinode_addr[3] = addr_block & 0xFF;
    addr_block = IP6_ADDR_BLOCK8(ptr_link_local_addr);
    _ptr_mw_acl_settings->ipv6_solinode_addr[4] = (addr_block >> 8) & 0xFF;
    _ptr_mw_acl_settings->ipv6_solinode_addr[5] = addr_block & 0xFF;

    _ptr_mw_acl_settings->ipv6_da_meter_id.field.id = AIR_MAX_NUM_OF_METER_ID;
    _ptr_mw_acl_settings->ipv6_nd_udf_id.field.id = AIR_MAX_NUM_OF_UDF_ENTRY;
    _ptr_mw_acl_settings->ipv6_nd_rate_limit_id.field.id = AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID;
#endif

    _ptr_mw_acl_settings->sysmac_meter_id.field.id = AIR_MAX_NUM_OF_METER_ID;

    return MW_E_OK;
}

void
mw_acl_settings_deinit(
    UI32_T unit)
{
    if (NULL != _ptr_mw_acl_settings)
    {
        MW_FREE(_ptr_mw_acl_settings);
    }
}

