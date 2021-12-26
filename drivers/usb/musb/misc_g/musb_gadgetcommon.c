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
  * Gadget code, common amongst new development and demo devices
  *
  * $Revision: 1.1.1.1 $ 
  */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/spinlock.h>

#ifdef MUSB_V26
#include <linux/device.h>
#else
struct usb_tt {
        struct usb_device       *hub;   /* upstream highspeed hub */
        int                     multi;  /* true means one TT per port */
};
#endif

#include <linux/init.h>
#include <linux/usb_ch9.h>

#include "musbdefs.h"
#include "musb_gadgetdefs.h"

/**********************************************************************
 */

/* end point names, can be confogured using modules (not implemented yet!) */
MGC_GadgetLocalEnd MGC_aGadgetLocalEnd[MUSB_C_NUM_EPS]; /* counters and queues */
static char aGadgetEndNames[MUSB_C_NUM_EPS][6]; /* current endpoint names */

void*
	MGC_MallocEp0Buffer(const MGC_LinuxCd* pThis);
/* ----------------------------------------------------------------------
 *
 *
 */


#ifdef MUSB_CONFIG_PROC_FS
char*
decode_dev_ep_protocol(MGC_LinuxCd* pThis, unsigned bEnd) {
    char* pProto = "Err ";
	 
	switch(MGC_aGadgetLocalEnd[bEnd].bTrafficType) {
		case USB_ENDPOINT_XFER_CONTROL: pProto = "Ctrl"; break;
		case USB_ENDPOINT_XFER_ISOC: pProto = "Isoc"; break;
		case USB_ENDPOINT_XFER_BULK: pProto = "Bulk"; break;
		case USB_ENDPOINT_XFER_INT: pProto = "Intr"; break;
	}
	
	return pProto;
}
#endif

/**********************************************************************
 *
 *
 */

/** general operations */
int MGC_GadgetGetFrame(struct usb_gadget *gadget)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

#ifdef MUSB_PARANOID
    if( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return -EINVAL;
    }
#endif
    return (int)MGC_Read16(pThis->pRegs, MGC_O_HDRC_FRAME);
}

/** */
int MGC_GadgetWakeup(struct usb_gadget *gadget)
{
    uint8_t power;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

#ifdef MUSB_PARANOID
    if( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return -EINVAL;
    }
#endif
    
    power = MGC_Read8(pThis->pRegs, MGC_O_HDRC_POWER);
    power |= MGC_M_POWER_RESUME;
    MGC_Write8(pThis->pRegs, MGC_O_HDRC_POWER, power);

    return 0;
}

/** */
int MGC_GadgetSetSelfPowered(struct usb_gadget *gadget, 
    int is_selfpowered)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

#ifdef MUSB_PARANOID
    if( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return -EINVAL;
    }
#endif

    pThis->bIsSelfPowered = (uint8_t)is_selfpowered;
    return 0;
}

#ifdef GADGET_FULL
/** */
int MGC_GadgetVbusSession(struct usb_gadget *gadget, int is_active)
{
    return -EINVAL;
}

/** */
int MGC_GadgetVbusDraw(struct usb_gadget *gadget, unsigned mA)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
	
#ifdef MUSB_PARANOID
    if( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return -EINVAL;
    }
#endif
    pThis->bIsSelfPowered=(mA)?TRUE:FALSE;
    return 0;
}

/** */
int MGC_GadgetPullup(struct usb_gadget *gadget, int is_on)
{
    uint8_t power;
    uint8_t* pBase;
    const MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

#ifdef MUSB_PARANOID
    if( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return -EINVAL;
    }
#endif

    pBase = pThis->pRegs;
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);

    if(is_on) {
		DEBUG_CODE(3, printk(KERN_INFO "gadget %s connecting to host\n", \
			pThis->pGadgetDriver->function); )
		power |= MGC_M_POWER_SOFTCONN;
    } else {
		DEBUG_CODE(3, printk(KERN_INFO "gadget %s disconnecting from host\n", \
			pThis->pGadgetDriver->function); )
		power &= ~MGC_M_POWER_SOFTCONN;
    }

    MGC_Write8(pBase, MGC_O_HDRC_POWER, power);

    return 0;
}
#endif


/**********************************************************************
 *
 */

/** 
 * @pre pThis!=NULL
 */
void MGC_GadgetResume(MGC_LinuxCd* pThis) {
    if( pThis->pGadgetDriver && pThis->pGadgetDriver->resume) {
	pThis->pGadgetDriver->resume( &pThis->Gadget );
    }
}

/**
 * @pre pThis!=NULL
 */
void MGC_GadgetSuspend(MGC_LinuxCd* pThis) {
    if( pThis->pGadgetDriver && pThis->pGadgetDriver->suspend) {
	pThis->pGadgetDriver->suspend( &pThis->Gadget );
    }
}

/** 
 * @pre pThis!=NULL
 */
void MGC_GadgetDisconnect(MGC_LinuxCd* pThis) {
    if( pThis->pGadgetDriver && pThis->pGadgetDriver->disconnect) {
	pThis->pGadgetDriver->disconnect( &pThis->Gadget );
    }
}

/**
 *
 * @pre pThis!=NULL
 */
void MGC_GadgetReset(MGC_LinuxCd* pThis) {
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;
    uint8_t devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
    
#ifdef MUSB_PARANOID 
   ASSERT_SPINLOCK_UNLOCKED(&pThis->Lock); 
   ASSERT_SPINLOCK_UNLOCKED(&MGC_aGadgetLocalEnd[0]); 
#endif

    DEBUG_CODE(3, printk(KERN_INFO "<== mode=%s, addr=%x\n", (devctl&MGC_M_DEVCTL_HM)?"host":"function", 
    	MGC_Read8(pBase, MGC_O_HDRC_FADDR)); )

    /* HR does NOT clear itself */
    if(devctl & MGC_M_DEVCTL_HR) {
		MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, MGC_M_DEVCTL_SESSION);
    }
    
    /* unconfigured */
    pThis->bAddress=0;
    pThis->bDeviceState=MGC_STATE_DEFAULT;
    pThis->bEnd0Stage=MGC_END0_STAGE_SETUP;
    pThis->wEnd0Offset=0;

    if ( pThis->pGadgetDriver ) {
		uint8_t bEnd;
		
		uint8_t power = MGC_Read8(pBase, MGC_O_HDRC_POWER);		
		pThis->Gadget.speed = (power & MGC_M_POWER_HSMODE)
			? USB_SPEED_HIGH : USB_SPEED_FULL;	     		

		/* disable the endpoints */
		for(bEnd = 1; bEnd < pThis->bEndCount; bEnd++) {
			MGC_GadgetDisableEnd(&MGC_aGadgetLocalEnd[bEnd].end_point);
		}
					
    }

	
#ifdef MUSB_MONITOR_DATA
    MGC_DisableDebug();
#endif

}

/* --------------------------------------------------------------------
 * Buffer functions; using the buffer functions in plat_uds.c
 *
 * -------------------------------------------------------------------- */

/**
 * Allocate a the memory for buffer allocated for an given 
 * endpoint; free the memeory with MGC_GadgetAllocateBuffer().
 * @param ep the end point the buffer should be freed for
 * @param bytes the bytes to allocate
 * @param dma the dma address
 * @param gfp_flags glafs memory should be allocated with
 */
void* MGC_GadgetAllocBuffer(struct usb_ep *ep, unsigned bytes,
	dma_addr_t *dma, int gfp_flags)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

    /* always off now, configure the gadget local end to enable it */
    dma_addr_t *wants_dma=((MGC_GadgetLocalEnd*)ep)->bDMAEnabled
    	? dma : 0L; 

#ifdef MUSB_PARANOID    
    if ( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return 0;
    }

    if ( MGC_GadgetFindEnd(ep) < 0 ) {
    	printk(KERN_ERR "invalid endpoint ep=%p\n", ep);
    	return 0;
    } 

    if ( !EP_NUMBER(ep) && wants_dma ) {
    	printk(KERN_ERR "requested DMA for ep0\n");
		wants_dma=0L;
    }

    if ( wants_dma ) {
    	WARN("requested DMA for ep%d\n", EP_NUMBER(ep));
    }
#endif
    
    /* DMA is controlled in the endpoint initialization; no DMA for now... */
    return MGC_AllocBufferMemory(pThis, bytes, 
        gfp_flags, wants_dma); 
}


/**
 * Free a buffer allocated from MGC_GadgetAllocateBuffer()
 * @param epo the end point the buffer should be freed for
 * @param address the address to free
 * @param dma the dma address
 * @param bytes the bytes to free
 */
 void MGC_GadgetFreeBuffer(struct usb_ep *ep, void *address, dma_addr_t dma,
 	unsigned bytes)
{
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);

#ifdef MUSB_PARANOID    
    if ( !pThis ) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return;
    }

    if ( MGC_GadgetFindEnd(ep) < 0 ) {
    	printk(KERN_ERR "invalid endpoint ep=%p\n", ep);
    	return;
    } 
#endif

    MGC_FreeBufferMemory(pThis, bytes, address, dma);
}

/**********************************************************************
 * FIFO functions
 *
 */

/**
 * FLUSH the FIFO for an endpoint
 *
 * @pre ep
 */
void MGC_GadgetFifoFlush(struct usb_ep *ep)
{
    uint16_t wCsr, wIntrTxE;
    unsigned long flags;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    uint8_t* pBase = pThis->pRegs;
    int nEnd = EP_NUMBER(ep);

#ifdef MUSB_PARANOID
    if (!pThis) {
    	printk(KERN_ERR "Gadget not initialized, pThis=NULL\n");
        return;
    }

	if ( nEnd<0 ) {
    	printk(KERN_ERR "invalid endpoint ep=%p\n", ep);
        return;
    }

   ASSERT_SPINLOCK_UNLOCKED(&pThis->Lock); 
#endif

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, (uint8_t)nEnd);

	/* disable interrupts */
    wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
    MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << nEnd));

    if(nEnd) {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd);
        wCsr |= MGC_M_TXCSR_FRCDATATOG;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd, wCsr);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, (uint8_t)nEnd, wCsr);

        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd);
		wCsr |= MGC_M_RXCSR_FLUSHFIFO;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd, wCsr);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, (uint8_t)nEnd, wCsr);
    } else {
		MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
    }

    /* re-enable interrupt */
    MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE);
    spin_unlock_irqrestore(&(pThis->Lock), flags);

}

/**
 * Return the FIFO status.
 *
 * @param ep the end point
 */
int MGC_GadgetFifoStatus(struct usb_ep *ep)
{
    unsigned long flags;
    int nResult = 0;
    MGC_LinuxCd* pThis = MGC_GetDriverByName(NULL);
    uint8_t* pBase=pThis->pRegs;
    int bEnd = EP_NUMBER(ep);

#ifdef MUSB_PARANOID
    ASSERT_SPINLOCK_UNLOCKED(&pThis->Lock); 

    if( MGC_GadgetFindEnd(ep)<0 ) {
    	printk(KERN_ERR "invalid endpoint ep=%p\n", ep);
        return -EINVAL;
    }
#endif

    spin_lock_irqsave(&(pThis->Lock), flags);
    MGC_SelectEnd(pBase, bEnd);    
    nResult = MGC_ReadCsr16(pBase, 
		(bEnd)? MGC_O_HDRC_RXCOUNT:MGC_O_HDRC_COUNT0, 
		bEnd);
    spin_unlock_irqrestore(&(pThis->Lock), flags);

    DEBUG_CODE(2, printk(KERN_INFO "==> %d\n", nResult); )
    return nResult;
}


/**********************************************************************
 * End point init
 *
 */
 
/**
 * Initialize the local endpoint of a gadget (MGC_aGadgetLocalEnd[] array): 
 * counters, lists, locks etc. The endpoint function are fixed: 
 * MGC_GadgetEndpointOperations, I could pass them as a parameter as well. 
 *
 * @param pThis the controller instance.
 * @param gadget the gadget
 */
void
	MGC_GadgetInit(MGC_LinuxCd *pThis, struct usb_gadget *gadget) 
{
	uint8_t bEnd;
	
	DEBUG_CODE(2, printk(KERN_INFO "<== Initializing %d end points\n", pThis->bEndCount); )

	pThis->pEnd0Buffer = MGC_MallocEp0Buffer(pThis);
	/* intialize the eplist when appropriate */ 
	if ( gadget ) {
		INIT_LIST_HEAD( &(gadget->ep_list) );
	}

    for(bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
    	memset(&MGC_aGadgetLocalEnd[bEnd], 0, sizeof( MGC_aGadgetLocalEnd[bEnd] ));

#if MUSB_DEBUG>0	
		MGC_aGadgetLocalEnd[bEnd].wPadFront=MGC_LOCAL_PAD;
#endif

		MGC_aGadgetLocalEnd[bEnd].bEndNumber=bEnd;
		MGC_aGadgetLocalEnd[bEnd].pThis=pThis; /* my controller */
		
    	spin_lock_init( &MGC_aGadgetLocalEnd[bEnd].Lock );    
		INIT_LIST_HEAD( &MGC_aGadgetLocalEnd[bEnd].req_list );	
	
		sprintf(aGadgetEndNames[bEnd], "ep%d", bEnd);	
		MGC_aGadgetLocalEnd[bEnd].end_point.name=aGadgetEndNames[bEnd];	
		MGC_aGadgetLocalEnd[bEnd].end_point.ops = &MGC_GadgetEndpointOperations;
		INIT_LIST_HEAD( &MGC_aGadgetLocalEnd[bEnd].end_point.ep_list );
		
		if ( bEnd ) {
			MGC_aGadgetLocalEnd[bEnd].end_point.maxpacket = 
				max(pThis->aLocalEnd[bEnd].wMaxPacketSizeTx, 
					pThis->aLocalEnd[bEnd].wMaxPacketSizeRx);		
							
			/* Add the end points ep<n> n>=1 to ep_list when appropriate */ 
			if ( gadget ) {
				list_add_tail(&MGC_aGadgetLocalEnd[bEnd].end_point.ep_list, 
	    			&(gadget->ep_list));
			}
		} else {
			MGC_aGadgetLocalEnd[0].end_point.maxpacket=MGC_END0_FIFOSIZE;
		}		

    }

}

/**************************************************************************
 *
 *
 *************************************************************************/

/** 
  * allocate real memory for ep0 buffer, when MUSB_TEST_GADGET 
  * is set, I'll be using the static buffer up there.
  * @param pThis the Controller memory shoudl be allocateted for.
  */ 
void*
	MGC_MallocEp0Buffer(const MGC_LinuxCd* pThis) 
{     
	void *buffer;
	
    KMALLOC(buffer, sizeof(MGC_End0Buffer), GFP_KERNEL);
    if ( !buffer) {
		printk(KERN_ERR "out of memory\n");	
		return NULL;
    }
    
    memset(buffer, 0, sizeof(MGC_End0Buffer));
	return buffer;
}

#ifndef MUSB_GSTORAGE

/**
 * Read a FULL header packet from the hardware. The buffer starts with 
 * struct usb_ctrlrequest (fields are converted to device specific
 * byte order).
 *
 * @param wCount>0
 * @return 0 when the packet is complete, a negative number when an error 
 * occurred, a positive number when still there are bytes to read.
 */
int MGC_ReadUSBControlRequest(MGC_LinuxCd* pThis, uint16_t wCount) {
    const uint8_t* pBase = (uint8_t*)pThis->pRegs;
    MGC_End0Buffer* pEnd0Buffer=(MGC_End0Buffer*)pThis->pEnd0Buffer;
    struct usb_ctrlrequest* pControlRequest=(struct usb_ctrlrequest*)pEnd0Buffer;
	
    DEBUG_CODE(4, printk(KERN_INFO "wCount=%u, pEnd0Buffer->wEnd0Count=%u\n", wCount, 
    	pEnd0Buffer->wEnd0Count); )

   /* what did u call me for?? */
   if (!wCount) {
   	return -EINVAL;
   }

    /* buffer overrun, it should never happen */
    if ( wCount>(MUSB_MAX_END0_PACKET-sizeof(struct usb_ctrlrequest)) ) {
	printk(KERN_ERR "buffer overrun! wCount=%d\n", wCount ); 
	return -EINVAL; 
    }	

    /* need to have at least enough bytes for the control request
     * comment this out to enable fifo size < 8 bytes */
    if ( wCount<sizeof(struct usb_ctrlrequest) ) {
		printk(KERN_ERR "wCount=%d<sizeof(struct usb_ctrlrequest)=%d\n",
				wCount, (int)sizeof(struct usb_ctrlrequest)); 
		return -EINVAL; 
    }

#ifdef MUSB_PARANOID 
   ASSERT_SPINLOCK_UNLOCKED(&pThis->Lock); 
#endif

    spin_lock(&pThis->Lock);
    MGC_SelectEnd(pBase, 0); /* select ep0 */
	
    /* wEnd0Count=0 means that I'm reading the USB standard header:
     * that's the first thing I need to read from the FIFO (8 bytes). */
    if ( 0==pEnd0Buffer->wEnd0Count ) {
		MGC_HdrcUnloadFifo(pBase, 0, sizeof(struct usb_ctrlrequest), 
			(uint8_t*)pControlRequest);
		wCount-=sizeof(struct usb_ctrlrequest);
		
		/* data from the USB bus must be converted from LSB to 
		 * host-specific byte ordering; the control request header 
		 * tell me the payload length etc. */
		le16_to_cpus( pControlRequest->wLength );
		le16_to_cpus( pControlRequest->wIndex );
		le16_to_cpus( pControlRequest->wValue );
	
		DEBUG_CODE(4, printk(KERN_INFO "bRequest=%02x, wValue=%04x, wIndex=%04x, wLength=%04x\n", 
			pControlRequest->bRequest,  
			pControlRequest->wValue, pControlRequest->wIndex, 
			pControlRequest->wLength); )
	
		/* the header will set me in the right stage:
			pThis->bEnd0Stage=MGC_END0_STAGE_TX
			pThis->bEnd0Stage=MGC_END0_STAGE_RX
		*/
		if( pControlRequest->bRequestType & USB_DIR_IN ) { 
			/* write to host: up to wLength bytes */
			pEnd0Buffer->wEnd0Count=0;
			pThis->bEnd0Stage = MGC_END0_STAGE_TX;
		} else if( pControlRequest->bRequestType & USB_DIR_OUT ) { 
			/* out to function: wLength to go for the payload */
			pEnd0Buffer->wEnd0Count=pControlRequest->wLength;
			pThis->bEnd0Stage = MGC_END0_STAGE_RX;
		} 
    }

    if ( wCount>0 ) {    
		/* now Im reading the rest of it, this will never be executed I guess */
		uint16_t offset=sizeof(struct usb_ctrlrequest)+ /* read past the header */
			(pControlRequest->wLength)-(pEnd0Buffer->wEnd0Count);	
		MGC_HdrcUnloadFifo(pBase, 0, wCount, &pEnd0Buffer->aEnd0Data[offset]);
		pEnd0Buffer->wEnd0Count-=wCount;
    }

    DEBUG_CODE(5, printk(KERN_INFO "pEnd0Buffer->wEnd0Count=%d, ep0stage=%d, %s\n", 
	pEnd0Buffer->wEnd0Count, pThis->bEnd0Stage, 
	(pEnd0Buffer->wEnd0Count)?"still to go":"header completed"); )

    spin_unlock(&pThis->Lock);
    
    /* 0 header completed, <0 error, >0 bytes to go*/
    return (pEnd0Buffer->wEnd0Count);
}
#endif

/**********************************************************************
 * DEBUG Functions
 *
 */

 /**
 * Sump the status of endpoints
 * 
 * @param pTHis
 * @see queue_length
 * @see dump_usb_request
 */
void dump_ep_status(MGC_LinuxCd *pThis) {
    uint8_t bEnd=0;
    MGC_GadgetLocalEnd *pEnd;
    struct usb_request *req;
	
    for(bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
		pEnd=&MGC_aGadgetLocalEnd[bEnd];

		req=MGC_CurrentRequest(pEnd);
		printk(KERN_INFO"End-%d (%s): qlenght=%d, %s\n", bEnd, 
			(pEnd->bIsTx)?"tx":"rx",
			queue_length(&pEnd->req_list), 
			(req) ? dump_usb_request(req):"none" );
    }			
	    
}

char* dump_usb_request(struct usb_request *req) {    
    static char buff[256];
	if ( req ) {
		sprintf(buff, "req=%p, req->request.length=0x%0x, req->request.zero=0x%x, "
		"req->request.actual=0x%x, req->request.status=%d", 
		req, req->length, req->zero, req->actual, req->status );
	} else {
		sprintf(buff, "null request");		
	}
	
    return buff;
}

/*
 */
int queue_length(struct list_head *lh) {
	int count=0;	
	struct list_head *p=lh;
	
	while ( p && (p->next!=lh) ) {
	    count++;
	    p=p->next;
	}

	return count;
}


#if MUSB_DEBUG>0
#ifndef MUSB_GSTORAGE
/*
 * Map an endpoint to it's number; GStorage is using it's own function
 * that is defined regardeless of the DEBUG flag.
 *
 * @param usb_ep* pGadgetEnd the endpoit to look for (!=NULL).
 * @return -1 if the endpoint is not mine.
 */
int MGC_GadgetFindEnd(struct usb_ep* pGadgetEnd) 
{
    uint8_t bEnd=0;
    int nResult = -1;
    MGC_LinuxCd* pThis=MGC_GetDriverByName(NULL);
    const uint8_t ends=min(pThis->bEndCount, 
    	(uint8_t)MUSB_C_NUM_EPS);
	
    for (bEnd = 0; nResult==-1 && (bEnd < ends); bEnd++) {
		if ( pGadgetEnd == (struct usb_ep*)&(MGC_aGadgetLocalEnd[bEnd])) {
			nResult = bEnd;
		}
    }                          
  
    return nResult;
}
#endif

/**
 * Test whether and endpoint structure is corrupted.
 * 
 * @param ep the endpoint
 */
uint8_t is_ep_corrupted(void *ep) {

    if ( !ep ) {
       printk(KERN_ERR "null pointer in is_ep_corrupted()");
       return -EINVAL;
    }
    
    if ( MGC_GadgetFindEnd(ep)<0 ) {
       printk(KERN_ERR "ep=%p is not an endpoint()", ep);
       return -EINVAL;
    }
    
    if ( ((MGC_GadgetLocalEnd*)ep)->wPadFront!=MGC_LOCAL_PAD ) {
       printk(KERN_ERR "ep=%p pad corrupted!()", ep);
       return -EINVAL;    
    }
    
    return 0; /* not corrupted */
}

#endif
