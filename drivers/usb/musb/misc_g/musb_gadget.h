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

/* Inventra Controller Driver (ICD) for Linux.
 * Interface to GADGET API
 * $Revision: 1.1.1.1 $
 */

#ifndef __MUSB_GADGET_H
#define __MUSB_GADGET_H

#include <linux/list.h>
#include <linux/errno.h>

#ifdef MUSB_V26

#include <linux/device.h>  /* for struct device */
#include <linux/usb_ch9.h> /* sources need to be add */
#include <linux/usb_gadget.h> /* sources need to be add */

#else
enum usb_device_speed {
        GUSB_SPEED_UNKNOWN = 0,                  /* enumerating */
        GUSB_SPEED_LOW, GUSB_SPEED_FULL,          /* usb 1.1 */
        GUSB_SPEED_HIGH                          /* usb 2.0 */
}; 

/* #define kdev_t	dev_t */

#ifdef MUSB_NEEDS_DEVICEH
#include <linux/device.h>  /* for struct device */
#endif
#ifdef MUSB_NEEDS_CH9
#include <linux/usb_ch9.h> /* sources need to be add */
#endif

#endif

#include <linux/usb_gadget.h> /* sources need to be add */

/* -----------------------CONSTANTS------------------- */

/** this where used to compile in directory gadgets 
 * This code will be osoleted
 */
#ifdef MUSB_V26
#define DAEMONIZE(_x)	daemonize(_x)

#define DEQUEUE_SIGNAL(_x, _y) dequeue_signal(current, _x, _y)
#define RECALC_SIGPENDING(_x) recalc_sigpending()

/**
 * The atomic_t field in the task structure is used to lock it,
 * I can simulate a spinlock on it, probably somethigng coded 
 * around <asm/atomic.h>
 *
 * atomic_dec_and_test(&current->usage)
 *
 * was: spin_lock_irq(&current->sigmask_lock)
 * 	spin_unlock_irq(&current->sigmask_lock)
 *
 * but the scheduler is changed hence the struct task used from it.
 */
#define SPINLOCK_CURRENT() 	{  }
#define SPINUNLOCK_CURRENT()	{  }

/* The new filesystem in 2.6 has obsoleted the use of blk_dev(),
 * could not find what to do... setting BLK_SIZE to 0 prevent the 
 * gadget to initialize (but it compiles)
 */
#define BLK_SIZE(_dev) 0
#define DEVBLK_SIZE(_dev) 0

#endif


#ifdef MUSB_V24
#define DAEMONIZE(_x)	daemonize()
#define DEQUEUE_SIGNAL(_x, _y) dequeue_signal(current, _x, _y)
#define RECALC_SIGPENDING(_x) recalc_sigpending(_x)

#define SPINLOCK_CURRENT() spin_lock_irq(&current->sigmask_lock);
#define SPINUNLOCK_CURRENT() spin_unlock_irq(&current->sigmask_lock);

#define BLK_SIZE(_dev) blk_size[MAJOR(_dev)]
#define DEVBLK_SIZE(_dev) ((loff_t)blk_size[MAJOR(_dev)][MINOR(_dev)]<<BLOCK_SIZE_BITS)
#endif

/* --------------------------- TYPES --------------------- */

/**
 * MGC_GadgetLocalEnd.
 * A gadget local endpoint (to remember certain things separately
 * from host mode, so we can swap roles in OTG)
 *
 * @field Lock spinlock
 * @field pRequest current request
 * @field wPacketSize programmed packet size
 * @field bTrafficType programmed traffic type
 * @field bIsTx TRUE if current direction is Tx
 */
typedef struct
{
    spinlock_t Lock;
    struct usb_request* pRequest;
    unsigned int dwOffset;
    unsigned int dwRequestSize;
    uint16_t wPacketSize;
    uint8_t bTrafficType;
    uint8_t bIsTx;
} MGC_GadgetLocalEnd;


#endif /* __MUSB_GADGET_H */
