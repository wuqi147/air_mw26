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

/* FILE NAME:  hal_sco_mdio.h
 * PURPOSE:
 *  Define MDIO module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_MDIO_H
#define HAL_SCO_MDIO_H

/* INCLUDE FILE DECLARTIONS
 */
#include "air_error.h"
#include "air_port.h"
#include "air_types.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define PHY_IAC                   (0x10218020)
#define CSR_PHY_ACS_ST_OFFT       (31)
#define CSR_PHY_ACS_ST_LENG       (1)
#define CSR_PHY_ACS_ST_RELMASK    (0x00000001)
#define CSR_PHY_ACS_ST_MASK       (CSR_PHY_ACS_ST_RELMASK << CSR_PHY_ACS_ST_OFFT)
#define CSR_MDIO_REG_ADDR_OFFT    (25)
#define CSR_MDIO_REG_ADDR_LENG    (5)
#define CSR_MDIO_REG_ADDR_RELMASK (0x0000001f)
#define CSR_MDIO_REG_ADDR_MASK    (CSR_MDIO_REG_ADDR_RELMASK << CSR_MDIO_REG_ADDR_OFFT)
#define CSR_MDIO_PHY_ADDR_OFFT    (20)
#define CSR_MDIO_PHY_ADDR_LENG    (5)
#define CSR_MDIO_PHY_ADDR_RELMASK (0x0000001f)
#define CSR_MDIO_PHY_ADDR_MASK    (CSR_MDIO_PHY_ADDR_RELMASK << CSR_MDIO_PHY_ADDR_OFFT)
#define CSR_MDIO_CMD_OFFT         (18)
#define CSR_MDIO_CMD_LENG         (2)
#define CSR_MDIO_CMD_RELMASK      (0x00000003)
#define CSR_MDIO_CMD_MASK         (CSR_MDIO_CMD_RELMASK << CSR_MDIO_CMD_OFFT)
#define CSR_MDIO_ST_OFFT          (16)
#define CSR_MDIO_ST_LENG          (2)
#define CSR_MDIO_ST_RELMASK       (0x00000003)
#define CSR_MDIO_ST_MASK          (CSR_MDIO_ST_RELMASK << CSR_MDIO_ST_OFFT)
#define CSR_MDIO_WR_DATA_OFFT     (0)
#define CSR_MDIO_WR_DATA_LENG     (16)
#define CSR_MDIO_WR_DATA_RELMASK  (0x0000FFFF)
#define CSR_MDIO_WR_DATA_MASK     (CSR_MDIO_WR_DATA_RELMASK << CSR_MDIO_WR_DATA_OFFT)

#define PHY_IAD                  (0x10218024)
#define CSR_MDIO_RD_DATA_OFFT    (0)
#define CSR_MDIO_RD_DATA_LENG    (16)
#define CSR_MDIO_RD_DATA_RELMASK (0x0000FFFF)
#define CSR_MDIO_RD_DATA_MASK    (CSR_MDIO_RD_DATA_RELMASK << CSR_MDIO_RD_DATA_OFFT)

#define PHY_SMI           (0x1021801C)
#define CSR_SMI_PMDC_MASK (0xc0)
#define CSR_SMI_PMDC_OFFT (6)

/* MACRO FUNCTION DECLARATIONS
 */

#define SET_MDIO_ACS_IDLE     (0 << CSR_PHY_ACS_ST_OFFT)
#define SET_MDIO_ACS_START    (1 << CSR_PHY_ACS_ST_OFFT)
#define SET_MDIO_CMD_CL22W    (1 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL22R    (2 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45ADDR (0 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45W    (1 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45INC  (2 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45R    (3 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_ST_CL45      (0 << CSR_MDIO_ST_OFFT)
#define SET_MDIO_ST_CL22      (1 << CSR_MDIO_ST_OFFT)
#define SET_MDIO_PHY(phy)     ((phy & CSR_MDIO_PHY_ADDR_RELMASK) << CSR_MDIO_PHY_ADDR_OFFT)
#define SET_MDIO_REG(reg)     ((reg & CSR_MDIO_REG_ADDR_RELMASK) << CSR_MDIO_REG_ADDR_OFFT)
#define SET_MDIO_DATA(data)   ((data & CSR_MDIO_WR_DATA_RELMASK) << CSR_MDIO_WR_DATA_OFFT)
#define GET_MDIO_DATA(data)   ((data & CSR_MDIO_RD_DATA_RELMASK) << CSR_MDIO_RD_DATA_OFFT)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

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
    const UI16_T reg_data);

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
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_sco_mdio_writeC45
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
    const UI16_T reg_data);

/* FUNCTION NAME: hal_sco_mdio_readC45
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
    UI16_T      *ptr_reg_data);

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
    const UI16_T reg_data);

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
    UI16_T      *ptr_reg_data);

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
    const UI16_T reg_data);

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
    UI16_T      *ptr_reg_data);

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
    const UI32_T reg_data);

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
    UI32_T      *ptr_reg_data);
#endif /* #ifdef AIR_EN_I2C_PHY */
#endif /* End of HAL_SCO_MDIO_H */
