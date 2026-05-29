/**
 * @file
 * Packet buffer management
 */

#include <stdio.h>
#include <stdlib.h>
#include "pbuf.h"

extern void *pvPortMalloc( size_t xWantedSize );
extern void vPortFree( void *pv );

static void pbuf_init_alloced_pbuf(
    struct pbuf *p,
    void *payload,
    unsigned short len)
{
    p->payload = payload;
    p->len = len;

    p->stag_hdr.rx_hdr.rsn = 0;
    p->stag_hdr.rx_hdr.vpm = 0;
    p->stag_hdr.rx_hdr.sp = 0;
    p->stag_hdr.rx_hdr.pcp = 0;
    p->stag_hdr.rx_hdr.dei = 0;
    p->stag_hdr.rx_hdr.vid = 0;
    p->vlan_num = 0;
    p->vlan_hdr.tpid = 0;
    p->vlan_hdr.priority = 0;
    p->vlan_hdr.cfi = 0;
    p->vlan_hdr.vid = 0;
}

struct pbuf* pbuf_alloc(unsigned short length)
{
    struct pbuf *p;

    /* modify reference to case PBUF_RAM */
    unsigned short payload_len = (unsigned short)(MEM_ALIGN_SIZE(length));
    unsigned short alloc_len = (unsigned short)(MEM_ALIGN_SIZE(SIZEOF_STRUCT_PBUF) + payload_len);

    p = (struct pbuf *) pvPortMalloc(alloc_len);
    if (p == NULL) {
        return NULL;
    }

    pbuf_init_alloced_pbuf(p, MEM_ALIGN((void *)((unsigned char *)p + SIZEOF_STRUCT_PBUF)), length);

    return p;
}

unsigned char pbuf_free(struct pbuf *p)
{
    if (p == NULL)
    {
        return 0;
    }

    vPortFree(p);
    return 1;
}


unsigned char pbuf_add_header(struct pbuf *p, size_t header_size_increment)
{
    void *payload;
    unsigned short increment_magnitude;

    if ((p == NULL) || (header_size_increment > 0xFFFF))
    {
        return 1;
    }
    if (header_size_increment == 0)
    {
        return 0;
    }

    increment_magnitude = (unsigned short)header_size_increment;
    /* Do not allow tot_len to wrap as a result. */
    if ((unsigned short)(increment_magnitude + p->len) < increment_magnitude) {
        return 1;
    }

    /* set new payload pointer */
    payload = (unsigned char *)p->payload - header_size_increment;
    /* boundary check fails? */
    if ((unsigned char *)payload < (unsigned char *)p + SIZEOF_STRUCT_PBUF)
    {
        return 1;
    }

    /* modify pbuf fields */
    p->payload = payload;
    p->len = (unsigned short)(p->len + increment_magnitude);

    return 0;
}

