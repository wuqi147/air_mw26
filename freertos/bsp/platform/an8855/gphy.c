#include <platform.h>
#include <util.h>
#include <stdint.h>

#define WIDTH1_MSK      (0x1)
#define WIDTH2_MSK      (0x3)
#define WIDTH3_MSK      (0x7)
#define WIDTH4_MSK      (0xF)
#define WIDTH5_MSK      (0x1F)
#define WIDTH6_MSK      (0x3F)
#define WIDTH7_MSK      (0x7F)
#define WIDTH8_MSK      (0xFF)
#define WIDTH16_MSK     (0xFFFF)

#define PB_PHY_REG_BASE             (0xA0000000)
#define PB_PHY_ACCESS_BIT           (0x00800000)
#define PB_PHY_ADDR_OFS             (24)
#define PB_PHY_ADDR_MSK             (WIDTH5_MSK)
#define PB_PHY_ADDR_SEL(phyAddr)    ((phyAddr & PB_PHY_ADDR_MSK) << PB_PHY_ADDR_OFS)
#define PB_PHY_PAGE_OFS             (12)
#define PB_PHY_PAGE_MSK             (WIDTH4_MSK)
#define PB_PHY_PAGE_SEL(page)       ((page & PB_PHY_PAGE_MSK) << PB_PHY_PAGE_OFS)
#define PB_CL22_MAIN_PAGE           (0x00000000)
#define PB_PHY_CL22_REG_OFS         (4)
#define PB_PHY_CL22_REG_MSK         (WIDTH5_MSK)
#define PB_PHY_REG_SEL(reg)         ((reg & WIDTH5_MSK) << PB_PHY_CL22_REG_OFS)

/* CL45 */
#define PB_PHY_CL45_DEV_OFS         (18)
#define PB_PHY_CL45_DEV_MSK         (WIDTH5_MSK)
#define PB_PHY_CL45_DEV_SEL(dev)    ((dev & PB_PHY_CL45_DEV_MSK) << PB_PHY_CL45_DEV_OFS)
#define PB_PHY_CL45_REG_OFS         (2)
#define PB_PHY_CL45_REG_MSK         (WIDTH16_MSK)
#define PB_PHY_CL45_REG_SEL(reg)    ((reg & PB_PHY_CL45_REG_MSK) << PB_PHY_CL45_REG_OFS)


void cl22_write(const uint8_t phyAddr, const uint8_t pageSel, const uint8_t regSel, const uint16_t wdata)
{
    uint32_t pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT
                    | PB_PHY_ADDR_SEL(phyAddr) | PB_CL22_MAIN_PAGE
                    | PB_PHY_PAGE_SEL(pageSel) | PB_PHY_REG_SEL(regSel));

    io_write16(pbusAddr, wdata);
}

uint16_t cl22_read(const uint8_t phyAddr, const uint8_t pageSel, const uint8_t regSel)
{
    uint32_t pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT
                    | PB_PHY_ADDR_SEL(phyAddr) | PB_CL22_MAIN_PAGE
                    | PB_PHY_PAGE_SEL(pageSel) | PB_PHY_REG_SEL(regSel));

    return io_read16(pbusAddr);
}

void cl45_write(
    const uint8_t phyAddr,
    const uint8_t devID,
    const uint16_t regSel,
    const uint16_t wdata)
{
    uint32_t pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT
                    |    PB_PHY_ADDR_SEL(phyAddr) | PB_PHY_CL45_DEV_SEL(devID)
                    |    PB_PHY_CL45_REG_SEL(regSel));

    io_write16(pbusAddr, wdata);
}

uint16_t cl45_read(const uint8_t phyAddr, const uint8_t devID, const uint16_t regSel)
{
    uint32_t pbusAddr = (PB_PHY_REG_BASE | PB_PHY_ACCESS_BIT
                    | PB_PHY_ADDR_SEL(phyAddr) | PB_PHY_CL45_DEV_SEL(devID)
                    | PB_PHY_CL45_REG_SEL(regSel));

    return io_read16(pbusAddr);
}

