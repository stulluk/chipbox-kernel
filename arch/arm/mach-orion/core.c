/*
 *  Architecture specific stuff.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/serial_8250.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/leds.h>
#include <asm/param.h>

#include <asm/mach/map.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <asm/arch/hardware.h>

#if defined(CONFIG_ARCH_ORION_CSM1201)
extern int __df_mem_initialize();
#endif

static int __init virgo_init(void)
{
	int ret=0;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	__df_mem_initialize();
#endif
	return ret;
}

arch_initcall(virgo_init);

extern void virgo_init_irq(void);

static struct map_desc virgo_io_desc[] __initdata = {
	{ VA_VIC_BASE , 	PA_VIC_BASE	, SZ_64K , MT_DEVICE },
	{ VA_UARTS_BASE, 	PA_UARTS_BASE	, SZ_16K , MT_DEVICE },
	{ VA_TIMERS_BASE, 	PA_TIMERS_BASE	, SZ_64K , MT_DEVICE },
	{ VA_ETH_BASE,		PA_ETH_BASE	, SZ_64K , MT_DEVICE },
	{ VA_ATA_BASE, 		PA_ATA_BASE	, SZ_16K , MT_DEVICE },
	{ VA_PCMCIA_BASE, 	PA_PCMCIA_BASE	, SZ_16K , MT_DEVICE }
	
};	// Data from Virgo MemoryMap 

static void __init virgo_map_io(void)
{
	iotable_init(virgo_io_desc, ARRAY_SIZE(virgo_io_desc));
}

#define IRQ_TIMER 4	// IRQ for Timer 0 and 1 in Virgo platform

#define	TIMER_CLK_FREQ	PCLK_FREQ
#define TICK_FREQ	HZ
#define VA_TIMER0_BASE	IO_ADDRESS(0x101e2000)	/* Timer 0*/
#define TIMER_LOADCOUNT	0
#define TIMER_CONTROL	8
#define TIMER_EOI	0x0c

static irqreturn_t
virgo_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	write_seqlock(&xtime_lock);
	readl(VA_TIMER0_BASE+TIMER_EOI);
	timer_tick(regs);
	write_sequnlock(&xtime_lock);
	return IRQ_HANDLED;
}

static struct irqaction virgo_timer_irq = {
	.name		= "ORION Timer",
	.flags		= SA_INTERRUPT,
	.handler	= virgo_timer_interrupt
};

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static void __init virgo_timer_init(void)
{

	/* timer counting speed is the pclk in Virgo */
	unsigned int timercount=TIMER_CLK_FREQ/TICK_FREQ - 1;
	writel(0, VA_TIMER0_BASE+TIMER_CONTROL);	/* disable it first */
//	printk("Virgo: timer count:%08x\n",timercount);
	/* in virgo, APB devices can only be accessed in halfword or byte mode*/
	writew(timercount&0xffff, VA_TIMER0_BASE+TIMER_LOADCOUNT);
	writew(timercount>>16,	VA_TIMER0_BASE+TIMER_LOADCOUNT+2);	
	writel(3, VA_TIMER0_BASE+TIMER_CONTROL);	/* enable timer itself*/
	setup_irq(IRQ_TIMER, &virgo_timer_irq);
}

static struct sys_timer virgo_timer = {
	.init		= virgo_timer_init,
};

MACHINE_START(VIRGO, "ORION")
	MAINTAINER("Xiaodong")
	BOOT_MEM(0x00000000, 0x101f1000,0xf11f1000)
	BOOT_PARAMS(0x00000100)
	MAPIO(virgo_map_io)
	INITIRQ(virgo_init_irq)
	.timer		= &virgo_timer,
MACHINE_END
