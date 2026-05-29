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

/* FILE NAME:  hal_sco_l2.c
 * PURPOSE:
 *  Implement L2 module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_l2.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <hal/switch/sco/hal_sco_stp.h>
#include <hal/switch/sco/hal_sco_vlan.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_L2_DELAY_US          (1000)
#define HAL_SCO_L2_WDOG_KICK_NUM     (1000)
#define HAL_SCO_L2_FORWARD_VALUE     (0xFFFFFFFF)
#define HAL_SCO_L2_AGING_MS_CONSTANT (1024)
#define HAL_SCO_L2_AGING_1000MS      (1000)

#define HAL_SCO_L2_AAC_DEFAULT_VALUE     (0x00125000)
#define HAL_SCO_L2_ADDRTBL_DEFAULT_VALUE (0)
#define HAL_SCO_L2_AGDIS_DEFAULT_VALUE   (0)
#define HAL_SCO_L2_LOGPORT_DEFAULT_VALUE (0xFFFFFFFF)

typedef enum
{
    _HAL_SCO_L2_MAC_TB_TY_MAC,
    _HAL_SCO_L2_MAC_TB_TY_DIP,
    _HAL_SCO_L2_MAC_TB_TY_DIP_SIP,
    _HAL_SCO_L2_MAC_TB_TY_SPEC_ENTRY,
    _HAL_SCO_L2_MAC_TB_TY_LAST
} _HAL_SCO_L2_MAC_TB_TY_T;

typedef enum
{
    _HAL_SCO_L2_MAC_MAT_MAC,
    _HAL_SCO_L2_MAC_MAT_DYNAMIC_MAC,
    _HAL_SCO_L2_MAC_MAT_STATIC_MAC,
    _HAL_SCO_L2_MAC_MAT_MAC_BY_VID,
    _HAL_SCO_L2_MAC_MAT_MAC_BY_FID,
    _HAL_SCO_L2_MAC_MAT_MAC_BY_PORT,
    _HAL_SCO_L2_MAC_MAT_MAC_BY_LAST
} _HAL_SCO_L2_MAC_MAT_T;

/* L2 MAC table multi-searching */
typedef enum
{
    _HAL_SCO_L2_MAC_MS_START, /* Start search */
    _HAL_SCO_L2_MAC_MS_NEXT,  /* Next search */
    _HAL_SCO_L2_MAC_MS_LAST
} _HAL_SCO_L2_MAC_MS_T;

typedef enum
{
    _HAL_SCO_L2_FWD_CTRL_DEFAULT = 0x0,
    _HAL_SCO_L2_FWD_CTRL_CPU_EXCLUDE = 0x4,
    _HAL_SCO_L2_FWD_CTRL_CPU_INCLUDE = 0x5,
    _HAL_SCO_L2_FWD_CTRL_CPU_ONLY = 0x6,
    _HAL_SCO_L2_FWD_CTRL_DROP = 0x7,
    _HAL_SCO_L2_FWD_CTRL_LAST
} _HAL_SCO_L2_FWD_CTRL_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SCO_L2_AGING_TIME(__cnt__, __unit__)                                                 \
    (((__cnt__) + 1) * ((__unit__) + 1) * HAL_SCO_L2_AGING_MS_CONSTANT / HAL_SCO_L2_AGING_1000MS)

#define _HAL_SCO_L2_AGING_TIME_2_CNT(__time__, __cnt__, __unit__)                             \
    do                                                                                        \
    {                                                                                         \
        UI32_T _value_ = (__time__) * HAL_SCO_L2_AGING_1000MS / HAL_SCO_L2_AGING_MS_CONSTANT; \
        (__unit__) = (_value_ / BIT(AAC_AGE_CNT_LENGTH) + 1);                                 \
        (__cnt__) = (_value_ / (__unit__) + 1);                                               \
    } while (0)

#define HAL_SCO_L2_TRANS_IDX_TO_ADDR(__i__, __a__, __b__) \
    do                                                    \
    {                                                     \
        (__a__) = (__i__) / HAL_SCO_L2_MAC_SET_NUM;       \
        (__b__) = (__i__) % HAL_SCO_L2_MAC_SET_NUM;       \
    } while (0);

#define HAL_SCO_L2_MAC_ADDR_DIFF(__ptr_ma_1__, __ptr_ma_2__) (osal_memcmp(__ptr_ma_1__, __ptr_ma_2__, 6))

#define HAL_SCO_L2_MAC_ENTRY_DIFF(__ptr_ety_1__, __ptr_ety_2__)                                                    \
    (HAL_SCO_L2_MAC_ADDR_DIFF((__ptr_ety_1__)->mac, (__ptr_ety_2__)->mac) ||                                       \
     ((__ptr_ety_1__)->cvid != (__ptr_ety_2__)->cvid) || ((__ptr_ety_1__)->fid != (__ptr_ety_2__)->fid) ||         \
     ((__ptr_ety_1__)->flags != (__ptr_ety_2__)->flags) || ((__ptr_ety_1__)->sa_fwd != (__ptr_ety_2__)->sa_fwd) || \
     !CMLIB_BITMAP_EQUAL((__ptr_ety_1__)->port_bitmap, (__ptr_ety_2__)->port_bitmap, AIR_PORT_BITMAP_SIZE))

#ifdef AIR_EN_L2_SHADOW
#define PTR_HAL_SCO_L2_SW_ENTRY(__unit__, __idx__) (&(_l2_fdb_cb[__unit__].ptr_sw_entry[__idx__]))

#define HAL_SCO_L2_RBT_HEAD(__unit__) (_l2_fdb_cb[__unit__].ptr_rbt_head)

#define PTR_HAL_SCO_L2_POLLING_CB(__unit__) (&(_l2_fdb_cb[__unit__].polling_cb))
#endif /* End of AIR_EN_L2_SHADOW */

#define PTR_HAL_SCO_L2_HW_TBL_MUTEX(__unit__) (&(_l2_fdb_cb[__unit__].hw_tbl_mutex))

#ifdef AIR_EN_L2_SHADOW
#define PTR_HAL_SCO_L2_SW_TBL_MUTEX(__unit__) (&(_l2_fdb_cb[__unit__].sw_tbl_mutex))

#define PTR_HAL_SCO_L2_NOTIFY_MUTEX(__unit__) (&(_l2_fdb_cb[__unit__].notify_mutex))

#define HAL_SCO_L2_USED_ENTRY_NUM(__unit__) (_l2_fdb_cb[__unit__].ptr_rbt_head->node_count)

#define PTR_HAL_SCO_L2_NOTIFY_HANDLER(__unit__, __idx__) (&(_l2_fdb_cb[__unit__].notify_handler[__idx__]))

#define HAL_SCO_L2_RBT_NODE_DBG_PRINT(__entry_id__, __ptr_node__)                                                  \
    do                                                                                                             \
    {                                                                                                              \
        DIAG_PRINT(HAL_DBG_INFO, "Node[%u](%08p),C(%s),L(%08p),R(%08p),P(%08p)\n", (__entry_id__), (__ptr_node__), \
                   ((__ptr_node__)->color == CMLIB_RBT_COLOR_TYPE_BLACK) ?                                         \
                       "B" :                                                                                       \
                       (((__ptr_node__)->color == CMLIB_RBT_COLOR_TYPE_RED) ?                                      \
                            "R" :                                                                                  \
                            (((__ptr_node__)->color == CMLIB_RBT_COLOR_TYPE_DOUBLE_BLACK) ? "DB" : "UN")),         \
                   (__ptr_node__)->ptr_left, (__ptr_node__)->ptr_right, (__ptr_node__)->ptr_parent);               \
    } while (0);

#endif /* End of AIR_EN_L2_SHADOW */

#define HAL_SCO_L2_PRINT_MAC_ENTRY(__name__, __entry__)                                                            \
    do                                                                                                             \
    {                                                                                                              \
        DIAG_PRINT(HAL_DBG_INFO,                                                                                   \
                   "[%s] MAC=%02X:%02X:%02X:%02X:%02X:%02X"                                                        \
                   ", cvid=%4u"                                                                                    \
                   ", fid=%2u\n",                                                                                  \
                   (__name__), (__entry__)->mac[0], (__entry__)->mac[1], (__entry__)->mac[2], (__entry__)->mac[3], \
                   (__entry__)->mac[4], (__entry__)->mac[5], (__entry__)->cvid, (__entry__)->fid);                 \
        DIAG_PRINT(HAL_DBG_INFO,                                                                                   \
                   "[%s] flags=%02X"                                                                               \
                   ", port_bitmap=%08X"                                                                            \
                   ", sa_fwd=%02X"                                                                                 \
                   ", timer=%8u\n",                                                                                \
                   (__name__), (__entry__)->flags, (__entry__)->port_bitmap[0], (__entry__)->sa_fwd,               \
                   (__entry__)->timer);                                                                            \
    } while (0);

/* DATA TYPE DECLARATIONS
 */

#ifdef AIR_EN_L2_SHADOW
typedef struct HAL_SCO_L2_FLUSH_INFO_S
{
    AIR_L2_MAC_FLUSH_TYPE_T type;
    UI32_T                  value;
} HAL_SCO_L2_FLUSH_INFO_T;
#endif /* End of AIR_EN_L2_SHADOW */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_L2, "hal_sco_l2.c");

/* STATIC VARIABLE DECLARATIONS
 */

static HAL_SCO_L2_FDB_CB_T _l2_fdb_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
#ifndef AIR_EN_L2_SHADOW
static BOOL_T _search_end = FALSE;
#endif /* End of AIR_EN_L2_SHADOW */

/* LOCAL SUBPROGRAM BODIES
 */

#ifdef AIR_EN_L2_SHADOW
/* FUNCTION NAME: _hal_sco_l2_traverse_dbg_func
 * PURPOSE:
 *      the debug function for L2 traversing
 * INPUT:
 *      ptr_user_param  --  Pointer to the user parameters
 *      ptr_node_data   --  Pointer to the node data
 *      ptr_cookie      --  Pointer to the cookie data
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_traverse_dbg_func(
    void *ptr_user_param,
    void *ptr_node_data,
    void *ptr_cookie)
{
    HAL_SCO_L2_FDB_SW_ENTRY_T *ptr_sw_entry = (HAL_SCO_L2_FDB_SW_ENTRY_T *)ptr_node_data;
    CMLIB_RBT_NODE_T          *ptr_node = ptr_sw_entry->ptr_rbt_node;
    HAL_SCO_L2_RBT_NODE_DBG_PRINT(ptr_sw_entry->entry_id, ptr_node);
    HAL_SCO_L2_PRINT_MAC_ENTRY("Traverse", &(ptr_sw_entry->entry));
    return AIR_E_OK;
}
#endif /* End of AIR_EN_L2_SHADOW */

/* FUNCTION NAME: _isMacEntryValid
 * PURPOSE:
 *      Verify whether the input MAC entry is valid.
 * INPUT:
 *      ptr_mac_entry   --  MAC Address entry
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE
 *      FALSE
 * NOTES:
 *      None
 */
static BOOL_T
_isMacEntryValid(
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    AIR_MAC_T all_zero = {0};
    return HAL_SCO_L2_MAC_ADDR_DIFF(ptr_mac_entry->mac, all_zero) ? TRUE : FALSE;
}

/* FUNCTION NAME: _fill_MAC_ATA
 * PURPOSE:
 *      Fill register ATA for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  Structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATA(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T u32dat;
    I32_T  i;

    /* Fill ATA1 */
    u32dat = 0;
    for (i = 0; i < 4; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0, 7))) << ((3 - i) * 8);
    }
    aml_writeReg(unit, ATA1, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);

    /* Fill ATA2 */
    u32dat = 0;
    for (i = 4; i < 6; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0, 7))) << ((7 - i) * 8);
    }
    if (!(ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC))
    {
        /* type is dynamic */
        u32dat |= BITS_OFF_L(1UL, ATA2_MAC_LIFETIME_OFFSET, ATA2_MAC_LIFETIME_LENGTH);
        /* set aging counter as system aging conuter */
        u32dat |= BITS_OFF_L(ptr_mac_entry->timer, ATA2_MAC_AGETIME_OFFSET, ATA2_MAC_AGETIME_LENGTH);
    }
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_UNAUTH)
    {
        u32dat |= BITS_OFF_L(1UL, ATA2_MAC_UNAUTH_OFFSET, ATA2_MAC_UNAUTH_LENGTH);
    }

    aml_writeReg(unit, ATA2, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);
}

/* FUNCTION NAME: _fill_MAC_ATWD
 * PURPOSE:
 *      Fill register ATWD for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  Structure of MAC Address table
 *      valid           --  TRUE
 *                          FALSE
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATWD(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry,
    const BOOL_T           valid)
{
    UI32_T u32dat = 0;
    UI32_T fwd_val = 0;

    u32dat = 0;
    /* Fill ATWD */
    /* set valid bit */
    if (TRUE == valid)
    {
        u32dat |= BITS_OFF_L(1UL, ATWD_MAC_LIVE_OFFSET, ATWD_MAC_LIVE_LENGTH);
    }

    /* set IVL */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        u32dat |= BITS_OFF_L(1UL, ATWD_MAC_IVL_OFFSET, ATWD_MAC_IVL_LENGTH);
    }
    /* set VID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->cvid, ATWD_MAC_VID_OFFSET, ATWD_MAC_VID_LENGTH);
    /* set FID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->fid, ATWD_MAC_FID_OFFSET, ATWD_MAC_FID_LENGTH);
    /* set leaky VLAN */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
    {
        u32dat |= BITS_OFF_L(1UL, ATWD_MAC_LEAK_OFFSET, ATWD_MAC_LEAK_LENGTH);
    }

    /* Set forwarding control */
    switch (ptr_mac_entry->sa_fwd)
    {
        case AIR_L2_FWD_CTRL_DEFAULT:
            fwd_val = _HAL_SCO_L2_FWD_CTRL_DEFAULT;
            break;
        case AIR_L2_FWD_CTRL_CPU_INCLUDE:
            fwd_val = _HAL_SCO_L2_FWD_CTRL_CPU_INCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_EXCLUDE:
            fwd_val = _HAL_SCO_L2_FWD_CTRL_CPU_EXCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_ONLY:
            fwd_val = _HAL_SCO_L2_FWD_CTRL_CPU_ONLY;
            break;
        case AIR_L2_FWD_CTRL_DROP:
            fwd_val = _HAL_SCO_L2_FWD_CTRL_DROP;
            break;
        default:
            break;
    }
    u32dat |= BITS_OFF_L(fwd_val, ATWD_MAC_FWD_OFFSET, ATWD_MAC_FWD_LENGTH);
    aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);

    /* Fill ATWD2 */
    u32dat = BITS_OFF_L(ptr_mac_entry->port_bitmap[0], ATWD2_MAC_PORT_OFFSET, ATWD2_MAC_PORT_LENGTH);
    aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);
}

/* FUNCTION NAME: _fill_MAC_ATRDS
 * PURPOSE:
 *      Fill register ATRDS for select bank after ATC search L2 table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      bank            --  Selected index of bank
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATRDS(
    const UI32_T unit,
    UI8_T        bank)
{
    UI32_T u32dat;

    /* Fill ATRDS */
    u32dat = BITS_OFF_L(bank, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
    aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);
}

/* FUNCTION NAME: _read_MAC_ATRD
 * PURPOSE:
 *      Read register ATRD for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mac_entry   --  Structure of MAC Address table
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_MAC_ATRD(
    const UI32_T     unit,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T u32dat;
    UI32_T i;
    BOOL_T live = FALSE;
    UI32_T type;
    UI32_T age_unit;
    UI32_T age_cnt;
    UI32_T sa_fwd;

    /* Read ATRD0 */
    aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
    live = BITS_OFF_R(u32dat, ATRD0_MAC_LIVE_OFFSET, ATRD0_MAC_LIVE_LENGTH);
    type = BITS_OFF_R(u32dat, ATRD0_MAC_TYPE_OFFSET, ATRD0_MAC_TYPE_LENGTH);
    if (FALSE == live)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    if (_HAL_SCO_L2_MAC_TB_TY_MAC != type)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    /* Clear table */
    osal_memset(ptr_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));

    ptr_mac_entry->cvid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_MAC_VID_OFFSET, ATRD0_MAC_VID_LENGTH);
    ptr_mac_entry->fid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_MAC_FID_OFFSET, ATRD0_MAC_FID_LENGTH);
    if (!!BITS_OFF_R(u32dat, ATRD0_MAC_LIFETIME_OFFSET, ATRD0_MAC_LIFETIME_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;
    }
    if (!!BITS_OFF_R(u32dat, ATRD0_MAC_IVL_OFFSET, ATRD0_MAC_IVL_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    if (!!BITS_OFF_R(u32dat, ATRD1_MAC_UNAUTH_OFFSET, ATRD1_MAC_UNAUTH_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_UNAUTH;
    }
    if (!!BITS_OFF_R(u32dat, ATRD0_MAC_LEAK_OFFSET, ATRD0_MAC_LEAK_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
    }

    /* Get the L2 MAC aging unit */
    aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));
    age_unit = BITS_OFF_R(u32dat, AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);

    /* Read ATRD1 */
    aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
    for (i = 4; i < 6; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (7 - i) * 8, 8);
    }
    /* Aging time */
    age_cnt = BITS_OFF_R(u32dat, ATRD1_MAC_AGETIME_OFFSET, ATRD1_MAC_AGETIME_LENGTH);
    ptr_mac_entry->timer = _HAL_SCO_L2_AGING_TIME(age_cnt, age_unit);
    /* SA forwarding */
    sa_fwd = BITS_OFF_R(u32dat, ATRD1_MAC_FWD_OFFSET, ATRD1_MAC_FWD_LENGTH);
    switch (sa_fwd)
    {
        case _HAL_SCO_L2_FWD_CTRL_DEFAULT:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
        case _HAL_SCO_L2_FWD_CTRL_CPU_INCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_INCLUDE;
            break;
        case _HAL_SCO_L2_FWD_CTRL_CPU_EXCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_EXCLUDE;
            break;
        case _HAL_SCO_L2_FWD_CTRL_CPU_ONLY:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_ONLY;
            break;
        case _HAL_SCO_L2_FWD_CTRL_DROP:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DROP;
            break;
        default:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
    }

    /* Read ATRD2 */
    aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
    for (i = 0; i < 4; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (3 - i) * 8, 8);
    }

    /* Read ATRD3 */
    aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
    ptr_mac_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);

    return AIR_E_OK;
}

/* FUNCTION NAME: _checkL2Busy
 * PURPOSE:
 *      Check BUSY bit of ATC
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkL2Busy(
    const UI32_T unit)
{
    UI32_T i;
    UI32_T reg_atc;

    /* Check BUSY bit is 0 */
    for (i = 0; i < HAL_SCO_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &reg_atc, sizeof(reg_atc));
        if (!BITS_OFF_R(reg_atc, ATC_BUSY_OFFSET, ATC_BUSY_LENGTH))
        {
            break;
        }
        osal_delayUs(HAL_SCO_L2_DELAY_US);
    }
    if (i >= HAL_SCO_L2_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _checkL2EntryHit
 * PURPOSE:
 *      Check entry hit of ATC
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The entry hit bitmap
 *
 * NOTES:
 *      None
 */
static UI32_T
_checkL2EntryHit(
    const UI32_T unit)
{
    UI32_T reg_atc;
    aml_readReg(unit, ATC, &reg_atc, sizeof(reg_atc));
    return BITS_OFF_R(reg_atc, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
}

/* FUNCTION NAME: _checkL2ReadSingleHit
 * PURPOSE:
 *      Check reading single entry hit of ATC
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The entry hit result
 *
 * NOTES:
 *      None
 */
static UI32_T
_checkL2ReadSingleHit(
    const UI32_T unit)
{
    UI32_T reg_atc;
    aml_readReg(unit, ATC, &reg_atc, sizeof(reg_atc));
    return BITS_OFF_R(reg_atc, ATC_SINGLE_HIT_OFFSET, ATC_SINGLE_HIT_LENGTH);
}

#ifndef AIR_EN_L2_SHADOW
/* FUNCTION NAME: _searchMacEntry
 * PURPOSE:
 *      Search MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ms              --  _HAL_SCO_L2_MAC_MS_START:           Start search command
 *                          _HAL_SCO_L2_MAC_MS_NEXT:            Next search command
 *      multi_target    --  _HAL_SCO_L2_MAC_MAT_MAC:            MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_DYNAMIC_MAC:    Dynamic MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_STATIC_MAC:     Static MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_VID:     MAC address entries with specific CVID
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_FID:     MAC address entries with specific FID
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_PORT:    MAC address entries with specific port
 * OUTPUT:
 *      ptr_addr        --  MAC Table Access Index
 *      ptr_bank        --  Searching result in which bank
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_searchMacEntry(
    const UI32_T                unit,
    const _HAL_SCO_L2_MAC_MS_T  ms,
    const _HAL_SCO_L2_MAC_MAT_T multi_target,
    UI32_T                     *ptr_addr,
    UI32_T                     *ptr_bank)
{
    UI32_T u32dat;

    u32dat = (ATC_SAT_MAC | ATC_START_BUSY);
    if (_HAL_SCO_L2_MAC_MS_START == ms)
    {
        /* Start search 1st valid entry */
        u32dat |= ATC_CMD_SEARCH;
    }
    else if (_HAL_SCO_L2_MAC_MS_NEXT == ms)
    {
        /* Search next valid entry */
        u32dat |= ATC_CMD_SEARCH_NEXT;
    }
    else
    {
        /* Unknown commnad */
        return AIR_E_BAD_PARAMETER;
    }

    switch (multi_target)
    {
        case _HAL_SCO_L2_MAC_MAT_MAC:
            u32dat |= ATC_MAT_MAC;
            break;
        case _HAL_SCO_L2_MAC_MAT_DYNAMIC_MAC:
            u32dat |= ATC_MAT_DYNAMIC_MAC;
            break;
        case _HAL_SCO_L2_MAC_MAT_STATIC_MAC:
            u32dat |= ATC_MAT_STATIC_MAC;
            break;
        case _HAL_SCO_L2_MAC_MAT_MAC_BY_VID:
            u32dat |= ATC_MAT_MAC_BY_VID;
            break;
        case _HAL_SCO_L2_MAC_MAT_MAC_BY_FID:
            u32dat |= ATC_MAT_MAC_BY_FID;
            break;
        case _HAL_SCO_L2_MAC_MAT_MAC_BY_PORT:
            u32dat |= ATC_MAT_MAC_BY_PORT;
            break;
        default:
            /* Unknown searching mode */
            return AIR_E_BAD_PARAMETER;
    }
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
    /* Get address */
    (*ptr_addr) = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
    /* Get banks */
    (*ptr_bank) = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
    if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (*ptr_addr))
    {
        _search_end = TRUE;
    }
    else
    {
        _search_end = FALSE;
    }

    return AIR_E_OK;
}
#endif /* End of AIR_EN_L2_SHADOW */

#ifdef AIR_EN_L2_SHADOW
/* FUNCTION NAME: _getSpecMacEntryByIndex
 * PURPOSE:
 *      Get specified MAC Address entry by address & bank.
 * INPUT:
 *      unit                --  Device ID
 *      addr                --  Address index of MAC entry
 *      bank                --  Bank index of MAC entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_getSpecMacEntryByIndex(
    const UI32_T unit,
    const UI32_T addr,
    const UI32_T bank)
{
    UI32_T u32dat;

    HAL_CHECK_UNIT(unit);

    /* Fill ATA1 */
    u32dat = BITS_OFF_L(addr, ATA1_SAT_ADDR_OFFSET, ATA1_SAT_ADDR_LENGTH);
    u32dat |= BITS_OFF_L(bank, ATA1_SAT_BANK_OFFSET, ATA1_SAT_BANK_LENGTH);
    aml_writeReg(unit, ATA1, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_SCO_L2_DELAY_US);

    /* Write ATC */
    osal_delayUs(HAL_SCO_L2_DELAY_US);
    u32dat = (ATC_CMD_READ | ATC_SAT_ADDR | ATC_START_BUSY);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}
#endif /* End of AIR_EN_L2_SHADOW */

/* FUNCTION NAME: _getSpecMacEntryByMac
 * PURPOSE:
 *      Get specified MAC Address entry by MAC & VID/FID
 * INPUT:
 *      unit                --  Device ID
 *      ptr_mac_entry       --  The pointer of MAC entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_getSpecMacEntryByMac(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry,
    UI32_T                *ptr_banks)
{
    UI32_T u32dat;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_mac_entry);
    _fill_MAC_ATA(unit, ptr_mac_entry);
    _fill_MAC_ATWD(unit, ptr_mac_entry, TRUE);

    /* Write ATC */
    u32dat = (ATC_CMD_READ | ATC_SAT_MAC | ATC_START_BUSY);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }
    if (!_checkL2ReadSingleHit(unit))
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
    (*ptr_banks) = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);

    return AIR_E_OK;
}

#ifdef AIR_EN_L2_SHADOW
/* FUNCTION NAME: _hal_sco_l2_rbtCmpCallback
 * PURPOSE:
 *      The compare callback function for the RBT in l2_fdb shadow table.
 * INPUT:
 *      ptr_user_param      --  The user parameter
 *      ptr_user_data       --  The user data
 *      ptr_node_data       --  The node data
 * OUTPUT:
 *      None
 * RETURN:
 *      1   --  The user data is larger than the node data
 *      0   --  The user data equals to the node data
 *      -1  --  The user data is smaller than the node data
 * NOTES:
 *      None
 */
static I32_T
_hal_sco_l2_rbtCmpCallback(
    void *ptr_user_param,
    void *ptr_user_data,
    void *ptr_node_data)
{
    HAL_SCO_L2_FDB_SW_ENTRY_T *ptr_user_entry = (HAL_SCO_L2_FDB_SW_ENTRY_T *)ptr_user_data;
    HAL_SCO_L2_FDB_SW_ENTRY_T *ptr_node_entry = (HAL_SCO_L2_FDB_SW_ENTRY_T *)ptr_node_data;
    I32_T                      rc = 0;

    if ((NULL == ptr_user_entry) || (NULL == ptr_node_entry))
    {
        DIAG_PRINT(HAL_DBG_INFO, "Compare NULL\n");
        return 1;
    }

    /* 1. compare MAC address */
    rc = osal_memcmp(ptr_user_entry->entry.mac, ptr_node_entry->entry.mac, sizeof(AIR_MAC_T));
    if (rc > 0)
    {
        DIAG_PRINT(HAL_DBG_INFO, "Compare MAC user bigger\n");
        return 1;
    }
    else if (rc < 0)
    {
        DIAG_PRINT(HAL_DBG_INFO, "Compare MAC user smaller\n");
        return -1;
    }

    /* 2. Compare IVL, IVL is bigger */
    if ((ptr_user_entry->entry.flags ^ ptr_node_entry->entry.flags) & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        if (ptr_user_entry->entry.flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare user IVL\n");
            return 1;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare node IVL\n");
            return -1;
        }
    }

    if (ptr_user_entry->entry.flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        /* 3-1. IVL, Compare VID */
        if (ptr_user_entry->entry.cvid > ptr_node_entry->entry.cvid)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare VID user bigger\n");
            return 1;
        }
        else if (ptr_user_entry->entry.cvid < ptr_node_entry->entry.cvid)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare VID user smaller\n");
            return -1;
        }
    }
    else
    {
        /* 3-2. SVL, Compare FID */
        if (ptr_user_entry->entry.fid > ptr_node_entry->entry.fid)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare FID user bigger\n");
            return 1;
        }
        else if (ptr_user_entry->entry.fid < ptr_node_entry->entry.fid)
        {
            DIAG_PRINT(HAL_DBG_INFO, "Compare FID user smaller\n");
            return -1;
        }
    }

    DIAG_PRINT(HAL_DBG_INFO, "Compare equal\n");
    return 0;
}

/* FUNCTION NAME: _hal_sco_l2_callNotifyCallback
 * PURPOSE:
 *      Call user registered callback function when L2 MAC address table updated
 * INPUT:
 *      unit                --  Device unit number
 *      reason              --  The L2 MAC address entry update reason
 *      ptr_mac_entry       --  The updated L2 MAC address entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_callNotifyCallback(
    const UI32_T                     unit,
    const AIR_L2_MAC_NOTIFY_REASON_T reason,
    const AIR_MAC_ENTRY_T           *ptr_entry)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    HAL_SCO_L2_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                       idx = 0;

    /* check parameter */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_entry);

    if (!HAL_SCO_L2_IS_UC_ADDR(ptr_entry->mac))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_NOTIFY_LOCK(unit);

    for (idx = 0; idx < HAL_SCO_L2_NOTIFY_HANDLER_CNT; idx++)
    {
        ptr_handler = PTR_HAL_SCO_L2_NOTIFY_HANDLER(unit, idx);
        if (NULL != ptr_handler->notify_func)
        {
            ptr_handler->notify_func(unit, reason, ptr_entry, ptr_handler->ptr_cookie);
            break;
        }
    }
    HAL_SCO_L2_NOTIFY_UNLOCK(unit);

    return rc;
}

/* FUNCTION NAME:   _hal_sco_l2_polling_thread
 * PURPOSE:
 *      The polling thread to update HW L2 table to SW shadow table
 * INPUT:
 *      ptr_argv            --  Pointer for device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      None
 * NOTES:
 *      None
 */
static void
_hal_sco_l2_polling_thread(
    void *ptr_argv)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     unit = (UI32_T)(AIR_HUGE_T)ptr_argv;
    HAL_SCO_L2_POLLING_CB_T   *ptr_polling_cb = NULL;
    AIR_MAC_ENTRY_T            hw_entry;
    AIR_PORT_BITMAP_T          port_bitmap;
    HAL_SCO_L2_FDB_SW_ENTRY_T *ptr_sw_entry = NULL;
    UI32_T                     idx, entry_id, addr, bank;
    BOOL_T                     sw_entry_valid = FALSE, hw_entry_valid = FALSE;
    CMLIB_RBT_NODE_T          *ptr_rbt_node = NULL;
    void                      *ptr_node_data = NULL;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        return;
    }
    ptr_polling_cb = PTR_HAL_SCO_L2_POLLING_CB(unit);
    while (AIR_E_OK == osal_isRunThread())
    {
        osal_sleepTask(ptr_polling_cb->interval);

        entry_id = ptr_polling_cb->cur_poll_num;
        for (idx = 0; idx < ptr_polling_cb->max_poll_num; idx++, entry_id++)
        {
            if (FALSE == ptr_polling_cb->state)
            {
                break;
            }
            if (entry_id >= HAL_SCO_L2_MAX_ENTRY_NUM)
            {
                entry_id %= HAL_SCO_L2_MAX_ENTRY_NUM;
            }

            /* transfer index to addr/bank */
            HAL_SCO_L2_TRANS_IDX_TO_ADDR(entry_id, addr, bank);

            HAL_SCO_L2_FDB_LOCK(unit);

            /* get HW MAC entry by addr/bank */
            rc = _getSpecMacEntryByIndex(unit, addr, bank);
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "unit=(%u), get spec. entry failed, rc=(%d)\n", unit, rc);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                continue;
            }

            /* Select bank */
            _fill_MAC_ATRDS(unit, bank);

            /* Read MAC entry */
            /* If target entry is invalid or not unicast MAC entry,
             * Set as NULL entry */
            osal_memset(&hw_entry, 0, sizeof(AIR_MAC_ENTRY_T));
            _read_MAC_ATRD(unit, &hw_entry);
            HAL_SCO_L2_FDB_UNLOCK(unit);

            /* Translate port bitmap to air_system format */
            HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_entry.port_bitmap, port_bitmap);
            AIR_PORT_BITMAP_COPY(hw_entry.port_bitmap, port_bitmap);

            HAL_SCO_L2_SW_LOCK(unit);

            /* get the sw entry by index */
            ptr_sw_entry = PTR_HAL_SCO_L2_SW_ENTRY(unit, entry_id);

            hw_entry_valid = _isMacEntryValid(&hw_entry);
            sw_entry_valid = ptr_sw_entry->is_valid;
            if (hw_entry_valid)
            {
                if (sw_entry_valid)
                {
                    if (HAL_SCO_L2_MAC_ENTRY_DIFF(&hw_entry, &ptr_sw_entry->entry))
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "=============================\n");
                        HAL_SCO_L2_PRINT_MAC_ENTRY("SW entry", &ptr_sw_entry->entry);
                        HAL_SCO_L2_PRINT_MAC_ENTRY("HW entry", &hw_entry);
                        DIAG_PRINT(HAL_DBG_INFO, "Modify RBT node(%d)\n", entry_id);
                        /* HW:valid, SW:valid, SW!=HW => modify, call callback handler */
                        osal_memcpy(&ptr_sw_entry->entry, &hw_entry, sizeof(AIR_MAC_ENTRY_T));
                        _hal_sco_l2_callNotifyCallback(unit, AIR_L2_MAC_NOTIFY_REASON_MODIFY, &hw_entry);
                    }
                    else if (hw_entry.timer != ptr_sw_entry->entry.timer)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "=============================\n");
                        HAL_SCO_L2_PRINT_MAC_ENTRY("SW entry", &ptr_sw_entry->entry);
                        HAL_SCO_L2_PRINT_MAC_ENTRY("HW entry", &hw_entry);
                        DIAG_PRINT(HAL_DBG_INFO, "Age-time change only, Do nothing (%d)\n", entry_id);
                        /* Ignore age-timer changing to avoid notification storm */
                        osal_memcpy(&ptr_sw_entry->entry, &hw_entry, sizeof(AIR_MAC_ENTRY_T));
                    }
                    else
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "=============================\n");
                        HAL_SCO_L2_PRINT_MAC_ENTRY("SW entry", &ptr_sw_entry->entry);
                        HAL_SCO_L2_PRINT_MAC_ENTRY("HW entry", &hw_entry);
                        /* HW:valid, SW:valid, SW==HW => do nothing */
                        DIAG_PRINT(HAL_DBG_INFO, "SW=HW, Do nothing (%d)\n", entry_id);
                    }
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_INFO, "=============================\n");
                    HAL_SCO_L2_PRINT_MAC_ENTRY("SW entry", &ptr_sw_entry->entry);
                    HAL_SCO_L2_PRINT_MAC_ENTRY("HW entry", &hw_entry);
                    DIAG_PRINT(HAL_DBG_INFO, "Insert RBT node(%d)\n", entry_id);
                    /* HW:valid, SW:NULL => add, call callback handler */
                    osal_memcpy(&ptr_sw_entry->entry, &hw_entry, sizeof(AIR_MAC_ENTRY_T));
                    rc = cmlib_rbt_insert(HAL_SCO_L2_RBT_HEAD(unit), ptr_sw_entry, FALSE, NULL, (void **)&ptr_rbt_node);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Insert RBT node(%d) failed, rc=(%d)\n", entry_id, rc);
                        HAL_SCO_L2_SW_UNLOCK(unit);
                        continue;
                    }
                    if (NULL == ptr_rbt_node)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Inserted RBT node(%d) is empty\n", entry_id);
                        HAL_SCO_L2_SW_UNLOCK(unit);
                        continue;
                    }
                    ptr_sw_entry->entry_id = entry_id;
                    ptr_sw_entry->is_valid = TRUE;
                    ptr_sw_entry->ptr_rbt_node = ptr_rbt_node;
                    DIAG_PRINT(HAL_DBG_INFO, "Used entry num=(%d)\n", HAL_SCO_L2_USED_ENTRY_NUM(unit));

                    _hal_sco_l2_callNotifyCallback(unit, AIR_L2_MAC_NOTIFY_REASON_ADD, &hw_entry);
                    cmlib_rbt_traverse(HAL_SCO_L2_RBT_HEAD(unit), _hal_sco_l2_traverse_dbg_func, NULL);
                }
            }
            else
            {
                if (sw_entry_valid)
                {
                    DIAG_PRINT(HAL_DBG_INFO, "=============================\n");
                    HAL_SCO_L2_PRINT_MAC_ENTRY("SW entry", &ptr_sw_entry->entry);
                    HAL_SCO_L2_PRINT_MAC_ENTRY("HW entry", &hw_entry);
                    DIAG_PRINT(HAL_DBG_INFO, "Delete RBT node(%d)\n", entry_id);
                    /* HW:NULL, SW:valid => delete, call callback handler */
                    osal_memcpy(&hw_entry, &ptr_sw_entry->entry, sizeof(AIR_MAC_ENTRY_T));
                    DIAG_PRINT(HAL_DBG_INFO, "ptr_sw_entry->ptr_rbt_node(%p)\n", ptr_sw_entry->ptr_rbt_node);
                    rc = cmlib_rbt_deleteNode(HAL_SCO_L2_RBT_HEAD(unit), ptr_sw_entry->ptr_rbt_node, &ptr_node_data,
                                              _hal_sco_l2_delete_callback);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "Delete RBT node(%d) failed, rc=(%d)\n", entry_id, rc);
                        HAL_SCO_L2_SW_UNLOCK(unit);
                        continue;
                    }
                    osal_memset(&ptr_sw_entry->entry, 0, sizeof(AIR_MAC_ENTRY_T));
                    ptr_sw_entry->is_valid = FALSE;
                    ptr_sw_entry->ptr_rbt_node = NULL;
                    DIAG_PRINT(HAL_DBG_INFO, "Used entry num=(%d)\n", HAL_SCO_L2_USED_ENTRY_NUM(unit));
                    DIAG_PRINT(HAL_DBG_INFO, "RBT root=(%p)\n", HAL_SCO_L2_RBT_HEAD(unit)->ptr_root);

                    _hal_sco_l2_callNotifyCallback(unit, AIR_L2_MAC_NOTIFY_REASON_DELETE, &hw_entry);

                    HAL_SCO_L2_RBT_NODE_DBG_PRINT(0, &(HAL_SCO_L2_RBT_HEAD(unit)->nil_node));
                    cmlib_rbt_traverse(HAL_SCO_L2_RBT_HEAD(unit), _hal_sco_l2_traverse_dbg_func, NULL);
                }
                else
                {
                    /* HW:NULL, SW:NULL => do nothing */
                }
            }
            HAL_SCO_L2_SW_UNLOCK(unit);
        }
        ptr_polling_cb->cur_poll_num = entry_id;
    }
    osal_exitRunThread();
}

/* FUNCTION NAME: _hal_sco_l2_initPollingMode
 * PURPOSE:
 *      To initialize L2 polling mode
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_initPollingMode(
    const UI32_T unit)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    HAL_SCO_L2_POLLING_CB_T *ptr_polling_cb = NULL;
    AIR_CFG_VALUE_T          cfg_pri;
    AIR_CFG_VALUE_T          cfg_stack_size;
    AIR_CFG_VALUE_T          cfg_poll_interval;
    AIR_CFG_VALUE_T          cfg_poll_en;
    UI32_T                   poll_interval[HAL_SCO_L2_POLL_INTERVAL_LAST] = {1000, 500, 250, 125};

    /* check parameter */
    HAL_CHECK_UNIT(unit);
    ptr_polling_cb = PTR_HAL_SCO_L2_POLLING_CB(unit);

    /* thread priority */
    osal_memset(&cfg_pri, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_pri.value = HAL_SCO_L2_POLL_THREAD_PRI;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_POLL_THREAD_PRI, &cfg_pri);
    HAL_CHECK_ERROR(rc);

    /* thread stack */
    osal_memset(&cfg_stack_size, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_stack_size.value = HAL_SCO_L2_POLL_THREAD_STACK;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_POLL_THREAD_STACK, &cfg_stack_size);
    HAL_CHECK_ERROR(rc);

    /* polling interval */
    osal_memset(&cfg_poll_interval, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_poll_interval.value = HAL_SCO_L2_POLL_INTERVAL_500MS;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_POLL_INTERVAL, &cfg_poll_interval);
    HAL_CHECK_ERROR(rc);
    if (HAL_SCO_L2_POLL_INTERVAL_LAST <= cfg_poll_interval.value)
    {
        DIAG_PRINT(HAL_DBG_WARN, "unknown AIR_CFG_TYPE_L2_POLL_INTERVAL=%d\n", cfg_poll_interval.value);
        cfg_poll_interval.value = HAL_SCO_L2_POLL_INTERVAL_500MS;
    }

    /* polling thread state */
    osal_memset(&cfg_poll_en, 0, sizeof(AIR_CFG_VALUE_T));
    cfg_poll_en.value = 1;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_POLL_ENABLE, &cfg_poll_en);
    HAL_CHECK_ERROR(rc);

    /* set control block */
    osal_memset(ptr_polling_cb, 0, sizeof(HAL_SCO_L2_POLLING_CB_T));
    ptr_polling_cb->state = (1 == cfg_poll_en.value) ? TRUE : FALSE;
    ptr_polling_cb->interval = poll_interval[cfg_poll_interval.value];
    ptr_polling_cb->max_poll_num = HAL_SCO_L2_POLL_MAX_NUM;
    ptr_polling_cb->cur_poll_num = 0;

    /* create polling thread */
    rc = osal_createThread("L2_FDB_POLL", cfg_stack_size.value, cfg_pri.value, _hal_sco_l2_polling_thread,
                           (void *)((AIR_HUGE_T)unit), &ptr_polling_cb->task_id);

    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_deinitPollingMode
 * PURPOSE:
 *      To de-initialize L2 polling mode
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_deinitPollingMode(
    const UI32_T unit)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    HAL_SCO_L2_POLLING_CB_T *ptr_polling_cb = NULL;

    /* check parameter */
    HAL_CHECK_UNIT(unit);
    ptr_polling_cb = PTR_HAL_SCO_L2_POLLING_CB(unit);

    /* Destory polling thread */
    rc = osal_stopThread(&ptr_polling_cb->task_id);
    HAL_CHECK_ERROR(rc);
    rc = osal_destroyThread(&ptr_polling_cb->task_id);
    HAL_CHECK_ERROR(rc);

    /* Reset control block */
    osal_memset(ptr_polling_cb, 0, sizeof(HAL_SCO_L2_POLLING_CB_T));

    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_flush_callback
 * PURPOSE:
 *      The callback function of hal_sco_l2_flushMacAddr
 * INPUT:
 *      unit                --  Device unit number
 *      ptr_mac_entry       --  The traversed L2 MAC address entry
 *      ptr_cookie          --  The condition for deciding whether to delete
 *                              MAC entry will be evaluated
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_flush_callback(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry,
    void                  *ptr_cookie)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    HAL_SCO_L2_FLUSH_INFO_T *ptr_flush_info;

    HAL_CHECK_PTR(ptr_mac_entry);
    HAL_CHECK_PTR(ptr_cookie);
    /* Skip the static MAC entry */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC)
    {
        return AIR_E_OK;
    }

    ptr_flush_info = (HAL_SCO_L2_FLUSH_INFO_T *)ptr_cookie;
    switch (ptr_flush_info->type)
    {
        case AIR_L2_MAC_FLUSH_TYPE_VID:
            if ((ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL) && (ptr_mac_entry->cvid == ptr_flush_info->value))
            {
                rc = hal_sco_l2_delMacAddr(unit, ptr_mac_entry);
            }
            break;
        case AIR_L2_MAC_FLUSH_TYPE_FID:
            if (!(ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL) && (ptr_mac_entry->fid == ptr_flush_info->value))
            {
                rc = hal_sco_l2_delMacAddr(unit, ptr_mac_entry);
            }
            break;
        case AIR_L2_MAC_FLUSH_TYPE_PORT:
            if (AIR_PORT_CHK(ptr_mac_entry->port_bitmap, ptr_flush_info->value))
            {
                rc = hal_sco_l2_delMacAddr(unit, ptr_mac_entry);
            }
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_delete_callback
 * PURPOSE:
 *      The callback function of cmlib_rbt_deleteNode to update node and data
 *      mapping when delete node from RB tree
 * INPUT:
 *      ptr_node            --  The pointer of moved RBT node
 *      ptr_user_data       --  The pointer of user data of original RBT node
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_hal_sco_l2_delete_callback(
    void *ptr_node,
    void *ptr_user_data)
{
    HAL_SCO_L2_FDB_SW_ENTRY_T *ptr_sw_entry = NULL;
    if (!ptr_node && !ptr_user_data)
    {
        DIAG_PRINT(HAL_DBG_ERR, "ptr_node=(%p), ptr_user_data=(%d)\n", ptr_node, ptr_user_data);
        return;
    }
    ptr_sw_entry = (HAL_SCO_L2_FDB_SW_ENTRY_T *)ptr_user_data;
    ptr_sw_entry->ptr_rbt_node = (CMLIB_RBT_NODE_T *)ptr_node;
    DIAG_PRINT(HAL_DBG_INFO, "ptr_sw_entry->ptr_rbt_node(%p)=(%d)\n", &ptr_sw_entry->ptr_rbt_node,
               ptr_sw_entry->ptr_rbt_node);
}

#endif /* End of AIR_EN_L2_SHADOW */

/* FUNCTION NAME: _hal_sco_l2_initRsrc
 * PURPOSE:
 *      To initialize module resource of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_initRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    HAL_SCO_L2_FDB_CB_T *ptr_cb = NULL;
#ifdef AIR_EN_L2_SHADOW
    UI32_T table_size;
#endif /* End of AIR_EN_L2_SHADOW */

    HAL_CHECK_UNIT(unit);

    ptr_cb = &(_l2_fdb_cb[unit]);
    osal_memset(ptr_cb, 0, sizeof(HAL_SCO_L2_FDB_CB_T));

#ifdef AIR_EN_L2_SHADOW
    /* Allocate memory for shadow table */
    table_size = HAL_SCO_L2_MAX_ENTRY_NUM * sizeof(HAL_SCO_L2_FDB_SW_ENTRY_T);
    ptr_cb->ptr_sw_entry = (HAL_SCO_L2_FDB_SW_ENTRY_T *)osal_alloc(table_size, air_module_getModuleName(AIR_MODULE_L2));
    if (NULL == ptr_cb->ptr_sw_entry)
    {
        DIAG_PRINT(HAL_DBG_ERR, "unit=(%u) allocate memory failed\n", unit);
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_cb->ptr_sw_entry, 0, table_size);

    /* Create RB Tree */
    rc = cmlib_rbt_create(NULL, 0, _hal_sco_l2_rbtCmpCallback, "l2_fdb_rbt", &(HAL_SCO_L2_RBT_HEAD(unit)));
    HAL_CHECK_ERROR(rc);
#endif /* End of AIR_EN_L2_SHADOW */

    /* Create mutex lock */
    rc = osal_createSemaphore("L2_HW_TABLE", AIR_SEMAPHORE_BINARY, PTR_HAL_SCO_L2_HW_TBL_MUTEX(unit),
                              air_module_getModuleName(AIR_MODULE_L2));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Create semaphore failed, rc=(%d)\n", rc);
    }

#ifdef AIR_EN_L2_SHADOW
    rc = osal_createSemaphore("L2_SW_TABLE", AIR_SEMAPHORE_BINARY, PTR_HAL_SCO_L2_SW_TBL_MUTEX(unit),
                              air_module_getModuleName(AIR_MODULE_L2));
    HAL_CHECK_ERROR(rc);

    rc = osal_createSemaphore("L2_NOTIFY", AIR_SEMAPHORE_BINARY, PTR_HAL_SCO_L2_NOTIFY_MUTEX(unit),
                              air_module_getModuleName(AIR_MODULE_L2));
    HAL_CHECK_ERROR(rc);
#endif /* End of AIR_EN_L2_SHADOW */
    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_deinitRsrc
 * PURPOSE:
 *      To deinitialize module resource of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_deinitRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    HAL_SCO_L2_FDB_CB_T *ptr_cb = NULL;

    HAL_CHECK_UNIT(unit);

    ptr_cb = &(_l2_fdb_cb[unit]);

    /* Destory mutex lock */
#ifdef AIR_EN_L2_SHADOW
    rc = osal_destroySemaphore(PTR_HAL_SCO_L2_NOTIFY_MUTEX(unit));
    rc = osal_destroySemaphore(PTR_HAL_SCO_L2_SW_TBL_MUTEX(unit));
#endif /* End of AIR_EN_L2_SHADOW */
    rc = osal_destroySemaphore(PTR_HAL_SCO_L2_HW_TBL_MUTEX(unit));

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Destroy semaphore failed, rc=(%d)\n", rc);
    }
#ifdef AIR_EN_L2_SHADOW
    HAL_CHECK_ERROR(rc);

    /* Destory RB Tree */
    rc = cmlib_rbt_destroy(HAL_SCO_L2_RBT_HEAD(unit), NULL);
    HAL_CHECK_ERROR(rc);

    /* Release memory from shadow table */
    osal_free(ptr_cb->ptr_sw_entry);
#endif /* End of AIR_EN_L2_SHADOW */

    /* Reset control block */
    osal_memset(ptr_cb, 0, sizeof(HAL_SCO_L2_FDB_CB_T));
    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_initModule
 * PURPOSE:
 *      To initialize module configuration of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_initModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    AIR_FORWARD_TYPE_T type;
    AIR_PORT_BITMAP_T  pbm = {0};
    AIR_CFG_VALUE_T    cfg_l2fdb_full;
    UI32_T             u32dat = 0;

    /* Translate port bitmap */
    AIR_PORT_BITMAP_COPY(pbm, HAL_PORT_BMP_ETH(unit));
    /* Exclude CPU port */
    if (HAL_IS_PORT_VALID(unit, HAL_CPU_PORT(unit)))
    {
        AIR_PORT_DEL(pbm, HAL_CPU_PORT(unit));
    }
    /* Initialize L2 forwarding mode as port-bitmap to all ether port for mc, bc, uuc, uipmc */
    for (type = AIR_FORWARD_TYPE_BCST; type < AIR_FORWARD_TYPE_LAST; type++)
    {
        /* Flood to ether ports without CPU port */
        hal_sco_l2_setForwardMode(unit, type, AIR_FORWARD_ACTION_TO_PBM, pbm);
    }
    /* Initialize L2 HW learning behavior when L2 FDB full */
    osal_memset(&cfg_l2fdb_full, 0, sizeof(AIR_CFG_VALUE_T));
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_FDB_FULL_BEHAVIOR, &cfg_l2fdb_full);
    HAL_CHECK_ERROR(rc);
    if (HAL_SCO_L2_FDB_FULL_HW_REPLACE_OLDEST_DYNAMIC_ENTRY == cfg_l2fdb_full.value)
    {
        aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
        u32dat |= BITS_OFF_L(1UL, AGC_MAC_OLD_RPLC_HW_OFFT, AGC_MAC_OLD_RPLC_HW_LENG);
        aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));
    }

    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_deinitModule
 * PURPOSE:
 *      To deinitialize module configuration of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_deinitModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat;

    hal_sco_l2_clearMacAddr(unit);

    u32dat = HAL_SCO_L2_AAC_DEFAULT_VALUE;
    aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_L2_ADDRTBL_DEFAULT_VALUE;
    aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
    aml_readReg(unit, ATA1, &u32dat, sizeof(u32dat));
    aml_readReg(unit, ATA2, &u32dat, sizeof(u32dat));
    aml_readReg(unit, ATWD, &u32dat, sizeof(u32dat));
    aml_readReg(unit, ATWD2, &u32dat, sizeof(u32dat));
    aml_readReg(unit, ATRDS, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_L2_AGDIS_DEFAULT_VALUE;
    aml_readReg(unit, AGDIS, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_L2_LOGPORT_DEFAULT_VALUE;
    aml_readReg(unit, LOGMACPORT, &u32dat, sizeof(u32dat));
    aml_readReg(unit, LOGAGEPORT, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_L2_FORWARD_VALUE;
    aml_readReg(unit, BCF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, UNMF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, UNUF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, UNIPMF, &u32dat, sizeof(u32dat));

    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_initThread
 * PURPOSE:
 *      To initialize thread of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_initThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_L2_SHADOW
    HAL_CHECK_UNIT(unit);

    /* Initailize polling mode */
    rc = _hal_sco_l2_initPollingMode(unit);
    HAL_CHECK_ERROR(rc);

#endif /* End of AIR_EN_L2_SHADOW */
    return rc;
}

/* FUNCTION NAME: _hal_sco_l2_deinitThread
 * PURPOSE:
 *      To deinitialize thread of L2 MAC table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_l2_deinitThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_L2_SHADOW
    rc = _hal_sco_l2_deinitPollingMode(unit);
    HAL_CHECK_ERROR(rc);
#endif /* End of AIR_EN_L2_SHADOW */
    return rc;
}

#ifndef AIR_EN_L2_SHADOW
/* FUNCTION NAME:  _hal_sco_l2_searchMacAddr
 * PURPOSE:
 *      Search L2 unicast MAC address entries by filter.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *                          AIR_MAC_ENTRY_T
 *      type            --  The type of searching by specific filter
 *                          AIR_L2_MAC_SEARCH_TYPE_T
 *      value           --  The value of filter
 *      filter          --  The filter type to search
 *                          _HAL_SCO_L2_MAC_MS_T
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 *                          AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
_hal_sco_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry,
    _HAL_SCO_L2_MAC_MS_T           search_cmd)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                i = 0;
    AIR_MAC_ENTRY_T       hw_mac_entry;
    UI32_T                addr = 0;
    UI32_T                banks = 0;
    UI32_T                mac_port = 0;
    _HAL_SCO_L2_MAC_MAT_T mat;

    osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
    switch (type)
    {
        case AIR_L2_MAC_SEARCH_TYPE_VID:
            mat = _HAL_SCO_L2_MAC_MAT_MAC_BY_VID;
            hw_mac_entry.cvid = (UI16_T)value;
            break;
        case AIR_L2_MAC_SEARCH_TYPE_FID:
            mat = _HAL_SCO_L2_MAC_MAT_MAC_BY_FID;
            hw_mac_entry.fid = (UI16_T)value;
            break;
        case AIR_L2_MAC_SEARCH_TYPE_PORT:
            mat = _HAL_SCO_L2_MAC_MAT_MAC_BY_PORT;
            HAL_AIR_PORT_TO_MAC_PORT(unit, value, mac_port);
            AIR_PORT_ADD(hw_mac_entry.port_bitmap, mac_port);
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }
    (*ptr_count) = 0;

    HAL_SCO_L2_FDB_LOCK(unit);
    _fill_MAC_ATWD(unit, &hw_mac_entry, TRUE);

    rc = _searchMacEntry(unit, search_cmd, mat, &addr, &banks);

    switch (rc)
    {
        case AIR_E_OK:
            /* Searching bank and read data */
            DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
            if (0 == banks)
            {
                rc = AIR_E_ENTRY_NOT_FOUND;
                break;
            }

            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &hw_mac_entry);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                        continue;
                    }
                    osal_memcpy(&ptr_mac_entry[(*ptr_count)], &hw_mac_entry, sizeof(AIR_MAC_ENTRY_T));
                    /* Translate port bitmap type */
                    HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_mac_entry.port_bitmap, ptr_mac_entry[(*ptr_count)].port_bitmap);
                    (*ptr_count)++;
                }
            }
            break;
        case AIR_E_TIMEOUT:
            /* Searching over time */
            rc = AIR_E_TIMEOUT;
            break;
        default:
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            rc = AIR_E_ENTRY_NOT_FOUND;
            break;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return rc;
}
#endif /* End of AIR_EN_L2_SHADOW */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_sco_l2_init
 * PURPOSE:
 *      Initialization of L2 MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE(unit))
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_sco_l2_initRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = _hal_sco_l2_initModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = _hal_sco_l2_initThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_L2) |= HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_deinit
 * PURPOSE:
 *      Deinitialization of L2 MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (!(HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE(unit)))
    {
        rc = AIR_E_NOT_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_sco_l2_deinitRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = _hal_sco_l2_deinitModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = _hal_sco_l2_deinitThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_L2) &= ~HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_addMacAddr
 * PURPOSE:
 *      Add or set a L2 unicast MAC address entry.
 *      If the address entry doesn't exist, it will add the entry.
 *      If the address entry already exists, it will set the entry
 *      with user input value.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TABLE_FULL
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_addMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T          u32dat;
    UI32_T          reg_aac;
    UI32_T          age_cnt;
    UI32_T          age_unit, sys_unit;
    AIR_MAC_ENTRY_T set_mac_entry;

    /* Check elements in the structure of MAC table */
    HAL_CHECK_PORT_BITMAP(unit, ptr_mac_entry->port_bitmap);
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        HAL_CHECK_VLAN(ptr_mac_entry->cvid);
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(ptr_mac_entry->fid, 0, (AIR_STP_FID_NUMBER - 1));
    }

    /* Set the target MAC entry as setting entry no mater the hash addrees is existed or not */
    osal_memcpy(&set_mac_entry, ptr_mac_entry, sizeof(AIR_MAC_ENTRY_T));
    /* Translate port bitmap type */
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_mac_entry->port_bitmap, set_mac_entry.port_bitmap);
    HAL_SCO_L2_FDB_LOCK(unit);
    /* set aging counter as system aging conuter */
    aml_readReg(unit, AAC, &reg_aac, sizeof(reg_aac));
    if (ptr_mac_entry->timer != AIR_L2_MAC_DEF_AGE_OUT_TIME)
    {
        _HAL_SCO_L2_AGING_TIME_2_CNT(ptr_mac_entry->timer, age_cnt, age_unit);
        sys_unit = BITS_OFF_R(reg_aac, AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH) + 1;
        if (age_unit == sys_unit)
        {
            set_mac_entry.timer = age_cnt;
        }
        else if (age_unit > sys_unit)
        {
            return AIR_E_BAD_PARAMETER;
        }
        else
        {
            set_mac_entry.timer = ptr_mac_entry->timer / sys_unit;
        }
    }
    else
    {
        set_mac_entry.timer = BITS_OFF_R(reg_aac, AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);
    }

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &set_mac_entry);
    _fill_MAC_ATWD(unit, &set_mac_entry, TRUE);

    /* Write data by ATC */
    u32dat = (ATC_SAT_MAC | ATC_CMD_WRITE | ATC_START_BUSY);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }
    if (!_checkL2EntryHit(unit))
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TABLE_FULL;
    }
    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_delMacAddr
 * PURPOSE:
 *      Delete a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
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
hal_sco_l2_delMacAddr(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T          u32dat;
    AIR_MAC_ENTRY_T del_mac_entry;

    /* Check elements in the structure of MAC table */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        HAL_CHECK_VLAN(ptr_mac_entry->cvid);
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(ptr_mac_entry->fid, 0, (AIR_STP_FID_NUMBER - 1));
    }

    osal_memcpy(&del_mac_entry, ptr_mac_entry, sizeof(AIR_MAC_ENTRY_T));

    HAL_SCO_L2_FDB_LOCK(unit);

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &del_mac_entry);
    _fill_MAC_ATWD(unit, &del_mac_entry, FALSE);

    /* Write data by ATC to delete entry */
    u32dat = (ATC_SAT_MAC | ATC_CMD_WRITE | ATC_START_BUSY);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME:  hal_sco_l2_getMacAddr
 * PURPOSE:
 *      Get a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is empty.
 *      It means to search the first valid MAC address entry in MAC address table.
 *      Otherwise, to search the specific MAC address entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
hal_sco_l2_getMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_MAC_ENTRY_T hw_mac_entry;
    UI32_T          bank = 0, banks = 0;
    UI32_T          u32dat = 0;
#ifndef AIR_EN_L2_SHADOW
    UI32_T i = 0;
    UI32_T addr = 0;
#endif /* End of AIR_EN_L2_SHADOW */

    /* Check elements in the structure of MAC table */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        HAL_CHECK_VLAN(ptr_mac_entry->cvid);
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(ptr_mac_entry->fid, 0, (AIR_STP_FID_NUMBER - 1));
    }

    (*ptr_count) = 0;

    /* Check MAC Address field of input data */
    if (_isMacEntryValid(ptr_mac_entry))
    {
        /* MAC address isn't empty, means to search a specific MAC entry */
        HAL_SCO_L2_FDB_LOCK(unit);
        rc = _getSpecMacEntryByMac(unit, ptr_mac_entry, &banks);
        if (AIR_E_OK != rc)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            return rc;
        }
        /* Select bank */
        aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        bank = 0;
        while (banks > 1)
        {
            banks >>= 1;
            bank++;
        }
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(%08lX), bank=%u\n", u32dat, bank);
        _fill_MAC_ATRDS(unit, bank);

        /* Read MAC entry */
        osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
        rc = _read_MAC_ATRD(unit, &hw_mac_entry);
        HAL_SCO_L2_FDB_UNLOCK(unit);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            return rc;
        }

        /* The found MAC is the target, restore data and leave */
        osal_memcpy(ptr_mac_entry, &hw_mac_entry, sizeof(AIR_MAC_ENTRY_T));
        /* Translate port bitmap type */
        HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_mac_entry.port_bitmap, ptr_mac_entry->port_bitmap);
        (*ptr_count) = 1;
        return AIR_E_OK;
    }
    else
    {
#ifdef AIR_EN_L2_SHADOW
        DIAG_PRINT(HAL_DBG_WARN, "unit(%u), MAC address is empty\n", unit);
        return AIR_E_BAD_PARAMETER;
#else  /* Else of AIR_EN_L2_SHADOW */
        /* MAC address is empty, means to search the 1st MAC entry */
        HAL_SCO_L2_FDB_LOCK(unit);
        rc = _searchMacEntry(unit, _HAL_SCO_L2_MAC_MS_START, _HAL_SCO_L2_MAC_MAT_MAC, &addr, &banks);

        switch (rc)
        {
            case AIR_E_OK:
                /* Searching bank and read data */
                DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
                if (0 == banks)
                {
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
                {
                    if (!!BITS_OFF_R(banks, i, 1))
                    {
                        /* Found a valid MAC entry */
                        /* Select bank */
                        _fill_MAC_ATRDS(unit, i);

                        /* Read MAC entry */
                        osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
                        rc = _read_MAC_ATRD(unit, &hw_mac_entry);
                        if (AIR_E_OK != rc)
                        {
                            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                            continue;
                        }
                        osal_memcpy(&ptr_mac_entry[(*ptr_count)], &hw_mac_entry, sizeof(AIR_MAC_ENTRY_T));
                        /* Translate port bitmap type */
                        HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_mac_entry.port_bitmap,
                                                 ptr_mac_entry[(*ptr_count)].port_bitmap);
                        (*ptr_count)++;
                    }
                }
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_OK;
            case AIR_E_TIMEOUT:
                /* Searching over time */
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_TIMEOUT;
            default:
                DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
        }
#endif /* End of AIR_EN_L2_SHADOW */
    }
}

/* FUNCTION NAME: hal_sco_l2_getNextMacAddr
 * PURPOSE:
 *      Get the next L2 unicast MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than ((# of Bank) * (Table size))
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is empty.
 *      It means to search the next valid MAC address entries of last searching result.
 *      Otherwise, to search the next valid MAC address entry of the specific MAC address
 *      entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
hal_sco_l2_getNextMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
#ifdef AIR_EN_L2_SHADOW
    return AIR_E_NOT_SUPPORT;
#else  /* Else of AIR_EN_L2_SHADOW */
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          i = 0;
    BOOL_T          found_target = FALSE;
    AIR_MAC_ENTRY_T hw_mac_entry;
    UI32_T          addr;
    UI32_T          banks;

    /* Check elements in the structure of MAC table */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        HAL_CHECK_VLAN(ptr_mac_entry->cvid);
    }
    else
    {
        HAL_CHECK_MIN_MAX_RANGE(ptr_mac_entry->fid, 0, (AIR_STP_FID_NUMBER - 1));
    }

    /* If found the lastest entry last time, we couldn't keep to search the next entry */
    if (TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    (*ptr_count) = 0;

    HAL_SCO_L2_FDB_LOCK(unit);
    /* Check MAC Address field of input data */
    if (_isMacEntryValid(ptr_mac_entry))
    {
        /* MAC address isn't empty, means to search the next entries of input MAC Address */
        /* Search the target MAC entry */
        _fill_MAC_ATA(unit, ptr_mac_entry);
        rc = _searchMacEntry(unit, _HAL_SCO_L2_MAC_MS_START, _HAL_SCO_L2_MAC_MAT_MAC, &addr, &banks);
        while (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
            if (0 == banks)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &hw_mac_entry);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                        continue;
                    }
                    if (!HAL_SCO_L2_MAC_ADDR_DIFF(ptr_mac_entry->mac, hw_mac_entry.mac))
                    {
                        /* The found MAC is the target, restore data and leave */
                        found_target = TRUE;
                        break;
                    }
                }
            }

            if (TRUE == found_target)
            {
                break;
            }

            /* The found MAC isn't the target, keep searching or leave
             * when found the last entry */
            if (TRUE == _search_end)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            else
            {
                rc = _searchMacEntry(unit, _HAL_SCO_L2_MAC_MS_NEXT, _HAL_SCO_L2_MAC_MAT_MAC, &addr, &banks);
            }
        }

        if (FALSE == found_target)
        {
            /* Entry not bank */
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
        else
        {
            /* Found the target MAC entry, and try to search the next address */
            rc = _searchMacEntry(unit, _HAL_SCO_L2_MAC_MS_NEXT, _HAL_SCO_L2_MAC_MAT_MAC, &addr, &banks);
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
                if (0 == banks)
                {
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
                {
                    if (!!BITS_OFF_R(banks, i, 1))
                    {
                        /* Found a valid MAC entry */
                        /* Select bank */
                        _fill_MAC_ATRDS(unit, i);

                        /* Read MAC entry */
                        osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
                        rc = _read_MAC_ATRD(unit, &hw_mac_entry);
                        if (AIR_E_OK != rc)
                        {
                            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                            continue;
                        }
                        osal_memcpy(&ptr_mac_entry[(*ptr_count)], &hw_mac_entry, sizeof(AIR_MAC_ENTRY_T));
                        /* Translate port bitmap type */
                        HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_mac_entry.port_bitmap,
                                                 ptr_mac_entry[(*ptr_count)].port_bitmap);
                        (*ptr_count)++;
                    }
                }
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_OK;
            }
            else
            {
                DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        /* MAC address is empty, means to search next entry */
        rc = _searchMacEntry(unit, _HAL_SCO_L2_MAC_MS_NEXT, _HAL_SCO_L2_MAC_MAT_MAC, &addr, &banks);
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
            if (0 == banks)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &hw_mac_entry);
                    if (AIR_E_OK != rc)
                    {
                        DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                        continue;
                    }
                    osal_memcpy(&ptr_mac_entry[(*ptr_count)], &hw_mac_entry, sizeof(AIR_MAC_ENTRY_T));
                    /* Translate port bitmap type */
                    HAL_MAC_PBMP_TO_AIR_PBMP(unit, hw_mac_entry.port_bitmap, ptr_mac_entry[(*ptr_count)].port_bitmap);
                    (*ptr_count)++;
                }
            }
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_OK;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }
#endif /* End of AIR_EN_L2_SHADOW */
}

/* FUNCTION NAME:  hal_sco_l2_searchMacAddr
 * PURPOSE:
 *      Search L2 unicast MAC address entries by filter.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *                          AIR_MAC_ENTRY_T
 *      type            --  The type of searching by specific filter
 *                          AIR_L2_MAC_SEARCH_TYPE_T
 *      value           --  The value of filter
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 *                          AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      hal_sco_l2_searchNextMacAddr(). It should not be used interchangeably
 *      with hal_sco_l2_getMacAddr()/hal_sco_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_sco_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)
{
#ifdef AIR_EN_L2_SHADOW
    return AIR_E_NOT_SUPPORT;
#else  /* Else of AIR_EN_L2_SHADOW */
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _hal_sco_l2_searchMacAddr(unit, type, value, ptr_count, ptr_mac_entry, _HAL_SCO_L2_MAC_MS_START);
    return rc;
#endif /* End of AIR_EN_L2_SHADOW */
}

/* FUNCTION NAME: hal_sco_l2_searchNextMacAddr
 * PURPOSE:
 *      Search the next L2 unicast MAC address entries by filter.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *                          AIR_MAC_ENTRY_T
 *      type            --  The type of searching by specific filter
 *                          AIR_L2_MAC_SEARCH_TYPE_T
 *      value           --  The value of filter
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than
 *                          ((# of Bank) * (Size of entry structure))
 *                          AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      This function is designed to be used in conjunction with
 *      hal_sco_l2_searchMacAddr(). It should not be used interchangeably
 *      with hal_sco_l2_getMacAddr()/hal_sco_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_sco_l2_searchNextMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)

{
#ifdef AIR_EN_L2_SHADOW
    return AIR_E_NOT_SUPPORT;
#else  /* Else of AIR_EN_L2_SHADOW */
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    rc = _hal_sco_l2_searchMacAddr(unit, type, value, ptr_count, ptr_mac_entry, _HAL_SCO_L2_MAC_MS_NEXT);
    return rc;
#endif /* End of AIR_EN_L2_SHADOW */
}

/* FUNCTION NAME: hal_sco_l2_clearMacAddr
 * PURPOSE:
 *      Clear all L2 unicast MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
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
hal_sco_l2_clearMacAddr(
    const UI32_T unit)
{
    UI32_T u32dat;

    /* Write data by ATC to clear all MAC address entries */
    u32dat = (ATC_MAT_MAC | ATC_CMD_CLEAN | ATC_START_BUSY);
    HAL_SCO_L2_FDB_LOCK(unit);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_flushMacAddr
 * PURPOSE:
 *      Flush all L2 unicast MAC address entries by vid, by fid or by port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      type            --  Flush l2 mac address by vid/fid/port
 *      value           --  value of vid/fid/port
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
hal_sco_l2_flushMacAddr(
    const UI32_T                  unit,
    const AIR_L2_MAC_FLUSH_TYPE_T type,
    const UI32_T                  value)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_L2_SHADOW
    HAL_SCO_L2_FLUSH_INFO_T flush_info;
    flush_info.type = type;
    flush_info.value = value;
    rc = hal_sco_l2_traverseMacAddr(unit, _hal_sco_l2_flush_callback, &flush_info);
    return rc;
#else  /* Else of AIR_EN_L2_SHADOW */
#ifdef AIR_EN_L2_HW_FLUSH
    UI32_T reg_atc, reg_atwd;
    UI32_T mac_port = 0;

    HAL_SCO_L2_FDB_LOCK(unit);

    reg_atc = (ATC_START_BUSY | ATC_CMD_CLEAN);
    switch (type)
    {
        case AIR_L2_MAC_FLUSH_TYPE_VID:
            reg_atc |= ATC_MAT_MAC_BY_VID;
            reg_atwd = BITS_OFF_L(value, ATWD_MAC_VID_OFFSET, ATWD_MAC_VID_LENGTH);
            aml_writeReg(unit, ATWD, &reg_atwd, sizeof(reg_atwd));
            break;
        case AIR_L2_MAC_FLUSH_TYPE_FID:
            reg_atc |= ATC_MAT_MAC_BY_FID;
            reg_atwd = BITS_OFF_L(value, ATWD_MAC_FID_OFFSET, ATWD_MAC_FID_LENGTH);
            aml_writeReg(unit, ATWD, &reg_atwd, sizeof(reg_atwd));
            break;
        case AIR_L2_MAC_FLUSH_TYPE_PORT:
            reg_atc |= ATC_MAT_MAC_BY_PORT;
            HAL_AIR_PORT_TO_MAC_PORT(unit, value, mac_port);
            reg_atwd = BITS_OFF_L(1UL << mac_port, ATWD2_MAC_PORT_OFFSET, ATWD2_MAC_PORT_LENGTH);
            aml_writeReg(unit, ATWD2, &reg_atwd, sizeof(reg_atwd));
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
    }
    if (AIR_E_OK == rc)
    {
        osal_delayUs(HAL_SCO_L2_DELAY_US);
        aml_writeReg(unit, ATC, &reg_atc, sizeof(reg_atc));
        if (AIR_E_TIMEOUT == _checkL2Busy(unit))
        {
            rc = AIR_E_TIMEOUT;
        }
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return rc;
#else  /* Else of AIR_EN_L2_HW_FLUSH */
    UI8_T            count = 0;
    UI32_T           i = 0, mac_num = 0, set_num = 0, wdog_count = 0;
    AIR_MAC_ENTRY_T *ptr_mac_entry = NULL;

    /* Set size of Mac Table with the defined set_num in order to reduce memory usage*/
    rc = hal_sco_l2_getMacBucketSize(unit, &set_num);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "get max. set number failed, rc=(%u)\n", rc);
        return (rc);
    }
    ptr_mac_entry = osal_alloc(sizeof(AIR_MAC_ENTRY_T) * set_num, air_module_getModuleName(AIR_MODULE_L2));
    if (NULL == ptr_mac_entry)
    {
        DIAG_PRINT(HAL_DBG_WARN, "allocate memory failed\n");
        return AIR_E_NO_MEMORY;
    }

    /* Using mac_num to represent the number of Mac Addr found in this loop, initial mac_num = 0 */
    while (1)
    {
        mac_num++;
        osal_memset(ptr_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T) * set_num);
        /* In the first loop, mac_num = 1, using hal_sco_l2_getMacAddr to find the 1st Mac Addr */
        if (1 == mac_num)
        {
            rc = hal_sco_l2_getMacAddr(unit, &count, ptr_mac_entry);
            /* If not found Mac Addr in first loop, means Mac table is empty, then break */
            if (AIR_E_ENTRY_NOT_FOUND == rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "MAC table is empty\n");
                break;
            }
        }
        /* In other loops, mac_num >= 2, using hal_sco_l2_getNextMacAddr to get the other Mac Addr */
        else
        {
            rc = hal_sco_l2_getNextMacAddr(unit, &count, ptr_mac_entry);
            if (AIR_E_ENTRY_NOT_FOUND == rc)
            {
                break;
            }
        }
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "flush mac table failed, rc=(%u)\n", rc);
            break;
        }
        /* Function get(Next)MacAddr one entry from each bank,
         * if there are two banks, then get two mac addr */
        for (i = 0; i < count; i++)
        {
            /* Continue if a mac address is static */
            if (ptr_mac_entry[i].flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC)
            {
                continue;
            }
            if (AIR_L2_MAC_FLUSH_TYPE_PORT == type)
            {
                /* If flush Mac Addr by port,
                 * (1 << value) represents port num on which the Mac Addr would be flushed */
                if ((1 << value) & ptr_mac_entry[i].port_bitmap[0])
                {
                    rc = hal_sco_l2_delMacAddr(unit, &(ptr_mac_entry[i]));
                }
            }
            else if (AIR_L2_MAC_FLUSH_TYPE_FID == type)
            {
                if ((!(ptr_mac_entry[i].flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)) && (value == ptr_mac_entry[i].fid))
                {
                    rc = hal_sco_l2_delMacAddr(unit, &(ptr_mac_entry[i]));
                }
            }
            else if (AIR_L2_MAC_FLUSH_TYPE_VID == type)
            {
                if ((ptr_mac_entry[i].flags & AIR_L2_MAC_ENTRY_FLAGS_IVL) && (value == ptr_mac_entry[i].cvid))
                {
                    rc = hal_sco_l2_delMacAddr(unit, &(ptr_mac_entry[i]));
                }
            }
            else
            {
                continue;
            }
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "delete mac address failed, rc=(%u)\n", rc);
            }
        }

        wdog_count += count;
        if (wdog_count >= HAL_SCO_L2_WDOG_KICK_NUM)
        {
            osal_wdog_kick();
            wdog_count -= HAL_SCO_L2_WDOG_KICK_NUM;
        }
    }

    osal_free(ptr_mac_entry);
    return AIR_E_OK;
#endif /* End of AIR_EN_L2_HW_FLUSH */
#endif /* End of AIR_EN_L2_SHADOW */
}

/* FUNCTION NAME: hal_sco_l2_setMacAddrAgeOut
 * PURPOSE:
 *      Set the age out time of L2 MAC address entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *      age_time        --  age out time (second)
 *                          (1..AIR_L2_MAC_MAX_AGE_OUT_TIME)
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
hal_sco_l2_setMacAddrAgeOut(
    const UI32_T unit,
    const UI32_T age_time)
{
    UI32_T u32dat;
    UI32_T age_cnt, age_unit;

    /* Read the old register value */
    aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));

    u32dat &= ~BITS_RANGE(AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    u32dat &= ~BITS_RANGE(AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);

    /* Calcuate the aging count/unit */
    _HAL_SCO_L2_AGING_TIME_2_CNT(age_time, age_cnt, age_unit);

    /* Write the new register value */
    u32dat |= BITS_OFF_L((age_unit - 1), AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    u32dat |= BITS_OFF_L((age_cnt - 1), AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);

    aml_writeReg(unit, AAC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_getMacAddrAgeOut
 * PURPOSE:
 *      Get the age out time of unicast MAC address.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_age_time    --  age out time
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_getMacAddrAgeOut(
    const UI32_T unit,
    UI32_T      *ptr_age_time)
{
    UI32_T u32dat;
    UI32_T age_cnt, age_unit;

    /* Read data from register */
    aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));

    age_cnt = BITS_OFF_R(u32dat, AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);
    age_unit = BITS_OFF_R(u32dat, AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    (*ptr_age_time) = _HAL_SCO_L2_AGING_TIME(age_cnt, age_unit);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_setMacAddrAgeOutMode
 * PURPOSE:
 *      Set the age out mode for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      enable          --  TRUE:   Enable L2 MAC table aging out.
 *                          FALSE:  Disable L2 MAC table aging out.
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
hal_sco_l2_setMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI32_T u32dat;
    UI32_T mac_port;

    /* Translate port type */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Access regiser */
    aml_readReg(unit, AGDIS, &u32dat, sizeof(u32dat));
    if (TRUE == enable)
    {
        u32dat &= ~BIT(mac_port);
    }
    else
    {
        u32dat |= BIT(mac_port);
    }
    aml_writeReg(unit, AGDIS, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_getMacAddrAgeOutMode
 * PURPOSE:
 *      Get the age out mode for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_enable      --  TRUE:   Enable L2 MAC table aging out.
 *                          FALSE:  Disable L2 MAC table aging out.
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_getMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat;
    UI32_T mac_port;

    /* Translate port type */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Access regiser */
    aml_readReg(unit, AGDIS, &u32dat, sizeof(u32dat));
    (*ptr_enable) = (!!BITS_OFF_R(u32dat, mac_port, 1)) ? FALSE : TRUE;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_getMacBucketSize
 * PURPOSE:
 *      Get the bucket size of one MAC address set when searching L2 table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_size        --  The bucket size
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_getMacBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size)
{
    /* Access regiser */
    (*ptr_size) = HAL_SCO_L2_MAC_SET_NUM;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_setForwardMode
 * PURPOSE:
 *      Set per port forwarding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  AIR_FORWARD_TYPE_BCST
 *                          AIR_FORWARD_TYPE_UMCST
 *                          AIR_FORWARD_TYPE_UUCST
 *                          AIR_FORWARD_TYPE_UIPMCST
 *      action          --  AIR_FORWARD_DROP
 *                          AIR_FORWARD_FLOODING
 *                          AIR_FORWARD_TO_PBM
 *      port_bitmap     --  Forwarding port bitmap
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
hal_sco_l2_setForwardMode(
    const UI32_T               unit,
    const AIR_FORWARD_TYPE_T   type,
    const AIR_FORWARD_ACTION_T action,
    const AIR_PORT_BITMAP_T    port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            offset = 0;
    AIR_PORT_BITMAP_T pbm = {0}, mac_pbm = {0};

    switch (type)
    {
        case AIR_FORWARD_TYPE_BCST:
            offset = BCF;
            break;
        case AIR_FORWARD_TYPE_UMCST:
            offset = UNMF;
            break;
        case AIR_FORWARD_TYPE_UUCST:
            offset = UNUF;
            break;
        case AIR_FORWARD_TYPE_UIPMCST:
            offset = UNIPMF;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    switch (action)
    {
        case AIR_FORWARD_ACTION_FLOODING:
            /* Translate port bitmap type */
            AIR_PORT_BITMAP_COPY(pbm, HAL_PORT_BMP_ETH(unit));
            /* Translate port bitmap type */
            HAL_AIR_PBMP_TO_MAC_PBMP(unit, pbm, mac_pbm);
            break;
        case AIR_FORWARD_ACTION_DROP:
            AIR_PORT_BITMAP_CLEAR(mac_pbm);
            break;
        case AIR_FORWARD_ACTION_TO_PBM:
            /* Translate port bitmap type */
            HAL_AIR_PBMP_TO_MAC_PBMP(unit, port_bitmap, mac_pbm);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    DIAG_PRINT(HAL_DBG_INFO, "Type=(%d), Action=(%d), pbm[0]=(0x%x)\n", type, action, mac_pbm[0]);
    if (AIR_E_OK == rc)
    {
        rc = aml_writeReg(unit, offset, &(mac_pbm[0]), sizeof(UI32_T));
        /* the forwarding port bitmap of QRYP & DRP should follow UIPMCST */
        if (AIR_FORWARD_TYPE_UIPMCST == type)
        {
            aml_writeReg(unit, QRYP, &(mac_pbm[0]), sizeof(UI32_T));
            aml_writeReg(unit, DRP, &(mac_pbm[0]), sizeof(UI32_T));
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_getForwardMode
 * PURPOSE:
 *      Get per port forwarding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  AIR_FORWARD_TYPE_BCST
 *                          AIR_FORWARD_TYPE_UMCST
 *                          AIR_FORWARD_TYPE_UUCST
 *                          AIR_FORWARD_TYPE_UIPMCST
 * OUTPUT:
 *      ptr_action      --  AIR_FORWARD_DROP
 *                          AIR_FORWARD_FLOODING
 *                          AIR_FORWARD_TO_PBM
 *      port_bitmap     --  Forwarding port bitmap
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_getForwardMode(
    const UI32_T             unit,
    const AIR_FORWARD_TYPE_T type,
    AIR_FORWARD_ACTION_T    *ptr_action,
    AIR_PORT_BITMAP_T        port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            offset = 0, getAction;
    AIR_PORT_BITMAP_T getPbm, air_Pbm;

    switch (type)
    {
        case AIR_FORWARD_TYPE_BCST:
            offset = BCF;
            break;
        case AIR_FORWARD_TYPE_UMCST:
            offset = UNMF;
            break;
        case AIR_FORWARD_TYPE_UUCST:
            offset = UNUF;
            break;
        case AIR_FORWARD_TYPE_UIPMCST:
            offset = UNIPMF;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK == rc)
    {
        /* Read data */
        aml_readReg(unit, offset, &(getPbm[0]), sizeof(UI32_T));
        /* Translate port bitmap type */
        HAL_MAC_PBMP_TO_AIR_PBMP(unit, getPbm, air_Pbm);

        if (AIR_PORT_BITMAP_EQUAL(HAL_PORT_BMP_ETH(unit), air_Pbm))
        {
            getAction = AIR_FORWARD_ACTION_FLOODING;
            AIR_PORT_BITMAP_CLEAR(port_bitmap);
        }
        else if (AIR_PORT_BITMAP_EMPTY(air_Pbm))
        {
            getAction = AIR_FORWARD_ACTION_DROP;
            AIR_PORT_BITMAP_CLEAR(port_bitmap);
        }
        else
        {
            getAction = AIR_FORWARD_ACTION_TO_PBM;
            AIR_PORT_BITMAP_COPY(port_bitmap, air_Pbm);
        }

        *ptr_action = getAction;
        DIAG_PRINT(HAL_DBG_INFO, "Type=(%d), Action=(%u), pbm[0]=(0x%x)\n", type, getAction, port_bitmap[0]);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_lockL2FdbResource
 *
 * PURPOSE:
 *      Lock the resource of L2 table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_l2_lockL2FdbResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_L2);

    return HAL_COMMON_LOCK_RESOURCE(PTR_HAL_SCO_L2_HW_TBL_MUTEX(unit), AIR_SEMAPHORE_WAIT_FOREVER);
}

/* FUNCTION NAME: hal_sco_l2_unlockL2FdbResource
 *
 * PURPOSE:
 *      Unlock the resource of L2 table.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_l2_unlockL2FdbResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_L2);

    return HAL_COMMON_FREE_RESOURCE(PTR_HAL_SCO_L2_HW_TBL_MUTEX(unit));
}

#ifdef AIR_EN_L2_SHADOW
/* FUNCTION NAME: hal_sco_l2_traverseMacAddr
 * PURPOSE:
 *      Traverse all L2 unicast address, and handle the address entry by user's callback.
 * INPUT:
 *      unit                --  Device unit number
 *      callback            --  Callback function
 *      ptr_cookie          --  The cookie data as input parameter of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_traverseMacAddr(
    const UI32_T                 unit,
    const AIR_L2_TRAVERSE_FUNC_T callback,
    void                        *ptr_cookie)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_MAC_ENTRY_T  *ptr_sw_entry = NULL;
    AIR_MAC_ENTRY_T   mac_entry;
    CMLIB_RBT_NODE_T *ptr_rbt_node = NULL;

    HAL_CHECK_PTR(callback);

    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE_TASK_RSRC)
    {
        HAL_SCO_L2_SW_LOCK(unit);

        /* get the 1st RBT node */
        ptr_rbt_node = cmlib_rbt_getFirst(HAL_SCO_L2_RBT_HEAD(unit));
        while (ptr_rbt_node)
        {
            /* Find the MAC entry data */
            ptr_sw_entry = (AIR_MAC_ENTRY_T *)ptr_rbt_node->ptr_data;

            /* Handle unicast entry only by user's callback */

            if (HAL_SCO_L2_IS_UC_ADDR(ptr_sw_entry->mac))
            {
                osal_memcpy(&mac_entry, ptr_sw_entry, sizeof(AIR_MAC_ENTRY_T));
                rc = callback(unit, &mac_entry, ptr_cookie);
                if (AIR_E_OP_STOPPED == rc)
                {
                    break;
                }
            }

            /* get the next RBT node */
            ptr_rbt_node = cmlib_rbt_getNext(HAL_SCO_L2_RBT_HEAD(unit), ptr_rbt_node);
        }
        HAL_SCO_L2_SW_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_registerMacAddrNotifyCallback
 * PURPOSE:
 *      Register a callback routine that will be called whenever an entry is
 *      inserted into or deleted from L2 address table.
 * INPUT:
 *      unit                --  Device unit number
 *      notify_func         --  Callback function
 *      ptr_cookie          --  The cookie data as input parameter of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_TABLE_FULL    --  Table is full.
 *      AIR_E_ENTRY_EXISTS  --  Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_registerMacAddrNotifyCallback(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    HAL_SCO_L2_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                       idx = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE_TASK_RSRC)
    {
        HAL_SCO_L2_NOTIFY_LOCK(unit);
        for (idx = 0; idx < HAL_SCO_L2_NOTIFY_HANDLER_CNT; idx++)
        {
            ptr_handler = PTR_HAL_SCO_L2_NOTIFY_HANDLER(unit, idx);
            if ((notify_func == ptr_handler->notify_func) && (ptr_cookie == ptr_handler->ptr_cookie))
            {
                rc = AIR_E_ENTRY_EXISTS;
                break;
            }
            else if ((NULL == ptr_handler->notify_func) && (NULL == ptr_handler->ptr_cookie))
            {
                ptr_handler->notify_func = notify_func;
                ptr_handler->ptr_cookie = ptr_cookie;
                break;
            }
        }
        HAL_SCO_L2_NOTIFY_UNLOCK(unit);
        if (HAL_SCO_L2_NOTIFY_HANDLER_CNT == idx)
        {
            rc = AIR_E_TABLE_FULL;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_deregisterMacAddrNotifyCallback
 * PURPOSE:
 *      Deregister a callback routine that will be called whenever an entry is
 *      inserted into or deleted from L2 address table.
 * INPUT:
 *      unit                    --  Device unit number
 *      notify_func             --  Callback function
 *      ptr_cookie              --  The cookie data as input parameter of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER     --  Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND   --  Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_deregisterMacAddrNotifyCallback(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    HAL_SCO_L2_NOTIFY_HANDLER_T *ptr_handler = NULL;
    UI32_T                       idx = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE_TASK_RSRC)
    {
        HAL_SCO_L2_NOTIFY_LOCK(unit);
        for (idx = 0; idx < HAL_SCO_L2_NOTIFY_HANDLER_CNT; idx++)
        {
            ptr_handler = PTR_HAL_SCO_L2_NOTIFY_HANDLER(unit, idx);
            if ((notify_func == ptr_handler->notify_func) && (ptr_cookie == ptr_handler->ptr_cookie))
            {
                ptr_handler->notify_func = NULL;
                ptr_handler->ptr_cookie = NULL;
                break;
            }
        }
        HAL_SCO_L2_NOTIFY_UNLOCK(unit);
        if (HAL_SCO_L2_NOTIFY_HANDLER_CNT == idx)
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_setShadowCfg
 * PURPOSE:
 *      Set the configurations of L2 shadow table.
 * INPUT:
 *      unit                     -- Device unit number
 *      polling_en               -- The state of polling mode
 *      interval                 -- The interval time between each maximum polling number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER      -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_setShadowCfg(
    const UI32_T unit,
    const BOOL_T polling_en,
    const UI32_T interval)
{
    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE_TASK_RSRC)
    {
        PTR_HAL_SCO_L2_POLLING_CB(unit)->state = polling_en;
        PTR_HAL_SCO_L2_POLLING_CB(unit)->interval = interval;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_l2_getShadowCfg
 * PURPOSE:
 *      Get the configurations of L2 shadow table.
 * INPUT:
 *      unit                     -- Device unit number
 * OUTPUT:
 *      ptr_polling_en           -- Pointer to the state of polling mode
 *      ptr_interval             -- Pointer to the interval time between each maximum
 *                                  polling number
 * RETURN:
 *      AIR_E_OK                 -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER      -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_l2_getShadowCfg(
    const UI32_T unit,
    BOOL_T      *ptr_polling_en,
    UI32_T      *ptr_interval)
{
    if (HAL_MODULE_INITED(unit, AIR_MODULE_L2) & HAL_INIT_STAGE_TASK_RSRC)
    {
        *ptr_polling_en = PTR_HAL_SCO_L2_POLLING_CB(unit)->state;
        *ptr_interval = PTR_HAL_SCO_L2_POLLING_CB(unit)->interval;
    }

    return AIR_E_OK;
}

#endif /* End of AIR_EN_L2_SHADOW */

/* FUNCTION NAME: hal_sco_l2_getCapacity
 * PURPOSE:
 *      Get the l2 resource capacity
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
 */
AIR_ERROR_NO_T
hal_sco_l2_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_L2_FDB == type)
    {
        *ptr_size = HAL_SCO_L2_MAX_ENTRY_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_l2_getUsage
 * PURPOSE:
 *      Get the l2 resource usage
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
hal_sco_l2_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_agsc = 0;

    if (AIR_SWC_RSRC_L2_FDB == type)
    {
        aml_readReg(unit, REG_AGSC, &reg_agsc, sizeof(reg_agsc));
        *ptr_cnt = BITS_OFF_R(reg_agsc, REG_AGSC_GLB_SA_LRN_CNT_OFFT, REG_AGSC_GLB_SA_LRN_CNT_LENGTH);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}
