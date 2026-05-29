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

/* FILE NAME:   air_module.h
 * PURPOSE:
 *      Define the software modules in AIR SDK.
 * NOTES:
 */

#ifndef AIR_MODULE_H
#define AIR_MODULE_H

/* INCLUDE FILE DECLARATIONS
 */
#include <osal/osal_type.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_MODULE_MAX_LENGTH_OF_NAME (8)

/* DATA TYPE DECLARATIONS
 */
typedef enum AIR_MODULE_E
{
    AIR_MODULE_VLAN = 0,
    AIR_MODULE_STP,
    AIR_MODULE_MIR,
    AIR_MODULE_PORT,
    AIR_MODULE_DIAG,
    AIR_MODULE_OSAL,
    AIR_MODULE_AML,
    AIR_MODULE_HAL,
    AIR_MODULE_CHIP,
    AIR_MODULE_LAG,
    AIR_MODULE_INIT,
    AIR_MODULE_IPMC,
    AIR_MODULE_L2,
    AIR_MODULE_LED,
    AIR_MODULE_MIB,
    AIR_MODULE_QOS,
    AIR_MODULE_SWC,
    AIR_MODULE_SEC,
    AIR_MODULE_SFLOW,
    AIR_MODULE_SVLAN,
    AIR_MODULE_STAG,
    AIR_MODULE_DOS,
    AIR_MODULE_ACL,
    AIR_MODULE_IFMON,
    AIR_MODULE_MDIO,
    AIR_MODULE_PHY,
    AIR_MODULE_SIF,
    AIR_MODULE_PERIF,
    AIR_MODULE_POE,
    AIR_MODULE_LAST
} AIR_MODULE_T;
/*   !!!! Don't forget update _air_module_list in air_module.c, diag.c    !!!!*/

#define AIR_MODULE_NUMER_OF_MODULES (AIR_MODULE_LAST)

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_module_getModuleName
 * PURPOSE:
 *      This API returns the module name by given module ID. All modules could
 *      invoke this API.
 *
 * INPUT:
 *      const UI32_T module_id  -- Module ID, range from 0 to
 *                                 (AIR_MODULE_LAST - 1)
 * OUTPUT:
 *      UI8_T *ptr_module_name      -- Point to module name. The caller should
 *                                  prepare memory to receive this output. The
 *                                  maximum length of module name is
 *                                  AIR_MODULE_MAX_LENGTH_OF_NAME.
 * RETURN:
 *      AIR_E_OK             -- Successfully get module name.
 *      AIR_E_BAD_PARAMETER  -- Invalid parameter.
 *
 * NOTES:
 */

const C8_T *
air_module_getModuleName(
    const UI32_T module_id);
#endif /* AIR_MODULE_H */
