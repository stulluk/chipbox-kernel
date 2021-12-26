#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>     
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/errno.h> 
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <linux/interrupt.h> 
#include <linux/poll.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "orion_spi.h"

/* Register operation */
#define spi_writeb(a,v)    	writeb(v, (orion_spi_base + (a)))
#define spi_writew(a,v)    	writew(v, (orion_spi_base + (a)))
#define spi_writel(a,v)    	writel(v, (orion_spi_base + (a)))

#define spi_readb(a)      	readb((orion_spi_base + (a)))
#define spi_readw(a)       	readw((orion_spi_base + (a)))
#define spi_readl(a)       	readl((orion_spi_base + (a)))

//#define CONFIG_SPI_DEBUG
static int SPI_DEBUG = 0;
#ifdef CONFIG_SPI_DEBUG
# ifdef __KERNEL__
#  define SDEBUG(args...) printk(args)
# else
#  define SDEBUG(args...) printf(args)	
# endif  
#else
# define SDEBUG(args...) \
	if (SPI_DEBUG) printk(args)
#endif

#if 0
static volatile u32 __iomem *orion_spi_base = NULL;
#define spi_readl(a)            *(orion_spi_base + (a))
#define spi_writel(a,v)         do {*(orion_spi_base + (a)) = (v); } while(0)
#endif

/* The varable definiation */
static void __iomem *orion_spi_base;
static struct proc_dir_entry *spi_proc_entry = NULL;
static orion_spi_t *spi = NULL;

/* File operation */
static int orion_spi_open(struct inode *inode, struct file *file);
static int orion_spi_release(struct inode *inode, struct file *file);
static ssize_t orion_spi_read(struct file *file, char __user * buffer, size_t len, loff_t * offset);
static ssize_t orion_spi_write(struct file *file, const char __user * buffer, size_t len, loff_t * offset);
static loff_t orion_spi_lseek(struct file *file, loff_t offset, int orig);
static int orion_spi_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

static void
__spi_bus_init(void)
{
	unsigned int ctrl0 = 0;

	/* Disable the ssi by write 0 to SSIEN,tx and rx buffer was cleared */
	spi_writel(rSSIENR, 0x0);

	/* Config the CTRLR0 register 		*/
	ctrl0 = 0x0 << 12  |	/* CFS 		*/
		0x0 << 11  |	/* SRL 0	*/
		0x0 << 10  |	/* SLV_OE 	*/
		0x0 << 8   |	/* TMOD master  */
		0x1 << 7   |	/* SCPOL clock	*/
		0x1 << 6   |	/* SCPH 	*/
		0x0 << 4   |	/* frame format	*/
		0x7 << 0;	/* DFS		*/

	spi_writel(rCTRLR0, ctrl0);

	/* only take effect on eeprom and receive only mode */
	spi_writel(rCTRLR1, 0x3ff);

	/* 
	 * SSI_CLK baud out 
	 * 250K: 0xC8
	 * 500K: 0x6e
	 * 1M  : 0x37
	 */
#ifndef CONFIG_SLAVE_SPEED
	spi_writel(rBAUDR, 0x37);
#endif

	/* set the threshold of tx data */
	spi_writel(rTXFTLR, 0x0);
	spi_writel(rRXFTLR, 0x0);

	/* 
	 * rIMR, unmask intr 
	 * 0x1: Except Receive FIFO Full Interrupt, mask all others 
	 */
	spi_writel(rIMR, 0x0);

	/* rSER, select the device */
	spi_writel(rSER, 0x1);

	/* rSSIENR, enable the ssi bus */
	spi_writel(rSSIENR, 0x1);

	/* 
	 * Finish step3, for now normal operation can be performed 
	 */
}

static void __spi_set_ndf(u32 ndf)
{
	if (ndf > IFD_BUF_SIZE-1) ndf = IFD_BUF_SIZE-1;

	spi_writel(rSSIENR, 0);
	spi_writel(rCTRLR1, ndf);
	spi_writel(rSSIENR, 1);
}

static void __spi_set_tmod(CSSPI_TMOD tmod)
{
	u32 ctrl0 = spi_readl(rCTRLR0);
	ctrl0 &= ~(0x3<<8) & 0xffff;

	spi_writel(rSSIENR, 0);
	switch (tmod) {
		case CSSPI_TXRX:
		case CSSPI_TXO:
		case CSSPI_RXO:
		case CSSPI_EEPROM:
			ctrl0 |= tmod<<8 & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		default:
			SDEBUG("%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}
	spi_writel(rSSIENR, 1);
}

static void __spi_set_baud(CSSPI_SCBAUD rate)
{
	spi_writel(rSSIENR, 0);

	switch (rate) {
		case CSSPI_500K:
			spi_writel(rBAUDR, 0x6e);
			break;
		case CSSPI_1MH:
			spi_writel(rBAUDR, 0x37);
			break;
		case CSSPI_2MH:
			spi_writel(rBAUDR, 0x1B);
			break;
		case CSSPI_3MH:
			spi_writel(rBAUDR, 0x12);
			break;
		case CSSPI_4MH:
			spi_writel(rBAUDR, 0x0E);
			break;
		case CSSPI_5MH:
			spi_writel(rBAUDR, 0x0B);
			break;
		default:
			SDEBUG("%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}
	spi_writel(rSSIENR, 1);

}

static void __spi_set_spi(CSSPI_SPI status)
{
	switch (status) {
		case CSSPI_DISABLE:
			spi_writel(rSSIENR, status);
			break;
		case CSSPI_ENABLE:
			spi_writel(rSSIENR, status);
			break;
		default:
			SDEBUG("%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}
}

static void __spi_set_slave(CSSPI_SLAVE cs) 
{
	/* 
	 * Force minimal delay between two transfers - in case two transfers
	 * follow each other w/o delay, then we have to wait here in order for
	 * the peripheral device to detect cs transition from inactive to active. 
	 */
	switch (cs) {
		case CSSPI_SDISABLE:
			spi_writel(rSER, cs);
			break;
		case CSSPI_SENABLE:
			spi_writel(rSER, cs);
			break;
		default:
			SDEBUG("%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}

	/* Give a wait time to ensure the state can be set */
	udelay(1);
}


static void __spi_set_timing(CSSPI_TIMING mode)
{
	u32 ctrl0 = spi_readl(rCTRLR0);
	ctrl0 &= ~(0x3<<6) & 0xffff;

	spi_writel(rSSIENR, 0);
	switch (mode) {
		case CSSPI_MODE0:
			ctrl0 |= mode<<6 & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		case CSSPI_MODE1:
			ctrl0 |= mode<<6 & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		case CSSPI_MODE2:
			ctrl0 |= mode<<6 & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		case CSSPI_MODE3:
			ctrl0 |= mode<<6 & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		default:
			SDEBUG("%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}
	spi_writel(rSSIENR, 1);
}

/* ----------------------------------------------------------------------------
 * The following two interface is used mainly for operate convenience
 * for userspace
 */
static void __spi_set_polarity(CSSPI_SCPOL pol)
{
	u32 ctrl0 = spi_readl(rCTRLR0);

	spi_writel(rSSIENR, 0);

	switch (pol) {
		case CSSPI_LOWPOL:
			ctrl0 &= ~(0x1<<7) & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		case CSSPI_HIGHPOL:
			ctrl0 |= (0x1<<7) & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		default:
			printk(KERN_ERR "%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}

	spi_writel(rSSIENR, 1);
}

static void __spi_set_phase(CSSPI_SCPH phase)
{
	u32 ctrl0 = spi_readl(rCTRLR0);

	spi_writel(rSSIENR, 0);

	switch (phase) {
		case CSSPI_MIDDLE:
			ctrl0 &= ~(0x1<<6) & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		case CSSPI_START:
			ctrl0 |= (0x1<<6) & 0xffff;
			spi_writel(rCTRLR0, ctrl0);
			break;
		default:
			printk(KERN_ERR "%s[%d]Invalid value!\n", 
			       __FUNCTION__, __LINE__);
			break;
	}

	spi_writel(rSSIENR, 1);
}

void __spi_intr_mask(unsigned int m)
{
	unsigned int val;
	val = spi_readl(rIMR);
	spi_writel(rIMR, val&~m);
}

void __spi_intr_unmask(unsigned int m)
{
	unsigned int val;
	val = spi_readl(rIMR);
	spi_writel(rIMR, val|m);
}

/*----------------------------------------------------------------------------*/

int __flash_read(unsigned int chip_addr, int alen, unsigned char *buf, int size)
{
	int i, j;
	unsigned char *byte_addr = (unsigned char *) &chip_addr;

	if (size > 1024 || size <= 0)
		return -1;

	__spi_set_tmod(CSSPI_EEPROM);
	__spi_set_ndf(0x3ff);
	__spi_set_timing(CSSPI_MODE3);

	spi_writew(rDR, 0x3);

	for (i = alen - 1; i >= 0; i--)
		spi_writew(rDR, byte_addr[i]);

	i = 0;
	while (i < size) {
		while (!(j = spi_readw(rRXFLR)));
		while (j > 0) {
			buf[i] = spi_readw(rDR);
			i++; j--;
		}
	}

	/* Are we free? */
	while (spi_readw(rSR) & 0x1);

	/* stop addressing */
	spi_writew(rSSIENR, 0x0);

	return 0;
}

/* max size is 256(one page) */
int __flash_write(unsigned int chip_addr, int alen, unsigned char *buf, int size)
{
	int i;
	unsigned char *byte_addr = (unsigned char *) &chip_addr;

	if (size > 256 || size <=0)
		return -1;

	__spi_set_tmod(CSSPI_TXRX);
	__spi_set_timing(CSSPI_MODE0);
	spi_writew(rDR, 0x06);
	mdelay(1);
	__spi_set_timing(CSSPI_MODE3);

	spi_writew(rDR, 0x2);

	for (i = alen - 1; i >= 0; i--)
		spi_writew(rDR, byte_addr[i]);

	for (i = 0; i < size; i++) {
		spi_writew(rDR, buf[i]);
		while (spi_readw(rTXFLR));
	}

	mdelay(2);

	while (spi_readw(rSR) & 0x1);

	return 0;
}

int __flash_erase(unsigned int chip_addr, int alen, int type)
{
	int i;
	unsigned char *byte_addr = (unsigned char *) &chip_addr;

	__spi_set_tmod(CSSPI_TXRX);
	__spi_set_timing(CSSPI_MODE0);
	spi_writew(rDR, 0x06);
	mdelay(1);
	__spi_set_timing(CSSPI_MODE3);

	switch (type) {
		case 0: /* 4K sector */
			spi_writew(rDR, 0x20);
			for (i = alen - 1; i >= 0; i--)
				spi_writew(rDR, byte_addr[i]);
			mdelay(150);
			break;
		case 1: /* 64K block */
			spi_writew(rDR, 0xd8); /* block */
			for (i = alen - 1; i >= 0; i--)
				spi_writew(rDR, byte_addr[i]);
			mdelay(800);
			break;
		case 2: /* chip 512K */
			spi_writew(rDR, 0xc7);
			mdelay(5000);   /* ten second for EON80, i.e. erase 512KB */
			break;
		case 3: /* chip 1M */
			spi_writew(rDR, 0xc7);
			mdelay(10000);   /* ten second for EON80, i.e. erase 512KB */
			break;
		default:
			SDEBUG("Error! No such chioce.\n");
	}

	while (spi_readw(rSR) & 0x1);

	return 0;
}

/* ----------------------------------------------------------------------------
 * Flash Simple Test (R/W)
 */
static void __flash_wren_s(int val)
{
	__spi_set_timing(CSSPI_MODE0);
	if (1 == val)
		spi_writew(rDR, 0x06);
	else
		spi_writew(rDR, 0x04);
	mdelay(1);
	__spi_set_timing(CSSPI_MODE3);
}

static void __flash_read_s(void)
{
	__spi_set_tmod(CSSPI_TXRX);
	__spi_set_timing(CSSPI_MODE3);

	/* write data */
	spi_writew(rDR, 0x3);
	/* write addr */
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);

	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);

	/* polling for finish */
	while (spi_readw(rSR) & 0x1);
	while (spi_readw(rTXFLR) & 0x1);
	while (! (spi_readw(rSR) & 0x8));
	mdelay(10);	/* important */

	/* read dummy data */
	spi_readw(rDR);
	spi_readw(rDR);
	spi_readw(rDR);
	spi_readw(rDR);
	
	printk("read data 1: 	0x%x\n", spi_readw(rDR));
	printk("read data 2: 	0x%x\n", spi_readw(rDR));
	printk("read data 3: 	0x%x\n", spi_readw(rDR));
}

static void __flash_write_s(void)
{
	__spi_set_tmod(CSSPI_TXRX);
	__spi_set_timing(CSSPI_MODE3);

	__flash_wren_s(1);

	/* write data */
	spi_writew(rDR, 0x2);
	/* write addr */
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);

	/* write 3 byte */
	spi_writew(rDR, 0x1);	
	spi_writew(rDR, 0x2);
	spi_writew(rDR, 0x3);

	/* polling for finish */
	while (spi_readw(rSR) & 0x1);
	while (spi_readw(rTXFLR) & 0x1);
	while (! (spi_readw(rSR) & 0x8));
	mdelay(20);	/* important */

	__flash_wren_s(0);
}

static void __flash_erase_s(void)
{
	__spi_set_tmod(CSSPI_TXRX);
	__spi_set_timing(CSSPI_MODE3);

	__flash_wren_s(1);

	/* write data */
	spi_writew(rDR, 0xd8);
	/* write addr */
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);
	spi_writew(rDR, 0x0);

	/* polling for finish */
	while (spi_readw(rSR) & 0x1);
	while (spi_readw(rTXFLR) & 0x1);
	while (! (spi_readw(rSR) & 0x8));
	mdelay(500);	/* important */

	__flash_wren_s(0);
}

/*----------------------------------------------------------------------------*/

static int 
__proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	orion_spi_t *spi = (orion_spi_t *) file->private_data;
	u32 addr;
	u32 val;
	unsigned long flags;
	const char *cmd_line = buffer;;

	if (strncmp("rl", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = spi_readl(addr);
		printk(" readl [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);
	} else if (strncmp("wl", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = simple_strtoul(&cmd_line[7], NULL, 16);
		spi_writel(addr, val);
		printk(" writel [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);

	} else if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = spi_readw(addr);
		printk(" readw [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);
	} else if (strncmp("ww", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = simple_strtoul(&cmd_line[7], NULL, 16);
		spi_writew(addr, (u16)val);
		printk(" writew [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);

	} else if (strncmp("rb", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = spi_readb(addr);
		printk(" readb [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);
	} else if (strncmp("wb", cmd_line, 2) == 0) {
		addr = simple_strtoul(&cmd_line[3], NULL, 16);
		val = simple_strtoul(&cmd_line[7], NULL, 16);
		spi_writeb(addr, (u8)val);
		printk(" writeb [0x%04x](%p ==> %p) = 0x%08x \n", 
		       addr, orion_spi_base, orion_spi_base+addr, val);

		/*************************************************
		  TEST serial flash OPERATION
		 *************************************************/
	} else if (strncmp("sfr", cmd_line, 3) == 0) {
		spin_lock_irqsave(&spi->spi_lock, flags);
		__flash_read_s();
		spin_unlock_irqrestore(&spi->spi_lock, flags);

	} else if (strncmp("sfw", cmd_line, 3) == 0) {
		spin_lock_irqsave(&spi->spi_lock, flags);
		__flash_write_s();
		spin_unlock_irqrestore(&spi->spi_lock, flags);

	} else if (strncmp("sfe", cmd_line, 3) == 0) {
		spin_lock_irqsave(&spi->spi_lock, flags);
		__flash_erase_s();
		spin_unlock_irqrestore(&spi->spi_lock, flags);
	} else if (strncmp("eeid", cmd_line, 4) == 0) {
		spin_lock_irqsave(&spi->spi_lock, flags);
		__spi_set_tmod(CSSPI_EEPROM);
		__spi_set_ndf(0x3);
		__spi_set_timing(CSSPI_MODE3);
		spi_writel(rDR, 0x9f);

		while (spi_readw(rRXFLR) < 3);

		printk("Manufacture ID: 0x%x\n", spi_readw(rDR));
		printk("Device ID 1:    0x%x\n", spi_readw(rDR));
		printk("Device ID 2:    0x%x\n", spi_readw(rDR));
		spin_unlock_irqrestore(&spi->spi_lock, flags);
	} else if (strncmp("dbg", cmd_line, 3) == 0) {	/* set remote controller protocals */
		if (SPI_DEBUG) 	SPI_DEBUG = 0;
		else 		SPI_DEBUG = 1;
	}

	return count;
}

static void spi_do_tasklet(unsigned long data)
{
}

static int
orion_spi_open(struct inode *inode, struct file *file)
{
	SDEBUG("Spi device opened!\n");
	spi = (orion_spi_t *) kmalloc(sizeof(orion_spi_t), GFP_KERNEL);
	memset(spi, 0, sizeof(orion_spi_t));
	spi->baud = CONFIG_SLAVE_SPEED;
#ifdef CONFIG_SFLASH
	spi->alen = CONFIG_FLASH_ALEN;
	spi->size = CONFIG_FLASH_SIZE;
#endif
#ifdef CONFIG_SPI_COMMON
	spi->open_cnt++;
	spi->rxtimeout = RXTIMEOUT_DEFAULT;
	spi->receive_exception = 0;
	spi->tasklet.func = spi_do_tasklet;
	init_MUTEX(&spi->tq_sem);
	init_waitqueue_head(&spi->rq_queue);
	init_waitqueue_head(&spi->tq_queue);
	__spi_intr_unmask(SPI_RXFIS | SPI_RXOIS | SPI_RXUIS);
#endif
	spin_lock_init(&spi->spi_lock);
	file->private_data = (void *)spi;

	return 0;
}

static int 
orion_spi_release(struct inode *inode, struct file *file)
{
#ifdef CONFIG_SPI_COMMON
	/* ajust really close */
	spi->open_cnt--;
	if (spi->open_cnt) 
		return 0;

	__spi_intr_mask(0x1f);
#endif
	kfree(spi);
	SDEBUG("Spi device closed!\n");
	return 0;
}

#ifdef CONFIG_SPI_COMMON
static unsigned int orion_spi_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(filp, &spi->rq_queue, wait);
	poll_wait(filp, &spi->tq_queue, wait);

	if (!EMPTY(spi->rq)) mask |= POLLIN;
	if (CHARS(spi->rq) < IFD_BUF_SIZE/10) mask |= POLLPRI;
	if (!FULL(spi->tq)) mask |= POLLOUT;

	return mask;
}
#endif

static ssize_t
orion_spi_read(struct file *file, char __user * buffer, size_t len, loff_t * f_pos)
{
	unsigned long flags;
	int i, j;

	unsigned char *rbuf = (char *) kmalloc(len*sizeof(char), GFP_KERNEL);
	memset(rbuf, 0, len);

	SDEBUG("chip_addr: 0x%x, len: 0x%x\n", *f_pos, len);

#ifdef CONFIG_SFLASH
	if (*f_pos > spi->size)
		return -ENOMEM;
	
	if ((spi->size - *f_pos)  < len)
		len = spi->size - *f_pos;

	i = len / 1024;
	j =  len % 1024;
	SDEBUG("i = %d, j = %d, size: %d\n",i , j, len);

	spin_lock_irqsave(&spi->spi_lock, flags);
	while (i > 0) {
		__flash_read(*f_pos, spi->alen, rbuf, 1024);
		*f_pos += 1024;	/* chip offset */
		rbuf += 1024;	/* receive buffer */
		i--;
	}
	__flash_read(*f_pos, spi->alen, rbuf, j);
	spin_unlock_irqrestore(&spi->spi_lock, flags);
#endif

#ifdef CONFIG_SPI_COMMON	
	if (EMPTY(spi->rq) && (file->f_flags & O_NONBLOCK))
		return -EAGAIN;

	if (len > IFD_BUF_SIZE) len = IFD_BUF_SIZE;

	if (!wait_event_interruptible_timeout(spi->rq_queue, spi->rq.size >= len || spi->receive_exception, spi->rxtimeout*HZ))
		SDEBUG("SPI read timeout! LEFT(spi->rq): %d, spi->receive_exception: %d\n", LEFT(spi->rq), spi->receive_exception);

	switch (spi->receive_exception) {
		case SPI_RXUIS:
			break;
		case SPI_TXOIS:
			break;
		case SPI_RXOIS:
			SDEBUG("SPI RECEIVE EXCEPTION!( Fifo Overflow. )\n");
			spi->receive_exception = 0;
			return -EINTR;
		default:
			SDEBUG("SPI RECEIVE EXCEPTION! spi->receive_exception: %x\n", spi->receive_exception);
	}

#if 0
	spin_lock_irqsave(&spi->spi_lock, flags);
	if (CHARS(rq) < len) 
		len =  CHARS(rq);
	spin_unlock_irqrestore(&spi->spi_lock, flags);
#endif

	SDEBUG("spi->rq.size: %d\n", spi->rq.size);
	for (i = 0; i < len && spi->rq.size; i++) {
		GETCH(spi->rq, rbuf[i]); 
		spi->rq.size--;
	}
	len = i;
#endif
	if (unlikely(copy_to_user(buffer, rbuf, len))) 
		return -EFAULT;
	kfree(rbuf);
	return len;
}

static ssize_t
orion_spi_write(struct file *file, const char __user * buffer, size_t len, loff_t * f_pos)
{
	int i, j;
	int space;
	size_t written = 0;
	unsigned long flags;

	unsigned char *wbuf = (char *) kmalloc(len*sizeof(char), GFP_KERNEL);
	memset(wbuf, 0, len);

	SDEBUG("chip_addr: 0x%x, size: %x\n", *f_pos, len);

#ifdef CONFIG_SFLASH
	if (*f_pos > spi->size)
		return -ENOMEM;

	if ((spi->size - *f_pos)  < len)
		len = spi->size - *f_pos;
#endif

	if (unlikely(copy_from_user(wbuf, buffer, len)))
		return -EFAULT;

#ifdef CONFIG_SFLASH
        i = len / 256;
        j = len % 256;
        SDEBUG("i = %d, j = %d, size: %d\n",i , j, len);
	spin_lock_irqsave(&spi->spi_lock, flags);
        while (i > 0) {
                __flash_write(*f_pos, spi->alen, wbuf, 256);
                *f_pos += 256;
                wbuf += 256;
                i--;
        }
        __flash_write(*f_pos, spi->alen, wbuf, j);
	spin_unlock_irqrestore(&spi->spi_lock, flags);
#endif

#ifdef CONFIG_SPI_COMMON
	SDEBUG("CHARS(spi->tq): %x, LEFT(spi->tq): %x\n", CHARS(spi->tq), LEFT(spi->tq));
	if (FULL(spi->tq) && (file->f_flags & O_NONBLOCK))
		return -EAGAIN;

	down_interruptible(&spi->tq_sem);
	j = 0;
	while (written < len) {
		space = (( CHARS(spi->tq) + written) > len) ? (len - written) : CHARS(spi->tq);
		for (i = 0; i < space; i++) {
			PUTCH(wbuf[j+i], spi->tq); 
			SDEBUG("\nwirte_buf[%d]=0x%x", i, wbuf[i]);
		}

#if 0 		
		{	/* FIXME */
			__spi_intr_unmask(SPI_TXOIS);
			for (i = 0; i < 100; i++)
				spi_writew(rDR, 1);
			printk("i = %d\n", i);
			return 0;
		}
#endif
		__spi_intr_unmask(SPI_TXEIS);
		wait_event_interruptible(spi->tq_queue, EMPTY(spi->tq));
		SDEBUG("%s: wake up! space(handle chars) = %d\n", __FUNCTION__, space);

		written += i;
		j += i;
	}

	len = written;
	*f_pos += written;
	up(&spi->tq_sem);
#endif
	kfree(wbuf);
	return len;
}

static loff_t
orion_spi_lseek(struct file *file, loff_t offset, int orig)
{
	down(&file->f_dentry->d_inode->i_sem);
	switch (orig) {
		case 0:	/* SEEK_SET */
			file->f_pos = offset;
			break;
		case 1: /* SEEK_CUR */
			file->f_pos += offset;
			break;
		case 2: /* SEEK_END */
			break;
		default:
			SDEBUG("No such chioce!\n");
	}

	up(&file->f_dentry->d_inode->i_sem);
	return 0;
}

static int
orion_spi_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long flags;

	switch (cmd) {
		case SPI_BUS_STAT:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_spi(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SLAVE_STAT:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_slave(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_BAUD:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_baud(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_TMOD:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_tmod(arg);	/* FIXME arg2 hard coded!!! */
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_POL:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_polarity(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_PH:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_phase(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_TIMING:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_timing(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_SET_NDF:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__spi_set_ndf(arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;

		case SPI_FLASH_R:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__flash_read_s();
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_FLASH_W:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__flash_write_s();
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_FLASH_E:
			spin_lock_irqsave(&spi->spi_lock, flags);
			__flash_erase_s();
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;

		case SPI_READ_BYTE:
			spin_lock_irqsave(&spi->spi_lock, flags);
			return spi_readw(rDR);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		case SPI_WRITE_BYTE:
			spin_lock_irqsave(&spi->spi_lock, flags);
			spi_writew(rDR, (unsigned char)arg);
			spin_unlock_irqrestore(&spi->spi_lock, flags);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static struct file_operations orion_spi_fops = {                                                           
	.owner 	= THIS_MODULE,
	.read 	= orion_spi_read,
	.write 	= orion_spi_write,
#ifdef CONFIG_SPI_COMMON
	.poll 	= orion_spi_poll,
#endif
	.llseek = orion_spi_lseek,
	.ioctl 	= orion_spi_ioctl,
	.open	= orion_spi_open,
	.release= orion_spi_release,
};

static struct miscdevice orion_spi_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_spi",
	&orion_spi_fops
};

#ifdef CONFIG_SPI_COMMON
static irqreturn_t spi_dev_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
	int intr_status;
	int count;
	unsigned char c;

	intr_status = spi_readw(rISR);
	SDEBUG("intr_status: %x, Received CNT: %d\n", intr_status, spi_readw(rRXFLR));
	if (intr_status & SPI_RXFIS) {
		SDEBUG("Intr! Reveive fifo full!\n");
		count = spi_readw(rRXFLR);
		SDEBUG("fifo count: %d\n", spi_readw(rRXFLR));
		while (count--) {
			c = spi_readw(rDR);
			PUTCH(c,spi->rq);
			/* 
			 * TODO: this value is controlled by IFD_BUF_SIZE
			 * 	 we set NDF according to the buffer size.
			 */
			spi->rq.size++;
		}
		wake_up_interruptible(&spi->rq_queue);
/*                tasklet.data = (unsigned int)0;*/
/*                tasklet_schedule(&tasklet);*/
	} 
	
	if (intr_status & SPI_RXUIS) {
		spi_readw(rRXUICR);
		SDEBUG("Intr! Receive fifo underflow!\n");
		spi->receive_exception = SPI_RXUIS;
		wake_up_interruptible(&spi->rq_queue);
	} 
	
	if (intr_status & SPI_RXOIS) {
		spi_readw(rRXOICR);
		SDEBUG("Intr! Receive fifo overflow!\n");
		spi->receive_exception = SPI_RXOIS;
		wake_up_interruptible(&spi->rq_queue);
	} 
	
	if (intr_status & SPI_TXOIS) {
		SDEBUG("Intr! Transmit fifo overflow!\n");
		spi->receive_exception = SPI_TXOIS;
		spi_readw(rTXOICR);
	} 
	
	if (intr_status & SPI_TXEIS) {
		SDEBUG("Intr! Transmit fifo empty!\n");
		while (!EMPTY(spi->tq)) {
			GETCH(spi->tq, c);
			spi_writew(rDR, c);
			while (spi_readw(rTXFLR));
		}
		while (spi_readw(rSR) & 0x1);
		
		__spi_intr_mask(SPI_TXEIS);
		wake_up_interruptible(&spi->tq_queue);
	} 
	
	if (intr_status & SPI_MSTIS) {
		spi_readw(rMSTICR);
		SDEBUG("Intr! Multi-Master Contention Raw Interrupt active!\n");
	} 

	return IRQ_HANDLED;
}
#endif

int __init
orion_spi_init(void)
{
	int ret = 0;

	SDEBUG("Register spi module!\n");

	if (misc_register(&orion_spi_miscdev)) {
		ret = -ENODEV;
		goto err1;
	}

	if (!request_mem_region(ORION_SPI_BASE, ORION_SPI_SIZE, "orion_spi")) {
		ret = -ENOMEM;
		goto err2;
	}

	orion_spi_base = ioremap(ORION_SPI_BASE, ORION_SPI_SIZE);
	if (!orion_spi_base) {
		ret = -EIO;
		goto err3;
	}

	spi_proc_entry = create_proc_entry("spi_io", 0, NULL);
	if (NULL == spi_proc_entry) {
		ret = -EFAULT;
		goto err4; 
	} else {
		spi_proc_entry->write_proc = &__proc_write;
	}

#ifdef CONFIG_SLAVE_SPEED
	__spi_set_baud(CONFIG_SLAVE_SPEED);
#endif
	__spi_bus_init();

#ifdef CONFIG_SPI_COMMON
	/* SA_INTERRUPT */
	if (request_irq(ORION_SPI_IRQ, spi_dev_interrupt, 0, "orion spi", NULL)) {
		ret = -EBUSY;
		goto err5;
	}
#endif

	goto out;

err5:
	remove_proc_entry("spi_io", NULL);
err4:
	iounmap((void *)orion_spi_base);
err3:
	release_mem_region(ORION_SPI_BASE, ORION_SPI_SIZE);
err2:
	misc_deregister(&orion_spi_miscdev);
err1:
out:
	return ret;

}

static void __exit
orion_spi_exit(void)
{
	SDEBUG("Unregister spi module!\n");

	__spi_set_slave(0);

	free_irq(ORION_SPI_IRQ, NULL);
	if (NULL != spi_proc_entry) remove_proc_entry("spi_io", NULL);
	iounmap((void *)orion_spi_base);
	release_mem_region(ORION_SPI_BASE, ORION_SPI_SIZE);
	misc_deregister(&orion_spi_miscdev);
}

module_init(orion_spi_init);
module_exit(orion_spi_exit);

/* ----------------------------------------------------------------------------
 *  Export interface to use outside.
 */
void spi_mode0(void)
{
	unsigned short ctrl0 = spi_readl(rCTRLR0);

	spi_writel(rSSIENR, 0x0);

	ctrl0 &= ~(1<<6) & 0xffff;
	ctrl0 &= ~(1<<7) & 0xffff;
	spi_writel(rCTRLR0, ctrl0);
	spi_writel(rSSIENR, 0x1);
}

/* To get a continous timing */
void spi_mode3(void)
{
	unsigned short ctrl0 = spi_readl(rCTRLR0);
	spi_writel(rSSIENR, 0x0);
	ctrl0 |= (1<<6) & 0xffff;
	ctrl0 |= (1<<7) & 0xffff;
	spi_writel(rCTRLR0, ctrl0);
	spi_writel(rSSIENR, 0x1);
}

u32 orion_spi_reg_read(void)
{
	int ret = 0;
	/* The minimal interval for a continous transfer */
	while (spi_readl(rSR) & SPI_SR_BUSY);
	while (spi_readl(rSR) & SPI_SR_RFNE);

	ret = spi_readl(rDR);
	return  ret;
}

u32 orion_spi_reg_write(u8 *data, int len)
{
	u32 cnt = 0;

	if (NULL == data || 0 == len || len > 0x30)
		return -EINVAL;

	/* Set mode */
	if (1 == len) {
		__spi_set_timing(CSSPI_MODE0);
	} else {
		__spi_set_timing(CSSPI_MODE3);
	}

	while (cnt < len) {
		if (spi_readl(rSR) & SPI_SR_TFNF) {
			spi_writel(rDR, data[cnt]);
		} else {
			continue;
		}

		cnt++;
	}

	/* Polling for available */
	while (spi_readl(rSR) & SPI_SR_BUSY 
	       || spi_readl(rTXFLR) & 0x1);

	/* Delay to complete */
	udelay(1);

	return cnt;
}

u32 orion_spi_simple_read(u32 reg)
{
	return spi_readl(reg);
}

u32 orion_spi_simple_write(u32 reg, u32 val)
{
	 spi_writel(reg, val);
	 return 0;
}
EXPORT_SYMBOL(spi_mode0);
EXPORT_SYMBOL(spi_mode3);

EXPORT_SYMBOL(orion_spi_simple_read);
EXPORT_SYMBOL(orion_spi_simple_write);

EXPORT_SYMBOL(orion_spi_reg_read);
EXPORT_SYMBOL(orion_spi_reg_write);

/*----------------------------------------------------------------------------*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
MODULE_AUTHOR("Sun He, <he.sun@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Serial Synchoronous Interface driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual BSD/GPL");
#endif
