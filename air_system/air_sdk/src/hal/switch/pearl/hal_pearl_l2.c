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

/* FILE NAME:  hal_pearl_l2.c
 * PURPOSE:
 *  Implement L2 module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_l2.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/pearl/hal_pearl_reg.h>
#include <hal/switch/pearl/hal_pearl_stp.h>
#include <hal/switch/pearl/hal_pearl_vlan.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_L2_DELAY_US          (1000)
#define HAL_PEARL_L2_WDOG_KICK_NUM     (1000)
#define HAL_PEARL_L2_FORWARD_VALUE     (0xFFFFFFFF)
#define HAL_PEARL_L2_AGING_MS_CONSTANT (1024)
#define HAL_PEARL_L2_AGING_1000MS      (1000)

#define HAL_PEARL_L2_AAC_DEFAULT_VALUE     (0x00125000)
#define HAL_PEARL_L2_ADDRTBL_DEFAULT_VALUE (0)
#define HAL_PEARL_L2_AGDIS_DEFAULT_VALUE   (0)
#define HAL_PEARL_L2_LOGPORT_DEFAULT_VALUE (0xFFFFFFFF)

typedef enum
{
    _HAL_PEARL_L2_MAC_TB_TY_MAC,
    _HAL_PEARL_L2_MAC_TB_TY_DIP,
    _HAL_PEARL_L2_MAC_TB_TY_DIP_SIP,
    _HAL_PEARL_L2_MAC_TB_TY_SPEC_ENTRY,
    _HAL_PEARL_L2_MAC_TB_TY_LAST
} _HAL_PEARL_L2_MAC_TB_TY_T;

typedef enum
{
    _HAL_PEARL_L2_MAC_MAT_MAC,
    _HAL_PEARL_L2_MAC_MAT_DYNAMIC_MAC,
    _HAL_PEARL_L2_MAC_MAT_STATIC_MAC,
    _HAL_PEARL_L2_MAC_MAT_MAC_BY_VID,
    _HAL_PEARL_L2_MAC_MAT_MAC_BY_FID,
    _HAL_PEARL_L2_MAC_MAT_MAC_BY_PORT,
    _HAL_PEARL_L2_MAC_MAT_MAC_BY_LAST
} _HAL_PEARL_L2_MAC_MAT_T;

/* L2 MAC table multi-searching */
typedef enum
{
    _HAL_PEARL_L2_MAC_MS_START, /* Start search */
    _HAL_PEARL_L2_MAC_MS_NEXT,  /* Next search */
    _HAL_PEARL_L2_MAC_MS_LAST
} _HAL_PEARL_L2_MAC_MS_T;

typedef enum
{
    _HAL_PEARL_L2_FWD_CTRL_DEFAULT = 0x0,
    _HAL_PEARL_L2_FWD_CTRL_CPU_EXCLUDE = 0x4,
    _HAL_PEARL_L2_FWD_CTRL_CPU_INCLUDE = 0x5,
    _HAL_PEARL_L2_FWD_CTRL_CPU_ONLY = 0x6,
    _HAL_PEARL_L2_FWD_CTRL_DROP = 0x7,
    _HAL_PEARL_L2_FWD_CTRL_LAST
} _HAL_PEARL_L2_FWD_CTRL_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_PEARL_L2_AGING_TIME(__cnt__, __unit__)                                                   \
    (((__cnt__) + 1) * ((__unit__) + 1) * HAL_PEARL_L2_AGING_MS_CONSTANT / HAL_PEARL_L2_AGING_1000MS)

#define _HAL_PEARL_L2_AGING_TIME_2_CNT(__time__, __cnt__, __unit__)                               \
    do                                                                                            \
    {                                                                                             \
        UI32_T _value_ = (__time__) * HAL_PEARL_L2_AGING_1000MS / HAL_PEARL_L2_AGING_MS_CONSTANT; \
        (__unit__) = (_value_ / BIT(PEARL_AAC_AGE_CNT_LENGTH) + 1);                               \
        (__cnt__) = (_value_ / (__unit__) + 1);                                                   \
    } while (0)

#define HAL_PEARL_L2_TRANS_IDX_TO_ADDR(__i__, __a__, __b__) \
    do                                                      \
    {                                                       \
        (__a__) = (__i__) / HAL_PEARL_L2_MAC_SET_NUM;       \
        (__b__) = (__i__) % HAL_PEARL_L2_MAC_SET_NUM;       \
    } while (0);

#define HAL_PEARL_L2_MAC_ADDR_DIFF(__ptr_ma_1__, __ptr_ma_2__) (osal_memcmp(__ptr_ma_1__, __ptr_ma_2__, 6))

#define HAL_PEARL_L2_MAC_ENTRY_DIFF(__ptr_ety_1__, __ptr_ety_2__)                                                  \
    (HAL_PEARL_L2_MAC_ADDR_DIFF((__ptr_ety_1__)->mac, (__ptr_ety_2__)->mac) ||                                     \
     ((__ptr_ety_1__)->cvid != (__ptr_ety_2__)->cvid) || ((__ptr_ety_1__)->fid != (__ptr_ety_2__)->fid) ||         \
     ((__ptr_ety_1__)->flags != (__ptr_ety_2__)->flags) || ((__ptr_ety_1__)->sa_fwd != (__ptr_ety_2__)->sa_fwd) || \
     !CMLIB_BITMAP_EQUAL((__ptr_ety_1__)->port_bitmap, (__ptr_ety_2__)->port_bitmap, AIR_PORT_BITMAP_SIZE))

#define PTR_HAL_PEARL_L2_HW_TBL_MUTEX(__unit__) (&(_l2_fdb_cb[__unit__].hw_tbl_mutex))

#define HAL_PEARL_L2_PRINT_MAC_ENTRY(__name__, __entry__)                                                          \
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

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_L2, "hal_pearl_l2.c");

/* STATIC VARIABLE DECLARATIONS
 */
static HAL_PEARL_L2_FDB_CB_T _l2_fdb_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
static BOOL_T                _search_end = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */

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
    return HAL_PEARL_L2_MAC_ADDR_DIFF(ptr_mac_entry->mac, all_zero) ? TRUE : FALSE;
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

    /* Fill PEARL_ATA1 */
    u32dat = 0;
    for (i = 0; i < 4; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0, 7))) << ((3 - i) * 8);
    }
    aml_writeReg(unit, PEARL_ATA1, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_PEARL_L2_DELAY_US);

    /* Fill PEARL_ATA2 */
    u32dat = 0;
    for (i = 4; i < 6; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0, 7))) << ((7 - i) * 8);
    }
    if (!(ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC))
    {
        /* type is dynamic */
        u32dat |= BITS_OFF_L(1UL, PEARL_ATA2_MAC_LIFETIME_OFFSET, PEARL_ATA2_MAC_LIFETIME_LENGTH);
        /* set aging counter as system aging conuter */
        u32dat |= BITS_OFF_L(ptr_mac_entry->timer, PEARL_ATA2_MAC_AGETIME_OFFSET, PEARL_ATA2_MAC_AGETIME_LENGTH);
    }
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_UNAUTH)
    {
        u32dat |= BITS_OFF_L(1UL, PEARL_ATA2_MAC_UNAUTH_OFFSET, PEARL_ATA2_MAC_UNAUTH_LENGTH);
    }

    aml_writeReg(unit, PEARL_ATA2, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_PEARL_L2_DELAY_US);
}

/* FUNCTION NAME: _fill_MAC_ATWD
 * PURPOSE:
 *      Fill register PEARL_ATWD for MAC Address table.
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
    /* Fill PEARL_ATWD */
    /* set valid bit */
    if (TRUE == valid)
    {
        u32dat |= BITS_OFF_L(1UL, PEARL_ATWD_MAC_LIVE_OFFSET, PEARL_ATWD_MAC_LIVE_LENGTH);
    }

    /* set IVL */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        u32dat |= BITS_OFF_L(1UL, PEARL_ATWD_MAC_IVL_OFFSET, PEARL_ATWD_MAC_IVL_LENGTH);
    }
    /* set VID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->cvid, PEARL_ATWD_MAC_VID_OFFSET, PEARL_ATWD_MAC_VID_LENGTH);
    /* set FID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->fid, PEARL_ATWD_MAC_FID_OFFSET, PEARL_ATWD_MAC_FID_LENGTH);
    /* set leaky VLAN */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
    {
        u32dat |= BITS_OFF_L(1UL, PEARL_ATWD_MAC_LEAK_OFFSET, PEARL_ATWD_MAC_LEAK_LENGTH);
    }

    /* Set forwarding control */
    switch (ptr_mac_entry->sa_fwd)
    {
        case AIR_L2_FWD_CTRL_DEFAULT:
            fwd_val = _HAL_PEARL_L2_FWD_CTRL_DEFAULT;
            break;
        case AIR_L2_FWD_CTRL_CPU_INCLUDE:
            fwd_val = _HAL_PEARL_L2_FWD_CTRL_CPU_INCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_EXCLUDE:
            fwd_val = _HAL_PEARL_L2_FWD_CTRL_CPU_EXCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_ONLY:
            fwd_val = _HAL_PEARL_L2_FWD_CTRL_CPU_ONLY;
            break;
        case AIR_L2_FWD_CTRL_DROP:
            fwd_val = _HAL_PEARL_L2_FWD_CTRL_DROP;
            break;
        default:
            break;
    }
    u32dat |= BITS_OFF_L(fwd_val, PEARL_ATWD_MAC_FWD_OFFSET, PEARL_ATWD_MAC_FWD_LENGTH);
    aml_writeReg(unit, PEARL_ATWD, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_PEARL_L2_DELAY_US);

    /* Fill PEARL_ATWD2 */
    u32dat = BITS_OFF_L(ptr_mac_entry->port_bitmap[0], PEARL_ATWD2_MAC_PORT_OFFSET, PEARL_ATWD2_MAC_PORT_LENGTH);
    aml_writeReg(unit, PEARL_ATWD2, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_PEARL_L2_DELAY_US);
}

/* FUNCTION NAME: _fill_MAC_ATRDS
 * PURPOSE:
 *      Fill register PEARL_ATRDS for select bank after PEARL_ATC search L2 table.
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

    /* Fill PEARL_ATRDS */
    u32dat = BITS_OFF_L(bank, PEARL_ATRD0_MAC_SEL_OFFSET, PEARL_ATRD0_MAC_SEL_LENGTH);
    aml_writeReg(unit, PEARL_ATRDS, &u32dat, sizeof(u32dat));
    osal_delayUs(HAL_PEARL_L2_DELAY_US);
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

    /* Read PEARL_ATRD0 */
    aml_readReg(unit, PEARL_ATRD0, &u32dat, sizeof(u32dat));
    live = BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_LIVE_OFFSET, PEARL_ATRD0_MAC_LIVE_LENGTH);
    type = BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_TYPE_OFFSET, PEARL_ATRD0_MAC_TYPE_LENGTH);
    if (FALSE == live)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    if (_HAL_PEARL_L2_MAC_TB_TY_MAC != type)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    /* Clear table */
    osal_memset(ptr_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));

    ptr_mac_entry->cvid = (UI16_T)BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_VID_OFFSET, PEARL_ATRD0_MAC_VID_LENGTH);
    ptr_mac_entry->fid = (UI16_T)BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_FID_OFFSET, PEARL_ATRD0_MAC_FID_LENGTH);
    if (!!BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_LIFETIME_OFFSET, PEARL_ATRD0_MAC_LIFETIME_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;
    }
    if (!!BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_IVL_OFFSET, PEARL_ATRD0_MAC_IVL_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    if (!!BITS_OFF_R(u32dat, PEARL_ATRD1_MAC_UNAUTH_OFFSET, PEARL_ATRD1_MAC_UNAUTH_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_UNAUTH;
    }
    if (!!BITS_OFF_R(u32dat, PEARL_ATRD0_MAC_LEAK_OFFSET, PEARL_ATRD0_MAC_LEAK_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
    }

    /* Get the L2 MAC aging unit */
    aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
    age_unit = BITS_OFF_R(u32dat, PEARL_AAC_AGE_UNIT_OFFSET, PEARL_AAC_AGE_UNIT_LENGTH);

    /* Read PEARL_ATRD1 */
    aml_readReg(unit, PEARL_ATRD1, &u32dat, sizeof(u32dat));
    for (i = 4; i < 6; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (7 - i) * 8, 8);
    }
    /* Aging time */
    age_cnt = BITS_OFF_R(u32dat, PEARL_ATRD1_MAC_AGETIME_OFFSET, PEARL_ATRD1_MAC_AGETIME_LENGTH);
    ptr_mac_entry->timer = _HAL_PEARL_L2_AGING_TIME(age_cnt, age_unit);
    /* SA forwarding */
    sa_fwd = BITS_OFF_R(u32dat, PEARL_ATRD1_MAC_FWD_OFFSET, PEARL_ATRD1_MAC_FWD_LENGTH);
    switch (sa_fwd)
    {
        case _HAL_PEARL_L2_FWD_CTRL_DEFAULT:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
        case _HAL_PEARL_L2_FWD_CTRL_CPU_INCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_INCLUDE;
            break;
        case _HAL_PEARL_L2_FWD_CTRL_CPU_EXCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_EXCLUDE;
            break;
        case _HAL_PEARL_L2_FWD_CTRL_CPU_ONLY:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_ONLY;
            break;
        case _HAL_PEARL_L2_FWD_CTRL_DROP:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DROP;
            break;
        default:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
    }

    /* Read PEARL_ATRD2 */
    aml_readReg(unit, PEARL_ATRD2, &u32dat, sizeof(u32dat));
    for (i = 0; i < 4; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (3 - i) * 8, 8);
    }

    /* Read PEARL_ATRD3 */
    aml_readReg(unit, PEARL_ATRD3, &u32dat, sizeof(u32dat));
    ptr_mac_entry->port_bitmap[0] = BITS_OFF_R(u32dat, PEARL_ATRD3_MAC_PORT_OFFSET, PEARL_ATRD3_MAC_PORT_LENGTH);

    return AIR_E_OK;
}

/* FUNCTION NAME: _checkL2Busy
 * PURPOSE:
 *      Check BUSY bit of PEARL_ATC
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
    for (i = 0; i < HAL_PEARL_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, PEARL_ATC, &reg_atc, sizeof(reg_atc));
        if (!BITS_OFF_R(reg_atc, PEARL_ATC_BUSY_OFFSET, PEARL_ATC_BUSY_LENGTH))
        {
            break;
        }
        osal_delayUs(HAL_PEARL_L2_DELAY_US);
    }
    if (i >= HAL_PEARL_L2_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _checkL2EntryHit
 * PURPOSE:
 *      Check entry hit of PEARL_ATC
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
    aml_readReg(unit, PEARL_ATC, &reg_atc, sizeof(reg_atc));
    return BITS_OFF_R(reg_atc, PEARL_ATC_ENTRY_HIT_OFFSET, PEARL_ATC_ENTRY_HIT_LENGTH);
}

/* FUNCTION NAME: _checkL2ReadSingleHit
 * PURPOSE:
 *      Check reading single entry hit of PEARL_ATC
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
    aml_readReg(unit, PEARL_ATC, &reg_atc, sizeof(reg_atc));
    return BITS_OFF_R(reg_atc, PEARL_ATC_SINGLE_HIT_OFFSET, PEARL_ATC_SINGLE_HIT_LENGTH);
}

/* FUNCTION NAME: _searchMacEntry
 * PURPOSE:
 *      Search MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ms              --  _HAL_PEARL_L2_MAC_MS_START:           Start search command
 *                          _HAL_PEARL_L2_MAC_MS_NEXT:            Next search command
 *      multi_target    --  _HAL_PEARL_L2_MAC_MAT_MAC:            MAC address entries
 *                          _HAL_PEARL_L2_MAC_MAT_DYNAMIC_MAC:    Dynamic MAC address entries
 *                          _HAL_PEARL_L2_MAC_MAT_STATIC_MAC:     Static MAC address entries
 *                          _HAL_PEARL_L2_MAC_MAT_MAC_BY_VID:     MAC address entries with specific CVID
 *                          _HAL_PEARL_L2_MAC_MAT_MAC_BY_FID:     MAC address entries with specific FID
 *                          _HAL_PEARL_L2_MAC_MAT_MAC_BY_PORT:    MAC address entries with specific port
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
    const UI32_T                  unit,
    const _HAL_PEARL_L2_MAC_MS_T  ms,
    const _HAL_PEARL_L2_MAC_MAT_T multi_target,
    UI32_T                       *ptr_addr,
    UI32_T                       *ptr_bank)
{
    UI32_T u32dat;

    u32dat = (PEARL_ATC_SAT_MAC | PEARL_ATC_START_BUSY);
    if (_HAL_PEARL_L2_MAC_MS_START == ms)
    {
        /* Start search 1st valid entry */
        u32dat |= PEARL_ATC_CMD_SEARCH;
    }
    else if (_HAL_PEARL_L2_MAC_MS_NEXT == ms)
    {
        /* Search next valid entry */
        u32dat |= PEARL_ATC_CMD_SEARCH_NEXT;
    }
    else
    {
        /* Unknown commnad */
        return AIR_E_BAD_PARAMETER;
    }

    switch (multi_target)
    {
        case _HAL_PEARL_L2_MAC_MAT_MAC:
            u32dat |= PEARL_ATC_MAT_MAC;
            break;
        case _HAL_PEARL_L2_MAC_MAT_DYNAMIC_MAC:
            u32dat |= PEARL_ATC_MAT_DYNAMIC_MAC;
            break;
        case _HAL_PEARL_L2_MAC_MAT_STATIC_MAC:
            u32dat |= PEARL_ATC_MAT_STATIC_MAC;
            break;
        case _HAL_PEARL_L2_MAC_MAT_MAC_BY_VID:
            u32dat |= PEARL_ATC_MAT_MAC_BY_VID;
            break;
        case _HAL_PEARL_L2_MAC_MAT_MAC_BY_FID:
            u32dat |= PEARL_ATC_MAT_MAC_BY_FID;
            break;
        case _HAL_PEARL_L2_MAC_MAT_MAC_BY_PORT:
            u32dat |= PEARL_ATC_MAT_MAC_BY_PORT;
            break;
        default:
            /* Unknown searching mode */
            return AIR_E_BAD_PARAMETER;
    }
    aml_writeReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    aml_readReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    /* Get address */
    (*ptr_addr) = BITS_OFF_R(u32dat, PEARL_ATC_ADDR_OFFSET, PEARL_ATC_ADDR_LENGTH);
    /* Get banks */
    (*ptr_bank) = BITS_OFF_R(u32dat, PEARL_ATC_ENTRY_HIT_OFFSET, PEARL_ATC_ENTRY_HIT_LENGTH);
    if ((HAL_PEARL_L2_MAX_ADDR_NUM - 1) == (*ptr_addr))
    {
        _search_end = TRUE;
    }
    else
    {
        _search_end = FALSE;
    }

    return AIR_E_OK;
}

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
    u32dat = (PEARL_ATC_CMD_READ | PEARL_ATC_SAT_MAC | PEARL_ATC_START_BUSY);
    aml_writeReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }
    if (!_checkL2ReadSingleHit(unit))
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    aml_readReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    (*ptr_banks) = BITS_OFF_R(u32dat, PEARL_ATC_ENTRY_HIT_OFFSET, PEARL_ATC_ENTRY_HIT_LENGTH);

    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_pearl_l2_initRsrc
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
_hal_pearl_l2_initRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;

    HAL_PEARL_L2_FDB_CB_T *ptr_cb = NULL;

    HAL_CHECK_UNIT(unit);

    ptr_cb = &(_l2_fdb_cb[unit]);
    osal_memset(ptr_cb, 0, sizeof(HAL_PEARL_L2_FDB_CB_T));

    /* Create mutex lock */
    rc = osal_createSemaphore("L2_HW_TABLE", AIR_SEMAPHORE_BINARY, PTR_HAL_PEARL_L2_HW_TBL_MUTEX(unit),
                              air_module_getModuleName(AIR_MODULE_L2));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Create semaphore failed, rc=(%d)\n", rc);
    }

    return rc;
}

/* FUNCTION NAME: _hal_pearl_l2_deinitRsrc
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
_hal_pearl_l2_deinitRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    HAL_PEARL_L2_FDB_CB_T *ptr_cb = NULL;

    HAL_CHECK_UNIT(unit);

    ptr_cb = &(_l2_fdb_cb[unit]);

    /* Destory mutex lock */
    rc = osal_destroySemaphore(PTR_HAL_PEARL_L2_HW_TBL_MUTEX(unit));

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Destroy semaphore failed, rc=(%d)\n", rc);
    }

    /* Reset control block */
    osal_memset(ptr_cb, 0, sizeof(HAL_PEARL_L2_FDB_CB_T));
    return rc;
}

/* FUNCTION NAME: _hal_pearl_l2_initModule
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
_hal_pearl_l2_initModule(
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
        hal_pearl_l2_setForwardMode(unit, type, AIR_FORWARD_ACTION_TO_PBM, pbm);
    }
    /* Initialize L2 HW learning behavior when L2 FDB full */
    osal_memset(&cfg_l2fdb_full, 0, sizeof(AIR_CFG_VALUE_T));
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_L2_FDB_FULL_BEHAVIOR, &cfg_l2fdb_full);
    HAL_CHECK_ERROR(rc);
    if (HAL_PEARL_L2_FDB_FULL_HW_REPLACE_OLDEST_DYNAMIC_ENTRY == cfg_l2fdb_full.value)
    {
        aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
        u32dat |= BITS_OFF_L(1UL, PEARL_AGC_MAC_OLD_RPLC_HW_OFFT, PEARL_AGC_MAC_OLD_RPLC_HW_LENG);
        aml_writeReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
    }

    return rc;
}

/* FUNCTION NAME: _hal_pearl_l2_deinitModule
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
_hal_pearl_l2_deinitModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat;

    hal_pearl_l2_clearMacAddr(unit);

    u32dat = HAL_PEARL_L2_AAC_DEFAULT_VALUE;
    aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
    u32dat = HAL_PEARL_L2_ADDRTBL_DEFAULT_VALUE;
    aml_readReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_ATA1, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_ATA2, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_ATWD, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_ATWD2, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_ATRDS, &u32dat, sizeof(u32dat));
    u32dat = HAL_PEARL_L2_AGDIS_DEFAULT_VALUE;
    aml_readReg(unit, PEARL_AGDIS, &u32dat, sizeof(u32dat));
    u32dat = HAL_PEARL_L2_LOGPORT_DEFAULT_VALUE;
    aml_readReg(unit, PEARL_LOGMACPORT, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_LOGAGEPORT, &u32dat, sizeof(u32dat));
    u32dat = HAL_PEARL_L2_FORWARD_VALUE;
    aml_readReg(unit, PEARL_BCF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_UNMF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_UNUF, &u32dat, sizeof(u32dat));
    aml_readReg(unit, PEARL_UNIPMF, &u32dat, sizeof(u32dat));

    return rc;
}

/* FUNCTION NAME: _hal_pearl_l2_initThread
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
_hal_pearl_l2_initThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}

/* FUNCTION NAME: _hal_pearl_l2_deinitThread
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
_hal_pearl_l2_deinitThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}

/* FUNCTION NAME:  _hal_pearl_l2_searchMacAddr
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
 *                          _HAL_PEARL_L2_MAC_MS_T
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
_hal_pearl_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry,
    _HAL_PEARL_L2_MAC_MS_T         search_cmd)
{
    AIR_ERROR_NO_T          rc = AIR_E_OK;
    UI32_T                  i = 0;
    AIR_MAC_ENTRY_T         hw_mac_entry;
    UI32_T                  addr = 0;
    UI32_T                  banks = 0;
    UI32_T                  mac_port = 0;
    _HAL_PEARL_L2_MAC_MAT_T mat;

    osal_memset(&hw_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));
    switch (type)
    {
        case AIR_L2_MAC_SEARCH_TYPE_VID:
            mat = _HAL_PEARL_L2_MAC_MAT_MAC_BY_VID;
            hw_mac_entry.cvid = (UI16_T)value;
            break;
        case AIR_L2_MAC_SEARCH_TYPE_FID:
            mat = _HAL_PEARL_L2_MAC_MAT_MAC_BY_FID;
            hw_mac_entry.fid = (UI16_T)value;
            break;
        case AIR_L2_MAC_SEARCH_TYPE_PORT:
            mat = _HAL_PEARL_L2_MAC_MAT_MAC_BY_PORT;
            HAL_AIR_PORT_TO_MAC_PORT(unit, value, mac_port);
            AIR_PORT_ADD(hw_mac_entry.port_bitmap, mac_port);
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }
    (*ptr_count) = 0;

    HAL_PEARL_L2_FDB_LOCK(unit);
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

            for (i = 0; i < HAL_PEARL_L2_MAC_SET_NUM; i++)
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

    HAL_PEARL_L2_FDB_UNLOCK(unit);
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_pearl_l2_init
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
hal_pearl_l2_init(
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
            rc = _hal_pearl_l2_initRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = _hal_pearl_l2_initModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = _hal_pearl_l2_initThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_L2) |= HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_deinit
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
hal_pearl_l2_deinit(
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
            rc = _hal_pearl_l2_deinitRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = _hal_pearl_l2_deinitModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = _hal_pearl_l2_deinitThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_L2) &= ~HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_addMacAddr
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
hal_pearl_l2_addMacAddr(
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
    HAL_PEARL_L2_FDB_LOCK(unit);
    /* set aging counter as system aging conuter */
    aml_readReg(unit, PEARL_AAC, &reg_aac, sizeof(reg_aac));
    if (ptr_mac_entry->timer != AIR_L2_MAC_DEF_AGE_OUT_TIME)
    {
        _HAL_PEARL_L2_AGING_TIME_2_CNT(ptr_mac_entry->timer, age_cnt, age_unit);
        sys_unit = BITS_OFF_R(reg_aac, PEARL_AAC_AGE_UNIT_OFFSET, PEARL_AAC_AGE_UNIT_LENGTH) + 1;
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
        set_mac_entry.timer = BITS_OFF_R(reg_aac, PEARL_AAC_AGE_CNT_OFFSET, PEARL_AAC_AGE_CNT_LENGTH);
    }

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &set_mac_entry);
    _fill_MAC_ATWD(unit, &set_mac_entry, TRUE);

    /* Write data by PEARL_ATC */
    u32dat = (PEARL_ATC_SAT_MAC | PEARL_ATC_CMD_WRITE | PEARL_ATC_START_BUSY);
    aml_writeReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_PEARL_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }
    if (!_checkL2EntryHit(unit))
    {
        HAL_PEARL_L2_FDB_UNLOCK(unit);
        return AIR_E_TABLE_FULL;
    }
    HAL_PEARL_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_delMacAddr
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
hal_pearl_l2_delMacAddr(
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

    HAL_PEARL_L2_FDB_LOCK(unit);

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &del_mac_entry);
    _fill_MAC_ATWD(unit, &del_mac_entry, FALSE);

    /* Write data by PEARL_ATC to delete entry */
    u32dat = (PEARL_ATC_SAT_MAC | PEARL_ATC_CMD_WRITE | PEARL_ATC_START_BUSY);
    aml_writeReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_PEARL_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    HAL_PEARL_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME:  hal_pearl_l2_getMacAddr
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
hal_pearl_l2_getMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_MAC_ENTRY_T hw_mac_entry;
    UI32_T          bank = 0, banks = 0;
    UI32_T          u32dat = 0;
    UI32_T          i = 0;
    UI32_T          addr = 0;

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
        HAL_PEARL_L2_FDB_LOCK(unit);
        rc = _getSpecMacEntryByMac(unit, ptr_mac_entry, &banks);
        if (AIR_E_OK != rc)
        {
            HAL_PEARL_L2_FDB_UNLOCK(unit);
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            return rc;
        }
        /* Select bank */
        aml_readReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
        banks = BITS_OFF_R(u32dat, PEARL_ATC_ENTRY_HIT_OFFSET, PEARL_ATC_ENTRY_HIT_LENGTH);
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
        HAL_PEARL_L2_FDB_UNLOCK(unit);
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
        /* MAC address is empty, means to search the 1st MAC entry */
        HAL_PEARL_L2_FDB_LOCK(unit);
        rc = _searchMacEntry(unit, _HAL_PEARL_L2_MAC_MS_START, _HAL_PEARL_L2_MAC_MAT_MAC, &addr, &banks);

        switch (rc)
        {
            case AIR_E_OK:
                /* Searching bank and read data */
                DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
                if (0 == banks)
                {
                    HAL_PEARL_L2_FDB_UNLOCK(unit);
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < HAL_PEARL_L2_MAC_SET_NUM; i++)
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
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_OK;
            case AIR_E_TIMEOUT:
                /* Searching over time */
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_TIMEOUT;
            default:
                DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
        }
    }
}

/* FUNCTION NAME: hal_pearl_l2_getNextMacAddr
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
hal_pearl_l2_getNextMacAddr(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
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

    HAL_PEARL_L2_FDB_LOCK(unit);
    /* Check MAC Address field of input data */
    if (_isMacEntryValid(ptr_mac_entry))
    {
        /* MAC address isn't empty, means to search the next entries of input MAC Address */
        /* Search the target MAC entry */
        _fill_MAC_ATA(unit, ptr_mac_entry);
        rc = _searchMacEntry(unit, _HAL_PEARL_L2_MAC_MS_START, _HAL_PEARL_L2_MAC_MAT_MAC, &addr, &banks);
        while (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
            if (0 == banks)
            {
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < HAL_PEARL_L2_MAC_SET_NUM; i++)
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
                    if (!HAL_PEARL_L2_MAC_ADDR_DIFF(ptr_mac_entry->mac, hw_mac_entry.mac))
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
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            else
            {
                rc = _searchMacEntry(unit, _HAL_PEARL_L2_MAC_MS_NEXT, _HAL_PEARL_L2_MAC_MAT_MAC, &addr, &banks);
            }
        }

        if (FALSE == found_target)
        {
            /* Entry not bank */
            HAL_PEARL_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
        else
        {
            /* Found the target MAC entry, and try to search the next address */
            rc = _searchMacEntry(unit, _HAL_PEARL_L2_MAC_MS_NEXT, _HAL_PEARL_L2_MAC_MAT_MAC, &addr, &banks);
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
                if (0 == banks)
                {
                    HAL_PEARL_L2_FDB_UNLOCK(unit);
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < HAL_PEARL_L2_MAC_SET_NUM; i++)
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
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_OK;
            }
            else
            {
                DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        /* MAC address is empty, means to search next entry */
        rc = _searchMacEntry(unit, _HAL_PEARL_L2_MAC_MS_NEXT, _HAL_PEARL_L2_MAC_MAT_MAC, &addr, &banks);
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "banks=(%d)\n", banks);
            if (0 == banks)
            {
                HAL_PEARL_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < HAL_PEARL_L2_MAC_SET_NUM; i++)
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
            HAL_PEARL_L2_FDB_UNLOCK(unit);
            return AIR_E_OK;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_INFO, "rc=(%d)\n", rc);
            HAL_PEARL_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }
}

/* FUNCTION NAME:  hal_pearl_l2_searchMacAddr
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
 *      hal_pearl_l2_searchNextMacAddr(). It should not be used interchangeably
 *      with hal_pearl_l2_getMacAddr()/hal_pearl_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_pearl_l2_searchMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _hal_pearl_l2_searchMacAddr(unit, type, value, ptr_count, ptr_mac_entry, _HAL_PEARL_L2_MAC_MS_START);
    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_searchNextMacAddr
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
 *      hal_pearl_l2_searchMacAddr(). It should not be used interchangeably
 *      with hal_pearl_l2_getMacAddr()/hal_pearl_air_l2_getNextMacAddr() before
 *      the search is finished, otherwise the results obtained will noe meet
 *      expectations.
 */
AIR_ERROR_NO_T
hal_pearl_l2_searchNextMacAddr(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry)

{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    rc = _hal_pearl_l2_searchMacAddr(unit, type, value, ptr_count, ptr_mac_entry, _HAL_PEARL_L2_MAC_MS_NEXT);
    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_clearMacAddr
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
hal_pearl_l2_clearMacAddr(
    const UI32_T unit)
{
    UI32_T u32dat;

    /* Write data by PEARL_ATC to clear all MAC address entries */
    u32dat = (PEARL_ATC_MAT_MAC | PEARL_ATC_CMD_CLEAN | PEARL_ATC_START_BUSY);
    HAL_PEARL_L2_FDB_LOCK(unit);
    aml_writeReg(unit, PEARL_ATC, &u32dat, sizeof(u32dat));
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        HAL_PEARL_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    HAL_PEARL_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_flushMacAddr
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
hal_pearl_l2_flushMacAddr(
    const UI32_T                  unit,
    const AIR_L2_MAC_FLUSH_TYPE_T type,
    const UI32_T                  value)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_atc, reg_atwd, reg_ata2;
    UI32_T         mac_port = 0;

    HAL_PEARL_L2_FDB_LOCK(unit);

    reg_atc = (PEARL_ATC_START_BUSY | PEARL_ATC_CMD_CLEAN);
    /* Configure to vid/fid/port for ATWD */
    switch (type)
    {
        case AIR_L2_MAC_FLUSH_TYPE_VID:
            reg_atc |= PEARL_ATC_MAT_MAC_BY_LIFETIME_VID;
            reg_atwd = BITS_OFF_L(value, PEARL_ATWD_MAC_VID_OFFSET, PEARL_ATWD_MAC_VID_LENGTH);
            aml_writeReg(unit, PEARL_ATWD, &reg_atwd, sizeof(reg_atwd));
            break;
        case AIR_L2_MAC_FLUSH_TYPE_FID:
            reg_atc |= PEARL_ATC_MAT_MAC_BY_LIFETIME_FID;
            reg_atwd = BITS_OFF_L(value, PEARL_ATWD_MAC_FID_OFFSET, PEARL_ATWD_MAC_FID_LENGTH);
            aml_writeReg(unit, PEARL_ATWD, &reg_atwd, sizeof(reg_atwd));
            break;
        case AIR_L2_MAC_FLUSH_TYPE_PORT:
            reg_atc |= PEARL_ATC_MAT_MAC_BY_LIFETIME_PORT;
            HAL_AIR_PORT_TO_MAC_PORT(unit, value, mac_port);
            reg_atwd = BITS_OFF_L(1UL << mac_port, PEARL_ATWD2_MAC_PORT_OFFSET, PEARL_ATWD2_MAC_PORT_LENGTH);
            aml_writeReg(unit, PEARL_ATWD2, &reg_atwd, sizeof(reg_atwd));
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
    }
    /* Configure to dynamic entry for ATA */
    reg_ata2 = BITS_OFF_L(1UL, PEARL_ATA2_MAC_LIFETIME_OFFSET, PEARL_ATA2_MAC_LIFETIME_LENGTH);
    aml_writeReg(unit, PEARL_ATA2, &reg_ata2, sizeof(reg_ata2));

    if (AIR_E_OK == rc)
    {
        osal_delayUs(HAL_PEARL_L2_DELAY_US);
        aml_writeReg(unit, PEARL_ATC, &reg_atc, sizeof(reg_atc));
        if (AIR_E_TIMEOUT == _checkL2Busy(unit))
        {
            rc = AIR_E_TIMEOUT;
        }
    }

    HAL_PEARL_L2_FDB_UNLOCK(unit);
    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_setMacAddrAgeOut
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
hal_pearl_l2_setMacAddrAgeOut(
    const UI32_T unit,
    const UI32_T age_time)
{
    UI32_T u32dat;
    UI32_T age_cnt, age_unit;

    /* Read the old register value */
    aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));

    u32dat &= ~BITS_RANGE(PEARL_AAC_AGE_UNIT_OFFSET, PEARL_AAC_AGE_UNIT_LENGTH);
    u32dat &= ~BITS_RANGE(PEARL_AAC_AGE_CNT_OFFSET, PEARL_AAC_AGE_CNT_LENGTH);

    /* Calcuate the aging count/unit */
    _HAL_PEARL_L2_AGING_TIME_2_CNT(age_time, age_cnt, age_unit);

    /* Write the new register value */
    u32dat |= BITS_OFF_L((age_unit - 1), PEARL_AAC_AGE_UNIT_OFFSET, PEARL_AAC_AGE_UNIT_LENGTH);
    u32dat |= BITS_OFF_L((age_cnt - 1), PEARL_AAC_AGE_CNT_OFFSET, PEARL_AAC_AGE_CNT_LENGTH);

    aml_writeReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_getMacAddrAgeOut
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
hal_pearl_l2_getMacAddrAgeOut(
    const UI32_T unit,
    UI32_T      *ptr_age_time)
{
    UI32_T u32dat;
    UI32_T age_cnt, age_unit;

    /* Read data from register */
    aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));

    age_cnt = BITS_OFF_R(u32dat, PEARL_AAC_AGE_CNT_OFFSET, PEARL_AAC_AGE_CNT_LENGTH);
    age_unit = BITS_OFF_R(u32dat, PEARL_AAC_AGE_UNIT_OFFSET, PEARL_AAC_AGE_UNIT_LENGTH);
    (*ptr_age_time) = _HAL_PEARL_L2_AGING_TIME(age_cnt, age_unit);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_setMacAddrAgeOutMode
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
hal_pearl_l2_setMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI32_T u32dat;
    UI32_T mac_port;

    /* Translate port type */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Access regiser */
    aml_readReg(unit, PEARL_AGDIS, &u32dat, sizeof(u32dat));
    if (TRUE == enable)
    {
        u32dat &= ~BIT(mac_port);
    }
    else
    {
        u32dat |= BIT(mac_port);
    }
    aml_writeReg(unit, PEARL_AGDIS, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_getMacAddrAgeOutMode
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
hal_pearl_l2_getMacAddrAgeOutMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat;
    UI32_T mac_port;

    /* Translate port type */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Access regiser */
    aml_readReg(unit, PEARL_AGDIS, &u32dat, sizeof(u32dat));
    (*ptr_enable) = (!!BITS_OFF_R(u32dat, mac_port, 1)) ? FALSE : TRUE;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_getMacBucketSize
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
hal_pearl_l2_getMacBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size)
{
    /* Access regiser */
    (*ptr_size) = HAL_PEARL_L2_MAC_SET_NUM;

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_l2_setForwardMode
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
hal_pearl_l2_setForwardMode(
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
            offset = PEARL_BCF;
            break;
        case AIR_FORWARD_TYPE_UMCST:
            offset = PEARL_UNMF;
            break;
        case AIR_FORWARD_TYPE_UUCST:
            offset = PEARL_UNUF;
            break;
        case AIR_FORWARD_TYPE_UIPMCST:
            offset = PEARL_UNIPMF;
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
            aml_writeReg(unit, PEARL_QRYP, &(mac_pbm[0]), sizeof(UI32_T));
            aml_writeReg(unit, PEARL_DRP, &(mac_pbm[0]), sizeof(UI32_T));
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_getForwardMode
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
hal_pearl_l2_getForwardMode(
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
            offset = PEARL_BCF;
            break;
        case AIR_FORWARD_TYPE_UMCST:
            offset = PEARL_UNMF;
            break;
        case AIR_FORWARD_TYPE_UUCST:
            offset = PEARL_UNUF;
            break;
        case AIR_FORWARD_TYPE_UIPMCST:
            offset = PEARL_UNIPMF;
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

/* FUNCTION NAME: hal_pearl_l2_lockL2FdbResource
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
hal_pearl_l2_lockL2FdbResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_L2);

    return HAL_COMMON_LOCK_RESOURCE(PTR_HAL_PEARL_L2_HW_TBL_MUTEX(unit), AIR_SEMAPHORE_WAIT_FOREVER);
}

/* FUNCTION NAME: hal_pearl_l2_unlockL2FdbResource
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
hal_pearl_l2_unlockL2FdbResource(
    const UI32_T unit)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_L2);

    return HAL_COMMON_FREE_RESOURCE(PTR_HAL_PEARL_L2_HW_TBL_MUTEX(unit));
}

/* FUNCTION NAME: hal_pearl_l2_getCapacity
 * PURPOSE:
 *      Get the l2 resource capacity
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
hal_pearl_l2_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_L2_FDB == type)
    {
        *ptr_size = HAL_PEARL_L2_MAX_ENTRY_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_pearl_l2_getUsage
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
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_l2_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_agsc = 0;

    if (AIR_SWC_RSRC_L2_FDB == type)
    {
        aml_readReg(unit, PEARL_AGSC, &reg_agsc, sizeof(reg_agsc));
        *ptr_cnt = BITS_OFF_R(reg_agsc, PEARL_AGSC_GLB_SA_LRN_CNT_OFFT, PEARL_AGSC_GLB_SA_LRN_CNT_LENGTH);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}
