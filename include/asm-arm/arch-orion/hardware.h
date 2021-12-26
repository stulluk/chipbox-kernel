/*
 *  linux/include/asm-arm/arch-virgo/hardware.h
 *
 *  This file contains the hardware definitions of the Celestial Virgo SoC.
 *
 *  Copyright (C) 2005 Celestial Semiconductor. 
 *
 *  derived from versatile boards hardware.h originally copyright by ARM.
 * 
 *  Copyright (C) 2003 ARM Limited.
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
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include <asm/arch/platform.h>


/* macro to get at IO space when running virtually */
#define IO_ADDRESS(x)		(((x) & 0x0fffffff) + (((x) >> 4) & 0x0f000000) + 0xf0000000)

#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1100) 
#define CPU_FREQ 220000000
#elif defined(CONFIG_ARCH_ORION_CSM1201)
#define CPU_FREQ 216000000  // possible 196000000
#endif 

#define PCLK_FREQ	(CPU_FREQ/4)   //possible 55000000, 54000000, 49000000

#define PA_UARTS_BASE		0x101f1000
#define PA_TIMERS_BASE		0x101e0000
#define PA_DMAC_BASE		0x10130000
#define PA_VIC_BASE		    0x10140000
#define PA_IDSCS_BASE		0x10171000
#define PA_I2C_BASE		    0x10170000
#define PA_BLTER_BASE		0x40000000
#define PA_VENUS_BASE		0x41000000
#define PA_VENUSMEM_BASE	0x06000000


#define PA_ETH_BASE 		0x35000000
#define PA_MAC_BASE         0x34080000

#define PA_ATA_BASE 		0x10210000
#define PA_PCMCIA_BASE 		0x10250000
 


#define VIRGO_IRQ_UART0		12
#define VIRGO_IRQ_UART1		13

#if defined(CONFIG_ARCH_ORION_CSM1100)
#define VIRGO_IRQ_ETH 		29
#endif

#define VA_UARTS_BASE		IO_ADDRESS(PA_UARTS_BASE)
#define VA_TIMERS_BASE		IO_ADDRESS(PA_TIMERS_BASE)
#define VA_DMAC_BASE		IO_ADDRESS(PA_DMAC_BASE)
#define VA_VIC_BASE		    IO_ADDRESS(PA_VIC_BASE)
#define VA_IDSCS_BASE		IO_ADDRESS(PA_IDSCS_BASE)
#define VA_I2C_BASE		    IO_ADDRESS(PA_I2C_BASE)
#define VA_BLTER_BASE		IO_ADDRESS(PA_BLTER_BASE)
#define VA_VENUS_BASE		IO_ADDRESS(PA_VENUS_BASE)
#define VA_VENUSMEM_BASE	0xf6000000
#define VA_ATA_BASE		IO_ADDRESS(PA_ATA_BASE)
#define VA_PCMCIA_BASE		IO_ADDRESS(PA_PCMCIA_BASE)

#define VA_UART0_BASE		VA_UARTS_BASE
#define VA_UART1_BASE		(VA_UARTS_BASE+0x1000)

#define VA_ETH_BASE 		IO_ADDRESS(PA_ETH_BASE)
#define VA_MAC_BASE         IO_ADDRESS(PA_MAC_BASE)

#define VIRGO_ETH_MAC_FROM_FLASH
#endif
