/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  igmp_snoop_lag.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snoop LAG functionality.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_lag.h"
#include "igmp_snoop.h"
#include "igmp_snoop_log.h"
#include "igmp_snoop_port.h"
#ifdef AIR_SUPPORT_LACP
#include "air_lag.h"
#endif
#include "syncd_api_lag.h"

/* NAMING CONSTANT DECLARATIONS
 */

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
static IGMP_SNP_LAG_INFO_T _igmp_snp_lag_info;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: igmp_snp_lag_initVariable
 * PURPOSE:
 *      Initialize the IGMP SNP LAG variables.
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
MW_ERROR_NO_T
igmp_snp_lag_initVariable(
    void)
{
    osapi_memset(&_igmp_snp_lag_info, 0, sizeof(IGMP_SNP_LAG_INFO_T));
    return MW_E_OK;
}

/* FUNCTION NAME:   igmp_snp_lag_getMinMemberPortByLagId
 * PURPOSE:
 *      This API is used to get the minimum member port of a specific LAG.
 *
 * INPUT:
 *      lag_id       --  LAG ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Minimum member port of the LAG
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_getMinMemberPortByLagId(
    UI32_T  lag_id)
{
    UI8_T                   min_member = 0;
    UI32_T                  member_idx = 0;
#ifdef AIR_SUPPORT_LACP
    I32_T                   rc = MW_E_OK;
    UI32_T                  max_group_cnt = 0, max_member_cnt = 0, member_cnt = 0, unit = 0;
    UI32_T                  *ptr_member = NULL;
#endif

    if ((0 == lag_id) || (MAX_TRUNK_NUM < lag_id))
    {
        return 0;
    }

    if(0 == (_igmp_snp_lag_info.lag[(lag_id - 1)]).member_bmp)
    {
        return 0;
    }

#ifdef AIR_SUPPORT_LACP
    if(LAG_MODE_STATIC != (_igmp_snp_lag_info.lag[(lag_id - 1)]).mode)
    {
        if(0 == _igmp_snp_lag_info.aggregated_mbr[(lag_id - 1)])
        {
            return 0;
        }
        rc = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
        if (AIR_E_OK != rc)
        {
            return 0;
        }
        osapi_calloc(sizeof(UI32_T) * max_member_cnt, IGMP_SNP_MODULE_NAME, (void**)&ptr_member);
        if (NULL == ptr_member)
        {
            return 0;
        }
        rc = air_lag_getMember(unit, (lag_id - 1), &member_cnt, ptr_member);
        if (AIR_E_OK != rc)
        {
            MW_FREE(ptr_member);
            return 0;
        }
        min_member = ptr_member[0];
        MW_FREE(ptr_member);
    }
    else
#endif /* AIR_SUPPORT_LACP */
    {
        for(member_idx = 1; member_idx <= PLAT_MAX_PORT_NUM; member_idx++)
        {
            if(0 != ((_igmp_snp_lag_info.lag[(lag_id - 1)]).member_bmp & BIT(member_idx - 1)))
            {
                min_member = member_idx;
                break;
            }
        }
    }

    IGMP_SNP_LOG_DEBUG("lag_idx:%d, min_member:%d", lag_id, min_member);
    return min_member;
}

/* FUNCTION NAME:   igmp_snp_lag_getMembers
 * PURPOSE:
 *      This API is used to get the member port bitmap of a specific LAG.
 *
 * INPUT:
 *      lag_id       --  LAG ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The member port bitmap of the specified LAG.
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_getMembers(
    UI32_T lag_id)
{
    UI32_T              lag_member = 0;

    if ((0 == lag_id) || (MAX_TRUNK_NUM < lag_id))
    {
        return 0;
    }

#ifdef AIR_SUPPORT_LACP
    if((LAG_MODE_LACP_ACTIVE == (_igmp_snp_lag_info.lag[(lag_id - 1)]).mode) ||
        (LAG_MODE_LACP_PASSIVE == (_igmp_snp_lag_info.lag[(lag_id - 1)]).mode))
    {
        lag_member = _igmp_snp_lag_info.aggregated_mbr[(lag_id - 1)];
    }
    else
#endif /* AIR_SUPPORT_LACP */
    {
        lag_member = (_igmp_snp_lag_info.lag[(lag_id - 1)]).member_bmp;
    }

    return lag_member;
}

/* FUNCTION NAME:   igmp_snp_lag_updatePortBmpWithLagInfo
 * PURPOSE:
 *      This API is used to get the member port bitmap of a specific LAG.
 *
 * INPUT:
 *      PortMap      --  Pointer to the port bitmap to be updated
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The port bitmap of updated by LAG information
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_updatePortBmpWithLagInfo(
    UI32_T PortMap)
{
    UI8_T               i = 0;
    UI32_T              newPortMap = PortMap;
    UI32_T              lag_member = 0;

    for(i = 0; i < MAX_TRUNK_NUM; i++)
    {
        /* Bit 0 in the trunk member bitmap represents port 1 not port 0, refer to DB table TRUNK_PORT, TRUNK_MEMBER */
        lag_member = ((_igmp_snp_lag_info.lag[i]).member_bmp << 1);
        if(0 != (newPortMap & lag_member))
        {
            newPortMap |= lag_member;
        }
    }

    return newPortMap;
}

/* FUNCTION NAME:   igmp_snp_lag_updateLinkStatusBmpWithLagInfo
 * PURPOSE:
 *      This API is used to update the port bitmap based on the link status and lag information.
 *
 * INPUT:
 *      PortMap   --  port bit map of to be update.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map
 *
 * NOTES:
 *      A bit value of 1 does not guarantee that the port is linkup. The LAG contains the port may be link up.
 *      To determine the actual link status of a port, use igmp_snp_port_getPortBmpLinkStatus().
 */
UI32_T
igmp_snp_lag_updateLinkStatusBmpWithLagInfo(
    UI32_T PortMap)
{
    UI32_T newPortMap = 0, lagPortMap = 0, linkstatsPortMap = 0;

    lagPortMap = igmp_snp_lag_updatePortBmpWithLagInfo(PortMap);
    linkstatsPortMap = igmp_snp_port_getPortBmpLinkStatus(lagPortMap);
    newPortMap = (PortMap & igmp_snp_lag_updatePortBmpWithLagInfo(linkstatsPortMap));
    IGMP_SNP_LOG_DEBUG("PortMap:0x%x, newPortMap:0x%x, lagPortMap:0x%x, linkstatsPortMap:0x%x",
                                                PortMap, newPortMap, lagPortMap, linkstatsPortMap);

    return newPortMap;
}

/* FUNCTION NAME:   igmp_snp_lag_getPortLagId
 * PURPOSE:
 *      This API is used to get the LAG id to which a port belongs.
 *
 * INPUT:
 *      port_id       --  the port id
 *
 * OUTPUT:
 *      lag_id        --  the LAG id which a port belongs
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      If the port is not in the trunk, the LAG id '0' returned
 */
UI32_T
igmp_snp_lag_getPortLagId(
    UI32_T  port_id)
{
    UI32_T  lag_id = 0, i = 0, lag_member = 0;

    if(0 == port_id)
    {
        return port_id;
    }
    for(i = 0; i < MAX_TRUNK_NUM; i++)
    {
        lag_member = igmp_snp_lag_getMembers(i + 1);
        if(0 != ((1 << (port_id - 1)) & lag_member))
        {
            lag_id = (i + 1);
            break;
        }
    }

    return lag_id;
}

/* FUNCTION NAME:   igmp_snp_lag_getMinMemberPortByPortId
 * PURPOSE:
 *      This API is used to get the minimum member port of a specific port.
 *
 * INPUT:
 *      port_id       --  the port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      min_port      --  port of minimum LAG port id.
 *
 * NOTES:
 *      If the port is not in the LAG, the port itself is returned
 */
UI32_T
igmp_snp_lag_getMinMemberPortByPortId(
    UI32_T   port_id)
{
    UI32_T             lag_id = 0, min_port = port_id;

    if((0 == port_id) || (MAX_PORT_NUM < port_id))
    {
        return 0;
    }
    lag_id = igmp_snp_lag_getPortLagId(port_id);
    if((0 != lag_id) && (MAX_TRUNK_NUM >= lag_id))
    {
        min_port = igmp_snp_lag_getMinMemberPortByLagId(lag_id);
    }
    IGMP_SNP_LOG_DEBUG("port_id:%d, min_port:%d", port_id, min_port);
    return min_port;
}

/* FUNCTION NAME:   igmp_snp_lag_updateTxBmpBySrcPort
 * PURPOSE:
 *      This API is used to update tx port member based on LAg infomation and source port.
 *
 * INPUT:
 *      sendPortMap   --  port bit map of to send.
 *      srcPort       --  the source port of the packet to be sent.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_updateTxBmpBySrcPort(
    UI32_T sendPortMap,
    UI32_T srcPort)
{
    UI8_T               i = 0;
    UI32_T              idx = 0;
    UI32_T              newPortMap = sendPortMap;
    UI32_T              srcBitMap = (0x01 << srcPort);
    UI32_T              lag_member = 0;

    for(i = 0; i < MAX_TRUNK_NUM; i++)
    {
        /* Bit 0 in the trunk member bitmap represents port 1 not port 0, refer to DB table TRUNK_PORT, TRUNK_MEMBER */
        lag_member = ((_igmp_snp_lag_info.lag[i]).member_bmp << 1);
        if(0 != (newPortMap & lag_member))
        {
            newPortMap = newPortMap & (~ lag_member);
            if(0 != (lag_member & srcBitMap))
            {
                IGMP_SNP_LOG_DEBUG("Src port %d is LAG[%d]:0x%x member.", srcPort, i, lag_member);
                continue;
            }
            for(idx = 1; idx <= PLAT_MAX_PORT_NUM; idx++)
            {
                if(0 != (lag_member & (0x01 << idx)))
                {
                    newPortMap |= (0x01 << idx);
                }
            }
        }
    }

    return newPortMap;
}

/* FUNCTION NAME: igmp_snp_lag_dumpInfo
 * PURPOSE:
 *      Dump IGMP SNP LAG information.
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
igmp_snp_lag_dumpInfo(
    void)
{
    UI32_T  i = 0;

    MW_CMD_OUTPUT("\tLAG INFO:\n");
    for(i = 0; i < MAX_TRUNK_NUM; i++)
    {
        MW_CMD_OUTPUT("\t[%d]: lag_id: %d, mode: %d, portbmp: 0x%x, min_mbr: %d",
                    i, (i + 1),
                    (_igmp_snp_lag_info.lag[i]).mode,
                    (_igmp_snp_lag_info.lag[i]).member_bmp,
                    igmp_snp_lag_getMinMemberPortByLagId(i + 1));

#ifdef AIR_SUPPORT_LACP
        MW_CMD_OUTPUT(", aggregated_mbr=0x%x", _igmp_snp_lag_info.aggregated_mbr[i]);
#endif /* AIR_SUPPORT_LACP */
        MW_CMD_OUTPUT("\n");
    }
    return;
}

/* FUNCTION NAME:   igmp_snp_lag_getConfig
 * PURPOSE:
 *      This API is used for MW to get static variable of igmp_snp LAG.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of _igmp_snp_lag_info
 *
 * NOTES:
 *      None
 */
IGMP_SNP_LAG_INFO_T *
igmp_snp_lag_getConfig(
    void)
{
    return (IGMP_SNP_LAG_INFO_T *)&_igmp_snp_lag_info;
}