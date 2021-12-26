#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/system.h>

#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

MODULE_AUTHOR("Zhongkai Du, <zhongkai.du@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Front Panel Controller driver");
MODULE_LICENSE("GPL");

/* Register Map */
#define FPC_INTS 		(0x00  >> 1) /* Interrupt Status Register */
#define FPC_INTM 		(0x04  >> 1) /* Interrupt Mask Register */
#define FPC_RC_CTL_L 		(0x08  >> 1) /* Remote Control Register */
#define FPC_RC_CMD_L 		(0x100 >> 1) /* RC5 Command Register */
#define FPC_NEC_CMD_L 		(0x104 >> 1) /* NEC Command Register */
#define FPC_LED_DATA_L 		(0x200 >> 1) /* Led Display Data Register */
#define FPC_LED_CNTL_L 		(0x204 >> 1) /* Led Display Control Register */
#define FPC_KSCAN_DATA_L 	(0x300 >> 1) /* Key Scan Data Register */
#define FPC_KSCAN_CNTL_L	(0x304 >> 1) /* Key Scan Control Register */
#define FPC_ARBCNTL_L 		(0x308 >> 1) /* Led & Key Scan Arbiter Control Register */
#define FPC_RC_CTL_H 		(0x0a  >> 1) /* Remote Control Register */
#define FPC_RC_CMD_H 		(0x102 >> 1) /* RC5 Command Register */
#define FPC_NEC_CMD_H		(0x106 >> 1) /* NEC Command Register */
#define FPC_LED_DATA_H 		(0x202 >> 1) /* Led Display Data Register */
#define FPC_LED_CNTL_H 		(0x206 >> 1) /* Led Display Control Register */
#define FPC_KSCAN_DATA_H 	(0x302 >> 1) /* Key Scan Data Register */
#define FPC_KSCAN_CNTL_H 	(0x306 >> 1) /* Key Scan Control Register */
#define FPC_ARBCNTL_H 		(0x30a >> 1) /* Led & Key Scan Arbiter Control Register */

#define ORION_FPC_BASE 		0x10172000
#define ORION_FPC_SIZE 		0x100
#define ORION_FPC_IRQ		8

#define FPC_MAGIC 'z'

#define FPC_LED_EN	 	        _IOW(FPC_MAGIC, 0x06, int)
#define FPC_LED_DISP	 	    _IOW(FPC_MAGIC, 0x08, int)
#define FPC_RC_SET_SYSTEMCODE	_IOW(FPC_MAGIC, 0x09, int)
#define FPC_LED_GET		        _IOR(FPC_MAGIC, 0x0a, int)
#define FPC_KEYSCAN_EN          _IOW(FPC_MAGIC, 0x0b, int)
#define FPC_KSCAN_GET		    _IOR(FPC_MAGIC, 0x0c, int)

/* Adjust bit time count outside for nonstandard rc protocal */
#define FPC_SET_BitTimeCnt	_IOR(FPC_MAGIC, 0x0d, int)
#define FPC_GET_BitTimeCnt	_IOR(FPC_MAGIC, 0x0e, int)


/*
#define FPC_LED_EN_DIM1         _IOW(FPC_MAGIC, 0x0d, int)
#define FPC_LED_EN_DIM2         _IOW(FPC_MAGIC, 0x0e, int)
#define FPC_LED_EN_DIM3         _IOW(FPC_MAGIC, 0x0f, int)
#define FPC_LED_EN_DIM4		    _IOW(FPC_MAGIC, 0x10, int)
#define FPC_LED_EN_DIM5		    _IOW(FPC_MAGIC, 0x11, int)
#define FPC_LED_EN_DIM6		    _IOW(FPC_MAGIC, 0x12, int)
#define FPC_LED_EN_DIM7		    _IOW(FPC_MAGIC, 0x13, int)*/

#define FPC_RC_EN      _IOW(FPC_MAGIC, 0x14, int)

int rc_system_code = 0xffffffff;

static struct proc_dir_entry *fpc_proc_entry = NULL;
static volatile u16 *orion_fpc_base = NULL;
DEFINE_SPINLOCK(orion_fpc_lock);

#define fpc_writew(v,a)    	do { *(orion_fpc_base + (a)) = v; }while(0)
#define fpc_readw(a)       	*(orion_fpc_base + (a))

extern int orion_gpio_register_module_status(const char * module, unsigned int  orion_module_status); /* status: 0 disable, 1 enable */
static int orion_fpc_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	unsigned long flags,userbit,Key_Val;
	unsigned short rc_ctl, btc;
	//    unsigned long *arg_temp = (unsigned long *)arg;
	switch (cmd) {
		case FPC_LED_EN:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8000, FPC_LED_CNTL_H);
			fpc_writew(0 != arg ? 0x1000 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", arg ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
			/*		case FPC_LED_EN_DIM1:
					spin_lock_irqsave(&orion_fpc_lock, flags);

					fpc_writew(0x8001, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0100 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
			case FPC_LED_EN_DIM2:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8002, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0010 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
			case FPC_LED_EN_DIM3:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8004, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0001 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;

			case FPC_LED_EN_DIM4:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8005, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0000 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
			case FPC_LED_EN_DIM5:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8006, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0000 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
			case FPC_LED_EN_DIM6:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8006, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0000 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
			case FPC_LED_EN_DIM7:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew(0x8007, FPC_LED_CNTL_H);
			//			fpc_writew(0 != *arg_temp ? 0x0000 : 0x0000, FPC_LED_CNTL_L);

			orion_gpio_register_module_status("FPC", *arg_temp ? 1 : 0);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;*/
		case FPC_LED_DISP:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			fpc_writew((arg >> 16) & 0xffff, FPC_LED_DATA_H);
			fpc_writew(arg & 0xffff, FPC_LED_DATA_L);

			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
		case FPC_LED_GET:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			userbit = fpc_readw(FPC_LED_DATA_H);
			userbit = userbit << 16;
			userbit |= fpc_readw(FPC_LED_DATA_L);
			copy_to_user((unsigned long *) arg,&userbit,4);

			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;

		case FPC_RC_SET_SYSTEMCODE:
			spin_lock_irqsave(&orion_fpc_lock, flags);
			rc_system_code = arg;
			spin_unlock_irqrestore(&orion_fpc_lock, flags);

			break;
		case FPC_KEYSCAN_EN:
			spin_lock_irqsave(&orion_fpc_lock, flags);
			fpc_writew((arg >> 16)& 0xffff,FPC_KSCAN_CNTL_H);
			fpc_writew(arg & 0xffff,FPC_KSCAN_CNTL_L);
			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			if (arg == 1){
				orion_gpio_register_module_status("FPC", 1);                    
			}
			else{
				orion_gpio_register_module_status("FPC", 0);                    
			}
			break;
		case FPC_KSCAN_GET:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			Key_Val = fpc_readw(FPC_KSCAN_DATA_H);
			Key_Val = Key_Val << 16;
			Key_Val |= fpc_readw(FPC_KSCAN_DATA_L);
			copy_to_user((unsigned long*)arg,&Key_Val,4);

			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
		case FPC_RC_EN:
			spin_lock_irqsave(&orion_fpc_lock, flags);
			rc_ctl = fpc_readw(FPC_RC_CTL_H);
			if (arg == 1){
				fpc_writew(rc_ctl | 0x8000, FPC_RC_CTL_H);
				orion_gpio_register_module_status("RC", 1);                    
			}
			else if (arg == 0){
				fpc_writew(rc_ctl & 0x7fff,FPC_RC_CTL_H);
				orion_gpio_register_module_status("RC", 0);
			}
			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
		case FPC_SET_BitTimeCnt:
			spin_lock_irqsave(&orion_fpc_lock, flags);

			rc_ctl = fpc_readw(FPC_RC_CTL_L);
			rc_ctl &= 0x00ff; rc_ctl |= (arg & 0xff) << 8;
			fpc_writew(rc_ctl, FPC_RC_CTL_L);

			rc_ctl = fpc_readw(FPC_RC_CTL_H);
			rc_ctl &= 0xff00; rc_ctl |= (arg >> 8) & 0xff;
			fpc_writew(rc_ctl, FPC_RC_CTL_H);

			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
		case FPC_GET_BitTimeCnt:
			spin_lock_irqsave(&orion_fpc_lock, flags);
			rc_ctl = fpc_readw(FPC_RC_CTL_L);
			btc = (rc_ctl & 0xff00) >> 8;

			rc_ctl = fpc_readw(FPC_RC_CTL_H);
			btc |= (rc_ctl & 0xff) << 8;

			copy_to_user((unsigned long*)arg,&btc,2);

			spin_unlock_irqrestore(&orion_fpc_lock, flags);
			break;
		default:
			break;
	}

	return 0;
}

static struct file_operations orion_fpc_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= orion_fpc_ioctl,
};

static struct miscdevice orion_fpc_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_fpc",
	&orion_fpc_fops
};


static int fpc_proc_write(struct file *file, 
                const char *buffer, unsigned long count, void *data)
{
	u32 addr;
	u16 val;

        const char *cmd_line = buffer;;

        if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
                val = fpc_readw(addr>>1);
                printk(" readl [0x%04x] = 0x%04x \n", addr, val);
        }
        else if (strncmp("ww", cmd_line, 2) == 0) {
                addr = simple_strtol(&cmd_line[3], NULL, 16);
                val = simple_strtol(&cmd_line[7], NULL, 16);
                fpc_writew(val, addr>>1);
        }
        else if (strncmp("reboot", cmd_line, 6) == 0) {
            printk("reboot the systme ....\n ");
            arch_reset(0);
        }
        
        return count;
}

int __init orion_fpc_init(void)
{
	int ret = 0;

	ret = -ENODEV;
	if (misc_register(&orion_fpc_miscdev)) 
		goto ERR_NODEV;

	ret = -EIO;
	if (!request_mem_region(ORION_FPC_BASE, ORION_FPC_SIZE, "ORION I2C"))
		goto ERR_MEMREQ;

	ret = -EIO;
	if (NULL == (orion_fpc_base = ioremap(ORION_FPC_BASE, ORION_FPC_SIZE)))
		goto ERR_MEMMAP;

	fpc_proc_entry = create_proc_entry("fpc_io", 0, NULL);
	if (NULL != fpc_proc_entry) {
		fpc_proc_entry->write_proc = &fpc_proc_write;
	}

	return ret;

ERR_MEMMAP:
	release_mem_region(ORION_FPC_BASE, ORION_FPC_SIZE);
ERR_MEMREQ:
	misc_deregister(&orion_fpc_miscdev);
ERR_NODEV:
	return ret;
}	

static void __exit orion_fpc_exit(void)
{
	iounmap((void*)orion_fpc_base);
	release_mem_region(ORION_FPC_BASE, ORION_FPC_SIZE);
	if (NULL != fpc_proc_entry) remove_proc_entry("fpc_io", NULL);

	misc_deregister(&orion_fpc_miscdev);
}

module_init(orion_fpc_init);
module_exit(orion_fpc_exit);

EXPORT_SYMBOL(rc_system_code);

