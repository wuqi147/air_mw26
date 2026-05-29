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

/* FILE NAME:  hal_sco_mdio.c
 * PURPOSE:
 *  Implement MDIO module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_mdio.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#ifdef AIR_EN_I2C_PHY
#include <hal/switch/sco/hal_sco_sif.h>
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_MDIO, "hal_sco_mdio.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
#define IAC_MAX_BUSY_TIME (1000)
#ifdef AIR_EN_I2C_PHY
#define I2C_CHANNEL_ID_1      (1)
#define I2C_MDIO_REG_DATA_LEN (4)
#define I2C_MDIO_REG_ADDR_LEN (4)
#endif

/* STATIC VARIABLE DECLARATIONS */

/* table/register control blocks */

/* EXPORTED SUBPROGRAM BODIES*/
/* FUNCTION NAME: hal_sco_mdio_writeC22
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
hal_sco_mdio_writeC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         wdata = 0;
    UI32_T         checkBit = 0, busytime = 0;

    DIAG_PRINT(HAL_DBG_INFO, " write_c22 unit(%u) bus(0x%X) phy(0x%X) reg(0x%X) val(0x%X)\n", unit, bus_id, phy_addr,
               reg_addr, reg_data);
    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_DATA(reg_data) | (SET_MDIO_CMD_CL22W) |
            (SET_MDIO_ST_CL22);
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    wdata |= SET_MDIO_ACS_START;
    aml_writeReg(unit, PHY_IAC, &wdata, 4);

    while (1)
    {
        rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
        if (AIR_E_OK == rv)
        {
            if (!(checkBit & SET_MDIO_ACS_START))
            {
                break;
            }
            else if (busytime >= IAC_MAX_BUSY_TIME)
            {
                rv = AIR_E_TIMEOUT;
                break;
            }
            busytime++;
        }
        else
        {
            break;
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) Write BusId[%u] PhyAddr[0x%x] Reg[0x%X] : 0x%08X\n", unit, bus_id, phy_addr,
                   reg_addr, reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_readC22
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
hal_sco_mdio_readC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         wdata = 0;
    UI32_T         checkBit = 0, busytime = 0;
    UI32_T         data = 0;

    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_CMD_CL22R | SET_MDIO_ST_CL22;
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    wdata |= SET_MDIO_ACS_START;
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    while (1)
    {
        rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
        if (AIR_E_OK == rv)
        {
            if (!(checkBit & SET_MDIO_ACS_START))
            {
                break;
            }
            else if (busytime >= IAC_MAX_BUSY_TIME)
            {
                rv = AIR_E_TIMEOUT;
                break;
            }
            busytime++;
        }
        else
        {
            break;
        }
    }

    if (AIR_E_OK == rv)
    {
        rv = aml_readReg(unit, PHY_IAD, &data, 4);
        if (AIR_E_OK == rv)
        {
            *ptr_reg_data = GET_MDIO_DATA(data);
            DIAG_PRINT(HAL_DBG_INFO, " read_c22 unit(%u) bus(0x%X) phy(0x%X) reg(0x%X) val(0x%X)\n", unit, bus_id,
                       phy_addr, reg_addr, *ptr_reg_data);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
    }

    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_writeC45
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port Number
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
hal_sco_mdio_writeC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         wdata = 0;
    UI32_T         checkBit = 0, busytime = 0;

    DIAG_PRINT(HAL_DBG_INFO, " write_c45 unit(%u) bus(0x%X) phy(0x%X) dev(0x%X) reg(0x%X) val(0x%X)\n", unit, bus_id,
               phy_addr, dev_type, reg_addr, reg_data);
    /* phase1 set ctrl mode */
    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
            (SET_MDIO_ST_CL45);
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    wdata |= SET_MDIO_ACS_START;
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    while (1)
    {
        rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
        if (AIR_E_OK == rv)
        {
            if (!(checkBit & SET_MDIO_ACS_START))
            {
                break;
            }
            else if (busytime >= IAC_MAX_BUSY_TIME)
            {
                rv = AIR_E_TIMEOUT;
                break;
            }
            busytime++;
        }
        else
        {
            break;
        }
    }

    if (AIR_E_OK == rv)
    {
        /* phase2 set write mode & write data */
        wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_data) | (SET_MDIO_CMD_CL45W) |
                (SET_MDIO_ST_CL45);
        aml_writeReg(unit, PHY_IAC, &wdata, 4);
        wdata |= SET_MDIO_ACS_START;
        aml_writeReg(unit, PHY_IAC, &wdata, 4);
        busytime = 0;
        while (1)
        {
            rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
            if (AIR_E_OK == rv)
            {
                if (!(checkBit & SET_MDIO_ACS_START))
                {
                    break;
                }
                else if (busytime >= IAC_MAX_BUSY_TIME)
                {
                    rv = AIR_E_TIMEOUT;
                    break;
                }
                busytime++;
            }
            else
            {
                break;
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) Write BusId[%u] PhyAddr[0x%x] DevId[0x%X] Reg[0x%X] : 0x%08X\n", unit, bus_id,
                   phy_addr, dev_type, reg_addr, reg_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_readC45
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_sco_mdio_readC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI32_T         wdata = 0;
    UI32_T         checkBit = 0, busytime = 0;
    UI32_T         data = 0;

    /* phase1 set ctrl mode */
    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
            (SET_MDIO_ST_CL45);
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    wdata |= SET_MDIO_ACS_START;
    aml_writeReg(unit, PHY_IAC, &wdata, 4);
    while (1)
    {
        rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
        if (AIR_E_OK == rv)
        {
            if (!(checkBit & SET_MDIO_ACS_START))
            {
                break;
            }
            else if (busytime >= IAC_MAX_BUSY_TIME)
            {
                rv = AIR_E_TIMEOUT;
                break;
            }
            busytime++;
        }
        else
        {
            break;
        }
    }

    if (AIR_E_OK == rv)
    {
        /* phase2 set read mode */
        wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | (SET_MDIO_CMD_CL45R) | (SET_MDIO_ST_CL45);
        aml_writeReg(unit, PHY_IAC, &wdata, 4);
        wdata |= SET_MDIO_ACS_START;
        aml_writeReg(unit, PHY_IAC, &wdata, 4);
        busytime = 0;
        while (1)
        {
            rv = aml_readReg(unit, PHY_IAC, &checkBit, 4);
            if (AIR_E_OK == rv)
            {
                if (!(checkBit & SET_MDIO_ACS_START))
                {
                    break;
                }
                else if (busytime >= IAC_MAX_BUSY_TIME)
                {
                    rv = AIR_E_TIMEOUT;
                    break;
                }
                busytime++;
            }
            else
            {
                break;
            }
        }
        if (AIR_E_OK == rv)
        {
            /* phase3 read data */
            rv = aml_readReg(unit, PHY_IAD, &data, 4);
            if (AIR_E_OK == rv)
            {
                *ptr_reg_data = GET_MDIO_DATA(data);
                DIAG_PRINT(HAL_DBG_INFO, " read_c45 unit(%u) bus(0x%X) phy(0x%X) dev(0x%X) reg(0x%X) val(0x%X)\n", unit,
                           bus_id, phy_addr, dev_type, reg_addr, *ptr_reg_data);
            }
            else
            {
                DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
            }
        }
    }

    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) Read BusId[%u] PhyAddr[0x%x] DevId[0x%X] Reg[0x%X] : 0x%08X\n", unit, bus_id,
                   phy_addr, dev_type, reg_addr, data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, " (%u) rv=(%d)\n", unit, rv);
    }

    return rv;
}

#ifdef AIR_EN_I2C_PHY
/* FUNCTION NAME: hal_sco_mdio_writeC22ByI2c
 * PURPOSE:
 *      Write the mdio data for IEEE clause 22 by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
 *      phy_addr        --  PHY address
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
hal_sco_mdio_writeC22ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    UI32_T          wdata = 0, rdata = 0, busytime = 0;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    DIAG_PRINT(HAL_DBG_INFO, " i2c_write_c22 unit(%u) bus(0x%X) i2c(0x%X) phy(0x%X) reg(0x%X) val(0x%X)\n", unit,
               bus_id, i2c_addr, phy_addr, reg_addr, reg_data);
    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_DATA((unsigned short)reg_data) |
            (SET_MDIO_CMD_CL22W) | (SET_MDIO_ST_CL22);

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = PHY_IAC;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_readC22ByI2c
 * PURPOSE:
 *      Read the mdio data for IEEE clause 22 by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
 *      phy_addr        --  PHY address
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
hal_sco_mdio_readC22ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    UI32_T          wdata = 0, rdata = 0, busytime = 0;
    AIR_ERROR_NO_T  rv = AIR_E_OK;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;

    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg_addr) | SET_MDIO_CMD_CL22R | SET_MDIO_ST_CL22;

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = PHY_IAC;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.addr = PHY_IAD;

    rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    rdata = sifm_param.info.data;
    *ptr_reg_data = (UI16_T)rdata;
    DIAG_PRINT(HAL_DBG_INFO, " i2c_read_c22 unit(%u) bus(0x%X) i2c(0x%X) phy(0x%X) reg(0x%X) val(0x%X)\n", unit, bus_id,
               i2c_addr, phy_addr, reg_addr, *ptr_reg_data);
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_writeC45ByI2c
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
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
hal_sco_mdio_writeC45ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    UI32_T          wdata = 0, rdata = 0, busytime = 0;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    DIAG_PRINT(HAL_DBG_INFO, " i2c_write_c45 unit(%u) bus(0x%X) i2c(0x%X) phy(0x%X) dev(0x%X) reg(0x%X) val(0x%X)\n",
               unit, bus_id, i2c_addr, phy_addr, dev_type, reg_addr, reg_data);
    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
            (SET_MDIO_ST_CL45);

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = PHY_IAC;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_data) | (SET_MDIO_CMD_CL45W) |
            (SET_MDIO_ST_CL45);

    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_readC45ByI2c
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45 by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
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
hal_sco_mdio_readC45ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data)
{
    UI32_T          wdata = 0, rdata = 0, busytime = 0;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | SET_MDIO_DATA(reg_addr) | (SET_MDIO_CMD_CL45ADDR) |
            (SET_MDIO_ST_CL45);

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = PHY_IAC;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(dev_type) | (SET_MDIO_CMD_CL45R) | (SET_MDIO_ST_CL45);

    sifm_param.info.data = wdata;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.info.data |= SET_MDIO_ACS_START;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_write failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    while (1)
    {
        rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
        if (rv != AIR_E_OK)
        {
            DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
            break;
        }
        rdata = sifm_param.info.data;
        if (!(rdata & SET_MDIO_ACS_START))
        {
            break;
        }
        else if (busytime >= IAC_MAX_BUSY_TIME)
        {
            rv = AIR_E_TIMEOUT;
            break;
        }
        busytime++;
    }
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    sifm_param.addr = PHY_IAD;

    rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    rdata = sifm_param.info.data;
    *ptr_reg_data = (UI16_T)rdata;
    DIAG_PRINT(HAL_DBG_INFO, " i2c_read_c45 unit(%u) bus(0x%X) i2c(0x%X) phy(0x%X) dev(0x%X) reg(0x%X) val(0x%X)\n",
               unit, bus_id, i2c_addr, phy_addr, dev_type, reg_addr, *ptr_reg_data);
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_writeRegByI2c
 * PURPOSE:
 *      Write the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
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
hal_sco_mdio_writeRegByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI32_T reg_addr,
    const UI32_T reg_data)
{
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = reg_addr;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = reg_data;

    rv = hal_sco_sif_write(unit, &sif_info, &sifm_param);
    return rv;
}

/* FUNCTION NAME: hal_sco_mdio_readRegByI2c
 * PURPOSE:
 *      Read the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      bus_id          --  Bus ID
 *      i2c_addr        --  I2C address
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
hal_sco_mdio_readRegByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI32_T reg_addr,
    UI32_T      *ptr_reg_data)
{
    UI32_T          rdata = 0;
    AIR_SIF_INFO_T  sif_info;
    AIR_SIF_PARAM_T sifm_param;
    AIR_ERROR_NO_T  rv = AIR_E_OK;

    sif_info.channel = bus_id;
    sif_info.slave_id = i2c_addr;

    sifm_param.addr_len = I2C_MDIO_REG_ADDR_LEN;
    sifm_param.addr = reg_addr;
    sifm_param.data_len = I2C_MDIO_REG_DATA_LEN;
    sifm_param.info.data = rdata;

    rv = hal_sco_sif_read(unit, &sif_info, &sifm_param);
    if (rv != AIR_E_OK)
    {
        DIAG_PRINT(HAL_DBG_ERR, " i2c_read failed! (%u) rv=(%d)\n", unit, rv);
        return rv;
    }

    *ptr_reg_data = sifm_param.info.data;
    return rv;
}

#endif /* #ifdef AIR_EN_I2C_PHY */
