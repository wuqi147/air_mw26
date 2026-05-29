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

/* FILE NAME:  syncd_api_macvlan.c
 * PURPOSE:
 *  Implement mac based vlan API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>

#include <syncd_api_macvlan.h>
#include <vlan_utils.h>
#include <air_vlan.h>

/* NAMING CONSTANT DECLARATIONS
*/
#define MACVLAN_STATE_ENABLE  (1)
#define MACVLAN_STATE_DISABLE (0)

/* MACRO FUNCTION DECLARATIONS
 */
typedef struct MAC_VLAN_CONTROL_BLOCK_S
{
    /* VLAN Entry Vaild */
    UI16_T valid:1;
    /* VID */
    UI16_T vid:12;
    /* Priority */
    UI16_T priority:3;
}MAC_VLAN_CONTROL_BLOCK_T;

typedef struct MAC_OUI_CONTROL_BLOCK_S
{
    /* OUI Mac Entry */
    UI8_T macOUI[3];
    UI8_T type;
}MAC_OUI_CONTROL_BLOCK_T;

enum {
    VOICE_VLAN_IDX = 0,
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    SURVEI_VLAN_IDX,
#endif
    VLAN_IDX_NUM
};

/* GLOBAL VARIABLE DECLARATIONS
*/
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
AIR_MAC_T ouiMask = {0xff, 0xff, 0xff, 0x0, 0x0, 0x0};

/* Mac base vlan Control block */
MAC_VLAN_CONTROL_BLOCK_T vlanInfo[VLAN_IDX_NUM];
MAC_OUI_CONTROL_BLOCK_T ouiEntry[MAX_OUI_NUM];
UI8_T InitControlBlocl = FALSE;
#endif

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
/* Transform MAC entry format from syncd style to SDK style and write control block */
static void
_init_controlBlock()
{

    SYNCD_LOG_DEBUG("Init vlan info and OUI mac entry control block \r\n");
    if(!InitControlBlocl)
    {
        memset(vlanInfo, 0, sizeof(vlanInfo));
        memset(ouiEntry, 0, sizeof(ouiEntry));
        InitControlBlocl = TRUE;
    }
    else
    {
        SYNCD_LOG_DEBUG("Already Init, do nothing \r\n");
    }
}

static void
_syncd_api_macvlan_transform(
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry,
    const UI8_T vlanInfoIdx,
    const MW_MAC_T *ptr_mac_addr,
    const UI8_T method)
{
    memset(ptr_entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));

    SYNCD_LOG_DEBUG("Check CB idx[%d] vaild=%d, vid=%d, priority=%d \n ", vlanInfoIdx, vlanInfo[vlanInfoIdx].valid, vlanInfo[vlanInfoIdx].vid, vlanInfo[vlanInfoIdx].priority);

    /* MAC */
    memcpy(ptr_entry->mac, ptr_mac_addr, sizeof(MW_MAC_T));
    /* VID */
    ptr_entry->vid = vlanInfo[vlanInfoIdx].vid;
    /* Prio */
    ptr_entry->pri = vlanInfo[vlanInfoIdx].priority;
    /* Mac Mask */
    memcpy(ptr_entry->mac_mask, ouiMask, sizeof(MW_MAC_T));
}
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
#ifdef AIR_SUPPORT_VOICE_VLAN
/* FUNCTION NAME:   syncd_api_macvlan_setVoiceVlanInfo
 * PURPOSE:
 *    Set voice vlan info from DB
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_macvlan_setVoiceVlanInfo(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;
    MW_MAC_T mac_oui;
    UI8_T *ptr_data = NULL;
    UI8_T *vid_ptr = NULL, *priority_ptr = NULL;
    UI8_T state = 0, priority = 0;
    UI16_T vid = 0;
    UI8_T i = 0, entryIdx = 0;
    VOVLAN_OUI_T *ptr_mac_addr = NULL;

    MW_CHECK_PTR(ptr_api_arg);
    _init_controlBlock();
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    vid_ptr = (UI8_T *)(ptr_data + sizeof(UI8_T));
    priority_ptr = (UI8_T *)(vid_ptr + sizeof(UI16_T));

    state = *ptr_data;
    vid = vid_ptr[0] | (vid_ptr[1] << 8);
    priority = *priority_ptr;
    SYNCD_LOG_DEBUG("Check ControlBlock vid = %d pro=%d state=%d \n", vid, priority, state);
    /* Update vlanInfo Control block*/
    vlanInfo[VOICE_VLAN_IDX].valid = state;
    vlanInfo[VOICE_VLAN_IDX].vid = vid;
    vlanInfo[VOICE_VLAN_IDX].priority = priority;

    /* avoid invalid vlan set to SDK*/
    if ((0 == vid) && (MACVLAN_STATE_ENABLE == state))
    {
        SYNCD_LOG_DEBUG("Invalid VlanInfo vid(%d)", vid);
        return MW_E_OK;
    }

    for (i = 0; i < MAX_OUI_NUM; i++)
    {
        SYNCD_LOG_DEBUG("CB_idx[%d] [%x:%x:%x] type = %d \n", i, ouiEntry[i].macOUI[0], ouiEntry[i].macOUI[1], ouiEntry[i].macOUI[2], ouiEntry[i].type);
        if (VLAN_MAC_VOICE_ENABLE != ouiEntry[i].type)
        {
            SYNCD_LOG_DEBUG("vlan type is not voice vlan \n ");
            continue;
        }
        ptr_mac_addr = (VOVLAN_OUI_T *)ouiEntry[i].macOUI;
        if (!memcmp(ptr_mac_addr, &ouiMask[3], sizeof(VOVLAN_OUI_T))) /* OUI entry CB is null*/
        {
            SYNCD_LOG_DEBUG("not entries \r\n ");
            continue;
        }
        entryIdx = i;

        rc = air_vlan_getMacBasedVlan(0, entryIdx, &entry);
        if (AIR_E_OK == rc)
        {
            rc = air_vlan_delMacBasedVlan(0, entryIdx);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Remove MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }

        if (MACVLAN_STATE_DISABLE != state)
        {
            memset(&mac_oui, 0 , sizeof(MW_MAC_T));
            memcpy(&mac_oui, ptr_mac_addr, sizeof(VOVLAN_OUI_T));

            memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));
            memcpy(&entry.mac, mac_oui, sizeof(MW_MAC_T));
            entry.vid = vid;
            entry.pri= priority;
            memcpy(&entry.mac_mask, ouiMask, sizeof(MW_MAC_T));

            rc = air_vlan_addMacBasedVlan(0, entryIdx, &entry);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Set MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    return MW_E_OK;
}
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/* FUNCTION NAME:   syncd_api_macvlan_setSurveilVlanInfo
 * PURPOSE:
 *    Set Surveillance vlan info from DB
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_macvlan_setSurveilVlanInfo(
    const SYNCD_API_ARG_T *ptr_api_arg)
{

    AIR_ERROR_NO_T rc;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;
    MW_MAC_T mac_oui;
    UI8_T *ptr_data = NULL;
    UI8_T *vid_ptr = NULL,*priority_ptr = NULL;
    UI8_T state = 0, priority = 0;
    UI16_T vid = 0;
    UI8_T i = 0, entryIdx = 0;
    VOVLAN_OUI_T *ptr_mac_addr = NULL;

    MW_CHECK_PTR(ptr_api_arg);
    _init_controlBlock();
    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    vid_ptr = (UI8_T *)(ptr_data + sizeof(UI8_T));
    priority_ptr = (UI8_T *)(vid_ptr + sizeof(UI16_T));

    state = *ptr_data;
    vid = vid_ptr[0] | (vid_ptr[1] << 8);
    priority = *priority_ptr;
    SYNCD_LOG_DEBUG("Check ControlBlock vid = %d pro=%d state=%d \n", vid, priority, state);
    /* update vlanInfo control block*/
    vlanInfo[SURVEI_VLAN_IDX].valid = state;
    vlanInfo[SURVEI_VLAN_IDX].vid = vid;
    vlanInfo[SURVEI_VLAN_IDX].priority = priority;

    /* avoid invalid vlan set to SDK*/
    if ((0 == vid) && (MACVLAN_STATE_ENABLE == state))
    {
        SYNCD_LOG_DEBUG("Invalid VlanInfo vid(%d)", vid);
        return MW_E_OK;
    }

    for (i = 0; i < MAX_OUI_NUM; i++)
    {
        SYNCD_LOG_DEBUG("CB_idx[%d] [%x:%x:%x] type = %d \n", i, ouiEntry[i].macOUI[0], ouiEntry[i].macOUI[1], ouiEntry[i].macOUI[2], ouiEntry[i].type);
        if (VLAN_MAC_SURVEIL_ENABLE != ouiEntry[i].type)
        {
            SYNCD_LOG_DEBUG("vlan type is not surveillance vlan \n ");
            continue;
        }

        ptr_mac_addr = (VOVLAN_OUI_T *)ouiEntry[i].macOUI;
        if (!memcmp(ptr_mac_addr, &ouiMask[3], sizeof(VOVLAN_OUI_T))) /* OUI entry CB is null*/
        {
            SYNCD_LOG_DEBUG("not entries \r\n ");
            continue;
        }
        entryIdx = i;

        rc = air_vlan_getMacBasedVlan(0, entryIdx, &entry);
        if (AIR_E_OK == rc)
        {
            rc = air_vlan_delMacBasedVlan(0, entryIdx);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Remove MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }

        if (MACVLAN_STATE_DISABLE != state)
        {
            memset(&mac_oui, 0 , sizeof(MW_MAC_T));
            memcpy(&mac_oui, ptr_mac_addr, sizeof(VOVLAN_OUI_T));

            memset(&entry, 0, sizeof(AIR_VLAN_MAC_BASED_ENTRY_T));
            memcpy(&entry.mac, mac_oui, sizeof(MW_MAC_T));
            entry.vid = vid;
            entry.pri = priority;
            memcpy(&entry.mac_mask, ouiMask, sizeof(MW_MAC_T));

            rc = air_vlan_addMacBasedVlan(0, entryIdx, &entry);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Set MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    return MW_E_OK;
}
#endif

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
/* FUNCTION NAME:   syncd_api_macvlan_setMacOUIEntry
 * PURPOSE:
 *    Set mac oui entry from DB
 *
 * INPUT:
 *      ptr_api_arg --  porinter to API arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_macvlan_setMacOUIEntry(
    const SYNCD_API_ARG_T *ptr_api_arg)
{

    AIR_ERROR_NO_T rc;
    AIR_VLAN_MAC_BASED_ENTRY_T entry;
    UI16_T e_idx;
    UI16_T expect_entry_num;
    UI8_T *ptr_data = NULL, method;
    UI8_T entryIdx = 0, cbIdx = 0;
    MW_MAC_T mac_oui;
    UI8_T *ptr_vlanType = NULL, vlanInfoIdx = 0;

    MW_CHECK_PTR(ptr_api_arg);
    _init_controlBlock();
    e_idx = ptr_api_arg->ptr_type->e_idx;
    method = ptr_api_arg->method;
    expect_entry_num = (DB_ALL_ENTRIES == e_idx) ? MAX_OUI_NUM : 1;

    ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
    ptr_vlanType = ptr_data + (sizeof(VOVLAN_OUI_T) * expect_entry_num);

    if (M_GET == method)
    {
        for (e_idx = 0; e_idx < MAX_OUI_NUM; e_idx++)
        {
            /* Update Control block */
            memset(&mac_oui, 0 , sizeof(MW_MAC_T));
            memcpy(&mac_oui, &ptr_data[(e_idx * 3)], sizeof(VOVLAN_OUI_T));
            memcpy(ouiEntry[e_idx].macOUI, &mac_oui, sizeof(ouiEntry[e_idx].macOUI));
            ouiEntry[e_idx].type = ptr_vlanType[e_idx];
            if (VLAN_MAC_NONE == ptr_vlanType[e_idx])
            {
                continue;
            }

#if (defined(AIR_SUPPORT_VOICE_VLAN)) && (defined(AIR_SUPPORT_SURVEILLANCE_VLAN))
            vlanInfoIdx = (VLAN_MAC_SURVEIL_ENABLE == ptr_vlanType[e_idx])? SURVEI_VLAN_IDX: VOICE_VLAN_IDX;
#elif defined(AIR_SUPPORT_VOICE_VLAN)
            vlanInfoIdx = VOICE_VLAN_IDX;
#elif defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
            vlanInfoIdx = SURVEI_VLAN_IDX;
#endif

            if((0 == vlanInfo[vlanInfoIdx].valid) || (0 == vlanInfo[vlanInfoIdx].vid))
            {
                /* If vlanInfo not ready, only update OUI control block */
                SYNCD_LOG_DEBUG(" vlanInfo not ready, Idx:%d \n", e_idx);
                continue;
            }
            entryIdx = e_idx;
            SYNCD_LOG_DEBUG("Check CB[%d] MacOUI[%x:%x:%x] type=%d \n ", e_idx, ouiEntry[e_idx].macOUI[0], ouiEntry[e_idx].macOUI[1], ouiEntry[e_idx].macOUI[2], ouiEntry[e_idx].type);
            _syncd_api_macvlan_transform(&entry, vlanInfoIdx, (const MW_MAC_T *)&mac_oui, method);

            rc = air_vlan_addMacBasedVlan(0, entryIdx, &entry);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Set MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }

        return MW_E_OK;
    }
    else if ((M_UPDATE == ptr_api_arg ->method) || (M_DELETE == ptr_api_arg ->method))
    {
        /* Receive M_CREATE from DB means create a MAC base vlan entry */
        if (e_idx == DB_ALL_ENTRIES)
        {
            SYNCD_LOG_ERROR("%s: Cannot update/delete all oui entries at one time\n ", __func__);
            return MW_E_OP_INCOMPLETE;
        }
        cbIdx = (e_idx - 1);
        entryIdx = cbIdx;

        if (M_DELETE == method)
        {
            SYNCD_LOG_DEBUG("Check CB[%d] MacOUI[%x:%x:%x] type=%d \n ", cbIdx, ouiEntry[cbIdx].macOUI[0], ouiEntry[cbIdx].macOUI[1], ouiEntry[cbIdx].macOUI[2], ouiEntry[cbIdx].type);
            rc = air_vlan_delMacBasedVlan(0, entryIdx);
            if (AIR_E_OK == rc)
            {
                memset(&ouiEntry[entryIdx], 0, sizeof(MAC_OUI_CONTROL_BLOCK_T));
                return MW_E_OK;
            }
            SYNCD_LOG_ERROR("%s: Remove MAC base vlan failed(%d) \n ", __func__, rc);
            return MW_E_OP_INCOMPLETE;
        }

        /* Update Control block */
        memset(&mac_oui, 0 , sizeof(MW_MAC_T));
        memcpy(&mac_oui, ptr_data, sizeof(VOVLAN_OUI_T));
        memcpy(ouiEntry[cbIdx].macOUI, &mac_oui, sizeof(ouiEntry[cbIdx].macOUI));
        ouiEntry[cbIdx].type = *ptr_vlanType;
        if (VLAN_MAC_NONE == *ptr_vlanType)
        {
            SYNCD_LOG_ERROR("%s: Cannot add a None type oui entry\n ", __func__);
            return MW_E_OP_INCOMPLETE;
        }

#if (defined(AIR_SUPPORT_VOICE_VLAN)) && (defined(AIR_SUPPORT_SURVEILLANCE_VLAN))
        vlanInfoIdx = (VLAN_MAC_SURVEIL_ENABLE == *ptr_vlanType)? SURVEI_VLAN_IDX: VOICE_VLAN_IDX;
#elif defined(AIR_SUPPORT_VOICE_VLAN)
        vlanInfoIdx = VOICE_VLAN_IDX;
#elif defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
        vlanInfoIdx = SURVEI_VLAN_IDX;
#endif

        SYNCD_LOG_DEBUG("Check CB[%d] MacOUI[%x:%x:%x] type=%d \n ", cbIdx, ouiEntry[cbIdx].macOUI[0], ouiEntry[cbIdx].macOUI[1], ouiEntry[cbIdx].macOUI[2], ouiEntry[cbIdx].type);

        _syncd_api_macvlan_transform(&entry, vlanInfoIdx, (const MW_MAC_T *)&mac_oui, method);

        if (0 != vlanInfo[vlanInfoIdx].vid) /* avoid invalid vlan set to SDK*/
        {
            rc = air_vlan_addMacBasedVlan(0, entryIdx, &entry);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("%s: Set MAC base vlan failed(%d) \n ", __func__, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("%s: Method(%X) is not supported\n", __func__, ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}
#endif
