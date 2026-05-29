#include <stdarg.h>
#include "flash_util.h"

#define HIGH_BIT_UNC        (0x1<<30)

#define CR_UART_BASE        (0x1000A000)
#define CR_UART_THR         (0x00+CR_UART_BASE+0)
#define CR_UART_LSR         (0x14+CR_UART_BASE+0)
#define LSR_THRE            (0x20)

unsigned int onram_io_read32(unsigned int addr)
{
    addr |= HIGH_BIT_UNC;
    return VPint(addr);
}

void onram_io_write32(unsigned int addr, unsigned int value)
{
    addr |= HIGH_BIT_UNC;
    VPint(addr) = value;
}

static void onram_serial_outc(char c)
{
    while (!((onram_io_read32(CR_UART_LSR)) & LSR_THRE)) ;

    onram_io_write32(CR_UART_THR, c);
}

int onram_outbyte(char c)
{
    onram_serial_outc(c);
    if (c =='\n')
        onram_serial_outc('\r');
    return c;
}

/* for simple_printf*/
static char* uitoa(unsigned int num, int base, char *str)
{
    int idx = 11;
    int remain = 0;
    str[11] = 0;

    while(num)
    {
        remain = num % base;

        if(remain > 9)
        {
            str[--idx] = 'a' + (remain - 10);
        }
        else
        {
            str[--idx] = '0' + remain;
        }

        num = num / base;
    }

    if(11 == idx)
        str[--idx] = '0';

    return &str[idx];
}

static unsigned int onram_strlen(const char *s)
{
    unsigned int len = 0;

    while (*s != '\0')
    {
        ++len;
        ++s;
    }

    return len;
}

int simple_printf(const char *fmt, ...)
{
    va_list args;
    const char *p;
    char sbuf[12];
    char *sval;
    unsigned int ival;
    unsigned int len;
    int idx;

    va_start(args, fmt);

    for(p = fmt; *p; p++)
    {
        if(*p != '%')
        {
            onram_outbyte(*p);
            continue;
        }

        switch (*++p)
        {
            case 'u':
                ival = va_arg(args, unsigned int);
                sval = uitoa(ival, 10, sbuf);

                len = onram_strlen(sval);
                for(idx = 0; idx <len; ++idx)
                    onram_outbyte(sval[idx]);

                break;

            case 's':
                sval = va_arg(args, char *);

                len = onram_strlen(sval);
                for(idx = 0; idx <len; ++idx)
                    onram_outbyte(sval[idx]);

                break;

            case 'x':
                ival = va_arg(args, unsigned int);
                sval = uitoa(ival, 16, sbuf);

                len = onram_strlen(sval);
                for(idx = 0; idx <len; ++idx)
                    onram_outbyte(sval[idx]);

                break;

            default:
                onram_outbyte(*p);
                break;
        }
    }

    va_end(args);

    return 0;
}

void onram_memcpy(void *dst, void *src, unsigned int n)
{
    char *cur_src = (char *)src;
    char *cur_dst = (char *)dst;
    int i;
    for (i = 0; i < n; ++i)
        cur_dst[i] = cur_src[i];
}