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
 * Definitions to abstract different versions of Linux for
 * the Inventra Controller Driver
 * $Revision: 1.1.1.1 $
 */

#ifndef __MGC_MUSBDEFS_H__
#define __MGC_MUSBDEFS_H__

#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>

#include <linux/slab.h>
#include <linux/list.h>
#include <linux/smp_lock.h>
#include <linux/fs.h>

#include "../drivers/usb/core/hub.h"
#include "../drivers/usb/core/hcd.h"

/* Board-specific definitions (hard-wired controller locations/IRQs) */

#include "plat_cnf.h"
#include "plat_arc.h"
#include "musbhdrc.h"
#include "virthub.h"
#include "debug.h"

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

/****************************** CONSTANTS ********************************/

#ifndef USB_DT_DEVICE_QUALIFIER
#define USB_DT_DEVICE_QUALIFIER 	6
#endif

#ifndef USB_DT_DEVICE_QUALIFIER_SIZE
#define USB_DT_DEVICE_QUALIFIER_SIZE 	10
#endif

#ifndef USB_DT_OTHER_SPEED
#define USB_DT_OTHER_SPEED 		7
#endif

#ifndef MGC_MAX_END0_PACKET
#define MGC_MAX_END0_PACKET ((uint16_t)MGC_END0_FIFOSIZE)
#endif

#define MGC_END0_START  		0
#define MGC_END0_OUT    		1
#define MGC_END0_IN     		2
#define MGC_END0_STATUS 		3

#define MGC_END0_STAGE_SETUP 		0x0
#define MGC_END0_STAGE_TX		0x2
#define MGC_END0_STAGE_RX		0x4
#define MGC_END0_STAGE_STATUSIN		0x8
#define MGC_END0_STAGE_STATUSOUT        0xf
#define MGC_END0_STAGE_STALL_BIT	0x10

#define MGC_TEST_PACKET_SIZE		53

#define MGC_PAD_FRONT   		0xa5deadfe
#define MGC_PAD_BACK    		0xabadcafe

#define USB_ISO_ASAP            	0x0002
#define USB_ASYNC_UNLINK        	0x0008

#define USB_ST_NOERROR          	0
#define USB_ST_CRC             		(-EILSEQ)
#define USB_ST_BITSTUFF         	(-EPROTO)
#define USB_ST_NORESPONSE       	(-ETIMEDOUT)	/* device not responding/handshaking */
#define USB_ST_DATAOVERRUN      	(-EOVERFLOW)
#define USB_ST_DATAUNDERRUN     	(-EREMOTEIO)
#define USB_ST_BUFFEROVERRUN    	(-ECOMM)
#define USB_ST_BUFFERUNDERRUN   	(-ENOSR)
#define USB_ST_INTERNALERROR    	(-EPROTO)	/* unknown error */
#define USB_ST_SHORT_PACKET     	(-EREMOTEIO)
#define USB_ST_PARTIAL_ERROR    	(-EXDEV)	/* ISO transfer nly partially completed */
#define USB_ST_URB_KILLED       	(-ENOENT)	/* URB canceled by user */
#define USB_ST_URB_PENDING       	(-EINPROGRESS)
#define USB_ST_REMOVED          	(-ENODEV)	/* device not existing or removed */
#define USB_ST_TIMEOUT          	(-ETIMEDOUT)	/* communication timed out, also in urb->status**/
#define USB_ST_NOTSUPPORTED     	(-ENOSYS)
#define USB_ST_BANDWIDTH_ERROR  	(-ENOSPC)	/* too much bandwidth used */
#define USB_ST_URB_INVALID_ERROR  	(-EINVAL)	/* invalid value/transfer type */
#define USB_ST_URB_REQUEST_ERROR  	(-ENXIO)		/* invalid endpoint */
#define USB_ST_STALL            	(-EPIPE)		/* pipe stalled, also in urb->status*/

#define USB_ZERO_PACKET         	0x0040  		/* Finish bulk OUTs always with zero length packet */

typedef enum {
        MGC_STATE_DEFAULT,
        MGC_STATE_ADDRESS,
        MGC_STATE_CONFIGURED
} MGC_DeviceState;

/* failure codes */
#define MGC_ERR_VBUS		-1
#define MGC_ERR_BABBLE		-2
#define MGC_ERR_CORRUPTED	-3
#define MGC_ERR_IRQ		-4
#define MGC_ERR_SHUTDOWN	-5
#define MGC_ERR_RESTART		-6

/****************************** FUNCTIONS ********************************/

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
#define USB_HALT_ENDPOINT(_dev, _pipe_ep, _pipe_out) do {}while(0)
#define USB_ENDPOINT_HALTED(_dev, _pipe_ep, _pipe_out) (0)
#else
#define USB_HALT_ENDPOINT(_dev, _pipe_ep, _pipe_out) usb_endpoint_halt(_dev, _pipe_ep, _pipe_out)
#define USB_ENDPOINT_HALTED(_dev, _pipe_ep, _pipe_out) usb_endpoint_halted(_dev, _pipe_ep, _pipe_out)
#endif

#define MGC_HST_MODE(_pthis) { _pthis->bIsHost=TRUE; _pthis->bIsDevice=FALSE; }
#define MGC_DEV_MODE(_pthis) { _pthis->bIsHost=FALSE; _pthis->bIsDevice=TRUE; }
#define MGC_OTG_MODE(_pthis) { _pthis->bIsHost=FALSE; _pthis->bIsDevice=FALSE; }
#define MGC_ERR_MODE(_pthis) { _pthis->bIsHost=TRUE; _pthis->bIsDevice=TRUE; }

#define MGC_MODE(_x) ((_x->bIsHost) ? ((!_x->bIsDevice)?"HOST":"ERROR"):(_x->bIsDevice)?"DEVICE":"OTG")
#define MGC_IS_HST(_x) (_x->bIsHost && !_x->bIsDevice )
#define MGC_IS_DEV(_x) ( !_x->bIsHost && _x->bIsDevice )
#define MGC_IS_OTG(_x) ( !_x->bIsHost && !_x->bIsDevice )
#define MGC_IS_ERR(_x) ( _x->bIsHost && _x->bIsDevice )

/* indexed vs. flat register model */
#ifdef MGC_FLAT_REG
#define MGC_SelectEnd(_pBase, _bEnd)
#define MGC_ReadCsr8(_pBase, _bOffset, _bEnd) MGC_Read8(_pBase, MGC_END_OFFSET(_bEnd, _bOffset))
#define MGC_ReadCsr16(_pBase, _bOffset, _bEnd) MGC_Read16(_pBase, MGC_END_OFFSET(_bEnd, _bOffset))
#define MGC_WriteCsr8(_pBase, _bOffset, _bEnd, _bData) MGC_Write8(_pBase, MGC_END_OFFSET(_bEnd, _bOffset), _bData)
#define MGC_WriteCsr16(_pBase, _bOffset, _bEnd, _bData) MGC_Write16(_pBase, MGC_END_OFFSET(_bEnd, _bOffset), _bData)
#else
#define MGC_SelectEnd(_pBase, _bEnd) MGC_Write8(_pBase, MGC_O_HDRC_INDEX, _bEnd)
#define MGC_ReadCsr8(_pBase, _bOffset, _bEnd) MGC_Read8(_pBase, (_bOffset + 0x10))
#define MGC_ReadCsr16(_pBase, _bOffset, _bEnd) MGC_Read16(_pBase, (_bOffset + 0x10))
#define MGC_WriteCsr8(_pBase, _bOffset, _bEnd, _bData) MGC_Write8(_pBase, (_bOffset + 0x10), _bData)
#define MGC_WriteCsr16(_pBase, _bOffset, _bEnd, _bData) MGC_Write16(_pBase, (_bOffset + 0x10), _bData)
#endif


/******************************** DMA TYPES **********************************/

#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)

#ifdef MGC_DMA
#include "dma.h"
#define WANTS_DMA(_pUrb) ((_pUrb)->transfer_dma && (_pUrb->transfer_flags & URB_NO_TRANSFER_DMA_MAP))
//#define DMA_BUFFER(_pUrb) ((_pUrb)->transfer_dma)
#define DMA_BUFFER(pUrb) ((void*)0x000666) //LAND cp from mentor source replace above line
#ifndef MGC_HSDMA_CHANNELS
#define MGC_HSDMA_CHANNELS	2
#endif

extern MGC_DmaControllerFactory MGC_HdrcDmaControllerFactory;
#endif

/******************************** TYPES **********************************/

struct urb;
struct usb_device;

typedef struct {
        uint8_t bmRequestType;
        uint8_t bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
} MGC_DeviceRequest;

/**
 * MGC_LinuxLocalEnd.
 * Local endpoint resource.
 * @field Lock spinlock
 * @field pUrb current URB
 * @field urb_list list
 * @field dwOffset current buffer offset
 * @field dwRequestSize how many bytes were last requested to move
 * @field wMaxPacketSizeTx local Tx FIFO size
 * @field wMaxPacketSizeRx local Rx FIFO size
 * @field wPacketSize programmed packet size
 * @field bIsSharedFifo 0 if FIFO is shared between Tx and Rx
 * @field bAddress programmed bus address
 * @field bEnd programmed remote endpoint address
 * @field bTrafficType programmed traffic type
 * @field bIsClaimed 0 if claimed
 * @field bIsTx 0 if current direction is Tx
 * @field bIsReady 0 if ready (available for new URB)
 */
typedef struct {
#if MGC_DEBUG > 0
        uint32_t dwPadFront;
#endif
        spinlock_t Lock;
        struct urb* pUrb;
        struct list_head urb_list;
        unsigned int dwOffset;
        unsigned int dwRequestSize;
        unsigned int dwIsoPacket;
        unsigned int dwWaitFrame;
#ifdef MGC_DMA
        MGC_DmaChannel* pDmaChannel;
#endif
        uint16_t wMaxPacketSizeTx;
        uint16_t wMaxPacketSizeRx;
        uint16_t wPacketSize;
        uint8_t bIsSharedFifo;
        uint8_t bAddress;
        uint8_t bEnd;
        uint8_t bTrafficType;
        uint8_t bIsClaimed;
        uint8_t bIsTx;
        uint8_t bIsReady;
        uint8_t bRetries;
        uint8_t bLocalEnd;
#if MGC_DEBUG > 0
        uint32_t dwPadBack;
#endif
} MGC_LinuxLocalEnd;

/**
 * MGC_LinuxCd.
 * Driver instance data.
 * @field Lock spinlock
 * @field Timer interval timer for various things
 * @field pBus pointer to Linux USBD bus
 * @field RootHub virtual root hub
 * @field PortServices services provided to virtual root hub
 * @field pRootDevice root device pointer, to track connection speed
 * @field nIrq IRQ number (needed by free_irq)
 * @field nIsPci 0 if PCI
 * @field bIsMultipoint 0 if multi-point core
 * @field bIsHost 0 if host
 * @field bIsDevice 0 if peripheral
 * @field nIackAddr IACK address (PCI only)
 * @field nIackSize size of IACK PCI region (needed by release_region)
 * @field nRegsAddr address of registers PCI region (needed by release_region)
 * @field nRegsSize size of registers region (needed by release_region)
 * @field pIack pointer to mapped IACK region (PCI only)
 * @field pRegs pointer to mapped registers
 */
typedef struct {
#if MGC_DEBUG > 0
        uint32_t dwPadFront;
#endif
        spinlock_t Lock;
        struct timer_list Timer;
        struct usb_bus *pBus;
        char aName[32];
        MGC_VirtualHub RootHub;
        MGC_PortServices PortServices;
        struct usb_device* pRootDevice;
#ifdef MGC_DMA
        MGC_DmaController* pDmaController;
#endif
        int nIrq;
        int nIsPci;
        int nBabbleCount;
        unsigned int nIackAddr;
        unsigned int nIackSize;
        unsigned int nRegsAddr;
        unsigned int nRegsSize;
        void* pIack;
        void* pRegs;
        MGC_LinuxLocalEnd aLocalEnd[MGC_C_NUM_EPS];
        uint16_t wEndMask;
        uint8_t bEndCount;
        uint8_t bRootSpeed;
        uint8_t bIsMultipoint;
        uint8_t bIsHost;
        uint8_t bIsDevice;

        uint8_t bIgnoreDisconnect; /* during bus resets I got fake disconnects */
        uint8_t bVbusErrors; /* bus errors found */

        uint8_t bBulkTxEnd;
        uint8_t bBulkRxEnd;
        uint8_t bBulkSplit;
        uint8_t bBulkCombine;

		 uint8_t bIntTxEnd;
        uint8_t bIntRxEnd;

		 uint8_t bIsoTxEnd;
        uint8_t bIsoRxEnd;

        uint8_t bEnd0Stage; /* end0 stage while in host or device mode */
#define MGC_BUFFER_POOLS	4
struct dma_pool		*pool [MGC_BUFFER_POOLS];
#if MGC_DEBUG > 0
        uint32_t dwPadBack;
#endif
} MGC_LinuxCd;

extern void mgc_hdrc_start(MGC_LinuxCd* pThis);
extern void mgc_hdrc_stop(MGC_LinuxCd* pThis);


#endif	/* __MGC_MUSBDEFS_H__ */

