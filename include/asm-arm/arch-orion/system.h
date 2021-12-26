/*
 *  linux/include/asm-arm/arch-virgo/system.h
 *
 *  Copyright (C) 2003 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

static inline void arch_reset(char mode)
{
     	/* WatchDog base */
	volatile unsigned short *p;
	p = (volatile unsigned short *)ioremap(0x101e1000, 0x1000);

	/* WDT_TORR */
	p[2] = 0x0;					
	p[3] = 0x0; 
	/* WDT_CCVR */
	p[4] = 0x0;
	p[5] = 0x0;
	/* WDT_CR, enable WDT and generated System Reset */
	p[0] = 0x1;
}

#endif
