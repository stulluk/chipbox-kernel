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

#include "blit_ioctl.h"
#include "orion_blit.h"

#define  ORION_BLIT_BASE 0x40000000
#define  ORION_BLIT_SIZE 0x100

#define BLIT_MAJOR  	0

static int blit_major 	=  BLIT_MAJOR;

module_param		( blit_major, int, BLIT_MAJOR );
MODULE_PARM_DESC 	( blit_major, "BLIT major number");

/*
 * The devices
 */
blit_dev*  pblit_dev = NULL;	/* allocated in blit_init_module */

/* function declaration ------------------------------------- */
static int __init 	blit_init(void);
static void __exit 	blit_exit(void);

static void 		blit_setup_cdev(blit_dev *dev, int index);

static int		blit_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

static spinlock_t 	blit_lock;
/* ----------------------------------------------------------- */

/* blit device struct */
struct file_operations blit_fops = 
{
	.owner 	=  THIS_MODULE,
	.ioctl  =  blit_ioctl,
};

static void blit_setup_cdev (blit_dev *dev, int index)
{
	int errorcode = 0;
	char devfs_name[20];
	
	int devno = MKDEV ( blit_major, index);
    
	cdev_init ( &dev->cdev, &blit_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &blit_fops;
	
	errorcode = cdev_add (&dev->cdev, devno, 1);
	if ( errorcode )
	{
		printk(KERN_NOTICE "BLIT: Error %d adding blit %d", errorcode, index);
	}

	snprintf(devfs_name, sizeof(devfs_name), "misc/orion_blit");  //only one device actually, "index" is useless
        errorcode = devfs_mk_cdev(devno, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP, devfs_name);
	if ( errorcode )
	{
		printk(KERN_NOTICE "BLIT: Error %d make blit node %d", errorcode, index);
	}
}

static int blit_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int err = -1;
    union _cmd_param_
    {
	CSBLIT_FillRectParams_t fillparam;
	CSBLIT_CopyRectParams_t copyparam;
	CSBLIT_ScalorParams_t 	scalorparam;
	CSBLIT_CompParams_t 	compparam;
    } cmd_param;

    CSGFXOBJ_Rectangle_t tmprect;		/* only for dest rectangle copy */

    if ( pblit_dev == NULL )
	return -EINVAL;
    if ( ( _IOC_TYPE(cmd) != BLIT_MAGIC ) || ( _IOC_NR(cmd) > BLIT_MAXNR ))
	return -ENOTTY;

    if ( _IOC_DIR(cmd) & _IOC_READ )
    {
	err = !access_ok ( VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if ( _IOC_DIR(cmd) & _IOC_WRITE )
    {
	err = !access_ok ( VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if ( err )
	return -EFAULT;

    /* into critical resource */
    spin_lock(&blit_lock);
    switch(cmd) 
    {
	case CMD_FILL:
	    if(copy_from_user(&cmd_param.fillparam, (void *)arg, sizeof(CSBLIT_FillRectParams_t)))
		goto ERR_INVAL;
	    err = Fill(&pblit_dev->hwdev, 1, 		\
			&cmd_param.fillparam.Bitmap, 	\
			&cmd_param.fillparam.Rectangle,	\
			&cmd_param.fillparam.Color, 	\
			cmd_param.fillparam.AluMode);
	    break;

	case CMD_COPY:
	    if(copy_from_user(&cmd_param.copyparam, (void *)arg, sizeof(CSBLIT_CopyRectParams_t)))
		goto ERR_INVAL;

	    tmprect.PositionX = cmd_param.copyparam.DstPositionX;
	    tmprect.PositionY = cmd_param.copyparam.DstPositionY;
	    tmprect.Width = cmd_param.copyparam.SrcRectangle.Width;
	    tmprect.Height = cmd_param.copyparam.SrcRectangle.Height;
	    err = Copy(&pblit_dev->hwdev, 1, 		\
		    &cmd_param.copyparam.SrcBitmap,	\
		    &cmd_param.copyparam.SrcRectangle,	\
		    0, 					\
		    &cmd_param.copyparam.DstBitmap,	\
		    &tmprect,				\
		    cmd_param.copyparam.AluMode,  0);
	    break;

	case CMD_SCALOR:
	    if(copy_from_user(&cmd_param.scalorparam, (void *)arg, sizeof(CSBLIT_ScalorParams_t)))
		goto ERR_INVAL;

	    if(cmd_param.scalorparam.VInitialPhase == 0 || cmd_param.scalorparam.VInitialPhase > 7)
		cmd_param.scalorparam.VInitialPhase = 4;
	    if(cmd_param.scalorparam.HInitialPhase > 7)
		cmd_param.scalorparam.HInitialPhase = 0;

	    err = Scalor(&pblit_dev->hwdev,  		\
		    &cmd_param.scalorparam.Src,		\
		    &cmd_param.scalorparam.Dst,		\
		    cmd_param.scalorparam.VInitialPhase,\
		    cmd_param.scalorparam.HInitialPhase,\
		    cmd_param.scalorparam.AluMode);
	    break;

	case CMD_COMP:
	    if(copy_from_user(&cmd_param.compparam, (void *)arg, sizeof(CSBLIT_CompParams_t)))
		goto ERR_INVAL;

	    if(cmd_param.compparam.Src0.Type == CSBLIT_SOURCE_TYPE_BITMAP && 
		    cmd_param.compparam.Src1.Type == CSBLIT_SOURCE_TYPE_BITMAP)
	    {
		err = Composite(&pblit_dev->hwdev,		\
			&cmd_param.compparam.Src0, 		\
			&cmd_param.compparam.Src1, 		\
			&cmd_param.compparam.Dst, 		\
			cmd_param.compparam.AluMode, 		\
			cmd_param.compparam.BlendEnable, 	\
			cmd_param.compparam.IsS0OnTopS1, 	\
			cmd_param.compparam.ROPAlphaCtrl);
	    }
	    else if(cmd_param.compparam.Src0.Type == CSBLIT_SOURCE_TYPE_COLOR &&
		    cmd_param.compparam.Src1.Type == CSBLIT_SOURCE_TYPE_BITMAP)
	    {
		err = CompositeSrc1(&pblit_dev->hwdev,		\
			&cmd_param.compparam.Src0, 		\
			&cmd_param.compparam.Src1, 		\
			&cmd_param.compparam.Dst, 		\
			cmd_param.compparam.AluMode, 		\
			cmd_param.compparam.BlendEnable, 	\
			cmd_param.compparam.IsS0OnTopS1, 	\
			cmd_param.compparam.ROPAlphaCtrl);
	    }
	    else if(cmd_param.compparam.Src1.Type == CSBLIT_SOURCE_TYPE_COLOR &&
		    cmd_param.compparam.Src0.Type == CSBLIT_SOURCE_TYPE_BITMAP)
	    {
		err = CompositeSrc1(&pblit_dev->hwdev,		\
			&cmd_param.compparam.Src0, 		\
			&cmd_param.compparam.Src1, 		\
			&cmd_param.compparam.Dst, 		\
			cmd_param.compparam.AluMode, 		\
			cmd_param.compparam.BlendEnable, 	\
			!cmd_param.compparam.IsS0OnTopS1,	/*src0/1 reversed*/ 	\
			cmd_param.compparam.ROPAlphaCtrl);
	    }
	    else 
	    {
		printk(KERN_WARNING "Slave Now can't support 2 A0 Source Composite!\n");
		goto ERR_INVAL;
	    }

	    break;

	default:
		goto ERR_INVAL;
    }

    spin_unlock(&blit_lock);
    return err;

ERR_INVAL:
    spin_unlock(&blit_lock);
    return -EINVAL;
}

static int __init blit_init(void)
{
	int result;
	dev_t dev = 0;
	u32 blit_base;

	if (blit_major)	{
		dev = MKDEV (blit_major, 0);
		result = register_chrdev_region(dev, 1, "blit");
	} 
	else {
		result = alloc_chrdev_region(&dev, 0, 1, "blit");
		blit_major = MAJOR(dev);
	}

	if (result < 0) {
		printk(KERN_WARNING "BLIT: can't get major %d\n", blit_major);
		return result;
	}

        if (!request_mem_region(ORION_BLIT_BASE, ORION_BLIT_SIZE, "ORION BLIT"))
        {
		unregister_chrdev_region(dev, 1);
                return -EIO;
        }
	
	blit_base = (u32)ioremap(ORION_BLIT_BASE, ORION_BLIT_SIZE);
	if(!blit_base) {
		unregister_chrdev_region (dev, 1);
		printk(KERN_WARNING "BLIT: ioremap failed.\n");
		return -EIO;
	}

//printk("size_blit_dev:%d, %d",sizeof(blit_dev),sizeof(CSBlit_HW_Device_t));
	pblit_dev = kmalloc ( 1 * sizeof(blit_dev), GFP_KERNEL);
	if (!pblit_dev) {
		iounmap((void *)blit_base);
		release_mem_region(ORION_BLIT_BASE, ORION_BLIT_SIZE);
        	unregister_chrdev_region (dev, 1);
		return -ENOMEM;
	}
	memset ( pblit_dev, 0, 1 * sizeof(blit_dev));

	spin_lock_init(&blit_lock);
	
	spin_lock(&blit_lock);

	/* Initialize hardware device. */
	pblit_dev->hwdev.RegBaseAddr = blit_base;
	pblit_dev->hwdev.BlitType = BLIT_TYPE_1200;	//for Orion1.4
	blit_initialize(&pblit_dev->hwdev);

	spin_unlock(&blit_lock);

	blit_setup_cdev( pblit_dev, 0 );

        printk(KERN_INFO "ORION BLIT at 0x%x, size: 0x%x\n", ORION_BLIT_BASE, ORION_BLIT_SIZE);

	return 0;
}

static void __exit blit_exit(void)
{
	dev_t devno = MKDEV(blit_major, 0);

	if (pblit_dev) 
	{
	    cdev_del  ( &pblit_dev->cdev );
	    iounmap((void *)pblit_dev->hwdev.RegBaseAddr); // maybe 0 ~ BLIT_NR_DEVS-1
	    kfree ( pblit_dev );
	    pblit_dev = NULL;
	}

        release_mem_region(ORION_BLIT_BASE, ORION_BLIT_SIZE);
	unregister_chrdev_region (devno, 1);
}

module_init(blit_init);
module_exit(blit_exit);

MODULE_AUTHOR("Xm.chen,Celestial Semiconductor");
MODULE_DESCRIPTION("Celestial Semiconductor BLIT driver");
MODULE_LICENSE("GPL");
