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

/* FILE NAME:  hal_sco_ipmc.c
 * PURPOSE:
 *  Implement Multicast module.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_ipmc.h>

#include <air_cfg.h>
#include <air_error.h>
#include <air_ipmc.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_util.h>
#include <hal/common/hal.h>
#include <hal/switch/sco/hal_sco_l2.h>
#include <hal/switch/sco/hal_sco_port.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, __ip__, __size__)                                                  \
    {                                                                                                          \
        UI32_T reversed_addr = 0, ip6addr = 0;                                                                 \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[0], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA7, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[4], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA5, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[8], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA3, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[12], __size__);                                        \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA1, &reversed_addr, __size__);                                                    \
    }

#define HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, __ip__, __size__)                                                 \
    {                                                                                                          \
        UI32_T reversed_addr = 0, ip6addr = 0;                                                                 \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[0], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA8, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[4], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA6, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[8], __size__);                                         \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA4, &reversed_addr, __size__);                                                    \
        osal_memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[12], __size__);                                        \
        reversed_addr =                                                                                        \
            (ip6addr >> 24) | ((ip6addr >> 8) & 0x0000FF00) | ((ip6addr << 8) & 0x00FF0000) | (ip6addr << 24); \
        aml_writeReg(unit, ATA2, &reversed_addr, __size__);                                                    \
    }

#define HAL_SCO_IPMC_U32_ENDIAN_XCHG(__ip__)                                                                \
    __ip__ = (__ip__ >> 24) | ((__ip__ >> 8) & 0x0000FF00) | ((__ip__ << 8) & 0x00FF0000) | (__ip__ << 24);

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_IPMC, "hal_sco_ipmc.c");
static BOOL_T _search_end = FALSE;

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*******************************************************************************/
/*  resource mangement api                                                     */
/*******************************************************************************/

/****************************** resource protection api *****************************/
/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

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
    UI32_T         i = 0;
    UI32_T         reg_atc = 0;

    /* Check BUSY bit is 0 */
    for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &reg_atc, sizeof(reg_atc));
        if (!BITS_OFF_R(reg_atc, ATC_BUSY_OFFSET, ATC_BUSY_LENGTH))
        {
            break;
        }
        osal_delayUs(1000);
    }
    if (i >= HAL_L2_MAX_BUSY_TIME)
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

/* FUNCTION NAME: _findDIPEntry
 * PURPOSE:
 *      Find DIP table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      vid             --  Specific VLAN ID
 *
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findDIPEntry(
    const UI32_T       unit,
    AIR_IPMC_TYPE_T    type,
    const UI32_T       gaddr,
    const UI32_T       vid,
    AIR_PORT_BITMAP_T *p_portmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            u32dat = 0, i = 0, banks = 0;
    AIR_PORT_BITMAP_T filter_mac_port_bitmap;

    aml_writeReg(unit, ATA1, &gaddr, sizeof(u32dat));
    u32dat |= (vid << 16);
    aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));

    u32dat = (HAL_CMD_READ | HAL_SAT_DIP | HAL_CMD_START);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

    /* Check timeout */
    if (AIR_E_OK == rc)
    {
        rc = _checkAtcBusy(unit, &u32dat);
    }

    if (AIR_E_OK == rc)
    {
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks(0x%x).\n", u32dat,
                   BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH), banks);
        if (banks)
        {
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                if (TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                    aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                    if (u32dat == gaddr)
                    {
                        /* Check vid and save the group member */
                        aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                        if (BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) == vid)
                        {
                            aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                            (*p_portmap)[0] |= u32dat;
                            HAL_AIR_PBMP_TO_MAC_PBMP(unit, HAL_PORT_BMP_ETH(unit), filter_mac_port_bitmap);
                            CMLIB_PORT_BITMAP_AND((*p_portmap), filter_mac_port_bitmap);
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        /* Source address mismatch, search next */
                        continue;
                    }
                }
            }
            if (HAL_SCO_L2_MAC_SET_NUM == i)
            {
                DIAG_PRINT(HAL_DBG_INFO, "gaddr=(0x%x), vid=(0x%x), entry not found.\n", gaddr, vid);
                rc = AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    return rc;
}

/* FUNCTION NAME: _findSIPEntry
 * PURPOSE:
 *      Find SIP table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      saddr           --  If not zero, will find specific source address
 *                          memeber based on specific group address
 *      vid             --  Specific VLAN ID
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findSIPEntry(
    const UI32_T       unit,
    const UI32_T       gaddr,
    const UI32_T       saddr,
    const UI32_T       vid,
    AIR_PORT_BITMAP_T *p_portmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            u32dat = 0, i = 0, addr = 0, banks = 0;
    BOOL_T            search_next = TRUE;
    AIR_PORT_BITMAP_T filter_mac_port_bitmap;

    aml_writeReg(unit, ATA1, &gaddr, sizeof(UI32_T));

    u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP_SIP_BY_DIP4 | HAL_CMD_START);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    AIR_PORT_BITMAP_CLEAR(*p_portmap);

    while (TRUE == search_next)
    {
        /* Check timeout */
        rc = _checkAtcBusy(unit, &u32dat);
        if (AIR_E_OK != rc)
        {
            search_next = FALSE;
        }

        if (AIR_E_OK == rc)
        {
            /* Get address */
            addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
            if (banks)
            {
                for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
                {
                    if (TRUE == BITS_OFF_R(banks, i, 1))
                    {
                        /* Select bank */
                        u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                        aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                        aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                        if (u32dat == saddr)
                        {
                            /* Save specific srouce ip of specific group member */
                            aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                            if (BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) == vid)
                            {
                                aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                                (*p_portmap)[0] |= u32dat;
                                HAL_AIR_PBMP_TO_MAC_PBMP(unit, HAL_PORT_BMP_ETH(unit), filter_mac_port_bitmap);
                                CMLIB_PORT_BITMAP_AND((*p_portmap), filter_mac_port_bitmap);
                                search_next = FALSE;
                                break;
                            }
                            /* the else case means the vid did not match, so search continues */
                        }
                        /* the else case means the source address did not match, so search continues */
                    }
                    /* the else case means this bank did not hit, so search continues */
                }
            }
            else
            {
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr) && (TRUE == search_next))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%u), return ENTRY_NOT_FOUND\n", addr);
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if (TRUE == search_next)
            {
                u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP_SIP_BY_DIP4 | HAL_CMD_START);
                aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: _findDIP6Entry
 * PURPOSE:
 *      Find DIP6 table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      vid             --  Specific VLAN ID
 *
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findDIP6Entry(
    const UI32_T        unit,
    AIR_IPMC_TYPE_T     type,
    const AIR_IP_ADDR_T gaddr,
    const UI32_T        vid,
    AIR_PORT_BITMAP_T  *p_portmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            u32dat = 0, bank = 0;
    C8_T              hit = FALSE;
    UI32_T            group_addr6[4];
    UI32_T            addr, banks;
    BOOL_T            search_next = TRUE;
    AIR_PORT_BITMAP_T filter_mac_port_bitmap;

    osal_memset(group_addr6, 0, sizeof(UI32_T) * 4);
    osal_memcpy(&group_addr6[0], &gaddr.ip_addr.ipv6_addr, sizeof(UI32_T) * 4);

    if (type == AIR_IPMC_TYPE_GRP_SRC)
    {
        HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, gaddr, sizeof(UI32_T));
        u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP_SIP_BY_DIP6 | HAL_CMD_START);
    }
    else
    {
        u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP6 | HAL_CMD_START);
    }
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    AIR_PORT_BITMAP_CLEAR(*p_portmap);

    while (TRUE == search_next)
    {
        /* Check timeout */
        rc = _checkAtcBusy(unit, &u32dat);
        if (AIR_E_OK != rc)
        {
            search_next = FALSE;
        }

        if (AIR_E_OK == rc)
        {
            /* Get address */
            addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
            if (banks == 0xF) /*IPv6 occupy all banks*/
            {
                aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                if (BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) == vid)
                {
                    for (bank = 0; bank < HAL_SCO_L2_MAC_SET_NUM; bank++)
                    {
                        aml_writeReg(unit, ATRDS, &bank, sizeof(bank));
                        aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                        HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                        if (u32dat == group_addr6[HAL_SCO_L2_MAC_SET_NUM - (bank + 1)])
                        {
                            hit++;
                        }
                        else
                        {
                            /* Source address mismatch, search next */
                            break;
                        }
                    }

                    if (hit == HAL_SCO_L2_MAC_SET_NUM)
                    {
                        aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                        (*p_portmap)[0] |= u32dat;
                        HAL_AIR_PBMP_TO_MAC_PBMP(unit, HAL_PORT_BMP_ETH(unit), filter_mac_port_bitmap);
                        CMLIB_PORT_BITMAP_AND((*p_portmap), filter_mac_port_bitmap);
                        search_next = FALSE;
                    }
                    /* the else case means the multicast address did not match, so search continues */
                }
                /* the else case means the vid did not match, so search continues */
            }
            else
            {
                /* the else case means when looking up IPMC table using DIP6 or DIPSIP6, the number of banks in not 4.
                   It should trigger an error. */
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if (((HAL_SCO_L2_MAX_ADDR_NUM - 1) == addr) && (TRUE == search_next))
            {
                /* if the last entry is reached and search_next is TRUE, it means we need to stop searching.*/
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if (TRUE == search_next)
            {
                u32dat = (HAL_CMD_SEARCH_NEXT |
                          ((type == AIR_IPMC_TYPE_GRP_SRC) ? HAL_MAT_DIP_SIP_BY_DIP6 : HAL_MAT_DIP6) | HAL_CMD_START) &
                         BITS_RANGE(0, 32);
                aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: _findSIP6Entry
 * PURPOSE:
 *      Find SIP6 table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      saddr           --  If not zero, will find specific source address
 *                          memeber based on specific group address
 *      vid             --  Specific VLAN ID
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findSIP6Entry(
    const UI32_T        unit,
    const AIR_IP_ADDR_T gaddr,
    const AIR_IP_ADDR_T saddr,
    const UI32_T        vid,
    AIR_PORT_BITMAP_T  *p_portmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            u32dat = 0, bank = 0;
    C8_T              hit = 0;
    UI32_T            group_addr6[4];
    UI32_T            source_addr6[4];
    UI32_T            addr, banks;
    BOOL_T            search_next = TRUE;
    AIR_PORT_BITMAP_T filter_mac_port_bitmap;

    osal_memset(group_addr6, 0, sizeof(UI32_T) * 4);
    osal_memset(source_addr6, 0, sizeof(UI32_T) * 4);

    osal_memcpy(&group_addr6[0], &gaddr.ip_addr.ipv6_addr, sizeof(UI32_T) * 4);
    osal_memcpy(&source_addr6[0], &saddr.ip_addr.ipv6_addr, sizeof(UI32_T) * 4);
    HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, gaddr, sizeof(UI32_T));

    u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP_SIP_BY_DIP6 | HAL_CMD_START);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    AIR_PORT_BITMAP_CLEAR(*p_portmap);

    while (TRUE == search_next)
    {
        /* Check timeout */
        rc = _checkAtcBusy(unit, &u32dat);
        if (AIR_E_OK != rc)
        {
            search_next = FALSE;
        }

        if (AIR_E_OK == rc)
        {
            /* Get address */
            addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
            if (banks == 0xF) /*IPv6 occupy all banks*/
            {
                aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                if (BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) == vid)
                {
                    if (!(CMLIB_UTIL_IP_ADDR_IS_ZERO(saddr)))
                    {
                        for (bank = 0; bank < HAL_SCO_L2_MAC_SET_NUM; bank++)
                        {
                            aml_writeReg(unit, ATRDS, &bank, sizeof(bank));
                            aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                            HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                            if (u32dat == source_addr6[HAL_SCO_L2_MAC_SET_NUM - (bank + 1)])
                            {
                                hit++;
                            }
                            else
                            {
                                /* Source address mismatch, search next */
                                break;
                            }
                        }

                        if (hit == HAL_SCO_L2_MAC_SET_NUM)
                        {
                            aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                            (*p_portmap)[0] |= u32dat;
                            HAL_AIR_PBMP_TO_MAC_PBMP(unit, HAL_PORT_BMP_ETH(unit), filter_mac_port_bitmap);
                            CMLIB_PORT_BITMAP_AND((*p_portmap), filter_mac_port_bitmap);
                            search_next = FALSE;
                        }
                        /* the else case means the multicast address did not match, so search continues */
                    }
                    else
                    {
                        /* Save match group address member */
                        aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                        (*p_portmap)[0] |= u32dat;
                        HAL_AIR_PBMP_TO_MAC_PBMP(unit, HAL_PORT_BMP_ETH(unit), filter_mac_port_bitmap);
                        CMLIB_PORT_BITMAP_AND((*p_portmap), filter_mac_port_bitmap);
                    }
                }
                /* the else case means the vid did not match, so search continues */
            }
            else
            {
                /* the else case means when looking up IPMC table using DIPSIP6, the number of banks in not 4.
                   It should trigger an error. */
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if (((HAL_SCO_L2_MAX_ADDR_NUM - 1) == addr) && (TRUE == search_next))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                search_next = FALSE;
                rc = AIR_E_ENTRY_NOT_FOUND;
            }

            if (TRUE == search_next)
            {
                u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP_SIP_BY_DIP6 | HAL_CMD_START);
                aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_sco_ipmc_setMcastLookupType
 * PURPOSE:
 *      This API is used to set multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *      type            --  The multicast type
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_setMcastLookupType(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    if (AIR_IPMC_TYPE_GRP_SRC == type)
    {
        /* set igmpv3 */
        u32dat |= ISC_CSR_IGMPV3_EN;
    }
    else if (AIR_IPMC_TYPE_GRP == type)
    {
        /* set igmpv2 */
        u32dat = 0;
    }
    else if (0 <= type && AIR_IPMC_TYPE_LAST > type)
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        aml_writeReg(unit, ISC, &u32dat, sizeof(UI32_T));
    }

    return rc;
}

/* FUNCTION NAME:   hal_sco_ipmc_getMcastLookupType
 * PURPOSE:
 *      This API is used to get multicast type.
 * INPUT:
 *      unit            --  Device unit number
 *
 * OUTPUT:
 *      ptr_type        --  The multicast type
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getMcastLookupType(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type)
{
    UI32_T u32dat = 0;
    aml_readReg(unit, ISC, &u32dat, sizeof(UI32_T));

    if (u32dat & ISC_CSR_IGMPV3_EN)
    {
        *ptr_type = AIR_IPMC_TYPE_GRP_SRC;
    }
    else
    {
        *ptr_type = AIR_IPMC_TYPE_GRP;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_addMcastAddr
 * PURPOSE:
 *      Add or set a multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_EXISTS
 *
 * NOTES:
 *      1.Can't add group if the group address is exist.
 */
AIR_ERROR_NO_T
hal_sco_ipmc_addMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    UI32_T            u32dat = 0, i = 0, banks = 0, port = 0;
    AIR_PORT_BITMAP_T air_portmap, p_portmap, curr_portmap;
    AIR_ERROR_NO_T    ret = 0;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    /* Check group address*/
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_BITMAP_CLEAR(air_portmap);
    AIR_PORT_BITMAP_CLEAR(p_portmap);
    AIR_PORT_BITMAP_CLEAR(curr_portmap);

    AIR_PORT_BITMAP_COPY(air_portmap, ptr_entry->port_bitmap);
    CMLIB_PORT_BITMAP_AND(air_portmap, HAL_PORT_BMP_ETH(unit));
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, air_portmap, p_portmap);

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        if (_findSIPEntry(unit, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->source_addr.ip_addr.ipv4_addr,
                          ptr_entry->vid, &curr_portmap) == AIR_E_ENTRY_NOT_FOUND)
        {
            ret = _findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid,
                                &curr_portmap);
            if ((ret == AIR_E_ENTRY_NOT_FOUND) || (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC))
            {
                /* Create new entry or update v3 group filter */
                /* Set entry type DIP */
                u32dat = 0;
                u32dat |= (TRUE << ATWD_IPM_VLD_OFFSET);
                /* Set attributes */
                if (ptr_entry->flags & AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
                {
                    u32dat |= (1UL << ATWD_IPM_LEAKY_OFFSET);
                }
                u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
                aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));

                /* Set member ports of IPMC entry */
                u32dat = 0;
                AIR_PORT_FOREACH(p_portmap, port)
                {
                    u32dat |= (1 << port);
                }
                aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

                /* Set DIP address */
                aml_writeReg(unit, ATA1, &ptr_entry->group_addr.ip_addr.ipv4_addr, sizeof(u32dat));

                /* For IGMPv3 write source ip table */
                if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
                    (AIR_IPV4_ZERO != ptr_entry->source_addr.ip_addr.ipv4_addr))
                {
                    /* Set source address */
                    aml_writeReg(unit, ATA2, &ptr_entry->source_addr.ip_addr.ipv4_addr, sizeof(u32dat));
                }

                /* Write DIP_SIP  table */
                if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
                {
                    u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
                }
                else
                {
                    u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
                }
                aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
                /* Check write state */
                for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
                {
                    aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
                    if (FALSE == (u32dat & HAL_TABLE_BUSY))
                    {
                        break;
                    }
                    osal_delayUs(1000);
                }
                if (i == HAL_L2_MAX_BUSY_TIME)
                {
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_TIMEOUT;
                }
                /* Get banks */
                banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
                if (banks)
                {
                    DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                               BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH), banks);
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                               BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH), banks);
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_OTHERS;
                }
            }
            else
            {
                /* IGMPv2 group already exist */
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_EXISTS;
            }
        }
        else
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_EXISTS;
        }
    }
    else
    {
        if (_findSIP6Entry(unit, ptr_entry->group_addr, ptr_entry->source_addr, ptr_entry->vid, &curr_portmap) ==
            AIR_E_ENTRY_NOT_FOUND)
        {
            ret = _findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &curr_portmap);
            if ((ret == AIR_E_ENTRY_NOT_FOUND) || (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC))
            {
                /* Create new entry or update v3 group filter */
                /* Set entry type DIP */
                u32dat = 0;
                u32dat |= (TRUE << ATWD_IPM_VLD_OFFSET);
                u32dat |= (TRUE << ATWD_IPM_IPV6_OFFSET);
                /* Set attributes */
                if (ptr_entry->flags & AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
                {
                    u32dat |= (1UL << ATWD_IPM_LEAKY_OFFSET);
                }
                u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
                aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));

                /* Set member ports of IPMC entry */
                u32dat = 0;
                AIR_PORT_FOREACH(p_portmap, port)
                {
                    u32dat |= (1 << port);
                }
                aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

                /* Set DIP address */
                HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

                /* For IGMPv3 write source ip table */
                if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) && !(CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr)))
                {
                    /* Set SIP address */
                    HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, ptr_entry->source_addr, sizeof(u32dat));
                }

                /* Write DIP_SIP  table */
                if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
                {
                    u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
                }
                else
                {
                    u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
                }
                aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
                /* Check write state */
                for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
                {
                    aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
                    if (FALSE == (u32dat & HAL_TABLE_BUSY))
                    {
                        break;
                    }
                    osal_delayUs(1000);
                }
                if (i == HAL_L2_MAX_BUSY_TIME)
                {
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_TIMEOUT;
                }
                /* Get banks */
                banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
                if (banks)
                {
                    DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                               BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH), banks);
                }
                else
                {
                    DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat,
                               BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH), banks);
                    HAL_SCO_L2_FDB_UNLOCK(unit);
                    return AIR_E_OTHERS;
                }
            }
            else
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_EXISTS;
            }
        }
        else
        {
            /* IGMPv3 group already exist*/
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_EXISTS;
        }
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_getMcastAddr
 * PURPOSE:
 *      Get multicast address entry for specific group and source address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      ptr_entry       --  The multicast entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getMcastAddr(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    UI32_T            u32dat = 0, atwd = 0, i = 0, addr = 0, banks = 0, port = 0;
    AIR_PORT_BITMAP_T portmap;
    UI16_T            vid = 0;
    AIR_IP_ADDR_T     group_addr, source_addr;
    UI32_T            mac_port = 0;

    AIR_PORT_BITMAP_CLEAR(portmap);
    osal_memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));
    osal_memset(&source_addr, 0, sizeof(AIR_IP_ADDR_T));

    if ((CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr)) && (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Search DIP_SIP table */
        atwd |= (((UI32_T)ptr_entry->vid) << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        aml_writeReg(unit, ATA1, &ptr_entry->group_addr.ip_addr.ipv4_addr, sizeof(u32dat));

        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            /* Search SIP table based on specific group addr */
            if (ptr_entry->source_addr.ip_addr.ipv4_addr != AIR_IPV4_ZERO)
            {
                aml_writeReg(unit, ATA2, &ptr_entry->source_addr.ip_addr.ipv4_addr, sizeof(u32dat));
            }
            else
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_BAD_PARAMETER;
            }
            u32dat = (HAL_CMD_READ | HAL_SAT_SIP | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_CMD_READ | HAL_SAT_DIP | HAL_CMD_START);
        }

        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        /* Check timeout */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }

        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks)
        {
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                if (TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                    vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if (ptr_entry->vid != vid)
                    {
                        continue;
                    }
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                    group_addr.ipv4 = TRUE;
                    group_addr.ip_addr.ipv4_addr = u32dat;
                    if (AIR_IPV4_ZERO != ptr_entry->group_addr.ip_addr.ipv4_addr)
                    {
                        if (ptr_entry->group_addr.ip_addr.ipv4_addr != group_addr.ip_addr.ipv4_addr)
                        {
                            continue;
                        }
                    }

                    if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
                    {
                        aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                        source_addr.ipv4 = TRUE;
                        source_addr.ip_addr.ipv4_addr = u32dat;
                        if (AIR_IPV4_ZERO != ptr_entry->source_addr.ip_addr.ipv4_addr)
                        {
                            if (ptr_entry->source_addr.ip_addr.ipv4_addr != u32dat)
                            {
                                /* source addr mismatch, find next */
                                continue;
                            }
                        }
                    }
                    /* group address match */
                    aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
                    {
                        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                        if (u32dat & (0x01 << mac_port))
                        {
                            AIR_PORT_ADD(portmap, port);
                        }
                    }
                    AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
                    DIAG_PRINT(HAL_DBG_INFO, "addr=0x%x, bank=0x%x\n", addr, i);
                }
            }
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
            }
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        /* Search DIP_SIP table */
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            /* Search SIP table based on specific group addr */
            if (!(CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr)))
            {
                HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, ptr_entry->source_addr, sizeof(u32dat));
            }
            else
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_BAD_PARAMETER;
            }
            u32dat = (HAL_CMD_READ | HAL_SAT_SIP | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_CMD_READ | HAL_SAT_DIP | HAL_CMD_START);
        }

        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        /* Check timeout */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }

        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks == 0xF) /*IPv6 occupy all banks*/
        {
            aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
            vid = BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                /* Select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                group_addr.ipv4 = FALSE;
                osal_memcpy(&group_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat,
                            sizeof(u32dat));
                if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
                {
                    aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                    HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                    source_addr.ipv4 = FALSE;
                    osal_memcpy(&source_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat,
                                sizeof(u32dat));
                }
            }
            /* group address match */
            aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
            AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
            {
                HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                if (u32dat & (0x01 << mac_port))
                {
                    AIR_PORT_ADD(portmap, port);
                }
            }
            AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
            DIAG_PRINT(HAL_DBG_INFO, "addr=0x%x, bank=0x%x\n", addr, 0xf);
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
            }
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_delMcastAddr
 * PURPOSE:
 *      Delete multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      1.del group address will delete group address relate all source address.
 */
AIR_ERROR_NO_T
hal_sco_ipmc_delMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;

    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Set group address */
        aml_writeReg(unit, ATA1, &ptr_entry->group_addr.ip_addr.ipv4_addr, sizeof(u32dat));

        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            aml_writeReg(unit, ATA2, &ptr_entry->source_addr.ip_addr.ipv4_addr, sizeof(u32dat));
        }

        /* Set DIP STATUS = 0 */
        u32dat = 0;
        u32dat |= (FALSE << ATWD_IPM_VLD_OFFSET);
        u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }
    else
    {
        /* Set group address */
        HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, ptr_entry->source_addr, sizeof(u32dat));
        }

        /* Set DIP STATUS = 0 */
        u32dat = 0;
        u32dat |= (FALSE << ATWD_IPM_VLD_OFFSET);
        u32dat |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }

    if (!BITS_OFF_R(u32dat, ATC_SINGLE_HIT_OFFSET, ATC_SINGLE_HIT_LENGTH))
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_delAllMcastAddr
 * PURPOSE:
 *      Delete all multicast address entry.
 *
 * INPUT:
 *      unit            --  Select device ID
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
AIR_ERROR_NO_T
hal_sco_ipmc_delAllMcastAddr(
    const UI32_T unit)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;

    HAL_SCO_L2_FDB_LOCK(unit);
    /* Clear all SIP entry */
    u32dat = (HAL_CMD_CLEAN | HAL_MAT_DIP_SIP | HAL_CMD_START);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    /* Check write state */
    for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
        if (FALSE == (u32dat & HAL_TABLE_BUSY))
        {
            break;
        }
        osal_delayUs(1000);
    }
    if (i == HAL_L2_MAX_BUSY_TIME)
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    /* Clear all DIP entry*/
    u32dat = (HAL_CMD_CLEAN | HAL_MAT_DIP | HAL_CMD_START);
    aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
    /* Check write state */
    for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
        if (FALSE == (u32dat & HAL_TABLE_BUSY))
        {
            break;
        }
        osal_delayUs(1000);
    }
    if (i == HAL_L2_MAX_BUSY_TIME)
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_TIMEOUT;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_addMcastMember
 * PURPOSE:
 *      Add member for a specific multicast entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
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
hal_sco_ipmc_addMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    UI32_T            u32dat = 0, atwd = 0, i = 0, port = 0;
    AIR_PORT_BITMAP_T map, portmask, p_portmap, mt_portmap, st_portmap;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    AIR_PORT_BITMAP_CLEAR(map);
    AIR_PORT_BITMAP_CLEAR(portmask);
    AIR_PORT_BITMAP_CLEAR(p_portmap);
    AIR_PORT_BITMAP_CLEAR(mt_portmap);
    AIR_PORT_BITMAP_CLEAR(st_portmap);

    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_entry->port_bitmap, p_portmap);
    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Check source address*/
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Check group exist or not */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            if (_findSIPEntry(unit, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->source_addr.ip_addr.ipv4_addr,
                              ptr_entry->vid, &st_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else if (ptr_entry->type == AIR_IPMC_TYPE_GRP)
        {
            if (_findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid,
                              &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if (ptr_entry->type != AIR_IPMC_TYPE_GRP && ptr_entry->type != AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for (i = 0; i < (HAL_SCO_MAX_NUM_OF_PORTS - 1); i++)
        {
            AIR_PORT_ADD(portmask, i);
        }

        /* Set DIP table to new member */
        aml_writeReg(unit, ATA1, &ptr_entry->group_addr.ip_addr.ipv4_addr, sizeof(u32dat));

        /* Assign new member*/
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            /* Add member to SIP/DIP specitic entry */
            aml_writeReg(unit, ATA2, &ptr_entry->source_addr.ip_addr.ipv4_addr, sizeof(u32dat));
            AIR_PORT_FOREACH(st_portmap, port)
            {
                AIR_PORT_ADD(p_portmap, port);
            }

            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(p_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }
        else
        {
            /* Add member to DIP specitic group entry */
            AIR_PORT_FOREACH(mt_portmap, port)
            {
                AIR_PORT_ADD(p_portmap, port);
            }

            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(p_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        u32dat = 0;
        AIR_PORT_FOREACH(map, port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if ((u32dat & HAL_TABLE_BUSY) == 0)
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }
    else
    {
        /* Check group exist or not */
        if (_findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &mt_portmap) ==
            AIR_E_ENTRY_NOT_FOUND)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }

        /* V3 check source addr */
        if (AIR_IPMC_TYPE_GRP_SRC == ptr_entry->type)
        {
            if (_findSIP6Entry(unit, ptr_entry->group_addr, ptr_entry->source_addr, ptr_entry->vid, &st_portmap) ==
                AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if (ptr_entry->type != AIR_IPMC_TYPE_GRP && ptr_entry->type != AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for (i = 0; i < (HAL_SCO_MAX_NUM_OF_PORTS - 1); i++)
        {
            AIR_PORT_ADD(portmask, i);
        }

        /* Set DIP table to new member */
        HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        /* Assign new member*/
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            /* Add member to SIP/DIP specitic entry */
            HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, ptr_entry->source_addr, sizeof(u32dat));
            AIR_PORT_FOREACH(st_portmap, port)
            {
                AIR_PORT_ADD(p_portmap, port);
            }

            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(p_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }
        else
        {
            /* Add member to DIP specitic group entry */
            AIR_PORT_FOREACH(mt_portmap, port)
            {
                AIR_PORT_ADD(p_portmap, port);
            }

            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(p_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        u32dat = 0;
        AIR_PORT_FOREACH(map, port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if ((u32dat & HAL_TABLE_BUSY) == 0)
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }
    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_delMcastMember
 * PURPOSE:
 *      Delete member for a specific multicast entry.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      None
 *
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
hal_sco_ipmc_delMcastMember(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    UI32_T            u32dat = 0, atwd = 0, i = 0, port = 0;
    AIR_PORT_BITMAP_T portmask, map, p_portmap, mt_portmap, st_portmap;

    HAL_CHECK_PORT_BITMAP(unit, ptr_entry->port_bitmap);

    AIR_PORT_BITMAP_CLEAR(portmask);
    AIR_PORT_BITMAP_CLEAR(map);
    AIR_PORT_BITMAP_CLEAR(p_portmap);
    AIR_PORT_BITMAP_CLEAR(mt_portmap);
    AIR_PORT_BITMAP_CLEAR(st_portmap);

    HAL_AIR_PBMP_TO_MAC_PBMP(unit, ptr_entry->port_bitmap, p_portmap);

    /* Check group address */
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Check source address*/
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Check group exist or not */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            if (_findSIPEntry(unit, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->source_addr.ip_addr.ipv4_addr,
                              ptr_entry->vid, &st_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else if (ptr_entry->type == AIR_IPMC_TYPE_GRP)
        {
            if (_findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid,
                              &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if (ptr_entry->type != AIR_IPMC_TYPE_GRP && ptr_entry->type != AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for (i = 0; i < (HAL_SCO_MAX_NUM_OF_PORTS - 1); i++)
        {
            AIR_PORT_ADD(portmask, i);
        }

        /* Set DIP table to new member */
        aml_writeReg(unit, ATA1, &ptr_entry->group_addr.ip_addr.ipv4_addr, sizeof(u32dat));

        /* Assign new member*/
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            aml_writeReg(unit, ATA2, &ptr_entry->source_addr.ip_addr.ipv4_addr, sizeof(u32dat));

            /* Delete specific member */
            AIR_PORT_FOREACH(p_portmap, port)
            {
                AIR_PORT_DEL(st_portmap, port);
            }
            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(st_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }
        else
        {
            /* Delete member to DIP specitic group entry */
            AIR_PORT_FOREACH(p_portmap, port)
            {
                AIR_PORT_DEL(mt_portmap, port);
            }
            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(mt_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        u32dat = 0;
        AIR_PORT_FOREACH(map, port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if ((u32dat & HAL_TABLE_BUSY) == 0)
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }
    else
    {
        /* Check group exist or not */
        if (_findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &mt_portmap) ==
            AIR_E_ENTRY_NOT_FOUND)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_ENTRY_NOT_FOUND;
        }

        /* V3 check source addr */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            if (_findSIP6Entry(unit, ptr_entry->group_addr, ptr_entry->source_addr, ptr_entry->vid, &st_portmap) ==
                AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if (ptr_entry->type != AIR_IPMC_TYPE_GRP && ptr_entry->type != AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for (i = 0; i < (HAL_SCO_MAX_NUM_OF_PORTS - 1); i++)
        {
            AIR_PORT_ADD(portmask, i);
        }

        /* Set DIP table to new member */
        HAL_SCO_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        /* Assign new member*/
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            HAL_SCO_IPMC_WRITE_IPV6_SOURCE(unit, ptr_entry->source_addr, sizeof(u32dat));

            /* Delete specific member */
            AIR_PORT_FOREACH(p_portmap, port)
            {
                AIR_PORT_DEL(st_portmap, port);
            }
            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(st_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }
        else
        {
            /* Delete member to DIP specitic group entry */
            AIR_PORT_FOREACH(p_portmap, port)
            {
                AIR_PORT_DEL(mt_portmap, port);
            }
            AIR_PORT_FOREACH(portmask, port)
            {
                if (AIR_PORT_CHK(mt_portmap, port))
                {
                    AIR_PORT_ADD(map, port);
                }
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, &atwd, sizeof(atwd));
        u32dat = 0;
        AIR_PORT_FOREACH(map, port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, &u32dat, sizeof(u32dat));

        /* Write DIP_SIP table */
        if (ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC)
        {
            u32dat = (HAL_SAT_SIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        else
        {
            u32dat = (HAL_SAT_DIP | HAL_CMD_WRITE | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));
        /* Check write state */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if ((u32dat & HAL_TABLE_BUSY) == 0)
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }
    }
    HAL_SCO_L2_FDB_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_getMcastMemberCnt
 * PURPOSE:
 *      Get member count for specific multicast group.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_entry       --  AIR_IPMC_ENTRY_T
 *
 * OUTPUT:
 *      ptr_count       --  Member count
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getMcastMemberCnt(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_count)
{
    UI32_T            u32dat = 0;
    AIR_PORT_BITMAP_T mt_portmap;

    AIR_PORT_BITMAP_CLEAR(mt_portmap);

    /* Check group address*/
    if (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->group_addr) || !HAL_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Check source address */
    if ((ptr_entry->type == AIR_IPMC_TYPE_GRP_SRC) &&
        (CMLIB_UTIL_IP_ADDR_IS_ZERO(ptr_entry->source_addr) || HAL_L3_IP_IS_MULTICAST(&ptr_entry->source_addr)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (ptr_entry->group_addr.ipv4 == TRUE)
    {
        if (_findSIPEntry(unit, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->source_addr.ip_addr.ipv4_addr,
                          ptr_entry->vid, &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
        {
            if (_findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid,
                              &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Counter port map counter */
        CMLIB_PORT_BITMAP_COUNT(mt_portmap, u32dat);
        *ptr_count = u32dat;
    }
    else
    {
        if (_findSIP6Entry(unit, ptr_entry->group_addr, ptr_entry->source_addr, ptr_entry->vid, &mt_portmap) ==
            AIR_E_ENTRY_NOT_FOUND)
        {
            if (_findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &mt_portmap) ==
                AIR_E_ENTRY_NOT_FOUND)
            {
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Counter port map counter */
        CMLIB_PORT_BITMAP_COUNT(mt_portmap, u32dat);
        *ptr_count = u32dat;
    }
    HAL_SCO_L2_FDB_UNLOCK(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ipmc_getMcastBucketSize
 * PURPOSE:
 *      Get the bucket size of one multicast address set when searching multicast.
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
hal_sco_ipmc_getMcastBucketSize(
    const UI32_T unit,
    UI32_T      *ptr_size)
{
    /* Access regiser */
    (*ptr_size) = HAL_SCO_L2_MAC_SET_NUM;

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ipmc_getFirstMcastAddr
 * PURPOSE:
 *      This API is used to get a the first multicast address entry.
 *
 * INPUT:
 *      unit            --  Device unit number
 *      match_type      --  The type to search multicast entry
 *
 * OUTPUT:
        ptr_entry_cnt   --  The number of multicast address entries
 *      ptr_entry       --  The multicast entry
 *
 * RETURN:
 *      AIR_E_OK        --  Operation succeeded.
 *      Others          --  Operation failed.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getFirstMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    UI32_T            u32dat = 0, i = 0, port = 0;
    UI32_T            addr = 0, banks = 0;
    AIR_PORT_BITMAP_T portmap;
    UI32_T            mac_port = 0;

    AIR_PORT_BITMAP_CLEAR(portmap);
    *ptr_entry_cnt = 0;

    HAL_SCO_L2_FDB_LOCK(unit);
    _search_end = FALSE;

    if (AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type || AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC == match_type)
    {
        if (AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
        {
            u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP4 | HAL_CMD_START);
        }
        else /* AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC */
        {
            u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP_SIP_IPV4 | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        /* Check timeout */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks)
        {
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                AIR_PORT_BITMAP_CLEAR(portmap);
                if (TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                    ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                    ptr_entry->group_addr.ipv4 = TRUE;
                    ptr_entry->group_addr.ip_addr.ipv4_addr = u32dat;

                    if (AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC == match_type)
                    {
                        aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                        ptr_entry->type = AIR_IPMC_TYPE_GRP_SRC;
                        ptr_entry->source_addr.ipv4 = TRUE;
                        ptr_entry->source_addr.ip_addr.ipv4_addr = u32dat;
                    }

                    aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
                    {
                        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                        if (u32dat & (0x01 << mac_port))
                        {
                            AIR_PORT_ADD(portmap, port);
                        }
                    }
                    AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
                    (*ptr_entry_cnt)++;
                    ptr_entry++;
                }
            }
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr) && (*ptr_entry_cnt) == 0)
            {
                DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else if (AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type || AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC == match_type)
    {
        if (AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
        {
            u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP6 | HAL_CMD_START);
        }
        else /* AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC */
        {
            u32dat = (HAL_CMD_SEARCH | HAL_MAT_DIP_SIP_IPV6 | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks == 0xF)
        {
            aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
            ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                /* select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                ptr_entry->group_addr.ipv4 = FALSE;
                osal_memcpy(&ptr_entry->group_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat,
                            sizeof(u32dat));
                if (AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC == match_type)
                {
                    aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                    HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                    ptr_entry->type = AIR_IPMC_TYPE_GRP_SRC;
                    ptr_entry->source_addr.ipv4 = FALSE;
                    osal_memcpy(&ptr_entry->source_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4],
                                &u32dat, sizeof(u32dat));
                }
            }
            aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
            AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
            {
                HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                if (u32dat & (0x01 << mac_port))
                {
                    AIR_PORT_ADD(portmap, port);
                }
            }
            AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
            (*ptr_entry_cnt)++;
            ptr_entry++;
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_ipmc_getNextMcastAddr
 * PURPOSE:
 *      Get next multicast source address entry for specific group and source address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      match_type      --  The type to search multicast entry
 *
 * OUTPUT:
 *      ptr_entry_cnt   --  The number of returned multicast entries
 *      ptr_entry       --  The multicast searching result.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getNextMcastAddr(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry)
{
    UI32_T            u32dat = 0, i = 0, port = 0;
    UI32_T            addr = 0, banks = 0;
    AIR_PORT_BITMAP_T portmap;
    UI32_T            mac_port = 0;

    AIR_PORT_BITMAP_CLEAR(portmap);
    *ptr_entry_cnt = 0;

    /* If found the lastest entry last time, we couldn't keep to search the next entry */
    if (TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    HAL_SCO_L2_FDB_LOCK(unit);
    if (AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type || AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC == match_type)
    {
        if (AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
        {
            u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP4 | HAL_CMD_START);
        }
        else /* AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC */
        {
            u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP_SIP_IPV4 | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        /* Check timeout */
        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == addr)
        {
            _search_end = TRUE;
        }
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks)
        {
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                AIR_PORT_BITMAP_CLEAR(portmap);
                if (TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
                    ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                    ptr_entry->group_addr.ipv4 = TRUE;
                    ptr_entry->group_addr.ip_addr.ipv4_addr = u32dat;

                    if (AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC == match_type)
                    {
                        aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                        ptr_entry->type = AIR_IPMC_TYPE_GRP_SRC;
                        ptr_entry->source_addr.ipv4 = TRUE;
                        ptr_entry->source_addr.ip_addr.ipv4_addr = u32dat;
                    }

                    aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
                    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
                    {
                        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                        if (u32dat & (0x01 << mac_port))
                        {
                            AIR_PORT_ADD(portmap, port);
                        }
                    }
                    AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
                    (*ptr_entry_cnt)++;
                    ptr_entry++;
                }
            }
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr) && (*ptr_entry_cnt) == 0)
            {
                DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else if (AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type || AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC == match_type)
    {
        if (AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
        {
            u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP6 | HAL_CMD_START);
        }
        else /* AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC */
        {
            u32dat = (HAL_CMD_SEARCH_NEXT | HAL_MAT_DIP_SIP_IPV6 | HAL_CMD_START);
        }
        aml_writeReg(unit, ATC, &u32dat, sizeof(u32dat));

        for (i = 0; i < HAL_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat, sizeof(u32dat));
            if (FALSE == (u32dat & HAL_TABLE_BUSY))
            {
                break;
            }
            osal_delayUs(1000);
        }
        if (i == HAL_L2_MAX_BUSY_TIME)
        {
            HAL_SCO_L2_FDB_UNLOCK(unit);
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == addr)
        {
            _search_end = TRUE;
        }
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        DIAG_PRINT(HAL_DBG_INFO, "u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
        if (banks == 0xF)
        {
            aml_readReg(unit, ATRD0, &u32dat, sizeof(u32dat));
            ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for (i = 0; i < HAL_SCO_L2_MAC_SET_NUM; i++)
            {
                /* select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, &u32dat, sizeof(u32dat));
                aml_readReg(unit, ATRD1, &u32dat, sizeof(u32dat));
                HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                ptr_entry->group_addr.ipv4 = FALSE;
                osal_memcpy(&ptr_entry->group_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4], &u32dat,
                            sizeof(u32dat));
                if (AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC == match_type)
                {
                    aml_readReg(unit, ATRD2, &u32dat, sizeof(u32dat));
                    HAL_SCO_IPMC_U32_ENDIAN_XCHG(u32dat);
                    ptr_entry->type = AIR_IPMC_TYPE_GRP_SRC;
                    ptr_entry->source_addr.ipv4 = FALSE;
                    osal_memcpy(&ptr_entry->source_addr.ip_addr.ipv6_addr[(HAL_SCO_L2_MAC_SET_NUM - (i + 1)) * 4],
                                &u32dat, sizeof(u32dat));
                }
            }
            aml_readReg(unit, ATRD3, &u32dat, sizeof(u32dat));
            AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
            {
                HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
                if (u32dat & (0x01 << mac_port))
                {
                    AIR_PORT_ADD(portmap, port);
                }
            }
            AIR_PORT_BITMAP_COPY(ptr_entry->port_bitmap, portmap);
            (*ptr_entry_cnt)++;
            ptr_entry++;
        }
        else
        {
            if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
            {
                DIAG_PRINT(HAL_DBG_INFO, "addr=(%d), return ENTRY_NOT_FOUND\n", addr);
                HAL_SCO_L2_FDB_UNLOCK(unit);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        HAL_SCO_L2_FDB_UNLOCK(unit);
        return AIR_E_BAD_PARAMETER;
    }

    HAL_SCO_L2_FDB_UNLOCK(unit);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ipmc_setPortIpmcMode
 * PURPOSE:
 *      This API is used to set IPMC mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 *      enable          --  The IPMC Mode
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_setPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    if (TRUE == enable)
    {
        /* Enable igmp snooping */
        u32dat |= (PIC_PORT_IGMP_CTRL_CSR_IPM_01 | PIC_PORT_IGMP_CTRL_CSR_IPM_33 | PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        /* Disable igmp snooping */
        u32dat = 0;
    }
    DIAG_PRINT(HAL_DBG_INFO, "mac_port=(%u), enable=(0x%u), u32dat=(0x%x)\n", mac_port, enable, u32dat);
    aml_writeReg(unit, PIC(mac_port), &u32dat, sizeof(UI32_T));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ipmc_getPortIpmcMode
 * PURPOSE:
 *      This API is used to get IGMP snooping mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  The configuring port
 *
 * OUTPUT:
 *      ptr_enable      --  The IGMP snooping mode refer to
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_ipmc_getPortIpmcMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PIC(mac_port), &u32dat, sizeof(UI32_T));
    if (u32dat == (PIC_PORT_IGMP_CTRL_CSR_IPM_01 | PIC_PORT_IGMP_CTRL_CSR_IPM_33 | PIC_PORT_IGMP_CTRL_CSR_IPM_224))
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

/* FUNCTION NAME: hal_sco_ipmc_getCapacity
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
hal_sco_ipmc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_IPMC_FDB == type)
    {
        *ptr_size = HAL_SCO_L2_MAX_ENTRY_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_ipmc_getUsage
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
hal_sco_ipmc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cnt = 0, i = 0;
    UI32_T         reg_atc = 0, addr = 0, banks = 0;
    UI32_T         mat[2] = {HAL_MAT_DIP, HAL_MAT_DIP_SIP};

    if (AIR_SWC_RSRC_IPMC_FDB != type)
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_SCO_L2_FDB_LOCK(unit);

        /* Search all IP entry */
        for (i = 0; i < 2; i++)
        {
            /* Search the 1st entry */
            reg_atc = (HAL_CMD_SEARCH | HAL_CMD_START | mat[i]);
            aml_writeReg(unit, ATC, &reg_atc, sizeof(reg_atc));
            /* Check write state */
            rc = _checkAtcBusy(unit, &reg_atc);
            /* Get address */
            addr = BITS_OFF_R(reg_atc, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(reg_atc, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "reg_atc=(0x%x), addr=(0x%x), banks=(0x%x)\n", reg_atc, addr, banks);
            while ((AIR_E_OK == rc) && (0 != banks))
            {
                while (banks != 0)
                {
                    cnt += banks & 1;
                    banks >>= 1;
                }
                if ((HAL_SCO_L2_MAX_ADDR_NUM - 1) == (addr))
                {
                    break;
                }
                /* Search the next entry*/
                reg_atc = (HAL_CMD_SEARCH_NEXT | HAL_CMD_START | mat[i]);
                aml_writeReg(unit, ATC, &reg_atc, sizeof(reg_atc));
                /* Check write state */
                rc = _checkAtcBusy(unit, &reg_atc);
                /* Get address */
                addr = BITS_OFF_R(reg_atc, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
                /* Get banks */
                banks = BITS_OFF_R(reg_atc, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
                DIAG_PRINT(HAL_DBG_INFO, "reg_atc=(0x%x), addr=(0x%x), banks=(0x%x)\n", reg_atc, addr, banks);
            }

            if (AIR_E_OK != rc)
            {
                break;
            }
        }

        HAL_SCO_L2_FDB_UNLOCK(unit);
    }

    if (AIR_E_OK == rc)
    {
        *ptr_cnt = cnt;
    }

    return rc;
}

/* FUNCTION NAME:   hal_sco_ipmc_setPortLookupIpTypeCtrl
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
hal_sco_ipmc_setPortLookupIpTypeCtrl(
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
    aml_readReg(unit, PIC(mac_port), &u32dat, sizeof(UI32_T));
    /* set the config value by IP family type */
    if (AIR_IP_TYPE_IPV4 == ip_type)
    {
        cfg_val = (PIC_PORT_IGMP_CTRL_CSR_IPM_01 | PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        cfg_val = PIC_PORT_IGMP_CTRL_CSR_IPM_33;
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
    aml_writeReg(unit, PIC(mac_port), &u32dat, sizeof(UI32_T));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_ipmc_getPortLookupIpTypeCtrl
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
hal_sco_ipmc_getPortLookupIpTypeCtrl(
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
    aml_readReg(unit, PIC(mac_port), &u32dat, sizeof(UI32_T));
    /* set the config value by IP family type */
    if (AIR_IP_TYPE_IPV4 == ip_type)
    {
        cfg_val = (PIC_PORT_IGMP_CTRL_CSR_IPM_01 | PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        cfg_val = PIC_PORT_IGMP_CTRL_CSR_IPM_33;
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
