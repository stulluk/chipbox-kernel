#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <asm/io.h>

#define ORIONRC_BASE		0x10172000
#define ORIONRC_SIZE		0x1000
#define ORIONRC_INT_STA 	(0x000 >> 1)
#define ORIONRC_INT_MSK 	(0x004 >> 1)
#define ORIONRC_CTL_LO		(0x008 >> 1)
#define ORIONRC_CTL_HI		(0x00A >> 1)
#define ORIONRC_RC5_LO  	(0x100 >> 1)
#define ORIONRC_RC5_HI  	(0x102 >> 1)
#define ORIONRC_NEC_LO  	(0x104 >> 1)
#define ORIONRC_NEC_HI		(0x106 >> 1)
#define ORIONKEY_DATA_L 	(0x300 >> 1) /* Key Scan Data Register */
#define ORIONKEY_CNTL_L		(0x304 >> 1) /* Key Scan Control Register */
#define ORIONKEY_CNTL_H 	(0x306 >> 1) /* Key Scan Control Register */

#define ORIONRC_IRQ		8
#define DRIVER_DESC		"ORION Remote Controller"

#define REPEAT_PERIOD		150	/* NEC: 110ms, RC5: 114ms */

#define RC_HEAD 0x0000
#define KEYSCAN_HEAD 0x1000
#define FPC_KEY_NUM  0x1ff

MODULE_AUTHOR("Tu BaoZhao <bz.tu@celestialsemi.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

DEFINE_SPINLOCK(orionrc_lock);

extern int orion_gpio_register_module_status(const char * module, unsigned int  orion_module_status); /* status: 0 disable, 1 enable */

extern int rc_system_code; /* it locates drivers/char/orion_fpc.c */

static char *orionrc_name = DRIVER_DESC;

struct orionrc {
	struct input_dev dev;
	struct timer_list timer;
	unsigned short keycode[FPC_KEY_NUM];
	char phys[32];
	unsigned long current_key;
};

static int orionrc_init_flags = 0;
static struct orionrc *orionrc;
static int repeat_lock=0;
static volatile u_short *pctl_base;

int send_input_event(int cmd)
{
	unsigned long  flags;
	spin_lock_irqsave(&orionrc_lock, flags);

	if (orionrc_init_flags) {
		if(!(orionrc->current_key & 0x80000000)) {
			input_report_key(&orionrc->dev, orionrc->current_key & 0x1ff, 0);
			input_sync(&orionrc->dev);
		}       
		input_report_key(&orionrc->dev, cmd, 1);
		input_sync(&orionrc->dev);
		orionrc->current_key = cmd;
		mod_timer(&orionrc->timer, jiffies + msecs_to_jiffies(REPEAT_PERIOD));
	}

	spin_unlock_irqrestore(&orionrc_lock, flags);

	return 0;
}

static void release_key(unsigned long data)
{
	unsigned long key;
	unsigned long  flags;

	spin_lock_irqsave(&orionrc_lock, flags);

	key = orionrc->current_key & 0x1ff;
	
	if(orionrc->current_key & 0x80000000) { /* key release event already emit */
		repeat_lock =0;	
		return;
	}
	if(orionrc->current_key & 0x40000000) {	/* key still pressed */
		orionrc->current_key &= ~0x40000000;
		mod_timer(&orionrc->timer, jiffies + msecs_to_jiffies(REPEAT_PERIOD));
		return;	
	}
		
	input_report_key(&orionrc->dev, key, 0);
	input_sync(&orionrc->dev);
	orionrc->current_key |= 0x80000000;
	repeat_lock =0;
	spin_unlock_irqrestore(&orionrc_lock, flags);
}

static irqreturn_t orionrc_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
	unsigned short intr_status;
	unsigned short cmd_lo;
	unsigned short cmd_hi;
	unsigned char  valid;
	unsigned short cmd;
	unsigned char  repeat;
	unsigned long  flags;
#ifdef CONFIG_ORIONRC_RC5
	static unsigned char toggle = 0xff;
#endif	

	intr_status = pctl_base[ORIONRC_INT_STA];
	if(intr_status & 0x1) {
#ifdef CONFIG_ORIONRC_RC5
		cmd_lo = pctl_base[ORIONRC_RC5_LO];
		cmd_hi = pctl_base[ORIONRC_RC5_HI];
		valid  = cmd_lo & 0x4000;
		cmd    = (cmd_lo & 0x3f) | ((~cmd_lo >> 6) & 0x40); /* RC5X, S2 as 7th CMD */
		repeat = (cmd_lo & 0x0800) == toggle;
		toggle = (cmd_lo & 0x0800);
#else
		cmd_lo = pctl_base[ORIONRC_NEC_LO];
		cmd_hi = pctl_base[ORIONRC_NEC_HI];
		valid  = cmd_hi & 0x2;
		cmd    = cmd_lo & 0xff;
		repeat = cmd_hi & 0x1;

		if (0xffffffff != rc_system_code) {
			if ((cmd_lo >> 8) != rc_system_code) 
				valid = 0;
		}
#endif
		//static int count=0;
		//count ++;
		//printk("in interrupt ---count=%d---key=0x%x---valid=%d--repeat=%d---",count,orionrc->current_key,valid,repeat);
		//printk("                                                  cmd=0x%x key=0x%x lok=%d rept=%d  syscod=0x%x\n",cmd,orionrc->current_key,repeat_lock,repeat,(cmd_lo >> 8));
		if(valid) {
			
			if( (orionrc->current_key & 0x80000000) && (repeat == 1) && ((orionrc->current_key&0xff) == cmd) ){
				//orionrc->current_key++;
				return IRQ_HANDLED;
			}

			spin_lock_irqsave(&orionrc_lock, flags);
			
			if(!repeat || repeat_lock == 0) {
				if(!(orionrc->current_key & 0x80000000)) {
					input_report_key(&orionrc->dev, (orionrc->current_key & 0x1ff), 0);
					//printk("(Report a Key 1 0x%x)\n", orionrc->current_key & 0x1ff);
					input_sync(&orionrc->dev);
				}

				input_report_key(&orionrc->dev, cmd, 1);
		    	//printk("(Report a Key 2 0x%x)\n", orionrc->current_key & 0x1ff);

				input_sync(&orionrc->dev);
				orionrc->current_key = cmd;
				repeat_lock = 1;
				mod_timer(&orionrc->timer, jiffies + msecs_to_jiffies(REPEAT_PERIOD));
			} else {
				orionrc->current_key |= 0x40000000;

			}

			spin_unlock_irqrestore(&orionrc_lock, flags);
		}
	}
	else if(intr_status & 0x2) {
		static unsigned long ticks = 0;
		unsigned long curr_ticks = jiffies; 
		
		if (curr_ticks - ticks > 23) {
			cmd_lo = pctl_base[ORIONKEY_DATA_L];
			cmd = cmd_lo & 0xff;

			if (cmd_lo & 0x80) {
				spin_lock_irqsave(&orionrc_lock, flags);

				cmd += 0x100; /* for distinguishing key source that it's from KeyScan. */

				if(!(orionrc->current_key & 0x80000000)) {

					input_report_key(&orionrc->dev, (orionrc->current_key & 0x1ff) , 0);
					input_sync(&orionrc->dev);
				}       
				input_report_key(&orionrc->dev, cmd , 1);
				input_sync(&orionrc->dev);
				orionrc->current_key = cmd;
				mod_timer(&orionrc->timer, jiffies + msecs_to_jiffies(REPEAT_PERIOD));

				spin_unlock_irqrestore(&orionrc_lock, flags);
			}

			ticks = curr_ticks;
		}
	}

	return IRQ_HANDLED;
}

static int __init orionrc_init(void)
{
	int i;
	unsigned long rc_mode;
	unsigned long panel_sel;
	unsigned long bit_time_cnt;

        if (request_irq(ORIONRC_IRQ, orionrc_interrupt, 0, "orionrc", NULL)) {
                printk(KERN_ERR "orionrc.c: Can't allocate irq %d\n", ORIONRC_IRQ);
                return -EBUSY;
        }

	if(!(pctl_base = (u_short *)ioremap(ORIONRC_BASE, ORIONRC_SIZE))) {
		free_irq(ORIONRC_IRQ, NULL);

		return -EBUSY;
	}

	if (!(orionrc = kmalloc(sizeof(struct orionrc), GFP_KERNEL))) {
		free_irq(ORIONRC_IRQ, NULL);
		iounmap((void *)pctl_base);
		return -ENOMEM;
	}

	memset(orionrc, 0, sizeof(struct orionrc));

	init_timer(&orionrc->timer);
	orionrc->current_key = 0x80000000;
	orionrc->timer.function = release_key;

	orionrc->dev.evbit[0] = BIT(EV_KEY) | BIT(EV_REP);

	init_input_dev(&orionrc->dev);
	orionrc->dev.keycode = orionrc->keycode;
	orionrc->dev.keycodesize = sizeof(unsigned short);
	orionrc->dev.keycodemax = ARRAY_SIZE(orionrc->keycode);
	orionrc->dev.private = orionrc;

	for (i = 0; i < FPC_KEY_NUM; i++)			/* Output RAW Code */
		orionrc->keycode[i] = i;

	for (i = 0; i < FPC_KEY_NUM; i++)
		set_bit(orionrc->keycode[i], orionrc->dev.keybit);

	orionrc->dev.name = orionrc_name;
	orionrc->dev.phys = "orionrc";
	orionrc->dev.id.bustype = BUS_HOST;
	orionrc->dev.id.vendor = 0x0001;
	orionrc->dev.id.product = 0x0001;
	orionrc->dev.id.version = 0x0104;

	input_register_device(&orionrc->dev);

	/* default repeat is too fast */
	orionrc->dev.rep[REP_DELAY]   = 500;
        orionrc->dev.rep[REP_PERIOD]  = 100;

#define PCLK_PERIOD	1818    /* in 10 picosecond */
	panel_sel    = 11;	
#ifdef CONFIG_ORIONRC_RC5
	rc_mode	     = 0x0;
	bit_time_cnt = 177800000 / PCLK_PERIOD;
#else
	rc_mode	     = 0x1;
	bit_time_cnt = 112000000 / PCLK_PERIOD;
#endif
	pctl_base[ORIONRC_CTL_LO]  = rc_mode | (panel_sel << 2) | ((bit_time_cnt & 0xff) << 8);
	pctl_base[ORIONRC_CTL_HI]  = 0xc000 | (bit_time_cnt >> 8);  /* enable, inv_base_pol */
	pctl_base[ORIONKEY_CNTL_L] = 0x0001;
	pctl_base[ORIONKEY_CNTL_H] = 0x000f;
	orion_gpio_register_module_status("FPC", 1);

	pctl_base[ORIONRC_INT_MSK] &= ~0x3;                         /* enable IRQ */

	orionrc_init_flags = 1;
	/* to inform GPIO driver that this driver will hold some GPIOs. */
	orion_gpio_register_module_status("RC", 1);

	return 0;
}

static void __exit orionrc_exit(void)
{
	orionrc_init_flags = 0;

	free_irq(ORIONRC_IRQ, NULL);
	iounmap((void *)pctl_base);
	del_timer_sync(&orionrc->timer);
	input_unregister_device(&orionrc->dev);
	kfree(orionrc);
}

module_init(orionrc_init);
module_exit(orionrc_exit);

EXPORT_SYMBOL(send_input_event);

