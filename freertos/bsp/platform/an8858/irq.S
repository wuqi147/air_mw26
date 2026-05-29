! -----------------------------------------------------------------------------
! irq.S
!
! Copyright (c) 2005-2018, Andes Techonology Corporation
! All rights reserved.
! -----------------------------------------------------------------------------

#include "ISR_Support.h"
#include <platform.h>

!********************************************************************************************************
!	Setup HW Interrupt vector handler
!********************************************************************************************************

	.macro  SET_HWISR num, handler
	.global interrupt_HW\num
	.set    interrupt_HW\num, \handler
	.endm

	! Setup the AE3XX HW interrupt vector handler

	SET_HWISR IRQ_GPIO, 		interrupt_hipri_vsr
	SET_HWISR IRQ_TMR0,  		interrupt_hipri_vsr
	SET_HWISR IRQ_TMR1,       	interrupt_hipri_vsr
	SET_HWISR IRQ_TMR2,      	interrupt_hipri_vsr
	SET_HWISR IRQ_TMR3,      	interrupt_hipri_vsr
	SET_HWISR IRQ_UART,       	interrupt_hipri_vsr
	SET_HWISR IRQ_CPU_TMR0,     interrupt_hipri_vsr
	SET_HWISR IRQ_CPU_TMR1,     interrupt_hipri_vsr
	SET_HWISR IRQ_BUS_TOUT,    	interrupt_hipri_vsr
	SET_HWISR IRQ_RG_ABIST,		interrupt_hipri_vsr
	SET_HWISR IRQ_SFC,			interrupt_hipri_vsr
	SET_HWISR IRQ_I2C2RBUS,		interrupt_hipri_vsr
	SET_HWISR IRQ_GDMP,      	interrupt_hipri_vsr
	SET_HWISR IRQ_GDMA,      	interrupt_hipri_vsr
	SET_HWISR IRQ_PDMA,			interrupt_hipri_vsr
	SET_HWISR IRQ_GSW,			interrupt_hipri_vsr
	!SET_HWISR IRQ_SWI_VECTOR,	interrupt_hipri_vsr
	SET_HWISR IRQ_RESV17,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV18,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV19,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV20,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV21,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV22,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV23,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV24,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV25,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV26,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV27,		interrupt_hipri_vsr
	SET_HWISR IRQ_EXT_00,		interrupt_hipri_vsr
	SET_HWISR IRQ_EXT_01,		interrupt_hipri_vsr
	SET_HWISR IRQ_EXT_02,		interrupt_hipri_vsr
	SET_HWISR IRQ_RESV31,		interrupt_hipri_vsr

!********************************************************************************************************
!       Highest priority HW Interrupt vector service routine (Non-Nested interrupt)
!********************************************************************************************************

interrupt_hipri_vsr:
	! We enter here with the orginal r0~r5 is saved and
	! r0 is HW interrup vector number.

	portSAVE_CALLER

#if ( configSUPPORT_IFC == 1 )
	mfusr   $r1, $IFC_LP
        pushm   $r0, $r1                /* $0 is dummy for 8-byte alignment stack pointer */
#endif

#ifdef IRQ_DBG_SUPPORT
    mfsr $r1, $int_mask2
    pushm $r0, $r1

    li $r1, 1
    slli $r1, $r1, IRQ_TMR3
    mtsr $r1, $int_mask2
    setgie.e
    dsb
#endif
	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la      $r1, ISR_TABLE
	lw      $r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral    $r1

#ifdef IRQ_DBG_SUPPORT
    setgie.d
    dsb

    popm $r0, $r1
    mtsr $r1, $int_mask2
#endif


#if ( configSUPPORT_IFC == 1 )
	popm    $r0, $r1
	mtusr   $r1, $IFC_LP
#endif

	portRESTORE_CALLER

	iret


!********************************************************************************************************
!       Interrupt default handler
!********************************************************************************************************

	! Macro to define default handlers
	.macro WEAK_DEFAULT weak_sym, default_handler
	.weak \weak_sym
	.set \weak_sym ,\default_handler
	.endm

	! Define default handler to be weak symbol.
	! They can be overwritten by other handlers
	WEAK_DEFAULT gpio_irq_handler,		default_irq_handler0
	WEAK_DEFAULT timer0_irq_handler,	default_irq_handler1
	WEAK_DEFAULT timer1_irq_handler,	default_irq_handler2
	WEAK_DEFAULT timer2_irq_handler,	default_irq_handler3
	WEAK_DEFAULT timer3_irq_handler,	default_irq_handler4
	WEAK_DEFAULT uart_irq_handler,		default_irq_handler5
	WEAK_DEFAULT cputmr0_irq_handler,	FreeRTOS_Tick_Handler
	WEAK_DEFAULT cputmr1_irq_handler,	default_irq_handler7
	WEAK_DEFAULT bus_tout_irq_handler,	default_irq_handler8
	WEAK_DEFAULT rg_abist_irq_handler,	default_irq_handler9
	WEAK_DEFAULT sfc_irq_handler,		default_irq_handler10
	WEAK_DEFAULT i2c2rbus_irq_handler ,	default_irq_handler11
	WEAK_DEFAULT gdmp_irq_handler,		default_irq_handler12
	WEAK_DEFAULT gdma_irq_handler,		default_irq_handler13
	WEAK_DEFAULT pdma_irq_handler,		default_irq_handler14
	WEAK_DEFAULT rsv15_irq_handler,		default_irq_handler15
	WEAK_DEFAULT swi_irq_handler,		default_irq_handler16
	WEAK_DEFAULT rsv17_irq_handler,		default_irq_handler17
	WEAK_DEFAULT rsv18_irq_handler,		default_irq_handler18
	WEAK_DEFAULT rsv19_irq_handler,		default_irq_handler19
	WEAK_DEFAULT rsv20_irq_handler,		default_irq_handler20
	WEAK_DEFAULT rsv21_irq_handler,		default_irq_handler21
	WEAK_DEFAULT rsv22_irq_handler,		default_irq_handler22
	WEAK_DEFAULT rsv23_irq_handler,		default_irq_handler23
	WEAK_DEFAULT rsv24_irq_handler,		default_irq_handler24
	WEAK_DEFAULT rsv25_irq_handler,		default_irq_handler25
	WEAK_DEFAULT rsv26_irq_handler,		default_irq_handler26
	WEAK_DEFAULT rsv27_irq_handler,		default_irq_handler27
	WEAK_DEFAULT ext00_irq_handler,		default_irq_handler28
	WEAK_DEFAULT ext01_irq_handler,		default_irq_handler29
	WEAK_DEFAULT ext02_irq_handler,		default_irq_handler30
	WEAK_DEFAULT rsv31_irq_handler,		default_irq_handler31

!********************************************************************************************************
!       Interrupt handler Table
!********************************************************************************************************

	.section .data
	.align  2

	! These tables contain the isr pointers used to deliver interrupts
	.global ISR_TABLE
ISR_TABLE:
	.long   gpio_irq_handler		!  0: GPIO
	.long   timer0_irq_handler		!  1: TIMER0
	.long   timer1_irq_handler		!  2: TIMER1
	.long   timer2_irq_handler		!  3: TIMER2
	.long   timer3_irq_handler		!  4: TIMER3
	.long   uart_irq_handler		!  5: UART
	.long   cputmr0_irq_handler		!  6: CPU_TMR0
	.long   cputmr0_irq_handler		!  7: CPU_TMR1
	.long   bus_tout_irq_handler	!  8: RBUS/PBUS TIMEOUT
	.long   rg_abist_irq_handler	!  9: ABIST TIMEOUT
	.long   sfc_irq_handler			! 10: SFC
	.long   i2c2rbus_irq_handler	! 11: I2C to RBUS
	.long   gdmp_irq_handler		! 12: GDMP
	.long   gdma_irq_handler		! 13: GDMA
	.long   pdma_irq_handler		! 14: PDMA
	.long   rsv15_irq_handler		! 15: Reserved15
	.long   swi_irq_handler			! 16: SWI
	.long   rsv17_irq_handler		! 17: Reserved17
	.long   rsv18_irq_handler		! 18: Reserved18
	.long   rsv19_irq_handler		! 19: Reserved19
	.long   rsv20_irq_handler		! 20: Reserved20
	.long   rsv21_irq_handler		! 21: Reserved21
	.long   rsv22_irq_handler		! 22: Reserved22
	.long   rsv23_irq_handler		! 23: Reserved23
	.long   rsv24_irq_handler		! 24: Reserved24
	.long   rsv25_irq_handler		! 25: Reserved25
	.long   rsv26_irq_handler		! 26: Reserved26
	.long   rsv27_irq_handler		! 27: Reserved27
	.long   ext00_irq_handler		! 28: Reserved28
	.long   ext01_irq_handler		! 29: Reserved29
	.long   ext02_irq_handler		! 30: Reserved30
	.long   rsv31_irq_handler		! 31: Reserved31

	! Interrupt stack
	.section .bss

	.skip 1024
	.align 3
xISRStack:
	.end
