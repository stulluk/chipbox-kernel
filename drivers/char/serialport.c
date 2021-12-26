/* 
 * CS : Linux driver for serial port
 */

/*System Header Files */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <linux/devfs_fs_kernel.h>
/*User Header Files */
#include "serialport.h"


#define serial_writew(v,a)	do{*(uart_base + (a)) = v; } while(0)
#define serial_readw(a)		*(uart_base + (a))

dev_t dev_serial = 0;

static int port;
static spinlock_t serial_lock;
static volatile u16 *uart_base = NULL;
//static struct proc_dir_entry *serialport_proc = NULL;
volatile unsigned long buffer;

typedef struct serialport_devs{
	u32 serialport_id;
	struct cdev cdev;
} serialport_devs_t;

struct serialport_devs *serialport_devices = NULL;


/*prototypes declaration here */
static int __init cs_serialport_rc_init(void);
static void __exit cs_serialport_rc_exit(void);
static int serialport_open (struct inode *inode, struct file *filp);
static int serialport_release (struct inode *inode, struct file *filp);
static ssize_t serialport_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t serialport_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int serialport_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static void serialport_setup_cdev(struct serialport_devs *dev, int index);
static unsigned int serialport_poll(struct file *filp, poll_table *wait);


static DECLARE_WAIT_QUEUE_HEAD(serial_wait_queue);
static DEFINE_SPINLOCK(serialp_lock);
static int irq_flag = 0;



/*Device Open */
static int serialport_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*Device Close */
static int serialport_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*Device Read */
static ssize_t serialport_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	char serialport_buffer;

	serialport_buffer = serial_readw(UART1_RBR);
	copy_to_user(buf, &serialport_buffer, 1);
	if (*f_pos == 0)
	{
		*f_pos += 1;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*Device Write */
static ssize_t serialport_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{

	char serialport_buffer;

	count++;
	copy_from_user(&serialport_buffer, buf, 1);
	serial_writew(serialport_buffer,UART1_THR);
	return 0;
}

static unsigned int serialport_poll(struct file *filp, poll_table *wait)
{	
	unsigned int mask = 0;
//	printk("<CS-RC> CSFP Run in poll\n");
	poll_wait(filp, &serial_wait_queue, wait);
//	printk("<CS-RC> CSFP Poll run \n");	

	if(irq_flag == 1){
		spin_lock_irq(serialp_lock);
		irq_flag = 0;
		spin_unlock_irq(serialp_lock);
		mask = POLLIN | POLLWRNORM;
	}
	return mask;
}

/*Device Input / Output control */
static int serialport_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	unsigned long temp;
//	unsigned long *arg_temp = (unsigned long *) arg;
	switch(cmd)
	{
		case SP_SET_UART_CONFIG:
			spin_lock_irqsave(&serial_lock, flags);
			temp = serial_readw(UART1_USR);
			printk("<CS-RC> UART Modem status register before setting the configuration, arg = 0x%lx\n", temp);
			if (( temp & 0x0001 ) == 0x0000 )
			{
				serial_writew(0x0000, UART1_FCR);
				serial_writew(0x0083, UART1_LCR);
				serial_writew(0x0066, UART1_DLL);
				serial_writew(0x0001, UART1_DLH);
				serial_writew(0x0003, UART1_LCR);
				serial_writew(0x0000, UART1_MCR);
				serial_writew(0x0001, UART1_IER); /* Interrupt is only enabled for "Enable Received Data Available Interrupt" */
				printk("<CS-RC> 9600, 8 bit, 1 stop bit, no parity, no flow control : UART MODE is set \n");
			}
			temp = serial_readw(UART1_USR);
			printk("<CS-RC> UART Modem status register after setting the configuration, arg = 0x%lx\n", temp);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_UART_LOOPBACK:
			spin_lock_irqsave(&serial_lock, flags);
			printk("<CS-RC> Setting UART Loopback Mode for testing \n");
			serial_writew(0x0010, UART1_MCR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_RST_LOOPBACK:
			spin_lock_irqsave(&serial_lock, flags);
			printk("<CS-RC> Resetting Loopback Mode \n");
			serial_writew(0x0000, UART1_MCR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_IR_MODE:
			spin_lock_irqsave(&serial_lock, flags);
			printk("<CS-RC> Setting IR Mode \n");
			serial_writew(0x0040, UART1_MCR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_IR_LOOPBACK:
			spin_lock_irqsave(&serial_lock, flags);
			printk("<CS-RC> Setting IR Loopback Mode \n");
			serial_writew(0x0050, UART1_MCR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_WR_TX_DATA_UART:
			spin_lock_irqsave(&serial_lock, flags);
			printk("<CS-RC> writing 0x%lx to TX register \n", arg);
			serial_writew(arg, UART1_THR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;
		/*************************************************************************************/
		case SP_SET_ENABLE_SERIALIN: /* Enable FIFO's and Interrupts */
			spin_lock_irqsave(&serial_lock, flags);
			serial_writew(0x0000, UART1_FCR);
			serial_writew(0x0000, UART1_MCR);
			serial_writew(0x0001, UART1_IER);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_DISABLE_SERIALIN: /* Disable FIFO's & Interrupts */
			spin_lock_irqsave(&serial_lock, flags);
			serial_writew(0x0000, UART1_FCR); 
			serial_writew(0x0000, UART1_IER);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_RESET_SERIALIN:  /* Flush FIFO's */
			spin_lock_irqsave(&serial_lock, flags);
			serial_writew(0x0000, UART1_FCR); 
			serial_writew(0x0001, UART1_FCR);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_SERIALIN_ATTR: /* Sets Divisor bits for setting Baud rate */
			spin_lock_irqsave(&serial_lock, flags);
			temp = serial_readw(UART1_USR);
			if (( temp & 0x0001 ) == 0x0000 )
			{
				serial_writew((arg & 0x00FF), UART1_DLL);
				serial_writew(((arg & 0xFF00) >> 8), UART1_DLH);
			}
			else
			{
				printk("<CS-RC> UART Modem Busy, USR arg = 0x%lx\n", temp);
			}
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_SET_LCR: /* Necessary for setting baud rate */
			spin_lock_irqsave(&serial_lock, flags);
			temp = serial_readw(UART1_USR);
			if (( temp & 0x0001 ) == 0x0000 )
			{	
				serial_writew(arg, UART1_LCR);
			}
			else
			{
				printk("<CS-RC> UART Modem Busy, USR arg = 0x%lx\n", temp);
			}
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		case SP_GET_RBR_DATA: /* required to pass the data to callback function */
			spin_lock_irqsave(&serial_lock, flags);
			temp = serial_readw(UART1_RBR);
			copy_to_user((unsigned long *)arg, &temp, 1);
			spin_unlock_irqrestore(&serial_lock, flags);
			break;

		/*************************************************************************************/
		default:
			break;
	}
	return 0;
}

static struct file_operations serialport_fops = {
	.owner = THIS_MODULE,
	.read = serialport_read,
    .write = serialport_write,
	.poll = serialport_poll,
	.ioctl = serialport_ioctl,
    .open = serialport_open,
    .release = serialport_release,
};


/*Interrupt Routine */
static irqreturn_t serialport_intr(int irq, void *dev_id, struct pt_regs *regs)
{

	unsigned long arg;

//	printk("<CS-RC>Got an Interrupt \n");
	arg = serial_readw(UART1_IIR); /*read this register to identify the interrupt */
//	printk("<CS-RC> Value of Interrupt Identity Register = 0x%lx\n", arg);

	if((arg & 0x00C0) == 0x00C0){
//		printk("<CS-RC> FIFO's are enabled\n");
	}
	else{
//		printk("<CS-RC> FIFO's are disabled\n");
	}

	if((arg & 0x000F) == 0x0000)
	{
		printk("<CS-RC> CTS / DSR / RI / Carrier Detect : Reading Modem Status Register\n");
	} 
	else if ((arg & 0x000F) == 0x0001)
	{
		printk("<CS-RC> No Pending Interrupts \n");
	} 
	else if ((arg & 0x000F) == 0x0002)
	{
		printk("<CS-RC> THR Empty: Reading Interrupt Identity Register\n");
	} 
	else if ((arg & 0x000F) == 0x0004)
	{
//		printk("<CS-RC> Receiver Data Available : Reading Receiver Buffer\n");
		buffer = serial_readw(UART1_RBR);  /*read this register to clear the interrupt */
//		printk("<CS-RC> Received Data = 0x%lx\n", buffer);
		spin_lock_irq(serialp_lock);
		if (irq_flag == 0) {
			irq_flag = 1;
			wake_up(&serial_wait_queue);
//			printk("<CS-RC> wake up queue\n");
//			wait_event_interruptible(serial_wait_queue, irq_flag != 0);
		}
		spin_unlock_irq(serialp_lock);
	} 
	else if ((arg & 0x000F) == 0x0006)
	{
		printk("<CS-RC> Reading Receiver Line Status : (overrun / parity / framing / break interrupt) \n");
//		buffer = serial_readw(UART1_LSR);
//		printk("<CS-RC> LSR = 0x%lx\n", buffer);
	} 
	else if ((arg & 0x000F) == 0x0007)
	{
		printk("<CS-RC> Busy Detected: USR[0] is set to 1\n");
	} 
	else if ((arg & 0x000F) == 0x000C)
	{
		printk("<CS-RC> Character Timeout: No Character in or out of Receiver FIFO\n");
	}
	
	return IRQ_HANDLED;
}

static void serialport_setup_cdev (struct serialport_devs *dev, int index)
{
	int errorcode = 0;
	char devfs_name[80];
	
	int devno = MKDEV(serialport_major, index);
	cdev_init(&dev->cdev, &serialport_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &serialport_fops;

	errorcode = cdev_add (&dev->cdev, devno, 1);
	if(errorcode){
		printk("<CS-RC>serialport : Error adding serialport device\n");
	}
	snprintf(devfs_name, sizeof(devfs_name), "serialport%d",index);
	errorcode = devfs_mk_cdev(devno,S_IFCHR | S_IRUSR| S_IWUSR | S_IRGRP, devfs_name);
	if(errorcode){
		printk("<CS-RC>serialport : Error making serialport device\n");
	}
}


/*Module Initialization */
static int __init cs_serialport_rc_init(void)
{
	int retval = 0;
	int result = 0;
	int region = 0;
    

/*Getting Major number */
	if(serialport_major)
	{
		dev_serial = MKDEV(serialport_major, 0);
		retval = register_chrdev_region(dev_serial, 1, "serialport");
	}
	else
	{
		retval = alloc_chrdev_region(&dev_serial, 0, 1, "serialport");
		serialport_major = MAJOR(dev_serial);
	}
	if (retval < 0)
	{
		printk("<CS-RC> serialport : Cannot obtain major number %d\n", serialport_major);
		return retval;
	}
	else
	{
		printk("<CS-RC> Inserting serialport module with major number = %d\n", serialport_major);
	}


/*check and request memory region */
	region = check_mem_region(UART1_BASE, UART1_BASE_SIZE);
	if(region)
	{
		printk("<CS-RC> serialport: cannot get the mem region\n");
		return -ENOMEM;
	}
	if(!request_mem_region(UART1_BASE, UART1_BASE_SIZE, "serialport")){
		unregister_chrdev_region(dev_serial,1);
		return -EIO;
	}

	uart_base = (volatile unsigned short *) ioremap (UART1_BASE, UART1_BASE_SIZE);
	if(!uart_base)
	{	
		unregister_chrdev_region(dev_serial,1);
		printk("<CS-RC> serialport: ioremap failed \n");
		return -EIO;
	}
	
	serialport_devices = kmalloc (1 * sizeof(struct serialport_devs), GFP_KERNEL);
	if(!serialport_devices){
		iounmap((void *) uart_base);
		release_mem_region(UART1_BASE, UART1_BASE_SIZE);
		unregister_chrdev_region(dev_serial,1);
		return -ENOMEM;
	}
	memset(serialport_devices, 0, 1 * sizeof(struct serialport_devs));

	serialport_devices->serialport_id = 1;
	serialport_setup_cdev(serialport_devices,1);
		

/*get the requested IRQ */
	result = request_irq(UART1_IRQ, serialport_intr, SA_INTERRUPT, "serialport", NULL);
	if(result)
	{
		printk("<CS-RC> Can't get irq\n");
		kfree(serialport_devices);
        iounmap ((void *) uart_base);
        if (!port)
            {
                release_mem_region(UART1_BASE,UART1_BASE_SIZE);
            }
        devfs_remove("serialport1");
        unregister_chrdev_region(dev_serial, 1);
        return -EIO;
	}
	
/*Add entry to proc */
	/*serialport_proc = create_proc_entry("serialport",0,NULL);
	if(serialport_proc != NULL)
	{
		serialport_proc -> write_proc = &serialport_proc_write;
		serialport_proc -> read_proc = &serialport_proc_read;
	}*/
	return 0;

}

/*Module De-initialization */
static void __exit cs_serialport_rc_exit(void)
{
	kfree(serialport_devices);
	iounmap ((void *) uart_base);
	if (!port)
	{
		release_mem_region(UART1_BASE,UART1_BASE_SIZE);
	}
	devfs_remove("serialport1");
	unregister_chrdev_region(dev_serial, 1);
	free_irq(UART1_IRQ,NULL);
}


/*Module Entry and Exit Points */
module_init(cs_serialport_rc_init);
module_exit(cs_serialport_rc_exit);

/*Module Parameters */
MODULE_AUTHOR("TATA-CS");
MODULE_DESCRIPTION("CS SERIAL PORT DRIVER");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");

/*End of File */
