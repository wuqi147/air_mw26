/**
 * @file
 * pbuf Header
 */

#ifndef PBUF_H
#define PBUF_H

#include "stddef.h"

/* MACRO FUNCTION DECLARATIONS
 */
#define ETH_HWADDR_LEN      6

#define MEM_ALIGNMENT       4
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1U) & ~(MEM_ALIGNMENT - 1U))
#define MEM_ALIGN(addr) ((void *)(((size_t)(addr) + MEM_ALIGNMENT - 1) & ~(size_t)(MEM_ALIGNMENT-1)))

#define SIZEOF_ETH_TYPE     (2)
#define SIZEOF_STAG_HDR     (4)
#define SIZEOF_VLAN_HDR     (4)
#define SIZEOF_ETH_HDR      (14)
#define SIZEOF_STRUCT_PBUF  MEM_ALIGN_SIZE(sizeof(struct pbuf))


#if defined (BIG_ENDIAN)
#define htons(x) ((unsigned short)(x))
#define ntohs(x) ((unsigned short)(x))
#define htonl(x) ((unsigned int)(x))
#define ntohl(x) ((unsigned int)(x))

#else /* LITTLE_ENDIAN */

#ifndef htons
#define htons(x) ((unsigned short)((((x) & (unsigned short)0x00ffU) << 8) | (((x) & (unsigned short)0xff00U) >> 8)))
#endif
#ifndef ntohs
#define ntohs(x) htons(x)
#endif
#ifndef htonl
#define htonl(x) ((((x) & (unsigned long)0x000000ffUL) << 24) | \
                 (((x) & (unsigned long)0x0000ff00UL) <<  8) | \
                 (((x) & (unsigned long)0x00ff0000UL) >>  8) | \
                 (((x) & (unsigned long)0xff000000UL) >> 24))
#endif
#ifndef ntohl
#define ntohl(x) htonl(x)
#endif

#endif


/* DATA TYPE DECLARATIONS
 */
/* Ethernet MAC address */
struct eth_addr
{
    unsigned char addr[ETH_HWADDR_LEN];
} __attribute__((packed));

/* Ethernet header */
struct eth_hdr
{
    struct eth_addr dest;
    struct eth_addr src;
    unsigned short type;
} __attribute__((packed));

/* RX STAG */
struct eth_rx_stag_hdr
{
    unsigned long rsn:3;
    unsigned long vpm:2;
    unsigned long sp:5;
    unsigned long unused:6;
    unsigned long pcp:3;
    unsigned long dei:1;
    unsigned long vid:12;
} __attribute__((packed));

/* TX STAG */
struct eth_tx_stag_hdr
{
    unsigned char mode;
    union
    {
        struct
        {
            unsigned long opc:3;
            unsigned long dp:29;
        } tx_stag_insert;

        struct
        {
            unsigned short opc:3;
            unsigned short vpm:2;
            unsigned short dp:11;
        } tx_stag_replace;
    };
} __attribute__((packed));

/* VLAN STAG */
struct eth_vlan_hdr
{
    unsigned short tpid;
    unsigned short priority:3;
    unsigned short cfi:1;
    unsigned short vid:12;
} __attribute__((packed));


struct pbuf
{
    /** pointer to the actual data in the buffer */
    void *payload;

    /** length of this buffer */
    unsigned short len;

    /** ethernet header */
    struct eth_hdr ether_hdr;

    /** special tag rx/tx */
    union
    {
        struct eth_rx_stag_hdr rx_hdr;
        struct eth_tx_stag_hdr tx_hdr;
    } stag_hdr;

    /** vlan header infomation */
    unsigned char vlan_num;
    struct eth_vlan_hdr vlan_hdr;
};

typedef enum
{
  STAG_HDR_MODE_START = 0,
  STAG_HDR_MODE_REPLACE,
  STAG_HDR_MODE_INSERT,
  STAG_HDR_MODE_LAST
} STAG_HDR_MODE_T;


struct pbuf* pbuf_alloc(unsigned short length);
unsigned char pbuf_free(struct pbuf *p);
unsigned char pbuf_add_header(struct pbuf *p, size_t header_size_increment);

#endif /* PBUF_H */
