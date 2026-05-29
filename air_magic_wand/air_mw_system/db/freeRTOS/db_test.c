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

/* FILE NAME:  db_test.c
 * PURPOSE:
 *      DB test tasks
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "db_api.h"
#include "db_main.h"
#include "osapi_message.h"
#include "osapi_memory.h"
#include "osapi_string.h"
#include "osapi_thread.h"
#include "osapi_mutex.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MAX_TEST_CASES    (5)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
UI8_T test_subscribed;
UI8_T test_done;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/
static void
_test_cases(
    C8_T *client,
    UI32_T p_case)
{
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;
    static UI8_T enable = TRUE;
    UI8_T method = 0;
    UI8_T idx = 0;
    UI8_T count = 0;
    UI16_T data_size = 0;
    UI16_T offset = 0;
    UI32_T msg_size = 0;
    DB_REQUEST_TYPE_T req;
    DB_TRUNK_PORT_T trunk1_member;
    DB_TRUNK_PORT_T trunk2_member;

    if (NULL == client)
    {
        return;
    }

    trunk1_member.members.member_bmp = BITS(1,4);
    trunk1_member.members.mode = 1;
    trunk2_member.members.member_bmp = BITS(7,8);
    trunk2_member.members.mode = 1;

    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "current test case is :%d\n", p_case);
    msg_size = DB_MSG_HEADER_SIZE;
    switch (p_case)
    {
        case 0:
            method = M_GET;
            count = 1;
            req.t_idx = SYS_INFO;
            req.f_idx = SYS_SW_VERSION;
            req.e_idx = DB_ALL_ENTRIES;
            dbapi_getDataSize(req, &data_size);
            msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
            break;
        case 1:
            method = M_UPDATE;
            count = 4;
            while (idx < count)
            {
                idx++;
                req.t_idx = PORT_CFG_INFO;
                req.f_idx = PORT_ADMIN_STATUS;
                req.e_idx = idx;
                dbapi_getDataSize(req, &data_size);
                msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
            }
            break;
        case 2:
            method = M_CREATE;
            count = 2;
            while (idx < count)
            {
                idx++;
                req.t_idx = TRUNK_PORT;
                req.f_idx = TRUNK_MEMBERS;
                req.e_idx = idx;
                dbapi_getDataSize(req, &data_size);
                msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
            }
            break;
        case 3:
            method = M_GET;
            count = 7;
            while (idx < count)
            {
                idx++;
                req.t_idx = PORT_OPER_INFO;
                req.f_idx = PORT_OPER_STATUS;
                req.e_idx = idx;
                dbapi_getDataSize(req, &data_size);
                msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
            }
            break;
        case 4:
            method = M_DELETE;
            count = 2;
            while (idx < count)
            {
                idx++;
                req.t_idx = TRUNK_PORT;
                req.f_idx = TRUNK_MEMBERS;
                req.e_idx = idx;
                dbapi_getDataSize(req, &data_size);
                msg_size += DB_MSG_PAYLOAD_SIZE + data_size;
            }
            break;
        default:
            return;
    }

    if (MW_E_OK != osapi_calloc(msg_size, client, (void **)&ptr_msg))
    {
        DB_LOG_ERROR("create the test2 msg structure (size: %d)", msg_size);
        return;
    }

    /* fill in message */
    /* message header */
    offset = dbapi_setMsgHeader(ptr_msg, client, method, count);
    ptr_data = (UI8_T *)ptr_msg;
    idx = 0;
    switch (p_case)
    {
        case 0:
            /* get sys_version */
            offset += dbapi_setMsgPayload(method,
                SYS_INFO, SYS_SW_VERSION, DB_ALL_ENTRIES,
                NULL, ptr_data + offset);
            count = dbapi_getData(client, ptr_msg, 5000);
            if (MW_E_OK == count)
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 receive method[0x%x] with result %d\n",
                    ptr_msg->method, ptr_msg->type.result);
            }
            osapi_free(ptr_msg);
            break;
        case 1:
            /* update port admin state of p1-p4 */
            enable = (0 == enable)? 1 : 0;
            while (idx < count)
            {
                idx++;
                offset += dbapi_setMsgPayload(method,
                    PORT_CFG_INFO, PORT_ADMIN_STATUS, idx,
                    &enable, ptr_data + offset);
            }
            count = dbapi_setData(client, ptr_msg, 5000);
            if (MW_E_OK != count)
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 set data failed with result %d\n", count);
            }
            break;
        case 2:
            /* create trunk 1, 2 member */
            offset += dbapi_setMsgPayload(method,
                TRUNK_PORT, TRUNK_MEMBERS, 1,
                &trunk1_member, ptr_data + offset);

            offset += dbapi_setMsgPayload(method,
                TRUNK_PORT, TRUNK_MEMBERS, 2,
                &trunk2_member, ptr_data + offset);

            count = dbapi_setData(client, ptr_msg, 5000);
            if (MW_E_OK != count)
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 set data failed with result %d\n", count);
            }
            break;
        case 3:
            /* Test to get port operational state */
            while (idx < count)
            {
                idx++;
                offset += dbapi_setMsgPayload(method,
                    PORT_OPER_INFO, PORT_OPER_STATUS, idx,
                    NULL, ptr_data + offset);
            }
            if (MW_E_OK == dbapi_getData(client, ptr_msg, 5000))
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 receive method[0x%x] with result %d\n",
                    ptr_msg->method, ptr_msg->type.result);
            }
            osapi_free(ptr_msg);
            break;
        case 4:
            /* Test to delete trunk */
            offset += dbapi_setMsgPayload(method,
                TRUNK_PORT, TRUNK_MEMBERS, 1,
                NULL, ptr_data + offset);
            offset += dbapi_setMsgPayload(method,
                TRUNK_PORT, TRUNK_MEMBERS, 2,
                NULL, ptr_data + offset);
            count = dbapi_setData(client, ptr_msg, 5000);
            if (MW_E_OK != count)
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 set data failed with result %d\n", count);
            }
            break;
        default:
            return;
    }
}
/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: db_test_task_1
 * PURPOSE:
 *      Test task 1 for subscriptions testing
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 *
 * NOTES:
 *
 */
void
db_test_task_1(
    void *args)
{
#define t1_qname    "dt1"
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T *data = NULL;
    DB_MSG_T *ptr_msg = NULL;
    UI32_T msg_size = (DB_MSG_HEADER_SIZE + (8 * DB_MSG_PAYLOAD_SIZE));
    UI16_T offset= 0;
    const threadhandle_t ptr_handle = *(const threadhandle_t *)args;

    test_subscribed= FALSE;
    test_done = FALSE;

    /* Create the testing queue */
    ret = osapi_msgCreate(t1_qname, 10, DB_MSG_PTR_SIZE);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test1 create testing message queue(ret = %u)", (UI8_T)(ret));
        osapi_processDelete(ptr_handle);
        return;
    }

    do
    {
        ret = dbapi_dbisReady();
        osapi_delay(100);
    }while (MW_E_OK != ret);

    /* create the subscribe data payload */
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test1 acts as a subscriber\n");
    ret = osapi_calloc(msg_size, t1_qname, (void **)(&ptr_msg));
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test1 create msg structure(ret = %u)", (UI8_T)(ret));
        osapi_msgDelete(t1_qname);
        osapi_processDelete(ptr_handle);
        return;
    }

    /* fill in message */
    /* message header */
    offset = dbapi_setMsgHeader(ptr_msg, t1_qname, M_SUBSCRIBE, 8);
    data = (UI8_T *)ptr_msg;
    /* payload 1 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, SYS_INFO, SYS_SW_VERSION, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 2 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 3 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, NULL, data + offset);
#ifdef AIR_SUPPORT_LP
    /* payload 6 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, NULL, data + offset);
#endif
    /* payload 7 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 8 */
    offset += dbapi_setMsgPayload(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, NULL, data + offset);

    /* send request */
    ret = dbapi_sendMsg(ptr_msg, 1000);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test1 send subscribe(ret = %u)", (UI8_T)(ret));
        osapi_free(ptr_msg);
        osapi_msgDelete(t1_qname);
        osapi_processDelete(ptr_handle);
        return;
    }

    /* receive response and notifications */
    while (test_done == FALSE)
    {
        ptr_msg = NULL;
        MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test1 waiting for DB message\n");
        ret = dbapi_recvMsg(t1_qname, &ptr_msg, DB_Q_WAITTIME);
        if (MW_E_TIMEOUT == ret)
        {
            DB_LOG_ERROR("test1 receive message timeout(ret = %u)", (UI8_T)(ret));
            continue;
        }
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("test1 receive a message(ret = %u)", (UI8_T)(ret));
            break;
        }
        else
        {
            MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test1 receive method[0x%x]\n", ptr_msg->method);
            /* Receive the notifications */
            if (M_B_RESPONSE == (ptr_msg->method & M_B_RESPONSE))
            {
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Payload result: %X\n", ptr_msg->type.result);
                test_subscribed= TRUE;
            }
            else
            {
                UI8_T paycount = 0;
                DB_REQUEST_TYPE_T  req;
                UI16_T payload_size;
                MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Payload count: %d\n", ptr_msg->type.count);
                data = (UI8_T*)(&(ptr_msg->ptr_payload));
                while (paycount < ptr_msg->type.count)
                {
                    memcpy((void *)&req, (const void *)data, sizeof(DB_REQUEST_TYPE_T));
                    data += sizeof(DB_REQUEST_TYPE_T);
                    memcpy((void *)&payload_size, (const void *)data, sizeof(UI16_T));
                    data += sizeof(UI16_T);
                    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Payload ####%d\n", paycount);
                    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "#### table/field/entry : %u/%u/%u\n", req.t_idx, req.f_idx, req.e_idx);
                    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "#### data size : %u\n", payload_size);
                    data += payload_size;
                    paycount++;
                }
            }
            osapi_free(ptr_msg);
        }
    }

    /* create the unsubscribe request */
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test1 unsubscribe all of the message\n");
    /* send request */
    ret = osapi_calloc(msg_size, t1_qname, (void **)(&ptr_msg));
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test1 create msg structure(ret = %u)\n", (UI8_T)(ret));
        osapi_msgDelete(t1_qname);
        osapi_processDelete(ptr_handle);
        return;
    }

    /* fill in message */
    /* message header */
    offset = dbapi_setMsgHeader(ptr_msg, t1_qname, M_UNSUBSCRIBE, 8);
    data = (UI8_T *)ptr_msg;
    /* payload 1 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, SYS_INFO, SYS_SW_VERSION, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 2 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 3 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, PORT_CFG_INFO, PORT_SETTINGS, DB_ALL_ENTRIES, NULL, data + offset);
#ifdef AIR_SUPPORT_LP
    /* payload 6 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, NULL, data + offset);
#endif
    /* payload 7 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, NULL, data + offset);
    /* payload 8 */
    offset += dbapi_setMsgPayload(M_UNSUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, NULL, data + offset);

    /* send request */
    ret = dbapi_sendMsg(ptr_msg, 1000);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test1 send subscribe(ret = %u)", (UI8_T)(ret));
        osapi_free(ptr_msg);
        osapi_msgDelete(t1_qname);
        osapi_processDelete(ptr_handle);
        return;
    }

    do
    {
        ret = dbapi_recvMsg(t1_qname, &ptr_msg, DB_Q_WAITTIME);
        if (MW_E_TIMEOUT == ret)
        {
            DB_LOG_ERROR("test1 receive message timeout(ret = %u)", (UI8_T)(ret));
            continue;
        }
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("test1 receive a message(ret = %u)", (UI8_T)(ret));
            break;
        }
        else
        {
            MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test1 receive method[0x%x] with result=0x%x\n",
                    ptr_msg->method, ptr_msg->type.result);
            /* Receive the notifications */
            if (M_B_RESPONSE != (ptr_msg->method & M_B_RESPONSE))
            {
                osapi_free(ptr_msg);
                continue;
            }
            osapi_free(ptr_msg);
        }
        break;
    }while (TRUE);

    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "Delete test1 queue\n");
    osapi_msgDelete(t1_qname);
    osapi_processDelete(ptr_handle);
}
/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: db_test_task_2
 * PURPOSE:
 *      Test task 2 for sending requests every 30s
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 *
 * NOTES:
 *
 */
void
db_test_task_2(
    void *args)
{
#define t2_qname    "dt2"
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T testcase = 0;
    const threadhandle_t ptr_handle = *(const threadhandle_t *)args;

    /* Create the testing queue */
    ret = osapi_msgCreate(t2_qname, MAX_TEST_CASES, DB_MSG_PTR_SIZE);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("test2 create testing message queue(ret = %u)", (UI8_T)(ret));
        osapi_processDelete(ptr_handle);
        return;
    }

    do
    {
        ret = dbapi_dbisReady();
        osapi_delay(100);
    }while ((MW_E_OK != ret) || (test_subscribed == FALSE));

    /* create the subscribe data payload */
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 acts as a updater\n");

    /* receive response and notifications */
    while (testcase < MAX_TEST_CASES)
    {
        _test_cases(t2_qname, testcase);
        testcase++;
    }

    test_done = TRUE;
    MW_LOG_RAW(DB, MW_LOG_LEVEL_DEBUG, "test2 test done\n");
    osapi_msgDelete(t2_qname);
    osapi_processDelete(ptr_handle);
}
