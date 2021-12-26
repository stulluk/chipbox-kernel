/*
 *	Serial Device Initialisation for Lasi/Asp/Wax/Dino
 *
 *	(c) Copyright Matthew Wilcox <willy@debian.org> 2001-2002
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/serial_core.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/device.h>

#include <asm/hardware.h>
#include <asm/io.h>


#include "8250.h"



#define UART0_MEM	0x101F1000
#define UART1_MEM	0x101F2000
#define UART0_IRQ	12
#define UART1_IRQ	13

static int serial_line[2];
static struct platform_device *orion_serial_dev;

static int __init serial_orion_init(void)
{
	struct uart_port port;

	orion_serial_dev = platform_device_register_simple("serial_orion",
                                                            -1, NULL, 0);

	memset(&port, 0, sizeof(struct uart_port));

	port.mapbase = UART0_MEM;
	port.irq = UART0_IRQ;
	port.iotype = UPIO_MEM;
	port.flags = UPF_IOREMAP | UPF_BOOT_AUTOCONF;
	port.uartclk = PCLK_FREQ;
	port.regshift = 2;
	port.dev = &orion_serial_dev->dev;

	serial_line[0] = serial8250_register_port(&port);
	if (serial_line[0] < 0) {
		printk(KERN_WARNING "UART0: serial8250_register_port returned error %d\n", serial_line[0]);
		return serial_line[0];
	}
#ifndef  CONFIG_CS_SERIAL_RC
	port.mapbase = UART1_MEM;
	port.irq = UART1_IRQ;
	port.iotype = UPIO_MEM;
	port.flags = UPF_IOREMAP | UPF_BOOT_AUTOCONF;
	port.uartclk = PCLK_FREQ;
	port.regshift = 2;
	port.dev = &orion_serial_dev->dev;

	serial_line[1] = serial8250_register_port(&port);
	if (serial_line[1] < 0) {
		printk(KERN_WARNING "UART1: serial8250_register_port returned error %d\n", serial_line[1]);
		return serial_line[1];
	}
#endif
	return 0;
}

static void __exit serial_orion_exit(void)
{
	serial8250_unregister_port(serial_line[0]);
	serial8250_unregister_port(serial_line[1]);
}

module_init(serial_orion_init);
module_exit(serial_orion_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ORION platform UART driver");
