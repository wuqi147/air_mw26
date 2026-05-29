#include <FreeRTOS.h>
#include <stdint.h>
#include <nds32_intrinsic.h>
#include "erps_hw_init.h"
#include "util.h"
#include "interrupt.h"
#include "timer.h"
#ifdef CONFIG_ERPS_API_DEMO
#include <stdio.h>
#endif
#ifdef AIR_SUPPORT_ERPS
#include "erps.h"
#endif /* AIR_SUPPORT_ERPS */

timer_callback_t *timer_callback = NULL;

#ifdef CONFIG_ERPS_API_DEMO
unsigned int erps_total_count = 0;
unsigned int erps_over_count = 0;
unsigned int erps_pre_over_count = 0;
unsigned int erps_pre_dual_count = 0;
#endif
unsigned int erps_1ms_count = 0;
unsigned int erps_goal_count = 0;
void *erps_data = NULL;
unsigned int reload = 0;
unsigned int reload_jitter = 0;

#ifdef CONFIG_ERPS_API_DEMO
unsigned char link_sts[MAX_PORT];
#endif

void erps_gsw_isr(void)
{
    unsigned int int_staust = 0;
#ifdef CONFIG_ERPS_API_DEMO
    unsigned int psr_p3_p0 = ~0, psr_p7_p4 = ~0;
    unsigned char link_status = LINK_DOWN;
    int i = 0;
#endif

    int_staust = io_read32(CR_GSW_SYS_INT_STS);
    io_write32(CR_GSW_SYS_INT_STS, int_staust);
    __nds32__dsb();
#ifdef AIR_SUPPORT_ERPS
    erps_queue_link_change_noti(&int_staust);
#endif /* AIR_SUPPORT_ERPS */

#ifdef CONFIG_ERPS_API_DEMO
    if (int_staust & 0xF)
    {
        psr_p3_p0 = io_read32(PSR_P3_P0_REG);

        for (i = 0; i < (MAX_PORT >> 1); i++)
        {
            if (int_staust & (1 << i))
            {
                link_status = GET_PSR_LINK_STS(psr_p3_p0, i);
                if (link_sts[i] != link_status)
                {
                    link_sts[i] = link_status;
                    printf("Port%x change to %s\n", i, link_sts[i] ? "LINK UP" : "LINK DOWN");
                }
            }
        }
    }

    if (int_staust & 0xF0)
    {
        psr_p7_p4 = io_read32(PSR_P7_P4_REG);

        for (i = (MAX_PORT >> 1); i < MAX_PORT; i++)
        {
            if (int_staust & (1 << i))
            {
                link_status = GET_PSR_LINK_STS(psr_p7_p4, i);
                if (link_sts[i] != link_status)
                {
                    link_sts[i] = link_status;
                    printf("Port%x change to %s\n", i, link_sts[i] ? "LINK UP" : "LINK DOWN");
                }
            }
        }
    }
#endif
}

void erps_gsw_isr_init(void)
{
#ifdef CONFIG_ERPS_API_DEMO
    int i = 0;

    for (i = 0; i < MAX_PORT; i++)
    {
        link_sts[i] = LINK_DOWN;
    }
#endif

    io_write32(CR_GSW_SYS_INT_EN, 0x0);
    io_write32(CR_GSW_SYS_INT_STS, io_read32(CR_GSW_SYS_INT_STS));
    register_isr(IRQ_GSW, erps_gsw_isr);
}

void erps_gsw_isr_port_set(unsigned char port_bitmap, unsigned char enable)
{
    if (enable)
    {
        io_write32(CR_GSW_SYS_INT_EN, (io_read32(CR_GSW_SYS_INT_EN) | port_bitmap));
    }
    else
    {
        io_write32(CR_GSW_SYS_INT_EN, (io_read32(CR_GSW_SYS_INT_EN) & ~(port_bitmap)));
    }
}

void erps_timer_isr(void)
{
    unsigned int now_timer_cmp = 0, next_timer_cmp = 0, now_timer_cnt = 0, new_timer_cnt = 0;
    unsigned int count = 0;
    long long diff = 0;

    record_interrupts(*(int *)erps_data);

    now_timer_cmp = io_read32(CPU_TIMER1_CMP);
    io_write32(CPU_TIMER1_CMP, now_timer_cmp);
    __nds32__dsb();
    next_timer_cmp = (now_timer_cmp + reload);
    count = 1;
    now_timer_cnt = io_read32(CPU_TIMER1_CVR);

    if (now_timer_cnt > now_timer_cmp)
    {
        diff = now_timer_cnt - now_timer_cmp;
    }
    else
    {
        diff = now_timer_cnt + (0xFFFFFFFF - now_timer_cmp + 1);
    }

    while (diff > reload)
    {
        count += 1;
#ifdef CONFIG_ERPS_API_DEMO
        erps_over_count += 1;
#endif
        next_timer_cmp += reload;
        diff -= reload;
    }

    new_timer_cnt = io_read32(CPU_TIMER1_CVR);
    if (new_timer_cnt > now_timer_cnt)
    {
        diff += new_timer_cnt - now_timer_cnt;
    }
    else
    {
        diff += new_timer_cnt + (0xFFFFFFFF - now_timer_cnt + 1);
    }

    while (diff > reload)
    {
        count += 1;
#ifdef CONFIG_ERPS_API_DEMO
        erps_pre_over_count += 1;
#endif
        next_timer_cmp += reload;
        diff -= reload;
    }

    if (diff < reload_jitter)
    {
        count += 1;
#ifdef CONFIG_ERPS_API_DEMO
        erps_pre_dual_count += 1;
#endif
        next_timer_cmp += reload;
    }

    io_write32(CPU_TIMER1_CMP, (unsigned int) next_timer_cmp);
    __nds32__dsb();

#ifdef CONFIG_ERPS_API_DEMO
    erps_total_count += count;
#endif
    erps_1ms_count += count;

    if (erps_goal_count <= erps_1ms_count)
    {
        erps_1ms_count -= erps_goal_count;
        if (timer_callback != NULL)
            timer_callback(erps_data);
    }
}

void erps_timer_init(unsigned int tick, unsigned int jitter)
{
    reload = tick;
    reload_jitter = jitter;

    register_isr(IRQ_CPU_TMR1, erps_timer_isr);
}

void erps_timer_deinit(void)
{
    reload = 0;
    reload_jitter = 0;

    erps_timer_stop();
    unregister_isr(IRQ_CPU_TMR1);
}

void erps_timer_start(unsigned int ms, void *callback_data, timer_callback_t *callback_func)
{
    erps_data = callback_data;
    timer_callback = callback_func;

    io_write32(CPU_TIMER1_CMP, reload);
    io_write32(CPU_TIMER1_CVR, 0);
#ifdef CONFIG_ERPS_API_DEMO
    erps_total_count = 0;
    erps_over_count = 0;
    erps_pre_over_count = 0;
    erps_pre_dual_count = 0;
#endif
    erps_1ms_count = 0;
    erps_goal_count = ms;

    enable_cpu_timer(ERPS_TIMER, 1);
}

void erps_timer_stop(void)
{
    enable_cpu_timer(ERPS_TIMER, 0);

    erps_data = NULL;
    timer_callback = NULL;

    io_write32(CPU_TIMER1_CMP, 0);
    io_write32(CPU_TIMER1_CVR, 0);
#ifdef CONFIG_ERPS_API_DEMO
    erps_total_count = 0;
    erps_over_count = 0;
    erps_pre_over_count = 0;
    erps_pre_dual_count = 0;
#endif
    erps_1ms_count = 0;
    erps_goal_count = 0;
}

unsigned int erps_get_current_tick(void)
{
    unsigned int now_timer_cnt = 0;

    now_timer_cnt = io_read32(CPU_TIMER1_CVR);
    return now_timer_cnt;
}
