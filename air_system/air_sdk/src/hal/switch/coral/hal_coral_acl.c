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
/* FILE NAME:  hal_coral_acl.c
 * PURPOSE:
 *  Implement ACL module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_acl.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_bitmap.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data_buffer, offset, width, dst)          \
    do                                                                                 \
    {                                                                                  \
        (dst) = _hal_coral_acl_deriveTblMultiFields((data_buffer), (offset), (width)); \
    } while (0)

#define HAL_CORAL_ACL_BYTE_TO_BIT_MASK(byte_cnt, byte_mask, bit_mask) \
    do                                                                \
    {                                                                 \
        UI32_T i;                                                     \
        for (i = 0; i < byte_cnt; i++)                                \
        {                                                             \
            if (byte_mask & (1 << i))                                 \
            {                                                         \
                bit_mask |= (0xff << 8 * i);                          \
            }                                                         \
        }                                                             \
    } while (0)

#define HAL_CORAL_ACL_BIT_TO_BYTE_MASK(byte_cnt, byte_mask, bit_mask) \
    do                                                                \
    {                                                                 \
        UI32_T i;                                                     \
        for (i = 0; i < byte_cnt; i++)                                \
        {                                                             \
            if (bit_mask & (0xff << 8 * i))                           \
            {                                                         \
                byte_mask |= (1 << i);                                \
            }                                                         \
        }                                                             \
    } while (0)

#define HAL_CORAL_ACL_COVERT_TO_TCELL(key, mask)         ((UI32_T)((key) | (~(mask))))
#define HAL_CORAL_ACL_COVERT_TO_CCELL(key, mask)         ((UI32_T)((~(key)) | (~(mask))))
#define HAL_CORAL_ACL_PARSE_CELL_TO_KEY(t_cell, c_cell)  ((t_cell) & (~(c_cell)))
#define HAL_CORAL_ACL_PARSE_CELL_TO_MASK(t_cell, c_cell) ((t_cell) ^ (c_cell))

#define HAL_CORAL_ACL_RULL_FILL_TCELL(t_cell, offset, width, key, mask)                                  \
    do                                                                                                   \
    {                                                                                                    \
        _hal_coral_acl_fillTblMultiFields(t_cell, HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT, (offset), (width), \
                                          HAL_CORAL_ACL_COVERT_TO_TCELL(key, mask));                     \
    } while (0)

#define HAL_CORAL_ACL_RULL_FILL_CCELL(c_cell, offset, width, key, mask)                                  \
    do                                                                                                   \
    {                                                                                                    \
        _hal_coral_acl_fillTblMultiFields(c_cell, HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT, (offset), (width), \
                                          HAL_CORAL_ACL_COVERT_TO_CCELL(key, mask));                     \
    } while (0)

#define HAL_CORAL_ACL_RULL_GET_KEY_FROM_CELL(t_cell, c_cell, offset, width, key)                                 \
    do                                                                                                           \
    {                                                                                                            \
        (key) = HAL_CORAL_ACL_PARSE_CELL_TO_KEY(_hal_coral_acl_deriveTblMultiFields(t_cell, (offset), (width)),  \
                                                _hal_coral_acl_deriveTblMultiFields(c_cell, (offset), (width))); \
    } while (0)

#define HAL_CORAL_ACL_RULL_GET_FROM_CELL(t_cell, c_cell, offset, width, key, mask)                                 \
    do                                                                                                             \
    {                                                                                                              \
        (key) = HAL_CORAL_ACL_PARSE_CELL_TO_KEY(_hal_coral_acl_deriveTblMultiFields(t_cell, (offset), (width)),    \
                                                _hal_coral_acl_deriveTblMultiFields(c_cell, (offset), (width)));   \
        (mask) = HAL_CORAL_ACL_PARSE_CELL_TO_MASK(_hal_coral_acl_deriveTblMultiFields(t_cell, (offset), (width)),  \
                                                  _hal_coral_acl_deriveTblMultiFields(c_cell, (offset), (width))); \
    } while (0)

/* Remap the acl entry index for the abst lowest index has the highest priority.
 */
#define HAL_CORAL_ACL_ENTRY_IDX_REMAP(__idx__, __entry_idx__)                                          \
    do                                                                                                 \
    {                                                                                                  \
        (__entry_idx__) = HAL_CORAL_MAX_NUM_OF_ACL_ENTRY - HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP +        \
                          2 * ((__idx__) & (HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP - 1)) - (__idx__);      \
        DIAG_PRINT(HAL_DBG_INFO, "remap entry %d to acl rule entry %d\n", (__idx__), (__entry_idx__)); \
    } while (0)

/* DATA TYPE DECLARATIONS
 */
#define HAL_CORAL_ACL_ENTRY_NUM_WORDS              AIR_BITMAP_SIZE(HAL_CORAL_MAX_NUM_OF_ACL_ENTRY)
#define HAL_CORAL_ACL_COUNTER_NUM_WORDS            AIR_BITMAP_SIZE(HAL_CORAL_MAX_NUM_OF_MIB_ID)
#define HAL_CORAL_ACL_ARP_NO_FIELDMAP_REVISION     (1)
#define HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION (1)
#define HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT         (12) /* each rule entry costs 3 blocks * 4 regs */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_ACL, "hal_coral_acl.c");

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_checkDone(
    const UI32_T                     unit,
    const HAL_CORAL_ACL_CHECK_TYPE_T type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         check_bit = 0, i = 0, reg = 0, value = 0, offset = 0;
    switch (type)
    {
        case HAL_CORAL_ACL_CHECK_ACL:
            check_bit = 1;
            reg = CORAL_ACL_MEM_CFG;
            offset = CORAL_ACL_MEM_CFG_DONE_OFFSET;
            break;
        case HAL_CORAL_ACL_CHECK_UDF:
            check_bit = 0;
            reg = CORAL_ACL_AUTC;
            offset = CORAL_ACL_UDF_ACC_OFFSET;
            break;
        case HAL_CORAL_ACL_CHECK_TRTCM:
            check_bit = 0;
            reg = CORAL_ACL_TRTCMA;
            offset = CORAL_ACL_TRTCM_BUSY_OFFSET;
            break;
        case HAL_CORAL_ACL_CHECK_METER:
            check_bit = 0;
            reg = CORAL_ACLRMC;
            offset = CORAL_ACL_RATE_BUSY_OFFSET;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    if (AIR_E_OK == rc)
    {
        for (i = 0; i < HAL_CORAL_ACL_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, reg, &value, sizeof(UI32_T));
            if (check_bit == (value >> offset))
            {
                break;
            }
            osal_delayUs(10);
        }
        if (i >= HAL_CORAL_ACL_MAX_BUSY_TIME)
        {
            rc = AIR_E_TIMEOUT;
        }
    }
    return rc;
}

static void
_hal_coral_acl_fillTblMultiFields(
    UI32_T      *data_buffer,
    UI32_T       data_count,
    const UI32_T offset,
    const UI32_T width,
    const UI32_T value)
{
    UI32_T data_index = 0, bit_index = 0;
    UI32_T extended_data[2] = {0};
    UI32_T extended_mask[2] = {0};
    UI32_T msk;
    UI32_T val;

    if (!((NULL == data_buffer) || (0 == data_count) || (0 == width) || (width > 32) ||
          (offset + width > data_count * 32)))
    {
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
    }
}

static UI32_T
_hal_coral_acl_deriveTblMultiFields(
    UI32_T      *data_buffer,
    const UI32_T offset,
    const UI32_T width)
{
    UI32_T data_index, bit_index, msk;
    UI32_T extended_data[2] = {0};
    UI32_T extended_mask[2] = {0};

    if ((NULL != data_buffer) && (width > 0) && (width <= 32))
    {
        msk = ((1U << (width - 1U)) << 1U) - 1U;
        data_index = offset / 32;
        bit_index = offset % 32;
        extended_mask[0] = msk << bit_index;
        extended_mask[1] = (msk >> (31U - bit_index)) >> 1U;
        extended_data[0] = (data_buffer[data_index] & extended_mask[0]) >> bit_index;
        extended_data[1] = ((data_buffer[data_index + 1] & extended_mask[1]) << (31U - bit_index)) << 1U;
    }
    return (extended_data[0] | extended_data[1]);
}

static void
_hal_coral_acl_setActionTable(
    const HAL_CORAL_ACL_ACTION_T *ptr_action,
    UI32_T                       *data)
{
    _hal_coral_acl_fillTblMultiFields(data, 4, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, ptr_action->port_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH, ptr_action->vlan_port_sel);
    _hal_coral_acl_fillTblMultiFields(data, 4, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH, ptr_action->dest_port_sel);
    _hal_coral_acl_fillTblMultiFields(data, 4, PORT_OFFSET, PORT_WIDTH, ptr_action->portmap[0]);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, ptr_action->cnt_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->cnt_idx);
    _hal_coral_acl_fillTblMultiFields(data, 4, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, ptr_action->attack_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH, ptr_action->attack_idx);
    _hal_coral_acl_fillTblMultiFields(data, 4, RATE_EN_OFFSET, RATE_EN_WIDTH, ptr_action->rate_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->rate_idx);
    _hal_coral_acl_fillTblMultiFields(data, 4, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, ptr_action->fwd_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->fwd);
    _hal_coral_acl_fillTblMultiFields(data, 4, DONT_LEARN_OFFSET, DONT_LEARN_WIDTH, ptr_action->dont_learn);
    _hal_coral_acl_fillTblMultiFields(data, 4, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirrormap);
    _hal_coral_acl_fillTblMultiFields(data, 4, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, ptr_action->pri_user_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);
    _hal_coral_acl_fillTblMultiFields(data, 4, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, ptr_action->egtag_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->egtag);
    _hal_coral_acl_fillTblMultiFields(data, 4, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, ptr_action->lyvlan_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lyvlan);
    _hal_coral_acl_fillTblMultiFields(data, 4, BPDU_OFFSET, BPDU_WIDTH, ptr_action->bpdu);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->mang);
    _hal_coral_acl_fillTblMultiFields(data, 4, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, ptr_action->trtcm_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH, ptr_action->trtcm.drop_pcd_sel);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH,
                                      ptr_action->trtcm.drop_pcd_r);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH,
                                      ptr_action->trtcm.drop_pcd_y);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH,
                                      ptr_action->trtcm.drop_pcd_g);
    _hal_coral_acl_fillTblMultiFields(data, 4, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH,
                                      ptr_action->trtcm.cls_slr_sel);
    _hal_coral_acl_fillTblMultiFields(data, 4, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->trtcm.cls_slr);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH, ptr_action->trtcm.tcm_sel);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->trtcm.usr_tcm);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH, ptr_action->trtcm.tcm_idx);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, ptr_action->vlan_en);
    _hal_coral_acl_fillTblMultiFields(data, 4, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH, ptr_action->vlan_idx);
}

static void
_hal_coral_acl_getActionTable(
    UI32_T                 *data,
    HAL_CORAL_ACL_ACTION_T *ptr_action)
{
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, ptr_action->port_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH, ptr_action->vlan_port_sel);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH, ptr_action->dest_port_sel);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_OFFSET, PORT_WIDTH, ptr_action->portmap[0]);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, ptr_action->cnt_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->cnt_idx);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, ptr_action->attack_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH, ptr_action->attack_idx);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, RATE_EN_OFFSET, RATE_EN_WIDTH, ptr_action->rate_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->rate_idx);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, ptr_action->fwd_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->fwd);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirrormap);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, DONT_LEARN_OFFSET, DONT_LEARN_WIDTH, ptr_action->dont_learn);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, ptr_action->pri_user_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, ptr_action->egtag_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->egtag);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, ptr_action->lyvlan_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lyvlan);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, BPDU_OFFSET, BPDU_WIDTH, ptr_action->bpdu);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->mang);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, ptr_action->trtcm_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH, ptr_action->trtcm.drop_pcd_sel);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH,
                                         ptr_action->trtcm.drop_pcd_r);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH,
                                         ptr_action->trtcm.drop_pcd_y);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH,
                                         ptr_action->trtcm.drop_pcd_g);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH,
                                         ptr_action->trtcm.cls_slr_sel);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->trtcm.cls_slr);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH, ptr_action->trtcm.tcm_sel);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->trtcm.usr_tcm);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH, ptr_action->trtcm.tcm_idx);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, ptr_action->vlan_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH, ptr_action->vlan_idx);
}

static AIR_ERROR_NO_T
_hal_coral_acl_writeReg(
    const UI32_T                    unit,
    const UI32_T                    rule_idx,
    const UI32_T                    block_num,
    const HAL_CORAL_ACL_RULE_TCAM_T type,
    const HAL_CORAL_ACL_MEM_SEL_T   sel,
    const HAL_CORAL_ACL_MEM_FUNC_T  func,
    const UI32_T                   *data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         bn = 0, value = 0;

    for (bn = 0; bn < block_num; bn++)
    {
        if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_ACL))
        {
            return AIR_E_TIMEOUT;
        }
        aml_writeReg(unit, CORAL_ACL_MEM_CFG_WDATA0, data + bn * 4, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ACL_MEM_CFG_WDATA1, data + bn * 4 + 1, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ACL_MEM_CFG_WDATA2, data + bn * 4 + 2, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ACL_MEM_CFG_WDATA3, data + bn * 4 + 3, sizeof(UI32_T));
        value = (rule_idx << CORAL_ACL_MEM_CFG_RULE_ID_OFFSET) | (type << CORAL_ACL_MEM_CFG_TCAM_CELL_OFFSET) |
                (bn << CORAL_ACL_MEM_CFG_DATA_BN_OFFSET) | (sel << CORAL_ACL_MEM_CFG_MEM_SEL_OFFSET) |
                (func << CORAL_ACL_MEM_CFG_FUNC_SEL_OFFSET) | CORAL_ACL_MEM_CFG_EN;
        rc = aml_writeReg(unit, CORAL_ACL_MEM_CFG, &value, sizeof(UI32_T));
        if (AIR_E_OK != rc)
        {
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_acl_readReg(
    const UI32_T                    unit,
    const UI32_T                    rule_idx,
    const UI32_T                    block_num,
    const HAL_CORAL_ACL_RULE_TCAM_T type,
    const HAL_CORAL_ACL_MEM_SEL_T   sel,
    const HAL_CORAL_ACL_MEM_FUNC_T  func,
    UI32_T                         *data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         bn = 0, value = 0;

    for (bn = 0; bn < block_num; bn++)
    {
        value = (rule_idx << CORAL_ACL_MEM_CFG_RULE_ID_OFFSET) | (type << CORAL_ACL_MEM_CFG_TCAM_CELL_OFFSET) |
                (bn << CORAL_ACL_MEM_CFG_DATA_BN_OFFSET) | (sel << CORAL_ACL_MEM_CFG_MEM_SEL_OFFSET) |
                (func << CORAL_ACL_MEM_CFG_FUNC_SEL_OFFSET) | CORAL_ACL_MEM_CFG_EN;
        rc = aml_writeReg(unit, CORAL_ACL_MEM_CFG, &value, sizeof(UI32_T));
        if (AIR_E_OK == rc)
        {
            rc = _checkDone(unit, HAL_CORAL_ACL_CHECK_ACL);
            if (AIR_E_OK == rc)
            {
                aml_readReg(unit, CORAL_ACL_MEM_CFG_RDATA0, data + bn * 4, sizeof(UI32_T));
                aml_readReg(unit, CORAL_ACL_MEM_CFG_RDATA1, data + bn * 4 + 1, sizeof(UI32_T));
                aml_readReg(unit, CORAL_ACL_MEM_CFG_RDATA2, data + bn * 4 + 2, sizeof(UI32_T));
                aml_readReg(unit, CORAL_ACL_MEM_CFG_RDATA3, data + bn * 4 + 3, sizeof(UI32_T));
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_acl_getRuleType(
    const UI32_T unit,
    const UI32_T entry_idx,
    UI32_T      *ptr_type)
{
    UI32_T         data = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_PTR(ptr_type);

    data = (entry_idx << CORAL_ACL_MEM_CFG_RULE_ID_OFFSET) |
           (HAL_CORAL_ACL_RULE_T_CELL << CORAL_ACL_MEM_CFG_TCAM_CELL_OFFSET) |
           (HAL_CORAL_ACL_MEM_SEL_RULE << CORAL_ACL_MEM_CFG_MEM_SEL_OFFSET) |
           (HAL_CORAL_ACL_MEM_FUNC_READ << CORAL_ACL_MEM_CFG_FUNC_SEL_OFFSET) | CORAL_ACL_MEM_CFG_EN;
    rc = aml_writeReg(unit, CORAL_ACL_MEM_CFG, &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        rc = _checkDone(unit, HAL_CORAL_ACL_CHECK_ACL);
        if (AIR_E_OK == rc)
        {
            data = 0;
            aml_readReg(unit, CORAL_ACL_MEM_CFG_RDATA0, &data, sizeof(UI32_T));
        }
    }
    if (AIR_E_OK == rc)
    {
        /* rule format type is a must field, no need to check mask */
        HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(&data, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, *ptr_type);
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_acl_getfromCell(
    const UI32_T unit,
    const UI32_T entry_idx,
    UI32_T      *ptr_tcell,
    UI32_T      *ptr_ccell)
{
    HAL_CHECK_PTR(ptr_tcell);
    HAL_CHECK_PTR(ptr_ccell);

    _hal_coral_acl_readReg(unit, entry_idx, 3, HAL_CORAL_ACL_RULE_T_CELL, HAL_CORAL_ACL_MEM_SEL_RULE,
                           HAL_CORAL_ACL_MEM_FUNC_READ, ptr_tcell);
    _hal_coral_acl_readReg(unit, entry_idx, 3, HAL_CORAL_ACL_RULE_C_CELL, HAL_CORAL_ACL_MEM_SEL_RULE,
                           HAL_CORAL_ACL_MEM_FUNC_READ, ptr_ccell);
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_coral_acl_settoCell(
    const UI32_T unit,
    const UI32_T entry_idx,
    UI32_T      *ptr_tcell,
    UI32_T      *ptr_ccell)
{
    HAL_CHECK_PTR(ptr_tcell);
    HAL_CHECK_PTR(ptr_ccell);

    _hal_coral_acl_writeReg(unit, entry_idx, 3, HAL_CORAL_ACL_RULE_T_CELL, HAL_CORAL_ACL_MEM_SEL_RULE,
                            HAL_CORAL_ACL_MEM_FUNC_WRITE, ptr_tcell);
    _hal_coral_acl_writeReg(unit, entry_idx, 3, HAL_CORAL_ACL_RULE_C_CELL, HAL_CORAL_ACL_MEM_SEL_RULE,
                            HAL_CORAL_ACL_MEM_FUNC_WRITE, ptr_ccell);
    return AIR_E_OK;
}

static void
_hal_coral_acl_fillRuleFormat0(
    const UI32_T    unit,
    AIR_ACL_RULE_T *ptr_rule,
    UI32_T         *ptr_tcell,
    UI32_T         *ptr_ccell)
{
    UI32_T            i = 0, mask = 0, n = 0;
    AIR_PORT_BITMAP_T macportmap = {0};
    UI16_T            fieldmap = 0;

    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_0,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));
    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_0,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));

    for (i = AIR_ACL_DMAC_KEY; i < AIR_ACL_RULE_FIELD_LAST; i++)
    {
        if (BIT_CHK(ptr_rule->field_valid, i))
        {
            mask = 0;
            switch (i)
            {
                case AIR_ACL_DMAC_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.dmac, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.dmac[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.dmac[n], mask);
                    }
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_DMAC);
                    break;
                case AIR_ACL_SMAC_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.smac, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.smac[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.smac[n], mask);
                    }
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_SMAC);
                    break;
                case AIR_ACL_ETYPE_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(2, ptr_rule->mask.etype, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, ETYPE_OFFSET, ETYPE_WIDTH, ptr_rule->key.etype, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, ETYPE_OFFSET, ETYPE_WIDTH, ptr_rule->key.etype, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_ETYPE);
                    break;
                case AIR_ACL_STAG_KEY:
                    if (ptr_rule->mask.stag & 0x1)
                    {
                        mask |= 0xfff;
                    }
                    if (ptr_rule->mask.stag & 0x2)
                    {
                        mask |= 0xf000;
                    }
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_STAG);
                    break;
                case AIR_ACL_CTAG_KEY:
                    if (ptr_rule->mask.ctag & 0x1)
                    {
                        mask |= 0xfff;
                    }
                    if (ptr_rule->mask.ctag & 0x2)
                    {
                        mask |= 0xf000;
                    }
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_CTAG);
                    break;
                case AIR_ACL_DIP_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.dip, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DIP_OFFSET, DIP_WIDTH, ptr_rule->key.dip, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DIP_OFFSET, DIP_WIDTH, ptr_rule->key.dip, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_DIP);
                    break;
                case AIR_ACL_SIP_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.sip, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SIP_OFFSET, SIP_WIDTH, ptr_rule->key.sip, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SIP_OFFSET, SIP_WIDTH, ptr_rule->key.sip, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_SIP);
                    break;
                case AIR_ACL_DSCP_KEY:
                    mask = 0xff;
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DSCP_OFFSET, DSCP_WIDTH, ptr_rule->key.dscp, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DSCP_OFFSET, DSCP_WIDTH, ptr_rule->key.dscp, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_DSCP);
                    break;
                case AIR_ACL_NEXT_HEADER_KEY:
                    mask = 0xff;
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, PROTOCOL_OFFSET, PROTOCOL_WIDTH, ptr_rule->key.next_header,
                                                  mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, PROTOCOL_OFFSET, PROTOCOL_WIDTH, ptr_rule->key.next_header,
                                                  mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_PROTOCOL);
                    break;
                case AIR_ACL_FLOW_LABEL_KEY:
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_FLOW_LABEL);
                    break;
                case AIR_ACL_DPORT_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(2, ptr_rule->mask.dport, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DPORT_OFFSET, DPORT_WIDTH, ptr_rule->key.dport, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DPORT_OFFSET, DPORT_WIDTH, ptr_rule->key.dport, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_DPORT);
                    break;
                case AIR_ACL_SPORT_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(2, ptr_rule->mask.sport, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SPORT_OFFSET, SPORT_WIDTH, ptr_rule->key.sport, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SPORT_OFFSET, SPORT_WIDTH, ptr_rule->key.sport, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_SPORT);
                    break;
                case AIR_ACL_UDF_KEY:
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                                  ptr_rule->mask.udf);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                                  ptr_rule->mask.udf);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_UDF);
                    break;
                case AIR_ACL_DIPV6_KEY:
                    n = (ptr_rule->key.dipv6[12] << 24) | (ptr_rule->key.dipv6[13] << 16) |
                        (ptr_rule->key.dipv6[14] << 8) | ptr_rule->key.dipv6[15];
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.dipv6, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DIP_OFFSET, DIP_WIDTH, n, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DIP_OFFSET, DIP_WIDTH, n, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_DIP);
                    break;
                case AIR_ACL_SIPV6_KEY:
                    n = (ptr_rule->key.sipv6[12] << 24) | (ptr_rule->key.sipv6[13] << 16) |
                        (ptr_rule->key.sipv6[14] << 8) | ptr_rule->key.sipv6[15];
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.sipv6, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SIP_OFFSET, SIP_WIDTH, n, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SIP_OFFSET, SIP_WIDTH, n, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_SIP);
                    break;
                default:
                    /* do nothing */
                    break;
            }
        }
    }
    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, FIELDMAP_OFFSET, FIELDMAP_WIDTH, fieldmap, fieldmap);
    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, FIELDMAP_OFFSET, FIELDMAP_WIDTH, fieldmap, fieldmap);
    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, IS_IPV6_OFFSET, IS_IPV6_WIDTH, ptr_rule->isipv6, ptr_rule->isipv6);
    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, IS_IPV6_OFFSET, IS_IPV6_WIDTH, ptr_rule->isipv6, ptr_rule->isipv6);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_rule->portmap, macportmap);
    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, PORTMAP_OFFSET, PORTMAP_WIDTH, macportmap[0],
                                  BITS(0, (PORTMAP_WIDTH - 1)));

#ifdef AIR_EN_DEBUG
    for (i = 0; i < HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT; i++)
    {
        /* T/C cell print for debugging */
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:t acl data[%d]: 0x%08x\n", i, ptr_tcell[i]);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:c acl data[%d]: 0x%08x\n", i, ptr_ccell[i]);
    }
#endif
}

static void
_hal_coral_acl_getRuleFormat0(
    const UI32_T    unit,
    AIR_ACL_RULE_T *ptr_rule,
    UI32_T         *ptr_tcell,
    UI32_T         *ptr_ccell)
{
    UI32_T            i = 0, data = 0, n = 0;
    AIR_PORT_BITMAP_T macportmap = {0};
    UI32_T            fieldmap = 0;

    HAL_CORAL_ACL_RULL_GET_KEY_FROM_CELL(ptr_tcell, ptr_ccell, FIELDMAP_OFFSET, FIELDMAP_WIDTH, fieldmap);
    HAL_CORAL_ACL_RULL_GET_KEY_FROM_CELL(ptr_tcell, ptr_ccell, IS_IPV6_OFFSET, IS_IPV6_WIDTH, data);
    ptr_rule->isipv6 = data & 0xf;

    /* rule entry conversion */
    for (i = HAL_CORAL_ACL_DMAC; i < HAL_CORAL_ACL_FIELD_TYPE_LAST; i++)
    {
        if (BIT_CHK(fieldmap, i))
        {
            data = 0;
            switch (i)
            {
                case HAL_CORAL_ACL_DMAC:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        data = 0;
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell,
                                                         DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                         DMAC_WIDTH, ptr_rule->key.dmac[n], data);
                        if (0xff == data)
                        {
                            BIT_ADD(ptr_rule->mask.dmac, (AIR_MAC_ADDR_LEN - 1 - n));
                        }
                    }
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_DMAC_KEY);
                    break;
                case HAL_CORAL_ACL_SMAC:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        data = 0;
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell,
                                                         SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                         SMAC_WIDTH, ptr_rule->key.smac[n], data);
                        if (0xff == data)
                        {
                            BIT_ADD(ptr_rule->mask.smac, (AIR_MAC_ADDR_LEN - 1 - n));
                        }
                    }
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_SMAC_KEY);
                    break;
                case HAL_CORAL_ACL_ETYPE:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, ETYPE_OFFSET, ETYPE_WIDTH,
                                                     ptr_rule->key.etype, data);
                    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(2, ptr_rule->mask.etype, data);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ETYPE_KEY);
                    break;
                case HAL_CORAL_ACL_STAG:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag,
                                                     data);
                    if (data & 0xfff)
                    {
                        ptr_rule->mask.stag |= 0x1;
                    }
                    if (data & 0xf000)
                    {
                        ptr_rule->mask.stag |= 0x2;
                    }
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_STAG_KEY);
                    break;
                case HAL_CORAL_ACL_CTAG:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag,
                                                     data);
                    if (data & 0xfff)
                    {
                        ptr_rule->mask.ctag |= 0x1;
                    }
                    if (data & 0xf000)
                    {
                        ptr_rule->mask.ctag |= 0x2;
                    }
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_CTAG_KEY);
                    break;
                case HAL_CORAL_ACL_DIP:
                    if (FALSE == ptr_rule->isipv6)
                    {
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, DIP_OFFSET, DIP_WIDTH, ptr_rule->key.dip,
                                                         data);
                        HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.dip, data);
                        BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIP_KEY);
                    }
                    else
                    {
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, DIP_OFFSET, DIP_WIDTH, data, n);
                        HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.dipv6, n);
                        for (n = 0; n < 4; n++)
                        {
                            ptr_rule->key.dipv6[15 - n] = BITS_OFF_R(data, 8 * n, 8);
                        }
                        BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY);
                    }
                    break;
                case HAL_CORAL_ACL_SIP:
                    if (FALSE == ptr_rule->isipv6)
                    {
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, SIP_OFFSET, SIP_WIDTH, ptr_rule->key.sip,
                                                         data);
                        HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.sip, data);
                        BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIP_KEY);
                    }
                    else
                    {
                        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, SIP_OFFSET, SIP_WIDTH, data, n);
                        HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.sipv6, n);
                        for (n = 0; n < 4; n++)
                        {
                            ptr_rule->key.sipv6[15 - n] = BITS_OFF_R(data, 8 * n, 8);
                        }
                        BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY);
                    }
                    break;
                case HAL_CORAL_ACL_DSCP:
                    HAL_CORAL_ACL_RULL_GET_KEY_FROM_CELL(ptr_tcell, ptr_ccell, DSCP_OFFSET, DSCP_WIDTH,
                                                         ptr_rule->key.dscp);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_DSCP_KEY);
                    break;
                case HAL_CORAL_ACL_PROTOCOL:
                    HAL_CORAL_ACL_RULL_GET_KEY_FROM_CELL(ptr_tcell, ptr_ccell, PROTOCOL_OFFSET, PROTOCOL_WIDTH,
                                                         ptr_rule->key.next_header);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_NEXT_HEADER_KEY);
                    break;
                case HAL_CORAL_ACL_DPORT:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, DPORT_OFFSET, DPORT_WIDTH,
                                                     ptr_rule->key.dport, data);
                    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(2, ptr_rule->mask.dport, data);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_DPORT_KEY);
                    break;
                case HAL_CORAL_ACL_SPORT:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, SPORT_OFFSET, SPORT_WIDTH,
                                                     ptr_rule->key.sport, data);
                    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(2, ptr_rule->mask.sport, data);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_SPORT_KEY);
                    break;
                case HAL_CORAL_ACL_UDF:
                    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                                     ptr_rule->mask.udf);
                    BIT_ADD(ptr_rule->field_valid, AIR_ACL_UDF_KEY);
                    break;
                default:
                    break;
            }
        }
    }
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(ptr_tcell, PORTMAP_OFFSET, PORTMAP_WIDTH, macportmap[0]);
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, macportmap, ptr_rule->portmap);
}

static void
_hal_coral_acl_fillRuleFormat1(
    const UI32_T    unit,
    AIR_ACL_RULE_T *ptr_rule,
    UI32_T         *ptr_tcell,
    UI32_T         *ptr_ccell)
{
    UI32_T data = 0, n = 0, mask = 0;

    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_1,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));
    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_1,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));

    if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_FLOW_LABEL_KEY))
    {
        HAL_CORAL_ACL_BYTE_TO_BIT_MASK(3, ptr_rule->mask.flow_label, mask);
        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, FLOW_LABEL_OFFSET, FLOW_LABEL_WIDTH, ptr_rule->key.flow_label, mask);
        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, FLOW_LABEL_OFFSET, FLOW_LABEL_WIDTH, ptr_rule->key.flow_label, mask);
    }

    if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY))
    {
        for (n = 4; n < sizeof(AIR_IPV6_T); n += 4)
        {
            mask = 0;
            data = (ptr_rule->key.dipv6[12 - n] << 24) | (ptr_rule->key.dipv6[13 - n] << 16) |
                   (ptr_rule->key.dipv6[14 - n] << 8) | ptr_rule->key.dipv6[15 - n];
            HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, (ptr_rule->mask.dipv6 >> n), mask);
            HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DIP_IPV6_OFFSET + DIP_IPV6_WIDTH * (n / 4 - 1), DIP_IPV6_WIDTH,
                                          data, mask);
            HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DIP_IPV6_OFFSET + DIP_IPV6_WIDTH * (n / 4 - 1), DIP_IPV6_WIDTH,
                                          data, mask);
        }
    }
    if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY))
    {
        for (n = 4; n < sizeof(AIR_IPV6_T); n += 4)
        {
            mask = 0;
            data = (ptr_rule->key.sipv6[12 - n] << 24) | (ptr_rule->key.sipv6[13 - n] << 16) |
                   (ptr_rule->key.sipv6[14 - n] << 8) | ptr_rule->key.sipv6[15 - n];
            HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, (ptr_rule->mask.sipv6 >> n), mask);
            HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SIP_IPV6_OFFSET + SIP_IPV6_WIDTH * (n / 4 - 1), SIP_IPV6_WIDTH,
                                          data, mask);
            HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SIP_IPV6_OFFSET + SIP_IPV6_WIDTH * (n / 4 - 1), SIP_IPV6_WIDTH,
                                          data, mask);
        }
    }
}

static void
_hal_coral_acl_getRuleFormat1(
    const UI32_T    unit,
    AIR_ACL_RULE_T *ptr_rule,
    UI32_T         *ptr_tcell,
    UI32_T         *ptr_ccell)
{
    UI32_T data = 0, mask = 0, n = 0, j = 0;

    /* DIPv6 */
    for (n = 1; n < HAL_CORAL_IPV6_ADDR_WORD_SIZE; n++)
    {
        data = 0;
        mask = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, DIP_IPV6_OFFSET + DIP_IPV6_WIDTH * (n - 1),
                                         DIP_IPV6_WIDTH, data, mask);
        for (j = 0; j < sizeof(UI32_T); j++)
        {
            ptr_rule->key.dipv6[15 - n * 4 - j] = BITS_OFF_R(data, 8 * j, 8);
            if (0xff & mask >> (8 * j))
            {
                BIT_ADD(ptr_rule->mask.dipv6, n * 4 + j);
            }
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY);

    /* SIPv6 */
    for (n = 1; n < HAL_CORAL_IPV6_ADDR_WORD_SIZE; n++)
    {
        data = 0;
        mask = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, SIP_IPV6_OFFSET + SIP_IPV6_WIDTH * (n - 1),
                                         SIP_IPV6_WIDTH, data, mask);
        for (j = 0; j < sizeof(UI32_T); j++)
        {
            ptr_rule->key.sipv6[15 - n * 4 - j] = BITS_OFF_R(data, 8 * j, 8);
            if (0xff & mask >> (8 * j))
            {
                BIT_ADD(ptr_rule->mask.sipv6, n * 4 + j);
            }
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY);

    /* flow label */
    mask = 0;
    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, FLOW_LABEL_OFFSET, FLOW_LABEL_WIDTH,
                                     ptr_rule->key.flow_label, mask);
    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(3, ptr_rule->mask.flow_label, mask);
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_FLOW_LABEL_KEY);
}

static void
_hal_coral_acl_fillRuleFormat2(
    const UI32_T        unit,
    AIR_ACL_ARP_RULE_T *ptr_rule,
    UI32_T             *ptr_tcell,
    UI32_T             *ptr_ccell)
{
    UI32_T            i = 0, mask = 0, n = 0;
    AIR_PORT_BITMAP_T macportmap = {0};
    UI8_T             fieldmap = 0;

    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_2,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));
    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, HAL_CORAL_ACL_RULE_TYPE_2,
                                  BITS_RANGE(0, RULE_TYPE_WIDTH));

    /* rule entry conversion */
    for (i = AIR_ACL_ARP_DMAC_KEY; i < AIR_ACL_ARP_RULE_FIELD_LAST; i++)
    {
        if (BIT_CHK(ptr_rule->field_valid, i))
        {
            mask = 0;
            switch (i)
            {
                case AIR_ACL_ARP_DMAC_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.dmac, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.dmac[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.dmac[n], mask);
                    }
                    break;
                case AIR_ACL_ARP_SMAC_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.smac, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      SMAC_WIDTH, ptr_rule->key.smac[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      SMAC_WIDTH, ptr_rule->key.smac[n], mask);
                    }
                    break;
                case AIR_ACL_ARP_STAG_KEY:
                    if (ptr_rule->mask.stag & 0x1)
                    {
                        mask |= 0xfff;
                    }
                    if (ptr_rule->mask.stag & 0x2)
                    {
                        mask |= 0xf000;
                    }
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_ARP_STAG);
                    break;
                case AIR_ACL_ARP_CTAG_KEY:
                    if (ptr_rule->mask.ctag & 0x1)
                    {
                        mask |= 0xfff;
                    }
                    if (ptr_rule->mask.ctag & 0x2)
                    {
                        mask |= 0xf000;
                    }
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag, mask);
                    BIT_ADD(fieldmap, HAL_CORAL_ACL_ARP_CTAG);
                    break;
                case AIR_ACL_ARP_THA_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.tha, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell,
                                                      ARP_THA_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.tha[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell,
                                                      ARP_THA_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      DMAC_WIDTH, ptr_rule->key.tha[n], mask);
                    }
                    break;
                case AIR_ACL_ARP_SHA_KEY:
                    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
                    {
                        mask = 0;
                        if (BIT_CHK(ptr_rule->mask.sha, (AIR_MAC_ADDR_LEN - 1 - n)))
                        {
                            mask = 0xff;
                        }
                        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell,
                                                      ARP_SHA_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      SMAC_WIDTH, ptr_rule->key.sha[n], mask);
                        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell,
                                                      ARP_SHA_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                                      SMAC_WIDTH, ptr_rule->key.sha[n], mask);
                    }
                    break;
                case AIR_ACL_ARP_DIP_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.dip, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, ARP_TPA_OFFSET, DIP_WIDTH, ptr_rule->key.dip, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, ARP_TPA_OFFSET, DIP_WIDTH, ptr_rule->key.dip, mask);
                    break;
                case AIR_ACL_ARP_SIP_KEY:
                    HAL_CORAL_ACL_BYTE_TO_BIT_MASK(4, ptr_rule->mask.sip, mask);
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, ARP_SPA_OFFSET, SIP_WIDTH, ptr_rule->key.sip, mask);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, ARP_SPA_OFFSET, SIP_WIDTH, ptr_rule->key.sip, mask);
                    break;
                case AIR_ACL_ARP_UDF_KEY:
                    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE2_UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                                  ptr_rule->mask.udf);
                    HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, RULE2_UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                                  ptr_rule->mask.udf);
                    break;
                default:
                    /* do nothing */
                    break;
            }
        }
    }
    if (HAL_CORAL_ACL_ARP_NO_FIELDMAP_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        /* only ECO version support fieldmap */
        HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE2_FIELDMAP_OFFSET, RULE2_FIELDMAP_WIDTH, fieldmap, fieldmap);
        HAL_CORAL_ACL_RULL_FILL_CCELL(ptr_ccell, RULE2_FIELDMAP_OFFSET, RULE2_FIELDMAP_WIDTH, fieldmap, fieldmap);
    }
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_rule->portmap, macportmap);
    HAL_CORAL_ACL_RULL_FILL_TCELL(ptr_tcell, RULE2_PORTMAP_OFFSET, PORTMAP_WIDTH, macportmap[0],
                                  BITS(0, (PORTMAP_WIDTH - 1)));

#ifdef AIR_EN_DEBUG
    for (i = 0; i < HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT; i++)
    {
        /* T/C cell print for debugging */
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:t acl data[%d]: 0x%08x\n", i, ptr_tcell[i]);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:c acl data[%d]: 0x%08x\n", i, ptr_ccell[i]);
    }
#endif
}

static void
_hal_coral_acl_getRuleFormat2(
    const UI32_T        unit,
    AIR_ACL_ARP_RULE_T *ptr_rule,
    UI32_T             *ptr_tcell,
    UI32_T             *ptr_ccell)
{
    UI32_T            data = 0, n = 0;
    AIR_PORT_BITMAP_T macportmap = {0};

    /* rule entry conversion */
    /* DMAC */
    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, DMAC_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                         DMAC_WIDTH, ptr_rule->key.dmac[n], data);
        if (0xff == data)
        {
            BIT_ADD(ptr_rule->mask.dmac, (AIR_MAC_ADDR_LEN - 1 - n));
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_DMAC_KEY);
    /* SMAC */
    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, SMAC_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                         SMAC_WIDTH, ptr_rule->key.smac[n], data);
        if (0xff == data)
        {
            BIT_ADD(ptr_rule->mask.smac, (AIR_MAC_ADDR_LEN - 1 - n));
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_SMAC_KEY);
    /* STAG and CTAG format different in ECO version */
    if (HAL_CORAL_ACL_ARP_NO_FIELDMAP_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        data = 0;
        DIAG_PRINT(HAL_DBG_INFO, "acl arp rule support fieldmap.\n");
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, RULE2_FIELDMAP_OFFSET, RULE2_FIELDMAP_WIDTH, data, n);
        if (BIT_CHK(data, HAL_CORAL_ACL_ARP_STAG))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_STAG_KEY);
        }
        if (BIT_CHK(data, HAL_CORAL_ACL_ARP_CTAG))
        {
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_CTAG_KEY);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "acl arp rule not support fieldmap.\n");
        BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_STAG_KEY);
        BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_CTAG_KEY);
    }
    /* STAG */
    if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_ARP_STAG_KEY))
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, STAG_OFFSET, STAG_WIDTH, ptr_rule->key.stag, data);
        if (data & 0xfff)
        {
            ptr_rule->mask.stag |= 0x1;
        }
        if (data & 0xf000)
        {
            ptr_rule->mask.stag |= 0x2;
        }
    }
    /* CTAG */
    if (BIT_CHK(ptr_rule->field_valid, AIR_ACL_ARP_CTAG_KEY))
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, CTAG_OFFSET, CTAG_WIDTH, ptr_rule->key.ctag, data);
        if (data & 0xfff)
        {
            ptr_rule->mask.ctag |= 0x1;
        }
        if (data & 0xf000)
        {
            ptr_rule->mask.ctag |= 0x2;
        }
    }
    /* THA */
    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, ARP_THA_OFFSET + DMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                         DMAC_WIDTH, ptr_rule->key.tha[n], data);
        if (0xff == data)
        {
            BIT_ADD(ptr_rule->mask.tha, (AIR_MAC_ADDR_LEN - 1 - n));
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_THA_KEY);
    /* SHA */
    for (n = 0; n < AIR_MAC_ADDR_LEN; n++)
    {
        data = 0;
        HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, ARP_SHA_OFFSET + SMAC_WIDTH * (AIR_MAC_ADDR_LEN - 1 - n),
                                         SMAC_WIDTH, ptr_rule->key.sha[n], data);
        if (0xff == data)
        {
            BIT_ADD(ptr_rule->mask.sha, (AIR_MAC_ADDR_LEN - 1 - n));
        }
    }
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_SHA_KEY);
    /* DIP */
    data = 0;
    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, ARP_TPA_OFFSET, DIP_WIDTH, ptr_rule->key.dip, data);
    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.dip, data);
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_DIP_KEY);
    /* SIP */
    data = 0;
    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, ARP_SPA_OFFSET, SIP_WIDTH, ptr_rule->key.sip, data);
    HAL_CORAL_ACL_BIT_TO_BYTE_MASK(4, ptr_rule->mask.sip, data);
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_SIP_KEY);
    /* UDF */
    HAL_CORAL_ACL_RULL_GET_FROM_CELL(ptr_tcell, ptr_ccell, RULE2_UDF_OFFSET, UDF_WIDTH, ptr_rule->key.udf,
                                     ptr_rule->mask.udf);
    BIT_ADD(ptr_rule->field_valid, AIR_ACL_ARP_UDF_KEY);
    /* PORBITMAP */
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(ptr_tcell, RULE2_PORTMAP_OFFSET, PORTMAP_WIDTH, macportmap[0]);
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, macportmap, ptr_rule->portmap);
}

/* FUNCTION NAME: _hal_coral_acl_setRuleCtrl
 * PURPOSE:
 *      Set ACL rule control.
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *      ipv6_type       --  IPv6 or not
 *      ptr_ctrl        --  Structure of ACL rule control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_coral_acl_setRuleCtrl(
    const UI32_T          unit,
    const UI32_T          rule_idx,
    const BOOL_T          ipv6_type,
    HAL_CORAL_ACL_CTRL_T *ptr_ctrl)
{
    UI32_T data[4] = {0};

    if (TRUE == ptr_ctrl->rule_en)
    {
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                               data);
        CMLIB_BITMAP_BIT_ADD(data, rule_idx);
        if (TRUE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_ADD(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }
    else
    {
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                               data);
        CMLIB_BITMAP_BIT_DEL(data, rule_idx);
        if (TRUE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_DEL(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }

    if (TRUE == ptr_ctrl->end)
    {
        osal_memset(data, 0, sizeof(UI32_T) * 4);
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
        if (FALSE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_ADD(data, rule_idx);
        }
        else
        {
            CMLIB_BITMAP_BIT_DEL(data, rule_idx);
            CMLIB_BITMAP_BIT_ADD(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }
    else
    {
        osal_memset(data, 0, sizeof(UI32_T) * 4);
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
        if (FALSE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_DEL(data, rule_idx);
        }
        else
        {
            CMLIB_BITMAP_BIT_DEL(data, rule_idx);
            CMLIB_BITMAP_BIT_DEL(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }

    if (TRUE == ptr_ctrl->reverse)
    {
        osal_memset(data, 0, sizeof(UI32_T) * 4);
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                               data);
        CMLIB_BITMAP_BIT_ADD(data, rule_idx);
        if (TRUE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_ADD(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }
    else
    {
        osal_memset(data, 0, sizeof(UI32_T) * 4);
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                               data);
        CMLIB_BITMAP_BIT_DEL(data, rule_idx);
        if (TRUE == ipv6_type)
        {
            CMLIB_BITMAP_BIT_DEL(data, rule_idx + 1);
        }
        _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                                data);
    }

    if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        if (TRUE == ptr_ctrl->end_reverse)
        {
            osal_memset(data, 0, sizeof(UI32_T) * 4);
            _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE, 1, 0, 0,
                                   HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
            if (FALSE == ipv6_type)
            {
                CMLIB_BITMAP_BIT_ADD(data, rule_idx);
            }
            else
            {
                CMLIB_BITMAP_BIT_DEL(data, rule_idx);
                CMLIB_BITMAP_BIT_ADD(data, rule_idx + 1);
            }
            _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE, 1, 0, 0,
                                    HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE, data);
        }
        else
        {
            osal_memset(data, 0, sizeof(UI32_T) * 4);
            _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE, 1, 0, 0,
                                   HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
            if (FALSE == ipv6_type)
            {
                CMLIB_BITMAP_BIT_DEL(data, rule_idx);
            }
            else
            {
                CMLIB_BITMAP_BIT_DEL(data, rule_idx);
                CMLIB_BITMAP_BIT_DEL(data, rule_idx + 1);
            }
            _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE, 1, 0, 0,
                                    HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE, data);
        }
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_coral_acl_getRuleCtrl
 * PURPOSE:
 *      Get ACL rule control.
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 * OUTPUT:
 *      ptr_ctrl        --  Structure of ACL rule control
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_coral_acl_getRuleCtrl(
    const UI32_T          unit,
    const UI32_T          rule_idx,
    HAL_CORAL_ACL_CTRL_T *ptr_ctrl)
{
    UI32_T data[4] = {0};

    _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
    if (CMLIB_BITMAP_BIT_CHK(data, rule_idx))
    {
        ptr_ctrl->rule_en = TRUE;
    }

    osal_memset(data, 0, sizeof(UI32_T) * 4);
    _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
    if (CMLIB_BITMAP_BIT_CHK(data, rule_idx))
    {
        ptr_ctrl->end = TRUE;
    }

    osal_memset(data, 0, sizeof(UI32_T) * 4);
    _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ, data);
    if (CMLIB_BITMAP_BIT_CHK(data, rule_idx))
    {
        ptr_ctrl->reverse = TRUE;
    }
    if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        osal_memset(data, 0, sizeof(UI32_T) * 4);
        _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_SEC_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                               data);
        if (CMLIB_BITMAP_BIT_CHK(data, rule_idx))
        {
            ptr_ctrl->end_reverse = TRUE;
        }
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_coral_acl_getRuleUsage
 * PURPOSE:
 *      Get the acl rules usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of acl rules
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getRuleUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    UI32_T hw_entry_bmp[HAL_CORAL_ACL_ENTRY_NUM_WORDS] = {0};

    _hal_coral_acl_readReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_READ,
                           hw_entry_bmp);
    CMLIB_BITMAP_COUNT(hw_entry_bmp, *ptr_cnt, HAL_CORAL_ACL_ENTRY_NUM_WORDS);
}

/* FUNCTION NAME: _hal_coral_acl_getActionUsage
 * PURPOSE:
 *      Get the acl action usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of acl actions
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getActionUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           entry_idx, usage_count = 0;
    AIR_ACL_ACTION_T action;

    for (entry_idx = 0; entry_idx < HAL_CORAL_MAX_NUM_OF_ACL_ENTRY; entry_idx++)
    {
        rc = hal_coral_acl_getAction(unit, entry_idx, &action);
        if ((AIR_E_OK == rc) && (action.field_valid > 0))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_coral_acl_getUdfEntryUsage
 * PURPOSE:
 *      Get the acl udf entries usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of udf entries
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getUdfUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             entry_idx, usage_count = 0;
    AIR_ACL_UDF_RULE_T udf_rule;

    for (entry_idx = 0; entry_idx < HAL_CORAL_MAX_NUM_OF_UDF_ENTRY; entry_idx++)
    {
        rc = hal_coral_acl_getUdfRule(unit, entry_idx, &udf_rule);
        if ((AIR_E_OK == rc) && (TRUE == udf_rule.udf_rule_en))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_coral_acl_getCounterUsage
 * PURPOSE:
 *      Get the acl mib counters usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of mib counters
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getCounterUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    UI32_T                 entry_idx;
    UI32_T                 data[4] = {0};
    HAL_CORAL_ACL_ACTION_T action_entry = {0};
    UI32_T                 mib_bmp[HAL_CORAL_ACL_COUNTER_NUM_WORDS] = {0};

    for (entry_idx = 0; entry_idx < HAL_CORAL_MAX_NUM_OF_ACL_ENTRY; entry_idx++)
    {
        _hal_coral_acl_readReg(unit, entry_idx, 1, 0, HAL_CORAL_ACL_MEM_SEL_ACTION, HAL_CORAL_ACL_MEM_FUNC_READ, data);
        _hal_coral_acl_getActionTable(data, &action_entry);
        if (TRUE == action_entry.cnt_en)
        {
            CMLIB_BITMAP_BIT_ADD(mib_bmp, action_entry.cnt_idx);
        }
    }
    CMLIB_BITMAP_COUNT(mib_bmp, *ptr_cnt, HAL_CORAL_ACL_COUNTER_NUM_WORDS);
}

/* FUNCTION NAME: _hal_coral_acl_getMeterUsage
 * PURPOSE:
 *      Get the acl mib counters usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of meters
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getMeterUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         entry_idx, usage_count = 0;
    BOOL_T         enable;
    UI32_T         rate = 0;

    for (entry_idx = 0; entry_idx < HAL_CORAL_MAX_NUM_OF_METER_ID; entry_idx++)
    {
        rc = hal_coral_acl_getMeterTable(unit, entry_idx, &enable, &rate);
        if ((AIR_E_OK == rc) && (TRUE == enable))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: _hal_coral_acl_getTrtcmUsage
 * PURPOSE:
 *      Get the acl Trtcm entry usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of trtcm entries
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_coral_acl_getTrtcmUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          entry_idx, usage_count = 0;
    AIR_ACL_TRTCM_T tcm;

    for (entry_idx = 0; entry_idx < HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY; entry_idx++)
    {
        rc = hal_coral_acl_getTrtcmTable(unit, entry_idx, &tcm);
        if ((AIR_E_OK == rc) && ((tcm.cbs != 0) || (tcm.pbs != 0) || (tcm.cir != 0) || (tcm.pir != 0)))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:
 *      hal_coral_acl_getGlobalState
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
hal_coral_acl_getGlobalState(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value;

    /* get value from CHIP*/
    rc = aml_readReg(unit, CORAL_ACL_GLOBAL_CFG, &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        value &= CORAL_ACL_EN_MASK;
        *ptr_enable = value ? TRUE : FALSE;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setGlobalState
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
hal_coral_acl_setGlobalState(
    const UI32_T unit,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, data = 0;

    value = (enable != 0) ? 1 : 0;
    rc = aml_readReg(unit, CORAL_ACL_GLOBAL_CFG, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    data = (data & ~CORAL_ACL_EN_MASK) | value;
    return aml_writeReg(unit, CORAL_ACL_GLOBAL_CFG, &data, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getPortState
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
hal_coral_acl_getPortState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_ACL_PORT_EN, &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        value = (value >> mac_port) & 0x1;
        *ptr_enable = value ? TRUE : FALSE;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setPortState
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
hal_coral_acl_setPortState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, data = 0, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    value = (enable != 0) ? 1 : 0;
    rc = aml_readReg(unit, CORAL_ACL_PORT_EN, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    data = (data & ~(CORAL_ACL_EN_MASK << mac_port)) | (value << mac_port);
    return aml_writeReg(unit, CORAL_ACL_PORT_EN, &data, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_clearAll
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
hal_coral_acl_clearAll(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value = 0;
    UI32_T         data[4] = {0};

    /* clear all rules */
    value = (HAL_CORAL_ACL_MEM_FUNC_CLEAR << CORAL_ACL_MEM_CFG_FUNC_SEL_OFFSET) | CORAL_ACL_MEM_CFG_EN;
    rc = aml_writeReg(unit, CORAL_ACL_MEM_CFG, &value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* clear all actions */
    value = (HAL_CORAL_ACL_MEM_SEL_ACTION << CORAL_ACL_MEM_CFG_MEM_SEL_OFFSET) |
            (HAL_CORAL_ACL_MEM_FUNC_CLEAR << CORAL_ACL_MEM_CFG_FUNC_SEL_OFFSET) | CORAL_ACL_MEM_CFG_EN;
    rc = aml_writeReg(unit, CORAL_ACL_MEM_CFG, &value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* clear acl config */
    _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE, data);
    _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_END, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE, data);
    _hal_coral_acl_writeReg(unit, HAL_CORAL_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, HAL_CORAL_ACL_MEM_FUNC_CONFIG_WRITE,
                            data);
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getRule
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
hal_coral_acl_getRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0}, type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    UI32_T         acl_entry_idx = 0;
    HAL_CORAL_ACL_RULE_TYPE_T type = HAL_CORAL_ACL_RULE_TYPE_0;
    HAL_CORAL_ACL_CTRL_T      ctrl;

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PTR(ptr_rule);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    osal_memset(ptr_rule, 0, sizeof(AIR_ACL_RULE_T));
    osal_memset(&ctrl, 0, sizeof(HAL_CORAL_ACL_CTRL_T));

    _hal_coral_acl_getRuleType(unit, acl_entry_idx, &type);
    if (HAL_CORAL_ACL_RULE_TYPE_2 == type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "acl rule entry %d is not an ACL rule.\n", entry_idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }
    else if (HAL_CORAL_ACL_RULE_TYPE_1 == type)
    {
        _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx, &ctrl);
        ptr_rule->rule_en = ctrl.rule_en;
        ptr_rule->reverse = ctrl.reverse;
        ptr_rule->end = ctrl.end;
        if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
        {
            ptr_rule->end_reverse = ctrl.end_reverse;
        }
        _hal_coral_acl_getfromCell(unit, acl_entry_idx, type_t, type_c);
        _hal_coral_acl_getRuleFormat1(unit, ptr_rule, type_t, type_c);
        if ((acl_entry_idx % HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP) > 0)
        {
            /* If rule contains IPv6 DIP or SIP, need to get another entry */
            osal_memset(&ctrl, 0, sizeof(HAL_CORAL_ACL_CTRL_T));
            _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx - 1, &ctrl);
            if (FALSE == ctrl.end)
            {
                osal_memset(type_t, 0, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
                osal_memset(type_c, 0, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
                _hal_coral_acl_getfromCell(unit, acl_entry_idx - 1, type_t, type_c);
                HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(type_t, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, type);
                if (HAL_CORAL_ACL_RULE_TYPE_0 == type)
                {
                    _hal_coral_acl_getRuleFormat0(unit, ptr_rule, type_t, type_c);
                }
            }
        }
        else
        {
            /* format 1 has no field_enable key, check the value exist */
            ptr_rule->isipv6 = TRUE;
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY);
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY);
            BIT_ADD(ptr_rule->field_valid, AIR_ACL_FLOW_LABEL_KEY);
        }
    }
    else
    {
        _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx, &ctrl);
        ptr_rule->rule_en = ctrl.rule_en;
        ptr_rule->reverse = ctrl.reverse;
        ptr_rule->end = ctrl.end;
        if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
        {
            ptr_rule->end_reverse = ctrl.end_reverse;
        }
        _hal_coral_acl_getfromCell(unit, acl_entry_idx, type_t, type_c);
        _hal_coral_acl_getRuleFormat0(unit, ptr_rule, type_t, type_c);
        if ((FALSE == ctrl.end) && (acl_entry_idx + 1 < HAL_CORAL_MAX_NUM_OF_ACL_ENTRY) && (TRUE == ptr_rule->isipv6))
        {
            osal_memset(type_t, 0, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
            osal_memset(type_c, 0, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
            _hal_coral_acl_getfromCell(unit, acl_entry_idx + 1, type_t, type_c);
            HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(type_t, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, type);
            if (HAL_CORAL_ACL_RULE_TYPE_1 == type)
            {
                _hal_coral_acl_getRuleFormat1(unit, ptr_rule, type_t, type_c);
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setRule
 * PURPOSE:
 *      Add ACL rule of specified entry index.
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
 *      ACL will hit each rule, then reverse and then aggregate.
 */
AIR_ERROR_NO_T
hal_coral_acl_setRule(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    UI32_T               type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    HAL_CORAL_ACL_CTRL_T ctrl;
    UI32_T               acl_entry_idx = 0;
    BOOL_T               ipv6_type = FALSE;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_ACL_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dmac, 0, HAL_CORAL_MAX_NUM_OF_DMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.smac, 0, HAL_CORAL_MAX_NUM_OF_SMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.etype, 0, HAL_CORAL_MAX_NUM_OF_ETYPE_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.stag, 0, HAL_CORAL_MAX_NUM_OF_STAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.ctag, 0, HAL_CORAL_MAX_NUM_OF_CTAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.flow_label, 0, HAL_CORAL_MAX_NUM_OF_FLOW_LABEL_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dip, 0, HAL_CORAL_MAX_NUM_OF_DIP_MASK_IPV4);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sip, 0, HAL_CORAL_MAX_NUM_OF_SIP_MASK_IPV4);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dport, 0, HAL_CORAL_MAX_NUM_OF_DPORT_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sport, 0, HAL_CORAL_MAX_NUM_OF_SPORT_MASK);
    HAL_CHECK_PORT_BITMAP(unit, ptr_rule->portmap);
    HAL_CHECK_BOOL(ptr_rule->isipv6);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dipv6, 0, HAL_CORAL_MAX_NUM_OF_DIP_MASK_IPV6);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sipv6, 0, HAL_CORAL_MAX_NUM_OF_SIP_MASK_IPV6);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    if ((BIT_CHK(ptr_rule->field_valid, AIR_ACL_FLOW_LABEL_KEY) || BIT_CHK(ptr_rule->field_valid, AIR_ACL_DIPV6_KEY) ||
         BIT_CHK(ptr_rule->field_valid, AIR_ACL_SIPV6_KEY)))
    {
        HAL_CHECK_PARAM(
            (HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP - 1) == (acl_entry_idx % HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP),
            AIR_E_BAD_PARAMETER);
        HAL_CHECK_PARAM(FALSE == ptr_rule->isipv6, AIR_E_BAD_PARAMETER);
        ipv6_type = TRUE;
    }

    /* Reset to all as don't care in t/c cell to improve performance */
    osal_memset(type_t, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
    osal_memset(type_c, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
    /* rule entry conversion ad set to Cell */
    _hal_coral_acl_fillRuleFormat0(unit, ptr_rule, type_t, type_c);
    _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);
    if (TRUE == ipv6_type)
    {
        /* Reset to all as don't care in t/c cell to improve performance */
        osal_memset(type_t, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
        osal_memset(type_c, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
        /* rule entry conversion ad set to Cell */
        _hal_coral_acl_fillRuleFormat1(unit, ptr_rule, type_t, type_c);
        _hal_coral_acl_settoCell(unit, acl_entry_idx + 1, type_t, type_c);
    }

    /* Config rule enable/end/rev */
    ctrl.rule_en = ptr_rule->rule_en;
    if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        ctrl.end_reverse = ptr_rule->end_reverse;
    }
    if ((TRUE == ptr_rule->reverse) && (TRUE == ipv6_type) &&
        (0 ==
         (ptr_rule->field_valid & ~(BIT(AIR_ACL_DIPV6_KEY) | BIT(AIR_ACL_SIPV6_KEY) | BIT(AIR_ACL_FLOW_LABEL_KEY)))) &&
        ((0 == (ptr_rule->field_valid & ~BIT(AIR_ACL_FLOW_LABEL_KEY))) ||
         ((0 == (ptr_rule->mask.dipv6 & 0xf)) && (0 == (ptr_rule->mask.sipv6 & 0xf)))))
    {
        /* only enable flow_label or ipv6 ignore_foramt0 is true and set the deny rules */
        ctrl.reverse = FALSE;
        ctrl.end = FALSE;
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, FALSE, &ctrl);
        ctrl.reverse = TRUE;
        ctrl.end = ptr_rule->end;
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx + 1, FALSE, &ctrl);
    }
    else if ((TRUE == ptr_rule->reverse) && (TRUE == ipv6_type) &&
             !BIT_CHK(ptr_rule->field_valid, HAL_CORAL_ACL_FLOW_LABEL) && (0 == (ptr_rule->mask.dipv6 & 0xfff0)) &&
             (0 == (ptr_rule->mask.sipv6 & 0xfff0)))
    {
        /* incase to only reverse the format0 rule for deny ipv6 DIP/SIP address */
        ctrl.reverse = TRUE;
        ctrl.end = FALSE;
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, FALSE, &ctrl);
        ctrl.reverse = FALSE;
        ctrl.end = ptr_rule->end;
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx + 1, FALSE, &ctrl);
    }
    else
    {
        ctrl.reverse = ptr_rule->reverse;
        ctrl.end = ptr_rule->end;
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, ipv6_type, &ctrl);
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_delRule
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
hal_coral_acl_delRule(
    const UI32_T unit,
    const UI32_T entry_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0}, type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    UI32_T         acl_entry_idx = 0;
    BOOL_T         ipv6_type = FALSE;
    HAL_CORAL_ACL_RULE_TYPE_T type = HAL_CORAL_ACL_RULE_TYPE_0;
    HAL_CORAL_ACL_CTRL_T      ctrl = {0};

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    rc = _hal_coral_acl_getRuleType(unit, acl_entry_idx, &type);
    if (HAL_CORAL_ACL_RULE_TYPE_2 == type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Info]:acl rule entry %d is not found or is an ACL-ARP rule.\n", entry_idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }
    else
    {
        /* If rule contains IPv6 DIP or SIP, need to get another entry */
        if (HAL_CORAL_ACL_RULE_TYPE_1 == type)
        {
            if ((acl_entry_idx % HAL_CORAL_MAX_RULE_NUM_OF_SUBGROUP) > 0)
            {
                _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx - 1, &ctrl);
                if (FALSE == ctrl.end)
                {
                    /* Make sure the rules are aggregated */
                    rc = _hal_coral_acl_getRuleType(unit, acl_entry_idx - 1, &type);
                    if ((AIR_E_OK == rc) && (HAL_CORAL_ACL_RULE_TYPE_0 == type))
                    {
                        ipv6_type = TRUE;
                    }
                }
                /* Delete the entry from ACL rule table */
                osal_memset(&ctrl, 0, sizeof(HAL_CORAL_ACL_CTRL_T));
            }
            if (TRUE == ipv6_type)
            {
                _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx - 1, ipv6_type, &ctrl);
                _hal_coral_acl_settoCell(unit, acl_entry_idx - 1, type_t, type_c);
                _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);
            }
            else
            {
                _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, ipv6_type, &ctrl);
                _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);
            }
        }
        else
        {
            _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx, &ctrl);
            if (FALSE == ctrl.end)
            {
                /* Check if the rules are aggregated for ipv6 rule */
                rc = _hal_coral_acl_getRuleType(unit, acl_entry_idx + 1, &type);
                if ((AIR_E_OK == rc) && (HAL_CORAL_ACL_RULE_TYPE_1 == type))
                {
                    ipv6_type = TRUE;
                }
            }

            /* Delete the entry from ACL rule table */
            osal_memset(&ctrl, 0, sizeof(HAL_CORAL_ACL_CTRL_T));
            _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, ipv6_type, &ctrl);
            _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);
            if (TRUE == ipv6_type)
            {
                _hal_coral_acl_settoCell(unit, acl_entry_idx + 1, type_t, type_c);
            }
        }
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getAction
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
hal_coral_acl_getAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action)
{
    UI32_T                 data[4] = {0};
    HAL_CORAL_ACL_ACTION_T action_entry = {0};
    UI32_T                 mirror_en = 0, drop_pcd_en = 0, class_slr_en = 0, trtcm_idx_en = 0;
    UI32_T                 acl_entry_idx = 0;

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PTR(ptr_action);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    _hal_coral_acl_readReg(unit, acl_entry_idx, 1, 0, HAL_CORAL_ACL_MEM_SEL_ACTION, HAL_CORAL_ACL_MEM_FUNC_READ, data);
    _hal_coral_acl_getActionTable(data, &action_entry);

    /* action conversion */
    if (TRUE == action_entry.port_en)
    {
        ptr_action->dst_port_swap = action_entry.dest_port_sel;
        ptr_action->vlan_port_swap = action_entry.vlan_port_sel;
        HAL_MAC_PBMP_TO_AIR_PBMP(unit, action_entry.portmap, ptr_action->portmap);
    }
    if (TRUE == action_entry.cnt_en)
    {
        ptr_action->acl_mib_id = action_entry.cnt_idx;
    }
    if (TRUE == action_entry.attack_en)
    {
        ptr_action->attack_rate_id = action_entry.attack_idx;
    }
    if (TRUE == action_entry.rate_en)
    {
        ptr_action->meter_id = action_entry.rate_idx;
    }
    if (TRUE == action_entry.vlan_en)
    {
        ptr_action->acl_vlan_vid = action_entry.vlan_idx;
    }
    ptr_action->mirror_port = action_entry.mirrormap;
    mirror_en = action_entry.mirrormap ? 1 : 0;
    if (TRUE == action_entry.pri_user_en)
    {
        ptr_action->pri_user = action_entry.pri_user;
    }
    if (TRUE == action_entry.lyvlan_en)
    {
        ptr_action->lky_vlan = action_entry.lyvlan;
    }
    ptr_action->acl_mang = action_entry.mang;
    if (TRUE == action_entry.fwd_en)
    {
        ptr_action->port_fw = action_entry.fwd;
    }
    if (TRUE == action_entry.egtag_en)
    {
        ptr_action->eg_tag = action_entry.egtag;
    }
    if (TRUE == action_entry.trtcm_en)
    {
        ptr_action->class_slr_sel = action_entry.trtcm.cls_slr_sel;
        ptr_action->class_slr = action_entry.trtcm.cls_slr;
        ptr_action->drop_pcd_sel = action_entry.trtcm.drop_pcd_sel;
        ptr_action->drop_pcd_g = action_entry.trtcm.drop_pcd_g;
        ptr_action->drop_pcd_y = action_entry.trtcm.drop_pcd_y;
        ptr_action->drop_pcd_r = action_entry.trtcm.drop_pcd_r;
        ptr_action->acl_tcm_sel = action_entry.trtcm.tcm_sel;
        ptr_action->def_color = action_entry.trtcm.usr_tcm;
        ptr_action->trtcm_id = action_entry.trtcm.tcm_idx;
        drop_pcd_en = 1;
        class_slr_en = 1;
        trtcm_idx_en = 1;
    }
    ptr_action->dont_learn = action_entry.dont_learn;
    ptr_action->as_bpdu = action_entry.bpdu;
    ptr_action->field_valid = (action_entry.port_en << AIR_ACL_PORT) | (action_entry.cnt_en << AIR_ACL_MIB) |
                              (action_entry.attack_en << AIR_ACL_ATTACK) | (action_entry.rate_en << AIR_ACL_METER) |
                              (action_entry.vlan_en << AIR_ACL_VLAN) | (mirror_en << AIR_ACL_MIRROR) |
                              (action_entry.pri_user_en << AIR_ACL_PRI) | (action_entry.fwd_en << AIR_ACL_FW_PORT) |
                              (action_entry.lyvlan_en << AIR_ACL_LKY_VLAN) | (action_entry.mang << AIR_ACL_MANG) |
                              (action_entry.egtag_en << AIR_ACL_EGTAG) | (drop_pcd_en << AIR_ACL_DROP_PCD) |
                              (class_slr_en << AIR_ACL_CLASS_SLR) | (trtcm_idx_en << AIR_ACL_TRTCM) |
                              (action_entry.dont_learn << AIR_ACL_DONT_LEARN) | (action_entry.bpdu << AIR_ACL_BPDU);
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setAction
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
hal_coral_acl_setAction(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action)
{
    UI32_T                 data[4] = {0};
    UI32_T                 acl_entry_idx = 0;
    I32_T                  i;
    HAL_CORAL_ACL_ACTION_T action_entry = {0};

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_ACL_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->acl_mib_id, 0, HAL_CORAL_MAX_NUM_OF_MIB_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->attack_rate_id, 0, HAL_CORAL_MAX_NUM_OF_ATTACK_RATE_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->meter_id, 0, HAL_CORAL_MAX_NUM_OF_METER_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->mirror_port, 0, HAL_CORAL_MAX_NUM_OF_MIRROR_PORT);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->pri_user, 0, HAL_CORAL_MAX_NUM_OF_USER_PRI - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_r, 0, HAL_CORAL_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_y, 0, HAL_CORAL_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->drop_pcd_g, 0, HAL_CORAL_MAX_NUM_OF_DROP_PCD - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->class_slr, 0, HAL_CORAL_MAX_NUM_OF_CLASS_SLR - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->trtcm_id, 0, HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->acl_vlan_vid, 0, HAL_CORAL_MAX_NUM_OF_VLAN_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->port_fw, 0, BITS_RANGE(0, FW_PORT_WIDTH));
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->eg_tag, 0, BITS_RANGE(0, EG_TAG_WIDTH));
    HAL_CHECK_MIN_MAX_RANGE(ptr_action->def_color, 0, BITS_RANGE(0, ACL_TCM_WIDTH));
    HAL_CHECK_PORT_BITMAP(unit, ptr_action->portmap);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    for (i = AIR_ACL_PORT; i < AIR_ACL_ACTION_FIELD_LAST; i++)
    {
        if ((1U << i) & ptr_action->field_valid)
        {
            switch (i)
            {
                case AIR_ACL_PORT:
                    action_entry.port_en = TRUE;
                    action_entry.dest_port_sel = ptr_action->dst_port_swap;
                    action_entry.vlan_port_sel = ptr_action->vlan_port_swap;
                    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_action->portmap, action_entry.portmap);
                    break;
                case AIR_ACL_MIB:
                    action_entry.cnt_en = TRUE;
                    action_entry.cnt_idx = ptr_action->acl_mib_id;
                    break;
                case AIR_ACL_ATTACK:
                    action_entry.attack_en = TRUE;
                    action_entry.attack_idx = ptr_action->attack_rate_id;
                    break;
                case AIR_ACL_METER:
                    action_entry.rate_en = TRUE;
                    action_entry.rate_idx = ptr_action->meter_id;
                    break;
                case AIR_ACL_FW_PORT:
                    action_entry.fwd_en = TRUE;
                    action_entry.fwd = ptr_action->port_fw;
                    break;
                case AIR_ACL_MIRROR:
                    action_entry.mirrormap = ptr_action->mirror_port;
                    break;
                case AIR_ACL_PRI:
                    action_entry.pri_user_en = TRUE;
                    action_entry.pri_user = ptr_action->pri_user;
                    break;
                case AIR_ACL_EGTAG:
                    action_entry.egtag_en = TRUE;
                    action_entry.egtag = ptr_action->eg_tag;
                    break;
                case AIR_ACL_LKY_VLAN:
                    action_entry.lyvlan_en = TRUE;
                    action_entry.lyvlan = ptr_action->lky_vlan;
                    break;
                case AIR_ACL_MANG:
                    action_entry.mang = ptr_action->acl_mang;
                    break;
                case AIR_ACL_DROP_PCD:
                    action_entry.trtcm_en = TRUE;
                    action_entry.trtcm.drop_pcd_sel = ptr_action->drop_pcd_sel;
                    action_entry.trtcm.drop_pcd_g = ptr_action->drop_pcd_g;
                    action_entry.trtcm.drop_pcd_y = ptr_action->drop_pcd_y;
                    action_entry.trtcm.drop_pcd_r = ptr_action->drop_pcd_r;
                    break;
                case AIR_ACL_CLASS_SLR:
                    action_entry.trtcm_en = TRUE;
                    action_entry.trtcm.cls_slr_sel = ptr_action->class_slr_sel;
                    action_entry.trtcm.cls_slr = ptr_action->class_slr;
                    break;
                case AIR_ACL_TRTCM:
                    action_entry.trtcm_en = TRUE;
                    action_entry.trtcm.tcm_sel = ptr_action->acl_tcm_sel;
                    action_entry.trtcm.usr_tcm = ptr_action->def_color;
                    action_entry.trtcm.tcm_idx = ptr_action->trtcm_id;
                    break;
                case AIR_ACL_VLAN:
                    action_entry.vlan_en = TRUE;
                    action_entry.vlan_idx = ptr_action->acl_vlan_vid;
                    break;
                case AIR_ACL_DONT_LEARN:
                    action_entry.dont_learn = ptr_action->dont_learn;
                    break;
                case AIR_ACL_BPDU:
                    action_entry.bpdu = ptr_action->as_bpdu;
                    break;
                case AIR_ACL_MAC_CHG:
                    return AIR_E_NOT_SUPPORT;
                default:
                    return AIR_E_BAD_PARAMETER;
            }
        }
    }

    _hal_coral_acl_setActionTable(&action_entry, data);
    _hal_coral_acl_writeReg(unit, acl_entry_idx, 1, 0, HAL_CORAL_ACL_MEM_SEL_ACTION, HAL_CORAL_ACL_MEM_FUNC_WRITE,
                            data);
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_coral_acl_delAction
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
hal_coral_acl_delAction(
    const UI32_T unit,
    const UI32_T entry_idx)
{
    UI32_T data[4] = {0};
    UI32_T acl_entry_idx = 0;

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);
    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);
    _hal_coral_acl_writeReg(unit, acl_entry_idx, 1, 0, HAL_CORAL_ACL_MEM_SEL_ACTION, HAL_CORAL_ACL_MEM_FUNC_WRITE,
                            data);
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getMibCnt
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
 *      ACL has total 64 mib counters, the counter index can be set by hal_coral_acl_setAction.
 */
AIR_ERROR_NO_T
hal_coral_acl_getMibCnt(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(cnt_index, 0, HAL_CORAL_MAX_NUM_OF_MIB_ID - 1);
    value = (cnt_index & CORAL_ACL_MIB_SEL_MASK) << CORAL_ACL_MIB_SEL_OFFSET;
    rc = aml_writeReg(unit, CORAL_ACL_MIB_CNT_CFG, &value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    return aml_readReg(unit, CORAL_ACL_MIB_CNT, ptr_cnt, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_clearMibCnt
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
 *      For clear all mib counters, ACL has total 64 mib counters, the counter index can be set by
 *      hal_coral_acl_setAction.
 */
AIR_ERROR_NO_T
hal_coral_acl_clearMibCnt(
    const UI32_T unit)
{
    UI32_T value = CORAL_ACL_MIB_CLEAR;
    return aml_writeReg(unit, CORAL_ACL_MIB_CNT_CFG, &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getUdfRule
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
hal_coral_acl_getUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            value;
    UI32_T            data[3];
    AIR_PORT_BITMAP_T air_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_UDF_ENTRY - 1);
    value = (entry_idx & CORAL_ACL_UDF_ADDR_MASK) | CORAL_ACL_UDF_READ | (1U << CORAL_ACL_UDF_ACC_OFFSET);
    rc = aml_writeReg(unit, CORAL_ACL_AUTC, &value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_UDF))
    {
        return AIR_E_TIMEOUT;
    }
    rc = aml_readReg(unit, CORAL_ACL_AUTR0, data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_readReg(unit, CORAL_ACL_AUTR1, data + 1, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_readReg(unit, CORAL_ACL_AUTR2, data + 2, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, air_portmap[0]);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->cmp_mask);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->cmp_pat);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, ptr_udf_rule->udf_rule_en);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, ptr_udf_rule->udf_pkt_type);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, WORD_OFST_OFFSET, WORD_OFST_WIDTH, ptr_udf_rule->word_ofst);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_SEL_OFFSET, CMP_SEL_WIDTH, ptr_udf_rule->cmp_sel);
    HAL_MAC_PBMP_TO_AIR_PBMP(unit, air_portmap, ptr_udf_rule->portmap);
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setUdfRule
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
hal_coral_acl_setUdfRule(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            value = 0;
    UI32_T            data[3] = {0};
    AIR_PORT_BITMAP_T mac_portmap;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_UDF_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->udf_pkt_type, 0, HAL_CORAL_MAX_NUM_OF_OFST_TP - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->word_ofst, 0, HAL_CORAL_MAX_NUM_OF_WORD_OFST - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_sel, 0, HAL_CORAL_MAX_NUM_OF_CMP_SEL - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_pat, 0, HAL_CORAL_MAX_NUM_OF_CMP_PAT);
    HAL_CHECK_MIN_MAX_RANGE(ptr_udf_rule->cmp_mask, 0, HAL_CORAL_MAX_NUM_OF_CMP_BIT);
    HAL_CHECK_PORT_BITMAP(unit, ptr_udf_rule->portmap);
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_udf_rule->portmap, mac_portmap);

    _hal_coral_acl_fillTblMultiFields(data, 3, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, mac_portmap[0]);
    _hal_coral_acl_fillTblMultiFields(data, 3, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->cmp_mask);
    _hal_coral_acl_fillTblMultiFields(data, 3, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->cmp_pat);
    _hal_coral_acl_fillTblMultiFields(data, 3, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, ptr_udf_rule->udf_rule_en);
    _hal_coral_acl_fillTblMultiFields(data, 3, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, ptr_udf_rule->udf_pkt_type);
    _hal_coral_acl_fillTblMultiFields(data, 3, WORD_OFST_OFFSET, WORD_OFST_WIDTH, ptr_udf_rule->word_ofst);
    _hal_coral_acl_fillTblMultiFields(data, 3, CMP_SEL_OFFSET, CMP_SEL_WIDTH, ptr_udf_rule->cmp_sel);
    DIAG_PRINT(HAL_DBG_INFO, "========acl udf rule=======\n");
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data0: 0x%08x\n", data[0]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data1: 0x%08x\n", data[1]);
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]:acl data2: 0x%08x\n", data[2]);

    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_UDF))
    {
        return AIR_E_TIMEOUT;
    }

    rc = aml_writeReg(unit, CORAL_ACL_AUTW0, data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_writeReg(unit, CORAL_ACL_AUTW1, data + 1, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_writeReg(unit, CORAL_ACL_AUTW2, data + 2, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    value = (entry_idx & CORAL_ACL_UDF_ADDR_MASK) | CORAL_ACL_UDF_WRITE | (1U << CORAL_ACL_UDF_ACC_OFFSET);
    return aml_writeReg(unit, CORAL_ACL_AUTC, &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_delUdfRule
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
hal_coral_acl_delUdfRule(
    const UI32_T unit,
    const UI8_T  entry_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value;
    UI32_T         data = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_UDF_ENTRY - 1);
    rc = aml_writeReg(unit, CORAL_ACL_AUTW0, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_writeReg(unit, CORAL_ACL_AUTW1, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_writeReg(unit, CORAL_ACL_AUTW2, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    value = (entry_idx & CORAL_ACL_UDF_ADDR_MASK) | CORAL_ACL_UDF_WRITE | (1U << CORAL_ACL_UDF_ACC_OFFSET);
    return aml_writeReg(unit, CORAL_ACL_AUTC, &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_clearUdfRule
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
hal_coral_acl_clearUdfRule(
    const UI32_T unit)
{
    UI32_T value = CORAL_ACL_UDF_CLEAR | (1U << CORAL_ACL_UDF_ACC_OFFSET);

    return aml_writeReg(unit, CORAL_ACL_AUTC, &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getMeterTable
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
hal_coral_acl_getMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(meter_id, 0, HAL_CORAL_MAX_NUM_OF_METER_ID - 1);
    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_METER))
    {
        return AIR_E_TIMEOUT;
    }
    value = (1 << CORAL_ACL_RATE_BUSY_OFFSET) | CORAL_ACL_RATE_READ |
            ((meter_id & CORAL_ACL_RATE_ID_MASK) << CORAL_ACL_RATE_ID_OFFSET) | CORAL_ACL_RATE_EN;
    rc = aml_writeReg(unit, CORAL_ACLRMC, &value, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    rc = aml_readReg(unit, CORAL_ACLRMD1, &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_enable = ((value >> CORAL_ACL_RATE_EN_OFFSET) & 0x1) ? TRUE : FALSE;
        *ptr_rate = value & CORAL_ACL_RATE_TOKEN_MASK;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setMeterTable
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
hal_coral_acl_setMeterTable(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate)
{
    UI32_T value;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(meter_id, 0, HAL_CORAL_MAX_NUM_OF_METER_ID - 1);
    HAL_CHECK_MIN_MAX_RANGE(rate, 0, HAL_CORAL_MAX_NUM_OF_TOKEN);
    if (TRUE == enable)
    {
        value = (1 << CORAL_ACL_RATE_BUSY_OFFSET) | CORAL_ACL_RATE_WRITE |
                ((meter_id & CORAL_ACL_RATE_ID_MASK) << CORAL_ACL_RATE_ID_OFFSET) | CORAL_ACL_RATE_EN |
                (rate & CORAL_ACL_RATE_TOKEN_MASK);
    }
    else if (FALSE == enable)
    {
        value = (1 << CORAL_ACL_RATE_BUSY_OFFSET) | CORAL_ACL_RATE_WRITE |
                ((meter_id & CORAL_ACL_RATE_ID_MASK) << CORAL_ACL_RATE_ID_OFFSET) | CORAL_ACL_RATE_DIS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_METER))
    {
        return AIR_E_TIMEOUT;
    }
    return aml_writeReg(unit, CORAL_ACLRMC, &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getDropEn
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
hal_coral_acl_getDropEn(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_DPCR_EN(mac_port), &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_enable = (value != 0) ? TRUE : FALSE;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setDropEn
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
hal_coral_acl_setDropEn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI32_T value, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    value = (TRUE == enable) ? 1 : 0;
    return aml_writeReg(unit, CORAL_DPCR_EN(mac_port), &value, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getDropThrsh
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
hal_coral_acl_getDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_low = data & CORAL_DPCR_LOW_THRSH_WIDTH;
        *ptr_high = (data >> CORAL_DPCR_HIGH_THRSH_OFFSET) & CORAL_DPCR_HIGH_THRSH_WIDTH;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setDropThrsh
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
hal_coral_acl_setDropThrsh(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data, mac_port = 0;

    HAL_CHECK_MIN_MAX_RANGE(low, 0, HAL_CORAL_MAX_NUM_OF_DROP_THRSH);
    HAL_CHECK_MIN_MAX_RANGE(high, 0, HAL_CORAL_MAX_NUM_OF_DROP_THRSH);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        return rc;
    }
    data = (data & ~(CORAL_DPCR_LOW_THRSH_WIDTH)) | low;
    data = (data & ~(CORAL_DPCR_HIGH_THRSH_WIDTH << CORAL_DPCR_HIGH_THRSH_OFFSET)) |
           (high << CORAL_DPCR_HIGH_THRSH_OFFSET);
    return aml_writeReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
}

/* FUNCTION NAME:
 *      hal_coral_acl_getDropPbb
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
hal_coral_acl_getDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data, mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_pbb = (data >> CORAL_DPCR_PBB_OFFSET) & CORAL_DPCR_PBB_WIDTH;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setDropPbb
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
hal_coral_acl_setDropPbb(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data, mac_port = 0;

    HAL_CHECK_MIN_MAX_RANGE(pbb, 0, CORAL_DPCR_PBB_WIDTH);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    rc = aml_readReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        data = (data & ~(CORAL_DPCR_PBB_WIDTH << CORAL_DPCR_PBB_OFFSET)) | (pbb << CORAL_DPCR_PBB_OFFSET);
        rc = aml_writeReg(unit, CORAL_DPCR(mac_port, color, queue), &data, sizeof(UI32_T));
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getDropExMfrm
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
hal_coral_acl_getDropExMfrm(
    const UI32_T unit,
    BOOL_T      *ptr_type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = aml_readReg(unit, CORAL_GIRLCR, &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        data = (data >> CORAL_DP_MFRM_EX_OFFSET) & 0x1;
        *ptr_type = data ? TRUE : FALSE;
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setDropExMfrm
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
hal_coral_acl_setDropExMfrm(
    const UI32_T unit,
    const BOOL_T type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         data = 0;

    rc = aml_readReg(unit, CORAL_GIRLCR, &data, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        data = (data & ~(1U << CORAL_DP_MFRM_EX_OFFSET)) | ((type ? 1 : 0) << CORAL_DP_MFRM_EX_OFFSET);
        rc = aml_writeReg(unit, CORAL_GIRLCR, &data, sizeof(UI32_T));
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getTrtcmEn
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
hal_coral_acl_getTrtcmEn(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value = 0;

    rc = aml_readReg(unit, CORAL_ACL_TRTCM, &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        if (value & BIT(CORAL_ACL_TRTCM_EN_OFFSET))
        {
            *ptr_enable = TRUE;
        }
        else
        {
            *ptr_enable = FALSE;
        }
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setTrtcmEn
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
hal_coral_acl_setTrtcmEn(
    const UI32_T unit,
    BOOL_T       enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value = 0;

    rc = aml_readReg(unit, CORAL_ACL_TRTCM, &value, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        value &= ~(BIT(CORAL_ACL_TRTCM_EN_OFFSET));
        if (TRUE == enable)
        {
            value |= BIT(CORAL_ACL_TRTCM_EN_OFFSET);
        }
        rc = aml_writeReg(unit, CORAL_ACL_TRTCM, &value, sizeof(UI32_T));
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getTrtcmTable
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
hal_coral_acl_getTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, rdata_cbs = 0, rdata_ebs = 0, rdata_cir = 0, rdata_eir = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(tcm_idx, 0, HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY - 1);
    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_TRTCM))
    {
        return AIR_E_TIMEOUT;
    }

    value = (1 << CORAL_ACL_TRTCM_BUSY_OFFSET);
    value |= (CORAL_ACL_TRTCM_READ | ((tcm_idx & CORAL_ACL_TRTCM_ID_MASK) << CORAL_ACL_TRTCM_ID_OFFSET));
    aml_writeReg(unit, CORAL_ACL_TRTCMA, &value, sizeof(UI32_T));
    rc = _checkDone(unit, HAL_CORAL_ACL_CHECK_TRTCM);
    if (AIR_E_OK == rc)
    {
        aml_readReg(unit, CORAL_ACL_TRTCM_CBS_RDATA, &rdata_cbs, sizeof(UI32_T));
        aml_readReg(unit, CORAL_ACL_TRTCM_EBS_RDATA, &rdata_ebs, sizeof(UI32_T));
        aml_readReg(unit, CORAL_ACL_TRTCM_CIR_RDATA, &rdata_cir, sizeof(UI32_T));
        aml_readReg(unit, CORAL_ACL_TRTCM_EIR_RDATA, &rdata_eir, sizeof(UI32_T));
        ptr_tcm->cbs = rdata_cbs & CORAL_ACL_TRTCM_CBS_MASK;
        ptr_tcm->pbs = rdata_ebs & CORAL_ACL_TRTCM_EBS_MASK;
        ptr_tcm->cir = rdata_cir & CORAL_ACL_TRTCM_CIR_MASK;
        ptr_tcm->pir = rdata_eir & CORAL_ACL_TRTCM_EIR_MASK;
    }

    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setTrtcmTable
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
hal_coral_acl_setTrtcmTable(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         value, wdata_cbs = 0, wdata_ebs = 0, wdata_cir = 0, wdata_eir = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(tcm_idx, 0, HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->cbs, 0, HAL_CORAL_MAX_NUM_OF_CBS);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->cir, 0, HAL_CORAL_MAX_NUM_OF_CIR);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->pbs, 0, HAL_CORAL_MAX_NUM_OF_PBS);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcm->pir, 0, HAL_CORAL_MAX_NUM_OF_PIR);

    if (AIR_E_TIMEOUT == _checkDone(unit, HAL_CORAL_ACL_CHECK_TRTCM))
    {
        return AIR_E_TIMEOUT;
    }
    wdata_cbs = ((ptr_tcm->cbs & CORAL_ACL_TRTCM_CBS_MASK) << CORAL_ACL_TRTCM_CBS_OFFSET);
    wdata_ebs = ((ptr_tcm->pbs & CORAL_ACL_TRTCM_EBS_MASK) << CORAL_ACL_TRTCM_EBS_OFFSET);
    wdata_cir = ((ptr_tcm->cir & CORAL_ACL_TRTCM_CIR_MASK) << CORAL_ACL_TRTCM_CIR_OFFSET);
    wdata_eir = ((ptr_tcm->pir & CORAL_ACL_TRTCM_EIR_MASK) << CORAL_ACL_TRTCM_EIR_OFFSET);

    aml_writeReg(unit, CORAL_ACL_TRTCM_CBS_WDATA, &wdata_cbs, sizeof(UI32_T));
    aml_writeReg(unit, CORAL_ACL_TRTCM_EBS_WDATA, &wdata_ebs, sizeof(UI32_T));
    aml_writeReg(unit, CORAL_ACL_TRTCM_CIR_WDATA, &wdata_cir, sizeof(UI32_T));
    aml_writeReg(unit, CORAL_ACL_TRTCM_EIR_WDATA, &wdata_eir, sizeof(UI32_T));

    value = (1 << CORAL_ACL_TRTCM_BUSY_OFFSET);
    value |= (CORAL_ACL_TRTCM_WRITE | ((tcm_idx & CORAL_ACL_TRTCM_ID_MASK) << CORAL_ACL_TRTCM_ID_OFFSET));
    aml_writeReg(unit, CORAL_ACL_TRTCMA, &value, sizeof(UI32_T));
    rc = _checkDone(unit, HAL_CORAL_ACL_CHECK_TRTCM);

    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_init
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
hal_coral_acl_init(
    const UI32_T unit)
{
    return hal_coral_acl_clearAll(unit);
}

/* FUNCTION NAME:
 *      hal_coral_acl_deinit
 * PURPOSE:
 *      This API is used to deinit acl.
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
hal_coral_acl_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc;

    rc = hal_coral_acl_clearUdfRule(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = hal_coral_acl_clearMibCnt(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = hal_coral_acl_clearAll(unit);

    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_getArpRule
 * PURPOSE:
 *      Get ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 * OUTPUT:
 *      ptr_rule         -- pointer buffer of ARP rule
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0}, type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    UI32_T         acl_entry_idx = 0;
    HAL_CORAL_ACL_RULE_TYPE_T type = HAL_CORAL_ACL_RULE_TYPE_0;
    HAL_CORAL_ACL_CTRL_T      ctrl;

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);
    HAL_CHECK_PTR(ptr_rule);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    osal_memset(ptr_rule, 0, sizeof(AIR_ACL_ARP_RULE_T));
    osal_memset(&ctrl, 0, sizeof(HAL_CORAL_ACL_CTRL_T));

    rc = _hal_coral_acl_getfromCell(unit, acl_entry_idx, type_t, type_c);
    HAL_CORAL_ACL_DERIVE_TBL_MULTIFIELDS(type_t, RULE_TYPE_OFFSET, RULE_TYPE_WIDTH, type);
    if ((AIR_E_OK != rc) || (HAL_CORAL_ACL_RULE_TYPE_2 != type))
    {
        DIAG_PRINT(HAL_DBG_INFO, "acl rule entry %d is not an ACL-ARP rule.\n", entry_idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    if (AIR_E_OK == rc)
    {
        rc = _hal_coral_acl_getRuleCtrl(unit, acl_entry_idx, &ctrl);
        ptr_rule->rule_en = ctrl.rule_en;
        ptr_rule->reverse = ctrl.reverse;
        ptr_rule->end = ctrl.end;
        if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
        {
            ptr_rule->end_reverse = ctrl.end_reverse;
        }
        _hal_coral_acl_getRuleFormat2(unit, ptr_rule, type_t, type_c);
    }
    return rc;
}

/* FUNCTION NAME:
 *      hal_coral_acl_setArpRule
 * PURPOSE:
 *      Add ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 *      ptr_rule         -- pointer buffer of ARP rule
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_setArpRule(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule)
{
    UI32_T               type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT];
    UI32_T               type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT];
    HAL_CORAL_ACL_CTRL_T ctrl;
    UI32_T               acl_entry_idx = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(entry_idx, 0, HAL_CORAL_MAX_NUM_OF_ACL_ENTRY - 1);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dmac, 0, HAL_CORAL_MAX_NUM_OF_DMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.smac, 0, HAL_CORAL_MAX_NUM_OF_SMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.stag, 0, HAL_CORAL_MAX_NUM_OF_STAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.ctag, 0, HAL_CORAL_MAX_NUM_OF_CTAG_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.tha, 0, HAL_CORAL_MAX_NUM_OF_DMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sha, 0, HAL_CORAL_MAX_NUM_OF_SMAC_MASK);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.dip, 0, HAL_CORAL_MAX_NUM_OF_DIP_MASK_IPV4);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rule->mask.sip, 0, HAL_CORAL_MAX_NUM_OF_SIP_MASK_IPV4);
    HAL_CHECK_PORT_BITMAP(unit, ptr_rule->portmap);

    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);
    /* Reset to all as don't care in t/c cell to improve performance */
    osal_memset(type_t, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
    osal_memset(type_c, 0xff, sizeof(UI32_T) * HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT);
    /* Fill rule type table */
    _hal_coral_acl_fillRuleFormat2(unit, ptr_rule, type_t, type_c);
    _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);

    /* Config rule enable/end/rev */
    ctrl.rule_en = ptr_rule->rule_en;
    ctrl.reverse = ptr_rule->reverse;
    ctrl.end = ptr_rule->end;
    if (HAL_CORAL_ACL_CTRL_NO_END_REVERSE_REVISION != HAL_DEVICE_REV_ID(unit))
    {
        ctrl.end_reverse = ptr_rule->end_reverse;
    }
    _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, FALSE, &ctrl);

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_coral_acl_delArpRule
 * PURPOSE:
 *      Delete ACL ARP rule of specified entry index.
 * INPUT:
 *      unit             -- unit id
 *      entry_idx        -- ACL ARP rule entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_delArpRule(
    const UI32_T unit,
    const UI32_T entry_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         type_t[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0}, type_c[HAL_CORAL_ACL_ENTRY_REG_DATA_COUNT] = {0};
    UI32_T         acl_entry_idx = 0;
    HAL_CORAL_ACL_RULE_TYPE_T type = HAL_CORAL_ACL_RULE_TYPE_0;
    HAL_CORAL_ACL_CTRL_T      ctrl = {0};

    HAL_CHECK_PARAM((entry_idx >= HAL_CORAL_MAX_NUM_OF_ACL_ENTRY), AIR_E_BAD_PARAMETER);
    HAL_CORAL_ACL_ENTRY_IDX_REMAP(entry_idx, acl_entry_idx);

    rc = _hal_coral_acl_getRuleType(unit, acl_entry_idx, &type);
    if (HAL_CORAL_ACL_RULE_TYPE_2 != type)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Info]:acl rule entry %d is not found or is not an ARP rule.\n", entry_idx);
        rc = AIR_E_ENTRY_NOT_FOUND;
    }

    if (AIR_E_OK == rc)
    {
        /* Config rule enable/end/rev */
        _hal_coral_acl_setRuleCtrl(unit, acl_entry_idx, FALSE, &ctrl);

        /* Delete the entry from ACL rule table */
        _hal_coral_acl_settoCell(unit, acl_entry_idx, type_t, type_c);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_acl_getCapacity
 * PURPOSE:
 *      Get the acl resource capacity
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
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
        case AIR_SWC_RSRC_ACL_ACTION:
            *ptr_size = HAL_CORAL_MAX_NUM_OF_ACL_ENTRY;
            break;
        case AIR_SWC_RSRC_ACL_UDF:
            *ptr_size = HAL_CORAL_MAX_NUM_OF_UDF_ENTRY;
            break;
        case AIR_SWC_RSRC_ACL_COUNTER:
            *ptr_size = HAL_CORAL_MAX_NUM_OF_MIB_ID;
            break;
        case AIR_SWC_RSRC_ACL_METER:
            *ptr_size = HAL_CORAL_MAX_NUM_OF_METER_ID;
            break;
        case AIR_SWC_RSRC_ACL_TRTCM:
            *ptr_size = HAL_CORAL_MAX_NUM_OF_TRTCM_ENTRY;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_acl_getUsage
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
 *      None
 */
AIR_ERROR_NO_T
hal_coral_acl_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
            _hal_coral_acl_getRuleUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_ACTION:
            _hal_coral_acl_getActionUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_UDF:
            _hal_coral_acl_getUdfUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_COUNTER:
            _hal_coral_acl_getCounterUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_METER:
            _hal_coral_acl_getMeterUsage(unit, ptr_cnt);
            break;
        case AIR_SWC_RSRC_ACL_TRTCM:
            _hal_coral_acl_getTrtcmUsage(unit, ptr_cnt);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}
