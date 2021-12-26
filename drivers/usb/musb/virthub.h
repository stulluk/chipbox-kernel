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
 * Definitions for a virtual hub intended for embedding in HCDs
 * for controllers lacking an embedded root hub in hardware
 * $Revision: 1.1.1.1 $
 */

#ifndef __MGC_VIRTUALHUB_H__
#define __MGC_VIRTUALHUB_H__

#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/usb.h>

/**
 * Introduction.
 * For USB controllers lacking embedded root hubs,
 * this module can be used as a virtual root hub,
 * with one or more controllers as the virtual hub's ports.
 */

/****************************** CONSTANTS ********************************/

/** Maximum number of ports to accomodate */
#define MGC_VIRTUALHUB_MAX_PORTS	7

/******************************** TYPES **********************************/

/**
 * Set a port's power on or off.
 * @param pPrivateData pPrivateData from port services
 * @param bPortIndex 0-based index of port
 * @param bPower 0 to power on the port; -1 to power off
 */
typedef void (*MGC_pfSetPortPower)(void* pPrivateData, uint8_t bPortIndex, uint8_t bPower);

/**
 * Enable or disable a port.
 * @param pPrivateData pPrivateData from port services
 * @param bPortIndex 0-based index of port
 * @param bEnable 0 to enable port; -1 to disable
 */
typedef void (*MGC_pfSetPortEnable)(void* pPrivateData, uint8_t bPortIndex, uint8_t bEnable);

/**
 * Set a port's suspend mode on or off.
 * @param pPrivateData pPrivateData from port services
 * @param bPortIndex 0-based index of port
 * @param bSuspend 0 to suspend port; -1 to resume
 */
typedef void (*MGC_pfSetPortSuspend)(void* pPrivateData, uint8_t bPortIndex, uint8_t bSuspend);

/**
 * Set a port's reset on or off.
 * @param pPrivateData pPrivateData from port services
 * @param bPortIndex 0-based index of port
 * @param bReset 0 to assert reset on the bus behind a port; -1 to deassert
 */
typedef void (*MGC_pfSetPortReset)(void* pPrivateData, uint8_t bPortIndex, uint8_t bReset);

/**
 * MGC_PortServices.
 * Services provided to a virtual by a USB port controller.
 * @field pPrivateData port controller's implementation data;
 * not to be interpreted by virtual hub
 * @param pfSetPortPower set-port-power call
 * @param pfSetPortEnable set-port-enable call
 * @param pfSetPortSuspend set-port-suspend call
 * @param pfSetPortReset set-port-reset call
 */
typedef struct {
        void* pPrivateData;
        MGC_pfSetPortPower pfSetPortPower;
        MGC_pfSetPortEnable pfSetPortEnable;
        MGC_pfSetPortSuspend pfSetPortSuspend;
        MGC_pfSetPortReset pfSetPortReset;
} MGC_PortServices;

/**
 * MGC_HubPortStatusChange.
 * @field wStatus status
 * @field wChange change
 */
typedef struct {
        volatile uint16_t wStatus;
        volatile uint16_t wChange;
} MGC_HubPortStatusChange;

/**
 * MGC_VirtualHub.
 * Virtual USB hub instance data.
 * @field Lock spinlock
 * @field pDevice our device pointer
 * @field pUrb pointer to interrupt URB for status change
 * @field pPortServices pointer to port services
 * @field Timer interval timer for status change interrupts
 * @field aPortStatusChange status/change array
 * @field bPortCount how many ports
 * @field wInterval actual interval in milliseconds
 * @field bIsChanged 0 if changes to report
 * @field bAddress address assigned by usbcore
 */
typedef struct {
        spinlock_t Lock;
        struct usb_device* pDevice;
        void *pUrb;
        struct usb_bus* pBus;
        MGC_PortServices* pPortServices;
        struct timer_list Timer;
        MGC_HubPortStatusChange aPortStatusChange[MGC_VIRTUALHUB_MAX_PORTS];
        uint8_t bPortCount;
        uint16_t wInterval;
        uint8_t bIsChanged;
        uint8_t bAddress;
} MGC_VirtualHub;

/****************************** FUNCTIONS ********************************/

/**
 * Initialize a virtual hub.
 * @param pHub hub struct pointer; struct filled on success
 * @param pDevice pointer to bus
 * @param bPortCount how many ports to support
 * @param pPortServices port services
 * @return 0 on success
 * @return -1 on failure
 */
extern uint8_t MGC_VirtualHubInit(MGC_VirtualHub* pHub,
                                          struct usb_bus* pBus,
                                          uint8_t bPortCount,
                                          MGC_PortServices* pPortServices);

/**
 * Destroy a virtual hub
 */
extern void MGC_VirtualHubDestroy(MGC_VirtualHub* pHub);

/**
 * Start a virtual hub
 */
extern void MGC_VirtualHubStart(MGC_VirtualHub* pHub);

/**
 * Stop a virtual hub
 */
extern void MGC_VirtualHubStop(MGC_VirtualHub* pHub);

/**
 * Submit an URB to a virtual hub.
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param pUrb URB pointer
 * @return Linux status code
 * @see #MGC_VirtualHubInit
 */
extern int MGC_VirtualHubSubmitUrb(MGC_VirtualHub* pHub, struct urb* pUrb);

/**
 * Unlink an URB from a virtual hub.
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param pUrb URB pointer
 * @return Linux status code
 * @see #MGC_VirtualHubInit
 */
extern int MGC_VirtualHubUnlinkUrb(MGC_VirtualHub* pHub, struct urb* pUrb);

/**
 * A port reset is complete
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param bPortIndex 0-based index of port
 * @see #MGC_VirtualHubInit
 */
extern void MGC_VirtualHubPortResetDone(MGC_VirtualHub* pHub,
                                                uint8_t bPortIndex, uint8_t bHubSpeed);

/**
 * A device has effectively been connected to a virtual hub port
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param bPortIndex 0-based index of port with connected device
 * @param bSpeed device speed (0=>low, 1=>full, 2=>high)
 * @see #MGC_VirtualHubInit
 */
extern void MGC_VirtualHubPortConnected(MGC_VirtualHub* pHub,
                                                uint8_t bPortIndex, uint8_t bSpeed);

/**
 * A device has effectively resumed a virtual hub port
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param bPortIndex 0-based index of port of resume
 * @see #MGC_VirtualHubInit
 */
extern void MGC_VirtualHubPortResumed(MGC_VirtualHub* pHub, uint8_t bPortIndex);

/**
 * A device has effectively been disconnected from a virtual hub port
 * @param pHub pointer to hub initialized by successful MGC_VirtualHubInit
 * @param bPortIndex 0-based index of port of disconnected device
 * @see #MGC_VirtualHubInit
 */
extern void MGC_VirtualHubPortDisconnected(MGC_VirtualHub* pHub, uint8_t bPortIndex);

#endif	/* __MGC_VIRTUALHUB_H__ */

