/*
 *  linux/include/asm-arm/arch-virgo/uncompress.h
 *  DXL: derived from versatile PB code
 * 
 *  Copyright (C) 2003 ARM Limited
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
#define AMBA_UART_DR	(*(volatile unsigned char *)0x101F1000)
#define AMBA_UART_LSR	(*(volatile unsigned char *)0x101F1014)
#define LSR_THRE    	0x20        /* Xmit holding register empty */

/*
 * This does not append a newline
 */
static void putstr(const char *s)
{
	while (*s) {
		AMBA_UART_DR = *s;
		while ((AMBA_UART_LSR & LSR_THRE) == 0)
			barrier();

		if (*s == '\n') {
			AMBA_UART_DR = '\r';
			while ((AMBA_UART_LSR & LSR_THRE) == 0)
				barrier();

		}
		s++;
	}
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
