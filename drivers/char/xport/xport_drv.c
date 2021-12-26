
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/devfs_fs_kernel.h>
//#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/semaphore.h>
  
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/firmware.h>

#include "xport_drv.h"
#include "xport_dma.h"
#include "xport_mips.h"
#include "xport_filter.h"

#if 0
#if defined(CONFIG_ARCH_ORION_CSM1201)
#include "fw1201/xport_fw_data.h"
#include "fw1201/xport_fw_text.h"
#else
#include "xport_fw_data.h"
#include "xport_fw_text.h"
#endif

#endif //del fw


#if defined(CONFIG_ARCH_ORION_CSM1200)
#define MAX_FW_DATA_SIZE  1024*4
#define MAX_FW_TEXT_SIZE  4096*4
#elif defined(CONFIG_ARCH_ORION_CSM1201)
#define MAX_FW_DATA_SIZE 2048*4
#define MAX_FW_TEXT_SIZE  3072*4
#endif


#define SLEEP_TIMEOUT 2  /* sec*/

void __iomem *xport_regs_base;
void __iomem *xport_mem_base;

static struct class_simple *csm120x_xport_class;
static struct platform_device *xport_pdev;

static unsigned int irq0_dev_id = XPORT_IRQ0_ID;
static unsigned int irq1_dev_id = XPORT_IRQ1_ID;

static unsigned int crc_err_idx = 0x0000ffff;
static unsigned int last_crc_error_idx = -1;

static DECLARE_WAIT_QUEUE_HEAD(crc_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(irq0_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(irq1_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(irq0_dma0_wait_queue);
static DECLARE_WAIT_QUEUE_HEAD(irq0_dma1_wait_queue);

//static struct semaphore read_sem;

static wait_queue_head_t filter_wait_queue[MAX_FILTER_NUM];

void xport_writeb(int a,int v)
{ 
	writeb(v, xport_regs_base + __OFFSET_ADDR__(a)); udelay(5); 
}

void xport_writew(int a,int v)
{
  	writew(v, xport_regs_base + __OFFSET_ADDR__(a)); udelay(5);
}

void xport_writel(int a,int v)
{
	writel(v, xport_regs_base + __OFFSET_ADDR__(a)); udelay(5);
}

unsigned char xport_readb(int a) 
{
	return readb( xport_regs_base    + __OFFSET_ADDR__(a) );
}

unsigned short xport_readw(int a)
{
     	return readw( xport_regs_base    + __OFFSET_ADDR__(a) );
}

unsigned int xport_readl(int a)
{
     	return readl( xport_regs_base    + __OFFSET_ADDR__(a) );
}

EXPORT_SYMBOL(xport_readb);
EXPORT_SYMBOL(xport_readw);
EXPORT_SYMBOL(xport_readl);
EXPORT_SYMBOL(xport_writeb);
EXPORT_SYMBOL(xport_writew);
EXPORT_SYMBOL(xport_writel);

static int xport_load_firmware(void)
{
	int ret,i;
    const struct firmware *xport_data_fw=NULL;
    const struct firmware *xport_text_fw=NULL;
	unsigned int wr_data;

    printk("loading xport firmware\n ");
    ret = request_firmware(&xport_data_fw, "xport_fw_data.bin", &(xport_pdev->dev));
    if (ret != 0 || xport_data_fw == NULL ) {
        printk(KERN_ERR "Failed to load xport firmware data section\n");
        return -1;
    }
    if (xport_data_fw->size > MAX_FW_DATA_SIZE){
        printk(KERN_ERR "Firmware data senction is too larage!\n");
        return -1;
    }

	printk("readed xport fw data section size =%d\n", xport_data_fw->size);
    ret = request_firmware(&xport_text_fw, "xport_fw_text.bin", &(xport_pdev->dev));
    if (ret != 0 || xport_text_fw == NULL) {
        printk(KERN_ERR "Failed to load xport firmware code section\n");
        return -1;
    }
    if (xport_text_fw->size > MAX_FW_TEXT_SIZE){
        printk(KERN_ERR "Firmware text senction is too larage!\n");
        return -1;
    }
    printk("readed xport fw text section size =%d\n", xport_text_fw->size);
    
	xport_writel(MIPS_FW_EN, 1);
    for (i = 0; i < xport_data_fw->size; i+=4) {
		wr_data = (xport_data_fw->data)[i+3]<<24 | (xport_data_fw->data)[i+2]<<16 | (xport_data_fw->data)[i+1]<<8 | (xport_data_fw->data)[i];
		xport_writel(MIPS_FW_WRITE_DATA+i, wr_data);
	}
	for (; i < MAX_FW_DATA_SIZE; i+=4) {
		xport_writel(MIPS_FW_WRITE_DATA +i, 0);
	}


    for (i = 0; i < xport_text_fw->size; i+=4) {
		wr_data = (xport_text_fw->data)[i+3]<<24 | (xport_text_fw->data)[i+2]<<16 | (xport_text_fw->data)[i+1]<<8 | (xport_text_fw->data)[i];
		xport_writel(MIPS_FW_WRITE_INST+i, wr_data);
	}

	xport_writel(MIPS_FW_EN, 0);

    printk("loded xport firmware\n");
    release_firmware(xport_data_fw);
    release_firmware(xport_text_fw);

	return 0;
}

// static int xport_load_firmware(void)
// {
// 	int i, wr_offset;
// 	unsigned int wr_data;

// 	xport_writel(MIPS_FW_EN, 1);

// 	wr_offset = sizeof(uiXport_MIPS_Data) / sizeof(uiXport_MIPS_Data[0]);
// #if defined(CONFIG_ARCH_ORION_CSM1200)
// 	if (wr_offset > 1024)
// #elif defined(CONFIG_ARCH_ORION_CSM1201)
// 	if (wr_offset > 2048)
// #endif
// 		return -1;	/* data overflow. */

// 	for (i = 0; i < wr_offset; i++) {
// 		wr_data = uiXport_MIPS_Data[i];
// 		wr_data = (wr_data << 24) | ((wr_data << 8) & 0xff0000) | ((wr_data >> 8) & 0xff00) | (wr_data >> 24);
// 		xport_writel(MIPS_FW_WRITE_DATA + (i << 2), wr_data);
// 	}

// 	for (; wr_offset < 1024; wr_offset++) {
// 		xport_writel(MIPS_FW_WRITE_DATA + (wr_offset << 2), 0);
// 	}

// 	wr_offset = sizeof(uiXport_MIPS_Inst) / sizeof(uiXport_MIPS_Inst[0]);
// #if defined(CONFIG_ARCH_ORION_CSM1200)
// 	if (wr_offset > 4096)
// #elif defined(CONFIG_ARCH_ORION_CSM1201)
// 	if (wr_offset > (1024*3))
// #endif
// 		return -1;	/* instruction overflow. */

// 	for (i = 0; i < wr_offset; i++) {
// 		wr_data = uiXport_MIPS_Inst[i];
// 		wr_data = (wr_data << 24) | ((wr_data << 8) & 0xff0000) | ((wr_data >> 8) & 0xff00) | (wr_data >> 24);
// 		xport_writel(MIPS_FW_WRITE_INST + (i << 2), wr_data);
// 	}

// 	xport_writel(MIPS_FW_EN, 0);

// 	return 0;
// }

static int xport_config_buf(void)
{
	/* config mips base addres */
	xport_mips_write(MIPS_EXTERNAL_BASE,XPORT_MIPS_BASE_ADDR); 

	xport_mips_write(MIPS_CHL_BUF_LOW_ADDR(0), CPB0_REGION);
	xport_mips_write(MIPS_CHL_BUF_UP_ADDR(0), CPB0_REGION + CPB0_SIZE);
	xport_mips_write(MIPS_CHL_DIR_LOW_ADDR(0), CPB0_DIR_REGION);
	xport_mips_write(MIPS_CHL_DIR_UP_ADDR(0), CPB0_DIR_REGION + CPB0_DIR_SIZE);

	xport_mips_write(MIPS_CHL_BUF_LOW_ADDR(1), CPB1_REGION);
	xport_mips_write(MIPS_CHL_BUF_UP_ADDR(1), CPB1_REGION + CPB1_SIZE);
	xport_mips_write(MIPS_CHL_DIR_LOW_ADDR(1), CPB1_DIR_REGION);
	xport_mips_write(MIPS_CHL_DIR_UP_ADDR(1), CPB1_DIR_REGION + CPB1_DIR_SIZE);

	xport_mips_write(MIPS_CHL_DIR_LOW_ADDR(2), CAB_REGION);
	xport_mips_write(MIPS_CHL_DIR_UP_ADDR(2), CAB_REGION + CAB_SIZE);

	xport_mips_write(MIPS_CHL_DIR_LOW_ADDR(3), AUD_PTS_REGION);
	xport_mips_write(MIPS_CHL_DIR_UP_ADDR(3), AUD_PTS_REGION + AUD_PTS_SIZE);

	return 0;
}

static int xport_hw_init(void)
{
	xport_writel(XPORT_CHL0_BASE_ADDR, (XPORT_CHL0_BASE_ADDR_DEF >> 3));
	xport_writel(XPORT_CHL0_CFG_ADDR, XPORT_CHL0_CFG_DEF);
	xport_writel(XPORT_CHL0_CFG_ADDR, XPORT_CHL0_CFG_DEF);

	xport_writel(XPORT_CHL1_BASE_ADDR, (XPORT_CHL1_BASE_ADDR_DEF >> 3));
	xport_writel(XPORT_CHL1_CFG_ADDR, XPORT_CHL1_CFG_DEF);
	xport_writel(XPORT_CHL1_CFG_ADDR, XPORT_CHL1_CFG_DEF);

	xport_writel(XPORT_CFG_ADDR1,
		     XPORT_CFG1_OUT_CHL0_LINE_SYNC |
		     XPORT_CFG1_OUT_CHL1_LINE_SYNC | XPORT_CFG1_OUT_CHL2_LINE_SYNC | XPORT_CFG1_OUT_CHL3_LINE_SYNC);

	xport_writel(CLK0_PWM_CTRL_ADDR, 0x2);      // for high

	return 0;
}

irqreturn_t xport_irq0(int irq, void *dev_id, struct pt_regs * egs)
{
	unsigned int irq_reg0;
	unsigned int idx;

	irq_reg0 = xport_readl(XPORT_INT_REG_ADDR0);
	idx = xport_readl(XPORT_EXT_INT_ADDR);

	xport_writel(XPORT_INT_CLS_ADDR0, irq_reg0);
	xport_writel(XPORT_EXT_INT_ADDR, xport_readl(XPORT_EXT_INT_ADDR) ^ idx);

	if (irq_reg0 & (XPORT_IRQ0_DMA0_MSK | XPORT_IRQ0_DMA0_EMPTY_MSK)) {
	     	//printk("entering xport_irq0 stage 2 0x%08x\n", irq_reg0);
		wake_up_interruptible(&irq0_dma0_wait_queue);
	}

	if (irq_reg0 & (XPORT_IRQ0_DMA1_MSK | XPORT_IRQ0_DMA1_EMPTY_MSK)) {
		wake_up_interruptible(&irq0_dma1_wait_queue);
	}

	if (irq_reg0 & XPORT_IRQ0_CRC_NOTIFY) {
		crc_err_idx = (irq_reg0 & XPORT_IRQ0_CRC_INDEX) >> XPORT_IRQ0_CRC_IDX_SHIFT;
		wake_up_interruptible(&crc_wait_queue);
	}

	if (irq_reg0 & XPORT_IRQ0_FILTER_NOTIFY) {
		unsigned int i;
		for (i = 32; i < MAX_FILTER_NUM; i++){
			if (idx & 0x1)
			{
				wake_up_interruptible(&filter_wait_queue[i]);
			}
			idx >>= 1;
		}
	}

     	//printk("entering xport_irq0 stage 3 0x%08x\n", irq_reg0);

	wake_up_interruptible(&irq0_wait_queue);

	return IRQ_HANDLED;
}

irqreturn_t xport_irq1(int irq, void *dev_id, struct pt_regs * egs)
{
	unsigned int i;
	unsigned int irq_reg1;

	irq_reg1 = xport_readl(XPORT_INT_REG_ADDR1);
	xport_writel(XPORT_INT_CLS_ADDR1, irq_reg1);

	for (i = 0; i < 32; i++) {
		if (irq_reg1 & 1) {
			wake_up_interruptible(&filter_wait_queue[i]);
		}
		irq_reg1 >>= 1;
	}



	wake_up_interruptible(&irq1_wait_queue);

	return IRQ_HANDLED;
}

static int xport_open(struct inode *inode, struct file *file)
{
	unsigned int filter_index;
	XPORT_DEV *xport_dev = NULL;

	xport_dev = kmalloc(sizeof(XPORT_DEV), GFP_KERNEL);
	if (xport_dev == NULL)
		return -EBUSY;

	xport_dev->dev_minor = iminor(inode);
	//init_waitqueue_head(&(xport_dev->wait_queue));

	if (xport_dev->dev_minor == XPORT_MINOR_VID0) {
		xport_dev->irq_wait_queue_ptr = &irq0_dma0_wait_queue;

		xport_dma_reset(0);

		/* xunli: init code */
		//xport_writel(DMA_INPUT0_HEAD_ADDR, 0);	/* clear data in DMA linker */
		xport_writel(XPORT_CHL0_RP_ADDR, /*chl0_wp | */0x40000000);
		xport_writel(XPORT_CHL_DMA0_WP_ADDR, /*chl0_wp*/0);

		memset(xport_mem_base + XPORT_CHL0_BASE_ADDR_DEF - XPORT_MEM_BASE, 0, XPORT_CHL0_UNIT_SIZE_DEF * XPORT_CHL0_UNIT_NUM_DEF);
	}
	else if (xport_dev->dev_minor == XPORT_MINOR_VID1) {
		xport_dev->irq_wait_queue_ptr = &irq0_dma1_wait_queue;

		xport_dma_reset(1);

		/* xunli: init code */
		//xport_writel(DMA_INPUT1_HEAD_ADDR, 0);	/* clear data in DMA linker */
		xport_writel(XPORT_CHL1_RP_ADDR, /*chl1_wp | */0x40000000);	/* xunli: clear CHL_FIFO */
		xport_writel(XPORT_CHL_DMA1_WP_ADDR, /*chl1_wp*/0);	/* xunli: clear direct DMA CHL_FIFO */
		memset(xport_mem_base + XPORT_CHL1_BASE_ADDR_DEF - XPORT_MEM_BASE, 0, XPORT_CHL1_UNIT_SIZE_DEF * XPORT_CHL1_UNIT_NUM_DEF);
	}
	else if ((xport_dev->dev_minor >= XPORT_MINOR_FT_BASE)
		 && (xport_dev->dev_minor < (XPORT_MINOR_FT_BASE + MAX_FILTER_NUM))) {
		filter_index = xport_dev->dev_minor - XPORT_MINOR_FT_BASE;

		xport_dev->filter_type = FILTER_TYPE_NUKOWN;

		xport_dev->irq_wait_queue_ptr = &filter_wait_queue[filter_index];

		xport_filter_reset(filter_index);
	}
	else
		xport_dev->irq_wait_queue_ptr = NULL;

	spin_lock_init(&(xport_dev->spin_lock));

	file->private_data = (void *) xport_dev;

	return 0;
}

static int xport_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static ssize_t xport_read(struct file *file, char __user * buffer, size_t len, loff_t * offset)
{
	unsigned int addr;
	unsigned int filter_index;
	ssize_t ret = -EFAULT;
        
	XPORT_DEV *xport_dev = file->private_data;

	if (likely((xport_dev->dev_minor >= XPORT_MINOR_FT_BASE)
                    && (xport_dev->dev_minor < (XPORT_MINOR_FT_BASE + MAX_FILTER_NUM)))) {

        if (unlikely(xport_dev->filter_type == FILTER_TYPE_NUKOWN)) {
            return -EFAULT;
        }
		
		filter_index = xport_dev->dev_minor - XPORT_MINOR_FT_BASE;

        spin_lock(&xport_dev->spin_lock);
        
        if (xport_dev->filter_type == FILTER_TYPE_SECTION) {
            ret = xport_filter_check_section_number(filter_index);
            if (ret > 0) {
                ret = xport_filter_read_section_data(filter_index, buffer, len);
            }
        }
        else if (xport_dev->filter_type == FILTER_TYPE_PES) {
            ret = xport_filter_check_pes_size(filter_index);
            if (ret > 0) {
                ret = xport_filter_read_pes_data(filter_index, buffer, len);
            }
        }
        else {
            ret = xport_filter_check_data_size(filter_index);
            if (ret >= len) {
                ret = xport_filter_read_data(filter_index, buffer, len);
                
            } else{
                ret = 0;
            }
        }
        spin_unlock(&xport_dev->spin_lock);
        
        return ret;
	}
    else if (unlikely(xport_dev->dev_minor == XPORT_MINOR)) { /* It should be deleted branch*/
		void __iomem *read_addr;

        spin_lock(&(xport_dev->spin_lock));
		copy_from_user(&addr, buffer, 4);	/* FIXME@debug used interface, can dump all date of xport buffer. It can be deleted */
        spin_unlock(&(xport_dev->spin_lock));
		read_addr = xport_mem_base + (addr - XPORT_MEM_BASE);
		if (copy_to_user(buffer, read_addr, len))
			return -EFAULT;

		return len;
	}

	return -EFAULT;
}

typedef ssize_t(*WRITE_FN) (const char __user * buffer, size_t len, unsigned int dma_id);

static ssize_t xport_write(struct file *file, const char __user * buffer, size_t len, loff_t * offset)
{
	WRITE_FN fn;
	unsigned int dma_id;
	unsigned int chl_type;

	ssize_t ret = -EFAULT;
	XPORT_DEV *xport_dev = file->private_data;

	DECLARE_WAITQUEUE(wait, current);

	if (xport_dev->dev_minor == XPORT_MINOR_VID0)
		dma_id = 0;
	else if (xport_dev->dev_minor == XPORT_MINOR_VID1)
		dma_id = 1;
	else
		return -EFAULT;

	if (len < 188 || ((len % 188) != 0))
		return -EFAULT;

	/*xunli: for direct dma mode */
	chl_type = xport_readl(XPORT_CHL0_CFG_ADDR + 24 * dma_id);
	chl_type = (chl_type >> 29) & 0x3;
	if (chl_type == 3)
		fn = xport_dma_direct_write;
	else
		fn = xport_dma_write;

	//printk("entering xport_write stage 1\n");
	add_wait_queue(xport_dev->irq_wait_queue_ptr, &wait);

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);

		ret = xport_dma_half_empty_check(dma_id);

		if (ret == -EFAULT)
			ret = 0;
		else
			ret = fn(buffer, len, dma_id);

		if (ret == -EFAULT || ret == len)
			break;

		else if (file->f_flags & O_NONBLOCK)
			break;

		else if (ret > 0 && ret < len) {
			buffer += ret;
			len -= ret;
		}

		if (signal_pending(current)) {
			ret = -EFAULT;
			break;
		}

		//printk("entering xport_write stage 2\n");
		schedule();
		//printk("entering xport_write stage 3\n");
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(xport_dev->irq_wait_queue_ptr, &wait);

	//printk("entering xport_write stage 4\n");

	return ret;
}

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
static inline int uncached_access(struct file *file, unsigned long addr)
{
	/*
	 * Accessing memory above the top the kernel knows about or through a file pointer
	 * that was marked O_SYNC will be done non-cached.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	return addr >= __pa(high_memory);
}

static int xport_mmap(struct file *filp, struct vm_area_struct *vma)
{
#if defined(__HAVE_PHYS_MEM_ACCESS_PROT)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	vma->vm_page_prot = phys_mem_access_prot(file, offset,
						 vma->vm_end - vma->vm_start,
						 vma->vm_page_prot);
#elif defined(pgprot_noncached)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int uncached;

	uncached = uncached_access(filp, offset);
	if (uncached)
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end-vma->vm_start,
			    vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static unsigned int xport_poll(struct file *file, poll_table * wait)
{
	unsigned int filter_index;
	XPORT_DEV *xport_dev = file->private_data;

	poll_wait(file, xport_dev->irq_wait_queue_ptr, wait);
	poll_wait(file, &crc_wait_queue, wait);

	//printk(KERN_INFO"wait_queue %d reponse!\n", xport_dev->dev_minor);

	if (xport_dev->dev_minor == XPORT_MINOR_VID0) {
		if (xport_dma_input_check(0))
			return POLLOUT | POLLWRNORM;
	}

	else if (xport_dev->dev_minor == XPORT_MINOR_VID1) {
		if (xport_dma_input_check(1))
			return POLLOUT | POLLWRNORM;
	}

	else if ((xport_dev->dev_minor >= XPORT_MINOR_FT_BASE)
		 && (xport_dev->dev_minor < (XPORT_MINOR_FT_BASE + MAX_FILTER_NUM))) {
		filter_index = xport_dev->dev_minor - XPORT_MINOR_FT_BASE;

		if (xport_dev->filter_type == FILTER_TYPE_NUKOWN)
			return 0;

		else if (xport_dev->filter_type == FILTER_TYPE_SECTION) {
			if (0x0000ffff != crc_err_idx) 
			{
				last_crc_error_idx = crc_err_idx;
				crc_err_idx = 0x0000ffff;

				return POLLPRI;
			}

			if (xport_filter_check_section_number(filter_index) > 0)
				return POLLIN | POLLRDNORM;
		}

		else if (xport_dev->filter_type == FILTER_TYPE_PES) {
			if (xport_filter_check_pes_size(filter_index) > 0)
				return POLLIN | POLLRDNORM;
		}

		else {
			if (xport_filter_check_data_size(filter_index) > 0)
				return POLLIN | POLLRDNORM;
		}
	}

	return 0;
}

typedef struct __ioctl_params__
{
        unsigned int pid_idx;
        unsigned int pid_en;
        unsigned int pid_val;
        unsigned int pid_chl;
        unsigned int des_idx;
        unsigned int des_en;
        unsigned int des_len;
        unsigned int des_key[6];

        unsigned int avout_idx;
        unsigned int avout_en;
        unsigned int avout_pid;
        unsigned int avout_mode;
	unsigned int avout_chl_switch;

        unsigned int filter_idx;
        unsigned int filter_en;
        unsigned int filter_crc_idx;
        unsigned int filter_crc_en;
        unsigned int filter_crc_save;
        unsigned int filter_crc_notify_en;

        unsigned int pcr_idx;
        unsigned int pcr_en;
        unsigned int pcr_pid;
        unsigned int pcr_hi_val;
        unsigned int pcr_lo_val;

} xport_ioctl_params;

static int xport_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int regs_val = 0;
	unsigned int tmp_val = 0;
	unsigned int mask_val = 0;
	unsigned int filter_index = 0;
	unsigned int filter_data =0;
	unsigned char filter_mask[24];
	
	xport_ioctl_params ioc_params;

	XPORT_DEV *xport_dev = file->private_data;
	


	if ((xport_dev->dev_minor >= XPORT_MINOR_FT_BASE)
	    && (xport_dev->dev_minor < (XPORT_MINOR_FT_BASE + MAX_FILTER_NUM))) 
	{
		filter_index = xport_dev->dev_minor - XPORT_MINOR_FT_BASE;

		switch (cmd) {
		case XPORT_FILTER_IOC_PID0:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 0);
			break;

		case XPORT_FILTER_IOC_PID1:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 1);
			break;

		case XPORT_FILTER_IOC_PID2:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 2);
			break;

		case XPORT_FILTER_IOC_PID3:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 3);
			break;

		case XPORT_FILTER_IOC_PID4:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 4);
			break;

		case XPORT_FILTER_IOC_PID5:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 5);
			break;

		case XPORT_FILTER_IOC_PID6:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 6);
			break;

		case XPORT_FILTER_IOC_PID7:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 7);
			break;

		case XPORT_FILTER_IOC_PID8:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 8);
			break;

		case XPORT_FILTER_IOC_PID9:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 9);
			break;

		case XPORT_FILTER_IOC_PID10:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 10);
			break;

		case XPORT_FILTER_IOC_PID11:
			__get_user(regs_val, (int __user *) arg);
			xport_filter_set_pidx(filter_index, regs_val, 11);
			break;

		case XPORT_FILTER_IOC_FILTER:
			copy_from_user(filter_mask, (int __user *) arg, 24);
			xport_filter_set_filter(filter_index, filter_mask, filter_mask + 12);
			break;

		case XPORT_FILTER_IOC_FILTER_COND:
			copy_from_user(filter_mask, (int __user *)arg, 12);
			xport_filter_set_filter_cond(filter_index, filter_mask);
			break;

		case XPORT_FILTER_IOC_TYPE:
			__get_user(regs_val, (int __user *) arg);
			if (regs_val == FILTER_TYPE_SECTION || regs_val == FILTER_TYPE_TS || regs_val == FILTER_TYPE_PES
			    || regs_val == FILTER_TYPE_ES) 
			{
				if (xport_filter_set_type(filter_index, regs_val) == 0)
					xport_dev->filter_type = regs_val;
			}
			break;

		case XPORT_FILTER_IOC_ENABLE:
			__get_user(regs_val, (int __user *) arg);
			if (xport_dev->filter_type == FILTER_TYPE_SECTION
			    || xport_dev->filter_type == FILTER_TYPE_TS
			    || xport_dev->filter_type == FILTER_TYPE_PES || xport_dev->filter_type == FILTER_TYPE_ES) {
				if (regs_val)
					xport_filter_enable(filter_index, &(xport_dev->spin_lock));
				else
					xport_filter_disable(filter_index, &(xport_dev->spin_lock));

				xport_filter_clear_buffer(filter_index);
			}
			break;

		case XPORT_FILTER_IOC_QUERY_NUM:
			if (xport_dev->filter_type == FILTER_TYPE_SECTION) {
				regs_val = xport_filter_check_section_number(filter_index);
			}
			else if (xport_dev->filter_type == FILTER_TYPE_PES) {
				regs_val = xport_filter_check_pes_size(filter_index);
			}
			else if (xport_dev->filter_type == FILTER_TYPE_TS || xport_dev->filter_type == FILTER_TYPE_ES) {
				regs_val = xport_filter_check_data_size(filter_index);
			}
			else
				regs_val = 0;

			__put_user(regs_val, (int __user *) arg);

			break;

		case XPORT_FILTER_IOC_QUERY_SIZE:
			if (xport_dev->filter_type == FILTER_TYPE_SECTION)
				regs_val = xport_filter_check_section_size(filter_index);
			else
				regs_val = 0;

			__put_user(regs_val, (int __user *) arg);

			break;

		case XPORT_FILTER_IOC_CRC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			tmp_val  = ioc_params.filter_crc_en << 7;
			tmp_val |= ioc_params.filter_crc_save << 6;
			tmp_val |= ioc_params.filter_crc_idx;
			xport_mips_write(MIPS_CHL_CRC_EN(filter_index + 4), tmp_val); /* the section filter index starts from 4, so +4 */

			break;

		case XPORT_FILTER_IOC_CRC_NOTIFY_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			tmp_val  = ioc_params.filter_crc_notify_en << 31;
			xport_mips_write(MIPS_CHL_CRC_NOTIFY_EN(filter_index + 4), tmp_val); /* the same reason, see the above */

			break;

		case XPORT_FILTER_IOC_SAVE_ENABLE:
            copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
                        
			xport_mips_read(MIPS_CHL_CRC_EN(filter_index + 4), &tmp_val);
            tmp_val |= ioc_params.filter_crc_save << 6;
            xport_mips_write(MIPS_CHL_CRC_EN(filter_index + 4), tmp_val); /* the same reason, see the above */

			break;

		case XPORT_FILTER_IOC_SWITCH:
			xport_mips_write(MIPS_CHL_SWITCH(filter_index + 4), (int)arg);
			break;

		case XPORT_FILTER_IOC_BUFFER_UPADDR:
			if ((void *)arg != NULL){
			  filter_data=FILTER_BUF_UP(filter_index);
			  copy_to_user((void __user *)arg, &filter_data, sizeof(unsigned int));
			}
            else 
				return -EINVAL;
			break;

		case XPORT_FILTER_IOC_BUFFER_LOWADDR:
			if ((void *)arg != NULL){
			  filter_data=FILTER_BUF_LOW(filter_index);
			  copy_to_user((void __user *)arg, &filter_data, sizeof(unsigned int));
			}
            else 
				return -EINVAL;
			break;

		case XPORT_FILTER_IOC_BUFFER_RP_ADDR:
			if ((void *)arg != NULL){
			  filter_data=xport_filter_rp(filter_index);
			  copy_to_user((void __user *)arg, &filter_data, sizeof(unsigned int));
			}
            else 
				return -EINVAL;
			break;
		case XPORT_FILTER_IOC_BUFFER_WP_ADDR:
			if ((void *)arg != NULL){
			  filter_data=xport_filter_wp(filter_index);
			  copy_to_user((void __user *)arg, &filter_data, sizeof(unsigned int));
			}
            else 
				return -EINVAL;
			break;
			
		}

		return 0;
	}

	else if (__IS_HW_ADDR__(cmd)) 
	{
		if (__WR_FLAGS__(cmd)) 
		{
			regs_val = arg;
			xport_writel(cmd, regs_val);
		}
		else {
			regs_val = xport_readl(cmd);
			__put_user(regs_val, (int __user *) arg);
		}

		return 0;
	}
	else if (__IS_MIPS_ADDR__(cmd)) 
	{
		if (__WR_FLAGS__(cmd)) 
		{
			regs_val = arg;
			if (xport_mips_write(__OFFSET_ADDR__(cmd), regs_val))
				return -EINVAL;
		}
		else {
			if (xport_mips_read(__OFFSET_ADDR__(cmd), &regs_val))
				return -EINVAL;
			__put_user(regs_val, (int __user *) arg);
		}

		return 0;
	}

	else {
		int i;

		switch (cmd) {
		case XPORT_PIDFT_IOC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			tmp_val = xport_readl(__PID_FILTER__(ioc_params.pid_idx));
			if (ioc_params.pid_en)
				tmp_val |= 0x80000000;
			else
				tmp_val &= ~0x80000000;
			xport_writel(__PID_FILTER__(ioc_params.pid_idx), tmp_val);

			break;

		case XPORT_PIDFT_IOC_CHANNEL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			tmp_val = xport_readl(__PID_FILTER__(ioc_params.pid_idx));
			if (ioc_params.pid_chl)
				tmp_val |= 0x40000000;
			else
				tmp_val &= ~0x40000000;
			xport_writel(__PID_FILTER__(ioc_params.pid_idx), tmp_val);

			break;

		case XPORT_PIDFT_IOC_PIDVAL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			tmp_val = xport_readl(__PID_FILTER__(ioc_params.pid_idx));
			tmp_val &= ~0x20001fff;//lixun changed, no descramble
			tmp_val |= ioc_params.pid_val;
			xport_writel(__PID_FILTER__(ioc_params.pid_idx), tmp_val);

			break;

		case XPORT_PIDFT_IOC_DESC_ODDKEY:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			for (i = 0; i < ioc_params.des_len; i++)
				xport_writel(__DESC_ODD_ADDR__(ioc_params.des_idx, i),
					     ioc_params.des_key[i]);

			break;

		case XPORT_PIDFT_IOC_DESC_EVENKEY:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			for (i = 0; i < ioc_params.des_len; i++)
				xport_writel(__DESC_EVEN_ADDR__(ioc_params.des_idx, i),
					     ioc_params.des_key[i]);

			break;

		case XPORT_PIDFT_IOC_DESC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			/* write bit26~29 of PID_FILTERx. */
			/* bit29: enable / disable DES */
			/* bit26~28: DES id */
			// regs_val = i;
			// regs_val |= 0x80000000; /* bit31 indicates enabling flags, 1 - enable, 0 = disable. */
			// regs_val |= (pidft_ptr->pid_filter_id << 30);

			tmp_val = xport_readl(__PID_FILTER__(ioc_params.pid_idx));
			tmp_val &= ~0x3c000000;

			if (ioc_params.des_en) {
				tmp_val |= 0x20000000;
				tmp_val |= (ioc_params.des_idx << 26);
			}

			xport_writel(__PID_FILTER__(ioc_params.pid_idx), tmp_val);

			break;

		case XPORT_CHL_IOC_CLEAR:
			if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
			{
				xport_writel(XPORT_CHL0_RP_ADDR, /*chl0_wp | */0x40000000);
				xport_writel(XPORT_CHL_DMA0_WP_ADDR, /*chl0_wp*/0);
				memset(xport_mem_base + XPORT_CHL0_BASE_ADDR_DEF - XPORT_MEM_BASE, 
					0, 
					XPORT_CHL0_UNIT_SIZE_DEF * XPORT_CHL0_UNIT_NUM_DEF);
			}
			else 
			{
				xport_writel(XPORT_CHL1_RP_ADDR, /*chl1_wp | */0x40000000);	/* xunli: clear CHL_FIFO */
				xport_writel(XPORT_CHL_DMA1_WP_ADDR, /*chl1_wp*/0);	/* xunli: clear direct DMA CHL_FIFO */
				memset(xport_mem_base + XPORT_CHL1_BASE_ADDR_DEF - XPORT_MEM_BASE, 
					0, 
					XPORT_CHL1_UNIT_SIZE_DEF * XPORT_CHL1_UNIT_NUM_DEF);
			}

			break;

		case XPORT_CHL_IOC_ENABLE:
			regs_val = (unsigned int) arg;

			if (xport_dev->dev_minor == XPORT_MINOR_VID0) tmp_val = xport_readl(XPORT_CHL0_CFG_ADDR);
			else tmp_val = xport_readl(XPORT_CHL1_CFG_ADDR);

			tmp_val &= ~0x80000000;
			if (regs_val)
				tmp_val |= 0x80000000;

			if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
			{
				xport_writel(XPORT_CHL0_CFG_ADDR, tmp_val);
			}
			else 
			{
				xport_writel(XPORT_CHL1_CFG_ADDR, tmp_val);
			}

			tmp_val = xport_readl(XPORT_TUNER_EN);
			if(regs_val){
				if (xport_dev->dev_minor == XPORT_MINOR_VID0)
					tmp_val |= 0x00000001;
				else
					tmp_val |= 0x00000002;
			}
			else{
				if (xport_dev->dev_minor == XPORT_MINOR_VID0)
					tmp_val &= ~0x00000001;
				else
					tmp_val &= ~0x00000002;
	
			}
			xport_writel(XPORT_TUNER_EN, tmp_val);

			break;

		case XPORT_CHL_IOC_DES_MODE:
			regs_val = (unsigned int) arg;
			
			tmp_val = xport_readl(XPORT_CFG_ADDR0);
			switch (regs_val) {
				case 0: // DVB DES
					if (xport_dev->dev_minor == XPORT_MINOR_VID0)
						tmp_val &= ~0x10;
					else tmp_val &= ~0x20;
					
					break;
					
				case 1: // ATSC DES
					if (xport_dev->dev_minor == XPORT_MINOR_VID0)
						tmp_val |= 0x10;
					else tmp_val |= 0x20;
					
					break;
			}
			xport_writel(XPORT_CFG_ADDR0, tmp_val);

			break;
			
		case XPORT_CHL_IOC_TUNER_MODE:
			regs_val = (unsigned int) arg;
			
			tmp_val = xport_readl(XPORT_CFG_ADDR0);
			switch (regs_val) {
				case 0: // parallel mode
					if (xport_dev->dev_minor == XPORT_MINOR_VID0)
						tmp_val &= ~0x01;
					else tmp_val &= ~0x02;
					
					break;
					
				case 1: // serial mode
					if (xport_dev->dev_minor == XPORT_MINOR_VID0)
						tmp_val |= 0x01;
					else tmp_val |= 0x02;
					
					break;
			}
			xport_writel(XPORT_CFG_ADDR0, tmp_val);

			break;
			
		case XPORT_CHL_IOC_INPUT_MODE:
			regs_val = (unsigned int) arg;

			tmp_val = xport_readl(XPORT_CFG_ADDR0);

			if (xport_dev->dev_minor == XPORT_MINOR_VID0) mask_val = 0x00000004;
			else mask_val = 0x00000020;

			if (regs_val != 0) /* DMA_MODE or DIRECT_MODE */
				tmp_val |= mask_val;
			else 
				tmp_val &= (~mask_val);

			xport_writel(XPORT_CFG_ADDR0, tmp_val);

			if (xport_dev->dev_minor == XPORT_MINOR_VID0) tmp_val = xport_readl(XPORT_CHL0_CFG_ADDR);
			else tmp_val = xport_readl(XPORT_CHL1_CFG_ADDR);

			tmp_val &= ~0x60000000;

			switch (regs_val) {
			case 0:	/* tuner */
				break;

			case 1:	/* DMA */
				tmp_val |= 0x40000000;
				break;

			case 2:	/* direct */
				tmp_val |= 0x60000000;
				break;
			}

			if (xport_dev->dev_minor == XPORT_MINOR_VID0) xport_writel(XPORT_CHL0_CFG_ADDR, tmp_val);
			else xport_writel(XPORT_CHL1_CFG_ADDR, tmp_val);

			/*
			 * enable interrupt, according the following flow:
			 * INT0_ENB_ADDR | 0x41, for DEMUX_INPUT_MOD_TUNER.
			 * INT0_ENB_ADDR | 0x41, for DMA_MOD / CHL0.
			 * INT0_ENB_ADDR | 0x82, for DMA_MOD / CHL1.
			 */

			tmp_val = xport_readl(XPORT_INT_ENB_ADDR0);


			switch (regs_val) {
			case 0:	/* tuner */
				if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
					tmp_val &= ~0x41;	/* WARNING: this parameters are undocumented, pls don't change it. */
				else 
					tmp_val &= ~0x82;	/* WARNING: this parameters are undocumented, pls don't change it. */
				
				break;

			case 1:	/* DMA */
			case 2:	/* direct */
				if (XPORT_MINOR_VID0 == xport_dev->dev_minor)
					tmp_val |= 0x41;
				else
					tmp_val |= 0x82;

				break;
			}

			xport_writel(XPORT_INT_ENB_ADDR0, tmp_val);

			break;

		case XPORT_CHL_IOC_RESET:
			break;

		case XPORT_CHL_IOC_DMA_RESET:
			xport_dma_reset(xport_dev->dev_minor - XPORT_MINOR_VID0);

			break;

		case XPORT_VID_IOC_OUTPUT_MODE:
			break;

		case XPORT_VID_IOC_RESET:
			break;

		case XPORT_VID_IOC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			if (ioc_params.avout_en)
			{
			    tmp_val = 0x80000000; /* enable flags */
			    tmp_val |= ioc_params.avout_mode << 7; /* output mode, bit7 */
			    tmp_val |= 0x00000002; /* output type, ES */

			    xport_mips_write(MIPS_CHL_EN(ioc_params.avout_idx), tmp_val);
			}
			else
			    xport_mips_write(MIPS_CHL_DISABLE(ioc_params.avout_idx), 0);

			break;

		case XPORT_VID_IOC_PIDVAL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
			xport_mips_write(MIPS_CHL_PID0(ioc_params.avout_idx), ioc_params.avout_pid);

			break;

		case XPORT_AUD_IOC_OUTPUT_MODE:
			break;

		case XPORT_AUD_IOC_RESET:
			break;

		case XPORT_AUD_IOC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
			if (ioc_params.avout_en)
			{
			    tmp_val  = 0x80000000; /* enable flags */
			    tmp_val |= ioc_params.avout_mode << 7; /* output mode, bit7 */
			    tmp_val |= 0x00000002; /* output type, ES */

			    xport_mips_write(MIPS_CHL_EN(ioc_params.avout_idx), tmp_val);
			}
			else
			    xport_mips_write(MIPS_CHL_DISABLE(ioc_params.avout_idx), 0);

			break;

		case XPORT_AUD_IOC_PIDVAL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
			xport_mips_write(MIPS_CHL_PID0(ioc_params.avout_idx), ioc_params.avout_pid);

			break;

		case XPORT_PCR_IOC_ENABLE:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			xport_mips_read(MIPS_PCR_PID(ioc_params.pcr_idx), &tmp_val);
			tmp_val &= ~0x80000000;
			tmp_val |= (ioc_params.pcr_en << 31);
			xport_mips_write(MIPS_PCR_PID(ioc_params.pcr_idx), tmp_val);

			break;

		case XPORT_PCR_IOC_GETVAL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			xport_mips_read_ex(MIPS_PCR_GET(ioc_params.pcr_idx), &ioc_params.pcr_hi_val, &ioc_params.pcr_lo_val);
			copy_to_user((void*)arg, &ioc_params, sizeof(xport_ioctl_params));

			break;

		case XPORT_PCR_IOC_PIDVAL:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));

			xport_mips_read(MIPS_PCR_PID(ioc_params.pcr_idx), &tmp_val);
			tmp_val &= ~0x1fff;
			tmp_val |= ioc_params.pcr_pid;
			xport_mips_write(MIPS_PCR_PID(ioc_params.pcr_idx), tmp_val);

			break;

		case XPORT_FW_INIT:
			xport_load_firmware();
			xport_config_buf();

			break;

		case XPORT_VID_IOC_SWITCH: 
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
			xport_mips_write(MIPS_CHL_SWITCH(ioc_params.avout_idx ? 1 : 0), ioc_params.avout_chl_switch);
			break;
			
		case XPORT_AUD_IOC_SWITCH:
			copy_from_user(&ioc_params, (void*)arg, sizeof(xport_ioctl_params));
			xport_mips_write(MIPS_CHL_SWITCH(2/*only one audio output*/), ioc_params.avout_chl_switch);
			break;

		case XPORT_IOC_MEM_BASE_PHYADDR:
            if (NULL != (void *)arg){
			  tmp_val = XPORT_MEM_BASE;
			  copy_to_user((void __user *)arg, &tmp_val, sizeof(unsigned int));
            }
			else
				return -EINVAL;
			break;

		case XPORT_IOC_MEM_SIZE:
            if (NULL != (void *)arg){
				tmp_val = XPORT_MEM_SIZE;
				copy_to_user((void __user *)arg, &tmp_val, sizeof(unsigned int));
            }
			else
				return -EINVAL;

			break;
        case XPORT_CHL_IOC_UNIT_NUM_DEF:
            if (NULL != (void *)arg){
             
                if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
                    tmp_val = XPORT_CHL0_UNIT_NUM_DEF;
                else if (xport_dev->dev_minor == XPORT_MINOR_VID1)
                    tmp_val = XPORT_CHL1_UNIT_NUM_DEF;
                else 
                    return -EINVAL;
				copy_to_user((void __user *)arg, &tmp_val, sizeof(unsigned int));
            }
			else
				return -EINVAL;
            
			break;
        case XPORT_CHL_IOC_UNIT_SIZE_DEF:

            if (NULL != (void *)arg){

                if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
                    tmp_val = XPORT_CHL0_UNIT_SIZE_DEF;
                else if (xport_dev->dev_minor == XPORT_MINOR_VID1)
                    tmp_val = XPORT_CHL1_UNIT_SIZE_DEF;
                else
                    return -EINVAL;
				copy_to_user((void __user *)arg, &tmp_val, sizeof(unsigned int));
            }
			else
				return -EINVAL;
			break;

        case XPORT_CHL_IOC_MIN_SPACES:
            if (NULL != (void *)arg){
                if (xport_dev->dev_minor == XPORT_MINOR_VID0) 
                    tmp_val = XPORT_CHL0_MIN_SPACES;
                else if (xport_dev->dev_minor == XPORT_MINOR_VID1)
                    tmp_val = XPORT_CHL1_MIN_SPACES;
                else 
                    return -EINVAL;			  
				copy_to_user((void __user *)arg, &tmp_val, sizeof(unsigned int));
            }
			else
				return -EINVAL;
			break;
          

		}

		return 0;
	}

	return -EINVAL;
}

static struct file_operations xport_fops = {
	.owner = THIS_MODULE,
	.open = xport_open,
	.release = xport_release,
	.read = xport_read,
	.write = xport_write,
	.poll = xport_poll,
	.ioctl = xport_ioctl,
	.mmap = xport_mmap,
};

// static struct miscdevice orion_xport_miscdev = {
// 	MISC_DYNAMIC_MINOR,
// 	"orion_xport",
// 	&xport_fops
// };

static struct proc_dir_entry *xport_proc_entry = NULL;

static void xport_dump_regs(void)
{
	unsigned int chl_id;
	unsigned int reg_val;

	printk("==============> Hardware Register <=====================\n");

	reg_val = xport_readl(0x41400000 + (0x0008 << 2));
	printk("xport_cfg0         = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0009 << 2));
	printk("xport_cfg1         = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0018 << 2));
	printk("tuner enable       = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0011 << 2));
	printk("IRQ0 En            = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0015 << 2));
	printk("IRQ1 En            = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0010 << 2));
	printk("IRQ0 ST            = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0014 << 2));
	printk("IRQ1 ST            = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0040 << 2));
	printk("channel0 base addr = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0041 << 2));
	printk("channel0 cfg       = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0042 << 2));
	printk("channel0 rp        = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0043 << 2));
	printk("channel0 wp        = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0045 << 2));
	printk("channel0 st        = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0060 << 2));
	printk("pid filter0        = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0061 << 2));
	printk("pid filter1        = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0003 << 2));
	printk("Mail Box3          = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0004 << 2));
	printk("Mail Box4          = 0x%08x\n", reg_val);
	reg_val = xport_readl(0x41400000 + (0x0005 << 2));
	printk("Mail Box5          = 0x%08x\n", reg_val);

	printk("\n==============>   Mips Register   <=====================\n");
	for (chl_id = 0; chl_id < 6; chl_id++) {
		printk("==============> Output Channel %d <=====================\n", chl_id);

		xport_mips_read(MIPS_CHL_EN(chl_id), &reg_val);
		printk("channel en           = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_PID0(chl_id), &reg_val);
		printk("channel PID0         = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_PID1(chl_id), &reg_val);
		printk("channel PID1         = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_PID2(chl_id), &reg_val);
		printk("channel PID2         = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_PID3(chl_id), &reg_val);
		printk("channel PID3         = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_DIR_LOW_ADDR(chl_id), &reg_val);
		printk("channel DIR_LOW_ADRR = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_DIR_UP_ADDR(chl_id), &reg_val);
		printk("channel DIR_UP_ADRR  = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_WP(chl_id), &reg_val);
		printk("channel WP           = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_BUF_LOW_ADDR(chl_id), &reg_val);
		printk("channel BUF_LOW_ADRR = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_BUF_UP_ADDR(chl_id), &reg_val);
		printk("channel BUF_UP_ADRR  = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_ERR_PKT_CNT(chl_id), &reg_val);
		printk("channel ERR Packet   = 0x%08x\n", reg_val);

		xport_mips_read(MIPS_CHL_OUT_PKT_CNT(chl_id), &reg_val);
		printk("channel Out Packet   = 0x%08x\n", reg_val);
	}

	return;
}

static int xport_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	u32 addr;
	u32 val;

	const char *cmd_line = buffer;;

	if (strncmp("rl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = xport_readl(addr);
		printk(" xport_readl [0x%08x] = 0x%08x \n", addr, val);
	}
	else if (strncmp("wl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[12], NULL, 16);
		xport_writel(addr, val);
	}
	else if (strncmp("mips_rl", cmd_line, 7) == 0) {
		addr = simple_strtol(&cmd_line[8], NULL, 16);
		xport_mips_read(addr, &val);
		printk(" xport_readl [0x%08x] = 0x%08x \n", addr, val);
	}
	else if (strncmp("mips_wl", cmd_line, 7) == 0) {
		addr = simple_strtol(&cmd_line[8], NULL, 16);
		val = simple_strtol(&cmd_line[13], NULL, 16);
		xport_mips_write(addr, val);
	}
	else if (strncmp("dumpall", cmd_line, 7) == 0)
		xport_dump_regs();
	else if (strncmp("load", cmd_line, 4) == 0) 
		xport_load_firmware();
	else if (strncmp("allocbufs", cmd_line, 9) == 0)
		xport_config_buf();
	else if (strncmp("clearpids", cmd_line, 9) == 0)
	{
		int i;
		for (i = 0; i < 64; i ++) 
			xport_writel(__PID_FILTER__(i), 0);
	}
	else if (strncmp("crcerr", cmd_line, 6) == 0)
		printk(" last_crc_error_idx = %d \n", last_crc_error_idx);
	else if (strncmp("getpcr", cmd_line, 6) == 0)
	{
		unsigned int pcr_hi, pcr_lo;
		xport_mips_write(MIPS_PCR_PID(0), 0x80000000);
		xport_mips_read_ex(MIPS_PCR_GET(0), &pcr_hi, &pcr_lo);
		printk(" pcr = %08x:%08x \n", pcr_hi, pcr_lo);
	}
	else {
		printk("\nonly following commands are supported: \n");
		printk(" rl \n");
		printk(" wl \n");
		printk(" mips_rl \n");
		printk(" mips_wl \n");
		printk(" dumpall \n");
	}

	return count;
}

static void xport_setup_dev(dev_t xport_dev, char * dev_name)
{

    //	devfs_mk_cdev(xport_dev, S_IFCHR | S_IRUGO | S_IWUSR, dev_name);
    class_simple_device_add(csm120x_xport_class, xport_dev, NULL, dev_name); 

}

static int __init xport_init(void)
{
	int i;
	char devname[100];

	if (NULL == request_mem_region(XPORT_REGS_BASE, XPORT_REGS_SIZE, "Orion Register space")) {
		goto outerr7;
	}

	xport_regs_base = ioremap(XPORT_REGS_BASE, XPORT_REGS_SIZE);
	if (NULL == xport_regs_base) {
		goto outerr6;
	}

	if (NULL == request_mem_region(XPORT_MEM_BASE, XPORT_MEM_SIZE, "Orion Memory space")) {
		goto outerr5;
	}

	xport_mem_base = ioremap(XPORT_MEM_BASE, XPORT_MEM_SIZE);
	if (NULL == xport_mem_base) {
		goto outerr4;
	}

	for (i = 0; i < MAX_FILTER_NUM; i++)
		init_waitqueue_head(&filter_wait_queue[i]);


	if (register_chrdev(XPORT_MAJOR, "orion_xport", &xport_fops)){
        goto outerr3;
    }

    csm120x_xport_class = class_simple_create(THIS_MODULE,"csm_xport");
    if (IS_ERR(csm120x_xport_class)){
        printk(KERN_ERR "XPORT: class create failed.\n");
        goto outerr2;
     // return -EIO;
    }

    //create a platform device aim to obtain a device structure
    xport_pdev = platform_device_register_simple("xport_device", 0, NULL, 0);
    if (IS_ERR(xport_pdev)) {
        printk(KERN_ERR "XPORT: Failed to register device for \"%s\"\n",
               "xport");
        goto outerr1;
    //  return -EINVAL;
    }

	xport_setup_dev(MKDEV(XPORT_MAJOR, XPORT_MINOR), "csm_xport");
	xport_setup_dev(MKDEV(XPORT_MAJOR, XPORT_MINOR_VID0), "video0");
	xport_setup_dev(MKDEV(XPORT_MAJOR, XPORT_MINOR_VID1), "video1");
	xport_setup_dev(MKDEV(XPORT_MAJOR, XPORT_MINOR_AUD0), "audio0");

	for (i = 0; i < MAX_FILTER_NUM; i++) {
		sprintf(devname, "xport_filter%d", i);
		xport_setup_dev(MKDEV(XPORT_MAJOR, XPORT_MINOR_FT_BASE + i), devname);
	}

	xport_dma_init();
	xport_hw_init();

	if (0 != request_irq(XPORT_IRQ0, xport_irq0, SA_INTERRUPT, "orion_xport0", &irq0_dev_id)) {
		printk(KERN_ERR "XPORT: cannot register IRQ0 \n");
        goto outerr0;
     //	return -EIO;
	}

	if (0 != request_irq(XPORT_IRQ1, xport_irq1, SA_INTERRUPT, "orion_xport1", &irq1_dev_id)) {
		printk(KERN_ERR "XPORT: cannot register IRQ1 \n");
        goto outerr0;
     //	return -EIO;
	}

	xport_proc_entry = create_proc_entry("xport_io", 0, NULL);
	if (NULL != xport_proc_entry) {
		xport_proc_entry->write_proc = &xport_proc_write;
	}

    // sema_init(&read_sem, 1);
	printk(KERN_INFO "Xport: Init OK [0x%08x]. \n", XPORT_MEM_BASE);
	return 0;

 outerr0:
    platform_device_unregister(xport_pdev);
 outerr1: 
    class_simple_destroy(csm120x_xport_class);
 outerr2:
    unregister_chrdev(XPORT_MAJOR, "orion_xport");
 outerr3:
    iounmap(xport_mem_base);    
 outerr4:
    release_mem_region(XPORT_MEM_BASE, XPORT_MEM_SIZE);
 outerr5:
    iounmap(xport_regs_base);
 outerr6:
    release_mem_region(XPORT_REGS_BASE, XPORT_REGS_SIZE);
 outerr7:
    return -ENODEV;
}

static void __exit xport_exit(void)
{
	int i;
	char devname[100];

	for (i = 0; i < 48; i++) {
		sprintf(devname, "orion_xport/filter%d", i);
		devfs_remove(devname);
	}

	if (NULL != xport_proc_entry)
		remove_proc_entry("xport_io", NULL);

// 	devfs_remove("orion_xport/video0");
// 	devfs_remove("orion_xport/video1");
// 	devfs_remove("orion_xport/audio0");
// 	devfs_remove("orion_xport/orion_xport");

    class_simple_device_remove(MKDEV(XPORT_MAJOR, XPORT_MINOR));
    class_simple_device_remove(MKDEV(XPORT_MAJOR, XPORT_MINOR_VID0));
    class_simple_device_remove(MKDEV(XPORT_MAJOR, XPORT_MINOR_VID1));
    class_simple_device_remove(MKDEV(XPORT_MAJOR, XPORT_MINOR_AUD0));

	for (i = 0; i < MAX_FILTER_NUM; i++) {
		class_simple_device_remove(MKDEV(XPORT_MAJOR, XPORT_MINOR_FT_BASE + i));
	}

    platform_device_unregister(xport_pdev);
    class_simple_destroy(csm120x_xport_class);
	iounmap(xport_regs_base);
	release_mem_region(XPORT_REGS_BASE, XPORT_REGS_SIZE);

	iounmap(xport_mem_base);
	release_mem_region(XPORT_MEM_BASE, XPORT_MEM_SIZE);

	free_irq(XPORT_IRQ0, &irq0_dev_id);
	free_irq(XPORT_IRQ1, &irq1_dev_id);

	unregister_chrdev(XPORT_MAJOR, "orion_xport");

	printk(KERN_INFO " ORION Xport Exit ...... OK. \n");

	return;
}

module_init(xport_init);
module_exit(xport_exit);

MODULE_AUTHOR("Yao.chen (yao.chen@celestialsemi.com)");
// MODULE_MAINTAINER("Zhongkai Du (zhongkai.du@celestialsemi.com)");
MODULE_DESCRIPTION("Orion1.4 Xport Driver");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");
