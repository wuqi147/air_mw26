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

/* FILE NAME:   air_module.c
 * PURPOSE:
 *      Define the software modules in AIR SDK.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_module.h>

/* NAMING CONSTANT DECLARATIONS
 */

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
/* clang-format off */
const static C8_T *_air_module_list[] =
{
    "vlan",
    "stp",
    "mir",
    "port",
    "diag",
    "osal",
    "aml",
    "hal",
    "chip",
    "lag",
    "init",
    "ipmc",
    "l2",
    "led",
    "mib",
    "qos",
    "swc",
    "sec",
    "sflow",
    "svlan",
    "stag",
    "dos",
    "acl",
    "ifmon",
    "mdio",
    "phy",
    "sif",
    "perif",
    "poe",
};
/* clang-format on */

/* EXPORTED SUBPROGRAM BODIES
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
    const UI32_T module_id)
{
    if (module_id >= AIR_MODULE_LAST)
    {
        return "unknown";
    }

    return _air_module_list[module_id];
} /* End of air_module_getModuleName */

/* LOCAL SUBPROGRAM BODIES
 */
