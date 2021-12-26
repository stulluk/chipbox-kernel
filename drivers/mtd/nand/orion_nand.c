/*
 *  drivers/mtd/nand/orion_nand.c
 *
 *  Copyrigth (C) 2007 Celestial Semiconductor
 *  
 *
 *  Derived from drivers/mtd/nand/h1910.c
 *       Copyright (C) 2003 Joshua Wise (joshua@joshuawise.com)
 *
 * $Id: orion_nand.c,v 1.0 2007/08/02   $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This is a device driver for the NAND flash device found on the
 *   Celestial CSM1200 SOC EVB board. 
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/sizes.h>


#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)
#define PA_SMC_BASE         0x10100000

#ifdef CONFIG_MTD_NAND_ORION_NANDBOOT
   #define CE_CFG_0            0x10100000  	
#else
   #define CE_CFG_0            0x1010000C
#endif
   #define PA_SMC_STA          0x10100030
   #define PA_SMC_BOOT         0x10100034

static void __iomem *smc_sta_addr = NULL;
static void __iomem *smc_cfg_addr = NULL;
#endif 

#ifdef CONFIG_MTD_NAND_ORION_NANDBOOT
#define PA_ORION_NAND_ADDR 0x34000000
#else
#define PA_ORION_NAND_ADDR 0x34800000
#endif

/*
 * MTD structure for CSM1100/CSM1200 EVB  board
 */
static struct mtd_info *orion_nand_mtd = NULL;

/*
 * Module stuff
 */



#ifdef CONFIG_MTD_PARTITIONS
/*
 * Define static partitions for flash device
 */
static struct mtd_partition partition_info[] = {
	{ .name   = "nand partition0",
      .offset = 0,
      .size   = 1024*1024 },
   
    { .name   = "nand partition1",
      .offset = MTDPART_OFS_NXTBLK,
      .size   = 4*1024*1024 },
  
    { .name   = "nand partition2",
      .offset = MTDPART_OFS_NXTBLK,
      .size   = 4*1024*1024 },
  
    { .name   = "nand partition3",
      .offset = MTDPART_OFS_NXTBLK,
      .size   = 10*1024*1024 },

    { .name   = "nand partition4",
      .offset = MTDPART_OFS_NXTBLK,
      .size   = MTDPART_SIZ_FULL }
};

#define NUM_PARTITIONS 5

#endif


static int orion_device_ready(struct mtd_info *mtd)
{
#if defined( CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201) 
	
    if (readw(smc_sta_addr) & 0x1){
        return 1;
    }
    else {
        return 0;
    }
#else 

    return 1;

#endif
	
}

/* 
 *	hardware specific access to control-lines
 */
static void orion_hwcontrol(struct mtd_info *mtd, int cmd) 
{
	struct nand_chip* this = (struct nand_chip *) (mtd->priv);
	unsigned int value ;
	switch(cmd) {
		
	case NAND_CTL_SETCLE: 
		this->IO_ADDR_R = (void __iomem *)((unsigned int) this->IO_ADDR_R | 0x2); //(1 << 1);
		this->IO_ADDR_W = (void __iomem *)((unsigned int) this->IO_ADDR_W | 0x2); //(1 << 1);
		break;
	case NAND_CTL_CLRCLE: 
		this->IO_ADDR_R = (void __iomem *)((unsigned int) this->IO_ADDR_R & (~(1<<1)));
		this->IO_ADDR_W = (void __iomem *)((unsigned int) this->IO_ADDR_W & (~(1<<1)));
		break;
		
	case NAND_CTL_SETALE:
		this->IO_ADDR_R = (void __iomem *)((unsigned int) this->IO_ADDR_R | (1 << 2));
		this->IO_ADDR_W = (void __iomem *)((unsigned int) this->IO_ADDR_W | (1 << 2));
		break;
	case NAND_CTL_CLRALE:
		this->IO_ADDR_R = (void __iomem *)((unsigned int) this->IO_ADDR_R & (~(1<<2)));
		this->IO_ADDR_W = (void __iomem *)((unsigned int) this->IO_ADDR_W & (~(1<<2)));
		break;
		
	case NAND_CTL_SETNCE:
        value = *((unsigned int *)smc_cfg_addr);
        *((unsigned int *)smc_cfg_addr) = value | 0x200;
		break;
	case NAND_CTL_CLRNCE:
        value = *((unsigned int *)smc_cfg_addr);
        *((unsigned int *)smc_cfg_addr) = value & (~((unsigned int)0x200));

		break;
	}
}


/*
 * Main initialization routine
 */
static int __init orion_nand_init (void)
{
	struct nand_chip *this;
	const char *part_type = 0;
	int mtd_parts_nb = 0;
	struct mtd_partition *mtd_parts = 0;
	void __iomem *nandaddr;
   
#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)    
    void __iomem *smc_boot_addr; 
#endif


#ifdef CONFIG_MTD_CMDLINE_PARTS

extern int parse_cmdline_partitions(struct mtd_info *master, struct mtd_partition **pparts, char *);
#endif


	nandaddr = ioremap(PA_ORION_NAND_ADDR, 0x100);
	if (!nandaddr) {
		printk("Failed to ioremap nand flash.\n");
		return -ENOMEM;
	}

#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)

	smc_cfg_addr = ioremap(CE_CFG_0, 0x4);
	if (!smc_cfg_addr) {
		printk("Failed to ioremap flash controler.\n");
		return -ENOMEM;
	}

	smc_sta_addr = ioremap(PA_SMC_STA, 0x4);
	if (!smc_sta_addr) {
		printk("Failed to ioremap flash controler.\n");
		return -ENOMEM;
	}

	smc_boot_addr = ioremap(PA_SMC_BOOT, 0x4);
	if (!smc_boot_addr) {
		printk("Failed to ioremap flash controler.\n");
		return -ENOMEM;
	}
#endif
	
	/* Allocate memory for MTD device structure and private data */
	orion_nand_mtd = kmalloc(sizeof(struct mtd_info) + 
			     sizeof(struct nand_chip),
			     GFP_KERNEL);
	if (!orion_nand_mtd) {
		printk("Unable to allocate orion NAND MTD device structure.\n");
		iounmap ((void *) nandaddr);
#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)
        iounmap ((void *) smc_cfg_addr);
        iounmap ((void *) smc_sta_addr);
        iounmap ((void *) smc_boot_addr);
#endif
		return -ENOMEM;
	}

 /* Get pointer to private data */
	this = (struct nand_chip *) (&orion_nand_mtd[1]);
	
	/* Initialize structures */
	memset((char *) orion_nand_mtd, 0, sizeof(struct mtd_info));
	memset((char *) this, 0, sizeof(struct nand_chip));
	
	/* Link the private data with the MTD structure */
	orion_nand_mtd->priv = this;
	orion_nand_mtd->name ="ORION";
	
	/* insert callbacks */
	this->IO_ADDR_R = nandaddr;
	this->IO_ADDR_W = nandaddr;
	this->hwcontrol = orion_hwcontrol;
	this->dev_ready = orion_device_ready;
	this->chip_delay = 25;
	this->eccmode = NAND_ECC_SOFT;
	this->options |= NAND_USE_FLASH_BBT | NAND_BBT_LASTBLOCK | 
                     NAND_BBT_CREATE | NAND_BBT_WRITE;	

	/* Scan to find existence of the device */
	if (nand_scan (orion_nand_mtd, 1)) {
		printk(KERN_NOTICE "No NAND device - returning -ENXIO\n");
		kfree (orion_nand_mtd);
		iounmap ((void *) nandaddr);
#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)
        iounmap ((void *) smc_cfg_addr);
       
        iounmap ((void *) smc_sta_addr);
        iounmap ((void *) smc_boot_addr);
#endif
		return -ENXIO;
}
	
#ifdef CONFIG_MTD_CMDLINE_PARTS
	mtd_parts_nb = parse_cmdline_partitions(orion_nand_mtd, &mtd_parts, 
						"orion-nand");
	if (mtd_parts_nb > 0)
	  part_type = "command line";
	else
	  mtd_parts_nb = 0;
#endif
	if (mtd_parts_nb == 0)
	{
		mtd_parts = partition_info;
		mtd_parts_nb = NUM_PARTITIONS;
		part_type = "static";
	}
	
	/* Register the partitions */
	printk(KERN_NOTICE "Using %s partition definition\n", part_type);
	add_mtd_partitions(orion_nand_mtd, mtd_parts, mtd_parts_nb);
	
	/* Return happy */
	return 0;
}
module_init(orion_nand_init);

/*
 * Clean up routine
 */
static void __exit orion_cleanup (void)
{
	struct nand_chip *this = (struct nand_chip *) &orion_nand_mtd[1];
	
	/* Release resources, unregister device */
	nand_release (orion_nand_mtd);

	/* Release io resource */
	iounmap ((void *) this->IO_ADDR_W);
#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201)
    iounmap((void *) smc_sta_addr);
    iounmap ((void *) smc_cfg_addr);
 
#endif
 
	/* Free the MTD device structure */
	kfree (orion_nand_mtd);
}
module_exit(orion_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiaodong fan <xiaodong.fan at celestialsemi dot com>");
MODULE_DESCRIPTION("NAND flash driver for celestialsemi serials SOC");
