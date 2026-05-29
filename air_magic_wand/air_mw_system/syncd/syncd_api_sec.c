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

/* FILE NAME:  syncd_api_sec.c
 * PURPOSE:
 *  Implement security API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <osapi.h>
#include <syncd_in.h>
#include <syncd_api_sec.h>
#include <air_sec.h>

/* NAMING CONSTANT DECLARATIONS
*/

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

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: syncd_api_sec_macLimit
 * PURPOSE:
 *      Set the max. MAC learning number of a specific port.
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
syncd_api_sec_macLimit(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T air_rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T  unit = 0;
    UI16_T  index;
    UI32_T  port;
    UI16_T  *ptr_macLimit= NULL;
    UI16_T  macLimit;
    UI16_T  expect_entry_num;
    AIR_SEC_MAC_LIMIT_PORT_CFG_T mlp_cfg;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg ->ptr_type ->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    if ((sizeof(UI16_T) * expect_entry_num) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_macLimit = (UI16_T *)(ptr_api_arg ->ptr_data);
    if (M_GET == ptr_api_arg ->method)
    {
        /* Receive M_GET from DB means the 1st notification */
        if (DB_ALL_ENTRIES != index)
        {
            SYNCD_LOG_ERROR("The entry id(%u) of 1st notification is wrong", index);
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

            macLimit = htons(get16(((UI8_T *)ptr_macLimit)));
            SYNCD_LOG_DEBUG("port=%u, MAC Limit=%u", port, macLimit);
            /* Get current value */
            air_rc = air_sec_getMacLimitPortCfg(unit, port, &mlp_cfg);
            if (AIR_E_OK != air_rc)
            {
                SYNCD_LOG_ERROR("Port-%u Get Port security failed(%d)", port, air_rc);
                mw_rc = MW_E_OP_INCOMPLETE;
                ptr_macLimit++;
                continue;
            }
            /* If current value is different with setting value */
            if (mlp_cfg.sa_lmt_cnt != (UI32_T)macLimit)
            {
                /* if macLimit is equal to 0, means disable mac limitation */
                mlp_cfg.sa_lmt_cnt = (UI32_T)macLimit;
                if(SYNCD_MAC_LIMIT_DISABLE == macLimit)
                {
                    mlp_cfg.flags &= ~AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
                }
                else
                {
                    mlp_cfg.flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
                }
                SYNCD_LOG_DEBUG("Port-%u Set MAC Limit = %lu", port, mlp_cfg.sa_lmt_cnt);
                air_rc = air_sec_setMacLimitPortCfg(unit, port, &mlp_cfg);
                if (AIR_E_OK != air_rc)
                {
                    SYNCD_LOG_ERROR("Port-%u Set security failed(%d)", port, air_rc);
                    mw_rc = MW_E_OP_INCOMPLETE;
                    ptr_macLimit++;
                    continue;
                }
            }
            ptr_macLimit++;
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
        macLimit = htons(get16(((UI8_T *)ptr_macLimit)));
        SYNCD_LOG_DEBUG("port=%u, MAC Limit=%u", port, macLimit);
        /* Get current value */
        air_rc = air_sec_getMacLimitPortCfg(unit, port, &mlp_cfg);
        if (AIR_E_OK != air_rc)
        {
            SYNCD_LOG_ERROR("Port-%u Get Port security failed(%d)", port, air_rc);
            return MW_E_OP_INCOMPLETE;
        }
        /* if macLimit is equal to 0, means disable mac limitation */
        mlp_cfg.sa_lmt_cnt = (UI32_T)macLimit;
        if(SYNCD_MAC_LIMIT_DISABLE == macLimit)
        {
            mlp_cfg.flags &= ~AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
        }
        else
        {
            mlp_cfg.flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
        }
        SYNCD_LOG_DEBUG("Port-%u Set MAC Limit = %lu", port, mlp_cfg.sa_lmt_cnt);
        air_rc = air_sec_setMacLimitPortCfg(unit, port, &mlp_cfg);
        if (AIR_E_OK != air_rc)
        {
            SYNCD_LOG_ERROR("Port-%u Set security failed(%d)", port, air_rc);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }
    return MW_E_OK;
}

