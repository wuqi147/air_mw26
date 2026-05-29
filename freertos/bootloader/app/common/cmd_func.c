#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>
#include "util.h"
#include "cmd_func.h"
#include "spinorwrite.h"

#define MAX_WRITE_FLASH_SIZE       (16)
#define FLASH_ADDR_MASK            (0x1fffffff)
#define VALID_START_FLASH_ADDR     (ManufactureBase)
#define VALID_END_FLASH_ADDR       (MainSystemBase)


#define CHECK_HEX(__ptr__)   \
    ('0' != (__ptr__)[0])?0: \
    ('x' == (__ptr__)[1])?1: \
    ('X' == (__ptr__)[1])?1:0

#define GET_KEY_VAL(__tok__, __idx__, __key__, __format__, __value__) do \
    {                                                                    \
        /* check key name */                                             \
        if (E_SYNTAX_OK !=                                               \
            strcmp(__tok__[__idx__], __key__))                           \
        {                                                                \
            printf("key : %s\n", __tok__[__idx__]);                      \
            return E_SYNTAX_ERROR;                                       \
        }                                                                \
        /* get key value */                                              \
        sscanf(__tok__[__idx__+1], __format__, __value__);               \
    } while(0)

static int trans_string_to_bytes(const char *ptr_str, unsigned char *ptr_value, unsigned int *data_len);
static void split_string(char *tokens[], char *cmd);
static void dump_flash_data(unsigned int addr, unsigned int len);

static int trans_string_to_bytes(const char *ptr_str, unsigned char *ptr_value, unsigned int *data_len)
{
    unsigned int idx = 0, ch = 2, shift = 0;
    unsigned char value = 0;
    unsigned int str_len = strlen(ptr_str);

    if(1 == CHECK_HEX(ptr_str))
    {
        for (idx = 2; idx < str_len; idx++)
        {
            if ('.' == ptr_str[idx] || (ch == 0))
            {
                if('.' == ptr_str[idx] && (ch == 1))
                {
                    ptr_value[shift++] = value >> 4;
                    value = 0;
                    ch = 2;
                }
                else
                {
                    ptr_value[shift++] = value;
                    value = 0;
                    ch = 2;
                }
            }

            if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
            {
                value += ((unsigned int)(ptr_str[idx] - '0')) << (4 * (ch-1));
                ch--;
            }
            else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
            {
                value += ((unsigned int)(ptr_str[idx] - 'a' + 10)) << (4 * (ch-1));
                ch--;
            }
            else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
            {
                value += ((unsigned int)(ptr_str[idx] - 'A' + 10)) << (4 * (ch-1));
                ch--;
            }
            else if ('.' == ptr_str[idx])
            {
                continue;
            }
            else
            {
                printf("illegal data value\n");
                return E_SYNTAX_ERROR;
            }
        }
        ptr_value[shift++] = (ch==1)?(value>>4):value;
    }
    else
    {
        printf("data format : <HEX>\n");
        return E_SYNTAX_ERROR;
    }
    *data_len = shift;
    return E_SYNTAX_OK;
}

static void split_string(char *tokens[], char *cmd)
{
    int i = 0, begin = 0, shift = 0;
    int len = strlen(cmd);

    for(i = 0; i < len; i++)
    {
        if((' ' == cmd[i]) || ('=' == cmd[i]))
        {
            if(i > begin)
            {
                tokens[shift++] = cmd + begin;
            }
            cmd[i] = '\0';
            begin = i + 1;
        }
    }
    if(cmd[i] != ' ')
    {
        tokens[shift++] = cmd + begin;
    }
}

static void dump_flash_data(unsigned int addr, unsigned int len)
{
    register int n, m, r, t;
    unsigned char temp[16];

    printf("flash       +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f\n");
    for( n = len; n > 0; )
    {
        t = addr - (addr & 0xfffffff0);
        printf("0x%08x ", addr & 0xfffffff0);
        /*
         * t : current line's offset
         * n : total write length
         * r : write bytes in current line
         *  n = 28, t = 5, r = 16-5 = 11
         *  n = 17, t = 0, r = 16-0 = 16
         *  n = 1 , t = 0, r = 1
         *  flash     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f
         *  xxxx                      x  x  x  x  x  x  x  x  x  x  x
         *  xxxx       x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x
         *  xxxx       x
         */
        r = ((t+n) > 16) ? (16-t) : n;
        memcpy((void *) temp, (void *) addr, r);
        addr += r;
        for( m = 0; m < t; ++m )
        {
            printf("   ");
        }
        for( m = 0; m < r; ++m )
        {
            printf(" %02x", temp[m]);
        }
        n -= r;
        printf("\n");
    }
    printf("\n");
}

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
    printf("0x%08x\t0x%08x\r\n", addr, value);

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

int doSysMemrb_raw(unsigned int addr)
{
    unsigned char value;

    value = ramRead8(addr);

    printf("\r\n<Address>\t<Value>\r\n");
    printf("0x%08x\t0x%02x\r\n", addr, value);

    return 0;
}

int doSysMemwb_raw(unsigned int addr, unsigned char value)
{
    ramWrite8(addr, value);
    return 0;
}

int doSysMemrl_raw(unsigned int addr)
{
    unsigned int value;

    value = ramRead32(addr);

    printf("\r\n<Address>\t<Value>\r\n");
    printf("0x%08x\t0x%08x\r\n", addr, value);

    return 0;
}

int doSysMemwl_raw(unsigned int addr, unsigned int value)
{
    ramWrite32(addr, value);
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

int dump(unsigned int addr, unsigned long len)
{
    register int n, m, c, r;
    unsigned char temp[16];

    printf("\n");

    for( n = len; n > 0; ){

            printf("%08x ", addr);
            r = n < 16? n: 16;
            memcpy_endian((void *) temp, (void *) addr, r);
            addr += r;
            for( m = 0; m < r; ++m ){
                    printf("%c", (m & 3) == 0 && m > 0? '.': ' ');
                    printf("%02x", temp[m]);
            }
            for(; m < 16; ++m )
                    printf("   ");
            printf("  |");
            for( m = 0; m < r; ++m ){
                c = temp[m];
                printf("%c", ' ' <= c && c <= '~'? c: '.');
            }
            n -= r;
            for(; m < 16; ++m )
                    printf(" ");
            printf("|\n");
    }

    printf("\n");
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

int doSysMemory_raw(unsigned int addr, unsigned int len)
{
    dump(addr, len);

    return 0;
}

void cpu_reg_dump (void)
{
    printf("NDS32_SR_PSW:0x%x\n", __nds32__mfsr(NDS32_SR_PSW));
    printf("NDS32_SR_INT_MASK2:0x%x\n", __nds32__mfsr(NDS32_SR_INT_MASK2));
    printf("NDS32_SR_INT_PEND2:0x%x\n", __nds32__mfsr(NDS32_SR_INT_PEND2));
}



int write_flash_data(const char *input)
{
    unsigned int flash_dst_addr, flash_data_len, data_len;
    char *tokens[10] = {0};
    /*prevent stack overflow*/
    static char string_data[64] = {0};
    static unsigned char flash_data_buf[64] = {0};

    memset(string_data, 0, sizeof(string_data));
    memset(flash_data_buf, 0, sizeof(flash_data_buf));
    printf("\n");
    /*
     *e.g. *input = "flash_write  addr=0x1fff0 data-len=1 data=0x11",
     *     *tokens[] = {*+0, *+13, *+18, *+26, *+33, *+35, *+37, *+42}
     */
    split_string(tokens, (char *)input);

    /*
     *compare key and get key value
     *e.g. flash_dst_addr = 0x1fff0, flash_data_len = 1, string_data = "0x11"
     */
    GET_KEY_VAL(tokens, 1, "addr", "%x", &flash_dst_addr);
    GET_KEY_VAL(tokens, 3, "data-len", "%d", &flash_data_len);
    GET_KEY_VAL(tokens, 5, "data", "%s", string_data);

    /*e.g. string_data = "0x11", data_len = 4, flash_data_buf[0] = 0x11*/
    if(trans_string_to_bytes(string_data, flash_data_buf, &data_len))
    {
        return E_SYNTAX_ERROR;
    }

    /*
     *if user key "flash_write  addr=0x1fff0 data-len=1 data=0x11.12"
     *flash_data_len = 1, data_len = 2
     */
    if(flash_data_len != data_len)
    {
        printf("data-len is mismatch with data\n");
        return E_SYNTAX_ERROR;
    }

    if(flash_data_len > MAX_WRITE_FLASH_SIZE || data_len > MAX_WRITE_FLASH_SIZE)
    {
        printf("maximum write length : %d\n", MAX_WRITE_FLASH_SIZE);
        return E_SYNTAX_ERROR;
    }

    /*
     *user can only write flash 0x2001f000~0x20020000
     */
    if(flash_dst_addr >= VALID_START_FLASH_ADDR && (flash_dst_addr+data_len) <= VALID_END_FLASH_ADDR)
    {
        spinor_write((unsigned int)flash_data_buf, flash_dst_addr, data_len);
        return E_SYNTAX_OK;
    }
    else
    {
        printf("valid address range : 0x%08x ~ 0x%08x\n", VALID_START_FLASH_ADDR, VALID_END_FLASH_ADDR);
        return E_SYNTAX_ERROR;
    }
}

int read_flash_data(const char *input)
{
    unsigned int flash_dst_addr, flash_data_len;
    char *tokens[10] = {0};

    printf("\n");
    split_string(tokens, (char *)input);
    GET_KEY_VAL(tokens, 1, "addr", "%x", &flash_dst_addr);
    GET_KEY_VAL(tokens, 3, "data-len", "%d", &flash_data_len);
    /*
     *user can only read flash 0x2001f000~0x20020000
     */
    if(flash_dst_addr >= VALID_START_FLASH_ADDR && (flash_dst_addr+flash_data_len) <= VALID_END_FLASH_ADDR)
    {
        dump_flash_data(flash_dst_addr, flash_data_len);
        return E_SYNTAX_OK;
    }
    else
    {
        printf("valid address range : 0x%08x ~ 0x%08x\n", VALID_START_FLASH_ADDR, VALID_END_FLASH_ADDR);
        return E_SYNTAX_ERROR;
    }
}
