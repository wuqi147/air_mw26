/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:  sfp_db.c
 * PURPOSE:
 *      DB operation for SFP.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal.h>
#include <air_init.h>
#include "sfp_db.h"
#include "sfp_util.h"
#include "sfp_task.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_DB_QUEUE_CLI         "SFD"
#define SFP_DB_QUEUE_LEN         (1)
#define SFP_DB_QUEUE_TIMEOUT     (0xFFFFFFFF)
#define SFP_DB_ACCEPTMBOX_SIZE   (4)

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

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_sfp_queue_sendMessageToDB(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    I32_T rc = dbapi_dbisReady();

    if (MW_E_OK != rc)
    {
        return AIR_E_OP_INVALID;
    }

    rc = dbapi_sendRequesttoDb(size, ptr_msg);
    if (MW_E_OK != rc)
    {
        SFP_LOG_ERROR("%s failed", __func__);
    }

    return MW_E_OK == rc ? AIR_E_OK : AIR_E_OP_INCOMPLETE;
}

/* FUNCTION NAME:   _sfp_db_queue_send
 * PURPOSE:
 *      Generate a DB message and send it to DB queue.
 *
 * INPUT:
 *      method              -- The method bitmap
 *      t_idx               -- The enum of the table
 *      f_idx               -- The enum of the field
 *      e_idx               -- The entry index in the table
 *      ptr_data            -- The pointer to the message data
 *      size                -- The size of the message data pointed by ptr_data
 * OUTPUT:
 *      pptr_out_msg        -- A double pointer to the db message generated
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NO_MEMORY
 *      AIR_E_OP_INVALID
 *      AIR_E_OP_INCOMPLETE
 *
 * NOTES:
 *      When AIR_E_OK is returned, pptr_out_msg should be freed when it is not needed.
 */
static AIR_ERROR_NO_T
_sfp_db_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg)
{
    I32_T rc = AIR_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    DB_PAYLOAD_T *ptr_payload = NULL;
    UI32_T msg_size;

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(msg_size, SFP_DB_QUEUE_CLI, (void**)&ptr_msg);
    if (MW_E_OK != rc)
    {
        SFP_LOG_ERROR("%s ret: %d", __func__, rc);
        return AIR_E_NO_MEMORY;
    }

    /* message */
    osapi_strncpy(ptr_msg->cq_name, SFP_DB_QUEUE_CLI, DB_Q_NAME_SIZE);
    ptr_msg->method = method;
    ptr_msg->type.count = 1;

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    if ((size > 0) && (method != M_GET))
    {
        osapi_memcpy(&(ptr_payload->ptr_data), ptr_data, size);
    }

    /* Send message to DB */
    rc = _sfp_queue_sendMessageToDB(ptr_msg, msg_size);
    if (AIR_E_OK != rc)
    {
        osapi_free(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    return AIR_E_OK;
}

/* FUNCTION NAME:   _sfp_db_queue_recv
 * PURPOSE:
 *      Receive a DB communication message from DB queue.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      pptr_buf             -- A double pointer to the received message from DB queue
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_db_queue_recv(
    void **pptr_buf)
{
    I32_T rc;
    UI8_T *ptr_msg = NULL;

    rc = osapi_msgRecv(SFP_DB_QUEUE_CLI,
                       &ptr_msg,
                       0,
                       SFP_DB_QUEUE_TIMEOUT);
    if (MW_E_OK == rc)
    {
        (*pptr_buf) = ptr_msg;
    }
    else
    {
        rc = AIR_E_OP_INCOMPLETE;
        SFP_LOG_ERROR("%s ret:%d", __func__, rc);
    }

    return rc;
}

/* FUNCTION NAME:   _sfp_db_queue_setData
 * PURPOSE:
 *      Generate a DB message and send it to DB queue directly.
 *
 * INPUT:
 *      method              -- The method bitmap
 *      t_idx               -- The enum of the table
 *      f_idx               -- The enum of the field
 *      e_idx               -- The entry index in the table
 *      ptr_data            -- A pointer to the message data
 *      size                -- The size of ptr_data
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NO_MEMORY
 *      AIR_E_OP_INVALID
 *      AIR_E_OP_INCOMPLETE
 *
 * NOTES:
 *      The input parameters are depend on structure of DB.
 */
static AIR_ERROR_NO_T
_sfp_db_queue_setData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size)
{
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    DB_MSG_T *ptr_msg = NULL;

    rc = _sfp_db_queue_send(method, t_idx, f_idx, e_idx, ptr_data, size, &ptr_msg);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* wait for respond messgae and free ptr_msg */
    rc = _sfp_db_queue_recv((void **)&ptr_msg);

    osapi_free(ptr_msg);
    return rc;
}

static AIR_ERROR_NO_T
_sfp_db_updatePortMode(
    const UI8_T port_mode,
    const UI32_T port_num)
{
    if (0 == port_num)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* set port_mode*/
    return _sfp_db_queue_setData(M_UPDATE, PORT_OPER_INFO, PORT_OPER_MODE, port_num, &port_mode, sizeof(UI8_T));
}

static AIR_ERROR_NO_T
_sfp_db_getPortMode(
    UI8_T *ptr_port_mode,
    const UI8_T port_num)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    DB_MSG_T *ptr_db_msg = NULL;
    UI16_T db_size = 0;
    void *ptr_db_data = NULL;

    if ((NULL == ptr_port_mode) || (0 == port_num))
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = sfp_db_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE, port_num, &ptr_db_msg, &db_size, &ptr_db_data);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    memcpy(ptr_port_mode, ptr_db_data, db_size);
    osapi_free(ptr_db_msg);
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_db_updatePortMode
 * PURPOSE:
 *      Update combo_mode and serdes_mode flags of port_oper_mode DB field.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 *      combo_mode           -- The new combo mode to be updated
 *      serdes_mode          -- The new serdes mode to be updated
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *      AIR_E_NOT_INITED     -- Not initialized
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
sfp_db_updatePortMode(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    AIR_PORT_COMBO_MODE_T combo_mode,
    AIR_PORT_SERDES_MODE_T serdes_mode)
{
    UI8_T port_mode = 0;
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;

    ret = _sfp_db_getPortMode(&port_mode, ptr_port_info->port);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    /* Update COMBO mode */
    if ((TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port)) &&
        (FALSE == sfp_port_is_pureComboSerdesPort(unit, ptr_port_info->port)))
    {
        port_mode &= ~((PORT_MODE_COMBO_MODE_BITMASK) << PORT_MODE_COMBO_MODE_OFFSET);
        port_mode |= (combo_mode & PORT_MODE_COMBO_MODE_BITMASK) << PORT_MODE_COMBO_MODE_OFFSET;
    }

    /* Update SERDES mode */
    port_db_updateSerdesModeFlags(&port_mode, serdes_mode);

    SFP_LOG_DEBUG("update port_mode:%x to DB for port:%d", port_mode, ptr_port_info->port);
    return _sfp_db_updatePortMode(port_mode, ptr_port_info->port);
}

/* FUNCTION NAME:   sfp_db_updateSFPInfo
 * PURPOSE:
 *      Update the information of an SFP module to DB.
 *
 * INPUT:
 *      index                -- The entry ID
 *      ptr_module_info_db   -- A pointer to DB data
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_db_updateSFPInfo(
    const UI16_T index,
    DB_SFP_MODULE_INFO_T *ptr_module_info_db)
{
    if (NULL == ptr_module_info_db)
    {
        return AIR_E_BAD_PARAMETER;
    }

    return _sfp_db_queue_setData(M_UPDATE, SFP_MODULE_INFO, DB_ALL_FIELDS, index, ptr_module_info_db, sizeof(DB_SFP_MODULE_INFO_T));
}

/* FUNCTION NAME:   sfp_db_queue_create
 * PURPOSE:
 *      Create a queue to communicate with DB queue.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_db_queue_create(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;

    /* message CLI name */
    ret = osapi_msgCreate(SFP_DB_QUEUE_CLI,
                          SFP_DB_QUEUE_LEN,
                          SFP_DB_ACCEPTMBOX_SIZE);

    return MW_E_OK == ret ? AIR_E_OK : AIR_E_OP_INCOMPLETE;
}

/* FUNCTION NAME:   sfp_db_queue_free
 * PURPOSE:
 *      Free the queue created by sfp_db_queue_create().
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *
 * NOTES:
 *      Refer to sfp_db_queue_create().
 */
AIR_ERROR_NO_T
sfp_db_queue_free(
    void)
{
    osapi_msgDelete(SFP_DB_QUEUE_CLI);
    return AIR_E_OK;
}

/* FUNCTION NAME:   sfp_db_queue_getData
 * PURPOSE:
 *      1. Calculate db data size based on tid,fid,eid and then alloc memory
 *      2. Send a message to DB queue to get data and receive data from DB queue
 *
 * INPUT:
 *      t_idx               -- The enum of the table
 *      f_idx               -- The enum of the field
 *      e_idx               -- The entry index in the table
 * OUTPUT:
 *      pptr_out_msg        -- A double pointer to the received DB message
 *      ptr_out_size        -- A pointer to the size of ptr_data
 *      pptr_out_data       -- A double pointer to the DB data of the DB message
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INVALID     -- Operation is invalid
 *
 * NOTES:
 *      When return AIR_E_OK, caller need to free the memory which is pointed by *pptr_out_msg!
 */
AIR_ERROR_NO_T
sfp_db_queue_getData(
    const UI8_T in_t_idx,
    const UI8_T in_f_idx,
    const UI16_T in_e_idx,
    DB_MSG_T **pptr_out_msg,
    UI16_T *ptr_out_size,
    void **pptr_out_data)
{
    I32_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T total_size = 0;
    DB_PAYLOAD_T *ptr_pload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = in_t_idx,
        .f_idx = in_f_idx,
        .e_idx = in_e_idx
    };

    rc = dbapi_getDataSize(request, &total_size);
    if (MW_E_OK != rc)
    {
       SFP_LOG_ERROR("dbapi_getDataSize failed(%d)", rc);
       return AIR_E_OP_INCOMPLETE;
    }

    rc = _sfp_db_queue_send(M_GET, in_t_idx, in_f_idx, in_e_idx, NULL, total_size, &ptr_msg);
    if (AIR_E_OK != rc)
    {
       return rc;
    }

    /* wait for DB response messgae */
    rc = _sfp_db_queue_recv((void **)&ptr_msg);
    if(AIR_E_OK != rc)
    {
        osapi_free(ptr_msg);
        return rc;
    }

    (*pptr_out_msg) = ptr_msg;
    (*ptr_out_size) = total_size;

    ptr_pload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    (*pptr_out_data) = &(ptr_pload->ptr_data);

    return AIR_E_OK;
}

