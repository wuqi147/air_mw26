/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "spinorwrite.h"
#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include <platform.h>

extern void io_write32(unsigned int addr, unsigned int vlaue);
extern unsigned int io_read32(unsigned int addr);
extern unsigned char io_read8(unsigned int addr);
extern unsigned long ISR_TABLE;
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
extern char __dmem_flashbuffer_vma;
#define DEBUG_ROOM      (&__dmem_flashbuffer_vma + 0x100)  // overlap with g_flash_buf, add offset 0x100 to fit struct pt_regs
#define DEBUG_ROOM_SIZE 0x200
#define PT_REGS         ((struct pt_regs *) (DEBUG_ROOM - sizeof(struct pt_regs)))

void disable_interrupt_mask (unsigned int intSrcNum);


typedef void (*isr_t)(void);
GDMPSRAM_DATA static char isISRregistered[IRQ_MAX_NUM]={0};

#define ONLY_COMPARE_GP 1
#define STACK_DUMP      1
#define LOG_ADDR2LINE   1

extern void prvTaskExitError( void );
void show_caller(struct pt_regs *regs)
{
#if LOG_ADDR2LINE
    int dump_function_cnt = 0;
#endif
    long *debug_room = (long *) DEBUG_ROOM;
    long *temp =  (long *) DEBUG_ROOM;

    while (((long) (temp + 3)) < (((long) debug_room) + DEBUG_ROOM_SIZE))
    {
#if ONLY_COMPARE_GP
        if ((*(temp + 1) == regs->gp))
#else
        if ((*temp == regs->fp) && (*(temp + 1) == regs->gp))
#endif
        {
            if( ((long)prvTaskExitError) == (*(temp + 2)) )
            {
                printf("--> %16lx\n\r", *(temp + 2));
            }
            else
            {
                printf("--> %16lx\n\r", (*(temp + 2) - sizeof(long)));
            }

            temp += 3;
#if LOG_ADDR2LINE
            ++dump_function_cnt;
#endif
        }
        else
        {
            temp += 1;
        }
    }

#if STACK_DUMP
    {
        printf("\n\rStackDump:");
        long next_line = 0;
        temp =  (long *) DEBUG_ROOM;
        while ((temp) < ( debug_room + (DEBUG_ROOM_SIZE/8)))
        {
            next_line %= 4;

            if(next_line==0)
            {
                printf("\n\r%08x", *temp);
            }
            else
            {
                printf(" %08x ", *temp);
            }

            next_line += 1;
            temp += 1;
        }
        printf("\n\r");
    }
#endif

#if LOG_ADDR2LINE
    if(dump_function_cnt)
    {
        printf("\n\r");
        printf("Use below cmd to dump more detail backtrace\n\r");
        printf("addr2line -e airRTOSSystem.elf -f -a -p -s");

        temp =  (long *) DEBUG_ROOM;
        while (((long) (temp + 3)) < (((long) debug_room) + DEBUG_ROOM_SIZE))
        {
#if ONLY_COMPARE_GP
            if ((*(temp + 1) == regs->gp))
#else
            if ((*temp == regs->fp) && (*(temp + 1) == regs->gp))
#endif
            {
                if( ((long)prvTaskExitError) == (*(temp + 2)) )
                {
                    printf(" %8lx", *(temp + 2));
                }
                else
                {
                    printf(" %8lx", (*(temp + 2) - sizeof(long)));
                }

                temp += 3;
            }
            else
            {
                temp += 1;
            }
        }

        printf("\n\r");
        printf("\n\r");
    }
#endif
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

void DUMP_STACK(void)
{
    struct pt_regs *regs = PT_REGS;
    SAVE_ALL();
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

#ifdef SYS_LOG_2_FLASH

#define SYS_LOG_2_FLASH__PATTERN    (0xbeef0bbb)
#define RING_BUFFER_SIZE            (4096)
#define RING_BUFFER_IS_OVERFLOW     (0x0001)

unsigned char g_sys_log_2_flash__on = 0;
unsigned char g_sys_log_2_flash__ring_buffer[RING_BUFFER_SIZE] = {0};

struct ring_buffer {
    uint32_t pattern;
    uint32_t sequence;
    uint16_t tag1;
    uint16_t tag2;
    uint16_t index;
    uint16_t flags;
};

#define RING_BUFFER_BEG_INDEX sizeof(struct ring_buffer)

void sys_log_2_flash_init(void)
{
    struct ring_buffer *ring_control_block = (struct ring_buffer *) &g_sys_log_2_flash__ring_buffer;

    ring_control_block->pattern = SYS_LOG_2_FLASH__PATTERN;
    ring_control_block->index = sizeof(struct ring_buffer);
    ring_control_block->flags = 0;

    printf("sys_log_2_flash 0x%08x %d\n", SYS_LOG_2_FLASH___ADDR, SYS_LOG_2_FLASH___RECORD_NUM);

#ifdef SYS_LOG_2_FLASH___VIA_TFTP
    printf("  tftp_put 192.168.0.199 DUT_exception.log 0x%08x 4096\n", SYS_LOG_2_FLASH___ADDR);
#endif
    // debug cmd
    //(1.5M)
    //  tftp_put 192.168.0.199 DUT_exception.log 0x204b0000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x204b1000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x204b2000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x204b3000 4096
    //(2.5M)
    //  tftp_put 192.168.0.199 DUT_exception.log 0x207b0000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x207b1000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x207b2000 4096
    //  tftp_put 192.168.0.199 DUT_exception.log 0x207b3000 4096

    g_sys_log_2_flash__on = 1;
}

void sys_log_2_flash__write(unsigned char c)
{
    if(!g_sys_log_2_flash__on)
    {
        return;
    }

    struct ring_buffer *ring_control_block = (struct ring_buffer *) &g_sys_log_2_flash__ring_buffer;
    g_sys_log_2_flash__ring_buffer[ring_control_block->index++] = c;

    if(ring_control_block->index == RING_BUFFER_SIZE)
    {
        ring_control_block->flags |= RING_BUFFER_IS_OVERFLOW;
        ring_control_block->index = RING_BUFFER_BEG_INDEX;
    }
}

static uint32_t sys_log_2_flash__getseq(uint32_t index)
{
    uint32_t seq = 0;

    uint32_t flash_addr = SYS_LOG_2_FLASH___ADDR + index*4096;
    struct ring_buffer *ring_control_block = (struct ring_buffer *) flash_addr;
    if(ring_control_block->pattern == SYS_LOG_2_FLASH__PATTERN)
    {
        seq = ring_control_block->sequence;
    }

    return seq;
}

static void sys_log_2_flash__save(uint32_t index, uint32_t new_seq)
{
    printf("%s %d %d\n", __FUNCTION__, index, new_seq);

    uint32_t flash_addr = SYS_LOG_2_FLASH___ADDR + index*4096;

    struct ring_buffer *ptr = (struct ring_buffer *) &g_sys_log_2_flash__ring_buffer;

    ptr->sequence = new_seq;

    uint32_t beg;
    uint32_t end = ptr->index;
    if(ptr->flags & RING_BUFFER_IS_OVERFLOW)
    {
        beg = end + 1;

        if(beg == RING_BUFFER_SIZE)
        {
            beg = RING_BUFFER_BEG_INDEX;
        }
    }
    else
    {
        beg = RING_BUFFER_BEG_INDEX;
    }

    if(beg != RING_BUFFER_BEG_INDEX )
    {
        SPI_ERR_T result;

        uint32_t SrcAddr;
        uint32_t len;
        uint32_t flash_offset;

        SrcAddr = (uint32_t)ptr;
        flash_offset = 0;
        len = RING_BUFFER_BEG_INDEX;
        result = spinor_write( SrcAddr, flash_addr+flash_offset, len);
        if(SPI_SUCCESS!=result)
        {
            printf("error spinor_write ret:%d\n", result);
            return;
        }

        SrcAddr = (uint32_t)ptr + beg;
        flash_offset = len;
        len = (RING_BUFFER_SIZE - beg);
        result = spinor_write( SrcAddr, flash_addr+flash_offset, len);
        if(SPI_SUCCESS!=result)
        {
            printf("error spinor_write ret:%d\n", result);
            return;
        }

        SrcAddr = ptr + RING_BUFFER_BEG_INDEX;
        flash_offset = RING_BUFFER_BEG_INDEX + len;
        len = ( 1 + end - RING_BUFFER_BEG_INDEX);
        result = spinor_write( SrcAddr, flash_addr+flash_offset, len);
        if(SPI_SUCCESS!=result)
        {
            printf("error spinor_write ret:%d\n", result);
            return;
        }
    }
    else
    {
        SPI_ERR_T result = spinor_write( (uint32_t)ptr, flash_addr, 4096);
        if(SPI_SUCCESS!=result)
        {
            printf("error spinor_write ret:%d\n", result);
        }
    }
}

void sys_log_2_flash(void)
{
    printf("*********** sys_log_2_flash beg ***********\n");

    g_sys_log_2_flash__on = 0;

    uint32_t seqMax = 0;
    uint32_t index = 0;
    uint32_t i;
    for(i=0;i<SYS_LOG_2_FLASH___RECORD_NUM;++i)
    {
        uint32_t seq = sys_log_2_flash__getseq(i);
        if(seq>seqMax)
        {
            seqMax = seq;
            index = i;
        }
    }

    if(seqMax!=0)
    {
        ++index;
        index %= SYS_LOG_2_FLASH___RECORD_NUM;
    }

    sys_log_2_flash__save(index, seqMax+1);

    g_sys_log_2_flash__on = 1;
}

#endif
void trap_General_Exception(void)
{
    SAVE_ALL();
    show_reg();
    serial_outc('E');
    serial_outc('7');
#ifdef SYS_LOG_2_FLASH
    sys_log_2_flash();
#endif
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
    unsigned int reg;
    priority &= 0x3;

    if (check_intr_num(intSrcNum)) return;

    if (intSrcNum < 16)
    {
        reg = __nds32__mfsr(NDS32_SR_INT_PRI);
        reg &= ~((0x3 << (intSrcNum << 1)));
        reg |= (priority << ((intSrcNum << 1)));
        __nds32__mtsr(reg, NDS32_SR_INT_PRI);
        __nds32__dsb();
    }
    else
    {
        reg = __nds32__mfsr(NDS32_SR_INT_PRI2);
        reg &= ~((0x3 << ((intSrcNum - 16) << 1)));
        reg |= (priority << (((intSrcNum - 16) << 1)));
        __nds32__mtsr(reg, NDS32_SR_INT_PRI2);
        __nds32__dsb();

    }

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
    unsigned long *long_p = &ISR_TABLE;

    if (check_intr_num(intSrcNum)) return;

    if (isISRregistered[intSrcNum]) {
        printf("\nError: interrupt %d is already registered\n", intSrcNum);
        return;
    }

    long_p[intSrcNum] = (unsigned long)isr;

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

