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

/* FILE NAME:  igmp_snoop_timer.c
 * PURPOSE:
 *    This file contains the implementation of the igmp snooping timer APIs.
 *
 * NOTES:
 *
 */
/*INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_timer.h"
#include "igmp_snoop.h"
#include "igmp_snoop_utils.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_port.h"
#include "mw_msg.h"
#include "osapi_timer.h"
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier_timer.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static UI8_T   _igmp_snp_timer_expire_count = 0;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void
_igmp_snp_timer(
    TimerHandle_t xTimer);

static MW_ERROR_NO_T
_igmp_snp_timer_handleGroupTimeOutEvent(
    void **pptr_grp,
    UI8_T port_id);

static MW_ERROR_NO_T
_igmp_snp_timer_handleReportSuppTimeOutEvent(
    void **pptr_grp);

static MW_ERROR_NO_T
_igmp_snp_timer_handleMrouterTimeOutEvent(
    void **pptr_mrtr,
    UI32_T port_id);

/* STATIC VARIABLE DECLARATIONS
 */
static TimerHandle_t        _igmp_snp_timer_handle;

/* LOCAL SUBPROGRAM BODIES
 */
static void
_igmp_snp_timer(
    TimerHandle_t xTimer)
{
    if(_igmp_snp_timer_expire_count < IGMP_SNP_MAX_TMR_EXPIRE_CNT)
    {
        _igmp_snp_timer_expire_count++;
    }
    return;
}

/* FUNCTION NAME:   _igmp_snp_timer_handleGroupTimeOutEvent
 * PURPOSE:
 *      IGMP snooping group time-out function.
 *      Refer to RFC2236 8.4 Group Membership Interval.
 *
 * INPUT:
 *      pptr_grp     --  the pointer to pointer of removing group entry
 *      port_id      --  ingress front port ID
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
static MW_ERROR_NO_T
_igmp_snp_timer_handleGroupTimeOutEvent(
    void **pptr_grp,
    UI8_T port_id)
{
    IGMP_SNP_LIST_T      *ptr_igmp_snp = NULL;

    MW_CHECK_PTR(pptr_grp);
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = *pptr_grp;
    MW_CHECK_PTR(ptr_grp);
    IGMP_SNP_LOG_DEBUG("vid=%d, port=%d",
                    ptr_grp->vlan_id, port_id);

    ptr_igmp_snp = igmp_snp_getConfig();
    if((0 == port_id) || (NULL == ptr_igmp_snp))
    {
        return MW_E_BAD_PARAMETER;
    }
    MW_PORT_DEL(ptr_grp->portbmp, port_id);
    ptr_grp->leave_flag_bmp &= ~(1 << port_id);
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
    ptr_igmp_snp->port_group_num[(port_id - 1)]--;
#endif

    if (0 != ptr_grp->portbmp[0])
    {
        igmp_snp_updateGroupEntry(ptr_grp);
    }
    else
    {
        igmp_snp_delGroupEntry(pptr_grp);
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   _igmp_snp_timer_handleReportSuppTimeOutEvent
 * PURPOSE:
 *      IGMP snooping report suppress time-out function.
 *
 * INPUT:
 *      pptr_grp      --  the pointer to pointer of the group entry
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
static MW_ERROR_NO_T
_igmp_snp_timer_handleReportSuppTimeOutEvent(
    void **pptr_grp)
{
    MW_CHECK_PTR(pptr_grp);
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp = *pptr_grp;
    MW_CHECK_PTR(ptr_grp);
    ptr_grp->rpt_suppress_flag = FALSE;

    return MW_E_OK;
}

/* FUNCTION NAME:   _igmp_snp_timer_handleMrouterTimeOutEvent
 * PURPOSE:
 *      IGMP snooping mrouter time-out function.
 *
 * INPUT:
 *      pptr_mrtr    --  the pointer to pointer of the mrouter entry
 *      port_id      --  ingress front port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 */
static MW_ERROR_NO_T
_igmp_snp_timer_handleMrouterTimeOutEvent(
    void **pptr_mrtr,
    UI32_T port_id)
{
    return igmp_snp_port_delMrouterPort(pptr_mrtr, port_id);
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   lp_timer_init
 * PURPOSE:
 *      This API is used to initial IGMP SNP software timer.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_timer_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    rc = osapi_timerCreate(IGMP_SNP_TIMER_NAME, _igmp_snp_timer, TRUE, IGMP_SNP_TMR_INTERVAL, NULL, &_igmp_snp_timer_handle);
    if (MW_E_OK != rc)
    {
        /* Error create timer */
        IGMP_SNP_LOG_ERROR("Error: create IGMP SNP timer fail\n");
        return MW_E_NOT_INITED;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_timer_deinit
 * PURPOSE:
 *      This API is used to de-initial IGMP SNP software timer.
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
igmp_snp_timer_deinit(
    void)
{
    if (NULL != _igmp_snp_timer_handle)
    {
        if(MW_E_OK == osapi_timerActive(_igmp_snp_timer_handle))
        {
            osapi_timerStop(_igmp_snp_timer_handle);
        }
        osapi_timerDelete(_igmp_snp_timer_handle);
        _igmp_snp_timer_handle = NULL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_timer_start
 * PURPOSE:
 *      This API is used to start IGMP SNP software timer.
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
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_timer_start(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL == _igmp_snp_timer_handle)
    {
        return MW_E_OTHERS;
    }
    if(MW_E_OK != osapi_timerActive(_igmp_snp_timer_handle))
    {
        rc = osapi_timerStart(_igmp_snp_timer_handle);
    }
    return rc;
}

/* FUNCTION NAME:   igmp_snp_timer_handleExpirationEvent
 * PURPOSE:
 *      IGMP snooping check timer timer timeout function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
igmp_snp_timer_handleExpirationEvent(
    void)
{
    IGMP_SNP_LIST_T           *ptr_igmp_snp = NULL;
    UI32_T                    i = 0;
    UI8_T                     timer_count = 1;

    while (0 < _igmp_snp_timer_expire_count)
    {
        ptr_igmp_snp = igmp_snp_getConfig();
        if((NULL != ptr_igmp_snp) && (ENABLE == ptr_igmp_snp->cfg_info.enable))
        {
            struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
            struct IGMP_SNP_GROUP_LIST_S   *ptr_grp_next = NULL;
            struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
            struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr_next = NULL;

            /* Suppress timer of general QUERY */
            ptr_igmp_snp->glb_suppress_timer = ((ptr_igmp_snp->glb_suppress_timer > timer_count) ? (ptr_igmp_snp->glb_suppress_timer - timer_count) : 0);
            CSLIST_FOREACH_SAFE(ptr_grp, ptr_grp_next, &(ptr_igmp_snp->group_head), next)
            {
                /* Suppress timer of specific QUERY */
                if (ptr_grp->rpt_suppress_timer > 0)
                {
                    /* Update the remaining expiration time of timer */
                    ptr_grp->rpt_suppress_timer = ((ptr_grp->rpt_suppress_timer > timer_count) ? (ptr_grp->rpt_suppress_timer - timer_count) : 0);
                }
                if((TRUE == ptr_grp->rpt_suppress_flag) &&
                    (0 == ptr_grp->rpt_suppress_timer) &&
                    (0 == ptr_igmp_snp->glb_suppress_timer) &&
                    (0 != ptr_igmp_snp->cfg_info.rpt_suppress))
                {
                    _igmp_snp_timer_handleReportSuppTimeOutEvent((void **)&ptr_grp);
                }

                AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, i)
                {
                    if(PLAT_CPU_PORT == i)
                    {
                        /* Skip cpu port */
                        continue;
                    }
                    if (ptr_grp->timer[(i-1)] > 0)
                    {
                        /* Update the remaining expiration time of timer */
                        ptr_grp->timer[(i-1)] = ((ptr_grp->timer[(i-1)] > timer_count) ? (ptr_grp->timer[(i-1)] - timer_count) : 0);
                        if(0 == ptr_grp->timer[(i-1)])
                        {
                            _igmp_snp_timer_handleGroupTimeOutEvent((void **)&ptr_grp, i);
                            if(NULL == ptr_grp)
                            {
                                /* group entry may be deleted */
                                break;
                            }
                        }
                    }
                }
            }

            CSLIST_FOREACH_SAFE(ptr_mrtr, ptr_mrtr_next, &(ptr_igmp_snp->mrouter_head), next)
            {
                AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, i)
                {
                    if(PLAT_CPU_PORT == i)
                    {
                        /* Skip cpu port */
                        continue;
                    }
                    if (ptr_mrtr->timer[(i-1)] > 0 && (ptr_mrtr->timer[(i-1)] != IGMP_SNP_STATIC_MROUTER))
                    {
                        /* Update the remaining expiration time of timer */
                        ptr_mrtr->timer[(i-1)] = ((ptr_mrtr->timer[(i-1)] > timer_count) ? (ptr_mrtr->timer[(i-1)] - timer_count) : 0);
                        if (0 == ptr_mrtr->timer[(i-1)])
                        {
                            _igmp_snp_timer_handleMrouterTimeOutEvent((void **)&ptr_mrtr, i);
                            if(NULL == ptr_mrtr)
                            {
                                /* mrouter entry may be deleted */
                                break;
                            }
                        }
                    }
                }
            }
#ifndef IGMP_SNP_MW_SUPPORT
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, i)
            {
                if (PLAT_CPU_PORT == i)
                {
                    continue;
                }

#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
                if (0 != ptr_igmp_snp->port_group_num[i - 1])
#endif
                {
                    if (0 == igmp_snp_port_getPortBmpLinkStatus((UI32_T) BIT(i)))
                    {
                        igmp_snp_port_clearEntry(i);
                    }
                }
            }
#endif /* IGMP_SNP_MW_SUPPORT */

        }

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
        igmp_querier_timer_handleExpirationEvent();
#endif
        _igmp_snp_timer_expire_count--;
    }

    return;
}
