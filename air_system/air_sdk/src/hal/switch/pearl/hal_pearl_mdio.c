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

/* FILE NAME:  hal_pearl_mdio.c
 * PURPOSE:
 *  Implement MDIO module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_mdio.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define WIDTH4_MSK  (0xF)
#define WIDTH5_MSK  (0x1F)
#define WIDTH16_MSK (0xFFFF)

#define PB_PHY_REG_BASE          (0xA0000000)
#define PB_PHY_ACCESS_BIT        (0x00800000)
#define PB_PHY_ADDR_OFS          (24)
#define PB_PHY_ADDR_MSK          (WIDTH5_MSK)
#define PB_PHY_ADDR_SEL(phyAddr) ((phyAddr & PB_PHY_ADDR_MSK) << PB_PHY_ADDR_OFS)
#define PB_PHY_PAGE_OFS          (12)
#define PB_PHY_PAGE_MSK          (WIDTH4_MSK)
#define PB_PHY_PAGE_SEL(page)    ((page & PB_PHY_PAGE_MSK) << PB_PHY_PAGE_OFS)
#define PB_CL22_MAIN_PAGE        (0x00000000)
#define PB_PHY_CL22_REG_OFS      (4)
#define PB_PHY_CL22_REG_MSK      (WIDTH5_MSK)
#define PB_PHY_REG_SEL(reg)      ((reg & WIDTH5_MSK) << PB_PHY_CL22_REG_OFS)

#define PB_PHY_CL45_DEV_OFS      (18)
#define PB_PHY_CL45_DEV_MSK      (WIDTH5_MSK)
#define PB_PHY_CL45_DEV_SEL(dev) ((dev & PB_PHY_CL45_DEV_MSK) << PB_PHY_CL45_DEV_OFS)
#define PB_PHY_CL45_REG_OFS      (2)
#define PB_PHY_CL45_REG_MSK      (WIDTH16_MSK)
#define PB_PHY_CL45_REG_SEL(reg) ((reg & PB_PHY_CL45_REG_MSK) << PB_PHY_CL45_REG_OFS)

#define IAC_MAX_BUSY_TIME (1000)
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_MDIO, "hal_pearl_mdio.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static AIR_ERROR_NO_T
_hal_pearl_mdio_checkAccessDone(const UI32_T unit);

/* STATIC VARIABLE DECLARATIONS */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_pearl_mdio_checkAccessDone
 * PURPOSE:
 *      wait for mdio access done
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_TIMEOUT   --  Operation timeout.
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_pearl_mdio_checkAccessDone(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         busy_time = 0, check_bit;

    while (1)
    {
        rv = aml_readReg(unit, PHY_IAC, &check_bit, 4);

        if (AIR_E_OK == rv)
        {
            if (!(check_bit & SET_MDIO_ACS_START))
            {
                break;
            }
            else if (busy_time >= IAC_MAX_BUSY_TIME)
            {
                rv = AIR_E_TIMEOUT;
                break;
            }
            else
            {
                busy_time++;
            }
        }
        else
        {
            break;
        }
    }

    return rv;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_pearl_mdio_writeC22
 * PURPOSE:
 *      Write the mdio data for IEEE clause 22
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      reg_addr        --  Register address
 *      reg_data        --  Register data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_OTHERS    --  Failed to write the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_writeC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         write_data = 0;

    write_data = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_DATA(reg_data) | (SET_MDIO_CMD_CL22W) |
                 (SET_MDIO_ST_CL22);
    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);

    if (AIR_E_OK == rv)
    {
        write_data |= SET_MDIO_ACS_START;
        rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
        if (AIR_E_OK == rv)
        {
            rv = _hal_pearl_mdio_checkAccessDone(unit);
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, write phy=0x%x reg=0x%x val=0x%x\n\n", unit, bus_id, phy_addr, reg_addr,
                   reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, write phy=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr, reg_addr,
                   rv);
    }

    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_readC22
 * PURPOSE:
 *      Read the mdio data for IEEE clause 22
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      reg_addr        --  Register address
 *
 * OUTPUT:
 *      ptr_reg_data    --  Data of mdio slave
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_OTHERS    --  Failed to read the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_readC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         write_data = 0, read_data;

    write_data = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_CMD_CL22R | SET_MDIO_ST_CL22;
    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);

    if (AIR_E_OK == rv)
    {
        write_data |= SET_MDIO_ACS_START;
        rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
        if (AIR_E_OK == rv)
        {
            rv = _hal_pearl_mdio_checkAccessDone(unit);
            if (AIR_E_OK == rv)
            {
                rv = aml_readReg(unit, PHY_IAC, &read_data, 4);
                if (AIR_E_OK == rv)
                {
                    *ptr_reg_data = GET_MDIO_DATA(read_data);
                }
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, read phy=0x%x reg=0x%x val=0x%x\n", unit, bus_id, phy_addr, reg_addr,
                   *ptr_reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, read phy=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr, reg_addr,
                   rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_writeC45
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      dev_type        --  Device type
 *      reg_addr        --  Register address
 *      reg_data        --  Register data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_OTHERS    --  Failed to write the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_writeC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         write_data = 0;

    /* phase1 set ctrl mode */
    write_data = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
                 (SET_MDIO_ST_CL45);
    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);

    if (AIR_E_OK == rv)
    {
        write_data |= SET_MDIO_ACS_START;
        rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
        if (AIR_E_OK == rv)
        {
            rv = _hal_pearl_mdio_checkAccessDone(unit);
            if (AIR_E_OK == rv)
            {
                /* phase2 set write mode & write data */
                write_data = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_data) |
                             (SET_MDIO_CMD_CL45W) | (SET_MDIO_ST_CL45);
                rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
                if (AIR_E_OK == rv)
                {
                    write_data |= SET_MDIO_ACS_START;
                    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
                    if (AIR_E_OK == rv)
                    {
                        rv = _hal_pearl_mdio_checkAccessDone(unit);
                    }
                }
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u write phy=0x%x dev=0x%x reg=0x%x val=0x%0x\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, write phy=0x%x dev=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, rv);
    }

    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_readC45
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      dev_type        --  Device type
 *      reg_addr        --  Register address
 *
 * OUTPUT:
 *      ptr_reg_data    --  Data of mdio slave
 *
 * RETURN:
 *      AIR_E_OK        --  Operation success.
 *      AIR_E_OTHERS    --  Failed to read the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_readC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         write_data = 0, read_data;

    /* phase1 set ctrl mode */
    write_data = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
                 (SET_MDIO_ST_CL45);
    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);

    if (AIR_E_OK == rv)
    {
        write_data |= SET_MDIO_ACS_START;
        rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
        if (AIR_E_OK == rv)
        {
            rv = _hal_pearl_mdio_checkAccessDone(unit);
            if (AIR_E_OK == rv)
            {
                /* phase2 set read mode */
                write_data =
                    SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | (SET_MDIO_CMD_CL45R) | (SET_MDIO_ST_CL45);
                rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
                if (AIR_E_OK == rv)
                {
                    write_data |= SET_MDIO_ACS_START;
                    rv = aml_writeReg(unit, PHY_IAC, &write_data, 4);
                    if (AIR_E_OK == rv)
                    {
                        rv = _hal_pearl_mdio_checkAccessDone(unit);
                        if (AIR_E_OK == rv)
                        {
                            /* phase3 read data */
                            rv = aml_readReg(unit, PHY_IAC, &read_data, 4);
                            if (AIR_E_OK == rv)
                            {
                                *ptr_reg_data = GET_MDIO_DATA(read_data);
                            }
                        }
                    }
                }
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, read phy=0x%x dev=0x%x reg=0x%x val=0x%x\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, *ptr_reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, read phy=0x%x dev=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_writeC22FromIntBus
 * PURPOSE:
 *      Write the mdio data for IEEE clause 22 from internal bus
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      page_sel        --  page select
 *      reg_addr        --  Register address
 *      reg_data        --  Register data
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_writeC22FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbusAddr = 0, writeData = 0;

    pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT | PB_PHY_ADDR_SEL(phy_addr) | PB_CL22_MAIN_PAGE |
                PB_PHY_PAGE_SEL(page_sel) | PB_PHY_REG_SEL(reg_addr));
    writeData = reg_data;

    rv = aml_writeReg(unit, pbusAddr, &writeData, 4);
    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, write phy=0x%x page=%u reg=0x%x val=0x%x\n", unit, bus_id, phy_addr,
                   page_sel, reg_addr, reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, write phy=0x%x page=%u reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   page_sel, reg_addr, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_readC22FromIntBus
 * PURPOSE:
 *      Read the mdio data for IEEE clause 22 from internal bus
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      page_sel        --  page select
 *      reg_addr        --  Register address
 *
 * OUTPUT:
 *      ptr_reg_data    -- Data of mdio slave
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_readC22FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbusAddr = 0, readData = 0;

    pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT | PB_PHY_ADDR_SEL(phy_addr) | PB_CL22_MAIN_PAGE |
                PB_PHY_PAGE_SEL(page_sel) | PB_PHY_REG_SEL(reg_addr));

    rv = aml_readReg(unit, pbusAddr, &readData, 4);
    if (AIR_E_OK == rv)
    {
        *ptr_reg_data = (UI16_T)readData;
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, read phy=0x%x page=%u reg=0x%x val=0x%x\n", unit, bus_id, phy_addr,
                   page_sel, reg_addr, *ptr_reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, read phy=0x%x page=%u reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   page_sel, reg_addr, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_writeC45FromIntBus
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45 from internal bus
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      dev_type        --  Device type
 *      reg_addr        --  Register address
 *      reg_data        --  Register data
 *
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_writeC45FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbusAddr = 0, writeData = 0;

    pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT | PB_PHY_ADDR_SEL(phy_addr) | PB_PHY_CL45_DEV_SEL(dev_type) |
                PB_PHY_CL45_REG_SEL(reg_addr));
    writeData = reg_data;

    rv = aml_writeReg(unit, pbusAddr, &writeData, 4);
    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, write phy=0x%x dev=0x%x reg=0x%x val=0x%x\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, write phy=0x%x dev=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_pearl_mdio_readC45FromIntBus
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45 from internal bus
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      phy_addr        --  PHY address
 *      dev_type        --  Device type
 *      reg_addr        --  Register address
 *
 * OUTPUT:
 *      ptr_reg_data    -- Data of mdio slave
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_mdio_readC45FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         pbusAddr = 0, readData = 0;

    pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT | PB_PHY_ADDR_SEL(phy_addr) | PB_PHY_CL45_DEV_SEL(dev_type) |
                PB_PHY_CL45_REG_SEL(reg_addr));

    rv = aml_readReg(unit, pbusAddr, &readData, 4);
    if (AIR_E_OK == rv)
    {
        *ptr_reg_data = (UI16_T)readData;
        DIAG_PRINT(HAL_DBG_INFO, "u=%u bus=%u, read phy=0x%x dev=0x%x reg=0x%x val=0x%x\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, *ptr_reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u bus=%u, read phy=0x%x dev=0x%x reg=0x%x failed rv=%d\n", unit, bus_id, phy_addr,
                   dev_type, reg_addr, rv);
    }
    return rv;
}