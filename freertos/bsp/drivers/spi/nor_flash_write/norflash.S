#include <platform.h>

#define SPI_CLK_SPEED  (0x10005044)
#define SPI_READ_MODE  (0x10003000)
#define SPI_SIDLY      (0x10003008)
#define SPI_PADCTL     (0x1000309c)

    .global spiclk_speedup_entry
    .global spiclk_speedup_exit

spiclk_speedup_entry:
    li   r0, SPI_CLK_SPEED
    li   r1, SPI_SPEED
    swi  r1, [r0]
    li   r0, SPI_READ_MODE
    li   r1, SPI_READ_CMD
    swi  r1, [r0]
    li   r0, SPI_SIDLY
    li   r1, SPI_DELAY_CELL
    swi  r1, [r0]
    li   r0, SPI_PADCTL
    li   r1, SPI_CLOCK_EDGE
    swi  r1, [r0]
    ret5 lp
spiclk_speedup_exit: