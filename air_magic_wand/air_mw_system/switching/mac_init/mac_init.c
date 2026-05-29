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

/* FILE NAME:   mac_init.c
 * PURPOSE:
 *      Implement initialization function of MAC.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "mw_utils.h"
#include "mw_log.h"
#include "db_api.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "osapi_message.h"
#include "osapi_thread.h"
#include "mac_init.h"
#include "mw_platform.h"
#include "mw_portbmp.h"
#include "sys_mgmt.h"

#include "air_port.h"
#include "air_swc.h"
#include "air_port.h"
#include "air_l2.h"
#include "air_qos.h"
#include "air_sec.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAC_INIT_NAME           "mac_init"
#define MAC_INIT_MSG_NAME       "mac"
#define MAC_INIT_MSG_LEN        (2)
#define MAC_INIT_MSG_TIMEOUT    (10000)
#define MAC_INIT_STACK_SIZE     (192)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static threadhandle_t _ptr_mac_init_handle = NULL;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void
_mac_init(
    void *ptr_pvParameters)
{
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    DB_PAYLOAD_T *ptr_payload = NULL;
    UI8_T *ptr_data = NULL;
    UI32_T msg_size = 0;

    /* Create message */
    mw_rc = osapi_msgCreate(
        MAC_INIT_MSG_NAME,
        MAC_INIT_MSG_LEN,
        DB_MSG_PTR_SIZE);
    if (MW_E_OK != mw_rc)
    {
        MW_LOG_INIT_PRINTF("Create message failed(%d)\n", mw_rc);
        osapi_processDelete(_ptr_mac_init_handle);
        return;
    }

    /* Allocate message buffer */
    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + sizeof(MW_MAC_T);
    mw_rc = osapi_calloc(
            msg_size,
            MAC_INIT_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != mw_rc)
    {
        MW_LOG_INIT_PRINTF("Allocate memory failed(%d)\n", mw_rc);
        osapi_msgDelete(MAC_INIT_MSG_NAME);
        osapi_processDelete(_ptr_mac_init_handle);
        return;
    }

    /* Fill message */
    osapi_strncpy(ptr_msg ->cq_name, MAC_INIT_MSG_NAME, DB_MSG_PTR_SIZE);
    ptr_msg ->method = M_UPDATE;
    ptr_msg ->type.count = 1;

    /* Fill payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg ->ptr_payload);
    ptr_payload ->request.t_idx = SYS_OPER_INFO;
    ptr_payload ->request.f_idx = SYS_OPER_MAC;
    ptr_payload ->request.e_idx = 1;
    ptr_payload ->data_size = sizeof(MW_MAC_T);

    /* Fill data */
    ptr_data = (UI8_T *)&ptr_payload ->ptr_data;
    osapi_memcpy(ptr_data, PLAT_MAC_ADDR, ptr_payload ->data_size);

    /* Waitting until DB is ready */
    do{
        mw_rc = dbapi_dbisReady();
        if (MW_E_OK != mw_rc)
        {
            osapi_delay(10);
        }
        else
        {
            break;
        }
    }while(1);

    /* Send message to set MAC address of CPU */
    mw_rc = dbapi_sendRequesttoDb(msg_size, ptr_msg);
    if (MW_E_OK != mw_rc)
    {
        /* This message could not be send, drop it */
        MW_LOG_INIT_PRINTF("Send to DB failed\n");
        MW_FREE(ptr_msg);
        osapi_msgDelete(MAC_INIT_MSG_NAME);
        osapi_processDelete(_ptr_mac_init_handle);
        return;
    }

    /* Receive response message from DB */
    mw_rc = osapi_msgRecv(
        MAC_INIT_MSG_NAME,
        (UI8_T **)&ptr_msg,
        0,
        MAC_INIT_MSG_TIMEOUT);
    if (MW_E_OK != mw_rc)
    {
        MW_LOG_INIT_PRINTF("Receive queue from DB failed\n");
        MW_FREE(ptr_msg);
        osapi_msgDelete(MAC_INIT_MSG_NAME);
        osapi_processDelete(_ptr_mac_init_handle);
        return;
    }

    /* Release message */
    MW_FREE(ptr_msg);
    /* Delete Queue */
    osapi_msgDelete(MAC_INIT_MSG_NAME);
    /* Delete Process */
    osapi_processDelete(_ptr_mac_init_handle);

    return;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: mac_init
 * PURPOSE:
 *      Initialize MAC related setting.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
mac_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T unit = 0;
    MW_PORT_BITMAP_T port_bitmap;
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    AIR_SWC_MGMT_FRAME_CFG_T mgmt = {0};
    AIR_SEC_MAC_LIMIT_PORT_CFG_T sec_p_cfg;
    AIR_SEC_MAC_LIMIT_CFG_T global_cfg = {0};
    UI16_T i;
    UI16_T size;
    const UI16_T ctrl_frm_only_to_CPU[] =
    {
    };

    const UI16_T ctrl_frm_default[] =
    {
        AIR_SWC_MGMT_FRAME_TYPE_IGMP,
        AIR_SWC_MGMT_FRAME_TYPE_PPPOE,
        AIR_SWC_MGMT_FRAME_TYPE_ARP,
        AIR_SWC_MGMT_FRAME_TYPE_PAE,
        AIR_SWC_MGMT_FRAME_TYPE_DHCP,
        AIR_SWC_MGMT_FRAME_TYPE_BPDU,
        AIR_SWC_MGMT_FRAME_TYPE_TTL_0,
        AIR_SWC_MGMT_FRAME_TYPE_MLD,
        AIR_SWC_MGMT_FRAME_TYPE_REV_01,
        AIR_SWC_MGMT_FRAME_TYPE_REV_02,
        AIR_SWC_MGMT_FRAME_TYPE_REV_03,
        AIR_SWC_MGMT_FRAME_TYPE_REV_0E,
        AIR_SWC_MGMT_FRAME_TYPE_REV_10,
        AIR_SWC_MGMT_FRAME_TYPE_REV_20,
        AIR_SWC_MGMT_FRAME_TYPE_REV_21,
        AIR_SWC_MGMT_FRAME_TYPE_REV_UN,
#ifdef AIR_EN_CORAL
        AIR_SWC_MGMT_FRAME_TYPE_LLDP
#endif
    };

    MW_LOG_INIT_PRINTF("Initializing MAC address...\n");

    /* Create a process to get MAC address of CPU port from global config and set to DB */
    rc = osapi_processCreate(
        MAC_INIT_NAME,
        MAC_INIT_STACK_SIZE,
        MW_TASK_PRIORITY_MAC_INIT,
        _mac_init,
        NULL,
        &_ptr_mac_init_handle);
    if (MW_E_OK != rc)
    {
        MW_LOG_INIT_PRINTF("Failed to create Process(%d)!\n", rc);
    }

    /* Set those control frames only forwarded to CPU */
    size = sizeof(ctrl_frm_only_to_CPU)/sizeof(UI16_T);
    for (i = 0; i < size; i++)
    {
        mgmt.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        mgmt.frame_type = ctrl_frm_only_to_CPU[i];
        mgmt.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
        air_rc = air_swc_setMgmtFrameCfg(unit, &mgmt);
        if (AIR_E_OK != air_rc)
        {
            MW_LOG_INIT_PRINTF("Failed to set Mgmt Frame(0x%x) for cpu only!\n", i);
        }
    }

    /* Set those control frames forwarded by default */
    size = sizeof(ctrl_frm_default)/sizeof(UI16_T);
    for (i = 0; i < size; i++)
    {
        mgmt.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        mgmt.frame_type = ctrl_frm_default[i];
        mgmt.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
        air_rc = air_swc_setMgmtFrameCfg(unit, &mgmt);
        if (AIR_E_OK != air_rc)
        {
            MW_LOG_INIT_PRINTF("Failed to set Mgmt Frame(0x%x) for default!\n", i);
        }
    }
    /* Set forwarding rule of unknown frames for all front ports except cpu port */
    AIR_PORT_BITMAP_COPY(port_bitmap, PLAT_PORT_BMP_TOTAL);
    AIR_PORT_DEL(port_bitmap, PLAT_CPU_PORT);
    for (i = 0; i < AIR_FORWARD_TYPE_LAST; i++)
    {
        air_rc = air_l2_setForwardMode(unit, i, AIR_FORWARD_ACTION_TO_PBM, port_bitmap);
        if (AIR_E_OK != air_rc)
        {
            MW_LOG_INIT_PRINTF("Failed to set forward mode for type %d!\n", i);
        }
    }

    /* Disable SA-learning of CPU port */
    memset(&sec_p_cfg, 0, sizeof(sec_p_cfg));
    air_rc = air_sec_setMacLimitPortCfg(unit, PLAT_CPU_PORT, &sec_p_cfg);
    if (AIR_E_OK != air_rc)
    {
        MW_LOG_INIT_PRINTF("Failed to config CPU port(%u)!\n", PLAT_CPU_PORT);
    }
    /* Enable global mac limit drop */
    air_rc = air_sec_setMacLimitGlobalMode(unit, TRUE);
    air_rc |= air_sec_getMacLimitGlobalCfg(unit, &global_cfg);
    global_cfg.flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL;
    air_rc |= air_sec_setMacLimitGlobalCfg(unit, &global_cfg);
    if (AIR_E_OK != air_rc)
    {
        MW_LOG_INIT_PRINTF("Failed to set global mac limit drop!\n");
    }
    return;
}

