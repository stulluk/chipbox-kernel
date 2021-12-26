#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/mem_define.h>

#define CMD_READ_GPIO	 _IOR('G', 1, unsigned long)
#define CMD_WRITE_GPIO   _IOW('G', 2, unsigned long)
#define CMD_SET_DIRECTION _IOW('G', 3, unsigned long)
#define CMD_READ_GPIO2	 _IOR('g', 6, unsigned long)
#define CMD_WRITE_GPIO2   _IOW('g', 7, unsigned long)
#define CMD_SET_DIRECTION_GPIO2 _IOW('g', 8, unsigned long)

#define GPIO_READ 0
#define GPIO_WRITE 1


struct gpio_cmd {
	int gpio;
	int value;
};

#define  ORION_GPIO_BASE 0x10260000
#define  ORION_GPIO_SIZE 0x110

#define REG_GPIO_SWPORTA_DR             0x20
#define REG_GPIO_SWPORTA_DDR            0x24
#define REG_GPIO_EXT_PORTA              0x60


#define REG_GPIO_SWPORTB_DR             0x34
#define REG_GPIO_SWPORTB_DDR            0x38
#define REG_GPIO_EXT_PORTB              0x64

#define GPIO_MAJOR  	0
#define GPIO_NR_DEVS	55

#define GPIO_PINMUXA    0x30 
#define GPIO_PINMUXB    0x44
 
static int gpio2_major = GPIO_MAJOR;
static int gpio2_nr_devs = GPIO_NR_DEVS;



module_param(gpio2_major, int, GPIO_MAJOR);
module_param(gpio2_nr_devs, int, GPIO_NR_DEVS);
MODULE_PARM_DESC(gpio2_major, "GPIO2 major number");
MODULE_PARM_DESC(gpio2_nr_devs, "GPIO2 number of lines");


typedef struct gpio_devs {
	u32 gpio_id;
	struct cdev cdev;
} gpio_devs_t;

static struct class_simple *csm120x_gpio2_class;

/*
 * The devices
 */
struct gpio_devs *gpio2_devices = NULL;	/* allocated in gpio_init_module */

/* function declaration ------------------------------------- */
static int __init orion_gpio2_init(void);
static void __exit orion_gpio2_exit(void);

static void gpio2_setup_cdev(struct gpio_devs *dev, int index);

static ssize_t orion_gpio2_write(struct file *filp, const char __user * buf,
			  size_t count, loff_t * f_pos);
static ssize_t orion_gpio2_read(struct file *filp, char __user * buf, size_t count,
			 loff_t * f_pos);
static int orion_gpio2_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		      unsigned long arg);
static int orion_gpio2_open(struct inode *inode, struct file *file);
static int orion_gpio2_close (struct inode *inode, struct file *file);
int orion_gpio2_register_module_set(const char * module, unsigned int  orion_module_status);

static spinlock_t gpio2_lock;
static volatile unsigned int *gpio2_base;
static unsigned int gpio_status1 = 0;
static unsigned int gpio_status2 = 0;
static unsigned int gpio_pinmux_status1 =0;
static unsigned int gpio_pinmux_status2 =0;
static struct proc_dir_entry *gpio2_proc_entry = NULL;
//static struct proc_dir_entry *system_config_proc_entry=NULL;
/* ----------------------------------------------------------- */

/* gpio device struct */
struct file_operations gpio2_fops = {
	.owner = THIS_MODULE,
	.read = orion_gpio2_read,
	.write = orion_gpio2_write,
	.ioctl = orion_gpio2_ioctl,
	.open = orion_gpio2_open,
	.release  = orion_gpio2_close,
};


int
gpio2_hw_set_direct(int gpio_id, int dir)
{
	unsigned int flags;
	spin_lock(&gpio2_lock);
	if((gpio_id < 32)&&(gpio_id >= 0))
	{
		
		flags = (1<<gpio_id);
		if (dir == GPIO_READ) {
			gpio2_base[REG_GPIO_SWPORTA_DDR >> 2] |= flags;	
		} else if (dir == GPIO_WRITE){
			gpio2_base[REG_GPIO_SWPORTA_DDR >> 2] &= ~flags;	
		} else{
			return -1;
		}
		
	}
	else if((gpio_id >= 32)&&(gpio_id < 55))
	{
		gpio_id = gpio_id - 32;
		flags = (1<<gpio_id);
		if (dir == GPIO_READ) {
			gpio2_base[REG_GPIO_SWPORTB_DDR >> 2] |= flags;
		} else if (dir == GPIO_WRITE){
			gpio2_base[REG_GPIO_SWPORTB_DDR >> 2] &= ~flags;
		} else{
			return -1;
		}
	}	

	spin_unlock(&gpio2_lock);
	return 0;
}

static unsigned short
gpio2_hw_read(int gpio_id)
{
	if((gpio_id < 32)&&(gpio_id >= 0))
	{
		return (gpio2_base[REG_GPIO_EXT_PORTA >> 2] >> gpio_id) & 0x1;
	}
	else if((gpio_id >= 32)&&(gpio_id < 55))
	{
		gpio_id = gpio_id - 32;
		return (gpio2_base[REG_GPIO_EXT_PORTB >> 2] >> gpio_id) & 0x1;
	}
	return 0;
}

int
gpio2_hw_write(int gpio_id, unsigned short data)
{
	unsigned int flags ;
	if((gpio_id < 32)&&(gpio_id >= 0))
	{
		flags = (1<<gpio_id);
		if (!data) {
			gpio2_base[REG_GPIO_SWPORTA_DR>>2 ] &= ~flags;
		} else {
			gpio2_base[REG_GPIO_SWPORTA_DR>>2 ] |= flags;
		}
	}
	else if((gpio_id >= 32)&&(gpio_id < 55))
	{
		gpio_id = gpio_id - 32;
		flags = (1<<gpio_id);
		if (!data) {
			gpio2_base[REG_GPIO_SWPORTB_DR>>2 ] &= ~flags;
		} else {
			gpio2_base[REG_GPIO_SWPORTB_DR>>2 ] |= flags;
		}
	}
	return 0;
}

static void
gpio2_setup_cdev(struct gpio_devs *dev, int index)
{
	int errorcode = 0;
	char device_name[80];

	int devno = MKDEV(gpio2_major, index);

	cdev_init(&dev->cdev, &gpio2_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &gpio2_fops;

	errorcode = cdev_add(&dev->cdev, devno, 1);
	if (errorcode) {
		printk(KERN_NOTICE "GPIO2: Error %d adding gpio2 %d", errorcode,
		       index);
	}

	snprintf(device_name, sizeof (device_name), "gpio2_%d", index);
	class_simple_device_add(csm120x_gpio2_class, devno, NULL, device_name);
#if 0
	#ifdef  CONFIG_DEVFS_FS 
	errorcode =
	    devfs_mk_cdev(devno, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP,
			  device_name);
	if (errorcode) {
		printk(KERN_NOTICE "GPIO2: Error %d make gpio2 node %d",
		       errorcode, index);
	}
	#endif
#endif
}

static void gpio2_configure_status_bit(unsigned int bit, const char * configure_cmd, char is_pinmux)
{

	if((bit >= 0)&&(bit < 32))
	{
		if (strncmp("CLR", configure_cmd, 3) == 0){
			gpio_status1 &= ~(1<<bit);
			if (is_pinmux == 1)
				gpio_pinmux_status1 &= ~(1<<bit);
		}
		if (strncmp("SET", configure_cmd, 3) == 0){
			gpio_status1 |= (1<<bit);
			if (is_pinmux == 1)
				gpio_pinmux_status1 |= (1<<bit);
		}
	}
	else
	{
		bit = bit - 32;
		if (strncmp("CLR", configure_cmd, 3) == 0){
			gpio_status2 &= ~(1<<bit);
			if (is_pinmux == 1)
				gpio_pinmux_status2 &= ~(1<<bit);
		}
		if (strncmp("SET", configure_cmd, 3) == 0){
			gpio_status2 |= (1<<bit);
			if (is_pinmux == 1)
				gpio_pinmux_status2 |= (1<<bit);
		}
	}
}

static int gpio2_get_status_bit(unsigned int gpio, char is_pinmux)
{
	int ret;
	if((gpio >= 0)&&(gpio < 32))
	{
		ret = ((gpio_status1 >> gpio) & 0x1);
		if (is_pinmux ==1)
			ret = ret & ((gpio_pinmux_status1 >> gpio) & 0x1);
	}
	else
	{
		gpio = gpio - 32;
		ret = ((gpio_status2 >> gpio) & 0x1);
		if (is_pinmux ==1)
			ret = ret & ((gpio_pinmux_status2 >> gpio) & 0x1);
	}
	return ret;
}

int orion_gpio2_register_module_set(const char * module, unsigned int  orion_module_status) /* status: 0 disable, 1 enable */
{
	unsigned int ret = gpio2_base[GPIO_PINMUXA>>2];
	unsigned int retb = gpio2_base[GPIO_PINMUXB>>2];
	if(strncmp("UART", module, 4)==0){ 
					if (orion_module_status == 1){
						gpio2_configure_status_bit(20 ,"SET", 1);
						gpio2_configure_status_bit(21 ,"SET", 1);
						if((ret&0x300000) != 0)
						{
							gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0x300000);
						}
						
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(20 ,"CLR", 1);
						gpio2_configure_status_bit(21 ,"CLR", 1);
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0x300000;
					}
 	}else if(strncmp("SPI", module, 3)==0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(16, "SET", 1);
						gpio2_configure_status_bit(17, "SET", 1);
						gpio2_configure_status_bit(18, "SET", 1);
						gpio2_configure_status_bit(19, "SET", 1);
						if((ret&0xf0000)!=0)
							gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0xf0000);
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(16, "CLR", 1);
						gpio2_configure_status_bit(17, "CLR", 1);
						gpio2_configure_status_bit(18, "CLR", 1);
						gpio2_configure_status_bit(19, "CLR", 1);
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0xf0000;
					}
 	}else if(strncmp("I2C", module, 3)==0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(27 ,"SET", 1);
						gpio2_configure_status_bit(28 ,"SET", 1);
						if((ret&0x18000000)!=0)
  						gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0x18000000);
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(27 ,"CLR", 1);
						gpio2_configure_status_bit(28 ,"CLR", 1);
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0x18000000;
					}
 	}else if (strncmp("DISPLAY", module, 7)== 0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(24 ,"SET", 1);
						gpio2_configure_status_bit(25 ,"SET", 1);
						gpio2_configure_status_bit(26 ,"SET", 1);
						if((ret&0x7000000)!=0)
							gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0x7000000);
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(24 ,"CLR", 1);
						gpio2_configure_status_bit(25 ,"CLR", 1);
						gpio2_configure_status_bit(26 ,"CLR", 1);
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0x7000000;
					}
	}else if (strncmp("GPIO1", module, 5)== 0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(0 ,"SET", 1);
						gpio2_configure_status_bit(1 ,"SET", 1);
						gpio2_configure_status_bit(2 ,"SET", 1);
						gpio2_configure_status_bit(3 ,"SET", 1);
						gpio2_configure_status_bit(4 ,"SET", 1);
						gpio2_configure_status_bit(5 ,"SET", 1);
						gpio2_configure_status_bit(6 ,"SET", 1);
						gpio2_configure_status_bit(7 ,"SET", 1);
						gpio2_configure_status_bit(8 ,"SET", 1);
						gpio2_configure_status_bit(9 ,"SET", 1);
						gpio2_configure_status_bit(10 ,"SET", 1);
						gpio2_configure_status_bit(11 ,"SET", 1);
						gpio2_configure_status_bit(12 ,"SET", 1);
						gpio2_configure_status_bit(13 ,"SET", 1);
						gpio2_configure_status_bit(14 ,"SET", 1);
						gpio2_configure_status_bit(15 ,"SET", 1);
						if((ret&0xffff)!=0)
							gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0xffff);
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(0 ,"CLR", 1);
						gpio2_configure_status_bit(1 ,"CLR", 1);
						gpio2_configure_status_bit(2 ,"CLR", 1);
						gpio2_configure_status_bit(3 ,"CLR", 1);
						gpio2_configure_status_bit(4 ,"CLR", 1);
						gpio2_configure_status_bit(5 ,"CLR", 1);
						gpio2_configure_status_bit(6 ,"CLR", 1);
						gpio2_configure_status_bit(7 ,"CLR", 1);
						gpio2_configure_status_bit(8 ,"CLR", 1);
						gpio2_configure_status_bit(9 ,"CLR", 1);
						gpio2_configure_status_bit(10 ,"CLR", 1);
						gpio2_configure_status_bit(11 ,"CLR", 1);
						gpio2_configure_status_bit(12 ,"CLR", 1);
						gpio2_configure_status_bit(13 ,"CLR", 1);
						gpio2_configure_status_bit(14 ,"CLR", 1);
						gpio2_configure_status_bit(15 ,"CLR", 1);
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0xffff;
					}
	}else if (strncmp("VIDOUT", module, 6)== 0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(29 ,"SET", 1);
						gpio2_configure_status_bit(30 ,"SET", 1);
						gpio2_configure_status_bit(31 ,"SET", 1);
						gpio2_configure_status_bit(32 ,"SET", 1);
						gpio2_configure_status_bit(33 ,"SET", 1);
						gpio2_configure_status_bit(34 ,"SET", 1);
						gpio2_configure_status_bit(35 ,"SET", 1);
						gpio2_configure_status_bit(36 ,"SET", 1);
						gpio2_configure_status_bit(37 ,"SET", 1);
						gpio2_configure_status_bit(38 ,"SET", 1);
						gpio2_configure_status_bit(39 ,"SET", 1);
						gpio2_configure_status_bit(40 ,"SET", 1);
						gpio2_configure_status_bit(41 ,"SET", 1);
						gpio2_configure_status_bit(42 ,"SET", 1);
						gpio2_configure_status_bit(43 ,"SET", 1);
						gpio2_configure_status_bit(44 ,"SET", 1);
						gpio2_configure_status_bit(45 ,"SET", 1);
						
						if(((ret&0xe0000000)!=0) || ((retb&0x3fff) != 0))
						{
							gpio2_base[(GPIO_PINMUXA >> 2)]&=(~0xe0000000);
							gpio2_base[(GPIO_PINMUXB >> 2)]&=(~0x3fff);
						}
						
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(29 ,"CLR", 1);
						gpio2_configure_status_bit(30 ,"CLR", 1);
						gpio2_configure_status_bit(31 ,"CLR", 1);
						gpio2_configure_status_bit(32 ,"CLR", 1); // B-0
						gpio2_configure_status_bit(33 ,"CLR", 1); // B-1
						gpio2_configure_status_bit(34 ,"CLR", 1); // B-2
						gpio2_configure_status_bit(35 ,"CLR", 1); // B-3
						gpio2_configure_status_bit(36 ,"CLR", 1); // B-4
						gpio2_configure_status_bit(37 ,"CLR", 1); // B-5
						gpio2_configure_status_bit(38 ,"CLR", 1); // B-6
						gpio2_configure_status_bit(39 ,"CLR", 1); // B-7
						gpio2_configure_status_bit(40 ,"CLR", 1); // B-8
						gpio2_configure_status_bit(41 ,"CLR", 1); // B-9
						gpio2_configure_status_bit(42 ,"CLR", 1); // B-10
						gpio2_configure_status_bit(43 ,"CLR", 1); // B-11
						gpio2_configure_status_bit(44 ,"CLR", 1); // B-12
						gpio2_configure_status_bit(45 ,"CLR", 1); // B-13
						gpio2_base[(GPIO_PINMUXA >> 2)]|=0xe0000000;
						gpio2_base[(GPIO_PINMUXB >> 2)]|=0x3fff;
					}
	}else if(strncmp("VIB", module, 3)==0){
					if (orion_module_status == 1){
						gpio2_configure_status_bit(46, "SET", 1); // B-14
						gpio2_configure_status_bit(47, "SET", 1); // B-15
						gpio2_configure_status_bit(48, "SET", 1); // B-16
						gpio2_configure_status_bit(49, "SET", 1); // B-17
						gpio2_configure_status_bit(50, "SET", 1); // B-18
						gpio2_configure_status_bit(51, "SET", 1); // B-19
						gpio2_configure_status_bit(52, "SET", 1); // B-20
						gpio2_configure_status_bit(53, "SET", 1); // B-21
						gpio2_configure_status_bit(54, "SET", 1); // B-22
						if((retb&0x7fc000)!=0)
						{
							gpio2_base[(GPIO_PINMUXB >> 2)]&=(~0x7fc000);
						}
					}
					if (orion_module_status == 0){
						gpio2_configure_status_bit(46, "CLR", 1);
						gpio2_configure_status_bit(47, "CLR", 1);
						gpio2_configure_status_bit(48, "CLR", 1);
						gpio2_configure_status_bit(49, "CLR", 1);
						gpio2_configure_status_bit(50, "CLR", 1);
						gpio2_configure_status_bit(51, "CLR", 1);
						gpio2_configure_status_bit(52, "CLR", 1);
						gpio2_configure_status_bit(53, "CLR", 1);
						gpio2_configure_status_bit(54, "CLR", 1);
						gpio2_base[(GPIO_PINMUXB >> 2)]|=0x7fc000;
					}
	}else {
		return 0;
	}

 return 1;
}

EXPORT_SYMBOL(orion_gpio2_register_module_set);
EXPORT_SYMBOL(gpio2_hw_write);
EXPORT_SYMBOL(gpio2_hw_set_direct);
static int
orion_gpio2_open(struct inode *inode, struct file *file)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	if (gpio2_get_status_bit(m,1) != 0 )
		{
			printk("\n EBUSY   \n");
			return -EBUSY;
		}
	gpio2_configure_status_bit(m ,"SET",0);
	return 0;
}

static int 
orion_gpio2_close (struct inode *inode, struct file *file)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	if (gpio2_get_status_bit(m,1) != 0 )
		return -EBUSY;
	gpio2_configure_status_bit(m ,"CLR",0);
	return 0;
}

static ssize_t
orion_gpio2_write(struct file *file, const char __user * data,
	   size_t len, loff_t * ppos)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	size_t i;
	for (i = 0; i < len; ++i) {
		char c;
		if (get_user(c, data + i))
			return -EFAULT;
		switch (c) {
		case '0':
			gpio2_hw_write(m, 0);
			break;
		case '1':
			gpio2_hw_write(m, 1);
			break;
		case 'O':
			gpio2_hw_set_direct(m, GPIO_WRITE);
			break;
		case 'o':
			gpio2_hw_set_direct(m, GPIO_READ);
			break;
		}
	}

	return len;
}

static int gpio2_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int gpio,gpiob;
	int len=0;
	for (gpio=0;gpio<55;gpio++){
		if(gpio < 32)
		{
			if ((gpio_status1>>gpio) & 0x1)
			{
				if ((gpio_pinmux_status1>>gpio) & 0x1)
					len += sprintf(page+len,"==> GPIO2a %d is held by other component. \n",gpio);
				else
					len += sprintf(page+len,"==> GPIO2a %d is using. \n",gpio);

			}
			else
			{
				len += sprintf(page+len,"==> GPIO %d is free.\n",gpio);
			}
		}
		else
		{
			gpiob = gpio - 32;	
			if ((gpio_status2>>gpiob) & 0x1)
			{
				if ((gpio_pinmux_status2>>gpiob) & 0x1)
					len += sprintf(page+len,"==> GPIO2b %d(%d) is held by other component. \n",gpiob,gpio);
				else
					len += sprintf(page+len,"==> GPIO2b %d(%d) is using. \n",gpiob,gpio);

			}
			else
			{
				len += sprintf(page+len,"==> GPIO2b %d(%d) free.\n",gpiob,gpio);
			}
		}	
	}
	return len;
}

static int gpio2_proc_write(struct file *file, const char __user *buffer,
			   unsigned long count, void *data)
{
	unsigned int addr;
	unsigned int val;

	const char *cmd_line = buffer;;

	if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 0);
		val = gpio2_base[addr>>2];
		printk("readed addr[0x%x] = 0x%x \n", addr, val);
	} else if (strncmp("ww", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 0);
		val = simple_strtol(&cmd_line[7], NULL, 0);
		gpio2_base[addr>>2] = val;
	}

	return count;
}


static ssize_t
orion_gpio2_read(struct file *file, char __user * buf, size_t len, loff_t * ppos)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	int value;
	value = gpio2_hw_read(m);
	if (put_user(value ? '1' : '0', buf))
		return -EFAULT;

	return 1;
}

static int
orion_gpio2_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	struct gpio_cmd gpio_cmd;
	unsigned long direction;
	unsigned m = iminor(file->f_dentry->d_inode);
	switch (cmd) {
	case CMD_READ_GPIO2:
	case CMD_WRITE_GPIO2:
		if (copy_from_user
		    (&gpio_cmd, (void *) arg, sizeof (struct gpio_cmd)))
			return -EFAULT;

		switch (cmd) {
		case CMD_READ_GPIO2:
			gpio_cmd.value = gpio2_hw_read(gpio_cmd.gpio);
			if (copy_to_user
			    ((void *) arg, &gpio_cmd, sizeof (struct gpio_cmd)))
				return -EFAULT;
			else
				return 0;

		case CMD_WRITE_GPIO2:
			gpio2_hw_set_direct(gpio_cmd.gpio, GPIO_WRITE);
			gpio2_hw_write(gpio_cmd.gpio, gpio_cmd.value);
			return 0;
		}
		break;
	case CMD_SET_DIRECTION_GPIO2:
		direction = arg;
		if (direction == GPIO_READ){
			gpio2_hw_set_direct(m,GPIO_READ);
		}else if ( direction == GPIO_WRITE){
			gpio2_hw_set_direct(m,GPIO_WRITE);
		}else {
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#if defined(CONFIG_ARCH_ORION_CSM1201)

#define GPIO_PINMUX() do {udelay(20);} while(0)

#else

#define GPIO_PINMUX() do {} while(0)

#endif

#define ORION_GPIO_IRQ            10
#define REG_GPIO_INTMASKA		0x28
#define REG_GPIO_INTMASKB		0x3c
#define REG_GPIOA_INT_S		0x68
#define REG_GPIOB_INT_S		0x6c

static irqreturn_t gpio2_dev_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
//	unsigned long  flags;
//	int intr_status;

	//printk("Entered gpio intrrupt! status: 0x%x\n", gpio_base[REG_GPIO_INT_S>>2]);

	return IRQ_HANDLED;
}

#if defined(CONFIG_ARCH_ORION_CSM1201)

#define GPIO_REGINTR() do { \
	gpio2_base[REG_GPIO_INTMASKA >> 2] |= 0xffffffff; \
	gpio2_base[REG_GPIO_INTMASKB >> 2] |= 0x7fffff; \
	if (request_irq(ORION_GPIO_IRQ, gpio2_dev_interrupt, 0, "gpio2", NULL)) { \
		return -EBUSY; \
	} \
} while(0)

#else

#define GPIO_REGINTR() do {} while(0)

#endif

static int __init
orion_gpio2_init(void)
{
	int result, i;
	dev_t dev = 0;
	int err=0;
	
	if (gpio2_major) {
		dev = MKDEV(gpio2_major, 0);
		result = register_chrdev_region(dev, gpio2_nr_devs, "gpio2");
	} else {
		result = alloc_chrdev_region(&dev, 0, gpio2_nr_devs, "gpio2");
		gpio2_major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "GPIO2: can't get major %d\n", gpio2_major);
		return result;
	}

	if (!request_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE, "ORION GPIO2")) {
		unregister_chrdev_region(dev, gpio2_nr_devs);
		return -EIO;
	}
	//static volatile unsigned char *gpio2_base  = 
		//(volatile unsigned int *)(ORION_GPIO_BASE);
	gpio2_base =
	    (volatile unsigned int *) ioremap(ORION_GPIO_BASE,
						ORION_GPIO_SIZE);
	if (!gpio2_base) {
		unregister_chrdev_region(dev, gpio2_nr_devs);
		printk(KERN_WARNING "GPIO2: ioremap failed.\n");
		return -EIO;
	}
	
	csm120x_gpio2_class = class_simple_create(THIS_MODULE,"gpio2");

    if (IS_ERR(csm120x_gpio2_class)){
        err = PTR_ERR(csm120x_gpio2_class);
		unregister_chrdev_region(dev, gpio2_nr_devs);
        printk(KERN_WARNING "GPIO2: class create failed.\n");
        return -EIO;
    }

	gpio2_devices =
	    kmalloc(gpio2_nr_devs * sizeof (struct gpio_devs), GFP_KERNEL);
	if (!gpio2_devices) {
		iounmap((void *) gpio2_base);
		release_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE);
		unregister_chrdev_region(dev, gpio2_nr_devs);
		return -ENOMEM;
	}
	memset(gpio2_devices, 0, gpio2_nr_devs * sizeof (struct gpio_devs));

	/* Initialize each device. */
	for (i = 0; i < gpio2_nr_devs; i++) {
		gpio2_devices[i].gpio_id = i;
		gpio2_setup_cdev(&gpio2_devices[i], i);
	}

	spin_lock_init(&gpio2_lock);

	printk(KERN_INFO "ORION GPIO2 at 0x%x, %d lines\n", ORION_GPIO_BASE,
	       gpio2_nr_devs);

	gpio2_proc_entry = create_proc_entry("gpio2", 0, NULL);
	if (NULL != gpio2_proc_entry) {
		gpio2_proc_entry->write_proc = &gpio2_proc_write;
		gpio2_proc_entry->read_proc = &gpio2_proc_read;
	}

	#if 0
	system_config_proc_entry = create_proc_entry("systemconfig", 0, NULL);
	if (NULL != system_config_proc_entry) {
        system_config_proc_entry->read_proc = &system_config_proc_read;
	}
	#endif 

	GPIO_PINMUX();
	gpio2_base[(GPIO_PINMUXA >> 2)] |= 0xc00000;
	gpio2_base[(GPIO_PINMUXB >> 2)] |= 0x7fc000;
	orion_gpio2_register_module_set("VIDOUT",1);
	orion_gpio2_register_module_set("DISPLAY",1);
	orion_gpio2_register_module_set("GPIO1",1);
	orion_gpio2_register_module_set("SPI",1);
	orion_gpio2_register_module_set("UART",1);
	orion_gpio2_register_module_set("I2C",1);
	//orion_gpio2_register_module_set("VIB",1);
	//orion_gpio2_register_module_set("VIB",0);

	return 0;
}

static void __exit
orion_gpio2_exit(void)
{
	int i;
	dev_t devno = MKDEV(gpio2_major, 0);

	if (gpio2_devices) {
		for (i = 0; i < gpio2_nr_devs; i++) {
			cdev_del(&gpio2_devices[i].cdev);
		}
		kfree(gpio2_devices);
		gpio2_devices = NULL;
	}

	iounmap((void *) gpio2_base);
	release_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE);
	unregister_chrdev_region(devno, gpio2_nr_devs);
}

module_init(orion_gpio2_init);
module_exit(orion_gpio2_exit);

MODULE_AUTHOR("Celestial Semiconductor");
MODULE_DESCRIPTION("Celestial Semiconductor GPIO2 driver");
MODULE_LICENSE("GPL");
