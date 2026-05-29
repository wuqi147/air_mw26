#ifndef INTERNAL_FEATURE_H
#define INTERNAL_FEATURE_H

/*************************************************/
// Below definitions are for platform debug

#define SYS_INTERNAL_DEBUG
#ifdef SYS_INTERNAL_DEBUG
#define SYS_MCU_DCACHE_DISABLE
#endif

//#define SYS_LOG_2_FLASH
#ifdef SYS_LOG_2_FLASH
#define SYS_LOG_2_FLASH___ADDR       (TempSystemBase + TempSystemSize)  // customer todo
#define SYS_LOG_2_FLASH___RECORD_NUM (4)
#define SYS_LOG_2_FLASH___VIA_TFTP
#endif

#define SYS_MONITOR_DIS_INTERRUPT
#ifdef SYS_MONITOR_DIS_INTERRUPT
#define SYS_SCREG_WF1 0x10005014
#endif

#define SYS_MONITOR_WDOG_KICK
#ifdef SYS_MONITOR_WDOG_KICK
#define SYS_MONITOR_WDOG_KICK___TASK_STATS_PC
#endif

#define SYS_HEAP_CHECK_EN
#ifndef __ASSEMBLER__
#ifdef SYS_HEAP_CHECK_EN
extern void sys_heap_check(void);
#define SYS_HEAP_CHECK sys_heap_check();
#else
#define SYS_HEAP_CHECK
#endif
#endif

/*************************************************/
// Below definitions are for sdk shrink sram
#define SRAM_SHRINK_ENABLE 1
#if SRAM_SHRINK_ENABLE

#define SRAM_SHRINK___XMODEM_BUF
#define SRAM_SHRINK___XMODEM_CMD_REMOVE

#endif

/*************************************************/
#define NON_FIXED_HEAP_SIZE
#ifdef NON_FIXED_HEAP_SIZE
//#define NON_FIXED_HEAP_SIZE_DBG
#endif
/*************************************************/
//#define DBG_LOG printf("%s %d:",__FUNCTION__, __LINE__ ); printf
#define DBG_LOG(...)
/*************************************************/
#define CPU_TIMER_HW_FIX_WORKAROUND
/*************************************************/
#if 1
#define GDMPSRAM_DATA __attribute__((section (".gdmpdata")))
#define GDMPSRAM_BSS  __attribute__((section (".gdmpbss")))
#else
#define GDMPSRAM_DATA
#define GDMPSRAM_BSS
#endif
/*************************************************/


#endif
