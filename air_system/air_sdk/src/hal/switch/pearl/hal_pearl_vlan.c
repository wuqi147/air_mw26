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

/* FILE NAME:  hal_pearl_vlan.c
 * PURPOSE:
 *  Implement VLAN module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_vlan.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_util.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/pearl/hal_pearl_reg.h>
#include <osal/osal_lib.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_VLAN_ARRAY_SIZE_UI8_MAC        (6)
#define HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC       (2)
#define HAL_PEARL_VLAN_ARRAY_SIZE_UI8_IPV6       (16)
#define HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6      (4)
#define HAL_PEARL_VLAN_MAX_HW_IDX_MAC_BASED      (pearl_max_qinq_entry_cnt + pearl_max_mac_based_entry_cnt - 1)
#define HAL_PEARL_VLAN_MAX_HW_IDX_IPV4_BASED     (HAL_PEARL_VLAN_QINQ_MAC_IPV4_MAX_ENTRY_CNT - 1)
#define HAL_PEARL_VLAN_MAX_HW_IDX_IPV6_BASED     (HAL_PEARL_VLAN_IPV6_BASED_MAX_ENTRY_CNT - 1)
#define HAL_PEARL_VLAN_MAX_HW_IDX_PROTOCOL_BASED (HAL_PEARL_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT - 1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
UI32_T       pearl_max_qinq_entry_cnt = 0;
UI32_T       pearl_max_mac_based_entry_cnt = 0;
UI32_T       pearl_max_ipv4_based_entry_cnt = 0;

const UI32_T PEARL_MAC_FULL_MASK[] = {0xffffffff, 0x0000ffff};
const UI32_T PEARL_IPV6_FULL_MASK[] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

DIAG_SET_MODULE_INFO(AIR_MODULE_VLAN, "hal_pearl_vlan.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES
 */

static void
_hal_pearl_vlan_generateContinMaskByLeng(
    const UI32_T max_mask_length,
    const UI32_T mask_length,
    UI32_T      *ptr_mask)
{
    UI32_T arr_cnt = CMLIB_UTIL_CEIL(max_mask_length, 32);
    /* Use the maximum mask size to prevent coverity coding standard violation */
    UI32_T tmp_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6] = {0};
    int    zero_cnt = max_mask_length - mask_length;
    UI32_T i = 0, range = 0;

    if (HAL_PEARL_VLAN_MASK_LENGTH_MAC_BASED == max_mask_length)
    {
        osal_memcpy(tmp_mask, PEARL_MAC_FULL_MASK, sizeof(UI32_T) * arr_cnt);
    }
    else
    {
        osal_memcpy(tmp_mask, PEARL_IPV6_FULL_MASK, sizeof(UI32_T) * arr_cnt);
    }

    while (1)
    {
        if (0 == zero_cnt)
        {
            break;
        }
        range = (zero_cnt > 32) ? 32 : zero_cnt;
        tmp_mask[i++] &= ~BITS_RANGE(0, range);
        zero_cnt -= 32;
        if (zero_cnt < 0)
        {
            break;
        }
    }
    osal_memcpy(ptr_mask, tmp_mask, sizeof(UI32_T) * arr_cnt);
}

static AIR_ERROR_NO_T
_hal_pearl_vlan_checkContinueousMask(
    UI32_T      *ptr_mask,
    const UI32_T arr_cnt,
    const UI32_T max_mask_length)
{
    AIR_ERROR_NO_T rc = AIR_E_BAD_PARAMETER;
    int            i = 0;
    UI32_T         max_arr_length = CMLIB_UTIL_CEIL(max_mask_length, 32);
    /* Use the maximum mask size to prevent coverity coding standard violation */
    UI32_T         tmp_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];

    for (i = max_mask_length; i >= 0; i--)
    {
        osal_memset(tmp_mask, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);
        _hal_pearl_vlan_generateContinMaskByLeng(max_mask_length, i, tmp_mask);

        if (osal_memcmp(ptr_mask, tmp_mask, sizeof(UI32_T) * max_arr_length) == 0)
        {
            rc = AIR_E_OK;
            break;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "invalid mask\n");
    }

    return rc;
}

static void
_hal_pearl_vlan_readData(
    const UI32_T                                 unit,
    const UI32_T                                 addr,
    const HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_T ctrl_type,
    UI32_T                                      *ptr_value0,
    UI32_T                                      *ptr_value1,
    UI32_T                                      *ptr_value2,
    UI32_T                                      *ptr_value3,
    UI32_T                                      *ptr_value4)
{
    UI32_T val = 0, cnt = 0;

    val = PEARL_VTCR_BUSY_MASK + (ctrl_type << PEARL_VTCR_FUNC_OFFT) + addr;
    aml_writeReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
    while (1)
    {
        if (cnt >= HAL_PEARL_VLAN_VTCR_BUSY_WAIT_MAX_CNT)
        {
            DIAG_PRINT(HAL_DBG_ERR, "Wait VTCR Complete Timeout.\n");
            break;
        }
        aml_readReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
        if ((val & 0x80000000) == 0)
        {
            break;
        }
        osal_delayUs(10);
        cnt++;
    };
    aml_readReg(unit, PEARL_VLNRDATA0, ptr_value0, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA1, ptr_value1, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA2, ptr_value2, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA3, ptr_value3, sizeof(UI32_T));
    aml_readReg(unit, PEARL_VLNRDATA4, ptr_value4, sizeof(UI32_T));
}

static void
_hal_pearl_vlan_writeData(
    const UI32_T                                 unit,
    const UI32_T                                 addr,
    const HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_T ctrl_type,
    UI32_T                                      *ptr_value0,
    UI32_T                                      *ptr_value1,
    UI32_T                                      *ptr_value2,
    UI32_T                                      *ptr_value3,
    UI32_T                                      *ptr_value4)
{
    UI32_T val = 0, cnt = 0;

    aml_writeReg(unit, PEARL_VLNWDATA0, ptr_value0, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA1, ptr_value1, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA2, ptr_value2, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA3, ptr_value3, sizeof(UI32_T));
    aml_writeReg(unit, PEARL_VLNWDATA4, ptr_value4, sizeof(UI32_T));

    val = PEARL_VTCR_BUSY_MASK + (ctrl_type << PEARL_VTCR_FUNC_OFFT) + addr;
    aml_writeReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
    while (1)
    {
        if (cnt >= HAL_PEARL_VLAN_VTCR_BUSY_WAIT_MAX_CNT)
        {
            DIAG_PRINT(HAL_DBG_ERR, "Wait VTCR Complete Timeout.\n");
            break;
        }
        aml_readReg(unit, PEARL_VTCR_ADDR, &val, sizeof(UI32_T));
        if ((val & PEARL_VTCR_BUSY_MASK) == 0)
        {
            break;
        }
        osal_delayUs(10);
        cnt++;
    };
}

void
_hal_pearl_vlan_regValToEntry(
    const UI32_T            value0,
    const UI32_T            value1,
    const UI32_T            value2,
    const UI32_T            value3,
    const UI32_T            value4,
    HAL_PEARL_VLAN_ENTRY_T *vlan_entry)
{
    I8_T i = 0;

    vlan_entry->valid = value0 & 0x1;
    vlan_entry->fid = (value0 >> 1) & 0xf;
    vlan_entry->ivl = (value0 >> 5) & 0x1;
    vlan_entry->copy_pri = (value0 >> 6) & 0x1;
    vlan_entry->user_pri = (value0 >> 7) & 0x7;
    vlan_entry->eg_con = (value0 >> 11) & 0x1;
    vlan_entry->egtag_ctl_en = (value0 >> 10) & 0x1;
    vlan_entry->member_port[0] = ((value0 >> 26) & 0x3f) | ((value1 & 0x1) << 6);
    for (i = 0; i <= 6; i++)
    {
        vlan_entry->egtag_ctl[i] = (value0 >> (12 + 2 * i)) & 0x3;
    }
    vlan_entry->stag = (value1 >> 2) & 0xfff;
}

void
_hal_pearl_vlan_entryToRegVal(
    const HAL_PEARL_VLAN_ENTRY_T *vlan_entry,
    UI32_T                       *value0,
    UI32_T                       *value1,
    UI32_T                       *value2,
    UI32_T                       *value3,
    UI32_T                       *value4)
{
    I8_T i = 0;

    *value0 = 0;
    *value1 = 0;
    *value2 = 0;
    *value3 = 0;
    *value4 = 0;

    *value0 |= vlan_entry->valid ? 0x1 : 0x0;
    *value0 |= (vlan_entry->fid & 0xf) << 1;
    *value0 |= (vlan_entry->ivl ? 0x1 : 0x0) << 5;
    *value0 |= (vlan_entry->copy_pri ? 0x1 : 0x0) << 6;
    *value0 |= (vlan_entry->user_pri & 0x7) << 7;
    *value0 |= (vlan_entry->eg_con ? 0x1 : 0x0) << 11;
    *value0 |= (vlan_entry->egtag_ctl_en ? 0x1 : 0x0) << 10;
    for (i = 0; i <= 6; i++)
    {
        *value0 |= (vlan_entry->egtag_ctl[i] & 0x3) << (12 + 2 * i);
    }
    *value0 |= (vlan_entry->member_port[0] & 0x3f) << 26;
    *value1 |= (vlan_entry->member_port[0] & 0x40) >> 6;
    *value1 |= (vlan_entry->stag & 0xfff) << 2;
}

void
_hal_pearl_vlan_readEntry(
    const UI32_T            unit,
    const UI16_T            vid,
    HAL_PEARL_VLAN_ENTRY_T *vlan_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_readData(unit, vid, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_READ, &val0, &val1, &val2, &val3,
                             &val4);
    _hal_pearl_vlan_regValToEntry(val0, val1, val2, val3, val4, vlan_entry);
}

void
_hal_pearl_vlan_writeEntry(
    const UI32_T            unit,
    const UI16_T            vid,
    HAL_PEARL_VLAN_ENTRY_T *vlan_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_entryToRegVal(vlan_entry, &val0, &val1, &val2, &val3, &val4);
    _hal_pearl_vlan_writeData(unit, vid, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_WRITE, &val0, &val1, &val2, &val3,
                              &val4);
}

static void
_hal_pearl_vlan_convertRgToEntryMacBased(
    const UI32_T                value0,
    const UI32_T                value1,
    const UI32_T                value2,
    const UI32_T                value3,
    const UI32_T                value4,
    BOOL_T                     *ptr_valid,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry)
{
    UI32_T mac_mask_length = 0;
    UI32_T arr_mac[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC];
    UI32_T arr_mac_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC];

    osal_memset(arr_mac, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC);
    osal_memset(arr_mac_mask, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC);

    *ptr_valid = (value0 & PEARL_MAC_BASED_VLAN_ENTRY_VALID) ? TRUE : FALSE;
    arr_mac[0] = BITS_OFF_R(value0, PEARL_MAC_BASED_VLAN_MACADDR_OFFSET, PEARL_MAC_BASED_VLAN_MACADDR_L_LENG);
    arr_mac[0] |= BITS_OFF_L(value1, PEARL_MAC_BASED_VLAN_MACADDR_L_LENG, PEARL_MAC_BASED_VLAN_MACADDR_OFFSET);
    arr_mac[1] = BITS_OFF_R(value1, PEARL_MAC_BASED_VLAN_MACADDR_OFFSET,
                            PEARL_MAC_BASED_VLAN_MACADDR_H_LENG - PEARL_MAC_BASED_VLAN_MACADDR_OFFSET);

    CMLIB_UTIL_HAL_MAC_TO_AIR_MAC(arr_mac, ptr_entry->mac);
    mac_mask_length = BITS_OFF_R(value1, PEARL_MAC_BASED_VLAN_MACMASK_OFFSET, PEARL_MAC_BASED_VLAN_MACMASK_LENG);
    _hal_pearl_vlan_generateContinMaskByLeng(HAL_PEARL_VLAN_MASK_LENGTH_MAC_BASED, mac_mask_length, arr_mac_mask);
    CMLIB_UTIL_HAL_MAC_TO_AIR_MAC(arr_mac_mask, ptr_entry->mac_mask);

    ptr_entry->pri = BITS_OFF_R(value1, PEARL_MAC_BASED_VLAN_PRI_OFFSET, PEARL_MAC_BASED_VLAN_PRI_LENG);
    ptr_entry->vid = BITS_OFF_R(value1, PEARL_MAC_BASED_VLAN_CVID_OFFSET, PEARL_MAC_BASED_VLAN_CVID_L_LENG);
    ptr_entry->vid |= BITS_OFF_L(value2, PEARL_MAC_BASED_VLAN_CVID_L_LENG, PEARL_MAC_BASED_VLAN_CVID_H_LENG);
}

static void
_hal_pearl_vlan_convertEntryToRgMacBased(
    const BOOL_T                      valid,
    const AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry,
    UI32_T                           *ptr_value0,
    UI32_T                           *ptr_value1,
    UI32_T                           *ptr_value2,
    UI32_T                           *ptr_value3,
    UI32_T                           *ptr_value4)
{
    UI32_T arr_mac[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC];
    UI32_T arr_mac_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC];
    UI32_T mac_mask_length = 0, i = 0;

    *ptr_value0 = 0;
    *ptr_value1 = 0;
    *ptr_value2 = 0;
    *ptr_value3 = 0;
    *ptr_value4 = 0;
    osal_memset(arr_mac, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC);
    osal_memset(arr_mac_mask, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC);

    CMLIB_UTIL_AIR_MAC_TO_HAL_MAC(ptr_entry->mac, arr_mac);
    CMLIB_UTIL_AIR_MAC_TO_HAL_MAC(ptr_entry->mac_mask, arr_mac_mask);
    for (i = 0; i < HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC; i++)
    {
        mac_mask_length += cmlib_util_popcount(arr_mac_mask[i]);
    }

    *ptr_value0 = (valid) ? PEARL_MAC_BASED_VLAN_ENTRY_VALID : PEARL_MAC_BASED_VLAN_ENTRY_INVALID;
    *ptr_value0 |= PEARL_VLN_TYPE_MAC_VOICE_SURVEILLANCE_BASE;
    *ptr_value0 |= BITS_OFF_L(arr_mac[0], PEARL_MAC_BASED_VLAN_MACADDR_OFFSET, PEARL_MAC_BASED_VLAN_MACADDR_L_LENG);

    *ptr_value1 |= BITS_OFF_R(arr_mac[0], PEARL_MAC_BASED_VLAN_MACADDR_L_LENG, PEARL_MAC_BASED_VLAN_MACADDR_OFFSET);
    *ptr_value1 |= BITS_OFF_L(arr_mac[1], PEARL_MAC_BASED_VLAN_MACADDR_OFFSET,
                              PEARL_MAC_BASED_VLAN_MACADDR_H_LENG - PEARL_MAC_BASED_VLAN_MACADDR_OFFSET);
    *ptr_value1 |= BITS_OFF_L(mac_mask_length, PEARL_MAC_BASED_VLAN_MACMASK_OFFSET, PEARL_MAC_BASED_VLAN_MACMASK_LENG);
    *ptr_value1 |= BITS_OFF_L(ptr_entry->pri, PEARL_MAC_BASED_VLAN_PRI_OFFSET, PEARL_MAC_BASED_VLAN_PRI_LENG);
    *ptr_value1 |= BITS_OFF_L(ptr_entry->vid, PEARL_MAC_BASED_VLAN_CVID_OFFSET, PEARL_MAC_BASED_VLAN_CVID_L_LENG);

    *ptr_value2 |= BITS_OFF_R(ptr_entry->vid, PEARL_MAC_BASED_VLAN_CVID_L_LENG, PEARL_MAC_BASED_VLAN_CVID_H_LENG);
}

static void
_hal_pearl_vlan_convertRgToEntryIpv4Based(
    const UI32_T                   value0,
    const UI32_T                   value1,
    const UI32_T                   value2,
    const UI32_T                   value3,
    const UI32_T                   value4,
    BOOL_T                        *ptr_valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    *ptr_valid = (value0 & PEARL_IPV4_BASED_VLAN_ENTRY_VALID) ? TRUE : FALSE;
    ptr_entry->ip_addr.ipv4 = TRUE;
    ptr_entry->ip_addr.ip_addr.ipv4_addr =
        BITS_OFF_R(value0, PEARL_IPV4_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV4_BASED_VLAN_IPADDR_L_LENG);

    ptr_entry->ip_addr.ip_addr.ipv4_addr |=
        BITS_OFF_L(value1, PEARL_IPV4_BASED_VLAN_IPADDR_L_LENG, PEARL_IPV4_BASED_VLAN_IPADDR_H_LENG);
    ptr_entry->ip_mask.ipv4 = TRUE;
    ptr_entry->ip_mask.ip_addr.ipv4_addr =
        BITS_OFF_R(value1, PEARL_IPV4_BASED_VLAN_IPMASK_OFFSET, PEARL_IPV4_BASED_VLAN_IPMASK_L_LENG);

    ptr_entry->ip_mask.ip_addr.ipv4_addr |=
        BITS_OFF_L(value2, PEARL_IPV4_BASED_VLAN_IPMASK_L_LENG, PEARL_IPV4_BASED_VLAN_IPMASK_H_LENG);
    ptr_entry->pri = BITS_OFF_R(value2, PEARL_IPV4_BASED_VLAN_PRI_OFFSET, PEARL_IPV4_BASED_VLAN_PRI_LENG);
    ptr_entry->vid = BITS_OFF_R(value2, PEARL_IPV4_BASED_VLAN_CVID_OFFSET, PEARL_IPV4_BASED_VLAN_CVID_LENG);
}

static void
_hal_pearl_vlan_convertEntryToRgIpv4Based(
    const BOOL_T                         valid,
    const AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry,
    UI32_T                              *ptr_value0,
    UI32_T                              *ptr_value1,
    UI32_T                              *ptr_value2,
    UI32_T                              *ptr_value3,
    UI32_T                              *ptr_value4)
{
    *ptr_value0 = 0;
    *ptr_value1 = 0;
    *ptr_value2 = 0;
    *ptr_value3 = 0;
    *ptr_value4 = 0;

    *ptr_value0 = (valid) ? PEARL_IPV4_BASED_VLAN_ENTRY_VALID : PEARL_IPV4_BASED_VLAN_ENTRY_INVALID;
    *ptr_value0 |= PEARL_VLN_TYPE_IPV4_BASE;
    *ptr_value0 |= BITS_OFF_L(ptr_entry->ip_addr.ip_addr.ipv4_addr, PEARL_IPV4_BASED_VLAN_IPADDR_OFFSET,
                              PEARL_IPV4_BASED_VLAN_IPADDR_L_LENG);

    *ptr_value1 |= BITS_OFF_R(ptr_entry->ip_addr.ip_addr.ipv4_addr, PEARL_IPV4_BASED_VLAN_IPADDR_L_LENG,
                              PEARL_IPV4_BASED_VLAN_IPADDR_H_LENG);
    *ptr_value1 |= BITS_OFF_L(ptr_entry->ip_mask.ip_addr.ipv4_addr, PEARL_IPV4_BASED_VLAN_IPMASK_OFFSET,
                              PEARL_IPV4_BASED_VLAN_IPMASK_L_LENG);

    *ptr_value2 |= BITS_OFF_R(ptr_entry->ip_mask.ip_addr.ipv4_addr, PEARL_IPV4_BASED_VLAN_IPMASK_L_LENG,
                              PEARL_IPV4_BASED_VLAN_IPMASK_H_LENG);
    *ptr_value2 |= BITS_OFF_L(ptr_entry->pri, PEARL_IPV4_BASED_VLAN_PRI_OFFSET, PEARL_IPV4_BASED_VLAN_PRI_LENG);
    *ptr_value2 |= BITS_OFF_L(ptr_entry->vid, PEARL_IPV4_BASED_VLAN_CVID_OFFSET, PEARL_IPV4_BASED_VLAN_CVID_LENG);
}

static void
_hal_pearl_vlan_convertRgToEntryIpv6Based(
    const UI32_T                   value0,
    const UI32_T                   value1,
    const UI32_T                   value2,
    const UI32_T                   value3,
    const UI32_T                   value4,
    BOOL_T                        *ptr_valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    UI32_T arr_ipv6[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];
    UI32_T arr_ipv6_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];
    UI32_T ipv6_mask_length = 0;

    osal_memset(arr_ipv6, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);
    osal_memset(arr_ipv6_mask, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);

    *ptr_valid = (value0 & PEARL_IPV6_BASED_VLAN_ENTRY_VALID) ? TRUE : FALSE;
    arr_ipv6[0] = BITS_OFF_R(value0, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);
    arr_ipv6[0] |= BITS_OFF_L(value1, PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    arr_ipv6[1] = BITS_OFF_R(value1, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);
    arr_ipv6[1] |= BITS_OFF_L(value2, PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    arr_ipv6[2] = BITS_OFF_R(value2, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);
    arr_ipv6[2] |= BITS_OFF_L(value3, PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    arr_ipv6[3] = BITS_OFF_R(value3, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);
    arr_ipv6[3] |= BITS_OFF_L(value4, PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);

    CMLIB_UTIL_HAL_IPV6_TO_AIR_IPV6(arr_ipv6, ptr_entry->ip_addr.ip_addr.ipv6_addr);
    ipv6_mask_length = BITS_OFF_R(value4, PEARL_IPV6_BASED_VLAN_IPMASK_OFFSET, PEARL_IPV6_BASED_VLAN_IPMASK_LENG);
    _hal_pearl_vlan_generateContinMaskByLeng(HAL_PEARL_VLAN_MASK_LENGTH_SUBNET_BASED_IPV6, ipv6_mask_length,
                                             arr_ipv6_mask);
    CMLIB_UTIL_HAL_IPV6_TO_AIR_IPV6(arr_ipv6_mask, ptr_entry->ip_mask.ip_addr.ipv6_addr);

    ptr_entry->pri = BITS_OFF_R(value4, PEARL_IPV6_BASED_VLAN_PRI_OFFSET, PEARL_IPV6_BASED_VLAN_PRI_LENG);
    ptr_entry->vid = BITS_OFF_R(value4, PEARL_IPV6_BASED_VLAN_CVID_OFFSET, PEARL_IPV6_BASED_VLAN_CVID_LENG);
}

static void
_hal_pearl_vlan_convertEntryToRgIpv6Based(
    const BOOL_T                         valid,
    const AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry,
    UI32_T                              *ptr_value0,
    UI32_T                              *ptr_value1,
    UI32_T                              *ptr_value2,
    UI32_T                              *ptr_value3,
    UI32_T                              *ptr_value4)
{
    UI32_T arr_ipv6[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];
    UI32_T arr_ipv6_mask[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];
    UI32_T ipv6_mask_length = 0, i = 0;

    *ptr_value0 = 0;
    *ptr_value1 = 0;
    *ptr_value2 = 0;
    *ptr_value3 = 0;
    *ptr_value4 = 0;
    osal_memset(arr_ipv6, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);
    osal_memset(arr_ipv6_mask, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);

    CMLIB_UTIL_AIR_IPV6_TO_HAL_IPV6(ptr_entry->ip_addr.ip_addr.ipv6_addr, arr_ipv6);
    CMLIB_UTIL_AIR_IPV6_TO_HAL_IPV6(ptr_entry->ip_mask.ip_addr.ipv6_addr, arr_ipv6_mask);
    for (i = 0; i < HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6; i++)
    {
        ipv6_mask_length += cmlib_util_popcount(arr_ipv6_mask[i]);
    }

    *ptr_value0 = (valid) ? PEARL_IPV6_BASED_VLAN_ENTRY_VALID : PEARL_IPV6_BASED_VLAN_ENTRY_INVALID;
    *ptr_value0 |= BITS_OFF_L(arr_ipv6[0], PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);

    *ptr_value1 |= BITS_OFF_R(arr_ipv6[0], PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    *ptr_value1 |= BITS_OFF_L(arr_ipv6[1], PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);

    *ptr_value2 |= BITS_OFF_R(arr_ipv6[1], PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    *ptr_value2 |= BITS_OFF_L(arr_ipv6[2], PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);

    *ptr_value3 |= BITS_OFF_R(arr_ipv6[2], PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    *ptr_value3 |= BITS_OFF_L(arr_ipv6[3], PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET, PEARL_IPV6_BASED_VLAN_IPADDR_LENG);

    *ptr_value4 |= BITS_OFF_R(arr_ipv6[3], PEARL_IPV6_BASED_VLAN_IPADDR_LENG, PEARL_IPV6_BASED_VLAN_IPADDR_OFFSET);
    *ptr_value4 |= BITS_OFF_L(ipv6_mask_length, PEARL_IPV6_BASED_VLAN_IPMASK_OFFSET, PEARL_IPV6_BASED_VLAN_IPMASK_LENG);
    *ptr_value4 |= BITS_OFF_L(ptr_entry->pri, PEARL_IPV6_BASED_VLAN_PRI_OFFSET, PEARL_IPV6_BASED_VLAN_PRI_LENG);
    *ptr_value4 |= BITS_OFF_L(ptr_entry->vid, PEARL_IPV6_BASED_VLAN_CVID_OFFSET, PEARL_IPV6_BASED_VLAN_CVID_LENG);
}

static void
_hal_pearl_vlan_convertRgToEntryProtocolBased(
    const UI32_T                     value0,
    const UI32_T                     value1,
    const UI32_T                     value2,
    const UI32_T                     value3,
    const UI32_T                     value4,
    BOOL_T                          *ptr_valid,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry)
{
    *ptr_valid = (value0 & PEARL_PROTOCOL_BASED_VLAN_ENTRY_VALID) ? TRUE : FALSE;
    switch (BITS_OFF_R(value0, PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_OFFSET, PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_LENG))
    {
        case PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_ETHERNET:
            ptr_entry->frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET;
            ptr_entry->type_data.eth_type = BITS_OFF_R(value0, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                                       PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            break;
        case PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_RFC1042:
            ptr_entry->frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042;
            ptr_entry->type_data.rfc1042_eth_type = BITS_OFF_R(value0, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                                               PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            break;
        case PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_LLC_OTHER:
            ptr_entry->frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER;
            ptr_entry->type_data.llc_dsap_ssap = BITS_OFF_R(value0, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                                            PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            break;
        case PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_SNAP_OTHER:
            ptr_entry->frame_type = AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER;
            ptr_entry->type_data.snap_pid[0] = BITS_OFF_R(value0, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                                          PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            ptr_entry->type_data.snap_pid[0] |= BITS_OFF_L(value1, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_H_OFFSET,
                                                           PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET);
            ptr_entry->type_data.snap_pid[1] |=
                BITS_OFF_R(value1, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                           PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_H_LENG - PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET);
            break;
        default:
            break;
    }

    ptr_entry->group_id =
        BITS_OFF_R(value1, PEARL_PROTOCOL_BASED_VLAN_GROUP_ID_OFFSET, PEARL_PROTOCOL_BASED_VLAN_GROUP_ID_LENG);
}

static void
_hal_pearl_vlan_convertEntryToRgProtocolBased(
    const BOOL_T                           valid,
    const AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry,
    UI32_T                                *ptr_value0,
    UI32_T                                *ptr_value1,
    UI32_T                                *ptr_value2,
    UI32_T                                *ptr_value3,
    UI32_T                                *ptr_value4)
{
    *ptr_value0 = 0;
    *ptr_value1 = 0;
    *ptr_value2 = 0;
    *ptr_value3 = 0;
    *ptr_value4 = 0;

    *ptr_value0 = (valid) ? PEARL_PROTOCOL_BASED_VLAN_ENTRY_VALID : PEARL_PROTOCOL_BASED_VLAN_ENTRY_INVALID;
    switch (ptr_entry->frame_type)
    {
        case AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET:
            *ptr_value0 |= PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_ETHERNET << PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_OFFSET;
            *ptr_value0 |= BITS_OFF_L(ptr_entry->type_data.eth_type, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                      PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            *ptr_value1 = 0;
            break;
        case AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042:
            *ptr_value0 |= PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_RFC1042 << PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_OFFSET;
            *ptr_value0 |=
                BITS_OFF_L(ptr_entry->type_data.rfc1042_eth_type, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                           PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            *ptr_value1 = 0;
            break;
        case AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER:
            *ptr_value0 |= PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_LLC_OTHER
                           << PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_OFFSET;
            *ptr_value0 |= BITS_OFF_L(ptr_entry->type_data.llc_dsap_ssap, PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                      PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            *ptr_value1 = 0;
            break;
        case AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER:
            *ptr_value0 |= PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_SNAP_OTHER
                           << PEARL_PROTOCOL_BASED_VLAN_FRAME_TYPE_OFFSET;
            *ptr_value0 |= BITS_OFF_L(ptr_entry->type_data.snap_pid[0], PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                                      PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG);
            *ptr_value1 |= BITS_OFF_R(ptr_entry->type_data.snap_pid[0], PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_LENG,
                                      PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET);
            *ptr_value1 |=
                BITS_OFF_L(ptr_entry->type_data.snap_pid[1], PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET,
                           PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_H_LENG - PEARL_PROTOCOL_BASED_VLAN_TYPE_VALUE_L_OFFSET);
            break;
        default:
            break;
    }

    *ptr_value1 |= BITS_OFF_L(ptr_entry->group_id, PEARL_PROTOCOL_BASED_VLAN_GROUP_ID_OFFSET,
                              PEARL_PROTOCOL_BASED_VLAN_GROUP_ID_LENG);
}

static void
_hal_pearl_vlan_readMacBasedEntry(
    const UI32_T                unit,
    const UI32_T                idx,
    BOOL_T                     *ptr_valid,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_readData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_READ, &val0, &val1, &val2,
                             &val3, &val4);
    _hal_pearl_vlan_convertRgToEntryMacBased(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
}

static void
_hal_pearl_vlan_writeMacBasedEntry(
    const UI32_T                unit,
    const UI32_T                idx,
    const BOOL_T                valid,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_convertEntryToRgMacBased(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);
    _hal_pearl_vlan_writeData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_WRITE, &val0, &val1, &val2,
                              &val3, &val4);
}

static void
_hal_pearl_vlan_readIpv4BasedEntry(
    const UI32_T                   unit,
    const UI32_T                   idx,
    BOOL_T                        *ptr_valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_readData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_READ, &val0, &val1, &val2,
                             &val3, &val4);
    _hal_pearl_vlan_convertRgToEntryIpv4Based(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
}

static void
_hal_pearl_vlan_writeIpv4BasedEntry(
    const UI32_T                   unit,
    const UI32_T                   idx,
    const BOOL_T                   valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_convertEntryToRgIpv4Based(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);
    _hal_pearl_vlan_writeData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_WRITE, &val0, &val1, &val2,
                              &val3, &val4);
}

static void
_hal_pearl_vlan_readIpv6BasedEntry(
    const UI32_T                   unit,
    const UI32_T                   idx,
    BOOL_T                        *ptr_valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_readData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_READ, &val0, &val1, &val2, &val3,
                             &val4);
    _hal_pearl_vlan_convertRgToEntryIpv6Based(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
}

static void
_hal_pearl_vlan_writeIpv6BasedEntry(
    const UI32_T                   unit,
    const UI32_T                   idx,
    const BOOL_T                   valid,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_convertEntryToRgIpv6Based(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);
    _hal_pearl_vlan_writeData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_WRITE, &val0, &val1, &val2, &val3,
                              &val4);
}

static void
_hal_pearl_vlan_readProtocolBasedEntry(
    const UI32_T                     unit,
    const UI32_T                     idx,
    BOOL_T                          *ptr_valid,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_readData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_READ, &val0, &val1, &val2, &val3,
                             &val4);
    _hal_pearl_vlan_convertRgToEntryProtocolBased(val0, val1, val2, val3, val4, ptr_valid, ptr_entry);
}

static void
_hal_pearl_vlan_writeProtocolBasedEntry(
    const UI32_T                     unit,
    const UI32_T                     idx,
    const BOOL_T                     valid,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry)
{
    UI32_T val0, val1, val2, val3, val4;

    _hal_pearl_vlan_convertEntryToRgProtocolBased(valid, ptr_entry, &val0, &val1, &val2, &val3, &val4);
    _hal_pearl_vlan_writeData(unit, idx, HAL_PEARL_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_WRITE, &val0, &val1, &val2,
                              &val3, &val4);
}

static void
_hal_pearl_vlan_setBpduUntag(
    BOOL_T mode)
{
    UI32_T       unit = 0, val = 0;
    const UI32_T EG_TAG_CTRL = 4; /* untagged */

    aml_readReg(unit, PEARL_BPC, &val, sizeof(UI32_T));
    val &= ~(BITS_RANGE(PEARL_REG_BPDU_EG_TAG_OFFT, PEARL_REG_BPDU_EG_TAG_LENGTH));
    if (TRUE == mode)
    {
        val |= BITS_OFF_L(EG_TAG_CTRL, PEARL_REG_BPDU_EG_TAG_OFFT, PEARL_REG_BPDU_EG_TAG_LENGTH);
    }
    aml_writeReg(unit, PEARL_BPC, &val, sizeof(UI32_T));
}

/* FUNCTION NAME:   hal_pearl_vlan_create
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      ptr_entry   -- A pointer of struct for vlan entry configuration
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Vlan creation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      If ptr_entry is null, vlan would be created, which means the existence of vlan should be checked.
 *      Else, vlan would be init and don't care the existence of vlan for sake of speed.
 */
AIR_ERROR_NO_T
hal_pearl_vlan_create(
    const UI32_T            unit,
    const UI16_T            vid,
    AIR_VLAN_ENTRY_T *const ptr_entry)
{
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;
    UI32_T                 mac_port, port = 0;
    AIR_PORT_BITMAP_T      mac_pbmp;

    /*check para*/
    if (NULL != ptr_entry)
    {
        HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);
    }

    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
    if (NULL == ptr_entry)
    {
        _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
        if (TRUE == vlan_entry.valid)
        {
            return AIR_E_ENTRY_EXISTS;
        }
        vlan_entry.valid = TRUE;
    }
    else
    {
        vlan_entry.valid = TRUE;
        vlan_entry.fid = ptr_entry->fid;
        vlan_entry.ivl = (ptr_entry->flags & AIR_VLAN_ENTRY_FLAGS_EN_IVL) ? TRUE : FALSE;
        vlan_entry.copy_pri = (ptr_entry->flags & AIR_VLAN_ENTRY_FLAGS_EN_CP_PRI) ? TRUE : FALSE;
        vlan_entry.user_pri = ptr_entry->user_pri;
        vlan_entry.eg_con = (ptr_entry->flags & AIR_VLAN_ENTRY_FLAGS_EN_EG_CON) ? TRUE : FALSE;
        vlan_entry.egtag_ctl_en = (ptr_entry->flags & AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL) ? TRUE : FALSE;
        HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_entry->port_bitmap, mac_pbmp);
        AIR_PORT_BITMAP_COPY(vlan_entry.member_port, mac_pbmp);
        AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
            if (AIR_PORT_CHK(vlan_entry.member_port, mac_port))
            {
                vlan_entry.egtag_ctl[mac_port] = ptr_entry->egtag_ctl[port];
            }
        }
        vlan_entry.stag = ptr_entry->stag;
    }

    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan create unit=%d, vid=%d \n", unit, vid);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_destroy
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- Successfully read the data.
 *      AIR_E_OTHERS -- Vlan destroy failed.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_destroy(
    const UI32_T unit,
    const UI16_T vid)
{
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
    vlan_entry.valid = 0;

    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan destroy unit=%d, vid=%d \n", unit, vid);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_vlan_getVlan
 * PURPOSE:
 *      Get the VLAN entry when searching VLAN table.
 *
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *
 * OUTPUT:
 *      ptr_entry                -- vlan entry based on vid
 *
 * RETURN:
 *      AIR_E_OK                 -- Successfully read the data.
 *      AIR_E_ENTRY_NOT_FOUND    -- Valid vlan entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getVlan(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_VLAN_ENTRY_T *ptr_entry)
{
    UI32_T                 mac_port;
    AIR_PORT_BITMAP_T      port_pmap;
    UI32_T                 port = 0;
    HAL_PEARL_VLAN_ENTRY_T entry = {0};

    _hal_pearl_vlan_readEntry(unit, vid, &entry);
    if (TRUE == entry.valid)
    {
        ptr_entry->fid = entry.fid;
        if (entry.ivl)
        {
            ptr_entry->flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
        }
        else
        {
            ptr_entry->flags &= ~(AIR_VLAN_ENTRY_FLAGS_EN_IVL);
        }

        if (entry.copy_pri)
        {
            ptr_entry->flags |= AIR_VLAN_ENTRY_FLAGS_EN_CP_PRI;
        }
        else
        {
            ptr_entry->flags &= ~(AIR_VLAN_ENTRY_FLAGS_EN_CP_PRI);
        }

        ptr_entry->user_pri = entry.user_pri;

        if (entry.eg_con)
        {
            ptr_entry->flags |= AIR_VLAN_ENTRY_FLAGS_EN_EG_CON;
        }
        else
        {
            ptr_entry->flags &= ~(AIR_VLAN_ENTRY_FLAGS_EN_EG_CON);
        }

        if (entry.egtag_ctl_en)
        {
            ptr_entry->flags |= AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL;
        }
        else
        {
            ptr_entry->flags &= ~(AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL);
        }

        HAL_MAC_PBMP_TO_AIR_PBMP(unit, entry.member_port, port_pmap);
        AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, port_pmap);
        AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
            if (AIR_PORT_CHK(entry.member_port, mac_port))
            {
                ptr_entry->egtag_ctl[port] = entry.egtag_ctl[mac_port];
            }
        }
        ptr_entry->stag = entry.stag;
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get entry=%d, vid=%d, valid\n", unit, vid);
        return AIR_E_OK;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get entry=%d, vid=%d, not valid\n", unit, vid);
        return AIR_E_ENTRY_NOT_FOUND;
    }
}

/* FUNCTION NAME:   hal_pearl_vlan_setFid
 * PURPOSE:
 *      Set the filter id of the vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      fid         -- filter id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setFid(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T fid)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    /* VID check */
    HAL_CHECK_MIN_MAX_RANGE(fid, 0, HAL_PEARL_VLAN_FILTER_ID_MAX + 1);

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.fid = fid;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, fid=%d \n", unit, fid);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getFid
 * PURPOSE:
 *      Get the filter id of the vlan from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id to be created
 * OUTPUT:
 *      ptr_fid     -- filter id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getFid(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_fid)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_fid = vlan_entry.fid;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, fid=%d \n", unit, *ptr_fid);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_addMemberPort
 * PURPOSE:
 *      Add one vlan member to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_addMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 mac_port;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    AIR_PORT_ADD(vlan_entry.member_port, mac_port);
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan add member unit=%d, vid=%d, port=%d \n", unit, vid, port);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_delMemberPort
 * PURPOSE:
 *      Delete one vlan member from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_delMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 mac_port;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    AIR_PORT_DEL(vlan_entry.member_port, mac_port);
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan del member unit=%d, vid=%d, port=%d \n", unit, vid, port);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setMemberPort
 * PURPOSE:
 *      Replace the vlan members in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port_bitmap -- member port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setMemberPort(
    const UI32_T            unit,
    const UI16_T            vid,
    const AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;
    AIR_PORT_BITMAP_T      mac_pbmp;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_AIR_PBMP_TO_MAC_PBMP(unit, port_bitmap, mac_pbmp);

    AIR_PORT_BITMAP_COPY(vlan_entry.member_port, mac_pbmp);
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set member unit=%d, vid=%d, bitmap=%x \n", unit, vid, port_bitmap[0]);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getMemberPort
 * PURPOSE:
 *      Get the vlan members from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      port_bitmap -- member port bitmap
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getMemberPort(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_PORT_BITMAP_T port_bitmap)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;
    AIR_PORT_BITMAP_T      port_pbmp;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, vlan_entry.member_port, port_pbmp);

    AIR_PORT_BITMAP_COPY(port_bitmap, port_pbmp);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get member unit=%d, vid=%d, bitmap=%x \n", unit, vid, port_bitmap[0]);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setIVL
 * PURPOSE:
 *      Set L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable IVL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setIVL(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.ivl = enable ? 1 : 0;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, vid=%d, ivl=%d \n", unit, vid, enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getIVL
 * PURPOSE:
 *      Get L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable IVL
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getIVL(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_enable = vlan_entry.ivl ? TRUE : FALSE;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, vid=%d, ivl=%d \n", unit, vid, *ptr_enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setIngressTagKeeping
 * PURPOSE:
 *      Set per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag consistent
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.eg_con = enable ? 1 : 0;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, vid=%d, egsTagCon=%d \n", unit, vid, enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getIngressTagKeeping
 * PURPOSE:
 *      Get per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag consistent
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_enable = vlan_entry.eg_con ? TRUE : FALSE;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, vid=%d, egsTagCon=%d \n", unit, vid, *ptr_enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setEgsTagCtlEnable
 * PURPOSE:
 *      Set per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setEgsTagCtlEnable(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.egtag_ctl_en = enable ? 1 : 0;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, vid=%d, egsTagCtlEn=%d \n", unit, vid, enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getEgsTagCtlEnable
 * PURPOSE:
 *      Get per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getEgsTagCtlEnable(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_enable = vlan_entry.egtag_ctl_en ? TRUE : FALSE;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, vid=%d, egsTagCtlEn=%d \n", unit, vid, *ptr_enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortEgsTagCtl
 * PURPOSE:
 *      Set vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 *      tag_ctl     -- egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortEgsTagCtl(
    const UI32_T                            unit,
    const UI16_T                            vid,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_ctl)
{
    UI32_T                 mac_port;
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    vlan_entry.egtag_ctl[mac_port] = tag_ctl;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, vid=%d, egsTagCtl=%d \n", unit, vid, tag_ctl);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortEgsTagCtl
 * PURPOSE:
 *      Get vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_tag_ctl -- egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortEgsTagCtl(
    const UI32_T                       unit,
    const UI16_T                       vid,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_tag_ctl)
{
    UI32_T                 mac_port;
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    *ptr_tag_ctl = vlan_entry.egtag_ctl[mac_port];

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, vid=%d, egsTagCtl=%d \n", unit, vid, *ptr_tag_ctl);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortAcceptFrameType
 * PURPOSE:
 *      Set vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      type        -- accept frame type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortAcceptFrameType(
    const UI32_T                       unit,
    const UI32_T                       port,
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVC_ACC_FRM_MASK;
    val |= (type & PEARL_PVC_ACC_FRM_RELMASK) << PEARL_PVC_ACC_FRM_OFFT;
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, acceptFrameType=%d \n", unit, port, type);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortAcceptFrameType
 * PURPOSE:
 *      Get vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_type    -- accept frame type
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortAcceptFrameType(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *ptr_type)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    *ptr_type = (val >> PEARL_PVC_ACC_FRM_OFFT) & PEARL_PVC_ACC_FRM_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, port=%d, acceptFrameType=%d \n", unit, port, *ptr_type);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortAttr
 * PURPOSE:
 *      Set vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- vlan port attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortAttr(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_VLAN_PORT_ATTR_T attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVC_VLAN_ATTR_MASK;
    val |= (attr & PEARL_PVC_VLAN_ATTR_RELMASK) << PEARL_PVC_VLAN_ATTR_OFFT;
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, vlanMode=%d \n", unit, port, attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortAttr
 * PURPOSE:
 *      Get vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- vlan port attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortAttr(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_VLAN_PORT_ATTR_T *ptr_attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    *ptr_attr = (val >> PEARL_PVC_VLAN_ATTR_OFFT) & PEARL_PVC_VLAN_ATTR_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, port=%d, vlanMode=%d \n", unit, port, *ptr_attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setIgrPortTagAttr
 * PURPOSE:
 *      Set vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setIgrPortTagAttr(
    const UI32_T                      unit,
    const UI32_T                      port,
    const AIR_IGR_PORT_EGS_TAG_ATTR_T attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVC_EG_TAG_MASK;
    val |= (attr & PEARL_PVC_EG_TAG_RELMASK) << PEARL_PVC_EG_TAG_OFFT;
    aml_writeReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, igsPortEgsTagAttr=%d \n", unit, port, attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getIgrPortTagAttr
 * PURPOSE:
 *      Get vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getIgrPortTagAttr(
    const UI32_T                 unit,
    const UI32_T                 port,
    AIR_IGR_PORT_EGS_TAG_ATTR_T *ptr_attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVC(mac_port), &val, sizeof(UI32_T));
    *ptr_attr = (val >> PEARL_PVC_EG_TAG_OFFT) & PEARL_PVC_EG_TAG_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, port=%d, igsPortEgsTagAttr=%d \n", unit, port, *ptr_attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortEgsTagAttr
 * PURPOSE:
 *      Set vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortEgsTagAttr(
    const UI32_T                            unit,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PCR_EG_TAG_MASK;
    val |= (attr & PEARL_PCR_EG_TAG_RELMASK) << PEARL_PCR_EG_TAG_OFFT;
    aml_writeReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, portEgsTagAttr=%d \n", unit, port, attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortEgsTagAttr
 * PURPOSE:
 *      Get vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortEgsTagAttr(
    const UI32_T                       unit,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_attr)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    *ptr_attr = (val >> PEARL_PCR_EG_TAG_OFFT) & PEARL_PCR_EG_TAG_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, portEgsTagAttr=%d \n", unit, port, *ptr_attr);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortPVID
 * PURPOSE:
 *      Set PEARL_PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pvid        -- native vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortPVID(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T pvid)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVID(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PVID_CTAG_PORT_VID_MASK;
    val |= (pvid & PEARL_PVID_CTAG_VID_RELMASK) << PEARL_PVID_CTAG_VID_OFFT;
    aml_writeReg(unit, PEARL_PVID(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, pvid=%d \n", unit, port, pvid);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortPVID
 * PURPOSE:
 *      Get PEARL_PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_pvid    -- native vlan id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortPVID(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_pvid)
{
    UI32_T         mac_port;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PVID(mac_port), &val, sizeof(UI32_T));
    *ptr_pvid = (val >> PEARL_PVID_CTAG_VID_OFFT) & PEARL_PVID_CTAG_VID_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, pvid=%d \n", unit, port, *ptr_pvid);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_init
 * PURPOSE:
 *      This API is used to init vlan
 * INPUT:
 *      unit -- Device unit number
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI16_T                 vid;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;
    AIR_CFG_VALUE_T        entry_cnt;
    UI32_T                 total_entry_cnt;
    UI32_T                 port, mac_port;

    osal_memset(&vlan_entry, 0, sizeof(vlan_entry));
    for (vid = AIR_VLAN_ID_MIN; vid < AIR_VLAN_ID_MAX + 1; vid++)
    {
        _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);
    }

    /* All ports in VALN 1 */
    vid = 1;
    vlan_entry.valid = TRUE;
    vlan_entry.ivl = TRUE;
    vlan_entry.egtag_ctl_en = TRUE;
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
        AIR_PORT_ADD(vlan_entry.member_port, mac_port);
        vlan_entry.egtag_ctl[mac_port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
    }
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    osal_memset(&entry_cnt, 0, sizeof(AIR_CFG_VALUE_T));
    entry_cnt.value = HAL_PEARL_VLAN_DEFAULT_ENTRY_CNT_QINQ;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_ENTRY_CNT_SERVICE_VLAN, &entry_cnt);
    pearl_max_qinq_entry_cnt = entry_cnt.value;

    osal_memset(&entry_cnt, 0, sizeof(AIR_CFG_VALUE_T));
    entry_cnt.value = HAL_PEARL_VLAN_DEFAULT_ENTRY_CNT_MAC_BASED;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_ENTRY_CNT_MAC_BASED_VLAN, &entry_cnt);
    pearl_max_mac_based_entry_cnt = entry_cnt.value;

    osal_memset(&entry_cnt, 0, sizeof(AIR_CFG_VALUE_T));
    entry_cnt.value = HAL_PEARL_VLAN_DEFAULT_ENTRY_CNT_IPV4_BASED;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_ENTRY_CNT_IPV4_BASED_VLAN, &entry_cnt);
    pearl_max_ipv4_based_entry_cnt = entry_cnt.value;

    total_entry_cnt = pearl_max_qinq_entry_cnt + pearl_max_mac_based_entry_cnt + pearl_max_ipv4_based_entry_cnt;
    if (HAL_PEARL_VLAN_QINQ_MAC_IPV4_MAX_ENTRY_CNT < total_entry_cnt)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Set all ethernet ports & CPU port as VLAN ingress check disabled */
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        hal_pearl_vlan_setIngressVlanFilter(unit, port, FALSE);
    }

    /* Set BPDU egress tag to be untag */
    if (AIR_E_OK == rc)
    {
        _hal_pearl_vlan_setBpduUntag(TRUE);
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setCopyPriority
 * PURPOSE:
 *      Set the state of copying of CTAG priority per VLAN.
 *      Enable to copy priority in CTAG as STAG priority;
 *      Disable to use user priority which is configured in air_vlan_setUserPriority.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable copy priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setCopyPriority(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry = {0};

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.copy_pri = enable ? TRUE : FALSE;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, vid=%d, copyPri=%d \n", unit, vid, enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getCopyPriority
 * PURPOSE:
 *      Get per vlan copy priority state.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable copy priority
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getCopyPriority(
    const UI32_T  unit,
    const UI16_T  vid,
    BOOL_T *const ptr_enable)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry = {0};

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_enable = vlan_entry.copy_pri ? TRUE : FALSE;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, vid=%d, copyPri=%d \n", unit, vid, *ptr_enable);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setUserPriority
 * PURPOSE:
 *      Set user priority value in vlan table for STAG when copy priority is disabled.
 *      Copy priority is configured in air_vlan_setPriorityCopy.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      pri         -- user priority value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    const UI8_T  pri)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.user_pri = pri;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, user_pri=%d \n", unit, pri);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getUserPriority
 * PURPOSE:
 *      Get per vlan user priority value.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_pri     -- user priority value
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    UI8_T *const ptr_pri)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_VLAN_ENTRY_T vlan_entry;

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_pri = vlan_entry.user_pri;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, user_pri=%d \n", unit, *ptr_pri);
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setStag
 * PURPOSE:
 *      Set the service tag id in vlan table for vlan stack vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      stag        -- stag id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setStag(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T stag)
{
    HAL_PEARL_VLAN_ENTRY_T vlan_entry = {0};

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    vlan_entry.stag = stag;
    _hal_pearl_vlan_writeEntry(unit, vid, &vlan_entry);

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, stag=%d \n", unit, stag);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_getStag
 * PURPOSE:
 *      Get the service tag id in vlan table for vlan stack vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_stag    -- stag id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getStag(
    const UI32_T  unit,
    const UI16_T  vid,
    UI16_T *const ptr_stag)
{
    HAL_PEARL_VLAN_ENTRY_T vlan_entry = {0};

    _hal_pearl_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    *ptr_stag = vlan_entry.stag;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan get unit=%d, stag=%d \n", unit, *ptr_stag);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_setPortPSVID
 * PURPOSE:
 *      Set the VID of the service tag of the specified port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      psvid       -- service vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setPortPSVID(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T psvid)
{
    UI32_T mac_port;
    UI32_T val = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PSVID(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PSVID_STAG_PORT_VID_MASK;
    val |= (psvid & PEARL_PSVID_STAG_VID_RELMASK) << PEARL_PSVID_STAG_VID_OFFT;
    aml_writeReg(unit, PEARL_PSVID(mac_port), &val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, psvid=%d \n", unit, port, psvid);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_getPortPSVID
 * PURPOSE:
 *      Get the VID of the service tag of the specified port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_psvid   -- service vlan id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getPortPSVID(
    const UI32_T  unit,
    const UI32_T  port,
    UI16_T *const ptr_psvid)
{
    UI32_T mac_port;
    UI32_T val = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PEARL_PSVID(mac_port), &val, sizeof(UI32_T));
    *ptr_psvid = (val >> PEARL_PSVID_STAG_VID_OFFT) & PEARL_PSVID_STAG_VID_RELMASK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do vlan set unit=%d, port=%d, psvid=%d \n", unit, port, *ptr_psvid);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_getServiceVlanMaxEntryCnt
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit                    -- unit id
 * OUTPUT:
 *      ptr_entry_cnt           -- Max entry count
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getServiceVlanMaxCnt(
    const UI32_T unit,
    UI32_T      *ptr_entry_cnt)
{
    *ptr_entry_cnt = pearl_max_qinq_entry_cnt;
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_pearl_vlan_addMacBasedVlan
 * PURPOSE:
 *      Add MAC-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- MAC-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_addMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    BOOL_T                     valid;
    AIR_VLAN_MAC_BASED_ENTRY_T tmp_entry;
    UI32_T                     hw_idx;
    UI32_T                     mask_arr[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC];
    C8_T                       mac_str[CMLIB_UTIL_MAC_STR_SIZE];
    C8_T                       mask_str[CMLIB_UTIL_MAC_STR_SIZE];

    CMLIB_UTIL_MAC_TO_STR(mac_str, ptr_entry->mac);
    CMLIB_UTIL_MAC_TO_STR(mask_str, ptr_entry->mac_mask);
    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, mac=%s, mac_mask=%s\n", idx, mac_str, mask_str);
    DIAG_PRINT(HAL_DBG_INFO, "vid=%u, pri=%u \n", ptr_entry->vid, ptr_entry->pri);

    if (!pearl_max_mac_based_entry_cnt)
    {
        DIAG_PRINT(HAL_DBG_ERR, "MAC-based VLAN is not supported. pearl_max_mac_based_entry_cnt=%u \n",
                   pearl_max_mac_based_entry_cnt);
        rc = AIR_E_NOT_SUPPORT;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_mac_based_entry_cnt - 1);
        HAL_CHECK_VLAN(ptr_entry->vid);
        HAL_CHECK_ENUM_RANGE(ptr_entry->pri, AIR_VLAN_USER_PRI_MAX_NUM);

        osal_memset(mask_arr, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC);

        CMLIB_UTIL_AIR_MAC_TO_HAL_MAC(ptr_entry->mac_mask, mask_arr);
        rc = _hal_pearl_vlan_checkContinueousMask(mask_arr, HAL_PEARL_VLAN_ARRAY_SIZE_UI32_MAC,
                                                  HAL_PEARL_VLAN_MASK_LENGTH_MAC_BASED);

        if (AIR_E_OK == rc)
        {
            osal_memset(&tmp_entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));
            hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_MAC_BASED - idx;
            DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
            _hal_pearl_vlan_readMacBasedEntry(unit, hw_idx, &valid, &tmp_entry);
            if (valid)
            {
                DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, MAC-based entry is exists. \n", hw_idx);
                rc = AIR_E_ENTRY_EXISTS;
            }

            if (AIR_E_OK == rc)
            {
                _hal_pearl_vlan_writeMacBasedEntry(unit, hw_idx, TRUE, ptr_entry);
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_vlan_getMacBasedVlan
 * PURPOSE:
 *      Get a MAC based entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      ptr_entry       --  MAC based entry
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         hw_idx;
    BOOL_T         valid;
    C8_T           mac_str[CMLIB_UTIL_MAC_STR_SIZE];
    C8_T           mask_str[CMLIB_UTIL_MAC_STR_SIZE];

    DIAG_PRINT(HAL_DBG_INFO, "get MAC based entry unit=%d, idx=%d \n", unit, idx);
    if (!pearl_max_mac_based_entry_cnt)
    {
        DIAG_PRINT(HAL_DBG_ERR, "MAC-based VLAN is not supported. pearl_max_mac_based_entry_cnt=%u \n",
                   pearl_max_mac_based_entry_cnt);
        rc = AIR_E_NOT_SUPPORT;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_mac_based_entry_cnt - 1);

        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_MAC_BASED - idx;
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
        _hal_pearl_vlan_readMacBasedEntry(unit, hw_idx, &valid, ptr_entry);
        rc = (valid) ? AIR_E_OK : AIR_E_ENTRY_NOT_FOUND;
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, MAC-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }

        if (AIR_E_OK == rc)
        {
            CMLIB_UTIL_MAC_TO_STR(mac_str, ptr_entry->mac);
            CMLIB_UTIL_MAC_TO_STR(mask_str, ptr_entry->mac_mask);
            DIAG_PRINT(HAL_DBG_INFO, "idx=%u, mac=%s mac_mask=%s \n", idx, mac_str, mask_str);
            DIAG_PRINT(HAL_DBG_INFO, "vid=%u, pri=%u \n", ptr_entry->vid, ptr_entry->pri);
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_vlan_delMacBasedVlan
 * PURPOSE:
 *      Delete MAC based entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_delMacBasedVlan(
    const UI32_T unit,
    const UI32_T idx)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    BOOL_T                     valid;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;
    UI32_T                     hw_idx;

    DIAG_PRINT(HAL_DBG_INFO, "delete MAC based entry unit=%d, idx=%d \n", unit, idx);
    if (!pearl_max_mac_based_entry_cnt)
    {
        DIAG_PRINT(HAL_DBG_ERR, "MAC-based VLAN is not supported. pearl_max_mac_based_entry_cnt=%u \n",
                   pearl_max_mac_based_entry_cnt);
        rc = AIR_E_NOT_SUPPORT;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_mac_based_entry_cnt - 1);

        osal_memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));

        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_MAC_BASED - idx;
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
        _hal_pearl_vlan_readMacBasedEntry(unit, hw_idx, &valid, &entry);
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, MAC-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }

        if (AIR_E_OK == rc)
        {
            osal_memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));
            _hal_pearl_vlan_writeMacBasedEntry(unit, hw_idx, FALSE, &entry);
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_addSubnetBasedVlan
 * PURPOSE:
 *      Add Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- Subnet-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_addSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    BOOL_T                        valid;
    AIR_VLAN_SUBNET_BASED_ENTRY_T tmp_entry;
    UI32_T                        ipv6_mask_arr[HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6];
    UI32_T                        hw_idx;
    C8_T                          ipv4_str[CMLIB_UTIL_IPV4_STR_SIZE];
    C8_T                          ipv4_mask_str[CMLIB_UTIL_IPV4_STR_SIZE];
    C8_T                          ipv6_str[CMLIB_UTIL_IPV6_STR_SIZE];
    C8_T                          ipv6_mask_str[CMLIB_UTIL_IPV6_STR_SIZE];

    CMLIB_UTIL_IPV4_TO_STR(ipv4_str, ptr_entry->ip_addr.ip_addr.ipv4_addr);
    CMLIB_UTIL_IPV4_TO_STR(ipv4_mask_str, ptr_entry->ip_mask.ip_addr.ipv4_addr);
    CMLIB_UTIL_IPV6_TO_STR(ipv6_str, ptr_entry->ip_addr.ip_addr.ipv6_addr);
    CMLIB_UTIL_IPV6_TO_STR(ipv6_mask_str, ptr_entry->ip_mask.ip_addr.ipv6_addr);
    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, ip_addr.ipv4=%s ip_mask.ipv4=%s\n", idx,
               (ptr_entry->ip_addr.ipv4) ? "TRUE" : "FALSE", (ptr_entry->ip_mask.ipv4) ? "TRUE" : "FALSE");
    DIAG_PRINT(HAL_DBG_INFO, "ip_addr.ip_addr.ipv4_addr=%s \n", ipv4_str);
    DIAG_PRINT(HAL_DBG_INFO, "ip_mask.ip_addr.ipv4_addr=%s \n", ipv4_mask_str);
    DIAG_PRINT(HAL_DBG_INFO, "ip_addr.ip_addr.ipv6_addr=%s \n", ipv6_str);
    DIAG_PRINT(HAL_DBG_INFO, "ip_mask.ip_addr.ipv6_addr=%s \n", ipv6_mask_str);
    DIAG_PRINT(HAL_DBG_INFO, "vid=%u, pri=%u \n", ptr_entry->vid, ptr_entry->pri);

    if (ptr_entry->ip_addr.ipv4 != ptr_entry->ip_mask.ipv4)
    {
        rc = AIR_E_BAD_PARAMETER;
    }
    if (AIR_E_OK == rc)
    {
        if (ptr_entry->ip_addr.ipv4)
        {
            if (!pearl_max_ipv4_based_entry_cnt)
            {
                DIAG_PRINT(HAL_DBG_ERR, "IPV4-based VLAN is not supported. pearl_max_ipv4_based_entry_cnt=%u \n",
                           pearl_max_ipv4_based_entry_cnt);
                rc = AIR_E_NOT_SUPPORT;
            }
            else
            {
                HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_ipv4_based_entry_cnt - 1);
            }
        }
        else
        {
            HAL_CHECK_MIN_MAX_RANGE(idx, pearl_max_ipv4_based_entry_cnt,
                                    HAL_PEARL_VLAN_IPV6_BASED_MAX_ENTRY_CNT + pearl_max_ipv4_based_entry_cnt - 1);
        }

        if (AIR_E_OK == rc)
        {
            HAL_CHECK_VLAN(ptr_entry->vid);
            HAL_CHECK_ENUM_RANGE(ptr_entry->pri, AIR_VLAN_USER_PRI_MAX_NUM);
            osal_memset(ipv6_mask_arr, 0, sizeof(UI32_T) * HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6);
            if (!ptr_entry->ip_addr.ipv4)
            {
                CMLIB_UTIL_AIR_IPV6_TO_HAL_IPV6(ptr_entry->ip_mask.ip_addr.ipv6_addr, ipv6_mask_arr);
                rc = _hal_pearl_vlan_checkContinueousMask(ipv6_mask_arr, HAL_PEARL_VLAN_ARRAY_SIZE_UI32_IPV6,
                                                          HAL_PEARL_VLAN_MASK_LENGTH_SUBNET_BASED_IPV6);
            }

            if (AIR_E_OK == rc)
            {
                osal_memset(&tmp_entry, 0, sizeof(AIR_VLAN_SUBNET_BASED_ENTRY_T));
                if (idx < pearl_max_ipv4_based_entry_cnt)
                {
                    hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV4_BASED - idx;
                    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
                    _hal_pearl_vlan_readIpv4BasedEntry(unit, hw_idx, &valid, &tmp_entry);
                    if (valid)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV4-based entry is exists. \n", hw_idx);
                        rc = AIR_E_ENTRY_EXISTS;
                    }
                    if (AIR_E_OK == rc)
                    {
                        _hal_pearl_vlan_writeIpv4BasedEntry(unit, hw_idx, TRUE, ptr_entry);
                    }
                }
                else
                {
                    hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV6_BASED - (idx - pearl_max_ipv4_based_entry_cnt);
                    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
                    _hal_pearl_vlan_readIpv6BasedEntry(unit, hw_idx, &valid, &tmp_entry);
                    if (valid)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV6-based entry is exists. \n", hw_idx);
                        rc = AIR_E_ENTRY_EXISTS;
                    }
                    if (AIR_E_OK == rc)
                    {
                        _hal_pearl_vlan_writeIpv6BasedEntry(unit, hw_idx, TRUE, ptr_entry);
                    }
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getSubnetBasedVlan
 * PURPOSE:
 *      Get Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      ptr_entry               -- Subnet-based VLAN entry
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         hw_idx;
    BOOL_T         valid;
    C8_T           ipv4_str[CMLIB_UTIL_IPV4_STR_SIZE], ipv4_mask_str[CMLIB_UTIL_IPV4_STR_SIZE];
    C8_T           ipv6_str[CMLIB_UTIL_IPV6_STR_SIZE], ipv6_mask_str[CMLIB_UTIL_IPV6_STR_SIZE];

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_ipv4_based_entry_cnt + HAL_PEARL_VLAN_IPV6_BASED_MAX_ENTRY_CNT - 1);

    if (idx < pearl_max_ipv4_based_entry_cnt)
    {
        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV4_BASED - idx;
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
        _hal_pearl_vlan_readIpv4BasedEntry(unit, hw_idx, &valid, ptr_entry);
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV4-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV6_BASED - (idx - pearl_max_ipv4_based_entry_cnt);
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
        _hal_pearl_vlan_readIpv6BasedEntry(unit, hw_idx, &valid, ptr_entry);
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV6-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    if (AIR_E_OK == rc)
    {
        CMLIB_UTIL_IPV4_TO_STR(ipv4_str, ptr_entry->ip_addr.ip_addr.ipv4_addr);
        CMLIB_UTIL_IPV4_TO_STR(ipv4_mask_str, ptr_entry->ip_mask.ip_addr.ipv4_addr);
        CMLIB_UTIL_IPV6_TO_STR(ipv6_str, ptr_entry->ip_addr.ip_addr.ipv6_addr);
        CMLIB_UTIL_IPV6_TO_STR(ipv6_mask_str, ptr_entry->ip_mask.ip_addr.ipv6_addr);
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, ip_addr.ipv4=%s ip_mask.ipv4=%s\n", idx,
                   (ptr_entry->ip_addr.ipv4) ? "TRUE" : "FALSE", (ptr_entry->ip_mask.ipv4) ? "TRUE" : "FALSE");
        DIAG_PRINT(HAL_DBG_INFO, "ip_addr.ip_addr.ipv4_addr=%s \n", ipv4_str);
        DIAG_PRINT(HAL_DBG_INFO, "ip_mask.ip_addr.ipv4_addr=%s \n", ipv4_mask_str);
        DIAG_PRINT(HAL_DBG_INFO, "ip_addr.ip_addr.ipv6_addr=%s \n", ipv6_str);
        DIAG_PRINT(HAL_DBG_INFO, "ip_mask.ip_addr.ipv6_addr=%s \n", ipv6_mask_str);
        DIAG_PRINT(HAL_DBG_INFO, "vid=%u, pri=%u \n", ptr_entry->vid, ptr_entry->pri);
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_delSubnetBasedVlan
 * PURPOSE:
 *      Delete Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_delSubnetBasedVlan(
    const UI32_T unit,
    const UI32_T idx)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    BOOL_T                        valid;
    AIR_VLAN_SUBNET_BASED_ENTRY_T entry;
    UI32_T                        hw_idx;

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, pearl_max_ipv4_based_entry_cnt + HAL_PEARL_VLAN_IPV6_BASED_MAX_ENTRY_CNT - 1);

    if (idx < pearl_max_ipv4_based_entry_cnt)
    {
        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV4_BASED - idx;
        DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
        _hal_pearl_vlan_readIpv4BasedEntry(unit, hw_idx, &valid, &entry);
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV4-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }

        if (AIR_E_OK == rc)
        {
            osal_memset(&entry, 0, sizeof(AIR_VLAN_SUBNET_BASED_ENTRY_T));
            _hal_pearl_vlan_writeIpv4BasedEntry(unit, hw_idx, FALSE, &entry);
        }
    }
    else
    {
        hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_IPV6_BASED - (idx - pearl_max_ipv4_based_entry_cnt);
        _hal_pearl_vlan_readIpv6BasedEntry(unit, hw_idx, &valid, &entry);
        if (!valid)
        {
            DIAG_PRINT(HAL_DBG_ERR, "hw_idx=%u, IPV6-based entry is not found. \n", hw_idx);
            rc = AIR_E_ENTRY_NOT_FOUND;
        }

        if (AIR_E_OK == rc)
        {
            osal_memset(&entry, 0, sizeof(AIR_VLAN_SUBNET_BASED_ENTRY_T));
            _hal_pearl_vlan_writeIpv6BasedEntry(unit, hw_idx, FALSE, &entry);
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_addProtocolBasedVlan
 * PURPOSE:
 *      Add Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- Protocol-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_addProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    BOOL_T                          valid;
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T tmp_entry;
    UI32_T                          hw_idx;

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, HAL_PEARL_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_entry->group_id, AIR_VLAN_PROTOCOL_BASED_GROUP_ID_MIN,
                            AIR_VLAN_PROTOCOL_BASED_GROUP_ID_MAX);

    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, frame_type=%u, group_id=%u\n", idx, ptr_entry->frame_type, ptr_entry->group_id);
    if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET == ptr_entry->frame_type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "type_data.eth_type=0x%04x\n", ptr_entry->type_data.eth_type);
    }
    else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042 == ptr_entry->frame_type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "type_data.rfc1042_eth_type=0x%04x\n", ptr_entry->type_data.rfc1042_eth_type);
    }
    else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER == ptr_entry->frame_type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "type_data.llc_dsap_ssap=0x%04x\n", ptr_entry->type_data.llc_dsap_ssap);
    }
    else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER == ptr_entry->frame_type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "type_data.snap_pid[0]=0x%08x, type_data.snap_pid[1]=0x%08x\n",
                   ptr_entry->type_data.snap_pid[0], ptr_entry->type_data.snap_pid[1]);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "type_data=N/A");
    }

    hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_PROTOCOL_BASED - idx;
    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
    _hal_pearl_vlan_readProtocolBasedEntry(unit, hw_idx, &valid, &tmp_entry);
    if (valid)
    {
        DIAG_PRINT(HAL_DBG_ERR, "idx=%u, Protocol-based entry is exists. \n", idx);
        rc = AIR_E_ENTRY_EXISTS;
    }

    if (AIR_E_OK == rc)
    {
        _hal_pearl_vlan_writeProtocolBasedEntry(unit, hw_idx, TRUE, ptr_entry);
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getProtocolBasedVlan
 * PURPOSE:
 *      Get Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      ptr_entry               -- Protocol-based VLAN entry
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         valid;
    UI32_T         hw_idx;

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, HAL_PEARL_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT - 1);

    hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_PROTOCOL_BASED - idx;
    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
    _hal_pearl_vlan_readProtocolBasedEntry(unit, hw_idx, &valid, ptr_entry);
    if (!valid)
    {
        DIAG_PRINT(HAL_DBG_ERR, "idx=%u, Protocol-based entry is not found. \n", idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:idx=%u, frame_type=%u, group_id=%u\n", idx, ptr_entry->frame_type,
                   ptr_entry->group_id);
        if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET == ptr_entry->frame_type)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:type_data.eth_type=0x%04x\n", ptr_entry->type_data.eth_type);
        }
        else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042 == ptr_entry->frame_type)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:type_data.rfc1042_eth_type=0x%04x\n",
                       ptr_entry->type_data.rfc1042_eth_type);
        }
        else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER == ptr_entry->frame_type)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:type_data.llc_dsap_ssap=0x%04x\n", ptr_entry->type_data.llc_dsap_ssap);
        }
        else if (AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER == ptr_entry->frame_type)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:type_data.snap_pid[0]=0x%08x, type_data.snap_pid[1]=0x%08x\n",
                       ptr_entry->type_data.snap_pid[0], ptr_entry->type_data.snap_pid[1]);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:type_data=N/A");
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_delProtocolBasedVlan
 * PURPOSE:
 *      Delete Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_delProtocolBasedVlan(
    const UI32_T unit,
    const UI32_T idx)
{
    AIR_ERROR_NO_T                  rc = AIR_E_OK;
    BOOL_T                          valid;
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T entry;
    UI32_T                          hw_idx;

    HAL_CHECK_MIN_MAX_RANGE(idx, 0, HAL_PEARL_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT - 1);

    hw_idx = HAL_PEARL_VLAN_MAX_HW_IDX_PROTOCOL_BASED - idx;
    DIAG_PRINT(HAL_DBG_INFO, "idx=%u, hw_idx=%u\n", idx, hw_idx);
    _hal_pearl_vlan_readProtocolBasedEntry(unit, hw_idx, &valid, &entry);
    if (!valid)
    {
        DIAG_PRINT(HAL_DBG_ERR, "idx=%u, Protocol-based entry is not found. \n", idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(&entry, 0, sizeof(AIR_VLAN_PROTOCOL_BASED_ENTRY_T));
        _hal_pearl_vlan_writeProtocolBasedEntry(unit, hw_idx, FALSE, &entry);
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Set Protocol-based VLAN port attribute
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      group_id                -- group id
 *      vid                     -- vlan id
 *      pri                     -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    const UI16_T vid,
    const UI16_T pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T value = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "port=%u, mac_port=%u, group_id=%u, vid=%u, pri=%u\n", port, mac_port, group_id, vid, pri);

    switch (group_id / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEARL_PPBV1(mac_port), &value, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                value &=
                    ~(HAL_PEARL_VLAN_PPBV_VID_MASK << (HAL_PEARL_VLAN_PPBV_VID_OFFSET + ((!(group_id % 2)) ? 0 : 16)));
                value |= vid << (HAL_PEARL_VLAN_PPBV_VID_OFFSET + ((!(group_id % 2)) ? 0 : 16));
                value &=
                    ~(HAL_PEARL_VLAN_PPBV_PRI_MASK << (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + ((!(group_id % 2)) ? 0 : 16)));
                value |= pri << (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + ((!(group_id % 2)) ? 0 : 16));
                rc = aml_writeReg(unit, PEARL_PPBV1(mac_port), &value, sizeof(UI32_T));
            }
            break;
        case 1:
            rc = aml_readReg(unit, PEARL_PPBV2(mac_port), &value, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                value &=
                    ~(HAL_PEARL_VLAN_PPBV_VID_MASK << (HAL_PEARL_VLAN_PPBV_VID_OFFSET + ((!(group_id % 2)) ? 0 : 16)));
                value |= vid << (HAL_PEARL_VLAN_PPBV_VID_OFFSET + ((!(group_id % 2)) ? 0 : 16));
                value &=
                    ~(HAL_PEARL_VLAN_PPBV_PRI_MASK << (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + ((!(group_id % 2)) ? 0 : 16)));
                value |= pri << (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + ((!(group_id % 2)) ? 0 : 16));
                rc = aml_writeReg(unit, PEARL_PPBV2(mac_port), &value, sizeof(UI32_T));
            }
            break;
        default:
            DIAG_PRINT(HAL_DBG_ERR, "Invalid case: %u. \n", group_id / 2);
            break;
    }

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Set Protocol-based VLAN port attribute
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      group_id                -- group id
 * OUTPUT:
 *      ptr_vid                 -- vlan id
 *      ptr_pri                 -- priority
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    UI16_T      *ptr_vid,
    UI16_T      *ptr_pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T value = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (group_id / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEARL_PPBV1(mac_port), &value, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_vid = (value >> (HAL_PEARL_VLAN_PPBV_VID_OFFSET + (!(group_id % 2) ? 0 : 16))) &
                           HAL_PEARL_VLAN_PPBV_VID_MASK;
                *ptr_pri = (value >> (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + (!(group_id % 2) ? 0 : 16))) &
                           HAL_PEARL_VLAN_PPBV_PRI_MASK;
            }
            break;
        case 1:
            rc = aml_readReg(unit, PEARL_PPBV2(mac_port), &value, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_vid = (value >> (HAL_PEARL_VLAN_PPBV_VID_OFFSET + (!(group_id % 2) ? 0 : 16))) &
                           HAL_PEARL_VLAN_PPBV_VID_MASK;
                *ptr_pri = (value >> (HAL_PEARL_VLAN_PPBV_PRI_OFFSET + (!(group_id % 2) ? 0 : 16))) &
                           HAL_PEARL_VLAN_PPBV_PRI_MASK;
            }
            break;
        default:
            DIAG_PRINT(HAL_DBG_ERR, "Invalid case: %u. \n", group_id / 2);
            break;
    }

    DIAG_PRINT(HAL_DBG_INFO, "port=%u, mac_port=%u, group_id=%u, vid=%u, pri=%u\n", port, mac_port, group_id, *ptr_vid,
               *ptr_pri);

    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_setIngressVlanFilter
 * PURPOSE:
 *      Set ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      enable                  -- TRUE: vlan filter enable
 *                                 FALSE: vlan filter disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_setIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    val &= ~PEARL_PCR_PORT_VLAN_MASK;
    if (TRUE == enable)
    {
        val |= (HAL_PEARL_VLAN_PORT_MODE_SECURITY & PEARL_PCR_PORT_VLAN_RELMASK) << PEARL_PCR_PORT_VLAN_OFFT;
    }
    else
    {
        val |= (HAL_PEARL_VLAN_PORT_MODE_CHECK & PEARL_PCR_PORT_VLAN_RELMASK) << PEARL_PCR_PORT_VLAN_OFFT;
    }
    aml_writeReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    return rc;
}

/* FUNCTION NAME:   hal_pearl_vlan_getIngressVlanFilter
 * PURPOSE:
 *      Get ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_enable              -- TRUE: vlan filter enable
 *                                 FALSE: vlan filter disable
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_NOT_SUPPORT       -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_vlan_getIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         val = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PEARL_PCR(mac_port), &val, sizeof(UI32_T));
    switch ((val >> PEARL_PCR_PORT_VLAN_OFFT) & PEARL_PCR_PORT_VLAN_RELMASK)
    {
        case HAL_PEARL_VLAN_PORT_MODE_SECURITY:
            *ptr_enable = TRUE;
            break;
        case HAL_PEARL_VLAN_PORT_MODE_CHECK:
            *ptr_enable = FALSE;
            break;
        default:
            rc = AIR_E_NOT_SUPPORT;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_pearl_vlan_getCapacity
 * PURPOSE:
 *      Get the vlan resource capacity
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
hal_pearl_vlan_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_VLAN_MAC:
            *ptr_size = pearl_max_mac_based_entry_cnt;
            break;
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
            *ptr_size = pearl_max_ipv4_based_entry_cnt;
            break;
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
            *ptr_size = HAL_PEARL_VLAN_IPV6_BASED_MAX_ENTRY_CNT;
            break;
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            *ptr_size = HAL_PEARL_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}
