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

/* FILE NAME:  syncd_api_stormctrl.c
 * PURPOSE:
 *  Implement stormctrl API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <air_sec.h>
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */
#define SYNCD_STORMCTL_UNIT_64        (64)
#define SYNCD_STORMCTL_UNIT_256       (256)
#define SYNCD_STORMCTL_UNIT_1024      (1024)
#define SYNCD_STORMCTL_UNIT_4096      (4096)
#define SYNCD_STORMCTL_UNIT_16384     (16384)
#define SYNCD_STORMCTL_RATE_MAX_CNT   (0xff)

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
*/

/* FUNCTION NAME: syncd_api_stormctrl_en
 * PURPOSE:
 *      Set stormctrl function enable/disable.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stormctrl_en(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    MW_ERROR_NO_T   mw_rc = MW_E_OK;
    UI32_T unit = 0;
    UI32_T port;
    UI8_T storm_cfg[MAX_PORT_NUM];
    UI16_T index;
    UI16_T expect_entry_num;
    AIR_SEC_STORM_TYPE_T type;

    MW_CHECK_PTR(ptr_api_arg);

    index = ptr_api_arg->ptr_type->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG("index=%u, expect_entry_num=%u", index, expect_entry_num);

    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if(DB_ALL_ENTRIES == index)
    {
        memcpy(&storm_cfg, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        switch (ptr_api_arg->ptr_type->f_idx)
        {
            case PORT_STORM_BC_CFG:
                type = AIR_SEC_STORM_TYPE_BC;
                break;
            case PORT_STORM_MC_CFG:
                type = AIR_SEC_STORM_TYPE_UMC;
                break;
            case PORT_STORM_UC_CFG:
                type = AIR_SEC_STORM_TYPE_UUC;
                break;
            default:
                SYNCD_LOG_ERROR("Not support this mode");
                return MW_E_BAD_PARAMETER;
        }
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            /* Not support CPU port */
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            rc = air_sec_setPortStormCtrl(unit, port, type, storm_cfg[port - 1]);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d) port %d storm_cfg %d", rc, port,storm_cfg[port - 1]);
                mw_rc = MW_E_OP_INCOMPLETE;
            }
        }

    }
    else
    {
        memcpy(&storm_cfg[index - 1], (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        switch (ptr_api_arg->ptr_type->f_idx)
        {
            case PORT_STORM_BC_CFG:
                type = AIR_SEC_STORM_TYPE_BC;
                break;
            case PORT_STORM_MC_CFG:
                type = AIR_SEC_STORM_TYPE_UMC;
                break;
            case PORT_STORM_UC_CFG:
                type = AIR_SEC_STORM_TYPE_UUC;
                break;
            default:
                SYNCD_LOG_ERROR("Not support this mode");
                return MW_E_BAD_PARAMETER;
        }
        port = (UI32_T)index;
        rc = air_sec_setPortStormCtrl(unit, port, type, storm_cfg[index - 1]);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Call function failed(%d) port %d storm_cfg %d", rc, port,storm_cfg[index - 1]);
            mw_rc = MW_E_OP_INCOMPLETE;
        }
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_stormctrl_cfg
 * PURPOSE:
 *      Set stormctrl config value.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stormctrl_cfg(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    AIR_SEC_STORM_TYPE_T        type = AIR_SEC_STORM_TYPE_LAST;
    AIR_SEC_STORM_RATE_MODE_T   mode;
    UI32_T  unit = 0;
    UI32_T  port;
    UI8_T   storm_mode[MAX_PORT_NUM];
    UI16_T  index;
    UI16_T  expect_entry_num;
    UI32_T  rate;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg->ptr_type->e_idx;

    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG("ety_idx=%u, expect_entry_num=%u", index, expect_entry_num);
    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    switch(ptr_api_arg->ptr_type->f_idx)
    {
        case PORT_STORM_BC_MODE:
            type = AIR_SEC_STORM_TYPE_BC;
            break;
        case PORT_STORM_MC_MODE:
            type = AIR_SEC_STORM_TYPE_UMC;
            break;
        case PORT_STORM_UC_MODE:
            type = AIR_SEC_STORM_TYPE_UUC;
            break;
        default:
            SYNCD_LOG_ERROR("Not support this mode");
            return MW_E_BAD_PARAMETER;
    }

    if(DB_ALL_ENTRIES == index)
    {
        memcpy(&storm_mode, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            /* Not support CPU port */
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            rc = air_sec_getPortStormCtrlRate(unit, port, type, &mode, &rate);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Get original configuration failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
            rc = air_sec_setPortStormCtrlRate(unit, port, type, storm_mode[port - 1], rate);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Set new configuration failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        memcpy(&storm_mode[index - 1], (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        port = (UI32_T)index;
        rc = air_sec_getPortStormCtrlRate(unit, port, type, &mode, &rate);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Get original configuration failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }

        rc = air_sec_setPortStormCtrlRate(unit, port, type, storm_mode[index - 1], rate);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Set new configuration failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: syncd_api_stormctrl_rate
 * PURPOSE:
 *      Set stormctrl rate value.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      Use min ratelimit unit
 */
MW_ERROR_NO_T
syncd_api_stormctrl_rate(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    AIR_SEC_STORM_TYPE_T        type = AIR_SEC_STORM_TYPE_LAST;
    AIR_SEC_STORM_RATE_MODE_T   mode;
    UI32_T  unit = 0;
    UI32_T  port;
    UI32_T  storm_rate[MAX_PORT_NUM];
    UI16_T  index;
    UI16_T  expect_entry_num;
    UI32_T  rate;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg->ptr_type->e_idx;

    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG("ety_idx=%u, expect_entry_num=%u", index, expect_entry_num);
    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI32_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    switch(ptr_api_arg->ptr_type->f_idx)
    {
        case PORT_STORM_BC_RATE:
            type = AIR_SEC_STORM_TYPE_BC;
            break;
        case PORT_STORM_MC_RATE:
            type = AIR_SEC_STORM_TYPE_UMC;
            break;
        case PORT_STORM_UC_RATE:
            type = AIR_SEC_STORM_TYPE_UUC;
            break;
        default:
            SYNCD_LOG_ERROR("Not support this mode(%d)", ptr_api_arg->ptr_type->f_idx);
            return MW_E_BAD_PARAMETER;
    }

    if(DB_ALL_ENTRIES == index)
    {
        memcpy(&storm_rate, (UI32_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            /* Not support CPU port */
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            rc = air_sec_getPortStormCtrlRate(unit, port, type, &mode, &rate);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Get original configuration failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
            rc = air_sec_setPortStormCtrlRate(unit, port, type, mode, storm_rate[port - 1]);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Set new configuration failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        memcpy(&storm_rate[index - 1], (UI32_T*)ptr_api_arg->ptr_data, ptr_api_arg->data_size);
        port = (UI32_T)index;
        rc = air_sec_getPortStormCtrlRate(unit, port, type, &mode, &rate);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Get original configuration failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }

        rc = air_sec_setPortStormCtrlRate(unit, port, type, mode, storm_rate[index - 1]);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Set new configuration failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }
    }

    return AIR_E_OK;
}

