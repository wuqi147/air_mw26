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
/* FILE NAME:  hal_coral_svlan.c
 * PURPOSE:
 * It provides hal svlan module API
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_svlan.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/switch/coral/hal_coral_reg.h>
#include <hal/switch/coral/hal_coral_vlan.h>

/* NAMING CONSTANT DECLARATIONS
 */

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
DIAG_SET_MODULE_INFO(AIR_MODULE_SVLAN, "hal_coral_svlan.c");

/* LOCAL SUBPROGRAM BODIES
 */
static void
_hal_coral_svlan_regValToEntry(
    const UI32_T       value0,
    const UI32_T       value1,
    const UI32_T       value2,
    const UI32_T       value3,
    const UI32_T       value4,
    BOOL_T            *ptr_valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    if (value0 & CORAL_SELECTIVE_QINQ_VLAN_ENTRY_VAILD)
    {
        *ptr_valid = TRUE;
    }
    else
    {
        *ptr_valid = FALSE;
    }
    ptr_entry->eth_type =
        BITS_OFF_R(value0, CORAL_SELECTIVE_QINQ_ETHERTYPE_OFFSET, CORAL_SELECTIVE_QINQ_ETHERTYPE_LENG);
    ptr_entry->c_vid = BITS_OFF_R(value0, CORAL_SELECTIVE_QINQ_CVID_OFFSET, CORAL_SELECTIVE_QINQ_CVID_LENG);
    ptr_entry->c_pri = BITS_OFF_R(value1, CORAL_SELECTIVE_QINQ_CVID_PRI_OFFSET, CORAL_SELECTIVE_QINQ_CVID_PRI_LENG);
    if (BITS_OFF_R(value1, CORAL_SELECTIVE_QINQ_ETHERTYPE_MSK_OFFSET, CORAL_SELECTIVE_QINQ_ETHERTYPE_LENG))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE;
    }
    if (BITS_OFF_R(value1, CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET, (32 - CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET)) &&
        BITS_OFF_R(value2, 0, (CORAL_SELECTIVE_QINQ_SVID_LENG - (32 - CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET))))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CVID;
    }
    if (BITS_OFF_R(value2, CORAL_SELECTIVE_QINQ_CVID_PRI_MSK_OFFSET, CORAL_SELECTIVE_QINQ_CVID_PRI_LENG))
    {
        ptr_entry->flags |= AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI;
    }
    ptr_entry->s_vid = BITS_OFF_R(value2, CORAL_SELECTIVE_QINQ_SVID_OFFSET, CORAL_SELECTIVE_QINQ_SVID_LENG);
    ptr_entry->s_pri = BITS_OFF_R(value2, CORAL_SELECTIVE_QINQ_SVID_PRI_OFFSET, CORAL_SELECTIVE_QINQ_SVID_PRI_LENG);
}

static void
_hal_coral_svlan_entryToRegVal(
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

    *value0 = (valid) ? CORAL_SELECTIVE_QINQ_VLAN_ENTRY_VAILD : CORAL_SELECTIVE_QINQ_VLAN_ENTRY_INVAILD;
    *value0 |= CORAL_VLN_TYPE_SELECTIVE_QINQ_BASE;
    *value0 |=
        BITS_OFF_L(ptr_entry->eth_type, CORAL_SELECTIVE_QINQ_ETHERTYPE_OFFSET, CORAL_SELECTIVE_QINQ_ETHERTYPE_LENG);
    *value0 |= BITS_OFF_L(ptr_entry->c_vid, CORAL_SELECTIVE_QINQ_CVID_OFFSET, CORAL_SELECTIVE_QINQ_CVID_LENG);

    *value1 |= BITS_OFF_L(ptr_entry->c_pri, CORAL_SELECTIVE_QINQ_CVID_PRI_OFFSET, CORAL_SELECTIVE_QINQ_CVID_PRI_LENG);
    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_ETH_TYPE)
    {
        *value1 |= (BITS_RANGE(0, CORAL_SELECTIVE_QINQ_ETHERTYPE_LENG) << CORAL_SELECTIVE_QINQ_ETHERTYPE_MSK_OFFSET);
    }
    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_CVID)
    {
        *value1 |= (BITS_RANGE(0, (32 - CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET)) << CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET);
        *value2 |= (BITS_RANGE(0, (CORAL_SELECTIVE_QINQ_CVID_LENG - (32 - CORAL_SELECTIVE_QINQ_CVID_MSK_OFFSET))));
    }
    if (ptr_entry->flags & AIR_SVLAN_ENTRY_FLAGS_CHK_CPRI)
    {
        *value2 |= (BITS_RANGE(0, CORAL_SELECTIVE_QINQ_CVID_PRI_LENG) << CORAL_SELECTIVE_QINQ_CVID_PRI_MSK_OFFSET);
    }

    *value2 |= BITS_OFF_L(ptr_entry->s_pri, CORAL_SELECTIVE_QINQ_SVID_PRI_OFFSET, CORAL_SELECTIVE_QINQ_SVID_PRI_LENG);
    *value2 |= BITS_OFF_L(ptr_entry->s_vid, CORAL_SELECTIVE_QINQ_SVID_OFFSET, CORAL_SELECTIVE_QINQ_SVID_LENG);
}

static AIR_ERROR_NO_T
_hal_coral_svlan_writeEntry(
    const UI32_T       unit,
    const UI32_T       idx,
    const BOOL_T       valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_coral_svlan_entryToRegVal(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);
    return hal_coral_vlan_writeData(unit, idx, HAL_CORAL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_WRITE, &val0, &val1,
                                    &val2, &val3, &val4);
}

static AIR_ERROR_NO_T
_hal_coral_svlan_readEntry(
    const UI32_T       unit,
    const UI32_T       idx,
    BOOL_T            *ptr_valid,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val0, val1, val2, val3, val4;

    rc = hal_coral_vlan_readData(unit, idx, HAL_CORAL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_READ, &val0, &val1,
                                 &val2, &val3, &val4);
    if (AIR_E_OK == rc)
    {
        _hal_coral_svlan_regValToEntry(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
    }

    return rc;
}

static void
_hal_coral_svlan_getUsage(
    UI32_T  unit,
    UI32_T *ptr_cnt)
{
    UI32_T            idx = 0, hw_idx = 0;
    BOOL_T            valid = FALSE;
    AIR_SVLAN_ENTRY_T entry;
    UI32_T            max_entry_cnt = 0;

    hal_coral_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    for (idx = 0; idx < max_entry_cnt; idx++)
    {
        hw_idx = max_entry_cnt - 1 - idx;
        _hal_coral_svlan_readEntry(unit, hw_idx, &valid, &entry);
        if (TRUE == valid)
        {
            (*ptr_cnt)++;
        }
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_svlan_addSvlan
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
hal_coral_svlan_addSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_SVLAN_ENTRY_T tmp_entry;
    BOOL_T            valid;
    UI32_T            max_entry_cnt = 0;
    UI32_T            hw_idx;

    hal_coral_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (0 != max_entry_cnt)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
        HAL_CHECK_VLAN(ptr_entry->c_vid);
        HAL_CHECK_VLAN(ptr_entry->s_vid);
        HAL_CHECK_MIN_MAX_RANGE(ptr_entry->c_pri, 0, HAL_CORAL_SVLAN_PRI_MAX);
        HAL_CHECK_MIN_MAX_RANGE(ptr_entry->s_pri, 0, HAL_CORAL_SVLAN_PRI_MAX);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:idx=%u, cvid=%u EtherType=0x%x \n", idx, ptr_entry->c_vid, ptr_entry->eth_type);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:cvid_pri=%u, svid=%u svid_pri=%u \n", ptr_entry->c_pri, ptr_entry->s_vid,
                   ptr_entry->s_pri);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:flags=%x \n", ptr_entry->flags);

        hw_idx = max_entry_cnt - 1 - idx;

        osal_memset(&tmp_entry, 0, sizeof(AIR_SVLAN_ENTRY_T));
        rc = _hal_coral_svlan_readEntry(unit, hw_idx, &valid, &tmp_entry);
        if (AIR_E_OK == rc)
        {
            if (TRUE == valid)
            {
                rc = AIR_E_ENTRY_EXISTS;
            }
            else
            {
                valid = TRUE;
                rc = _hal_coral_svlan_writeEntry(unit, hw_idx, valid, ptr_entry);
            }
        }
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getSvlan
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
hal_coral_svlan_getSvlan(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         valid;
    UI32_T         max_entry_cnt = 0;
    UI32_T         hw_idx;

    hal_coral_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (0 != max_entry_cnt)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
        HAL_CHECK_PTR(ptr_entry);

        hw_idx = max_entry_cnt - 1 - idx;

        rc = _hal_coral_svlan_readEntry(unit, hw_idx, &valid, ptr_entry);
        if (AIR_E_OK == rc)
        {
            if (TRUE == valid)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:idx=%u, cvid=%u EtherType=0x%x \n", idx, ptr_entry->c_vid,
                           ptr_entry->eth_type);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:cvid_pri=%u, svid=%u svid_pri=%u \n", ptr_entry->c_pri,
                           ptr_entry->s_vid, ptr_entry->s_pri);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:flags=%x \n", ptr_entry->flags);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:valid=%s \n", (valid) ? "TRUE" : "FALSE");
            }
            else
            {
                rc = AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_delSvlan
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
hal_coral_svlan_delSvlan(
    const UI32_T unit,
    const UI32_T idx)
{
    AIR_SVLAN_ENTRY_T svlan_entry;
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    BOOL_T            valid = FALSE;
    UI32_T            max_entry_cnt = 0;
    UI32_T            hw_idx;

    hal_coral_vlan_getServiceVlanMaxCnt(unit, &max_entry_cnt);
    if (0 != max_entry_cnt)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, max_entry_cnt - 1);
        hw_idx = max_entry_cnt - 1 - idx;

        rc = _hal_coral_svlan_readEntry(unit, hw_idx, &valid, &svlan_entry);
        if (AIR_E_OK == rc)
        {
            if (TRUE == valid)
            {
                osal_memset(&svlan_entry, 0, sizeof(svlan_entry));
                valid = FALSE;

                rc = _hal_coral_svlan_writeEntry(unit, hw_idx, valid, &svlan_entry);

                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan destroy unit=%d, idx=%d \n", unit, idx);
            }
            else
            {
                rc = AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_setCustomPort
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
hal_coral_svlan_setCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, portMode, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    portMode = enable ? AIR_VLAN_PORT_ATTR_USER_PORT : AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT;

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &val, sizeof(UI32_T));

    val &= ~BITS_RANGE(CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG);
    val |= BITS_OFF_L(portMode, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG);

    /* Write data to register */
    aml_writeReg(unit, CORAL_PVC(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, portMode=%d \n", unit, mac_port, portMode);

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getCustomPort
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
hal_coral_svlan_getCustomPort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &val, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, mac_port=%d, portMode=%d \n", unit, mac_port,
               BITS_OFF_R(val, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG));

    *ptr_enable = FALSE;
    if (AIR_VLAN_PORT_ATTR_USER_PORT == BITS_OFF_R(val, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG))
    {
        *ptr_enable = TRUE;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_setServicePort
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
hal_coral_svlan_setServicePort(
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
    aml_readReg(unit, CORAL_PVC(mac_port), &val, sizeof(UI32_T));
    aml_readReg(unit, CORAL_PCR(mac_port), &val2, sizeof(UI32_T));

    val &= ~BITS_RANGE(CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG);
    val |= BITS_OFF_L(portMode, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG);

    val2 &= ~BITS_RANGE(CORAL_PCR_EG_TAG_OFFT, CORAL_PCR_EG_TAG_LENG);
    val2 |= BITS_OFF_L(egsTagAttr, CORAL_PCR_EG_TAG_OFFT, CORAL_PCR_EG_TAG_LENG);

    /* Write data to register */
    aml_writeReg(unit, CORAL_PVC(mac_port), &val, sizeof(UI32_T));
    aml_writeReg(unit, CORAL_PCR(mac_port), &val2, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, portMode=%d tag=%d \n", unit, mac_port, portMode, egsTagAttr);

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getServicePort
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
hal_coral_svlan_getServicePort(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, val2, is_egs_tagged, is_user_port, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    is_egs_tagged = FALSE;
    is_user_port = FALSE;

    /* Read data from register */
    aml_readReg(unit, CORAL_PCR(mac_port), &val, sizeof(UI32_T));
    if (AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK == BITS_OFF_R(val, CORAL_PCR_EG_TAG_OFFT, CORAL_PCR_EG_TAG_LENG))
    {
        is_egs_tagged = TRUE;
    }

    aml_readReg(unit, CORAL_PVC(mac_port), &val2, sizeof(UI32_T));
    if (AIR_VLAN_PORT_ATTR_STACK_PORT == BITS_OFF_R(val2, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG))
    {
        is_user_port = TRUE;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: unit=%d, port=%d, EgsTag=%d PortMode=%d \n", unit, mac_port,
               BITS_OFF_R(val, CORAL_PCR_EG_TAG_OFFT, CORAL_PCR_EG_TAG_LENG),
               BITS_OFF_R(val2, CORAL_PVC_VLAN_ATTR_OFFT, CORAL_PVC_VLAN_ATTR_LENG));

    *ptr_enable = ((TRUE == is_egs_tagged) && (TRUE == is_user_port)) ? TRUE : FALSE;

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_setEgsOuterTPID
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
hal_coral_svlan_setEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Outer TPID=0x%x \n", mac_port, tpid);

    /* Read data from register */
    aml_readReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));
    val &= ~CORAL_EG_TPID_TPID_MASK(CORAL_EG_TPID_CSR_ETAG_TPID_0_OFFT);
    val |= (tpid & CORAL_EG_TPID_TPID_RELMASK) << CORAL_EG_TPID_CSR_ETAG_TPID_0_OFFT;

    /* Write data to register */
    aml_writeReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getEgsOuterTPID
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
hal_coral_svlan_getEgsOuterTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));
    *ptr_tpid = val & CORAL_EG_TPID_TPID_RELMASK;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Outer TPID=0x%x \n", mac_port, *ptr_tpid);

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_setEgsInnerTPID
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
hal_coral_svlan_setEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Inner TPID=0x%x \n", mac_port, tpid);

    /* Read data from register */
    aml_readReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));
    val &= ~CORAL_EG_TPID_TPID_MASK(CORAL_EG_TPID_CSR_ETAG_TPID_1_OFFT);
    val |= (tpid & CORAL_EG_TPID_TPID_RELMASK) << CORAL_EG_TPID_CSR_ETAG_TPID_1_OFFT;

    /* Write data to register */
    aml_writeReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getEgsInnerTPID
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
hal_coral_svlan_getEgsInnerTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_EG_TPID(mac_port), &val, sizeof(val));
    *ptr_tpid = (val >> CORAL_EG_TPID_CSR_ETAG_TPID_1_OFFT) & CORAL_EG_TPID_TPID_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, Inner TPID=0x%x \n", mac_port, *ptr_tpid);

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_setRecvTagTPID
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
hal_coral_svlan_setRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, stack tag TPID=0x%x \n", mac_port, tpid);
    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &val, sizeof(val));
    val &= ~BITS_RANGE(CORAL_PVC_STAG_VPID_OFFT, CORAL_PVC_STAG_VPID_LENG);
    val |= BITS_OFF_L(tpid, CORAL_PVC_STAG_VPID_OFFT, CORAL_PVC_STAG_VPID_LENG);

    /* Write data to register */
    aml_writeReg(unit, CORAL_PVC(mac_port), &val, sizeof(val));

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getRecvTagTPID
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
hal_coral_svlan_getRecvTagTPID(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *ptr_tpid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, portId, mac_port);

    /* Read data from register */
    aml_readReg(unit, CORAL_PVC(mac_port), &val, sizeof(val));
    *ptr_tpid = BITS_OFF_R(val, CORAL_PVC_STAG_VPID_OFFT, CORAL_PVC_STAG_VPID_LENG);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:portId=%u, stack tag TPID=0x%x \n", mac_port, *ptr_tpid);

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getCapacity
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
hal_coral_svlan_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_SVLAN:
            hal_coral_vlan_getServiceVlanMaxCnt(unit, ptr_size);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_svlan_getUsage
 * PURPOSE:
 *      Get the svlan resource usage
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
hal_coral_svlan_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_SVLAN:
            _hal_coral_svlan_getUsage(unit, ptr_cnt);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}