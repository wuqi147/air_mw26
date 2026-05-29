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
void enable_cpu_timer (unsigned int tmr, unsigned int enable);
void set_cpu_timer_cmpTickVal (unsigned int tmr, unsigned int cmpTickVal);
unsigned int get_cpu_timer_cnt (unsigned int tmr);
void cpu_timer_init(unsigned int tmr, unsigned int enable, unsigned int cmpTickVal);
