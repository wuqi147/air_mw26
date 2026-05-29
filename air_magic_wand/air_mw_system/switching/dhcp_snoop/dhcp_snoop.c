/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:   dhcp_snoop.c
 * PURPOSE:
 *      Define DHCP snooping main function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>
#include <dhcp_snoop_log.h>
#include <dhcp_snoop_db.h>
#include <dhcp_snoop_pkt.h>
#include <sys_mgmt.h>

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
static DHCP_SNOOP_TASK_T dhcpsnp_task_ctx = {0};

/* LOCAL SUBPROGRAM BODIES
 */

static inline void
_dhcp_snp_checkDbReady(
    void)
{
    /* Check DB is ready */
    while (dbapi_dbisReady() != MW_E_OK)
    {
        dhcp_snp_dbg("DB is not ready");
        osapi_delay(DHCP_SNP_DB_READY_DELAY);
    }
}

static void
_dhcp_snp_task(
    void *ptr_args)
{
    MW_ERROR_NO_T           rc        = MW_E_OK;
    NET_MSG_T               *ptr_msg  = NULL;

    dhcp_snp_profiling_init();

    UNUSED(ptr_args);

    _dhcp_snp_checkDbReady();

    /* Subscribe API items to DB */
    dhcp_snp_db_subscribe();

    /* main thread */
    while (1)
    {
        ptr_msg = NULL;
        rc = osapi_msgRecv(DHCP_SNP_QUEUE_NAME,
                           (UI8_T **)&ptr_msg,
                           DB_MSG_PTR_SIZE,
                           DHCP_SNP_TASK_DELAY);
        if (rc == MW_E_OK && ptr_msg != NULL)
        {
            dhcp_snp_dbg("ptr_msg = 0x%p, msg_id = %d", ptr_msg, ptr_msg->msg_id);
            if (ptr_msg->msg_id == DHCP_SNP_MSG_PKT)
            {
                dhcp_snp_profiling_start();
                dhcp_snp_pkt_process((DHCP_SNP_PKT_MSG_T *)ptr_msg);
                dhcp_snp_profiling_end("Packet main process");
            }
            else if (ptr_msg->msg_id == DHCP_SNP_MSG_DB)
            {
                dhcp_snp_profiling_start();
                dhcp_snp_db_handle((DB_MSG_T *)ptr_msg);
                dhcp_snp_profiling_end("DB handle process");
            }
            /* Free message buffer */
            osapi_free(ptr_msg);
        }
    }
    dhcp_snp_dbg("Never reach here");
}

static MW_ERROR_NO_T
_dhcp_snp_task_deinit(
    void)
{
    if (dhcpsnp_task_ctx.task_handle)
    {
        osapi_threadDelete(dhcpsnp_task_ctx.task_handle);
        dhcpsnp_task_ctx.task_handle = NULL;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_task_init(
    void)
{
    if (osapi_threadCreateStatic(DHCP_SNP_MODULE_NAME,
                                 DHCP_SNP_TASK_STACK_SIZE,
                                 MW_TASK_PRIORITY_DHCPSNP,
                                 _dhcp_snp_task,
                                 NULL,
                                 dhcpsnp_task_ctx.task_stack,
                                 &dhcpsnp_task_ctx.task_tcb,
                                 &dhcpsnp_task_ctx.task_handle) != MW_E_OK)
    {
        dhcp_snp_err("osapi_threadCreate for DHCP SNP failed !");
        return MW_E_NO_MEMORY;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_msgctrl_deinit(
    void)
{
    osapi_msgDelete(DHCP_SNP_QUEUE_NAME);
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_msgctrl_init(
    void)
{
    if (osapi_msgCreateStatic(DHCP_SNP_QUEUE_NAME,
                              DHCP_SNP_QUEUE_LENGTH,
                              sizeof(void *),
                              (UI8_T *)dhcpsnp_task_ctx.queue_pool,
                              &dhcpsnp_task_ctx.queue_handle) != MW_E_OK)
    {
        dhcp_snp_err("osapi_msgCreate %s fail", DHCP_SNP_QUEUE_NAME);
        return MW_E_NO_MEMORY;
    }
    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_deinit
 * PURPOSE:
 *      Deinit DHCP snooping function
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
dhcp_snp_deinit(
    void)
{
    _dhcp_snp_task_deinit();
    dhcp_snp_pkt_deinit();
    dhcp_snp_db_deinit();
    _dhcp_snp_msgctrl_deinit();
    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_init
 * PURPOSE:
 *      Init DHCP snooping function
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
dhcp_snp_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_LOG_INIT_PRINTF("Initializing DHCP snooping...\n");

    if ((rc = _dhcp_snp_msgctrl_init()) == MW_E_OK)
    {
        dhcp_snp_dbg("msgctrl init OK");
    }
    if (rc == MW_E_OK)
    {
        if ((rc = dhcp_snp_db_init()) == MW_E_OK)
        {
            dhcp_snp_dbg("db init OK");
        }
    }
    if (rc == MW_E_OK)
    {
        if ((rc = dhcp_snp_pkt_init()) == MW_E_OK)
        {
            dhcp_snp_dbg("packet init OK");
        }
    }
    if (rc == MW_E_OK)
    {
        if ((rc = _dhcp_snp_task_init()) == MW_E_OK)
        {
            dhcp_snp_dbg("task init OK");
        }
    }
    if (rc != MW_E_OK)
    {
        dhcp_snp_deinit();
        return MW_E_NOT_INITED;
    }
    return MW_E_OK;
}

