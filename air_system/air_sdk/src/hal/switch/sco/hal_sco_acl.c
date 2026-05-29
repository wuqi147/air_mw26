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

/* FILE NAME:  hal_sco_acl.c
 * PURPOSE:
 *  Implement ACL module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_acl.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_ACL_GLOBAL_CFG_DEFAULT_VALUE            (0x000001F0)
#define HAL_SCO_ACL_PORT_EN_DEFAULT_VALUE               (0)
#define HAL_SCO_ACL_GROUP_CFG_DEFAULT_VALUE             (0x00208080)
#define HAL_SCO_ACL_MEM_CFG_DEFAULT_VALUE               (0x80000000)
#define HAL_SCO_ACL_MEM_CFG_WDATA_DEFAULT_VALUE         (0)
#define HAL_SCO_ACL_MIB_CNT_CFG_DEFAULT_VALUE           (0)
#define HAL_SCO_ACL_UDF_TABLE_DEFAULT_VALUE             (0)
#define HAL_SCO_ACL_RMC_DEFAULT_VALUE                   (0)
#define HAL_SCO_ACL_TRTCM_DEFAULT_VALUE                 (0)
#define HAL_SCO_ACL_ROUTING_ADDRESS_TABLE_DEFAULT_VALUE (0)
#define HAL_SCO_ACL_DPCR_DEFAULT_VALUE                  (0)

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data_buffer, offset, width, dst)           \
    ({                                                                                      \
        AIR_ERROR_NO_T __rc = 0;                                                            \
        UI32_T         value;                                                               \
        __rc = _hal_deriveTblMultiFields((unit), (data_buffer), (offset), (width), &value); \
        if (AIR_E_OK == __rc)                                                               \
        {                                                                                   \
            (dst) = value;                                                                  \
        }                                                                                   \
        __rc;                                                                               \
    })

/* DATA TYPE DECLARATIONS
 */
#define HAL_SCO_ACL_COUNTER_NUM_WORDS AIR_BITMAP_SIZE(HAL_SCO_MAX_NUM_OF_MIB_ID)

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_ACL, "hal_sco_acl.c");

/* FUNCTION NAME: _checkAclDone
 * PURPOSE:
 *      Check done bit of ACL
 * INPUT:
 *      unit            --  unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkAclDone(
    const UI32_T unit)
{
    UI32_T i;
    UI32_T done;

    /* Check done bit is 1 */
    for (i = 0; i < HAL_SCO_ACL_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ACL_MEM_CFG, &done, sizeof(UI32_T));
        if (1 == (done >> ACL_MEM_CFG_DONE_OFFSET))
        {
            break;
        }
        osal_delayUs(10);
    }
    if (i >= HAL_SCO_ACL_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _checkAclUdfDone
 * PURPOSE:
 *      Check access bit of UDF ACL
 * INPUT:
 *      unit            --  unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkAclUdfDone(
    const UI32_T unit)
{
    UI32_T i;
    UI32_T access;

    /* Check access bit is 0 */
    for (i = 0; i < HAL_SCO_ACL_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ACL_AUTC, &access, sizeof(UI32_T));
        if (0 == (access >> ACL_UDF_ACC_OFFSET))
        {
            break;
        }
        osal_delayUs(10);
    }
    if (i >= HAL_SCO_ACL_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _checkMeterBusy
 * PURPOSE:
 *      Check busy bit of Meter
 * INPUT:
 *      unit            --  unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkMeterBusy(
    const UI32_T unit)
{
    UI32_T i;
    UI32_T busy;

    /* Check busy bit is 0 */
    for (i = 0; i < HAL_SCO_ACL_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ACLRMC, &busy, sizeof(UI32_T));
        if (0 == (busy >> ACL_RATE_BUSY_OFFSET))
        {
            break;
        }
        osal_delayUs(10);
    }
    if (i >= HAL_SCO_ACL_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _checkTrtcmBusy
 * PURPOSE:
 *      Check busy bit of Trtcm
 * INPUT:
 *      unit            --  unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkTrtcmBusy(
    const UI32_T unit)
{
    UI32_T i;
    UI32_T busy;

    /* Check busy bit is 0 */
    for (i = 0; i < HAL_SCO_ACL_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ACL_TRTCMA, &busy, sizeof(UI32_T));
        if (0 == (busy >> ACL_TRTCM_BUSY_OFFSET))
        {
            break;
        }
        osal_delayUs(10);
    }
    if (i >= HAL_SCO_ACL_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      _hal_fillTblMultiFields
 * PURPOSE:
 *      Fill multi fields in table.
 * INPUT:
 *      unit            -- unit id
 *      offset          -- field offset in table
 *      data_count      -- buffer size
 *      width           -- field width
 *      value           -- field data to fill in table
 * OUTPUT:
 *      data_buffer[]   -- store table data
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
_hal_fillTblMultiFields(
    const UI32_T unit,
    UI32_T       data_buffer[/*data_count*/],
    UI32_T       data_count,
    const UI32_T offset,
    const UI32_T width,
    const UI32_T value)
{
    UI32_T data_index, bit_index;
    UI32_T extended_data[2];
    UI32_T extended_mask[2];
    UI32_T msk;
    UI32_T val;

    HAL_CHECK_PTR(data_buffer);

    if ((0 == data_count) || (0 == width) || (width > 32) || (offset + width > data_count * 32))
    {
        return AIR_E_BAD_PARAMETER;
    }

    msk = ((1U << (width - 1U)) << 1U) - 1U;
    val = value & msk;
    data_index = offset / 32;
    bit_index = offset % 32;

    extended_data[0] = val << bit_index;
    extended_data[1] = (val >> (31U - bit_index)) >> 1U;
    extended_mask[0] = msk << bit_index;
    extended_mask[1] = (msk >> (31U - bit_index)) >> 1U;

    data_buffer[data_index] = (data_buffer[data_index] & ~extended_mask[0]) | extended_data[0];
    if ((data_index + 1) < data_count)
    {
        data_buffer[data_index + 1] = (data_buffer[data_index + 1] & ~extended_mask[1]) | extended_data[1];
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      _hal_deriveTblMultiFields
 * PURPOSE:
 *      Derive multi fields from table.
 * INPUT:
 *      unit            -- unit id
 *      data_buffer     -- store table data
 *      offset          -- field offset in table
 *      width           -- field width
 * OUTPUT:
 *      ptr_value       -- pointer buffer of fields
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
_hal_deriveTblMultiFields(
    const UI32_T unit,
    UI32_T       data_buffer[],
    const UI32_T offset,
    const UI32_T width,
    UI32_T      *ptr_value)
{
    UI32_T data_index, bit_index;
    UI32_T extended_data[2];
    UI32_T extended_mask[2];
    UI32_T msk;

    HAL_CHECK_PTR(data_buffer);
    HAL_CHECK_PTR(ptr_value);

    if (width == 0 || width > 32)
    {
        return AIR_E_BAD_PARAMETER;
    }
    msk = ((1U << (width - 1U)) << 1U) - 1U;
    data_index = offset / 32;
    bit_index = offset % 32;

    extended_mask[0] = msk << bit_index;
    extended_mask[1] = (msk >> (31U - bit_index)) >> 1U;
    extended_data[0] = (data_buffer[data_index] & extended_mask[0]) >> bit_index;
    extended_data[1] = ((data_buffer[data_index + 1] & extended_mask[1]) << (31U - bit_index)) << 1U;

    *ptr_value = extended_data[0] | extended_data[1];

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_sco_acl_writeReg(
    const UI32_T                unit,
    const UI32_T                rule_idx,
    const UI32_T                block_num,
    const HAL_SCO_ACL_MEM_SEL_T sel,
    const UI32_T               *data)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         bn = 0, value = 0;

    if (sel >= HAL_SCO_ACL_MEM_SEL_LAST)
    {
        return AIR_E_BAD_PARAMETER;
    }

    for (bn = 0; bn < block_num; bn++)
    {
        if (AIR_E_TIMEOUT == _checkAclDone(unit))
        {
            return AIR_E_TIMEOUT;
        }
        ret = aml_writeReg(unit, ACL_MEM_CFG_WDATA0, data + bn * 4, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_writeReg(unit, ACL_MEM_CFG_WDATA1, data + bn * 4 + 1, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_writeReg(unit, ACL_MEM_CFG_WDATA2, data + bn * 4 + 2, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_writeReg(unit, ACL_MEM_CFG_WDATA3, data + bn * 4 + 3, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }

        value = (rule_idx << ACL_MEM_CFG_RULE_ID_OFFSET) | (bn << ACL_MEM_CFG_DATA_BN_OFFSET) |
                (sel << ACL_MEM_CFG_MEM_SEL_OFFSET) | ACL_MEM_CFG_WRITE | ACL_MEM_CFG_EN;
        if ((ret = aml_writeReg(unit, ACL_MEM_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    return ret;
}

static AIR_ERROR_NO_T
_hal_sco_acl_readReg(
    const UI32_T                unit,
    const UI32_T                rule_idx,
    const UI32_T                block_num,
    const HAL_SCO_ACL_MEM_SEL_T sel,
    UI32_T                     *data)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         bn = 0, value = 0;

    if (sel >= HAL_SCO_ACL_MEM_SEL_LAST)
    {
        return AIR_E_BAD_PARAMETER;
    }
    for (bn = 0; bn < block_num; bn++)
    {
        value = (rule_idx << ACL_MEM_CFG_RULE_ID_OFFSET) | (bn << ACL_MEM_CFG_DATA_BN_OFFSET) |
                (sel << ACL_MEM_CFG_MEM_SEL_OFFSET) | ACL_MEM_CFG_READ | ACL_MEM_CFG_EN;
        if ((ret = aml_writeReg(unit, ACL_MEM_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }

        if (AIR_E_TIMEOUT == _checkAclDone(unit))
        {
            return AIR_E_TIMEOUT;
        }
        ret = aml_readReg(unit, ACL_MEM_CFG_RDATA0, data + bn * 4, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_readReg(unit, ACL_MEM_CFG_RDATA1, data + bn * 4 + 1, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_readReg(unit, ACL_MEM_CFG_RDATA2, data + bn * 4 + 2, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        ret = aml_readReg(unit, ACL_MEM_CFG_RDATA3, data + bn * 4 + 3, sizeof(UI32_T));
        if (AIR_E_OK != ret)
        {
            return ret;
        }
        DIAG_PRINT(HAL_DBG_INFO, "===== get block %d=======\n", bn);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data%d: 0x%08x\n", bn * 4, data[bn * 4]);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data%d: 0x%08x\n", bn * 4 + 1, data[bn * 4 + 1]);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data%d: 0x%08x\n", bn * 4 + 2, data[bn * 4 + 2]);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data%d: 0x%08x\n", bn * 4 + 3, data[bn * 4 + 3]);
    }

    return ret;
}

static AIR_ERROR_NO_T
_hal_sco_acl_setRuleTable(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T    ret;
    I32_T             i, n;
    UI32_T            data[16];
    UI32_T            last_valid = 0;
    AIR_PORT_BITMAP_T mac_portmap;

    ret = _hal_sco_acl_readReg(unit, entry_idx, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_rule->portmap, mac_portmap);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FIELD_ENABLE_OFFSET, FIELD_ENABLE_WIDTH, last_valid);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:last_valid:   %d \n", last_valid);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:current_valid:%d \n", ptr_rule->field_valid);
    for (i = AIR_ACL_DMAC_KEY; i < AIR_ACL_RULE_FIELD_LAST; i++)
    {
        if ((1U << i) & ptr_rule->field_valid)
        {
            switch (i)
            {
                case AIR_ACL_DMAC_KEY:
                    for (n = 0; n < 6; n++)
                    {
                        _hal_fillTblMultiFields(unit, data, 16, DMAC_KEY_OFFSET + DMAC_KEY_WIDTH * (5 - n),
                                                DMAC_KEY_WIDTH, ptr_rule->key.dmac[n]);
                    }
                    _hal_fillTblMultiFields(unit, data, 16, DMAC_BYTE_MASK_OFFSET, DMAC_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.dmac);
                    break;
                case AIR_ACL_SMAC_KEY:
                    for (n = 0; n < 6; n++)
                    {
                        _hal_fillTblMultiFields(unit, data, 16, SMAC_KEY_OFFSET + SMAC_KEY_WIDTH * (5 - n),
                                                SMAC_KEY_WIDTH, ptr_rule->key.smac[n]);
                    }
                    _hal_fillTblMultiFields(unit, data, 16, SMAC_BYTE_MASK_OFFSET, SMAC_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.smac);
                    break;
                case AIR_ACL_ETYPE_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, ETYPE_KEY_OFFSET, ETYPE_KEY_WIDTH, ptr_rule->key.etype);
                    _hal_fillTblMultiFields(unit, data, 16, ETYPE_BYTE_MASK_OFFSET, ETYPE_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.etype);
                    break;
                case AIR_ACL_STAG_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, STAG_KEY_OFFSET, STAG_KEY_WIDTH, ptr_rule->key.stag);
                    _hal_fillTblMultiFields(unit, data, 16, STAG_BYTE_MASK_OFFSET, STAG_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.stag);
                    break;
                case AIR_ACL_CTAG_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, CTAG_KEY_OFFSET, CTAG_KEY_WIDTH, ptr_rule->key.ctag);
                    _hal_fillTblMultiFields(unit, data, 16, CTAG_BYTE_MASK_OFFSET, CTAG_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.ctag);
                    break;
                case AIR_ACL_DIP_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, DIP_KEY_OFFSET, DIP_KEY_WIDTH, ptr_rule->key.dip);
                    _hal_fillTblMultiFields(unit, data, 16, DIP_BYTE_MASK_OFFSET, DIP_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.dip);
                    break;
                case AIR_ACL_SIP_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, SIP_KEY_OFFSET, SIP_KEY_WIDTH, ptr_rule->key.sip);
                    _hal_fillTblMultiFields(unit, data, 16, SIP_BYTE_MASK_OFFSET, SIP_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.sip);
                    break;
                case AIR_ACL_DSCP_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, DSCP_KEY_OFFSET, DSCP_KEY_WIDTH, ptr_rule->key.dscp);
                    break;
                case AIR_ACL_NEXT_HEADER_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, NEXT_HEADER_KEY_OFFSET, NEXT_HEADER_KEY_WIDTH,
                                            ptr_rule->key.next_header);
                    break;
                case AIR_ACL_FLOW_LABEL_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, FLOW_LABEL_KEY_OFFSET, FLOW_LABEL_KEY_WIDTH,
                                            ptr_rule->key.flow_label);
                    _hal_fillTblMultiFields(unit, data, 16, FLOW_LABEL_BYTE_MASK_OFFSET, FLOW_LABEL_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.flow_label);
                    break;
                case AIR_ACL_DPORT_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, DPORT_KEY_OFFSET, DPORT_KEY_WIDTH, ptr_rule->key.dport);
                    _hal_fillTblMultiFields(unit, data, 16, DPORT_BYTE_MASK_OFFSET, DPORT_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.dport);
                    break;
                case AIR_ACL_SPORT_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, SPORT_KEY_OFFSET, SPORT_KEY_WIDTH, ptr_rule->key.sport);
                    _hal_fillTblMultiFields(unit, data, 16, SPORT_BYTE_MASK_OFFSET, SPORT_BYTE_MASK_WIDTH,
                                            ptr_rule->mask.sport);
                    break;
                case AIR_ACL_UDF_KEY:
                    _hal_fillTblMultiFields(unit, data, 16, UDF_KEY_OFFSET, UDF_KEY_WIDTH, ptr_rule->key.udf);
                    _hal_fillTblMultiFields(unit, data, 16, UDF_BIT_MASK_OFFSET, UDF_BIT_MASK_WIDTH,
                                            ptr_rule->mask.udf);
                    break;
                default:
                    return AIR_E_BAD_PARAMETER;
            }
        }
    }
    _hal_fillTblMultiFields(unit, data, 16, FIELD_TYPE_OFFSET, FIELD_TYPE_WIDTH, ptr_rule->field_type);
    _hal_fillTblMultiFields(unit, data, 16, IS_IPV6_RULE_OFFSET, IS_IPV6_RULE_WIDTH, ptr_rule->isipv6);
    _hal_fillTblMultiFields(unit, data, 16, RULE_END_OFFSET, RULE_END_WIDTH, ptr_rule->end);
    _hal_fillTblMultiFields(unit, data, 16, RULE_ENABLE_OFFSET, RULE_ENABLE_WIDTH, ptr_rule->rule_en);
    _hal_fillTblMultiFields(unit, data, 16, REVERSE_BIT_OFFSET, REVERSE_BIT_WIDTH, ptr_rule->reverse);
    _hal_fillTblMultiFields(unit, data, 16, PORTMAP_OFFSET, PORTMAP_WIDTH, mac_portmap[0]);
    _hal_fillTblMultiFields(unit, data, 16, FIELD_ENABLE_OFFSET, FIELD_ENABLE_WIDTH,
                            (last_valid | ptr_rule->field_valid));

    DIAG_PRINT(HAL_DBG_INFO, "========acl rule=======\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data0: 0x%08x\n", data[0]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data1: 0x%08x\n", data[1]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data2: 0x%08x\n", data[2]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data3: 0x%08x\n", data[3]);
    DIAG_PRINT(HAL_DBG_INFO, "=======================\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data4: 0x%08x\n", data[4]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data5: 0x%08x\n", data[5]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data6: 0x%08x\n", data[6]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data7: 0x%08x\n", data[7]);
    DIAG_PRINT(HAL_DBG_INFO, "=======================\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data8: 0x%08x\n", data[8]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data9: 0x%08x\n", data[9]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data10:0x%08x\n", data[10]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data11:0x%08x\n", data[11]);
    DIAG_PRINT(HAL_DBG_INFO, "=======================\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data12:0x%08x\n", data[12]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data13:0x%08x\n", data[13]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data14:0x%08x\n", data[14]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data15:0x%08x\n", data[15]);

    return _hal_sco_acl_writeReg(unit, entry_idx, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
}

/* FUNCTION NAME: _hal_sco_acl_getRulesUsage
 * PURPOSE:
 *      Get the acl rules usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of acl rules
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getRulesUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         entry_idx, usage_count = 0;
    AIR_ACL_RULE_T rule;

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_ACL_ENTRY; entry_idx++)
    {
        ret = hal_sco_acl_getRule(unit, entry_idx, &rule);
        if ((AIR_E_OK == ret) && (TRUE == rule.rule_en))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_sco_acl_getActionUsage
 * PURPOSE:
 *      Get the acl action usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of acl actions
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getActionUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T   ret = AIR_E_OK;
    UI32_T           entry_idx, usage_count = 0;
    AIR_ACL_ACTION_T action;

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_ACL_ENTRY; entry_idx++)
    {
        ret = hal_sco_acl_getAction(unit, entry_idx, &action);
        if ((AIR_E_OK == ret) && (action.field_valid > 0))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_sco_acl_getUdfEntryUsage
 * PURPOSE:
 *      Get the acl udf entries usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of udf entries
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getUdfUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T     ret = AIR_E_OK;
    UI32_T             entry_idx, usage_count = 0;
    AIR_ACL_UDF_RULE_T udf_rule;

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_UDF_ENTRY; entry_idx++)
    {
        ret = hal_sco_acl_getUdfRule(unit, entry_idx, &udf_rule);
        if ((AIR_E_OK == ret) && (TRUE == udf_rule.udf_rule_en))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_sco_acl_getCounterUsage
 * PURPOSE:
 *      Get the acl mib counters usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of mib counters
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getCounterUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T   ret = AIR_E_OK;
    UI32_T           entry_idx;
    AIR_ACL_ACTION_T action;
    UI32_T           mib_bmp[HAL_SCO_ACL_COUNTER_NUM_WORDS] = {0};

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_ACL_ENTRY; entry_idx++)
    {
        ret = hal_sco_acl_getAction(unit, entry_idx, &action);
        if ((AIR_E_OK == ret) && BIT_CHK(action.field_valid, AIR_ACL_MIB))
        {
            CMLIB_BITMAP_BIT_ADD(mib_bmp, action.acl_mib_id);
        }
    }
    CMLIB_BITMAP_COUNT(mib_bmp, *ptr_cnt, HAL_SCO_ACL_COUNTER_NUM_WORDS);
}

/* FUNCTION NAME: _hal_sco_acl_getMeterUsage
 * PURPOSE:
 *      Get the acl mib counters usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of meters
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getMeterUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         entry_idx, usage_count = 0;
    BOOL_T         enable;
    UI32_T         rate = 0;

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_METER_ID; entry_idx++)
    {
        ret = hal_sco_acl_getMeterTable(unit, entry_idx, &enable, &rate);
        if ((AIR_E_OK == ret) && (TRUE == enable))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_sco_acl_getTrtcmUsage
 * PURPOSE:
 *      Get the acl Trtcm entry usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of trtcm entries
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_acl_getTrtcmUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T  ret = AIR_E_OK;
    UI32_T          entry_idx, usage_count = 0;
    AIR_ACL_TRTCM_T tcm;

    for (entry_idx = 0; entry_idx < HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY; entry_idx++)
    {
        ret = hal_sco_acl_getTrtcmTable(unit, entry_idx, &tcm);
        if ((AIR_E_OK == ret) && (tcm.cbs != 0) && (tcm.pbs != 0) && (tcm.cir != 0) && (tcm.pir != 0))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:
 *      hal_sco_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      ptr_enable       -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getGlobalState(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    I32_T  ret;
    UI32_T value;

    /* get value from CHIP*/
    if ((ret = aml_readReg(unit, ACL_GLOBAL_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    value &= ACL_EN_MASK;
    *ptr_enable = value ? TRUE : FALSE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit        -- unit id
 *      enable      -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setGlobalState(
    const UI32_T unit,
    const BOOL_T enable)
{
    I32_T  ret;
    UI32_T value, data;

    value = enable ? 1 : 0;
    if ((ret = aml_readReg(unit, ACL_GLOBAL_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    data = (data & ~ACL_EN_MASK) | value;
    if ((ret = aml_writeReg(unit, ACL_GLOBAL_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getPortState
 * PURPOSE:
 *      Get enable status of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getPortState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    I32_T  ret;
    UI32_T value, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* get value from CHIP*/
    if ((ret = aml_readReg(unit, ACL_PORT_EN, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    value = (value >> mac_port) & 0x1;
    *ptr_enable = value ? TRUE : FALSE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setPortState
 * PURPOSE:
 *      Set enable state of ACL on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 *      enable          -- enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setPortState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    I32_T  ret;
    UI32_T value, data, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    value = enable ? 1 : 0;
    if ((ret = aml_readReg(unit, ACL_PORT_EN, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    data = (data & ~(ACL_EN_MASK << mac_port)) | (value << mac_port);
    if ((ret = aml_writeReg(unit, ACL_PORT_EN, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_clearAll
 * PURPOSE:
 *      Clear ACL all rule and action HW memory.
 * INPUT:
 *      unit        -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_clearAll(
    const UI32_T unit)
{
    I32_T  ret;
    UI32_T value;

    value = ACL_MEM_CFG_CLEAR | ACL_MEM_CFG_EN;
    if ((ret = aml_writeReg(unit, ACL_MEM_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getRule
 * PURPOSE:
 *      Get ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      ptr_rule         -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T    ret;
    UI32_T            n, v6_idx, i;
    UI32_T            data[16];
    AIR_PORT_BITMAP_T air_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);

    ret = _hal_sco_acl_readReg(unit, entry_idx, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    for (n = 0; n < 6; n++)
    {
        HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DMAC_KEY_OFFSET + DMAC_KEY_WIDTH * (5 - n), DMAC_KEY_WIDTH,
                                           ptr_rule->key.dmac[n]);
    }
    for (n = 0; n < 6; n++)
    {
        HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SMAC_KEY_OFFSET + SMAC_KEY_WIDTH * (5 - n), SMAC_KEY_WIDTH,
                                           ptr_rule->key.smac[n]);
    }
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ETYPE_KEY_OFFSET, ETYPE_KEY_WIDTH, ptr_rule->key.etype);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, STAG_KEY_OFFSET, STAG_KEY_WIDTH, ptr_rule->key.stag);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CTAG_KEY_OFFSET, CTAG_KEY_WIDTH, ptr_rule->key.ctag);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DIP_KEY_OFFSET, DIP_KEY_WIDTH, ptr_rule->key.dip);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SIP_KEY_OFFSET, SIP_KEY_WIDTH, ptr_rule->key.sip);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DSCP_KEY_OFFSET, DSCP_KEY_WIDTH, ptr_rule->key.dscp);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, NEXT_HEADER_KEY_OFFSET, NEXT_HEADER_KEY_WIDTH,
                                       ptr_rule->key.next_header);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FLOW_LABEL_KEY_OFFSET, FLOW_LABEL_KEY_WIDTH,
                                       ptr_rule->key.flow_label);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DPORT_KEY_OFFSET, DPORT_KEY_WIDTH, ptr_rule->key.dport);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SPORT_KEY_OFFSET, SPORT_KEY_WIDTH, ptr_rule->key.sport);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, UDF_KEY_OFFSET, UDF_KEY_WIDTH, ptr_rule->key.udf);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DMAC_BYTE_MASK_OFFSET, DMAC_BYTE_MASK_WIDTH, ptr_rule->mask.dmac);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SMAC_BYTE_MASK_OFFSET, SMAC_BYTE_MASK_WIDTH, ptr_rule->mask.smac);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ETYPE_BYTE_MASK_OFFSET, ETYPE_BYTE_MASK_WIDTH, ptr_rule->mask.etype);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, STAG_BYTE_MASK_OFFSET, STAG_BYTE_MASK_WIDTH, ptr_rule->mask.stag);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CTAG_BYTE_MASK_OFFSET, CTAG_BYTE_MASK_WIDTH, ptr_rule->mask.ctag);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DIP_BYTE_MASK_OFFSET, DIP_BYTE_MASK_WIDTH, ptr_rule->mask.dip);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SIP_BYTE_MASK_OFFSET, SIP_BYTE_MASK_WIDTH, ptr_rule->mask.sip);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FLOW_LABEL_BYTE_MASK_OFFSET, FLOW_LABEL_BYTE_MASK_WIDTH,
                                       ptr_rule->mask.flow_label);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DPORT_BYTE_MASK_OFFSET, DPORT_BYTE_MASK_WIDTH, ptr_rule->mask.dport);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SPORT_BYTE_MASK_OFFSET, SPORT_BYTE_MASK_WIDTH, ptr_rule->mask.sport);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, UDF_BIT_MASK_OFFSET, UDF_BIT_MASK_WIDTH, ptr_rule->mask.udf);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FIELD_TYPE_OFFSET, FIELD_TYPE_WIDTH, ptr_rule->field_type);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, IS_IPV6_RULE_OFFSET, IS_IPV6_RULE_WIDTH, ptr_rule->isipv6);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, RULE_END_OFFSET, RULE_END_WIDTH, ptr_rule->end);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, RULE_ENABLE_OFFSET, RULE_ENABLE_WIDTH, ptr_rule->rule_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, REVERSE_BIT_OFFSET, REVERSE_BIT_WIDTH, ptr_rule->reverse);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PORTMAP_OFFSET, PORTMAP_WIDTH, ptr_rule->portmap[0]);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FIELD_ENABLE_OFFSET, FIELD_ENABLE_WIDTH, ptr_rule->field_valid);

    HAL_MAC_PBMP_TO_AIR_PBMP(unit, ptr_rule->portmap, air_portmap);
    AIR_PORT_BITMAP_COPY(ptr_rule->portmap, air_portmap);
    if ((TRUE == ptr_rule->isipv6) &&
        (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIP_KEY) || BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIP_KEY)))
    {
        /* IPV6 root entry */
        v6_idx = entry_idx - (entry_idx % HAL_SCO_ENTRY_NUM_PER_IPV6_RULE);

        if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIP_KEY))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY);
            BIT_DEL(ptr_rule->field_valid, AIR_ACL_DIP_KEY);
        }
        if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIP_KEY))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY);
            BIT_DEL(ptr_rule->field_valid, AIR_ACL_SIP_KEY);
        }
        /* Get DIPV6 or SIPV6 */
        for (n = 0; n < HAL_SCO_ENTRY_NUM_PER_IPV6_RULE; n++)
        {
            ret = _hal_sco_acl_readReg(unit, v6_idx + n, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
            if (AIR_E_OK != ret)
            {
                return ret;
            }
            if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY))
            {
                HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DIP_KEY_OFFSET, DIP_KEY_WIDTH, ptr_rule->key.dip);
                HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DIP_BYTE_MASK_OFFSET, DIP_BYTE_MASK_WIDTH,
                                                   ptr_rule->mask.dip);
                for (i = 0; i < 4; i++)
                {
                    ptr_rule->key.dipv6[12 - n * 4 + i] = (ptr_rule->key.dip >> (8 * (3 - i))) & 0xff;
                }
                ptr_rule->mask.dipv6 |= (ptr_rule->mask.dip << (4 * n));
            }

            if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY))
            {
                HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SIP_KEY_OFFSET, SIP_KEY_WIDTH, ptr_rule->key.sip);
                HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SIP_BYTE_MASK_OFFSET, SIP_BYTE_MASK_WIDTH,
                                                   ptr_rule->mask.sip);
                for (i = 0; i < 4; i++)
                {
                    ptr_rule->key.sipv6[12 - n * 4 + i] = (ptr_rule->key.sip >> (8 * (3 - i))) & 0xff;
                }
                ptr_rule->mask.sipv6 |= (ptr_rule->mask.sip << (4 * n));
            }
        }
    }
    return ret;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setRule
 * PURPOSE:
 *      Add/Update ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 *      ptr_rule         -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      1.If isipv6=1, dip&sip&dscp&next header will hit only when packet is IPv6;
 *      2.Every 4 rule can do aggregation. For rule aggregation, final hit id rule-end must be 1;
 *        When rule-end=0, means that it can be aggregated with next rule;
 *        Rule3 cannot aggregate with rule4, if rule3 end=0, rule3 will be an invalid rule;
 *      3.For reverse function use, do reverse before rule aggregation.
 *      4.When byte mask of each field is set 0, key of each field must be set 0;
 *      5.Udf-list is bit mask of udf-rule entry, range 1-16;
 */
AIR_ERROR_NO_T
hal_sco_acl_setRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T ret;
    UI8_T          v6_idx;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dmac, 0, HAL_SCO_MAX_NUM_OF_DMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.smac, 0, HAL_SCO_MAX_NUM_OF_SMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.etype, 0, HAL_SCO_MAX_NUM_OF_ETYPE_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.stag, 0, HAL_SCO_MAX_NUM_OF_STAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.ctag, 0, HAL_SCO_MAX_NUM_OF_CTAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.flow_label, 0, HAL_SCO_MAX_NUM_OF_FLOW_LABEL_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dip, 0, HAL_SCO_MAX_NUM_OF_DIP_MASK_IPV4);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sip, 0, HAL_SCO_MAX_NUM_OF_SIP_MASK_IPV4);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dport, 0, HAL_SCO_MAX_NUM_OF_DPORT_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sport, 0, HAL_SCO_MAX_NUM_OF_SPORT_MASK);
    HAL_CHECK_PORT_BITMAP(unit, ptr_rule->portmap);
    HAL_CHECK_BOOL(ptr_rule->isipv6);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dipv6, 0, HAL_SCO_MAX_NUM_OF_DIP_MASK_IPV6);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sipv6, 0, HAL_SCO_MAX_NUM_OF_SIP_MASK_IPV6);

    /* Fill the IPv6 value to ACL rule */
    if ((TRUE == ptr_rule->isipv6) &&
        (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY) || BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY)))
    {
        if (0 != (entry_idx % HAL_SCO_MAX_NUM_OF_BLOCK))
        {
            /*illegal rule id, must be multiples of 4*/
            return AIR_E_BAD_PARAMETER;
        }
        if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIP_KEY);
            BIT_DEL(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY);
        }
        if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIP_KEY);
            BIT_DEL(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY);
        }
        for (v6_idx = 0; v6_idx < sizeof(AIR_IPV6_T); v6_idx += 4)
        {
            if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIP_KEY))
            {
                /*
                    rule3.key.dip = (dip[0]<<24) | (dip[1]<<16) | (dip[2]<<8) | dip[3];
                    rule2.key.dip = (dip[4]<<24) | (dip[5]<<16) | (dip[6]<<8) | dip[7];
                    rule1.key.dip = (dip[8]<<24) | (dip[9]<<16) | (dip[10]<<8) | dip[11];
                    rule0.key.dip = (dip[12]<<24) | (dip[13]<<16) | (dip[14]<<8) | dip[15];
                    rule0.mask.dip = dmask & 0xf;
                    rule1.mask.dip = (dmask >> 4) & 0xf;
                    rule2.mask.dip = (dmask >> 8) & 0xf;
                    rule3.mask.dip = (dmask >> 12) & 0xf;
                */
                ptr_rule->key.dip = (ptr_rule->key.dipv6[12 - v6_idx] << 24) |
                                    (ptr_rule->key.dipv6[13 - v6_idx] << 16) | (ptr_rule->key.dipv6[14 - v6_idx] << 8) |
                                    ptr_rule->key.dipv6[15 - v6_idx];
                ptr_rule->mask.dip = (ptr_rule->mask.dipv6 >> v6_idx) & 0xf;
            }

            if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIP_KEY))
            {
                /*
                    rule3.key.sip = (sip[0]<<24) | (sip[1]<<16) | (sip[2]<<8) | sip[3];
                    rule2.key.sip = (sip[4]<<24) | (sip[5]<<16) | (sip[6]<<8) | sip[7];
                    rule1.key.sip = (sip[8]<<24) | (sip[9]<<16) | (sip[10]<<8) | sip[11];
                    rule0.key.sip = (sip[12]<<24) | (sip[13]<<16) | (sip[14]<<8) | sip[15];
                    rule0.mask.sip = smask & 0xf;
                    rule1.mask.sip = (smask >> 4) & 0xf;
                    rule2.mask.sip = (smask >> 8) & 0xf;
                    rule3.mask.sip = (smask >> 12) & 0xf;
                */
                ptr_rule->key.sip = (ptr_rule->key.sipv6[12 - v6_idx] << 24) |
                                    (ptr_rule->key.sipv6[13 - v6_idx] << 16) | (ptr_rule->key.sipv6[14 - v6_idx] << 8) |
                                    ptr_rule->key.sipv6[15 - v6_idx];
                ptr_rule->mask.sip = (ptr_rule->mask.sipv6 >> v6_idx) & 0xf;
            }
            /* set the rule */
            if (0 != v6_idx)
            {
                ptr_rule->end = FALSE;
            }
            ret = _hal_sco_acl_setRuleTable(unit, entry_idx + (v6_idx / 4), ptr_rule);
            if (AIR_E_OK != ret)
            {
                break;
            }
        }
    }
    else
    {
        if (((HAL_SCO_MAX_NUM_OF_BLOCK - 1) == (entry_idx % HAL_SCO_MAX_NUM_OF_BLOCK)) && (FALSE == ptr_rule->end))
        {
            /*rules in different blocks cannot be aggregate*/
            return AIR_E_BAD_PARAMETER;
        }
        ret = _hal_sco_acl_setRuleTable(unit, entry_idx, ptr_rule);
    }

    return ret;
}

/* FUNCTION NAME:
 *      hal_sco_acl_delRule
 * PURPOSE:
 *      Delete ACL rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delRule(
    const UI32_T unit,
    const UI32_T entry_idx)
{
    AIR_ERROR_NO_T ret;
    UI32_T         data[16] = {0};
    UI32_T         n, v6_idx;
    AIR_ACL_RULE_T rule;
    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);

    ret = _hal_sco_acl_readReg(unit, entry_idx, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }
    osal_memset(&rule, 0x0, sizeof(AIR_ACL_RULE_T));
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, IS_IPV6_RULE_OFFSET, IS_IPV6_RULE_WIDTH, rule.isipv6);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FIELD_ENABLE_OFFSET, FIELD_ENABLE_WIDTH, rule.field_valid);

    if ((TRUE == rule.isipv6) &&
        (BIT_CHK(rule.field_valid, AIR_ACL_DIP_KEY) || BIT_CHK(rule.field_valid, AIR_ACL_SIP_KEY)))
    {
        /* IPV6 root entry */
        v6_idx = entry_idx - (entry_idx % HAL_SCO_ENTRY_NUM_PER_IPV6_RULE);
        /* Delete DIPV6 or SIPV6 rules */
        osal_memset(data, 0x0, sizeof(UI32_T) * 16);
        for (n = 0; n < HAL_SCO_ENTRY_NUM_PER_IPV6_RULE; n++)
        {
            ret = _hal_sco_acl_writeReg(unit, v6_idx + n, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
            if (AIR_E_OK != ret)
            {
                break;
            }
        }
    }
    else
    {
        osal_memset(data, 0x0, sizeof(UI32_T) * 16);
        ret = _hal_sco_acl_writeReg(unit, entry_idx, 4, HAL_SCO_ACL_MEM_SEL_RULE, data);
    }

    return ret;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getAction
 * PURPOSE:
 *      Get ACL action of specified entry index.
 * INPUT:
 *      unit              -- unit id
 *      entry_idx         -- ACL action entry index
 * OUTPUT:
 *      ptr_action        -- pointer buffer of action
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action)
{
    AIR_ERROR_NO_T    ret;
    UI32_T            data[4];
    AIR_ACL_FIELD_T   field;
    AIR_PORT_BITMAP_T air_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);

    osal_memset(&field, 0, sizeof(AIR_ACL_FIELD_T));

    ret = _hal_sco_acl_readReg(unit, entry_idx, 1, HAL_SCO_ACL_MEM_SEL_ACTION, data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, field.port_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PORT_OFFSET, PORT_WIDTH, ptr_action->portmap[0]);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH,
                                       ptr_action->vlan_port_swap);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH,
                                       ptr_action->dst_port_swap);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, field.mib_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->acl_mib_id);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, field.attack_rate_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH,
                                       ptr_action->attack_rate_id);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, RATE_EN_OFFSET, RATE_EN_WIDTH, field.rate_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->meter_id);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, field.port_fw_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->port_fw);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirror_port);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, field.pri_user_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, field.eg_tag_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->eg_tag);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, field.lky_vlan_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lky_vlan);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->acl_mang);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, field.trtcm_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH, ptr_action->drop_pcd_sel);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH, ptr_action->drop_pcd_r);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH, ptr_action->drop_pcd_y);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH, ptr_action->drop_pcd_g);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH,
                                       ptr_action->class_slr_sel);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->class_slr);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH, ptr_action->acl_tcm_sel);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->def_color);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH, ptr_action->trtcm_id);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, field.vlan_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH, ptr_action->acl_vlan_vid);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, MAC_CHG_OFFSET, MAC_CHG_WIDTH, field.mac_chg);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, DA_INDEX_OFFSET, DA_INDEX_WIDTH, ptr_action->da_index);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, SA_INDEX_OFFSET, SA_INDEX_WIDTH, ptr_action->sa_index);
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, ptr_action->portmap, air_portmap);
    AIR_PORT_BITMAP_COPY(ptr_action->portmap, air_portmap);

    field.mirror_en = ptr_action->mirror_port ? 1 : 0;
    field.mang_en = ptr_action->acl_mang ? 1 : 0;
    if (1 == field.trtcm_en)
    {
        field.drop_pcd_en = 1;
        field.class_slr_en = 1;
        field.trtcm_idx_en = 1;
    }
    ptr_action->field_valid =
        (field.port_en << AIR_ACL_PORT) | (field.mib_en << AIR_ACL_MIB) | (field.attack_rate_en << AIR_ACL_ATTACK) |
        (field.rate_en << AIR_ACL_METER) | (field.port_fw_en << AIR_ACL_FW_PORT) | (field.mirror_en << AIR_ACL_MIRROR) |
        (field.pri_user_en << AIR_ACL_PRI) | (field.eg_tag_en << AIR_ACL_EGTAG) |
        (field.lky_vlan_en << AIR_ACL_LKY_VLAN) | (field.mang_en << AIR_ACL_MANG) |
        (field.drop_pcd_en << AIR_ACL_DROP_PCD) | (field.class_slr_en << AIR_ACL_CLASS_SLR) |
        (field.trtcm_idx_en << AIR_ACL_TRTCM) | (field.vlan_en << AIR_ACL_VLAN) | (field.mac_chg << AIR_ACL_MAC_CHG);

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setAction
 * PURPOSE:
 *      Add/Update ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 *      ptr_action       -- pointer buffer of action
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If acl-tcm=defined, the corresponding action is defined-color;
 *      If acl-tcm=meter, the corresponding action is meter-id;
 */
AIR_ERROR_NO_T
hal_sco_acl_setAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action)
{
    I32_T             ret;
    UI32_T            data[4];
    I32_T             i;
    AIR_PORT_BITMAP_T mac_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->acl_mib_id, 0, HAL_SCO_MAX_NUM_OF_MIB_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->attack_rate_id, 0, HAL_SCO_MAX_NUM_OF_ATTACK_RATE_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->meter_id, 0, HAL_SCO_MAX_NUM_OF_METER_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->mirror_port, 0, HAL_SCO_MAX_NUM_OF_MIRROR_PORT);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->pri_user, 0, HAL_SCO_MAX_NUM_OF_USER_PRI - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_r, 0, HAL_SCO_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_y, 0, HAL_SCO_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_g, 0, HAL_SCO_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->class_slr, 0, HAL_SCO_MAX_NUM_OF_CLASS_SLR - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->trtcm_id, 0, HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->acl_vlan_vid, 0, HAL_SCO_MAX_NUM_OF_VLAN_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->port_fw, 0, BITS_RANGE(0, FW_PORT_WIDTH));
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->eg_tag, 0, BITS_RANGE(0, EG_TAG_WIDTH));
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->def_color, 0, BITS_RANGE(0, ACL_TCM_WIDTH));
    HAL_CHECK_PORT_BITMAP(unit, ptr_action->portmap);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_action->portmap, mac_portmap);

    ret = _hal_sco_acl_readReg(unit, entry_idx, 1, HAL_SCO_ACL_MEM_SEL_ACTION, data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    for (i = AIR_ACL_PORT; i < AIR_ACL_ACTION_FIELD_LAST; i++)
    {
        if ((1U << i) & ptr_action->field_valid)
        {
            switch (i)
            {
                case AIR_ACL_PORT:
                    _hal_fillTblMultiFields(unit, data, 4, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, PORT_OFFSET, PORT_WIDTH, mac_portmap[0]);
                    _hal_fillTblMultiFields(unit, data, 4, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH,
                                            ptr_action->vlan_port_swap);
                    _hal_fillTblMultiFields(unit, data, 4, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH,
                                            ptr_action->dst_port_swap);
                    break;
                case AIR_ACL_MIB:
                    _hal_fillTblMultiFields(unit, data, 4, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->acl_mib_id);
                    break;
                case AIR_ACL_ATTACK:
                    _hal_fillTblMultiFields(unit, data, 4, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH,
                                            ptr_action->attack_rate_id);
                    break;
                case AIR_ACL_METER:
                    _hal_fillTblMultiFields(unit, data, 4, RATE_EN_OFFSET, RATE_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->meter_id);
                    break;
                case AIR_ACL_FW_PORT:
                    _hal_fillTblMultiFields(unit, data, 4, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->port_fw);
                    break;
                case AIR_ACL_MIRROR:
                    _hal_fillTblMultiFields(unit, data, 4, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirror_port);
                    break;
                case AIR_ACL_PRI:
                    _hal_fillTblMultiFields(unit, data, 4, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);
                    break;
                case AIR_ACL_EGTAG:
                    _hal_fillTblMultiFields(unit, data, 4, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->eg_tag);
                    break;
                case AIR_ACL_LKY_VLAN:
                    _hal_fillTblMultiFields(unit, data, 4, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lky_vlan);
                    break;
                case AIR_ACL_MANG:
                    _hal_fillTblMultiFields(unit, data, 4, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->acl_mang);
                    break;
                case AIR_ACL_DROP_PCD:
                    _hal_fillTblMultiFields(unit, data, 4, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH,
                                            ptr_action->drop_pcd_sel);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH,
                                            ptr_action->drop_pcd_r);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH,
                                            ptr_action->drop_pcd_y);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH,
                                            ptr_action->drop_pcd_g);
                    break;
                case AIR_ACL_CLASS_SLR:
                    _hal_fillTblMultiFields(unit, data, 4, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH,
                                            ptr_action->class_slr_sel);
                    _hal_fillTblMultiFields(unit, data, 4, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->class_slr);
                    break;
                case AIR_ACL_TRTCM:
                    _hal_fillTblMultiFields(unit, data, 4, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH,
                                            ptr_action->acl_tcm_sel);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->def_color);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH,
                                            ptr_action->trtcm_id);
                    break;
                case AIR_ACL_VLAN:
                    _hal_fillTblMultiFields(unit, data, 4, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH,
                                            ptr_action->acl_vlan_vid);
                    break;
                case AIR_ACL_MAC_CHG:
                    _hal_fillTblMultiFields(unit, data, 4, MAC_CHG_OFFSET, MAC_CHG_WIDTH, 1);
                    _hal_fillTblMultiFields(unit, data, 4, DA_INDEX_OFFSET, DA_INDEX_WIDTH, ptr_action->da_index);
                    _hal_fillTblMultiFields(unit, data, 4, SA_INDEX_OFFSET, SA_INDEX_WIDTH, ptr_action->sa_index);
                    break;
                case AIR_ACL_DONT_LEARN:
                case AIR_ACL_BPDU:
                    return AIR_E_NOT_SUPPORT;
                default:
                    return AIR_E_BAD_PARAMETER;
            }
        }
    }
    DIAG_PRINT(HAL_DBG_INFO, "========acl action=======\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data0: 0x%08x\n", data[0]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data1: 0x%08x\n", data[1]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data2: 0x%08x\n", data[2]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data3: 0x%08x\n", data[3]);

    ret = _hal_sco_acl_writeReg(unit, entry_idx, 1, HAL_SCO_ACL_MEM_SEL_ACTION, data);

    return ret;
}

/* FUNCTION NAME:
 *      hal_sco_acl_delAction
 * PURPOSE:
 *      Delete ACL action of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL action entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delAction(
    const UI32_T unit,
    const UI32_T entry_idx)
{
    UI32_T data[4] = {0};

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_ACL_ENTRY - 1);

    return _hal_sco_acl_writeReg(unit, entry_idx, 1, HAL_SCO_ACL_MEM_SEL_ACTION, data);
}

/* FUNCTION NAME:
 *      hal_sco_acl_getBlockState
 * PURPOSE:
 *      Get ACL block combine state.
 * INPUT:
 *      unit             -- unit id
 *      block            -- block id
 * OUTPUT:
 *      ptr_enable       -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by setting block combination state;
 *      When combination=enable, means this block can combine with next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
hal_sco_acl_getBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    BOOL_T               *ptr_enable)
{
    I32_T  ret;
    UI32_T value, data;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(block, AIR_ACL_BLOCK_3);

    if ((ret = aml_readReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    if (block == AIR_ACL_BLOCK_0)
    {
        value = (data >> ACL_ACT_GROUP_END_0_OFFSET) & 0x1;
    }
    else if (block == AIR_ACL_BLOCK_1)
    {
        value = (data >> ACL_ACT_GROUP_END_1_OFFSET) & 0x1;
    }
    else if (block == AIR_ACL_BLOCK_2)
    {
        value = (data >> ACL_ACT_GROUP_END_2_OFFSET) & 0x1;
    }
    *ptr_enable = value ? FALSE : TRUE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setBlockState
 * PURPOSE:
 *      Set block combination state.
 * INPUT:
 *      unit             -- unit id
 *      block            -- block id
 *      enable           -- enable state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has 4 blocks, block can aggregate to group by setting block combination state;
 *      When combination=enable, means this block can combine with next block;
 *      In one group, first hit rule has highest priority.
 */
AIR_ERROR_NO_T
hal_sco_acl_setBlockState(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const BOOL_T          enable)
{
    I32_T  ret;
    UI32_T value, data;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(block, AIR_ACL_BLOCK_3);

    value = enable ? 0 : 1;
    if ((ret = aml_readReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    if (block == AIR_ACL_BLOCK_0)
    {
        data = (data & ~(1U << ACL_ACT_GROUP_END_0_OFFSET)) | (value << ACL_ACT_GROUP_END_0_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    else if (block == AIR_ACL_BLOCK_1)
    {
        data = (data & ~(1U << ACL_ACT_GROUP_END_1_OFFSET)) | (value << ACL_ACT_GROUP_END_1_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    else if (block == AIR_ACL_BLOCK_2)
    {
        data = (data & ~(1U << ACL_ACT_GROUP_END_2_OFFSET)) | (value << ACL_ACT_GROUP_END_2_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getLastLine
 * PURPOSE:
 *      Get specified block last ACL rule row number.
 * INPUT:
 *      unit                -- unit id
 *      block               -- block id
 * OUTPUT:
 *      ptr_last_line       -- last rule row number of specified block
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Each block has total 128 ACL rules, and they will be split as 4 banks, each bank has 32 rows.
 */
AIR_ERROR_NO_T
hal_sco_acl_getLastLine(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    UI32_T               *ptr_last_line)
{
    I32_T  ret;
    UI32_T data;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(block, AIR_ACL_BLOCK_LAST);

    if ((ret = aml_readReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    if (block == AIR_ACL_BLOCK_0)
    {
        *ptr_last_line = data & ACL_LAST_LINE_MASK;
    }
    else if (block == AIR_ACL_BLOCK_1)
    {
        *ptr_last_line = (data >> ACL_LAST_LINE_1_OFFSET) & ACL_LAST_LINE_MASK;
    }
    else if (block == AIR_ACL_BLOCK_2)
    {
        *ptr_last_line = (data >> ACL_LAST_LINE_2_OFFSET) & ACL_LAST_LINE_MASK;
    }
    else if (block == AIR_ACL_BLOCK_3)
    {
        *ptr_last_line = (data >> ACL_LAST_LINE_3_OFFSET) & ACL_LAST_LINE_MASK;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setLastLine
 * PURPOSE:
 *      Set specified block last ACL rule row number.
 * INPUT:
 *      unit                -- unit id
 *      block               -- block id
 *      last_line           -- last rule row number of specified block
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Each block has total 128 ACL rules, and they will be split as 4 banks, each bank has 32 rows.
 */
AIR_ERROR_NO_T
hal_sco_acl_setLastLine(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const UI32_T          last_line)
{
    I32_T  ret;
    UI32_T data;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(block, AIR_ACL_BLOCK_LAST);
    HAL_CHECK_MIN_MAX_RANGE(last_line, 0, HAL_SCO_MAX_NUM_OF_LAST_LINE - 1);

    if ((ret = aml_readReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    if (block == AIR_ACL_BLOCK_0)
    {
        data = (data & ~(ACL_LAST_LINE_MASK << ACL_LAST_LINE_0_OFFSET)) | (last_line << ACL_LAST_LINE_0_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    else if (block == AIR_ACL_BLOCK_1)
    {
        data = (data & ~(ACL_LAST_LINE_MASK << ACL_LAST_LINE_1_OFFSET)) | (last_line << ACL_LAST_LINE_1_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    else if (block == AIR_ACL_BLOCK_2)
    {
        data = (data & ~(ACL_LAST_LINE_MASK << ACL_LAST_LINE_2_OFFSET)) | (last_line << ACL_LAST_LINE_2_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }
    else if (block == AIR_ACL_BLOCK_3)
    {
        data = (data & ~(ACL_LAST_LINE_MASK << ACL_LAST_LINE_3_OFFSET)) | (last_line << ACL_LAST_LINE_3_OFFSET);
        if ((ret = aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T))) != AIR_E_OK)
        {
            return ret;
        }
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getMibCnt
 * PURPOSE:
 *      Get ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 *      cnt_index        -- mib counter index
 * OUTPUT:
 *      ptr_cnt          -- pointer to receive count
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      ACL has total 64 mib counters, the counter index can be set by hal_sco_acl_setAction.
 */
AIR_ERROR_NO_T
hal_sco_acl_getMibCnt(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt)
{
    I32_T  ret;
    UI32_T value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(cnt_index, 0, HAL_SCO_MAX_NUM_OF_MIB_ID - 1);

    value = (cnt_index & ACL_MIB_SEL_MASK) << ACL_MIB_SEL_OFFSET;
    if ((ret = aml_writeReg(unit, ACL_MIB_CNT_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    if ((ret = aml_readReg(unit, ACL_MIB_CNT, ptr_cnt, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_clearMibCnt
 * PURPOSE:
 *      Clear ACL mib counter.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      For clear all mib counters, ACL has total 64 mib counters,
 *      the counter index can be set by hal_sco_acl_setAction.
 */
AIR_ERROR_NO_T
hal_sco_acl_clearMibCnt(
    const UI32_T unit)
{
    I32_T  ret;
    UI32_T value;

    value = ACL_MIB_CLEAR;
    if ((ret = aml_writeReg(unit, ACL_MIB_CNT_CFG, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule     -- pointer buffer of rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    I32_T             ret;
    UI32_T            value;
    UI32_T            data[3];
    AIR_PORT_BITMAP_T air_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_UDF_ENTRY - 1);

    value = (entry_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_READ | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    if (AIR_E_TIMEOUT == _checkAclUdfDone(unit))
    {
        return AIR_E_TIMEOUT;
    }
    aml_readReg(unit, ACL_AUTR0, data, sizeof(UI32_T));
    aml_readReg(unit, ACL_AUTR1, data + 1, sizeof(UI32_T));
    aml_readReg(unit, ACL_AUTR2, data + 2, sizeof(UI32_T));

    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, ptr_udf_rule->portmap[0]);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->cmp_mask);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->cmp_pat);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, ptr_udf_rule->udf_rule_en);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, ptr_udf_rule->udf_pkt_type);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, WORD_OFST_OFFSET, WORD_OFST_WIDTH, ptr_udf_rule->word_ofst);
    HAL_SCO_ACL_DERIVE_TBL_MULTIFIELDS(unit, data, CMP_SEL_OFFSET, CMP_SEL_WIDTH, ptr_udf_rule->cmp_sel);
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, ptr_udf_rule->portmap, air_portmap);
    AIR_PORT_BITMAP_COPY(ptr_udf_rule->portmap, air_portmap);
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 *      ptr_udf_rule     -- pointer buffer of rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      If cmp=pattern, the corresponding setting is cmp-key&cmp-mask;
 *      If cmp=threshold, the corresponding setting is cmp-low&cmp-high;
 */
AIR_ERROR_NO_T
hal_sco_acl_setUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    I32_T             ret = AIR_E_OK;
    UI32_T            value = 0;
    UI32_T            data[3] = {0};
    AIR_PORT_BITMAP_T mac_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_UDF_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->udf_pkt_type, 0, HAL_SCO_MAX_NUM_OF_OFST_TP - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->word_ofst, 0, HAL_SCO_MAX_NUM_OF_WORD_OFST - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_sel, 0, HAL_SCO_MAX_NUM_OF_CMP_SEL - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_pat, 0, HAL_SCO_MAX_NUM_OF_CMP_PAT);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_mask, 0, HAL_SCO_MAX_NUM_OF_CMP_BIT);
    HAL_CHECK_PORT_BITMAP(unit, ptr_udf_rule->portmap);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_udf_rule->portmap, mac_portmap);

    _hal_fillTblMultiFields(unit, data, 3, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, mac_portmap[0]);
    _hal_fillTblMultiFields(unit, data, 3, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->cmp_mask);
    _hal_fillTblMultiFields(unit, data, 3, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->cmp_pat);
    _hal_fillTblMultiFields(unit, data, 3, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, ptr_udf_rule->udf_rule_en);
    _hal_fillTblMultiFields(unit, data, 3, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, ptr_udf_rule->udf_pkt_type);
    _hal_fillTblMultiFields(unit, data, 3, WORD_OFST_OFFSET, WORD_OFST_WIDTH, ptr_udf_rule->word_ofst);
    _hal_fillTblMultiFields(unit, data, 3, CMP_SEL_OFFSET, CMP_SEL_WIDTH, ptr_udf_rule->cmp_sel);

    DIAG_PRINT(HAL_DBG_INFO, "========acl udf rule=======\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data0: 0x%08x\n", data[0]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data1: 0x%08x\n", data[1]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data2: 0x%08x\n", data[2]);
    if (AIR_E_TIMEOUT == _checkAclUdfDone(unit))
    {
        return AIR_E_TIMEOUT;
    }
    aml_writeReg(unit, ACL_AUTW0, data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW1, data + 1, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW2, data + 2, sizeof(UI32_T));
    value = (entry_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_WRITE | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_delUdfRule(
    const UI32_T unit,
    const UI8_T  entry_idx)
{
    I32_T  ret;
    UI32_T value;
    UI32_T data = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_SCO_MAX_NUM_OF_UDF_ENTRY - 1);

    aml_writeReg(unit, ACL_AUTW0, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW1, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW2, &data, sizeof(UI32_T));

    value = (entry_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_WRITE | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_clearUdfRule(
    const UI32_T unit)
{
    I32_T  ret;
    UI32_T value;

    value = ACL_UDF_CLEAR | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getMeterTable
 * PURPOSE:
 *      get meter table configuration.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 * OUTPUT:
 *      ptr_enable          -- meter enable state
 *      ptr_rate            -- ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate)
{
    I32_T  ret;
    UI32_T value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(meter_id, 0, HAL_SCO_MAX_NUM_OF_METER_ID - 1);

    if (AIR_E_TIMEOUT == _checkMeterBusy(unit))
    {
        return AIR_E_TIMEOUT;
    }
    value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_READ | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) |
            ACL_RATE_EN;

    if ((ret = aml_writeReg(unit, ACLRMC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    aml_readReg(unit, ACLRMD1, &value, sizeof(UI32_T));
    *ptr_enable = ((value >> ACL_RATE_EN_OFFSET) & 0x1) ? TRUE : FALSE;
    *ptr_rate = value & ACL_RATE_TOKEN_MASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                -- unit id
 *      meter_id            -- meter id
 *      enable              -- meter enable state
 *      rate                -- ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate)
{
    I32_T  ret;
    UI32_T value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(meter_id, 0, HAL_SCO_MAX_NUM_OF_METER_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(rate, 0, HAL_SCO_MAX_NUM_OF_TOKEN);

    if (TRUE == enable)
    {
        value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_WRITE | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) |
                ACL_RATE_EN | (rate & ACL_RATE_TOKEN_MASK);
    }
    else if (FALSE == enable)
    {
        value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_WRITE | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) |
                ACL_RATE_DIS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_TIMEOUT == _checkMeterBusy(unit))
    {
        return AIR_E_TIMEOUT;
    }
    if ((ret = aml_writeReg(unit, ACLRMC, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getDropEn
 * PURPOSE:
 *      Get enable state of drop precedence on specified port.
 * INPUT:
 *      unit            -- unit id
 *      port            -- port id
 * OUTPUT:
 *      ptr_enable      -- enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropEn(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    UI32_T en, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, DPCR_EN(mac_port), &en, sizeof(UI32_T));
    *ptr_enable = en ? TRUE : FALSE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setDropEn
 * PURPOSE:
 *      Set enable state of drop precedence on specified port.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      enable      -- enable state of drop precedence
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropEn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI32_T en, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    en = enable ? 1 : 0;
    aml_writeReg(unit, DPCR_EN(mac_port), &en, sizeof(UI32_T));
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getDropThrsh
 * PURPOSE:
 *      Get ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_high        --  High threshold
 *      ptr_low         --  Low threshold
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low)
{
    UI32_T data, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    *ptr_low = data & DPCR_LOW_THRSH_WIDTH;
    *ptr_high = (data >> DPCR_HIGH_THRSH_OFFSET) & DPCR_HIGH_THRSH_WIDTH;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setDropThrsh
 * PURPOSE:
 *      Set ACL drop threshold.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      high            --  High threshold
 *      low             --  Low threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low)
{
    UI32_T data, mac_port;

    HAL_CHECK_MIN_MAX_RANGE(low, 0, HAL_SCO_MAX_NUM_OF_DROP_THRSH);
    HAL_CHECK_MIN_MAX_RANGE(high, 0, HAL_SCO_MAX_NUM_OF_DROP_THRSH);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    data = (data & ~(DPCR_LOW_THRSH_WIDTH)) | low;
    data = (data & ~(DPCR_HIGH_THRSH_WIDTH << DPCR_HIGH_THRSH_OFFSET)) | (high << DPCR_HIGH_THRSH_OFFSET);
    aml_writeReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getDropPbb
 * PURPOSE:
 *      Get ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 * OUTPUT:
 *      ptr_pbb         --  Drop probability(unit:1/1023)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb)
{
    UI32_T data, mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    *ptr_pbb = (data >> DPCR_PBB_OFFSET) & DPCR_PBB_WIDTH;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setDropPbb
 * PURPOSE:
 *      Set ACL drop probability.
 * INPUT:
 *      unit            --  unit id
 *      port            --  port id
 *      color           --  AIR_ACL_DP_COLOR_GREEN : Green
 *                          AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      pbb             --  Drop probability(unit:1/1023)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb)
{
    UI32_T data, mac_port;

    HAL_CHECK_MIN_MAX_RANGE(pbb, 0, DPCR_PBB_WIDTH);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    data = (data & ~(DPCR_PBB_WIDTH << DPCR_PBB_OFFSET)) | (pbb << DPCR_PBB_OFFSET);
    aml_writeReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            -- unit id
 * OUTPUT:
 *      ptr_type        -- TRUE: Exclude management frame
 *                         FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getDropExMfrm(
    const UI32_T unit,
    BOOL_T      *ptr_type)
{
    UI32_T data;

    aml_readReg(unit, GIRLCR, &data, sizeof(UI32_T));
    data = (data >> DP_MFRM_EX_OFFSET) & 0x1;
    *ptr_type = data ? TRUE : FALSE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setDropExMfrm
 * PURPOSE:
 *      Exclude/Include management frames to drop precedence control.
 * INPUT:
 *      unit            --  unit id
 *      type            --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setDropExMfrm(
    const UI32_T unit,
    const BOOL_T type)
{
    UI32_T data;

    aml_readReg(unit, GIRLCR, &data, sizeof(UI32_T));
    data = (data & ~(1U << DP_MFRM_EX_OFFSET)) | ((type ? 1 : 0) << DP_MFRM_EX_OFFSET);
    aml_writeReg(unit, GIRLCR, &data, sizeof(UI32_T));
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getTrtcmEn
 * PURPOSE:
 *      Get TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_enable      --  Enable/Disable trTCM
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getTrtcmEn(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    UI32_T value;

    aml_readReg(unit, ACL_TRTCM, &value, sizeof(UI32_T));
    if (value & BIT(ACL_TRTCM_EN_OFFSET))
    {
        *ptr_enable = TRUE;
    }
    else
    {
        *ptr_enable = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setTrtcmEn
 * PURPOSE:
 *      Set TRTCM enable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  Enable/Disable TRTCM
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setTrtcmEn(
    const UI32_T unit,
    BOOL_T       enable)
{
    UI32_T value;

    aml_readReg(unit, ACL_TRTCM, &value, sizeof(UI32_T));

    value &= ~(BIT(ACL_TRTCM_EN_OFFSET));
    if (TRUE == enable)
    {
        value |= BIT(ACL_TRTCM_EN_OFFSET);
    }

    aml_writeReg(unit, ACL_TRTCM, &value, sizeof(UI32_T));

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getTrtcmTable
 * PURPOSE:
 *      Get a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *
 * OUTPUT:
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    I32_T  ret;
    UI32_T value, trtcmr1, trtcmr2;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(tcm_idx, 0, HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY - 1);

    if (AIR_E_TIMEOUT == _checkTrtcmBusy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    value = (1 << ACL_TRTCM_BUSY_OFFSET) | ACL_TRTCM_READ | ((tcm_idx & ACL_TRTCM_ID_MASK) << ACL_TRTCM_ID_OFFSET);

    if ((ret = aml_writeReg(unit, ACL_TRTCMA, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    aml_readReg(unit, ACL_TRTCMR1, &trtcmr1, sizeof(UI32_T));
    aml_readReg(unit, ACL_TRTCMR2, &trtcmr2, sizeof(UI32_T));

    ptr_tcm->cbs = (trtcmr1 >> ACL_TRTCM_CBS_OFFSET) & ACL_TRTCM_CBS_MASK;
    ptr_tcm->pbs = (trtcmr1 >> ACL_TRTCM_EBS_OFFSET) & ACL_TRTCM_EBS_MASK;
    ptr_tcm->cir = (trtcmr2 >> ACL_TRTCM_CIR_OFFSET) & ACL_TRTCM_CIR_MASK;
    ptr_tcm->pir = (trtcmr2 >> ACL_TRTCM_EIR_OFFSET) & ACL_TRTCM_EIR_MASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setTrtcmTable
 * PURPOSE:
 *      Set a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    I32_T  ret;
    UI32_T value, trtcmw1, trtcmw2;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(tcm_idx, 0, HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->cbs, 0, HAL_SCO_MAX_NUM_OF_CBS);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->cir, 0, HAL_SCO_MAX_NUM_OF_CIR);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->pbs, 0, HAL_SCO_MAX_NUM_OF_PBS);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->pir, 0, HAL_SCO_MAX_NUM_OF_PIR);

    if (AIR_E_TIMEOUT == _checkTrtcmBusy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    trtcmw1 = ((ptr_tcm->cbs & ACL_TRTCM_CBS_MASK) << ACL_TRTCM_CBS_OFFSET) |
              ((ptr_tcm->pbs & ACL_TRTCM_EBS_MASK) << ACL_TRTCM_EBS_OFFSET);
    trtcmw2 = ((ptr_tcm->cir & ACL_TRTCM_CIR_MASK) << ACL_TRTCM_CIR_OFFSET) |
              ((ptr_tcm->pir & ACL_TRTCM_EIR_MASK) << ACL_TRTCM_EIR_OFFSET);
    aml_writeReg(unit, ACL_TRTCMW1, &trtcmw1, sizeof(UI32_T));
    aml_writeReg(unit, ACL_TRTCMW2, &trtcmw2, sizeof(UI32_T));

    value = (1 << ACL_TRTCM_BUSY_OFFSET) | ACL_TRTCM_WRITE | ((tcm_idx & ACL_TRTCM_ID_MASK) << ACL_TRTCM_ID_OFFSET);

    if ((ret = aml_writeReg(unit, ACL_TRTCMA, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_getL3RoutingTable
 * PURPOSE:
 *      Get DA/SA of the specific index in routing address table.
 *
 * INPUT:
 *      unit            --  device ID
 *      mac_sel         --  It indicates the routing address table select is DA or SA.
 *      id              --  the specific table index
 *
 * OUTPUT:
 *      ptr_mac         --  routing address
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_getL3RoutingTable(
    const UI32_T unit,
    const UI32_T mac_sel,
    const UI32_T id,
    AIR_MAC_T    ptr_mac)
{
    UI32_T mac_h = 0, mac_l = 0, value = 0;

    /* Check parameter */
    if (AIR_RAT_MAC_SEL_SA == mac_sel)
    {
        HAL_CHECK_MIN_MAX_RANGE(id, 0, BITS_RANGE(0, RAT_ADDR_SA_LENG));
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(id, 0, BITS_RANGE(0, RAT_ADDR_DA_LENG));
    }

    value = BITS_OFF_L(mac_sel, RAT_SEL_DA_OFFT, RAT_SEL_DA_LENG);
    value |= BITS_OFF_L(0, RAT_SEL_WRITE_OFFT, RAT_SEL_WRITE_LENG);
    value |= (AIR_RAT_MAC_SEL_SA == mac_sel) ? BITS_OFF_L(id, RAT_ADDR_OFFT, RAT_ADDR_SA_LENG) :
                                               BITS_OFF_L(id, RAT_ADDR_OFFT, RAT_ADDR_DA_LENG);
    aml_writeReg(unit, RATCR, &value, sizeof(UI32_T));

    aml_readReg(unit, RATDPH, &mac_h, sizeof(UI32_T));
    ptr_mac[0] = BITS_OFF_R(mac_h, 8, 8);
    ptr_mac[1] = BITS_OFF_R(mac_h, 0, 8);

    aml_readReg(unit, RATDPL, &mac_l, sizeof(UI32_T));
    ptr_mac[2] = BITS_OFF_R(mac_l, 24, 8);
    ptr_mac[3] = BITS_OFF_R(mac_l, 16, 8);
    ptr_mac[4] = BITS_OFF_R(mac_l, 8, 8);
    ptr_mac[5] = BITS_OFF_R(mac_l, 0, 8);

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_setL3RoutingTable
 * PURPOSE:
 *      Set DA/SA routing address table.
 * INPUT:
 *      unit            --  device ID
 *      mac_sel         --  It indicates the routing address table select is DA or SA.
 *      id              --  the specific table index
 *      ptr_mac         --  routing address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_setL3RoutingTable(
    const UI32_T    unit,
    const UI32_T    mac_sel,
    const UI32_T    id,
    const AIR_MAC_T ptr_mac)
{
    I32_T  ret;
    UI32_T mac_h = 0, mac_l = 0, value = 0;

    /* Check parameter */
    if (AIR_RAT_MAC_SEL_SA == mac_sel)
    {
        HAL_CHECK_MIN_MAX_RANGE(id, 0, BITS_RANGE(0, RAT_ADDR_SA_LENG));
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(id, 0, BITS_RANGE(0, RAT_ADDR_DA_LENG));
    }

    mac_h = BITS_OFF_L(ptr_mac[0], 8, 8);
    mac_h |= ptr_mac[1];
    aml_writeReg(unit, RATDPH, &mac_h, sizeof(UI32_T));

    mac_l = BITS_OFF_L(ptr_mac[2], 24, 8);
    mac_l |= BITS_OFF_L(ptr_mac[3], 16, 8);
    mac_l |= BITS_OFF_L(ptr_mac[4], 8, 8);
    mac_l |= ptr_mac[5];
    aml_writeReg(unit, RATDPL, &mac_l, sizeof(UI32_T));

    value = BITS_OFF_L(mac_sel, RAT_SEL_DA_OFFT, RAT_SEL_DA_LENG);
    value |= BITS_OFF_L(1U, RAT_SEL_WRITE_OFFT, RAT_SEL_WRITE_LENG);
    value |= (AIR_RAT_MAC_SEL_SA == mac_sel) ? BITS_OFF_L(id, RAT_ADDR_OFFT, RAT_ADDR_SA_LENG) :
                                               BITS_OFF_L(id, RAT_ADDR_OFFT, RAT_ADDR_DA_LENG);

    if ((ret = aml_writeReg(unit, RATCR, &value, sizeof(UI32_T))) != AIR_E_OK)
    {
        return ret;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_init
 * PURPOSE:
 *      This API is used to init acl.
 * INPUT:
 *      unit             -- unit id
 *      acl_info         -- acl information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_init(
    const UI32_T unit)
{
    UI32_T i;

    hal_sco_acl_clearAll(unit);
    for (i = AIR_ACL_BLOCK_0; i < AIR_ACL_BLOCK_LAST; i++)
    {
        hal_sco_acl_setLastLine(unit, i, (AIR_MAX_NUM_OF_LAST_LINE - 1));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_acl_deinit
 * PURPOSE:
 *      This API is used to deinit acl.
 * INPUT:
 *      unit             -- unit id
 *      acl_info         -- acl information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_acl_deinit(
    const UI32_T unit)
{
    UI32_T             data, mac_port;
    AIR_ACL_DP_COLOR_T color;
    UI8_T              queue;

    hal_sco_acl_clearUdfRule(unit);
    hal_sco_acl_clearMibCnt(unit);
    hal_sco_acl_clearAll(unit);

    data = HAL_SCO_ACL_GLOBAL_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_GLOBAL_CFG, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_PORT_EN_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_PORT_EN, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_GROUP_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_GROUP_CFG, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_MEM_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_MEM_CFG, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_MEM_CFG_WDATA_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_MEM_CFG_WDATA0, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_MEM_CFG_WDATA1, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_MEM_CFG_WDATA2, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_MEM_CFG_WDATA3, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_MIB_CNT_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_MIB_CNT_CFG, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_UDF_TABLE_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_AUTC, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW0, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW1, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_AUTW2, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_RMC_DEFAULT_VALUE;
    aml_writeReg(unit, ACLRMC, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_TRTCM_DEFAULT_VALUE;
    aml_writeReg(unit, ACL_TRTCM, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_TRTCMA, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_TRTCMW1, &data, sizeof(UI32_T));
    aml_writeReg(unit, ACL_TRTCMW2, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_ROUTING_ADDRESS_TABLE_DEFAULT_VALUE;
    aml_writeReg(unit, RATDPH, &data, sizeof(UI32_T));
    aml_writeReg(unit, RATDPL, &data, sizeof(UI32_T));
    aml_writeReg(unit, RATCR, &data, sizeof(UI32_T));
    data = HAL_SCO_ACL_DPCR_DEFAULT_VALUE;
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), mac_port)
    {
        aml_writeReg(unit, DPCR_EN(mac_port), &data, sizeof(UI32_T));
        for (color = 0; color < AIR_ACL_DP_COLOR_LAST; color++)
        {
            for (queue = 0; queue < AIR_QOS_QUEUE_MAX_NUM; queue++)
            {
                aml_writeReg(unit, DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
            }
        }
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_acl_getCapacity
 * PURPOSE:
 *      Get the acl resource capacity
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_acl_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
        case AIR_SWC_RSRC_ACL_ACTION:
            *ptr_size = HAL_SCO_MAX_NUM_OF_ACL_ENTRY;
            break;
        case AIR_SWC_RSRC_ACL_UDF:
            *ptr_size = HAL_SCO_MAX_NUM_OF_UDF_ENTRY;
            break;
        case AIR_SWC_RSRC_ACL_COUNTER:
            *ptr_size = HAL_SCO_MAX_NUM_OF_MIB_ID;
            break;
        case AIR_SWC_RSRC_ACL_METER:
            *ptr_size = HAL_SCO_MAX_NUM_OF_METER_ID;
            break;
        case AIR_SWC_RSRC_ACL_TRTCM:
            *ptr_size = HAL_SCO_MAX_NUM_OF_TRTCM_ENTRY;
            break;
        default:
            ret = AIR_E_BAD_PARAMETER;
            break;
    }

    return ret;
}

/* FUNCTION NAME: hal_sco_acl_getUsage
 * PURPOSE:
 *      Get the acl resource usage
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
 */
AIR_ERROR_NO_T
hal_sco_acl_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    *ptr_cnt = 0;
    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
            _hal_sco_acl_getRulesUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_ACTION:
            _hal_sco_acl_getActionUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_UDF:
            _hal_sco_acl_getUdfUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_COUNTER:
            _hal_sco_acl_getCounterUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_METER:
            _hal_sco_acl_getMeterUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_TRTCM:
            _hal_sco_acl_getTrtcmUsage(unit, ptr_cnt);
            break;
        default:
            ret = AIR_E_BAD_PARAMETER;
            break;
    }

    return ret;
}
