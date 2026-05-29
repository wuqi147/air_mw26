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

/* FILE NAME:  syncd_api_port.c
 * PURPOSE:
 *  Implement port API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include "syncd_api_port.h"
#include "syncd_api_l2.h"
#include "syncd_in.h"

#include "air_port.h"
#include "air_swc.h"
#include "mw_platform.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_task.h"
#include "sfp_module_handle.h"
#include "sfp_auto_adaptation.h"
#include "sfp_pin.h"
#include "sfp_port.h"
#endif
#include "air_l2.h"
#include "air_lag.h"
#include "port_utils.h"
#ifdef AIR_SUPPORT_ERPS
#include "erps.h"
#include "erps_queue.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/

#define MW_PORT_CFG_AUTO_MODE   (2)
#define MW_PORT_CFG_SPD_ABILITY_2500M   (5)

/* MACRO FUNCTION DECLARATIONS
 */

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
#ifdef AIR_SUPPORT_SFP
BOOL_T _syncd_api_port_processGetMethod(
    UI32_T unit,
    UI8_T port,
    UI16_T e_idx)
{
    if ((DB_ALL_ENTRIES != e_idx) && (port != e_idx))
    {
        /* Not the target port */
        return FALSE;
    }

    /* Do not affect a port that is not a serdes port nor a combo port. */
    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboPort(unit, port)))
    {
        return TRUE;
    }

    /* Do not process when it is not triggered by SFP module */
    if ((DB_ALL_ENTRIES == e_idx) || (FALSE == sfp_check_ready()))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL_T _syncd_api_port_processUpdateMethod(
    UI32_T unit,
    UI8_T port)
{
    /* Do not affect a port that is not a serdes port nor a combo port. */
    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboPort(unit, port)))
    {
        return TRUE;
    }

    if (FALSE == sfp_check_ready())
    {
        SYNCD_LOG_DEBUG("SFP task is not ready. port:%d", port);
        return FALSE;
    }

    return TRUE;
}
#endif /* AIR_SUPPORT_SFP */

MW_ERROR_NO_T
_synd_api_port_setPortUnderANMode(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T portSettings)
{
    I32_T ret = MW_E_OK;
    AIR_PORT_PHY_AN_ADV_T adv = {0};
    UI16_T speedAbility = 0;
    UI8_T flowCtrl = 0xFF, isSerdesPort = FALSE;

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
    {
        isSerdesPort = TRUE;
    }
#endif

    ret = port_parsePortSettings(portSettings, NULL, &speedAbility, &flowCtrl);
    if (FALSE == isSerdesPort)
    {
        ret |= air_port_getPhyLocalAdvAbility(unit, port, &adv);
    }
#ifdef AIR_SUPPORT_SFP
    else
    {
        ret |= sfp_port_getPhyLocalAdvAbility(unit, port, &adv);
    }
#endif

    if (MW_E_OK != ret)
    {
        SYNCD_LOG_ERROR("setPortUnderANMode failed(%d)", ret);
        return ret;
    }

    port_covertSpeedAbility(&speedAbility);
    adv.flags &= ~PORT_SETTINGS_AIR_SPEED_ABILITY_BITMASK;
    adv.flags |= speedAbility;
    if (PORT_SETTINGS_FLOW_CTRL_OFF != flowCtrl)
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE;
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE;
    }
    else
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE);
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    }

    if (FALSE == isSerdesPort)
    {
        ret = air_port_setPhyLocalAdvAbility(unit, port, &adv);
        ret |= air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_ENABLE);
    }
#ifdef AIR_SUPPORT_SFP
    else
    {
        ret = sfp_port_setPhyLocalAdvAbility(unit, port, &adv);
        ret |= sfp_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_ENABLE);
    }
#endif
    if (AIR_E_OK != ret)
    {
        SYNCD_LOG_ERROR("Set local ability failed(%d)", ret);
    }

    return ret;
}

MW_ERROR_NO_T
_synd_api_port_setPortUnderForceMode(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T portSettings)
{
    I32_T ret = MW_E_OK;
    UI16_T speedAbility = 0;
    UI8_T flowCtrl = 0xFF, isSerdesPort = FALSE;
    AIR_PORT_SPEED_T        speed = AIR_PORT_SPEED_LAST;
    AIR_PORT_DUPLEX_T       duplex = AIR_PORT_DUPLEX_LAST;
#ifdef AIR_SUPPORT_SFP
    AIR_PORT_SERDES_MODE_T  serdes = AIR_PORT_SERDES_MODE_LAST;
    AIR_ERROR_NO_T          rc = AIR_E_OK;
#endif

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
    {
        isSerdesPort = TRUE;
    }
#endif

    ret = port_parsePortSettings(portSettings, NULL, &speedAbility, &flowCtrl);
    if (MW_E_OK != ret)
    {
        SYNCD_LOG_ERROR("setPortUnderForceMode(%d) line:%d", ret, __LINE__);
        return ret;
    }

    if ((0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_100HFDX)) ||
        (0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_100FUDX)))
    {
        speed = AIR_PORT_SPEED_100M;
    }
    else if (0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_2500M))
    {
        speed = AIR_PORT_SPEED_2500M;
    }
    else
    {
        speed = AIR_PORT_SPEED_10M;
    }

    if ((0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_10FUDX)) ||
        (0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_100FUDX)) ||
        (0 != (speedAbility & PORT_SETTINGS_RATE_DUPLEX_2500M)))
    {
        duplex = AIR_PORT_DUPLEX_FULL;
    }
    else
    {
        duplex = AIR_PORT_DUPLEX_HALF;
    }

    if (FALSE == isSerdesPort)
    {
        ret = air_port_setSpeed(unit, port, speed);
        ret |= air_port_setDuplex(unit, port, duplex);
        ret |= air_port_setFlowCtrl(unit, port, AIR_PORT_DIR_BOTH, (flowCtrl == PORT_SETTINGS_FLOW_CTRL_ON));
        ret |= air_port_setBackPressure(unit, port, (flowCtrl == PORT_SETTINGS_FLOW_CTRL_ON));
        ret |= air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_DISABLE);
    }
#ifdef AIR_SUPPORT_SFP
    else
    {
        ret = sfp_port_setSpeed(unit, port, speed);
        ret |= sfp_port_setDuplex(unit, port, duplex);
        ret |= sfp_port_setFlowCtrl(unit, port, AIR_PORT_DIR_BOTH, (flowCtrl == PORT_SETTINGS_FLOW_CTRL_ON));
        ret |= sfp_port_setBackPressure(unit, port, (flowCtrl == PORT_SETTINGS_FLOW_CTRL_ON));
        rc = air_port_getSerdesMode(unit, port, &serdes);
        ret |= rc;
        if (AIR_E_OK == rc)
        {
            if ((AIR_PORT_SERDES_MODE_100BASE_FX != serdes) &&
                (AIR_PORT_SERDES_MODE_HSGMII != serdes))
            {
                ret |= sfp_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_DISABLE);
            }
        }
    }
#endif
    if (AIR_E_OK != ret)
    {
        SYNCD_LOG_DEBUG("setPortUnderForceMode(%d) line:%d", ret, __LINE__);
        return ret;
    }

    return ret;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_port_status
 * PURPOSE:
 *      Set state of a specific port.
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
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_status(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T  unit = 0;
    UI32_T  port;
    UI16_T  index;
    UI8_T   *ptr_state = NULL;
    UI16_T  expect_entry_num;
#ifdef AIR_SUPPORT_ERPS
    UI32_T  erps_data = 0;
#endif

    MW_CHECK_PTR(ptr_api_arg);

    index = ptr_api_arg ->ptr_type ->e_idx;
    /* DB defines MAX_PORT_NUM as the maximum port number */
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_state = ptr_api_arg ->ptr_data;
    if (M_GET == ptr_api_arg ->method)
    {
#ifndef AIR_SUPPORT_SFP
        /* Receive M_GET from DB means the 1st notification */
        if (DB_ALL_ENTRIES != index)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of 1st notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }
#endif
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            /* Not support CPU port */
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            SYNCD_LOG_DEBUG("port=%u, state=%u", port, *ptr_state);
#ifdef AIR_SUPPORT_SFP
            if (FALSE == _syncd_api_port_processGetMethod(unit, port, index))
            {
                continue;
            }

            if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                (TRUE == sfp_port_is_comboPort(unit, port)))
            {
                MW_ERROR_NO_T result = MW_E_OK;

                SYNCD_LOG_DEBUG("\n %s line:%d port:%d sfp_ready:%d\n", __func__, __LINE__, port, sfp_check_ready());
                rc = sfp_port_setAdminState(unit, port, (BOOL_T)*ptr_state);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d) port:%d", rc, port);
                    mw_rc = MW_E_OP_INCOMPLETE;
                    result = mw_rc;
                }

                if ((MW_E_OK == result) && (FALSE == sfp_get_port_inited(port)))
                {
                    sfp_set_port_inited(port, SFP_PORT_INITED_BITMASK_ADMIN_STATUS);
                }
            }
            else
#endif /* AIR_SUPPORT_SFP */
            {
                rc = air_port_setAdminState(unit, port, (BOOL_T)*ptr_state);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d) port:%d", rc, port);
                    mw_rc = MW_E_OP_INCOMPLETE;
                }
            }
            ptr_state++;
        }
        return mw_rc;
    }
    else if (M_UPDATE == ptr_api_arg ->method)
    {
        /* Receive M_UPDATE from DB means Update data */
        if (DB_ALL_ENTRIES == index)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }
        port = (UI32_T)index;
        SYNCD_LOG_DEBUG("port=%u, state=%u", port, *ptr_state);
#ifdef AIR_SUPPORT_SFP
        if (FALSE == _syncd_api_port_processUpdateMethod(unit, port))
        {
            return MW_E_OK;
        }

        if((TRUE == sfp_port_is_serdesPort(unit, port)) ||
           (TRUE == sfp_port_is_comboPort(unit, port)))
        {
            rc = sfp_port_setAdminState(unit, port, (BOOL_T)*ptr_state);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
#ifdef AIR_SUPPORT_ERPS
            /* Notify erps task sfp port admin state change */
            if ((MW_E_OK == erps_check_port_is_ring_port(port)) &&
                (FALSE == *ptr_state))
            {
                erps_data = ((port & 0xFFFF) << 16) | (*ptr_state & 0xFFFF);
                erps_queue_port_admin_state_change_noti(&erps_data);
            }
#endif
        }
        else
#endif /* AIR_SUPPORT_SFP */
        {
            rc = air_port_setAdminState(unit, port, (BOOL_T)*ptr_state);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
syncd_api_port_settings(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    I32_T rc = MW_E_OK, ret = MW_E_OK;
    UI32_T unit = 0;
    UI32_T port = 0;
    UI16_T entryID = 0, portSettings = 0;
    UI8_T portMode = 0xFF;

    MW_CHECK_PTR(ptr_api_arg);

    entryID = ptr_api_arg->ptr_type->e_idx;
    /* DB defines PLAT_MAX_PORT_NUM as the maximum port number */
    if (((DB_ALL_ENTRIES == entryID) && ((sizeof(UI16_T) * PLAT_MAX_PORT_NUM) != ptr_api_arg->data_size)) ||
        ((DB_ALL_ENTRIES != entryID) && (sizeof(UI16_T) != ptr_api_arg->data_size)))
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if (M_GET == ptr_api_arg->method)
    {
#ifndef AIR_SUPPORT_SFP
        /* Receive M_GET from DB means the 1st notification */
        if (DB_ALL_ENTRIES != entryID)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of 1st notification is wrong", entryID);
            return MW_E_BAD_PARAMETER;
        }
#endif
        for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
        {
#ifdef AIR_SUPPORT_SFP
            if (FALSE == _syncd_api_port_processGetMethod(unit, port, entryID))
            {
                /* Skip ports when needed. */
                continue;
            }
#endif
            if (DB_ALL_ENTRIES != entryID)
            {
                osapi_memcpy(&portSettings, (UI8_T *)ptr_api_arg->ptr_data, sizeof(UI16_T));
            }
            else
            {
                osapi_memcpy(&portSettings, (UI8_T *)ptr_api_arg->ptr_data + sizeof(UI16_T) * (port - 1), sizeof(UI16_T));
            }
            SYNCD_LOG_DEBUG("Get port=%d, portSettings=0x%x", port, portSettings);
            rc = port_parsePortSettings(portSettings, &portMode, NULL, NULL);
            if (MW_E_OK != rc)
            {
                ret = rc;
                continue;
            }

            if (PORT_SETTINGS_PORT_MODE_FORCE != portMode)
            {
                rc = _synd_api_port_setPortUnderANMode(unit, port, portSettings);
            }
            else
            {
                rc = _synd_api_port_setPortUnderForceMode(unit, port, portSettings);
            }
            if (MW_E_OK != rc)
            {
                ret = rc;
                continue;
            }

#ifdef AIR_SUPPORT_SFP
            if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                (TRUE == sfp_port_is_comboPort(unit, port)))
            {
                if (FALSE == sfp_get_port_inited(port))
                {
                    sfp_set_port_inited(port, SFP_PORT_INITED_BITMASK_ADMIN_SPEED |
                                              SFP_PORT_INITED_BITMASK_ADMIN_FLOW_CTRL |
                                              SFP_PORT_INITED_BITMASK_ADMIN_SPD_ABILITY);
                }
            }
#endif
        }

        return ret;
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        /* Receive M_UPDATE from DB means Update data */
        if (DB_ALL_ENTRIES == entryID)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of notification is wrong", entryID);
            return MW_E_BAD_PARAMETER;
        }
        port = (UI32_T)entryID;
#ifdef AIR_SUPPORT_SFP
        if (FALSE == _syncd_api_port_processGetMethod(unit, port, entryID))
        {
            return MW_E_OP_INCOMPLETE;
        }
#endif
        osapi_memcpy(&portSettings, (UI8_T *)ptr_api_arg->ptr_data, sizeof(UI16_T));
        SYNCD_LOG_DEBUG("Update port=%d, portSettings=0x%x", port, portSettings);
        rc = port_parsePortSettings(portSettings, &portMode, NULL, NULL);
        if (MW_E_OK != rc)
        {
            return rc;
        }

        if (PORT_SETTINGS_PORT_MODE_FORCE != portMode)
        {
            rc = _synd_api_port_setPortUnderANMode(unit, port, portSettings);
        }
        else
        {
            rc = _synd_api_port_setPortUnderForceMode(unit, port, portSettings);
        }

        return rc;
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}


/* FUNCTION NAME: syncd_api_port_eeeConfig
 * PURPOSE:
 *      Set EEE mode of a specific port.
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
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_eeeConfig(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T air_rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T  unit = 0;
    UI32_T  port;
    UI16_T  index;
    UI8_T   *ptr_eee = NULL;
    UI16_T  expect_entry_num;
    AIR_PORT_PHY_AN_ADV_T adv;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg->ptr_type->e_idx;
    /* DB defines MAX_PORT_NUM as the maximum port number of eee_enable */
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("index:%d data_size(%d) is wrong", index, ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_eee = ptr_api_arg ->ptr_data;
    if (M_GET == ptr_api_arg ->method)
    {
#ifndef AIR_SUPPORT_SFP
        /* Receive M_GET from DB means the 1st notification */
        if (DB_ALL_ENTRIES != index)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of 1st notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }
#endif

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            SYNCD_LOG_DEBUG("port=%u, ps mode=%u", port, *ptr_eee);
            /* Not support CPU port */
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support EEE", port);
                continue;
            }
#ifdef AIR_SUPPORT_SFP
            if (FALSE == _syncd_api_port_processGetMethod(unit, port, index))
            {
                continue;
            }

            if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                (TRUE == sfp_port_is_comboPort(unit, port)))
            {
                SYNCD_LOG_DEBUG("\n %s line:%d port:%d sfp_ready:%d\n", __func__, __LINE__, port, sfp_check_ready());
                /* Get currnet value */
                air_rc = sfp_port_getPhyLocalAdvAbility(unit, port, &adv);
                if ((AIR_E_OK != air_rc) && (AIR_E_NOT_SUPPORT != air_rc))
                {
                    SYNCD_LOG_ERROR("Port-%u Get Port ADV failed(%d)", port, air_rc);
                    mw_rc = MW_E_OP_INCOMPLETE;
                    continue;
                }
                else if (AIR_E_OK == air_rc)
                {
                    /* If current value is different with setting value */
                    if ((TRUE == *ptr_eee) ^ (!!(adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_EEE)))
                    {
                        if (TRUE == *ptr_eee)
                        {
                            adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                        }
                        else
                        {
                            adv.flags &= ~AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                        }
                        SYNCD_LOG_DEBUG("Port-%u Set Port ADV = %X", port, adv.flags);
                        air_rc = sfp_port_setPhyLocalAdvAbility(unit, port, &adv);
                        if ((AIR_E_OK != air_rc) && (AIR_E_NOT_SUPPORT != air_rc))
                        {
                            SYNCD_LOG_ERROR("Port-%u Set PS Mode failed(%d)", port, air_rc);
                            mw_rc = MW_E_OP_INCOMPLETE;
                            continue;
                        }
                        (void)sfp_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
                    }
                }

                if (FALSE == sfp_get_port_inited(port))
                {
                    sfp_set_port_inited(port, SFP_PORT_INITED_BITMASK_EEE_ENABLE);
                }
            }
            else
#endif /* AIR_SUPPORT_SFP */
            {
                /* Get currnet value */
                air_rc = air_port_getPhyLocalAdvAbility(unit, port, &adv);
                if (AIR_E_OK != air_rc)
                {
                    SYNCD_LOG_ERROR("Port-%u Get Port ADV failed(%d)", port, air_rc);
                    mw_rc = MW_E_OP_INCOMPLETE;
                    continue;
                }

                /* If current value is different with setting value */
                if ((TRUE == *ptr_eee) ^ (!!(adv.flags & AIR_PORT_PHY_AN_ADV_FLAGS_EEE)))
                {
                    if (TRUE == *ptr_eee)
                    {
                        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                    }
                    else
                    {
                        adv.flags &= ~AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                    }
                    SYNCD_LOG_DEBUG("Port-%u Set Port ADV = %X", port, adv.flags);
                    air_rc = air_port_setPhyLocalAdvAbility(unit, port, &adv);
                    if (AIR_E_OK != air_rc)
                    {
                        SYNCD_LOG_ERROR("Port-%u Set PS Mode failed(%d)", port, air_rc);
                        mw_rc = MW_E_OP_INCOMPLETE;
                        continue;
                    }
                    (void)air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
                }
            }
            ptr_eee++;
        }
        return mw_rc;
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        /* Receive M_UPDATE from DB means Update data */
        if (DB_ALL_ENTRIES == index)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }
#ifdef AIR_SUPPORT_SFP
        if (FALSE == _syncd_api_port_processUpdateMethod(unit, index))
        {
            return MW_E_OK;
        }

        if ((TRUE == sfp_port_is_serdesPort(unit, index)) ||
            (TRUE == sfp_port_is_comboPort(unit, index)))
        {
            /* Get currnet value */
            air_rc = sfp_port_getPhyLocalAdvAbility(unit, index, &adv);
            if ((AIR_E_OK != air_rc) && (AIR_E_NOT_SUPPORT != air_rc))
            {
                SYNCD_LOG_DEBUG("port=%u, ps mode=%u", index, *ptr_eee);
                return MW_E_OP_INCOMPLETE;
            }
            else if (AIR_E_OK == air_rc)
            {
                if (TRUE == *ptr_eee)
                {
                    adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                }
                else
                {
                    adv.flags &= ~AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
                }
                SYNCD_LOG_DEBUG("Port-%u Set Port ADV = %X", index, adv.flags);
                air_rc = sfp_port_setPhyLocalAdvAbility(unit, index, &adv);
                if ((AIR_E_OK != air_rc) && (AIR_E_NOT_SUPPORT != air_rc))
                {
                    SYNCD_LOG_ERROR("Port-%u Set PS Mode failed(%d)", index, air_rc);
                    return MW_E_OP_INCOMPLETE;
                }
                (void)sfp_port_setPhyAutoNego(unit, index, AIR_PORT_PHY_AN_RESTART);
            }
        }
        else
#endif /* AIR_SUPPORT_SFP */
        {
            /* Get currnet value */
            air_rc = air_port_getPhyLocalAdvAbility(unit, index, &adv);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_DEBUG("port=%u, ps mode=%u", index, *ptr_eee);
                return MW_E_OP_INCOMPLETE;
            }
            if (TRUE == *ptr_eee)
            {
                adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
            }
            else
            {
                adv.flags &= ~AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
            }
            SYNCD_LOG_DEBUG("Port-%u Set Port ADV = %X", index, adv.flags);
            air_rc = air_port_setPhyLocalAdvAbility(unit, index, &adv);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_ERROR("Port-%u Set PS Mode failed(%d)", index, air_rc);
                return MW_E_OP_INCOMPLETE;
            }
            (void)air_port_setPhyAutoNego(unit, index, AIR_PORT_PHY_AN_RESTART);
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }
    return MW_E_OK;
}


/* FUNCTION NAME: syncd_api_port_jumbo
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
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
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_jumbo(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T port;
    UI8_T *ptr_jumbo;
    UI32_T jumbo;
    UI32_T jumboFrame[] = {0,
                           0,
                           AIR_SWC_JUMBO_SIZE_2048,
                           AIR_SWC_JUMBO_SIZE_3072,
                           AIR_SWC_JUMBO_SIZE_4096,
                           AIR_SWC_JUMBO_SIZE_5120,
                           AIR_SWC_JUMBO_SIZE_6144,
                           AIR_SWC_JUMBO_SIZE_7168,
                           AIR_SWC_JUMBO_SIZE_8192,
                           AIR_SWC_JUMBO_SIZE_9216,
                           AIR_SWC_JUMBO_SIZE_12288,
                           AIR_SWC_JUMBO_SIZE_15360};

    MW_CHECK_PTR(ptr_api_arg);

    port = ptr_api_arg->ptr_type->e_idx;

    if ((sizeof(UI32_T)) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_jumbo = ptr_api_arg->ptr_data;
    jumbo = *ptr_jumbo;
    SYNCD_LOG_DEBUG("ptr_jumbo=%p, jumbo=%u, frame=%d", ptr_jumbo, jumbo, jumboFrame[jumbo]);

    if ((M_GET == ptr_api_arg->method) || (M_UPDATE == ptr_api_arg->method))
    {
        /*
         * Receive M_GET from DB means the 1st notification
         * Receive M_UPDATE from DB means Update data
         */
        if (DB_ALL_ENTRIES != port)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of notification is wrong", port);
            return MW_E_BAD_PARAMETER;
        }

        /* 0 internally to represent disable jumbo frame */
        if (0 == jumbo)
        {
            rc = air_swc_setJumboSize(unit, AIR_SWC_JUMBO_SIZE_1518);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            rc = air_swc_setJumboSize(unit, jumboFrame[jumbo]);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_check_port_is_in_lag
 * PURPOSE:
 *      Check whether the port is in the lag.
 *
 * INPUT:
 *      port_id     --  the port id to be check
 *
 * OUTPUT:
 *      ptr_lag_id  --  porinter to lag id
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      If the port is in the trunk, MW_E_OK will be returned
 */
MW_ERROR_NO_T
syncd_api_check_port_is_in_lag(
    UI32_T      port_id,
    AIR_PORT_T  *ptr_lag_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    AIR_PORT_T lag_id = 0;
    UI32_T unit = 0;
    UI32_T max_group_cnt = 0, max_member_cnt = 0, member_cnt = 0, member_idx = 0;
    UI32_T *ptr_member = NULL;

    MW_CHECK_PTR(ptr_lag_id);
    if(PLAT_MAX_PORT_NUM < port_id)
    {
        return MW_E_BAD_PARAMETER;
    }

    rc = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
    if (AIR_E_OK != rc)
    {
        return MW_E_OTHERS;
    }

    osapi_calloc(sizeof(UI32_T) * max_member_cnt, "syncd", (void**)&ptr_member);
    if (NULL == ptr_member)
    {
        return MW_E_OTHERS;
    }

    for (lag_id = 0; lag_id < max_group_cnt; lag_id++)
    {
        rc = air_lag_getMember(unit, lag_id, &member_cnt, ptr_member);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_DEBUG("rc:%d member_cnt:%d\n", rc, member_cnt);
            continue;
        }
        /* Check if port is the member of the current trunk. */
        for (member_idx = 0; member_idx < member_cnt; member_idx++)
        {
            if(port_id == ptr_member[member_idx])
            {
                *ptr_lag_id = lag_id;
                MW_FREE(ptr_member);
                return MW_E_OK;
            }
        }
    }

    MW_FREE(ptr_member);
    return MW_E_ENTRY_NOT_FOUND;
}

/* FUNCTION NAME: syncd_api_flushPortBmpMacAddr
 * PURPOSE:
 *      This API is used to flush port bitmap dynamic mac address.
 *
 * INPUT:
 *      unit        -- Device ID
 *      portbmp     --  port bitmap to flush
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_flushPortBmpMacAddr(
    const UI32_T    unit,
    const MW_PORT_BITMAP_T portbmp)
{
    UI32_T          port = 0;
    AIR_ERROR_NO_T  rc = AIR_E_OK;

    MW_PORT_FOREACH(portbmp, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        rc = syncd_api_flushMacAddr(unit, port, SYNCD_API_L2_FLUSH_PORT);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("***Error***, flush port %d fail\n", port);
            break;
        }
    }

    if(AIR_E_OK != rc)
    {
        return MW_E_OTHERS;
    }
    return MW_E_OK;
}

