/**
 * @file
 * Ethernet protocol definitions
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_HDR_PROT_ETHERNET_H
#define LWIP_HDR_PROT_ETHERNET_H

#include "lwip/arch.h"
#include "lwip/prot/ieee.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ETH_HWADDR_LEN
#ifdef ETHARP_HWADDR_LEN
#define ETH_HWADDR_LEN    ETHARP_HWADDR_LEN /* compatibility mode */
#else
#define ETH_HWADDR_LEN    6
#endif
#endif

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
/** An Ethernet MAC address */
struct eth_addr {
  PACK_STRUCT_FLD_8(u8_t addr[ETH_HWADDR_LEN]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

/** Initialize a struct eth_addr with its 6 bytes (takes care of correct braces) */
#define ETH_ADDR(b0, b1, b2, b3, b4, b5) {{b0, b1, b2, b3, b4, b5}}

/* IEEE Std 802.1D Bridge Group Address */
#define IEEE_802_RESERVE_00 {{0x01, 0x80, 0xc2, 0x00, 0x00, 0x00}}

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
/** Ethernet header */
struct eth_hdr {
#if ETH_PAD_SIZE
  PACK_STRUCT_FLD_8(u8_t padding[ETH_PAD_SIZE]);
#endif
  PACK_STRUCT_FLD_S(struct eth_addr dest);
  PACK_STRUCT_FLD_S(struct eth_addr src);
  PACK_STRUCT_FIELD(u16_t type);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

#ifdef LLC_SUPPORT
PACK_STRUCT_BEGIN
struct llc_hdr {
  PACK_STRUCT_FLD_8(u8_t llc_dsap);
  PACK_STRUCT_FLD_8(u8_t llc_ssap);
  PACK_STRUCT_FLD_8(u8_t llc_ctrl);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#endif /* LLC_SUPPORT */

#define SIZEOF_ETH_HDR (14 + ETH_PAD_SIZE)

#ifdef STAG_INSERT_MODE
struct eth_rx_stag_hdr {
  u32_t rsn:3;
  u32_t vpm:2;
  u32_t sp:5;
  u32_t unused:6;
  u32_t pcp:3;
  u32_t dei:1;
  u32_t vid:12;
} PACK_STRUCT_STRUCT;

struct eth_tx_stag_hdr {
  u8_t mode;
  union {
    struct {
      u32_t opc:3;
      u32_t dp:29;
    } tx_stag_insert;

    struct {
      u16_t opc:3;
      u16_t vpm:2;
      u16_t dp:11;
    } tx_stag_replace;
   };
} PACK_STRUCT_STRUCT;

struct eth_vlan_hdr {
  u16_t tpid;
  u16_t priority:3;
  u16_t cfi:1;
  u16_t vid:12;
} PACK_STRUCT_STRUCT;

#define SIZEOF_ETH_TYPE  (2)
#define SIZEOF_STAG_HDR  (4)
#define SIZEOF_VLAN_HDR  (4)
#ifdef LLC_SUPPORT
#define SIZEOF_LLC_HDR   (3)
#endif /* LLC_SUPPORT */

typedef enum
{
  STAG_HDR_MODE_START = 0,
  STAG_HDR_MODE_REPLACE,
  STAG_HDR_MODE_INSERT,
  STAG_HDR_MODE_LAST
} STAG_HDR_MODE_T;

#else /* STAG_INSERT_MODE */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
/** VLAN header inserted between ethernet header and payload
 * if 'type' in ethernet header is ETHTYPE_VLAN.
 * See IEEE802.Q */
struct eth_vlan_hdr {
  PACK_STRUCT_FIELD(u16_t prio_vid);
  PACK_STRUCT_FIELD(u16_t tpid);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif
#endif /* STAG_INSERT_MODE */

#define VLAN_ID(vlan_hdr) (lwip_htons((vlan_hdr)->prio_vid) & 0xFFF)

/** The 24-bit IANA IPv4-multicast OUI is 01-00-5e: */
#define LL_IP4_MULTICAST_ADDR_0 0x01
#define LL_IP4_MULTICAST_ADDR_1 0x00
#define LL_IP4_MULTICAST_ADDR_2 0x5e

/** IPv6 multicast uses this prefix */
#define LL_IP6_MULTICAST_ADDR_0 0x33
#define LL_IP6_MULTICAST_ADDR_1 0x33

#define eth_addr_cmp(addr1, addr2) (memcmp((addr1)->addr, (addr2)->addr, ETH_HWADDR_LEN) == 0)

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_PROT_ETHERNET_H */
