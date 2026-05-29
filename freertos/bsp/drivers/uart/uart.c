
#include <platform.h>

#include "FreeRTOS.h"
#include "queue.h"

#define CR_UART_RBR         (0x00+CR_UART_BASE+0)
#define CR_UART_THR         (0x00+CR_UART_BASE+0)
#define CR_UART_IER         (0x04+CR_UART_BASE+0)
#define CR_UART_IIR         (0x08+CR_UART_BASE+0)
#define CR_UART_FCR         (0x08+CR_UART_BASE+0)
#define CR_UART_LCR         (0x0c+CR_UART_BASE+0)
#define CR_UART_MCR         (0x10+CR_UART_BASE+0)
#define CR_UART_LSR         (0x14+CR_UART_BASE+0)
#define CR_UART_MSR         (0x18+CR_UART_BASE+0)
#define CR_UART_SCR         (0x1c+CR_UART_BASE+0)
#define CR_UART_BRDL        (0x00+CR_UART_BASE+0)
#define CR_UART_BRDH        (0x04+CR_UART_BASE+0)
#define CR_UART_WORDA       (0x20+CR_UART_BASE+0x00)
#define CR_UART_HWORDA      (0x28+CR_UART_BASE+0x00)
#define CR_UART_MISCC       (0x24+CR_UART_BASE+0)
#define CR_UART_XYD         (0x2c+CR_UART_BASE)

#define UART_BRD_ACCESS     0x80
#define UART_XYD_Y          65000
#define UART_XYD_X          59904

#define UART_UCLK_115200    0
#define UART_UCLK_57600     1
#define UART_UCLK_38400     2
#define UART_UCLK_28800     3
#define UART_UCLK_19200     4
#define UART_UCLK_14400     5
#define UART_UCLK_9600      6
#define UART_UCLK_4800      7
#define UART_UCLK_2400      8
#define UART_UCLK_1200      9
#define UART_UCLK_600       10
#define UART_UCLK_300       11
#define UART_UCLK_110       12
#define UART_BRDL           0x03
#define UART_BRDH           0x00
#define UART_BRDL_20M       0x01
#define UART_BRDH_20M       0x00
#define UART_LCR            0x03
#define FIFOMEN_EN          0x1
#define FIFOMEN_DIS         0x0
#define WMSET_1BYTE         (0x0<<6)
#define WMSET_4BYTE         (0x1<<6)
#define UART_MCR            0x0
#define UART_MISCC          0x0
#define UART_IER            0x01

#define IER_RECEIVED_DATA_INTERRUPT_ENABLE  0x01
#define IER_THRE_INTERRUPT_ENABLE           0x02
#define IER_LINE_STATUS_INTERRUPT_ENABLE    0x04

#define IIR_RECEIVED_LINE_STATUS            0x06
#define IIR_RECEIVED_DATA_AVAILABLE         0x04
#define IIR_RECEIVER_IDLE_TRIGGER           0x0C
#define IIR_TRANSMITTED_REGISTER_EMPTY      0x02
#define LSR_RECEIVED_DATA_READY             0x01
#define LSR_OVERRUN                         0x02
#define LSR_PARITY_ERROR                    0x04
#define LSR_FRAME_ERROR                     0x08
#define LSR_BREAK                           0x10
#define LSR_THRE                            0x20
#define LSR_THE                             0x40
#define LSR_RFIFO_FLAG                      0x80

typedef void(*uart_input_callback)(char c);
uart_input_callback uart_input_cb = NULL;

void uart_register_input_cb(uart_input_callback cb)
{
    uart_input_cb = cb;
}

void serial_outc(char c)
{
    while (!((io_read32(CR_UART_LSR)) & LSR_THRE)) ;

    io_write32(CR_UART_THR, c);
#ifdef SYS_LOG_2_FLASH
    if( c != '\r' )
    {
        sys_log_2_flash__write(c);
    }
#endif
}

char serial_inc(void)
{
    unsigned char c;

    while (!(io_read8(CR_UART_LSR) & LSR_RECEIVED_DATA_READY));
    c = io_read8(CR_UART_RBR);

    return (char)c;
}

int serial_tstc(void)
{
    return (io_read8(CR_UART_LSR) & LSR_RECEIVED_DATA_READY);
}

void air_uart_isr (void)
{
    unsigned char ch[4];
    int index=0, i;

    record_interrupts(IRQ_UART);

    while ((io_read8(CR_UART_LSR)) & LSR_RECEIVED_DATA_READY)
    {
        ch[index] = io_read8(CR_UART_RBR);
        index++;

        if (index >= 4)
        {
            break;
        }
    }

    //send char to cmd recv task
    for(i=0; i< index; i++)
    {
        if (NULL != uart_input_cb)
        {
            uart_input_cb(ch[i]);
        }
    }
    return;
}

void uart_init(void)
{
    unsigned long   div_x, div_y;
    unsigned long   word;

    // Disable RFIFO mode , set watermark=0x00 (1 byte)
    io_write32(CR_UART_FCR, (FIFOMEN_EN|WMSET_4BYTE));

    // Set modem control to 0
    io_write32(CR_UART_MCR, UART_MCR);

    // Disable IRDA, Disable Power Saving Mode, RTS , CTS flow control
    io_write32(CR_UART_MISCC, UART_MISCC);

    // Set interrupt Enable to, enable Tx, Rx and Line status
    io_write32(CR_UART_IER, UART_IER);

    /* access the bardrate divider */

    io_write32(CR_UART_LCR, UART_BRD_ACCESS);

    div_y = (UART_XYD_Y);
    div_x = (UART_XYD_X);
    word = (div_x<<16)|div_y;
    io_write32(CR_UART_XYD, word);

    /* Set Baud Rate Divisor to 1*16 */
    io_write32(CR_UART_BRDL, UART_BRDL_20M); /* for baud rate: 11520*/
    io_write32(CR_UART_BRDH, UART_BRDH_20M);

    /* Set DLAB = 0, clength = 8, stop =1, no parity check  */
    io_write32(CR_UART_LCR, UART_LCR);

    register_isr(IRQ_UART, air_uart_isr);

    return;
}

int outbyte(int c)
{
    serial_outc(c);
    if (c =='\n')
        serial_outc('\r');
    return c;
}

