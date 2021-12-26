/******************************************************************
 * Copyright 2005 Mentor Graphics Corporation
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

#ifndef __MUSB_GADGETDEFS_H
#define __MUSB_GADGETDEFS_H

#include <linux/list.h>
#include <linux/errno.h>

#ifdef MUSB_V26
#include <linux/device.h>
#endif

#include <linux/usb_gadget.h>

/* ----------------------- Gadget Cross Compile ----------------------- */

#ifdef __bluecat__
#define GADGET24_29
#elif defined(__mvl21__) 
#define GADGET24_29
#define GADGET_FULL
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20) && LINUX_VERSION_CODE<KERNEL_VERSION(2,6,0) )
#define GADGET24_29
#define GADGET_FULL
#elif ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9) )
#define GADGET26_pre9
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9) )
#define GADGET26_9
#define GADGET_FULL
#endif  

/* ---------------------------- debug stmts ------------------------- */

#define MGC_LOCAL_PAD  				0xa5deadfe

/* ---------------------------- end point status ------------------------- */

#define MUSB_GADGET_EP_ACTIVE		0
#define MUSB_GADGET_EP_HALTED		1
#define MUSB_GADGET_EP_DISABLED		2

/* ---------------------------- data structures ------------------------- */

/* dealing with Linix differences */
struct usb_ep;
struct usb_request;
struct usb_ctrlrequest;
 
/* compatibility, need to be osoleted 
 * used from gstorage */	
typedef struct {
    struct usb_request* pRequest;
} MGC_GadgetCompletionData;

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
typedef struct {
	/* this memory is currently wasted with gstorage */
    struct usb_ep end_point;

#if MUSB_DEBUG>0
    uint32_t	wPadFront;
#endif
    
    spinlock_t Lock;
	
	/* my reqyests */
    struct list_head req_list;
    unsigned int dwRequestSize;
    uint16_t wPacketSize;
    uint8_t bTrafficType;
    uint8_t bIsTx;   
	
    uint8_t bEndNumber;
    MGC_LinuxCd	*pThis;
	
    uint8_t bDMAEnabled; /* disabled by default, ep0 cannot be DMA!!! */

#ifdef MUSB_DMA
    MGC_DmaChannel* pDmaChannel;
#endif

#ifdef MUSB_CONFIG_PROC_FS
	/* not used yet! */
    unsigned long dwTotalTxBytes;
    unsigned long dwTotalRxBytes;
    unsigned long dwTotalTxPackets;
    unsigned long dwTotalRxPackets;
    unsigned long dwErrorTxPackets;
    unsigned long dwErrorRxPackets;
    unsigned long dwMissedTxPackets;
    unsigned long dwMissedRxPackets;
#endif

    /* compatibility, need to be osoleted, used from gstorage */	
    struct usb_request *pRequest;
    unsigned int dwOffset;
    MGC_GadgetCompletionData CompletionData;
    uint8_t bInactive; /* set when the ep is halted/request are queued but not executed */	
#if MUSB_DEBUG>0
    struct usb_endpoint_descriptor epDescriptor;
#endif

} MGC_GadgetLocalEnd;

/* ------------------------------- globals  ---------------------------- */

/* defined from the gadget code; each gadget is free to implement this
 * in a different way 
 */
//extern struct usb_gadget_ops MGC_GadgetOperations;
extern struct usb_ep_ops MGC_GadgetEndpointOperations;

/* in gadgetcommon.c */
extern char MGC_aGadgetEndName[6][MUSB_C_NUM_EPS];
extern MGC_GadgetLocalEnd MGC_aGadgetLocalEnd[MUSB_C_NUM_EPS]; /* counters and queues */

/* ------------------------ common function prototypes --------------------- */

extern void* MGC_GadgetAllocBuffer(struct usb_ep *ep, unsigned bytes,
	dma_addr_t *dma, int gfp_flags);
extern void MGC_GadgetFreeBuffer(struct usb_ep *ep, void *buf, dma_addr_t dma,
	unsigned bytes);
extern int MGC_GadgetFifoStatus(struct usb_ep *ep);
extern void MGC_GadgetFifoFlush(struct usb_ep *ep);

extern int MGC_GadgetGetFrame(struct usb_gadget *gadget);
extern int MGC_GadgetWakeup(struct usb_gadget *gadget);
extern int MGC_GadgetSetSelfPowered(struct usb_gadget *gadget,
	int is_selfpowered);

	
/* endpoints */	
#define MGC_GetEpDriver(_ep)	(((MGC_GadgetLocalEnd*)_ep)->pThis)

extern int MGC_GadgetDisableEnd(struct usb_ep *ep);
extern int MGC_GadgetEnableEnd(struct usb_ep *ep, 
	const struct usb_endpoint_descriptor *desc);

/* ------------------------ function prototypes -------------------------- */

extern void MGC_RestartRequest(MGC_LinuxCd *pThis, struct usb_request *pRequest);

extern uint8_t is_tx_request(const struct usb_ctrlrequest *pControlRequest);
extern uint8_t is_rx_request(const struct usb_ctrlrequest *pControlRequest);
extern uint8_t is_zerodata_request(const struct usb_ctrlrequest *pControlRequest);

extern void MGC_FlushEpRequests(MGC_GadgetLocalEnd* ep, const int status);

extern void MGC_HdrcServiceDeviceTxAvail(MGC_LinuxCd* pThis, uint8_t bEnd);
extern void MGC_HdrcServiceDeviceRxReady(MGC_LinuxCd* pThis, uint8_t bEnd);

/* -------------------------- from plat uds.c ------------------------------ */

extern void MGC_HdrcLoadFifo(const uint8_t* pBase, uint8_t bEnd, 
	uint16_t wCount, const uint8_t* pSource);
extern void MGC_HdrcUnloadFifo(const uint8_t* pBase, uint8_t bEnd, 
	uint16_t wCount, uint8_t* pDest);

/* export from musb_gadget.c */
extern int MGC_LinuxGadgetQueue(const uint8_t nEnd, struct usb_request *pRequest,
	int gfp_flags);

extern void MGC_HdrcHandleGadgetEp0Request(struct usb_request *req);
extern void mgc_complete_ep0_request(void);
extern int MGC_ReadUSBControlRequest(MGC_LinuxCd* pThis, uint16_t wCount);

extern int ep0_txstate(void);
extern int ep0_rxstate(void);

extern int MGC_gtest_init(void); 

static inline struct usb_request* MGC_CurrentRequest(MGC_GadgetLocalEnd* pEnd) {
    return ( list_empty(&((pEnd)->req_list) )? NULL : \
	list_entry((pEnd)->req_list.next, struct usb_request, list) );
}

extern char* dump_usb_request(struct usb_request *req);

#if MUSB_DEBUG>0
extern uint8_t is_ep_corrupted(void *ep);
extern int MGC_GadgetFindEnd(struct usb_ep* pGadgetEnd);
#endif

extern int MGC_GadgetSetHalt(struct usb_ep *ep, int value);


/* --------------------------- commodities  --------------------------- */
 
#define DOWHILE(_x)	do { _x } while(0)

#ifdef MUSB_GSTORAGE
#define __ep_to_endp(_ep)	MGC_GadgetFindEnd(_ep)
#else
#define __ep_to_endp(_ep)	( ((MGC_GadgetLocalEnd*)(_ep))->bEndNumber )
#endif

#ifdef MUSB_PARANOID
#define EP_NUMBER(_ep)  ( is_ep_corrupted(_ep)?-1: __ep_to_endp(_ep) )
#define MGC_IsIdle(_ep) ( is_ep_corrupted(_ep)?1:list_empty(&((MGC_GadgetLocalEnd*)(_ep))->req_list))
#else
#define MGC_IsIdle(_ep) ( list_empty(&((MGC_GadgetLocalEnd*)(_ep))->req_list) )
#define EP_NUMBER(_ep)  __ep_to_endp(_ep)
#endif

#define EP_SPIN_LOCK(_ep)	spin_lock( &((MGC_GadgetLocalEnd*)(_ep))->Lock )
#define EP_SPIN_UNLOCK(_ep)	spin_unlock( &((MGC_GadgetLocalEnd*)(_ep))->Lock )

#define EP_SPIN_LOCK_IRQSAVE(_ep, _flags) spin_lock_irqsave(&((MGC_GadgetLocalEnd*)(_ep))->Lock, _flags )	
#define EP_SPIN_UNLOCK_IRQRESTORE(_ep, _flags) spin_unlock_irqrestore( &((MGC_GadgetLocalEnd*)(_ep))->Lock, _flags)

#define EP_QUEUE_LENGTH(_ep)    queue_length(&(MGC_GadgetLocalEnd*)(_ep)->req_list)

/* ----------------------- Gadget Cross Compile ----------------------- */

#ifdef GADGET_FULL
#define GADGET_SET_IS_DUALSPEED(_g, _v)	DOWHILE((_g)->is_dualspeed =_v;)
#define GADGET_SET_IS_OTG(_g, _v)	DOWHILE((_g)->is_otg =_v;)
#define GADGET_SET_B_HNP_ENABLE(_g, _v)	DOWHILE((_g)->b_hnp_enable =_v;)
#define GADGET_SET_A_HNP_SUPPORT(_g, _v) DOWHILE((_g)->a_hnp_support = _v;)
#define GADGET_SET_A_ALT_HNP_SUPPORT(_g, _v) DOWHILE((_g)->a_alt_hnp_support = _v;)
#else
#define GADGET_SET_IS_DUALSPEED(_g, _v)		do { } while(0)
#define GADGET_SET_IS_OTG(_g, _v)		do { } while(0)	
#define GADGET_SET_B_HNP_ENABLE(_g, _v)		do { } while(0)
#define GADGET_SET_A_HNP_SUPPORT(_g, _v)	do { } while(0)
#define GADGET_SET_A_ALT_HNP_SUPPORT(_g, _v) 	do { } while(0)
#endif


#endif /* __MUSB_GADGETDEFS_H */
