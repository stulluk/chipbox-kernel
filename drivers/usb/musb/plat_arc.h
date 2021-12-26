/******************************************************************
 * Copyright 2006 Mentor Graphics Corporation
 *
 * This file is part of the Inventra Controller Driver for Linux.
 *
 * The Inventra Controller Driver for Linux is free software; you
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * The Inventra Controller Driver for Linux is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Inventra Controller Driver for Linux ; if not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * ANY DOWNLOAD, USE, REPRODUCTION, MODIFICATION OR DISTRIBUTION
 * OF THIS DRIVER INDICATES YOUR COMPLETE AND UNCONDITIONAL ACCEPTANCE
 * OF THOSE TERMS.THIS DRIVER IS PROVIDED "AS IS" AND MENTOR GRAPHICS
 * MAKES NO WARRANTIES, EXPRESS OR IMPLIED, RELATED TO THIS DRIVER.
 * MENTOR GRAPHICS SPECIFICALLY DISCLAIMS ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY; FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  MENTOR GRAPHICS DOES NOT PROVIDE SUPPORT
 * SERVICES OR UPDATES FOR THIS DRIVER, EVEN IF YOU ARE A MENTOR
 * GRAPHICS SUPPORT CUSTOMER.
 ******************************************************************/

/*
 * Linux-specific architecture definitions
 * $Revision: 1.1.1.1 $
 */

#ifndef __MGC_LINUX_PLATFORM_ARCH_H__
#define __MGC_LINUX_PLATFORM_ARCH_H__

int usb_g=1;
int usb_r=1;

#if 0

#define MGC_Read8(_pBase, _offset) usb_g?( (usb_r=*(volatile uint8_t*)(_pBase + _offset),printk( "%s:read8(%lx, %x)= %02x\n", \
   __FUNCTION__,(unsigned long)_pBase, _offset, usb_r)), usb_r):*(volatile uint8_t*)(_pBase + _offset)
#define MGC_Read16(_pBase, _offset) usb_g?((usb_r=*(volatile uint16_t*)(_pBase + _offset),printk( "%s:Read16(%lx, %x)=%04x\n", \
    __FUNCTION__, (unsigned long)_pBase, _offset, usb_r)),usb_r):*(volatile uint16_t*)(_pBase + _offset)
#define MGC_Read32(_pBase, _offset) usb_g? ((usb_r=*(volatile uint32_t*)(_pBase + _offset), printk("%s:Read32(%lx, %x=%08x\n", \
  		 __FUNCTION__, (unsigned long)_pBase, _offset, usb_r)),usb_r):*(volatile uint32_t*)(_pBase + _offset)

#else

#define MGC_Read8(_pBase, _offset) *(volatile uint8_t*)(_pBase + _offset)
#define MGC_Read16(_pBase, _offset) *(volatile uint16_t*)(_pBase + _offset)
#define MGC_Read32(_pBase, _offset) *(volatile uint32_t*)(_pBase + _offset)

#endif
		 
#if 0

#define MGC_Write8(_pBase, _offset, _data) { \
if(usb_g)\
{\
printk( "%s:Write8(%lx, %x, %02x)\n", \
   __FUNCTION__,(unsigned long)_pBase, _offset, _data); \
}\
    *(volatile uint8_t*)(_pBase + _offset) = _data; \
}

#define MGC_Write16(_pBase, _offset, _data) { \
	if(usb_g)\
		{\
printk( "%s:Write16(%lx, %x, %04x)\n", \
    __FUNCTION__, (unsigned long)_pBase, _offset, _data); \
		}\
    *(volatile uint16_t*)(_pBase + _offset) = _data; \
}

#define MGC_Write32(_pBase, _offset, _data) { \
	if(usb_g)\
		{\
   printk("%s:Write32(%lx, %x, %08x)\n", \
   __FUNCTION__, (unsigned long)_pBase, _offset, _data);  \
		}\
    *(volatile uint32_t*)(_pBase + _offset) = _data; \
}

#else

#define MGC_Write8(_pBase, _offset, _data) (MGC_Read8(_pBase, _offset)) = _data
#define MGC_Write16(_pBase, _offset, _data) (MGC_Read16(_pBase, _offset)) = _data
#define MGC_Write32(_pBase, _offset, _data) (MGC_Read32(_pBase, _offset)) = _data
#endif /* end of MGC_DEBUG > 0 */

#endif	/* multiple inclusion protection */
