/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  igmp_snoop.c
 * PURPOSE:
 * It provides IGMP Snooping module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_acl.h"
#include "igmp_snoop_mode.h"
#include "igmp_snoop_packet.h"
#include "igmp_snoop_queue.h"
#include "igmp_snoop_port.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_msg.h"
#include "igmp_snoop_lag.h"
#include "igmp_snoop_db.h"
#endif
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_timer.h"
#include "osapi_thread.h"
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier.h"
#include "igmp_querier_db.h"
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
#include "igmp_snp_config_customer.h"
#endif
#include "mw_utils.h"

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
static void
_igmp_snp_task(
    void *ptr_param);

/* STATIC VARIABLE DECLARATIONS
 */
static IGMP_SNP_LIST_T      igmp_snp;
static threadhandle_t       _igmp_snp_task_handle = NULL;
static StackType_t          _igmp_snp_task_stack[IGMP_SNP_TASK_STACK_SIZE] = {0};
static StaticTask_t         _igmp_snp_task_tcb;

/* LOCAL SUBPROGRAM BODIES
 */
static void
_igmp_snp_task(
    void *ptr_param)
{
    MW_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = MW_E_OK;
#ifdef IGMP_SNP_MW_SUPPORT
    IGMP_SNP_MSG_T      *ptr_task_msg = NULL;

    igmp_snp_db_subscribeModule();
#endif
    /* Just to kill the compiler warning. */
    UNUSED(ptr_param);
    while(1)
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        igmp_snp_timer_handleExpirationEvent();
        rc = osapi_msgRecv(IGMP_SNP_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, IGMP_SNP_TASK_DELAY);
        if(MW_E_OK == rc)
        {
            switch (ptr_msg->msg_id)
            {
                case MW_MSG_ID_ETHERNET_PBUF:
                {
                    NET_MSG_T *ptr_pkt_msg = (NET_MSG_T *)ptr_msg;

                    if(NULL != ptr_pkt_msg->ptr_pbuf)
                    {
                        IGMP_SNP_LOG_DEBUG("->igmp_snp_packet_input()");
                        igmp_snp_packet_input(ptr_pkt_msg->ptr_pbuf);
                        IGMP_SNP_LOG_DEBUG("->igmp_snp_packet_input(), Done");
                    }
                    break;
                }

#ifdef IGMP_SNP_MW_SUPPORT
                case MW_MSG_ID_DB:
                {
                    igmp_snp_db_handleMsg((DB_MSG_T *)ptr_msg);
                    break;
                }

                case IGMP_SNP_MSG_CLEAR_ENTRY:
                {
                    ptr_task_msg = (IGMP_SNP_MSG_T *)ptr_msg;
                    IGMP_SNP_MSG_CLEAR_ENTRY_T clear_entry = {0};

                    osapi_memcpy(&clear_entry, ptr_task_msg->data, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));
                    igmp_snp_clearEntryByType((void * )&clear_entry);
                    break;
                }

#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
                case IGMP_SNP_MSG_UPDATE_PORT_GROUP_ENTRY:
                {
                    MW_PORT_BITMAP_T    portbmp = {0};

                    ptr_task_msg = (IGMP_SNP_MSG_T *)ptr_msg;
                    osapi_memcpy(portbmp, ptr_task_msg->data, sizeof(MW_PORT_BITMAP_T));
                    igmp_snp_port_updateEntry(portbmp);
                    break;
                }
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

#endif /* IGMP_SNP_MW_SUPPORT */

                default:
                {
                    /* unknown notification */
                    IGMP_SNP_LOG_DEBUG("Receive unknown notification msg_id:%d", ptr_msg->msg_id);
                    break;
                }
            }

            MW_FREE(ptr_msg);
        }
    }
}

/* FUNCTION NAME:   igmp_snp_deinit
 * PURPOSE:
 *      Free the resources in IGMP snooping init function.
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
igmp_snp_deinit(
    void)
{
    igmp_snp_queue_deinit();
    igmp_snp_timer_deinit();
    if(NULL != _igmp_snp_task_handle)
    {
        osapi_threadDelete(_igmp_snp_task_handle);
        _igmp_snp_task_handle = NULL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_init
 * PURPOSE:
 *      This IGMP snooping init function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
    const IGMP_SNP_CUSTOMER_CONFIG_FLAGS_T *ptr_customer_config = igmp_snp_getCustomerConfig();
#endif

    if(NULL != _igmp_snp_task_handle)
    {
        IGMP_SNP_LOG_ERROR("igmp snoop task already created\n");
        return MW_E_OTHERS;
    }
    MW_LOG_INIT_PRINTF("Initializing IGMP Snooping...\n");
    osapi_memset(&igmp_snp, 0, sizeof(IGMP_SNP_LIST_T));
    /* Initial loop list */
    CSLIST_INIT(&igmp_snp.group_head);
    CSLIST_INIT(&igmp_snp.mrouter_head);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    CSLIST_INIT(&igmp_snp.querier_head);
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    igmp_snp_vlan_initVariable();
    igmp_snp_acl_initVariable();
    igmp_snp_packet_initVariable();
#ifdef IGMP_SNP_MW_SUPPORT
    igmp_snp_db_initVariable();
    igmp_snp_lag_initVariable();
#else
    igmp_snp.cfg_info.fast_leave = ENABLE;
#endif

#ifdef AIR_SUPPORT_IGMPV3_AWARE
#ifdef IGMP_SNP_MW_SUPPORT
    igmp_snp_setV3AwareMode(ENABLE);
#else
#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
    igmp_snp_setV3AwareMode(ptr_customer_config->igmpv3_aware_enable);
#endif
#endif /* IGMP_SNP_MW_SUPPORT */
#endif /* AIR_SUPPORT_IGMPV3_AWARE */

#ifdef IGMP_SNP_CUSTOMER_CONFIG_SUPPORT
    igmp_snp_setAdminMode(ptr_customer_config->igmp_snp_enable);
    igmp_snp_setUnIpmcForwardMode(ptr_customer_config->igmp_uipmc_drop);
#endif

    do
    {
        ret = igmp_snp_queue_init();
        if(MW_E_OK != ret)
        {
            break;
        }

        /* Create IGMP Snooping task(static) */
        ret = osapi_threadCreateStatic(IGMP_SNP_TASK_NAME,
                                        IGMP_SNP_TASK_STACK_SIZE,
                                        MW_TASK_PRIORITY_IGMPSNP,
                                        _igmp_snp_task,
                                        NULL,
                                        _igmp_snp_task_stack,
                                        &_igmp_snp_task_tcb,
                                        &_igmp_snp_task_handle);

        if(MW_E_OK != ret)
        {
            IGMP_SNP_LOG_ERROR("xTaskCreateStatic for IGMP SNP failed !");
            break;
        }

        /* Create timer */
        ret = igmp_snp_timer_init();
        if (MW_E_OK != ret)
        {
            break;
        }
        ret = igmp_snp_timer_start();
        if (MW_E_OK != ret)
        {
            IGMP_SNP_LOG_ERROR("Error: start IGMP SNP timer fail");
            break;
        }

    } while (0);

    if (MW_E_OK != ret)
    {
        IGMP_SNP_LOG_ERROR("IGMP SNP nit failed, ret:%d", ret);
        igmp_snp_deinit();
        return MW_E_NOT_INITED;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_getConfig
 * PURPOSE:
 *      This API is used for MW to get static variable of igmp_snp.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of igmp_snp
 *
 * NOTES:
 *      None
 */
IGMP_SNP_LIST_T *
igmp_snp_getConfig(
    void)
{
    return (IGMP_SNP_LIST_T *)&igmp_snp;
}
