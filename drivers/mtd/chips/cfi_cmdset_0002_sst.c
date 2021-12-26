/*
 * Created by lichq 20081114, celestailsemi
 * 
 * To support supplemental feature to operate the security ID space of sst flash.
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>


enum {
    SST_READ_SID,
    SST_READ_UID,
    SST_WRITE_UID,
    SST_LOCK_SID_SPACE        
};

struct sid_uid_buf {
    unsigned short ids[8];
};

#define SST_MANUFACT	0x00BF

static int SST_DEBUG = 0;
#define DBG(format, args...)\
do {\
if (SST_DEBUG) {\
    printk(format, ## args);}\
}while(0)


void __iomem *sst_flash_virt_base = NULL;
extern struct semaphore ebi_mutex_lock;
static int  major_dev = -1;


static int sst_qry_present(void) 
{
    int i;
    unsigned short cfi_data[8];
	volatile unsigned short *addr0 = sst_flash_virt_base;

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x0098;

    for (i = 0x10; i < 0x18; i++) {
        cfi_data[i - 0x10] = addr0[i];
    }

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x00F0;

    if (('Q' == (unsigned char)cfi_data[0]) && 
        ('R' == (unsigned char)cfi_data[1]) && 
        ('Y' == (unsigned char)cfi_data[2])) {
        return 1;
    }

    return 0;

}


static int sst_check_manu(void) 
{
    int i;
    unsigned short manu_data;
	volatile unsigned short *addr0 = sst_flash_virt_base;

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x0090;
    
    manu_data = addr0[0];

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x00F0;

    if (manu_data == SST_MANUFACT) {
        return 1;
    }

    return 0;

}

static int sst_lock_sid_space(void) 
{
   	volatile unsigned short *addr0 = sst_flash_virt_base;
    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x0085;
    addr0[0x00FF] = 0x0000;
    udelay(10);

    return 0;
}


static int sst_toggle_ready(unsigned short dst) 
{
    int i;
    unsigned short old, ready;
    volatile unsigned short *addr0 = sst_flash_virt_base;

    for (i = 0; i < 0x07FFFFFF; i++) {
        old = addr0[dst];        
        old = old & 0x0040;

        ready = addr0[dst];        
        ready = ready & 0x0040;
        if (old == ready) {
            return 1;
        }
    }

    return 0;
}


static int sst_uid_write(struct sid_uid_buf* buf) 
{
    int i;
    volatile unsigned short *addr0 = sst_flash_virt_base;
    
    for (i = 0x10; i < 0x18; i++) {
        addr0[0x555] = 0x00AA;
        addr0[0x2AA] = 0x0055;
        addr0[0x555] = 0x00A5;
        udelay(1);
        
        addr0[i] = buf->ids[i - 0x10];

        if (sst_toggle_ready(i)) {
            DBG("sid byte[%d] program ok\n", i);
        } else {
            DBG("sid byte[%d] program failed\n", i);
        }
    }

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x00F0;
    udelay(1);

    return 0;
}


static int sst_sid_uid_read(struct sid_uid_buf* buf, u_int cmd) 
{
    int i;
   	volatile unsigned short *addr0 = sst_flash_virt_base;
    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x0088;
    udelay(1);

    if (cmd == SST_READ_SID) {
        for (i = 0x0; i < 0x8; i++) {
            buf->ids[i] = addr0[i];
        }
    } else {
        for (i = 0x10; i < 0x18; i++) {
            buf->ids[i - 0x10] = addr0[i];
        }
    }

    addr0[0x555] = 0x00AA;
    addr0[0x2AA] = 0x0055;
    addr0[0x555] = 0x00F0;
    udelay(1);

  	DBG("SID : \n");
    for (i = 0x0; i < 0x8; i++) {
        DBG("%04x ", buf->ids[i]);
    }
    DBG("\n");
    for (i = 0x10; i < 0x18; i++) {
        DBG("%04x ", buf->ids[i]);
    }
    DBG("\n");

    return 0;
}


static int sst_ioctl(struct inode *inode, struct file *filp, u_int cmd, u_long arg)
{
    int ret = 0;
    void __user *uarg = (char __user *)arg;
    struct sid_uid_buf tmp_sid_uid_buf;

    down(&ebi_mutex_lock);

    switch (cmd) {
        case SST_READ_SID:
        case SST_READ_UID:
        {
            if (sst_sid_uid_read(&tmp_sid_uid_buf, cmd)) {
                ret = -EIO;
                break;
            }
            if (copy_to_user(uarg, &tmp_sid_uid_buf, sizeof(tmp_sid_uid_buf))) {
                ret = -EFAULT;
            }
        }
        break;
        case SST_WRITE_UID:
        {
            if (copy_from_user((unsigned char *)(&tmp_sid_uid_buf), uarg, sizeof(tmp_sid_uid_buf))) {
                ret = -EFAULT;
                break;
            }
            if (sst_uid_write(&tmp_sid_uid_buf)) {
                ret = -EIO;
            }
        }
        break;
        case SST_LOCK_SID_SPACE:
        {
            if (sst_lock_sid_space()) {
                ret = -EIO;
            }
        }
        break;
        default:
        {
            ret = -EIO;
        }
        break;
    }

    up(&ebi_mutex_lock);
	return ret;
} /* memory_ioctl */


static int sst_open(struct inode *inode, struct file *file)
{
	return 0;
}


static int sst_close(struct inode *inode, struct file *file)
{
	return 0;
}


static struct file_operations sst_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= sst_ioctl,
	.open		= sst_open,
	.release	= sst_close,
};


static struct proc_dir_entry *sst_proc_entry = NULL;


static int sst_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned int addr;
	unsigned int val;
	const char *cmd_line = buffer;

    down(&ebi_mutex_lock);
	if (strncmp("cfi", cmd_line, 3) == 0) {
        if (sst_qry_present()) {
            printk("CFI: QRY exits\n");
        }
    } else if (strncmp("man", cmd_line, 3) == 0) {
        if (sst_check_manu()) {
            printk("SST flash detected\n");
        }
    } else if (strncmp("sid", cmd_line, 3) == 0) {
        struct sid_uid_buf tmp_sid_uid_buf;
        sst_sid_uid_read(&tmp_sid_uid_buf, SST_READ_SID);
        printk("SID: %04x %04x %04x %04x %04x %04x %04x %04x\n", 
            tmp_sid_uid_buf.ids[0], tmp_sid_uid_buf.ids[1], tmp_sid_uid_buf.ids[2], tmp_sid_uid_buf.ids[3],
            tmp_sid_uid_buf.ids[4], tmp_sid_uid_buf.ids[5], tmp_sid_uid_buf.ids[6], tmp_sid_uid_buf.ids[7]);
	} else if (strncmp("uid", cmd_line, 3) == 0) {
        struct sid_uid_buf tmp_sid_uid_buf;
        sst_sid_uid_read(&tmp_sid_uid_buf, SST_READ_UID);
        printk("UID: %04x %04x %04x %04x %04x %04x %04x %04x\n", 
            tmp_sid_uid_buf.ids[0], tmp_sid_uid_buf.ids[1], tmp_sid_uid_buf.ids[2], tmp_sid_uid_buf.ids[3],
            tmp_sid_uid_buf.ids[4], tmp_sid_uid_buf.ids[5], tmp_sid_uid_buf.ids[6], tmp_sid_uid_buf.ids[7]);
	} else if (strncmp("slk", cmd_line, 3) == 0) {
        sst_lock_sid_space();
        printk("Security ID space locked\n");
	} else if (strncmp("dbg", cmd_line, 3) == 0) {
        if (SST_DEBUG) {
            SST_DEBUG = 0;
        } else {
            SST_DEBUG = 1;
        }
	} else {
    	printk(KERN_ERR "Illegal command\n");
	}

    up(&ebi_mutex_lock);

    return count;   
}


static int __init init_sst(void)
{
    sst_flash_virt_base = ioremap(0x34000000, 0x800000);
  	if (sst_flash_virt_base == NULL) {
        printk(KERN_ERR "SST: unable to remap memory\n");
        return -ENOMEM;
    }

    if (!sst_qry_present() || !sst_check_manu()) {
        iounmap(sst_flash_virt_base);
        sst_flash_virt_base = 0;
        printk("Not SST flash.");
        return 0;
    }

    major_dev = register_chrdev(0, "sst_raw_flash", &sst_fops);
    if (major_dev < 0) {
        printk(KERN_NOTICE "unable to find a free device for sst raw driver (error=%d)\n", major_dev);
        major_dev = -1;
        if (sst_flash_virt_base > 0) {
            iounmap(sst_flash_virt_base);
            sst_flash_virt_base = 0;
        }
        return -ENOMEM;
    }

	sst_proc_entry = create_proc_entry("sst_io", 0, NULL);
	if (NULL != sst_proc_entry) {
		sst_proc_entry->write_proc = &sst_proc_write;
        printk("Succesefully create sst proc entry!\n");
	} else {
	    printk("Failed to create sst proc entry!\n");
	}

    return 0;
}


static void __exit cleanup_sst(void)
{
	if (major_dev != -1) {
        unregister_chrdev(major_dev, "sst_raw_flash");
        major_dev = -1;
    }

    if (sst_flash_virt_base > 0) {
        iounmap(sst_flash_virt_base);
        sst_flash_virt_base = 0;
    }

    remove_proc_entry(sst_proc_entry, NULL);
    sst_proc_entry = NULL;
    
    return;
}

module_init(init_sst);
module_exit(cleanup_sst);

MODULE_AUTHOR("LICHQ");
MODULE_DESCRIPTION("SST FLASH DRIVER FOR SECURE ID SPACE");
MODULE_LICENSE("GPL");

