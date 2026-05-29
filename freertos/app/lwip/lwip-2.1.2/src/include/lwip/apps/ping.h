#ifndef LWIP_PING_H
#define LWIP_PING_H

#include "lwip/ip_addr.h"
#ifdef AIR_SUPPORT_ICMP_CLIENT
#include "icmp_client.h"
#endif /* AIR_SUPPORT_ICMP_CLIENT */

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif

#ifdef AIR_SUPPORT_ICMP_CLIENT
typedef struct ping_info_s {
    u16_t send_packets;
    u16_t recv_packets;
    u16_t lost_packets;
    u16_t min_rtt;
    u16_t max_rtt;
    u16_t ave_rtt;
    u16_t status;
    u16_t ping_num;
    u32_t sum_rtt;
    u8_t  loss_ratio;
}ping_info_t;
extern ping_info_t ping_info;
extern ip_addr_t* ping_target;

#if PING_USE_SOCKETS
void ping_thread(void *arg);
int ping_get_socket(
  void);
#endif /* PING_USE_SOCKETS */
#endif /* AIR_SUPPORT_ICMP_CLIENT */
void ping_init(const ip_addr_t* ping_addr);

#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#endif /* LWIP_PING_H */
