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

/* FILE NAME:  hal_coral_drv.c
 * PURPOSE:
 *      Provide HAL AN8858 driver and driver functions.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_drv.h>

#include <hal/common/hal.h>
#include <hal/common/hal_dev.h>
#include <hal/common/hal_drv.h>
#include <hal/switch/coral/hal_coral_acl.h>
#include <hal/switch/coral/hal_coral_chip.h>
#include <hal/switch/coral/hal_coral_dos.h>
#include <hal/switch/coral/hal_coral_ifmon.h>
#include <hal/switch/coral/hal_coral_ipmc.h>
#include <hal/switch/coral/hal_coral_l2.h>
#include <hal/switch/coral/hal_coral_lag.h>
#include <hal/switch/coral/hal_coral_mdio.h>
#include <hal/switch/coral/hal_coral_mib.h>
#include <hal/switch/coral/hal_coral_mir.h>
#include <hal/switch/coral/hal_coral_perif.h>
#include <hal/switch/coral/hal_coral_port.h>
#include <hal/switch/coral/hal_coral_qos.h>
#include <hal/switch/coral/hal_coral_sec.h>
#include <hal/switch/coral/hal_coral_sflow.h>
#include <hal/switch/coral/hal_coral_sif.h>
#include <hal/switch/coral/hal_coral_stag.h>
#include <hal/switch/coral/hal_coral_stp.h>
#include <hal/switch/coral/hal_coral_svlan.h>
#include <hal/switch/coral/hal_coral_swc.h>
#include <hal/switch/coral/hal_coral_vlan.h>
#include <osal/osal.h>

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
#ifdef AIR_EN_DUMB_FIRMWARE_SUPPORT
const static HAL_CHIP_FUNC_VEC_T chip_func_vec = {
    hal_coral_chip_readDeviceInfo,
    hal_coral_chip_init,
    NULL,
};

const static HAL_STP_FUNC_VEC_T stp_func_vec = {
    NULL,
    NULL,
    hal_coral_stp_setPortState,
    hal_coral_stp_getPortState,
};

const static HAL_MIR_FUNC_VEC_T mir_func_vec = {
    hal_coral_mir_init,
    NULL,
    hal_coral_mir_addSession,
    hal_coral_mir_delSession,
    hal_coral_mir_getSession,
    hal_coral_mir_setSessionAdminMode,
    hal_coral_mir_getSessionAdminMode,
    hal_coral_mir_setMirrorPort,
    hal_coral_mir_getMirrorPort,
};

const static HAL_PORT_FUNC_VEC_T port_func_vec = {
    hal_coral_port_init,
    NULL,
    hal_coral_port_setSerdesMode,
    NULL,
    hal_coral_port_setPhyAutoNego,
    hal_coral_port_getPhyAutoNego,
    hal_coral_port_setPhyLocalAdvAbility,
    hal_coral_port_getPhyLocalAdvAbility,
    hal_coral_port_getPhyRemoteAdvAbility,
    hal_coral_port_setSpeed,
    NULL,
    hal_coral_port_setDuplex,
    NULL,
    hal_coral_port_setBackPressure,
    hal_coral_port_getBackPressure,
    hal_coral_port_setFlowCtrl,
    NULL,
    NULL,
    NULL,
    hal_coral_port_getPortStatus,
    hal_coral_port_setAdminState,
    NULL,
    NULL,
    NULL,
    hal_coral_port_setPortMatrix,
    hal_coral_port_getPortMatrix,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_port_setComboMode,
    hal_coral_port_getComboMode,
    hal_coral_port_setPhyLedCtrlMode,
    hal_coral_port_getPhyLedCtrlMode,
    hal_coral_port_setPhyLedForceState,
    hal_coral_port_getPhyLedForceState,
    hal_coral_port_setPhyLedForcePattCfg,
    hal_coral_port_getPhyLedForcePattCfg,
    NULL,
    NULL,
    hal_coral_port_setPhyOpMode,
    hal_coral_port_getPhyOpMode,
    NULL,
    NULL,
};

const static HAL_LAG_FUNC_VEC_T lag_func_vec = {
    hal_coral_lag_init,
    NULL,
    hal_coral_lag_getMaxGroupCnt,
    hal_coral_lag_createGroup,
    hal_coral_lag_destroyGroup,
    hal_coral_lag_getGroup,
    hal_coral_lag_setMember,
    hal_coral_lag_getMember,
    hal_coral_lag_setHashControl,
    hal_coral_lag_getHashControl,
};

const static HAL_L2_FUNC_VEC_T l2_func_vec = {
    hal_coral_l2_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_l2_clearMacAddr,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_l2_setForwardMode,
    NULL,
    hal_coral_l2_flushMacAddr,
#ifdef AIR_EN_L2_SHADOW
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* End of AIR_EN_L2_SHADOW */
};

const static HAL_IPMC_FUNC_VEC_T ipmc_func_vec = {
    NULL,
    NULL,
    hal_coral_ipmc_addMcastAddr,
    hal_coral_ipmc_getMcastAddr,
    hal_coral_ipmc_delMcastAddr,
    NULL,
    hal_coral_ipmc_addMcastMember,
    hal_coral_ipmc_delMcastMember,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_ipmc_setPortIpmcMode,
    hal_coral_ipmc_getPortIpmcMode,
    hal_coral_ipmc_setPortLookupIpTypeCtrl,
    hal_coral_ipmc_getPortLookupIpTypeCtrl,
};

const static HAL_MIB_FUNC_VEC_T mib_func_vec = {
    hal_coral_mib_clearAllCnt,
    hal_coral_mib_clearPortCnt,
    hal_coral_mib_getPortCnt,
    NULL,
};

const static HAL_VLAN_FUNC_VEC_T vlan_func_vec = {
    hal_coral_vlan_init,
    NULL,
    hal_coral_vlan_create,
    hal_coral_vlan_destroy,
    hal_coral_vlan_getVlan,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_vlan_setPortAttr,
    hal_coral_vlan_getPortAttr,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_vlan_setPortPVID,
    hal_coral_vlan_getPortPVID,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_vlan_setIngressVlanFilter,
    hal_coral_vlan_getIngressVlanFilter,
};

const static HAL_QOS_FUNC_VEC_T qos_func_vec = {
    hal_coral_qos_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_qos_setTrustMode,
    hal_coral_qos_getTrustMode,
    hal_coral_qos_setPriToQueue,
    hal_coral_qos_getPriToQueue,
    hal_coral_qos_setDscpToPri,
    hal_coral_qos_getDscpToPri,
    hal_coral_qos_setScheduleMode,
    hal_coral_qos_getScheduleMode,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_qos_setPortPriority,
    hal_coral_qos_getPortPriority,
    hal_coral_qos_setPortTrustMode,
    hal_coral_qos_getPortTrustMode,
};

const static HAL_SWC_FUNC_VEC_T swc_func_vec = {
    hal_coral_swc_init,
    NULL,
    hal_coral_swc_setMgmtFrameCfg,
    hal_coral_swc_getMgmtFrameCfg,
    hal_coral_swc_setSystemMac,
    hal_coral_swc_getSystemMac,
    hal_coral_swc_setJumboSize,
    hal_coral_swc_getJumboSize,
    hal_coral_swc_setProperty,
    hal_coral_swc_getProperty,
    hal_coral_swc_setLoopDetectCtrl,
    NULL,
    hal_coral_swc_setLoopDetectFrameSrcMac,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_swc_getChipUid,
};

const static HAL_SEC_FUNC_VEC_T sec_func_vec = {
    hal_coral_sec_init,
    NULL,
    /* 802.1x */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    /* MAC Limits */
    hal_coral_sec_setMacLimitGlobalMode,
    hal_coral_sec_getMacLimitGlobalMode,
    hal_coral_sec_setMacLimitGlobalCfg,
    hal_coral_sec_getMacLimitGlobalCfg,
    hal_coral_sec_setMacLimitPortCfg,
    hal_coral_sec_getMacLimitPortCfg,
    /* Storm Control */
    hal_coral_sec_setPortStormCtrl,
    hal_coral_sec_getPortStormCtrl,
    hal_coral_sec_setPortStormCtrlRate,
    hal_coral_sec_getPortStormCtrlRate,
    NULL,
    NULL,
};

const static HAL_SFLOW_FUNC_VEC_T sflow_func_vec = {
    hal_coral_sflow_init,
    hal_coral_sflow_deinit,
    hal_coral_sflow_setSampling,
    hal_coral_sflow_getSampling,
    hal_coral_sflow_setMngFrm,
    hal_coral_sflow_getMngFrm,
};

const static HAL_SVLAN_FUNC_VEC_T svlan_func_vec = {
    hal_coral_svlan_addSvlan,
    hal_coral_svlan_getSvlan,
    hal_coral_svlan_delSvlan,
    hal_coral_svlan_setCustomPort,
    hal_coral_svlan_getCustomPort,
    hal_coral_svlan_setServicePort,
    hal_coral_svlan_getServicePort,
    hal_coral_svlan_setEgsOuterTPID,
    hal_coral_svlan_getEgsOuterTPID,
    hal_coral_svlan_setEgsInnerTPID,
    hal_coral_svlan_getEgsInnerTPID,
    hal_coral_svlan_setRecvTagTPID,
    hal_coral_svlan_getRecvTagTPID,
};

const static HAL_STAG_FUNC_VEC_T stag_func_vec = {
    hal_coral_stag_init,
    hal_coral_stag_deinit,
    hal_coral_stag_setPort,
    hal_coral_stag_getPort,
    hal_coral_stag_setMode,
    hal_coral_stag_getMode,
    hal_coral_stag_encodeTxStag,
    hal_coral_stag_decodeRxStag,
};

const static HAL_DOS_FUNC_VEC_T dos_func_vec = {
    NULL,
    NULL,
    hal_coral_dos_setGlobalCfg,
    hal_coral_dos_getGlobalCfg,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_dos_setRateLimitCfg,
    hal_coral_dos_getRateLimitCfg,
    hal_coral_dos_clearRateLimitCfg,
    NULL,
    NULL,
    NULL,
};

const static HAL_ACL_FUNC_VEC_T acl_func_vec = {
    hal_coral_acl_init,
    NULL,
    hal_coral_acl_getGlobalState,
    hal_coral_acl_setGlobalState,
    hal_coral_acl_getPortState,
    hal_coral_acl_setPortState,
    NULL,
    hal_coral_acl_getRule,
    hal_coral_acl_setRule,
    hal_coral_acl_delRule,
    hal_coral_acl_getAction,
    hal_coral_acl_setAction,
    hal_coral_acl_delAction,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_coral_acl_getUdfRule,
    hal_coral_acl_setUdfRule,
    hal_coral_acl_delUdfRule,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

const static HAL_MDIO_FUNC_VEC_T mdio_func_vec = {
    hal_coral_mdio_writeC22,
    hal_coral_mdio_readC22,
    hal_coral_mdio_writeC45,
    hal_coral_mdio_readC45,
    hal_coral_mdio_writeC22FromIntBus,
    hal_coral_mdio_readC22FromIntBus,
    hal_coral_mdio_writeC45FromIntBus,
    hal_coral_mdio_readC45FromIntBus,
#ifdef AIR_EN_I2C_PHY
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* #ifdef AIR_EN_I2C_PHY */
};

const static HAL_IFMON_FUNC_VEC_T ifmon_func_vec = {
    hal_coral_ifmon_init,
    hal_coral_ifmon_deinit,
    hal_coral_ifmon_register,
    hal_coral_ifmon_deregister,
    hal_coral_ifmon_setMode,
    hal_coral_ifmon_getMode,
    hal_coral_ifmon_setMonitorState,
    hal_coral_ifmon_getMonitorState,
    hal_coral_ifmon_getPortStatus,
};

const static HAL_SIF_FUNC_VEC_T sif_func_vec = {
    hal_coral_sif_init,
    hal_coral_sif_deinit,
    hal_coral_sif_write,
    hal_coral_sif_read,
    NULL,
    NULL,
};

const static HAL_PERIF_FUNC_VEC_T perif_func_vec = {
    hal_coral_perif_init,
    hal_coral_perif_deinit,
    hal_coral_perif_setGpioDirection,
    hal_coral_perif_getGpioDirection,
    hal_coral_perif_setGpioOutputData,
    hal_coral_perif_getGpioInputData,
    hal_coral_perif_setGpioOutputAutoMode,
    hal_coral_perif_getGpioOutputAutoMode,
    hal_coral_perif_setGpioOutputAutoPatt,
    hal_coral_perif_getGpioOutputAutoPatt,
};

const static HAL_FUNC_VEC_T _coral_func_vector = {
    /* chip multiplexing functions */
    &chip_func_vec,

    /* port multiplexing functions */
    &port_func_vec,

    /* L2 protocol functions */
    &l2_func_vec,

    /* Mib multiplexing functions */
    &mib_func_vec,

    /* vlan multiplexing functions */
    &vlan_func_vec,

    /* mirror multiplexing functions */
    NULL,

    /* QoS multiplexing functions */
    &qos_func_vec,

    /* switch multiplexing functions */
    &swc_func_vec,

    /* ifmon functions */
    &ifmon_func_vec,

    /* Mdio functions */
    &mdio_func_vec,

    /* SIF functions */
    &sif_func_vec,

    /* perif functions */
    &perif_func_vec,

    /* stp multiplexing functions */
    &stp_func_vec,

    /* Lag multiplexing functions */
    &lag_func_vec,

    /* Mcast multiplexing functions */
    &ipmc_func_vec,

    /* Security multiplexing functions */
    &sec_func_vec,

    /* sFlow multiplexing functions */
    NULL,

    /* Svlan multiplexing functions */
    NULL,

    /* stag multiplexing functions */
    &stag_func_vec,

    /* DoS multiplexing functions */
    &dos_func_vec,

    /* Acl multiplexing functions */
    &acl_func_vec,
};
#else
const static HAL_CHIP_FUNC_VEC_T chip_func_vec = {
    hal_coral_chip_readDeviceInfo,
    hal_coral_chip_init,
    hal_coral_chip_deinit,
};

const static HAL_STP_FUNC_VEC_T stp_func_vec = {
    NULL,
    NULL,
    hal_coral_stp_setPortState,
    hal_coral_stp_getPortState,
};

const static HAL_MIR_FUNC_VEC_T mir_func_vec = {
    hal_coral_mir_init,
    NULL,
    hal_coral_mir_addSession,
    hal_coral_mir_delSession,
    hal_coral_mir_getSession,
    hal_coral_mir_setSessionAdminMode,
    hal_coral_mir_getSessionAdminMode,
    hal_coral_mir_setMirrorPort,
    hal_coral_mir_getMirrorPort,
};

const static HAL_PORT_FUNC_VEC_T port_func_vec = {
    hal_coral_port_init,
    hal_coral_port_deinit,
    hal_coral_port_setSerdesMode,
    hal_coral_port_getSerdesMode,
    hal_coral_port_setPhyAutoNego,
    hal_coral_port_getPhyAutoNego,
    hal_coral_port_setPhyLocalAdvAbility,
    hal_coral_port_getPhyLocalAdvAbility,
    hal_coral_port_getPhyRemoteAdvAbility,
    hal_coral_port_setSpeed,
    hal_coral_port_getSpeed,
    hal_coral_port_setDuplex,
    hal_coral_port_getDuplex,
    hal_coral_port_setBackPressure,
    hal_coral_port_getBackPressure,
    hal_coral_port_setFlowCtrl,
    hal_coral_port_getFlowCtrl,
    hal_coral_port_setPhyLoopBack,
    hal_coral_port_getPhyLoopBack,
    hal_coral_port_getPortStatus,
    hal_coral_port_setAdminState,
    hal_coral_port_getAdminState,
    hal_coral_port_setPhySmartSpeedDown,
    hal_coral_port_getPhySmartSpeedDown,
    hal_coral_port_setPortMatrix,
    hal_coral_port_getPortMatrix,
    hal_coral_port_setVlanMode,
    hal_coral_port_getVlanMode,
    hal_coral_port_setPhyLedOnCtrl,
    hal_coral_port_getPhyLedOnCtrl,
    hal_coral_port_setComboMode,
    hal_coral_port_getComboMode,
    hal_coral_port_setPhyLedCtrlMode,
    hal_coral_port_getPhyLedCtrlMode,
    hal_coral_port_setPhyLedForceState,
    hal_coral_port_getPhyLedForceState,
    hal_coral_port_setPhyLedForcePattCfg,
    hal_coral_port_getPhyLedForcePattCfg,
    hal_coral_port_triggerCableTest,
    hal_coral_port_getCableTestRawData,
    hal_coral_port_setPhyOpMode,
    hal_coral_port_getPhyOpMode,
    NULL,
    hal_coral_port_dumpDebugInfo,
};

const static HAL_LAG_FUNC_VEC_T lag_func_vec = {
    hal_coral_lag_init,
    NULL,
    hal_coral_lag_getMaxGroupCnt,
    hal_coral_lag_createGroup,
    hal_coral_lag_destroyGroup,
    hal_coral_lag_getGroup,
    hal_coral_lag_setMember,
    hal_coral_lag_getMember,
    hal_coral_lag_setHashControl,
    hal_coral_lag_getHashControl,
};

const static HAL_L2_FUNC_VEC_T l2_func_vec = {
    hal_coral_l2_init,
    hal_coral_l2_deinit,
    hal_coral_l2_addMacAddr,
    hal_coral_l2_delMacAddr,
    hal_coral_l2_getMacAddr,
    hal_coral_l2_getNextMacAddr,
    hal_coral_l2_searchMacAddr,
    hal_coral_l2_searchNextMacAddr,
    hal_coral_l2_clearMacAddr,
    hal_coral_l2_setMacAddrAgeOut,
    hal_coral_l2_getMacAddrAgeOut,
    hal_coral_l2_setMacAddrAgeOutMode,
    hal_coral_l2_getMacAddrAgeOutMode,
    hal_coral_l2_getMacBucketSize,
    hal_coral_l2_setForwardMode,
    hal_coral_l2_getForwardMode,
    hal_coral_l2_flushMacAddr,
#ifdef AIR_EN_L2_SHADOW
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* End of AIR_EN_L2_SHADOW */
};

const static HAL_IPMC_FUNC_VEC_T ipmc_func_vec = {
    hal_coral_ipmc_setMcastLookupType,
    hal_coral_ipmc_getMcastLookupType,
    hal_coral_ipmc_addMcastAddr,
    hal_coral_ipmc_getMcastAddr,
    hal_coral_ipmc_delMcastAddr,
    hal_coral_ipmc_delAllMcastAddr,
    hal_coral_ipmc_addMcastMember,
    hal_coral_ipmc_delMcastMember,
    hal_coral_ipmc_getMcastMemberCnt,
    hal_coral_ipmc_getMcastBucketSize,
    hal_coral_ipmc_getFirstMcastAddr,
    hal_coral_ipmc_getNextMcastAddr,
    hal_coral_ipmc_setPortIpmcMode,
    hal_coral_ipmc_getPortIpmcMode,
    hal_coral_ipmc_setPortLookupIpTypeCtrl,
    hal_coral_ipmc_getPortLookupIpTypeCtrl,
};

const static HAL_MIB_FUNC_VEC_T mib_func_vec = {
    hal_coral_mib_clearAllCnt,
    hal_coral_mib_clearPortCnt,
    hal_coral_mib_getPortCnt,
    hal_coral_mib_getFeatureCnt,
};

const static HAL_VLAN_FUNC_VEC_T vlan_func_vec = {
    hal_coral_vlan_init,
    hal_coral_vlan_deinit,
    hal_coral_vlan_create,
    hal_coral_vlan_destroy,
    hal_coral_vlan_getVlan,
    hal_coral_vlan_setFid,
    hal_coral_vlan_getFid,
    hal_coral_vlan_addMemberPort,
    hal_coral_vlan_delMemberPort,
    hal_coral_vlan_setMemberPort,
    hal_coral_vlan_getMemberPort,
    hal_coral_vlan_setIVL,
    hal_coral_vlan_getIVL,
    hal_coral_vlan_setIngressTagKeeping,
    hal_coral_vlan_getIngressTagKeeping,
    hal_coral_vlan_setEgsTagCtlEnable,
    hal_coral_vlan_getEgsTagCtlEnable,
    hal_coral_vlan_setPortEgsTagCtl,
    hal_coral_vlan_getPortEgsTagCtl,
    hal_coral_vlan_setPortAcceptFrameType,
    hal_coral_vlan_getPortAcceptFrameType,
    hal_coral_vlan_setPortAttr,
    hal_coral_vlan_getPortAttr,
    hal_coral_vlan_setIgrPortTagAttr,
    hal_coral_vlan_getIgrPortTagAttr,
    hal_coral_vlan_setPortEgsTagAttr,
    hal_coral_vlan_getPortEgsTagAttr,
    hal_coral_vlan_setPortPVID,
    hal_coral_vlan_getPortPVID,
    hal_coral_vlan_setCopyPriority,
    hal_coral_vlan_getCopyPriority,
    hal_coral_vlan_setUserPriority,
    hal_coral_vlan_getUserPriority,
    hal_coral_vlan_setStag,
    hal_coral_vlan_getStag,
    hal_coral_vlan_setPortPSVID,
    hal_coral_vlan_getPortPSVID,
    hal_coral_vlan_addMacBasedVlan,
    hal_coral_vlan_getMacBasedVlan,
    hal_coral_vlan_delMacBasedVlan,
    hal_coral_vlan_addSubnetBasedVlan,
    hal_coral_vlan_getSubnetBasedVlan,
    hal_coral_vlan_delSubnetBasedVlan,
    hal_coral_vlan_addProtocolBasedVlan,
    hal_coral_vlan_getProtocolBasedVlan,
    hal_coral_vlan_delProtocolBasedVlan,
    hal_coral_vlan_setProtocolBasedVlanPortAttr,
    hal_coral_vlan_getProtocolBasedVlanPortAttr,
    hal_coral_vlan_setIngressVlanFilter,
    hal_coral_vlan_getIngressVlanFilter,
};

const static HAL_QOS_FUNC_VEC_T qos_func_vec = {
    hal_coral_qos_init,
    NULL,
    hal_coral_qos_setRateLimitEnable,
    hal_coral_qos_getRateLimitEnable,
    hal_coral_qos_setRateLimitCfg,
    hal_coral_qos_getRateLimitCfg,
    NULL,
    hal_coral_qos_getRateLimitExcludeMgmt,
    hal_coral_qos_setTrustMode,
    hal_coral_qos_getTrustMode,
    hal_coral_qos_setPriToQueue,
    hal_coral_qos_getPriToQueue,
    hal_coral_qos_setDscpToPri,
    hal_coral_qos_getDscpToPri,
    hal_coral_qos_setScheduleMode,
    hal_coral_qos_getScheduleMode,
    hal_coral_qos_setShaperMode,
    hal_coral_qos_getShaperMode,
    hal_coral_qos_setShaperRate,
    hal_coral_qos_getShaperRate,
    hal_coral_qos_setShaperWeight,
    hal_coral_qos_getShaperWeight,
    hal_coral_qos_setPortPriority,
    hal_coral_qos_getPortPriority,
    hal_coral_qos_setPortTrustMode,
    hal_coral_qos_getPortTrustMode,
};

const static HAL_SWC_FUNC_VEC_T swc_func_vec = {
    hal_coral_swc_init,
    NULL,
    hal_coral_swc_setMgmtFrameCfg,
    hal_coral_swc_getMgmtFrameCfg,
    hal_coral_swc_setSystemMac,
    hal_coral_swc_getSystemMac,
    hal_coral_swc_setJumboSize,
    hal_coral_swc_getJumboSize,
    hal_coral_swc_setProperty,
    hal_coral_swc_getProperty,
    hal_coral_swc_setLoopDetectCtrl,
    hal_coral_swc_getLoopDetectCtrl,
    hal_coral_swc_setLoopDetectFrameSrcMac,
    hal_coral_swc_getLoopDetectFrameSrcMac,
    hal_coral_swc_getLoopDetectStatus,
    hal_coral_swc_clearLoopDetectStatus,
    hal_coral_swc_getGlobalFreePages,
    hal_coral_swc_getPortAllocatedPages,
    hal_coral_swc_getPortUsedPages,
    hal_coral_swc_setPortMgmtFrameCfg,
    hal_coral_swc_getPortMgmtFrameCfg,
    hal_coral_swc_getCapacity,
    hal_coral_swc_getUsage,
    hal_coral_swc_getChipUid,
};

const static HAL_SEC_FUNC_VEC_T sec_func_vec = {
    hal_coral_sec_init,
    hal_coral_sec_deinit,
    /* 802.1x */
    hal_coral_sec_set8021xGlobalMode,
    hal_coral_sec_get8021xGlobalMode,
    hal_coral_sec_set8021xGlobalCfg,
    hal_coral_sec_get8021xGlobalCfg,
    hal_coral_sec_set8021xMacBasedCfg,
    hal_coral_sec_get8021xMacBasedCfg,
    hal_coral_sec_set8021xPortBasedCfg,
    hal_coral_sec_get8021xPortBasedCfg,
    /* MAC Limits */
    hal_coral_sec_setMacLimitGlobalMode,
    hal_coral_sec_getMacLimitGlobalMode,
    hal_coral_sec_setMacLimitGlobalCfg,
    hal_coral_sec_getMacLimitGlobalCfg,
    hal_coral_sec_setMacLimitPortCfg,
    hal_coral_sec_getMacLimitPortCfg,
    /* Storm Control */
    hal_coral_sec_setPortStormCtrl,
    hal_coral_sec_getPortStormCtrl,
    hal_coral_sec_setPortStormCtrlRate,
    hal_coral_sec_getPortStormCtrlRate,
    hal_coral_sec_setStormCtrlMgmtMode,
    hal_coral_sec_getStormCtrlMgmtMode,
};

const static HAL_SFLOW_FUNC_VEC_T sflow_func_vec = {
    hal_coral_sflow_init,
    hal_coral_sflow_deinit,
    hal_coral_sflow_setSampling,
    hal_coral_sflow_getSampling,
    hal_coral_sflow_setMngFrm,
    hal_coral_sflow_getMngFrm,
};

const static HAL_SVLAN_FUNC_VEC_T svlan_func_vec = {
    hal_coral_svlan_addSvlan,
    hal_coral_svlan_getSvlan,
    hal_coral_svlan_delSvlan,
    hal_coral_svlan_setCustomPort,
    hal_coral_svlan_getCustomPort,
    hal_coral_svlan_setServicePort,
    hal_coral_svlan_getServicePort,
    hal_coral_svlan_setEgsOuterTPID,
    hal_coral_svlan_getEgsOuterTPID,
    hal_coral_svlan_setEgsInnerTPID,
    hal_coral_svlan_getEgsInnerTPID,
    hal_coral_svlan_setRecvTagTPID,
    hal_coral_svlan_getRecvTagTPID,
};

const static HAL_STAG_FUNC_VEC_T stag_func_vec = {
    hal_coral_stag_init,
    hal_coral_stag_deinit,
    hal_coral_stag_setPort,
    hal_coral_stag_getPort,
    hal_coral_stag_setMode,
    hal_coral_stag_getMode,
    hal_coral_stag_encodeTxStag,
    hal_coral_stag_decodeRxStag,
};

const static HAL_DOS_FUNC_VEC_T dos_func_vec = {
    hal_coral_dos_init,
    hal_coral_dos_deinit,
    hal_coral_dos_setGlobalCfg,
    hal_coral_dos_getGlobalCfg,
    hal_coral_dos_setIcmpLengthLimit,
    hal_coral_dos_getIcmpLengthLimit,
    hal_coral_dos_setTcpFlagCfg,
    hal_coral_dos_getTcpFlagCfg,
    hal_coral_dos_setPortActionCfg,
    hal_coral_dos_getPortActionCfg,
    hal_coral_dos_clearPortActionCfg,
    hal_coral_dos_clearActionCfg,
    hal_coral_dos_setRateLimitCfg,
    hal_coral_dos_getRateLimitCfg,
    hal_coral_dos_clearRateLimitCfg,
    hal_coral_dos_clearAllRateLimitCfg,
    hal_coral_dos_getDropCnt,
    hal_coral_dos_clearDropCnt,
};

const static HAL_ACL_FUNC_VEC_T acl_func_vec = {
    hal_coral_acl_init,
    hal_coral_acl_deinit,
    hal_coral_acl_getGlobalState,
    hal_coral_acl_setGlobalState,
    hal_coral_acl_getPortState,
    hal_coral_acl_setPortState,
    hal_coral_acl_clearAll,
    hal_coral_acl_getRule,
    hal_coral_acl_setRule,
    hal_coral_acl_delRule,
    hal_coral_acl_getAction,
    hal_coral_acl_setAction,
    hal_coral_acl_delAction,
    NULL,
    NULL,
    hal_coral_acl_getMibCnt,
    hal_coral_acl_clearMibCnt,
    hal_coral_acl_getUdfRule,
    hal_coral_acl_setUdfRule,
    hal_coral_acl_delUdfRule,
    hal_coral_acl_clearUdfRule,
    hal_coral_acl_getMeterTable,
    hal_coral_acl_setMeterTable,
    hal_coral_acl_getDropEn,
    hal_coral_acl_setDropEn,
    hal_coral_acl_getDropThrsh,
    hal_coral_acl_setDropThrsh,
    hal_coral_acl_getDropPbb,
    hal_coral_acl_setDropPbb,
    hal_coral_acl_getDropExMfrm,
    hal_coral_acl_setDropExMfrm,
    hal_coral_acl_getTrtcmEn,
    hal_coral_acl_setTrtcmEn,
    hal_coral_acl_getTrtcmTable,
    hal_coral_acl_setTrtcmTable,
    NULL,
    NULL,
    hal_coral_acl_getArpRule,
    hal_coral_acl_setArpRule,
    hal_coral_acl_delArpRule,
};

const static HAL_MDIO_FUNC_VEC_T mdio_func_vec = {
    hal_coral_mdio_writeC22,
    hal_coral_mdio_readC22,
    hal_coral_mdio_writeC45,
    hal_coral_mdio_readC45,
    hal_coral_mdio_writeC22FromIntBus,
    hal_coral_mdio_readC22FromIntBus,
    hal_coral_mdio_writeC45FromIntBus,
    hal_coral_mdio_readC45FromIntBus,
#ifdef AIR_EN_I2C_PHY
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif /* #ifdef AIR_EN_I2C_PHY */
};

const static HAL_IFMON_FUNC_VEC_T ifmon_func_vec = {
    hal_coral_ifmon_init,
    hal_coral_ifmon_deinit,
    hal_coral_ifmon_register,
    hal_coral_ifmon_deregister,
    hal_coral_ifmon_setMode,
    hal_coral_ifmon_getMode,
    hal_coral_ifmon_setMonitorState,
    hal_coral_ifmon_getMonitorState,
    hal_coral_ifmon_getPortStatus,
};

const static HAL_SIF_FUNC_VEC_T sif_func_vec = {
    hal_coral_sif_init,
    hal_coral_sif_deinit,
    hal_coral_sif_write,
    hal_coral_sif_read,
    NULL,
    NULL,
};

const static HAL_PERIF_FUNC_VEC_T perif_func_vec = {
    hal_coral_perif_init,
    hal_coral_perif_deinit,
    hal_coral_perif_setGpioDirection,
    hal_coral_perif_getGpioDirection,
    hal_coral_perif_setGpioOutputData,
    hal_coral_perif_getGpioInputData,
    hal_coral_perif_setGpioOutputAutoMode,
    hal_coral_perif_getGpioOutputAutoMode,
    hal_coral_perif_setGpioOutputAutoPatt,
    hal_coral_perif_getGpioOutputAutoPatt,
};

const static HAL_FUNC_VEC_T _coral_func_vector = {
    /* chip multiplexing functions */
    &chip_func_vec,

    /* port multiplexing functions */
    &port_func_vec,

    /* L2 protocol functions */
    &l2_func_vec,

    /* Mib multiplexing functions */
    &mib_func_vec,

    /* vlan multiplexing functions */
    &vlan_func_vec,

    /* mirror multiplexing functions */
    &mir_func_vec,

    /* QoS multiplexing functions */
    &qos_func_vec,

    /* switch multiplexing functions */
    &swc_func_vec,

    /* ifmon functions */
    &ifmon_func_vec,

    /* Mdio functions */
    &mdio_func_vec,

    /* SIF functions */
    &sif_func_vec,

    /* perif functions */
    &perif_func_vec,

    /* stp multiplexing functions */
    &stp_func_vec,

    /* Lag multiplexing functions */
    &lag_func_vec,

    /* Mcast multiplexing functions */
    &ipmc_func_vec,

    /* Security multiplexing functions */
    &sec_func_vec,

    /* sFlow multiplexing functions */
    &sflow_func_vec,

    /* Svlan multiplexing functions */
    &svlan_func_vec,

    /* stag multiplexing functions */
    &stag_func_vec,

    /* DoS multiplexing functions */
    &dos_func_vec,

    /* Acl multiplexing functions */
    &acl_func_vec,
};
#endif

const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858c[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    { AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    { AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    { AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0,  8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0,  9},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 10}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858e[] = {
    {AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    {AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    {AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    {AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    {AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    {AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    {AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    {AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {  AIR_INIT_PORT_TYPE_CPU, -1, -1, 10}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858h[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    { AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    { AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    { AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0,  8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0,  9},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 10}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858b[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    { AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    { AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0,  8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0,  9},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 10}
};

#ifdef AIR_LINUX
const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858f[] = {
    {AIR_INIT_PORT_TYPE_BASET, -1, 0, 0},
    {AIR_INIT_PORT_TYPE_BASET, -1, 1, 1},
    {AIR_INIT_PORT_TYPE_BASET, -1, 2, 2},
    {AIR_INIT_PORT_TYPE_BASET, -1, 3, 3},
    {AIR_INIT_PORT_TYPE_BASET, -1, 4, 4},
    {AIR_INIT_PORT_TYPE_BASET, -1, 5, 5},
    {AIR_INIT_PORT_TYPE_BASET, -1, 6, 6},
    {AIR_INIT_PORT_TYPE_BASET, -1, 7, 7},
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_coral_mac_port_map_8858d[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1, 0, 0},
    { AIR_INIT_PORT_TYPE_BASET, -1, 1, 1},
    { AIR_INIT_PORT_TYPE_BASET, -1, 2, 2},
    { AIR_INIT_PORT_TYPE_BASET, -1, 3, 3},
    { AIR_INIT_PORT_TYPE_BASET, -1, 4, 4},
    { AIR_INIT_PORT_TYPE_BASET, -1, 5, 5},
    { AIR_INIT_PORT_TYPE_BASET, -1, 6, 6},
    { AIR_INIT_PORT_TYPE_BASET, -1, 7, 7},
    {AIR_INIT_PORT_TYPE_XSGMII,  1, 0, 8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0, 0, 9},
};
#endif

/* clang-format off */
const static HAL_DRV_MAC_PORT_INFO_T _hal_coral_mac_port_info[] = {
    {
        HAL_CORAL_DEVICE_ID_AN8858C,
        (sizeof(_hal_coral_mac_port_map_8858c) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858c[0]
    },
    {
        HAL_CORAL_DEVICE_ID_AN8858E,
        (sizeof(_hal_coral_mac_port_map_8858e) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858e[0]
    },
    {
        HAL_CORAL_DEVICE_ID_AN8858H,
        (sizeof(_hal_coral_mac_port_map_8858h) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858h[0]
    },
    {
        HAL_CORAL_DEVICE_ID_AN8858B,
        (sizeof(_hal_coral_mac_port_map_8858b) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858b[0]
    },
#ifdef AIR_LINUX
    {
        HAL_CORAL_DEVICE_ID_AN8858F,
        (sizeof(_hal_coral_mac_port_map_8858f) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858f[0]
    },
    {
        HAL_CORAL_DEVICE_ID_AN8858D,
        (sizeof(_hal_coral_mac_port_map_8858d) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_coral_mac_port_map_8858d[0]
    },
#endif
    {
        /* always put into last and use to indicate entry end */
        HAL_INVALID_DEVICE_ID,
        0,
        NULL
    }
};
/* clang-format on */

/* Based table attribute to fill this table field */
const HAL_DRIVER_T _ext_coral_e1_driver = {
    "Driver for Coral switch chip",
    &_coral_func_vector,
    &_hal_coral_mac_port_info[0]
};

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_drv_initDriver
 * PURPOSE:
 *      hal_coral_drv_initDriver() is a function that will base on revision_id
 *      to find best-matched EN8851 driver entity.
 *
 * INPUT:
 *      revision_id     -- The revision ID of this switch chip.
 * OUTPUT:
 *      pptr_hal_driver -- The pointer (address) of the EN8851 device driver
 *                         that used for this revision ID.
 * RETURN:
 *      AIR_E_OK        -- Retrieve EN8851 device driver successfully.
 *      AIR_E_OTHERS    -- Fail to retrieve EN8851 device driver.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_coral_drv_initDriver(
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver)
{
    /* use revision ID for switch case statement */
    switch (revision_id)
    {
        case HAL_CORAL_REVISION_ID_E1:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_coral_e1_driver;
            break;
        default:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_coral_e1_driver;
            break;
    }
    return (AIR_E_OK);
}
