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

/* FILE NAME:  syncd_api_stp.c
 * PURPOSE:
 *  Implement STP API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <air_swc.h>
#include <air_acl.h>
#include <air_port.h>
#include "mw_utils.h"
#include "mw_acl.h"
#include "syncd_api_stp.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define SYNCD_API_STP_DEL_ACL(entry_id)         \
    do {                                        \
        if (MW_ACL_ID_INVALID != entry_id)      \
        {                                       \
            air_acl_delAction(0, entry_id);     \
            air_acl_delRule(0, entry_id);       \
        }                                       \
    } while (0)

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T _port_block_bmp = 0;
#ifdef AIR_SUPPORT_LLDPD
static UI8_T _lldp_state = FALSE;
static UI32_T _lldp_acl_id[SYNCD_API_STP_ACL_LAST] = {0};
#endif
static SYNCD_API_STP_PORT_INFO_T *_port_state_bmp = NULL;
#ifdef AIR_SUPPORT_MSTP
static SYNCD_API_STP_INS_PORT_INFO_T *_ptr_port_info_state_bmp = NULL;
#endif

/* LOCAL SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_LLDPD
static MW_ERROR_NO_T
_syncd_api_stp_getLldpAcl(
    SYNCD_API_STP_ACL_T type,
    UI32_T              *ptr_entryId,
    AIR_ACL_RULE_T      *ptr_rule,
    AIR_ACL_ACTION_T    *ptr_action)
{
    UI32_T              unit = 0, i;

    if ((MW_E_OK == mw_acl_getAvailableRule(ptr_entryId, ptr_rule, unit, MW_ACL_ID_DYNAMIC_MIN))
        && (MW_ACL_ID_INVALID != (*ptr_entryId)))
    {
        osapi_memset(ptr_rule, 0, sizeof(AIR_ACL_RULE_T));
        osapi_memset(ptr_action, 0, sizeof(AIR_ACL_ACTION_T));
#ifndef AIR_EN_CORAL
        if (SYNCD_API_STP_ACL_DROP == type)
        {
            ptr_action->port_fw = MW_ACL_ACT_PORT_FW_DROP;
            ptr_action->field_valid = (1U << AIR_ACL_FW_PORT);
            ptr_rule->rule_en = TRUE;
            ptr_rule->end = TRUE;
            ptr_rule->key.etype = SYNCD_API_STP_LLDP_ETYPE;
            ptr_rule->mask.etype = 0x3;
            ptr_rule->field_valid |= (1U << AIR_ACL_ETYPE_KEY);
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if (BIT_CHK(_port_block_bmp, i))
                {
                    AIR_PORT_ADD(ptr_rule->portmap, (i+1));
                    continue;
                }
                if ((0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_DISABLE])
                    || (0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_LISTEN])
                    || (0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_LEARN]))
                {
                    AIR_PORT_ADD(ptr_rule->portmap, (i+1));
                }
            }
        }
        else
#endif
        {
            ptr_rule->rule_en = TRUE;
            ptr_rule->end = TRUE;
            ptr_rule->key.etype = SYNCD_API_STP_LLDP_ETYPE;
            ptr_rule->mask.etype = 0x3;
            ptr_rule->field_valid |= (1U << AIR_ACL_ETYPE_KEY);
            ptr_action->dst_port_swap = 1;
            ptr_action->port_fw = MW_ACL_ACT_PORT_FW_CPU_EXCLUDE;
            ptr_action->field_valid = (1U << AIR_ACL_PORT) | (1U << AIR_ACL_FW_PORT);

            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if (BIT_CHK(_port_block_bmp, i))
                {
                    continue;
                }
                if ((0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_DISABLE])
                    || (0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_LISTEN])
                    || (0 != _port_state_bmp[i].user_bmp[AIR_STP_STATE_LEARN]))
                {
                    continue;
                }
                AIR_PORT_ADD(ptr_rule->portmap, (i+1));
                AIR_PORT_ADD(ptr_action->portmap, (i+1));
            }
        }
    }
    else
    {
        return MW_E_TABLE_FULL;
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_syncd_api_stp_addLldpAcl(
    void)
{
    UI32_T              unit = 0;
    AIR_ACL_RULE_T      acl_rule;
    UI32_T              entry_id;
    AIR_ACL_ACTION_T    acl_action;
    AIR_ERROR_NO_T      air_rc = AIR_E_OK;
    MW_ERROR_NO_T       ret;

    SYNCD_API_STP_DEL_ACL(_lldp_acl_id[SYNCD_API_STP_ACL_FWD]);
    ret = _syncd_api_stp_getLldpAcl(SYNCD_API_STP_ACL_FWD, &entry_id, &acl_rule, &acl_action);
    if (MW_E_OK != ret)
    {
        SYNCD_LOG_ERROR("Get lldp acl failed, rc=%u", ret);
        return ret;
    }

    if (!AIR_PORT_BITMAP_EMPTY(acl_rule.portmap))
    {
        air_rc = air_acl_setRule(unit, entry_id, &acl_rule);
        if (AIR_E_OK == air_rc)
        {
          /* keep rule ID */
          _lldp_acl_id[SYNCD_API_STP_ACL_FWD] = entry_id;
        }
        else
        {
            SYNCD_LOG_DEBUG("Add lldp rule-id %u failed, rc=%u", entry_id, air_rc);
        }

        if (AIR_E_OK == air_rc)
        {
            air_rc = air_acl_setAction(unit, entry_id, &acl_action);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_DEBUG("Add lldp action-id %u failed, rc=%u", entry_id, air_rc);
            }
        }
    }
#ifndef AIR_EN_CORAL
    SYNCD_API_STP_DEL_ACL(_lldp_acl_id[SYNCD_API_STP_ACL_DROP]);
    ret = _syncd_api_stp_getLldpAcl(SYNCD_API_STP_ACL_DROP, &entry_id, &acl_rule, &acl_action);
    if (MW_E_OK != ret)
    {
        SYNCD_LOG_ERROR("Get lldp acl failed, rc=%u", ret);
        return ret;
    }

    if (!AIR_PORT_BITMAP_EMPTY(acl_rule.portmap))
    {
        air_rc = air_acl_setRule(unit, entry_id, &acl_rule);
        if (AIR_E_OK == air_rc)
        {
          /* keep rule ID */
          _lldp_acl_id[SYNCD_API_STP_ACL_DROP] = entry_id;
        }
        else
        {
            SYNCD_LOG_DEBUG("Add lldp drop rule-id %u failed, rc=%u", entry_id, air_rc);
        }

        if (AIR_E_OK == air_rc)
        {
            air_rc = air_acl_setAction(unit, entry_id, &acl_action);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_DEBUG("Add lldp drop action-id %u failed, rc=%u", entry_id, air_rc);
            }
        }
    }
#endif
    if (AIR_E_OK != air_rc)
    {
        return MW_E_OTHERS;
    }

    return MW_E_OK;
}

static void
_syncd_api_stp_delLldpAcl(
    SYNCD_API_STP_ACL_T type)
{
    if (type >= SYNCD_API_STP_ACL_LAST)
    {
        return;
    }
    SYNCD_API_STP_DEL_ACL(_lldp_acl_id[type]);
    _lldp_acl_id[type] = MW_ACL_ID_INVALID;

    return;
}

static void
_syncd_api_stp_delLldpAllAcl(
    void)
{
    UI32_T i;

    for (i = 0; i < SYNCD_API_STP_ACL_LAST; i++)
    {
        _syncd_api_stp_delLldpAcl(i);
    }

    return;
}

static MW_ERROR_NO_T
_syncd_api_stp_updateAclPort(
    UI32_T port,
    AIR_STP_STATE_T state)
{
    AIR_ACL_ACTION_T    acl_action;
    AIR_ERROR_NO_T      air_rc = AIR_E_OK;
    UI32_T              unit = 0, i;
    UI32_T              entryId;
    AIR_ACL_RULE_T      acl_rule;
    MW_ERROR_NO_T       ret;

    if (TRUE == _lldp_state)
    {
        return MW_E_OK;
    }

    ret = mw_acl_mutex_take();
    if (MW_E_OK != ret)
    {
        return ret;
    }

    for (i = 0; i < SYNCD_API_STP_ACL_LAST; i++)
    {
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        osapi_memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
        entryId = _lldp_acl_id[i];
        if (MW_ACL_ID_INVALID == entryId)
        {
            ret = _syncd_api_stp_getLldpAcl(i, &entryId, &acl_rule, &acl_action);
            if (MW_E_OK != ret)
            {
                SYNCD_LOG_ERROR("Get lldp acl failed, rc=%u", ret);
                break;
            }
        }
        else
        {
            air_rc = air_acl_getRule(unit, entryId, &acl_rule);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_ERROR("Get lldp type %d acl rule id %d fail, rc %d",i, entryId, air_rc);
                break;
            }

            air_rc = air_acl_getAction(unit, entryId, &acl_action);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_ERROR("Get lldp type %d acl action id %d fail, rc %d",i, entryId, air_rc);
                break;
            }
        }

        if (SYNCD_API_STP_ACL_FWD == i)
        {
            if (AIR_STP_STATE_FORWARD == state)
            {
                AIR_PORT_ADD(acl_rule.portmap, port);
                air_rc = air_acl_setRule(unit, entryId, &acl_rule);
                AIR_PORT_ADD(acl_action.portmap, port);
                air_rc = air_acl_setAction(unit, entryId, &acl_action);
                if (AIR_E_OK == air_rc)
                {
                    _lldp_acl_id[i] = entryId;
                }
            }
            else
            {
                AIR_PORT_DEL(acl_rule.portmap, port);
                if (!AIR_PORT_BITMAP_EMPTY(acl_rule.portmap))
                {
                    air_rc = air_acl_setRule(unit, entryId, &acl_rule);
                    AIR_PORT_DEL(acl_action.portmap, port);
                    air_rc = air_acl_setAction(unit, entryId, &acl_action);
                    if (AIR_E_OK == air_rc)
                    {
                        _lldp_acl_id[i] = entryId;
                    }
                }
                else
                {
                    _syncd_api_stp_delLldpAcl(i);
                }
            }
        }
#ifndef AIR_EN_CORAL
        else if (SYNCD_API_STP_ACL_DROP == i)
        {
            if (AIR_STP_STATE_FORWARD == state)
            {
                AIR_PORT_DEL(acl_rule.portmap, port);
            }
            else
            {
                AIR_PORT_ADD(acl_rule.portmap, port);
            }

            if (!AIR_PORT_BITMAP_EMPTY(acl_rule.portmap))
            {
                air_rc = air_acl_setRule(unit, entryId, &acl_rule);
                air_rc = air_acl_setAction(unit, entryId, &acl_action);
                if (AIR_E_OK == air_rc)
                {
                    _lldp_acl_id[i] = entryId;
                }
            }
            else
            {
                _syncd_api_stp_delLldpAcl(i);
            }
        }
#endif
    }
    mw_acl_mutex_release();

    if (AIR_E_OK != air_rc)
    {
        SYNCD_LOG_ERROR("Add port %d state %d lldp acl failed, rc=%u",
            port, state, air_rc);
        return MW_E_OTHERS;
    }

    return MW_E_OK;
}

#ifdef AIR_EN_CORAL
static void
_syncd_api_stp_setAllPortMgmtFrame(
    UI8_T state)
{
    AIR_SWC_MGMT_FRAME_CFG_T    frame_cfg;
    UI32_T port, unit = 0;
    AIR_ERROR_NO_T air_rc;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }

        if ((0 == _port_state_bmp[port-1].user_bmp[AIR_STP_STATE_DISABLE])
            && (0 == _port_state_bmp[port-1].user_bmp[AIR_STP_STATE_LISTEN])
            && (0 == _port_state_bmp[port-1].user_bmp[AIR_STP_STATE_LEARN]))
        {
            continue;
        }
        osapi_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;

        air_rc = air_swc_getPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            continue;
        }

        if (FALSE == state)
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }

        air_rc = air_swc_setPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            SYNCD_LOG_ERROR("Call function failed(%d)", air_rc);
        }
    }


    _lldp_state = state;

    return;
}

static void
_syncd_api_stp_updatePortMgmtFrame(
    UI32_T port,
    AIR_STP_STATE_T state)
{
    AIR_SWC_MGMT_FRAME_CFG_T    frame_cfg;
    UI32_T unit = 0;
    AIR_ERROR_NO_T air_rc;

    if (TRUE == _lldp_state)
    {
        osapi_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
        air_rc = air_swc_getPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            return;
        }

        if (AIR_STP_STATE_FORWARD != state)
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }

        air_rc = air_swc_setPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            return;
        }
    }
    else
    {
        osapi_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
        air_rc = air_swc_getPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            return;
        }

        if (AIR_STP_STATE_FORWARD != state)
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }

        air_rc = air_swc_setPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != air_rc)
        {
            return;
        }
    }

    return;
}
#endif
#endif
/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_stp_setBpduCtrl
 * PURPOSE:
 *      Set BPDU management forward control.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setBpduCtrl(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    AIR_ERROR_NO_T              air_rc = AIR_E_OK;
    UI8_T                      *ptr_data;
    UI32_T                      unit = 0;
    AIR_SWC_MGMT_FRAME_CFG_T    frame_cfg;

    MW_CHECK_PTR(ptr_api_arg);
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    MW_CHECK_PTR(ptr_data);

    memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));

    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d, *ptr_data=%d",
                ptr_api_arg->method, ptr_api_arg->data_size, *ptr_data);

    if (M_UPDATE == ptr_api_arg->method || M_GET == ptr_api_arg->method)
    {
        /* convert ovs STP state number to AIR STP state number */
        frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_BPDU;
        switch(*ptr_data)
        {
            case 0: /* FALSE */
                frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
                frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
                break;
            case 1: /* TRUE */
                frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
                frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
                break;
            default:
                rc = MW_E_NOT_SUPPORT;
        }

        if (MW_E_OK == rc)
        {
            air_rc = air_swc_setMgmtFrameCfg(unit, &frame_cfg);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d)", air_rc);
                rc = MW_E_OP_INCOMPLETE;
            }
        }
    }

    return rc;
}
#ifdef AIR_SUPPORT_LLDPD
/* FUNCTION NAME: syncd_api_stp_setLldpEnable
 * PURPOSE:
 *      Enable/Disable lldp acl.
 *
 * INPUT:
 *      state -- lldp enable/disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setLldpEnable(
    UI8_T state)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if (MW_E_OK == mw_acl_mutex_take())
    {
        if (TRUE == state)
        {
            _syncd_api_stp_delLldpAllAcl();
        }
        else
        {
            ret = _syncd_api_stp_addLldpAcl();
            if (MW_E_OK != ret)
            {
                _syncd_api_stp_delLldpAllAcl();
            }
        }
        _lldp_state = state;
        mw_acl_mutex_release();
    }

#ifdef AIR_EN_CORAL
    _syncd_api_stp_setAllPortMgmtFrame(state);
#endif
    return ret;
}
#endif

#ifdef AIR_SUPPORT_MSTP
static inline MW_ERROR_NO_T
_syncd_api_stp_setPortStateByFid(
    const UI32_T port,
    const UI32_T fid)
{
    AIR_ERROR_NO_T  air_rc;
    UI32_T          i = 0, unit = 0;

    for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
    {
        if (0 != _ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[i])
        {
            break;
        }
    }

    if (i >= AIR_STP_STATE_LAST)
    {
        return MW_E_OP_INVALID;
    }

    air_rc = air_stp_setPortState(unit, port, fid, i);
    if (AIR_E_OK != air_rc)
    {
        SYNCD_LOG_ERROR("air_stp_setPortState() failed(%d) port:%d fid:%d state:%d.", air_rc, port, fid, i);
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_syncd_api_stp_setInstancePortState(
    const UI32_T port,
    const UI32_T fid,
    AIR_STP_STATE_T state)
{
    MW_ERROR_NO_T   ret = MW_E_OK;
    UI32_T          i = 0, unit = 0;
    AIR_ERROR_NO_T  air_rc;

    MW_CHECK_PTR(_port_state_bmp);

    if (0 == fid)
    {
        for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
        {
            BIT_DEL(_port_state_bmp[port-1].user_bmp[i], SYNCD_API_STP_USER_RSTP);
        }
        BIT_SET(_port_state_bmp[port-1].user_bmp[state], SYNCD_API_STP_USER_RSTP);

        if (BIT_CHK(_port_block_bmp, (port-1)))
        {
            SYNCD_LOG_DEBUG("Port:%d is in block state.", port);
            return ret;
        }

        for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
        {
            if (0 != _ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[i])
            {
                break;
            }
        }

        if (i >= AIR_STP_STATE_LAST)
        {
            return MW_E_OP_INCOMPLETE;
        }

        air_rc = air_stp_setPortState(unit, port, fid, i);
        if (AIR_E_OK != air_rc)
        {
            SYNCD_LOG_ERROR("air_stp_setPortState() failed(%d) port:%d fid:%d state:%d.", air_rc, port, fid, i);
            ret = MW_E_OP_INCOMPLETE;
        }

        for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
        {
            BIT_DEL(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[i], SYNCD_API_STP_USER_RSTP);
        }
        BIT_SET(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[state], SYNCD_API_STP_USER_RSTP);
    }
    else
    {
        for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
        {
            BIT_DEL(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[i], SYNCD_API_STP_USER_RSTP);
        }
        BIT_SET(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[state], SYNCD_API_STP_USER_RSTP);
        ret = _syncd_api_stp_setPortStateByFid(port, fid);
    }

    return ret;
}

#endif

/* FUNCTION NAME: syncd_api_stp_setPortStateByUser
 * PURPOSE:
 *      Block specific ports by user.
 *
 * INPUT:
 *      port --     port id
 *      state --    stp state
 *      user --     caller
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setPortStateByUser(
    const UI32_T port,
    AIR_STP_STATE_T state,
    SYNCD_API_STP_USER_T user)
{
    MW_ERROR_NO_T   ret = MW_E_OK;
    AIR_ERROR_NO_T  air_rc;
    UI32_T          i = 0, unit = 0, fid = 0;

    MW_CHECK_PTR(_port_state_bmp);
    MW_CHECK_MIN_MAX_RANGE(port, 0, PLAT_MAX_PORT_NUM);
    MW_CHECK_MIN_MAX_RANGE(state, AIR_STP_STATE_DISABLE, (AIR_STP_STATE_LAST-1));
    MW_CHECK_MIN_MAX_RANGE(user, SYNCD_API_STP_USER_RSTP, (SYNCD_API_STP_USER_LAST-1));

    SYNCD_LOG_DEBUG("Set port:%d state:%d by user %d.", port, i, user);
    for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
    {
        BIT_DEL(_port_state_bmp[port-1].user_bmp[i], user);
    }
    BIT_SET(_port_state_bmp[port-1].user_bmp[state], user);

    if (BIT_CHK(_port_block_bmp, (port-1)))
    {
        SYNCD_LOG_DEBUG("Port:%d is in block state.", port);
        return ret;
    }

#ifdef AIR_SUPPORT_MSTP
    for (fid = 0; fid < SYNCD_API_STP_FID_MAX; fid++)
    {
        for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
        {
            BIT_DEL(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[i], user);
        }
        BIT_SET(_ptr_port_info_state_bmp[port-1].portInfo[fid].user_bmp[state], user);

        _syncd_api_stp_setPortStateByFid(port, fid);
    }
#endif

    fid = 0;
    for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
    {
        if (0 != _port_state_bmp[port-1].user_bmp[i])
        {
            break;
        }
    }

    if (i >= AIR_STP_STATE_LAST)
    {
        SYNCD_LOG_ERROR("Port %d set state %d failed by user %d.", port, state, user);
        return MW_E_OTHERS;
    }

    air_rc = air_stp_setPortState(unit, port, fid, i);
    if (AIR_E_OK != air_rc)
    {
        SYNCD_LOG_ERROR("air_stp_setPortState() failed(%d) port:%d fid:%d state:%d.", air_rc, port, fid, i);
        ret = MW_E_OP_INCOMPLETE;
    }
    SYNCD_LOG_DEBUG("air_stp_setPortState() port:%d state:%d.", port, i);

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#ifdef AIR_SUPPORT_LLDPD
#ifdef AIR_EN_CORAL
    _syncd_api_stp_updatePortMgmtFrame(port, i);
#endif
    _syncd_api_stp_updateAclPort(port, i);
#endif
#endif

    return ret;
}

/* FUNCTION NAME: syncd_api_stp_getPortStateByUser
 * PURPOSE:
 *      Get specific ports block state by user.
 *
 * INPUT:
 *      port --     port id
 *      state --    stp state
 *      user --     caller
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_getPortStateByUser(
    const UI32_T port,
    SYNCD_API_STP_USER_T user,
    AIR_STP_STATE_T *state)
{
    MW_ERROR_NO_T   ret = MW_E_ENTRY_NOT_FOUND;
    UI32_T          i = 0;

    MW_CHECK_PTR(_port_state_bmp);
    MW_CHECK_MIN_MAX_RANGE(user, SYNCD_API_STP_USER_RSTP, (SYNCD_API_STP_USER_LAST-1));
    MW_CHECK_MIN_MAX_RANGE(port, 0, PLAT_MAX_PORT_NUM);
    MW_CHECK_PTR(state);

    *state = AIR_STP_STATE_LAST;
    for (i = AIR_STP_STATE_DISABLE; i < AIR_STP_STATE_LAST; i++)
    {
        if (BIT_CHK(_port_state_bmp[port-1].user_bmp[i], user))
        {
            *state = i;
            ret = MW_E_OK;
            break;
        }
    }

    SYNCD_LOG_DEBUG("Get port %d state %d by user %d.", port, *state, user);
    return ret;
}

/* FUNCTION NAME: syncd_api_stp_setPortState
 * PURPOSE:
 *      Set STP state of specific ports.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setPortState(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI8_T           *ptr_data;
    UI32_T          port_id = 0;
    UI32_T          port = 0;
    AIR_STP_STATE_T state = AIR_STP_STATE_LAST;

    MW_CHECK_PTR(ptr_api_arg);
    ptr_data = ptr_api_arg->ptr_data;
    MW_CHECK_PTR(ptr_data);

    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d, *ptr_data=%d",
                ptr_api_arg->method, ptr_api_arg->data_size, *ptr_data);

    if (M_UPDATE == ptr_api_arg->method)
    {
        port = ptr_api_arg->ptr_type->e_idx;
        for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; port_id++)
        {
            /* convert ovs STP state number to AIR STP state number */
            switch(ptr_data[port_id-1])
            {
                case 0: /* RSTP_DISABLED */
                    state = AIR_STP_STATE_DISABLE;
                    break;
                case 1: /* RSTP_LEARNING */
                    state = AIR_STP_STATE_LEARN;
                    break;
                case 2: /* RSTP_FORWARDING */
                    state = AIR_STP_STATE_FORWARD;
                    break;
                case 3: /* RSTP_DISCARDING */
                    state = AIR_STP_STATE_LISTEN;
                    break;
                default:
                    return MW_E_NOT_SUPPORT;
            }
            if ((DB_ALL_ENTRIES != port) && (port != port_id))
            {
                continue;
            }

#ifdef AIR_SUPPORT_MSTP
            rc = _syncd_api_stp_setInstancePortState(port_id, 0, state);
#else
            rc = syncd_api_stp_setPortStateByUser(port_id, state, SYNCD_API_STP_USER_RSTP);
#endif
            if (MW_E_OK != rc)
            {
                rc = MW_E_OP_INCOMPLETE;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: syncd_api_stp_blockPort
 * PURPOSE:
 *      Block specific ports.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_blockPort(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    AIR_ERROR_NO_T  air_rc = AIR_E_OK;
    UI8_T           *ptr_data;
    UI32_T          port = 0, port_id;
    AIR_STP_STATE_T state = AIR_STP_STATE_LAST;
    const UI32_T    unit = 0;
    UI32_T          fid = 0;

    MW_CHECK_PTR(ptr_api_arg);
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    MW_CHECK_PTR(ptr_data);

    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d, *ptr_data=%d",
                ptr_api_arg->method, ptr_api_arg->data_size, *ptr_data);

    if (M_UPDATE == ptr_api_arg->method)
    {
        if (FALSE == *ptr_data)
        {
            /* Disable block, get current STP state of the port */
            port = ptr_api_arg->ptr_type->e_idx;
            for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; port_id++)
            {
                if ((DB_ALL_ENTRIES != port) && (port != port_id))
                {
                    continue;
                }

                /* convert ovs STP state number to AIR STP state number */
                syncd_api_stp_getPortStateByUser(port_id, SYNCD_API_STP_USER_RSTP, &state);
                SYNCD_LOG_DEBUG("%s %u port=%u oper_state=%u, chip_state=%u\n", __func__, __LINE__,
                            port, *ptr_data, state);

                BIT_DEL(_port_block_bmp, (port_id-1));
                if (MW_E_OK != syncd_api_stp_setPortStateByUser(port_id, state, SYNCD_API_STP_USER_RSTP))
                {
                    SYNCD_LOG_ERROR("air_stp_setPortState() failed(%d) port:%d fid:%d state:%d", air_rc, port_id, fid, state);
                    rc = MW_E_OP_INCOMPLETE;
                }
            }
        }
        else if (TRUE == *ptr_data)
        {
            port = ptr_api_arg->ptr_type->e_idx;
            for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; port_id++)
            {
                if ((DB_ALL_ENTRIES != port) && (port != port_id))
                {
                    continue;
                }
                SYNCD_LOG_DEBUG("%s %u port=%u oper_state=%u\n", __func__, __LINE__, port, *ptr_data);
                air_rc = air_stp_setPortState(unit, port_id, fid, AIR_STP_STATE_LISTEN);
                if (AIR_E_OK != air_rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d)", air_rc);
                    rc = MW_E_OP_INCOMPLETE;
                }
                BIT_SET(_port_block_bmp, (port_id-1));
#ifdef AIR_SUPPORT_LLDPD
                _syncd_api_stp_updateAclPort(port_id, AIR_STP_STATE_LISTEN);
#ifdef AIR_EN_CORAL
                _syncd_api_stp_updatePortMgmtFrame(port_id, AIR_STP_STATE_LISTEN);
#endif
#endif
            }
        }
        else
        {
            SYNCD_LOG_DEBUG("Unknown data(%u)", *ptr_data);
        }
    }
    else
    {
        SYNCD_LOG_DEBUG("Not support method(%u)", ptr_api_arg->method);
    }

    return rc;
}

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: syncd_api_stp_setInstancePortState
 * PURPOSE:
 *      Set STP state of specific ports with fid.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setInstancePortState(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI32_T          port_id = 0;
    UI32_T          port = 0;
    UI32_T          fid = 0, idx = 0;
    AIR_STP_STATE_T state = AIR_STP_STATE_LAST;
    UI8_T           stp_state[SYNCD_API_STP_FID_MAX];
    UI32_T          offset;

    MW_CHECK_PTR(ptr_api_arg);
    MW_CHECK_PTR(ptr_api_arg->ptr_data);

    SYNCD_LOG_DEBUG("ptr_api_arg->method=%d, data_size=%d",
                ptr_api_arg->method, ptr_api_arg->data_size);

    if (DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx)
    {
        offset = ptr_api_arg->data_size / PLAT_MAX_PORT_NUM / sizeof(UI8_T);
    }
    else
    {
        offset = ptr_api_arg->data_size / sizeof(UI8_T);
    }

    if (M_UPDATE == ptr_api_arg->method)
    {
        port = ptr_api_arg->ptr_type->e_idx;
        for (port_id = 1; port_id <= PLAT_MAX_PORT_NUM; port_id++)
        {
            if ((DB_ALL_ENTRIES != port) && (port != port_id))
            {
                continue;
            }

            osapi_memcpy(stp_state, (((UI8_T *)ptr_api_arg->ptr_data) + (idx * offset)), offset);
            for (fid = 0; fid < offset; fid++)
            {
                /* convert ovs STP state number to AIR STP state number */
                switch(stp_state[fid])
                {
                    case 0: /* RSTP_DISABLED */
                        state = AIR_STP_STATE_FORWARD;
                        break;
                    case 1: /* RSTP_LEARNING */
                        state = AIR_STP_STATE_LEARN;
                        break;
                    case 2: /* RSTP_FORWARDING */
                        state = AIR_STP_STATE_FORWARD;
                        break;
                    case 3: /* RSTP_DISCARDING */
                        state = AIR_STP_STATE_LISTEN;
                        break;
                    default:
                        return MW_E_NOT_SUPPORT;
                }

                if (MW_E_OK != _syncd_api_stp_setInstancePortState(port_id, fid, state))
                {
                    rc = MW_E_OP_INCOMPLETE;
                }
            }
            idx++;
        }
    }

    return rc;
}
#endif

/* FUNCTION NAME: syncd_api_stp_init
 * PURPOSE:
 *      Initialization stp information
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
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_init()
{
    UI32_T              len, i;
#ifdef AIR_SUPPORT_MSTP
    UI32_T j;
#endif
    MW_ERROR_NO_T       ret;

    len = sizeof(SYNCD_API_STP_PORT_INFO_T) * PLAT_MAX_PORT_NUM;
    ret = osapi_calloc(len, "SyncD", (void **)&_port_state_bmp);
    if (MW_E_OK != ret)
    {
        return ret;
    }
#ifdef AIR_SUPPORT_MSTP
    len = sizeof(SYNCD_API_STP_INS_PORT_INFO_T) * PLAT_MAX_PORT_NUM;
    ret = osapi_calloc(len, "SyncD", (void **)&_ptr_port_info_state_bmp);
    if (MW_E_OK != ret)
    {
        MW_FREE(_port_state_bmp);
        return ret;
    }
#endif

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        /* set all ports forward */
        BIT_SET(_port_state_bmp[i].user_bmp[AIR_STP_STATE_FORWARD], SYNCD_API_STP_USER_RSTP);
#ifdef AIR_SUPPORT_MSTP
        for (j = 0; j < SYNCD_API_STP_FID_MAX; j++)
        {
            BIT_SET(_ptr_port_info_state_bmp[i].portInfo[j].user_bmp[AIR_STP_STATE_FORWARD], SYNCD_API_STP_USER_RSTP);
        }
#endif
    }

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#ifdef AIR_SUPPORT_LLDPD
    for (i = 0; i < SYNCD_API_STP_ACL_LAST; i++)
    {
        _lldp_acl_id[i] = MW_ACL_ID_INVALID;
    }
    _syncd_api_stp_addLldpAcl();
#endif
#endif

    return MW_E_OK;
}
