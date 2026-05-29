#ifndef __NDS32_UTIL_H__
#define __NDS32_UTIL_H__

#include <nds32_intrinsic.h>

#define get_pfm_cnt0 __nds32__mfsr(NDS32_SR_PFMC0)
#define get_pfm_cnt1 __nds32__mfsr(NDS32_SR_PFMC1)
#define get_pfm_cnt2 __nds32__mfsr(NDS32_SR_PFMC2)

extern void startPFM(unsigned int counter, unsigned int event);
extern void stopPFM(unsigned int counter);

unsigned int io_read32(unsigned int addr);
void io_write32(unsigned int addr, unsigned int vlaue);
unsigned char ramRead8(unsigned int addr);
void ramWrite8(unsigned int addr, unsigned char vlaue);
unsigned int ramRead32(unsigned int addr);
void ramWrite32(unsigned int addr, unsigned int vlaue);
unsigned char io_read8(unsigned int addr);
void io_write8(unsigned int reg, unsigned char vlaue);
unsigned short io_read16(unsigned int addr);
void io_write16(unsigned int reg, unsigned short vlaue);
#endif

