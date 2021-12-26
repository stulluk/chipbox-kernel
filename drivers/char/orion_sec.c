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

#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

MODULE_AUTHOR("John Thodiyil, <john.thodiyil@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Secure Engine driver");
MODULE_LICENSE("GPL");

/* Register Map */
#define SEC_MAILBOX0            (0x00)      /* Command from host CPU */
#define SEC_MAILBOX1            (0x04)      /* Parameters for the command  */
#define SEC_MAILBOX2            (0x08)      /* Parameters for the command  */
#define SEC_MAILBOX3            (0x0C)      /* Parameters for the command  */
#define SEC_MAILBOX4            (0x10)      /* Parameters for the command  */
#define SEC_MAILBOX5            (0x14)      /* Parameters for the command  */
#define SEC_MAILBOX6            (0x18)      /* Parameters for the command  */
#define SEC_MAILBOX7            (0x1C)      /* Status of host CPU */

#define ORION_SEC_BASE 		0xFFFFF800
#define ORION_SEC_SIZE 		0x100

#define ORION_OTP_BASE  	0xFFFFF000 
#define ORION_OTP_SIZE          0x100

#define SEC_MAGIC 'z'

#define SEC_WR_MAILBOX0         _IOW(SEC_MAGIC, 0x00, int)         
#define SEC_WR_MAILBOX1         _IOW(SEC_MAGIC, 0x01, int)         
#define SEC_WR_MAILBOX2         _IOW(SEC_MAGIC, 0x02, int)         
#define SEC_WR_MAILBOX3         _IOW(SEC_MAGIC, 0x03, int)         
#define SEC_WR_MAILBOX4         _IOW(SEC_MAGIC, 0x04, int)         
#define SEC_WR_MAILBOX5         _IOW(SEC_MAGIC, 0x05, int)         
#define SEC_WR_MAILBOX6         _IOW(SEC_MAGIC, 0x06, int)         

#define SEC_RD_MAILBOX0         _IOR(SEC_MAGIC, 0x07, int)         
#define SEC_RD_MAILBOX1         _IOR(SEC_MAGIC, 0x08, int)         
#define SEC_RD_MAILBOX2         _IOR(SEC_MAGIC, 0x09, int)         
#define SEC_RD_MAILBOX3         _IOR(SEC_MAGIC, 0x0a, int)         
#define SEC_RD_MAILBOX4         _IOR(SEC_MAGIC, 0x0b, int)         
#define SEC_RD_MAILBOX5         _IOR(SEC_MAGIC, 0x0c, int)         
#define SEC_RD_MAILBOX6         _IOR(SEC_MAGIC, 0x0d, int)         
#define SEC_RD_MAILBOX7         _IOR(SEC_MAGIC, 0x0e, int)         

#define SEC_SET_OTPOFST         _IOW(SEC_MAGIC, 0x80, int)
#define SEC_RD_OTP              _IOR(SEC_MAGIC, 0x81, int)

static u32  curr_otp_offset = 0x00000000; /* OTP accessed in bytes */
static struct proc_dir_entry *sec_proc_entry = NULL;
static volatile u32  *orion_sec_base = NULL;
static volatile u32  *orion_otp_base = NULL;
DEFINE_SPINLOCK(orion_sec_lock);

#define sec_writew(v,a)    	do { *(orion_sec_base + (a)) = v; }while(0)
#define sec_readw(a)       	*(orion_sec_base + (a))
#define otp_readw(a)       	*(orion_otp_base + (a))

static int orion_sec_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{

	unsigned long  otpdata;
	unsigned long  flags;
	unsigned long  mailboxdata;

	switch (cmd) {
		case SEC_SET_OTPOFST:
			spin_lock_irqsave(&orion_sec_lock, flags);
			curr_otp_offset = arg;
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_RD_OTP:
			spin_lock_irqsave(&orion_sec_lock, flags);
			otpdata = otp_readw(curr_otp_offset);
			copy_to_user((unsigned long *) arg,&otpdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
		
		case SEC_WR_MAILBOX0:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX0); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX1:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX1); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX2:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX2); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX3:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX3); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX4:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX4); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX5:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX5); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_WR_MAILBOX6:
			spin_lock_irqsave(&orion_sec_lock, flags);
			sec_writew(arg, SEC_MAILBOX6); 
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;

		case SEC_RD_MAILBOX0:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX0);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX1:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX1);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX2:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX2);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX3:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX3);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX4:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX4);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX5:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX5);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX6:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX6);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		case SEC_RD_MAILBOX7:
			spin_lock_irqsave(&orion_sec_lock, flags);
			mailboxdata = sec_readw(SEC_MAILBOX7);
			copy_to_user((unsigned long *) arg,&mailboxdata,4);
			spin_unlock_irqrestore(&orion_sec_lock, flags);
			break;
 
		default:
			break;
	}

	return 0;
}

static struct file_operations orion_sec_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= orion_sec_ioctl,
};

static struct miscdevice orion_sec_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_sec",
	&orion_sec_fops
};

int __init orion_sec_init(void)
{
	int ret = 0;

	ret = -ENODEV;
	if (misc_register(&orion_sec_miscdev)) 
		goto ERR_NODEV;

	ret = -EIO;
	if (!request_mem_region(ORION_SEC_BASE, ORION_SEC_SIZE, "ORION SEC"))
		goto ERR_MEMREQ1;

	if (!request_mem_region(ORION_OTP_BASE, ORION_OTP_SIZE, "ORION OTP"))
		goto ERR_MEMREQ2;

	ret = -EIO;
	if (NULL == (orion_sec_base = ioremap(ORION_SEC_BASE, ORION_SEC_SIZE)))
		goto ERR_MEMMAP;

	if (NULL == (orion_otp_base = ioremap(ORION_OTP_BASE, ORION_OTP_SIZE)))
		goto ERR_MEMMAP;

	return ret;

ERR_MEMMAP:
	release_mem_region(ORION_OTP_BASE, ORION_OTP_SIZE);
ERR_MEMREQ2:
	release_mem_region(ORION_SEC_BASE, ORION_SEC_SIZE);
ERR_MEMREQ1:
	misc_deregister(&orion_sec_miscdev);
ERR_NODEV:
	return ret;
}	

static void __exit orion_sec_exit(void)
{
	iounmap((void*)orion_sec_base);
	release_mem_region(ORION_SEC_BASE, ORION_SEC_SIZE);

	iounmap((void*)orion_otp_base);
	release_mem_region(ORION_OTP_BASE, ORION_OTP_SIZE);

	if (NULL != sec_proc_entry) remove_proc_entry("sec_io", NULL);

	misc_deregister(&orion_sec_miscdev);
}

module_init(orion_sec_init);
module_exit(orion_sec_exit);

