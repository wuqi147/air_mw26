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

/* FILE NAME:  hal_mdio.h
 * PURPOSE:
 *      It provides MDIO module API.
 * NOTES:
 */

#ifndef HAL_MDIO_H
#define HAL_MDIO_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct HAL_MDIO_CB_S
{
    AIR_SEMAPHORE_ID_T mdio_sem;
#ifdef AIR_EN_I2C_PHY
    AIR_SEMAPHORE_ID_T i2c_mdio_sem;
#endif
} HAL_MDIO_CB_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: hal_mdio_writeC22
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
hal_mdio_writeC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC22
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
hal_mdio_readC22(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC22ByPort
 * PURPOSE:
 *      Write the mdio data for IEEE clause 22 by port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_writeC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC22ByPort
 * PURPOSE:
 *      Read the mdio data for IEEE clause 22 by port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_readC22ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC45
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
hal_mdio_writeC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC45
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
hal_mdio_readC45(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC45ByPort
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45 by port
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
hal_mdio_writeC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_initRsrc
 * PURPOSE:
 *      To initialize MDIO module resource.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *        AIR_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_mdio_initRsrc(
    const UI32_T unit);

/* FUNCTION NAME: hal_mdio_deinitRsrc
 * PURPOSE:
 *      To deinitialize MDIO module resource.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_mdio_deinitRsrc(
    const UI32_T unit);

/* FUNCTION NAME: hal_mdio_readC45ByPort
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45 by port
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
hal_mdio_readC45ByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC22FromIntBus
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
hal_mdio_writeC22FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC22FromIntBus
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
hal_mdio_readC22FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC22FromIntBusByPort
 * PURPOSE:
 *      Write the mdio data for IEEE clause 22 from internal bus by port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_writeC22FromIntBusByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC22FromIntBusByPort
 * PURPOSE:
 *      Read the mdio data for IEEE clause 22 from internal bus by port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_readC22FromIntBusByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC45FromIntBus
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
hal_mdio_writeC45FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC45FromIntBus
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
hal_mdio_readC45FromIntBus(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC45FromIntBusByPort
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45 from internal bus by port
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
hal_mdio_writeC45FromIntBusByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC45FromIntBusByPort
 * PURPOSE:
 *      Read the mdio data for IEEE clause 45 from internal bus by port
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
hal_mdio_readC45FromIntBusByPort(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

#ifdef AIR_EN_I2C_PHY
/* FUNCTION NAME: hal_mdio_writeC22ByI2c
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
hal_mdio_writeC22ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC22ByI2c
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
hal_mdio_readC22ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeC45ByI2c
 * PURPOSE:
 *      Write the mdio data for IEEE clause 45 by i2c
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
hal_mdio_writeC45ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

/* FUNCTION NAME: hal_mdio_readC45ByI2c
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
hal_mdio_readC45ByI2c(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

/* FUNCTION NAME: hal_mdio_writeRegByI2c
 * PURPOSE:
 *      Write the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_writeRegByI2c(
    const UI32_T unit,
    const UI16_T port,
    const UI32_T reg_addr,
    const UI32_T reg_data);

/* FUNCTION NAME: hal_mdio_readRegByI2c
 * PURPOSE:
 *      Read the register data by i2c
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port number
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
hal_mdio_readRegByI2c(
    const UI32_T unit,
    const UI16_T port,
    const UI32_T reg_addr,
    UI32_T      *ptr_reg_data);

#endif /* #ifdef AIR_EN_I2C_PHY */
#endif /* #ifndef HAL_MDIO_H */