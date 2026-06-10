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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#include "platform.h"
#include "air_rtos_config.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/*
 * The FreeRTOS Quark port implements a full interrupt nesting model.
 *
 * Interrupts that are assigned a priority at or below
 * configMAX_API_CALL_INTERRUPT_PRIORITY can call interrupt safe API functions
 * and will nest.
 *
 * Interrupts that are assigned a priority above
 * configMAX_API_CALL_INTERRUPT_PRIORITY cannot call any FreeRTOS API functions,
 * will nest, and will not be masked by FreeRTOS critical sections (although all
 * interrupts are briefly masked by the hardware itself on interrupt entry).
 *
 * FreeRTOS functions that can be called from an interrupt are those that end in
 * "FromISR".  FreeRTOS maintains a separate interrupt safe API to enable
 * interrupt entry to be shorter, faster, simpler and smaller.
 *
 * The NDS32 port implements 4 unique interrupt priorities.  For the purpose of
 * setting configMAX_API_CALL_INTERRUPT_PRIORITY 3 represents the lowest
 * priority (higher priorities are lower numeric values).
 *
 * User definable interrupt priorities range from 0 (the highest) to 3 (the
 * lowest).
 */
#define configMAX_API_CALL_INTERRUPT_PRIORITY	0

/* The NDS32 port utilizes a software interrupt to perform context switch, then
 * define configIRQ_SWI_VECTOR in FreeRTOSConfig.h to specify the platform software
 * interrupt number. */
#define configIRQ_SWI_VECTOR        			IRQ_SWI_VECTOR


#define configCPU_CLOCK_HZ						( CPUFREQ )
#define configPERIPHERAL_CLOCK_HZ				( BUSCLKFREQ )
#define configCPUTMR_CLOCK_HZ				    ( CPUTMRCLK )

#define configUSE_TICKLESS_IDLE					0

#define configCPUTMR_CNT_PER_TICK			    (configCPUTMR_CLOCK_HZ/configTICK_RATE_HZ)

/* task stack configuration */
#define configMACRCV_STACK_SIZE					( 384 )
#define configCMD_STACK_SIZE                    ( 864 )

#endif /* FREERTOS_CONFIG_H */

