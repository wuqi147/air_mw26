
#include <nds32_init.inc>
#include "nds32_defs.h"
#include <platform.h>

!********************************************************************************************************
!	Macro for exception/interruption vectors
!********************************************************************************************************

	.macro VECTOR name
	.weak   trap_\name
	.set    trap_\name, default_exception_vsr
	.align 4	! 16-byte alignment
__vector_\name:
	push    $r0
	la      $r0, trap_\name
	jr5     $r0
	.endm

	.macro INTERRUPT num
	.align 4
__vector_HW\num:
	.if     \num == IRQ_SWI_VECTOR
	pushm   $r28,$r30
	la      $fp, FreeRTOS_SWI_Handler
	jr5     $fp
	.else
	.weak   interrupt_HW\num
	.set    interrupt_HW\num, default_interrupt_vsr
	pushm   $r0, $r5
	li      $r0, \num
	la      $r1, interrupt_HW\num
	jr5     $r1
	.endif
	.endm

!********************************************************************************************************
!	Create Vector Entry Table
!********************************************************************************************************

	! Define standard NDS32 vector table entry point of
	! exception/interruption vectors

	.section .nds32_init, "ax"

	.global exception_vectors
	.global exception_vectors_end

exception_vectors:
	! Exception vectors
	b       _start              !  (0) Trap Reset/NMI
	VECTOR  TLB_Fill            !  (1) Trap TLB fill
	VECTOR  PTE_Not_Present     !  (2) Trap PTE not present
	VECTOR  TLB_Misc            !  (3) Trap TLB misc
	VECTOR  TLB_VLPT_Miss       !  (4) Trap TLB VLPT miss
	VECTOR  Machine_Error       !  (5) Trap Machine error
	VECTOR  Debug_Related       !  (6) Trap Debug related
	VECTOR  General_Exception   !  (7) Trap General exception
	VECTOR  Syscall             !  (8) Syscall

	! Interrupt vectors
	.altmacro
	.set    irqno, 0
	.rept   VECTOR_NUMINTRS
	INTERRUPT %irqno
	.set    irqno, irqno+1
	.endr

	.align 4
exception_vectors_end:

!******************************************************************************************************
!	Start Entry
!******************************************************************************************************
	.section .text
	.global _start
	.type   _start, @function
_start:
	!************************** Begin of do-not-modify **************************
	! Initialize the registers used by the compiler
	! nds32_init will Initialize $gp and $sp

	nds32_init	! NDS32 startup initial macro in <nds32_init.inc> in toolchain

	!*************************** End of do-not-modify ***************************

	/* ---------------------------------------------
	 * SPI Initial
	 * ---------------------------------------------
	 */
#ifdef FLASH_AVL_TEST
	li      r0, DMEM_BASE
	li      r1, 0x00000146
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0xb8000058
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x009220b6
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x08000064
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x03000146
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x009240b6
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x08000064
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x9c000058
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x009260b6
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x08000064
	swi     r1, [r0]
	addi    r0, r0, 0x4
	li      r1, 0x1edd0092
	swi     r1, [r0]
	lwi     r2, [r0]
	li      r0, DMEM_BASE
	li      r1, SPI_SPEED
	li      r2, SPI_READ_CMD
	li      r3, SPI_CLOCK_EDGE
	jral    r0

	li      r0, 0x20000000
	li      r1, 0xf246
	li      r3, 0x10000098
	li      r4, 0
cloop:
	lwi     r2, [r0]
	dsb
	beq     r2, r1, cloop_end
	addi    r4, r4, 0x1
	b       cloop
cloop_end:
	swi     r4, [r3]
	dsb
#endif

#ifdef CFG_CACHE_ENABLE
	! disable I/D caches
	mfsr    $r0, $CACHE_CTL
	li      $r1, ~(CACHE_CTL_mskIC_EN | CACHE_CTL_mskDC_EN)
	and     $r0, $r0, $r1
	mtsr    $r0, $CACHE_CTL
#endif

	! Do system low level setup. It must be a leaf function.
	bal     _nds32_init_mem

	! Clear bss
	! We do this on a word basis.
	! Currently, the default linker script guarantee
	! the __bss_start/_end boundary word-aligned.
	
	la      $r0, __bss_start
	la      $r1, _end
	sub     $r2, $r1, $r0       ! $r2: Size of .bss
	beqz    $r2, clear_end

	andi    $r7, $r2, 0x1f      ! $r7 = $r2 mod 32
	movi    $r3, 0
	movi    $r4, 0
	movi    $r5, 0
	movi    $r6, 0
	movi    $r8, 0
	movi    $r9, 0
	movi    $r10, 0
	beqz    $r7, clear_loop     !if $r7 == 0, bss_size%32 == 0
	sub     $r2, $r2, $r7

first_clear:
	swi.bi  $r3, [$r0], #4      !clear each word
	addi    $r7, $r7, -4
	bnez    $r7, first_clear
	li      $r1, 0xffffffe0
	and     $r2, $r2, $r1       !check bss_size/32 == 0 or not
	beqz    $r2, clear_end      !if bss_size/32 == 0 , needless to clear

clear_loop:
	smw.bim $r3, [$r0], $r10    !clear each 8 words
	addi    $r2, $r2, -32
	bgez    $r2, clear_loop

clear_end:
	! andes cpu bring up
	bal     cpu_bringup

	! Infinite loop, if returned accidently
1:
	b       1b

	/* Default exceptions / interrupts handler */
default_exception_vsr:
default_interrupt_vsr:
die:	b       die
