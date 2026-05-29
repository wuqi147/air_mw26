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

/* FILE NAME:  hal_pearl_drv.c
 * PURPOSE:
 *      Provide HAL EN8851 driver and driver functions.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/pearl/hal_pearl_drv.h>

#include <hal/common/hal.h>
#include <hal/common/hal_dev.h>
#include <hal/common/hal_drv.h>
#include <hal/switch/pearl/hal_pearl_acl.h>
#include <hal/switch/pearl/hal_pearl_chip.h>
#include <hal/switch/pearl/hal_pearl_dos.h>
#include <hal/switch/pearl/hal_pearl_ifmon.h>
#include <hal/switch/pearl/hal_pearl_ipmc.h>
#include <hal/switch/pearl/hal_pearl_l2.h>
#include <hal/switch/pearl/hal_pearl_lag.h>
#include <hal/switch/pearl/hal_pearl_mdio.h>
#include <hal/switch/pearl/hal_pearl_mib.h>
#include <hal/switch/pearl/hal_pearl_mirror.h>
#include <hal/switch/pearl/hal_pearl_perif.h>
#include <hal/switch/pearl/hal_pearl_port.h>
#include <hal/switch/pearl/hal_pearl_qos.h>
#include <hal/switch/pearl/hal_pearl_sec.h>
#include <hal/switch/pearl/hal_pearl_sflow.h>
#include <hal/switch/pearl/hal_pearl_sif.h>
#include <hal/switch/pearl/hal_pearl_stag.h>
#include <hal/switch/pearl/hal_pearl_stp.h>
#include <hal/switch/pearl/hal_pearl_svlan.h>
#include <hal/switch/pearl/hal_pearl_swc.h>
#include <hal/switch/pearl/hal_pearl_vlan.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
// extern HAL_INTR_INFO_T _ext_hal_pearl_intr_info;

const static HAL_CHIP_FUNC_VEC_T chip_func_vec = {
    hal_pearl_chip_readDeviceInfo,
    hal_pearl_chip_init,
    hal_pearl_chip_deinit,
};

const static HAL_STP_FUNC_VEC_T stp_func_vec = {
    NULL,
    NULL,
    hal_pearl_stp_setPortState,
    hal_pearl_stp_getPortState,
};

const static HAL_MIR_FUNC_VEC_T mir_func_vec = {
    hal_pearl_mir_init,
    NULL,
    hal_pearl_mir_addSession,
    hal_pearl_mir_delSession,
    hal_pearl_mir_getSession,
    hal_pearl_mir_setSessionAdminMode,
    hal_pearl_mir_getSessionAdminMode,
    hal_pearl_mir_setMirrorPort,
    hal_pearl_mir_getMirrorPort,
};

const static HAL_PORT_FUNC_VEC_T port_func_vec = {
    hal_pearl_port_init,
    hal_pearl_port_deinit,
    hal_pearl_port_setSerdesMode,
    hal_pearl_port_getSerdesMode,
    hal_pearl_port_setPhyAutoNego,
    hal_pearl_port_getPhyAutoNego,
    hal_pearl_port_setPhyLocalAdvAbility,
    hal_pearl_port_getPhyLocalAdvAbility,
    hal_pearl_port_getPhyRemoteAdvAbility,
    hal_pearl_port_setSpeed,
    hal_pearl_port_getSpeed,
    hal_pearl_port_setDuplex,
    hal_pearl_port_getDuplex,
    hal_pearl_port_setBackPressure,
    hal_pearl_port_getBackPressure,
    hal_pearl_port_setFlowCtrl,
    hal_pearl_port_getFlowCtrl,
    hal_pearl_port_setPhyLoopBack,
    hal_pearl_port_getPhyLoopBack,
    hal_pearl_port_getPortStatus,
    hal_pearl_port_setAdminState,
    hal_pearl_port_getAdminState,
    hal_pearl_port_setPhySmartSpeedDown,
    hal_pearl_port_getPhySmartSpeedDown,
    hal_pearl_port_setPortMatrix,
    hal_pearl_port_getPortMatrix,
    hal_pearl_port_setVlanMode,
    hal_pearl_port_getVlanMode,
    hal_pearl_port_setPhyLedOnCtrl,
    hal_pearl_port_getPhyLedOnCtrl,
    hal_pearl_port_setComboMode,
    hal_pearl_port_getComboMode,
    hal_pearl_port_setPhyLedCtrlMode,
    hal_pearl_port_getPhyLedCtrlMode,
    hal_pearl_port_setPhyLedForceState,
    hal_pearl_port_getPhyLedForceState,
    hal_pearl_port_setPhyLedForcePattCfg,
    hal_pearl_port_getPhyLedForcePattCfg,
    hal_pearl_port_triggerCableTest,
    hal_pearl_port_getCableTestRawData,
    hal_pearl_port_setPhyOpMode,
    hal_pearl_port_getPhyOpMode,
    NULL,
};

const static HAL_LAG_FUNC_VEC_T lag_func_vec = {
    hal_pearl_lag_init,
    NULL,
    hal_pearl_lag_getMaxGroupCnt,
    hal_pearl_lag_createGroup,
    hal_pearl_lag_destroyGroup,
    hal_pearl_lag_getGroup,
    hal_pearl_lag_setMember,
    hal_pearl_lag_getMember,
    hal_pearl_lag_setHashControl,
    hal_pearl_lag_getHashControl,
};

const static HAL_L2_FUNC_VEC_T l2_func_vec = {
    hal_pearl_l2_init,
    hal_pearl_l2_deinit,
    hal_pearl_l2_addMacAddr,
    hal_pearl_l2_delMacAddr,
    hal_pearl_l2_getMacAddr,
    hal_pearl_l2_getNextMacAddr,
    hal_pearl_l2_searchMacAddr,
    hal_pearl_l2_searchNextMacAddr,
    hal_pearl_l2_clearMacAddr,
    hal_pearl_l2_setMacAddrAgeOut,
    hal_pearl_l2_getMacAddrAgeOut,
    hal_pearl_l2_setMacAddrAgeOutMode,
    hal_pearl_l2_getMacAddrAgeOutMode,
    hal_pearl_l2_getMacBucketSize,
    hal_pearl_l2_setForwardMode,
    hal_pearl_l2_getForwardMode,
    hal_pearl_l2_flushMacAddr,
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
    hal_pearl_ipmc_setMcastLookupType,
    hal_pearl_ipmc_getMcastLookupType,
    hal_pearl_ipmc_addMcastAddr,
    hal_pearl_ipmc_getMcastAddr,
    hal_pearl_ipmc_delMcastAddr,
    hal_pearl_ipmc_delAllMcastAddr,
    hal_pearl_ipmc_addMcastMember,
    hal_pearl_ipmc_delMcastMember,
    hal_pearl_ipmc_getMcastMemberCnt,
    hal_pearl_ipmc_getMcastBucketSize,
    hal_pearl_ipmc_getFirstMcastAddr,
    hal_pearl_ipmc_getNextMcastAddr,
    hal_pearl_ipmc_setPortIpmcMode,
    hal_pearl_ipmc_getPortIpmcMode,
    hal_pearl_ipmc_setPortLookupIpTypeCtrl,
    hal_pearl_ipmc_getPortLookupIpTypeCtrl,
};

const static HAL_MIB_FUNC_VEC_T mib_func_vec = {
    hal_pearl_mib_clearAllCnt,
    hal_pearl_mib_clearPortCnt,
    hal_pearl_mib_getPortCnt,
    hal_pearl_mib_getFeatureCnt,
};

const static HAL_VLAN_FUNC_VEC_T vlan_func_vec = {
    hal_pearl_vlan_init,
    NULL,
    hal_pearl_vlan_create,
    hal_pearl_vlan_destroy,
    hal_pearl_vlan_getVlan,
    hal_pearl_vlan_setFid,
    hal_pearl_vlan_getFid,
    hal_pearl_vlan_addMemberPort,
    hal_pearl_vlan_delMemberPort,
    hal_pearl_vlan_setMemberPort,
    hal_pearl_vlan_getMemberPort,
    hal_pearl_vlan_setIVL,
    hal_pearl_vlan_getIVL,
    hal_pearl_vlan_setIngressTagKeeping,
    hal_pearl_vlan_getIngressTagKeeping,
    hal_pearl_vlan_setEgsTagCtlEnable,
    hal_pearl_vlan_getEgsTagCtlEnable,
    hal_pearl_vlan_setPortEgsTagCtl,
    hal_pearl_vlan_getPortEgsTagCtl,
    hal_pearl_vlan_setPortAcceptFrameType,
    hal_pearl_vlan_getPortAcceptFrameType,
    hal_pearl_vlan_setPortAttr,
    hal_pearl_vlan_getPortAttr,
    hal_pearl_vlan_setIgrPortTagAttr,
    hal_pearl_vlan_getIgrPortTagAttr,
    hal_pearl_vlan_setPortEgsTagAttr,
    hal_pearl_vlan_getPortEgsTagAttr,
    hal_pearl_vlan_setPortPVID,
    hal_pearl_vlan_getPortPVID,
    hal_pearl_vlan_setCopyPriority,
    hal_pearl_vlan_getCopyPriority,
    hal_pearl_vlan_setUserPriority,
    hal_pearl_vlan_getUserPriority,
    hal_pearl_vlan_setStag,
    hal_pearl_vlan_getStag,
    hal_pearl_vlan_setPortPSVID,
    hal_pearl_vlan_getPortPSVID,
    hal_pearl_vlan_addMacBasedVlan,
    hal_pearl_vlan_getMacBasedVlan,
    hal_pearl_vlan_delMacBasedVlan,
    hal_pearl_vlan_addSubnetBasedVlan,
    hal_pearl_vlan_getSubnetBasedVlan,
    hal_pearl_vlan_delSubnetBasedVlan,
    hal_pearl_vlan_addProtocolBasedVlan,
    hal_pearl_vlan_getProtocolBasedVlan,
    hal_pearl_vlan_delProtocolBasedVlan,
    hal_pearl_vlan_setProtocolBasedVlanPortAttr,
    hal_pearl_vlan_getProtocolBasedVlanPortAttr,
    hal_pearl_vlan_setIngressVlanFilter,
    hal_pearl_vlan_getIngressVlanFilter,
};

const static HAL_QOS_FUNC_VEC_T qos_func_vec = {
    hal_pearl_qos_init,
    NULL,
    hal_pearl_qos_setRateLimitEnable,
    hal_pearl_qos_getRateLimitEnable,
    hal_pearl_qos_setRateLimitCfg,
    hal_pearl_qos_getRateLimitCfg,
    NULL,
    hal_pearl_qos_getRateLimitExcludeMgmt,
    hal_pearl_qos_setTrustMode,
    hal_pearl_qos_getTrustMode,
    hal_pearl_qos_setPriToQueue,
    hal_pearl_qos_getPriToQueue,
    hal_pearl_qos_setDscpToPri,
    hal_pearl_qos_getDscpToPri,
    hal_pearl_qos_setScheduleMode,
    hal_pearl_qos_getScheduleMode,
    hal_pearl_qos_setShaperMode,
    hal_pearl_qos_getShaperMode,
    hal_pearl_qos_setShaperRate,
    hal_pearl_qos_getShaperRate,
    hal_pearl_qos_setShaperWeight,
    hal_pearl_qos_getShaperWeight,
    hal_pearl_qos_setPortPriority,
    hal_pearl_qos_getPortPriority,
    NULL,
    NULL,
};

const static HAL_SWC_FUNC_VEC_T swc_func_vec = {
    hal_pearl_swc_init,
    NULL,

    hal_pearl_swc_setMgmtFrameCfg,
    hal_pearl_swc_getMgmtFrameCfg,
    hal_pearl_swc_setSystemMac,
    hal_pearl_swc_getSystemMac,
    hal_pearl_swc_setJumboSize,
    hal_pearl_swc_getJumboSize,
    hal_pearl_swc_setProperty,
    hal_pearl_swc_getProperty,
    hal_pearl_swc_setLoopDetectCtrl,
    hal_pearl_swc_getLoopDetectCtrl,
    hal_pearl_swc_setLoopDetectFrameSrcMac,
    hal_pearl_swc_getLoopDetectFrameSrcMac,
    hal_pearl_swc_getLoopDetectStatus,
    hal_pearl_swc_clearLoopDetectStatus,
    hal_pearl_swc_getGlobalFreePages,
    hal_pearl_swc_getPortAllocatedPages,
    hal_pearl_swc_getPortUsedPages,
    NULL,
    NULL,
    hal_pearl_swc_getCapacity,
    hal_pearl_swc_getUsage,
    hal_pearl_swc_getChipUid,
};

const static HAL_SEC_FUNC_VEC_T sec_func_vec = {
    hal_pearl_sec_init,
    hal_pearl_sec_deinit,
    /* 802.1x */
    hal_pearl_sec_set8021xGlobalMode,
    hal_pearl_sec_get8021xGlobalMode,
    hal_pearl_sec_set8021xGlobalCfg,
    hal_pearl_sec_get8021xGlobalCfg,
    hal_pearl_sec_set8021xMacBasedCfg,
    hal_pearl_sec_get8021xMacBasedCfg,
    hal_pearl_sec_set8021xPortBasedCfg,
    hal_pearl_sec_get8021xPortBasedCfg,
    /* MAC Limits */
    hal_pearl_sec_setMacLimitGlobalMode,
    hal_pearl_sec_getMacLimitGlobalMode,
    hal_pearl_sec_setMacLimitGlobalCfg,
    hal_pearl_sec_getMacLimitGlobalCfg,
    hal_pearl_sec_setMacLimitPortCfg,
    hal_pearl_sec_getMacLimitPortCfg,
    /* Storm control */
    hal_pearl_sec_setPortStormCtrl,
    hal_pearl_sec_getPortStormCtrl,
    hal_pearl_sec_setPortStormCtrlRate,
    hal_pearl_sec_getPortStormCtrlRate,
    hal_pearl_sec_setStormCtrlMgmtMode,
    hal_pearl_sec_getStormCtrlMgmtMode,
};

const static HAL_SFLOW_FUNC_VEC_T sflow_func_vec = {
    hal_pearl_sflow_init,
    hal_pearl_sflow_deinit,
    hal_pearl_sflow_setSampling,
    hal_pearl_sflow_getSampling,
    hal_pearl_sflow_setMngFrm,
    hal_pearl_sflow_getMngFrm,
};

const static HAL_SVLAN_FUNC_VEC_T svlan_func_vec = {
    hal_pearl_svlan_addSvlan,
    hal_pearl_svlan_getSvlan,
    hal_pearl_svlan_delSvlan,
    hal_pearl_svlan_setCustomPort,
    hal_pearl_svlan_getCustomPort,
    hal_pearl_svlan_setServicePort,
    hal_pearl_svlan_getServicePort,
    hal_pearl_svlan_setEgsOuterTPID,
    hal_pearl_svlan_getEgsOuterTPID,
    hal_pearl_svlan_setEgsInnerTPID,
    hal_pearl_svlan_getEgsInnerTPID,
    hal_pearl_svlan_setRecvTagTPID,
    hal_pearl_svlan_getRecvTagTPID,
};

const static HAL_STAG_FUNC_VEC_T stag_func_vec = {
    hal_pearl_stag_init,
    hal_pearl_stag_deinit,
    hal_pearl_stag_setPort,
    hal_pearl_stag_getPort,
    hal_pearl_stag_setMode,
    hal_pearl_stag_getMode,
    hal_pearl_stag_encodeTxStag,
    hal_pearl_stag_decodeRxStag,
};

const static HAL_DOS_FUNC_VEC_T dos_func_vec = {
    hal_pearl_dos_init,
    hal_pearl_dos_deinit,
    hal_pearl_dos_setGlobalCfg,
    hal_pearl_dos_getGlobalCfg,
    hal_pearl_dos_setIcmpLengthLimit,
    hal_pearl_dos_getIcmpLengthLimit,
    hal_pearl_dos_setTcpFlagCfg,
    hal_pearl_dos_getTcpFlagCfg,
    hal_pearl_dos_setPortActionCfg,
    hal_pearl_dos_getPortActionCfg,
    hal_pearl_dos_clearPortActionCfg,
    hal_pearl_dos_clearActionCfg,
    hal_pearl_dos_setRateLimitCfg,
    hal_pearl_dos_getRateLimitCfg,
    hal_pearl_dos_clearRateLimitCfg,
    hal_pearl_dos_clearAllRateLimitCfg,
    hal_pearl_dos_getDropCnt,
    hal_pearl_dos_clearDropCnt,
};

const static HAL_ACL_FUNC_VEC_T acl_func_vec = {
    hal_pearl_acl_init,
    hal_pearl_acl_deinit,

    hal_pearl_acl_getGlobalState,
    hal_pearl_acl_setGlobalState,
    hal_pearl_acl_getPortState,
    hal_pearl_acl_setPortState,
    hal_pearl_acl_clearAll,
    hal_pearl_acl_getRule,
    hal_pearl_acl_setRule,
    hal_pearl_acl_delRule,
    hal_pearl_acl_getAction,
    hal_pearl_acl_setAction,
    hal_pearl_acl_delAction,
    NULL,
    NULL,
    hal_pearl_acl_getMibCnt,
    hal_pearl_acl_clearMibCnt,
    hal_pearl_acl_getUdfRule,
    hal_pearl_acl_setUdfRule,
    hal_pearl_acl_delUdfRule,
    hal_pearl_acl_clearUdfRule,
    hal_pearl_acl_getMeterTable,
    hal_pearl_acl_setMeterTable,
    hal_pearl_acl_getDropEn,
    hal_pearl_acl_setDropEn,
    hal_pearl_acl_getDropThrsh,
    hal_pearl_acl_setDropThrsh,
    hal_pearl_acl_getDropPbb,
    hal_pearl_acl_setDropPbb,
    hal_pearl_acl_getDropExMfrm,
    hal_pearl_acl_setDropExMfrm,
    hal_pearl_acl_getTrtcmEn,
    hal_pearl_acl_setTrtcmEn,
    hal_pearl_acl_getTrtcmTable,
    hal_pearl_acl_setTrtcmTable,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

const static HAL_MDIO_FUNC_VEC_T mdio_func_vec = {
    hal_pearl_mdio_writeC22,
    hal_pearl_mdio_readC22,
    hal_pearl_mdio_writeC45,
    hal_pearl_mdio_readC45,
    hal_pearl_mdio_writeC22FromIntBus,
    hal_pearl_mdio_readC22FromIntBus,
    hal_pearl_mdio_writeC45FromIntBus,
    hal_pearl_mdio_readC45FromIntBus,
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
    hal_pearl_ifmon_init,
    hal_pearl_ifmon_deinit,
    hal_pearl_ifmon_register,
    hal_pearl_ifmon_deregister,
    hal_pearl_ifmon_setMode,
    hal_pearl_ifmon_getMode,
    hal_pearl_ifmon_setMonitorState,
    hal_pearl_ifmon_getMonitorState,
    hal_pearl_ifmon_getPortStatus,
};

/* clang-format off */
const static HAL_SIF_FUNC_VEC_T sif_func_vec = {
    hal_pearl_sif_init,
    hal_pearl_sif_deinit,
    hal_pearl_sif_write,
    hal_pearl_sif_read,
    NULL,
    NULL,
};
/* clang-format on */

const static HAL_PERIF_FUNC_VEC_T perif_func_vec = {
    hal_pearl_perif_init,
    hal_pearl_perif_deinit,
    hal_pearl_perif_setGpioDirection,
    hal_pearl_perif_getGpioDirection,
    hal_pearl_perif_setGpioOutputData,
    hal_pearl_perif_getGpioInputData,
    hal_pearl_perif_setGpioOutputAutoMode,
    hal_pearl_perif_getGpioOutputAutoMode,
    hal_pearl_perif_setGpioOutputAutoPatt,
    hal_pearl_perif_getGpioOutputAutoPatt,
};

const static HAL_FUNC_VEC_T _pearl_func_vector = {
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

const static HAL_DRV_MAC_PORT_MAP_T _hal_pearl_mac_port_map_8855m[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0, 0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1, 1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2, 2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3, 3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4, 4},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0, 5},
    {   AIR_INIT_PORT_TYPE_CPU, -1, -1, 6}
};

const static HAL_DRV_MAC_PORT_MAP_T _hal_pearl_mac_port_map_8855h[] = {
    { AIR_INIT_PORT_TYPE_BASET, -1,  0, 0},
    { AIR_INIT_PORT_TYPE_BASET, -1,  1, 1},
    { AIR_INIT_PORT_TYPE_BASET, -1,  2, 2},
    { AIR_INIT_PORT_TYPE_BASET, -1,  3, 3},
    { AIR_INIT_PORT_TYPE_BASET, -1,  4, 4},
    {AIR_INIT_PORT_TYPE_XSGMII,  0,  0, 5}
};

/* clang-format off */
const static HAL_DRV_MAC_PORT_INFO_T _hal_pearl_mac_port_info[] = {
    {
        HAL_PEARL_DEVICE_ID_AN8855M,
        (sizeof(_hal_pearl_mac_port_map_8855m) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_pearl_mac_port_map_8855m[0]
    },
    {
        HAL_PEARL_DEVICE_ID_AN8855H,
        (sizeof(_hal_pearl_mac_port_map_8855h) / sizeof(HAL_DRV_MAC_PORT_MAP_T)),
        &_hal_pearl_mac_port_map_8855h[0]
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
const HAL_DRIVER_T _ext_pearl_e1_driver = {
    "Driver for Pearl switch chip",
    &_pearl_func_vector,
    &_hal_pearl_mac_port_info[0]
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_pearl_drv_initDriver
 * PURPOSE:
 *      hal_pearl_drv_initDriver() is a function that will base on revision_id
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
hal_pearl_drv_initDriver(
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver)
{
    /* use revision ID for switch case statement */
    switch (revision_id)
    {
        case HAL_PEARL_REVISION_ID_E1:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_pearl_e1_driver;
            break;
        default:
            (*pptr_hal_driver) = (HAL_DRIVER_T *)&_ext_pearl_e1_driver;
            break;
    }
    return (AIR_E_OK);
}
