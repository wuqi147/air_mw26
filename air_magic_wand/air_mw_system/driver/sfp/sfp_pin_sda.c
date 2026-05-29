/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2024
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

/* FILE NAME:   sfp_pin_sda.c
 * PURPOSE:
 *      Provide the interfaces to access SFP module via the SFP SDA PIN.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "sfp_pin_sda.h"
#include "sfp_config_customer.h"
#include "sfp_sff_data.h"
#include "sfp_task.h"
#include "sfp_module_inside_phy.h"
#ifdef AIR_EN_I2C_BITBANG
#include "i2c_bitbang.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_pin_sda_driver_init0
 * PURPOSE:
 *      Initialize the SFP PIN driver for each SFP port or each COMBO port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Success
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_init() is recommended
 *      when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_init0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data)
{
    UNUSED(unit)
    UNUSED(port)
    UNUSED(ptr_hwParam)
    UNUSED(ptr_data)

    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_switchCtrlToPort0
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_switchCtrlToPort0(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data)
{
    UNUSED(ptr_data)

    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    static UI8_T last_port = 0;
    MW_ERROR_NO_T ret = MW_E_NOT_SUPPORT;

    if ((NULL == ptr_hwParam) || (NULL == ptr_portSettings))
    {
        SFP_LOG_ERROR("port:%d ptr_hwParam:0x%p", port, ptr_hwParam);
        return MW_E_BAD_PARAMETER;
    }

    /* It has been switched to the port. */
    if (last_port == port)
    {
        return MW_E_OK;
    }

    sif_info.channel = ptr_hwParam->i2c_channel;
    sif_info.slave_id = ptr_hwParam->i2c_addr;
    sif_param.addr_len = 1;
    sif_param.addr = 0;
    sif_param.data_len = 1;
    sif_param.info.data = ptr_portSettings->mux_channel;
    ret = sfp_util_sif_write(unit, &sif_info, &sif_param);
    if (MW_E_OK == ret)
    {
        last_port = port;
    }
    else
    {
        SFP_LOG_WARN("switchCtrlToPort: channel:%d slave_id:0x%x addr:%x addr_len:%d data_len:%d data:%x ret:%d",
                  sif_info.channel, sif_info.slave_id, sif_param.addr, sif_param.addr_len, sif_param.data_len, sif_param.info.data, ret);
    }

    return ret;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_write0
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_write0(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;

    if ((0 != length) && (NULL != ptr_data))
    {
        sif_info.channel = i2c_channel;
        sif_info.slave_id = sff_2wire_addr;
        sif_param.addr = offset;
        sif_param.addr_len = 1;
        sif_param.data_len = length;
        if (length <= 4)
        {
            sif_param.info.data = *ptr_data;
        }
        else
        {
            sif_param.info.ptr_data = ptr_data;
        }
        ret = sfp_util_sif_write(unit, &sif_info, &sif_param);
    }

    return ret;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_read0
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_read0(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
    AIR_SIF_INFO_T sif_info;
    AIR_SIF_PARAM_T sif_param;
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;

    if ((0 != length) && (NULL != ptr_data))
    {
        sif_info.channel = i2c_channel;
        sif_info.slave_id = sff_2wire_addr;
        sif_param.addr = offset;
        sif_param.addr_len = 1;

        sif_param.data_len = length;
        if (length <= 4)
        {
            ret = sfp_util_sif_read(unit, &sif_info, &sif_param);
            if (MW_E_OK == ret)
            {
                osapi_memcpy(ptr_data, &(sif_param.info.data), length);
            }
        }
        else
        {
            sif_param.info.ptr_data = ptr_data;
            ret = sfp_util_sif_read(unit, &sif_info, &sif_param);
        }
    }

    return ret;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_switchCtrlToPort1
 * PURPOSE:
 *      Switch the control to an specific port before accessing its SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      ptr_hwParam          -- HW parameters
 *      ptr_data             -- Additional data for the initialization
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_switchCtrlToPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_switchCtrlToPort1(
    const UI32_T unit,
    const UI32_T port,
    const SFP_HW_PARAM_T *ptr_hwParam,
    void *ptr_data)
{
    UNUSED(unit);
    UNUSED(port);
    UNUSED(ptr_hwParam);
    UNUSED(ptr_data);

    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_write1
 * PURPOSE:
 *      Write data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be written to
 *      length               -- Data length
 *      ptr_data             -- A pointer points to the data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_write() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_write1(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
#ifdef AIR_EN_I2C_BITBANG
    I2C_BITBANG_PARAM_T bitbang_param = {0};

    bitbang_param.addr = offset;
    bitbang_param.addr_len = 1;
    bitbang_param.data_len = length;
    if (length <= 4)
    {
        bitbang_param.info.data = *ptr_data;
    }
    else
    {
        bitbang_param.info.ptr_data = ptr_data;
    }

    return sfp_util_i2c_bitbang_write(unit, i2c_channel, sff_2wire_addr, &bitbang_param);
#else
    return MW_E_NOT_SUPPORT;
#endif

}

/* FUNCTION NAME:   sfp_pin_sda_driver_read1
 * PURPOSE:
 *      Read data through a 2-wire address of an SFP module via the SFP SDA PIN.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      i2c_channel          -- I2C channel
 *      sff_2wire_addr       -- 2-wire address of an SFP module
 *      offset               -- The offset of the byte(s) to be read from
 *      length               -- The length of buffer ptr_data points to
 * OUTPUT:
 *      ptr_data             -- A pointer points to the buffer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Invalid parameter
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_read() is
 *      recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_read1(
    const UI32_T unit,
    const UI8_T i2c_channel,
    const UI8_T sff_2wire_addr,
    const UI16_T offset,
    const UI32_T length,
    UI32_T *const ptr_data)
{
#ifdef AIR_EN_I2C_BITBANG
    I2C_BITBANG_PARAM_T bitbang_param = {0};
    AIR_ERROR_NO_T ret = AIR_E_OK;

    bitbang_param.addr = offset;
    bitbang_param.addr_len = 1;
    bitbang_param.data_len = length;

    if (length <= 4)
    {
        ret = sfp_util_i2c_bitbang_read(unit, i2c_channel, sff_2wire_addr, &bitbang_param);
        if (AIR_E_OK == ret)
        {
            osapi_memcpy(ptr_data, &(bitbang_param.info.data), length);
        }
    }
    else
    {
        bitbang_param.info.ptr_data = ptr_data;
        ret = sfp_util_i2c_bitbang_read(unit, i2c_channel, sff_2wire_addr, &bitbang_param);
    }

    return ret;
#else
    return MW_E_NOT_SUPPORT;
#endif

}

/* FUNCTION NAME:   sfp_pin_sda_driver_writeC22ByPort0
 * PURPOSE:
 *      Write data to a CL22 PHY register of the inside PHY of an SFP module
 *      inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      reg_addr             -- PHY register address to be written to
 *      reg_data             -- Data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_writeC22ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI32_T data = SFP_CONVERT_REGISTER_DATA(reg_data);
    SFP_TASK_PORT_INFO_T *ptr_portInfo = sfp_task_getPortInfo(port);

    if ((NULL == ptr_portInfo) || (SFP_SFF_INVALID_ADDRESS == ptr_portInfo->phy_2wire_addr))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_switchCtrlToPort(unit, port);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    ret = sfp_pin_sda_write(unit, port, ptr_portInfo->phy_2wire_addr, reg_addr, 2, &data);
    if (MW_E_OK != ret)
    {
        SFP_LOG_ERROR("port:%d write Reg failed. slave_id:0x:%x addr:0x%x reg_data:0x%x sif_data:0x%x ret:%d",
                  port, ptr_portInfo->phy_2wire_addr, reg_addr, reg_data, data, ret);
    }
    SFP_LOG_DEBUG("port:%d write Reg. slave_id:0x:%x addr:0x%x reg_data:0x%x sif_data:0x%x ret:%d",
              port, ptr_portInfo->phy_2wire_addr, reg_addr, reg_data, data, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_readC22ByPort0
 * PURPOSE:
 *      Read a CL22 PHY register of the inside PHY of an SFP module inserted to
 *      a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      reg_addr             -- PHY register address to be read from
 * OUTPUT:
 *      ptr_reg_data         -- A pointer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_readC22ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_readC22ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI32_T data = 0;
    SFP_TASK_PORT_INFO_T *ptr_portInfo = sfp_task_getPortInfo(port);

    if ((NULL == ptr_reg_data) || (NULL == ptr_portInfo) || (SFP_SFF_INVALID_ADDRESS == ptr_portInfo->phy_2wire_addr))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_switchCtrlToPort(unit, port);
    if (MW_E_OK != ret)
    {
        SFP_LOG_WARN("switchCtrlToPort failed. port:%d", port);
        return ret;
    }

    ret = sfp_pin_sda_read(unit, port, ptr_portInfo->phy_2wire_addr, reg_addr, 2, &data);
    if (MW_E_OK == ret)
    {
        *ptr_reg_data = (UI16_T)SFP_CONVERT_REGISTER_DATA(data);
    }
    else
    {
        SFP_LOG_ERROR("port:%d read Reg failed. slave_id:0x:%x addr:0x%x reg_data:0x%x sif_data:0x%x ret:%d",
                  port, ptr_portInfo->phy_2wire_addr, reg_addr, *ptr_reg_data, data, ret);
    }

    SFP_LOG_DEBUG("port:%d read Reg. slave_id:0x:%x addr:0x%x reg_data:0x%x sif_data:0x%x ret:%d",
              port, ptr_portInfo->phy_2wire_addr, reg_addr, *ptr_reg_data, data, ret);
    return ret;
}

/* FUNCTION NAME:   sfp_pin_sda_driver_writeC45ByPort0
 * PURPOSE:
 *      Write data to a CL45 PHY register of the inside PHY of an SFP module
 *      inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      dev_type             -- MMD address
 *      reg_addr             -- PHY register address to be written to
 *      reg_data             -- Data to write
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_writeC45ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_writeC45ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI16_T data = 0;

    /* Set address - indicate MMD */
    data = dev_type & 0x1F;
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_CTRL, data);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Set address - indicate MMD register */
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_DATA, reg_addr);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Write data - indicate MMD */
    data |= 0x4000;
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_CTRL, data);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Write data - indicate MMD register */
    return sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_DATA, reg_data);
}

/* FUNCTION NAME:   sfp_pin_sda_driver_readC45ByPort0
 * PURPOSE:
 *      Read a CL45 PHY register of the inside PHY of an SFP module inserted to
 *      a port.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 *      dev_type             -- MMD address
 *      reg_addr             -- PHY register address to be read from
 * OUTPUT:
 *      ptr_reg_data         -- A pointer to return the data read
 * RETURN:
 *      MW_E_OK              -- Operation success
 *      MW_E_BAD_PARAMETER   -- Parameter is wrong
 *      MW_E_NOT_SUPPORT     -- Operation is not supported
 *      MW_E_OP_INCOMPLETE   -- Operation incomplete
 *
 * NOTES:
 *      Do not use this function directly. Instead, sfp_pin_sda_readC45ByPort()
 *      is recommended when it is needed.
 */
MW_ERROR_NO_T
sfp_pin_sda_driver_readC45ByPort0(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T *const ptr_reg_data)
{
    MW_ERROR_NO_T ret = MW_E_BAD_PARAMETER;
    UI16_T data = 0;

    /* Set address - indicate MMD */
    data = dev_type & 0x1F;
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_CTRL, data);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Set address - indicate MMD register */
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_DATA, reg_addr);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Read data - indicate MMD */
    data |= 0x4000;
    ret = sfp_pin_sda_writeC22ByPort(unit, port, SFP_PHY_MII_MMD_CTRL, data);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Read data - indicate MMD register */
    return sfp_pin_sda_readC22ByPort(unit, port, SFP_PHY_MII_MMD_DATA, ptr_reg_data);
}

