/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Platform includes. */
#include <platform.h>
#include "timer.h"

#define TICK_TIMER  0

static int cpuTmrIsrCnt=0;
int tmpCpuTimerCnt = 0;

/*-----------------------------------------------------------*/
#define USER_TIMESTAMP_PRECISE 1

#if USER_TIMESTAMP_PRECISE
volatile unsigned int g_air_user_tick_precise_HB;
volatile unsigned int g_air_user_tick_precise;
#endif
/*-----------------------------------------------------------*/

/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function. This file contains a function
 * that is suitable for use on the Andes AE210P.
 */
void vConfigureTickInterrupt( void )
{
    cpu_timer_init(TICK_TIMER, ENABLE, configCPUTMR_CNT_PER_TICK);

#if USER_TIMESTAMP_PRECISE
    g_air_user_tick_precise_HB = 0;
    g_air_user_tick_precise = 0;
#endif
}

#if USER_TIMESTAMP_PRECISE

unsigned long long vGetSystem1msTick(void)
{
    unsigned int now_timer_cnt;
    unsigned long long result_long;

    now_timer_cnt = get_cpu_timer_cnt(TICK_TIMER);
    if( now_timer_cnt < g_air_user_tick_precise)
    {
        ++g_air_user_tick_precise_HB;
    }
    g_air_user_tick_precise = now_timer_cnt;

    result_long = 0x100000000ULL * g_air_user_tick_precise_HB;
    result_long += now_timer_cnt;

    result_long /= (CPUTMRCLK/KHz);

    return result_long;
}

unsigned long long vGetSystemTick(void)
{
    unsigned long long result_long = vGetSystem1msTick();
    result_long /= (portTICK_PERIOD_MS);
    return result_long;
}

#endif // USER_TIMESTAMP_PRECISE

/*-----------------------------------------------------------*/

void vClearTickInterrupt( void )
{
    /* Manually clear interrupt for edge-triggered interrupt.
    Because this API is also used by task for not handling IRQ
    (for configUSE_TICKLESS_IDLE), we can't just rely on HW auto clear. */
    if( isIntrEdgeTriggered(IRQ_CPU_TMR0) )
        clearIntrPendingBit(IRQ_CPU_TMR0);
}

/*-----------------------------------------------------------*/

void vEnableTickInterrupt( long xEnable )
{
    if (xEnable == pdTRUE)
        enable_cpu_timer(TICK_TIMER, ENABLE);
    else
        enable_cpu_timer(TICK_TIMER, DISABLE);
}
/*-----------------------------------------------------------*/

void vSetTickReload( uint32_t ulReload )
{
    set_cpu_timer_cmpTickVal(TICK_TIMER, ulReload);
}
/*-----------------------------------------------------------*/

void vUpdateTickReload(void)
{
    unsigned int now_timer_cnt = get_cpu_timer_cnt(TICK_TIMER);
    set_cpu_timer_cmpTickVal(TICK_TIMER, now_timer_cnt+configCPUTMR_CNT_PER_TICK);
    record_interrupts(IRQ_CPU_TMR0);
    tmpCpuTimerCnt++;

#if USER_TIMESTAMP_PRECISE
    if( now_timer_cnt < g_air_user_tick_precise)
    {
        ++g_air_user_tick_precise_HB;
    }
    g_air_user_tick_precise = now_timer_cnt;
#endif

    if (isWdogEnabled()) {
        cpuTmrIsrCnt++;
        if (cpuTmrIsrCnt>=configTICK_RATE_HZ) {
            cpuTmrIsrCnt=0;

            wdog_sec_handler();
        }
    }
}
/*-----------------------------------------------------------*/

uint32_t ulGetTickCounter( void )
{
    return get_cpu_timer_cnt(TICK_TIMER);
}
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
}
/*-----------------------------------------------------------*/

uint32_t ulGetRunTimeCounterValue( void )
{
    uint32_t CounterValue;
    uint32_t ulPreviousMask;

    configCLEAR_TICK_INTERRUPT();
    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        CounterValue = vGetSystem1msTick()/(100); // unit: 100 ms
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );

    return CounterValue;
}
