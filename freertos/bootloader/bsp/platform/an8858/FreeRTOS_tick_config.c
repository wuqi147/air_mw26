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
#include "interrupt.h"

#define TICK_TIMER  0

extern int isWdogEnabled(void);
extern TaskHandle_t wdog_taskHandle;
static int cpuTmrIsrCnt=0;
int tmpCpuTimerCnt = 0;

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
}
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
    set_cpu_timer_cmpTickVal(TICK_TIMER, get_cpu_timer_cnt(TICK_TIMER)+configCPUTMR_CNT_PER_TICK);
    record_interrupts(IRQ_CPU_TMR0);
    tmpCpuTimerCnt++;
    if (isWdogEnabled()) {
        cpuTmrIsrCnt++;
        if (cpuTmrIsrCnt>=configTICK_RATE_HZ) {
            cpuTmrIsrCnt=0;
            xTaskResumeFromISR(wdog_taskHandle);
        }
    }
}
/*-----------------------------------------------------------*/

uint32_t ulGetTickCounter( void )
{
	return get_cpu_timer_cnt(TICK_TIMER);
}
