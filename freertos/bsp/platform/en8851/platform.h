#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/*****************************************************************************
 * useful defines
 ****************************************************************************/
#define VPchar  *(volatile unsigned char *)
#define VPshort *(volatile unsigned short *)
#define VPint   *(volatile unsigned int *)
#define S_256   (0x100)
#define S_1K    (0x400)
#define S_4K    (0x1000)
#define S_16K   (0x4000)
#define S_29K   (0x7400)
#define S_32K   (0x8000)
#define S_40K   (0xA000)
#define S_64K   (0x10000)
#define S_112K  (0x1C000)
#define S_124K  (0x1F000)
#define S_128K  (0x20000)
#define S_192K  (0x30000)
#define S_256K  (0x40000)
#define S_512K  (0x80000)
#define S_1M    (0x100000)
#define S_16M   (0x1000000)
#define S_1G    (0x40000000)
#define ENABLE  (1)
#define DISABLE (0)
#define ALL_FF  (0xFFFFFFFF)

#define HIGH_BIT_UNC        (0x1<<30)

//#define memcpy(d,s,n) air_memcpy(d,s,n)
#define air_dcache_inv(start,len) nds32_dcache_invalidate_range(((unsigned long)start),(((unsigned long)(start))+len))
#define air_dcache_wback_inv(start,len) nds32_dcache_flush_range(((unsigned long)start),(((unsigned long)(start))+len))
#define air_icache_inv(start,len) nds32_icache_invalidate_range(((unsigned long)start),(((unsigned long)(start))+len))

/*****************************************************************************
 * System clock
 ****************************************************************************/
#define KHz                     1000
#define MHz                     1000000

#define CPUFREQ                 (500 * MHz)
#define BUSCLKFREQ              (225 * MHz)
#define CPUTMRCLK               (100 * MHz)


#define APBCLKFREQ              (BUSCLKFREQ>>1)
#define TIMERCLK                (APBCLKFREQ)
#define DMEM_CTRL_CLK           (BUSCLKFREQ)

/*****************************************************************************
 * Peripheral Base Address
 ****************************************************************************/
#define CR_CHIP_SCU_BASE        (0x10000000)
#define CR_RBUS_BASE            (0x10001000)
#define CR_CPU_CTRL_INFRA_BASE  (0x10002000)
#define CR_NP_SCU_BASE          (0x10005000)
#define CR_UART_BASE            (0x1000A000)
#define CR_TIMER_BASE           (0x1000A100)
#define CR_CPUTMR_BASE          (0x1000A200)
#define CR_GPIO_BASE            (0x1000A300)


#define REGS_BASE           (0x10000000)
#define REGS_SIZE           (S_16M)

#define UNC_ADDR_OFFSET     (S_1G)

#define GDMPSRAM_BASE       (0x10010000)
#define GDMPSRAM_UNC_BASE   (GDMPSRAM_BASE+UNC_ADDR_OFFSET)
#define GDMPSRAM_SIZE       (S_32K)

#define DMEM_BASE           (0x80000000)
#define DMEM_UNC_BASE       (DMEM_BASE+UNC_ADDR_OFFSET)
#define DMEM_SIZE           (S_256K)

#define DMEM_PDMA_SIZE      (3 * S_4K)
#define DMEM_PMDA_BASE      (DMEM_BASE + DMEM_SIZE - DMEM_PDMA_SIZE)
/* Init stack size for booting. It is configured with 8K by default (at least 6K). */
#define DMEM_INITSTACK_SIZE (2 * S_4K)
#define DMEM_MIN_HEAP       (S_64K)

/*****************************************************************************
 * RBus Timeout Registers
 ****************************************************************************/
#define TIMEOUT_STS0        (CR_RBUS_BASE + 0xd0)
#define TIMEOUT_STS1        (CR_RBUS_BASE + 0xd4)
#define TIMEOUT_CFG0        (CR_RBUS_BASE + 0xd8)
#define TIMEOUT_CFG1        (CR_RBUS_BASE + 0xdc)
#define TIMEOUT_CFG2        (CR_RBUS_BASE + 0xe0)

/*****************************************************************************
 * RBus readBypassWrite Registers
 ****************************************************************************/
#define RDBYPASSWT_CFG      (CR_CPU_CTRL_INFRA_BASE + 0x4)
#define RDBYPASSWT_MASK     (CR_CPU_CTRL_INFRA_BASE + 0x8)
#define RDBYPASSWT_CNT      (CR_CPU_CTRL_INFRA_BASE + 0xC)

/*****************************************************************************
 * PBus Timeout Registers
 ****************************************************************************/
#define PB_TIMEOUT_CFG      (CR_NP_SCU_BASE+0x70)
#define PB_TIMEOUT_INFO     (CR_NP_SCU_BASE+0x74)
#define PB_TIMEOUT_INT      (CR_NP_SCU_BASE+0x78)

/*****************************************************************************
 * DRAM Illegal Access Registers
 ****************************************************************************/
#define DRAM_ILLACC_CHKEN   (CR_RBUS_BASE+0x18)
#define DRAM_ILLADDR_START1 (CR_RBUS_BASE+0x1C)
#define DRAM_ILLADDR_END1   (CR_RBUS_BASE+0x20)
#define DRAM_ILLADDR_START2 (CR_RBUS_BASE+0x24)
#define DRAM_ILLADDR_END2   (CR_RBUS_BASE+0x28)
#define DRAM_ILLACC_ADDR    (CR_RBUS_BASE+0x2C)
#define DRAM_ILLACC_INFO    (CR_RBUS_BASE+0x30)
#define DRAM_END_ADDR       (CR_RBUS_BASE+0x34)
#define DRAM_ILL_IID_FILT1  (CR_RBUS_BASE+0xC0)
#define DRAM_ILL_IID_FILT2  (CR_RBUS_BASE+0xC4)

/*****************************************************************************
 * Interrupt Source Number
 ****************************************************************************/
#define IRQ_GPIO            0
#define IRQ_TMR0            1
#define IRQ_TMR1            2
#define IRQ_TMR2            3
#define IRQ_TMR3            4
#define IRQ_UART            5
#define IRQ_CPU_TMR0        6
#define IRQ_CPU_TMR1        7
#define IRQ_BUS_TOUT        8
#define IRQ_RG_ABIST        9
#define IRQ_SFC             10
#define IRQ_I2C2RBUS        11
#define IRQ_GDMP            12
#define IRQ_GDMA            13
/* !! take GDMA irq for illegal irq to use in illegal_irq_0723 bitfile !! */
#define IRQ_ILL_ACCESS      IRQ_GDMA  /* illegal_irq_handler needs to be defined in ISR_TABLE in irq.S later */
#define IRQ_PDMA            14
#define IRQ_GSW             15
#define IRQ_SWI_VECTOR      16
#define IRQ_RESV17          17
#define IRQ_RESV18          18
#define IRQ_RESV19          19
#define IRQ_RESV20          20
#define IRQ_RESV21          21
#define IRQ_RESV22          22
#define IRQ_RESV23          23
#define IRQ_RESV24          24
#define IRQ_RESV25          25
#define IRQ_RESV26          26
#define IRQ_RESV27          27
#define IRQ_EXT_00          28
#define IRQ_EXT_01          29
#define IRQ_EXT_02          30
#define IRQ_RESV31          31
#define IRQ_MAX_NUM         32 /* should be the last one */
#define VECTOR_NUMINTRS     32 /* according to IVB.NIVIC */

/*****************************************************************************
 * Reset Control Registers
 ****************************************************************************/
#define CR_RST_CTRL2        0x100050c4
#define GSW_RST             0x400

/*****************************************************************************
 * GMAC Registers
 ****************************************************************************/
#define CR_GMAC_PORT_BASE   0x10210000
#define CR_GMAC_CPU_PORT    0x10213800
#define GMAC_PORT_OFFSET    0x200
#define FORCE_LINKUP        0x20001
#define FORCE_LINKDOWN      0x20000

/*****************************************************************************
 * GSW Top Registers
 ****************************************************************************/
#define CR_GSW_SYS_STATUS   (0x10218010)
#define CR_GSW_SYS_INT_EN   (0x10218014)
#define CR_GSW_SYS_INT_STS  (0x10218018)
#define GSW_INIT_DOWN       (0x7FFFE)
/* PHY SMI Register */
#define CR_PHY_SMI          (0x1021801C)

/*****************************************************************************
 * Internal delay for spi sample data
 ****************************************************************************/
#define SPI_SPEED           SPI_CLOCK_75M
#define SPI_CLOCK_75M       (0x401)
#define SPI_CLOCK_60M       (0x501)
#define SPI_CLOCK_50M       (0x601)

#define SPI_CLOCK_EDGE      SPI_CK_SEL_2
#define SPI_CK_SEL_0        (0x8)
#define SPI_CK_SEL_1        (0x9)
#define SPI_CK_SEL_2        (0xA)

#define SPI_DELAY_CELL      SPI_DCL_SEL_3
#define SPI_DCL_SEL_0       (0x0)
#define SPI_DCL_SEL_1       (0x1)
#define SPI_DCL_SEL_2       (0x2)
#define SPI_DCL_SEL_3       (0x3)

#define SPI_READ_CMD        SPI_DUAL_O
#define SPI_NORMAL_READ     (0x0)
#define SPI_FAST_READ       (0x1)
#define SPI_DUAL_O          (0x2)
#define SPI_DUAL_IO         (0x3)

/*****************************************************************************
 * freeRTOS
 ****************************************************************************/

/*****************************************************************************
 * Flash Layout
 ****************************************************************************/
#define MainSystem              (0x00)
#define BackupSystem            (0x01)
#define SystemConfig0           (0x02)
#define SystemConfig1           (0x03)
#define TempSystem              (0x04)

#if 1
#define DEFAULT_SYSTEMSIZE  (S_1M + S_1M + S_512K)
#else
#define DEFAULT_SYSTEMSIZE  (S_1M + S_512K)
#endif
#define SystemSize          (DEFAULT_SYSTEMSIZE)

#define BootLoaderBase      (0x20000000)
#define BootLoaderSize      (S_112K)
#define BootConfigBase      (BootLoaderBase + BootLoaderSize)
#define TempBootConfigBase  (BootConfigBase + S_4K)
#define BootConfigSize      (12*1024)
#define ManufactureBase     (BootConfigBase + BootConfigSize)
#define ManufactureSize     (S_4K)
#define MainSystemBase      (ManufactureBase + ManufactureSize)
#define MainSystemSize      (SystemSize)
#if DUAL_IMAGE
#define BackupSystemBase    (MainSystemBase + MainSystemSize)
#define BackupSystemSize    (SystemSize)
#define SystemConfigBase    (BackupSystemBase + BackupSystemSize)
#else
#define SystemConfigBase    (MainSystemBase + MainSystemSize)
#endif
#define SystemConfigSize    (S_64K)
#define TempSystemBase      (SystemConfigBase + SystemConfigSize)
#define TempSystemSize      (SystemSize)
#ifdef FW_SIGNATURE
#define FwSignatureBase     (TempSystemBase + TempSystemSize)
#define FwSignatureSize     (S_64K)
#endif

/*****************************************************************************
 * Watch Dog Setting
 ****************************************************************************/
#define Wdog_Timeout_Counter        (5)
#define Wdog_Timeout_Min_Counter    (5)
#define Wdog_Interrupt_Counter      (2)
#define Sw_Wdog_Timeout_Counter     (300)

#endif /* __PLATFORM_H__ */
