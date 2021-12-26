/*
 * linux/arch/arm/mach-virgo/irq.c
 *
 * by Ding Xiaoliang, 2005-8-23
 *
 * derived from linux/arch/ppc/kernel/i8259.c and:
 * include/asm-arm/arch-ebsa110/irq.h
 * Copyright (C) 1996-1998 Russell King
 */

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ptrace.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach/irq.h>

#include <asm/arch/hardware.h>

/*
 * This contains the irq mask for Designware VIC irq controller,
 * 32 IRQ sources
 */

/*
 * These have to be protected by the irq controller spinlock
 * before being called.
 */

void virgo_irq_enable(unsigned int irq)
{
	volatile unsigned int *irq_inten_l=(unsigned int *)(VA_VIC_BASE+VIC_IRQ_ENABLE);
	*irq_inten_l|=(1<<irq);
}

void virgo_irq_disable(unsigned int irq)
{
	volatile unsigned int *irq_inten_l=(unsigned int *)(VA_VIC_BASE+VIC_IRQ_ENABLE);
	*irq_inten_l&=~(1<<irq);
}
void virgo_irq_ack(unsigned int irq)
{	
	volatile unsigned int *irq_inten_l=(unsigned int *)(VA_VIC_BASE+VIC_IRQ_ENABLE);
	*irq_inten_l&=~(1<<irq);
}

#if 0
static irqreturn_t bogus_int(int irq, void *dev_id, struct pt_regs *regs)
{
	printk("Got interrupt %i!\n",irq);
	return IRQ_NONE;
}
#endif 

static struct irqchip vic_chip = {
	.ack	= virgo_irq_ack,
	.mask	= virgo_irq_disable,
	.unmask = virgo_irq_enable,
};

void __init virgo_init_irq(void)
{
        unsigned int i;

        writel(0, VA_VIC_BASE + VIC_IRQ_ENABLE);
        writel(0, VA_VIC_BASE + VIC_IRQ_MASK);
	
        for (i = IRQ_VIC_START; i <= IRQ_VIC_END; i++) {
			//    printk("irq %d \n",i);
                set_irq_chip(i, &vic_chip);
                set_irq_handler(i, do_level_IRQ);
                set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
        }
}

