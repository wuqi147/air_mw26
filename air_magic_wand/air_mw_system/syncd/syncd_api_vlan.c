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

/* FILE NAME:  syncd_api_vlan.c
 * PURPOSE:
 *  Implement VLAN API function table.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <string.h>
#include <syncd_in.h>
#include <vlan_utils.h>
#include <air_vlan.h>
#include "air_l2.h"
#include <mw_platform.h>
#include "syncd_api_stp.h"
#include "syncd_api_l2.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
*/
/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
/* DATA TYPE DECLARATIONS
*/
/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T syncd_vlan_state = VLAN_NONE;
static SYNCD_VLAN_GLOBAL_T syncd_vlan_g = {0};

/* LOCAL SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
_syncd_api_port_vlan_entry(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI16_T ety_idx, expect_entry_num;
    UI16_T port = 0;
    UI32_T unit = 0, port_untag_mbr = 0, port_tag_mbr = 0;
    AIR_PORT_BITMAP_T vlan_port_bitmap = {0};
    AIR_VLAN_ENTRY_T vlan_entry;

    MW_CHECK_PTR(ptr_api_arg);
    ety_idx = ptr_api_arg->ptr_type->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == ety_idx)? MAX_VLAN_ENTRY_NUM : 1;
    SYNCD_LOG_DEBUG( "ety_idx=%u, expect_entry_num=%u", ety_idx, expect_entry_num);
    /* Check DB_MSG data_size is correct or not */
    if ((VLAN_ENTRY_SIZE * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    if (((M_GET == ptr_api_arg->method) && (DB_ALL_ENTRIES == ety_idx)) || (M_CREATE == ptr_api_arg->method))
    {
        /* No matter DB send what data, create default VLAN 1 with all port member only */
        /* Handle cpu port + user port*/
        vlan_get_plat_max_portBmp(&port_untag_mbr);
        BIT_SET(port_tag_mbr, PLAT_CPU_PORT);
        BITMAP_PORT_ADD(vlan_port_bitmap[0], port_tag_mbr, port_untag_mbr);
        SYNCD_LOG_DEBUG( "port_tag_mbr=0x%x, port_untag_mbr=0x%x, total_mbr=0x%x", port_tag_mbr, port_untag_mbr, vlan_port_bitmap[0]);

        /* Set VLAN entry parameter */
        memset(&vlan_entry, 0, sizeof(AIR_VLAN_ENTRY_T));
        vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
        vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL;
        vlan_entry.port_bitmap[0] = vlan_port_bitmap[0];
        BITMAP_PORT_FOREACH(port_tag_mbr, port)
        {
            vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED;
        }
        BITMAP_PORT_FOREACH(port_untag_mbr, port)
        {
            vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
        }

        /* Create VLAN */
        rc = air_vlan_createVlan(unit, VLAN_DEFAULT_VID, &vlan_entry);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("create VLAN %d failed(%d)", VLAN_DEFAULT_VID, rc);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else if ((M_UPDATE == ptr_api_arg->method) || (M_DELETE == ptr_api_arg->method))
    {
        SYNCD_LOG_DEBUG( "method(%X) do nothing", ptr_api_arg->method);
    }
    else
    {
        SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
_syncd_api_8021q_vlan_entry(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI16_T ety_idx, expect_entry_num;
    UI16_T vid = 0, port = 0;
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    UI8_T fid = 0;
#endif
    UI32_T unit = 0, tag_mbr, untag_mbr;
    void *ptr_data = NULL;
    UI8_T *ptr_vid = NULL;
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    UI8_T *ptr_fid = NULL;
#endif
    UI8_T *ptr_port_mbr = NULL, *ptr_untag_mbr = NULL, *ptr_tag_mbr = NULL;
    AIR_PORT_BITMAP_T vlan_port_bitmap = {0};
    AIR_VLAN_ENTRY_T vlan_entry;

    MW_CHECK_PTR(ptr_api_arg);
    ety_idx = ptr_api_arg->ptr_type->e_idx;
    expect_entry_num = (DB_ALL_ENTRIES == ety_idx)? MAX_VLAN_ENTRY_NUM : 1;
    SYNCD_LOG_DEBUG( "ety_idx=%u, expect_entry_num=%u", ety_idx, expect_entry_num);

    if ((VLAN_ENTRY_SIZE * expect_entry_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_data = ptr_api_arg->ptr_data;
    ptr_vid = ptr_data;
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    ptr_fid = ptr_data + (sizeof(UI16_T) * expect_entry_num);
#endif
    ptr_port_mbr = ptr_data + (sizeof(UI16_T) * expect_entry_num) +
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    (sizeof(UI8_T) * expect_entry_num) +
#endif
    (sizeof(VLAN_DESCR_T) * expect_entry_num);
    ptr_tag_mbr = ptr_port_mbr + (sizeof(UI32_T) * expect_entry_num);
    ptr_untag_mbr = ptr_tag_mbr + (sizeof(UI32_T) * expect_entry_num);

    vid = ptr_vid[0] | (ptr_vid[1] << 8);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    fid = ptr_fid[0];
#endif
    tag_mbr = ptr_tag_mbr[0] | (ptr_tag_mbr[1] << 8) | (ptr_tag_mbr[2] << 16) | (ptr_tag_mbr[3] << 24);
    untag_mbr = ptr_untag_mbr[0] | (ptr_untag_mbr[1] << 8) | (ptr_untag_mbr[2] << 16) | (ptr_untag_mbr[3] << 24);
    SYNCD_LOG_DEBUG( "ptr_data=%p", ptr_data);

    if ((M_GET == ptr_api_arg->method) && (DB_ALL_ENTRIES == ety_idx))
    {
        for (ety_idx = 0; ety_idx < MAX_VLAN_ENTRY_NUM; ety_idx++)
        {
            SYNCD_LOG_DEBUG( "[M_GET] ety_idx=%u", ety_idx);

            /* offset 2 bytes for the type of vid */
            OFFSET_2_BYTES(vid, ptr_vid, (ety_idx * 2));
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            OFFSET_1_BYTES(fid, ptr_fid, ety_idx);
#endif
            /* offset 4 bytes for the type of port_mbr, tag_mbr, untag_mbr */
            OFFSET_4_BYTES(tag_mbr, ptr_tag_mbr, (ety_idx * 4));
            OFFSET_4_BYTES(untag_mbr, ptr_untag_mbr, (ety_idx * 4));

            if (!VLAN_IS_VID_VALID(vid))
            {
                SYNCD_LOG_DEBUG( "invalid VID(%d). Do not handle", vid);
                continue;
            }

            /* Handle cpu port + user port */
            BIT_SET(tag_mbr, PLAT_CPU_PORT);
            BITMAP_PORT_ADD(vlan_port_bitmap[0], tag_mbr, untag_mbr);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            SYNCD_LOG_DEBUG( "vid=%u, fid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, fid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);
#else
            SYNCD_LOG_DEBUG( "vid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);
#endif
            /* Set VLAN entry parameter */
            memset(&vlan_entry, 0, sizeof(AIR_VLAN_ENTRY_T));
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            vlan_entry.fid = fid;
#endif
            vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
            vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL;
            vlan_entry.port_bitmap[0] = vlan_port_bitmap[0];
            BITMAP_PORT_FOREACH(tag_mbr, port)
            {
                vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED;
            }
            BITMAP_PORT_FOREACH(untag_mbr, port)
            {
                vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
            }

            /* Create VLAN */
            rc = air_vlan_createVlan(unit, vid, &vlan_entry);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("create VLAN %d failed(%d)", vid, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else if (M_CREATE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == ety_idx)
        {
            SYNCD_LOG_ERROR("entry id(%u) of notification is wrong", ety_idx);
            return MW_E_BAD_PARAMETER;
        }
        /* offset 2 bytes for the type of vid */
        OFFSET_2_BYTES(vid, ptr_vid, 0);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
        OFFSET_1_BYTES(fid, ptr_fid, 0);
#endif
        /* offset 4 bytes for the type of port_mbr, tag_mbr, untag_mbr */
        OFFSET_4_BYTES(tag_mbr, ptr_tag_mbr, 0);
        OFFSET_4_BYTES(untag_mbr, ptr_untag_mbr, 0);

        if (!VLAN_IS_VID_VALID(vid))
        {
            SYNCD_LOG_ERROR("invalid VID(%d).", vid);
            return MW_E_BAD_PARAMETER;
        }

        /* Handle cpu port + user port */
        BIT_SET(tag_mbr, PLAT_CPU_PORT);
        BITMAP_PORT_ADD(vlan_port_bitmap[0], tag_mbr, untag_mbr);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
        SYNCD_LOG_DEBUG( "vid=%u, fid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, fid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);
#else
        SYNCD_LOG_DEBUG( "vid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);
#endif
        /* Set VLAN entry parameter */
        memset(&vlan_entry, 0, sizeof(AIR_VLAN_ENTRY_T));
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
        vlan_entry.fid = fid;
#endif
        vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_IVL;
        vlan_entry.flags |= AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL;
        vlan_entry.port_bitmap[0] = vlan_port_bitmap[0];
        BITMAP_PORT_FOREACH(tag_mbr, port)
        {
            vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED;
        }
        BITMAP_PORT_FOREACH(untag_mbr, port)
        {
            vlan_entry.egtag_ctl[port] = AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED;
        }

        /* Create VLAN */
        rc = air_vlan_createVlan(unit, vid, &vlan_entry);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("create VLAN %d failed(%d)", vid, rc);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        for (ety_idx = 0; ety_idx < expect_entry_num; ety_idx++)
        {
            SYNCD_LOG_DEBUG( "[M_UPDATE] ety_idx=%u", ety_idx);

            /* offset 2 bytes for the type of vid */
            OFFSET_2_BYTES(vid, ptr_vid, (ety_idx * 2));
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            OFFSET_1_BYTES(fid, ptr_fid, ety_idx);
#endif
            /* offset 4 bytes for the type of port_mbr, tag_mbr, untag_mbr */
            OFFSET_4_BYTES(tag_mbr, ptr_tag_mbr, (ety_idx * 4));
            OFFSET_4_BYTES(untag_mbr, ptr_untag_mbr, (ety_idx * 4));

            if (!VLAN_IS_VID_VALID(vid))
            {
                SYNCD_LOG_DEBUG( "invalid VID(%d). Do not handle", vid);
                continue;
            }

            /* Handle cpu port + user port */
            BIT_SET(tag_mbr, PLAT_CPU_PORT);
            BITMAP_PORT_ADD(vlan_port_bitmap[0], tag_mbr, untag_mbr);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            SYNCD_LOG_DEBUG( "vid=%u, fid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, fid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);

#else
            SYNCD_LOG_DEBUG( "vid=%u, tag_mbr=0x%x, untag_mbr=0x%x, total_mbr=0x%x", vid, tag_mbr, untag_mbr, vlan_port_bitmap[0]);
#endif
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            /* Update fid */
            rc = air_vlan_setFid(unit, vid, fid);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Update fid failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
#endif
            /* Update VLAN members */
            rc = air_vlan_setMember(unit, vid, vlan_port_bitmap);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("Set VLAN members failed(%d)", rc);
                return MW_E_OP_INCOMPLETE;
            }
            /* Update VLAN port egress tag control */
            BITMAP_PORT_FOREACH(tag_mbr, port)
            {
                rc = air_vlan_setPortEgressTagCtrl(unit, vid, port, AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Set VLAN port %d egress tag control failed(%d)", port, rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
            BITMAP_PORT_FOREACH(untag_mbr, port)
            {
                rc = air_vlan_setPortEgressTagCtrl(unit, vid, port, AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("Set VLAN port %d egress untag control failed(%d)", port, rc);
                    return MW_E_OP_INCOMPLETE;
                }
            }
        }
    }
    else if (M_DELETE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == ety_idx)
        {
            SYNCD_LOG_ERROR("entry id(%u) of notification is wrong", ety_idx);
            return MW_E_BAD_PARAMETER;
        }
        /* offset 2 bytes for the type of vid */
        OFFSET_2_BYTES(vid, ptr_vid, 0);
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
        OFFSET_1_BYTES(fid, ptr_fid, 0);
#endif
        SYNCD_LOG_DEBUG( "[M_DELETE] vid=%u", vid);

        if (!VLAN_IS_VID_VALID(vid))
        {
            SYNCD_LOG_ERROR("invalid VID(%d).", vid);
            return MW_E_BAD_PARAMETER;
        }

        /* Delete VLAN */
        rc = air_vlan_destroyVlan(unit, vid);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("delete VLAN failed(%d)", rc);
            return MW_E_OP_INCOMPLETE;
        }
    }
    else
    {
        SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: syncd_api_vlan_pvid
 * PURPOSE:
 *      process Port PVID setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_pvid(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T index, pvid, expect_port_num;
    UI32_T port;
    UI8_T *ptr_pvid = NULL;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg->ptr_type->e_idx;
    expect_port_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG( "eidx=%u, expect_entry_num=%u", index, expect_port_num);

    if ((sizeof(UI16_T) * expect_port_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_pvid = (UI8_T *)ptr_api_arg->ptr_data;
    pvid = htons(get16(ptr_pvid));
    if (!VLAN_IS_VID_VALID(pvid))
    {
        SYNCD_LOG_ERROR("invalid PVID(%d). Do not handle", pvid);
        return MW_E_BAD_PARAMETER;
    }

    if (M_GET == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES != index)
        {
            SYNCD_LOG_ERROR("entry id(%u) of 1st notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }

            pvid = htons(get16(ptr_pvid));
            SYNCD_LOG_DEBUG( "port=%u, pvid=%u", port, pvid);
            rc = air_vlan_setPortCvid(unit, port, pvid);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) pvid failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
            ptr_pvid += 2;
        }
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == index)
        {
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
            {
                if (PLAT_CPU_PORT == port)
                {
                    continue;
                }

                pvid = htons(get16(ptr_pvid));
                SYNCD_LOG_DEBUG( "port=%u, pvid=%u", port, pvid);
                rc = air_vlan_setPortCvid(unit, port, pvid);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("set port(%d) pvid failed(%d)", port, rc);
                    return MW_E_OP_INCOMPLETE;
                }
                ptr_pvid += 2;
            }
        }
        else
        {
            port = (UI32_T)index;
            pvid = htons(get16(ptr_pvid));
            SYNCD_LOG_DEBUG( "port=%u, pvid=%u", port, pvid);
            rc = air_vlan_setPortCvid(unit, port, pvid);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) pvid failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_vlan_mode
 * PURPOSE:
 *      process Port ingress check setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_mode(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T index, expect_port_num;
    UI32_T port;
    UI8_T *ptr_incheck = NULL;
    BOOL_T vlan_mode = FALSE;

    MW_CHECK_PTR(ptr_api_arg);
    index = ptr_api_arg->ptr_type->e_idx;
    expect_port_num = (DB_ALL_ENTRIES == index)? PLAT_MAX_PORT_NUM : 1;
    SYNCD_LOG_DEBUG( "eidx=%u, expect_entry_num=%u", index, expect_port_num);

    if ((sizeof(UI8_T) * expect_port_num) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_incheck = (UI8_T *)ptr_api_arg->ptr_data;

    if (M_GET == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES != index)
        {
            SYNCD_LOG_ERROR("entry id(%u) of 1st notification is wrong", index);
            return MW_E_BAD_PARAMETER;
        }

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            vlan_mode = (1 == *ptr_incheck) ? TRUE : FALSE;

            SYNCD_LOG_DEBUG( "port=%u, vlan_mode=%u", port, vlan_mode);
            rc = air_vlan_setIngressVlanFilter(unit, port, vlan_mode);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) ingress vlan filter failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
            ptr_incheck++;
        }
    }
    else if (M_UPDATE == ptr_api_arg->method)
    {
        if (DB_ALL_ENTRIES == index)
        {
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
            {
                if (PLAT_CPU_PORT == port)
                {
                    continue;
                }
                vlan_mode = (1 == *ptr_incheck) ? TRUE : FALSE;
                SYNCD_LOG_DEBUG( "port=%u, vlan_mode=%u", port, vlan_mode);
                rc = air_vlan_setIngressVlanFilter(unit, port, vlan_mode);
                if (AIR_E_OK != rc)
                {
                    SYNCD_LOG_ERROR("set port(%d) ingress vlan filter failed(%d)", port, rc);
                    return MW_E_OP_INCOMPLETE;
                }
                ptr_incheck++;
            }
        }
        else
        {
            port = (UI32_T)index;
            vlan_mode = (1 == *ptr_incheck) ? TRUE : FALSE;
            SYNCD_LOG_DEBUG( "port=%u, vlan_mode=%u", port, vlan_mode);
            rc = air_vlan_setIngressVlanFilter(unit, port, vlan_mode);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) ingress vlan filter failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_vlan_config
 * PURPOSE:
 *      process VLAN state config setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_config(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI32_T unit = 0;
    UI16_T e_id = 0, port = 0;
    AIR_VLAN_PORT_ATTR_T port_attr = AIR_VLAN_PORT_ATTR_LAST;
    BOOL_T vlan_mode = FALSE;
    DB_VLAN_CFG_INFO_T *ptr_cfg = NULL;

    MW_CHECK_PTR(ptr_api_arg);
    e_id = (ptr_api_arg->ptr_type->e_idx);
    SYNCD_LOG_DEBUG( "eidx=%u", e_id);

    if (sizeof(DB_VLAN_CFG_INFO_T) != ptr_api_arg->data_size)
    {
        SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
        return MW_E_BAD_PARAMETER;
    }

    ptr_cfg = (DB_VLAN_CFG_INFO_T *)ptr_api_arg->ptr_data;

    if ((M_GET == ptr_api_arg->method && DB_ALL_ENTRIES == e_id) ||
        (M_UPDATE == ptr_api_arg->method && DB_ALL_ENTRIES == e_id))
    {
        if ((FALSE == ptr_cfg->enable_8021q_b) && ((TRUE == ptr_cfg->enable_port_b) || (TRUE == ptr_cfg->enable_mtu)))
        {
            port_attr = AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT;
            vlan_mode = FALSE;
            syncd_vlan_state = VLAN_PORT_ENABLE;
        }
        else if ((FALSE == ptr_cfg->enable_port_b) && (TRUE == ptr_cfg->enable_8021q_b) && (FALSE == ptr_cfg->enable_mtu))
        {
            port_attr = AIR_VLAN_PORT_ATTR_USER_PORT;
            vlan_mode = FALSE;
            syncd_vlan_state = VLAN_1Q_ENABLE;
        }
        else
        {
            SYNCD_LOG_ERROR("vlan state config[%u|%u] data is wrong.", ptr_cfg->enable_port_b, ptr_cfg->enable_8021q_b);
            return MW_E_NOT_SUPPORT;
        }
        SYNCD_LOG_DEBUG( "set port_attr=%d, vlan_mode=%d", port_attr, vlan_mode);

        /* Handle CPU port */
        /* No matter in port-based VLAN mode or 1Q VLAN mode,
         * CPU port should set in USER port since CPU port need to identify VLAN Tag.
         */
        rc = air_vlan_setPortAttr(unit, PLAT_CPU_PORT, AIR_VLAN_PORT_ATTR_USER_PORT);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("set cpu-port(%d) attr failed(%d)", PLAT_CPU_PORT, rc);
            return MW_E_OP_INCOMPLETE;
        }
        rc = air_vlan_setIngressVlanFilter(unit, PLAT_CPU_PORT, vlan_mode);
        if (AIR_E_OK != rc)
        {
            SYNCD_LOG_ERROR("set cpu-port(%d) attr failed(%d)", PLAT_CPU_PORT, rc);
            return MW_E_OP_INCOMPLETE;
        }

        /* Handle User port */
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }

            rc = air_vlan_setPortAttr(unit, port, port_attr);
            if (AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) port_attr failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
            rc = air_vlan_setIngressVlanFilter(unit, port, vlan_mode);
            if(AIR_E_OK != rc)
            {
                SYNCD_LOG_ERROR("set port(%d) vlan_mode failed(%d)", port, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
    }
    else
    {
        SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
        return MW_E_NOT_SUPPORT;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_vlan_entry
 * PURPOSE:
 *      process VLAN entry setting
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_entry(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;

    if ((VLAN_PORT_ENABLE != syncd_vlan_state) && (VLAN_1Q_ENABLE != syncd_vlan_state))
    {
        SYNCD_LOG_ERROR("VLAN State(%u) is wrong", syncd_vlan_state);
        return MW_E_NOT_SUPPORT;
    }

    if (VLAN_PORT_ENABLE == syncd_vlan_state)
    {
        rc = _syncd_api_port_vlan_entry(ptr_api_arg);
    }
    else if (VLAN_1Q_ENABLE == syncd_vlan_state)
    {
        rc = _syncd_api_8021q_vlan_entry(ptr_api_arg);
    }

    return rc;
}

/* FUNCTION NAME: syncd_api_vlan_matrix
 * PURPOSE:
 *      Set port isolation setting from DB.
 * INPUT:
 *      ptr_api_arg --  pointer to API arguments
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NOT_SUPPORT
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_vlan_matrix(
    const SYNCD_API_ARG_T *ptr_api_arg)
{
    AIR_ERROR_NO_T rc;
    UI16_T idx, expect_port_num, t_idx, f_idx;
    UI32_T port, i = 0;
    UI32_T unit = 0, port_matrix = 0;
    UI8_T *ptr_data = NULL, loop_state = 0;
    AIR_PORT_BITMAP_T port_bitmap = {0};
#ifdef AIR_LP_USE_STP_BLOCK
    MW_ERROR_NO_T     ret;
#endif

    MW_CHECK_PTR(ptr_api_arg);
    t_idx = ptr_api_arg->ptr_type->t_idx;
    f_idx = ptr_api_arg->ptr_type->f_idx;
    if ((PORT_CFG_INFO == t_idx) && (PORT_ISOLATION== f_idx))
    {
        idx = ptr_api_arg->ptr_type->e_idx;
        expect_port_num = (DB_ALL_ENTRIES == idx)? PLAT_MAX_PORT_NUM : 1;
        SYNCD_LOG_DEBUG( "eidx=%u, expect_entry_num=%u", idx, expect_port_num);

        if ((sizeof(UI32_T) * expect_port_num) != ptr_api_arg->data_size)
        {
            SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
            return MW_E_BAD_PARAMETER;
        }

        ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
        if ((M_GET == ptr_api_arg->method) || (M_UPDATE == ptr_api_arg->method))
        {
            idx = 0;
            for(i = 0; i < expect_port_num; i++)
            {
                port = ((DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx) ? (i + 1): ptr_api_arg->ptr_type->e_idx);
                OFFSET_4_BYTES(port_matrix, ptr_data, (idx * 4));
                port_matrix |= (1 << port);
                syncd_vlan_g.portmatrix[port] = port_matrix;
                SYNCD_LOG_DEBUG( "port=%d, port_matrix=0x%x, blockpbmp=0x%x", port, port_matrix, syncd_vlan_g.blockpbmp);
                if(0 != (syncd_vlan_g.blockpbmp & (0x1 << port)))
                {
                    port_matrix = 0;
                }
                else
                {
                    port_matrix &= (~syncd_vlan_g.blockpbmp);
                }
                BIT_SET(port_matrix, PLAT_CPU_PORT);
                BITMAP_PORT_ADD(port_bitmap[0], port_matrix, 0);
                SYNCD_LOG_DEBUG( "port=%d, matrix-bmp=0x%x", port, port_bitmap[0]);
                if(M_UPDATE == ptr_api_arg->method)
                {
                    rc = air_port_setPortMatrix(unit, port, port_bitmap);
                    if (AIR_E_OK != rc)
                    {
                        SYNCD_LOG_ERROR("set port(%d) matrix failed(%d)", port, rc);
                        return MW_E_OP_INCOMPLETE;
                    }
                }
                /* Else if method is M_GET, don't set port matrix. It will be set within syncd timer handler */
                idx++;
            }
        }
        else
        {
            SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
            return MW_E_NOT_SUPPORT;
        }
    }
    else if ((PORT_OPER_INFO == t_idx) && (PORT_LOOP_STATE== f_idx))
    {
        idx = ptr_api_arg->ptr_type->e_idx;
        expect_port_num = (DB_ALL_ENTRIES == idx)? PLAT_MAX_PORT_NUM : 1;
        SYNCD_LOG_DEBUG( "eidx=%u, expect_entry_num=%u", idx, expect_port_num);

        if ((sizeof(UI8_T) * expect_port_num) != ptr_api_arg->data_size)
        {
            SYNCD_LOG_ERROR("data_size(%d) is wrong", ptr_api_arg->data_size);
            return MW_E_BAD_PARAMETER;
        }

        ptr_data = (UI8_T *)ptr_api_arg->ptr_data;
        if ((M_GET == ptr_api_arg->method) || (M_UPDATE == ptr_api_arg->method))
        {
            for(i = 0; i < expect_port_num; i++)
            {
                idx = ((DB_ALL_ENTRIES == ptr_api_arg->ptr_type->e_idx) ? (i + 1): ptr_api_arg->ptr_type->e_idx);
                osapi_memcpy(&loop_state, ((UI8_T*)ptr_data + i), sizeof(UI8_T));
                if(1 == loop_state)
                {
                    if(0 == (syncd_vlan_g.blockpbmp & (0x1 << idx)))
                    {
                        syncd_vlan_g.blockpbmp |= (0x1 << idx);
#ifndef AIR_LP_USE_STP_BLOCK
                        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
                        {
                            if (PLAT_CPU_PORT == port)
                            {
                                continue;
                            }
                            if(idx == port)
                            {
                                AIR_PORT_BITMAP_CLEAR(port_bitmap);
                                AIR_PORT_ADD(port_bitmap, PLAT_CPU_PORT);
                                air_port_setPortMatrix(unit, idx, port_bitmap);
                            }
                            else
                            {
                                air_port_getPortMatrix(unit, port, port_bitmap);
                                AIR_PORT_DEL(port_bitmap, idx);
                                air_port_setPortMatrix(unit, port, port_bitmap);
                            }
                        }
#else
                        ret = syncd_api_stp_setPortStateByUser(idx, AIR_STP_STATE_LISTEN, SYNCD_API_STP_USER_LP);
                        if (MW_E_OK != ret)
                        {
                            SYNCD_LOG_ERROR("LP block port[%d] failed, rc(%d)", idx, ret);
                        }
#endif /* AIR_LP_USE_STP_BLOCK */
                        SYNCD_LOG_DEBUG( "blockpbmp=0x%x", syncd_vlan_g.blockpbmp);
                        if(M_UPDATE == ptr_api_arg->method)
                        {
                            syncd_api_flushMacAddr(unit, idx, SYNCD_API_L2_FLUSH_PORT);
                        }
                    }
                }
                else
                {
                    if(0 != (syncd_vlan_g.blockpbmp & (0x1 << idx)))
                    {
                        syncd_vlan_g.blockpbmp &= (~(0x1 << idx));
#ifndef AIR_LP_USE_STP_BLOCK
                        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
                        {
                            if (PLAT_CPU_PORT == port)
                            {
                                continue;
                            }

                            SYNCD_LOG_DEBUG( "port=%d, db port_matrix=0x%x, blockpbmp=0x%x", port, syncd_vlan_g.portmatrix[port], syncd_vlan_g.blockpbmp);
                            if(0 != (syncd_vlan_g.blockpbmp & (0x1 << port)))
                            {
                                port_matrix = 0;
                            }
                            else
                            {
                                port_matrix = syncd_vlan_g.portmatrix[port] & (~syncd_vlan_g.blockpbmp);
                            }

                            BIT_SET(port_matrix, PLAT_CPU_PORT);
                            SYNCD_LOG_DEBUG( "port=%d, port_matrix=0x%x", port, port_matrix);
                            BITMAP_PORT_ADD(port_bitmap[0], port_matrix, 0);
                            if(M_UPDATE == ptr_api_arg->method)
                            {
                                air_port_setPortMatrix(unit, port, port_bitmap);
                            }
                            /* Else if method is M_GET, don't set port matrix. It will be set within syncd timer handler */
                        }
#else
                        ret = syncd_api_stp_setPortStateByUser(idx, AIR_STP_STATE_FORWARD, SYNCD_API_STP_USER_LP);
                        if (MW_E_OK != ret)
                        {
                            SYNCD_LOG_ERROR("LP unblock port[%d] failed, rc(%d)", idx, ret);
                        }
#endif /* AIR_LP_USE_STP_BLOCK */
                    }
                }
            }
        }
        else
        {
            SYNCD_LOG_ERROR("method(%X) is not supported", ptr_api_arg->method);
            return MW_E_NOT_SUPPORT;
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_api_update_port_matrix
 * PURPOSE:
 *      This API is used to update the port matrix.
 * INPUT:
 *      portbmp     --  Port bit map of to be updated
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_api_update_port_matrix(
    MW_PORT_BITMAP_T portbmp)
{
    UI32_T              port = 0;
    UI32_T              unit = 0, port_matrix = 0;
    AIR_PORT_BITMAP_T   port_bitmap = {0};

    AIR_PORT_FOREACH(portbmp, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }

#ifndef AIR_LP_USE_STP_BLOCK
        SYNCD_LOG_DEBUG( "port=%d, db port_matrix=0x%x, blockpbmp=0x%x", port, syncd_vlan_g.portmatrix[port], syncd_vlan_g.blockpbmp);
        if(0 != (syncd_vlan_g.blockpbmp & (0x1 << port)))
        {
            port_matrix = 0;
        }
        else
        {
            port_matrix = syncd_vlan_g.portmatrix[port] & (~syncd_vlan_g.blockpbmp);
        }
#else
        port_matrix = syncd_vlan_g.portmatrix[port];
#endif
        BIT_SET(port_matrix, PLAT_CPU_PORT);
        SYNCD_LOG_DEBUG( "port=%d, port_matrix=0x%x", port, port_matrix);
        BITMAP_PORT_ADD(port_bitmap[0], port_matrix, 0);
        air_port_setPortMatrix(unit, port, port_bitmap);
    }
    return MW_E_OK;
}
