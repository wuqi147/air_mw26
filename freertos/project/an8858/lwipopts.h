/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#ifdef LWIP_CONFIG_FILE
#include LWIP_CONFIG_FILE
#else /* LWIP_CONFIG_FILE */
/* Functions used to obtain and release exclusive access to the Tx buffer.  The
Get function will block if the Tx buffer is not available - use with care! */
signed char *pcLwipBlockingGetTxBuffer( void );
void vLwipAppsReleaseTxBuffer( void );


#define NO_SYS    (0)

/* SNMP options */
#ifdef AIR_SUPPORT_SNMP
#define LWIP_SNMP                       (1)
#else
#define LWIP_SNMP                       (0)
#endif /* AIR_SUPPORT_SNMP */
#if LWIP_SNMP
#define SNMP_USE_NETCONN                (1)
#define SNMP_USE_RAW                    (0)
#if SNMP_USE_NETCONN
#define SNMP_THREAD_PRIO                (2)
#define SNMP_STACK_SIZE                 (configMINIMAL_STACK_SIZE * 4)
#define DEFAULT_UDP_RECVMBOX_SIZE       (5)
#define RECV_BUFSIZE_DEFAULT            (2048)
#endif /* SNMP_USE_NETCONN */
#define SNMP_OUTBOUND_PBUF_SIZE         (512)

#define MIB2_STATS                      (1)
#endif /* LWIP_SNMP */

/* HTTPD options */
#ifdef AIR_SUPPORT_HTTPD
#define LWIP_HTTPD                      (1)
#endif
#define LWIP_HTTPD_UPLOAD_MSG           (1)
#define LWIP_HTTPD_CGI                  (1)
#define LWIP_HTTPD_SSI                  (1)
#define LWIP_HTTPD_SUPPORT_POST         (1)
#define LWIP_HTTPD_SUPPORT_11_KEEPALIVE (1)
#define LWIP_HTTPD_SSI_INCLUDE_TAG      (0)
#define LWIP_HTTPD_MAX_TAG_NAME_LEN     (20)
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN   (1024)
/* When HTTPS is enabled, (LWIP_HTTPD_MAX_SESSION_LEN + (HTTP header size)) should
 * be no more than MBEDTLS_SSL_MAX_CONTENT_LEN. The HTTP header size depends on the
 * browser used and is almost 1K currently. Therefore, set it 1K less than
 * MBEDTLS_SSL_MAX_CONTENT_LEN.
 */
#define LWIP_HTTPD_MAX_SESSION_LEN      (3 * 1024)
#define LWIP_HTTPD_SSI_MULTIPART    (1)

/* TLS options */
#ifdef ENABLE_TLS
#define LWIP_ALTCP    (1)
#define LWIP_ALTCP_TLS    (1)
#define ALTCP_MBEDTLS_CIPHERSUITES_CONFIG    (0)
#endif
/* HTTPDS options. */
#ifdef ENABLE_HTTPS
#define HTTPD_ENABLE_HTTPS              (1)
#define TCP_HTTPS_MSL                   (5)
#else
#define HTTPD_ENABLE_HTTPS              (0)
#endif

/* IGMP options. */
#define LWIP_IGMP    (0)

/* IGMP Snooping options. */
#define LWIP_IGMPSNP    (1)

/* DNS options. */
#if defined(AIR_WEBSOCKET_DEMO) || defined(AIR_MW_SUPPORT)
#define LWIP_DNS    (1)
#endif

#if defined(LWIP_DNS) && !defined(LWIP_MPU_COMPATIBLE) && !defined(DNS_LOCAL_HOSTLIST)
#define LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR    (1)
#define LWIP_DNS_MAX_IP_ADDR_NUM             (5)
#endif

/* SNTP options */
#define LWIP_DHCP_MAX_NTP_SERVERS           (3)

/* Socket options */
#define LWIP_SOCKET    (NO_SYS==0)
#define LWIP_NETCONN    (1)
#define LWIP_SO_SNDTIMEO    (1)
#define LWIP_SO_RCVTIMEO    (1)
#define LWIP_SO_RCVBUF    (1)
#define LWIP_SO_LINGER    (1)
#ifdef AIR_SUPPORT_SSH
#define SO_REUSE (1)
#endif
/* TCP options */
#define LWIP_TCP_KEEPALIVE    (1)
#define TCPIP_THREAD_NAME    "tcpip"
#define TCPIP_THREAD_PRIO    configLWIP_TASK_PRIORITY
#if HTTPD_ENABLE_HTTPS
#define TCPIP_THREAD_STACKSIZE (configMINIMAL_STACK_SIZE * 5)
#define MAX_HTTPS_CONNECTIONS   (4)
#else
#define TCPIP_THREAD_STACKSIZE (configMINIMAL_STACK_SIZE * 4)
#endif
#define DEFAULT_RAW_RECVMBOX_SIZE    (5)
#define DEFAULT_TCP_RECVMBOX_SIZE    (5)
#define DEFAULT_ACCEPTMBOX_SIZE    (5)
#define TCPIP_MBOX_SIZE    (10)
#ifdef AIR_SUPPORT_SSH
#define TCP_LISTEN_BACKLOG    (1)
#else
#define TCP_LISTEN_BACKLOG    (0)
#endif
#define LWIP_TCP_FIN_WAIT_TIMEOUT   (3000)
#define LWIP_TCP_SYN_RCVD_TIMEOUT   (3000)

/* ICMP options */
#define LWIP_ICMP    (1)

/* IPv4 options */
#define LWIP_IPV4    (1)
#define LWIP_NETIF_EXT_STATUS_CALLBACK    (1)
#define LWIP_NETIF_API    (1)

/* IPv6 options */
#ifdef AIR_SUPPORT_IPV6
#define LWIP_IPV6    (1)
extern const void* mw_ipv6_get_gateway_hook(void *netif, const void *dest);
#define LWIP_HOOK_ND6_GET_GW(netif, dest) ((ip6_addr_t *)mw_ipv6_get_gateway_hook(netif, dest))
#else
#define LWIP_IPV6    (0)
#endif
#if LWIP_IPV6
#define LWIP_IPV6_DUP_DETECT_ATTEMPTS   (3)
#define LWIP_IPV6_FRAG    (0)
#define LWIP_IPV6_REASS    (0)
/*The following line is add for ND6 temp test*/
#define LWIP_ND6_REDIRECT_ENABLE (0)
/* end of adding
*/
#define LWIP_IPV6_VARIABLE_LENGTH_PREFIX     (1)
#endif /* LWIP_IPV6 */

/* Loopback options */
#define LWIP_HAVE_LOOPIF    (0)
#define LWIP_NETIF_LOOPBACK    (1)

/* Debug options */
/* Uncomment the line below to enable LWIP debug logs. */
/* #define LWIP_DEBUG */
#ifdef LWIP_DEBUG
#define LWIP_DBG_MIN_LEVEL         0
#define PPP_DEBUG                  LWIP_DBG_ON
#define MEM_DEBUG                  LWIP_DBG_ON
#define MEMP_DEBUG                 LWIP_DBG_ON
#define PBUF_DEBUG                 LWIP_DBG_ON
#define PBUF_PARSE_DEBUG           LWIP_DBG_ON
#define API_LIB_DEBUG              LWIP_DBG_ON
#define API_MSG_DEBUG              LWIP_DBG_ON
#define TCPIP_DEBUG                LWIP_DBG_ON
#define NETIF_DEBUG                LWIP_DBG_ON
#define SOCKETS_DEBUG              LWIP_DBG_ON
#define DNS_DEBUG                  LWIP_DBG_ON
#define AUTOIP_DEBUG               LWIP_DBG_ON
#define DHCP_DEBUG                 LWIP_DBG_ON
#define IP_DEBUG                   LWIP_DBG_ON
#define IP_REASS_DEBUG             LWIP_DBG_ON
#define ICMP_DEBUG                 LWIP_DBG_ON
#define IGMP_DEBUG                 LWIP_DBG_ON
#define UDP_DEBUG                  LWIP_DBG_ON
#define TCP_DEBUG                  LWIP_DBG_ON
#define TCP_INPUT_DEBUG            LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG           LWIP_DBG_ON
#define TCP_RTO_DEBUG              LWIP_DBG_ON
#define TCP_CWND_DEBUG             LWIP_DBG_ON
#define TCP_WND_DEBUG              LWIP_DBG_ON
#define TCP_FR_DEBUG               LWIP_DBG_ON
#define TCP_QLEN_DEBUG             LWIP_DBG_ON
#define TCP_RST_DEBUG              LWIP_DBG_ON
#define HTTPD_DEBUG                LWIP_DBG_ON
#define HTTPD_DB_DEBUG             LWIP_DBG_ON
#define MQTT_DEBUG                 LWIP_DBG_ON
#endif

#define LWIP_DBG_TYPES_ON         (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT)


/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
/* MSVC port: intel processors don't need 4-byte alignment,
   but are faster that way! */
#define MEM_ALIGNMENT            4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                10240

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF            10

/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW "connection". */
#define LWIP_RAW                1
#define MEMP_NUM_RAW_PCB        1

/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#ifdef AIR_SUPPORT_DHCP_SNOOP
#define MEMP_NUM_UDP_PCB_DHCP_SNP    2  /* client/server */
#else
#define MEMP_NUM_UDP_PCB_DHCP_SNP    0
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#define MEMP_NUM_UDP_PCB        (4   \
   + MEMP_NUM_UDP_PCB_DHCP_SNP     \
   )

/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        30

/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 10

/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        10

/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    15

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         2

/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        10

/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
 * for sequential API communication and incoming packets. Used in
 * src/api/tcpip.c.
 * It should be less than or equal to TCPIP_MBOX_SIZE + 1.
 */
#define MEMP_NUM_TCPIP_MSG_API   (10)

/* It should be less than or equal to TCPIP_MBOX_SIZE + 1. */
#define MEMP_NUM_TCPIP_MSG_INPKT (10)

#define MEMP_NUM_ARP_QUEUE        5

/* ---------- Pbuf options ---------- */
/* STAG_INSERT_MODE: the define for special tag insert mode. */
#define STAG_INSERT_MODE        1

/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE            (30)

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE        LWIP_MEM_ALIGN_SIZE(384)

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
 * link level header. No space is reserved for any VLAN header.
 */
#if STAG_INSERT_MODE
#define PBUF_LINK_HLEN            (14 + SIZEOF_STAG_HDR + ETH_PAD_SIZE)
#else
#define PBUF_LINK_HLEN            (14 + ETH_PAD_SIZE)
#endif /* STAG_INSERT_MODE */

/* PBUF_L2_FORWARDING: TX packet for L2 packet forwarding. */
#define PBUF_L2_FORWARDING    1

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    (NO_SYS==0)


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                    255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ            1

/* TCP Maximum segment size. It should be 1440 for IPv6. However,
 * it does not matter because TCP_CALCULATE_EFF_SEND_MSS is enabled.
 */
#define TCP_MSS                    (1460)
#define TCP_CALCULATE_EFF_SEND_MSS              (1)

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF                ( TCP_MSS * 2 )

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (4 * TCP_SND_BUF/TCP_MSS)

/* TCP writable space (bytes). This must be less than or equal
   to TCP_SND_BUF. It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT            (TCP_SND_BUF/2)

/* TCP receive window. */
#define TCP_WND                    ( PBUF_POOL_SIZE * (PBUF_POOL_BUFSIZE - (PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN)) )

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX                12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX            4


/* ---------- ARP options ---------- */
#define LWIP_ARP                1
#define ARP_TABLE_SIZE            10
#define ARP_QUEUEING            1


/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD                0

/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
#define IP_REASSEMBLY            0
#define IP_REASS_MAX_PBUFS        10
#define MEMP_NUM_REASSDATA        10
#define IP_FRAG                    0

#ifdef AIR_SUPPORT_DHCP_SNOOP
/** Some defines for DHCP to let link-layer-addressed packets through while the
 * netif is down.
 * To use this in your own application/protocol, define LWIP_IP_ACCEPT_UDP_PORT(port)
 * to return 1 if the port is accepted and 0 if the port is not accepted.
 */
#define LWIP_IP_ACCEPT_UDP_PORT(port) \
   (((port) == PP_NTOHS(LWIP_IANA_PORT_DHCP_CLIENT)) || ((port) == PP_NTOHS(LWIP_IANA_PORT_DHCP_SERVER)))
#endif /* AIR_SUPPORT_DHCP_SNOOP */

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. */
#define LWIP_DHCP                1

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK        (LWIP_DHCP)


/* ---------- AUTOIP options ------- */
#define LWIP_AUTOIP                0
#define LWIP_DHCP_AUTOIP_COOP    (LWIP_DHCP && LWIP_AUTOIP)


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define LWIP_UDPLITE            1
#define UDP_TTL                    255

/* ---------- Statistics options ---------- */

#define LWIP_STATS                1
#define LWIP_STATS_DISPLAY        1

#if LWIP_STATS
    #define LINK_STATS                1
    #define IP_STATS                1
    #define ICMP_STATS                0
    #define IGMP_STATS                0
    #define IGMPSNP_STATS                1
    #define IPFRAG_STATS            0
    #define UDP_STATS                1
    #define TCP_STATS                1
    #define MEM_STATS                1
    #define MEMP_STATS                1
    #define PBUF_STATS                1
    #define SYS_STATS                1
#endif /* LWIP_STATS */


/* ---------- PPP options ---------- */

#define PPP_SUPPORT             0      /* Set > 0 for PPP */

#if PPP_SUPPORT

    #define NUM_PPP                    1      /* Max PPP sessions. */

    /* Select modules to enable.  Ideally these would be set in the makefile but
     * we're limited by the command line length so you need to modify the settings
     * in this file.
     */
    #define PPPOE_SUPPORT            1
    #define PPPOS_SUPPORT            1
    #define PAP_SUPPORT                1      /* Set > 0 for PAP. */
    #define CHAP_SUPPORT            1      /* Set > 0 for CHAP. */
    #define MSCHAP_SUPPORT            0      /* Set > 0 for MSCHAP (NOT FUNCTIONAL!) */
    #define CBCP_SUPPORT            0      /* Set > 0 for CBCP (NOT FUNCTIONAL!) */
    #define CCP_SUPPORT                0      /* Set > 0 for CCP (NOT FUNCTIONAL!) */
    #define VJ_SUPPORT                1      /* Set > 0 for VJ header compression. */
    #define MD5_SUPPORT                1      /* Set > 0 for MD5 (see also CHAP) */

#endif /* PPP_SUPPORT */
#endif /* LWIP_CONFIG_FILE */
#endif /* __LWIPOPTS_H__ */
