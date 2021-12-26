/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

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

#include <linux/device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "orion_spidev.h"

#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_WRSR		0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(40 * HZ)	/* M25P16 specs 40s max chip erase */
#define	CMD_SIZE		4

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

struct s25f {
	u8 __iomem 		*base;
	struct semaphore	lock;
	spinlock_t 		slock;
	struct mtd_info		mtd;
	unsigned		partitioned:1;
	u8			erase_opcode;
	u8			command[CMD_SIZE + FAST_READ_DUMMY_BYTE];
};

/******************************************************************/
/* The varable definiation */
static struct s25f *flash = NULL;

/******************************************************************/
static void controller_init(void)
{

	unsigned short ctrl0 = 0;

	spi_write(rSSIENR, 0x0);
	/* Config the CTRLR0 register 		*/
	ctrl0 = 0x0 << 12  |	/* CFS 		*/
		0x0 << 11  |	/* SRL 0	*/
		0x0 << 10  |	/* SLV_OE 	*/
		0x0 << 8   |	/* TMOD master  */
		0x1 << 7   |	/* SCPOL clock	*/
		0x1 << 6   |	/* SCPH 	*/
		0x0 << 4   |	/* frame format	*/
		0x7 << 0;	/* DFS		*/

	spi_write(rCTRLR0, ctrl0);
	/* only take effect on eeprom and receive only mode */
	spi_write(rCTRLR1, 0x3ff);
	/* rBAUDR 10M */
	spi_write(rBAUDR, 0x05); 
	/* rSER, select the device */
	spi_write(rSER, 0x1);
	/* rSSIENR, enable the ssi bus */
	spi_write(rSSIENR, 0x1);
}

static void controller_speed(unsigned short baud)
{
	spi_write(rSSIENR, 0x0);
	spi_write(rBAUDR, baud); 
	spi_write(rSSIENR, 0x1);
}

static inline void 
master_set_mode(TMOD_E tmod, TIMING_E tm)
{
	unsigned short ctrl0 = spi_read(rCTRLR0);

	ctrl0 &= ~(0xf<<6);
	ctrl0 |= (tmod<<8 | tm<<6);

	spi_write(rSSIENR, 0x0);
	spi_write(rCTRLR0, ctrl0);
	spi_write(rSSIENR, 0x1);
}

static inline unsigned char 
master_check_mode(TMOD_E tmod, TIMING_E tm)
{
	unsigned short ctrl0 = spi_read(rCTRLR0);

	if (ctrl0 & (tmod<<8 | tm<<6)) 
		return 1;

	return 0;
}

#define IO_TIMEOUT_US 2000
static inline int
read_out_data(volatile unsigned char *buf, int size)
{
	int i = 0, j = 0;

	while (i < size) {
		while (!(j = spi_read(rRXFLR)));
		while (j > 0) {
			if (buf) buf[i] = spi_read(rDR);
			else     spi_read(rDR);

			i++; j--;
		}
	}

	return i;
}

static inline int
write_out_data(const unsigned char *buf, int size)
{
        int i = 0;

        while (i < size) {
                if (spi_readl(rSR) & 0x2)
			spi_writel(rDR, buf[i++]);
		else
                        while (spi_readl(rTXFLR));
        }

	return i;
}

/* Only Used For Erase Read Write */
static inline void 
flash_send_cmd(unsigned char cmd, unsigned int caddr)
{
	int i;
	unsigned char *byte_addr = (unsigned char *) &caddr;

	if (/*CMD_WRITE == cmd || */ CMD_ER_SEC == cmd || 
	    CMD_ER_BLK == cmd || CMD_ER_CHIP == cmd) {

		master_set_mode(TMOD_TXRX, TIMING3);
		if (CMD_ER_CHIP == cmd) {
			spi_write(rDR, cmd); 
			read_out_data(NULL, 1);
		} else {
			spi_write(rDR, cmd); 
			for (i = ADDRESS_LEN - 1; i >= 0; i--)
				spi_write(rDR, byte_addr[i]);
			
			/*if (CMD_WRITE != cmd)*/
				read_out_data(NULL, ADDRESS_LEN+1);
		}
		
	} else if (CMD_WREN == cmd) {

/*                master_set_mode(TMOD_TXRX, TIMING3);*/
		master_set_mode(TMOD_TXRX, TIMING0);
		spi_write(rDR, cmd); 
		read_out_data(NULL, 1);

	} else if (CMD_WRITE == cmd) {

		master_set_mode(TMOD_TX, TIMING3);
		spi_write(rDR, cmd); 
		for (i = ADDRESS_LEN - 1; i >= 0; i--)
			spi_write(rDR, byte_addr[i]);

	} else if (CMD_READ == cmd || CMD_RDID == cmd) {

		master_set_mode(TMOD_EE, TIMING3);
		if (CMD_RDID == cmd) {
			spi_write(rDR, cmd); 
		} else {
			spi_write(rDR, cmd); 
			for (i = ADDRESS_LEN - 1; i >= 0; i--)
				spi_write(rDR, byte_addr[i]);
		}

	} else {
		DEBUG(MTD_DEBUG_LEVEL2, "flash_send_cmd: Not Support this command!\n");
	}

	return;
}

/****************************************************************************/

static inline struct s25f *mtd_to_s25f(struct mtd_info *mtd)
{
	return container_of(mtd, struct s25f, mtd);
}

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static inline unsigned char read_sr(void)
{
	/* TODO: use int */
	unsigned char buf[2];

	if (! master_check_mode(TMOD_TXRX, TIMING3))
		return -1;

	spi_write(rDR, OPCODE_RDSR); spi_write(rDR, 0x0); 
	read_out_data(buf, 2);
/*        printk("status : %x\n", buf[1]);*/

	return buf[1];
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static int write_sr(u8 val)
{
	/*TODO: To be implemented! */
#if 0
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return spi_write(flash->spi, flash->command, 2);
#endif
	return 0;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline void write_enable(void)
{
	flash_send_cmd(OPCODE_WREN, 0);
}


/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
#define FLASH_MAX_TIMEOUT 	80000
static int wait_till_ready(void)
{
	int tmp = 0;

	while (read_sr() & SR_WIP) {
		mdelay(1);
		if (tmp >= FLASH_MAX_TIMEOUT)
			return 1;
		tmp++;
	}

	return 0;
}

/***************************************************************************************************/
/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(void)
{
	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %lldKiB\n",
	      "spi-flash", __func__,
	      (long long)(flash->mtd.size >> 10));

	/* Send write enable, then erase commands. */
	flash_send_cmd(OPCODE_WREN, 0);

	flash_send_cmd(OPCODE_CHIP_ERASE, 0); 

	/* Wait until finished previous write command. */
	if (wait_till_ready())
		return 1;

	return 0;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(u32 offset)
{
	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %dKiB at 0x%08x\n",
			"spi-flash", __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Send write enable, then erase commands. */
	flash_send_cmd(CMD_WREN, 0);

	/* Set up command buffer. */
	flash_send_cmd(OPCODE_SE, offset); 

	/* Wait until finished previous write command. */
	if (wait_till_ready())
		return 1;

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int s25f_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	unsigned long flags; 

	u32 addr,len;
	uint32_t rem;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%llx, len %lld\n",
	      "spi-flash", __func__, "at",
	      (long long)instr->addr, (long long)instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;

	rem = instr->len % mtd->erasesize;
	if (rem)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	down(&flash->lock);
	spin_lock_irqsave(&flash->slock, flags);

	/* whole-chip erase? */
	if (len == flash->mtd.size) {
		if (erase_chip()) {
			instr->state = MTD_ERASE_FAILED;
			spin_unlock_irqrestore(&flash->slock, flags);
			up(&flash->lock);
			return -EIO;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			if (erase_sector(addr)) {
				instr->state = MTD_ERASE_FAILED;
				spin_unlock_irqrestore(&flash->slock, flags);
				up(&flash->lock);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	spin_unlock_irqrestore(&flash->slock, flags);
	up(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int s25f_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	unsigned long flags;

	u_char *tbuf = buf;
	size_t remain = len;
	loff_t caddr = from;

	int size;
	int ret_cnt;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			"spi-flash", __func__, "from",
			(u32)from, len);

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	down(&flash->lock);
	spin_lock_irqsave(&flash->slock, flags);

	/* Set up the write data buffer. */
	while (remain > 0) {
		size = (remain >= RD_SZ) ? RD_SZ : remain;

		flash_send_cmd(OPCODE_READ, caddr);
		ret_cnt = read_out_data(tbuf, size);

		caddr += size;
		tbuf += size;
		remain -= size;
	}

	*retlen = len - remain;

	/* Wait till previous write/erase is done. 
	 *      HS: Not needed!
	 */

	spin_unlock_irqrestore(&flash->slock, flags);
	up(&flash->lock);

	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int s25f_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	unsigned long flags; 

	u32 page_offset, page_size;
	int ret_cnt;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			"spi-flash", __func__, "to",
			(u32)to, len);

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > flash->mtd.size)
		return -EINVAL;

	controller_speed(DEF_W_SPEED);

	down(&flash->lock);
	spin_lock_irqsave(&flash->slock, flags);

	flash_send_cmd(OPCODE_WREN, 0);

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) {

		flash_send_cmd(OPCODE_PP, to);
		ret_cnt = write_out_data(buf, len);

		*retlen = ret_cnt;
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		flash_send_cmd(OPCODE_PP, to);
		ret_cnt = write_out_data(buf, page_size);

		*retlen = ret_cnt;

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			/*TODO*/
/*                        wait_till_ready(flash);*/
/*                        udelay(PP_TIME_US);*/
			mdelay(2);

			flash_send_cmd(OPCODE_WREN, 0);

			/* write the next page to flash */
			flash_send_cmd(OPCODE_PP, (to+i));
			ret_cnt = write_out_data((buf+i), page_size);

			if (retlen)
				*retlen += ret_cnt;
		}
	}

	/* Wait until finished previous write command. */
	/* TODO: To be implemented, use status method */
/*        udelay(PP_TIME_US);*/
	mdelay(2);

	spin_unlock_irqrestore(&flash->slock, flags);
	up(&flash->lock);

	controller_speed(DEF_W_SPEED);

	return 0;
}

/****************************************************************************/
/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;
	u16             ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16		n_sectors;

	u16		flags;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
};


/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata s25f_data [] = {

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 0, 32 * 1024, 4, SECT_4K, },
	{ "at25fs040",  0x1f6604, 0, 64 * 1024, 8, SECT_4K, },

	{ "at25df041a", 0x1f4401, 0, 64 * 1024, 8, SECT_4K, },
	{ "at25df641",  0x1f4800, 0, 64 * 1024, 128, SECT_4K, },

	{ "at26f004",   0x1f0400, 0, 64 * 1024, 8, SECT_4K, },
	{ "at26df081a", 0x1f4501, 0, 64 * 1024, 16, SECT_4K, },
	{ "at26df161a", 0x1f4601, 0, 64 * 1024, 32, SECT_4K, },
	{ "at26df321",  0x1f4701, 0, 64 * 1024, 64, SECT_4K, },

	/* Macronix */
	{ "mx25l12805d", 0xc22018, 0, 64 * 1024, 256, },

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 0, 64 * 1024, 8, },
	{ "s25sl008a", 0x010213, 0, 64 * 1024, 16, },
	{ "s25sl016a", 0x010214, 0, 64 * 1024, 32, },
	{ "s25sl032a", 0x010215, 0, 64 * 1024, 64, },
	{ "s25sl064a", 0x010216, 0, 64 * 1024, 128, },
        { "s25sl12800", 0x012018, 0x0300, 256 * 1024, 64, },
	{ "s25sl12801", 0x012018, 0x0301, 64 * 1024, 256, },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 0, 64 * 1024, 8, SECT_4K, },
	{ "sst25vf080b", 0xbf258e, 0, 64 * 1024, 16, SECT_4K, },
	{ "sst25vf016b", 0xbf2541, 0, 64 * 1024, 32, SECT_4K, },
	{ "sst25vf032b", 0xbf254a, 0, 64 * 1024, 64, SECT_4K, },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  0, 32 * 1024, 2, },
	{ "m25p10",  0x202011,  0, 32 * 1024, 4, },
	{ "m25p20",  0x202012,  0, 64 * 1024, 4, },
	{ "m25p40",  0x202013,  0, 64 * 1024, 8, },
	{ "m25p80",         0,  0, 64 * 1024, 16, },
	{ "m25p16",  0x202015,  0, 64 * 1024, 32, },
	{ "m25p32",  0x202016,  0, 64 * 1024, 64, },
	{ "m25p64",  0x202017,  0, 64 * 1024, 128, },
	{ "m25p128", 0x202018, 0, 256 * 1024, 64, },

	{ "m45pe10", 0x204011,  0, 64 * 1024, 2, },
	{ "m45pe80", 0x204014,  0, 64 * 1024, 16, },
	{ "m45pe16", 0x204015,  0, 64 * 1024, 32, },

	{ "m25pe80", 0x208014,  0, 64 * 1024, 16, },
	{ "m25pe16", 0x208015,  0, 64 * 1024, 32, SECT_4K, },

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", 0xef3011, 0, 64 * 1024, 2, SECT_4K, },
	{ "w25x20", 0xef3012, 0, 64 * 1024, 4, SECT_4K, },
	{ "w25x40", 0xef3013, 0, 64 * 1024, 8, SECT_4K, },
	{ "w25x80", 0xef3014, 0, 64 * 1024, 16, SECT_4K, },
	{ "w25x16", 0xef3015, 0, 64 * 1024, 32, SECT_4K, },
	{ "w25x32", 0xef3016, 0, 64 * 1024, 64, SECT_4K, },
	{ "w25x64", 0xef3017, 0, 64 * 1024, 128, SECT_4K, },

	{ "s25fl008a", 0x1c2017, 0, 64 * 1024, 128, },
	{ "mx25l6445e", 0xc22017, 0, 64 * 1024, 128, },
};

static int get_manufacture_id(unsigned char *buf, int size)
{
	flash_send_cmd(CMD_RDID, 0);
	read_out_data(buf, size);
#if 0
	printk("Manufacture ID: 0x%x\n", buf[0]);
	printk("Device ID 1: 	0x%x\n", buf[1]);
	printk("Device ID 2: 	0x%x\n", buf[2]);
#endif
	return 0;
}

static struct flash_info *__devinit jedec_probe(void)
{
	int			tmp;
	u8			id[5];
	u32			jedec;
	u16                     ext_jedec;
	struct flash_info	*info;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	tmp = get_manufacture_id(id, 5);
	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			"spi-flash", tmp);
		return NULL;
	}
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	ext_jedec = id[3] << 8 | id[4];

	for (tmp = 0, info = s25f_data;
			tmp < ARRAY_SIZE(s25f_data);
			tmp++, info++) {
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return info;
		}
	}
	DEBUG(MTD_DEBUG_LEVEL2, "unrecognized JEDEC id %06x\n", jedec);

	return NULL;
}

/***************************************************************************************************/

#define NB_OF(x) (sizeof (x) / sizeof (x[0]))

static struct mtd_erase_region_info erase_regions[] = {
	/* parameter blocks */
	{
		.offset		= 0x00000000,
		.erasesize	= 4 * 1024,
		.numblocks	= 2,
	},
	/* main blocks */
	{
		.offset	 	= 0x00002000,
		.erasesize	= 8 * 1024,
		.numblocks	= 1,
	},
	{
		.offset	 	= 0x00004000,
		.erasesize	= 16 * 1024,
		.numblocks	= 1,
	},
	{
		.offset	 	= 0x00008000,
		.erasesize	= 32 * 1024,
		.numblocks	= 1,
	},
	{
		.offset	 	= 0x00010000,
		.erasesize	= 64 * 1024,
		.numblocks	= 127,
	}
};

#ifdef CONFIG_MTD_PARTITIONS
/*
 * Define static partitions for flash device
 */
#define NUM_PARTITIONS 3
static struct mtd_partition spi_partition_info[] = {
    { .name   = "spi partition0",
      .offset = 0x0,
      .size   = 0x100000 },
   
    { .name   = "spi partition1",
      .offset = 0x100000,
      .size   = 0x400000 },
  
    { .name   = "spi partition2",
      .offset = 0x500000,
      .size   = 0x300000 }
};
#endif

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit s25f_probe(struct device *dev)
{
	struct flash_info		*info;
	unsigned			i;

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash) {
		return -ENOMEM; 
	}

	if (!request_mem_region(ORION_SPI_BASE, 
				ORION_SPI_SIZE, "orion_spi")) {
		kfree(flash); flash = NULL;
		return -ENOMEM;
	}

	flash->base = ioremap(ORION_SPI_BASE, ORION_SPI_SIZE);
	if (!flash->base) {
		release_mem_region(ORION_SPI_BASE, ORION_SPI_SIZE);
		return -EIO;
	}

	/* Config with default value */
	controller_init();

	info = jedec_probe();
	if (!info)
		return -ENODEV;

	init_MUTEX(&flash->lock);
	spin_lock_init(&flash->slock);

	/*
	 * Atmel serial flash tend to power up
	 * with the software protection bits set
	 */

	if (info->jedec_id >> 16 == 0x1f) {
		write_enable();
		write_sr(0);
	}

	flash->mtd.name = "s25f";
	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;

	flash->mtd.erase = s25f_erase;
	flash->mtd.read = s25f_read;
	flash->mtd.write = s25f_write;

#if 1
	/* HS: Newly added */
	if (!strcmp(info->name, "s25fl008a")) {
		flash->mtd.numeraseregions = NB_OF (erase_regions);
		flash->mtd.eraseregions = erase_regions;
	}
#endif

	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	DEBUG(MTD_DEBUG_LEVEL2, "%s (%lld Kbytes)\n", info->name,
			(long long)flash->mtd.size >> 10);

	DEBUG(MTD_DEBUG_LEVEL2,
		"mtd .name = %s, .size = 0x%llx (%lldMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		(long long)flash->mtd.size, (long long)(flash->mtd.size >> 20),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	/* HS */
#if 0
	{
		erase_chip();	
/*                erase_sector(0x400);*/
		return NULL;
	}
#endif
	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL2,
/*                        printk(*/
				"mtd.eraseregions[%d] = { .offset = 0x%llx, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (long long)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);


	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	{
		struct mtd_partition	*parts = NULL;
		int			nr_parts = 0;


#ifdef CONFIG_MTD_CMDLINE_PARTS
		static const char *part_probes[]
			= { "cmdlinepart", NULL, };

		nr_parts = parse_mtd_partitions(&flash->mtd,
						part_probes, &parts, 0);

#endif

		if (nr_parts <= 0) {
			parts = spi_partition_info;
			nr_parts = NUM_PARTITIONS;
		}

		if (nr_parts > 0) {
			for (i = 0; i < nr_parts; i++) {
				DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
					"{.name = %s, .offset = 0x%llx, "
						".size = 0x%llx (%lldKiB) }\n",
					i, parts[i].name,
					(long long)parts[i].offset,
					(long long)parts[i].size,
					(long long)(parts[i].size >> 10));
			}
			flash->partitioned = 1;
			return add_mtd_partitions(&flash->mtd, parts, nr_parts);
		}
	} 

	return add_mtd_device(&flash->mtd) == 1 ? -ENODEV : 0;
}

static int __devexit s25f_remove(struct device *dev)
{
	int		status;

	if (flash == NULL) {
		DEBUG(MTD_DEBUG_LEVEL2, "%s: flash resources already freed!\n", __func__);
		return -EINVAL;
	}

	/* Clean up MTD stuff. */
	if (flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);

	iounmap((void *)flash->base);
	release_mem_region(ORION_SPI_BASE, ORION_SPI_SIZE);
	kfree(flash);

	return 0;
}

/********************************************************************************/
static struct device_driver s25f_driver = {
	.name           = "s25fl008a",
	.bus            = &platform_bus_type,
	.probe          = s25f_probe,
	.remove         = s25f_remove,
};

static struct platform_device s25f_platform_device = {
	.name   = "s25fl008a",
	.id     = 0,
};

static int s25f_init(void)
{
	int ret;

	ret = driver_register(&s25f_driver);
	if (!ret) {
		ret = platform_device_register(&s25f_platform_device);
		if (ret)
			driver_unregister(&s25f_driver);
        }

	return 0;
}

static void s25f_exit(void)
{
	 platform_device_unregister(&s25f_platform_device);
	 driver_unregister(&s25f_driver);
}

module_init(s25f_init);
module_exit(s25f_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sun He");
MODULE_DESCRIPTION("MTD SPI driver for Winbond S25Fxxx flash chips");

