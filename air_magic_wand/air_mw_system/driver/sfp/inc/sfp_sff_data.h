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

/* FILE NAME:  sfp_sff_data.h
 * PURPOSE:
 *      Read SFF data.
 * NOTES:
 *
 */

#ifndef SFP_SFF_DATA_H
#define SFP_SFF_DATA_H

/* INCLUDE FILE DECLARATIONS
 */
#include "air_error.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_SFF_A0H_ADDRESS (0x50)
#define SFP_SFF_A2H_ADDRESS (0x51)
#define SFP_SFF_ACH_ADDRESS (0x56)
#define SFP_SFF_INVALID_ADDRESS (0xFF)
#define SFP_SFF_DEFAULT_ADDRESS (SFP_SFF_ACH_ADDRESS)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFP_SFF_MEDIA_TYPE_COPPER,
    SFP_SFF_MEDIA_TYPE_FIBER,

    SFP_SFF_MEDIA_TYPE_LAST
} SFP_SFF_MEDIA_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI32_T port);

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
    UI32_T *ptr_signaling_rate);

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
    const UI32_T buffer_length);

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
    const UI32_T buffer_length);

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
    const UI32_T buffer_length);

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
    const UI32_T buffer_length);

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
    UI8_T *ptr_implemented);

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
    UI8_T *ptr_rx_los);

#endif /*End of SFP_SFF_DATA_H*/

