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

/* FILE NAME:  hal_sco_drv.c
 * PURPOSE:
 *      Provide HAL EN8851 driver and driver functions.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_drv.h>

#include <hal/common/hal.h>
#include <hal/common/hal_dev.h>
#include <hal/common/hal_drv.h>
#include <hal/switch/sco/hal_sco_acl.h>
#include <hal/switch/sco/hal_sco_chip.h>
#include <hal/switch/sco/hal_sco_dos.h>
#include <hal/switch/sco/hal_sco_ifmon.h>
#include <hal/switch/sco/hal_sco_ipmc.h>
#include <hal/switch/sco/hal_sco_l2.h>
#include <hal/switch/sco/hal_sco_lag.h>
#include <hal/switch/sco/hal_sco_mdio.h>
#include <hal/switch/sco/hal_sco_mib.h>
#include <hal/switch/sco/hal_sco_mirror.h>
#include <hal/switch/sco/hal_sco_perif.h>
#include <hal/switch/sco/hal_sco_port.h>
#include <hal/switch/sco/hal_sco_qos.h>
#include <hal/switch/sco/hal_sco_sec.h>
#include <hal/switch/sco/hal_sco_sflow.h>
#include <hal/switch/sco/hal_sco_sif.h>
#include <hal/switch/sco/hal_sco_stag.h>
#include <hal/switch/sco/hal_sco_stp.h>
#include <hal/switch/sco/hal_sco_svlan.h>
#include <hal/switch/sco/hal_sco_swc.h>
#include <hal/switch/sco/hal_sco_vlan.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
// extern HAL_INTR_INFO_T _ext_hal_sco_intr_info;

const static HAL_CHIP_FUNC_VEC_T chip_func_vec = {
    hal_sco_chip_readDeviceInfo,
    hal_sco_chip_init,
    hal_sco_chip_deinit,
};

const static HAL_STP_FUNC_VEC_T stp_func_vec = {
    hal_sco_stp_init,
    hal_sco_stp_deinit,
    hal_sco_stp_setPortState,
    hal_sco_stp_getPortState,
};

const static HAL_MIR_FUNC_VEC_T mir_func_vec = {
    hal_sco_mir_init,
    hal_sco_mir_deinit,
    hal_sco_mir_addSession,
    hal_sco_mir_delSession,
    hal_sco_mir_getSession,
    hal_sco_mir_setSessionAdminMode,
    hal_sco_mir_getSessionAdminMode,
    hal_sco_mir_setMirrorPort,
    hal_sco_mir_getMirrorPort,
};

const static HAL_PORT_FUNC_VEC_T port_func_vec = {
    hal_sco_port_init,
    hal_sco_port_deinit,
    hal_sco_port_setSerdesMode,
    hal_sco_port_getSerdesMode,
    hal_sco_port_setPhyAutoNego,
    hal_sco_port_getPhyAutoNego,
    hal_sco_port_setPhyLocalAdvAbility,
    hal_sco_port_getPhyLocalAdvAbility,
    hal_sco_port_getPhyRemoteAdvAbility,
    hal_sco_port_setSpeed,
    hal_sco_port_getSpeed,
    hal_sco_port_setDuplex,
    hal_sco_port_getDuplex,
    hal_sco_port_setBackPressure,
    hal_sco_port_getBackPressure,
    hal_sco_port_setFlowCtrl,
    hal_sco_port_getFlowCtrl,
    hal_sco_port_setPhyLoopBack,
    hal_sco_port_getPhyLoopBack,
    hal_sco_port_getPortStatus,
    hal_sco_port_setAdminState,
    hal_sco_port_getAdminState,
    hal_sco_port_setPhySmartSpeedDown,
    hal_sco_port_getPhySmartSpeedDown,
    hal_sco_port_setPortMatrix,
    hal_sco_port_getPortMatrix,
    hal_sco_port_setVlanMode,
    hal_sco_port_getVlanMode,
    hal_sco_port_setPhyLedOnCtrl,
    hal_sco_port_getPhyLedOnCtrl,
    hal_sco_port_setComboMode,
    hal_sco_port_getComboMode,
    hal_sco_port_setPhyLedCtrlMode,
    hal_sco_port_getPhyLedCtrlMode,
    hal_sco_port_setPhyLedForceState,
    hal_sco_port_getPhyLedForceState,
    hal_sco_port_setPhyLedForcePattCfg,
    hal_sco_port_getPhyLedForcePattCfg,
    hal_sco_port_triggerCableTest,
    hal_sco_port_getCableTestRawData,
    hal_sco_port_setPhyOpMode,
    hal_sco_port_getPhyOpMode,
    hal_sco_port_triggerLinkDownCableTest,
};

const static HAL_LAG_FUNC_VEC_T lag_func_vec = {
    hal_sco_lag_init,
    hal_sco_lag_deinit,
    hal_sco_lag_getMaxGroupCnt,
    hal_sco_lag_createGroup,
    hal_sco_lag_destroyGroup,
    hal_sco_lag_getGroup,
    hal_sco_lag_setMember,
    hal_sco_lag_getMember,
    hal_sco_lag_setHashControl,
    hal_sco_lag_getHashControl,
};

const static HAL_L2_FUNC_VEC_T l2_func_vec = {
    hal_sco_l2_init,
    hal_sco_l2_deinit,
    hal_sco_l2_addMacAddr,
    hal_sco_l2_delMacAddr,
    hal_sco_l2_getMacAddr,
    hal_sco_l2_getNextMacAddr,
    hal_sco_l2_searchMacAddr,
    hal_sco_l2_searchNextMacAddr,
    hal_sco_l2_clearMacAddr,
    hal_sco_l2_setMacAddrAgeOut,
    hal_sco_l2_getMacAddrAgeOut,
    hal_sco_l2_setMacAddrAgeOutMode,
    hal_sco_l2_getMacAddrAgeOutMode,
    hal_sco_l2_getMacBucketSize,
    hal_sco_l2_setForwardMode,
    hal_sco_l2_getForwardMode,
    hal_sco_l2_flushMacAddr,
#ifdef AIR_EN_L2_SHADOW
    hal_sco_l2_lockL2FdbResource,
    hal_sco_l2_unlockL2FdbResource,
    hal_sco_l2_traverseMacAddr,
    hal_sco_l2_registerMacAddrNotifyCallback,
    hal_sco_l2_deregisterMacAddrNotifyCallback,
    hal_sco_l2_setShadowCfg,
    hal_sco_l2_getShadowCfg,
#endif /* End of AIR_EN_L2_SHADOW */
};

const static HAL_IPMC_FUNC_VEC_T ipmc_func_vec = {
    hal_sco_ipmc_setMcastLookupType,
    hal_sco_ipmc_getMcastLookupType,
    hal_sco_ipmc_addMcastAddr,
    hal_sco_ipmc_getMcastAddr,
    hal_sco_ipmc_delMcastAddr,
    hal_sco_ipmc_delAllMcastAddr,
    hal_sco_ipmc_addMcastMember,
    hal_sco_ipmc_delMcastMember,
    hal_sco_ipmc_getMcastMemberCnt,
    hal_sco_ipmc_getMcastBucketSize,
    hal_sco_ipmc_getFirstMcastAddr,
    hal_sco_ipmc_getNextMcastAddr,
    hal_sco_ipmc_setPortIpmcMode,
    hal_sco_ipmc_getPortIpmcMode,
    hal_sco_ipmc_setPortLookupIpTypeCtrl,
    hal_sco_ipmc_getPortLookupIpTypeCtrl,
};

const static HAL_MIB_FUNC_VEC_T mib_func_vec = {
    hal_sco_mib_clearAllCnt,
    hal_sco_mib_clearPortCnt,
    hal_sco_mib_getPortCnt,
    hal_sco_mib_getFeatureCnt,
};

const static HAL_VLAN_FUNC_VEC_T vlan_func_vec = {
    hal_sco_vlan_init,
    hal_sco_vlan_deinit,
    hal_sco_vlan_create,
    hal_sco_vlan_destroy,
    hal_sco_vlan_getVlan,
    hal_sco_vlan_setFid,
    hal_sco_vlan_getFid,
    hal_sco_vlan_addMemberPort,
    hal_sco_vlan_delMemberPort,
    hal_sco_vlan_setMemberPort,
    hal_sco_vlan_getMemberPort,
    hal_sco_vlan_setIVL,
    hal_sco_vlan_getIVL,
    hal_sco_vlan_setIngressTagKeeping,
    hal_sco_vlan_getIngressTagKeeping,
    hal_sco_vlan_setEgsTagCtlEnable,
    hal_sco_vlan_getEgsTagCtlEnable,
    hal_sco_vlan_setPortEgsTagCtl,
    hal_sco_vlan_getPortEgsTagCtl,
    hal_sco_vlan_setPortAcceptFrameType,
    hal_sco_vlan_getPortAcceptFrameType,
    hal_sco_vlan_setPortAttr,
    hal_sco_vlan_getPortAttr,
    hal_sco_vlan_setIgrPortTagAttr,
    hal_sco_vlan_getIgrPortTagAttr,
    hal_sco_vlan_setPortEgsTagAttr,
    hal_sco_vlan_getPortEgsTagAttr,
    hal_sco_vlan_setPortPVID,
    hal_sco_vlan_getPortPVID,
    hal_sco_vlan_setCopyPriority,
    hal_sco_vlan_getCopyPriority,
    hal_sco_vlan_setUserPriority,
    hal_sco_vlan_getUserPriority,
    hal_sco_vlan_setStag,
    hal_sco_vlan_getStag,
    hal_sco_vlan_setPortPSVID,
    hal_sco_vlan_getPortPSVID,
    hal_sco_vlan_addMacBasedVlan,
    hal_sco_vlan_getMacBasedVlan,
    hal_sco_vlan_delMacBasedVlan,
    hal_sco_vlan_addSubnetBasedVlan,
    hal_sco_vlan_getSubnetBasedVlan,
    hal_sco_vlan_delSubnetBasedVlan,
    hal_sco_vlan_addProtocolBasedVlan,
    hal_sco_vlan_getProtocolBasedVlan,
    hal_sco_vlan_delProtocolBasedVlan,
    hal_sco_vlan_setProtocolBasedVlanPortAttr,
    hal_sco_vlan_getProtocolBasedVlanPortAttr,
    hal_sco_vlan_setIngressVlanFilter,
    hal_sco_vlan_getIngressVlanFilter,
};

const static HAL_QOS_FUNC_VEC_T qos_func_vec = {
    hal_sco_qos_init,
    hal_sco_qos_deinit,
    hal_sco_qos_setRateLimitEnable,
    hal_sco_qos_getRateLimitEnable,
    hal_sco_qos_setRateLimitCfg,
    hal_sco_qos_getRateLimitCfg,
    hal_sco_qos_setRateLimitExcludeMgmt,
    hal_sco_qos_getRateLimitExcludeMgmt,
    hal_sco_qos_setTrustMode,
    hal_sco_qos_getTrustMode,
    hal_sco_qos_setPriToQueue,
    hal_sco_qos_getPriToQueue,
    hal_sco_qos_setDscpToPri,
    hal_sco_qos_getDscpToPri,
    hal_sco_qos_setScheduleMode,
    hal_sco_qos_getScheduleMode,
    hal_sco_qos_setShaperMode,
    hal_sco_qos_getShaperMode,
    hal_sco_qos_setShaperRate,
    hal_sco_qos_getShaperRate,
    hal_sco_qos_setShaperWeight,
    hal_sco_qos_getShaperWeight,
    hal_sco_qos_setPortPriority,
    hal_sco_qos_getPortPriority,
    NULL,
    NULL,
};

const static HAL_SWC_FUNC_VEC_T swc_func_vec = {
    hal_sco_swc_init,
    hal_sco_swc_deinit,

    hal_sco_swc_setMgmtFrameCfg,
    hal_sco_swc_getMgmtFrameCfg,
    hal_sco_swc_setSystemMac,
    hal_sco_swc_getSystemMac,
    hal_sco_swc_setJumboSize,
    hal_sco_swc_getJumboSize,
    hal_sco_swc_setProperty,
    hal_sco_swc_getProperty,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    hal_sco_swc_getGlobalFreePages,
    hal_sco_swc_getPortAllocatedPages,
    hal_sco_swc_getPortUsedPages,
    NULL,
    NULL,
    hal_sco_swc_getCapacity,
    hal_sco_swc_getUsage,
    hal_sco_swc_getChipUid,
};

const static HAL_SEC_FUNC_VEC_T sec_func_vec = {
    hal_sco_sec_init,
    hal_sco_sec_deinit,
    /* 802.1x */
    hal_sco_sec_set8021xGlobalMode,
    hal_sco_sec_get8021xGlobalMode,
    hal_sco_sec_set8021xGlobalCfg,
    hal_sco_sec_get8021xGlobalCfg,
    hal_sco_sec_set8021xMacBasedCfg,
    hal_sco_sec_get8021xMacBasedCfg,
    hal_sco_sec_set8021xPortBasedCfg,
    hal_sco_sec_get8021xPortBasedCfg,
    /* MAC Limits */
    hal_sco_sec_setMacLimitGlobalMode,
    hal_sco_sec_getMacLimitGlobalMode,
    hal_sco_sec_setMacLimitGlobalCfg,
    hal_sco_sec_getMacLimitGlobalCfg,
    hal_sco_sec_setMacLimitPortCfg,
    hal_sco_sec_getMacLimitPortCfg,
    /* Storm Control */
    hal_sco_sec_setPortStormCtrl,
    hal_sco_sec_getPortStormCtrl,
    hal_sco_sec_setPortStormCtrlRate,
    hal_sco_sec_getPortStormCtrlRate,
    hal_sco_sec_setStormCtrlMgmtMode,
    hal_sco_sec_getStormCtrlMgmtMode,
};

const static HAL_SFLOW_FUNC_VEC_T sflow_func_vec = {
    hal_sco_sflow_init,
    hal_sco_sflow_deinit,
    hal_sco_sflow_setSampling,
    hal_sco_sflow_getSampling,
    hal_sco_sflow_setMngFrm,
    hal_sco_sflow_getMngFrm,
};

const static HAL_SVLAN_FUNC_VEC_T svlan_func_vec = {
    hal_sco_svlan_addSvlan,
    hal_sco_svlan_getSvlan,
    hal_sco_svlan_delSvlan,
    hal_sco_svlan_setCustomPort,
    hal_sco_svlan_getCustomPort,
    hal_sco_svlan_setServicePort,
    hal_sco_svlan_getServicePort,
    hal_sco_svlan_setEgsOuterTPID,
    hal_sco_svlan_getEgsOuterTPID,
    hal_sco_svlan_setEgsInnerTPID,
    hal_sco_svlan_getEgsInnerTPID,
    hal_sco_svlan_setRecvTagTPID,
    hal_sco_svlan_getRecvTagTPID,
};

const static HAL_STAG_FUNC_VEC_T stag_func_vec = {
    hal_sco_stag_init,
    hal_sco_stag_deinit,
    hal_sco_stag_setPort,
    hal_sco_stag_getPort,
    hal_sco_stag_setMode,
    hal_sco_stag_getMode,
    hal_sco_stag_encodeTxStag,
    hal_sco_stag_decodeRxStag,
};

const static HAL_DOS_FUNC_VEC_T dos_func_vec = {
    hal_sco_dos_init,
    hal_sco_dos_deinit,
    hal_sco_dos_setGlobalCfg,
    hal_sco_dos_getGlobalCfg,
    hal_sco_dos_setIcmpLengthLimit,
    hal_sco_dos_getIcmpLengthLimit,
    hal_sco_dos_setTcpFlagCfg,
    hal_sco_dos_getTcpFlagCfg,
    hal_sco_dos_setPortActionCfg,
    hal_sco_dos_getPortActionCfg,
    hal_sco_dos_clearPortActionCfg,
    hal_sco_dos_clearActionCfg,
    hal_sco_dos_setRateLimitCfg,
    hal_sco_dos_getRateLimitCfg,
    hal_sco_dos_clearRateLimitCfg,
    hal_sco_dos_clearAllRateLimitCfg,
    hal_sco_dos_getDropCnt,
    hal_sco_dos_clearDropCnt,
};

const static HAL_ACL_FUNC_VEC_T acl_func_vec = {
    hal_sco_acl_init,
    hal_sco_acl_deinit,

    hal_sco_acl_getGlobalState,
    hal_sco_acl_setGlobalState,
    hal_sco_acl_getPortState,
    hal_sco_acl_setPortState,
    hal_sco_acl_clearAll,
    hal_sco_acl_getRule,
    hal_sco_acl_setRule,
    hal_sco_acl_delRule,
    hal_sco_acl_getAction,
    hal_sco_acl_setAction,
    hal_sco_acl_delAction,
    hal_sco_acl_getBlockState,
    hal_sco_acl_setBlockState,
    hal_sco_acl_getMibCnt,
    hal_sco_acl_clearMibCnt,
    hal_sco_acl_getUdfRule,
    hal_sco_acl_setUdfRule,
    hal_sco_acl_delUdfRule,
    hal_sco_acl_clearUdfRule,
    hal_sco_acl_getMeterTable,
    hal_sco_acl_setMeterTable,
    hal_sco_acl_getDropEn,
    hal_sco_acl_setDropEn,
    hal_sco_acl_getDropThrsh,
    hal_sco_acl_setDropThrsh,
    hal_sco_acl_getDropPbb,
    hal_sco_acl_setDropPbb,
    hal_sco_acl_getDropExMfrm,
    hal_sco_acl_setDropExMfrm,
    hal_sco_acl_getTrtcmEn,
    hal_sco_acl_setTrtcmEn,
    hal_sco_acl_getTrtcmTable,
    hal_sco_acl_setTrtcmTable,
    hal_sco_acl_getL3RoutingTable,
    hal_sco_acl_setL3RoutingTable,
    NULL,
    NULL,
    NULL,
};

const static HAL_MDIO_FUNC_VEC_T mdio_func_vec = {
    hal_sco_mdio_writeC22,
    hal_sco_mdio_readC22,
    hal_sco_mdio_writeC45,
    hal_sco_mdio_readC45,
    NULL,
    NULL,
    NULL,
    NULL,
#ifdef AIR_EN_I2C_PHY
    hal_sco_mdio_writeC22ByI2c,
    hal_sco_mdio_readC22ByI2c,
    hal_sco_mdio_writeC45ByI2c,
    hal_sco_mdio_readC45ByI2c,
    hal_sco_mdio_writeRegByI2c,
    hal_sco_mdio_readRegByI2c,
#endif /* #ifdef AIR_EN_I2C_PHY */
};

const static HAL_IFMON_FUNC_VEC_T ifmon_func_vec = {
    hal_sco_ifmon_init,
    hal_sco_ifmon_deinit,
    hal_sco_ifmon_register,
    hal_sco_ifmon_deregister,
    hal_sco_ifmon_setMode,
    hal_sco_ifmon_getMode,
    hal_sco_ifmon_setMonitorState,
    hal_sco_ifmon_getMonitorState,
    hal_sco_ifmon_getPortStatus,
};

const static HAL_SIF_FUNC_VEC_T sif_func_vec = {
    hal_sco_sif_init,
    hal_sco_sif_deinit,
    hal_sco_sif_write,
    hal_sco_sif_read,
    hal_sco_sif_writeByRemote,
    hal_sco_sif_readByRemote,
};

const static HAL_PERIF_FUNC_VEC_T perif_func_vec = {
    hal_sco_perif_init,
    hal_sco_perif_deinit,
    hal_sco_perif_setGpioDirection,
    hal_sco_perif_getGpioDirection,
    hal_sco_perif_setGpioOutputData,
    hal_sco_perif_getGpioInputData,
    hal_sco_perif_setGpioOutputAutoMode,
    hal_sco_perif_getGpioOutputAutoMode,
    hal_sco_perif_setGpioOutputAutoPatt,
    hal_sco_perif_getGpioOutputAutoPatt,
};

const static HAL_FUNC_VEC_T       _scorpio_func_vector = {
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

const static HAL_DRV_MAC_PORT_MAP_T _hal_sco_mac_port_map_8851c[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    { AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    { AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    { AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0, 23},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0, 24},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  1,  8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  2,  9},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  3, 10},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 28}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_sco_mac_port_map_8851e[] = {
    {AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    {AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    {AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    {AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    {AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    {AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    {AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    {AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {  AIR_INIT_PORT_TYPE_CPU, -1, -1, 28}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_sco_mac_port_map_8853c[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0,  0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1,  1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2,  2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3,  3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4,  4},
    { AIR_INIT_PORT_TYPE_BASET, -1,  5,  5},
    { AIR_INIT_PORT_TYPE_BASET, -1,  6,  6},
    { AIR_INIT_PORT_TYPE_BASET, -1,  7,  7},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0, 23},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  1,  8},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  2,  9},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  3, 10},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0, 24},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  1, 11},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  2, 12},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  3, 13},
    {AIR_INIT_PORT_TYPE_XSGMII,  2,  0, 25},
    {AIR_INIT_PORT_TYPE_XSGMII,  2,  1, 14},
    {AIR_INIT_PORT_TYPE_XSGMII,  2,  2, 15},
    {AIR_INIT_PORT_TYPE_XSGMII,  2,  3, 16},
    {AIR_INIT_PORT_TYPE_XSGMII,  3,  0, 26},
    {AIR_INIT_PORT_TYPE_XSGMII,  3,  1, 17},
    {AIR_INIT_PORT_TYPE_XSGMII,  3,  2, 18},
    {AIR_INIT_PORT_TYPE_XSGMII,  3,  3, 19},
    {AIR_INIT_PORT_TYPE_XSGMII,  4,  0, 27},
    {AIR_INIT_PORT_TYPE_XSGMII,  4,  1, 20},
    {AIR_INIT_PORT_TYPE_XSGMII,  4,  2, 21},
    {AIR_INIT_PORT_TYPE_XSGMII,  4,  3, 22},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 28}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_sco_mac_port_map_8860c[] = {
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0, 23},
    {AIR_INIT_PORT_TYPE_XSGMII,  1,  0, 24},
    {AIR_INIT_PORT_TYPE_XSGMII,  2,  0, 25},
    {AIR_INIT_PORT_TYPE_XSGMII,  3,  0, 26},
    {AIR_INIT_PORT_TYPE_XSGMII,  4,  0, 27},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 28}
};

/* clang-format off */
const static HAL_DRV_MAC_PORT_INFO_T _hal_sco_mac_port_info[] = {
    {
        HAL_SCO_DEVICE_ID_EN8851C,
        (sizeof(_hal_sco_mac_port_map_8851c) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_sco_mac_port_map_8851c[0]
    },
    {
        HAL_SCO_DEVICE_ID_EN8851E,
        (sizeof(_hal_sco_mac_port_map_8851e) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_sco_mac_port_map_8851e[0]
    },
    {
        HAL_SCO_DEVICE_ID_EN8853C,
        (sizeof(_hal_sco_mac_port_map_8853c) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_sco_mac_port_map_8853c[0]
    },
    {
        HAL_SCO_DEVICE_ID_EN8860C,
        (sizeof(_hal_sco_mac_port_map_8860c) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_sco_mac_port_map_8860c[0]
    },
    {
        /* always put into last and use to indicate entry end */
        HAL_INVALID_DEVICE_ID,
        0,
        NULL
    }
};
/* clang-format on */

/* Based table attribute to fill this table field */
const HAL_DRIVER_T _ext_sco_e1_driver = {
    "Driver for Scorpio switch chip",
    &_scorpio_func_vector,
    &_hal_sco_mac_port_info[0],
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_sco_drv_initDriver
 * PURPOSE:
 *      hal_sco_drv_initDriver() is a function that will base on revision_id
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
hal_sco_drv_initDriver(
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver)
{
    /* use revision ID for switch case statement */
    switch (revision_id)
    {
        case HAL_SCO_REVISION_ID_E1:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_sco_e1_driver;
            break;
        default:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_sco_e1_driver;
            break;
    }
    return (AIR_E_OK);
}
