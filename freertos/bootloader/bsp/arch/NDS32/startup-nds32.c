
#include "nds32_intrinsic.h"
#include "nds32_defs.h"
#include "cache.h"
#include <platform.h>
#define VECTOR_BASE	0x20000000


#define PSW_MSK                                         \
        (PSW_mskGIE | PSW_mskINTL | PSW_mskPOM | PSW_mskAEN | PSW_mskIFCON | PSW_mskCPL)
#define PSW_INIT                                        \
        (0x0UL << PSW_offGIE                            \
         | 0x0UL << PSW_offINTL                         \
         | 0x1UL << PSW_offPOM                          \
         | 0x0UL << PSW_offAEN                          \
         | 0x0UL << PSW_offIFCON                        \
         | 0x7UL << PSW_offCPL)

#define IVB_MSK                                         \
        (IVB_mskEVIC | IVB_mskESZ | IVB_mskIVBASE)
#define IVB_INIT                                        \
        ((VECTOR_BASE >> IVB_offIVBASE) << IVB_offIVBASE\
         | 0x1UL << IVB_offESZ                          \
         | 0x0UL << IVB_offEVIC)

/* Interrupt priority
 * Default: lowest priority */
#define PRI1_DEFAULT            0xFFFFFFFF
#define PRI2_DEFAULT            0xFFFFFFFF

#define CACHE_NONE              0
#define CACHE_WRITEBACK         2
#define CACHE_WRITETHROUGH      3

#ifdef CFG_CACHE_ENABLE
/* Cacheable */
#ifdef CFG_CACHE_WRITETHROUGH
#define CACHE_MODE              CACHE_WRITETHROUGH
#else
#define CACHE_MODE              CACHE_WRITEBACK
#endif
#else
/* Uncacheable */
#define CACHE_MODE              CACHE_NONE
#endif

#define CACHE_CTL_MSK                                   \
        (CACHE_CTL_mskIC_EN                             \
         | CACHE_CTL_mskDC_EN                           \
         | CACHE_CTL_mskICALCK                          \
         | CACHE_CTL_mskDCALCK                          \
         | CACHE_CTL_mskDCCWF                           \
         | CACHE_CTL_mskDCPMW)

/* ICache/DCache enable */
#define CACHE_CTL_CACHE_ON                              \
        (0x1UL << CACHE_CTL_offIC_EN                    \
         | 0x1UL << CACHE_CTL_offDC_EN                  \
         | 0x0UL << CACHE_CTL_offICALCK                 \
         | 0x0UL << CACHE_CTL_offDCALCK                 \
         | 0x1UL << CACHE_CTL_offDCCWF                  \
         | 0x1UL << CACHE_CTL_offDCPMW)

#define MMU_CTL_MSK                                     \
        (MMU_CTL_mskD                                   \
         | MMU_CTL_mskNTC0                              \
         | MMU_CTL_mskNTC1                              \
         | MMU_CTL_mskNTC2                              \
         | MMU_CTL_mskNTC3                              \
         | MMU_CTL_mskTBALCK                            \
         | MMU_CTL_mskMPZIU                             \
         | MMU_CTL_mskNTM0                              \
         | MMU_CTL_mskNTM1                              \
         | MMU_CTL_mskNTM2                              \
         | MMU_CTL_mskNTM3)
/*
 * NTM0 (cacheable by NTC0) -> PA0:0x00000000~0x3FFFFFFF, for cached fetching instructions from Flash
 * NTM1 (non-cache by NTC1) -> PA0:0x00000000~0x3FFFFFFF, for uncached accessing registers
 * NTM2 (cache_wb by NTC2)  -> PA2:0x80000000~0xBFFFFFFF, for cached accessing DMEM
 * NTM3 (non-cache by NTC3) -> PA2:0x80000000~0xBFFFFFFF, for uncached accessing DMEM
 *   -- NTM0/1/2/3 starts at VA: 0x0/0x40000000/0x80000000/0xC0000000 respectively
 */
#define MMU_CTL_INIT                                    \
           (0x0UL << MMU_CTL_offD                       \
         | (CACHE_MODE) << MMU_CTL_offNTC0              \
         | 0x0UL << MMU_CTL_offNTC1                     \
         | (CACHE_MODE) << MMU_CTL_offNTC2              \
         | 0x0UL << MMU_CTL_offNTC3                     \
         | 0x0UL << MMU_CTL_offTBALCK                   \
         | 0x0UL << MMU_CTL_offMPZIU                    \
         | 0x0UL << MMU_CTL_offNTM0                     \
         | 0x0UL << MMU_CTL_offNTM1                     \
         | 0x2UL << MMU_CTL_offNTM2                     \
         | 0x2UL << MMU_CTL_offNTM3)

void c_startup(void);
void c_startup(void)
{
#define MEMCPY(des, src, n)     __builtin_memcpy ((des), (src), (n))
unsigned int size;
extern char __data_lmastart, __data_start, _edata;
/* Copy data section from LMA to VMA */
        size = &_edata - &__data_start;
        MEMCPY(&__data_start, &__data_lmastart, size);
}

static void cpu_init(void)
{
	unsigned int reg;

	/* Enable BTB & RTP since the default setting is disabled. */
	reg = __nds32__mfsr(NDS32_SR_MISC_CTL) & ~(MISC_CTL_makBTB | MISC_CTL_makRTP);
	__nds32__mtsr(reg, NDS32_SR_MISC_CTL);

	/* Set PSW GIE/INTL to 0, superuser & CPL to 7 */
	reg = (__nds32__mfsr(NDS32_SR_PSW) & ~PSW_MSK) | PSW_INIT;
	__nds32__mtsr(reg, NDS32_SR_PSW);

	/* Set PPL2FIX_EN to 0 to enable Programmable Priority Level */
	__nds32__mtsr(0x0, NDS32_SR_INT_CTRL);

	/* Set vector size: 16 byte, base: VECTOR_BASE, mode: IVIC */
	reg = (__nds32__mfsr(NDS32_SR_IVB) & ~IVB_MSK) | IVB_INIT;
	__nds32__mtsr(reg, NDS32_SR_IVB);

	/* Mask and clear hardware interrupts */
	if (reg & IVB_mskIVIC_VER) {
		/* IVB.IVIC_VER >= 1*/
		__nds32__mtsr(0x0, NDS32_SR_INT_MASK2);
		__nds32__mtsr(-1, NDS32_SR_INT_PEND2);
	} else {
		__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK) & ~0xFFFF, NDS32_SR_INT_MASK);
	}

	/* Set default Hardware interrupts priority with lowest priority */
	__nds32__mtsr(PRI1_DEFAULT, NDS32_SR_INT_PRI);
	__nds32__mtsr(PRI2_DEFAULT, NDS32_SR_INT_PRI2);

}

/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((naked));
void _nds32_init_mem(void)
{
}

/*
 * Initialize MMU configure and cache ability.
 */
static void mmu_init(void)
{
#ifndef __NDS32_ISA_V3M__
	unsigned int reg;

	/* MMU initialization: NTC0~NTC3, NTM0~NTM3 */
	reg = (__nds32__mfsr(NDS32_SR_MMU_CTL) & ~MMU_CTL_MSK) | MMU_CTL_INIT;
	__nds32__mtsr_dsb(reg, NDS32_SR_MMU_CTL);
#endif
}

void cache_init(void)
{

#ifdef CFG_CACHE_ENABLE
	unsigned int reg;

	/* Invalid ICache */
	nds32_icache_flush();

	/* Invalid DCache */
	nds32_dcache_invalidate();

	/* Enable I/Dcache */
	reg = (__nds32__mfsr(NDS32_SR_CACHE_CTL) & ~CACHE_CTL_MSK) | CACHE_CTL_CACHE_ON;
	__nds32__mtsr(reg, NDS32_SR_CACHE_CTL);
#endif
}


/*
 * NDS32 reset handler to reset all devices sequentially and call application
 * entry function.
 */
void cpu_bringup(void)
{
	extern int main(void);
	/*
	 * Initialize LMA/VMA sections.
	 * Relocation for any sections that need to be copied from LMA to VMA.
	 */
	c_startup();

	/*
	 * Initialize CPU to a post-reset state, ensuring the ground doesn't
	 * shift under us while we try to set things up.
	 */
	cpu_init();

	mmu_init();

	cache_init();

	/* Application entry function */
	main();

	/* Never go back here! */
	while(1);
}
