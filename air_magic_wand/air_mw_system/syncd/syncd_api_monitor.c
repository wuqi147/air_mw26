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

/* FILE NAME:  syncd_api_monitor.c
 * PURPOSE:
 *  Implement traffic monitor API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include "syncd_api_monitor.h"
#include "syncd_log.h"
#include "syncd_in.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    SYNCD_TRAFFIC_MONITER_CLEAR_NONE,         /* Do not clear MIB counter */
    SYNCD_TRAFFIC_MONITER_CLEAR_SINGLE_PORT,  /* Clear MIB counter of a port */
    SYNCD_TRAFFIC_MONITER_CLEAR_ALL_PORTS,    /* Clear MIB counters of all ports */
    SYNCD_TRAFFIC_MONITER_CLEAR_STATE_LAST
} SYNCD_TRAFFIC_MONITER_CLEAR_T;

/* EXPORTED SUBPROGRAM BODIES
*/

/* FUNCTION NAME: syncd_api_clear_mib_counter
 * PURPOSE:
 *      Clear MIB counter.
 *
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_clear_mib_counter(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    UI8_T clear_mib_counter = SYNCD_TRAFFIC_MONITER_CLEAR_NONE;
    UI8_T clear_mib_counter_entry[PLAT_MAX_PORT_NUM];
    UI32_T unit = 0, rc = MW_E_BAD_PARAMETER, i;

    memcpy(&clear_mib_counter, (UI8_T *)ptr_api_arg->ptr_data, sizeof(UI8_T));

    if (SYNCD_TRAFFIC_MONITER_CLEAR_NONE == clear_mib_counter)
    {
        return MW_E_OK;
    }
    else if (SYNCD_TRAFFIC_MONITER_CLEAR_SINGLE_PORT == clear_mib_counter)
    {
        air_mib_clearPortCnt((const UI32_T)unit, (const UI32_T)ptr_api_arg->ptr_type->e_idx);
        syncd_mib_set_clear_flag((const UI32_T)ptr_api_arg->ptr_type->e_idx);
        clear_mib_counter = SYNCD_TRAFFIC_MONITER_CLEAR_NONE;
        rc = (MW_ERROR_NO_T)syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                                M_UPDATE,
                                                PORT_OPER_INFO,
                                                PORT_MIB_COUNTER_CLEAR,
                                                ptr_api_arg->ptr_type->e_idx,
                                                &clear_mib_counter,
                                                MSG_TIMEOUT_WAIT_INDEFINITELY);
    }
    else if (SYNCD_TRAFFIC_MONITER_CLEAR_ALL_PORTS == clear_mib_counter)
    {
        for (i = 1; PLAT_MAX_PORT_NUM >= i; i++)
        {
            air_mib_clearPortCnt((const UI32_T)unit, (const UI32_T)i);
            syncd_mib_set_clear_flag((const UI32_T)i);
        }

        clear_mib_counter = SYNCD_TRAFFIC_MONITER_CLEAR_NONE;
        memset(clear_mib_counter_entry, clear_mib_counter, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
        rc = (MW_ERROR_NO_T)syncd_queue_db_send(SYNCD_MSG_QUEUE_NAME,
                                                M_UPDATE,
                                                PORT_OPER_INFO,
                                                PORT_MIB_COUNTER_CLEAR,
                                                DB_ALL_ENTRIES,
                                                &clear_mib_counter_entry,
                                                MSG_TIMEOUT_WAIT_INDEFINITELY);
    }

    /* Do not clear MIB counter in DB or in SYNCD context here. SYNCD updates MIB counter in DB and in its context periodically.
     * As long as the webpage flush interval is larger than the interval (SYNCD_TMR_MIB_PERIOD) SYNCD updates MIB counter, MIB data
     * shown on webpage will be correct.
     */
    return rc;
}

