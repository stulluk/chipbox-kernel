/*
 *  Copyright (C) 2007 Celestial Corporation
 *
 *  Authors:
 *  yao.chen  <yao.chen@celestialsemi.com>
 */

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

#include "xport_drv.h"
#include "xport_filter.h"
#include "xport_mips.h"

#define  FILTER_EN_ADDR(x)                   (0x0400+((x)<<8))
#define  FILTER_FILTER0_ADDR(x)              (0x0401+((x)<<8))
#define  FILTER_FILTER1_ADDR(x)              (0x0402+((x)<<8))
#define  FILTER_FILTER2_ADDR(x)              (0x0403+((x)<<8))
#define  FILTER_MASK0_ADDR(x)                (0x0404+((x)<<8))
#define  FILTER_MASK1_ADDR(x)                (0x0405+((x)<<8))
#define  FILTER_MASK2_ADDR(x)		     (0x0406+((x)<<8))
#define  FILTER_BITWISE0_ADDR(x)	     (0x0439+((x)<<8))
#define  FILTER_BITWISE1_ADDR(x)	     (0x043a+((x)<<8))
#define  FILTER_BITWISE2_ADDR(x)             (0x043b+((x)<<8))
#define  FILTER_BUF_LOW_ADDR(x)              (0x0407+((x)<<8))
#define  FILTER_BUF_UP_ADDR(x)               (0x0408+((x)<<8))

#define  FILTER_TC0_ERR_CNT_ADDR(x)          (0x040a+((x)<<8))
#define  FILTER_TC1_ERR_CNT_ADDR(x)          (0x040b+((x)<<8))
#define  FILTER_TC2_ERR_CNT_ADDR(x)          (0x040c+((x)<<8))
#define  FILTER_TC3_ERR_CNT_ADDR(x)          (0x040d+((x)<<8))
#define  FILTER_ERR_PACKET_CNT_ADDR(x)       ((0x040e)+((x)<<8))
#define  FILTER_OUT_PACKET_CNT_ADDR(x)       (0x040f+((x)<<8))

#define  FILTER_PID0_ADDR(x)                 (0x0416+((x)<<8))
#define  FILTER_PID1_ADDR(x)                 (0x0417+((x)<<8))
#define  FILTER_PID2_ADDR(x)                 (0x0418+((x)<<8))
#define  FILTER_PID3_ADDR(x)                 (0x0419+((x)<<8))
#define  FILTER_PID4_ADDR(x)                 (0x041a+((x)<<8))
#define  FILTER_PID5_ADDR(x)                 (0x041b+((x)<<8))
#define  FILTER_PID6_ADDR(x)                 (0x041c+((x)<<8))
#define  FILTER_PID7_ADDR(x)                 (0x041d+((x)<<8))
#define  FILTER_PID8_ADDR(x)                 (0x041e +((x)<<8))   //(0x041e+((x)<<8))
#define  FILTER_PID9_ADDR(x)                 (0x041f+((x)<<8))
#define  FILTER_PID10_ADDR(x)                (0x0420+((x)<<8))
#define  FILTER_PID11_ADDR(x)                (0x0421+((x)<<8))

#define  FILTER_DISABLE_ADDR(x)		     (0x0436+((x)<<8))

#define  FILTER_BUF_WP_ADDR(x)               (0x41400000+(0x0110<<2)+((x)<<3))
#define  FILTER_BUF_RP_ADDR(x)               (0x41400000+(0x0111<<2)+((x)<<3))

inline unsigned int FILTER_BUF_LOW(unsigned int filter_index)
{
	switch (filter_index) {
	case 0:
		return FILTER0_BUF_BASE_ADDR;
	case 1:
		return FILTER1_BUF_BASE_ADDR;
	case 2:
		return FILTER2_BUF_BASE_ADDR;
	case 3:
		return FILTER3_BUF_BASE_ADDR;
	case 4:
		return FILTER4_BUF_BASE_ADDR;
	case 5:
		return FILTER5_BUF_BASE_ADDR;
	case 6:
		return FILTER6_BUF_BASE_ADDR;
	case 7:
		return FILTER7_BUF_BASE_ADDR;
	case 8:
		return FILTER8_BUF_BASE_ADDR;
	case 9:
		return FILTER9_BUF_BASE_ADDR;
	case 10:
		return FILTER10_BUF_BASE_ADDR;
	case 11:
		return FILTER11_BUF_BASE_ADDR;
	case 12:
		return FILTER12_BUF_BASE_ADDR;
	case 13:
		return FILTER13_BUF_BASE_ADDR;
	case 14:
		return FILTER14_BUF_BASE_ADDR;
	case 15:
		return FILTER15_BUF_BASE_ADDR;
	case 16:
		return FILTER16_BUF_BASE_ADDR;
	case 17:
		return FILTER17_BUF_BASE_ADDR;
	case 18:
		return FILTER18_BUF_BASE_ADDR;
	case 19:
		return FILTER19_BUF_BASE_ADDR;
	case 20:
		return FILTER20_BUF_BASE_ADDR;
	case 21:
		return FILTER21_BUF_BASE_ADDR;
	case 22:
		return FILTER22_BUF_BASE_ADDR;
	case 23:
		return FILTER23_BUF_BASE_ADDR;
	case 24:
		return FILTER24_BUF_BASE_ADDR;
	case 25:
		return FILTER25_BUF_BASE_ADDR;
	case 26:
		return FILTER26_BUF_BASE_ADDR;
	case 27:
		return FILTER27_BUF_BASE_ADDR;
	case 28:
		return FILTER28_BUF_BASE_ADDR;
	case 29:
		return FILTER29_BUF_BASE_ADDR;
	case 30:
		return FILTER30_BUF_BASE_ADDR;
	case 31:
		return FILTER31_BUF_BASE_ADDR;
        case 32:
                return FILTER32_BUF_BASE_ADDR;
        case 33:
                return FILTER33_BUF_BASE_ADDR;
        case 34:
                return FILTER34_BUF_BASE_ADDR;
        case 35:
                return FILTER35_BUF_BASE_ADDR;
        case 36:
                return FILTER36_BUF_BASE_ADDR;
        case 37:
                return FILTER37_BUF_BASE_ADDR;
        case 38:
                return FILTER38_BUF_BASE_ADDR;
        case 39:
                return FILTER39_BUF_BASE_ADDR;
        case 40:
                return FILTER40_BUF_BASE_ADDR;
        case 41:
                return FILTER41_BUF_BASE_ADDR;
        case 42:
                return FILTER42_BUF_BASE_ADDR;
        case 43:
                return FILTER43_BUF_BASE_ADDR;
        case 44:
                return FILTER44_BUF_BASE_ADDR;
        case 45:
                return FILTER45_BUF_BASE_ADDR;
        case 46:
                return FILTER46_BUF_BASE_ADDR;
        case 47:
                return FILTER47_BUF_BASE_ADDR;
        case 48:
                return FILTER48_BUF_BASE_ADDR;
        case 49:
                return FILTER49_BUF_BASE_ADDR;
        case 50:
                return FILTER50_BUF_BASE_ADDR;
        case 51:
                return FILTER51_BUF_BASE_ADDR;
        case 52:
                return FILTER52_BUF_BASE_ADDR;
        case 53:
                return FILTER53_BUF_BASE_ADDR;
        case 54:
                return FILTER54_BUF_BASE_ADDR;
        case 55:
                return FILTER55_BUF_BASE_ADDR;
        case 56:
                return FILTER56_BUF_BASE_ADDR;
        case 57:
                return FILTER57_BUF_BASE_ADDR;
        case 58:
                return FILTER58_BUF_BASE_ADDR;
        case 59:
                return FILTER59_BUF_BASE_ADDR;
        case 60:
                return FILTER60_BUF_BASE_ADDR;
        case 61:
                return FILTER61_BUF_BASE_ADDR;
        case 62:
                return FILTER62_BUF_BASE_ADDR;
        case 63:
                return FILTER63_BUF_BASE_ADDR;

	}

	return -1;
}
inline unsigned int FILTER_BUF_UP(unsigned int filter_index)
{
	switch (filter_index) {
	case 0:
		return FILTER0_BUF_BASE_ADDR + FILTER0_BUF_SIZE;
	case 1:
		return FILTER1_BUF_BASE_ADDR + FILTER1_BUF_SIZE;
	case 2:
		return FILTER2_BUF_BASE_ADDR + FILTER2_BUF_SIZE;
	case 3:
		return FILTER3_BUF_BASE_ADDR + FILTER3_BUF_SIZE;
	case 4:
		return FILTER4_BUF_BASE_ADDR + FILTER4_BUF_SIZE;
	case 5:
		return FILTER5_BUF_BASE_ADDR + FILTER5_BUF_SIZE;
	case 6:
		return FILTER6_BUF_BASE_ADDR + FILTER6_BUF_SIZE;
	case 7:
		return FILTER7_BUF_BASE_ADDR + FILTER7_BUF_SIZE;
	case 8:
		return FILTER8_BUF_BASE_ADDR + FILTER8_BUF_SIZE;
	case 9:
		return FILTER9_BUF_BASE_ADDR + FILTER9_BUF_SIZE;
	case 10:
		return FILTER10_BUF_BASE_ADDR + FILTER10_BUF_SIZE;
	case 11:
		return FILTER11_BUF_BASE_ADDR + FILTER11_BUF_SIZE;
	case 12:
		return FILTER12_BUF_BASE_ADDR + FILTER12_BUF_SIZE;
	case 13:
		return FILTER13_BUF_BASE_ADDR + FILTER13_BUF_SIZE;
	case 14:
		return FILTER14_BUF_BASE_ADDR + FILTER14_BUF_SIZE;
	case 15:
		return FILTER15_BUF_BASE_ADDR + FILTER15_BUF_SIZE;
	case 16:
		return FILTER16_BUF_BASE_ADDR + FILTER16_BUF_SIZE;
	case 17:
		return FILTER17_BUF_BASE_ADDR + FILTER17_BUF_SIZE;
	case 18:
		return FILTER18_BUF_BASE_ADDR + FILTER18_BUF_SIZE;
	case 19:
		return FILTER19_BUF_BASE_ADDR + FILTER19_BUF_SIZE;
	case 20:
		return FILTER20_BUF_BASE_ADDR + FILTER20_BUF_SIZE;
	case 21:
		return FILTER21_BUF_BASE_ADDR + FILTER21_BUF_SIZE;
	case 22:
		return FILTER22_BUF_BASE_ADDR + FILTER22_BUF_SIZE;
	case 23:
		return FILTER23_BUF_BASE_ADDR + FILTER23_BUF_SIZE;
	case 24:
		return FILTER24_BUF_BASE_ADDR + FILTER24_BUF_SIZE;
	case 25:
		return FILTER25_BUF_BASE_ADDR + FILTER25_BUF_SIZE;
	case 26:
		return FILTER26_BUF_BASE_ADDR + FILTER26_BUF_SIZE;
	case 27:
		return FILTER27_BUF_BASE_ADDR + FILTER27_BUF_SIZE;
	case 28:
		return FILTER28_BUF_BASE_ADDR + FILTER28_BUF_SIZE;
	case 29:
		return FILTER29_BUF_BASE_ADDR + FILTER29_BUF_SIZE;
	case 30:
		return FILTER30_BUF_BASE_ADDR + FILTER30_BUF_SIZE;
	case 31:
		return FILTER31_BUF_BASE_ADDR + FILTER31_BUF_SIZE;
        case 32:
                return FILTER32_BUF_BASE_ADDR + FILTER32_BUF_SIZE;
        case 33:
                return FILTER33_BUF_BASE_ADDR + FILTER33_BUF_SIZE;
        case 34:
                return FILTER34_BUF_BASE_ADDR + FILTER34_BUF_SIZE;
        case 35:
                return FILTER35_BUF_BASE_ADDR + FILTER35_BUF_SIZE;
        case 36:
                return FILTER36_BUF_BASE_ADDR + FILTER36_BUF_SIZE;
        case 37:
                return FILTER37_BUF_BASE_ADDR + FILTER37_BUF_SIZE;
        case 38:
                return FILTER38_BUF_BASE_ADDR + FILTER38_BUF_SIZE;
        case 39:
                return FILTER39_BUF_BASE_ADDR + FILTER39_BUF_SIZE;
        case 40:
                return FILTER40_BUF_BASE_ADDR + FILTER40_BUF_SIZE;
        case 41:
                return FILTER41_BUF_BASE_ADDR + FILTER41_BUF_SIZE;
        case 42:
                return FILTER42_BUF_BASE_ADDR + FILTER42_BUF_SIZE;
        case 43:
                return FILTER43_BUF_BASE_ADDR + FILTER43_BUF_SIZE;
        case 44:
                return FILTER44_BUF_BASE_ADDR + FILTER44_BUF_SIZE;
        case 45:
                return FILTER45_BUF_BASE_ADDR + FILTER45_BUF_SIZE;
        case 46:
                return FILTER46_BUF_BASE_ADDR + FILTER46_BUF_SIZE;
        case 47:
                return FILTER47_BUF_BASE_ADDR + FILTER47_BUF_SIZE;
        case 48:
                return FILTER48_BUF_BASE_ADDR + FILTER48_BUF_SIZE;
        case 49:
                return FILTER49_BUF_BASE_ADDR + FILTER49_BUF_SIZE;
        case 50:
                return FILTER50_BUF_BASE_ADDR + FILTER50_BUF_SIZE;
        case 51:
                return FILTER51_BUF_BASE_ADDR + FILTER51_BUF_SIZE;
        case 52:
                return FILTER52_BUF_BASE_ADDR + FILTER52_BUF_SIZE;
        case 53:
                return FILTER53_BUF_BASE_ADDR + FILTER53_BUF_SIZE;
        case 54:
                return FILTER54_BUF_BASE_ADDR + FILTER54_BUF_SIZE;
        case 55:
                return FILTER55_BUF_BASE_ADDR + FILTER55_BUF_SIZE;
        case 56:
                return FILTER56_BUF_BASE_ADDR + FILTER56_BUF_SIZE;
        case 57:
                return FILTER57_BUF_BASE_ADDR + FILTER57_BUF_SIZE;
        case 58:
                return FILTER58_BUF_BASE_ADDR + FILTER58_BUF_SIZE;
        case 59:
                return FILTER59_BUF_BASE_ADDR + FILTER59_BUF_SIZE;
        case 60:
                return FILTER60_BUF_BASE_ADDR + FILTER60_BUF_SIZE;
        case 61:
                return FILTER61_BUF_BASE_ADDR + FILTER61_BUF_SIZE;
        case 62:
                return FILTER62_BUF_BASE_ADDR + FILTER62_BUF_SIZE;
        case 63:
                return FILTER63_BUF_BASE_ADDR + FILTER63_BUF_SIZE;

	}

	return -1;
}

inline unsigned int xport_filter_wp(unsigned int filter_index)
{
	return xport_readl(FILTER_BUF_WP_ADDR(filter_index));
}

inline unsigned int xport_filter_rp(unsigned int filter_index)
{
	return xport_readl(FILTER_BUF_RP_ADDR(filter_index));
}


int xport_filter_reset(unsigned int filter_index)
{
#if 1    
	xport_mips_write(FILTER_DISABLE_ADDR(filter_index), 0);
#else	
	xport_mips_write(FILTER_EN_ADDR(filter_index), 0);

	xport_mips_write(FILTER_PID0_ADDR(filter_index), 0x1fff);
	xport_mips_write(FILTER_PID1_ADDR(filter_index), 0x1fff);
	xport_mips_write(FILTER_PID2_ADDR(filter_index), 0x1fff);
	xport_mips_write(FILTER_PID3_ADDR(filter_index), 0x1fff);

	xport_mips_write(FILTER_MASK0_ADDR(filter_index), 0);
	xport_mips_write(FILTER_MASK1_ADDR(filter_index), 0);
	xport_mips_write(FILTER_MASK2_ADDR(filter_index), 0);
#endif
	xport_mips_write(FILTER_BUF_LOW_ADDR(filter_index), FILTER_BUF_LOW(filter_index));
	xport_mips_write(FILTER_BUF_UP_ADDR(filter_index), FILTER_BUF_UP(filter_index));

	xport_writel(FILTER_BUF_WP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));
	xport_writel(FILTER_BUF_RP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));

	return 0;
}

int xport_filter_set_type(unsigned int filter_index, XPORT_FILTER_TYPE filter_type)
{
	unsigned int en;
	int rt;
	rt = xport_mips_read(FILTER_EN_ADDR(filter_index), &en);

	en >>= 31;		/* bit31: enable flag: 1 - output channel enabled */

	if ((rt == 0) && (en == 0)) {
		if (filter_type == FILTER_TYPE_SECTION)
			en = 3;	/* section output */
		else if (filter_type == FILTER_TYPE_TS)
			en = 1;	/* TS output */
		else if (filter_type == FILTER_TYPE_PES)
			en = 4;	/* PES output */
		else if (filter_type == FILTER_TYPE_ES)
			en = 2;	/* ES output */
		else
			en = 0;	/* reserved, no output type */

		xport_mips_write(FILTER_EN_ADDR(filter_index), en);

		return 0;
	}

	return -1;
}

int xport_filter_set_input(unsigned int filter_index, XPORT_INPUT_CHANNEL input_channel)
{
	return -1;
}

int xport_filter_set_crc(unsigned int filter_index, unsigned int crc_index)
{
	return -1;
}

int xport_filter_set_pidx(unsigned int filter_index, unsigned int pid, unsigned int slot)
{
	switch (slot) 
	{
		case 0:
			return xport_mips_write(FILTER_PID0_ADDR(filter_index), pid);

		case 1:
			return xport_mips_write(FILTER_PID1_ADDR(filter_index), pid);

		case 2:
			return xport_mips_write(FILTER_PID2_ADDR(filter_index), pid);

		case 3:
			return xport_mips_write(FILTER_PID3_ADDR(filter_index), pid);

		case 4:
			return xport_mips_write(FILTER_PID4_ADDR(filter_index), pid);

		case 5:
			return xport_mips_write(FILTER_PID5_ADDR(filter_index), pid);

		case 6:
			return xport_mips_write(FILTER_PID6_ADDR(filter_index), pid);

		case 7:
			return xport_mips_write(FILTER_PID7_ADDR(filter_index), pid);

		case 8:
			return xport_mips_write(FILTER_PID8_ADDR(filter_index), pid);

		case 9:
			return xport_mips_write(FILTER_PID9_ADDR(filter_index), pid);

		case 10:
			return xport_mips_write(FILTER_PID10_ADDR(filter_index), pid);

		case 11:
			return xport_mips_write(FILTER_PID11_ADDR(filter_index), pid);
	};

	return -1;
}

int xport_filter_set_filter(unsigned int filter_index, unsigned char *filter, unsigned char *mask)
{
	xport_mips_write(FILTER_FILTER0_ADDR(filter_index), *(unsigned int *) filter);
	xport_mips_write(FILTER_FILTER1_ADDR(filter_index), *(unsigned int *) (filter + 4));
	xport_mips_write(FILTER_FILTER2_ADDR(filter_index), *(unsigned int *) (filter + 8));
	xport_mips_write(FILTER_MASK0_ADDR(filter_index), *(unsigned int *) mask);
	xport_mips_write(FILTER_MASK1_ADDR(filter_index), *(unsigned int *) (mask + 4));
	xport_mips_write(FILTER_MASK2_ADDR(filter_index), *(unsigned int *) (mask + 8));

	return 0;
}

int xport_filter_set_filter_cond(unsigned int filter_index, unsigned char *filter_cond)
{
        xport_mips_write(FILTER_BITWISE0_ADDR(filter_index), *(unsigned int *) filter_cond);
        xport_mips_write(FILTER_BITWISE1_ADDR(filter_index), *(unsigned int *) (filter_cond + 4));
        xport_mips_write(FILTER_BITWISE2_ADDR(filter_index), *(unsigned int *) (filter_cond + 8));

	return 0;
}

int xport_filter_crc_enable(unsigned int filter_index)
{
	return -1;
}

int xport_filter_crc_disable(unsigned int filter_index)
{
	return -1;
}

int xport_filter_crc_is_enable(unsigned int filter_index)
{
	return -1;
}

int xport_filter_enable(unsigned int filter_index, spinlock_t * spin_lock_ptr)
{
	int rt;
	unsigned int en;
	unsigned long irq1_en;
	unsigned long spin_flags;

	rt = xport_mips_read(FILTER_EN_ADDR(filter_index), &en);

	if (rt != 0)
		return -1;

	xport_writel(FILTER_BUF_WP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));
	xport_writel(FILTER_BUF_RP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));

	spin_lock_irqsave(spin_lock_ptr, spin_flags);

	irq1_en = xport_readl(XPORT_INT_ENB_ADDR1);
	irq1_en |= (1 << filter_index);

	xport_writel(XPORT_INT_CLS_ADDR1, (1 << filter_index));
	xport_writel(XPORT_INT_ENB_ADDR1, irq1_en);

	spin_unlock_irqrestore(spin_lock_ptr, spin_flags);

	return xport_mips_write(FILTER_EN_ADDR(filter_index), 0x80000000 | en);
}

int xport_filter_disable(unsigned int filter_index, spinlock_t * spin_lock_ptr)
{
	int rt;
	unsigned int en;
	unsigned long irq1_en;
	unsigned long spin_flags;
#if 1
	rt = xport_mips_read(FILTER_EN_ADDR(filter_index), &en);

	if (rt != 0)
		return -1;
#endif
	spin_lock_irqsave(spin_lock_ptr, spin_flags);

	irq1_en = xport_readl(XPORT_INT_ENB_ADDR1);
	irq1_en &= (~(1 << filter_index));
	xport_writel(XPORT_INT_ENB_ADDR1, irq1_en);

	spin_unlock_irqrestore(spin_lock_ptr, spin_flags);
#if 1
	return xport_mips_write(FILTER_EN_ADDR(filter_index), 0x7fffffff & en);
#else
	return xport_mips_write(FILTER_DISABLE_ADDR(filter_index), 0);
#endif
}

int xport_filter_is_enable(unsigned int filter_index)
{
	int rt;
	unsigned int en;

	rt = xport_mips_read(FILTER_EN_ADDR(filter_index), &en);
	en >>= 31;

	if (rt == 0)
		return en;

	return -1;
}
/* seciton filter */
int xport_filter_check_section_number(unsigned int filter_index)
{
	unsigned int len;
	unsigned int wp, wp_tog;
	unsigned int rp, rp_tog;
	unsigned int up_addr, low_addr;

	up_addr = FILTER_BUF_UP(filter_index);
	low_addr = FILTER_BUF_LOW(filter_index);

	wp = xport_readl(FILTER_BUF_WP_ADDR(filter_index));
	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));

	wp_tog = wp & 0x80000000;
	rp_tog = rp & 0x80000000;

	wp = wp & 0x7fffffff;
	rp = rp & 0x7fffffff;

	if (wp_tog == rp_tog) {
		len = wp - rp;
	}
	else {
		len = (up_addr - low_addr) + wp - rp;
	}

	len = len & 0xfffff000;

	return len;
}

int xport_filter_check_section_size(unsigned int filter_index)
{
	return 0;
}

int xport_filter_clear_buffer(unsigned int filter_index)
{
        xport_writel(FILTER_BUF_RP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));
        xport_writel(FILTER_BUF_WP_ADDR(filter_index), FILTER_BUF_LOW(filter_index));

	return 0;
}

int xport_filter_read_section_data(unsigned int filter_index, char __user * buffer, size_t len)
{
	void __iomem *read_addr;

	unsigned int rp, rp_tog;
	unsigned int section_len;

	if (xport_filter_check_section_number(filter_index) <= 0) {
		return -1;
	}

	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));

	rp_tog = rp & 0x80000000;
	rp = rp & 0x7fffffff;

	read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);

	section_len = ((unsigned char *) read_addr)[1];
	section_len <<= 8;
	section_len += ((unsigned char *) read_addr)[2];
	section_len &= 0xfff;
	section_len += 3;

	if (section_len > 4096 || section_len > len) {
		return -EFAULT;
	}

	if (copy_to_user(buffer, read_addr, section_len)) {
		return -EFAULT;
	}

	rp += 0x1000;

	if (rp >= FILTER_BUF_UP(filter_index)) {
		rp = FILTER_BUF_LOW(filter_index);
		rp_tog = rp_tog ^ 0x80000000;
	}
	xport_writel(FILTER_BUF_RP_ADDR(filter_index), rp_tog | rp);

	return section_len;
}
/* ts filter */
int xport_filter_check_data_size(unsigned int filter_index)
{
	unsigned int len = 0;
	unsigned int wp, wp_tog;
	unsigned int rp, rp_tog;
	unsigned int up_addr, low_addr;

	up_addr = FILTER_BUF_UP(filter_index);
	low_addr = FILTER_BUF_LOW(filter_index);

	wp = xport_readl(FILTER_BUF_WP_ADDR(filter_index));
	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));

	wp_tog = wp & 0x80000000;
	rp_tog = rp & 0x80000000;
	wp = wp & 0x7fffffff;
	rp = rp & 0x7fffffff;

	if (wp_tog == rp_tog) {
		len = wp - rp;
	}
	else {
		len = (up_addr - low_addr) + wp - rp;
	}
	
	len = (len / 188) * 188;

	len = (len / 188) * 188;

	return len;
}

int xport_filter_read_data(unsigned int filter_index, char __user * buffer, size_t len)
{
	void __iomem *read_addr;

	unsigned int rp, rp_tog;
	unsigned int size_tmp;

	if (xport_filter_check_data_size(filter_index) < len)
		return -1;

	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));
	rp_tog = rp & 0x80000000;
	rp = rp & 0x7fffffff;

	size_tmp = FILTER_BUF_UP(filter_index) - rp;
	if (size_tmp <= len) {
		read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);
		if (copy_to_user(buffer, read_addr, size_tmp))
			return -EFAULT;

		len -= size_tmp;
		buffer += size_tmp;
		size_tmp += len;

		rp = FILTER_BUF_LOW(filter_index);
		rp_tog = rp_tog ^ 0x80000000;
	}
	else
		size_tmp = len;

	if (len > 0) {
		read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);
		if (copy_to_user(buffer, read_addr, len))
			return -EFAULT;

		rp += len;
	}

	xport_writel(FILTER_BUF_RP_ADDR(filter_index), rp_tog | rp);

	return size_tmp;
}

#define SECTION_BUCKET_SIZE  0x1000
/* pes filter */
int xport_filter_check_pes_size(unsigned int filter_index)
{
	int len;
	unsigned int pes_len = 0;

	unsigned int wp, wp_tog;
	unsigned int rp, rp_tog;
	unsigned int up_addr, low_addr;

	unsigned int section_size, data[3];
	unsigned int mask = SECTION_BUCKET_SIZE - 1;
	void __iomem *read_addr;

	up_addr = FILTER_BUF_UP(filter_index);
	low_addr = FILTER_BUF_LOW(filter_index);

	wp = xport_readl(FILTER_BUF_WP_ADDR(filter_index));
	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));

	wp_tog = wp & 0x80000000;
	rp_tog = rp & 0x80000000;
	wp = wp & (0x7fffffff & ~mask);
	rp = rp & (0x7fffffff & ~mask);

	if (wp_tog == rp_tog) {
		len = wp - rp;
	}
	else {
		len = (up_addr - low_addr) + wp - rp;
	}

	if (len < SECTION_BUCKET_SIZE)
		return 0;

	if (rp & mask)		/* rp is not aligned with 0x1000 size */
		return -EFAULT;

	section_size = 0;

	/* find the position that new pes packet starts */
	while ((len & ~mask) > 0) {
		read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);

		data[0] = *(unsigned int *) (read_addr);	/* first 4 byte of bucket header */
		data[1] = *(unsigned int *) (read_addr + 4);
		data[2] = *(unsigned int *) (read_addr + 8);

		if ((data[0] & 0x010000) && ((data[1] & 0xffffff) == 0x010000)) {	/* new pes packet */
			section_size += data[0] & 0xffff;
			section_size -= 4;
			pes_len = (data[2] & 0xff) << 8;
			pes_len |= (data[2] & 0xff00) >> 8;

			xport_writel(FILTER_BUF_RP_ADDR(filter_index), rp | rp_tog);

			rp += SECTION_BUCKET_SIZE;
			len -= SECTION_BUCKET_SIZE;

			if (rp >= up_addr) {
				rp = low_addr;
				rp_tog ^= 0x80000000;
			}

			break;
		}

		rp += SECTION_BUCKET_SIZE;
		len -= SECTION_BUCKET_SIZE;

		if (rp >= up_addr) {
			rp = low_addr;
			rp_tog ^= 0x80000000;
		}
	}

	if (section_size == 0) {	/* new pes packet is not found */
		xport_writel(FILTER_BUF_RP_ADDR(filter_index), (wp & 0xfffff000) | wp_tog);
		return 0;
	}

	/* 
	 * find the the position of next new pes packet, to get the 
	 * size of current pes packet
	 */
	while (len > 0) {
		read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);
		data[0] = *(unsigned int *) (read_addr);	/* first 4 byte of bucket header */
		data[1] = *(unsigned int *) (read_addr + 4);
		data[2] = *(unsigned int *) (read_addr + 8);

		if ((data[0] & 0x010000) && ((data[1] & 0xffffff) == 0x010000))
			break;
		else {
			section_size += (data[0] & 0xffff);
			section_size -= 4;
		}

		if (section_size > pes_len + 6)
		    section_size = pes_len + 6;

		if ((section_size == pes_len + 6) && (pes_len != 0))
			break;	/* video pes is exception */

		rp += SECTION_BUCKET_SIZE;
		len -= SECTION_BUCKET_SIZE;

		if (rp >= up_addr) {
			rp = low_addr;
			rp_tog ^= 0x80000000;
		}
	}

	if (len <= 0) {
		unsigned int buffer_size = up_addr - low_addr;
		if (section_size >= (buffer_size - (buffer_size >> 10))) {
			printk(KERN_INFO
			       "PES packet size exceed the size of filter buffer: filter id = %d, buffer size = %d section size = %d\n",
			       filter_index, up_addr - low_addr, section_size);
			return -EFAULT;
		}
		else if ((section_size == pes_len + 6) && (pes_len != 0))
			return section_size;
		else
			/* not find the next pes packet, that means current packet is not ready */
			return 0;
	}

	return section_size;
}

int xport_filter_read_pes_data(unsigned int filter_index, char __user * buffer, size_t len)
{
	unsigned int rp, rp_tog;
	unsigned int up_addr, low_addr;
	unsigned int data, bucket_size;
	int pes_size, byte_written;
	unsigned int mask = SECTION_BUCKET_SIZE - 1;

	void __iomem *read_addr;

	pes_size = xport_filter_check_pes_size(filter_index);
	if (pes_size <= 0) {
		printk(KERN_INFO "pes size = %d, empty now \n", pes_size);
		return -EFAULT;
	}
	if (pes_size > len)
		pes_size = len;

	byte_written = pes_size;

	up_addr = FILTER_BUF_UP(filter_index);
	low_addr = FILTER_BUF_LOW(filter_index);

	rp = xport_readl(FILTER_BUF_RP_ADDR(filter_index));

	rp_tog = rp & 0x80000000;
	rp = rp & 0x7fffffff;
	if (rp & mask) {	/* rp is not aligned with 0x1000 */
		printk(KERN_INFO "rp 0x%08x is not aligned with 0x1000\n", rp);
		return -EFAULT;
	}

	while (pes_size > 0) {
		read_addr = xport_mem_base + (rp - XPORT_MEM_BASE);
		data = *(unsigned int *) read_addr;	/* first 4 byte of bucket header */
		read_addr += 4;
		bucket_size = (data & 0xffff);
		bucket_size -= 4;

		if (bucket_size > pes_size)
			bucket_size = pes_size;

		if (copy_to_user(buffer, read_addr, bucket_size) > 0) {
			printk(KERN_INFO " user buffer is not enough \n");
			return -EFAULT;
		}

		pes_size -= bucket_size;
		buffer += bucket_size;
		rp += SECTION_BUCKET_SIZE;

		if (rp >= up_addr) {
			rp = low_addr;
			rp_tog ^= 0x80000000;
		}
	}

	xport_writel(FILTER_BUF_RP_ADDR(filter_index), rp_tog | rp);

	return byte_written;
}

