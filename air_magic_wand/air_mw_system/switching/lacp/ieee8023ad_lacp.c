/* $NetBSD: ieee8023ad_lacp.c,v 1.3 2005/12/11 12:24:54 christos Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2005 YAMAMOTO Takashi,
 * Copyright (c)2008 Andrew Thompson <thompsa@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifdef AIR_SUPPORT_LACP
#include "ieee8023ad_lacp.h"
#include "syncd_api_lag.h"
#include "syncd_api_stp.h"
#include "air_swc.h"
#include "sys_mgmt.h"
#include "ethernet.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "sfp_port.h"
#endif

static void lacp_fill_actorinfo(struct lacp_port *, struct lacp_peerinfo *);
static void lacp_fill_markerinfo(struct lacp_port *,
            struct lacp_markerinfo *);

static uint32_t lacp_aggregator_bandwidth(struct lacp_aggregator *);
static void lacp_suppress_distributing(struct lacp_softc *,
            struct lacp_aggregator *);
static void lacp_select_active_aggregator(struct lacp_softc *);
static int tlv_check(const void *, size_t, const struct tlvhdr *,
            const struct tlv_template *, BOOL_T);
static void lacp_tick(void *);

static void lacp_fill_aggregator_id(struct lacp_aggregator *,
            const struct lacp_port *);
static void lacp_fill_aggregator_id_peer(struct lacp_peerinfo *,
            const struct lacp_peerinfo *);
static BOOL_T lacp_aggregator_is_compatible(const struct lacp_aggregator *,
            const struct lacp_port *);
static BOOL_T lacp_peerinfo_is_compatible(const struct lacp_peerinfo *,
            const struct lacp_peerinfo *);

static struct lacp_aggregator *lacp_aggregator_get(struct lacp_softc *,
            struct lacp_port *);
static void lacp_aggregator_addref(struct lacp_softc *,
            struct lacp_aggregator *);
static void lacp_aggregator_delref(struct lacp_softc *,
            struct lacp_aggregator *);

/* receive machine */

static int lacp_pdu_input(struct lacp_port *, struct pbuf *);
static int lacp_marker_input(struct lacp_port *, struct pbuf *);
static void lacp_sm_rx(struct lacp_port *, const struct lacpdu *);
static void lacp_sm_rx_timer(struct lacp_port *);
static void lacp_sm_rx_set_expired(struct lacp_port *);
static void lacp_sm_rx_update_ntt(struct lacp_port *,
            const struct lacpdu *);
static void lacp_sm_rx_record_pdu(struct lacp_port *,
            const struct lacpdu *);
static void lacp_sm_rx_update_selected(struct lacp_port *,
            const struct lacpdu *);
static void lacp_sm_rx_record_default(struct lacp_port *);
static void lacp_sm_rx_update_default_selected(struct lacp_port *);
static void lacp_sm_rx_update_selected_from_peerinfo(struct lacp_port *,
            const struct lacp_peerinfo *);

/* mux machine */

static void lacp_sm_mux(struct lacp_port *);
static void lacp_set_mux(struct lacp_port *, enum lacp_mux_state);
static void lacp_sm_mux_timer(struct lacp_port *);

/* periodic transmit machine */

static void lacp_sm_ptx_update_timeout(struct lacp_port *, uint8_t);
static void lacp_sm_ptx_tx_schedule(struct lacp_port *);
static void lacp_sm_ptx_timer(struct lacp_port *);

/* transmit machine */

static void lacp_sm_tx(struct lacp_port *);

static void lacp_run_timers(struct lacp_port *);
static int lacp_compare_peerinfo(const struct lacp_peerinfo *,
            const struct lacp_peerinfo *);
static int lacp_compare_systemid(const struct lacp_systemid *,
            const struct lacp_systemid *);
static void lacp_select(struct lacp_port *);
static void lacp_disable_collecting(struct lacp_port *);
static void lacp_enable_collecting(struct lacp_port *);
static void lacp_disable_distributing(struct lacp_port *);
static void lacp_enable_distributing(struct lacp_port *);
static int lacp_xmit_lacpdu(struct lacp_port *);
static int lacp_xmit_marker(struct lacp_port *);

/* Debugging */

static void lacp_dump_lacpdu(const struct lacpdu *);
static const char *lacp_format_partner(const struct lacp_peerinfo *, char *,
            size_t);
static const char *lacp_format_lagid(const struct lacp_peerinfo *,
            const struct lacp_peerinfo *, char *, size_t);

/*
 * actor system priority and port priority.
 * XXX should be configurable.
 */
const uint8_t ethermulticastaddr_slowprotocols[ETHER_ADDR_LEN] =
{ 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };

static const struct tlv_template lacp_info_tlv_template[] = {
    { LACP_TYPE_ACTORINFO,
        sizeof(struct tlvhdr) + sizeof(struct lacp_peerinfo) },
    { LACP_TYPE_PARTNERINFO,
        sizeof(struct tlvhdr) + sizeof(struct lacp_peerinfo) },
    { LACP_TYPE_COLLECTORINFO,
        sizeof(struct tlvhdr) + sizeof(struct lacp_collectorinfo) },
    { 0, 0 },
};

static const struct tlv_template marker_info_tlv_template[] = {
    { MARKER_TYPE_INFO,
        sizeof(struct tlvhdr) + sizeof(struct lacp_markerinfo) },
    { 0, 0 },
};

static const struct tlv_template marker_response_tlv_template[] = {
    { MARKER_TYPE_RESPONSE,
        sizeof(struct tlvhdr) + sizeof(struct lacp_markerinfo) },
    { 0, 0 },
};

typedef void (*lacp_timer_func_t)(struct lacp_port *);

lacp_info_t *ptr_g_lacp_info = NULL;

static const char *lacp_state_bit_desc[8] = {
    "ACTIVITY",
    "TIMEOUT",
    "AGGREGATION",
    "SYNC",
    "COLLECTING",
    "DISTRIBUTING",
    "DEFAULTED",
    "EXPIRED"
};

static const char *lacp_mux_state_bit_desc[5] = {
    "DETACHED",
    "WAITING",
    "ATTACHED",
    "COLLECTING",
    "DISTRIBUTING",
};

/*
 * partner administration variables.
 * XXX should be configurable.
 */
static const struct lacp_peerinfo lacp_partner_admin_optimistic = {
    .lip_systemid = {.lsi_prio = 0xffff },
    .lip_portid = {.lpi_prio = 0xffff },
    .lip_state = LACP_STATE_SYNC | LACP_STATE_AGGREGATION |
        LACP_STATE_COLLECTING | LACP_STATE_DISTRIBUTING,
};

static const struct lacp_peerinfo lacp_partner_admin_strict = {
    .lip_systemid = {.lsi_prio = 0xffff },
    .lip_portid = {.lpi_prio = 0xffff },
    .lip_state = 0,
};

static const lacp_timer_func_t lacp_timer_funcs[LACP_NTIMER] = {
    [LACP_TIMER_CURRENT_WHILE] = lacp_sm_rx_timer,
    [LACP_TIMER_PERIODIC] = lacp_sm_ptx_timer,
    [LACP_TIMER_WAIT_WHILE] = lacp_sm_mux_timer,
};

static __inline BOOL_T
lacp_isactive(struct lacp_port *lp)
{
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    struct lacp_aggregator *la = lp->lp_aggregator;

    /* This port is joined to the active aggregator */
    return (la != NULL && la == lsc->lsc_active_aggregator);
}

static __inline BOOL_T
lacp_iscollecting(struct lacp_port *lp)
{
    return (lp->lp_state & LACP_STATE_COLLECTING);
}

static __inline BOOL_T
lacp_isdistributing(struct lacp_port *lp)
{
    return (lp->lp_state & LACP_STATE_DISTRIBUTING);
}

static void
_lacp_print_port_state(
    UI8_T *ptr_lacp_state)
{
    UI8_T i;
    char *ptr_portlist;
    char buf[3] = { 0 };

    osapi_calloc(LACP_PORTLIST_MAX, LACP_MODULE, (void **)&ptr_portlist);
    for (i = 0;i < PLAT_MAX_PORT_NUM;i++)
    {
        if (PORT_LACP_STATE_IDLE == ptr_lacp_state[i])
        {
            snprintf(buf, 3, "%d,", i + 1);
            osapi_strcat(ptr_portlist, buf);
            osapi_memset(buf, 0, 3);
        }
    }
    MW_LOG_INFO(LACP, "[%d][%s]LACP IDLE portlist: %s", __LINE__, __func__, (ptr_portlist[0] == '\0') ? "None" : ptr_portlist);
    osapi_memset(ptr_portlist, 0, LACP_PORTLIST_MAX);
    for (i = 0;i < PLAT_MAX_PORT_NUM;i++)
    {
        if ((NULL != ptr_g_lacp_info->lag_port[i]) && (PORT_LACP_STATE_AGGREGATED == ptr_lacp_state[i]))
        {
            snprintf(buf, 3, "%d,", i + 1);
            osapi_strcat(ptr_portlist, buf);
            osapi_memset(buf, 0, 3);
        }
    }
    MW_LOG_INFO(LACP, "[%d][%s]LACP Aggregated portlist: %s", __LINE__, __func__, (ptr_portlist[0] == '\0') ? "None" : ptr_portlist);
    osapi_memset(ptr_portlist, 0, LACP_PORTLIST_MAX);
    for (i = 0;i < PLAT_MAX_PORT_NUM;i++)
    {
        if ((NULL != ptr_g_lacp_info->lag_port[i]) && (PORT_LACP_STATE_SELECTED == ptr_lacp_state[i]))
        {
            snprintf(buf, 3, "%d,", i + 1);
            osapi_strcat(ptr_portlist, buf);
            osapi_memset(buf, 0, 3);
        }
    }
    MW_LOG_INFO(LACP, "[%d][%s]LACP Selected portlist: %s", __LINE__, __func__, (ptr_portlist[0] == '\0') ? "None" : ptr_portlist);
    osapi_memset(ptr_portlist, 0, LACP_PORTLIST_MAX);
    for (i = 0;i < PLAT_MAX_PORT_NUM;i++)
    {
        if ((NULL != ptr_g_lacp_info->lag_port[i]) && (PORT_LACP_STATE_UNSELECTED == ptr_lacp_state[i]))
        {
            snprintf(buf, 3, "%d,", i + 1);
            osapi_strcat(ptr_portlist, buf);
            osapi_memset(buf, 0, 3);
        }
    }
    MW_LOG_INFO(LACP, "[%d][%s]LACP Unselected portlist: %s", __LINE__, __func__, (ptr_portlist[0] == '\0') ? "None" : ptr_portlist);

    MW_FREE(ptr_portlist);
}

int
lacp_input(struct lacp_port *lp, struct pbuf *p)
{
    int error = 0;

    if (lp == NULL || p == NULL)
    {
        return MW_E_BAD_PARAMETER;
    }

    uint8_t subtype = *((UI8_T *)(p->payload));

    MW_LOG_INFO(LACP, "[%d][%s]PDU subtype = %d", __LINE__, __func__, subtype);
    switch (subtype)
    {
        case SLOWPROTOCOLS_SUBTYPE_LACP:
            error = lacp_pdu_input(lp, p);
            break;

        case SLOWPROTOCOLS_SUBTYPE_MARKER:
            error = lacp_marker_input(lp, p);
            break;
    }

    /* Not a subtype we are interested in */
    return (error);
}

/*
 * lacp_pdu_input: process lacpdu
 */
static int
lacp_pdu_input(struct lacp_port *lp, struct pbuf *p)
{
    struct lacpdu du;
    int error = 0;

    osapi_memset(&du, 0, sizeof(du));

    if ((p->len + sizeof(struct eth_hdr)) != sizeof(du))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]len(%lu) != pdu len(%lu)", __LINE__, __func__, p->len + sizeof(struct eth_hdr), sizeof(du));
        goto bad;
    }

    osapi_memcpy(&du.ldu_sph, p->payload, sizeof(struct lacpdu) - sizeof(struct eth_hdr));

    if (osapi_memcmp(&p->ether_hdr.dest, &ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]DA(%02x:%02x:%02x:%02x:%02x:%02x) != slow proto multa ether addr", __LINE__, __func__,
                    p->ether_hdr.dest.addr[0],
                    p->ether_hdr.dest.addr[1],
                    p->ether_hdr.dest.addr[2],
                    p->ether_hdr.dest.addr[3],
                    p->ether_hdr.dest.addr[4],
                    p->ether_hdr.dest.addr[5]);
        goto bad;
    }

    /*
     * ignore the version for compatibility with
     * the future protocol revisions.
     */
#if 0
    if (du->ldu_sph.sph_version != 1)
    {
        goto bad;
    }
#endif

    /*
     * ignore tlv types for compatibility with
     * the future protocol revisions.
     */
    if (tlv_check(&du, sizeof(du), &du.ldu_tlv_actor,
                  lacp_info_tlv_template, FALSE))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]tlv check wrong", __LINE__, __func__);
        goto bad;
    }

    lacp_dump_lacpdu(&du);

    ptr_g_lacp_info->lacpdu_rx_count[lp->port_id - 1]++;
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] LACPDU RX done(rx count:%d)", __LINE__, __func__, lp->port_id, ptr_g_lacp_info->lacpdu_rx_count[lp->port_id - 1]);
    lacp_sm_rx(lp, &du);

    pbuf_free(p);
    return (error);

bad:
    pbuf_free(p);
    return (-1);
}

static void
lacp_fill_actorinfo(struct lacp_port *lp, struct lacp_peerinfo *info)
{
    AIR_MAC_T mac;
    UI32_T unit = 0;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]***Error***, get sys-mac error", __LINE__, __func__);
        return;
    }

    info->lip_systemid.lsi_prio = htons(ptr_g_lacp_info->lacp_system_priority);
    osapi_memcpy(&info->lip_systemid.lsi_mac, mac, ETHER_ADDR_LEN);
    info->lip_portid.lpi_prio = htons(ptr_g_lacp_info->lacp_port_priority[lp->port_id - 1]);
    info->lip_portid.lpi_portno = htons(lp->port_id);
    info->lip_state = lp->lp_state;
    MW_LOG_INFO(LACP, "[%d][%s]sys prio:%d, port prio:%d, port id:%d, state:%d", __LINE__, __func__,
             ntohs(info->lip_systemid.lsi_prio), ntohs(info->lip_portid.lpi_prio), ntohs(info->lip_portid.lpi_portno), info->lip_state);
}

static void
lacp_fill_markerinfo(struct lacp_port *lp, struct lacp_markerinfo *info)
{
    /* Fill in the port index and system id (encoded as the MAC) */
    info->mi_rq_port = htons(lp->port_id);
    osapi_memcpy(&info->mi_rq_system, lp->lp_systemid.lsi_mac, ETHER_ADDR_LEN);
    info->mi_rq_xid = htonl(0);
    MW_LOG_INFO(LACP, "[%d][%s]port[%u] mi_rq_port:%d, mi_rq_xid:%lu, mi_rq_system=%02x:%02x:%02x:%02x:%02x:%02x", __LINE__, __func__,
                lp->port_id,
                ntohs(info->mi_rq_port),
                ntohl(info->mi_rq_xid),
                info->mi_rq_system[0],
                info->mi_rq_system[1],
                info->mi_rq_system[2],
                info->mi_rq_system[3],
                info->mi_rq_system[4],
                info->mi_rq_system[5]);
}

static int
lacp_xmit_lacpdu(struct lacp_port *lp)
{
    struct pbuf *p;
    struct lacpdu *du;
    AIR_MAC_T mac;
    UI32_T unit = 0;
    err_t rc;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]***Error***, get sys-mac error", __LINE__, __func__);
        return ERR_ABRT;
    }

    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(*du) - sizeof(struct eth_hdr), PBUF_RAM);
    p->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
    p->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK;
    p->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << (lp->port_id));

    du = (struct lacpdu *)((UI8_T *)(p->payload) - sizeof(struct eth_hdr));
    osapi_memset(du, 0, sizeof(*du));
    osapi_memcpy(&du->ldu_eh.dest, ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN);
    osapi_memcpy(&p->ether_hdr.dest, ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN);
    sys_mgmt_convert_port_mac(mac, lp->port_id);
    osapi_memcpy(&du->ldu_eh.src, mac, ETHER_ADDR_LEN);
    osapi_memcpy(&p->ether_hdr.src, mac, ETHER_ADDR_LEN);
    du->ldu_eh.type = htons(ETHTYPE_LACP_SLOW);
    du->ldu_sph.sph_subtype = SLOWPROTOCOLS_SUBTYPE_LACP;
    du->ldu_sph.sph_version = 1;

    TLV_SET(&du->ldu_tlv_actor, LACP_TYPE_ACTORINFO, sizeof(du->ldu_actor));
    du->ldu_actor = lp->lp_actor;

    TLV_SET(&du->ldu_tlv_partner, LACP_TYPE_PARTNERINFO, sizeof(du->ldu_partner));
    du->ldu_partner = lp->lp_partner;

    TLV_SET(&du->ldu_tlv_collector, LACP_TYPE_COLLECTORINFO, sizeof(du->ldu_collector));
    du->ldu_collector.lci_maxdelay = 0;

    lacp_dump_lacpdu(du);
    rc = ethernet_output_use_default_netif(p, ETHTYPE_LACP_SLOW);
    if (ERR_OK == rc)
    {
        ptr_g_lacp_info->lacpdu_tx_count[lp->port_id - 1]++;
    }
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] LACPDU TX done(tx count:%d). rc:%d", __LINE__, __func__, lp->port_id, ptr_g_lacp_info->lacpdu_tx_count[lp->port_id - 1], rc);
    pbuf_free(p);
    return (rc);
}

static int
lacp_xmit_marker(struct lacp_port *lp)
{
    struct pbuf *p;
    struct markerdu *mdu;
    AIR_MAC_T mac;
    UI32_T unit = 0;
    err_t rc;

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]***Error***, get sys-mac error", __LINE__, __func__);
        return ERR_ABRT;
    }

    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(*mdu) - sizeof(struct eth_hdr), PBUF_RAM);
    p->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
    p->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK;
    p->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << (lp->port_id));

    mdu = (struct markerdu *)((UI8_T *)p->payload - sizeof(struct eth_hdr));
    osapi_memset(mdu, 0, sizeof(*mdu));
    osapi_memcpy(&mdu->mdu_eh.dest, ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN);
    osapi_memcpy(&p->ether_hdr.dest, ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN);
    sys_mgmt_convert_port_mac(mac, lp->port_id);
    osapi_memcpy(&mdu->mdu_eh.src, mac, ETHER_ADDR_LEN);
    osapi_memcpy(&p->ether_hdr.src, mac, ETHER_ADDR_LEN);
    mdu->mdu_eh.type = htons(ETHTYPE_LACP_SLOW);
    mdu->mdu_sph.sph_subtype = SLOWPROTOCOLS_SUBTYPE_MARKER;
    mdu->mdu_sph.sph_version = 1;

    /* Bump the transaction id and copy over the marker info */
    lp->lp_marker.mi_rq_xid = htonl(ntohl(lp->lp_marker.mi_rq_xid) + 1);
    TLV_SET(&mdu->mdu_tlv, MARKER_TYPE_INFO, sizeof(mdu->mdu_info));
    mdu->mdu_info = lp->lp_marker;

    MW_LOG_INFO(LACP, "[%d][%s]marker transmit, port=%d, sys=[%02X:%02X:%02X:%02X:%02X:%02X], id=%lu", __LINE__, __func__,
                ntohs(mdu->mdu_info.mi_rq_port),
                mdu->mdu_info.mi_rq_system[0],
                mdu->mdu_info.mi_rq_system[1],
                mdu->mdu_info.mi_rq_system[2],
                mdu->mdu_info.mi_rq_system[3],
                mdu->mdu_info.mi_rq_system[4],
                mdu->mdu_info.mi_rq_system[5],
                ntohl(mdu->mdu_info.mi_rq_xid));

    rc = ethernet_output_use_default_netif(p, ETHTYPE_LACP_SLOW);
    if (ERR_OK == rc)
    {
        ptr_g_lacp_info->markerpdu_tx_count[lp->port_id - 1]++;
    }
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] Marker PDU TX done(tx count:%d). rc:%d", __LINE__, __func__, lp->port_id, ptr_g_lacp_info->markerpdu_tx_count[lp->port_id - 1], rc);
    pbuf_free(p);
    return (rc);
}

void
lacp_linkstate(struct lacp_port *lp)
{
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    AIR_PORT_STATUS_T p_state;
    UI32_T unit = 0;

    MW_LOG_INFO(LACP, "[%d][%s]Port[%d] check link state", __LINE__, __func__, lp->port_id);
    /*
     * If the port is not an active full duplex Ethernet link then it can
     * not be aggregated.
     */
#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, lp->port_id)) ||
        (TRUE == sfp_port_is_comboPort(unit, lp->port_id)))
    {
        lp->lp_media = LACP_SFP_PORT;
        air_rc = sfp_port_getPortStatus(unit, lp->port_id, &p_state);
    }
    else
#endif
    {
        air_rc = air_port_getPortStatus(unit, lp->port_id, &p_state);
    }
    if (AIR_E_OK != air_rc)
    {
        MW_LOG_ERROR(LACP, "[%d][%s]Port[%d] Get link status failed", __LINE__, __func__, lp->port_id);
        lacp_port_disable(lp);
        return;
    }

    MW_LOG_INFO(LACP, "[%d][%s]Port[%d] link state: flag=0x%x, duplex=%d, speed=%d", __LINE__, __func__, lp->port_id, p_state.flags, p_state.duplex, p_state.speed);
    if ((p_state.duplex == AIR_PORT_DUPLEX_HALF) ||
        !(p_state.flags & AIR_PORT_STATUS_FLAGS_LINK_UP))
    {
        lacp_port_disable(lp);
    }
    else
    {
        lacp_port_enable(lp);
    }
    /* lp->lp_key = lacp_compose_key(lp); */
    lp->lp_key = lacp_mw_compose_key(lp);
}

static void
lacp_tick(void *arg)
{
    struct lacp_softc *lsc = arg;
    struct lacp_port *lp;
    UI8_T i;

    if (NULL == lsc)
    {
        return;
    }

    for (i = 0; i < lsc->member_cnt; i++)
    {
        lp = ptr_g_lacp_info->lag_port[lsc->port_prio_descending[i] - 1];
        if (lp == NULL)
        {
            continue;
        }
        if ((lp->lp_state & LACP_STATE_AGGREGATION) == 0)
            continue;

        lacp_run_timers(lp);

        lacp_select(lp);
        lacp_sm_mux(lp);
        lacp_sm_tx(lp);
        lacp_sm_ptx_tx_schedule(lp);
    }
}

int
lacp_port_create(struct lacp_port **lp, UI8_T group, UI16_T port)
{
    struct lacp_port *lp_tmp;

    osapi_calloc(sizeof(struct lacp_port), LACP_MODULE, (void **)&lp_tmp);
    if (lp_tmp == NULL)
        return (-2);
    (*lp) = lp_tmp;
    (*lp)->port_id = port;
    (*lp)->group_id = group;

    lacp_fill_actorinfo((*lp), &(*lp)->lp_actor);
    lacp_fill_markerinfo((*lp), &(*lp)->lp_marker);
    (*lp)->lp_state = LACP_STATE_ACTIVITY;
    (*lp)->lp_aggregator = NULL;
    (*lp)->lp_selected = LACP_UNSELECTED;
    lacp_linkstate((*lp));

    return (0);
}

void
lacp_port_destroy(struct lacp_port *lp)
{
    int i;

    for (i = 0; i < LACP_NTIMER; i++)
    {
        LACP_TIMER_DISARM(lp, i);
    }

    lacp_set_mux(lp, LACP_MUX_DETACHED);
    lacp_xmit_lacpdu(lp);

    MW_FREE(lp);
}

static void
lacp_disable_collecting(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]Port[%d] collecting disabled", __LINE__, __func__, lp->port_id);
    lp->lp_state &= ~LACP_STATE_COLLECTING;
}

static void
lacp_enable_collecting(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]Port[%d] collecting enabled", __LINE__, __func__, lp->port_id);
    lp->lp_state |= LACP_STATE_COLLECTING;
}

static void
lacp_disable_distributing(struct lacp_port *lp)
{
    struct lacp_aggregator *la = lp->lp_aggregator;
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    char *ptr_buf;

    if (la == NULL || (lp->lp_state & LACP_STATE_DISTRIBUTING) == 0)
    {
        return;
    }

    KASSERT(!TAILQ_EMPTY(&la->la_ports), "no aggregator ports");
    if (la->la_nports > 0)
    {
        MW_LOG_WARN(LACP, "[%d][%s]nports invalid (%d)", __LINE__, __func__, la->la_nports);
    }

    KASSERT(la->la_refcnt >= la->la_nports, "aggregator refcnt invalid");

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]disable distributing on aggregator %s, nports %d -> %d", __LINE__, __func__,
                lacp_format_lagid_aggregator(la, ptr_buf, LACP_LAGIDSTR_MAX + 1),
                la->la_nports, la->la_nports - 1);

    TAILQ_REMOVE(&la->la_ports, lp, lp_dist_q);
    la->la_nports--;

    if (lsc->lsc_active_aggregator == la)
    {
        lacp_suppress_distributing(lsc, la);
        lacp_select_active_aggregator(lsc);
        /* regenerate the port map, the active aggregator has changed */
    }

    lp->lp_state &= ~LACP_STATE_DISTRIBUTING;
    MW_FREE(ptr_buf);
}

static void
lacp_enable_distributing(struct lacp_port *lp)
{
    struct lacp_aggregator *la = lp->lp_aggregator;
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    char *ptr_buf;

    MW_LOG_INFO(LACP, "[%d][%s]port[%d] state:0x%x(%s distributing)", __LINE__, __func__, lp->port_id, lp->lp_state, (lp->lp_state & LACP_STATE_DISTRIBUTING) ? "is" : "not");

    if ((lp->lp_state & LACP_STATE_DISTRIBUTING) != 0)
    {
        return;
    }

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]enable distributing on aggregator %s, nports %d -> %d, refcnt %d", __LINE__, __func__,
                lacp_format_lagid_aggregator(la, ptr_buf, LACP_LAGIDSTR_MAX + 1),
                la->la_nports, la->la_nports + 1, la->la_refcnt);

    KASSERT(la->la_refcnt > la->la_nports, "aggregator refcnt invalid");
    TAILQ_INSERT_HEAD(&la->la_ports, lp, lp_dist_q);
    la->la_nports++;

    lp->lp_state |= LACP_STATE_DISTRIBUTING;

    if (lsc->lsc_active_aggregator == la)
    {
        lacp_suppress_distributing(lsc, la);
    }
    else
    {
    /* try to become the active aggregator */
        lacp_select_active_aggregator(lsc);
    }
    MW_FREE(ptr_buf);
}

void
lacp_transit_expire(UI8_T group)
{
    MW_LOG_INFO(LACP, "[%d][%s]disable group[%d] suppress distributing", __LINE__, __func__, group);
    if (NULL != ptr_g_lacp_info->lag_group[group - 1])
    {
        if (ptr_g_lacp_info->lag_group[group - 1]->lsc_suppress_distributing)
        {
            ptr_g_lacp_info->lag_group[group - 1]->lsc_suppress_distributing = FALSE;
            lacp_update_port_state();
        }
    }
    else
    {
        MW_LOG_INFO(LACP, "[%d][%s]group[%d] is not already exist", __LINE__, __func__, group);
        lacp_update_port_state();
    }
}

static void
lacp_suppress_distributing(struct lacp_softc *lsc, struct lacp_aggregator *la)
{
    struct lacp_port *lp;
    MW_ERROR_NO_T rc = MW_E_OK;

    if (lsc->lsc_active_aggregator != la)
    {
        return;
    }

    MW_LOG_INFO(LACP, "[%d][%s]enable group[%d] suppress distributing", __LINE__, __func__, lsc->group_id);

    lsc->lsc_suppress_distributing = TRUE;

    TAILQ_FOREACH(lp, &lsc->lsc_active_aggregator->la_ports, lp_dist_q)
    {
        if (lp == NULL)
        {
            continue;
        }
        lp->lp_flags |= LACP_PORT_MARK;

        if (lacp_xmit_marker(lp) != 0)
        {
            lp->lp_flags &= ~LACP_PORT_MARK;
        }
    }

    lacp_update_port_state();

    if (NULL == ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1])
    {
        rc = osapi_timerCreate(LACP_TIMER, lacp_timer_suppress_distributing, FALSE, LACP_TRANSIT_DELAY, (void *)((uintptr_t)lsc->group_id), &ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1]);
        if ((MW_E_OK != rc) || (NULL == ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1]))
        {
            MW_LOG_ERROR(LACP, "[%d][%s]Create LACP group[%d] suppress distributing timer fail! rc:%d", __LINE__, __func__, lsc->group_id, rc);
        }
        else
        {
            MW_LOG_INFO(LACP, "[%d][%s]Create LACP group[%d] suppress distributing timer success!", __LINE__, __func__, lsc->group_id);
            rc = osapi_timerStart(ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1]);
            if (MW_E_OK != rc)
            {
                MW_LOG_ERROR(LACP, "[%d][%s]Start LACP group[%d] suppress distributing timer fail! rc:%d", __LINE__, __func__, lsc->group_id, rc);
                osapi_timerDelete(ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1]);
            }
            else
            {
                MW_LOG_INFO(LACP, "[%d][%s]Start LACP group[%d] suppress distributing timer success!", __LINE__, __func__, lsc->group_id);
            }
        }
    }
    else
    {
        if (pdFAIL == xTimerReset(ptr_g_lacp_info->lacp_sd_timer_handle[lsc->group_id - 1], 0))
        {
            MW_LOG_ERROR(LACP, "[%d][%s]Restart LACP group[%d] suppress distributing timer fail!", __LINE__, __func__, lsc->group_id);
        }
        else
        {
            MW_LOG_INFO(LACP, "[%d][%s]Restart LACP group[%d] suppress distributing timer success!", __LINE__, __func__, lsc->group_id);
        }
    }

    MW_LOG_INFO(LACP, "[%d][%s]group[%d] marker timer start! rc:%d", __LINE__, __func__, lsc->group_id, rc);
}

static int
lacp_compare_peerinfo(const struct lacp_peerinfo *a,
    const struct lacp_peerinfo *b)
{
    return (osapi_memcmp(a, b, offsetof(struct lacp_peerinfo, lip_state)));
}

static int
lacp_compare_systemid(const struct lacp_systemid *a,
    const struct lacp_systemid *b)
{
    return (osapi_memcmp(a, b, sizeof(*a)));
}

static uint32_t
lacp_aggregator_bandwidth(struct lacp_aggregator *la)
{
    struct lacp_port *lp;
    UI32_T unit = 0;
    UI32_T speed_tot = 0;
    AIR_ERROR_NO_T air_rc;
    AIR_PORT_STATUS_T port_status;

    UI32_T speed_m[AIR_PORT_SPEED_LAST] = { 10, 100, 1000, 2500, 5000, 10000 };

    lp = TAILQ_FIRST(&la->la_ports);
    if (lp == NULL)
    {
        return (0);
    }

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, lp->port_id)) ||
        (TRUE == sfp_port_is_comboPort(unit, lp->port_id)))
    {
        air_rc = sfp_port_getPortStatus(unit, lp->port_id, &port_status);
    }
    else
#endif
    {
        air_rc = air_port_getPortStatus(unit, lp->port_id, &port_status);
    }
    if (air_rc != AIR_E_OK)
    {
        return speed_tot;
    }
    speed_tot = speed_m[port_status.speed] * (la->la_nports);

    MW_LOG_INFO(LACP, "[%d][%s]Total speed:%d, media=%s, nports=%d", __LINE__, __func__,
        speed_tot, (lp->lp_media) ? "SFP" : "Normal", la->la_nports);

    return (speed_tot);
}

/*
 * lacp_select_active_aggregator: select an aggregator to be used to transmit
 * packets from lagg(4) interface.
 */

static void
lacp_select_active_aggregator(struct lacp_softc *lsc)
{
    struct lacp_aggregator *la;
    struct lacp_aggregator *best_la = NULL;
    char *ptr_buf;

    MW_LOG_INFO(LACP, "[%d][%s]Group[%d] select active aggregator!", __LINE__, __func__, lsc->group_id);

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);

    TAILQ_FOREACH(la, &lsc->lsc_aggregators, la_q)
    {
        uint32_t speed;

        if (la->la_nports == 0)
        {
            continue;
        }

        speed = lacp_aggregator_bandwidth(la);
        MW_LOG_INFO(LACP, "[%d][%s]%s, speed=%lu, nports=%d", __LINE__, __func__,
            lacp_format_lagid_aggregator(la, ptr_buf, LACP_LAGIDSTR_MAX + 1),
            speed, la->la_nports);

        /*
         * This aggregator is chosen if the partner has a better
         * system priority or, the total aggregated speed is higher
         * or, it is already the chosen aggregator
         */
        if (((best_la != NULL) && (LACP_SYS_PRI(la->la_partner) < LACP_SYS_PRI(best_la->la_partner))) ||
            (NULL == best_la))
        {
            best_la = la;
            /* TODO: seletc aggregator with priority? */
        }
    }

    KASSERT((best_la == NULL) || (best_la->la_nports > 0), "invalid aggregator refcnt");
    KASSERT((best_la == NULL) || (!TAILQ_EMPTY(&best_la->la_ports)), "invalid aggregator list");

    if (lsc->lsc_active_aggregator != best_la)
    {
        MW_LOG_INFO(LACP, "[%d][%s]active aggregator changed", __LINE__, __func__);
    }
    else
    {
        MW_LOG_INFO(LACP, "[%d][%s]active aggregator not changed", __LINE__, __func__);
    }
    MW_LOG_INFO(LACP, "[%d][%s]old %s", __LINE__, __func__, lacp_format_lagid_aggregator(lsc->lsc_active_aggregator, ptr_buf, LACP_LAGIDSTR_MAX + 1));
    MW_LOG_INFO(LACP, "[%d][%s]new %s", __LINE__, __func__, lacp_format_lagid_aggregator(best_la, ptr_buf, LACP_LAGIDSTR_MAX + 1));

    if (lsc->lsc_active_aggregator != best_la)
    {
        lsc->lsc_active_aggregator = best_la;
        if (best_la)
        {
            lacp_suppress_distributing(lsc, best_la);
            lacp_update_port_state();
        }
    }

    MW_FREE(ptr_buf);
}

/*
 * Updated the inactive portmap array with the new list of ports and
 * make it live.
 */
void
lacp_update_port_state(void)
{
    struct lacp_aggregator *la;
    struct lacp_port *lp;
    int i;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL;
    UI16_T payload_size = PLAT_MAX_PORT_NUM;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_msg = NULL;
    UI8_T *ptr_lacp_state = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_msg)
    {
        MW_LOG_ERROR(LACP, "[%d][%s]Failed to create DB message", __LINE__, __func__);
        return;
    }

    osapi_calloc(PLAT_MAX_PORT_NUM, LACP_MODULE, (void **)&ptr_lacp_state);
    osapi_memset(ptr_lacp_state, PORT_LACP_STATE_UNSELECTED, PLAT_MAX_PORT_NUM);

    for (i = 0;i < MAX_TRUNK_NUM;i++)
    {
        if (ptr_g_lacp_info->lag_group[i] == NULL)
        {
            continue;
        }
        la = ptr_g_lacp_info->lag_group[i]->lsc_active_aggregator;
        if ((la != NULL) && (la->la_nports > 0) && (!ptr_g_lacp_info->lag_group[i]->lsc_suppress_distributing))
        {
            TAILQ_FOREACH(lp, &la->la_ports, lp_dist_q)
            {
                if (LACP_MUX_DISTRIBUTING == lp->lp_mux_state)
                {
                    ptr_lacp_state[lp->port_id - 1] = PORT_LACP_STATE_AGGREGATED;
                }
            }
        }
    }
    for (i = 0;i < PLAT_MAX_PORT_NUM;i++)
    {
        if (ptr_g_lacp_info->lag_port[i] == NULL)
        {
            ptr_lacp_state[i] = PORT_LACP_STATE_IDLE;
            continue;
        }
        if (ptr_lacp_state[i] == PORT_LACP_STATE_AGGREGATED)
        {
            continue;
        }
        if (ptr_g_lacp_info->lag_port[i]->lp_selected == LACP_SELECTED)
        {
            ptr_lacp_state[i] = PORT_LACP_STATE_SELECTED;
        }
        else if (ptr_g_lacp_info->lag_port[i]->lp_selected == LACP_UNSELECTED)
        {
            ptr_lacp_state[i] = PORT_LACP_STATE_UNSELECTED;
        }
    }

    _lacp_print_port_state(ptr_lacp_state);

    request.t_idx = PORT_OPER_INFO;
    request.f_idx = PORT_LACP_STATE;
    request.e_idx = DB_ALL_ENTRIES;
    rc = dbapi_appendMsgPayload(&request, ptr_lacp_state, &ptr_msg, &msg_size, &ptr_payload);
    if (MW_E_OK == rc)
    {
        rc = dbapi_sendMsg(ptr_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
        if (MW_E_OK != rc)
        {
            MW_LOG_ERROR(LACP, "[%d][%s]Failed to send message, rc:%d", __LINE__, __func__, rc);
        }
    }
    else
    {
        MW_FREE(ptr_msg);
        MW_LOG_ERROR(LACP, "[%d][%s]Failed to append message payload, rc:%d", __LINE__, __func__, rc);
    }
    MW_LOG_INFO(LACP, "[%d][%s]Update all ports lag state to DB. rc:%d", __LINE__, __func__, rc);
    MW_FREE(ptr_lacp_state);
}

UI16_T
lacp_mw_compose_key(struct lacp_port *lp)
{
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    UI32_T unit = 0;
    AIR_PORT_STATUS_T port_status;
    UI8_T fc = 0;
    UI16_T key;

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(unit, lp->port_id)) ||
        (TRUE == sfp_port_is_comboPort(unit, lp->port_id)))
    {
        air_rc = sfp_port_getPortStatus(unit, lp->port_id, &port_status);
    }
    else
#endif
    {
        air_rc = air_port_getPortStatus(unit, lp->port_id, &port_status);
    }
    if (AIR_E_OK != air_rc)
    {
        MW_LOG_ERROR(LACP, "[%d][%s]port[%d] get info(generate key) failed", __LINE__, __func__, lp->port_id);
        return 0;
    }

    fc = (((port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX) ? 1 : 0) << 1) + ((port_status.flags & AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX) ? 1 : 0);

    key = (((lp->group_id - 1) & 0x3) << 14) | (((fc) & 0x3) << 12) | (((port_status.speed) & 0xf) << 8);

#ifdef AIR_SUPPORT_SFP
    key |= (((lp->lp_media) & 0x1) << 7);
#endif

    MW_LOG_INFO(LACP, "[%d][%s]port[%d] generate key=0x%x: group id(bit14~15)=%d, fc(bit12~13)=0x%x, speed(bit8~11)=%d"
#ifdef AIR_SUPPORT_SFP
            ", media(bit7)=%d"
#endif
            , __LINE__, __func__, lp->port_id, key, (lp->group_id - 1) & 0x3, fc & 0x3, port_status.speed & 0xf
#ifdef AIR_SUPPORT_SFP
            , (lp->lp_media) & 0x1
#endif
    );
    return key;
}

static void
lacp_aggregator_addref(struct lacp_softc *lsc, struct lacp_aggregator *la)
{
    char *ptr_buf;

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]lagid=%s, refcnt %d -> %d, nports %d", __LINE__, __func__,
        lacp_format_lagid(&la->la_actor, &la->la_partner,
                          ptr_buf, LACP_LAGIDSTR_MAX + 1),
        la->la_refcnt, la->la_refcnt + 1, la->la_nports);

    KASSERT(la->la_refcnt > 0, "refcount <= 0");
    la->la_refcnt++;
    KASSERT(la->la_refcnt > la->la_nports, "invalid refcount");

    MW_FREE(ptr_buf);
}

static void
lacp_aggregator_delref(struct lacp_softc *lsc, struct lacp_aggregator *la)
{
    char *ptr_buf;

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]lagid=%s, refcnt %d -> %d, nports %d", __LINE__, __func__,
        lacp_format_lagid(&la->la_actor, &la->la_partner,
                          ptr_buf, LACP_LAGIDSTR_MAX + 1),
        la->la_refcnt, la->la_refcnt - 1, la->la_nports);

    KASSERT(la->la_refcnt > la->la_nports, "invalid refcnt");
    la->la_refcnt--;
    if (la->la_refcnt > 0)
    {
        MW_FREE(ptr_buf);
        return;
    }

    KASSERT(la->la_refcnt == 0, "refcount not zero");
    KASSERT(lsc->lsc_active_aggregator != la, "aggregator active");

    TAILQ_REMOVE(&lsc->lsc_aggregators, la, la_q);

    MW_LOG_INFO(LACP, "[%d][%s]Delete aggregator:0x%lx(%s)!", __LINE__, __func__, (uintptr_t)la, lacp_format_lagid(&la->la_actor, &la->la_partner, ptr_buf, LACP_LAGIDSTR_MAX + 1));
    MW_FREE(la);
    MW_FREE(ptr_buf);
}

/*
 * lacp_aggregator_get: allocate an aggregator.
 */

static struct lacp_aggregator *
lacp_aggregator_get(struct lacp_softc *lsc, struct lacp_port *lp)
{
    struct lacp_aggregator *la;

    osapi_calloc(sizeof(*la), LACP_MODULE, (void **)&la);
    if (la)
    {
        la->la_refcnt = 1;
        la->la_nports = 0;
        TAILQ_INIT(&la->la_ports);
        la->la_pending = 0;
        TAILQ_INSERT_TAIL(&lsc->lsc_aggregators, la, la_q);
    }

    MW_LOG_INFO(LACP, "[%d][%s]create aggregator:0x%lx for port[%d]", __LINE__, __func__, (uintptr_t)la, lp->port_id);
    return (la);
}

/*
 * lacp_fill_aggregator_id: setup a newly allocated aggregator from a port.
 */

static void
lacp_fill_aggregator_id(struct lacp_aggregator *la, const struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]fill la:0x%lx partner", __LINE__, __func__, (uintptr_t)la);
    lacp_fill_aggregator_id_peer(&la->la_partner, &lp->lp_partner);
    MW_LOG_INFO(LACP, "[%d][%s]fill la:0x%lx actor", __LINE__, __func__, (uintptr_t)la);
    lacp_fill_aggregator_id_peer(&la->la_actor, &lp->lp_actor);

    la->la_actor.lip_state = lp->lp_state & LACP_STATE_AGGREGATION;
}

static void
lacp_fill_aggregator_id_peer(struct lacp_peerinfo *lpi_aggr,
    const struct lacp_peerinfo *lpi_port)
{
    osapi_memset(lpi_aggr, 0, sizeof(*lpi_aggr));
    lpi_aggr->lip_systemid = lpi_port->lip_systemid;
    lpi_aggr->lip_key = lpi_port->lip_key;
    MW_LOG_INFO(LACP, "[%d][%s]fill aggregator id [sys priority:%d, sys mac:[%02X:%02X:%02X:%02X:%02X:%02X], key:0x%x]", __LINE__, __func__,
            ntohs(lpi_aggr->lip_systemid.lsi_prio),
            lpi_aggr->lip_systemid.lsi_mac[0],
            lpi_aggr->lip_systemid.lsi_mac[1],
            lpi_aggr->lip_systemid.lsi_mac[2],
            lpi_aggr->lip_systemid.lsi_mac[3],
            lpi_aggr->lip_systemid.lsi_mac[4],
            lpi_aggr->lip_systemid.lsi_mac[5],
            lpi_aggr->lip_key);
}

/*
 * lacp_aggregator_is_compatible: check if a port can join to an aggregator.
 */

static BOOL_T
lacp_aggregator_is_compatible(const struct lacp_aggregator *la,
    const struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]compare la[0x%lx] and port[%d](state:0x%x, partner state:0x%x), la actor state:0x%x", __LINE__, __func__,
                (uintptr_t)la, lp->port_id, lp->lp_state, lp->lp_partner.lip_state, la->la_actor.lip_state);
    if (!(lp->lp_state & LACP_STATE_AGGREGATION) ||
        !(lp->lp_partner.lip_state & LACP_STATE_AGGREGATION))
    {
        MW_LOG_INFO(LACP, "[%d][%s]port[%d] actor or partner is not in aggregation state", __LINE__, __func__, lp->port_id);
        return (FALSE);
    }

    if (!(la->la_actor.lip_state & LACP_STATE_AGGREGATION))
    {
        MW_LOG_INFO(LACP, "[%d][%s]aggregator is not in aggregation state", __LINE__, __func__);
        return (FALSE);
    }

    if (FALSE == lacp_peerinfo_is_compatible(&la->la_partner, &lp->lp_partner))
    {
        MW_LOG_INFO(LACP, "[%d][%s]la partner is not compatible with port partner", __LINE__, __func__);
        return (FALSE);
    }

    if (FALSE == lacp_peerinfo_is_compatible(&la->la_actor, &lp->lp_actor))
    {
        MW_LOG_INFO(LACP, "[%d][%s]la actor is not compatible with port actor", __LINE__, __func__);
        return (FALSE);
    }

    MW_LOG_INFO(LACP, "[%d][%s]la[0x%lx] and port[%d] is compatible", __LINE__, __func__, (uintptr_t)la, lp->port_id);
    return (TRUE);
}

static BOOL_T
lacp_peerinfo_is_compatible(const struct lacp_peerinfo *a,
    const struct lacp_peerinfo *b)
{
    if (osapi_memcmp(&a->lip_systemid, &b->lip_systemid,
                     sizeof(a->lip_systemid)) != 0)
    {
        return (FALSE);
    }

    if (osapi_memcmp(&a->lip_key, &b->lip_key, sizeof(a->lip_key)) != 0)
        return (FALSE);

    return (TRUE);
}

void
lacp_port_enable(struct lacp_port *lp)
{
    lp->lp_state |= LACP_STATE_AGGREGATION;
    lacp_sm_rx_set_expired(lp);
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] enable and set aggregatable state", __LINE__, __func__, lp->port_id);
}

void
lacp_port_disable(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] disable and set unaggregatable state", __LINE__, __func__, lp->port_id);
    lp->lp_selected = LACP_UNSELECTED;
    lacp_set_mux(lp, LACP_MUX_DETACHED);

    lp->lp_state &= ~LACP_STATE_AGGREGATION;
    lacp_sm_rx_record_default(lp);
    lp->lp_partner.lip_state &= ~LACP_STATE_AGGREGATION;
    lp->lp_state &= ~LACP_STATE_EXPIRED;
}

/*
 * lacp_select: select an aggregator.  create one if necessary.
 */
static void
lacp_select(struct lacp_port *lp)
{
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    struct lacp_aggregator *la;
    struct lacp_port *lp_tmp;
    char *ptr_buf;
    UI8_T i;
    BOOL_T selected = FALSE;

    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Enter*************", __LINE__, __func__, lp->port_id);

    if (lp->lp_aggregator)
    {
        MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Exit*************", __LINE__, __func__, lp->port_id);
        return;
    }

    /* If we haven't heard from our peer, skip this step. */
    if ((0 != (lp->lp_state & LACP_STATE_DEFAULTED)) || (0 != (lp->lp_state & LACP_STATE_EXPIRED)))
    {
        return;
    }

    KASSERT(!LACP_TIMER_ISARMED(lp, LACP_TIMER_WAIT_WHILE), "timer_wait_while still active");

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] lagid=%s", __LINE__, __func__, lp->port_id,
        lacp_format_lagid(&lp->lp_actor, &lp->lp_partner, ptr_buf, LACP_LAGIDSTR_MAX + 1));

    la = TAILQ_FIRST(&lsc->lsc_aggregators);

    if (la == NULL)
    {
        la = lacp_aggregator_get(lsc, lp);
        if (la == NULL)
        {
            MW_LOG_ERROR(LACP, "[%d][%s]aggregator creation failed", __LINE__, __func__);
            MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Exit*************", __LINE__, __func__, lp->port_id);
            MW_FREE(ptr_buf);
            return;
        }
        lacp_fill_aggregator_id(la, lp);
        if (lacp_aggregator_is_compatible(la, lp))
        {
            selected = TRUE;
        }
        MW_LOG_INFO(LACP, "[%d][%s]aggregator[0x%p] created", __LINE__, __func__, la);
    }
    else
    {
        if (lacp_aggregator_is_compatible(la, lp))
        {
            MW_LOG_INFO(LACP, "[%d][%s]compatible aggregator[0x%p] found", __LINE__, __func__, la);
            if (la->la_refcnt == ptr_g_lacp_info->lacp_max_port_num)
            {
                MW_LOG_INFO(LACP, "[%d][%s]la_refcnt is already max port number(%d) ", __LINE__, __func__, ptr_g_lacp_info->lacp_max_port_num);
                MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Exit*************", __LINE__, __func__, lp->port_id);
                MW_FREE(ptr_buf);
                return;
            }
            lacp_aggregator_addref(lsc, la);
            selected = TRUE;
            MW_LOG_INFO(LACP, "[%d][%s]select aggregator[0x%p] lagid=%s", __LINE__, __func__, la, lacp_format_lagid(&la->la_actor, &la->la_partner, ptr_buf, LACP_LAGIDSTR_MAX + 1));
        }
        else if (LACP_SYS_PRI(la->la_partner) < LACP_SYS_PRI(lp->lp_partner))
        {
            MW_LOG_INFO(LACP, "[%d][%s]port[%d] partner system priority(%d) is higher than current group[%d] aggregator(%s) system priority(%d)!", __LINE__, __func__,
                lp->port_id, ntohs(LACP_SYS_PRI(lp->lp_partner)), lsc->group_id, lacp_format_lagid(&la->la_actor, &la->la_partner, ptr_buf, LACP_LAGIDSTR_MAX + 1), ntohs(LACP_SYS_PRI(la->la_partner)));
            for (i = 0;i < lsc->member_cnt;i++)
            {
                lp_tmp = ptr_g_lacp_info->lag_port[lsc->port_prio_descending[i] - 1];
                if (lp_tmp == lp)
                {
                    continue;
                }
                if (lp_tmp->lp_aggregator == la)
                {
                    lacp_unselect(lp_tmp);
                }
            }
            la = TAILQ_FIRST(&lsc->lsc_aggregators);
            if (la == NULL)
            {
                la = lacp_aggregator_get(lsc, lp);
                if (la == NULL)
                {
                    MW_LOG_ERROR(LACP, "[%d][%s]aggregator creation failed", __LINE__, __func__);
                    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Exit*************", __LINE__, __func__, lp->port_id);
                    MW_FREE(ptr_buf);
                    return;
                }
                lacp_fill_aggregator_id(la, lp);
                if (lacp_aggregator_is_compatible(la, lp))
                {
                    selected = TRUE;
                }
                MW_LOG_INFO(LACP, "[%d][%s]aggregator[0x%p] created", __LINE__, __func__, la);
            }
            else
            {
                MW_LOG_ERROR(LACP, "[%d][%s]aggregator(%s) is still exist!", __LINE__, __func__, lacp_format_lagid(&la->la_actor, &la->la_partner, ptr_buf, LACP_LAGIDSTR_MAX + 1));
            }
        }
    }

    if ((TRUE == selected) && (NULL != la))
    {
        lp->lp_aggregator = la;
        lp->lp_selected = LACP_SELECTED;
        lacp_update_port_state();
    }

    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] Selection Logic SM Exit*************", __LINE__, __func__, lp->port_id);
    MW_FREE(ptr_buf);
}

/*
 * lacp_unselect: finish unselect/detach process.
 */
void
lacp_unselect(struct lacp_port *lp)
{
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    struct lacp_aggregator *la = lp->lp_aggregator;

    MW_LOG_INFO(LACP, "[%d][%s]Unselecting port[%d]", __LINE__, __func__, lp->port_id);

    KASSERT(!LACP_TIMER_ISARMED(lp, LACP_TIMER_WAIT_WHILE), "timer_wait_while still active");

    if (la == NULL)
    {
        MW_LOG_INFO(LACP, "[%d][%s]la is NULL", __LINE__, __func__);
        return;
    }

    lp->lp_aggregator = NULL;
    lacp_aggregator_delref(lsc, la);
}

/* mux machine */

static void
lacp_sm_mux(struct lacp_port *lp)
{
    enum lacp_mux_state new_state;
    BOOL_T p_sync =
        (lp->lp_partner.lip_state & LACP_STATE_SYNC) != 0;
    BOOL_T p_collecting =
        (lp->lp_partner.lip_state & LACP_STATE_COLLECTING) != 0;
    enum lacp_selected selected = lp->lp_selected;
    struct lacp_aggregator *la;
    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] MUX SM Enter*************", __LINE__, __func__, lp->port_id);
    MW_LOG_DEBUG(LACP, "[%d][%s]port[%d] mux_state= %s, selected= 0x%x, "
        "p_sync= 0x%x, p_collecting= 0x%x", __LINE__, __func__, lp->port_id,
        lacp_format_mux_state(lp->lp_mux_state), selected, p_sync, p_collecting);

re_eval:
    la = lp->lp_aggregator;
    KASSERT((lp->lp_mux_state == LACP_MUX_DETACHED) || (la != NULL), "MUX not detached");
    new_state = lp->lp_mux_state;
    switch (lp->lp_mux_state)
    {
        case LACP_MUX_DETACHED:
            if (selected != LACP_UNSELECTED)
            {
                new_state = LACP_MUX_WAITING;
            }
            break;
        case LACP_MUX_WAITING:
            KASSERT((la->la_pending > 0) || (!LACP_TIMER_ISARMED(lp, LACP_TIMER_WAIT_WHILE)), "timer_wait_while still active");
            if (selected == LACP_SELECTED && la->la_pending == 0)
            {
                new_state = LACP_MUX_ATTACHED;
            }
            else if (selected == LACP_UNSELECTED)
            {
                new_state = LACP_MUX_DETACHED;
            }
            break;
        case LACP_MUX_ATTACHED:
            if (selected == LACP_SELECTED && p_sync)
            {
                new_state = LACP_MUX_COLLECTING;
            }
            else if (selected != LACP_SELECTED)
            {
                new_state = LACP_MUX_DETACHED;
            }
            break;
        case LACP_MUX_COLLECTING:
            if (selected == LACP_SELECTED && p_sync && p_collecting)
            {
                new_state = LACP_MUX_DISTRIBUTING;
            }
            else if (selected != LACP_SELECTED || !p_sync)
            {
                new_state = LACP_MUX_ATTACHED;
            }
            break;
        case LACP_MUX_DISTRIBUTING:
            if (selected != LACP_SELECTED || !p_sync || !p_collecting)
            {
                new_state = LACP_MUX_COLLECTING;
                MW_LOG_INFO(LACP, "[%d][%s]Interface stopped DISTRIBUTING, possible flapping", __LINE__, __func__);
            }
            break;
        default:
            MW_LOG_ERROR(LACP, "[%d][%s]unknown state", __LINE__, __func__);
    }

    if (lp->lp_mux_state == new_state)
    {
        MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] MUX SM Exit*************", __LINE__, __func__, lp->port_id);
        return;
    }

    lacp_set_mux(lp, new_state);
    goto re_eval;
}

static void
lacp_set_mux(struct lacp_port *lp, enum lacp_mux_state new_state)
{
    struct lacp_aggregator *la = lp->lp_aggregator;

    if (lp->lp_mux_state == new_state)
    {
        return;
    }

    switch (new_state)
    {
        case LACP_MUX_DETACHED:
            lp->lp_state &= ~LACP_STATE_SYNC;
            lacp_disable_distributing(lp);
            lacp_disable_collecting(lp);
            lacp_sm_assert_ntt(lp);
            /* cancel timer */
            if (LACP_TIMER_ISARMED(lp, LACP_TIMER_WAIT_WHILE))
            {
                KASSERT(la->la_pending > 0, "timer_wait_while not active");
                la->la_pending--;
            }
            LACP_TIMER_DISARM(lp, LACP_TIMER_WAIT_WHILE);
            lacp_unselect(lp);
            break;
        case LACP_MUX_WAITING:
            LACP_TIMER_ARM(lp, LACP_TIMER_WAIT_WHILE, LACP_AGGREGATE_WAIT_TIME);
            la->la_pending++;
            break;
        case LACP_MUX_ATTACHED:
            lp->lp_state |= LACP_STATE_SYNC;
            lacp_disable_collecting(lp);
            lacp_sm_assert_ntt(lp);
            break;
        case LACP_MUX_COLLECTING:
            lacp_enable_collecting(lp);
            lacp_disable_distributing(lp);
            lacp_sm_assert_ntt(lp);
            break;
        case LACP_MUX_DISTRIBUTING:
            lacp_enable_distributing(lp);
            lacp_sm_assert_ntt(lp);
            break;
        default:
            MW_LOG_ERROR(LACP, "[%d][%s]unknown state", __LINE__, __func__);
    }

    MW_LOG_INFO(LACP, "[%d][%s]port[%d] mux_state %s -> %s", __LINE__, __func__, lp->port_id, lacp_format_mux_state(lp->lp_mux_state), lacp_format_mux_state(new_state));

    lp->lp_mux_state = new_state;
}

static void
lacp_sm_mux_timer(struct lacp_port *lp)
{
    struct lacp_aggregator *la = lp->lp_aggregator;
    char *ptr_buf;

    KASSERT(la->la_pending > 0, "no pending event");

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] aggregator %s, pending %d -> %d", __LINE__, __func__, lp->port_id,
        lacp_format_lagid(&la->la_actor, &la->la_partner,
                          ptr_buf, LACP_LAGIDSTR_MAX + 1),
        la->la_pending, la->la_pending - 1);

    la->la_pending--;
    MW_FREE(ptr_buf);
}

/* periodic transmit machine */

static void
lacp_sm_ptx_update_timeout(struct lacp_port *lp, uint8_t oldpstate)
{
    if (LACP_STATE_EQ(oldpstate, lp->lp_partner.lip_state, LACP_STATE_TIMEOUT))
    {
        return;
    }

    MW_LOG_INFO(LACP, "[%d][%s]port[%d] partner timeout changed", __LINE__, __func__, lp->port_id);

    /*
     * FAST_PERIODIC -> SLOW_PERIODIC
     * or
     * SLOW_PERIODIC (-> PERIODIC_TX) -> FAST_PERIODIC
     *
     * let lacp_sm_ptx_tx_schedule to update timeout.
     */

    if (lp->lp_timer[LACP_TIMER_PERIODIC] == LACP_FAST_PERIODIC_TIME)
    {
        lacp_sm_assert_ntt(lp);
    }
    LACP_TIMER_DISARM(lp, LACP_TIMER_PERIODIC);

    /*
     * if timeout has been shortened, assert NTT.
     */

    if ((lp->lp_partner.lip_state & LACP_STATE_TIMEOUT))
    {
        lacp_sm_assert_ntt(lp);
    }
}

static void
lacp_sm_ptx_tx_schedule(struct lacp_port *lp)
{
    int timeout;

    if (!(lp->lp_state & LACP_STATE_ACTIVITY) &&
        !(lp->lp_partner.lip_state & LACP_STATE_ACTIVITY))
    {
        LACP_TIMER_DISARM(lp, LACP_TIMER_PERIODIC);
        return;
    }

    if (LACP_TIMER_ISARMED(lp, LACP_TIMER_PERIODIC))
    {
        return;
    }

    timeout = (lp->lp_partner.lip_state & LACP_STATE_TIMEOUT) ? LACP_FAST_PERIODIC_TIME : LACP_SLOW_PERIODIC_TIME;

    LACP_TIMER_ARM(lp, LACP_TIMER_PERIODIC, timeout);
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] reload periodic tx timer to %ds", __LINE__, __func__, lp->port_id, timeout);
}

static void
lacp_sm_ptx_timer(struct lacp_port *lp)
{
    lacp_sm_assert_ntt(lp);
}

static void
lacp_sm_rx(struct lacp_port *lp, const struct lacpdu *du)
{
    int timeout;
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];

    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] RX SM Enter*************", __LINE__, __func__, lp->port_id);

    /*
     * check LACP_DISABLED first
     */

    if (!(lp->lp_state & LACP_STATE_AGGREGATION))
    {
        MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] RX SM Exit*************", __LINE__, __func__, lp->port_id);
        return;
    }

    /*
     * check loopback condition.
     */

    if (!lacp_compare_systemid(&du->ldu_actor.lip_systemid,
                               &lp->lp_actor.lip_systemid))
    {
        if (lsc->member & BIT(ntohs(du->ldu_actor.lip_portid.lpi_portno) - 1))
        {
            MW_LOG_INFO(LACP, "[%d][%s]A loop is formed on port[%d] and port[%d] in LAG group[%d]",
                    __LINE__, __func__, lp->port_id, ntohs(du->ldu_actor.lip_portid.lpi_portno), lp->group_id);
            MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] RX SM Exit*************", __LINE__, __func__, lp->port_id);
            return;
        }
        struct lacp_port *lp2 = ptr_g_lacp_info->lag_port[ntohs(du->ldu_actor.lip_portid.lpi_portno) - 1];
        if (NULL == lp2)
        {
            MW_LOG_INFO(LACP, "[%d][%s]Port[%d] at LACPDU is not in any LAG group!", __LINE__, __func__, ntohs(du->ldu_actor.lip_portid.lpi_portno));
        }
        else
        {
            struct lacp_softc *lsc2 = ptr_g_lacp_info->lag_group[lp2->group_id - 1];
            MW_LOG_INFO(LACP, "[%d][%s]A loop is formed on port[%d] in LAG group[%d] and port[%d] in LAG group[%d]",
                    __LINE__, __func__, lp->port_id, lp->group_id, ntohs(du->ldu_actor.lip_portid.lpi_portno), lsc2->group_id);
        }
    }

    /*
     * EXPIRED, DEFAULTED, CURRENT -> CURRENT
     */

    lp->lp_last_lacpdu_rx = osapi_sysTickGet();
    lacp_sm_rx_update_selected(lp, du);
    lacp_sm_rx_update_ntt(lp, du);
    lacp_sm_rx_record_pdu(lp, du);

    timeout = (lp->lp_state & LACP_STATE_TIMEOUT) ? LACP_SHORT_TIMEOUT_TIME : LACP_LONG_TIMEOUT_TIME;
    LACP_TIMER_ARM(lp, LACP_TIMER_CURRENT_WHILE, timeout);

    lp->lp_state &= ~LACP_STATE_EXPIRED;

    /*
     * kick transmit machine without waiting the next tick.
     */

    lacp_sm_tx(lp);
    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] RX SM Exit*************", __LINE__, __func__, lp->port_id);
}

static void
lacp_sm_rx_set_expired(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] rx set expired state", __LINE__, __func__, lp->port_id);
    lp->lp_partner.lip_state &= ~LACP_STATE_SYNC;
    lp->lp_partner.lip_state |= LACP_STATE_TIMEOUT;
    LACP_TIMER_ARM(lp, LACP_TIMER_CURRENT_WHILE, LACP_SHORT_TIMEOUT_TIME);
    LACP_TIMER_ARM(lp, LACP_TIMER_PERIODIC, LACP_FAST_PERIODIC_TIME);
    lp->lp_state |= LACP_STATE_EXPIRED;
}

static void
lacp_sm_rx_timer(struct lacp_port *lp)
{
    if ((lp->lp_state & LACP_STATE_EXPIRED) == 0)
    {
        MW_LOG_INFO(LACP, "[%d][%s]Rx machine: CURRENT -> EXPIRED", __LINE__, __func__);
        lacp_sm_rx_set_expired(lp);
    }
    else
    {
        MW_LOG_INFO(LACP, "[%d][%s]Rx machine: EXPIRED -> DEFAULTED", __LINE__, __func__);
        lacp_sm_rx_update_default_selected(lp);
        lacp_sm_rx_record_default(lp);
        lp->lp_state &= ~LACP_STATE_EXPIRED;
    }
}

static void
lacp_sm_rx_record_pdu(struct lacp_port *lp, const struct lacpdu *du)
{
    BOOL_T active;
    uint8_t oldpstate;
    char *ptr_buf;

    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] record PDU", __LINE__, __func__, lp->group_id, lp->port_id);

    oldpstate = lp->lp_partner.lip_state;

    active = (du->ldu_actor.lip_state & LACP_STATE_ACTIVITY)
        || ((lp->lp_state & LACP_STATE_ACTIVITY) &&
        (du->ldu_partner.lip_state & LACP_STATE_ACTIVITY));

    lp->lp_partner = du->ldu_actor;
    if (active &&
        ((LACP_STATE_EQ(lp->lp_state, du->ldu_partner.lip_state, LACP_STATE_AGGREGATION) &&
          !lacp_compare_peerinfo(&lp->lp_actor, &du->ldu_partner))
         || (du->ldu_partner.lip_state & LACP_STATE_AGGREGATION) == 0))
    {
/* lp->lp_partner.lip_state |= LACP_STATE_SYNC; */
    }
    else
    {
        lp->lp_partner.lip_state &= ~LACP_STATE_SYNC;
    }

    lp->lp_state &= ~LACP_STATE_DEFAULTED;

    osapi_calloc(LACP_STATESTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    if (oldpstate != lp->lp_partner.lip_state)
    {
        MW_LOG_INFO(LACP, "[%d][%s]old pstate %s", __LINE__, __func__,
            lacp_format_state(oldpstate, ptr_buf, LACP_STATESTR_MAX + 1));
        MW_LOG_INFO(LACP, "[%d][%s]new pstate %s", __LINE__, __func__,
            lacp_format_state(lp->lp_partner.lip_state, ptr_buf, LACP_STATESTR_MAX + 1));
    }

    lacp_sm_ptx_update_timeout(lp, oldpstate);
    MW_FREE(ptr_buf);
}

static void
lacp_sm_rx_update_ntt(struct lacp_port *lp, const struct lacpdu *du)
{

    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] update ntt", __LINE__, __func__, lp->group_id, lp->port_id);

    if (lacp_compare_peerinfo(&lp->lp_actor, &du->ldu_partner) ||
        !LACP_STATE_EQ(lp->lp_state, du->ldu_partner.lip_state, LACP_STATE_ACTIVITY | LACP_STATE_SYNC | LACP_STATE_AGGREGATION))
    {
        lacp_sm_assert_ntt(lp);
    }
}

static void
lacp_sm_rx_record_default(struct lacp_port *lp)
{
    uint8_t oldpstate;

    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] record default", __LINE__, __func__, lp->group_id, lp->port_id);

    oldpstate = lp->lp_partner.lip_state;
    lp->lp_partner = lacp_partner_admin_strict;
    lp->lp_state |= LACP_STATE_DEFAULTED;
    lacp_sm_ptx_update_timeout(lp, oldpstate);
}

static void
lacp_sm_rx_update_selected_from_peerinfo(struct lacp_port *lp,
    const struct lacp_peerinfo *info)
{
    if (lacp_compare_peerinfo(&lp->lp_partner, info) ||
        !LACP_STATE_EQ(lp->lp_partner.lip_state, info->lip_state,
                       LACP_STATE_AGGREGATION))
    {
        lp->lp_selected = LACP_UNSELECTED;
        /* mux machine will clean up lp->lp_aggregator */
    }
    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] selected(%d) from peerinfo", __LINE__, __func__, lp->group_id, lp->port_id, lp->lp_selected);
}

static void
lacp_sm_rx_update_selected(struct lacp_port *lp, const struct lacpdu *du)
{

    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] update selected", __LINE__, __func__, lp->group_id, lp->port_id);

    lacp_sm_rx_update_selected_from_peerinfo(lp, &du->ldu_actor);
}

static void
lacp_sm_rx_update_default_selected(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] update default selected", __LINE__, __func__, lp->group_id, lp->port_id);
    lacp_sm_rx_update_selected_from_peerinfo(lp, &lacp_partner_admin_strict);
}

/* transmit machine */

static void
lacp_sm_tx(struct lacp_port *lp)
{
    int error = 0;
    UI32_T tick, dur_tick;

    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] TX SM Enter*************", __LINE__, __func__, lp->port_id);

    if (!(lp->lp_state & LACP_STATE_AGGREGATION)
#if 1
        || (!(lp->lp_state & LACP_STATE_ACTIVITY)
            && !(lp->lp_partner.lip_state & LACP_STATE_ACTIVITY))
#endif
        )
    {
        lp->lp_flags &= ~LACP_PORT_NTT;
    }

    if (!(lp->lp_flags & LACP_PORT_NTT))
    {
        MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] TX SM Exit*************", __LINE__, __func__, lp->port_id);
        return;
    }

    tick = osapi_sysTickGet();
    if (tick >= lp->lp_last_lacpdu)
    {
        dur_tick = tick - lp->lp_last_lacpdu;
    }
    else
    {
        dur_tick = 0xFFFFFFFF - lp->lp_last_lacpdu + tick + 1;
    }
    if (dur_tick >= (LACP_FAST_PERIODIC_TIME * 1000))
    {
        lp->lp_last_lacpdu = tick;
        lp->lp_lacpdu_sent = 0;
    }

    if (lp->lp_lacpdu_sent < (3 / LACP_FAST_PERIODIC_TIME))
    {
        lp->lp_lacpdu_sent++;
    }
    else
    {
        MW_LOG_INFO(LACP, "[%d][%s]LACPDU is not sent by rate limited", __LINE__, __func__);
        MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] TX SM Exit*************", __LINE__, __func__, lp->port_id);
        return;
    }

    error = lacp_xmit_lacpdu(lp);

    if (error == 0)
    {
        lp->lp_flags &= ~LACP_PORT_NTT;
    }
    else
    {
        MW_LOG_ERROR(LACP, "[%d][%s]lacpdu transmit failure, error %d", __LINE__, __func__, error);
    }
    MW_LOG_DEBUG(LACP, "[%d][%s]*************Port[%d] TX SM Exit*************", __LINE__, __func__, lp->port_id);
}

void
lacp_sm_assert_ntt(struct lacp_port *lp)
{
    MW_LOG_INFO(LACP, "[%d][%s]group[%d] port[%d] assert NTT", __LINE__, __func__, lp->group_id, lp->port_id);
    lp->lp_flags |= LACP_PORT_NTT;
}

static void
lacp_run_timers(struct lacp_port *lp)
{
    int i;
    UI32_T time_diff;

    for (i = 0; i < LACP_NTIMER; i++)
    {
        KASSERT(lp->lp_timer[i] >= 0, "invalid timer value %d", __LINE__, __func__, lp->lp_timer[i]);
        if (lp->lp_timer[i] == 0)
        {
            continue;
        }
        else
        {
            if (i == LACP_TIMER_CURRENT_WHILE)
            {
                time_diff = osapi_sysTickGet();
                time_diff -= lp->lp_last_lacpdu_rx;
                if (time_diff >= 1000)
                {
                    /* At least one sec has elapsed since last LACP packet. */
                    --lp->lp_timer[i];
                }
            }
            else
            {
                --lp->lp_timer[i];
            }
            MW_LOG_DEBUG(LACP, "[%d][%s]port[%d] lp timer[%d] Count down from %d to %d", __LINE__, __func__, lp->port_id, i, lp->lp_timer[i] + 1, lp->lp_timer[i]);
            if ((lp->lp_timer[i] <= 0) && (lacp_timer_funcs[i]))
            {
                (*lacp_timer_funcs[i])(lp);
            }
        }
    }
}

int
lacp_marker_input(struct lacp_port *lp, struct pbuf *p)
{
    struct lacp_softc *lsc = ptr_g_lacp_info->lag_group[lp->group_id - 1];
    struct lacp_port *lp2;
    struct markerdu *mdu;
    int error = 0;
    int pending = 0;
    AIR_MAC_T mac;
    UI32_T unit = 0;
    UI8_T i;
    err_t rc;

    ptr_g_lacp_info->markerpdu_rx_count[lp->port_id - 1]++;
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] Marker PDU RX done(rx count:%d)", __LINE__, __func__, lp->port_id, ptr_g_lacp_info->markerpdu_rx_count[lp->port_id - 1]);

    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]***Error***, get sys-mac error", __LINE__, __func__);
        goto bad;
    }

    if ((p->len + sizeof(struct eth_hdr)) != sizeof(*mdu))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]group[%d] port[%d] len(%lu) != market pdu len(%lu)", __LINE__, __func__, lp->group_id, lp->port_id, p->len + sizeof(struct eth_hdr), sizeof(*mdu));
        goto bad;
    }

    mdu = (struct markerdu *)((UI8_T *)(p->payload) - sizeof(struct eth_hdr));

    if (osapi_memcmp(&p->ether_hdr.dest, &ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN))
    {
        MW_LOG_ERROR(LACP, "[%d][%s]DA(%02X.%02X.%02X.%02X.%02X.%02X) != slow proto multa ether addr", __LINE__, __func__,
                    p->ether_hdr.dest.addr[0],
                    p->ether_hdr.dest.addr[1],
                    p->ether_hdr.dest.addr[2],
                    p->ether_hdr.dest.addr[3],
                    p->ether_hdr.dest.addr[4],
                    p->ether_hdr.dest.addr[5]);
        goto bad;
    }

    if (mdu->mdu_sph.sph_version != 1)
    {
        MW_LOG_ERROR(LACP, "[%d][%s]group[%d] port[%d] subversion != 1", __LINE__, __func__, lp->group_id, lp->port_id);
        goto bad;
    }

    MW_LOG_INFO(LACP, "[%d][%s]RX Marker PDU type:%d", __LINE__, __func__, mdu->mdu_tlv.tlv_type);
    switch (mdu->mdu_tlv.tlv_type)
    {
        case MARKER_TYPE_INFO:
            if (tlv_check(mdu, sizeof(*mdu), &mdu->mdu_tlv,
                          marker_info_tlv_template, TRUE))
            {
                MW_LOG_ERROR(LACP, "[%d][%s]group[%d] port[%d] tlv check fail", __LINE__, __func__, lp->group_id, lp->port_id);
                goto bad;
            }
            mdu->mdu_tlv.tlv_type = MARKER_TYPE_RESPONSE;
            osapi_memcpy(&mdu->mdu_eh.dest, &ethermulticastaddr_slowprotocols, ETHER_ADDR_LEN);
            sys_mgmt_convert_port_mac(mac, lp->port_id);
            osapi_memcpy(&mdu->mdu_eh.src, mac, ETHER_ADDR_LEN);
            p->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
            p->stag_hdr.tx_hdr.tx_stag_insert.opc = AIR_STAG_OPC_PORTMAP_BYPASS_TRUNK;
            p->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << (lp->port_id));
            rc = ethernet_output_use_default_netif(p, ETHTYPE_LACP_SLOW);
            MW_LOG_INFO(LACP, "[%d][%s]port[%d] Marker Response TX done. rc:%d", __LINE__, __func__, lp->port_id, rc);
            break;

        case MARKER_TYPE_RESPONSE:
            if (tlv_check(mdu, sizeof(*mdu), &mdu->mdu_tlv, marker_response_tlv_template, TRUE))
            {
                MW_LOG_ERROR(LACP, "[%d][%s]group[%d] port[%d] tlv check fail", __LINE__, __func__, lp->group_id, lp->port_id);
                goto bad;
            }
            MW_LOG_INFO(LACP, "[%d][%s]marker response, port=%d, sys=[%02X:%02X:%02X:%02X:%02X:%02X], id=%lu, pad=0x%x", __LINE__, __func__,
                        ntohs(mdu->mdu_info.mi_rq_port),
                        mdu->mdu_info.mi_rq_system[0],
                        mdu->mdu_info.mi_rq_system[1],
                        mdu->mdu_info.mi_rq_system[2],
                        mdu->mdu_info.mi_rq_system[3],
                        mdu->mdu_info.mi_rq_system[4],
                        mdu->mdu_info.mi_rq_system[5],
                        ntohl(mdu->mdu_info.mi_rq_xid),
                        *(UI16_T *)mdu->mdu_info.mi_pad);

            /* Verify that it is the last marker we sent out */
            if (osapi_memcmp(&mdu->mdu_info, &lp->lp_marker,
                             sizeof(struct lacp_markerinfo)))
            {
                MW_LOG_WARN(LACP, "[%d][%s]group[%d] port[%d] verify that it is the last marker we sent out fail", __LINE__, __func__, lp->group_id, lp->port_id);
                goto bad;
            }

            lp->lp_flags &= ~LACP_PORT_MARK;

            if (lsc->lsc_suppress_distributing)
            {
                /* Check if any ports are waiting for a response */
                for (i = 0; i < MAX_TRUNK_MEMBER_NUM; i++)
                {
                    lp2 = ptr_g_lacp_info->lag_port[i];
                    if (lp2 == NULL || lp2->group_id != lsc->group_id)
                    {
                        continue;
                    }
                    if (lp2->lp_flags & LACP_PORT_MARK)
                    {
                        pending = 1;
                        break;
                    }
                }

                if (pending == 0)
                {
                    MW_LOG_INFO(LACP, "[%d][%s]All marker responses received correctly", __LINE__, __func__);
                    lsc->lsc_suppress_distributing = FALSE;
                    lacp_update_port_state();
                }
            }

            break;

        default:
            goto bad;
    }

    pbuf_free(p);
    MW_LOG_INFO(LACP, "[%d][%s]port[%d] Marker PDU Handle done(rx count:%d)", __LINE__, __func__, lp->port_id, ptr_g_lacp_info->markerpdu_rx_count[lp->port_id - 1]);
    return (error);

bad:
    MW_LOG_INFO(LACP, "[%d][%s]bad marker frame", __LINE__, __func__);
    pbuf_free(p);
    return (-1);
}

static int
tlv_check(const void *p, size_t size, const struct tlvhdr *tlv,
    const struct tlv_template *tmpl, BOOL_T check_type)
{
    while (/* CONSTCOND */ 1)
    {
        if ((const char *)tlv - (const char *)p + sizeof(*tlv) > size)
        {
            return (-1);
        }
        if ((check_type && tlv->tlv_type != tmpl->tmpl_type) ||
            tlv->tlv_length != tmpl->tmpl_length)
        {
            return (-1);
        }
        if (tmpl->tmpl_type == 0)
        {
            break;
        }
        tlv = (const struct tlvhdr *)
            ((const char *)tlv + tlv->tlv_length);
        tmpl++;
    }

    return (0);
}

/* Debugging */
const char *
lacp_format_mac(const uint8_t *mac, char *buf, size_t buflen)
{
    snprintf(buf, buflen, "%02X-%02X-%02X-%02X-%02X-%02X",
        (int)mac[0],
        (int)mac[1],
        (int)mac[2],
        (int)mac[3],
        (int)mac[4],
        (int)mac[5]);

    return (buf);
}

const char *
lacp_format_systemid(const struct lacp_systemid *sysid,
    char *buf, size_t buflen)
{
    char *ptr_macbuf;

    osapi_calloc(LACP_MACSTR_MAX + 1, LACP_MODULE, (void **)&ptr_macbuf);

    snprintf(buf, buflen, "%04X,%s",
        ntohs(sysid->lsi_prio),
        lacp_format_mac(sysid->lsi_mac, ptr_macbuf, LACP_MACSTR_MAX + 1));

    MW_FREE(ptr_macbuf);
    return (buf);
}

const char *
lacp_format_portid(const struct lacp_portid *portid, char *buf, size_t buflen)
{
    snprintf(buf, buflen, "%04X,%04X",
        ntohs(portid->lpi_prio),
        ntohs(portid->lpi_portno));

    return (buf);
}

const char *
lacp_format_partner(const struct lacp_peerinfo *peer, char *buf, size_t buflen)
{
    char *ptr_sysid;
    char *ptr_portid;

    osapi_calloc(LACP_SYSTEMIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_sysid);
    osapi_calloc(LACP_PORTIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_portid);
    snprintf(buf, buflen, "(%s,%04X,%s)",
        lacp_format_systemid(&peer->lip_systemid, ptr_sysid, LACP_SYSTEMIDSTR_MAX + 1),
        peer->lip_key,
        lacp_format_portid(&peer->lip_portid, ptr_portid, LACP_PORTIDSTR_MAX + 1));

    MW_FREE(ptr_sysid);
    MW_FREE(ptr_portid);
    return (buf);
}

const char *
lacp_format_lagid(const struct lacp_peerinfo *a,
    const struct lacp_peerinfo *b, char *buf, size_t buflen)
{
    char *ptr_astr;
    char *ptr_bstr;

    osapi_calloc(LACP_PARTNERSTR_MAX + 1, LACP_MODULE, (void **)&ptr_astr);
    osapi_calloc(LACP_PARTNERSTR_MAX + 1, LACP_MODULE, (void **)&ptr_bstr);
    snprintf(buf, buflen, "[%s,%s]",
        lacp_format_partner(a, ptr_astr, LACP_PARTNERSTR_MAX + 1),
        lacp_format_partner(b, ptr_bstr, LACP_PARTNERSTR_MAX + 1));

    MW_FREE(ptr_astr);
    MW_FREE(ptr_bstr);
    return (buf);
}

const char *
lacp_format_lagid_aggregator(const struct lacp_aggregator *la,
    char *buf, size_t buflen)
{
    if (la == NULL)
    {
        return ("(none)");
    }

    return (lacp_format_lagid(&la->la_actor, &la->la_partner, buf, buflen));
}

const char *
lacp_format_state(uint8_t state, char *buf, size_t buflen)
{
    int offset = 0, i;

    for (i = 0; i < 8; ++i)
    {
        if (state & (1 << i))
        {
            int len = snprintf(buf + offset, buflen - offset, "%s%s",
                               offset == 0 ? "" : ", ",
                               lacp_state_bit_desc[i]);
            offset += len;
            if (offset >= buflen)
            {
                buf[buflen - 1] = '\0';
                return buf;
            }
        }
    }

    if (offset == 0)
    {
        snprintf(buf, buflen, "NONE");
    }
    else
    {
        buf[offset] = '\0';
    }

    return buf;
}

const char *
lacp_format_mux_state(lacp_mux_state_t state)
{
    return lacp_mux_state_bit_desc[state];
}

static void
lacp_dump_lacpdu(const struct lacpdu *du)
{
    char *ptr_buf;
    char *ptr_buf2;

    osapi_calloc(LACP_PARTNERSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    osapi_calloc(LACP_STATESTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf2);
    MW_LOG_DEBUG(LACP, "[%d][%s]actor=%s", __LINE__, __func__,
        lacp_format_partner(&du->ldu_actor, ptr_buf, LACP_PARTNERSTR_MAX + 1));
    MW_LOG_DEBUG(LACP, "[%d][%s]actor.state=%s", __LINE__, __func__,
        lacp_format_state(du->ldu_actor.lip_state, ptr_buf2, LACP_STATESTR_MAX + 1));
    MW_LOG_DEBUG(LACP, "[%d][%s]actor.port_priority=%d, port id=%d", __LINE__, __func__,
        ntohs(du->ldu_actor.lip_portid.lpi_prio), ntohs(du->ldu_actor.lip_portid.lpi_portno));
    MW_LOG_DEBUG(LACP, "[%d][%s]partner=%s", __LINE__, __func__,
        lacp_format_partner(&du->ldu_partner, ptr_buf, LACP_PARTNERSTR_MAX + 1));
    MW_LOG_DEBUG(LACP, "[%d][%s]partner.state=%s", __LINE__, __func__,
        lacp_format_state(du->ldu_partner.lip_state, ptr_buf2, LACP_STATESTR_MAX + 1));
    MW_LOG_DEBUG(LACP, "[%d][%s]partner.port_priority=%d, port id=%d", __LINE__, __func__,
        ntohs(du->ldu_partner.lip_portid.lpi_prio), ntohs(du->ldu_partner.lip_portid.lpi_portno));
    MW_LOG_DEBUG(LACP, "[%d][%s]Maxdelay=%d", __LINE__, __func__, ntohs(du->ldu_collector.lci_maxdelay));

    MW_FREE(ptr_buf);
    MW_FREE(ptr_buf2);
}

void
mw_lacp_tick(
    void)
{
    UI8_T i;

    for (i = 0;i < MAX_TRUNK_NUM;i++)
    {
        lacp_tick(ptr_g_lacp_info->lag_group[i]);
    }
}

MW_ERROR_NO_T
lacp_allocate_settings_buffer(
    void)
{
    osapi_calloc(sizeof(lacp_info_t), LACP_MODULE, (void **)&ptr_g_lacp_info);
    if (NULL == ptr_g_lacp_info)
    {
        return MW_E_NO_MEMORY;
    }

    return MW_E_OK;
}

lacp_info_t *
lacp_get_settings(
    void)
{
    return ptr_g_lacp_info;
}
#endif