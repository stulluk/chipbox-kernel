/*======================================================================

  Device driver for the PCMCIA control functionality of ORION CSM1200
  microprocessors.

    The contents of this file may be used under the
    terms of the GNU Public License version 2 (the "GPL")

    derived from sa11xx_base.c

     Portions created by John G. Dorsey are
     Copyright (C) 1999 John G. Dorsey.

  ======================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/cpufreq.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/ss.h>
#include <pcmcia/bulkmem.h>
#include <pcmcia/cistpl.h>

#include "cs_internal.h"
#include "soc_common.h"
#include "orion_socketbase.h"

//#define debugk(s) printk(s)
#define debugk(s) 

extern struct semaphore ebi_mutex_lock;

static struct pcmcia_irqs irqs[] = {
	/*--------------------------------------------------
	* { 0, 31, "PCMCIA0 CD" },
	* { 0, 30, "PCMCIA0 TIMOUT" },
	* { 0, 29, "PCMCIA0 DATAEXCHG" },
	*--------------------------------------------------*/
};

static unsigned int timing_mrd = 0xE1;
static unsigned int timing_mwr = 0x04A1;
static unsigned int timing_io  = 0x7FFF;


static void orion_pcmcia_hw_set_cis_rmode(void)
{
	down(&ebi_mutex_lock);
    	iowrite32(0x00, OPMODE);			
    	iowrite32(timing_mrd, TIMECFG);			
	up(&ebi_mutex_lock);
}
static void orion_pcmcia_hw_set_cis_wmode(void)
{
	down(&ebi_mutex_lock);
    	iowrite32(0x00, OPMODE);			
    	iowrite32(timing_mwr, TIMECFG);			
	up(&ebi_mutex_lock);
}
static void orion_pcmcia_hw_set_comm_rmode(void)
{
	down(&ebi_mutex_lock);
    	iowrite32(0x02, OPMODE);			
    	iowrite32(timing_mrd, TIMECFG);			
	up(&ebi_mutex_lock);
}
static void orion_pcmcia_hw_set_comm_wmode(void)
{
	down(&ebi_mutex_lock);
    	iowrite32(0x02, OPMODE);			
    	iowrite32(timing_mwr, TIMECFG);			
	up(&ebi_mutex_lock);
}
static void orion_pcmcia_hw_set_io_mode(void)
{
	down(&ebi_mutex_lock);
    	iowrite32(0x04, OPMODE);			
    	iowrite32(timing_io, TIMECFG);			
	up(&ebi_mutex_lock);
}

static unsigned char orion_pcmcia_hw_read_cis(unsigned int addr)
{
	unsigned char val = 0;
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), ATTRBASE);			// orion pcmcia have only 15 addr lines
	val = ioread8((void*)(VA_PCMCIA_BASE + (addr & 0xFF)));	// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
	return val;
}

static unsigned char orion_pcmcia_hw_read_comm(unsigned int addr)
{
	unsigned char val = 0;
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), COMMBASE);			// orion pcmcia have only 15 addr lines
	val = ioread8((void*)(VA_PCMCIA_BASE + (addr & 0xFF)));	// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
	return val;
}

static unsigned char orion_pcmcia_hw_read_io(unsigned int addr)
{
	unsigned char val = 0;
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), IOBASE);			// orion pcmcia have only 15 addr lines
	val = ioread8((void*)(VA_PCMCIA_BASE + (addr & 0xFF)));	// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
	return val;
}

static void orion_pcmcia_hw_write_cis(unsigned char val, unsigned int addr)
{
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), ATTRBASE);			// orion pcmcia have only 15 addr lines
	iowrite8(val, (void*)(VA_PCMCIA_BASE + (addr & 0xFF)));		// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
}

static void orion_pcmcia_hw_write_comm(unsigned char val, unsigned int addr)
{
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), COMMBASE);			// orion pcmcia have only 15 addr lines
	iowrite8(val, (void*)(VA_PCMCIA_BASE + (addr & 0xFF)));		// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
}

static void orion_pcmcia_hw_write_io(unsigned char val, unsigned int addr)
{
	down(&ebi_mutex_lock);
    	iowrite32(((addr >> 8) & 0x3F), IOBASE);			// orion pcmcia have only 15 addr lines
	iowrite8(val, (void*)(VA_PCMCIA_BASE + (addr & 0xFF)));		// device access window addr 0x00~0xFF
	up(&ebi_mutex_lock);
}

static int orion_pcmcia_hw_init(struct soc_pcmcia_socket *skt)
{
	/*
	 * Setup default state of GPIO12 to PCMCIA_RESET_n
	 */
	unsigned int reg;

debugk("init in\n");

	//gpio_hw_set_direct(15, 1); //set gpio15 to write mode

	iowrite16(0x1000, (void*)WAITTMR);	//FIXME default time out error while reading & writing

	if (skt->irq == NO_IRQ){
		//skt->irq = skt->nr ? IRQ_S1_READY_NINT : IRQ_S0_READY_NINT;
	}

debugk("init out\n");
	return soc_pcmcia_request_irqs(skt, irqs, ARRAY_SIZE(irqs));
}

static void orion_pcmcia_hw_shutdown(struct soc_pcmcia_socket *skt)
{
debugk("shutdown in\n");

debugk("shutdown out\n");
	soc_pcmcia_free_irqs(skt, irqs, ARRAY_SIZE(irqs));
}

static void orion_pcmcia_socket_state(struct soc_pcmcia_socket *skt, struct pcmcia_state *state)
{
	unsigned char inserted;
	unsigned char vs;
	unsigned short rawstat, pinctl;

	state->ready = 0;
	state->detect = 0;
	state->vs_3v = 0;

	rawstat = ioread16((void*)RAWSTAT);
	pinctl = ioread16((void*)PINCTL);

debugk("hw state in\n");
	switch (skt->nr) {
	    case 0:
		//FIXME
		vs = 0x02;
		if((rawstat & INT_CDCHG) && ((PIN_CD1 | PIN_CD2) & pinctl))
		    inserted = 1;
		else
		    inserted = 0;
		break;

	    default: 	/* should never happen */
		return;
	}

	if (inserted) {
		switch (vs) {
			case 0:
			case 2: /* 3.3V */
				state->vs_3v = 1;
				//printk("pcmcia card detected!\n");
				break;
			case 3: /* 5V */
				break;
			default:
				/* return without setting 'detect' */
				printk(KERN_ERR "pcmcia bad VS (%d)\n", vs);
		}
		state->detect = 1;
		//if(pinctl & PIN_RDYIREQ){
		    //printk("pcmcia card is ready!\n");
		    state->ready = 1;
		//}
	}
	else {
		/* if the card was previously inserted and then ejected,
		 * we should enable the card detect interrupt
		 */
/*--------------------------------------------------
* enable_irq(31);
*--------------------------------------------------*/
		//printk(KERN_ERR "pcmcia not avaliable!\n");
	}

	state->bvd1 = 1;
	state->bvd2 = 1;
	state->wrprot = 0;
	state->vs_Xv = 0;
/*--------------------------------------------------
*     //clear CD interrupt ?  failed!!
*     unsigned short tmp;
*     tmp = ioread16((void*)RAWSTAT);
*     iowrite16(tmp, (void*)RAWSTAT);
*--------------------------------------------------*/
debugk("hw state out\n");
}

static int orion_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
        unsigned short pinctl;
	debug("config_skt %d Vcc %dV Vpp %dV, reset %d\n",
			skt->nr, state->Vcc, state->Vpp,
			state->flags & SS_RESET);
#if 0
	if ((skt->nr == 0) && (state->flags & SS_RESET)){

	    gpio_hw_write(15, 1);
	    msleep(1);
	    gpio_hw_write(15, 0);
	    msleep(1);

	}
#endif
	return 0;
}

/*
 * Enable card status IRQs on (re-)initialisation.  This can
 * be called at initialisation, power management event, or
 * pcmcia event.
 */
static void orion_pcmcia_socket_init(struct soc_pcmcia_socket *skt)
{
}

/*
 * Disable card status IRQs and PCMCIA bus on suspend.
 */
static void orion_pcmcia_socket_suspend(struct soc_pcmcia_socket *skt)
{
}

/*
 * Hardware specific timing routines.
 * If provided, the get_timing routine overrides the SOC default.
 */
static unsigned int orion_pcmcia_socket_get_timing(struct soc_pcmcia_socket *skt, unsigned int a, unsigned int b)
{
    return 0;
}

static int orion_pcmcia_socket_set_timing(struct soc_pcmcia_socket *skt)
{
    //iowrite16(0x00E1, (void*)TIMECFG);

    return 0;
}

static int orion_pcmcia_socket_show_timing(struct soc_pcmcia_socket *skt, char *buf)
{
    return 0;
}

static struct pcmcia_low_level orion_pcmcia_ops = {
	.owner			= THIS_MODULE,

	.nr			= 1,
	.first			= 0,
	.hw_init 		= orion_pcmcia_hw_init,
	.hw_shutdown		= orion_pcmcia_hw_shutdown,

	.socket_state		= orion_pcmcia_socket_state,
	.configure_socket	= orion_pcmcia_configure_socket,

	.socket_init		= orion_pcmcia_socket_init,
	.socket_suspend		= orion_pcmcia_socket_suspend,
	
	.get_timing		= NULL,
	.set_timing		= orion_pcmcia_socket_set_timing,
	.show_timing		= orion_pcmcia_socket_show_timing,

	/* ORION CSM1200 specified */
	.set_cis_rmode		= orion_pcmcia_hw_set_cis_rmode,
	.set_cis_wmode		= orion_pcmcia_hw_set_cis_wmode,
	.set_comm_rmode		= orion_pcmcia_hw_set_comm_rmode,
	.set_comm_wmode		= orion_pcmcia_hw_set_comm_wmode,
	.set_io_mode		= orion_pcmcia_hw_set_io_mode,
	.read_cis		= orion_pcmcia_hw_read_cis,
	.write_cis		= orion_pcmcia_hw_write_cis,
	.read_comm		= orion_pcmcia_hw_read_comm,
	.write_comm		= orion_pcmcia_hw_write_comm,
	.read_io		= orion_pcmcia_hw_read_io,
	.write_io		= orion_pcmcia_hw_write_io
};

static struct proc_dir_entry *pcmcia_proc_entry = NULL;


static unsigned int stoi(const char* s) 
{
    char *p = s; 
    char c; 
    unsigned int i = 0; 
    
    while (c = *p++) { 
        if(c >= '0' && c <= '9') { 
            i  =  i * 16 + (c - '0'); 
        } else if (c >= 'a'  &&  c <= 'f') {
            i  =  i * 16 + (c - 'a' + 10); 
        } else if (c >= 'A'  &&  c <= 'F') {
            i  =  i * 16 + (c - 'A' + 10); 
        } else if (c == 0x0a) {
            break;
        } else {
            printk("illegal char: %c\n", c);    
            return  0xffffffff;
        }
    }

    return  i; 
} 


static int pcmcia_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned int addr;
	unsigned int val;
    unsigned int timing_val;


	const char *cmd_line = buffer;
    printk("pcmcia_proc_write\n");

	if (strncmp("help", cmd_line, 4) == 0) {
        printk("command:\n");
        printk("  rst: reset the pc caard\n");
        printk("  reg: show pcmcia controller registers\n");
        printk("  cis: show cis of attribute space\n");
        printk("  tmr_xxx: set the timing for memory read    mode, xxx should be hexadecimal and none-0x headed\n");
        printk("  tmw_xxx: set the timing for memory write   mode, xxx should be hexadecimal and none-0x headed\n");
        printk("  tio_xxx: set the timing for IO (read/write)mode, xxx should be hexadecimal and none-0x headed\n");
    } else if (strncmp("rst", cmd_line, 3) == 0) {
        NULL;
    } else if (strncmp("tmr_", cmd_line, 4) == 0) {
        timing_val = stoi(cmd_line + 4);
        
        if (0xffffffff == timing_val) {
            printk("Illegal command, timing_val = %x\n", timing_val);
        } else {
            timing_mrd = timing_val;
            printk("timing_mrd: %x\n", timing_mrd);
        }
    } else if (strncmp("tmw_", cmd_line, 4) == 0) {
        timing_val = stoi(cmd_line + 4);
        if (0xffffffff == timing_val) {
            printk("Illegal command\n");
        } else {
            timing_mwr = timing_val;
            printk("timing_mwr: %x\n", timing_mwr);
        }
    } else if (strncmp("tio_", cmd_line, 4) == 0) {
        timing_val = stoi(cmd_line + 4);
        if (0xffffffff == timing_val) {
            printk("Illegal command\n");
        } else {
            timing_io = timing_val;
            printk("timing_io: %x\n", timing_io);
        }
    } else if (strncmp("cis", cmd_line, 3) == 0) {
        int off = 0;
        char ch;

        orion_pcmcia_hw_set_cis_rmode();
        printk("reading cis:\n");
        printk("data cis:\n");

        for (off = 0; off < 128; off++) {
                ch = orion_pcmcia_hw_read_cis(2*off);
                printk("%02x ", ch);
            }
        printk("\n");
        
    } else if (strncmp("reg", cmd_line, 3) == 0) {
        printk("Show pcmcia contrller registers:\n");
        printk("TIMECFG  (off = 0x100), val = 0x%08x\n", ioread16((void*)TIMECFG));
        printk("OPMODE   (off = 0x104), val = 0x%08x\n", ioread16((void*)OPMODE));
        printk("RAWSTAT  (off = 0x108), val = 0x%08x\n", ioread16((void*)RAWSTAT));
        printk("INTSTAT  (off = 0x10c), val = 0x%08x\n", ioread16((void*)INTSTAT));
        printk("INTENA   (off = 0x110), val = 0x%08x\n", ioread16((void*)INTENA));
        printk("PINCTL   (off = 0x114), val = 0x%08x\n", ioread16((void*)PINCTL));
        printk("ATTRBASE (off = 0x118), val = 0x%08x\n", ioread16((void*)ATTRBASE));
        printk("COMMBASE (off = 0x11c), val = 0x%08x\n", ioread16((void*)COMMBASE));
        printk("IOBASE   (off = 0x120), val = 0x%08x\n", ioread16((void*)IOBASE));
        printk("WAITTMR  (off = 0x124), val = 0x%08x\n", ioread16((void*)WAITTMR));

        printk("Timing  (for memory read),   val = 0x%08x\n", timing_mrd);
        printk("Timing  (for memory write),  val = 0x%08x\n", timing_mwr);
        printk("Timing  (for io read/write), val = 0x%08x\n", timing_io);
	} else if (strncmp("rst", cmd_line, 3) == 0)
    {
        /*gpio_hw_write(15, 1);
	    msleep(4000);
	    gpio_hw_write(15, 0);
	    msleep(4000);

        printk("reset complete\n");*/
       }
	else {
    	printk("Illegal command\n");
	}

    return count;   
}

int __init orion_drv_pcmcia_probe(struct device *dev)
{
	int ret;
	struct pcmcia_low_level *ops;

	printk("=====>> orion_drv_pcmcia_probe \n" );
	
	//struct pcmcia_low_level dft;	//added by xm.chen
	int first, nr;

	if (!dev)
		return -ENODEV;

	//FIXME
	if(!dev->platform_data)
	    	dev->platform_data = (void*)&orion_pcmcia_ops;

	ops = (struct pcmcia_low_level *)dev->platform_data;
	first = ops->first;
	nr = ops->nr;

#ifdef CONFIG_CPU_FREQ
	ops->frequency_change = orion_pcmcia_frequency_change;
#endif

	pcmcia_proc_entry = create_proc_entry("pcmcia_io", 0, NULL);
	if (NULL != pcmcia_proc_entry) {
		pcmcia_proc_entry->write_proc = &pcmcia_proc_write;
        printk("Succesefully create pcmcia proc entry!\n");
	} else {
	    printk("Failed to create pcmcia proc entry!\n");
	}



	ret = soc_common_drv_pcmcia_probe(dev, ops, first, nr);

	return ret;
}
EXPORT_SYMBOL(orion_drv_pcmcia_probe);

static int orion_drv_pcmcia_suspend(struct device *dev, pm_message_t state, u32 level)
{
	int ret = 0;
	if (level == SUSPEND_SAVE_STATE)
		ret = pcmcia_socket_dev_suspend(dev, state);
	return ret;
}

static int orion_drv_pcmcia_resume(struct device *dev, u32 level)
{
	int ret = 0;
	if (level == RESUME_RESTORE_STATE)
	{
		/*--------------------------------------------------
		* struct pcmcia_low_level *ops = dev->platform_data;
		* int nr = ops ? ops->nr : 0;
		*--------------------------------------------------*/

		/*--------------------------------------------------
		* MECR = nr > 1 ? MECR_CIT | MECR_NOS : (nr > 0 ? MECR_CIT : 0);
		*--------------------------------------------------*/
		ret = pcmcia_socket_dev_resume(dev);
	}
	return ret;
}

static struct device_driver orion_pcmcia_driver = {
	.probe		= NULL,//orion_drv_pcmcia_probe,
	.remove		= NULL,//soc_common_drv_pcmcia_remove,
	.suspend 	= NULL,//orion_drv_pcmcia_suspend,
	.resume 	= NULL,//orion_drv_pcmcia_resume,
	.name		= "orion-pcmcia",
	.bus		= &platform_bus_type,
};

static struct platform_device orion_pcmcia_device = {
	.name   = "orion-pcmcia",
	.id     = 0,
};

static int __init orion_pcmcia_init(void)
{
	//printk("PCMCIA: %s called!\n", __FUNCTION__);
	int ret = 0;

	/*ret = driver_register(&orion_pcmcia_driver);

	if (!ret) {
		ret = platform_device_register(&orion_pcmcia_device);
		if (ret)
			driver_unregister(&orion_pcmcia_driver);
	}*/

	return ret;
}

static void __exit orion_pcmcia_exit(void)
{
	platform_device_unregister(&orion_pcmcia_device);
	driver_unregister(&orion_pcmcia_driver);
}

module_init(orion_pcmcia_init);
module_exit(orion_pcmcia_exit);

MODULE_AUTHOR("ChenXiming <xm.chen@celestialsemi.com> ");
MODULE_DESCRIPTION("Linux PCMCIA Card Services: PALMCHIP core socket driver");
MODULE_LICENSE("GPL");

