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

/* FILE NAME:  vlanVoiceSurveil.c
 * PURPOSE:
 *  CGI and SSI handler for Voice VLAN and Surveillance VLAN webpage.
 *
 * NOTES:
 *
 */
#if (defined(AIR_SUPPORT_VOICE_VLAN)) || (defined(AIR_SUPPORT_SURVEILLANCE_VLAN))
/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "mw_utils.h"
#include "mac_utils.h"
#include "vlan_utils.h"
#include "httpd_queue.h"
#include "web.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct OUI_ENTRY_INFO_S
{
    VOVLAN_OUI_T    mac;           /* The MSB 3 bytes of MAC address */
    UI8_T           type;          /* Voice VLAN or Surveillance VLAN */
    OUI_DESCR_T     descr;         /* The description of the OUI */
} ATTRIBUTE_PACK OUI_ENTRY_INFO_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
#ifdef AIR_SUPPORT_SFP
static void
_cgi_VoiceSurveil_setPortModeByTrunkInfo(
    UI32_T trunkBmp,
    UI32_T oldPortMode,
    UI32_T *ptr_portMode);
#endif
/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/
#ifdef AIR_SUPPORT_SFP
static void
_cgi_VoiceSurveil_setPortModeByTrunkInfo(
    UI32_T trunkBmp,
    UI32_T oldPortMode,
    UI32_T *ptr_portMode)
{
    UI8_T i = 0;
    if (NULL == ptr_portMode)
    {
        return;
    }

    CGI_LOG_DEBUG(VLAN, "before ptr_data->port_mode = 0x%08X\n", (*ptr_portMode));
    for (; i < PLAT_MAX_PORT_NUM; i++)
    {
        if ( trunkBmp & (UI32_T)(1<<i) )
        {
            if ( oldPortMode & (UI32_T)(1<<i))
            {
                (*ptr_portMode) |= (UI32_T)(1<<i);
            }
            else
            {
                (*ptr_portMode) &= (UI32_T)( ~(1<<i) );
            }
        }
    }
    CGI_LOG_DEBUG(VLAN, "after ptr_data->port_mode = 0x%08X\n", (*ptr_portMode));
}
#endif
MW_ERROR_NO_T _cgi_VoiceSurveil_VlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[], UI16_T vlan_type) {

    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i;
    BOOL_T isChanged = false;
#ifdef AIR_SUPPORT_VOICE_VLAN
    DB_VOICE_VLAN_INFO_T vlan_info = { 0 };
#elif defined AIR_SUPPORT_SURVEILLANCE_VLAN
    DB_SURVEI_VLAN_INFO_T vlan_info = { 0 };
#endif

    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

#ifdef AIR_SUPPORT_VOICE_VLAN
    /* get voice or surveillance vlan info */
    if(VLAN_MAC_VOICE_ENABLE == vlan_type)
    {
        DB_MSG_T *db_msg = NULL;
        UI16_T db_size = 0;
        DB_VOICE_VLAN_INFO_T *db_data = NULL;

        rc = httpd_queue_getData(VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&db_data);
        if(MW_E_OK == rc) {
            CGI_LOG_DEBUG(VLAN, "%s: get voice vlan info success, ptr_msg =%p \n", __func__, db_msg);
        } else {
            CGI_LOG_DEBUG(VLAN, "%s: get voice vlan info failed(%d) \n", __func__, rc);
            return ERR_VAL;
        }
        vlan_info.port_mode = db_data->port_mode;
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        vlan_info.security_mode = db_data->security_mode;
#endif
        MW_FREE(db_msg);
    }
#endif

    /* Parser name=value from cgi parameter */
    for (i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "state")){
            vlan_info.vlan_state = atoi(pcValue[i]);
            isChanged = true;
        }
        if(!strcmp(pcParam[i], "vid")){
            vlan_info.vlan_id = atoi(pcValue[i]);
            isChanged = true;
        }
        if(!strcmp(pcParam[i], "pri")){
            vlan_info.vlan_priority = atoi(pcValue[i]);
            isChanged = true;
        }
    }
    if(false == isChanged) return rc;

    if (vlan_info.vlan_state == FALSE)
    {
        /* Reset to zero if disable the voice vlan or surveillance vlan
           In case of voice vlan member both be tagged and untagged member issue
         */
        vlan_info.vlan_id = 0;
        vlan_info.vlan_priority = 0;
    }
    CGI_LOG_DEBUG(VLAN, "%s[%d] state: %d, vid: %d, pri: %d \n", __func__, __LINE__, vlan_info.vlan_state, vlan_info.vlan_id, vlan_info.vlan_priority);

    /* Update to db */
#ifdef AIR_SUPPORT_VOICE_VLAN
    if (VLAN_MAC_VOICE_ENABLE == vlan_type)
    {
        rc = httpd_queue_setData(M_UPDATE, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &vlan_info, sizeof(DB_VOICE_VLAN_INFO_T));
    }
    else
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    if(VLAN_MAC_SURVEIL_ENABLE == vlan_type)
    {
        rc = httpd_queue_setData(M_UPDATE, SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &vlan_info, sizeof(DB_SURVEI_VLAN_INFO_T));
    }
    else
#endif
    {
        CGI_LOG_DEBUG(VLAN, "%s: Invalid parameter vlan type(%d) \n", __func__, vlan_type);
        return MW_E_BAD_PARAMETER;
    }

    if(MW_E_OK != rc) {
        CGI_LOG_ERROR(VLAN, "%s: httpd_queue_setData failed(%d) \n", __func__, rc);
    }

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);

    return rc;
}
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
MW_ERROR_NO_T _cgi_VoiceSurveil_OuiSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[], UI16_T vlan_type) {

    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i, oui_idx = 0;
    BOOL_T isChanged = false;
    OUI_ENTRY_INFO_T new_oui_entry = {{0}};

    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "idx")){
            oui_idx = atoi(pcValue[i]);
            isChanged = true;
        }
        if(!strcmp(pcParam[i], "oui")){
            strToOui(pcValue[i], &new_oui_entry.mac);
            isChanged = true;
        }
        if(!strcmp(pcParam[i], "descr")){
            snprintf((char *)new_oui_entry.descr, sizeof(OUI_DESCR_T), "%s", pcValue[i]);
            isChanged = true;
        }
    }
    if(false == isChanged) return rc;

    CGI_LOG_DEBUG(VLAN, "%s[%d] oui: %02x:%02x:%02x \n", __func__, __LINE__, new_oui_entry.mac[0], new_oui_entry.mac[1], new_oui_entry.mac[2]);
    CGI_LOG_DEBUG(VLAN, "%s[%d] descr: %s \n", __func__, __LINE__, new_oui_entry.descr);

    /* Prepare data which will update to db */
    new_oui_entry.type = vlan_type;

    rc = httpd_queue_setData(M_UPDATE, OUI_ENTRY, DB_ALL_FIELDS, oui_idx, &new_oui_entry, sizeof(OUI_ENTRY_INFO_T));
    if(MW_E_OK != rc) {
        CGI_LOG_DEBUG(VLAN, "%s: httpd_queue_setData failed(%d) \n", __func__, rc);
    }

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);

    return rc;
}

MW_ERROR_NO_T _cgi_VoiceSurveil_OuiDel(int iIndex, int iNumParams, char *pcParam[], char *pcValue[], UI16_T vlan_type) {

    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i = 0, count = 0;
    UI32_T ouiBmp = 0;

    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "oui_bmp")){
            ouiBmp = atoi(pcValue[i]);
        }
        if(!strcmp(pcParam[i], "count")){
            count = atoi(pcValue[i]);
        }
    }
    if(0 == count) return rc;

    CGI_LOG_DEBUG(VLAN, "%s[%d] ouiBmp: %d, count: %d \n", __func__, __LINE__, ouiBmp, count);

    for(i = 0; i < MAX_OUI_NUM; i++)
    {
        if(ouiBmp & (1 << i))
        {
            rc = httpd_queue_setData(M_DELETE, OUI_ENTRY, DB_ALL_FIELDS, (i+1), NULL, 0);
            if(MW_E_OK != rc) {
                CGI_LOG_DEBUG(VLAN, "%s: httpd_queue_setData failed(%d) \n", __func__, rc);
                break;
            }
        }
    }

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);

    return rc;
}
#endif

#ifdef AIR_SUPPORT_VOICE_VLAN
MW_ERROR_NO_T _cgi_Voice_PortSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {

    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i = 0, count = 0;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    DB_VOICE_VLAN_INFO_T *ptr_data = NULL;
    UI32_T curPortMode =0;
#if(defined(AIR_SUPPORT_SFP))
    UI32_T voicePortBitmap =0;
    UI32_T trunkBitMap = 0;
#endif
    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

    rc = httpd_queue_getData(VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&ptr_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s: get voice vlan info success, ptr_msg =%p \n", __func__, db_msg);
    } else {
        CGI_LOG_ERROR(VLAN, "%s: get voice vlan info failed(%d) \n", __func__, rc);
        return ERR_VAL;
    }

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "portmode")){
            curPortMode = atoi(pcValue[i]);
            count++;
        }
#if(defined(AIR_SUPPORT_SFP))
        if (0 == strcmp(pcParam[i], "trunkBitMap"))
        {
            trunkBitMap = atoi(pcValue[i]);
            count++;
        }
        if (0 == strcmp(pcParam[i], "portBitmap"))
        {
            voicePortBitmap = atoi(pcValue[i]);
            count++;
        }
#endif
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        if(!strcmp(pcParam[i], "secmode"))
        {
            ptr_data->security_mode = atoi(pcValue[i]);
            count++;
        }
#endif
    }
    if(0 == count) {
        MW_FREE(db_msg);
        return rc;
    }
#if(defined(AIR_SUPPORT_SFP))
    CGI_UTILITY_REVISE_PBMP_BY_TRUNK_INFO(trunkBitMap,&voicePortBitmap,0);
    _cgi_VoiceSurveil_setPortModeByTrunkInfo(voicePortBitmap,curPortMode,&(ptr_data->port_mode));
#else
    ptr_data->port_mode = curPortMode;
#endif
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    CGI_LOG_DEBUG(VLAN, "%s[%d] portmode: %d, secmode: %d \n", __func__, __LINE__, ptr_data->port_mode, ptr_data->security_mode);
#else
    CGI_LOG_DEBUG(VLAN, "%s[%d] portmode: %d \n", __func__, __LINE__, ptr_data->port_mode);
#endif

    rc = httpd_queue_setData(M_UPDATE, VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, (const void *)ptr_data, db_size);
    MW_FREE(db_msg);
    if(MW_E_OK != rc) {
        CGI_LOG_DEBUG(VLAN, "%s: httpd_queue_setData failed(%d) \n", __func__, rc);
    }

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);
    return rc;
}
#endif

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
char _ssi_VoiceServeil_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags, UI16_T vlan_type)
{
    UI8_T i = 0;
    UI32_T total_len = 0;
    UI16_T len;
    C8_T err = 0;
    C8_T buf[(MAX_VLAN_ENTRY_NUM * 6)] = {0};

    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0;
    void *db_data = NULL;

#ifdef AIR_SUPPORT_VOICE_VLAN
    DB_VOICE_VLAN_INFO_T *vlan_info = NULL;
#elif defined AIR_SUPPORT_SURVEILLANCE_VLAN
    DB_SURVEI_VLAN_INFO_T *vlan_info = NULL;
#endif
    DB_OUI_ENTRY_T *oui_entry_arr = NULL;
    UI16_T vlan_count = 0;
    UI8_T surveil_count = 0, voice_count = 0;
    UI32_T voice_bmp = 0, surveil_bmp = 0;
#ifdef AIR_SUPPORT_VOICE_VLAN
    UI32_T memberbmp = 0;
    UI32_T *ptr_port_member = NULL;
#endif
    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

    err = send_format_response(&len, pcb, apiflags, "<script> var portMaxNum = %d;\n", PLAT_MAX_PORT_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* get vlan cfg info */
    rc = httpd_queue_getData(VLAN_CFG_INFO, VLAN_8021Q_B_ENABLE, DB_ALL_ENTRIES, &db_msg, &db_size, &db_data);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s: get vlan cfg info success, ptr_msg =%p \n", __func__, db_msg);
    } else {
        CGI_LOG_ERROR(VLAN, "%s: get vlan cfg info failed(%d) \n", __func__, rc);
        return ERR_VAL;
    }

    if(VLAN_STATE_ENABLE == (*((UI8_T *)db_data))) {
        (*((UI8_T *)db_data)) = VLAN_1Q_ENABLE;
    } else {
        (*((UI8_T *)db_data)) = VLAN_PORT_ENABLE;
    }

    err = send_format_response(&len, pcb, apiflags, "var vlanState=%d;\n", *((UI8_T *)db_data));
    MW_FREE(db_msg);
    if (ERR_OK != err) {
        return err;
    }
    total_len += len;

    /* compose ouiEntryMaxLimit */
    err = send_format_response(&len, pcb, apiflags, "var ouiEntryMaxLimit=%d;\n", MAX_OUI_NUM);
    if(err != ERR_OK) {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_VOICE_VLAN
    /* get voice or surveillance vlan info */
    if(VLAN_MAC_VOICE_ENABLE == vlan_type)
    {
        rc = httpd_queue_getData(VOICE_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&vlan_info);
    }
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    if(VLAN_MAC_SURVEIL_ENABLE == vlan_type)
    {
        rc = httpd_queue_getData(SURVEI_VLAN_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&vlan_info);
    }
#endif

    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s: get surveillance vlan info success, ptr_msg =%p \n", __func__, db_msg);
    } else {
        CGI_LOG_ERROR(VLAN, "%s: get surveillance vlan info failed(%d) \n", __func__, rc);
        return ERR_VAL;
    }

#ifdef AIR_SUPPORT_VOICE_VLAN
    if (VLAN_MAC_VOICE_ENABLE == vlan_type)
    {
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
        err = send_format_response(&len, pcb, apiflags, "var secshow=true;\n");
        if (ERR_OK != err) {
            MW_FREE(db_msg);
            return err;
        }
        total_len += len;

        err = send_format_response(&len, pcb, apiflags, "var voiceinfo={state:%d,vid:%d,pri:%d,portmode:%d,secmode:%d}; ",
                                    vlan_info->vlan_state, vlan_info->vlan_id, vlan_info->vlan_priority, vlan_info->port_mode, vlan_info->security_mode);
#else
        err = send_format_response(&len, pcb, apiflags, "var secshow=false;\n");
        if (ERR_OK != err) {
            MW_FREE(db_msg);
            return err;
        }
        total_len += len;

        err = send_format_response(&len, pcb, apiflags, "var voiceinfo={state:%d,vid:%d,pri:%d,portmode:%d}; ",
                                    vlan_info->vlan_state, vlan_info->vlan_id, vlan_info->vlan_priority, vlan_info->port_mode);
#endif
    }
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    if(VLAN_MAC_SURVEIL_ENABLE == vlan_type)
    {
        err = send_format_response(&len, pcb, apiflags, "var surveilinfo={state:%d,vid:%d,pri:%d}; ",
                                    vlan_info->vlan_state, vlan_info->vlan_id, vlan_info->vlan_priority);
    }
#endif

    MW_FREE(db_msg);
    if (ERR_OK != err) {
        return err;
    }
    total_len += len;

#ifdef AIR_SUPPORT_VOICE_VLAN
    rc = httpd_queue_getData(VOICE_OPER_INFO, VOICE_OPER_MEMBER_STATE, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&ptr_port_member);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s: get voice vlan memberstate success, ptr_msg =%p \n", __func__, db_msg);
    } else {
        CGI_LOG_ERROR(VLAN, "%s: get voice vlan memberstate failed(%d) \n", __func__, rc);
        return ERR_VAL;
    }
    memberbmp = *ptr_port_member;
    MW_FREE(db_msg);

    err = send_format_response(&len, pcb, apiflags, "var memberstate=%d; ", memberbmp);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;
#endif

    /* get oui entry data */
    rc = httpd_queue_getData(OUI_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&oui_entry_arr);
    if(MW_E_OK == rc) {
        CGI_LOG_DEBUG(VLAN, "%s: get OUI entry success, ptr_msg =%p \n", __func__, db_msg);
    } else {
        CGI_LOG_ERROR(VLAN, "%s: get OUI entry failed(%d)) \n", __func__, rc);
        return ERR_VAL;
    }

    for(i = 0; i < MAX_OUI_NUM; i++)
    {
#ifdef AIR_SUPPORT_VOICE_VLAN
        if (VLAN_MAC_VOICE_ENABLE == oui_entry_arr->type[i])
        {
            voice_bmp |= (1 << i);
            voice_count++;
        }
        else
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
        if(VLAN_MAC_SURVEIL_ENABLE == oui_entry_arr->type[i])
        {
            surveil_bmp |= (1 << i);
            surveil_count++;
        }
        else
#endif
        {
            continue;
        }
    }

    /* compose voice oui entry data */
    err = send_format_response(&len, pcb, apiflags, "var voiceinfo_ds = {oui_info:[");
    if (ERR_OK != err) {
        MW_FREE(db_msg);
        return err;
    }
    total_len += len;

    vlan_count = 0;
    for(i = 0; i < MAX_OUI_NUM; i++)
    {
        if(voice_bmp & (1 << i))
        {
            memset(&buf, 0, sizeof(buf));

            snprintf(buf, sizeof(buf), "{index:%d,oui:'%02X:%02X:%02X',desc:'%s'}", i+1,
                        oui_entry_arr->mac[i][0], oui_entry_arr->mac[i][1], oui_entry_arr->mac[i][2], oui_entry_arr->descr[i]);

            err = send_format_response(&len, pcb, apiflags, "%s,", buf);
            if(ERR_OK != err)
            {
                MW_FREE(db_msg);
                return err;
            }
            total_len += len;

            vlan_count++;
        }
        if(vlan_count == voice_count) break;
    }

    err = send_format_response(&len, pcb, apiflags, "], voiceOuiCount:%d}; ", voice_count);
    if (ERR_OK != err) {
        MW_FREE(db_msg);
        return err;
    }
    total_len += len;

    /* compose surveillance oui entry data */
    err = send_format_response(&len, pcb, apiflags, "var surveilinfo_ds = {oui_info:[");
    if (ERR_OK != err) {
        MW_FREE(db_msg);
        return err;
    }
    total_len += len;

    vlan_count = 0;
    for(i = 0; i < MAX_OUI_NUM; i++)
    {
        if(surveil_bmp & (1 << i))
        {
            memset(&buf, 0, sizeof(buf));

            snprintf(buf, sizeof(buf), "{index:%d,oui:'%02X:%02X:%02X',desc:'%s'}", i+1,
                        oui_entry_arr->mac[i][0], oui_entry_arr->mac[i][1], oui_entry_arr->mac[i][2], oui_entry_arr->descr[i]);

            err = send_format_response(&len, pcb, apiflags, "%s,", buf);
            if(ERR_OK != err) {
                MW_FREE(db_msg);
                return err;
            }
            total_len += len;

            vlan_count++;
        }
        if(vlan_count == surveil_count) break;
    }

    err = send_format_response(&len, pcb, apiflags, "], surveilOuiCount:%d}; ", surveil_count);
    if (ERR_OK != err) {
        MW_FREE(db_msg);
        return err;
    }
    total_len += len;
    MW_FREE(db_msg);

    err = send_format_response(&len, pcb, apiflags, "</script>");
    if(ERR_OK != err) {
        return err;
    }
    total_len += len;
    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);

    return ERR_OK;
}
#endif
char _ssi_VoiceServeil_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags, UI16_T vlan_type)
{
    UI8_T i = 0, ety_cnt = 0;
    UI32_T total_len = 0;
    C8_T err = 0;

    MW_ERROR_NO_T rc = MW_E_OK;
    DB_MSG_T *db_msg = NULL;
    UI16_T db_size = 0, len = 0;
    UI8_T *ptr_trunk = NULL;
    DB_TRUNK_PORT_T trunk_info;
    UI32_T member = 0;

    CGI_LOG_DEBUG(VLAN, "============== %s: enter ================\n", __func__);

    err = send_format_response(&len, pcb, apiflags, "<script> var trunk_conf = {\n");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "maxTrunkNum:%d,\ninfo:[\n", MAX_TRUNK_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    /* Get DB TRUNK_PORT */
    rc = httpd_queue_getData(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES, &db_msg, &db_size, (void **)&ptr_trunk);
    if(MW_E_OK != rc)
    {
        return rc;
    }

    for(i = 0; i < MAX_TRUNK_NUM; i++)
    {
        len = 0;
        memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
        member = trunk_info.members.member_bmp;

        if(member != 0)
        {
            ety_cnt++;
        }

        err = send_format_response(&len, pcb, apiflags,
                "{valid:%d,mbrs:%u},\n",
                (member != 0)? 1 : 0,
                member);
        if(MW_E_OK != err)
        {
            MW_FREE(db_msg);
            return err;
        }
        total_len += len;
    }
    MW_FREE(db_msg);

    err = send_format_response(&len, pcb, apiflags, "],count:%u};</script> \n", ety_cnt);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(VLAN, "============== %s: leave ================\n", __func__);

    return ERR_OK;
}

/* EXPORTED SUBPROGRAM BODIES
*/
#ifdef AIR_SUPPORT_VOICE_VLAN
MW_ERROR_NO_T cgi_set_handle_voiceVlanSet(int iIndex, int iNumParams, char* pcParam[], char* pcValue[])
{
    /* expected CGI format:
     * voicevlanSet.cgi?state=1&vid=2&pri=3
     */
    return _cgi_VoiceSurveil_VlanSet(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_VOICE_ENABLE);
}


MW_ERROR_NO_T cgi_set_handle_voiceOuiSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * voiceOuiSet.cgi?idx=3&oui=000FE2&descr=huawei3com
     */
    return _cgi_VoiceSurveil_OuiSet(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_VOICE_ENABLE);
}

MW_ERROR_NO_T cgi_set_handle_voiceOuiDel(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * voiceOuiDel.cgi?oui_bmp=2&count=1
     */
    return _cgi_VoiceSurveil_OuiDel(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_VOICE_ENABLE);
}

MW_ERROR_NO_T cgi_set_handle_voicePortSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * voicePortSet.cgi?portmode=268435455
     */
    return _cgi_Voice_PortSet(iIndex, iNumParams, pcParam, pcValue);
}

char ssi_get_voice_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    /* expected javascript string:
     *      var portMaxNum = 28;
     *      var vlanState = 2;
     *      var ouiEntryMaxLimit = 32;
     *      var secshow=true;
     *      var voiceinfo = {state:1,vid:3,pri:6,portmode:268435455};
     *      var memberstate = 0;
     *      var voiceinfo_ds = {oui_info:[{index:2, oui:'00:03:6B', desc: 'cisco'}], voiceOuiCount:1};
     *      var surveilinfo_ds = {oui_info:[{index:1, oui:'00:E0:BB', desc: '3com'},], surveilOuiCount:1};
     */
    return _ssi_VoiceServeil_info_Handle(length, pcb, apiflags, VLAN_MAC_VOICE_ENABLE);
}

char ssi_get_voice_vlan_extra_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    /* expected javascript string:
     *      var trunk_conf = {
     *          maxTrunkNum :4,
     *          info: [
     *              {valid:1,mbrs:34},
     *              {valid:0,mbrs:0},
     *              {valid:1,mbrs:2930628},
     *              {valid:0,mbrs:0},
     *          ], count: 2};
     */
    return _ssi_VoiceServeil_extra_info_Handle(length, pcb, apiflags, VLAN_MAC_VOICE_ENABLE);
}
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
MW_ERROR_NO_T cgi_set_handle_surveilvlanSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * surveilvlanSet.cgi?state=1&vid=2&pri=6
     */
    return _cgi_VoiceSurveil_VlanSet(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_SURVEIL_ENABLE);
}

MW_ERROR_NO_T cgi_set_handle_surveilOuiSet(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * surveilOuiSet.cgi?idx=3&oui=000FE2&descr=huawei3com
     */
    return _cgi_VoiceSurveil_OuiSet(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_SURVEIL_ENABLE);
}

MW_ERROR_NO_T cgi_set_handle_surveilOuiDel(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    /* expected CGI format:
     * surveilOuiDel.cgi?oui_bmp=1&count=1
     */
    return _cgi_VoiceSurveil_OuiDel(iIndex, iNumParams, pcParam, pcValue, VLAN_MAC_SURVEIL_ENABLE);
}

char ssi_get_serveil_vlan_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    /* expected javascript string:
     *      var portMaxNum = 28;
     *      var vlanState = 2;
     *      var ouiEntryMaxLimit = 32;
     *      var surveilinfo = {state:1,vid:2,pri:3};
     *      var voiceinfo_ds = {oui_info:[{index:2, oui:'00:03:6B', desc: 'cisco'}], voiceOuiCount:1};
     *      var surveilinfo_ds = {oui_info:[{index:1, oui:'00:E0:BB', desc: '3com'},], surveilOuiCount:1};
     */
    return _ssi_VoiceServeil_info_Handle(length, pcb, apiflags, VLAN_MAC_SURVEIL_ENABLE);
}
#endif
#endif
