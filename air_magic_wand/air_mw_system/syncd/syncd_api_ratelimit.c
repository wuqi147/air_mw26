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

/* FILE NAME:  syncd_api_ratelimit.c
 * PURPOSE:
 *  Implement rate limit API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <syncd_api_ratelimit.h>
#include <air_qos.h>
#include <air_port.h>
#include <osapi_string.h>

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

/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: syncd_api_ratelimit
 * PURPOSE:
 *      Set port rate limit from DB.
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
syncd_api_ratelimit(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    UI32_T  port;
    UI16_T  index;
    UI8_T   direction;
    UI8_T   *ptr_rateLimit = NULL;
    AIR_QOS_RATE_LIMIT_CFG_T rate;

    MW_CHECK_PTR(ptr_api_arg);


    index = ptr_api_arg ->ptr_type ->e_idx;

    if (M_GET == ptr_api_arg ->method && DB_ALL_ENTRIES == index )
    {

        /* Receive M_GET from DB means the 1st notification */
        direction = (ptr_api_arg ->ptr_type ->f_idx);
        ptr_rateLimit = ptr_api_arg ->ptr_data;
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            memset(&rate, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
            /* read current value */
            if(AIR_E_OK != air_qos_getRateLimitCfg(0, port, &rate))
            {
                SYNCD_LOG_ERROR("***Error***, get port=%u rateLimit error", port);
                return MW_E_OP_INCOMPLETE;
            }
            if (PORT_INGRESS_RATE == direction)
            {
                rate.ingress_cir = htonl(get32(ptr_rateLimit));
            }
            else if (PORT_EGRESS_RATE == direction)
            {
                rate.egress_cir = htonl(get32(ptr_rateLimit));
            }
            SYNCD_LOG_DEBUG("Port  = %d \n", port);
            /* Enable rate-limit */
            SYNCD_LOG_DEBUG("[Egress] state  = %d, [Ingress] state  = %d \n", !!rate.egress_cir, !!rate.ingress_cir);
            if(AIR_E_OK != air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_INGRESS, !!rate.ingress_cir))
            {
                SYNCD_LOG_ERROR("***Error***, set port=%u rateLimitEnable error", port);
                return MW_E_OP_INCOMPLETE;
            }
            if(AIR_E_OK != air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_EGRESS, !!rate.egress_cir))
            {
                SYNCD_LOG_ERROR("***Error***, set port=%u rateLimitEnable error", port);
                return MW_E_OP_INCOMPLETE;
            }

            /* Setup rate-limit value */
            SYNCD_LOG_DEBUG("[Egress] value  = %d, [Ingress] value  = %d \n", rate.egress_cir, rate.ingress_cir);
            if(AIR_E_OK != air_qos_setRateLimitCfg(0, port, &rate))
            {
                SYNCD_LOG_ERROR("***Error***, set port=%u rateLimit error", port);
                return MW_E_OP_INCOMPLETE;
            }
            ptr_rateLimit = ptr_rateLimit + sizeof(UI32_T);
        }
    }
    else if (M_UPDATE == ptr_api_arg ->method)
    {
        /* Receive M_UPDATE from DB means Update data */
        index = (ptr_api_arg ->ptr_type ->e_idx);
        direction = (ptr_api_arg ->ptr_type ->f_idx);

        memset(&rate, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
        port = (UI32_T)index;

        /* read current value */
        if(AIR_E_OK != air_qos_getRateLimitCfg(0, port, &rate))
        {
            SYNCD_LOG_ERROR("***Error***, get port=%u rateLimit error", port);
            return MW_E_OP_INCOMPLETE;
        }

        if (PORT_INGRESS_RATE == direction)
        {
            osapi_memcpy((UI8_T*)&(rate.ingress_cir), (UI8_T*)ptr_api_arg->ptr_data, 4);
        }
        else if (PORT_EGRESS_RATE == direction)
        {
            osapi_memcpy((UI8_T*)&(rate.egress_cir), (UI8_T*)ptr_api_arg->ptr_data, 4);

        }
        SYNCD_LOG_DEBUG("Port  = %d \n", port);
        /* Enable rate-limit */
        SYNCD_LOG_DEBUG("[Egress] state  = %d, [Ingress] state  = %d \n", !!rate.egress_cir, !!rate.ingress_cir);
        if(AIR_E_OK != air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_INGRESS, !!rate.ingress_cir))
        {
            SYNCD_LOG_ERROR("***Error***, set port=%u rateLimitEnable error", port);
            return MW_E_OP_INCOMPLETE;
        }
        if(AIR_E_OK != air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_EGRESS, !!rate.egress_cir))
        {
            SYNCD_LOG_ERROR("***Error***, set port=%u rateLimitEnable error", port);
            return MW_E_OP_INCOMPLETE;
        }

        SYNCD_LOG_DEBUG("[Egress] value  = %d, [Ingress] value  = %d \n", rate.egress_cir, rate.ingress_cir);
        if(AIR_E_OK != air_qos_setRateLimitCfg(0, port, &rate))
        {
            SYNCD_LOG_ERROR("***Error***, set port=%u rateLimit error", port);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else
    {
        SYNCD_LOG_ERROR("Method(%X) is not supported", ptr_api_arg ->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

