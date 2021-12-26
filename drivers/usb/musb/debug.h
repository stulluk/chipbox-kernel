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

#ifndef __MGC_DEBUG_H__
#define __MGC_DEBUG_H__

/*
 * Linux HCD (Host Controller Driver) for HDRC and/or MHDRC.
 * Debug support routines
 *
 * $Revision: 1.1.1.1 $
 */

#include <linux/completion.h>
#include <linux/usb.h>

#if MGC_DEBUG > 0

extern int MGC_DebugLevel;

extern void dump_struct(char *var, char *name, void *ptr);
extern void dump_urb(struct urb *urb);
extern char *decode_csr0(uint16_t csr0);
extern char *decode_txcsr(uint16_t txcsr);
extern char *decode_devctl(uint16_t devclt);
extern char *decode_ep0stage(uint8_t stage);
extern void dump_all_regs(uint8_t* pBase, int multipoint, uint8_t bEnd);

#define DUMP_STRUCT(_name, _ptr) dump_struct("_ptr", _name, _ptr)

#define DEBUG_CODE(level, code)	do { \
	if ( (level>=-1 && MGC_DebugLevel>level) || \
	MGC_DebugLevel==level) { code } } while (0);

#define TRACE(n) DEBUG_CODE(n, printk(KERN_INFO "%s:%s:%d: trace\n", \
	__FILE__, __FUNCTION__, __LINE__); )

#else /* debug no defined */

#define DEBUG_CODE(x, y) do { }while(0);
#define TRACE(n)

#endif /* MGC_DEBUG */

#define DBG(_l,_t) DEBUG_CODE( _l, printk(KERN_INFO "%s", _t); )

#endif /* __MGC_DEBUG_H__ */


