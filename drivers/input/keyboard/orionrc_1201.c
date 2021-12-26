#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/current.h>

#include "orionrc_1201.h"


//#define REPEAT_PERIOD		150	/* NEC: 110ms, RC5: 114ms */

#define rc_readw(a)		readw((orionrc_base + (a)))
#define rc_writew(a, v)		writew(v, (orionrc_base + (a)))

/* Global variables */
static orionrc_t *rc_dev;
static void __iomem *orionrc_base;
static struct proc_dir_entry *rc_proc_entry;
extern int rc_system_code; /* it locates drivers/char/orion_fpc.c */

/* Predefined value */
typedef struct {
	unsigned int h_low;
	unsigned int h_high;
	unsigned int t_low;
	unsigned int t_high;
} pta_t;	/* protocal time ajust */

/* NEC State Machine */
#define NEC_HEAD   	0
#define NEC_REPEAT  	1
#define NEC_ZERO    	2
#define NEC_ONE     	3
#define NEC_DROP   	4

#define NEC_ENTRY 4
static pta_t pta_nec[NEC_ENTRY] = {	
	{ 8500, 9500, 13000, 14000 },	/* head */
	{ 8500, 9500, 11000, 12000 },	/* repeat */
	{ 450, 700, 1000, 1200 },	/* zero */
	{ 450, 700, 2100, 2300 }	/* one */
};

/* RC5 State Machine */
#define RC5_ONE_BIT	0
#define RC5_ZO_BITS	1
#define RC5_OZ_BITS	2
#define RC5_OZO_BITS	3
#define RC5_DROP	4

static pta_t pta_rc5[] = {
	{ 750, 1050, 1600, 1800 },	/* normal */
	{ 750, 1050, 2400, 2600 },	/* zero one one */
	{ 1600, 1800, 2400, 2600 },	/* one zero zero */
	{ 1600, 1800, 3200, 3400 },	/* one zero one */
};

/* Decode protocal */
static int orion_nec_decode_bit(unsigned int high_val, unsigned int total_val)
{
	int ret = NEC_DROP;

	if((high_val >= pta_nec[NEC_HEAD].h_low) && 
	   (high_val <= pta_nec[NEC_HEAD].h_high) && 
	   (total_val >= pta_nec[NEC_HEAD].t_low) && 
	   (total_val <= pta_nec[NEC_HEAD].t_high)) {

		PDEBUG("NEC_HEAD: high_val: %d, total_val: %d\n", high_val, total_val);
		ret = NEC_HEAD;

	} else if((high_val >= pta_nec[NEC_REPEAT].h_low) && 
		(high_val <= pta_nec[NEC_REPEAT].h_high) && 
		(total_val >= pta_nec[NEC_REPEAT].t_low) && 
		(total_val <= pta_nec[NEC_REPEAT].t_high)) {

		PDEBUG("NEC_REPEAT: high_val: %d, total_val: %d\n", high_val, total_val);
		ret = NEC_REPEAT;

	} else if((high_val >= pta_nec[NEC_ZERO].h_low) && 
		(high_val <= pta_nec[NEC_ZERO].h_high) && 
		(total_val >= pta_nec[NEC_ZERO].t_low) && 
		(total_val <= pta_nec[NEC_ZERO].t_high)) {

		PDEBUG("NEC_ZERO: high_val: %d, total_val: %d\n", high_val, total_val);
		ret = NEC_ZERO;

	} else if ((high_val >= pta_nec[NEC_ONE].h_low) && 
		 (high_val <= pta_nec[NEC_ONE].h_high) && 
		 (total_val >= pta_nec[NEC_ONE].t_low) && 
		 (total_val <= pta_nec[NEC_ONE].t_high)) {

		PDEBUG("NEC_ONE : high_val: %d, total_val: %d\n", high_val, total_val);
		ret = NEC_ONE;

	} else {

		PDEBUG("XXXXXXXXX_DROP: high_val: %d, total_val: %d\n", high_val, total_val);
		ret = NEC_DROP;
	}

	return ret;		
}

static int orion_nec_decode(int idx)
{
	int nec_st;

	PDEBUG("idx %2d ", idx);
	nec_st = orion_nec_decode_bit(rc_dev->fifo1[idx], rc_dev->fifo2[idx]);

	switch(rc_dev->record.decode_period) {
		case HEAD_PERIOD:              /* Head Period */
			switch (nec_st) {
				case NEC_HEAD:
					rc_dev->record.bit = 0;
					rc_dev->record.rpt = 0;
					rc_dev->record.decode_period = ADDR_PERIOD;
					break;

				case NEC_REPEAT:
					rc_dev->record.bit = 0;
					// rc_dev->max_rpt = 2; /* FIXME: To be set outside */
#if 1
					if (rc_dev->record.rpt == (rc_dev->max_rpt-1)) {
						rc_dev->record.rpt = 0;
						return 1;
					} else {
						rc_dev->record.rpt++;
					}
#else	/* for test */
					rc_dev->record.rpt++;
					return 1;
#endif
					break;

				case NEC_DROP:
				case NEC_ZERO:
				case NEC_ONE:
				default:
					rc_dev->record.bit = 0;
			}

			break;

		case ADDR_PERIOD:        /* Address Period */
			switch (nec_st) {
				case NEC_HEAD:
					rc_dev->record.bit = 0;
					rc_dev->record.rpt = 0;
					rc_dev->record.decode_period = ADDR_PERIOD;
					return 0;
				case NEC_REPEAT:
					rc_dev->record.rpt++;
					rc_dev->record.decode_period = HEAD_PERIOD;
					break;
				case NEC_DROP:
					rc_dev->record.bit = 0;
					rc_dev->record.rpt = 0;
					rc_dev->record.decode_period = HEAD_PERIOD;
					break;
				case NEC_ZERO:
					clear_bit(rc_dev->record.bit, (unsigned long *)&rc_dev->record.addr);
					break;
				case NEC_ONE:
					set_bit(rc_dev->record.bit, (unsigned long *)&rc_dev->record.addr);
					break;
			}

			if(rc_dev->record.bit == (16-1)) {
				rc_dev->record.bit = 0;
				rc_dev->record.addr = rc_dev->record.addr & 0xffff;
				rc_dev->record.decode_period = CMD_PERIOD;	/* FIXME: Sun: temparary enable cmd process */
			} else {
				rc_dev->record.bit++;
			}
#if 0
			if(rc_dev->addr == rc_dev->record.addr)
				rc_dev->record.decode_period = CMD_PERIOD;
			else
				rc_dev->record.decode_period = HEAD_PERIOD;
#endif

			break;

		case CMD_PERIOD:                                           /* Command Period */
			switch (nec_st) {
				case NEC_HEAD:
					rc_dev->record.bit = 0;
					rc_dev->record.rpt = 0;
					rc_dev->record.decode_period = ADDR_PERIOD;
					return 0;
				case NEC_REPEAT:
				case NEC_DROP:
					rc_dev->record.bit = 0;
					rc_dev->record.rpt = 0;
					rc_dev->record.decode_period = HEAD_PERIOD;
					break;
				case NEC_ZERO:
					clear_bit(rc_dev->record.bit, (unsigned long *)&rc_dev->record.cmd);
					break;
				case NEC_ONE:
					set_bit(rc_dev->record.bit, (unsigned long *)&rc_dev->record.cmd);
					break;
			}

			if(rc_dev->record.bit == (16-1)) {
				rc_dev->record.bit = 0;
				rc_dev->record.cmd = rc_dev->record.cmd & 0xff;
				rc_dev->record.decode_period = HEAD_PERIOD;
				return 1;
			} else {
				rc_dev->record.bit++;
			}

			break;

		default:
			;	
	}  

	return 0;
}

/*RC5 Decorder*/
static unsigned int orion_rc5_decode_bit(unsigned int high_val,unsigned int total_val)
{
	int ret = RC5_DROP;

	if(((high_val >= pta_rc5[RC5_ONE_BIT].h_low) && 
	   (high_val <= pta_rc5[RC5_ONE_BIT].h_high)) && 

	   (((total_val >= pta_rc5[RC5_ONE_BIT].t_low) && 
	    (total_val <= pta_rc5[RC5_ONE_BIT].t_high)) || 

	    rc_dev->record.bit == 13)) { /* total_val equal 0 , this means received last  one bit, (one or zero) */

		PDEBUG("RC5_ONE_BIT: bit: %d, high_val: %d, total_val: %d\n", rc_dev->record.bit, high_val, total_val);
		ret = RC5_ONE_BIT;

	} else if((high_val >= pta_rc5[RC5_ZO_BITS].h_low) && 
		(high_val <= pta_rc5[RC5_ZO_BITS].h_high) && 
		(total_val >= pta_rc5[RC5_ZO_BITS].t_low) && 
		(total_val <= pta_rc5[RC5_ZO_BITS].t_high)) {

		PDEBUG("RC5_ZO_BITS: bit: %d, high_val: %d, total_val: %d\n", rc_dev->record.bit, high_val, total_val);
		ret = RC5_ZO_BITS;

	} else if(((high_val >= pta_rc5[RC5_OZ_BITS].h_low) && 
		(high_val <= pta_rc5[RC5_OZ_BITS].h_high)) && 

		(((total_val >= pta_rc5[RC5_OZ_BITS].t_low) && 
		  (total_val <= pta_rc5[RC5_OZ_BITS].t_high)) || 
		 
		 rc_dev->record.bit == 12)) { /* total_val equal 0, it means received both one and zero bits */

		PDEBUG("RC5_OZ_BITS: bit: %d, high_val: %d, total_val: %d\n", rc_dev->record.bit, high_val, total_val);
		ret = RC5_OZ_BITS;

	} else if((high_val >= pta_rc5[RC5_OZO_BITS].h_low) && 
		(high_val <= pta_rc5[RC5_OZO_BITS].h_high) && 
		(total_val >= pta_rc5[RC5_OZO_BITS].t_low) && 
		(total_val <= pta_rc5[RC5_OZO_BITS].t_high)) {

		PDEBUG("RC5_OZO_BITS: bit: %d, high_val: %d, total_val: %d\n", rc_dev->record.bit, high_val, total_val);
		ret = RC5_OZO_BITS;

	} else {

		PDEBUG("XXXXXXXXX_DROP: bit: %d, high_val: %d, total_val: %d\n", rc_dev->record.bit, high_val, total_val);
		ret = RC5_DROP;
	}

	return ret;		
}

static unsigned int orion_rc5_decode(int idx)
{
	int rc5_st;
	int toggle;

	PDEBUG("idx %2d ", idx);
	rc5_st = orion_rc5_decode_bit(rc_dev->fifo1[idx], rc_dev->fifo2[idx]);

	switch (rc5_st) {
		case RC5_ONE_BIT:
			rc_dev->record.rc5_data <<= 1;
			if (! rc_dev->record.next_bit_is_zero)
				rc_dev->record.rc5_data |= 1;
			rc_dev->record.bit += 1;
			break;
		case RC5_ZO_BITS:
			rc_dev->record.rc5_data <<= 1;
			//rc_dev->record.rc5_data |= 1;
			rc_dev->record.next_bit_is_zero = 0;
			rc_dev->record.bit += 1;
			break;
		case RC5_OZ_BITS:
			rc_dev->record.rc5_data <<= 2;
			rc_dev->record.rc5_data |= 2;
			rc_dev->record.next_bit_is_zero = 1;
			rc_dev->record.bit += 2;
			break;
		case RC5_OZO_BITS:
			rc_dev->record.rc5_data <<= 2;
			rc_dev->record.rc5_data |= 2;
			rc_dev->record.next_bit_is_zero = 0;
			rc_dev->record.bit += 2;
			break;
		default:	/* Error occurs */
			rc_dev->record.bit = 0;
			rc_dev->record.next_bit_is_zero = 0;
			rc_dev->bit_err = 1;
	}

	if (rc_dev->record.bit == 14) {	/* Ok, we received all bits */

		rc_dev->record.addr = (rc_dev->record.rc5_data & 0x07c0) >> 6;
		rc_dev->record.cmd = rc_dev->record.rc5_data & 0x003f;
		toggle = (rc_dev->record.rc5_data & 0x0800) ? 1: 0;

		PDEBUG ("bit : %d, rc5_data: 0x%4x, toggle: %x, addr %x, cmd %x\n", 
			rc_dev->record.bit, rc_dev->record.rc5_data, toggle, rc_dev->record.addr, rc_dev->record.cmd);

#if 0
		/* Handle repeat */
		if (rc_dev->record.pre_T == toggle && rc_dev->record.rpt++ != 0) {
			if (rc_dev->record.rpt < 3) {
				rc_dev->record.bit = 0;
				rc_dev->record.next_bit_is_zero = 0;
				return 0;
			} else {
				rc_dev->record.pre_T = toggle;
			}
		}
#endif

		rc_dev->record.rpt = 0;
		rc_dev->record.bit = 0;
		rc_dev->record.next_bit_is_zero = 0;
		return 1;
	}

	return 0;
}

#if 0
static unsigned int orion_rc6_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}

static unsigned int orion_rcmm_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}
static unsigned int orion_recs80_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}
static unsigned int orion_rca_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}
static unsigned int orion_xsat_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}

static unsigned int orion_jvc_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}

static unsigned int orion_nrc17_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}

static unsigned int orion_sharp_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}
static unsigned int orion_sirc_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}

static unsigned int orion_itt_decode(struct orion_rc *rc)
{
	unsigned int ret;
	PDEBUG("The protocol type %d is not a valid type\n",orion_rc_gbl->protocal);
	ret=0;
	return ret;
}
#endif

static void rc_do_tasklet(unsigned long data)
{
	unsigned long flags;
	int idx;
	int got_data = 0;
	

#ifdef CONFIG_RC_DEBUG	/* Print the fifo count */
	int i;
	for (i = 0; i < rc_dev->fc_idx; i++) 
		PDEBUG("  FIFO COUNT fifo1_cnt: %d, fifo2_cnt: %d\n", rc_dev->fc[i][0], rc_dev->fc[i][1]);
	spin_lock_irqsave(&rc_dev->lock, flags);
	rc_dev->fc_idx = 0;
	spin_unlock_irqrestore(&rc_dev->lock, flags);
	PDEBUG("***************  f1_idx: %d, f2_idx: %d ***************\n", rc_dev->f1_idx, rc_dev->f2_idx);
#endif

	for (idx = 0; idx < rc_dev->f2_idx || idx< rc_dev->f1_idx; idx++) {
		if (rc_dev->bit_err)
			break;

		switch (rc_dev->protocal) {
			case NEC:
				got_data = orion_nec_decode(idx);
				break;
			case RC5:
				got_data =orion_rc5_decode(idx);
				break;
#if 0
			case TYPE_RC6:
				decoder_status=orion_rc6_decode(rc_dev);
				break;
			case TYPE_RCMM:
				decoder_status=orion_rcmm_decode(rc_dev); 
				break;
			case TYPE_RECS80:
				decoder_status=orion_recs80_decode(rc_dev);
				break;
			case TYPE_RCA:
				decoder_status=orion_rca_decode(rc_dev);
				break;
			case TYPE_XSAT:
				decoder_status=orion_xsat_decode(rc_dev);
				break;
			case TYPE_JVC:
				decoder_status=orion_jvc_decode(rc_dev);
				break;
			case TYPE_NRC17:
				decoder_status=orion_nrc17_decode(rc_dev);
				break;
			case TYPE_SHARP:
				decoder_status=orion_sharp_decode(rc_dev);
				break;
			case TYPE_SIRC:
				decoder_status=orion_sirc_decode(rc_dev);
				break;
			case TYPE_ITT:
				decoder_status=orion_itt_decode(rc_dev);
				break;
#endif
			default:
				PDEBUG("The protocol type %d is not a valid type\n",rc_dev->protocal);
		}

		/* FIXME: this value should be determined outside */
		if(got_data) {
			PDEBUG(" \nYES YES YES Got it! bit: %x rpt: %x addr: %x, cmd: %x\n\n", 
			       rc_dev->record.bit, rc_dev->record.rpt, rc_dev->record.addr, rc_dev->record.cmd);

			if ((0xffffffff == rc_system_code) || 
			    (rc_dev->record.addr == rc_system_code)) {
				input_report_key(&rc_dev->dev, rc_dev->record.cmd, 1);
				input_report_key(&rc_dev->dev, rc_dev->record.cmd, 0);
				input_sync(&rc_dev->dev);
			}
		}
	}

	spin_lock_irqsave(&rc_dev->lock, flags);
	/* When data is handled, clear the fifo, and reset err state */
	rc_dev->bit_err = 0;
	rc_dev->f1_idx = rc_dev->f2_idx = 0;
#if 0
	memset(rc_dev->fifo1, 0, ARRAY_SIZE(rc_dev->fifo1));
	memset(rc_dev->fifo2, 0, ARRAY_SIZE(rc_dev->fifo2));
#endif
	spin_unlock_irqrestore(&rc_dev->lock, flags);
}

static irqreturn_t rc_dev_interrupt(int irq, void *dummy, struct pt_regs *fp)
{
	unsigned long  flags;
	int intr_status;
	int fifo1_cnt, fifo2_cnt;

	intr_status = rc_readw(RC_INTS_L);
	if ((intr_status & 0x1) || (intr_status & 0x4)) {	/* rc available | timeout */
		spin_lock_irqsave(&rc_dev->lock, flags);

		fifo1_cnt = rc_readw(RC_FIFO1_CNT_L) & 0xf;
		fifo2_cnt = rc_readw(RC_FIFO2_CNT_L) & 0xf;

#ifdef CONFIG_RC_DEBUG
		rc_dev->fc[rc_dev->fc_idx][0] = fifo1_cnt;
		rc_dev->fc[rc_dev->fc_idx][1] = fifo2_cnt;
		rc_dev->fc_idx += 1;
#endif

		while (fifo1_cnt--) { 	/* handle fifo1 data */
			rc_dev->fifo1[rc_dev->f1_idx++] = rc_readw(RC_FIFO1_L);
			if (rc_dev->f1_idx == FIFO_LEN) rc_dev->f1_idx = 0;
		}

		while (fifo2_cnt--) {	/* handle fifo2 data */
			rc_dev->fifo2[rc_dev->f2_idx++] = rc_readw(RC_FIFO2_L);
			if (rc_dev->f2_idx == FIFO_LEN) rc_dev->f2_idx = 0;
		}

		rc_dev->tasklet.data = (unsigned int)0;	/* FIXME: to be modified */
		tasklet_schedule(&rc_dev->tasklet);

		spin_unlock_irqrestore(&rc_dev->lock, flags);
	} else if (intr_status & 0x2) {
		static unsigned long ticks = 0;
		unsigned long curr_ticks = jiffies; 

		if (curr_ticks - ticks > 23) {
			rc_dev->record.cmd = rc_readw(RC_KSCAN_DATA_L) & 0xff;
			rc_dev->record.cmd |= 0x100;

			if (rc_dev->record.cmd & 0x80) {
				spin_lock_irqsave(&rc_dev->lock, flags);

				input_report_key(&rc_dev->dev, rc_dev->record.cmd, 1);
				input_report_key(&rc_dev->dev, rc_dev->record.cmd, 0);
				input_sync(&rc_dev->dev);

				spin_unlock_irqrestore(&rc_dev->lock, flags);
			}

			ticks = curr_ticks;
		}
		//PDEBUG("key scan!!\n");
	}

	return IRQ_HANDLED;
}

static int __proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned short addr;
	unsigned short val;

	const char *cmd_line = buffer;;

	if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = rc_readw(addr);
		PDEBUG(" readw [0x%04x] = 0x%04x \n", addr, val);
	} else if (strncmp("ww", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		rc_writew(addr, val);
	} else if (strncmp("st", cmd_line, 2) == 0) {	/* set time ajust value */
		int entry;
		int hl, hh, tl, th;
		entry = simple_strtol(&cmd_line[3], NULL, 10);

		hl = simple_strtol(&cmd_line[5], NULL, 10);
		hh = simple_strtol(&cmd_line[11], NULL, 10);
		tl = simple_strtol(&cmd_line[17], NULL, 10);
		th = simple_strtol(&cmd_line[23], NULL, 10);

		pta_nec[entry].h_low = hl; /* pta_nec: protocal time ajust */
		pta_nec[entry].h_high = hh;
		pta_nec[entry].t_low = tl;
		pta_nec[entry].t_high = th;
	} else if (strncmp("rt", cmd_line, 2) == 0) {	/* read time ajust value */
		int entry;
		entry = simple_strtol(&cmd_line[3], NULL, 10);
		printk (
		       "pta: pta[%d].h_low  = %d\n"
		       "pta: pta[%d].h_high = %d\n"
		       "pta: pta[%d].t_low  = %d\n" 
		       "pta: pta[%d].t_high = %d\n", 
		       entry, pta_nec[entry].h_low,
		       entry, pta_nec[entry].h_high,
		       entry, pta_nec[entry].t_low,
		       entry, pta_nec[entry].t_high
		       );
	} else if (strncmp("sp", cmd_line, 2) == 0) {	/* set remote controller protocals */
		int protocal;
		protocal = simple_strtol(&cmd_line[3], NULL, 10);
		rc_dev->protocal = protocal;
	} else if (strncmp("rpt", cmd_line, 2) == 0) {	/* set remote controller protocals */
		val = simple_strtol(&cmd_line[4], NULL, 10);
		rc_dev->max_rpt = val;
	} else if (strncmp("dbg", cmd_line, 3) == 0) {	/* set remote controller protocals */
		if (RC_DEBUG) 	RC_DEBUG = 0;
		else 		RC_DEBUG = 1;
	} else {
		printk(KERN_ERR "Illegal command\n");
	}

	return count;
}

static int __init orionrc_init(void)
{
	int i;
	int ret = 0;
	unsigned short dummy;

	PDEBUG("initialized!\n");

	if (request_irq(ORION_RC_IRQ, rc_dev_interrupt, 0, "orionrc", NULL)) {
		ret = -EBUSY;
		goto err1;
	}

	if(!(orionrc_base = (unsigned short *)ioremap(ORION_RC_BASE, ORION_RC_SIZE))) {
		ret =  -EIO;
		goto err2;
	}

	/* Proc handle */
	rc_proc_entry = create_proc_entry("rc_io", 0, NULL);
	if (NULL == rc_proc_entry) {
		ret =  -EFAULT;
		goto err3;
	} else {
		rc_proc_entry->write_proc = &__proc_write;
	}

	if (!(rc_dev = kmalloc(sizeof(orionrc_t), GFP_KERNEL))) {
		ret = -ENOMEM;
		goto err4;
	}

	memset(rc_dev, 0, sizeof(orionrc_t));

	rc_dev->dev.evbit[0] = BIT(EV_KEY) | BIT(EV_REP);

	init_input_dev(&rc_dev->dev);
	rc_dev->dev.keycode = rc_dev->keycode;
	rc_dev->dev.keycodesize = sizeof(unsigned short);
	rc_dev->dev.keycodemax = ARRAY_SIZE(rc_dev->keycode);
	rc_dev->dev.private = rc_dev;

	for (i = 0; i < ORION_RC_KNUM; i++)			/* Output RAW Code */
		rc_dev->keycode[i] = i;

	for (i = 0; i < ORION_RC_KNUM; i++)
		set_bit(rc_dev->keycode[i], rc_dev->dev.keybit);

	rc_dev->dev.name = DRIVER_DESC;
	rc_dev->dev.phys = "orionrc";
	rc_dev->dev.id.bustype = BUS_HOST;
	rc_dev->dev.id.vendor = 0x0001;
	rc_dev->dev.id.product = 0x0001;
	rc_dev->dev.id.version = 0x0104;

	input_register_device(&rc_dev->dev);

	/* default repeat is too fast */
	rc_dev->dev.rep[REP_DELAY]   = 500;
	rc_dev->dev.rep[REP_PERIOD]  = 100;
	rc_dev->max_rpt = 3;	/* real repeat every 2 repeat */

	/* init spin lock */
	spin_lock_init(rc_dev->lock);
	/* init tasklet */
	rc_dev->tasklet.func = rc_do_tasklet;

	{
		rc_dev->protocal = NEC;

		unsigned short fir_psn;
		unsigned int rc_ctl;

		/* Remote Controller config */
		rc_writew(RC_INTM_L, 0x0);	/* Intr mask */

		/* Noise filter */
		fir_psn = FILTER_PRECISION * 27 / PCLK_MHz;
		rc_writew(RC_FIR_L, fir_psn);	/* Filter precision (0xe0*55)/27 = 450us */
		rc_writew(RC_FIR_H, 0x0037);	/* Devider */

		/* Interrupt count */
		rc_writew(RC_INTR_CNT_L, 0x4); /* time_out << 4 | const_count */
		rc_writew(RC_INTR_CNT_H, 0x3a94); /* time out count */

		/* RC control */
		rc_ctl = (RC_CTL_OPTION<<29) | (BITTIME_CNT<<8) | (PANNEL_SELECT<<2);
		rc_writew(RC_CTL_L, (rc_ctl<<16)>>16);	/* bittime_cnt & pannel_select ; 0x372c */
		/* noise filter must be enabled, or else no interrupt */
		rc_writew(RC_CTL_H, rc_ctl>>16);	/* 00a, rc_en | rc_pol | noise_filter_en | bittime_cnt ; 0xe000 */

#if 0		
		/* temparary not set */
		rc_writew(RC_TIME_INTERVAL_L, 0x1); /* RC_TIME_INTERVAL */
#endif
		dummy = rc_readw(RC_FIFO1_L);
		PDEBUG("Throw out useless data: fifo1_cnt: %d, fifo2_cnt: %d , dummy: %d\n", 
		       rc_readw(RC_FIFO1_CNT_L), rc_readw(RC_FIFO2_CNT_L), dummy);

		/* KeyScan config */
		rc_writew(RC_KSCAN_CNTL_L, 0x0001);	/* enable keyscan */
		rc_writew(RC_KSCAN_CNTL_H, 0x000f);	/* key debound time: Pclk_period*{ (Key Scan Divider<<8) & 0xff} */
	}

	orion_gpio_register_module_status("RC", 1);
	orion_gpio_register_module_status("FPC", 1);

	goto out;

err5:
	kfree(rc_dev);
err4:
	remove_proc_entry("rc_io", NULL);
err3:
	iounmap((void *)orionrc_base);
err2:
	free_irq(ORION_RC_IRQ, NULL);
err1:

out:
	return ret;
}

static void __exit orionrc_exit(void)
{
	input_unregister_device(&rc_dev->dev);
	kfree(rc_dev);
	remove_proc_entry("rc_io", NULL);
	iounmap((void *)orionrc_base);
	free_irq(ORION_RC_IRQ, NULL);
}

module_init(orionrc_init);
module_exit(orionrc_exit);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
MODULE_AUTHOR("Sun He, <he.sun@celestialsemi.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual BSD/GPL");
#endif
