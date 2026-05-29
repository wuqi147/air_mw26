/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"
#include "uart.h"

#include "bootmenu.h"
#include "spinorwrite.h"

#ifdef AIR_SUPPORT_ACE
#include "ace_main.h"
#endif

/* GLOBAL VARIABLE DECLARATIONS
 */
#ifdef AIR_SUPPORT_CPU_PROCESS
extern void pdma_init (void);
extern int mac_rcv_init(TaskHandle_t *xHandle);
TaskHandle_t g_xMacRcvHandle = NULL;
#endif

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/*-----------------------------------------------------------*/
typedef void (*isr_t)(void);
extern void air_wdog_isr(void);
extern void airInitHardware(void);
extern void wdog_init (unsigned int enable, unsigned int timeOut/*ms*/, unsigned int wdogThld/*ms*/);
extern void register_isr (unsigned int intSrcNum, isr_t isr);
extern void create_queue_recv_task(void);
TaskHandle_t wdog_taskHandle;

static void airSetupHardware( void )
{
    airInitHardware();
    return;
}

static void prvSetupHardware( void )
{
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
    state.  Interrupts are automatically enabled when the scheduler is
    started. */
    portDISABLE_INTERRUPTS();
}

static void air_wdog_task( void *pvParameters )
{
    /* init wdog:
     * if cpu doesn't feed wdog for [Wdog_Timeout_Counter] secs, system will reboot. In last [Wdog_Interrupt_Counter] secs, wdog interrupt will be issued */
    wdog_init(1, (Wdog_Timeout_Counter*1000), (Wdog_Interrupt_Counter*1000));
    register_isr(IRQ_TMR3, air_wdog_isr);

    for( ;; )
    {
        vTaskSuspend(NULL); /* NULL: suspend ownself */
        air_wdog_kick();
    }
}

void create_wdog_task(void)
{
    xTaskCreate(air_wdog_task,                 /* The function that implements the task. */
                "WDOG",                         /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                configMINIMAL_STACK_SIZE,       /* The size of the stack to allocate to the task. */
                NULL,                           /* The parameter passed to the task - not used in this simple case. */
                (tskIDLE_PRIORITY),             /* The priority assigned to the task. */
                &wdog_taskHandle );

    return;
}

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();
    airSetupHardware();

    /* create tasks */
    create_queue_recv_task();
    create_wdog_task();

    spinor_write_init();
    create_flash_conf_task();

    if(fw_upgrade() < 0)
    {
        printf("firmware upgrade failed!\n");
    }

#ifdef AIR_SUPPORT_CPU_PROCESS
    mac_rcv_init(&g_xMacRcvHandle);
    pdma_init();
#endif

#if defined(AIR_SUPPORT_ACE)
    dumb_switch_init();
    ACE_init();
#endif

    air_uart_set_isr_enable(pdFALSE);
    bootmenu_show(BOOTMENU_AUTOBOOT_DELAY_TIME);
    air_uart_set_isr_enable(pdTRUE);

    /* start tasks scheduling */
    vTaskStartScheduler();
    while(1); /* Don't expect to reach here. */

    /* Don't expect to reach here. */
    return 0;
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
volatile unsigned long ul = 0;

    ( void ) pcFile;
    ( void ) ulLine;

    printf( "ASSERT! Line %d, file %s\r\n", ( int )ulLine, pcFile );

    taskENTER_CRITICAL();
    {
        /* Set ul to a non-zero value using the debugger to step out of this
        function. */
        while( ul == 0 )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
    size of the     heap available to pvPortMalloc() is defined by
    configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
    API function can be used to query the size of free heap space that remains
    (although it does not provide information on how the remaining heap might be
    fragmented).  See http://www.freertos.org/a00111.html for more
    information. */
    vAssertCalled( __FILE__, __LINE__ );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    vAssertCalled( __FILE__, __LINE__ );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeHeapSpace;

    /* This is just a trivial example of an idle hook.  It is called on each
    cycle of the idle task.  It must *NOT* attempt to block.  In this case the
    idle task just queries the amount of FreeRTOS heap that remains.  See the
    memory management section on the http://www.FreeRTOS.org web site for memory
    management options.  If there is a lot of heap memory free then the
    configTOTAL_HEAP_SIZE value in FreeRTOSConfig.h can be reduced to free up
    RAM. */
    xFreeHeapSpace = xPortGetFreeHeapSize();

    /* Remove compiler warning about xFreeHeapSpace being set but never used. */
    ( void ) xFreeHeapSpace;
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{

}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/

void vPreSleepProcessing( unsigned long uxModifiableIdleTime )
{
    /* Called by the kernel before it places the MCU into a sleep mode because
    configPRE_SLEEP_PROCESSING() is #defined to vPreSleepProcessing().

    NOTE:  Additional actions can be taken here to get the power consumption
    even lower.  For example, peripherals can be turned off here, and then back
    on again in the post sleep processing function.  For maximum power saving
    ensure all unused pins are in their lowest power state. */

    /* Avoid compiler warnings about the unused parameter. */
    ( void ) uxModifiableIdleTime;
}
/*-----------------------------------------------------------*/

void vPostSleepProcessing( unsigned long uxModifiableIdleTime )
{
    /* Called by the kernel when the MCU exits a sleep mode because
    configPOST_SLEEP_PROCESSING is #defined to vPostSleepProcessing(). */

    /* Avoid compiler warnings about the unused parameter. */
    ( void ) uxModifiableIdleTime;
}
