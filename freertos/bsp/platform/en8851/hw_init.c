#include <FreeRTOS.h>
#include <task.h>
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>
#include "pp_def.h"
#include <cache.h>
#ifdef CONFIG_ERPS_API
#include "erps_hw_init.h"
#endif

#define PHI_SMI_PMDC_MASK       (~(0x3 << 6))
#define PHI_SMI_PMDC_11_2M      (0xc0)          /* 11.2 MHZ */
#define PHI_SMI_PMDC_5_6M       (0x80)          /* 5.6 MHZ */
#define PHI_SMI_PMDC_2_8M       (0x40)          /* 2.8 MHZ */

extern char __data_start, _edata, __sbss_end;

#ifdef SYS_INTERNAL_DEBUG

#define SYS_INTERNAL_DEBUG__SKIP_FAKE

typedef void (*ill_isr_callback_t)(uint32_t info);
struct auto_setup_ill_t
{
    uint32_t pattern;
    uint32_t enable;
    uint32_t addr;
    uint32_t data_size;
    uint32_t skip_count;
    uint16_t hold;
    uint8_t  dump;
    uint8_t  skip_dump;
    ill_isr_callback_t ill_isr_callback;
    uint32_t pattern_end;
};

struct auto_setup_ill_t auto_setup_ill = {
                                            .pattern = 0x34125a5a,
                                            .enable = 0,
                                            .addr = 0x0,
                                            .data_size = 0x0,
                                            .skip_count = 0x0,
                                            .hold = 0,
                                            .dump = 0,
                                            .skip_dump = 0,
                                            .ill_isr_callback = 0,
                                            .pattern_end = 0x78565a5a,
                                          };


void ill_isr(void)
{
    SAVE_ALL();

#ifdef SYS_INTERNAL_DEBUG__SKIP_FAKE
    if( io_read32(0x1000102c)==0xfffffff8 )
    {
        return;
    }
#endif

    if(auto_setup_ill.skip_count>0)
    {
        --auto_setup_ill.skip_count;
    }

    if(!auto_setup_ill.skip_count)
    {
        printf("ill_isr\n");
        printf("  task name: %s\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
        printf("  access addr: 0x%x\n", io_read32(0x1000102c));
        printf("  info: 0x%x\n", io_read32(0x10001030));
        show_reg();

        if(auto_setup_ill.ill_isr_callback)
        {
            auto_setup_ill.ill_isr_callback(0);
        }

        if(auto_setup_ill.dump)
        {
            uint32_t addr_beg = auto_setup_ill.addr;
            uint32_t addr_end = addr_beg + auto_setup_ill.data_size;

            while(addr_beg<addr_end)
            {
                printf(" [%x]:%08x\n", addr_beg, *(unsigned int *)addr_beg);

                addr_beg +=4;
            }
        }

        if(auto_setup_ill.hold)
        {
            portDISABLE_INTERRUPTS();

            printf("hold for debug\n");

            extern void wdog_kill(void);
            wdog_kill();

            io_write32(0x10000090, 0x1ffc);

            while(1);
        }
    }
    else
    {
        if(auto_setup_ill.skip_dump)
        {
            uint32_t addr_beg = auto_setup_ill.addr;
            uint32_t addr_end = addr_beg + auto_setup_ill.data_size;

            while(addr_beg<addr_end)
            {
                printf(" [%x]:%08x\n", addr_beg, *(unsigned int *)addr_beg);

                addr_beg +=4;
            }
        }
    }

    io_write32(0x10001030, (1 << 31));
}

void setup_ill_acc(unsigned int start, unsigned int size)
{
    auto_setup_ill.addr = start;
    auto_setup_ill.data_size = size;

    io_write32(0x10001018, 1 << 3);
    io_write32(0x1000101c, start);
    io_write32(0x10001020, start + size - 4);

    register_isr(IRQ_ILL_ACCESS, ill_isr);

    int delay = 100;
    while(--delay)
    {
        asm volatile ("nop");
    }

    printf("setup_ill_acc %08x - %08x\n", start, start + size - 4);
}

void setup_ill_acc_callback(ill_isr_callback_t callback)
{
    auto_setup_ill.ill_isr_callback = callback;
}

void apply_auto_setup_ill(void)
{
    if(auto_setup_ill.enable==2)
    {
        setup_ill_acc(auto_setup_ill.addr, auto_setup_ill.data_size);
    }
}

#endif // SYS_INTERNAL_DEBUG

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

    return;
}

void air_pbus_tout_isr(void)
{
    int isBusTout=0;
    unsigned int errAddr;


    isBusTout = (io_read32(PB_TIMEOUT_INT)&0x1);
    io_write32(PB_TIMEOUT_INT, 1); /* clear intr */

    errAddr = io_read32(PB_TIMEOUT_INFO);

    if (isBusTout)
        printf("\n\npbus timeout interrupt errAddr=%08lx\n\n", errAddr);
    else
        printf("\n\nunknown bus timeout interrupt errAddr=%08lx\n\n", errAddr);

    return;
}

void pbus_timeout_init(void)
{
    unsigned int val;

    register_isr(IRQ_BUS_TOUT, air_pbus_tout_isr);

    val = BUSCLKFREQ/10; /* timeout time: 100 ms */
    io_write32(PB_TIMEOUT_CFG, val);

    val |= (1<<31); /* enable pbus timeout */
    io_write32(PB_TIMEOUT_CFG, val);

    return;
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

    //printf("RDBYPASSWT_CFG:0x%x, RDBYPASSWT_MASK:0x%x\n", io_read32(RDBYPASSWT_CFG), io_read32(RDBYPASSWT_MASK));

    return;
}

#ifdef CONFIG_ERPS_API_DEMO
#define ERPS_PRINT 1
int intr_number = IRQ_CPU_TMR1;
extern unsigned int interrupts_count[IRQ_MAX_NUM];
extern unsigned int erps_total_count;
extern unsigned int erps_over_count;
extern unsigned int erps_pre_over_count;
extern unsigned int erps_pre_dual_count;
unsigned int print_count = 0;
#if ERPS_PRINT
unsigned int test_count = 10000;
#else
unsigned int test_count = 100;
#endif

void show_interrupt(void *data)
{
#if ERPS_PRINT
    int i = *(int *)data;
    unsigned int current_tick = erps_get_current_tick();

    print_count++;
    if ((print_count * test_count) >= 60000)
    {
        print_count = 0;

        printf("\r\ntc %d oc %d poc %d pdc %d ic %d ct %u",
                erps_total_count, erps_over_count, erps_pre_over_count, erps_pre_dual_count, interrupts_count[i], current_tick);
    }
#else
    printf("\r\n.");
#endif
}
#endif

void airInitHardware( void )
{
    unsigned int val;
    uart_init();
    printf("\n[%s] __data_start:0x%x, _edata:0x%x, __sbss_end:0x%x\n",
            __func__, (unsigned int)&__data_start, (unsigned int)&_edata, (unsigned int)&__sbss_end);
    rbus_timeout_init();
    rbus_rdbypasswt_init();
    pbus_timeout_init();

    /* enable timers */
    timer_init(0, 1, 1000); /* enable timer0 for delay1ms */

    /* set MDC clock */
    val = io_read32(CR_PHY_SMI);
    val = (val & PHI_SMI_PMDC_MASK) | (PHI_SMI_PMDC_2_8M);
    io_write32(CR_PHY_SMI, val);

    init_interrupts_count();

    printf("build %s %s!\n",__DATE__,__TIME__);

#ifdef SYS_LOG_2_FLASH
    sys_log_2_flash_init();
#endif

#ifdef SYS_INTERNAL_DEBUG
    apply_auto_setup_ill();
#endif

#ifdef SYS_MONITOR_DIS_INTERRUPT
    unsigned int u32dat;
    u32dat = io_read32(CHIPSCU_SYS_CTRL1);
    extern unsigned int g_u32dat_reset_reason;
    g_u32dat_reset_reason = u32dat; // keep for air_chipscu_getBootReason
    io_write32(CHIPSCU_SYS_CTRL1, u32dat); // clear
    printf("reset reason: ");
    if( u32dat & (1<<31))
    {
        printf("WDT\n");
        extern void sys_monitor_dis_interrupt_dump(void);
        sys_monitor_dis_interrupt_dump();
    }
    else if( u32dat & (1<<30))
    {
        printf("SW\n");
    }
    else
    {
        printf("PWR\n");
    }
#endif

#ifdef CONFIG_ERPS_API
    erps_timer_init(ERPS_1MS_TICK, ERPS_1US_TICK);
    erps_gsw_isr_init();
#ifdef CONFIG_ERPS_API_DEMO
    erps_gsw_isr_port_set(0xFF, 1);
    erps_timer_start(test_count, (void *) &intr_number, show_interrupt);
#endif
#endif
}
