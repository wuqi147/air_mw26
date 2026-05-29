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

/* FILE NAME:  menu.c
 * PURPOSE:
 *  Implement CGI/SSI handler function for add/remove specific webpage in menu.html.
 *
 * NOTES:
 *
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "web.h"
#include "httpd_queue.h"
#ifdef AIR_SUPPORT_POE
#include "poe_api.h"
#endif

MW_ERROR_NO_T
ssi_get_menu_option_info_Handle(
    int *length,
    struct tcp_pcb *pcb,
    unsigned int apiflags,
    MENU_OPTION_T opList)
{
    UI16_T len = 0;
    char err = 0;

    switch(opList)
    {
        case MENU_OPTION_DHCPSNOOP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"security_DhcpSnooping\", 0, 0, parent.window.str_menu.dhcpSnooping, 1, 0,");
            break;
        case MENU_OPTION_IGSNP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_IgmpSnoopingRpm\", 0, 0, parent.window.str_menu.igmpSnooping, 1, 0,");
            break;
        case MENU_OPTION_LOOP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_LoopPreventionRpm\", 0, 0, parent.window.str_menu.loopPrevention, 1, 0,");
            break;
        case MENU_OPTION_ICMPC:
            err = send_format_response(&len, pcb, apiflags,
                    "\"tools_icmpClient\", 0, 0, parent.window.str_menu.toolsping, 1, 0,");
            break;
        case MENU_OPTION_VOICE:
            err = send_format_response(&len, pcb, apiflags,
                    "\"vlan_Voice\", 0, 0, parent.window.str_menu.voiceVlan, 1, 0,");
            break;
        case MENU_OPTION_SURVEILLANCE:
            err = send_format_response(&len, pcb, apiflags,
                    "\"vlan_Surveillance\", 0, 0, parent.window.str_menu.surveilVlan, 1, 0,");
            break;
        case MENU_OPTION_SNMP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_SnmpRpm\", 0, 0, parent.window.str_menu.Snmp, 1, 0,");
            break;
        case MENU_OPTION_RSTP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_SpanningTree\", 0, 0, parent.window.str_menu.spanningTree, 1, 0,");
            break;
#ifdef AIR_SUPPORT_MSTP
        case MENU_OPTION_MSTP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_SpanningTreeInstance\", 0, 0, parent.window.str_menu.spanningTreeInstance, 1, 0,");
            break;
#endif
        case MENU_OPTION_STP_SECURITY:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_SpanningTreeSecurity\", 0, 0, parent.window.str_menu.spanningTreeSecurity, 1, 0,");
            break;
        case MENU_OPTION_MQTTD:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_CloudConnection\", 0, 0, parent.window.str_menu.cloudConnect, 1, 0,");
            break;
        case MENU_OPTION_LLDP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_LldpConfigRpm\", 0, 0, parent.window.str_menu.lldpConfig, 1, 0,");
            break;
        case MENU_OPTION_LLDP_NEIGHBOR:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_LldpNeighborRpm\", 0, 0, parent.window.str_menu.lldpNeighbor, 1, 0,");
            break;
        case MENU_OPTION_DDM:
            err = send_format_response(&len, pcb, apiflags,
                    "\"sys_OpticalModuleStatus\", 0, 0, parent.window.str_menu.opticalModuleStatus, 1, 0,");
            break;
#ifdef AIR_SUPPORT_POE
        case MENU_OPTION_POE:
        {
            if (TRUE == poe_checkPseStatus())
            {
                err = send_format_response(&len, pcb, apiflags,
                    "\"poe\", 0, 1, parent.window.str_menu.poe, 1, 2,");
            }
            break;
        }
        case MENU_OPTION_POE_CONFIG:
        {
            if (TRUE == poe_checkPseStatus())
            {
                err = send_format_response(&len, pcb, apiflags,
                    "\"poeConfig\", 0, 0, parent.window.str_menu.poeConfig, 1, 0,");
            }
            break;
        }
#ifdef AIR_SUPPORT_POE_WATCHDOG
        case MENU_OPTION_POE_WATCHDOG:
        {
            if (TRUE == poe_checkPseStatus())
            {
                err = send_format_response(&len, pcb, apiflags,
                    "\"poe_WatchDog\", 0, 0, parent.window.str_menu.poeWatchDog, 1, 0,");
            }
            break;
        }
#endif
#endif
        case MENU_OPTION_DIAG:
            err = send_format_response(&len, pcb, apiflags,
                    "\"tools_Cablediag\", 0, 0, parent.window.str_menu.toolsCablediag, 1, 0,");
            break;
        case MENU_OPTION_SNTP:
            err = send_format_response(&len, pcb, apiflags,
                    "\"cfg_Systime\", 0, 0, parent.window.str_menu.Systime, 1, 0,");
            break;
#ifdef AIR_SUPPORT_ERPS
        case MENU_OPTION_ERPS:
            {
                err = send_format_response(&len, pcb, apiflags,
                            "\"cfg_erps\", 0, 0, parent.window.str_menu.ErpsConfig, 1, 0,");
                break;
            }
#endif
        default:
            break;
    }
    if(err != ERR_OK)
    {
        return err;
    }
    *length = (int)len;

    return MW_E_OK;
}

