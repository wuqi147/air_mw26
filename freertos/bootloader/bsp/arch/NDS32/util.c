#include <stdio.h>
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>
#include "util.h"

#define PFM_CTL_EN0     (1<<0)
#define PFM_CTL_EN1     (1<<1)
#define PFM_CTL_EN2     (1<<2)
#define PFM_CTL_IE0     (1<<3)
#define PFM_CTL_IE1     (1<<4)
#define PFM_CTL_IE2     (1<<5)
#define PFM_CTL_OVF0    (1<<6)
#define PFM_CTL_OVF1    (1<<7)
#define PFM_CTL_OVF2    (1<<8)
#define PFM_CTL_KS0     (1<<9)
#define PFM_CTL_KS1     (1<<10)
#define PFM_CTL_KS2     (1<<11)
#define PFM_CTL_KU0     (1<<12)
#define PFM_CTL_KU1     (1<<13)
#define PFM_CTL_KU2     (1<<14)
#define PFM_CTL_SEL0_BIT (15)
#define PFM_CTL_SEL1_BIT (16)
#define PFM_CTL_SEL2_BIT (22)


unsigned char io_read8(unsigned int addr)
{
    addr |= HIGH_BIT_UNC;
    return VPchar(addr);
}

void io_write8(unsigned int reg, unsigned char vlaue)
{
    reg |= HIGH_BIT_UNC;
    VPchar(reg) = vlaue;
}

unsigned short io_read16(unsigned int addr)
{
    addr |= HIGH_BIT_UNC;
    return VPshort(addr);
}

void io_write16(unsigned int reg, unsigned short vlaue)
{
    reg |= HIGH_BIT_UNC;
    VPshort(reg) = vlaue;
}

unsigned int io_read32(unsigned int addr)
{
    addr |= HIGH_BIT_UNC;
    return VPint(addr);
}

void io_write32(unsigned int addr, unsigned int vlaue)
{
    addr |= HIGH_BIT_UNC;
    VPint(addr) = vlaue;
}

unsigned char ramRead8(unsigned int addr)
{
    return VPchar(addr);
}

void ramWrite8(unsigned int addr, unsigned char vlaue)
{
    VPchar(addr) = vlaue;
}

unsigned int ramRead32(unsigned int addr)
{
    return VPint(addr);
}

void ramWrite32(unsigned int addr, unsigned int vlaue)
{
    VPint(addr) = vlaue;
}

#if 0
void *air_memcpy(void *dst, void *src, unsigned int size)
{
    int i;
    char *d = dst;
    char *s = src;

    for (i=0; i<size; i++,s++,d++) {
        VPchar(d) = VPchar(s);
    }

    return dst;
}
#endif
void startPFM(unsigned int counter, unsigned int event)
{
    unsigned int ctrl = 0, msc_cfg = 0;

    if (counter>2) {
        printf("Wrong counter:%d! Should be 0,1,2.\n", counter);
        return;
    }

    if (counter==0) {
        if (event>1) {
            printf("Wrong event:%d! Should be 0,1.\n", event);
            return;
        }
    }
    else if (counter==1) {
        if (event>31) {
            printf("Wrong event:%d! Should be 0~31.\n", event);
            return;
        }
    }
    else { /* counter==2*/
        if (event>30) {
            printf("Wrong event:%d! Should be 0~30.\n", event);
            return;
        }
    }

    msc_cfg = __nds32__mfsr(NDS32_SR_MSC_CFG);
    if (!(msc_cfg & 0x4)) {
        printf("This CPU config doesn't have PFM to use.\n");
        return;
    }

    ctrl = __nds32__mfsr(NDS32_SR_PFM_CTL);

    if (counter==0) {
        ctrl &= (~(PFM_CTL_EN0|PFM_CTL_IE0|PFM_CTL_KS0|PFM_CTL_KU0|PFM_CTL_OVF0|(1<<PFM_CTL_SEL0_BIT)));
        ctrl |= (PFM_CTL_EN0|(event<<PFM_CTL_SEL0_BIT));
        __nds32__mtsr_dsb(0, NDS32_SR_PFMC0); /* reset Performence Counter 0 */
    }
    else if (counter==1) {
        ctrl &= (~(PFM_CTL_EN1|PFM_CTL_IE1|PFM_CTL_KS1|PFM_CTL_KU1|PFM_CTL_OVF1|(0x3f<<PFM_CTL_SEL1_BIT)));
        ctrl |= (PFM_CTL_EN1|(event<<PFM_CTL_SEL1_BIT));
        __nds32__mtsr_dsb(0, NDS32_SR_PFMC1); /* reset Performence Counter 1 */
    }
    else { /* counter==2 */
        ctrl &= (~(PFM_CTL_EN2|PFM_CTL_IE2|PFM_CTL_KS2|PFM_CTL_KU2|PFM_CTL_OVF2|(0x3f<<PFM_CTL_SEL2_BIT)));
        ctrl |= (PFM_CTL_EN2|(event<<PFM_CTL_SEL2_BIT));
        __nds32__mtsr_dsb(0, NDS32_SR_PFMC2); /* reset Performence Counter 2 */
    }

    __nds32__mtsr(ctrl, NDS32_SR_PFM_CTL);
    //printf("\nNDS32_SR_PFM_CTL:0x%x\n", __nds32__mfsr(NDS32_SR_PFM_CTL));
    return;
}

void stopPFM(unsigned int counter)
{
    unsigned int ctrl=0, msc_cfg = 0;

    if (counter>2) {
        printf("Wrong counter:%d! Should be 0,1,2.\n", counter);
        return;
    }

    msc_cfg = __nds32__mfsr(NDS32_SR_MSC_CFG);
    if (!(msc_cfg & 0x4)) {
        printf("This CPU config doesn't have pfm to use.\n");
        return;
    }

    ctrl = __nds32__mfsr(NDS32_SR_PFM_CTL);

    if (counter==0) {
        ctrl &= (~(PFM_CTL_EN0));
    }
    else if (counter==1) {
        ctrl &= (~(PFM_CTL_EN1));
    }
    else { /* counter==2 */
        ctrl &= (~(PFM_CTL_EN2));
    }

    __nds32__mtsr(ctrl, NDS32_SR_PFM_CTL);
    return;
}


