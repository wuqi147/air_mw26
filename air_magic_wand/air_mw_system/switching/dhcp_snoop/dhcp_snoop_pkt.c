/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:   dhcp_snoop_pkt.c
 * PURPOSE:
 *      Define DHCP snooping packet process function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>
#include <dhcp_snoop_log.h>
#include <dhcp_snoop_db.h>
#include <dhcp_snoop_pkt.h>

#include <inet_utils.h>
#include <vlan_utils.h>
#include <air_stag.h>
#include <air_l2.h>

#include <lwip/udp.h>
#include <syncd_api_lag.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define dhcp_snp_pkt_pbuf(ptr_info)      ((ptr_info)->ptr_pbuf)
#define dhcp_snp_pkt_dhcphdr(ptr_info)   ((ptr_info)->ptr_dhcphdr)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#ifdef DHCP_SNP_DEBUG_DUMP
static void
dhcp_snp_pkt_dumpInfo(
    DHCP_SNP_PKT_INFO_T *ptr_info,
    const C8_T          *ptr_title);
#else
static void
dhcp_snp_pkt_dumpInfo(
    DHCP_SNP_PKT_INFO_T *ptr_info,
    const C8_T          *ptr_title)
{
    UNUSED(ptr_info);
    UNUSED(ptr_title);
}
#endif /* DHCP_SNP_DEBUG_DUMP */

/* STATIC VARIABLE DECLARATIONS
 */
struct udp_pcb            *ptr_dhcpsnp_pcb_req = NULL;
struct udp_pcb            *ptr_dhcpsnp_pcb_ack = NULL;
const struct eth_addr     ethbcast             = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
static UI8_T              *ptr_dhcpsnp_pbuf    = NULL;

/* LOCAL SUBPROGRAM BODIES
 */

static inline MW_ERROR_NO_T
_dhcp_snp_pkt_copyToLocal(
    struct pbuf *ptr_src)
{
    dhcp_snp_dbg("Enter function");
    osapi_memset(ptr_dhcpsnp_pbuf, 0, DHCP_SNP_PKT_BUFF_SIZE);
    if (pbuf_copy_partial(ptr_src, ptr_dhcpsnp_pbuf, ptr_src->tot_len, 0) != ptr_src->tot_len)
    {
        dhcp_snp_warn("Fail to copy content to local buffer");
        return MW_E_OTHERS;
    }
    return MW_E_OK;
}

static inline MW_ERROR_NO_T
_dhcp_snp_pkt_copyToPbuf(
    struct pbuf *ptr_target,
    UI16_T      total_len)
{
    dhcp_snp_dbg("Enter function");
    if (pbuf_take(ptr_target, ptr_dhcpsnp_pbuf, total_len) != ERR_OK)
    {
        dhcp_snp_warn("Fail to copy content to packet buffer");
        return MW_E_OTHERS;
    }
    return MW_E_OK;
}

static inline MW_ERROR_NO_T
_dhcp_snp_chk_trunkMember(
    UI8_T port,
    UI8_T *trunk_id,
    UI8_T *lower_port)
{
    UI32_T pbmp = 0;
    UI8_T  t = 0, p = 0;

    dhcp_snp_db_lock();
    for (t = 0; t < MAX_TRUNK_NUM; ++t)
    {
        pbmp = dhcpsnp_db_trunk_group[t].members.member_bmp;
        if (BIT_CHK(pbmp, port - 1))
        {
            /* Find the lowest port id */
            for (p = 1; p <= PLAT_MAX_PORT_NUM; ++p)
            {
                if (BIT_CHK(pbmp, p - 1))
                {
                    if (((LAG_MODE_STATIC != dhcpsnp_db_trunk_group[t].members.mode) &&
                        (PORT_LACP_STATE_AGGREGATED == dhcpsnp_db_port_state[p - 1]))
                        ||
                        (LAG_MODE_STATIC == dhcpsnp_db_trunk_group[t].members.mode))
                    {
                        *trunk_id   = t + 1;
                        *lower_port = p;
                        dhcp_snp_db_unlock();
                        dhcp_snp_dbg("Recevie from trunk member port (%d), find the lowest port(%d) in trunk group(%d)", port, p, t);
                        return MW_E_OK;
                    }
                }
            }
        }
    }
    dhcp_snp_db_unlock();
    dhcp_snp_dbg("Not found in trunk group");
    return MW_E_ENTRY_NOT_FOUND;
}

static inline UI32_T
dhcp_snp_chk_trunkTxPbmp(
    UI32_T tx_pbmp)
{
    UI8_T  t = 0;
    UI32_T p = 0;
    UI32_T pbmp = tx_pbmp, tbmp = 0;

    dhcp_snp_db_lock();
    for (t = 0; t < MAX_TRUNK_NUM; t++)
    {
        tbmp = dhcpsnp_db_trunk_group[t].members.member_bmp << 1;
        if ((pbmp & tbmp) != 0)
        {
            pbmp = pbmp & ~(tbmp);
            for (p = 1; p <= PLAT_MAX_PORT_NUM; ++p)
            {
                if (BIT_CHK(tbmp, p))
                {
                    if (((LAG_MODE_STATIC != dhcpsnp_db_trunk_group[t].members.mode) &&
                        (PORT_LACP_STATE_AGGREGATED == dhcpsnp_db_port_state[p - 1]))
                        ||
                        (LAG_MODE_STATIC == dhcpsnp_db_trunk_group[t].members.mode))
                    {
                        pbmp |= BIT(p);
                        break;
                    }
                }
            }
        }
    }
    dhcp_snp_db_unlock();

    return pbmp;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_parseOptions(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    struct dhcp_msg        *ptr_dhcphdr  = dhcp_snp_pkt_dhcphdr(ptr_info);
    UI8_T                  *ptr_opt_last = ((UI8_T *)ptr_dhcphdr) + dhcp_snp_pkt_pbuf(ptr_info)->tot_len;
    UI8_T                  *ptr_optptr   = ptr_dhcphdr->options;
    BOOL_T                 is_dhcp       = FALSE;

    while (ptr_optptr <= ptr_opt_last)
    {
        const UI8_T op = ptr_optptr[DHCP_OPTION_OP];
        if (op == DHCP_OPTION_PAD)
        {
            ++ptr_optptr;
            continue;
        }
        else if (op == DHCP_OPTION_MESSAGE_TYPE)
        {
            is_dhcp = TRUE;
            ptr_info->msg_type = ptr_optptr[DHCP_OPTION_DATA];
        }
        else if (op == DHCP_OPTION_RELAY_AGENT)
        {
            UI8_T *ptr_suboptptr = ptr_optptr + 2;

            if (!is_dhcp)
            {
                break;
            }

            ptr_info->ptr_opt82 = ptr_optptr;
            if (ptr_suboptptr[DHCP_OPTION_OP] == DHCP_SUBOPTION_CIRCUIT_ID)
            {
                ptr_info->ptr_subopt_circuit_id = ptr_suboptptr;
                ptr_suboptptr += (ptr_suboptptr[DHCP_OPTION_LEN] + 2);
            }
            if (ptr_suboptptr[DHCP_OPTION_OP] == DHCP_SUBOPTION_REMOTE_ID)
            {
                ptr_info->ptr_subopt_remote_id = ptr_suboptptr;
            }
        }
        else if (op == DHCP_OPTION_END)
        {
            ptr_info->ptr_opt_end = ptr_optptr;
            break;
        }
        ptr_optptr += (ptr_optptr[DHCP_OPTION_LEN] + 2);
    }

    dhcp_snp_pkt_dumpInfo(ptr_info, "[_dhcp_snp_pkt_parseOptions] RX packet info:");
    if (is_dhcp && ptr_info->ptr_opt_end)
    {
        return MW_E_OK;
    }
    return MW_E_OTHERS;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_verifyReq(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    if ((ptr_info->msg_type != DHCP_DISCOVER) &&
            (ptr_info->msg_type != DHCP_REQUEST) &&
            (ptr_info->msg_type != DHCP_DECLINE) &&
            (ptr_info->msg_type != DHCP_RELEASE) &&
            (ptr_info->msg_type != DHCP_INFORM))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_info->action = DHCP_SNP_PKT_FORWARD;

    dhcp_snp_db_lock();
    if (dhcp_snp_db_portcfg(option82_enable, ptr_info->port_id))
    {
        if (ptr_info->ptr_opt82)
        {
            switch (dhcp_snp_db_portcfg(option82_mode, ptr_info->port_id))
            {
            case DHCP_SNP_OPT82_MODE_NOTOUCH:
                ptr_info->action = DHCP_SNP_PKT_FORWARD;
                break;
            case DHCP_SNP_OPT82_MODE_REPLACE:
                ptr_info->action = (DHCP_SNP_PKT_FORWARD | DHCP_SNP_PKT_OPTION82_REPLACE);
                break;
            case DHCP_SNP_OPT82_MODE_DISCARD:
                ptr_info->action = DHCP_SNP_PKT_DROP;
                break;
            default:
                ptr_info->action = DHCP_SNP_PKT_FORWARD;
                break;
            }
        }
        else
        {
            /* Forward and assign option 82 */
            ptr_info->action = (DHCP_SNP_PKT_FORWARD | DHCP_SNP_PKT_OPTION82_ADD);
        }
    }
    dhcp_snp_db_unlock();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_verifyAck(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    if ((ptr_info->msg_type != DHCP_OFFER) &&
            (ptr_info->msg_type != DHCP_ACK) &&
            (ptr_info->msg_type != DHCP_NAK))
    {
        return MW_E_BAD_PARAMETER;
    }

    ptr_info->action = DHCP_SNP_PKT_FORWARD;

    dhcp_snp_db_lock();
    if (dhcp_snp_db_portcfg(option82_enable, ptr_info->port_id))
    {
        const UI8_T fwd_miss_match = dhcp_snp_db_cfg(fwd_miss_match);

        if (ptr_info->ptr_opt82)
        {
            /* Strip option 82 */
            ptr_info->action = (DHCP_SNP_PKT_FORWARD | DHCP_SNP_PKT_OPTION82_REMOVE);
        }
        else
        {
            dhcp_snp_dbg("No option 82 found!");
            ptr_info->action = (fwd_miss_match) ? DHCP_SNP_PKT_FORWARD : DHCP_SNP_PKT_DROP;
        }
    }
    dhcp_snp_db_unlock();
    return MW_E_OK;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_recv(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    struct dhcp_msg *const ptr_dhcphdr = dhcp_snp_pkt_dhcphdr(ptr_info);
    const UI8_T            port_id     = ptr_info->port_id;

    dhcp_snp_dbg("Receive packet with VLAN = %d, sp = %d",
                 dhcp_snp_pkt_pbuf(ptr_info)->vlan_hdr.vid, port_id);

    if (ptr_dhcphdr->hlen > sizeof(ptr_dhcphdr->chaddr))
    {
        dhcp_snp_info("Invalid hlen: %d", ptr_dhcphdr->hlen);
        return MW_E_OTHERS;
    }
    else if (ptr_dhcphdr->cookie != PP_HTONL(DHCP_MAGIC_COOKIE))
    {
        dhcp_snp_info("Invalid magic cookie: 0x%x", (UI32_T)ptr_dhcphdr->cookie);
        return MW_E_OTHERS;
    }

    if (_dhcp_snp_pkt_parseOptions(ptr_info) == MW_E_OK)
    {
        /* client -> server */
        if (ptr_dhcphdr->op == DHCP_BOOTREQUEST)
        {
            return _dhcp_snp_pkt_verifyReq(ptr_info);
        }
        /* server -> client */
        else if (ptr_dhcphdr->op == DHCP_BOOTREPLY)
        {
            UI8_T is_trust_port = FALSE;

            dhcp_snp_db_lock();
            is_trust_port = dhcp_snp_db_portcfg(trust_port, port_id);
            dhcp_snp_db_unlock();

            /* Check if the packet is coming from trust port */
            if (is_trust_port)
            {
                return _dhcp_snp_pkt_verifyAck(ptr_info);
            }
            else
            {
                dhcp_snp_info("Discard DHCP reply from untrust port(%d)", port_id);
                return MW_E_OTHERS;
            }
        }
    }
    dhcp_snp_info("Not a valid DHCP packet.");
    return MW_E_OTHERS;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_stripOpt82(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    UI8_T *const ptr_optptr = ptr_info->ptr_opt82;

    dhcp_snp_dbg("Enter function");
    if (ptr_optptr)
    {
        const UI16_T opt82_len     = ptr_optptr[DHCP_OPTION_LEN] + 2;
        UI8_T *const ptr_optnxt    = ptr_optptr + opt82_len;
        UI8_T *const ptr_opt_last  = ((UI8_T *)dhcp_snp_pkt_dhcphdr(ptr_info)) + dhcp_snp_pkt_pbuf(ptr_info)->tot_len;

        if (ptr_optnxt > ptr_opt_last)
        {
            dhcp_snp_info("Invalid DHCP packet.");
            return MW_E_OTHERS;
        }
        osapi_memmove(ptr_optptr, ptr_optnxt, ptr_opt_last - ptr_optnxt);
        osapi_memset(ptr_opt_last - opt82_len, 0, opt82_len);
        ptr_info->ptr_opt_end -= opt82_len;
        dhcp_snp_dbg("Strip Option 82.");
    }
    return MW_E_OK;
}

static UI16_T
_dhcp_snp_pkt_parseSpToken(
    DHCP_SNP_PKT_INFO_T *ptr_info,
    C8_T                *ptr_dest,
    C8_T                *ptr_src,
    const UI16_T        max_len)
{
    C8_T    *ptr_ch = NULL, *ptr_last = NULL;
    UI16_T  len = 0, total_len = 0, port_id = ptr_info->port_id;

    if (ptr_dest == NULL || ptr_src == NULL)
    {
        return 0;
    }
    if (ptr_info->trunk_id != 0)
    {
        port_id = ptr_info->trunk_id + PLAT_MAX_PORT_NUM;
    }

    ptr_ch = ptr_src;
    ptr_last = ptr_src + osapi_strlen(ptr_src);
    while ((ptr_ch < ptr_last) && (total_len < (max_len - 1)))
    {
        if (*ptr_ch == '%')
        {
            if (0 == osapi_strncmp(ptr_ch, DHCP_SNP_OPT82_ID_HOSTNAME_TOKEN, DHCP_SNP_OPT82_ID_HOSTNAME_TOKEN_LEN))
            {
                osapi_snprintf(ptr_dest + total_len, max_len - total_len, dhcpsnp_db_hostname);
                len = osapi_strlen(ptr_dest + total_len);
                ptr_ch += DHCP_SNP_OPT82_ID_HOSTNAME_TOKEN_LEN;
            }
            else if (0 == osapi_strncmp(ptr_ch, DHCP_SNP_OPT82_ID_MAC_TOKEN, DHCP_SNP_OPT82_ID_MAC_TOKEN_LEN))
            {
                osapi_snprintf(ptr_dest + total_len, max_len - total_len, "%02X:%02X:%02X:%02X:%02X:%02X",
                               PLAT_MAC_ADDR[0], PLAT_MAC_ADDR[1], PLAT_MAC_ADDR[2],
                               PLAT_MAC_ADDR[3], PLAT_MAC_ADDR[4], PLAT_MAC_ADDR[5]);
                len = osapi_strlen(ptr_dest + total_len);
                ptr_ch += DHCP_SNP_OPT82_ID_MAC_TOKEN_LEN;
            }
            else if (0 == osapi_strncmp(ptr_ch, DHCP_SNP_OPT82_ID_PORT_TOKEN, DHCP_SNP_OPT82_ID_PORT_TOKEN_LEN))
            {
                osapi_snprintf(ptr_dest + total_len, max_len - total_len, "%02u", port_id);
                len = osapi_strlen(ptr_dest + total_len);
                ptr_ch += DHCP_SNP_OPT82_ID_PORT_TOKEN_LEN;
            }
            else
            {
                len = 1;
                *(ptr_dest + total_len) = *ptr_ch++;
            }
            total_len += len;
        }
        else
        {
            *(ptr_dest + total_len) = *ptr_ch++;
            total_len++;
        }
    }
    return total_len;
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_addOpt82(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    struct pbuf  *ptr_newpbuf   = NULL;
    UI16_T       opt82_len      = 0;
    UI8_T        circuit_id_len = 0;
    UI8_T        remote_id_len  = 0;
    UI8_T        replaceOpt82   = FALSE;
    const UI8_T  port_id        = ptr_info->port_id;
    const UI16_T vlan_id        = ptr_info->vlan_id;
    C8_T         circuit_id[MAX_OPT82_ID_LEN] = {0};
    C8_T         remote_id[MAX_OPT82_ID_LEN] = {0};

    dhcp_snp_dbg("Enter function");
    if (ptr_info->ptr_opt82)
    {
        if (ptr_info->action & DHCP_SNP_PKT_OPTION82_REPLACE)
        {
            dhcp_snp_dbg("Replace option 82");
            replaceOpt82 = TRUE;
        }
        else
        {
            dhcp_snp_dbg("No modify only forward.");
            return MW_E_OK;
        }
    }

    dhcp_snp_db_lock();
    if (dhcp_snp_db_portcfg(circuit_id_type, port_id) == DHCP_SNP_OPT82_CIR_ID_USER_CONFIG)
    {
        /* User config */
        circuit_id_len = _dhcp_snp_pkt_parseSpToken(ptr_info,
                         circuit_id,
                         dhcp_snp_db_portcfg(circuit_id, port_id),
                         MAX_OPT82_ID_LEN);
        dhcp_snp_dbg("circuit usr id(len: %d): %s", circuit_id_len, circuit_id);
    }
    if (circuit_id_len == 0)
    {
        UI8_T  cir_id_port_id = port_id;

        if (ptr_info->trunk_id != 0)
        {
            cir_id_port_id = ptr_info->trunk_id + PLAT_MAX_PORT_NUM;
        }
        /* default config */
        circuit_id[0] = ((vlan_id >> 8) & 0xff);
        circuit_id[1] = (vlan_id & 0xff);
        circuit_id[2] = (cir_id_port_id & 0xff);

        circuit_id_len = DHCP_SNP_OPT82_CIR_ID_DEFAULT_LEN;
    }
    opt82_len += circuit_id_len + 2;

    if (dhcp_snp_db_portcfg(remote_id_type, port_id) == DHCP_SNP_OPT82_REM_ID_USER_CONFIG)
    {
        /* User config */
        remote_id_len = _dhcp_snp_pkt_parseSpToken(ptr_info,
                        remote_id,
                        dhcp_snp_db_portcfg(remote_id, port_id),
                        MAX_OPT82_ID_LEN);
        dhcp_snp_dbg("remote usr id(len: %d): %s", remote_id_len, remote_id);
    }
    else if (dhcp_snp_db_portcfg(remote_id_type, port_id) == DHCP_SNP_OPT82_REM_ID_DEFAULT_MAC)
    {
        /* default config: MAC */
        osapi_memcpy(remote_id, PLAT_MAC_ADDR, DHCP_SNP_OPT82_REM_ID_DEFAULT_LEN);
        remote_id_len = DHCP_SNP_OPT82_REM_ID_MAC_LEN;
    }
    else if (dhcp_snp_db_portcfg(remote_id_type, port_id) == DHCP_SNP_OPT82_REM_ID_DEFAULT_IP)
    {
        /* default config: IP */
        struct netif *xNetIf = netif_find_default();

        if (xNetIf != NULL)
        {
            ip4_addr_copy(*(ip4_addr_t *)remote_id, *(ip_2_ip4(&(xNetIf->ip_addr))));
        }
        remote_id_len = DHCP_SNP_OPT82_REM_ID_IP_LEN;
    }
    if (remote_id_len == 0)
    {
        /* default config */
        osapi_memcpy(remote_id, PLAT_MAC_ADDR, DHCP_SNP_OPT82_REM_ID_DEFAULT_LEN);
        remote_id_len = DHCP_SNP_OPT82_REM_ID_DEFAULT_LEN;
    }
    opt82_len += remote_id_len + 2;
    dhcp_snp_db_unlock();

    /* Not to add option 82 in DHCP packet if length is out of range on option 82.
     */
    if (opt82_len > DHCP_SNP_OPT82_MAX_LEN)
    {
        dhcp_snp_info("Invalid option 82 configuration.");
        return MW_E_OK;
    }

    if (replaceOpt82)
    {
        _dhcp_snp_pkt_stripOpt82(ptr_info);
    }

    /* Clone the receive packet */
    ptr_newpbuf = pbuf_alloc(ptr_info->ethdr_offset, dhcp_snp_pkt_pbuf(ptr_info)->tot_len + opt82_len + 2, PBUF_RAM);
    if (ptr_newpbuf)
    {
        UI8_T *ptr_optptr = ptr_info->ptr_opt_end;

        /* Append option 82 */
        *ptr_optptr++ = DHCP_OPTION_RELAY_AGENT;
        *ptr_optptr++ = opt82_len;

        /* Append circuit id */
        *ptr_optptr++ = DHCP_SUBOPTION_CIRCUIT_ID;
        *ptr_optptr++ = circuit_id_len;
        osapi_memcpy(ptr_optptr, circuit_id, circuit_id_len);
        ptr_optptr += circuit_id_len;

        /* Append remote id */
        *ptr_optptr++ = DHCP_SUBOPTION_REMOTE_ID;
        *ptr_optptr++ = remote_id_len;
        osapi_memcpy(ptr_optptr, remote_id, remote_id_len);
        ptr_optptr += remote_id_len;

        /* Append option end */
        *ptr_optptr++ = DHCP_OPTION_END;
        /* Clone receive packet info */
        pbuf_copy_info(ptr_newpbuf, dhcp_snp_pkt_pbuf(ptr_info), ptr_info->ethdr_offset);
        dhcp_snp_pkt_pbuf(ptr_info) = ptr_newpbuf;
        dhcp_snp_dbg("Add Option 82.");
        return MW_E_OK;
    }
    dhcp_snp_dbg("No room in DHCP packet.");
    return MW_E_NO_MEMORY;
}

static inline void
_dhcp_snp_pkt_sendto(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    struct pbuf        *ptr_pbuf = dhcp_snp_pkt_pbuf(ptr_info);
    UI32_T             port      = 0;
    UI32_T             dport     = 0;
    AIR_PORT_BITMAP_T  tx_pbmp   = {0};
    ip_addr_t          dest_ip   = {};
    ip_addr_t          src_ip    = {};
    UI32_T             sp        = ptr_info->port_id;

    dhcp_snp_pkt_profiling_init();
    dhcp_snp_pkt_profiling_start();

#if PBUF_L2_FORWARDING
    ptr_pbuf->flags = (PBUF_FLAG_OVERRIDE_ETH | PBUF_FLAG_BYPASS_ARP);
#endif /* PBUF_L2_FORWARDING */

    /* Assign TX ports */
    ptr_pbuf->stag_hdr.tx_hdr.mode               = STAG_HDR_MODE_INSERT;
    ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP;
    ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp  = 0;
    if (!eth_addr_cmp(&ptr_pbuf->ether_hdr.dest, &ethbcast))
    {
        const UI32_T    unit  = 0;
        UI8_T           count = 0;
        AIR_MAC_ENTRY_T entry = {};

        osapi_memcpy(entry.mac, &ptr_pbuf->ether_hdr.dest, sizeof(AIR_MAC_T));
        entry.cvid  = (UI16_T)ptr_pbuf->vlan_hdr.vid;
        entry.flags = AIR_L2_MAC_ENTRY_FLAGS_IVL;
        if (air_l2_getMacAddr(unit, &count, &entry) == AIR_E_OK)
        {
            AIR_PORT_BITMAP_COPY(tx_pbmp, entry.port_bitmap);
        }
        /* Send all port in other case */
    }
    else
    {
        dhcp_snp_db_getVlanMember(ptr_pbuf->vlan_hdr.vid, tx_pbmp);
    }
    /* Send port expect receive port and CPU port */
    AIR_PORT_FOREACH(tx_pbmp, port)
    {
        if (port == PLAT_CPU_PORT || port == sp)
        {
            continue;
        }
        dport |= BIT(port);
    }
    dhcp_snp_dbg("sp = %d, dport = 0x%x\n", sp, dport);
    /* Check trunk bitmap */
    dport = dhcp_snp_chk_trunkTxPbmp(dport);
    dport &= (~BIT(sp % 32));

    ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = dport;
    dhcp_snp_pkt_profiling_end("air_l2_getMacAddr() get output portbmp");
    dhcp_snp_dbg("Send packet with VLAN = %d, dp = 0x%x",
                 ptr_pbuf->vlan_hdr.vid,
                 ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp);

    if (ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp == 0)
    {
        dhcp_snp_dbg("No TX port, discard the packet.");
        return;
    }

    dhcp_snp_pkt_profiling_start();
    ip_addr_set_ip4_u32_val(dest_ip, ip4_addr_get_u32(&ptr_info->dest_ip));
    ip_addr_set_ip4_u32_val(src_ip, ip4_addr_get_u32(&ptr_info->src_ip));
    if (ptr_info->dhcp_request)
    {
        udp_sendto_if_src(ptr_dhcpsnp_pcb_req,
                          ptr_pbuf,
                          &dest_ip,
                          DHCP_SNP_UDP_PORT_DHCP_SERVER,
                          netif_get_by_index(ptr_pbuf->if_idx),
                          &src_ip);
        dhcp_snp_dbg("Forward DHCP request packet");
    }
    else
    {
        udp_sendto_if_src(ptr_dhcpsnp_pcb_ack,
                          ptr_pbuf,
                          &dest_ip,
                          DHCP_SNP_UDP_PORT_DHCP_CLIENT,
                          netif_get_by_index(ptr_pbuf->if_idx),
                          &src_ip);
        dhcp_snp_dbg("Forward DHCP response packet");
    }
    dhcp_snp_pkt_profiling_end("lwip udp_send()");
}

static MW_ERROR_NO_T
_dhcp_snp_pkt_send(
    DHCP_SNP_PKT_INFO_T *ptr_info)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    dhcp_snp_dbg("Enter function");
    dhcp_snp_pkt_profiling_init();
    dhcp_snp_pkt_profiling_start();

    if (ptr_info->action & (DHCP_SNP_PKT_OPTION82_ADD | DHCP_SNP_PKT_OPTION82_REPLACE))
    {
        rc = _dhcp_snp_pkt_addOpt82(ptr_info);
    }
    else if (ptr_info->action & DHCP_SNP_PKT_OPTION82_REMOVE)
    {
        rc = _dhcp_snp_pkt_stripOpt82(ptr_info);
    }
    if (rc == MW_E_OK)
    {
        if (ptr_info->action & (DHCP_SNP_PKT_OPTION82_ADD | DHCP_SNP_PKT_OPTION82_REMOVE | DHCP_SNP_PKT_OPTION82_REPLACE))
        {
            if (_dhcp_snp_pkt_copyToPbuf(dhcp_snp_pkt_pbuf(ptr_info), dhcp_snp_pkt_pbuf(ptr_info)->tot_len) != MW_E_OK)
            {
                return MW_E_OTHERS;
            }
        }
    }
    dhcp_snp_pkt_profiling_end("dhcp_snp_pkt_add/strip_Opt82()");

    if (rc == MW_E_OK)
    {
        dhcp_snp_pkt_dumpInfo(ptr_info, "[_dhcp_snp_pkt_send] TX packet info:");
        _dhcp_snp_pkt_sendto(ptr_info);
#ifdef DHCP_SNP_BINDING_DB
        /* TODO: Add/update binding entry */
#endif /* DHCP_SNP_BINDING_DB */
    }
    return MW_E_OK;
}

static void
dhcp_snp_pkt_handle(
    void              *ptr_arg,
    struct udp_pcb    *ptr_pcb,
    struct pbuf       *ptr_pbuf,
    const ip_addr_t   *ptr_addr,
    u16_t             port)
{
    MW_ERROR_NO_T      ret      = MW_E_OK;
    DHCP_SNP_PKT_MSG_T *ptr_msg = NULL;
    UI8_T              enable   = 0;

    dhcp_snp_profiling_init();

    UNUSED(ptr_arg);
    UNUSED(ptr_pcb);

    dhcp_snp_db_lock();
    enable = dhcp_snp_db_cfg(global_enable);
    dhcp_snp_db_unlock();

    if (!enable)
    {
        dhcp_snp_dbg("Main function disabled");
        pbuf_free(ptr_pbuf);
        return;
    }

    dhcp_snp_profiling_start();
    if (osapi_malloc(sizeof(DHCP_SNP_PKT_MSG_T), DHCP_SNP_MODULE_NAME, (void **)&ptr_msg) != MW_E_OK)
    {
        dhcp_snp_err("Failed to allocated message for packet");
        pbuf_free(ptr_pbuf);
        return;
    }

    ptr_msg->msg_id       = DHCP_SNP_MSG_PKT;
    ptr_msg->ptr_pbuf     = ptr_pbuf;
    ip4_addr_copy(ptr_msg->src_ip, *(ip_2_ip4(ptr_addr)));
    ptr_msg->dest_ip      = *ip4_current_dest_addr();
    ptr_msg->dhcp_request = (port == DHCP_SNP_UDP_PORT_DHCP_CLIENT);
    /* Ethernet header & VLAN headers offset */
    ptr_msg->ethdr_offset = SIZEOF_ETH_HDR + SIZEOF_STAG_HDR + (ptr_pbuf->vlan_num * SIZEOF_VLAN_HDR) +
                            IPH_HL_BYTES(ip4_current_header()) + UDP_HLEN;
    ret = osapi_msgSend(DHCP_SNP_QUEUE_NAME, (UI8_T *)ptr_msg, sizeof(void *), DHCP_SNP_QUEUE_TIMEOUT);
    if (MW_E_OK != ret)
    {
        dhcp_snp_err("Fail to send message (ret = %u)", ret);
        osapi_free(ptr_msg);
        pbuf_free(ptr_pbuf);
    }
    else
    {
        dhcp_snp_dbg("Send message successfully");
    }
    dhcp_snp_profiling_end("Handle packet from lwip to packet queue");
    return;
}

#ifdef DHCP_SNP_DEBUG_DUMP
static void
dhcp_snp_pkt_dumpInfo(
    DHCP_SNP_PKT_INFO_T *ptr_info,
    const C8_T          *ptr_title)
{
    ip4_addr_p_t ip = {0};

    MW_LOG_PRINTF("%s\n", ptr_title);
    MW_LOG_PRINTF("  action                       = 0x%x\n", ptr_info->action);
    MW_LOG_PRINTF("  dhcp_request                 = 0x%x\n", ptr_info->dhcp_request);
    ip.addr = ntohl(ptr_info->src_ip.addr);
    MW_LOG_PRINTF("  src  IP                      = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
    ip.addr = ntohl(ptr_info->dest_ip.addr);
    MW_LOG_PRINTF("  dest IP                      = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
    MW_LOG_PRINTF("  source port                  = %d\n", ptr_info->port_id);
    MW_LOG_PRINTF("  VLAN id                      = %d\n", ptr_info->vlan_id);

    if (dhcp_snp_pkt_pbuf(ptr_info))
    {
        MW_LOG_PRINTF("  pbuf                         = 0x%p\n", dhcp_snp_pkt_pbuf(ptr_info));
        MW_LOG_PRINTF("  pbuf    dst                  = %02X:%02X:%02X:%02X:%02X:%02X\n",
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[0], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[1],
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[2], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[3],
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[4], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.dest.addr[5]);
        MW_LOG_PRINTF("  pbuf    src                  = %02X:%02X:%02X:%02X:%02X:%02X\n",
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[0], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[1],
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[2], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[3],
                     dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[4], dhcp_snp_pkt_pbuf(ptr_info)->ether_hdr.src.addr[5]);
        MW_LOG_PRINTF("  pbuf    len                  = %d\n", dhcp_snp_pkt_pbuf(ptr_info)->len);
        MW_LOG_PRINTF("  pbuf    stag_hdr.rx_hdr.sp   = %d\n", dhcp_snp_pkt_pbuf(ptr_info)->stag_hdr.rx_hdr.sp);
        MW_LOG_PRINTF("  pbuf    stag_hdr.tx_hdr.dp   = 0x%x\n", dhcp_snp_pkt_pbuf(ptr_info)->stag_hdr.tx_hdr.tx_stag_insert.dp);
    }

    if (dhcp_snp_pkt_dhcphdr(ptr_info))
    {
        MW_LOG_PRINTF("  dhcphdr                      = 0x%p\n", dhcp_snp_pkt_dhcphdr(ptr_info));
        MW_LOG_PRINTF("  dhcphdr hlen                 = 0x%x\n", dhcp_snp_pkt_dhcphdr(ptr_info)->hlen);
        ip.addr = ntohl(dhcp_snp_pkt_dhcphdr(ptr_info)->ciaddr.addr);
        MW_LOG_PRINTF("  dhcphdr ciaddr               = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
        ip.addr = ntohl(dhcp_snp_pkt_dhcphdr(ptr_info)->yiaddr.addr);
        MW_LOG_PRINTF("  dhcphdr yiaddr               = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
        ip.addr = ntohl(dhcp_snp_pkt_dhcphdr(ptr_info)->siaddr.addr);
        MW_LOG_PRINTF("  dhcphdr siaddr               = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
        ip.addr = ntohl(dhcp_snp_pkt_dhcphdr(ptr_info)->giaddr.addr);
        MW_LOG_PRINTF("  dhcphdr giaddr               = %s\n", (const C8_T *)inet_host_ntoa((const ip4_addr_t *)&ip));
        MW_LOG_PRINTF("  dhcphdr chaddr               = %02X:%02X:%02X:%02X:%02X:%02X\n",
                     dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[0], dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[1], dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[2],
                     dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[3], dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[4], dhcp_snp_pkt_dhcphdr(ptr_info)->chaddr[5]);
        MW_LOG_PRINTF("  dhcphdr sname                = %s\n", dhcp_snp_pkt_dhcphdr(ptr_info)->sname);
        MW_LOG_PRINTF("  dhcphdr file                 = %s\n", dhcp_snp_pkt_dhcphdr(ptr_info)->file);
        MW_LOG_PRINTF("  dhcphdr cookie               = 0x%x\n", (UI32_T)dhcp_snp_pkt_dhcphdr(ptr_info)->cookie);
        MW_LOG_PRINTF("  dhcphdr op                   = 0x%x\n", dhcp_snp_pkt_dhcphdr(ptr_info)->op);
        MW_LOG_PRINTF("  dhcphdr opt_end              = 0x%p\n", ptr_info->ptr_opt_end);
    }

    if (ptr_info->ptr_opt82)
    {
        UI8_T str[256] = {0};
        MW_LOG_PRINTF("  opt82                        = 0x%p\n", ptr_info->ptr_opt82);
        MW_LOG_PRINTF("  opt82 len                    = %d\n", ptr_info->ptr_opt82[DHCP_OPTION_LEN]);
        MW_LOG_PRINTF("  circuit_id                   = 0x%p\n", ptr_info->ptr_subopt_circuit_id);
        if (ptr_info->ptr_subopt_circuit_id)
        {
            osapi_memcpy(str, &ptr_info->ptr_subopt_circuit_id[DHCP_OPTION_DATA], ptr_info->ptr_subopt_circuit_id[DHCP_OPTION_LEN]);
            MW_LOG_PRINTF("  circuit_id len               = %d\n", ptr_info->ptr_subopt_circuit_id[DHCP_OPTION_LEN]);
            MW_LOG_PRINTF("  circuit_id data              = %s\n", str);
        }
        MW_LOG_PRINTF("  remote_id                    = 0x%p\n", ptr_info->ptr_subopt_remote_id);
        if (ptr_info->ptr_subopt_remote_id)
        {
            osapi_memset(str, 0, 255);
            osapi_memcpy(str, &ptr_info->ptr_subopt_remote_id[DHCP_OPTION_DATA], ptr_info->ptr_subopt_remote_id[DHCP_OPTION_LEN]);
            MW_LOG_PRINTF("  remote_id len                = %d\n", ptr_info->ptr_subopt_remote_id[DHCP_OPTION_LEN]);
            MW_LOG_PRINTF("  remote_id data               = %s\n", str);
        }
    }
}
#endif /* DHCP_SNP_DEBUG_DUMP */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_pkt_process
 * PURPOSE:
 *      DHCP snooping packet process function
 *
 * INPUT:
 *      ptr_msg   -  DHCP snooping packet message
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
dhcp_snp_pkt_process(
    DHCP_SNP_PKT_MSG_T *ptr_msg)
{
    UI8_T admin_enable = 0;

    dhcp_snp_pkt_profiling_init();

    dhcp_snp_db_lock();
    admin_enable = dhcp_snp_db_cfg(global_enable);
    dhcp_snp_db_unlock();

    if (admin_enable)
    {
        DHCP_SNP_PKT_INFO_T pkt_info = {0};
        UI8_T trunk_id = 0, lower_port = 0;

        if (ptr_dhcpsnp_pbuf == NULL)
        {
            if (osapi_malloc(DHCP_SNP_PKT_BUFF_SIZE, DHCP_SNP_MODULE_NAME, (void **)&ptr_dhcpsnp_pbuf) != MW_E_OK)
            {
                dhcp_snp_err("Failed to allocated memory for packet buffer");
                /* Release RX packet */
                pbuf_free(ptr_msg->ptr_pbuf);
                return MW_E_OK;
            }
        }

        if (_dhcp_snp_pkt_copyToLocal(ptr_msg->ptr_pbuf) != MW_E_OK)
        {
            /* Release RX packet */
            pbuf_free(ptr_msg->ptr_pbuf);
            osapi_free(ptr_dhcpsnp_pbuf);
            ptr_dhcpsnp_pbuf = NULL;
            return MW_E_OK;
        }
        pkt_info.ptr_pbuf     = ptr_msg->ptr_pbuf;
        pkt_info.src_ip       = ptr_msg->src_ip;
        pkt_info.dest_ip      = ptr_msg->dest_ip;
        pkt_info.ethdr_offset = ptr_msg->ethdr_offset;
        pkt_info.port_id      = ptr_msg->ptr_pbuf->stag_hdr.rx_hdr.sp;
        pkt_info.vlan_id      = ptr_msg->ptr_pbuf->vlan_hdr.vid;
        pkt_info.ptr_dhcphdr  = (struct dhcp_msg *)ptr_dhcpsnp_pbuf;
        pkt_info.dhcp_request = ptr_msg->dhcp_request;

        if (_dhcp_snp_chk_trunkMember(pkt_info.port_id, &trunk_id, &lower_port) == MW_E_OK)
        {
            pkt_info.port_id = lower_port;
            pkt_info.trunk_id = trunk_id;
        }

        if ((pkt_info.port_id < 1) || (pkt_info.port_id > PLAT_MAX_PORT_NUM))
        {
            dhcp_snp_info("Discard packet from invalid port (%d)", pkt_info.port_id);
        }
        else if (!VLAN_IS_VID_VALID(pkt_info.vlan_id))
        {
            dhcp_snp_info("Discard packet from invalid VLAN (%d)", pkt_info.vlan_id);
        }
        else
        {
            dhcp_snp_pkt_profiling_start();
#ifdef DHCP_SNP_VLAN
            dhcp_snp_dbg("Incoming from VLAN(%d)", pkt_info.vlan_id);
            /* TODO: check VLAN list */
#endif /* DHCP_SNP_VLAN */
            if (_dhcp_snp_pkt_recv(&pkt_info) == MW_E_OK)
            {
                dhcp_snp_pkt_profiling_end("_dhcp_snp_pkt_recv()");
#ifdef DHCP_SNP_BINDING_DB
                /* TODO: Check packet with binding entry */
#endif /* DHCP_SNP_BINDING_DB */
                if (pkt_info.action & DHCP_SNP_PKT_FORWARD)
                {
                    _dhcp_snp_pkt_send(&pkt_info);
                    if (pkt_info.ptr_pbuf != ptr_msg->ptr_pbuf)
                    {
                        /* Release new DHCP TX packet */
                        pbuf_free(pkt_info.ptr_pbuf);
                    }
                }
                else
                {
                    dhcp_snp_info("No forwarding DHCP packet");
                }
            }
            else
            {
                dhcp_snp_pkt_profiling_end("Invalid packet");
                dhcp_snp_info("Discard invalid packet");
            }
        }
        osapi_free(ptr_dhcpsnp_pbuf);
        ptr_dhcpsnp_pbuf = NULL;
    }
    /* Release RX packet */
    pbuf_free(ptr_msg->ptr_pbuf);
    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_pkt_filter
 * PURPOSE:
 *      DHCP snooping packet filtering function
 *
 * INPUT:
 *      ptr_pbuf   -  Packet buffer
 *      src_port   -  UDP source port
 *      dest_port  -  UDP destination port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      0  - Not to filter packet
 *      1  - filter packet
 *
 * NOTES:
 *      None
 */
int
dhcp_snp_pkt_filter(
    struct pbuf       *ptr_pbuf,
    u16_t             src_port,
    u16_t             dest_port)
{
    UI16_T  port_id = 0, vlan_id = 0;
    UI8_T   admin_enable = 0;
    UI8_T   is_trust_port = FALSE;
    UI8_T   trunk_id = 0, lower_port = 0;

    dhcp_snp_db_lock();
    admin_enable = dhcp_snp_db_cfg(global_enable);
    dhcp_snp_db_unlock();

    if (!admin_enable)
    {
        /* Filtering disable */
        return 0;
    }

    if ((src_port == DHCP_SNP_UDP_PORT_DHCP_CLIENT) && (dest_port == DHCP_SNP_UDP_PORT_DHCP_SERVER))
    {
        /* Ignore DHCP request */
        return 0;
    }

    port_id      = ptr_pbuf->stag_hdr.rx_hdr.sp;
    vlan_id      = ptr_pbuf->vlan_hdr.vid;
    if (_dhcp_snp_chk_trunkMember(port_id, &trunk_id, &lower_port) == MW_E_OK)
    {
        port_id = lower_port;
    }
    if ((port_id < 1) || (port_id > PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_info("Discard packet from invalid port (%d)", port_id);
    }
    else if (!VLAN_IS_VID_VALID(vlan_id))
    {
        dhcp_snp_info("Discard packet from invalid VLAN (%d)", vlan_id);
    }
    else
    {
        dhcp_snp_db_lock();
        is_trust_port = dhcp_snp_db_portcfg(trust_port, port_id);
        dhcp_snp_db_unlock();
    }
    /* Forward packet if receive from trust port */
    return (is_trust_port) ? 0 : 1;
}

/* FUNCTION NAME: dhcp_snp_pkt_deinit
 * PURPOSE:
 *      Deinit DHCP snooping packet function
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
dhcp_snp_pkt_deinit(
    void)
{
    if (ptr_dhcpsnp_pcb_req)
    {
        udp_remove(ptr_dhcpsnp_pcb_req);
        ptr_dhcpsnp_pcb_req = NULL;
    }
    if (ptr_dhcpsnp_pcb_ack)
    {
        udp_remove(ptr_dhcpsnp_pcb_ack);
        ptr_dhcpsnp_pcb_ack = NULL;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: dhcp_snp_pkt_init
 * PURPOSE:
 *      Init DHCP snooping packet function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
dhcp_snp_pkt_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    ptr_dhcpsnp_pcb_req = udp_new_ip_type(IPADDR_TYPE_ANY);
    ptr_dhcpsnp_pcb_ack = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (ptr_dhcpsnp_pcb_req == NULL || ptr_dhcpsnp_pcb_ack == NULL)
    {
        dhcp_snp_err("Failed to allocate pcb req=0x%p ack=0x%p", ptr_dhcpsnp_pcb_req, ptr_dhcpsnp_pcb_ack);
        dhcp_snp_pkt_deinit();
        return MW_E_NO_MEMORY;
    }

    udp_recv(ptr_dhcpsnp_pcb_req, dhcp_snp_pkt_handle, NULL);
    udp_recv(ptr_dhcpsnp_pcb_ack, dhcp_snp_pkt_handle, NULL);
    ip_set_option(ptr_dhcpsnp_pcb_req, (SOF_REUSEADDR | SOF_BROADCAST));
    ip_set_option(ptr_dhcpsnp_pcb_ack, (SOF_REUSEADDR | SOF_BROADCAST));

    ip_addr_set_any(0, &ptr_dhcpsnp_pcb_req->local_ip);
    ip_addr_set_any(0, &ptr_dhcpsnp_pcb_ack->local_ip);
    ip_addr_set_any(0, &ptr_dhcpsnp_pcb_req->remote_ip);
    ip_addr_set_any(0, &ptr_dhcpsnp_pcb_ack->remote_ip);
    ptr_dhcpsnp_pcb_req->local_port = DHCP_SNP_UDP_PORT_DHCP_CLIENT;
    ptr_dhcpsnp_pcb_ack->local_port = DHCP_SNP_UDP_PORT_DHCP_SERVER;
    ptr_dhcpsnp_pcb_req->remote_port = DHCP_SNP_UDP_PORT_DHCP_SERVER;
    ptr_dhcpsnp_pcb_ack->remote_port = DHCP_SNP_UDP_PORT_DHCP_CLIENT;

    return rc;
}

