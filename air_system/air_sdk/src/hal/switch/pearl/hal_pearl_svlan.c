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
/* FILE NAME:  hal_pearl_svlan.c
 * PURPOSE:
 * It provides hal svlan module API
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/pearl/hal_pearl_svlan.h>

#include <air_svlan.h>
#include <air_vlan.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/pearl/hal_pearl_reg.h>
#include <hal/switch/pearl/hal_pearl_vlan.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

#define HAL_PEARL_SVLAN_MAX_BITS_LENGTH (32)

/* MACRO FUNCTION DECLARATIONS
 */

#define HAL_PEARL_SVLAN_BITS_LENG_CHK(__offset__, __leng__) (HAL_PEARL_SVLAN_MAX_BITS_LENGTH >= (__offset__ + __leng__))

#define HAL_PEARL_SVLAN_CAL_BITS_LENG(__big_val__, __small_val__) ((__big_val__ - __small_val__) - 1)

#define HAL_PEARL_SVLAN_FLD_TO_REG_VAL(__FLD_VAL__, __VAL0__, __VAL1__, __FLD_NAME__)                      \
    do                                                                                                     \
    {                                                                                                      \
        UI32_T __tmp_leng = 0;                                                                             \
        if (HAL_PEARL_SVLAN_BITS_LENG_CHK(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,                      \
                                          PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET))                   \
        {                                                                                                  \
            __VAL0__ |= BITS_OFF_L(__FLD_VAL__, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET,              \
                                   PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG);                            \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                    \
                                                       PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET) +     \
                         1;                                                                                \
            __VAL0__ |= BITS_OFF_L(__FLD_VAL__, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET, __tmp_leng); \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET +      \
                                                           PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,     \
                                                       HAL_PEARL_SVLAN_MAX_BITS_LENGTH) +                  \
                         1;                                                                                \
            __VAL1__ |= BITS_OFF_R(__FLD_VAL__, 0, __tmp_leng);                                            \
        }                                                                                                  \
    } while (0)

#define HAL_PEARL_SVLAN_FLD_TO_REG_VAL_MSK(__VAL0__, __VAL1__, __FLD_NAME__)                               \
    do                                                                                                     \
    {                                                                                                      \
        UI32_T __tmp_leng = 0;                                                                             \
        if (HAL_PEARL_SVLAN_BITS_LENG_CHK(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,                      \
                                          PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET))               \
        {                                                                                                  \
            __VAL0__ |= (BITS(0, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG - 1)                           \
                         << PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET);                             \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                    \
                                                       PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET) + \
                         1;                                                                                \
            __VAL0__ |= BITS_RANGE(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET, __tmp_leng);          \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET +  \
                                                           PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,     \
                                                       HAL_PEARL_SVLAN_MAX_BITS_LENGTH) +                  \
                         1;                                                                                \
            __VAL1__ |= BITS_RANGE(0, __tmp_leng);                                                         \
        }                                                                                                  \
    } while (0)

#define HAL_PEARL_SVLAN_REG_VAL_TO_FLD(__VAR__, __VAL0__, __VAL1__, __FLD_NAME__)                                \
    do                                                                                                           \
    {                                                                                                            \
        UI32_T __tmp_mask = 0, __tmp_leng = 0;                                                                   \
        if (HAL_PEARL_SVLAN_BITS_LENG_CHK(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,                            \
                                          PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET))                         \
        {                                                                                                        \
            __VAR__ = BITS_OFF_R(__VAL0__, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET,                         \
                                 PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG);                                    \
        }                                                                                                        \
        else                                                                                                     \
        {                                                                                                        \
            __VAR__ = BITS_OFF_R(__VAL0__, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET,                         \
                                 (HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                 \
                                                                PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET))); \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET +            \
                                                           PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,           \
                                                       HAL_PEARL_SVLAN_MAX_BITS_LENGTH) +                        \
                         1;                                                                                      \
            __tmp_mask = BITS_RANGE(0, __tmp_leng);                                                              \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                          \
                                                       PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET);            \
            __VAR__ |= ((__tmp_mask & __VAL0__) << __tmp_leng);                                                  \
        }                                                                                                        \
    } while (0)

#define HAL_PEARL_SVLAN_REG_VAL_TO_FLD_MSK(__VAR__, __VAL0__, __VAL1__, __FLD_NAME__)                      \
    do                                                                                                     \
    {                                                                                                      \
        UI32_T __tmp_mask = 0, __tmp_leng = 0;                                                             \
        if (HAL_PEARL_SVLAN_BITS_LENG_CHK(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,                      \
                                          PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET))               \
        {                                                                                                  \
            __VAR__ |= BITS(0, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG - 1)                             \
                       << PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET;                                \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                    \
                                                       PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET);  \
            __VAR__ = BITS_OFF_R(__VAL0__, PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_OFFSET, __tmp_leng);      \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET +  \
                                                           PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_LENG,     \
                                                       HAL_PEARL_SVLAN_MAX_BITS_LENGTH) +                  \
                         1;                                                                                \
            __tmp_mask = BITS_RANGE(0, __tmp_leng);                                                        \
            __tmp_leng = HAL_PEARL_SVLAN_CAL_BITS_LENG(HAL_PEARL_SVLAN_MAX_BITS_LENGTH,                    \
                                                       PEARL_SELECTIVE_QINQ_##__FLD_NAME__##_MSK_OFFSET) + \
                         1;                                                                                \
            __VAR__ |= ((__tmp_mask & __VAL0__) << __tmp_leng);                                            \
        }                                                                                                  \
    } while (0)
/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SVLAN, "hal_pearl_svlan.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

void
_hal_pearl_svlan_regValToEntry(
    const UI32_T       value0,
    const UI32_T       value1,
    const UI32_T       value2,
    const UI32_T       value3,
    const UI32_T       value4,
    BOOL_T            *ptr_valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    UI32_T mask = 0;

    if (value0 & 0x1)
    {
        *ptr_valid = TRUE;
    }
    else
    {
        *ptr_valid = FALSE;
    }
    HAL_PEARL_SVLAN_REG_VAL_TO_FLD(ptr_entry->eth_type, value0, value1, ETHERTYPE);
    HAL_PEARL_SVLAN_REG_VAL_TO_FLD(ptr_entry->c_vid, value0, value1, CVID);
    HAL_PEARL_SVLAN_REG_VAL_TO_FLD(ptr_entry->c_pri, value1, value2, CVID_PRI);

    HAL_PEARL_SVLAN_REG_VAL_TO_FLD_MSK(mask, value1, value2, ETHERTYPE);
    if (mask & BITS_RANGE(0, PEARL_SELECTIVE_QINQ_ETHERTYPE_LENG))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE;
    }

    HAL_PEARL_SVLAN_REG_VAL_TO_FLD_MSK(mask, value1, value2, CVID);
    if (mask & BITS_RANGE(0, PEARL_SELECTIVE_QINQ_CVID_LENG))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CVID;
    }

    HAL_PEARL_SVLAN_REG_VAL_TO_FLD_MSK(mask, value2, value3, CVID_PRI);
    if (mask & BITS_RANGE(0, PEARL_SELECTIVE_QINQ_CVID_PRI_LENG))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI;
    }
    HAL_PEARL_SVLAN_REG_VAL_TO_FLD(ptr_entry->s_vid, value2, value3, SVID);
    HAL_PEARL_SVLAN_REG_VAL_TO_FLD(ptr_entry->s_pri, value2, value3, SVID_PRI);
}

void
_hal_pearl_svlan_entryToRegVal(
    const BOOL_T             valid,
    const AIR_SVLAN_ENTRY_T *ptr_entry,
    UI32_T                  *value0,
    UI32_T                  *value1,
    UI32_T                  *value2,
    UI32_T                  *value3,
    UI32_T                  *value4)
{
    *value0 = 0;
    *value1 = 0;
    *value2 = 0;
    *value3 = 0;
    *value4 = 0;

    *value0 = (valid) ? PEARL_SELECTIVE_QINQ_VLAN_ENTRY_VAILD : PEARL_SELECTIVE_QINQ_VLAN_ENTRY_INVAILD;
    *value0 |= PEARL_VLN_TYPE_SELECTIVE_QINQ_BASE;
    HAL_PEARL_SVLAN_FLD_TO_REG_VAL(ptr_entry->eth_type, *value0, *value1, ETHERTYPE);
    HAL_PEARL_SVLAN_FLD_TO_REG_VAL(ptr_entry->c_vid, *value0, *value1, CVID);
    HAL_PEARL_SVLAN_FLD_TO_REG_VAL(ptr_entry->c_pri, *value1, *value2, CVID_PRI);

    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE)
    {
        HAL_PEARL_SVLAN_FLD_TO_REG_VAL_MSK(*value1, *value2, ETHERTYPE);
    }
    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_CVID)
    {
        HAL_PEARL_SVLAN_FLD_TO_REG_VAL_MSK(*value1, *value2, CVID);
    }
    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI)
    {
        HAL_PEARL_SVLAN_FLD_TO_REG_VAL_MSK(*value2, *value3, CVID_PRI);
    }

    HAL_PEARL_SVLAN_FLD_TO_REG_VAL(ptr_entry->s_pri, *value2, *value3, SVID_PRI);
    HAL_PEARL_SVLAN_FLD_TO_REG_VAL(ptr_entry->s_vid, *value2, *value3, SVID);
}

void
_hal_pearl_svlan_writeEntry(
    const UI32_T       unit,
    const UI32_T       idx,
    const BOOL_T       valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    UI32_T val, val0, val1, val2, val3, val4;

    _hal_pearl_svlan_entryToRegVal(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);

    aml_writeReg(unit, PEARL_VLNWDATA0, &val0, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA1, &val1, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA2, &val2, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA3, &val3, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA4, &val4, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:PEARL_VLNWDATA0-4 = %x, %x, %x, %x, %x\n", val0, val1, val2, val3, val4);

    val = (0x80005000 + idx);    // write MAC_IPv4_QinQ_cmd
    aml_writeReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:PEARL_VTCR_ADDR = %x \n", val);
    while (1)
    {
        aml_readReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
        if ((val & 0x80000000) == 0)
        {
            break;
        }
        osal_delayUs(10);
    };
}

void
_hal_pearl_svlan_readEntry(
    const UI32_T       unit,
    const UI32_T       idx,
    BOOL_T            *ptr_valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    UI32_T val, val0, val1, val2, val3, val4;

    val = (0x80004000 + idx);    // read MAC_IPv4_QinQ_cmd
    aml_writeReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));

    while (1)
    {
        aml_readReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
        if ((val & 0x80000000) == 0)
        {
            break;
        }
        osal_delayUs(10);
    };

    aml_readReg(unit, PEARL_VLNRDATA0, &val0, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA1, &val1, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA2, &val2, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA3, &val3, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA4, &val4, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:PEARL_VLNRDATA0-4 = %x, %x, %x, %x, %x\n", val0, val1, val2, val3, val4);
    _hal_pearl_svlan_regValToEntry(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
}

/* FUNCTION NAME: hal_pearl_svlan_addSvlan
 * PURPOSE:
 *      Add or set a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 *      ptr_entry       --  Structure of svlan table
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_EXISTS
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_addSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_SVLAN_ENTRY_T tmp_entry;
    BOOL_T            valid;
    UI32_T            max_entry_cnt = 0;
    UI32_T            hw_idx;

    hal_pearl_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (!max_entry_cnt)
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
    HAL_CHECK_VLAN(ptr_entry->c_vid);
    HAL_CHECK_VLAN(ptr_entry->s_vid);
    HAL_CHECK_MIN_MAX_RANGE(ptr_entry->s_pri, 0, HAL_PEARL_SVLAN_PRI_MAX);
    HAL_CHECK_MIN_MAX_RANGE(ptr_entry->c_pri, 0, HAL_PEARL_SVLAN_PRI_MAX);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:idx=%u, cvid=%u EtherType=0x%x \n", idx, ptr_entry->c_vid, ptr_entry->eth_type);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:cvid_pri=%u, svid=%u svid_pri=%u \n", ptr_entry->c_pri, ptr_entry->s_vid,
               ptr_entry->s_pri);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:flags=%x \n", ptr_entry->flags);
    hw_idx = max_entry_cnt - 1 - idx;

    osal_memset(&tmp_entry, 0, sizeof(AIR_SVLAN_ENTRY_T));
    _hal_pearl_svlan_readEntry(unit, hw_idx, &valid, &tmp_entry);
    if (valid)
    {
        return AIR_E_ENTRY_EXISTS;
    }

    valid = TRUE;
    _hal_pearl_svlan_writeEntry(unit, hw_idx, valid, ptr_entry);

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getSvlan
 * PURPOSE:
 *      Get a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      ptr_entry       --  Structure of svlan table
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         valid;
    UI32_T         max_entry_cnt = 0;
    UI32_T         hw_idx;

    hal_pearl_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (!max_entry_cnt)
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);
    hw_idx = max_entry_cnt - 1 - idx;

    _hal_pearl_svlan_readEntry(unit, hw_idx, &valid, ptr_entry);
    if (!valid)
    {
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:idx=%u, cvid=%u EtherType=0x%x \n", idx, ptr_entry->c_vid, ptr_entry->eth_type);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:cvid_pri=%u, svid=%u svid_pri=%u \n", ptr_entry->c_pri, ptr_entry->s_vid,
               ptr_entry->s_pri);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:flags=%x \n", ptr_entry->flags);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:valid=%s \n", (valid) ? "TRUE" : "FALSE");

    return rc;
}
/* FUNCTION NAME: hal_pearl_svlan_delSvlan
 * PURPOSE:
 *      Delete a svlan entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_delSvlan(
    const UI32_T unit,
    const UI32_T idx)
{
    AIR_SVLAN_ENTRY_T svlan_entry;
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    BOOL_T            valid = FALSE;
    UI32_T            max_entry_cnt = 0;
    UI32_T            hw_idx;

    hal_pearl_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (!max_entry_cnt)
    {
        return AIR_E_NOT_SUPPORT;
    }

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_MIN_MAX_RANGE(idx, 0, HAL_PEARL_SVLAN_ID_MAX);
    hw_idx = max_entry_cnt - 1 - idx;

    _hal_pearl_svlan_readEntry(unit, hw_idx, &valid, &svlan_entry);
    if (!valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    osal_memset(&svlan_entry, 0, sizeof(svlan_entry));
    valid = FALSE;

    _hal_pearl_svlan_writeEntry(unit, hw_idx, valid, &svlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan destroy unit=%d, idx=%d \n", unit, idx);

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_setCustomPort
 * PURPOSE:
 *      Set a svlan port to custom port
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      enable          --  Enable svlan custom port
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_setCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, portMode, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    portMode = enable ? AIR_VLAN_PORT_ATTR_USER_PORT : AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT;

    /* Read data from register */
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVC_VLAN_ATTR_MASK;
    val |= (portMode & PEARL_PVC_VLAN_ATTR_RELMASK) << PEARL_PVC_VLAN_ATTR_OFFT;

    /* Write data to register */
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, portMode=%d \n", unit, mac_port, portMode);
    return rc;
}
/* FUNCTION NAME: hal_pearl_svlan_getCustomPort
 * PURPOSE:
 *      Get svlan custom port enable status
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_enable      --  Svlan custom port enable status
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, portMode=%d \n", unit, mac_port,
               ((val >> PEARL_PVC_VLAN_ATTR_OFFT) & PEARL_PVC_VLAN_ATTR_RELMASK));

    *ptr_enable = (((val >> PEARL_PVC_VLAN_ATTR_OFFT) & PEARL_PVC_VLAN_ATTR_RELMASK) == AIR_VLAN_PORT_ATTR_USER_PORT) ?
                      TRUE :
                      FALSE;

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_setServicePort
 * PURPOSE:
 *      Set a svlan port to service port
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      enable          --  Enable svlan custom port
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_setServicePort(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, val2, egsTagAttr, portMode, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    egsTagAttr = enable ? AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK : AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
    portMode = enable ? AIR_VLAN_PORT_ATTR_STACK_PORT : AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT;

    /* Read data from register */
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVC_VLAN_ATTR_MASK;
    val |= (portMode & PEARL_PVC_VLAN_ATTR_RELMASK) << PEARL_PVC_VLAN_ATTR_OFFT; /* set port mode to user port */

    aml_readReg(unit, PEARL_PCR(mac_port), &val2, sizeof(UI32_T));
    val2 &= ~PEARL_PCR_EG_TAG_MASK;
    val2 |= (egsTagAttr & PEARL_PCR_EG_TAG_RELMASK) << PEARL_PCR_EG_TAG_OFFT; /* set tagged out */

    /* Write data to register */
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_PCR(mac_port), &val2, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, portMode=%d tag=%d \n", unit, mac_port, portMode, egsTagAttr);
    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getServicePort
 * PURPOSE:
 *      Get svlan service port enable status
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_enable      --  Svlan custom port enable status
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getServicePort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, val2, isEgsTagged, isUserPort, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    isEgsTagged =
        (((val >> PEARL_PCR_EG_TAG_OFFT) & PEARL_PCR_EG_TAG_RELMASK) == AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK) ? TRUE :
                                                                                                                 FALSE;

    aml_readReg(unit, PEARL_PVC(mac_port), &val2, sizeof(UI32_T));
    isUserPort = (((val2 >> PEARL_PVC_VLAN_ATTR_OFFT) & PEARL_PVC_VLAN_ATTR_RELMASK) == AIR_VLAN_PORT_ATTR_STACK_PORT) ?
                     TRUE :
                     FALSE;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, EgsTag=%d UserPort=%d \n", unit, mac_port,
               ((val >> PEARL_PVC_EG_TAG_OFFT) & PEARL_PVC_EG_TAG_RELMASK),
               ((val2 >> PEARL_PVC_VLAN_ATTR_OFFT) & PEARL_PVC_VLAN_ATTR_RELMASK));
    *ptr_enable = (isEgsTagged && isUserPort) ? TRUE : FALSE;
    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_setEgsOuterTPID
 * PURPOSE:
 *      Set svlan outer tag-protocol identifier(TPID) for egress frame
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_setEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Outer TPID=0x%x \n", mac_port, tpid);

    /* Read data from register */
    aml_readReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));
    val &= ~PEARL_EG_TPID_TPID_MASK(PEARL_EG_TPID_CSR_ETAG_TPID_0_OFFT);
    val |= (tpid & PEARL_EG_TPID_TPID_RELMASK) << PEARL_EG_TPID_CSR_ETAG_TPID_0_OFFT;

    /* Write data to register */
    aml_writeReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getEgsOuterTPID
 * PURPOSE:
 *      Get svlan outer tag-protocol identifier(TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));
    *ptr_tpid = val & PEARL_EG_TPID_TPID_RELMASK;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Outer TPID=0x%x \n", mac_port, *ptr_tpid);
    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_setEgsInnerTPID
 * PURPOSE:
 *      Set svlan inner tag-protocol identifier(TPID) for egress frame
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_setEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Inner TPID=0x%x \n", mac_port, tpid);

    /* Read data from register */
    aml_readReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));
    val &= ~PEARL_EG_TPID_TPID_MASK(PEARL_EG_TPID_CSR_ETAG_TPID_1_OFFT);
    val |= (tpid & PEARL_EG_TPID_TPID_RELMASK) << PEARL_EG_TPID_CSR_ETAG_TPID_1_OFFT;

    /* Write data to register */
    aml_writeReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getEgsInnerTPID
 * PURPOSE:
 *      Get svlan inner tag-protocol identifier(TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, PEARL_EG_TPID(mac_port), &val, sizeof(val));
    *ptr_tpid = (val >> PEARL_EG_TPID_CSR_ETAG_TPID_1_OFFT) & PEARL_EG_TPID_TPID_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Inner TPID=0x%x \n", mac_port, *ptr_tpid);
    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_setRecvTagTPID
 * PURPOSE:
 *      Set svlan stack tag TPID(outer TPID), it will be regarded as legal
 *      stack tag frame for incomming stack frame check
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 *      tpid            --  TPID value
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_setRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, stack tag TPID=0x%x \n", mac_port, tpid);
    /* Read data from register */
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(val));
    val &= ~PEARL_PVC_STAG_VPID_MASK;
    val |= (tpid & PEARL_PVC_STAG_VPID_RELMASK) << PEARL_PVC_STAG_VPID_OFFT;

    /* Write data to register */
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getRecvTagTPID
 * PURPOSE:
 *      Get svlan stack tag TPID(outer TPID) value
 *
 * INPUT:
 *      unit            --  Device ID
 *      portId          --  Port ID
 * OUTPUT:
 *      ptr_tpid        --  TPID value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_svlan_getRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(val));
    *ptr_tpid = (val >> PEARL_PVC_STAG_VPID_OFFT) & PEARL_PVC_STAG_VPID_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, stack tag TPID=0x%x \n", mac_port, *ptr_tpid);
    return rc;
}

/* FUNCTION NAME: hal_pearl_svlan_getCapacity
 * PURPOSE:
 *      Get the svlan resource capacity
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
hal_pearl_svlan_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_SVLAN:
            hal_pearl_vlan_getServiceVlanMaxCnt(unit, ptr_size);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}