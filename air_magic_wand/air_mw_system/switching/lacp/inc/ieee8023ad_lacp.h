/* $NetBSD: ieee8023ad_impl.h,v 1.2 2005/12/10 23:21:39 elad Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2005 YAMAMOTO Takashi,
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

/*
 * IEEE802.3ad LACP
 *
 * implementation details.
 */
#ifndef _IEEE8023AD_LACP_H_
#define _IEEE8023AD_LACP_H_

#include "osapi.h"
#include "mw_error.h"
#include "db_api.h"
#include "mw_msg.h"
#include "mw_log.h"
#include "mac_utils.h"
#include "mw_lacp.h"
#include "osapi_message.h"
#include "osapi_thread.h"
#include "osapi_timer.h"
#include "osapi_mutex.h"
#include "osapi_string.h"
#include "osapi_memory.h"
#include "ethernetif.h"
#include "lwip/pbuf.h"
#include "lwip/ip4.h"
#include "air_stag.h"
#include "air_stp.h"

#define LACP_TIMER_CURRENT_WHILE 0
#define LACP_TIMER_PERIODIC  1
#define LACP_TIMER_WAIT_WHILE  2
#define LACP_NTIMER   3
#define LACP_SYSTEM_PRIO 0x8000
#define LACP_PORT_PRIO  0x8000
#define LACP_SFP_PORT       (1)

#define LACP_LOG_LV_DISABLE (0)
#define LACP_LOG_LV_ERROR (LACP_LOG_LV_DISABLE + 1)
#define LACP_LOG_LV_WARNING (LACP_LOG_LV_ERROR + 1)
#define LACP_LOG_LV_INFO (LACP_LOG_LV_WARNING + 1)
#define LACP_LOG_LV_DEBUG (LACP_LOG_LV_INFO + 1)

#define LACP_MUTEX   (0)

/* following constants don't include terminating NUL */
#define LACP_MACSTR_MAX  (2*6 + 5)
#define LACP_SYSTEMPRIOSTR_MAX (4)
#define LACP_SYSTEMIDSTR_MAX (LACP_SYSTEMPRIOSTR_MAX + 1 + LACP_MACSTR_MAX)
#define LACP_PORTPRIOSTR_MAX (4)
#define LACP_PORTNOSTR_MAX (4)
#define LACP_PORTIDSTR_MAX (LACP_PORTPRIOSTR_MAX + 1 + LACP_PORTNOSTR_MAX)
#define LACP_KEYSTR_MAX  (4)
#define LACP_PARTNERSTR_MAX \
 (1 + LACP_SYSTEMIDSTR_MAX + 1 + LACP_KEYSTR_MAX + 1 \
 + LACP_PORTIDSTR_MAX + 1)
#define LACP_LAGIDSTR_MAX \
 (1 + LACP_PARTNERSTR_MAX + 1 + LACP_PARTNERSTR_MAX + 1)
#define LACP_STATESTR_MAX (255) /* XXX */
#define LACP_PORTLIST_MAX (PLAT_MAX_PORT_NUM * 3 + 1)

/*
 * IEEE802.3ad LACP
 *
 * protocol definitions.
 */

#define LACP_STATE_ACTIVITY (1<<0)
#define LACP_STATE_TIMEOUT (1<<1)
#define LACP_STATE_AGGREGATION (1<<2)
#define LACP_STATE_SYNC  (1<<3)
#define LACP_STATE_COLLECTING (1<<4)
#define LACP_STATE_DISTRIBUTING (1<<5)
#define LACP_STATE_DEFAULTED (1<<6)
#define LACP_STATE_EXPIRED (1<<7)

#define LACP_PORT_NTT  0x00000001
#define LACP_PORT_MARK  0x00000002

#define LACP_STATE_BITS  \
 "\020"   \
 "\001ACTIVITY"  \
 "\002TIMEOUT"  \
 "\003AGGREGATION" \
 "\004SYNC"  \
 "\005COLLECTING" \
 "\006DISTRIBUTING" \
 "\007DEFAULTED"  \
 "\010EXPIRED"

/*
 * IEEE802.3 slow protocols
 *
 * protocol (on-wire) definitions.
 *
 * XXX should be elsewhere.
 */
#define SLOWPROTOCOLS_SUBTYPE_LACP 1
#define SLOWPROTOCOLS_SUBTYPE_MARKER 2

#define LACP_TYPE_ACTORINFO 1
#define LACP_TYPE_PARTNERINFO 2
#define LACP_TYPE_COLLECTORINFO 3

/* timeout values (in sec) */
#define LACP_FAST_PERIODIC_TIME  (1)
#define LACP_SLOW_PERIODIC_TIME  (30)
#define LACP_SHORT_TIMEOUT_TIME  (3 * LACP_FAST_PERIODIC_TIME)
#define LACP_LONG_TIMEOUT_TIME  (3 * LACP_SLOW_PERIODIC_TIME)
#define LACP_CHURN_DETECTION_TIME (60)
#define LACP_AGGREGATE_WAIT_TIME (2)
#define LACP_TRANSIT_DELAY  3000 /* in msec */

#define LACP_STATE_EQ(s1, s2, mask) \
 ((((s1) ^ (s2)) & (mask)) == 0)

#define LACP_SYS_PRI(peer) (peer).lip_systemid.lsi_prio

#define LACP_PORT(_lp) ((struct lacp_port *)(_lp)->lp_psc)
#define LACP_SOFTC(_sc) ((struct lacp_softc *)(_sc)->sc_psc)
#define LACP_TIMER_ARM(port, timer, val) \
 (port)->lp_timer[(timer)] = (val)
#define LACP_TIMER_DISARM(port, timer) \
 (port)->lp_timer[(timer)] = 0
#define LACP_TIMER_ISARMED(port, timer) \
 ((port)->lp_timer[(timer)] > 0)

#define KASSERT(cond, format, ...) \
do { \
    if (!(cond)) { \
        MW_LOG_DEBUG(LACP, format, ##__VA_ARGS__); \
    } \
} while (0)

struct slowprothdr {
    uint8_t  sph_subtype;
    uint8_t  sph_version;
} ATTRIBUTE_PACK;

/*
 * TLV on-wire structure.
 */

struct tlvhdr {
    uint8_t  tlv_type;
    uint8_t  tlv_length;
    /* uint8_t tlv_value[]; */
} ATTRIBUTE_PACK;

/*
 * ... and our implementation.
 */

#define TLV_SET(tlv, type, length) \
 do { \
  (tlv)->tlv_type = (type); \
  (tlv)->tlv_length = sizeof(*tlv) + (length); \
 } while (/*CONSTCOND*/0)

struct tlv_template {
    uint8_t   tmpl_type;
    uint8_t   tmpl_length;
};

struct lacp_systemid {
    uint16_t  lsi_prio;
    uint8_t   lsi_mac[6];
} ATTRIBUTE_PACK;

struct lacp_portid {
    uint16_t  lpi_prio;
    uint16_t  lpi_portno;
} ATTRIBUTE_PACK;

struct lacp_peerinfo {
    struct lacp_systemid lip_systemid;
    uint16_t  lip_key;
    struct lacp_portid lip_portid;
    uint8_t   lip_state;
    uint8_t   lip_resv[3];
} ATTRIBUTE_PACK;

struct lacp_collectorinfo {
    uint16_t  lci_maxdelay;
    uint8_t   lci_resv[12];
} ATTRIBUTE_PACK;

struct lacpdu {
    struct eth_hdr ldu_eh;
    struct slowprothdr ldu_sph;

    struct tlvhdr  ldu_tlv_actor;
    struct lacp_peerinfo ldu_actor;
    struct tlvhdr  ldu_tlv_partner;
    struct lacp_peerinfo ldu_partner;
    struct tlvhdr  ldu_tlv_collector;
    struct lacp_collectorinfo ldu_collector;
    struct tlvhdr  ldu_tlv_term;
    uint8_t   ldu_resv[50];
} ATTRIBUTE_PACK;

/*
 * IEEE802.3ad marker protocol
 *
 * protocol (on-wire) definitions.
 */
struct lacp_markerinfo {
    uint16_t  mi_rq_port;
    uint8_t   mi_rq_system[ETHER_ADDR_LEN];
    uint32_t  mi_rq_xid;
    uint8_t   mi_pad[2];
} ATTRIBUTE_PACK;

struct markerdu {
    struct eth_hdr mdu_eh;
    struct slowprothdr mdu_sph;

    struct tlvhdr  mdu_tlv;
    struct lacp_markerinfo mdu_info;
    struct tlvhdr  mdu_tlv_term;
    uint8_t   mdu_resv[90];
} ATTRIBUTE_PACK;

#define MARKER_TYPE_INFO 0x01
#define MARKER_TYPE_RESPONSE 0x02

enum lacp_selected {
    LACP_UNSELECTED,
    LACP_STANDBY, /* not used in this implementation */
    LACP_SELECTED,
};

typedef enum lacp_mux_state {
    LACP_MUX_DETACHED,
    LACP_MUX_WAITING,
    LACP_MUX_ATTACHED,
    LACP_MUX_COLLECTING,
    LACP_MUX_DISTRIBUTING,
} lacp_mux_state_t;

#define LACP_MAX_PORTS  MAX_TRUNK_MEMBER_NUM

struct lacp_port {
    TAILQ_ENTRY(lacp_port) lp_dist_q;
    UI8_T group_id;
    UI8_T port_id;
    struct lacp_peerinfo lp_partner;
    struct lacp_peerinfo lp_actor;
    struct lacp_markerinfo lp_marker;
#define lp_state lp_actor.lip_state
#define lp_key  lp_actor.lip_key
#define lp_systemid lp_actor.lip_systemid
    UI32_T  lp_last_lacpdu;
    UI32_T  lp_last_lacpdu_rx;
    int   lp_lacpdu_sent;
    enum lacp_mux_state lp_mux_state;
    enum lacp_selected lp_selected;
    int   lp_flags;
    u_int  lp_media;
    int   lp_timer[LACP_NTIMER];

    struct lacp_aggregator *lp_aggregator;
};

struct lacp_aggregator {
    TAILQ_ENTRY(lacp_aggregator) la_q;
    int   la_refcnt; /* num of ports which selected us */
    int   la_nports; /* num of distributing ports  */
    TAILQ_HEAD(, lacp_port) la_ports; /* distributing ports */
    struct lacp_peerinfo la_partner;
    struct lacp_peerinfo la_actor;
    int   la_pending; /* number of ports in wait_while */
};

struct lacp_softc {
    struct lacp_aggregator *lsc_active_aggregator;
    TAILQ_HEAD(, lacp_aggregator) lsc_aggregators;
    BOOL_T  lsc_suppress_distributing;

    UI8_T group_id;
    UI32_T member;
    UI8_T member_cnt;
    UI8_T port_prio_descending[MAX_TRUNK_MEMBER_NUM];
    BOOL_T  lsc_fast_timeout; /* if set, fast timeout */
};

typedef struct lacp_info_s {
    timehandle_t lacp_sm_timer_handle;
    timehandle_t lacp_sd_timer_handle[MAX_TRUNK_NUM];
    struct lacp_softc *lag_group[MAX_TRUNK_NUM];
    struct lacp_port *lag_port[MAX_PORT_NUM];
    UI32_T lacp_acl_id;
    UI32_T lacp_rate_limit_id;
    UI16_T lacpdu_tx_count[MAX_PORT_NUM];
    UI16_T lacpdu_rx_count[MAX_PORT_NUM];
    UI16_T markerpdu_tx_count[MAX_PORT_NUM];
    UI16_T markerpdu_rx_count[MAX_PORT_NUM];
    UI8_T lacp_max_port_num;
    UI16_T lacp_system_priority;
    UI16_T lacp_port_priority[MAX_PORT_NUM];
#if LACP_MUTEX
    semaphorehandle_t lacp_mutex;
#endif
} lacp_info_t;

int lacp_input(struct lacp_port *, struct pbuf *);
int lacp_port_create(struct lacp_port **, UI8_T, UI16_T);
void lacp_port_destroy(struct lacp_port *);
void lacp_linkstate(struct lacp_port *);
void lacp_port_enable(struct lacp_port *lp);
void lacp_port_disable(struct lacp_port *lp);
void lacp_unselect(struct lacp_port *lp);
void lacp_update_port_state(void);
UI16_T lacp_mw_compose_key(struct lacp_port *lp);
void lacp_transit_expire(UI8_T);
void lacp_sm_assert_ntt(struct lacp_port *);
const char *lacp_format_lagid_aggregator(const struct lacp_aggregator *, char *, size_t);
const char *lacp_format_state(uint8_t, char *, size_t);
const char *lacp_format_mac(const uint8_t *, char *, size_t);
const char *lacp_format_systemid(const struct lacp_systemid *, char *,
            size_t);
const char *lacp_format_portid(const struct lacp_portid *, char *,
            size_t);
const char *lacp_format_mux_state(lacp_mux_state_t);
void mw_lacp_tick(void);
MW_ERROR_NO_T lacp_allocate_settings_buffer(void);
lacp_info_t *lacp_get_settings(void);

#endif /* _IEEE8023AD_LACP_H */
