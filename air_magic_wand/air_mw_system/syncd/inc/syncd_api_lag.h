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

/* FILE NAME:  syncd_api_lag.h
 * PURPOSE:
 *      It provides lag API functions in syncd.
 *
 * NOTES:
 */

#ifndef _SYNCD_API_LAG_H_
#define _SYNCD_API_LAG_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "syncd_api.h"
#include "db_api.h"
#include "default_config.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define PORTYPE(lwr, sync)  (lwr | (sync << 8))
#define PORTLWR(port)       (port & 0xFF)
#define PORTSYN(port)       ((port >> 8) & 0xFF)

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    LAG_MODE_NONE = 0,
    LAG_MODE_STATIC,
    LAG_MODE_LACP_ACTIVE,
    LAG_MODE_LACP_PASSIVE,
    LAG_MODE_LAST
} LAG_MODE_T;

typedef enum
{
    LAG_PROCESS_STATE_IDLE = 0,
    LAG_PROCESS_STATE_PROCESSING,
    LAG_PROCESS_STATE_LAST
} LAG_PROCESS_STATE_T;

typedef enum
{
    LAG_ALGORITHM_MAC_SA_DA = 0,
    LAG_ALGORITHM_MAC_DA,
    LAG_ALGORITHM_MAC_SA,
    LAG_ALGORITHM_LAST
} LAG_ALGORITHM_T;

typedef enum
{
    PORT_LACP_STATE_IDLE = 0,
    PORT_LACP_STATE_UNSELECTED,
    PORT_LACP_STATE_SELECTED,
    PORT_LACP_STATE_AGGREGATED,
    PORT_LACP_STATE_LAST
} PORT_LACP_STATE_T;

/* The port mirror information table */
typedef struct SYNCD_PORT_MIRROR_DB_ONE_ENTRY_S
{
    UI8_T           enable;      /* The admin status of the port mirror session */
    UI8_T           dest_port;   /* The destination port of the mirror session */
    UI32_T          src_in_port; /* The ingress traffic of the source port */
    UI32_T          src_eg_port; /* The egress traffic of the source port */
} ATTRIBUTE_PACK SYNCD_PORT_MIRROR_DB_ONE_ENTRY_T;

/* FUNCTION NAME: syncd_api_lag_init
 * PURPOSE:
 *      Initialization lag information
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_lag_init();

/* FUNCTION NAME: syncd_api_lag_config
 * PURPOSE:
 *      process lag function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_lag_config(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_lag_lacp_config
 * PURPOSE:
 *      process lag lacp function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
#ifdef AIR_SUPPORT_LACP
MW_ERROR_NO_T
syncd_api_lag_lacp_config(
    const SYNCD_API_ARG_T *ptr_api_arg);
#endif

/* FUNCTION NAME: syncd_api_lag_algo_config
 * PURPOSE:
 *      process lag lacp function
 *
 * INPUT:
 *      ptr_api_arg         --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_lag_algo_config(
    const SYNCD_API_ARG_T *ptr_api_arg);

/* FUNCTION NAME: syncd_api_lag_get_member0
 * PURPOSE:
 *      Get the point of the lag member0
 *
 * INPUT:
 *      Node
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The point of the lag member0
 *
 * NOTES:
 *      None
 */
UI8_T *
syncd_api_lag_get_member0(
    void);
#endif  /*_SYNCD_API_LAG_H_*/

