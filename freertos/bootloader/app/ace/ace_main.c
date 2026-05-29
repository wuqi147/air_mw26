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

/* FILE NAME:   ace_main.c
 * PURPOSE:
 *      Define Airoha Command Environment main functions.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_main.h"

#include "FreeRTOS.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ethernetif.h"
#include "cmd_interpreter.h"

#include "ace_cmd.h"
#include "ace_cmd_debug.h"
#include "ace_cmd_info.h"
#include "ace_log.h"

extern unsigned int io_read32(unsigned int addr);
extern void io_write32(unsigned int addr, unsigned int vlaue);

/* NAMING CONSTANT DECLARATIONS
 */

#define ETH_PKT_MIN_LEN (64)
#define PAYLOAD_MIN_LEN (50)
#define VLAN_NUM        (0)
#define PHY_PORT_PBMP             (0xff)

#define ACL_UDF_ACE_IDX           (0x0F)

#define ACE_SYS_TRAP_MODE         (0x0200)
#define ACE_SYS_BOOT_MODE         (0x0201)

#define MAC_PORT_PBMP             (0x0fffffff)

#if defined(AIR_8851_SUPPORT)
#define AIR_CPU_PORT              (28)
#define ACL_ACE_START_IDX         (0)  /* 1 rule */
#elif defined(AIR_8855_SUPPORT)
#define AIR_CPU_PORT              (6)
#define ACL_ACE_START_IDX         (7)
#elif defined(AIR_8858_SUPPORT)
#define AIR_CPU_PORT              (10)
#define ACL_ACE_START_IDX         (0)
#else
    #error "Not implemented"
#endif


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

extern PLAT_SYS_INFO_T g_sys_info;
static TaskHandle_t g_ace_task = NULL;
static QueueHandle_t g_ace_queue_set = NULL;
QueueHandle_t        ACE_pkt_reg_handle;
NET_FILTER_T         *ACE_pkt_netf = NULL;
uint32_t             ACE_timestamp = 0;
uint32_t             g_trap_mode_acl = 0;
uint32_t             g_trap_mode_flag = 0;
uint32_t             g_trap_mode_cmd = 0;

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   ACE_free_resource
 * PURPOSE:
 *      Free the resources in ACE module.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ACE_E_OK
 *
 * NOTES:
 *      None
 */
ACE_ERR_T
ACE_free_resource(void)
{
    ACE_ERR_T rc;

    /* clear UDF rule */
    io_write32(0x10200208, 0x0);
    io_write32(0x1020020c, 0x0);
    io_write32(0x10200210, 0x0);
    io_write32(0x10200200, 0x9000000f);

    if (ACE_pkt_reg_handle)
    {
        vQueueDelete(ACE_pkt_reg_handle);
    }
    ACE_pkt_netf->state = NET_FILTER_DEREGISTER;
    QueueHandle_t regHandle = NULL;
    int res = ethernetif_regHandleFind(&regHandle);
    if ((0 != res) || (NULL == regHandle))
    {
        rc = -1;
    }
    else
    {
        if (pdPASS != xQueueSend(regHandle, &ACE_pkt_netf, (100 / portTICK_RATE_MS)))
        {
            rc = -1;
        }
    }
    if (ACE_E_OK != rc)
    {
        ACE_DBG_PRINT("Error: NET_FILTER_DEREGISTER failed! error code: %d\n", rc);
    }

    vPortFree(ACE_pkt_netf);
    vTaskDelete(g_ace_task);
    vQueueDelete(g_ace_queue_set);

    return ACE_E_OK;
}

/* FUNCTION NAME:   ACE_send
 * PURPOSE:
 *      Send ACE packet by ethernet frame.
 *
 * INPUT:
 *      ptr_ace_pkt
 *      ptr_pbuf_in
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ACE_E_OK
 *
 * NOTES:
 *      None
 */
void
ACE_send(ACE_PKT_ACK_T *ptr_ace_pkt, struct pbuf *ptr_pbuf_in)
{
    struct pbuf   *ptr_pbuf  = NULL;
    uint16_t      port_id = -1;

    if (NULL != ptr_pbuf_in)
    {
        port_id = ptr_pbuf_in->stag_hdr.rx_hdr.sp;
    }
    else
    {
#ifdef AIR_SUPPORT_ACE_ETHERNET_LOG
        port_id = g_log_port;
#endif
    }
    uint16_t pbuf_len_0 = 0;
    uint8_t *ptr_ace_payload = NULL;

    int32_t ace_pkt_len = sizeof(ptr_ace_pkt->hdr.pktId) +
                          sizeof(ptr_ace_pkt->hdr.type) +
                          sizeof(ptr_ace_pkt->hdr.length) +
                          ptr_ace_pkt->hdr.length;
    int32_t payload_len = 0;
    /* "ethernet packet length must be larger or equal to 64 bytes */
    if (SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + VLAN_NUM * SIZEOF_VLAN_HDR + ace_pkt_len >= ETH_PKT_MIN_LEN)
    {
        payload_len = SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + VLAN_NUM * SIZEOF_VLAN_HDR + ace_pkt_len;
    }
    else
    {
        payload_len = SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + VLAN_NUM * SIZEOF_VLAN_HDR + PAYLOAD_MIN_LEN;
    }

    ptr_pbuf = pbuf_alloc(payload_len);
    if (NULL != ptr_pbuf)
    {
        memset(ptr_pbuf->payload, 0, payload_len);
        if (NULL != ptr_pbuf_in)
        {
            memcpy(ptr_pbuf->ether_hdr.dest.addr, ptr_pbuf_in->ether_hdr.src.addr, MAC_ADDRESS_LEN);
        }
        else
        {
#ifdef AIR_SUPPORT_ACE_ETHERNET_LOG
            memcpy(ptr_pbuf->ether_hdr.dest.addr, &g_da_ether_hdr, MAC_ADDRESS_LEN);
#endif
        }

        memcpy(ptr_pbuf->ether_hdr.src.addr, g_sys_info.mac_addr, MAC_ADDRESS_LEN);
        ptr_pbuf->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = 0;
        ptr_pbuf->vlan_num = VLAN_NUM;

        ptr_ace_payload = ptr_pbuf->payload + (SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + (ptr_pbuf->vlan_num * SIZEOF_VLAN_HDR));
        pbuf_len_0 = ptr_pbuf->len - (SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + (ptr_pbuf->vlan_num * SIZEOF_VLAN_HDR));

        memcpy(ptr_ace_payload, ptr_ace_pkt, ace_pkt_len);
        if (PHY_PORT_PBMP | (1 << port_id))
        {
            ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << port_id);
            ptr_pbuf->len = pbuf_len_0;
            ptr_pbuf->payload = ptr_ace_payload;

            if (0 == g_ace_log_enabled)
            {
                ACE_DBG_PRINT("send ACE ethernet frame to port %d\n", port_id);
            }
            ethernet_output(ptr_pbuf, &ptr_pbuf->ether_hdr.src, &ptr_pbuf->ether_hdr.dest, ETHTYPE_ACE);
        }
        pbuf_free(ptr_pbuf);
    }
    return;
}

/* FUNCTION NAME:   ACE_rx
 * PURPOSE:
 *      Process the rx ACE frame.
 *
 * INPUT:
 *      ptr_pbuf
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void ACE_rx(struct pbuf *ptr_pbuf)
{
    if (0 == ptr_pbuf->len)
    {
        return;
    }

    ACE_PKT_T *ptr_ace_cmd = (ACE_PKT_T *)(ptr_pbuf->payload);
    if (ptr_ace_cmd->hdr.length > ptr_pbuf->len)
    {
        return;
    }
    if (ptr_ace_cmd->hdr.length > 1520)
    {
        return;
    }

#ifdef AIR_SUPPORT_ACE_ETHERNET_LOG
    if (ptr_ace_cmd->hdr.id == ACE_DEBUG_ENABLE_ETHERNET_LOG)
    {
        g_log_port = ptr_pbuf->stag_hdr.rx_hdr.sp;
        ACE_DBG_PRINT("log port: %d\n", (int)g_log_port);
        memcpy(&g_da_ether_hdr, ptr_pbuf->ether_hdr.src.addr, MAC_ADDRESS_LEN);
    }
#endif

    ACE_PKT_ACK_T *ptr_ace_evt = (ACE_PKT_ACK_T *)ACE_CmdHandler(ptr_ace_cmd, ptr_pbuf);
    if (NULL != ptr_ace_evt)
    {
        ACE_send(ptr_ace_evt, ptr_pbuf);
        ACE_FreePacket(ptr_ace_evt);
        /* ACE Trap */
        if (ptr_ace_cmd->hdr.id == ACE_SYS_TRAP_MODE)
        {
            if (g_trap_mode_flag == 0xACE && ptr_ace_cmd->hdr.type == 0x5A)
            {
                g_trap_mode_cmd = 1;
            }
        }
        /* ACE Boot */
        if (ptr_ace_cmd->hdr.id == ACE_SYS_BOOT_MODE)
        {
            if (ptr_ace_cmd->payload[0] == 0)
            {
                /* reset */
                ACE_DBG_PRINT("ACE reset\n");
                char input[16] = {0};
                char output[128] = {0};
                memcpy(input, "reset", strlen("reset"));
                cmd_process(input, output, sizeof(output));
            }
            else if (ptr_ace_cmd->payload[0] == 1)
            {
                /* boot */
                ACE_DBG_PRINT("ACE boot\n");
                char input[16] = {0};
                char output[128] = {0};
                memcpy(input, "boot-rtos", strlen("boot-rtos"));
                cmd_process(input, output, sizeof(output));
            }
        }
    }
    pbuf_free(ptr_pbuf);
    return;
}

/* FUNCTION NAME:   ACE_Process
 * PURPOSE:
 *      This is ACE task.
 *
 * INPUT:
 *      ptr_pvParameters
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
static void
ACE_Process(void *ptr_pvParameters)
{
    ACE_DBG_PRINT("ACE_Process\n");

    struct pbuf *ptr_pbuf;
    QueueSetMemberHandle_t xActivatedMember;
    int rc = 0;
    TickType_t xLastWakeTime;
    /* ACE check trap */
    unsigned int ace_cnt = io_read32(0x10200538);
    if (ace_cnt > 0)
    {
        g_trap_mode_flag = io_read32(0x10005010);
        ACE_DBG_PRINT("Detect trap\n");
        g_trap_mode_acl = 1;
    }

    /* Just to kill the compiler warning. */
    (void)ptr_pvParameters;

    do
    {
        rc = mac_rcv_readyGet();
    } while(0 != rc);
    ACE_pkt_netf = (NET_FILTER_T *) pvPortMalloc(sizeof(NET_FILTER_T));
    if (NULL != ACE_pkt_netf)
    {
        memset(ACE_pkt_netf->name, 0, MSG_MAX_NAME_LEN);
        sprintf((char *)ACE_pkt_netf->name, "%s", (char *)ACE_PKT_QUEUE_NAME);
        ACE_pkt_netf->protocol = PROTO_ACE;
        ACE_pkt_netf->state = NET_FILTER_REGISTER;
        /* Create ACE packet queue */
        ACE_pkt_reg_handle = xQueueCreate(128, sizeof(void *));
        if (NULL == ACE_pkt_reg_handle)
        {
            ACE_DBG_PRINT("Error: ACE_pkt_reg_handle == NULL\n");
            vPortFree(ACE_pkt_netf);
            return;
        }
        ACE_pkt_netf->handle = ACE_pkt_reg_handle;
        QueueHandle_t regHandle = NULL;
        int res = ethernetif_regHandleFind(&regHandle);
        rc = 0;
        if ((0 != res) || (NULL == regHandle))
        {
            rc = -1;
        }
        else
        {
            if (pdPASS != xQueueSend(regHandle, &ACE_pkt_netf, (100 / portTICK_RATE_MS)))
            {
                rc = -1;
            }
        }
        if (0 != rc)
        {
            ACE_DBG_PRINT("Error: NET_FILTER_REGISTER failed\n");
            vPortFree(ACE_pkt_netf);
            return;
        }
    }
    else
    {
        return;
    }

    rc = ACE_E_OK;
    if (NULL == g_ace_queue_set)
    {
        rc = ACE_E_ENTRY_NOT_FOUND;
    }
    else
    {
        if (pdPASS != xQueueAddToSet(ACE_pkt_reg_handle, g_ace_queue_set))
        {
            rc = ACE_E_OTHERS;
        }
    }
    if (ACE_E_OK != rc)
    {
        ACE_DBG_PRINT("Error: xQueueAddToSet failed %d\n", rc);
        ACE_free_resource();
        return;
    }
    ACE_DBG_PRINT("Start ACE\n");

    while (1)
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        if (NULL == g_ace_queue_set)
        {
            rc = ACE_E_ENTRY_NOT_FOUND;
            xActivatedMember = NULL;
        }
        else
        {
            xActivatedMember = xQueueSelectFromSet(g_ace_queue_set, (ACE_TASK_DELAY * 10 / portTICK_RATE_MS));
        }
        if (xActivatedMember == ACE_pkt_reg_handle)
        {
            if (pdPASS == xQueueReceive(ACE_pkt_reg_handle, &ptr_pbuf, (ACE_TASK_DELAY / portTICK_RATE_MS)))
            {
                ACE_rx(ptr_pbuf);
                /* ACE check trap */
                xLastWakeTime = xTaskGetTickCount();
                ACE_DBG_PRINT("trap: acl=%X, flag=%X, cmd=%X\n", (int)g_trap_mode_acl, (int)g_trap_mode_flag, (int)g_trap_mode_cmd);
                ACE_DBG_PRINT("tick=%u\n", (unsigned int)xLastWakeTime);
            }
        }
        /* ACE check trap */
        xLastWakeTime = xTaskGetTickCount();
        // if (xLastWakeTime > 2000 && xLastWakeTime < 2100)
        // {
        //     ACE_DBG_PRINT("trap: acl=%X, flag=%X, cmd=%X\n", g_trap_mode_acl, g_trap_mode_flag, g_trap_mode_cmd);
        //     ACE_DBG_PRINT("tick: %u\n", (unsigned int)xLastWakeTime);
        // }
        if (g_trap_mode_flag == 0xACE && g_trap_mode_acl > 0)
        {
            if (g_trap_mode_cmd > 0)
            {
                if (xLastWakeTime > 1500)
                {
                    ACE_DBG_PRINT("trap cmd timeout to boot\n");
                    char input[16] = {0};
                    char output[128] = {0};
                    memcpy(input, "boot-rtos", strlen("boot-rtos"));
                    cmd_process(input, output, sizeof(output));
                }
            }
            else
            {
                if (xLastWakeTime > 200)
                {
                    ACE_DBG_PRINT("trap acl timeout to boot\n");
                    char input[16] = {0};
                    char output[128] = {0};
                    memcpy(input, "boot-rtos", strlen("boot-rtos"));
                    cmd_process(input, output, sizeof(output));
                }
            }
        }
    }
}

#if defined(AIR_8851_SUPPORT)
/* FUNCTION NAME: ACE_acl_8851
 * PURPOSE:
 *      Set ACL for ACE
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
static void
ACE_acl_8851(void)
{
    uint8_t *sector_addr0 = (uint8_t *)(ACE_FLAGS_ADDRESS_ACE_ENABLE | HIGH_BIT_UNC);
    if ((*sector_addr0) != ACE_FLAGS_ACE_ENABLED)
    {
        ACE_DBG_PRINT("[%s] ACE disabled due to ACE ACL enable flag = %x\n", __FUNCTION__, (*sector_addr0));
        return;
    }

    /* acl add rule entry-id=x state=enable portlist=0-27 rule-end etype=0x88B5 */
    /* bank1 */
    io_write32(0x10200510, 0);
    io_write32(0x10200514, 0);
    io_write32(0x10200518, 0);
    io_write32(0x1020051c, ETHTYPE_ACE);
    io_write32(0x1020050c, (0x0003 | (ACL_ACE_START_IDX << 16)));
    /* bank2 */
    io_write32(0x10200510, 0);
    io_write32(0x10200514, 0);
    io_write32(0x10200518, 0);
    io_write32(0x1020051c, 0);
    io_write32(0x1020050c, (0x0103 | (ACL_ACE_START_IDX << 16)));

    /* bank3 */
    io_write32(0x10200510, 0x0000000);
    io_write32(0x10200514, 0x00030008); /* ethtype mask , UDF mask b15 */
    io_write32(0x10200518, 0x00100000); /* udf mask b15 */
    io_write32(0x1020051c, 0xFFFB4010); /* pbmap: 0 ~ 12, rule-enable, rule-end, field-enable: ethtype, UDF */
    io_write32(0x1020050c, (0x0203 | (ACL_ACE_START_IDX << 16)));

    /* bank4 */
    io_write32(0x10200510, 0xFFFF); /* pbmap: 13 ~ 28 */
    io_write32(0x10200514, 0);
    io_write32(0x10200518, 0);
    io_write32(0x1020051c, 0);
    io_write32(0x1020050c, (0x0303 | (ACL_ACE_START_IDX << 16)));

    /* acl add action entry-id=x mib-id=0 port-fw=cpu-port-included */
    io_write32(0x10200510, 0);
    io_write32(0x10200514, 0);
    io_write32(0x10200518, 0x0100000D); /* mib-enable, mib-id, rate-en, rate-idx, port-fw-en:enabled(1), fw-port:cpu-port-included(5) */
    io_write32(0x1020051c, 0);
    io_write32(0x1020050c, (0x0013 | (ACL_ACE_START_IDX << 16)));

    /* udf rule ACL_UDF_ACE_IDX */
    io_write32(0x10200208, 0x00000003); /* offset 0, l2 payload */
    io_write32(0x1020020C, 0xFFFF055A); /* CMP mask 0xFFFF, CMP pattern 0x055A */
    io_write32(0x10200210, 0x0FFFFFFF); /* pbmp */
    io_write32(0x10200200, 0x90000000 | ACL_UDF_ACE_IDX ); /* write, rule ACL_UDF_ACE_IDX */
}
#elif defined(AIR_8855_SUPPORT)
/* FUNCTION NAME: ACE_acl_8855
 * PURPOSE:
 *      Set ACL for ACE
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
static void
ACE_acl_8855(void)
{
    printf("ACE_acl_8855()---udf rule 15  \n");
    /* set acl rule--ethertype 0x88b5, udf rule 15 */
    io_write32(0x10200510, 0xfffffffe);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x3 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xffff116b);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x103 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xffffffff);
    io_write32(0x10200514, 0xffffffdf);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x203 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xffffffff);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1003 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xfffeee95);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1103 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xefdeffff);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1203 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, (0x1 << ACL_ACE_START_IDX));
    io_write32(0x10200514, 0x0);
    io_write32(0x10200518, 0x0);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, 0xb);

    io_write32(0x10200510, (0x1 << ACL_ACE_START_IDX));
    io_write32(0x10200514, 0x0);
    io_write32(0x10200518, 0x0);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, 0x1000b);

    /* set udf rule--l2 payload, 0x55a */
    io_write32(0x10200208, 0x00000003);
    io_write32(0x1020020c, 0xffff055a);
    io_write32(0x10200210, 0x0000003f);
    io_write32(0x10200200, (0x90000000 | ACL_UDF_ACE_IDX));

    /* set acl action--mib id 0, cpu port included */
    io_write32(0x10200510, 0x0);
    io_write32(0x10200514, 0x001a0000);
    io_write32(0x10200518, 0x00000200);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, (0x13 | (ACL_ACE_START_IDX << 16)));

    //io_write32(0x10200500, 0x1f1);
    //io_write32(0x10200504, 0x3f);
}
#elif defined(AIR_8858_SUPPORT)
/* FUNCTION NAME: ACE_acl_8858
 * PURPOSE:
 *      Set ACL for ACE
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
static void
ACE_acl_8858(void)
{
    printf("ACE_acl_8858()---udf rule %d  \n", ACL_UDF_ACE_IDX);

    io_write32(0x10200500, 0x1f1);
    io_write32(0x10200504, 0x3ff);

    /* set acl rule--ethertype 0x88b5, udf rule 15 */
    io_write32(0x10200510, 0xfffffffc);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x3 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xfffe22d7); //etype 0x88b5
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x103 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0x7fffffff); // field enable: etype
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x203 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xffffffff);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1003 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xfffddd2b);
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1103 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, 0xdfbdffff); // udf index 0xF
    io_write32(0x10200514, 0xffffffff);
    io_write32(0x10200518, 0xffffffff);
    io_write32(0x1020051c, 0xffffffff);
    io_write32(0x1020050c, (0x1203 | (ACL_ACE_START_IDX << 16)));

    io_write32(0x10200510, (0x1 << ACL_ACE_START_IDX));
    io_write32(0x10200514, 0x0);
    io_write32(0x10200518, 0x0);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, 0xb);

    io_write32(0x10200510, (0x1 << ACL_ACE_START_IDX));
    io_write32(0x10200514, 0x0);
    io_write32(0x10200518, 0x0);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, 0x1000b);

    /* set udf rule--l2 payload, 0x55a */
    io_write32(0x10200208, 0x00000003);
    io_write32(0x1020020c, 0xffff055a);
    io_write32(0x10200210, 0x000003ff); /* ACL UDF port bit map */
    io_write32(0x10200200, (0x90000000 | ACL_UDF_ACE_IDX));

    /* set acl action--mib id 0, cpu port included */
    io_write32(0x10200510, 0x0);
    io_write32(0x10200514, 0x00380000);
    io_write32(0x10200518, 0x00000400);
    io_write32(0x1020051c, 0x0);
    io_write32(0x1020050c, (0x13 | (ACL_ACE_START_IDX << 16)));
}
#else
    #error "Not implemented"
#endif

/* FUNCTION NAME: ACE_init
 * PURPOSE:
 *      ACE initialization function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ACE_E_OK
 *      ACE_E_NO_MEMORY
 *      ACE_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
ACE_ERR_T
ACE_init(void)
{
    unsigned int ace_cnt;
    int i;

    printf("ACE_init() .....\n");
    if (NULL != g_ace_task)
    {
        return ACE_E_ALREADY_INITED;
    }
    g_ace_queue_set = xQueueCreateSet(ACE_QUEUE_SIZE);
    if (NULL == g_ace_queue_set)
    {
        ACE_DBG_PRINT("Error: create ACE pool failed\n");
        return ACE_E_NO_MEMORY;
    }
    /* Create task for ACE */
    if (pdPASS != xTaskCreate(ACE_Process, ACE_TASK_NAME, ACE_STACK_SIZE, NULL, ACE_TASK_PRI, &g_ace_task))
    {
        ACE_DBG_PRINT("Error: create proc failed\n");
        return ACE_E_NO_MEMORY;
    }
    /* Create ACL for ACE */
#if defined(AIR_8851_SUPPORT)
    ACE_acl_8851();
#elif defined(AIR_8855_SUPPORT)
    ACE_acl_8855();
#elif defined(AIR_8858_SUPPORT)
    ACE_acl_8858();
#else
    #error "Not implemented"
#endif
    /* delay for ACE Trap */
    for (i = 0; i < 20; i++)
    {
        delay1ms(100);
        ace_cnt = io_read32(0x10200538);
        if (ace_cnt > 0)
        {
            break;
        }
    }

    return ACE_E_OK;
}

void dumb_switch_init()
{
    uint32_t u32dat = 0, u32glo = 0, port = 0;

    /* Set unknown unicast forwarding control's port-map. */
    //  UNUF.csr_unu_ffp
    io_write32(UNUF, MAC_PORT_PBMP);

    /* Set unknown multicast forwarding control's port-map. */
    //  UNMF.csr_unm_ffp
    io_write32(UNMF, MAC_PORT_PBMP);

    /* Set broadcast forwarding control's port-map. */
    //  BCF.csr_bc_ffp
    io_write32(BCF, MAC_PORT_PBMP);

    /* Set unknown IP multicast forwarding control's port-map. */
    //  UNIPMF.csr_unipm_ffp
    io_write32(UNIPMF, MAC_PORT_PBMP);

    //CPU setting
    port = AIR_CPU_PORT;
    printf("AIR_CPU_PORT=%d\n", (int)port);

    /* Enable CPU port and set CPU port as 28. */
    //  MFC.csr_cpu_en: Enable(1'b1)
    //  MFC.csr_cpu_port: Port X

    u32dat = io_read32(MFC);
    u32dat |= ((1 << REG_CPU_EN_OFFT) | (port << REG_CPU_PORT_OFFT));
    io_write32(MFC, u32dat);

    /* Disable CPU port's SMAC Learning. */
    u32dat = io_read32(PSC(port));
    u32dat |= (1 << PSC_DIS_LRN_OFFSET);
    io_write32(PSC(port), u32dat);

    u32dat = io_read32(PVC(port));
    /* Set CPU port's stag mode as insert mode. */
    //  PVC.csr_stag_mode: insert mode(1'b0)
    u32dat &= ~BIT(PVC_SPTAG_MODE_OFFT);
    u32dat |= (0x1 << PVC_SPTAG_EN_OFFT);
    /* Set CPU port's vlan_port_attribute as user port. */
    //  PVC.csr_port_type: user port(2'b00)
    u32dat &= ~PVC_VLAN_ATTR_MASK;
    u32dat |= (AIR_VLAN_PORT_ATTR_USER_PORT & PVC_VLAN_ATTR_RELMASK) << PVC_VLAN_ATTR_OFFT;
    io_write32(PVC(port), u32dat);

    /* For Egress rate setting */
    /* Set egress rate CIR */
    u32dat = io_read32(ERLCR(port));
    u32dat &= ~BITS_RANGE(REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    u32dat |= 16;
    /* Set egress rate CBS */
    u32dat &= ~BITS_RANGE(REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    u32dat |= 0;
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    /* Set token period to 4ms */
    u32dat &= ~BITS_RANGE(REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_4MS, REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    io_write32(ERLCR(port), u32dat);

    /* Get egress register value */
    u32dat = io_read32(ERLCR(port));
    u32dat |= BIT(REG_RATE_EN_OFFT);
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    io_write32(ERLCR(port), u32dat);

    /* Rate include preamble/IPG/CRC */
    u32glo = io_read32(GERLCR);
    u32glo &= ~BITS_RANGE(REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
    u32glo |= HAL_SCO_QOS_L1_RATE_LIMIT;
    io_write32(GERLCR, u32glo);

    /* Disable IGMP report/query on IMC. */
    io_write32(0x1020001c, 0x0);
    /* Disable MLD report/query on MMC. */
    io_write32(0x102000d4, 0x0);

    /* clear acl */
    io_write32(0x1020050c, 5);
    /* set acl global state */
    io_write32(0x10200500, 0x1f1);
    /* set acl port state */
    io_write32(0x10200504, 0xffffffff);
    /* set last-line */
    io_write32(0x10200508, 0x0020809f);
}


