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

/* FILE NAME:  lldp.c
 * PURPOSE:
 * It provides LLDP module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "inc/lldp.h"
#include "air_stag.h"
#ifdef AIR_SUPPORT_LP
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
#include "lp.h"
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
#endif
#include "air_swc.h"
#include "air_vlan.h"
#include "ethernet.h"
#include "mw_log.h"
#include "mw_cmd_util.h"

lldp_cfg_info_t          lldp_info = {0};
threadhandle_t           _ptr_lldp = NULL;
NET_FILTER_T             *lldp_pkt_netf = NULL;
TimerHandle_t            lldp_timer_handle = NULL;
UI8_T                    lldp_notify_msg_count = 0;
UI8_T                    port_status_down_count[MAX_PORT_NUM] = {0};
UI16_T                   lldp_global_tx_count = 0;
UI8_T                    port_send_shutdown_msg[MAX_PORT_NUM] = {0};
UI16_T                   port_info_change_tx_count[MAX_PORT_NUM] = {0};
UI16_T                   port_msap_rx_count[MAX_PORT_NUM][MAX_LLDP_CLIENT_PER_PORT];
UI32_T                   port_tx_timestamp[MAX_PORT_NUM] = {0};
lldp_rx_info_t           *port_msap_rx[MAX_PORT_NUM][MAX_LLDP_CLIENT_PER_PORT];
UI8_T                    lldp_global_cfg = 0;
UI8_T                    lldp_neibor_index[MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT] = {0};
static UI8_T             lldp_debug_level = 0;
static msghandle_t       lldp_q_handle;
static LLDP_QUEUE_CONTEXT_T lldp_queue_ctx;
static StackType_t       lldp_task_stack[LLDP_STACK_SIZE] = {0};
static StaticTask_t      lldp_task_tcb = {0};

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
static UI8_T             lldp_per_sys_per_mac = 0;
static UI8_T             lldp_not_send_blocking = 0;
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */


/* 32-bit integer manipulation macros (little endian) */
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ]       )        \
        | ( (unsigned long) (b)[(i) + 1] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 2] << 16 )        \
        | ( (unsigned long) (b)[(i) + 3] << 24 );       \
}
#endif

#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n)       );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 24 );       \
}
#endif

/* MD5 context setup */
void md5_starts( md5_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void md5_process( md5_context *ctx, const unsigned char data[64] )
{
    unsigned long X[16], A, B, C, D;
    GET_ULONG_LE( X[ 0], data,  0 );
    GET_ULONG_LE( X[ 1], data,  4 );
    GET_ULONG_LE( X[ 2], data,  8 );
    GET_ULONG_LE( X[ 3], data, 12 );
    GET_ULONG_LE( X[ 4], data, 16 );
    GET_ULONG_LE( X[ 5], data, 20 );
    GET_ULONG_LE( X[ 6], data, 24 );
    GET_ULONG_LE( X[ 7], data, 28 );
    GET_ULONG_LE( X[ 8], data, 32 );
    GET_ULONG_LE( X[ 9], data, 36 );
    GET_ULONG_LE( X[10], data, 40 );
    GET_ULONG_LE( X[11], data, 44 );
    GET_ULONG_LE( X[12], data, 48 );
    GET_ULONG_LE( X[13], data, 52 );
    GET_ULONG_LE( X[14], data, 56 );
    GET_ULONG_LE( X[15], data, 60 );
#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                                \
{                                                       \
    a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}
    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

    P( A, B, C, D,  0,  7, 0xD76AA478 );
    P( D, A, B, C,  1, 12, 0xE8C7B756 );
    P( C, D, A, B,  2, 17, 0x242070DB );
    P( B, C, D, A,  3, 22, 0xC1BDCEEE );
    P( A, B, C, D,  4,  7, 0xF57C0FAF );
    P( D, A, B, C,  5, 12, 0x4787C62A );
    P( C, D, A, B,  6, 17, 0xA8304613 );
    P( B, C, D, A,  7, 22, 0xFD469501 );
    P( A, B, C, D,  8,  7, 0x698098D8 );
    P( D, A, B, C,  9, 12, 0x8B44F7AF );
    P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
    P( B, C, D, A, 11, 22, 0x895CD7BE );
    P( A, B, C, D, 12,  7, 0x6B901122 );
    P( D, A, B, C, 13, 12, 0xFD987193 );
    P( C, D, A, B, 14, 17, 0xA679438E );
    P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

    P( A, B, C, D,  1,  5, 0xF61E2562 );
    P( D, A, B, C,  6,  9, 0xC040B340 );
    P( C, D, A, B, 11, 14, 0x265E5A51 );
    P( B, C, D, A,  0, 20, 0xE9B6C7AA );
    P( A, B, C, D,  5,  5, 0xD62F105D );
    P( D, A, B, C, 10,  9, 0x02441453 );
    P( C, D, A, B, 15, 14, 0xD8A1E681 );
    P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
    P( A, B, C, D,  9,  5, 0x21E1CDE6 );
    P( D, A, B, C, 14,  9, 0xC33707D6 );
    P( C, D, A, B,  3, 14, 0xF4D50D87 );
    P( B, C, D, A,  8, 20, 0x455A14ED );
    P( A, B, C, D, 13,  5, 0xA9E3E905 );
    P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
    P( C, D, A, B,  7, 14, 0x676F02D9 );
    P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F

#define F(x,y,z) (x ^ y ^ z)

    P( A, B, C, D,  5,  4, 0xFFFA3942 );
    P( D, A, B, C,  8, 11, 0x8771F681 );
    P( C, D, A, B, 11, 16, 0x6D9D6122 );
    P( B, C, D, A, 14, 23, 0xFDE5380C );
    P( A, B, C, D,  1,  4, 0xA4BEEA44 );
    P( D, A, B, C,  4, 11, 0x4BDECFA9 );
    P( C, D, A, B,  7, 16, 0xF6BB4B60 );
    P( B, C, D, A, 10, 23, 0xBEBFBC70 );
    P( A, B, C, D, 13,  4, 0x289B7EC6 );
    P( D, A, B, C,  0, 11, 0xEAA127FA );
    P( C, D, A, B,  3, 16, 0xD4EF3085 );
    P( B, C, D, A,  6, 23, 0x04881D05 );
    P( A, B, C, D,  9,  4, 0xD9D4D039 );
    P( D, A, B, C, 12, 11, 0xE6DB99E5 );
    P( C, D, A, B, 15, 16, 0x1FA27CF8 );
    P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

    P( A, B, C, D,  0,  6, 0xF4292244 );
    P( D, A, B, C,  7, 10, 0x432AFF97 );
    P( C, D, A, B, 14, 15, 0xAB9423A7 );
    P( B, C, D, A,  5, 21, 0xFC93A039 );
    P( A, B, C, D, 12,  6, 0x655B59C3 );
    P( D, A, B, C,  3, 10, 0x8F0CCC92 );
    P( C, D, A, B, 10, 15, 0xFFEFF47D );
    P( B, C, D, A,  1, 21, 0x85845DD1 );
    P( A, B, C, D,  8,  6, 0x6FA87E4F );
    P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
    P( C, D, A, B,  6, 15, 0xA3014314 );
    P( B, C, D, A, 13, 21, 0x4E0811A1 );
    P( A, B, C, D,  4,  6, 0xF7537E82 );
    P( D, A, B, C, 11, 10, 0xBD3AF235 );
    P( C, D, A, B,  2, 15, 0x2AD7D2BB );
    P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F
    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

/* MD5 process buffer */
void md5_update( md5_context *ctx, const unsigned char *input, int ilen )
{
    int fill;
    unsigned long left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;
    ctx->total[0] += ilen;
    ctx->total[0] &= 0xFFFFFFFF;
    if( ctx->total[0] < (unsigned long) ilen )
        ctx->total[1]++;
    if( left && ilen >= fill )
    {
        osapi_memcpy( (void *) (ctx->buffer + left),
                input, fill );
        md5_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }
    while( ilen >= 64 )
    {
        md5_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }
    if( ilen > 0 )
    {
        osapi_memcpy( (void *) (ctx->buffer + left),
                input, ilen );
    }
}

static const unsigned char md5_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* MD5 final digest */
void md5_finish( md5_context *ctx, unsigned char output[16] )
{
    unsigned long last, padn;
    unsigned long high, low;
    unsigned char msglen[8];
    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_LE( low,  msglen, 0 );
    PUT_ULONG_LE( high, msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );
    md5_update( ctx, md5_padding, padn );
    md5_update( ctx, msglen, 8 );

    PUT_ULONG_LE( ctx->state[0], output,  0 );
    PUT_ULONG_LE( ctx->state[1], output,  4 );
    PUT_ULONG_LE( ctx->state[2], output,  8 );
    PUT_ULONG_LE( ctx->state[3], output, 12 );
}

/* output = MD5( input buffer ) */
void md5( unsigned char *input, int ilen, unsigned char output[16] )
{
    md5_context ctx;
    md5_starts( &ctx );
    md5_update( &ctx, input, ilen );
    md5_finish( &ctx, output );
}

static MW_ERROR_NO_T
_lldp_queue_send(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    MW_ERROR_NO_T rc;
    MW_CHECK_PTR(ptr_msg);
    rc = dbapi_dbisReady();
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_LOG_ERROR(LLDP, "%s[%d]: db is not ready", __func__, __LINE__);
        MW_FREE(ptr_msg);
        return rc;
    }
    rc = dbapi_sendRequesttoDb(size, ptr_msg);
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_LOG_ERROR(LLDP, "%s[%d]: db_sendRequesttoDb() failed", __func__, __LINE__);
        MW_FREE(ptr_msg);
    }
    return rc;
}

MW_ERROR_NO_T
lldp_db_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T          msg_size;

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(
            msg_size,
            LLDP_MODULE_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: allocate memory failed(%d)", __func__, __LINE__, rc);
        return MW_E_NO_MEMORY;
    }
    /* message */
    osapi_strncpy(ptr_msg->cq_name, LLDP_QUEUE_NAME, DB_Q_NAME_SIZE);
    ptr_msg->method = method;
    ptr_msg->type.count = 1;
    MW_LOG_INFO(LLDP, "%s[%d]: ptr_msg=%p, cq_name=%s, method=0x%X, count=%u, size=%u", __func__, __LINE__,
                  ptr_msg, ptr_msg->cq_name, ptr_msg->method, ptr_msg->type.count, size);
    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    if (size > 0 && method != M_GET)
    {
        memcpy(&(ptr_payload->ptr_data), ptr_data, size);
    }
    MW_LOG_INFO(LLDP, "%s[%d]: ptr_payload=%p, t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u", __func__, __LINE__,
                  ptr_payload,
                  ptr_payload->request.t_idx,
                  ptr_payload->request.f_idx,
                  ptr_payload->request.e_idx,
                  ptr_payload->data_size);
    /* Send message to DB */
    rc = _lldp_queue_send(ptr_msg, msg_size);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: Send message to DB failed(%d)", __func__, __LINE__, rc);
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
lldp_cli_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T          msg_size;


    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(
            msg_size,
            LLDP_MODULE_NAME,
            (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: allocate memory failed(%d)", __func__, __LINE__, rc);
        return MW_E_NO_MEMORY;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: ptr_msg=%p", __func__, __LINE__, ptr_msg);
    /* message */
    osapi_strncpy(ptr_msg ->cq_name, LLDP_CLI_QUEUE_NAME, DB_Q_NAME_SIZE);
    ptr_msg ->method = method;
    ptr_msg ->type.count = 1;
    MW_LOG_INFO(LLDP, "%s[%d]: cq_name=%s", __func__, __LINE__, ptr_msg ->cq_name);
    MW_LOG_INFO(LLDP, "%s[%d]: method=0x%X", __func__, __LINE__, ptr_msg ->method);
    MW_LOG_INFO(LLDP, "%s[%d]: count=%u", __func__, __LINE__, ptr_msg ->type.count);
    MW_LOG_INFO(LLDP, "%s[%d]: size=%u", __func__, __LINE__, size);

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg ->ptr_payload);
    ptr_payload ->request.t_idx = t_idx;
    ptr_payload ->request.f_idx = f_idx;
    ptr_payload ->request.e_idx = e_idx;
    ptr_payload ->data_size = size;

    MW_LOG_INFO(LLDP, "%s[%d]: ptr_payload=%p", __func__, __LINE__, ptr_payload);
    MW_LOG_INFO(LLDP, "%s[%d]: t_idx=%u", __func__, __LINE__, ptr_payload ->request.t_idx);
    MW_LOG_INFO(LLDP, "%s[%d]: f_idx=%u", __func__, __LINE__, ptr_payload ->request.f_idx);
    MW_LOG_INFO(LLDP, "%s[%d]: e_idx=%u", __func__, __LINE__, ptr_payload ->request.e_idx);
    MW_LOG_INFO(LLDP, "%s[%d]: data_size=%u", __func__, __LINE__, ptr_payload ->data_size);

    /* Send message to DB */
    rc = _lldp_queue_send(ptr_msg, msg_size);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: Send message to DB failed(%d)", __func__, __LINE__, rc);
        MW_FREE(ptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    (*pptr_out_msg) = ptr_msg;
    return MW_E_OK;
}

static MW_ERROR_NO_T
lldp_cli_queue_recv(
    void **pptr_buf)
{
    MW_ERROR_NO_T rc;
    UI8_T *ptr_msg = NULL;

    rc = osapi_msgRecv(
        LLDP_CLI_QUEUE_NAME,
        &ptr_msg,
        0,
        LLDP_QUEUE_TIMEOUT);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: msgRecv failed(%d)", __func__, __LINE__, rc);
        return rc;
    }

    MW_LOG_DEBUG(LLDP, "%s[%d]: ptr_msg=%p", __func__, __LINE__, ptr_msg);
    (*pptr_buf) = ptr_msg;

    return MW_E_OK;
}

static MW_ERROR_NO_T
lldp_cli_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;
    UI16_T           total_size = 0;
    DB_PAYLOAD_T    *ptr_pload = NULL;

    DB_REQUEST_TYPE_T request = {
        .t_idx = in_t_idx,
        .f_idx = in_f_idx,
        .e_idx = in_e_idx
    };

    rc = dbapi_getDataSize(request, &total_size);
    if (MW_E_OK != rc)
    {
       MW_LOG_ERROR(LLDP, "%s[%d]: dbapi_getDataSize failed(%d)\n", __func__, __LINE__, rc);
       return rc;
    }
    MW_LOG_INFO(LLDP, "%s[%d]: dbapi_getDataSize sucess, total_size = %d \n", __func__, __LINE__, total_size);

    rc = lldp_cli_queue_send(M_GET, in_t_idx, in_f_idx, in_e_idx, NULL, total_size, &ptr_msg);
    if (MW_E_OK != rc)
    {
       MW_LOG_ERROR(LLDP, "%s[%d]: lldp_cli_queue_send failed(%d)\n", __func__, __LINE__, rc);
       return rc;
    }

    /* wait for DB response messgae */
    rc = lldp_cli_queue_recv((void **)&ptr_msg);
    if(MW_E_OK == rc)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: lldp_cli_queue_recv success \n", __func__, __LINE__);
    }
    else
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: lldp_cli_queue_recv failed(%d) \n", __func__, __LINE__, rc);
        MW_FREE(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    (*ptr_out_size) = total_size;

    ptr_pload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    (*pptr_out_data) = &(ptr_pload->ptr_data);

    MW_LOG_INFO(LLDP, "%s[%d]: *pptr_out_msg = %p, *pptr_out_data = %p \n", __func__, __LINE__, *pptr_out_msg, *pptr_out_data);

    return MW_E_OK;
}

MW_ERROR_NO_T
lldp_timer_send_to_queue(
   MW_MSG_T      task_msg,
   UI32_T        wait_time)
{
    MW_MSG_T *ptr_msg = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;

    osapi_calloc(sizeof(MW_MSG_T), LLDP_MODULE_NAME, (void**)&ptr_msg);
    if(NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    ptr_msg->msg_id = task_msg.msg_id;
    ret = osapi_msgSend(LLDP_QUEUE_NAME, (UI8_T *)ptr_msg, 0, wait_time);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
    }
    MW_LOG_INFO(LLDP, "%s[%d]: msg type %d, msgSend ret %d\n", __func__, __LINE__, task_msg.msg_id, ret);
    return ret;
}

static MW_ERROR_NO_T
lldp_cli_queue_init(void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* Create message */
    rc = osapi_msgCreateStatic(LLDP_CLI_QUEUE_NAME,
                               LLDP_CLI_QUEUE_LENGTH,
                               sizeof(MW_MSG_T *),
                               (UI8_T *)lldp_queue_ctx.cliQueueStorage,
                               &lldp_queue_ctx.cli_queue_handle);
    if (MW_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: msgCreate failed(%d) \n", __func__, __LINE__, rc);
        return MW_E_NOT_INITED;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
lldp_free_resource(void)
{
    if (NULL != lldp_pkt_netf)
    {
        lldp_pkt_netf->state = NET_FILTER_DEREGISTER;
        if (MW_E_OK != osapi_netRegister(lldp_pkt_netf))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: osapi_netRegister deregister failed", __func__, __LINE__);
        }
        MW_FREE(lldp_pkt_netf);
    }
    if (NULL != _ptr_lldp)
    {
        osapi_threadDelete(_ptr_lldp);
    }
    if (NULL != lldp_q_handle)
    {
        lldp_q_handle = NULL;
    }
    if (NULL != lldp_timer_handle)
    {
        if (osapi_timerDelete(lldp_timer_handle) != MW_E_OK)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: osapi_timerDelete lldp Tmr failed !", __func__, __LINE__);
        }
    }
    if (osapi_msgDelete(LLDP_QUEUE_NAME) != MW_E_OK)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_msgDelete for %s failed", __func__, __LINE__, LLDP_QUEUE_NAME);
    }
    if (osapi_msgDelete(LLDP_CLI_QUEUE_NAME) != MW_E_OK)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_msgDelete for %s failed", __func__, __LINE__, LLDP_CLI_QUEUE_NAME);
    }
    return MW_E_OK;
}

static void
lldp_global_var_init(void)
{
    UI8_T i = 0;
    UI8_T j = 0;
    I32_T ret = 0;
    lldp_debug_level = LLDP_DEBUG_LEVEL_ERROR;
    lldp_info.global_enable = DISABLE;
    lldp_info.tx_hold = 4;
    lldp_info.tx_interval = 30;
    lldp_info.reinit_delay = 2;
    lldp_info.tx_delay = 2;
    lldp_info.voice_vlan_state = 0;
    lldp_info.voice_vlan_id = 0;
    lldp_info.voice_vlan_pri = 0;
    lldp_info.survei_vlan_state = 0;
    lldp_info.survei_vlan_id = 0;
    lldp_info.survei_vlan_pri = 0;
    lldp_global_tx_count = lldp_info.tx_interval;
    lldp_global_cfg = 0;
    ret = osapi_snprintf(lldp_info.sys_name, 7, "EN8851");
    if(ret < 0)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_snprintf error", __func__, __LINE__);
    }
    lldp_info.sys_name[6] = '\0';
    for(i = 0; i < MAX_PORT_NUM; i++)
    {
        lldp_info.lldp_admin[i] = LLDP_PORT_DISABLE;
        lldp_info.port_admin[i] = 1;
        lldp_info.oper_status[i] = 0;
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
        lldp_info.rstp_status[i] = 0;
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
        for(j = 0; j < MAX_LLDP_CLIENT_PER_PORT; j++)
        {
            port_msap_rx_count[i][j] = 0;
            port_msap_rx[i][j] = NULL;
        }
    }
    for(i = 0; i < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; i++)
    {
        lldp_neibor_index[i] = 0;
    }
}

static void
lldp_mgmt_attr_init(void)
{
    UI32_T unit = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;
#ifdef AIR_EN_CORAL
    UI16_T port = 0;
    AIR_SWC_MGMT_FRAME_CFG_T port_mgmt_cfg = {0};
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        osapi_memset(&port_mgmt_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        port_mgmt_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
        rc = air_swc_getPortMgmtFrameCfg(unit, port, &port_mgmt_cfg);
        if (AIR_E_OK != rc)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: get port(%d) port_mgmt_cfg failed(%d)", __func__, __LINE__, port, rc);
        }
        port_mgmt_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        rc = air_swc_setPortMgmtFrameCfg(unit, port, &port_mgmt_cfg);
        if (AIR_E_OK != rc)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: set port(%d) port_mgmt_cfg failed(%d)", __func__, __LINE__, port, rc);
            return;
        }
    }
#else
    AIR_SWC_MGMT_FRAME_CFG_T frame03_cfg = {0};
    AIR_SWC_MGMT_FRAME_CFG_T frame0E_cfg = {0};
    frame03_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_03;
    frame0E_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_0E;
    rc = air_swc_getMgmtFrameCfg(unit, &frame03_cfg);
    if(AIR_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: get REV_03 mgmt cfg failed(%d)", __func__, __LINE__, rc);
    }
    frame03_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
    rc = air_swc_setMgmtFrameCfg(unit, &frame03_cfg);
    if(AIR_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: set REV_03 mgmt cfg as_bpdu failed(%d)", __func__, __LINE__, rc);
        return;
    }
    rc = air_swc_getMgmtFrameCfg(unit, &frame0E_cfg);
    if(AIR_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: get REV_0E mgmt cfg failed(%d)", __func__, __LINE__, rc);
    }
    frame0E_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
    rc = air_swc_setMgmtFrameCfg(unit, &frame0E_cfg);
    if(AIR_E_OK != rc)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: set REV_0E mgmt cfg as_bpdu failed(%d)", __func__, __LINE__, rc);
        return;
    }
#endif
}

#ifndef AIR_EN_CORAL
UI8_T
lldp_get_global_en(void)
{
    return lldp_info.global_enable;
}
#endif
/* FUNCTION NAME:   lldp_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg lldp glo info.
 *
 * INPUT:
 *      type        --  0-5
                        0:lldp glo enable
                        1:lldp tx interval
                        2:lldp tx hold
                        3:lldp reinit delay
                        4:lldp tx delay
                        5:lldp debug level
                        6:lldp per sys per_mac
                        7:lldp not send blocking
 *      value       --  value
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
lldp_cmd_set(
    UI32_T        type,
    UI32_T        value)
{
    /* lldp_glo_cfg type={ 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7} value={value} */
    if(type == 0)
    {
        UI8_T enable = (UI8_T)value;
        if((enable != TRUE) && (enable != FALSE))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: enable is not 1 and 0, command is error, please check", __func__, __LINE__);
            return;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_INFO, GLOBAL_ENABLE, DB_ALL_ENTRIES, enable=%d)", __func__, __LINE__, enable);
        lldp_db_queue_send(M_UPDATE, LLDP_INFO, GLOBAL_ENABLE, DB_ALL_ENTRIES, &enable, sizeof(enable));
        lldp_info.global_enable = enable;
        lldp_global_cfg = enable;
    }
    else if(type == 1)
    {
        UI16_T tx_interval = (UI16_T)value;
        if((tx_interval < LLDP_TX_INTERVAL_MIN) || (tx_interval > LLDP_TX_INTERVAL_MAX))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: tx_interval is not between 5 and 32768, command is error, please check", __func__, __LINE__);
            return;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_INTERVAL, DB_ALL_ENTRIES, tx_interval=%d)", __func__, __LINE__, tx_interval);
        lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_INTERVAL, DB_ALL_ENTRIES, &tx_interval, sizeof(tx_interval));
        lldp_info.tx_interval = tx_interval;
        lldp_global_tx_count = lldp_info.tx_interval;
    }
    else if(type == 2)
    {
        UI8_T tx_hold = (UI8_T)value;
        if((tx_hold < LLDP_TX_HOLD_MIN) || (tx_hold > LLDP_TX_HOLD_MAX))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: tx_hold is not between 2 and 10, command is error, please check", __func__, __LINE__);
            return;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_HOLD, DB_ALL_ENTRIES, tx_hold=%d)", __func__, __LINE__, tx_hold);
        lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_HOLD, DB_ALL_ENTRIES, &tx_hold, sizeof(tx_hold));
        lldp_info.tx_hold  = tx_hold;
    }
    else if(type == 3)
    {
        UI16_T reinit_delay = (UI16_T)value;
        if((reinit_delay < LLDP_REINIT_DELAY_MIN) || (reinit_delay > LLDP_REINIT_DELAY_MAX))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: reinit_delay is not between 1 and 10, command is error, please check", __func__, __LINE__);
            return;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_INFO, REINIT_DELAY, DB_ALL_ENTRIES, reinit_delay=%d)", __func__, __LINE__, reinit_delay);
        lldp_db_queue_send(M_UPDATE, LLDP_INFO, REINIT_DELAY, DB_ALL_ENTRIES, &reinit_delay, sizeof(reinit_delay));
        lldp_info.reinit_delay = reinit_delay;
    }
    else if(type == 4)
    {
        UI16_T tx_delay = (UI16_T)value;
        if((tx_delay < LLDP_TX_DELAY_MIN) || (tx_delay > LLDP_TX_DELAY_MAX))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: tx_delay is not between 1 and 8192, command is error, please check", __func__, __LINE__);
            return;
        }
        if(tx_delay > (lldp_info.tx_interval * 0.25))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: tx_delay is bigger than tx_interval * 0.25(%d), command is error, please check", __func__, __LINE__, lldp_info.tx_interval/4);
            return;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_DELAY, DB_ALL_ENTRIES, tx_delay=%d)", __func__, __LINE__, tx_delay);
        lldp_db_queue_send(M_UPDATE, LLDP_INFO, TX_DELAY, DB_ALL_ENTRIES, &tx_delay, sizeof(tx_delay));
        lldp_info.tx_delay = tx_delay;
    }
    else if(type == 5)
    {
        UI8_T debug_level  = (UI8_T)value;
        if(debug_level > LLDP_DEBUG_LEVEL_LAST)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: debug_level is bigger than 5, command is error, please check", __func__, __LINE__);
            return;
        }
        lldp_debug_level = value;
    }
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    else if(type == 6)
    {
        UI8_T enable = (UI8_T)value;
        if((enable != TRUE) && (enable != FALSE))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: enable is not 1 and 0, command is error, please check", __func__, __LINE__);
            return;
        }
        lldp_per_sys_per_mac = enable;
    }
    else if(type == 7)
    {
        UI8_T enable = (UI8_T)value;
        if((enable != TRUE) && (enable != FALSE))
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: enable is not 1 and 0, command is error, please check", __func__, __LINE__);
            return;
        }
        lldp_not_send_blocking = enable;
    }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
    else
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: type %d is not supported in this FW!", __func__, __LINE__, type);
        return;
    }

    return;
}

/* FUNCTION NAME:   lldp_port_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg lldp port info.
 *
 * INPUT:
 *      port        --  0-28
 *      mode       --   0:disable;1:tx only;2:rx only;3:tx & rx
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
lldp_port_cmd_set(
    UI32_T        port_num,
    UI32_T        mode)
{
    UI8_T port = (UI8_T)port_num;
    UI8_T port_admin = (UI8_T)mode;
    if(port > 28)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port is not between 0 and 28, command is error, please check", __func__, __LINE__);
        return;
    }
    if(port_admin > LLDP_PORT_TX_RX)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port_admin is bigger than 3, command is error, please check", __func__, __LINE__);
        return;
    }
    if(port == 0)
    {
        UI8_T tmp_port_admin[MAX_PORT_NUM];
        UI8_T i = 0;
        for(; i < PLAT_MAX_PORT_NUM; i++)
        {
            tmp_port_admin[i] = port_admin;
            lldp_info.lldp_admin[i] = port_admin;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, admin=%d)", __func__, __LINE__, port_admin);
        lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, tmp_port_admin, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    }
    else
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, ENTRY(%d), admin=%d)", __func__, __LINE__, port, port_admin);
        lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, port, &port_admin, sizeof(port_admin));
        lldp_info.lldp_admin[port-1] = port_admin;
    }
    return;
}
#ifdef AIR_SUPPORT_VOICE_VLAN
/* FUNCTION NAME:   lldp_voice_cfg_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg voice info.
 *
 * INPUT:
 *      state        --   0-1(0:voice vlan disable;1:voice vlan enable)
 *      vlanid       --   1-4094
 *      vlanpri        -  0-7
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
lldp_voice_cfg_set(
    UI32_T        state,
    UI32_T        vlanid,
    UI32_T        vlanpri)
{
    DB_VOICE_VLAN_INFO_T tmp_cfg = {0};
    tmp_cfg.vlan_state = (UI8_T)state;
    tmp_cfg.vlan_id = (UI16_T)vlanid;
    tmp_cfg.vlan_priority = (UI8_T)vlanpri;
    if((tmp_cfg.vlan_state != TRUE) && (tmp_cfg.vlan_state != FALSE))
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: voice vlan state is not 0 & 1, command is error, please check", __func__, __LINE__);
        return;
    }
    if((tmp_cfg.vlan_id < 1) || (tmp_cfg.vlan_id > 4094))
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: voice vlan id is not between 1-4094, command is error, please check", __func__, __LINE__);
        return;
    }
    if(tmp_cfg.vlan_priority > 7)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: voice vlan pri is bigger than 7, command is error, please check", __func__, __LINE__);
        return;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, VOICE_VLAN_INFO, DB_ALL_FIELDS, state(%d) vlanid(%d) vlanpri(%d)", __func__, __LINE__, \
                                      tmp_cfg.vlan_state, tmp_cfg.vlan_id, tmp_cfg.vlan_priority);
    lldp_db_queue_send(M_UPDATE, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &tmp_cfg, sizeof(tmp_cfg));
    lldp_info.voice_vlan_state = tmp_cfg.vlan_state;
    lldp_info.voice_vlan_id = tmp_cfg.vlan_id;
    lldp_info.voice_vlan_pri = tmp_cfg.vlan_priority;
    return;
}
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/* FUNCTION NAME:   lldp_survei_cfg_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg survei info.
 *
 * INPUT:
 *      state        --   0-1(0:survei vlan disable;1:survei vlan enable)
 *      vlanid       --   1-4094
 *      vlanpri        -  0-7
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
lldp_survei_cfg_set(
    UI32_T        state,
    UI32_T        vlanid,
    UI32_T        vlanpri)
{
    DB_SURVEI_VLAN_INFO_T tmp_cfg = {0};
    tmp_cfg.vlan_state = (UI8_T)state;
    tmp_cfg.vlan_id = (UI16_T)vlanid;
    tmp_cfg.vlan_priority = (UI8_T)vlanpri;
    if((tmp_cfg.vlan_state != TRUE) && (tmp_cfg.vlan_state != FALSE))
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: survei vlan state is not 0 & 1, command is error, please check", __func__, __LINE__);
        return;
    }
    if((tmp_cfg.vlan_id < 1) || (tmp_cfg.vlan_id > 4094))
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: survei vlan id is not between 1-4094, command is error, please check", __func__, __LINE__);
        return;
    }
    if(tmp_cfg.vlan_priority > 7)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: survei vlan pri is bigger than 7, command is error, please check", __func__, __LINE__);
        return;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, SURVEI_VLAN_INFO, DB_ALL_FIELDS, state(%d) vlanid(%d) vlanpri(%d)", __func__, __LINE__, \
                                      tmp_cfg.vlan_state, tmp_cfg.vlan_id, tmp_cfg.vlan_priority);
    lldp_db_queue_send(M_UPDATE, SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &tmp_cfg, sizeof(tmp_cfg));
    lldp_info.survei_vlan_state = tmp_cfg.vlan_state;
    lldp_info.survei_vlan_id = tmp_cfg.vlan_id;
    lldp_info.survei_vlan_pri = tmp_cfg.vlan_priority;
    return;
}
#endif

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
#ifdef AIR_SUPPORT_RSTP
/* FUNCTION NAME:   lldp_rstp_port_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to cfg rstp port state.
 *
 * INPUT:
 *      port_num    --  0-28
 *      state       --  0:disable;1:learning;2:forwarding;3:discarding
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
lldp_rstp_port_cmd_set(
    UI32_T port_num,
    UI32_T state)
{
    UI8_T port = (UI8_T)port_num;
    UI8_T port_state = (UI8_T)state;
    if(port > 28)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port is not between 0 and 28, command is error, please check", __func__, __LINE__);
        return;
    }
    if(port_state > 0x3)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port_state is bigger than 3, command is error, please check", __func__, __LINE__);
        return;
    }
    if(port == 0)
    {
        UI8_T tmp_port_state[MAX_PORT_NUM];
        UI8_T i = 0;
        for(; i < PLAT_MAX_PORT_NUM; i++)
        {
            tmp_port_state[i] = port_state;
            lldp_info.rstp_status[i] = port_state;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, STP_PORT_OPER_INFO, RSTP_PORT_OPER_STATE, DB_ALL_ENTRIES, state=%d)", __func__, __LINE__, port_state);
        lldp_db_queue_send(M_UPDATE, STP_PORT_OPER_INFO, STP_PORT_OPER_STATE, DB_ALL_ENTRIES, tmp_port_state, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    }
    else
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, STP_PORT_OPER_INFO, RSTP_PORT_OPER_STATE, ENTRY(%d), state=%d)", __func__, __LINE__, port, port_state);
        lldp_db_queue_send(M_UPDATE, STP_PORT_OPER_INFO, STP_PORT_OPER_STATE, port, &port_state, sizeof(port_state));
        lldp_info.rstp_status[port-1] = port_state;
    }
}
#endif
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */

static void
lldp_print_client_info(UI8_T * ptr)
{
    I8_T i = 0;
    UI8_T tmp_len = 0;
    UI16_T tmp_value = 0;
    UI32_T tmp_value2 = 0;
    MW_CMD_OUTPUT("chassis id is");
    for(i = (MAX_LLDP_STR_SIZE - 2); i >= 0; i--)
    {
        if((((DB_LLDP_CLIENT_INFO_T*)ptr)->chassis_id[i] & 0xff) != 0x00)
        {
            tmp_len = i + 1;
            break;
        }
    }
    for(i = 0; i < tmp_len; i++)
    {
        MW_CMD_OUTPUT(" ");
        MW_CMD_OUTPUT("%x", ((DB_LLDP_CLIENT_INFO_T*)ptr)->chassis_id[i] & 0xff);
    }
    MW_CMD_OUTPUT("\n");
    MW_CMD_OUTPUT("port id is");
    for(i = (MAX_LLDP_STR_SIZE - 2); i >= 0; i--)
    {
        if((((DB_LLDP_CLIENT_INFO_T*)ptr)->port_id[i] & 0xff) != 0x00)
        {
            tmp_len = i + 1;
            break;
        }
    }
    for(i = 0; i < tmp_len; i++)
    {
        MW_CMD_OUTPUT(" ");
        MW_CMD_OUTPUT("%x", ((DB_LLDP_CLIENT_INFO_T*)ptr)->port_id[i] & 0xff);
    }
    MW_CMD_OUTPUT("\n");
    MW_CMD_OUTPUT("ttl is %d(s)\n", ((DB_LLDP_CLIENT_INFO_T*)ptr)->time_to_live);
    tmp_value = osapi_strlen(((DB_LLDP_CLIENT_INFO_T*)ptr)->system_name);
    if(tmp_value != 0)
    {
        MW_CMD_OUTPUT("system name is ");
        for(i = 0; i < tmp_value; i++)
        {
            MW_CMD_OUTPUT("%c", ((DB_LLDP_CLIENT_INFO_T*)ptr)->system_name[i]);
        }
        MW_CMD_OUTPUT("\n");
    }
    MW_CMD_OUTPUT("med capabilities is:\n");
    tmp_value = (((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0001;
    MW_CMD_OUTPUT("...............%d LLDP-MED Capabilities: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0002) >> 1;
    MW_CMD_OUTPUT("..............%d. Network Policy: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0004) >> 2;
    MW_CMD_OUTPUT(".............%d.. Location Identification: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0008) >> 3;
    MW_CMD_OUTPUT("............%d... Extended Pow via MDI-PSE: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0010) >> 4;
    MW_CMD_OUTPUT("...........%d.... Extended Pow via MDI-PD: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_capability) & 0x0020) >> 5;
    MW_CMD_OUTPUT("..........%d..... Inventory: %s\n", tmp_value, (tmp_value == 1) ? "Capable":"Not Capable");
    tmp_value = ((DB_LLDP_CLIENT_INFO_T*)ptr)->med_capabi.med_device_type;
    if(tmp_value == 0)
    {
        MW_CMD_OUTPUT("Type Not Defined\n");
    }
    else if(tmp_value == 1)
    {
        MW_CMD_OUTPUT("Endpoint Class I\n");
    }
    else if(tmp_value == 2)
    {
        MW_CMD_OUTPUT("Endpoint Class II\n");
    }
    else if(tmp_value == 3)
    {
        MW_CMD_OUTPUT("Endpoint Class III\n");
    }
    else if(tmp_value == 4)
    {
        MW_CMD_OUTPUT("Network Connectivity\n");
    }
    else
    {
        MW_CMD_OUTPUT("Reserved\n");
    }
    for(i = 0; i < MAX_NETWORK_POLICY_TLV_NUM; i++)
    {
        tmp_value2 = ((DB_LLDP_CLIENT_INFO_T*)ptr)->med_network_policy[i].net_policy;
        if(tmp_value2 != 0)
        {
            MW_CMD_OUTPUT("network policy %d:\n",i);
            switch((tmp_value2 & 0xff000000) >> 24)
            {
                case 1:
                    MW_CMD_OUTPUT("App type Voice\n");
                    break;
                case 2:
                    MW_CMD_OUTPUT("App type Voice Signaling\n");
                    break;
                case 3:
                    MW_CMD_OUTPUT("App type Guest Voice\n");
                    break;
                case 4:
                    MW_CMD_OUTPUT("App type Guest Voice Signaling\n");
                    break;
                case 5:
                    MW_CMD_OUTPUT("App type Softphone Voice\n");
                    break;
                case 6:
                    MW_CMD_OUTPUT("App type Video Conferencing\n");
                    break;
                case 7:
                    MW_CMD_OUTPUT("App type Streaming Video\n");
                    break;
                case 8:
                    MW_CMD_OUTPUT("App type Video Signaling\n");
                    break;
                default:
                    MW_CMD_OUTPUT("App type Reserved\n");
                    break;
            }
            if((tmp_value2 & 0x00800000) >> 23)
            {
                MW_CMD_OUTPUT("Policy: Unknown\n");
            }
            else
            {
                MW_CMD_OUTPUT("Policy: Defined\n");
            }
            if((tmp_value2 & 0x00400000) >> 22)
            {
                MW_CMD_OUTPUT("Tagged: Yes\n");
            }
            else
            {
                MW_CMD_OUTPUT("Tagged: No\n");
            }
            MW_CMD_OUTPUT("VLAN ID: %d\n", (tmp_value2 & 0x001ffe00) >> 9);
            MW_CMD_OUTPUT("L2 Priority: %d\n", (tmp_value2 & 0x000001c0) >> 6);
            MW_CMD_OUTPUT("DSCP Priority: %d\n", tmp_value2 & 0x0000003f);
        }
    }
    tmp_value = ((DB_LLDP_CLIENT_INFO_T*)ptr)->med_exten_power.power_info.p_info;
    if(((tmp_value & 0xc0) >> 6) == 0x0)
    {
        MW_CMD_OUTPUT("00...... Pow Type: PSE Device\n");
        if(((tmp_value & 0x30) >> 4) == 0x0)
        {
            MW_CMD_OUTPUT("..00.... Pow Source: Unknown\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x1)
        {
            MW_CMD_OUTPUT("..01.... Pow Source: Primary Power Source\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x2)
        {
            MW_CMD_OUTPUT("..10.... Pow Source: Backup Power Source\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x3)
        {
            MW_CMD_OUTPUT("..11.... Pow Source: Reserved\n");
        }
    }
    else if(((tmp_value & 0xc0) >> 6) == 0x1)
    {
        MW_CMD_OUTPUT("01...... Pow Type: PD Device\n");
        if(((tmp_value & 0x30) >> 4) == 0x0)
        {
            MW_CMD_OUTPUT("..00.... Pow Source: Unknown\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x1)
        {
            MW_CMD_OUTPUT("..01.... Pow Source: PSE\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x2)
        {
            MW_CMD_OUTPUT("..10.... Pow Source: Local\n");
        }
        else if(((tmp_value & 0x30) >> 4) == 0x3)
        {
            MW_CMD_OUTPUT("..11.... Pow Source: PSE and Local\n");
        }
    }
    else if(((tmp_value & 0xc0) >> 6) == 0x2)
    {
        MW_CMD_OUTPUT("10...... Pow Type: Reserved\n");
    }
    else if(((tmp_value & 0xc0) >> 6) == 0x3)
    {
        MW_CMD_OUTPUT("11...... Pow Type: Reserved\n");
    }

    if((tmp_value & 0x0f) == 0x0)
    {
        MW_CMD_OUTPUT("....0000 Pow Priority: Unknown\n");
    }
    else if((tmp_value & 0x0f) == 0x1)
    {
        MW_CMD_OUTPUT("....0001 Pow Priority: Critical\n");
    }
    else if((tmp_value & 0x0f) == 0x2)
    {
        MW_CMD_OUTPUT("....0010 Pow Priority: High\n");
    }
    else if((tmp_value & 0x0f) == 0x3)
    {
        MW_CMD_OUTPUT("....0011 Pow Priority: Low\n");
    }
    else
    {
        MW_CMD_OUTPUT("........ Pow Priority: Reserved\n");
    }

    tmp_value = (*(ptr+LLDP_CLIENT_INFO_SIZE-1) << 8) | (*(ptr+LLDP_CLIENT_INFO_SIZE-2));
    if(tmp_value <= 1023)
    {
        MW_CMD_OUTPUT("Power Value: %d mw\n", tmp_value * 100);
    }
    else
    {
        MW_CMD_OUTPUT("Power Value: Reserved\n");
    }
}

static void
lldp_get_client_info(UI16_T * tmp_index)
{
    DB_MSG_T *ptr_msg = NULL;
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *tmp_ptr = NULL;
    UI8_T i = 0;
    lldp_cli_queue_getData(LLDP_CLIENT_INFO, DB_ALL_FIELDS, *tmp_index, &ptr_msg, &data_size, (void **)&ptr_data);
    MW_CMD_OUTPUT("client info size %d\n", data_size);
    if(ptr_data != NULL)
    {
        tmp_ptr = ptr_data;
    }
    for(; i < data_size; i++)
    {
        if((i != 0) && ((i % 16) == 0))
        {
            MW_CMD_OUTPUT("\n");
        }
        MW_CMD_OUTPUT("%02x ", *tmp_ptr);
        tmp_ptr++;
    }
    MW_CMD_OUTPUT("\n");
    lldp_print_client_info(ptr_data);
    MW_FREE(ptr_msg);
}

/* FUNCTION NAME:   lldp_dump_cmd
 * PURPOSE:
 *      This API is used for mw cmd to dump lldp port client info.
 *
 * INPUT:
 *      port        --  0-28
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
lldp_dump_cmd(
    UI32_T        port_num,
    UI32_T        ttl)
{
    UI8_T i = 0;
    UI8_T port = (UI8_T)port_num;
    UI16_T tmp_ttl = (UI16_T)ttl;

    if(port > 28)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port is not between 0 and 28, command is error, please check", __func__, __LINE__);
        return;
    }
    if(ttl > 65535)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: ttl is bigger than 65535, command is error, please check", __func__, __LINE__);
        return;
    }

    if(port == 0)
    {
        UI8_T j = 1;
        for(; j <= PLAT_MAX_PORT_NUM; j++)
        {
            for(i = 0; i < MAX_LLDP_CLIENT_PER_PORT; i++)
            {
                if(port_msap_rx[j-1][i] != NULL)
                {
                    UI16_T tmp_index = 0;
                    if(j > PLAT_CPU_PORT)
                    {
                        tmp_index = (j - 1) * MAX_LLDP_CLIENT_PER_PORT + i + 1;
                    }
                    MW_CMD_OUTPUT("port %d neighbor %d addr is %p\n", j, i+1, port_msap_rx[j-1][i]);
                    MW_CMD_OUTPUT("port %d neighbor %d info is\n", j, i+1);
                    lldp_get_client_info(&tmp_index);
                }
                else
                {
                    MW_CMD_OUTPUT("port %d has no neighbor %d\n", j, i+1);
                }
            }
        }
    }
    else
    {
        for(i = 0; i < MAX_LLDP_CLIENT_PER_PORT; i++)
        {
            if(port_msap_rx[port-1][i] != NULL)
            {
                UI16_T tmp_index = 0;
                if(port > PLAT_CPU_PORT)
                {
                    tmp_index = (port - 1) * MAX_LLDP_CLIENT_PER_PORT + i + 1;
                }
                MW_CMD_OUTPUT("port %d neighbor %d addr is %p\n", port, i+1, port_msap_rx[port-1][i]);
                MW_CMD_OUTPUT("port %d neighbor %d info is\n", port, i+1);
                lldp_get_client_info(&tmp_index);
                MW_CMD_OUTPUT("port %d neighbor %d msap count set to %d, force to free\n", port, i+1, tmp_ttl);
                port_msap_rx_count[port-1][i] = tmp_ttl;
            }
            else
            {
                MW_CMD_OUTPUT("port %d has no neighbor %d\n", port, i+1);
            }
        }
    }
    return;
}

static void
lldp_clear_port_neighbo_info(UI8_T * u_portId)
{
    UI8_T i = 0;
    UI8_T tmp_index = 0;

    for(i = 1; i < (MAX_LLDP_CLIENT_PER_PORT + 1); i++)
    {
        if(port_msap_rx[*u_portId][i - 1] != NULL)
        {
            DB_LLDP_CLIENT_INFO_T msap_info;
            osapi_memset(&msap_info, 0, LLDP_CLIENT_INFO_SIZE);
            tmp_index = (*u_portId) * MAX_LLDP_CLIENT_PER_PORT + i;
            lldp_db_queue_send(M_UPDATE, LLDP_CLIENT_INFO, DB_ALL_FIELDS, tmp_index, &msap_info, LLDP_CLIENT_INFO_SIZE);
        }
    }
}

static MW_ERROR_NO_T
lldp_tlv_check(UI8_T tlv_type,
    UI8_T tlv_sub_type,
    UI16_T tlv_len,
    UI8_T * ptr_data,
    UI16_T pkt_len)
{
    UI8_T * ptr_tlv = ptr_data;
    UI8_T  oui[3] = {0x00, 0x12, 0xbb};
    UI8_T tmp_oui1 = 0;
    UI8_T tmp_oui2 = 0;
    UI8_T tmp_oui3 = 0;
    UI8_T sub_type = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (pkt_len <= tlv_len)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len error at %d tlv, pkt_len %d tlv_len %d", __func__, __LINE__, tlv_type, pkt_len, tlv_len);
        rc = MW_E_OTHERS;
        return rc;
    }
    switch(tlv_type)
    {
        case LLDP_TLV_CHASSIS_ID_TYPE:
        case LLDP_TLV_PORT_ID_TYPE:
            if ((tlv_len < 2) || (tlv_len > 256))
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt error at %d tlv, pkt_len %d tlv_len %d", __func__, __LINE__, tlv_type, pkt_len, tlv_len);
                rc = MW_E_OTHERS;
                break;
            }
            /*sub type mac address*/
            if (((tlv_type == LLDP_TLV_CHASSIS_ID_TYPE) && (tlv_sub_type == 4))
                || ((tlv_type == LLDP_TLV_PORT_ID_TYPE) && (tlv_sub_type == 3)))
            {
                /*sub type + mac address length = 7*/
                if (tlv_len != 7)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt error at %d tlv(mac address), tlv_len %d", __func__, __LINE__, tlv_type, tlv_len);
                    rc = MW_E_OTHERS;
                }
            }
            break;
        case LLDP_TLV_SYSTEM_NAME_TYPE:
            if (tlv_len > 255)
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt error at system name tlv, pkt_len %d tlv_len %d", __func__, __LINE__, pkt_len, tlv_len);
                rc = MW_E_OTHERS;
            }
            break;
        case LLDP_TLV_MED_TYPE:
            /* judge oui is 00-12-bb */
            tmp_oui1 = *ptr_tlv;
            ptr_tlv += 1;
            tmp_oui2 = *ptr_tlv;
            ptr_tlv += 1;
            tmp_oui3 = *ptr_tlv;
            ptr_tlv += 1;
            MW_LOG_DEBUG(LLDP, "%s[%d]: %x %x %x", __func__, __LINE__, tmp_oui1, tmp_oui2, tmp_oui3);
            if ((tmp_oui1 != oui[0]) || (tmp_oui2 != oui[1]) || (tmp_oui3 != oui[2]))
            {
                MW_LOG_INFO(LLDP, "%s[%d]: org uni %x %x %x not supported now", __func__, __LINE__, tmp_oui1, tmp_oui2, tmp_oui3);
                rc = MW_E_NOT_SUPPORT;
                break;
            }
            sub_type = *ptr_tlv;
            if ((sub_type != LLDP_TLV_MED_CAP_SUB_TYPE) && (sub_type != LLDP_TLV_MED_NET_SUB_TYPE) && (sub_type != LLDP_TLV_MED_EXT_SUB_TYPE))
            {
                MW_LOG_INFO(LLDP, "%s[%d]: med sub type %d not supported now", __func__, __LINE__, sub_type);
                rc = MW_E_NOT_SUPPORT;
                break;
            }
            if (((sub_type == LLDP_TLV_MED_CAP_SUB_TYPE) && (tlv_len != 7))
                || ((sub_type == LLDP_TLV_MED_NET_SUB_TYPE) && (tlv_len != 8))
                || ((sub_type == LLDP_TLV_MED_EXT_SUB_TYPE) && (tlv_len != 7)))
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt error at med tlv(sub type %d), tlv_len %d", __func__, __LINE__, sub_type, tlv_len);
                rc = MW_E_OTHERS;
            }
            break;
        default:
            MW_LOG_DEBUG(LLDP, "%s[%d]: something is error, please check!", __func__, __LINE__);
            rc = MW_E_OTHERS;
            break;
    }

    return rc;
}

static MW_ERROR_NO_T
lldp_decode(
    struct pbuf        *ptr_pbuf,
    DB_LLDP_CLIENT_INFO_T        *neigh_info,
    UI8_T        *chassid_len,
    UI8_T        *portid_len)
{
    UI8_T * p = NULL, * p_buf = NULL;
    UI16_T pkt_len = 0;
    static UI8_T network_tlv_num = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    if((ptr_pbuf->next != NULL) && (ptr_pbuf->next->payload != NULL))
    {
        pkt_len = ptr_pbuf->tot_len;
        if (osapi_calloc(pkt_len, LLDP_TASK_NAME, (void**)&p_buf) != MW_E_OK)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: lldp_decode: no heap to decode", __func__, __LINE__);
            rc = MW_E_NO_MEMORY;
            return rc;
        }
        pbuf_copy_partial(ptr_pbuf, p_buf, pkt_len, 0);
        p = p_buf;
    }
    else
    {
        p = (UI8_T *)ptr_pbuf->payload;
        pkt_len = ptr_pbuf->len;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
    while(pkt_len > 0)
    {
        UI8_T tlv_type = 0;
        UI16_T tlv_len = 0;
        UI8_T med_sub_type = 0;
        UI8_T i = 0;
        UI8_T tmp_chassid_len = 0;
        UI8_T tmp_portid_len = 0;
        UI8_T tmp_chassid_type = 0;
        UI8_T tmp_portid_type = 0;
        tlv_type = (*p) >> 1;
        switch(tlv_type)
        {
            case LLDP_TLV_END_TYPE:
                MW_LOG_DEBUG(LLDP, "%s[%d]: reach end tlv", __func__, __LINE__);
                pkt_len -= LLDP_TLV_END_LEN;
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                if(pkt_len != 0)
                {
                    /* pkt_len now is 2 or 4 or other value,for there is padding field(fcs or complement length) exist */
                    MW_LOG_INFO(LLDP, "%s[%d]: left len is %d", __func__, __LINE__, pkt_len);
                    pkt_len = 0;
                }
                break;
            case LLDP_TLV_CHASSIS_ID_TYPE:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                MW_LOG_DEBUG(LLDP, "%s[%d]: tlv_length is %d", __func__, __LINE__, tlv_len);
                p += 2;
                pkt_len -= 2;
                tmp_chassid_type = *p;
                if(*p == 0x5)
                {
                    p += 2;
                    pkt_len -= 2;
                    tmp_chassid_len = tlv_len - 2;
                }
                else
                {
                    p += 1;
                    pkt_len -= 1;
                    tmp_chassid_len = tlv_len - 1;
                }
                rc = lldp_tlv_check(LLDP_TLV_CHASSIS_ID_TYPE, tmp_chassid_type, tlv_len, p, pkt_len);
                if(MW_E_OK != rc)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: chassisid tlv check error %d", __func__, __LINE__, rc);
                    goto exit;
                }
                if(tmp_chassid_len >= (MAX_LLDP_STR_SIZE - 1))
                {
                    osapi_memcpy(neigh_info->chassis_id, p, (MAX_LLDP_STR_SIZE - 1));
                    neigh_info->chassis_id[MAX_LLDP_STR_SIZE - 1] = tmp_chassid_type;
                    *chassid_len = (MAX_LLDP_STR_SIZE - 1);
                }
                else
                {
                    osapi_memcpy(neigh_info->chassis_id, p, tmp_chassid_len);
                    neigh_info->chassis_id[MAX_LLDP_STR_SIZE - 1] = tmp_chassid_type;
                    *chassid_len = tmp_chassid_len;
                }
                for(i = 0; i < (*chassid_len); i++)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: chassis_id[%d] is %x", __func__, __LINE__, i, (neigh_info->chassis_id[i] & 0xff));
                }
                pkt_len -= tmp_chassid_len;
                p += tmp_chassid_len;
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                break;
            case LLDP_TLV_PORT_ID_TYPE:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                MW_LOG_DEBUG(LLDP, "%s[%d]: tlv_length is %d", __func__, __LINE__, tlv_len);
                p += 2;
                pkt_len -= 2;
                tmp_portid_type = *p;
                if(*p == 0x4)
                {
                    p += 2;
                    pkt_len -= 2;
                    tmp_portid_len = tlv_len - 2;
                }
                else
                {
                    p += 1;
                    pkt_len -= 1;
                    tmp_portid_len = tlv_len - 1;
                }
                rc = lldp_tlv_check(LLDP_TLV_PORT_ID_TYPE, tmp_portid_type, tlv_len, p, pkt_len);
                if(MW_E_OK != rc)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: portid tlv check error %d", __func__, __LINE__, rc);
                    goto exit;
                }
                if(tmp_portid_len >= (MAX_LLDP_STR_SIZE - 1))
                {
                    osapi_memcpy(neigh_info->port_id, p, (MAX_LLDP_STR_SIZE - 1));
                    neigh_info->port_id[MAX_LLDP_STR_SIZE - 1] = tmp_portid_type;
                    *portid_len = (MAX_LLDP_STR_SIZE - 1);
                }
                else
                {
                    osapi_memcpy(neigh_info->port_id, p, tmp_portid_len);
                    neigh_info->port_id[MAX_LLDP_STR_SIZE - 1] = tmp_portid_type;
                    *portid_len = tmp_portid_len;
                }
                for(i = 0; i < (*portid_len); i++)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: port_id[%d] is %x", __func__, __LINE__, i, (neigh_info->port_id[i] & 0xff));
                }
                pkt_len -= tmp_portid_len;
                p += tmp_portid_len;
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                break;
            case LLDP_TLV_TTL_TYPE:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                MW_LOG_DEBUG(LLDP, "%s[%d]: tlv_length is %d", __func__, __LINE__, tlv_len);
                if (2 != tlv_len)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: ttl tlv length error %d", __func__, __LINE__, tlv_len);
                    rc = MW_E_OTHERS;
                    goto exit;
                }
                p += 2;
                pkt_len -= 2;
                neigh_info->time_to_live = ntohs(p[1] << 8 | p[0]);
                pkt_len -= tlv_len;
                p += tlv_len;
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                break;
            case LLDP_TLV_SYSTEM_NAME_TYPE:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                MW_LOG_DEBUG(LLDP, "%s[%d]: tlv_length is %d", __func__, __LINE__, tlv_len);
                p += 2;
                pkt_len -= 2;
                rc = lldp_tlv_check(LLDP_TLV_SYSTEM_NAME_TYPE, 0, tlv_len, p, pkt_len);
                if(MW_E_OK != rc)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: system name tlv check error %d", __func__, __LINE__, rc);
                    goto exit;
                }
                if(tlv_len > (MAX_LLDP_STR_SIZE - 1))
                {
                    osapi_memcpy(neigh_info->system_name, p, (MAX_LLDP_STR_SIZE - 1));
                    neigh_info->system_name[MAX_LLDP_STR_SIZE - 1] = '\0';
                }
                else
                {
                    osapi_memcpy(neigh_info->system_name, p, tlv_len);
                    neigh_info->system_name[tlv_len] = '\0';
                }
                pkt_len -= tlv_len;
                p += tlv_len;
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                break;
            case LLDP_TLV_MED_TYPE:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                MW_LOG_DEBUG(LLDP, "%s[%d]: tlv_length is %d", __func__, __LINE__, tlv_len);
                p += 2;
                pkt_len -= 2;
                rc = lldp_tlv_check(LLDP_TLV_MED_TYPE, 0, tlv_len, p, pkt_len);
                if (MW_E_NOT_SUPPORT == rc)
                {
                    pkt_len -= tlv_len;
                    p += tlv_len;
                    rc = MW_E_OK;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                    break;
                }
                else if (MW_E_OK != rc)
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: med tlv check error %d", __func__, __LINE__, rc);
                    goto exit;
                }
                p += 3;
                pkt_len -= 3;
                med_sub_type = *p;
                if(med_sub_type == LLDP_TLV_MED_CAP_SUB_TYPE)
                {
                    p += 1;
                    pkt_len -= 1;
                    neigh_info->med_capabi.med_capability = ntohs(p[1] << 8 | p[0]);
                    MW_LOG_DEBUG(LLDP, "%s[%d]: med_capability is %d", __func__, __LINE__, neigh_info->med_capabi.med_capability);
                    p += 2;
                    pkt_len -= 2;
                    neigh_info->med_capabi.med_device_type = *p;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: med_device_type is %d", __func__, __LINE__, neigh_info->med_capabi.med_device_type);
                    p += 1;
                    pkt_len -= 1;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                }
                else if(med_sub_type == LLDP_TLV_MED_NET_SUB_TYPE)
                {
                    p += 1;
                    pkt_len -= 1;
                    if(network_tlv_num < MAX_NETWORK_POLICY_TLV_NUM)
                    {
                        neigh_info->med_network_policy[network_tlv_num].net_policy = ntohl(p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: net policy is %x", __func__, __LINE__, neigh_info->med_network_policy[network_tlv_num].net_policy);
                    }
                    else
                    {
                        MW_LOG_INFO(LLDP, "%s[%d]: med network policy tlv only support %d only now", __func__, __LINE__, MAX_NETWORK_POLICY_TLV_NUM);
                    }
                    p += 4;
                    pkt_len -= 4;
                    network_tlv_num++;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                }
                else if(med_sub_type == LLDP_TLV_MED_EXT_SUB_TYPE)
                {
                    p += 1;
                    pkt_len -= 1;
                    neigh_info->med_exten_power.power_info.p_info = *p;
                    p += 1;
                    pkt_len -= 1;
                    neigh_info->med_exten_power.power_value = ntohs(p[1] << 8 | p[0]);
                    p += 2;
                    pkt_len -= 2;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                }
                else
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: med unknown sub type %d", __func__, __LINE__, med_sub_type);
                }
                break;
            default:
                tlv_len = ntohs(p[1] << 8 | p[0]) & 0x01ff;
                if(pkt_len < (2 + tlv_len))
                {
                    MW_LOG_DEBUG(LLDP, "%s[%d]: unknown tlv error, pkt len is %d and tlv len is %d", __func__, __LINE__, pkt_len, tlv_len);
                    rc = MW_E_OTHERS;
                    goto exit;
                }
                pkt_len -= (2 + tlv_len);
                p += (2 + tlv_len);
                MW_LOG_DEBUG(LLDP, "%s[%d]: pkt len is %d", __func__, __LINE__, pkt_len);
                MW_LOG_INFO(LLDP, "%s[%d]: tlv type %d is not supported now", __func__, __LINE__, tlv_type);
                break;
        }
    }
exit:
    if(p_buf != NULL)
    {
        osapi_free(p_buf);
    }
    network_tlv_num = 0;
    return rc;
}

static void
lldp_recv(struct pbuf * ptr_pbuf)
{
    UI8_T    rx_u_portId = 0, tmp_port = 0;
    UI16_T   rx_u_type = 0;
    void *  ptr_str = NULL;
    UI8_T    tmp_size = 0, tmp_chassid_len = 0, tmp_portid_len = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_LLDP_CLIENT_INFO_T neigh_info;

    if(NULL == ptr_pbuf)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: lldp_recv: invalid pbuf\n", __func__, __LINE__);
        return;
    }

    if(DISABLE == lldp_info.global_enable)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: lldp glo disable", __func__, __LINE__);
        pbuf_free(ptr_pbuf);
        return;
    }
    osapi_memset(&neigh_info, 0, LLDP_CLIENT_INFO_SIZE);
    /*TODO: parse special tag to find out the correct port*/
    rx_u_portId = ptr_pbuf->stag_hdr.rx_hdr.sp;
    if(rx_u_portId > PLAT_MAX_PORT_NUM)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: port %d is invalid", __func__, __LINE__, rx_u_portId);
        pbuf_free(ptr_pbuf);
        return;
    }
    rx_u_type = ptr_pbuf->ether_hdr.type;
    if(rx_u_type != LLDP_ETHER_TYPE)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: pkt ether type %x is not 0x88cc", __func__, __LINE__, rx_u_type);
        pbuf_free(ptr_pbuf);
        return;
    }
#ifdef AIR_SUPPORT_LP
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    MW_PORT_BITMAP_T lp_blocked_pbmp;
    lp_getBlockPbmp(lp_blocked_pbmp);
    if(AIR_PORT_CHK(lp_blocked_pbmp, rx_u_portId))
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d is lp blocked, rx drop", __func__, __LINE__, rx_u_portId);
        pbuf_free(ptr_pbuf);
        return;
    }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
#endif /* AIR_SUPPORT_LP */
    tmp_port = rx_u_portId;
    if(rx_u_portId > PLAT_CPU_PORT)
        tmp_port -= 1;
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    if((TRUE == lldp_not_send_blocking) && (lldp_info.rstp_status[tmp_port] == LLDP_RSTP_PORT_OPER_STAT_DISCARD))
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d is rstp blocking state, rx drop", __func__, __LINE__, rx_u_portId);
        pbuf_free(ptr_pbuf);
        return;
    }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
    /*TODO: judge port rx enable*/
    if(lldp_info.lldp_admin[tmp_port] < LLDP_PORT_RX_ONLY)
    {
        MW_LOG_INFO(LLDP, "%s[%d]: port %d rx is disabled", __func__, __LINE__, rx_u_portId);
        pbuf_free(ptr_pbuf);
        return;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: decode lldp frame from port %d", __func__, __LINE__, rx_u_portId);
    rc = lldp_decode(ptr_pbuf, &neigh_info, &tmp_chassid_len, &tmp_portid_len);
    if (MW_E_OK != rc)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: lldp_decode error ret %d", __func__, __LINE__, rc);
        pbuf_free(ptr_pbuf);
        return;
    }
    tmp_size = tmp_chassid_len + tmp_portid_len + 1;
    if (osapi_calloc(tmp_size, LLDP_TASK_NAME, (void**)&ptr_str) == MW_E_OK)
    {
        UI8_T    i = 0;
        UI8_T    j = 0;
        UI8_T    tmp_index = 0;
        UI8_T   msap_hash[LLDP_HASH_VALUE_LENGTH] = {0};
        MW_LOG_DEBUG(LLDP, "%s[%d]: malloc ptr_str %p\n", __func__, __LINE__, ptr_str);
        osapi_memcpy(ptr_str, neigh_info.chassis_id, tmp_chassid_len);
        osapi_memcpy((ptr_str + tmp_chassid_len), neigh_info.port_id, tmp_portid_len);
        md5((UI8_T *)ptr_str, (tmp_size - 1), msap_hash);
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d receive msap hash", __func__, __LINE__, rx_u_portId);
        for(i = 0; i < LLDP_HASH_VALUE_LENGTH; i++)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: hash[%d] %x", __func__, __LINE__, i, msap_hash[i]);
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: free ptr_str %p\n", __func__, __LINE__, ptr_str);
        MW_FREE(ptr_str);
        for(i = 0; i < MAX_LLDP_CLIENT_PER_PORT; i++)
        {
            if((port_msap_rx[tmp_port][i] != NULL) &&
               (osapi_memcmp(port_msap_rx[tmp_port][i]->hash_value, msap_hash, LLDP_HASH_VALUE_LENGTH) == 0))
            {
                if(neigh_info.time_to_live != 0)
                {
                    MW_LOG_INFO(LLDP, "%s[%d]: known msap,only update", __func__, __LINE__);
                    MW_LOG_DEBUG(LLDP, "%s[%d]: update:port %d neighbor %d addr is %p\n", __func__, __LINE__, tmp_port+1, i+1, port_msap_rx[tmp_port][i]);
                    /* known msap, update neighbor info to db, update msap count */
                    tmp_index = tmp_port * MAX_LLDP_CLIENT_PER_PORT + i + 1;
                    lldp_db_queue_send(M_UPDATE, LLDP_CLIENT_INFO, DB_ALL_FIELDS, tmp_index, &neigh_info, LLDP_CLIENT_INFO_SIZE);
                    port_msap_rx_count[tmp_port][i] = neigh_info.time_to_live;
                }
                else
                {
                    /*if ttl is 0, need to clear neighbor info*/
                    MW_LOG_INFO(LLDP, "%s[%d]: known msap, ttl is 0", __func__, __LINE__);
                    MW_LOG_DEBUG(LLDP, "%s[%d]: clear:port %d neighbor %d\n", __func__, __LINE__, tmp_port+1, i+1);
                    port_msap_rx_count[tmp_port][i] = 1;
                }
                pbuf_free(ptr_pbuf);
                return;
            }
        }
        /* unknown msap */
        for(i = 0; i < MAX_LLDP_CLIENT_PER_PORT; i++)
        {
            if((port_msap_rx[tmp_port][i] == NULL) && (lldp_info.global_enable == ENABLE) && (neigh_info.time_to_live != 0))
            {
                MW_LOG_INFO(LLDP, "%s[%d]: unknown msap, store", __func__, __LINE__);
                if(osapi_calloc(LLDP_HASH_VALUE_LENGTH, LLDP_TASK_NAME, (void**)&(port_msap_rx[tmp_port][i])) == MW_E_OK)
                {
                    /* has space to store, store to db */
                    MW_LOG_DEBUG(LLDP, "%s[%d]: malloc:port %d neighbor %d addr is %p\n", __func__, __LINE__, tmp_port+1, i+1, port_msap_rx[tmp_port][i]);
                    tmp_index = tmp_port * MAX_LLDP_CLIENT_PER_PORT + i + 1;
                    lldp_db_queue_send(M_UPDATE, LLDP_CLIENT_INFO, DB_ALL_FIELDS, tmp_index, &neigh_info, LLDP_CLIENT_INFO_SIZE);
                    osapi_memcpy(port_msap_rx[tmp_port][i]->hash_value, msap_hash, LLDP_HASH_VALUE_LENGTH);
                    port_msap_rx_count[tmp_port][i] = neigh_info.time_to_live;
                    for(j = 0; j < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; j++)
                    {
                        if(lldp_neibor_index[j] == 0)
                        {
                            lldp_neibor_index[j] = tmp_port * MAX_LLDP_CLIENT_PER_PORT + i + 1;
                            break;
                        }
                    }
                    pbuf_free(ptr_pbuf);
                    return;
                }
                else
                {
                    MW_LOG_ERROR(LLDP, "%s[%d]: lldp malloc %d fail", __func__, __LINE__, LLDP_HASH_VALUE_LENGTH);
                }
            }
        }
        /* do not have space to store, do nothing, wait ptr_pbuf free */
    }
    else
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: lldp malloc %d fail", __func__, __LINE__, tmp_size);
    }
    pbuf_free(ptr_pbuf);
}

static int
lldp_compute_payload_length(
    UI8_T        portId_len,
    UI8_T        sysName_len)
{
    UI16_T length = 0;
    length += LLDP_TLV_CHASSIS_ID_LEN; /* chassis id tlv,type mac */
    length += (3 + portId_len);        /* portid tlv */
    length += LLDP_TLV_TTL_LEN;        /* ttl tlv */
    length += (2 + sysName_len);       /* system name tlv */
    length += LLDP_TLV_MED_CAP_LEN;    /* med cap tlv */
    if(TRUE == lldp_info.voice_vlan_state)
    {
        length += LLDP_TLV_MED_NET_POLICY_LEN; /* med voice net policy */
    }
    if (TRUE == lldp_info.survei_vlan_state)
    {
        length += LLDP_TLV_MED_NET_POLICY_LEN; /* med voice net policy */
    }
    length += LLDP_TLV_END_LEN;       /* end tlv */
    return length;
}

static int
lldp_compute_shutdown_msg_length(UI8_T portId_len)
{
    UI16_T length = 0;
    length += LLDP_TLV_CHASSIS_ID_LEN; /* chassis id tlv,type mac */
    length += (3 + portId_len);        /* portid tlv */
    length += LLDP_TLV_TTL_LEN;        /* ttl tlv */
    length += LLDP_TLV_END_LEN;       /* end tlv */
    return length;
}

static int
lldp_orga_and_tx_packet(UI8_T * u_portId)
{
    UI16_T pkt_len = 0;
    UI32_T ttl = 0;
    UI8_T * p = NULL;
    C8_T   port_id[LLDP_PORT_ID_SIZE];
    UI8_T portId_len = 0;
    UI8_T sysName_len = 0;
    I32_T ret = 0;
    UI8_T tmp_send_net_voice = 0;
    UI8_T tmp_send_net_survei = 0;
    AIR_PORT_BITMAP_T port_bitmap = { 0 };
    AIR_ERROR_NO_T err = AIR_E_OK;
    struct pbuf * tmp_ptr = NULL;
#ifdef AIR_SUPPORT_LP
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    MW_PORT_BITMAP_T lp_blocked_pbmp;

    if (*u_portId == 0)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: invalid port %d, skip", __func__, __LINE__, *u_portId);
        return MW_E_BAD_PARAMETER;
    }

    lp_getBlockPbmp(lp_blocked_pbmp);
    if(AIR_PORT_CHK(lp_blocked_pbmp, *u_portId))
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d is lp blocked, do not send", __func__, __LINE__, *u_portId);
        return MW_E_OK;
    }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    if((TRUE == lldp_not_send_blocking) && (lldp_info.rstp_status[*u_portId - 1] == LLDP_RSTP_PORT_OPER_STAT_DISCARD))
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d is rstp blocking state, do not send", __func__, __LINE__, *u_portId);
        return MW_E_OK;
    }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
    osapi_memset(port_id, 0, sizeof(port_id));
    ret = osapi_snprintf(port_id, LLDP_PORT_ID_SIZE, "port_%d", *u_portId);
    if(ret < 0)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_snprintf error", __func__, __LINE__);
        return MW_E_OTHERS;
    }
    else
    {
        portId_len = ret & 0xff;
    }
    sysName_len = osapi_strlen(lldp_info.sys_name);
    if(TRUE == port_send_shutdown_msg[(*u_portId) - 1])
    {
        pkt_len = lldp_compute_shutdown_msg_length(portId_len);
    }
    else
    {
        pkt_len = lldp_compute_payload_length(portId_len, sysName_len);
    }
    tmp_ptr = pbuf_alloc(PBUF_TRANSPORT, pkt_len, PBUF_RAM);
    if(tmp_ptr != NULL)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: pbuf_alloc ok, tmp_ptr %p", __func__, __LINE__, tmp_ptr);
        if(tmp_ptr->payload == NULL)
        {
            pbuf_free(tmp_ptr);
            MW_LOG_ERROR(LLDP, "%s[%d]: tmp_ptr payload is NULL,return", __func__, __LINE__);
            return MW_E_OTHERS;
        }
        osapi_memset(tmp_ptr->payload, 0, pkt_len);
        tmp_ptr->ether_hdr.dest.addr[0] = 0x01;
        tmp_ptr->ether_hdr.dest.addr[1] = 0x80;
        tmp_ptr->ether_hdr.dest.addr[2] = 0xc2;
        tmp_ptr->ether_hdr.dest.addr[3] = 0x00;
        tmp_ptr->ether_hdr.dest.addr[4] = 0x00;
        tmp_ptr->ether_hdr.dest.addr[5] = 0x0e;

        osapi_memcpy(tmp_ptr->ether_hdr.src.addr, _mw_mac_addr, MAC_ADDRESS_LEN);
        if((tmp_ptr->ether_hdr.src.addr[5] + (*u_portId)) >= 0xff)
        {
            tmp_ptr->ether_hdr.src.addr[4] += 1;
            tmp_ptr->ether_hdr.src.addr[5] = tmp_ptr->ether_hdr.src.addr[5] + (*u_portId) - 0xff;
        }
        else
        {
            tmp_ptr->ether_hdr.src.addr[5] += (*u_portId);
        }
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
        if(TRUE == lldp_per_sys_per_mac)
        {
            osapi_memcpy(tmp_ptr->ether_hdr.src.addr, _mw_mac_addr, MAC_ADDRESS_LEN);
        }
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
        tmp_ptr->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
#ifdef AIR_EN_CORAL
        tmp_ptr->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK;
#else
        tmp_ptr->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP;
#endif
        tmp_ptr->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << (*u_portId));
        tmp_ptr->vlan_num = 1;
        tmp_ptr->vlan_hdr.tpid = 0x8100;
        tmp_ptr->vlan_hdr.priority = 7;
        tmp_ptr->vlan_hdr.vid = 1;
        tmp_ptr->ether_hdr.type = LLDP_ETHER_TYPE;
        p = (UI8_T *)tmp_ptr->payload;
        p[0] = 0x02;
        p[1] = 0x07;
        p += 2;
        *p = 0x04;
        p += 1;
        osapi_memcpy(p, _mw_mac_addr, MAC_ADDRESS_LEN);
        p += MAC_ADDRESS_LEN;
        p[0] = 0x04;
        p[1] = (1 + portId_len);
        p += 2;
        *p = 0x07;
        p += 1;
        osapi_memcpy(p, port_id, portId_len);
        p += portId_len;
        p[0] = 0x06;
        p[1] = 0x02;
        p += 2;
        if(TRUE == port_send_shutdown_msg[(*u_portId) - 1])
        {
            p[0] = 0x00;
            p[1] = 0x00;
            p += 2;
            p[0] = 0x00;
            p[1] = 0x00;
            MW_LOG_DEBUG(LLDP, "%s[%d]: begin to send lldp packet", __func__, __LINE__);
            ethernet_output_use_default_netif(tmp_ptr, ETHTYPE_LLDP);
            MW_LOG_DEBUG(LLDP, "%s[%d]: free tmp_ptr %p", __func__, __LINE__, tmp_ptr);
            pbuf_free(tmp_ptr);
            tmp_ptr = NULL;
            port_send_shutdown_msg[(*u_portId) - 1] = 0;
            return MW_E_OK;
        }
        ttl = lldp_info.tx_interval * lldp_info.tx_hold;
        if(ttl > LLDP_TX_TTL_MAX)
        {
            ttl = LLDP_TX_TTL_MAX;
        }
        p[0] = (ttl & 0xff00) >> 8;
        p[1] = ttl & 0xff;
        p += 2;
        p[0] = 0x0a;
        p[1] = ((5 << 9) | sysName_len) & 0xff;
        p += 2;
        osapi_memcpy(p, lldp_info.sys_name, sysName_len);
        p += sysName_len;
        p[0] = 0xfe;
        p[1] = 0x07;
        p += 2;
        *p = 0x00;
        p += 1;
        *p = 0x12;
        p += 1;
        *p = 0xbb;
        p += 1;
        *p = 1;
        p += 1;
        if (((TRUE == lldp_info.voice_vlan_state) && (lldp_info.voice_vlan_id != 0))
            || ((TRUE == lldp_info.survei_vlan_state) && (lldp_info.survei_vlan_id != 0)))
        {
            if((TRUE == lldp_info.voice_vlan_state) && (lldp_info.voice_vlan_id != 0))
            {
                err = air_vlan_getMember(0, lldp_info.voice_vlan_id, port_bitmap);
                if(err == AIR_E_OK)
                {
                    if((1 << (*u_portId)) & port_bitmap[0])
                    {
                        tmp_send_net_voice = 1;
                    }
                }
                else
                {
                    /*indicate vlan is not set port member, do nothing*/
                    MW_LOG_DEBUG(LLDP, "%s[%d]: air_vlan_getMember failed port %d", __func__, __LINE__, (*u_portId));
                }
            }
            if ((TRUE == lldp_info.survei_vlan_state) && (lldp_info.survei_vlan_id != 0))
            {
                err = air_vlan_getMember(0, lldp_info.survei_vlan_id, port_bitmap);
                if(err == AIR_E_OK)
                {
                    if((1 << (*u_portId)) & port_bitmap[0])
                    {
                        tmp_send_net_survei = 1;
                    }
                }
                else
                {
                    /*indicate vlan is not set port member, do nothing*/
                    MW_LOG_DEBUG(LLDP, "%s[%d]: air_vlan_getMember failed port %d", __func__, __LINE__, (*u_portId));
                }
            }
            if ((TRUE == tmp_send_net_voice) || (TRUE == tmp_send_net_survei))
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: send net voice %d and send net survei %d", __func__, __LINE__, tmp_send_net_voice, tmp_send_net_survei);
                p[0] = 0x00;
                p[1] = 0x03;
            }
            else
            {
                p[0] = 0x00;
                p[1] = 0x01;
            }
        }
        else
        {
            p[0] = 0x00;
            p[1] = 0x01;
        }
        p += 2;
        *p = 4;
        p += 1;
        if ((TRUE == lldp_info.voice_vlan_state) && (TRUE == tmp_send_net_voice))
        {
            UI32_T tmp_value = 0;
            p[0] = 0xfe;
            p[1] = 0x08;
            p += 2;
            *p = 0x00;
            p += 1;
            *p = 0x12;
            p += 1;
            *p = 0xbb;
            p += 1;
            *p = 2;
            p += 1;
            tmp_value = (1 << 24) | (1 << 22) | (lldp_info.voice_vlan_id << 9) | (lldp_info.voice_vlan_pri << 6);
            p[0] = (tmp_value & 0xff000000) >> 24;
            p[1] = (tmp_value & 0x00ff0000) >> 16;
            p[2] = (tmp_value & 0x0000ff00) >> 8;
            p[3] = tmp_value & 0xff;
            p += 4;
        }
        if ((TRUE == lldp_info.survei_vlan_state) && (TRUE == tmp_send_net_survei))
        {
            UI32_T tmp_value = 0;
            p[0] = 0xfe;
            p[1] = 0x08;
            p += 2;
            *p = 0x00;
            p += 1;
            *p = 0x12;
            p += 1;
            *p = 0xbb;
            p += 1;
            *p = 2;
            p += 1;
            tmp_value = (7 << 24) | (1 << 22) | (lldp_info.survei_vlan_id << 9) | (lldp_info.survei_vlan_pri << 6);
            p[0] = (tmp_value & 0xff000000) >> 24;
            p[1] = (tmp_value & 0x00ff0000) >> 16;
            p[2] = (tmp_value & 0x0000ff00) >> 8;
            p[3] = tmp_value & 0xff;
            p += 4;
        }
        p[0] = 0x00;
        p[1] = 0x00;
        MW_LOG_DEBUG(LLDP, "%s[%d]: begin to send lldp packet", __func__, __LINE__);
        ethernet_output_use_default_netif(tmp_ptr, ETHTYPE_LLDP);
        MW_LOG_DEBUG(LLDP, "%s[%d]: free tmp_ptr %p", __func__, __LINE__, tmp_ptr);
        pbuf_free(tmp_ptr);
        tmp_ptr = NULL;
    }
    else
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: pbuf_alloc fail, tmp_ptr is NULL", __func__, __LINE__);
        return MW_E_NO_MEMORY;
    }
    return MW_E_OK;
}

static void
single_port_tx(
    UI8_T        *u_portId,
    LLDP_TX_TYPE_INFO_T        type)
{
    UI16_T tmp_index = 0;
    UI8_T tmp_port = 0;
    UI32_T local_time = 0;
    I32_T  tmp_time = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    tmp_port = *u_portId;
    tmp_index = *u_portId + 1;
    MW_LOG_DEBUG(LLDP, "%s[%d]: port %d", __func__, __LINE__,tmp_index);
    if((((lldp_info.lldp_admin[tmp_port] == LLDP_PORT_TX_ONLY) ||
        (lldp_info.lldp_admin[tmp_port] == LLDP_PORT_TX_RX)) &&
        (lldp_info.port_admin[tmp_port] == TRUE)) ||
        (port_send_shutdown_msg[tmp_port] == TRUE))
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d lldp status %d", __func__, __LINE__, tmp_index, lldp_info.lldp_admin[tmp_port]);
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d admin status %d", __func__, __LINE__, tmp_index, lldp_info.port_admin[tmp_port]);
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d oper status %d", __func__, __LINE__, tmp_index, lldp_info.oper_status[tmp_port]);
        if(lldp_info.oper_status[tmp_port])
        {
            local_time = sys_now();
            tmp_time = local_time - port_tx_timestamp[tmp_port];
            if((tmp_time < 0) || (tmp_time > lldp_info.tx_delay * 1000) || (port_send_shutdown_msg[tmp_port] == TRUE))
            {
                UI8_T index = (UI8_T)tmp_index;
                rc = lldp_orga_and_tx_packet(&index);
                if(rc != MW_E_OK)
                {
                    MW_LOG_ERROR(LLDP, "%s[%d]: lldp tx packet fail", __func__, __LINE__);
                }
                else
                {
                    port_tx_timestamp[tmp_port] = local_time;
                }
            }
            else
            {
                if(LLDP_PORT_INFO_CHANGE_TX == type)
                {
                    port_info_change_tx_count[tmp_port] = (lldp_info.tx_delay * 1000 - tmp_time)/1000;
                    if(port_info_change_tx_count[tmp_port] == 0)
                    {
                        port_info_change_tx_count[tmp_port] = 1;
                    }
                }
            }
        }
    }
}

static void
lldp_global_tx_timeout(void)
{
    UI8_T u_portId = 0;
    UI8_T tmp_port = 0;
    LLDP_TX_TYPE_INFO_T tmp_type = LLDP_GLOBAL_TX;

    if(FALSE == lldp_info.global_enable)
    {
        return;
    }

    MW_LOG_DEBUG(LLDP, "%s[%d]: _mw_port_bmp_total is %x", __func__, __LINE__, _mw_port_bmp_total[0]);
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if(PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        tmp_port = u_portId;
        if(u_portId > PLAT_CPU_PORT)
        {
            tmp_port -= 1;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d", __func__, __LINE__,tmp_port);
        if(tmp_port >= MAX_PORT_NUM)
        {
            continue;
        }
        single_port_tx(&tmp_port, tmp_type);
        osapi_delay(100);
    }
    return;
}

static void
lldp_port_info_change_timeout(UI8_T        *u_portId)
{
    LLDP_TX_TYPE_INFO_T tmp_type = LLDP_PORT_INFO_CHANGE_TX;
    MW_LOG_DEBUG(LLDP, "%s[%d]: port is %x", __func__, __LINE__, * u_portId);
    single_port_tx(u_portId, tmp_type);
    osapi_delay(50);
}

static void
lldp_arrange_neibor_index(void)
{
    UI32_T i = 0, j = 0;
    for(i = 0; i < (MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT - 1); i++)
    {
        if(lldp_neibor_index[i] != 0)
        {
            continue;
        }
        for(j = i + 1; j < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; j++)
        {
            if(lldp_neibor_index[j] == 0)
            {
                continue;
            }
            lldp_neibor_index[i] = lldp_neibor_index[j];
            lldp_neibor_index[j] = 0;
            break;
        }
        if(j == MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT)
        {
            break;
        }
    }
}

static void
lldp_port_status_down_timeout(UI8_T * u_portId)
{
    UI8_T i = 0;
    UI8_T j = 0;
    MW_LOG_DEBUG(LLDP, "%s[%d]: port is %x", __func__, __LINE__, (*u_portId + 1));
    lldp_clear_port_neighbo_info(u_portId);

    for(i = 0; i < MAX_LLDP_CLIENT_PER_PORT; i++)
    {
        if(port_msap_rx[*u_portId][i] != NULL)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: status down:port %d neighbor %d addr is %p\n", __func__, __LINE__, (*u_portId + 1), i+1, port_msap_rx[*u_portId][i]);
            osapi_memset(port_msap_rx[*u_portId][i]->hash_value, 0, LLDP_HASH_VALUE_LENGTH);
            MW_FREE(port_msap_rx[*u_portId][i]);
            for(j = 0; j < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; j++)
            {
                if(lldp_neibor_index[j] == ((*u_portId) * MAX_LLDP_CLIENT_PER_PORT + i +1))
                {
                    lldp_neibor_index[j] = 0;
                    break;
                }
            }
            lldp_arrange_neibor_index();
        }
        port_msap_rx_count[*u_portId][i] = 0;
    }
}

static void
lldp_port_msap_rx_timeout(
    UI8_T        *u_portId,
    UI8_T        index)
{
    DB_LLDP_CLIENT_INFO_T msap_info;
    UI8_T tmp_index = 0;
    UI8_T j = 0;
    if(port_msap_rx[*u_portId][index] != NULL)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port is %d", __func__, __LINE__, (*u_portId + 1));
        osapi_memset(&msap_info, 0, LLDP_CLIENT_INFO_SIZE);
        tmp_index = (*u_portId) * MAX_LLDP_CLIENT_PER_PORT + index + 1;
        lldp_db_queue_send(M_UPDATE, LLDP_CLIENT_INFO, DB_ALL_FIELDS, tmp_index, &msap_info, LLDP_CLIENT_INFO_SIZE);

        MW_LOG_DEBUG(LLDP, "%s[%d]: rx timeout:port %d neighbor %d addr is %p\n", __func__, __LINE__, (*u_portId + 1), index+1, port_msap_rx[*u_portId][index]);
        osapi_memset(port_msap_rx[*u_portId][index]->hash_value, 0, LLDP_HASH_VALUE_LENGTH);
        MW_FREE(port_msap_rx[*u_portId][index]);
        for(j = 0; j < MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT; j++)
        {
            if(lldp_neibor_index[j] == ((*u_portId) * MAX_LLDP_CLIENT_PER_PORT + index +1))
            {
                lldp_neibor_index[j] = 0;
                break;
            }
        }
        lldp_arrange_neibor_index();
        MW_LOG_DEBUG(LLDP, "%s[%d]: rx timeout free ok\n", __func__, __LINE__);
    }
}

static void
lldp_enable_set(UI8_T enable)
{
    UI8_T i = 0, j = 0;
    UI8_T tmp_port_admin[MAX_PORT_NUM] = {0};
    MW_LOG_DEBUG(LLDP, "%s[%d]: enable=%d", __func__, __LINE__, enable);
    if (enable == lldp_info.global_enable)
    {
        return;
    }

    if (TRUE == enable)
    {
        lldp_info.global_enable = ENABLE;
        lldp_global_cfg = ENABLE;
        lldp_global_tx_count = lldp_info.tx_interval;
        if(lldp_global_tx_count > LLDP_TX_INTERVAL_MAX)
        {
            lldp_global_tx_count = LLDP_TX_INTERVAL_MAX;
            MW_LOG_DEBUG(LLDP, "%s[%d]: global tx %d", __func__, __LINE__, (unsigned int)lldp_global_tx_count);
        }
    }
    else
    {
        lldp_info.global_enable = DISABLE;
        lldp_global_cfg = DISABLE;
        lldp_global_tx_count = 0;
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            port_info_change_tx_count[i] = 0;
            port_status_down_count[i] = 0;
            port_tx_timestamp[i] = 0;
            if ((LLDP_PORT_TX_RX == lldp_info.lldp_admin[i]) || (LLDP_PORT_TX_ONLY == lldp_info.lldp_admin[i]))
            {
                port_send_shutdown_msg[i] = 1;
            }
            for(j = 0; j < MAX_LLDP_CLIENT_PER_PORT; j++)
            {
                port_msap_rx_count[i][j] = 0;
                lldp_port_msap_rx_timeout(&i, j);
            }
            lldp_port_info_change_timeout(&i);
        }
        /*if lldp disable, port lldp status set to disable*/
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            tmp_port_admin[i] = LLDP_PORT_DISABLE;
            lldp_info.lldp_admin[i] = LLDP_PORT_DISABLE;
        }
        MW_LOG_DEBUG(LLDP, "%s[%d]: ->lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, admin=%d)", __func__, __LINE__, LLDP_PORT_DISABLE);
        lldp_db_queue_send(M_UPDATE, LLDP_PORT_INFO, LLDP_ENABLE, DB_ALL_ENTRIES, tmp_port_admin, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    }
    return;
}

static void
lldp_timer(TimerHandle_t xTimer)
{
    MW_MSG_T        msg = {0};
    msg.msg_id = LLDP_MSG_ID_TIMER_NOTIFY;

    if(ENABLE == lldp_info.global_enable)
    {
        if(lldp_notify_msg_count < LLDP_MAX_TIMER_NOTIFY)
        {
            if(MW_E_OK != lldp_timer_send_to_queue(msg, 0))
            {
                MW_LOG_ERROR(LLDP, "%s[%d]: lldp_timer_send_to_queue fail\n", __func__, __LINE__);
            }
            else
            {
                lldp_notify_msg_count++;
                MW_LOG_DEBUG(LLDP, "%s[%d]: lldp notify msg count %d\n", __func__, __LINE__, lldp_notify_msg_count);
            }
        }
        else
        {
            MW_LOG_WARN(LLDP, "%s[%d]: lldp queue has too timer notify to process, not send\n", __func__, __LINE__);
        }
    }
    return;
}

static void
lldp_db_subsribe(void)
{
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, GLOBAL_ENABLE)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, GLOBAL_ENABLE, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_HOLD)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_HOLD, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_INTERVAL)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_INTERVAL, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, REINIT_DELAY)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, REINIT_DELAY, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_DELAY)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_INFO, TX_DELAY, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, LLDP_PORT_INFO)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, LLDP_PORT_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, PORT_ADMIN_STATUS)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES, 0, 0);
#ifdef AIR_SUPPORT_VOICE_VLAN
    /* voice & survei vlan info need to subscribe all fields */
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, VOICE_VLAN_INFO)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, SURVEI_VLAN_INFO)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);
#endif

    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(M_SUBSCRIBE, SYS_INFO)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, SYS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0);

    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(PORT_OPER_INFO, PORT_OPER_STATUS)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, 0, 0);

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
#ifdef AIR_SUPPORT_RSTP
    MW_LOG_INFO(LLDP, "%s[%d]: lldp_db_queue_send(STP_PORT_OPER_INFO, STP_PORT_OPER_STATE)", __func__, __LINE__);
    lldp_db_queue_send(M_SUBSCRIBE, STP_PORT_OPER_INFO, STP_PORT_OPER_STATE, DB_ALL_ENTRIES, 0, 0);
#endif
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */

    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
}

static void
lldp_timer_notify_handle(void)
{
    UI8_T u_portId = 0;
    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
    if(lldp_global_tx_count > 0)
    {
        lldp_global_tx_count--;
        MW_LOG_DEBUG(LLDP, "%s[%d]: global tx %d", __func__, __LINE__, (unsigned int)lldp_global_tx_count);
        if(lldp_global_tx_count == 0)
        {
            lldp_global_tx_timeout();
            lldp_global_tx_count = lldp_info.tx_interval;
            if(lldp_global_tx_count > LLDP_TX_INTERVAL_MAX)
            {
                lldp_global_tx_count = LLDP_TX_INTERVAL_MAX;
            }
        }
    }
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if(PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        UI8_T tmp_port = u_portId;
        if(u_portId > PLAT_CPU_PORT)
        {
            tmp_port -= 1;
        }
        if(tmp_port >= MAX_PORT_NUM)
        {
            continue;
        }
        /*note:u_portId 0 is default cpu port, port from 1 to 28*/
        if(port_info_change_tx_count[tmp_port] > 0)
        {
            port_info_change_tx_count[tmp_port]--;
            if(port_info_change_tx_count[tmp_port] == 0)
            {
                lldp_port_info_change_timeout(&tmp_port);
            }
        }
    }
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if(PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        UI8_T tmp_port = u_portId;
        if(u_portId > PLAT_CPU_PORT)
        {
            tmp_port -= 1;
        }
        if(tmp_port >= MAX_PORT_NUM)
        {
            continue;
        }
        if(port_status_down_count[tmp_port] > 0)
        {
            port_status_down_count[tmp_port]--;
            if(port_status_down_count[tmp_port] == 0)
            {
                lldp_port_status_down_timeout(&tmp_port);
                osapi_delay(50);
            }
        }
    }
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if(PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }
        UI8_T tmp_port = u_portId;
        UI8_T j = 0;
        if(u_portId > PLAT_CPU_PORT)
        {
            tmp_port -= 1;
        }
        if(tmp_port >= MAX_PORT_NUM)
        {
            continue;
        }
        for(j = 0; j < MAX_LLDP_CLIENT_PER_PORT; j++)
        {
            if(port_msap_rx_count[tmp_port][j] > 0)
            {
                port_msap_rx_count[tmp_port][j]--;
                if(port_msap_rx_count[tmp_port][j] == 0)
                {
                    lldp_port_msap_rx_timeout(&tmp_port, j);
                    osapi_delay(20);
                }
            }
        }
    }
    if(lldp_notify_msg_count >= 1)
    {
        lldp_notify_msg_count--;
        MW_LOG_DEBUG(LLDP, "%s[%d]: lldp notify msg count %d\n", __func__, __LINE__, lldp_notify_msg_count);
    }
}

static void
lldp_db_lldp_info_handle(void *ptr_data, DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;
    switch(ptr_type->f_idx)
    {
        case GLOBAL_ENABLE:
            if ((TRUE == (*((UI8_T*)ptr_data))) ||
                (FALSE == (*((UI8_T*)ptr_data))))
            {
                /* lldp admin enable/disable */
                MW_LOG_DEBUG(LLDP, "%s[%d]: lldp global %d", __func__, __LINE__, *((UI8_T*)ptr_data));
                lldp_enable_set(*((UI8_T*)ptr_data));
            }
            else
            {
                MW_LOG_ERROR(LLDP, "%s[%d]: lldp: Invalid parameter - %d", __func__, __LINE__, *((UI8_T*)ptr_data));
            }
            break;
        case TX_HOLD:
            MW_LOG_DEBUG(LLDP, "%s[%d]: lldp tx_hold %d", __func__, __LINE__, *((UI8_T*)ptr_data));
            if(lldp_info.tx_hold != (*((UI8_T*)ptr_data)))
            {
                lldp_info.tx_hold = *((UI8_T*)ptr_data);
                MW_LOG_INFO(LLDP, "%s[%d]: lldp tx_hold %d", __func__, __LINE__, lldp_info.tx_hold);
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
            break;
        case REINIT_DELAY:
            MW_LOG_DEBUG(LLDP, "%s[%d]: lldp reinit_delay %d", __func__, __LINE__, *((UI16_T*)ptr_data));
            if(lldp_info.reinit_delay != (*((UI16_T*)ptr_data)))
            {
                lldp_info.reinit_delay = *((UI16_T*)ptr_data);
                MW_LOG_INFO(LLDP, "%s[%d]: lldp reinit_delay %d", __func__, __LINE__, lldp_info.reinit_delay);
            }
            break;
        case TX_INTERVAL:
            MW_LOG_DEBUG(LLDP, "%s[%d]: lldp tx_interval %d", __func__, __LINE__, *((UI16_T*)ptr_data));
            if(lldp_info.tx_interval != (*((UI16_T*)ptr_data)))
            {
                lldp_info.tx_interval = *((UI16_T*)ptr_data);
                MW_LOG_INFO(LLDP, "%s[%d]: lldp tx_interval %d", __func__, __LINE__, lldp_info.tx_interval);
                lldp_global_tx_count = lldp_info.tx_interval;
                MW_LOG_INFO(LLDP, "%s[%d]: global tx %d", __func__, __LINE__, (unsigned int)lldp_global_tx_count);
                if(lldp_global_tx_count > LLDP_TX_INTERVAL_MAX)
                {
                    lldp_global_tx_count = LLDP_TX_INTERVAL_MAX;
                    MW_LOG_DEBUG(LLDP, "%s[%d]: global tx %d", __func__, __LINE__, (unsigned int)lldp_global_tx_count);
                }
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
            break;
        case TX_DELAY:
            MW_LOG_DEBUG(LLDP, "%s[%d]: lldp tx_delay %d", __func__, __LINE__, *((UI16_T*)ptr_data));
            if(lldp_info.tx_delay != (*((UI16_T*)ptr_data)))
            {
                lldp_info.tx_delay = *((UI16_T*)ptr_data);
                MW_LOG_INFO(LLDP, "%s[%d]: lldp tx_delay %d", __func__, __LINE__, lldp_info.tx_delay);
            }
            break;
        default:
            MW_LOG_DEBUG(LLDP, "%s[%d]: lldp info field idx %d", __func__, __LINE__, ptr_type->f_idx);
            break;
    }
}

static void
lldp_db_lldp_port_info_handle(void *ptr_data, UI16_T data_size, DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;

    if ((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size)
    {
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if(lldp_info.lldp_admin[i] == ((UI8_T*)ptr_data)[i])
            {
                continue;
            }
            else
            {
                UI8_T   tmp_lldp_admin = 0;
                tmp_lldp_admin = ((UI8_T*)ptr_data)[i];
                MW_LOG_DEBUG(LLDP, "%s[%d]: port %d lldp admin %d", __func__, __LINE__, i+1, tmp_lldp_admin);
                if((tmp_lldp_admin == LLDP_PORT_TX_ONLY) || (tmp_lldp_admin == LLDP_PORT_DISABLE))
                {
                    port_status_down_count[i] = 1;
                }
                if(((tmp_lldp_admin == LLDP_PORT_TX_ONLY) || (tmp_lldp_admin == LLDP_PORT_TX_RX)) &&
                    (lldp_info.lldp_admin[i] != LLDP_PORT_TX_ONLY) && (lldp_info.lldp_admin[i] != LLDP_PORT_TX_RX))
                {
                    port_info_change_tx_count[i] = 1;
                }
                if(((tmp_lldp_admin == LLDP_PORT_RX_ONLY) || (tmp_lldp_admin == LLDP_PORT_DISABLE)) &&
                    (lldp_info.lldp_admin[i] != LLDP_PORT_RX_ONLY) && (lldp_info.lldp_admin[i] != LLDP_PORT_DISABLE))
                {
                    port_info_change_tx_count[i] = 1;
                    port_send_shutdown_msg[i] = 1;
                }
                lldp_info.lldp_admin[i] = tmp_lldp_admin;
            }
        }
    }
    else if(sizeof(UI8_T) == data_size)
    {
        MW_LOG_DEBUG(LLDP, "%s[%d]: port %d lldp admin %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
        if (ptr_type->e_idx == 0)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: invalid port %d, skip", __func__, __LINE__, ptr_type->e_idx);
            return;
        }
        if(lldp_info.lldp_admin[ptr_type->e_idx - 1] != (*((UI8_T*)ptr_data)))
        {
            UI8_T   tmp_lldp_admin = 0;
            tmp_lldp_admin = *((UI8_T*)ptr_data);
            MW_LOG_DEBUG(LLDP, "%s[%d]: port %d lldp admin %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
            if((tmp_lldp_admin == LLDP_PORT_TX_ONLY) || (tmp_lldp_admin == LLDP_PORT_DISABLE))
            {
                port_status_down_count[ptr_type->e_idx - 1] = 1;
            }
            /* if change to tx only or tx/rx, need to send lldp packet immediately*/
            if(((tmp_lldp_admin == LLDP_PORT_TX_ONLY) || (tmp_lldp_admin == LLDP_PORT_TX_RX)) &&
                (lldp_info.lldp_admin[ptr_type->e_idx - 1] != LLDP_PORT_TX_ONLY) && (lldp_info.lldp_admin[ptr_type->e_idx - 1] != LLDP_PORT_TX_RX))
            {
                port_info_change_tx_count[ptr_type->e_idx - 1] = 1;
            }
            /* if change to rx only or disabled, need to send shutdown(ttl = 0) lldp packet immediately */
            if(((tmp_lldp_admin == LLDP_PORT_RX_ONLY) || (tmp_lldp_admin == LLDP_PORT_DISABLE)) &&
                (lldp_info.lldp_admin[ptr_type->e_idx - 1] != LLDP_PORT_RX_ONLY) && (lldp_info.lldp_admin[ptr_type->e_idx - 1] != LLDP_PORT_DISABLE))
            {
                port_info_change_tx_count[ptr_type->e_idx - 1] = 1;
                port_send_shutdown_msg[ptr_type->e_idx - 1] = 1;
            }
            lldp_info.lldp_admin[ptr_type->e_idx - 1] = tmp_lldp_admin;
        }
    }
}

static void
lldp_db_port_cfg_info_handle(void *ptr_data, UI16_T data_size, DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;

    if(ptr_type->f_idx == PORT_ADMIN_STATUS)
    {
        if ((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size)
        {
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if(lldp_info.port_admin[i] == ((UI8_T*)ptr_data)[i])
                {
                    continue;
                }
                else
                {
                    lldp_info.port_admin[i] = ((UI8_T*)ptr_data)[i];
                    MW_LOG_INFO(LLDP, "%s[%d]: lldp port %d status %d", __func__, __LINE__, i+1, lldp_info.port_admin[i]);
                    if(TRUE == lldp_info.port_admin[i])
                    {
                        port_status_down_count[i] = 1;
                    }
                }
            }
        }
        else if(sizeof(UI8_T) == data_size)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: port %d admin %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
            if (ptr_type->e_idx == 0)
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: invalid port %d, skip", __func__, __LINE__, ptr_type->e_idx);
                return;
            }
            if(lldp_info.port_admin[ptr_type->e_idx - 1] != (*((UI8_T*)ptr_data)))
            {
                lldp_info.port_admin[ptr_type->e_idx - 1] = *((UI8_T*)ptr_data);
                MW_LOG_DEBUG(LLDP, "%s[%d]: port %d admin %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
                if(TRUE == lldp_info.port_admin[ptr_type->e_idx - 1])
                {
                    port_status_down_count[ptr_type->e_idx - 1] = 1;
                }
            }
        }
    }
}
#ifdef AIR_SUPPORT_VOICE_VLAN
static void
lldp_db_voice_vlan_info_handle(void *ptr_data, UI16_T data_size)
{
    UI8_T i = 0;

    if (sizeof(DB_VOICE_VLAN_INFO_T)  == data_size)
    {
        DB_VOICE_VLAN_INFO_T *ptr_vlan_info = (DB_VOICE_VLAN_INFO_T *)ptr_data;
        MW_LOG_DEBUG(LLDP, "%s[%d]: voice vlan state %d vlan id %d vlan pri %d", __func__, __LINE__, ptr_vlan_info->vlan_state, ptr_vlan_info->vlan_id, ptr_vlan_info->vlan_priority);
        if(lldp_info.voice_vlan_state != ptr_vlan_info->vlan_state)
        {
            lldp_info.voice_vlan_state = ptr_vlan_info->vlan_state;
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                port_info_change_tx_count[i] = 1;
            }
        }
        if(lldp_info.voice_vlan_id != ptr_vlan_info->vlan_id)
        {
            lldp_info.voice_vlan_id = ptr_vlan_info->vlan_id;
            if(TRUE == lldp_info.voice_vlan_state)
            {
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
        }
        if(lldp_info.voice_vlan_pri != ptr_vlan_info->vlan_priority)
        {
            lldp_info.voice_vlan_pri = ptr_vlan_info->vlan_priority;
            if(TRUE == lldp_info.voice_vlan_state)
            {
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
        }
    }
}
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
static void
lldp_db_survei_vlan_info_handle(void *ptr_data, UI16_T data_size)
{
    UI8_T i = 0;

    if (sizeof(DB_SURVEI_VLAN_INFO_T)  == data_size)
    {
        DB_SURVEI_VLAN_INFO_T *ptr_vlan_info = (DB_SURVEI_VLAN_INFO_T *)ptr_data;
        MW_LOG_DEBUG(LLDP, "%s[%d]: survei vlan state %d vlan id %d vlan pri %d", __func__, __LINE__, ptr_vlan_info->vlan_state, ptr_vlan_info->vlan_id, ptr_vlan_info->vlan_priority);
        if(lldp_info.survei_vlan_state != ptr_vlan_info->vlan_state)
        {
            lldp_info.survei_vlan_state = ptr_vlan_info->vlan_state;
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                port_info_change_tx_count[i] = 1;
            }
        }
        if(lldp_info.survei_vlan_id != ptr_vlan_info->vlan_id)
        {
            lldp_info.survei_vlan_id = ptr_vlan_info->vlan_id;
            if(TRUE == lldp_info.survei_vlan_state)
            {
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
        }
        if(lldp_info.survei_vlan_pri != ptr_vlan_info->vlan_priority)
        {
            lldp_info.survei_vlan_pri = ptr_vlan_info->vlan_priority;
            if(TRUE == lldp_info.survei_vlan_state)
            {
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    port_info_change_tx_count[i] = 1;
                }
            }
        }
    }
}
#endif
static void
lldp_db_sys_info_handle(void *ptr_data, UI16_T data_size, DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;
    if(ptr_type->f_idx == DB_ALL_FIELDS)
    {
        DB_SYS_INFO_T * ptr_cfg_sys_info = (DB_SYS_INFO_T *)ptr_data;
        if(osapi_memcmp((const void*)lldp_info.sys_name, ptr_cfg_sys_info->sys_name, MAX_SYS_NAME_SIZE))
        {
            memset(lldp_info.sys_name, 0 , MAX_SYS_NAME_SIZE);
            osapi_memcpy(lldp_info.sys_name, ptr_cfg_sys_info->sys_name, MAX_SYS_NAME_SIZE);
            MW_LOG_INFO(LLDP, "%s[%d]: sysname change to %s", __func__, __LINE__, lldp_info.sys_name);
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                port_info_change_tx_count[i] = 1;
            }
        }
    }
    else if(ptr_type->f_idx == SYS_NAME)
    {
        memset(lldp_info.sys_name, 0 , MAX_SYS_NAME_SIZE);
        osapi_memcpy(lldp_info.sys_name, (C8_T *)ptr_data, data_size);
        MW_LOG_INFO(LLDP, "%s[%d]: sysname change to %s", __func__, __LINE__, lldp_info.sys_name);
        for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            port_info_change_tx_count[i] = 1;
        }
    }
}

static void
lldp_db_port_oper_info_handle(void *ptr_data, UI16_T data_size, DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;
    if(ptr_type->f_idx == PORT_OPER_STATUS)
    {
        if ((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size)
        {
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if(lldp_info.oper_status[i] == ((UI8_T*)ptr_data)[i])
                {
                    continue;
                }
                else
                {
                    lldp_info.oper_status[i] = ((UI8_T*)ptr_data)[i];
                    MW_LOG_INFO(LLDP, "%s[%d]: lldp port %d oper status %d", __func__, __LINE__, i+1, lldp_info.oper_status[i]);
                    if(TRUE == lldp_info.oper_status[i])
                    {
                        port_info_change_tx_count[i] = 1;
                        port_status_down_count[i] = 1;
                    }
                }
            }
        }
        else if (sizeof(UI8_T) == data_size)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: port %d oper status %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
            if (ptr_type->e_idx == 0)
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: invalid port %d, skip", __func__, __LINE__, ptr_type->e_idx);
                return;
            }
            if(lldp_info.oper_status[ptr_type->e_idx - 1] != (*((UI8_T*)ptr_data)))
            {
                lldp_info.oper_status[ptr_type->e_idx - 1] = *((UI8_T*)ptr_data);
                MW_LOG_DEBUG(LLDP, "%s[%d]: port %d oper status %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
                if(TRUE == lldp_info.oper_status[ptr_type->e_idx - 1])
                {
                    port_info_change_tx_count[ptr_type->e_idx - 1] = 1;
                    port_status_down_count[ptr_type->e_idx - 1] = 1;
                }
            }
        }
    }
}

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
#ifdef AIR_SUPPORT_RSTP
static void
lldp_db_rstp_port_oper_info_handle(
    void *ptr_data,
    UI16_T data_size,
    DB_REQUEST_TYPE_T *ptr_type)
{
    UI8_T i = 0;

    if(STP_PORT_OPER_STATE == ptr_type->f_idx)
    {
        if ((sizeof(UI8_T) * PLAT_MAX_PORT_NUM) == data_size)
        {
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if(lldp_info.rstp_status[i] == ((UI8_T*)ptr_data)[i])
                {
                    continue;
                }
                else
                {
                    lldp_info.rstp_status[i] = ((UI8_T*)ptr_data)[i];
                    MW_LOG_INFO(LLDP, "%s[%d]: lldp port %d rstp port oper status %d", __func__, __LINE__, i+1, lldp_info.rstp_status[i]);
                }
            }
        }
        else if (sizeof(UI8_T) == data_size)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: port %d rstp port oper status %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
            if (ptr_type->e_idx == 0)
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: invalid port %d, skip", __func__, __LINE__, ptr_type->e_idx);
                return;
            }
            if(lldp_info.rstp_status[ptr_type->e_idx - 1] != (*((UI8_T*)ptr_data)))
            {
                lldp_info.rstp_status[ptr_type->e_idx - 1] = *((UI8_T*)ptr_data);
                MW_LOG_DEBUG(LLDP, "%s[%d]: port %d rstp port oper status %d", __func__, __LINE__, ptr_type->e_idx, *((UI8_T*)ptr_data));
            }
        }
    }
}
#endif
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */

static MW_ERROR_NO_T
lldp_db_msg_process(
    UI8_T method,
    DB_REQUEST_TYPE_T *ptr_type,
    UI16_T data_size,
    void * ptr_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T f_num = 0;

    MW_CHECK_PTR(ptr_type);
    MW_PARAM_CHK((ptr_type ->t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    rc = dbapi_getFieldsNum((UI8_T)ptr_type ->t_idx, &f_num);
    if(MW_E_OK != rc)
    {
        return rc;
    }
    MW_PARAM_CHK((ptr_type ->f_idx >= f_num), MW_E_BAD_PARAMETER);
    if(data_size > 0)
    {
        MW_CHECK_PTR(ptr_data);
    }
    switch(method)
    {
        case M_GET:
        case M_UPDATE:
            if(LLDP_INFO == ptr_type ->t_idx)
            {
                lldp_db_lldp_info_handle(ptr_data, ptr_type);
            }
            else if(LLDP_PORT_INFO == ptr_type ->t_idx)
            {
                lldp_db_lldp_port_info_handle(ptr_data, data_size, ptr_type);
            }
            else if(PORT_CFG_INFO == ptr_type ->t_idx)
            {
                lldp_db_port_cfg_info_handle(ptr_data, data_size, ptr_type);
            }
#ifdef AIR_SUPPORT_VOICE_VLAN
            else if(VOICE_VLAN_INFO == ptr_type ->t_idx)
            {
                lldp_db_voice_vlan_info_handle(ptr_data, data_size);
            }
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
            else if (SURVEI_VLAN_INFO == ptr_type ->t_idx)
            {
                lldp_db_survei_vlan_info_handle(ptr_data, data_size);
            }
#endif
            else if (SYS_INFO == ptr_type ->t_idx)
            {
                lldp_db_sys_info_handle(ptr_data, data_size, ptr_type);
            }
            else if(PORT_OPER_INFO == ptr_type ->t_idx)
            {
                lldp_db_port_oper_info_handle(ptr_data, data_size, ptr_type);
            }
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
#ifdef AIR_SUPPORT_RSTP
            else if(STP_PORT_OPER_INFO == ptr_type ->t_idx)
            {
                lldp_db_rstp_port_oper_info_handle(ptr_data, data_size, ptr_type);
            }
#endif
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */
            break;
        default:
            MW_LOG_DEBUG(LLDP, "%s[%d]: recv method: [%02X]", __func__, __LINE__, method);
            break;
    }
    return MW_E_OK;
}

static void
_lldpRxProcess(
    void * ptr_pvParameters)
{
    MW_ERROR_NO_T xRet = MW_E_OK;
    NET_MSG_T     *ptr_data = NULL;
    DB_MSG_T      *ptr_msg = NULL;
    struct pbuf   *ptr_pb = NULL;

    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
    (void)ptr_pvParameters;

    /* Check DB is ready or not */
    do{
        xRet = dbapi_dbisReady();
    }while(MW_E_OK != xRet);
    MW_LOG_DEBUG(LLDP, "%s[%d]: db is ready", __func__, __LINE__);
    lldp_db_subsribe();
    while(1)
    {
        xRet = osapi_msgRecv(LLDP_QUEUE_NAME, (UI8_T **)&ptr_data, 0, 500);
        if (MW_E_OK != xRet)
        {
            continue;
        }
        if(ptr_data->msg_id == MW_MSG_ID_ETHERNET_PBUF)
        {
            /* Recv: mac_rcv */
            ptr_pb = (struct pbuf *)(ptr_data->ptr_pbuf);
            MW_FREE(ptr_data);
            lldp_recv(ptr_pb);
            MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
        }
        else if(ptr_data->msg_id == LLDP_MSG_ID_TIMER_NOTIFY)
        {
            MW_LOG_DEBUG(LLDP, "%s[%d]: recv lldp timer notify msg, osapi_free(ptr_data %p)", __func__, __LINE__, ptr_data);
            MW_FREE(ptr_data);
            lldp_timer_notify_handle();
        }
        else if(ptr_data->msg_id == MW_MSG_ID_DB)
        {
            UI32_T i = 0;
            DB_REQUEST_TYPE_T request = {0};
            UI16_T data_size = 0;
            UI8_T *ptr_msg_data = NULL;
            UI8_T *ptr_payload_data = NULL;

            ptr_msg = (DB_MSG_T *)ptr_data;
            MW_LOG_DEBUG(LLDP, "%s[%d]: recv method - %02X", __func__, __LINE__, ptr_msg->method);
            if((ptr_msg->method == M_ACK) || (ptr_msg->method == M_B_RESPONSE))
            {
                MW_LOG_DEBUG(LLDP, "%s[%d]: result code - %d", __func__, __LINE__, ptr_msg->type.result);
            }
            else
            {
                /*Process the notification message*/
                do{
                    xRet = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_msg_data, &ptr_payload_data);
                    if(MW_E_OK == xRet)
                    {
                        MW_LOG_DEBUG(LLDP, "%s[%d]: index=%u", __func__, __LINE__, i++);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: ptr_payload=%p", __func__, __LINE__, ptr_msg_data);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: t_idx=%u", __func__, __LINE__, request.t_idx);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: f_idx=%u", __func__, __LINE__, request.f_idx);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: e_idx=%u", __func__, __LINE__, request.e_idx);
                        MW_LOG_DEBUG(LLDP, "%s[%d]: data_size=%u", __func__, __LINE__, data_size);
                        xRet = lldp_db_msg_process(ptr_msg->method,
                                               &request,
                                               data_size,
                                               ptr_msg_data);
                        if (MW_E_OK != xRet)
                        {
                            MW_LOG_DEBUG(LLDP, "%s[%d]: process db msg failed!(%d)", __func__, __LINE__, xRet);
                        }
                    }
                }while((MW_E_OK == xRet) && (NULL != ptr_payload_data));
            }
            MW_LOG_DEBUG(LLDP, "%s[%d]: MW_FREE(ptr_data %p).", __func__, __LINE__, ptr_data);
            MW_FREE(ptr_data);
        }
        else
        {
            /*unknown msg*/
            MW_FREE(ptr_data);
        }
    }
    MW_LOG_ERROR(LLDP, "%s[%d]: could not reach here, there is something wrong", __func__, __LINE__);
}

MW_ERROR_NO_T
lldp_init_timer()
{
    MW_ERROR_NO_T ret = MW_E_OK;
    if (osapi_timerCreate("lldp Tmr", lldp_timer, TRUE, LLDP_TMR_INTERVAL, 0, &lldp_timer_handle) != MW_E_OK)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_timerCreate lldp Tmr failed!", __func__, __LINE__);
        return MW_E_NO_MEMORY;
    }
    ret = osapi_timerStart(lldp_timer_handle);
    if (ret != MW_E_OK)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_timerStart lldp Tmr failed, ret %d!", __func__, __LINE__, ret);
        if (osapi_timerDelete(lldp_timer_handle) != MW_E_OK)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: osapi_timerDelete lldp Tmr failed !", __func__, __LINE__);
        }
        return MW_E_OTHERS;
    }

    MW_LOG_INFO(LLDP, "%s[%d]: osapi_timerStart lldp Tmr OK !", __func__, __LINE__);
    return MW_E_OK;
}

MW_ERROR_NO_T
lldp_init(void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    I32_T ret2 = 0;

    if(NULL != _ptr_lldp)
    {
        return MW_E_ALREADY_INITED;
    }

    MW_LOG_INIT_PRINTF("Initializing LLDP module...\n");
    /* Create message queue to receive both DB and Socket and Task message */
    ret = osapi_msgCreateStatic(LLDP_QUEUE_NAME,
                                LLDP_QUEUE_LENGTH,
                                sizeof(MW_MSG_T *),
                                (UI8_T *)lldp_queue_ctx.msgQueueStorage,
                                &lldp_queue_ctx.msg_queue_handle);
    if (ret != MW_E_OK)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: osapi_msgCreateStatic %s fail", __func__, __LINE__, LLDP_QUEUE_NAME);
        return ret;
    }
    lldp_q_handle = osapi_msgFindHandle(LLDP_QUEUE_NAME);
    if (NULL == lldp_q_handle)
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: lldp_q_handle is NULL", __func__, __LINE__);
        lldp_free_resource();
        ret = MW_E_OTHERS;
        return ret;
    }

    if (osapi_malloc(sizeof(NET_FILTER_T), LLDP_MODULE_NAME, (void**)&lldp_pkt_netf) == MW_E_OK)
    {
        osapi_memset(lldp_pkt_netf->name, 0, MSG_MAX_NAME_LEN);
        ret2 = osapi_snprintf((C8_T*)lldp_pkt_netf->name, 4, "%s", (C8_T*)LLDP_QUEUE_NAME);
        if(ret2 < 0)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: osapi_snprintf error", __func__, __LINE__);
        }
        lldp_pkt_netf->protocol = PROTO_LLDP;
        lldp_pkt_netf->state = NET_FILTER_REGISTER;
        lldp_pkt_netf->handle = lldp_q_handle;

        ret = osapi_netRegister(lldp_pkt_netf);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LLDP, "%s[%d]: register lldp pkt netf fail", __func__, __LINE__);
            lldp_free_resource();
            return ret;
        }
    }
    else
    {
        MW_LOG_ERROR(LLDP, "%s[%d]: mallooc lldp_pkt_netf fail", __func__, __LINE__);
        lldp_free_resource();
        return MW_E_NO_MEMORY;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
    lldp_cli_queue_init();
    lldp_global_var_init();
    lldp_mgmt_attr_init();
    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
    ret = osapi_threadCreateStatic(LLDP_TASK_NAME,
                                   LLDP_STACK_SIZE,
                                   MW_TASK_PRIORITY_LLDP,
                                   _lldpRxProcess,
                                   NULL,
                                   lldp_task_stack,
                                   &lldp_task_tcb,
                                   &_ptr_lldp);
    if(MW_E_OK != ret)
    {
        /* Error create task */
        MW_LOG_ERROR(LLDP, "%s[%d]: Error: create lldp task fail\n", __func__, __LINE__);
        lldp_free_resource();
        return ret;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: _lldpRxProcess create success ", __func__, __LINE__);
    if (lldp_init_timer() != MW_E_OK)
    {
        lldp_free_resource();
        return MW_E_OTHERS;
    }
    MW_LOG_DEBUG(LLDP, "%s[%d]: ", __func__, __LINE__);
    return ret;
}

