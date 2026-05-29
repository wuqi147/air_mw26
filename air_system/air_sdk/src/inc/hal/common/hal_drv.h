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

/* FILE NAME:  hal_drv.h
 * PURPOSE:
 *  Provide HAL driver structure and driver help APIs.
 *
 * NOTES:
 */

#ifndef HAL_DRV_H
#define HAL_DRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_acl.h>
#include <air_dos.h>
#include <air_error.h>
#include <air_ifmon.h>
#include <air_init.h>
#include <air_ipmc.h>
#include <air_l2.h>
#include <air_lag.h>
#include <air_mib.h>
#include <air_mirror.h>
#include <air_perif.h>
#include <air_port.h>
#include <air_qos.h>
#include <air_sec.h>
#include <air_sif.h>
#include <air_stag.h>
#include <air_stp.h>
#include <air_svlan.h>
#include <air_swc.h>
#include <air_types.h>
#include <air_vlan.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_RUN_CHIP_MODE (0x0)
#define HAL_RUN_FPGA_MODE (0x1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct HAL_CHIP_INIT_PARAM_S
{
    void *port_info;
} HAL_CHIP_INIT_PARAM_T;

/* chip multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_CHIP_READDEVICEINFO_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_device_id,
    UI32_T      *ptr_revision_id);

typedef AIR_ERROR_NO_T (*HAL_CHIP_INIT_FUNC_T)(
    const UI32_T           unit,
    HAL_CHIP_INIT_PARAM_T *ptr_chip_init_param);

typedef AIR_ERROR_NO_T (*HAL_CHIP_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef struct
{
    HAL_CHIP_READDEVICEINFO_FUNC_T hal_chip_readDeviceInfo;
    HAL_CHIP_INIT_FUNC_T           hal_chip_init;
    HAL_CHIP_DEINIT_FUNC_T         hal_chip_deinit;
} HAL_CHIP_FUNC_VEC_T;

/* stp multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_STP_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_STP_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_STP_SETPORTSTATE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          fid,
    const AIR_STP_STATE_T state);

typedef AIR_ERROR_NO_T (*HAL_STP_GETPORTSTATE_FUNC_T)(
    const UI32_T     unit,
    const UI32_T     port,
    const UI32_T     fid,
    AIR_STP_STATE_T *ptr_state);

/* mirror multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_MIRROR_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_ADDSESSION_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_DELSESSION_FUNC_T)(
    const UI32_T unit,
    const UI32_T session);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_GETSESSION_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_SETSESSIONADMINMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T state);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_GETSESSIONADMINMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T      *state);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_SETMIRRORPORT_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             session_id,
    const AIR_MIR_SESSION_T *ptr_session);

typedef AIR_ERROR_NO_T (*HAL_MIRROR_GETMIRRORPORT_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       session_id,
    AIR_MIR_SESSION_T *ptr_session);

/* CPU port multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_CPU_SETPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_CPU_GETPORT_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_port,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_CPU_INITPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

/* lag multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_LAG_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_LAG_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_LAG_GETMAXGROUPCNT_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_group_cnt,
    UI32_T      *ptr_member_cnt);

typedef AIR_ERROR_NO_T (*HAL_LAG_CREATEGROUP_FUNC_T)(
    const UI32_T unit,
    const UI32_T lag_id);

typedef AIR_ERROR_NO_T (*HAL_LAG_DESTROYGROUP_FUNC_T)(
    const UI32_T unit,
    const UI32_T lag_id);

typedef AIR_ERROR_NO_T (*HAL_LAG_GETGROUP_FUNC_T)(
    const UI32_T unit,
    const UI32_T lag_id,
    BOOL_T      *ptr_state);

typedef AIR_ERROR_NO_T (*HAL_LAG_SETMEMBER_FUNC_T)(
    const UI32_T      unit,
    const UI32_T      lag_id,
    const UI32_T      member_cnt,
    const AIR_PORT_T *ptr_member);

typedef AIR_ERROR_NO_T (*HAL_LAG_GETMEMBER_FUNC_T)(
    const UI32_T unit,
    const UI32_T lag_id,
    UI32_T      *member_cnt,
    AIR_PORT_T  *ptr_member);

typedef AIR_ERROR_NO_T (*HAL_LAG_SETHASHCONTROL_FUNC_T)(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl);

typedef AIR_ERROR_NO_T (*HAL_LAG_GETHASHCONTROL_FUNC_T)(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl);

/*ipmc multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_IPMC_ADDMCASTADDR_FUNC_T)(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETMCASTADDR_FUNC_T)(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_DELMCASTADDR_FUNC_T)(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_DELALLMCASTADDR_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_IPMC_ADDMCASTMEMBER_FUNC_T)(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_DELMCASTMEMBER_FUNC_T)(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETMCASTMEMBERCNT_FUNC_T)(
    const UI32_T      unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    UI32_T           *ptr_count);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETMCASTBUCKETSIZE_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_size);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETFIRSTMCASTADDR_FUNC_T)(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETNEXTMCASTADDR_FUNC_T)(
    const UI32_T          unit,
    AIR_IPMC_MATCH_TYPE_T match_type,
    UI32_T               *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T     *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_IPMC_SETIGMPSNOOPVERSION_FUNC_T)(
    const UI32_T          unit,
    const AIR_IPMC_TYPE_T type);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETIGMPSNOOPVERSION_FUNC_T)(
    const UI32_T     unit,
    AIR_IPMC_TYPE_T *ptr_type);

typedef AIR_ERROR_NO_T (*HAL_IPMC_SETPORTIPMCMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETPORTIPMCMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_IPMC_SETPORTLOOKUPIPTYPECTRL_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    const BOOL_T        enable);

typedef AIR_ERROR_NO_T (*HAL_IPMC_GETPORTLOOKUPIPTYPECTRL_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    const AIR_IP_TYPE_T ip_type,
    BOOL_T             *ptr_enable);

/* Port multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_PORT_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_PORT_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETSGMIIMODE_FUNC_T)(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETSGMIIMODE_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETANMODE_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETANMODE_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETLOCALADV_FUNC_T)(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETLOCALADV_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETREMOTEADV_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETSPEED_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETSPEED_FUNC_T)(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETDUPLEX_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETDUPLEX_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETBCKPRES_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETBCKPRES_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_bckPres);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETFLOWCTRL_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETFLOWCTRL_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETLPBACK_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              lpBack_en);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETLPBACK_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_lpBack_en);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETLINK_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETSTATUS_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETSTATUS_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_state);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPSMODE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_PS_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPSMODE_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_PS_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETSMTSPDDWN_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETSMTSPDDWN_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPORTMATRIX)(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPORTMATRIX)(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T ptr_port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETVLANMODE)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETVLANMODE)(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *p_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYLEDONCTRL)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYLEDONCTRL)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETCOMBOMODE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETCOMBOMODE_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYLEDCTRLMODE_FUNC_T)(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYLEDCTRLMODE_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYLEDCTRLMODE_FUNC_T)(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYLEDCTRLMODE_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYLEDFORCESTATE_FUNC_T)(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   led_id,
    const AIR_PORT_PHY_LED_STATE_T state);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYLEDFORCESTATE_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    AIR_PORT_PHY_LED_STATE_T *ptr_state);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYLEDFORCEPATT_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const AIR_PORT_PHY_LED_PATT_T pattern);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYLEDFORCEPATT_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    AIR_PORT_PHY_LED_PATT_T *ptr_pattern);

typedef AIR_ERROR_NO_T (*HAL_PORT_TRIGGERCABLETEST_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETCABLETESTRAWDATA_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

typedef AIR_ERROR_NO_T (*HAL_PORT_SETPHYOPMODE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_OP_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_GETPHYOPMODE_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_OP_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_PORT_TRIGGERLINKDOWNCABLETEST_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

typedef AIR_ERROR_NO_T (*HAL_PORT_DUMPDEBUGINFO_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

/* PoE multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_POE_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_POE_DEINIT_FUNC_T)(
    const UI32_T unit);

/* Mib multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_MIB_CLEARALLCNT)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_MIB_CLEARPORTCNT)(
    const UI32_T unit,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_MIB_GETPORTCNT)(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_MIB_CNT_RX_T *ptr_rx_mib,
    AIR_MIB_CNT_TX_T *ptr_tx_mib);

typedef AIR_ERROR_NO_T (*HAL_MIB_GETFEATURECNT)(
    const UI32_T   unit,
    const UI32_T   port,
    AIR_MIB_CNT_T *ptr_mib);

/* Define multiplexing functions end */

/* L2 multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_L2_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_L2_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_L2_ADDMACADDR_FUNC_T)(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mt);

typedef AIR_ERROR_NO_T (*HAL_L2_DELMACADDR_FUNC_T)(
    const UI32_T           unit,
    const AIR_MAC_ENTRY_T *ptr_mt);

typedef AIR_ERROR_NO_T (*HAL_L2_GETMACADDR_FUNC_T)(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mt);

typedef AIR_ERROR_NO_T (*HAL_L2_GETNEXTMACADDR_FUNC_T)(
    const UI32_T     unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mt);

typedef AIR_ERROR_NO_T (*HAL_L2_SEARCHMACADDR_FUNC_T)(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

typedef AIR_ERROR_NO_T (*HAL_L2_SEARCHNEXTMACADDR_FUNC_T)(
    const UI32_T                   unit,
    const AIR_L2_MAC_SEARCH_TYPE_T type,
    const UI32_T                   value,
    UI8_T                         *ptr_count,
    AIR_MAC_ENTRY_T               *ptr_mac_entry);

typedef AIR_ERROR_NO_T (*HAL_L2_CLEARMACADDR_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_L2_SETMACADDRAGEOUT_FUNC_T)(
    const UI32_T unit,
    const UI32_T age_time);

typedef AIR_ERROR_NO_T (*HAL_L2_GETMACADDRAGEOUT_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_age_time);

typedef AIR_ERROR_NO_T (*HAL_L2_SETMACADDRAGEOUTMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T mode);

typedef AIR_ERROR_NO_T (*HAL_L2_GETMACADDRAGEOUTMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_L2_GETMACBUCKETSIZE_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_num);

typedef AIR_ERROR_NO_T (*HAL_L2_SETFORWARDMODE_FUNC_T)(
    const UI32_T               unit,
    const AIR_FORWARD_TYPE_T   type,
    const AIR_FORWARD_ACTION_T action,
    const AIR_PORT_BITMAP_T    port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_L2_GETFORWARDMODE_FUNC_T)(
    const UI32_T             unit,
    const AIR_FORWARD_TYPE_T type,
    AIR_FORWARD_ACTION_T    *action,
    AIR_PORT_BITMAP_T        port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_L2_FLUSHMACADDR_FUNC_T)(
    const UI32_T                  unit,
    const AIR_L2_MAC_FLUSH_TYPE_T type,
    const UI32_T                  value);

typedef AIR_ERROR_NO_T (*HAL_L2_LOCKL2FDBRESOURCE_FUNC_T)(
    const UI32_T unit);

#ifdef AIR_EN_L2_SHADOW
typedef AIR_ERROR_NO_T (*HAL_L2_UNLOCKL2FDBRESOURCE_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_L2_TRAVERSEMACADDR_FUNC_T)(
    const UI32_T                 unit,
    const AIR_L2_TRAVERSE_FUNC_T callback,
    void                        *ptr_cookie);

typedef AIR_ERROR_NO_T (*HAL_L2_REGISTERMACADDRNOTIFYCALLBACK_FUNC_T)(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie);

typedef AIR_ERROR_NO_T (*HAL_L2_DEREGISTERMACADDRNOTIFYCALLBACK_FUNC_T)(
    const UI32_T                   unit,
    const AIR_L2_MAC_NOTIFY_FUNC_T notify_func,
    void                          *ptr_cookie);

typedef AIR_ERROR_NO_T (*HAL_L2_SETSHADOWCFG_FUNC_T)(
    const UI32_T unit,
    const BOOL_T polling_en,
    const UI32_T interval);

typedef AIR_ERROR_NO_T (*HAL_L2_GETSHADOWCFG_FUNC_T)(
    const UI32_T unit,
    BOOL_T      *ptr_polling_en,
    UI32_T      *ptr_interval);

#endif /* End of AIR_EN_L2_SHADOW */
/* L2 multiplexing functions end */

/* vlan multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_VLAN_CREATE)(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_VLAN_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DESTROY)(
    const UI32_T unit,
    const UI16_T vid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETVLAN)(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_VLAN_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETFID)(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T fid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETFID)(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_fid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_ADDMEMBERPORT)(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DELMEMBERPORT)(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETMEMBERPORT)(
    const UI32_T            unit,
    const UI16_T            vid,
    const AIR_PORT_BITMAP_T port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETMEMBERPORT)(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_PORT_BITMAP_T port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETIVL)(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETIVL)(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETINGRESSTAGKEEPING)(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETINGRESSTAGKEEPING)(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETEGSTAGCTLENABLE)(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETEGSTAGCTLENABLE)(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTEGSTAGCTL)(
    const UI32_T                            unit,
    const UI16_T                            vid,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_ctl);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTEGSTAGCTL)(
    const UI32_T                       unit,
    const UI16_T                       vid,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_tag_ctl);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTACCEPTFRAMETYPE)(
    const UI32_T                       unit,
    const UI32_T                       port,
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTACCEPTFRAMETYPE)(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *ptr_type);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTATTR)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_VLAN_PORT_ATTR_T attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTATTR)(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_VLAN_PORT_ATTR_T *ptr_attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETIGRPORTTAGATTR)(
    const UI32_T                      unit,
    const UI32_T                      port,
    const AIR_IGR_PORT_EGS_TAG_ATTR_T attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETIGRPORTTAGATTR)(
    const UI32_T                 unit,
    const UI32_T                 port,
    AIR_IGR_PORT_EGS_TAG_ATTR_T *ptr_attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTEGSTAGATTR)(
    const UI32_T                            unit,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTEGSTAGATTR)(
    const UI32_T                       unit,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_attr);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTPVID)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T pvid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTPVID)(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_pvid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_INIT)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DEINIT)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETCOPYPRIORITY)(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETCOPYPRIORITY)(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETUSERPRIORITY)(
    const UI32_T unit,
    const UI16_T vid,
    const UI8_T  pri);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETUSERPRIORITY)(
    const UI32_T unit,
    const UI16_T vid,
    UI8_T       *ptr_pri);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETSTAG)(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T stag);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETSTAG)(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_stag);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPORTPSVID)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T psvid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPORTPSVID)(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_psvid);

typedef AIR_ERROR_NO_T (*HAL_VLAN_ADDMACBASEDVLAN)(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETMACBASEDVLAN)(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DELMACBASEDVLAN)(
    const UI32_T unit,
    const UI32_T idx);

typedef AIR_ERROR_NO_T (*HAL_VLAN_ADDSUBNETBASEDVLAN)(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETSUBNETBASEDVLAN)(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DELSUBNETBASEDVLAN)(
    const UI32_T unit,
    const UI32_T idx);

typedef AIR_ERROR_NO_T (*HAL_VLAN_ADDPROTOCOLBASEDVLAN)(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPROTOCOLBASEDVLAN)(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_VLAN_DELPROTOCOLBASEDVLAN)(
    const UI32_T unit,
    const UI32_T idx);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETPROTOCOLBASEDVLANPORTATTR)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    const UI16_T vid,
    const UI16_T pri);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETPROTOCOLBASEDVLANPORTATTR)(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    UI16_T      *ptr_vid,
    UI16_T      *ptr_pri);

typedef AIR_ERROR_NO_T (*HAL_VLAN_SETINGRESSVLANFILTER)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_VLAN_GETINGRESSVLANFILTER)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* vlan multiplexing functions end */

/* qos multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_QOS_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_QOS_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETRATELIMITENABLE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             enable);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETRATELIMITENABLE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETRATELIMITCFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETRATELIMITCFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETRATELIMITEXCLUDEMGMT_FUNC_T)(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             exclude);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETRATELIMITEXCLUDEMGMT_FUNC_T)(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_exclude);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETTRUSTMODE_FUNC_T)(
    const UI32_T               unit,
    const AIR_QOS_TRUST_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETTRUSTMODE_FUNC_T)(
    const UI32_T          unit,
    AIR_QOS_TRUST_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETPRITOQUEUE_FUNC_T)(
    const UI32_T unit,
    const UI32_T pri,
    const UI32_T queue);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETPRITOQUEUE_FUNC_T)(
    const UI32_T unit,
    const UI32_T pri,
    UI32_T      *ptr_queue);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETDSCPTOPRI_FUNC_T)(
    const UI32_T unit,
    const UI32_T dscp,
    const UI32_T pri);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETDSCPTOPRI_FUNC_T)(
    const UI32_T unit,
    const UI32_T dscp,
    UI32_T      *ptr_pri);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETSCHEDULEMODE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             queue,
    const AIR_QOS_SCH_MODE_T sch_mode,
    const UI32_T             weight);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETSCHEDULEMODE_FUNC_T)(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        queue,
    AIR_QOS_SCH_MODE_T *ptr_sch_mode,
    UI32_T             *ptr_weight);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETSHAPERMODE_FUNC_T)(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       queue,
    const AIR_QOS_SHAPER_MODE_T        min,
    const AIR_QOS_SHAPER_MODE_T        max,
    const AIR_QOS_EXCEED_SHAPER_MODE_T exceed);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETSHAPERMODE_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  queue,
    AIR_QOS_SHAPER_MODE_T        *ptr_min,
    AIR_QOS_SHAPER_MODE_T        *ptr_max,
    AIR_QOS_EXCEED_SHAPER_MODE_T *ptr_exceed);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETSHAPERRATE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETSHAPERRATE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETSHAPERWEIGHT_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  queue,
    const AIR_QOS_SHAPER_WEIGHT_T weight);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETSHAPERWEIGHT_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             queue,
    AIR_QOS_SHAPER_WEIGHT_T *ptr_weight);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETPORTPRIORITY_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pri);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETPORTPRIORITY_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_pri);

typedef AIR_ERROR_NO_T (*HAL_QOS_SETPORTTRUSTMODE_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_QOS_TRUST_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_QOS_GETPORTTRUSTMODE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_QOS_TRUST_MODE_T *ptr_mode);
/* qos multiplexing functions end */

/* swc multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_SWC_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SWC_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETMGMTFRAMECFG_FUNC_T)(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETMGMTFRAMECFG_FUNC_T)(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETPORTMGMTFRAMECFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETPORTMGMTFRAMECFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETSYSTEMMAC_FUNC_T)(
    const UI32_T    unit,
    const AIR_MAC_T mac);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETSYSTEMMAC_FUNC_T)(
    const UI32_T unit,
    AIR_MAC_T    mac);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETJUMBO_FUNC_T)(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T frame_len);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETJUMBO_FUNC_T)(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_frame_len);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETPROPERTY_FUNC_T)(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETPROPERTY_FUNC_T)(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETLOOPDETECTCTRL_FUNC_T)(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETLOOPDETECTCTRL_FUNC_T)(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                         *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_SWC_SETLOOPDETECTFRAMESRCMAC_FUNC_T)(
    const UI32_T    unit,
    const AIR_MAC_T mac);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETLOOPDETECTFRAMESRCMAC_FUNC_T)(
    const UI32_T unit,
    AIR_MAC_T    mac);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETLOOPDETECTSTATUS_FUNC_T)(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_SWC_CLEARLOOPDETECTSTATUS_FUNC_T)(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETGLOBALFREEPAGES_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_fp_cnt,
    UI32_T      *ptr_min_fp_cnt);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETPORTALLOCATEDPAGES_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_fp_cnt);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETPORTUSEDPAGES_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T queue,
    const UI32_T mode,
    UI32_T      *ptr_cnt);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETCAPACITY_FUNC_T)(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETUSAGE_FUNC_T)(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

typedef AIR_ERROR_NO_T (*HAL_SWC_GETCHIPUID_FUNC_T)(
    const UI32_T unit,
    UI64_T      *ptr_uid);
/* swc multiplexing functions end */

/* Security multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_SEC_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SEC_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SEC_SET8021XGLOBALMODE_FUNC_T)(
    const UI32_T               unit,
    const AIR_SEC_8021X_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_SEC_GET8021XGLOBALMODE_FUNC_T)(
    const UI32_T          unit,
    AIR_SEC_8021X_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_SEC_SET8021XGLOBALCFG_FUNC_T)(
    const UI32_T               unit,
    const AIR_SEC_8021X_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_GET8021XGLOBALCFG_FUNC_T)(
    const UI32_T         unit,
    AIR_SEC_8021X_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_SET8021XMACBASEDCFG_FUNC_T)(
    const UI32_T                   unit,
    const AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_GET8021XMACBASEDCFG_FUNC_T)(
    const UI32_T             unit,
    AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_SET8021XPORTBASEDCFG_FUNC_T)(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_GET8021XPORTBASEDCFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETMACLIMITGLOBALMODE_FUNC_T)(
    const UI32_T unit,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETMACLIMITGLOBALMODE_FUNC_T)(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETMACLIMITGLOBALCFG_FUNC_T)(
    const UI32_T                   unit,
    const AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETMACLIMITGLOBALCFG_FUNC_T)(
    const UI32_T             unit,
    AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETMACLIMITPORTCFG_FUNC_T)(
    const UI32_T                        unit,
    const UI32_T                        port,
    const AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETMACLIMITPORTCFG_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETPORTSTORMCTRL_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    const BOOL_T               enable);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETPORTSTORMCTRL_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    BOOL_T                    *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETPORTSTORMCTRLRATE_FUNC_T)(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_STORM_TYPE_T      type,
    const AIR_SEC_STORM_RATE_MODE_T mode,
    const UI32_T                    rate);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETPORTSTORMCTRLRATE_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    AIR_SEC_STORM_RATE_MODE_T *ptr_mode,
    UI32_T                    *ptr_rate);

typedef AIR_ERROR_NO_T (*HAL_SEC_SETSTORMCTRLMGMTMODE_FUNC_T)(
    const UI32_T                    unit,
    const AIR_SEC_STORM_CTRL_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_SEC_GETSTORMCTRLMGMTMODE_FUNC_T)(
    const UI32_T               unit,
    AIR_SEC_STORM_CTRL_MODE_T *ptr_mode);

/* Security multiplexing functions end */

/* sFlow multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_SFLOW_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SFLOW_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SFLOW_SETSAMPLING_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T rate,
    const UI32_T number);

typedef AIR_ERROR_NO_T (*HAL_SFLOW_GETSAMPLING_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_rate,
    UI32_T      *ptr_number);

typedef AIR_ERROR_NO_T (*HAL_SFLOW_SETMNGFRM_FUNC_T)(
    const UI32_T unit,
    const BOOL_T state);

typedef AIR_ERROR_NO_T (*HAL_SFLOW_GETMNGFRM_FUNC_T)(
    const UI32_T unit,
    BOOL_T      *ptr_state);
/* sFlow multiplexing functions end */

/* Svlan multiplexing functions */
typedef AIR_ERROR_NO_T (*HAL_SVLAN_ADDSVLAN_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETSVLAN_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_SVLAN_ENTRY_T *ptr_entry);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_DELSVLAN_FUNC_T)(
    const UI32_T unit,
    const UI32_T idx);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_SETCUSTOMPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETCUSTOMPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *enable);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_SETSERVICEPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETSERVICEPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    BOOL_T      *enable);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_SETEGSTPID_0_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETEGSTPID_0_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *tpid);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_SETEGSTPID_1_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETEGSTPID_1_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *tpid);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_SETSTAGTPID_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    const UI32_T tpid);

typedef AIR_ERROR_NO_T (*HAL_SVLAN_GETSTAGTPID_FUNC_T)(
    const UI32_T unit,
    const UI32_T portId,
    UI32_T      *tpid);
/* Svlan multiplexing functions end */

/* STAG multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_STAG_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_STAG_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_STAG_SETPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T stag_en);

typedef AIR_ERROR_NO_T (*HAL_STAG_GETPORT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *stag_en);

typedef AIR_ERROR_NO_T (*HAL_STAG_SETMODE_FUNC_T)(
    const UI32_T          unit,
    const UI32_T          port,
    const AIR_STAG_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_STAG_GETMODE_FUNC_T)(
    const UI32_T     unit,
    const UI32_T     port,
    AIR_STAG_MODE_T *mode);

typedef AIR_ERROR_NO_T (*HAL_STAG_ENCODETXSTAG_FUNC_T)(
    const UI32_T          unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T   *ptr_stag_tx,
    UI8_T                *ptr_buf,
    UI32_T               *ptr_len);

typedef AIR_ERROR_NO_T (*HAL_STAG_DECODERXSTAG_FUNC_T)(
    const UI32_T        unit,
    const UI8_T        *ptr_buf,
    const UI32_T        len,
    AIR_STAG_RX_PARA_T *ptr_stag_rx);
/* STAG multiplexing functions end */

/* DoS multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_DOS_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_DOS_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_DOS_SETGLOBALCFG_FUNC_T)(
    const UI32_T unit,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETGLOBALCFG_FUNC_T)(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_DOS_SETICMPLENGTHLIMIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T length);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETICMPLENGTHLIMIT_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_length);

typedef AIR_ERROR_NO_T (*HAL_DOS_SETTCPFLAGCFG_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETTCPFLAGCFG_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_SETPORTACTIONCFG_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETPORTACTIONCFG_FUNC_T)(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_CLEARPORTACTIONCFG_FUNC_T)(
    const UI32_T unit,
    const UI32_T port);

typedef AIR_ERROR_NO_T (*HAL_DOS_CLEARACTIONCFG_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_DOS_SETRATELIMITCFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETRATELIMITCFG_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg);

typedef AIR_ERROR_NO_T (*HAL_DOS_CLEARRATELIMITCFG_FUNC_T)(
    const UI32_T unit,
    const UI32_T rate_cfg_idx);

typedef AIR_ERROR_NO_T (*HAL_DOS_CLEARALLRATELIMITCFG_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_DOS_GETDROPCNT_FUNC_T)(
    const UI32_T unit,
    UI32_T      *ptr_drop_cnt);

typedef AIR_ERROR_NO_T (*HAL_DOS_CLEARDROPCNT_FUNC_T)(
    const UI32_T unit);
/* DoS multiplexing functions end */
/* acl multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_ACL_INIT_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_ACL_DEINIT_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETGLOBALSTATE_T)(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETGLOBALSTATE_T)(
    const UI32_T unit,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETPORTSTATE_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETPORTSTATE_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_CLEARALL_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETRULE_T)(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETRULE_T)(
    const UI32_T    unit,
    const UI32_T    entry_idx,
    AIR_ACL_RULE_T *ptr_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_DELRULE_T)(
    const UI32_T unit,
    const UI32_T entry_idx);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETACTION_T)(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETACTION_T)(
    const UI32_T      unit,
    const UI32_T      entry_idx,
    AIR_ACL_ACTION_T *ptr_action);

typedef AIR_ERROR_NO_T (*HAL_ACL_DELACTION_T)(
    const UI32_T unit,
    const UI32_T entry_idx);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETBLOCKSTATE_T)(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    BOOL_T               *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETBLOCKSTATE_T)(
    const UI32_T          unit,
    const AIR_ACL_BLOCK_T block,
    const BOOL_T          enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETMIBCNT_T)(
    const UI32_T unit,
    const UI32_T cnt_index,
    UI32_T      *ptr_cnt);

typedef AIR_ERROR_NO_T (*HAL_ACL_CLEARMIBCNT_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETUDFRULE_T)(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETUDFRULE_T)(
    const UI32_T        unit,
    const UI8_T         entry_idx,
    AIR_ACL_UDF_RULE_T *ptr_udf_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_DELUDFRULE_T)(
    const UI32_T unit,
    const UI8_T  entry_idx);

typedef AIR_ERROR_NO_T (*HAL_ACL_CLEARUDFRULE_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETMETERTABLE_T)(
    const UI32_T unit,
    const UI32_T meter_id,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_rate);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETMETERTABLE_T)(
    const UI32_T unit,
    const UI32_T meter_id,
    const BOOL_T enable,
    const UI32_T rate);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETDROPEN_T)(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETDROPEN_T)(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETDROPTHRSH_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_high,
    UI32_T                  *ptr_low);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETDROPTHRSH_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             high,
    const UI32_T             low);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETDROPPBB_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    UI32_T                  *ptr_pbb);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETDROPPBB_T)(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T              queue,
    const UI32_T             pbb);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETDROPEXMFRM_T)(
    const UI32_T unit,
    BOOL_T      *ptr_type);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETDROPEXMFRM_T)(
    const UI32_T unit,
    const BOOL_T type);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETTRTCMEN_T)(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETTRTCMEN_T)(
    const UI32_T unit,
    BOOL_T       enable);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETTRTCMTABLE_T)(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETTRTCMTABLE_T)(
    const UI32_T     unit,
    const UI32_T     tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETL3ROUTINGTABLE_T)(
    const UI32_T unit,
    const UI32_T mac_sel,
    const UI32_T id,
    AIR_MAC_T    ptr_mac);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETL3ROUTINGTABLE_T)(
    const UI32_T    unit,
    const UI32_T    mac_sel,
    const UI32_T    id,
    const AIR_MAC_T ptr_mac);

typedef AIR_ERROR_NO_T (*HAL_ACL_GETARPRULE_T)(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_SETARPRULE_T)(
    const UI32_T        unit,
    const UI32_T        entry_idx,
    AIR_ACL_ARP_RULE_T *ptr_rule);

typedef AIR_ERROR_NO_T (*HAL_ACL_DELARPRULE_T)(
    const UI32_T unit,
    const UI32_T entry_idx);
/* acl multiplexing functions end */

/* ifmon multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_IFMON_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_IFMON_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_IFMON_REGISTER_FUNC_T)(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

typedef AIR_ERROR_NO_T (*HAL_IFMON_DEREGISTER_FUNC_T)(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

typedef AIR_ERROR_NO_T (*HAL_IFMON_SETMODE_FUNC_T)(
    const UI32_T            unit,
    const AIR_IFMON_MODE_T  mode,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            interval);

typedef AIR_ERROR_NO_T (*HAL_IFMON_GETMODE_FUNC_T)(
    const UI32_T       unit,
    AIR_IFMON_MODE_T  *ptr_mode,
    AIR_PORT_BITMAP_T *ptr_port_bitmap,
    UI32_T            *ptr_interval);

typedef AIR_ERROR_NO_T (*HAL_IFMON_SETMONITORSTATE_FUNC_T)(
    const UI32_T unit,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_IFMON_GETMONITORSTATE_FUNC_T)(
    const UI32_T unit,
    BOOL_T      *ptr_enable);
typedef AIR_ERROR_NO_T (*HAL_IFMON_GETPORTSTATUS_FUNC_T)(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps);
/* ifmon multiplexing functions end */

/* SIF multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_SIF_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SIF_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_SIF_WRITE_FUNC_T)(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

typedef AIR_ERROR_NO_T (*HAL_SIF_READ_FUNC_T)(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

typedef AIR_ERROR_NO_T (*HAL_SIF_WRITEBYREMOTE_FUNC_T)(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_info,
    const AIR_SIF_INFO_T *ptr_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);

typedef AIR_ERROR_NO_T (*HAL_SIF_READBYREMOTE_FUNC_T)(
    const UI32_T          unit,
    const AIR_SIF_INFO_T *ptr_info,
    const AIR_SIF_INFO_T *ptr_remote_info,
    AIR_SIF_PARAM_T      *ptr_sif_param);
/* SIF multiplexing functions end */

/* peripheral multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_PERIF_INIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_PERIF_DEINIT_FUNC_T)(
    const UI32_T unit);

typedef AIR_ERROR_NO_T (*HAL_PERIF_SETGPIODIRECTION_FUNC_T)(
    const UI32_T                     unit,
    const UI32_T                     pin,
    const AIR_PERIF_GPIO_DIRECTION_T direction);

typedef AIR_ERROR_NO_T (*HAL_PERIF_GETGPIODIRECTION_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                pin,
    AIR_PERIF_GPIO_DIRECTION_T *ptr_direction);

typedef AIR_ERROR_NO_T (*HAL_PERIF_SETGPIOOUTPUTDATA_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_DATA_T data);

typedef AIR_ERROR_NO_T (*HAL_PERIF_GETGPIOINPUTDATA_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_DATA_T *ptr_data);

typedef AIR_ERROR_NO_T (*HAL_PERIF_SETGPIOOUTPUTAUTOMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable);

typedef AIR_ERROR_NO_T (*HAL_PERIF_GETGPIOOUTPUTAUTOMODE_FUNC_T)(
    const UI32_T unit,
    const UI32_T pin,
    BOOL_T      *ptr_enable);

typedef AIR_ERROR_NO_T (*HAL_PERIF_SETGPIOOUTPUTAUTOPATT_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_PATT_T pattern);

typedef AIR_ERROR_NO_T (*HAL_PERIF_GETGPIOOUTPUTAUTOPATT_FUNC_T)(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_PATT_T *ptr_pattern);

/* peripheral multiplexing functions end */

typedef struct
{
    HAL_STP_INIT_FUNC_T         hal_stp_init;
    HAL_STP_DEINIT_FUNC_T       hal_stp_deinit;
    HAL_STP_SETPORTSTATE_FUNC_T hal_stp_setPortState;
    HAL_STP_GETPORTSTATE_FUNC_T hal_stp_getPortState;
} HAL_STP_FUNC_VEC_T;

typedef struct
{
    HAL_MIRROR_INIT_FUNC_T                hal_mir_init;
    HAL_MIRROR_DEINIT_FUNC_T              hal_mir_deinit;
    HAL_MIRROR_ADDSESSION_FUNC_T          hal_mir_addSession;
    HAL_MIRROR_DELSESSION_FUNC_T          hal_mir_delSession;
    HAL_MIRROR_GETSESSION_FUNC_T          hal_mir_getSession;
    HAL_MIRROR_SETSESSIONADMINMODE_FUNC_T hal_mir_setSessionAdminMode;
    HAL_MIRROR_GETSESSIONADMINMODE_FUNC_T hal_mir_getSessionAdminMode;
    HAL_MIRROR_SETMIRRORPORT_FUNC_T       hal_mir_setMirrorPort;
    HAL_MIRROR_GETMIRRORPORT_FUNC_T       hal_mir_getMirrorPort;
} HAL_MIR_FUNC_VEC_T;

typedef struct
{
    HAL_PORT_INIT_FUNC_T                     hal_port_init;
    HAL_PORT_DEINIT_FUNC_T                   hal_port_deinit;
    HAL_PORT_SETSGMIIMODE_FUNC_T             hal_port_setSerdesMode;
    HAL_PORT_GETSGMIIMODE_FUNC_T             hal_port_getSerdesMode;
    HAL_PORT_SETANMODE_FUNC_T                hal_port_setPhyAutoNego;
    HAL_PORT_GETANMODE_FUNC_T                hal_port_getPhyAutoNego;
    HAL_PORT_SETLOCALADV_FUNC_T              hal_port_setPhyLocalAdvAbility;
    HAL_PORT_GETLOCALADV_FUNC_T              hal_port_getPhyLocalAdvAbility;
    HAL_PORT_GETREMOTEADV_FUNC_T             hal_port_getPhyRemoteAdvAbility;
    HAL_PORT_SETSPEED_FUNC_T                 hal_port_setSpeed;
    HAL_PORT_GETSPEED_FUNC_T                 hal_port_getSpeed;
    HAL_PORT_SETDUPLEX_FUNC_T                hal_port_setDuplex;
    HAL_PORT_GETDUPLEX_FUNC_T                hal_port_getDuplex;
    HAL_PORT_SETBCKPRES_FUNC_T               hal_port_setBackPressure;
    HAL_PORT_GETBCKPRES_FUNC_T               hal_port_getBackPressure;
    HAL_PORT_SETFLOWCTRL_FUNC_T              hal_port_setFlowCtrl;
    HAL_PORT_GETFLOWCTRL_FUNC_T              hal_port_getFlowCtrl;
    HAL_PORT_SETLPBACK_FUNC_T                hal_port_setPhyLoopBack;
    HAL_PORT_GETLPBACK_FUNC_T                hal_port_getPhyLoopBack;
    HAL_PORT_GETLINK_FUNC_T                  hal_port_getPortStatus;
    HAL_PORT_SETSTATUS_FUNC_T                hal_port_setAdminState;
    HAL_PORT_GETSTATUS_FUNC_T                hal_port_getAdminState;
    HAL_PORT_SETSMTSPDDWN_FUNC_T             hal_port_setPhySmartSpeedDown;
    HAL_PORT_GETSMTSPDDWN_FUNC_T             hal_port_getPhySmartSpeedDown;
    HAL_PORT_SETPORTMATRIX                   hal_port_setPortMatrix;
    HAL_PORT_GETPORTMATRIX                   hal_port_getPortMatrix;
    HAL_PORT_SETVLANMODE                     hal_port_setVlanMode;
    HAL_PORT_GETVLANMODE                     hal_port_getVlanMode;
    HAL_PORT_SETPHYLEDONCTRL                 hal_port_setPhyLedOnCtrl;
    HAL_PORT_GETPHYLEDONCTRL                 hal_port_getPhyLedOnCtrl;
    HAL_PORT_SETCOMBOMODE_FUNC_T             hal_port_setComboMode;
    HAL_PORT_GETCOMBOMODE_FUNC_T             hal_port_getComboMode;
    HAL_PORT_SETPHYLEDCTRLMODE_FUNC_T        hal_port_setPhyLedCtrlMode;
    HAL_PORT_GETPHYLEDCTRLMODE_FUNC_T        hal_port_getPhyLedCtrlMode;
    HAL_PORT_SETPHYLEDFORCESTATE_FUNC_T      hal_port_setPhyLedForceState;
    HAL_PORT_GETPHYLEDFORCESTATE_FUNC_T      hal_port_getPhyLedForceState;
    HAL_PORT_SETPHYLEDFORCEPATT_FUNC_T       hal_port_setPhyLedForcePattCfg;
    HAL_PORT_GETPHYLEDFORCEPATT_FUNC_T       hal_port_getPhyLedForcePattCfg;
    HAL_PORT_TRIGGERCABLETEST_FUNC_T         hal_port_triggerCableTest;
    HAL_PORT_GETCABLETESTRAWDATA_FUNC_T      hal_port_getcableTestRawData;
    HAL_PORT_SETPHYOPMODE_FUNC_T             hal_port_setPhyOpMode;
    HAL_PORT_GETPHYOPMODE_FUNC_T             hal_port_getPhyOpMode;
    HAL_PORT_TRIGGERLINKDOWNCABLETEST_FUNC_T hal_port_triggerLinkDownCableTest;
    HAL_PORT_DUMPDEBUGINFO_FUNC_T            hal_port_dumpDebugInfo;
} HAL_PORT_FUNC_VEC_T;

typedef struct
{
    HAL_CPU_SETPORT_FUNC_T  hal_cpu_setPortCfg;
    HAL_CPU_GETPORT_FUNC_T  hal_cpu_getPortCfg;
    HAL_CPU_INITPORT_FUNC_T hal_cpu_init;
} HAL_CPU_FUNC_VEC_T;

typedef struct
{
    HAL_POE_INIT_FUNC_T   hal_poe_init;
    HAL_POE_DEINIT_FUNC_T hal_poe_deinit;
} HAL_POE_FUNC_VEC_T;

typedef struct
{
    HAL_LAG_INIT_FUNC_T           hal_lag_init;
    HAL_LAG_DEINIT_FUNC_T         hal_lag_deinit;
    HAL_LAG_GETMAXGROUPCNT_FUNC_T hal_lag_getMaxGroupCnt;
    HAL_LAG_CREATEGROUP_FUNC_T    hal_lag_createGroup;
    HAL_LAG_DESTROYGROUP_FUNC_T   hal_lag_destroyGroup;
    HAL_LAG_GETGROUP_FUNC_T       hal_lag_getGroup;
    HAL_LAG_SETMEMBER_FUNC_T      hal_lag_setMember;
    HAL_LAG_GETMEMBER_FUNC_T      hal_lag_getMember;
    HAL_LAG_SETHASHCONTROL_FUNC_T hal_lag_setHashControl;
    HAL_LAG_GETHASHCONTROL_FUNC_T hal_lag_getHashControl;
} HAL_LAG_FUNC_VEC_T;

typedef struct
{
    HAL_IPMC_SETIGMPSNOOPVERSION_FUNC_T     hal_ipmc_setMcastLookupType;
    HAL_IPMC_GETIGMPSNOOPVERSION_FUNC_T     hal_ipmc_getMcastLookupType;
    HAL_IPMC_ADDMCASTADDR_FUNC_T            hal_ipmc_addMcastAddr;
    HAL_IPMC_GETMCASTADDR_FUNC_T            hal_ipmc_getMcastAddr;
    HAL_IPMC_DELMCASTADDR_FUNC_T            hal_ipmc_delMcastAddr;
    HAL_IPMC_DELALLMCASTADDR_FUNC_T         hal_ipmc_delAllMcastAddr;
    HAL_IPMC_ADDMCASTMEMBER_FUNC_T          hal_ipmc_addMcastMember;
    HAL_IPMC_DELMCASTMEMBER_FUNC_T          hal_ipmc_delMcastMember;
    HAL_IPMC_GETMCASTMEMBERCNT_FUNC_T       hal_ipmc_getMcastMemberCnt;
    HAL_IPMC_GETMCASTBUCKETSIZE_FUNC_T      hal_ipmc_getMcastBucketSize;
    HAL_IPMC_GETFIRSTMCASTADDR_FUNC_T       hal_ipmc_getFirstMcastAddr;
    HAL_IPMC_GETNEXTMCASTADDR_FUNC_T        hal_ipmc_getNextMcastAddr;
    HAL_IPMC_SETPORTIPMCMODE_FUNC_T         hal_ipmc_setPortIpmcMode;
    HAL_IPMC_GETPORTIPMCMODE_FUNC_T         hal_ipmc_getPortIpmcMode;
    HAL_IPMC_SETPORTLOOKUPIPTYPECTRL_FUNC_T hal_ipmc_setPortLookupIpTypeCtrl;
    HAL_IPMC_GETPORTLOOKUPIPTYPECTRL_FUNC_T hal_ipmc_getPortLookupIpTypeCtrl;
} HAL_IPMC_FUNC_VEC_T;

typedef struct
{
    HAL_L2_INIT_FUNC_T                 hal_l2_init;
    HAL_L2_DEINIT_FUNC_T               hal_l2_deinit;
    HAL_L2_ADDMACADDR_FUNC_T           hal_l2_addMacAddr;
    HAL_L2_DELMACADDR_FUNC_T           hal_l2_delMacAddr;
    HAL_L2_GETMACADDR_FUNC_T           hal_l2_getMacAddr;
    HAL_L2_GETNEXTMACADDR_FUNC_T       hal_l2_getNextMacAddr;
    HAL_L2_SEARCHMACADDR_FUNC_T        hal_l2_searchMacAddr;
    HAL_L2_SEARCHNEXTMACADDR_FUNC_T    hal_l2_searchNextMacAddr;
    HAL_L2_CLEARMACADDR_FUNC_T         hal_l2_clearMacAddr;
    HAL_L2_SETMACADDRAGEOUT_FUNC_T     hal_l2_setMacAddrAgeOut;
    HAL_L2_GETMACADDRAGEOUT_FUNC_T     hal_l2_getMacAddrAgeOut;
    HAL_L2_SETMACADDRAGEOUTMODE_FUNC_T hal_l2_setMacAddrAgeOutMode;
    HAL_L2_GETMACADDRAGEOUTMODE_FUNC_T hal_l2_getMacAddrAgeOutMode;
    HAL_L2_GETMACBUCKETSIZE_FUNC_T     hal_l2_getMacBucketSize;
    HAL_L2_SETFORWARDMODE_FUNC_T       hal_l2_setForwardMode;
    HAL_L2_GETFORWARDMODE_FUNC_T       hal_l2_getForwardMode;
    HAL_L2_FLUSHMACADDR_FUNC_T         hal_l2_flushMacAddr;
#ifdef AIR_EN_L2_SHADOW
    HAL_L2_LOCKL2FDBRESOURCE_FUNC_T               hal_l2_lockL2FdbResource;
    HAL_L2_UNLOCKL2FDBRESOURCE_FUNC_T             hal_l2_unlockL2FdbResource;
    HAL_L2_TRAVERSEMACADDR_FUNC_T                 hal_l2_traverseMacAddr;
    HAL_L2_REGISTERMACADDRNOTIFYCALLBACK_FUNC_T   hal_l2_registerMacAddrNotifyCallback;
    HAL_L2_DEREGISTERMACADDRNOTIFYCALLBACK_FUNC_T hal_l2_deregisterMacAddrNotifyCallback;
    HAL_L2_SETSHADOWCFG_FUNC_T                    hal_l2_setShadowCfg;
    HAL_L2_GETSHADOWCFG_FUNC_T                    hal_l2_getShadowCfg;
#endif /* End of AIR_EN_L2_SHADOW */
} HAL_L2_FUNC_VEC_T;

typedef struct
{
    HAL_MIB_CLEARALLCNT   hal_mib_clearAllCnt;
    HAL_MIB_CLEARPORTCNT  hal_mib_clearPortCnt;
    HAL_MIB_GETPORTCNT    hal_mib_getPortCnt;
    HAL_MIB_GETFEATURECNT hal_mib_getFeatureCnt;
} HAL_MIB_FUNC_VEC_T;

typedef struct
{
    HAL_VLAN_INIT                         hal_vlan_init;
    HAL_VLAN_DEINIT                       hal_vlan_deinit;
    HAL_VLAN_CREATE                       hal_vlan_create;
    HAL_VLAN_DESTROY                      hal_vlan_destroy;
    HAL_VLAN_GETVLAN                      hal_vlan_getVlan;
    HAL_VLAN_SETFID                       hal_vlan_setFid;
    HAL_VLAN_GETFID                       hal_vlan_getFid;
    HAL_VLAN_ADDMEMBERPORT                hal_vlan_addMemberPort;
    HAL_VLAN_DELMEMBERPORT                hal_vlan_delMemberPort;
    HAL_VLAN_SETMEMBERPORT                hal_vlan_setMemberPort;
    HAL_VLAN_GETMEMBERPORT                hal_vlan_getMemberPort;
    HAL_VLAN_SETIVL                       hal_vlan_setIVL;
    HAL_VLAN_GETIVL                       hal_vlan_getIVL;
    HAL_VLAN_SETINGRESSTAGKEEPING         hal_vlan_setIngressTagKeeping;
    HAL_VLAN_GETINGRESSTAGKEEPING         hal_vlan_getIngressTagKeeping;
    HAL_VLAN_SETEGSTAGCTLENABLE           hal_vlan_setEgsTagCtlEnable;
    HAL_VLAN_GETEGSTAGCTLENABLE           hal_vlan_getEgsTagCtlEnable;
    HAL_VLAN_SETPORTEGSTAGCTL             hal_vlan_setPortEgsTagCtl;
    HAL_VLAN_GETPORTEGSTAGCTL             hal_vlan_getPortEgsTagCtl;
    HAL_VLAN_SETPORTACCEPTFRAMETYPE       hal_vlan_setPortAcceptFrameType;
    HAL_VLAN_GETPORTACCEPTFRAMETYPE       hal_vlan_getPortAcceptFrameType;
    HAL_VLAN_SETPORTATTR                  hal_vlan_setPortAttr;
    HAL_VLAN_GETPORTATTR                  hal_vlan_getPortAttr;
    HAL_VLAN_SETIGRPORTTAGATTR            hal_vlan_setIgrPortTagAttr;
    HAL_VLAN_GETIGRPORTTAGATTR            hal_vlan_getIgrPortTagAttr;
    HAL_VLAN_SETPORTEGSTAGATTR            hal_vlan_setPortEgsTagAttr;
    HAL_VLAN_GETPORTEGSTAGATTR            hal_vlan_getPortEgsTagAttr;
    HAL_VLAN_SETPORTPVID                  hal_vlan_setPortPVID;
    HAL_VLAN_GETPORTPVID                  hal_vlan_getPortPVID;
    HAL_VLAN_SETCOPYPRIORITY              hal_vlan_setCopyPriority;
    HAL_VLAN_GETCOPYPRIORITY              hal_vlan_getCopyPriority;
    HAL_VLAN_SETUSERPRIORITY              hal_vlan_setUserPriority;
    HAL_VLAN_GETUSERPRIORITY              hal_vlan_getUserPriority;
    HAL_VLAN_SETSTAG                      hal_vlan_setStag;
    HAL_VLAN_GETSTAG                      hal_vlan_getStag;
    HAL_VLAN_SETPORTPSVID                 hal_vlan_setPortPSVID;
    HAL_VLAN_GETPORTPSVID                 hal_vlan_getPortPSVID;
    HAL_VLAN_ADDMACBASEDVLAN              hal_vlan_addMacBasedVlan;
    HAL_VLAN_GETMACBASEDVLAN              hal_vlan_getMacBasedVlan;
    HAL_VLAN_DELMACBASEDVLAN              hal_vlan_delMacBasedVlan;
    HAL_VLAN_ADDSUBNETBASEDVLAN           hal_vlan_addSubnetBasedVlan;
    HAL_VLAN_GETSUBNETBASEDVLAN           hal_vlan_getSubnetBasedVlan;
    HAL_VLAN_DELSUBNETBASEDVLAN           hal_vlan_delSubnetBasedVlan;
    HAL_VLAN_ADDPROTOCOLBASEDVLAN         hal_vlan_addProtocolBasedVlan;
    HAL_VLAN_GETPROTOCOLBASEDVLAN         hal_vlan_getProtocolBasedVlan;
    HAL_VLAN_DELPROTOCOLBASEDVLAN         hal_vlan_delProtocolBasedVlan;
    HAL_VLAN_SETPROTOCOLBASEDVLANPORTATTR hal_vlan_setProtocolBasedVlanPortAttr;
    HAL_VLAN_GETPROTOCOLBASEDVLANPORTATTR hal_vlan_getProtocolBasedVlanPortAttr;
    HAL_VLAN_SETINGRESSVLANFILTER         hal_vlan_setIngressVlanFilter;
    HAL_VLAN_GETINGRESSVLANFILTER         hal_vlan_getIngressVlanFilter;
} HAL_VLAN_FUNC_VEC_T;

typedef struct
{
    HAL_QOS_INIT_FUNC_T                    hal_qos_init;
    HAL_QOS_DEINIT_FUNC_T                  hal_qos_deinit;
    HAL_QOS_SETRATELIMITENABLE_FUNC_T      hal_qos_setRateLimitEnable;
    HAL_QOS_GETRATELIMITENABLE_FUNC_T      hal_qos_getRateLimitEnable;
    HAL_QOS_SETRATELIMITCFG_FUNC_T         hal_qos_setRateLimitCfg;
    HAL_QOS_GETRATELIMITCFG_FUNC_T         hal_qos_getRateLimitCfg;
    HAL_QOS_SETRATELIMITEXCLUDEMGMT_FUNC_T hal_qos_setRateLimitExcludeMgmt;
    HAL_QOS_GETRATELIMITEXCLUDEMGMT_FUNC_T hal_qos_getRateLimitExcludeMgmt;
    HAL_QOS_SETTRUSTMODE_FUNC_T            hal_qos_setTrustMode;
    HAL_QOS_GETTRUSTMODE_FUNC_T            hal_qos_getTrustMode;
    HAL_QOS_SETPRITOQUEUE_FUNC_T           hal_qos_setPriToQueue;
    HAL_QOS_GETPRITOQUEUE_FUNC_T           hal_qos_getPriToQueue;
    HAL_QOS_SETDSCPTOPRI_FUNC_T            hal_qos_setDscpToPri;
    HAL_QOS_GETDSCPTOPRI_FUNC_T            hal_qos_getDscpToPri;
    HAL_QOS_SETSCHEDULEMODE_FUNC_T         hal_qos_setScheduleMode;
    HAL_QOS_GETSCHEDULEMODE_FUNC_T         hal_qos_getScheduleMode;
    HAL_QOS_SETSHAPERMODE_FUNC_T           hal_qos_setShaperMode;
    HAL_QOS_GETSHAPERMODE_FUNC_T           hal_qos_getShaperMode;
    HAL_QOS_SETSHAPERRATE_FUNC_T           hal_qos_setShaperRate;
    HAL_QOS_GETSHAPERRATE_FUNC_T           hal_qos_getShaperRate;
    HAL_QOS_SETSHAPERWEIGHT_FUNC_T         hal_qos_setShaperWeight;
    HAL_QOS_GETSHAPERWEIGHT_FUNC_T         hal_qos_getShaperWeight;
    HAL_QOS_SETPORTPRIORITY_FUNC_T         hal_qos_setPortPriority;
    HAL_QOS_GETPORTPRIORITY_FUNC_T         hal_qos_getPortPriority;
    HAL_QOS_SETPORTTRUSTMODE_FUNC_T        hal_qos_setPortTrustMode;
    HAL_QOS_GETPORTTRUSTMODE_FUNC_T        hal_qos_getPortTrustMode;
} HAL_QOS_FUNC_VEC_T;

typedef struct
{
    HAL_SWC_INIT_FUNC_T                     hal_swc_init;
    HAL_SWC_DEINIT_FUNC_T                   hal_swc_deinit;
    HAL_SWC_SETMGMTFRAMECFG_FUNC_T          hal_swc_setMgmtFrameCfg;
    HAL_SWC_GETMGMTFRAMECFG_FUNC_T          hal_swc_getMgmtFrameCfg;
    HAL_SWC_SETSYSTEMMAC_FUNC_T             hal_swc_setSystemMac;
    HAL_SWC_GETSYSTEMMAC_FUNC_T             hal_swc_getSystemMac;
    HAL_SWC_SETJUMBO_FUNC_T                 hal_swc_setJumboSize;
    HAL_SWC_GETJUMBO_FUNC_T                 hal_swc_getJumboSize;
    HAL_SWC_SETPROPERTY_FUNC_T              hal_swc_setProperty;
    HAL_SWC_GETPROPERTY_FUNC_T              hal_swc_getProperty;
    HAL_SWC_SETLOOPDETECTCTRL_FUNC_T        hal_swc_setLoopDetectCtrl;
    HAL_SWC_GETLOOPDETECTCTRL_FUNC_T        hal_swc_getLoopDetectCtrl;
    HAL_SWC_SETLOOPDETECTFRAMESRCMAC_FUNC_T hal_swc_setLoopDetectFrameSrcMac;
    HAL_SWC_GETLOOPDETECTFRAMESRCMAC_FUNC_T hal_swc_getLoopDetectFrameSrcMac;
    HAL_SWC_GETLOOPDETECTSTATUS_FUNC_T      hal_swc_getLoopDetectStatus;
    HAL_SWC_CLEARLOOPDETECTSTATUS_FUNC_T    hal_swc_clearLoopDetectStatus;
    HAL_SWC_GETGLOBALFREEPAGES_FUNC_T       hal_swc_getGlobalFreePages;
    HAL_SWC_GETPORTALLOCATEDPAGES_FUNC_T    hal_swc_getPortAllocatedPages;
    HAL_SWC_GETPORTUSEDPAGES_FUNC_T         hal_swc_getPortUsedPages;
    HAL_SWC_SETPORTMGMTFRAMECFG_FUNC_T      hal_swc_setPortMgmtFrameCfg;
    HAL_SWC_GETPORTMGMTFRAMECFG_FUNC_T      hal_swc_getPortMgmtFrameCfg;
    HAL_SWC_GETCAPACITY_FUNC_T              hal_swc_getCapacity;
    HAL_SWC_GETUSAGE_FUNC_T                 hal_swc_getUsage;
    HAL_SWC_GETCHIPUID_FUNC_T               hal_swc_getChipUid;
} HAL_SWC_FUNC_VEC_T;

typedef struct
{
    HAL_SEC_INIT_FUNC_T                  hal_sec_init;
    HAL_SEC_DEINIT_FUNC_T                hal_sec_deinit;

    HAL_SEC_SET8021XGLOBALMODE_FUNC_T    hal_sec_set8021xGlobalMode;
    HAL_SEC_GET8021XGLOBALMODE_FUNC_T    hal_sec_get8021xGlobalMode;
    HAL_SEC_SET8021XGLOBALCFG_FUNC_T     hal_sec_set8021xGlobalCfg;
    HAL_SEC_GET8021XGLOBALCFG_FUNC_T     hal_sec_get8021xGlobalCfg;
    HAL_SEC_SET8021XMACBASEDCFG_FUNC_T   hal_sec_set8021xMacBasedCfg;
    HAL_SEC_GET8021XMACBASEDCFG_FUNC_T   hal_sec_get8021xMacBasedCfg;
    HAL_SEC_SET8021XPORTBASEDCFG_FUNC_T  hal_sec_set8021xPortBasedCfg;
    HAL_SEC_GET8021XPORTBASEDCFG_FUNC_T  hal_sec_get8021xPortBasedCfg;

    HAL_SEC_SETMACLIMITGLOBALMODE_FUNC_T hal_sec_setMacLimitGlobalMode;
    HAL_SEC_GETMACLIMITGLOBALMODE_FUNC_T hal_sec_getMacLimitGlobalMode;
    HAL_SEC_SETMACLIMITGLOBALCFG_FUNC_T  hal_sec_setMacLimitGlobalCfg;
    HAL_SEC_GETMACLIMITGLOBALCFG_FUNC_T  hal_sec_getMacLimitGlobalCfg;
    HAL_SEC_SETMACLIMITPORTCFG_FUNC_T    hal_sec_setMacLimitPortCfg;
    HAL_SEC_GETMACLIMITPORTCFG_FUNC_T    hal_sec_getMacLimitPortCfg;

    HAL_SEC_SETPORTSTORMCTRL_FUNC_T      hal_sec_setPortStormCtrl;
    HAL_SEC_GETPORTSTORMCTRL_FUNC_T      hal_sec_getPortStormCtrl;
    HAL_SEC_SETPORTSTORMCTRLRATE_FUNC_T  hal_sec_setPortStormCtrlRate;
    HAL_SEC_GETPORTSTORMCTRLRATE_FUNC_T  hal_sec_getPortStormCtrlRate;
    HAL_SEC_SETSTORMCTRLMGMTMODE_FUNC_T  hal_sec_setStormCtrlMgmtMode;
    HAL_SEC_GETSTORMCTRLMGMTMODE_FUNC_T  hal_sec_getStormCtrlMgmtMode;
} HAL_SEC_FUNC_VEC_T;

typedef struct
{
    HAL_SFLOW_INIT_FUNC_T        hal_sflow_init;
    HAL_SFLOW_DEINIT_FUNC_T      hal_sflow_deinit;
    HAL_SFLOW_SETSAMPLING_FUNC_T hal_sflow_setSampling;
    HAL_SFLOW_GETSAMPLING_FUNC_T hal_sflow_getSampling;
    HAL_SFLOW_SETMNGFRM_FUNC_T   hal_sflow_setMngFrm;
    HAL_SFLOW_GETMNGFRM_FUNC_T   hal_sflow_getMngFrm;
} HAL_SFLOW_FUNC_VEC_T;

typedef struct
{
    HAL_SVLAN_ADDSVLAN_FUNC_T       hal_svlan_addSvlan;
    HAL_SVLAN_GETSVLAN_FUNC_T       hal_svlan_getSvlan;
    HAL_SVLAN_DELSVLAN_FUNC_T       hal_svlan_delSvlan;
    HAL_SVLAN_SETCUSTOMPORT_FUNC_T  hal_svlan_setCustomPort;
    HAL_SVLAN_GETCUSTOMPORT_FUNC_T  hal_svlan_getCustomPort;
    HAL_SVLAN_SETSERVICEPORT_FUNC_T hal_svlan_setServicePort;
    HAL_SVLAN_GETSERVICEPORT_FUNC_T hal_svlan_getServicePort;
    HAL_SVLAN_SETEGSTPID_0_FUNC_T   hal_svlan_setEgsOuterTPID;
    HAL_SVLAN_GETEGSTPID_0_FUNC_T   hal_svlan_getEgsOuterTPID;
    HAL_SVLAN_SETEGSTPID_1_FUNC_T   hal_svlan_setEgsInnerTPID;
    HAL_SVLAN_GETEGSTPID_1_FUNC_T   hal_svlan_getEgsInnerTPID;
    HAL_SVLAN_SETSTAGTPID_FUNC_T    hal_svlan_setRecvTagTPID;
    HAL_SVLAN_GETSTAGTPID_FUNC_T    hal_svlan_getRecvTagTPID;
} HAL_SVLAN_FUNC_VEC_T;

typedef struct
{
    HAL_STAG_INIT_FUNC_T         hal_stag_init;
    HAL_STAG_DEINIT_FUNC_T       hal_stag_deinit;
    HAL_STAG_SETPORT_FUNC_T      hal_stag_setPort;
    HAL_STAG_GETPORT_FUNC_T      hal_stag_getPort;
    HAL_STAG_SETMODE_FUNC_T      hal_stag_setMode;
    HAL_STAG_GETMODE_FUNC_T      hal_stag_getMode;
    HAL_STAG_ENCODETXSTAG_FUNC_T hal_stag_encodeTxStag;
    HAL_STAG_DECODERXSTAG_FUNC_T hal_stag_decodeRxStag;
} HAL_STAG_FUNC_VEC_T;

typedef struct
{
    HAL_DOS_INIT_FUNC_T                 hal_dos_init;
    HAL_DOS_DEINIT_FUNC_T               hal_dos_deinit;
    HAL_DOS_SETGLOBALCFG_FUNC_T         hal_dos_setGlobalCfg;
    HAL_DOS_GETGLOBALCFG_FUNC_T         hal_dos_getGlobalCfg;
    HAL_DOS_SETICMPLENGTHLIMIT_FUNC_T   hal_dos_setIcmpLengthLimit;
    HAL_DOS_GETICMPLENGTHLIMIT_FUNC_T   hal_dos_getIcmpLengthLimit;
    HAL_DOS_SETTCPFLAGCFG_FUNC_T        hal_dos_setTcpFlagCfg;
    HAL_DOS_GETTCPFLAGCFG_FUNC_T        hal_dos_getTcpFlagCfg;
    HAL_DOS_SETPORTACTIONCFG_FUNC_T     hal_dos_setPortActionCfg;
    HAL_DOS_GETPORTACTIONCFG_FUNC_T     hal_dos_getPortActionCfg;
    HAL_DOS_CLEARPORTACTIONCFG_FUNC_T   hal_dos_clearPortActionCfg;
    HAL_DOS_CLEARACTIONCFG_FUNC_T       hal_dos_clearActionCfg;
    HAL_DOS_SETRATELIMITCFG_FUNC_T      hal_dos_setRateLimitCfg;
    HAL_DOS_GETRATELIMITCFG_FUNC_T      hal_dos_getRateLimitCfg;
    HAL_DOS_CLEARRATELIMITCFG_FUNC_T    hal_dos_clearRateLimitCfg;
    HAL_DOS_CLEARALLRATELIMITCFG_FUNC_T hal_dos_clearAllRateLimitCfg;
    HAL_DOS_GETDROPCNT_FUNC_T           hal_dos_getDropCnt;
    HAL_DOS_CLEARDROPCNT_FUNC_T         hal_dos_clearDropCnt;
} HAL_DOS_FUNC_VEC_T;

typedef struct
{
    HAL_ACL_INIT_T              hal_acl_init;
    HAL_ACL_DEINIT_T            hal_acl_deinit;
    HAL_ACL_GETGLOBALSTATE_T    hal_acl_getGlobalState;
    HAL_ACL_SETGLOBALSTATE_T    hal_acl_setGlobalState;
    HAL_ACL_GETPORTSTATE_T      hal_acl_getPortState;
    HAL_ACL_SETPORTSTATE_T      hal_acl_setPortState;
    HAL_ACL_CLEARALL_T          hal_acl_clearAll;
    HAL_ACL_GETRULE_T           hal_acl_getRule;
    HAL_ACL_SETRULE_T           hal_acl_setRule;
    HAL_ACL_DELRULE_T           hal_acl_delRule;
    HAL_ACL_GETACTION_T         hal_acl_getAction;
    HAL_ACL_SETACTION_T         hal_acl_setAction;
    HAL_ACL_DELACTION_T         hal_acl_delAction;
    HAL_ACL_GETBLOCKSTATE_T     hal_acl_getBlockState;
    HAL_ACL_SETBLOCKSTATE_T     hal_acl_setBlockState;
    HAL_ACL_GETMIBCNT_T         hal_acl_getMibCnt;
    HAL_ACL_CLEARMIBCNT_T       hal_acl_clearMibCnt;
    HAL_ACL_GETUDFRULE_T        hal_acl_getUdfRule;
    HAL_ACL_SETUDFRULE_T        hal_acl_setUdfRule;
    HAL_ACL_DELUDFRULE_T        hal_acl_delUdfRule;
    HAL_ACL_CLEARUDFRULE_T      hal_acl_clearUdfRule;
    HAL_ACL_GETMETERTABLE_T     hal_acl_getMeterTable;
    HAL_ACL_SETMETERTABLE_T     hal_acl_setMeterTable;
    HAL_ACL_GETDROPEN_T         hal_acl_getDropEn;
    HAL_ACL_SETDROPEN_T         hal_acl_setDropEn;
    HAL_ACL_GETDROPTHRSH_T      hal_acl_getDropThrsh;
    HAL_ACL_SETDROPTHRSH_T      hal_acl_setDropThrsh;
    HAL_ACL_GETDROPPBB_T        hal_acl_getDropPbb;
    HAL_ACL_SETDROPPBB_T        hal_acl_setDropPbb;
    HAL_ACL_GETDROPEXMFRM_T     hal_acl_getDropExMfrm;
    HAL_ACL_SETDROPEXMFRM_T     hal_acl_setDropExMfrm;
    HAL_ACL_GETTRTCMEN_T        hal_acl_getTrtcmEn;
    HAL_ACL_SETTRTCMEN_T        hal_acl_setTrtcmEn;
    HAL_ACL_GETTRTCMTABLE_T     hal_acl_getTrtcmTable;
    HAL_ACL_SETTRTCMTABLE_T     hal_acl_setTrtcmTable;
    HAL_ACL_GETL3ROUTINGTABLE_T hal_acl_getL3RoutingTable;
    HAL_ACL_SETL3ROUTINGTABLE_T hal_acl_setL3RoutingTable;
    HAL_ACL_GETARPRULE_T        hal_acl_getArpRule;
    HAL_ACL_SETARPRULE_T        hal_acl_setArpRule;
    HAL_ACL_DELARPRULE_T        hal_acl_delArpRule;
} HAL_ACL_FUNC_VEC_T;

/* =============== Peripheral =============== */
/* MDIO multiplexing functions start */
typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC22_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC22_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC45_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC45_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC22FROMINTBUS_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC22FROMINTBUS_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T page_sel,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC45FROMINTBUS_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC45FROMINTBUS_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

#ifdef AIR_EN_I2C_PHY
typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC22BYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC22BYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEC45BYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    const UI16_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READC45BYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI16_T phy_addr,
    const UI16_T dev_type,
    const UI16_T reg_addr,
    UI16_T      *ptr_reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_WRITEREGBYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI32_T reg_addr,
    const UI32_T reg_data);

typedef AIR_ERROR_NO_T (*HAL_MDIO_READREGBYI2C_FUNC_T)(
    const UI32_T unit,
    const UI16_T bus_id,
    const UI16_T i2c_addr,
    const UI32_T reg_addr,
    UI32_T      *ptr_reg_data);
#endif /* #ifdef AIR_EN_I2C_PHY */

typedef struct
{
    HAL_MDIO_WRITEC22_FUNC_T           hal_mdio_writeC22;
    HAL_MDIO_READC22_FUNC_T            hal_mdio_readC22;
    HAL_MDIO_WRITEC45_FUNC_T           hal_mdio_writeC45;
    HAL_MDIO_READC45_FUNC_T            hal_mdio_readC45;
    HAL_MDIO_WRITEC22FROMINTBUS_FUNC_T hal_mdio_writeC22FromIntBus;
    HAL_MDIO_READC22FROMINTBUS_FUNC_T  hal_mdio_readC22FromIntBus;
    HAL_MDIO_WRITEC45FROMINTBUS_FUNC_T hal_mdio_writeC45FromIntBus;
    HAL_MDIO_READC45FROMINTBUS_FUNC_T  hal_mdio_readC45FromIntBus;
#ifdef AIR_EN_I2C_PHY
    HAL_MDIO_WRITEC22BYI2C_FUNC_T hal_mdio_writeC22ByI2c;
    HAL_MDIO_READC22BYI2C_FUNC_T  hal_mdio_readC22ByI2c;
    HAL_MDIO_WRITEC45BYI2C_FUNC_T hal_mdio_writeC45ByI2c;
    HAL_MDIO_READC45BYI2C_FUNC_T  hal_mdio_readC45ByI2c;
    HAL_MDIO_WRITEREGBYI2C_FUNC_T hal_mdio_writeRegByI2c;
    HAL_MDIO_READREGBYI2C_FUNC_T  hal_mdio_readRegByI2c;
#endif /* #ifdef AIR_EN_I2C_PHY */
} HAL_MDIO_FUNC_VEC_T;
/* MDIO multiplexing functions end */

typedef struct
{
    HAL_IFMON_INIT_FUNC_T            hal_ifmon_init;
    HAL_IFMON_DEINIT_FUNC_T          hal_ifmon_deinit;
    HAL_IFMON_REGISTER_FUNC_T        hal_ifmon_register;
    HAL_IFMON_DEREGISTER_FUNC_T      hal_ifmon_deregister;
    HAL_IFMON_SETMODE_FUNC_T         hal_ifmon_setMode;
    HAL_IFMON_GETMODE_FUNC_T         hal_ifmon_getMode;
    HAL_IFMON_SETMONITORSTATE_FUNC_T hal_ifmon_setMonitorState;
    HAL_IFMON_GETMONITORSTATE_FUNC_T hal_ifmon_getMonitorState;
    HAL_IFMON_GETPORTSTATUS_FUNC_T   hal_ifmon_getPortStatus;
} HAL_IFMON_FUNC_VEC_T;

/* SIF Function Structure */

/* sif module function vector */
typedef struct
{
    HAL_SIF_INIT_FUNC_T          hal_sif_init;
    HAL_SIF_DEINIT_FUNC_T        hal_sif_deinit;
    HAL_SIF_WRITE_FUNC_T         hal_sif_write;
    HAL_SIF_READ_FUNC_T          hal_sif_read;
    HAL_SIF_WRITEBYREMOTE_FUNC_T hal_sif_writeByRemote;
    HAL_SIF_READBYREMOTE_FUNC_T  hal_sif_readByRemote;
} HAL_SIF_FUNC_VEC_T;

/* peripheral module function vector */
typedef struct
{
    HAL_PERIF_INIT_FUNC_T                  hal_perif_init;
    HAL_PERIF_DEINIT_FUNC_T                hal_perif_deinit;
    HAL_PERIF_SETGPIODIRECTION_FUNC_T      hal_perif_setGpioDirection;
    HAL_PERIF_GETGPIODIRECTION_FUNC_T      hal_perif_getGpioDirection;
    HAL_PERIF_SETGPIOOUTPUTDATA_FUNC_T     hal_perif_setGpioOutputData;
    HAL_PERIF_GETGPIOINPUTDATA_FUNC_T      hal_perif_getGpioInputData;
    HAL_PERIF_SETGPIOOUTPUTAUTOMODE_FUNC_T hal_perif_setGpioOutputAutoMode;
    HAL_PERIF_GETGPIOOUTPUTAUTOMODE_FUNC_T hal_perif_getGpioOutputAutoMode;
    HAL_PERIF_SETGPIOOUTPUTAUTOPATT_FUNC_T hal_perif_setGpioOutputAutoPatt;
    HAL_PERIF_GETGPIOOUTPUTAUTOPATT_FUNC_T hal_perif_getGpioOutputAutoPatt;
} HAL_PERIF_FUNC_VEC_T;

typedef struct
{
    /* chip multiplexing functions */
    const HAL_CHIP_FUNC_VEC_T *const  chip_func_vec;
    /* Port multiplexing functions */
    const HAL_PORT_FUNC_VEC_T *const  port_func_vec;
    /* L2 multiplexing functions */
    const HAL_L2_FUNC_VEC_T *const    l2_func_vec;
    /* Mib multiplexing functions */
    const HAL_MIB_FUNC_VEC_T *const   mib_func_vec;
    /* vlan multiplexing functions */
    const HAL_VLAN_FUNC_VEC_T *const  vlan_func_vec;
    /* mirr multiplexing functions */
    const HAL_MIR_FUNC_VEC_T *const   mir_func_vec;
    /* QoS multiplexing functions */
    const HAL_QOS_FUNC_VEC_T *const   qos_func_vec;
    /* Switch multiplexing functions */
    const HAL_SWC_FUNC_VEC_T *const   swc_func_vec;
    /* ifmon multiplexing functions */
    const HAL_IFMON_FUNC_VEC_T *const ifmon_func_vec;
    /* MDIO functions */
    const HAL_MDIO_FUNC_VEC_T *const  mdio_func_vec;
    /* SIF functions */
    const HAL_SIF_FUNC_VEC_T *const   sif_func_vec;
    /* periferal functions */
    const HAL_PERIF_FUNC_VEC_T *const perif_func_vec;
    /* stp multiplexing functions */
    const HAL_STP_FUNC_VEC_T *const   stp_func_vec;
    /* Lag multiplexing functions */
    const HAL_LAG_FUNC_VEC_T *const   lag_func_vec;
    /* ipmc multiplexing functions */
    const HAL_IPMC_FUNC_VEC_T *const  ipmc_func_vec;
    /* Security multiplexing functions */
    const HAL_SEC_FUNC_VEC_T *const   sec_func_vec;
    /* sFlow multiplexing functions */
    const HAL_SFLOW_FUNC_VEC_T *const sflow_func_vec;
    /* Svlan multiplexing functions */
    const HAL_SVLAN_FUNC_VEC_T *const svlan_func_vec;
    /* Stag multiplexing functions */
    const HAL_STAG_FUNC_VEC_T *const  stag_func_vec;
    /* DoS multiplexing functions */
    const HAL_DOS_FUNC_VEC_T *const   dos_func_vec;
    /* Acl multiplexing functions */
    const HAL_ACL_FUNC_VEC_T *const   acl_func_vec;
    /* PoE multiplexing functions */
    const HAL_POE_FUNC_VEC_T *const   poe_func_vec;
} HAL_FUNC_VEC_T;

typedef struct
{
    AIR_INIT_PORT_TYPE_T type;
    I32_T                xsgmii_pkg_id;
    I32_T                phy_pkg_id;
    I32_T                mac_port_id;
} HAL_DRV_MAC_PORT_MAP_T;

typedef struct
{
    UI32_T                        device_id;
    UI32_T                        mac_port_cnt;
    const HAL_DRV_MAC_PORT_MAP_T *ptr_mac_port_map;
} HAL_DRV_MAC_PORT_INFO_T;

typedef struct
{
    const C8_T *const                    driver_desc;       /* driver description */
    const HAL_FUNC_VEC_T *const          ptr_func_vector;   /* function vector pointer */
    const HAL_DRV_MAC_PORT_INFO_T *const ptr_mac_port_info; /* all mac port information of this chip family */
} HAL_DRIVER_T;

typedef AIR_ERROR_NO_T (*HAL_DRIVER_INIT_FUNC_T)(
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver);

typedef struct
{
    UI32_T                 family_id;      /* family ID */
    HAL_DRIVER_INIT_FUNC_T hal_initDriver; /* driver handler function pointer */
} HAL_DEVICE_DRIVER_MAP_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   hal_drv_initDeviceDriver
 * PURPOSE:
 *      hal_drv_initDeviceDriver() is an API that will use device ID and
 *      revision ID to find correct device driver.
 *
 * INPUT:
 *      device_id       -- The device ID of this device.
 *      revision_id     -- The revision ID of this device.
 * OUTPUT:
 *      pptr_hal_driver -- The pointer (address) of the HAL device driver
 *                         that used for this device with the device_id and
 *                         revision_id.
 * RETURN:
 *      AIR_E_OK        -- Initialize the device driver successfully .
 *      AIR_E_OTHERS    -- Fail to initialize the device driver.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_drv_initDeviceDriver(
    const UI32_T   device_id,
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver);

#endif /* #ifndef HAL_DRV_H */
