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

/* FILE NAME:  syncd_in.h
 * PURPOSE:
 *      It provides syncd internal moudle functions.
 *
 * NOTES:
 */

#ifndef _SYNCD_IN_H_
#define _SYNCD_IN_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"
#include "osapi_memory.h"
#include "osapi_string.h"
#include "syncd_timer.h"
#include "syncd_log.h"
#include "syncd_api.h"
#include "syncd_queue.h"

#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
*/
/* Owner */
#define SYNCD_NAME              "SyncD"

#define SYNCD_DELAY_TIME        (10)

/* MACRO FUNCTION DECLARATIONS
*/
#define SYNCD_ASPRINTF(ptr, str)    \
({                                  \
    MW_ERROR_NO_T rc;               \
    rc = osapi_calloc(              \
        (osapi_strlen(str)+1),      \
        SYNCD_NAME,                 \
        (void **)&(ptr));           \
    if (MW_E_OK == rc)              \
    {                               \
        osapi_sprintf(ptr, str);    \
    }                               \
    rc;                             \
})

/* DATA TYPE DECLARATIONS
*/
typedef struct SYNCD_PORT_S
{
    AIR_PORT_STATUS_T  port_status; /* Port operation state */
    UI32_T port_settings;  /* Port operational settings */
}SYNCD_PORT_T;

/* syncd global configuration */
typedef struct SYNCD_CFG_S
{
    /* process */
    /* It must be 4bytes aligned. */
    StackType_t stack_buffer[AIR_MAX_SYNCD_STACK_SIZE];
    StaticTask_t task_buffer;
    threadhandle_t proc_handle;

    /* Timer */
    SYNCD_TIMER_T tmr;     /* Port status & MIB Counter */

    /* Port status including loop state and operational settings. */
    SYNCD_PORT_T *ptr_port;

#ifndef AIR_LITE_MW
    /* API table */
    SYNCD_API_TB_T api_tid[TABLES_LAST];
#endif
}SYNCD_CFG_T;

typedef struct SYNCD_VLAN_GLOBAL_S
{
    UI32_T       blockpbmp;
    UI32_T       portmatrix[MAX_PORT_NUM + 1];
}SYNCD_VLAN_GLOBAL_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* syncd internal functions */
//===========================
/* syncd_queue.c */
/* FUNCTION NAME: syncd_queue_init
 * PURPOSE:
 *      Initialize DB communication message receiver.
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
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_init(
    void);

/* FUNCTION NAME: syncd_queue_free
 * PURPOSE:
 *      Release all allocated memory in syncd queue.
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
syncd_queue_free(
    void);

/* FUNCTION NAME: syncd_queue_recv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer to the message buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_recv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T timeout);

/* FUNCTION NAME: syncd_queue_db_send
 * PURPOSE:
 *      Package the message and send it to DB. If ptr_queue_name is NULL, DB
 *      will not send any response and the message itself will be freed by DB.
 *
 * INPUT:
 *      ptr_queue_name   --  A pointer to the queue that DB will return a
 *                           response to. If it is NULL, no response is expected.
 *      method           --  the method bitmap
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      e_idx            --  the entry index in the table
 *      ptr_data         --  A pointer to the message data
 *      timeout          --  The maximun amout of time the thread will wait for
 *                           send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
syncd_queue_db_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI32_T timeout);

/* FUNCTION NAME: syncd_queue_db_appendMsg
 * PURPOSE:
 *      Append db msg
 *
 * INPUT:
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      e_idx            --  the entry index in the table
 *      ptr_data         --  A pointer to the message data
 *
 * OUTPUT:
 *      ptr_msg           -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      msg_size          -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      ptr_payload       -- A double pointer returns the position of the
 *                                 payload to store the next request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_db_appendMsg(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    DB_MSG_T  **ptr_msg,
    UI16_T     *msg_size,
    UI8_T     **ptr_payload);

/* FUNCTION NAME: syncd_queue_db_getData
 * PURPOSE:
 *      Get the raw data based on t_idx, f_idx and e_idx from DB. It is a
 *      blocking API.
 *
 * INPUT:
 *      method          --  The method of the getting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a db message received from DB
 *      ptr_data_size   --  A pointer returns the size of raw data obtained from DB
 *      pptr_data       --  A double pointer returns the raw data obtained from DB
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. If MW_E_OK is returned, the db message returned by pptr_msg need be
 *      freed after use.
 *      2. dbapi_parseMsg() cannot be used for the message returned by
 *         pptr_msg.
 */
MW_ERROR_NO_T
syncd_queue_db_getData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_data_size,
    void **pptr_data);

/* FUNCTION NAME: syncd_queue_db_setData
 * PURPOSE:
 *      Set the raw data based on t_idx, f_idx and e_idx to DB. If
 *      SYNCD_QUEUE_DB_BLOCK is set to the block input parameter, it will work
 *      as a blocking API.
 *
 * INPUT:
 *      method          --  The method of the setting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *      ptr_data        --  A pointer to the raw data to be set
 *      block           --  Indicate if a response from DB is wanted or waited for
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      ptr_data need be freed after the function returns if it is allocated
 *      from heap and will not be used any more.
 */
MW_ERROR_NO_T
syncd_queue_db_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block);

/* FUNCTION NAME: syncd_queue_db_getMultiData
 * PURPOSE:
 *      Get multiple raw data from DB indicated by the payload of pptr_msg
 *      inputted.
 *
 * INPUT:
 *      pptr_msg        --  A double pointer points to a DB message with one or
 *                          more requests
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a DB message with data
 *                          received from DB for the single or multiple requests
 *      ptr_data_size   --  A pointer returns the size of data received for the
 *                          first request
 *      pptr_data       --  A double pointer returns the data received for the
 *                          first request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      If any error occors, the message pointed by *pptr_msg will be freed
 *      before the function returns. As a result the message pointed by *pptr_msg
 *      need be freed after use only when MW_E_OK is returned.
 */
MW_ERROR_NO_T
syncd_queue_db_getMultiData(
    DB_MSG_T **pptr_msg);

/* FUNCTION NAME: syncd_queue_db_setMultiData
 * PURPOSE:
 *      Set multiple raw data to DB based on the payload of ptr_msg inputted.
 *
 * INPUT:
 *      ptr_msg         --  A pointer points to a DB message with one or more
 *                          requests
 *      block           --  Indicate if a response from DB is wanted and how to
 *                          receive the response
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      The message pointed by *pptr_msg need not be freed no matter what error
 *      code is returned. It will be freed by SYNCD or DB.
 */
MW_ERROR_NO_T
syncd_queue_db_setMultiData(
    DB_MSG_T *ptr_msg,
    const SYNCD_QUEUE_DB_BLOCK_ENUM block);

/* FUNCTION NAME: syncd_queue_task_sendMsg
 * PURPOSE:
 *      Send a message to the SYNCD task.
 *
 * INPUT:
 *      ptr_msg         --  A pointer to the message buffer
 *      solo_msg        --  Indicate if there is at most one message in the queue
 *                          for a specific msg_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_queue_task_sendMsg(
    MW_MSG_T *ptr_msg,
    BOOL_T solo_msg);

#ifdef SYNCD_TEST_ENABLE
void
syncd_queue_test_handleTestMsg(
    SYNCD_MSG_TEST_REQ_T *ptr_msg);

void
syncd_queue_test_taskQueue(
    void);

void
syncd_queue_test_dbSingleData(
    void);

void
syncd_queue_test_dbSetMultiData(
    void);

void
syncd_queue_test_dbGetMultiData(
    void);

void
syncd_queue_test_dbGetMultiDataByReq(
    void);
#endif /* SYNCD_TEST_ENABLE */

//=========================================================
/* syncd_timer.c */
/* FUNCTION NAME:   syncd_port_status_handle
 * PURPOSE
 *       Handle port status.
 *
 * INPUT:
 *      ptr_cfg       --  A pointer points to port status msg
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_port_status_handle(
    SYNCD_CFG_T *ptr_cfg);

/* FUNCTION NAME:   syncd_stp_flush_fdb_handle
 * PURPOSE
 *       Handle stp flush fdb.
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
syncd_stp_flush_fdb_handle();

/* FUNCTION NAME:   syncd_port_matrix_resume_handle
 * PURPOSE
 *       Handle port matrix resuming.
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
syncd_port_matrix_resume_handle();

/* FUNCTION NAME:   syncd_timer_init
 * PURPOSE
 *      Initialize all timer in syncd.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_timer_init(
    SYNCD_CFG_T *ptr_cfg);

/* FUNCTION NAME:   syncd_timer_free
 * PURPOSE:
 *      Delete all timer in syncd
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
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
syncd_timer_free(
    SYNCD_CFG_T *ptr_cfg);

/* FUNCTION NAME:   syncd_mib_set_clear_flag
 * PURPOSE:
 *      set clear flag of high 32bit of mib counter in syncd.
 *
 * INPUT:
 *      port                 -- Index of port number
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
syncd_mib_set_clear_flag(
     UI32_T port);

//=========================================================
/* syncd_api.c */
/* FUNCTION NAME: syncd_api_is_subscribed
 * PURPOSE:
 *      Subscribe each item to DB.
 *
 * INPUT:
 *      t_idx       --  table index
 *      f_idx       --  field index
 *      e_idx       --  entry index
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
BOOL_T
syncd_api_is_subscribed(
    UI8_T t_idx,
    UI8_T f_idx,
    UI16_T e_idx);
/* FUNCTION NAME: syncd_api_subscribe
 * PURPOSE:
 *      Subscribe each item to DB.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *      method      --  M_SUBSCRIBE:    Subscribe all field in API function table
 *                      M_UNSUBSCRIBE:  Unsubscribe all field in API function table
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_subscribe(
    SYNCD_CFG_T *ptr_cfg,
    const UI8_T method);

/* FUNCTION NAME: syncd_api_process
 * PURPOSE:
 *      Process API function of reveiced DB notify message
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *      method      --  the method bitmap
 *      ptr_type    --  pointer to request type which includes table-id, field-id and entry-id
 *      ptr_data    --  pointer to message data
 *      data_size   --  size of ptr_data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_process(
    SYNCD_CFG_T *ptr_cfg,
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_type,
    const UI16_T data_size,
    const void * ptr_data);

/* FUNCTION NAME: syncd_api_init
 * PURPOSE:
 *      Initialize function index of API function table.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */

MW_ERROR_NO_T
syncd_api_init(
    SYNCD_CFG_T *ptr_cfg);

/* FUNCTION NAME:   syncd_api_free
 * PURPOSE:
 *      Unsubscribe all field in API function table.
 *      Release all allocated memory in syncd queue.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
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
MW_ERROR_NO_T
syncd_api_free(
    SYNCD_CFG_T *ptr_cfg);

/* FUNCTION NAME: syncd_api_setPortAdmin
 * PURPOSE:
 *      Set admin status of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *      admin           --  port admin status
 *
 * OUTPUT:
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_setPortAdmin(
    const UI8_T p_idx,
    const BOOL_T admin);

/* FUNCTION NAME: syncd_api_getPortAdmin
 * PURPOSE:
 *      Get admin status of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *
 * OUTPUT:
 *      ptr_admin       --  port admin status
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_getPortAdmin(
    const UI8_T p_idx,
    BOOL_T * const ptr_admin);

/* FUNCTION NAME: syncd_api_setPortLpBlock
 * PURPOSE:
 *      Set loop prevention blocking state of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *      lp_block        --  Loop prevention blocking state
 *
 * OUTPUT:
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_setPortLpBlock(
    const UI8_T p_idx,
    const BOOL_T lp_block);

/* FUNCTION NAME: syncd_api_getPortLpBlock
 * PURPOSE:
 *      Get loop prevention blocking state of specific port
 *
 * INPUT:
 *      p_idx           --  Port idx, start from 0
 *
 * OUTPUT:
 *      ptr_lp_block    --  Loop prevention blocking state
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_getPortLpBlock(
    const UI8_T p_idx,
    BOOL_T * const ptr_lp_block);

#endif  /*_SYNCD_IN_H_*/
