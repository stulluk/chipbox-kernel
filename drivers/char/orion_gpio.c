#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
//#include <linux/devfs_fs_kernel.h>
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
#define GPIO_READ 0
#define GPIO_WRITE 1


struct gpio_cmd {
	int gpio;
	int value;
};

#define  ORION_GPIO_BASE 0x101E4000
#define  ORION_GPIO_SIZE 0x100

#define REG_GPIO_SWPORTA_DR             0x000
#define REG_GPIO_SWPORTA_DDR            0x004
#define REG_GPIO_EXT_PORTA              0x050

#define GPIO_MAJOR  	0
#define GPIO_NR_DEVS	16

 
static int gpio_major = GPIO_MAJOR;
static int gpio_nr_devs = GPIO_NR_DEVS;

module_param(gpio_major, int, GPIO_MAJOR);
module_param(gpio_nr_devs, int, GPIO_NR_DEVS);
MODULE_PARM_DESC(gpio_major, "GPIO major number");
MODULE_PARM_DESC(gpio_nr_devs, "GPIO number of lines");

typedef struct gpio_devs {
	u32 gpio_id;
	struct cdev cdev;
} gpio_devs_t;

static struct class_simple *csm120x_gpio_class;
// should use in higher kernel version:static struct class *csm120x_gpio_class;
/*
 * The devices
 */
struct gpio_devs *gpio_devices = NULL;	/* allocated in gpio_init_module */

/* function declaration ------------------------------------- */
static int __init orion_gpio_init(void);
static void __exit orion_gpio_exit(void);

static void gpio_setup_cdev(struct gpio_devs *dev, int index);

static ssize_t orion_gpio_write(struct file *filp, const char __user * buf,
			  size_t count, loff_t * f_pos);
static ssize_t orion_gpio_read(struct file *filp, char __user * buf, size_t count,
			 loff_t * f_pos);
static int orion_gpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		      unsigned long arg);
static int orion_gpio_open(struct inode *inode, struct file *file);
static int orion_gpio_close (struct inode *inode, struct file *file);

static spinlock_t gpio_lock;
static volatile unsigned short *gpio_base;
static unsigned int gpio_status = 0;
static unsigned int gpio_pinmux_status =0;
static struct proc_dir_entry *gpio_proc_entry = NULL;
static struct proc_dir_entry *system_config_proc_entry=NULL;
/* ----------------------------------------------------------- */

/* gpio device struct */
struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read = orion_gpio_read,
	.write = orion_gpio_write,
	.ioctl = orion_gpio_ioctl,
	.open = orion_gpio_open,
	.release  = orion_gpio_close,
};


int
gpio_hw_set_direct(int gpio_id, int dir)
{
	unsigned short flags = (1 << gpio_id);

	spin_lock(&gpio_lock);

	if (dir == GPIO_READ) {
		gpio_base[REG_GPIO_SWPORTA_DDR >> 1] &= ~flags;
	} else if (dir == GPIO_WRITE){
		gpio_base[REG_GPIO_SWPORTA_DDR >> 1] |= flags;
	} else{
		return -1;
	}

	spin_unlock(&gpio_lock);

	return 0;
}

static unsigned short
gpio_hw_read(int gpio_id)
{
	return (gpio_base[REG_GPIO_EXT_PORTA >> 1] >> gpio_id) & 0x1;
}

int
gpio_hw_write(int gpio_id, unsigned short data)
{
	unsigned short flags = (1 << gpio_id);

	spin_lock(&gpio_lock);

	if (!data) {
		gpio_base[REG_GPIO_SWPORTA_DR >> 1] &= ~flags;
	} else {
		gpio_base[REG_GPIO_SWPORTA_DR >> 1] |= flags;
	}

	spin_unlock(&gpio_lock);

	return 0;
}

static void
gpio_setup_cdev(struct gpio_devs *dev, int index)
{
	int errorcode = 0;
	char device_name[20];

	int devno = MKDEV(gpio_major, index);

	cdev_init(&dev->cdev, &gpio_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &gpio_fops;

	errorcode = cdev_add(&dev->cdev, devno, 1);
	if (errorcode) {
		printk(KERN_NOTICE "GPIO: Error %d adding gpio %d", errorcode,
		       index);
	}

	snprintf(device_name, sizeof (device_name), "%d", index);

    //device_create(csm120x_gpio_class, NULL, devno, device_name);
    class_simple_device_add(csm120x_gpio_class, devno, NULL, device_name);

// 	snprintf(device_name, sizeof (device_name), "gpio/%d", index);

//   	errorcode =
//   	    devfs_mk_cdev(devno, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP,
//   			  device_name);
//   	if (errorcode) {
//   		printk(KERN_NOTICE "GPIO: Error %d make gpio node %d",
//   		       errorcode, index);
//   	}

}

static void gpio_configure_status_bit(unsigned int bit, const char * configure_cmd, char is_pinmux)
{
		
	if (strncmp("CLR", configure_cmd, 3) == 0){
		gpio_status &= ~(1<<bit);
		if (is_pinmux == 1)
			gpio_pinmux_status &= ~(1<<bit);
	}
	if (strncmp("SET", configure_cmd, 3) == 0){
		gpio_status |= (1<<bit);
		if (is_pinmux == 1)
			gpio_pinmux_status |= (1<<bit);
	}			
}

static int gpio_get_status_bit(unsigned int gpio, char is_pinmux)
{
	int ret;
	ret = ((gpio_status >> gpio) & 0x1);
	if (is_pinmux ==1)
		ret = ret & ((gpio_pinmux_status >> gpio) & 0x1);
	return ret;
}

int orion_gpio_register_module_status(const char * module, unsigned int  orion_module_status) /* status: 0 disable, 1 enable */
{
	
	if(strncmp("RC", module, 2)==0){ 
					if (orion_module_status == 1){
						gpio_configure_status_bit(11 ,"SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(11 ,"CLR", 1);
					}
 	}else if(strncmp("FPC", module, 3)==0){
					if (orion_module_status == 1){
						gpio_configure_status_bit(10, "SET", 1);
						gpio_configure_status_bit(8,  "SET", 1);
						gpio_configure_status_bit(9,  "SET", 1);
						gpio_configure_status_bit(7,  "SET", 1);
						gpio_configure_status_bit(5,  "SET", 1);
						gpio_configure_status_bit(4,  "SET", 1);
						gpio_configure_status_bit(2,  "SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(10, "CLR", 1);
						gpio_configure_status_bit(9,  "CLR", 1);
						gpio_configure_status_bit(8,  "CLR", 1);
						gpio_configure_status_bit(7,  "CLR", 1);
						gpio_configure_status_bit(5,  "CLR", 1);
						gpio_configure_status_bit(4,  "CLR", 1);
						gpio_configure_status_bit(2,  "CLR", 1);
					}
 	}else if(strncmp("I2C-2", module, 5)==0){
					if (orion_module_status == 1){
						gpio_configure_status_bit(1 ,"SET", 1);
						gpio_configure_status_bit(0 ,"SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(1 ,"CLR", 1);
						gpio_configure_status_bit(0 ,"CLR", 1);
					}
 	}else if (strncmp("IRDA", module, 4)== 0){
					if (orion_module_status == 1){
						gpio_configure_status_bit(6 ,"SET", 1);
						gpio_configure_status_bit(3 ,"SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(6 ,"CLR", 1);
						gpio_configure_status_bit(3 ,"CLR", 1);
					}
	}else if (strncmp("PATA", module, 4)== 0){
					if (orion_module_status == 1){
						gpio_configure_status_bit(14 ,"SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(14 ,"CLR", 1);
					}
	}else if (strncmp("PCMCIA", module, 6)== 0){
					if (orion_module_status == 1){
						gpio_configure_status_bit(12 ,"SET", 1);
					}
					if (orion_module_status == 0){
						gpio_configure_status_bit(12 ,"CLR", 1);
					}
	}else {
		return 0;
	}

 return 1;
}

EXPORT_SYMBOL(orion_gpio_register_module_status);
EXPORT_SYMBOL(gpio_hw_write);
EXPORT_SYMBOL(gpio_hw_set_direct);
static int
orion_gpio_open(struct inode *inode, struct file *file)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	if (gpio_get_status_bit(m,1) != 0 )
		return -EBUSY;
	gpio_configure_status_bit(m ,"SET",0);
	return 0;
}

static int 
orion_gpio_close (struct inode *inode, struct file *file)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	if (gpio_get_status_bit(m,1) != 0 )
		return -EBUSY;
	gpio_configure_status_bit(m ,"CLR",0);
	return 0;
}

static ssize_t
orion_gpio_write(struct file *file, const char __user * data,
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
			gpio_hw_write(m, 0);
			break;
		case '1':
			gpio_hw_write(m, 1);
			break;
		case 'O':
			gpio_hw_set_direct(m, GPIO_WRITE);
			break;
		case 'o':
			gpio_hw_set_direct(m, GPIO_READ);
			break;
		}
	}

	return len;
}

static int gpio_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int gpio;
	int len=0;
	for (gpio=0;gpio<16;gpio++){
		if ((gpio_status>>gpio) & 0x1){
			if ((gpio_pinmux_status>>gpio) & 0x1)
				len += sprintf(page+len,"==> GPIO %d is held by other component. \n",gpio);
			else
				len += sprintf(page+len,"==> GPIO %d is using. \n",gpio);

		}
		else 
			len += sprintf(page+len,"==> GPIO %d is free.\n",gpio);
		
	}
	return len;
}

static int gpio_proc_write(struct file *file, const char __user *buffer,
			   unsigned long count, void *data)
{
	unsigned short addr;
	unsigned short val;

	const char *cmd_line = buffer;;

	if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = gpio_base[addr>>1];
		printk(" readw [0x%04x] = 0x%04x \n", addr, val);
	} else if (strncmp("ww", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		gpio_base[addr>>1] = val;
	}

	return count;
}

static int system_config_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len=0;

#if defined(CONFIG_ARCH_ORION_CSM1201_J)
    len += sprintf(page+len,"Linux kernel is configured for CSM1201_J. \n");
#elif defined(CONFIG_ARCH_ORION_CSM1201)
    len += sprintf(page+len,"Linux kernel is configured for CSM1201. \n");
#elif defined(CONFIG_ARCH_ORION_CSM1200_J)
    len += sprintf(page+len,"Linux kernel is configured for CSM1200_J. \n");
#elif defined(CONFIG_ARCH_ORION_CSM1200)
    len += sprintf(page+len,"Linux kernel is configured for CSM1200. \n");
#elif deined(CONFIG_ARCH_ORION_CSM11100)
    len += sprintf(page+len,"Linux kernel is configured for CSM1100. \n");
#else
    len += sprintf(page+len,"Linux kernel is configured for UNKNOWN system. \n");
#endif


#if defined(CONFIG_BASE_DDR_ADDR)
    len += sprintf(page+len,"Memsize is %dMB. \n",CONFIG_BASE_DDR_ADDR/(1024*1024));
	len += sprintf(page+len,"FB0 buffer size is %dMB.\n",FB0_SIZE/(1024*1024));
	len += sprintf(page+len,"FB1 buffer size is %dMB. \n",FB1_SIZE/(1024*1024));
	len += sprintf(page+len,"Xport buffer size is %dMB. \n",XPORT_SIZE/(1024*1024));
	len += sprintf(page+len,"Audio buffer size is %dKB. \n",AUD_SIZE/1024);
	len += sprintf(page+len,"DBP0 start address is 0x%x. \n",DPB0_REGION);
	len += sprintf(page+len,"DBP0 size is %dMB. \n",DPB0_SIZE/(1024*1024));
	len += sprintf(page+len,"CBP0 start address is 0x%x. \n",CPB0_REGION);
	len += sprintf(page+len,"CPB0 size is %dKB. \n",CPB0_SIZE/1024);
	len += sprintf(page+len,"DBP1 is %dMB. \n",DPB1_SIZE/(1024*1024));
	len += sprintf(page+len,"CPB1 is %dKB. \n",CPB1_SIZE/1024);
	
	len += sprintf(page+len,"VIDEO_USER_DATA_SIZE is %dKB. \n",VIDEO_USER_DATA_SIZE/1024);
	len += sprintf(page+len,"VIDEO_USER_DATA start address is 0x%x(=%dMB). \n",VIDEO_USER_DATA_REGION, VIDEO_USER_DATA_REGION/(1024*1024));
    
#if defined(HD2SD_DATA_SIZE)
	len += sprintf(page+len,"HD2SD data buffer size is %dKB. \n",HD2SD_DATA_SIZE/1024);
	len += sprintf(page+len,"HD2SD start is 0x%x(=%dMB). \n",HD2SD_DATA_REGION,HD2SD_DATA_REGION/(1024*1024));
    len += sprintf(page+len,"System/Firmwares used memory size is %dMB. \n",HD2SD_DATA_SIZE/(1024*1024));
#else 
    len += sprintf(page+len,"System/Firmware used memory size is %dMB. \n", VIDEO_USER_DATA_REGION/(1024*1024));
#endif

#else
    len += sprintf(page+len,"Cannot find Maxmium DDR Size. \n");    
#endif


	return len;
}

static ssize_t
orion_gpio_read(struct file *file, char __user * buf, size_t len, loff_t * ppos)
{
	unsigned m = iminor(file->f_dentry->d_inode);
	int value;

	value = gpio_hw_read(m);
	if (put_user(value ? '1' : '0', buf))
		return -EFAULT;

	return 1;
}

static int
orion_gpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	struct gpio_cmd gpio_cmd;
	unsigned long direction;
	unsigned m = iminor(file->f_dentry->d_inode);
	switch (cmd) {
	case CMD_READ_GPIO:
	case CMD_WRITE_GPIO:
		if (copy_from_user
		    (&gpio_cmd, (void *) arg, sizeof (struct gpio_cmd)))
			return -EFAULT;

		switch (cmd) {
		case CMD_READ_GPIO:
			gpio_cmd.value = gpio_hw_read(gpio_cmd.gpio);
			if (copy_to_user
			    ((void *) arg, &gpio_cmd, sizeof (struct gpio_cmd)))
				return -EFAULT;
			else
				return 0;

		case CMD_WRITE_GPIO:
			gpio_hw_set_direct(gpio_cmd.gpio, GPIO_WRITE);
			gpio_hw_write(gpio_cmd.gpio, gpio_cmd.value);
			return 0;
		}
		break;
	case CMD_SET_DIRECTION:
		//if (get_user(direction, (int __user *)arg))
		//	return -EFAULT;
		direction = arg;
		if (direction == GPIO_READ){
			gpio_hw_set_direct(m,GPIO_READ);
		}else if ( direction == GPIO_WRITE){
			gpio_hw_set_direct(m,GPIO_WRITE);
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

#define ORION_GPIO_IRQ            6
#define REG_GPIO_INTEN		0x30
#define REG_GPIO_INT_S		0x40

static irqreturn_t gpio_dev_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
	//unsigned long  flags;
	//int intr_status;

	printk("Entered gpio intrrupt! status: 0x%x\n", gpio_base[REG_GPIO_INT_S>>1]);

	return IRQ_HANDLED;
}

#if defined(CONFIG_ARCH_ORION_CSM1201)

#define GPIO_REGINTR() do { \
	gpio_base[REG_GPIO_INTEN >> 1] |= 0xfffa; \
	if (request_irq(ORION_GPIO_IRQ, gpio_dev_interrupt, 0, "gpio", NULL)) { \
		return -EBUSY; \
	} \
} while(0)

#else

#define GPIO_REGINTR() do {} while(0)

#endif


static int __init
orion_gpio_init(void)
{
	int result, i;
	dev_t dev = 0;
    int err=0;
	if (gpio_major) {
		dev = MKDEV(gpio_major, 0);
		result = register_chrdev_region(dev, gpio_nr_devs, "gpio");
	} else {
		result = alloc_chrdev_region(&dev, 0, gpio_nr_devs, "gpio");
		gpio_major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "GPIO: can't get major %d\n", gpio_major);
		return result;
	}

	if (!request_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE, "ORION GPIO")) {
		unregister_chrdev_region(dev, gpio_nr_devs);
		return -EIO;
	}

	gpio_base =
	    (volatile unsigned short *) ioremap(ORION_GPIO_BASE,
						ORION_GPIO_SIZE);
	if (!gpio_base) {
		unregister_chrdev_region(dev, gpio_nr_devs);
		printk(KERN_WARNING "GPIO: ioremap failed.\n");
		return -EIO;
	}

    csm120x_gpio_class = class_simple_create(THIS_MODULE,"gpio");

    if (IS_ERR(csm120x_gpio_class)){
        err = PTR_ERR(csm120x_gpio_class);
		unregister_chrdev_region(dev, gpio_nr_devs);
        printk(KERN_WARNING "GPIO: class create failed.\n");
        return -EIO;
    }

	gpio_devices =
	    kmalloc(gpio_nr_devs * sizeof (struct gpio_devs), GFP_KERNEL);
	if (!gpio_devices) {
		iounmap((void *) gpio_base);
		release_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE);
		unregister_chrdev_region(dev, gpio_nr_devs);
		return -ENOMEM;
	}
	memset(gpio_devices, 0, gpio_nr_devs * sizeof (struct gpio_devs));

	/* Initialize each device. */
	for (i = 0; i < gpio_nr_devs; i++) {
		gpio_devices[i].gpio_id = i;
		gpio_setup_cdev(&gpio_devices[i], i);
	}

	spin_lock_init(&gpio_lock);

	printk(KERN_INFO "ORION GPIO at 0x%x, %d lines\n", ORION_GPIO_BASE,
	       gpio_nr_devs);

	gpio_proc_entry = create_proc_entry("gpio", 0, NULL);
	if (NULL != gpio_proc_entry) {
		gpio_proc_entry->write_proc = &gpio_proc_write;
		gpio_proc_entry->read_proc = &gpio_proc_read;
	}

	system_config_proc_entry = create_proc_entry("systemconfig", 0, NULL);
	if (NULL != system_config_proc_entry) {
        system_config_proc_entry->read_proc = &system_config_proc_read;
	}

	GPIO_PINMUX();

	/* Enable cvbs/svideo */
	gpio_hw_set_direct(0, GPIO_WRITE);
	gpio_hw_write(0, 1);;
/*        GPIO_REGINTR();*/

	return 0;
}

static void __exit
orion_gpio_exit(void)
{
	int i;
	dev_t devno = MKDEV(gpio_major, 0);

	if (gpio_devices) {
		for (i = 0; i < gpio_nr_devs; i++) {
			cdev_del(&gpio_devices[i].cdev);
		}
		kfree(gpio_devices);
		gpio_devices = NULL;
	}

	iounmap((void *) gpio_base);
	release_mem_region(ORION_GPIO_BASE, ORION_GPIO_SIZE);
	unregister_chrdev_region(devno, gpio_nr_devs);
}

module_init(orion_gpio_init);
module_exit(orion_gpio_exit);

MODULE_AUTHOR("Celestial Semiconductor");
MODULE_DESCRIPTION("Celestial Semiconductor GPIO driver");
MODULE_LICENSE("GPL");
