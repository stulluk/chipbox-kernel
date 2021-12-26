#ifndef __XPORT_DRV_H__
#define __XPORT_DRV_H__

#include <linux/mem_define.h>

#include "xport_regs.h"
#include "xport_filter.h"

#define XPORT_TIMEOUT 			msecs_to_jiffies(5)

#define XPORT_REGS_BASE        		0x41400000
#define XPORT_REGS_SIZE        		0x18010

#define XPORT_MEM_BASE        		XPORT_REGION
#define XPORT_MEM_SIZE        		XPORT_SIZE

#define MAX_FILTER_NUM 			63

#define XPORT_CHL0_BASE_ADDR_DEF    	XPORT_MEM_BASE	// 119M

#ifndef CONFIG_XPORT_BIG_DMABUFFER
#define XPORT_CHL0_UNIT_SIZE_DEF    	400	// 400 bytes
#define XPORT_CHL0_UNIT_NUM_DEF     	320
#define XPORT_CHL0_CFG_DEF          	((XPORT_CHL0_UNIT_NUM_DEF<<8) | (XPORT_CHL0_UNIT_SIZE_DEF>>3))
#else
#define XPORT_CHL0_UNIT_SIZE_DEF    	2000	// 400 bytes
#define XPORT_CHL0_UNIT_NUM_DEF     	128
#define XPORT_CHL0_CFG_DEF          	((0x60000000) | (XPORT_CHL0_UNIT_NUM_DEF<<8) | (XPORT_CHL0_UNIT_SIZE_DEF>>3))
#endif

#define XPORT_CHL0_RP_DEF           	0
#define XPORT_CHL0_WP_DEF           	0

#define XPORT_CHL1_BASE_ADDR_DEF    	(XPORT_MEM_BASE+0x20000)
#define XPORT_CHL1_UNIT_SIZE_DEF    	400	// 400 bytes
#define XPORT_CHL1_UNIT_NUM_DEF     	320
#define XPORT_CHL1_CFG_DEF          	((XPORT_CHL1_UNIT_NUM_DEF<<8) | (XPORT_CHL1_UNIT_SIZE_DEF>>3))
#define XPORT_CHL1_RP_DEF           	0
#define XPORT_CHL1_WP_DEF           	0

#define MDA0_BUF0_BASE_ADDR             (XPORT_MEM_BASE+0x40000)
#define MDA0_BUF1_BASE_ADDR             (XPORT_MEM_BASE+0x50000)
#define DMA0_MAX_BLOCK_SIZE             2048    // 2k  bytes
#define DMA0_MAX_BLOCK_NUM              16      //

#define MDA1_BUF0_BASE_ADDR             (XPORT_MEM_BASE+0x60000)
#define MDA1_BUF1_BASE_ADDR             (XPORT_MEM_BASE+0x70000)
#define DMA1_MAX_BLOCK_SIZE             2048    // 2k  bytes
#define DMA1_MAX_BLOCK_NUM              16      //

#define MDA0_LIST0_HEAD_ADDR            (XPORT_MEM_BASE+0x80000)
#define MDA0_LIST1_HEAD_ADDR            (XPORT_MEM_BASE+0x80800)
#define MDA1_LIST0_HEAD_ADDR            (XPORT_MEM_BASE+0x81000)
#define MDA1_LIST1_HEAD_ADDR            (XPORT_MEM_BASE+0x81800)

#define XPORT_MIPS_BASE_ADDR		(XPORT_MEM_BASE+0x82000)
#define XPORT_MIPS_SIZE			0xE000

#define FILTER0_BUF_BASE_ADDR		(XPORT_MIPS_BASE_ADDR + XPORT_MIPS_SIZE) //(XPORT_MEM_BASE+0x90000)
#define FILTER0_BUF_SIZE          	0x20000 // 128k
#define FILTER1_BUF_BASE_ADDR     	(FILTER0_BUF_BASE_ADDR + FILTER0_BUF_SIZE)
#define FILTER1_BUF_SIZE          	0x20000	// 128k
#define FILTER2_BUF_BASE_ADDR     	(FILTER1_BUF_BASE_ADDR + FILTER1_BUF_SIZE)
#define FILTER2_BUF_SIZE          	0x20000	// 128k
#define FILTER3_BUF_BASE_ADDR     	(FILTER2_BUF_BASE_ADDR + FILTER2_BUF_SIZE)
#define FILTER3_BUF_SIZE          	0x20000	// 128k
#define FILTER4_BUF_BASE_ADDR     	(FILTER3_BUF_BASE_ADDR + FILTER3_BUF_SIZE)
#define FILTER4_BUF_SIZE          	0x20000	// 128k
#define FILTER5_BUF_BASE_ADDR     	(FILTER4_BUF_BASE_ADDR + FILTER4_BUF_SIZE)
#define FILTER5_BUF_SIZE          	0x20000	// 128k
#define FILTER6_BUF_BASE_ADDR     	(FILTER5_BUF_BASE_ADDR + FILTER5_BUF_SIZE)
#define FILTER6_BUF_SIZE          	0x20000	// 128k
#define FILTER7_BUF_BASE_ADDR     	(FILTER6_BUF_BASE_ADDR + FILTER6_BUF_SIZE)
#define FILTER7_BUF_SIZE          	0x20000	// 128k
#define FILTER8_BUF_BASE_ADDR     	(FILTER7_BUF_BASE_ADDR + FILTER7_BUF_SIZE)
#define FILTER8_BUF_SIZE          	0x20000	// 128k
#define FILTER9_BUF_BASE_ADDR     	(FILTER8_BUF_BASE_ADDR + FILTER8_BUF_SIZE)
#define FILTER9_BUF_SIZE          	0x20000	// 128k

#define FILTER10_BUF_BASE_ADDR    	(FILTER9_BUF_BASE_ADDR + FILTER9_BUF_SIZE)
#define FILTER10_BUF_SIZE         	0x10000	// 64k
#define FILTER11_BUF_BASE_ADDR    	(FILTER10_BUF_BASE_ADDR+FILTER10_BUF_SIZE)
#define FILTER11_BUF_SIZE         	0x10000	// 64k
#define FILTER12_BUF_BASE_ADDR    	(FILTER11_BUF_BASE_ADDR+FILTER11_BUF_SIZE)
#define FILTER12_BUF_SIZE         	0x10000	// 64k
#define FILTER13_BUF_BASE_ADDR    	(FILTER12_BUF_BASE_ADDR+FILTER12_BUF_SIZE)
#define FILTER13_BUF_SIZE         	0x10000	// 64k
#define FILTER14_BUF_BASE_ADDR    	(FILTER13_BUF_BASE_ADDR+FILTER13_BUF_SIZE)
#define FILTER14_BUF_SIZE         	0x10000	// 64k
#define FILTER15_BUF_BASE_ADDR    	(FILTER14_BUF_BASE_ADDR+FILTER14_BUF_SIZE)

#if !defined(CONFIG_ARCH_ORION_CSM1201_J) && !defined (CONFIG_ARCH_ORION_CSM1200_J)
#ifndef CONFIG_XPORT_BIG_DMABUFFER
#define FILTER15_BUF_SIZE         	0x190000 // 2048k
#else
#define FILTER15_BUF_SIZE         	0x10000 // 64k
#endif
#else // for csm1201_j
#define FILTER15_BUF_SIZE         	0x10000 // 64k
#endif //csm1201_j

#define FILTER16_BUF_BASE_ADDR    	(FILTER15_BUF_BASE_ADDR+FILTER15_BUF_SIZE + 0x1000)
#define FILTER16_BUF_SIZE         	0x10000	// 64k
#define FILTER17_BUF_BASE_ADDR    	(FILTER16_BUF_BASE_ADDR+FILTER16_BUF_SIZE)
#define FILTER17_BUF_SIZE         	0x10000	// 64k
#define FILTER18_BUF_BASE_ADDR    	(FILTER17_BUF_BASE_ADDR+FILTER17_BUF_SIZE)
#define FILTER18_BUF_SIZE         	0x10000	// 64k
#define FILTER19_BUF_BASE_ADDR    	(FILTER18_BUF_BASE_ADDR+FILTER18_BUF_SIZE)
#define FILTER19_BUF_SIZE         	0x10000	// 64k

#define FILTER20_BUF_BASE_ADDR    	(FILTER19_BUF_BASE_ADDR+FILTER19_BUF_SIZE)
#define FILTER20_BUF_SIZE         	0x10000	// 64k
#define FILTER21_BUF_BASE_ADDR    	(FILTER20_BUF_BASE_ADDR+FILTER20_BUF_SIZE)
#define FILTER21_BUF_SIZE         	0x10000	// 64k
#define FILTER22_BUF_BASE_ADDR    	(FILTER21_BUF_BASE_ADDR+FILTER21_BUF_SIZE)
#define FILTER22_BUF_SIZE         	0x10000	// 64k
#define FILTER23_BUF_BASE_ADDR    	(FILTER22_BUF_BASE_ADDR+FILTER22_BUF_SIZE)
#define FILTER23_BUF_SIZE         	0x10000	// 64k
#define FILTER24_BUF_BASE_ADDR    	(FILTER23_BUF_BASE_ADDR+FILTER23_BUF_SIZE)
#define FILTER24_BUF_SIZE         	0x10000	// 64k
#define FILTER25_BUF_BASE_ADDR    	(FILTER24_BUF_BASE_ADDR+FILTER24_BUF_SIZE)
#define FILTER25_BUF_SIZE         	0x10000	// 64k
#define FILTER26_BUF_BASE_ADDR    	(FILTER25_BUF_BASE_ADDR+FILTER25_BUF_SIZE)
#define FILTER26_BUF_SIZE         	0x10000	// 64k
#define FILTER27_BUF_BASE_ADDR    	(FILTER26_BUF_BASE_ADDR+FILTER26_BUF_SIZE)
#define FILTER27_BUF_SIZE         	0x10000	// 64k
#define FILTER28_BUF_BASE_ADDR    	(FILTER27_BUF_BASE_ADDR+FILTER27_BUF_SIZE)
#define FILTER28_BUF_SIZE         	0x10000	// 64k
#define FILTER29_BUF_BASE_ADDR    	(FILTER28_BUF_BASE_ADDR+FILTER28_BUF_SIZE)
#define FILTER29_BUF_SIZE         	0x10000	// 64k

#define FILTER30_BUF_BASE_ADDR    	(FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER30_BUF_SIZE         	0x10000	// 64k
#define FILTER31_BUF_BASE_ADDR    	(FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER31_BUF_SIZE         	0x10000 // 64k

#define FILTER32_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER32_BUF_SIZE               0x10000 // 64k
#define FILTER33_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER33_BUF_SIZE               0x10000 // 64k
#define FILTER34_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER34_BUF_SIZE               0x10000 // 64k
#define FILTER35_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER35_BUF_SIZE               0x10000 // 64k
#define FILTER36_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER36_BUF_SIZE               0x10000 // 64k
#define FILTER37_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER37_BUF_SIZE               0x10000 // 64k
#define FILTER38_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER38_BUF_SIZE               0x10000 // 64k
#define FILTER39_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER39_BUF_SIZE               0x10000 // 64k
#define FILTER40_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER40_BUF_SIZE               0x10000 // 64k
#define FILTER41_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER41_BUF_SIZE               0x10000 // 64k
#define FILTER42_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER42_BUF_SIZE               0x10000 // 64k
#define FILTER43_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER43_BUF_SIZE               0x10000 // 64k

#define FILTER44_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER44_BUF_SIZE               0x10000 // 64k
#define FILTER45_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER45_BUF_SIZE               0x10000 // 64k
#define FILTER46_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER46_BUF_SIZE               0x10000 // 64k
#define FILTER47_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER47_BUF_SIZE               0x10000 // 64k
#define FILTER48_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER48_BUF_SIZE               0x10000 // 64k
#define FILTER49_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER49_BUF_SIZE               0x10000 // 64k
#define FILTER50_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER50_BUF_SIZE               0x10000 // 64k
#define FILTER51_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER51_BUF_SIZE               0x10000 // 64k
#define FILTER52_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER52_BUF_SIZE               0x10000 // 64k
#define FILTER53_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER53_BUF_SIZE               0x10000 // 64k

#define FILTER54_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER54_BUF_SIZE               0x10000 // 64k
#define FILTER55_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER55_BUF_SIZE               0x10000 // 64k
#define FILTER56_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER56_BUF_SIZE               0x10000 // 64k
#define FILTER57_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER57_BUF_SIZE               0x10000 // 64k
#define FILTER58_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER58_BUF_SIZE               0x10000 // 64k
#define FILTER59_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER59_BUF_SIZE               0x10000 // 64k
#define FILTER60_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER60_BUF_SIZE               0x10000 // 64k
#define FILTER61_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER61_BUF_SIZE               0x10000 // 64k
#define FILTER62_BUF_BASE_ADDR          (FILTER29_BUF_BASE_ADDR+FILTER29_BUF_SIZE)
#define FILTER62_BUF_SIZE               0x10000 // 64k
#define FILTER63_BUF_BASE_ADDR          (FILTER30_BUF_BASE_ADDR+FILTER30_BUF_SIZE)
#define FILTER63_BUF_SIZE               0x10000 // 64k

// Driver Constant Defines
////////////////////////////////////////////////
#define XPORT_MAJOR          		100
#define XPORT_MINOR          		0
#define XPORT_MINOR_VID0     		10
#define XPORT_MINOR_VID1     		11
#define XPORT_MINOR_AUD0     		12
#define XPORT_MINOR_FT_BASE  		100

#define XPORT_CFG0_TUNER0_SER_MOD       0x00000001
#define XPORT_CFG0_TUNER1_SER_MOD       0x00000002
#define XPORT_CFG0_CHL0_DMA_EN          0x00000004
#define XPORT_CFG0_CHL1_DMA_EN          0x00000010
#define XPORT_CFG0_CHL0_ATSC_EN         0x00000100
#define XPORT_CFG0_CHL1_ATSC_EN         0x00000200

#define XPORT_CFG1_OUT_CHL0_LINE_SYNC   0x00000001
#define XPORT_CFG1_OUT_CHL1_LINE_SYNC   0x00000002
#define XPORT_CFG1_OUT_CHL2_LINE_SYNC   0x00000004
#define XPORT_CFG1_OUT_CHL3_LINE_SYNC   0x00000008

#define XPORT_TUNER0_EN                 0x00000001
#define XPORT_TUNER1_EN                 0x00000002

#define XPORT_IRQ0                	16
#define XPORT_IRQ1                	17

#define XPORT_IRQ0_ID                   100
#define XPORT_IRQ1_ID                   101

////////////////////////////////////////////////
// Driver Macro Defines
////////////////////////////////////////////////

extern void __iomem *xport_regs_base;
extern void __iomem *xport_mem_base;

#define __IS_HW_ADDR__(x) 	(((x)&0x7ff00000)==0x41400000)
#define __IS_MIPS_ADDR__(x) 	(((x)&0x7ff00000)==0x41500000)

#define __WR_FLAGS__(x)     	((x)>>31)
#define __OFFSET_ADDR__(x) 	((x)&0x000fffff)

void xport_writeb(int a,int v);
void xport_writew(int a,int v);
void xport_writel(int a,int v);
unsigned char xport_readb(int a);
unsigned short xport_readw(int a);
unsigned int xport_readl(int a);

////////////////////////////////////////////////
// Data Struct Defines
////////////////////////////////////////////////

typedef struct XPORT_DEV_t {
	unsigned int dev_minor;
	XPORT_FILTER_TYPE filter_type;
	wait_queue_head_t wait_queue;
	wait_queue_head_t *irq_wait_queue_ptr;
	spinlock_t spin_lock;
    
} XPORT_DEV;

typedef struct DMA_LIST_NODE_t {
	unsigned int buf_addr;
	unsigned int data_size;
	struct DMA_LIST_NODE_t *next_addr;
	unsigned int next_valid;
} DMA_LIST_NODE;

typedef struct DMA_LIST_t {
	DMA_LIST_NODE *head_node;
	unsigned int max_block_size;
	unsigned int max_block_num;
	unsigned int cur_index;
	unsigned int next_index;
	unsigned int locked_flag;
	unsigned int block_size_sum;
} DMA_LIST;

typedef struct XPORT_DMA_t {
	DMA_LIST dma_list[2];
	unsigned long next_jiffies;
	unsigned int cur_index;
	struct semaphore dma_sem;
} XPORT_DMA;

#define XPORT_FILTER_IOC_PID0           _IOW('k', 1, int)
#define XPORT_FILTER_IOC_PID1           _IOW('k', 2, int)
#define XPORT_FILTER_IOC_PID2           _IOW('k', 3, int)
#define XPORT_FILTER_IOC_PID3           _IOW('k', 4, int)
#define XPORT_FILTER_IOC_FILTER         _IOW('k', 5, int)
#define XPORT_FILTER_IOC_FILTER_COND    _IOW('k', 7, int)

#define XPORT_FILTER_IOC_TYPE           _IOW('k', 6, int)
#define XPORT_FILTER_IOC_ENABLE         _IOW('k', 8, int)

#define XPORT_FILTER_IOC_QUERY_NUM      _IOR('k', 10, int)
#define XPORT_FILTER_IOC_QUERY_SIZE     _IOR('k', 11, int)

#define XPORT_FILTER_IOC_CRC_ENABLE     _IOW('k', 12, int)

#define XPORT_PIDFT_IOC_ENABLE          _IOW('k', 13, int)
#define XPORT_PIDFT_IOC_CHANNEL         _IOW('k', 14, int)
#define XPORT_PIDFT_IOC_PIDVAL          _IOW('k', 15, int)
#define XPORT_PIDFT_IOC_DESC_ODDKEY     _IOW('k', 16, int)
#define XPORT_PIDFT_IOC_DESC_EVENKEY    _IOW('k', 17, int)
#define XPORT_PIDFT_IOC_DESC_ENABLE     _IOW('k', 18, int)

#define XPORT_CHL_IOC_ENABLE            _IOW('k', 19, int)
#define XPORT_CHL_IOC_INPUT_MODE        _IOW('k', 20, int)
#define XPORT_CHL_IOC_RESET             _IOW('k', 21, int)
#define XPORT_CHL_IOC_DMA_RESET         _IOW('k', 22, int)

#define XPORT_VID_IOC_OUTPUT_MODE       _IOW('k', 23, int)
#define XPORT_VID_IOC_RESET             _IOW('k', 24, int)
#define XPORT_VID_IOC_ENABLE            _IOW('k', 25, int)
#define XPORT_VID_IOC_PIDVAL            _IOW('k', 26, int)

#define XPORT_AUD_IOC_OUTPUT_MODE       _IOW('k', 27, int)
#define XPORT_AUD_IOC_RESET             _IOW('k', 28, int)
#define XPORT_AUD_IOC_ENABLE            _IOW('k', 29, int)
#define XPORT_AUD_IOC_PIDVAL            _IOW('k', 30, int)

#define XPORT_PCR_IOC_ENABLE            _IOW('k', 31, int)
#define XPORT_PCR_IOC_GETVAL            _IOW('k', 32, int)
#define XPORT_PCR_IOC_PIDVAL            _IOW('k', 33, int)

#define XPORT_FW_INIT			_IOW('k', 34, int)

#define XPORT_FILTER_IOC_CRC_NOTIFY_ENABLE _IOW('k', 35, int)
#define XPORT_FILTER_IOC_SAVE_ENABLE	 _IOW('k', 36, int)

#define XPORT_FILTER_IOC_PID4           _IOW('k', 37, int)
#define XPORT_FILTER_IOC_PID5           _IOW('k', 38, int)
#define XPORT_FILTER_IOC_PID6           _IOW('k', 39, int)
#define XPORT_FILTER_IOC_PID7           _IOW('k', 40, int)
#define XPORT_FILTER_IOC_PID8           _IOW('k', 41, int)
#define XPORT_FILTER_IOC_PID9           _IOW('k', 42, int)
#define XPORT_FILTER_IOC_PID10          _IOW('k', 43, int)
#define XPORT_FILTER_IOC_PID11          _IOW('k', 44, int)

#define XPORT_CHL_IOC_CLEAR		        _IOW('k', 45, int)
#define XPORT_CHL_IOC_TUNER_MODE	    _IOW('k', 46, int)

#define XPORT_VID_IOC_SWITCH		    _IOW('k', 47, int)
#define XPORT_AUD_IOC_SWITCH		    _IOW('k', 48, int)
#define XPORT_FILTER_IOC_SWITCH		    _IOW('k', 49, int)
#define XPORT_CHL_IOC_DES_MODE          _IOW('k', 50, int)


#define XPORT_IOC_MEM_BASE_PHYADDR      _IOR('k', 51, int)
#define XPORT_IOC_MEM_SIZE              _IOR('k', 52, int)
#define XPORT_FILTER_IOC_BUFFER_UPADDR  _IOR('k', 53, int)
#define XPORT_FILTER_IOC_BUFFER_LOWADDR _IOR('k', 54, int)
#define XPORT_FILTER_IOC_BUFFER_RP_ADDR _IOR('k', 55, int)
#define XPORT_FILTER_IOC_BUFFER_WP_ADDR _IOR('k', 56, int)

#define XPORT_CHL_IOC_UNIT_NUM_DEF      _IOR('k', 57, int)
#define XPORT_CHL_IOC_UNIT_SIZE_DEF     _IOR('k', 58, int)
#define XPORT_CHL_IOC_MIN_SPACES        _IOR('k', 59, int)
       
#endif



