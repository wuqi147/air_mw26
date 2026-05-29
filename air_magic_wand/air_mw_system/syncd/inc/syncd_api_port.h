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

/* FILE NAME:  syncd_api_port.h
 * PURPOSE:
 *      It provides port API functions in syncd.
 *
 * NOTES:
 */

#ifndef _SYNCD_API_PORT_H_
#define _SYNCD_API_PORT_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "syncd_api.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: syncd_api_port_status
 * PURPOSE:
 *      Set state of a specific port.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_status(
    const SYNCD_API_ARG_T *ptr_api_arg);

MW_ERROR_NO_T
syncd_api_port_settings(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_port_eeeConfig
 * PURPOSE:
 *      Set EEE mode of a specific port.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_eeeConfig(
    const SYNCD_API_ARG_T *ptr_api_arg);


/* FUNCTION NAME: syncd_api_port_jumbo
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_port_jumbo(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_check_port_is_in_lag
 * PURPOSE:
 *      Check whether the port is in the lag.
 *
 * INPUT:
 *      port_id     --  the port id to be check
 *
 * OUTPUT:
 *      ptr_lag_id  --  porinter to lag id
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      If the port is in the trunk, MW_E_OK will be returned
 */
MW_ERROR_NO_T
syncd_api_check_port_is_in_lag(
    UI32_T      port_id,
    AIR_PORT_T  *ptr_lag_id);

/* FUNCTION NAME: syncd_api_flushPortBmpMacAddr
 * PURPOSE:
 *      This API is used to flush port bitmap dynamic mac address.
 *
 * INPUT:
 *      unit        -- Device ID
 *      portbmp     --  port bitmap to flush
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_flushPortBmpMacAddr(
    const UI32_T    unit,
    const MW_PORT_BITMAP_T portbmp);

#endif  /*_SYNCD_API_PORT_H_*/
