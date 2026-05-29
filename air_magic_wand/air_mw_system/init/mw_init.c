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

/* FILE NAME:   mw_init.c
 * PURPOSE:
 *      Middleware initialization function call.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <mw_init.h>
#include <syncd.h>
#include <mac_init.h>
#include <air_l2.h>
#include <air_swc.h>
#include <air_init.h>
#include <mw_platform.h>
#include <mw_portbmp.h>
#include <air_ver.h>
#ifdef AIR_SUPPORT_POE
#include "poe_main.h"
#endif
#include "air_dos.h"
#include "air_sec.h"
#if defined(AIR_SUPPORT_SFP) && defined(AIR_SUPPORT_SFP_WITH_THREAD)
#include "sfp_task.h"
#endif
#ifdef AIR_SUPPORT_ERPS
#include "erps.h"
#endif /* AIR_SUPPORT_ERPS */
#include "mw_rate_limit.h"
#if (!defined(AIR_LITE_MW))
#include "mw_sif.h"
#endif
#include "mw_acl.h"
#ifdef AIR_SUPPORT_TELNET
#include "mw_telnet.h"
#endif
#ifdef AIR_SUPPORT_SSH
#include "mw_ssh.h"
#endif
#ifdef AIR_SUPPORT_CLI
#include "cli_port.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_EN_CORAL
#define MW_VERSION_PLATFORM "AN8858, "
#elif defined AIR_LITE_MW
#define MW_VERSION_PLATFORM "AN8855, "
#else
#define MW_VERSION_PLATFORM "EN8851, "
#endif
#define MW_VERSION_MW "MW "MW_VER_STR", "
#define MW_VERSION_SDK "SDK "AIR_VER_SDK", "
#define MW_VERSION_FREERTOS "FreeRTOS Kernel V10.1.1"
#define SA_ZERO (1)
#define DA_ZERO (2)
#define BCAST_MAC_UDF_NUM (3)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
unsigned int _mw_total_port = MW_DEFAULT_MAX_PORT_NUM;
unsigned int _mw_eth_port = MW_DEFAULT_MAX_PORT_NUM;
unsigned int _mw_cpu_port = MW_DEFAULT_CPU_PORT;
#ifdef AIR_SUPPORT_POE
unsigned int _mw_poe_port = MW_DEFAULT_MAX_PORT_NUM;
#endif
AIR_PORT_BITMAP_T _mw_port_capacity;
AIR_PORT_BITMAP_T _mw_port_bmp_total;
AIR_PORT_BITMAP_T _mw_port_gphy;
static const C8_T _default_mac_addr[MAC_ADDRESS_LEN] = {0x0, 0xaa, 0xbb, 0x11, 0x22, 0x33};
GDMPSRAM_BSS_EX unsigned char _mw_mac_addr[MAC_ADDRESS_LEN] = {0};
GDMPSRAM_BSS_EX char _mw_sys_desc[MW_SYS_DESCRI_LEN_MAX] = {0};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
MW_ERROR_NO_T
_system_cpu_port_init(
    void);

static MW_ERROR_NO_T
_mw_system_block_port(
    UI8_T port);

/* FUNCTION NAME: mw_general_arp_acl
 * PURPOSE:
 *      Set ACL for forward the ARP packet.
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
mw_general_arp_acl()
{
    UI32_T           unit = 0;
    UI32_T           acl_rule_id = 0;
    AIR_ACL_RULE_T   acl_rule;
    I32_T            rc = MW_E_OP_INCOMPLETE;
    AIR_ACL_ACTION_T action;
    AIR_DOS_RATE_LIMIT_CFG_T dos_cfg = {0};
    UI32_T           uni_acl_ap_id = sys_mgmt_get_uni_acl_ap_id();

    mw_dos_setGlobalCfg(unit, TRUE);
    if (MW_ATTACK_ID_INVALID == uni_acl_ap_id)
    {
        if (MW_E_OK == MW_ATTACK_ID_GET_AVAILABLERULE(&uni_acl_ap_id, &dos_cfg, unit))
        {
            dos_cfg.pkt_thld = MW_ARP_PACKET_THRESHOLD;
            dos_cfg.time_span = 1000;
            dos_cfg.block_time = 1;
            dos_cfg.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
            air_dos_setRateLimitCfg(unit, uni_acl_ap_id, &dos_cfg);
            sys_mgmt_set_uni_acl_ap_id(uni_acl_ap_id);
        }
        else
        {
            MW_LOG_ERROR(SYSTEM, "Add ARP unicast dos failed, rc=%d.", rc);
            mw_dos_setGlobalCfg(unit, FALSE);
            return MW_E_OP_INCOMPLETE;
        }
    }

    rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
    if (MW_E_OK == rc)
    {
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = TRUE;
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        acl_rule.end = TRUE;
        acl_rule.key.etype = ETHTYPE_ARP;
        acl_rule.mask.etype = 0x3;
        acl_rule.field_valid = 0;
        acl_rule.field_valid = BIT(AIR_ACL_ETYPE_KEY);
        rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
        if (rc == AIR_E_OK)
        {
            osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
            action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
            action.field_valid |= (1U << AIR_ACL_MIB);
            action.port_fw = MW_ACL_ACT_PORT_FW_DEFAULT;
            action.field_valid |= (1U << AIR_ACL_FW_PORT);

            if (MW_ATTACK_ID_INVALID != uni_acl_ap_id)
            {
                action.attack_rate_id = uni_acl_ap_id;
                action.field_valid |= (1U << AIR_ACL_ATTACK);
            }

            rc = air_acl_setAction(unit, acl_rule_id, &action);
            if (AIR_E_OK != rc)
            {
                MW_LOG_CONSOLE_PRINTF("Add ARP ACL rule entry-id %d action fail, rc=%d.\n", acl_rule_id, rc);
                air_acl_delRule(unit, acl_rule_id);
            }
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Add ARP ACL rule entry-id %d failed, rc=%d.\n", acl_rule_id, rc);
        }
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Get free ACL rule-id for ARP failed !\n");
    }

    return rc;
}


/* FUNCTION NAME: allzero_mac_acl
 * PURPOSE:
 *      Set ACL for dropping the packet with SA/DA=all-zero MAC.
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
allzero_mac_acl(UI8_T mac_opt)
{
    UI32_T           unit = 0;
    UI32_T           acl_rule_id = 0;
    AIR_ACL_RULE_T   acl_rule;
    I32_T            rc = MW_E_OP_INCOMPLETE;
    AIR_ACL_ACTION_T action;

    rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
    if (MW_E_OK == rc)
    {
        osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
        acl_rule.rule_en = TRUE;
        AIR_PORT_BITMAP_COPY(acl_rule.portmap, PLAT_PORT_BMP_TOTAL);
        AIR_PORT_DEL(acl_rule.portmap, PLAT_CPU_PORT);
        acl_rule.end = TRUE;
        if (SA_ZERO == mac_opt)
        {
            acl_rule.mask.smac = 0x3f;
            acl_rule.field_valid |= (1U << AIR_ACL_SMAC_KEY);
        }
        else if (DA_ZERO == mac_opt)
        {
            acl_rule.mask.dmac = 0x3f;
            acl_rule.field_valid |= BIT(AIR_ACL_DMAC_KEY);
        }
        rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
        if (rc == AIR_E_OK)
        {
            osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
            action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id);/* Set mib id as acl rule id. */
            action.field_valid |= (1U << AIR_ACL_MIB);
            action.port_fw = MW_ACL_ACT_PORT_FW_DROP;
            action.field_valid |= (1U << AIR_ACL_FW_PORT);
            rc = air_acl_setAction(unit, acl_rule_id, &action);
            if (AIR_E_OK != rc)
            {
                MW_LOG_CONSOLE_PRINTF("Add all-zero MAC ACL rule entry-id %d action fail, rc=%d.\n", acl_rule_id, rc);
                air_acl_delRule(unit, acl_rule_id);
            }
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Add all-zero MAC ACL rule entry-id %d failed, rc=%d.\n", acl_rule_id, rc);
        }
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Get free ACL rule-id for all-zero MAC failed !\n");
    }

    return rc;
}

/* FUNCTION NAME: _mw_system_init
 * PURPOSE:
 *      Initial board related information
 *
 * INPUT:
 *      unit            --  The unit ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
_mw_system_init(
    const UI32_T unit)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    unsigned char i = 0;
    AIR_INIT_PORT_MAP_T *ptr_portMapList = NULL, *cur_ptr = NULL;
    UI32_T ver_len = 0;

    /*
     * Init port information
     */
    osapi_calloc((sizeof(AIR_INIT_PORT_MAP_T) * MW_DEFAULT_MAX_PORT_NUM), "MW", (void**)&ptr_portMapList);
    if(NULL != ptr_portMapList)
    {
        rc = air_init_getSdkPortMap(unit, &_mw_total_port, ptr_portMapList);
        if(AIR_E_OK == rc)
        {
            _mw_cpu_port = 0;
            AIR_PORT_BITMAP_CLEAR(_mw_port_capacity);
            AIR_PORT_BITMAP_CLEAR(_mw_port_bmp_total);

            cur_ptr = ptr_portMapList;
            for(i = 0; i < _mw_total_port; i++)
            {
                switch(cur_ptr->port_type)
                {
                    case AIR_INIT_PORT_TYPE_BASET:
                    case AIR_INIT_PORT_TYPE_ENHANCED_BASET:
                        AIR_PORT_ADD(_mw_port_bmp_total, (cur_ptr->port));
                        AIR_PORT_ADD(_mw_port_gphy, (cur_ptr->port));
                        break;
                    case AIR_INIT_PORT_TYPE_XSGMII:
                        if(AIR_INIT_PORT_SPEED_2500M == cur_ptr->max_speed)
                        {
                            AIR_PORT_ADD(_mw_port_capacity, (cur_ptr->port));
                        }
                        AIR_PORT_ADD(_mw_port_bmp_total, (cur_ptr->port));
                        break;
                    case AIR_INIT_PORT_TYPE_CPU:
                        _mw_cpu_port = cur_ptr->port;
                        AIR_PORT_ADD(_mw_port_bmp_total, (cur_ptr->port));
                        break;
                    default:
                        MW_LOG_CONSOLE_PRINTF("Warning: Wrong type of port info.\n");
                        break;
                }
                cur_ptr++;
            }
            _mw_eth_port = _mw_total_port - 1;
        }
        else
        {
            MW_LOG_CONSOLE_PRINTF("Error: Failed to get sdk port mapping!\n");
        }
        osapi_free(ptr_portMapList);
    }
    else
    {
        MW_LOG_CONSOLE_PRINTF("Warning: Failed to initialize port, use default port mapping!\n");
    }

    rc |= mw_acl_init(unit);
    rc |= mw_rate_limit_init();
#if (!defined(AIR_LITE_MW))
    rc |= mw_sif_init();
#endif
    /* ARP ACL ID should be smaller than allzero mac ACL ID */
#ifndef AIR_EN_CORAL
    rc |= mw_general_arp_acl();
#endif
    rc |= allzero_mac_acl(SA_ZERO);
    rc |= allzero_mac_acl(DA_ZERO);
    rc |= _system_cpu_port_init();

    if(AIR_E_OK != rc)
    {
        ret = MW_E_OTHERS;
    }

    /*
     * Set system description
     */
    ver_len = osapi_strlen(MW_VERSION_PLATFORM) + osapi_strlen(MW_VERSION_MW) + osapi_strlen(MW_VERSION_SDK) + osapi_strlen(MW_VERSION_FREERTOS);
    if (ver_len > (MW_SYS_DESCRI_LEN_MAX - 1))
    {
        MW_LOG_CONSOLE_PRINTF("Error: Version too long.\n");
        return MW_E_BAD_PARAMETER;
    }

    osapi_memset(_mw_sys_desc, 0, MW_SYS_DESCRI_LEN_MAX);
    osapi_strcat(_mw_sys_desc, MW_VERSION_PLATFORM);
    osapi_strcat(_mw_sys_desc, MW_VERSION_MW);
    osapi_strcat(_mw_sys_desc, MW_VERSION_SDK);
    osapi_strcat(_mw_sys_desc, MW_VERSION_FREERTOS);

    MW_LOG_INIT_PRINTF("MW Version: %s\r\n", MW_VER_STR);

    return ret;
}

static MW_ERROR_NO_T
_mw_system_block_port(
    UI8_T port)
{
    UI8_T               idx = 0;
    UI32_T              unit = 0;
    AIR_PORT_BITMAP_T   port_bitmap = {0};

    /* Block port by set port-matrix */
    MW_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, idx)
    {
        if (PLAT_CPU_PORT == idx)
        {
            continue;
        }
        if(idx == port)
        {
            AIR_PORT_BITMAP_CLEAR(port_bitmap);
            AIR_PORT_ADD(port_bitmap, PLAT_CPU_PORT);
            air_port_setPortMatrix(unit, port, port_bitmap);
        }
        else
        {
            air_port_getPortMatrix(unit, idx, port_bitmap);
            AIR_PORT_DEL(port_bitmap, port);
            air_port_setPortMatrix(unit, idx, port_bitmap);
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: _system_cpu_port_init
 * PURPOSE:
 *      This function initializes the CPU port.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
_system_cpu_port_init(
    void)
{
    UI8_T                           storm_cfg = ENABLE;
    BOOL_T                          fc_en = DISABLE;
    UI32_T                          unit = 0;
    UI32_T                          port = PLAT_CPU_PORT;
    UI32_T                          bc_rate = 64;
    AIR_ERROR_NO_T                  air_rc = AIR_E_OK;
    AIR_SEC_STORM_TYPE_T            type = AIR_SEC_STORM_TYPE_BC;
    AIR_SEC_STORM_RATE_MODE_T       storm_mode = AIR_SEC_STORM_RATE_MODE_PPS;

    /* Set CPU port storm control enable [broadcast packets] */
    air_rc = air_sec_setPortStormCtrl(unit, port, type, storm_cfg);
    if(AIR_E_OK != air_rc)
    {
        MW_LOG_INIT_PRINTF("Set CPU port storm control failed. air_rc=%d.\n", air_rc);
        return MW_E_OP_INCOMPLETE;
    }
    /* Set CPU port storm control rate [broadcast packets] */
    air_rc = air_sec_setPortStormCtrlRate(unit, port, type, storm_mode, bc_rate);
    if(AIR_E_OK != air_rc)
    {
        MW_LOG_INIT_PRINTF("Set CPU port storm control failed. air_rc=%d.\n", air_rc);
        return MW_E_OP_INCOMPLETE;
    }

    /* Set CPU port Tx flow control */
    air_rc = air_port_setFlowCtrl(unit, port, AIR_PORT_DIR_TX, fc_en);
    /* Set CPU port Rx flow control */
    air_rc |= air_port_setFlowCtrl(unit, port, AIR_PORT_DIR_RX, fc_en);
    if(AIR_E_OK != air_rc)
    {
        MW_LOG_INIT_PRINTF("Set CPU port flow control failed. air_rc=%d.\n", air_rc);
        return MW_E_OP_INCOMPLETE;
    }

    /* Block all ports except CPU port */
    MW_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }
        _mw_system_block_port(port);
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_init_sysmac(
    const UI32_T unit)
{
    UI32_T i = 0;
    I32_T rc = MW_E_OP_INCOMPLETE;

    /*
     * Init system mac
     */
    read_mac_addr(_mw_mac_addr);
    MW_LOG_CONSOLE_PRINTF("Initializing sys mac addr: %02X-%02X-%02X-%02X-%02X-%02X...\n"
            , _mw_mac_addr[0], _mw_mac_addr[1], _mw_mac_addr[2]
            , _mw_mac_addr[3], _mw_mac_addr[4], _mw_mac_addr[5]);
    for (; i < MAC_ADDRESS_LEN; i++)
    {
        if ((0 != _mw_mac_addr[i]) && (0xFF != _mw_mac_addr[i]))
        {
            break;
        }
    }
    if (MAC_ADDRESS_LEN <= i)
    {
        /* Read all-zero from flash, set the default MAC address */
        osapi_memcpy(_mw_mac_addr, _default_mac_addr, MAC_ADDRESS_LEN);
        MW_LOG_CONSOLE_PRINTF("Initializing default mac addr: %02X-%02X-%02X-%02X-%02X-%02X...\n"
                , _mw_mac_addr[0], _mw_mac_addr[1], _mw_mac_addr[2]
                , _mw_mac_addr[3], _mw_mac_addr[4], _mw_mac_addr[5]);
    }

    /*
     * Set system MAC address
     */
    rc = air_swc_setSystemMac(unit, _mw_mac_addr);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to set system MAC address!\n");
    }

    return rc;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: mw_init_initModule
 * PURPOSE:
 *      This API is used to initialize the middleware modules.
 *
 * INPUT:
 *      unit            --  The unit ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_init_initModule(
    const UI32_T unit)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Initial board platform */
    _mw_system_init(unit);

    /* Initial DB task */
    MW_LOG_INIT_PRINTF("Initializing DB...\n");
    db_task_init();

    /* SyncD initialization */
    MW_LOG_INIT_PRINTF("Initializing syncd...\n");
    syncd_init();

    MW_LOG_INIT_PRINTF("Initializing switching module...\n");
    ret = mw_init_initSwitching(unit);
    if(MW_E_OK != ret)
    {
        MW_LOG_INIT_PRINTF("Failed to initialize switching module!\n");
    }
    /* sys_mgmt initialization */
    MW_LOG_INIT_PRINTF("Initializing system management...\n");
    ret = sys_mgmt_init();
    if(MW_E_OK != ret)
    {
        MW_LOG_INIT_PRINTF("Failed to initialize system management!\n");
    }
#ifdef AIR_SUPPORT_POE
    MW_LOG_INIT_PRINTF("Initializing PoE APP...\n");
    ret = poe_app_init();
    if (MW_E_NOT_SUPPORT == ret)
    {
        MW_LOG_INIT_PRINTF("Warnning: PoE driver is not initialized, disable PoE APP!\n");
        ret = MW_E_OK;
    }
    else if(MW_E_OK != ret)
    {
        MW_LOG_INIT_PRINTF("Failed to initialize PoE APP!\n");
    }
#endif

    /* SFP initialization */
#if defined(AIR_SUPPORT_SFP) && defined(AIR_SUPPORT_SFP_WITH_THREAD)
    MW_LOG_INIT_PRINTF("Initializing SFP task...\n");
    sfp_task_create();
#endif

#ifdef AIR_SUPPORT_CLI
    cli_cmd_port_replacePortLable();
#endif

    return ret;
}

/* FUNCTION NAME: mw_init_initSwitching
 * PURPOSE:
 *      This API is used to initialize the switching modules.
 *
 * INPUT:
 *      unit            --  The unit ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ALREADY_INITED
 *      MW_E_OP_STOPPED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_init_initSwitching(
    const UI32_T unit)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Initialize MAC related setting */
    mac_init();

#ifdef AIR_SUPPORT_LP
    /* Initial loop prevention module */
    ret = lp_init();
    if(MW_E_OK != ret)
    {
        MW_LOG_INIT_PRINTF("Failed to initialize loop prevention!\n");
    }
#endif

#ifdef AIR_SUPPORT_IGMP_SNP
    /* Initial IGMP snooping thread. */
    igmp_snp_init();
#endif

#ifdef AIR_SUPPORT_DHCP_SNOOP
    /* Initial DHCP snooping */
    dhcp_snp_init();
#endif /* AIR_SUPPORT_DHCP_SNOOP */

#ifdef AIR_SUPPORT_VOICE_VLAN
    /* Initial Voice VLAN */
    voice_vlan_init();
#endif /* AIR_SUPPORT_VOICE_VLAN */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    /* Initial STP */
    stp_sys_init();
#endif /* AIR_SUPPORT_RSTP || AIR_SUPPORT_MSTP */
#ifdef AIR_SUPPORT_LLDPD
    /*Initial LLDP daemon*/
    lldp_init();
#endif /*AIR_SUPPORT_LLDPD*/
#ifdef AIR_SUPPORT_ERPS
    erps_init();
#endif /* AIR_SUPPORT_ERPS */
#ifdef AIR_SUPPORT_TELNET
    mw_telnet_init();
#endif /* AIR_SUPPORT_TELNET */
#ifdef AIR_SUPPORT_SSH
    mw_ssh_init();
#endif /* AIR_SUPPORT_SSH  */
    return ret;
}

MW_ERROR_NO_T
mw_init_preInitModule(
    const UI32_T unit)
{
    return _mw_init_sysmac(unit);
}
