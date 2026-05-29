/* FILE NAME:  pdma.c
 * PURPOSE:
 *      It provides pdma driver function.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <platform.h>
#include <util.h>
#include <cache.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <nds32_intrinsic.h>
#include "task.h"
#include "queue.h"
#include "pdma.h"
#include "timer.h"
#include "interrupt.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* Register definition */
#define PDMA_CSR_BASE                           0x10009000
#define PDMA_CSR_REG(r)                         (PDMA_CSR_BASE + (r))

#define TX_BASE_PTR(n)                          PDMA_CSR_REG((n)*0x10 + 0x0)
#define TX_MAX_CNT(n)                           PDMA_CSR_REG((n)*0x10 + 0x4)
#define TX_CTX_IDX(n)                           PDMA_CSR_REG((n)*0x10 + 0x8)
#define TX_DTX_IDX(n)                           PDMA_CSR_REG((n)*0x10 + 0xc)

#define RX_BASE_PTR(n)                          PDMA_CSR_REG((n)*0x10 + 0x100)
#define RX_MAX_CNT(n)                           PDMA_CSR_REG((n)*0x10 + 0x104)
#define RX_CALC_IDX(n)                          PDMA_CSR_REG((n)*0x10 + 0x108)
#define RX_DRX_IDX(n)                           PDMA_CSR_REG((n)*0x10 + 0x10c)

#define PDMA_INFO                               PDMA_CSR_REG(0x200)
#define PDMA_INFO_TX_RING_NUM_OFFT              (0)
#define PDMA_INFO_TX_RING_NUM_LENG              (8)
#define PDMA_INFO_TX_RING_NUM_RELMASK           (0x000000ff)
#define PDMA_INFO_TX_RING_NUM_MASK              (PDMA_INFO_TX_RING_NUM_RELMASK << PDMA_INFO_TX_RING_NUM_OFFT)
#define PDMA_INFO_RX_RING_NUM_OFFT              (8)
#define PDMA_INFO_RX_RING_NUM_LENG              (8)
#define PDMA_INFO_RX_RING_NUM_RELMASK           (0x000000ff)
#define PDMA_INFO_RX_RING_NUM_MASK              (PDMA_INFO_RX_RING_NUM_RELMASK << PDMA_INFO_RX_RING_NUM_OFFT)
#define PDMA_INFO_BASE_PTR_WIDTH_OFFT           (16)
#define PDMA_INFO_BASE_PTR_WIDTH_LENG           (8)
#define PDMA_INFO_BASE_PTR_WIDTH_RELMASK        (0x000000ff)
#define PDMA_INFO_BASE_PTR_WIDTH_MASK           (PDMA_INFO_BASE_PTR_WIDTH_RELMASK << PDMA_INFO_BASE_PTR_WIDTH_OFFT)
#define PDMA_INFO_INDEX_WIDTH_OFFT              (24)
#define PDMA_INFO_INDEX_WIDTH_LENG              (4)
#define PDMA_INFO_INDEX_WIDTH_RELMASK           (0x0000000f)
#define PDMA_INFO_INDEX_WIDTH_MASK              (PDMA_INFO_INDEX_WIDTH_RELMASK << PDMA_INFO_INDEX_WIDTH_OFFT)
#define PDMA_INFO_VERSION_OFFT                  (28)
#define PDMA_INFO_VERSION_LENG                  (4)
#define PDMA_INFO_VERSION_RELMASK               (0x0000000f)
#define PDMA_INFO_VERSION_MASK                  (PDMA_INFO_VERSION_RELMASK << PDMA_INFO_VERSION_OFFT)

#define PDMA_GLO_CFG                            PDMA_CSR_REG(0x204)
#define PDMA_GLO_CFG_TX_DMA_EN_OFFT             (0)
#define PDMA_GLO_CFG_TX_DMA_EN_LENG             (1)
#define PDMA_GLO_CFG_TX_DMA_EN_RELMASK          (0x00000001)
#define PDMA_GLO_CFG_TX_DMA_EN_MASK             (PDMA_GLO_CFG_TX_DMA_EN_RELMASK << PDMA_GLO_CFG_TX_DMA_EN_OFFT)
#define PDMA_GLO_CFG_TX_DMA_BUSY_OFFT           (1)
#define PDMA_GLO_CFG_TX_DMA_BUSY_LENG           (1)
#define PDMA_GLO_CFG_TX_DMA_BUSY_RELMASK        (0x00000001)
#define PDMA_GLO_CFG_TX_DMA_BUSY_MASK           (PDMA_GLO_CFG_TX_DMA_BUSY_RELMASK << PDMA_GLO_CFG_TX_DMA_BUSY_OFFT)
#define PDMA_GLO_CFG_RX_DMA_EN_OFFT             (2)
#define PDMA_GLO_CFG_RX_DMA_EN_LENG             (1)
#define PDMA_GLO_CFG_RX_DMA_EN_RELMASK          (0x00000001)
#define PDMA_GLO_CFG_RX_DMA_EN_MASK             (PDMA_GLO_CFG_RX_DMA_EN_RELMASK << PDMA_GLO_CFG_RX_DMA_EN_OFFT)
#define PDMA_GLO_CFG_RX_DMA_BUSY_OFFT           (3)
#define PDMA_GLO_CFG_RX_DMA_BUSY_LENG           (1)
#define PDMA_GLO_CFG_RX_DMA_BUSY_RELMASK        (0x00000001)
#define PDMA_GLO_CFG_RX_DMA_BUSY_MASK           (PDMA_GLO_CFG_RX_DMA_BUSY_RELMASK << PDMA_GLO_CFG_RX_DMA_BUSY_OFFT)
#define PDMA_GLO_CFG_PDMA_BT_SIZE_OFFT          (4)
#define PDMA_GLO_CFG_PDMA_BT_SIZE_LENG          (2)
#define PDMA_GLO_CFG_PDMA_BT_SIZE_RELMASK       (0x00000003)
#define PDMA_GLO_CFG_PDMA_BT_SIZE_MASK          (PDMA_GLO_CFG_PDMA_BT_SIZE_RELMASK << PDMA_GLO_CFG_PDMA_BT_SIZE_OFFT)
#define PDMA_GLO_CFG_CSR_WB_DDONE_OFFT          (6)
#define PDMA_GLO_CFG_CSR_WB_DDONE_LENG          (1)
#define PDMA_GLO_CFG_CSR_WB_DDONE_RELMASK       (0x00000001)
#define PDMA_GLO_CFG_CSR_WB_DDONE_MASK          (PDMA_GLO_CFG_CSR_WB_DDONE_RELMASK << PDMA_GLO_CFG_CSR_WB_DDONE_OFFT)
#define PDMA_GLO_CFG_BIG_ENDIAN_OFFT            (7)
#define PDMA_GLO_CFG_BIG_ENDIAN_LENG            (1)
#define PDMA_GLO_CFG_BIG_ENDIAN_RELMASK         (0x00000001)
#define PDMA_GLO_CFG_BIG_ENDIAN_MASK            (PDMA_GLO_CFG_BIG_ENDIAN_RELMASK << PDMA_GLO_CFG_BIG_ENDIAN_OFFT)
#define PDMA_GLO_CFG_CSR_32_DESC_EN_OFFT        (8)
#define PDMA_GLO_CFG_CSR_32_DESC_EN_LENG        (1)
#define PDMA_GLO_CFG_CSR_32_DESC_EN_RELMASK     (0x00000001)
#define PDMA_GLO_CFG_CSR_32_DESC_EN_MASK        (PDMA_GLO_CFG_CSR_32_DESC_EN_RELMASK << PDMA_GLO_CFG_CSR_32_DESC_EN_OFFT)
#define PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_OFFT     (9)
#define PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_LENG     (1)
#define PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_RELMASK  (0x00000001)
#define PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_MASK     (PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_RELMASK << PDMA_GLO_CFG_CSR_SHARE_FIFO_EN_OFFT)
#define PDMA_GLO_CFG_CSR_MULTI_DMA_EN_OFFT      (10)
#define PDMA_GLO_CFG_CSR_MULTI_DMA_EN_LENG      (1)
#define PDMA_GLO_CFG_CSR_MULTI_DMA_EN_RELMASK   (0x00000001)
#define PDMA_GLO_CFG_CSR_MULTI_DMA_EN_MASK      (PDMA_GLO_CFG_CSR_MULTI_DMA_EN_RELMASK << PDMA_GLO_CFG_CSR_MULTI_DMA_EN_OFFT)
#define PDMA_GLO_CFG_CSR_BYTE_SWAP_OFFT         (29)
#define PDMA_GLO_CFG_CSR_BYTE_SWAP_LENG         (1)
#define PDMA_GLO_CFG_CSR_BYTE_SWAP_RELMASK      (0x00000001)
#define PDMA_GLO_CFG_CSR_BYTE_SWAP_MASK         (PDMA_GLO_CFG_CSR_BYTE_SWAP_RELMASK << PDMA_GLO_CFG_CSR_BYTE_SWAP_OFFT)
#define PDMA_GLO_CFG_CSR_CLKGATE_BYP_OFFT       (30)
#define PDMA_GLO_CFG_CSR_CLKGATE_BYP_LENG       (1)
#define PDMA_GLO_CFG_CSR_CLKGATE_BYP_RELMASK    (0x00000001)
#define PDMA_GLO_CFG_CSR_CLKGATE_BYP_MASK       (PDMA_GLO_CFG_CSR_CLKGATE_BYP_RELMASK << PDMA_GLO_CFG_CSR_CLKGATE_BYP_OFFT)
#define PDMA_GLO_CFG_RX_2B_OFFSET_OFFT          (31)
#define PDMA_GLO_CFG_RX_2B_OFFSET_LENG          (1)
#define PDMA_GLO_CFG_RX_2B_OFFSET_RELMASK       (0x00000001)
#define PDMA_GLO_CFG_RX_2B_OFFSET_MASK          (PDMA_GLO_CFG_RX_2B_OFFSET_RELMASK << PDMA_GLO_CFG_RX_2B_OFFSET_OFFT)

#define PDMA_RST_IDX                            PDMA_CSR_REG(0x208)
#define PDMA_RST_IDX_RST_DTX_IDX0_OFFT          (0)
#define PDMA_RST_IDX_RST_DTX_IDX0_LENG          (1)
#define PDMA_RST_IDX_RST_DTX_IDX0_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DTX_IDX0_MASK          (PDMA_RST_IDX_RST_DTX_IDX0_RELMASK << PDMA_RST_IDX_RST_DTX_IDX0_OFFT)
#define PDMA_RST_IDX_RST_DTX_IDX1_OFFT          (1)
#define PDMA_RST_IDX_RST_DTX_IDX1_LENG          (1)
#define PDMA_RST_IDX_RST_DTX_IDX1_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DTX_IDX1_MASK          (PDMA_RST_IDX_RST_DTX_IDX1_RELMASK << PDMA_RST_IDX_RST_DTX_IDX1_OFFT)
#define PDMA_RST_IDX_RST_DRX_IDX0_OFFT          (16)
#define PDMA_RST_IDX_RST_DRX_IDX0_LENG          (1)
#define PDMA_RST_IDX_RST_DRX_IDX0_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DRX_IDX0_MASK          (PDMA_RST_IDX_RST_DRX_IDX0_RELMASK << PDMA_RST_IDX_RST_DRX_IDX0_OFFT)
#define PDMA_RST_IDX_RST_DRX_IDX1_OFFT          (17)
#define PDMA_RST_IDX_RST_DRX_IDX1_LENG          (1)
#define PDMA_RST_IDX_RST_DRX_IDX1_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DRX_IDX1_MASK          (PDMA_RST_IDX_RST_DRX_IDX1_RELMASK << PDMA_RST_IDX_RST_DRX_IDX1_OFFT)
#define PDMA_RST_IDX_RST_DRX_IDX2_OFFT          (18)
#define PDMA_RST_IDX_RST_DRX_IDX2_LENG          (1)
#define PDMA_RST_IDX_RST_DRX_IDX2_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DRX_IDX2_MASK          (PDMA_RST_IDX_RST_DRX_IDX2_RELMASK << PDMA_RST_IDX_RST_DRX_IDX2_OFFT)
#define PDMA_RST_IDX_RST_DRX_IDX3_OFFT          (19)
#define PDMA_RST_IDX_RST_DRX_IDX3_LENG          (1)
#define PDMA_RST_IDX_RST_DRX_IDX3_RELMASK       (0x00000001)
#define PDMA_RST_IDX_RST_DRX_IDX3_MASK          (PDMA_RST_IDX_RST_DRX_IDX3_RELMASK << PDMA_RST_IDX_RST_DRX_IDX3_OFFT)

#define DELAY_INT_CFG                           PDMA_CSR_REG(0x20c)
#define DELAY_INT_CFG_RXMAX_PTIME_OFFT          (0)
#define DELAY_INT_CFG_RXMAX_PTIME_LENG          (8)
#define DELAY_INT_CFG_RXMAX_PTIME_RELMASK       (0x000000ff)
#define DELAY_INT_CFG_RXMAX_PTIME_MASK          (DELAY_INT_CFG_RXMAX_PTIME_RELMASK << DELAY_INT_CFG_RXMAX_PTIME_OFFT)
#define DELAY_INT_CFG_RXMAX_PINT_OFFT           (8)
#define DELAY_INT_CFG_RXMAX_PINT_LENG           (7)
#define DELAY_INT_CFG_RXMAX_PINT_RELMASK        (0x0000007f)
#define DELAY_INT_CFG_RXMAX_PINT_MASK           (DELAY_INT_CFG_RXMAX_PINT_RELMASK << DELAY_INT_CFG_RXMAX_PINT_OFFT)
#define DELAY_INT_CFG_RXDLY_INT_EN_OFFT         (15)
#define DELAY_INT_CFG_RXDLY_INT_EN_LENG         (1)
#define DELAY_INT_CFG_RXDLY_INT_EN_RELMASK      (0x00000001)
#define DELAY_INT_CFG_RXDLY_INT_EN_MASK         (DELAY_INT_CFG_RXDLY_INT_EN_RELMASK << DELAY_INT_CFG_RXDLY_INT_EN_OFFT)
#define DELAY_INT_CFG_TXMAX_PTIME_OFFT          (16)
#define DELAY_INT_CFG_TXMAX_PTIME_LENG          (8)
#define DELAY_INT_CFG_TXMAX_PTIME_RELMASK       (0x000000ff)
#define DELAY_INT_CFG_TXMAX_PTIME_MASK          (DELAY_INT_CFG_TXMAX_PTIME_RELMASK << DELAY_INT_CFG_TXMAX_PTIME_OFFT)
#define DELAY_INT_CFG_TXMAX_PINT_OFFT           (24)
#define DELAY_INT_CFG_TXMAX_PINT_LENG           (7)
#define DELAY_INT_CFG_TXMAX_PINT_RELMASK        (0x0000007f)
#define DELAY_INT_CFG_TXMAX_PINT_MASK           (DELAY_INT_CFG_TXMAX_PINT_RELMASK << DELAY_INT_CFG_TXMAX_PINT_OFFT)
#define DELAY_INT_CFG_TXDLY_INT_EN_OFFT         (31)
#define DELAY_INT_CFG_TXDLY_INT_EN_LENG         (1)
#define DELAY_INT_CFG_TXDLY_INT_EN_RELMASK      (0x00000001)
#define DELAY_INT_CFG_TXDLY_INT_EN_MASK         (DELAY_INT_CFG_TXDLY_INT_EN_RELMASK << DELAY_INT_CFG_TXDLY_INT_EN_OFFT)

#define FREEQ_THRES                             PDMA_CSR_REG(0x210)
#define FREEQ_THRES_FREEQ_THRES_OFFT            (0)
#define FREEQ_THRES_FREEQ_THRES_LENG            (4)
#define FREEQ_THRES_FREEQ_THRES_RELMASK         (0x0000000f)
#define FREEQ_THRES_FREEQ_THRES_MASK            (FREEQ_THRES_FREEQ_THRES_RELMASK << FREEQ_THRES_FREEQ_THRES_OFFT)

#define INT_STATUS                              PDMA_CSR_REG(0x220)
#define INT_MASK                                PDMA_CSR_REG(0x228)
#define RX_COHERENT                             (1 << 31)
#define RX_DLY_INT                              (1 << 30)
#define TX_COHERENT                             (1 << 29)
#define TX_DLY_INT                              (1 << 28)
#define RX_DONE_INT3                            (1 << 19)
#define RX_DONE_INT2                            (1 << 18)
#define RX_DONE_INT1                            (1 << 17)
#define RX_DONE_INT0                            (1 << 16)
#define TX_DONE_INT1                            (1 << 1)
#define TX_DONE_INT0                            (1 << 0)

#define TICKER_RST                              PDMA_CSR_REG(0x240)
#define TICKER_RST_CSR_SW_RST_OFFT              (0)
#define TICKER_RST_CSR_SW_RST_LENG              (1)
#define TICKER_RST_CSR_SW_RST_RELMASK           (0x00000001)
#define TICKER_RST_CSR_SW_RST_MASK              (TICKER_RST_CSR_SW_RST_RELMASK << TICKER_RST_CSR_SW_RST_OFFT)

#define TICKER_CFG                              PDMA_CSR_REG(0x244)
#define TICKER_CFG_CSR_TICKER1US_CNT_OFFT       (0)
#define TICKER_CFG_CSR_TICKER1US_CNT_LENG       (8)
#define TICKER_CFG_CSR_TICKER1US_CNT_RELMASK    (0x000000ff)
#define TICKER_CFG_CSR_TICKER1US_CNT_MASK       (TICKER_CFG_CSR_TICKER1US_CNT_RELMASK << TICKER_CFG_CSR_TICKER1US_CNT_OFFT)
#define TICKER_CFG_CSR_XTAL_FREQ_SEL_OFFT       (8)
#define TICKER_CFG_CSR_XTAL_FREQ_SEL_LENG       (2)
#define TICKER_CFG_CSR_XTAL_FREQ_SEL_RELMASK    (0x00000003)
#define TICKER_CFG_CSR_XTAL_FREQ_SEL_MASK       (TICKER_CFG_CSR_XTAL_FREQ_SEL_RELMASK << TICKER_CFG_CSR_XTAL_FREQ_SEL_OFFT)
#define TICKER_CFG_CSR_FORCE_TICKER_CNT_OFFT    (31)
#define TICKER_CFG_CSR_FORCE_TICKER_CNT_LENG    (1)
#define TICKER_CFG_CSR_FORCE_TICKER_CNT_RELMASK (0x00000001)
#define TICKER_CFG_CSR_FORCE_TICKER_CNT_MASK    (TICKER_CFG_CSR_FORCE_TICKER_CNT_RELMASK << TICKER_CFG_CSR_FORCE_TICKER_CNT_OFFT)

#define TICKER2_CFG                             PDMA_CSR_REG(0x248)
#define TICKER2_CFG_CSR_TICKER_125_CNT_OFFT     (0)
#define TICKER2_CFG_CSR_TICKER_125_CNT_LENG     (8)
#define TICKER2_CFG_CSR_TICKER_125_CNT_RELMASK  (0x000000ff)
#define TICKER2_CFG_CSR_TICKER_125_CNT_MASK     (TICKER2_CFG_CSR_TICKER_125_CNT_RELMASK << TICKER2_CFG_CSR_TICKER_125_CNT_OFFT)
#define TICKER2_CFG_CSR_TICKER_20_CNT_OFFT      (8)
#define TICKER2_CFG_CSR_TICKER_20_CNT_LENG      (8)
#define TICKER2_CFG_CSR_TICKER_20_CNT_RELMASK   (0x000000ff)
#define TICKER2_CFG_CSR_TICKER_20_CNT_MASK      (TICKER2_CFG_CSR_TICKER_20_CNT_RELMASK << TICKER2_CFG_CSR_TICKER_20_CNT_OFFT)
#define TICKER2_CFG_CSR_COMP_125_VAL_OFFT       (16)
#define TICKER2_CFG_CSR_COMP_125_VAL_LENG       (8)
#define TICKER2_CFG_CSR_COMP_125_VAL_RELMASK    (0x000000ff)
#define TICKER2_CFG_CSR_COMP_125_VAL_MASK       (TICKER2_CFG_CSR_COMP_125_VAL_RELMASK << TICKER2_CFG_CSR_COMP_125_VAL_OFFT)
#define TICKER2_CFG_CSR_COMP_20_VAL_OFFT        (24)
#define TICKER2_CFG_CSR_COMP_20_VAL_LENG        (8)
#define TICKER2_CFG_CSR_COMP_20_VAL_RELMASK     (0x000000ff)
#define TICKER2_CFG_CSR_COMP_20_VAL_MASK        (TICKER2_CFG_CSR_COMP_20_VAL_RELMASK << TICKER2_CFG_CSR_COMP_20_VAL_OFFT)

#define PDMA_FC_CFG                             PDMA_CSR_REG(0x250)
#define PDMA_FC_CFG_CSR_PRT_XFC_OFFT            (0)
#define PDMA_FC_CFG_CSR_PRT_XFC_LENG            (1)
#define PDMA_FC_CFG_CSR_PRT_XFC_RELMASK         (0x00000001)
#define PDMA_FC_CFG_CSR_PRT_XFC_MASK            (PDMA_FC_CFG_CSR_PRT_XFC_RELMASK << PDMA_FC_CFG_CSR_PRT_XFC_OFFT)
#define PDMA_FC_CFG_CSR_QUE_XFC_OFFT            (1)
#define PDMA_FC_CFG_CSR_QUE_XFC_LENG            (1)
#define PDMA_FC_CFG_CSR_QUE_XFC_RELMASK         (0x00000001)
#define PDMA_FC_CFG_CSR_QUE_XFC_MASK            (PDMA_FC_CFG_CSR_QUE_XFC_RELMASK << PDMA_FC_CFG_CSR_QUE_XFC_OFFT)
#define PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_OFFT    (2)
#define PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_LENG    (1)
#define PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_RELMASK (0x00000001)
#define PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_MASK    (PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_RELMASK << PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_OFFT)
#define PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_OFFT    (3)
#define PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_LENG    (1)
#define PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_RELMASK (0x00000001)
#define PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_MASK    (PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_RELMASK << PDMA_FC_CFG_CSR_GSW2PDMA_CRC_EN_OFFT)
#define PDMA_FC_CFG_CSR_RXQ_THRESHOLD_OFFT      (16)
#define PDMA_FC_CFG_CSR_RXQ_THRESHOLD_LENG      (12)
#define PDMA_FC_CFG_CSR_RXQ_THRESHOLD_RELMASK   (0x00000fff)
#define PDMA_FC_CFG_CSR_RXQ_THRESHOLDN_MASK     (PDMA_FC_CFG_CSR_RXQ_THRESHOLD_RELMASK << PDMA_FC_CFG_CSR_RXQ_THRESHOLD_OFFT)

#define PDMA_CPORT_CHN_MAP                          PDMA_CSR_REG(0x254)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_OFFT     (0)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_LENG     (5)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_RELMASK  (0x0000001f)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_MASK     (PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_RELMASK << PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_0_OFFT)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_OFFT     (8)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_LENG     (5)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_RELMASK  (0x0000001f)
#define PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_MASK     (PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_RELMASK << PDMA_CPORT_CHN_MAP_CSR_CPORT_CHN_1_OFFT)

#define PDMA_SQ_MAP_PQ                          PDMA_CSR_REG(0x258)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_OFFT     (0)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_0_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_OFFT     (4)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_1_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_OFFT     (8)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_2_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_OFFT     (12)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_3_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_OFFT     (16)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_4_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_OFFT     (20)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_5_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_OFFT     (24)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_6_OFFT)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_OFFT     (28)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_LENG     (2)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_RELMASK  (0x00000003)
#define PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_MASK     (PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_RELMASK << PDMA_SQ_MAP_PQ_CSR_SQ_MAP_PQ_7_OFFT)

#define PDMA_SCH_Q01_CFG                            PDMA_CSR_REG(0x280)
#define PDMA_SCH_Q01_CFG_MAX_RATE0_OFFT             (0)
#define PDMA_SCH_Q01_CFG_MAX_RATE0_LENG             (10)
#define PDMA_SCH_Q01_CFG_MAX_RATE0_RELMASK          (0x000003ff)
#define PDMA_SCH_Q01_CFG_MAX_RATE0_MASK             (PDMA_SCH_Q01_CFG_MAX_RATE0_RELMASK << PDMA_SCH_Q01_CFG_MAX_RATE0_OFFT)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_OFFT       (10)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_LENG       (2)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_RELMASK    (0x00000003)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_MASK       (PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_RELMASK << PDMA_SCH_Q01_CFG_MIN_RATE_RATIO0_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT0_OFFT           (12)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT0_LENG           (2)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT0_RELMASK        (0x00000003)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT0_MASK           (PDMA_SCH_Q01_CFG_MAX_WEIGHT0_RELMASK << PDMA_SCH_Q01_CFG_MAX_WEIGHT0_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_OFFT        (14)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_LENG        (1)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_RELMASK     (0x00000001)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_MASK        (PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_RELMASK << PDMA_SCH_Q01_CFG_MAX_RATE_ULMT0_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_OFFT         (15)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_LENG         (1)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_RELMASK      (0x00000001)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_MASK         (PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_RELMASK << PDMA_SCH_Q01_CFG_MAX_BKT_SIZE0_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_RATE1_OFFT             (16)
#define PDMA_SCH_Q01_CFG_MAX_RATE1_LENG             (10)
#define PDMA_SCH_Q01_CFG_MAX_RATE1_RELMASK          (0x000003ff)
#define PDMA_SCH_Q01_CFG_MAX_RATE1_MASK             (PDMA_SCH_Q01_CFG_MAX_RATE1_RELMASK << PDMA_SCH_Q01_CFG_MAX_RATE1_OFFT)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_OFFT       (26)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_LENG       (2)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_RELMASK    (0x00000003)
#define PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_MASK       (PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_RELMASK << PDMA_SCH_Q01_CFG_MIN_RATE_RATIO1_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT1_OFFT           (28)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT1_LENG           (2)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT1_RELMASK        (0x00000003)
#define PDMA_SCH_Q01_CFG_MAX_WEIGHT1_MASK           (PDMA_SCH_Q01_CFG_MAX_WEIGHT1_RELMASK << PDMA_SCH_Q01_CFG_MAX_WEIGHT1_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_OFFT        (30)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_LENG        (1)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_RELMASK     (0x00000001)
#define PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_MASK        (PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_RELMASK << PDMA_SCH_Q01_CFG_MAX_RATE_ULMT1_OFFT)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_OFFT         (31)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_LENG         (1)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_RELMASK      (0x00000001)
#define PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_MASK         (PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_RELMASK << PDMA_SCH_Q01_CFG_MAX_BKT_SIZE1_OFFT)

/* Register value definition */
/* PDMA_GLO_CFG */
#define RX_2BYTE_OFFSET                 (1<<31)
#define CSR_CLKGATE                     (1<<30)
#define PDMA_BYTE_SWAP                  (1<<29)
#define PDMA_BIG_ENDIAN                 (1<<7)
#define TX_WB_DDONE                     (1<<6)
#define PDMA_BT_SIZE_SHIFT              (4)
#define PDMA_BT_SIZE                    (0x3<<PDMA_BT_SIZE_SHIFT)
#define RX_DMA_BUSY                     (1<<3)
#define RX_DMA_EN                       (1<<2)
#define TX_DMA_BUSY                     (1<<1)
#define TX_DMA_EN                       (1<<0)

#define PDMA_BT_SIZE_4DW                (0)
#define PDMA_BT_SIZE_8DW                (1)
#define PDMA_BT_SIZE_16DW               (2)
#define PDMA_BT_SIZE_32DW               (3)

/* PDMA_RST_IDX */
#define RST_DRX_IDX(n)                  (1<<(16+(n)))
#define RST_DTX_IDX(n)                  (1<<(n))

/* macro definition */
#define PDMA_4BYTES_ALIGN(size) (((size) + 4 - 1U) & ~(4 - 1U))
extern unsigned char __pdmabuffer_vmastart, __pdmabuffer_vmaend;
#define PDMA_BUFFER_START_ADDR          ((unsigned int)&__pdmabuffer_vmastart)

#define PDMA_RX_QUEUE_NUM               (1)
#define PDMA_RXDESC_UNIT                (5)
#define PDMA_RXDESC_NO                  (PDMA_RXDESC_UNIT*5)
#define PDMA_TX_QUEUE_NUM               (1)
#define PDMA_TXDESC_NO                  (5)
/* 1530: ETH_HWADDR_LEN * 2 + SIZEOF_STAG_HDR + SIZEOF_VLAN_HDR * PKT_MAX_VLAN_TAG + SIZEOF_ETH_TYPE + ethernet_MTU(1500). */
#define PDMA_ETHERNET_RX_PKT_LEN        (1530)
/* 1518: ETH_HWADDR_LEN * 2 + SIZEOF_STAG_HDR + SIZEOF_ETH_TYPE + ethernet_MTU(1500) */
#define PDMA_ETHERNET_TX_PKT_LEN        (1518)
/* PDMA buffers need be aligned to 32 bits boundaries. */
#define PDMA_RX_PKT_LEN                 PDMA_4BYTES_ALIGN(760)
#define PDMA_TX_PKT_LEN                 PDMA_4BYTES_ALIGN(PDMA_ETHERNET_TX_PKT_LEN)

#define PDMA_ISR_RX_PKT_THRESHOLD       (16)
#define PDMA_TX_BUF_RELEASE_THRESHOLD   (3)

#define ETHERNET_RX_PKT_BUFFER_ADDR     (0x10010000)
#define ETHERNET_RX_PKT_BUFFER_NUM      (20)

#define DESC_SIZE                       (16)
#define PDMA_BUFEFR_SIZE                (PDMA_RX_QUEUE_NUM*PDMA_RXDESC_NO*DESC_SIZE+PDMA_TX_QUEUE_NUM*PDMA_TXDESC_NO*DESC_SIZE+\
                                        PDMA_RX_PKT_LEN*PDMA_RXDESC_UNIT+PDMA_TX_PKT_LEN*PDMA_TXDESC_NO)
static unsigned char pdma_buffer[PDMA_BUFEFR_SIZE] __attribute__((section(".dmem_pdmabuffer")));

/* MACRO FUNCTION DECLARATIONS
 */
#define PDMA_DEBUG_DETAIL(fmt,...) do                                  \
{                                                                      \
    if (g_debug_detail)                                                \
    {                                                                  \
        printf(fmt, ##__VA_ARGS__);                                    \
    }                                                                  \
}while (0)

/* DATA TYPE DECLARATIONS
 */
/*=========================================
      PDMA RX Descriptor Format define
=========================================*/
typedef union
{
    struct _PDMA_RXD_INFO1_
    {
        unsigned int    PDP0;
    } bits;
    unsigned int word;
} PDMA_RXD_INFO1_T;

typedef union
{
    struct _PDMA_RXD_INFO2_
    {
#ifdef __BIG_ENDIAN
        unsigned int    DDONE_bit             : 1;
        unsigned int    LS0                   : 1;
        unsigned int    PLEN0                 : 14;
        unsigned int    UN_USED               : 1;
        unsigned int    LS1                   : 1;
        unsigned int    PLEN1                 : 14;
#else
        unsigned int    PLEN1                 : 14;
        unsigned int    LS1                   : 1;
        unsigned int    UN_USED               : 1;
        unsigned int    PLEN0                 : 14;
        unsigned int    LS0                   : 1;
        unsigned int    DDONE_bit             : 1;
#endif
    } bits;
    unsigned int word;
} PDMA_RXD_INFO2_T;

typedef union
{
    struct _PDMA_RXD_INFO3_
    {
        unsigned int    UN_USE1;
    } bits;
    unsigned int word;
} PDMA_RXD_INFO3_T;

typedef union
{
    struct _PDMA_RXD_INFO4_
    {
#ifdef __BIG_ENDIAN
        unsigned int    RSV                 : 3;
        unsigned int    PKT_INFO            : 6;
        unsigned int    SPORT               : 4;
        unsigned int    CRSN                : 5;
        unsigned int    FOE_Entry           : 14;
#else
        unsigned int    FOE_Entry           : 14;
        unsigned int    CRSN                : 5;
        unsigned int    SPORT               : 4;
        unsigned int    PKT_INFO            : 6;
        unsigned int    RSV                 : 3;
#endif
    } bits;
    unsigned int word;
} PDMA_RXD_INFO4_T;

typedef struct
{
    PDMA_RXD_INFO1_T rxd_info1;
    PDMA_RXD_INFO2_T rxd_info2;
    PDMA_RXD_INFO3_T rxd_info3;
    PDMA_RXD_INFO4_T rxd_info4;
} RX_DESC_T;

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
typedef union
{
    struct _PDMA_TXD_INFO1_
    {
        unsigned int    SDP0;
    } bits;
    unsigned int word;
} PDMA_TXD_INFO1_T;

typedef union
{
    struct _PDMA_TXD_INFO2_
    {
#ifdef __BIG_ENDIAN
        unsigned int    DDONE_bit             : 1;
        unsigned int    LS0_bit               : 1;
        unsigned int    SDL0                  : 14;
        unsigned int    BURST_bit             : 1;
        unsigned int    LS1_bit               : 1;
        unsigned int    SDL1                  : 14;
#else
        unsigned int    SDL1                  : 14;
        unsigned int    LS1_bit               : 1;
        unsigned int    BURST_bit             : 1;
        unsigned int    SDL0                  : 14;
        unsigned int    LS0_bit               : 1;
        unsigned int    DDONE_bit             : 1;
#endif
    } bits;
    unsigned int word;
} PDMA_TXD_INFO2_T;

typedef union
{
    struct _PDMA_TXD_INFO3_
    {
        unsigned int    SDP1;
    } bits;
    unsigned int word;
} PDMA_TXD_INFO3_T;

typedef union
{
    struct _PDMA_TXD_INFO4_
    {
#ifdef __BIG_ENDIAN
        unsigned int    ICO                 : 1;
        unsigned int    UCO                 : 1;
        unsigned int    TCO                 : 1;
        unsigned int    TSO                 : 1;
        unsigned int    UDF                 : 6;
        unsigned int    PN                  : 3;//the same with FPORT
        unsigned int    INSV                : 1;
        unsigned int    TPID                : 2;
        unsigned int    VPRI                : 3;
        unsigned int    CFI                 : 1;
        unsigned int    VIDX                : 12;
#else
        unsigned int    VIDX                : 12;
        unsigned int    CFI                 : 1;
        unsigned int    VPRI                : 3;
        unsigned int    TPID                : 2;
        unsigned int    INSV                : 1;
        unsigned int    PN                  : 3;//the same with FPORT
        unsigned int    UDF                 : 6;
        unsigned int    TSO                 : 1;
        unsigned int    TCO                 : 1;
        unsigned int    UCO                 : 1;
        unsigned int    ICO                 : 1;
#endif
    } bits;
    unsigned int word;
} PDMA_TXD_INFO4_T;

typedef struct
{
    PDMA_TXD_INFO1_T txd_info1;
    PDMA_TXD_INFO2_T txd_info2;
    PDMA_TXD_INFO3_T txd_info3;
    PDMA_TXD_INFO4_T txd_info4;
} TX_DESC_T;

typedef struct
{
    RX_DESC_T rx_desc_buf[PDMA_RX_QUEUE_NUM][PDMA_RXDESC_NO];
} RX_MEM_POOL_T;

typedef struct
{
    TX_DESC_T tx_desc_buf[PDMA_TX_QUEUE_NUM][PDMA_TXDESC_NO];
} TX_MEM_POOL_T;

typedef struct
{
    unsigned int use;
    unsigned int data;
    unsigned int len;
} ETHERNET_PKT_MSG_T;

typedef struct
{
    SemaphoreHandle_t  rx_pkt_mutex[ETHERNET_RX_PKT_BUFFER_NUM];
    ETHERNET_PKT_MSG_T rx_pkt_msg[ETHERNET_RX_PKT_BUFFER_NUM];
} ETHERNET_PKT_MSG_POOL_T;

typedef struct
{
    RX_MEM_POOL_T      *rx_mem_pool;
    TX_MEM_POOL_T      *tx_mem_pool;

    void* rx_ring_base_addr[PDMA_RX_QUEUE_NUM];             /* Receive Descr Ring Virtual Address */
    void* tx_ring_base_addr[PDMA_TX_QUEUE_NUM];             /* Transmit Descr Ring Virtual Address */
    unsigned int rx_cur_desc[PDMA_RX_QUEUE_NUM];            /* index to current rx descriptor */
    unsigned int tx_cur_desc[PDMA_TX_QUEUE_NUM];            /* index to current tx descriptor */
    unsigned int tx_unreleased_desc[PDMA_TX_QUEUE_NUM];     /* index to the unreleased desc of Tx */
    unsigned int tx_unreleased_buf_cnt[PDMA_TX_QUEUE_NUM];  /* Unreleased buffer cnt hold by Tx */

    void *rx_pkt_data[PDMA_RX_QUEUE_NUM][PDMA_RXDESC_UNIT];
    void *tx_pkt_data[PDMA_TX_QUEUE_NUM][PDMA_TXDESC_NO];

    unsigned int rx_cur_pkt_msg;
    ETHERNET_PKT_MSG_POOL_T *rx_pkt_msg_pool;
} PDMA_CFG_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
static PDMA_CFG_T g_pdma_cfg = {0};

/* debug vars */
static unsigned int g_pdma_tx_full_cnt = 0;
static unsigned int g_pdma_tx_done_bit_err = 0;
static unsigned char g_debug_detail = 0;
static unsigned char g_debug_record = 0;
static unsigned int g_debug_record_cnt = 0;
static unsigned int g_debug_record_drop_cnt = 0;
static unsigned int g_debug_record_get_cnt = 0;
static unsigned int g_debug_record_release_cnt = 0;

static unsigned int g_probe_cnt = 0;
static unsigned int g_probe_isr_cnt[12] = {0};
static unsigned int g_probe_cur_idx[12] = {0};
static unsigned int g_probe_cpu_idx[12] = {0};
static unsigned int g_probe_dma_idx[12] = {0};
static unsigned int g_probe_pkt_cnt[12] = {0};
static unsigned int g_probe_pkt_cnt_change_in_intr[12][12] = {{0}};

/* pmda test tx control */
static unsigned int g_test_tx_cnt = 1;
static unsigned int g_test_tx_len = 64;
static unsigned int g_test_tx_delay = 0;

extern QueueHandle_t g_pkt_queue;

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
void* pdma_get_ethernet_pkt_buf(unsigned int index, unsigned int* ptr_len)
{
    ETHERNET_PKT_MSG_T* p_rx_pkt_msg = NULL;

    if (index >= ETHERNET_RX_PKT_BUFFER_NUM)
    {
        return NULL;
    }

    p_rx_pkt_msg = &g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[index];

    if(g_debug_record)
    {
        g_debug_record_get_cnt++;
    }

    *ptr_len = p_rx_pkt_msg->len;
    return (void*)p_rx_pkt_msg->data;
}

void pdma_release_ethernet_pkt_buf(unsigned int index)
{
    SemaphoreHandle_t  p_rx_pkt_mutex = NULL;
    ETHERNET_PKT_MSG_T* p_rx_pkt_msg = NULL;

    if (index >= ETHERNET_RX_PKT_BUFFER_NUM)
    {
        return;
    }

    p_rx_pkt_mutex = g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_mutex[index];
    p_rx_pkt_msg = &g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[index];

    if (xSemaphoreTake(p_rx_pkt_mutex, portMAX_DELAY))
    {
        if (pdTRUE != p_rx_pkt_msg->use)
        {
            PDMA_DEBUG_DETAIL("Warning: Abnormal status of ethernet pkt buf %d\n", index);
        }
        p_rx_pkt_msg->use = pdFALSE;

        if(g_debug_record)
        {
            g_debug_record_release_cnt++;
        }
        xSemaphoreGive(p_rx_pkt_mutex);
    }
    else
    {
        PDMA_DEBUG_DETAIL("Error: Failed to release ethernet pkt buf %d\n", index);
    }
}

void dump_pkt_data(unsigned char *addr, unsigned long len)
{
    register int n, m, c, r;
    unsigned char temp[16];

    printf("\n");

    for( n = len; n > 0; )
    {
        printf("%08lx ", (long unsigned int)addr);
        r = n < 16? n: 16;
        memcpy((void *) temp, (void *) addr, r);
        addr += r;
        for( m = 0; m < r; ++m )
        {
            printf("%c", (m & 3) == 0 && m > 0? '.': ' ');
            printf("%02x", temp[m]);
        }
        for(; m < 16; ++m )
            printf("   ");
        printf("  |");
        for( m = 0; m < r; ++m )
        {
            c = temp[m];
            printf("%c", ' ' <= c && c <= '~'? c: '.');
        }
        n -= r;
        for(; m < 16; ++m )
                printf(" ");
        printf("|\n");
    }

    printf("\n");
}


void pdma_dump_rx_desc(void)
{
    RX_DESC_T *p_rx_desc = NULL;
    int i = 0, rxq = 0;
    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        printf("================== Rx Ring %d ==================\n", rxq);
        printf("Current DESC idx : %d\n", g_pdma_cfg.rx_cur_desc[rxq]);
        for (i = 0 ; i< PDMA_RXDESC_NO; i++)
        {
            p_rx_desc = &g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][i];
            air_dcache_inv(p_rx_desc, sizeof(RX_DESC_T));

            printf("DESC %2d, PDP0: 0x%x, PLEN0: %d, LS0: 0x%x, DDONE_bit: 0x%x\n",
                i, p_rx_desc->rxd_info1.bits.PDP0, p_rx_desc->rxd_info2.bits.PLEN0,
                   p_rx_desc->rxd_info2.bits.LS0, p_rx_desc->rxd_info2.bits.DDONE_bit);
        }
    }
}

void pdma_dump_tx_desc(void)
{
    TX_DESC_T *p_tx_desc = NULL;
    int i = 0, txq = 0;

    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        printf("================== Tx Ring %d ==================\n", txq);
        printf("Current DESC idx   : %d\n", g_pdma_cfg.tx_cur_desc[txq]);
        printf("Unreleased DESC    : %d\n", g_pdma_cfg.tx_unreleased_desc[txq]);
        printf("Unreleased BUF cnt : %d\n", g_pdma_cfg.tx_unreleased_buf_cnt[txq]);
        for (i = 0 ; i < PDMA_TXDESC_NO ; i++)
        {
            p_tx_desc = &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][i];
            air_dcache_inv(p_tx_desc, sizeof(TX_DESC_T));

            printf("DESC %2d, SDP0: 0x%x, SDL1: %d, LS1_bit: 0x%x, BURST_bit: 0x%x, SDL0: 0x%x, LS0_bit: 0x%x, DDONE_bit: 0x%x, SDP1: 0x%x\n",
                i, p_tx_desc->txd_info1.bits.SDP0, p_tx_desc->txd_info2.bits.SDL1,
                   p_tx_desc->txd_info2.bits.LS1_bit, p_tx_desc->txd_info2.bits.BURST_bit,
                   p_tx_desc->txd_info2.bits.SDL0, p_tx_desc->txd_info2.bits.LS0_bit,
                   p_tx_desc->txd_info2.bits.DDONE_bit, p_tx_desc->txd_info3.bits.SDP1);
        }
    }
}

void pdma_debug(const char * cmd)
{
    char str[32];
    char sub_cmd[32];
    char str_buf[20];
    unsigned int enable = 0;
    int i, j;

    sscanf(cmd, "%s %s %d", str, sub_cmd, &enable);

    if (!strncmp(sub_cmd, "detail", strlen("detail")))
    {
        if (enable)
        {
            g_debug_detail = enable;
        }
        else
        {
            g_debug_detail = 0;
        }
    }
    else if (!strncmp(sub_cmd, "record", strlen("record")))
    {
        if (3 == enable)
        {
            printf("%-12s%s%d\n", "CNT",": ",g_probe_cnt%12);
            printf("%-12s%s", "ISR_CNT",": ");
            for (i = 0; i < 12; i++)
            {
                printf("%8d ", g_probe_isr_cnt[i]);
            }
            printf("\n");
            printf("%-12s%s", "CUR_IDX",": ");
            for (i = 0; i < 12; i++)
            {
                printf("%8d ", g_probe_cur_idx[i]);
            }
            printf("\n");
            printf("%-12s%s", "CPU_IDX",": ");
            for (i = 0; i < 12; i++)
            {
                printf("%8d ", g_probe_cpu_idx[i]);
            }
            printf("\n");
            printf("%-12s%s", "DMA_IDX",": ");
            for (i = 0; i < 12; i++)
            {
                printf("%8d ", g_probe_dma_idx[i]);
            }
            printf("\n");
            printf("%-12s%s", "PKT_CNT",": ");
            for (i = 0; i < 12; i++)
            {
                printf("%8d ", g_probe_pkt_cnt[i]);
            }
            printf("\n");
            for (i = 0; i < 12; i++)
            {
                memset(str_buf, 0, sizeof(str_buf));
                sprintf((char *)str_buf, "%s[%d]", "PKT_CNT", i);
                printf("%-12s%s", str_buf, ": ");
                for(j = 0; j < 12; j++)
                {
                    printf("%8d ", g_probe_pkt_cnt_change_in_intr[i][j]);
                }
                printf("\n");
            }
            printf("\n");
        }
        else if (2 == enable)
        {
            /* pdma_rx_ring_proc + get/release ethernet_pkt buf */
            printf("PDMA Rx pkt cnt         : %u\n     Rx drop pkt cnt    : %u\n     Rx eth get cnt     : %u\n     Rx eth release cnt : %u\n",
                    g_debug_record_cnt, g_debug_record_drop_cnt, g_debug_record_get_cnt, g_debug_record_release_cnt);

            /* pdma_tx_pkt */
            printf("PDMA Tx full cnt        : %u\n     Tx donebit err     : %u\n", g_pdma_tx_full_cnt, g_pdma_tx_done_bit_err);
        }
        else if (enable)
        {
            g_debug_record = 1;
        }
        else
        {
            g_debug_record = 0;
            g_debug_record_cnt = 0;
            g_debug_record_drop_cnt = 0;
            g_debug_record_get_cnt = 0;
            g_debug_record_release_cnt = 0;
            g_pdma_tx_full_cnt = 0;
            g_pdma_tx_done_bit_err = 0;
        }
    }
    else if (!strncmp(sub_cmd, "dump-rx-desc", strlen("dump-rx-desc")))
    {
        pdma_dump_rx_desc();
    }
    else if (!strncmp(sub_cmd, "dump-tx-desc", strlen("dump-tx-desc")))
    {
        pdma_dump_tx_desc();
    }
    else if (!strncmp(sub_cmd, "test-tx-cnt", strlen("test-tx-cnt")))
    {
        g_test_tx_cnt = enable;
    }
    else if (!strncmp(sub_cmd, "test-tx-len", strlen("test-tx-len")))
    {
        g_test_tx_len = enable;
    }
    else if (!strncmp(sub_cmd, "test-tx-delay", strlen("test-tx-delay")))
    {
        g_test_tx_delay = enable;
    }
    else if (!strncmp(sub_cmd, "test-tx-start", strlen("test-tx-start")))
    {
        pdma_test_tx(enable);
    }
    else if (!strncmp(sub_cmd, "perf-test", strlen("perf-test")))
    {
        pdma_performance_test();
    }
}

void pdma_rx_pkt(int rxq)
{
    RX_DESC_T *p_rx_desc = NULL;
    //void* pkt_data;
    //int pkt_len;

    p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][g_pdma_cfg.rx_cur_desc[rxq]];
    air_dcache_inv(((unsigned long)p_rx_desc), sizeof(RX_DESC_T));
    p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);

    if (p_rx_desc->rxd_info2.bits.DDONE_bit)
    {
        PDMA_DEBUG_DETAIL("\r\nDebug: RX Ring %d, rx_cur_desc %d\r\n", rxq, g_pdma_cfg.rx_cur_desc[rxq]);
        PDMA_DEBUG_DETAIL("Debug: INFO1.PDP0: 0x%x, INFO2.PLEN0: %d, INFO2.LS0: 0x%x, INFO2.DDONE_bit: 0x%x\n",
            p_rx_desc->rxd_info1.bits.PDP0, p_rx_desc->rxd_info2.bits.PLEN0,
            p_rx_desc->rxd_info2.bits.LS0, p_rx_desc->rxd_info2.bits.DDONE_bit);

        //pkt_data = p_rx_desc->rxd_info1.bits.PDP0;
        //pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;

        if (g_debug_record)
            g_debug_record_cnt++;

        //air_dcache_inv(((unsigned long)pkt_data), pkt_len);

        //memset(pkt_data, 0, pkt_len);
        //air_dcache_wback_inv(((unsigned long)pkt_data), pkt_len);

        p_rx_desc->rxd_info1.word = 0;
        p_rx_desc->rxd_info2.word = 0;
        p_rx_desc->rxd_info3.word = 0;
        p_rx_desc->rxd_info4.word = 0;

        p_rx_desc->rxd_info2.bits.LS0 = 1;
        p_rx_desc->rxd_info1.bits.PDP0 = (unsigned int)g_pdma_cfg.rx_pkt_data[rxq][g_pdma_cfg.rx_cur_desc[rxq]%PDMA_RXDESC_UNIT];
        p_rx_desc->rxd_info2.bits.PLEN0  = PDMA_RX_PKT_LEN;

        /* cache write back */
        //air_dcache_wback_inv(p_rx_desc, sizeof(RX_DESC_T));

        io_write32(RX_CALC_IDX(rxq), (g_pdma_cfg.rx_cur_desc[rxq]+PDMA_RXDESC_UNIT)%PDMA_RXDESC_NO);
        g_pdma_cfg.rx_cur_desc[rxq] = (g_pdma_cfg.rx_cur_desc[rxq] + 1) % PDMA_RXDESC_NO;
    }
}

void pdma_performance_test(void)
{
    unsigned int val = 0;
    unsigned char * pkt = NULL;
    int i;

    pkt = pvPortMalloc(PDMA_ETHERNET_TX_PKT_LEN);
    if (!pkt)
    {
        PDMA_DEBUG_DETAIL("Error: not enough memory!\n");
        return;
    }

    /* prepare packet payload */
    pkt[0]  = 0x00;
    pkt[1]  = 0x02;
    pkt[2]  = 0x00;
    pkt[3]  = 0x02;
    pkt[4]  = 0x00;
    pkt[5]  = 0x01;
    pkt[6]  = 0x00;
    pkt[7]  = 0x10;
    pkt[8]  = 0x94;
    pkt[9]  = 0x00;
    pkt[10] = 0x00;
    pkt[11] = 0x11;
    pkt[12] = 0x88;
    pkt[13] = 0xb5;

    for (i = 14; i < g_test_tx_len; i++)
    {
        if (i % 2)
            pkt[i] = 0xaa;
        else
            pkt[i] = 0x55;
    }

    val = RX_COHERENT | TX_COHERENT;
    io_write32(INT_MASK, val);

    while(1)
    {
        for (i = 0; i < g_test_tx_cnt; i++)
            pdma_tx_pkt(pkt, g_test_tx_len, 0, 0);
        for (i = 0; i < g_test_tx_cnt; i++)
            pdma_rx_pkt(0);
    }

    vPortFree(pkt);
}

void pdma_test_tx(int burst)
{
    unsigned char * pkt = NULL;
    int i;

    pkt = pvPortMalloc(PDMA_ETHERNET_TX_PKT_LEN);
    if (!pkt)
    {
        PDMA_DEBUG_DETAIL("Error: not enough memory!\n");
        return;
    }

    /* prepare packet payload */
    pkt[0]  = 0x00;
    pkt[1]  = 0x02;
    pkt[2]  = 0x00;
    pkt[3]  = 0x02;
    pkt[4]  = 0x00;
    pkt[5]  = 0x01;
    pkt[6]  = 0x00;
    pkt[7]  = 0x10;
    pkt[8]  = 0x94;
    pkt[9]  = 0x00;
    pkt[10] = 0x00;
    pkt[11] = 0x11;
    pkt[12] = 0x88;
    pkt[13] = 0xb5;

    for (i = 14; i < g_test_tx_len; i++)
    {
        if (i % 2)
            pkt[i] = 0xaa;
        else
            pkt[i] = 0x55;
    }

    for (i = 0; i < g_test_tx_cnt; i++)
    {
        pdma_tx_pkt(pkt, g_test_tx_len, 0, burst);
        if (g_test_tx_delay)
        {
            vTaskDelay(pdMS_TO_TICKS(g_test_tx_delay));
        }
    }

    vPortFree(pkt);
}

static void pdma_tx_ring_proc(int txq)
{
    TX_DESC_T *p_tx_desc = NULL;

    p_tx_desc = &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][g_pdma_cfg.tx_unreleased_desc[txq]];
    air_dcache_inv(((unsigned long)p_tx_desc), sizeof(TX_DESC_T));
    p_tx_desc = (TX_DESC_T*)((unsigned int)p_tx_desc | HIGH_BIT_UNC);

    while (g_pdma_cfg.tx_unreleased_buf_cnt[txq] != 0)
    {
        if (!p_tx_desc->txd_info2.bits.DDONE_bit)
        {
            return;
        }

        if (g_pdma_cfg.tx_unreleased_desc[txq] == (PDMA_TXDESC_NO - 1))
        {
            g_pdma_cfg.tx_unreleased_desc[txq] = 0;
        }
        else
        {
            g_pdma_cfg.tx_unreleased_desc[txq]++;
        }
        g_pdma_cfg.tx_unreleased_buf_cnt[txq]--;

        p_tx_desc = &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][g_pdma_cfg.tx_unreleased_desc[txq]];
        air_dcache_inv(((unsigned long)p_tx_desc), sizeof(TX_DESC_T));
        p_tx_desc = (TX_DESC_T*)((unsigned int)p_tx_desc | HIGH_BIT_UNC);
    }
}

int pdma_tx_pkt(unsigned char* data, int len, int txq, int cp_to_sec_desc)
{
    TX_DESC_T *p_tx_desc = NULL;

    if (!data || (len > PDMA_ETHERNET_TX_PKT_LEN) || (txq >= PDMA_TX_QUEUE_NUM))
    {
        return -1;
    }

    //TODO: MUTEX needed
    if (g_pdma_cfg.tx_unreleased_buf_cnt[txq] >= PDMA_TX_BUF_RELEASE_THRESHOLD)
    {
        pdma_tx_ring_proc(txq);
    }

    if (g_pdma_cfg.tx_unreleased_buf_cnt[txq] == (PDMA_TXDESC_NO - 1))
    {
        g_pdma_tx_full_cnt++;
        PDMA_DEBUG_DETAIL("\r\nDebug: TX ring %d full!\r\n", txq);
        return -1;
    }

    p_tx_desc = &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][g_pdma_cfg.tx_cur_desc[txq]];
    air_dcache_inv(((unsigned long)p_tx_desc), sizeof(TX_DESC_T));
    p_tx_desc = (TX_DESC_T*)((unsigned int)p_tx_desc | HIGH_BIT_UNC);

    if (!p_tx_desc->txd_info2.bits.DDONE_bit)
    {
        g_pdma_tx_done_bit_err++;
        PDMA_DEBUG_DETAIL("\r\nDebug: TX ring %d done bit error!\r\n", txq);
        return -1;
    }

    p_tx_desc->txd_info1.bits.SDP0 = (unsigned int)g_pdma_cfg.tx_pkt_data[txq][g_pdma_cfg.tx_cur_desc[txq]];
    p_tx_desc->txd_info2.bits.SDL0 = len > 60 ? len : 60;
    if (cp_to_sec_desc)
    {
        p_tx_desc->txd_info3.bits.SDP1 = (unsigned int)g_pdma_cfg.tx_pkt_data[txq][g_pdma_cfg.tx_cur_desc[txq]];
        p_tx_desc->txd_info2.bits.SDL1 = len > 60 ? len : 60;
        p_tx_desc->txd_info2.bits.LS1_bit = 1;
        p_tx_desc->txd_info2.bits.BURST_bit = 1;
    }
    else
    {
        p_tx_desc->txd_info3.bits.SDP1 = 0;
        p_tx_desc->txd_info2.bits.SDL1 = 0;
        p_tx_desc->txd_info2.bits.LS1_bit = 0;
        p_tx_desc->txd_info2.bits.BURST_bit = 0;
    }
    //p_tx_desc->txd_info4.word = 0;
    p_tx_desc->txd_info2.bits.DDONE_bit = 0;

    PDMA_DEBUG_DETAIL("\r\nDebug: TX Ring %d, tx_cur_desc %d\r\n", txq, g_pdma_cfg.tx_cur_desc[txq]);
    PDMA_DEBUG_DETAIL("Debug: INFO1.SDP0: 0x%x, INFO2.SDL0: %d, INFO2.LS0_bit: 0x%x, INFO2.DDONE_bit: 0x%x\n",
        p_tx_desc->txd_info1.bits.SDP0, p_tx_desc->txd_info2.bits.SDL0,
        p_tx_desc->txd_info2.bits.LS0_bit, p_tx_desc->txd_info2.bits.DDONE_bit);

    if ((g_debug_detail == 2) && (len <= PDMA_TX_PKT_LEN))
    {
        dump_pkt_data(data, len);
    }
    memcpy(g_pdma_cfg.tx_pkt_data[txq][g_pdma_cfg.tx_cur_desc[txq]], data, len);
    if (len < 60)
    {
        memset((((char *)g_pdma_cfg.tx_pkt_data[txq][g_pdma_cfg.tx_cur_desc[txq]])+len), 0, 60 - len);
    }
    air_dcache_wback_inv(((unsigned long)g_pdma_cfg.tx_pkt_data[txq][g_pdma_cfg.tx_cur_desc[txq]]), len);

    g_pdma_cfg.tx_cur_desc[txq] = (g_pdma_cfg.tx_cur_desc[txq] + 1) % PDMA_TXDESC_NO;
    io_write32(TX_CTX_IDX(txq), g_pdma_cfg.tx_cur_desc[txq]);

    g_pdma_cfg.tx_unreleased_buf_cnt[txq]++;

    return 0;
}

static void pdma_desc_init(void)
{
    RX_DESC_T *p_rx_desc = NULL;
    TX_DESC_T *p_tx_desc = NULL;
    int i = 0, txq = 0, rxq = 0;

    /* init. Rx descriptor */
    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        for (i = 0 ; i< PDMA_RXDESC_NO; i++)
        {
            p_rx_desc = &g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][i];
            air_dcache_inv(((unsigned long)p_rx_desc), sizeof(RX_DESC_T));
            p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);

            // Init Descriptor
            p_rx_desc->rxd_info1.word = 0;
            p_rx_desc->rxd_info2.word = 0;
            p_rx_desc->rxd_info3.word = 0;
            p_rx_desc->rxd_info4.word = 0;

            // Assign flag
            p_rx_desc->rxd_info2.bits.LS0 = 1;
            p_rx_desc->rxd_info1.bits.PDP0 = (unsigned int)g_pdma_cfg.rx_pkt_data[rxq][i%PDMA_RXDESC_UNIT];
            p_rx_desc->rxd_info2.bits.PLEN0  = PDMA_RX_PKT_LEN;

            //air_dcache_wback_inv(p_rx_desc, sizeof(RX_DESC_T));
        }
    }

    /* init. tx descriptor, don't allocate memory */
    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        for (i = 0 ; i < PDMA_TXDESC_NO ; i++)
        {
            p_tx_desc = &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][i];
            air_dcache_inv(((unsigned long)p_tx_desc), sizeof(TX_DESC_T));
            p_tx_desc = (TX_DESC_T*)((unsigned int)p_tx_desc | HIGH_BIT_UNC);

            // Init descriptor
            p_tx_desc->txd_info1.word = 0;
            p_tx_desc->txd_info2.word = 0;
            p_tx_desc->txd_info3.word = 0;
            p_tx_desc->txd_info4.word = 0;

            // Assign flag
            p_tx_desc->txd_info2.bits.LS0_bit = 1;
            p_tx_desc->txd_info2.bits.DDONE_bit = 1;

            /* CPU */
            //pTxDescp->txd_info4.bits.PN = 0;
            /* GDMA1 */
            p_tx_desc->txd_info4.bits.PN = 1;

            //air_dcache_wback_inv(p_tx_desc, sizeof(TX_DESC_T));
        }
    }

    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        g_pdma_cfg.rx_cur_desc[rxq] = 0;
    }

    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        g_pdma_cfg.tx_cur_desc[txq] = 0;
        g_pdma_cfg.tx_unreleased_desc[txq] = 0;
        g_pdma_cfg.tx_unreleased_buf_cnt[txq] = 0;
    }
}

static void pdma_driver_reg_init(void)
{
    unsigned int val = 0;
    int txq = 0, rxq = 0;

    /* pdma intr mask */
    //val = RX_COHERENT | RX_DLY_INT | TX_COHERENT | TX_DLY_INT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0 | TX_DONE_INT1 | TX_DONE_INT0;
    val = RX_COHERENT | RX_DLY_INT | TX_COHERENT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0;
    //val = RX_COHERENT | RX_DLY_INT | TX_COHERENT |  RX_DONE_INT1 | RX_DONE_INT0;
    io_write32(INT_MASK, val);

    /* DESC ptr & counter */
    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        io_write32(TX_BASE_PTR(txq), (unsigned int)g_pdma_cfg.tx_ring_base_addr[txq]);
        io_write32(TX_MAX_CNT(txq), PDMA_TXDESC_NO);
        io_write32(TX_CTX_IDX(txq), 0);
        io_write32(PDMA_RST_IDX, RST_DTX_IDX(txq));
    }

    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        io_write32(RX_BASE_PTR(rxq), (unsigned int)g_pdma_cfg.rx_ring_base_addr[rxq]);
        io_write32(RX_MAX_CNT(rxq), PDMA_RXDESC_NO);
        io_write32(RX_CALC_IDX(rxq), PDMA_RXDESC_UNIT - 1);
        io_write32(PDMA_RST_IDX, RST_DRX_IDX(rxq));
    }

    val = io_read32(PDMA_FC_CFG);
    val |= PDMA_FC_CFG_CSR_PDMA2GSW_CRC_EN_MASK;
    io_write32(PDMA_FC_CFG, val);

    val = io_read32(PDMA_GLO_CFG);
    val |= PDMA_GLO_CFG_CSR_WB_DDONE_MASK;
    //val &= ~PDMA_GLO_CFG_PDMA_BT_SIZE_MASK;
    //val |= (PDMA_BT_SIZE_32DW & PDMA_GLO_CFG_PDMA_BT_SIZE_RELMASK) << PDMA_GLO_CFG_PDMA_BT_SIZE_OFFT;
    val |= PDMA_GLO_CFG_RX_DMA_EN_MASK | PDMA_GLO_CFG_TX_DMA_EN_MASK;
    //val |= PDMA_GLO_CFG_CSR_BYTE_SWAP_MASK;
    //val |= PDMA_GLO_CFG_RX_2B_OFFSET_MASK;
    io_write32(PDMA_GLO_CFG, val);

    /* set cpu port 28 mac force 1G, FC enable */

/* macro definition */
#if defined(AIR_8851_SUPPORT)
    io_write32(0x10213800, 0x17983b);
#elif defined(AIR_8855_SUPPORT)
    io_write32(0x10210c00, 0xa31593c0);
#elif defined(AIR_8858_SUPPORT)
    io_write32(0x10211400, 0xa31593f0);
#else
    #error "todo"
#endif

}

void pdma_rx_ring_proc(int rxq)
{
    RX_DESC_T *p_rx_desc = NULL;
    void* pkt_data;
    int pkt_len;
    int pkt_cnt = 0;
    int eth_pkt_idx = 0;
    SemaphoreHandle_t  p_rx_pkt_mutex = NULL;
    ETHERNET_PKT_MSG_T* p_rx_pkt_msg = NULL;
    BaseType_t drop_pkt = pdFALSE;
    int rx_cur_desc_start = 0;
    int rx_split_cnt = 1, i = 0, merge_len = 0, loop_time = 0;

    if (rxq >= PDMA_RX_QUEUE_NUM)
    {
        return;
    }

    p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][g_pdma_cfg.rx_cur_desc[rxq]];
    air_dcache_inv(((unsigned long)p_rx_desc), sizeof(RX_DESC_T));
    p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);

    while(p_rx_desc->rxd_info2.bits.DDONE_bit)
    {
        PDMA_DEBUG_DETAIL("\r\nDebug: RX Ring %d, rx_cur_desc %d\r\n", rxq, g_pdma_cfg.rx_cur_desc[rxq]);
        //PDMA_DEBUG_DETAIL("Debug: INFO1 0x%x, INFO2:0x%x, INFO3:0x%x, INFO4:0x%x\r\n",
        //    p_rx_desc->rxd_info1.word, p_rx_desc->rxd_info2.word, p_rx_desc->rxd_info3.word, p_rx_desc->rxd_info4.word);
        PDMA_DEBUG_DETAIL("Debug: INFO1.PDP0: 0x%x, INFO2.PLEN0: %d, INFO2.LS0: 0x%x, INFO2.DDONE_bit: 0x%x\n",
            p_rx_desc->rxd_info1.bits.PDP0, p_rx_desc->rxd_info2.bits.PLEN0,
            p_rx_desc->rxd_info2.bits.LS0, p_rx_desc->rxd_info2.bits.DDONE_bit);

        if(p_rx_desc->rxd_info2.bits.LS0)
        {
            pkt_data = (void *)p_rx_desc->rxd_info1.bits.PDP0;
            pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;

        //PDMA_DEBUG_DETAIL("Debug: PDP0: 0x%x, PLEN0: %d\r\n", pkt_data, pkt_len);
        if (g_debug_record)
            g_debug_record_cnt++;

            air_dcache_inv(((unsigned long)pkt_data), pkt_len);

            if ((g_debug_detail == 2) && (pkt_len <= PDMA_RX_PKT_LEN))
            {
                dump_pkt_data(pkt_data, pkt_len);
            }

        if (g_pkt_queue)
        {
            drop_pkt = pdTRUE;
            for (eth_pkt_idx = 0; eth_pkt_idx < ETHERNET_RX_PKT_BUFFER_NUM; eth_pkt_idx++)
            {
                p_rx_pkt_mutex = g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_mutex[(g_pdma_cfg.rx_cur_pkt_msg + eth_pkt_idx) % ETHERNET_RX_PKT_BUFFER_NUM];
                if (xSemaphoreTakeFromISR(p_rx_pkt_mutex, NULL))
                {
                    p_rx_pkt_msg = &g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[(g_pdma_cfg.rx_cur_pkt_msg + eth_pkt_idx) % ETHERNET_RX_PKT_BUFFER_NUM];
                    if (pdFALSE == p_rx_pkt_msg->use)
                    {
                        /* update rx pkt msg content */
                        p_rx_pkt_msg->use = pdTRUE;
                        memcpy((void *)p_rx_pkt_msg->data, pkt_data, pkt_len);
                        p_rx_pkt_msg->len = pkt_len;

                        /* set drop_pkt flag to FALSE to escape rx pkt buf lookup */
                        drop_pkt = pdFALSE;

                        /* send found rx pkt buf idx to MAC recv task by queue */
                        if (pdPASS == xQueueSendFromISR(g_pkt_queue, &g_pdma_cfg.rx_cur_pkt_msg, NULL))
                        {
                            PDMA_DEBUG_DETAIL("\r\nDebug: rx_cur_pkt_msg %d, data 0x%x, len %d\r\n", g_pdma_cfg.rx_cur_pkt_msg, p_rx_pkt_msg->data, pkt_len);
                        }
                        else
                        {
                            /* roll back rx pkt buf use flag */
                            p_rx_pkt_msg->use = pdFALSE;
                            PDMA_DEBUG_DETAIL("Warning: MAC recv queue full!\n");
                        }
                    }

                    xSemaphoreGiveFromISR(p_rx_pkt_mutex, NULL);
                }
                g_pdma_cfg.rx_cur_pkt_msg = (g_pdma_cfg.rx_cur_pkt_msg + 1) % ETHERNET_RX_PKT_BUFFER_NUM;

                if (pdFALSE == drop_pkt)
                {
                    break;
                }
            }

            if (pdTRUE == drop_pkt)
            {
                PDMA_DEBUG_DETAIL("Warning: Pkt dropped due to no available pkt buf!\n");
                if (g_debug_record)
                {
                    g_debug_record_drop_cnt++;
                }
            }
        }

        memset(pkt_data, 0, pkt_len);
        air_dcache_wback_inv(((unsigned long)pkt_data), pkt_len);

        p_rx_desc->rxd_info1.word = 0;
        p_rx_desc->rxd_info2.word = 0;
        p_rx_desc->rxd_info3.word = 0;
        p_rx_desc->rxd_info4.word = 0;

        p_rx_desc->rxd_info2.bits.LS0 = 1;
        p_rx_desc->rxd_info1.bits.PDP0 = (unsigned int)g_pdma_cfg.rx_pkt_data[rxq][g_pdma_cfg.rx_cur_desc[rxq]%PDMA_RXDESC_UNIT];
        p_rx_desc->rxd_info2.bits.PLEN0  = PDMA_RX_PKT_LEN;

        /* cache write back */
        //air_dcache_wback_inv(p_rx_desc, sizeof(RX_DESC_T));

        io_write32(RX_CALC_IDX(rxq), (g_pdma_cfg.rx_cur_desc[rxq] + PDMA_RXDESC_UNIT) % PDMA_RXDESC_NO);
        g_pdma_cfg.rx_cur_desc[rxq] = (g_pdma_cfg.rx_cur_desc[rxq] + 1) % PDMA_RXDESC_NO;
        p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][g_pdma_cfg.rx_cur_desc[rxq]];
        air_dcache_inv(p_rx_desc, sizeof(RX_DESC_T));
        p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);

        pkt_cnt++;
        }
        else
        {
            rx_cur_desc_start = g_pdma_cfg.rx_cur_desc[rxq]; /* save start desc index*/

            rx_split_cnt = 1;
            /*get packet first desc length*/
            merge_len = p_rx_desc->rxd_info2.bits.PLEN0;
            while( p_rx_desc->rxd_info2.bits.LS0 != 1) /* calculate split cnt */
            {
                if(p_rx_desc->rxd_info2.bits.DDONE_bit)
                {
                    /* find next rx desc */
                    g_pdma_cfg.rx_cur_desc[rxq] = (g_pdma_cfg.rx_cur_desc[rxq] + 1) % PDMA_RXDESC_NO;
                    p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][g_pdma_cfg.rx_cur_desc[rxq]];
                    air_dcache_inv(p_rx_desc, sizeof(RX_DESC_T));
                    p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);
                    /*caculate the accumulated length of the rx desc received packet length*/
                    pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;
                    merge_len = merge_len + pkt_len;
                }
                rx_split_cnt ++;
            }

            if (g_debug_record)
                g_debug_record_cnt++;

            /* rx_split_cnt start at 1, it would include the last desc segment (LS bits = 1) */
            PDMA_DEBUG_DETAIL("rx_split_cnt : %d, pkt_len : %d\r\n", rx_split_cnt, merge_len);

            /* reload start desc index*/
            PDMA_DEBUG_DETAIL("rx_cur_desc_start %d\r\n", rx_cur_desc_start);
            p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][rx_cur_desc_start];
            pkt_data = (void *)p_rx_desc->rxd_info1.bits.PDP0;
            pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;

            /*if packet size exceeds PDMA_ETHERNET_RX_PKT_LEN, the data will not be copied to ETHERNET_RX_PKT_BUFFER_ADDR*/
            if (g_pkt_queue && merge_len <= PDMA_ETHERNET_RX_PKT_LEN)
            {
                drop_pkt = pdTRUE;
                for (eth_pkt_idx = 0; eth_pkt_idx < ETHERNET_RX_PKT_BUFFER_NUM; eth_pkt_idx++)
                {
                    p_rx_pkt_mutex = g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_mutex[(g_pdma_cfg.rx_cur_pkt_msg + eth_pkt_idx) % ETHERNET_RX_PKT_BUFFER_NUM];
                    if (xSemaphoreTakeFromISR(p_rx_pkt_mutex, NULL))
                    {
                        p_rx_pkt_msg = &g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[(g_pdma_cfg.rx_cur_pkt_msg + eth_pkt_idx) % ETHERNET_RX_PKT_BUFFER_NUM];
                        if (pdFALSE == p_rx_pkt_msg->use)
                        {
                            PDMA_DEBUG_DETAIL("\r\n eth_pkt_idx %d \r\n", eth_pkt_idx);
                            /* update rx pkt msg content */
                            p_rx_pkt_msg->use = pdTRUE;

                            merge_len = 0;
                            for(i= 0; i< rx_split_cnt; i++)
                            {
                                p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][(rx_cur_desc_start + i) % PDMA_RXDESC_NO];
                                pkt_data = (void *)p_rx_desc->rxd_info1.bits.PDP0;
                                pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;
                                air_dcache_inv(((unsigned long)pkt_data), pkt_len);
                                memcpy((void *)(p_rx_pkt_msg->data + merge_len), pkt_data, pkt_len);
                                merge_len = merge_len + pkt_len;
                            }
                            p_rx_pkt_msg->len = merge_len;

                            /* Dump packet result with merge from multiple rx desc */
                            if ((g_debug_detail == 2) && (merge_len <= PDMA_RX_PKT_LEN))
                            {
                                dump_pkt_data((unsigned char *)p_rx_pkt_msg->data, p_rx_pkt_msg->len);
                            }

                            /* set drop_pkt flag to FALSE to escape rx pkt buf lookup */
                            drop_pkt = pdFALSE;

                            /* send found rx pkt buf idx to MAC recv task by queue */
                            if (pdPASS == xQueueSendFromISR(g_pkt_queue, &g_pdma_cfg.rx_cur_pkt_msg, NULL))
                            {
                                PDMA_DEBUG_DETAIL("\r\nDebug: rx_cur_pkt_msg %d, data 0x%x, len %d\r\n", g_pdma_cfg.rx_cur_pkt_msg, p_rx_pkt_msg->data, p_rx_pkt_msg->len);
                            }
                            else
                            {
                                /* roll back rx pkt buf use flag */
                                p_rx_pkt_msg->use = pdFALSE;
                                PDMA_DEBUG_DETAIL("Warning: MAC recv queue full!\n");
                            }
                        }

                        xSemaphoreGiveFromISR(p_rx_pkt_mutex, NULL);
                    }
                    g_pdma_cfg.rx_cur_pkt_msg = (g_pdma_cfg.rx_cur_pkt_msg + 1) % ETHERNET_RX_PKT_BUFFER_NUM;

                    if (pdFALSE == drop_pkt)
                    {
                        break;
                    }

                }

                if (pdTRUE == drop_pkt)
                {
                    if (g_debug_record)
                    {
                        g_debug_record_drop_cnt++;
                    }
                }
            }

            for(i= 0; i< rx_split_cnt; i++) /* clear rx desc */
            {
                p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][(rx_cur_desc_start + i) % PDMA_RXDESC_NO];
                air_dcache_inv(p_rx_desc, sizeof(RX_DESC_T));
                p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);

                pkt_data = (void *)p_rx_desc->rxd_info1.bits.PDP0;
                pkt_len  = p_rx_desc->rxd_info2.bits.PLEN0;

                memset(pkt_data, 0, pkt_len);
                air_dcache_wback_inv(((unsigned long)pkt_data), pkt_len);

                p_rx_desc->rxd_info1.word = 0;
                p_rx_desc->rxd_info2.word = 0;
                p_rx_desc->rxd_info3.word = 0;
                p_rx_desc->rxd_info4.word = 0;

                p_rx_desc->rxd_info2.bits.LS0 = 1;
                p_rx_desc->rxd_info1.bits.PDP0 = (unsigned int)g_pdma_cfg.rx_pkt_data[rxq][((rx_cur_desc_start + i) % PDMA_RXDESC_NO) % PDMA_RXDESC_UNIT];
                p_rx_desc->rxd_info2.bits.PLEN0  = PDMA_RX_PKT_LEN;

                PDMA_DEBUG_DETAIL("[clear][idx %d ] INFO1.PDP0: 0x%x, INFO2.PLEN0: %d, INFO2.LS0: 0x%x, INFO2.DDONE_bit: 0x%x\n",
                    (rx_cur_desc_start + i) % PDMA_RXDESC_NO,
                    p_rx_desc->rxd_info1.bits.PDP0, p_rx_desc->rxd_info2.bits.PLEN0,
                    p_rx_desc->rxd_info2.bits.LS0, p_rx_desc->rxd_info2.bits.DDONE_bit);

                pkt_cnt++;
            }

            /* if this packet size is larger than PDMA_RXDESC_UNIT*PDMA_RX_PKT_LEN, then may encounter a situation
             * where there are some small packet following this packet. However, if pkt_cnt exceeds PDMA_ISR_RX_PKT_THRESHOLD,
             * it will break isr, if CALC_IDX = DRX_IDX at this time, PDMA will no longer generate an interrupt.
             */
            if(rx_split_cnt >= PDMA_RXDESC_UNIT)
            {
                pkt_cnt-=rx_split_cnt;
            }

            /* cache write back */
            //io_write32(RX_CALC_IDX(rxq), rx_cur_desc_start);
            io_write32(RX_CALC_IDX(rxq), (g_pdma_cfg.rx_cur_desc[rxq] + PDMA_RXDESC_UNIT) % PDMA_RXDESC_NO);
            PDMA_DEBUG_DETAIL("\r\n RX_CALC_IDX(rxq) = %d[cur] \r\n", (g_pdma_cfg.rx_cur_desc[rxq] + PDMA_RXDESC_UNIT) % PDMA_RXDESC_NO);
            g_pdma_cfg.rx_cur_desc[rxq] = (g_pdma_cfg.rx_cur_desc[rxq] + 1) % PDMA_RXDESC_NO;
            PDMA_DEBUG_DETAIL("\r\n new g_pdma_cfg.rx_cur_desc[rxq] = %d \r\n", g_pdma_cfg.rx_cur_desc[rxq]);
            p_rx_desc = (RX_DESC_T*)&g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][g_pdma_cfg.rx_cur_desc[rxq]];
            air_dcache_inv(p_rx_desc, sizeof(RX_DESC_T));
            p_rx_desc = (RX_DESC_T*)((unsigned int)p_rx_desc | HIGH_BIT_UNC);
        }

        g_probe_pkt_cnt_change_in_intr[g_probe_cnt % 12][loop_time] = pkt_cnt;
        loop_time++;

        if (pkt_cnt >= PDMA_ISR_RX_PKT_THRESHOLD)
        {
            break;
        }
    }
    g_probe_pkt_cnt[g_probe_cnt % 12] = pkt_cnt;
}

void pdma_isr (void)
{
    unsigned int val;
    unsigned int i;

    record_interrupts(IRQ_PDMA);

    val = io_read32(INT_STATUS);

    PDMA_DEBUG_DETAIL("Debug: pdma_isr INT_STATUS 0x%x\r\n", val);

    g_probe_isr_cnt[g_probe_cnt % 12] = g_probe_cnt;

    // ----------Packet Received----------------------
    if (val & (RX_DLY_INT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0))
    {
        //io_write32(INT_MASK, io_read32(INT_MASK) & ~(RX_DLY_INT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0));
        io_write32(INT_STATUS, val & (RX_DLY_INT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0));

        g_probe_cur_idx[g_probe_cnt % 12] = g_pdma_cfg.rx_cur_desc[0];
        g_probe_pkt_cnt[g_probe_cnt % 12] = 0;
        for(i = 0; i < 12; i++)
        {
            g_probe_pkt_cnt_change_in_intr[g_probe_cnt % 12][i] = 0;
        }
        if (val & RX_DONE_INT3)
            pdma_rx_ring_proc(3);
        if (val & RX_DONE_INT2)
            pdma_rx_ring_proc(2);
        if (val & RX_DONE_INT1)
            pdma_rx_ring_proc(1);
        if (val & RX_DONE_INT0)
            pdma_rx_ring_proc(0);

        g_probe_cpu_idx[g_probe_cnt % 12] = io_read32(RX_CALC_IDX(0));
        g_probe_dma_idx[g_probe_cnt % 12] = io_read32(RX_DRX_IDX(0));
        g_probe_cnt++;

        //io_write32(INT_MASK, io_read32(INT_MASK) | (RX_DLY_INT | RX_DONE_INT3 | RX_DONE_INT2 | RX_DONE_INT1 | RX_DONE_INT0));
    }

    if (val & RX_COHERENT)
    {
        io_write32(INT_MASK, io_read32(INT_MASK) & ~RX_COHERENT);
        io_write32(INT_STATUS, RX_COHERENT);

        printf("Error: PDMA RX_COHERENT!\n");
        pdma_dump_rx_desc();

        io_write32(INT_MASK, io_read32(INT_MASK) | RX_COHERENT);
    }
    if (val & TX_COHERENT)
    {
        io_write32(INT_MASK, io_read32(INT_MASK) & ~TX_COHERENT);
        io_write32(INT_STATUS, TX_COHERENT);

        printf("Error: PDMA TX_COHERENT!\n");
        pdma_dump_tx_desc();

        io_write32(INT_MASK, io_read32(INT_MASK) | TX_COHERENT);
    }
}

void pdma_init (void)
{
    int i, txq = 0, rxq = 0;
    void *pkt_data = NULL;
    memset(pdma_buffer, 0, sizeof(pdma_buffer));
    PDMA_DEBUG_DETAIL("========================================================================\n");
    PDMA_DEBUG_DETAIL("PDMA INIT STAGE:\n");
    PDMA_DEBUG_DETAIL("========================================================================\n");

    PDMA_DEBUG_DETAIL("Ethernet buffer ptr: 0x%x, Ethernet buffer cnt: %d\n", ETHERNET_RX_PKT_BUFFER_ADDR, ETHERNET_RX_PKT_BUFFER_NUM);
    g_pdma_cfg.rx_cur_pkt_msg = 0;
    g_pdma_cfg.rx_pkt_msg_pool = (ETHERNET_PKT_MSG_POOL_T*)ETHERNET_RX_PKT_BUFFER_ADDR;
    pkt_data = (void*)((void*)g_pdma_cfg.rx_pkt_msg_pool + sizeof(ETHERNET_PKT_MSG_POOL_T));
    PDMA_DEBUG_DETAIL("Ethernet buffer start ptr: 0x%x\n", (unsigned int)pkt_data);
    for (i = 0; i < ETHERNET_RX_PKT_BUFFER_NUM; i++)
    {
        g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_mutex[i] = xSemaphoreCreateMutex();
        if (!g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_mutex[i])
        {
            PDMA_DEBUG_DETAIL("Fatal: Failed to create mutex for ethernet rx pkt buf!\n");
            return;
        }
        g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[i].use  = pdFALSE;
        g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[i].data = (unsigned int)pkt_data;
        g_pdma_cfg.rx_pkt_msg_pool->rx_pkt_msg[i].len  = 0;
        pkt_data += PDMA_ETHERNET_RX_PKT_LEN;
    }
    PDMA_DEBUG_DETAIL("Ethernet buffer end ptr: 0x%x\n", (unsigned int)pkt_data);
    PDMA_DEBUG_DETAIL("PDMA buffer ptr: 0x%x, RX_MEM_POOL size: %d, TX_MEM_POOL size: %d\n", (unsigned int)PDMA_BUFFER_START_ADDR, (unsigned int)sizeof(RX_MEM_POOL_T), (unsigned int)sizeof(TX_MEM_POOL_T));
    g_pdma_cfg.rx_mem_pool = (RX_MEM_POOL_T*)PDMA_BUFFER_START_ADDR;
    g_pdma_cfg.tx_mem_pool = (TX_MEM_POOL_T*)((void*)g_pdma_cfg.rx_mem_pool + sizeof(RX_MEM_POOL_T));
    PDMA_DEBUG_DETAIL("RX_MEM_POOL ptr: 0x%x, TX_MEM_POOL ptr: 0x%x\n", (unsigned int)g_pdma_cfg.rx_mem_pool, (unsigned int)g_pdma_cfg.tx_mem_pool);

    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        g_pdma_cfg.rx_ring_base_addr[rxq] = (void*) &g_pdma_cfg.rx_mem_pool->rx_desc_buf[rxq][0];
        PDMA_DEBUG_DETAIL("RX RING %d base ptr: 0x%x\n", rxq, (unsigned int)g_pdma_cfg.rx_ring_base_addr[rxq]);
    }

    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        g_pdma_cfg.tx_ring_base_addr[txq] = (void*) &g_pdma_cfg.tx_mem_pool->tx_desc_buf[txq][0];
        PDMA_DEBUG_DETAIL("TX RING %d base ptr: 0x%x\n", txq, (unsigned int)g_pdma_cfg.tx_ring_base_addr[txq]);
    }

    PDMA_DEBUG_DETAIL("PDMA desc pkt length: %d\n", PDMA_TX_PKT_LEN);
    pkt_data = (void*)((void*)g_pdma_cfg.tx_mem_pool + sizeof(TX_MEM_POOL_T));
    for (rxq = 0; rxq < PDMA_RX_QUEUE_NUM; rxq++)
    {
        for (i = 0; i < PDMA_RXDESC_UNIT; i++)
        {
            g_pdma_cfg.rx_pkt_data[rxq][i] = pkt_data;
            pkt_data += PDMA_RX_PKT_LEN;
            PDMA_DEBUG_DETAIL("RX Ring %d DESC %d pkt ptr: 0x%x\n", rxq, i, (unsigned int)g_pdma_cfg.rx_pkt_data[rxq][i]);
        }
    }

    for (txq = 0; txq < PDMA_TX_QUEUE_NUM; txq++)
    {
        for (i = 0; i < PDMA_TXDESC_NO; i++)
        {
            g_pdma_cfg.tx_pkt_data[txq][i] = pkt_data;
            pkt_data += PDMA_TX_PKT_LEN;
            PDMA_DEBUG_DETAIL("TX Ring %d DESC %d pkt ptr: 0x%x\n", txq, i, (unsigned int)g_pdma_cfg.tx_pkt_data[txq][i]);
        }
    }

    pdma_desc_init();

    pdma_driver_reg_init();

    register_isr(IRQ_PDMA, pdma_isr);
    PDMA_DEBUG_DETAIL("__pdmabuffer_vmastart = %x\n", (unsigned int)&__pdmabuffer_vmastart);
    PDMA_DEBUG_DETAIL("__pdmabuffer_vmaend = %x\n", (unsigned int)&__pdmabuffer_vmaend);
    PDMA_DEBUG_DETAIL("========================================================================\n");
    PDMA_DEBUG_DETAIL("PDMA INIT DONE!\n");
    PDMA_DEBUG_DETAIL("========================================================================\n");
}

