#ifndef __XPORT_DMA_H__
#define __XPORT_DMA_H__

#include "xport_regs.h"

#define CLK27_MHZ                       27	/* 27MHZ */
#define MAX_DMA_MBPS                    20	/* 20MB/s */

#define XPORT_CHL0_MIN_SPACES           (XPORT_CHL0_UNIT_NUM_DEF >> 1)
#define XPORT_CHL1_MIN_SPACES           (XPORT_CHL1_UNIT_NUM_DEF >> 1)

#define XPORT_IRQ0_DMA0_MSK             0x00000001
#define XPORT_IRQ0_DMA1_MSK             0x00000002

#define XPORT_IRQ0_DMA0_EMPTY_MSK       0x00000040
#define XPORT_IRQ0_DMA1_EMPTY_MSK       0x00000080

void xport_dma_init(void);
int xport_dma_set(unsigned int dma_id);
int xport_dma_write(const char __user * buffer, size_t len, unsigned int dma_id);
int xport_dma_direct_write(const char __user * buffer, size_t len, unsigned int dma_id);
int xport_dma_input_check(unsigned int dma_id);
int xport_dma_reset(unsigned int dma_id);
int xport_dma_half_empty_check(unsigned int dma_id);

#endif
