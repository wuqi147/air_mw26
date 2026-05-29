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

/* FILE NAME:  mw_init.h
 * PURPOSE:
 *      Middleware initialization function call.
 *
 * NOTES:
 */

#ifndef MW_INIT_H
#define MW_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#ifdef AIR_SUPPORT_LP
#include "lp.h"
#endif
#include "db_main.h"
#ifdef AIR_SUPPORT_IGMP_SNP
#include "igmp_snoop.h"
#endif
#include "sys_mgmt.h"
#ifdef AIR_SUPPORT_DHCP_SNOOP
#include "dhcp_snoop.h"
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_VOICE_VLAN
#include "voice_vlan.h"
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#include "stp.h"
#endif
#ifdef AIR_SUPPORT_LLDPD
#include "lldp.h"
#endif /*AIR_SUPPORT_LLDPD*/
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_DHCP     LWIP_DHCP

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: mw_init_initModule
 * PURPOSE:
 *      This API is used to initialize the middleware modules.
 *
 * INPUT:
 *      unit            --  The unit ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_init_initModule(
    const UI32_T unit);

/* FUNCTION NAME: mw_init_initSwitching
 * PURPOSE:
 *      This API is used to initialize the switching modules.
 *
 * INPUT:
 *      unit            --  The unit ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_init_initSwitching(
    const UI32_T unit);

MW_ERROR_NO_T
mw_init_preInitModule(
    const UI32_T unit);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/

#endif  /* #ifndef MW_INIT_H */

