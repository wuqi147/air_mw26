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

/* FILE NAME:  syncd_api_stp.h
 * PURPOSE:
 *      It provides STP API functions in syncd.
 *
 * NOTES:
 */

#ifndef _SYNCD_API_STP_H_
#define _SYNCD_API_STP_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "syncd_api.h"
#include <air_stp.h>

/* NAMING CONSTANT DECLARATIONS
*/
#define SYNCD_API_STP_LLDP_ETYPE    (0x88cc)
#define SYNCD_API_STP_FID_MAX       (MAX_MSTP_INSTANCE_NUM)
#if defined(AIR_LITE_MW) || defined(AIR_EN_CORAL)
#define AIR_LP_USE_STP_BLOCK   (1)
#endif
/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct
{
    UI8_T user_bmp[AIR_STP_STATE_LAST];
} __attribute__((packed)) SYNCD_API_STP_PORT_INFO_T;

typedef struct
{
    SYNCD_API_STP_PORT_INFO_T portInfo[SYNCD_API_STP_FID_MAX];
} __attribute__((packed)) SYNCD_API_STP_INS_PORT_INFO_T;

typedef enum
{
    SYNCD_API_STP_USER_RSTP,
    SYNCD_API_STP_USER_LACP,
    SYNCD_API_STP_USER_LP,

    SYNCD_API_STP_USER_LAST
}SYNCD_API_STP_USER_T;

typedef enum
{
#ifndef AIR_EN_CORAL
    SYNCD_API_STP_ACL_DROP,
#endif
    SYNCD_API_STP_ACL_FWD,

    SYNCD_API_STP_ACL_LAST
}SYNCD_API_STP_ACL_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: syncd_api_stp_setBpduCtrl
 * PURPOSE:
 *      Set port STP state.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setBpduCtrl(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_stp_setLldpEnable
 * PURPOSE:
 *      Enable/Disable lldp acl.
 *
 * INPUT:
 *      state -- lldp enable/disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setLldpEnable(
    UI8_T state);

/* FUNCTION NAME: syncd_api_stp_setPortStateByUser
 * PURPOSE:
 *      Block specific ports by user.
 *
 * INPUT:
 *      port --     port id
 *      state --    stp state
 *      user --     caller
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setPortStateByUser(
    const UI32_T port,
    AIR_STP_STATE_T state,
    SYNCD_API_STP_USER_T user);

/* FUNCTION NAME: syncd_api_stp_getPortStateByUser
 * PURPOSE:
 *      Get specific ports block state by user.
 *
 * INPUT:
 *      port --     port id
 *      state --    stp state
 *      user --     caller
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_getPortStateByUser(
    const UI32_T port,
    SYNCD_API_STP_USER_T user,
    AIR_STP_STATE_T *state);

/* FUNCTION NAME: syncd_api_stp_setPortState
 * PURPOSE:
 *      Set port STP state.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setPortState(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_stp_blockPort
 * PURPOSE:
 *      Block specific ports.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_blockPort(
    const SYNCD_API_ARG_T *ptr_api_arg);

#ifdef AIR_SUPPORT_RSTP
/* FUNCTION NAME:   syncd_rstp_set_flush_flag
 * PURPOSE:
 *      Set port's flush flag.
 *
 * INPUT:
 *      port                 -- Index of port number
 *      flag                 -  set---TRUE,unset---FALSE
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
syncd_rstp_set_flush_flag(
     UI32_T port,
     UI8_T  flag);
#endif

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME:   syncd_mstp_set_flush_flag
 * PURPOSE:
 *      Set vlan flush flag.
 *
 * INPUT:
 *      ptr_vid              -  VLANs
 *      num                  -  num of VLANs
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
syncd_mstp_set_flush_flag(
     UI16_T *ptr_vid,
     UI32_T num);

/* FUNCTION NAME: syncd_api_stp_setInstancePortState
 * PURPOSE:
 *      Set STP state of specific ports with fid.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_setInstancePortState(
    const SYNCD_API_ARG_T *ptr_api_arg);
#endif

/* FUNCTION NAME: syncd_api_stp_init
 * PURPOSE:
 *      Initialization stp information
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_stp_init();

#endif  /*_SYNCD_API_STP_H_*/

