#include <stdio.h>
#include <platform.h>
#include <nds32_intrinsic.h>
#include "util.h"
#include "interrupt.h"

#define TIMER_EN    (1)
#define TIMER_DIS   (0)

/**************************
 * Timer Module Registers *
 **************************/
#define CR_TIMER_CTL    	(CR_TIMER_BASE + 0x00)
#define CR_TIMER0_LVR   	(CR_TIMER_BASE + 0x04)
#define CR_TIMER0_CVR    	(CR_TIMER_BASE + 0x08)
#define CR_TIMER1_LVR       (CR_TIMER_BASE + 0x0C)
#define CR_TIMER1_CVR       (CR_TIMER_BASE + 0x10)
#define CR_TIMER2_LVR       (CR_TIMER_BASE + 0x14)
#define CR_TIMER2_CVR       (CR_TIMER_BASE + 0x18)
#define CR_TIMER3_LVR       (CR_TIMER_BASE + 0x2C)
#define CR_TIMER3_CVR       (CR_TIMER_BASE + 0x30)
#define CR_WDOG_THSLD       (CR_TIMER_BASE + 0x34)
#define CR_WDOG_RLD         (CR_TIMER_BASE + 0x38)

#define CPU_TIMER_CTRL      (CR_CPUTMR_BASE + 0x00)
#define CPU_TIMER0_CMP      (CR_CPUTMR_BASE + 0x04)
#define CPU_TIMER0_CVR      (CR_CPUTMR_BASE + 0x08)
#define CPU_TIMER1_CMP      (CR_CPUTMR_BASE + 0x0c)
#define CPU_TIMER1_CVR      (CR_CPUTMR_BASE + 0x10)

static int isWdogEn=0;
static int isTimerEn[4]={0};

unsigned int interrupts_count[IRQ_MAX_NUM];


#if defined(AIR_8851_SUPPORT)
char *intr_name[IRQ_MAX_NUM] = {
    "GPIO",         "TMR0",         "TMR1",         "TMR2",
    "TMR3",         "UART",         "CPUTMR0",      "CPUTMR1",
    "BUS_TOUT",     "RG_ABIST",     "SFC",          "I2C2RBUS",
    "GDMP",         "ILLEGAL_ACS",  "PDMA",         "GSW_SYS",
    "SWI",          "QSGMII0_WSA",  "QSGMII0_PCA",  "QSGMII1_WSA",
    "QSGMII1_PCA",  "QSGMII2_WSA",  "QSGMII2_PCA",  "QSGMII3_WSA",
    "QSGMII3_PCA",  "QSGMII4_WSA",  "QSGMII4_PCA",  "I2C2RBUS2",
    "EXT_IRQ0",     "EXT_IRQ1",     "EXT_IRQ2",     "Reserved"
};
#elif defined(AIR_8855_SUPPORT) 
char *intr_name[IRQ_MAX_NUM] = {
    "GPIO",       "TMR0",        "TMR1",    "TMR2", 
    "TMR3",       "UART",        "CPUTMR0", "CPUTMR1",
    "BUS_TOUT",   "RG_ABIST",    "SFC",     "I2C2RBUS",
    "GDMP",       "ILLEGAL_ACS", "PDMA",    "GSW_SYS",
     "SWI",       "PTP",         "WOL0",    "WOL1",
     "EFUSE",     "EFIFO1",      "EFIFO2",  "EFIFO3",
     "EFIFO4",    "GPHY",        "HSGMII",  "I2C2RBUS2",
    "EXT_IRQ0",   "EXT_IRQ1",    "EXT_IRQ2","MACEBUS"
};
#elif defined(AIR_8858_SUPPORT)
char * intr_name[IRQ_MAX_NUM] = {
    "GPIO",       "TMR0",        "TMR1",    "TMR2",
    "TMR3",       "UART",        "CPUTMR0", "CPUTMR1",
    "BUS_TOUT",   "RG_ABIST",    "SFC",     "I2C2RBUS",
    "GDMP",       "ILLEGAL_ACS", "PDMA",    "GSW_SYS",
     "SWI",       "PTP",         "WOL0",    "WOL1",
     "EFUSE",     "EFIFO1",      "EFIFO2",  "EFIFO3",
     "EFIFO4",    "GPHY",        "HSGMII",  "I2C2RBUS2",
    "EXT_IRQ0",   "EXT_IRQ1",    "EXT_IRQ2","MACEBUS"
};

#else
#error "todo"
#endif


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

    return;
}

void air_wdog_kick(void)
{
    if (isWdogEn)
        io_write32(CR_WDOG_RLD, 0x1);
    return;
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

static unsigned int get_1ms_timeTick_by_timeClk(unsigned int clk /*MHz*/)
{
    /* if timer_clk is 1 MHz, timeLoad should be 1000 to stand for 1ms */
    return clk*KHz;
}

static unsigned int get_tmrTick_by_msTime (unsigned int msTime)
{
    return (get_1ms_timeTick_by_timeClk(TIMERCLK/MHz)*msTime);
}

unsigned int get_cpuTmrTick_by_msTime (unsigned int msTime)
{
    return (get_1ms_timeTick_by_timeClk(CPUTMRCLK/MHz)*msTime);
}

#ifdef CPU_BUS_TEST
unsigned int get_msTime_by_cpuTmrTick (unsigned int cpuTmrTick)
{
    return (cpuTmrTick/(CPUTMRCLK/KHz));
}

unsigned int get_timer0_tick(void)
{
    /* by timer0 */
    if (isTimerEnabled(0)==0) {
        printf("\n[%s] timer 0 is disabled, so just return!\n", __func__);
        return 0;
    }

    return io_read32(CR_TIMER0_CVR);
}
#endif

void delay1ms(int ms)
{
    unsigned int timer_no = 0; /* by timer0 */
    unsigned int cr_timer_load;
    unsigned int cr_timer_curr;
	volatile unsigned int timer_now, timer_last;
	volatile unsigned int tick_acc;
	volatile unsigned int tick_wait = ms * (get_1ms_timeTick_by_timeClk(TIMERCLK/MHz)); 
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
void wdog_init (unsigned int enable, unsigned int timeOut/*ms*/, unsigned int wdogThld/*ms*/)
{
    unsigned int tmr=3;
    unsigned int regVal;

    #ifdef CPU_BUS_TEST
    /* disable timer3 before enabling it, otherwise ERROR will happen due to timer3 already being enabled during booting */
    timer_init(tmr, 0, timeOut);
    #endif
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
    return;
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

