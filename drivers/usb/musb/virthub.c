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
 * A virtual hub intended for embedding in HCDs
 * for controllers lacking an embedded root hub in hardware
 * $Revision: 1.1.1.1 $
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/timer.h>

#include <linux/usb.h>
#include <linux/version.h>

#include "musbdefs.h"

/******************************* FORWARDS ********************************/

static void MGC_VirtualHubActivateTimer(MGC_VirtualHub* pHub,
                                        void (*pfExpired)(unsigned long),
                                        unsigned long timeout);
static void MGC_VirtualHubCompleteIrq(MGC_VirtualHub* pHub, struct urb* pUrb);
static void MGC_VirtualHubTimerExpired(unsigned long ptr);

/******************************* GLOBALS *********************************/

/** device descriptor */
static uint8_t MGC_aVirtualHubDeviceDesc[] = {
        USB_DT_DEVICE_SIZE,
        USB_DT_DEVICE,
        0x00, 0x02,		/* bcdUSB */
        USB_CLASS_HUB,		/* bDeviceClass */
        0,			/* bDeviceSubClass */
        1,			/* bDeviceProtocol (single TT) */
        64,			/* bMaxPacketSize0 */
        0xd6, 0x4,		/* idVendor */
        0, 0,			/* idProduct */
        0, 0,			/* bcdDevice */
        0,			/* iManufacturer */
        0,			/* iProduct */
        0,			/* iSerialNumber */
        1			/* bNumConfigurations */
};

/** device qualifier */
static uint8_t MGC_aVirtualHubQualifierDesc[] = {
        USB_DT_DEVICE_QUALIFIER_SIZE,
        USB_DT_DEVICE_QUALIFIER,
        0x00, 0x02,		/* bcdUSB */
        USB_CLASS_HUB,		/* bDeviceClass */
        0,			/* bDeviceSubClass */
        0,			/* bDeviceProtocol */
        64,			/* bMaxPacketSize0 */
        0xd6, 0x4,		/* idVendor */
        0, 0,			/* idProduct */
        0, 0,			/* bcdDevice */
        0,			/* iManufacturer */
        0,			/* iProduct */
        0,			/* iSerialNumber */
        1			/* bNumConfigurations */
};

/** Configuration descriptor */
static uint8_t MGC_VirtualHubConfigDesc[] = {
        USB_DT_CONFIG_SIZE,
        USB_DT_CONFIG,
        USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE + USB_DT_ENDPOINT_SIZE, 0,
        0x01,			/* bNumInterfaces */
        0x01,			/* bConfigurationValue */
        0x00,			/* iConfiguration */
        0xE0,			/* bmAttributes (self-powered, remote wake) */
        0x00,			/* MaxPower */

        /* interface */
        USB_DT_INTERFACE_SIZE,
        USB_DT_INTERFACE,
        0x00,			/* bInterfaceNumber */
        0x00,			/* bAlternateSetting */
        0x01,			/* bNumEndpoints */
        USB_CLASS_HUB,		/* bInterfaceClass */
        0x00,			/* bInterfaceSubClass */
        0x00,			/* bInterfaceProtocol */
        0x00,			/* iInterface */

        /* endpoint */
        USB_DT_ENDPOINT_SIZE,
        USB_DT_ENDPOINT,
        USB_DIR_IN | 1,	        /* bEndpointAddress: IN Endpoint 1 */
        USB_ENDPOINT_XFER_INT,	/* bmAttributes: Interrupt */
        (MGC_VIRTUALHUB_MAX_PORTS + 8) / 8, 0,	/* wMaxPacketSize */
        12			/* bInterval: 256 ms */
};

/** other-speed Configuration descriptor */
static uint8_t MGC_VirtualHubOtherConfigDesc[] = {
        USB_DT_CONFIG_SIZE,
        USB_DT_OTHER_SPEED,
        USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE + USB_DT_ENDPOINT_SIZE, 0,
        0x01,			/* bNumInterfaces */
        0x01,			/* bConfigurationValue */
        0x00,			/* iConfiguration */
        0xE0,			/* bmAttributes (self-powered, remote wake) */
        0x00,			/* MaxPower */

        /* interface */
        USB_DT_INTERFACE_SIZE,
        USB_DT_INTERFACE,
        0x00,			/* bInterfaceNumber */
        0x00,			/* bAlternateSetting */
        0x01,			/* bNumEndpoints */
        USB_CLASS_HUB,		/* bInterfaceClass */
        0x00,			/* bInterfaceSubClass */
        0x00,			/* bInterfaceProtocol */
        0x00,			/* iInterface */

        /* endpoint */
        USB_DT_ENDPOINT_SIZE,
        USB_DT_ENDPOINT,
        USB_DIR_IN | 1,	        /* bEndpointAddress: IN Endpoint 1 */
        USB_ENDPOINT_XFER_INT,	/* bmAttributes: Interrupt */
        (MGC_VIRTUALHUB_MAX_PORTS + 8) / 8, 0,	/* wMaxPacketSize */
        0xff			/* bInterval: 255 ms */
};

/****************************** FUNCTIONS ********************************/

/**
 * Generic timer activation helper. Requires the hub structure to
 * be locked.
 *
 * @param pHub pointer to hub struct
 * @param pfExpired callback function
 * @param timeout millisecs
 * @requires spin_lock(pHub->Lock)
 */
static void MGC_VirtualHubActivateTimer(MGC_VirtualHub* pHub,
                                        void (*pfExpired)(unsigned long), unsigned long timeout)
{
        mod_timer( &(pHub->Timer), jiffies + timeout * HZ / 1000);
}

/*
 * assumes pHub to be locked!
 * @requires spin_lock(pHub->Lock)
 */
static void MGC_VirtualHubCompleteIrq(MGC_VirtualHub* pHub, struct urb* pUrb)
{
        int nLength, nPort;
        uint8_t bData, bBit;
        uint8_t* pData;

        pHub->bIsChanged = FALSE;

        /* how many bits are needed/possible */
        nLength = min(pUrb->transfer_buffer_length * 8,
                      pHub->bPortCount + 1);
        bData = 0;
        bBit = 1;
        pData = (uint8_t*)pUrb->transfer_buffer;

        /* count 1..N to accomodate hub status bit */
        for (nPort = 1; nPort <= nLength; nPort++) {
                if (pHub->aPortStatusChange[nPort-1].wChange & 1) {
                        bData |= 1 << bBit;
                }
                if (++bBit > 7) {
                        *pData++ = bData;
                        bData = bBit = 0;
                }
        }

        if (bBit) {
                *pData++ = bData;
        }

        pUrb->actual_length = (int)pData - (int)pUrb->transfer_buffer;
        if (pUrb->actual_length && pUrb->complete) {

                DEBUG_CODE(1, printk(KERN_INFO "%s: completing hub interrupt URB\n", \
                                     __FUNCTION__); )
                pUrb->status = 0;
				  pUrb->hcpriv = NULL;
				 // printk("MGC_VirtualHubCompleteIrq 1\n");
                pUrb->complete(pUrb, NULL);
        }
}

/**
 * Timer expiration function to complete the interrupt URB on changes
 * @param ptr standard expiration param (hub pointer)
 */
static void MGC_VirtualHubTimerExpired(unsigned long ptr)
{
        MGC_VirtualHub* pHub = (MGC_VirtualHub*)ptr;
        struct urb* pUrb;

        spin_lock(&pHub->Lock);
        pUrb=pHub->pUrb;
        if (pUrb && (pUrb->hcpriv == pHub)) {
                uint8_t bPort;

                for (bPort = 0; bPort < pHub->bPortCount; bPort++) {
                        if ( pHub->aPortStatusChange[bPort].wChange ) {
							// printk ("KB MGC_VirtualHubTimerExpired : wChange[%d]\n", pHub->aPortStatusChange[bPort].wChange);
                                MGC_VirtualHubCompleteIrq(pHub, pUrb);
									 spin_unlock(&pHub->Lock);
									 return;
                                //break;
                        }
                }
        }

        /* re-activate timer */
        MGC_VirtualHubActivateTimer(pHub, MGC_VirtualHubTimerExpired,
                                    pHub->wInterval);

        spin_unlock(&pHub->Lock);
}

/* the below function is defined in ../core/message.c */
extern int usb_get_device_descriptor(struct usb_device *dev, unsigned int size);

/** Implementation */
uint8_t MGC_VirtualHubInit(MGC_VirtualHub* pHub, struct usb_bus* pBus,
                           uint8_t bPortCount, MGC_PortServices* pPortServices)
{
        uint8_t bPort;

        if (bPortCount > MGC_VIRTUALHUB_MAX_PORTS) {
                printk(KERN_INFO "%s: Cannot allocate a %d-port device (too many ports)",
                       __FUNCTION__, bPortCount);
                return FALSE;
        }

        /* allocate device */
        pHub->pDevice = usb_alloc_dev(NULL, pBus, bPortCount);
        if (!pHub->pDevice) {
                printk(KERN_INFO "%s: Cannot allocate a %d-port device",
                       __FUNCTION__, bPortCount);
                return FALSE;
        }

        DEBUG_CODE(1, \
                   printk(KERN_INFO "%s: New device (%d-port virtual hub) @%#lx allocated\n", \
                          __FUNCTION__, bPortCount, (unsigned long)pHub->pDevice); )

        pHub->pDevice->speed=USB_SPEED_HIGH;
        pHub->pDevice->state = USB_STATE_ADDRESS;
        pHub->pDevice->devnum = 1;
        pHub->pDevice->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
        pBus->devnum_next = 2;
        usb_get_device_descriptor(pHub->pDevice, USB_DT_DEVICE_SIZE);

        pHub->Lock = SPIN_LOCK_UNLOCKED;
        pHub->pBus = pBus;
        pHub->pUrb = NULL;
        pHub->pPortServices = pPortServices;
        pHub->bPortCount = bPortCount;
        pHub->bIsChanged = FALSE;
        init_timer(&(pHub->Timer)); /* I will need this later */
        pHub->Timer.function = MGC_VirtualHubTimerExpired;
        pHub->Timer.data = (unsigned long)pHub;

        for (bPort = 0; bPort < bPortCount; bPort++) {
                pHub->aPortStatusChange[bPort].wStatus = 0;
                pHub->aPortStatusChange[bPort].wChange = 0;
        }

        DEBUG_CODE(3, printk(KERN_INFO "%s: announcing device to usbcore\n", \
                             __FUNCTION__); )

        return TRUE;
}

/** Implementation */
extern void MGC_VirtualHubDestroy(MGC_VirtualHub* pHub)
{
}

/** Implementation */
void MGC_VirtualHubStart(MGC_VirtualHub* pHub)
{
        DBG(2, "<== announcing device to usbcore\n");

        if (0 != usb_new_device(pHub->pDevice)) {
                printk(KERN_ERR "%s: usb_new_device failed\n", __FUNCTION__);
        }
        DBG(2, "==> announced to usbcore\n");
}

/** Implementation */
void MGC_VirtualHubStop(MGC_VirtualHub* pHub)
{
        /* stop interrupt timer */
        del_timer_sync(&pHub->Timer);
}

/** Implementation */
int MGC_VirtualHubSubmitUrb(MGC_VirtualHub* pHub, struct urb* pUrb)
{
        uint8_t bRecip;		/* from standard request */
        uint8_t bReqType;	        /* from standard request */
        uint8_t bType;		/* requested descriptor type */
        uint16_t wValue;	        /* from standard request */
        uint16_t wIndex;	        /* from standard request */
        uint16_t wLength;	        /* from standard request */
        uint8_t bPort;
        const MGC_DeviceRequest* pRequest;
        uint16_t wSize = 0xffff;
        uint8_t* pData = (uint8_t*)pUrb->transfer_buffer;
        unsigned int pipe = pUrb->pipe;

        usb_get_urb(pUrb);

        spin_lock(&pHub->Lock);

        pUrb->hcpriv = pHub;
        pUrb->status = -EINPROGRESS;
        if (usb_pipeint (pipe)) {
                DEBUG_CODE(3, printk(KERN_ERR "%s: is periodic status/change event\n",
                                     __FUNCTION__); )

                /* this is the one for periodic status/change events */
                pHub->pUrb = pUrb;
                pHub->wInterval = (pUrb->interval < 16) ? (1 << (pUrb->interval - 4)) :
                                  pUrb->interval;
                MGC_VirtualHubActivateTimer(pHub, MGC_VirtualHubTimerExpired,
                                            pHub->wInterval);
                spin_unlock(&pHub->Lock);
                return 0;
        }

        /* handle hub requests/commands */
        pRequest = (const MGC_DeviceRequest*)pUrb->setup_packet;
        bReqType = pRequest->bmRequestType & USB_TYPE_MASK;
        bRecip = pRequest->bmRequestType & USB_RECIP_MASK;
        wValue = le16_to_cpu(pRequest->wValue);
        wIndex = le16_to_cpu(pRequest->wIndex);
        wLength = le16_to_cpu(pRequest->wLength);

        DEBUG_CODE(2, \
                   printk(KERN_INFO "%s,%d: bRequest=%02x, bmRequestType=%02x, wLength=%04x\n", \
                          __FUNCTION__, __LINE__, pRequest->bRequest, pRequest->bmRequestType, \
                          wLength); )

        switch (pRequest->bRequest) {
        case USB_REQ_GET_STATUS:

                DEBUG_CODE(3, printk(KERN_INFO "%s: GET_STATUS(), bType=%02x, bRecip=%02x, wIndex=%04x\n", \
                                     __FUNCTION__, bReqType, bRecip, wIndex); )

                if (USB_TYPE_STANDARD == bReqType) {
                        if (USB_RECIP_DEVICE == bRecip) {
                                /* self-powered */
                                pData[0] = 1;
                                pData[1] = 0;
                                wSize = 2;
                        } else {
                                pData[0] = 0;
                                pData[1] = 0;
                                wSize = 2;
                        }
                } else if (USB_TYPE_CLASS == bReqType) {
                        if ((USB_RECIP_OTHER == bRecip) && (wIndex <= pHub->bPortCount)) {
                                /* port status/change report */
									spin_lock(&pHub->Lock);
                                memcpy(pData,
                                       (uint8_t*)(&(pHub->aPortStatusChange[wIndex-1].wStatus)), 2);
                                memcpy(&(pData[2]),
                                       (uint8_t*)(&(pHub->aPortStatusChange[wIndex-1].wChange)), 2);
                                /* reset change (TODO: lock) */
                                pHub->aPortStatusChange[wIndex-1].wChange = 0;
									 spin_unlock(&pHub->Lock);
                                wSize = 4;
                        } else {
                                /* hub status */
                                memset(pData, 0, 4);
                                wSize = 4;
                        }

                        DEBUG_CODE(1, \
                                   printk(KERN_INFO "%s: status report=%02x%02x%02x%02x\n", \
                                          __FUNCTION__, pData[0], pData[1], pData[2], pData[3]); )

                }
                break;

        case USB_REQ_CLEAR_FEATURE:
                if ((USB_TYPE_STANDARD == bReqType) && (USB_RECIP_ENDPOINT == bRecip)) {
                        wSize = 0;
                } else if (USB_TYPE_CLASS == bReqType) {

                        if (USB_RECIP_OTHER == bRecip) {
                                bPort = (uint8_t)(wIndex & 0xff) - 1;

                                DEBUG_CODE(3, printk(KERN_INFO "%s: CLEAR_PORT_FEATURE(%04x), \
                                                     port %d\n", __FUNCTION__, wValue, bPort); )

                                switch (wValue) {
#if (MGC_DEBUG>0)
                                case USB_PORT_FEAT_CONNECTION:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat connection port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_OVER_CURRENT:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat feat over current %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_POWER:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat feat power port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_LOWSPEED:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat lo-speed port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_HIGHSPEED:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat hi-speed port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_TEST:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat hi-test port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_INDICATOR:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: feat indicator port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        wSize = 0;
                                        break;
#else
                                case USB_PORT_FEAT_CONNECTION:
                                case USB_PORT_FEAT_OVER_CURRENT:
                                case USB_PORT_FEAT_POWER:
                                case USB_PORT_FEAT_LOWSPEED:
                                case USB_PORT_FEAT_HIGHSPEED:
                                case USB_PORT_FEAT_TEST:
                                case USB_PORT_FEAT_INDICATOR:
                                        wSize = 0;
                                        break;
#endif
                                case USB_PORT_FEAT_ENABLE:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: enable port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        pHub->pPortServices->pfSetPortEnable(
                                                pHub->pPortServices->pPrivateData, bPort, FALSE);
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_SUSPEND:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: suspend port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        pHub->pPortServices->pfSetPortSuspend(
                                                pHub->pPortServices->pPrivateData, bPort, FALSE);
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_RESET:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: reset port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        pHub->pPortServices->pfSetPortReset(
                                                pHub->pPortServices->pPrivateData, bPort, FALSE);
                                        wSize = 0;
                                        break;

                                        /* acknowledge changes: */
                                case USB_PORT_FEAT_C_CONNECTION:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: ack connection port %d\n", \
                                                             __FUNCTION__, bPort); )

                                        pHub->aPortStatusChange[bPort].wChange &= ~1;
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_C_ENABLE:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: ack enable port %d\n", \
                                                             __FUNCTION__, bPort); )

                                        pHub->aPortStatusChange[bPort].wChange &= ~USB_PORT_STAT_ENABLE;
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_C_SUSPEND:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: ack suspend port %d\n", \
                                                             __FUNCTION__, bPort); )

                                        pHub->aPortStatusChange[bPort].wChange &= ~USB_PORT_STAT_SUSPEND;
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_C_RESET:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: ack reset port %d\n", \
                                                             __FUNCTION__, bPort); )
                                        pHub->aPortStatusChange[bPort].wChange &= ~USB_PORT_STAT_RESET;
                                        wSize = 0;
                                        break;
                                case USB_PORT_FEAT_C_OVER_CURRENT:
                                        DEBUG_CODE(3, printk(KERN_INFO "%s: ack over current port %d\n", \
                                                             __FUNCTION__, bPort); )

                                        wSize = 0;
                                        break;
                                }
                        } else {
                                switch (wValue) {
                                case C_HUB_LOCAL_POWER:
                                case C_HUB_OVER_CURRENT:
                                        wSize = 0;
                                        break;
                                }
                        }
                }
                break;

        case USB_REQ_SET_FEATURE:
                if ((USB_TYPE_CLASS == bReqType) && (USB_RECIP_OTHER == bRecip)) {
                        bPort = (uint8_t)(wIndex & 0xff) - 1;

                        DEBUG_CODE(3, printk(KERN_INFO "%s: SET_PORT_FEATURE(%04x), port %d\n", \
                                             __FUNCTION__, wValue, bPort); )

                        switch (wValue) {
                        case USB_PORT_FEAT_SUSPEND:
                                DEBUG_CODE(1, printk(KERN_INFO "%s: suspend port %d\n",\
                                                     __FUNCTION__, bPort); )
                                pHub->pPortServices->pfSetPortSuspend(pHub->pPortServices->pPrivateData, bPort, TRUE);
                                pHub->aPortStatusChange[bPort].wStatus |= USB_PORT_STAT_SUSPEND;
                                pHub->bIsChanged = TRUE;
                                wSize = 0;
                                break;

                        case USB_PORT_FEAT_RESET:
                                DEBUG_CODE(3, printk(KERN_INFO "%s: reset port %d\n",\
                                                     __FUNCTION__, bPort); )

                                pHub->aPortStatusChange[bPort].wStatus |= USB_PORT_STAT_RESET;
                                pHub->aPortStatusChange[bPort].wStatus |= USB_PORT_STAT_ENABLE;
                                pHub->aPortStatusChange[bPort].wChange |= USB_PORT_STAT_RESET;
                                pHub->bIsChanged = TRUE;
                                pHub->pPortServices->pfSetPortReset(pHub->pPortServices->pPrivateData,
                                                                    bPort, TRUE);
                                wSize = 0;
                                break;

                        case USB_PORT_FEAT_POWER:
                                DEBUG_CODE(3, printk(KERN_INFO "%s: power port %d\n", \
                                                     __FUNCTION__, bPort); )

                                pHub->pPortServices->pfSetPortPower(pHub->pPortServices->pPrivateData,
                                                                    bPort, TRUE);
                                pHub->aPortStatusChange[bPort].wStatus |= USB_PORT_STAT_POWER;
                                wSize = 0;

                                break;

                        case USB_PORT_FEAT_ENABLE:
                                DEBUG_CODE(3, printk(KERN_INFO "%s: enable port %d\n", \
                                                     __FUNCTION__, bPort); )
                                pHub->pPortServices->pfSetPortEnable(pHub->pPortServices->pPrivateData,
                                                                     bPort, TRUE);
                                pHub->aPortStatusChange[bPort].wStatus |= USB_PORT_STAT_ENABLE;
                                wSize = 0;
                                break;
                        }
                } else {
                        DEBUG_CODE(3, printk(KERN_INFO "%s: SET_FEATURE(%04x), but feature unknown\n", \
                                             __FUNCTION__, wValue); )
                }
                break;

        case USB_REQ_SET_ADDRESS:
                pHub->bAddress = (wValue & 0x7f);
                DEBUG_CODE(2, printk(KERN_INFO "%s: SET_ADDRESS(%x) \n", \
                                     __FUNCTION__, pHub->bAddress); )
                wSize = 0;
                break;

        case USB_REQ_GET_DESCRIPTOR:
                if (USB_TYPE_CLASS == bReqType) {
                        DEBUG_CODE(2, printk(KERN_INFO "%s: GET_CLASS_DESCRIPTOR()\n", \
                                             __FUNCTION__); )
                        pData[0] = 9;
                        pData[1] = 0x29;
                        pData[2] = pHub->bPortCount;
                        /* min characteristics */
                        pData[3] = 1;  /* invidual port power switching */
                        pData[4] = 0;
                        /* PowerOn2PowerGood */
                        pData[5] = 50;
                        /* no current */
                        pData[6] = 0;
                        /* removable ports */
                        pData[7] = 0;
                        /* reserved */
                        pData[8] = 0xff;
                        wSize = pData[0];
                        break;
                } else {
                        bType = (uint8_t)(wValue >> 8);

                        DEBUG_CODE(2, printk(KERN_INFO "%s: GET_DESCRIPTOR(%d)\n", \
                                             __FUNCTION__, bType); )

                        switch (bType) {
                        case USB_DT_DEVICE: /* 1 */
                                wSize = min(wLength, (uint16_t)MGC_aVirtualHubDeviceDesc[0]);
                                memcpy(pData, MGC_aVirtualHubDeviceDesc, wSize);
                                break;
                        case USB_DT_DEVICE_QUALIFIER:
                                wSize = min(wLength, (uint16_t)MGC_aVirtualHubQualifierDesc[0]);
                                memcpy(pData, MGC_aVirtualHubQualifierDesc, wSize);
                                break;
                        case USB_DT_CONFIG: /* 2 */
                                wSize = min(wLength, (uint16_t)MGC_VirtualHubConfigDesc[2]);
                                memcpy(pData, MGC_VirtualHubConfigDesc, wSize);
                                break;
                        case USB_DT_OTHER_SPEED:
                                wSize = min(wLength, (uint16_t)MGC_VirtualHubOtherConfigDesc[2]);
                                memcpy(pData, MGC_VirtualHubOtherConfigDesc, wSize);
                                break;
                        }
                }
                break;

        case USB_REQ_GET_CONFIGURATION:
                DEBUG_CODE(3, printk(KERN_INFO "%s: GET_CONFIG() => 1\n", \
                                     __FUNCTION__); )
                pData[0] = 1;
                wSize = 1;
                break;

        case USB_REQ_SET_CONFIGURATION:
                DEBUG_CODE(3, printk(KERN_INFO "%s: SET_CONFIG(%04x)\n", \
                                     __FUNCTION__, wValue); )
                wSize = 0;
                break;

        }   /* END: switch on request type */

        if (0xffff == wSize) {
                pUrb->status = USB_ST_STALL;
        } else {
                pUrb->actual_length = wSize;
                pUrb->status = 0;
        }

        spin_unlock(&pHub->Lock);

        if (pUrb->complete) {
                DEBUG_CODE(5, printk(KERN_INFO "%s: completing URB\n", \
                                     __FUNCTION__); )
                pUrb->hcpriv = NULL;
				// printk("MGC_VirtualHubSubmitUrb 1\n");
                pUrb->complete(pUrb, NULL);
                usb_put_urb(pUrb);
                DEBUG_CODE(5, printk(KERN_INFO "%s: URB completed\n", \
                                     __FUNCTION__); )
        }

        DEBUG_CODE(2, printk(KERN_ERR "%s: pUrb->status=%d %s, length=%d, completed=%s\n", \
                             __FUNCTION__, pUrb->status, (pUrb->status)?"(STALL)":"", \
                             pUrb->actual_length, (pUrb->complete)?"yes":"no"); )

        return 0;
}

/** Implementation */
int MGC_VirtualHubUnlinkUrb(MGC_VirtualHub* pHub, struct urb* pUrb)
{
        spin_lock(&pHub->Lock);
        if (pUrb && (pHub->pUrb == pUrb) && (pUrb->hcpriv == pHub)) {
                pUrb->hcpriv = NULL;
                pHub->bIsChanged = FALSE;
                pHub->pUrb = NULL;

                if (pUrb->transfer_flags & USB_ASYNC_UNLINK) {
                        pUrb->status = -ECONNRESET;
                        if (pUrb->complete) {
							printk("MGC_VirtualHubUnlinkUrb 1\n");
                                pUrb->complete(pUrb, NULL);
                        }
                } else {
                        pUrb->status = -ENOENT;
                }
        }

        usb_put_urb(pUrb);

        spin_unlock(&pHub->Lock);

        return 0;
}


/**
 * assumes bPortIndex < MGC_VIRTUALHUB_MAX_PORTS
  * AND pHub->Loclk to be... locked :)

 */
static void MGC_SetVirtualHubPortSpeed(MGC_VirtualHub* pHub,
                                       uint8_t bPortIndex, uint8_t bSpeed)
{
        uint16_t wSpeedMask = 0;

        switch (bSpeed) {
        case 0:
                wSpeedMask = USB_PORT_STAT_LOW_SPEED;
                break;
        case 2:
                wSpeedMask = USB_PORT_STAT_HIGH_SPEED;
                break;
        }


        pHub->aPortStatusChange[bPortIndex].wStatus &=
                ~(USB_PORT_STAT_LOW_SPEED | USB_PORT_STAT_HIGH_SPEED);
        //pHub->aPortStatusChange[bPortIndex].wStatus |= 1 | wSpeedMask;
        pHub->aPortStatusChange[bPortIndex].wStatus |= wSpeedMask;
        pHub->bIsChanged = TRUE;
}

/** Implementation */
void MGC_VirtualHubPortResetDone(MGC_VirtualHub* pHub, uint8_t bPortIndex,
                                 uint8_t bHubSpeed)
{
        spin_lock(&pHub->Lock);
        DEBUG_CODE(1, printk(KERN_INFO "%s: port %d reset complete\n", \
                             __FUNCTION__, bPortIndex); )
        if (bPortIndex < MGC_VIRTUALHUB_MAX_PORTS) {
                MGC_SetVirtualHubPortSpeed(pHub, bPortIndex, bHubSpeed);

                pHub->aPortStatusChange[bPortIndex].wStatus &= ~USB_PORT_STAT_RESET;
                pHub->aPortStatusChange[bPortIndex].wStatus |= USB_PORT_STAT_ENABLE;
                pHub->aPortStatusChange[bPortIndex].wChange = USB_PORT_STAT_RESET |
                                USB_PORT_STAT_ENABLE;
                pHub->bIsChanged = TRUE;
        }
        spin_unlock(&pHub->Lock);
}

/** Implementation */
void MGC_VirtualHubPortConnected(MGC_VirtualHub* pHub, uint8_t bPortIndex,
                                 uint8_t bSpeed)
{
        struct urb* pUrb;

        DEBUG_CODE(1, printk(KERN_INFO "%s: port %d connected, core reports speed=%d\n", \
                             __FUNCTION__, bPortIndex, bSpeed); )

        if (bPortIndex < MGC_VIRTUALHUB_MAX_PORTS) {
                spin_lock(&pHub->Lock);

                pUrb=pHub->pUrb;
                MGC_SetVirtualHubPortSpeed(pHub, bPortIndex, bSpeed);
				  pHub->aPortStatusChange[bPortIndex].wStatus |= USB_PORT_STAT_CONNECTION;
                pHub->aPortStatusChange[bPortIndex].wChange |= USB_PORT_STAT_CONNECTION;

                /*if (pUrb && (pUrb->hcpriv == pHub)) {
                        DEBUG_CODE(1, printk(KERN_INFO "%s: Activated timer for %d ms\n", \
                                             __FUNCTION__, pHub->wInterval); )
                        MGC_VirtualHubActivateTimer(pHub, MGC_VirtualHubTimerExpired,
                                                    pHub->wInterval);
                }*/

                spin_unlock(&pHub->Lock);
        }

}

/** Implementation */
void MGC_VirtualHubPortDisconnected(MGC_VirtualHub* pHub, uint8_t bPortIndex)
{
        struct urb* pUrb;

        DEBUG_CODE(1, printk(KERN_INFO "%s: port %d disconnected\n", \
                             __FUNCTION__, bPortIndex); )

        if (bPortIndex < MGC_VIRTUALHUB_MAX_PORTS) {
                spin_lock(&pHub->Lock);
                pUrb= pHub->pUrb;

                //del_timer_sync(&pHub->Timer);
                pHub->aPortStatusChange[bPortIndex].wStatus &= ~USB_PORT_STAT_CONNECTION;
                pHub->aPortStatusChange[bPortIndex].wChange |= USB_PORT_STAT_CONNECTION;
                pHub->bIsChanged = TRUE;

                /*if (pUrb && (pUrb->hcpriv == pHub)) {
                        MGC_VirtualHubCompleteIrq(pHub, pUrb);
                }*/

                spin_unlock(&pHub->Lock);
        }

}

/** Implementation */
void MGC_VirtualHubPortResumed(MGC_VirtualHub* pHub, uint8_t bPortIndex)
{
        if (bPortIndex < MGC_VIRTUALHUB_MAX_PORTS) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: resume port %d\n", \
                                     __FUNCTION__, bPortIndex); )
                pHub->aPortStatusChange[bPortIndex].wStatus &= ~USB_PORT_STAT_SUSPEND;
                pHub->aPortStatusChange[bPortIndex].wChange |= USB_PORT_STAT_SUSPEND;
                pHub->bIsChanged = TRUE;
                spin_unlock(&pHub->Lock);
        }
}
