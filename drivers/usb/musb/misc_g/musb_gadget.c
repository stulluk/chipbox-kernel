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

#include <linux/kernel.h>
#include <linux/usb.h>

#include "musbdefs.h"
 

static char MGC_aGadgetEndName[6][MUSB_C_NUM_EPS];
static struct usb_ep MGC_aGadgetEnd[MUSB_C_NUM_EPS];
static MGC_GadgetLocalEnd MGC_aGadgetLocalEnd[MUSB_C_NUM_EPS];

/**************************************************************************
Gadget Functions
**************************************************************************/

/* endpoint operations */
STATIC int MGC_GadgetEnableEnd(struct usb_ep *ep,
	const struct usb_endpoint_descriptor *desc);
STATIC int MGC_GadgetDisableEnd(struct usb_ep *ep);

STATIC struct usb_request* MGC_GadgetAllocRequest(struct usb_ep *ep,
	int gfp_flags);
STATIC void MGC_GadgetFreeRequest(struct usb_ep *ep, struct usb_request *req);
STATIC void* MGC_GadgetAllocBuffer(struct usb_ep *ep, unsigned bytes,
	dma_addr_t *dma, int gfp_flags);
STATIC void MGC_GadgetFreeBuffer(struct usb_ep *ep, void *buf, dma_addr_t dma,
	unsigned bytes);
STATIC int MGC_GadgetQueue(struct usb_ep *ep, struct usb_request *req,
	int gfp_flags);
STATIC int MGC_GadgetDequeue(struct usb_ep *ep, struct usb_request *req);
STATIC int MGC_GadgetSetHalt(struct usb_ep *ep, int value);
STATIC int MGC_GadgetFifoStatus(struct usb_ep *ep);
STATIC void MGC_GadgetFifoFlush(struct usb_ep *ep);

/* general operations */
STATIC int MGC_GadgetGetFrame(struct usb_gadget *gadget);
STATIC int MGC_GadgetWakeup(struct usb_gadget *gadget);
STATIC int MGC_GadgetSetSelfPowered(struct usb_gadget *gadget, 
				    int is_selfpowered);
STATIC int MGC_GadgetVbusSession(struct usb_gadget *gadget, int is_active);
STATIC int MGC_GadgetVbusDraw(struct usb_gadget *gadget, unsigned mA);
STATIC int MGC_GadgetPullup(struct usb_gadget *gadget, int is_on);
STATIC int MGC_GadgetIoctl(struct usb_gadget *gadget,
			   unsigned code, unsigned long param);

static struct usb_ep_ops MGC_GadgetEndpointOperations =
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

static struct usb_gadget_ops MGC_GadgetOperations =
{
	MGC_GadgetGetFrame,
	MGC_GadgetWakeup,
	MGC_GadgetSetSelfPowered,
#ifdef MUSB_OTG
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
    MGC_END0_FIFOSIZE
};

/**************************************************************************
Gadget Functions
**************************************************************************/

STATIC int MGC_GadgetFindEnd(struct usb_ep* pGadgetEnd)
{
    uint8_t bEnd;
    int nResult = -1;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(pGadgetEnd == &MGC_GadgetEnd0)
    {
        nResult = 0;
    }
    else
    {
        for(bEnd = 0; bEnd < MUSB_C_NUM_EPS; bEnd++)
	{
	    if(pGadgetEnd == &(MGC_aGadgetEnd[bEnd]))
	    {
	        nResult = bEnd;
		break;
	    }
	}
    }
    return nResult;

}

/**
 */
STATIC void MGC_HdrcProgramDeviceEnd(MGC_LinuxCd* pThis, uint8_t bEnd)
{
#ifdef MUSB_DMA
    MUSB_DmaController* pDmaController;
    MUSB_DmaChannel* pDmaChannel;
    uint8_t bAllocChannel
    uint8_t bDmaOk = FALSE;
#endif
    uint16_t wCsrVal;
    uint8_t* pFifoDest = NULL;
    uint16_t wFifoCount = 0;
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    MGC_LinuxLocalEnd* pHcdEnd = &(pThis->aLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;
    uint8_t bIsBulk = (USB_ENDPOINT_XFER_BULK == pEnd->bTrafficType);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    pFifoDest = (uint8_t*)pRequest->buf;
    pEnd->dwOffset = 0L;
    if( 0==bEnd ) {    
    	TRACE( -1 )    
	wCsrVal = MGC_M_CSR0_TXPKTRDY;
#if 0
	/* end0 should only be to transmit an IN control response */
	wFifoCount = min(MUSB_MAX_END0_PACKET, pRequest->length);
	MGC_HdrcLoadFifo(pBase, 0, wFifoCount, pFifoDest);
	pThis->wEnd0Offset = wFifoCount;
#endif
	if(wFifoCount < MUSB_MAX_END0_PACKET)
	{
	    wCsrVal |= MGC_M_CSR0_P_DATAEND;
	}	
	MGC_WriteCsr8(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
    } else {

    	TRACE( -1 )    

#if 0

    	/* prepare CSR */
	wCsrVal = MGC_M_TXCSR_MODE;
	if(USB_ENDPOINT_XFER_ISOC == pEnd->bTrafficType)
	{
	    wCsrVal |= MGC_M_TXCSR_ISO;
	}

	/* determine how much to send (used only for transmit) */
	if(bIsBulk && pThis->bBulkSplit)
	{
	    wFifoCount = min(pHcdEnd->wMaxPacketSizeTx, 
	    	pRequest->length);
	}
	else
	{
	    wFifoCount = min(pEnd->wPacketSize, pRequest->length);
	}
	pEnd->dwRequestSize = wFifoCount;
	
#ifdef MUSB_DMA
	if(bIsBulk && (pRequest->length > pEnd->wPacketSize))
	{
	    /* candidate for DMA */
	    bAllocChannel = FALSE;
	    pDmaController = pThis->pDmaController;
	    pDmaChannel = pHcdEnd->pDmaChannel;
	    if(pDmaController && !pDmaChannel)
	    {
		pDmaChannel = pHcdEnd->pDmaChannel = 
		    pDmaController->pfDmaAllocateChannel(
			pDmaController->pPrivateData, bEnd, TRUE, 
			pEnd->bTrafficType, pEnd->wPacketSize);
		bAllocChannel = TRUE;
	    }
	    if(pDmaChannel)
	    {
	    	pEnd->dwRequestSize = min(pRequest->length, 
		    pDmaChannel->dwMaxLength);
		bDmaOk = pDmaController->pfDmaProgramChannel(pDmaChannel, 
		    pEnd->wPacketSize, TRUE, pFifoDest, pEnd->dwRequestSize);
		if(bDmaOk)
		{
		    pDmaChannel->dwActualLength = 0L;
		    wFifoCount = 0;
		}
		else if(bAllocChannel)
		{
		    pDmaController->pfDmaReleaseChannel(pDmaChannel);
		    pHcdEnd->pDmaChannel = NULL;
		}
	    }
	}
#endif
	if(pEnd->bIsTx)
	{
	    if(wFifoCount)
	    {
		MGC_HdrcLoadFifo(pBase, bEnd, wFifoCount, pFifoDest);
		wCsrVal |= MGC_M_TXCSR_TXPKTRDY;
	    }
#ifdef MUSB_DMA
	    if(bDmaOk)
	    {
	    	wCsrVal |= (MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB | 
	    		MGC_M_TXCSR_DMAMODE);
	    }
#endif
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd, pEnd->wPacketSize);
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsrVal);
	}
	else
	{
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd, pEnd->wPacketSize);
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, 0);
	    wCsrVal = 0;
	    if(USB_ENDPOINT_XFER_ISOC == pEnd->bTrafficType)
	    {
		wCsrVal |= MGC_M_RXCSR_P_ISO;
	    }
	    if(USB_ENDPOINT_XFER_INT == pEnd->bTrafficType)
	    {
		wCsrVal |= MGC_M_RXCSR_DISNYET;
	    }
#ifdef MUSB_DMA
	    if(bDmaOk)
	    {
	    	wCsrVal |= (MGC_M_RXCSR_AUTOCLEAR | MGC_M_RXCSR_DMAENAB | 
	    		MGC_M_RXCSR_DMAMODE);
	    }
#endif
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
	}	
#endif	

    }
}


/**
 * handle a request, the end0 buffer contains the current request
 * that is supposed to be a standard control request. 
 *
 * NOTE: it modifies the content of the End0Buffer
 *
 * @return -EINVAL when a request is not standard
 *
 */
uint8_t MGC_HandleRequest(MGC_LinuxCd* pThis) {
    uint8_t bResult[2];
    uint8_t bStall = FALSE;
    uint8_t bHandled = TRUE;
    uint8_t bEnd;
    uint16_t wTest;
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;
    struct usb_ctrlrequest *pControlRequest=(struct usb_ctrlrequest*)
    	pThis->pEnd0Buffer;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
			
    /* see if we handle it */
    if (USB_TYPE_STANDARD==(pControlRequest->bRequestType&USB_TYPE_MASK)) 
    {
	const uint8_t bRecip=pControlRequest->bRequestType 
    	    & USB_RECIP_MASK;

	le16_to_cpus(pControlRequest->wIndex);
	le16_to_cpus(pControlRequest->wLength);
	le16_to_cpus(pControlRequest->wValue);

	DEBUG_CODE(1, \
	    printk(KERN_INFO \
	    "%s: bRequest=%02x, bmRequestType=%02x, wValue=%04x, \
	    wIndex=%04x, wLength=%04x\n", \
	    __FUNCTION__, pControlRequest->bRequest, \
	    pControlRequest->bRequestType, pControlRequest->wValue, \
	    pControlRequest->wIndex, pControlRequest->wLength); )

	switch(pControlRequest->bRequest)
	{
	case USB_REQ_GET_STATUS:
	    switch(bRecip)
	    {
	    case USB_RECIP_DEVICE:
		if((pThis->bDeviceState > MGC_STATE_ADDRESS) ||
		   (0 == pControlRequest->wIndex))
		{
		    bResult[0] = pThis->bIsSelfPowered ? 1 : 0;
		    bResult[0] |= 2;
		    bResult[1] = 0;
		    MGC_HdrcLoadFifo(pBase, 0, 2, (uint8_t*)&bResult);
		}
		else
		{
		    bStall = TRUE;
		}
		break;
	    case USB_RECIP_ENDPOINT:
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
		}
		else
		{
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
		switch(bRecip)
		{
		case USB_RECIP_DEVICE:
		case USB_RECIP_INTERFACE:
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
		    break;
		}
	    }
	    else
	    {
		bStall = TRUE;
	    }
	    /* END: CLEAR_FEATURE */
	    break;
	case USB_REQ_SET_FEATURE:
	    if((pThis->bDeviceState > MGC_STATE_ADDRESS) ||
	       (0 == pControlRequest->wIndex))
	    {
		switch(bRecip)
		{
		case USB_RECIP_DEVICE:
		    switch(pControlRequest->wValue)
		    {
		    case 1:
			/* remote wakeup */
			while (0) { } 
			break;
		    case 2:
			if(pControlRequest->wIndex & 0xff)
			{
			    bStall = TRUE;
			}
			else
			{
			    pThis->bTestMode = TRUE;
			    wTest = (uint8_t)pControlRequest->wIndex >> 8;
			    switch(wTest)
			    {
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
			    }
			}
			break;
		    case 3:
     #ifdef MUSB_OTG
			pThis->Gadget.b_hnp_enable = 1;
			MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
			    pControlRequest->wValue);
     #endif
			break;
		    case 4:
     #ifdef MUSB_OTG
			pThis->Gadget.a_hnp_support = 1;
			MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
			    pControlRequest->wValue);
     #endif
			break;
		    case 5:
     #ifdef MUSB_OTG
			pThis->Gadget.a_alt_hnp_support = 1;
			MGC_OtgMachineSetFeature(&(pThis->OtgMachine), 
			    pControlRequest->wValue);
     #endif
			break;
		    }
		    break;
		case USB_RECIP_INTERFACE:
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
		    break;
		}
	    }
	    else
	    {
		bStall = TRUE;
	    }
	    /* END: SET_FEATURE */
	    break;

	/* remember state but do NOT handle */
	case USB_REQ_SET_CONFIGURATION:
	    pThis->bDeviceState = (pControlRequest->wValue & 0xff)
		? MGC_STATE_CONFIGURED : MGC_STATE_ADDRESS;
	    break;
	case USB_REQ_SET_ADDRESS:
	    pThis->bAddress = (uint8_t)(pControlRequest->wValue & 0x7f);
	    pThis->bSetAddress = TRUE;
	    pThis->bDeviceState = MGC_STATE_ADDRESS;
	    break;

	/* it's not a standard request, delegate it to the driver */
	default: 
	    bHandled=FALSE;
	    break;
	}
    } else {
	    bHandled=FALSE;
    }
    
    /* try the gadget driver before stalling the system due to 
     * protocol error. */
    if ( !bHandled && pThis->pGadgetDriver ) {    
    	int rc=0;
    
	DEBUG_CODE(1, printk(KERN_INFO "%s: calling gadget driver\n", __FUNCTION__); )
	rc=(pThis->pGadgetDriver->setup(&(pThis->Gadget), 
		(struct usb_ctrlrequest*)pThis->pEnd0Buffer))>=0;
	bStall=rc<0;		
	DEBUG_CODE(1, printk(KERN_INFO "%s: called gadget driver; (bStall=%d) status %s\n", \
	    __FUNCTION__, bStall , bStall ? "stall" : "OK"); )
    } else {
	bStall=TRUE;
    }

    return bStall;
}

/**
 * Read a FULL packet from the hardware.
 *
 *@return 0 when the packet is complet, a negative number when an error 
 * occurred.
 */
static int MGC_HdrcReadUSBControlRequest(MGC_LinuxCd* pThis, uint16_t wCount) {
    uint16_t wLength=0;
    uint16_t offset=sizeof(struct usb_ctrlrequest);
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_End0Buffer *pEnd0Buffer=(MGC_End0Buffer*)pThis->pEnd0Buffer;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__ ); )
    
    if ( !pEnd0Buffer ) {
	KMALLOC(pEnd0Buffer, sizeof(MGC_End0Buffer), GFP_KERNEL);
	if ( !pEnd0Buffer ) {
	    return -ENOMEM;
	}    
	
	memset(pEnd0Buffer, 0, sizeof(MGC_End0Buffer));
	pThis->pEnd0Buffer=pEnd0Buffer;
    }     

    DEBUG_CODE(2, printk(KERN_INFO "=> %s, wCount=%u, pEnd0Buffer->wEnd0Count=%u\n", \
    	__FUNCTION__, wCount, pEnd0Buffer->wEnd0Count); )
	
    if ( !wCount ) {
    	const uint16_t count=pEnd0Buffer->wEnd0Count;
    	if ( count ) { /* ok ONLY when Im not waiting for new data */
	    memset(pEnd0Buffer, 0, sizeof(MGC_End0Buffer));    
	};
    	return (count)?-EINVAL:1; 
    }
        
    /* wEnd0Count=0 means that is im waiting for the usb standard header,
     * that's the first thing I'll read from the FIFO */
    if ( 0==pEnd0Buffer->wEnd0Count ) { 
	if (wCount > (MUSB_MAX_END0_PACKET-sizeof(struct usb_ctrlrequest)) ) {
	    return -EINVAL; /* buffer overrun */
	}	

	/* need to read the first control request */
	if ( wCount<sizeof(struct usb_ctrlrequest)) {
	    return -EINVAL; /** need to read a full usb_ctrlrequest */
	}

	DEBUG_CODE(2, printk(KERN_INFO "%s:%d Reading header\n", \
		__FUNCTION__, __LINE__ ); )

	MGC_HdrcUnloadFifo(pBase, 0, sizeof(struct usb_ctrlrequest), 
		pEnd0Buffer->aEnd0Data);
	wCount-=sizeof(struct usb_ctrlrequest);

	le16_to_cpus( ((struct usb_ctrlrequest*)pEnd0Buffer)->wLength );
	le16_to_cpus( ((struct usb_ctrlrequest*)pEnd0Buffer)->wIndex);
	le16_to_cpus( ((struct usb_ctrlrequest*)pEnd0Buffer)->wValue);

	DEBUG_CODE(1, { \
	    struct usb_ctrlrequest *pControlRequest=(struct usb_ctrlrequest*) \
	    	pEnd0Buffer->aEnd0Data; \
	    printk(KERN_INFO \
	    "%s: bRequest=%02x, bmRequestType=%02x, wValue=%04x, \
	    wIndex=%04x, wLength=%04x\n", \
	    __FUNCTION__, pControlRequest->bRequest, \
	    pControlRequest->bRequestType, pControlRequest->wValue, \
	    pControlRequest->wIndex, pControlRequest->wLength); } )

	if(((struct usb_ctrlrequest *)pEnd0Buffer)->bRequestType & USB_DIR_IN)	
	{ /* go back to host */
	    pEnd0Buffer->wEnd0Count=0;
	    pThis->bEnd0Stage = MGC_END0_IN;
	} else	{ /* shall be analyzed by the device */
	    pEnd0Buffer->wEnd0Count=((struct usb_ctrlrequest*)pEnd0Buffer)
			->wLength; /** wLength to go for the payload */
	    pThis->bEnd0Stage = MGC_END0_OUT;
	}

    } else {
        wLength=((struct usb_ctrlrequest*)pEnd0Buffer)->wLength;
	offset+=wLength-pEnd0Buffer->wEnd0Count;
    }

    /* now Im reading the rest of it */
    if ( wCount>0 ) {    
	MGC_HdrcUnloadFifo(pBase, 0, wCount, 
	    &pEnd0Buffer->aEnd0Data[offset]);
	pEnd0Buffer->wEnd0Count-=wCount;
    }
    
    DEBUG_CODE(1, printk(KERN_INFO "%s: pEnd0Buffer->wEnd0Count=%d, %s\n", \
	__FUNCTION__, pEnd0Buffer->wEnd0Count, \
	(pEnd0Buffer->wEnd0Count)?"still to go":"header completed"); )
    
    return (pEnd0Buffer->wEnd0Count);
}

/**
 * Handle default endpoint interrupt as device.
 * @param pThis this
 */
STATIC void MGC_HdrcServiceDeviceDefaultEnd(MGC_LinuxCd* pThis)
{
    uint8_t bStall = FALSE;
    uint16_t wCsrVal, wCount;
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;


    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    MGC_SelectEnd(pBase, 0);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
    wCount = MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0);

    DEBUG_CODE(2, printk(KERN_INFO "%s: wCrsVal=0x%x, wCount=%d\n",
	    __FUNCTION__, wCsrVal, wCount); )

    if(!wCsrVal && !wCount && (MGC_END0_STATUS == pThis->bEnd0Stage))
    {
	pThis->bEnd0Stage = MGC_END0_START;

	/* update address if needed */
	if(pThis->bSetAddress)
	{
	    pThis->bSetAddress = FALSE;
	    MGC_Write8(pBase, MGC_O_HDRC_FADDR, pThis->bAddress);
	}
	
	TRACE( -1 )
	
	/* enter test mode if needed */
	if(pThis->bTestMode)
	{
	    if (MGC_M_TEST_PACKET == pThis->bTestModeValue)
	    {
		MGC_HdrcLoadFifo(pBase, 0, sizeof(MGC_aTestPacket), 
		    MGC_aTestPacket);
	    }

	    MGC_Write8(pBase, MGC_O_HDRC_TESTMODE, 
	    	pThis->bTestModeValue);
	}

	return;
    }


    if(wCsrVal & MGC_M_CSR0_P_SENTSTALL)
    {
	pThis->wEnd0Offset = 0;
	pThis->bEnd0Stage = MGC_END0_START;
	wCsrVal &= ~MGC_M_CSR0_P_SENTSTALL;
    }

    if(wCsrVal & MGC_M_CSR0_P_SETUPEND)
    {
	pThis->wEnd0Offset = 0;
	pThis->bEnd0Stage = MGC_END0_START;
	wCsrVal |= MGC_M_CSR0_P_SVDSETUPEND;
    }

	/* packet received from host,ready to unload it */
    if(wCsrVal & MGC_M_CSR0_RXPKTRDY)
    {
        /* packet received */
	wCsrVal |= MGC_M_CSR0_P_SVDRXPKTRDY;
	TRACE( -1 )	
    	if ( wCount ) {	
    	    const int count=( pThis->pfFillBuffer )
		    ? (*pThis->pfFillBuffer)(pThis)
		    : MGC_HdrcReadUSBControlRequest(pThis, wCount);
#if 0	
    	    if ( count<0 ) {
               bStall=TRUE;
  	    } else if ( 0==count ) {
		bStall=MGC_HandleRequest(pThis);
		pThis->bEnd0Stage = MGC_END0_STATUS; 
	    }
#endif
	    if(bStall) {
		wCsrVal |= MGC_M_CSR0_P_SENDSTALL;
	    }	
	}	
    } else {
    	uint16_t wFifoCount;	
        uint8_t* pFifoDest = NULL;
	const MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[0]);

	TRACE( -1 )
	
#if 0	
	/* we must have transmitted something */
	if(pEnd->pRequest && (MGC_END0_IN == pThis->bEnd0Stage))
	{
	    pFifoDest = (uint8_t*)pEnd->pRequest->buf + 
	      pThis->wEnd0Offset;
	    wFifoCount = min(MGC_END0_FIFOSIZE,
			     pEnd->pRequest->length - pThis->wEnd0Offset);
	    if(wFifoCount || pEnd->pRequest->zero)
	    {
		MGC_HdrcLoadFifo(pBase, 0, wFifoCount, pFifoDest);
		wCsrVal |= MGC_M_CSR0_TXPKTRDY;
	    }
	
	    if(!wFifoCount)
	    {
	        // pThis->bEnd0Stage = MGC_END0_START;
	        pThis->bEnd0Stage = MGC_END0_STATUS; // WAS START!
		wCsrVal |= MGC_M_CSR0_P_DATAEND;
		pEnd->pRequest->actual = pThis->wEnd0Offset;
		if(pEnd->pRequest->complete)
		{
		    pEnd->pRequest->status = 0;
		    pEnd->pRequest->complete(&(MGC_aGadgetEnd[0]), 
					     pEnd->pRequest);
		}
	    }
	}
	else
	{
	    pThis->bEnd0Stage = MGC_END0_START;
	    wCsrVal |= MGC_M_CSR0_P_DATAEND;
	}
#endif	

	    pThis->bEnd0Stage = MGC_END0_START;
	    wCsrVal |= MGC_M_CSR0_P_DATAEND;

    }

	TRACE( -1 )

    /* program core (ACK the interrupts ) */
    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);

	TRACE( -1 )
}

STATIC void MGC_HdrcServiceDeviceTxAvail(MGC_LinuxCd* pThis, uint8_t bEnd)
{
    uint16_t wVal;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_LinuxLocalEnd* pLocalEnd = &(pThis->aLocalEnd[bEnd]);
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    MGC_SelectEnd(pBase, bEnd);
    wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

    do
    {
	if(wVal & MGC_M_TXCSR_P_SENTSTALL)
	{
	    wVal &= ~MGC_M_TXCSR_P_SENTSTALL;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
	    break;
	}
	if(wVal & MGC_M_TXCSR_P_UNDERRUN)
	{
#ifdef CONFIG_PROC_FS
	    pThis->aLocalEnd[bEnd].dwMissedTxPackets++;
#endif
	    wVal &= ~MGC_M_TXCSR_P_UNDERRUN;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
	    break;
	}

#ifdef MUSB_DMA
	if(pEnd->pDmaChannel)
	{
	    if(MUSB_DMA_STATUS_FREE == 
		pPort->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel))
	    {
		pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
	    }
	}
	else
#endif
	    pEnd->dwOffset += pLocalEnd->dwRequestSize;

	if(pEnd->dwOffset < pRequest->length)
	{
	    /* for now, assume any DMA controller can move maximum-length request */
	    wFifoCount = min(pLocalEnd->wMaxPacketSizeTx, (pRequest->length - pEnd->dwOffset));
	    MGC_HdrcLoadFifo(pBase, bEnd, wFifoCount, (uint8_t*)pRequest->buf + pEnd->dwOffset);
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, MGC_M_TXCSR_TXPKTRDY);
	}
	else
	{
	    if(pEnd->pRequest->complete)
	    {
		pEnd->pRequest->complete(&(MGC_aGadgetEnd[bEnd]), pEnd->pRequest);
		/* next */
		if(list_empty(&pEnd->pRequest->list))
		{
		    pEnd->pRequest = NULL;
		}
		else
		{
		    pRequest = list_entry(pEnd->pRequest->list.next,
			struct usb_request, list);
		    list_del(&pRequest->list);
		    pEnd->pRequest = pRequest;
		}
	    }
	}
    } while(FALSE);
}

STATIC void MGC_HdrcServiceDeviceRxReady(MGC_LinuxCd* pThis, uint8_t bEnd)
{
    uint16_t wCsrVal, wCount;
    uint16_t wFifoCount = 0;
    uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_GadgetLocalEnd* pEnd = &(MGC_aGadgetLocalEnd[bEnd]);
    struct usb_request* pRequest = pEnd->pRequest;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    MGC_SelectEnd(pBase, bEnd);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
    wCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);

    do
    {
	if(wCsrVal & MGC_M_RXCSR_P_SENTSTALL)
	{
	    wCsrVal &= ~MGC_M_RXCSR_P_SENTSTALL;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
	    break;
	}
	if(wCsrVal & MGC_M_RXCSR_P_OVERRUN)
	{
#ifdef CONFIG_PROC_FS
	    pThis->aLocalEnd[bEnd].dwMissedRxPackets++;
#endif
	    wCsrVal &= ~MGC_M_RXCSR_P_OVERRUN;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
	    break;
	}
#ifdef CONFIG_PROC_FS
	if(wCsrVal & MGC_M_RXCSR_INCOMPRX)
	{
	    pThis->aLocalEnd[bEnd].dwErrorRxPackets++;
	}
#endif
#ifdef MUSB_DMA
	if(pEnd->pDmaChannel)
	{
	    if(MUSB_DMA_STATUS_FREE == 
		pPort->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel))
	    {
		pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
	    }
	}
	else
#endif
	    pEnd->dwOffset += pEnd->dwRequestSize;

	if(pEnd->dwOffset < pRequest->length)
	{
	    /* for now, assume any DMA controller can move maximum-length request */
	    wFifoCount = min(wCount, pRequest->length - pEnd->dwOffset);
	    MGC_HdrcUnloadFifo(pBase, bEnd, wFifoCount, (uint8_t*)pRequest->buf + pEnd->dwOffset);
	    wCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsrVal);
	}
	else
	{
	    if(pEnd->pRequest->complete)
	    {
		pEnd->pRequest->complete(&(MGC_aGadgetEnd[bEnd]), pEnd->pRequest);
		/* next */
		if(list_empty(&pEnd->pRequest->list))
		{
		    pEnd->pRequest = NULL;
		}
		else
		{
		    pRequest = list_entry(pEnd->pRequest->list.next,
			struct usb_request, list);
		    list_del(&pRequest->list);
		    pEnd->pRequest = pRequest;
		}
	    }
	}
    } while(FALSE);
}

/**
 * The endpoint selection policy should have already selected
 * an appropriate usb_ep from us, since we follow the naming convention.
 */
STATIC int MGC_GadgetEnableEnd(struct usb_ep *ep,
			       const struct usb_endpoint_descriptor *desc)
{
    unsigned long flags;	
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

	TRACE(-1) 
	
    if(!pThis || (nEnd < 0) || 
       (nEnd != (desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
    {
        return -EINVAL;
    }

	TRACE(-1) 

    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
    spin_lock_irqsave(&pEnd->Lock, flags);
    pEnd->wPacketSize = desc->wMaxPacketSize;
    pEnd->bTrafficType = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    pEnd->bIsTx = (desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? 
      TRUE : FALSE;
    pEnd->pRequest = NULL;
    spin_unlock_irqrestore(&pEnd->Lock, flags);
	
	TRACE(-1) 

    return 0;
}

STATIC int MGC_GadgetDisableEnd(struct usb_ep *ep)
{
    unsigned long flags;
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || (nEnd < 0))
    {
        return -EINVAL;
    }
    
    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
    spin_lock_irqsave(&pEnd->Lock, flags);
    pEnd->pRequest = NULL;
    spin_unlock_irqrestore(&pEnd->Lock, flags);

    return 0;
}

STATIC struct usb_request* MGC_GadgetAllocRequest(struct usb_ep *ep,
						  int gfp_flags)
{
    struct usb_request* pResult=NULL;
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
    KMALLOC(pResult, sizeof(struct usb_request), gfp_flags);
    return pResult;
}

STATIC void MGC_GadgetFreeRequest(struct usb_ep *ep, struct usb_request *req)
{
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
    KFREE(req);
}

STATIC void* MGC_GadgetAllocBuffer(struct usb_ep *ep, unsigned bytes,
				   dma_addr_t *dma, int gfp_flags)
{
    void* pResult=NULL;
    DEBUG_CODE(2, printk(KERN_INFO "=> %s for %d\n", __FUNCTION__, \
    	bytes); )
    KMALLOC(pResult, bytes, gfp_flags);
    DEBUG_CODE(2, printk(KERN_INFO "%s allocated %d bytes at %p\n", __FUNCTION__, \
    	bytes, pResult); )
    return pResult;
}

STATIC void MGC_GadgetFreeBuffer(struct usb_ep *ep, void *buf, dma_addr_t dma,
				 unsigned bytes)
{
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    /* no DMA for now */
    KFREE(buf);
}

STATIC int MGC_GadgetQueue(struct usb_ep *ep, struct usb_request *req,
			   int gfp_flags)
{
    uint16_t wLength;
    unsigned long flags;
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    DEBUG_CODE(2, printk(KERN_INFO "=> %s: pThis=%p, nEnd=%d\n", \
    	__FUNCTION__, pThis, nEnd); )

    if(!pThis || (nEnd < 0))
    {
        return -EINVAL;
    }

    DEBUG_CODE(2, printk(KERN_INFO "=> %s: pThis->bEnd0Stage=%d\n", \
    	__FUNCTION__, pThis->bEnd0Stage); )

    /* if end0 Rx, we already grabbed data; just copy and complete */
    if(!nEnd && (MGC_END0_OUT == pThis->bEnd0Stage))
    {
    	TRACE( -1 ) 

        wLength = min(req->length, pThis->wEnd0Offset - 8);
	memcpy(req->buf, (void*)(uint8_t*)pThis->aEnd0Data + 8, wLength);
	req->status = 0;
	if(req->complete)
	{
	    req->complete(&MGC_GadgetEnd0, req);
	}

	return 0;
    }

    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s: pEnd=%p pEnd->pRequest=%p\n", \
    	__FUNCTION__, pEnd, pEnd->pRequest); )

    spin_lock_irqsave(&pEnd->Lock, flags);
    if(pEnd->pRequest)
    {
	list_add_tail(&req->list, &pEnd->pRequest->list);
    } else {
	pEnd->pRequest = req;
	if(pThis->bIsDevice){
	    TRACE( -1 )
	    MGC_HdrcProgramDeviceEnd(pThis, (uint8_t)nEnd);
	}
    }
    
    spin_unlock_irqrestore(&pEnd->Lock, flags);
    
    return 0;
}

STATIC int MGC_GadgetDequeue(struct usb_ep *ep, struct usb_request *pRequest)
{
    unsigned long flags;
    MGC_GadgetLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || (nEnd < 0))
    {
        return -EINVAL;
    }
    
    pEnd = &(MGC_aGadgetLocalEnd[nEnd]);
    spin_lock_irqsave(&pEnd->Lock, flags);

    if(pRequest == pEnd->pRequest)
    {
	/* TODO: stop end */
    }
    list_del_init(&pRequest->list);
    spin_unlock_irqrestore(&pEnd->Lock, flags);

    return 0;
}

STATIC int MGC_GadgetSetHalt(struct usb_ep *ep, int value)
{
    uint16_t wCsr;
    uint8_t* pBase;
    unsigned long flags;
    MGC_LinuxLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || (nEnd < 0) || !pThis->bIsDevice)
    {
        return -EINVAL;
    }
    pBase = pThis->pRegs;

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, (uint8_t)nEnd);
    pEnd = &(pThis->aLocalEnd[nEnd]);
    spin_lock_irqsave(&(pEnd->Lock), flags);
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
    spin_unlock_irqrestore(&(pEnd->Lock), flags);
    spin_unlock_irqrestore(&(pThis->Lock), flags);

    return 0;
}

STATIC int MGC_GadgetFifoStatus(struct usb_ep *ep)
{
    uint8_t* pBase;
    unsigned long flags;
    MGC_LinuxLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);
    int nResult = 0;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || (nEnd < 0) || !pThis->bIsDevice)
    {
        return -EINVAL;
    }
    pBase = pThis->pRegs;

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, (uint8_t)nEnd);
    pEnd = &(pThis->aLocalEnd[nEnd]);
    spin_lock_irqsave(&(pEnd->Lock), flags);
    if(nEnd)
    {
        nResult = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, (uint8_t)nEnd);
    }
    else
    {
        nResult = MGC_ReadCsr16(pBase, MGC_O_HDRC_COUNT0, 0);
    }
    spin_unlock_irqrestore(&(pEnd->Lock), flags);
    spin_unlock_irqrestore(&(pThis->Lock), flags);

    return nResult;
}

STATIC void MGC_GadgetFifoFlush(struct usb_ep *ep)
{
    uint16_t wCsr, wIntrTxE;
    uint8_t* pBase;
    unsigned long flags;
    MGC_LinuxLocalEnd* pEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    int nEnd = MGC_GadgetFindEnd(ep);

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || (nEnd < 0) || !pThis->bIsDevice)
    {
        return;
    }
    pBase = pThis->pRegs;

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, (uint8_t)nEnd);
    pEnd = &(pThis->aLocalEnd[nEnd]);
    spin_lock_irqsave(&(pEnd->Lock), flags);

    /* disable interrupt in case we flush */
    wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
    MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << nEnd));

    if(nEnd)
    {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd);
        wCsr |= MGC_M_TXCSR_FRCDATATOG;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd, wCsr);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd, wCsr);
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd);
	wCsr |= MGC_M_RXCSR_FLUSHFIFO;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd, wCsr);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd, wCsr);
    }
    else
    {
	MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
    }

    /* re-enable interrupt */
    MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE);

    spin_unlock_irqrestore(&(pEnd->Lock), flags);
    spin_unlock_irqrestore(&(pThis->Lock), flags);
}

/* general operations */
STATIC int MGC_GadgetGetFrame(struct usb_gadget *gadget)
{
    uint8_t* pBase;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis)
    {
        return -EINVAL;
    }
    pBase = pThis->pRegs;
    return (int)MGC_Read16(pBase, MGC_O_HDRC_FRAME);
}

/** */
STATIC int MGC_GadgetWakeup(struct usb_gadget *gadget)
{
    uint8_t power;
    uint8_t* pBase;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis || !pThis->bIsDevice)
    {
        return -EINVAL;
    }
    pBase = pThis->pRegs;
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
    power |= MGC_M_POWER_RESUME;
    MGC_Write8(pBase, MGC_O_HDRC_POWER, power);

    return 0;
}

/** */
STATIC int MGC_GadgetSetSelfPowered(struct usb_gadget *gadget, 
				    int is_selfpowered)
{
    MGC_LinuxCd* pThis = MGC_pFirstDriver;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis)
    {
        return -EINVAL;
    }

    pThis->bIsSelfPowered = (uint8_t)is_selfpowered;
    return 0;
}

/** */
STATIC int MGC_GadgetVbusSession(struct usb_gadget *gadget, int is_active)
{
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    return -EINVAL;
}

/** */
STATIC int MGC_GadgetVbusDraw(struct usb_gadget *gadget, unsigned mA)
{
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    pThis->bIsSelfPowered=(mA)?TRUE:FALSE;
    return 0;
}

STATIC int MGC_GadgetPullup(struct usb_gadget *gadget, int is_on)
{
    uint8_t power;
    uint8_t* pBase;
    const MGC_LinuxCd* pThis = MGC_pFirstDriver;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis)
    {
        return -EINVAL;
    }

    pBase = pThis->pRegs;
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);

    if(is_on)
    {
	DEBUG_CODE(1, printk(KERN_INFO "%s: gadget %s connecting to host\n", \
		__FUNCTION__, pThis->pGadgetDriver->function); )

	power |= MGC_M_POWER_SOFTCONN;
    }
    else
    {
	DEBUG_CODE(1, \
		printk(KERN_INFO "%s: gadget %s disconnecting from host\n", \
			__FUNCTION__, pThis->pGadgetDriver->function); )

	power &= ~MGC_M_POWER_SOFTCONN;
    }

    MGC_Write8(pBase, MGC_O_HDRC_POWER, power);

    return 0;
}

STATIC int MGC_GadgetIoctl(struct usb_gadget *gadget,
			   unsigned code, unsigned long param)
{
    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
    /* TODO: what? */
    return -EINVAL;
}

/* registration operations */
int usb_gadget_register_driver (struct usb_gadget_driver *driver)
{
    uint8_t* pBase;
    uint8_t bEnd, power;
    MGC_LinuxLocalEnd* pEnd;
    struct usb_ep* pGadgetEnd;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;
    unsigned long flags;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
    	
    if(!pThis) {
    	DEBUG_CODE(1, printk(KERN_INFO "%s: musb-hdrc is missing\n", \
		__FILE__); )
        return -EINVAL;
    }
    	
    DEBUG_CODE(1, printk(KERN_INFO "%s: registering gadget %s\n", \
	__FUNCTION__, driver->function); )

    /* fill services for driver */
    spin_lock_irqsave(pThis->Lock, flags);
    
    /** redirect the opretations to myself */
    pThis->Gadget.ops = &MGC_GadgetOperations;
    INIT_LIST_HEAD(&(MGC_GadgetEnd0.ep_list));
    pThis->Gadget.ep0 = &MGC_GadgetEnd0;
    INIT_LIST_HEAD(&(pThis->Gadget.ep_list));

    for(bEnd = 1; bEnd < min(pThis->bEndCount, MUSB_C_NUM_EPS); bEnd++)
    {
	pEnd = &(pThis->aLocalEnd[bEnd]);
	pGadgetEnd = &(MGC_aGadgetEnd[bEnd]);
	INIT_LIST_HEAD(&(pGadgetEnd->ep_list));
	sprintf(MGC_aGadgetEndName[bEnd], "ep%d", bEnd);
	pGadgetEnd->name = MGC_aGadgetEndName[bEnd];
	pGadgetEnd->ops = &MGC_GadgetEndpointOperations;
	pGadgetEnd->maxpacket = max(pEnd->wMaxPacketSizeTx,
		pEnd->wMaxPacketSizeRx);
	list_add_tail(&pGadgetEnd->ep_list, &(pThis->Gadget.ep_list));
    }

#ifdef MUSB_OTG    
    pThis->Gadget.is_dualspeed = 1;
    pThis->Gadget.is_otg = 1;
    pThis->Gadget.a_hnp_support = 1;
#endif
	
    pThis->Gadget.name = pThis->aName; /* musbhdrc0 */
    pThis->pGadgetDriver = driver;
	
    pThis->Gadget.speed = USB_SPEED_FULL;
    pBase = pThis->pRegs;
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
    if(power & MGC_M_POWER_HSMODE)
    {
	pThis->Gadget.speed = USB_SPEED_HIGH;
    }

    /* bind */
    MUSB_DEV_MODE(pThis);
    driver->bind( &(pThis->Gadget) );
    spin_unlock_irqrestore(pThis->Lock, flags);

    DEBUG_CODE(1, printk(KERN_INFO "%s: gadget registered\n", \
	__FILE__); )	

    return 0;
}

int usb_gadget_unregister_driver (struct usb_gadget_driver *driver)
{
    unsigned long flags;
    MGC_LinuxCd* pThis = MGC_pFirstDriver;

    DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

    if(!pThis)
    {
        return -EINVAL;
    }

    DEBUG_CODE(1, printk(KERN_INFO "%s: unregistering gadget %s\n", \
    	__FUNCTION__, driver->function); )

    spin_lock_irqsave(pThis->Lock, flags);
    pThis->pGadgetDriver = NULL;
#ifdef MUSB_OTG
    MUSB_OTG_MODE(pThis);
#else
    MUSB_ERR_MODE(pThis);
#endif
    spin_unlock_irqrestore(pThis->Lock, flags);

    return 0;
}

// EXPORT_SYMBOL(usb_gadget_register_driver);
// EXPORT_SYMBOL(usb_gadget_unregister_driver);
