
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

#include "xport_mips.h"
#include "xport_drv.h"

#ifdef CONFIG_ORION_XPORT_DEBUG
#define DEBUG(n, args...)				\
 	do {						\
		if (n <= CONFIG_XPORT_DEBUG_VERBOSE)	\
			printk(KERN_INFO args);		\
	} while(0)
#else /* CONFIG_XPORT_DEBUG */
#define DEBUG(n, args...) do { } while(0)
#endif


DECLARE_MUTEX(mips_cmd_sem);

int xport_mips_write(unsigned int cmd, unsigned int req_dat)
{
	unsigned int cnt = 1000;
	unsigned int regs_val;
	union firmware_req req;

	req.val = cmd;
	req.val |= 0xc0000000;	/* bit31 = 1, indicates that will have a request */
				/* bit30 = 1, indicates that a request will send to firmware */

	if (down_interruptible(&mips_cmd_sem)) {
		DEBUG(" mips_write error: down_interruptible () \n");
		return -1;
	}

	do {
		regs_val = xport_readl(MIPS_CMD_REQ);
		udelay(50);
	} while ((cnt-- > 0) && (regs_val >> 31));

	if (regs_val >> 31) {
		up(&mips_cmd_sem);
		DEBUG(" mips_write error: timeout \n");

		return -1;
	}

	xport_writel(MIPS_CMD_DATA, req_dat);	/* write a request to firmware */
	xport_writel(MIPS_CMD_REQ, req.val);

	up(&mips_cmd_sem);

	return 0;
}

int xport_mips_read(unsigned int cmd, unsigned int *req_dat)
{
	unsigned int cnt = 1000;
	unsigned int regs_val = 0;

	union firmware_req req;

	req.val = cmd;
	req.val |= 0x80000000;	/* bit31 = 1, indicates that will have a request */
				/* bit30 = 0, indicates that get information from firmware */

	if (down_interruptible(&mips_cmd_sem)) {
		DEBUG(" mips_read error: down_interruptible () \n");
		return -1;
	}

	do {
		regs_val = xport_readl(MIPS_CMD_REQ);
		udelay(50);
	} while ((cnt-- > 0) && (regs_val >> 31));

	if (regs_val >> 31) {
		up(&mips_cmd_sem);
		DEBUG(" mips_read error: timeout0 \n");

		return -1;
	}

	xport_writel(MIPS_CMD_REQ, req.val);	/* send read-request to firmware */
	udelay(50);

	cnt = 1000;
	do {
		regs_val = xport_readl(MIPS_CMD_REQ);
		udelay(50);
	} while ((cnt-- > 0) && (regs_val >> 31));

	if (regs_val >> 31) {
		up(&mips_cmd_sem);
		DEBUG(" mips_read error: timeout1 \n");

		return -1;
	}

	*req_dat = xport_readl(MIPS_CMD_DATA);

	up(&mips_cmd_sem);

	return 0;
}

int xport_mips_read_ex(unsigned int cmd, unsigned int *req_dat, unsigned int *req_dat2)
{
	unsigned int cnt = 1000;
	unsigned int regs_val = 0;

	union firmware_req req;

	req.val = cmd;
	req.val |= 0x80000000;	/* bit31 = 1, indicates that will have a request */
				/* bit30 = 0, indicates that get information from firmware */

	if (down_interruptible(&mips_cmd_sem)) {
		DEBUG(" mips_read error: down_interruptible () \n");
		return -1;
	}

	do {
		regs_val = xport_readl(MIPS_CMD_REQ);
		udelay(50);
	} while ((cnt-- > 0) && (regs_val >> 31));

	if (regs_val >> 31) {
		up(&mips_cmd_sem);
		DEBUG(KERN_INFO " mips_read error: timeout0 \n");

		return -1;
	}

	xport_writel(MIPS_CMD_REQ, req.val);	/* send read-request to firmware */
	udelay(50);

	cnt = 1000;
	do {
		regs_val = xport_readl(MIPS_CMD_REQ);
		udelay(50);
	} while ((cnt-- > 0) && (regs_val >> 31));

	if (regs_val >> 31) {
		up(&mips_cmd_sem);
		DEBUG( " mips_read error: timeout1 \n");

		return -1;
	}

	*req_dat = xport_readl(MIPS_CMD_DATA);
	*req_dat2 = xport_readl(MIPS_CMD_DATA2);

	up(&mips_cmd_sem);

	return 0;
}

EXPORT_SYMBOL(xport_mips_write);
EXPORT_SYMBOL(xport_mips_read);
EXPORT_SYMBOL(xport_mips_read_ex);

