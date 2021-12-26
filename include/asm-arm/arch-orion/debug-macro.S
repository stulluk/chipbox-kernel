/* linux/include/asm-arm/arch-virgo/debug-macro.S
 *
 * DXL: derived from versatile PB code.
 * Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <asm/hardware/amba_serial.h>

		.macro	addruart,rx
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1			@ MMU enabled?
		moveq	\rx,      #0x10000000
		movne	\rx,      #0xf1000000	@ virtual base
		orr	\rx, \rx, #0x001F0000
		orr	\rx, \rx, #0x00001000
		.endm

		.macro	senduart,rd,rx
		strb	\rd, [\rx, #UART01x_DR]
		.endm

		.macro	waituart,rd,rx		@ HW Flow control not supprted
		.endm

		.macro	busyuart,rd,rx
1001:		ldr	\rd, [\rx, #0x14]	@ LSR
		tst	\rd, #1 << 5		@ LSR THRE (TX EMPTY)
		beq	1001b
		.endm

