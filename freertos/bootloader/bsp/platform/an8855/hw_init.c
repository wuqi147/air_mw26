#include <stdio.h>
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>
#include <cache.h>
#include "util.h"
#include "interrupt.h"
#include "timer.h"
#include "uart.h"
#if defined(AIR_SUPPORT_ACE)
#include <stdint.h>
#include <string.h>
#include "ace_cmd_info.h"
#endif

#define RG_GPIO_L_PU            (CR_CHIP_SCU_BASE + 0x1c)
#define RG_GPIO_L_PD            (CR_CHIP_SCU_BASE + 0x20)
#define RG_GPIO_UART1_MODE      (CR_CHIP_SCU_BASE + 0x74)
#define RG_RST_CTRL2            (CR_NP_SCU_BASE + 0xc4)

#define TIMER_RST               (0x1)
#define TIMER_RST_OFS           (1)
#define TIMER_RST_EN_MSK        (1 << TIMER_RST_OFS)
#define UART_RST                (0x1)
#define UART_RST_OFS            (0)
#define UART_RST_EN_MSK         (1 << UART_RST_OFS)

#define GPIO0_UART_OFS          (0)
#define GPIO0_UART_MSK          (0x1)
#define GPIO_UART_RX            (0)
#define GPIO_UART_TX            (1)

extern char __data_start, _edata, __sbss_end;

extern void cl22_write(const unsigned char phyAddr, const unsigned char pageSel, const unsigned char regSel, const unsigned short wdata);

void uart_pre_init(void);

void rbus_timeout_check(void)
{
    if ((io_read32(TIMEOUT_STS0) & 0x1) == 1)
    {
        printf("\r\nWARNING !!! Rbus timeout happended !!!\r\n");
        printf("Rbus timeout status:0x%x, errAddr:0x%x\r\n", io_read32(TIMEOUT_STS0), io_read32(TIMEOUT_STS1));
        /* clear the status*/
        io_write32(TIMEOUT_STS0, 0);
        io_write32(TIMEOUT_STS1, 0);
    }
}

void rbus_timeout_init(void)
{
    rbus_timeout_check();

    /* set cmd/wdata/rdata timeout_cnt as 100 ms */
    io_write32(TIMEOUT_CFG0, DMEM_CTRL_CLK/10);
    io_write32(TIMEOUT_CFG1, DMEM_CTRL_CLK/10);
    io_write32(TIMEOUT_CFG2, DMEM_CTRL_CLK/10);

    /* enable timeout.
     * Note: ISR won't work due to data/bss section in DMEM */
    io_write32(TIMEOUT_STS0, 0x80000000);
}

void air_pbus_tout_isr(void)
{
    int isBusTout=0;
    unsigned int errAddr;

    isBusTout = (io_read32(PB_TIMEOUT_INT)&0x1);
    io_write32(PB_TIMEOUT_INT, 1); /* clear intr */

    errAddr = io_read32(PB_TIMEOUT_INFO);

    if (isBusTout)
        printf("\n\npbus timeout interrupt errAddr=%08x\n\n", errAddr);
    else
        printf("\n\nunknown bus timeout interrupt errAddr=%08x\n\n", errAddr);
}

void pbus_timeout_init(void)
{
    unsigned int val;

    register_isr(IRQ_BUS_TOUT, air_pbus_tout_isr);

    val = BUSCLKFREQ/10; /* timeout time: 100 ms */
    io_write32(PB_TIMEOUT_CFG, val);

    val |= (1<<31); /* enable pbus timeout */
    io_write32(PB_TIMEOUT_CFG, val);
}

void rbus_rdbypasswt_init(void)
{
    unsigned int line_size = CACHE_LINE_SIZE(DCACHE);
    unsigned int mask = ALL_FF-(line_size-1);
    unsigned int val;

    /* disable rdbypasswt before setting mask */
    val = io_read32(RDBYPASSWT_CFG);
    val &= (~0x1);
    io_write32(RDBYPASSWT_CFG, val);

    /* set rdbypasswt mask according to dcache line size */
    io_write32(RDBYPASSWT_MASK, mask);

    /* enable rdbypasswt.
     * Note: don't change cmd_fifo and rdcmd_fifo, otherwise, HW will go wrong */
    val = io_read32(RDBYPASSWT_CFG);
    val |= (0x1);
    io_write32(RDBYPASSWT_CFG, val);
}

#if defined(AIR_SUPPORT_ACE)
/* FUNCTION NAME: sys_info_init
 * PURPOSE:
 *      Initialize sys info
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
/* fields of global mac control */
#define GMAC_GLOBAL_BASE                    (0x10213E00)
#define SMACCR0                             (GMAC_GLOBAL_BASE + 0x10)
#define SMACCR1                             (GMAC_GLOBAL_BASE + 0x14)
PLAT_SYS_INFO_T g_sys_info;
void sys_info_init(void)
{
    uint32_t u32dat;
    uint8_t i;
    uint8_t mac_addr[MAC_ADDRESS_LEN];
    /* Read system MAC from flash ROM */
    uint8_t *sector_addr0 = (uint8_t *)(EEP_CONFIG_DATA_LOCATION | HIGH_BIT_UNC);
    memcpy(mac_addr, sector_addr0, MAC_ADDRESS_LEN);/* Get MAC address from the flash rom location 56KB - 6, the 6 bytes before the CONFIG. */
    if (mac_addr[0] != 0xFF)
    {
        /* Write the system MAC to rg SMACCR */
        /* SMACCR 1 */
        u32dat = 0;
        for (i = 0; i < 2; i++)
        {
            u32dat |= BITS_OFF_L(mac_addr[i], ((1 - i) * 8), 8);
        }
        io_write32(SMACCR1, u32dat);
        /* SMACCR 0 */
        u32dat=0;
        for (i = 2; i < 6; i++)
        {
            u32dat |= BITS_OFF_L(mac_addr[i], ((5 - i) * 8), 8);
        }
        io_write32(SMACCR0, u32dat);
    }
    /* SMACCR 1 */
    u32dat = io_read32(SMACCR1);
    for (i = 0; i < 2; i++)
    {
        g_sys_info.mac_addr[i] = BITS_OFF_R(u32dat, ((1 - i) * 8), 8);
    }
    /* SMACCR 0 */
    u32dat = io_read32(SMACCR0);
    for (i = 2; i < 6; i++)
    {
        g_sys_info.mac_addr[i] = BITS_OFF_R(u32dat, ((5 - i) * 8), 8);
    }

    return;
}
#endif


void airInitHardware( void )
{
    int i;
    unsigned int rst_ctrl;

    uart_pre_init();
    uart_init();

    /*reset timer*/
    rst_ctrl = io_read32(RG_RST_CTRL2);
    io_write32(RG_RST_CTRL2, rst_ctrl | TIMER_RST_EN_MSK);
    io_write32(RG_RST_CTRL2, rst_ctrl & (~TIMER_RST_EN_MSK));

    printf("\n[%s] __data_start:0x%x, _edata:0x%x, __sbss_end:0x%x\n",
            __func__, (unsigned int)&__data_start, (unsigned int)&_edata, (unsigned int)&__sbss_end);
    rbus_timeout_init();
    rbus_rdbypasswt_init();
    pbus_timeout_init();

    /* enable timers */
    timer_init(0, 1, 1000); /* enable timer0 for delay1ms */

    /* init phy */
    io_write32(0x1028c840, 0);
    for(i = 0; i < 5; i++)
    {
        cl22_write(i, 0, 0, 0x1040);
    }

    init_interrupts_count();
}

void uart_pre_init(void)
{
    /*
     * 1. held UART reset to prevent UART RX_FIFO corrupted by default pull down signal.
     * 2. Change gpio 0, 1 default pull down to pull up, to meet the idle signal of UART.
     * 3. release uart reset.
     */
    unsigned int reg, rst_ctrl;

    rst_ctrl = io_read32(RG_RST_CTRL2);
    rst_ctrl |= UART_RST_EN_MSK;
    io_write32(RG_RST_CTRL2, rst_ctrl);

    reg = io_read32(RG_GPIO_L_PU);
    reg |= ((1 << GPIO_UART_RX) | (1 << GPIO_UART_TX));
    io_write32(RG_GPIO_L_PU, reg);

    reg = io_read32(RG_GPIO_L_PD);
    reg &= ~((1 << GPIO_UART_RX) | (1 << GPIO_UART_TX));
    io_write32(RG_GPIO_L_PD, reg);

    rst_ctrl = io_read32(RG_RST_CTRL2);
    rst_ctrl &= ~(UART_RST_EN_MSK);
    io_write32(RG_RST_CTRL2, rst_ctrl);

    /* Enable IOMUX for UART pins.
     * Note: Only support in AN8801F GPIO8/GPIO9, this setting should be checked or revised at AN8855 IC back.
     */
    /* read 0x10000074 */
    reg = io_read32(RG_GPIO_UART1_MODE);

    /* setup uart on GPIO 0,1 */
    reg |= ((1 & GPIO0_UART_MSK) << GPIO0_UART_OFS);
    io_write32(RG_GPIO_UART1_MODE, reg);

}
