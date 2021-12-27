# Diff between CelestialSemi BSP kernel and Mainline 2.6.12.5 kernel

CelestialSemi provided a 2.6.12.5 kernel to us.

To port new kernels to Chipbox, Arnd suggested to check a diff
between mainline 2.6.12.5 and BSP 2.6.12.5

To create this, I used this command:

```
diff -ur mainline-2.6.12.5/linux-2.6.12.5/ chipbox-kernel/ > Chipbox.patch
```

Here is the diff:

```diff

Only in chipbox-kernel/arch/arm/boot: Image.gz
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/boot/Makefile chipbox-kernel/arch/arm/boot/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/boot/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/boot/Makefile	2021-12-27 13:08:34.856996921 +0300
@@ -64,6 +64,7 @@
 $(obj)/uImage:	$(obj)/zImage FORCE
 	$(call if_changed,uimage)
 	@echo '  Image $@ is ready'
+	@if [ -n "$(TFTPT)" ]; then cp -v $@ $(TFTPT)/$(shell basename $@).$(shell basename $(O)); fi
 
 $(obj)/bootp/bootp: $(obj)/zImage initrd FORCE
 	$(Q)$(MAKE) $(build)=$(obj)/bootp $@
Only in chipbox-kernel/arch/arm/boot: uImage_gaian
Only in chipbox-kernel/arch/arm/boot: uImage_gold
Only in chipbox-kernel/arch/arm/boot: uImage_p
Only in chipbox-kernel/arch/arm/configs: csm1200j_norboot_flash_64M_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1200_nandboot_flash_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1200_norboot_flash_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1200_norboot_hdplayer_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1200_norboot_ide_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201j_norboot_flash_64M_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_flash_256M_32Mflashmode_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_flash_256M_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_flash_32Mflashmode_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_flash_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_hdplayer_defconfig
Only in chipbox-kernel/arch/arm/configs: csm1201_norboot_ide_128M_defconfig
Only in chipbox-kernel/arch/arm/configs: log
Only in chipbox-kernel/arch/arm/configs: .svn
Only in chipbox-kernel/arch/arm/configs: virgo_defconfig
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/Kconfig chipbox-kernel/arch/arm/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/Kconfig	2021-12-27 13:08:34.856996921 +0300
@@ -194,6 +194,13 @@
 	help
 	  This enables support for ARM Ltd Versatile board.
 
+config ARCH_CELESTIAL_ORION
+	bool "Celestial Semiconductor, ORION Series"
+	help
+	  This enables support for ORION Series SoC from Celestial
+	  Semiconductor.
+
+
 config ARCH_IMX
 	bool "IMX"
 
@@ -234,6 +241,99 @@
 
 source "arch/arm/mach-versatile/Kconfig"
 
+source "arch/arm/mach-orion/Kconfig"
+
+config AVRES_ALLOC_SUPPORT
+	tristate "A/V resources allocation support"
+          
+config BASE_DDR_ADDR
+	hex "Default start address for A/V resources" if AVRES_ALLOC_SUPPORT
+        default "0x08000000"
+        help 
+          The default value is 0x08000000. the value depends on the implementation 
+          of your hardware. Exactly, you should know physical memory size of your 
+          hardware configured.
+
+config FB0_SIZE
+	hex "Default buffer size of Frame Buffer 0" if AVRES_ALLOC_SUPPORT
+	default "0x200000"
+
+config FB0_REGION
+	string
+	default "(BASE_DDR_ADDR - FB0_SIZE)"
+
+config FB1_SIZE
+	hex "Default buffer size of Frame Buffer 1" if AVRES_ALLOC_SUPPORT
+	default "0x200000"
+
+config FB1_REGION
+	string
+	default "(FB0_REGION - FB1_SIZE)"
+
+config FB2_SIZE
+	hex "Default buffer size of Frame Buffer 2 (Part of FB0)" if AVRES_ALLOC_SUPPORT
+	default "0x100000"
+
+config FB2_REGION
+	string
+	default "(FB0_REGION + FB0_SIZE - FB2_SIZE)"
+
+config FB3_SIZE
+	hex "Default buffer size of Frame Buffer 3 (Part of FB1)" if AVRES_ALLOC_SUPPORT
+	default "0x100000"
+
+config FB3_REGION
+	string
+	default "(FB1_REGION + FB1_SIZE - FB3_SIZE)"
+
+config ETHERNET_SIZE
+	hex "Default buffer size for Ethernet controller" if AVRES_ALLOC_SUPPORT
+	default "0x100000"
+
+config ETHERNET_REGION
+	string
+	default "(FB1_REGION - ETHERNET_SIZE)"
+
+config XPORT_SIZE
+	hex "Default buffer size for Xport device" if AVRES_ALLOC_SUPPORT
+	default "0x400000"
+
+config XPORT_REGION
+	string
+	default "(ETHERNET_REGION - XPORT_SIZE)"
+
+config AUD_SIZE
+	hex "Default buffer size for Audio device" if AVRES_ALLOC_SUPPORT
+	default "0x100000"
+
+config AUD_REGION
+	string
+	default "(XPORT_REGION - AUD_SIZE)"
+
+config DPB0_SIZE
+	hex "Default DPB buffer size for Video codec" if AVRES_ALLOC_SUPPORT
+	default "0x1a00000"
+
+config DPB0_REGION
+	string
+	default "(AUD_REGION - DPB0_SIZE)"
+
+config CPB0_SIZE
+	hex "Default CPB buffer size for Video codec" if AVRES_ALLOC_SUPPORT
+	default "0x400000"
+
+config CPB0_REGION
+	string
+	default "(DPB0_REGION - CPB0_SIZE)"
+
+config CPB0_DIR_SIZE
+	hex "Defaut CPB0 DIR buffer for Video codec" if AVRES_ALLOC_SUPPORT
+	default "0x1000"
+
+config CPB0_DIR_REGION
+	string
+	default "(CPB0_REGION - CPB0_DIR_SIZE)"
+
 # Definitions to make life easier
 config ARCH_ACORN
 	bool
@@ -691,7 +791,8 @@
 
 if PCMCIA || ARCH_CLPS7500 || ARCH_IOP3XX || ARCH_IXP4XX \
 	|| ARCH_L7200 || ARCH_LH7A40X || ARCH_PXA || ARCH_RPC \
-	|| ARCH_S3C2410 || ARCH_SA1100 || ARCH_SHARK || FOOTBRIDGE
+	|| ARCH_S3C2410 || ARCH_SA1100 || ARCH_SHARK || FOOTBRIDGE \
+	|| ARCH_CELESTIAL_ORION
 source "drivers/ide/Kconfig"
 endif
 
@@ -717,6 +818,8 @@
 
 source "drivers/i2c/Kconfig"
 
+#source "drivers/spi/Kconfig"
+
 #source "drivers/l3/Kconfig"
 
 source "drivers/misc/Kconfig"
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/head.S chipbox-kernel/arch/arm/kernel/head.S
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/head.S	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/kernel/head.S	2021-12-27 13:08:34.892997470 +0300
@@ -166,7 +166,6 @@
 	b	start_kernel
 
 
-
 /*
  * Setup common bits before finally enabling the MMU.  Essentially
  * this is just loading the page table pointer and domain access
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/irq.c chipbox-kernel/arch/arm/kernel/irq.c
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/irq.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/kernel/irq.c	2021-12-27 13:08:34.892997470 +0300
@@ -126,6 +126,9 @@
 
 	spin_lock_irqsave(&irq_controller_lock, flags);
 	desc->disable_depth++;
+	/*Celestial DXL: no actual IRQ disable code here, add one*/
+	desc->chip->mask(irq);
+
 	list_del_init(&desc->pend);
 	spin_unlock_irqrestore(&irq_controller_lock, flags);
 }
@@ -327,13 +330,20 @@
 	unsigned int status;
 	int ret, retval = 0;
 
+   // if (irq==29) printk("in __do_irq\n");
+	
 	spin_unlock(&irq_controller_lock);
-
+	
+   // if (irq==29) printk("in __do_irq_1\n");
+	
 	if (!(action->flags & SA_INTERRUPT))
 		local_irq_enable();
-
+  //  if (irq==29) printk("in __do_irq___2\n");
 	status = 0;
 	do {
+//		 if (irq==29) printk("in __do_irq_____3\n");
+        
+//		 if (irq==29) printk("in __do_irq_____3\n");
 		ret = action->handler(irq, action->dev_id, regs);
 		if (ret == IRQ_HANDLED)
 			status |= action->flags;
@@ -464,9 +474,15 @@
 		 * Return with this interrupt masked if no action
 		 */
 		action = desc->action;
+ /* Virgo
+ 	if(irq==29){
+			printk("IRQ %d, action=%08x\t",irq, desc->action);
+			if(action)
+				printk("handler=%08x\n",action->handler);
+		}
+*/
 		if (action) {
 			int ret = __do_irq(irq, desc->action, regs);
-
 			if (ret != IRQ_HANDLED)
 				report_bad_irq(irq, regs, desc, ret);
 
@@ -695,7 +711,6 @@
 			desc->chip->unmask(irq);
 		}
 	}
-
 	spin_unlock_irqrestore(&irq_controller_lock, flags);
 	return 0;
 }
@@ -736,7 +751,7 @@
 {
 	unsigned long retval;
 	struct irqaction *action;
-
+	
 	if (irq >= NR_IRQS || !irq_desc[irq].valid || !handler ||
 	    (irq_flags & SA_SHIRQ && !dev_id))
 		return -EINVAL;
@@ -750,10 +765,8 @@
 	cpus_clear(action->mask);
 	action->name = devname;
 	action->next = NULL;
-	action->dev_id = dev_id;
-
+	action->dev_id = dev_id;  
 	retval = setup_irq(irq, action);
-
 	if (retval)
 		kfree(action);
 	return retval;
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/setup.c chipbox-kernel/arch/arm/kernel/setup.c
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/kernel/setup.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/kernel/setup.c	2021-12-27 13:08:34.892997470 +0300
@@ -95,7 +95,7 @@
 char elf_platform[ELF_PLATFORM_SIZE];
 EXPORT_SYMBOL(elf_platform);
 
-unsigned long phys_initrd_start __initdata = 0;
+unsigned long phys_initrd_start __initdata = 0;	
 unsigned long phys_initrd_size __initdata = 0;
 
 static struct meminfo meminfo __initdata = { 0, };
@@ -703,7 +703,6 @@
 			squash_mem_tags(tags);
 		parse_tags(tags);
 	}
-
 	init_mm.start_code = (unsigned long) &_text;
 	init_mm.end_code   = (unsigned long) &_etext;
 	init_mm.end_data   = (unsigned long) &_edata;
Only in chipbox-kernel/arch/arm: mach-orion
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mach-versatile/core.c chipbox-kernel/arch/arm/mach-versatile/core.c
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mach-versatile/core.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/mach-versatile/core.c	2021-12-27 13:08:34.936998139 +0300
@@ -916,3 +916,4 @@
 	.init		= versatile_timer_init,
 	.offset		= versatile_gettimeoffset,
 };
+
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/Makefile chipbox-kernel/arch/arm/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/Makefile	2021-12-27 13:08:34.856996921 +0300
@@ -95,6 +95,7 @@
  machine-$(CONFIG_ARCH_S3C2410)	   := s3c2410
  machine-$(CONFIG_ARCH_LH7A40X)	   := lh7a40x
  machine-$(CONFIG_ARCH_VERSATILE)  := versatile
+ machine-$(CONFIG_ARCH_CELESTIAL_ORION) :=orion
  machine-$(CONFIG_ARCH_IMX)	   := imx
  machine-$(CONFIG_ARCH_H720X)	   := h720x
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mm/init.c chipbox-kernel/arch/arm/mm/init.c
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mm/init.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/mm/init.c	2021-12-27 13:08:34.940998197 +0300
@@ -260,7 +260,7 @@
 	}
 
 	if (initrd_node == -1) {
-		printk(KERN_ERR "initrd (0x%08lx - 0x%08lx) extends beyond "
+  		printk(KERN_ERR "initrd (0x%08lx - 0x%08lx) extends beyond "
 		       "physical memory - disabling initrd\n",
 		       phys_initrd_start, end);
 		phys_initrd_start = phys_initrd_size = 0;
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mm/Kconfig chipbox-kernel/arch/arm/mm/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/mm/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/mm/Kconfig	2021-12-27 13:08:34.936998139 +0300
@@ -121,8 +121,8 @@
 # ARM926T
 config CPU_ARM926T
 	bool "Support ARM926T processor" if ARCH_INTEGRATOR
-	depends on ARCH_INTEGRATOR || ARCH_VERSATILE_PB || MACH_VERSATILE_AB || ARCH_OMAP730 || ARCH_OMAP16XX
-	default y if ARCH_VERSATILE_PB || MACH_VERSATILE_AB || ARCH_OMAP730 || ARCH_OMAP16XX
+	depends on ARCH_INTEGRATOR || ARCH_VERSATILE_PB || MACH_VERSATILE_AB || ARCH_OMAP730 || ARCH_OMAP16XX || ARCH_CELESTIAL_ORION
+	default y if ARCH_VERSATILE_PB || MACH_VERSATILE_AB || ARCH_OMAP730 || ARCH_OMAP16XX || ARCH_CELESTIAL_ORION
 	select CPU_32v5
 	select CPU_ABRT_EV5TJ
 	select CPU_CACHE_VIVT
diff -ur mainline-2.6.12.5/linux-2.6.12.5/arch/arm/tools/mach-types chipbox-kernel/arch/arm/tools/mach-types
--- mainline-2.6.12.5/linux-2.6.12.5/arch/arm/tools/mach-types	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/arch/arm/tools/mach-types	2021-12-27 13:08:34.956998443 +0300
@@ -724,3 +724,4 @@
 omap_comet3		MACH_COMET3		COMET3			716
 omap_comet4		MACH_COMET4		COMET4			717
 csb625			MACH_CSB625		CSB625			718
+virgo			MACH_CELESTIAL_VIRGO	VIRGO			719
Only in chipbox-kernel/: b
Only in chipbox-kernel/: ba
Only in chipbox-kernel/: ca
Only in chipbox-kernel/: .config
Only in chipbox-kernel/: .config.cmd
Only in chipbox-kernel/: .config.old
Only in chipbox-kernel/drivers: built-in.result
Only in chipbox-kernel/drivers/char: audio
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/char/Kconfig chipbox-kernel/drivers/char/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/char/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/char/Kconfig	2021-12-27 13:08:35.521007008 +0300
@@ -413,6 +413,38 @@
          If you have an SGI Altix with an attached SABrick
          say Y or M here, otherwise say N.
 
+config ORION_FPC
+       tristate "ORION platform Frontend Led driver"
+
+config ORION_GPIO
+       tristate "ORION platform GPIO driver"
+ 
+config ORION_GPIO2
+       tristate "ORION platform GPIO2 driver"
+
+config ORION_SCI
+       tristate "ORION platform Smartcard Interface driver"
+
+config ORION_SCI_DEBUG
+	bool "Debugging"
+	depends on ORION_SCI
+	help
+	  This turns on low-level debugging for the ORION Smart Card driver.
+	  Normally, you should say 'N'.
+
+
+config SCI_DEBUG_VERBOSE
+	int "Debugging verbosity (0 = quiet, 3 = noisy)"
+	depends on ORION_SCI_DEBUG
+	default "0"
+	help
+	  Determines the verbosity level of the SCI debugging messages.
+
+config CS_SERIAL_RC
+       tristate "ORION platform UART 1 Driver for UART based Remote Controller"           
+
+
+
 source "drivers/serial/Kconfig"
 
 config UNIX98_PTYS
@@ -997,6 +1029,12 @@
 	  Altix system timer.
 
 source "drivers/char/tpm/Kconfig"
+source "drivers/char/xport/Kconfig"
+source "drivers/char/video/Kconfig"
+source "drivers/char/audio/Kconfig"
+source "drivers/char/orion_spi/Kconfig"
+source "drivers/char/orion_i2c/Kconfig"
+source "drivers/char/orion_df/Kconfig"
 
 endmenu
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/char/Makefile chipbox-kernel/drivers/char/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/char/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/char/Makefile	2021-12-27 13:08:35.521007008 +0300
@@ -82,6 +82,13 @@
 obj-$(CONFIG_SCx200_GPIO) += scx200_gpio.o
 obj-$(CONFIG_TANBAC_TB0219) += tb0219.o
 
+obj-$(CONFIG_ORION_FPC) += orion_fpc.o
+obj-$(CONFIG_ORION_SEC) += orion_sec.o
+obj-$(CONFIG_ORION_GPIO) += orion_gpio.o
+obj-$(CONFIG_ORION_GPIO2) += orion_gpio2.o
+obj-$(CONFIG_ORION_SCI) += orion_sci.o
+obj-$(CONFIG_CS_SERIAL_RC) +=serialport.o
+
 obj-$(CONFIG_WATCHDOG)	+= watchdog/
 obj-$(CONFIG_MWAVE) += mwave/
 obj-$(CONFIG_AGP) += agp/
@@ -91,6 +98,13 @@
 
 obj-$(CONFIG_HANGCHECK_TIMER) += hangcheck-timer.o
 obj-$(CONFIG_TCG_TPM) += tpm/
+obj-$(CONFIG_XPORT_SUPPORT) += xport/
+obj-$(CONFIG_VIDEO_SUPPORT) += video/
+obj-$(CONFIG_AUDIO_SUPPORT) += audio/
+obj-$(CONFIG_ORION_SPI) += orion_spi/
+obj-$(CONFIG_ORION_I2C) += orion_i2c/
+obj-$(CONFIG_ORION_DF) += orion_df/
+
 # Files generated that shall be removed upon make clean
 clean-files := consolemap_deftbl.c defkeymap.c qtronixmap.c
 
Only in chipbox-kernel/drivers/char: orion_df
Only in chipbox-kernel/drivers/char: orion_fpc.c
Only in chipbox-kernel/drivers/char: orion_gpio2.c
Only in chipbox-kernel/drivers/char: orion_gpio.c
Only in chipbox-kernel/drivers/char: orion_i2c
Only in chipbox-kernel/drivers/char: orion_sci.c
Only in chipbox-kernel/drivers/char: orion_sec.c
Only in chipbox-kernel/drivers/char: orion_spi
Only in chipbox-kernel/drivers/char: serialport.c
Only in chipbox-kernel/drivers/char: serialport.h
Only in chipbox-kernel/drivers/char: video
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/char/watchdog/Kconfig chipbox-kernel/drivers/char/watchdog/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/char/watchdog/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/char/watchdog/Kconfig	2021-12-27 13:08:35.601008221 +0300
@@ -60,6 +60,13 @@
 
 # ARM Architecture
 
+config ORION_WDT
+	tristate "Orion watchdog"
+	depends on WATCHDOG
+	help
+	  This component is an AMBA 2.0-compliant Advanced Peripheral Bus (APB) 
+	  slave device and is part of the DesignWare AMBA On-chip Bus family.
+
 config 21285_WATCHDOG
 	tristate "DC21285 watchdog"
 	depends on WATCHDOG && FOOTBRIDGE
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/char/watchdog/Makefile chipbox-kernel/drivers/char/watchdog/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/char/watchdog/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/char/watchdog/Makefile	2021-12-27 13:08:35.601008221 +0300
@@ -34,6 +34,8 @@
 obj-$(CONFIG_IXP2000_WATCHDOG) += ixp2000_wdt.o
 obj-$(CONFIG_8xx_WDT) += mpc8xx_wdt.o
 
+obj-$(CONFIG_ORION_WDT) += orion_wdt.o
+
 # Only one watchdog can succeed. We probe the hardware watchdog
 # drivers first, then the softdog driver.  This means if your hardware
 # watchdog dies or is 'borrowed' for some reason the software watchdog
Only in chipbox-kernel/drivers/char/watchdog: orion_wdt.c
Only in chipbox-kernel/drivers/char: xport
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/ide.c chipbox-kernel/drivers/ide/ide.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/ide.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/ide/ide.c	2021-12-27 13:08:35.637008770 +0300
@@ -1855,6 +1855,9 @@
 #ifdef CONFIG_H8300
 	h8300_ide_init();
 #endif
+#ifdef CONFIG_BLK_DEV_ORION
+	palm_bk3710_init();
+#endif
 }
 
 void ide_register_subdriver(ide_drive_t *drive, ide_driver_t *driver)
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/Kconfig chipbox-kernel/drivers/ide/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/ide/Kconfig	2021-12-27 13:08:35.637008770 +0300
@@ -276,10 +276,16 @@
 
 config IDE_GENERIC
 	tristate "generic/default IDE chipset support"
-	default y
+	default n
 	help
 	  If unsure, say Y.
 
+config BLK_DEV_ORION
+	tristate "IDE Controller driver on Orion 1.4"
+	select BLK_DEV_IDEDMA
+	select BLK_DEV_IDEDMA_PCI
+	default y
+
 config BLK_DEV_CMD640
 	bool "CMD640 chipset bugfix/support"
 	depends on X86
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/Makefile chipbox-kernel/drivers/ide/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/ide/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/ide/Makefile	2021-12-27 13:08:35.637008770 +0300
@@ -43,6 +43,7 @@
 
 obj-$(CONFIG_BLK_DEV_IDE)		+= ide-core.o
 obj-$(CONFIG_IDE_GENERIC)		+= ide-generic.o
+obj-$(CONFIG_BLK_DEV_ORION)		+= palm_bk3710.o
 
 obj-$(CONFIG_BLK_DEV_IDEDISK)		+= ide-disk.o
 obj-$(CONFIG_BLK_DEV_IDECD)		+= ide-cd.o
Only in chipbox-kernel/drivers/ide: palm_bk3710.c
Only in chipbox-kernel/drivers/ide: palm_bk3710.h
Only in chipbox-kernel/drivers/input/keyboard: dmesg.log
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/input/keyboard/Kconfig chipbox-kernel/drivers/input/keyboard/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/input/keyboard/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/input/keyboard/Kconfig	2021-12-27 13:08:35.665009195 +0300
@@ -112,6 +112,37 @@
 	  To compile this driver as a module, choose M here: the
 	  module will be called xtkbd.
 
+config KEYBOARD_ORIONRC
+	tristate "ORION Remote Controller"
+	depends on ARCH_ORION_CSM1200
+	help
+	  Say Y here if you want to use the remote controller on ORION 1200 platform.
+
+	  To compile this driver as a module, choose M here: the
+	  module will be called orionrc.
+	  
+if KEYBOARD_ORIONRC
+
+config	ORIONRC_RC5
+	bool "Choose Philips RC5 Remote Controller"
+
+endif
+
+config KEYBOARD_ORIONRC_1201
+	tristate "ORION Remote Controller for CSM1201"
+	depends on ARCH_ORION_CSM1201
+	help
+	  Say Y here if you want to use the remote controller on ORION 1201 platform.
+
+	  To compile this driver as a module, choose M here: the
+	  module will be called orionrc.
+
+config RC_DEBUG
+	bool "ORIONRC DEBUG"
+	depends on KEYBOARD_ORIONRC_1201
+	help
+	  Say Y here to add debug message.
+
 config KEYBOARD_NEWTON
 	tristate "Newton keyboard"
 	select SERIO
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/input/keyboard/Makefile chipbox-kernel/drivers/input/keyboard/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/input/keyboard/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/input/keyboard/Makefile	2021-12-27 13:08:35.665009195 +0300
@@ -9,6 +9,8 @@
 obj-$(CONFIG_KEYBOARD_SUNKBD)		+= sunkbd.o
 obj-$(CONFIG_KEYBOARD_LKKBD)		+= lkkbd.o
 obj-$(CONFIG_KEYBOARD_XTKBD)		+= xtkbd.o
+obj-$(CONFIG_KEYBOARD_ORIONRC)		+= orionrc.o
+obj-$(CONFIG_KEYBOARD_ORIONRC_1201)	+= orionrc_1201.o
 obj-$(CONFIG_KEYBOARD_AMIGA)		+= amikbd.o
 obj-$(CONFIG_KEYBOARD_LOCOMO)		+= locomokbd.o
 obj-$(CONFIG_KEYBOARD_NEWTON)		+= newtonkbd.o
Only in chipbox-kernel/drivers/input/keyboard: orionrc_1201.c
Only in chipbox-kernel/drivers/input/keyboard: orionrc_1201.h
Only in chipbox-kernel/drivers/input/keyboard: orionrc.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/cfi_cmdset_0002.c chipbox-kernel/drivers/mtd/chips/cfi_cmdset_0002.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/cfi_cmdset_0002.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/chips/cfi_cmdset_0002.c	2021-12-27 13:08:35.781010954 +0300
@@ -35,6 +35,10 @@
 #include <linux/mtd/mtd.h>
 #include <linux/mtd/cfi.h>
 
+/* mutex for pcmcia and nor flash */
+DECLARE_MUTEX(ebi_mutex_lock);
+EXPORT_SYMBOL(ebi_mutex_lock);
+
 #define AMD_BOOTLOC_BUG
 #define FORCE_WORD_WRITE 0
 
@@ -88,7 +92,7 @@
 
 	printk("  Silicon revision: %d\n", extp->SiliconRevision >> 1);
 	printk("  Address sensitive unlock: %s\n", 
-	       (extp->SiliconRevision & 1) ? "Not required" : "Required");
+			(extp->SiliconRevision & 1) ? "Not required" : "Required");
 
 	if (extp->EraseSuspend < ARRAY_SIZE(erase_suspend))
 		printk("  Erase Suspend: %s\n", erase_suspend[extp->EraseSuspend]);
@@ -102,20 +106,20 @@
 
 
 	printk("  Temporary block unprotect: %s\n",
-	       extp->TmpBlkUnprotect ? "Supported" : "Not supported");
+			extp->TmpBlkUnprotect ? "Supported" : "Not supported");
 	printk("  Block protect/unprotect scheme: %d\n", extp->BlkProtUnprot);
 	printk("  Number of simultaneous operations: %d\n", extp->SimultaneousOps);
 	printk("  Burst mode: %s\n",
-	       extp->BurstMode ? "Supported" : "Not supported");
+			extp->BurstMode ? "Supported" : "Not supported");
 	if (extp->PageMode == 0)
 		printk("  Page mode: Not supported\n");
 	else
 		printk("  Page mode: %d word page\n", extp->PageMode << 2);
 
 	printk("  Vpp Supply Minimum Program/Erase Voltage: %d.%d V\n", 
-	       extp->VppMin >> 4, extp->VppMin & 0xf);
+			extp->VppMin >> 4, extp->VppMin & 0xf);
 	printk("  Vpp Supply Maximum Program/Erase Voltage: %d.%d V\n", 
-	       extp->VppMax >> 4, extp->VppMax & 0xf);
+			extp->VppMax >> 4, extp->VppMax & 0xf);
 
 	if (extp->TopBottom < ARRAY_SIZE(top_bottom))
 		printk("  Top/Bottom Boot Block: %s\n", top_bottom[extp->TopBottom]);
@@ -168,10 +172,10 @@
 	struct map_info *map = mtd->priv;
 	struct cfi_private *cfi = map->fldrv_priv;
 	if ((cfi->cfiq->NumEraseRegions == 1) &&
-		((cfi->cfiq->EraseRegionInfo[0] & 0xffff) == 0)) {
+			((cfi->cfiq->EraseRegionInfo[0] & 0xffff) == 0)) {
 		mtd->erase = cfi_amdstd_erase_chip;
 	}
-	
+
 }
 
 static struct cfi_fixup cfi_fixup_table[] = {
@@ -260,17 +264,17 @@
 		bootloc = extp->TopBottom;
 		if ((bootloc != 2) && (bootloc != 3)) {
 			printk(KERN_WARNING "%s: CFI does not contain boot "
-			       "bank location. Assuming top.\n", map->name);
+					"bank location. Assuming top.\n", map->name);
 			bootloc = 2;
 		}
 
 		if (bootloc == 3 && cfi->cfiq->NumEraseRegions > 1) {
 			printk(KERN_WARNING "%s: Swapping erase regions for broken CFI table.\n", map->name);
-			
+
 			for (i=0; i<cfi->cfiq->NumEraseRegions / 2; i++) {
 				int j = (cfi->cfiq->NumEraseRegions-1)-i;
 				__u32 swap;
-				
+
 				swap = cfi->cfiq->EraseRegionInfo[i];
 				cfi->cfiq->EraseRegionInfo[i] = cfi->cfiq->EraseRegionInfo[j];
 				cfi->cfiq->EraseRegionInfo[j] = swap;
@@ -281,11 +285,11 @@
 		cfi->addr_unlock2 = 0x2aa;
 		/* Modify the unlock address if we are in compatibility mode */
 		if (	/* x16 in x8 mode */
-			((cfi->device_type == CFI_DEVICETYPE_X8) && 
-				(cfi->cfiq->InterfaceDesc == 2)) ||
-			/* x32 in x16 mode */
-			((cfi->device_type == CFI_DEVICETYPE_X16) &&
-				(cfi->cfiq->InterfaceDesc == 4))) 
+				((cfi->device_type == CFI_DEVICETYPE_X8) && 
+				 (cfi->cfiq->InterfaceDesc == 2)) ||
+				/* x32 in x16 mode */
+				((cfi->device_type == CFI_DEVICETYPE_X16) &&
+				 (cfi->cfiq->InterfaceDesc == 4))) 
 		{
 			cfi->addr_unlock1 = 0xaaa;
 			cfi->addr_unlock2 = 0x555;
@@ -304,9 +308,9 @@
 		cfi->chips[i].buffer_write_time = 1<<cfi->cfiq->BufWriteTimeoutTyp;
 		cfi->chips[i].erase_time = 1<<cfi->cfiq->BlockEraseTimeoutTyp;
 	}		
-	
+
 	map->fldrv = &cfi_amdstd_chipdrv;
-	
+
 	return cfi_amdstd_setup(mtd);
 }
 
@@ -320,23 +324,23 @@
 	int i,j;
 
 	printk(KERN_NOTICE "number of %s chips: %d\n", 
-	       (cfi->cfi_mode == CFI_MODE_CFI)?"CFI":"JEDEC",cfi->numchips);
+			(cfi->cfi_mode == CFI_MODE_CFI)?"CFI":"JEDEC",cfi->numchips);
 	/* Select the correct geometry setup */ 
 	mtd->size = devsize * cfi->numchips;
 
 	mtd->numeraseregions = cfi->cfiq->NumEraseRegions * cfi->numchips;
 	mtd->eraseregions = kmalloc(sizeof(struct mtd_erase_region_info)
-				    * mtd->numeraseregions, GFP_KERNEL);
+			* mtd->numeraseregions, GFP_KERNEL);
 	if (!mtd->eraseregions) { 
 		printk(KERN_WARNING "Failed to allocate memory for MTD erase region info\n");
 		goto setup_err;
 	}
-			
+
 	for (i=0; i<cfi->cfiq->NumEraseRegions; i++) {
 		unsigned long ernum, ersize;
 		ersize = ((cfi->cfiq->EraseRegionInfo[i] >> 8) & ~0xff) * cfi->interleave;
 		ernum = (cfi->cfiq->EraseRegionInfo[i] & 0xffff) + 1;
-			
+
 		if (mtd->erasesize < ersize) {
 			mtd->erasesize = ersize;
 		}
@@ -356,20 +360,20 @@
 	// debug
 	for (i=0; i<mtd->numeraseregions;i++){
 		printk("%d: offset=0x%x,size=0x%x,blocks=%d\n",
-		       i,mtd->eraseregions[i].offset,
-		       mtd->eraseregions[i].erasesize,
-		       mtd->eraseregions[i].numblocks);
+				i,mtd->eraseregions[i].offset,
+				mtd->eraseregions[i].erasesize,
+				mtd->eraseregions[i].numblocks);
 	}
 #endif
 
 	/* FIXME: erase-suspend-program is broken.  See
-	   http://lists.infradead.org/pipermail/linux-mtd/2003-December/009001.html */
+http://lists.infradead.org/pipermail/linux-mtd/2003-December/009001.html */
 	printk(KERN_NOTICE "cfi_cmdset_0002: Disabling erase-suspend-program due to code brokenness.\n");
 
 	__module_get(THIS_MODULE);
 	return mtd;
 
- setup_err:
+setup_err:
 	if(mtd) {
 		if(mtd->eraseregions)
 			kfree(mtd->eraseregions);
@@ -408,94 +412,94 @@
 	unsigned long timeo;
 	struct cfi_pri_amdstd *cfip = (struct cfi_pri_amdstd *)cfi->cmdset_priv;
 
- resettime:
+resettime:
 	timeo = jiffies + HZ;
- retry:
+retry:
 	switch (chip->state) {
 
-	case FL_STATUS:
-		for (;;) {
-			if (chip_ready(map, adr))
-				break;
-
-			if (time_after(jiffies, timeo)) {
-				printk(KERN_ERR "Waiting for chip to be ready timed out.\n");
+		case FL_STATUS:
+			for (;;) {
+				if (chip_ready(map, adr))
+					break;
+
+				if (time_after(jiffies, timeo)) {
+					printk(KERN_ERR "Waiting for chip to be ready timed out.\n");
+					cfi_spin_unlock(chip->mutex);
+					return -EIO;
+				}
 				cfi_spin_unlock(chip->mutex);
-				return -EIO;
+				cfi_udelay(1);
+				cfi_spin_lock(chip->mutex);
+				/* Someone else might have been playing with it. */
+				goto retry;
 			}
-			cfi_spin_unlock(chip->mutex);
-			cfi_udelay(1);
-			cfi_spin_lock(chip->mutex);
-			/* Someone else might have been playing with it. */
-			goto retry;
-		}
-				
-	case FL_READY:
-	case FL_CFI_QUERY:
-	case FL_JEDEC_QUERY:
-		return 0;
 
-	case FL_ERASING:
-		if (mode == FL_WRITING) /* FIXME: Erase-suspend-program appears broken. */
-			goto sleep;
-
-		if (!(mode == FL_READY || mode == FL_POINT
-		      || !cfip
-		      || (mode == FL_WRITING && (cfip->EraseSuspend & 0x2))
-		      || (mode == FL_WRITING && (cfip->EraseSuspend & 0x1))))
-			goto sleep;
-
-		/* We could check to see if we're trying to access the sector
-		 * that is currently being erased. However, no user will try
-		 * anything like that so we just wait for the timeout. */
-
-		/* Erase suspend */
-		/* It's harmless to issue the Erase-Suspend and Erase-Resume
-		 * commands when the erase algorithm isn't in progress. */
-		map_write(map, CMD(0xB0), chip->in_progress_block_addr);
-		chip->oldstate = FL_ERASING;
-		chip->state = FL_ERASE_SUSPENDING;
-		chip->erase_suspended = 1;
-		for (;;) {
-			if (chip_ready(map, adr))
-				break;
+		case FL_READY:
+		case FL_CFI_QUERY:
+		case FL_JEDEC_QUERY:
+			return 0;
 
-			if (time_after(jiffies, timeo)) {
-				/* Should have suspended the erase by now.
-				 * Send an Erase-Resume command as either
-				 * there was an error (so leave the erase
-				 * routine to recover from it) or we trying to
-				 * use the erase-in-progress sector. */
-				map_write(map, CMD(0x30), chip->in_progress_block_addr);
-				chip->state = FL_ERASING;
-				chip->oldstate = FL_READY;
-				printk(KERN_ERR "MTD %s(): chip not ready after erase suspend\n", __func__);
-				return -EIO;
-			}
-			
-			cfi_spin_unlock(chip->mutex);
-			cfi_udelay(1);
-			cfi_spin_lock(chip->mutex);
-			/* Nobody will touch it while it's in state FL_ERASE_SUSPENDING.
-			   So we can just loop here. */
-		}
-		chip->state = FL_READY;
-		return 0;
+		case FL_ERASING:
+			if (mode == FL_WRITING) /* FIXME: Erase-suspend-program appears broken. */
+				goto sleep;
+
+			if (!(mode == FL_READY || mode == FL_POINT
+						|| !cfip
+						|| (mode == FL_WRITING && (cfip->EraseSuspend & 0x2))
+						|| (mode == FL_WRITING && (cfip->EraseSuspend & 0x1))))
+				goto sleep;
+
+			/* We could check to see if we're trying to access the sector
+			 * that is currently being erased. However, no user will try
+			 * anything like that so we just wait for the timeout. */
+
+			/* Erase suspend */
+			/* It's harmless to issue the Erase-Suspend and Erase-Resume
+			 * commands when the erase algorithm isn't in progress. */
+			map_write(map, CMD(0xB0), chip->in_progress_block_addr);
+			chip->oldstate = FL_ERASING;
+			chip->state = FL_ERASE_SUSPENDING;
+			chip->erase_suspended = 1;
+			for (;;) {
+				if (chip_ready(map, adr))
+					break;
+
+				if (time_after(jiffies, timeo)) {
+					/* Should have suspended the erase by now.
+					 * Send an Erase-Resume command as either
+					 * there was an error (so leave the erase
+					 * routine to recover from it) or we trying to
+					 * use the erase-in-progress sector. */
+					map_write(map, CMD(0x30), chip->in_progress_block_addr);
+					chip->state = FL_ERASING;
+					chip->oldstate = FL_READY;
+					printk(KERN_ERR "MTD %s(): chip not ready after erase suspend\n", __func__);
+					return -EIO;
+				}
 
-	case FL_POINT:
-		/* Only if there's no operation suspended... */
-		if (mode == FL_READY && chip->oldstate == FL_READY)
+				cfi_spin_unlock(chip->mutex);
+				cfi_udelay(1);
+				cfi_spin_lock(chip->mutex);
+				/* Nobody will touch it while it's in state FL_ERASE_SUSPENDING.
+				   So we can just loop here. */
+			}
+			chip->state = FL_READY;
 			return 0;
 
-	default:
-	sleep:
-		set_current_state(TASK_UNINTERRUPTIBLE);
-		add_wait_queue(&chip->wq, &wait);
-		cfi_spin_unlock(chip->mutex);
-		schedule();
-		remove_wait_queue(&chip->wq, &wait);
-		cfi_spin_lock(chip->mutex);
-		goto resettime;
+		case FL_POINT:
+			/* Only if there's no operation suspended... */
+			if (mode == FL_READY && chip->oldstate == FL_READY)
+				return 0;
+
+		default:
+sleep:
+			set_current_state(TASK_UNINTERRUPTIBLE);
+			add_wait_queue(&chip->wq, &wait);
+			cfi_spin_unlock(chip->mutex);
+			schedule();
+			remove_wait_queue(&chip->wq, &wait);
+			cfi_spin_lock(chip->mutex);
+			goto resettime;
 	}
 }
 
@@ -505,20 +509,20 @@
 	struct cfi_private *cfi = map->fldrv_priv;
 
 	switch(chip->oldstate) {
-	case FL_ERASING:
-		chip->state = chip->oldstate;
-		map_write(map, CMD(0x30), chip->in_progress_block_addr);
-		chip->oldstate = FL_READY;
-		chip->state = FL_ERASING;
-		break;
-
-	case FL_READY:
-	case FL_STATUS:
-		/* We should really make set_vpp() count, rather than doing this */
-		DISABLE_VPP(map);
-		break;
-	default:
-		printk(KERN_ERR "MTD: put_chip() called with oldstate %d!!\n", chip->oldstate);
+		case FL_ERASING:
+			chip->state = chip->oldstate;
+			map_write(map, CMD(0x30), chip->in_progress_block_addr);
+			chip->oldstate = FL_READY;
+			chip->state = FL_ERASING;
+			break;
+
+		case FL_READY:
+		case FL_STATUS:
+			/* We should really make set_vpp() count, rather than doing this */
+			DISABLE_VPP(map);
+			break;
+		default:
+			printk(KERN_ERR "MTD: put_chip() called with oldstate %d!!\n", chip->oldstate);
 	}
 	wake_up(&chip->wq);
 }
@@ -530,6 +534,8 @@
 	struct cfi_private *cfi = map->fldrv_priv;
 	int ret;
 
+	down(&ebi_mutex_lock);
+
 	adr += chip->start;
 
 	/* Ensure cmd read/writes are aligned. */ 
@@ -552,6 +558,9 @@
 	put_chip(map, chip, cmd_addr);
 
 	cfi_spin_unlock(chip->mutex);
+
+	up(&ebi_mutex_lock);
+
 	return 0;
 }
 
@@ -604,7 +613,9 @@
 	unsigned long timeo = jiffies + HZ;
 	struct cfi_private *cfi = map->fldrv_priv;
 
- retry:
+	down(&ebi_mutex_lock);
+
+retry:
 	cfi_spin_lock(chip->mutex);
 
 	if (chip->state != FL_READY){
@@ -613,7 +624,7 @@
 #endif
 		set_current_state(TASK_UNINTERRUPTIBLE);
 		add_wait_queue(&chip->wq, &wait);
-		
+
 		cfi_spin_unlock(chip->mutex);
 
 		schedule();
@@ -634,17 +645,19 @@
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x88, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
-	
+
 	map_copy_from(map, buf, adr, len);
 
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x90, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x00, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
-	
+
 	wake_up(&chip->wq);
 	cfi_spin_unlock(chip->mutex);
 
+	up(&ebi_mutex_lock);
+
 	return 0;
 }
 
@@ -710,6 +723,8 @@
 	map_word oldd;
 	int retry_cnt = 0;
 
+	down(&ebi_mutex_lock);
+
 	adr += chip->start;
 
 	cfi_spin_lock(chip->mutex);
@@ -720,7 +735,7 @@
 	}
 
 	DEBUG( MTD_DEBUG_LEVEL3, "MTD %s(): WRITE 0x%.8lx(0x%.8lx)\n",
-	       __func__, adr, datum.x[0] );
+			__func__, adr, datum.x[0] );
 
 	/*
 	 * Check for a NOP for the case when the datum to write is already
@@ -731,12 +746,12 @@
 	oldd = map_read(map, adr);
 	if (map_word_equal(map, oldd, datum)) {
 		DEBUG( MTD_DEBUG_LEVEL3, "MTD %s(): NOP\n",
-		       __func__);
+				__func__);
 		goto op_done;
 	}
 
 	ENABLE_VPP(map);
- retry:
+retry:
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0x55, cfi->addr_unlock2, chip->start, map, cfi, cfi->device_type, NULL);
 	cfi_send_gen_cmd(0xA0, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
@@ -768,7 +783,7 @@
 			goto op_done;
 
 		if (time_after(jiffies, timeo))
-                        break;
+			break;
 
 		/* Latency issues. Drop the lock, wait a while and retry */
 		cfi_spin_unlock(chip->mutex);
@@ -785,17 +800,19 @@
 		goto retry;
 
 	ret = -EIO;
- op_done:
+op_done:
 	chip->state = FL_READY;
 	put_chip(map, chip, adr);
 	cfi_spin_unlock(chip->mutex);
 
+	up(&ebi_mutex_lock);
+
 	return ret;
 }
 
 
 static int cfi_amdstd_write_words(struct mtd_info *mtd, loff_t to, size_t len,
-				  size_t *retlen, const u_char *buf)
+		size_t *retlen, const u_char *buf)
 {
 	struct map_info *map = mtd->priv;
 	struct cfi_private *cfi = map->fldrv_priv;
@@ -819,7 +836,7 @@
 		int n = 0;
 		map_word tmp_buf;
 
- retry:
+retry:
 		cfi_spin_lock(cfi->chips[chipnum].mutex);
 
 		if (cfi->chips[chipnum].state != FL_READY) {
@@ -847,14 +864,14 @@
 
 		/* Number of bytes to copy from buffer */
 		n = min_t(int, len, map_bankwidth(map)-i);
-		
+
 		tmp_buf = map_word_load_partial(map, tmp_buf, buf, i, n);
 
 		ret = do_write_oneword(map, &cfi->chips[chipnum], 
-				       bus_ofs, tmp_buf);
+				bus_ofs, tmp_buf);
 		if (ret) 
 			return ret;
-		
+
 		ofs += n;
 		buf += n;
 		(*retlen) += n;
@@ -867,7 +884,7 @@
 				return 0;
 		}
 	}
-	
+
 	/* We are now aligned, write as much as possible */
 	while(len >= map_bankwidth(map)) {
 		map_word datum;
@@ -875,7 +892,7 @@
 		datum = map_word_load(map, buf);
 
 		ret = do_write_oneword(map, &cfi->chips[chipnum],
-				       ofs, datum);
+				ofs, datum);
 		if (ret)
 			return ret;
 
@@ -897,7 +914,7 @@
 	if (len & (map_bankwidth(map)-1)) {
 		map_word tmp_buf;
 
- retry1:
+retry1:
 		cfi_spin_lock(cfi->chips[chipnum].mutex);
 
 		if (cfi->chips[chipnum].state != FL_READY) {
@@ -923,12 +940,12 @@
 		cfi_spin_unlock(cfi->chips[chipnum].mutex);
 
 		tmp_buf = map_word_load_partial(map, tmp_buf, buf, 0, len);
-	
+
 		ret = do_write_oneword(map, &cfi->chips[chipnum], 
 				ofs, tmp_buf);
 		if (ret) 
 			return ret;
-		
+
 		(*retlen) += len;
 	}
 
@@ -940,7 +957,7 @@
  * FIXME: interleaved mode not tested, and probably not supported!
  */
 static inline int do_write_buffer(struct map_info *map, struct flchip *chip, 
-				  unsigned long adr, const u_char *buf, int len)
+		unsigned long adr, const u_char *buf, int len)
 {
 	struct cfi_private *cfi = map->fldrv_priv;
 	unsigned long timeo = jiffies + HZ;
@@ -951,6 +968,8 @@
 	int z, words;
 	map_word datum;
 
+	down(&ebi_mutex_lock);
+
 	adr += chip->start;
 	cmd_adr = adr;
 
@@ -964,7 +983,7 @@
 	datum = map_word_load(map, buf);
 
 	DEBUG( MTD_DEBUG_LEVEL3, "MTD %s(): WRITE 0x%.8lx(0x%.8lx)\n",
-	       __func__, adr, datum.x[0] );
+			__func__, adr, datum.x[0] );
 
 	ENABLE_VPP(map);
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
@@ -1001,7 +1020,7 @@
 	cfi_spin_lock(chip->mutex);
 
 	timeo = jiffies + uWriteTimeout; 
-		
+
 	for (;;) {
 		if (chip->state != FL_WRITING) {
 			/* Someone's suspended the write. Sleep */
@@ -1019,7 +1038,7 @@
 
 		if (chip_ready(map, adr))
 			goto op_done;
-		    
+
 		if( time_after(jiffies, timeo))
 			break;
 
@@ -1030,24 +1049,26 @@
 	}
 
 	printk(KERN_WARNING "MTD %s(): software timeout\n",
-	       __func__ );
+			__func__ );
 
 	/* reset on all failures. */
 	map_write( map, CMD(0xF0), chip->start );
 	/* FIXME - should have reset delay before continuing */
 
 	ret = -EIO;
- op_done:
+op_done:
 	chip->state = FL_READY;
 	put_chip(map, chip, adr);
 	cfi_spin_unlock(chip->mutex);
 
+	up(&ebi_mutex_lock);
+
 	return ret;
 }
 
 
 static int cfi_amdstd_write_buffers(struct mtd_info *mtd, loff_t to, size_t len,
-				    size_t *retlen, const u_char *buf)
+		size_t *retlen, const u_char *buf)
 {
 	struct map_info *map = mtd->priv;
 	struct cfi_private *cfi = map->fldrv_priv;
@@ -1069,7 +1090,7 @@
 		if (local_len > len)
 			local_len = len;
 		ret = cfi_amdstd_write_words(mtd, ofs + (chipnum<<cfi->chipshift),
-					     local_len, retlen, buf);
+				local_len, retlen, buf);
 		if (ret)
 			return ret;
 		ofs += local_len;
@@ -1095,7 +1116,7 @@
 			size -= size % map_bankwidth(map);
 
 		ret = do_write_buffer(map, &cfi->chips[chipnum], 
-				      ofs, buf, size);
+				ofs, buf, size);
 		if (ret)
 			return ret;
 
@@ -1116,7 +1137,7 @@
 		size_t retlen_dregs = 0;
 
 		ret = cfi_amdstd_write_words(mtd, ofs + (chipnum<<cfi->chipshift),
-					     len, &retlen_dregs, buf);
+				len, &retlen_dregs, buf);
 
 		*retlen += retlen_dregs;
 		return ret;
@@ -1138,6 +1159,8 @@
 	DECLARE_WAITQUEUE(wait, current);
 	int ret = 0;
 
+	down(&ebi_mutex_lock);
+
 	adr = cfi->addr_unlock1;
 
 	cfi_spin_lock(chip->mutex);
@@ -1148,7 +1171,7 @@
 	}
 
 	DEBUG( MTD_DEBUG_LEVEL3, "MTD %s(): ERASE 0x%.8lx\n",
-	       __func__, chip->start );
+			__func__, chip->start );
 
 	ENABLE_VPP(map);
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
@@ -1200,18 +1223,20 @@
 	}
 
 	printk(KERN_WARNING "MTD %s(): software timeout\n",
-	       __func__ );
+			__func__ );
 
 	/* reset on all failures. */
 	map_write( map, CMD(0xF0), chip->start );
 	/* FIXME - should have reset delay before continuing */
 
 	ret = -EIO;
- op_done:
+op_done:
 	chip->state = FL_READY;
 	put_chip(map, chip, adr);
 	cfi_spin_unlock(chip->mutex);
 
+	up(&ebi_mutex_lock);
+
 	return ret;
 }
 
@@ -1223,6 +1248,8 @@
 	DECLARE_WAITQUEUE(wait, current);
 	int ret = 0;
 
+	down(&ebi_mutex_lock);
+
 	adr += chip->start;
 
 	cfi_spin_lock(chip->mutex);
@@ -1233,7 +1260,7 @@
 	}
 
 	DEBUG( MTD_DEBUG_LEVEL3, "MTD %s(): ERASE 0x%.8lx\n",
-	       __func__, adr );
+			__func__, adr );
 
 	ENABLE_VPP(map);
 	cfi_send_gen_cmd(0xAA, cfi->addr_unlock1, chip->start, map, cfi, cfi->device_type, NULL);
@@ -1246,7 +1273,7 @@
 	chip->state = FL_ERASING;
 	chip->erase_suspended = 0;
 	chip->in_progress_block_addr = adr;
-	
+
 	cfi_spin_unlock(chip->mutex);
 	msleep(chip->erase_time/2);
 	cfi_spin_lock(chip->mutex);
@@ -1283,19 +1310,20 @@
 		schedule_timeout(1);
 		cfi_spin_lock(chip->mutex);
 	}
-	
+
 	printk(KERN_WARNING "MTD %s(): software timeout\n",
-	       __func__ );
-	
+			__func__ );
+
 	/* reset on all failures. */
 	map_write( map, CMD(0xF0), chip->start );
 	/* FIXME - should have reset delay before continuing */
 
 	ret = -EIO;
- op_done:
+op_done:
 	chip->state = FL_READY;
 	put_chip(map, chip, adr);
 	cfi_spin_unlock(chip->mutex);
+	up(&ebi_mutex_lock);
 	return ret;
 }
 
@@ -1314,7 +1342,7 @@
 
 	instr->state = MTD_ERASE_DONE;
 	mtd_erase_callback(instr);
-	
+
 	return 0;
 }
 
@@ -1337,7 +1365,7 @@
 
 	instr->state = MTD_ERASE_DONE;
 	mtd_erase_callback(instr);
-	
+
 	return 0;
 }
 
@@ -1354,35 +1382,35 @@
 	for (i=0; !ret && i<cfi->numchips; i++) {
 		chip = &cfi->chips[i];
 
-	retry:
+retry:
 		cfi_spin_lock(chip->mutex);
 
 		switch(chip->state) {
-		case FL_READY:
-		case FL_STATUS:
-		case FL_CFI_QUERY:
-		case FL_JEDEC_QUERY:
-			chip->oldstate = chip->state;
-			chip->state = FL_SYNCING;
-			/* No need to wake_up() on this state change - 
-			 * as the whole point is that nobody can do anything
-			 * with the chip now anyway.
-			 */
-		case FL_SYNCING:
-			cfi_spin_unlock(chip->mutex);
-			break;
+			case FL_READY:
+			case FL_STATUS:
+			case FL_CFI_QUERY:
+			case FL_JEDEC_QUERY:
+				chip->oldstate = chip->state;
+				chip->state = FL_SYNCING;
+				/* No need to wake_up() on this state change - 
+				 * as the whole point is that nobody can do anything
+				 * with the chip now anyway.
+				 */
+			case FL_SYNCING:
+				cfi_spin_unlock(chip->mutex);
+				break;
 
-		default:
-			/* Not an idle state */
-			add_wait_queue(&chip->wq, &wait);
-			
-			cfi_spin_unlock(chip->mutex);
+			default:
+				/* Not an idle state */
+				add_wait_queue(&chip->wq, &wait);
 
-			schedule();
+				cfi_spin_unlock(chip->mutex);
 
-			remove_wait_queue(&chip->wq, &wait);
-			
-			goto retry;
+				schedule();
+
+				remove_wait_queue(&chip->wq, &wait);
+
+				goto retry;
 		}
 	}
 
@@ -1392,7 +1420,7 @@
 		chip = &cfi->chips[i];
 
 		cfi_spin_lock(chip->mutex);
-		
+
 		if (chip->state == FL_SYNCING) {
 			chip->state = chip->oldstate;
 			wake_up(&chip->wq);
@@ -1416,22 +1444,22 @@
 		cfi_spin_lock(chip->mutex);
 
 		switch(chip->state) {
-		case FL_READY:
-		case FL_STATUS:
-		case FL_CFI_QUERY:
-		case FL_JEDEC_QUERY:
-			chip->oldstate = chip->state;
-			chip->state = FL_PM_SUSPENDED;
-			/* No need to wake_up() on this state change - 
-			 * as the whole point is that nobody can do anything
-			 * with the chip now anyway.
-			 */
-		case FL_PM_SUSPENDED:
-			break;
+			case FL_READY:
+			case FL_STATUS:
+			case FL_CFI_QUERY:
+			case FL_JEDEC_QUERY:
+				chip->oldstate = chip->state;
+				chip->state = FL_PM_SUSPENDED;
+				/* No need to wake_up() on this state change - 
+				 * as the whole point is that nobody can do anything
+				 * with the chip now anyway.
+				 */
+			case FL_PM_SUSPENDED:
+				break;
 
-		default:
-			ret = -EAGAIN;
-			break;
+			default:
+				ret = -EAGAIN;
+				break;
 		}
 		cfi_spin_unlock(chip->mutex);
 	}
@@ -1443,7 +1471,7 @@
 			chip = &cfi->chips[i];
 
 			cfi_spin_lock(chip->mutex);
-		
+
 			if (chip->state == FL_PM_SUSPENDED) {
 				chip->state = chip->oldstate;
 				wake_up(&chip->wq);
@@ -1451,7 +1479,7 @@
 			cfi_spin_unlock(chip->mutex);
 		}
 	}
-	
+
 	return ret;
 }
 
@@ -1464,11 +1492,11 @@
 	struct flchip *chip;
 
 	for (i=0; i<cfi->numchips; i++) {
-	
+
 		chip = &cfi->chips[i];
 
 		cfi_spin_lock(chip->mutex);
-		
+
 		if (chip->state == FL_PM_SUSPENDED) {
 			chip->state = FL_READY;
 			map_write(map, CMD(0xF0), chip->start);
Only in chipbox-kernel/drivers/mtd/chips: cfi_cmdset_0002_sst.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/jedec_probe.c chipbox-kernel/drivers/mtd/chips/jedec_probe.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/jedec_probe.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/chips/jedec_probe.c	2021-12-27 13:08:35.781010954 +0300
@@ -153,6 +153,7 @@
 #define SST49LF030A	0x001C
 #define SST49LF040A	0x0051
 #define SST49LF080A	0x005B
+#define SST39VF6401B    0x236D
 
 /* Toshiba */
 #define TC58FVT160	0x00C2
@@ -1449,6 +1450,21 @@
                        ERASEINFO(0x1000,256)
                }
 
+	}, {
+               .mfr_id         = MANUFACTURER_SST,     /* should be CFI */
+               .dev_id         = SST39VF6401B,
+               .name           = "SST 39VF6401B",
+               .uaddr          = {
+                       [0] = MTD_UADDR_0x5555_0x2AAA,  /* x8 */
+                       [1] = MTD_UADDR_0x5555_0x2AAA   /* x16 */
+               },
+               .DevSize        = SIZE_8MiB,
+               .CmdSet         = P_ID_AMD_STD,
+               .NumEraseRegions= 1,
+               .regions        = {
+                       ERASEINFO(0x10000,128)
+               }
+
        }, {
 		.mfr_id		= MANUFACTURER_ST,	/* FIXME - CFI device? */
 		.dev_id		= M29W800DT,
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/Makefile chipbox-kernel/drivers/mtd/chips/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/chips/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/chips/Makefile	2021-12-27 13:08:35.781010954 +0300
@@ -15,7 +15,7 @@
 obj-$(CONFIG_MTD_CFI)		+= cfi_probe.o
 obj-$(CONFIG_MTD_CFI_UTIL)	+= cfi_util.o
 obj-$(CONFIG_MTD_CFI_STAA)	+= cfi_cmdset_0020.o
-obj-$(CONFIG_MTD_CFI_AMDSTD)	+= cfi_cmdset_0002.o
+obj-$(CONFIG_MTD_CFI_AMDSTD)	+= cfi_cmdset_0002.o cfi_cmdset_0002_sst.o
 obj-$(CONFIG_MTD_CFI_INTELEXT)	+= cfi_cmdset_0001.o
 obj-$(CONFIG_MTD_GEN_PROBE)	+= gen_probe.o
 obj-$(CONFIG_MTD_JEDEC)		+= jedec.o
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/cmdlinepart.c chipbox-kernel/drivers/mtd/cmdlinepart.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/cmdlinepart.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/cmdlinepart.c	2021-12-27 13:08:35.785011015 +0300
@@ -288,7 +288,7 @@
  * information. It returns partitions for the requested mtd device, or
  * the first one in the chain if a NULL mtd_id is passed in.
  */
-static int parse_cmdline_partitions(struct mtd_info *master, 
+int parse_cmdline_partitions(struct mtd_info *master, 
                              struct mtd_partition **pparts,
                              unsigned long origin)
 {
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/devices/Kconfig chipbox-kernel/drivers/mtd/devices/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/devices/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/devices/Kconfig	2021-12-27 13:08:35.785011015 +0300
@@ -255,5 +255,10 @@
 	  LinuxBIOS or if you need to recover a DiskOnChip Millennium on which
 	  you have managed to wipe the first block.
 
+config MTD_SPIDEV
+	bool "CSM120X spi flash support"
+	help
+	  spi flash support for csm120x
+
 endmenu
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/devices/Makefile chipbox-kernel/drivers/mtd/devices/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/devices/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/devices/Makefile	2021-12-27 13:08:35.785011015 +0300
@@ -23,3 +23,4 @@
 obj-$(CONFIG_MTD_LART)		+= lart.o
 obj-$(CONFIG_MTD_BLKMTD)	+= blkmtd.o
 obj-$(CONFIG_MTD_BLOCK2MTD)	+= block2mtd.o
+obj-$(CONFIG_MTD_SPIDEV)	+= orion_spidev.o
Only in chipbox-kernel/drivers/mtd/devices: orion_spidev.c
Only in chipbox-kernel/drivers/mtd/devices: orion_spidev.h
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/mtd_blkdevs.c chipbox-kernel/drivers/mtd/mtd_blkdevs.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/mtd_blkdevs.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/mtd_blkdevs.c	2021-12-27 13:08:35.793011136 +0300
@@ -289,11 +289,39 @@
 	gd->major = tr->major;
 	gd->first_minor = (new->devnum) << tr->part_bits;
 	gd->fops = &mtd_blktrans_ops;
-	
-	snprintf(gd->disk_name, sizeof(gd->disk_name),
-		 "%s%c", tr->name, (tr->part_bits?'a':'0') + new->devnum);
-	snprintf(gd->devfs_name, sizeof(gd->devfs_name),
-		 "%s/%c", tr->name, (tr->part_bits?'a':'0') + new->devnum);
+
+    if (tr->part_bits){
+        if (new->devnum < 26){
+            snprintf(gd->disk_name, sizeof(gd->disk_name),
+                     "%s%c", tr->name, 'a' + new->devnum);
+            snprintf(gd->devfs_name, sizeof(gd->devfs_name),
+                     "%s/%c", tr->name, 'a' + new->devnum);
+            
+        }
+        else{
+            snprintf(gd->disk_name, sizeof(gd->disk_name),
+                     "%s%c%c", tr->name,
+                     'a' - 1 + new->devnum / 26,
+                     'a' + new->devnum % 26);
+            snprintf(gd->devfs_name, sizeof(gd->devfs_name),
+                     "%s/%c%c", tr->name,
+                     'a' - 1 + new->devnum / 26,
+                     'a' + new->devnum % 26);
+        }
+    }
+    else{
+        snprintf(gd->disk_name, sizeof(gd->disk_name),
+                 "%s%d", tr->name, new->devnum);
+        snprintf(gd->devfs_name, sizeof(gd->devfs_name),
+                 "%s/%d", tr->name, new->devnum);
+        
+    }
+    
+// 	snprintf(gd->disk_name, sizeof(gd->disk_name),
+// 		 "%s%c", tr->name, (tr->part_bits?'a':'0') + new->devnum);
+
+// 	snprintf(gd->devfs_name, sizeof(gd->devfs_name),
+// 		 "%s/%c", tr->name, (tr->part_bits?'a':'0') + new->devnum);
 
 	/* 2.5 has capacity in units of 512 bytes while still
 	   having BLOCK_SIZE_BITS set to 10. Just to keep us amused. */
@@ -379,7 +407,7 @@
 	memset(tr->blkcore_priv, 0, sizeof(*tr->blkcore_priv));
 
 	down(&mtd_table_mutex);
-
+    //printk("register %s block device on major %d\n",tr->name, tr->major);
 	ret = register_blkdev(tr->major, tr->name);
 	if (ret) {
 		printk(KERN_WARNING "Unable to register %s block device on major %d: %d\n",
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/mtdchar.c chipbox-kernel/drivers/mtd/mtdchar.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/mtdchar.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/mtdchar.c	2021-12-27 13:08:35.793011136 +0300
@@ -14,15 +14,23 @@
 #include <linux/init.h>
 #include <linux/fs.h>
 #include <asm/uaccess.h>
-
+#include <linux/device.h>
 #ifdef CONFIG_DEVFS_FS
 #include <linux/devfs_fs_kernel.h>
 
+static struct class_simple *mtd_class;
 static void mtd_notify_add(struct mtd_info* mtd)
 {
 	if (!mtd)
 		return;
 
+	class_simple_device_add(mtd_class, MKDEV(MTD_CHAR_MAJOR, mtd->index*2),
+			    NULL, "mtd%d", mtd->index);
+	
+	class_simple_device_add(mtd_class, 
+			    MKDEV(MTD_CHAR_MAJOR, mtd->index*2+1),
+			    NULL, "mtd%dro", mtd->index);
+
 	devfs_mk_cdev(MKDEV(MTD_CHAR_MAJOR, mtd->index*2),
 		      S_IFCHR | S_IRUGO | S_IWUGO, "mtd/%d", mtd->index);
 		
@@ -34,6 +42,10 @@
 {
 	if (!mtd)
 		return;
+
+	class_simple_device_remove(MKDEV(MTD_CHAR_MAJOR, mtd->index*2));
+	class_simple_device_remove(MKDEV(MTD_CHAR_MAJOR, mtd->index*2+1));
+
 	devfs_remove("mtd/%d", mtd->index);
 	devfs_remove("mtd/%dro", mtd->index);
 }
@@ -543,6 +555,15 @@
 		return -EAGAIN;
 	}
 
+	mtd_class = class_simple_create(THIS_MODULE, "mtd");
+
+	if (IS_ERR(mtd_class)) {
+		printk(KERN_ERR "Error creating mtd class.\n");
+		unregister_chrdev(MTD_CHAR_MAJOR, "mtd");
+		return PTR_ERR(mtd_class);
+	}
+
+
 	mtdchar_devfs_init();
 	return 0;
 }
@@ -550,7 +571,9 @@
 static void __exit cleanup_mtdchar(void)
 {
 	mtdchar_devfs_exit();
-	unregister_chrdev(MTD_CHAR_MAJOR, "mtd");
+	class_simple_destroy(mtd_class);
+    unregister_chrdev(MTD_CHAR_MAJOR, "mtd");
+   
 }
 
 module_init(init_mtdchar);
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/Kconfig chipbox-kernel/drivers/mtd/nand/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/Kconfig	2021-12-27 13:08:35.797011198 +0300
@@ -94,6 +94,36 @@
 	help
 	  This enables the NAND flash driver on the PPChameleon EVB Board.
 
+config MTD_NAND_ORION
+        tristate "NAND Flash support for CSM1100/CSM1200 SoC"
+        depends on (ARCH_ORION_CSM1100 || ARCH_ORION_CSM1200 || ARCH_ORION_CSM1201) && MTD_NAND
+        help
+          This enables the NAND flash on the CSM1100/CSM1200 without NAND booting.
+
+          No board specfic support is done by this driver, each board
+          must advertise a platform_device for the driver to attach.
+
+
+config MTD_NAND_ORION_SUPPORT
+        bool "NAND Flash support for CSM1100/CSM1200 SoC"
+        depends on MTD_NAND_ORION
+        help
+          This enables the NAND flash on the CSM1100/CSM1200.
+
+          No board specfic support is done by this driver, each board
+          must advertise a platform_device for the driver to attach.
+
+
+config MTD_NAND_ORION_NANDBOOT
+        bool "NAND Flash support for CSM1200 SoC and NAND booting"
+        depends on (ARCH_ORION_CSM1200 || ARCH_ORION_CSM1201 )&& MTD_NAND_ORION && MTD_NAND_ORION_SUPPORT
+        help
+          This enables the NAND flash on the CSM1200 and the system is booted from NAND flash.
+
+          No board specfic support is done by this driver, each board
+          must advertise a platform_device for the driver to attach.
+
+
 config MTD_NAND_S3C2410
 	tristate "NAND Flash support for S3C2410 SoC"
 	depends on ARCH_S3C2410 && MTD_NAND
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/Makefile chipbox-kernel/drivers/mtd/nand/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/Makefile	2021-12-27 13:08:35.797011198 +0300
@@ -20,5 +20,5 @@
 obj-$(CONFIG_MTD_NAND_RTC_FROM4)	+= rtc_from4.o
 obj-$(CONFIG_MTD_NAND_SHARPSL)		+= sharpsl.o
 obj-$(CONFIG_MTD_NAND_NANDSIM)		+= nandsim.o
-
+obj-$(CONFIG_MTD_NAND_ORION)            += orion_nand.o
 nand-objs = nand_base.o nand_bbt.o
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_base.c chipbox-kernel/drivers/mtd/nand/nand_base.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_base.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/nand_base.c	2021-12-27 13:08:35.797011198 +0300
@@ -2235,12 +2235,17 @@
 			extid = this->read_byte(mtd);
 			/* The 4th id byte is the important one */
 			extid = this->read_byte(mtd);
+
+            printk("Debug Info: extid4=0x%x\n",extid);
 			/* Calc pagesize */
 			mtd->oobblock = 1024 << (extid & 0x3);
 			extid >>= 2;
 			/* Calc oobsize */
-			mtd->oobsize = (8 << (extid & 0x03)) * (mtd->oobblock / 512);
-			extid >>= 2;
+			mtd->oobsize = (8 << (extid & 0x1)) * (mtd->oobblock / 512);
+
+			printk("Debug Info: oobsize=%d\n",mtd->oobsize);
+
+            extid >>= 2;
 			/* Calc blocksize. Blocksize is multiples of 64KiB */
 			mtd->erasesize = (64 * 1024)  << (extid & 0x03);
 			extid >>= 2;
@@ -2287,7 +2292,7 @@
 		/* Check if this is a not a samsung device. Do not clear the options
 		 * for chips which are not having an extended id.
 		 */	
-		if (nand_maf_id != NAND_MFR_SAMSUNG && !nand_flash_ids[i].pagesize)
+		if (nand_maf_id != NAND_MFR_SAMSUNG && nand_maf_id !=NAND_MFR_HYNIX && !nand_flash_ids[i].pagesize)
 			this->options &= ~NAND_SAMSUNG_LP_OPTIONS;
 		
 		/* Check for AND chips with 4 page planes */
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_bbt.c chipbox-kernel/drivers/mtd/nand/nand_bbt.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_bbt.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/nand_bbt.c	2021-12-27 13:08:35.797011198 +0300
@@ -60,7 +60,7 @@
 #include <linux/mtd/compatmac.h>
 #include <linux/bitops.h>
 #include <linux/delay.h>
-
+#include <linux/vmalloc.h>
 
 /** 
  * check_pattern - [GENERIC] check if a pattern is in the buffer
@@ -814,10 +814,13 @@
 	/* Allocate a temporary buffer for one eraseblock incl. oob */
 	len = (1 << this->bbt_erase_shift);
 	len += (len >> this->page_shift) * mtd->oobsize;
-	buf = kmalloc (len, GFP_KERNEL);
+
+
+//	buf = kmalloc (len, GFP_KERNEL);
+    buf = vmalloc(len);
 	if (!buf) {
 		printk (KERN_ERR "nand_bbt: Out of memory\n");
-		kfree (this->bbt);
+		vfree (this->bbt);
 		this->bbt = NULL;
 		return -ENOMEM;
 	}
@@ -838,7 +841,7 @@
 	if (md)
 		mark_bbt_region (mtd, md);
 	
-	kfree (buf);
+	vfree (buf);
 	return res;
 }
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_ids.c chipbox-kernel/drivers/mtd/nand/nand_ids.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/nand_ids.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/nand_ids.c	2021-12-27 13:08:35.797011198 +0300
@@ -118,6 +118,7 @@
 	{NAND_MFR_NATIONAL, "National"},
 	{NAND_MFR_RENESAS, "Renesas"},
 	{NAND_MFR_STMICRO, "ST Micro"},
+        {NAND_MFR_HYNIX,   "HYNIX"},
 	{0x0, "Unknown"}
 };
 
Only in chipbox-kernel/drivers/mtd/nand: orion_nand.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/s3c2410.c chipbox-kernel/drivers/mtd/nand/s3c2410.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/mtd/nand/s3c2410.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/mtd/nand/s3c2410.c	2021-12-27 13:08:35.797011198 +0300
@@ -436,7 +436,7 @@
 }
 
 /* device management functions */
-
+:
 static int s3c2410_nand_remove(struct device *dev)
 {
 	struct s3c2410_nand_info *info = to_nand_info(dev);
Only in chipbox-kernel/drivers/net: cn100
Only in chipbox-kernel/drivers/net: dm9000.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/net/Kconfig chipbox-kernel/drivers/net/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/net/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/net/Kconfig	2021-12-27 13:08:35.805011319 +0300
@@ -292,6 +292,7 @@
 	  and read the Ethernet-HOWTO, available from
 	  <http://www.tldp.org/docs.html#howto>.
 
+
 config MAC89x0
 	tristate "Macintosh CS89x0 based ethernet cards"
 	depends on NET_ETHERNET && MAC && BROKEN
@@ -824,6 +825,26 @@
 	  <file:Documentation/networking/net-modules.txt>. The module
 	  will be called smc9194.
 
+config DM9000
+        tristate "DM9000 support"
+        depends on NET_ETHERNET && ARCH_ORION_CSM1100
+        select CRC32
+        select MII
+        ---help---
+          Support for DM9000 chipset.
+
+          To compile this driver as a module, choose M here and read
+          <file:Documentation/networking/net-modules.txt>.  The module will be
+          called dm9000.
+
+config CN100
+        tristate "ORION CN100 support"
+        depends on NET_ETHERNET && (ARCH_ORION_CSM1200 || ARCH_ORION_CSM1201)
+        select CRC32
+        select MII
+        ---help---
+          Support for built-in 100M ethernet in CSM1200 from Celestial Semiconductor.
+
 config NET_VENDOR_RACAL
 	bool "Racal-Interlan (Micom) NI cards"
 	depends on NET_ETHERNET && ISA
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/net/Makefile chipbox-kernel/drivers/net/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/net/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/net/Makefile	2021-12-27 13:08:35.805011319 +0300
@@ -181,6 +181,8 @@
 obj-$(CONFIG_S2IO) += s2io.o
 obj-$(CONFIG_SMC91X) += smc91x.o
 obj-$(CONFIG_FEC_8XX) += fec_8xx/
+obj-$(CONFIG_DM9000) += dm9000.o
+obj-$(CONFIG_CN100)  += cn100/
 
 obj-$(CONFIG_ARM) += arm/
 obj-$(CONFIG_DEV_APPLETALK) += appletalk/
Only in chipbox-kernel/drivers/net: .mii.c.swp
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/cistpl.c chipbox-kernel/drivers/pcmcia/cistpl.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/cistpl.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/cistpl.c	2021-12-27 13:08:35.917013020 +0300
@@ -127,6 +127,7 @@
     
     cs_dbg(s, 3, "read_cis_mem(%d, %#x, %u)\n", attr, addr, len);
 
+//printk("read_cis_mem(%d, %#x, %u)\n", attr, addr, len);
     if (attr & IS_INDIRECT) {
 	/* Indirect accesses use a bunch of special registers at fixed
 	   locations in common memory */
@@ -136,6 +137,18 @@
 	    flags = ICTRL0_AUTOINC;
 	}
 
+#ifdef ORION_PCMCIA
+	s->ops->set_comm_wmode();
+	s->ops->write_comm(flags, CISREG_ICTRL0);
+	s->ops->write_comm(addr & 0xff, CISREG_IADDR0);
+	s->ops->write_comm((addr>>8) & 0xff, CISREG_IADDR1);
+	s->ops->write_comm((addr>>16) & 0xff, CISREG_IADDR2);
+	s->ops->write_comm((addr>>24) & 0xff, CISREG_IADDR3);
+
+	s->ops->set_comm_rmode();
+	for ( ; len > 0; len--, buf++)
+	    *buf = s->ops->read_comm(CISREG_IDATA0);
+#else
 	sys = set_cis_map(s, 0, MAP_ACTIVE | ((cis_width) ? MAP_16BIT : 0));
 	if (!sys) {
 	    memset(ptr, 0xff, len);
@@ -149,6 +162,7 @@
 	writeb((addr>>24) & 0xff, sys+CISREG_IADDR3);
 	for ( ; len > 0; len--, buf++)
 	    *buf = readb(sys+CISREG_IDATA0);
+#endif
     } else {
 	u_int inc = 1, card_offset, flags;
 
@@ -157,8 +171,22 @@
 	    flags |= MAP_ATTRIB;
 	    inc++;
 	    addr *= 2;
+#ifdef ORION_PCMCIA
+	    s->ops->set_cis_rmode();
+#endif
+	}
+#ifdef ORION_PCMCIA
+	else
+	    s->ops->set_comm_rmode();
+
+	for ( ; len > 0; len--, buf++, addr += inc) {
+	    if(attr)
+		*buf = s->ops->read_cis(addr);
+	    else
+		*buf = s->ops->read_comm(addr);
+//printk("    addr:0x%x	*buf:0x%02x.\n",(unsigned int)addr, *buf);
 	}
-
+#else
 	card_offset = addr & ~(s->map_size-1);
 	while (len) {
 	    sys = set_cis_map(s, card_offset, flags);
@@ -176,6 +204,7 @@
 	    card_offset += s->map_size;
 	    addr = 0;
 	}
+#endif
     }
     cs_dbg(s, 3, "  %#2.2x %#2.2x %#2.2x %#2.2x ...\n",
 	  *(u_char *)(ptr+0), *(u_char *)(ptr+1),
@@ -200,6 +229,17 @@
 	    flags = ICTRL0_AUTOINC;
 	}
 
+#ifdef ORION_PCMCIA
+	s->ops->set_comm_wmode();
+	s->ops->write_comm(flags, CISREG_ICTRL0);
+	s->ops->write_comm(addr & 0xff, CISREG_IADDR0);
+	s->ops->write_comm((addr>>8) & 0xff, CISREG_IADDR1);
+	s->ops->write_comm((addr>>16) & 0xff, CISREG_IADDR2);
+	s->ops->write_comm((addr>>24) & 0xff, CISREG_IADDR3);
+
+	for ( ; len > 0; len--, buf++)
+	    s->ops->write_comm(*buf, CISREG_IDATA0);
+#else
 	sys = set_cis_map(s, 0, MAP_ACTIVE | ((cis_width) ? MAP_16BIT : 0));
 	if (!sys)
 		return; /* FIXME: Error */
@@ -211,6 +251,7 @@
 	writeb((addr>>24) & 0xff, sys+CISREG_IADDR3);
 	for ( ; len > 0; len--, buf++)
 	    writeb(*buf, sys+CISREG_IDATA0);
+#endif
     } else {
 	u_int inc = 1, card_offset, flags;
 
@@ -219,8 +260,21 @@
 	    flags |= MAP_ATTRIB;
 	    inc++;
 	    addr *= 2;
+#ifdef ORION_PCMCIA
+	    s->ops->set_cis_wmode();
+#endif
+	}
+#ifdef ORION_PCMCIA
+	else
+	    s->ops->set_comm_wmode();
+
+	for ( ; len > 0; len--, buf++, addr += inc) {
+	    if(attr)
+		s->ops->write_cis(*buf, addr);
+	    else
+		s->ops->write_comm(*buf, addr);
 	}
-
+#else
 	card_offset = addr & ~(s->map_size-1);
 	while (len) {
 	    sys = set_cis_map(s, card_offset, flags);
@@ -237,6 +291,7 @@
 	    card_offset += s->map_size;
 	    addr = 0;
 	}
+#endif
     }
 }
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/cs.c chipbox-kernel/drivers/pcmcia/cs.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/cs.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/cs.c	2021-12-27 13:08:35.917013020 +0300
@@ -64,6 +64,15 @@
 #define OPTIONS PCI_OPT CB_OPT PM_OPT
 #endif
 
+/* added by xm.chen */
+/*--------------------------------------------------
+* #undef cs_dbg(skt, lvl, fmt, arg...)
+* #define cs_dbg(skt, lvl, fmt, arg...) do {		\
+* 		printk("cs: %s: " fmt, 	\
+* 		       cs_socket_name(skt) , ## arg);	\
+* } while (0)
+*--------------------------------------------------*/
+
 static const char *release = "Linux Kernel Card Services";
 static const char *options = "options: " OPTIONS;
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/ds.c chipbox-kernel/drivers/pcmcia/ds.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/ds.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/ds.c	2021-12-27 13:08:35.917013020 +0300
@@ -70,6 +70,13 @@
 #define ds_dbg(lvl, fmt, arg...) do { } while (0)
 #endif
 
+/*add by xm.chen */
+/*--------------------------------------------------
+* #undef ds_dbg(lvl, fmt, arg...)
+* #define ds_dbg(lvl, fmt, arg...) do {				\
+* 		printk("ds: " fmt , ## arg);		\
+* } while (0)
+*--------------------------------------------------*/
 /*====================================================================*/
 
 /* Device user information */
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/Kconfig chipbox-kernel/drivers/pcmcia/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/Kconfig	2021-12-27 13:08:35.917013020 +0300
@@ -75,6 +75,22 @@
 
 comment "PC-card bridges"
 
+config PCMCIA_ORION
+        tristate "Orion PCMCIA(Socket Service) support for CSM120x SoC"
+        depends on PCMCIA
+        help
+          This enables the PCMCIA Socket Service on the CSM1200 .
+
+          No board specfic support is done by this driver, each board
+          must advertise a platform_device for the driver to attach.
+
+config PCMCIA_ORION_CI
+        tristate "Orion PCMCIA Common Interface clinet driver"
+        depends on PCMCIA_ORION
+        help
+	  Common Interface Specification for Conditional 
+	  Access and other Digital Video Broadcasting Decoder Applications
+
 config YENTA
 	tristate "CardBus yenta-compatible bridge support"
 	depends on PCI
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/Makefile chipbox-kernel/drivers/pcmcia/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/Makefile	2021-12-27 13:08:35.917013020 +0300
@@ -10,13 +10,15 @@
 pcmcia_core-$(CONFIG_CARDBUS)			+= cardbus.o
 obj-$(CONFIG_PCCARD)				+= pcmcia_core.o
 
-pcmcia-y					+= ds.o pcmcia_compat.o
+pcmcia-y					+= ds.o pcmcia_compat.o soc_common.o
 obj-$(CONFIG_PCMCIA)				+= pcmcia.o
 
 obj-$(CONFIG_PCCARD_NONSTATIC)			+= rsrc_nonstatic.o
 
 
 # socket drivers
+obj-$(CONFIG_PCMCIA_ORION)			+= orion_socket.o
+obj-$(CONFIG_PCMCIA_ORION_CI)			+= orion_socket_ci.o
 
 obj-$(CONFIG_YENTA) 				+= yenta_socket.o
 
Only in chipbox-kernel/drivers/pcmcia: orion_socket.bac
Only in chipbox-kernel/drivers/pcmcia: orion_socketbase.bah
Only in chipbox-kernel/drivers/pcmcia: orion_socketbase.h
Only in chipbox-kernel/drivers/pcmcia: orion_socket.c
Only in chipbox-kernel/drivers/pcmcia: orion_socket_ci.bac
Only in chipbox-kernel/drivers/pcmcia: orion_socket_ci.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/pcmcia_compat.c chipbox-kernel/drivers/pcmcia/pcmcia_compat.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/pcmcia_compat.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/pcmcia_compat.c	2021-12-27 13:08:35.921013083 +0300
@@ -123,3 +123,126 @@
 }
 EXPORT_SYMBOL(pcmcia_access_configuration_register);
 
+/*** CSM1200 specified **************/
+int pcmcia_orion_set_cis_rmode(client_handle_t handle)
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->set_cis_rmode();
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_set_cis_rmode);
+
+int pcmcia_orion_set_cis_wmode(client_handle_t handle)
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->set_cis_wmode();
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_set_cis_wmode);
+
+int pcmcia_orion_set_comm_rmode(client_handle_t handle)
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->set_comm_rmode();
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_set_comm_rmode);
+
+int pcmcia_orion_set_comm_wmode(client_handle_t handle)
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->set_comm_wmode();
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_set_comm_wmode);
+
+int pcmcia_orion_set_io_mode(client_handle_t handle)
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->set_io_mode();
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_set_io_mode);
+
+int pcmcia_orion_read_cis(client_handle_t handle, unsigned char *val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	*val = s->ops->read_cis(addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_read_cis);
+
+int pcmcia_orion_write_cis(client_handle_t handle, unsigned char val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->write_cis(val, addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_write_cis);
+
+int pcmcia_orion_read_comm(client_handle_t handle, unsigned char *val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	*val = s->ops->read_comm(addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_read_comm);
+
+int pcmcia_orion_write_comm(client_handle_t handle, unsigned char val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->write_comm(val, addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_write_comm);
+
+int pcmcia_orion_read_io(client_handle_t handle, unsigned char *val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	*val = s->ops->read_io(addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_read_io);
+
+int pcmcia_orion_write_io(client_handle_t handle, unsigned char val, unsigned int addr )
+{
+	struct pcmcia_socket *s;
+	if (CHECK_HANDLE(handle))
+		return CS_BAD_HANDLE;
+	s = SOCKET(handle);
+	s->ops->write_io(val, addr);
+	return 0;
+}
+EXPORT_SYMBOL(pcmcia_orion_write_io);
+
+/*** CSM1200 specified over **************/
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/soc_common.c chipbox-kernel/drivers/pcmcia/soc_common.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/soc_common.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/soc_common.c	2021-12-27 13:08:35.921013083 +0300
@@ -54,6 +54,15 @@
 #include <asm/arch/pxa-regs.h>
 #endif
 
+/*added by xm.chen */
+/*--------------------------------------------------
+* #undef debug(skt, lvl, fmt, arg...)
+* #define debug(skt, lvl, fmt, arg...) do {		\
+* 		printk("soc_comm:%s: " fmt, 	\
+* 		       "0" , ## arg);	\
+* } while (0)
+*--------------------------------------------------*/
+
 #ifdef DEBUG
 
 static int pc_debug;
@@ -70,6 +79,7 @@
 		va_end(args);
 	}
 }
+EXPORT_SYMBOL(soc_pcmcia_debug);
 
 #endif
 
@@ -261,10 +271,21 @@
 {
 	struct soc_pcmcia_socket *skt = dev;
 
+if(irq == 31) //card detect
+    disable_irq(irq);
+
 	debug(skt, 3, "servicing IRQ %d\n", irq);
 
+printk("\nservicing IRQ %d  RAWSTAT:0x%x INTSTAT:0x%x INTENA:0x%x PINCTL:0x%x\n\n", 
+	irq, 
+	ioread16((void*)(VA_PCMCIA_BASE + 0x0108)),
+	ioread16((void*)(VA_PCMCIA_BASE + 0x010c)),
+	ioread16((void*)(VA_PCMCIA_BASE + 0x0110)),
+	ioread16((void*)(VA_PCMCIA_BASE + 0x0114)));
+
 	soc_common_check_status(skt);
 
+//enable_irq(irq);
 	return IRQ_HANDLED;
 }
 
@@ -452,6 +473,7 @@
 
 	map->static_start = res->start + map->card_start;
 
+printk("res->start:0x%x  map->card_start:%d\n",res->start, map->card_start);
 	return 0;
 }
 
@@ -659,7 +681,7 @@
 #define soc_pcmcia_cpufreq_unregister()
 #endif
 
-int soc_common_drv_pcmcia_probe(struct device *dev, struct pcmcia_low_level *ops, int first, int nr)
+int  soc_common_drv_pcmcia_probe(struct device *dev, struct pcmcia_low_level *ops, int first, int nr)
 {
 	struct skt_dev_info *sinfo;
 	struct soc_pcmcia_socket *skt;
@@ -683,6 +705,19 @@
 		skt = &sinfo->skt[i];
 
 		skt->socket.ops = &soc_common_pcmcia_operations;
+#ifdef ORION_PCMCIA
+		skt->socket.ops->set_cis_rmode	= ops->set_cis_rmode;
+		skt->socket.ops->set_cis_wmode	= ops->set_cis_wmode;	
+		skt->socket.ops->set_comm_rmode	= ops->set_comm_rmode;	
+		skt->socket.ops->set_comm_wmode	= ops->set_comm_wmode;	
+		skt->socket.ops->set_io_mode	= ops->set_io_mode;	
+		skt->socket.ops->read_cis	= ops->read_cis;	
+		skt->socket.ops->write_cis	= ops->write_cis;	
+		skt->socket.ops->read_comm	= ops->read_comm;	
+		skt->socket.ops->write_comm	= ops->write_comm;	
+		skt->socket.ops->read_io	= ops->read_io;	
+		skt->socket.ops->write_io	= ops->write_io;	
+#endif
 		skt->socket.owner = ops->owner;
 		skt->socket.dev.dev = dev;
 
@@ -704,6 +739,9 @@
 		ret = request_resource(&iomem_resource, &skt->res_skt);
 		if (ret)
 			goto out_err_1;
+/*--------------------------------------------------
+* printk("\n1xm.chen-0x%x, 0x%x, %s    %s\n",skt->res_skt.start,skt->res_skt.end,skt->res_skt.name,iomem_resource.name);
+*--------------------------------------------------*/
 
 		skt->res_io.start	= _PCMCIAIO(skt->nr);
 		skt->res_io.end		= _PCMCIAIO(skt->nr) + PCMCIAIOSp - 1;
@@ -713,6 +751,9 @@
 		ret = request_resource(&skt->res_skt, &skt->res_io);
 		if (ret)
 			goto out_err_2;
+/*--------------------------------------------------
+* printk("\n2xm.chen-0x%x, 0x%x, %s    %s\n",skt->res_io.start,skt->res_io.end,skt->res_io.name,iomem_resource.name);
+*--------------------------------------------------*/
 
 		skt->res_mem.start	= _PCMCIAMem(skt->nr);
 		skt->res_mem.end	= _PCMCIAMem(skt->nr) + PCMCIAMemSp - 1;
@@ -722,6 +763,9 @@
 		ret = request_resource(&skt->res_skt, &skt->res_mem);
 		if (ret)
 			goto out_err_3;
+/*--------------------------------------------------
+* printk("\n3xm.chen-0x%x, 0x%x, %s    %s\n",skt->res_mem.start,skt->res_mem.end,skt->res_mem.name,iomem_resource.name);
+*--------------------------------------------------*/
 
 		skt->res_attr.start	= _PCMCIAAttr(skt->nr);
 		skt->res_attr.end	= _PCMCIAAttr(skt->nr) + PCMCIAAttrSp - 1;
@@ -731,6 +775,9 @@
 		ret = request_resource(&skt->res_skt, &skt->res_attr);
 		if (ret)
 			goto out_err_4;
+/*--------------------------------------------------
+* printk("\n4xm.chen-0x%x, 0x%x, %s\n",skt->res_attr.start,skt->res_attr.end,skt->res_attr.name);
+*--------------------------------------------------*/
 
 		skt->virt_io = ioremap(skt->res_io.start, 0x10000);
 		if (skt->virt_io == NULL) {
@@ -809,6 +856,7 @@
 	up(&soc_pcmcia_sockets_lock);
 	return ret;
 }
+EXPORT_SYMBOL(soc_common_drv_pcmcia_probe);
 
 int soc_common_drv_pcmcia_remove(struct device *dev)
 {
@@ -848,3 +896,4 @@
 
 	return 0;
 }
+EXPORT_SYMBOL(soc_common_drv_pcmcia_remove);
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/soc_common.h chipbox-kernel/drivers/pcmcia/soc_common.h
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/pcmcia/soc_common.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/pcmcia/soc_common.h	2021-12-27 13:08:35.921013083 +0300
@@ -111,6 +111,22 @@
 	 */
 	int (*frequency_change)(struct soc_pcmcia_socket *, unsigned long, struct cpufreq_freqs *);
 #endif
+
+#ifdef ORION_PCMCIA
+	/*** added by xm.chen ::: only for orion chip ****/
+	void (*set_cis_rmode)(void);	//setting for CIS reading
+	void (*set_cis_wmode)(void);	//setting for CIS writing
+	void (*set_comm_rmode)(void);	//setting for common memory reading
+	void (*set_comm_wmode)(void);	//setting for common memory writing
+	void (*set_io_mode)(void);		//setting for io mode;
+	/*** note: the addr is not virtual addr, but the absolute addr in pc card ****/
+	unsigned char (*read_cis)(unsigned int addr);
+	void (*write_cis)(unsigned char val, unsigned int addr);
+	unsigned char (*read_comm)(unsigned int addr);
+	void (*write_comm)(unsigned char val, unsigned int addr);
+	unsigned char (*read_io)(unsigned int addr);
+	void (*write_io)(unsigned char val, unsigned int addr);
+#endif
 };
 
 
@@ -191,4 +207,23 @@
 #define iostschg bvd1
 #define iospkr   bvd2
 
+/*
+ * Personal Computer Memory Card International Association (PCMCIA) sockets
+ */
+/* 
+ * Actually this is not used. PC Card mem/io is accessed by PALMCHIP core
+ */
+#define PCMCIAPrtSp	0x8000		/* PCMCIA Partition Space [byte]   */
+#define PCMCIASp	(0x800000)	/* PCMCIA Space [byte]             */
+#define PCMCIAIOSp	PCMCIAPrtSp	/* PCMCIA I/O Space [byte]         */
+#define PCMCIAAttrSp	PCMCIAPrtSp	/* PCMCIA Attribute Space [byte]   */
+#define PCMCIAMemSp	PCMCIAPrtSp	/* PCMCIA Memory Space [byte]      */
+
+
+#define _PCMCIA(Nb)		(0x35700000) /* not really */
+#define _PCMCIAIO(Nb)		_PCMCIA(Nb)
+#define _PCMCIAAttr(Nb) 	(0x35800000)
+#define _PCMCIAMem(Nb)	        (0x35A00000)
+
+
 #endif
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/8250.c chipbox-kernel/drivers/serial/8250.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/8250.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/serial/8250.c	2021-12-27 13:08:36.021014600 +0300
@@ -288,6 +288,10 @@
 static _INLINE_ void
 serial_out(struct uart_8250_port *up, int offset, int value)
 {
+#ifdef CONFIG_SERIAL_8250_ORION
+	if(offset == UART_LCR)
+		up->lcr = value;
+#endif
 	offset <<= up->port.regshift;
 
 	switch (up->port.iotype) {
@@ -1245,6 +1249,7 @@
 		up = list_entry(l, struct uart_8250_port, list);
 
 		iir = serial_in(up, UART_IIR);
+
 		if (!(iir & UART_IIR_NO_INT)) {
 			spin_lock(&up->port.lock);
 			serial8250_handle_port(up, regs);
@@ -1253,6 +1258,15 @@
 			handled = 1;
 
 			end = NULL;
+#ifdef CONFIG_SERIAL_8250_ORION
+		} else if((iir & 0x0f) == 7) {
+		        while(serial_in(up, 0x1f) & 0x1); 	/* USR[0]: transfer busy */
+			serial_out(up, UART_LCR, up->lcr);	/* Re-write failed LCR */
+
+			handled = 1;
+
+			end = NULL;
+#endif
 		} else if (end == NULL)
 			end = l;
 
@@ -1817,7 +1831,11 @@
  */
 static int serial8250_request_std_resource(struct uart_8250_port *up)
 {
+#ifdef CONFIG_SERIAL_8250_ORION
+	unsigned int size = 0x20 << up->port.regshift;
+#else
 	unsigned int size = 8 << up->port.regshift;
+#endif
 	int ret = 0;
 
 	switch (up->port.iotype) {
@@ -2237,7 +2255,11 @@
 static struct uart_driver serial8250_reg = {
 	.owner			= THIS_MODULE,
 	.driver_name		= "serial",
+#ifdef CONFIG_SERIAL_8250_ORION
+	.devfs_name		= "ttyS",
+#else
 	.devfs_name		= "tts/",
+#endif
 	.dev_name		= "ttyS",
 	.major			= TTY_MAJOR,
 	.minor			= 64,
Only in chipbox-kernel/drivers/serial: 8250_orion.c
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/Kconfig chipbox-kernel/drivers/serial/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/serial/Kconfig	2021-12-27 13:08:36.021014600 +0300
@@ -156,6 +156,12 @@
 	help
 	  ::: To be written :::
 
+config SERIAL_8250_ORION
+	bool "Support ORION serial ports"
+	depends on SERIAL_8250
+	help
+	  Support UART on ORION SOC Platform.
+
 comment "Non-8250 serial port support"
 
 config SERIAL_8250_ACORN
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/Makefile chipbox-kernel/drivers/serial/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/serial/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/serial/Makefile	2021-12-27 13:08:36.021014600 +0300
@@ -13,9 +13,11 @@
 
 obj-$(CONFIG_SERIAL_CORE) += serial_core.o
 obj-$(CONFIG_SERIAL_21285) += 21285.o
+obj-$(CONFIG_SERIAL_VIRGO) += serial_virgo.o
 obj-$(CONFIG_SERIAL_8250) += 8250.o $(serial-8250-y)
 obj-$(CONFIG_SERIAL_8250_CS) += serial_cs.o
 obj-$(CONFIG_SERIAL_8250_ACORN) += 8250_acorn.o
+obj-$(CONFIG_SERIAL_8250_ORION) += 8250_orion.o
 obj-$(CONFIG_SERIAL_8250_CONSOLE) += 8250_early.o
 obj-$(CONFIG_SERIAL_AMBA_PL010) += amba-pl010.o
 obj-$(CONFIG_SERIAL_AMBA_PL011) += amba-pl011.o
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/hcd.c chipbox-kernel/drivers/usb/core/hcd.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/hcd.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/core/hcd.c	2021-12-27 13:08:36.037014842 +0300
@@ -758,7 +758,7 @@
  * Assigns a bus number, and links the controller into usbcore data
  * structures so that it can be seen by scanning the bus list.
  */
-static int usb_register_bus(struct usb_bus *bus)
+int usb_register_bus(struct usb_bus *bus)
 {
 	int busnum;
 	int retval;
@@ -802,7 +802,7 @@
  * Recycles the bus number, and unlinks the controller from usbcore data
  * structures so that it won't be seen by scanning the bus list.
  */
-static void usb_deregister_bus (struct usb_bus *bus)
+void usb_deregister_bus (struct usb_bus *bus)
 {
 	dev_info (bus->controller, "USB bus %d deregistered\n", bus->busnum);
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/hub.c chipbox-kernel/drivers/usb/core/hub.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/hub.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/core/hub.c	2021-12-27 13:08:36.037014842 +0300
@@ -291,6 +291,7 @@
 
 void usb_kick_khubd(struct usb_device *hdev)
 {
+	printk("KB usb_kick_khubd\n");
 	kick_khubd(hdev_to_hub(hdev));
 }
 
@@ -330,6 +331,8 @@
 	hub->nerrors = 0;
 
 	/* Something happened, let khubd figure it out */
+	// printk("KB hub_irq\n");
+
 	kick_khubd(hub);
 
 resubmit:
@@ -472,6 +475,7 @@
 		schedule_delayed_work(&hub->leds, LED_CYCLE_PERIOD);
 
 	/* scan all ports ASAP */
+	printk("KB hub_activate\n");
 	kick_khubd(hub);
 }
 
@@ -1459,6 +1463,7 @@
 	 */
 
 	set_bit(port1, hub->change_bits);
+	printk("KB hub_port_logical_disconnect\n");
  	kick_khubd(hub);
 }
 
@@ -2113,6 +2118,7 @@
 	unsigned		delay = HUB_SHORT_RESET_TIME;
 	enum usb_device_speed	oldspeed = udev->speed;
 
+	printk("hub_port_init : \n");
 	/* root hub ports have a slightly longer reset period
 	 * (from USB 2.0 spec, section 7.1.7.5)
 	 */
@@ -2410,6 +2416,12 @@
 	struct usb_device *hdev = hub->hdev;
 	struct device *hub_dev = hub->intfdev;
 	int status, i;
+
+	/* By KB */
+	/*
+	printk("hub_port_connect_change : port %d, status %04x, change %04x\n",
+		port1, portstatus, portchange);
+		*/
  
 	dev_dbg (hub_dev,
 		"port %d, status %04x, change %04x, %s\n",
@@ -2612,6 +2624,7 @@
 	while (1) {
 
 		/* Grab the first entry at the beginning of the list */
+		// printk("hub_events : 1\n");
 		spin_lock_irq(&hub_event_lock);
 		if (list_empty(&hub_event_list)) {
 			spin_unlock_irq(&hub_event_lock);
@@ -2640,7 +2653,9 @@
 
 		/* Is this is a root hub wanting to be resumed? */
 		if (i)
+		{
 			usb_resume_device(hdev);
+		}
 
 		/* Lock the device, then check to see if we were
 		 * disconnected while waiting for the lock to succeed. */
@@ -2951,6 +2966,7 @@
 	struct usb_hub			*hub = NULL;
 	int 				i, ret = 0, port1 = -1;
 
+	printk("usb_reset_device : device state %d\n", udev->state);
 	if (udev->state == USB_STATE_NOTATTACHED ||
 			udev->state == USB_STATE_SUSPENDED) {
 		dev_dbg(&udev->dev, "device reset not allowed in state %d\n",
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/message.c chipbox-kernel/drivers/usb/core/message.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/core/message.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/core/message.c	2021-12-27 13:08:36.037014842 +0300
@@ -225,7 +225,7 @@
 static void sg_complete (struct urb *urb, struct pt_regs *regs)
 {
 	struct usb_sg_request	*io = (struct usb_sg_request *) urb->context;
-
+//	printk("complete!!\n");
 	spin_lock (&io->lock);
 
 	/* In 2.5 we require hcds' endpoint queues not to progress after fault
@@ -265,6 +265,7 @@
 			if (!io->urbs [i] || !io->urbs [i]->dev)
 				continue;
 			if (found) {
+				printk("\n \n unlink urb %d \n\n",i);
 				status = usb_unlink_urb (io->urbs [i]);
 				if (status != -EINPROGRESS && status != -EBUSY)
 					dev_err (&io->dev->dev,
@@ -390,6 +391,7 @@
 			io->urbs [i]->transfer_buffer =
 				page_address (sg [i].page) + sg [i].offset;
 			len = sg [i].length;
+	//		printk("scatter_list[%d],address=0x%08x,len=%d\n" , i , io->urbs [i]->transfer_buffer ,len);
 		}
 
 		if (length) {
@@ -464,7 +466,7 @@
 
 		io->urbs [i]->dev = io->dev;
 		retval = usb_submit_urb (io->urbs [i], SLAB_ATOMIC);
-
+//		printk("%s:submit urb[%d]\n",__FUNCTION__,i);
 		/* after we submit, let completions or cancelations fire;
 		 * we handshake using io->status.
 		 */
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/Kconfig chipbox-kernel/drivers/usb/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/Kconfig	2021-12-27 13:08:36.033014784 +0300
@@ -123,5 +123,7 @@
 
 source "drivers/usb/gadget/Kconfig"
 
+source "drivers/usb/musb/Kconfig"
+
 endmenu
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/Makefile chipbox-kernel/drivers/usb/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/Makefile	2021-12-27 13:08:36.033014784 +0300
@@ -72,3 +72,4 @@
 
 obj-$(CONFIG_USB_ATM)		+= atm/
 obj-$(CONFIG_USB_SPEEDTOUCH)	+= atm/
+obj-$(CONFIG_USB_INVENTRA_HCD)	+= musb/
Only in chipbox-kernel/drivers/usb: musb
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/storage/transport.c chipbox-kernel/drivers/usb/storage/transport.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/storage/transport.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/storage/transport.c	2021-12-27 13:08:36.085015570 +0300
@@ -508,6 +508,9 @@
 		/* no scatter-gather, just make the request */
 		result = usb_stor_bulk_transfer_buf(us, pipe, buf, 
 				length_left, &partial);
+		if(us->srb->cmnd[0] == MODE_SENSE){				
+				printk("===========> MODE_SENSE data(%d): 0x%x,0x%x,0x%x,0x%x\n",length_left,*((char *)buf+0),*((char *)buf+1),*((char *)buf+2),*((char *)buf+3));
+		}
 		length_left -= partial;
 	}
 
@@ -758,7 +761,7 @@
 	unsigned int transfer_length = srb->request_bufflen;
 	unsigned int pipe = 0;
 	int result;
-
+US_DEBUGP("%s :transfer_length=%d\n", __FUNCTION__,transfer_length);
 	/* COMMAND STAGE */
 	/* let's send the command via the control pipe */
 	result = usb_stor_ctrl_transfer(us, us->send_ctrl_pipe,
@@ -857,7 +860,7 @@
 {
 	unsigned int transfer_length = srb->request_bufflen;
 	int result;
-
+US_DEBUGP("%s :transfer_length=%d\n", __FUNCTION__,transfer_length);
 	/* COMMAND STAGE */
 	/* let's send the command via the control pipe */
 	result = usb_stor_ctrl_transfer(us, us->send_ctrl_pipe,
@@ -956,7 +959,7 @@
 	int fake_sense = 0;
 	unsigned int cswlen;
 	unsigned int cbwlen = US_BULK_CB_WRAP_LEN;
-
+US_DEBUGP("%s :transfer_length=%d\n", __FUNCTION__,transfer_length);
 	/* Take care of BULK32 devices; set extra byte to 0 */
 	if ( unlikely(us->flags & US_FL_BULK32)) {
 		cbwlen = 32;
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/storage/usb.c chipbox-kernel/drivers/usb/storage/usb.c
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/usb/storage/usb.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/usb/storage/usb.c	2021-12-27 13:08:36.085015570 +0300
@@ -371,6 +371,13 @@
 		/* we've got a command, let's do it! */
 		else {
 			US_DEBUG(usb_stor_show_command(us->srb));
+			
+			if(us->srb->cmnd[0] == MODE_SENSE){
+				us->srb->cmnd[4] = 0x4;
+				us->srb->request_bufflen = 0x4;
+				printk("===========> MODE_SENSE modify: cmnd[4] = 0x4 & request_bufflen = 0x4\n");
+			}
+			
 			us->proto_handler(us->srb, us);
 		}
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/video/Kconfig chipbox-kernel/drivers/video/Kconfig
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/video/Kconfig	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/video/Kconfig	2021-12-27 13:08:36.089015632 +0300
@@ -1468,6 +1468,28 @@
 	  working with S1D13806). Product specs at
 	  <http://www.erd.epson.com/vdc/html/legacy_13xxx.htm>
 
+config FB_ORION
+	tristate "ORION Frame Buffer support"
+	depends on FB
+	select FB_CFB_FILLRECT
+	select FB_CFB_COPYAREA
+	select FB_CFB_IMAGEBLIT
+	select FB_SOFT_CURSOR
+	---help---
+	  Support for ORION platform framebuffer device.
+
+	  To compile this driver as a module, choose M here: the
+	  module will be called orionfb.
+
+config FB_ORION_BLIT
+	tristate "ORION Blitter(2D Engine) support"
+	depends on FB
+	---help---
+	  Support for ORION platform blitter device.
+
+	  To compile this driver as a module, choose M here: the
+	  module will be called orionblit.
+
 config FB_VIRTUAL
 	tristate "Virtual Frame Buffer support (ONLY FOR TESTING!)"
 	depends on FB
diff -ur mainline-2.6.12.5/linux-2.6.12.5/drivers/video/Makefile chipbox-kernel/drivers/video/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/drivers/video/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/drivers/video/Makefile	2021-12-27 13:08:36.089015632 +0300
@@ -91,6 +91,8 @@
 obj-$(CONFIG_FB_TX3912)		  += tx3912fb.o
 obj-$(CONFIG_FB_S1D13XXX)	  += s1d13xxxfb.o
 obj-$(CONFIG_FB_IMX)              += imxfb.o
+obj-$(CONFIG_FB_ORION)		  += orionfb/
+obj-$(CONFIG_FB_ORION_BLIT)	  += orionblit/
 
 # Platform or fallback drivers go here
 obj-$(CONFIG_FB_VESA)             += vesafb.o
Only in chipbox-kernel/drivers/video: orionblit
Only in chipbox-kernel/drivers/video: orionfb
diff -ur mainline-2.6.12.5/linux-2.6.12.5/fs/jffs2/wbuf.c chipbox-kernel/fs/jffs2/wbuf.c
--- mainline-2.6.12.5/linux-2.6.12.5/fs/jffs2/wbuf.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/fs/jffs2/wbuf.c	2021-12-28 00:16:12.444397313 +0300
@@ -578,8 +578,12 @@
 {
 	int ret;
 
+	if (!c->wbuf) return 0;
+	
 	down_write(&c->wbuf_sem);
 	ret = __jffs2_flush_wbuf(c, PAD_NOACCOUNT);
+	if (ret)
+		ret= __jffs2_flush_wbuf(c, PAD_NOACCOUNT);
 	up_write(&c->wbuf_sem);
 
 	return ret;
diff -ur mainline-2.6.12.5/linux-2.6.12.5/fs/sysfs/inode.c chipbox-kernel/fs/sysfs/inode.c
--- mainline-2.6.12.5/linux-2.6.12.5/fs/sysfs/inode.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/fs/sysfs/inode.c	2021-12-27 13:08:36.277018486 +0300
@@ -146,7 +146,14 @@
 void sysfs_hash_and_remove(struct dentry * dir, const char * name)
 {
 	struct sysfs_dirent * sd;
-	struct sysfs_dirent * parent_sd = dir->d_fsdata;
+	//struct sysfs_dirent * parent_sd = dir->d_fsdata;;
+	struct sysfs_dirent * parent_sd;
+
+	if (!dir)
+ 		/* no inode means this hasn't been made visible yet ------ modified by bin.sun@celestialsemi.cn 2008.07.23 */
+ 		return;
+ 
+	parent_sd = dir->d_fsdata;
 
 	down(&dir->d_inode->i_sem);
 	list_for_each_entry(sd, &parent_sd->s_children, s_sibling) {
Only in chipbox-kernel/: .git
Only in chipbox-kernel/: .gitignore
Only in chipbox-kernel/: images
Only in chipbox-kernel/include: asm
Only in chipbox-kernel/include/asm-arm: arch-orion
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/asm-arm/cpu-single.h chipbox-kernel/include/asm-arm/cpu-single.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/asm-arm/cpu-single.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/asm-arm/cpu-single.h	2021-12-27 13:08:36.389020188 +0300
@@ -41,4 +41,4 @@
 extern void cpu_dcache_clean_area(void *, int);
 extern void cpu_do_switch_mm(unsigned long pgd_phys, struct mm_struct *mm);
 extern void cpu_set_pte(pte_t *ptep, pte_t pte);
-extern volatile void cpu_reset(unsigned long addr);
+extern void cpu_reset(unsigned long addr) __attribute__((noreturn));
Only in chipbox-kernel/include/asm-arm/hardware: ns16550.h
Only in mainline-2.6.12.5/linux-2.6.12.5/include/asm-frv: dm9000.h
Only in chipbox-kernel/include: config
Only in chipbox-kernel/include/linux: autoconf.h
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/linux/device.h chipbox-kernel/include/linux/device.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/linux/device.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/linux/device.h	2021-12-27 13:08:36.877027593 +0300
@@ -401,6 +401,9 @@
 #define dev_printk(level, dev, format, arg...)	\
 	printk(level "%s %s: " format , (dev)->driver ? (dev)->driver->name : "" , (dev)->bus_id , ## arg)
 
+#define dev_kb(dev, format, arg...)		\
+	dev_printk(KERN_DEBUG , dev , format , ## arg)
+	
 #ifdef DEBUG
 #define dev_dbg(dev, format, arg...)		\
 	dev_printk(KERN_DEBUG , dev , format , ## arg)
Only in chipbox-kernel/include/linux: dm9000.h
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/linux/ide.h chipbox-kernel/include/linux/ide.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/linux/ide.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/linux/ide.h	2021-12-27 13:08:36.893027836 +0300
@@ -218,7 +218,7 @@
 		ide_rz1000,	ide_trm290,
 		ide_cmd646,	ide_cy82c693,	ide_4drives,
 		ide_pmac,	ide_etrax100,	ide_acorn,
-		ide_forced
+		ide_forced,	ide_palm3710
 } hwif_chipset_t;
 
 /*
Only in chipbox-kernel/include/linux: mem_define.h
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/linux/mtd/mtd.h chipbox-kernel/include/linux/mtd/mtd.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/linux/mtd/mtd.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/linux/mtd/mtd.h	2021-12-27 13:08:36.909028081 +0300
@@ -24,7 +24,7 @@
 
 #define MTD_CHAR_MAJOR 90
 #define MTD_BLOCK_MAJOR 31
-#define MAX_MTD_DEVICES 16
+#define MAX_MTD_DEVICES 32
 
 #define MTD_ERASE_PENDING      	0x01
 #define MTD_ERASING		0x02
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/linux/mtd/nand.h chipbox-kernel/include/linux/mtd/nand.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/linux/mtd/nand.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/linux/mtd/nand.h	2021-12-27 13:08:36.909028081 +0300
@@ -349,6 +349,7 @@
 #define NAND_MFR_NATIONAL	0x8f
 #define NAND_MFR_RENESAS	0x07
 #define NAND_MFR_STMICRO	0x20
+#define NAND_MFR_HYNIX          0xad
 
 /**
  * struct nand_flash_dev - NAND Flash Device ID Structure
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/linux/netlink.h chipbox-kernel/include/linux/netlink.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/linux/netlink.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/linux/netlink.h	2021-12-27 13:08:36.925028322 +0300
@@ -19,6 +19,7 @@
 #define NETLINK_DNRTMSG		14	/* DECnet routing messages */
 #define NETLINK_KOBJECT_UEVENT	15	/* Kernel messages to userspace */
 #define NETLINK_TAPBASE		16	/* 16 to 31 are ethertap */
+#define NETLINK_TVE_CHANGE	17	/* just for Orion 1.x platforms */
 
 #define MAX_LINKS 32		
 
Only in chipbox-kernel/include/linux: platform.h.bak_device
Only in chipbox-kernel/include/linux: version.h
diff -ur mainline-2.6.12.5/linux-2.6.12.5/include/pcmcia/ss.h chipbox-kernel/include/pcmcia/ss.h
--- mainline-2.6.12.5/linux-2.6.12.5/include/pcmcia/ss.h	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/include/pcmcia/ss.h	2021-12-27 13:08:36.981029171 +0300
@@ -20,6 +20,8 @@
 #include <pcmcia/bulkmem.h>
 #include <linux/device.h>
 
+#define ORION_PCMCIA		//Celestial ORION CSM1200 feature
+
 /* Definitions for card status flags for GetStatus */
 #define SS_WRPROT	0x0001
 #define SS_CARDLOCK	0x0002
@@ -113,6 +115,22 @@
 	int (*set_socket)(struct pcmcia_socket *sock, socket_state_t *state);
 	int (*set_io_map)(struct pcmcia_socket *sock, struct pccard_io_map *io);
 	int (*set_mem_map)(struct pcmcia_socket *sock, struct pccard_mem_map *mem);
+
+#ifdef ORION_PCMCIA
+	/*** added by xm.chen ::: only for orion chip ****/
+	void (*set_cis_rmode)(void);	//setting for CIS reading
+	void (*set_cis_wmode)(void);	//setting for CIS writing
+	void (*set_comm_rmode)(void);	//setting for common memory reading
+	void (*set_comm_wmode)(void);	//setting for common memory writing
+	void (*set_io_mode)(void);		//setting for io mode;
+	/*** note: the addr is not virtual addr, but the absolute addr in pc card ****/
+	unsigned char (*read_cis)(unsigned int addr);
+	void (*write_cis)(unsigned char val, unsigned int addr);
+	unsigned char (*read_comm)(unsigned int addr);
+	void (*write_comm)(unsigned char val, unsigned int addr);
+	unsigned char (*read_io)(unsigned int addr);
+	void (*write_io)(unsigned char val, unsigned int addr);
+#endif
 };
 
 struct pccard_resource_ops {
diff -ur mainline-2.6.12.5/linux-2.6.12.5/init/main.c chipbox-kernel/init/main.c
--- mainline-2.6.12.5/linux-2.6.12.5/init/main.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/init/main.c	2021-12-27 13:08:36.997029417 +0300
@@ -480,6 +480,8 @@
 	profile_init();
 	local_irq_enable();
 #ifdef CONFIG_BLK_DEV_INITRD
+//	extern unsigned int phys_initrd_start, phys_initrd_size;
+	
 	if (initrd_start && !initrd_below_start_ok &&
 			initrd_start < min_low_pfn << PAGE_SHIFT) {
 		printk(KERN_CRIT "initrd overwritten (0x%08lx < 0x%08lx) - "
@@ -693,7 +695,9 @@
 
 	(void) sys_dup(0);
 	(void) sys_dup(0);
-	
+//#define TEST_STR	"Output via console driver!\n"
+//	sys_write(0,TEST_STR,strlen(TEST_STR));
+//	run_init_process("/bin/sh");	
 	/*
 	 * We try each of these until one succeeds.
 	 *
diff -ur mainline-2.6.12.5/linux-2.6.12.5/kernel/printk.c chipbox-kernel/kernel/printk.c
--- mainline-2.6.12.5/linux-2.6.12.5/kernel/printk.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/kernel/printk.c	2021-12-27 13:08:37.009029600 +0300
@@ -439,8 +439,43 @@
 	_call_console_drivers(start_print, end, msg_level);
 }
 
+/*#define	VIRGO_EARLY_PRINTK*/
+#ifdef	VIRGO_EARLY_PRINTK
+static int virgo_io_mapped = 1;
+static void UART_putchar(char c)
+{
+	if(c=='\n'){
+		*(volatile unsigned char *)0xf11f1000='\r';
+		while(((*(volatile unsigned char*)0xf11f1014) & 0x20) == 0);
+	}
+	*(volatile unsigned char *)0xf11f1000=c;
+	while(((*(volatile unsigned char*)0xf11f1014) & 0x20) == 0);
+	return;
+}
+#endif
+
 static void emit_log_char(char c)
 {
+#ifdef	VIRGO_EARLY_PRINTK	
+	int i;
+	static char buf[2000];
+	static int bufptr=1;
+	//DXL: early print
+	if(virgo_io_mapped){
+		if(bufptr>0){
+			UART_putchar('+');
+			for(i=1;i<bufptr;i++){
+				UART_putchar(buf[bufptr]);
+			}
+			bufptr=0;
+		}
+		UART_putchar(c);
+	}else{
+		buf[bufptr++]=c;
+		return;
+	}
+#endif
+
 	LOG_BUF(log_end) = c;
 	log_end++;
 	if (log_end - log_start > log_buf_len)
@@ -449,6 +484,7 @@
 		con_start = log_end - log_buf_len;
 	if (logged_chars < log_buf_len)
 		logged_chars++;
+
 }
 
 /*
@@ -871,6 +907,7 @@
 			continue;
 		if (console->index < 0)
 			console->index = console_cmdline[i].index;
+
 		if (console->setup &&
 		    console->setup(console, console_cmdline[i].options) != 0)
 			break;
diff -ur mainline-2.6.12.5/linux-2.6.12.5/lib/kobject.c chipbox-kernel/lib/kobject.c
--- mainline-2.6.12.5/linux-2.6.12.5/lib/kobject.c	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/lib/kobject.c	2021-12-27 13:08:37.021029779 +0300
@@ -71,9 +71,12 @@
 	 * Add 1 to strlen for leading '/' of each level.
 	 */
 	do {
-		length += strlen(kobject_name(parent)) + 1;
-		parent = parent->parent;
+			if (kobject_name(parent) == NULL)		/* modified by sunbin 20090427*/
+				return 0;
+			length += strlen(kobject_name(parent)) + 1;
+			parent = parent->parent;
 	} while (parent);
+
 	return length;
 }
 
diff -ur mainline-2.6.12.5/linux-2.6.12.5/Makefile chipbox-kernel/Makefile
--- mainline-2.6.12.5/linux-2.6.12.5/Makefile	2005-08-15 03:20:18.000000000 +0300
+++ chipbox-kernel/Makefile	2021-12-27 13:08:34.840996680 +0300
@@ -190,8 +190,12 @@
 # Default value for CROSS_COMPILE is not to prefix executables
 # Note: Some architectures assign CROSS_COMPILE in their arch/*/Makefile
 
-ARCH		?= $(SUBARCH)
-CROSS_COMPILE	?=
+#ARCH		?= $(SUBARCH)
+#CROSS_COMPILE	?=
+ARCH		?= arm
+CROSS_COMPILE	?= arm-linux-
+#arm-linux-
+
 
 # Architecture as present in compile.h
 UTS_MACHINE := $(ARCH)
@@ -727,6 +731,9 @@
 # vmlinux image - including updated kernel symbols
 vmlinux: $(vmlinux-lds) $(vmlinux-init) $(vmlinux-main) $(kallsyms.o) FORCE
 	$(call if_changed_rule,vmlinux__)
+#	$(CROSS_COMPILE)objcopy --change-address -0xc0000000 vmlinux vmlinux.elf
+#	cp vmlinux.elf vmlinux
+#	$(CROSS_COMPILE)strip vmlinux.elf
 
 # The actual objects are generated when descending, 
 # make sure no implicit rule kicks in
@@ -747,7 +754,7 @@
 # A multi level approach is used. prepare1 is updated first, then prepare0.
 # prepare-all is the collection point for the prepare targets.
 
-.PHONY: prepare-all prepare prepare0 prepare1 prepare2
+.PHONY: prepare-all prepare prepare0 prepare1 prepare2 av_layout
 
 # prepare2 is used to check if we are building in a separate output directory,
 # and if so do:
@@ -776,7 +783,7 @@
 endif
 
 # All the preparing..
-prepare-all: prepare0 prepare
+prepare-all:  prepare0 prepare av_layout 
 
 #	Leave this as default for preprocessing vmlinux.lds.S, which is now
 #	done in arch/$(ARCH)/kernel/Makefile
@@ -1343,3 +1350,202 @@
 endif	# skip-makefile
 
 FORCE:
+
+# Scripts to generate include/linux/mem_define.h
+# ---------------------------------------------------------------------------
+av_layout:
+
+	@echo "#ifndef __MEM_DEFINE_H__"  > ${obj}/include/linux/mem_define.h
+	@echo "#define __MEM_DEFINE_H__"  >> ${obj}/include/linux/mem_define.h
+	@echo ""  >> ${obj}/include/linux/mem_define.h 
+	@grep "CONFIG_BASE_DDR_ADDR" ${obj}/include/linux/autoconf.h -A 21 | sed -e "s/CONFIG_//g" | sed -e "s/\"//g" >> ${obj}/include/linux/mem_define.h
+
+	@##
+	@##
+	@# WARNING: the following definitions should be deleted 
+	@echo "#define ETH_SIZE 	ETHERNET_SIZE" >> ${obj}/include/linux/mem_define.h
+	@echo "#define ETH_REGION 	ETHERNET_REGION" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define FB_SIZE 	FB0_SIZE" >> ${obj}/include/linux/mem_define.h
+	@echo "#define FB_REGION 	FB0_REGION" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+	@echo "#if (FB2_SIZE > FB0_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "#error \"ERROR: FB2 has configured large than FB0!\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#elif (FB3_SIZE > FB1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "#error \"ERROR: FB3 has configured large than FB1!\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#endif" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define DPB1_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define DPB1_REGION 	(CPB0_DIR_REGION - DPB1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define CPB1_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define CPB1_REGION 	(DPB1_REGION - CPB1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define CPB1_DIR_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define CPB1_DIR_REGION	(CPB1_REGION - CPB1_DIR_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define PPB0_SIZE 	0x001a0000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define PPB0_SIZE 	0x001a0000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define PPB0_SIZE 	0x00600000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define PPB0_REGION 	(CPB1_DIR_REGION - PPB0_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+
+	@echo "#define PPB1_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define PPB1_REGION 	(PPB0_REGION - PPB1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define IPREDLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define IPREDLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define IPREDLB_SIZE 	0x00020000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define IPREDLB_REGION 	(PPB1_REGION - IPREDLB_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define MVLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define MVLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define MVLB_SIZE 	0x00020000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define MVLB_REGION 	(IPREDLB_REGION - MVLB_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define MVBUF0_SIZE 	0x00240000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define MVBUF0_SIZE 	0x00240000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define MVBUF0_SIZE 	0x00480000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define MVBUF0_REGION 	(MVLB_REGION - MVBUF0_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define MVBUF1_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define MVBUF1_REGION 	(MVBUF0_REGION - MVBUF1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define FWBUF0_SIZE 	0x00110000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define FWBUF0_SIZE 	0x00110000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define FWBUF0_SIZE 	0x00400000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define FWBUF0_REGION 	(MVBUF1_REGION - FWBUF0_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define FWBUF1_SIZE 	0x00000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define FWBUF1_REGION 	(FWBUF0_REGION - FWBUF1_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define VIDMIPS_SIZE 	0x0040000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define VIDMIPS_SIZE 	0x0040000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define VIDMIPS_SIZE 	0x00100000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define VIDMIPS_REGION 	(FWBUF1_REGION - VIDMIPS_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define LFLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#define LFLB_SIZE 	0x00010000" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define LFLB_SIZE 	0x00020000" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+	@echo "#define LFLB_REGION 	(VIDMIPS_REGION - LFLB_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+
+	@echo "#define CAB_SIZE 	0x0018000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define CAB_REGION 	(LFLB_REGION - CAB_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo "#define AUD_PTS_SIZE 	0x0003000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define AUD_PTS_REGION 	(CAB_REGION - AUD_PTS_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+	
+	@echo "#define VIDEO_USER_DATA_SIZE 	0x00080000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define VIDEO_USER_DATA_REGION 	(AUD_PTS_REGION - VIDEO_USER_DATA_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+ifdef CONFIG_ARCH_ORION_CSM1200
+ifdef CONFIG_ARCH_ORION_CSM1200_J
+	@echo "#if VIDEO_USER_DATA_REGION < 0x02100000" >> ${obj}/include/linux/mem_define.h
+	@echo "#warning \"memory assigned overlap with debug address region, that might be cause unexpected over-written\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#endif" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#if VIDEO_USER_DATA_REGION < 0x04000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#warning \"memory assigned overlap with debug address region, that might be cause unexpected over-written\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#endif" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+endif
+endif
+
+ifdef CONFIG_ARCH_ORION_CSM1201	
+ifdef CONFIG_ARCH_ORION_CSM1201_J
+	@echo "#define HD2SD_DATA_SIZE 	0x0000000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define HD2SD_DATA_REGION 	(VIDEO_USER_DATA_REGION - HD2SD_DATA_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+	
+
+	@echo "#if HD2SD_DATA_REGION < 0x02100000" >> ${obj}/include/linux/mem_define.h
+	@echo "#warning \"memory assigned overlap with debug address region, that might be cause unexpected over-written\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#endif" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+else
+	@echo "#define HD2SD_DATA_SIZE 	0x005A0000" >> ${obj}/include/linux/mem_define.h
+	@echo "#define HD2SD_DATA_REGION 	(VIDEO_USER_DATA_REGION - HD2SD_DATA_SIZE)" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+	
+	@echo "#if HD2SD_DATA_REGION < 0x02900000" >> ${obj}/include/linux/mem_define.h
+	@echo "#warning \"memory assigned overlap with debug address region, that might be cause unexpected over-written\"" >> ${obj}/include/linux/mem_define.h
+	@echo "#endif" >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+endif
+
+endif
+	@# WARNING: the above definitions should be deleted 
+	@##
+	@##
+
+	@echo "#endif"  >> ${obj}/include/linux/mem_define.h
+	@echo "" >> ${obj}/include/linux/mem_define.h
+
+	@echo ""
+	@echo "  a A/V resources configuration file was generated, and located at ${obj}/include/linux/mem_define.h"
+	@echo ""
+ 
Only in chipbox-kernel/: Module.symvers
Only in chipbox-kernel/: README.md
Only in chipbox-kernel/scripts/basic: docproc
Only in chipbox-kernel/scripts/basic: .docproc.cmd
Only in chipbox-kernel/scripts/basic: fixdep
Only in chipbox-kernel/scripts/basic: .fixdep.cmd
Only in chipbox-kernel/scripts/basic: split-include
Only in chipbox-kernel/scripts/basic: .split-include.cmd
Only in chipbox-kernel/scripts: bin2c
Only in chipbox-kernel/scripts: conmakehash
Only in chipbox-kernel/scripts: kallsyms
Only in chipbox-kernel/scripts/kconfig: conf
Only in chipbox-kernel/scripts/kconfig: .conf.cmd
Only in chipbox-kernel/scripts/kconfig: conf.o
Only in chipbox-kernel/scripts/kconfig: .conf.o.cmd
Only in chipbox-kernel/scripts/kconfig: kxgettext.o
Only in chipbox-kernel/scripts/kconfig: .kxgettext.o.cmd
Only in chipbox-kernel/scripts/kconfig: lex.zconf.c
Only in chipbox-kernel/scripts/kconfig: mconf
Only in chipbox-kernel/scripts/kconfig: mconf.o
Only in chipbox-kernel/scripts/kconfig: .mconf.o.cmd
Only in chipbox-kernel/scripts/kconfig: zconf.tab.c
Only in chipbox-kernel/scripts/kconfig: zconf.tab.h
Only in chipbox-kernel/scripts/kconfig: zconf.tab.o
Only in chipbox-kernel/scripts/kconfig: .zconf.tab.o.cmd
Only in chipbox-kernel/scripts/lxdialog: lxdialog
Only in chipbox-kernel/scripts/mod: elfconfig.h
Only in chipbox-kernel/scripts/mod: mk_elfconfig
Only in chipbox-kernel/scripts/mod: modpost
Only in chipbox-kernel/: tmp
Only in chipbox-kernel/: .version
```
