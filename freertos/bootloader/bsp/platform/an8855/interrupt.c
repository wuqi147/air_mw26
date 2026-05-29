#include <stdio.h>
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>
#include "interrupt.h"
#include "uart.h"

extern char ISR_TABLE;
typedef void (*isr_t)(void);
static char isISRregistered[IRQ_MAX_NUM]={0};
struct pt_regs
{
    long    PSW;
    long    IPSW;
    long    P_IPSW;
    long    IVB;
    long    EVA;
    long    P_EVA;
    long    ITYPE;
    long    P_ITYPE;
    long    MERR;
    long    IPC;
    long    P_IPC;
    long    OIPC;
    long    r1;
    long    r2;
    long    r3;
    long    r4;
    long    r5;
    long    r6;
    long    r7;
    long    r8;
    long    r9;
    long    r10;
    long    fp;
    long    gp;
    long    sp;
};

#define DEBUG_ROOM      (debug_room_buffer + PT_REGS_SIZE)
#define DEBUG_ROOM_SIZE (0x200)
#define PT_REGS         ((struct pt_regs *) ((unsigned int)DEBUG_ROOM - sizeof(struct pt_regs)))
/*reserve 256 bytes for registers*/
#define PT_REGS_SIZE    (0x100)
/*debug_room_buffer is in section SRAM_code_flashwriter(exe_region_name))*/
static unsigned char debug_room_buffer[DEBUG_ROOM_SIZE+PT_REGS_SIZE] __attribute__((section(".dmem_debugbuffer")));

void show_caller(struct pt_regs *regs)
{
    unsigned int *debug_room = (unsigned int *) DEBUG_ROOM;
    unsigned int *temp =  (unsigned int *) DEBUG_ROOM;

    while (((unsigned int) (temp + 3)) < (((unsigned int) debug_room) + DEBUG_ROOM_SIZE))
    {
        if ((*temp == regs->fp) && (*(temp + 1) == regs->gp))
        {
            printf("--> %16x\n\r", (unsigned int)(*(temp + 2) - sizeof(unsigned int)));
            temp += 3;
        }
        else
            temp += 1;
    }
}

void show_reg(void)
{
    struct pt_regs *regs = (struct pt_regs *)PT_REGS;

    printf("PSW    :%16lx | IPSW   :%16lx | P_IPSW  :%16lx\n\r", regs->PSW, regs->IPSW, regs->P_IPSW);
    printf("IVB    :%16lx | EVA    :%16lx | P_EVA   :%16lx\n\r", regs->IVB, regs->EVA, regs->P_EVA);
    printf("ITYPE  :%16lx | P_ITYPE:%16lx | MERR    :%16lx\n\r", regs->ITYPE, regs->P_ITYPE, regs->MERR);
    printf("IPC    :%16lx | P_IPC  :%16lx | OIPC    :%16lx\n\r", regs->IPC, regs->P_IPC, regs->OIPC);
    printf("r1     :%16lx | r2     :%16lx | r3      :%16lx\n\r", regs->r1, regs->r2, regs->r3);
    printf("r4     :%16lx | r5     :%16lx | r6      :%16lx\n\r", regs->r4, regs->r5, regs->r6);
    printf("r7     :%16lx | r8     :%16lx | r9      :%16lx\n\r", regs->r7, regs->r8, regs->r9);
    printf("r10    :%16lx | fp     :%16lx | gp      :%16lx\n\r", regs->r10, regs->fp, regs->gp);
    printf("sp     :%16lx\n\r", regs->sp);
    show_caller(regs);
}

void SAVE_ALL(void)
{
    asm("move    $r0, $sp");
    asm("la      $sp, %0" : : "i"(DEBUG_ROOM));
    asm("push    $r0");
    asm("push    $gp");
    asm("push    $fp");
    asm("pushm   $r1, $r10");
    asm("mfsr    $r15, $ir11");
    asm("push    $r15");
    asm("mfsr    $r15, $ir10");
    asm("push    $r15");
    asm("mfsr    $r15, $ir9");
    asm("push    $r15");
    asm("mfsr    $r15, $ir8");
    asm("push    $r15");
    asm("mfsr    $r15, $ir7");
    asm("push    $r15");
    asm("mfsr    $r15, $ir6");
    asm("push    $r15");
    asm("mfsr    $r15, $ir5");
    asm("push    $r15");
    asm("mfsr    $r15, $ir4");
    asm("push    $r15");
    asm("mfsr    $r15, $ir3");
    asm("push    $r15");
    asm("mfsr    $r15, $ir2");
    asm("push    $r15");
    asm("mfsr    $r15, $ir1");
    asm("push    $r15");
    asm("mfsr    $r15, $ir0");
    asm("push    $r15");
    asm("move    $sp, $r0");
    asm("move    $r1, $r0");
    asm("la      r0, %0" : : "i"(DEBUG_ROOM));
    asm("addi    r2, $r1, %0" : : "i"(DEBUG_ROOM_SIZE));
asm("copy_stack:");
    asm("lwi     r3, [r1]");
    asm("swi     r3, [r0]");
    asm("addi    r0, r0, 0x4");
    asm("addi    r1, r1, 0x4");
    asm("bne     r1, r2, copy_stack");
}

/*
 *  Exception handlers
 */
void trap_TLB_Fill(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('1');
    while(1);
}
void trap_PTE_Not_Present(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('2');
    while(1);
}
void trap_TLB_Misc(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('3');
    while(1);
}
void trap_TLB_VLPT_Miss(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('4');
    while(1);
}
void trap_Machine_Error(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('5');
    while(1);
}
void trap_Debug_Related(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('6');
    while(1);
}
void trap_General_Exception(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('7');
    while(1);
}
void trap_Syscall(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('8');
    while(1);
}

/*
 *  default irq handlers
 */
void default_irq_handler0(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler1(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler2(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler3(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler4(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler5(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler6(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler7(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler8(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler9(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler10(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler11(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler12(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler13(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler14(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler15(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}
void default_irq_handler16(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler17(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler18(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler19(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler20(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler21(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler22(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler23(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler24(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler25(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler26(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler27(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler28(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler29(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler30(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

void default_irq_handler31(void){
    printf("\nError: %s triggered!\n", __func__);
    while(1);
}

static int check_intr_num (unsigned int intSrcNum)
{
    if (intSrcNum>=IRQ_MAX_NUM) {
        printf("\nERROR: intSrcNum:%d exceeds %d\n", intSrcNum, IRQ_MAX_NUM);
        return -1;
    }
    return 0;
}

/*
 * intSrcNum: defined in platform.h
 * priority: 0~3 (0 is highest priority, 3 is lowest priority)
 */
void set_interrupt_priority (unsigned int intSrcNum, unsigned int priority)
{
    priority &= 0x3;

    if (check_intr_num(intSrcNum)) return;

    if (intSrcNum < 16)
        __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_PRI ) | ( priority << ( ( intSrcNum ) << 1 ) ), NDS32_SR_INT_PRI );
    else
        __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_PRI2 ) | ( priority << ( ( ( intSrcNum ) - 16 ) << 1 ) ), NDS32_SR_INT_PRI2 );

    return;
}

/*
 * intSrcNum: defined in platform.h
 */
void enable_interrupt_mask (unsigned int intSrcNum)
{
    if (check_intr_num(intSrcNum)) return;

    __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_MASK2 ) | ( 1 << ( intSrcNum ) ), NDS32_SR_INT_MASK2 );

    return;
}

void register_isr (unsigned int intSrcNum, isr_t isr)
{
    unsigned int *long_p = (unsigned int *)&ISR_TABLE;

    if (check_intr_num(intSrcNum)) return;

    if (isISRregistered[intSrcNum]) {
        printf("\nError: interrupt %d is already registered\n", intSrcNum);
        return;
    }

    long_p[intSrcNum] = (unsigned int)isr;

    enable_interrupt_mask(intSrcNum);

    isISRregistered[intSrcNum]=1;

    return;
}

void unregister_isr (unsigned int intSrcNum)
{
    if (check_intr_num(intSrcNum)) return;

    disable_interrupt_mask(intSrcNum);

    isISRregistered[intSrcNum]=0;

    return;
}

/*
 * intSrcNum: defined in platform.h
 */
void disable_interrupt_mask (unsigned int intSrcNum)
{
    if (check_intr_num(intSrcNum)) return;

    __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_MASK2 ) & (~( 1 << ( intSrcNum ))), NDS32_SR_INT_MASK2 );

    return;
}

/*
 * when triggering SW_Int, FreeRTOS_SWI_Handler (in portasm.S) then vTaskSwitchContext (in tasks.c) will be called.
 *      Note: SW_Int, defined in portYIELD (in portmacro.h), is frequently triggered to switch tasks in freeRTOS.
 */
void air_trigger_swint(void)
{
    __nds32__set_pending_swint();
}

int isIntrEdgeTriggered (unsigned int intSrc)
{
    return ( __nds32__mfsr( NDS32_SR_INT_TRIGGER ) | (1 << ( intSrc )) );
}

void clearIntrPendingBit (unsigned int intSrc)
{
    __nds32__mtsr_isb( 1 << ( intSrc ), NDS32_SR_INT_PEND2 );
}

