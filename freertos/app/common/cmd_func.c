
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>

int isDmemAddr(unsigned int addr)
{
    if ((addr>=DMEM_BASE) && (addr<(DMEM_BASE+DMEM_SIZE)))
        return 1;
    else
        return 0;
}

int isRegAddr(unsigned int addr)
{
    if ((addr>=REGS_BASE) && (addr<(REGS_BASE+REGS_SIZE)))
        return 1;
    else
        return 0;
}

void errAddrInfo (unsigned int addr)
{
    printf("\nError: addr:0x%08x is wrong\n", addr);
    printf("\tvalid DEME addr: 0x%08x~0x%08x\n", DMEM_BASE, (DMEM_BASE+DMEM_SIZE));
    printf("\tvalid Register addr: 0x%08x~0x%08x\n\n", REGS_BASE, (REGS_BASE+REGS_SIZE));
    return;
}

int isValidAddr(unsigned int addr)
{
    if ((isDmemAddr(addr)==0) && (isRegAddr(addr)==0)) {
        errAddrInfo(addr);
        return 0;
    }
    else
        return 1;
}

int memRead32(unsigned int addr, unsigned int *value_p)
{
    if(isDmemAddr(addr) || isRegAddr(addr)) {
        *value_p = io_read32(addr);
    }
    else {
        errAddrInfo(addr);
        return -1;
    }

    return 0;
}

int doSysMemrl(unsigned int addr)
{
    unsigned int value;

    if (memRead32(addr, &value)==-1)
        return -1;

    printf("\r\n<Address>\t<Value>\r\n");
    printf("0x%08lx\t0x%08lx\r\n", addr, value);

    return 0;
}

int doSysMemwl(unsigned int addr, unsigned int value)
{
    if(isDmemAddr(addr) || isRegAddr(addr)) {
        io_write32(addr, value);
    }
    else {
        errAddrInfo(addr);
        return -1;
    }

    return 0;
}

int doSysModifyBit(unsigned int addr, unsigned int pos, unsigned int isSetBit)
{
    unsigned int value;

    if (memRead32(addr, &value)==-1)
        return -1;

    if(isSetBit)
        value |= (0x1 << pos);
    else
        value &= (~(0x1 << pos));

    io_write32(addr, value);

    return 0;
}

int doSysMemwl2(unsigned int addr, unsigned int val, unsigned int isOrVal)
{
    unsigned int value;

    if (memRead32(addr, &value)==-1)
        return -1;

    if(isOrVal)
        value |= val;
    else
        value &= val;

    io_write32(addr, value);

    return 0;
}

static void * memcpy_endian(void * dest,const void *src, unsigned int count)
{
    char *tmp = (char *) dest, *s = (char *) src;
#if 0 //def __BIG_ENDIAN
    unsigned int offset=0;
#else
    unsigned int offset=3;
#endif

    while (count--) {
        *tmp++ = *(char *)(((unsigned long)s) ^ offset);
        s++;
    }

    return dest;
}

static int dump(unsigned long addr, unsigned long len)
{
    unsigned char *byte = (unsigned char *)addr;
    unsigned int i, j;

    for(i = 0; i < len; i+= 16) {
        printf("%08lx ", addr + i);

        for(j = 0; j < 16; j++) {
            if(i + j < len) {
                printf("%02x ", byte[i + j]);
            } else {
                printf("   ");
            }
        }

        printf(" |");

        for(j = 0; j < 16; j++) {
            if(i + j < len) {
                printf("%c", isprint(byte[i + j]) ? byte[i + j] : '.');
            } else {
                printf(" ");
            }
        }

        printf("|\n");
    }

    return 0;

}


int doSysMemory(unsigned int addr, unsigned int len)
{
    if(isDmemAddr(addr)) {
        addr |= HIGH_BIT_UNC;
    }
    else if (isRegAddr(addr)) {
        addr |= HIGH_BIT_UNC;
    }
    else {
        errAddrInfo(addr);
        return -1;
    }

    dump(addr, len);

    return 0;
}

void cpu_reg_dump (void)
{
    printf("NDS32_SR_PSW:0x%x\n", __nds32__mfsr(NDS32_SR_PSW));
    printf("NDS32_SR_INT_MASK2:0x%x\n", __nds32__mfsr(NDS32_SR_INT_MASK2));
    printf("NDS32_SR_INT_PEND2:0x%x\n", __nds32__mfsr(NDS32_SR_INT_PEND2));
}


