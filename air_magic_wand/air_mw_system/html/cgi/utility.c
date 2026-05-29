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

/* FILE NAME:  utility.c
 * PURPOSE:
 *  Implement the Interface for CGI/SSI handler function calling.
 *
 * NOTES:
 *
 */
/*INCLUDE FILE DECLARATIONS
*/
#include "web.h"
#include "mw_utils.h"
#include "mw_portbmp.h"
#include "osapi_memory.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* Add security headers here */
#define HTTP_HDR_SECURE_HDRS \
    LWIP_HTTPD_SUPPORT_X_FRAME_OPTIONS \
    LWIP_HTTPD_SUPPORT_CSP \
    LWIP_HTTPD_SUPPORT_X_CONTENT_TYPE_OPTIONS \
    LWIP_HTTPD_SUPPORT_REFERER_POLICY

#define HTTP_HDR_JS_SECURE_HDRS \
    LWIP_HTTPD_SUPPORT_X_CONTENT_TYPE_OPTIONS

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T
_cgi_utility_getCurTrunkBitMap(
    void);

static MW_ERROR_NO_T
_ssi_utility_parsePortTypesFromDbData(
    UI8_T *ptr_list,
    UI32_T list_len,
    UI8_T *ptr_db_data,
    UI32_T db_data_len);

/* STATIC VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROMGRAM BODIES
*/
/* EXPORTED SUBPROGRAM BODIES
 */

static UI32_T
_cgi_utility_getCurTrunkBitMap(
    void)
{
    UI32_T member = 0;
    UI16_T size = 0, num = 0,i = 0;
    char err = ERR_OK;
    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_data = NULL;

    err = httpd_queue_getData(PORT_CFG_INFO, PORT_TRUNK_ID, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK == err)
    {
        member = 0;
        for(num = 1; num <= MAX_TRUNK_NUM; num++)
        {
            for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if( num == ptr_data[i] )
                {
                    member |= (0x01 << i);
                }
            }
        }
    }
    MW_FREE(ptr_msg);
    return member;
}

static MW_ERROR_NO_T
_ssi_utility_parsePortTypesFromDbData(
    UI8_T *ptr_list,
    UI32_T list_len,
    UI8_T *ptr_db_data,
    UI32_T db_data_len)
{   MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T portIdx = 0;
    PORT_DB_TYPE_T tempType = 0;

    if ( (NULL == ptr_list) || (NULL == ptr_db_data))
    {
        CGI_LOG_ERROR(SYSTEM, "ptr_list=0x%p, ptr_db_data=0x%p", ptr_list, ptr_db_data);
        return MW_E_BAD_PARAMETER;
    }

    CGI_LOG_DEBUG(SYSTEM, "list len = %d, db_data len = %d", list_len, db_data_len);
    for (; portIdx < MIN(list_len, db_data_len); portIdx++)
    {
        rc = port_db_getPortType(0, portIdx + 1, ptr_db_data[portIdx], &(tempType));
        if (MW_E_OK != rc)
        {
            CGI_LOG_WARN(SYSTEM, "sfp_db_port_getPortType rc=%d",rc);
        }
        ptr_list[portIdx] = (UI8_T)tempType;
        CGI_LOG_DEBUG(SYSTEM, "db_data[%d]=0x%x, list[%d]=0x%02x", portIdx, ptr_db_data[portIdx], portIdx, ptr_list[portIdx]);
    }
    return rc;
}

/*FUNCTION NAME: cgi_utility_revisePortBitMapByTrunkInfo
 * PURPOSE:
 *        to revise the bit map of ports by trunk information
 * INPUT:
 *       trunkPbm -- the bitmap that represents all the trunk members
 *       indexStart -- the start index of the bitmap can be determined
 *       ptr_parent_func -- for debugging, the pointer refers to the string name of the parent function
 * OUTPUT:
 *       ptr_portBitMap -- the port bitmap has been update by consulting the trunk information
 * RETURN:
 *       NONE
 * NOTES:
 *       None
*/
void
cgi_utility_revisePortBitMapByTrunkInfo(
    UI32_T trunkPbm,
    UI32_T *ptr_portBitMap,
    UI32_T indexStart,
    const C8_T *ptr_parent_func)
{
    UI32_T member = 0,mask = 0;
    UI16_T i = 0;

    if (NULL == ptr_portBitMap || 0 == trunkPbm)
    {
        CGI_LOG_ERROR(SYSTEM, "ptr_portBitMap = %p, trunkPbm=%u", ptr_portBitMap, trunkPbm);
        return;
    }

    if (NULL != ptr_parent_func)
    {
        CGI_LOG_DEBUG(SYSTEM, "%s call", ptr_parent_func);
    }

    /*Get Port trunk member */
    member = _cgi_utility_getCurTrunkBitMap();
    CGI_LOG_DEBUG(SYSTEM, "member=%08X,trunkPortBitmap=%08X,portbmp=%08X,i=%d", member, trunkPbm, (*ptr_portBitMap), i);
    /* revise Port Bit Map by comparing trunk info*/
    for (i = 0; i < MW_PORT_NUM; i++)
    {
        mask = (UI32_T)(1 << i);
        if ( (member & mask) != (trunkPbm & mask) )
        {
            (*ptr_portBitMap) &= (~(mask << indexStart));
        }
    }
    CGI_LOG_DEBUG(SYSTEM, "portbmp=%08X,i=%d", (*ptr_portBitMap), i);
}

/*FUNCTION NAME: ssi_utility_getAllPortType
 * PURPOSE:
 *        to get the list of all port type
 * INPUT:
 *       ptr_list -- the type list of all ports
 *       list_len -- the length of type list
 * OUTPUT:
 *       NONE
 * RETURN:
 *       MW_E_OK
 *       MW_E_BAD_PARAMETER
 * NOTES:
 *      None
*/

MW_ERROR_NO_T
ssi_utility_getAllPortType(
    UI8_T *ptr_list,
    UI32_T list_len)
{
    DB_MSG_T *ptr_msg = NULL;
    void *ptr_data = NULL;
    UI16_T size = 0;
    MW_ERROR_NO_T rc = MW_E_OK;

    if (NULL != ptr_list)
    {
        rc = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_MODE,DB_ALL_ENTRIES,&ptr_msg,&size,&ptr_data);
        if (MW_E_OK == rc)
        {
            if (size > list_len)
            {
                size = list_len;
            }
            rc = _ssi_utility_parsePortTypesFromDbData(ptr_list,size,ptr_data,size);
        }
    }
    osapi_free(ptr_msg);

    return rc;
}

/*FUNCTION NAME: cgi_utility_checkPortAvailForCableDiag
 * PURPOSE:
 *        to check the selected port whether available for cable diagnostic
 * INPUT:
 *       port_idx -- the index of port
 * OUTPUT:
 *       NONE
 * RETURN:
 *       FALSE -- Not available for cable diagnostic
 *       TRUE -- available for cable diagnostic
 * NOTES:
 *      None
*/
BOOL_T
cgi_utility_checkPortAvailForCableDiag(
    UI8_T port_idx)
{
    BOOL_T ret = FALSE;
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    void *ptr_data = NULL;
    PORT_DB_TYPE_T port_mode = PORT_DB_TYPE_COPPER;
    UI16_T size = 0;

    rc = httpd_queue_getData(PORT_OPER_INFO , PORT_OPER_MODE, port_idx, &ptr_msg, &size, &ptr_data);
    if (MW_E_OK != rc || NULL == ptr_data)
    {
        ret = FALSE;
        port_mode = PORT_DB_TYPE_COPPER;
    }
    else
    {
        rc = port_db_getPortType(0, port_idx, *((UI8_T *)ptr_data), &port_mode);
    }
    osapi_free(ptr_msg);
    ptr_msg = NULL;
    if ( (PORT_DB_TYPE_COPPER == port_mode) || (PORT_DB_TYPE_COMBO_COPPER == port_mode) )
    {
        ret =  TRUE;
    }

    return ret;
}

MW_ERROR_NO_T
ssi_get_sechdr_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
#ifdef AIR_SUPPORT_HTTPD_SECURE
    UI16_T len = 0;
    char err = 0;

    err = send_format_response_no_chunk(&len, ptr_pcb, apiflags, HTTP_HDR_SECURE_HDRS, http_nonce_buf, http_nonce_buf);
    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }

    *length = len;
#endif
    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_js_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
#ifdef AIR_SUPPORT_HTTPD_SECURE
    UI16_T len = 0;
    char err = 0;

    err = send_format_response(&len, ptr_pcb, apiflags, "<script nonce='%s'", http_nonce_buf);
    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }

    *length = len;
#endif /* AIR_SUPPORT_HTTPD_SECURE */
    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_css_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
#ifdef AIR_SUPPORT_HTTPD_SECURE
    UI16_T len = 0;
    char err = 0;

    err = send_format_response(&len, ptr_pcb, apiflags, "<style nonce='%s'", http_nonce_buf);
    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }

    *length = len;
#endif /* AIR_SUPPORT_HTTPD_SECURE */
    return MW_E_OK;
}

MW_ERROR_NO_T
ssi_get_jshdr_info_Handle(
    int *length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
#ifdef AIR_SUPPORT_HTTPD_SECURE
    UI16_T len = 0;
    char err = 0;

    err = send_format_response_no_chunk(&len, ptr_pcb, apiflags, HTTP_HDR_JS_SECURE_HDRS);
    if(err != ERR_OK)
    {
        return MW_E_OTHERS;
    }

    *length = len;
#endif /* AIR_SUPPORT_HTTPD_SECURE */
    return MW_E_OK;
}
