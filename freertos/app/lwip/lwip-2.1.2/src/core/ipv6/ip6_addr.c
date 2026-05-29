/**
 * @file
 *
 * IPv6 addresses.
 */

/*
 * Copyright (c) 2010 Inico Technologies Ltd.
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
 * Author: Ivan Delamer <delamer@inicotech.com>
 *
 * Functions for handling IPv6 addresses.
 *
 * Please coordinate changes and requests with Ivan Delamer
 * <delamer@inicotech.com>
 */

#include "lwip/opt.h"

#if LWIP_IPV6  /* don't build if not configured for use in lwipopts.h */

#include "lwip/ip_addr.h"
#include "lwip/def.h"

#include <string.h>

#if LWIP_IPV4
#include "lwip/ip4_addr.h" /* for ip6addr_aton to handle IPv4-mapped addresses */
#endif /* LWIP_IPV4 */

/* used by IP6_ADDR_ANY(6) in ip6_addr.h */
const ip_addr_t ip6_addr_any = IPADDR6_INIT(0ul, 0ul, 0ul, 0ul);

#define lwip_xchar(i)        ((char)((i) < 10 ? '0' + (i) : 'A' + (i) - 10))

/**
 * Check whether "cp" is a valid ascii representation
 * of an IPv6 address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 *
 * @param cp IPv6 address in ascii representation (e.g. "FF01::1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int
ip6addr_aton(const char *cp, ip6_addr_t *addr)
{
  u32_t addr_index, zero_blocks, current_block_index, current_block_value;
  const char *s;
#if LWIP_IPV4
  int check_ipv4_mapped = 0;
#endif /* LWIP_IPV4 */

  /* Count the number of colons, to count the number of blocks in a "::" sequence
     zero_blocks may be 1 even if there are no :: sequences */
  zero_blocks = 8;
  for (s = cp; *s != 0; s++) {
    if (*s == ':') {
      zero_blocks--;
#if LWIP_IPV4
    } else if (*s == '.') {
      if ((zero_blocks == 5) ||(zero_blocks == 2)) {
        check_ipv4_mapped = 1;
        /* last block could be the start of an IPv4 address */
        zero_blocks--;
      } else {
        /* invalid format */
        return 0;
      }
      break;
#endif /* LWIP_IPV4 */
    } else if (!lwip_isxdigit(*s)) {
      break;
    }
  }

  /* parse each block */
  addr_index = 0;
  current_block_index = 0;
  current_block_value = 0;
  for (s = cp; *s != 0; s++) {
    if (*s == ':') {
      if (addr) {
        if (current_block_index & 0x1) {
          addr->addr[addr_index++] |= current_block_value;
        }
        else {
          addr->addr[addr_index] = current_block_value << 16;
        }
      }
      current_block_index++;
#if LWIP_IPV4
      if (check_ipv4_mapped) {
        if (current_block_index == 6) {
          ip4_addr_t ip4;
          int ret = ip4addr_aton(s + 1, &ip4);
          if (ret) {
            if (addr) {
              addr->addr[3] = lwip_htonl(ip4.addr);
              current_block_index++;
              goto fix_byte_order_and_return;
            }
            return 1;
          }
        }
      }
#endif /* LWIP_IPV4 */
      current_block_value = 0;
      if (current_block_index > 7) {
        /* address too long! */
        return 0;
      }
      if (s[1] == ':') {
        if (s[2] == ':') {
          /* invalid format: three successive colons */
          return 0;
        }
        s++;
        /* "::" found, set zeros */
        while (zero_blocks > 0) {
          zero_blocks--;
          if (current_block_index & 0x1) {
            addr_index++;
          } else {
            if (addr) {
              addr->addr[addr_index] = 0;
            }
          }
          current_block_index++;
          if (current_block_index > 7) {
            /* address too long! */
            return 0;
          }
        }
      }
    } else if (lwip_isxdigit(*s)) {
      /* add current digit */
      current_block_value = (current_block_value << 4) +
          (lwip_isdigit(*s) ? (u32_t)(*s - '0') :
          (u32_t)(10 + (lwip_islower(*s) ? *s - 'a' : *s - 'A')));
    } else {
      /* unexpected digit, space? CRLF? */
      break;
    }
  }

  if (addr) {
    if (current_block_index & 0x1) {
      addr->addr[addr_index++] |= current_block_value;
    }
    else {
      addr->addr[addr_index] = current_block_value << 16;
    }
#if LWIP_IPV4
fix_byte_order_and_return:
#endif
    /* convert to network byte order. */
    for (addr_index = 0; addr_index < 4; addr_index++) {
      addr->addr[addr_index] = lwip_htonl(addr->addr[addr_index]);
    }

    ip6_addr_clear_zone(addr);
  }

  if (current_block_index != 7) {
    return 0;
  }

  return 1;
}

/**
 * Convert numeric IPv6 address into ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip6 address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         representation of addr
 */
char *
ip6addr_ntoa(const ip6_addr_t *addr)
{
  static char str[40];
  return ip6addr_ntoa_r(addr, str, 40);
}

/**
 * Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param addr ip6 address in network order to convert
 * @param buf target buffer where the string is stored
 * @param buflen length of buf
 * @return either pointer to buf which now holds the ASCII
 *         representation of addr or NULL if buf was too small
 */
char *
ip6addr_ntoa_r(const ip6_addr_t *addr, char *buf, int buflen)
{
  u32_t current_block_index, current_block_value, next_block_value;
  s32_t i;
  u8_t zero_flag, empty_block_flag;

#if LWIP_IPV4
  if (ip6_addr_isipv4mappedipv6(addr)) {
    /* This is an IPv4 mapped address */
    ip4_addr_t addr4;
    char *ret;
#define IP4MAPPED_HEADER "::FFFF:"
    char *buf_ip4 = buf + sizeof(IP4MAPPED_HEADER) - 1;
    int buflen_ip4 = buflen - sizeof(IP4MAPPED_HEADER) + 1;
    if (buflen < (int)sizeof(IP4MAPPED_HEADER)) {
      return NULL;
    }
    memcpy(buf, IP4MAPPED_HEADER, sizeof(IP4MAPPED_HEADER));
    addr4.addr = addr->addr[3];
    ret = ip4addr_ntoa_r(&addr4, buf_ip4, buflen_ip4);
    if (ret != buf_ip4) {
      return NULL;
    }
    return buf;
  }
#endif /* LWIP_IPV4 */
  i = 0;
  empty_block_flag = 0; /* used to indicate a zero chain for "::' */

  for (current_block_index = 0; current_block_index < 8; current_block_index++) {
    /* get the current 16-bit block */
    current_block_value = lwip_htonl(addr->addr[current_block_index >> 1]);
    if ((current_block_index & 0x1) == 0) {
      current_block_value = current_block_value >> 16;
    }
    current_block_value &= 0xffff;

    /* Check for empty block. */
    if (current_block_value == 0) {
      if (current_block_index == 7 && empty_block_flag == 1) {
        /* special case, we must render a ':' for the last block. */
        buf[i++] = ':';
        if (i >= buflen) {
          return NULL;
        }
        break;
      }
      if (empty_block_flag == 0) {
        /* generate empty block "::", but only if more than one contiguous zero block,
         * according to current formatting suggestions RFC 5952. */
        next_block_value = lwip_htonl(addr->addr[(current_block_index + 1) >> 1]);
        if ((current_block_index & 0x1) == 0x01) {
            next_block_value = next_block_value >> 16;
        }
        next_block_value &= 0xffff;
        if (next_block_value == 0) {
          empty_block_flag = 1;
          buf[i++] = ':';
          if (i >= buflen) {
            return NULL;
          }
          continue; /* move on to next block. */
        }
      } else if (empty_block_flag == 1) {
        /* move on to next block. */
        continue;
      }
    } else if (empty_block_flag == 1) {
      /* Set this flag value so we don't produce multiple empty blocks. */
      empty_block_flag = 2;
    }

    if (current_block_index > 0) {
      buf[i++] = ':';
      if (i >= buflen) {
        return NULL;
      }
    }

    if ((current_block_value & 0xf000) == 0) {
      zero_flag = 1;
    } else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf000) >> 12));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    if (((current_block_value & 0xf00) == 0) && (zero_flag)) {
      /* do nothing */
    } else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf00) >> 8));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    if (((current_block_value & 0xf0) == 0) && (zero_flag)) {
      /* do nothing */
    }
    else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf0) >> 4));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    buf[i++] = lwip_xchar((current_block_value & 0xf));
    if (i >= buflen) {
      return NULL;
    }
  }

  buf[i] = 0;

  return buf;
}

#if LWIP_IPV6_VARIABLE_LENGTH_PREFIX
/* FUNCTION NAME:   ip6_addr_netcmp_prefix
 * PURPOSE:
 *      Compare two IPv6 addresses with a given prefix length.
 *
 * INPUT:
 *      addr1                        -- Pointer to first IPv6 address
 *      addr2                        -- Pointer to second IPv6 address
 *      prefix_len                   -- Prefix length to set
 * OUTPUT:
 *      None
 * RETURN:
 *      1 if the addresses match up to the prefix length, 0 otherwise.
 * NOTES:
 *      None
 */
int
ip6_addr_netcmp_prefix(
    const ip6_addr_t *ptr_addr1,
    const ip6_addr_t *ptr_addr2,
    const u8_t prefix_len)
{
  if (prefix_len == ip6_addr_netcmp_zoneless_prefix(ptr_addr1, ptr_addr2, prefix_len))
  {
    return 1;
  }

  return 0;
}

/* FUNCTION NAME:   ip6_addr_netcmp_zoneless_prefix
 * PURPOSE:
 *      Compare two IPv6 addresses with a given prefix length and
 *      return how many leading bits (from bit 0) are identical,
 *      up to at most prefix_len.
 *
 * INPUT:
 *      ptr_addr1                    -- Pointer to first IPv6 address
 *      ptr_addr2                    -- Pointer to second IPv6 address
 *      prefix_len                   -- Maximum number of prefix bits to compare
 * OUTPUT:
 *      None
 * RETURN:
 *      Number of equal leading bits in [0, prefix_len].
 * NOTES:
 *      - Zone information is ignored.
 *      - If the first differing bit is within prefix_len, the count
 *        stops at that bit.
 */
int
ip6_addr_netcmp_zoneless_prefix(
    const ip6_addr_t *ptr_addr1,
    const ip6_addr_t *ptr_addr2,
    const u8_t prefix_len)
{
  const u8_t *ptr_a1 = (const u8_t *)ptr_addr1;
  const u8_t *ptr_a2 = (const u8_t *)ptr_addr2;
  u8_t full_bytes;
  u8_t rem_bits;
  u8_t matched_bits = 0;
  u8_t i;

  /* IPv6 has 128 bits; clamp prefix_len defensively */
  if (prefix_len > 128) {
    return matched_bits;
  }

  full_bytes = (u8_t)(prefix_len >> 3);
  rem_bits   = (u8_t)(prefix_len & 0x07);

  /* Compare all full bytes first */
  for (i = 0; i < full_bytes; i++) {
    if (ptr_a1[i] == ptr_a2[i]) {
      matched_bits = (u8_t)(matched_bits + 8);
    } else {
      /* Mismatch within this byte: find first differing bit (MSB first) */
      u8_t diff = (u8_t)(ptr_a1[i] ^ ptr_a2[i]);
      u8_t bit;
      for (bit = 0; bit < 8; bit++) {
        if (diff & (u8_t)(0x80U >> bit)) {
          return (u8_t)(matched_bits + bit);
        }
      }
      /* Should not reach here since diff != 0 when ptr_a1[i] != ptr_a2[i] */
      return (u8_t)(matched_bits + 8);
    }
  }

  /* Now handle the remaining (partial) bits, if any */
  if (rem_bits != 0) {
    u8_t mask = (u8_t)(0xFFU << (8U - rem_bits));
    u8_t b1   = (u8_t)(ptr_a1[full_bytes] & mask);
    u8_t b2   = (u8_t)(ptr_a2[full_bytes] & mask);

    if (b1 == b2) {
      /* All remaining bits match */
      matched_bits = (u8_t)(matched_bits + rem_bits);
    } else {
      /* Mismatch somewhere in the remaining prefix bits, find first differing bit */
      u8_t diff = (u8_t)((ptr_a1[full_bytes] ^ ptr_a2[full_bytes]) & mask);
      u8_t bit;
      for (bit = 0; bit < rem_bits; bit++) {
        if (diff & (u8_t)(0x80U >> bit)) {
          return (u8_t)(matched_bits + bit);
        }
      }
      /* Should not reach here since mask mismatch implies diff != 0 */
    }
  }

  return matched_bits;
}

/* FUNCTION NAME:   ip6_addr_nethostcmp_prefix
 * PURPOSE:
 *      Compare the host part (the lower bits) of two IPv6 addresses.
 *      According to the given remaining_prefix value, return how many
 *      of the last remaining_prefix bits are identical.
 *
 * INPUT:
 *      ptr_addr1             -- Pointer to first IPv6 address
 *      ptr_addr2             -- Pointer to second IPv6 address
 *      prefix_len            -- Number of bits (from the least significant side)
 *                               to compare, 0..128. For example, if remaining_prefix
 *                               is 10, this function compares the lowest 10 bits
 *                               of the two addresses (i.e., bits 119..128 in
 *                               1-based numbering).
 * OUTPUT:
 *      None
 * RETURN:
 *      Number of equal bits in the host part, in the range [0, remaining_prefix].
 * NOTES:
 *      None
 */
int
ip6_addr_nethostcmp_prefix(
    const ip6_addr_t *ptr_addr1,
    const ip6_addr_t *ptr_addr2,
    const u8_t prefix_len)
{
  const u8_t *ptr_a1 = (const u8_t *)ptr_addr1;
  const u8_t *ptr_a2 = (const u8_t *)ptr_addr2;
  u8_t max_bits;
  u8_t bit;
  int matched = 0;
  u8_t remaining_prefix = 128 - prefix_len;

  if (remaining_prefix == 0)
  {
    return 0;
  }

  /* IPv6 address length is 128 bits */
  if (remaining_prefix > 128)
  {
    max_bits = 128;
  }
  else
  {
    max_bits = remaining_prefix;
  }

  /* Compare from least significant bit upwards */
  for (bit = 0; bit < max_bits; bit++)
  {
    /* Map bit index (0 = least significant bit) to byte/bit position.
     * IPv6 address is stored in network order: byte 0 = most significant,
     * byte 15 = least significant. */
    u8_t byte_index = (u8_t)(15 - (bit >> 3));      /* 15 .. 0 */
    u8_t bit_mask   = (u8_t)(1U << (bit & 0x07));   /* bit 0..7 in that byte */

    if (((u8_t)(ptr_a1[byte_index] ^ ptr_a2[byte_index]) & bit_mask) != 0)
    {
      /* First differing bit in the host part */
      break;
    }

    matched++;
  }

  return matched;
}
#endif /* LWIP_IPV6_VARIABLE_LENGTH_PREFIX */

#endif /* LWIP_IPV6 */
