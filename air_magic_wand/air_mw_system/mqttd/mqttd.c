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

/* FILE NAME:  mqttd.c
 * PURPOSE:
 *  Implement MQTT client daemon for RTOS turnkey
 *
 * NOTES:
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mqttd.h"
#include "mqttd_queue.h"
#include "mw_error.h"
#include "lwip/opt.h"
#ifdef AIR_SUPPORT_HTTPD_MUTEX
#include "httpd_util.h"
#endif
#include "lwip/ip.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_opts.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "osapi.h"
#include "osapi_timer.h"
#include "osapi_thread.h"
#include "osapi_string.h"
#include "osapi_mutex.h"
#include "db_api.h"
#include "inet_utils.h"
#include "web.h"
#include "lwip/tcpip.h"
#include "lwip/api.h"
#include "lwip/priv/tcpip_priv.h"
#include "mw_log.h"
#include "mw_cmd_util.h"

#if LWIP_ALTCP && LWIP_ALTCP_TLS
#include "lwip/altcp_tls.h"
/* enable MQTT over TLS. */
#define MQTTD_SUPPORT_TLS    (1)
/* change Your CA here. */
#define CRLF "\r\n"
static unsigned char const CA[] =
"-----BEGIN CERTIFICATE-----"CRLF
"MIIDnzCCAocCFGkd1/mFlQh3AHgwdy7SI48OLEP+MA0GCSqGSIb3DQEBCwUAMIGN"CRLF
"MSEwHwYJKoZIhvcNAQkBFhJkYXZlLmhlQGFpcm9oYS5jb20xCzAJBgNVBAYTAlRX"CRLF
"MQ8wDQYDVQQIDAZUYWl3YW4xEDAOBgNVBAcMB0hzaW5jaHUxDzANBgNVBAoMBkFp"CRLF
"cm9oYTEOMAwGA1UECwwFRU5CQ0ExFzAVBgNVBAMMDjE3Mi4yNC4xNTcuMTA4MB4X"CRLF
"DTIzMDIxNzA4MTYwM1oXDTI0MDIxMjA4MTYwM1owgYkxIjAgBgkqhkiG9w0BCQEW"CRLF
"E3J5bm4ueWVoQGFpcm9oYS5jb20xCzAJBgNVBAYTAlRXMQ8wDQYDVQQIDAZUYWl3"CRLF
"YW4xDzANBgNVBAcMBlRhaXBlaTEPMA0GA1UECgwGQWlyb2hhMRIwEAYDVQQLDAlF"CRLF
"TkJDbGllbnQxDzANBgNVBAMMBkVOODg1MTCCASIwDQYJKoZIhvcNAQEBBQADggEP"CRLF
"ADCCAQoCggEBANlY/W6HifZtkpF4ZH5wvzHeNa5+GM1rE31WKUtOMU3le7kuNpU2"CRLF
"GBOAZqC+WHWeNus3FsGOPn9eIqBsWrP4/5wKWCFkzm6MV90H4obHNJrzymqbH/qI"CRLF
"/9JCSw3bN+BVC59yg27oV1QGyphjFujZD7G6CTMuNlwnIAkQwGoRRUV6Yxxwdni5"CRLF
"jSLK2/07+hdaDglrqcVUNvvZjn7bSz/UXO/YodZT5Jp4Bh1an85/28fv0TAFw+f9"CRLF
"7L6kAI96KjFaOT7LJGho6CXXv0ldMY61DTIGZGTboIrE6tlYlnXPBH1AnL2ZawjY"CRLF
"I2ZxWhGX6G4kJYdccXnEUUTmdEs78JzhUtkCAwEAATANBgkqhkiG9w0BAQsFAAOC"CRLF
"AQEAt6GgkcPGTnT76ARJUL9UXjL7KkwR0g24sjgJ4H3z2OPREE8llpVYpdL1d90x"CRLF
"RoPBuHPrQQPDE6wlajwe9iTmR0mN3ZM915PmKIo2uqkt7R0NKsvGEsKOabpqbfUH"CRLF
"eQplrjG9PWIy4rorK8X01LYJPiS2IelfDyqfqXmAKb9MID7gUQSgPm1gu8P7Fr5j"CRLF
"/8HE2X+SX8XnwqG4giWwlVh5mr2KWhkVOtS9+aC/UQpEnvcDkztXK0GB+pILbu78"CRLF
"1ip7Kq5g+wtaDde57a7HnEBGEdvnMCUUC7PUSsE/4FfIEFbYgC0BOkWEG2gBBHWs"CRLF
"I3DFmUFKq47Or9gWBlpzOA7qyg=="CRLF
"-----END CERTIFICATE-----"CRLF;
#else
#define MQTTD_SUPPORT_TLS    (0)
#endif

/* NAMING CONSTANT DECLARATIONS
*/

/* MQTTD Client Daemon task
*/
#define MQTTD_TASK_NAME             "mqttd"
#define MQTTD_TIMER_NAME            "mqttdTmr"
#define MQTTD_STACK_SIZE            AIR_MAX_MQTT_STACK_SIZE
#define MQTTD_THREAD_PRI            (4)
#define MQTTD_MAX_TOPIC_SIZE        (44)   // Topic MAX Length 42 + tail end
#define MQTTD_TIMER_PERIOD          (500)
#define MQTTD_MUX_LOCK_TIME         AIR_MAX_MQTT_LOCK_TIME
#define MQTTD_MAX_REMAIN_MSG        (64)
#define MQTTD_MAX_BUFFER_SIZE       (64)
#define MQTTD_RECONNECT_TIMER_NAME  "mqttdReTmr"
#define MQTTD_TIMER_RECONNECT_PERIOD (1000)

/* MQTTD Client ID
*/
#define MQTTD_MAX_CLIENT_ID_SIZE     (24)
#define MQTTD_CLIENT_ID_FMT          "AIR%s%02x%02x%02x%02x%02x%02x"
#define MQTTD_CLIENT_ID_PRODUCT_DFT  AIR_DEFAULT_SYSTEM_NAME
#define MQTTD_CLIENT_CONNECT_MSG_FMT "{\"port_num\":%2d, \"msg_ver\":\"%s\"}"
#define MQTTD_MSG_VER                "2.0"

/* MQTTD Server Login
*/
//const ip_addr_t mqttd_server_ip = IPADDR4_INIT_BYTES(122, 116, 233, 146);  // for OA domain
const ip_addr_t mqttd_server_ip = IPADDR4_INIT_BYTES(54, 87, 224, 92);  // for AWS domain
#if LWIP_DNS
const C8_T cloud_hostname[] = "cloudmw.airoha.com";
#endif

#define MQTTD_USERNAME              "mw-client"
#define MQTTD_PASSWD                "magicwand"
#define MQTTD_KEEP_ALIVE            AIR_MAX_MQTT_KEEP_ALIVE

/* MQTTD Topics
*/
#define MQTTD_TOPIC_PREFIX          "mw"
#define MQTTD_TOPIC_CLOUD_PREFIX    "cmw"
//Will message
#define MQTTD_WILL_TOPIC            MQTTD_TOPIC_PREFIX "/will"
#define MQTTD_WILL_QOS              (2)
#define MQTTD_WILL_RETAIN           (1)
//Normal message
#define MQTTD_TOPIC_NEW             MQTTD_TOPIC_PREFIX "/new"
#define MQTTD_TOPIC_DB              MQTTD_TOPIC_PREFIX "/db"
#define MQTTD_TOPIC_INIT            MQTTD_TOPIC_DB "/init"
#define MQTTD_TOPIC_DBRC            MQTTD_TOPIC_DB "/rc"
#define MQTTD_REQUEST_QOS           (0)
#define MQTTD_REQUEST_RETAIN        (0)
//Cloud message
#define MQTTD_CLOUD_TODB            "mwcloud/db"
#define MQTTD_SUB_CLOUD_FILTER      MQTTD_TOPIC_CLOUD_PREFIX "/#"
#define MQTTD_CLOUD_CONNECTED       MQTTD_TOPIC_CLOUD_PREFIX "/connected"
#define MQTTD_CLOUD_WILL            MQTTD_TOPIC_CLOUD_PREFIX "/will"
#define MQTTD_CLOUD_CGI             MQTTD_TOPIC_CLOUD_PREFIX "/cgi"
//Normal Publish Topic format
/* <msg topic>/<cldb_ID>/<table_name>/<field_name>/eidx */
#define MQTTD_TOPIC_FORMAT          "%s/%hu/%s/%s/%hu"

/* MQTTD Publish Application message
*/
#define MQTTD_MQX_OUTPUT_SIZE       MQTT_OUTPUT_RINGBUF_SIZE
#define MQTTD_MAX_PACKET_SIZE       (MQTTD_MQX_OUTPUT_SIZE - 2 - 7)   /* MAX size of topic + payload, exclude the fix header and length */
#define MQTTD_MSG_HEADER_SIZE       (sizeof(MQTTD_PUB_MSG_T) - DB_MSG_PTR_SIZE)     /* size of message header and length in PUBLISH payload */
#define MQTTD_MAX_SESSION_ID        (255)

/* Reconnect related */
#define INITIAL_RECONNECT_INTERVAL    10000
#define SECOND_RECONNECT_INTERVAL     30000
#define FINAL_RECONNECT_INTERVAL      60000
#define MAX_INITIAL_RETRIES           5
#define MAX_SECOND_RETRIES            5
#define MAX_FINAL_RETRIES             5

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/
/* The MQTTD state machine */
typedef enum {
    MQTTD_STATE_CONNECTING = 0,
    MQTTD_STATE_CONNECTED = 1,
    MQTTD_STATE_SUBSCRIBE = 2,
    MQTTD_STATE_SUBACK = 3,
    MQTTD_STATE_INITING = 4,
    MQTTD_STATE_RUN = 5,
    MQTTD_STATE_DISCONNECTED = 6,
    MQTTD_STATE_SHUTDOWN = 7
}MQTTD_STATE_T;

typedef MW_ERROR_NO_T (*queue_recv_t)(void **ptr_buf);
typedef MW_ERROR_NO_T (*queue_send_t)(const UI8_T method, const UI8_T t_idx, const UI8_T f_idx, const UI16_T e_idx, const void *ptr_data, const UI16_T size, DB_MSG_T **pptr_out_msg);
typedef MW_ERROR_NO_T (*queue_setData_t)(const UI8_T method, const UI8_T t_idx, const UI8_T f_idx, const UI16_T e_idx, const void *ptr_data, const UI16_T size);
typedef MW_ERROR_NO_T (*queue_getData_t)(const UI8_T in_t_idx, const UI8_T in_f_idx, const UI16_T in_e_idx, DB_MSG_T **pptr_out_msg, UI16_T *ptr_out_size, void **pptr_out_data);

/* The publish application message format */
// Obsolete
typedef struct MQTTD_PUB_MSG_OLD_S
{
    UI16_T          cldb_id;       /* The switch identifier in cloud DB */
    UI16_T          session_id;    /* The cloud request session identifier */
    UI8_T           method;        /* The method bitmap */
    union {
        UI8_T       count;         /* The data payload count in request or notification */
        UI8_T       result;        /* The response result with type MW_ERROR_NO_T */
    } type;
    DB_REQUEST_TYPE_T  request;    /* The type of the message */
    UI16_T          data_size;     /* The incoming data size */
    void*           ptr_data;
} ATTRIBUTE_PACK MQTTD_PUB_MSG_OLD_T;

typedef struct MQTTD_PUB_MSG_S
{
    UI8_T           pid;           /* The packet index of the whole message */
    UI8_T           count;         /* The data payload count in request or notification */
    UI8_T           method;        /* The method bitmap */
    UI16_T          data_size;     /* The data size */
    void*           ptr_data;
} ATTRIBUTE_PACK MQTTD_PUB_MSG_T;

typedef struct MQTTD_PUB_LIST_S
{
    C8_T            topic[MQTTD_MAX_TOPIC_SIZE];
    UI16_T          msg_size;
    void*           msg;
    struct MQTTD_PUB_LIST_S *next;
} ATTRIBUTE_PACK MQTTD_PUB_LIST_T;

/* The MQTTD ctrl structure */
typedef struct MQTTD_CTRL_S
{
    MQTTD_STATE_T   state;
    ip_addr_t       server_ip;
    UI16_T          cldb_id;        /* The switch identifier in cloud DB */
    C8_T            client_id[MQTTD_MAX_CLIENT_ID_SIZE];
    mqtt_client_t*  ptr_client;
    UI8_T           db_subscribed;
    UI8_T           reconnect;
    C8_T            pub_in_topic[MQTTD_MAX_TOPIC_SIZE];
    UI8_T           remain_msgs;    /* Not yet sent message count */
    MQTTD_PUB_LIST_T *msg_head;
} ATTRIBUTE_PACK MQTTD_CTRL_T;

typedef struct MQTTD_PARAM_S
{
    const char*                 topic;
    const void*                 payload;
    u16_t                       len;
    u8_t                        qos;
    u8_t                        retain;
    u8_t                        sub;
} ATTRIBUTE_PACK MQTTD_PARAM_T;

typedef struct MQTTD_TCPIP_CALL_MSG_S
{
    struct tcpip_api_call_data  call;
    MQTTD_CTRL_T*               ptr_mqttd_ctrl;
    MQTTD_PARAM_T               mqtt_param;
} ATTRIBUTE_PACK MQTTD_TCPIP_CALL_MSG_T;

/* GLOBAL VARIABLE DECLARATIONS
*/
UI8_T mqttd_enable;
MQTTD_CTRL_T mqttd;
#if MQTTD_SUPPORT_TLS
struct altcp_tls_config *tls_config = NULL;
#endif
/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static void _mqttd_ctrl_init(MQTTD_CTRL_T *ptr_mqttd, ip_addr_t *server_ip);
static err_t _mqttd_ctrl_free_callback(struct tcpip_api_call_data *ptr_arg);
static MW_ERROR_NO_T _mqttd_append_remain_msg(MQTTD_CTRL_T *ptr_mqttd, C8_T *topic, UI16_T msg_size, void *ptr_msg);
static void _mqttd_send_remain_msg(MQTTD_CTRL_T *ptr_mqttd);
static void _mqttd_tmr(timehandle_t ptr_xTimer);
/*=== DB related local functions ===*/
static UI8_T _mqttd_gen_client_id(MQTTD_CTRL_T *ptr_mqttd);
/* for example */
#if 0
static MW_ERROR_NO_T _mqttd_subscribe_db(MQTTD_CTRL_T *ptr_mqttd);
static MW_ERROR_NO_T _mqttd_unsubscribe_db(MQTTD_CTRL_T *ptr_mqttd);
#endif
static void _mqttd_listen_db(MQTTD_CTRL_T *ptr_mqttd);
/*=== MQTT related local functions ===*/
static void _mqttd_cgi_proxy(MQTTD_CTRL_T *ptr_mqttd, const u8_t *data, u16_t len);
static void _mqttd_dataDump(const void *data, UI16_T data_size);
static UI16_T _mqttd_db_topic_set(MQTTD_CTRL_T *ptr_mqttd, const UI8_T method, const UI8_T t_idx, const UI8_T f_idx, const UI16_T e_idx, C8_T *topic, UI16_T buf_size);
static void _mqttd_publish_cb(void *arg, err_t err);
static MW_ERROR_NO_T _mqttd_publish_data(MQTTD_CTRL_T *ptr_mqttd, const UI8_T method, C8_T *topic, const UI16_T data_size, const void *ptr_data);
static void _mqttd_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void _mqttd_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void _mqttd_subscribe_cb(void *arg, err_t err);
static void _mqttd_send_subscribe(mqtt_client_t *client, void *arg);
static void _mqttd_send_clientId(mqtt_client_t *client, void *arg);
static void _mqttd_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static err_t _mqttd_lookup_server(MQTTD_CTRL_T *ptr_mqttd);
static err_t _mqttd_client_connect_callback(struct tcpip_api_call_data *ptr_arg);
static err_t _mqttd_client_disconnect_callback(struct tcpip_api_call_data *ptr_arg);
static err_t mqttdapi_tcpip_call(tcpip_api_call_fn call_func);
static void _mqttd_main(void *arg);
MW_ERROR_NO_T _mqttd_deinit(void);
/*=== MQTT reconnect functions ===*/
static void _mqttd_reconnect(void *ptr_arg);
static void _mqttd_connect_init(void);
static err_t _mqttd_tcpip_publish_cb(struct tcpip_api_call_data *ptr_arg);
static err_t _mqttd_tcpip_subscribe_cb(struct tcpip_api_call_data *ptr_arg);

/* STATIC VARIABLE DECLARATIONS
 */
static threadhandle_t ptr_mqttdmain = NULL;
static timehandle_t ptr_mqttd_time = NULL;
static semaphorehandle_t ptr_mqttmutex = NULL;

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  _mqttd_ctrl_init
 * PURPOSE:
 *      Initialize the control structure
 *
 * INPUT:
 *      ptr_mqttd  -- The control structure
 *      server_ip  -- The remote mqtt server ip
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
static void _mqttd_ctrl_init(MQTTD_CTRL_T *ptr_mqttd, ip_addr_t *server_ip)
{
    MW_LOG_DEBUG(MQTTD, "Initialize the MQTTD control structure.");

    ptr_mqttd->state = MQTTD_STATE_CONNECTING;
    ptr_mqttd->db_subscribed = FALSE;
    if ((server_ip != NULL) && (server_ip != &(ptr_mqttd->server_ip)))
    {
        osapi_memcpy((void *)&(ptr_mqttd->server_ip), (const void *)server_ip, sizeof(ip_addr_t));
    }
    ptr_mqttd->cldb_id = 0;
    ptr_mqttd->ptr_client = NULL;
    osapi_memset(ptr_mqttd->pub_in_topic, 0, MQTTD_MAX_TOPIC_SIZE);
    ptr_mqttd->remain_msgs = 0;
    ptr_mqttd->msg_head = NULL;
    ptr_mqttd->reconnect = FALSE;
}
static err_t mqttdapi_tcpip_call(tcpip_api_call_fn call_func)
{
  err_t err;
  MQTTD_TCPIP_CALL_MSG_T msg;

  msg.ptr_mqttd_ctrl = &mqttd;
  err = tcpip_api_call(call_func, &msg.call);
  return err;
}

/* FUNCTION NAME:  _mqttd_ctrl_free
 * PURPOSE:
 *      Free the control structure
 *
 * INPUT:
 *      ptr_mqttd  -- The control structure
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
static err_t _mqttd_ctrl_free_callback(struct tcpip_api_call_data *ptr_arg)
{
    MQTTD_TCPIP_CALL_MSG_T *ptr_callback_msg = (MQTTD_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    MQTTD_CTRL_T *ptr_mqttd = ptr_callback_msg->ptr_mqttd_ctrl;
    MQTTD_PUB_LIST_T *ptr_msg = NULL;
    MQTTD_PUB_LIST_T *ptr_head = ptr_mqttd->msg_head;

    MW_LOG_DEBUG(MQTTD, "Free the MQTTD control structure.");

#if MQTTD_SUPPORT_TLS
    if(NULL != tls_config)
    {
        altcp_tls_free_config(tls_config);
        tls_config = NULL;
    }
#endif

    while (ptr_mqttd->msg_head != NULL)
    {
        ptr_msg = ptr_mqttd->msg_head;
        osapi_free(ptr_msg->msg);
        osapi_free(ptr_mqttd->msg_head);
        ptr_mqttd->msg_head = ptr_msg->next;
        if (ptr_mqttd->msg_head == ptr_head)
        {
            /* The last message pointer*/
            ptr_mqttd->msg_head = NULL;
        }
    }
    ptr_mqttd->remain_msgs = 0;
    if (NULL != ptr_mqttd->ptr_client)
    {
        mqtt_client_free(ptr_mqttd->ptr_client);
    }
    ptr_mqttd->ptr_client = NULL;
    ptr_mqttd->cldb_id = 0;
    osapi_memset(ptr_mqttd->pub_in_topic, 0, MQTTD_MAX_TOPIC_SIZE);
    return ERR_OK;
}

/* FUNCTION NAME:  _mqttd_append_remain_msg
 * PURPOSE:
 *      If the MQTT request list is full, try to keep the msg in MQTTD
 *
 * INPUT:
 *      ptr_mqttd  -- The control structure
 *      topic      -- The publish topic
 *      msg_size   -- The publish message size
 *      ptr_msg    -- The publish message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T _mqttd_append_remain_msg(MQTTD_CTRL_T *ptr_mqttd, C8_T *topic, UI16_T msg_size, void *ptr_msg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    MQTTD_PUB_LIST_T *ptr_temp = NULL;
    MQTTD_PUB_LIST_T *ptr_new = NULL;

    MW_CHECK_PTR(ptr_mqttd);
    MW_CHECK_PTR(topic);

    MW_LOG_DEBUG(MQTTD, "MQTTD append the remain message in list.");
    if (MQTTD_MAX_REMAIN_MSG <= ptr_mqttd->remain_msgs)
    {
        MW_LOG_ERROR(MQTTD, "MQTTD remain msg list is full.");
        return MW_E_NO_MEMORY;
    }

    if (MW_E_OK == osapi_mutexTake(ptr_mqttmutex, MQTTD_MUX_LOCK_TIME))
    {
        rc = osapi_calloc(sizeof(MQTTD_PUB_LIST_T), MQTTD_TASK_NAME, (void **)&(ptr_new));
        if (MW_E_OK != rc)
        {
            MW_LOG_ERROR(MQTTD, "%s: allocate memory failed(%d)", __func__, rc);
            osapi_mutexGive(ptr_mqttmutex);
            return MW_E_NO_MEMORY;
        }

        /* Assign data */
        osapi_strncpy(ptr_new->topic, topic, sizeof(ptr_new->topic));
        ptr_new->msg_size = msg_size;
        ptr_new->msg = ptr_msg;
        ptr_new->next = ptr_mqttd->msg_head;

        if (ptr_mqttd->msg_head == NULL)
        {
            ptr_mqttd->msg_head = ptr_new;
            ptr_mqttd->msg_head->next = ptr_mqttd->msg_head;
            MW_LOG_DEBUG(MQTTD, "Head not exist, create the MQTTD msg_head(%p)->next(%p).", ptr_mqttd->msg_head, ptr_mqttd->msg_head->next);
        }
        else
        {
            /* Find the tail */
            ptr_temp = ptr_mqttd->msg_head;
            while (ptr_mqttd->msg_head != ptr_temp->next)
            {
                MW_LOG_DEBUG(MQTTD, "Search the tail: temp(%p)->next(%p).", ptr_temp, ptr_temp->next);
                ptr_temp = ptr_temp->next;
            }
            ptr_temp->next = ptr_new;
        }

        ptr_mqttd->remain_msgs++;
        osapi_mutexGive(ptr_mqttmutex);
    }
    MW_LOG_DEBUG(MQTTD, "MQTTD append the remain message done.");
    return rc;
}

/* FUNCTION NAME:  _mqttd_send_remain_msg
 * PURPOSE:
 *      Try to send the remained message in MQTTD
 *
 * INPUT:
 *      ptr_mqttd  -- The control structure
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
static void _mqttd_send_remain_msg(MQTTD_CTRL_T *ptr_mqttd)
{
    err_t err;
    MQTTD_PUB_LIST_T *ptr_msg = NULL;
    MQTTD_PUB_LIST_T *ptr_head = NULL;
    MQTTD_TCPIP_CALL_MSG_T msg;
    MW_LOG_DEBUG(MQTTD, "Send the remain message to cloud.");

    /* Try to PUBLISH data ASAP */

    if (0 == ptr_mqttd->remain_msgs)
    {
        MW_LOG_DEBUG(MQTTD, "No remain message to send.");
        return;
    }
    if (MW_E_OK == osapi_mutexTake(ptr_mqttmutex, MQTTD_MUX_LOCK_TIME))
    {
        ptr_head = ptr_mqttd->msg_head;
        while (ptr_mqttd->msg_head != NULL)
        {
            if (ptr_mqttd->state > MQTTD_STATE_RUN)
            {
                break;
            }
            ptr_msg = ptr_mqttd->msg_head;
            msg.ptr_mqttd_ctrl = ptr_mqttd;
            msg.mqtt_param.topic = ptr_msg->topic;
            msg.mqtt_param.payload = (const void *)ptr_msg->msg;
            msg.mqtt_param.len = ptr_msg->msg_size;
            msg.mqtt_param.qos = MQTTD_REQUEST_QOS;
            msg.mqtt_param.retain = MQTTD_REQUEST_RETAIN;
            err = tcpip_api_call(_mqttd_tcpip_publish_cb, &msg.call);

            if (ERR_OK == err)
            {
                MW_LOG_DEBUG(MQTTD, "Send the MQTTD list(%p)->msg(%p).", ptr_msg, ptr_msg->msg);
                /* Already in MQTT request list, so the allocate memory can be free */
                osapi_free(ptr_msg->msg);
                ptr_msg->msg = NULL;
                ptr_mqttd->msg_head = ptr_msg->next;
                if (ptr_msg->next == ptr_head)
                {
                    /* The last message sent */
                    ptr_mqttd->msg_head = NULL;
                }
                osapi_free(ptr_msg);
                ptr_msg = NULL;
                ptr_mqttd->remain_msgs--;
            }
            else if (ERR_CONN == err)
            {
                MW_LOG_ERROR(MQTTD, "Error (%d): The MQTT connection is broken", err);
                ptr_mqttd->state = MQTTD_STATE_DISCONNECTED;
                break;
            }
            else
            {
                MW_LOG_ERROR(MQTTD, "Error (%d): Send the MQTTD list(%p)->msg(%p).", err, ptr_msg, ptr_msg->msg);
                break;
            }
        }

        /* Re-link the ring list */
        if ((ptr_mqttd->msg_head != NULL) && (ptr_mqttd->msg_head != ptr_head))
        {
            while (ptr_head != ptr_msg->next)
            {
                ptr_msg = ptr_msg->next;
            }
            ptr_msg->next = ptr_mqttd->msg_head;
        }

        osapi_mutexGive(ptr_mqttmutex);
    }
}


/* FUNCTION NAME:  _mqttd_tmr
 * PURPOSE:
 *      The timer process
 *
 * INPUT:
 *      ptr_xTimer
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
static void _mqttd_tmr(timehandle_t ptr_xTimer)
{
    if (mqttd.state == MQTTD_STATE_RUN)
    {
        _mqttd_send_remain_msg(&mqttd);
    }
}
/*=== DB related local functions ===*/
/* FUNCTION NAME:  _mqttd_gen_client_id
 * PURPOSE:
 *      Get the hardware verion and MAC address of the device
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of MQTTD ctrl structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The string length of generated client_id
 *
 * NOTES:
 *      None
 */
static UI8_T _mqttd_gen_client_id(MQTTD_CTRL_T *ptr_mqttd)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    MW_MAC_T sys_mac;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;

    /* get sys_mac */
    osapi_memset(sys_mac, 0, sizeof(sys_mac));
    rc = mqttd_queue_getData(SYS_OPER_INFO, SYS_OPER_MAC, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        MW_LOG_INFO(MQTTD, "get sys_mac success, ptr_msg =%p", db_msg);
        memcpy(sys_mac, db_data, db_size);
        MW_FREE(db_msg);
    }
    else
    {
        MW_LOG_ERROR(MQTTD, "%s", "get sys_mac failed");
    }

    /* get hardware version ex. EN8851*/
    rc = mqttd_queue_getData(SYS_OPER_INFO, SYS_OPER_HW_VER, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc)
    {
        MW_LOG_INFO(MQTTD, "get hw_version success, ptr_msg =%p", db_msg);
        osapi_snprintf(ptr_mqttd->client_id, MQTTD_MAX_CLIENT_ID_SIZE, MQTTD_CLIENT_ID_FMT, (char *)db_data,
            sys_mac[0], sys_mac[1], sys_mac[2], sys_mac[3], sys_mac[4], sys_mac[5]);
        MW_FREE(db_msg);
    }
    else
    {
        MW_LOG_ERROR(MQTTD, "%s", "get hw_version failed");
        osapi_snprintf(ptr_mqttd->client_id, MQTTD_MAX_CLIENT_ID_SIZE, MQTTD_CLIENT_ID_FMT, MQTTD_CLIENT_ID_PRODUCT_DFT,
            sys_mac[0], sys_mac[1], sys_mac[2], sys_mac[3], sys_mac[4], sys_mac[5]);
    }

    MW_LOG_DEBUG(MQTTD, "Create MQTTD client id: %s", ptr_mqttd->client_id);
    return osapi_strlen(ptr_mqttd->client_id);
}

/* for example */
#if 0
/* FUNCTION NAME:  _mqttd_subscribe_db
 * PURPOSE:
 *      Subscribe all internal DB tables
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of MQTTD ctrl structure
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
static MW_ERROR_NO_T
_mqttd_subscribe_db(
    MQTTD_CTRL_T *ptr_mqttd)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI32_T msg_size = (DB_MSG_HEADER_SIZE + (TABLES_LAST * DB_MSG_PAYLOAD_SIZE));
    UI16_T offset= 0;
    UI8_T db_tidx = 0;

    MW_LOG_INFO(MQTTD, "Subscirbe internal DB");

    if (TRUE == ptr_mqttd->db_subscribed)
    {
        return MW_E_OK;
    }
    if (ptr_mqttd->state == MQTTD_STATE_DISCONNECTED)
    {
        return MW_E_OP_INVALID;
    }

    /* create the subscribe data payload */
    rc = osapi_calloc(msg_size, MQTTD_QUEUE_NAME, (void **)(&ptr_msg));
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Failed to allocate memory(rc = %u)", rc);
        return rc;
    }

    offset = dbapi_setMsgHeader(ptr_msg, MQTTD_QUEUE_NAME, M_SUBSCRIBE, TABLES_LAST);
    ptr_data = (UI8_T *)ptr_msg;
    for (db_tidx = 0; db_tidx < TABLES_LAST; db_tidx++)
    {
        /* message header */
        #ifdef AIR_LITE_MW
        if(MIB_CNT == db_tidx)
        {
            continue;
        }
        #endif
        offset += dbapi_setMsgPayload(M_SUBSCRIBE, db_tidx, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL, ptr_data + offset);
    }

    /* send request */
    rc = dbapi_sendMsg(ptr_msg, MQTTD_MUX_LOCK_TIME);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Failed to send message to DB Queue");
        osapi_free(ptr_msg);
        return rc;
    }
    MW_LOG_INFO(MQTTD, "Subscribe DB success, db_msg =%p", ptr_msg);
    ptr_mqttd->db_subscribed = TRUE;
    return rc;
}

/* FUNCTION NAME:  _mqttd_unsubscribe_db
 * PURPOSE:
 *      Unsubscribe all internal DB tables
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of MQTTD ctrl structure
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
static MW_ERROR_NO_T
_mqttd_unsubscribe_db(
    MQTTD_CTRL_T *ptr_mqttd)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI32_T msg_size = (DB_MSG_HEADER_SIZE + (TABLES_LAST * DB_MSG_PAYLOAD_SIZE));
    UI16_T offset= 0;
    UI8_T db_tidx = 0;

    MW_LOG_INFO(MQTTD, "Unsubscirbe internal DB");

    if (FALSE == ptr_mqttd->db_subscribed)
    {
        return MW_E_OK;
    }

    /* create the subscribe data payload */
    rc = osapi_calloc(msg_size, MQTTD_QUEUE_NAME, (void **)(&ptr_msg));
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Failed to allocate memory(rc = %u)", rc);
        return rc;
    }

    offset = dbapi_setMsgHeader(ptr_msg, MQTTD_QUEUE_NAME, M_UNSUBSCRIBE, TABLES_LAST);
    ptr_data = (UI8_T *)ptr_msg;
    for (db_tidx = 0; db_tidx < TABLES_LAST; db_tidx++)
    {
        /* message header */
        offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, db_tidx, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL, ptr_data + offset);
    }

    /* send request */
    rc = dbapi_sendMsg(ptr_msg, MQTTD_MUX_LOCK_TIME);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Failed to send message to DB Queue");
        osapi_free(ptr_msg);
        return rc;
    }
    MW_LOG_INFO(MQTTD, "Unsubscribe DB success, db_msg =%p", ptr_msg);
    ptr_mqttd->db_subscribed = FALSE;
    return rc;
}
#endif

/* FUNCTION NAME:  _mqttd_listen_db
 * PURPOSE:
 *      Listen to DB's notification
 *
 * INPUT:
 *      ptr_mqttd    --  the pointer of MQTTD ctrl structure
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
_mqttd_listen_db(
    MQTTD_CTRL_T *ptr_mqttd)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T *ptr_topic = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    UI16_T msg_size = 0;
    UI8_T count = 0;
    UI8_T method = 0;
    DB_REQUEST_TYPE_T req;

    /* Block mode to receive the DB message */
    rc = dbapi_recvMsg(
            MQTTD_QUEUE_NAME,
            &ptr_msg,
            MQTTD_QUEUE_TIMEOUT);
    if (MW_E_OK != rc)
    {
        return;
    }

    MW_LOG_INFO(MQTTD, "get ptr_msg =%p", ptr_msg);
    if (M_B_RESPONSE == (ptr_msg->method & M_B_RESPONSE))
    {
        MW_LOG_INFO(MQTTD, "free the response meesage: ptr_msg =%p", ptr_msg);
        osapi_free(ptr_msg);
        return;
    }

    osapi_calloc(MQTTD_MAX_TOPIC_SIZE, MQTTD_TASK_NAME, (void **)&ptr_topic);
    /* Send the updates to Cloud */
    if ((ptr_mqttd->ptr_client != NULL) && (NULL != ptr_topic))
    {
        count = ptr_msg->type.count;
        MW_LOG_INFO(MQTTD, "count =%d", count);
        if (M_GET == ptr_msg->method)
        {
            method = M_CREATE;
        }
        else
        {
            method = ptr_msg->method;
        }
        MW_LOG_INFO(MQTTD, "method =%u", method);
        ptr_data = (UI8_T *)&(ptr_msg->ptr_payload);
        while (count > 0)
        {
            memcpy((void *)&req, (const void *)ptr_data, sizeof(DB_REQUEST_TYPE_T));
            ptr_data += sizeof(DB_REQUEST_TYPE_T);
            MW_LOG_INFO(MQTTD, "T/F/E =%u/%u/%u", req.t_idx, req.f_idx, req.e_idx);
            /* append db json data to PUBLISH request list */
            memcpy((void *)&msg_size, (const void *)ptr_data, sizeof(UI16_T));
            MW_LOG_INFO(MQTTD, "data size =%u", msg_size);
            ptr_data += sizeof(UI16_T);
            #ifdef AIR_LITE_MW
            if(MIB_CNT == req.t_idx)
            {
                count--;
                ptr_data += msg_size;
                continue;
            }
            #endif
            /* append db rawdata to PUBLISH request list */
            if (0 != _mqttd_db_topic_set(ptr_mqttd, method, req.t_idx, req.f_idx, req.e_idx, ptr_topic, MQTTD_MAX_TOPIC_SIZE))
            {
                /* append db rawdata to PUBLISH request list */
                (void)_mqttd_publish_data(ptr_mqttd, method, ptr_topic, msg_size, ptr_data);
            }
            else
            {
                MW_LOG_ERROR(MQTTD, "Cannot send data: [T/F/E] %u/%u/%u", req.t_idx, req.f_idx, req.e_idx);
            }
            count--;
            ptr_data += msg_size;
        }
    }
    MW_LOG_INFO(MQTTD, "free ptr_msg =%p\n", ptr_msg);
    MW_FREE(ptr_msg);
    MW_FREE(ptr_topic);
}

/* FUNCTION NAME:  _mqttd_db_proxy
 * PURPOSE:
 *      Proxy the mqtt request to internal DB
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of MQTTD ctrl structure
 *      data       --  incoming data
 *      len        --  incoming data length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      Only support RAWDATA
 */
static void _mqttd_db_proxy(MQTTD_CTRL_T *ptr_mqttd, const u8_t *data, u16_t len)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    MQTTD_PUB_MSG_OLD_T *ptr_msg = (MQTTD_PUB_MSG_OLD_T *)data;
    UI8_T *ptr_data = NULL;
    DB_MSG_T *ptr_db_msg = NULL;
    void *ptr_db_data = NULL;
    DB_REQUEST_TYPE_T req;
    UI32_T msg_size = 0;
    UI16_T data_size = 0;
    UI16_T offset = 0;
    UI8_T count = 0;

    MW_LOG_DEBUG(MQTTD, "Proxy the cloud PUBLISH request to DB.");

    if ((ptr_mqttd == NULL) || (len == 0))
    {
        MW_LOG_DEBUG(MQTTD, "Argument error.");
        return;
    }
    if (ptr_msg->cldb_id != ptr_mqttd->cldb_id)
    {
        MW_LOG_DEBUG(MQTTD, "The message is not for me.");
        return;
    }

    /* message */
    MW_LOG_DEBUG(MQTTD, "Incoming session_id=%u", ptr_msg->session_id);
    MW_LOG_DEBUG(MQTTD, "Incoming method=0x%X", ptr_msg->method);
    MW_LOG_DEBUG(MQTTD, "Incoming count=%u", ptr_msg->type.count);

    count = ptr_msg->type.count;
    ptr_data = (UI8_T *)&(ptr_msg->request);
    while (count > 0)
    {
        ptr_data += sizeof(DB_REQUEST_TYPE_T);
        data_size = ((*(ptr_data+1) << 8) | (*ptr_data));
        msg_size += data_size;
        ptr_data += sizeof(UI16_T);
        _mqttd_dataDump((const void *)ptr_data, data_size);
        MW_LOG_DEBUG(MQTTD, "Incoming t_idx/f_idx/e_idx=%u/%u/%u", req.t_idx, req.f_idx, req.e_idx);
        MW_LOG_DEBUG(MQTTD, "Incoming data_size=%u", data_size);
        count--;
    }
    msg_size += DB_MSG_HEADER_SIZE;

    /* allocate the message and send to internal DB */
    rc = osapi_calloc(msg_size, MQTTD_QUEUE_NAME, (void **)&ptr_db_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "%s: allocate memory failed(%d)", __func__, rc);
        return;
    }

    count = ptr_msg->type.count;
    offset = dbapi_setMsgHeader(ptr_db_msg, MQTTD_QUEUE_NAME, ptr_msg->method, count);
    ptr_db_data = (void *)ptr_db_msg;
    ptr_data = (UI8_T *)&(ptr_msg->request);
    while (count > 0)
    {
        memcpy((void *)&req, (const void *)&ptr_data, sizeof(DB_REQUEST_TYPE_T));
        ptr_data += sizeof(DB_REQUEST_TYPE_T);
        data_size = ((*(ptr_data+1) << 8) | (*ptr_data));
        ptr_data += sizeof(UI16_T);
        offset += dbapi_setMsgPayload(ptr_msg->method, req.t_idx, req.f_idx, req.e_idx, ptr_data, ptr_db_data + offset);
        ptr_data += data_size;
        count--;
    }

    /* send request */
    rc = dbapi_sendMsg(ptr_db_msg, MQTTD_MUX_LOCK_TIME);
    if (MW_E_OK != rc)
    {
        osapi_free(ptr_db_msg);
    }
}

/**
 * Extract URI parameters from the parameter-part of an URI in the form
 * "test.cgi?x=y" @todo: better explanation!
 * This function refers to httpd.c extract_uri_parameters function
 * and modifies the incoming buffer directly.
 *
 * @param params pointer to the NULL-terminated parameter string from the URI
 * @return number of parameters extracted
 */
static I32_T extract_uri_parameters(C8_T *params, C8_T **ptr_param, C8_T **ptr_param_val)
{
    C8_T *pair;
    C8_T *equals;
    I32_T loop;

    /* If we have no parameters at all, return immediately. */
    if (!params || (params[0] == '\0'))
    {
        return (0);
    }

    /* Get a pointer to our first parameter */
    pair = params;

    /* Parse up to LWIP_HTTPD_MAX_CGI_PARAMETERS from the passed string and ignore the
     * remainder (if any) */
    for (loop = 0; (loop < LWIP_HTTPD_MAX_CGI_PARAMETERS) && pair; loop++)
    {
        /* Save the name of the parameter */
        ptr_param[loop] = pair;

        /* Remember the start of this name=value pair */
        equals = pair;

        /* Find the start of the next name=value pair and replace the delimiter
         * with a 0 to terminate the previous pair string. */
        pair = strchr(pair, '&');
        if (pair)
        {
            *pair = '\0';
            pair++;
        }
        else
        {
            /* We didn't find a new parameter so find the end of the URI and
             * replace the space with a '\0' */
            pair = strchr(equals, ' ');
            if (pair)
            {
                *pair = '\0';
            }

            /* Revert to NULL so that we exit the loop as expected. */
            pair = NULL;
        }

        /* Now find the '=' in the previous pair, replace it with '\0' and save
         * the parameter value string. */
        equals = strchr(equals, '=');
        if (equals)
        {
            *equals = '\0';
            ptr_param_val[loop] = equals + 1;
        }
        else
        {
            ptr_param_val[loop] = NULL;
        }
    }

    return loop;
}

/* FUNCTION NAME:  _mqttd_cgi_proxy
 * PURPOSE:
 *      Proxy the mqtt request to CGI function
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of MQTTD ctrl structure
 *      data       --  incoming data
 *      len        --  incoming data length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      Only support MW html/cgi
 */
static void _mqttd_cgi_proxy(MQTTD_CTRL_T *ptr_mqttd, const u8_t *data, u16_t len)
{
    C8_T * uri = NULL;
    C8_T * params = NULL;
    C8_T * buff_end = NULL;
    const tCGI *mqttd_cgis = CGIURLs;
    UI16_T mqttd_cgi_num = get_numCgiHandler();
    UI16_T i = 0;
    I32_T  cgi_paramcount;
    C8_T *ptr_params[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Params extracted from the request URI */
    C8_T *ptr_param_vals[LWIP_HTTPD_MAX_CGI_PARAMETERS]; /* Values for each extracted param */

    if ((ptr_mqttd == NULL) || (data == NULL))
    {
        return;
    }
    if ((len > 0) && (mqttd_cgi_num > 0))
    {
        uri = (C8_T *)data;
        buff_end = (C8_T *)&ptr_mqttd->ptr_client->rx_buffer[MQTT_VAR_HEADER_BUFFER_LEN];
        if ((uri+len) <= buff_end)
        {
            uri[len] = '\0';
        }
        else
        {
            uri[len - 1] = '\0';
        }
        MW_LOG_DEBUG(MQTTD, "Incoming request: %s", uri);
        params = (char *)strchr(uri, '?');
        if (params != NULL) {
            /* URI contains parameters. NULL-terminate the base URI */
            *params = '\0';
            params++;
        }

        for (i = 0; i < mqttd_cgi_num; i++)
        {
            if (osapi_strcmp(uri, mqttd_cgis[i].pcCGIName) == 0)
            {
                /*
                 * We found a CGI that handles this URI so extract the
                 * parameters and call the handler.
                 */
                cgi_paramcount = extract_uri_parameters(params, ptr_params, ptr_param_vals);
                /* CGI handle function */
                MW_LOG_DEBUG(MQTTD, "handle=[%d] paramCnt=[%d]", i, cgi_paramcount);
#ifdef AIR_SUPPORT_HTTPD_MUTEX
                if (MW_E_OK == mw_httpd_mutex_take(MQTTD_MUX_LOCK_TIME))
                {
                    mqttd_cgis[i].pfnCGIHandler(i, cgi_paramcount, ptr_params, ptr_param_vals);
                    MW_LOG_DEBUG(MQTTD, "CGI Done");
                    mw_httpd_mutex_give();
                }
                else
                {
                    MW_LOG_DEBUG(MQTTD, "Failed to take httpd mutex");
                }
#else
                mqttd_cgis[i].pfnCGIHandler(i, cgi_paramcount, ptr_params, ptr_param_vals);
                MW_LOG_DEBUG(MQTTD, "CGI Done");
#endif
                break;
            }
        }
    }
}

/*=== MQTT related local functions ===*/
/* FUNCTION NAME: _mqttd_dataDump
 * PURPOSE:
 *       Dump raw data if debug level is greater than MQTT_DEBUG_DISABLE
 *
 * INPUT:
 *       data       -- The raw data
 *       daia_size  -- The size of raw data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *
 */
static void _mqttd_dataDump(const void *data, UI16_T data_size)
{
    UI32_T idx = 0, i = 0;
    I32_T count = 0;
    UI8_T *ptr_raw = (UI8_T *)data;
    C8_T rawdata[4] = {0};
    C8_T rawstr[MQTTD_MAX_BUFFER_SIZE] = {0};
    MW_LOG_LEVEL_T mqttd_debug_level = MW_LOG_LEVEL_OFF;

    if ((MW_E_OK != mw_log_get_level(MW_LOG_MODULE_MQTTD, &mqttd_debug_level)) ||
        (mqttd_debug_level < MW_LOG_LEVEL_DEBUG))
    {
        return;
    }

    while (idx < data_size)
    {
        memset(rawstr, 0, sizeof(rawstr));
        osapi_snprintf(rawstr, MQTTD_MAX_BUFFER_SIZE, "%04X: ", idx);
        for (i = 0; i < 16; i++)
        {
            count = osapi_sprintf(rawdata, " %02X", *ptr_raw);
            if (count <= 0)
            {
                idx = data_size;
                break;
            }
            strncat(rawstr, rawdata, count);
            ptr_raw++;
            idx++;
            if (idx >= data_size)
            {
                break;
            }
        }
        MW_LOG_DEBUG(MQTTD, "    %s", rawstr);
    }
}

/* FUNCTION NAME: _mqttd_db_topic_set
 * PURPOSE:
 *      Set the DB notification to MQTT message topic
 *
 * INPUT:
 *      ptr_mqttd       -- The control structure
 *      method          --  the method bitmap
 *      t_idx           --  the enum of the table
 *      f_idx           --  the enum of the field
 *      e_idx           --  the entry index in the table
 *      buf_size        --  the topic buffer size
 *
 * OUTPUT:
 *      topic           --  the topic to be sent
 *
 * RETURN:
 *      topic_length    -- the string legnth of topic
 *
 * NOTES:
 *
 */
static UI16_T _mqttd_db_topic_set(MQTTD_CTRL_T *ptr_mqttd, const UI8_T method, const UI8_T t_idx, const UI8_T f_idx, const UI16_T e_idx, C8_T *topic, UI16_T buf_size)
{
    I32_T length = 0;
    C8_T table_name[DB_MAX_KEY_SIZE] = {0};
    C8_T field_name[DB_MAX_KEY_SIZE] = {0};

    MW_PARAM_CHK((topic == NULL), 0);
    MW_PARAM_CHK((buf_size == 0), 0);

    if (MW_E_OK != dbapi_getTableName(t_idx, DB_MAX_KEY_SIZE, table_name))
    {
        return 0;
    }
    if (MW_E_OK != dbapi_getFieldName(t_idx, f_idx, DB_MAX_KEY_SIZE, field_name))
    {
        return 0;
    }

    /* The topic:
     * <msg topic>/<cldb_ID>/<table_name>/<field_name>/eidx
     */
    if (method == M_CREATE)
    {
        //PUBLISH the inital data
        length = osapi_snprintf(topic, buf_size, MQTTD_TOPIC_FORMAT,
                MQTTD_TOPIC_INIT, ptr_mqttd->cldb_id, table_name, field_name, e_idx);
    }
    else
    {
        //PUBLISH the internal DB notification
        length = osapi_snprintf(topic, buf_size, MQTTD_TOPIC_FORMAT,
                 MQTTD_TOPIC_DB, ptr_mqttd->cldb_id, table_name, field_name, e_idx);
    }
    if (length <= 0)
    {
        return 0;
    }
    MW_LOG_INFO(MQTTD, "Set topic: %s(len = %d).\n", topic, length);
    return (UI16_T)(length & 0xFFFF);
}

/* FUNCTION NAME: _mqttd_publish_cb
 * PURPOSE:
 *      MQTTD publish callback function
 *
 * INPUT:
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *
 */
static void _mqttd_publish_cb(void *arg, err_t err)
{
    MW_LOG_DEBUG(MQTTD, "Send Publish control packet code: (%d).\n", err);
}

/* FUNCTION NAME: _mqttd_tcpip_publish_cb
 * PURPOSE:
 *      Dispatched callback to call mqtt_publish in tcpip_thread context.
 *
 * INPUT:
 *      ptr_arg   -- The tcpip_api_call_data containing publish parameters
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ERR_OK if successful, or error code from mqtt_publish
 *
 * NOTES:
 *      This function ensures thread safety by executing the publish operation
 *      within the lwIP core thread.
 */
static err_t _mqttd_tcpip_publish_cb(struct tcpip_api_call_data *ptr_arg)
{
    MQTTD_TCPIP_CALL_MSG_T* p = (MQTTD_TCPIP_CALL_MSG_T *)(void *)ptr_arg;

    return mqtt_publish(p->ptr_mqttd_ctrl->ptr_client, p->mqtt_param.topic, p->mqtt_param.payload, p->mqtt_param.len, p->mqtt_param.qos, p->mqtt_param.retain, _mqttd_publish_cb, p->ptr_mqttd_ctrl);
}

/* FUNCTION NAME: _mqttd_tcpip_subscribe_cb
 * PURPOSE:
 *      Dispatched callback to call mqtt_sub_unsub in tcpip_thread context.
 *
 * INPUT:
 *      ptr_arg   -- The tcpip_api_call_data containing subscription parameters
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ERR_OK if successful, or error code from mqtt_sub_unsub
 *
 * NOTES:
 *      This function ensures thread safety by executing the (un)subscribe operation
 *      within the lwIP core thread.
 */
static err_t _mqttd_tcpip_subscribe_cb(struct tcpip_api_call_data *ptr_arg)
{
    MQTTD_TCPIP_CALL_MSG_T* p = (MQTTD_TCPIP_CALL_MSG_T *)(void *)ptr_arg;

    return mqtt_sub_unsub(p->ptr_mqttd_ctrl->ptr_client, p->mqtt_param.topic, p->mqtt_param.qos, p->mqtt_param.sub ? _mqttd_subscribe_cb : NULL, p->ptr_mqttd_ctrl, p->mqtt_param.sub);
}

/* FUNCTION NAME: _mqttd_publish_data
 * PURPOSE:
 *      Generate the data payload and PUBLISH
 *
 * INPUT:
 *      ptr_mqttd       -- The control structure
 *      method          --  the method bitmap
 *      topic           --  the message topic
 *      data_size       --  the message data size
 *      ptr_data        --  pointer to message data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *
 * NOTES:
 *
 */
static MW_ERROR_NO_T _mqttd_publish_data(MQTTD_CTRL_T *ptr_mqttd, const UI8_T method, C8_T *topic, const UI16_T data_size, const void *ptr_data)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    MQTTD_PUB_MSG_T *ptr_msg = NULL;
    MQTTD_TCPIP_CALL_MSG_T msg;
    UI8_T           *pdata = (UI8_T *)ptr_data;
    UI8_T           pkt_idx = 1;
    UI8_T           pkt_count = 1;
    UI16_T          topic_size = 0;
    UI16_T          msg_size = 0;
    UI16_T          total_size = 0;
    UI16_T          sent_size = data_size;

    MW_CHECK_PTR(ptr_mqttd);
    MW_CHECK_PTR(topic);

    if (ptr_mqttd->state == MQTTD_STATE_DISCONNECTED)
    {
        return MW_E_OP_INVALID;
    }
    MW_LOG_DEBUG(MQTTD, "Publish topic: %s", topic);

    /* Calculate the packet size */
    topic_size = osapi_strlen(topic);
    msg_size = MQTTD_MSG_HEADER_SIZE + sent_size;
    total_size = topic_size + msg_size;
    if (total_size > MQTTD_MAX_PACKET_SIZE)
    {
        msg_size = MQTTD_MAX_PACKET_SIZE - topic_size;
        sent_size = msg_size - MQTTD_MSG_HEADER_SIZE;
        pkt_count = (data_size / sent_size) + 1;
    }

    do
    {
        ptr_msg = NULL;
        /* allocate the message payload memory */
        rc = osapi_calloc(msg_size, MQTTD_TASK_NAME, (void **)&ptr_msg);
        if (MW_E_OK != rc)
        {
            MW_LOG_ERROR(MQTTD, "%s: allocate memory failed(%d)", __func__, rc);
            return MW_E_NO_MEMORY;
        }
        MW_LOG_DEBUG(MQTTD, "ptr_msg=%p", ptr_msg);

        /* message */
        ptr_msg->pid = pkt_idx;
        ptr_msg->count = pkt_count;
        ptr_msg->method = method;
        ptr_msg->data_size = sent_size;
        MW_LOG_DEBUG(MQTTD, "Publish pid/count: %u/%u, method=0x%X, data_size=%u", ptr_msg->pid, ptr_msg->count, ptr_msg->method, ptr_msg->data_size);

        if ((sent_size > 0) && (pdata != NULL))
        {
            /* copy the data to buffer */
            _mqttd_dataDump((const void *)pdata, sent_size);
            memcpy(&(ptr_msg->ptr_data), pdata, sent_size);
            pdata += sent_size;
        }

        /* Send PUBLISH message directly if no remain msg */
        if (ptr_mqttd->msg_head == NULL)
        {
            msg.ptr_mqttd_ctrl = ptr_mqttd;
            msg.mqtt_param.topic = topic;
            msg.mqtt_param.payload = (const void *)ptr_msg;
            msg.mqtt_param.len = msg_size;
            msg.mqtt_param.qos = MQTTD_REQUEST_QOS;
            msg.mqtt_param.retain = MQTTD_REQUEST_RETAIN;
            err_t err = tcpip_api_call(_mqttd_tcpip_publish_cb, &msg.call);
            if (ERR_OK == err)
            {
                MW_LOG_DEBUG(MQTTD, "PUBLISH directly, free the memory ptr_msg (%p)", ptr_msg);
                /* Already in MQTT request list, so the allocated memory can be free */
                osapi_free(ptr_msg);
                ptr_msg = NULL;
                rc = MW_E_OK;
            }
            else
            {
                MW_LOG_DEBUG(MQTTD, "Error (%d): PUBLISH failed: ptr_msg(%p).", err, ptr_msg);
                rc = MW_E_OP_INCOMPLETE;
            }
        }
        else
        {
            rc = MW_E_OP_INCOMPLETE;
        }

        if (MW_E_OK != rc)
        {
            /* Try to keep the data, but if the memory is full, then the packet will be lost */
            if (MW_E_OK != _mqttd_append_remain_msg(ptr_mqttd, (C8_T *)topic, msg_size, (void *)ptr_msg))
            {
                osapi_free(ptr_msg);
                ptr_msg = NULL;
                return MW_E_NO_MEMORY;
            }
        }

        if (pkt_idx == pkt_count)
        {
            /* All data sent or keep */
            break;
        }

        /* Calculate the remained data_size  */
        total_size = total_size - sent_size;
        if (total_size > MQTTD_MAX_PACKET_SIZE)
        {
            msg_size = MQTTD_MAX_PACKET_SIZE - topic_size;
        }
        else
        {
            msg_size = total_size - topic_size;
        }
        sent_size = msg_size - MQTTD_MSG_HEADER_SIZE;
        pkt_idx++;
    } while (pkt_idx <= pkt_count);
    return MW_E_OK;
}

/* FUNCTION NAME: _mqttd_incoming_publish_cb
 * PURPOSE:
 *      MQTTD publish header callback function
 *
 * INPUT:
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *
 */
static void _mqttd_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;
    MW_LOG_DEBUG(MQTTD, "Incoming topic is \"%s\"", topic);
    osapi_memset(ptr_mqttd->pub_in_topic, 0, MQTTD_MAX_TOPIC_SIZE);
    osapi_strncpy(ptr_mqttd->pub_in_topic, topic, (MQTTD_MAX_TOPIC_SIZE-1));
}

/* FUNCTION NAME: _mqttd_incoming_data_cb
 * PURPOSE:
 *      MQTTD publish data payload callback function
 *
 * INPUT:
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *
 */
static void _mqttd_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;
    UI16_T idata = 0;
    UI32_T cldb_idx;
    C8_T new_cldbId[MQTTD_MAX_TOPIC_SIZE] = {0};
    osapi_snprintf(new_cldbId, MQTTD_MAX_TOPIC_SIZE, "%s/new/%s", MQTTD_TOPIC_CLOUD_PREFIX, ptr_mqttd->client_id);

    MW_LOG_DEBUG(MQTTD, "Incoming data length: %d", len);

    /* cmw/connected */
    if (0 == osapi_strcmp(ptr_mqttd->pub_in_topic, MQTTD_CLOUD_CONNECTED))
    {
        MW_LOG_DEBUG(MQTTD, "Incoming data: %s", data);
        MW_LOG_DEBUG(MQTTD, "cloud is connected");
    }
    /* cmw/new/<clientID> */
    else if (0 == osapi_strcmp(ptr_mqttd->pub_in_topic, new_cldbId))
    {
        if (len == sizeof(UI8_T))
        {
            idata = *data;
        }
        else
        {
            //idata = *((UI16_T *)data);
            idata = ((*(data+1) << 8) | (*data));
        }
        MW_LOG_DEBUG(MQTTD, "Get new cloud ID: %d", idata);

        if (ptr_mqttd->cldb_id != idata)
        {
            /* connect to new broker */
            ptr_mqttd->cldb_id = idata;
            MW_LOG_DEBUG(MQTTD, "mqttd switch id is \"%d\"", ptr_mqttd->cldb_id);
            ptr_mqttd->state = MQTTD_STATE_INITING;
        }
    }
    /* mwcloud/db */
    else if (0 == osapi_strcmp(ptr_mqttd->pub_in_topic, MQTTD_CLOUD_TODB))
    {
        MW_LOG_DEBUG(MQTTD, "receive cloud DB request");
        /* Send the data to internal DB */
        _mqttd_db_proxy(ptr_mqttd, data, len);
    }
    /* cmw/cgi/<cldb_ID> */
    else if (1 == sscanf(ptr_mqttd->pub_in_topic, MQTTD_CLOUD_CGI "/%u", &cldb_idx))
    {
        idata = (UI16_T)(cldb_idx & 0xFFFF);
        if ((ptr_mqttd->cldb_id == idata) && (cldb_idx <= 0xFFFF))
        {
            MW_LOG_DEBUG(MQTTD, "receive cloud CGI request");
            /* Call CGI function */
            _mqttd_cgi_proxy(ptr_mqttd, data, len);
        }
    }
    /* cmw/will */
    else if (0 == osapi_strcmp(ptr_mqttd->pub_in_topic, MQTTD_CLOUD_WILL))
    {
        MW_LOG_DEBUG(MQTTD, "receive cloud disconnected");
        ptr_mqttd->state = MQTTD_STATE_DISCONNECTED;
        MW_LOG_DEBUG(MQTTD, "Incoming data: %s", data);
    }
     /* Do nothing */
    else {}
}

/* FUNCTION NAME:  _mqttd_send_clientId
 * PURPOSE:
 *      MQTTD PUBLISH Device Client ID to Cloud
 *
 * INPUT:
 *      client  --  The pointer of client handle
 *      arg     --  The pointer of MQTTD ctrl structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 *
 */
static void _mqttd_send_clientId(mqtt_client_t *client, void *arg)
{
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;
    C8_T reg_new[32] = {0};
    C8_T reg_msg[36] = {0};
    MQTTD_TCPIP_CALL_MSG_T msg;
    err_t pub_err = ERR_MEM;
    I32_T msg_len = 0;

    /* If SUBACK received, then PUBLISH the device client-id to get the cloud DB ID */
    osapi_snprintf(reg_new, 32, "%s/%s", MQTTD_TOPIC_NEW, ptr_mqttd->client_id);
    msg_len = osapi_snprintf(reg_msg, 36, MQTTD_CLIENT_CONNECT_MSG_FMT, PLAT_MAX_PORT_NUM, MQTTD_MSG_VER);
    msg.ptr_mqttd_ctrl = ptr_mqttd;
    msg.mqtt_param.topic = reg_new;
    msg.mqtt_param.payload = (const void *)reg_msg;
    msg.mqtt_param.len = (u16_t)msg_len;
    msg.mqtt_param.qos = MQTTD_REQUEST_QOS;
    msg.mqtt_param.retain = MQTTD_REQUEST_RETAIN;
    pub_err = tcpip_api_call(_mqttd_tcpip_publish_cb, &msg.call);

    if (ERR_OK == pub_err)
    {
        ptr_mqttd->state = MQTTD_STATE_INITING;
    }
}


/* FUNCTION NAME:  _mqttd_subscribe_cb
 * PURPOSE:
 *      MQTTD SUBSCRIBE callback function
 *
 * INPUT:
 *      arg     --  The pointer of MQTTD ctrl structure
 *      err     --  The subscribe process error status
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 *
 */
static void _mqttd_subscribe_cb(void *arg, err_t err)
{
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;
    MW_LOG_DEBUG(MQTTD, "mqtt subscribe error state is (%d)\n", (UI8_T)err);

    if (err == ERR_OK)
    {
        ptr_mqttd->state = MQTTD_STATE_SUBACK;
    }
    else
    {
        /* If SUBSCRIBE failed, then try to re-subscribe again */
        MW_LOG_ERROR(MQTTD, "\n MQTT SUBSCRIBE failed (%d)", (UI8_T)err);
        ptr_mqttd->state = MQTTD_STATE_CONNECTED;
    }
}

/* FUNCTION NAME:  _mqttd_send_subscribe
 * PURPOSE:
 *      MQTTD send SUBSCRIBE ctrl packet
 *
 * INPUT:
 *      client  --  The pointer of client handle
 *      arg     --  The pointer of MQTTD ctrl structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 *
 */
static void _mqttd_send_subscribe(mqtt_client_t *client, void *arg)
{
    err_t err;
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;
    MQTTD_TCPIP_CALL_MSG_T msg;
    MW_LOG_DEBUG(MQTTD, "mqtt send subscribe\n");

    msg.ptr_mqttd_ctrl = ptr_mqttd;
    msg.mqtt_param.topic = MQTTD_CLOUD_TODB;
    msg.mqtt_param.qos = MQTTD_REQUEST_QOS;
    msg.mqtt_param.sub = 1; /* 1 for subscribe */
    err = tcpip_api_call(_mqttd_tcpip_subscribe_cb, &msg.call);
    if (err != ERR_OK)
    {
        return;
    }

    msg.mqtt_param.topic = MQTTD_SUB_CLOUD_FILTER;
    err = tcpip_api_call(_mqttd_tcpip_subscribe_cb, &msg.call);
    if (err == ERR_OK)
    {
        ptr_mqttd->state = MQTTD_STATE_SUBSCRIBE;
    }
}

/* FUNCTION NAME:  _mqttd_connection_cb
 * PURPOSE:
 *      MQTTD connection callback function
 *
 * INPUT:
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *  This callback function may be called when MQTT_CONNECTED or TCP_DISCONNECTED
 */
static void _mqttd_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    MQTTD_CTRL_T *ptr_mqttd = (MQTTD_CTRL_T *)arg;

    MW_LOG_INFO(MQTTD, "mqtt connection state is \"%s(%d)\"", (status == MQTT_CONNECT_ACCEPTED)? "CONNECTED" : "DISCONNECTED", (UI16_T)status);

    if (status != MQTT_CONNECT_ACCEPTED)
    {
        // If callback by mqtt_close, then free the client
        if (ptr_mqttd->state < MQTTD_STATE_DISCONNECTED)
        {
            ptr_mqttd->state = MQTTD_STATE_DISCONNECTED;
            ptr_mqttd->reconnect = TRUE;
        }
        return;
    }

    // Set publish callback functions
    mqtt_set_inpub_callback(mqttd.ptr_client, _mqttd_incoming_publish_cb, _mqttd_incoming_data_cb, (void *)&mqttd);
    ptr_mqttd->state = MQTTD_STATE_CONNECTED;
}

/* FUNCTION NAME: _mqttd_lookup_server
 * PURPOSE:
 *      Lookup the remote server IP address
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of the mqttd ctrl structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
static err_t _mqttd_lookup_server(MQTTD_CTRL_T *ptr_mqttd)
{
    I8_T err = ERR_OK;
    ip4_addr_t *ptr_server = NULL;

    ptr_server = ip_2_ip4(&(ptr_mqttd->server_ip));
    if (ptr_server->addr != IPADDR_ANY)
    {
        /* server ip set by console */
        MW_LOG_DEBUG(MQTTD, "Connect to original MQTT remote server");
        return mqttdapi_tcpip_call(_mqttd_client_connect_callback);
    }

#if LWIP_DNS
    err = netconn_gethostbyname((const char *)cloud_hostname, &(ptr_mqttd->server_ip));
#else
    ip_addr_copy(ptr_mqttd->server_ip, mqttd_server_ip);
#endif
    if (err == ERR_OK)
    {
        return mqttdapi_tcpip_call(_mqttd_client_connect_callback);
    }
    else
    {
        // failed due to memory or argument error
        MW_LOG_WARN(MQTTD, "Cannot use DNS:(error %d), use MQTTD default remote server", (int)err);
        ip_addr_copy(ptr_mqttd->server_ip, mqttd_server_ip);
        return mqttdapi_tcpip_call(_mqttd_client_connect_callback);
    }
    return ERR_OK;
}
/* FUNCTION NAME:  _mqttd_client_connect
 * PURPOSE:
 *      Generate the Client ID and connect to the remote MQTT server.
 *
 * INPUT:
 *      ptr_mqttd  --  the pointer of the mqttd ctrl structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
static err_t _mqttd_client_connect_callback(struct tcpip_api_call_data *ptr_arg)
{
    I8_T err = ERR_OK;
    struct mqtt_connect_client_info_t client_info;
    UI16_T portnum = MQTT_PORT;
    MQTTD_TCPIP_CALL_MSG_T* ptr_callback_msg = (MQTTD_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    MQTTD_CTRL_T *ptr_mqttd = ptr_callback_msg->ptr_mqttd_ctrl;

    if (ptr_mqttd->ptr_client != NULL)
    {
        if (TRUE == mqtt_client_is_connected(ptr_mqttd->ptr_client))
        {
            return ERR_OK;
        }
    }

#if MQTTD_SUPPORT_TLS
    portnum = MQTT_TLS_PORT;
    const UI8_T* ptr_ca_cert = CA;
    size_t ca_len = sizeof(CA);
    tls_config = altcp_tls_create_config_client(ptr_ca_cert, ca_len);
    if (NULL == tls_config)
    {
        MW_LOG_ERROR(MQTTD, "Unable to create TLS config for MQTT, close MQTT!");
        return ERR_MEM;
    }
#endif

    /* Generate the client information */
    client_info = (struct mqtt_connect_client_info_t){
        (const char *)ptr_mqttd->client_id,  /* Client identifier, must be set by caller */
        MQTTD_USERNAME,           /* User name, set to NULL if not used */
        MQTTD_PASSWD,             /* Password, set to NULL if not used */
        MQTTD_KEEP_ALIVE,         /* keep alive time in seconds, 0 to disable keep alive functionality*/
        MQTTD_WILL_TOPIC,         /* will topic, set to NULL if will is not to be used,
                                     will_msg, will_qos and will retain are then ignored */
        (const char *)ptr_mqttd->client_id,  /* will_msg, see will_topic */
        MQTTD_WILL_QOS,           /* will_qos, see will_topic */
        MQTTD_WILL_RETAIN         /* will_retain, see will_topic */
#if MQTTD_SUPPORT_TLS
        ,tls_config               /* TLS configuration for secure connections */
#endif
        };

    MW_LOG_DEBUG(MQTTD, "MQTTD create a new client (%p) try to connect to %s", ptr_mqttd->ptr_client, ipaddr_ntoa(&ptr_mqttd->server_ip));
    if (ptr_mqttd->ptr_client == NULL)
    {
        ptr_mqttd->ptr_client = mqtt_client_new();
        if (ptr_mqttd->ptr_client == NULL)
        {
#if MQTTD_SUPPORT_TLS
            altcp_tls_free_config(tls_config);
            tls_config = NULL;
#endif
            MW_LOG_ERROR(MQTTD, "\nconnect_mqtt: create new mqtt client failed.");
            return ERR_MEM;
        }
    }

    // connect to MQTT server
    err = mqtt_client_connect(ptr_mqttd->ptr_client, (const ip_addr_t *)&(ptr_mqttd->server_ip), portnum,
            _mqttd_connection_cb, (void *)ptr_mqttd, (const struct mqtt_connect_client_info_t *)&client_info);
    if (ERR_OK != err)
    {
        MW_LOG_WARN(MQTTD, "\nconnect_mqtt: connect to remote mqtt server failed: %d.", err);
        if (ERR_MEM == err)
        {
#if MQTTD_SUPPORT_TLS
            altcp_tls_free_config(tls_config);
            tls_config = NULL;
#endif
            return ERR_MEM;
        }
        else
        {
            return ERR_VAL;
        }
    }
    return ERR_OK;
}

/* FUNCTION NAME:  _mqttd_client_disconnect
 * PURPOSE:
 *      Disconnect to the remote MQTT server.
 *
 * INPUT:
 *      ptr_mqttclient   --  the pointer of the client
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
static err_t _mqttd_client_disconnect_callback(struct tcpip_api_call_data *ptr_arg)
{
    MQTTD_TCPIP_CALL_MSG_T* ptr_callback_msg = (MQTTD_TCPIP_CALL_MSG_T *)(void *)ptr_arg;
    if (ptr_callback_msg->ptr_mqttd_ctrl->ptr_client != NULL)
    {
        // Disconnect MQTT
        MW_LOG_DEBUG(MQTTD, "MQTTD client disconnect.");
        mqtt_disconnect(ptr_callback_msg->ptr_mqttd_ctrl->ptr_client);
    }
    return ERR_OK;
}

static void _mqttd_reconnect(void *ptr_arg)
{
    mqttdapi_tcpip_call(_mqttd_client_disconnect_callback);
    mqttdapi_tcpip_call(_mqttd_ctrl_free_callback);

    if(NULL != ptr_mqttd_time)
    {
        MW_LOG_DEBUG(MQTTD, "MQTTD free the timer.");
        if(MW_E_OK == osapi_timerActive(ptr_mqttd_time))
        {
            osapi_timerStop(ptr_mqttd_time);
        }
        osapi_timerDelete(ptr_mqttd_time);
        ptr_mqttd_time = NULL;
    }

    _mqttd_ctrl_init(&mqttd, (ip_addr_t *)ptr_arg);
    osapi_timerCreate(
        MQTTD_TIMER_NAME,
        _mqttd_tmr,
        TRUE,
        MQTTD_TIMER_PERIOD,
        NULL,
        &ptr_mqttd_time);
    if(NULL == ptr_mqttd_time)
    {
        MW_LOG_ERROR(MQTTD, "Failed to create MQTTD timer.");
        mqttd.state = MQTTD_STATE_SHUTDOWN;
        return;
    }
    _mqttd_connect_init();
}

static void _mqttd_connect_init(void)
{
    MW_ERROR_NO_T rc = MW_E_NOT_INITED;
    struct netif *xNetIf = NULL;
    err_t err = ERR_OK;

    /* Waiting until DB and Netif are ready */
    do
    {
        rc = dbapi_dbisReady();
    }while(rc != MW_E_OK);

    /* Initialize client ID */
    (void)_mqttd_gen_client_id(&mqttd);

    do{
        xNetIf = netif_find_default();
        osapi_delay(MQTTD_MUX_LOCK_TIME);
    } while (xNetIf == NULL);

    MW_LOG_DEBUG(MQTTD, "MQTTD use NetIf num:%d, name:%s, mac:[%02x:%02x:%02x:%02x:%02x:%02x]",
        xNetIf->num, xNetIf->name,
        xNetIf->hwaddr[0],
        xNetIf->hwaddr[1],
        xNetIf->hwaddr[2],
        xNetIf->hwaddr[3],
        xNetIf->hwaddr[4],
        xNetIf->hwaddr[5]);

    /* Connect to MQTT */
    do{
        /* Connect to MQTT */
        err = _mqttd_lookup_server(&mqttd);
        if (err == ERR_MEM)
        {
            MW_LOG_WARN(MQTTD, "Failed to connect to mqtt server due to no memory, close mqttd!");
            mqttd.state = MQTTD_STATE_SHUTDOWN;
            break;
        }
        else if (err != ERR_OK)
        {
            MW_LOG_WARN(MQTTD, "Failed to connect to mqtt server.");
            mqttd.reconnect = TRUE;
            mqttd.state = MQTTD_STATE_DISCONNECTED;
            break;
        }
        else
        {
            if(MW_E_OK != osapi_timerStart(ptr_mqttd_time))
            {
                MW_LOG_ERROR(MQTTD, "Failed to start MQTTD timer. MQTTD Stopped.");
                mqttd.state = MQTTD_STATE_SHUTDOWN;
                break;
            }
        }
    }while(0);
}

static void _mqttd_main(void *arg)
{
    UI32_T reconnect_counter = 0;
    UI32_T reconnect_interval = INITIAL_RECONNECT_INTERVAL;

    _mqttd_connect_init();

    while(mqttd.state < MQTTD_STATE_SHUTDOWN)
    {
        switch (mqttd.state)
        {
            case MQTTD_STATE_CONNECTED:
            {
                _mqttd_send_subscribe(mqttd.ptr_client, (void *)&mqttd);
                break;
            }
            case MQTTD_STATE_SUBACK:
            {
                // Set publish callback functions
                _mqttd_send_clientId(mqttd.ptr_client, (void *)&mqttd);
                break;
            }
            case MQTTD_STATE_INITING:
            {
                if (0 != mqttd.cldb_id)
                {
                    mqttd.state = MQTTD_STATE_RUN;
                }
                else
                {
                    osapi_delay(MQTTD_MUX_LOCK_TIME);
                }
                break;
            }
            case MQTTD_STATE_RUN:
            {
                /* Waiting for DB send notification */
                _mqttd_listen_db(&mqttd);
                break;
            }
            case MQTTD_STATE_DISCONNECTED:
            {
                MW_LOG_INFO(MQTTD, "Reconnect to remote server: %s", ipaddr_ntoa(&mqttd.server_ip));
                _mqttd_reconnect(((void *)&(mqttd.server_ip)));
                if (mqttd.state != MQTTD_STATE_DISCONNECTED)
                {
                    reconnect_counter = 0;
                    reconnect_interval = INITIAL_RECONNECT_INTERVAL;
                }
                else
                {
                    reconnect_counter++;
                    if (reconnect_counter <= MAX_INITIAL_RETRIES)
                    {
                        reconnect_interval = INITIAL_RECONNECT_INTERVAL;
                    }
                    else if (reconnect_counter <= (MAX_INITIAL_RETRIES + MAX_SECOND_RETRIES))
                    {
                        reconnect_interval = SECOND_RECONNECT_INTERVAL;
                    }
                    else if (reconnect_counter <= (MAX_INITIAL_RETRIES + MAX_SECOND_RETRIES + MAX_FINAL_RETRIES))
                    {
                        reconnect_interval = FINAL_RECONNECT_INTERVAL;
                    }
                    else
                    {
                        MW_LOG_INFO(MQTTD, "Stop the MQTT daemon");
                        mqttd.state = MQTTD_STATE_SHUTDOWN;
                        break;
                    }
                    osapi_delay(reconnect_interval);
                }
                break;
            }
            default:
            {
                osapi_delay(MQTTD_MUX_LOCK_TIME);
                break;
            }
        }
    }

    _mqttd_deinit();
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mqttd_init
 * PURPOSE:
 *      Initialize MQTT client daemon
 *
 * INPUT:
 *      arg  --  the remote server IP
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      If connect to default remote MQTT server,  then set arg to NULL.
 */
MW_ERROR_NO_T mqttd_init(void *arg)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if (ptr_mqttdmain != NULL)
    {
        MW_LOG_WARN(MQTTD, "MQTTD %p already exist.",ptr_mqttdmain);
        return MW_E_ALREADY_INITED;
    }
    MW_LOG_DEBUG(MQTTD, "Create the MQTTD task.");

    /* mqttd control structure initialize */
    _mqttd_ctrl_init(&mqttd, (ip_addr_t *)arg);

    /* mqttd internal DB queue */
    rc = mqttd_queue_init();
    if (MW_E_OK != rc)
    {
        return MW_E_NOT_INITED;
    }

    /* mqttd remain message mutex */
    rc = osapi_mutexCreate(
            MQTTD_TASK_NAME,
            &ptr_mqttmutex);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Failed to create remain message mutex");
        mqttd_queue_free();
        return MW_E_NOT_INITED;
    }
    MW_LOG_DEBUG(MQTTD, "Create the remain msg mutex %p",ptr_mqttmutex);

    /* Create timer */
    osapi_timerCreate(
            MQTTD_TIMER_NAME,
            _mqttd_tmr,
            TRUE,
            MQTTD_TIMER_PERIOD,
            NULL,
            &ptr_mqttd_time);
    if(NULL == ptr_mqttd_time)
    {
        MW_LOG_ERROR(MQTTD, "Failed to create MQTTD timer.");
        mqttd_queue_free();
        osapi_mutexDelete(ptr_mqttmutex);
        return MW_E_NOT_INITED;
    }

    /* mqttd main process */
    rc = osapi_processCreate(
            MQTTD_TASK_NAME,
            MQTTD_STACK_SIZE,
            MQTTD_THREAD_PRI,
            _mqttd_main,
            (void *)&ptr_mqttdmain,
            &ptr_mqttdmain);

    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(MQTTD, "Delete the remain message mutex and process mutex due to process create failed");
        mqttd_queue_free();
        osapi_mutexDelete(ptr_mqttmutex);
        osapi_timerDelete(ptr_mqttd_time);
        return MW_E_NOT_INITED;
    }
    mqttd_enable = TRUE;

    MW_LOG_INFO(MQTTD, "\nMQTTD create the process: %p",ptr_mqttdmain);
    return MW_E_OK;
}

/* FUNCTION NAME: _mqttd_deinit
 * PURPOSE:
 *      Deinitialize MQTT client daemon
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
MW_ERROR_NO_T _mqttd_deinit(void)
{
    mqttd_enable = FALSE;
    if (ptr_mqttdmain == NULL)
    {
        return MW_E_OK;
    }

    mqttdapi_tcpip_call(_mqttd_client_disconnect_callback);
    mqttdapi_tcpip_call(_mqttd_ctrl_free_callback);

    if (NULL != ptr_mqttd_time)
    {
        MW_LOG_DEBUG(MQTTD, "MQTTD free the timer.");
        if(MW_E_OK == osapi_timerActive(ptr_mqttd_time))
        {
            osapi_timerStop(ptr_mqttd_time);
        }
        osapi_timerDelete(ptr_mqttd_time);
        ptr_mqttd_time = NULL;
    }

    if (NULL != ptr_mqttmutex)
    {
        MW_LOG_DEBUG(MQTTD, "MQTTD free the remain message semaphore.");
        osapi_mutexDelete(ptr_mqttmutex);
        ptr_mqttmutex = NULL;
    }

    mqttd_queue_free();

    ptr_mqttdmain = NULL;
    vTaskDelete(NULL);

    return MW_E_OK;
}

/* FUNCTION NAME: mqttd_dump_topic
 * PURPOSE:
 *      Dump all mqtt supported topics
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
 *      For debug using only
 */
void mqttd_dump_topic(void)
{
    C8_T *ptr_topic = NULL;
    UI8_T t_idx = 0, f_idx = 0, f_count;
    UI16_T e_idx = 0, length;

    MW_CMD_OUTPUT("\nMQTTD supported DB Topics:\n");
    osapi_calloc(MQTTD_MAX_TOPIC_SIZE, MQTTD_TASK_NAME, (void **)&ptr_topic);
    if (NULL == ptr_topic)
    {
        MW_CMD_OUTPUT("Internal Error! No memory.\n");
        return;
    }
    while (t_idx < TABLES_LAST)
    {
        if (MW_E_OK == dbapi_getFieldsNum(t_idx, &f_count))
        {
            for (f_idx = 0; f_idx < f_count; f_idx++)
            {
                length = _mqttd_db_topic_set(&mqttd, M_CREATE, t_idx, f_idx, e_idx, ptr_topic, MQTTD_MAX_TOPIC_SIZE);
                MW_CMD_OUTPUT("(%-3u/%-3u/%-5u): %-42s (len= %2d).\n", t_idx, f_idx, e_idx, ptr_topic, length);
            }
        }
        t_idx++;
    }
    MW_FREE(ptr_topic);
}

/* FUNCTION NAME: mqttd_show_state
 * PURPOSE:
 *      To show the mqttd status
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
void mqttd_show_state(void)
{
    if (MQTTD_STATE_RUN > mqttd.state)
    {
        MW_CMD_OUTPUT("\nMQTTD state 	  : Connecting(%d)\n", mqttd.state);
        return;
    }
    else if (MQTTD_STATE_RUN < mqttd.state)
    {
        MW_CMD_OUTPUT("\nMQTTD state 	  : Stopped(%d)\n", mqttd.state);
        return;
    }
    else
    {
        MW_CMD_OUTPUT("\nMQTTD state 	  : Running(%d)\n", mqttd.state);
    }
    MW_CMD_OUTPUT("MQTT remote server: %s\n", ipaddr_ntoa(&mqttd.server_ip));
    MW_CMD_OUTPUT("MQTTD Client ID   : %s\n", mqttd.client_id);
    MW_CMD_OUTPUT("MQTTD cloud ID    : %d\n", mqttd.cldb_id);
}

/* FUNCTION NAME: mqttd_shutdown
 * PURPOSE:
 *      To shutdown the MQTTD
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
void mqttd_shutdown(void)
{
    if (ptr_mqttdmain == NULL)
    {
        MW_LOG_DEBUG(MQTTD, "MQTTD stopped.");
        if (MQTTD_STATE_SHUTDOWN != mqttd.state)
        {
            mqttd.state = MQTTD_STATE_SHUTDOWN;
        }
        return;
    }
    MW_LOG_DEBUG(MQTTD, "Shutdown the MQTTD task.");

    mqttd.state = MQTTD_STATE_SHUTDOWN;
    mqttd.reconnect = FALSE;
    mqttd_enable = FALSE;
}

/* FUNCTION NAME: mqttd_get_state
 * PURPOSE:
 *      To get the MQTTD status
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      BOOL_T
 *
 * NOTES:
 *      None
 */
UI8_T
mqttd_get_state(
    void)
{
    return mqttd_enable;
}

/* FUNCTION NAME: mqttd_is_subscribed
 * PURPOSE:
 *      To check is data change subscribed by MQTTD
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      BOOL_T
 *
 * NOTES:
 *      None
 */
BOOL_T
mqttd_is_subscribed(
    UI8_T t_idx,
    UI8_T f_idx,
    UI16_T e_idx)
{

#ifdef AIR_LITE_MW
    if ((t_idx != MIB_CNT)
        && (mqttd_get_state()) && (MQTTD_STATE_RUN == mqttd.state))
#else
    if (mqttd_get_state() && (MQTTD_STATE_RUN == mqttd.state))
#endif
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
