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

/* FILE NAME:  syncd_api_qos.c
 * PURPOSE:
 *  Implement qos API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <air_qos.h>

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
/* FUNCTION NAME: syncd_api_qos_mode
 * PURPOSE:
 *      Set QOS mode value.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_mode(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI8_T mode;

    MW_CHECK_PTR(ptr_api_arg);

    if (sizeof(UI8_T) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if(DB_ALL_ENTRIES != ptr_api_arg ->ptr_type ->e_idx)
    {
        SYNCD_LOG_ERROR("Not support specify entry idx in qos mode!");
        return MW_E_BAD_PARAMETER;
    }

    memcpy(&mode, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);

    rc = air_qos_setTrustMode(unit, mode);
    if(AIR_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Call function failed(%d), mode is %d", rc, mode);
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_qos_queue
 * PURPOSE:
 *      Set QoS priority to queue mapping.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_queue(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T unit = 0;
    PRI_Q_T priority;
    int i;

    MW_CHECK_PTR(ptr_api_arg);

    if(DB_ALL_ENTRIES != ptr_api_arg ->ptr_type ->e_idx)
    {
        SYNCD_LOG_ERROR("Not support specify entry idx in qos pri&queue mapping!");
        return MW_E_BAD_PARAMETER;
    }
    if (sizeof(priority) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }
    memcpy(&priority, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
    for (i = 0; i < AIR_QOS_PRI_MAX_NUM; i++)
    {
        rc = air_qos_setPriToQueue(unit, i, priority[i]);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("port %d Call function failed(%d), priority %d", i, rc, priority[i]);
            mw_rc = MW_E_OP_INCOMPLETE;
        }
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_qos_portPri
 * PURPOSE:
 *      Set port based QoS priority.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_portPri(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T unit = 0;
    UI8_T portPriority[MAX_PORT_NUM];
    UI32_T port = 0;
    UI16_T index;
    UI16_T expect_entry_num;

    index = ptr_api_arg ->ptr_type ->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == index )? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG("index=%u, expect_entry_num=%u", index, expect_entry_num);
    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg ->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if(DB_ALL_ENTRIES == index)
    {
        memcpy(&portPriority, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            SYNCD_LOG_DEBUG("portid %d pri %d", port, portPriority[port-1]);
            rc = air_qos_setPortPriority(unit, port, portPriority[port-1]);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d) portid %d pri %d", rc, port ,portPriority[port-1]);
                mw_rc = MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        port = (UI32_T)index;
        memcpy(&portPriority[port-1], (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
        SYNCD_LOG_DEBUG("portid %d pri %d", port, portPriority[port-1]);
        rc = air_qos_setPortPriority(unit, port, portPriority[port-1]);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Call function failed(%d) portid %d pri %d", rc, port, portPriority[port-1]);
            mw_rc = MW_E_OP_INCOMPLETE;
        }
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_qos_dscpPri
 * PURPOSE:
 *      Set QoS DSCP to priority mapping.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_dscpPri(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T unit = 0;
    DSCP_Q_T  dscp;
    int    i;

    MW_CHECK_PTR(ptr_api_arg);

    if(DB_ALL_ENTRIES != ptr_api_arg ->ptr_type ->e_idx)
    {
        SYNCD_LOG_ERROR("Not support specify entry idx in qos dscp&pri mapping!");
        return MW_E_BAD_PARAMETER;
    }

    if(AIR_QOS_QUEUE_DSCP_MAX_NUM != ptr_api_arg ->data_size)
    {
        SYNCD_LOG_ERROR("Error data size");
        return MW_E_BAD_PARAMETER;
    }

    memcpy(&dscp, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
    for (i = 0; i < AIR_QOS_QUEUE_DSCP_MAX_NUM; i++)
    {
        rc = air_qos_setDscpToPri(unit, i, dscp[i]);
        if(AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("Call function failed(%d) i %u dscp %u", rc, i , dscp[i]);
            mw_rc = MW_E_OP_INCOMPLETE;
        }
    }
    return mw_rc;
}

/* FUNCTION NAME: syncd_api_qos_shapermode
 * PURPOSE:
 *      Set QoS shaper mode value.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_shapermode(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T unit = 0;
    UI8_T qos_schedule[PLAT_MAX_PORT_NUM];
    int j;
    UI32_T port;
    UI32_T default_weight = 1;
    AIR_QOS_SHAPER_WEIGHT_T sch_weight;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T size = 0;
    UI16_T index;
    UI16_T expect_entry_num;

    index = ptr_api_arg ->ptr_type ->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;

    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    MW_CHECK_PTR(ptr_api_arg);
    SYNCD_LOG_DEBUG("index-%u", index);

    if(DB_ALL_ENTRIES != index)
    {
        if (PLAT_CPU_PORT == index)
        {
            SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", index);
            return MW_E_BAD_PARAMETER;
        }
        memcpy(&qos_schedule[index-1], (UI8_T *)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
        SYNCD_LOG_DEBUG("port %d qos schedule %u", index, qos_schedule[index-1]);
        if (AIR_QOS_SHAPER_MODE_SP == qos_schedule[index-1])
        {
            for (j = 0; j < MAX_QUEUE_NUM; j++)
            {
                rc = air_qos_setScheduleMode(unit, index, j, AIR_QOS_SCH_MODE_SP, default_weight);
            }
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d) port %u queue %d", rc, index, j);
                mw_rc = MW_E_OP_INCOMPLETE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_WRR == qos_schedule[index-1])
        {
            for (j = 0; j < MAX_QUEUE_NUM; j++)
            {
                mw_rc = syncd_queue_db_getData(M_GET, PORT_QOS, PORT_QOS_Q0_WEIGHT_WRR + j, index, &ptr_msg, &size, (void**)&ptr_data);
                if (MW_E_OK != mw_rc)
                {
                    SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", index, j);
                    return mw_rc;
                }
                sch_weight.min_weight = 0;
                osapi_memcpy(&sch_weight.min_weight, ptr_data, size);
                MW_FREE(ptr_msg);

                if(AIR_E_OK == rc)
                {
                    rc = air_qos_setScheduleMode(unit, index, j, AIR_QOS_SCH_MODE_WRR, sch_weight.min_weight);
                    if(AIR_E_OK != rc)
                    {
                        SYNCD_LOG_ERROR("set schedule mode failed port %u queue %d", index, j);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
                else
                {
                    SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", index, j);
                    return MW_E_OP_INCOMPLETE;
                }
            }
        }
        else if (AIR_QOS_SHAPER_MODE_WFQ == qos_schedule[index-1])
        {
            for (j = 0; j < MAX_QUEUE_NUM; j++)
            {
                mw_rc = syncd_queue_db_getData(M_GET, PORT_QOS, PORT_QOS_Q0_WEIGHT_WFQ + j, index, &ptr_msg, &size, (void**)&ptr_data);
                if (MW_E_OK != mw_rc)
                {
                    SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", index, j);
                    return mw_rc;
                }
                sch_weight.max_weight = 0;
                osapi_memcpy(&sch_weight.max_weight, ptr_data, size);
                MW_FREE(ptr_msg);

                if(AIR_E_OK == rc)
                {
                    rc = air_qos_setScheduleMode(unit, index, j, AIR_QOS_SCH_MODE_WFQ, sch_weight.max_weight);
                    if(AIR_E_OK != rc)
                    {
                        SYNCD_LOG_ERROR("set schedule mode failed port %u queue %d", index, j);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
                else
                {
                    SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", index, j);
                    return MW_E_OP_INCOMPLETE;
                }
            }
        }
        else
        {
            SYNCD_LOG_ERROR("Not Support this mode %hhu in ", qos_schedule[index]);
            return MW_E_BAD_PARAMETER;
        }

    }
    else
    {
        memcpy(&qos_schedule, (UI8_T *)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            SYNCD_LOG_DEBUG("port %d qos schedule %u", port, qos_schedule[port-1]);
            if (AIR_QOS_SHAPER_MODE_SP == qos_schedule[port-1])
            {
                for (j = 0; j < MAX_QUEUE_NUM; j++)
                {
                    rc = air_qos_setScheduleMode(unit, port, j, AIR_QOS_SCH_MODE_SP, default_weight);
                }
                if(AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d) port %u queue %d", rc, port, j);
                    mw_rc = MW_E_OP_INCOMPLETE;
                }
            }
            else if (AIR_QOS_SHAPER_MODE_WRR == qos_schedule[port-1])
            {
                for (j = 0; j < MAX_QUEUE_NUM; j++)
                {
                    mw_rc = syncd_queue_db_getData(M_GET, PORT_QOS, PORT_QOS_Q0_WEIGHT_WRR + j, port, &ptr_msg, &size, (void**)&ptr_data);
                    if (MW_E_OK != mw_rc)
                    {
                        SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", port, j);
                        return mw_rc;
                    }
                    sch_weight.min_weight = 0;
                    osapi_memcpy(&sch_weight.min_weight, ptr_data, size);
                    MW_FREE(ptr_msg);

                    if(AIR_E_OK == rc)
                    {
                        rc = air_qos_setScheduleMode(unit, port, j, AIR_QOS_SCH_MODE_WRR, sch_weight.min_weight);
                        if(AIR_E_OK != rc)
                        {
                            SYNCD_LOG_ERROR("set schedule mode failed port %u queue %d", port, j);
                            return MW_E_OP_INCOMPLETE;
                        }
                    }
                    else
                    {
                        SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", port, j);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
            }
            else if (AIR_QOS_SHAPER_MODE_WFQ == qos_schedule[port-1])
            {
                for (j = 0; j < MAX_QUEUE_NUM; j++)
                {
                    mw_rc = syncd_queue_db_getData(M_GET, PORT_QOS, PORT_QOS_Q0_WEIGHT_WFQ + j, port, &ptr_msg, &size, (void**)&ptr_data);
                    if (MW_E_OK != mw_rc)
                    {
                        SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", port, j);
                        return mw_rc;
                    }
                    sch_weight.max_weight = 0;
                    osapi_memcpy(&sch_weight.max_weight, ptr_data, size);
                    MW_FREE(ptr_msg);

                    if(AIR_E_OK == rc)
                    {
                        rc = air_qos_setScheduleMode(unit, port, j, AIR_QOS_SCH_MODE_WFQ, sch_weight.max_weight);
                        if(AIR_E_OK != rc)
                        {
                            SYNCD_LOG_ERROR("set schedule mode failed port %u queue %d", port, j);
                            return MW_E_OP_INCOMPLETE;
                        }
                    }
                    else
                    {
                        SYNCD_LOG_ERROR("get shaper weight failed port %u queue %d", port, j);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
            }
            else
            {
                SYNCD_LOG_ERROR("Not Support this mode %hhu in ", qos_schedule[port]);
                return MW_E_BAD_PARAMETER;
            }
        }
    }

    return mw_rc;
}

/* FUNCTION NAME: syncd_api_qos_shaperweight
 * PURPOSE:
 *      Set QoS shaper weight value.
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
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_qos_shaperweight(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    UI32_T unit = 0;
    UI8_T weight_all[MAX_PORT_NUM] = {0};
    AIR_QOS_SHAPER_WEIGHT_T weight;
    UI32_T port;
    UI16_T index;
    UI16_T expect_entry_num;
    UI8_T queue;
    UI8_T default_weight = 1;

    index = ptr_api_arg ->ptr_type ->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;

    /* Check DB_MSG data_size is correct or not */
    if ((sizeof(UI8_T) * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    weight.min_weight = AIR_QOS_SHAPER_NOSETTING;
    weight.max_weight = AIR_QOS_SHAPER_NOSETTING;

    MW_CHECK_PTR(ptr_api_arg);

    if(DB_ALL_ENTRIES != index)
    {
        port = (UI32_T)index;
        /*WRR setting*/
        if (PORT_QOS_Q7_WEIGHT_WRR >= ptr_api_arg->ptr_type->f_idx && PORT_QOS_Q0_WEIGHT_WRR <= ptr_api_arg->ptr_type->f_idx)
        {
            weight.min_weight = 0;
            memcpy(&weight.min_weight, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
            queue = ptr_api_arg->ptr_type->f_idx - PORT_QOS_Q0_WEIGHT_WRR;
            SYNCD_LOG_DEBUG("port:%d  queue:%d  wrr weight:%d", port, queue, weight.min_weight);
            rc = air_qos_setScheduleMode(unit, port, queue, AIR_QOS_SCH_MODE_WRR, weight.min_weight);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d), weight is %u ", rc, weight.min_weight);
                return MW_E_OP_INCOMPLETE;
            }
        }
        else if(PORT_QOS_Q0_WEIGHT_WFQ <= ptr_api_arg->ptr_type->f_idx)
        {
            /*WFQ setting*/
            weight.max_weight = 0;
            memcpy(&weight.max_weight, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
            queue = ptr_api_arg->ptr_type->f_idx - PORT_QOS_Q0_WEIGHT_WFQ;
            SYNCD_LOG_DEBUG("port:%d  queue:%d  wfq weight:%d", port, queue, weight.max_weight);
            rc = air_qos_setScheduleMode(unit, port, queue, AIR_QOS_SCH_MODE_WFQ, weight.max_weight);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Call function failed(%d), weight is %u ", rc, weight.max_weight);
                return MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            SYNCD_LOG_ERROR("No qos weight got, please check");
            return MW_E_OP_INCOMPLETE;
        }
    }
    else
    {
        if(DB_ALL_ENTRIES == ptr_api_arg ->ptr_type ->f_idx)
        {
            SYNCD_LOG_ERROR("get wrong f_idx is %d", ptr_api_arg ->ptr_type ->f_idx);
            return MW_E_BAD_PARAMETER;
        }

        memcpy(&weight_all, (UI8_T*)ptr_api_arg->ptr_data, ptr_api_arg ->data_size);
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                SYNCD_LOG_DEBUG("Port-%u is CPU port, not support", port);
                continue;
            }
            if (0 == weight_all[port-1])
            {
               weight_all[port-1] = default_weight;
            }
            if (PORT_QOS_Q7_WEIGHT_WRR >= ptr_api_arg->ptr_type->f_idx && PORT_QOS_Q0_WEIGHT_WRR <= ptr_api_arg->ptr_type->f_idx)
            {
                /*WRR setting*/
                weight.min_weight = weight_all[port-1];
                queue = ptr_api_arg->ptr_type->f_idx - PORT_QOS_Q0_WEIGHT_WRR;
                SYNCD_LOG_DEBUG("port:%d  queue:%d  wrr weight:%d", port, queue, weight.min_weight);
                rc = air_qos_setScheduleMode(unit, port, queue, AIR_QOS_SCH_MODE_WRR, weight.min_weight);
                if(AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d) weight is %u", rc, weight.min_weight);
                    mw_rc = MW_E_OP_INCOMPLETE;
                }
            }
            else if(PORT_QOS_Q0_WEIGHT_WFQ <= ptr_api_arg->ptr_type->f_idx)
            {
                /*WFQ setting*/
                weight.max_weight = weight_all[port-1];
                queue = ptr_api_arg->ptr_type->f_idx - PORT_QOS_Q0_WEIGHT_WFQ;
                SYNCD_LOG_DEBUG("port:%d  queue:%d  wfq weight:%d", port, queue, weight.max_weight);
                rc = air_qos_setScheduleMode(unit, port, queue, AIR_QOS_SCH_MODE_WFQ, weight.max_weight);
                if(AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Call function failed(%d) weight is %u", rc, weight.max_weight);
                    mw_rc = MW_E_OP_INCOMPLETE;
                }
            }
            else
            {
                SYNCD_LOG_ERROR("No qos weight got, please check");
                mw_rc = MW_E_OP_INCOMPLETE;
            }
        }
    }
    return mw_rc;
}
