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

/* FILE NAME:  hal_coral_ipmc.c
 * PURPOSE:
 *  Implement Multicast module.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_ipmc.h>

#include <air_error.h>
#include <air_ipmc.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/switch/coral/hal_coral_l2.h>
#include <hal/switch/coral/hal_coral_port.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    HAL_CORAL_IPMC_ADDRESS_TYPE_GROUP,
    HAL_CORAL_IPMC_ADDRESS_TYPE_SOURCE,
    HAL_CORAL_IPMC_ADDRESS_TYPE_LAST
} HAL_CORAL_IPMC_ADDRESS_TYPE_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_IPMC_U32_ENDIAN_XCHG(__ip__)                                                              \
    __ip__ = (__ip__ >> 24) | ((__ip__ >> 8) & 0x0000FF00) | ((__ip__ << 8) & 0x00FF0000) | (__ip__ << 24);

#define HAL_IPV4_IS_MULTICAST(addr) (0xE0000000 == ((addr) & 0xF0000000))
#define HAL_IPV6_IS_MULTICAST(addr) (0xFF == (((UI8_T *)(addr))[0]))
#define HAL_L3_IP_IS_MULTICAST(ptr_ip)                                               \
    ((TRUE == (ptr_ip)->ipv4) ? HAL_IPV4_IS_MULTICAST((ptr_ip)->ip_addr.ipv4_addr) : \
                                HAL_IPV6_IS_MULTICAST((ptr_ip)->ip_addr.ipv6_addr))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_IPMC, "hal_coral_ipmc.c");
static BOOL_T _search_end = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _writeIPAddress
 * PURPOSE:
 *      Write the IP address
 * INPUT:
 *      unit            --  Select device ID
 *      ipaddr          --  Specific IP address
 *      type            --  1: group
 *                          0: source
 *      is_ipv4         --  TRUE: IPv4
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_writeIPAddress(
    const UI32_T                        unit,
    const AIR_IP_ADDR_T                *ptr_ipaddr,
    const HAL_CORAL_IPMC_ADDRESS_TYPE_T type,
    const BOOL_T                        is_ipv4)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         ip6addr = 0;
    UI32_T         ATA_grp[4] = {CORAL_ATA7, CORAL_ATA5, CORAL_ATA3, CORAL_ATA1};
    UI32_T         ATA_src[4] = {CORAL_ATA8, CORAL_ATA6, CORAL_ATA4, CORAL_ATA2};
    UI8_T          i;

    if (TRUE == is_ipv4)
    {
        if (HAL_CORAL_IPMC_ADDRESS_TYPE_GROUP == type)
        {
            aml_writeReg(unit, CORAL_ATA1, &ptr_ipaddr->ip_addr.ipv4_addr, sizeof(UI32_T));
        }
        else
        {
            aml_writeReg(unit, CORAL_ATA2, &ptr_ipaddr->ip_addr.ipv4_addr, sizeof(UI32_T));
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            osal_memcpy(&ip6addr, &ptr_ipaddr->ip_addr.ipv6_addr[i * 4], sizeof(UI32_T));
            HAL_CORAL_IPMC_U32_ENDIAN_XCHG(ip6addr);
            if (HAL_CORAL_IPMC_ADDRESS_TYPE_GROUP == type)
            {
                aml_writeReg(unit, ATA_grp[i], &ip6addr, sizeof(UI32_T));
            }
            else
            {
                aml_writeReg(unit, ATA_src[i], &ip6addr, sizeof(UI32_T));
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: _fill_IPMC_ATA
 * PURPOSE:
 *      Fill register CORAL_ATA for IPMC Address table.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_fill_IPMC_ATA(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    /* Set DIP address */
    _writeIPAddress(unit, &ptr_entry->group_addr, HAL_CORAL_IPMC_ADDRESS_TYPE_GROUP, ptr_entry->group_addr.ipv4);

    /* For IGMPv3 write source ip table */
    if (AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type)
    {
        /* Set source address */
        if (!(CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr)))
        {
            _writeIPAddress(unit, &ptr_entry->source_addr, HAL_CORAL_IPMC_ADDRESS_TYPE_SOURCE,
                            ptr_entry->group_addr.ipv4);
        }
    }
}

/* FUNCTION NAME: _fill_IPMC_ATWD
 * PURPOSE:
 *      Fill register CORAL_ATWD for IPMC Address table.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 *      is_valid        --  BOOL_T
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_fill_IPMC_ATWD(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry,
    const BOOL_T            is_valid)
{
    UI32_T u32dat = 0;

    u32dat = 0;
    /* Set attributes */
    if (TRUE == is_valid)
    {
        u32dat |= BIT(CORAL_ATWD_IPMC_LIVE_OFFSET);
    }
    if (ptr_entry->flags & AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
    {
        u32dat |= BIT(CORAL_ATWD_IPMC_LEAK_OFFSET);
    }
    if (FALSE == ptr_entry->group_addr.ipv4)
    {
        u32dat |= BIT(CORAL_ATWD_IPMC_IPV6_OFFSET);
    }
    u32dat |= BITS_OFF_L(ptr_entry->vid, CORAL_ATWD_IPMC_VID_OFFSET, CORAL_ATWD_IPMC_VID_LENGTH);
    aml_writeReg(unit, CORAL_ATWD, &u32dat, sizeof(UI32_T));
}

/* FUNCTION NAME: _fill_IPMC_ATWD2
 * PURPOSE:
 *      Fill register CORAL_ATWD2 for IPMC Address table.
 * INPUT:
 *      unit            --  Device ID
 *      mac_port_bitmap --  Port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_fill_IPMC_ATWD2(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T mac_port_bitmap)
{
    UI32_T u32dat = 0, mac_port = 0;

    /* Set member ports of IPMC entry */
    u32dat = 0;
    AIR_PORT_FOREACH(mac_port_bitmap, mac_port)
    {
        u32dat |= BIT(mac_port);
    }
    aml_writeReg(unit, CORAL_ATWD2, &u32dat, sizeof(UI32_T));
}

/* FUNCTION NAME: _fill_IPMC_ATC_single
 * PURPOSE:
 *      Fill register CORAL_ATC for IPMC Address single accessing.
 * INPUT:
 *      unit            --  Select device ID
 *      ip_type         --  AIR_IPMC_TYPE_T
 *      cmd             --  Access command
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_fill_IPMC_ATC_single(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T ip_type,
    const UI32_T          cmd)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;
    AIR_IPMC_TYPE_T lookup_type;

    /* Check the Look Up Type */
    rc = hal_coral_ipmc_getMcastLookupType(unit, &lookup_type);
    if (AIR_E_OK == rc)
    {
        u32dat = CORAL_ATC_START_BUSY | cmd;
        if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == lookup_type)
        {
            if (AIR_IPMC_TYPE_GRP_SRC == ip_type)
            {
                u32dat |= CORAL_ATC_SAT_HALF_SIP;
            }
            else
            {
                u32dat |= CORAL_ATC_SAT_HALF_DIP;
            }
        }
        else
        {
            if (AIR_IPMC_TYPE_GRP_SRC == ip_type)
            {
                u32dat |= CORAL_ATC_SAT_SIP;
            }
            else
            {
                u32dat |= CORAL_ATC_SAT_DIP;
            }
        }
        aml_writeReg(unit, CORAL_ATC, &u32dat, sizeof(UI32_T));
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "get the lookup type failed, rc=%u\n", rc);
    }

    return rc;
}

/* FUNCTION NAME: _fill_IPMC_ATC_multi
 * PURPOSE:
 *      Fill register CORAL_ATC for IPMC Address multiple accessing.
 * INPUT:
 *      unit            --  Select device ID
 *      mat             --  Multiple access target
 *      cmd             --  Access command
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_fill_IPMC_ATC_multi(
    const UI32_T unit,
    const UI32_T mat,
    const UI32_T cmd)
{
    UI32_T u32dat = 0;

    u32dat = CORAL_ATC_START_BUSY | mat | cmd;
    aml_writeReg(unit, CORAL_ATC, &u32dat, sizeof(UI32_T));
}

/* FUNCTION NAME: _fill_IPMC_ATRDS
 * PURPOSE:
 *      To fill IPMC ATRDS register with selected bank number
 * INPUT:
 *      unit            --  Select device ID
 *      bank            --  Select bank number
 * OUTPUT:
 *      AIR_E_OK
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_fill_IPMC_ATRDS(
    const UI32_T unit,
    const BOOL_T bank)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    /* Select bank */
    u32dat = BITS_OFF_L(bank, CORAL_ATRDS_MAC_SEL_OFFSET, CORAL_ATRDS_MAC_SEL_LENGTH);
    aml_writeReg(unit, CORAL_ATRDS, &u32dat, sizeof(UI32_T));

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD0
 * PURPOSE:
 *      Read the value from ATWD register 0.
 * INPUT:
 *      unit            --  Select device ID
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD0(
    const UI32_T      unit,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI16_T         vid = 0;

    aml_readReg(unit, CORAL_ATRD0, &u32dat, sizeof(UI32_T));
    vid = (UI16_T)BITS_OFF_R(u32dat, CORAL_ATRD0_IPM_VID_OFFSET, CORAL_ATRD0_IPM_VID_RANGE);
    if (TRUE == check_key)
    {
        if (ptr_entry->vid != vid)
        {
            DIAG_PRINT(HAL_DBG_INFO, "VID is not match\n");
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        if (TRUE == read_back)
        {
            ptr_entry->vid = vid;
        }
    }

    if (AIR_E_OK == rc)
    {
        if (TRUE == read_back)
        {
            if (BITS_OFF_R(u32dat, CORAL_ATRD0_IPM_LEAKY_OFFSET, CORAL_ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD12_IPV4
 * PURPOSE:
 *      Read the IPv4 address from ATWD register 1, 2.
 * INPUT:
 *      unit            --  Select device ID
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD12_IPV4(
    const UI32_T      unit,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    /* take ATRD1 */
    aml_readReg(unit, CORAL_ATRD1, &u32dat, sizeof(UI32_T));
    if (TRUE == check_key)
    {
        if ((AIR_IPV4_ZERO != ptr_entry->group_addr.ip_addr.ipv4_addr) &&
            (ptr_entry->group_addr.ip_addr.ipv4_addr != u32dat))
        {
            DIAG_PRINT(HAL_DBG_INFO, "IPv4 address is not match\n");
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        if (TRUE == read_back)
        {
            ptr_entry->group_addr.ip_addr.ipv4_addr = u32dat;
            ptr_entry->group_addr.ipv4 = TRUE;
        }
    }

    if (AIR_E_OK == rc)
    {
        /* take ATRD2 */
        aml_readReg(unit, CORAL_ATRD2, &u32dat, sizeof(UI32_T));
        if (TRUE == check_key)
        {
            if ((AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type) &&
                (AIR_IPV4_ZERO != ptr_entry->source_addr.ip_addr.ipv4_addr) &&
                (ptr_entry->source_addr.ip_addr.ipv4_addr != u32dat))
            {
                DIAG_PRINT(HAL_DBG_INFO, "SIPv4 address is not match\n");
                rc = AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            if (TRUE == read_back)
            {
                ptr_entry->source_addr.ip_addr.ipv4_addr = u32dat;
                ptr_entry->source_addr.ipv4 = TRUE;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD12_IPV6
 * PURPOSE:
 *      Read the IPv6 address from ATWD register 1, 2.
 * INPUT:
 *      unit            --  Select device ID
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD12_IPV6(
    const UI32_T      unit,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, i = 0;
    AIR_IP_ADDR_T  group_addr, source_addr;

    for (i = 0; i < HAL_CORAL_L2_MAC_SET_NUM; i++)
    {
        _fill_IPMC_ATRDS(unit, i);

        /* take ATRD1 */
        aml_readReg(unit, CORAL_ATRD1, &u32dat, sizeof(UI32_T));
        HAL_CORAL_IPMC_U32_ENDIAN_XCHG(u32dat);
        group_addr.ipv4 = FALSE;
        osal_memcpy(&group_addr.ip_addr.ipv6_addr[(HAL_CORAL_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat, sizeof(UI32_T));

        /* take ATRD1 */
        aml_readReg(unit, CORAL_ATRD2, &u32dat, sizeof(UI32_T));
        HAL_CORAL_IPMC_U32_ENDIAN_XCHG(u32dat);
        source_addr.ipv4 = FALSE;
        osal_memcpy(&source_addr.ip_addr.ipv6_addr[(HAL_CORAL_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat, sizeof(UI32_T));
    }
    if (TRUE == check_key)
    {
        if (osal_memcmp(&ptr_entry->group_addr, &group_addr, sizeof(AIR_IP_ADDR_T)))
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
        if ((AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type) &&
            osal_memcmp(&ptr_entry->source_addr, &source_addr, sizeof(AIR_IP_ADDR_T)))
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        if (TRUE == read_back)
        {
            osal_memcpy(&ptr_entry->group_addr, &group_addr, sizeof(AIR_IP_ADDR_T));
            osal_memcpy(&ptr_entry->source_addr, &source_addr, sizeof(AIR_IP_ADDR_T));
        }
    }

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD12
 * PURPOSE:
 *      Read the IP address from ATWD register 1, 2.
 * INPUT:
 *      unit            --  Select device ID
 *      is_ipv4         --  TRUE: IPv4
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD12(
    const UI32_T      unit,
    const BOOL_T      is_ipv4,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (is_ipv4)
    {
        rc = _read_IPMC_ATRD12_IPV4(unit, check_key, read_back, ptr_entry);
    }
    else
    {
        rc = _read_IPMC_ATRD12_IPV6(unit, check_key, read_back, ptr_entry);
    }

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD3
 * PURPOSE:
 *      Read the value from ATWD register 3.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD3(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            u32dat = 0, port = 0, mac_port = 0;
    AIR_PORT_BITMAP_T air_port_bitmap;

    AIR_PORT_BITMAP_CLEAR(air_port_bitmap);

    /* take ATRD3 */
    aml_readReg(unit, CORAL_ATRD3, &u32dat, sizeof(UI32_T));
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
        if (u32dat & (0x01 << mac_port))
        {
            AIR_PORT_ADD(air_port_bitmap, port);
        }
    }
    AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, air_port_bitmap);

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD
 * PURPOSE:
 *      Read the IPMC table
 * INPUT:
 *      unit            --  Select device ID
 *      is_ipv4         --  TRUE: IPv4
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD(
    const UI32_T      unit,
    const BOOL_T      is_ipv4,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_E_OK == rc)
    {
        rc = _read_IPMC_ATRD0(unit, check_key, read_back, ptr_entry);
    }

    if (AIR_E_OK == rc)
    {
        rc = _read_IPMC_ATRD12(unit, is_ipv4, check_key, read_back, ptr_entry);
    }

    if (AIR_E_OK == rc)
    {
        if (TRUE == read_back)
        {
            rc = _read_IPMC_ATRD3(unit, ptr_entry);
        }
    }

    return rc;
}

/* FUNCTION NAME: _read_IPMC_ATRD_with_bank
 * PURPOSE:
 *      Read the IPMC table
 * INPUT:
 *      unit            --  Select device ID
 *      is_ipv4         --  TRUE: IPv4
 *      banks           --  The hit result banks from ATC
 *      check_key       --  Check the key of IPMC entry, and return if it's not matched
 *      read_back       --  Read the value back or not
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 *      ptr_entry_cnt   --  The number of IPMC entries
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_IPMC_ATRD_with_bank(
    const UI32_T      unit,
    const BOOL_T      is_ipv4,
    const UI32_T      banks,
    const BOOL_T      check_key,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_entry_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i = 0;

    DIAG_PRINT(HAL_DBG_INFO, "unit=(%u), is_ipv4=(%u), banks=(0x%x), check_key=(%u), read_back=(%u)\n", unit, is_ipv4,
               banks, check_key, read_back);
    if (TRUE == is_ipv4)
    {
        for (i = 0; i < HAL_CORAL_L2_MAC_SET_NUM; i++)
        {
            if (!BIT_CHK(banks, i))
            {
                /* skip null bank */
                continue;
            }

            _fill_IPMC_ATRDS(unit, i);
            if (NULL == ptr_entry)
            {
                break;
            }
            rc = _read_IPMC_ATRD(unit, is_ipv4, check_key, read_back, ptr_entry);
            if (AIR_E_OK != rc)
            {
                break;
            }
            if ((NULL != ptr_entry) && (FALSE == check_key))
            {
                ptr_entry++;
            }
            if (NULL != ptr_entry_cnt)
            {
                (*ptr_entry_cnt)++;
            }
        }
    }
    else
    {
        if (banks != BITS_RANGE(0, HAL_CORAL_L2_MAC_SET_NUM))
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
        else
        {
            if (NULL != ptr_entry)
            {
                rc = _read_IPMC_ATRD(unit, is_ipv4, check_key, read_back, ptr_entry);
            }
            if ((AIR_E_OK == rc) && (NULL != ptr_entry_cnt))
            {
                (*ptr_entry_cnt)++;
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: _checkAtcBusy
 * PURPOSE:
 *      Check BUSY bit of ATC
 * INPUT:
 *      unit            --  Device ID
 * OUTPUT:
 *      ptr_atc         --  The last status of register ATC
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkAtcBusy(
    const UI32_T unit,
    UI32_T      *ptr_atc)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i;
    UI32_T         reg_atc;

    /* Check BUSY bit is 0 */
    for (i = 0; i < HAL_CORAL_IPMC_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, CORAL_ATC, &reg_atc, sizeof(reg_atc));
        if (!BITS_OFF_R(reg_atc, CORAL_ATC_BUSY_OFFSET, CORAL_ATC_BUSY_LENGTH))
        {
            break;
        }
        osal_delayUs(HAL_CORAL_IPMC_DELAY_US);
    }
    if (i >= HAL_CORAL_IPMC_MAX_BUSY_TIME)
    {
        DIAG_PRINT(HAL_DBG_ERR, "IPMC table access timeout\n");
        rc = AIR_E_TIMEOUT;
    }
    if ((AIR_E_OK == rc) && (NULL != ptr_atc))
    {
        *ptr_atc = reg_atc;
    }
    return rc;
}

/* FUNCTION NAME: _setIpmcEntry
 * PURPOSE:
 *      Writing IPMC table entry execution function
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 *      mac_port_bitmap --  Bitmap of ports which are members of the group
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_setIpmcEntry(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry,
    const AIR_PORT_BITMAP_T mac_port_bitmap)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, banks = 0;

    /* Create new entry or update group filter */
    /* Fill attributions info of IPMC table */
    _fill_IPMC_ATWD(unit, ptr_entry, TRUE);
    _fill_IPMC_ATWD2(unit, mac_port_bitmap);

    /* Fill DIP & SIP info */
    _fill_IPMC_ATA(unit, ptr_entry);

    /* Fill ATC command to access the entry */
    if (AIR_E_OK == rc)
    {
        rc = _fill_IPMC_ATC_single(unit, ptr_entry->type, CORAL_ATC_CMD_WRITE);
    }

    /* Check write state */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, &u32dat);
    }

    /* Get banks */
    if (AIR_E_OK == rc)
    {
        banks = BITS_OFF_R(u32dat, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                   BITS_OFF_R(u32dat, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH), banks);
        if (0 == banks)
        {
            rc = AIR_E_OTHERS;
        }
    }

    return rc;
}

/* FUNCTION NAME: _getIpmcEntry
 * PURPOSE:
 *      Read IPMC table entry.
 * INPUT:
 *      unit            --  Device ID
 *      read_back       --  Read the value back or not
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      ptr_entry       --  Structure of IPMC address entry
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_getIpmcEntry(
    const UI32_T      unit,
    const BOOL_T      read_back,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, addr = 0, banks = 0;
    BOOL_T         is_ipv4 = ptr_entry->group_addr.ipv4;

    /* Fill attributions info of IPMC table */
    _fill_IPMC_ATWD(unit, ptr_entry, TRUE);

    /* Fill DIP & SIP info */
    _fill_IPMC_ATA(unit, ptr_entry);

    /* Fill ATC command to access the entry */
    if (AIR_E_OK == rc)
    {
        rc = _fill_IPMC_ATC_single(unit, ptr_entry->type, CORAL_ATC_CMD_READ);
    }

    /* Check write state */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, &u32dat);
    }

    if (AIR_E_OK == rc)
    {
        /* Get address */
        addr = BITS_OFF_R(u32dat, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);

        if (banks)
        {
            /* Get attributes w/ check key */
            rc = _read_IPMC_ATRD_with_bank(unit, is_ipv4, banks, TRUE, read_back, ptr_entry, NULL);
        }
        else
        {
            if ((HAL_CORAL_L2_MAX_ADDR_NUM - 1) == addr)
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
            }
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }
    return rc;
}

/* FUNCTION NAME: _addIpmcEntry
 * PURPOSE:
 *      Write IPMC table entry.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 *      mac_port_bitmap --  Bitmap of ports which are members of the group
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_addIpmcEntry(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry,
    const AIR_PORT_BITMAP_T mac_port_bitmap)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T tmp_entry = {.type = 0,
                                  .vid = 0,
                                  .group_addr.ip_addr.ipv6_addr = {0},
                                  .group_addr.ipv4 = 0,
                                  .source_addr.ip_addr.ipv6_addr = {0},
                                  .source_addr.ipv4 = 0,
                                  .flags = 0,
                                  .port_bitmap = {0}};

    /* Find the target entry in IPMC table. */
    osal_memcpy(&tmp_entry, ptr_entry, sizeof(AIR_IPMC_ENTRY_T));
    rc = _getIpmcEntry(unit, FALSE, &tmp_entry);

    if (AIR_E_ENTRY_NOT_FOUND == rc)
    {
        /* Add the new entry into IPMC table. */
        rc = _setIpmcEntry(unit, ptr_entry, mac_port_bitmap);
    }
    else
    {
        rc = AIR_E_ENTRY_EXISTS;
    }

    return rc;
}

/* FUNCTION NAME: _deleteIpmcEntry
 * PURPOSE:
 *      Delete an IPMC table entry.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_deleteIpmcEntry(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, banks = 0;

    /* Fill attributions info of IPMC table */
    _fill_IPMC_ATWD(unit, ptr_entry, FALSE);

    /* Fill DIP & SIP info */
    _fill_IPMC_ATA(unit, ptr_entry);

    /* Fill ATC command to access the entry */
    if (AIR_E_OK == rc)
    {
        rc = _fill_IPMC_ATC_single(unit, ptr_entry->type, CORAL_ATC_CMD_WRITE);
    }

    /* Check write state */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, &u32dat);
    }

    /* Get banks */
    if (AIR_E_OK == rc)
    {
        banks = BITS_OFF_R(u32dat, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                   BITS_OFF_R(u32dat, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH), banks);
        if (0 == banks)
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    return rc;
}

/* FUNCTION NAME: _cfgIpmcMember
 * PURPOSE:
 *      To configure the member into IPMC address entry
 * INPUT:
 *      unit            --  Device ID
 *      ptr_entry       --  Structure of IPMC address entry
 *      is_add          --  TRUE: add new member
 *                           FALSE: delete member
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_cfgIpmcMember(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry,
    const BOOL_T            is_add)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            port = 0;
    AIR_PORT_BITMAP_T final_port_bitmap, cfg_port_bitmap, air_port_bitmap;
    AIR_IPMC_ENTRY_T  tmp_entry;

    AIR_PORT_BITMAP_CLEAR(final_port_bitmap);
    AIR_PORT_BITMAP_CLEAR(cfg_port_bitmap);
    AIR_PORT_BITMAP_CLEAR(air_port_bitmap);

    /* Get the port bitmap to be configured to IPMC table entry */
    AIR_PORT_BITMAP_COPY(air_port_bitmap, ptr_entry->port_bitmap);
    CMLIB_PORT_BITMAP_AND(air_port_bitmap, HAL_PORT_BMP_ETH(unit));
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, air_port_bitmap, cfg_port_bitmap);

    /* Get the original port bitmap from IPMC table entry */
    osal_memcpy(&tmp_entry, ptr_entry, sizeof(AIR_IPMC_ENTRY_T));
    rc = _getIpmcEntry(unit, TRUE, &tmp_entry);

    if (AIR_E_OK == rc)
    {
        /* Translate the port bitmap to MAC port bitmap */
        HAL_AIR_PBMP_TO_MAC_PBMP(unit, tmp_entry.port_bitmap, final_port_bitmap);

        /* Fill DIP & SIP info */
        _fill_IPMC_ATA(unit, ptr_entry);

        /* Set ATWD table to new member */
        AIR_PORT_FOREACH(cfg_port_bitmap, port)
        {
            if (is_add)
            {
                AIR_PORT_ADD(final_port_bitmap, port);
            }
            else
            {
                AIR_PORT_DEL(final_port_bitmap, port);
            }
        }

        /* Fill attributions info of IPMC table */
        _fill_IPMC_ATWD(unit, ptr_entry, TRUE);
        _fill_IPMC_ATWD2(unit, final_port_bitmap);

        /* Fill ATC command to access the entry */
        rc = _fill_IPMC_ATC_single(unit, ptr_entry->type, CORAL_ATC_CMD_WRITE);
    }

    /* Check write state */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, NULL);
    }

    return rc;
}

/* FUNCTION NAME: _searchIpmcEntry
 * PURPOSE:
 *      To configure the member into IPMC address entry
 * INPUT:
 *      unit            --  Device ID
 *      match_type      --  Match type
 *      is_first        --  TRUE: search first entry
 *                          FALSE: search next entry
 *      ptr_entry_cnt   --  Number of entries found
 *      ptr_entry       --  Structure of IPMC address entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_searchIpmcEntry(
    const UI32_T                unit,
    const AIR_IPMC_MATCH_TYPE_T match_type,
    const BOOL_T                is_first,
    UI32_T                     *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T           *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, addr = 0, banks = 0, mat = 0;
    BOOL_T         is_ipv4 = TRUE;

    if (AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
    {
        mat = CORAL_ATC_MAT_DIPV4;
        is_ipv4 = TRUE;
    }
    else if (AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC == match_type)
    {
        mat = CORAL_ATC_MAT_SIPV4;
        is_ipv4 = TRUE;
    }
    else if (AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
    {
        mat = CORAL_ATC_MAT_DIPV6;
        is_ipv4 = FALSE;
    }
    else if (AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC == match_type)
    {
        mat = CORAL_ATC_MAT_SIPV6;
        is_ipv4 = FALSE;
    }

    _fill_IPMC_ATC_multi(unit, mat, (TRUE == is_first) ? CORAL_ATC_CMD_SEARCH : CORAL_ATC_CMD_SEARCH_NEXT);

    /* Check write state */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, &u32dat);
    }

    if (AIR_E_OK == rc)
    {
        /* Get address */
        addr = BITS_OFF_R(u32dat, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks)
        {
            /* Get attributes w/o check key */
            rc = _read_IPMC_ATRD_with_bank(unit, is_ipv4, banks, FALSE, TRUE, ptr_entry, ptr_entry_cnt);
        }
        else
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }

        if ((HAL_CORAL_L2_MAX_ADDR_NUM - 1) == addr)
        {
            _search_end = TRUE;
        }
    }
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_ipmc_setMcastLookupType
 * PURPOSE:
 *      This API is used to set multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *      type            --  The multicast type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setMcastLookupType(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == type)
    {
        /* set igmpv3 & v2 */
        u32dat = CORAL_ISC_CSR_IGMPV3V2_EN;
    }
    else if (AIR_IPMC_TYPE_GRP_SRC == type)
    {
        /* set igmp v3 */
        u32dat = CORAL_ISC_CSR_IGMPV3_EN;
    }
    else if (AIR_IPMC_TYPE_GRP == type)
    {
        /* set igmpv2 */
        u32dat = 0;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        aml_writeReg(unit, CORAL_ISC, &u32dat, sizeof(UI32_T));
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_getMcastLookupType
 * PURPOSE:
 *      This API is used to get multicast type.
 * INPUT:
 *      unit            --  Device unit number
 * OUTPUT:
 *      ptr_type        --  The multicast type
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastLookupType(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    aml_readReg(unit, CORAL_ISC, &u32dat, sizeof(UI32_T));

    if (u32dat & CORAL_ISC_CSR_IGMPV3V2_EN)
    {
        *ptr_type = AIR_IPMC_TYPE_GRP_SRC_AND_GRP;
    }
    else if (u32dat & CORAL_ISC_CSR_IGMPV3_EN)
    {
        *ptr_type = AIR_IPMC_TYPE_GRP_SRC;
    }
    else
    {
        *ptr_type = AIR_IPMC_TYPE_GRP;
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_addMcastAddr
 * PURPOSE:
 *      Add or set a multicast address entry.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_EXISTS
 * NOTES:
 *      1.Can't add group if the group address is exist.
 */
AIR_ERROR_NO_T
hal_coral_ipmc_addMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_PORT_BITMAP_T air_port_bitmap, mac_port_bitmap;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check group address*/
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        AIR_PORT_BITMAP_CLEAR(air_port_bitmap);
        AIR_PORT_BITMAP_CLEAR(mac_port_bitmap);

        AIR_PORT_BITMAP_COPY(air_port_bitmap, ptr_entry->port_bitmap);
        CMLIB_PORT_BITMAP_AND(air_port_bitmap, HAL_PORT_BMP_ETH(unit));
        HAL_AIR_PBMP_TO_MAC_PBMP(unit, air_port_bitmap, mac_port_bitmap);

        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _addIpmcEntry(unit, ptr_entry, mac_port_bitmap);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_getMcastAddr
 * PURPOSE:
 *      Get multicast address entry for specific group and source address.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      ptr_entry       --  The multicast entry
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastAddr(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check IP address validation */
    if ((CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr)) && (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _getIpmcEntry(unit, TRUE, ptr_entry);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_delMcastAddr
 * PURPOSE:
 *      Delete multicast address entry.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      1.del group address will delete group address relate all source address.
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _deleteIpmcEntry(unit, ptr_entry);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_delAllMcastAddr
 * PURPOSE:
 *      Delete all multicast address entry.
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delAllMcastAddr(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    HAL_CORAL_L2_FDB_LOCK(unit);
    /* Clear all SIP entry */
    if (AIR_E_OK == rc)
    {
        _fill_IPMC_ATC_multi(unit, CORAL_ATC_MAT_SIP, CORAL_ATC_CMD_CLEAN);
        rc = _checkAtcBusy(unit, &u32dat);
    }

    /* Clear all DIP entry*/
    if (AIR_E_OK == rc)
    {
        _fill_IPMC_ATC_multi(unit, CORAL_ATC_MAT_DIP, CORAL_ATC_CMD_CLEAN);
        rc = _checkAtcBusy(unit, &u32dat);
    }
    HAL_CORAL_L2_FDB_UNLOCK(unit);
    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_addMcastMember
 * PURPOSE:
 *      Add member for a specific multicast entry.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_addMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check source address*/
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check version */
    if ((AIR_IPMC_TYPE_GRP != ptr_entry->type) && (AIR_IPMC_TYPE_GRP_SRC != ptr_entry->type))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _cfgIpmcMember(unit, ptr_entry, TRUE);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_delMcastMember
 * PURPOSE:
 *      Delete member for a specific multicast entry.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_delMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check source address*/
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check version */
    if ((AIR_IPMC_TYPE_GRP != ptr_entry->type) && (AIR_IPMC_TYPE_GRP_SRC != ptr_entry->type))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _cfgIpmcMember(unit, ptr_entry, FALSE);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_getMcastMemberCnt
 * PURPOSE:
 *      Get member count for specific multicast group.
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      ptr_count       --  Member count
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastMemberCnt(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_count)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check group address*/
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    /* get the IPMC entry */
    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        rc = _getIpmcEntry(unit, TRUE, ptr_entry);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    if (AIR_E_OK == rc)
    {
        /* count the port quantity in the bitmap */
        CMLIB_PORT_BITMAP_COUNT(ptr_entry->port_bitmap, *ptr_count);
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_getMcastBucketSize
 * PURPOSE:
 *      Get the bucket size of one multicast address set when searching multicast.
 * INPUT:
 *      unit            --  Device ID
 * OUTPUT:
 *      ptr_size        --  The bucket size
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getMcastBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    /* Access regiser */
    (*ptr_size) = HAL_CORAL_L2_MAC_SET_NUM;

    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_getFirstMcastAddr
 * PURPOSE:
 *      This API is used to get a the first multicast address entry.
 * INPUT:
 *      unit            --  Device unit number
 *      match_type      --  The type to search multicast entry
 * OUTPUT:
        ptr_entry_cnt   --  The number of multicast address entries
 *      ptr_entry       --  The multicast entry
 * RETURN:
 *      AIR_E_OK        --  Operation succeeded.
 *      Others          --  Operation failed.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getFirstMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        _search_end = FALSE;
        /* Search the 1st entry with same group address */
        rc = _searchIpmcEntry(unit, match_type, TRUE, ptr_entry_cnt, ptr_entry);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_getNextMcastAddr
 * PURPOSE:
 *      Get next multicast source address entry for specific group and source address.
 * INPUT:
 *      unit            --  Select device ID
 *      match_type      --  The type to search multicast entry
 * OUTPUT:
 *      ptr_entry_cnt   --  The number of returned multicast entries
 *      ptr_entry       --  The multicast searching result.
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getNextMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Check IP address type */
    if (AIR_IPMC_TYPE_GRP_SRC_AND_GRP == ptr_entry->type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        /* If found the lastest entry last time, we couldn't keep to search the next entry */
        if (TRUE == _search_end)
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);
        /* Search the next available entry */
        rc = _searchIpmcEntry(unit, match_type, FALSE, ptr_entry_cnt, ptr_entry);
        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_setPortIpmcMode
 * PURPOSE:
 *      This API is used to set IPMC mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 *      enable          --  The IPMC Mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    if (TRUE == enable)
    {
        /* Enable igmp snooping */
        u32dat |= (CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_01 | CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_33 |
                   CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        /* Disable igmp snooping */
        u32dat = 0;
    }
    DIAG_PRINT(HAL_DBG_INFO, "mac_port=(%u), enable=(0x%u), u32dat=(0x%x)\n", mac_port, enable, u32dat);
    aml_writeReg(unit, CORAL_PIC(mac_port), &u32dat, sizeof(UI32_T));

    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_getPortIpmcMode
 * PURPOSE:
 *      This API is used to get IGMP snooping mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 * OUTPUT:
 *      ptr_enable      --  The IGMP snooping mode refer to
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, CORAL_PIC(mac_port), &u32dat, sizeof(UI32_T));
    if (u32dat == (CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_01 | CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_33 |
                   CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_224))
    {
        *ptr_enable = TRUE;
    }
    else if (u32dat == 0)
    {
        *ptr_enable = FALSE;
    }
    else
    {
        rc = AIR_E_OTHERS;
    }
    DIAG_PRINT(HAL_DBG_INFO, "mac_port=(%u), u32dat=(0x%x), ptr_enable=(%u), rc=(%u)\n", mac_port, u32dat, *ptr_enable,
               rc);

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_getCapacity
 * PURPOSE:
 *      Get the ipmc resource capacity
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
hal_coral_ipmc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_IPMC_FDB == type)
    {
        *ptr_size = HAL_CORAL_L2_MAX_ENTRY_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_ipmc_getUsage
 * PURPOSE:
 *      Get the ipmc resource usage
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
hal_coral_ipmc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cnt = 0, i = 0;
    UI32_T         reg_atc = 0, addr = 0, banks = 0;
    UI32_T         mat[2] = {CORAL_ATC_MAT_DIP, CORAL_ATC_MAT_SIP};

    if (AIR_SWC_RSRC_IPMC_FDB != type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_CORAL_L2_FDB_LOCK(unit);

        /* Search all IP entry */
        for (i = 0; i < 2; i++)
        {
            /* Search the 1st entry */
            _fill_IPMC_ATC_multi(unit, mat[i], CORAL_ATC_CMD_SEARCH);
            /* Check write state */
            rc = _checkAtcBusy(unit, &reg_atc);
            /* Get address */
            addr = BITS_OFF_R(reg_atc, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(reg_atc, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "reg_atc=(0x%x), addr=(0x%x), banks=(0x%x)\n", reg_atc, addr, banks);
            while ((AIR_E_OK == rc) && (0 != banks))
            {
                while (banks != 0)
                {
                    cnt += banks & 1;
                    banks >>= 1;
                }
                if ((HAL_CORAL_L2_MAX_ADDR_NUM - 1) == (addr))
                {
                    break;
                }
                /* Search the next entry*/
                _fill_IPMC_ATC_multi(unit, mat[i], CORAL_ATC_CMD_SEARCH_NEXT);
                /* Check write state */
                rc = _checkAtcBusy(unit, &reg_atc);
                /* Get address */
                addr = BITS_OFF_R(reg_atc, CORAL_ATC_ADDR_OFFSET, CORAL_ATC_ADDR_LENGTH);
                /* Get banks */
                banks = BITS_OFF_R(reg_atc, CORAL_ATC_ENTRY_HIT_OFFSET, CORAL_ATC_ENTRY_HIT_LENGTH);
                DIAG_PRINT(HAL_DBG_INFO, "reg_atc=(0x%x), addr=(0x%x), banks=(0x%x)\n", reg_atc, addr, banks);
            }

            if (AIR_E_OK != rc)
            {
                break;
            }
        }

        HAL_CORAL_L2_FDB_UNLOCK(unit);
    }

    if (AIR_E_OK == rc)
    {
        *ptr_cnt = cnt;
    }

    return rc;
}

/* FUNCTION NAME:   hal_coral_ipmc_setPortLookupIpTypeCtrl
 * PURPOSE:
 *      This API is used to set which type of IP packets will be
 *      treated as IP Multicast for lookup purposes.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      type                     -- IP family type
 *                                  AIR_IP_TYPE_T
 *      enable                   -- Treated as IPMC or not
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_setPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    const BOOL_T        enable)
{
    UI32_T u32dat = 0, cfg_val = 0;
    UI32_T mac_port = 0;

    /* get the MAC port index */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /* get the original register value */
    aml_readReg(unit, CORAL_PIC(mac_port), &u32dat, sizeof(UI32_T));
    /* set the config value by IP family type */
    if (AIR_IP_TYPE_IPV4 == ip_type)
    {
        cfg_val = (CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_01 | CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        cfg_val = CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_33;
    }
    /* set the enabled value */
    if (TRUE == enable)
    {
        u32dat |= cfg_val;
    }
    else
    {
        u32dat &= ~cfg_val;
    }
    DIAG_PRINT(HAL_DBG_INFO, "mac_port=(%u), ip_type=(%u), enable=(0x%u), cfg_val=(0x%x), u32dat=(0x%x)\n", mac_port,
               ip_type, enable, cfg_val, u32dat);
    aml_writeReg(unit, CORAL_PIC(mac_port), &u32dat, sizeof(UI32_T));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_coral_ipmc_getPortLookupIpTypeCtrl
 * PURPOSE:
 *      This API is used to get which type of IP packets will be
 *      treated as IP Multicast for lookup purposes.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      type                     -- IP family type
 *                                  AIR_IP_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- Treated as IPMC or not
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ipmc_getPortLookupIpTypeCtrl(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    BOOL_T             *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, cfg_val = 0;
    UI32_T         mac_port = 0;

    /* get the MAC port index */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /* get the original register value */
    aml_readReg(unit, CORAL_PIC(mac_port), &u32dat, sizeof(UI32_T));
    /* set the config value by IP family type */
    if (AIR_IP_TYPE_IPV4 == ip_type)
    {
        cfg_val = (CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_01 | CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        cfg_val = CORAL_PIC_PORT_IGMP_CTRL_CSR_IPM_33;
    }
    /* get the enabled state */
    if ((cfg_val & u32dat) == cfg_val)
    {
        *ptr_enable = TRUE;
    }
    else if ((cfg_val & u32dat) == 0)
    {
        *ptr_enable = FALSE;
    }
    else
    {
        rc = AIR_E_OTHERS;
    }
    DIAG_PRINT(HAL_DBG_INFO, "mac_port=(%u), ip_type=(%u), u32dat=(0x%x), cfg_val=(0x%x), ptr_enable=(%u), rc=(%u)\n",
               mac_port, ip_type, u32dat, cfg_val, *ptr_enable, rc);

    return rc;
}
