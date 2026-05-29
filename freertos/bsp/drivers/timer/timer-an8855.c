#include <stdint.h>
#include <platform.h>
#include <nds32_intrinsic.h>
#include "timer.h"
#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"

/*********************************************************/

/*
    When TIMERCLK is 225M
    REFINE_TIMETICK_PRECISE will fix current tmrTick
    run faster than expect about 4000 ppm (0.004%)
*/
#define REFINE_TIMETICK_PRECISE 1

/*********************************************************/

GDMPSRAM_DATA static int isWdogEn=0;
GDMPSRAM_BSS  static unsigned int g_WDT_SW_cnt;
GDMPSRAM_BSS static unsigned int g_WDT_SW_threshold;

GDMPSRAM_DATA GDMPSRAM_DATA static int isTimerEn[4]={0};

GDMPSRAM_BSS unsigned short interrupts_count[IRQ_MAX_NUM];
char * const intr_name[IRQ_MAX_NUM] = {
    "GPIO",       "TMR0",        "TMR1",    "TMR2",
    "TMR3",       "UART",        "CPUTMR0", "CPUTMR1",
    "BUS_TOUT",   "RG_ABIST",    "SFC",     "I2C2RBUS",
    "GDMP",       "ILLEGAL_ACS", "PDMA",    "GSW_SYS",
     "SWI",       "PTP",         "WOL0",    "WOL1",
     "EFUSE",     "EFIFO1",      "EFIFO2",  "EFIFO3",
     "EFIFO4",    "GPHY",        "HSGMII",  "I2C2RBUS2",
    "EXT_IRQ0",   "EXT_IRQ1",    "EXT_IRQ2","MACEBUS"
};

void init_interrupts_count(void)
{
    int i;
    for (i=0; i<IRQ_MAX_NUM; i++)
        interrupts_count[i]=0;
}

void record_interrupts(unsigned int intSrc)
{
    interrupts_count[intSrc]++;
}

void show_interrupts(void)
{
    int i;

    printf("\r\n<no>\t<count>\t<name>\r\n");
    for (i=0; i<IRQ_MAX_NUM; i++)
        printf("%d\t%d\t%s\r\n", i, interrupts_count[i], intr_name[i]);

    printf("\n");
    return;
}

static void timer_isr (unsigned int timer_no)
{
    unsigned int val;

    /*
     * CPU clears the interrupt source then executes the ISR
     */
    val = io_read32(CR_TIMER_CTL);
    val &= 0x2000027; /* keep enable_bits */
    val |= (1<<(16+timer_no)); /* add the intr_bit */
    io_write32(CR_TIMER_CTL, val); /* clear timer source and keep timer enabled */

    if (timer_no==5)
        timer_no=3;
    record_interrupts(IRQ_TMR0+timer_no);

    return;
}

void air_timer0_isr (void) {
    timer_isr(0);
}
void air_timer1_isr (void) {
    timer_isr(1);
}
void air_timer2_isr (void) {
    timer_isr(2);
}
void air_timer3_isr (void) {
    timer_isr(5);
}

#ifdef SYS_MONITOR_WDOG_KICK
#define SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE 32

uint32_t g_wdog_task_loading[SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE];
uint32_t g_wdog_task_data[SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE];
uint16_t g_wdog_task_context_switch[SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE];

void sys_monitor_wdog_kick(uint32_t taskID, uint32_t runtime, uint32_t data)
{
    if( SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE > taskID )
    {
        g_wdog_task_loading[taskID] += runtime;
        g_wdog_task_data[taskID] = data;
        ++g_wdog_task_context_switch[taskID];
    }
}

void sys_monitor_wdog_kick_reset(void)
{
    memset(g_wdog_task_loading, 0, sizeof(g_wdog_task_loading));
}

void sys_monitor_wdog_kick_dump(void)
{
    printf("task tick dump\n");

    extern uint32_t TaskCurrentTaskID(void);
    uint32_t cur_ipc = __nds32__mfsr(__NDS32_REG_IPC__);
    uint32_t cur_taskID = TaskCurrentTaskID();;

    uint32_t taskID;
    for(taskID=0;taskID<SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE;++taskID)
    {
        if(g_wdog_task_loading[taskID])
        {
            if(cur_taskID==taskID)
            {
                printf("  taskID: %2u, tick: %8u, pc: %08x (current)\n", taskID, g_wdog_task_loading[taskID], g_wdog_task_data[taskID]);
            }
            else
            {
                printf("  taskID: %2u, tick: %8u, pc: %08x\n"          , taskID, g_wdog_task_loading[taskID], g_wdog_task_data[taskID]);
            }
        }
    }
    printf("\n");
}

#ifdef SYS_MONITOR_WDOG_KICK___TASK_STATS_PC

void sys_monitor_wdog_kick_dump_for_taskstats(uint32_t taskID)
{
    if( SYS_MONITOR_WDOG_KICK__TASK_TABLE_SIZE > taskID )
    {
        taskENTER_CRITICAL();
        uint32_t task_pc = g_wdog_task_data[taskID];
        uint32_t context_switch_cnt = g_wdog_task_context_switch[taskID];
        taskEXIT_CRITICAL();

        printf("\t%u\t%x", context_switch_cnt, (int)task_pc);
    }
}
#endif
#endif

void air_wdog_isr(void)
{
    unsigned int word;

    /* clear wdog timer's interrupt */
    word = io_read32(CR_TIMER_CTL);
    word &= 0xffc0ffff;
    word |= 0x00200000;
    io_write32(CR_TIMER_CTL, word);

    printf("\n%s\n", __func__);

    printf("IPSW:0x%x\tITYPE:0x%x\tIPC:0x%x\tINT_MASK2:0x%x\n",
            __nds32__mfsr(__NDS32_REG_IPSW__), __nds32__mfsr(__NDS32_REG_ITYPE__),
            __nds32__mfsr(__NDS32_REG_IPC__), __nds32__mfsr(__NDS32_REG_INT_MASK2__));
    printf("INT_PEND2:0x%x\n", __nds32__mfsr(__NDS32_REG_INT_PEND2__));

    printf("WDT_SW(%u,%u)\n", g_WDT_SW_threshold, g_WDT_SW_cnt);

#ifdef SYS_MONITOR_WDOG_KICK
    sys_monitor_wdog_kick_dump();

    // make exception to dump more info
    volatile int test = 1;
    test /= 0;
#endif

    return;
}

void air_wdog_kick(void)
{
    if (isWdogEn)
    {
        io_write32(CR_WDOG_RLD, 0x1);

        #ifdef SYS_MONITOR_WDOG_KICK
        sys_monitor_wdog_kick_reset();
        #endif
    }
}

int isWdogEnabled(void) {
    return isWdogEn;
}

unsigned int isTimerEnabled(unsigned int timer_no) /*timer_no: 0~3*/
{
    unsigned int word;

    timer_no &= 0x3;

    if (timer_no==3) timer_no=5;

    word = io_read32(CR_TIMER_CTL);
    return ((word>>timer_no)&0x1);
}

#if (REFINE_TIMETICK_PRECISE==0)
static unsigned int get_1ms_timeTick_by_timeClk(unsigned int clk /*MHz*/)
{
    /* if timer_clk is 1 MHz, timeLoad should be 1000 to stand for 1ms */
    return clk*KHz;
}
#endif

static unsigned int get_tmrTick_by_msTime (unsigned int msTime)
{
#if (REFINE_TIMETICK_PRECISE==0)
    return (get_1ms_timeTick_by_timeClk(TIMERCLK/MHz)*msTime);
#else
    return (TIMERCLK/KHz)*msTime;
#endif
}

unsigned int get_cpuTmrTick_by_msTime (unsigned int msTime)
{
#if (REFINE_TIMETICK_PRECISE==0)
    return (get_1ms_timeTick_by_timeClk(CPUTMRCLK/MHz)*msTime);
#else
    return (CPUTMRCLK/KHz)*msTime;
#endif
}

unsigned int get_msTime_by_cpuTmrTick (unsigned int cpuTmrTick)
{
    return (cpuTmrTick/(CPUTMRCLK/KHz));
}

void delay1ms(int ms)
{
    unsigned int timer_no = 0; /* by timer0 */
    unsigned int cr_timer_load;
    unsigned int cr_timer_curr;
    volatile unsigned int timer_now, timer_last;
    volatile unsigned int tick_acc;
#if (REFINE_TIMETICK_PRECISE==0)
    volatile unsigned int tick_wait = ms * (get_1ms_timeTick_by_timeClk(TIMERCLK/MHz));
#else
    volatile unsigned int tick_wait = get_tmrTick_by_msTime(ms);
#endif
    volatile unsigned int timer_ldv;

    if (isTimerEnabled(timer_no)==0) {
        printf("%s timer_no:%d is disabled, so just return!\n", __func__, timer_no);
        return;
    }

    if (timer_no==3) timer_no=5;

    cr_timer_load = CR_TIMER0_LVR+(timer_no<<3);
    cr_timer_curr = CR_TIMER0_CVR+(timer_no<<3);

    timer_ldv = io_read32(cr_timer_load);

    tick_acc = 0;
    timer_last = io_read32(cr_timer_curr);
    do {
        timer_now = io_read32(cr_timer_curr);
        /* timer counts down, so usually timer_last is larger timer_now */
        if (timer_last >= timer_now)
            tick_acc += timer_last - timer_now;
        else
            tick_acc += timer_ldv - timer_now + timer_last;
        timer_last = timer_now;
    } while (tick_acc < tick_wait);
}

void
timer_init(
    unsigned int timer_no,  /*0, 1, 2, or 3*/
    unsigned int enable,    /*0: disable, 1: enable*/
    unsigned int loadTime   /* ms */
)
{
    unsigned int word;

    if (timer_no>3) {
        printf("ERROR(%s): timer_no:%d is wrong, should be 0,1,2,or 3\n", __func__, timer_no);
        return;
    }

    if (enable) {
        if (isTimerEn[timer_no]) {
            printf("ERROR(%s): timer_no:%d is occupied, please use another timer\n", __func__, timer_no);
            return;
        }
        isTimerEn[timer_no]=1;
    }
    else {
        isTimerEn[timer_no]=0;
    }

    /* timer3's registers are placed in timer5's address */
    if (timer_no==3) timer_no=5;

    if (enable) {

        io_write32(CR_TIMER0_LVR+(timer_no<<3), get_tmrTick_by_msTime(loadTime));
        word = io_read32(CR_TIMER_CTL);
        word |= (1 << timer_no);
        io_write32(CR_TIMER_CTL, word);
    }
    else {
        word = io_read32(CR_TIMER_CTL);
        word &= (~(1 << timer_no));
        io_write32(CR_TIMER_CTL, word);
    }

    return;
}

/*
 * when timeOut, wdog reboots.
 * when wdogThld reached, wdog issues interrupt
 */
static void wdog_init (unsigned int enable, unsigned int timeOut/*ms*/, unsigned int wdogThld/*ms*/)
{
    unsigned int tmr=3;
    unsigned int regVal;

    timer_init(tmr, enable, timeOut);

    if (enable) {

        /* set wdog threshold. when the threshold is reached, wdog will issue interrupt. */
        io_write32(CR_WDOG_THSLD, get_tmrTick_by_msTime(wdogThld));

        /* clear tmr3 first due to being enabled in timer_init eariler */
        regVal = io_read32(CR_TIMER_CTL);
        regVal &= (~(1<<5));
        io_write32(CR_TIMER_CTL, regVal);
        /* enable wdog on tmr3 */
        regVal = io_read32(CR_TIMER_CTL);
        regVal |= ((1<<5)|(1<<25));
        io_write32(CR_TIMER_CTL, regVal);

        isWdogEn=1;

#ifdef IRQ_DBG_SUPPORT
        set_interrupt_priority(IRQ_TMR3, 0x0);
#endif

    }

    return;
}

void wdog_kill(void)
{
    isWdogEn=0;
    wdog_init(0, 1, 1);
    printf("\nwdog killed!\n");
    return;
}

void wdog_setup(unsigned int hwdog_secs,unsigned int swdog_secs)
{
    if( hwdog_secs < Wdog_Timeout_Min_Counter )
    {
        hwdog_secs = Wdog_Timeout_Min_Counter;
    }

    if( swdog_secs < (hwdog_secs*5) )
    {
        swdog_secs = hwdog_secs*5;
    }

    printf("hwdog_secs: %d, swdog_secs: %d\n", hwdog_secs, swdog_secs);

    /* if cpu doesn't feed wdog for hwdog_secs, system will reboot. In last [Wdog_Interrupt_Counter] secs, wdog interrupt will be issued */
    wdog_init(1, (hwdog_secs*1000), (Wdog_Interrupt_Counter*1000));

    g_WDT_SW_threshold = swdog_secs;
    g_WDT_SW_cnt = 0;

    register_isr(IRQ_TMR3, air_wdog_isr);
}

void wdog_sec_handler(void)
{
    ++g_WDT_SW_cnt;
    if(g_WDT_SW_cnt<g_WDT_SW_threshold)
    {
        air_wdog_kick();
    }
}

void wdog_idle_handler(void)
{
    portENTER_CRITICAL();
    g_WDT_SW_cnt = 0;
    portEXIT_CRITICAL();
}

void enable_cpu_timer (unsigned int tmr, unsigned int enable)
{
    unsigned int regVal;

    if (enable) {
        regVal = io_read32(CPU_TIMER_CTRL);
        regVal |= (1<<tmr);
        io_write32(CPU_TIMER_CTRL, regVal);
    }
    else {
        regVal = io_read32(CPU_TIMER_CTRL);
        regVal &= (~(1<<tmr));
        io_write32(CPU_TIMER_CTRL, regVal);
    }

    return;
}

void set_cpu_timer_cmpTickVal (unsigned int tmr, unsigned int cmpTickVal)
{
    io_write32(CPU_TIMER0_CMP+(tmr<<3), cmpTickVal);
#ifdef CPU_TIMER_HW_FIX_WORKAROUND
    #define CPU_TIMER_HW_FIX_WORKAROUND_TIMEOUT 10000
    unsigned int loop_cnt = 0;
    // To avoid HW system bus latency by other traffic
    // FW wait interrupt status to clear
    while( io_read32(CPU_TIMER_CTRL) & ((1+tmr)<<16) )
    {
         ++loop_cnt;
         if(loop_cnt>CPU_TIMER_HW_FIX_WORKAROUND_TIMEOUT)
         {
            break;
         }
    }
#endif
}

unsigned int get_cpu_timer_cnt (unsigned int tmr)
{
    return io_read32(CPU_TIMER0_CVR+(tmr<<3));
}

void cpu_timer_init(unsigned int tmr, unsigned int enable, unsigned int cmpTickVal)
{
    if (tmr>2)
        printf("%s cpu_tmr:%d is wrong, should be 0 or 1\n", __func__, tmr);

    if (enable) {

        enable_interrupt_mask(IRQ_CPU_TMR0);

        /*set compare value and reset current value */
        set_cpu_timer_cmpTickVal(tmr, cmpTickVal);
        io_write32(CPU_TIMER0_CVR+(tmr<<3), 0);
    }
    else {
        disable_interrupt_mask(IRQ_CPU_TMR0);
    }

    /* enable/disable tmr */
    enable_cpu_timer(tmr, enable);

    return;
}

