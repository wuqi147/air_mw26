#ifndef __TIMER__H__
#define __TIMER__H__

#define TIMER_EN    (1)
#define TIMER_DIS   (0)

/**************************
 * Timer Module Registers *
 **************************/
#define CR_TIMER_CTL        (CR_TIMER_BASE + 0x00)
#define CR_TIMER0_LVR       (CR_TIMER_BASE + 0x04)
#define CR_TIMER0_CVR       (CR_TIMER_BASE + 0x08)
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

/***************************************************/

void
timer_init(
    unsigned int timer_no,  /*0, 1, 2, or 3*/
    unsigned int enable,    /*0: disable, 1: enable*/
    unsigned int loadTime   /* ms */
);
void init_interrupts_count(void);
void record_interrupts(unsigned int intSrc);
void show_interrupts(void);
void air_wdog_kick(void);
int isWdogEnabled(void);
unsigned int isTimerEnabled(unsigned int timer_no); /*timer_no: 0~3*/
unsigned int get_cpuTmrTick_by_msTime (unsigned int msTime);
unsigned int get_msTime_by_cpuTmrTick (unsigned int cpuTmrTick);
unsigned int get_timer0_tick(void);
void delay1ms(int ms);
void wdog_kill(void);
void wdog_setup(unsigned int hwdog_secs,unsigned int swdog_secs);
void wdog_sec_handler(void);
void wdog_idle_handler(void);
void enable_cpu_timer (unsigned int tmr, unsigned int enable);
void set_cpu_timer_cmpTickVal (unsigned int tmr, unsigned int cmpTickVal);
unsigned int get_cpu_timer_cnt (unsigned int tmr);
void cpu_timer_init(unsigned int tmr, unsigned int enable, unsigned int cmpTickVal);
#endif