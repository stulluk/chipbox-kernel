
#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/delay.h>
#include <linux/init.h>

#include <asm/io.h>
#include <asm/hardware/clock.h>
#include <asm/mach-types.h>

#include "palm_bk3710.h"
#include "ide-timing.h"

MODULE_AUTHOR("Zhongkai Du, <zhongkai.du@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor IDE Controller driver");
MODULE_LICENSE("GPL");

/*
 * Standard (generic) timings for Taskfile modes, from ATA2 specification.
 * Some drives may specify a mode, while also specifying a different
 * value for cycle_time (from drive identification data).
 */
const palm_bk3710_piotiming palm_bk3710_tasktimings[] = {
	{290, 600},	/* PIO Mode 0 */
	{290, 383},	/* PIO Mode 1 */
	{290, 240},	/* PIO Mode 2 */
	{290, 240},	/* PIO Mode 2 */
	{290, 240}	/* PIO Mode 2 */
};

/*
 * Standard (generic) timings for PIO modes, from ATA2 specification.
 * Some drives may specify a mode, while also specifying a different
 * value for cycle_time (from drive identification data).
 */
const palm_bk3710_piotiming palm_bk3710_piotimings[] = {
	{165, 600},	/* PIO Mode 0 */
	{125, 383},	/* PIO Mode 1 */
	{100, 240},	/* PIO Mode 2 */
	{80, 180},	/* PIO Mode 3 with IORDY */
	{70, 120}	/* PIO Mode 4 with IORDY */
};

/*
 * Standard (generic) timings for DMA modes, from ATA2 specification.
 * Some drives may specify a mode, while also specifying a different
 * value for cycle_time (from drive identification data).
 */
const palm_bk3710_dmatiming palm_bk3710_dmatimings[] = {
	{215, 215, 480},/* DMA Mode 0 */
	{80,  50,  150},/* DMA Mode 1 */
	{70,  25,  120} /* DMA Mode 2 */
};

/*
 * Standard (generic) timings for UDMA modes, from ATA2 specification.
 * Some drives may specify a mode, while also specifying a different
 * value for cycle_time (from drive identification data).
 */
const palm_bk3710_udmatiming palm_bk3710_udmatimings[] = {
	{45, 160, 114},	/* UDMA Mode 0 */
	{45, 125, 75}, 	/* UDMA Mode 1 */
	{45, 100, 55},	/* UDMA Mode 2 */
	{20, 100, 39}, 	/* UDMA Mode 3 */
	{20, 85,  25}, 	/* UDMA Mode 4 */
	{20, 85,  17}	/* UDMA Mode 5 */
};

struct ide_pci_device_s  palm_bk3710_dummydata;
static ide_hwif_t	*palm_bk3710_hwif = NULL;
palm_bk3710_ideregs	*palm_bk3710_base = NULL;

// DELME static spinlock_t palm_bk3710_spinlock = SPIN_LOCK_UNLOCKED;

#if defined(PALM_BK3710_DEBUG)
#define DBG(fmt, arg...)	printk(fmt, ##arg)
#else
#define DBG(fmt, arg...) 
#endif

static int palm_bk3710_chipinit (void);
static int palm_bk3710_setdmamode (palm_bk3710_ideregs *, unsigned int, unsigned int);
static int palm_bk3710_setpiomode (palm_bk3710_ideregs *, unsigned int, unsigned int);
static void palm_bk3710_tune_drive (ide_drive_t*, u8);

/*
 * Set the device UDMA mode on Palm Chip 3710
 *
 * In: 
 *   handle, IDE Controller info
 *   dev, drive to tune
 *   level, desired level
 * 
 * Out:
 *   level in UDMA Mode
 */
static int palm_bk3710_setudmamode (palm_bk3710_ideregs *handle, unsigned int dev, unsigned int level)
{
	char is_slave = (dev == 1) ? 1 : 0;

	if (!is_slave) {
		/* setup master device parameters */

		/* udmatim Register */
		palm_bk3710_base->config.udmatim &= 0xFFF0;
		palm_bk3710_base->config.udmatim |= level;

		/* Enable UDMA for Device 0 */
		palm_bk3710_base->config.udmactl |= 1;
	} else {
		/* setup slave device parameters */

		/* udmatim Register */
		palm_bk3710_base->config.udmatim &= 0xFF0F;
		palm_bk3710_base->config.udmatim |= (((unsigned int)level) << 4);

		/* Enable UDMA for Device 0 */
		palm_bk3710_base->config.udmactl |= (1 << 1);
	}

	DBG("%s: udmatim=0x%x udmactl=0x%x \n",
		__FUNCTION__, 
		palm_bk3710_base->config.udmatim,
		palm_bk3710_base->config.udmactl);

	return level;
}

/*
 * Set the device DMA mode on Palm Chip 3710.
 *
 * In:
 *   handle, IDE Controller info
 *   dev, drive to tune
 *   level, desired level
 *
 * Out:
 *   level in DMA Mode
 */
static int palm_bk3710_setdmamode(palm_bk3710_ideregs * handle, unsigned int dev, unsigned int mode)
{
	char is_slave = (dev == 1) ? 1 : 0;

	if (!is_slave) {
		/* setup master device parameters */
		palm_bk3710_base->dmaengine.bmisp |= 0x20;

		/* Disable UDMA for Device 0 */
		palm_bk3710_base->config.udmactl &= 0xFF02;
	} else {
		/* setup slave device parameters */
		palm_bk3710_base->dmaengine.bmisp |= 0x40;

		/* Disable UDMA for Device 1 */
		palm_bk3710_base->config.udmactl &= 0xFF01;
	}

	DBG("%s: bmisp=0x%x udmactl=0x%x \n",
		__FUNCTION__, 
		palm_bk3710_base->dmaengine.bmisp,
		palm_bk3710_base->config.udmactl);

	return mode;
}

/*
 * Set the device PIO mode on Palm Chip 3710.
 *
 * In:
 *   handle, IDE Controller info
 *   dev, drive to tune
 *   level, desired level
 *
 * Out:
 *   level in PIO mode
 */
static int palm_bk3710_setpiomode (palm_bk3710_ideregs *handle, unsigned int dev, unsigned int mode)
{
	int is_slave = (dev == 1) ? 1 : 0;

	if (!is_slave) {
		/* setup master device parameters */
		palm_bk3710_base->config.idetimp |= 0xb301; 

		/* Disable UDMA for Device 0 */
		palm_bk3710_base->config.udmactl &= 0xFF02;

	} else {
		/* setup slave device parameters */
		palm_bk3710_base->config.idetimp |= 0xb310;
 
		/* Disable UDMA for Device 1 */
		palm_bk3710_base->config.udmactl &= 0xFF01;
	}

	DBG("%s: idetimp=0x%x udmactl=0x%x \n",
		__FUNCTION__, 
		palm_bk3710_base->config.idetimp,
		palm_bk3710_base->config.udmactl);

	return mode;
}

/*
 * Set a Palm Chip 3710 interface channel to the desired speeds. This involves
 * requires the right timing data into the 3710 timing override registers.
 */
static int palm_bk3710_hostdma (ide_drive_t *drive, u8 xferspeed)
{
	ide_hwif_t *hwif = HWIF(drive);
	int nspeed = -1, is_slave = (&hwif->drives[1] == drive);
	unsigned char speed = (XFER_UDMA_5 < xferspeed) ? XFER_UDMA_5 : xferspeed;

	DBG("%s: Set DMA Mode %u\n", __FUNCTION__, xferspeed);

	switch(speed) {
		case XFER_UDMA_5: nspeed = 1; break;
		case XFER_UDMA_4: nspeed = 2; break;
		case XFER_UDMA_3: nspeed = 3; break;
		case XFER_UDMA_2: nspeed = 4; break;
		case XFER_UDMA_1: nspeed = 5; break;
		case XFER_UDMA_0: nspeed = 6; break;
		case XFER_MW_DMA_2: nspeed = 8; break;
		case XFER_MW_DMA_1: nspeed = 9; break;
		case XFER_MW_DMA_0: nspeed = 10; break;
		
		default: return -1;
	}

	if (nspeed != -1) {
		if ((speed <= XFER_UDMA_5) && (speed >= XFER_UDMA_0)) {
			palm_bk3710_setudmamode(NULL, is_slave, 6 - nspeed);
		} else {
			palm_bk3710_setdmamode(NULL, is_slave, 10 - nspeed);
		}

		return (ide_config_drive_speed(drive, speed));
	}
	
	return 0;
}

/*
 * Set up a Palm Chip 3710 interface channel for the best available speed.
 * We prefer UDMA if it is available and then MWDMA. If DMA is
 * not available we switch to PIO and return 0.
 */
static inline int palm_bk3710_drivedma(ide_drive_t * pDrive)
{
	u8 speed = ide_dma_speed(pDrive, 2/*FIXME@ maybe a higher value is better, it depends on the implementation of hardware */);

	/* If no DMA/single word DMA was available or the chipset has DMA bugs
	   then disable DMA and use PIO */
	if (!speed) {
		palm_bk3710_tune_drive(pDrive, 255);
		return 0;
	}
	
	palm_bk3710_hostdma(pDrive, speed);	
	
	return ide_dma_enable(pDrive);
}

/*
 * Set up the Palm Chip 3710 interface for the best available speed on this
 * interface, preferring DMA to PIO.
 */
static int palm_bk3710_checkdma (ide_drive_t *drive)
{
	ide_hwif_t *hwif = HWIF(drive);
	struct hd_driveid *id = drive->id;

	drive->init_speed = 0;

	if ((id->capability & 1) && drive->autodma) {
		if (id->field_valid & 4) {
			if (id->dma_ultra & hwif->ultra_mask) {
				/* Force if Capable UltraDMA */
				if ((id->field_valid & 2) && (!palm_bk3710_drivedma(drive)))
					goto try_dma_modes;
			}
		} else if (id->field_valid & 2) {
try_dma_modes:
			if (id->dma_mword & hwif->mwdma_mask) {
				/* Force if Capable regular DMA modes */
				if (!palm_bk3710_drivedma(drive))
					goto no_dma_set;
			}
		} else {
			goto fast_ata_pio;
		}
		return hwif->ide_dma_on(drive);
		
	} else if ((id->capability & 8) || (id->field_valid & 2)) {
no_dma_set:
fast_ata_pio:
		hwif->tuneproc(drive, 255);
		return hwif->ide_dma_off_quietly(drive);
	}

	return 0;
}

/*
 * Tune a drive attached to a Palm Chip 3710, and then set the interface 
 * and device PIO mode
 */
static void palm_bk3710_tune_drive (ide_drive_t *drive, u8 pio)
{
	ide_hwif_t *hwif = HWIF(drive);
	ide_pio_data_t piodata;
	int is_slave = (&hwif->drives[1] == drive);

	/* 
	 * Get the best PIO Mode supported by the drive
	 * Obtain the drive PIO data for tuning the Palm Chip registers
	 */
	ide_get_best_pio_mode (drive, pio, 5, &piodata);
	
	DBG("%s: best_pio_mode=%u use_iordy=%u overridden=%u blk=%u cycle=%u \n",
		__FUNCTION__, 
		piodata.pio_mode,
		piodata.use_iordy,
		piodata.overridden,
		piodata.blacklisted,
		piodata.cycle_time);

	/* Check for IORDY here */
	if (piodata.cycle_time < ide_pio_timings[piodata.pio_mode].cycle_time)
	{
		piodata.cycle_time = ide_pio_timings[piodata.pio_mode].cycle_time;
	}
	
	palm_bk3710_setpiomode (NULL, is_slave, piodata.pio_mode);
}

#if defined(CONFIG_ARCH_ORION_CSM1200)

#define PIN_MUX_CONF() do { \
        volatile unsigned long *pinmux_base;                                    \
        pinmux_base = (volatile unsigned long *)ioremap(0x10171000, 0x1000);    \
        pinmux_base[0x400 >> 2] |= 0x00000010;			                \
        udelay(20);                                                             \
        iounmap((void *)pinmux_base);                                           \
} while(0)

#elif defined(CONFIG_ARCH_ORION_CSM1201)

#define PIN_MUX_CONF() do { \
        volatile unsigned long *pinmux_base;                                    \
        pinmux_base = (volatile unsigned long *)ioremap(0x10171000, 0x1000);    \
        pinmux_base[0x400 >> 2] |= 0x00000030;			                \
        udelay(20);                                                             \
        iounmap((void *)pinmux_base);                                           \
        pinmux_base = (volatile unsigned long *)ioremap(0x10260000, 0x1000);    \
        pinmux_base[0x030 >> 2] &= 0xffff0000;			                \
        udelay(20);                                                             \
        iounmap((void *)pinmux_base);                                           \
} while(0)

#else /* csm1100 */

#define PIN_MUX_CONF() do {} while(0)

#endif

/*
 * Initialize the Palm Chip 3710 IDE controller to default conditions.
 */
int __init palm_bk3710_init(void)
{
	int ret = 0;
	int index = 0;
	hw_regs_t ide_ctlr_info;

	ret = -EIO;
	if (!request_mem_region(IDE_PALM_REG_MMAP_BASE, 0x80, "Orion IDE control and config"))
		goto ERR_MEMREQ;

	ret = -EIO;
	if (NULL == (palm_bk3710_base = ioremap(IDE_PALM_REG_MMAP_BASE, 0x80)))
		goto ERR_MEMMAP;
	
	printk ("Palm Chip BK3710 IDE Initializing: base 0x%08x\n", (unsigned int)palm_bk3710_base);

	memset(&ide_ctlr_info, 0, sizeof(ide_ctlr_info));

	PIN_MUX_CONF();

	/* Configure the Palm Chip controller */
	palm_bk3710_chipinit();

	for (index = 0; index < IDE_NR_PORTS - 2; index++) {
		ide_ctlr_info.io_ports[index] = (unsigned long)(palm_bk3710_base) + IDE_PALM_ATA_PRI_REG_OFFSET + index;
	}

	ide_ctlr_info.io_ports[IDE_CONTROL_OFFSET] = (unsigned long)(palm_bk3710_base) + IDE_PALM_ATA_PRI_CTL_OFFSET;
	ide_ctlr_info.irq = IDE_PALM_IRQ;
	ide_ctlr_info.chipset = ide_palm3710;
	ide_ctlr_info.ack_intr = NULL;
	
	ret = -ENODEV;
	if (ide_register_hw(&ide_ctlr_info, &palm_bk3710_hwif) < 0) {
		printk(" Palm Chip BK3710 IDE Register Fail \n");
		goto ERR_REGISTER_IDE;
	}

	palm_bk3710_hwif->tuneproc = &palm_bk3710_tune_drive;
	palm_bk3710_hwif->noprobe = 0;

	/* Just put this for using the ide-dma.c init code */
	palm_bk3710_hwif->speedproc = &palm_bk3710_hostdma;
	palm_bk3710_dummydata.extra = 0;
	palm_bk3710_hwif->cds = &palm_bk3710_dummydata;

	/* Setup up the memory map base for this instance of hwif */
	palm_bk3710_hwif->mmio = 0;
	palm_bk3710_hwif->ide_dma_check = palm_bk3710_checkdma;
	palm_bk3710_hwif->ultra_mask = 0x7f; /* Ultra DMA Mode 6 Max */

	palm_bk3710_hwif->mwdma_mask = 0x7;
	palm_bk3710_hwif->swdma_mask = 0;
	palm_bk3710_hwif->dma_command = (unsigned long)(palm_bk3710_base);
	palm_bk3710_hwif->dma_status = (unsigned long)(palm_bk3710_base) + 2;
	palm_bk3710_hwif->dma_prdtable = (unsigned long)(palm_bk3710_base) + 4;

	palm_bk3710_hwif->drives[0].autodma = 1;
	palm_bk3710_hwif->drives[1].autodma = 1;
	
	ide_setup_dma(palm_bk3710_hwif, (unsigned long)palm_bk3710_base, 8);

#ifdef MODULE
	palm_bk3710_checkdma (&palm_bk3710_hwif->drives[0]);
	palm_bk3710_checkdma (&palm_bk3710_hwif->drives[1]);
#endif

	/* to inform GPIO driver that this driver will hold some GPIOs. */
	//extern int orion_gpio_register_module_status(const char * module, unsigned int  orion_module_status); /* status: 0 disable, 1 enable */
	//orion_gpio_register_module_status("PATA", 1);

	return 0; 

ERR_REGISTER_IDE:
	iounmap((void*)palm_bk3710_base);
ERR_MEMMAP:
	release_mem_region (IDE_PALM_REG_MMAP_BASE, 0x80);
ERR_MEMREQ:

	return ret;
}

/*
 * Configures the Palm Chip Controller in the desired default operating mode
 */
int palm_bk3710_chipinit (void)
{
	int atacount = 0;

	/* enable the reset_en of ATA controller so that when ata signals are brought
	 * out , by writing into device config. at that time por_n signal should not be
	 * 'Z' and have a stable value.
	 */
	palm_bk3710_base->config.miscctl = 0x0300;

	/* wait for some time and deassert the reset of ATA Device. */
	for(atacount = 0; atacount < 5; atacount++);

	/* Deassert the Reset */
	palm_bk3710_base->config.miscctl = 0x0200;

	/* 
	 * Program the IDETIMP Register Value based on the following assumptions
	 *
	 * (ATA_IDETIMP_IDEEN      , ENABLE ) |
	 * (ATA_IDETIMP_SLVTIMEN   , ENABLE) |
	 * (ATA_IDETIMP_RDYSMPL    , 70NS)    |
	 * (ATA_IDETIMP_RDYRCVRY   , 50NS)    |
	 * (ATA_IDETIMP_DMAFTIM1   , PIOCOMP) |
	 * (ATA_IDETIMP_PREPOST1   , ENABLE) |
	 * (ATA_IDETIMP_RDYSEN1    , ENABLE) |
	 * (ATA_IDETIMP_PIOFTIM1   , ENABLE) |
	 * (ATA_IDETIMP_DMAFTIM0   , PIOCOMP) |
	 * (ATA_IDETIMP_PREPOST0   , ENABLE) |
	 * (ATA_IDETIMP_RDYSEN0    , ENABLE) |
	 * (ATA_IDETIMP_PIOFTIM0   , ENABLE)
	 */
	palm_bk3710_base->config.idetimp = 0xb3ff;

	/* 
	 * Configure  SIDETIM  Register
	 * (ATA_SIDETIM_RDYSMPS1   ,120NS ) |
	 * (ATA_SIDETIM_RDYRCYS1   ,120NS )
	 */
	palm_bk3710_base->config.sidetim = 0;

	/* 
	 * UDMACTL Ultra-ATA DMA Control
	 * (ATA_UDMACTL_UDMAP1     ,0) |
	 * (ATA_UDMACTL_UDMAP0     ,0)
	 */
	palm_bk3710_base->config.udmactl = 0;

	/* 
	 * MISCCTL Miscellaneous Conrol Register
	 * (ATA_MISCCTL_RSTMODEP   ,1) |
	 * (ATA_MISCCTL_RESETP     ,0) |
	 * (ATA_MISCCTL_TIMORIDE   ,1)
	 */
	palm_bk3710_base->config.miscctl = 0x201;

	/* 
	 * IORDYTMP IORDY Timer for Primary Register
	 * (ATA_IORDYTMP_IORDYTMP  ,0xffff)
	 */
	palm_bk3710_base->config.iordytmp = 0xffff;

	/*
	 * Configure BMISP Register
	 * (ATA_BMISP_DMAEN1     ,DISABLE) |
	 * (ATA_BMISP_DMAEN0	 ,DISABLE) |
	 * (ATA_BMISP_IORDYINT	 ,CLEAR) |
	 * (ATA_BMISP_INTRSTAT	 ,CLEAR) |
	 * (ATA_BMISP_DMAERROR	 ,CLEAR)
	 */
	palm_bk3710_base->dmaengine.bmisp = 0;

	palm_bk3710_setpiomode (NULL, 0, 0);
	palm_bk3710_setpiomode (NULL, 1, 0);

	return 1;
}

#ifdef MODULE

static void __exit palm_bk3710_exit (void)
{
	palm_bk3710_hwif = NULL;
	ide_unregister (0);
}

module_init(palm_bk3710_init);
module_exit(palm_bk3710_exit);

#endif

