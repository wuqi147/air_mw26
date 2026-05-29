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

/* FILE NAME:  syncd_timer.h
 * PURPOSE:
 *      It provides syncd internal timer functions.
 *
 * NOTES:
 */

#ifndef _SYNCD_TIMER_H_
#define _SYNCD_TIMER_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "db_api.h"
#include "air_error.h"
#include "air_port.h"
#include "air_mib.h"

#include "osapi.h"
#include "osapi_memory.h"
#include "osapi_string.h"
#ifdef AIR_SUPPORT_SNMP
#include "lwip/apps/snmp.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/
#define SYNCD_TMR_PS_NAME       "syn_tm_ps"
#define SYNCD_TMR_PS_PERIOD     AIR_MAX_SYNCD_TMR_PS       /* unit:ms */

#define SYNCD_TMR_MIB_NAME      "syn_tm_mib"
#define SYNCD_TMR_MIB_PERIOD    (1000)      /* unit:ms */

#define SYNCD_TMR_RESUME_PERIOD     (2 * 1000 / AIR_MAX_SYNCD_TMR_PS)     /* unit:s */
#define SYNCD_TMR_MAX_RESUME_PERIOD (10 * 1000 / AIR_MAX_SYNCD_TMR_PS)     /* unit:s */

#define SYNCD_MIB_HC_MASK       (0xFFFFFFFF)
#define SYNCD_MIB_HC_CNT        (0x100000000)
#define SYNCD_MIB_HC_OFFT       (32)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct SYNCD_TIMER_S
{
    C8_T *              name;
    UI32_T              period;
    timehandle_t        th;
}SYNCD_TIMER_T;

/* Current MIB counter */
typedef struct SYNCD_MIB_HC_S
{
    BOOL_T clrFlag;
#ifdef AIR_LITE_MW
    UI64_T rx_ucp;
    UI64_T rx_mcp;
    UI64_T rx_bcp;

    UI64_T tx_ucp;
    UI64_T tx_mcp;
    UI64_T tx_bcp;
#else
    UI64_T rx_64;
    UI64_T rx_65;
    UI64_T rx_128;
    UI64_T rx_256;
    UI64_T rx_512;
    UI64_T rx_1024;
    UI64_T rx_1519;

    UI64_T tx_64;
    UI64_T tx_65;
    UI64_T tx_128;
    UI64_T tx_256;
    UI64_T tx_512;
    UI64_T tx_1024;
    UI64_T tx_1519;

#ifdef AIR_SUPPORT_SNMP
    UI64_T rx_ucp;
    UI64_T rx_mcp;
    UI64_T rx_bcp;

    UI64_T tx_ucp;
    UI64_T tx_mcp;
    UI64_T tx_bcp;
#endif
#endif
}SYNCD_MIB_HC_T;

#ifdef AIR_LITE_MW
typedef struct SYNCD_MIB_S
{
    DB_MIB_CNT_T     cnt;        /* MIB Counters */
}SYNCD_MIB_T;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   syncd_mib_counter_msg_handle
 * PURPOSE
 *      Polling port mib counter.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_mib_counter_msg_handle(
    void);

#endif  /* _SYNCD_TIMER_H_ */
