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

/* FILE NAME:  syncd_api_swc.c
 * PURPOSE:
 *  Implement switch API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <syncd_api_swc.h>
#include <osapi_thread.h>
#include <sys_mgmt.h>
#include "air_swc.h"
#include "air_stp.h"
#include "syncd_api_stp.h"
#include "mw_acl.h"

/* NAMING CONSTANT DECLARATIONS
*/
#ifdef AIR_SUPPORT_CABLE_DIAG
#define MW_CABLE_DIAGNOSTIC_TASK_NAME   "cdd"
#define MW_CABLE_DIAGNOSTIC_TASK_SIZE   (1024)
#define MW_CABLE_DIAGNOSTIC_TASK_PRI    (2)
#define MW_CABLE_DIAGNOSTIC_NO_SUPPORT  (98)
#define MW_CABLE_DIAGNOSTIC_TIMEOUT     (99)
#endif
/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
#ifdef AIR_SUPPORT_LLDPD
typedef struct
{
#define LLDP_LIMIT_PKT_THLD    (60)
#define LLDP_LIMIT_SPAN        (1300)
#define LLDP_LIMIT_BLK_TIME    (1)
#define LLDP_LIMIT_TICK        (AIR_DOS_RATE_TICKSEL_1MS)
    UI8_T lldp_global_enable;
    UI16_T lldp_attack_id;
    UI16_T lldp_acl_id_03;
    UI16_T lldp_acl_id_0e;
} __attribute__((packed)) LLDP_CTRL_T;

LLDP_CTRL_T lldp_ctrl = {0, MW_ATTACK_ID_INVALID, MW_ACL_ID_INVALID, MW_ACL_ID_INVALID};
#ifdef AIR_EN_CORAL
#define LLDP_PORT_DISABLE          (0)
#define LLDP_PORT_TX_ONLY          (1)
#define LLDP_PORT_RX_ONLY          (2)
#define LLDP_PORT_TX_RX            (3)
#endif
#endif

#ifdef AIR_SUPPORT_CABLE_DIAG
static threadhandle_t _ptr_cd = NULL;
#ifdef AIR_SUPPORT_MQTTD
/* Cable Diag */
volatile BOOL_T cable_diaging = FALSE; /* To pause the timer for saving mem */
#endif
#endif

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
#ifdef AIR_SUPPORT_CABLE_DIAG
static void
_caProcess(
    void *ptr_pvParameters);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_CABLE_DIAG
static void
_caProcess(
    void *ptr_pvParameters)
{
    AIR_PORT_CABLE_TEST_RSLT_T cable;
    DB_PORT_DIAG_T syncd_info;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI8_T i = 0;
    UI32_T ca_port = 0;

    ca_port = (UI32_T)ptr_pvParameters;
    memset(&syncd_info, 0, sizeof(DB_PORT_DIAG_T));
    rc = air_port_triggerCableTest(0, ca_port, AIR_PORT_CABLE_TEST_PAIR_ALL, &cable);
    if(AIR_E_OK == rc)
    {
        for(i = 0; i < AIR_PORT_CABLE_MAX_PAIR; i++)
        {
            syncd_info.state[i] = cable.status[i];
            syncd_info.length[i] = cable.length[i];
            syncd_info.port[i] = ca_port;
            SYNCD_LOG_DEBUG("state_%u-%u", i, syncd_info.state[i]);
            SYNCD_LOG_DEBUG("length_%u-%u", i, syncd_info.length[i]);
        }
    }
    else if(AIR_E_NOT_SUPPORT == rc)
    {
        syncd_info.set[0] = MW_CABLE_DIAGNOSTIC_NO_SUPPORT;
    }
    else
    {
        syncd_info.set[0] = MW_CABLE_DIAGNOSTIC_TIMEOUT;
    }
    /* Request DB for update cable data of specific port */
    mw_rc = syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                M_UPDATE,
                                PORT_DIAG,
                                0,
                                0,
                                (void*)&syncd_info,
                                MSG_TIMEOUT_WAIT_INDEFINITELY);
    if(MW_E_OK != mw_rc)
    {
        SYNCD_LOG_ERROR("Update cable test results error!");
    }
#ifdef AIR_SUPPORT_MQTTD
    cable_diaging = FALSE;
#endif
    /* clear _ptr_cd */
    if (NULL != _ptr_cd)
    {
        _ptr_cd = NULL;
    }
    vTaskDelete(NULL);

    return;
}

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_cableDiagnostic
 * PURPOSE:
 *      process cable diagnostic
 *
 * INPUT:
 *      ptr_api_arg         --  pointer to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_cableDiagnostic(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI8_T *ptr_data = NULL;
    UI32_T ca_port = 0;

    MW_CHECK_PTR(ptr_api_arg);
    /* add check _ptr_cd */
    if (_ptr_cd != NULL)
    {
        return MW_E_ALREADY_INITED;
    }

    ptr_data = (UI8_T *)ptr_api_arg ->ptr_data;
    SYNCD_LOG_DEBUG("entry index-%u", ptr_api_arg ->ptr_type ->e_idx);
    SYNCD_LOG_DEBUG("f_idx-%u", ptr_api_arg ->ptr_type ->f_idx);
    SYNCD_LOG_DEBUG("Port-%u", *ptr_data);
#if(defined(AIR_SUPPORT_SFP))
    if ((DB_ALL_ENTRIES == ptr_api_arg ->ptr_type ->e_idx) && ((UI8_T)MW_CABLE_DIAGNOSTIC_NO_SUPPORT == *ptr_data) )
    {
        return mw_rc;
    }
#endif
    if( (0 != *ptr_data))
    {
#ifdef AIR_SUPPORT_MQTTD
        cable_diaging = TRUE;
#endif
        ca_port = *ptr_data;
        osapi_processCreate(MW_CABLE_DIAGNOSTIC_TASK_NAME, MW_CABLE_DIAGNOSTIC_TASK_SIZE, MW_CABLE_DIAGNOSTIC_TASK_PRI, _caProcess, (void*)ca_port, &_ptr_cd);
    }

    return mw_rc;
}
#endif

#ifdef AIR_SUPPORT_LLDPD
MW_ERROR_NO_T
syncd_api_clear_lldp_ratelimit(
    void)
{
    MW_ERROR_NO_T               mw_rc = MW_E_OK;
    UI32_T                      unit = 0;

    air_acl_delAction(unit, (UI32_T)lldp_ctrl.lldp_acl_id_03);
    air_acl_delAction(unit, (UI32_T)lldp_ctrl.lldp_acl_id_0e);
    air_acl_delRule(unit, (UI32_T)lldp_ctrl.lldp_acl_id_03);
    air_acl_delRule(unit, (UI32_T)lldp_ctrl.lldp_acl_id_0e);
    air_dos_clearRateLimitCfg(unit, (UI32_T)lldp_ctrl.lldp_attack_id);
    lldp_ctrl.lldp_acl_id_03 = MW_ACL_ID_INVALID;
    lldp_ctrl.lldp_acl_id_0e = MW_ACL_ID_INVALID;
    lldp_ctrl.lldp_attack_id = MW_ATTACK_ID_INVALID;
    return mw_rc;
}

MW_ERROR_NO_T
syncd_api_set_lldp_dos_ratelimit(
    UI32_T                     attack_id,
    AIR_DOS_RATE_LIMIT_CFG_T * ptr_dos_cfg)
{
    UI32_T                      unit = 0;
    AIR_ERROR_NO_T              air_rc = AIR_E_OK;
    MW_ERROR_NO_T               mw_rc = MW_E_OK;

    ptr_dos_cfg->pkt_thld = LLDP_LIMIT_PKT_THLD;
    ptr_dos_cfg->time_span = LLDP_LIMIT_SPAN;
    ptr_dos_cfg->block_time = LLDP_LIMIT_BLK_TIME;
    ptr_dos_cfg->tick_sel = LLDP_LIMIT_TICK;
    air_rc = air_dos_setRateLimitCfg(unit, attack_id, ptr_dos_cfg);
    if (AIR_E_OK == air_rc)
    {
        /* keep attack ID */
        lldp_ctrl.lldp_attack_id = (UI16_T)attack_id;
    }
    else
    {
        mw_rc = MW_E_OP_INCOMPLETE;
        SYNCD_LOG_DEBUG("set dos cfg fail when set lldp dos ratelimit ");
    }
    return mw_rc;
}

MW_ERROR_NO_T
syncd_api_set_lldp_acl_rule(
    UI32_T             acl_rule_id,
    AIR_ACL_RULE_T  * ptr_acl_rule,
    UI8_T            rule_index)
{
    MW_ERROR_NO_T    mw_rc = MW_E_OK;
    AIR_ERROR_NO_T   air_rc = AIR_E_OK;
    AIR_ACL_ACTION_T acl_action;
    UI32_T           unit = 0;
    UI8_T            lldp_dmac[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};

    osapi_memset(&acl_action, 0, sizeof(AIR_ACL_ACTION_T));
    if(1 == rule_index)
    {
        lldp_dmac[5] = 0x03;
    }
    ptr_acl_rule->rule_en = TRUE;
    AIR_PORT_BITMAP_COPY(ptr_acl_rule->portmap, PLAT_PORT_BMP_TOTAL);
    AIR_PORT_DEL(ptr_acl_rule->portmap, PLAT_CPU_PORT);
    ptr_acl_rule->end = TRUE;
    ptr_acl_rule->key.etype = ETHTYPE_LLDP;
    ptr_acl_rule->mask.etype = 0x3;
    osapi_memcpy(ptr_acl_rule->key.dmac, lldp_dmac, sizeof(lldp_dmac));
    ptr_acl_rule->mask.dmac = 0x3f;
    ptr_acl_rule->field_valid |= ((1U << AIR_ACL_ETYPE_KEY) | (1U << AIR_ACL_DMAC_KEY));
    air_rc = air_acl_setRule(unit, acl_rule_id, ptr_acl_rule);
    if (AIR_E_OK == air_rc)
    {
        if(1 == rule_index)
        {
            lldp_ctrl.lldp_acl_id_03 = (UI16_T)acl_rule_id;
        }
        else
        {
            lldp_ctrl.lldp_acl_id_0e = (UI16_T)acl_rule_id;
        }
        acl_action.attack_rate_id = (UI8_T)lldp_ctrl.lldp_attack_id;
        acl_action.field_valid = (1U << AIR_ACL_ATTACK);
        air_rc = air_acl_setAction(unit, acl_rule_id, &acl_action);
        if (AIR_E_OK != air_rc)
        {
            mw_rc = MW_E_OP_INCOMPLETE;
            SYNCD_LOG_DEBUG("set acl action[%d] %d fail , air_rc=%d.\n", rule_index, acl_rule_id, air_rc);
        }
    }
    else
    {
        mw_rc = MW_E_OP_INCOMPLETE;
        SYNCD_LOG_DEBUG("set acl rule[%d] %d fail , air_rc=%d.\n", rule_index, acl_rule_id, air_rc);
    }
    return mw_rc;
}

MW_ERROR_NO_T
syncd_api_set_lldp_ratelimit(
    UI8_T mode)
{
    MW_ERROR_NO_T               mw_rc = MW_E_OK;
    AIR_ACL_RULE_T              acl_03_rule;
    AIR_ACL_RULE_T              acl_0e_rule;
    AIR_DOS_RATE_LIMIT_CFG_T    dos_rate_limit;
    UI32_T                      unit = 0, acl_03_id = 0, acl_0e_id = 0, attack_id = 0;

    osapi_memset(&acl_03_rule, 0, sizeof(AIR_ACL_RULE_T));
    osapi_memset(&acl_0e_rule, 0, sizeof(AIR_ACL_RULE_T));
    osapi_memset(&dos_rate_limit, 0, sizeof(AIR_DOS_RATE_LIMIT_CFG_T));
    if(MW_E_OK == mw_acl_mutex_take())
    {
        if(TRUE == mode)
        {
            if((MW_E_OK == MW_ATTACK_ID_GET_AVAILABLERULE(&attack_id, &dos_rate_limit, unit)) &&
                (MW_ATTACK_ID_INVALID != attack_id))
            {
                mw_rc = syncd_api_set_lldp_dos_ratelimit(attack_id, &dos_rate_limit);
                if(MW_E_OK == mw_rc)
                {
                    if((MW_E_OK == MW_ACL_GET_AVAILABLERULE(&acl_03_id, &acl_03_rule, unit)) &&
                        (MW_ACL_ID_INVALID != acl_03_id))
                    {
                        mw_rc = syncd_api_set_lldp_acl_rule(acl_03_id, &acl_03_rule, 1);
                        if(MW_E_OK != mw_rc)
                        {
                            SYNCD_LOG_DEBUG("set lldp acl rule 03 fail when set lldp ratelimit ");
                        }
                    }
                    else
                    {
                        SYNCD_LOG_DEBUG("get acl_03_id fail when set lldp ratelimit ");
                    }
                    if((MW_E_OK == MW_ACL_GET_AVAILABLERULE(&acl_0e_id, &acl_0e_rule, unit)) &&
                        (MW_ACL_ID_INVALID != acl_0e_id))
                    {
                        mw_rc = syncd_api_set_lldp_acl_rule(acl_0e_id, &acl_0e_rule, 2);
                        if(MW_E_OK != mw_rc)
                        {
                            SYNCD_LOG_DEBUG("set lldp acl rule 0e fail when set lldp ratelimit ");
                        }
                    }
                    else
                    {
                        SYNCD_LOG_DEBUG("get acl_0e_id fail when set lldp ratelimit ");
                    }
                }
                else
                {
                    SYNCD_LOG_DEBUG("set lldp dos ratelimit fail when set lldp ratelimit ");
                }
            }
            else
            {
                SYNCD_LOG_DEBUG("get attack_id fail when set lldp ratelimit ");
            }
        }
        else
        {
            syncd_api_clear_lldp_ratelimit();
        }
        mw_acl_mutex_release();
    }
    else
    {
        mw_rc = MW_E_OP_INCOMPLETE;
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_set_lldp_fwd_to_cpu
 * PURPOSE:
 *      set lldp packet to mgmt & high priority & froward to cpu
 *
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_set_lldp_fwd_to_cpu(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    UI8_T tmp_data = 0;
    UI16_T port = 0;
#ifndef AIR_EN_CORAL
    UI8_T block_flag = FALSE;
    AIR_STP_STATE_T stp_cfg = {0};
    AIR_SWC_MGMT_FRAME_CFG_T frame03_cfg = {0};
    AIR_SWC_MGMT_FRAME_CFG_T frame0E_cfg = {0};
#else
    AIR_SWC_MGMT_FRAME_CFG_T port_mgmt_cfg = {0};
#endif
    AIR_ERROR_NO_T rc = AIR_E_OK;

    MW_CHECK_PTR(ptr_api_arg);
    tmp_data = *(UI8_T *)ptr_api_arg ->ptr_data;
    if(lldp_ctrl.lldp_global_enable == tmp_data)
    {
        return rc;
    }
    lldp_ctrl.lldp_global_enable = tmp_data;
    syncd_api_set_lldp_ratelimit(tmp_data);
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    syncd_api_stp_setLldpEnable(tmp_data);
#endif
#ifndef AIR_EN_CORAL
    frame03_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_03;
    frame0E_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_0E;
    rc = air_swc_getMgmtFrameCfg(0, &frame03_cfg);
    if(AIR_E_OK != rc)
    {
        SYNCD_LOG_DEBUG("get REV_03 mgmt cfg failed(%d)", rc);
    }
    rc = air_swc_getMgmtFrameCfg(0, &frame0E_cfg);
    if(AIR_E_OK != rc)
    {
        SYNCD_LOG_DEBUG("get REV_0E mgmt cfg failed(%d)", rc);
    }
    if(TRUE == tmp_data)
    {
        /*RGAC2 bit28 & bit12 is set to 1 in SDK(hal_sco_swc_init)*/
        frame03_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        frame03_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        frame03_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;

        frame0E_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        frame0E_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        frame0E_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
    }
    else
    {
        frame03_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        frame03_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        frame03_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;

        frame0E_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        frame0E_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        frame0E_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
    }
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }
        osapi_memset(&stp_cfg, 0, sizeof(AIR_STP_STATE_T));
        rc = air_stp_getPortState(0, port, 0, &stp_cfg);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_DEBUG("get stp port state failed(%d)", rc);
        }
        if((AIR_STP_STATE_LISTEN == stp_cfg) || (AIR_STP_STATE_LEARN == stp_cfg))
        {
            block_flag = TRUE;
            break;
        }
    }
    if(TRUE == block_flag)
    {
        frame03_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        frame0E_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
    }
    rc = air_swc_setMgmtFrameCfg(0, &frame03_cfg);
    if(AIR_E_OK != rc)
    {
        SYNCD_LOG_DEBUG("set REV_03 mgmt frame & pri high & fwd fail, enable %d", tmp_data);
    }

    rc = air_swc_setMgmtFrameCfg(0, &frame0E_cfg);
    if(AIR_E_OK != rc)
    {
        SYNCD_LOG_DEBUG("set REV_0E mgmt frame & pri high & fwd fail, enable %d", tmp_data);
    }
#else
    if(FALSE == tmp_data)
    {

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            osapi_memset(&port_mgmt_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
            port_mgmt_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
            rc = air_swc_getPortMgmtFrameCfg(0, port, &port_mgmt_cfg);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("get port(%d) port_mgmt_cfg failed(%d)", port, rc);
            }

            port_mgmt_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;

            rc = air_swc_setPortMgmtFrameCfg(0, port, &port_mgmt_cfg);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) port_mgmt_cfg failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
#endif
    return rc;
}

#ifdef AIR_EN_CORAL
/* FUNCTION NAME: syncd_api_set_lldp_portMgmtFrameCfg
 * PURPOSE:
 *      set lldp packet to mgmt & high priority & froward to cpu per port on AN8858
 *
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_set_lldp_portMgmtFrameCfg(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI8_T tmp_lldp_admin = 0;
    UI32_T unit = 0;
    UI16_T e_id = 0, port = 0, tmp_port_index = 0;
#ifdef AIR_SUPPORT_RSTP
    AIR_STP_STATE_T stp_cfg = {0};
#endif
    AIR_SWC_MGMT_FRAME_CFG_T port_mgmt_cfg = {0};

    MW_CHECK_PTR(ptr_api_arg);
    e_id = (ptr_api_arg->ptr_type->e_idx);
    SYNCD_LOG_DEBUG("eidx=%u", e_id);

    if (((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) != ptr_api_arg->data_size) &&
        (sizeof(UI8_T) != ptr_api_arg->data_size))
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if (((M_UPDATE == ptr_api_arg->method) && (DB_ALL_ENTRIES == e_id)) ||
        ((M_GET == ptr_api_arg->method) && (DB_ALL_ENTRIES == e_id)))
    {
        /* Handle User port */
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            if (PLAT_CPU_PORT < port)
            {
                tmp_port_index = port - 1;
            }
            osapi_memset(&port_mgmt_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
            port_mgmt_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
            rc = air_swc_getPortMgmtFrameCfg(unit, port, &port_mgmt_cfg);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("get port(%d) port_mgmt_cfg failed(%d)", port, rc);
            }
            tmp_lldp_admin = ((UI8_T *)ptr_api_arg->ptr_data)[tmp_port_index];
            port_mgmt_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
            if (LLDP_PORT_DISABLE == tmp_lldp_admin || LLDP_PORT_TX_ONLY == tmp_lldp_admin)
            {
                port_mgmt_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            }
            else if (LLDP_PORT_TX_RX == tmp_lldp_admin || LLDP_PORT_RX_ONLY == tmp_lldp_admin)
            {
                port_mgmt_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
                port_mgmt_cfg.pri = AIR_SWC_PRI_MAX_NUM;
                port_mgmt_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
            }
#ifdef AIR_SUPPORT_RSTP
            osapi_memset(&stp_cfg, 0, sizeof(AIR_STP_STATE_T));
            syncd_api_stp_getPortStateByUser(port, SYNCD_API_STP_USER_LACP, &stp_cfg);
            if (AIR_STP_STATE_LISTEN != stp_cfg)
            {
                osapi_memset(&stp_cfg, 0, sizeof(AIR_STP_STATE_T));
                syncd_api_stp_getPortStateByUser(port, SYNCD_API_STP_USER_RSTP, &stp_cfg);
                if ((AIR_STP_STATE_LISTEN == stp_cfg) || (AIR_STP_STATE_LEARN == stp_cfg))
                {
                    port_mgmt_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
                }
            }
#endif
            rc = air_swc_setPortMgmtFrameCfg(unit, port, &port_mgmt_cfg);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) port_mgmt_cfg failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("method(%d) e_id(%d) error", ptr_api_arg->method, e_id);
        return MW_E_BAD_PARAMETER;
    }
    return rc;
}
#endif
#endif
/* FUNCTION NAME: syncd_api_system
 * PURPOSE:
 *      process system reset status
 *
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_system(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T *ptr_data = NULL;
    DB_SYSTEM_T *sys_ctrl = NULL;

    MW_CHECK_PTR(ptr_api_arg);

    ptr_data = (UI8_T *)ptr_api_arg ->ptr_data;
    sys_ctrl = (DB_SYSTEM_T*)ptr_data;

    SYNCD_LOG_DEBUG("Syste reset [%d:%d:%d]\n", sys_ctrl->save_running, sys_ctrl->reset, sys_ctrl->reset_factory);

    if(TRUE == sys_ctrl->reset)
    {
        //TODO: Call system reset
    }

    return rc;

}

