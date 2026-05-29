/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:  sfp_sff_data.c
 * PURPOSE:
 *      Read SFF data.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_pin.h"
#include "sfp_sff_data.h"
#include "sfp_config_customer.h"
#include "sfp_util.h"
#include "sfp_module_handle.h"
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_SFF_CONNECTOR_CODE_BASE (0x02)
#define SFP_SFF_ETHERNET_COMPLIANCE_CODE_BASE (0x06)
#define SFP_SFF_COMPLIANCE_CODE_FIBER_TYPE_BASE (0x07)
#define SFP_SFF_ETHERNET_COMPLIANCE_CODE_1000BASET (0x08)

#define SFP_SFF_SIGNALING_RATE_ADDR (12)
#define SFP_SFF_VENDOR_NAME_ADDR (0x14)
#define SFP_SFF_VENDOR_OUI_ADDR (0x25)
#define SFP_SFF_VENDOR_REV_ADDR (0x38)
#define SFP_SFF_VENDOR_PN_ADDR (0x28)
#define SFP_SFF_ENHANCED_OPTIONS_ADDR (93)
#define SFP_SFF_OPTIONAL_STATUS_CONTORL_BITS_ADDR (110)

#define SFP_SFF_COMPLIANCE_CODE_FIBER_TYPE_BITMASK    (0xFFFFF0FF)
#define SFP_SFF_CONNECTOR_TYPE_RJ45                   (0x22)
#define SFP_SFF_CONNECTOR_TYPE_SC                     (0x01)
#define SFP_SFF_CONNECTOR_TYPE_LC                     (0x07)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _sfp_sff_getEnhancedOptions
 * PURPOSE:
 *      Get the enhanced options of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_enhancedOptions  -- A pointer to return the enhanced options read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
_sfp_sff_getEnhancedOptions(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_enhancedOptions)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;
    UI32_T enhanced_options = 0;

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_ENHANCED_OPTIONS_ADDR, 1, &enhanced_options);
    if (MW_E_OK == ret)
    {
        *ptr_enhancedOptions = (UI8_T)(enhanced_options & 0xFF);
    }

    return ret;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_sff_getMediaType
 * PURPOSE:
 *      Get the media type of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                       -- Device ID
 *      port                       -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      SFP_SFF_MEDIA_TYPE_COPPER  -- Copper
 *      SFP_SFF_MEDIA_TYPE_FIBER   -- Fiber
 *      Other                      -- Failed
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
SFP_SFF_MEDIA_TYPE_T
sfp_sff_getMediaType(
    const UI32_T unit,
    const UI32_T port)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    UI32_T ether_code = 0, fiber_code = 0, connctor_code = 0;

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_COMPLIANCE_CODE_FIBER_TYPE_BASE, 4, &fiber_code);
    ret |= sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_ETHERNET_COMPLIANCE_CODE_BASE, 1, &ether_code);
    ret |= sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_CONNECTOR_CODE_BASE, 1, &connctor_code);
    if (MW_E_OK != ret)
    {
        return SFP_SFF_MEDIA_TYPE_LAST;
    }

    if (SFP_SFF_CONNECTOR_TYPE_RJ45 == connctor_code)
    {
        return SFP_SFF_MEDIA_TYPE_COPPER;
    }

    if ((SFP_SFF_CONNECTOR_TYPE_SC == connctor_code) ||
        (SFP_SFF_CONNECTOR_TYPE_LC == connctor_code))
    {
        return SFP_SFF_MEDIA_TYPE_FIBER;
    }

    if ((SFP_SFF_COMPLIANCE_CODE_FIBER_TYPE_BITMASK & fiber_code) || (SFP_SFF_ETHERNET_COMPLIANCE_CODE_1000BASET != ether_code))
    {
        return SFP_SFF_MEDIA_TYPE_FIBER;
    }

    return SFP_SFF_MEDIA_TYPE_COPPER;
}

/* FUNCTION NAME:   sfp_sff_getSignalingRate
 * PURPOSE:
 *      Get the signaling rate of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_signaling_rate   -- A pointer to return the signaling rate read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getSignalingRate(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_signaling_rate)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    *ptr_signaling_rate = 0;
    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_SIGNALING_RATE_ADDR, 1, ptr_signaling_rate);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    *ptr_signaling_rate = (*ptr_signaling_rate) * 100;
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_sff_getVendorName
 * PURPOSE:
 *      Get the vendor name of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 *      buffer_length        -- The lenght of buffer pointed by ptr_buffer
 * OUTPUT:
 *      ptr_buffer           -- A pointer to return the vendor name read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getVendorName(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_buffer,
    const UI32_T buffer_length)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    UI32_T temp_buffer[(SFP_MODULE_VENDOR_NAME_SIZE / sizeof(UI32_T)) + 1] = {0};

    if ((NULL == ptr_buffer) || (buffer_length < SFP_MODULE_VENDOR_NAME_SIZE))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_VENDOR_NAME_ADDR, SFP_MODULE_VENDOR_NAME_SIZE, temp_buffer);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    memcpy(ptr_buffer, temp_buffer, SFP_MODULE_VENDOR_NAME_SIZE);
    SFP_LOG_DEBUG("vendor name:%s", ptr_buffer);
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_sff_getVendorOui
 * PURPOSE:
 *      Get the vendor OUI of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 *      buffer_length        -- The lenght of buffer pointed by ptr_buffer
 * OUTPUT:
 *      ptr_buffer           -- A pointer to return the vendor OUI read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getVendorOui(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_buffer,
    const UI32_T buffer_length)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    UI32_T temp_oui = 0;

    if ((NULL == ptr_buffer) || (buffer_length < SFP_MODULE_VENDOR_OUI_SIZE))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_VENDOR_OUI_ADDR, SFP_MODULE_VENDOR_OUI_SIZE, &temp_oui);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    memcpy(ptr_buffer, &temp_oui, SFP_MODULE_VENDOR_OUI_SIZE);
    ptr_buffer[SFP_MODULE_VENDOR_OUI_SIZE] = '\0';
    SFP_LOG_DEBUG("vendor oui:0x%x 0x%x 0x%x", ptr_buffer[0], ptr_buffer[1], ptr_buffer[2]);
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_sff_getVendorRev
 * PURPOSE:
 *      Get the vendor Rev of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 *      buffer_length        -- The lenght of buffer pointed by ptr_buffer
 * OUTPUT:
 *      ptr_buffer           -- A pointer to return the vendor Rev read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getVendorRev(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_buffer,
    const UI32_T buffer_length)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    UI32_T temp_rev = 0;

    if ((NULL == ptr_buffer) || (buffer_length < SFP_MODULE_VENDOR_REV_SIZE))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_VENDOR_REV_ADDR, SFP_MODULE_VENDOR_REV_SIZE, &temp_rev);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    memcpy(ptr_buffer, &temp_rev, SFP_MODULE_VENDOR_REV_SIZE);
    ptr_buffer[SFP_MODULE_VENDOR_REV_SIZE] = '\0';
    SFP_LOG_DEBUG("vendor rev:%s", ptr_buffer);
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_sff_getVendorPN
 * PURPOSE:
 *      Get the vendor PN of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 *      buffer_length        -- The lenght of buffer pointed by ptr_buffer
 * OUTPUT:
 *      ptr_buffer           -- A pointer to return the vendor PN read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getVendorPN(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_buffer,
    const UI32_T buffer_length)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    UI32_T temp_buffer[(SFP_MODULE_VENDOR_PN_SIZE / sizeof(UI32_T)) + 1] = {0};

    if ((NULL == ptr_buffer) || (buffer_length < SFP_MODULE_VENDOR_PN_SIZE))
    {
        return MW_E_BAD_PARAMETER;
    }

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A0H_ADDRESS, SFP_SFF_VENDOR_PN_ADDR, SFP_MODULE_VENDOR_PN_SIZE, temp_buffer);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    memcpy(ptr_buffer, temp_buffer, SFP_MODULE_VENDOR_PN_SIZE);
    ptr_buffer[SFP_MODULE_VENDOR_PN_SIZE] = '\0';
    SFP_LOG_DEBUG("vendor PN:%s", ptr_buffer);
    return MW_E_OK;
}

/* FUNCTION NAME:   sfp_sff_isSWRXLOSImplemented
 * PURPOSE:
 *      Check if the soft RX_LOS of the SFP module inserted to a port is
 *      implemented or not.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_implemented      -- A pointer to return the implementation status.
 *                              TRUE: implemented
 *                              FALSE: not implemented
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_isSWRXLOSImplemented(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_implemented)
{
    UI8_T enhanced_options = 0;
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    ret = _sfp_sff_getEnhancedOptions(unit, port, &enhanced_options);
    *ptr_implemented = FALSE;
    if ((ret == MW_E_OK) && (enhanced_options & BIT(4)))
    {
        *ptr_implemented = TRUE;
    }

    return ret;
}

/* FUNCTION NAME:   sfp_sff_getSWRXLOS
 * PURPOSE:
 *      Get the soft RX_LOS of the SFP module inserted to a port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_rx_los           -- A pointer to return the soft RX_LOS read.
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_OTHERS         -- Other errors
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OP_INVALID     -- Operation is invalid
 *      MW_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      Only support the SERDES port or the COMBO SERDES port.
 */
MW_ERROR_NO_T
sfp_sff_getSWRXLOS(
    const UI32_T unit,
    const UI32_T port,
    UI8_T *ptr_rx_los)
{
    MW_ERROR_NO_T ret = MW_E_OP_INCOMPLETE;
    UI32_T rx_los = 0;

    ret = sfp_pin_sda_read(unit, port, SFP_SFF_A2H_ADDRESS, SFP_SFF_OPTIONAL_STATUS_CONTORL_BITS_ADDR, 1, &rx_los);
    if (MW_E_OK == ret)
    {
        *ptr_rx_los = (0 != (rx_los & BIT(1))) ? SFP_MODULE_PIN_HIGH : SFP_MODULE_PIN_LOW;
    }

    return ret;
}

