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


/* FILE NAME:  mw_acl.c
 * PURPOSE:
 * It implements the API for MW ACL.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_acl.h"
#include "mw_acl_settings.h"
#include "switch.h"
#include "osapi_mutex.h"
#include "mw_log.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define MW_ACL_MUTEX_NAME "MW_ACL_MUTEX"
#define MW_ACL_WAITTIME   (0xFFFFFFFF)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static semaphorehandle_t _mw_acl_mutex;   /* The mutext to protect acl thread safety */

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_mw_acl_enable(
    UI32_T unit,
    BOOL_T enable)
{
    UI32_T port = 0;
    I32_T rc = MW_E_NOT_INITED;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        rc = air_acl_setPortState(unit, port, enable);
        if (AIR_E_OK != rc)
        {
            return MW_E_OP_INCOMPLETE;
        }
    }

    rc = air_acl_setGlobalState(unit, enable);

    return (AIR_E_OK == rc) ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static MW_ERROR_NO_T
_mw_acl_mutex_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    if (NULL != _mw_acl_mutex)
    {
        return MW_E_ALREADY_INITED;
    }
    ret = osapi_mutexCreate(MW_ACL_MUTEX_NAME, &_mw_acl_mutex);
    if (MW_E_OK != ret)
    {
        _mw_acl_mutex = NULL;
    }
    return ret;
}

static MW_ERROR_NO_T
_mw_acl_mutex_deinit(
    void)
{
    if (NULL != _mw_acl_mutex)
    {
        osapi_mutexDelete(_mw_acl_mutex);
        _mw_acl_mutex = NULL;
    }
    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
mw_acl_mutex_take(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if(NULL != _mw_acl_mutex)
    {
        ret = osapi_mutexTake(_mw_acl_mutex, MW_ACL_WAITTIME);
    }

    return ret;
}

MW_ERROR_NO_T
mw_acl_mutex_release(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    if (NULL != _mw_acl_mutex )
    {
        ret = osapi_mutexGive(_mw_acl_mutex);
    }

    return ret;
}

MW_ERROR_NO_T
mw_acl_getAvailableRule(
    UI32_T *ptr_entryId,
    AIR_ACL_RULE_T *ptr_rule,
    const UI32_T unit,
    const UI32_T entry_id_start)
{
    UI32_T rule_id = entry_id_start;

    *ptr_entryId = MW_ACL_ID_INVALID;
    for (; rule_id <= MW_ACL_ID_DYNAMIC_MAX; rule_id++)
    {
        if((AIR_E_OK == air_acl_getRule(unit, rule_id, ptr_rule)) &&
           (FALSE == ptr_rule->rule_en))
        {
            *ptr_entryId = rule_id;
            return MW_E_OK;
        }
    }

    ptr_rule = NULL;
    return MW_E_ENTRY_NOT_FOUND;
}

MW_ERROR_NO_T
mw_acl_getAvailableRule_r(
    UI32_T *ptr_entryId,
    AIR_ACL_RULE_T *ptr_rule,
    const UI32_T unit,
    const UI32_T entry_id_start)
{
    UI32_T rule_id = entry_id_start;

    *ptr_entryId = MW_ACL_ID_INVALID;
    for (; rule_id >= MW_ACL_ID_DYNAMIC_MIN; rule_id--)
    {
        if ((AIR_E_OK == air_acl_getRule(unit, rule_id, ptr_rule)) &&
            (FALSE == ptr_rule->rule_en))
        {
            *ptr_entryId = rule_id;
            return MW_E_OK;
        }
    }

    ptr_rule = NULL;
    return MW_E_ENTRY_NOT_FOUND;
}

MW_ERROR_NO_T
mw_acl_getAvailableAttackID(
    UI32_T                      *ptr_id,
    AIR_DOS_RATE_LIMIT_CFG_T    *ptr_cfg,
    const UI32_T                unit,
    const UI32_T                entry_id_start)
{
    UI32_T id = entry_id_start;

    *ptr_id = MW_ATTACK_ID_INVALID;
    for (; id <= MW_ATTACK_ID_DYNAMIC_MAX; id++)
    {
        if((AIR_E_OK == air_dos_getRateLimitCfg(unit, id, ptr_cfg)) &&
            (0 == ptr_cfg->pkt_thld)&&
            (0 == ptr_cfg->time_span)&&
            (0 == ptr_cfg->block_time))
        {
            *ptr_id = id;
            return MW_E_OK;
        }
    }

    ptr_cfg = NULL;
    return MW_E_ENTRY_NOT_FOUND;
}

MW_ERROR_NO_T
mw_acl_init(
    UI32_T unit)
{
    MW_ERROR_NO_T rc = MW_E_NOT_INITED;

    rc = _mw_acl_mutex_init();
    rc |= _mw_acl_enable(unit, TRUE);
    rc |= mw_acl_settings_init(unit);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(SYSTEM, "%s failed, rc=%d.", __func__, rc);
        _mw_acl_mutex_deinit();
        _mw_acl_enable(unit, FALSE);
        mw_acl_settings_deinit(unit);
        return rc;
    }

    return rc;
}

