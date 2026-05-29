/*******************************************************************************
*  The software may not be copied and the information
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
/* FILE NAME:  tftp_client.c
 * PURPOSE:
 *    tftp_client to get file by lwip and write to flash.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "spinorwrite.h"
#include "semphr.h"
#include "lwip/apps/tftp_client.h"

#include <string.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
extern void air_wdog_kick(void);

/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if LWIP_UDP
/* STATIC VARIABLE DECLARATIONS
 */

#define TFTP_GET_FILE (1)
#define TFTP_PUT_FILE (2)

struct tftp_client_state{
    unsigned int type;
    unsigned int total_recv_size;
    unsigned char *src_addr;
    unsigned int src_data_size;
    unsigned int progress;
    TaskHandle_t xHandle;
    unsigned short tftp_error_code;
};

static struct tftp_client_state tftp_client_state;
SemaphoreHandle_t tftp_mutex;

static void print_tftp_progress(struct tftp_client_state *state)
{
    state->progress++;
    if(state->progress % 2 == 0)
        outbyte('#');
    if(state->progress % 120 == 0)
        outbyte('\n');
}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   tftp_open
 * PURPOSE:
 *      No used.
 * INPUT:
 *      fname               -- Filename.
 *      mode                -- Mode string from TFTP RFC 1350.
 *      is_write            -- Flag indicating read (0) or write (!= 0) access.
 * OUTPUT:
 *      None
 * RETURN:
 *      handle              -- File handle supplied to other functions.
 * NOTES:
 *
 */
static void* tftp_open(const char* fname, const char* mode, u8_t is_write)
{
    return NULL;
}

/* FUNCTION NAME:   tftp_close
 * PURPOSE:
 *      Write last data to flash and chechk image CRC and prepare fw upgrade.
 * INPUT:
 *      handle              -- File handle supplied to other functions.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *
 */
static void tftp_close(void* handle)
{
    struct tftp_client_state *state = (struct tftp_client_state *)handle;
    if(state->type == TFTP_GET_FILE)
    {
        if(state->tftp_error_code != 0xffff)
        {
            LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("%s : state->tftp_error_code = %d\n", __func__, state->tftp_error_code));
            /*notify 0 to current task if occur any TFTP transfer error*/
            xTaskNotify((TaskHandle_t) state->xHandle, 0, eSetValueWithOverwrite);
        }
        else
        {
            WriteLastBuffer();
            LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("%s : state->total_recv_size = %d\n", __func__, state->total_recv_size));
            xTaskNotify((TaskHandle_t) state->xHandle, state->total_recv_size, eSetValueWithOverwrite);
        }
    }
    else if(state->type == TFTP_PUT_FILE)
    {
        LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("%s : state->tftp_error = %d\n", __func__, state->tftp_error_code));
        xTaskNotify(((struct tftp_client_state *)handle)->xHandle, state->tftp_error_code, eSetValueWithOverwrite);
    }
}

/* FUNCTION NAME:   tftp_read
 * PURPOSE:
 *      No used.
 * INPUT:
 *      handle              -- File handle returned by open()/tftp_put()/tftp_get().
 *      buf                 -- Target buffer to copy read data to.
 *      bytes               -- Number of bytes to copy to buf.
 * OUTPUT:
 *      None
 * RETURN:
 *      return              -- &gt;= 0: Success; &lt; 0: Error.
 * NOTES:
 *
 */
static int tftp_read(void* handle, void* buf, int bytes)
{
    unsigned int copy_size;
    struct tftp_client_state *state =  (struct tftp_client_state*)handle;
    if(state->src_data_size >= TFTP_CLIENT_MAX_PAYLOAD_SIZE)
    {
        copy_size = TFTP_CLIENT_MAX_PAYLOAD_SIZE;
        memcpy(buf, (void *)(state->src_addr), TFTP_CLIENT_MAX_PAYLOAD_SIZE);
        state->src_addr += TFTP_CLIENT_MAX_PAYLOAD_SIZE;
        state->src_data_size -= TFTP_CLIENT_MAX_PAYLOAD_SIZE;
    }
    else
    {
        copy_size = state->src_data_size;
        memcpy(buf, (void *)(state->src_addr), state->src_data_size);
        state->src_addr += state->src_data_size;
        state->src_data_size -= state->src_data_size;
    }
    print_tftp_progress(state);
    return copy_size;
}

/* FUNCTION NAME:   tftp_write
 * PURPOSE:
 *      Write data to flash.
 * INPUT:
 *      handle              -- File handle returned by open()/tftp_put()/tftp_get().
 *      pbuf                -- PBUF adjusted such that payload pointer points
 *                             to the beginning of write data. In other words,
 *                             TFTP headers are stripped off.
 * OUTPUT:
 *      None
 * RETURN:
 *      return              -- &gt;= 0: Success; &lt; 0: Error.
 * NOTES:
 */
static int tftp_write(void* handle, struct pbuf* p)
{
    struct tftp_client_state *state = (struct tftp_client_state *)handle;
    int result = 0;
    while ((p != NULL) && (result == 0))
    {
        air_wdog_kick();
        result = WriteBuffer(p->payload, p->len);
        state->total_recv_size += p->len;
        p = p->next;
    }
    print_tftp_progress(state);
    return result;
}

/* FUNCTION NAME:   tftp_error
 * PURPOSE:
 *      Error indication from client or response from server.
 * INPUT:
 *      handle              -- File handle returned by open()/tftp_put()/tftp_get().
 *      err                 -- error code from client or server.
 *      msg                 -- error message from client or server.
 *      size                -- size size of msg.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 */
/* For TFTP client only */
static void tftp_error(void* handle, u16_t err, const char* msg, int size)
{
    char message[100];
    /*
     * pkt format : |opcode = 5|[Erroe code = err(0~7)|Error message = msg|
     * (not defined = 0)
     * TFTP_ERROR_FILE_NOT_FOUND    = 1,
     * TFTP_ERROR_ACCESS_VIOLATION  = 2,
     * TFTP_ERROR_DISK_FULL         = 3,
     * TFTP_ERROR_ILLEGAL_OPERATION = 4,
     * TFTP_ERROR_UNKNOWN_TRFR_ID   = 5,
     * TFTP_ERROR_FILE_EXISTS       = 6,
     * TFTP_ERROR_NO_SUCH_USER      = 7,
     * TFTP_ERROR_TIMEOUT           = 8 (only TFTP_ERROR_TIMEOUT is not from packet)
     */
    ((struct tftp_client_state *)handle)->tftp_error_code = err;

    memset(message, 0, sizeof(message));
    MEMCPY(message, msg, LWIP_MIN(sizeof(message)-1, (size_t)size));
    LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("\n%s : %d (%s)", __func__, ((struct tftp_client_state *)handle)->tftp_error_code, message));
}

static const struct tftp_context tftp = {
    tftp_open,
    tftp_close,
    tftp_read,
    tftp_write,
    tftp_error
};

/* EXPORTED SUBPROGRAM BODIES
 */

/* Initial to tftp client mode */
void tftp_init(void)
{
    err_t err;
    tftp_mutex = xSemaphoreCreateMutex("tftp");
    LWIP_ASSERT("tftp create mutex failed", tftp_mutex != NULL);
    err = tftp_init_client(&tftp);
    LWIP_ASSERT("tftp_init_client failed", err == ERR_OK);
}

/* Create tftp get task */
int tftp_get_file(const char *host_ip, char *fname, unsigned int flash_addr, unsigned int *total_rcv_size)
{
    err_t err = ERR_OK;
    ip_addr_t ip_addr;
    uint32_t ret;
    unsigned int i = 0;
    unsigned int temp_src = 0;
    unsigned int temp_dst =0;
    unsigned int temp_size = 0;

    if((!ipaddr_aton(host_ip, &ip_addr)) || (fname == NULL))
    {
        return -TFTP_CLIENT_ERR_VAL;
    }

    if(xSemaphoreTake(tftp_mutex, NULL) == pdTRUE)
    {
        WriteBufferInit((unsigned char *) TempSystemBase);
        memset(&tftp_client_state, 0, sizeof(tftp_client_state));
        tftp_client_state.tftp_error_code = 0xffff;
        tftp_client_state.type = TFTP_GET_FILE;
        tftp_client_state.xHandle = (void *)xTaskGetCurrentTaskHandle();
        tftp_client_state.total_recv_size = 0;
        if (ERR_OK != tftp_get(&tftp_client_state, &ip_addr, TFTP_PORT, fname, TFTP_MODE_OCTET))
        {
            xSemaphoreGive(tftp_mutex);
            return -TFTP_CLIENT_ERR_SEND_REQUEST;
        }

        /*current task is notified by tftp_close with the notification value tftp_error_code*/
        ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(ret)
        {
            /* Update raw image to special flash address */
            if (flash_addr != ~0UL)
            {
                temp_src = TempSystemBase;
                temp_dst = flash_addr;
                temp_size = ret;

                while (temp_size)
                {
                    if (temp_size > SPI_NOR_SECTOR_SIZE)
                        i = SPI_NOR_SECTOR_SIZE;
                    else
                        i = temp_size;

                    spinor_write(temp_src, temp_dst, i);
                    temp_src += i;
                    temp_dst += i;
                    temp_size -= i;
                }
            }
            *total_rcv_size = ret;
            xSemaphoreGive(tftp_mutex);
            return TFTP_CLIENT_SUCCESS;
        }
        else
        {
            *total_rcv_size = tftp_client_state.total_recv_size;
            xSemaphoreGive(tftp_mutex);
            return -TFTP_CLIENT_ERR_PROCESS;
        }
    }
    else
    {
        return -TFTP_CLIENT_ERR_ACQUIRE_SEM;
    }
}

int tftp_put_file(const char *host_ip, const char *fname, unsigned char *src_addr, unsigned int src_data_size)
{
    ip_addr_t ip_addr;
    uint32_t ret;

    if((!ipaddr_aton(host_ip, &ip_addr)) || (fname == NULL))
    {
        return -TFTP_CLIENT_ERR_VAL;
    }

    if(xSemaphoreTake(tftp_mutex, NULL) == pdTRUE)
    {
        memset(&tftp_client_state, 0, sizeof(tftp_client_state));
        /* After tftp transfer, check tftp_error_code is 0xffff or not.
        * If it is not 0xffff, it means error occurs.*/
        tftp_client_state.tftp_error_code = 0xffff;
        tftp_client_state.type = TFTP_PUT_FILE;
        tftp_client_state.src_addr = src_addr;
        tftp_client_state.src_data_size = src_data_size;
        tftp_client_state.xHandle = xTaskGetCurrentTaskHandle();

        LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("%s : task %s\n", __func__, pcTaskGetName(tftp_client_state.xHandle)));

        if (ERR_OK != tftp_put(&tftp_client_state, &ip_addr, TFTP_PORT, fname, TFTP_MODE_OCTET))
        {
            xSemaphoreGive(tftp_mutex);
            return -TFTP_CLIENT_ERR_SEND_REQUEST;
        }

        /*current task is notified by tftp_close with the notification value tftp_error_code*/
        ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        LWIP_DEBUGF(TFTP_DEBUG | LWIP_DBG_STATE, ("%s : ret = %d,  tftp_client_state.tftp_error_code = %d\n", __func__, ret, tftp_client_state.tftp_error_code));

        if(ret != 0xffff)
        {
            xSemaphoreGive(tftp_mutex);
            return -TFTP_CLIENT_ERR_PROCESS;
        }
        xSemaphoreGive(tftp_mutex);
        return TFTP_CLIENT_SUCCESS;
    }
    else
    {
        return -TFTP_CLIENT_ERR_ACQUIRE_SEM;
    }
}
#endif /* LWIP_UDP */