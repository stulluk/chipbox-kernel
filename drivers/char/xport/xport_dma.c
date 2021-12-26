
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/semaphore.h>

#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include "xport_dma.h"
#include "xport_drv.h"

#define  TS_SIZE      	188
#define  ML_ALIGN(x)   	((x)<<24 | (x)>>24 | ((x)>>8 & 0xff00) | ((x)<<8 & 0xff0000))

static XPORT_DMA xport_dma[2];

void xport_dma_init(void)
{
	unsigned int i;
	unsigned int tmp_buf_addr;
	unsigned int tmp_list_addr;

	xport_dma[0].dma_list[0].head_node = xport_mem_base + (MDA0_LIST0_HEAD_ADDR - XPORT_MEM_BASE);
	xport_dma[0].dma_list[0].max_block_size = DMA0_MAX_BLOCK_SIZE;
	xport_dma[0].dma_list[0].max_block_num = DMA0_MAX_BLOCK_NUM;
	xport_dma[0].dma_list[0].cur_index = 0;
	xport_dma[0].dma_list[0].next_index = 0;
	xport_dma[0].dma_list[0].locked_flag = 0;

	tmp_buf_addr = MDA0_BUF0_BASE_ADDR;
	tmp_list_addr = MDA0_LIST0_HEAD_ADDR;

	for (i = 0; i < DMA0_MAX_BLOCK_NUM; i++) {
		tmp_list_addr += sizeof(DMA_LIST_NODE);

		xport_dma[0].dma_list[0].head_node[i].buf_addr = ML_ALIGN(tmp_buf_addr);
		xport_dma[0].dma_list[0].head_node[i].data_size = 0;
		xport_dma[0].dma_list[0].head_node[i].next_addr = (DMA_LIST_NODE *) (ML_ALIGN(tmp_list_addr));
		xport_dma[0].dma_list[0].head_node[i].next_valid = 0;

		tmp_buf_addr += DMA0_MAX_BLOCK_SIZE;
	}

	xport_dma[0].dma_list[1].head_node = xport_mem_base + (MDA0_LIST1_HEAD_ADDR - XPORT_MEM_BASE);
	xport_dma[0].dma_list[1].max_block_size = DMA0_MAX_BLOCK_SIZE;
	xport_dma[0].dma_list[1].max_block_num = DMA0_MAX_BLOCK_NUM;
	xport_dma[0].dma_list[1].cur_index = 0;
	xport_dma[0].dma_list[1].next_index = 0;
	xport_dma[0].dma_list[1].locked_flag = 0;

	tmp_buf_addr = MDA0_BUF1_BASE_ADDR;
	tmp_list_addr = MDA0_LIST1_HEAD_ADDR;

	for (i = 0; i < DMA0_MAX_BLOCK_NUM; i++) {
		tmp_list_addr += sizeof(DMA_LIST_NODE);

		xport_dma[0].dma_list[1].head_node[i].buf_addr = ML_ALIGN(tmp_buf_addr);
		xport_dma[0].dma_list[1].head_node[i].data_size = 0;
		xport_dma[0].dma_list[1].head_node[i].next_addr = (DMA_LIST_NODE *) (ML_ALIGN(tmp_list_addr));
		xport_dma[0].dma_list[1].head_node[i].next_valid = 0;

		tmp_buf_addr += DMA0_MAX_BLOCK_SIZE;
	}

	xport_dma[1].dma_list[0].head_node = xport_mem_base + (MDA1_LIST0_HEAD_ADDR - XPORT_MEM_BASE);
	xport_dma[1].dma_list[0].max_block_size = DMA1_MAX_BLOCK_SIZE;
	xport_dma[1].dma_list[0].max_block_num = DMA1_MAX_BLOCK_NUM;
	xport_dma[1].dma_list[0].cur_index = 0;
	xport_dma[1].dma_list[0].next_index = 0;
	xport_dma[1].dma_list[0].locked_flag = 0;

	tmp_buf_addr = MDA1_BUF0_BASE_ADDR;
	tmp_list_addr = MDA1_LIST0_HEAD_ADDR;

	for (i = 0; i < DMA1_MAX_BLOCK_NUM; i++) {
		tmp_list_addr += sizeof(DMA_LIST_NODE);

		xport_dma[1].dma_list[0].head_node[i].buf_addr = ML_ALIGN(tmp_buf_addr);
		xport_dma[1].dma_list[0].head_node[i].data_size = 0;
		xport_dma[1].dma_list[0].head_node[i].next_addr = (DMA_LIST_NODE *) (ML_ALIGN(tmp_list_addr));
		xport_dma[1].dma_list[0].head_node[i].next_valid = 0;

		tmp_buf_addr += DMA1_MAX_BLOCK_SIZE;
	}

	xport_dma[1].dma_list[1].head_node = xport_mem_base + (MDA1_LIST1_HEAD_ADDR - XPORT_MEM_BASE);
	xport_dma[1].dma_list[1].max_block_size = DMA1_MAX_BLOCK_SIZE;
	xport_dma[1].dma_list[1].max_block_num = DMA1_MAX_BLOCK_NUM;
	xport_dma[1].dma_list[1].cur_index = 0;
	xport_dma[1].dma_list[1].next_index = 0;
	xport_dma[1].dma_list[1].locked_flag = 0;

	tmp_buf_addr = MDA1_BUF1_BASE_ADDR;
	tmp_list_addr = MDA1_LIST1_HEAD_ADDR;

	for (i = 0; i < DMA1_MAX_BLOCK_NUM; i++) {
		tmp_list_addr += sizeof(DMA_LIST_NODE);

		xport_dma[1].dma_list[1].head_node[i].buf_addr = ML_ALIGN(tmp_buf_addr);
		xport_dma[1].dma_list[1].head_node[i].data_size = 0;
		xport_dma[1].dma_list[1].head_node[i].next_addr = (DMA_LIST_NODE *) (ML_ALIGN(tmp_list_addr));
		xport_dma[1].dma_list[1].head_node[i].next_valid = 0;

		tmp_buf_addr += DMA1_MAX_BLOCK_SIZE;
	}

	init_MUTEX(&(xport_dma[0].dma_sem));
	init_MUTEX(&(xport_dma[1].dma_sem));

	xport_dma[0].cur_index = 0;
	xport_dma[1].cur_index = 0;

	xport_dma[0].next_jiffies = 0;
	xport_dma[1].next_jiffies = 0;

	return;
}

int xport_dma_set(unsigned int dma_id)
{
	int rt_val = -EFAULT;

	unsigned int regs_val = 0;
	unsigned int cur_delay_clk27;

	DMA_LIST *dma_cur_list_ptr = NULL;
	DMA_LIST *dma_done_list_ptr = NULL;

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return rt_val;

	dma_cur_list_ptr = &(xport_dma[dma_id].dma_list[xport_dma[dma_id].cur_index]);
	dma_done_list_ptr = &(xport_dma[dma_id].dma_list[1 - xport_dma[dma_id].cur_index]);

	regs_val = xport_readl((DMA_INPUT0_HEAD_ADDR + (dma_id << 2)));

	if (regs_val == 0 && dma_cur_list_ptr->next_index > 0) {
		dma_cur_list_ptr->head_node[dma_cur_list_ptr->cur_index].next_valid = 0;

		if (dma_id == 0) {
			xport_writel(XPORT_INT_CLS_ADDR0, XPORT_IRQ0_DMA0_MSK);

			if (xport_dma[dma_id].cur_index == 0) {
				xport_writel(DMA_INPUT0_HEAD_ADDR, MDA0_LIST0_HEAD_ADDR);
			}
			else {
				xport_writel(DMA_INPUT0_HEAD_ADDR, MDA0_LIST1_HEAD_ADDR);
			}
		}
		else {
			xport_writel(XPORT_INT_CLS_ADDR0, XPORT_IRQ0_DMA1_MSK);

			if (xport_dma[dma_id].cur_index == 0) {
				xport_writel(DMA_INPUT1_HEAD_ADDR, MDA1_LIST0_HEAD_ADDR);
			}
			else {
				xport_writel(DMA_INPUT1_HEAD_ADDR, MDA1_LIST1_HEAD_ADDR);
			}
		}

		/* software bitrate control */
		cur_delay_clk27 = ((dma_cur_list_ptr->block_size_sum * (CLK27_MHZ << 3)) / MAX_DMA_MBPS);

		if (dma_id)
			xport_writel(MAIL_BOX7_ADDR, cur_delay_clk27);
		else
			xport_writel(MAIL_BOX6_ADDR, cur_delay_clk27);
		/* end of SW bitrate control */

		dma_done_list_ptr->cur_index = 0;
		dma_done_list_ptr->next_index = 0;
		dma_done_list_ptr->block_size_sum = 0;

		xport_dma[dma_id].cur_index = 1 - xport_dma[dma_id].cur_index;

		rt_val = 0;
	}

	up(&(xport_dma[dma_id].dma_sem));

	return rt_val;
}

/* xunli: for direct dma mode */
int xport_dma_direct_write(const char __user * buffer, size_t len, unsigned int dma_id)
{
	ssize_t rt_val = -EFAULT;

	void __iomem *write_addr;
	void __iomem *base_addr;

	unsigned int tmp_addr = 0;
	unsigned int cfg_addr, wp_addr, rd_addr, baseaddr_addr;

	unsigned int chl_buf_unit_num, chl_buf_unit_size, chl_buf_type;
	unsigned int free_block_num, chl_buf_wp, chl_buf_rp;
	unsigned int regs_val = 0;
	unsigned int write_len;

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return -EFAULT;

	if (dma_id) {
		cfg_addr = XPORT_CHL1_CFG_ADDR;
		wp_addr = XPORT_CHL_DMA1_WP_ADDR;
		rd_addr = XPORT_CHL1_RP_ADDR;
		baseaddr_addr = XPORT_CHL1_BASE_ADDR;
	}
	else {
		cfg_addr = XPORT_CHL0_CFG_ADDR;
		wp_addr = XPORT_CHL_DMA0_WP_ADDR;
		rd_addr = XPORT_CHL0_RP_ADDR;
		baseaddr_addr = XPORT_CHL0_BASE_ADDR;
	}

	regs_val = xport_readl(cfg_addr);
	chl_buf_type = ((regs_val >> 29) & 0x3);
	chl_buf_unit_num = (regs_val >> 8) & 0xfff;
	chl_buf_unit_size = (regs_val & 0xff) << 3;

	tmp_addr = xport_readl(baseaddr_addr) << 3;
	if ((chl_buf_type != 3) || ((tmp_addr != XPORT_CHL0_BASE_ADDR_DEF) && (tmp_addr != XPORT_CHL1_BASE_ADDR_DEF))) {
		up(&(xport_dma[dma_id].dma_sem));
		return -EFAULT;
	}

	tmp_addr -= XPORT_MEM_BASE;
	base_addr = xport_mem_base + tmp_addr;

	chl_buf_wp = xport_readl(wp_addr);
	chl_buf_rp = xport_readl(rd_addr);

	if ((chl_buf_wp ^ chl_buf_rp) >> 31)
		free_block_num = (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);
	else
		free_block_num = chl_buf_unit_num + (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);

	rt_val = 0;

	while (((int) len >= TS_SIZE) && ((int) free_block_num > 0) && (free_block_num <= chl_buf_unit_num)) {

		write_addr = base_addr + ((chl_buf_wp & 0xfff) * chl_buf_unit_size);
		write_len = 0;

		while ((len >= TS_SIZE) && (write_len + TS_SIZE + 8 <= chl_buf_unit_size)) {
			write_len += TS_SIZE;
			len -= TS_SIZE;
		}

		if (write_len == 0) {
			up(&(xport_dma[dma_id].dma_sem));
			return -EFAULT;
		}

		writel(0, write_addr);
		write_addr += 4;
		writel(ML_ALIGN(write_len), write_addr);
		write_addr += 4;

		copy_from_user(write_addr, buffer, write_len);

		buffer += write_len;
		rt_val += write_len;

		/* update wp pointer */
		chl_buf_wp++;
		if ((chl_buf_wp & 0xfff) >= chl_buf_unit_num)
			chl_buf_wp = (~chl_buf_wp) & 0x80000000;

		free_block_num--;
	}

	xport_writel(wp_addr, chl_buf_wp);

	up(&(xport_dma[dma_id].dma_sem));

	return rt_val;
}

int xport_dma_write(const char __user * buffer, size_t len, unsigned int dma_id)
{
	ssize_t rt_val = -EFAULT;

	void __iomem *write_addr;
	DMA_LIST *dma_list_ptr = NULL;

	unsigned int tmp_addr = 0;
	unsigned int regs_val = 0;
	unsigned int hw_set_req = 0;

      START_LAB:
	hw_set_req = 0;

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return -EFAULT;

	dma_list_ptr = &(xport_dma[dma_id].dma_list[xport_dma[dma_id].cur_index]);

	if (len > dma_list_ptr->max_block_size) {
		up(&(xport_dma[dma_id].dma_sem));
		return -EFAULT;
	}

	if (dma_list_ptr->next_index < dma_list_ptr->max_block_num) {

		tmp_addr = ML_ALIGN(dma_list_ptr->head_node[dma_list_ptr->next_index].buf_addr);
		tmp_addr -= XPORT_MEM_BASE;
		write_addr = xport_mem_base + tmp_addr;

		copy_from_user(write_addr, buffer, len);

		dma_list_ptr->head_node[dma_list_ptr->next_index].data_size = ML_ALIGN(len);
		dma_list_ptr->head_node[dma_list_ptr->next_index].next_valid = 0;

		if (dma_list_ptr->next_index != 0) {
			dma_list_ptr->head_node[dma_list_ptr->cur_index].next_valid = 0x01000000;	//
			dma_list_ptr->cur_index++;
		}

		dma_list_ptr->next_index++;
		dma_list_ptr->block_size_sum += len;

		rt_val = len;
	}
	else {
		rt_val = 0;
		regs_val = xport_readl((DMA_INPUT0_HEAD_ADDR + (dma_id << 2)));
		if (regs_val == 0)
			hw_set_req = 1;
	}

	up(&(xport_dma[dma_id].dma_sem));

	if (hw_set_req) {
		xport_dma_set(dma_id);
		goto START_LAB;
	}

	return rt_val;
}

int xport_dma_reset(unsigned int dma_id)
{

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return -EFAULT;

	xport_dma[dma_id].cur_index = 0;
	xport_dma[dma_id].next_jiffies = 0;
	xport_dma[dma_id].dma_list[0].cur_index = 0;
	xport_dma[dma_id].dma_list[0].next_index = 0;
	xport_dma[dma_id].dma_list[0].locked_flag = 0;
	xport_dma[dma_id].dma_list[1].cur_index = 0;
	xport_dma[dma_id].dma_list[1].next_index = 0;
	xport_dma[dma_id].dma_list[1].locked_flag = 0;

	up(&(xport_dma[dma_id].dma_sem));

	return 0;
}

int xport_dma_input_check(unsigned int dma_id)
{
	int rt_val = -EFAULT;
	DMA_LIST *dma_list_ptr = NULL;

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return rt_val;

	dma_list_ptr = &(xport_dma[dma_id].dma_list[xport_dma[dma_id].cur_index]);
	if (dma_list_ptr->next_index < dma_list_ptr->max_block_num) {
		rt_val = 0;
	}

	up(&(xport_dma[dma_id].dma_sem));

	return rt_val;
}

int xport_dma_half_empty_check(unsigned int dma_id)
{
	int rt_val = -EFAULT;

	unsigned int chl_wp, chl_rp, chl_wp_addr, chl_rp_addr;
	unsigned int space_cnt = 0;
	unsigned int tmp;

	if (down_interruptible(&(xport_dma[dma_id].dma_sem)))
		return -EFAULT;

	if (dma_id == 0) {
		chl_wp_addr = XPORT_CHL0_WP_ADDR;
		chl_rp_addr = XPORT_CHL0_RP_ADDR;
		tmp = xport_readl(XPORT_CHL0_CFG_ADDR);
		tmp = (tmp >> 29) & 0x3;

		if (tmp == 3)
			chl_wp_addr = XPORT_CHL_DMA0_WP_ADDR;

		chl_wp = xport_readl(chl_wp_addr);
		chl_rp = xport_readl(chl_rp_addr);

		if ((chl_wp ^ chl_rp) >> 31)
			space_cnt = (chl_rp & 0xfff) - (chl_wp & 0xfff);
		else
			space_cnt = XPORT_CHL0_UNIT_NUM_DEF + (chl_rp & 0xfff) - (chl_wp & 0xfff);

		if (space_cnt > XPORT_CHL0_MIN_SPACES)
			rt_val = 0;
	}
	else if (dma_id == 1) {
		chl_wp_addr = XPORT_CHL1_WP_ADDR;
		chl_rp_addr = XPORT_CHL1_RP_ADDR;
		tmp = xport_readl(XPORT_CHL1_CFG_ADDR);
		tmp = (tmp >> 29) & 0x3;

		if (tmp == 3)
			chl_wp_addr = XPORT_CHL_DMA1_WP_ADDR;

		chl_wp = xport_readl(chl_wp_addr);
		chl_rp = xport_readl(chl_rp_addr);

		if ((chl_wp ^ chl_rp) >> 31)
			space_cnt = (chl_rp & 0xfff) - (chl_wp & 0xfff);
		else
			space_cnt = XPORT_CHL1_UNIT_NUM_DEF + (chl_rp & 0xfff) - (chl_wp & 0xfff);

		if (space_cnt > XPORT_CHL1_MIN_SPACES)
			rt_val = 0;
	}

	up(&(xport_dma[dma_id].dma_sem));

	return rt_val;
}
