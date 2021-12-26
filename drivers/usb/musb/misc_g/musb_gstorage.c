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
  * 
  * MGC Storage demo device
  * $Revision: 1.1.1.1 $ 
  */
 
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/timer.h>

#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <linux/usb_ch9.h>
#include "musbdefs.h"

#ifdef MUSB_V26
#include <linux/device.h>
#endif

#include <linux/init.h>

#include "musb_gadgetdefs.h"

/* -------------------------------------------------------------- */

#ifndef MUSB_C_NUM_EPS
#define MUSB_C_NUM_EPS 16
#endif

#ifndef MUSB_MAX_END0_PACKET
#define MUSB_MAX_END0_PACKET 64
#endif

#ifdef GSTORAGE_SHARED_EP
#define MGC_END_IN GSTORAGE_SHARED_EP
#define MGC_END_OUT GSTORAGE_SHARED_EP
#else
/* safest confoguration */
#define MGC_END_IN 1
#define MGC_END_OUT 2
#endif

#ifdef GSTORAGE_SCSI_CMD_DEBUG
#define SCSI_DEBUG(format, args...) do { printk(KERN_INFO "SCSI_CMD:" format , ## args); } while (0)
#else
#define SCSI_DEBUG(format, args...) do { } while (0)
#endif

/* -------------------------------------------------------------- */

STATIC void MGC_GadgetTaskletHandler(unsigned long data);

/* endpoint operations */
int MGC_GadgetEnableEnd(struct usb_ep *ep,
	const struct usb_endpoint_descriptor *desc);
int MGC_GadgetDisableEnd(struct usb_ep *ep);

static struct usb_request* MGC_GadgetAllocRequest(struct usb_ep *ep,
	int gfp_flags);
static void MGC_GadgetFreeRequest(struct usb_ep *ep, struct usb_request *req);

static int MGC_GadgetQueue(struct usb_ep *ep, struct usb_request *req,
			   int gfp_flags);
static int MGC_GadgetDequeue(struct usb_ep *ep, struct usb_request *req);

STATIC int MGC_GadgetFifoStatus(struct usb_ep *ep);

/* general operations */
STATIC int MGC_GadgetGetFrame(struct usb_gadget *gadget);
STATIC int MGC_GadgetWakeup(struct usb_gadget *gadget);
STATIC int MGC_GadgetSetSelfPowered(struct usb_gadget *gadget, 
				    int is_selfpowered);
#ifdef MUSB_OTG
STATIC int MGC_GadgetVbusSession(struct usb_gadget *gadget, int is_active);
STATIC int MGC_GadgetVbusDraw(struct usb_gadget *gadget, unsigned mA);
STATIC int MGC_GadgetPullup(struct usb_gadget *gadget, int is_on);
#endif

int MGC_GadgetSetHalt(struct usb_ep *ep, int value);
static int MGC_GadgetIoctl(struct usb_gadget *gadget,
	unsigned code, unsigned long param);
void MGC_GadgetQueueCompletion(MGC_GadgetLocalEnd* pEnd, 
	struct usb_request* pRequest);

/*--------------------  locals -----------------------*/

/* this is the hard drive */
static uint8_t MGC_aTestData[1024*2048+512];
	
struct usb_ep_ops MGC_GadgetEndpointOperations =
{
	MGC_GadgetEnableEnd,
	MGC_GadgetDisableEnd,
	MGC_GadgetAllocRequest,
	MGC_GadgetFreeRequest,
	MGC_GadgetAllocBuffer,
	MGC_GadgetFreeBuffer,
	MGC_GadgetQueue,
	MGC_GadgetDequeue,
	MGC_GadgetSetHalt,
	MGC_GadgetFifoStatus,
	MGC_GadgetFifoFlush
};

struct usb_gadget_ops MGC_GadgetOperations = {
	MGC_GadgetGetFrame,
	MGC_GadgetWakeup,
	MGC_GadgetSetSelfPowered,
#if defined(MUSB_OTG) && defined(GADGET_FULL)
	MGC_GadgetVbusSession,
	MGC_GadgetVbusDraw,
	MGC_GadgetPullup,
#endif
	MGC_GadgetIoctl
};

/* The current Gadget API allows only singleton */
static struct usb_ep MGC_GadgetEnd0 =
{
    NULL,
    "ep0",
    &MGC_GadgetEndpointOperations,
    {},
    MUSB_MAX_END0_PACKET
};

struct usb_ep MGC_aGadgetEnd[MUSB_C_NUM_EPS];
static uint8_t MGC_bConfiguration = 0;

/*
 * TEST DEVICE
 */
static const uint8_t MGC_aTestDeviceDesc[] =
{
    /* Device Descriptor */
    0x12,                      /* bLength              */
    USB_DT_DEVICE,            /* DEVICE               */
    0x00,0x02,                 /* USB 2.0              */
    0x00,                      /* CLASS                */
    0x00,                      /* Subclass             */
    0x00,                      /* Protocol             */
    0x40,                      /* bMaxPktSize0         */
    0xd6,0x04,                 /* idVendor             */
    0x33,0x22,                 /* idProduct            */
    0x00,0x02,                 /* bcdDevice            */
    0x01,                      /* iManufacturer        */
    0x02,                      /* iProduct             */
    0x03,                      /* iSerial Number       */
    0x01,                      /* One configuration    */
};

static const uint8_t MGC_aTestDeviceQual[] =
{
    0x0a,                      /* bLength              */
    6,  /* DEVICE Qualifier     */
    0x00,0x02,                 /* USB 2.0              */
    0,     /* CLASS                */
    0,  /* Subclass             */
    0x00,                      /* Protocol             */
    0x40,                      /* bMaxPacketSize0      */
    0x01,                      /* One configuration    */
    0x00,                      /* bReserved            */
};

static const uint8_t MGC_aTestConfigDesc[] =
{
    /* configuration */
    0x09,                                   /* bLength              */
    0x02,                                   /* CONFIGURATION        */
    0x23,                                   /* length               */
    0x0,                                    /* length               */
    0x01,                                   /* bNumInterfaces       */
    0x01,                                   /* bConfigurationValue  */
    0x00,                                   /* iConfiguration       */
    0xC0,                                   /* bmAttributes (required + self-powered) */
    0x2,                                    /* power                */

    /* interface */
    0x09,                                   /* bLength              */
    0x04,                                   /* INTERFACE            */
    0x0,                                    /* bInterfaceNumber     */
    0x0,                                    /* bAlternateSetting    */
    0x02,                                   /* bNumEndpoints        */
    0x08,                                   /* bInterfaceClass      */
    0x06,                                   /* bInterfaceSubClass (1=RBC, 6=SCSI) */
    0x50,                                   /* bInterfaceProtocol (BOT) */
    0x00,                                   /* iInterface           */

    /* Endpoint Descriptor  : Bulk-In */
    0x07,                                   /* bLength              */
    0x05,                                   /* ENDPOINT             */
    0x80 | MGC_END_IN,                      /* bEndpointAddress     */
    0x02,                                   /* bmAttributes         */
    0x00, 0x02,                             /* wMaxPacketSize       */
    0x04,                                   /* bInterval            */

    /* Endpoint Descriptor  : Bulk-Out */
    0x07,                                   /* bLength              */
    0x05,                                   /* ENDPOINT             */
    MGC_END_OUT,                            /* bEndpointAddress     */
    0x02,                                   /* bmAttributes         */
    0x00, 0x02,                             /* wMaxPacketSize       */
    0x04,                                   /* bInterval            */

    /* OTG */
    3, 9, 3
};

static const uint8_t MGC_aTestFullSpdConfigDesc[] =
{
    /* configuration */
    0x09,                                   /* bLength              */
    0x07,                                   /* other-speed CONFIGURATION        */
    0x23,                                   /* length               */
    0x0,                                    /* length               */
    0x01,                                   /* bNumInterfaces       */
    0x01,                                   /* bConfigurationValue  */
    0x00,                                   /* iConfiguration       */
    0xC0,                                   /* bmAttributes (required + self-powered) */
    0x2,                                    /* power                */

    /* interface */
    0x09,                                   /* bLength              */
    0x04,                                   /* INTERFACE            */
    0x0,                                    /* bInterfaceNumber     */
    0x0,                                    /* bAlternateSetting    */
    0x02,                                   /* bNumEndpoints        */
    0x08,                                   /* bInterfaceClass      */
    0x06,                                   /* bInterfaceSubClass (1=RBC, 6=SCSI) */
    0x50,                                   /* bInterfaceProtocol (BOT) */
    0x00,                                   /* iInterface           */

    /* Endpoint Descriptor  : Bulk-In */
    0x07,                                   /* bLength              */
    0x05,                                   /* ENDPOINT             */
    0x80 | MGC_END_IN,                      /* bEndpointAddress     */
    0x02,                                   /* bmAttributes         */
    0x40, 0x00,                             /* wMaxPacketSize       */
    0x10,                                   /* bInterval            */

    /* Endpoint Descriptor  : Bulk-Out */
    0x07,                                   /* bLength              */
    0x05,                                   /* ENDPOINT             */
    MGC_END_OUT,                                   /* bEndpointAddress     */
    0x02,                                   /* bmAttributes         */
    0x40, 0x00,                             /* wMaxPacketSize       */
    0x10,                                   /* bInterval            */

    /* OTG */
    3, 9, 3
};

static const uint8_t MGC_aString0[] =
{
    /* strings */
    2+2,
    USB_DT_STRING,
    0x09, 0x04,			/* English (U.S.) */
};

static const uint8_t MGC_aString1Lang0409[] =
{
    2+30,			/* Manufacturer: Mentor Graphics */
    USB_DT_STRING,
    'M', 0, 'e', 0, 'n', 0, 't', 0, 'o', 0, 'r', 0, ' ', 0,
    'G', 0, 'r', 0, 'a', 0, 'p', 0, 'h', 0, 'i', 0, 'c', 0, 's', 0,
};

static const uint8_t MGC_aString2Lang0409[] =
{
    2+8,			/* Product ID: Demo */
    USB_DT_STRING,
    'D', 0, 'e', 0, 'm', 0, 'o', 0,
};

static const uint8_t MGC_aString3Lang0409[] =
{
    2+24,			/* Serial #: 123412341234 */
    USB_DT_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0,
    '1', 0, '2', 0, '3', 0, '4', 0,
    '1', 0, '2', 0, '3', 0, '4', 0,
};

static const char MGC_TestInquiryData[] =
{
    0,
    (char)0x80,   /* removable (though this doesn't seem so) */
    0,	    /* 0=no comformance to any standard */
    0,	    /* 2=required response data format */
    0x1f,	    /* extra length */
    0,	    /* flags */
    0,	    /* flags */
    0,	    /* flags */
    'M', 'e', 'n', 't', 'o', 'r', ' ', ' ',
    'D' ,'e', 'm', 'o', ' ', 'D', 'i', 's', 'k', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '1', ' ', ' ', ' '
};

static uint8_t MGC_TestMaxLun = 0;
static uint8_t MGC_TestInterface = 0;
static uint16_t MGC_TestPacketSize = 512;

static uint8_t MGC_aTestRxBuf[512];
static uint8_t MGC_aTestCsw[13] = 
{
    0x55, 0x53, 0x42, 0x53
};

static const uint8_t MGC_aTestCapacity[] =
{
    0, 0, 0x10, 0, 0, 0, 2, 0
};

static const uint8_t MGC_aTestFormatCapacity[] =
{
    0, 0, 0, 8, 0, 0, 0x10, 0, 0, 0, 2, 0
};

static const uint8_t MGC_aTestModeData[] =
{
    2, 0, 0, 0
};

static const uint8_t MGC_TestSenseData[] =
{
    0xf0, 
    0, 
    0,		    /* sense key */
    0, 0, 0, 0,	    /* dwInfo */
    5,		    /* additional length */
    0, 0, 0, 0,	    /* dwCommandInfo */
    0,		    /* ASC */
};

static uint8_t MGC_bHighSpeed = 0;

static uint8_t* MGC_pTestRx = MGC_aTestRxBuf;
static uint32_t MGC_dwTestRxCount = 31;
static uint32_t MGC_dwTestRxOffset = 0;

static uint8_t MGC_bTestTxData = 0;
static uint8_t MGC_bTestRxData = 0;
static uint8_t MGC_bTestIsReady = 1;
static uint8_t MGC_bSendZero = 0;

static uint8_t* MGC_pTestTx;
static uint32_t MGC_dwTestTxSize;
static uint32_t MGC_dwTestTxCount;
static uint32_t MGC_dwTestTxOffset = 0;

/*--------------------  Test Gadget -----------------------*/


STATIC void MGC_TestSend(void)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_dwTestTxSize = min(((uint32_t)MGC_TestPacketSize), MGC_dwTestTxCount);

    MGC_dwTestTxOffset = 0L;
    MGC_SelectEnd(pBase, MGC_END_IN);
    MGC_HdrcLoadFifo(pBase, MGC_END_IN, (uint16_t)MGC_dwTestTxSize, MGC_pTestTx);
    MGC_WriteCsr8(pBase, MGC_O_HDRC_TXCSR, (uint8_t)MGC_END_IN, 
	(uint8_t)(MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE));
}

STATIC void MGC_TestSendCsw(void)
{
    MGC_pTestTx = MGC_aTestCsw;
    MGC_dwTestTxCount = 13;
    MGC_TestSend();
}

STATIC void MGC_TestTxComplete(void)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    uint8_t* pBase = (uint8_t*)pThis->pRegs;

    if(MGC_bSendZero) {
		MGC_bSendZero = 0;
		MGC_dwTestTxCount = 0;
		MGC_dwTestTxOffset = 0L;
		MGC_SelectEnd(pBase, MGC_END_IN);
		MGC_WriteCsr8(pBase, MGC_O_HDRC_TXCSR, (uint8_t)MGC_END_IN, 
			(uint8_t)(MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE));
    } else if(MGC_bTestTxData) {
		/* finished sending data; send CSW */
		MGC_bTestTxData = 0;
		MGC_TestSendCsw();
    }
}

STATIC void MGC_TestRxComplete(void)
{
    uint32_t dwLength;
    uint32_t dwBlock = 0;
    const uint8_t* pCmd;

    if(MGC_bTestRxData) {
		/* finished getting data; send CSW */
		MGC_bTestRxData = 0;
		MGC_pTestRx = MGC_aTestRxBuf;
		MGC_TestSendCsw();
    } else {
		/* seed successful CSW */
		memcpy(&(MGC_aTestCsw[4]), &(MGC_pTestRx[4]), 4);
		MGC_aTestCsw[12] = 0;
	
		/* switch on command */
		pCmd = &(MGC_pTestRx[15]);
		switch(pCmd[0]) {
			case 0:
				/* test-unit-ready */
				SCSI_DEBUG("TEST_UNIT_READY\n");
				if(!MGC_bTestIsReady) {
					MGC_aTestCsw[12] = 1;
				}
				MGC_TestSendCsw();
				break;
		
			case 0x03:
				/* request sense */
				SCSI_DEBUG("REQUEST_SENSE\n");
				MGC_pTestTx = (uint8_t*)MGC_TestSenseData;
				MGC_dwTestTxCount = sizeof(MGC_TestSenseData);
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x12:
				/* inquiry */
				SCSI_DEBUG("INQUIRY\n");
				MGC_pTestTx = (uint8_t*)MGC_TestInquiryData;
				MGC_dwTestTxCount = sizeof(MGC_TestInquiryData);
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x1a:
				/* mode sense */
				SCSI_DEBUG("MODE_SENSE\n");
				MGC_pTestTx = (uint8_t*)MGC_aTestModeData;
				MGC_dwTestTxCount = sizeof(MGC_aTestModeData);
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x1b:
				/* start/stop unit */
				SCSI_DEBUG("START_STOP\n");
				MGC_bTestIsReady = (pCmd[4] & 1) ? 1: 0;
				MGC_TestSendCsw();
				break;
		
			case 0x1e:
				/* prevent/allow medium removal */
				SCSI_DEBUG("PREVENT_ALLOW_MEDIA_REMOVAL\n");
				MGC_TestSendCsw();
				break;
		
			case 0x23:
				/* read format capacity */
				SCSI_DEBUG("READ_FORMAT_CAPACITY\n");
				MGC_pTestTx = (uint8_t*)MGC_aTestFormatCapacity;
				MGC_dwTestTxCount = sizeof(MGC_aTestFormatCapacity);
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x25:
				/* read capacity */
				SCSI_DEBUG("READ_CAPACITY\n");
				MGC_pTestTx = (uint8_t*)MGC_aTestCapacity;
				MGC_dwTestTxCount = sizeof(MGC_aTestCapacity);
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x28:
				/* read(10) */	
				dwLength = (pCmd[7] << 8) | pCmd[8];
				dwBlock = (pCmd[2] << 24) | (pCmd[3] << 16) | (pCmd[4] << 8) | pCmd[5];
				
				SCSI_DEBUG("READ(10) dwLength %d, dwBlock %d\n", dwLength, dwBlock);
				MGC_pTestTx = &(MGC_aTestData[dwBlock * 512]);
				MGC_dwTestTxCount = dwLength * 512;
				//MGC_bSendZero = 1;
				MGC_bTestTxData = 1;
				MGC_TestSend();
				break;
		
			case 0x2a:
				/* write(10) */
				dwLength = (pCmd[7] << 8) | pCmd[8];
				dwBlock = (pCmd[2] << 24) | (pCmd[3] << 16) | (pCmd[4] << 8) | pCmd[5];
		
				SCSI_DEBUG("WRITE(10) dwLength %d, dwBlock %d\n", dwLength, dwBlock);
				MGC_pTestRx = &(MGC_aTestData[dwBlock * 512]);
				MGC_dwTestRxCount = dwLength * 512;
				MGC_bTestRxData = 1;
				break;
		
			case 0x2f:
				/* verify */
		
				SCSI_DEBUG("VERIFY\n");
				dwLength = (pCmd[7] << 8) | pCmd[8];
				if(pCmd[1] & 2) {
					dwBlock = (pCmd[2] << 24) | (pCmd[3] << 16) | (pCmd[4] << 8) | pCmd[5];
					MGC_pTestRx = &(MGC_aTestData[dwBlock * 512]);
					MGC_dwTestRxCount = dwLength * 512;
					MGC_bTestRxData = 1;
				} else {
					MGC_TestSendCsw();
				}
				break;
		
			default:
				SCSI_DEBUG("UNKNOWN\n");
				MGC_aTestCsw[12] = 1;
				MGC_TestSendCsw();
		}
    }
}

/**************************************************************************
Completition Functions
**************************************************************************/

void MGC_GadgetTaskletHandler(unsigned long data)
{
    uint8_t bEnd;
    MGC_GadgetLocalEnd* pEnd;
    struct usb_request* pRequest;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

    for(bEnd = 0; bEnd < min(pThis->bEndCount, MUSB_C_NUM_EPS); bEnd++) {
		pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
		pRequest = pEnd->CompletionData.pRequest;
		if(pRequest) {
			pRequest->complete(&(MGC_aGadgetEnd[bEnd]), pRequest);
			pEnd->CompletionData.pRequest = NULL;
		}
    }
}

DECLARE_TASKLET(MGC_GadgetTasklet, MGC_GadgetTaskletHandler, 0L);

#if 0
int MGC_GadgetFindEnd(struct usb_ep* pGadgetEnd)
{
    uint8_t bEnd;
    int nResult = -1;

    if(pGadgetEnd == &MGC_GadgetEnd0) {
        nResult = 0;
    } else {
        for(bEnd = 0; bEnd < MUSB_C_NUM_EPS; bEnd++) {
			if(pGadgetEnd == &(MGC_aGadgetEnd[bEnd])) {
				nResult = bEnd;
				break;
			}
		}
    }
    return nResult;

}
#endif

/**************************************************************************
Gadget Functions
**************************************************************************/

static int MGC_GadgetIoctl(struct usb_gadget *gadget,
	unsigned code, unsigned long param)
{
	int rc=-EINVAL;
#ifdef GSTORAGE_IOCTLS
	/* TODO: code to save/load the image to from file */ 
#endif
    return rc;
}

/**
 * Program the device endpoint.
 */
STATIC void MGC_HdrcProgramDeviceEnd(MGC_LinuxCd* pThis, uint8_t bEnd)
{
#ifdef MUSB_DMA
    MGC_DmaController* pDmaController;
    MGC_DmaChannel* pDmaChannel;
    uint8_t bAllocChannel;
    uint8_t bDmaOk = FALSE;
#endif
    uint16_t wCsrVal=0, wCount;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    MGC_LinuxLocalEnd* pHcdEnd = &(pThis->aLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;
    uint8_t* pFifoDest = pRequest->buf;
    uint8_t bIsBulk = (USB_ENDPOINT_XFER_BULK == pEnd->bTrafficType);

    MGC_SelectEnd(pBase, bEnd);

    pFifoDest = (uint8_t*)pRequest->buf;
    pEnd->dwOffset = 0L;
    if(!bEnd) {
		/* end0 should only be to transmit an IN control response */
		wFifoCount = min(MUSB_MAX_END0_PACKET, pRequest->length);
		MGC_HdrcLoadFifo(pBase, 0, wFifoCount, pFifoDest);
		pThis->wEnd0Offset = wFifoCount;
		wCsrVal = MGC_M_CSR0_TXPKTRDY;
		if(wFifoCount < MUSB_MAX_END0_PACKET) {
			wCsrVal |= MGC_M_CSR0_P_DATAEND;
			pThis->bEnd0Stage = MGC_END0_STATUS;
		}
		MGC_WriteCsr8(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
    } else {
		if(pEnd->bIsTx) {
			/* prepare CSR */
			wCsrVal = MGC_M_TXCSR_MODE;
			if(USB_ENDPOINT_XFER_ISOC == pEnd->bTrafficType) {
				wCsrVal |= MGC_M_TXCSR_ISO;
			}
	
			/* determine how much to send (used only for transmit) */
			if(bIsBulk && pThis->bBulkSplit) {
				wFifoCount = min(pHcdEnd->wMaxPacketSizeTx, 
					pRequest->length);
			} else {
				wFifoCount = min(pEnd->wPacketSize, pRequest->length);
			}
			pEnd->dwRequestSize = wFifoCount;
		}
	
#ifdef MUSB_DMA
		if(bIsBulk && (pRequest->length > pEnd->wPacketSize)) {
			/* candidate for DMA */
			bAllocChannel = FALSE;
			pDmaController = pThis->pDmaController;
			pDmaChannel = pHcdEnd->pDmaChannel;
			if(pDmaController && !pDmaChannel) {
				pDmaChannel = pHcdEnd->pDmaChannel = 
					pDmaController->pfDmaAllocateChannel(
					pDmaController->pPrivateData, bEnd, TRUE, 
					pEnd->bTrafficType, pEnd->wPacketSize);
				bAllocChannel = TRUE;
			}

			if(pDmaChannel) {
				pEnd->dwRequestSize = min(pRequest->length, 
					pDmaChannel->dwMaxLength);
				bDmaOk = pDmaController->pfDmaProgramChannel(pDmaChannel, 
				pEnd->wPacketSize, TRUE, pFifoDest, pEnd->dwRequestSize);
				if(bDmaOk) {
					pDmaChannel->dwActualLength = 0L;
					wFifoCount = 0;
				} else if(bAllocChannel) {
					pDmaController->pfDmaReleaseChannel(pDmaChannel);
					pHcdEnd->pDmaChannel = NULL;
				}
			}
		}
#endif
		if(pEnd->bIsTx) {
			if(wFifoCount) {
				MGC_HdrcLoadFifo(pBase, bEnd, wFifoCount, pFifoDest);
				wCsrVal |= MGC_M_TXCSR_TXPKTRDY;
			}
#ifdef MUSB_DMA
			if(bDmaOk) {
				wCsrVal |= (MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB | 
					MGC_M_TXCSR_DMAMODE);
			}
#endif
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd, pEnd->wPacketSize);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsrVal);
		} else {
			/* handle residual if any */
			wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
			if(wCsrVal & MGC_M_RXCSR_RXPKTRDY) {
				wCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);
				pEnd->dwOffset += wCount;
				if(pEnd->dwOffset < pRequest->length) {
					wFifoCount = min(wCount, pRequest->length - pEnd->dwOffset);
					MGC_HdrcUnloadFifo(pBase, bEnd, wFifoCount, 
					(uint8_t*)pRequest->buf + pEnd->dwOffset);
					wCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
					MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
				}
				if((pEnd->dwOffset >= pRequest->length) || 
					(wCount < pEnd->wPacketSize)) 
				{
					pRequest->status = 0;
					pRequest->actual = pEnd->dwOffset;
					pEnd->dwOffset = 0L;
					MGC_GadgetQueueCompletion(pEnd, pRequest);
					return;
				}
			}
	
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd, pEnd->wPacketSize);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, 0);
			wCsrVal = 0;
			if(USB_ENDPOINT_XFER_ISOC == pEnd->bTrafficType) {
				wCsrVal |= MGC_M_RXCSR_P_ISO;
			}
			if(USB_ENDPOINT_XFER_INT == pEnd->bTrafficType) {
				wCsrVal |= MGC_M_RXCSR_DISNYET;
			}
#ifdef MUSB_DMA
			if(bDmaOk) {
				wCsrVal |= (MGC_M_RXCSR_AUTOCLEAR | MGC_M_RXCSR_DMAENAB | 
					MGC_M_RXCSR_DMAMODE);
			}
#endif
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
		}
    }
}

/**
 * Handle default endpoint interrupt as device. This is the entry point
 * from plat_uds.
 * @param pThis this
 */
void MGC_HdrcServiceDeviceDefaultEnd(MGC_LinuxCd* pThis)
{
    uint16_t wCsrVal, wCount, wTest;
    uint8_t bEnd, bRecip, bType, bNum;
    uint8_t bError = FALSE, bDone = FALSE, bHandled = FALSE, bStall = FALSE;
    uint8_t bResult[4];
    uint8_t* pFifoDest = NULL;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[0]);
    struct usb_request* pRequest = pEnd->pRequest;
    struct usb_ctrlrequest* pControlRequest = (struct usb_ctrlrequest*)pThis->aEnd0Data;

    MGC_SelectEnd(pBase, 0);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
    	wCount = MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0);
	
    DEBUG_CODE(3, printk(KERN_INFO "<== wCsr=%04x, wCount=%04x, pThis->bEnd0Stage=%d\n", 
	wCsrVal, wCount, pThis->bEnd0Stage); )
	
    if(!wCsrVal && !wCount && (MGC_END0_STATUS == pThis->bEnd0Stage)) {
		pThis->bEnd0Stage = MGC_END0_START;
		DEBUG_CODE(3, printk(KERN_INFO "ack interrupt\n"); )
		
		/* update address if needed */
		if(pThis->bSetAddress) {
			pThis->bSetAddress = FALSE;
			MGC_Write8(pBase, MGC_O_HDRC_FADDR, pThis->bAddress);
			DEBUG_CODE(3, printk(KERN_INFO "Address is set to pThis->bAddress=%d\n", 
			pThis->bAddress); )
		}
		
		/* enter test mode if needed */
		if(pThis->bTestMode) {
			printk(KERN_INFO "entering core test mode %d\n", pThis->bTestModeValue);
			if(MGC_M_TEST_PACKET == pThis->bTestModeValue) {
				MGC_HdrcLoadFifo(pBase, 0, sizeof(MGC_aTestPacket), 
				MGC_aTestPacket);
				/* despite explicit instruction, we still must kick-start */
				MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_TXPKTRDY);
			}
			MGC_Write8(pBase, MGC_O_HDRC_TESTMODE, pThis->bTestModeValue);
		}
		
		MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
		return;
    }
	
    if(wCsrVal & MGC_M_CSR0_P_SENTSTALL) {
		DEBUG_CODE(3, printk(KERN_INFO "sentstall\n"); )
		pThis->bEnd0Stage = MGC_END0_START;
		MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
		bError = TRUE;
    }
	
    if(wCsrVal & MGC_M_CSR0_P_SETUPEND) {
		DEBUG_CODE(3, printk(KERN_INFO "setup end\n"); )
		pThis->bEnd0Stage = MGC_END0_START;
		MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_P_SVDSETUPEND);
		bError = TRUE;
    }
	
    if(wCsrVal & MGC_M_CSR0_RXPKTRDY) {
		DEBUG_CODE(3, printk(KERN_INFO "rcvp pkt\n"); )
		
		/* packet received */
		wCsrVal = MGC_M_CSR0_P_SVDRXPKTRDY;
		
		/* see where we are */
		if(MGC_END0_START == pThis->bEnd0Stage) {
			/* first packet; starts with control request */
			pFifoDest = pThis->aEnd0Data;
			wFifoCount = min(wCount, MUSB_MAX_END0_PACKET);
		} else {
			/* additional packet */
			pFifoDest = (void*)(((unsigned long)pThis->aEnd0Data)+
			((unsigned long)pThis->wEnd0Offset));
			wFifoCount = min(wCount, 
			MUSB_MAX_END0_PACKET - pThis->wEnd0Offset);
		}
		
		/* unload FIFO */
		MGC_HdrcUnloadFifo(pBase, 0, wFifoCount, pFifoDest);
		pThis->wEnd0Offset += wFifoCount;
		
		
		if(MGC_END0_START == pThis->bEnd0Stage) {
			/* re-order standard request */
			le16_to_cpus(pControlRequest->wValue);
			le16_to_cpus(pControlRequest->wIndex);
			le16_to_cpus(pControlRequest->wLength);
			
			DEBUG_CODE(3, printk(KERN_INFO "bRequest=%02x, bmRequestType=%02x, wValue=%04x, wIndex=%04x, wLength=%04x\n", \
			pControlRequest->bRequest, pControlRequest->bRequestType, \
			pControlRequest->wValue, pControlRequest->wIndex, \
			pControlRequest->wLength); )
			
			/* see if we got everything and update stage */
			if(pControlRequest->bRequestType & USB_DIR_IN) {
				pThis->bEnd0Stage = MGC_END0_IN;
				MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
				wCsrVal = 0;
				bDone = TRUE;
			} else {
				pThis->bEnd0Stage = MGC_END0_OUT;
				bDone = (0 == (wCount - 8 - pControlRequest->wLength));
			}
			
			if(bDone) {
				/* see if we handle it */
				bRecip = pControlRequest->bRequestType & USB_RECIP_MASK;
				
				if(!pThis->pGadgetDriver && 
					(USB_TYPE_CLASS & 
					(pControlRequest->bRequestType & USB_TYPE_MASK)) && 
					(USB_RECIP_INTERFACE == bRecip))
				{
					switch(pControlRequest->bRequest) {
						case 0xfe:
						/* get-max-lun */
						bHandled = TRUE;
						MGC_HdrcLoadFifo(pBase, 0, 1, &MGC_TestMaxLun);
						wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
						pThis->bEnd0Stage = MGC_END0_IN;
						break;
						case 0xff:
						/* BOT reset */
						MGC_SelectEnd(pBase, 1);
						MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR,
						1, MGC_M_TXCSR_P_SENDSTALL | MGC_M_TXCSR_MODE);
						MGC_SelectEnd(pBase, 2);
						MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR,
						2, MGC_M_RXCSR_P_SENDSTALL);
						MGC_SelectEnd(pBase, 0);
						bHandled = TRUE;
						break;
					}
				}
				
				if(USB_TYPE_STANDARD == 
					(pControlRequest->bRequestType & USB_TYPE_MASK))
				{
					switch(pControlRequest->bRequest) {
						case USB_REQ_GET_STATUS:
						switch(bRecip) {
							case USB_RECIP_DEVICE:
							bHandled = TRUE;
							if(!pThis->pGadgetDriver) {
								pThis->bIsSelfPowered = TRUE;
							}
							if((pThis->bDeviceState >= MGC_STATE_ADDRESS) ||
								(0 == pControlRequest->wIndex))
							{
								bResult[0] = pThis->bIsSelfPowered ? 1 : 0;
								//bResult[0] |= 2;
								bResult[1] = 0;
								MGC_HdrcLoadFifo(pBase, 0, 2, 
								(uint8_t*)&bResult);
								wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
								pThis->bEnd0Stage = MGC_END0_IN;
							} else {
								bStall = TRUE;
							}
							break;
							case USB_RECIP_ENDPOINT:
							bHandled = TRUE;
							if((pThis->bDeviceState > MGC_STATE_ADDRESS) ||
								(0 == pControlRequest->wIndex))
							{
								bEnd = (uint8_t)pControlRequest->wIndex;
								MGC_SelectEnd(pBase, bEnd);
								wTest = MGC_ReadCsr16(pBase, 
								MGC_O_HDRC_TXCSR, bEnd);
								MGC_SelectEnd(pBase, 0);
								bResult[0] = (wTest & MGC_M_TXCSR_P_SENDSTALL) 
								? 1 : 0;
								bResult[1] = 0;
								MGC_HdrcLoadFifo(pBase, 0, 2, 
								(uint8_t*)&bResult);
								wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
								pThis->bEnd0Stage = MGC_END0_IN;
							} else {
								bStall = TRUE;
							}
							break;
						}
						/* END: GET_STATUS */
						break;
						
						case USB_REQ_CLEAR_FEATURE:
						if((pThis->bDeviceState > MGC_STATE_ADDRESS) ||
							(0 == pControlRequest->wIndex))
						{
							switch(bRecip) {
								case USB_RECIP_DEVICE:
								case USB_RECIP_INTERFACE:
								bHandled = TRUE;
								break;
								case USB_RECIP_ENDPOINT:
								bEnd = (uint8_t)pControlRequest->wIndex;
								MGC_SelectEnd(pBase, bEnd);
								/* clear for Tx */
								wTest = MGC_ReadCsr16(pBase, 
								MGC_O_HDRC_TXCSR, bEnd);
								wTest &= ~MGC_M_TXCSR_P_SENDSTALL;
								MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR,
								bEnd, wTest);
								/* clear for Rx */
								wTest = MGC_ReadCsr16(pBase, 
								MGC_O_HDRC_RXCSR, bEnd);
								wTest &= ~MGC_M_RXCSR_P_SENDSTALL;
								MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR,
								bEnd, wTest);
								MGC_SelectEnd(pBase, 0);
								bHandled = TRUE;
								break;
							}
						} else {
							bStall = TRUE;
						}
						/* END: CLEAR_FEATURE */
						break;
							case USB_REQ_SET_FEATURE:
							if((pThis->bDeviceState > MGC_STATE_ADDRESS) ||
								(0 == pControlRequest->wIndex)) {
									
								switch(bRecip)	{
									case USB_RECIP_DEVICE:
									switch(pControlRequest->wValue) {
										case 1:
										/* remote wakeup */
										while (0) { } 
										break;
										case 2:
										if(pControlRequest->wIndex & 0xff) {
											bStall = TRUE;
										} else {
											wTest = (uint8_t)(pControlRequest->wIndex >> 8);
											bHandled = TRUE;
											pThis->bTestMode = TRUE;
											printk(KERN_INFO "Entering TEST_MODE(%d)\n", wTest);
											switch(wTest) {
												case 1:
												/* TEST_J */
												pThis->bTestModeValue = MGC_M_TEST_J;
												break;
												case 2:
												/* TEST_K */
												pThis->bTestModeValue = MGC_M_TEST_K;
												break;
												case 3:
												/* TEST_SE0_NAK */
												pThis->bTestModeValue = MGC_M_TEST_SE0_NAK;
												break;
												case 4:
												/* TEST_PACKET */
												pThis->bTestModeValue = MGC_M_TEST_PACKET;
												break;
												default:
												bStall = TRUE;
												pThis->bTestMode = FALSE;
											}
										}
										break;
										case 3:
										#ifdef MUSB_OTG
										MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
										pControlRequest->wValue);
										#endif
										break;
										case 4:
										#ifdef MUSB_OTG
										MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
										pControlRequest->wValue);
										#endif
										break;
										case 5:
										#ifdef MUSB_OTG
										MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
										pControlRequest->wValue);
										#endif
										break;
									}
									bHandled = TRUE;
									break;
									case USB_RECIP_INTERFACE:
									bHandled = TRUE;
									break;
									case USB_RECIP_ENDPOINT:
									bEnd = (uint8_t)pControlRequest->wIndex;
									MGC_SelectEnd(pBase, bEnd);
									/* set for Tx */
									wTest = MGC_ReadCsr16(pBase, 
									MGC_O_HDRC_TXCSR, bEnd);
									wTest |= MGC_M_TXCSR_P_SENDSTALL;
									MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR,
									bEnd, wTest);
									/* set for Rx */
									wTest = MGC_ReadCsr16(pBase, 
									MGC_O_HDRC_RXCSR, bEnd);
									wTest |= MGC_M_RXCSR_P_SENDSTALL;
									MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR,
									bEnd, wTest);
									MGC_SelectEnd(pBase, 0);
									bHandled = TRUE;
									break;
								}
							} else {
								bStall = TRUE;
							}
							/* END: SET_FEATURE */
						break;
						
						case USB_REQ_GET_INTERFACE:
							/* handle this if there is no gadget driver */
							if(!pThis->pGadgetDriver) {
								bHandled = TRUE;
								MGC_HdrcLoadFifo(pBase, 0, 1, &MGC_TestInterface);
								wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
								pThis->bEnd0Stage = MGC_END0_IN;
							}
						break;
						
						case USB_REQ_SET_INTERFACE:
						/* handle this if there is no gadget driver */
						if(!pThis->pGadgetDriver) {
							bHandled = TRUE;
						}
						break;
						
						case USB_REQ_GET_CONFIGURATION:
							bHandled = TRUE;
							MGC_HdrcLoadFifo(pBase, 0, 1, &MGC_bConfiguration);
							wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
							pThis->bEnd0Stage = MGC_END0_IN;
						break;
						
						case USB_REQ_SET_CONFIGURATION:
							/* remember state but do NOT handle */
							MGC_bConfiguration = pControlRequest->wValue & 0xff;
							if(MGC_bConfiguration) {
								pThis->bDeviceState = MGC_STATE_CONFIGURED;
							} else {
								pThis->bDeviceState = MGC_STATE_ADDRESS;
							}
							/* BUT, handle this if there is no gadget driver */
							if(!pThis->pGadgetDriver) {
								bHandled = TRUE;
							}
						break;
						
						case USB_REQ_SET_ADDRESS:
							pThis->bAddress = (uint8_t)(pControlRequest->wValue & 0x7f);
							pThis->bSetAddress = TRUE;
							bHandled = TRUE;
							pThis->bDeviceState = MGC_STATE_ADDRESS;
							MGC_bConfiguration = 0;
							MGC_bHighSpeed = (MGC_Read8(pBase, MGC_O_HDRC_POWER) & MGC_M_POWER_HSMODE) ? 1 : 0;
							if(!pThis->pGadgetDriver) {
								MGC_TestPacketSize = MGC_bHighSpeed ? 512 : 64;
								MGC_SelectEnd(pBase, 1);
								MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, 1, MGC_TestPacketSize);
								MGC_SelectEnd(pBase, 0);
							}
						break;
						
						case USB_REQ_GET_DESCRIPTOR:
						DEBUG_CODE(3, printk(KERN_INFO "getting a descriptor\n"); )
						
						/* handle this only if there is no gadget driver */
						if(!pThis->pGadgetDriver && (USB_RECIP_DEVICE == bRecip)) {
							bType = (uint8_t)(pControlRequest->wValue >> 8);
							switch(bType) {
								case USB_DT_DEVICE:
									bHandled = TRUE;
									MGC_HdrcLoadFifo(pBase, 0, min(pControlRequest->wLength, 
									sizeof(MGC_aTestDeviceDesc)), 
									MGC_aTestDeviceDesc);
									wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
									pThis->bEnd0Stage = MGC_END0_IN;
									DEBUG_CODE(3, printk(KERN_INFO "it's DEVICE descriptor\n"); )
								break;
								
								case USB_DT_CONFIG:
									bHandled = TRUE;
									if(MGC_bHighSpeed) {
										wFifoCount = min(pControlRequest->wLength, 
										sizeof(MGC_aTestConfigDesc));
										MGC_HdrcLoadFifo(pBase, 0, wFifoCount, 
										MGC_aTestConfigDesc);
									} else {
										wFifoCount = min(pControlRequest->wLength, 
										sizeof(MGC_aTestFullSpdConfigDesc)) - 4;
										memcpy(bResult, MGC_aTestFullSpdConfigDesc, 4);
										bResult[1] = USB_DT_CONFIG;
										MGC_HdrcLoadFifo(pBase, 0, 4, bResult);
										MGC_HdrcLoadFifo(pBase, 0, wFifoCount, 
										MGC_aTestFullSpdConfigDesc + 4);
									}
									wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
									pThis->bEnd0Stage = MGC_END0_IN;
								break;
								
								case USB_DT_STRING:
									bHandled = TRUE;
									bNum = (uint8_t)(pControlRequest->wValue & 0xff);
									switch(bNum) {
										case 0:
										pFifoDest = (uint8_t*)MGC_aString0;
										wFifoCount = sizeof(MGC_aString0);
										break;
										case 1:
										pFifoDest = (uint8_t*)MGC_aString1Lang0409;
										wFifoCount = sizeof(MGC_aString1Lang0409);
										break;
										case 2:
										pFifoDest = (uint8_t*)MGC_aString2Lang0409;
										wFifoCount = sizeof(MGC_aString2Lang0409);
										break;
										case 3:
										pFifoDest = (uint8_t*)MGC_aString3Lang0409;
										wFifoCount = sizeof(MGC_aString3Lang0409);
										break;
										default:
										bHandled = FALSE;
									}
									
									if(bHandled) {
										MGC_HdrcLoadFifo(pBase, 0, 
										min(pControlRequest->wLength, wFifoCount),
										pFifoDest);
										wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
										pThis->bEnd0Stage = MGC_END0_IN;
									}
								break;
								
								case 6:
									bHandled = TRUE;
									MGC_HdrcLoadFifo(pBase, 0, 
									min(pControlRequest->wLength, 
									sizeof(MGC_aTestDeviceQual)), 
									MGC_aTestDeviceQual);
									wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
									pThis->bEnd0Stage = MGC_END0_IN;
								break;
								
								case 7:
									bHandled = TRUE;
									if(MGC_bHighSpeed) {
										MGC_HdrcLoadFifo(pBase, 0, 
										min(pControlRequest->wLength, 
										sizeof(MGC_aTestFullSpdConfigDesc)), 
										MGC_aTestFullSpdConfigDesc);
									} else {
										wFifoCount = min(pControlRequest->wLength, 
										sizeof(MGC_aTestConfigDesc)) - 4;
										memcpy(bResult, MGC_aTestConfigDesc, 4);
										bResult[1] = 7;
										MGC_HdrcLoadFifo(pBase, 0, 4, bResult);
										MGC_HdrcLoadFifo(pBase, 0, wFifoCount, 
										MGC_aTestConfigDesc + 4);
									}
									wCsrVal = MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_P_DATAEND;
									pThis->bEnd0Stage = MGC_END0_IN;
								break;
							}	/* END: switch(bType) */
						}	/* END: if no gadget driver registered */
					}		/* END: switch(bRequest) */
				}		/* END: if standard request */
			}			/* END: if(bDone) */
		}			/* END: if stage==start */
		
		
		if(!bHandled && bDone) {
			/* we didn't handle it, so call driver */
			DEBUG_CODE(2, printk(KERN_INFO "passing to driver: bRequest=%02x, bmRequestType=%02x, wValue=%04x, \
			wIndex=%04x, wLength=%04x\n", \
			pControlRequest->bRequest, \
			pControlRequest->bRequestType, pControlRequest->wValue, \
			pControlRequest->wIndex, pControlRequest->wLength); )
			
			bStall = TRUE;
			pThis->wEnd0Offset = 0;
#if 0
			if(pThis->pGadgetDriver &&
				(pThis->pGadgetDriver->setup(pThis->pGadget, 
			(struct usb_ctrlrequest*)pThis->pEnd0Buffer) >= 0))
			{
				bStall = FALSE;
			}
#endif
			
			DEBUG_CODE(2, printk(KERN_INFO "called driver; status %s\n", bStall ? "stall" : "OK"); )
			
		}
		
		
		if(bStall) {
			DEBUG_CODE(2, printk(KERN_INFO "stalled: bRequest=%02x, bmRequestType=%02x, wValue=%04x, wIndex=%04x, wLength=%04x\n", \
			pControlRequest->bRequest, pControlRequest->bRequestType, \
			pControlRequest->wValue, pControlRequest->wIndex, pControlRequest->wLength); )
			wCsrVal |= MGC_M_CSR0_P_SENDSTALL;
		}
		
		if(bDone && (MGC_END0_OUT == pThis->bEnd0Stage)) {
			pThis->bEnd0Stage = MGC_END0_STATUS;
			wCsrVal |= MGC_M_CSR0_P_DATAEND;
		}
		
    } else if(!bError) {
		DEBUG_CODE(3, printk(KERN_INFO "ack to a trasmission (dataend?)\n"); )
		
		/* no RxPktRdy; we must have transmitted something */
		if(pEnd->pRequest && (MGC_END0_IN == pThis->bEnd0Stage)) {
			pFifoDest = (uint8_t*)pEnd->pRequest->buf + 
			pThis->wEnd0Offset;
			wFifoCount = min(MUSB_MAX_END0_PACKET,
			pEnd->pRequest->length - pThis->wEnd0Offset);
			if(wFifoCount || pEnd->pRequest->zero) {
				MGC_HdrcLoadFifo(pBase, 0, wFifoCount, pFifoDest);
				wCsrVal |= MGC_M_CSR0_TXPKTRDY;
			}
			/* set DATAEND on last packet */
			if(wFifoCount < MUSB_MAX_END0_PACKET) {
				pThis->bEnd0Stage = MGC_END0_STATUS;
				wCsrVal |= MGC_M_CSR0_P_DATAEND;
			}
		} else {
			pThis->bEnd0Stage = MGC_END0_START;
			wCsrVal |= MGC_M_CSR0_P_DATAEND;
		}
    }
	
	
    /* program core */
    if(wCsrVal && !bError) {
		DEBUG_CODE(3, printk(KERN_INFO "programming core for transmission wCsrVal=0x%0x\n", wCsrVal); )
		MGC_SelectEnd(pBase, 0);
		MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
    }
	
}

void MGC_GadgetQueueCompletion(MGC_GadgetLocalEnd* pEnd, 
struct usb_request* pRequest)
{
    pEnd->CompletionData.pRequest = pRequest;
    tasklet_schedule(&MGC_GadgetTasklet);
}

void MGC_HdrcServiceDeviceTxAvail(MGC_LinuxCd* pThis, uint8_t bEnd)
{
    uint16_t wVal;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_LinuxLocalEnd* pLocalEnd = &(pThis->aLocalEnd[bEnd]);
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;

    MGC_SelectEnd(pBase, bEnd);
    wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

    do {
		if(wVal & MGC_M_TXCSR_P_SENTSTALL) {
			wVal &= ~MGC_M_TXCSR_P_SENTSTALL;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
			break;
		}
		if(wVal & MGC_M_TXCSR_P_UNDERRUN) {
#ifdef MUSB_CONFIG_PROC_FS
			pThis->aLocalEnd[bEnd].dwMissedTxPackets++;
#endif
			wVal &= ~MGC_M_TXCSR_P_UNDERRUN;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
		}

#ifdef MUSB_DMA
	if(pEnd->pDmaChannel) {
	    if( MGC_DMA_STATUS_FREE == 
			pThis->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel))
	    {
			pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
	    }
	} else
#endif
	    pEnd->dwOffset += pEnd->dwRequestSize;

		if(!pThis->pGadgetDriver && MGC_pTestTx) {
			MGC_dwTestTxOffset += MGC_dwTestTxSize;
			if(MGC_dwTestTxOffset < MGC_dwTestTxCount) {
				wFifoCount = min(MGC_TestPacketSize, (MGC_dwTestTxCount - MGC_dwTestTxOffset));
				MGC_dwTestTxSize = wFifoCount;
				MGC_HdrcLoadFifo(pBase, bEnd, wFifoCount, MGC_pTestTx + MGC_dwTestTxOffset);
				MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, (MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE));
			} else {
				MGC_TestTxComplete();
			}
			return;
		}
	
		if(pRequest) {
			if(pEnd->dwOffset < pRequest->length) {
				/* for now, assume any DMA controller can move maximum-length request */
				wFifoCount = min(pLocalEnd->wMaxPacketSizeTx, (pRequest->length - pEnd->dwOffset));
				MGC_HdrcLoadFifo(pBase, bEnd, wFifoCount, (uint8_t*)pRequest->buf + pEnd->dwOffset);
				MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, (MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE));
			} else {
				pRequest->status = 0;
				pRequest->actual = pEnd->dwOffset;
				pEnd->dwOffset = 0L;
				MGC_GadgetQueueCompletion(pEnd, pRequest);
				/* next */
				if(list_empty(&pEnd->req_list)) {
					pEnd->pRequest = NULL;
				} else {
					pRequest = list_entry(pEnd->req_list.next,
					struct usb_request, list);
					list_del(&pRequest->list);
					pEnd->pRequest = pRequest;
				}
			}
		}
	
    } while(FALSE);
}

/**
 *
 */
void MGC_HdrcServiceDeviceRxReady(MGC_LinuxCd* pThis, uint8_t bEnd)
{
    uint16_t wCsrVal, wCount;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;

    MGC_SelectEnd(pBase, bEnd);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
    wCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);

    do {
		if(wCsrVal & MGC_M_RXCSR_P_SENTSTALL) {
			wCsrVal &= ~MGC_M_RXCSR_P_SENTSTALL;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
			break;
		}
	
		if(wCsrVal & MGC_M_RXCSR_P_OVERRUN) {
#ifdef MUSB_CONFIG_PROC_FS
			pThis->aLocalEnd[bEnd].dwMissedRxPackets++;
#endif
			wCsrVal &= ~MGC_M_RXCSR_P_OVERRUN;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
			//break;
		}
		
#ifdef MUSB_CONFIG_PROC_FS
		if(wCsrVal & MGC_M_RXCSR_INCOMPRX) {
			pThis->aLocalEnd[bEnd].dwErrorRxPackets++;
		}
#endif
#ifdef MUSB_DMA
		if(pEnd->pDmaChannel) {
			if(MGC_DMA_STATUS_FREE == 
				pThis->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel))
			{
				pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
			}
		}
		else
#endif
			pEnd->dwOffset += wCount;
	
		if(!pThis->pGadgetDriver && MGC_pTestRx) {
			if(MGC_dwTestRxOffset < MGC_dwTestRxCount) {
				/* for now, assume any DMA controller can move maximum-length request */
				wFifoCount = min(wCount, MGC_dwTestRxCount - MGC_dwTestRxOffset);
				MGC_HdrcUnloadFifo(pBase, bEnd, wFifoCount, MGC_pTestRx + MGC_dwTestRxOffset);
				MGC_dwTestRxOffset += wFifoCount;
				wCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
				MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
			}
			if((MGC_dwTestRxOffset >= MGC_dwTestRxCount) || (wCount < MGC_TestPacketSize)) {
				MGC_dwTestRxOffset = 0L;
				MGC_TestRxComplete();
			}
			return;
		}
	
		if(pRequest) {
			if(pEnd->dwOffset < pRequest->length) {
				/* for now, assume any DMA controller can move maximum-length request */
				wFifoCount = min(wCount, pRequest->length - pEnd->dwOffset);
				MGC_HdrcUnloadFifo(pBase, bEnd, wFifoCount, (uint8_t*)pRequest->buf + pEnd->dwOffset);
				pEnd->dwOffset += wFifoCount;
				wCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
				MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
			}
			if((pEnd->dwOffset >= pRequest->length) || (wCount < pEnd->wPacketSize)) {
				pRequest->status = 0;
				pRequest->actual = pEnd->dwOffset;
				pEnd->dwOffset = 0L;
				MGC_GadgetQueueCompletion(pEnd, pRequest);
				/* next */
				if(list_empty(&pEnd->req_list)) {
					pEnd->pRequest = NULL;
				} else {
					pRequest = list_entry(pEnd->req_list.next,
					struct usb_request, list);
					list_del(&pRequest->list);
					pEnd->pRequest = pRequest;
					MGC_HdrcProgramDeviceEnd(pThis, bEnd);
				}
			}
		}
    } while(FALSE);
}

/**
 * The endpoint selection policy should have already selected
 * an appropriate usb_ep from us, since we follow the naming convention.
 */
int MGC_GadgetEnableEnd(struct usb_ep *ep,
			       const struct usb_endpoint_descriptor *desc)
{
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(3, printk(KERN_INFO "enabling ep %d\n", MGC_GadgetFindEnd(ep) ); )

    if(!pThis || (nEnd < 0) || 
       (nEnd != (desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
    {
        return -EINVAL;
    }
    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
//    spin_lock(&pEnd->Lock);
    pEnd->wPacketSize = desc->wMaxPacketSize;
    pEnd->bTrafficType = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    pEnd->bIsTx = (desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? 
      TRUE : FALSE;
    pEnd->pRequest = NULL;
//    spin_unlock(&pEnd->Lock);

    return 0;
}

/**
 * Disable an endpoint flushing the request queued.
 *
 * @param struct usb_ep *ep the endpoint to disable.
 */
int MGC_GadgetDisableEnd(struct usb_ep *ep)
{
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    int nEnd = MGC_GadgetFindEnd(ep);

    if(!pThis || (nEnd < 0)) {
        return -EINVAL;
    }
    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
//    spin_lock_irqsave(&pEnd->Lock, flags);
    spin_lock(&pEnd->Lock);
    pEnd->pRequest = NULL;
//    spin_unlock_irqrestore(&pEnd->Lock, flags);
    spin_unlock(&pEnd->Lock);

    return 0;
}

/**
 * Allocate a request for an endpoint.
 * @param ep
 * @param gfp_flags
 */
struct usb_request* MGC_GadgetAllocRequest(struct usb_ep *ep,
						  int gfp_flags)
{
    struct usb_request* pResult=NULL;

    KMALLOC(pResult, sizeof(struct usb_request), gfp_flags);
    return pResult;
}

/**
 * Free a request
 *
 * @param ep
 * @param req
 */
void MGC_GadgetFreeRequest(struct usb_ep *ep, struct usb_request *req)
{
    KFREE(req);
}

int MGC_GadgetQueue(struct usb_ep *ep, struct usb_request *req,
			   int gfp_flags)
{
    uint16_t wLength;
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    int nEnd = MGC_GadgetFindEnd(ep);

    if(!pThis || (nEnd < 0) || (nEnd > 16)) {
        return -EINVAL;
    }

    /* if end0 Rx, we already grabbed data; just copy and complete */
    if(!nEnd && (MGC_END0_OUT == pThis->bEnd0Stage)) {
        wLength = min((uint16_t)req->length, (pThis->wEnd0Offset - 8));
		memcpy(req->buf, (void*)(uint8_t*)pThis->pEnd0Buffer + 8, wLength);
		DEBUG_CODE(2, printk(KERN_INFO "satisfying request with %d bytes residual\n", wLength); )
		req->status = 0;
		if(req->complete) {
			req->complete(&MGC_GadgetEnd0, req);
		}
		return 0;
    }

    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
//    spin_lock_irqsave(&pEnd->Lock, flags);
    spin_lock(&pEnd->Lock);
    if(pEnd->pRequest) {
		DEBUG_CODE(2, printk(KERN_INFO "end busy; queueing request\n" ); )
		list_add_tail(&(req->list), &(pEnd->req_list));
    } else {
		pEnd->pRequest = req;
		
		if( MUSB_IS_DEV(pThis) ) {
			MGC_HdrcProgramDeviceEnd(pThis, (uint8_t)nEnd);
		}
    }
//    spin_unlock_irqrestore(&pEnd->Lock, flags);
    spin_unlock(&pEnd->Lock);
    return 0;
}

/**
 * Dequeue a request
 *
 * @param ep the endpoint
 * @param pRequest the request to dequeue  
 */
int MGC_GadgetDequeue(struct usb_ep *ep, struct usb_request *pRequest)
{
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    int nEnd = MGC_GadgetFindEnd(ep);

    if(!pThis || (nEnd < 0)) {
        return -EINVAL;
    }
    
	pEnd = &(MGC_aGadgetLocalEnd[nEnd]);

    spin_lock(&pEnd->Lock);

    if(pRequest == pEnd->pRequest) {
		/* TODO: stop end */
	
    }
    list_del(&pRequest->list);
    spin_unlock(&pEnd->Lock);

    return 0;
}

/**
 * Set clear the halt bit of an endpoint. A halted enpoint won't tx/rx any
 * data but will queue requests. 
 * 
 * @param ep the endpoint
 * @param value != 0 => halt, 0 == active  
 */
int MGC_GadgetSetHalt(struct usb_ep *ep, int value)
{
    uint16_t wCsr;
    uint8_t* pBase;
    unsigned long flags;
    MGC_LinuxLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    int nEnd = MGC_GadgetFindEnd(ep);

    if(!pThis || (nEnd < 0) || !MUSB_IS_DEV(pThis) ) {
        return -EINVAL;
    }
    pBase = pThis->pRegs;

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, (uint8_t)nEnd);
    pEnd = &(pThis->aLocalEnd[nEnd]);
    spin_lock(&(pEnd->Lock));
    if(nEnd)
    {
        /* transmit */
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd);
	if(value)
	{
	    wCsr |= MGC_M_TXCSR_P_SENDSTALL;
	}
	else
	{
	    wCsr &= ~MGC_M_TXCSR_P_SENDSTALL;
	}
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd, wCsr);
	/* receive */
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd);
	if(value)
	{
	    wCsr |= MGC_M_RXCSR_P_SENDSTALL;
	}
	else
	{
	    wCsr &= ~MGC_M_RXCSR_P_SENDSTALL;
	}
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd, wCsr);
    }
    else
    {
        /* stalling end 0 is bad... */
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
	if(value)
	{
	    wCsr |= MGC_M_CSR0_P_SENDSTALL;
	}
	else
	{
	    wCsr &= ~MGC_M_CSR0_P_SENDSTALL;
	}
	MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);
    }
    spin_unlock(&(pEnd->Lock));
    spin_unlock_irqrestore(&(pThis->Lock), flags);

    return 0;
}

